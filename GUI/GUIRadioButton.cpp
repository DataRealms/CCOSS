//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIRadioButton.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIRadioButton class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIRadioButton.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIRadioButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIRadioButton object in
//                  system memory.

GUIRadioButton::GUIRadioButton(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
GUIControl()
{
    m_ControlID = "RADIOBUTTON";
    m_Image = 0;    
    m_ControlManager = ControlManager;
    m_Checked = false;
    m_Font = 0;
    m_Mouseover = false;
    m_FontColor = 0;
    m_Text = "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIRadioButton::Create(const std::string Name, int X, int Y, int Width, int Height)
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

void GUIRadioButton::Create(GUIProperties *Props)
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

    // Get the values
    Props->GetValue("Text", &m_Text);
    Props->GetValue("Checked", &m_Checked);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIRadioButton::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the checkbox bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the radiobutton bitmap to draw.

void GUIRadioButton::BuildBitmap(void)
{
    string Filename;
    unsigned long ColorIndex = 0;
    int Values[4];

    // Load the image
    m_Skin->GetValue("RadioButton", "Filename", &Filename);
    m_Image = m_Skin->CreateBitmap(Filename);
    if (!m_Image)
        return;

    // Load the font
    m_Skin->GetValue("RadioButton", "Font", &Filename);
    m_Font = m_Skin->GetFont(Filename);

    m_Skin->GetValue("RadioButton", "FontColor", &m_FontColor);
    m_Skin->GetValue("RadioButton", "FontShadow", &m_FontShadow);
    m_Skin->GetValue("RadioButton", "FontKerning", &m_FontKerning);
    m_FontColor = m_Skin->ConvertColor(m_FontColor, m_Image->GetColorDepth());
    m_Font->CacheColor(m_FontColor);

    // Set the colorkey
    m_Skin->GetValue("RadioButton", "ColorKeyIndex", &ColorIndex);
    ColorIndex = m_Skin->ConvertColor(ColorIndex, m_Image->GetColorDepth());
    m_Image->SetColorKey(ColorIndex);

    // Load the source image rectangles

    // Base checkbox
    m_Skin->GetValue("RadioButton", "Base", Values, 4);
    SetRect(&m_ImageRects[0], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Mouse over checkbox
    m_Skin->GetValue("RadioButton", "MouseOver", Values, 4);
    SetRect(&m_ImageRects[1], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Normal check
    m_Skin->GetValue("RadioButton", "Check", Values, 4);
    SetRect(&m_ImageRects[2], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Greyed check (for disabled mode)
    m_Skin->GetValue("RadioButton", "GreyCheck", Values, 4);
    SetRect(&m_ImageRects[3], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIRadioButton::Draw(GUIScreen *Screen)
{
    if (!m_Image)
        return;

    // Setup the clipping
    Screen->GetBitmap()->SetClipRect(GetRect());

    // Calculate the y position of the base
    // Make it centred vertically
    int YPos = m_Height/2 - (m_ImageRects[0].bottom - m_ImageRects[0].top)/2 + m_Y;

    // Draw the base
    if (m_Mouseover || m_GotFocus)
        m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[1]);
    else
        m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[0]);

    // Draw the check
    if (m_Checked)
    {
        if (m_Enabled)
            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[2]);
//        else
//            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
    }

    // Should show as greyed out and disabled when it is, regardless of checked or not
    if (!m_Enabled)
        m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);


    // Draw the text
    
    // Add a space to make a gap between checkbox & text
    string Text;
    string space = " ";
    Text = space.append(m_Text);

    if (m_Font) {
        m_Font->SetColor(m_FontColor);
        m_Font->SetKerning(m_FontKerning);
        m_Font->Draw(Screen->GetBitmap(), 
                     m_X + (m_ImageRects[0].right-m_ImageRects[0].left),
//                     m_Y + m_Height/2-m_Font->CalculateHeight(Text)+2,
                     m_Y + (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1,
                     Text, m_FontShadow);
    }
    


    Screen->GetBitmap()->SetClipRect(0);

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIRadioButton::OnMouseDown(int X, int Y, int Buttons, int Modifier)
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

void GUIRadioButton::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    // If the mouse is over the button, add the command to the event queue
    if (PointInside(X, Y) && Buttons & MOUSE_LEFT) {
        SetCheck(true);
    }

    AddEvent(GUIEvent::Notification, UnPushed, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.

void GUIRadioButton::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUIRadioButton::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIRadioButton::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIRadioButton::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIRadioButton::Resize(int Width, int Height)
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

void GUIRadioButton::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIRadioButton::StoreProperties(void)
{
    m_Properties.AddVariable("Text", m_Text);
    m_Properties.AddVariable("Checked", m_Checked);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the check state.

void GUIRadioButton::SetCheck(bool Check)
{
    // Nothing to do if already in the same state
    if (m_Checked == Check)
        return;

    m_Checked = Check;

    AddEvent(GUIEvent::Notification, Changed, Check);
    
    // Don't worry if we are not checked
    if (!m_Checked)
        return;

        
    // Go through all my RadioButton siblings and uncheck them        
    if (m_ControlParent) {
        vector<GUIControl *>::iterator it;
        vector<GUIControl *> *Children = m_ControlParent->GetChildren();
        
        for(it = Children->begin(); it != Children->end(); it++) {
            GUIControl *C = *it;
            if (C) {
                // Make sure this is not me
                if (C->GetPanel() && GetPanel()) {
                    if (C->GetPanel()->GetPanelID() == GetPanel()->GetPanelID())
                        continue;
                }
                
                // Make sure the control is a radio button
                if (C->GetID().compare(GetID()) == 0) {
                    GUIRadioButton *R = (GUIRadioButton *)C;
                    R->SetCheck(false);
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the check state.

bool GUIRadioButton::GetCheck(void)
{
    return m_Checked;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text.

void GUIRadioButton::SetText(const string Text)
{
    m_Text = Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text.

string GUIRadioButton::GetText(void)
{
    return m_Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUIRadioButton::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Properties.GetValue("Text", &m_Text);
    m_Properties.GetValue("Checked", &m_Checked);
}