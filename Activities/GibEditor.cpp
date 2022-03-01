//////////////////////////////////////////////////////////////////////////////////////////
// File:            GibEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GibEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GibEditor.h"
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

#include "GibEditorGUI.h"

namespace RTE {

ConcreteClassInfo(GibEditor, EditorActivity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GibEditor, effectively
//                  resetting the members of this abstraction level only.

void GibEditor::Clear()
{
    m_pEditedObject = 0;
    m_pTestingObject = 0;
    m_TestCounter = 0;
    m_pObjectToLoad = 0;
    m_pEditorGUI = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GibEditor object ready for use.

int GibEditor::Create()
{
    if (EditorActivity::Create() < 0)
        return -1;


    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GibEditor to be identical to another, by deep copy.

int GibEditor::Create(const GibEditor &reference)
{
    if (EditorActivity::Create(reference) < 0)
        return -1;

    if (m_Description.empty())
        m_Description = "Edit how Movable Objects break apart into smaller pieces.";

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int GibEditor::ReadProperty(const std::string_view &propName, Reader &reader)
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
// Description:     Saves the complete state of this GibEditor with a Writer for
//                  later recreation with Create(Reader &reader);

int GibEditor::Save(Writer &writer) const {
	EditorActivity::Save(writer);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GibEditor object.

void GibEditor::Destroy(bool notInherited)
{
    delete m_pEditedObject;
    delete m_pTestingObject;
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

int GibEditor::Start()
{
    int error = EditorActivity::Start();

    //////////////////////////////////////////////
    // Allocate and (re)create the Editor GUI

    if (m_pEditorGUI)
        m_pEditorGUI->Destroy();
    else
        m_pEditorGUI = new GibEditorGUI;
    m_pEditorGUI->Create(&(m_PlayerController[0]));

    //////////////////////////////////////////////////////////////
    // Hooking up directly to the controls defined in the GUI ini

    m_pGUIController->Load("Base.rte/GUIs/GibEditorGUI.ini");

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
//    m_pNewModuleCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("NewModuleCB"));
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
    m_pLoadButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadButton"));
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
    m_pSaveNameBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("SaveNameTB"));
    m_pSaveModuleLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SaveModuleLabel"));
    m_pSaveButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SaveButton"));
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

    m_EditorMode = EditorActivity::EDITINGOBJECT;
    // Show the picker dialog to select an object to load
    m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::PICKOBJECTTOLOAD);
    m_ModeChange = true;

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GibEditor::SetPaused(bool pause)
{
    // Override the pause
    m_Paused = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GibEditor::End()
{
    EditorActivity::End();

    

    m_ActivityState = ActivityState::Over;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GibEditor. Supposed to be done every frame
//                  before drawing.

void GibEditor::Update()
{
    // And object hasn't been loaded yet, so get the loading picker going
    if (!m_pEditedObject && !m_pObjectToLoad)
    {
        m_NeedSave = false;
        m_HasEverBeenSaved = false;
        m_PreviousMode = EditorActivity::LOADDIALOG;
        m_EditorMode = EditorActivity::EDITINGOBJECT;
        // Show the picker dialog to select an object to load
        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::PICKOBJECTTOLOAD);
        m_ModeChange = true;
    }

    // Mode switching etc
    EditorActivity::Update();

    if (!g_SceneMan.GetScene())
        return;

    //////////////////////////////////////
    // Special testing mode

    if (m_EditorMode == EditorActivity::TESTINGOBJECT)
    {
        if (m_ModeChange)
        {
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }

        // We haven't detonated yet
        if (m_pTestingObject)
        {
            g_FrameMan.SetScreenText("Click to test gib the object!", 0, 333);

            // Detonate on command!
            if (m_PlayerController[0].IsState(PRESS_PRIMARY) || m_PlayerController[0].IsState(PRESS_SECONDARY) || m_PlayerController[0].IsState(PRESS_FACEBUTTON))
            {
                // This adds all the gibs to the movableman
                m_pTestingObject->GibThis();
                // Now safe to get rid of the test subject
                delete m_pTestingObject;
                m_pTestingObject = 0;
            }
        }
        // Test has blown up, now waiting for user to finish watching the pieces fly
        else
        {
            g_FrameMan.SetScreenText("Click again to go back to editing...");

            if (m_PlayerController[0].IsState(PRESS_PRIMARY) || m_PlayerController[0].IsState(PRESS_SECONDARY) || m_PlayerController[0].IsState(PRESS_FACEBUTTON))
            {
                // Clear out the terrain after a few tests
                if (m_TestCounter >= 3)
                {
                    g_SceneMan.GetScene()->GetTerrain()->ClearAllMaterial();
                    m_TestCounter = 0;
                }
                // Clear all crap still flying around
                g_MovableMan.PurgeAllMOs();
                g_MovableMan.Update();
                // Go back to editing the edited object
                m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);
                m_EditorMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
                // Pause the sim again
                m_Paused = true;
            }
        }
    }
    // All dialog boxes are gone and we're editing the object
    else if (m_pEditedObject && m_EditorMode == EditorActivity::EDITINGOBJECT)
    {
        if (m_ModeChange)
        {
            // Open the picker depending on whetehr there's somehting in the cursor hand or not
            m_pEditorGUI->SetEditorGUIMode(m_pEditorGUI->GetCurrentGib() ? GibEditorGUI::ADDINGGIB : GibEditorGUI::PICKINGGIB);
            // Hide the cursor for this layer of interface
            m_pGUIController->EnableMouse(false);
            m_ModeChange = false;
        }
        g_UInputMan.DisableKeys(false);
    }
    // We are doing something in the dialog boxes, so don't do anything in the editor interface
    else if (m_pEditedObject)
        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);

    /////////////////////////////////////////////////////
    // Update the editor interface

    m_pEditorGUI->Update();

    // Any edits made, dirtying the object?
    m_NeedSave = m_NeedSave || m_pEditorGUI->EditMade();

    // Get any mode change commands that the user gave the Editor GUI
    if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_NEW && m_EditorMode != NEWDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::NEWDIALOG;
        m_ModeChange = true;
    }
    // Loading is done differently, only when user has already picked an object ot load from the picker should the dialog box appear
    else if (m_pEditorGUI->GetObjectToLoad() && m_EditorMode != LOADDIALOG)
    {
        m_pObjectToLoad = m_pEditorGUI->GetObjectToLoad();
        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::LOADDIALOG;
        m_ModeChange = true;
    }
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_SAVE && m_EditorMode != SAVEDIALOG)
    {
        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);
        m_EditorMode = EditorActivity::SAVEDIALOG;
        m_ModeChange = true;
    }
    // Test the object by allowing the player to gib temporary test copy instances of the edited object
    else if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::PieSliceIndex::PSI_DONE)
    {
        // Make the copy of the current edited object
        delete m_pTestingObject;
        m_pTestingObject = dynamic_cast<MOSRotating *>(m_pEditedObject->Clone());

        // Put the proxy gibs into the test object
        StuffEditedGibs(m_pTestingObject);
        // Increment the number of tests, after certain amount the terrain is cleared of debris
        m_TestCounter++;

        m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::INACTIVE);
        m_PreviousMode = EditorActivity::EDITINGOBJECT;
        m_EditorMode = EditorActivity::TESTINGOBJECT;
        m_ModeChange = true;
        // Start running the sim
        m_Paused = false;
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
/*
            //////////////////////////////////////////////////////////
			// NEW button pressed; create a new object

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

                    // Add specified object layers
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

                    // Actually load the object's data and set it up as the current object
                    g_SceneMan.LoadScene(pNewScene);

                    // Reset the rest of the editor GUI
                    m_pEditorGUI->Destroy();
                    m_pEditorGUI->Create(&(m_PlayerController[0]), m_ModuleSpaceID);
                }

                m_NeedSave = false;
                m_HasEverBeenSaved = false;
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
            }
*/
            //////////////////////////////////////////////////////////
			// LOAD button pressed; load the selected Object

			if (anEvent.GetControl() == m_pLoadButton && m_pObjectToLoad)
            {
                m_pEditedObject = dynamic_cast<MOSRotating *>(m_pObjectToLoad->Clone());
                if (m_pEditedObject)
                {
                    // Get the module space ID
                    m_ModuleSpaceID = m_pEditedObject->GetModuleID();
                    RTEAssert(m_ModuleSpaceID >= 0, "Loaded Object's DataModule ID is negative? Should always be a specific one..");

                    // Restart the editor GUI
                    m_pEditorGUI->Destroy();
                    m_pEditorGUI->Create(&(m_PlayerController[0]), m_ModuleSpaceID);

                    // Set the position of the loaded edited object to the middle of the scene
                    m_pEditedObject->SetPos(Vector(g_SceneMan.GetSceneWidth() / 2, g_SceneMan.GetSceneHeight() / 2));
                    m_pEditedObject->Update();

                    // Make proxy copies of the loaded objects' gib reference instances and place them in the list to be edited
                    list<Gib> *pLoadedGibList = m_pEditedObject->GetGibList();
                    list<MovableObject *> *pEditedGibList = m_pEditorGUI->GetPlacedGibs();
                    MovableObject *pGibCopy = 0;

                    for (list<Gib>::iterator gItr = pLoadedGibList->begin(); gItr != pLoadedGibList->end(); ++gItr)
                    {
                        pGibCopy = dynamic_cast<MovableObject *>((*gItr).GetParticlePreset()->Clone());
                        if (pGibCopy)
                        {
                            pGibCopy->SetPos(m_pEditedObject->GetPos() + (*gItr).GetOffset());
                            pEditedGibList->push_back(pGibCopy);
                        }
                        pGibCopy = 0;
                    }

                    // Clear out the testing area
                    g_SceneMan.GetScene()->GetTerrain()->ClearAllMaterial();
                    m_TestCounter = 0;

                    m_pObjectToLoad = 0;
                }
                m_NeedSave = false;
                m_HasEverBeenSaved = true;
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
            }

            //////////////////////////////////////////////////////////
			// SAVE button pressed; save the selected Object

			if (anEvent.GetControl() == m_pSaveButton)
            {
                if (!m_pSaveNameBox->GetText().empty())
                {
                    // Save the object to the name specified in the text box
                    if (SaveObject(m_pSaveNameBox->GetText()))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the object
                        m_EditorMode = m_PreviousMode;
                        m_ModeChange = true;
                    }
                    // Should really leave dialog box open? error handling, bitte
                    else
                    {
                        ;
                    }
                }
            }

            ///////////////////////////////////////////////////////////////
			// Save Changes YES pressed

			if (anEvent.GetControl() == m_pChangesYesButton && m_pEditedObject)
            {
                if (m_HasEverBeenSaved)
                {
                    if (SaveObject(m_pEditedObject->GetPresetName(), true))
                    {
                        // Close the dialog box on success
                        m_NeedSave = false;
                        m_HasEverBeenSaved = true;
                        // Go back to previous mode after save dialog is done, may have been on the way to test the object
                        m_EditorMode = m_PreviousMode;
                        m_ModeChange = true;
                    }
                }
                // Open the save object dialog to ask user where to save it then
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
			// Overwrite Object YES pressed

			if (anEvent.GetControl() == m_pOverwriteYesButton && m_pEditedObject)
            {
                // Force overwrite
                if (SaveObject(m_pEditedObject->GetPresetName(), true))
                {
                    // Close the dialog box on success
                    m_NeedSave = false;
                    m_HasEverBeenSaved = true;
                    // Go back to previous mode after overwrite dialog is done, may have been on the way to test the object
                    m_EditorMode = m_PreviousMode != EditorActivity::SAVEDIALOG ? m_PreviousMode : EditorActivity::EDITINGOBJECT;
                    m_ModeChange = true;
                }
// TODO: Show overwrite error?
            }

            ///////////////////////////////////////////////////////////////
			// Overwrite Object NO pressed

			if (anEvent.GetControl() == m_pOverwriteNoButton)
            {
                // Just go back to previous mode
                m_EditorMode = m_PreviousMode;
                m_ModeChange = true;
            }

            ///////////////////////////////////////////////////////////////
			// CANCEL button pressed; exit any active dialog box

            // If load is cancel when just opening the scene
            if (anEvent.GetControl() == m_pLoadCancel && !m_pEditedObject)
            {
                m_PreviousMode = EditorActivity::LOADDIALOG;
                m_EditorMode = EditorActivity::EDITINGOBJECT;
                // Show the picker dialog to select an object to load
                m_pEditorGUI->SetEditorGUIMode(GibEditorGUI::PICKOBJECTTOLOAD);
                m_ModeChange = true;
            }
			if (anEvent.GetControl() == m_pNewCancel || anEvent.GetControl() == m_pLoadCancel || anEvent.GetControl() == m_pSaveCancel)
            {
                m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
                m_ModeChange = true;
            }
        }

        // Notifications
        else if (anEvent.GetType() == GUIEvent::Notification)
        {
            ///////////////////////////////////////
            // Clicks on the New Object Module combo

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

void GibEditor::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
{
    // Testing mode
    if (m_EditorMode == EditorActivity::TESTINGOBJECT)
    {
        if (m_pTestingObject)
            m_pTestingObject->Draw(pTargetBitmap, targetPos, g_DrawColor, true);
    }
    // Regular drawing mode
    else
    {
        // Draw ghost outline of edited object to place gibs upon
        if (m_pEditedObject)
        {
            g_FrameMan.SetTransTable(MoreTrans);
            // Draw only the MOSRotating since that's all we are adding gibs for; any attachables have to be edited separately
            m_pEditedObject->MOSRotating::Draw(pTargetBitmap, targetPos, g_DrawTrans, true);
        }

        m_pEditorGUI->Draw(pTargetBitmap, targetPos);
        EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GibEditor's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GibEditor::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    EditorActivity::Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current object to an appropriate ini file, and asks user if
//                  they want to overwrite first if object of this name exists.

bool GibEditor::SaveObject(string saveAsName, bool forceOverwrite)
{
    if (!m_pEditedObject)
        return false;

    // Set the name of the current object in effect
    m_pEditedObject->SetPresetName(saveAsName);

    // Replace the gibs of the object with the proxies that have been edited in the gui
    StuffEditedGibs(m_pEditedObject);

// TODO: do proper overwriting looking at the files etc
    // Try to add to the isntance man
    string objectFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/NewData/" + saveAsName + ".ini");

	// Check if file exists
	bool newDataFileExisted = exists(objectFilePath.c_str());

	// Try to create NewData directory if file does not exist
	if (!newDataFileExisted)
	{
		System::MakeDirectory(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/NewData");
	}

    if (g_PresetMan.AddEntityPreset(m_pEditedObject, m_ModuleSpaceID, forceOverwrite, objectFilePath))
// TEMP always overwrite for now until proper save system is in place
//    if (g_PresetMan.AddEntityPreset(m_pEditedObject, m_ModuleSpaceID, true))
    {
        // Does ini already exist? If yes, then no need to add it to a objects.ini etc
        bool objectFileExisted = exists(objectFilePath.c_str());
        // If the ini file already exists, and then ask if overwrite first
        if (objectFileExisted && !forceOverwrite)
        {
            // Gotto ask if we can overwrite the existing object/file
            m_PreviousMode = EditorActivity::SAVEDIALOG;
            m_EditorMode = EditorActivity::OVERWRITEDIALOG;
            m_ModeChange = true;
            return false;
        }
        // Create the writer
        Writer objectWriter(objectFilePath.c_str(), false);
        RTEAssert(objectWriter.WriterOK(), "Couldn't open file " + objectFilePath + "to write to! Check if directory exists..?");
        objectWriter.NewProperty("AddObject");
        // Write the object out to the new ini
        m_pEditedObject->MOSRotating::Save(objectWriter);
        objectWriter.ObjectEnd();
        // TODO: Make system for saving into/over the existing definition read originally from the ini's, wherever it was
        /*
                if (!objectFileExisted)
                {
                    // First find/create  a .rte/Scenes.ini file to include the new .ini into
                    string objectsFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes.ini");
                    bool objectsFileExisted = exists(objectsFilePath.c_str());
                    Writer objectsWriter(objectsFilePath.c_str(), true);
                    objectsWriter.NewProperty("\nIncludeFile");
                    objectsWriter << objectFilePath;

                    // Also add a line to the end of the modules' Index.ini to include the newly created Scenes.ini next startup
                    // If it's already included, it doens't matter, the definitions will just bounce the second time
                    if (!objectsFileExisted)
                    {
                        string indexFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Index.ini");
                        Writer indexWriter(indexFilePath.c_str(), true);
                        // Add extra tab since the DataModule has everything indented
                        indexWriter.NewProperty("\tIncludeFile");
                        indexWriter << objectsFilePath;
                    }
                }
        */
        return m_HasEverBeenSaved = true;
    }
    else
    {
        // Gotto ask if we can overwrite the existing object
        m_PreviousMode = EditorActivity::SAVEDIALOG;
        m_EditorMode = EditorActivity::OVERWRITEDIALOG;
        m_ModeChange = true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StuffEditedGibs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces all the gibs owned by the passed in MOSR with the ones being
//                  edited that represent the new gibbing setup.

void GibEditor::StuffEditedGibs(MOSRotating *pEditedObject)
{
    if (!pEditedObject)
        return;

    // Replace the gibs of the object with the proxies that have been edited in the gui
    list<Gib> *pObjectGibList = pEditedObject->GetGibList();
    pObjectGibList->clear();

    // Take each proxy object and stuff it into a Gib instance which then gets stuffed into the object to be saved
    list<MovableObject *> *pProxyGibList = m_pEditorGUI->GetPlacedGibs();
    for (list<MovableObject *>::iterator gItr = pProxyGibList->begin(); gItr != pProxyGibList->end(); ++gItr)
    {
        Gib newGib;
        // Only set the refernce instance directly from the isntanceman. OWNERSHIP IS NOT TRANSFERRED!
        newGib.m_GibParticle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset((*gItr)->GetClassName(), (*gItr)->GetPresetName(), m_ModuleSpaceID));
        if (newGib.m_GibParticle)
        {
            newGib.m_Count = 1;
            newGib.m_Offset = (*gItr)->GetPos() - pEditedObject->GetPos();
// TODO: do proper velocity calculations here!
// ... actually leave these as 0 and let them be calculated in GibThis
//            newGib.m_MinVelocity = (100.0f + 50.0f * NormalRand()) / (*gItr)->GetMass();
//            newGib.m_MaxVelocity = newGib.m_MinVelocity + ((100.0f * RandomNum()) / (*gItr)->GetMass());
            pObjectGibList->push_back(newGib);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateNewDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New dialog box, populates its lists etc.

void GibEditor::UpdateNewDialog()
{
/*
    // Only refill modules if empty
    if (m_pNewModuleCombo->GetCount() <= 0)
    {
        for (int module = 0; module < g_PresetMan.GetTotalModuleCount(); ++module)
            m_pNewModuleCombo->AddItem(g_PresetMan.GetDataModule(module)->GetFileName());

        // Select the first one
        m_pNewModuleCombo->SetSelectedIndex(0);
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
        m_pNewTerrainCombo->AddItem((*itr)->GetPresetName());
    // Select the first one
    m_pNewTerrainCombo->SetSelectedIndex(0);

    // Refill backdrops
    m_pNewBG1Combo->SetText("");
    m_pNewBG2Combo->SetText("");
    m_pNewBG3Combo->SetText("");
    m_pNewBG1Combo->ClearList();
    m_pNewBG2Combo->ClearList();
    m_pNewBG3Combo->ClearList();
    // Get the list of all read in terrains
    list<Entity *> bgList;
    g_PresetMan.GetAllOfTypeInModuleSpace(bgList, "SceneLayer", selectedModuleID);
    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
    {
        m_pNewBG1Combo->AddItem((*itr)->GetPresetName());
        m_pNewBG2Combo->AddItem((*itr)->GetPresetName());
        m_pNewBG3Combo->AddItem((*itr)->GetPresetName());
    }
    // Select the first one
    m_pNewBG1Combo->SetSelectedIndex(0);
    m_pNewBG2Combo->SetSelectedIndex(0);
    m_pNewBG3Combo->SetSelectedIndex(0);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.

void GibEditor::UpdateLoadDialog()
{
    if (m_pObjectToLoad)
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LoadNameLabel"))->SetText("Load object named " + m_pObjectToLoad->GetPresetName() + "?");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateSaveDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save dialog box, populates its lists etc.

void GibEditor::UpdateSaveDialog()
{
    if (!m_pEditedObject)
        return;

    m_pSaveNameBox->SetText((m_pEditedObject->GetPresetName() == "None" || !m_HasEverBeenSaved) ? "New Object" : m_pEditedObject->GetPresetName());
// TODO: Really?
    m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/NewData/");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChangesDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save Changes dialog box, populates its lists etc.

void GibEditor::UpdateChangesDialog()
{
    if (!m_pEditedObject)
        return;

    if (m_HasEverBeenSaved)
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Do you want to save your changes to:");
        m_pChangesNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/NewData/" + m_pEditedObject->GetPresetName() + ".ini");
    }
    else
    {
        dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Save your new Object first?");
        m_pChangesNameLabel->SetText("");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateOverwriteDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Overwrite dialog box, populates its lists etc.

void GibEditor::UpdateOverwriteDialog()
{
    if (m_pEditedObject)
        m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/NewData/" + m_pEditedObject->GetPresetName() + ".ini");
}

} // namespace RTE