//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIManager.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the GUIManager class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "Timer.h"


using namespace RTE;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIManager object in system
//                  memory.

GUIManager::GUIManager(GUIInput *input)
{
    m_Input = input;
    m_MouseEnabled = true;
    m_UseValidation = false;

    Clear();

    // Maximum time allowed between two clicks for a double click
    // In milliseconds
//    m_DoubleClickTime = GetDoubleClickTime();                // Use windows' system value
    m_DoubleClickTime = 500;
//    m_DoubleClickSize = GetSystemMetrics(SM_CXDOUBLECLK)/2;    // Use windows' system value
    m_DoubleClickSize = 2;
    m_DoubleClickButtons = GUIPanel::MOUSE_NONE;

    m_pTimer = new Timer();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      GUIManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GUIManager object.
// Arguments:       None.

GUIManager::~GUIManager()
{
    delete m_pTimer;
    m_pTimer = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the manager.

void GUIManager::Clear(void)
{
    m_PanelList.clear();
    m_CapturedPanel = 0;
    m_MouseOverPanel = 0;
    m_FocusPanel = 0;
    m_MouseEnabled = true;
    m_OldMouseX = m_OldMouseY = 0;
    m_UniqueIDCount = 0;

    m_HoverTrack = false;
    m_HoverPanel = 0;

    // Double click times
    m_LastMouseDown[0] = -99999.0f;
    m_LastMouseDown[1] = -99999.0f;
    m_LastMouseDown[2] = -99999.0f;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a panel to the list

void GUIManager::AddPanel(GUIPanel *panel)
{
    if (panel) {
        int Z = 0;
        
        // Get the last panel in the list
        if (m_PanelList.size() > 0) {
            GUIPanel *p = (GUIPanel *)m_PanelList.at(m_PanelList.size()-1);
            Z = p->GetZPos()+1;
        }

        // Setup the panel
        panel->Setup(this, Z);

        // Add the panel to the list
        m_PanelList.push_back(panel);        
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the GUI.

void GUIManager::Update(bool ignoreKeyboardEvents)
{
    m_Input->Update();

    /*
     * Mouse Events
     */
    int i;
    int MouseX = 0, MouseY = 0;
    int DeltaX, DeltaY;    
    int MouseButtons[3], MouseStates[3];
	int MouseWheelChange = 0;
    int Released = GUIPanel::MOUSE_NONE;
    int Pushed = GUIPanel::MOUSE_NONE;
    int Buttons = GUIPanel::MOUSE_NONE;
    int Repeated = GUIPanel::MOUSE_NONE;
    int Modifier = GUIInput::ModNone;
    int Mod = GUIPanel::MODI_NONE;

    float CurTime = m_pTimer->GetElapsedRealTimeMS();

    // Build the modifier state
    Modifier = m_Input->GetModifier();
    if (Modifier & GUIInput::ModShift)
        Mod |= GUIPanel::MODI_SHIFT;
    if (Modifier & GUIInput::ModCtrl)
        Mod |= GUIPanel::MODI_CTRL;
    if (Modifier & GUIInput::ModAlt)
        Mod |= GUIPanel::MODI_ALT;
	if (Modifier & GUIInput::ModCommand)
		Mod |= GUIPanel::MODI_COMMAND;

    // Get the mouse data
    if (m_MouseEnabled)
    {
        m_Input->GetMousePosition(&MouseX, &MouseY);
        m_Input->GetMouseButtons(MouseButtons, MouseStates);
		MouseWheelChange = m_Input->GetMouseWheelChange();

        // Calculate mouse movement
        DeltaX = MouseX - m_OldMouseX;
        DeltaY = MouseY - m_OldMouseY;
        m_OldMouseX = MouseX;
        m_OldMouseY = MouseY;

        // Panels that have captured the mouse get the events over anything else
        // Regardless where the mouse currently is
        GUIPanel *CurPanel = m_CapturedPanel;

        // Find the lowest panel in the tree that the mouse is over
        if (!CurPanel)
            CurPanel = FindTopPanel(MouseX, MouseY);
        
        // Build the states
        for(i=0; i<3; i++) {
            if (MouseButtons[i] == GUIInput::Released)
                Released |= 1<<i;
            if (MouseButtons[i] == GUIInput::Pushed)
                Pushed |= 1<<i;
            if (MouseButtons[i] == GUIInput::Repeat)
                Repeated |= 1<<i;
            if (MouseStates[i] == GUIInput::Down)
                Buttons |= 1<<i;
        }

        // Mouse Up
        if (Released != GUIPanel::MOUSE_NONE) {
            if (CurPanel)
                CurPanel->OnMouseUp(MouseX, MouseY, Released, Mod);
        }

        // Double click (on the mouse up)
        if (Released != GUIPanel::MOUSE_NONE && m_DoubleClickButtons != GUIPanel::MOUSE_NONE) {
            if (CurPanel)
                CurPanel->OnDoubleClick(MouseX, MouseY, m_DoubleClickButtons, Mod);
            m_LastMouseDown[0] = m_LastMouseDown[1] = m_LastMouseDown[2] = -99999.0f;
        }

        // Mouse Down
        if (Pushed != GUIPanel::MOUSE_NONE) {
            // Double click settings
            m_DoubleClickButtons = GUIPanel::MOUSE_NONE;

            // Check for a double click
            for(i=0; i<3; i++) {
                if (Pushed & (1<<i)) {
                    if (CurTime - m_LastMouseDown[i] < (float)(m_DoubleClickTime) && MouseInRect(&m_DoubleClickRect, MouseX, MouseY)) {
                        
                        m_DoubleClickButtons |= (1<<i);

                    } else {
                        // Setup the first click
                        m_DoubleClickButtons = GUIPanel::MOUSE_NONE;
                        m_LastMouseDown[i] = CurTime;
                    }
                }
            }

            // Setup the double click rectangle
            if (m_DoubleClickButtons == GUIPanel::MOUSE_NONE) {
                SetRect(&m_DoubleClickRect, MouseX-m_DoubleClickSize,
                                            MouseY-m_DoubleClickSize,
                                            MouseX+m_DoubleClickSize,
                                            MouseY+m_DoubleClickSize);
            }

            // OnMouseDown event
            if (CurPanel)
                CurPanel->OnMouseDown(MouseX, MouseY, Pushed, Mod);
        }

        // Mouse move
        if (DeltaX != 0 || DeltaY != 0) {
            if (CurPanel) {            
                CurPanel->OnMouseMove(MouseX, MouseY, Buttons, Mod);
            }
        }

        // Mouse Hover
        if (m_HoverTrack && m_HoverTime < CurTime) {
            // Disable it (panel will have to re-enable it if it wants to continue)
            m_HoverTrack = false;

            if (m_HoverPanel/* && CurPanel*/) {
                if (m_HoverPanel->PointInside(MouseX, MouseY)/*GetPanelID() == CurPanel->GetPanelID()*/) {

                    // call the OnMouseHover event
                    m_HoverPanel->OnMouseHover(MouseX, MouseY, Buttons, Mod);
                }
            }
        }


        // Mouse enter & leave
        bool Enter = false;
        bool Leave = false;
        if (!m_MouseOverPanel && CurPanel)
            Enter = true;
        if (!CurPanel && m_MouseOverPanel)
            Leave = true;
        if (m_MouseOverPanel && CurPanel) {
            if (m_MouseOverPanel->GetPanelID() != CurPanel->GetPanelID()) {
                Enter = true;
                Leave = true;
            }
        }

        // OnMouseEnter
        if (Enter) {
            if (CurPanel)
                CurPanel->OnMouseEnter(MouseX, MouseY, Buttons, Mod);
        }

        // OnMouseLeave
        if (Leave) {
            if (m_MouseOverPanel)
                m_MouseOverPanel->OnMouseLeave(MouseX, MouseY, Buttons, Mod);
        }

		if (MouseWheelChange) {
			if (CurPanel) {
				CurPanel->OnMouseWheelChange(MouseX, MouseY, Mod, MouseWheelChange);
			}
		}

        m_MouseOverPanel = CurPanel;
    }

	if (!ignoreKeyboardEvents) {
		/*
		 * Keyboard Events
		 */
		uint8_t KeyboardBuffer[256];
		m_Input->GetKeyboard(KeyboardBuffer);

		// If we don't have a panel with focus, just ignore keyboard events
		if (!m_FocusPanel)
			return;
		// If the panel is not enabled, don't send it key events
		if (!m_FocusPanel->IsEnabled())
			return;


		for(i=1; i<256; i++) {
			switch(KeyboardBuffer[i]) {
				// KeyDown & KeyPress
				case GUIInput::Pushed:
					m_FocusPanel->OnKeyDown(i, Mod);
					m_FocusPanel->OnKeyPress(i, Mod);
					break;

				// KeyUp
				case GUIInput::Released:
					m_FocusPanel->OnKeyUp(i, Mod);
					break;

				// KeyPress
				case GUIInput::Repeat:
					m_FocusPanel->OnKeyPress(i, Mod);
					break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draw all the panels

void GUIManager::Draw(GUIScreen *Screen)
{
    // Go through drawing panels that are invalid
    std::vector<GUIPanel *>::iterator it;

    for(it = m_PanelList.begin(); it != m_PanelList.end(); it++) {
        GUIPanel *p = *it;    

        // Draw the panel
        if ((!p->IsValid() || !m_UseValidation) && p->_GetVisible())
            p->Draw(Screen);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up capturing a mouse for a panel.

void GUIManager::CaptureMouse(GUIPanel *Panel)
{
    RTEAssert(Panel, "No panel!");

    // Release any old capture
    ReleaseMouse();

    // Setup the new capture
    m_CapturedPanel = Panel;
    m_CapturedPanel->SetCaptureState(true);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReleaseMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Releases a mouse capture.

void GUIManager::ReleaseMouse(void)
{
    if (m_CapturedPanel)
        m_CapturedPanel->SetCaptureState(false);

    m_CapturedPanel = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FindBottomPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through the panel list and selects the bottommost
//                  ('first', render wise) panel on a specific point.

GUIPanel *GUIManager::FindBottomPanel(int X, int Y)
{
    std::vector<GUIPanel *>::iterator it;

    for(it = m_PanelList.begin(); it != m_PanelList.end(); it++) {
        GUIPanel *P = *it;
        if (P) {
            GUIPanel *CurPanel = P->BottomPanelUnderPoint(X, Y);
            if (CurPanel)
                return CurPanel;
        }
    }

    // No panel found
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FindTopPanel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through the panel list and selects the topmost ('last', render
//                  wise) panel on a specific point.

GUIPanel *GUIManager::FindTopPanel(int X, int Y)
{
    std::vector<GUIPanel *>::reverse_iterator it;

    for(it = m_PanelList.rbegin(); it != m_PanelList.rend(); it++) {
        GUIPanel *P = *it;
        if (P) {
            GUIPanel *CurPanel = P->TopPanelUnderPoint(X, Y);
            if (CurPanel)
                return CurPanel;
        }
    }

    // No panel found
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPanelID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a unique ID for a panel.

int GUIManager::GetPanelID(void)
{
    return m_UniqueIDCount++;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateTimer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates the timer.

bool GUIManager::CreateTimer(void)
{
    LARGE_INTEGER        TicksPerSec;

    // Get the ticks per second
    QueryPerformanceFrequency(&TicksPerSec);

    m_TicksPerSecond = TicksPerSec.QuadPart;
    m_LastElapsedTime = 0;

    // Reset the timer
    LARGE_INTEGER        Time;
    QueryPerformanceCounter(&Time);
    m_TimeStarted = Time.QuadPart;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current time since the timer was created.

float GUIManager::m_pTimer->GetElapsedSimTimeMS(void)
{
    LARGE_INTEGER    Time;

    QueryPerformanceCounter(&Time);

    double Elapsed = (double)(Time.QuadPart - m_TimeStarted) / (double)m_TicksPerSecond;

    return (float)Elapsed;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseInRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if the mouse point is inside a rectangle.

bool GUIManager::MouseInRect(GUIRect *Rect, int X, int Y)
{
    if (!Rect)
        return false;

    if (X >= Rect->left && X <= Rect->right &&
       Y >= Rect->top && Y <= Rect->bottom)
       return true;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrackMouseHover
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the manager to enable/disable hover tracking of this panel

void GUIManager::TrackMouseHover(GUIPanel *Pan, bool Enabled, int Delay)
{
    RTEAssert(Pan, "No Panel!");
    m_HoverTrack = Enabled;
    m_HoverPanel = Pan;
    if (m_HoverTrack)
        m_HoverTime = m_pTimer->GetElapsedRealTimeMS() + ((float)Delay/1000.0f);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFocus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Give focus to a panel.

void GUIManager::SetFocus(GUIPanel *Pan)
{
    // Send the LoseFocus event to the old panel (if there is one)
    if (m_FocusPanel)
        m_FocusPanel->OnLoseFocus();

    m_FocusPanel = Pan;

    // Send the GainFocus event to the new panel
    if (m_FocusPanel)
        m_FocusPanel->OnGainFocus();
}