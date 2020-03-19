//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIControl.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIControl class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIControl object in
//                  system memory.

GUIControl::GUIControl()
{
    m_Skin = 0;
    m_SkinPreset = 1;
    m_Properties.Clear();
    m_ControlChildren.clear();
    m_ControlParent = 0;
    m_IsContainer = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIControl::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    m_Properties.Clear();
    m_Properties.AddVariable("Name", Name);
    m_Properties.AddVariable("Anchor", "Left, Top");
    m_Properties.AddVariable("ToolTip", "");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIControl::Create(GUIProperties *Props)
{
    assert(Props);

    // Add the default variables
    m_Properties.AddVariable("Name", "");
    m_Properties.AddVariable("Anchor", "Left, Top");
    m_Properties.AddVariable("ToolTip", "");

    m_Properties.Update(Props);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.
// Arguments:       None.

void GUIControl::Destroy(void)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control is activated and ready for use.

void GUIControl::Activate(void)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIControl::ChangeSkin(GUISkin *Skin)
{
    m_Skin = Skin;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Add a new event to the queue.

void GUIControl::AddEvent(int Type, int Msg, int Data)
{
    m_ControlManager->AddEvent(new GUIEvent(this, Type, Msg, Data));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control's name.

std::string GUIControl::GetName(void)
{
    std::string Name;
    m_Properties.GetValue("Name", &Name);

    return Name;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetToolTip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control's tooltip string.

std::string GUIControl::GetToolTip(void)
{
    std::string tip;
    m_Properties.GetValue("ToolTip", &tip);

    return tip;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a string representing the control's ID

std::string GUIControl::GetID(void)
{
    return m_ControlID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIControl::GetPanel(void)
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddChild
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a child to this control

void GUIControl::AddChild(GUIControl *Control)
{
    assert(Control);

    // Remove the control from any previous parent
    if (Control->GetParent())
        Control->GetParent()->GUIControl::RemoveChild(Control->GetName());

    Control->m_ControlParent = this;
    m_ControlChildren.push_back(Control);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetChildren
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the children lst

std::vector<GUIControl *> *GUIControl::GetChildren(void)
{
    return &m_ControlChildren;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the control properties.

bool GUIControl::Save(Writer *W)
{
    string OutString = "";
    string Name;

    // Get the control to store its properties
    StoreProperties();

    // Section Header
    m_Properties.GetValue("Name", &Name);
    
    OutString.append("[");
    OutString.append(Name);
    OutString.append("]\n");

    // General control values
    OutString.append("ControlType = ");
    OutString += m_ControlID;
    OutString.append("\n");

    // Parent
    OutString += "Parent = ";
    if (m_ControlParent)
        OutString += m_ControlParent->GetName();
    else
        OutString += "None";
    OutString += "\n";


    // Get the main panel and write its location
    GUIPanel *Pan = GetPanel();
    if (Pan)
        OutString.append(Pan->ToString());

    // Write out the properties
    OutString.append(m_Properties.ToString());

    // Write to the writer class
    *W << OutString;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIControl::Move(int X, int Y)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIControl::Resize(int Width, int Height)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIControl::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    // Zero the values for controls that don't override this
    if (X) *X = 0;
    if (Y) *Y = 0;
    if (Width) *Width = 0;
    if (Height) *Height = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAnchor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the anchor flags.

int GUIControl::GetAnchor(void)
{
    int Anchor = 0;
    string Value[4];

    int Count = m_Properties.GetValue("Anchor", Value, 4);

    for(int i=0; i<Count; i++) {
        if (stricmp(Value[i].c_str(), "left") == 0)
            Anchor |= Anchor_Left;
        if (stricmp(Value[i].c_str(), "top") == 0)
            Anchor |= Anchor_Top;
        if (stricmp(Value[i].c_str(), "right") == 0)
            Anchor |= Anchor_Right;
        if (stricmp(Value[i].c_str(), "bottom") == 0)
            Anchor |= Anchor_Bottom;
    }

    // The anchor cannot have both sides missing, so we default to Left, Top is that is the case
    if (!(Anchor & Anchor_Left) && !(Anchor & Anchor_Right))
        Anchor |= Anchor_Left;
    if (!(Anchor & Anchor_Top) && !(Anchor & Anchor_Bottom))
        Anchor |= Anchor_Top;

    return Anchor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIControl::StoreProperties(void)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the visibility of the control.

void GUIControl::SetVisible(bool Visible)
{
    /*
     * Default method is the grab the main panel
     * And directly set its state.
     * Controls that use multiple panels on the same layer
     * will need to override this function
     */
    GUIPanel *Panel = GetPanel();
    if (Panel)
        Panel->_SetVisible(Visible);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the visibility of the control.

bool GUIControl::GetVisible(void)
{
    // See SetVisible() comment

    GUIPanel *Panel = GetPanel();
    if (Panel)
        return Panel->_GetVisible();

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the enabled state of the control.

void GUIControl::SetEnabled(bool Enabled)
{
    // See SetVisible() comment

    GUIPanel *Panel = GetPanel();
    if (Panel)
        Panel->_SetEnabled(Enabled);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the enabled state of the control.

bool GUIControl::GetEnabled(void)
{
    // See SetVisible() comment

    GUIPanel *Panel = GetPanel();
    if (Panel)
        return Panel->_GetEnabled();

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the parent of this control.

GUIControl *GUIControl::GetParent(void)
{
    return m_ControlParent;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control properties.

GUIProperties *GUIControl::GetProperties(void)
{
    return &m_Properties;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIControl::ApplyProperties(GUIProperties *Props)
{
    assert(Props);

    m_Properties.Update(Props);

    int X, Y;
    int Width, Height;
    bool Enabled;
    bool Visible;
    Props->GetValue("X", &X);
    Props->GetValue("Y", &Y);
    Props->GetValue("Width", &Width);
    Props->GetValue("Height", &Height);
    Props->GetValue("Enabled", &Enabled);
    Props->GetValue("Visible", &Visible);

    // Adjust position from parent
    GUIPanel *P = GetPanel();
    if (P) {
        if (P->GetParentPanel()) {
            int px,py,pw,ph;
            P->GetParentPanel()->GetRect(&px, &py, &pw, &ph);
            X += px;
            Y += py;
        }
    }

    SetEnabled(Enabled);
    
    Move(X, Y);
    Resize(Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the IsContainer value.

bool GUIControl::IsContainer(void)
{
    return m_IsContainer;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveChild
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a child based on name.

void GUIControl::RemoveChild(const string Name)
{
    // Note: We do NOT free the children because they are still linked in through their
    // panels.
    // This merely removes the control from the list.
    // This will cause a small memory leak, but this is only designed for the GUI Editor
    // and is a bit of a hack

    vector<GUIControl *>::iterator it;

    for(it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
        GUIControl *C = *it;
        if (C) {
            if (C->GetName().compare(Name) == 0) {
                m_ControlChildren.erase(it);
                break;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveChildren
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all the children.

void GUIControl::RemoveChildren(void)
{
    // Note: We do NOT free the children because they are still linked in through their
    // panels.
    // This merely removes the control from the list.
    // This will cause a small memory leak, but this is only designed for the GUI Editor
    // and is a bit of a hack

    vector<GUIControl *>::iterator it;

    for(it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
        GUIControl *C = *it;

        if (C) {
            m_ControlManager->RemoveControl(C->GetName(), false);
        }
    }

    m_ControlChildren.clear();
}
