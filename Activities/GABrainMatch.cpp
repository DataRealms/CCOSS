//////////////////////////////////////////////////////////////////////////////////////////
// File:            GABrainMatch.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GABrainMatch class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GABrainMatch.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
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

ConcreteClassInfo(GABrainMatch, GameActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GABrainMatch, effectively
//                  resetting the members of this abstraction level only.

void GABrainMatch::Clear()
{
    m_pCPUBrain = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GABrainMatch object ready for use.

int GABrainMatch::Create()
{
    if (GameActivity::Create() < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Each team starts by building a bunker and installs their respective brain. When everyone is done, the battle starts and ends only when one team with a survivng brain remains!";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GameActivity to be identical to another, by deep copy.

int GABrainMatch::Create(const GABrainMatch &reference)
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

int GABrainMatch::ReadProperty(const std::string_view &propName, Reader &reader)
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
        return GameActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GABrainMatch with a Writer for
//                  later recreation with Create(Reader &reader);

int GABrainMatch::Save(Writer &writer) const {
	GameActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GABrainMatch object.

void GABrainMatch::Destroy(bool notInherited)
{
    if (!notInherited)
        GameActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GABrainMatch::Start()
{
    int error = GameActivity::Start();

    ////////////////////////////////
    // Set up teams

    for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        // If this is a CPU controlled team, then try to find a brain for them, or place one
        if (team == m_CPUTeam && !m_pCPUBrain)
        {
// TODO: Make special CPU brain actor which will only appear in CPU brain fights, and have to be placed in the scenes
            if (!(m_pCPUBrain = g_MovableMan.GetUnassignedBrain(m_CPUTeam)))
            {
                // Couldn't find an available brain in the scene, so make one and place it
                m_pCPUBrain = dynamic_cast<Actor *>(g_PresetMan.GetEntityPreset("Actor", "Brain Case")->Clone());
                if (m_pCPUBrain)
                {
                    Vector brainPos;
                    // Place opposite of the other team's brain
                    Actor *pOtherBrain = g_MovableMan.GetFirstOtherBrainActor(team);
                    if (pOtherBrain)
                    {
                        brainPos = pOtherBrain->GetPos();
                        brainPos.m_X += g_SceneMan.GetSceneWidth() / 2;
                        brainPos.m_Y = g_SceneMan.GetSceneHeight() - 200;
                        g_SceneMan.WrapPosition(brainPos);
                    }
                    // No brain found on other team... then just place somewhere in the ground, spaced out
                    else
                    {
                        if (team == Teams::TeamOne)
                            brainPos.SetXY((float)g_SceneMan.GetSceneWidth() * 0.25, (float)g_SceneMan.GetSceneHeight() * 0.75);
                        if (team == Teams::TeamTwo)
                            brainPos.SetXY((float)g_SceneMan.GetSceneWidth() * 0.75, (float)g_SceneMan.GetSceneHeight() * 0.75);
                    }
                    m_pCPUBrain->SetPos(brainPos);
                    m_pCPUBrain->SetTeam(team);
                    // Transfer ownership here
                    g_MovableMan.AddActor(m_pCPUBrain);
                    pOtherBrain = 0;
                }
            }
        }
    }

    ///////////////////////////////////////
    // Set up players

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (!m_IsActive[player])
            continue;

        // Make sure all players have resident brains, or make them edit one in
        if (!g_SceneMan.GetScene()->GetResidentBrain(player))
        {
            m_ActivityState = ActivityState::Editing;
            const Entity *pBrainBunker = g_PresetMan.GetEntityPreset("TerrainObject", "Brain Vault");
            m_pEditorGUI[player]->SetCurrentObject(dynamic_cast<SceneObject *>(pBrainBunker->Clone()));
            m_pEditorGUI[player]->SetEditorGUIMode(SceneEditorGUI::INSTALLINGBRAIN);
            // Place team's cursors on opposite sides of the map initially
            m_pEditorGUI[player]->SetCursorPos(Vector((g_SceneMan.GetSceneWidth() / 4) * (m_Team[player] == 0 ? 1 : 3), g_SceneMan.GetSceneHeight() * 0.66));
        }

/* Obsolete with resident brains
        // Find and assign any available brains in the scene to players without them
        if (!m_Brain[player])
        {
            // If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
            if (!(m_Brain[player] = g_MovableMan.GetUnassignedBrain(m_Team[player])))
            {
                m_ActivityState = ActivityState::Editing;
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
                SwitchToActor(m_Brain[player], player, m_Team[player]);
                m_ActorCursor[player] = m_Brain[player]->GetPos();
                m_LandingZone[player].m_X = m_Brain[player]->GetPos().m_X;
                // Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
                m_ObservationTarget[player] = m_Brain[player]->GetPos();
            }
        }
*/
    }

    if (m_ActivityState == ActivityState::Editing)
    {
        // Play editing music
        g_AudioMan.ClearMusicQueue();
        g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg");
    }

    // Second pass after we have determined whether we need to be editing or not
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (!m_IsActive[player])
            continue;

        g_FrameMan.ClearScreenText(ScreenOfPlayer(player));
        if (m_ActivityState == ActivityState::Editing)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Place your brain vault and build your bunker around it..." : "...then select \"DONE\" from the pie menu!", ScreenOfPlayer(player), 0, 3000);
        else if (m_ActivityState == ActivityState::Running)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Mine Gold and buy more firepower with the funds..." : "...then smash the competing brain to claim victory!", ScreenOfPlayer(player), 0, 3000);
    }

    // Disable AI if we are editing
    DisableAIs(m_ActivityState == ActivityState::Editing);

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GABrainMatch. Supposed to be done every frame
//                  before drawing.

void GABrainMatch::Update()
{
    // Avoid game logic when we're editing
    if (m_ActivityState == ActivityState::Editing)
    {
        UpdateEditing();
        return;
    }

    GameActivity::Update();

    // Clear all objective markers, they get re-added each frame
    ClearObjectivePoints();

    ///////////////////////////////////////////
    // Iterate through all human players

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (!m_IsActive[player])
            continue;

        // The current player's team
        int team = m_Team[player];
        if (team == Teams::NoTeam)
            continue;

        // Make sure the game is not already ending
        if (m_ActivityState != ActivityState::Over)
        {
            // Check if any player's brain is dead
            if (!g_MovableMan.IsActor(m_Brain[player]) || !m_Brain[player]->HasObjectInGroup("Brains"))
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
                    m_MessageTimer[player].Reset();
                }
            }
            else
            {
                // Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
                SetObservationTarget(m_Brain[player]->GetPos(), player);
                // Mark each brain to be protected and destroyed by the respective teams
                AddObjectivePoint("Protect!", m_Brain[player]->GetAboveHUDPos(), team, GameActivity::ARROWDOWN);
                for (int t = Teams::TeamOne; t < Teams::MaxTeamCount; ++t)
                {
                    if (!m_TeamActive[team])
                        continue;
                    if (t != team)
                        AddObjectivePoint("Destroy!", m_Brain[player]->GetAboveHUDPos(), t, GameActivity::ARROWDOWN);
                }
            }
        }
        // Game over, show the appropriate messages until a certain time
        else if (!m_GameOverTimer.IsPastSimMS(m_GameOverPeriod))
        {
            if (m_Team[player] == m_WinnerTeam)
                g_FrameMan.SetScreenText("Your competition's wetware is mush!", ScreenOfPlayer(player));
            else
                g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player));
        }
    }


    ///////////////////////////////////////////
    // Iterate through all teams

    for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        ///////////////////////////////////////////
        // Check for victory conditions

        // Make sure the game is not already ending
        if (m_ActivityState != ActivityState::Over)
        {
            // Check if the CPU brain is dead, if we're playing against the CPU
            if (m_CPUTeam >= 0 && team == m_CPUTeam && !g_MovableMan.IsActor(m_pCPUBrain))
            {
                m_pCPUBrain = 0;
                // Proclaim player winner and end
                m_WinnerTeam = OtherTeam(m_CPUTeam);
                // Finito!
                End();
            }


/*
            // Check for bankruptcy
// TODO Don't hardcode the rocket cost!
            if (m_TeamFunds[team] < 0)//&& Only brain is left of actors)
            {
                for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
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
                    m_MessageTimer[player].Reset();
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

void GABrainMatch::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    GameActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GABrainMatch's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GABrainMatch::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    GameActivity::Draw(pTargetBitmap, targetPos);
}

} // namespace RTE