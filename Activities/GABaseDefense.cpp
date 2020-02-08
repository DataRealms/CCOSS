//////////////////////////////////////////////////////////////////////////////////////////
// File:            GABaseDefense.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GABaseDefense class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GABaseDefense.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
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

namespace RTE {

CONCRETECLASSINFO(GABaseDefense, GameActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GABaseDefense, effectively
//                  resetting the members of this abstraction level only.

void GABaseDefense::Clear()
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
    m_SpawnTimer.Reset();
    m_SpawnInterval = 20000;
    m_SpawnIntervalEasiest = 20000;
    m_SpawnIntervalHardest = 8000;
    m_AttackerSpawns.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GABaseDefense object ready for use.

int GABaseDefense::Create()
{
    if (GameActivity::Create() < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Defend your base in an AI onslaught.";
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
// Description:     Creates a GABaseDefense to be identical to another, by deep copy.

int GABaseDefense::Create(const GABaseDefense &reference)
{
    if (GameActivity::Create(reference) < 0)
        return -1;

    m_SpawnInterval = reference.m_SpawnInterval;
    m_SpawnIntervalEasiest = reference.m_SpawnIntervalEasiest;
    m_SpawnIntervalHardest = reference.m_SpawnIntervalHardest;

    Actor *pSpawn = 0;
    for (vector<Actor *>::const_iterator itr = reference.m_AttackerSpawns.begin(); itr != reference.m_AttackerSpawns.end(); ++itr)
    {
        pSpawn = dynamic_cast<Actor *>((*itr)->Clone());
        if (pSpawn)
            m_AttackerSpawns.push_back(pSpawn);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int GABaseDefense::ReadProperty(std::string propName, Reader &reader)
{

    if (propName == "SpawnIntervalEasiest")
        reader >> m_SpawnIntervalEasiest;
    else if (propName == "SpawnIntervalHardest")
        reader >> m_SpawnIntervalHardest;
    else if (propName == "AddAttackerSpawn")
    {
        Actor *pNewSpawn = dynamic_cast<Actor *>(g_PresetMan.ReadReflectedPreset(reader));
        if (pNewSpawn)
            m_AttackerSpawns.push_back(pNewSpawn);
    }
    else

        // See if the base class(es) can find a match instead
        return GameActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GABaseDefense with a Writer for
//                  later recreation with Create(Reader &reader);

int GABaseDefense::Save(Writer &writer) const
{
    GameActivity::Save(writer);

    writer.NewProperty("SpawnIntervalEasiest");
    writer << m_SpawnIntervalEasiest;
    writer.NewProperty("SpawnIntervalHardest");
    writer << m_SpawnIntervalHardest;
    for (vector<Actor *>::const_iterator itr = m_AttackerSpawns.begin(); itr != m_AttackerSpawns.end(); ++itr)
    {
        writer.NewProperty("AddAttackerSpawn");
        writer << (*itr);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GABaseDefense object.

void GABaseDefense::Destroy(bool notInherited)
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

    for (vector<Actor *>::const_iterator itr = m_AttackerSpawns.begin(); itr != m_AttackerSpawns.end(); ++itr)
    {
        delete (*itr);
    }

    if (!notInherited)
        GameActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GABaseDefense::Start()
{
    int error = GameActivity::Start();
    
    ////////////////////////////////
    // Set up teams

    /* CPU attacker doesn't need a brain
    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;
    }
    */

    ///////////////////////////////////////
    // Set up players

    for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player)
    {
        if (!(m_IsActive[player] && m_IsHuman[player]))
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
        // Only applies to human players
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

    // Set the spawn intervals based ont he min-max and the currently difficulty
    m_SpawnInterval = floorf(LERP(MINDIFFICULTY, MAXDIFFICULTY, m_SpawnIntervalEasiest, m_SpawnIntervalHardest, m_Difficulty));

    m_SpawnTimer.Reset();

    if (m_Difficulty == TESTDIFFICULTY)
    {
//        m_SpawnInterval = 200000;
//        m_SpawnTimer.SetElapsedSimTimeMS(210000);
        m_SpawnInterval = 25000;
    }

    return error;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GABaseDefense::Pause(bool pause)
{
    m_Paused = pause;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GABaseDefense::End()
{
// TODO: Update these messages to reflect attack budget running out!

    // Show appropriate end game messages
    for (int player = 0; player < MAXPLAYERCOUNT; ++player)
    {
        if (!m_IsActive[player])
            continue;

        if (m_Team[player] == m_WinnerTeam)
            g_FrameMan.SetScreenText("Your competition's wetware is mush!", ScreenOfPlayer(player));
        else
            g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player));

        m_MsgTimer[player].Reset();
    }

    GameActivity::End();
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.

void GABaseDefense::UpdateEditing()
{
    GameActivity::UpdateEditing();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GABaseDefense. Supposed to be done every frame
//                  before drawing.

void GABaseDefense::Update()
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

        // Make sure the game is not already ending
        if (m_ActivityState != OVER)
        {
            // Check if any player's brain is dead
            if (!g_MovableMan.IsActor(m_pBrain[player]) || !m_pBrain[player]->HasObjectInGroup("Brains"))
            {
                SetPlayerBrain(0, player);
                // Try to find a new unassigned brain this player can use instead, or if his old brain entered a craft
                Actor *pNewBrain = g_MovableMan.GetUnassignedBrain(team);
                // Found new brain actor, assign it and keep on truckin'
                if (pNewBrain)
                {
                    SetPlayerBrain(pNewBrain, player);
                    SwitchToActor(pNewBrain, player, team);
                }
                else
                {
                    g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player), 333);
                    // Now see if all brains are dead of this player's team, and if so, check if there's only one team left with brains
                    if (!g_MovableMan.GetFirstBrainActor(team))
                    {
/* This is effectively a "suvival" activity right now.. only one team with brains against an endless AI onslaught
                        // If only one team left with any brains, they are the winners!
                        if (OneOrNoneTeamsLeft())
                        {
                            m_WinnerTeam = WhichTeamLeft();
                            End();
                        }
*/
                        m_WinnerTeam = OtherTeam(team);
                        End();
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
                    if (!m_TeamActive[team])
                        continue;
                    if (t != team)
                        AddObjectivePoint("Destroy!", m_pBrain[player]->GetAboveHUDPos(), t, GameActivity::ARROWDOWN);
                }
            }
        }
        // Game over, show the appropriate messages until a certain time
        else if (!m_GameOverTimer.IsPastSimMS(m_GameOverPeriod))
        {
// TODO: make more appropriate messages here for run out of funds endings
            if (m_Team[player] == m_WinnerTeam)
                g_FrameMan.SetScreenText("Your competition's wetware is mush!", ScreenOfPlayer(player));
            else
                g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player));
        }
    }

    ///////////////////////////////////////////
    // Iterate through all teams

    for (int team = 0; team < MAXTEAMCOUNT; ++team)
    {
        if (!m_TeamActive[team])
            continue;

        /////////////////////////////////////////
        // Attacking CPU team logic

        if (team == m_CPUTeam)
        {
            // Spawn the CPU team's attacking forces
            if (m_SpawnTimer.IsPastSimMS(m_SpawnInterval) && m_ActivityState != OVER)
            {
                if (!m_AttackerSpawns.empty())
                {
                    int whichSpawn = floorf(m_AttackerSpawns.size() * PosRand());
                    Actor *pSpawn = dynamic_cast<Actor *>(m_AttackerSpawns[whichSpawn]->Clone());
                    if (pSpawn)
                    {
                        Vector landingZone;
                        Actor *pEnemyBrain = g_MovableMan.GetFirstOtherBrainActor(team);

                        // Don't land right on top of player team's base
                        if (pEnemyBrain)
                        {
                            // Get player team's base pos
                            landingZone = pEnemyBrain->GetPos();
                            // Get the opposite side
                            landingZone.m_X += g_SceneMan.GetSceneWidth() / 2;
                            // Now give the zone width
                            landingZone.m_X += (g_SceneMan.GetSceneWidth() / 2) * 0.75 * NormalRand();
                            // Wrap
                            g_SceneMan.WrapPosition(landingZone);
                        }
                        else
                        {
                            landingZone.m_X = g_SceneMan.GetSceneWidth() * PosRand();
                        }
                        Vector dropStart(landingZone.m_X, -50);
                        pSpawn->SetPos(dropStart);
                        pSpawn->SetTeam(team);
                        pSpawn->SetControllerMode(Controller::CIM_AI);
                        // Let the spawn into the world, passing ownership
                        g_MovableMan.AddActor(pSpawn);
                        pSpawn = 0;
                    }
                }
                m_SpawnTimer.Reset();
            }
        }

        ///////////////////////////////////////////
        // Check for victory conditions

        // Make sure the game is not already ending
        if (m_ActivityState != OVER && team != m_CPUTeam)
        {
// TODO: Gotto have budget restrictions in this activity!
/*
            // Check for bankruptcy
// TODO Don't hardcode the rocket cost!
            if (m_TeamFunds[team] < 0)//&& Only brain is left of actors)
            {
                for (int player = 0; player < MAXPLAYERCOUNT; ++player)
                {
                    if (!m_IsActive[player])
                        continue;

                    if (m_Team[player] == team)
                        g_FrameMan.SetScreenText("Your team can't afford any more transportation!", ScreenOfPlayer(player));
                    else
                    {
                        g_FrameMan.SetScreenText("Your competition is bankrupt!", ScreenOfPlayer(player));
                        m_WinnerTeam = m_Team[player];
                    }
                    m_MsgTimer[player].Reset();
                }
                End();
            }
*/
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void GABaseDefense::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    GameActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GABaseDefense's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GABaseDefense::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    GameActivity::Draw(pTargetBitmap, targetPos);
}

} // namespace RTE