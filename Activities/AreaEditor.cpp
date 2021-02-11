//////////////////////////////////////////////////////////////////////////////////////////
// File:            AreaEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AreaEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AreaEditor.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "SLTerrain.h"
#include "Controller.h"
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

#include "AreaEditorGUI.h"
#include "PieMenuGUI.h"
#include "GABaseDefense.h"

extern bool g_ResetActivity;

namespace RTE {

ConcreteClassInfo(AreaEditor, EditorActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AreaEditor, effectively
//                  resetting the members of this abstraction level only.

void AreaEditor::Clear()
{
    m_pEditorGUI = 0;
    m_pNewAreaName = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AreaEditor object ready for use.

int AreaEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AreaEditor to be identical to another, by deep copy.

int AreaEditor::Create(const AreaEditor &reference)
{
    if (EditorActivity::Create(reference) < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Define and edit Areas on this Scene.";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int AreaEditor::ReadProperty(const std::string_view &propName, Reader &reader)
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
// Description:     Saves the complete state of this AreaEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int AreaEditor::Save(Writer &writer) const {
	EditorActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AreaEditor object.

void AreaEditor::Destroy(bool notInherited)
{
    delete m_pEditorGUI;

    if (!notInherited)
        EditorActivity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int AreaEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the Editor GUI

    if (m_pEditorGUI)
        m_pEditorGUI->Destroy();
    else
        m_pEditorGUI = new AreaEditorGUI;
    m_pEditorGUI->Create(&(m_PlayerController[0]), true);

    //////////////////////////////////////////////////////////////
    // Hooking up directly to the controls defined in the GUI ini

    m_pGUIController->Load("Base.rte/GUIs/AreaEditorGUI.ini");

    // Resize the invisible root container so it matches the screen rez
    GUICollectionBox *pRootBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"));
    if (pRootBox)
        pRootBox->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    // Make sure we have convenient points to the containing GUI dialog boxes that we will manipulate the positions of
    if (!m_pNewDialogBox)
    {
        m_pNewDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("NewDialogBox"));
//        m_pNewDialogBox->SetDrawType(GUICollectionBox::Color);
        m_pNewDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pNewDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pNewDialogBox->GetHeight() / 2));
        m_pNewDialogBox->SetVisible(false);
    }
    m_pNewAreaName = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NewAreaNameTB"));
    m_pNewButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("NewAreaButton"));
    m_pNewCancel = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("NewCancelButton"));

    // Make sure we have convenient points to the containing GUI dialog boxes that we will manipulate the positions of
    if (!m_pLoadDialogBox)
    {
        m_pLoadDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("LoadDialogBox"));
//        m_pLoadDialogBox->SetDrawType(GUICollectionBox::Color);
        m_pLoadDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pLoadDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pLoadDialogBox->GetHeight() / 2));
        m_pLoadDialogBox->SetVisible(false);
    }
    m_pLoadNameCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("LoadSceneCB"));
    m_pLoadButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadSceneButton"));
    m_pLoadCancel = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadCancelButton"));

    if (!m_pSaveDialogBox)
    {
        m_pSaveDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SaveDialogBox"));

        // Set the background image of the parent collection box
//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.png");
//        m_pSaveDialogBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
//        m_pSaveDialogBox->SetDrawBackground(true);
//        m_pSaveDialogBox->SetDrawType(GUICollectionBox::Image);
//        m_pSaveDialogBox->SetDrawType(GUICollectionBox::Color);
        m_pSaveDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pSaveDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pSaveDialogBox->GetHeight() / 2));
        m_pSaveDialogBox->SetVisible(false);
    }
    m_pSaveNameBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("SaveSceneNameTB"));
    m_pSaveModuleLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SaveModuleLabel"));
    m_pSaveButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SaveSceneButton"));
    m_pSaveCancel = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SaveCancelButton"));

    if (!m_pChangesDialogBox)
    {
        m_pChangesDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ChangesDialogBox"));
        m_pChangesDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pChangesDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pChangesDialogBox->GetHeight() / 2));
        m_pChangesDialogBox->SetVisible(false);
    }
    m_pChangesNameLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesNameLabel"));
    m_pChangesYesButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ChangesYesButton"));
    m_pChangesNoButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ChangesNoButton"));

    if (!m_pOverwriteDialogBox)
    {
        m_pOverwriteDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("OverwriteDialogBox"));
        m_pOverwriteDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pOverwriteDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pOverwriteDialogBox->GetHeight() / 2));
        m_pOverwriteDialogBox->SetVisible(false);
    }
    m_pOverwriteNameLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("OverwriteNameLabel"));
    m_pOverwriteYesButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("OverwriteYesButton"));
    m_pOverwriteNoButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("OverwriteNoButton"));

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void AreaEditor::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void AreaEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this AreaEditor. Supposed to be done every frame
//                  before drawing.

void AreaEditor::Update()
{
    EditorActivity::Update();

    if (!g_SceneMan.GetScene())
        return;

    Scene *pCurrentScene = g_SceneMan.GetScene();

    // Update the loaded objects of the loaded scene so they look right
    pCurrentScene->UpdatePlacedObjects(Scene::PLACEONLOAD);

    // If the scene has no Area:s yet, force the user to make a new one
    if (pCurrentScene->m_AreaList.empty())
        m_EditorMode = EditorActivity::NEWDIALOG;

    // All dialog boxes are gone and we're editing the scene's Area:s
    if (m_EditorMode == EditorActivity::EDITINGOBJECT)
    {
        if (m_ModeChange)
        {
            // Start in the add/move mode
            m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::PREADDMOVEBOX);
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
    // We are doing something int he dialog boxes, so don't do anything in the editor interface or show any text messages
    else
    {
        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::INACTIVE);
        g_FrameMan.ClearScreenText();
    }


    /////////////////////////////////////////////////////
    // Update the editor interface

    m_pEditorGUI->Update();

    // Any edits made, dirtying the scene?
    m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

    // Get any mode change commands that the user gave the Editor GUI
    if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_NEW && m_EditorMode != NEWDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::NEWDIALOG;
        m_ModeChange = true;
        // This is ahack so we don't get a 'save changes dialog' when we jsut want to create a new area.
        // Will turn on dirtyness immediately as New button is pressed below
        m_NeedSave = false;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_LOAD && m_EditorMode != LOADDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_SAVE && m_EditorMode != SAVEDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::SAVEDIALOG;
        m_ModeChange = true;
    }
    // Test the scene by starting a GABaseDefense with it, after saving
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_DONE || m_EditorMode == TESTINGOBJECT)
    {
        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::INACTIVE);

        if (m_NeedSave)
        {
            m_PreviousMode = EditorActivity::TESTINGOBJECT;
            m_EditorMode = EditorActivity::CHANGESDIALOG;
            m_ModeChange = true;
/*
            if (m_HasEverBeenSaved)
                SaveScene(pCurrentScene->GetPresetName());
            else
            {
                m_PreviousMode = TESTINGOBJECT;
                m_EditorMode = SAVEDIALOG;
                m_ModeChange = true;
            }
*/
        }
        else
        {
            g_SceneMan.SetSceneToLoad(pCurrentScene->GetPresetName());

			//Start a scripted 'Skirmish Defense' activity instead of obsolete GABaseDefense because it simply don't work
			const Activity *pActivityPreset = dynamic_cast<const Activity *>(g_PresetMan.GetEntityPreset("GAScripted", "Skirmish Defense"));
			Activity * pActivity = dynamic_cast<Activity *>(pActivityPreset->Clone());
			GameActivity *pTestGame = dynamic_cast<GameActivity *>(pActivity);
            RTEAssert(pTestGame, "Couldn't find the \"Skirmish Defense\" GAScripted Activity! Has it been defined?");
            pTestGame->SetTeamOfPlayer(0, 0);
            pTestGame->SetCPUTeam(1);
			pTestGame->SetStartingGold(10000);
			pTestGame->SetFogOfWarEnabled(false);
            pTestGame->SetDifficulty(DifficultySetting::MediumDifficulty);
            g_ActivityMan.SetStartActivity(pTestGame);
            g_ResetActivity = true;
        }
    }

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
			// NEW button pressed; create a Area in the current scene

			if (anEvent.GetControl() == m_pNewButton)
            {
                // Make sure we're not trying to create a noname area
                if (!m_pNewAreaName->GetText().empty())
                {
                    // Check if name is already taken, and if so, select the taken one instead of creating a new
                    if (Scene::Area *pArea = pCurrentScene->GetArea(m_pNewAreaName->GetText()))
                    {
                        m_pEditorGUI->SetCurrentArea(pArea);
                        m_pEditorGUI->SetEditorGUIMode(AreaEditorGUI::PREADDMOVEBOX);
                    }
                    else
                    {
                        // Make and name new Area
                        Scene::Area newArea(m_pNewAreaName->GetText());
                        pCurrentScene->m_AreaList.push_back(newArea);
                        // Set the new area as the active one in the GUI, note we're getting the correct one from the scene, it's a copy of the one passed in
                        m_pEditorGUI->SetCurrentArea(pCurrentScene->GetArea(newArea.GetName()));
                        // Update teh picker list of the GUI so we can mousewheel between all the Areas, incl the new one
                        m_pEditorGUI->UpdatePickerList(newArea.GetName());
                    }

                    // Change mode to start editing the new/newly selected Area
                    m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                    m_ModeChange = true;
                    m_NeedSave = true;
                }
                else
                {
// TODO: Play some error sound?
                }
            }

            //////////////////////////////////////////////////////////
			// LOAD button pressed; load the selected Scene

			if (anEvent.GetControl() == m_pLoadButton)
            {
                GUIListPanel::Item *pItem = m_pLoadNameCombo->GetItem(m_pLoadNameCombo->GetSelectedIndex());
                if (pItem && !pItem->m_Name.empty())
                {
                    // Attempt to load the scene, without applying its placed objects
                    g_SceneMan.SetSceneToLoad(pItem->m_Name, false);
                    g_SceneMan.LoadScene();
                    // Get the Module ID that the scene exists in, so we can limit the picker to only show objects from that DataModule space
                    m_ModuleSpaceID = g_SceneMan.GetScene()->GetModuleID();
                    if (pCurrentScene)
                    {
                        m_pEditorGUI->Destroy();
                        m_pEditorGUI->Create(&(m_PlayerController[0]), true);
// TODO: Should read in all the already placed objects in the loaded scene and have them appear int he editor instead
                    }
                }
                m_NeedSave = false;
                m_HasEverBeenSaved = true;
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
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
                    if (SaveScene(pCurrentScene->GetPresetName(), true))
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
                    m_EditorMode = EditorActivity::SAVEDIALOG;
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
                if (SaveScene(pCurrentScene->GetPresetName(), true))
                {
                    // Close the dialog box on success
                    m_NeedSave = false;
                    m_HasEverBeenSaved = true;
                    // Go back to previous mode after overwrite dialog is done, may have been on the way to test the scene
                    m_EditorMode = m_PreviousMode != EditorActivity::SAVEDIALOG ? m_PreviousMode : EditorActivity::EDITINGOBJECT;
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
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
            }
        }

        // Notifications
        else if (anEvent.GetType() == GUIEvent::Notification)
        {
/*
            ///////////////////////////////////////
            // Clicks on the New Scene Module combo

    		if (anEvent.GetControl() == m_pNewModuleCombo)
            {
                // Closed it, IE selected somehting
                if(anEvent.GetMsg() == GUIComboBox::Closed)
                    UpdateNewDialog();
            }
*/
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void AreaEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    m_pEditorGUI->Draw(pTargetBitmap, targetPos);

    EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AreaEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void AreaEditor::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current scene to an appropriate ini file, and asks user if
//                  they want to overwrite first if scene of this name exists.

bool AreaEditor::SaveScene(string saveAsName, bool forceOverwrite)
{
    // Set the name of the current scene in effect
    g_SceneMan.GetScene()->SetPresetName(saveAsName);

	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	{
		string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".ini");
		string previewFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".preview.bmp");
		if (g_PresetMan.AddEntityPreset(g_SceneMan.GetScene(), m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
			// Save preview
			g_SceneMan.GetScene()->SavePreview(previewFilePath);

			// Does ini already exist? If yes, then no need to add it to a scenes.ini etc
			bool sceneFileExisted = exists(sceneFilePath.c_str());
			// Create the writer
			Writer sceneWriter(sceneFilePath.c_str(), false);
			sceneWriter.NewProperty("AddScene");
			// Write the scene out to the new ini
			sceneWriter << g_SceneMan.GetScene();
			return m_HasEverBeenSaved = true;
		}
		else
		{
			// Gotto ask if we can overwrite the existing scene
			m_PreviousMode = EditorActivity::SAVEDIALOG;
			m_EditorMode = EditorActivity::OVERWRITEDIALOG;
			m_ModeChange = true;
		}
	}
	else
	{
		// Try to save to the data module
		string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + saveAsName + ".ini");
		string previewFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + saveAsName + ".preview.bmp");
		if (g_PresetMan.AddEntityPreset(g_SceneMan.GetScene(), m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
            // Save preview
            g_SceneMan.GetScene()->SavePreview(previewFilePath);

            // Does ini already exist? If yes, then no need to add it to a scenes.ini etc
            bool sceneFileExisted = exists(sceneFilePath.c_str());
            // Create the writer
            Writer sceneWriter(sceneFilePath.c_str(), false);
            sceneWriter.NewProperty("AddScene");
            // TODO: Check if the ini file already exists, and then ask if overwrite
                    // Write the scene out to the new ini
            sceneWriter << g_SceneMan.GetScene();

            if (!sceneFileExisted)
            {
                // First find/create  a .rte/Scenes.ini file to include the new .ini into
                string scenesFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes.ini");
                bool scenesFileExisted = exists(scenesFilePath.c_str());
                Writer scenesWriter(scenesFilePath.c_str(), true);
                scenesWriter.NewProperty("\nIncludeFile");
                scenesWriter << sceneFilePath;

                // Also add a line to the end of the modules' Index.ini to include the newly created Scenes.ini next startup
                // If it's already included, it doens't matter, the definitions will just bounce the second time
                if (!scenesFileExisted)
                {
                    string indexFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Index.ini");
                    Writer indexWriter(indexFilePath.c_str(), true);
                    // Add extra tab since the DataModule has everything indented
                    indexWriter.NewProperty("\tIncludeFile");
                    indexWriter << scenesFilePath;
                }
            }
			return m_HasEverBeenSaved = true;
		}
		else
		{
			// Gotto ask if we can overwrite the existing scene
			m_PreviousMode = EditorActivity::SAVEDIALOG;
			m_EditorMode = EditorActivity::OVERWRITEDIALOG;
			m_ModeChange = true;
		}
	}

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateNewDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New dialog box, populates its lists etc.

void AreaEditor::UpdateNewDialog()
{
    // Reset the new Area name text field
    if (m_pNewAreaName->GetText().empty())
        m_pNewAreaName->SetText("Test Area 1");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.

void AreaEditor::UpdateLoadDialog()
{
    // Clear out the control
    m_pLoadNameCombo->ClearList();

    // Get the list of all read in scenes
    list<Entity *> sceneList;
    g_PresetMan.GetAllOfType(sceneList, "Scene");

    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr)
    {
		Scene * pScene = dynamic_cast<Scene *>(*itr);
		if (pScene)
        // Don't add the special "Editor Scene" or metascenes, users shouldn't be messing with them
        if (pScene->GetPresetName() != "Editor Scene" && !pScene->IsMetagameInternal() && (pScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes()))
            m_pLoadNameCombo->AddItem(pScene->GetPresetName());
    }

    // Select the first one
    m_pLoadNameCombo->SetSelectedIndex(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateSaveDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save dialog box, populates its lists etc.

void AreaEditor::UpdateSaveDialog()
{
    m_pSaveNameBox->SetText((g_SceneMan.GetScene()->GetPresetName() == "None" || !m_HasEverBeenSaved) ? "New Scene" : g_SceneMan.GetScene()->GetPresetName());
	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
		m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/");
	else
		m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChangesDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save Changes dialog box, populates its lists etc.

void AreaEditor::UpdateChangesDialog()
{
    if (m_HasEverBeenSaved)
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Do you want to save your changes to:");
		if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
			m_pChangesNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
		else
			m_pChangesNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
    }
    else
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Save your new Scene first?");
        m_pChangesNameLabel->SetText("");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateOverwriteDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Overwrite dialog box, populates its lists etc.

void AreaEditor::UpdateOverwriteDialog()
{
	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
		m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
	else
		m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
}

} // namespace RTE