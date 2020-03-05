//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIProgressBar.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIProgressBar class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIProgressBar.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIProgressBar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIProgressBar object in
//                  system memory.

GUIProgressBar::GUIProgressBar(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "PROGRESSBAR";
    m_DrawBitmap = 0;
    m_IndicatorImage = 0;
    m_ControlManager = ControlManager;
    m_Spacing = 0;
    m_Value = 0;
    m_Minimum = 0;
    m_Maximum = 100;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIProgressBar::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 40;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 100;
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

    // Make sure the control isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIProgressBar::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 40;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 100;
    m_DefHeight = 20;

    // Setup the panel
    GUIPanel::LoadProperties(Props);

    // Make sure the control isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    Props->GetValue("Minimum", &m_Minimum);
    Props->GetValue("Maximum", &m_Maximum);
    Props->GetValue("Value", &m_Value);

    // Clamp the value
    m_Value = MAX(m_Value, m_Minimum);
    m_Value = MIN(m_Value, m_Maximum);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the panel has been destroyed.

void GUIProgressBar::Destroy(void)
{
    // Destroy the drawing bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Destroy the indicator bitmap
    if (m_IndicatorImage) {
        m_IndicatorImage->Destroy();
        delete m_IndicatorImage;
        m_IndicatorImage = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIProgressBar::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the progressbar bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the progressbar bitmap to draw.

void GUIProgressBar::BuildBitmap(void)
{
    // Free any old bitmaps
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }
    if (m_IndicatorImage) {
        m_IndicatorImage->Destroy();
        delete m_IndicatorImage;
        m_IndicatorImage = 0;
    }

    // Create a new bitmap.
    m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);
    
    // Build the background
    m_Skin->BuildStandardRect(m_DrawBitmap, "ProgressBar_Base", 0, 0, m_Width, m_Height);    

    // Build the indicator
    string Filename;
    m_Skin->GetValue("ProgressBar_Indicator", "Filename", &Filename);
    GUIBitmap *Src = m_Skin->CreateBitmap(Filename);
    if (!Src)
        return;

    int Values[4];
    GUIRect Rect;
    m_Skin->GetValue("ProgressBar_Indicator", "Top", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    m_IndicatorImage = m_Skin->CreateBitmap(Values[2], m_Height-4);
    if (!m_IndicatorImage)
        return;

    // Tile the centre piece
    m_Skin->GetValue("ProgressBar_Indicator", "Centre", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);    

    for(int y=0; y<m_IndicatorImage->GetHeight(); y+=Values[3]) {
        for(int x=0; x<m_IndicatorImage->GetWidth(); x+=Values[2]) {
            Src->Draw(m_IndicatorImage, x, y, &Rect);
        }
    }

    // Draw the top & bottom pieces
    m_Skin->GetValue("ProgressBar_Indicator", "Top", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);    
    Src->Draw(m_IndicatorImage, 0,0, &Rect);

    m_Skin->GetValue("ProgressBar_Indicator", "Bottom", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);    
    Src->Draw(m_IndicatorImage, 0,m_IndicatorImage->GetHeight()-Values[3], &Rect);

    m_Skin->GetValue("ProgressBar_Indicator", "Spacing", &m_Spacing);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIProgressBar::Draw(GUIScreen *Screen)
{
    // Draw the base
    Screen->DrawBitmap(m_DrawBitmap, m_X, m_Y, 0);

    // Draw the indicators
    if (!m_IndicatorImage)
        return;

    float Count = 0;
    if (m_Maximum-m_Minimum > 0) {
        float V = (float)(m_Value-m_Minimum) / (float)(m_Maximum-m_Minimum);
        Count = (float)m_Width*V;
    }
    if (m_IndicatorImage->GetWidth()+m_Spacing > 0)
        Count = Count / (float)(m_IndicatorImage->GetWidth()+m_Spacing);

    // Setup the clipping
    GUIRect Rect = *GetRect();
    Rect.left++;
    Rect.right-=2;
    Screen->GetBitmap()->SetClipRect(&Rect);

    int x = m_X+2;
    int Limit = (int)ceil(Count);
    for(int i=0; i<Limit; i++) {
        m_IndicatorImage->Draw(Screen->GetBitmap(), x, m_Y+2, 0);
        x += m_IndicatorImage->GetWidth() + m_Spacing;
    }

    Screen->GetBitmap()->SetClipRect(0);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIProgressBar::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    CaptureMouse();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIProgressBar::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    if (PointInside(X, Y))
        AddEvent(GUIEvent::Notification, Clicked, Buttons);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUIProgressBar::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIProgressBar::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIProgressBar::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIProgressBar::Resize(int Width, int Height)
{
    // Make sure the control isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    GUIPanel::SetSize(Width, Height);

    // Rebuild the bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIProgressBar::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIProgressBar::StoreProperties(void)
{
    m_Properties.AddVariable("Minimum", m_Minimum);
    m_Properties.AddVariable("Maximum", m_Maximum);
    m_Properties.AddVariable("Value", m_Value);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the value.

void GUIProgressBar::SetValue(int Value)
{
    int OldValue = m_Value;
    m_Value = Value;

    // Clamp the value
    m_Value = MIN(m_Value, m_Maximum);
    m_Value = MAX(m_Value, m_Minimum);
    
    // Changed?
    if (m_Value != OldValue)
        AddEvent(GUIEvent::Notification, Changed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the value.

int GUIProgressBar::GetValue(void)
{
    return m_Value;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMinimum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the minimum.

void GUIProgressBar::SetMinimum(int Minimum)
{
    m_Minimum = Minimum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMinimum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the minimum.

int GUIProgressBar::GetMinimum(void)
{
    return m_Minimum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaximum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the maximum.

void GUIProgressBar::SetMaximum(int Maximum)
{
    m_Maximum = Maximum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaximum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the maximum.

int GUIProgressBar::GetMaximum(void)
{
    return m_Maximum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIProgressBar::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Properties.GetValue("Minimum", &m_Minimum);
    m_Properties.GetValue("Maximum", &m_Maximum);
    m_Properties.GetValue("Value", &m_Value);

    // Clamp the value
    m_Value = MAX(m_Value, m_Minimum);
    m_Value = MIN(m_Value, m_Maximum);
}