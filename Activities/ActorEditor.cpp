//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActorEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ActorEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ActorEditor.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
//#include "AHuman.h"
//#include "MOPixel.h"
#include "SLTerrain.h"
#include "Controller.h"
//#include "AtomGroup.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUIComboBox.h"

#include "ObjectPickerGUI.h"
#include "PieMenuGUI.h"
#include "GABaseDefense.h"

namespace RTE {

ConcreteClassInfo(ActorEditor, EditorActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ActorEditor, effectively
//                  resetting the members of this abstraction level only.

void ActorEditor::Clear()
{
    m_pEditedActor = 0;
    m_ActorDataFilePath.clear();
    m_pPicker = 0;
    m_pPieMenu = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActorEditor object ready for use.

int ActorEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ActorEditor to be identical to another, by deep copy.

int ActorEditor::Create(const ActorEditor &reference)
{
    if (EditorActivity::Create(reference) < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Load and edit Actors.";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ActorEditor::ReadProperty(const std::string_view &propName, Reader &reader)
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
        return EditorActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ActorEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int ActorEditor::Save(Writer &writer) const {
	EditorActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActorEditor object.

void ActorEditor::Destroy(bool notInherited)
{
    delete m_pEditedActor;
    delete m_pPicker;

    if (!notInherited)
        EditorActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int ActorEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the picker GUI

    if (m_pPicker)
        m_pPicker->Reset();
    else
        m_pPicker = new ObjectPickerGUI;
    m_pPicker->Create(&(m_PlayerController[0]), -1, "Actor");

    if (m_pPieMenu)
        m_pPieMenu->Destroy();
    else
        m_pPieMenu = new PieMenuGUI;
    m_pPieMenu->Create(&(m_PlayerController[0]));

    m_EditorMode = EditorActivity::EDITINGOBJECT;
    m_ModeChange = true;

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void ActorEditor::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void ActorEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActorEditor. Supposed to be done every frame
//                  before drawing.

void ActorEditor::Update()
{
    // And object hasn't been loaded yet, so get the loading picker going
    if (!m_pEditedActor)
    {
        m_NeedSave = false;
        m_HasEverBeenSaved = false;
        // Show the picker dialog to select an object to load
        m_pPicker->SetEnabled(true);
        // Scroll to center of scene
        g_SceneMan.SetScrollTarget(g_SceneMan.GetSceneDim() * 0.5);
        m_EditorMode = m_PreviousMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }

    // Mode switching etc - don't need for this yet
//    EditorActivity::Update();
    // Update the player controller since we're not calling parent update
    m_PlayerController[0].Update();

    if (!g_SceneMan.GetScene())
        return;

    /////////////////////////////////////////////////////
    // Update the edited Actor

    if (m_pEditedActor)
    {
        m_pEditedActor->SetPos(g_SceneMan.GetSceneDim() * 0.5);
        m_pEditedActor->Update();
        g_SceneMan.SetScrollTarget(m_pEditedActor->GetPos());
    }

    /////////////////////////////////////////////////////
    // Picker logic

    // Enable or disable the picker
    m_pPicker->SetEnabled(m_EditorMode == EditorActivity::LOADDIALOG);

    // Update the picker GUI
    m_pPicker->Update();

    // Set the screen occlusion situation
    if (!m_pPicker->IsVisible())
        g_SceneMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(Players::PlayerOne));

    // Picking something to load into the editor
    if (m_EditorMode == EditorActivity::LOADDIALOG)
    {
        g_FrameMan.ClearScreenText();
        g_FrameMan.SetScreenText("Select an Actor to LOAD into the editor ->", 0, 333);

        // Picked something!
        if (m_pPicker->ObjectPicked() && !m_pPicker->IsEnabled())
        {
            // Load the picked Actor into the editor
            LoadActor(m_pPicker->ObjectPicked());
        }
    }
    else
    {
        g_FrameMan.ClearScreenText();
        g_FrameMan.SetScreenText("Control the actor to see how he moves. Reload data with the pie menu.", 0, 0, 5000);
    }

    //////////////////////////////////////////////
    // Pie menu logic

    m_pPieMenu->Update();

    // Show the pie menu only when the secondary button is held down
    if (m_PlayerController[0].IsState(PIE_MENU_ACTIVE) && m_EditorMode != EditorActivity::LOADDIALOG)
    {
        m_pPieMenu->SetEnabled(true);
//        m_pPieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);
        m_pPieMenu->SetPos(g_SceneMan.GetSceneDim() / 2);
    }
    else
        m_pPieMenu->SetEnabled(false);

    // Handle what user does with the pie menu
    if (m_pPieMenu->GetPieCommand() != PieSlice::PieSliceIndex::PSI_NONE) {
        // User chose to reload the Actor's data
        if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_LOAD) {
            ReloadActorData();
        // User chose to pick a new Actor to edit
        } else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_PICK) {
            m_EditorMode = EditorActivity::LOADDIALOG;
            m_ModeChange = true;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void ActorEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    // Draw the edited actor
    if (m_pEditedActor)
    {
        m_pEditedActor->Draw(pTargetBitmap, targetPos, g_DrawColor);
        m_pEditedActor->Draw(pTargetBitmap, targetPos, g_DrawDebug);
    }

    // Clear out annoying blooms
    g_PostProcessMan.ClearScenePostEffects();

    m_pPicker->Draw(pTargetBitmap);
    m_pPieMenu->Draw(pTargetBitmap, targetPos);

    EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActorEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void ActorEditor::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the Actor itself and sets up the pie menu to match its setup.

bool ActorEditor::LoadActor(const Entity *pActorToLoad)
{
    if (!pActorToLoad)
        return false;

    // Replace the old one
    delete m_pEditedActor;
    // Make a copy of the picked object reference
    m_pEditedActor = dynamic_cast<Actor *>(pActorToLoad->Clone());

    if (m_pEditedActor)
    {
        // Set up the editor for the new actor
        m_pEditedActor->SetControllerMode(Controller::CIM_PLAYER, Players::PlayerOne);
        // Set up the pie menu with the actor's own slices
        m_pPieMenu->ResetSlices();
        // Add the reload data slice
		PieSlice reloadSlice("Reload Actor's Data", PieSlice::PieSliceIndex::PSI_LOAD, PieSlice::SliceDirection::UP, true);
        m_pPieMenu->AddSlice(reloadSlice);
        PieSlice chooseSlice("Choose a different Actor", PieSlice::PieSliceIndex::PSI_PICK, PieSlice::SliceDirection::RIGHT, true);
		m_pPieMenu->AddSlice(chooseSlice);
        // Add the Actor's slices
        m_pEditedActor->AddPieMenuSlices(m_pPieMenu);
        // Final slice setup
        m_pPieMenu->RealignSlices();

        // Get the actor's data path so we can reload that file quickly
        m_ActorDataFilePath = g_PresetMan.GetEntityDataLocation(m_pEditedActor->GetClassName(), m_pEditedActor->GetPresetName(), m_pEditedActor->GetModuleID());
        m_EditorMode = EditorActivity::EDITINGOBJECT;
        m_ModeChange = true;
    }
    else
    {
        g_FrameMan.ClearScreenText();
        g_FrameMan.SetScreenText("There's something wrong with that picked Actor!?", 0, 333, 2000);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadActorData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the ini with the currently edited Actor's definitions.

bool ActorEditor::ReloadActorData()
{
    if (!m_pEditedActor || m_ActorDataFilePath.empty())
        return false;

    // Set up the reader to overwrite old definitions
    Reader reader(m_ActorDataFilePath.c_str(), true);

    // Read in all the objects defined in the ini, overwriting any previous definitions
    while (reader.NextProperty())
    {
        reader.ReadPropName();
        g_PresetMan.GetEntityPreset(reader);
    }

    // Now reload the actor from the new definition
    const Entity *pEntity = g_PresetMan.GetEntityPreset(m_pEditedActor->GetClassName(), m_pEditedActor->GetPresetName(), m_pEditedActor->GetModuleID());
    LoadActor(pEntity);

    return m_pEditedActor != 0;
}


} // namespace RTE