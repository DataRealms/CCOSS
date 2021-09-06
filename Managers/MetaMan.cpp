//////////////////////////////////////////////////////////////////////////////////////////
// File:            MetaMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MetaMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MetaMan.h"
#include "MetaSave.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

#include "MetagameGUI.h"
#include "Scene.h"
#include "SLTerrain.h"

namespace RTE {

const string MetaMan::c_ClassName = "MetaMan";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MetaMan, effectively
//                  resetting the members of this abstraction level only.

void MetaMan::Clear()
{
    m_pMetaGUI = 0;
    m_GameState = NOGAME;
    m_StateChanged = true;
    m_Suspended = false;
    m_GameSaved = false;
    m_GameName = DEFAULTGAMENAME;
    m_Players.clear();
    m_TeamCount = 0;
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        m_TeamIcons[team].Reset();
    m_CurrentRound = 0;
    m_Scenes.clear();
    m_RevealedScenes = 0;
    m_RevealRate = 0.5;
    m_RevealExtra = 3.0;
    m_RoundOffensives.clear();
    m_CurrentOffensive = 0;
    m_PhaseTimer.Reset();
	m_Difficulty = Activity::MediumDifficulty;

	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++)
		m_TeamAISkill[team] = Activity::DefaultSkill;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetaMan object ready for use.

int MetaMan::Initialize()
{
//    if (Serializable::Create() < 0)
//        return -1;

    // Allocate the metagame interface; it gets created manually later in Main
    if (!m_pMetaGUI)
        m_pMetaGUI = new MetagameGUI();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NewGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wipes any current and sets up a new game based on a size parameter.

int MetaMan::NewGame(int gameSize)
{
    // Grab a random selection of Scene presets from all available
    list<Scene *> scenePresets;
    SelectScenePresets(gameSize, &scenePresets);

    // Destroy and clear any pre-existing scenes from previous games
    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
        delete *sItr;
        *sItr = 0;
    }
    m_Scenes.clear();

    // Make deep copies of the selected Scene presets for use in the actual game about to start
    for (list<Scene *>::iterator pItr = scenePresets.begin(); pItr != scenePresets.end(); ++pItr)
    {
		// List for found metascenes to choose from
		vector<Scene *> metascenesList;
		// Add our actual scene to the list as it should always be there to be selected later
		metascenesList.push_back(*pItr);

		// Look for additional metascenes which can be used instead of this scene
		list<Entity *> sceneList;
		g_PresetMan.GetAllOfType(sceneList, "Scene");

		// Go through the list and add all compatible metascenes to the list
		for (list<Entity *>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr)
		{
			Scene * pScene = dynamic_cast<Scene *>(*itr);
			if (pScene)
			{
				if (pScene->GetMetasceneParent() == (*pItr)->GetModuleAndPresetName())
					metascenesList.push_back(pScene);
			}
		}
		// Finally select some random metascene
		int selection = RandomNum<int>(0, metascenesList.size() - 1);
		Scene * pSelectedScene = metascenesList.at(selection);

		//Copy selected scene
		m_Scenes.push_back(dynamic_cast<Scene *>(pSelectedScene->Clone()));
		// So we're messing with metascene, change it's name to conseal it's true nature
		if (selection > 0)
		{
			m_Scenes.back()->SetPresetName((*pItr)->GetPresetName());
			m_Scenes.back()->SetDescription((*pItr)->GetDescription());
			m_Scenes.back()->SetLocation((*pItr)->GetLocation());
			m_Scenes.back()->SetLocationOffset((*pItr)->GetLocationOffset());
			m_Scenes.back()->SetMetagameInternal(true);
		}
		// Add a metagame base area
		Scene::Area newArea(METABASE_AREA_NAME);
		newArea.AddBox(Box(0,0,1,1)); // Add an empty box, or the area won't be saved
        m_Scenes.back()->AddArea(newArea);


        // Make sure they are all hidden at game start
        m_Scenes.back()->SetRevealed(false);
        // Make them unique presets in their own Data Module so they don't get referenced from the original presets they were made from
        m_Scenes.back()->MigrateToModule(g_PresetMan.GetModuleID(METASAVEMODULENAME));
        // Make them unexplored by all teams
        for (int team = Activity::TeamOne; team < m_TeamCount; ++team)
            m_Scenes.back()->FillUnseenLayer(Vector(25, 25), team, false);

		//	Go through all AI plan elements and expand all bunker schemes to concrete assemblies 
		//	with fixed prices and place deployments
		m_Scenes.back()->ExpandAIPlanAssemblySchemes();
    }

    // The game that is currently being played is known as 
    m_GameName = DEFAULTGAMENAME;

    // Start the game/intro
    m_CurrentRound = 0;
    m_RevealedScenes = 0;
// TODO: Refine these, taking team numbers and total game size into account
    // Set the reveal rate of new sites each round
    m_RevealRate = 0.3;
    // Set the number of sites initially found at the start of the game
    m_RevealExtra = m_Players.size();
    m_GameState = GAMEINTRO;
    m_StateChanged = true;
    SetSuspend(false);

    ClearActivities();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EndGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wipes any current metagame and sets things back to as if program start.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

int MetaMan::EndGame()
{
	//Reset metagame UI
	m_pMetaGUI->SetToStartNewGame();

    // Destroy and clear any pre-existing scenes from previous games
    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
        delete *sItr;
        *sItr = 0;
    }
    m_Scenes.clear();

    // The game that is currently being played is known as     
    m_GameName = DEFAULTGAMENAME;

    m_GameState = NOGAME;
    m_StateChanged = true;
    SetSuspend(false);

	g_ActivityMan.EndActivity();

    g_MetaMan.m_Players.clear();

    ClearActivities();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Load
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Load a MetaMan from disk out of the special MetaMan.rte data module

int MetaMan::Load(const MetaSave *pSave)
{
    if (!pSave)
        return -1;

	// Reset Metaman's AI skill in case those won't be loaded from older saves
	for (int team = Activity::TeamOne ; team < Activity::MaxTeamCount; team++)
		m_TeamAISkill[team] = Activity::DefaultSkill;

    // Create the reader to read the metagame state from
    Reader reader(pSave->GetSavePath().c_str(), false, 0, false);
    if (!reader.ReaderOK())
        return -1;

    // Clear off players, scenes, and offensive activiies before filling up on new ones read from disk
    m_Players.clear();
    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
        delete (*sItr);
    m_Scenes.clear();
    for (vector<GAScripted *>::iterator aItr = m_RoundOffensives.begin(); aItr != m_RoundOffensives.end(); ++aItr)
        delete (*aItr);
    m_RoundOffensives.clear();

    // Now actually do the reading/loading from file
    Serializable::Create(reader, true, false);

    // Make sure all labels etc get set properly after load
    m_StateChanged = true;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.

int MetaMan::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "GameState")
        reader >> m_GameState;
    else if (propName == "GameName")
        reader >> m_GameName;
    else if (propName == "AddPlayer")
    {
        MetaPlayer player;
        reader >> player;
        m_Players.push_back(player);
    }
    else if (propName == "TeamCount")
        reader >> m_TeamCount;
    else if (propName == "Team1Icon")
        reader >> m_TeamIcons[Activity::TeamOne];
    else if (propName == "Team2Icon")
        reader >> m_TeamIcons[Activity::TeamTwo];
    else if (propName == "Team3Icon")
        reader >> m_TeamIcons[Activity::TeamThree];
    else if (propName == "Team4Icon")
        reader >> m_TeamIcons[Activity::TeamFour];
    else if (propName == "CurrentRound")
        reader >> m_CurrentRound;
    else if(propName == "AddScene")
    {
        Scene *pScene = new Scene;
        reader >> pScene;
        m_Scenes.push_back(pScene);
    }
    else if (propName == "RevealedScenes")
        reader >> m_RevealedScenes;
    else if (propName == "RevealRate")
        reader >> m_RevealRate;
    else if (propName == "RevealExtra")
        reader >> m_RevealExtra;
    else if(propName == "AddOffensive")
    {
        GAScripted *pOffensive = new GAScripted();
        reader >> pOffensive;
        m_RoundOffensives.push_back(pOffensive);
    }
    else if (propName == "CurrentOffensive")
        reader >> m_CurrentOffensive;
    else if (propName == "Difficulty")
        reader >> m_Difficulty;
    else if (propName == "Team1AISkill")
		reader >> m_TeamAISkill[Activity::TeamOne];
    else if (propName == "Team2AISkill")
		reader >> m_TeamAISkill[Activity::TeamTwo];
    else if (propName == "Team3AISkill")
		reader >> m_TeamAISkill[Activity::TeamThree];
    else if (propName == "Team4AISkill")
		reader >> m_TeamAISkill[Activity::TeamFour];
    else if (propName == "MetaGUI")
        reader >> m_pMetaGUI;
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MetaMan to an output stream for
//                  later recreation with Create(Reader &reader);

int MetaMan::Save(Writer &writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("GameState", m_GameState);
	writer.NewPropertyWithValue("GameName", m_GameName);
	writer.NewPropertyWithValue("Difficulty", m_Difficulty);
	writer.NewPropertyWithValue("Team1AISkill", m_TeamAISkill[Activity::TeamOne]);
	writer.NewPropertyWithValue("Team2AISkill", m_TeamAISkill[Activity::TeamTwo]);
	writer.NewPropertyWithValue("Team3AISkill", m_TeamAISkill[Activity::TeamThree]);
	writer.NewPropertyWithValue("Team4AISkill", m_TeamAISkill[Activity::TeamFour]);

	for (const MetaPlayer &metaPlayer : m_Players) {
		writer.NewPropertyWithValue("AddPlayer", metaPlayer);
	}

	writer.NewPropertyWithValue("TeamCount", m_TeamCount);

	if (m_TeamCount >= 1) {
		writer.NewProperty("Team1Icon");
		m_TeamIcons[Activity::TeamOne].SavePresetCopy(writer);
	}
	if (m_TeamCount >= 2) {
		writer.NewProperty("Team2Icon");
		m_TeamIcons[Activity::TeamTwo].SavePresetCopy(writer);
	}
	if (m_TeamCount >= 3) {
		writer.NewProperty("Team3Icon");
		m_TeamIcons[Activity::TeamThree].SavePresetCopy(writer);
	}
	if (m_TeamCount >= 4) {
		writer.NewProperty("Team4Icon");
		m_TeamIcons[Activity::TeamFour].SavePresetCopy(writer);
	}

	writer.NewPropertyWithValue("CurrentRound", m_CurrentRound);

	for (const Scene *metaScene : m_Scenes) {
		// Save the scene data to a good unique prefix for the Scene's layers' bitmap files as they are saved
		// This should be handled separately from any .ini writing
		//(*sItr)->SaveData(writer.GetFolderPath() + m_GameName + " - " + (*sItr)->GetPresetName());
		writer.NewPropertyWithValue("AddScene", metaScene);
	}

	writer.NewPropertyWithValue("RevealedScenes", m_RevealedScenes);
	writer.NewPropertyWithValue("RevealRate", m_RevealRate);
	writer.NewPropertyWithValue("RevealExtra", m_RevealExtra);

	for (const GAScripted *metaOffensive : m_RoundOffensives) {
		writer.NewPropertyWithValue("AddOffensive", metaOffensive);
	}

	writer.NewPropertyWithValue("CurrentOffensive", m_CurrentOffensive);
	writer.NewPropertyWithValue("MetaGUI", m_pMetaGUI);

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the bitmap data of all Scenes of this Metagame that are currently
//                  loaded.

int MetaMan::SaveSceneData(string pathBase)
{
    for (vector<Scene *>::const_iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
        // Only save the data of revealed scenes that have already had their layers built and saved into files
        if ((*sItr)->IsRevealed() && (*sItr)->GetTerrain() && (*sItr)->GetTerrain()->IsFileData())
        {
            // Save the scene data to a good unique prefix for the Scene's layers' bitmap files as they are saved
            if ((*sItr)->SaveData(pathBase + " - " + (*sItr)->GetPresetName()) < 0)
                return -1;
        }
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the bitmap data of all Scenes of this Metagame that have once
//                  been saved to files.

int MetaMan::LoadSceneData()
{
    for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        // Only load the data of revealed scenes that have already had their layers built and saved into files
        if ((*sItr)->IsRevealed() && (*sItr)->GetTerrain() && (*sItr)->GetTerrain()->IsFileData())
        {
            // Only load the scene layer data, don't place objects or do any init for actually playing the scene
            if ((*sItr)->LoadData(false, false) < 0)
                return -1;
        }
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the bitmap data of all Scenes of this Metagame that have once
//                  been saved to files.

int MetaMan::ClearSceneData()
{
    for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        if ((*sItr)->ClearData() < 0)
            return -1;
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MetaMan object.

void MetaMan::Destroy()
{
    delete m_pMetaGUI;

    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
        delete (*sItr);
    for (vector<GAScripted *>::iterator aItr = m_RoundOffensives.begin(); aItr != m_RoundOffensives.end(); ++aItr)
        delete (*aItr);

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerTurn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows which player's turn is now or coming up.

int MetaMan::GetPlayerTurn() const
{
    // Player 1's turn is coming up on this round
    if (g_MetaMan.m_GameState <= PLAYER1TURN)
        return Players::PlayerOne;
    // we're past the player turns on this round, so player 1 is up next again
    else if ((g_MetaMan.m_GameState - PLAYER1TURN) > (m_Players.size() - 1))
        return Players::PlayerOne;

    // Return whos player's turn it is
    return g_MetaMan.m_GameState - PLAYER1TURN;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMetaPlayerOfInGamePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MetaPlayer playing a specific in-game player, if any.

MetaPlayer * MetaMan::GetMetaPlayerOfInGamePlayer(int inGamePlayer)
{
    for (vector<MetaPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if ((*itr).GetInGamePlayer() == inGamePlayer)
            return &(*itr);
    }

    // Didn't find any metaplayer that is using that in-game player 
    return 0;    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextSceneOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Scene in play that is owned by a specific player.

const Scene * MetaMan::GetNextSceneOfPlayer(int player, const Scene *pStartScene) const
{
    if (m_Scenes.empty())
        return 0;

    const Scene *pFoundScene = 0;

    // If no valid start scene specified, just start search immediately
    bool foundStart = !(pStartScene && pStartScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(player));

    // Search for the next scene owned by the currently animated player
    int scenesSearched = 0;
    for (vector<Scene *>::const_iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        // Don't search beyond what has been revealed already
        if (scenesSearched >= std::floor(m_RevealedScenes))
            break;

        // Find the place where to start the actual search for the next owned Scene from
        if (!foundStart)
        {
            // Ok, found the start, now begin actual search
            if (*sItr == pStartScene)
                foundStart = true;
        }
        // Now find the actual next owned Scene of this player
        else if ((*sItr)->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(player))
        {
            pFoundScene = *sItr;
            break;
        }
        ++scenesSearched;
    }

    return pFoundScene;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalBrainCountOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of brains that a player has, including ones of
//                  his that are resident down on sites.

int MetaMan::GetTotalBrainCountOfPlayer(int metaPlayer, bool countPoolsOnly) const
{
    if (metaPlayer <= Players::NoPlayer || metaPlayer >= Players::MaxPlayerCount)
        return 0;

    // Count the pool first
    int brainCount = m_Players[metaPlayer].GetBrainPoolCount();
    // Plus any that are out travelling between sites
    brainCount += m_Players[metaPlayer].GetBrainsInTransit();

    if (!countPoolsOnly)
    {
        for (vector<Scene *>::const_iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
        {
            // Add up any brains installed as resident on any sites
            if ((*sItr)->IsRevealed() && (*sItr)->GetTeamOwnership() == GetTeamOfPlayer(metaPlayer) && (*sItr)->GetResidentBrain(m_Players[metaPlayer].GetInGamePlayer()))
                brainCount += 1;
        }
    }

    return brainCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total gold funds of all the players of a specific team combined.

int MetaMan::GetGoldCountOfTeam(int team) const
{
    if (team <= Activity::NoTeam || team >= m_TeamCount)
        return 0;

    float goldTotal = 0;
    // Go through all players and add up the funds of all who belong to this team
    for (int metaPlayer = Players::PlayerOne; metaPlayer < m_Players.size(); ++metaPlayer)
    {
        if (m_Players[metaPlayer].GetTeam() == team)
            goldTotal += m_Players[metaPlayer].GetFunds();
    }

    return goldTotal;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of bases that any specific team owns.

int MetaMan::GetSceneCountOfTeam(int team) const
{
    if (team <= Activity::NoTeam || team >= m_TeamCount)
        return 0;

    // Go through all scenes and add up all the ones owned by this team
    int sceneCount = 0;
    for (vector<Scene *>::const_iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
        // Add up any brains installed as resident on any sites
        if ((*sItr)->IsRevealed() && (*sItr)->GetTeamOwnership() == team)
            ++sceneCount;
    }

    return sceneCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalBrainCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of brains that a team has, including ones that
//                  are resident down on sites.

int MetaMan::GetTotalBrainCountOfTeam(int team, bool countPoolsOnly) const
{
    if (team <= Activity::NoTeam || team >= m_TeamCount)
        return 0;

    // Go through all players and add up the brains of the ones who are on this team 
    int brainCount = 0;

    for (int metaPlayer = Players::PlayerOne; metaPlayer < m_Players.size(); ++metaPlayer)
    {
        if (m_Players[metaPlayer].GetTeam() == team)
            brainCount += GetTotalBrainCountOfPlayer(metaPlayer, countPoolsOnly);
    }

    return brainCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnlyTeamWithAnyBrainPoolLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which team, if any, is the only one left with brains in its
//                  pool.

int MetaMan::OnlyTeamWithAnyBrainPoolLeft()
{
    // See if only one team remains with any brains
    int brainTeamCount = 0;
    int brainTeam = Activity::NoTeam;
    for (int t = Activity::TeamOne; t < m_TeamCount; ++t)
    {
        // Only count brains in pools; not resident ones also
        if (GetTotalBrainCountOfTeam(t, true) > 0)
        {
            brainTeamCount++;
            brainTeam = t;
        }
    }

    // If exactly one team with brains, return that
    if (brainTeamCount == 1)
        return brainTeam;

    // None OR more than two teams are left with brains!
    return Activity::NoTeam;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OneOrNoneTeamsLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether there is less than two teams left in this game with
//                  a brain in its ranks at all.

bool MetaMan::OneOrNoneTeamsLeft()
{
    // See if only one team remains with any brains
    int brainTeamCount = 0;
    int brainTeam = Activity::NoTeam;
    for (int t = Activity::TeamOne; t < m_TeamCount; ++t)
    {
        // Any brains left on this team? If so, they're a potential winner
        if (GetTotalBrainCountOfTeam(t) > 0)
        {
            brainTeamCount++;
            brainTeam = t;
        }
    }

    // If less than two teams left with any brains, they get indicated
    // Also, if NO teams with brain are left, that is indicated with NoTeam
    if (brainTeamCount <= 1)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which single team is left, if any.
// Arguments:       None.

int MetaMan::WhichTeamLeft()
{
    int whichTeam = Activity::NoTeam;

    // See if only one team remains with any brains
    int brainTeamCount = 0;
    int brainTeam = Activity::NoTeam;
    for (int t = Activity::TeamOne; t < m_TeamCount; ++t)
    {
        if (GetTotalBrainCountOfTeam(t) > 0)
        {
            brainTeamCount++;
            brainTeam = t;
        }
    }

    // If exactly one team with brains, return that
    if (brainTeamCount == 1)
        return brainTeam;

    // No team is left with brains!
    return Activity::NoTeam;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NoBrainsLeftInAnyPool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether there are no brains left in any active player's pool
//                  at all. This does NOT count deployed brain in bases.

bool MetaMan::NoBrainsLeftInAnyPool()
{
    // Go through all players and check each for any brains in any pool
    for (vector<MetaPlayer>::iterator mpItr = m_Players.begin(); mpItr != m_Players.end(); ++mpItr)
    {
        if ((*mpItr).GetBrainPoolCount() > 0)
            return false;
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamIsLeading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which single team has the most owned bases, and if there's a
//                  tie between two teams, total owned gold funds is used as a tiebreaker.

int MetaMan::WhichTeamIsLeading()
{
    int leaderTeam = Activity::NoTeam;
    bool tiedTeams[Activity::MaxTeamCount];
    for (int t = Activity::TeamOne; t < m_TeamCount; ++t)
        tiedTeams[t] = false;

    int baseCount = 0;
    // If we have a tie between two teams
    bool baseCountTie = false;
    // This is the record so far; negative so the first team with 0 won't detect as tied
    int highestBaseCount = -1;
    for (int team = Activity::TeamOne; team < m_TeamCount; ++team)
    {
        baseCount = GetSceneCountOfTeam(team);
        // We have a tie!
        if (baseCount == highestBaseCount)
        {
            // No leader - there's a tie
            leaderTeam = Activity::NoTeam;
            tiedTeams[team] = true;
            baseCountTie = true;  
        }
        // In the lead; clear all other tie flags
        if (baseCount > highestBaseCount)
        {
            // Leader!
            leaderTeam = team;
            highestBaseCount = baseCount;
            // No more tie
            for (int t = Activity::TeamOne; t < m_TeamCount; ++t)
                tiedTeams[t] = false;
            // This team is now tied with itself (ie not tied)
            tiedTeams[team] = true;
            // There's no tie as of now
            baseCountTie = false;
        }
    }

    // If we have a tie in base count; then break the tie by looking at total gold funds of all tied teams
    if (baseCountTie)
    {
        float highestGold = 0;
        // Go through all tied teams
        for (int team = Activity::TeamOne; team < m_TeamCount; ++team)
        {
            // One of the teams tied in bases
            if (tiedTeams[team])
            {
                if (GetGoldCountOfTeam(team) >= highestGold)
                {
                    // New leader!
                    highestGold = GetGoldCountOfTeam(team);
                    leaderTeam = team;
                }
            }
        }
    }

    // We have a winner!
    return leaderTeam;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneIncomeOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total income from all scenes owned by a specific player.

float MetaMan::GetSceneIncomeOfPlayer(int metaPlayer) const
{
    float totalIncome = 0;

    for (vector<Scene *>::const_iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        // Add up all the generated income for this player this round
        if ((*sItr)->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer))
            totalIncome += (*sItr)->GetRoundIncome();
    }
    return totalIncome;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBudgetedRatioOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio of funds already allocated to budgets of this player.

float MetaMan::GetBudgetedRatioOfPlayer(int metaPlayer, const Scene *pException, bool includeOffensive, bool includeDefensive) const
{
    float totalAllocated = 0;

    // Counting defensive allocations
    if (includeDefensive)
    {
        for (vector<Scene *>::const_iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            // Add up all the allocated funds so far this round, first of bases we're building
            if ((*sItr)->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer) && *sItr != pException)
                totalAllocated += (*sItr)->GetBuildBudget(m_Players[metaPlayer].GetInGamePlayer());
        }
    }

    // Also the money allocated for offensive action
    if (includeOffensive && !m_Players[metaPlayer].GetOffensiveTargetName().empty() && (!pException || (pException && pException->GetPresetName() != m_Players[metaPlayer].GetOffensiveTargetName())))
        totalAllocated += m_Players[metaPlayer].GetOffensiveBudget();

    return totalAllocated / m_Players[metaPlayer].GetFunds();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSuspend
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Suspends or unsuspends the game so exclusive GUIs and menus can be
//                  shown

void MetaMan::SetSuspend(bool suspend)
{
    if (suspend && !m_Suspended)
    {
        m_Suspended = true;
        m_GameSaved = false;
    }
    else if (!suspend && m_Suspended)
    {
        m_Suspended = false;
        m_GameSaved = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamOwnsAllSites
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether one team has ownership of all revealed sites.

int MetaMan::WhichTeamOwnsAllSites()
{
    int owner = Activity::NoTeam;
    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
        if ((*sItr)->IsRevealed())
        {
            // A site with no owner means that not all sites have been taken duh
            if ((*sItr)->GetTeamOwnership() == Activity::NoTeam)
            {
                owner = Activity::NoTeam;
                break;
            }

            // So the site is owned by someone, and that someone is the only encountered owner yet
            if (owner == Activity::NoTeam || (*sItr)->GetTeamOwnership() == owner)
                owner = (*sItr)->GetTeamOwnership();
            // We found two diff teams owning sites, so noone owns em all
            else
            {
                owner = Activity::NoTeam;
                break;                
            }
        }
    }
    return owner;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsGameOver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for game over condition

bool MetaMan::IsGameOver()
{
// This is the old condition of all sites being conquered
//    if (m_RevealedScenes >= m_Scenes.size() && WhichTeamOwnsAllSites() != Activity::NoTeam)
//        return true;

    // GAME IS OVER:
    // IF no players have any brains left in their respective pool, OR only one team does AND they are the leader in sites owned
    int onlyTeamLeft = OnlyTeamWithAnyBrainPoolLeft();
    if (NoBrainsLeftInAnyPool() || (onlyTeamLeft != Activity::NoTeam && WhichTeamIsLeading() == onlyTeamLeft))
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TotalScenePresets
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Yields a set of ALL eligible Scene presets for a new game.

int MetaMan::TotalScenePresets(std::list<Scene *> *pScenes)
{
    int totalCount = 0;
    // Get the list of ALL read-in Scene presets
    list<Entity *> allScenePresets;
    g_PresetMan.GetAllOfType(allScenePresets, "Scene");
    Scene *pScenePreset = 0;

    // Temporary list of planet locations already being used
    list<Vector> usedLocations;
    bool locationOK = true;

    if (pScenes)
        pScenes->clear();

    // Go through the preset list and count/copy over all eligible ones
    for (list<Entity *>::iterator sItr = allScenePresets.begin(); sItr != allScenePresets.end(); ++sItr)
    {
        pScenePreset = dynamic_cast<Scene *>(*sItr);
        // Filter out editor or special scenes, or ones that don't have locations defined.
        if (pScenePreset && !pScenePreset->GetLocation().IsZero() && pScenePreset->IsMetagamePlayable() && pScenePreset->GetMetasceneParent() == "")
        {
            // Make sure this exact site location on the planet isn't occupied already
            locationOK = true;
            for (list<Vector>::iterator vItr = usedLocations.begin(); vItr != usedLocations.end(); ++vItr)
            {
                if (pScenePreset->GetLocation() == *vItr)
                {
                    locationOK = false;
                    break;
                }
            }

            if (locationOK)
            {
                // Add this unique location to the list of locations that are now occupied
                usedLocations.push_back(pScenePreset->GetLocation());
                // Add to list if there is a list
                if (pScenes)
                    pScenes->push_back(pScenePreset);
                // Count the eligible scene
                totalCount++;
            }
        }
    }

    return totalCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectScenePresets
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Yields a set of randomly selected Scene presets for a new game.

int MetaMan::SelectScenePresets(int gameSize, list<Scene *> *pSelected)
{
    // Get the list of ALL eligible read-in Scene presets
    list<Scene *> scenePresets;
    TotalScenePresets(&scenePresets);

    // If we need to actually fill the list, do so
    if (pSelected)
    {
        // Go through the list and randomly knock out as many presets as necessary to reach the number we need for this game
		int randomIndex;
		int currentIndex;
        while (scenePresets.size() > gameSize)
        {
            // Randomly select one of the scenes and remove it
            currentIndex = 0;
			randomIndex = RandomNum<int>(0, scenePresets.size() - 1);
            for (list<Scene *>::iterator pItr = scenePresets.begin(); pItr != scenePresets.end(); ++pItr)
            {
                if (currentIndex == randomIndex)
                {
                    scenePresets.erase(pItr);
                    break;
                }
                currentIndex++;
            }
        }

        // Cast and copy (not deep!) to fill the provided list
        pSelected->clear();
		for (Scene *scenePointer : scenePresets) {
			pSelected->push_back(scenePointer);
		}
    }

    return gameSize;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearActivities
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the lined-up activities for the current round.

void MetaMan::ClearActivities()
{
    for (vector<GAScripted *>::iterator aItr = m_RoundOffensives.begin(); aItr != m_RoundOffensives.end(); ++aItr)
        delete (*aItr);
    m_RoundOffensives.clear();
    m_CurrentOffensive = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AIPlayerTurn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does all the things an AI player needs to do during his turn.

void MetaMan::AIPlayerTurn(int metaPlayer)
{
    if (metaPlayer < 0 || metaPlayer >= m_Players.size())
        return;

    MetaPlayer *pThisPlayer = &(m_Players[metaPlayer]);

    // If this player has no brains at all left, then do nothing
    if (GetTotalBrainCountOfPlayer(metaPlayer) <= 0)
    {
        pThisPlayer->SetGameOverRound(m_CurrentRound);
        pThisPlayer->SetOffensiveBudget(0);
        return;
    }

    // Tally up all the scenes according to who owns them
    int sceneCount = 0;
    int revealedScenes = std::floor(m_RevealedScenes);
    vector<Scene *> ownedScenes;
    vector<Scene *> enemyScenes;
    vector<Scene *> unclaimedScenes;

	float sceneMark = 0;
	Scene * pBestAttackCandidateScene = 0;

    for (vector<Scene *>::iterator sItr = m_Scenes.begin(); sItr != m_Scenes.end(); ++sItr)
    {
		float currentMark = 0;

        // We are only concerned with Scenes that are currently revealed and in play
        ++sceneCount;
        if (sceneCount > revealedScenes)
            break;

        // Scene is owned by this guy's team
        if ((*sItr)->GetTeamOwnership() == pThisPlayer->GetTeam())
            ownedScenes.push_back(*sItr);
        // Enemy-owned scene
        else if ((*sItr)->GetTeamOwnership() != Activity::NoTeam)
		{
            enemyScenes.push_back(*sItr);
			// Scenes with heavy investment owned by a team with lots of funds are less likely to attack
			currentMark = -((*sItr)->GetTotalInvestment() + GetGoldCountOfTeam((*sItr)->GetTeamOwnership()));
		}
        // Unoccupied scene
        else
		{
            unclaimedScenes.push_back(*sItr);
			// Unclaimed scenes are guaranteed to attack
			currentMark = 1000;
		}

		// Set new attack candidate if we have some better options to attack
		if (currentMark != 0 && currentMark > sceneMark)
		{
			sceneMark = currentMark;
			pBestAttackCandidateScene = (*sItr);
		}
    }

    // Decide how much of current budget to spend on offense vs defense, and also unassigned countering budget for when we are attacked at a base
    float counterRatio = 0.15;
    float offenseRatio = pThisPlayer->GetAggressiveness() * 0.8;
	float attackGoldThreshold = 250 + ownedScenes.size() * 250;

	if (!unclaimedScenes.empty())
		attackGoldThreshold = 250;

    // Special case: no brains left in pool to attack with
    if (pThisPlayer->GetBrainPoolCount() <= 0)
    {
        // Save for someone attacking back
        counterRatio = 0.5;
        // Nothing to attack with
        offenseRatio = 0;
    }
    // Special case: no owned bases
    else if (ownedScenes.empty())
    {
        // Don't save anything for counter or defenses if we don't have any bases!
        counterRatio = 0;
        // Also don't hold back anything for defense if we don't have any bases to spend on
        offenseRatio = 1.0;
		// Always attack now matter how low the funds are, there's no way to get them anyway
		attackGoldThreshold = -1;
    }

    // What remains is for defense
    float defenseRatio = 1.0 - offenseRatio - counterRatio;

    // Set the attack budget, if there's anything to attack, and anything to attack with
    if ((!enemyScenes.empty() || !unclaimedScenes.empty()) && offenseRatio > 0 && pThisPlayer->GetFunds() * offenseRatio >= attackGoldThreshold)
    {
        pThisPlayer->SetOffensiveBudget(pThisPlayer->GetFunds() * offenseRatio);
		// Use two methods to select which scene to attack, first one is based on the previously obtained scene mark and the second is mostly random
		if (RandomNum() < 0.6F && pBestAttackCandidateScene)
		{
			pThisPlayer->SetOffensiveTargetName(pBestAttackCandidateScene->GetPresetName());
		}
		else
		{
			// And the target scene, randomly selected for now from all unfriendly targets
			int unfriendlySceneCount = enemyScenes.size() + unclaimedScenes.size();
			int targetIndex = RandomNum(0, unfriendlySceneCount - 1);
			// Give it a strong preference for unclaimed scenes! They make more strategic sense than to attack a hardened target
			if (!unclaimedScenes.empty() && targetIndex >= unclaimedScenes.size())
				targetIndex = RandomNum() < 0.75F ? RandomNum<int>(0, unclaimedScenes.size() - 1) : targetIndex;
			// From index to actual Scene and selection
			Scene *selectedTarget = targetIndex < unclaimedScenes.size() ? unclaimedScenes[targetIndex] : enemyScenes[targetIndex - unclaimedScenes.size()];
			if (selectedTarget)
				pThisPlayer->SetOffensiveTargetName(selectedTarget->GetPresetName());
		}
    }
    // All on defense instead after all
    else
    {
        pThisPlayer->SetOffensiveBudget(0);
        defenseRatio = 1.0;
    }

    // Spread out the defensive budgets on the owned sites
    float totalDefenseBudget = pThisPlayer->GetFunds() * defenseRatio;
    int player = pThisPlayer->GetInGamePlayer();
    for (vector<Scene *>::iterator sItr = ownedScenes.begin(); sItr != ownedScenes.end(); ++sItr)
    {
        // Evenly, for now.. later, might want to prioritize sites with established bases, or opposite?
        (*sItr)->SetBuildBudget(player, totalDefenseBudget / ownedScenes.size());

        // Save the build budget ratio that was selected as well, so it can be re-used next round?
        // (AI doesn't really need to use this since they re-do their allocation algo above each round)
        if (pThisPlayer->GetFunds() > 0)
            (*sItr)->SetBuildBudgetRatio(player, (*sItr)->GetBuildBudget(player) / pThisPlayer->GetFunds());
        else
            (*sItr)->SetBuildBudgetRatio(player, 0);

        // Move building pieces from the Scene's AI blueprint queue to the actual blueprints, but only approximately as much as can afford, so the entire AI pre-built base plan isn't revealed
        (*sItr)->ApplyAIPlan(player);
    }

// TODO: Pay for and schedule to scan a random unfriendly site to keep things fair
    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this. Supposed to be done every frame before drawing.

void MetaMan::Update()
{
    m_pMetaGUI->Update();

    ////////////////////////////////////////////
    // METAGAME STATE MACHINE UPDATER

    // Game is temporarily suspended, don't do anything
    if (m_Suspended)
    {
        
    }
    // Game not started; the GUI will show the new game dialog
    else if (m_GameState == NOGAME)
    {
        // Show suspended so the new game dialog will show up
        m_Suspended = true;

        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            m_GameState = GAMEINTRO;
            m_StateChanged = true;
        }
    }

    // INTRO
    // Show some nice graphical intro of the campaign
    else if (m_GameState == GAMEINTRO)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body

        // State end
        if (1)//m_pMetaGUI->ContinuePhase())
        {
            m_GameState = NEWROUND;
            m_StateChanged = true;
        }
    }

    // NEW ROUND
    // Show a nice banner for the new round and its number
    else if (m_GameState == NEWROUND)
    {
        // State init
        if (m_StateChanged)
        {
            // New day; clear out old activities
            ClearActivities();
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body


        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            m_GameState = REVEALSCENES;
            m_StateChanged = true;
        }
    }

    // REVEAL SCENES
    // If not all the sites are revealed yet, reveal one or two (depending on how many players playing?)
    else if (m_GameState == REVEALSCENES)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body

        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            m_GameState = COUNTINCOME;
            m_StateChanged = true;
        }
    }

    // COUNT INCOME
    // Count all the income from all owned bases, add to respective players' funds:
    // Show this with lines from the bases adding to the floating player counters, showing income ratios, and leave the lines there to show base ownership and value during turns
    else if (m_GameState == COUNTINCOME)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body
        

        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            m_GameState = PLAYER1TURN;
            m_StateChanged = true;
        }
    }

    // PLAYER TURNS
    // Player-controlled player turn sequence:
    //     Show planet and current known sites, allow inspection of each
    //     Allow to zoom in on each site owned by this player:
    //         Build blueprints (allocating defense budget) in Scene Editor Activity until player done
    //     Allow clicking on sites unvisited by anyone
    //         Allocate expedition budget to this site with a slider
    //     Allow clicking on sites already claimed by other player
    //         Allocate attack budget to this site with a slider
    //     [Question: allow more than one expedition/offense action per turn?? - A: no]
    //     Wait til player hits [End Turn] button
    // AI-controlled player turn sequence:
    //     Determine offense/defense ratio of funds based on AI temperament/aggressiveness setting
    //     Select expansion site(s??), allocate the offense funds if we decide to go with more than one expansion/round
    //     Build as much as can afford on owned sites, based on the pre-defined base templates of the Scenes
    else if (m_GameState >= PLAYER1TURN && m_GameState <= PLAYER4TURN)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        int metaPlayer = m_GameState - PLAYER1TURN;

        // If an AI player, do the AI player's logic now and go to next player immediately afterward
        if (!m_Players[metaPlayer].IsHuman())
            AIPlayerTurn(metaPlayer);

        // State end - skip A.I. metaplayer turns in the GUI; also skip human player who have been knocked out of the game in previous rounds
        if (m_pMetaGUI->ContinuePhase() || !m_Players[metaPlayer].IsHuman() || m_Players[metaPlayer].IsGameOverByRound(m_CurrentRound))
        {
            // If this player is now done for, mark him as such so he'll be completely skipped in future rounds
            if (GetTotalBrainCountOfPlayer(metaPlayer) <= 0 && !m_Players[metaPlayer].IsGameOverByRound(m_CurrentRound))
                m_Players[metaPlayer].SetGameOverRound(m_CurrentRound);

            // Find the next player which is not out of the game yet
            do
            {
                // Next player, if any left
                m_GameState++;
                metaPlayer = m_GameState - PLAYER1TURN;
            }
            while (m_GameState <= PLAYER4TURN && metaPlayer < m_Players.size() && m_Players[metaPlayer].IsGameOverByRound(m_CurrentRound));

            // If not, jump to building bases
            if (m_GameState > PLAYER4TURN || metaPlayer >= m_Players.size())
                m_GameState = BUILDBASES;

            m_StateChanged = true;
        }
    }

    // BUILD BASES
    // Apply all blueprint pieces to all claimed/base sites (or just save them in list and apply at next load of Scene?)
    // In sequence, graphically show all defense investments of each player with lines from the floating funds meters to the bases spent on
    else if (m_GameState == BUILDBASES)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body

        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            m_GameState = RUNACTIVITIES;
            m_StateChanged = true;
        }
    }

    // RUN ACTIVITIES
    // Generate and go through list of all Activities caused by player offensive moves during their turns:
    //     Init the Activity with the corresponding Scene and player setups as dictated by the round's situation
    //     LOAD the Scene from disk, from the current metagame folder where all terrain damage etc is preserved
    //     Let the Activity play out til the end
    //     Show some stats of the outcome of the Activity
    //     Have the outcome reflected in the Metagame: Funds won/lost, site ownership change, etc
    //     SAVE the Scene to disk, preserving the Terrain
    else if (m_GameState == RUNACTIVITIES)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body

        // State end, either as signaled by the GUI, or by the fact that we are out of things to run
        if (m_pMetaGUI->ContinuePhase() || m_CurrentOffensive >= m_RoundOffensives.size())
        {
            m_GameState = ENDROUND;
            m_StateChanged = true;
        }
    }

    // END ROUND
    // If all sites of this metagame have been revealed, check if any player owns all the of them now - if so, GAME OVER
    else if (m_GameState == ENDROUND)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;

            // Check for GAME OVER condition
            if (IsGameOver())
            {
                m_GameState = GAMEOVER;
                m_StateChanged = true;
            }
        }

        // State body

        // Game isn't over yet, so start the next round when user is done looking at stats
        if (m_pMetaGUI->ContinuePhase())
        {
            m_CurrentRound++;
            m_GameState = NEWROUND;
            m_StateChanged = true;
        }
    }

    // GAME OVER
    // Show some game over graphical thing and perhaps stats
    else if (m_GameState == GAMEOVER)
    {
        // State init
        if (m_StateChanged)
        {
            m_PhaseTimer.Reset();
            m_StateChanged = false;
        }

        // State body
/* finito!
        // State end
        if (m_pMetaGUI->ContinuePhase())
        {
            // Next player
            m_GameState = GAMEOVER;
            m_StateChanged = true;
        }
*/
    }

    // Whoops, state is f'd up, restart the last round
    else
    {
        RTEAbort("Metagame State is out of bounds!?");
        m_GameState = REVEALSCENES;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MetaMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void MetaMan::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
/*
    GUIFont *pLargeFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();
    AllegroBitmap pBitmapInt(pTargetBitmap);

    // Iterate through all players
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        ;
    }
*/

    m_pMetaGUI->Draw(pTargetBitmap);
}
} // namespace RTE