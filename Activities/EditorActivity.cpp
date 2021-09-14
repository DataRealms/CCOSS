//////////////////////////////////////////////////////////////////////////////////////////
// File:            EditorActivity.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the EditorActivity class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "EditorActivity.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"
#include "AllegroInput.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUITab.h"
#include "GUIListBox.h"
#include "GUITextBox.h"
#include "GUIButton.h"
#include "GUILabel.h"
#include "GUIComboBox.h"

namespace RTE {

AbstractClassInfo(EditorActivity, Activity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this EditorActivity, effectively
//                  resetting the members of this abstraction level only.

void EditorActivity::Clear()
{
    // Most editors are single player affairs
    m_MaxPlayerSupport = 1;
    m_MinTeamsRequired = 1;
    m_EditorMode = NEWDIALOG;
    m_PreviousMode = EDITINGOBJECT;
    m_ModeChange = false;
    m_ModuleSpaceID = 0;
    m_NeedSave = false;
    m_HasEverBeenSaved = false;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_pNewDialogBox = 0;
    m_pNewModuleCombo = 0;
    m_pNewButton = 0;
    m_pNewCancel = 0;
    m_pLoadDialogBox = 0;
    m_pLoadNameCombo = 0;
    m_pLoadToNewButton = 0;
    m_pLoadButton = 0;
    m_pLoadCancel = 0;
    m_pSaveDialogBox = 0;
    m_pSaveNameBox = 0;
    m_pSaveModuleLabel = 0;
    m_pSaveButton = 0;
    m_pSaveCancel = 0;
    m_pChangesDialogBox = 0;
    m_pChangesNameLabel = 0;
    m_pChangesYesButton = 0;
    m_pChangesNoButton = 0;
    m_pOverwriteDialogBox = 0;
    m_pOverwriteNameLabel = 0;
    m_pOverwriteYesButton = 0;
    m_pOverwriteNoButton = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the EditorActivity object ready for use.

int EditorActivity::Create()
{
    if (Activity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a EditorActivity to be identical to another, by deep copy.

int EditorActivity::Create(const EditorActivity &reference)
{
    if (Activity::Create(reference) < 0)
        return -1;

//    m_Description = "No description defined for this Activity!";

    m_EditorMode = reference.m_EditorMode;
    m_ModuleSpaceID = reference.m_ModuleSpaceID;
    m_NeedSave = reference.m_NeedSave;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int EditorActivity::ReadProperty(const std::string_view &propName, Reader &reader)
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
        return Activity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this EditorActivity with a Writer for
//                  later recreation with Create(Reader &reader);

int EditorActivity::Save(Writer &writer) const {
	Activity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the EditorActivity object.

void EditorActivity::Destroy(bool notInherited)
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    if (!notInherited)
        Activity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int EditorActivity::Start()
{
    int error = 0;
// Don't, too different
//    Activity::Start();

    // Load the scene now
    error = g_SceneMan.LoadScene();
    if (error < 0)
        return error;

    // Clear the post effects
	g_PostProcessMan.ClearScenePostEffects();

    // Clear the screen messages
    g_FrameMan.ClearScreenText();

    // Reset the mousemoving so that it won't trap the mouse if the window isn't in focus (common after loading)
    g_UInputMan.DisableMouseMoving(true);
    g_UInputMan.DisableMouseMoving(false);

    m_ActivityState = ActivityState::Editing;
    m_Paused = true;
//    g_TimerMan.PauseSim(true);
    m_ModeChange = true;

    // Play editing music
    g_AudioMan.ClearMusicQueue();
    g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg");

    // Force the split screen config to just be one big screen for editing
    g_FrameMan.ResetSplitScreens(false, false);

    ///////////////////////////
    // GUI manager setup

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(-1, true); 
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins", "DefaultSkin.ini")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/DefaultSkin.ini");
	}

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void EditorActivity::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void EditorActivity::End()
{
    Activity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this EditorActivity. Supposed to be done every frame
//                  before drawing.

void EditorActivity::Update()
{
    // Always show teh messages of the editor
    m_MessageTimer[0].Reset();

    Activity::Update();

    if (!g_SceneMan.GetScene())
        return;

    // Hide all dialog boxes if mode changes, one will reappear below
    if (m_ModeChange)
    {
		if (m_pNewDialogBox)
			m_pNewDialogBox->SetVisible(false);
        m_pLoadDialogBox->SetVisible(false);
        m_pSaveDialogBox->SetVisible(false);
        m_pChangesDialogBox->SetVisible(false);
        m_pOverwriteDialogBox->SetVisible(false);
    }

    if (m_EditorMode == NEWDIALOG)
    {
        if (m_ModeChange)
        {
            if (m_NeedSave)
            {
                m_PreviousMode = NEWDIALOG;
                m_EditorMode = CHANGESDIALOG;
                m_ModeChange = true;
            }
            else
            {
                UpdateNewDialog();
                // Set the mouse cursor free
                m_pGUIController->EnableMouse(true);
                g_UInputMan.TrapMousePos(false, 0);
                // Show/hide relevant GUI elements
                m_pNewDialogBox->SetVisible(true);
                m_ModeChange = false;
            }
        }
    }
    else if (m_EditorMode == LOADDIALOG)
    {
        if (m_ModeChange)
        {
            if (m_NeedSave)
            {
                m_PreviousMode = LOADDIALOG;
                m_EditorMode = CHANGESDIALOG;
                m_ModeChange = true;
            }
            else
            {
                UpdateLoadDialog();
                // Set the mouse cursor free
                m_pGUIController->EnableMouse(true);
                g_UInputMan.TrapMousePos(false, 0);
                // Show/hide relevant GUI elements
                m_pLoadDialogBox->SetVisible(true);
                m_ModeChange = false;
            }
        }
    }
    else if (m_EditorMode == SAVEDIALOG)
    {
        if (m_ModeChange)
        {
            UpdateSaveDialog();
            // Temporarily disable the UInput from reading regular keypresses so we can type
            g_UInputMan.DisableKeys(true);
            // Set the mouse cursor free
            m_pGUIController->EnableMouse(true);
            g_UInputMan.TrapMousePos(false, 0);
            // Show/hide relevant GUI elements
            m_pSaveDialogBox->SetVisible(true);
            m_ModeChange = false;
        }
    }
    else if (m_EditorMode == CHANGESDIALOG)
    {
        if (m_ModeChange)
        {
            UpdateChangesDialog();
            // Set the mouse cursor free
            m_pGUIController->EnableMouse(true);
            g_UInputMan.TrapMousePos(false, 0);
            // Show/hide relevant GUI elements
            m_pChangesDialogBox->SetVisible(true);
            m_ModeChange = false;
        }
    }
    else if (m_EditorMode == OVERWRITEDIALOG)
    {
        if (m_ModeChange)
        {
            UpdateOverwriteDialog();
            // Set the mouse cursor free
            m_pGUIController->EnableMouse(true);
            g_UInputMan.TrapMousePos(false, 0);
            // Show/hide relevant GUI elements
            m_pOverwriteDialogBox->SetVisible(true);
            m_ModeChange = false;
        }
    }
/* Let this be handled by derived class
    // Quit all dialog boxes
    else
    {
        if (m_ModeChange)
        {
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
*/
    //////////////////////////////////////////
	// Update the ControlManager

	m_pGUIController->Update();

/* Do this in derived classes
    ////////////////////////////////////////////////////////
    // Handle events for mouse input on the controls

	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
        // If we're not supposed to have mouse control, then ignore these messages
// Uh this is not right, editor always has mouse control so far
//        if (!m_PlayerController[0].IsMouseControlled())
//            break;

		if (anEvent.GetType() == GUIEvent::Command)
        {
            //////////////////////////////////////////////////////////
			// NEW button pressed; create a new scene

			if (anEvent.GetControl() == m_pNewButton)
            {
                // Get the selected Module
                GUIListPanel::Item *pItem = m_pNewModuleCombo->GetItem(m_pNewModuleCombo->GetSelectedIndex());
                if (pItem && !pItem->m_Name.empty())
                {
                    m_ModuleSpaceID = g_PresetMan.GetModuleID(pItem->m_Name);

                    // Allocate Scene
                    Scene *pNewScene = new Scene();
                    // Get the selected Terrain and create the Scene using it
                    pItem = m_pNewTerrainCombo->GetItem(m_pNewTerrainCombo->GetSelectedIndex());
                    if (pItem && !pItem->m_Name.empty())
                    {
                        SLTerrain *pNewTerrain = dynamic_cast<SLTerrain *>(g_PresetMan.GetEntityPreset("SLTerrain", pItem->m_Name, m_ModuleSpaceID)->Clone());
                        RTEAssert(pNewTerrain, "No SLTerrain of that name defined!");
                        pNewScene->Create(pNewTerrain);
                    }

                    // Add specified scene layers
                    pItem = m_pNewBG1Combo->GetItem(m_pNewBG1Combo->GetSelectedIndex());
                    if (pItem && !pItem->m_Name.empty())
                    {
                        SceneLayer *pNewLayer = dynamic_cast<SceneLayer *>(g_PresetMan.GetEntityPreset("SceneLayer", pItem->m_Name, m_ModuleSpaceID)->Clone());
                        RTEAssert(pNewLayer, "No SceneLayer of the name set as BG1 is defined!");
                        pNewScene->GetBackLayers().push_back(pNewLayer);
                    }
                    pItem = m_pNewBG2Combo->GetItem(m_pNewBG2Combo->GetSelectedIndex());
                    if (pItem && !pItem->m_Name.empty())
                    {
                        SceneLayer *pNewLayer = dynamic_cast<SceneLayer *>(g_PresetMan.GetEntityPreset("SceneLayer", pItem->m_Name, m_ModuleSpaceID)->Clone());
                        RTEAssert(pNewLayer, "No SceneLayer of the name set as BG2 is defined!");
                        pNewScene->GetBackLayers().push_back(pNewLayer);
                    }
                    pItem = m_pNewBG3Combo->GetItem(m_pNewBG3Combo->GetSelectedIndex());
                    if (pItem && !pItem->m_Name.empty())
                    {
                        SceneLayer *pNewLayer = dynamic_cast<SceneLayer *>(g_PresetMan.GetEntityPreset("SceneLayer", pItem->m_Name, m_ModuleSpaceID)->Clone());
                        RTEAssert(pNewLayer, "No SceneLayer of the name set as BG3 is defined!");
                        pNewScene->GetBackLayers().push_back(pNewLayer);
                    }

                    // Actually load the scene's data and set it up as the current scene
                    g_SceneMan.LoadScene(pNewScene);

                    // Reset the rest of the editor GUI
                    m_pEditorGUI->Destroy();
                    m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
                }

                m_NeedSave = false;
                m_HasEverBeenSaved = false;
                m_EditorMode = m_PreviousMode = EDITINGOBJECT;
                m_ModeChange = true;
            }

            //////////////////////////////////////////////////////////
			// LOAD button pressed; load the selected Scene

			if (anEvent.GetControl() == m_pLoadButton)
            {
                GUIListPanel::Item *pItem = m_pLoadNameCombo->GetItem(m_pLoadNameCombo->GetSelectedIndex());
                if (pItem && !pItem->m_Name.empty())
                {
                    // Attempt to load the scene, without applying its placed objects
                    g_SceneMan.LoadScene(pItem->m_Name, false);
                    // Get the Module ID that the scene exists in, so we can limit the picker to only show objects from that DataModule space
                    if (g_SceneMan.GetScene())
                    {
                        m_ModuleSpaceID = g_SceneMan.GetScene()->GetModuleID();
                        RTEAssert(m_ModuleSpaceID >= 0, "Loaded Scene's DataModule ID is negative? Should always be a specific one..");
                        m_pEditorGUI->Destroy();
                        m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
// TODO: Should read in all the already placed objects in the loaded scene and have them appear int he editor instead
                    }
                }
                m_NeedSave = false;
                m_HasEverBeenSaved = true;
                m_EditorMode = m_PreviousMode = EDITINGOBJECT;
                m_ModeChange = true;
            }

            //////////////////////////////////////////////////////////
			// SAVE button pressed; save the selected Scene

			if (anEvent.GetControl() == m_pSaveButton)
            {
                if (!m_pSaveNameBox->GetText().empty())
                {
                    // Save the scene to the name specified in the text box
                    if (SaveScene(m_pSaveNameBox->GetText()))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the scene
                        m_EditorMode = m_PreviousMode;
                        m_ModeChange = true;
                    }
                    // Should really leave dialog box open?
                    else
                    {
                        ;
                    }
                }
            }

            ///////////////////////////////////////////////////////////////
			// Save Changes YES pressed

			if (anEvent.GetControl() == m_pChangesYesButton)
            {
                if (m_HasEverBeenSaved)
                {
                    if (SaveScene(g_SceneMan.GetScene()->GetPresetName(), true))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the scene
                        m_EditorMode = m_PreviousMode;
                        m_ModeChange = true;
                    }
                }
                // Open the save scene dialog to ask user where to save it then
                else
                {
                    m_PreviousMode = m_PreviousMode;
                    m_EditorMode = SAVEDIALOG;
                    m_ModeChange = true;
                }
            }

            ///////////////////////////////////////////////////////////////
			// Save Changes NO pressed

			if (anEvent.GetControl() == m_pChangesNoButton)
            {
                // Just go back to previous mode
                m_EditorMode = m_PreviousMode;
                m_ModeChange = true;
                m_NeedSave = false;
            }

            ///////////////////////////////////////////////////////////////
			// Overwrite Scene YES pressed

			if (anEvent.GetControl() == m_pOverwriteYesButton)
            {
                // Force overwrite
                if (SaveScene(g_SceneMan.GetScene()->GetPresetName(), true))
                {
                    // Close the dialog box on success
                    m_NeedSave = false;
                    m_HasEverBeenSaved = true;
                    // Go back to previous mode after overwrite dialog is done, may have been on the way to test the scene
                    m_EditorMode = m_PreviousMode != SAVEDIALOG ? m_PreviousMode : EDITINGOBJECT;
                    m_ModeChange = true;
                }
// TODO: Show overwrite error?
            }

            ///////////////////////////////////////////////////////////////
			// Overwrite Scene NO pressed

			if (anEvent.GetControl() == m_pOverwriteNoButton)
            {
                // Just go back to previous mode
                m_EditorMode = m_PreviousMode;
                m_ModeChange = true;
            }

            ///////////////////////////////////////////////////////////////
			// CANCEL button pressed; exit any active dialog box

			if (anEvent.GetControl() == m_pNewCancel || anEvent.GetControl() == m_pLoadCancel || anEvent.GetControl() == m_pSaveCancel)
            {
                m_EditorMode = m_PreviousMode = EDITINGOBJECT;
                m_ModeChange = true;
            }
        }

        // Notifications
        else if (anEvent.GetType() == GUIEvent::Notification)
        {
            ///////////////////////////////////////
            // Clicks on the New Scene Module combo

    		if (anEvent.GetControl() == m_pNewModuleCombo)
            {
                // Closed it, IE selected somehting
                if(anEvent.GetMsg() == GUIComboBox::Closed)
                    UpdateNewDialog();
            }

        }
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void EditorActivity::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    AllegroScreen drawScreen(pTargetBitmap);
    m_pGUIController->Draw(&drawScreen);
    if (m_EditorMode != EDITINGOBJECT)
        m_pGUIController->DrawMouse();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this EditorActivity's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void EditorActivity::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    
}

} // namespace RTE