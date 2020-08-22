//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIListBox.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIListBox class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIListBox.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIListBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIListBox object in
//                  system memory.

GUIListBox::GUIListBox(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIListPanel(Manager),
GUIControl()
{    
    
    m_ControlID = "LISTBOX";
    m_ControlManager = ControlManager;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIListBox::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 50;
    m_MinHeight = 12;

    // Default size of the control
    m_DefWidth = 100;
    m_DefHeight = 100;

    // Create the ListPanel
    int w = m_DefWidth;
    int h = m_DefHeight;
    if (Width != -1)
        w = Width;
    if (Height != -1)
        h = Height;
    
    // Make sure the control isn't too small
    w = MAX(w, m_MinWidth);
    h = MAX(h, m_MinHeight);

    GUIListPanel::Create(X, Y, w, h);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIListBox::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 100;
    m_MinHeight = 12;

    // Default size of the control
    m_DefWidth = 100;
    m_DefHeight = 100;

    // Setup the panel
    GUIPanel::LoadProperties(Props);    

    // Make sure the listbox isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    GUIListPanel::Create(m_X, m_Y, m_Width, m_Height);

    // Get the properties
    bool Multi = false;
    Props->GetValue("MultiSelect", &Multi);
    SetMultiSelect(Multi);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUIListBox::Destroy(void)
{
    GUIListPanel::Destroy();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIListBox::ChangeSkin(GUISkin *Skin)
{
    GUIListPanel::ChangeSkin(Skin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIListBox::Move(int X, int Y)
{
    SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIListBox::Resize(int Width, int Height)
{
    // Make sure the listbox isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    SetSize(Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIListBox::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIListBox::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIListPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIListBox::StoreProperties(void)
{
    m_Properties.AddVariable("MultiSelect", GetMultiSelect());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.

void GUIListBox::ReceiveSignal(GUIPanel *Source, int Code, int Data)
{
    if (Source->GetPanelID() == GetPanelID())
    {
		if (Code == GUIListPanel::MouseMove) {
			AddEvent(GUIEvent::Notification, MouseMove, Data);
		} else if (Code == GUIListPanel::MouseEnter) {
			AddEvent(GUIEvent::Notification, MouseEnter, Data);
		} else if (Code == GUIListPanel::MouseLeave) {
			AddEvent(GUIEvent::Notification, MouseLeave, Data);
		} else if (Code == GUIListPanel::Select) {
			AddEvent(GUIEvent::Notification, Select, 0);
		} else if (Code == GUIListPanel::MouseDown) {
			AddEvent(GUIEvent::Notification, MouseDown, 0);
		} else if (Code == GUIListPanel::DoubleClick) {
			AddEvent(GUIEvent::Notification, DoubleClick, 0);
		} else if (Code == GUIListPanel::KeyDown) {
			AddEvent(GUIEvent::Notification, KeyDown, Data);
		} else if (Code == GUIListPanel::EdgeHit) {
			AddEvent(GUIEvent::Notification, EdgeHit, Data);
		}
    }

    GUIListPanel::ReceiveSignal(Source, Code, Data);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIListBox::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    bool Multi = false;
    m_Properties.GetValue("MultiSelect", &Multi);
    SetMultiSelect(Multi);

    // Rebuild the bitmap
    BuildBitmap(true, true);
}