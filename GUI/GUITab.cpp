//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUITab.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUITab class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUITab.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUITab
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUITab object in
//                  system memory.

GUITab::GUITab(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
GUIControl()
{
    m_ControlID = "TAB";
    m_Image = 0;    
    m_ControlManager = ControlManager;
    m_Selected = false;
    m_Font = 0;
    m_Mouseover = false;
    m_FontColor = 0;
    m_Text = "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUITab::Create(const std::string Name, int X, int Y, int Width, int Height)
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

void GUITab::Create(GUIProperties *Props)
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
    Props->GetValue("Checked", &m_Selected);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUITab::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the checkbox bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the radiobutton bitmap to draw.

void GUITab::BuildBitmap(void)
{
    string Filename;
    unsigned long ColorIndex = 0;
    int Values[4];

    // Load the image
    m_Skin->GetValue("Tab", "Filename", &Filename);
    m_Image = m_Skin->CreateBitmap(Filename);
    if (!m_Image)
        return;

    // Load the font
    m_Skin->GetValue("Tab", "Font", &Filename);
    m_Font = m_Skin->GetFont(Filename);

    m_Skin->GetValue("Tab", "FontColor", &m_FontColor);
    m_Skin->GetValue("Tab", "FontShadow", &m_FontShadow);
    m_Skin->GetValue("Tab", "FontKerning", &m_FontKerning);
    m_FontColor = m_Skin->ConvertColor(m_FontColor, m_Image->GetColorDepth());
    m_Font->CacheColor(m_FontColor);

    // Set the colorkey
    m_Skin->GetValue("Tab", "ColorKeyIndex", &ColorIndex);
    ColorIndex = m_Skin->ConvertColor(ColorIndex, m_Image->GetColorDepth());
    m_Image->SetColorKey(ColorIndex);

    // Load the source image rectangles

    // Base checkbox
    m_Skin->GetValue("Tab", "Base", Values, 4);
    SetRect(&m_ImageRects[0], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Mouse over checkbox
    m_Skin->GetValue("Tab", "MouseOver", Values, 4);
    SetRect(&m_ImageRects[1], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Selected
    m_Skin->GetValue("Tab", "Selected", Values, 4);
    SetRect(&m_ImageRects[2], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);

    // Greyed out tab (for disabled mode)
    m_Skin->GetValue("Tab", "Disabled", Values, 4);
    SetRect(&m_ImageRects[3], Values[0], Values[1], Values[0]+Values[2], Values[1]+Values[3]);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUITab::Draw(GUIScreen *Screen)
{
    if (!m_Image)
        return;

    // Setup the clipping
    Screen->GetBitmap()->SetClipRect(GetRect());

    // Calculate the y position of the base
    // Make it centred vertically
    int YPos = m_Height/2 - (m_ImageRects[0].bottom - m_ImageRects[0].top)/2 + m_Y;

    // Draw the base
    m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[0]);

    // Draw the slected one
    if (m_Selected)
    {
        if (m_Enabled)
            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[2]);
//        else
//            m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
    }

    // If highlighted, draw that
    if (m_Mouseover || m_GotFocus)
        m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[1]);
    // Should show as greyed out and disabled when it is, regardless of checked or not
    else if (!m_Enabled)
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
// TODO: DONT HARDCODE TEXT OFFSET
                     m_X + 4,
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

void GUITab::OnMouseDown(int X, int Y, int Buttons, int Modifier)
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

void GUITab::OnMouseUp(int X, int Y, int Buttons, int Modifier)
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

void GUITab::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = true;
    AddEvent(GUIEvent::Notification, Hovered, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUITab::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
    m_Mouseover = false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUITab::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    if (PointInside(X, Y))
        SendSignal(MouseMove, Buttons);
    }
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUITab::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUITab::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUITab::Resize(int Width, int Height)
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

void GUITab::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUITab::StoreProperties(void)
{
    m_Properties.AddVariable("Text", m_Text);
    m_Properties.AddVariable("Selected", m_Selected);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the check state.

void GUITab::SetCheck(bool Check)
{
    // Nothing to do if already in the same state
    if (m_Selected == Check)
        return;

    m_Selected = Check;

    AddEvent(GUIEvent::Notification, Changed, Check);
    
    // Don't worry if we are not checked
    if (!m_Selected)
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
                    GUITab *R = (GUITab *)C;
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

bool GUITab::GetCheck(void)
{
    return m_Selected;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text.

void GUITab::SetText(const string Text)
{
    m_Text = Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text.

string GUITab::GetText(void)
{
    return m_Text;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUITab::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    m_Properties.GetValue("Text", &m_Text);
    m_Properties.GetValue("Selected", &m_Selected);
}