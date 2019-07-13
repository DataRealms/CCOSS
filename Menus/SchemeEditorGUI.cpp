//////////////////////////////////////////////////////////////////////////////////////////
// File:            SchemeEditorGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the SchemeEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SchemeEditorGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "SceneEditor.h"
#include "UInputMan.h"

#include "Controller.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "HDFirearm.h"
#include "TDExplosive.h"
#include "TerrainObject.h"
#include "AHuman.h"
#include "ACrab.h"
#include "SLTerrain.h"
#include "ObjectPickerGUI.h"
#include "PieMenuGUI.h"
#include "Scene.h"
#include "SettingsMan.h"

using namespace std;
using namespace RTE;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SchemeEditorGUI, effectively
//                  resetting the members of this abstraction level only.

void SchemeEditorGUI::Clear()
{
    m_pController = 0;
    m_FeatureSet = ONLOADEDIT;
    m_EditMade = false;
    m_EditorGUIMode = PICKINGOBJECT;
    m_PreviousMode = ADDINGOBJECT;
    m_ModeChanged = true;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_pPieMenu = 0;
    m_pPicker = 0;
    m_NativeTechModule = 0;
    m_GridSnapping = true;
    m_CursorPos.Reset();
    m_CursorOffset.Reset();
    m_CursorInAir = true;
    m_FacingLeft = false;
    m_PlaceTeam = Activity::TEAM_1;
    m_pCurrentObject = 0;
    m_ObjectListOrder = -1;
    m_DrawCurrentObject = true;
    m_pObjectToBlink = 0;
    m_BrainSkyPath.clear();
    m_EnterMenuSound.Reset();
    m_ExitMenuSound.Reset();
    m_FocusChangeSound.Reset();
    m_SelectionChangeSound.Reset();
    m_ItemChangeSound.Reset();
    m_ObjectPickedSound.Reset();
    m_UserErrorSound.Reset();
    m_PlacementBlip.Reset();
    m_PlacementThud.Reset();
    m_PlacementGravel.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SchemeEditorGUI object ready for use.

int SchemeEditorGUI::Create(Controller *pController, FeatureSets featureSet, int whichModuleSpace, int nativeTechModule, float foreignCostMult)
{
    AAssert(pController, "No controller sent to SchemeEditorGUI on creation!");
    m_pController = pController;

    m_FeatureSet = featureSet;

    // Allocate and (re)create the Editor GUIs
    if (!m_pPieMenu)
        m_pPieMenu = new PieMenuGUI();
    else
        m_pPieMenu->Destroy();
    m_pPieMenu->Create(pController);

    // Init the pie menu
    UpdatePieMenu();


    // Allocate and (re)create the Editor GUIs
    if (!m_pPicker)
        m_pPicker = new ObjectPickerGUI();
    else
        m_pPicker->Destroy();
    m_pPicker->Create(pController, whichModuleSpace);

    m_NativeTechModule = nativeTechModule;

    // Cursor init
    m_CursorPos = g_SceneMan.GetSceneDim() / 2;

    // Set initial focus, category list, and label settings
    m_EditorGUIMode = PICKINGOBJECT;
    m_ModeChanged = true;
    m_pCurrentObject = 0;

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();

    m_EnterMenuSound.Create("Base.rte/GUIs/Sounds/MenuEnter.wav", false);
    m_ExitMenuSound.Create("Base.rte/GUIs/Sounds/MenuExit.wav", false);
    m_FocusChangeSound.Create("Base.rte/GUIs/Sounds/Blip00.wav", false);
    m_SelectionChangeSound.Create("Base.rte/GUIs/Sounds/Blip01.wav", false);
    m_ItemChangeSound.Create("Base.rte/GUIs/Sounds/Click00.wav", false);
//    m_ObjectPickedSound.Create("Base.rte/GUIs/Sounds/MenuEnter.wav, false");
    m_UserErrorSound.Create("Base.rte/GUIs/Sounds/Error.wav", false);
    m_PlacementBlip.Create("Base.rte/GUIs/Sounds/Blip02.wav", false);
    m_PlacementThud.Create("Base.rte/GUIs/Sounds/Thud01.wav", false);
    m_PlacementThud.AddSample("Base.rte/GUIs/Sounds/Thud02.wav");
    m_PlacementGravel.Create("Base.rte/GUIs/Sounds/Gravel1.wav", false);
    m_PlacementGravel.AddSample("Base.rte/GUIs/Sounds/Gravel2.wav");
    m_PlacementGravel.AddSample("Base.rte/GUIs/Sounds/Gravel3.wav");
    m_PlacementGravel.AddSample("Base.rte/GUIs/Sounds/Gravel4.wav");

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SchemeEditorGUI object.

void SchemeEditorGUI::Destroy()
{
    delete m_pPieMenu;
    delete m_pPicker;

    delete m_pCurrentObject;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the controller used by this. The ownership of the controller is
//                  NOT transferred!

void SchemeEditorGUI::SetController(Controller *pController)
{
    m_pController = pController;
    m_pPieMenu->SetController(pController);
    m_pPicker->SetController(pController);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void SchemeEditorGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pPicker->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the new Object to be held at the cursor of this Editor. Ownership
//                  IS transferred!

bool SchemeEditorGUI::SetCurrentObject(SceneObject *pNewObject)
{
    if (m_pCurrentObject == pNewObject)
        return true;

    // Replace the current object with the new one
    delete m_pCurrentObject;
    m_pCurrentObject = pNewObject;

    if (!m_pCurrentObject)
        return false;

    m_pCurrentObject->SetTeam(m_FeatureSet == ONLOADEDIT ? m_PlaceTeam : m_pController->GetTeam());
    m_pCurrentObject->SetPlacedByPlayer(m_pController->GetPlayer());

    // Disable any controller, if an actor
    if (Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject))
    {
        pActor->GetController()->SetDisabled(true);
        pActor->SetStatus(Actor::INACTIVE);
    }




    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.

int SchemeEditorGUI::GetActivatedPieSlice()
{
    return m_pPieMenu->GetPieCommand();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.

void SchemeEditorGUI::SetModuleSpace(int moduleSpaceID)
{
    m_pPicker->SetModuleSpace(moduleSpaceID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void SchemeEditorGUI::Update()
{
    // Update the user controller
//    m_pController->Update();

    m_EditMade = false;
    m_pObjectToBlink = 0;
    // Which set of placed objects in the scene we're editing
    int editedSet = Scene::PLACEONLOAD;

    ////////////////////////////////////////////
    // Blinking logic
/*
    if (m_BlinkMode == OBJECTBLINK)
    {
        m_pCostLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
    }
    else if (m_BlinkMode == NOCRAFT)
    {
        bool blink = m_BlinkTimer.AlternateSim(250);
        m_pCraftLabel->SetVisible(blink);
        m_pCraftBox->SetVisible(blink);
    }

    // Time out the blinker
    if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastSimMS(1500))
    {
        m_pCostLabel->SetVisible(true);
        m_pCraftLabel->SetVisible(true);
        m_pCraftBox->SetVisible(true);
        m_BlinkMode = NOBLINK;
    }
*/
    /////////////////////////////////////////////
    // Repeating input logic

    bool pressLeft = m_pController->IsState(PRESS_LEFT);
    bool pressRight = m_pController->IsState(PRESS_RIGHT);
    bool pressUp = m_pController->IsState(PRESS_UP);
    bool pressDown = m_pController->IsState(PRESS_DOWN);

    // If no direciton is held down, then cancel the repeating
    if (!(m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN)))
    {
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
    }

    // Check if any direction has been held for the starting amount of time to get into repeat mode
    if (m_RepeatStartTimer.IsPastRealMS(200))
    {
        // Check for the repeat interval
        if (m_RepeatTimer.IsPastRealMS(30))
        {
            if (m_pController->IsState(MOVE_RIGHT))
                pressRight = true;
            else if (m_pController->IsState(MOVE_LEFT))
                pressLeft = true;

            if (m_pController->IsState(MOVE_UP))
                pressUp = true;
            else if (m_pController->IsState(MOVE_DOWN))
                pressDown = true;

            m_RepeatTimer.Reset();
        }
    }

    ///////////////////////////////////////////////
    // Analog cursor input

    Vector analogInput;
    if (m_pController->GetAnalogMove().GetMagnitude() > 0.1)
        analogInput = m_pController->GetAnalogMove();
//    else if (m_pController->GetAnalogAim().GetMagnitude() > 0.1)
//        analogInput = m_pController->GetAnalogAim();

    /////////////////////////////////////////////
    // PIE MENU

    m_pPieMenu->Update();

    // Show the pie menu only when the secondary button is held down
    if (m_pController->IsState(PRESS_SECONDARY) && m_EditorGUIMode != INACTIVE && m_EditorGUIMode != PICKINGOBJECT)
    {
        m_pPieMenu->SetEnabled(true);
        m_pPieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);
    }

    if (!m_pController->IsState(PIE_MENU_ACTIVE) || m_EditorGUIMode == INACTIVE || m_EditorGUIMode == PICKINGOBJECT)
        m_pPieMenu->SetEnabled(false);

    ///////////////////////////////////////
    // Handle pie menu selections

    if (m_pPieMenu->GetPieCommand() != PieMenuGUI::PSI_NONE)
    {
        if (m_pPieMenu->GetPieCommand() == PieMenuGUI::PSI_PICK)
            m_EditorGUIMode = PICKINGOBJECT;
        else if (m_pPieMenu->GetPieCommand() == PieMenuGUI::PSI_MOVE)
            m_EditorGUIMode = MOVINGOBJECT;
        else if (m_pPieMenu->GetPieCommand() == PieMenuGUI::PSI_REMOVE)
            m_EditorGUIMode = DELETINGOBJECT;
        else if (m_pPieMenu->GetPieCommand() == PieMenuGUI::PSI_TEAM1)
            m_PlaceTeam = Activity::TEAM_1;
        else if (m_pPieMenu->GetPieCommand() == PieMenuGUI::PSI_TEAM2)
            m_PlaceTeam = Activity::TEAM_2;

        UpdatePieMenu();
        m_ModeChanged = true;
    }

    //////////////////////////////////////////
    // Picker logic

    // Enable or disable the picker
    m_pPicker->SetEnabled(m_EditorGUIMode == PICKINGOBJECT);

    // Update the picker GUI
    m_pPicker->Update();

    if (m_EditorGUIMode == PICKINGOBJECT && m_pPicker->ObjectPicked())
    {
        // Assign a copy of the picked object to be the currently held one.
        if (SetCurrentObject(dynamic_cast<SceneObject *>(m_pPicker->ObjectPicked()->Clone())))
        {
            // Set the team
            if (m_FeatureSet != ONLOADEDIT)
                m_pCurrentObject->SetTeam(m_pController->GetTeam());
            // Set the list order to be at the end so new objects are added there
            m_ObjectListOrder = -1;
            // Update the object
            m_pCurrentObject->Update();
                
            // If done picking, revert to moving object mode
            if (m_pPicker->DonePicking())
            {
                m_EditorGUIMode = ADDINGOBJECT;
                UpdatePieMenu();
                m_ModeChanged = true;
            }
        }
    }

    if (!m_pPicker->IsVisible())
        g_SceneMan.SetScreenOcclusion(Vector(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    else
        g_FrameMan.SetScreenText("Pick what you want to place next", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

    /////////////////////////////////////
    // ADDING OBJECT MODE

    if (m_EditorGUIMode == ADDINGOBJECT && !m_pPieMenu->IsEnabled())
    {
        if (m_ModeChanged)
        {

            m_ModeChanged = false;
        }
        g_FrameMan.SetScreenText("Click to ADD a new object - Drag for precision", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));


        m_DrawCurrentObject = true;

        // Trap the mouse cursor
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

        // Move the cursor according to analog or mouse input
        if (!analogInput.IsZero())
        {
            m_CursorPos += analogInput * 8;
            // Re-enable snapping only when the cursor is moved again
            m_GridSnapping = true;
        }
        else if (!m_pController->GetMouseMovement().IsZero())
        {
            m_CursorPos += m_pController->GetMouseMovement();
            // Re-enable snapping only when the cursor is moved again
            m_GridSnapping = true;
        }
        // Digital input?
        else
        {
            if (pressUp)
                m_CursorPos.m_Y -= SCENESNAPSIZE;
            if (pressRight)
                m_CursorPos.m_X += SCENESNAPSIZE;
            if (pressDown)
                m_CursorPos.m_Y += SCENESNAPSIZE;
            if (pressLeft)
                m_CursorPos.m_X -= SCENESNAPSIZE;
            // Re-enable snapping only when the cursor is moved again
            if (pressUp || pressRight || pressDown || pressLeft)
                m_GridSnapping = true;
        }

        // Detect whether the cursor is in the air, or if it's overlapping some terrain
        Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
        m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;

        // Mousewheel is used as shortcut for getting next and prev items in teh picker's object list
        if (m_pController->IsState(SCROLL_UP))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetPrevObject();
            if (pNewObject)
            {
                // Set and update the cursor object
                if (SetCurrentObject(dynamic_cast<SceneObject *>(pNewObject->Clone())))
                    m_pCurrentObject->Update();
            }
        }
        else if (m_pController->IsState(SCROLL_DOWN))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetNextObject();
            if (pNewObject)
            {
                // Set and update the object
                if (SetCurrentObject(dynamic_cast<SceneObject *>(pNewObject->Clone())))
                    m_pCurrentObject->Update();
            }
        }

        // Start the timer when the button is first pressed, and when the picker has deactivated
        if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_BlinkTimer.Reset();
            m_EditorGUIMode = PLACINGOBJECT;
            m_PreviousMode = ADDINGOBJECT;
            m_ModeChanged = true;
            UpdatePieMenu();
            m_PlacementBlip.Play();
        }

        // Apply the team to the current actor, if applicable
        if (m_pCurrentObject && m_DrawCurrentObject)
        {
            // Set the team of SceneObject based on what's been selected
            // Only if full featured mode, otherwise it's based on the controller when placed
            if (m_FeatureSet == ONLOADEDIT)
                m_pCurrentObject->SetTeam(m_PlaceTeam);
        }
    }
    // PLACING OBJECT MODE
	else if (m_EditorGUIMode == PLACINGOBJECT)
    {
        if (m_ModeChanged)
        {
            m_ModeChanged = false;
        }

        if (m_PreviousMode == MOVINGOBJECT)
            g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
        else
            g_FrameMan.SetScreenText("Release to ADD the new object - Tap other button to cancel", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

        m_DrawCurrentObject = true;

        // Freeze when first pressing down and grid snapping is still engaged
        if (!(m_pController->IsState(PRIMARY_ACTION) && m_GridSnapping))
        {
            if (!analogInput.IsZero())
            {
                m_CursorPos += analogInput;
                m_FacingLeft = analogInput.m_X < 0 || (m_FacingLeft && analogInput.m_X == 0);
            }
            // Try the mouse
            else if (!m_pController->GetMouseMovement().IsZero())
            {
                m_CursorPos += m_pController->GetMouseMovement();
                m_FacingLeft = m_pController->GetMouseMovement().m_X < 0 || (m_FacingLeft && m_pController->GetMouseMovement().m_X == 0);
            }
            // Digital input?
            else
            {
                if (pressUp)
                    m_CursorPos.m_Y -= 1;
                if (pressRight)
                {
                    m_CursorPos.m_X += 1;
                    m_FacingLeft = false;
                }
                if (pressDown)
                    m_CursorPos.m_Y += 1;
                if (pressLeft)
                {
                    m_CursorPos.m_X -= 1;
                    m_FacingLeft = true;
                }
            }

            // Detect whether the cursor is in the air, or if it's overlapping some terrain
            Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
            m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;
            // Also check that it isn't over unseen areas, can't place there
            m_CursorInAir = m_CursorInAir && !g_SceneMan.IsUnseen(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY(), m_pController->GetTeam());
        }

        // Constrain the cursor to only be within specific scene areas
// TODO: THIS!!!

        // Disable snapping after a small interval of holding down the button, to avoid unintentional nudges when just placing on the grid
        if (m_pController->IsState(PRIMARY_ACTION) && m_BlinkTimer.IsPastRealMS(333) && m_GridSnapping)
        {
            m_GridSnapping = false;
            m_CursorPos = g_SceneMan.SnapPosition(m_CursorPos);
        }

        // Cancel placing if secondary button is pressed
        if (m_pController->IsState(PRESS_SECONDARY) || m_pController->IsState(PIE_MENU_ACTIVE))
        {
            m_EditorGUIMode = m_PreviousMode;
            m_ModeChanged = true;
            UpdatePieMenu();
        }
        // If previous mode was moving, tear the gib loose if the button is released to soo
        else if (m_PreviousMode == MOVINGOBJECT && m_pController->IsState(RELEASE_PRIMARY) && !m_BlinkTimer.IsPastRealMS(150))
        {
            m_EditorGUIMode = ADDINGOBJECT;
            m_ModeChanged = true;
            UpdatePieMenu();
        }
        // Only place if the picker and pie menus are completely out of view, to avoid immediate placing after picking
        else if (m_pCurrentObject && m_pController->IsState(RELEASE_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_pCurrentObject->Update();

			g_SceneMan.GetScene()->AddPlacedObject(editedSet, dynamic_cast<SceneObject *>(m_pCurrentObject->Clone()), m_ObjectListOrder);
			// Increment the list order so we place over last placed item
			if (m_ObjectListOrder >= 0)
				m_ObjectListOrder++;
			m_PlacementThud.Play();
			m_EditMade = true;
// TEMP REMOVE WEHN YOU CLEAN UP THE ABOVE HARDCODED BRAIN PLACEMENT
            if (m_EditorGUIMode != PICKINGOBJECT)
// TEMP REMOVE ABOVE
            // Go back to previous mode
            m_EditorGUIMode = m_PreviousMode;
            m_ModeChanged = true;
            UpdatePieMenu();
        }

        // Set the facing of AHumans based on right/left cursor movements
// TODO: Improve
        Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject);
        if (pActor && dynamic_cast<AHuman *>(pActor) || dynamic_cast<ACrab *>(pActor))
            pActor->SetHFlipped(m_FacingLeft);
    }

    /////////////////////////////////////////////////////////////
    // POINTING AT MODES

    else if ((m_EditorGUIMode == MOVINGOBJECT || m_EditorGUIMode == DELETINGOBJECT) && !m_pPieMenu->IsEnabled())
    {
        m_DrawCurrentObject = false;

        // Trap the mouse cursor
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

        // Move the cursor according to analog or mouse input
        if (!analogInput.IsZero())
            m_CursorPos += analogInput * 4;
        else if (!m_pController->GetMouseMovement().IsZero())
            m_CursorPos += m_pController->GetMouseMovement() / 2;
        // Digital input?
        else
        {
            if (pressUp)
                m_CursorPos.m_Y -= 1;
            if (pressRight)
                m_CursorPos.m_X += 1;
            if (pressDown)
                m_CursorPos.m_Y += 1;
            if (pressLeft)
                m_CursorPos.m_X -= 1;
        }

        /////////////////////////////////
        // MOVING OBJECT MODE

        if (m_EditorGUIMode == MOVINGOBJECT)
        {
            if (m_ModeChanged)
            {

                m_ModeChanged = false;
            }
            g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

            // Pick an object under the cursor and start moving it
            if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
            {
                const SceneObject *pPicked = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder);
                if (pPicked)
                {
                    // Grab the position and a copy of the the object itself before killing it from the scene
                    SetCurrentObject(dynamic_cast<SceneObject *>(pPicked->Clone()));
                    m_CursorOffset = m_CursorPos - m_pCurrentObject->GetPos();
                    g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
                    m_EditMade = true;

                    // Go to placing mode to move it around
                    m_EditorGUIMode = PLACINGOBJECT;
                    m_PreviousMode = MOVINGOBJECT;
                    m_ModeChanged = true;
                    UpdatePieMenu();
                    m_BlinkTimer.Reset();
                    m_PlacementBlip.Play();
                    m_PlacementGravel.Play();
                }
                else
                    m_UserErrorSound.Play();
            }
        }

        ////////////////////////////
        // REMOVING OBJECT MODE

        else if (m_EditorGUIMode == DELETINGOBJECT)
        {
            if (m_ModeChanged)
            {

                m_ModeChanged = false;
            }
            g_FrameMan.SetScreenText("Click and hold to select an object - release to DELETE it", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

            // When primary is held down, pick object and show which one will be nuked if released
            if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible())
            {
                m_pObjectToBlink = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos);
            }
            else if (m_pController->IsState(RELEASE_PRIMARY))
            {
                if (g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder))
                {
                    // Nuke it!
                    g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
                    m_EditMade = true;
// TODO: Add awesome destruction sound here
                }
                else
                    m_UserErrorSound.Play();
            }
        }
    }
    else if (m_EditorGUIMode == DONEEDITING)
    {
    }

    // Remove cursor offset if not applicable anymore
    if (m_EditorGUIMode != PLACINGOBJECT)
        m_CursorOffset.Reset();

    // Keep the cursor position within the world
    bool cursorWrapped = g_SceneMan.ForceBounds(m_CursorPos);
// TODO: make setscrolltarget with 'sloppy' target
    // Scroll to the cursor's scene position
    g_SceneMan.SetScrollTarget(m_CursorPos, 0.3, cursorWrapped, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    // Apply the cursor position to the currently held object
    if (m_pCurrentObject && m_DrawCurrentObject)
    {
        m_pCurrentObject->SetPos(g_SceneMan.SnapPosition(m_CursorPos - m_CursorOffset, m_GridSnapping));
        // If an actor, set it to be inactive so it doesn't scan around and reveal unseen areas
        if (Actor *pCurrentActor = dynamic_cast<Actor *>(m_pCurrentObject))
        {
            pCurrentActor->SetStatus(Actor::INACTIVE);
            pCurrentActor->GetController()->SetDisabled(true);
        }
        m_pCurrentObject->Update();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void SchemeEditorGUI::Draw(BITMAP *pTargetBitmap, const Vector &targetPos) const
{
    // Done, so don't draw the UI
    if (m_EditorGUIMode == DONEEDITING)
        return;

    // The get a list of the currently edited set of placed objects in the Scene
    const list<SceneObject *> *pSceneObjectList = 0;
    if (m_FeatureSet == ONLOADEDIT)
        pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);

    // Draw the set of currently edited objects already placed in the Scene
    if (pSceneObjectList)
    {
        // Draw all already placed Objects, and the currently held one in the order it is about to be placed in the scene
        int i = 0;
        Actor *pActor = 0;
//        HeldDevice *pDevice = 0;
        for (list<SceneObject *>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr, ++i)
        {
            // Draw the currently held object into the order of the list if it is to be placed inside
            if (m_pCurrentObject && m_DrawCurrentObject && i == m_ObjectListOrder)
            {
                g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? LessTrans : HalfTrans);
                m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
                pActor = dynamic_cast<Actor *>(m_pCurrentObject);
                if (pActor)
                    pActor->DrawHUD(pTargetBitmap, targetPos);
            }

            // Is the placed object an actor?
            pActor = dynamic_cast<Actor *>(*itr);
//            pItem = dynamic_cast<MovableObject *>(*itr);            

            // Blink trans if we are supposed to blink this one
            if ((*itr) == m_pObjectToBlink)
            {
                g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) ? LessTrans : HalfTrans);
                (*itr)->Draw(pTargetBitmap, targetPos, g_DrawTrans);
            }
            // Drawing of already placed objects that aren't highlighted or anything
            else
            {
                (*itr)->Draw(pTargetBitmap, targetPos);
            }

            // Draw basic HUD if an actor - don't do this for blueprints.. it is confusing
            if (pActor)
                pActor->DrawHUD(pTargetBitmap, targetPos);
        }
    }

    // Draw picking object crosshairs and not the selected object
    if (!m_DrawCurrentObject)
    {
        Vector center = m_CursorPos - targetPos;
        putpixel(pTargetBitmap, center.m_X, center.m_Y, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X - 5, center.m_Y, center.m_X - 2, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X + 5, center.m_Y, center.m_X + 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y - 5, center.m_Y - 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y + 5, center.m_Y + 2, g_YellowGlowColor);
    }
    // If the held object will be placed at the end of the list, draw it last to the scene, transperent blinking
    else if (m_pCurrentObject && (m_ObjectListOrder < 0 || m_ObjectListOrder == pSceneObjectList->size()))
    {
        g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? LessTrans : HalfTrans);
        m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
        Actor *pActor = dynamic_cast<Actor *>(m_pCurrentObject);
        if (pActor)
            pActor->DrawHUD(pTargetBitmap, targetPos);
    }

    m_pPicker->Draw(pTargetBitmap);

    // Draw the pie menu
    m_pPieMenu->Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePieMenu
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the PieMenu config based ont eh current editor state.

void SchemeEditorGUI::UpdatePieMenu()
{
    m_pPieMenu->ResetSlices();

	PieMenuGUI::Slice newSceneSlice("New Scene", PieMenuGUI::PSI_NEW, PieMenuGUI::Slice::UP);
    m_pPieMenu->AddSlice(newSceneSlice);
	PieMenuGUI::Slice loadSceneSlice("Load Scene", PieMenuGUI::PSI_LOAD, PieMenuGUI::Slice::UP);
	m_pPieMenu->AddSlice(loadSceneSlice);
	
	PieMenuGUI::Slice testSceneSlice("Test Scene", PieMenuGUI::PSI_DONE, PieMenuGUI::Slice::UP);
    m_pPieMenu->AddSlice(testSceneSlice);
    PieMenuGUI::Slice moveObjectsSlice("Move Objects", PieMenuGUI::PSI_MOVE, PieMenuGUI::Slice::LEFT);
	m_pPieMenu->AddSlice(moveObjectsSlice);
	
	PieMenuGUI::Slice removeObjectsSlice("Remove Objects", PieMenuGUI::PSI_REMOVE, PieMenuGUI::Slice::LEFT);
    m_pPieMenu->AddSlice(removeObjectsSlice);
	PieMenuGUI::Slice addNewSlice("Add New Object", PieMenuGUI::PSI_PICK, PieMenuGUI::Slice::RIGHT);
	m_pPieMenu->AddSlice(addNewSlice);
	
	PieMenuGUI::Slice saveSceneSlice("Save Scene", PieMenuGUI::PSI_SAVE, PieMenuGUI::Slice::DOWN);
    m_pPieMenu->AddSlice(saveSceneSlice);

	if (m_EditorGUIMode == ADDINGOBJECT)
    {
		PieMenuGUI::Slice inFrontSlice("Put In Front Of", PieMenuGUI::PSI_INFRONT, PieMenuGUI::Slice::LEFT);
        m_pPieMenu->AddSlice(inFrontSlice);
        PieMenuGUI::Slice behindSlice("Put Behind Of", PieMenuGUI::PSI_BEHIND, PieMenuGUI::Slice::LEFT);
		m_pPieMenu->AddSlice(behindSlice);
    }
    m_pPieMenu->RealignSlices();
}


