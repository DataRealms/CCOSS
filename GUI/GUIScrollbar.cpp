//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIScrollbar.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIScrollbar class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIScrollbar.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIScrollbar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIScrollbar object in
//                  system memory.

GUIScrollbar::GUIScrollbar(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIScrollPanel(Manager),
GUIControl()
{    
    
    m_ControlID = "SCROLLBAR";
    m_ControlManager = ControlManager;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIScrollbar::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 9;
    m_MinHeight = 9;

    // Note:: Default is Horizontal
    // Default size of the control
    m_DefWidth = 50;
    m_DefHeight = 12;

    // Create the ListPanel
    int w = m_DefWidth;
    int h = m_DefHeight;
    if (Width != -1)
        w = Width;
    if (Height != -1)
        h = Height;

    // Make sure the scrollpanel isn't too small
    w = MAX(w, m_MinWidth);
    h = MAX(h, m_MinHeight);
    
    GUIScrollPanel::Create(X, Y, w, h);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIScrollbar::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 9;
    m_MinHeight = 9;

    // Note:: Default is Horizontal
    // Default size of the control
    m_DefWidth = 50;
    m_DefHeight = 12;

    // Setup the panel
    GUIPanel::LoadProperties(Props);    

    // Make sure the scrollpanel isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    GUIScrollPanel::Create(m_X, m_Y, m_Width, m_Height);
    GUIScrollPanel::LoadProps(Props);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel has been destroyed.

void GUIScrollbar::Destroy(void)
{
    GUIScrollPanel::Destroy();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIScrollbar::ChangeSkin(GUISkin *Skin)
{
    GUIScrollPanel::ChangeSkin(Skin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIScrollbar::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.

void GUIScrollbar::ReceiveSignal(GUIPanel *Source, int Code, int Data)
{
    assert(Source);

    // Should be our scrollpanel
    if (Source->GetPanelID() == GetPanelID()) {
        if (Code == GUIScrollPanel::ChangeValue)
            AddEvent(GUIEvent::Notification, this->ChangeValue, 0);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIScrollbar::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    // Capture the mouse
    CaptureMouse();

    GUIScrollPanel::OnMouseDown(X, Y, Buttons, Modifier);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIScrollbar::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    // Release the mouse
    ReleaseMouse();

    GUIScrollPanel::OnMouseUp(X, Y, Buttons, Modifier);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIScrollbar::Move(int X, int Y)
{
    GUIScrollPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIScrollbar::Resize(int Width, int Height)
{
    // Make sure the control isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    GUIScrollPanel::SetSize(Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIScrollbar::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIScrollPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIScrollbar::StoreProperties(void)
{
    GUIScrollPanel::SaveProps(&m_Properties);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.
// Arguments:       GUIProperties.

void GUIScrollbar::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    GUIScrollPanel::LoadProps(&m_Properties);

    // Rebuild the bitmap
    BuildBitmap(true, true);
}