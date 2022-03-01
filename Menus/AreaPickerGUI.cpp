//////////////////////////////////////////////////////////////////////////////////////////
// File:            AreaPickerGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the AreaPickerGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AreaPickerGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUITab.h"
#include "GUIListBox.h"
#include "GUITextBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

#include "Controller.h"

using namespace RTE;

BITMAP *RTE::AreaPickerGUI::s_pCursor = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AreaPickerGUI, effectively
//                  resetting the members of this abstraction level only.

void AreaPickerGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_PickerEnabled = DISABLED;
    m_MenuSpeed = 0.3;
    m_ShowType.clear();
    m_SelectedGroupIndex = 0;
    m_SelectedAreaIndex = 0;
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_pParentBox = 0;
    m_pAreasList = 0;
    m_pDeleteAreaButton = 0;
    m_pPickedArea = 0;
    m_CursorPos.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AreaPickerGUI area ready for use.

int AreaPickerGUI::Create(Controller *pController, string onlyOfType)
{
    RTEAssert(pController, "No controller sent to AreaPickerGUI on creation!");
    m_pController = pController;

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(pController->GetPlayer());
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
	if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins", "DefaultSkin.ini")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/DefaultSkin.ini");
	}
    m_pGUIController->Load("Base.rte/GUIs/AreaPickerGUI.ini");
    m_pGUIController->EnableMouse(pController->IsMouseControlled());

    if (!s_pCursor)
    {
        ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
        s_pCursor = cursorFile.GetAsBitmap();
    }

    // Stretch the invisible root box to fill the screen
    dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    // Make sure we have convenient points to teh containing GUI colleciton boxes that we will manipulate the positions of
    if (!m_pParentBox)
    {
        m_pParentBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PickerGUIBox"));

        // Set the background image of the parent collection box
//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.png");
//        m_pParentBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
//        m_pParentBox->SetDrawBackground(true);
//        m_pParentBox->SetDrawType(GUICollectionBox::Image);
        m_pParentBox->SetDrawType(GUICollectionBox::Color);
    }
    m_pParentBox->SetPositionAbs(g_FrameMan.GetPlayerScreenWidth(), 0);
    m_pParentBox->SetEnabled(false);
    m_pParentBox->SetVisible(false);

    m_pAreasList = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("AreasLB"));
    m_pDeleteAreaButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("DeleteAreaButton"));

    // If we're not split screen horizontally, then stretch out the layout for all the relevant controls
    if (!g_FrameMan.GetHSplit())
    {
        int stretchAmount = g_FrameMan.GetResY() / 2;
        m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
        m_pAreasList->SetSize(m_pAreasList->GetWidth(), m_pAreasList->GetHeight() + stretchAmount);
        m_pDeleteAreaButton->SetPositionAbs(m_pDeleteAreaButton->GetXPos(), m_pDeleteAreaButton->GetYPos() + stretchAmount);
    }

    m_pAreasList->SetAlternateDrawMode(false);
    m_pAreasList->SetMultiSelect(false);

    // Populate the Areas list with the current Scene's Area:s
    UpdateAreasList();

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AreaPickerGUI area.

void AreaPickerGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void AreaPickerGUI::SetEnabled(bool enable)
{
    if (enable && m_PickerEnabled != ENABLED && m_PickerEnabled != ENABLING)
    {
        // If we're not split screen horizontally, then stretch out the layout for all the relevant controls
        int stretchAmount = g_FrameMan.GetPlayerScreenHeight() - m_pParentBox->GetHeight();
        if (stretchAmount != 0)
        {
            m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
            m_pAreasList->SetSize(m_pAreasList->GetWidth(), m_pAreasList->GetHeight() + stretchAmount);
        }

        m_PickerEnabled = ENABLING;
        // Reset repeat timers
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
        // Set the mouse cursor free
        g_UInputMan.TrapMousePos(false, m_pController->GetPlayer());
        // Move the mouse cursor to the middle of the player's screen
        int mouseOffX, mouseOffY;
        m_pGUIInput->GetMouseOffset(mouseOffX, mouseOffY);
        Vector mousePos(-mouseOffX + (g_FrameMan.GetPlayerScreenWidth() / 2), -mouseOffY + (g_FrameMan.GetPlayerScreenHeight() / 2));
        g_UInputMan.SetMousePos(mousePos, m_pController->GetPlayer());
        g_GUISound.EnterMenuSound()->Play();

        // Repopulate with the current Scene's list of Area:s
        UpdateAreasList();
    }
    else if (!enable && m_PickerEnabled != DISABLED && m_PickerEnabled != DISABLING)
    {
        m_PickerEnabled = DISABLING;
        // Trap the mouse cursor again
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());
        // Only play switching away sound
//        if (!m_pPickedArea)
            g_GUISound.ExitMenuSound()->Play();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void AreaPickerGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pGUIController->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next area in the areas list, even if the picker is disabled.

Scene::Area * AreaPickerGUI::GetNextArea()
{
    m_SelectedAreaIndex++;
    // Loop around
    if (m_SelectedAreaIndex >= m_pAreasList->GetItemList()->size())
        m_SelectedAreaIndex = 0;

    m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
    // Report the newly selected item as being 'picked', but don't close the picker
    GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
    if (pItem)
    {
        g_GUISound.SelectionChangeSound()->Play();
        return g_SceneMan.GetScene()->GetArea(pItem->m_Name);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the prev area in the areas list, even if the picker is disabled.

Scene::Area * AreaPickerGUI::GetPrevArea()
{
    m_SelectedAreaIndex--;
    // Loop around
    if (m_SelectedAreaIndex < 0)
        m_SelectedAreaIndex = m_pAreasList->GetItemList()->size() - 1;

    m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
    // Report the newly selected item as being 'picked', but don't close the picker
    GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
    if (pItem)
    {
        g_GUISound.SelectionChangeSound()->Play();
        return g_SceneMan.GetScene()->GetArea(pItem->m_Name);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateAreasList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all areas of a specific type already defined in PresetMan
//                  to the current Areas list

void AreaPickerGUI::UpdateAreasList(string selectAreaName)
{
    m_pAreasList->ClearList();

    if (g_SceneMan.GetScene() && !g_SceneMan.GetScene()->m_AreaList.empty())
    {
        Scene *pScene = g_SceneMan.GetScene();
        int indexToSelect = 0;
        // Add all the current Scene's Area:s to the list!
        for (list<Scene::Area>::iterator itr = pScene->m_AreaList.begin(); itr != pScene->m_AreaList.end(); ++itr)
        {
            m_pAreasList->AddItem((*itr).GetName());
            // If an Area's name matches the one we're supposed to leave selected after update, then save teh index
            if ((*itr).GetName() == selectAreaName)
                m_SelectedAreaIndex = indexToSelect;
            indexToSelect++;
        }

        m_pAreasList->ScrollToTop();
        // No actually select the Area with the matching name/index
        m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
        // Set the picked area to be the one now selected at the top
        GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
        if (pItem)
            m_pPickedArea = pScene->GetArea(pItem->m_Name);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void AreaPickerGUI::Update()
{
    // Enable mouse input if the controller allows it
    m_pGUIController->EnableMouse(m_pController->IsMouseControlled());

    // Reset the picked selector
    m_pPickedArea = 0;

    ////////////////////////////////////////////////////////////////////////
    // Animate the menu into and out of view if enabled or disabled

    if (m_PickerEnabled == ENABLING)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);

        Vector position, occlusion;

        float enabledPos = g_FrameMan.GetPlayerScreenWidth() - m_pParentBox->GetWidth();

        float toGo = std::floor((enabledPos - (float)m_pParentBox->GetXPos()) * m_MenuSpeed);
        position.m_X = m_pParentBox->GetXPos() + toGo;
        occlusion.m_X = m_pParentBox->GetXPos() - g_FrameMan.GetPlayerScreenWidth();

        m_pParentBox->SetPositionAbs(position.m_X, position.m_Y);
        g_SceneMan.SetScreenOcclusion(occlusion, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

        if (m_pParentBox->GetXPos() <= enabledPos)
            m_PickerEnabled = ENABLED;
    }
    // Animate the menu out of view
    else if (m_PickerEnabled == DISABLING)
    {
        float disabledPos = g_FrameMan.GetPlayerScreenWidth();

        float toGo = std::ceil((disabledPos - (float)m_pParentBox->GetXPos()) * m_MenuSpeed);
        m_pParentBox->SetPositionAbs(m_pParentBox->GetXPos() + toGo, 0);
        g_SceneMan.SetScreenOcclusion(Vector(m_pParentBox->GetXPos() - g_FrameMan.GetPlayerScreenWidth(), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

        if (m_pParentBox->GetXPos() >= g_FrameMan.GetPlayerScreenWidth())
        {
            m_pParentBox->SetEnabled(false);
            m_pParentBox->SetVisible(false);
            m_PickerEnabled = DISABLED;
        }
    }
    else if (m_PickerEnabled == ENABLED)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);
    }
    else if (m_PickerEnabled == DISABLED)
    {
        m_pParentBox->SetEnabled(false);
        m_pParentBox->SetVisible(false);
    }

    // Quit now if we aren't enabled
    if (m_PickerEnabled != ENABLED &&  m_PickerEnabled != ENABLING)
        return;

    // Update the user controller
//    m_pController->Update();


    /////////////////////////////////////////////////////
    // Mouse cursor logic

    int mouseX, mouseY;
    m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
    m_CursorPos.SetXY(mouseX, mouseY);

    /////////////////////////////////////////////
    // Repeating input logic

    bool pressLeft = m_pController->IsState(PRESS_LEFT);
    bool pressRight = m_pController->IsState(PRESS_RIGHT);
    bool pressUp = m_pController->IsState(PRESS_UP) || m_pController->IsState(SCROLL_UP);
    bool pressDown = m_pController->IsState(PRESS_DOWN) || m_pController->IsState(SCROLL_DOWN);

    // If no direciton is held down, then cancel the repeating
    if (!(/*m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || */m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN)))
    {
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
    }

    // Check if any direction has been held for the starting amount of time to get into repeat mode
    if (m_RepeatStartTimer.IsPastRealMS(200))
    {
        // Check for the repeat interval
        if (m_RepeatTimer.IsPastRealMS(75))
        {
/* L-R Not needed for picker
            if (m_pController->IsState(MOVE_RIGHT))
                pressRight = true;
            else if (m_pController->IsState(MOVE_LEFT))
                pressLeft = true;
*/
            if (m_pController->IsState(MOVE_UP))
                pressUp = true;
            else if (m_pController->IsState(MOVE_DOWN))
                pressDown = true;

            m_RepeatTimer.Reset();
        }
    }


    /////////////////////////////////////////
    // AREAS LIST

    int listSize = m_pAreasList->GetItemList()->size();
    if (pressDown)
    {
        m_SelectedAreaIndex++;
        // Loop around
        if (m_SelectedAreaIndex >= listSize)
            m_SelectedAreaIndex = 0;

        m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
        // Report the newly selected item as being 'picked', but don't close the picker
        GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
        if (pItem)
            m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name);
        g_GUISound.SelectionChangeSound()->Play();
    }
    else if (pressUp)
    {
        m_SelectedAreaIndex--;
        // Loop around
        if (m_SelectedAreaIndex < 0)
            m_SelectedAreaIndex = listSize - 1;

        m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
        // Report the newly selected item as being 'picked', but don't close the picker
        GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
        if (pItem)
            m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name);

        g_GUISound.SelectionChangeSound()->Play();
    }

    // Fire button picks the area and deactivates the picker GUI
    if (m_PickerEnabled == ENABLED && m_pController->IsState(PRESS_FACEBUTTON))
    {
        GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
        if (pItem)
        {
            // User has made final selection, so close the Picker
            if (m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name))
            {
                g_GUISound.AreaPickedSound()->Play();
                SetEnabled(false);
            }
        }
    }

    // Right click, or pie menu press close the menu
    if (m_pController->IsState(PRESS_SECONDARY))
    {
        GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
        if (pItem)
        {
            // User has made final selection, so close the Picker
            if (m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name))
            {
                g_GUISound.AreaPickedSound()->Play();
                SetEnabled(false);
            }
        }
    }

    //////////////////////////////////////////
	// Update the ControlManager

	m_pGUIController->Update();


    ////////////////////////////////////////////////////////
    // Handle events for mouse input on the controls

	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
        // If we're not supposed to have mouse control, then ignore these messages
        if (!m_pController->IsMouseControlled())
            break;

		if (anEvent.GetType() == GUIEvent::Command)
        {
            // Delete area button
			if(anEvent.GetControl() == m_pDeleteAreaButton)
            {
                UpdateAreasList();
                m_pDeleteAreaButton->SetFocus();

                GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
                // Only delete if there's more than one Area left
                if (pItem && g_SceneMan.GetScene()->m_AreaList.size() > 1)
                {
                    // Pick the next area, then remove the one specified
                    m_pPickedArea = GetNextArea();
                    g_SceneMan.GetScene()->RemoveArea(pItem->m_Name);
                    // Update the list so it shows the one removed
                    UpdateAreasList(m_pPickedArea->GetName());
                    g_GUISound.AreaPickedSound()->Play();
                }
                else
                    g_GUISound.UserErrorSound()->Play();
			}
        }
        else if (anEvent.GetType() == GUIEvent::Notification)
        {
            ///////////////////////////////////////////////
            // Clicks on the Areas List

            if (anEvent.GetControl() == m_pAreasList)
            {
                // Doubleclick
//                if(anEvent.GetMsg() == GUIListBox::DoubleClicked)
                if(anEvent.GetMsg() == GUIListBox::MouseDown)
                {
                    GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
                    if (pItem)
                    {
                        m_SelectedAreaIndex = m_pAreasList->GetSelectedIndex();
                        // User has made final selection, so close the Picker
                        if (m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name))
                        {
                            g_GUISound.AreaPickedSound()->Play();
                            SetEnabled(false);
                        }
                    }
                    // Undo the click deselection if nothing was selected
                    else
                        m_pAreasList->SetSelectedIndex(m_SelectedAreaIndex);
				}
            }
        }

        // If clicked outside the picker, then close the picker GUI
        if (anEvent.GetMsg() == GUIListBox::Click && m_PickerEnabled == ENABLED && m_CursorPos.m_X < m_pParentBox->GetXPos())
        {
            GUIListPanel::Item *pItem = m_pAreasList->GetSelected();
            if (pItem)
            {
                // User has made final selection, so close the Picker
                if (m_pPickedArea = g_SceneMan.GetScene()->GetArea(pItem->m_Name))
                {
                    g_GUISound.AreaPickedSound()->Play();
                    SetEnabled(false);
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void AreaPickerGUI::Draw(BITMAP *drawBitmap) const
{
    AllegroScreen drawScreen(drawBitmap);
    m_pGUIController->Draw(&drawScreen);

    // Draw the cursor on top of everything
    if (IsEnabled() && m_pController->IsMouseControlled())
        draw_sprite(drawBitmap, s_pCursor, m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY());
}