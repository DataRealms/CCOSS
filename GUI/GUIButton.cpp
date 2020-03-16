//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIButton.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIButton class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIButton.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIButton object in
//                  system memory.

GUIButton::GUIButton(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "BUTTON";
    m_DrawBitmap = 0;
    m_ControlManager = ControlManager;
    m_Pushed = false;
    m_Over = false;
    m_Text = "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIButton::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 10;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 40;

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

void GUIButton::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 10;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 60;
    m_DefHeight = 40;

    // Setup the panel
    GUIPanel::LoadProperties(Props);

    // Make sure the button isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    // Load the values
    Props->GetValue("Text", &m_Text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUIButton::Destroy(void)
{
    // Free the drawing bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIButton::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the button bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the button bitmap to draw.

void GUIButton::BuildBitmap(void)
{
    // Free any old bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Create a new bitmap. Same width, but triple the height to allow for Up, Down
    // and Over states
    m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height*3);

    // Pre-cache the font
    string Filename;
    m_Skin->GetValue("Button_Up", "Font", &Filename);
    m_Skin->GetValue("Button_Up", "FontColor", &m_FontColor);
    m_Skin->GetValue("Button_Up", "FontShadow", &m_FontShadow);
    m_Skin->GetValue("Button_Up", "FontKerning", &m_FontKerning);
    
    m_FontColor = m_Skin->ConvertColor(m_FontColor, m_DrawBitmap->GetColorDepth());
    
    m_Font = m_Skin->GetFont(Filename);
    if (m_Font)
        m_Font->CacheColor(m_FontColor);

    // Create the button image
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Up", 0, 0, m_Width, m_Height);
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Over", 0, m_Height, m_Width, m_Height);
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Down", 0, m_Height*2, m_Width, m_Height);

    // Draw the text
    //int y = m_Height/2-m_Font->CalculateHeight(m_Text)+2;
    int y = (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1;

    m_Font->SetColor(m_FontColor);
    m_Font->SetKerning(m_FontKerning);

    m_Font->DrawAligned(m_DrawBitmap, m_Width/2, y, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);
    m_Font->DrawAligned(m_DrawBitmap, m_Width/2, m_Height+y, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);
    m_Font->DrawAligned(m_DrawBitmap, m_Width/2+1, m_Height*2+y+1, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIButton::Draw(GUIScreen *Screen)
{
    GUIRect Rect;
    int y = 0;
    if (m_Pushed)
        y = m_Height*2;
    else if (m_Over || m_GotFocus)
        y = m_Height;

    SetRect(&Rect, 0, y, m_Width, y+m_Height);

    m_DrawBitmap->DrawTrans(Screen->GetBitmap(), m_X, m_Y, &Rect);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIButton::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    if (Buttons & MOUSE_LEFT) {
        // Push the button down
        m_Pushed = true;
        CaptureMouse();

        AddEvent(GUIEvent::Notification, Pushed, 0);
    }
    SetFocus();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIButton::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    if (PointInside(X, Y))
        AddEvent(GUIEvent::Notification, Clicked, Buttons);

    if (!IsCaptured())
        return;

    m_Pushed = false;
    ReleaseMouse();

    // If the mouse is over the button, add the command to the event queue
    if (PointInside(X, Y))
        AddEvent(GUIEvent::Command, 0, 0);

    AddEvent(GUIEvent::Notification, UnPushed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.

void GUIButton::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
    m_Over = true;

    AddEvent(GUIEvent::Notification, Focused, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUIButton::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
    m_Over = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUIButton::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    if (!(Buttons & MOUSE_LEFT) || !IsCaptured())
        return;

    // If the mouse goes outside of the button, un-push the button
    if (!PointInside(X, Y)) {
        if (m_Pushed) {
            AddEvent(GUIEvent::Notification, UnPushed, 0);
            m_Pushed = false;
        }
    } else {
        if (!m_Pushed) {
            AddEvent(GUIEvent::Notification, Pushed, 0);
            m_Pushed = true;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnKeyDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when a key goes down.

void GUIButton::OnKeyDown(int KeyCode, int Modifier)
{

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIButton::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIButton::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIButton::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIButton::Resize(int Width, int Height)
{
    // Make sure the button isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    GUIPanel::SetSize(Width, Height);

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIButton::StoreProperties(void)
{
    m_Properties.AddVariable("Text", m_Text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text.

void GUIButton::SetText(const string Text)
{
    m_Text = Text;

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text.

string GUIButton::GetText(void)
{
    return m_Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIButton::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Properties.GetValue("Text", &m_Text);

    BuildBitmap();
}