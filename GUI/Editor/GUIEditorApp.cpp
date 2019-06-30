//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIEditorApp.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUI Editor App Class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jboett@internode.on.net


#include "GUI.h"
#include "allegro.h"
#include "GUIEditorApp.h"
#include "DDTError.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "AllegroBitmap.h"

#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUIPropertyPage.h"
#include "GUICheckbox.h"

#include "GUIEditorLib.h"

using namespace RTE;


AllegroScreen    *g_Screen;
AllegroInput    *g_Input;

extern    GUIEditorApp g_GUIEditor;

#define ROOTORIGINX 300
#define ROOTORIGINY 120


//////////////////////////////////////////////////////////////////////////////////////////
// Quit Handler for Allegro
//////////////////////////////////////////////////////////////////////////////////////////
volatile bool g_Quit = false;
void QuitHandler(void)
{
    g_GUIEditor.OnQuitButton();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIEditorApp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIEditorApp object in system
//                  memory.

GUIEditorApp::GUIEditorApp()
{

}


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GUIEditorApp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GUIEditorApp object.

GUIEditorApp::~GUIEditorApp()
{

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Initialize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Initializes the editor app

bool GUIEditorApp::Initialize(void)
{
    allegro_init();
    install_keyboard();

    int ResW = 1024;
    int ResH = 768;
    int BPP = 32;

    COLOR_MAP m_LessTransTable;
    COLOR_MAP m_HalfTransTable;
    COLOR_MAP m_MoreTransTable;

    set_color_depth(BPP);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, ResW, ResH, 0, 0);
    set_close_button_callback(QuitHandler);
    set_window_title("Cortex Command: GUI Editor");

    set_color_conversion(COLORCONV_MOST);

    // Just going to discard the bitmap, we're only interested in the palette
    BITMAP *tempBitmap;
    PALETTE newPalette;
    //if (!(tempBitmap = load_bitmap("Palette.bmp", NULL)))// newPalette)))
      //  DDTAbort("Failed to load palette from bitmap with following path:\n\nBase.rte/palette.bmp");

    PALETTE ccpal;
    get_palette(ccpal);
    create_trans_table(&m_LessTransTable, ccpal, 192, 192, 192, 0);
    create_trans_table(&m_HalfTransTable, ccpal, 128, 128, 128, 0);
    create_trans_table(&m_MoreTransTable, ccpal, 64, 64, 64, 0);
	color_map = &m_HalfTransTable;

    m_pBackBuffer32 = create_bitmap_ex(BPP, ResW, ResH);
    clear_to_color(m_pBackBuffer32, 0);

    g_Screen = new AllegroScreen(m_pBackBuffer32);
    g_Input = new AllegroInput(-1);

    

    // Set the current palette
    set_palette(newPalette);

    // Update what black is now with the loaded palette
    m_BlackColor = bestfit_color(newPalette, 0, 0, 0);

    // Free the temp bitmap that had the palette
    //destroy_bitmap(tempBitmap);

    clear_to_color(m_pBackBuffer32, m_BlackColor);


    // Initialize the UI
    m_pControlManager = new GUIControlManager();
    m_pControlManager->Create(g_Screen, g_Input, "Base.rte/GUIs/Skins/Editor");

    m_pEditorManager = new GUIControlManager();
    m_pEditorManager->Create(g_Screen, g_Input, "Base.rte/GUIs/Skins/Editor");

    m_pEditorManager->EnableMouse();


    GUICollectionBox * g_EdBase = (GUICollectionBox *)m_pEditorManager->AddControl("base", "COLLECTIONBOX", NULL, 0,0, 1024, 768);
    g_EdBase->SetDrawBackground(true);
    g_EdBase->SetDrawColor(makecol(32, 32, 32));
    g_EdBase->SetDrawType(GUICollectionBox::Color);

    // Add an area showing the editing box
    GUICollectionBox *g_EdArea = (GUICollectionBox *)m_pEditorManager->AddControl("editArea", "COLLECTIONBOX", g_EdBase, ROOTORIGINX, ROOTORIGINY, 640, 480);
    g_EdArea->SetDrawBackground(true);
    g_EdArea->SetDrawColor(makecol(64, 64, 64));
    g_EdArea->SetDrawType(GUICollectionBox::Color);

    // Add the root collection box for the edited document
    GUICollectionBox *Root = (GUICollectionBox *)m_pControlManager->AddControl("root", "COLLECTIONBOX", NULL, ROOTORIGINX, ROOTORIGINY, 640, 480);
    Root->SetDrawBackground(false);
    m_pRootControl = Root;

    // Add the left tool buttons
    GUIButton *Quit = (GUIButton *)m_pEditorManager->AddControl("btnQuit", "BUTTON", g_EdBase, 5,5, 80,20);
    Quit->SetText("Quit");
    GUIButton *Load = (GUIButton *)m_pEditorManager->AddControl("btnLoad", "BUTTON", g_EdBase, 5,30, 80,20);
    Load->SetText("Load");
    GUIButton *Add = (GUIButton *)m_pEditorManager->AddControl("btnAdd", "BUTTON", g_EdBase, 90,30, 40,20);
    Add->SetText("Add");
    GUIButton *Save = (GUIButton *)m_pEditorManager->AddControl("btnSave", "BUTTON", g_EdBase, 5,55, 80,20);
    Save->SetText("Save");
    GUIButton *SaveAs = (GUIButton *)m_pEditorManager->AddControl("btnSaveAs", "BUTTON", g_EdBase, 5,80, 80,20);
    SaveAs->SetText("Save As");
    
    GUIButton *Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_BUTTON", "BUTTON", g_EdBase, 160,5, 80,20);
    Ctrl->SetText("BUTTON");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_CHECKBOX", "BUTTON", g_EdBase, 160,30, 80,20);
    Ctrl->SetText("CHECKBOX");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_COLLECTIONBOX", "BUTTON", g_EdBase, 160,55, 80,20);
    Ctrl->SetText("COLLECTIONBOX");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_COMBOBOX", "BUTTON", g_EdBase, 160,80, 80,20);
    Ctrl->SetText("COMBOBOX");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_LABEL", "BUTTON", g_EdBase, 250,5, 80,20);
    Ctrl->SetText("LABEL");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_LISTBOX", "BUTTON", g_EdBase, 250,30, 80,20);
    Ctrl->SetText("LISTBOX");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_PROGRESSBAR", "BUTTON", g_EdBase, 250,55, 80,20);
    Ctrl->SetText("PROGRESSBAR");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_RADIOBUTTON", "BUTTON", g_EdBase, 250,80, 80,20);
    Ctrl->SetText("RADIOBUTTON");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_SCROLLBAR", "BUTTON", g_EdBase, 340,5, 80,20);
    Ctrl->SetText("SCROLLBAR");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_SLIDER", "BUTTON", g_EdBase, 340,30, 80,20);
    Ctrl->SetText("SLIDER");
    Ctrl = (GUIButton *)m_pEditorManager->AddControl("C_TEXTBOX", "BUTTON", g_EdBase, 340,55, 80,20);
    Ctrl->SetText("TEXTBOX");

    m_pPropertyPage = (GUIPropertyPage *)m_pEditorManager->AddControl("props", "PROPERTYPAGE", g_EdBase, 5, 120, 250, 250);

    m_pActiveBoxList = (GUIListBox *)m_pEditorManager->AddControl("active", "LISTBOX", g_EdBase, 5, 400, 150, 250);

    GUICheckbox *Snap = (GUICheckbox *)m_pEditorManager->AddControl("snap", "CHECKBOX", g_EdBase, 450, 10, 80,16);
    Snap->SetText("Snap");
    Snap->SetCheck(GUICheckbox::Checked);

    m_bSnapGrid = true;
    m_nGridSize = 5;


    /*m_pControlManager->Load("Base.rte/GUIs/ObjectPickerGUI.ini");
    GUIControl *C = *m_pControlManager->GetControlList()->begin();
    m_pRootControl = C;
    C->Move(ROOTORIGINX, ROOTORIGINY);
    C->StoreProperties();

    GUIProperties cProps;
    cProps.Update(C->GetProperties(), true);
    C->GetPanel()->BuildProperties(&cProps);
    */


    // Clear settings
    m_bDirty = false;
    ClearSelection();

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the editor app.

bool GUIEditorApp::Update(void)
{
    clear_to_color(m_pBackBuffer32, m_BlackColor);

    m_pEditorManager->Update();

    // Process the editor events
    GUIEvent Event;
    while(m_pEditorManager->GetEvent(&Event))
    {
        switch(Event.GetType())
        {

            // Command
            case GUIEvent::Command:

                // Invoke an update                
                //EDInvokeUpdate();
                
                // Quit
                if (Event.GetControl()->GetName() == "btnQuit")
                {
                    OnQuitButton();
                    break;
                }

                // Load
                if (Event.GetControl()->GetName() == "btnLoad")
                {
                    OnLoadButton();
                    break;
                }

                // Add
                if (Event.GetControl()->GetName() == "btnAdd")
                {
                    OnLoadButton(true);
                    break;
                }

                // Save
                if (Event.GetControl()->GetName() == "btnSave")
                {
                    OnSaveButton();
                    break;
                }

                // Save As
                if (Event.GetControl()->GetName() == "btnSaveAs")
                {
                    OnSaveAsButton();
                    break;
                }

                // Add a control
                if (Event.GetControl()->GetName().substr(0, 2).compare("C_") == 0)
                {
                    string Class = ((GUIButton *)Event.GetControl())->GetText();
                    GUIControl *Parent = m_pRootControl;

                    // Is the focused control a container?
                    if (m_SelectionInfo.m_Control)
                    {
                        if (m_SelectionInfo.m_Control->IsContainer())
                            Parent = m_SelectionInfo.m_Control;
                    }

                    // Find a suitable control name
                    string Name = GenerateControlName(Class/*.substr(2)*/);

                    if (Parent)
                    {
                        m_pControlManager->AddControl(Name, Class, Parent, 0, 0, -1,-1);
                    }

                    break;
                }

                break;

            // Notification
            case GUIEvent::Notification:

                // Property Page changed
                if (Event.GetControl()->GetName() == "props")
                {

                    if (Event.GetMsg() == GUIPropertyPage::Enter)
                    {
                        // Update the focused control properties

                        // Apply the properties
                        GUIControl *C = m_SelectionInfo.m_Control;
                        if (C)
                        {
                            C->ApplyProperties(m_pPropertyPage->GetPropertyValues());
                            // Update the active box list in case the name of a top-level box changed
                            UpdateActiveBoxList();
                        }

                        m_bDirty = true;
                    }

                    if (Event.GetMsg() == GUIPropertyPage::Changed)
                    {
                        // The properties are dirty and need to be updated
                        m_bDirty = true;
                    }
                }

                // Active Box changed
                if (Event.GetControl()->GetName() == "active")
                {
                    // Clicks in the list
                    if(Event.GetMsg() == GUIListBox::MouseDown)
                    {
                        GUIListPanel::Item *pItem = m_pActiveBoxList->GetSelected();
                        if (pItem)
                        {
                            // Try to find the box of that name, and select it
                            GUIControl *pBoxControl = m_pControlManager->GetControl(pItem->m_Name);
                            if (pBoxControl)
                            {
                                m_SelectionInfo.m_GrabbedControl = false;
                                m_SelectionInfo.m_GrabbedHandle = false;
                                m_SelectionInfo.m_Control = pBoxControl;
                            }
                        }
                        // Deselection if clicked on no list item
                        else
                        {
                            m_SelectionInfo.m_GrabbedControl = false;
                            m_SelectionInfo.m_GrabbedHandle = false;
                            m_SelectionInfo.m_Control = NULL;
                        }
				    }
                }


                // Snap
                if (Event.GetControl()->GetName() == "snap")
                {
                    m_bSnapGrid = ((GUICheckbox *)Event.GetControl())->GetCheck() == GUICheckbox::Checked;
                }
                break;
        }
    }

    m_pEditorManager->Draw();

    m_pControlManager->Draw();

    // Process the editor
    ProcessEditor();

    if (m_SelectionInfo.m_Control)
        DrawSelectedControl(m_SelectionInfo.m_Control);

    m_pEditorManager->DrawMouse();


    blit(m_pBackBuffer32, screen, 0, 0, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h);

    return !g_Quit;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnQuitButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the quit button has been pushed

void GUIEditorApp::OnQuitButton(void)
{
    // If dirty, ask if we want to save
    // Otherwise, just quit
    if (m_bDirty)
    {
        int nRetCode = GUIEditorLib::QuitMessageBox("Save changes made?", "Cortex Command: GUI Editor");
        if (nRetCode == 1)
        {
            OnSaveButton();
            g_Quit = true;
        }
        if (nRetCode == -1)
            g_Quit = true;
    }
    else
    {
        g_Quit = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnLoadButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the load button has been pushed

void GUIEditorApp::OnLoadButton(bool addControls)
{
    string strFilename;
    if (GUIEditorLib::DisplayLoadGUIFile(&strFilename))
    {
        m_pControlManager->Load(strFilename, addControls);

        GUIControl *C = m_pControlManager->GetControlList()->front();
        m_pRootControl = C;

        C->Move(ROOTORIGINX, ROOTORIGINY);

        C->StoreProperties();

        GUIProperties cProps;
        cProps.Update(C->GetProperties(), true);
        C->GetPanel()->BuildProperties(&cProps);

        // Clear settings
        m_bDirty = false;
        ClearSelection();
        m_pPropertyPage->ClearValues();

        UpdateActiveBoxList();

        m_strFilename = strFilename;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSaveAsButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the save as button has been pushed

void GUIEditorApp::OnSaveAsButton(void)
{
    string strFilename;
    if (GUIEditorLib::DisplaySaveGUIFile(&strFilename))
    {
        // Move the root object to the origin before saving
        m_pRootControl->Move(0,0);
        
        m_pControlManager->Save(strFilename);

        // Move it back
        m_pRootControl->Move(ROOTORIGINX, ROOTORIGINY);

        m_strFilename = strFilename;

        m_bDirty = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSaveButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Called when the save button has been pushed

void GUIEditorApp::OnSaveButton(void)
{
    if (m_strFilename.size() == 0)
        OnSaveAsButton();
    else
    {
        // Move the root object to the origin before saving
        m_pRootControl->Move(0,0);

        m_pControlManager->Save(m_strFilename);

        // Move it back
        m_pRootControl->Move(ROOTORIGINX, ROOTORIGINY);

        m_bDirty = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ProcessEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Process the editor logic

void GUIEditorApp::ProcessEditor(void)
{
    int Events[3], States[3];
    int MouseX, MouseY;
    g_Input->GetMouseButtons(Events, States);
    g_Input->GetMousePosition(&MouseX, &MouseY);

    unsigned char KeyboardBuffer[256];
    g_Input->GetKeyboard(KeyboardBuffer);


    // Delete key
    if (KeyboardBuffer[GUIInput::Key_Delete] == GUIInput::Pushed && !m_pPropertyPage->HasTextFocus())
    {
        // Remove the control
        if (m_SelectionInfo.m_Control)
        {
            m_pControlManager->RemoveControl(m_SelectionInfo.m_Control->GetName(), true);

            m_SelectionInfo.m_Control = NULL;
            m_SelectionInfo.m_GrabbedControl = false;
            m_SelectionInfo.m_GrabbedHandle = false;

            m_pPropertyPage->ClearValues();
        }
    }

    // Escape key
    if (KeyboardBuffer[GUIInput::Key_Escape] == GUIInput::Pushed)
    {
        // Undo any grab
        m_SelectionInfo.m_GrabbedControl = false;
        m_SelectionInfo.m_GrabbedHandle = false;
    }


    // If click released
    if (Events[0] == GUIInput::Released) {
        
        // Move the control after a grab
        if (m_SelectionInfo.m_GrabbedControl && m_SelectionInfo.m_TriggerGrab)
        {
            if (m_SelectionInfo.m_Control)
            {
                // TODO: Check if not moved to another parent
                int DestX = ProcessSnapCoord(MouseX + m_SelectionInfo.m_GrabX);
                int DestY = ProcessSnapCoord(MouseY + m_SelectionInfo.m_GrabY);
                m_SelectionInfo.m_Control->Move(DestX, DestY); 

                m_bDirty = true;
            }
        }

        // Resize/Move control after a grab
        if (m_SelectionInfo.m_GrabbedHandle && m_SelectionInfo.m_TriggerGrab)
        {
            if (m_SelectionInfo.m_Control)
            {
                int X, Y, Width, Height;
                CalculateHandleResize(MouseX, MouseY, &X, &Y, &Width, &Height);

                m_SelectionInfo.m_Control->Move(X, Y);
                m_SelectionInfo.m_Control->Resize(Width, Height);

                m_bDirty = true;
            }
        }

        // Update properties
        if (!ControlUnderMouse(m_pPropertyPage, MouseX, MouseY))
        {
            if (m_SelectionInfo.m_Control)
            {
                m_SelectionInfo.m_Control->StoreProperties();

                GUIProperties P;
                P.Update(m_SelectionInfo.m_Control->GetProperties(), true);
                m_SelectionInfo.m_Control->GetPanel()->BuildProperties(&P);
                m_pPropertyPage->SetPropertyValues(&P);

                m_bDirty = true;
            }
        }

        m_SelectionInfo.m_GrabbedControl = false;
        m_SelectionInfo.m_GrabbedHandle = false;
        m_SelectionInfo.m_TriggerGrab = false;
    }


    // Check for grabbing handles
    if (!m_SelectionInfo.m_GrabbedControl && m_SelectionInfo.m_Control && Events[0] == GUIInput::Pushed)
    {
        int HandleIndex = HandleUnderMouse(m_SelectionInfo.m_Control, MouseX, MouseY);
        if (HandleIndex != -1)
        {
            m_SelectionInfo.m_GrabbedControl = false;
            m_SelectionInfo.m_GrabbedHandle = true;
            m_SelectionInfo.m_HandleIndex = HandleIndex;

            m_SelectionInfo.m_GrabX = MouseX;
            m_SelectionInfo.m_GrabY = MouseY;
            m_SelectionInfo.m_ClickX = MouseX;
            m_SelectionInfo.m_ClickY = MouseY;
        }
    }

    // If we've grabbed a control or handle, and we've moved far enough from the starting spot, trigger the grab
    // This prevents quickly selecting a control and slightly moving a couple pixels before releasing
    if ((m_SelectionInfo.m_GrabbedControl || m_SelectionInfo.m_GrabbedHandle) && !m_SelectionInfo.m_TriggerGrab)
    {
        int MoveDist = 4;

        if (fabsf(m_SelectionInfo.m_ClickX - MouseX) >= MoveDist || fabsf(m_SelectionInfo.m_ClickY - MouseY) >= MoveDist)
            m_SelectionInfo.m_TriggerGrab = true;
    }



    // Check if mouse clicked on a control
    if (!m_SelectionInfo.m_GrabbedControl && !m_SelectionInfo.m_GrabbedHandle && Events[0] == GUIInput::Pushed)
    {
        GUIControl *C = ControlUnderMouse(m_pRootControl, MouseX, MouseY);
        if (C && C != m_pRootControl) {
            int X, Y, Width, Height;
            C->GetControlRect(&X, &Y, &Width, &Height);

            m_SelectionInfo.m_GrabbedHandle = false;

            m_SelectionInfo.m_GrabbedControl = true;
            m_SelectionInfo.m_GrabX = X - MouseX;
            m_SelectionInfo.m_GrabY = Y - MouseY;
            m_SelectionInfo.m_ClickX = MouseX;
            m_SelectionInfo.m_ClickY = MouseY;

            m_SelectionInfo.m_Control = C;

            // Set the properties
            C->StoreProperties();

            GUIProperties P;
            P.Update(C->GetProperties(), true);
            C->GetPanel()->BuildProperties(&P);
            m_pPropertyPage->SetPropertyValues(&P);

        }
        else if (C == m_pRootControl)
        {
            // Unselect control
            m_SelectionInfo.m_GrabbedControl = false;
            m_SelectionInfo.m_GrabbedHandle = false;
            m_SelectionInfo.m_Control = NULL;

            m_pPropertyPage->ClearValues();
        }

        // Update the active box list in case we selected/deselected a top level collection box
        UpdateActiveBoxList();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateActiveBoxList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the list of Active CollectionBox:es found in the editor under
//                  the root box.

void GUIEditorApp::UpdateActiveBoxList(void)
{
    // Clear the list so we can repopulate it
    m_pActiveBoxList->ClearList();

    // Go through all the top-level (directly under root) controls and add only the ControlBoxs to the list here
    vector<GUIControl *> *pControls = m_pControlManager->GetControlList();
    GUICollectionBox *pBox = 0;
    for (vector<GUIControl *>::iterator itr = pControls->begin(); itr != pControls->end(); itr++)
    {
        // Look for CollectionBoxes with the root control as parent
        if ((pBox = dynamic_cast<GUICollectionBox *>(*itr)) && pBox->GetParent() == m_pRootControl)
        {
            m_pActiveBoxList->AddItem(pBox->GetName());
            // Check if this is selected in the editor, and if so, select it in the list too
            if (pBox == m_SelectionInfo.m_Control)
                m_pActiveBoxList->SetSelectedIndex(m_pActiveBoxList->GetItemList()->size() - 1);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ControlUnderMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a control is under the mouse point

GUIControl * GUIEditorApp::ControlUnderMouse(GUIControl *Parent, int MouseX, int MouseY)
{
    assert(Parent);

    // Clicked on the parent?
    int X, Y, Width, Height;
    Parent->GetControlRect(&X, &Y, &Width, &Height);


    if (MouseX < X || MouseX > X+Width)
        return NULL;

    if (MouseY < Y || MouseY > Y+Height)
        return NULL;

    // Check children
    vector<GUIControl *> *List = Parent->GetChildren();
    vector<GUIControl *>::reverse_iterator it;

    assert(List);

    for (it = List->rbegin(); it != List->rend(); it++)
    {
        GUIControl *C = ControlUnderMouse(*it, MouseX, MouseY);
        if (C)
            return C;
    }

    // Return this
    return Parent;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HandleUnderMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a control's handle is under the mouse point

int GUIEditorApp::HandleUnderMouse(GUIControl *Control, int MouseX, int MouseY)
{
    int X, Y, Width, Height;
    Control->GetControlRect(&X, &Y, &Width, &Height);

    int RegionSize = 6;

    int nHandle = 0;

    for (int i=0; i<3; i++)
    {
        if (MouseInsideBox(MouseX, MouseY, X-RegionSize, Y+i*(Height/2)-RegionSize, RegionSize*2, RegionSize*2))
            return nHandle;
        
        nHandle++;
        if (i != 1)
        {
            if (MouseInsideBox(MouseX, MouseY, X+Width/2-RegionSize, Y+i*(Height/2)-RegionSize, RegionSize*2, RegionSize*2))
                return nHandle;
        }

        nHandle++;

        if (MouseInsideBox(MouseX, MouseY, X+Width-RegionSize, Y+i*(Height/2)-RegionSize, RegionSize*2, RegionSize*2))
                return nHandle;

        nHandle++;
    }

    // Not over any handle
    return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawSelectedControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws selection info around a control

void GUIEditorApp::DrawSelectedControl(GUIControl *Control)
{
    int i;
    int HandleSize = 6;

    assert(Control);

    int MouseX, MouseY;
    g_Input->GetMousePosition(&MouseX, &MouseY);

    int X, Y, Width, Height;
    Control->GetControlRect(&X, &Y, &Width, &Height);

    // If we've grabbed the control, draw the selection lines where the mouse is
    if (m_SelectionInfo.m_GrabbedControl && m_SelectionInfo.m_TriggerGrab)
    {
        X = MouseX + m_SelectionInfo.m_GrabX;
        Y = MouseY + m_SelectionInfo.m_GrabY;

        X = ProcessSnapCoord(X);
        Y = ProcessSnapCoord(Y);
    }

    // Grabbed handles
    if (m_SelectionInfo.m_GrabbedHandle && m_SelectionInfo.m_TriggerGrab)
    {
        CalculateHandleResize(MouseX, MouseY, &X, &Y, &Width, &Height);
    }


    RECT Rect;
    SetRect(&Rect, X-6, Y-6, X+Width+6, Y+Height+6);
    g_Screen->GetBitmap()->SetClipRect(&Rect);

    g_Screen->GetBitmap()->DrawRectangle(X, Y, Width, Height, 0xFFCCCCCC, false);

    // Draw the handles
    for (int i=0; i<3; i++)
    {
        DrawSelectionHandle(X, Y+i*(Height/2), HandleSize, HandleSize);
        if (i != 1)
            DrawSelectionHandle(X+Width/2, Y+i*(Height/2), HandleSize, HandleSize);

        DrawSelectionHandle(X+Width, Y+i*(Height/2), HandleSize, HandleSize);
    }

    g_Screen->GetBitmap()->SetClipRect(NULL);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawSelectionHandle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a selection handle

void GUIEditorApp::DrawSelectionHandle(int X, int Y, int Width, int Height)
{
    g_Screen->GetBitmap()->DrawRectangle(X-Width/2, Y-Height/2, Width, Height, 0xFF000000, true);
    g_Screen->GetBitmap()->DrawRectangle(X-Width/2, Y-Height/2, Width, Height, 0xFFFFFFFF, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSelection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears selection info

void GUIEditorApp::ClearSelection(void)
{
    m_SelectionInfo.m_GrabbedControl = false;
    m_SelectionInfo.m_GrabbedHandle = false;
    m_SelectionInfo.m_TriggerGrab = false;

    m_SelectionInfo.m_Control = NULL;
    m_SelectionInfo.m_HandleIndex = 0;

    m_SelectionInfo.m_GrabX = 0;
    m_SelectionInfo.m_GrabY = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseInsideBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if the mouse point is inside a box

bool GUIEditorApp::MouseInsideBox(int MouseX, int MouseY, int X, int Y, int Width, int Height)
{
    if (MouseX >= X && MouseX <= X+Width && MouseY >= Y && MouseY <= Y+Height)
        return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateHandleResize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates new position/size of a control given a handle movement
// Arguments:       Control, Mouse, Handle, Pos, Size

void GUIEditorApp::CalculateHandleResize(int MouseX, int MouseY, int *X, int *Y, int *Width, int *Height)
{
    int CtrlX, CtrlY, CtrlWidth, CtrlHeight;
    m_SelectionInfo.m_Control->GetControlRect(&CtrlX, &CtrlY, &CtrlWidth, &CtrlHeight);
    GUIControl *Parent = m_SelectionInfo.m_Control->GetParent();

    int MinSize = 5;

    int ParentX, ParentY, ParentWidth, ParentHeight;
    Parent->GetControlRect(&ParentX, &ParentY, &ParentWidth, &ParentHeight);

    // Left Move/Resize
    if (m_SelectionInfo.m_HandleIndex == 0 || m_SelectionInfo.m_HandleIndex == 3 || m_SelectionInfo.m_HandleIndex == 6)
    {
        int Diff = MouseX - m_SelectionInfo.m_GrabX;
        if (CtrlX + Diff < ParentX)
            Diff = ParentX - CtrlX;
        if (CtrlWidth - Diff < MinSize)
            Diff = CtrlWidth - MinSize;

        Diff = ProcessSnapCoord(Diff);

        CtrlX += Diff;
        CtrlWidth -= Diff;
    }
    // Top Move/Resize
    if (m_SelectionInfo.m_HandleIndex == 0 || m_SelectionInfo.m_HandleIndex == 1 || m_SelectionInfo.m_HandleIndex == 2)
    {
        int Diff = MouseY - m_SelectionInfo.m_GrabY;
        if (CtrlY + Diff < ParentY)
            Diff = ParentY - CtrlY;
        if (CtrlHeight - Diff < MinSize)
            Diff = CtrlHeight - MinSize;

        Diff = ProcessSnapCoord(Diff);

        CtrlY += Diff;
        CtrlHeight -= Diff;
    }
    // Right Resize
    if (m_SelectionInfo.m_HandleIndex == 2 || m_SelectionInfo.m_HandleIndex == 5 || m_SelectionInfo.m_HandleIndex == 8)
    {
        int Diff = MouseX - m_SelectionInfo.m_GrabX;
        if (CtrlX + CtrlWidth + Diff > ParentX + ParentWidth)
            Diff = (ParentX + ParentWidth) - (CtrlX + CtrlWidth);

        Diff = ProcessSnapCoord(Diff);

        CtrlWidth += Diff;
    }
    // Bottom Resize
    if (m_SelectionInfo.m_HandleIndex == 6 || m_SelectionInfo.m_HandleIndex == 7 || m_SelectionInfo.m_HandleIndex == 8)
    {
        int Diff = MouseY - m_SelectionInfo.m_GrabY;
        if (CtrlY + CtrlHeight + Diff > ParentY + ParentHeight)
            Diff = (ParentY + ParentHeight) - (CtrlY + CtrlHeight);

        Diff = ProcessSnapCoord(Diff);

        CtrlHeight += Diff;
    }

    CtrlWidth = MAX(CtrlWidth, MinSize);
    CtrlHeight = MAX(CtrlHeight, MinSize);

    *X = CtrlX;
    *Y = CtrlY;
    *Width = CtrlWidth;
    *Height = CtrlHeight;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GenerateControlName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Generates a new control name based on the type

std::string GUIEditorApp::GenerateControlName(std::string strControlType)
{
    char buf[64];

    // 10,000 should be enough
    for (int i=1; i<10000; i++)
    {
        string Name = strControlType;

        // Use a lower case version of the string
        for (int s=0; s<Name.size(); s++)
            Name.at(s) = tolower(Name.at(s));

        Name.append(itoa(i, buf, 10));

        // Check if this name exists
        vector<GUIControl *> *ControlList = m_pControlManager->GetControlList();
        vector<GUIControl *>::iterator it;

        bool Found = false;

        for (it = ControlList->begin(); it != ControlList->end(); it++)
        {
            GUIControl *C = *it;
            if (C->GetName().compare(Name) == 0)
            {
                Found = true;
                break;
            }
        }

        if (!Found)
            return Name;
    }

    // Error: Tried all 10,000 numbers
    return strControlType;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ProcessSnapCoord
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the nearest snap position (if snap is on)

int GUIEditorApp::ProcessSnapCoord(int Position)
{
    if (m_bSnapGrid)
    {
        float Fraction = (float)Position / (float)m_nGridSize;
        int Value = (int)Fraction;
        Fraction -= Value;
        if (Fraction >= 0.5f)
            Fraction = 1;
        else
            Fraction = 0;

        Position = (Value + Fraction) * m_nGridSize;
    }

    return Position;
}