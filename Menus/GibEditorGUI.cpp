//////////////////////////////////////////////////////////////////////////////////////////
// File:            GibEditorGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the GibEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GibEditorGUI.h"
#include "GUISound.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "TerrainObject.h"
#include "AHuman.h"
#include "SLTerrain.h"
#include "ObjectPickerGUI.h"
#include "PieMenuGUI.h"

using namespace RTE;

#define MAXZOOMFACTOR 5
#define MINZOOMFACTOR 1

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GibEditorGUI, effectively
//                  resetting the members of this abstraction level only.

void GibEditorGUI::Clear()
{
    m_pController = 0;
    m_EditMade = false;
    m_EditorGUIMode = PICKINGGIB;
    m_PreviousMode = ADDINGGIB;
    m_pObjectToLoad = 0;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_pPieMenu = 0;
    m_ActivatedPieSliceType = PieSlice::PieSliceIndex::PSI_NONE;
    m_pPicker = 0;
    m_GridSnapping = false;
    m_pZoomSource = 0;
    m_ZoomFactor = 1;
    m_CursorPos.Reset();
    m_CursorOffset.Reset();
    m_CursorInAir = true;
    m_FacingLeft = false;
    m_PlacedGibs.clear();
    m_pCurrentGib = 0;
    m_GibListOrder = -1;
    m_DrawCurrentGib = true;
    m_pObjectToBlink = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GibEditorGUI object ready for use.

int GibEditorGUI::Create(Controller *pController, int whichModuleSpace)
{
    RTEAssert(pController, "No controller sent to GibEditorGUI on creation!");
    m_pController = pController;

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
        m_pPicker->Reset();
    // Only show MovableObject:s as valid gibs to be placed
    m_pPicker->Create(pController, whichModuleSpace, "MovableObject");

    // Intermediate zooming bitmap
    m_pZoomSource = create_bitmap_ex(8, 64, 64);

    // Cursor init
    m_CursorPos = g_SceneMan.GetSceneDim() / 2;

    // Set initial focus, category list, and label settings
    m_EditorGUIMode = PICKINGGIB;
    m_pCurrentGib = 0;

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GibEditorGUI object.

void GibEditorGUI::Destroy()
{
    delete m_pPieMenu;
    delete m_pPicker;

    destroy_bitmap(m_pZoomSource);

    for (list<MovableObject *>::iterator gItr = m_PlacedGibs.begin(); gItr != m_PlacedGibs.end(); ++gItr)
         delete (*gItr);

    delete m_pCurrentGib;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the controller used by this. The ownership of the controller is
//                  NOT transferred!

void GibEditorGUI::SetController(Controller *pController)
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

void GibEditorGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pPicker->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.

void GibEditorGUI::SetModuleSpace(int moduleSpaceID)
{
    m_pPicker->SetModuleSpace(moduleSpaceID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void GibEditorGUI::Update()
{
    // Update the user controller
//    m_pController->Update();

    m_pObjectToLoad = 0;
    m_EditMade = false;
    m_pObjectToBlink = 0;

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

    // If no direction is held down, then cancel the repeating
    if (!(m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN)))
    {
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
    }

    // Check if any direction has been held for the starting amount of time to get into repeat mode
    if (m_RepeatStartTimer.IsPastRealMS(200))
    {
        // Check for the repeat interval
        if (m_RepeatTimer.IsPastRealMS(50))
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
    if (m_pController->IsState(PRESS_SECONDARY) && m_EditorGUIMode != INACTIVE && m_EditorGUIMode != PICKINGGIB)
    {
        m_pPieMenu->SetEnabled(true);
        m_pPieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);
    }

    if (!m_pController->IsState(PIE_MENU_ACTIVE) || m_EditorGUIMode == INACTIVE || m_EditorGUIMode == PICKINGGIB)
        m_pPieMenu->SetEnabled(false);

    ///////////////////////////////////////
    // Handle pie menu selections

    m_ActivatedPieSliceType = m_pPieMenu->GetPieCommand();
    if (m_pPieMenu->GetPieCommand() != PieSlice::PieSliceIndex::PSI_NONE)
    {
        if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_PICK)
            m_EditorGUIMode = PICKINGGIB;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_LOAD)
        {
            // Set up the picker to pick an MOSRotating to load
            m_EditorGUIMode = PICKOBJECTTOLOAD;
            m_pPicker->ShowOnlyType("MOSRotating");
        }
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_MOVE)
            m_EditorGUIMode = MOVINGGIB;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_REMOVE)
            m_EditorGUIMode = DELETINGGIB;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_DONE)
            m_EditorGUIMode = DONEEDITING;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_ZOOMIN && m_ZoomFactor < MAXZOOMFACTOR)
            m_ZoomFactor++;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_ZOOMOUT && m_ZoomFactor > MINZOOMFACTOR)
            m_ZoomFactor--;
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_INFRONT)
        {
            m_PreviousMode = m_EditorGUIMode;
            m_EditorGUIMode = PLACEINFRONT;
        }
        else if (m_pPieMenu->GetPieCommand() == PieSlice::PieSliceIndex::PSI_BEHIND)
        {
            m_PreviousMode = m_EditorGUIMode;
            m_EditorGUIMode = PLACEBEHIND;
        }
        
        UpdatePieMenu();
    }

    //////////////////////////////////////////
    // Picker logic

    // Enable or disable the picker
    m_pPicker->SetEnabled(m_EditorGUIMode == PICKOBJECTTOLOAD || m_EditorGUIMode == PICKINGGIB);

    // Update the picker GUI
    m_pPicker->Update();

    // Picking something to load into the editor
    if (m_EditorGUIMode == PICKOBJECTTOLOAD)
    {
        g_FrameMan.ClearScreenText();
        g_FrameMan.SetScreenText("Select an object to LOAD into the gib editor ->", 0, 333);

        // Picked something!
        if (m_pPicker->ObjectPicked() && !m_pPicker->IsEnabled())
        {
            m_pObjectToLoad = dynamic_cast<const MOSRotating *>(m_pPicker->ObjectPicked());
            // Set picker back to showing all valid gib types
            if (m_pObjectToLoad)
            {
                m_pPicker->ShowOnlyType("MovableObject");
                g_FrameMan.ClearScreenText();
            }
        }
    }
    // Picking an object to place as a gib in currently edited object
    else if (m_EditorGUIMode == PICKINGGIB)
    {
        g_FrameMan.SetScreenText("Select a new Gib object to add onto the edited object ->");

        if (m_pPicker->ObjectPicked())
        {
            // Assign a copy of the picked object to be the currently held one.
            delete m_pCurrentGib;
            if (m_pCurrentGib = dynamic_cast<MovableObject *>(m_pPicker->ObjectPicked()->Clone()))
            {
                // Disable any controller, if an actor
                Actor *pActor = dynamic_cast<Actor *>(m_pCurrentGib);
                if (pActor)
                    pActor->GetController()->SetDisabled(true);
                // Set the list order to be at the end so new objects are added there
                m_GibListOrder = -1;
                // Update the object
                m_pCurrentGib->Update();
                // If done picking, revert to moving object mode
                if (m_pPicker->DonePicking())
                {
                    m_EditorGUIMode = ADDINGGIB;
                    UpdatePieMenu();
                }
            }
        }
    }

    if (!m_pPicker->IsVisible())
        g_SceneMan.SetScreenOcclusion(Vector(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

    /////////////////////////////////////
    // ADDING GIB MODE

    if (m_EditorGUIMode == ADDINGGIB && !m_pPieMenu->IsEnabled())
    {
        g_FrameMan.SetScreenText("Click to ADD a new gib to the edited object - Drag to place with precision", 0);

        m_DrawCurrentGib = true;

        // Trap the mouse cursor
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

        // Move the cursor according to analog or mouse input
        if (!analogInput.IsZero())
        {
            m_CursorPos += analogInput * 8;
            // Re-enable snapping only when the cursor is moved again
//            m_GridSnapping = true;
        }
        else if (!m_pController->GetMouseMovement().IsZero())
        {
            m_CursorPos += m_pController->GetMouseMovement();
            // Re-enable snapping only when the cursor is moved again
//            m_GridSnapping = true;
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
//            if (pressUp || pressRight || pressDown || pressLeft)
//                m_GridSnapping = true;
        }

        // Detect whether the cursor is in the air, or if it's overlapping some terrain
        Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
        m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;

        // Mousewheel is used as shortcut for getting next and prev items in teh picker's object list
        if (m_pController->IsState(SCROLL_UP) || m_pController->IsState(ControlState::ACTOR_NEXT))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetPrevObject();
            if (pNewObject)
            {
                delete m_pCurrentGib;
                m_pCurrentGib = dynamic_cast<MovableObject *>(pNewObject->Clone());
                // Disable any controller, if an actor
                Actor *pActor = dynamic_cast<Actor *>(m_pCurrentGib);
                if (pActor)
                    pActor->GetController()->SetDisabled(true);
                // Update the object
                m_pCurrentGib->Update();
            }
        }
        else if (m_pController->IsState(SCROLL_DOWN) || m_pController->IsState(ControlState::ACTOR_PREV))
        {
            // Assign a copy of the next picked object to be the currently held one.
            const SceneObject *pNewObject = m_pPicker->GetNextObject();
            if (pNewObject)
            {
                delete m_pCurrentGib;
                m_pCurrentGib = dynamic_cast<MovableObject *>(pNewObject->Clone());
                // Disable any controller, if an actor
                Actor *pActor = dynamic_cast<Actor *>(m_pCurrentGib);
                if (pActor)
                    pActor->GetController()->SetDisabled(true);
                // Update the object
                m_pCurrentGib->Update();
            }
        }

        // Start the timer when the button is first pressed, and when the picker has deactivated
        if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_BlinkTimer.Reset();
            m_EditorGUIMode = PLACINGGIB;
            m_PreviousMode = ADDINGGIB;
            UpdatePieMenu();
            g_GUISound.PlacementBlip()->Play();
        }
    }

    /////////////////////////////////////////////////////////////
    // PLACING MODE

    else if (m_EditorGUIMode == PLACINGGIB)
    {
        if (m_PreviousMode == MOVINGGIB)
            g_FrameMan.SetScreenText("Click and drag on a placed gib to MOVE it - Click quickly to DETACH", 0);
        else
            g_FrameMan.SetScreenText("Click to ADD a new gib to the edited object - Drag to place with precision", 0);

        m_DrawCurrentGib = true;

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
        }

        // Disable snapping after a small interval of holding down the button, to avoid unintentional nudges when just placing on the grid
        if (m_pController->IsState(PRIMARY_ACTION) && m_BlinkTimer.IsPastRealMS(333) && m_GridSnapping)
        {
            m_GridSnapping = false;
            m_CursorPos = g_SceneMan.SnapPosition(m_CursorPos);
        }

        if (m_pController->IsState(RELEASE_PRIMARY))

        // Cancel placing if secondary button is pressed
        if (m_pController->IsState(PRESS_SECONDARY) || m_pController->IsState(PIE_MENU_ACTIVE))
        {
            m_EditorGUIMode = m_PreviousMode;
            UpdatePieMenu();
        }
        // If previous mode was moving, tear the gib loose if the button is released to soo
        else if (m_PreviousMode == MOVINGGIB && m_pController->IsState(RELEASE_PRIMARY) && !m_BlinkTimer.IsPastRealMS(150))
        {
            m_EditorGUIMode = ADDINGGIB;
            UpdatePieMenu();
        }
        // Only place if the picker and pie menus are completely out of view, to avoid immediate placing after picking
        else if (m_pCurrentGib && m_pController->IsState(RELEASE_PRIMARY) && !m_pPicker->IsVisible())
        {
            m_pCurrentGib->Update();

            // Add to the placed objects list
            AddPlacedObject(dynamic_cast<MovableObject *>(m_pCurrentGib->Clone()), m_GibListOrder);
            // Increment the list order so we place over last placed item
            if (m_GibListOrder >= 0)
                m_GibListOrder++;
            g_GUISound.PlacementThud()->Play();
//                g_GUISound.PlacementGravel()->Play();
            m_EditMade = true;

// TEMP REMOVE WEHN YOU CLEAN UP THE ABOVE HARDCODED BRAIN PLACEMENT
            if (m_EditorGUIMode != PICKINGGIB)
// TEMP REMOVE ABOVE
            // Go back to previous mode
            m_EditorGUIMode = m_PreviousMode;
            UpdatePieMenu();
        }

        // Set the facing of AHumans based on right/left cursor movements
        AHuman *pAHuman = dynamic_cast<AHuman *>(m_pCurrentGib);
        if (pAHuman)
            pAHuman->SetHFlipped(m_FacingLeft);
    }

    /////////////////////////////////////////////////////////////
    // POINTING AT MODES

    else if ((m_EditorGUIMode == MOVINGGIB || m_EditorGUIMode == DELETINGGIB || m_EditorGUIMode == PLACEINFRONT || m_EditorGUIMode == PLACEBEHIND) && !m_pPieMenu->IsEnabled())
    {
        m_DrawCurrentGib = false;

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
        // MOVING GIB MODE

        if (m_EditorGUIMode == MOVINGGIB)
        {
            g_FrameMan.SetScreenText("Click and drag on a placed gib to MOVE it - Click quickly to DETACH", 0);

            // Pick an object under the cursor and start moving it
            if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible())
            {
                const MovableObject *pPicked = PickPlacedObject(m_CursorPos, &m_GibListOrder);
                if (pPicked)
                {
                    // Grab the position and a copy of the the object itself before killing it from the scene
                    m_pCurrentGib = dynamic_cast<MovableObject *>(pPicked->Clone());
                    m_CursorOffset = m_CursorPos - m_pCurrentGib->GetPos();
                    RemovePlacedObject(m_GibListOrder);
                    m_EditMade = true;

                    // Go to placing mode to move it around
                    m_EditorGUIMode = PLACINGGIB;
                    m_PreviousMode = MOVINGGIB;
                    UpdatePieMenu();
                    m_BlinkTimer.Reset();
                    g_GUISound.PlacementBlip()->Play();
                    g_GUISound.PlacementGravel()->Play();
                }
                else
                    g_GUISound.UserErrorSound()->Play();
            }
        }

        ////////////////////////////
        // REMOVING GIB MODE

        else if (m_EditorGUIMode == DELETINGGIB)
        {
            g_FrameMan.SetScreenText("Click and hold to select an object - release to DELETE it", 0);

            // When primary is held down, pick object and show which one will be nuked if released
            if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible())
            {
                m_pObjectToBlink = PickPlacedObject(m_CursorPos);
            }
            else if (m_pController->IsState(RELEASE_PRIMARY))
            {
                if (PickPlacedObject(m_CursorPos, &m_GibListOrder))
                {
                    // Nuke it!
                    RemovePlacedObject(m_GibListOrder);
                    m_EditMade = true;
// TODO: Add awesome destruction sound here
                }
                else
                    g_GUISound.UserErrorSound()->Play();
            }
        }

        /////////////////////////////////////
        // PLACE IN FRONT AND BEHIND OF MODES

        else if (m_EditorGUIMode == PLACEINFRONT || m_EditorGUIMode == PLACEBEHIND)
        {
            if (m_EditorGUIMode == PLACEINFRONT)
                g_FrameMan.SetScreenText("Click an object to place the next one IN FRONT of it", 0);
            else if (m_EditorGUIMode == PLACEBEHIND)
            g_FrameMan.SetScreenText("Click an object to place the next one BEHIND it", 0);

            // When primary is held down, pick object and show which one will be nuked if released
            if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible())
            {
                m_pObjectToBlink = PickPlacedObject(m_CursorPos);
            }
            else if (m_pController->IsState(RELEASE_PRIMARY))
            {
                if (PickPlacedObject(m_CursorPos, &m_GibListOrder))
                {
                    // Adjust the next list order to be in front if applicable (it's automatically behind if same order index)
                    if (m_EditorGUIMode == PLACEINFRONT)
                        m_GibListOrder++;

                    // Go back to previous mode
                    m_EditorGUIMode = m_PreviousMode;
                    UpdatePieMenu();
                }
                else
                    g_GUISound.UserErrorSound()->Play();
            }
        }
    }

    // Remove cursor offset if not applicable anymore
    if (m_EditorGUIMode != PLACINGGIB)
        m_CursorOffset.Reset();

    // Keep the cursor position within the world
    bool cursorWrapped = g_SceneMan.ForceBounds(m_CursorPos);
// TODO: make setscrolltarget with 'sloppy' target
    // Scroll to the cursor's scene position
    g_SceneMan.SetScrollTarget(m_CursorPos, 0.3, cursorWrapped, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
    // Apply the cursor position to the currently held object
    if (m_pCurrentGib && m_DrawCurrentGib)
    {
		Vector gibPos = g_SceneMan.SnapPosition(m_CursorPos - m_CursorOffset, m_GridSnapping);
		gibPos.SetX(static_cast<int>(gibPos.m_X));
		gibPos.SetY(static_cast<int>(gibPos.m_Y));

        m_pCurrentGib->SetPos(gibPos);
        m_pCurrentGib->Update();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void GibEditorGUI::Draw(BITMAP *pTargetBitmap, const Vector &targetPos) const
{
    // Done, so don't draw the UI
    if (m_EditorGUIMode == DONEEDITING)
        return;

    // Draw all already placed Objects, and the currently held one in the order it is about to be placed in the scene
    int i = 0;
    for (list<MovableObject *>::const_iterator itr = m_PlacedGibs.begin(); itr != m_PlacedGibs.end(); ++itr, ++i)
    {
        // Draw the currently held object into the order of the list if it is to be placed inside
        if (m_pCurrentGib && m_DrawCurrentGib && i == m_GibListOrder)
        {
            g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGGIB ? LessTrans : HalfTrans);
            m_pCurrentGib->Draw(pTargetBitmap, targetPos, g_DrawTrans);
            Actor *pActor = dynamic_cast<Actor *>(m_pCurrentGib);
            if (pActor)
                pActor->DrawHUD(pTargetBitmap, targetPos);
        }

        // Blink trans if we are supposed to blink this one
        if ((*itr) == m_pObjectToBlink)
        {
            g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) ? LessTrans : HalfTrans);
            (*itr)->Draw(pTargetBitmap, targetPos, g_DrawTrans);
        }
        else
            (*itr)->Draw(pTargetBitmap, targetPos);

        // Draw basic HUD if an actor
        Actor *pActor = dynamic_cast<Actor *>(*itr);
        if (pActor)
            pActor->DrawHUD(pTargetBitmap, targetPos);
    }

    // Draw picking object crosshairs and not the selected object
    if (!m_DrawCurrentGib)
    {
        Vector center = m_CursorPos - targetPos;
        putpixel(pTargetBitmap, center.m_X, center.m_Y, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X - 5, center.m_Y, center.m_X - 2, g_YellowGlowColor);
        hline(pTargetBitmap, center.m_X + 5, center.m_Y, center.m_X + 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y - 5, center.m_Y - 2, g_YellowGlowColor);
        vline(pTargetBitmap, center.m_X, center.m_Y + 5, center.m_Y + 2, g_YellowGlowColor);
    }
    // If the held object will be placed at the end of the list, draw it last to the scene, transperent blinking
    else if (m_pCurrentGib && (m_GibListOrder < 0 || m_GibListOrder == m_PlacedGibs.size()))
    {
        g_FrameMan.SetTransTable(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGGIB ? LessTrans : HalfTrans);
        m_pCurrentGib->Draw(pTargetBitmap, targetPos, g_DrawTrans);
        Actor *pActor = dynamic_cast<Actor *>(m_pCurrentGib);
        if (pActor)
            pActor->DrawHUD(pTargetBitmap, targetPos);
    }

    // Draw the zoom window, if active
    if (m_ZoomFactor > 1)
    {
        Vector sourceCenter = m_CursorPos - targetPos;

        // Make sure the source is within the target bitmap
        int halfWidth = m_pZoomSource->w / 2;
        if (sourceCenter.m_X - halfWidth < 0)
            sourceCenter.m_X = halfWidth;
        else if (sourceCenter.m_X + halfWidth >= pTargetBitmap->w)
            sourceCenter.m_X = pTargetBitmap->w - halfWidth;
        int halfHeight = m_pZoomSource->w / 2;
        if (sourceCenter.m_Y - halfHeight < 0)
            sourceCenter.m_Y = halfHeight;
        else if (sourceCenter.m_Y + halfHeight >= pTargetBitmap->h)
            sourceCenter.m_Y = pTargetBitmap->h - halfHeight;

        // Copy to the intermediate source bitmap
        blit(pTargetBitmap, m_pZoomSource, sourceCenter.m_X - halfWidth, sourceCenter.m_Y - halfHeight, 0, 0, m_pZoomSource->w, m_pZoomSource->h);

        
        Vector zoomedCenter = m_CursorPos - targetPos;

        // Make sure the zoomed view is within the target bitmap
        halfWidth = (m_pZoomSource->w / 2) * m_ZoomFactor;
        if (zoomedCenter.m_X - halfWidth < 0)
            zoomedCenter.m_X = halfWidth;
        else if (zoomedCenter.m_X + halfWidth >= pTargetBitmap->w)
            zoomedCenter.m_X = pTargetBitmap->w - halfWidth;
        halfHeight = (m_pZoomSource->w / 2) * m_ZoomFactor;
        if (zoomedCenter.m_Y - halfHeight < 0)
            zoomedCenter.m_Y = halfHeight;
        else if (zoomedCenter.m_Y + halfHeight >= pTargetBitmap->h)
            zoomedCenter.m_Y = pTargetBitmap->h - halfHeight;

        // Then draw right back but stretched to the target
        stretch_blit(m_pZoomSource, pTargetBitmap, 0, 0, m_pZoomSource->w, m_pZoomSource->h, zoomedCenter.m_X - halfWidth, zoomedCenter.m_Y - halfHeight, m_pZoomSource->w * m_ZoomFactor, m_pZoomSource->h * m_ZoomFactor);
        rect(pTargetBitmap, zoomedCenter.m_X - halfWidth, zoomedCenter.m_Y - halfHeight, zoomedCenter.m_X + halfWidth - 1, zoomedCenter.m_Y + halfHeight - 1, g_YellowGlowColor);
    }

    m_pPicker->Draw(pTargetBitmap);

    // Draw the pie menu
    m_pPieMenu->Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePieMenu
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the PieMenu config based ont eh current editor state.

void GibEditorGUI::UpdatePieMenu()
{
    m_pPieMenu->ResetSlices();

    // Add pie menu slices and align them
//    m_pPieMenu->AddSlice(PieSlice("New Object", PieSlice::PSI_NEW, PieSlice::UP));
	PieSlice loadObjectSlice("Load Object", PieSlice::PieSliceIndex::PSI_LOAD, PieSlice::SliceDirection::UP);
    m_pPieMenu->AddSlice(loadObjectSlice);
    PieSlice testGibSlice("Test Gib Object", PieSlice::PieSliceIndex::PSI_DONE, PieSlice::SliceDirection::UP);
	m_pPieMenu->AddSlice(testGibSlice);
	
	PieSlice moveGibSlice("(Re)Move Gibs", PieSlice::PieSliceIndex::PSI_MOVE, PieSlice::SliceDirection::LEFT);
    m_pPieMenu->AddSlice(moveGibSlice);
    PieSlice deleteGibSlice("Delete Gibs", PieSlice::PieSliceIndex::PSI_REMOVE, PieSlice::SliceDirection::LEFT);
	m_pPieMenu->AddSlice(deleteGibSlice);
	
	PieSlice addGibSlice("Add New Gib", PieSlice::PieSliceIndex::PSI_PICK, PieSlice::SliceDirection::RIGHT);
    m_pPieMenu->AddSlice(addGibSlice);
    PieSlice zoomInSlice("Zoom In", PieSlice::PieSliceIndex::PSI_ZOOMIN, PieSlice::SliceDirection::RIGHT, m_ZoomFactor < MAXZOOMFACTOR);
	m_pPieMenu->AddSlice(zoomInSlice);
	
	PieSlice zoomOutSlice("Zoom Out", PieSlice::PieSliceIndex::PSI_ZOOMOUT, PieSlice::SliceDirection::RIGHT, m_ZoomFactor > MINZOOMFACTOR);
    m_pPieMenu->AddSlice(zoomOutSlice);
    PieSlice saveObjectSlice("Save Object", PieSlice::PieSliceIndex::PSI_SAVE, PieSlice::SliceDirection::DOWN);
	m_pPieMenu->AddSlice(saveObjectSlice);
    if (m_EditorGUIMode == ADDINGGIB)
    {
		PieSlice inFrontSlice("Put In Front Of", PieSlice::PieSliceIndex::PSI_INFRONT, PieSlice::SliceDirection::LEFT);
        m_pPieMenu->AddSlice(inFrontSlice);
        PieSlice behindSlice("Put Behind Of", PieSlice::PieSliceIndex::PSI_BEHIND, PieSlice::SliceDirection::LEFT);
		m_pPieMenu->AddSlice(behindSlice);
    }

    m_pPieMenu->RealignSlices();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to be placed in this scene. Ownership IS transferred!

void GibEditorGUI::AddPlacedObject(MovableObject *pObjectToAdd, int listOrder)
{
    if (!pObjectToAdd)
        return;

    if (listOrder < 0 || listOrder >= m_PlacedGibs.size())
        m_PlacedGibs.push_back(pObjectToAdd);
    else
    {
        // Find the spot
        list<MovableObject *>::iterator itr = m_PlacedGibs.begin();
        for (int i = 0; i != listOrder && itr != m_PlacedGibs.end(); ++i, ++itr)
            ;

        // Put 'er in
        m_PlacedGibs.insert(itr, pObjectToAdd);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemovePlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject placed in this scene.

void GibEditorGUI::RemovePlacedObject(int whichToRemove)
{
    if (m_PlacedGibs.empty())
        return;

    if (whichToRemove < 0 || whichToRemove >= m_PlacedGibs.size())
    {
        delete m_PlacedGibs.back();
        m_PlacedGibs.pop_back();
    }
    else
    {
        // Find the spot
        list<MovableObject *>::iterator itr = m_PlacedGibs.begin();
        for (int i = 0; i != whichToRemove && itr != m_PlacedGibs.end(); ++i, ++itr)
            ;

        delete (*itr);
        m_PlacedGibs.erase(itr);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed object that graphically overlaps an absolute
//                  point in the scene.

const MovableObject * GibEditorGUI::PickPlacedObject(Vector &scenePoint, int *pListOrderPlace) const
{
    // REVERSE!
    int i = m_PlacedGibs.size() - 1;
    for (list<MovableObject *>::const_reverse_iterator itr = m_PlacedGibs.rbegin(); itr != m_PlacedGibs.rend(); ++itr, --i)
    {
        if ((*itr)->IsOnScenePoint(scenePoint))
        {
            if (pListOrderPlace)
                *pListOrderPlace = i;
            return *itr;
        }
    }

    if (pListOrderPlace)
        *pListOrderPlace = -1;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updated the objects in the placed scene objects list of this. This is
//                  mostly for the editor to represent the items correctly.

void GibEditorGUI::UpdatePlacedObjects()
{
    for (list<MovableObject *>::iterator itr = m_PlacedGibs.begin(); itr != m_PlacedGibs.end(); ++itr)
    {
        (*itr)->Update();
    }
}