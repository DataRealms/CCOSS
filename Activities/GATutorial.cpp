//////////////////////////////////////////////////////////////////////////////////////////
// File:            GATutorial.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GATutorial class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GATutorial.h"
#include "SceneMan.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "AudioMan.h"
#include "AHuman.h"
#include "ACrab.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Scene.h"
#include "Actor.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

extern bool g_ResetActivity;
extern bool g_InActivity;

#define MAPNAME(element) g_UInputMan.GetMappingName(m_TutorialPlayer, element)

namespace RTE {

ConcreteClassInfo(GATutorial, GameActivity, 0)


GATutorial::TutStep::TutStep(string text, int stepDuration, string screensPath, int frameCount, int frameDuration)
{
    m_Text = text;
    m_Duration = stepDuration;
    m_FrameDuration = frameDuration;

    if (!screensPath.empty())
    {
        ContentFile screenFile(screensPath.c_str());
        BITMAP **apScreens = screenFile.GetAsAnimation(frameCount);
        for (int frame = 0; frame < frameCount; ++frame)
            m_pScreens.push_back(apScreens[frame]);
        // Delete only the array of pointers, not the BITMAPs themselves
        delete[] apScreens;
        apScreens = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GATutorial, effectively
//                  resetting the members of this abstraction level only.

void GATutorial::Clear()
{
    m_TutorialPlayer = Players::PlayerOne;

    for (int area = 0; area < AREACOUNT; ++area)
    {
        m_TriggerBoxes[area].Reset();
        m_ScreenPositions[area].Reset();
        m_ScreenStates[area] = SCREENOFF;
        m_TextOffsets[area].Reset();
        m_TutAreaSteps[area].clear();
    }
    for (int room = 0; room < ROOMCOUNT; ++room)
    {
        m_RoomSignPositions[room].Reset();
        m_aapRoomSigns[room][UNLIT] = 0;
        m_aapRoomSigns[room][LIT] = 0;
    }
    m_AreaTimer.Reset();
    m_StepTimer.Reset();
    m_CurrentArea = BRAINCHAMBER;
    m_PrevArea = BRAINCHAMBER;
    m_ScreenChange = false;
    m_CurrentStep = 0;
    m_CurrentFrame = 0;
    m_CurrentRoom = ROOM0;

    for (int stage = 0; stage < FIGHTSTAGECOUNT; ++stage)
        m_FightTriggers[stage].Reset();

    m_CurrentFightStage = NOFIGHT;
    m_pCPUBrain = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GATutorial object ready for use.

int GATutorial::Create()
{
    if (GameActivity::Create() < 0)
        return -1;

    m_Description = "A tutorial for learning how to play Cortex Command. A good place to start!";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GATutorial to be identical to another, by deep copy.

int GATutorial::Create(const GATutorial &reference)
{
    if (GameActivity::Create(reference) < 0)
        return -1;

    for (int area = 0; area < AREACOUNT; ++area)
    {
        m_TriggerBoxes[area] = reference.m_TriggerBoxes[area];
        m_ScreenPositions[area] = reference.m_ScreenPositions[area];
        m_ScreenStates[area] = reference.m_ScreenStates[area];
        m_TextOffsets[area] = reference.m_TextOffsets[area];
        m_TutAreaSteps[area] = reference.m_TutAreaSteps[area];
    }
    for (int room = 0; room < ROOMCOUNT; ++room)
    {
        m_RoomSignPositions[room] = reference.m_RoomSignPositions[room];
    }
    m_AreaTimer.Reset();
    m_StepTimer.Reset();
    m_CurrentArea = reference.m_CurrentArea;
    m_PrevArea = reference.m_PrevArea;
    m_CurrentStep = reference.m_CurrentStep;
    m_CurrentFrame = reference.m_CurrentFrame;
    m_CurrentRoom = reference.m_CurrentRoom;

    for (int stage = 0; stage < FIGHTSTAGECOUNT; ++stage)
        m_FightTriggers[stage] = reference.m_FightTriggers[stage];

    m_CurrentFightStage = reference.m_CurrentFightStage;
// DOn't, owned and need to make deep copy in that case
//    m_pCPUBrain;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int GATutorial::ReadProperty(const std::string_view &propName, Reader &reader)
{
/*
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
*/
        return GameActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GATutorial with a Writer for
//                  later recreation with Create(Reader &reader);

int GATutorial::Save(Writer &writer) const {
	GameActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GATutorial object.

void GATutorial::Destroy(bool notInherited)
{
    if (!notInherited)
        GameActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SceneIsCompatible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if a particular Scene supports this specific Activity on it.
//                  Usually that means certain Area:s need to be defined in the Scene.

bool GATutorial::SceneIsCompatible(Scene *pScene, short teams)
{
    if (!GameActivity::SceneIsCompatible(pScene, teams))
        return false;

    // Quick and dirty hardcoded hack.. only this scene is compatible anyway
    if (pScene->GetPresetName() == "Tutorial Bunker")
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GATutorial::Start()
{
    int error = GameActivity::Start();
    
    ////////////////////////////////
    // Set up teams

    // Team 2 is always CPU
    SetCPUTeam(Teams::TeamTwo);

    for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
    {
        if (!m_TeamActive[team])
            continue;

        if (team == Teams::TeamOne)
        {
            // See if there are specified landing zone areas defined in the scene
            char str[64];
            std::snprintf(str, sizeof(str), "LZ Team %d", team + 1);
            Scene::Area *pArea = g_SceneMan.GetScene()->GetArea(str);
    //        pArea = pArea ? pArea : g_SceneMan.GetScene()->GetArea("Landing Zone");
            // If area is defined, save a copy so we can lock the LZ selection to within its boxes
            if (pArea && !pArea->HasNoArea())
                SetLZArea(team, *pArea);
        }

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
        // Give the player some scratch
        else
			m_TeamFunds[team] = this->GetStartingGold();
    }

    ///////////////////////////////////////
    // Set up players

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (!(m_IsActive[player] && m_IsHuman[player]))
            continue;

        // Expand all modules to show the goods
        m_pBuyGUI[player]->SetModuleExpanded(0);

        // No need for the LZ to follow the brain, the scene-set one is enough
        SetBrainLZWidth(player, 0);

        // If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
        if (!(m_Brain[player] = g_MovableMan.GetUnassignedBrain(m_Team[player])))
        {
            g_ConsoleMan.PrintString("ERROR: Can't find brain for tutorial game mode!");
        }
        // Set the found brain to be the selected actor at start
        else
        {
            m_TutorialPlayer = player;
            SwitchToActor(m_Brain[player], player, m_Team[player]);
            m_ActorCursor[player] = m_Brain[player]->GetPos();
            m_LandingZone[player].m_X = m_Brain[player]->GetPos().m_X;
            // Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
            m_ObservationTarget[player] = m_Brain[player]->GetPos();
        }
/*
        if (m_ActivityState == ActivityState::Editing)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Place your brain vault and build your bunker around it..." : "...then select \"DONE\" from the pie menu!", ScreenOfPlayer(player), 0);
        else if (m_ActivityState == ActivityState::Running)
            g_FrameMan.SetScreenText((player % 2 == 0) ? "Mine Gold and buy more firepower with the funds..." : "...then smash the competing brain to claim victory!", ScreenOfPlayer(player), 0);
*/
    }


    /////////////////////////////////////////////
    // SET UP TUTORIAL

    // COMMON SCREENS
    ContentFile screenFile;
    BITMAP **apScreens;
    screenFile.SetDataPath("Base.rte/GUIs/Tutorial/ScreenStatic.png");
    apScreens = screenFile.GetAsAnimation(3);
    m_apCommonScreens[SCREENOFF] = apScreens[0];
    m_apCommonScreens[STATICLITTLE] = apScreens[1];
    m_apCommonScreens[STATICLARGE] = apScreens[2];
    // Delete only the array of pointers, not the BITMAPs themselves
    delete[] apScreens;
    apScreens = 0;

    // ROOM SIGNS
    ContentFile signFile;
    BITMAP **apSigns;
    for (int room = 0; room < ROOMCOUNT; ++room)
    {
        if (room == ROOM0)
            signFile.SetDataPath("Base.rte/GUIs/Tutorial/TutEntryA.png");
        else if (room == ROOM1)
            signFile.SetDataPath("Base.rte/GUIs/Tutorial/TutEntryB.png");
        else if (room == ROOM2)
            signFile.SetDataPath("Base.rte/GUIs/Tutorial/TutEntryC.png");
        else if (room == ROOM3)
            signFile.SetDataPath("Base.rte/GUIs/Tutorial/TutEntryD.png");
        apSigns = signFile.GetAsAnimation(2);
        m_aapRoomSigns[room][UNLIT] = apSigns[0];
        m_aapRoomSigns[room][LIT] = apSigns[1];
        // Delete only the array of pointers, not the BITMAPs themselves
        delete[] apSigns;
        apSigns = 0;
    }

    m_RoomSignPositions[ROOM0].SetXY(744, 695);
    m_RoomSignPositions[ROOM1].SetXY(888, 695);
    m_RoomSignPositions[ROOM2].SetXY(888, 599);
    m_RoomSignPositions[ROOM3].SetXY(888, 431);
    m_CurrentRoom = ROOM3;

    //////////////////////////////////
    // Setup all the Tutorial Areas and their text instructions etc

    SetupAreas();

    // Disable all enemy AIs so they dont attack prematurely
    DisableAIs(true, Teams::TeamTwo);

    //////////////////////////////////
    // FIGHT TRIGGERS

    m_FightTriggers[DEFENDING].Create(Vector(1330, 0), 770, 400);

//    m_FightTriggerEast.Create(Vector(526, 0), 52, 840);
//    m_FightTriggerWest.Create(Vector(1336, 0), 52, 840);
/*
    // Start special tutorial playlist
    g_AudioMan.ClearMusicQueue();
    g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", 0);
    g_AudioMan.QueueSilence(30);
    g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");
    g_AudioMan.QueueSilence(30);
    g_AudioMan.QueueMusicStream("Base.rte/Music/Watts/Last Man.ogg");
    g_AudioMan.QueueSilence(30);
    g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");
*/
    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GATutorial::SetPaused(bool pause)
{
    GameActivity::SetPaused(pause);

    // Re-setup the ares with any updated control mappings that the player might have made in teh menu
    if (!pause)
        SetupAreas();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GATutorial::End()
{
    GameActivity::End();

    bool playerWon = false;
    // Show appropriate end game messages
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (!(m_IsActive[player] && m_IsHuman[player]))
            continue;

        if (m_Team[player] == m_WinnerTeam)
        {
            playerWon = true;
            // Set the winner's observation view to his controlled actors instead of his brain
            if (m_ControlledActor[player] && g_MovableMan.IsActor(m_ControlledActor[player]))
                m_ObservationTarget[player] = m_ControlledActor[player]->GetPos();
        }
    }

    // Play the approriate tune on player win/lose
    if (playerWon)
    {
        g_AudioMan.ClearMusicQueue();
        // Loop it twice, nice tune!
        g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2);
        g_AudioMan.QueueSilence(10);
        g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
    }
    else
    {
        g_AudioMan.ClearMusicQueue();
        g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 0);
        g_AudioMan.QueueSilence(10);
        g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
    }
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.

void GATutorial::UpdateEditing()
{
    GameActivity::UpdateEditing();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GATutorial. Supposed to be done every frame
//                  before drawing.

void GATutorial::Update()
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
        if (!(m_IsActive[player] && m_IsHuman[player]))
            continue;
        // The current player's team
        int team = m_Team[player];
        if (team == Teams::NoTeam)
            continue;

        // Make sure the game is not already ending
        if (m_ActivityState != ActivityState::Over)
        {
            // Check if any player's brain is dead
            if (!g_MovableMan.IsActor(m_Brain[player]))
            {
                m_Brain[player] = 0;
                g_FrameMan.SetScreenText("Your brain has been destroyed!", ScreenOfPlayer(player), 333);

                // Now see if all brains are dead of this player's team, and if so, end the game
                if (!g_MovableMan.GetFirstBrainActor(team))
                {
                    m_WinnerTeam = OtherTeam(team);
                    End();
                }

                m_MessageTimer[player].Reset();
            }
            // Mark the player brain to be protected when the fight happens
            else if (m_CurrentFightStage >= DEFENDING)
            {
                // Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
//                SetObservationTarget(m_Brain[player]->GetPos(), player);
                // Mark the player's brain to be protected by his team
                AddObjectivePoint("Protect!", m_Brain[player]->GetPos() + Vector(0, 10), team, GameActivity::ARROWUP);
                // Mark the CPU brain for desctruction too
                if (g_MovableMan.IsActor(m_pCPUBrain))
                    AddObjectivePoint("Destroy!", m_pCPUBrain->GetPos() + Vector(0, 12), team, GameActivity::ARROWUP);
            }
        }
        // Game over, show the appropriate messages until a certain time
        else if (!m_GameOverTimer.IsPastSimMS(m_GameOverPeriod))
        {
// TODO: make more appropriate messages here for run out of funds endings
            if (m_Team[player] == m_WinnerTeam)
                g_FrameMan.SetScreenText("You destroyed the dummy CPU!\nPress [SPACE] or [START] to continue", ScreenOfPlayer(player));
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
        /////////////////////////////////////////
        // Attacking CPU team logic
/*
        if (team == m_CPUTeam)
        {
            // Spawn the CPU team's attacking forces
            if (m_SpawnTimer.IsPastSimMS(m_SpawnInterval) && m_ActivityState != ActivityState::Over)
            {
                if (!m_AttackerSpawns.empty())
                {
                    int whichSpawn = std::floor(m_AttackerSpawns.size() * RandomNum());
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
                            landingZone.m_X = g_SceneMan.GetSceneWidth() * RandomNum();
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
*/
        ///////////////////////////////////////////
        // Check for victory conditions

        // Make sure the game is not already ending
        if (m_ActivityState != ActivityState::Over && team != m_CPUTeam)
        {
// TODO: Gotto have budget restrictions in this activity!
/*
            // Check for bankruptcy
// TODO Don't hardcode the rocket cost!
            if (m_TeamFunds[team] < 0)//&& Only brain is left of actors)
            {
                for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
                {
                    if (!(m_IsActive[player] && m_IsHuman[player]))
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

    ///////////////////////////////////////////
    // TUTORIAL LOGIC

    // Detect the player going into new areas
    if (m_ControlledActor[m_TutorialPlayer])
    {
        for (int area = 0; area < AREACOUNT; ++area)
        {
            // Switch if within the trigger box of a new area
            if (area != m_CurrentArea && m_TriggerBoxes[area].IsWithinBox(m_ControlledActor[m_TutorialPlayer]->GetPos()))
            {
                // Change to the new area
                m_PrevArea = m_CurrentArea;
                m_CurrentArea = (TutorialArea)area;
                m_CurrentStep = 0;
                m_ScreenChange = true;
                m_AreaTimer.Reset();
                m_StepTimer.Reset();
            }
        }
/*
        // Fight stage triggers
        for (int stage = 0; stage < FIGHTSTAGECOUNT; ++stage)
        {

            // Switch if within the trigger box of a new area
            if (area != m_CurrentArea && m_TriggerBoxes[area].IsWithinBox(m_ControlledActor[m_TutorialPlayer]->GetPos()))
            {

            if (m_FightTriggers[stage].Reset();
        }
*/
    }

    // Cycle through the steps of the current area
    if (m_StepTimer.IsPastRealMS(m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Duration))
    {
        // Go to next step, looping around to the first if necessary
        if (++m_CurrentStep == m_TutAreaSteps[m_CurrentArea].size())
            m_CurrentStep = 0;
        m_ScreenChange = true;
        // Start timing the new step
        m_StepTimer.Reset();
    }

    // Only mess with animation if there are more than one frame for this step and the frameduration is set to something
    int frameCount = m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_pScreens.size();
    if (frameCount > 0 && m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_FrameDuration > 0)
    {
        int newFrame = ((long)m_StepTimer.GetElapsedRealTimeMS() / m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_FrameDuration) % frameCount;
        // Clamp the frame
        newFrame = newFrame < 0 ? 0 : (newFrame >= frameCount ? frameCount - 1 : newFrame);
        if (newFrame != m_CurrentFrame)
        {
            m_CurrentFrame = newFrame;
            m_ScreenChange = true;
        }
    }

/* Draw this manually over the current screen in DrawGUI
    // Take over control of screen messages
    m_MessageTimer[m_TutorialPlayer].Reset();
    // Display the text of the current step
//    g_FrameMan.ClearScreenText();
    g_FrameMan.SetScreenText(m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Text, 0, 500, -1, true);//, m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Duration);
*/
    // Draw the correct current screens
    BITMAP *pScreen = 0;

    // Turn ON the screen of the CURRENT area, animating the static-y frames and drawing them to the scene
    // Then show the current step image
    if (m_ScreenStates[m_CurrentArea] != SHOWINGSTEP || m_ScreenChange)
    {
        // Figure out if to draw static or the step screen
        m_ScreenStates[m_CurrentArea] = m_AreaTimer.IsPastRealMS(200) ? SHOWINGSTEP : (m_AreaTimer.IsPastRealMS(100) ? STATICLARGE : STATICLITTLE);

        pScreen = 0;
        // Showing step image or a static screen?
        if (m_ScreenStates[m_CurrentArea] == SHOWINGSTEP)
        {
            if (m_CurrentFrame < frameCount)
                pScreen = m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_pScreens[m_CurrentFrame];
        }
        else
            pScreen = m_apCommonScreens[(int)(m_ScreenStates[m_CurrentArea])];

        // Draw to the scene bg layer
        if (pScreen)
            blit(pScreen, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_ScreenPositions[m_CurrentArea].GetFloorIntX(), m_ScreenPositions[m_CurrentArea].GetFloorIntY(), pScreen->w, pScreen->h);

        m_ScreenChange = false;
    }

    // Turn OFF the screen of all the other areas, animating the static-y frames and drawing them to the scene
    for (int area = 0; area < AREACOUNT; ++area)
    {
        pScreen = 0;
        // Turn off all other areas
        if (area != m_CurrentArea && m_ScreenStates[area] != SCREENOFF)
        {
            m_ScreenStates[area] = m_AreaTimer.IsPastRealMS(200) ? SCREENOFF : (m_AreaTimer.IsPastRealMS(100) ? STATICLITTLE : STATICLARGE);
            pScreen = m_apCommonScreens[(int)(m_ScreenStates[area])];
            if (pScreen)
                blit(pScreen, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_ScreenPositions[area].GetFloorIntX(), m_ScreenPositions[area].GetFloorIntY(), pScreen->w, pScreen->h);
        }
    }

    ////////////////////////
    // ROOM SIGNS
    
    // Translate current area to a room
    TutorialRoom prevRoom = m_CurrentRoom;
    if (m_CurrentArea == BRAINCHAMBER)
        m_CurrentRoom = ROOM0;
    else if (m_CurrentArea == BODYSTORAGE)
        m_CurrentRoom = ROOM1;
    else if (m_CurrentArea == OBSTACLECOURSE)
        m_CurrentRoom = ROOM2;
    else if (m_CurrentArea == FIRINGRANGE)
        m_CurrentRoom = ROOM3;

    // Draw the correct currently lit or blinking signs
    BITMAP *pSign = 0;
    if (prevRoom != m_CurrentRoom)
    {
        pSign = m_aapRoomSigns[ROOM0][m_CurrentRoom >= ROOM0 ? LIT : UNLIT];
        blit(pSign, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_RoomSignPositions[ROOM0].GetFloorIntX(), m_RoomSignPositions[ROOM0].GetFloorIntY(), pSign->w, pSign->h);
        pSign = m_aapRoomSigns[ROOM1][m_CurrentRoom >= ROOM1 ? LIT : UNLIT];
        blit(pSign, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_RoomSignPositions[ROOM1].GetFloorIntX(), m_RoomSignPositions[ROOM1].GetFloorIntY(), pSign->w, pSign->h);
        pSign = m_aapRoomSigns[ROOM2][m_CurrentRoom >= ROOM2 ? LIT : UNLIT];
        blit(pSign, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_RoomSignPositions[ROOM2].GetFloorIntX(), m_RoomSignPositions[ROOM2].GetFloorIntY(), pSign->w, pSign->h);
        pSign = m_aapRoomSigns[ROOM3][m_CurrentRoom >= ROOM3 ? LIT : UNLIT];
        blit(pSign, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_RoomSignPositions[ROOM3].GetFloorIntX(), m_RoomSignPositions[ROOM3].GetFloorIntY(), pSign->w, pSign->h);
    }
    // Blink the next room's sign
    if (m_CurrentRoom < ROOM3)
    {
        pSign = m_aapRoomSigns[m_CurrentRoom + 1][m_AreaTimer.AlternateReal(200) ? LIT : UNLIT];
        blit(pSign, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0, m_RoomSignPositions[m_CurrentRoom + 1].GetFloorIntX(), m_RoomSignPositions[m_CurrentRoom + 1].GetFloorIntY(), pSign->w, pSign->h);
    }

    ////////////////////////
    // FIGHT LOGIC

    if (m_ControlledActor[m_TutorialPlayer])
    {
        // Triggered defending stage
        if (m_CurrentFightStage == NOFIGHT && m_FightTriggers[DEFENDING].IsWithinBox(m_ControlledActor[m_TutorialPlayer]->GetPos()))
        {
            // Take over control of screen messages
            m_MessageTimer[m_TutorialPlayer].Reset();
            // Display the text of the current step
            g_FrameMan.ClearScreenText(ScreenOfPlayer(m_TutorialPlayer));
            g_FrameMan.SetScreenText("DEFEND YOUR BRAIN AGAINST THE INCOMING FORCES!", ScreenOfPlayer(m_TutorialPlayer), 500, 8000, true);
            // This will make all the enemy team AI's go into brain hunt mode
            GameActivity::InitAIs();
            DisableAIs(false, Teams::TeamTwo);

            // Advance the stage
            m_CurrentFightStage = DEFENDING;
        }
    }

    ///////////////////////////////////////////
    // Check for victory conditions

    // Check if the CPU brain is dead, if we're playing against the CPU
    if (!g_MovableMan.IsActor(m_pCPUBrain) && m_ActivityState != ActivityState::Over)
    {
        m_pCPUBrain = 0;
        // Proclaim player winner and end
        m_WinnerTeam = Teams::TeamOne;
        // Finito!
        End();
    }

    // After a while of game over and we won, exit to the campaign menu automatically
    if (m_ActivityState == ActivityState::Over && m_WinnerTeam == Teams::TeamOne)
    {
        if (m_GameOverTimer.IsPastSimMS(m_GameOverPeriod))
        {
            g_FrameMan.ClearScreenText(ScreenOfPlayer(m_TutorialPlayer));
            g_FrameMan.SetScreenText("Press [SPACE] or [START] to continue!", ScreenOfPlayer(m_TutorialPlayer), 750);
        }

        if (m_GameOverTimer.IsPastSimMS(54000) || g_UInputMan.AnyStartPress())
        {
            g_ActivityMan.EndActivity();
            g_InActivity = false;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void GATutorial::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    GameActivity::DrawGUI(pTargetBitmap, targetPos, which);

    if (IsRunning())// && (m_AreaTimer.AlternateReal(500) || m_AreaTimer.AlternateReal(250) || m_AreaTimer.AlternateReal(125)))
    {
        AllegroBitmap pBitmapInt(pTargetBitmap);
        Vector screenTextPos = m_ScreenPositions[m_CurrentArea] + m_TextOffsets[m_CurrentArea] - targetPos;
        // How long the revealing of the text period will be, clamped, and plus three for the last three dots added later
        float revealPeriod = (m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Text.size() + 3) * 30;
        if (revealPeriod > m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Duration * 0.85)
            revealPeriod = m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Duration * 0.85;
        // The normalized reveal control
        float revealed = m_StepTimer.GetElapsedRealTimeMS() / revealPeriod;
        if (revealed > 1.0)
            revealed = 1.0;
        string revealText = m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Text.substr(0, (m_TutAreaSteps[m_CurrentArea][m_CurrentStep].m_Text.size() + 3) * revealed);
        // Dot blinking logic
        if (revealed == 1.0)
        {
            int timePhase = (int)m_AreaTimer.GetElapsedRealTimeMS() % 1200;
            revealText = revealText + (timePhase > 900 ? "..." : (timePhase > 600 ? ".. " : (timePhase > 300 ? ".  " : "   ")));
        }
        g_FrameMan.GetSmallFont()->DrawAligned(&pBitmapInt, screenTextPos.m_X, screenTextPos.m_Y, revealText.c_str(), GUIFont::Centre);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GATutorial's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GATutorial::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    GameActivity::Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitAIs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through all Actor:s currently in the MovableMan and gives each
//                  one not controlled by a Controller a CAI and appropriate AIMode setting
//                  based on team and CPU team.

void GATutorial::InitAIs()
{
    Actor *pActor = 0;
    Actor *pFirstActor = 0;

    for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
    {
        if (!m_TeamActive[team])
            continue;
        // Get the first one
        pFirstActor = pActor = g_MovableMan.GetNextTeamActor(team);

        do
        {
            // Set up AI controller if currently not player controlled
            if (pActor && !pActor->GetController()->IsPlayerControlled())
            {
                pActor->SetControllerMode(Controller::CIM_AI);

                // If human, set appropriate AI mode
                if (dynamic_cast<AHuman *>(pActor) || dynamic_cast<ACrab *>(pActor))
                {
                    // Sentry default
                    if (team == m_CPUTeam)
                        pActor->SetAIMode(AHuman::AIMODE_SENTRY);
                    else if (team >= 0)
                        pActor->SetAIMode(AHuman::AIMODE_SENTRY);
                    // Let the non team actors be (the wildlife)
                    else
                        ;
                }
            }

            // Next!
            pActor = g_MovableMan.GetNextTeamActor(team, pActor);
        }
        while (pActor && pActor != pFirstActor);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetupAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up or resets the Tutorial Areas to show the current control
//                  mappings etc.

void GATutorial::SetupAreas()
{
    int device = g_UInputMan.GetControlScheme(m_TutorialPlayer)->GetDevice();
    int preset = g_UInputMan.GetControlScheme(m_TutorialPlayer)->GetPreset();

    // Adjust for special commands when using the keyboard-only setup
    string JumpName = MAPNAME(INPUT_L_UP);
    string CrouchName = MAPNAME(INPUT_L_DOWN);
    if (device == DEVICE_KEYB_ONLY)
    {
        JumpName = MAPNAME(INPUT_JUMP);
        CrouchName = MAPNAME(INPUT_CROUCH);
    }

    // If no preset, adjust the pie menu and fire names when using the defaults on a gamepad.. otherwise it'll show up as an unhelpful "Joystick"
    string PieName = MAPNAME(INPUT_PIEMENU);
    string FireName = MAPNAME(INPUT_FIRE);
    if (device >= DEVICE_GAMEPAD_1 && preset == PRESET_NONE)
    {
        PieName = "Pie Menu Trigger";
        FireName = "Fire Trigger";
    }

    // BRAINCHAMBER
    // Set up the trigger area
    m_TriggerBoxes[BRAINCHAMBER].SetCorner(Vector(631, 608));
    m_TriggerBoxes[BRAINCHAMBER].SetWidth(137);
    m_TriggerBoxes[BRAINCHAMBER].SetHeight(155);
    // Screen position
    m_ScreenPositions[BRAINCHAMBER].SetXY(673, 688);
    // Text offset from screen position
    m_TextOffsets[BRAINCHAMBER].SetXY(m_apCommonScreens[0]->w / 2, m_apCommonScreens[0]->h + 4);
    // Set up the steps
    m_TutAreaSteps[BRAINCHAMBER].clear();
    m_TutAreaSteps[BRAINCHAMBER].push_back(TutStep("Welcome to Cortex Command", 4000, "Base.rte/GUIs/Tutorial/CCLogo.png", 2));
    m_TutAreaSteps[BRAINCHAMBER].push_back(TutStep("Above is your disembodied brain", 4000, "Base.rte/GUIs/Tutorial/ArrowUp.png", 2));
    m_TutAreaSteps[BRAINCHAMBER].push_back(TutStep("With it, you can remotely control other bodies", 4000, "Base.rte/GUIs/Tutorial/BodyZap.png", 2));
    m_TutAreaSteps[BRAINCHAMBER].push_back(TutStep("Switch to one now by using [" + MAPNAME(INPUT_PREV) + "] or [" + MAPNAME(INPUT_NEXT) + "]", 4000, "Base.rte/GUIs/Tutorial/BodyZap.png", 2));
    m_TutAreaSteps[BRAINCHAMBER].push_back(TutStep("If you haven't set up your controls yet, hit [Esc] and do so", 8000, "Base.rte/GUIs/Tutorial/Joystick.png", 2));

    // BODYSTORAGE
    // Set up the trigger area
    m_TriggerBoxes[BODYSTORAGE].SetCorner(Vector(911, 662));
    m_TriggerBoxes[BODYSTORAGE].SetWidth(397);
    m_TriggerBoxes[BODYSTORAGE].SetHeight(98);
    // Screen position
    m_ScreenPositions[BODYSTORAGE].SetXY(961, 688);
    // Text offset from screen position
    m_TextOffsets[BODYSTORAGE].SetXY(m_apCommonScreens[0]->w / 2, m_apCommonScreens[0]->h + 4);
    // Set up the steps
    m_TutAreaSteps[BODYSTORAGE].clear();
    m_TutAreaSteps[BODYSTORAGE].push_back(TutStep("Here are some dummy bodies for practice", 4000, "Base.rte/GUIs/Tutorial/BodyHop.png", 2));
    m_TutAreaSteps[BODYSTORAGE].push_back(TutStep("Quickly switch control between them left and right with [" + MAPNAME(INPUT_PREV) + "] and [" + MAPNAME(INPUT_NEXT) + "]", 6000, "Base.rte/GUIs/Tutorial/BodyZap.png", 2));
    m_TutAreaSteps[BODYSTORAGE].push_back(TutStep("Or hold down either [" + MAPNAME(INPUT_PREV) + "] or [" + MAPNAME(INPUT_NEXT) + "] to get a selection cursor", 6000, "Base.rte/GUIs/Tutorial/BodyZap.png", 2));
    m_TutAreaSteps[BODYSTORAGE].push_back(TutStep("Switch to the leftmost body and walk it out of the room with [" + MAPNAME(INPUT_L_LEFT) + "]", 8000, "Base.rte/GUIs/Tutorial/ArrowLeft.png", 2));

    // SHAFT
    // Set up the trigger area
    m_TriggerBoxes[SHAFT].SetCorner(Vector(772, 385));
    m_TriggerBoxes[SHAFT].SetWidth(135);
    m_TriggerBoxes[SHAFT].SetHeight(380);
    // Screen position
    m_ScreenPositions[SHAFT].SetXY(817, 688);
    // Text offset from screen position
    m_TextOffsets[SHAFT].SetXY(m_apCommonScreens[0]->w / 2, m_apCommonScreens[0]->h + 4);
    // Set up the steps
    m_TutAreaSteps[SHAFT].clear();
    m_TutAreaSteps[SHAFT].push_back(TutStep("Use [" + JumpName + "] to activate jetpack", 4000, "Base.rte/GUIs/Tutorial/ArrowUp.png", 2));
    m_TutAreaSteps[SHAFT].push_back(TutStep("Fire jetpack in bursts for better control", 4000, "Base.rte/GUIs/Tutorial/BodyJetpack.png", 2));
    m_TutAreaSteps[SHAFT].push_back(TutStep("Adjust the jet direction by aiming or looking", 4000, "Base.rte/GUIs/Tutorial/BodyJetpack.png", 2));
    m_TutAreaSteps[SHAFT].push_back(TutStep("Jump height is affected by the body's total weight", 4000, "Base.rte/GUIs/Tutorial/BodyJetpack.png", 2));
    m_TutAreaSteps[SHAFT].push_back(TutStep("So the more you carry, the less you can jump", 8000, "Base.rte/GUIs/Tutorial/BodyJetpack.png", 2));

    // OBSTACLECOURSE
    // Set up the trigger area
    m_TriggerBoxes[OBSTACLECOURSE].SetCorner(Vector(915, 492));
    m_TriggerBoxes[OBSTACLECOURSE].SetWidth(395);
    m_TriggerBoxes[OBSTACLECOURSE].SetHeight(167);
    // Screen position
    m_ScreenPositions[OBSTACLECOURSE].SetXY(961, 592);
    // Text offset from screen position
    m_TextOffsets[OBSTACLECOURSE].SetXY(m_apCommonScreens[0]->w / 2, m_apCommonScreens[0]->h + 4);
    // Set up the steps
    m_TutAreaSteps[OBSTACLECOURSE].clear();
    m_TutAreaSteps[OBSTACLECOURSE].push_back(TutStep("Obstacle course", 2000, "Base.rte/GUIs/Tutorial/BodyHop.png", 2));
    m_TutAreaSteps[OBSTACLECOURSE].push_back(TutStep("Climb obstacles by holding [" + MAPNAME(INPUT_L_RIGHT) + "]", 4000, "Base.rte/GUIs/Tutorial/ArrowRight.png", 2));
    m_TutAreaSteps[OBSTACLECOURSE].push_back(TutStep("Even climb ladders by simply moving toward them", 8000, "Base.rte/GUIs/Tutorial/BodyClimb.png", 2));
    m_TutAreaSteps[OBSTACLECOURSE].push_back(TutStep("You can also crouch and crawl with [" + CrouchName + "]", 6000, "Base.rte/GUIs/Tutorial/BodyCrawl.png", 2));
    m_TutAreaSteps[OBSTACLECOURSE].push_back(TutStep("In tight spaces, you may need to angle your head down to get through", 8000, "Base.rte/GUIs/Tutorial/BodyCrawl.png", 2));

    // FIRINGRANGE
    // Set up the trigger area
    m_TriggerBoxes[FIRINGRANGE].SetCorner(Vector(913, 394));
    m_TriggerBoxes[FIRINGRANGE].SetWidth(389);
    m_TriggerBoxes[FIRINGRANGE].SetHeight(97);
    // Screen position
    m_ScreenPositions[FIRINGRANGE].SetXY(961, 424);
    // Text offset from screen position
    m_TextOffsets[FIRINGRANGE].SetXY(m_apCommonScreens[0]->w / 2, m_apCommonScreens[0]->h + 4);
    // Set up the steps
    m_TutAreaSteps[FIRINGRANGE].clear();
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Firing range", 2000, "Base.rte/GUIs/Tutorial/FireTarget.png", 1));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Pick up the weapon by first standing over it", 4000, "Base.rte/GUIs/Tutorial/ArrowRight.png", 2));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("And then hold down [" + PieName + "]", 4000, "Base.rte/GUIs/Tutorial/MenuPickUp.png", 1, 500));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Point up to 'Pick Up'", 4000, "Base.rte/GUIs/Tutorial/MenuPickUp.png", 2, 500));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Release [" + PieName + "] to complete the command", 4000, "Base.rte/GUIs/Tutorial/MenuPickUp.png", 1));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("If you aim continuously toward your target", 4000, "Base.rte/GUIs/Tutorial/BodyAim.png", 3));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("It improves your accuracy and view distance", 4000, "Base.rte/GUIs/Tutorial/BodyAim.png", 3));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Use [" + FireName + "] to Fire!", 4000, "Base.rte/GUIs/Tutorial/BodyFire.png", 2));
    m_TutAreaSteps[FIRINGRANGE].push_back(TutStep("Reload manually with [" + PieName + "] + up", 8000, "Base.rte/GUIs/Tutorial/BodyFire.png", 2));

    // ROOFTOP
    // Set up the trigger area
    m_TriggerBoxes[ROOFTOP].SetCorner(Vector(732, 176));
    m_TriggerBoxes[ROOFTOP].SetWidth(356);
    m_TriggerBoxes[ROOFTOP].SetHeight(210);
    // Screen position
    m_ScreenPositions[ROOFTOP].SetXY(961, 316);
    // Text offset from screen position
    m_TextOffsets[ROOFTOP].SetXY(m_apCommonScreens[0]->w / 2, -16);
    // Set up the steps
    m_TutAreaSteps[ROOFTOP].clear();
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Pick up the digging tool", 4000, "Base.rte/GUIs/Tutorial/MenuPickUp.png", 2, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Use it on the dirt here", 8000, "Base.rte/GUIs/Tutorial/DigPile.png", 2, 750));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("If you dig up gold, it is added to your team's funds", 4000, "Base.rte/GUIs/Tutorial/Funds.png", 2, 250));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Funds can be spent in the Buy Menu", 4000, "Base.rte/GUIs/Tutorial/Funds.png", 1, 333));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Which is opened through the Command Menu", 4000, "Base.rte/GUIs/Tutorial/MenuBuyMenu.png", 1, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Hold [" + PieName + "] and point up-left to 'Buy Menu'", 6000, "Base.rte/GUIs/Tutorial/MenuBuyMenu.png", 2, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("The Buy Menu works like a shopping cart", 6000, "Base.rte/GUIs/Tutorial/BuyMenuCargo.png", 1, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Add to the Cargo list the items you want delivered", 6000, "Base.rte/GUIs/Tutorial/BuyMenuCargo.png", 2, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Then use the BUY button, or click outside the menu", 4000, "Base.rte/GUIs/Tutorial/BuyMenuBuy.png", 2, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Finally select a flat area where you want the goods delivered", 8000, "Base.rte/GUIs/Tutorial/BuyMenuBuy.png", 1, 500));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Next, you can go explore to the west to try flying and climbing in the wild", 6000, "Base.rte/GUIs/Tutorial/ArrowLeft.png", 2));
    m_TutAreaSteps[ROOFTOP].push_back(TutStep("Or, go to the east to learn about squads!", 8000, "Base.rte/GUIs/Tutorial/ArrowRight.png", 2));

    // ROOFEAST
    // Set up the trigger area
    m_TriggerBoxes[ROOFEAST].SetCorner(Vector(1100, 176));
    m_TriggerBoxes[ROOFEAST].SetWidth(200);
    m_TriggerBoxes[ROOFEAST].SetHeight(210);
    // Screen position
    m_ScreenPositions[ROOFEAST].SetXY(1201, 316);
    // Text offset from screen position
    m_TextOffsets[ROOFEAST].SetXY(m_apCommonScreens[0]->w / 2, -16);
    // Set up the steps
    m_TutAreaSteps[ROOFEAST].clear();
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("Hold [" + PieName + "] and point up-right to 'Form Squad'", 4000, "Base.rte/GUIs/Tutorial/MenuTeam.png", 2, 500));
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("Adjust selection circle to select nearby bodies", 4000, "Base.rte/GUIs/Tutorial/TeamSelect.png", 4, 500));
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("All selected units will follow you, and engage on their own", 4000, "Base.rte/GUIs/Tutorial/TeamFollow.png", 2, 500));
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("Units with weapons similar to the leader's will fire in unison with him.", 4000, "Base.rte/GUIs/Tutorial/TeamFollow.png", 2, 500));
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("Hold [" + PieName + "] and point up-right again to disband squad", 4000, "Base.rte/GUIs/Tutorial/MenuTeam.png", 2, 500));
    m_TutAreaSteps[ROOFEAST].push_back(TutStep("Next, you can go to the east for a TRIAL BATTLE!", 8000, "Base.rte/GUIs/Tutorial/ArrowRight.png", 2));

    m_AreaTimer.Reset();
    m_StepTimer.Reset();
}

} // namespace RTE
