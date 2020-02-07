//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIPropertyPage.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIPropertyPage class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "GUIPropertyPage.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIPropertyPage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIPropertyPage object in
//                  system memory.

GUIPropertyPage::GUIPropertyPage(GUIManager *Manager, GUIControlManager *ControlManager)
: GUIPanel(Manager),
  GUIControl()
{
    m_ControlID = "PROPERTYPAGE";
    m_DrawBitmap = 0;
    m_ControlManager = ControlManager;
    m_Font = 0;
    m_VertScroll = 0;
    m_FontColor = 0;
    m_LineColor = 0;
    m_PageValues.Clear();
    m_TextPanelList.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIPropertyPage::Create(const std::string Name, int X, int Y, int Width, int Height)
{
    GUIControl::Create(Name, X, Y, Width, Height);

    // Minimum size of the control
    m_MinWidth = 50;
    m_MinHeight = 50;

    // Default size of the control
    m_DefWidth = 80;
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

    // Make sure the control isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    // Create the vertical scrollbar
    m_VertScroll = new GUIScrollPanel(m_Manager);

    m_VertScroll->Create(m_Width-12, 0, 12, m_Height);
    m_VertScroll->SetOrientation(GUIScrollPanel::Vertical);
    m_VertScroll->_SetVisible(false);
    m_VertScroll->SetValue(0);
    m_VertScroll->SetSignalTarget(this);

    GUIPanel::AddChild(m_VertScroll);

    // Create the text panels
    int H = 16;
    int Spacer = 0;
    int Size = m_Height/H;
    for(int i=0; i<Size; i++) {
        GUITextPanel *T = new GUITextPanel(m_Manager);
        T->Create(m_Width/2, i*H+Spacer, m_Width/2, H);
        T->_SetVisible(false);
        T->SetSignalTarget(this);
        GUIPanel::AddChild(T);

        m_TextPanelList.push_back(T);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been created.

void GUIPropertyPage::Create(GUIProperties *Props)
{
    GUIControl::Create(Props);

    // Minimum size of the control
    m_MinWidth = 50;
    m_MinHeight = 50;

    // Default size of the control
    m_DefWidth = 80;
    m_DefHeight = 100;

    // Setup the panel
    GUIPanel::LoadProperties(Props);

    // Make sure the control isn't too small
    m_Width = MAX(m_Width, m_MinWidth);
    m_Height = MAX(m_Height, m_MinHeight);

    // Create the vertical scrollbar
    m_VertScroll = new GUIScrollPanel(m_Manager);

    m_VertScroll->Create(m_Width-12, 0, 12, m_Height);
    m_VertScroll->SetOrientation(GUIScrollPanel::Vertical);
    m_VertScroll->_SetVisible(false);
    m_VertScroll->SetValue(0);
    m_VertScroll->SetSignalTarget(this);

    GUIPanel::AddChild(m_VertScroll);

    // Create the text panels
    int H = 16;
    int Spacer = 0;
    int Size = m_Height/H;
    for(int i=0; i<Size; i++) {
        GUITextPanel *T = new GUITextPanel(m_Manager);
        T->Create(m_Width/2, i*H+Spacer, m_Width/2, H);
        T->_SetVisible(false);
        T->SetSignalTarget(this);
        GUIPanel::AddChild(T);

        m_TextPanelList.push_back(T);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control has been destroyed.

void GUIPropertyPage::Destroy(void)
{
    // Free the drawing bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Free the vertical scrollbar
    if (m_VertScroll) {
        m_VertScroll->Destroy();
        delete m_VertScroll;
        m_VertScroll = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the skin has been changed.

void GUIPropertyPage::ChangeSkin(GUISkin *Skin)
{
    GUIControl::ChangeSkin(Skin);

    // Change the skin of the text panels
    for(int i=0; i<m_TextPanelList.size(); i++) {
        GUITextPanel *T = (GUITextPanel *)m_TextPanelList.at(i);
        T->ChangeSkin(Skin);
    }

    // Build the control bitmap
    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BuildBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create the control bitmap to draw.

void GUIPropertyPage::BuildBitmap(void)
{
    // Free any old bitmap
    if (m_DrawBitmap) {
        m_DrawBitmap->Destroy();
        delete m_DrawBitmap;
        m_DrawBitmap = 0;
    }

    // Create a new bitmap.
    m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

    m_Skin->BuildStandardRect(m_DrawBitmap, "PropertyPage", 0, 0, m_Width, m_Height);

    

    // Pre-cache the font
    string Filename;
    m_Skin->GetValue("PropertyPage", "Font", &Filename);
    m_Skin->GetValue("PropertyPage", "FontShadow", &m_FontShadow);
    m_Skin->GetValue("PropertyPage", "FontColor", &m_FontColor);
    m_Skin->GetValue("PropertyPage", "FontKerning", &m_FontKerning);
    
    m_FontColor = m_Skin->ConvertColor(m_FontColor, m_DrawBitmap->GetColorDepth());
    
    m_Font = m_Skin->GetFont(Filename);
    if (m_Font)
        m_Font->CacheColor(m_FontColor);

    m_Skin->GetValue("PropertyPage", "LineColor", &m_LineColor);
    m_LineColor = m_Skin->ConvertColor(m_LineColor, m_DrawBitmap->GetColorDepth());

/*    // Create the button image
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Up", 0, 0, m_Width, m_Height);
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Over", 0, m_Height, m_Width, m_Height);
    m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Down", 0, m_Height*2, m_Width, m_Height);    

    // Draw the text
//    int y = m_Height/2-m_Font->CalculateHeight(m_Text)+2;
    int y = (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1;

    m_Font->SetColor(m_FontColor);
    m_Font->SetKerning(m_FontKerning);

    m_Font->DrawAligned(m_DrawBitmap, m_Width/2, y, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);
    m_Font->DrawAligned(m_DrawBitmap, m_Width/2, m_Height+y, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);
    m_Font->DrawAligned(m_DrawBitmap, m_Width/2+1, m_Height*2+y+1, m_Text, GUIFont::Centre, GUIFont::Top, m_Width, m_FontShadow);*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the panel

void GUIPropertyPage::Draw(GUIScreen *Screen)
{
    if (m_DrawBitmap)
        m_DrawBitmap->Draw(Screen->GetBitmap(), m_X, m_Y, 0);

    // Check the font first
    if (!m_Font)
        return;

    // Draw the properties
    int Count = m_PageValues.GetCount();
    int Spacer = 2;
    int Y = m_Y+Spacer;
    int Size = 16;
    string Name, Value;    
    
    for(int i=0; i<Count; i++) {
        m_PageValues.GetVariable(i, &Name, &Value);
        m_Font->SetColor(m_FontColor);
        m_Font->SetKerning(m_FontKerning);
        m_Font->Draw(Screen->GetBitmap(), m_X+Spacer, Y, Name, m_FontShadow); 

        Screen->GetBitmap()->DrawRectangle(m_X+1, Y+Size + (m_Font->GetFontHeight()/2 - Size/2),
                                            m_Width-2, 0, m_LineColor, false);

        Y += Size;
    }

    Screen->GetBitmap()->DrawRectangle(m_X+m_Width/2, m_Y+1, 0, Y-m_Y-Spacer*2, m_LineColor, false);
    
    
    GUIPanel::Draw(Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseDown
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes down on the panel

void GUIPropertyPage::OnMouseDown(int X, int Y, int Buttons, int Modifier)
{
    if (Buttons & MOUSE_LEFT) {
        // Push the button down
        //m_Pushed = true;
        //CaptureMouse();

        //AddEvent(GUIEvent::Notification, Pushed, 0);
    }
    SetFocus();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseUp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse goes up on the panel

void GUIPropertyPage::OnMouseUp(int X, int Y, int Buttons, int Modifier)
{
    /*if (PointInside(X, Y))
        AddEvent(GUIEvent::Notification, Clicked, Buttons);

    if (!IsCaptured())
        return;

    m_Pushed = false;
    ReleaseMouse();

    // If the mouse is over the button, add the command to the event queue
    if (PointInside(X, Y))
        AddEvent(GUIEvent::Command, 0, 0);

    AddEvent(GUIEvent::Notification, UnPushed, 0);*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseEnter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse enters the panel.

void GUIPropertyPage::OnMouseEnter(int X, int Y, int Buttons, int Modifier)
{
    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseLeave
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse leaves the panel.

void GUIPropertyPage::OnMouseLeave(int X, int Y, int Buttons, int Modifier)
{
    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnMouseMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the mouse moves (over the panel, or when captured).

void GUIPropertyPage::OnMouseMove(int X, int Y, int Buttons, int Modifier)
{
    if (!(Buttons & MOUSE_LEFT) || !IsCaptured())
        return;

    // If the mouse goes outside of the button, un-push the button
    /*if (!PointInside(X, Y)) {
        if (m_Pushed) {
            AddEvent(GUIEvent::Notification, UnPushed, 0);
            m_Pushed = false;
        }
    } else {
        if (!m_Pushed) {
            AddEvent(GUIEvent::Notification, Pushed, 0);
            m_Pushed = true;
        }
    }*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the panel of the control.

GUIPanel *GUIPropertyPage::GetPanel(void)
{
    return this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rectangle of the control.

void GUIPropertyPage::GetControlRect(int *X, int *Y, int *Width, int *Height)
{
    GUIPanel::GetRect(X, Y, Width, Height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Move
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be moved.

void GUIPropertyPage::Move(int X, int Y)
{
    GUIPanel::SetPositionAbs(X, Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Resize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the control needs to be resized.

void GUIPropertyPage::Resize(int Width, int Height)
{
    // Make sure the control isn't too small
    Width = MAX(Width, m_MinWidth);
    Height = MAX(Height, m_MinHeight);

    GUIPanel::SetSize(Width, Height);

// TODO: Alter text panels

    BuildBitmap();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StoreProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control to store the values into properties.

void GUIPropertyPage::StoreProperties(void)
{
    // Note: This is for saving the control, not related directly to our control type
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPropertyValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Refreshes the page with new variables & values.

void GUIPropertyPage::SetPropertyValues(GUIProperties *Props)
{
    m_PageValues.Clear();
    m_PageValues.Update(Props, true);

    // Sort
    m_PageValues.Sort(true);


    // Update the text panels
    for(int i=0; i<m_TextPanelList.size(); i++) {
        GUITextPanel *T = (GUITextPanel *)m_TextPanelList.at(i);
        T->_SetVisible(false);
        T->SetText("");

        if (i < m_PageValues.GetCount()) {            
            T->_SetVisible(true);
            string Name, Value;
            if (m_PageValues.GetVariable(i, &Name, &Value))
                T->SetText(Value);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPropertyValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the properties in the page.

GUIProperties *GUIPropertyPage::GetPropertyValues(void)
{
    return &m_PageValues;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReceiveSignal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when receiving a signal.

void GUIPropertyPage::ReceiveSignal(GUIPanel *Source, int Code, int Data)
{
    assert(Source);

    bool TextSignal = false;

    // Is this a text panel?
    vector<GUITextPanel *>::iterator it;
    for(it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
        GUITextPanel *T = *it;

        if (Source->GetPanelID() == T->GetPanelID()) {
            TextSignal = true;

            // Change event. Do not update properties
            if (Code == GUITextPanel::Changed) {
                AddEvent(GUIEvent::Notification, GUIPropertyPage::Changed, 0);
                return;
            }

            break;
        }
    }

    // Update the properties.
    // If any of the values are different, fire a 'changed' notification event
    if (TextSignal) {
        
        // Update the text panels
        if (InvokeUpdate()) {
            // Fire the enter event
            AddEvent(GUIEvent::Notification, GUIPropertyPage::Enter, 0);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          InvokeUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Invokes an explicit update on text panels to property page.

bool GUIPropertyPage::InvokeUpdate(void)
{
    bool Changed = false;

    for(int i=0; i<m_TextPanelList.size(); i++) {
        GUITextPanel *T = (GUITextPanel *)m_TextPanelList.at(i);
        
        if (i < m_PageValues.GetCount()) {
            string Name, Value;
            if (m_PageValues.GetVariable(i, &Name, &Value)) {
                if (T->GetText().compare(Value) != 0)
                    Changed = true;
                
                // Set the value
                m_PageValues.SetVariable(i, Name, T->GetText());
            }
        }
    }

    return Changed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the property page values.

void GUIPropertyPage::ClearValues(void)
{
    m_PageValues.Clear();

    // Hide the text panels
    vector<GUITextPanel *>::iterator it;
    for(it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
        GUITextPanel *T = *it;
        T->_SetVisible(false);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasTextFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if any of the visible text panels have focus.

bool GUIPropertyPage::HasTextFocus(void)
{
    vector<GUITextPanel *>::iterator it;
    for(it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
        GUITextPanel *T = *it;

        // Visible & has focus??
        if (T->_GetVisible() && T->HasFocus())
            return true;
    }

    return false;
}
