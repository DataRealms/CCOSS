//////////////////////////////////////////////////////////////////////////////////////////
// File:            SchemeEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SchemeEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SchemeEditor.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "LicenseMan.h"
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

#include "SchemeEditorGUI.h"
#include "PieMenuGUI.h"
#include "GABaseDefense.h"

extern bool g_ResetActivity;
extern bool g_InActivity;

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(SchemeEditor, EditorActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SchemeEditor, effectively
//                  resetting the members of this abstraction level only.

void SchemeEditor::Clear()
{
    m_pEditorGUI = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SchemeEditor object ready for use.

int SchemeEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SchemeEditor to be identical to another, by deep copy.

int SchemeEditor::Create(const SchemeEditor &reference)
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

int SchemeEditor::ReadProperty(std::string propName, Reader &reader)
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
        return EditorActivity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SchemeEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int SchemeEditor::Save(Writer &writer) const
{
    EditorActivity::Save(writer);
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
// Description:     Destroys and resets (through Clear()) the SchemeEditor object.

void SchemeEditor::Destroy(bool notInherited)
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

int SchemeEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the Editor GUI

    if (m_pEditorGUI)
        m_pEditorGUI->Destroy();
    else
        m_pEditorGUI = new SchemeEditorGUI;
    m_pEditorGUI->Create(&(m_PlayerController[0]), SchemeEditorGUI::ONLOADEDIT);

    //////////////////////////////////////////////////////////////
    // Hooking up directly to the controls defined in the GUI ini

    m_pGUIController->Load("Base.rte/GUIs/SchemeEditorGUI.ini");

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
    m_pNewModuleCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewModuleCB"));
	m_pNewModuleCombo->SetEnabled(false);
    m_pNewButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("NewSceneButton"));
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
    m_pLoadToNewButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadToNewButton"));
    m_pLoadButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadSceneButton"));
    m_pLoadCancel = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadCancelButton"));

    if (!m_pSaveDialogBox)
    {
        m_pSaveDialogBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SaveDialogBox"));

        // Set the background image of the parent collection box
//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.bmp");
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

void SchemeEditor::Pause(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void SchemeEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = OVER;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SchemeEditor. Supposed to be done every frame
//                  before drawing.

void SchemeEditor::Update()
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
            m_pEditorGUI->SetEditorGUIMode(m_pEditorGUI->GetCurrentObject() ? SchemeEditorGUI::ADDINGOBJECT : SchemeEditorGUI::PICKINGOBJECT);
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
    // We are doing something int he dialog boxes, so don't do anything in the editor interface
    else
        m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);


    /////////////////////////////////////////////////////
    // Update the editor interface

    m_pEditorGUI->Update();

    // Any edits made, dirtying the scene?
    m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

    // Get any mode change commands that the user gave the Editor GUI
    if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_NEW && m_EditorMode != NEWDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::NEWDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_LOAD && m_EditorMode != LOADDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_SAVE && m_EditorMode != SAVEDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::SAVEDIALOG;
        m_ModeChange = true;
    }
    // Test the scene by starting a GABaseDefense with it, after saving
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieMenuGUI::PSI_DONE || m_EditorMode == TESTINGOBJECT)
    {
        m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);

        if (m_NeedSave)
        {
            m_PreviousMode = EditorActivity::TESTINGOBJECT;
            m_EditorMode = EditorActivity::CHANGESDIALOG;
            m_ModeChange = true;
/*
            if (m_HasEverBeenSaved)
                SaveScene(g_SceneMan.GetScene()->GetPresetName());
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
            g_SceneMan.SetSceneToLoad(g_SceneMan.GetScene()->GetPresetName());

			//Start a scripted 'Skirmish Defense' activity instead of obsolete GABaseDefense because it simply don't work
			const Activity *pActivityPreset = dynamic_cast<const Activity *>(g_PresetMan.GetEntityPreset("GAScripted", "Skirmish Defense"));
			Activity * pActivity = dynamic_cast<Activity *>(pActivityPreset->Clone());
			GameActivity *pTestGame = dynamic_cast<GameActivity *>(pActivity);
            AAssert(pTestGame, "Couldn't find the \"Skirmish Defense\" GAScripted Activity! Has it been defined?");
            pTestGame->SetPlayerCount(1);
            pTestGame->SetTeamCount(2);
            pTestGame->SetTeamOfPlayer(0, 0);
            pTestGame->SetCPUTeam(1);
			pTestGame->SetStartingGold(10000);
			pTestGame->SetFogOfWarEnabled(false);
            pTestGame->SetDifficulty(GameActivity::MEDIUMDIFFICULTY);
            g_ActivityMan.SetStartActivity(pTestGame);
            g_ResetActivity = true;


            /*GABaseDefense *pTestGame = dynamic_cast<GABaseDefense *>(g_PresetMan.GetEntityPreset("GABaseDefense", "Test Activity")->Clone());
            AAssert(pTestGame, "Couldn't find the \"Skirmish Defense\" GABaseDefense Activity! Has it been defined?");
            pTestGame->SetPlayerCount(1);
            pTestGame->SetTeamCount(2);
            pTestGame->SetTeamOfPlayer(0, 0);
            pTestGame->SetCPUTeam(1);
            pTestGame->SetDifficulty(GameActivity::MAXDIFFICULTY);
            pTestGame->Create();
            g_ActivityMan.SetStartActivity(pTestGame);
            g_ResetActivity = true;*/
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

					// Make random planet coord's for this scene
					float angle = RangeRand(0, 2 * PI);
                    Vector pos = Vector((int)(150 * cos(angle)), (int)(150 * sin(angle)));
					pNewScene->SetLocation(pos);

                    // Actually load the scene's data and set it up as the current scene
                    g_SceneMan.LoadScene(pNewScene);

                    // Reset the rest of the editor GUI
                    m_pEditorGUI->Destroy();
					if (m_ModuleSpaceID == g_PresetMan.GetModuleID("Scenes.rte"))
	                    m_pEditorGUI->Create(&(m_PlayerController[0]), SchemeEditorGUI::ONLOADEDIT, -1);
					else
	                    m_pEditorGUI->Create(&(m_PlayerController[0]), SchemeEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
                }

                m_NeedSave = false;
                m_HasEverBeenSaved = false;
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
            }

            //////////////////////////////////////////////////////////
			// LOAD TO NEW button pressed; go from the load to the new dialog

			if (anEvent.GetControl() == m_pLoadToNewButton)
            {
                m_pEditorGUI->SetEditorGUIMode(SchemeEditorGUI::INACTIVE);
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
                        m_ModuleSpaceID = g_SceneMan.GetScene()->GetModuleID();
                        AAssert(m_ModuleSpaceID >= 0, "Loaded Scene's DataModule ID is negative? Should always be a specific one..");
                        m_pEditorGUI->Destroy();
						if (m_ModuleSpaceID == g_PresetMan.GetModuleID("Scenes.rte"))
							m_pEditorGUI->Create(&(m_PlayerController[0]), SchemeEditorGUI::ONLOADEDIT, -1);
						else
							m_pEditorGUI->Create(&(m_PlayerController[0]), SchemeEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
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
                if (SaveScene(g_SceneMan.GetScene()->GetPresetName(), true))
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
                // Don't allow canceling out of diags if we're still in the special "Editor Scene", don't allow users to edit it!
                // Just exit the whole editor into the main menu
                if (g_SceneMan.GetScene()->GetPresetName() == "Editor Scene")
                {
                    g_ActivityMan.PauseActivity();
                    g_InActivity = false;
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

void SchemeEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    m_pEditorGUI->Draw(pTargetBitmap, targetPos);

    EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SchemeEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void SchemeEditor::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current scene to an appropriate ini file, and asks user if
//                  they want to overwrite first if scene of this name exists.

bool SchemeEditor::SaveScene(string saveAsName, bool forceOverwrite)
{
    // Set the name of the current scene in effect
    g_SceneMan.GetScene()->SetPresetName(saveAsName);

	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	{
		string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".ini");
		if (g_PresetMan.AddEntityPreset(g_SceneMan.GetScene(), m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
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
		if (g_PresetMan.AddEntityPreset(g_SceneMan.GetScene(), m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		{
			if (g_LicenseMan.HasValidatedLicense())
			{
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
					// First find/create a .rte/Scenes.ini file to include the new .ini into
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

void SchemeEditor::UpdateNewDialog()
{
    int scenesIndex = 0;
	
	// Only refill modules if empty
    if (m_pNewModuleCombo->GetCount() <= 0)
    {
        for (int module = 0; module < g_PresetMan.GetTotalModuleCount(); ++module)
		{
            m_pNewModuleCombo->AddItem(g_PresetMan.GetDataModule(module)->GetFileName());

			if (g_PresetMan.GetDataModule(module)->GetFileName() == "Scenes.rte")
				scenesIndex = m_pNewModuleCombo->GetCount() - 1;
		}

        // Select the "Scenes.rte" module
        m_pNewModuleCombo->SetSelectedIndex(scenesIndex);
    }

    // Get the ID of the module currently selected so we can limit the following boxes to only show stuff in that module
    int selectedModuleID = -1;
    GUIListPanel::Item *pItem = m_pNewModuleCombo->GetItem(m_pNewModuleCombo->GetSelectedIndex());
    if (pItem && !pItem->m_Name.empty())
        selectedModuleID = g_PresetMan.GetModuleID(pItem->m_Name);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.

void SchemeEditor::UpdateLoadDialog()
{
    // Clear out the control
    m_pLoadNameCombo->ClearList();

    // Get the list of all read in scenes
    list<Entity *> sceneList;
    g_PresetMan.GetAllOfType(sceneList, "Scene");

    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr)
    {
        // Don't add the special "Editor Scene", users shouldn't be messing with it
        if ((*itr)->GetPresetName() != "Editor Scene")
            m_pLoadNameCombo->AddItem((*itr)->GetPresetName());
    }

    // Select the first one
    m_pLoadNameCombo->SetSelectedIndex(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateSaveDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save dialog box, populates its lists etc.

void SchemeEditor::UpdateSaveDialog()
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

void SchemeEditor::UpdateChangesDialog()
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

void SchemeEditor::UpdateOverwriteDialog()
{
	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	    m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
	else
	    m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
}

} // namespace RTE
