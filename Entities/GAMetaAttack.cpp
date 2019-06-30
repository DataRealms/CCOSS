//////////////////////////////////////////////////////////////////////////////////////////
// File:            GAMetaAttack.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GAMetaAttack class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GAMetaAttack.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
//#include "AHuman.h"
//#include "MOPixel.h"
//#include "SLTerrain.h"
#include "Controller.h"
//#include "AtomGroup.h"
#include "Scene.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"
/*
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
*/
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

extern bool g_ResetActivity;

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(GAMetaAttack, GameActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GAMetaAttack, effectively
//                  resetting the members of this abstraction level only.

void GAMetaAttack::Clear()
{

/*
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        ;
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GAMetaAttack object ready for use.

int GAMetaAttack::Create()
{
    if (GameActivity::Create() < 0)
        return -1;

    m_Description = "A battle for the Metagame where more than one team fights over a site. The last team with a surviving brain in the Scene wins!";
/*
    ////////////////////////////////
    // Set up teams

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        ;
    }

    ///////////////////////////////////////
    // Set up players

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GameActivity to be identical to another, by deep copy.

int GAMetaAttack::Create(const GAMetaAttack &reference)
{
    if (GameActivity::Create(reference) < 0)
        return -1;

// Need to do deep copy, if anything, but not owned so NA
//    m_pCPUBrain = reference.m_pCPUBrain;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int GAMetaAttack::ReadProperty(std::string propName, Reader &reader)
{
/*
    if (propName == "CPUTeam")
        reader >> m_CPUTeam;
    else if (propName == "Difficulty")
        reader >> m_Difficulty;
    else if (propName == "DeliveryDelay")
        reader >> m_DeliveryDelay;
    else
*/
        // See if the base class(es) can find a match instead
        return GameActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GAMetaAttack with a Writer for
//                  later recreation with Create(Reader &reader);

int GAMetaAttack::Save(Writer &writer) const
{
    GameActivity::Save(writer);
/*
    writer.NewProperty("CPUTeam");
    writer << m_CPUTeam;
    writer.NewProperty("Difficulty");
    writer << m_Difficulty;
    writer.NewProperty("DeliveryDelay");
    writer << m_DeliveryDelay;
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GAMetaAttack object.

void GAMetaAttack::Destroy(bool notInherited)
{
/*
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        ;
    }

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        ;
    }
*/
    if (!notInherited)
        GameActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GAMetaAttack::Start()
{
    int error = GameActivity::Start();

    ////////////////////////////////
    // Set up teams

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;


    }

    ///////////////////////////////////////
    // Set up players

    for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player)
    {
        if (!m_IsActive[player])
            continue;

        // Make sure all players have resident brains, or make them edit one in
        if (!g_SceneMan.GetScene()->GetResidentBrain(player))
        {
            m_ActivityState = EDITING;
            const Entity *pBrainBunker = g_PresetMan.GetEntityPreset("TerrainObject", "Brain Vault");
            m_pEditorGUI[player]->SetCurrentObject(dynamic_cast<SceneObject *>(pBrainBunker->Clone()));
            m_pEditorGUI[player]->SetEditorGUIMode(SceneEditorGUI::INSTALLINGBRAIN);
            // Place team's cursors on opposite sides of the map initially
            m_pEditorGUI[player]->SetCursorPos(Vector((g_SceneMan.GetSceneWidth() / 4) * (m_Team[player] == 0 ? 1 : 3), g_SceneMan.GetSceneHeight() * 0.66));
        }

/* Obsolete with resident brains
        // Find and assign any available brains in the scene to players without them
        if (!m_pBrain[player])
        {
            // If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
            if (!(m_pBrain[player] = g_MovableMan.GetUnassignedBrain(m_Team[player])))
            {
                m_ActivityState = EDITING;
                // Play editing music
                g_AudioMan.ClearMusicQueue();
                g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg");
                const Entity *pBrainBunker = g_PresetMan.GetEntityPreset("TerrainObject", "Brain Vault");
                m_pEditorGUI[player]->SetCurrentObject(dynamic_cast<SceneObject *>(pBrainBunker->Clone()));
                m_pEditorGUI[player]->SetEditorGUIMode(SceneEditorGUI::ADDINGOBJECT);
                // Place team's cursors on opposite sides of the map initially
                m_pEditorGUI[player]->SetCursorPos(Vector((g_SceneMan.GetSceneWidth() / 4) * (m_Team[player] == 0 ? 1 : 3), g_SceneMan.GetSceneHeight() * 0.66));
            }
            // Set the found brain to be the selected actor at start
            else
            {
                SwitchToActor(m_pBrain[player], player, m_Team[player]);
                m_ActorCursor[player] = m_pBrain[player]->GetPos();
                m_LandingZone[player].m_X = m_pBrain[player]->GetPos().m_X;
                // Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
                m_ObservationTarget[player] = m_pBrain[player]->GetPos();
            }
        }
*/
    }

    if (m_ActivityState == EDITING)
    {
        // Play editing music
        g_AudioMan.ClearMusicQueue();
        g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg");
    }

    // Second pass after we have determined whether we need to be editing or not
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (!(m_IsActive[player] && m_IsHuman[player]))
            continue;

        g_FrameMan.ClearScreenText(ScreenOfPlayer(player));
        if (m_ActivityState == EDITING)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Place your brain vault and build your bunker around it..." : "...then select \"DONE\" from the pie menu!", ScreenOfPlayer(player), 0, 3000);
        else if (m_ActivityState == RUNNING)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Mine Gold and buy more firepower with the funds..." : "...then smash the competing brain to claim victory!", ScreenOfPlayer(player), 0, 3000);
    }

    // Disable AI if we are editing
    DisableAIs(m_ActivityState == EDITING);

    // Move any brains resident in the Scene to the MovableMan
    g_SceneMan.GetScene()->PlaceResidentBrains(*this);

    return error;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GAMetaAttack::Pause(bool pause)
{
    m_Paused = pause;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GAMetaAttack::End()
{
    GameActivity::End();

    // Show appropriate end game messages
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (!(m_IsActive[player] && m_IsHuman[player]))
            continue;

        // Winners
        if (m_Team[player] == m_WinnerTeam)
        {
            // Owned the base at start
            if (g_SceneMan.GetScene()->GetTeamOwnership() == m_Team[player])
                g_FrameMan.SetScreenText("Your team has successfully defended this site!", ScreenOfPlayer(player));
            else
                g_FrameMan.SetScreenText("Your team has successfully taken over this site!", ScreenOfPlayer(player));
        }
        // Losers
        else
        {
            if (g_SceneMan.GetScene()->GetTeamOwnership() == m_Team[player])
                g_FrameMan.SetScreenText("Your team's brains have been destroyed and therefore lost this site!", ScreenOfPlayer(player));
            else
                g_FrameMan.SetScreenText("Your attempt to take over this site has failed!", ScreenOfPlayer(player));
        }

        m_MsgTimer[player].Reset();
    }

    // Take away ownership if there is no victor, and there are no brains left in the scene
    if (m_WinnerTeam == Activity::NOTEAM)
    {
        bool noBrainsLeft = true;
        for (int player = 0; player < Activity::MAXPLAYERCOUNT; ++player)
            noBrainsLeft = noBrainsLeft && g_SceneMan.GetScene()->GetResidentBrain(player);
        if (noBrainsLeft)
            g_SceneMan.GetScene()->SetTeamOwnership(Activity::NOTEAM);
    }
    // Give the victorious team ownership of the played scene
    else
        g_SceneMan.GetScene()->SetTeamOwnership(m_WinnerTeam);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.

void GAMetaAttack::UpdateEditing()
{



}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GAMetaAttack. Supposed to be done every frame
//                  before drawing.

void GAMetaAttack::Update()
{
    // Avoid game logic when we're editing
    if (m_ActivityState == EDITING)
    {
        UpdateEditing();
        return;
    }

    GameActivity::Update();

    // Clear all objective markers, they get re-added each frame
    ClearObjectivePoints();

    ///////////////////////////////////////////
    // Iterate through all human players

    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (!m_IsActive[player])
            continue;

        // The current player's team
        int team = m_Team[player];
        if (team == NOTEAM)
            continue;

        // If demo end, don't do anything
        if (m_ActivityState == DEMOEND)
        {
            m_pBrain[player] = 0;
            g_FrameMan.SetScreenText("Demo time for this skirmish is up! Register for unlimited battles!\nPress Ctrl + [R] to restart", ScreenOfPlayer(player));
        }
        // END GAME CHECKS
        // Make sure the game is not already ending
        else if (m_ActivityState != OVER)
        {
            // Check if any player's brain is dead
            if (!g_MovableMan.IsActor(m_pBrain[player]) || !m_pBrain[player]->HasObjectInGroup("Brains"))
            {
                SetPlayerBrain(0, player);
                // Try to find a new unasigned brain this player can use instead, or if his old brain entered a craft
                Actor *pNewBrain = g_MovableMan.GetUnassignedBrain(team);
                // Found new brain actor, assign it and keep on truckin'
                if (pNewBrain)
                {
                    SetPlayerBrain(pNewBrain, player);
                    SwitchToActor(pNewBrain, player, team);
                }
                else
                {
                    g_FrameMan.SetScreenText("Your brain has been lost!", ScreenOfPlayer(player), 333);
                    // Now see if all brains are dead of this player's team, and if so, check if there's only one team left with brains
                    if (!g_MovableMan.GetFirstBrainActor(team))
                    {
                        // If only one team left with any brains, they are the winners!
                        if (OneOrNoneTeamsLeft())
                        {
                            m_WinnerTeam = WhichTeamLeft();
                            End();
                        }
                    }
                    m_MsgTimer[player].Reset();
                }
            }
            else
            {
                // Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
                SetObservationTarget(m_pBrain[player]->GetPos(), player);
                // Mark each brain to be protected and destroyed by the respective teams
                AddObjectivePoint("Protect!", m_pBrain[player]->GetAboveHUDPos(), team, GameActivity::ARROWDOWN);
                for (int t = Activity::TEAM_1; t < MAXTEAMCOUNT; ++t)
                {
                    if (!m_TeamActive[t])
                        continue;
                    if (t != team)
                        AddObjectivePoint("Destroy!", m_pBrain[player]->GetAboveHUDPos(), t, GameActivity::ARROWDOWN);
                }
            }

            // If there's only one active team, it means they are clearing out wildlife in this mission
            if (GetTeamCount() == 1)
            {
                // Check if there's any wildlife left to clear out
                float distance = 0;
                if (!g_MovableMan.GetClosestTeamActor(Activity::NOTEAM, Activity::NOPLAYER, m_ActorCursor[player], g_SceneMan.GetSceneWidth(), distance))
                {
                    m_WinnerTeam = m_Team[player];
                    End();
                }
            }
        }
        // Game over, show the appropriate messages until a certain time
        else if (!m_GameOverTimer.IsPastSimMS(m_GameOverPeriod))
        {
/* These are shown in End()
            if (m_Team[player] == m_WinnerTeam)
                g_FrameMan.SetScreenText("Your competition's wetware is mush!", ScreenOfPlayer(player));
            else
                g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player));
*/
        }

    }


    ///////////////////////////////////////////
    // Iterate through all teams

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        ///////////////////////////////////////////
        // Check for victory conditions

        // Make sure the game is not already ending
        if (m_ActivityState != OVER)
        {
            
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void GAMetaAttack::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    GameActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GAMetaAttack's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GAMetaAttack::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    GameActivity::Draw(pTargetBitmap, targetPos);
}

} // namespace RTE