//////////////////////////////////////////////////////////////////////////////////////////
// File:            AssemblyEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AssemblyEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AssemblyEditor.h"
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

#include "AssemblyEditorGUI.h"
#include "BunkerAssembly.h"
#include "BunkerAssemblyScheme.h"

namespace RTE {

ConcreteClassInfo(AssemblyEditor, EditorActivity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AssemblyEditor, effectively
//                  resetting the members of this abstraction level only.

void AssemblyEditor::Clear()
{
    m_pEditorGUI = 0;
	m_pModuleCombo = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AssemblyEditor object ready for use.

int AssemblyEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AssemblyEditor to be identical to another, by deep copy.

int AssemblyEditor::Create(const AssemblyEditor &reference)
{
    if (EditorActivity::Create(reference) < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Edit this Scene, including placement of all terrain objects and movable objects, AI blueprints, etc.";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int AssemblyEditor::ReadProperty(const std::string_view &propName, Reader &reader)
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
// Description:     Saves the complete state of this AssemblyEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int AssemblyEditor::Save(Writer &writer) const {
	EditorActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AssemblyEditor object.

void AssemblyEditor::Destroy(bool notInherited)
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

int AssemblyEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the Editor GUI

    if (m_pEditorGUI)
        m_pEditorGUI->Destroy();
    else
        m_pEditorGUI = new AssemblyEditorGUI;
    m_pEditorGUI->Create(&(m_PlayerController[0]), AssemblyEditorGUI::ONLOADEDIT);

    //////////////////////////////////////////////////////////////
    // Hooking up directly to the controls defined in the GUI ini

    m_pGUIController->Load("Base.rte/GUIs/AssemblyEditorGUI.ini");

    // Resize the invisible root container so it matches the screen rez
    GUICollectionBox *pRootBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"));
    if (pRootBox)
        pRootBox->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    m_pModuleCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ModuleCB"));

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

void AssemblyEditor::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void AssemblyEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this AssemblyEditor. Supposed to be done every frame
//                  before drawing.

void AssemblyEditor::Update()
{
    EditorActivity::Update();

    if (!g_SceneMan.GetScene())
        return;

    // Update the loaded objects of the loaded scene so they look right
    g_SceneMan.GetScene()->UpdatePlacedObjects(Scene::PLACEONLOAD);

    // All dialog boxes are gone and we're editing the scene
    if (m_EditorMode == EditorActivity::EDITINGOBJECT)
    {
        if (m_ModeChange)
        {
            // Open the picker depending on whetehr there's somehting in the cursor hand or not
            m_pEditorGUI->SetEditorGUIMode(m_pEditorGUI->GetCurrentObject() ? AssemblyEditorGUI::ADDINGOBJECT : AssemblyEditorGUI::PICKINGOBJECT);
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
    // We are doing something int he dialog boxes, so don't do anything in the editor interface
    else
        m_pEditorGUI->SetEditorGUIMode(AssemblyEditorGUI::INACTIVE);


    /////////////////////////////////////////////////////
    // Update the editor interface

    m_pEditorGUI->Update();

    // Any edits made, dirtying the scene?
    m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

    // Get any mode change commands that the user gave the Editor GUI
    if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_NEW && m_EditorMode != NEWDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AssemblyEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::NEWDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_LOAD && m_EditorMode != LOADDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AssemblyEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_SAVE && m_EditorMode != SAVEDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(AssemblyEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::SAVEDIALOG;
        m_ModeChange = true;
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
			// LOAD TO NEW button pressed; go from the load to the new dialog

			if (anEvent.GetControl() == m_pLoadToNewButton)
            {
                m_pEditorGUI->SetEditorGUIMode(AssemblyEditorGUI::INACTIVE);
                m_EditorMode = EditorActivity::NEWDIALOG;
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
                    g_SceneMan.SetSceneToLoad(pItem->m_Name, false);
                    g_SceneMan.LoadScene();
                    // Get the Module ID that the scene exists in, so we can limit the picker to only show objects from that DataModule space
                    if (g_SceneMan.GetScene())
                    {
                        //m_ModuleSpaceID = g_SceneMan.GetScene()->GetModuleID();
						m_ModuleSpaceID = g_PresetMan.GetModuleID(m_pModuleCombo->GetSelectedItem()->m_Name);
                        RTEAssert(m_ModuleSpaceID >= 0, "Loaded Scene's DataModule ID is negative? Should always be a specific one..");
                        m_pEditorGUI->Destroy();
						if (m_ModuleSpaceID == g_PresetMan.GetModuleID("Scenes.rte"))
							m_pEditorGUI->Create(&(m_PlayerController[0]), AssemblyEditorGUI::ONLOADEDIT, -1);
						else
							m_pEditorGUI->Create(&(m_PlayerController[0]), AssemblyEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
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
                    if (SaveAssembly(m_pSaveNameBox->GetText()))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the scene
                        m_EditorMode = EditorActivity::EDITINGOBJECT;
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
					if (SaveAssembly(m_pEditorGUI->GetCurrentAssemblyName(), true))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the scene
                        m_EditorMode = EditorActivity::EDITINGOBJECT;
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
				if (SaveAssembly(m_pEditorGUI->GetCurrentAssemblyName(), true))
                {
                    // Close the dialog box on success
                    m_NeedSave = false;
                    m_HasEverBeenSaved = true;
                    // Go back to previous mode after overwrite dialog is done, may have been on the way to test the scene
                    m_EditorMode = EditorActivity::EDITINGOBJECT;
                    m_ModeChange = true;
                }
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
                // Don't allow canceling out of diags if we're still in the special "Editor Scene", don't allow users to edit it!
                // Just exit the whole editor into the main menu
                if (g_SceneMan.GetScene()->GetPresetName() == "Editor Scene")
                {
                    g_ActivityMan.PauseActivity();
                }
                // Just do normal cancel of the dialog and go back to editing
                else
                    m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;

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
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void AssemblyEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    m_pEditorGUI->Draw(pTargetBitmap, targetPos);

    EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AssemblyEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void AssemblyEditor::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);    
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildAssembly
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and builds assembly which fits currently selected scheme and returns
//					it's pointer. Owhership IS transfered.

BunkerAssembly * AssemblyEditor::BuildAssembly(string saveAsName)
{
	// Create new bunker assembly to save
	BunkerAssembly *pBA = new BunkerAssembly();
	pBA->Create(m_pEditorGUI->GetCurrentAssemblyScheme());

	// Retreive some properties if we're overwriting existing bunker assembly
	const BunkerAssembly * pExistingBA = dynamic_cast<const BunkerAssembly *>(g_PresetMan.GetEntityPreset("BunkerAssembly", saveAsName, m_ModuleSpaceID));
	if (pExistingBA)
	{
		pBA->SetSymmetricAssemblyName(pExistingBA->GetSymmetricAssemblyName());
	}

	pBA->SetPresetName(saveAsName);
	m_pEditorGUI->SetCurrentAssemblyName(saveAsName);

    const std::list<SceneObject *> *pSceneObjectList = 0;
	pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);
    for (list<SceneObject *>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr)
    {
		//Check if object fits the assembly box
		bool skip = true;

		Vector pos = (*itr)->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset();
		Vector finalPos = pos;

		if ((pos.m_X >= 0) && (pos.m_X < pBA->GetBitmapWidth()) && 
			(pos.m_Y >= 0) && (pos.m_Y < pBA->GetBitmapHeight()))
			skip = false;
		
		// Try to move scene object across seams and see if it fits into assembly box
		if (g_SceneMan.GetScene()->WrapsX())
		{
			pos = (*itr)->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset() + Vector(g_SceneMan.GetScene()->GetWidth(), 0);

			if ((pos.m_X >= 0) && (pos.m_X < pBA->GetBitmapWidth()) && 
				(pos.m_Y >= 0) && (pos.m_Y < pBA->GetBitmapHeight()))
			{			
				skip = false;
				finalPos = pos;
			}

			pos = (*itr)->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset() - Vector(g_SceneMan.GetScene()->GetWidth(), 0);

			if ((pos.m_X >= 0) && (pos.m_X < pBA->GetBitmapWidth()) && 
				(pos.m_Y >= 0) && (pos.m_Y < pBA->GetBitmapHeight()))
			{			
				skip = false;
				finalPos = pos;
			}
		}

		if (g_SceneMan.GetScene()->WrapsY())
		{
			pos = (*itr)->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset() + Vector(0, g_SceneMan.GetScene()->GetHeight());

			if ((pos.m_X >= 0) && (pos.m_X < pBA->GetBitmapWidth()) && 
				(pos.m_Y >= 0) && (pos.m_Y < pBA->GetBitmapHeight()))
			{			
				skip = false;
				finalPos = pos;
			}

			pos = (*itr)->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset() - Vector(0, g_SceneMan.GetScene()->GetHeight());

			if ((pos.m_X >= 0) && (pos.m_X < pBA->GetBitmapWidth()) && 
				(pos.m_Y >= 0) && (pos.m_Y < pBA->GetBitmapHeight()))
			{			
				skip = false;
				finalPos = pos;
			}
		}
	
		if (!skip)
		{
			SceneObject *pNewSO = dynamic_cast<SceneObject *>((*itr)->Clone());

			//Set position relative to this Bunker Assembly
			//pNewSO->SetPos(pNewSO->GetPos() - pBA->GetPos() - pBA->GetBitmapOffset());
			pNewSO->SetPos(finalPos);
			pBA->AddPlacedObject(pNewSO);
		}
	}

	return pBA;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveAssembly
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current scene to an appropriate ini file, and asks user if
//                  they want to overwrite first if scene of this name exists.

bool AssemblyEditor::SaveAssembly(string saveAsName, bool forceOverwrite)
{
	BunkerAssembly *pBA = BuildAssembly(saveAsName);

	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	{
		string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".ini");
		if (g_PresetMan.AddEntityPreset(pBA, m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
			// Does ini already exist? If yes, then no need to add it to a scenes.ini etc
			bool sceneFileExisted = exists(sceneFilePath.c_str());
			// Create the writer
			Writer sceneWriter(sceneFilePath.c_str(), false);
			sceneWriter.NewProperty("AddBunkerAssembly");
			// Write the scene out to the new ini
			sceneWriter << pBA;
			delete pBA;
			pBA = 0;
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
		string sceneFilePath;

		// Try to save to the data module
		if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Base.rte")
			sceneFilePath = g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/Objects/Bunkers/BunkerAssemblies/" + saveAsName + ".ini";
		else
		{
			sceneFilePath = g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/BunkerAssemblies/" + saveAsName + ".ini";
			System::MakeDirectory((g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/BunkerAssemblies").c_str());
		}

		if (g_PresetMan.AddEntityPreset(pBA, m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
			// Does ini already exist? If yes, then no need to add it to a scenes.ini etc
			bool sceneFileExisted = exists(sceneFilePath.c_str());
			// Create the writer
			Writer sceneWriter(sceneFilePath.c_str(), false);
			sceneWriter.NewProperty("AddBunkerAssembly");
			// Write the scene out to the new ini
			BunkerAssembly *pBA = BuildAssembly(saveAsName);
			sceneWriter << pBA;
			delete pBA;
			pBA = 0;

			if (!sceneFileExisted)
			{
				// First find/create a .rte/Scenes.ini file to include the new .ini into
				string scenesFilePath;

				if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Base.rte")
					scenesFilePath = g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/Objects/Bunkers/BunkerAssemblies/BunkerAssemblies.ini";
				else
					scenesFilePath = g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/BunkerAssemblies/BunkerAssemblies.ini";

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

	delete pBA;
	pBA = 0;
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateNewDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

void AssemblyEditor::UpdateNewDialog()
{

}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.

void AssemblyEditor::UpdateLoadDialog()
{
	int scenesIndex = 0;

    if (m_pModuleCombo->GetCount() <= 0)
    {
        for (int module = 0; module < g_PresetMan.GetTotalModuleCount(); ++module)
		{
			// Cut-off vanilla modules except Base.rte
			bool isValid = false;

			// If metascenes are visible then allow to save assemblies to Base.rte
			if (g_SettingsMan.ShowMetascenes())
			{
				if ((module == 0 || module > 8) && g_PresetMan.GetDataModule(module)->GetFileName() != "Metagames.rte"
												&& g_PresetMan.GetDataModule(module)->GetFileName() != "Missions.rte")
					isValid = true;
			} else {
				if (module > 8 && g_PresetMan.GetDataModule(module)->GetFileName() != "Metagames.rte"
						       && g_PresetMan.GetDataModule(module)->GetFileName() != "Missions.rte")
					isValid = true;
			}

			// If Saving to Base.rte is enabled, every module is valid for saving
			if (g_SettingsMan.AllowSavingToBase())
				isValid = true;

			if (isValid)
			{
				m_pModuleCombo->AddItem(g_PresetMan.GetDataModule(module)->GetFileName());

				if (g_SettingsMan.AllowSavingToBase())
				{
					// If editors are in dev-mode then select Base.rte as default module to save stuff
					if (g_PresetMan.GetDataModule(module)->GetFileName() == "Base.rte")
						scenesIndex = m_pModuleCombo->GetCount() - 1;
				}
				else
				{
					if (g_PresetMan.GetDataModule(module)->GetFileName() == "Scenes.rte")
						scenesIndex = m_pModuleCombo->GetCount() - 1;
				}
			}
		}

        // Select the "Scenes.rte" module
        m_pModuleCombo->SetSelectedIndex(scenesIndex);
    }

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

void AssemblyEditor::UpdateSaveDialog()
{
	string defaultName = "";

	BunkerAssemblyScheme *pScheme = m_pEditorGUI->GetCurrentAssemblyScheme();
	if (pScheme)
		defaultName = pScheme->GetPresetName() + " - ";

    m_pSaveNameBox->SetText(m_pEditorGUI->GetCurrentAssemblyName() == "" ? defaultName : m_pEditorGUI->GetCurrentAssemblyName());
	m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChangesDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save Changes dialog box, populates its lists etc.

void AssemblyEditor::UpdateChangesDialog()
{
    if (m_HasEverBeenSaved)
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Do you want to save your changes to:");
        m_pChangesNameLabel->SetText(m_pEditorGUI->GetCurrentAssemblyName());
    }
    else
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Save your new Assembly first?");
        m_pChangesNameLabel->SetText(m_pEditorGUI->GetCurrentAssemblyName());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateOverwriteDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Overwrite dialog box, populates its lists etc.

void AssemblyEditor::UpdateOverwriteDialog()
{
	m_pOverwriteNameLabel->SetText(m_pEditorGUI->GetCurrentAssemblyName());
}

} // namespace RTE