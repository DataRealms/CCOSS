//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIComboBox.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIComboBox class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIComboBox.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIComboBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIComboBox object in
//                  system memory.

GUIComboBox::GUIComboBox(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
GUIControl()
{
    m_ControlID = "COMBOBOX";
    m_ControlManager = ControlManager;
    m_DrawBitmap = 0;
    m_OldSelection = 0;
    m_CreatedList = false;
    m_DropHeight = 80;
    m_DropDownStyle = DropDownList;

    // Create the listpanel
    m_ListPanel = new GUIListPanel(Manager);

    // Create the textpanel
    m_TextPanel = new GUITextPanel(Manager);

    // Create the button
    m_Button = new GUIComboBoxButton(Manager);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIComboBox::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 30;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 20;

    // Setup the panel
    m_X = X;
    m_Y = Y;
    m_Width = m_DefWidth;
    m_Height = m_DefHeight;

    if (Width != -1)
        m_Width = Width;
    if (Height != -1)
        m_Height = Height;

    // Make sure the textbox isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);
    
    m_TextPanel->Create(0, 0, m_Width-12, m_Height);    
    m_TextPanel->_SetVisible(true);
    m_TextPanel->SetLocked((m_DropDownStyle == DropDownList));
    m_TextPanel->SetSignalTarget(this);
    GUIPanel::AddChild(m_TextPanel);

    // Create the listpanel    
    m_ListPanel->_SetVisible(false);
    m_ListPanel->SetSignalTarget(this);
    m_ListPanel->SetMultiSelect(false);
    m_ListPanel->SetHotTracking(true);
    m_ListPanel->EnableScrollbars(false, true);
        
    // Create the button
    m_Button->Create(m_Width-17, 0, 17, m_Height);
    m_Button->SetSignalTarget(this);
    GUIPanel::AddChild(m_Button);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIComboBox::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 30;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 20;

    GUIPanel::LoadProperties(Props);

    // Make sure the textbox isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);
    
    m_TextPanel->Create(0, 0, m_Width-12, m_Height);    
    m_TextPanel->_SetVisible(true);
    m_TextPanel->SetSignalTarget(this);
    GUIPanel::AddChild(m_TextPanel);

    // Create the listpanel    
    m_ListPanel->_SetVisible(false);
    m_ListPanel->SetSignalTarget(this);
    m_ListPanel->SetMultiSelect(false);
    m_ListPanel->SetHotTracking(true);
    m_ListPanel->EnableScrollbars(false, true);
	m_ListPanel->SetMouseScrolling(true);
        

    // Create the button
    m_Button->Create(m_Width-17, 0, 17, m_Height);
    m_Button->SetSignalTarget(this);
    GUIPanel::AddChild(m_Button);

    // Load the info
    Props->GetValue("Dropheight", &m_DropHeight);
    m_DropHeight = MAX(m_DropHeight, 20);

    string Val;
    Props->GetValue("DropDownStyle", &Val);
    if (stricmp(Val.c_str(), "DropDownList") == 0)
        m_DropDownStyle = DropDownList;
    else if (stricmp(Val.c_str(), "DropDown") == 0)
        m_DropDownStyle = DropDown;

    m_TextPanel->SetLocked((m_DropDownStyle == DropDownList));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUIComboBox::Destroy(void)
{
    // Free the panels
    if (m_ListPanel) {
        m_ListPanel->Destroy();
        delete m_ListPanel;
        m_ListPanel = 0;
    }

    if (m_TextPanel) {
        delete m_TextPanel;
        m_TextPanel = 0;
    }

    if (m_Button) {
        m_Button->Destroy();
        delete m_Button;
        m_Button = 0;
    }

    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control is activated and ready for use.

void GUIComboBox::Activate(void)
{
    m_TextPanel->SetPositionAbs(m_X, m_Y);

    if (!m_ListPanel->GetParentPanel() && m_Parent) {
        m_Parent->AddChild(m_ListPanel);
        m_ListPanel->Create(m_X, m_Y+m_Height, m_Width, m_DropHeight);
        m_ListPanel->ChangeSkin(m_Skin);
        m_CreatedList = true;

        m_Button->Create(m_X+m_Width-17, m_Y, 17, m_Height);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIComboBox::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Free any old bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Create a new bitmap
    m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

    // Build the background
    m_Skin->BuildStandardRect(m_DrawBitmap, "TextBox", 0, 0, m_Width, m_Height);

    // Setup the skin in the panels too
    m_TextPanel->ChangeSkin(Skin);

    if (m_CreatedList)
        m_ListPanel->ChangeSkin(Skin);

    m_Button->ChangeSkin(Skin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIComboBox::Draw(GUIScreen *Screen)
{
    // Draw the background
    m_DrawBitmap->Draw(Screen->GetBitmap(), m_X, m_Y, 0);

// KLUDGE
    // If selected item has a bitmap AND no text to show, just show the bitmap as the selected thing
    if (m_ListPanel->GetSelected() && m_ListPanel->GetSelected()->m_Name.empty() && m_ListPanel->GetSelected()->m_pBitmap)
        m_ListPanel->GetSelected()->m_pBitmap->DrawTrans(Screen->GetBitmap(), m_X + 4, m_Y + 4, 0);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIComboBox::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.

void GUIComboBox::ReceiveSignal(GUIPanel* Source, int Code, int Data)
{
	assert(Source);

	int sourcePanelID = Source->GetPanelID();

	// ComboBoxButton
	if (sourcePanelID == m_Button->GetPanelID())
	{
		// Clicked and list panel is not visible. open the list panel.
		if (Code == GUIComboBoxButton::Clicked && !m_ListPanel->_GetVisible()) {
			m_ListPanel->_SetVisible(true);
			m_ListPanel->SetFocus();
			m_ListPanel->CaptureMouse();

			// Force a redraw
			m_ListPanel->EndUpdate();

			// Make this panel go above the rest
			m_ListPanel->ChangeZPosition(TopMost);

            // Save the current selection
            if (m_ListPanel->GetSelectedIndex() >= 0 && m_ListPanel->GetSelectedIndex() < m_ListPanel->GetItemList()->size())
                m_OldSelection = m_ListPanel->GetSelectedIndex();

			AddEvent(GUIEvent::Notification, Dropped, 0);
		}
	}

	// Textbox
	else if (sourcePanelID == m_TextPanel->GetPanelID()) {

		// MouseDown
		if (Code == GUITextPanel::MouseDown && m_DropDownStyle == DropDownList && Data & MOUSE_LEFT) {
			// Drop
			m_ListPanel->_SetVisible(true);
			m_ListPanel->SetFocus();
			m_ListPanel->CaptureMouse();

			// Force a redraw
			m_ListPanel->EndUpdate();

			// Make this panel go above the rest
			m_ListPanel->ChangeZPosition(TopMost);

            // Save the current selection
            if (m_ListPanel->GetSelectedIndex() >= 0 && m_ListPanel->GetSelectedIndex() < m_ListPanel->GetItemList()->size())
                m_OldSelection = m_ListPanel->GetSelectedIndex();

			AddEvent(GUIEvent::Notification, Dropped, 0);
		}

	}

	// ListPanel
	else if (sourcePanelID == m_ListPanel->GetPanelID()) {

		// MouseMove
		if (Code == GUIListPanel::MouseMove)// || Code == GUIListPanel::MouseUp)
		{
			m_Button->SetPushed(false);
			return;
		}

		int mouseX = 0;
		int mouseY = 0;
		m_Manager->GetInputController()->GetMousePosition(&mouseX, &mouseY);
		// Mouse down anywhere outside the list panel.
		if (Code == GUIListPanel::Click)
		{
			// Hide the list panel
			m_ListPanel->_SetVisible(false);
			m_ListPanel->ReleaseMouse();
			m_Manager->SetFocus(0);
			m_Button->SetPushed(false);

			// Restore the old selection
			m_ListPanel->SetSelectedIndex(m_OldSelection);

			AddEvent(GUIEvent::Notification, Closed, 0);
		}

		// Select on mouseup instead of down so we don't accidentally click stuff behind the disappearing listbox immediately after
		// Also only work if inside the actual list, and not its scrollbars
		else if (Code == GUIListPanel::MouseUp && m_ListPanel->PointInsideList(mouseX, mouseY)) {
			// Hide the list panel
			m_ListPanel->_SetVisible(false);
			m_ListPanel->ReleaseMouse();
			m_Manager->SetFocus(0);
			m_Button->SetPushed(false);

			AddEvent(GUIEvent::Notification, Closed, 0);

			// Set the text to the item in the list panel
			GUIListPanel::Item* Item = m_ListPanel->GetSelected();
			if (Item)
			{
				m_TextPanel->SetText(Item->m_Name);
				// Save the current selection - NO, don't, we want to keep this so we can still roll back to previous selection later
//                m_OldSelection = m_ListPanel->GetSelectedIndex();
			}
			else
			{
				// Restore the old selection
				m_ListPanel->SetSelectedIndex(m_OldSelection);
			}
		}

		if (m_DropDownStyle == DropDownList) {
			// Set the text to the item in the list panel
			GUIListPanel::Item* Item = m_ListPanel->GetSelected();
			if (Item)
				m_TextPanel->SetText(Item->m_Name);
		}

	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BeginUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Locks the control from updating every time a new item is added.

void GUIComboBox::BeginUpdate(void)
{
    m_ListPanel->BeginUpdate();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     UnLocks the control from updating every time a new item is added.
//                  Will automatically update the control.

void GUIComboBox::EndUpdate(void)
{
    m_ListPanel->EndUpdate();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Add an item to the list.

void GUIComboBox::AddItem(string Name, string ExtraText, GUIBitmap *pBitmap, const Entity *pEntity)
{
    m_ListPanel->AddItem(Name, ExtraText, pBitmap, pEntity);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the liost.

void GUIComboBox::ClearList(void)
{
    m_TextPanel->SetText("");
    m_ListPanel->ClearList();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIComboBox::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);

    m_ListPanel->SetPositionAbs(m_X, m_Y+m_Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIComboBox::Resize(int Width, int Height)
{
    // Make sure the textbox isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);
    Height = MIN(Height, 20);

    GUIPanel::SetSize(Width, Height);

    m_TextPanel->SetSize(m_Width-12, m_Height);
    m_TextPanel->SetPositionAbs(m_X, m_Y);

    m_Button->SetPositionAbs(m_X+m_Width-13, m_Y+1);
    m_Button->SetSize(12, m_Height-2);
    m_ListPanel->SetSize(m_Width, m_DropHeight);
    m_ListPanel->SetPositionAbs(m_X, m_Y+m_Height);

    // Force a bitmap rebuild
    ChangeSkin(m_Skin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIComboBox::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DeleteItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Delete an item from the list.

void GUIComboBox::DeleteItem(int Index)
{
    m_ListPanel->DeleteItem(Index);

    // Update the selection
    GUIListPanel::Item *Item = m_ListPanel->GetSelected();
    if (!Item) {
        m_ListPanel->SetSelectedIndex(0);
        Item = m_ListPanel->GetSelected();
    }

    if (Item)
        m_TextPanel->SetText(Item->m_Name);    
    else
        m_TextPanel->SetText("");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the item count.

int GUIComboBox::GetCount(void)
{
    return m_ListPanel->GetItemList()->size();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSelectedIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the index of the selected item.

int GUIComboBox::GetSelectedIndex(void)
{
    return m_ListPanel->GetSelectedIndex();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSelectedIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the index of the selected item.

void GUIComboBox::SetSelectedIndex(int Index)
{
    m_ListPanel->SetSelectedIndex(Index);
    m_OldSelection = Index;

    // Set the text to the item in the list panel
    GUIListPanel::Item *Item = m_ListPanel->GetSelected();
    if (Item)
        m_TextPanel->SetText(Item->m_Name);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RollbackSelection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rolls back the selection to the previous selected item.

bool GUIComboBox::RollbackSelection()
{
    // Restore the previous selection
    if (m_OldSelection >= 0 && m_OldSelection < m_ListPanel->GetItemList()->size() && m_OldSelection != m_ListPanel->GetSelectedIndex())
    {
        m_ListPanel->SetSelectedIndex(m_OldSelection);
        // Set the text to the item in the list panel
        GUIListPanel::Item *Item = m_ListPanel->GetSelected();
        if (Item)
        {
            m_TextPanel->SetText(Item->m_Name);
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the Item structure at the index.

GUIListPanel::Item *GUIComboBox::GetItem(int Index)
{
    return m_ListPanel->GetItem(Index);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDropHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drop height of the list.

void GUIComboBox::SetDropHeight(int Drop)
{
    m_DropHeight = Drop;
    m_DropHeight = MAX(m_DropHeight, 20);

    // Change the list panel
    m_ListPanel->SetSize(m_Width, m_DropHeight);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIComboBox::StoreProperties(void)
{
    m_Properties.AddVariable("DropHeight", m_DropHeight);
    if (m_DropDownStyle == DropDownList)
        m_Properties.AddVariable("DropDownStyle", "DropDownList");
    else if (m_DropDownStyle == DropDown)
        m_Properties.AddVariable("DropDownStyle", "DropDown");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDropDownStyle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drop down style of the combo box.

void GUIComboBox::SetDropDownStyle(int Style)
{
    if (Style == DropDown || Style == DropDownList)
        m_DropDownStyle = Style;

    m_TextPanel->SetLocked((m_DropDownStyle == DropDownList));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDropDownStyle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drop down style of the combo box.

int GUIComboBox::GetDropDownStyle(void)
{
    return m_DropDownStyle;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the visibility of the control.

void GUIComboBox::SetVisible(bool Visible)
{
    _SetVisible(Visible);
    if (!Visible && m_ListPanel)
        m_ListPanel->_SetVisible(false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the visibility of the control.

bool GUIComboBox::GetVisible(void)
{
    return _GetVisible();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the enabled state of the control.

void GUIComboBox::SetEnabled(bool Enabled)
{
    _SetEnabled(Enabled);
    if (m_ListPanel)
        m_ListPanel->_SetEnabled(Enabled);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the enabled state of the control.

bool GUIComboBox::GetEnabled(void)
{
    return _GetEnabled();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets text (only if style is DropDown).

string GUIComboBox::GetText(void)
{
    if (m_DropDownStyle != DropDown)
        return "";

    if (m_TextPanel)
        return m_TextPanel->GetText();

    return "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets text (only if style is DropDown).

void GUIComboBox::SetText(const string Text)
{
    if (m_DropDownStyle == DropDown && m_TextPanel)        
        m_TextPanel->SetText(Text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIComboBox::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Properties.GetValue("Dropheight", &m_DropHeight);
    m_DropHeight = MAX(m_DropHeight, 20);

    string Val;
    m_Properties.GetValue("DropDownStyle", &Val);
    if (stricmp(Val.c_str(), "DropDownList") == 0)
        m_DropDownStyle = DropDownList;
    else if (stricmp(Val.c_str(), "DropDown") == 0)
        m_DropDownStyle = DropDown;

    m_TextPanel->SetLocked((m_DropDownStyle == DropDownList));

    // Force a rebuild of the bitmap
    ChangeSkin(m_Skin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIComboBoxButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIComboBoxButton object in
//                  system memory.

GUIComboBoxButton::GUIComboBoxButton(GUIManager *Manager)
: GUIPanel(Manager)
{
    m_DrawBitmap = 0;
    m_Pushed = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIComboBoxButton::ChangeSkin(GUISkin *Skin)
{
    // Free any old bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Create a new bitmap. Same width, but double the height to allow for both up 
    // AND down states
    m_DrawBitmap = Skin->CreateBitmap(m_Width, m_Height*2);

    // Create the button image
    Skin->BuildStandardRect(m_DrawBitmap, "ComboBox_ButtonUp", 0, 0, m_Width, m_Height);
    Skin->BuildStandardRect(m_DrawBitmap, "ComboBox_ButtonDown", 0, m_Height, m_Width, m_Height);    

    // Draw the arrow
    string Filename;
    Skin->GetValue("ComboBox_Arrow", "Filename", &Filename);
    GUIBitmap *Arrow = Skin->CreateBitmap(Filename);
    if (!Arrow)
        return;

    unsigned long ColorKey;
    Skin->GetValue("ComboBox_Arrow", "ColorKeyIndex", &ColorKey);
    ColorKey = Skin->ConvertColor(ColorKey, m_DrawBitmap->GetColorDepth());
    Arrow->SetColorKey(ColorKey);

    int Values[4];
    GUIRect Rect;
    Skin->GetValue("ComboBox_Arrow", "Rect", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    Arrow->DrawTrans(m_DrawBitmap, 
                     (m_Width/2)-(Values[2]/2),
                     (m_Height/2)-(Values[3]/2),
                     &Rect);

    Arrow->DrawTrans(m_DrawBitmap, 
                     (m_Width/2)-(Values[2]/2)+1,
                     m_Height+(m_Height/2)-(Values[3]/2)+1,
                     &Rect);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIComboBoxButton::Draw(GUIScreen *Screen)
{
    GUIRect Rect;
    SetRect(&Rect, 0, m_Pushed ? m_Height : 0, m_Width, m_Pushed ? m_Height*2 : m_Height);

    m_DrawBitmap->Draw(Screen->GetBitmap(), m_X, m_Y, &Rect);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the panel.

void GUIComboBoxButton::Create(int X, int Y, int Width, int Height)
{
    m_X = X;
    m_Y = Y;
    m_Width = Width;
    m_Height = Height;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIComboBoxButton::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    if (Buttons & MOUSE_LEFT) {
        m_Pushed = true;
        SendSignal(Clicked, Buttons);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIComboBoxButton::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    m_Pushed = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPushed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the pushed state of the button.

void GUIComboBoxButton::SetPushed(bool Pushed)
{
    m_Pushed = Pushed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys the button.

void GUIComboBoxButton::Destroy(void)
{
    // Free the drawing bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }
}
