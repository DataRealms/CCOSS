//////////////////////////////////////////////////////////////////////////////////////////
// File:            ObjectPickerGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the ObjectPickerGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ObjectPickerGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"

#include "DataModule.h"
#include "Controller.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "ContentFile.h"
#include "EditorActivity.h"
#include "BunkerAssembly.h"
#include "BunkerAssemblyScheme.h"

using namespace RTE;

BITMAP *RTE::ObjectPickerGUI::s_pCursor = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ObjectPickerGUI, effectively
//                  resetting the members of this abstraction level only.

void ObjectPickerGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_PickerEnabled = DISABLED;
    m_PickerFocus = GROUPS;
    m_FocusChange = false;
    m_MenuSpeed = 0.3;
    m_ModuleSpaceID = -1;
    m_ShowType.clear();
    m_SelectedGroupIndex = 0;
    m_SelectedObjectIndex = 0;
    m_LastHoveredMouseIndex = 0;
    m_NativeTechModule = 0;
    m_ForeignCostMult = 4.0;
    int moduleCount = g_PresetMan.GetTotalModuleCount();
    m_aExpandedModules = new bool[moduleCount];
    for (int i = 0; i < moduleCount; ++i)
        m_aExpandedModules[i] = i == 0 ? true : false;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_pParentBox = 0;
    m_pPopupBox = 0;
    m_pPopupText = 0;
    m_pGroupsList = 0;
    m_pObjectsList = 0;
    m_pPickedObject = 0;
    m_CursorPos.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ObjectPickerGUI object ready for use.

int ObjectPickerGUI::Create(Controller *pController, int whichModuleSpace, string onlyOfType)
{
    RTEAssert(pController, "No controller sent to ObjectPickerGUI on creation!");
    m_pController = pController;

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(pController->GetPlayer());
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    if(!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/Base"))
        RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Base");
    m_pGUIController->Load("Base.rte/GUIs/ObjectPickerGUI.ini");
    m_pGUIController->EnableMouse(pController->IsMouseControlled());

    if (!s_pCursor)
    {
        ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
        s_pCursor = cursorFile.GetAsBitmap();
    }

    // Stretch the invisible root box to fill the screen
	if (g_FrameMan.IsInMultiplayerMode())
	{
		dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetPlayerFrameBufferWidth(pController->GetPlayer()), g_FrameMan.GetPlayerFrameBufferHeight(pController->GetPlayer()));
	}
	else
	{
		dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
	}

    // Make sure we have convenient points to teh containing GUI colleciton boxes that we will manipulate the positions of
    if (!m_pParentBox)
    {
        m_pParentBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PickerGUIBox"));

        // Set the background image of the parent collection box
//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.bmp");
//        m_pParentBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
//        m_pParentBox->SetDrawBackground(true);
//        m_pParentBox->SetDrawType(GUICollectionBox::Image);
        m_pParentBox->SetDrawType(GUICollectionBox::Color);
    }
    m_pParentBox->SetPositionAbs(g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()), 0);
    m_pParentBox->SetEnabled(false);
    m_pParentBox->SetVisible(false);

    if (!m_pPopupBox)
    {
        m_pPopupBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BuyGUIPopup"));
        m_pPopupText = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("PopupText"));

        m_pPopupBox->SetDrawType(GUICollectionBox::Panel);
	    m_pPopupBox->SetDrawBackground(true);
        // Never enable the popup, because it steals focus and cuases other windows to think teh cursor left them
        m_pPopupBox->SetEnabled(false);
        m_pPopupBox->SetVisible(false);
        // Set the font
        m_pPopupText->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));
    }

    m_pGroupsList = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("GroupsLB"));
    m_pObjectsList = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("ObjectsLB"));

    // If we're not split screen horizontally, then stretch out the layout for all the relevant controls
	if (g_FrameMan.IsInMultiplayerMode())
	{
		int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(pController->GetPlayer()) / 2;

		m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
		m_pGroupsList->SetSize(m_pGroupsList->GetWidth(), m_pGroupsList->GetHeight() + stretchAmount);
		m_pObjectsList->SetSize(m_pObjectsList->GetWidth(), m_pObjectsList->GetHeight() + stretchAmount);
	}
	else
	{
		if (!g_FrameMan.GetHSplit())
		{
			int stretchAmount = g_FrameMan.GetResY() / 2;

			m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
			m_pGroupsList->SetSize(m_pGroupsList->GetWidth(), m_pGroupsList->GetHeight() + stretchAmount);
			m_pObjectsList->SetSize(m_pObjectsList->GetWidth(), m_pObjectsList->GetHeight() + stretchAmount);
		}
	}

    m_pGroupsList->SetAlternateDrawMode(false);
    m_pObjectsList->SetAlternateDrawMode(true);
    m_pGroupsList->SetMultiSelect(false);
    m_pObjectsList->SetMultiSelect(false);
// Doing this manually instead in the mousemoved handling
//    m_pObjectsList->SetHotTracking(true);

    // Set initial focus, category list, and label settings
    m_PickerFocus = GROUPS;
    m_FocusChange = true;

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();

    // Set up the groups to show from the module space we are working within. This also updates the Objects list
    SetModuleSpace(whichModuleSpace);

    // Show only objects of this specific type
    ShowOnlyType(onlyOfType);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ObjectPickerGUI object.

void ObjectPickerGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    delete [] m_aExpandedModules;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void ObjectPickerGUI::SetEnabled(bool enable)
{
    if (enable && m_PickerEnabled != ENABLED && m_PickerEnabled != ENABLING)
    {
		if (g_FrameMan.IsInMultiplayerMode())
		{
			int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(m_pController->GetPlayer()) - m_pParentBox->GetHeight();
			if (stretchAmount != 0)
			{
				m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
				m_pGroupsList->SetSize(m_pGroupsList->GetWidth(), m_pGroupsList->GetHeight() + stretchAmount);
				m_pObjectsList->SetSize(m_pObjectsList->GetWidth(), m_pObjectsList->GetHeight() + stretchAmount);
			}
		}
		else
		{
			// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
			int stretchAmount = g_FrameMan.GetPlayerScreenHeight() - m_pParentBox->GetHeight();
			if (stretchAmount != 0)
			{
				m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
				m_pGroupsList->SetSize(m_pGroupsList->GetWidth(), m_pGroupsList->GetHeight() + stretchAmount);
				m_pObjectsList->SetSize(m_pObjectsList->GetWidth(), m_pObjectsList->GetHeight() + stretchAmount);
			}
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
        Vector mousePos(-mouseOffX + (g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()) / 2), -mouseOffY + (g_FrameMan.GetPlayerFrameBufferHeight(m_pController->GetPlayer()) / 2));
        g_UInputMan.SetMousePos(mousePos, m_pController->GetPlayer());
        // Default focus to the groups list if the objects are empty
        if (m_pObjectsList->GetItemList()->empty())
            m_PickerFocus = GROUPS;
        else
            m_PickerFocus = OBJECTS;
        m_FocusChange = true;
        g_GUISound.EnterMenuSound()->Play(m_pController->GetPlayer());
    }
    else if (!enable && m_PickerEnabled != DISABLED && m_PickerEnabled != DISABLING)
    {
        m_PickerEnabled = DISABLING;
        // Trap the mouse cursor again
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());
        // Only play switching away sound
//        if (!m_pPickedObject)
            g_GUISound.ExitMenuSound()->Play(m_pController->GetPlayer());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void ObjectPickerGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pGUIController->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.

void ObjectPickerGUI::SetModuleSpace(int moduleSpaceID)
{
    bool updateLists = m_ModuleSpaceID != moduleSpaceID;

    m_ModuleSpaceID = moduleSpaceID;

    // This updates the objects list too
    if (updateLists)
        UpdateGroupsList();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowSpecificGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes a group of a specific name show up in the picker, IF that group
//                  is currently shown!

bool ObjectPickerGUI::ShowSpecificGroup(string groupName)
{
    bool found = false;
    int index = 0;
    // Try to find the group
    for (vector<GUIListPanel::Item *>::iterator gItr = m_pGroupsList->GetItemList()->begin(); gItr != m_pGroupsList->GetItemList()->end(); ++gItr)
    {
        // Found it!
        if ((*gItr)->m_Name == groupName)
        {
            found = true;
            m_SelectedGroupIndex = index;
            // Select the found group
            m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex);
            // Select the first thigns in the objects list
            UpdateObjectsList();
            m_PickerFocus = OBJECTS;
            m_FocusChange = 1;
        }
        index++;
    }

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNativeTechModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule ID should be treated as the native tech of the
//                  user of this menu.

void ObjectPickerGUI::SetNativeTechModule(int whichModule)
{
    if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount())
    {
        // Set the multipliers and refresh everything that needs refreshing to reflect the change
        m_NativeTechModule = whichModule;
		// Object picker is first created with nativeTech = 0, which expands all modules but does not
		// display them as expanded. When visually collapsed module is first clicked, it internally turns 
		// itself into collapsed while the others display themselves as expanded. To avoid that, just don't
		// expand anything if we pass Base.rte as a native tech as Base.rte is always expanded by default 
		// anyway
		if (whichModule > 0)
			SetModuleExpanded(m_NativeTechModule);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleExpanded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether a data module shown in the item menu should be expanded
//                  or not.

void ObjectPickerGUI::SetModuleExpanded(int whichModule, bool expanded)
{
    int moduleCount = g_PresetMan.GetTotalModuleCount();
    if (whichModule > 0 && whichModule < moduleCount)
    {
        m_aExpandedModules[whichModule] = expanded;
        UpdateObjectsList(false);
    }
    // If base module (0), or out of range module, then affect all
    else
    {
        for (int m = 0; m < moduleCount; ++m)
            m_aExpandedModules[m] = expanded;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next object in the objects list, even if the picker is disabled.

const SceneObject * ObjectPickerGUI::GetNextObject()
{
    m_SelectedObjectIndex++;
    // Loop around
    if (m_SelectedObjectIndex >= m_pObjectsList->GetItemList()->size())
        m_SelectedObjectIndex = 0;

    m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
    // Report the newly selected item as being 'picked', but don't close the picker
    GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
    if (pItem)
    {
        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        return dynamic_cast<const SceneObject *>(pItem->m_pEntity);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the prev object in the objects list, even if the picker is disabled.

const SceneObject * ObjectPickerGUI::GetPrevObject()
{
    m_SelectedObjectIndex--;
    // Loop around
    if (m_SelectedObjectIndex < 0)
        m_SelectedObjectIndex = m_pObjectsList->GetItemList()->size() - 1;

    m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
    // Report the newly selected item as being 'picked', but don't close the picker
    GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
    if (pItem)
    {
        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        return dynamic_cast<const SceneObject *>(pItem->m_pEntity);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void ObjectPickerGUI::Update()
{
    // Enable mouse input if the controller allows it
    m_pGUIController->EnableMouse(m_pController->IsMouseControlled());

    // Reset the purchasing indicator
    m_pPickedObject = 0;

    // Popup box is hidden by default
    m_pPopupBox->SetVisible(false);

    ////////////////////////////////////////////////////////////////////////
    // Animate the menu into and out of view if enabled or disabled

    if (m_PickerEnabled == ENABLING)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);

        Vector position, occlusion;

        float enabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()) - m_pParentBox->GetWidth();

        float toGo = floorf((enabledPos - (float)m_pParentBox->GetXPos()) * m_MenuSpeed);
        position.m_X = m_pParentBox->GetXPos() + toGo;
        occlusion.m_X = m_pParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer());

        m_pParentBox->SetPositionAbs(position.m_X, position.m_Y);
        g_SceneMan.SetScreenOcclusion(occlusion, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

        if (m_pParentBox->GetXPos() <= enabledPos)
            m_PickerEnabled = ENABLED;
    }
    // Animate the menu out of view
    else if (m_PickerEnabled == DISABLING)
    {
        float disabledPos = g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer());

        float toGo = ceilf((disabledPos - (float)m_pParentBox->GetXPos()) * m_MenuSpeed);
        m_pParentBox->SetPositionAbs(m_pParentBox->GetXPos() + toGo, 0);
        g_SceneMan.SetScreenOcclusion(Vector(m_pParentBox->GetXPos() - g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
        m_pPopupBox->SetVisible(false);

        if (m_pParentBox->GetXPos() >= g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()))
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
        m_pPopupBox->SetVisible(false);
    }

    // Quit now if we aren't enabled
    if (m_PickerEnabled != ENABLED &&  m_PickerEnabled != ENABLING)
        return;

    // Update the user controller
//    m_pController->Update();

    ////////////////////////////////////////////
    // Notification blinking logic
/*
    if (m_BlinkMode == NOFUNDS)
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

    /////////////////////////////////////////////
    // Change focus as the user directs

    if (pressRight)
    {
        m_PickerFocus++;

        // Went too far
        if (m_PickerFocus >= FOCUSCOUNT)
        {
            m_PickerFocus = FOCUSCOUNT - 1;
            // Only play sound when the picker is completely deployed
            if (m_PickerEnabled == ENABLED)
                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        }
        else
            m_FocusChange = 1;
    }
    else if (pressLeft)
    {
        m_PickerFocus--;

        // Went too far
        if (m_PickerFocus < 0)
        {
            m_PickerFocus = 0;
            // Only play sound when the picker is completely deployed
            if (m_PickerEnabled == ENABLED)
                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        }
        else
            m_FocusChange = -1;
    }

    // Play focus change sound, if applicable
    if (m_FocusChange && m_PickerEnabled != ENABLING)
        g_GUISound.FocusChangeSound()->Play(m_pController->GetPlayer());

    /////////////////////////////////////////
    // GROUPS LIST focus

    if (m_PickerFocus == GROUPS)
    {
        if (m_FocusChange)
        {
            m_pGroupsList->SetFocus();
            // Select the top one in the item list if none is already selected
            if (!m_pGroupsList->GetItemList()->empty() && m_pGroupsList->GetSelectedIndex() < 0)
                m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex = 0);
            // Synch our index with the one already sleected in the list
            else
            {
                m_SelectedGroupIndex = m_pGroupsList->GetSelectedIndex();
                m_pGroupsList->ScrollToSelected();
            }

            m_FocusChange = 0;
        }

        int listSize = m_pGroupsList->GetItemList()->size();
        if (pressDown)
        {
            m_SelectedGroupIndex++;
            // Loop around
            if (m_SelectedGroupIndex >= listSize)
                m_SelectedGroupIndex = 0;

            m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex);

            GUIListPanel::Item *pItem = m_pGroupsList->GetSelected();
            if (pItem)
            {
                UpdateObjectsList();
                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
            }

//            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }
        else if (pressUp)
        {
            m_SelectedGroupIndex--;
            // Loop around
            if (m_SelectedGroupIndex < 0)
                m_SelectedGroupIndex = listSize - 1;

            m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex);

            GUIListPanel::Item *pItem = m_pGroupsList->GetSelected();
            if (pItem)
            {
                UpdateObjectsList();
                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
            }

//            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }

        // Move cursor to the object list if button is pressed on a groups list item
        if (m_pController->IsState(PRESS_FACEBUTTON))
        {
            GUIListPanel::Item *pItem = m_pGroupsList->GetItem(m_SelectedGroupIndex);
            if (pItem)
            {
                UpdateObjectsList();
                m_PickerFocus = OBJECTS;
                m_FocusChange = 1;
            }
        }
    }

    /////////////////////////////////////////
    // OBJECTS LIST focus

    else if (m_PickerFocus == OBJECTS)
    {
        // Changed to the list, so select the top one in the item list
        if (m_FocusChange)
        {
            m_pObjectsList->SetFocus();
            if (!m_pObjectsList->GetItemList()->empty() && m_pObjectsList->GetSelectedIndex() < 0)
                m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex = 0);
            // Synch our index with the one already selected in the list
            else
            {
                m_SelectedObjectIndex = m_pObjectsList->GetSelectedIndex();
                m_pObjectsList->ScrollToSelected();
            }

            // Set the picked object to be the one now selected
            GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
            if (pItem)
                m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity);

            m_FocusChange = 0;
        }

        int listSize = m_pObjectsList->GetItemList()->size();
        if (pressDown)
        {
            m_SelectedObjectIndex++;
            // Loop around
            if (m_SelectedObjectIndex >= listSize)
                m_SelectedObjectIndex = 0;

            m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
            // Report the newly selected item as being 'picked', but don't close the picker
            GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
            if (pItem)
                m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity);
            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }
        else if (pressUp)
        {
            m_SelectedObjectIndex--;
            // Loop around
            if (m_SelectedObjectIndex < 0)
                m_SelectedObjectIndex = listSize - 1;

            m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
            // Report the newly selected item as being 'picked', but don't close the picker
            GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
            if (pItem)
                m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity);

            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }

        // Get handle to the currently selected item, if any
        GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();

        // Show popup info box next to selected item, but only if it has a description
        string description = "";
        // Get it from the regular Entitiy preset
        if (pItem && pItem->m_pEntity && !pItem->m_pEntity->GetDescription().empty())
            description = pItem->m_pEntity->GetDescription();
        // Show popup info box next to selected module item, but only if it has a description
        else if (pItem && pItem->m_ExtraIndex >= 0)
        {
            const DataModule *pModule = g_PresetMan.GetDataModule(pItem->m_ExtraIndex);
            if (pModule && !pModule->GetDescription().empty())
                description = pModule->GetDescription();
        }

        // Now show the description, if we have any
        if (!description.empty())
        {
            // Show the popup box with the hovered item's description
            m_pPopupBox->SetVisible(true);
            // Need to add an offset to make it look better and not have the cursor obscure text
            m_pPopupBox->SetPositionAbs(m_pObjectsList->GetXPos() - m_pPopupBox->GetWidth() + 4, m_pObjectsList->GetYPos() + m_pObjectsList->GetStackHeight(pItem) - m_pObjectsList->GetScrollVerticalValue());
            // Make sure the popup box doesn't drop out of sight
            if (m_pPopupBox->GetYPos() + m_pPopupBox->GetHeight() > m_pParentBox->GetHeight())
                m_pPopupBox->SetPositionAbs(m_pPopupBox->GetXPos(), m_pParentBox->GetHeight() - m_pPopupBox->GetHeight());
            m_pPopupText->SetHAlignment(GUIFont::Right);
            m_pPopupText->SetText(description);
            // Resize the box height to fit the text
            int newHeight = m_pPopupText->ResizeHeightToFit();
            m_pPopupBox->Resize(m_pPopupBox->GetWidth(), newHeight + 10);
        }

        // Fire button picks the object and deactivates the picker GUI
        if (m_PickerEnabled == ENABLED && m_pController->IsState(PRESS_FACEBUTTON))
        {
            pItem = m_pObjectsList->GetSelected();
            // User pressed on a module group item; toggle its expansion!
            if (pItem && pItem->m_ExtraIndex >= 0)
            {
                // Make appropriate sound
                if (!m_aExpandedModules[pItem->m_ExtraIndex])
                    g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                // Different, maybe?
                else
                    g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                // Toggle the expansion of the module group item's items below
                m_aExpandedModules[pItem->m_ExtraIndex] = !m_aExpandedModules[pItem->m_ExtraIndex];
                // Re-populate the item list with the new module expansion configuation
                UpdateObjectsList(false);
            }
            // Selection of a regular item in the list
            else if (pItem)
            {
                // User has made final selection, so close the Picker
                if (m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
                {
                    g_GUISound.ObjectPickedSound()->Play(m_pController->GetPlayer());
                    SetEnabled(false);
                }
            }
        }
    }

    // Right click, or pie menu press close the menu
    if (m_pController->IsState(PRESS_SECONDARY))
    {
        GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
        if (pItem)
        {
            // User has made final selection, so close the Picker
            if (m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
            {
                g_GUISound.ObjectPickedSound()->Play(m_pController->GetPlayer());
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
			
        }
        else if (anEvent.GetType() == GUIEvent::Notification)
        {

            //////////////////////////////////////////
			// Clicks on the Groups List

			if(anEvent.GetControl() == m_pGroupsList)
            {
                // Regular click
                if(anEvent.GetMsg() == GUIListBox::MouseDown)
                {
                    GUIListPanel::Item *pItem = m_pGroupsList->GetSelected();
                    if (pItem)
                    {
                        m_SelectedGroupIndex = m_pGroupsList->GetSelectedIndex();
                        UpdateObjectsList();
                        g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                    }
                    // Undo the click deselection if nothing was selected
                    else
                        m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex);

                    m_PickerFocus = GROUPS;
				}
                // Mouse moved over the panel, show the popup with item description
                else if(anEvent.GetMsg() == GUIListBox::MouseMove)
                {
                    // Mouse is moving within the list's items, so make it focus on the list
//                    if (m_pGroupsList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y))
//                    {
                        m_pGroupsList->SetFocus();
                        m_PickerFocus = GROUPS;
//                    }
                    // Deselect the selection if we're fooling around with the mouse cursor outside any items, but still int he list area
//                    else if (m_PickerFocus == GROUPS)
//                        m_SelectedGroupIndex = -1;
                }
            }

            ///////////////////////////////////////////////
            // Clicks on the Objects List

            else if (anEvent.GetControl() == m_pObjectsList)
            {
/*
                // Somehting was just selected/highligted, so update the selection index to the new selected index
                if(anEvent.GetMsg() == GUIListBox::Select)
                {
                    if (m_SelectedObjectIndex != m_pObjectsList->GetSelectedIndex())
                        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                    m_SelectedObjectIndex = m_pObjectsList->GetSelectedIndex();
                }
                // MouseDown, picked somehting
                else*/ if (anEvent.GetMsg() == GUIListBox::MouseDown)
                {
                    GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
                    m_pObjectsList->ScrollToSelected();

                    // If a module group list item, toggle its expansion and update the list
                    if (pItem && pItem->m_ExtraIndex >= 0)
                    {
                        // Make appropriate sound
                        if (!m_aExpandedModules[pItem->m_ExtraIndex])
                            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                        // Different, maybe?
                        else
                            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                        // Toggle the expansion of the module group item's items below
                        m_aExpandedModules[pItem->m_ExtraIndex] = !m_aExpandedModules[pItem->m_ExtraIndex];
                        // Re-populate the item list with the new module expansion configuation
                        UpdateObjectsList(false);
                    }
                    // Only add if there's an entity attached to the list item
                    else if (pItem && pItem->m_pEntity)
                    {
                        m_SelectedObjectIndex = m_pObjectsList->GetSelectedIndex();
                        // User has made a pick, so close the Picker
                        if (m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
                        {
                            g_GUISound.ObjectPickedSound()->Play(m_pController->GetPlayer());
                            SetEnabled(false);
                        }
                    }
                    // Undo the click deselection if nothing was selected
//                    else
//                        m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);

                    m_PickerFocus = OBJECTS;
				}
                // Mouse moved over the panel, show the popup with item description
                else if(anEvent.GetMsg() == GUIListBox::MouseMove)
                {
                    // Mouse is moving within the list, so make it focus on the list
                    m_pObjectsList->SetFocus();
                    m_PickerFocus = OBJECTS;

                    // See if it's hovering over any item
                    GUIListPanel::Item *pItem = m_pObjectsList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y);
                    if (pItem)
                    {
                        // Don't let mouse movement change the index if it's still hovering inside the same item.
                        // This is to avoid erratic selection curosr if using both mouse and keyboard to work the menu
                        if (m_LastHoveredMouseIndex != pItem->m_ID)
                        {
                            m_LastHoveredMouseIndex = pItem->m_ID;

                            // Play select sound if new index
                            if (m_SelectedObjectIndex != pItem->m_ID)
                                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                            // Update the seleciton in both the GUI control and our menu
                            m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex = pItem->m_ID);
                        }
                    }
                    // Deselect the selection if we're fooling around with the mouse cursor outside any items, but still int he list area
//                    else if (m_PickerFocus == OBJECTS)
//                        m_SelectedObjectIndex = -1;
//                        m_pShopList->SetSelectedIndex(m_ListItemIndex);
//                        m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex = -1;
                }
            }
        }

        // If clicked outside the picker, then close the picker GUI
        if (anEvent.GetMsg() == GUIListBox::Click && m_PickerEnabled == ENABLED && m_CursorPos.m_X < m_pParentBox->GetXPos())
        {
            GUIListPanel::Item *pItem = m_pObjectsList->GetSelected();
            if (pItem)
            {
                // User has made final selection, so close the Picker
                if (m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
                {
                    g_GUISound.ObjectPickedSound()->Play(m_pController->GetPlayer());
                    SetEnabled(false);
                }
            }
        }
    }

/*
	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
		switch(anEvent.GetType())
        {
			// Command
			case GUIEvent::Command:

				// Quit
				if(anEvent.GetControl()->GetName() == "btnQuit")
					PostQuitMessage(0);

				if(anEvent.GetControl()->GetName() == "btnDown")
                {
					GUITextBox *T = (GUITextBox *)m_pGUIController->GetControl("text1");
					GUIComboBox *CB = (GUIComboBox *)m_pGUIController->GetControl("combo2");
					CB->AddItem(T->GetText());
				}


				if(anEvent.GetControl()->GetName() == "btnDown2")
                {
					GUIComboBox *CB = (GUIComboBox *)m_pGUIController->GetControl("combo2");
					GUIListBox *LB = (GUIListBox *)m_pGUIController->GetControl("list1");
					GUIListPanel::Item *I = CB->GetItem(CB->GetSelectedIndex());
					if(I)
						LB->AddItem(I->m_Name);
				}

				if(anEvent.GetControl()->GetName() == "btnMove")
                {
					GUIListBox *LB1 = (GUIListBox *)m_pGUIController->GetControl("list1");
					GUIListBox *LB2 = (GUIListBox *)m_pGUIController->GetControl("list2");

					vector<GUIListPanel::Item *> *List = LB1->GetSelectionList();
					vector<GUIListPanel::Item *>::iterator it;

					// Add the items in the selection list to the second list box
					for(it = List->begin(); it != List->end(); it++)
                    {
						GUIListPanel::Item *I = *it;

						LB2->AddItem(I->m_Name, I->m_Tag);

					}
				}
				break;

			// Notification
			case GUIEvent::Notification:

				// Slider changed
				if(anEvent.GetControl()->GetName() == "slider1")
                {
					int Value = 0;
					Value = ((GUISlider *)m_pGUIController->GetControl("slider1"))->GetValue();
					GUIProgressBar *P = (GUIProgressBar *)m_pGUIController->GetControl("progress1");
					P->SetValue(Value);
				}

				// Scrollbar changed
				if(anEvent.GetControl()->GetName() == "scroll1")
                {
					int Value = 0;
					GUIScrollbar *S = (GUIScrollbar *)anEvent.GetControl();
					Value = S->GetValue();
					GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
					char buf[64];
					L->SetText(itoa(Value, buf, 10));
				}

				// Double click on the listbox
				if(anEvent.GetControl()->GetName() == "list1")
                {
					if(anEvent.GetMsg() == GUIListBox::DoubleClicked)
                    {
						GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
						GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
						if(LB->GetSelected())
							L->SetText(LB->GetSelected()->m_Name);
					}

					if(anEvent.GetMsg() == GUIListBox::KeyDown)
                    {
						// Delete
						if(anEvent.GetData() == GUIInput::Key_Delete)
                        {
							GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
							LB->DeleteItem(LB->GetSelectedIndex());
						}
					}
				}

				if(anEvent.GetControl()->GetName() == "text1")
                {
					if(anEvent.GetMsg() == GUITextBox::Enter)
                    {
						GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
						GUITextBox *T = (GUITextBox *)anEvent.GetControl();

						L->SetText(T->GetText());
					}
				}
				break;
		}
	}
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void ObjectPickerGUI::Draw(BITMAP *drawBitmap) const
{
    AllegroScreen drawScreen(drawBitmap);
    m_pGUIController->Draw(&drawScreen);

    // Draw the cursor on top of everything
    if (IsEnabled() && m_pController->IsMouseControlled())
        draw_sprite(drawBitmap, s_pCursor, m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateGroupsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all groups with a specific type already defined in PresetMan
//                  to the current Objects list

void ObjectPickerGUI::UpdateGroupsList()
{
    // Load in all the groups that exist in the given modulespace
    m_pGroupsList->ClearList();

//    m_pGroupsList->AddItem("Bunker Modules");
/*
    m_pGroupsList->AddItem("Brains");
    m_pGroupsList->AddItem("Weapons");
    m_pGroupsList->AddItem("Clones");
    m_pGroupsList->AddItem("Robots");
    m_pGroupsList->AddItem("Brains");
*/
    // Get the registered groups of all official modules loaded before this + the specific module (official or not) one we're picking from
    list<string> groupList;
    g_PresetMan.GetModuleSpaceGroups(groupList, m_ModuleSpaceID, m_ShowType);
    list<Entity *> objectList;
    SceneObject *pSObject = 0;
    bool hasObjectsToShow = false;
	bool showSchemes = false;

	if (dynamic_cast<EditorActivity *>(g_ActivityMan.GetActivity()))
		showSchemes = true;

    for (list<string>::const_iterator gItr = groupList.begin(); gItr != groupList.end(); ++gItr)
    {
		bool onlyAssembliesInGroup = true;
		bool onlySchemesInGroup = true;

        // Get the actual object list for each group so we can check if they're empty or not
        objectList.clear();
        g_PresetMan.GetAllOfGroupInModuleSpace(objectList, *gItr, m_ShowType, m_ModuleSpaceID);

        // Go through the object list of this group and see if it contains any items we actually want to show
        hasObjectsToShow = false;
        for (list<Entity *>::iterator oItr = objectList.begin(); oItr != objectList.end(); ++oItr)
        {
			// Check if we have any other objects than assemblies to skip assembly groups.
			if (!dynamic_cast<BunkerAssembly *>(*oItr))
				onlyAssembliesInGroup = false;

			// Check if we have any other objects than schemes to skip schemes group.
			if (!dynamic_cast<BunkerAssemblyScheme *>(*oItr))
				onlySchemesInGroup = false;

            pSObject = dynamic_cast<SceneObject *>(*oItr);
            // Buyable?
            if (pSObject && pSObject->IsBuyable())
            {
                hasObjectsToShow = true;
                break;
            }
        }

		// If we have this assembly group in the list of visible assembly groups, then show it no matter what
		if (onlyAssembliesInGroup)
		{
			std::list<string> visibleAssemblyGroups = g_SettingsMan.GetVisibleAssemblyGroupsList();

			for (list<string>::const_iterator aItr = visibleAssemblyGroups.begin(); aItr != visibleAssemblyGroups.end(); ++aItr)
				if (*gItr == *aItr)
				{
					onlyAssembliesInGroup = false;
					break;
				}
		}

        // Only add the group if it has something in it!
        if (!objectList.empty() && hasObjectsToShow && (!onlyAssembliesInGroup || *gItr == "Assemblies") && (!onlySchemesInGroup || showSchemes))// && *gItr != "Bunker Modules")
            m_pGroupsList->AddItem(*gItr);
    }

    // Select and load the first group
    GUIListPanel::Item *pItem = m_pGroupsList->GetItem(0);
    if (pItem)
    {
        m_pGroupsList->SetSelectedIndex(m_SelectedGroupIndex = 0);
        UpdateObjectsList();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateObjectsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all objects of a specific type already defined in PresetMan
//                  to the current Objects list

void ObjectPickerGUI::UpdateObjectsList(bool selectTop)
{
    m_pObjectsList->ClearList();

    // The vector of lists which will be filled with objects, grouped by which data module they were read from
    vector<list<Entity *> > moduleList;
    int moduleID = 0;

    // Make as many datamodule entries as necessary in the vector
    while (moduleList.size() < g_PresetMan.GetTotalModuleCount())
        moduleList.push_back(list<Entity *>());

    // Get the name of the selected group
    GUIListPanel::Item *pItem = m_pGroupsList->GetSelected();
    if (pItem)
    {
        // Show objects from ALL modules
        if (m_ModuleSpaceID < 0)
        {
			if (g_SettingsMan.ShowForeignItems() || m_NativeTechModule <= 0)
			{
				// Go through ALL the data modules, gathering the objects that match the criteria in each one
				for (moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID)
					g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID);
			} else {
				for (moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID)
					if (moduleID == 0 || moduleID == m_NativeTechModule)
						g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID);
			}
        }
        // Only show objects from specific module space
        else
        {
            // Go through all the official data modules, gathering the objects that match the criteria in each one
            for (moduleID = 0; moduleID < g_PresetMan.GetOfficialModuleCount() && moduleID < m_ModuleSpaceID; ++moduleID)
                g_PresetMan.GetAllOfGroup(moduleList[moduleID], pItem->m_Name, m_ShowType, moduleID);

            // Now the the stuff from the current module, official or not
            g_PresetMan.GetAllOfGroup(moduleList[m_ModuleSpaceID], pItem->m_Name, m_ShowType, m_ModuleSpaceID);
        }
    }

    // Now fill the item list with the module-separated items of the currently selected group
    SceneObject *pSObject = 0;
    const DataModule *pModule = 0;
    GUIBitmap *pItemBitmap = 0;
    list<SceneObject *> tempList;
    // Go through all modules we've gathered from
    for (moduleID = 0; moduleID < moduleList.size(); ++moduleID)
    {
        // Don't add an empty module grouping
        if (!moduleList[moduleID].empty())
        {
            tempList.clear();

            // Move all valid/desired entities from the module list to the intermediate list
            for (list<Entity *>::iterator oItr = moduleList[moduleID].begin(); oItr != moduleList[moduleID].end(); ++oItr)
            {
                pSObject = dynamic_cast<SceneObject *>(*oItr);
                // Buyable?
                if (pSObject && pSObject->IsBuyable())
                    tempList.push_back(pSObject);
            }

            // Don't add anyhting to the real buy item list if the current module didn't yield any valid items
            if (!tempList.empty())
            {
                // Add the DataModule separator in the item list, with appropriate name and perhaps icon? Don't add for first base module
                if (moduleID != 0 && (pModule = g_PresetMan.GetDataModule(moduleID)))
                {
                    // Get the module icon so we can put it in the list separator
                    pItemBitmap = pModule->GetIcon() ? new AllegroBitmap(pModule->GetIcon()) : 0;
                    // Passing in ownership of the bitmap, making uppercase the name
                    string name = pModule->GetFriendlyName();
                    transform(name.begin(), name.end(), name.begin(), ::toupper);
                    m_pObjectsList->AddItem(name, m_aExpandedModules[moduleID] ? "-" : "+", pItemBitmap, 0, moduleID);
                }

                // If the module is expanded, add all the items within it below
                if (moduleID == 0 || m_aExpandedModules[moduleID])
                {
                    // Transfer from the temp intermediate list to the real gui list
                    for (list<SceneObject *>::iterator tItr = tempList.begin(); tItr != tempList.end(); ++tItr)
                    {
                        pItemBitmap = new AllegroBitmap((*tItr)->GetGraphicalIcon());
                        m_pObjectsList->AddItem((*tItr)->GetPresetName(), (*tItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *tItr);
                    }
                }
            }
        }
    }

    if (selectTop)
    {
        // Scroll the item list to the top
        m_pObjectsList->ScrollToTop();
        m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex = 0);
        // Set the picked object to be the one now selected at the top
        pItem = m_pObjectsList->GetSelected();
        if (pItem)
            m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity);
    }
    // Attempt to leave the selection where it was
    else
    {
        m_pObjectsList->SetSelectedIndex(m_SelectedObjectIndex);
        m_pObjectsList->ScrollToSelected();
        // Set the picked object to be the one now selected
        pItem = m_pObjectsList->GetSelected();
        if (pItem)
            m_pPickedObject = dynamic_cast<const SceneObject *>(pItem->m_pEntity);
    }
}
