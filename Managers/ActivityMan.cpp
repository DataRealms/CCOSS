//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActivityMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ActivityMan.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"
#include "GAScripted.h"
#include "Controller.h"
#include "Scene.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

extern bool g_ResetActivity;
extern bool g_ResumeActivity;
extern bool g_InActivity;

using namespace std;

namespace RTE
{
    
ABSTRACTCLASSINFO(Activity, Entity)
const string ActivityMan::m_ClassName = "ActivityMan";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Activity, effectively
//                  resetting the members of this abstraction level only.

void Activity::Clear()
{
    m_Description.clear();
    m_InCampaignStage = -1;
    m_MaxPlayerSupport = MAXPLAYERCOUNT;
    m_MinTeamsRequired = 2;
    m_ActivityState = NOTSTARTED;
    m_Paused = false;
    m_TeamCount = 1;
    m_PlayerCount = 1;
    m_Difficulty = 50;
	m_IsMultiplayerReady = false;

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        m_Team[player] = Activity::TEAM_1;
        m_FundsContribution[player] = 0;
        m_TeamFundsShare[player] = 1.0;
        // Player 1 is active by default, for the editor etc
        m_IsActive[player] = player == PLAYER_1;
        m_IsHuman[player] = player == PLAYER_1;
        m_PlayerScreen[player] = player == PLAYER_1 ? 0 : -1;
        m_pBrain[player] = 0;
        m_HadBrain[player] = false;
        m_BrainEvacuated[player] = false;
        m_ViewState[player] = NORMAL;
        m_pControlledActor[player] = 0;
        m_PlayerController[player].Reset();
        m_MsgTimer[player].Reset();
    }

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        // Team 1 is active by default, for the editors etc
        m_TeamActive[team] = team == TEAM_1;
        m_TeamFunds[team] = 2000;
        m_FundsChanged[team] = false;
        m_TeamIcons[team].Reset();
        m_TeamDeaths[team] = 0;
		m_TeamAISkillLevels[team] = DEFAULTSKILL;
    }

    m_TeamNames[TEAM_1] = "Team 1";
    m_TeamNames[TEAM_2] = "Team 2";
    m_TeamNames[TEAM_3] = "Team 3";
    m_TeamNames[TEAM_4] = "Team 4";

    m_FundsChangedSound.Reset();
    m_ActorSwitchSound.Reset();
    m_BrainSwitchSound.Reset();
    m_CameraTravelSound.Reset();
    m_ConfirmSound.Reset();
    m_SelectionChangeSound.Reset();
    m_UserErrorSound.Reset();

	m_CraftsOrbitAtTheEdge = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Activity object ready for use.

int Activity::Create()
{
    if (Entity::Create() < 0)
        return -1;

// This should be empty so that it can be detected if no desc has been set in the ini, and a good default can be given by the leaf classes
//    m_Description = "No description defined for this Activity!";

    ////////////////////////////////
    // Set up teams

    // Fill team icon flags with default ones that have already been loaded into the PresetMan
    m_TeamIcons[Activity::TEAM_1] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1 Default"));
    m_TeamIcons[Activity::TEAM_2] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2 Default"));
    m_TeamIcons[Activity::TEAM_3] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3 Default"));
    m_TeamIcons[Activity::TEAM_4] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4 Default"));
/*
    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        m_TeamIcons[team].Reset();
    }

    ///////////////////////////////////////
    // Set up players

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }
*/
// DON'T DO THIS HERE - This is now done by the ActivityMan jsut before starting this
// This clears out the team activations from ini that are supposed to tell the scenariogui what possiblities this activity has
//    SetupPlayers();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Activity to be identical to another, by deep copy.

int Activity::Create(const Activity &reference)
{
    Entity::Create(reference);

    m_Description = reference.m_Description;
    m_InCampaignStage = reference.m_InCampaignStage;
    m_MaxPlayerSupport = reference.m_MaxPlayerSupport;
    m_MinTeamsRequired = reference.m_MinTeamsRequired;
    m_ActivityState = reference.m_ActivityState;
    m_Paused = reference.m_Paused;
    m_TeamCount = reference.m_TeamCount;
    m_PlayerCount = reference.m_PlayerCount;
    m_Difficulty = reference.m_Difficulty;

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        m_Team[player] = reference.m_Team[player];
        m_FundsContribution[player] = reference.m_FundsContribution[player];
        m_TeamFundsShare[player] = reference.m_TeamFundsShare[player];
        m_IsActive[player] = reference.m_IsActive[player];
        m_IsHuman[player] = reference.m_IsHuman[player];
        m_PlayerScreen[player] = reference.m_PlayerScreen[player];
        m_pBrain[player] = reference.m_pBrain[player];
        m_HadBrain[player] = reference.m_HadBrain[player];
        m_BrainEvacuated[player] = reference.m_BrainEvacuated[player];
        m_ViewState[player] = reference.m_ViewState[player];
        m_pControlledActor[player] = reference.m_pControlledActor[player];
        m_PlayerController[player] = reference.m_PlayerController[player];
    }

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        m_TeamActive[team] = reference.m_TeamActive[team];
        m_TeamFunds[team] = reference.m_TeamFunds[team];
        m_FundsChanged[team] = reference.m_FundsChanged[team];
        m_TeamNames[team] = reference.m_TeamNames[team];
        if (reference.m_TeamIcons[team].GetFrameCount() > 0)
            m_TeamIcons[team] = reference.m_TeamIcons[team];
        m_TeamDeaths[team] = reference.m_TeamDeaths[team];
		m_TeamAISkillLevels[team] = reference.m_TeamAISkillLevels[team];
    }

    m_FundsChangedSound = reference.m_FundsChangedSound;
    m_ActorSwitchSound = reference.m_ActorSwitchSound;
    m_BrainSwitchSound = reference.m_BrainSwitchSound;
    m_CameraTravelSound = reference.m_CameraTravelSound;
    m_ConfirmSound = reference.m_ConfirmSound;
    m_SelectionChangeSound = reference.m_SelectionChangeSound;
    m_UserErrorSound = reference.m_UserErrorSound;

	m_CraftsOrbitAtTheEdge = reference.m_CraftsOrbitAtTheEdge;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Activity::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Description")
        reader >> m_Description;
    else if (propName == "SceneName")
        reader >> m_SceneName;
    else if (propName == "InCampaignStage")
        reader >> m_InCampaignStage;
    else if (propName == "MaxPlayerSupport")
        reader >> m_MaxPlayerSupport;
    else if (propName == "MinTeamsRequired")
        reader >> m_MinTeamsRequired;
// Gets tallied as team assignments are read in below, so doesn't really matter here, but should still be handled
    else if (propName == "TeamCount")
        reader.ReadPropValue();//reader >> m_TeamCount;
    else if (propName == "PlayerCount")
        reader >> m_PlayerCount;
    else if (propName == "Difficulty")
        reader >> m_Difficulty;
    else if (propName == "TeamOfPlayer1")
    {
        int team;
        reader >> team;
        if (team >= TEAM_1 && team < MAXTEAMCOUNT)
        {
            m_Team[PLAYER_1] = team;
            m_IsActive[PLAYER_1] = true;
            if (!m_TeamActive[team])
                m_TeamCount++;
            m_TeamActive[team] = true;
        }
        // Non-active player if team is out of range
        else
            m_IsActive[PLAYER_1] = false;
    }
    else if (propName == "TeamOfPlayer2")
    {
        int team;
        reader >> team;
        if (team >= TEAM_1 && team < MAXTEAMCOUNT)
        {
            m_Team[PLAYER_2] = team;
            m_IsActive[PLAYER_2] = true;
            if (!m_TeamActive[team])
                m_TeamCount++;
            m_TeamActive[team] = true;
        }
        // Non-active player if team is out of range
        else
            m_IsActive[PLAYER_2] = false;
    }
    else if (propName == "TeamOfPlayer3")
    {
        int team;
        reader >> team;
        if (team >= TEAM_1 && team < MAXTEAMCOUNT)
        {
            m_Team[PLAYER_3] = team;
            m_IsActive[PLAYER_3] = true;
            if (!m_TeamActive[team])
                m_TeamCount++;
            m_TeamActive[team] = true;
        }
        // Non-active player if team is out of range
        else
            m_IsActive[PLAYER_3] = false;
    }
    else if (propName == "TeamOfPlayer4")
    {
        int team;
        reader >> team;
        if (team >= TEAM_1 && team < MAXTEAMCOUNT)
        {
            m_Team[PLAYER_4] = team;
            m_IsActive[PLAYER_4] = true;
            if (!m_TeamActive[team])
                m_TeamCount++;
            m_TeamActive[team] = true;
        }
        // Non-active player if team is out of range
        else
            m_IsActive[PLAYER_4] = false;
    }
    else if (propName == "FundsContributionOfPlayer1")
        reader >> m_FundsContribution[PLAYER_1];
    else if (propName == "FundsContributionOfPlayer2")
        reader >> m_FundsContribution[PLAYER_2];
    else if (propName == "FundsContributionOfPlayer3")
        reader >> m_FundsContribution[PLAYER_3];
    else if (propName == "FundsContributionOfPlayer4")
        reader >> m_FundsContribution[PLAYER_4];
    else if (propName == "TeamFundsShareOfPlayer1")
        reader >> m_TeamFundsShare[PLAYER_1];
    else if (propName == "TeamFundsShareOfPlayer2")
        reader >> m_TeamFundsShare[PLAYER_2];
    else if (propName == "TeamFundsShareOfPlayer3")
        reader >> m_TeamFundsShare[PLAYER_3];
    else if (propName == "TeamFundsShareOfPlayer4")
        reader >> m_TeamFundsShare[PLAYER_4];
    else if (propName == "Player1IsHuman")
        reader >> m_IsHuman[PLAYER_1];
    else if (propName == "Player2IsHuman")
        reader >> m_IsHuman[PLAYER_2];
    else if (propName == "Player3IsHuman")
        reader >> m_IsHuman[PLAYER_3];
    else if (propName == "Player4IsHuman")
        reader >> m_IsHuman[PLAYER_4];
    else if (propName == "Team1Funds" || propName == "FundsOfTeam1")
        reader >> m_TeamFunds[TEAM_1];
    else if (propName == "Team2Funds" || propName == "FundsOfTeam2")
        reader >> m_TeamFunds[TEAM_2];
    else if (propName == "Team3Funds" || propName == "FundsOfTeam3")
        reader >> m_TeamFunds[TEAM_3];
    else if (propName == "Team4Funds" || propName == "FundsOfTeam4")
        reader >> m_TeamFunds[TEAM_4];
    else if (propName == "Team1Name")
    {
        reader >> m_TeamNames[TEAM_1];
        if (!m_TeamActive[TEAM_1])
            m_TeamCount++;
        m_TeamActive[TEAM_1] = true;
    }
    else if (propName == "Team2Name")
    {
        reader >> m_TeamNames[TEAM_2];
        if (!m_TeamActive[TEAM_2])
            m_TeamCount++;
        m_TeamActive[TEAM_2] = true;
    }
    else if (propName == "Team3Name")
    {
        reader >> m_TeamNames[TEAM_3];
        if (!m_TeamActive[TEAM_3])
            m_TeamCount++;
        m_TeamActive[TEAM_3] = true;
    }
    else if (propName == "Team4Name")
    {
        reader >> m_TeamNames[TEAM_4];
        if (!m_TeamActive[TEAM_4])
            m_TeamCount++;
        m_TeamActive[TEAM_4] = true;
    }
    else if (propName == "Team1Icon")
        reader >> m_TeamIcons[TEAM_1];
    else if (propName == "Team2Icon")
        reader >> m_TeamIcons[TEAM_2];
    else if (propName == "Team3Icon")
        reader >> m_TeamIcons[TEAM_3];
    else if (propName == "Team4Icon")
        reader >> m_TeamIcons[TEAM_4];
	else if (propName == "CraftsOrbitAtTheEdge")
		reader >> m_CraftsOrbitAtTheEdge;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Activity with a Writer for
//                  later recreation with Create(Reader &reader);

int Activity::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("Description");
    writer << m_Description;
    writer.NewProperty("SceneName");
    writer << m_SceneName;
    writer.NewProperty("InCampaignStage");
    writer << m_InCampaignStage;
    writer.NewProperty("MaxPlayerSupport");
    writer << m_MaxPlayerSupport;
    writer.NewProperty("MinTeamsRequired");
    writer << m_MinTeamsRequired;
    writer.NewProperty("TeamCount");
    writer << m_TeamCount;
    writer.NewProperty("PlayerCount");
    writer << m_PlayerCount;
    writer.NewProperty("Difficulty");
    writer << m_Difficulty;
    if (m_IsActive[PLAYER_1])
    {
        writer.NewProperty("TeamOfPlayer1");
        writer << m_Team[PLAYER_1];
        writer.NewProperty("FundsContributionOfPlayer1");
        writer << m_FundsContribution[PLAYER_1];
        writer.NewProperty("TeamFundsShareOfPlayer1");
        writer << m_TeamFundsShare[PLAYER_1];
        writer.NewProperty("Player1IsHuman");
        writer << m_IsHuman[PLAYER_1];
    }
    else
    {
        writer.NewProperty("TeamOfPlayer1");
        writer << NOTEAM;
    }
    if (m_IsActive[PLAYER_2])
    {
        writer.NewProperty("TeamOfPlayer2");
        writer << m_Team[PLAYER_2];
        writer.NewProperty("FundsContributionOfPlayer2");
        writer << m_FundsContribution[PLAYER_2];
        writer.NewProperty("TeamFundsShareOfPlayer2");
        writer << m_TeamFundsShare[PLAYER_2];
        writer.NewProperty("Player2IsHuman");
        writer << m_IsHuman[PLAYER_2];
    }
    else
    {
        writer.NewProperty("TeamOfPlayer2");
        writer << NOTEAM;
    }
    if (m_IsActive[PLAYER_3])
    {
        writer.NewProperty("TeamOfPlayer3");
        writer << m_Team[PLAYER_3];
        writer.NewProperty("FundsContributionOfPlayer3");
        writer << m_FundsContribution[PLAYER_3];
        writer.NewProperty("TeamFundsShareOfPlayer3");
        writer << m_TeamFundsShare[PLAYER_3];
        writer.NewProperty("Player3IsHuman");
        writer << m_IsHuman[PLAYER_3];
    }
    else
    {
        writer.NewProperty("TeamOfPlayer3");
        writer << NOTEAM;
    }
    if (m_IsActive[PLAYER_4])
    {
        writer.NewProperty("TeamOfPlayer4");
        writer << m_Team[PLAYER_4];
        writer.NewProperty("FundsContributionOfPlayer4");
        writer << m_FundsContribution[PLAYER_4];
        writer.NewProperty("TeamFundsShareOfPlayer4");
        writer << m_TeamFundsShare[PLAYER_4];
        writer.NewProperty("Player4IsHuman");
        writer << m_IsHuman[PLAYER_4];
    }
    else
    {
        writer.NewProperty("TeamOfPlayer4");
        writer << NOTEAM;
    }
    if (m_TeamActive[Activity::TEAM_1])
    {
        writer.NewProperty("Team1Funds");
        writer << m_TeamFunds[TEAM_1];
        writer.NewProperty("Team1Name");
        writer << m_TeamNames[TEAM_1];
        writer.NewProperty("Team1Icon");
        m_TeamIcons[TEAM_1].SavePresetCopy(writer);
    }
    if (m_TeamActive[Activity::TEAM_2])
    {
        writer.NewProperty("Team2Funds");
        writer << m_TeamFunds[TEAM_2];
        writer.NewProperty("Team2Name");
        writer << m_TeamNames[TEAM_2];
        writer.NewProperty("Team2Icon");
        m_TeamIcons[TEAM_2].SavePresetCopy(writer);
    }
    if (m_TeamActive[Activity::TEAM_3])
    {
        writer.NewProperty("Team3Funds");
        writer << m_TeamFunds[TEAM_3];
        writer.NewProperty("Team3Name");
        writer << m_TeamNames[TEAM_3];
        writer.NewProperty("Team3Icon");
        m_TeamIcons[TEAM_3].SavePresetCopy(writer);
    }
    if (m_TeamActive[Activity::TEAM_4])
    {
        writer.NewProperty("Team4Funds");
        writer << m_TeamFunds[TEAM_4];
        writer.NewProperty("Team4Name");
        writer << m_TeamNames[TEAM_4];
        writer.NewProperty("Team4Icon");
        m_TeamIcons[TEAM_4].SavePresetCopy(writer);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Activity object.

void Activity::Destroy(bool notInherited)
{
/*
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }
*/
/*
    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        ;
    }
*/

    if (!notInherited)
        Entity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearPlayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all players as not active in the current game.

void Activity::ClearPlayers(bool resetFunds)
{
    for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player)
    {
        m_IsActive[player] = false;
        m_IsHuman[player] = false;

        if (resetFunds)
        {
            m_FundsContribution[player] = 0;
            m_TeamFundsShare[player] = 1.0;
        }
    }
    
    for (int team = TEAM_1; team < MAXTEAMCOUNT; ++team)
    {
        m_TeamActive[team] = false;
        if (resetFunds)
            m_TeamFunds[team] = 0;
    }

    m_PlayerCount = m_TeamCount = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up a specific player for this activity, AI or Human

int Activity::AddPlayer(int player, bool isHuman, int team, int funds, const Icon *pTeamIcon)
{
    if (player < PLAYER_1 || player >= MAXPLAYERCOUNT || team < TEAM_1 || team >= MAXTEAMCOUNT)
        return m_PlayerCount;

    // Activate team
    if (!m_TeamActive[team])
    {
        // This guy is responsible for all this team's funds so far
        m_TeamFundsShare[player] = 1.0;
        m_TeamCount++;
    }
    // Team already existed, so adjust the contribution shares of all teammembers accordingly
    else
    {
        float totalFunds = m_TeamFunds[team] + funds;
        float newRatio = 1.0 + (funds / totalFunds);
        // Go through all teammembers of this team, not including the one we're just adding (he's not activated yet)
        for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p)
        {
            if (m_IsActive[p] && m_Team[p] == team)
                m_TeamFundsShare[p] /= newRatio;
        }

        // The new added player's share
        m_TeamFundsShare[player] = funds / totalFunds;
    }
    m_TeamActive[team] = true;
    if (pTeamIcon)
        SetTeamIcon(team, *pTeamIcon);

    // Activate player after we did the above team fund share computation
    if (!m_IsActive[player])
        m_PlayerCount++;
    m_IsActive[player] = true;

    m_IsHuman[player] = isHuman;
    m_Team[player] = team;
    m_TeamFunds[team] += funds;
    m_FundsContribution[player] = funds;

    return m_PlayerCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerFundsContribution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new amount of starting funds for a player, after he has already
//                  been added.

bool Activity::UpdatePlayerFundsContribution(int player, int newFunds)
{
    // Player and his team have to already be active and kicking
    if (player < PLAYER_1 || player >= MAXPLAYERCOUNT || !m_IsActive[player] || !m_TeamActive[m_Team[player]])
        return false;

    // If this guy is responsible for all funds, then just update his contribution and be done
    if (m_TeamFundsShare[player] == 1.0)
    {
        m_FundsContribution[player] = newFunds;
        m_TeamFunds[m_Team[player]] = newFunds;
    }
    // Ok more complex sitation of multiple players on this guy's team, so adjust all shares accordingly
    else
    {
        // Update the contribution of the player
        m_FundsContribution[player] = newFunds;

        // Tally up all the funds of all players on this guy's team
        for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p)
        {
            if (m_IsActive[p] && m_Team[p] == m_Team[player])
                m_TeamFunds[m_Team[player]] += m_FundsContribution[p];
        }

        // Now that we have the updated total, update the shares of all team players
        for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p)
        {
            if (m_IsActive[p] && m_Team[p] == m_Team[player])
                m_TeamFundsShare[p] = m_FundsContribution[p] / m_TeamFunds[m_Team[player]];
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DeactivatePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns off a player if they were active. Should only be done if brain
//                  etc are already taken care of and disposed of properly. Will also
//                  deactivate the team this player is on, if there's no other active
//                  players still on it.

bool Activity::DeactivatePlayer(int player)
{
    if (player < PLAYER_1 || player >= MAXPLAYERCOUNT)
        return false;

    // Nothing to deactivate, so just quit
    if (!m_IsActive[player] || !m_TeamActive[m_Team[player]])
        return false;

    // See if this was the last player on its team
    bool lastOnTeam = true;
    for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p)
    {
        if (m_IsActive[p] && p != player && m_Team[p] == m_Team[player])
            lastOnTeam = false;
    }
    // If we were the last, then deact the team as well
    if (lastOnTeam)
    {
        m_TeamActive[m_Team[player]] = false;
        m_TeamCount--;
    }

    // Deactivate the player
    m_IsActive[player] = false;
    m_PlayerCount--;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHumanCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of human players in the current game

int Activity::GetHumanCount() const
{
    int humans = 0;
    for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player] && m_IsHuman[player])
            humans++;
    }
    return humans;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current team a specific player belongs to.

void Activity::SetTeamOfPlayer(int player, int team)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT)
		return;

    m_Team[player] = team;
    m_TeamActive[team] = true;
    m_IsActive[player] = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayersInTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current number of players in a specific team.

int Activity::PlayersInTeamCount(int team) const
{
    int count = 0;
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player] && m_Team[player] == team)
            count++;
    }
    return count;
}

/* Now cached in m_PlayerScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ScreenOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts a player index into a screen index, and only if that player
//                  is human.

int Activity::ScreenOfPlayer(int player) const
{
    if (!m_IsActive[player] || !m_IsHuman[player] || player >= MAXPLAYERCOUNT)
        return -1;

    // Calculate which screen this human player is using, based on how many non-human players there are before him
    int screenIndex = -1;
    for (int p = Activity::PLAYER_1; p < MAXPLAYERCOUNT && p <= player; ++p)
    {
        if (m_IsActive[p] && m_IsHuman[p])
            screenIndex++;
    }
    return screenIndex;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPlayerBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current Brain actor for a specific player.

void Activity::SetPlayerBrain(Actor *pNewBrain, int player)
{
	if (player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT)
		return;

    if (pNewBrain)
    {
        pNewBrain->SetTeam(m_Team[player]);
        // Note that we have now had a brain
        m_HadBrain[player] = true;
    }

    m_pBrain[player] = pNewBrain;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HumanBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many human-controlled brains are left in this Activity.

int Activity::HumanBrainCount()
{
    int brainCount = 0;

    for (int player = Activity::PLAYER_1; player < Activity::MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player] && m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_pBrain[player]) || (m_pBrain[player] && m_pBrain[player]->HasObjectInGroup("Brains"))))
            brainCount++;
    }

    return brainCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AIBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many A.I.-controlled brains are left in this Activity.

int Activity::AIBrainCount()
{
    int brainCount = 0;

    for (int player = Activity::PLAYER_1; player < Activity::MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player] && !m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_pBrain[player]) || (m_pBrain[player] && m_pBrain[player]->HasObjectInGroup("Brains"))))
            brainCount++;
    }

    return brainCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeTeamFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes a team's funds level by a certain amount.

void Activity::ChangeTeamFunds(float howMuch, unsigned int which)
{
	if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT)
	{
		m_TeamFunds[which] += howMuch;
		m_FundsChanged[which] = true;
		if (IsPlayerTeam(which))
			m_FundsChangedSound.Play();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TeamFundsChanged
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whetehr the team funds changed since last time this was called.
//                  This also resets the state, so calling this again on the same team will
//                  ield false unless it's been changed again.

bool Activity::TeamFundsChanged(unsigned int which)
{
	if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT)
	{
		bool changed = m_FundsChanged[which];
		m_FundsChanged[which] = false;
		return changed;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EnteredOrbit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates an Actor as having left the game scene and entered orbit.
//                  OWNERSHIP IS NOT transferred, as the Actor's inventory is just 'unloaded'.

void Activity::EnteredOrbit(Actor *pActor)
{
    if (!pActor)
        return;

    int team  = pActor->GetTeam();
/*
    if (m_pOrbitRocket[team])
    {
        delete m_pOrbitRocket[team];
        m_pOrbitRocket[team] = 0;
    }
*/

	// Find out cost multipliers for metagame, or we end up AI's 
	// earning money when their crafts return if their nativeCostMultiplier is too low
    float foreignCostMult = 1.0;
	float nativeCostMult = 1.0;
	if (g_MetaMan.GameInProgress())
	{
		for (int player = 0 ; player < MAXPLAYERCOUNT; player++)
		{
			if (GetTeamOfPlayer(player) == pActor->GetTeam())
			{
				MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
				if (pMetaPlayer)
				{
					foreignCostMult = pMetaPlayer->GetForeignCostMultiplier();
					nativeCostMult = pMetaPlayer->GetNativeCostMultiplier();
				}
			}
		}
	}

    // Did a brain just evacuate the Scene??
    bool brainOnBoard = pActor->HasObjectInGroup("Brains");
    // Total value of ship and cargo and crew
    float totalValue = pActor->GetTotalValue(0, foreignCostMult, nativeCostMult);
// TODO ARGH WHAT IF TWO PLAYERS ON SAME TEAM ARE OF DIFFERENT TECHSS??!?!?!?$?!?!$?!$?
// A: Just let the base cost be the liquidation value.. they could cheat otherwise, one buying, one selling
    char str[64];
    sprintf(str, "Returned Craft + Cargo added %.0f oz to Funds!", totalValue);
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player])
        {
            // Figure out whose brain just left the building
            if (brainOnBoard && pActor == GetPlayerBrain(player))
            {
                m_BrainEvacuated[player] = true;
                sprintf(str, "YOUR BRAIN HAS BEEN EVACUATED BACK INTO ORBIT!");
            }

            if (m_Team[player] == team)
            {
                g_FrameMan.ClearScreenText(ScreenOfPlayer(ScreenOfPlayer(player)));
                g_FrameMan.SetScreenText(str, ScreenOfPlayer(player), 0, 3500);
                m_MsgTimer[player].Reset();
            }
        }
    }

    m_TeamFunds[team] += totalValue;
    pActor->SetGoldCarried(0);
    pActor->SetHealth(pActor->GetMaxHealth());
//    m_pOrbitRocket[team] = pActor;

    // Counter-adjust the death toll because the craft leaving (being deleted) will increment
    m_TeamDeaths[team]--;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReassignSquadLeader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the player's ControlledActor the leader of any squad it is a
//                  momber of.

void Activity::ReassignSquadLeader(const int player, const int team)
{
    if (m_pControlledActor[player]->GetAIMode() == Actor::AIMODE_SQUAD)
    {
        // Get the next actor
        MOID leaderID = m_pControlledActor[player]->GetAIMOWaypointID();   // Get the id of the Squad Leader
        if (leaderID != g_NoMOID)
        {
            Actor *pActor = g_MovableMan.GetNextTeamActor(team, m_pControlledActor[player]);
            
            do
            {
                // Set the controlled actor as new leader if pActor follow the old leader, and not player controlled and not brain
                if (pActor && (pActor->GetAIMode() == Actor::AIMODE_SQUAD) && (pActor->GetAIMOWaypointID() == leaderID) &&
                    !pActor->GetController()->IsPlayerControlled() && !pActor->IsInGroup("Brains"))
                {
                    pActor->ClearAIWaypoints();
                    pActor->AddAIMOWaypoint(m_pControlledActor[player]);
                    pActor->UpdateMovePath();   // Make sure pActor has m_pControlledActor registered as an AIMOWaypoint
                }
                // Set the old leader to follow the controlled actor
				else if (pActor && pActor->GetID() == leaderID)
                {
                    m_pControlledActor[player]->ClearAIWaypoints();
                    m_pControlledActor[player]->SetAIMode((Actor::AIMode)pActor->GetAIMode()); // Inherit the old leader's AI mode

                    if (m_pControlledActor[player]->GetAIMode() == Actor::AIMODE_GOTO)
                    {
                        // Copy the old leaders move orders
                        if (pActor->GetAIMOWaypointID() != g_NoMOID)
                        {
                            MovableObject * TargetMO = g_MovableMan.GetMOFromID(pActor->GetAIMOWaypointID());
                            if (TargetMO)
                                m_pControlledActor[player]->AddAIMOWaypoint(TargetMO);
                        }
                        else if ((pActor->GetLastAIWaypoint()-pActor->GetPos()).GetLargest() > 1)
                        {
                            m_pControlledActor[player]->AddAISceneWaypoint(pActor->GetLastAIWaypoint());
                        }
                    }
                    
                    pActor->ClearAIWaypoints();
                    pActor->SetAIMode(Actor::AIMODE_SQUAD);
                    pActor->AddAIMOWaypoint(m_pControlledActor[player]);
                    pActor->UpdateMovePath();   // Make sure pActor has m_pControlledActor registered as an AIMOWaypoint
                }
                
                // Next!
                pActor = g_MovableMan.GetNextTeamActor(team, pActor);
            }
            while (pActor && pActor != m_pControlledActor[player]);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchToActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the Activity to focus player control to a specific Actor for a
//                  specific team. OWNERSHIP IS NOT TRANSFERRED!

bool Activity::SwitchToActor(Actor *pActor, int player, int team)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT)
		return false;

    if (!pActor || !g_MovableMan.IsActor(pActor))
        return false;

    // Computer players don't focus on any Actor
    if (!m_IsHuman[player])
        return false;

    // If the desired actor is not the brain and is controlled by someone else, 
    // OR if it's actually a brain of another player, we can't switch to it
    if (pActor != m_pBrain[player] && pActor->IsPlayerControlled() || IsOtherPlayerBrain(m_pControlledActor[player], player))
    {
        m_UserErrorSound.Play();
        return false;
    }

    Actor *pPrevActor = 0;
    // Disable human player control of the actor we're switching from
    if (m_pControlledActor[player] && g_MovableMan.IsActor(m_pControlledActor[player]))
    {
        // Set AI controller active to the guy we're switching from
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_AI);
        m_pControlledActor[player]->GetController()->SetDisabled(false);
        // Stop showing its waypoints
//        m_pControlledActor[player]->DrawWaypoints(false);
        pPrevActor = m_pControlledActor[player];
    }

    // Switch
    m_pControlledActor[player] = pActor;

    // Now set the controller and team of the actor we switched to.
    if (m_pControlledActor[player])
    {
        m_pControlledActor[player]->SetTeam(team);
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);
        m_pControlledActor[player]->GetController()->SetDisabled(false);

        // Play actor switching sound effects
        // Brain has its own special sound effects
        if (m_pControlledActor[player] == m_pBrain[player])
            m_BrainSwitchSound.Play();
        else
            m_ActorSwitchSound.Play();
        // Only play air swoosh if actors are out of sight of each other
        if (pPrevActor && Vector(pPrevActor->GetPos() - m_pControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2)
            m_CameraTravelSound.Play();

        ReassignSquadLeader(player, team);
    }

    // Follow the new guy normally
    m_ViewState[player] = NORMAL;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchToNextActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the Activity to focus player control to the next Actor of a
//                  specific team, other than the current one focused on.

void Activity::SwitchToNextActor(int player, int team, Actor *pSkip)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT)
		return;

    Actor *pPrevActor = 0;
    // Disable human player control of the actor we're switching from
    if (m_pControlledActor[player] && g_MovableMan.IsActor(m_pControlledActor[player]))
    {
        // Set AI controller active to the guy we're switching from
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_AI);
        m_pControlledActor[player]->GetController()->SetDisabled(false);
        // Stop showing its waypoints
//        m_pControlledActor[player]->DrawWaypoints(false);
        pPrevActor = m_pControlledActor[player];
    }
    else
        m_pControlledActor[player] = 0;

    // Get next Actor after the current one.
    m_pControlledActor[player] = g_MovableMan.GetNextTeamActor(team, m_pControlledActor[player]);

    // If it's the skip guy, then skip him
    // OR if it's a non-brain guy controlled by another player on the same team, then skip him too
    Actor *pStart = m_pControlledActor[player];
    while ((pSkip && pSkip == m_pControlledActor[player]) ||
           (m_pControlledActor[player] && m_pControlledActor[player] != m_pBrain[player] && m_pControlledActor[player]->IsPlayerControlled()) ||
           (IsOtherPlayerBrain(m_pControlledActor[player], player)))
    {
        // Check previous in line
        m_pControlledActor[player] = g_MovableMan.GetNextTeamActor(team, m_pControlledActor[player]);

        // Looped around the whole actor chain, and couldn't find an available actor, so switch back to the original
        if (m_pControlledActor[player] == pPrevActor)
        {
            m_UserErrorSound.Play();
            m_pControlledActor[player] = pPrevActor;
            break;
        }
        // If we're gone around without even finding the one we started with (he might be dead),
        // then try to switch to the brain, and if not that, then just the start
        if (m_pControlledActor[player] == pStart)
        {
            if (m_pBrain[player])
                m_pControlledActor[player] = m_pBrain[player];
            else if (g_MovableMan.IsActor(pStart))
                m_pControlledActor[player] = pStart;
            else
                m_pControlledActor[player] = 0;
            break;
        }
    }

    // Now set the controller of the actor we switched to, and play switching sound fx
    if (m_pControlledActor[player])
    {
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);

        // Play actor switching sound effects
        // Brain has its own special sound effects
        if (m_pControlledActor[player] == m_pBrain[player])
            m_BrainSwitchSound.Play();
        else
            m_ActorSwitchSound.Play();
        // Only play air swoosh if actors are out of sight of each other
        if (pPrevActor && Vector(pPrevActor->GetPos() - m_pControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2)
            m_CameraTravelSound.Play();

        // Follow the new guy normally
        m_ViewState[player] = NORMAL;

        ReassignSquadLeader(player, team);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchToPrevActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the Activity to focus player control to the previous Actor of a
//                  specific team, other than the current one focused on.

void Activity::SwitchToPrevActor(int player, int team, Actor *pSkip)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT)
		return;

    Actor *pPrevActor = 0;
    // Disable human player control of the actor we're switching from
    if (m_pControlledActor[player] && g_MovableMan.IsActor(m_pControlledActor[player]))
    {
        // Set AI controller active to the guy we're switching from
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_AI);
        m_pControlledActor[player]->GetController()->SetDisabled(false);
        // Stop showing its waypoints
//        m_pControlledActor[player]->DrawWaypoints(false);
        pPrevActor = m_pControlledActor[player];
    }
    else
        m_pControlledActor[player] = 0;

    // Get previous Actor before the current one.
    m_pControlledActor[player] = g_MovableMan.GetPrevTeamActor(team, m_pControlledActor[player]);

    // If it's the skip guy, then skip him
    // OR if it's a non-brain guy controlled by another player on the same team, then skip him too
    // OR if it's a brain of another player
    Actor *pStart = m_pControlledActor[player];
    while ((pSkip && pSkip == m_pControlledActor[player]) ||
           (m_pControlledActor[player] && m_pControlledActor[player] != m_pBrain[player] && m_pControlledActor[player]->IsPlayerControlled()) ||
           (IsOtherPlayerBrain(m_pControlledActor[player], player)))
    {
        // Check previous in line
        m_pControlledActor[player] = g_MovableMan.GetPrevTeamActor(team, m_pControlledActor[player]);

        // Looped around the whole actor chain, and couldn't find an available actor, so switch back to the original
        if (m_pControlledActor[player] == pPrevActor)
        {
            m_UserErrorSound.Play();
            m_pControlledActor[player] = pPrevActor;
            break;
        }
        // If we're gone around without even finding the one we started with (he might be dead),
        // then try to switch to the brain, and if not that, then 0
        if (m_pControlledActor[player] == pStart)
        {
            if (m_pBrain[player])
                m_pControlledActor[player] = m_pBrain[player];
            else if (g_MovableMan.IsActor(pStart))
                m_pControlledActor[player] = pStart;
            else
                m_pControlledActor[player] = 0;
            break;
        }
    }

    // Now set the controller of the actor we switched to, and play switching sound fx
    if (m_pControlledActor[player])
    {
        m_pControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);

        // Play actor switching sound effects
        // Brain has its own special sound effects
        if (m_pControlledActor[player] == m_pBrain[player])
            m_BrainSwitchSound.Play();
        else
            m_ActorSwitchSound.Play();
        // Only play air swoosh if actors are out of sight of each other
        if (pPrevActor && Vector(pPrevActor->GetPos() - m_pControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2)
            m_CameraTravelSound.Play();

        ReassignSquadLeader(player, team);
    }

    // Follow the new guy normally
    m_ViewState[player] = NORMAL;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDifficulty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current difficulty setting

void Activity::SetDifficulty(int difficulty)
{
    // Translate specific enums to be within their ranges, and not just at the limits
    if (difficulty == CAKEDIFFICULTY)
        m_Difficulty = 10;
    else if (difficulty == EASYDIFFICULTY)
        m_Difficulty = 30;
    else if (difficulty == MEDIUMDIFFICULTY)
        m_Difficulty = 50;
    else if (difficulty == HARDDIFFICULTY)
        m_Difficulty = 70;
    else if (difficulty == NUTSDIFFICULTY)
        m_Difficulty = 95;
    else
        m_Difficulty = difficulty;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlayerTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a team is player controlled or not.

bool Activity::IsPlayerTeam(int team)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT)
		return false;

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (m_IsActive[player] && m_Team[player] == team && m_IsHuman[player])
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetupPlayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Precalculates the player-to-screen index map, counts the number of
//                  active players, teams etc.

void Activity::SetupPlayers()
{
    // Clear out the settings, we'll set them up again
    m_TeamCount = 0;
    m_PlayerCount = 0;
    // Don't clear a CPU team's active status though
    for (int team = Activity::TEAM_1; team < MAXTEAMCOUNT; ++team)
	{
		m_TeamActive[team] = false;
	}

    int screenIndex = -1;
    for (int player = Activity::PLAYER_1; player < MAXPLAYERCOUNT; ++player)
    {
        // Count all active players and teams
        if (m_IsActive[player])
        {
            m_PlayerCount++;
            if (!m_TeamActive[m_Team[player]])
                m_TeamCount++;
            // Also activate the team
            m_TeamActive[m_Team[player]] = true;
        }

        // Calculate which screen each human player is using, based on how many non-human players there are before him
        int screenIndex = -1;
        if (m_IsActive[player] && m_IsHuman[player])
        {
            for (int p = Activity::PLAYER_1; p < MAXPLAYERCOUNT && p <= player; ++p)
            {
                if (m_IsActive[p] && m_IsHuman[p])
                    screenIndex++;
            }
        }
        m_PlayerScreen[player] = screenIndex;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int Activity::Start()
{
    m_ActivityState = RUNNING;
    m_Paused = false;

    // Reset the mousemoving so that it won't trap the mouse if the window isn't in focus (common after loading)
	if (!g_FrameMan.IsInMultiplayerMode())
	{
		g_UInputMan.DisableMouseMoving(true);
		g_UInputMan.DisableMouseMoving(false);

		// Enable keys again
		g_UInputMan.DisableKeys(false);
	}

    // Load the scene
    int error = g_SceneMan.LoadScene();
    if (error < 0)
        return error;

    ////////////////////////////////
    // Set up teams

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        m_FundsChanged[team] = false;
        m_TeamDeaths[team] = 0;
    }

    ///////////////////////////////////////
    // Set up players

    // Intentionally doing all players, all need controllers
    int screen = -1;
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        m_ViewState[player] = NORMAL;
        g_FrameMan.ClearScreenText(ScreenOfPlayer(player));
        g_SceneMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(player));

        m_PlayerController[player].Destroy();
        m_PlayerController[player].Create(Controller::CIM_PLAYER, player);
        m_PlayerController[player].SetTeam(m_Team[player]);

        m_MsgTimer[player].Reset();
    }

    // Init sounds
    if (m_FundsChangedSound.GetSampleCount() < 1)
    {
        // Interface sounds should not be pitched, to reinforce the impression of time decoupling between simulation and UI
        m_FundsChangedSound.Create("Base.rte/GUIs/Sounds/Poing1.wav", false);
        m_FundsChangedSound.AddSample("Base.rte/GUIs/Sounds/Poing2.wav");
        m_FundsChangedSound.AddSample("Base.rte/GUIs/Sounds/Poing3.wav");
        m_FundsChangedSound.AddSample("Base.rte/GUIs/Sounds/Poing4.wav");
        m_FundsChangedSound.AddSample("Base.rte/GUIs/Sounds/Poing5.wav");
        m_FundsChangedSound.AddSample("Base.rte/GUIs/Sounds/Poing6.wav");
        m_ActorSwitchSound.Create("Base.rte/GUIs/Sounds/ActorSwitch3.wav", false);
        m_BrainSwitchSound.Create("Base.rte/GUIs/Sounds/ActorSwitch4.wav", false);
        m_CameraTravelSound.Create("Base.rte/GUIs/Sounds/Woosh1.wav", false);
        m_CameraTravelSound.AddSample("Base.rte/GUIs/Sounds/Woosh2.wav");
        m_CameraTravelSound.AddSample("Base.rte/GUIs/Sounds/Woosh3.wav");
        m_ConfirmSound.Create("Base.rte/GUIs/Sounds/MenuExitB.wav", false);
        m_SelectionChangeSound.Create("Base.rte/GUIs/Sounds/Splip01.wav", false);
        m_SelectionChangeSound.AddSample("Base.rte/GUIs/Sounds/Splip02.wav");
        m_UserErrorSound.Create("Base.rte/GUIs/Sounds/Error.wav", false);
    }

    // Precache the player to screen mappings
// DON'T DO THIS HERE - This is now done by the ActivityMan jsut before starting this
// This clears out the team activations from ini that are supposed to tell the scenariogui what possiblities this activity has
//    SetupPlayers();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void Activity::Pause(bool pause)
{
    m_Paused = pause;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void Activity::End()
{
    // Disable control of actors.. will be handed over to the observation targets instead
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        m_ViewState[player] = OBSERVE;
    }
/*
    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        ;
    }
*/
    m_ActivityState = OVER;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Activity. Supposed to be done every frame
//                  before drawing.

void Activity::Update()
{
    SLICK_PROFILE(0xFF343526);

    ///////////////////////////////////////////
    // Iterate through all human players

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        // Reset messages to nothing after a while
        if (m_MsgTimer[player].IsPastSimMS(5000))
//            g_FrameMan.SetScreenText("", ScreenOfPlayer(player));
            g_FrameMan.ClearScreenText(ScreenOfPlayer(player));

        // Update the player controllers.
        if (m_IsActive[player])
            m_PlayerController[player].Update();
    }

    ///////////////////////////////////////////
    // Iterate through all teams

    /*for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        ;
    }*/
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDifficultyString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns string representation of a given difficulty value

std::string Activity::GetDifficultyString(int difficulty)
{
        if (difficulty < CAKEDIFFICULTY)
            return "Cake";
        else if (difficulty < EASYDIFFICULTY)
            return "Easy";
        else if (difficulty < MEDIUMDIFFICULTY)
            return "Medium";
        else if (difficulty < HARDDIFFICULTY)
            return "Hard";
        else if (difficulty < NUTSDIFFICULTY)
            return "Nuts";
        else
            return "Nuts!";
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAISkillString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns string representation of a given AI skill value

std::string Activity::GetAISkillString(int skill)
{
        if (skill < INFERIORSKILL)
            return "Inferior";
        else if (skill < AVERAGESKILL)
            return "Average";
        else if (skill < GOODSKILL)
            return "Good";
        else
            return "Unfair";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamAISkill
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets AI skill level for specified team

void Activity::SetTeamAISkill(int team, int skill)
{
	if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT)
		return;

	if (skill < 1)
		skill = 1;
	if (skill > UNFAIRSKILL)
		skill = UNFAIRSKILL;

	if (team >= TEAM_1 && team < MAXTEAMCOUNT)
		m_TeamAISkillLevels[team] = skill;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamAISkill
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns skil level for specified team. If team is < 0 or > MAXTEAMCOUNT an average of all teams is returned.

int Activity::GetTeamAISkill(int team) const
{
	if (team >= TEAM_1 && team < MAXTEAMCOUNT)
		return m_TeamAISkillLevels[team];
	else
	{
		int avgskill = 0;
		int count = 0;

		for (int team = TEAM_1; team < MAXTEAMCOUNT; team++)
		{
			if (TeamActive(team))
			{
				avgskill += GetTeamAISkill(team);
				count++;
			}
		}
		if (count > 0)
			return avgskill / count;
		else
			return DEFAULTSKILL;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAISkillString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns string representation of a given AI skill value
// Arguments:       AI skill setting
// Return value:    Corresponding AI skill string


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void Activity::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
/*
    if (which >= 0 && which < MAXPLAYERCOUNT)
    {
        ;
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Activity's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void Activity::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
/*
    GUIFont *pLargeFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();
    AllegroBitmap pBitmapInt(pTargetBitmap);

    // Iterate through all players, drawing each currently used LZ cursor.
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }

    // Iterate through all teams, drawing all pending delivery cursors
    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        ;
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ActivityMan, effectively
//                  resetting the members of this abstraction level only.

void ActivityMan::Clear()
{
    m_DefaultActivityType = "GABaseDefense";
    m_DefaultActivityName = "Default Activity";
    m_pStartActivity = 0;
    m_pActivity = 0;
    m_LastMusicPath = "";
    m_LastMusicPos = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActivityMan object ready for use.

int ActivityMan::Create()
{
//    if (Serializable::Create() < 0)
//        return -1;

// Don't do this here, let RestartActivity be called separately
//    RestartActivity();

    return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActivityMan object.

void ActivityMan::Destroy(bool notInherited)
{
//    delete m_pStartActivity;
//    delete m_pActivity;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new activity for next restart, which can be customized and not
//                  recognized by PresetMan. You have to use RestartActivity to get it going.

void ActivityMan::SetStartActivity(Activity *pNewActivity)
{ 
    DAssert(pNewActivity, "Trying to replace an activity with a null one!");

    delete m_pStartActivity;
    m_pStartActivity = pNewActivity;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the activity passed in. Ownership IS transferred!

int ActivityMan::StartActivity(Activity *pActivity)
{
    DAssert(pActivity, "Trying to start a null activity!");

    int error = 0;

    // Stop all music, will be started by the Activity right below
    g_AudioMan.StopMusic();

    // Replace the start activity
    delete m_pStartActivity;
    m_pStartActivity = pActivity;

    // End and delete the old activity
// Don't end it, unnecessary causes endgame music playback problems
//    if (m_pActivity)
//        m_pActivity->End();
    delete m_pActivity;
    // Replace it with a clone of the start activity
    m_pActivity = dynamic_cast<Activity *>(m_pStartActivity->Clone());
    // Setup the players
    m_pActivity->SetupPlayers();
    // and START THAT BITCH
    error = m_pActivity->Start();

    if (error >= 0)
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was successfully started");
    else
    {
        g_ConsoleMan.PrintString("ERROR: Activity \"" + m_pActivity->GetPresetName() + "\" was NOT started due to errors!");
        m_pActivity->SetActivityState(Activity::INERROR);
        return error;
    }

    // Make sure the main menu and console exits and we're in the game when the activity starts
    g_ConsoleMan.SetEnabled(false);
    g_ResumeActivity = true;
    g_InActivity = true;

// Why was this being done?
//    g_MovableMan.Update();

    // Clear the post effects
    g_SceneMan.ClearPostEffects();

    // Clear the screen messages
    g_FrameMan.ClearScreenText();

    // Reset the mouse input to the center
    g_UInputMan.SetMouseValueMagnitude(0.05);
    
    // Reset the last music pos
    m_LastMusicPath = "";
    m_LastMusicPos = 0;

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially gets and starts the activity described.

int ActivityMan::StartActivity(string className, string instanceName)
{
    // Get the object instance first, make sure we can
    const Entity *pEntity = g_PresetMan.GetEntityPreset(className, instanceName);

    if (pEntity)
    {
        // Ok, then make a copy of it and get it going
        Activity *pNewActivity = dynamic_cast<Activity *>(pEntity->Clone());
        if (pNewActivity)
            return StartActivity(pNewActivity);
    }
    else
    {
        g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + instanceName + " to start! Has it been defined?");
        return -1;
    }
/*
    // Just load the default
    else
        StartActivity(m_DefaultActivityType, m_DefaultActivityName);
*/

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Completely restarts whatever activity was last started.

int ActivityMan::RestartActivity()
{
    // If we have a start activity set, then clone it and pass in. (have to clone, or will delete self in StartActivity)
    if (m_pStartActivity)
        return StartActivity(dynamic_cast<Activity *>(m_pStartActivity->Clone()));
    // Use the default activity instead
    else
        return StartActivity(m_DefaultActivityType, m_DefaultActivityName);

    // Report that we had to start the default because there wasn't a specified start activity
    return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void ActivityMan::PauseActivity(bool pause)
{
    if (!m_pActivity || (pause && m_pActivity->Paused()) || (!pause && !m_pActivity->Paused()))
        return;

    if (m_pActivity)
    {
        // Save the current in-game music position on pause
        if (pause)
        {
            m_LastMusicPath = g_AudioMan.GetMusicPath();
            m_LastMusicPos = g_AudioMan.GetMusicPosition();
        }
        // Re-start it again where it was on unpause
        else
        {
            // Only if we have a position to actually resume
            if (!m_LastMusicPath.empty() && m_LastMusicPos > 0)
            {
                g_AudioMan.ClearMusicQueue();
                g_AudioMan.PlayMusic(m_LastMusicPath.c_str());
                g_AudioMan.SetMusicPosition(m_LastMusicPos);
                g_AudioMan.QueueSilence(30);
                g_AudioMan.QueueMusicStream("Base.rte/Music/Watts/Last Man.ogg");
                g_AudioMan.QueueSilence(30);
                g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");
            }
        }

        m_pActivity->Pause(pause);
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was " + (pause ? "paused" : "unpaused"));
    }
    else
        g_ConsoleMan.PrintString("ERROR: No Activity to pause!");

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void ActivityMan::EndActivity()
{
    if (m_pActivity)
    {
        m_pActivity->End();
        g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_pActivity->GetPresetName() + "\" was ended");
    }
    else
        g_ConsoleMan.PrintString("ERROR: No Activity to end!");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this and the current Activity. Supposed to be
//                  done every frame before drawing.

void ActivityMan::Update()
{
    SLICK_PROFILE(0xFF879355);

    if (m_pActivity)
        m_pActivity->Update();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LateUpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only updates Global Scripts of the current activity with LateUpdate flag enabled

void ActivityMan::LateUpdateGlobalScripts()
{
	GAScripted * sa = dynamic_cast<GAScripted *>(m_pActivity);
	if (sa)
		sa->UpdateGlobalScripts(true);
}

} // namespace RTE
