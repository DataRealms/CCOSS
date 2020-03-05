//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUICheckbox.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUICheckbox class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUICheckbox.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUICheckbox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUICheckbox object in
//                  system memory.

GUICheckbox::GUICheckbox(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "CHECKBOX";
    m_Image = 0;
    m_ControlManager = ControlManager;
    m_Check = Unchecked;
    m_Mouseover = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUICheckbox::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 40;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 16;

    // Setup the panel
    m_X = X;
    m_Y = Y;
    m_Width = m_DefWidth;
    m_Height = m_DefHeight;

    if (Width != -1)
        m_Width = Width;
    if (Height != -1)
        m_Height = Height;

    // Make sure the button isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUICheckbox::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 40;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 16;

    // Setup the panel
    GUIPanel::LoadProperties(Props);

    // Make sure the button isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);


    // Grab the check value
    m_Check = Unchecked;
    string value;
    Props->GetValue("Checked", &value);
    if (stricmp(value.c_str(), "Checked") == 0)
        m_Check = Checked;
    else if (stricmp(value.c_str(), "Greycheck") == 0)
        m_Check = Greycheck;

    Props->GetValue("Text", &m_Text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUICheckbox::Destroy(void)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUICheckbox::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the checkbox bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the checkbox bitmap to draw.

void GUICheckbox::BuildBitmap(void)
{
    string Filename;
    unsigned long ColorIndex = 0;
    int Values[4];

    // Load the image
    m_Skin->GetValue("Checkbox", "Filename", &Filename);
    m_Image = m_Skin->CreateBitmap(Filename);
    if (!m_Image)
        return;

    // Load the font
    m_Skin->GetValue("Checkbox", "Font", &Filename);
    m_Font = m_Skin->GetFont(Filename);
    m_Skin->GetValue("Checkbox", "FontColor", &m_FontColor);
    m_Skin->GetValue("Checkbox", "FontShadow", &m_FontShadow);
    m_Skin->GetValue("Checkbox", "FontKerning", &m_FontKerning);
    m_FontColor = m_Skin->ConvertColor(m_FontColor, m_Image->GetColorDepth());
    m_Font->CacheColor(m_FontColor);

    // Set the colorkey
    m_Skin->GetValue("Checkbox", "ColorKeyIndex", &ColorIndex);
    ColorIndex = m_Skin->ConvertColor(ColorIndex, m_Image->GetColorDepth());
    m_Image->SetColorKey(ColorIndex);

    // Load the source image rectangles

    // Base checkbox
    m_Skin->GetValue("Checkbox", "Base", Values, 4);
    SetRect(&m_ImageRects[0], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Mouse over checkbox
    m_Skin->GetValue("Checkbox", "MouseOver", Values, 4);
    SetRect(&m_ImageRects[1], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Normal check
    m_Skin->GetValue("Checkbox", "Check", Values, 4);
    SetRect(&m_ImageRects[2], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Greyed check
    m_Skin->GetValue("Checkbox", "GreyCheck", Values, 4);
    SetRect(&m_ImageRects[3], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUICheckbox::Draw(GUIScreen *Screen)
{
    if (!m_Image)
        return;

    // Setup the clipping
    Screen->GetBitmap()->SetClipRect(GetRect());

    // Calculate the y position of the base
    // Make it centred vertically
    int YPos = m_Height/2 - (m_ImageRects[0].bottom - m_ImageRects[0].top)/2 + m_Y;

    // Draw the base
    if (m_Mouseover)
        m_Image->Draw(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[1]);
    else
        m_Image->Draw(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[0]);

    // Draw the check
    switch(m_Check) {
        // Normal check
        case Checked:
            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[2]);
            break;

        // Greyed check
        case Greycheck:
            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
            break;
    }

    // Draw the text
    string Text;
    string space = " ";
    Text = space.append(m_Text);

    if (m_Font) {
        m_Font->SetColor(m_FontColor);
        m_Font->SetKerning(m_FontKerning);
        m_Font->Draw(Screen->GetBitmap(),
                     m_X + (m_ImageRects[0].right - m_ImageRects[0].left) + 2,
//                     m_Y + m_Height/2-m_Font->CalculateHeight(Text)+2,
                     m_Y + (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1,
                     Text,
                     m_FontShadow);
    }
    


    Screen->GetBitmap()->SetClipRect(0);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUICheckbox::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    if (Buttons & MOUSE_LEFT) {
        // Push the checkbox down
        CaptureMouse();
        SetFocus();

        AddEvent(GUIEvent::Notification, Pushed, 0);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUICheckbox::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    // If the mouse is over the button, add the command to the event queue
    if (PointInside(X, Y) && Buttons & MOUSE_LEFT) {
        if (m_Check == Unchecked)
            m_Check = Checked;
        else
            m_Check = Unchecked;
        
        AddEvent(GUIEvent::Notification, Changed, 0);
    }

    AddEvent(GUIEvent::Notification, UnPushed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.

void GUICheckbox::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUICheckbox::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUICheckbox::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUICheckbox::Move(int X, int Y)
{
    SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUICheckbox::Resize(int Width, int Height)
{
    // Make sure the control isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    SetSize(Width, Height);

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUICheckbox::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUICheckbox::StoreProperties(void)
{
    if (m_Check == Unchecked)
        m_Properties.AddVariable("Checked", "Unchecked");
    else if (m_Check == Checked)
        m_Properties.AddVariable("Checked", "Checked");
    else if (m_Check == Greycheck)
        m_Properties.AddVariable("Checked", "Greycheck");

    m_Properties.AddVariable("Text", m_Text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text.

void GUICheckbox::SetText(const string Text)
{
    m_Text = Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text.

string GUICheckbox::GetText(void)
{
    return m_Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the check state.

void GUICheckbox::SetCheck(int Check)
{
    m_Check = Check;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the check state.

int GUICheckbox::GetCheck(void)
{
    return m_Check;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUICheckbox::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Check = Unchecked;
    string value;
    m_Properties.GetValue("Checked", &value);
    if (stricmp(value.c_str(), "Checked") == 0)
        m_Check = Checked;
    else if (stricmp(value.c_str(), "Greycheck") == 0)
        m_Check = Greycheck;

    m_Properties.GetValue("Text", &m_Text);
}