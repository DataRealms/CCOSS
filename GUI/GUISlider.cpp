//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUISlider.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUISlider class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUISlider.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUISlider
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUISliderr object in
//                  system memory.

GUISlider::GUISlider(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "SLIDER";
    m_DrawBitmap = 0;
    m_KnobImage = 0;
    m_ControlManager = ControlManager;
    m_Orientation = Horizontal;
    m_TickDirection = BottomRight;
    m_KnobPosition = 0;
    m_KnobSize = 0;
    m_KnobGrabbed = false;
    m_Minimum = 0;
    m_Value = 0;
    m_Maximum = 100;
	m_ValueResolution = 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUISlider::Create(const std::string Name, int X, int Y, int Width, int Height)
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

    // Re-Calculate the knob info
    CalculateKnob();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUISlider::Create(GUIProperties *Props)
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

    // Make sure the button isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);


    // Get the values
    string ori;
    Props->GetValue("Orientation", &ori);
    if (stricmp(ori.c_str(), "horizontal") == 0)
        m_Orientation = Horizontal;
    else if (stricmp(ori.c_str(), "vertical") == 0)
        m_Orientation = Vertical;

    string tick;
    Props->GetValue("TickDirection", &tick);
    if (stricmp(tick.c_str(), "TopLeft") == 0)
        m_TickDirection = TopLeft;
    else if (stricmp(tick.c_str(), "BottomRight") == 0)
        m_TickDirection = BottomRight;
        
    Props->GetValue("Minimum", &m_Minimum);
    Props->GetValue("Maximum", &m_Maximum);
    Props->GetValue("Value", &m_Value);
	if (!Props->GetValue("ValueResolution", &m_ValueResolution)) {
		m_ValueResolution = std::max((m_Maximum - m_Minimum) / 100, 1);
	}

	m_Value = std::clamp(m_Value, m_Minimum, m_Maximum);

    // Re-Calculate the knob info
    CalculateKnob();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUISlider::Destroy(void)
{
    // Destroy the draw bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Destroy the knob image bitmap
    if (m_KnobImage) {
        m_KnobImage->Destroy();
        delete m_KnobImage;
        m_KnobImage = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUISlider::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the progressbar bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the progressbar bitmap to draw.

void GUISlider::BuildBitmap(void)
{
    // Free any old bitmaps
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }
    if (m_KnobImage) {
        m_KnobImage->Destroy();
        delete m_KnobImage;
        m_KnobImage = 0;
    }

    string Section;
    if (m_Orientation == Horizontal)
        Section = "Slider_Horz";
    else
        Section = "Slider_Vert";

    // Load the source image
    string Filename;
    m_Skin->GetValue(Section, "Filename", &Filename);
    GUIBitmap *SrcImage = m_Skin->CreateBitmap(Filename);
    if (!SrcImage)
        return;

    // Build the line & ticks
    BuildLine(Section, SrcImage);

    // Load the indicator image
    string Side;
    if (m_TickDirection == TopLeft)
        Side = "TopLeftSlider";
    else
        Side = "BottomRightSlider";

    int Values[4];
    GUIRect Rect;
    m_Skin->GetValue(Section, Side, Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Create the indicator bitmap
    m_KnobImage = m_Skin->CreateBitmap(Values[2], Values[3]);
    SrcImage->Draw(m_KnobImage, 0,0, &Rect);

    unsigned long ColorKey;
    m_Skin->GetValue(Section, "ColorKeyIndex", &ColorKey);
    ColorKey = m_Skin->ConvertColor(ColorKey, m_KnobImage->GetColorDepth());
    m_KnobImage->SetColorKey(ColorKey);
    m_DrawBitmap->SetColorKey(ColorKey);

    // Re-Calculate the knob info
    CalculateKnob();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Builds the background line for the slider

void GUISlider::BuildLine(const string Section, GUIBitmap *SrcImage)
{
    int Values[4];
    GUIRect Rect;

    // Get the size of the middle bit for determining part of the bitmap size
    m_Skin->GetValue(Section, "Middle", Values, 4);
    if (m_Orientation == Horizontal)
        m_DrawBitmap = m_Skin->CreateBitmap(m_Width, Values[3]);
    else
        m_DrawBitmap = m_Skin->CreateBitmap(Values[2], m_Height);    

    // Tile the line
    m_Skin->GetValue(Section, "Middle", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);
    if (m_Orientation == Horizontal) {
        for(int i=0; i<m_Width; i+=Values[2])
            SrcImage->Draw(m_DrawBitmap, i, 0, &Rect);
    } else {
        for(int i=0; i<m_Height; i+=Values[3])
            SrcImage->Draw(m_DrawBitmap, 0, i, &Rect);
    }

    // Draw the start
    m_Skin->GetValue(Section, "Start", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);
    SrcImage->Draw(m_DrawBitmap, 0, 0, &Rect);

    m_EndThickness = m_Orientation == Horizontal ? Values[2] : Values[3];

    // Draw the end
    int X = 0, Y = 0;
    m_Skin->GetValue(Section, "End", Values, 4);
    SetRect(&Rect, Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);    
    if (m_Orientation == Horizontal)
        X = m_Width-Values[2];
    else
        Y = m_Height-Values[3];
    SrcImage->Draw(m_DrawBitmap, X, Y, &Rect);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUISlider::Draw(GUIScreen *Screen)
{
    int X = 0;
    int Y = 0;

    if (m_Orientation == Horizontal)
        Y = m_Height/2-m_DrawBitmap->GetHeight()/2;
    else
        X = m_Width/2-m_DrawBitmap->GetWidth()/2;

    // Draw the base
    Screen->DrawBitmap(m_DrawBitmap, m_X+X, m_Y+Y, 0);

    // Draw the indicator
    if (!m_KnobImage)
        return;

    int Half = 0;
    if (m_Orientation == Horizontal)    
        Half = m_DrawBitmap->GetHeight()/2;
    else
        Half = m_DrawBitmap->GetWidth()/2;

    if (m_Orientation == Horizontal)
        m_KnobImage->DrawTrans(Screen->GetBitmap(), m_X+m_KnobPosition, m_Y+Y+Half-m_KnobImage->GetHeight()/2, 0);
    else
        m_KnobImage->DrawTrans(Screen->GetBitmap(), m_X+X+Half-m_KnobImage->GetWidth()/2, m_Y+m_KnobPosition, 0);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUISlider::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    CaptureMouse();
    SetFocus();

    m_OldValue = m_Value;
    int Size = 0;

    // Only do stuff here on a left mouse click
    if (!(Buttons & MOUSE_LEFT))
        return;
/*
    // Grabbed the slider?
    if (m_Orientation == Horizontal) {
        if (X > m_X+m_KnobPosition-3 && X < m_X+m_KnobPosition+m_KnobSize+3) {
            m_KnobGrabbed = true;
            m_KnobGrabPos = X - (m_X+m_KnobPosition);
        }
    } else {
        if (Y > m_Y+m_KnobPosition-3 && Y < m_Y+m_KnobPosition+m_KnobSize+3) {
            m_KnobGrabbed = true;
            m_KnobGrabPos = Y - (m_Y+m_KnobPosition);
        }
    }
*/
    if (m_Orientation == Horizontal) {
        if (X > m_X + m_EndThickness && X < m_X + m_Width - m_EndThickness) {
            m_KnobGrabbed = true;
            m_KnobPosition = X - m_X - (m_KnobSize / 2);
            m_KnobGrabPos = X - (m_X+m_KnobPosition);
        }
        Size = m_Width;
    } else {
        if (Y > m_Y + m_EndThickness && Y < m_Y + m_Height - m_EndThickness) {
            m_KnobGrabbed = true;
            m_KnobPosition = Y - m_Y - (m_KnobSize / 2);
            m_KnobGrabPos = Y - (m_Y+m_KnobPosition);
        }
        Size = m_Height;
    }

	m_KnobPosition = std::clamp(m_KnobPosition, m_EndThickness, Size - m_KnobSize - m_EndThickness);

    // Calculate the new value
    int Area = Size-m_KnobSize;
    if (Area > 0) {
        float p = (float)m_KnobPosition / (float)(Area);
        int MaxRange = (m_Maximum - m_Minimum);
        m_Value = (float)MaxRange * p + m_Minimum;
    }

	m_Value = std::clamp(m_Value, m_Minimum, m_Maximum);

    // If the value has changed, add the "Changed" notification
    if (m_Value != m_OldValue)
        AddEvent(GUIEvent::Notification, Changed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUISlider::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    m_KnobGrabbed = false;
    
    // If the value has changed, add the "Changed" notification
    if (m_Value != m_OldValue)
        AddEvent(GUIEvent::Notification, Changed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUISlider::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    int MousePos = X;
    int KnobTop = 0;
    int Size = 1;

    // Horizontal
    if (m_Orientation == Horizontal) {
        MousePos = X;
        KnobTop = m_X+m_KnobPosition;
        Size = m_Width;
    }

    // Vertical
    if (m_Orientation == Vertical) {
        MousePos = Y;
        KnobTop = m_Y+m_KnobPosition;
        Size = m_Height;
    }

    // Move the knob if it is grabbed
    if (m_KnobGrabbed) {
        int Delta = m_KnobGrabPos - (MousePos-KnobTop);
        m_KnobPosition -= Delta;

        // Clamp the knob position
        m_KnobPosition = MAX(m_KnobPosition, 0);
        m_KnobPosition = MIN(m_KnobPosition, Size-m_KnobSize);

        // Calculate the new value
        int Area = Size-m_KnobSize;
        if (Area > 0) {
            float p = (float)m_KnobPosition / (float)(Area);
            int MaxRange = (m_Maximum - m_Minimum);
            m_Value = (float)MaxRange * p + m_Minimum;
        }

        // Clamp the value
        m_Value = MAX(m_Value, m_Minimum);
        m_Value = MIN(m_Value, m_Maximum);

        // Clamp the knob position again for the graphics
        m_KnobPosition = MAX(m_KnobPosition, m_EndThickness);
        m_KnobPosition = MIN(m_KnobPosition, Size-m_KnobSize-m_EndThickness);

        // If the value has changed, add the "Changed" notification
        if (m_Value != m_OldValue)
            AddEvent(GUIEvent::Notification, Changed, 0);

        m_OldValue = m_Value;
    }
}


void GUISlider::OnMouseWheelChange(int x, int y, int modifier, int mouseWheelChange) {
	m_OldValue = m_Value;

	if (mouseWheelChange < 0) {
		m_Value = std::max(m_Value - m_ValueResolution, m_Minimum);
	} else {
		m_Value = std::min(m_Value + m_ValueResolution, m_Maximum);
	}
	
	if (m_Value != m_OldValue) {
		CalculateKnob();
		AddEvent(GUIEvent::Notification, Changed, 0);
	}
}

/*

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a property value.

void GUISlider::SetValue(const std::string Name, const std::string Value)
{
    GUIControl::SetValue(Name, Value);

    m_Properties.GetValue("Minimum", &m_Minimum);
    m_Properties.GetValue("Maximum", &m_Maximum);
    m_Properties.GetValue("Value", &m_Value);

    // Clamp the value
    m_Value = MAX(m_Value, m_Minimum);
    m_Value = MIN(m_Value, m_Maximum);

    // Re-Calculate the knob info
    CalculateKnob();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a property value.

void GUISlider::GetValue(const std::string Name, std::string *Value)
{
    // Update our variables into the properties
    m_Properties.SetValue("Minimum", m_Minimum);
    m_Properties.SetValue("Maximum", m_Maximum);
    m_Properties.SetValue("Value", m_Value);

    GUIControl::GetValue(Name, Value);
}*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUISlider::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateKnob
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the knob position and size.

void GUISlider::CalculateKnob(void) {
	if (!m_KnobImage) {
		return;
	}

	if (m_Maximum > m_Minimum) {
		const bool horizontalOrientation = (m_Orientation == Horizontal);
		m_KnobSize = (horizontalOrientation) ? m_KnobImage->GetWidth() : m_KnobImage->GetHeight();
		const int size = (horizontalOrientation) ? m_Width : m_Height;
		const float valueRatio = static_cast<float>(m_Value - m_Minimum) / static_cast<float>(m_Maximum - m_Minimum);
		m_KnobPosition = m_EndThickness + static_cast<int>(static_cast<float>(size - m_KnobSize - (m_EndThickness * 2)) * valueRatio);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUISlider::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUISlider::Resize(int Width, int Height)
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

void GUISlider::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOrientation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the orientation of the slider.

void GUISlider::SetOrientation(int Orientation)
{
    m_Orientation = Orientation;

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOrientation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the orientation of the slider.

int GUISlider::GetOrientation(void)
{
    return m_Orientation;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTickDirection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the direction of the ticks.

void GUISlider::SetTickDirection(int TickDir)
{
    m_TickDirection = TickDir;

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTickDirection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the direction of the ticks.

int GUISlider::GetTickDirection(void)
{
    return m_TickDirection;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMinimum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the minimun value.

void GUISlider::SetMinimum(int Minimum)
{
	if (Minimum != m_Minimum) {
		m_Minimum = Minimum;
		m_Value = std::max(m_Value, m_Minimum);

		// Re-Calculate the knob info
		CalculateKnob();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMinimum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the minimun value.

int GUISlider::GetMinimum(void)
{
    return m_Minimum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaximum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the maximun value.

void GUISlider::SetMaximum(int Maximum)
{
	if (Maximum != m_Maximum) {
		m_Maximum = Maximum;
		m_Value = std::min(m_Value, m_Maximum);

		// Re-Calculate the knob info
		CalculateKnob();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaximum
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the maximun value.

int GUISlider::GetMaximum(void)
{
    return m_Maximum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the value.

void GUISlider::SetValue(int Value)
{
    int OldValue = m_Value;

	m_Value = std::clamp(Value, m_Minimum, m_Maximum);
    
	if (m_Value != OldValue) {
		CalculateKnob();
		AddEvent(GUIEvent::Notification, Changed, 0);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the value.

int GUISlider::GetValue(void)
{
    return m_Value;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUISlider::StoreProperties(void)
{
    m_Properties.AddVariable("Value", m_Value);
    m_Properties.AddVariable("Minimum", m_Minimum);
    m_Properties.AddVariable("Maximum", m_Maximum);
    m_Properties.AddVariable("Orientation", m_Orientation == Horizontal ? "Horizontal" : "Vertical");
    m_Properties.AddVariable("TickDirection", m_TickDirection == TopLeft ? "TopLeft" : "BottomRight");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUISlider::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    // Get the values
    string ori;
    m_Properties.GetValue("Orientation", &ori);
    if (stricmp(ori.c_str(), "horizontal") == 0)
        m_Orientation = Horizontal;
    else if (stricmp(ori.c_str(), "vertical") == 0)
        m_Orientation = Vertical;

    string tick;
    m_Properties.GetValue("TickDirection", &tick);
    if (stricmp(tick.c_str(), "TopLeft") == 0)
        m_TickDirection = TopLeft;
    else if (stricmp(tick.c_str(), "BottomRight") == 0)
        m_TickDirection = BottomRight;
        
    m_Properties.GetValue("Minimum", &m_Minimum);
    m_Properties.GetValue("Maximum", &m_Maximum);
    m_Properties.GetValue("Value", &m_Value);

    // Clamp the value
    m_Value = MAX(m_Value, m_Minimum);
    m_Value = MIN(m_Value, m_Maximum);

    BuildBitmap();
}

void GUISlider::SetValueResolution(int valueRes) {
	if (valueRes >= 1 && valueRes <= m_Maximum - m_Minimum) {
		m_ValueResolution = valueRes;
	}
}
