//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SceneEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SceneEditor.h"
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

#include "SceneEditorGUI.h"
#include "GABaseDefense.h"

namespace RTE {

ConcreteClassInfo(SceneEditor, EditorActivity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneEditor, effectively
//                  resetting the members of this abstraction level only.

void SceneEditor::Clear()
{
    m_pEditorGUI = 0;
    m_pNewTerrainCombo = 0;
    m_pNewBG1Combo = 0;
    m_pNewBG2Combo = 0;
    m_pNewBG3Combo = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneEditor object ready for use.

int SceneEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SceneEditor to be identical to another, by deep copy.

int SceneEditor::Create(const SceneEditor &reference)
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

int SceneEditor::ReadProperty(const std::string_view &propName, Reader &reader)
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
// Description:     Saves the complete state of this SceneEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int SceneEditor::Save(Writer &writer) const {
	EditorActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneEditor object.

void SceneEditor::Destroy(bool notInherited)
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

int SceneEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the Editor GUI

    if (m_pEditorGUI)
        m_pEditorGUI->Destroy();
    else
        m_pEditorGUI = new SceneEditorGUI;
    m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT);

    //////////////////////////////////////////////////////////////
    // Hooking up directly to the controls defined in the GUI ini

    m_pGUIController->Load("Base.rte/GUIs/SceneEditorGUI.ini");

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
	if (g_SettingsMan.AllowSavingToBase())
		m_pNewModuleCombo->SetEnabled(true);
	else
		m_pNewModuleCombo->SetEnabled(false);
    m_pNewTerrainCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewTerrainCB"));
    m_pNewBG1Combo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewBG1CB"));
    m_pNewBG2Combo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewBG2CB"));
    m_pNewBG3Combo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewBG3CB"));
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

void SceneEditor::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void SceneEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SceneEditor. Supposed to be done every frame
//                  before drawing.

void SceneEditor::Update()
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
            m_pEditorGUI->SetEditorGUIMode(m_pEditorGUI->GetCurrentObject() ? SceneEditorGUI::ADDINGOBJECT : SceneEditorGUI::PICKINGOBJECT);
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
    // We are doing something int he dialog boxes, so don't do anything in the editor interface
    else
        m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);


    /////////////////////////////////////////////////////
    // Update the editor interface

    m_pEditorGUI->Update();

    // Any edits made, dirtying the scene?
    m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

    // Get any mode change commands that the user gave the Editor GUI
    if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_NEW && m_EditorMode != NEWDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::NEWDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_LOAD && m_EditorMode != LOADDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_SAVE && m_EditorMode != SAVEDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::SAVEDIALOG;
        m_ModeChange = true;
    }
    // Test the scene by starting a GABaseDefense with it, after saving
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_DONE || m_EditorMode == TESTINGOBJECT)
    {
        m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);

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
			g_SceneMan.SetSceneToLoad(g_SceneMan.GetScene()->GetPresetName(), Scene::PLACEONLOAD);

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
			g_ActivityMan.SetRestartActivity();


            /*GABaseDefense *pTestGame = dynamic_cast<GABaseDefense *>(g_PresetMan.GetEntityPreset("GABaseDefense", "Test Activity")->Clone());
            RTEAssert(pTestGame, "Couldn't find the \"Skirmish Defense\" GABaseDefense Activity! Has it been defined?");
            pTestGame->SetTeamOfPlayer(0, 0);
            pTestGame->SetCPUTeam(1);
            pTestGame->SetDifficulty(GameActivity::MaxDifficulty);
            pTestGame->Create();
            g_ActivityMan.SetStartActivity(pTestGame);
            g_ActivityMan.SetRestartActivity();*/
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

					// Make random planet coord's for this scene
					float angle = RandomNum(0.0F, c_TwoPI);
                    Vector pos = Vector((int)(150 * cos(angle)), (int)(150 * sin(angle)));
					pNewScene->SetLocation(pos);

                    // Actually load the scene's data and set it up as the current scene
                    g_SceneMan.LoadScene(pNewScene, Scene::PLACEONLOAD);

                    // Reset the rest of the editor GUI
                    m_pEditorGUI->Destroy();
					if (m_ModuleSpaceID == g_PresetMan.GetModuleID("Scenes.rte"))
	                    m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, -1);
					else
	                    m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
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
                m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
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
                        RTEAssert(m_ModuleSpaceID >= 0, "Loaded Scene's DataModule ID is negative? Should always be a specific one..");
                        m_pEditorGUI->Destroy();
						if (m_ModuleSpaceID == g_PresetMan.GetModuleID("Scenes.rte"))
							m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, -1);
						else
							m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
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

void SceneEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    m_pEditorGUI->Draw(pTargetBitmap, targetPos);

    EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void SceneEditor::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current scene to an appropriate ini file, and asks user if
//                  they want to overwrite first if scene of this name exists.

bool SceneEditor::SaveScene(string saveAsName, bool forceOverwrite)
{
    // Set the name of the current scene in effect
    g_SceneMan.GetScene()->SetPresetName(saveAsName);

	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	{
		string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".ini");
		string previewFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + saveAsName + ".preview.png");
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
		string previewFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + saveAsName + ".preview.png");
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

void SceneEditor::UpdateNewDialog()
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

    // Refill Terrains
    m_pNewTerrainCombo->ClearList();
    // Get the list of all read in terrains
    list<Entity *> terrainList;
    g_PresetMan.GetAllOfTypeInModuleSpace(terrainList, "SLTerrain", selectedModuleID);
    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = terrainList.begin(); itr != terrainList.end(); ++itr)
	{
		if ((*itr)->GetPresetName() != "Editor Terrain" &&
			(*itr)->GetPresetName() != "Physics Test Terrain")
			m_pNewTerrainCombo->AddItem((*itr)->GetPresetName());
	}
    // Select the first one
    m_pNewTerrainCombo->SetSelectedIndex(0);

    // Refill backdrops
    m_pNewBG1Combo->SetText("");
    m_pNewBG2Combo->SetText("");
    m_pNewBG3Combo->SetText("");
    m_pNewBG1Combo->ClearList();
    m_pNewBG2Combo->ClearList();
    m_pNewBG3Combo->ClearList();

    // Get the list of all read in NEAR background layers
    list<Entity *> bgList;
    g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Near Backdrops", "SceneLayer", selectedModuleID);
    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
        m_pNewBG1Combo->AddItem((*itr)->GetPresetName());

    // Get the list of all read in MID background layers
    bgList.clear();
    g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Mid Backdrops", "SceneLayer", selectedModuleID);
    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
        m_pNewBG2Combo->AddItem((*itr)->GetPresetName());

    // Get the list of all read in FAR background layers
    bgList.clear();
    g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Far Backdrops", "SceneLayer", selectedModuleID);
    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
        m_pNewBG3Combo->AddItem((*itr)->GetPresetName());

    // Select the first one for each
    m_pNewBG1Combo->SetSelectedIndex(0);
    m_pNewBG2Combo->SetSelectedIndex(0);
    m_pNewBG3Combo->SetSelectedIndex(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.

void SceneEditor::UpdateLoadDialog()
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

void SceneEditor::UpdateSaveDialog()
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

void SceneEditor::UpdateChangesDialog()
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

void SceneEditor::UpdateOverwriteDialog()
{
	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == "Scenes.rte")
	    m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
	else
	    m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
}

} // namespace RTE