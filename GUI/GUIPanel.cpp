// File:            GUIPanel.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the GUIPanel class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIPanel object in system
//                  memory.

GUIPanel::GUIPanel(GUIManager *Manager)
{
    Clear();
    m_Manager = Manager;
    m_Font = 0;
    m_FontColor = 0;
    m_FontShadow = 0;
    m_FontKerning = 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIPanel object in system
//                  memory.

GUIPanel::GUIPanel()
{
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the settings.

void GUIPanel::Clear(void)
{
    m_X = 0;
    m_Y = 0;
    m_Width = 0;
    m_Height = 0;
    m_ID = -1;
    m_GotFocus = false;
    m_Captured = false;
    m_Visible = true;
    m_Enabled = true;

    m_Parent = 0;
    m_Children.clear();

    m_Manager = 0;
    m_ValidRegion = false;
    m_SignalTarget = this;
    m_ZPos = 0;

    m_Font = 0;
    m_FontColor = 0;
    m_FontShadow = 0;
    m_FontKerning = 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Setup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the panel for use with the manager

void GUIPanel::Setup(GUIManager *manager, int ZPos)
{
    m_Manager = manager;
    m_ZPos = ZPos;

    // Request a new ID
    m_ID = m_Manager->GetPanelID();

    // Set the manager for all the children
    int Z = 0;
    std::vector<GUIPanel *>::iterator it;
    for(it = m_Children.begin(); it != m_Children.end(); it++) {
        GUIPanel *P = *it;
        if (P) {
            P->Setup(manager, Z++);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddChild
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a child to this panel

void GUIPanel::AddChild(GUIPanel *child, bool convertToAbsolutePos)
{
    if (child) {
        // Convert the child's coordinates into absolute coords
        if (convertToAbsolutePos)
        {
            child->m_X += m_X;
            child->m_Y += m_Y;
        }
/* This is BS.. clipping is done when drawing instead
        // A child panel cannot go outside of the boundaries of the parents rectangle
        // So we must clip the child's rectangle if that happens
        if (child->m_X + child->m_Width > m_X + m_Width)
            child->m_Width -= (child->m_X + child->m_Width) - (m_X + m_Width);

        if (child->m_Y + child->m_Height > m_Y + m_Height)
            child->m_Height -= (child->m_Y + child->m_Height) - (m_Y + m_Height);
*/
        // Make sure the rectangle is valid
        child->m_Width = std::max(child->m_Width, 0);
        child->m_Height = std::max(child->m_Height, 0);

        int zPos = 0;
        if (m_Children.size() > 0) {
			GUIPanel *lastChild = m_Children.back();
			zPos = lastChild->GetZPos() + 1;
        }

        // Remove the child from any previous parent
		if (child->GetParentPanel()) {
			child->GetParentPanel()->GUIPanel::RemoveChild(child);
		}

        // Setup the inherited values
        child->m_Parent = this;
        child->Setup(m_Manager, zPos);

        // Add the child to the list
        m_Children.push_back(child);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveChild
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a child based on name.

void GUIPanel::RemoveChild(const GUIPanel *pChild)
{
    // Note: We do NOT free the children because they are still linked in through their
    // controls. This merely removes the panel from the list.
    // This will cause a small memory leak, but this is only designed for the GUI Editor
    // and is a bit of a hack

    for(vector<GUIPanel *>::iterator itr = m_Children.begin(); itr != m_Children.end(); itr++)
    {
        GUIPanel *pPanel = *itr;
        if (pPanel && pPanel == pChild)
        {
            m_Children.erase(itr);
            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the base data from a properties page

void GUIPanel::LoadProperties(GUIProperties *Props)
{
    assert(Props);

    Props->GetValue("X", &m_X);
    Props->GetValue("Y", &m_Y);
    Props->GetValue("Width", &m_Width);
    Props->GetValue("Height", &m_Height);

    Props->GetValue("Visible", &m_Visible);
    Props->GetValue("Enabled", &m_Enabled);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Invalidate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Invalidates the panel

void GUIPanel::Invalidate(void)
{
    m_ValidRegion = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsValid
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if the panel is valid

bool GUIPanel::IsValid(void)
{
    return m_ValidRegion;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIPanel::Draw(GUIScreen *Screen)
{
    // Validate this panel
    m_ValidRegion = true;

    // Calculate this panel's clipping region - set the clipping rect to be the intersection of what
    // was already set by the parent, and the dimensions of this panel.
    Screen->GetBitmap()->AddClipRect(GetRect());
    // Now save this interseciton clipping rect so we can re-set it before each new child is drawn
    GUIRect thisClip;
    Screen->GetBitmap()->GetClipRect(&thisClip);

    // Draw children
    std::vector<GUIPanel *>::iterator it;
    for(it = m_Children.begin(); it != m_Children.end(); it++) {
        GUIPanel *P = *it;

        if (P->_GetVisible())
        {
            // Re-set the clipping rect of this panel since the last child has messed with it
            Screen->GetBitmap()->SetClipRect(&thisClip);
            P->Draw(Screen);
        }
    }

    // Get rid of the clipping rect since the parent will re-set it if necessary
    Screen->GetBitmap()->SetClipRect(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIPanel::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIPanel::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnDoubleClick
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse has double-clicked on the pane.

void GUIPanel::OnDoubleClick(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUIPanel::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.

void GUIPanel::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUIPanel::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseHover
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse is hovering over the panel (has to be enabled)

void GUIPanel::OnMouseHover(int X, int Y, int Buttons, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnKeyDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key goes down.

void GUIPanel::OnKeyDown(int KeyCode, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnKeyUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key goes up.

void GUIPanel::OnKeyUp(int KeyCode, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnKeyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key is pressed (OnDown & repeating).

void GUIPanel::OnKeyPress(int KeyCode, int Modifier)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnGainFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel gains focus.

void GUIPanel::OnGainFocus(void)
{
    m_GotFocus = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnLoseFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel looses focus.

void GUIPanel::OnLoseFocus(void)
{
    m_GotFocus = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Captures the mouse.

void GUIPanel::CaptureMouse(void)
{
    m_Manager->CaptureMouse(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReleaseMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Releases the mouse.

void GUIPanel::ReleaseMouse(void)
{
    m_Manager->ReleaseMouse();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BottomPanelUnderPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recursively goes down the tree to check the last panel under a point

GUIPanel *GUIPanel::BottomPanelUnderPoint(int x, int y)
{
    if (!PointInside(x, y))
        return 0;
    
    // If this panel is invisible or disabled, this panel is ignored
    if (!m_Visible || !m_Enabled)
        return 0;

    
    // Go through the children
    GUIPanel *CurPanel = 0;
    std::vector<GUIPanel *>::iterator it;
    for(it = m_Children.begin(); it != m_Children.end(); it++) {
        GUIPanel *P = *it;
        if (P) {
            CurPanel = P->BottomPanelUnderPoint(x, y);

            if (CurPanel != 0)
                return CurPanel;
        }
    }

    // Return this panel
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TopPanelUnderPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recursively goes up the tree from to check the first panel under a point

GUIPanel *GUIPanel::TopPanelUnderPoint(int x, int y)
{
    if (!PointInside(x, y))
        return 0;
    
    // If this panel is invisible or disabled, this panel is ignored
    if (!m_Visible || !m_Enabled)
        return 0;

    
    // Go through the children
    GUIPanel *CurPanel = 0;
    std::vector<GUIPanel *>::reverse_iterator it;
    for(it = m_Children.rbegin(); it != m_Children.rend(); it++) {
        GUIPanel *P = *it;
        if (P) {
            CurPanel = P->TopPanelUnderPoint(x, y);

            if (CurPanel != 0)
                return CurPanel;
        }
    }

    // Return this panel
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PointInside
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a point is inside the panel

bool GUIPanel::PointInside(int X, int Y)
{
    // Can't be inside an invisible panel
    if (!m_Visible)
        return false;

    if (X < m_X || Y < m_Y)
        return false;
    if (X > m_X+m_Width || Y > m_Y+m_Height)
        return false;

    // Mouse is inside
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adjusts the size of the panel.

void GUIPanel::SetSize(int Width, int Height)
{
    m_Width = Width;
    m_Height = Height;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPositionAbs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adjusts the position of the panel.

void GUIPanel::SetPositionAbs(int X, int Y, bool moveChildren)
{
    int DX = X-m_X;
    int DY = Y-m_Y;

    m_X = X;
    m_Y = Y;

    // Move children
    if (moveChildren)
    {
        std::vector<GUIPanel *>::iterator it;
        for(it = m_Children.begin(); it != m_Children.end(); it++) {
            GUIPanel *P = *it;
            P->SetPositionAbs(P->m_X+DX, P->m_Y+DY);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:    SetPositionRel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the position of the panel, relative to its parent.

void GUIPanel::SetPositionRel(int X, int Y)
{
    X += m_Parent->GetXPos();
    Y += m_Parent->GetYPos();

    int DX = X - m_X;
    int DY = Y - m_Y;

    m_X = X;
    m_Y = Y;

    // Move children
    std::vector<GUIPanel *>::iterator it;
    for(it = m_Children.begin(); it != m_Children.end(); it++) {
        GUIPanel *P = *it;
        P->SetPositionAbs(P->m_X + DX, P->m_Y + DY);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  MoveRelative
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Moves the position of the panel by a relative amount.
// Arguments:       X, Y, relative.

void GUIPanel::MoveRelative(int dX, int dY)
{
    m_X += dX;
    m_Y += dY;

    // Move children
    std::vector<GUIPanel *>::iterator it;
    for(it = m_Children.begin(); it != m_Children.end(); it++) {
        GUIPanel *P = *it;
        P->SetPositionAbs(P->m_X+dX, P->m_Y+dY);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  CenterInParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Centers this in its parent, taking this' dimensions into consideration.

void GUIPanel::CenterInParent(bool centerX, bool centerY)
{
    int newRelX = m_X - m_Parent->GetXPos();
    int newRelY = m_Y - m_Parent->GetYPos();

    if (centerX)
    {
        newRelX = (m_Parent->GetWidth() / 2) - (GetWidth() / 2);
    }
    if (centerY)
    {
        newRelY = (m_Parent->GetHeight() / 2) - (GetHeight() / 2);
    }

    SetPositionRel(newRelX, newRelY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _SetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the visibility of the panel.

void GUIPanel::_SetVisible(bool Visible)
{
    m_Visible = Visible;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _GetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the visibility of the panel.

bool GUIPanel::_GetVisible(void)
{
    return m_Visible;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the enabled state of the panel.

void GUIPanel::_SetEnabled(bool Enabled)
{
    m_Enabled = Enabled;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _GetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the enabled state of the panel.

bool GUIPanel::_GetEnabled(void)
{
    return m_Enabled;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the panel.

int GUIPanel::GetWidth(void)
{
    return m_Width;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the panel.

int GUIPanel::GetHeight(void)
{
    return m_Height;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the panel.

GUIRect *GUIPanel::GetRect(void)
{
    SetRect(&m_Rect, m_X, m_Y, m_X+m_Width, m_Y+m_Height);
    
    return &m_Rect;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the panel.

void GUIPanel::GetRect(int *X, int *Y, int *Width, int *Height)
{
    if (X) *X = m_X;
    if (Y) *Y = m_Y;
    if (Width) *Width = m_Width;
    if (Height) *Height = m_Height;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanelID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the panel's ID.

int GUIPanel::GetPanelID(void)
{
    return m_ID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrackMouseHover
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the manager to enable/disable hover tracking of this panel

void GUIPanel::TrackMouseHover(bool Enabled, int Delay)
{
    m_Manager->TrackMouseHover(this, Enabled, Delay);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the focus of this panel.

void GUIPanel::SetFocus(void)
{    
    m_Manager->SetFocus(this);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the focus value of the panel.

bool GUIPanel::HasFocus(void)
{
    return m_GotFocus;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCaptureState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the panel's captured state.

void GUIPanel::SetCaptureState(bool Captured)
{
    m_Captured = Captured;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsCaptured
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the panel's captured state.

bool GUIPanel::IsCaptured(void)
{
    return m_Captured;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the panel's enabled state.

bool GUIPanel::IsEnabled(void)
{
    return m_Enabled;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SendSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sends a signal to the target.

void GUIPanel::SendSignal(int Code, int Data)
{
    if (m_SignalTarget)
        m_SignalTarget->ReceiveSignal(this, Code, Data);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.

void GUIPanel::ReceiveSignal(GUIPanel *Source, int Code, int Data)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSignalTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the target panel to receive signals.

void GUIPanel::SetSignalTarget(GUIPanel *Target)
{
    if (Target)
        m_SignalTarget = Target;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParentPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the parent of this panel.

GUIPanel *GUIPanel::GetParentPanel(void)
{
    return m_Parent;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetZPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Z index of the panel.

void GUIPanel::SetZPos(int Z)
{
    m_ZPos = Z;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetZPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Z index of the panel.

int GUIPanel::GetZPos(void)
{
    return m_ZPos;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeZPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes the Z Position of the panel.

void GUIPanel::ChangeZPosition(int Type)
{
    // If we don't have a parent, get the manager to alter the Z Position
    if (!m_Parent) {
        //m_Manager->ChangeZPosition(this, Type);
        return;
    }

    // Get the parent to change the position
    m_Parent->_ChangeZ(this, Type);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _ChangeZ
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes the Z position of a child panel.

void GUIPanel::_ChangeZ(GUIPanel *Child, int Type)
{
    assert(Child);

    int Index = -1;

    // Find the child in our children list
    vector<GUIPanel *>::iterator it;
    int Count = 0;
    for(it = m_Children.begin(); it != m_Children.end(); it++, Count++) {
        GUIPanel *P = *it;
        if (P) {
            if (P->GetPanelID() == Child->GetPanelID()) {
                Index = Count;
                break;
            }
        }
    }

    // Didn't find the child
    if (Index == -1)
        return;

    switch(Type) {
        // Put the child at the end of the list
        case TopMost:
            m_Children.erase(m_Children.begin()+Index);
            m_Children.push_back(Child);
            break;

        // Put the child at the start of the list
        case BottomMost:
            m_Children.erase(m_Children.begin()+Index);
            m_Children.insert(m_Children.begin(), Child);
            break;
    }

    // Go through and re-order the Z positions
    Count = 0;
    for(it = m_Children.begin(); it != m_Children.end(); it++, Count++) {
        GUIPanel *P = *it;
        if (P)
            P->SetZPos(Count);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Convert the properties in the panel to a string.

string GUIPanel::ToString(void)
{
    string OutString = "";

    // Subtract the position from the parent
    int X = m_X;
    int Y = m_Y;

    if (m_Parent) {
        X -= m_Parent->m_X;
        Y -= m_Parent->m_Y;
    }


    OutString += WriteValue("X", X);
    OutString += WriteValue("Y", Y);
    OutString += WriteValue("Width", m_Width);
    OutString += WriteValue("Height", m_Height);
    OutString += WriteValue("Visible", m_Visible);
    OutString += WriteValue("Enabled", m_Enabled);

    return OutString;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds this panels properties to a properties class.

void GUIPanel::BuildProperties(GUIProperties *Prop)
{
    assert(Prop);

    // Subtract the position from the parent
    int X = m_X;
    int Y = m_Y;

    if (m_Parent) {
        X -= m_Parent->m_X;
        Y -= m_Parent->m_Y;
    }

    Prop->AddVariable("X", X);
    Prop->AddVariable("Y", Y);
    Prop->AddVariable("Width", m_Width);
    Prop->AddVariable("Height", m_Height);
    Prop->AddVariable("Visible", m_Visible);
    Prop->AddVariable("Enabled", m_Enabled);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WriteValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes a single value to string.

string GUIPanel::WriteValue(const string Name, int Value)
{
    char buf[32];

    string OutString = Name;
    OutString += " = ";

    std::snprintf(buf, sizeof(buf), "%i", Value);
    OutString += buf;
    OutString += "\n";

    return OutString;
}

    
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WriteValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes a single value to string.

string GUIPanel::WriteValue(const string Name, bool Value)
{
    string OutString = Name;
    OutString += " = ";
    OutString += (Value ? "True" : "False");
    OutString += "\n";

    return OutString;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          _ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the panel.

void GUIPanel::_ApplyProperties(GUIProperties *Props)
{
    assert(Props);

    Props->GetValue("Visible", &m_Visible);
    Props->GetValue("Enabled", &m_Enabled);
}