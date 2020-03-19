//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUICollectionBox.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUICollectionBox class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUICollectionBox.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUICollectionBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUICollectionBox object in
//                  system memory.

GUICollectionBox::GUICollectionBox(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "COLLECTIONBOX";
    m_Background = 0;
    m_ControlManager = ControlManager;
    m_DrawBackground = true;
    m_DrawType = Color;
    m_DrawColor = 0;
    m_DrawBitmap = 0;

    m_IsContainer = true;        // We are a container
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUICollectionBox::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 10;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 100;
    m_DefHeight = 100;

    // Setup the panel
    m_X = X;
    m_Y = Y;
    m_Width = m_DefWidth;
    m_Height = m_DefHeight;

    if (Width != -1)
        m_Width = Width;
    if (Height != -1)
        m_Height = Height;

    // Make sure the box isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUICollectionBox::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 10;
    m_MinHeight = 10;

    // Default size of the control
    m_DefWidth = 100;
    m_DefHeight = 100;

    // Setup the panel
    GUIPanel::LoadProperties(Props);

    // Make sure the box isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    // Get the values
    Props->GetValue("DrawBackground", &m_DrawBackground);
    string v;
    Props->GetValue("DrawType", &v);
    if (stricmp(v.c_str(), "Color") == 0)
        m_DrawType = Color;
    else if (stricmp(v.c_str(), "Image") == 0)
        m_DrawType = Image;
    else if (stricmp(v.c_str(), "Panel") == 0)
        m_DrawType = Panel;

    Props->GetValue("DrawColor", &m_DrawColor);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and frees this' allocated data

void GUICollectionBox::Destroy()
{
    delete m_Background;
    delete m_DrawBitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUICollectionBox::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Build the panel bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the button bitmap to draw.

void GUICollectionBox::BuildBitmap(void)
{
    // Free any old bitmap
    delete m_DrawBitmap;

    // Create a new bitmap.
    m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

    // Create the button image
    m_Skin->BuildStandardRect(m_DrawBitmap, "CollectionBox_Panel", 0, 0, m_Width, m_Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUICollectionBox::Draw(GUIScreen *Screen)
{
    if (m_DrawBackground) {
        // Color
        if (m_DrawType == Color)
            Screen->GetBitmap()->DrawRectangle(m_X, m_Y, m_Width, m_Height, m_Skin->ConvertColor(m_DrawColor, Screen->GetBitmap()->GetColorDepth()), true);
        // Image
        else if (m_DrawType == Image) {
            if (m_DrawBitmap && m_DrawBackground) {
                // Setup the clipping
                Screen->GetBitmap()->SetClipRect(GetRect());

                // Draw the image
                m_DrawBitmap->DrawTrans(Screen->GetBitmap(), m_X, m_Y, 0);

                // Get rid of clipping
                Screen->GetBitmap()->SetClipRect(0);
            }
        }
        // Panel
        else if (m_DrawType == Panel && m_DrawBackground) {
            if (m_DrawBitmap) {
                GUIRect Rect;
                SetRect(&Rect, 0, 0, m_Width, m_Height);
                Screen->DrawBitmapTrans(m_DrawBitmap, m_X, m_Y, &Rect);
            }
        }
    }

    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUICollectionBox::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    CaptureMouse();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUICollectionBox::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    ReleaseMouse();

    AddEvent(GUIEvent::Notification, Clicked, Buttons);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUICollectionBox::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    AddEvent(GUIEvent::Notification, MouseMove, Buttons);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUICollectionBox::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUICollectionBox::Move(int X, int Y)
{
    int DX = X-m_X;
    int DY = Y-m_Y;

    m_X = X;
    m_Y = Y;

    // Go through all my children moving them
    vector<GUIControl *>::iterator it;
    for(it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
        GUIControl *C = *it;
        int CX, CY, CW, CH;
        C->GetControlRect(&CX, &CY, &CW, &CH);

        C->Move(CX+DX, CY+DY);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUICollectionBox::Resize(int Width, int Height)
{
    int OldWidth = m_Width;
    int OldHeight = m_Height;

    m_Width = Width;
    m_Height = Height;

    // Go through all my children moving them
    vector<GUIControl *>::iterator it;
    for(it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
        GUIControl *C = *it;
        int CX, CY, CW, CH;
        int Anchor = C->GetAnchor();

        C->GetControlRect(&CX, &CY, &CW, &CH);
        
        int DX = CX;
        int DY = CY;
        int W = CW;
        int H = CH;

        // Attached to Right and/or Bottom edges
        if ((Anchor & GUIControl::Anchor_Right) && !(Anchor & GUIControl::Anchor_Left))
            DX = m_Width - (OldWidth-(CX-m_X)) + m_X;
        if ((Anchor & GUIControl::Anchor_Bottom) && !(Anchor & GUIControl::Anchor_Top))
            DY = m_Height - (OldHeight-(CY-m_Y)) + m_Y;

        if (DX != CX || DY != CY)
            C->Move(DX, DY);

        CX -= m_X;
        CY -= m_Y;
        
        // Attached to opposing edges
        if (Anchor & GUIControl::Anchor_Left && Anchor & GUIControl::Anchor_Right)
            W = (m_Width - (OldWidth - (CX+CW))) - CX;
        if (Anchor & GUIControl::Anchor_Top && Anchor & GUIControl::Anchor_Bottom)
            H = (m_Height - (OldHeight-(CY+CH))) - CY;

        if (W != CW || H != CH)
            C->Resize(W, H);
    }

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUICollectionBox::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDrawImage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drawing image bitmap to draw

void GUICollectionBox::SetDrawImage(GUIBitmap *Bitmap)
{
    // Free any old bitmap
    delete m_DrawBitmap;

    m_DrawBitmap = Bitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDrawBackground
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether to draw the background.

void GUICollectionBox::SetDrawBackground(bool DrawBack)
{
    m_DrawBackground = DrawBack;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDrawType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drawing type.

void GUICollectionBox::SetDrawType(int Type)
{
    m_DrawType = Type;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDrawColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drawing color.

void GUICollectionBox::SetDrawColor(unsigned long Color)
{
    m_DrawColor = Color;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUICollectionBox::StoreProperties(void)
{
    m_Properties.AddVariable("DrawBackground", m_DrawBackground);
    m_Properties.AddVariable("DrawType", m_DrawType == Color ? "Color" : "Image");
    m_Properties.AddVariable("DrawColor", (int)m_DrawColor);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies new properties to the control.

void GUICollectionBox::ApplyProperties(GUIProperties *Props)
{
    GUIControl::ApplyProperties(Props);

    // Get the values
    m_Properties.GetValue("DrawBackground", &m_DrawBackground);
    string v;
    m_Properties.GetValue("DrawType", &v);
    if (stricmp(v.c_str(), "Color") == 0)
        m_DrawType = Color;
    else if (stricmp(v.c_str(), "Image") == 0)
        m_DrawType = Image;

    m_Properties.GetValue("DrawColor", &m_DrawColor);
}