//////////////////////////////////////////////////////////////////////////////////////////
// File:            AllegroInput.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation for the AllegroInput Class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

#include "GUI.h"
#include "AllegroInput.h"
#include "Timer.h"

#ifndef GUI_STANDALONE
#include "FrameMan.h"
#include "UInputMan.h"
#endif // GUI_STANDALONE

using namespace RTE;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AllegroInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AllegroInput object in system
//                  memory.

AllegroInput::AllegroInput(int whichPlayer, bool keyJoyMouseCursor):
    GUIInput(whichPlayer, keyJoyMouseCursor)
{
    install_keyboard();
    // Hack to not have the keyboard lose focus permanently when window is started without focus
//    win_grab_input();
    install_mouse();

#ifndef GUI_STANDALONE
    // Set the speed of the mouse
    float mouseDenominator = g_FrameMan.ResolutionMultiplier();
    // If Nx fullscreen, adjust the mouse speed accordingly
    if (mouseDenominator > 1)
        set_mouse_speed(1, 1);
    else
        set_mouse_speed(2, 2);
    set_mouse_range(0, 0, (g_FrameMan.GetResX() * mouseDenominator) - 3, (g_FrameMan.GetResY() * mouseDenominator) - 3);
#endif
    //set_mouse_speed(1, 1);
    //set_mouse_range(0, 0, (g_FrameMan.GetResX() * mouseDenominator) - 3, (g_FrameMan.GetResY() * mouseDenominator) - 3);

    memset(m_ModStates, 0, sizeof(int) * 6);
    memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * KEYBOARD_BUFFER_SIZE);
	memset(m_ScanCodeState, 0, sizeof(uint8_t) * KEYBOARD_BUFFER_SIZE);

    m_KeyDelay = 0.300f;        // 0.3 second delay
    m_KeyRepeat = 1.0f/30.0f;   // 30 repititions per second

    // Setup the keyboard timing
    for(int i=0; i<256; i++)
        m_KeyboardTimes[i] = -1;

    // Create the timers
    m_pTimer = new Timer();
    m_pCursorAccelTimer = new Timer();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AllegroInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AllegroInput object.
// Arguments:       None.

AllegroInput::~AllegroInput()
{
    delete m_pTimer; m_pTimer = 0;
    delete m_pCursorAccelTimer; m_pCursorAccelTimer = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:         Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Input.

void AllegroInput::Update(void)
{
	/*
		* Important!
		* We make the assumption that the CDXInput class has already been updated in the higher level code.
		* Such as the top of the main processing loop.
		* If this isn't the case, just uncomment the line below
		*/
		//m_Input->Update();


		/*
			* Keyboard
			*/

			// Clear the keyboard buffer, we need it to check for changes
	memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * KEYBOARD_BUFFER_SIZE);
	memset(m_ScanCodeState, 0, sizeof(uint8_t) * KEYBOARD_BUFFER_SIZE);
	while (keypressed())
	{
		uint32_t rawkey = readkey();
		uint8_t ascii = rawkey & 0xff;
		uint8_t scancode = (rawkey >> 8);

		m_ScanCodeState[scancode] = Pushed;

		m_KeyboardBuffer[ascii] = Pushed;
	}

	float elapsedS = m_pTimer->GetElapsedRealTimeS();
	m_pTimer->Reset();

	// Fill the keyb buffer with allegro buffer
//    for (int scancode = 0; scancode < KEY_ESC; ++scancode)
//        ConvertKeyEvent(scancode, scancode_to_ascii(scancode), elapsedS);

	// Special keys
	ConvertKeyEvent(KEY_SPACE, ' ', elapsedS);
	ConvertKeyEvent(KEY_BACKSPACE, Key_Backspace, elapsedS);
	ConvertKeyEvent(KEY_TAB, Key_Tab, elapsedS);
	ConvertKeyEvent(KEY_ENTER, Key_Enter, elapsedS);
	ConvertKeyEvent(KEY_ESC, Key_Escape, elapsedS);
	ConvertKeyEvent(KEY_LEFT, Key_LeftArrow, elapsedS);
	ConvertKeyEvent(KEY_RIGHT, Key_RightArrow, elapsedS);
	ConvertKeyEvent(KEY_UP, Key_UpArrow, elapsedS);
	ConvertKeyEvent(KEY_DOWN, Key_DownArrow, elapsedS);
	ConvertKeyEvent(KEY_INSERT, Key_Insert, elapsedS);
	ConvertKeyEvent(KEY_DEL, Key_Delete, elapsedS);
	ConvertKeyEvent(KEY_HOME, Key_Home, elapsedS);
	ConvertKeyEvent(KEY_END, Key_End, elapsedS);
	ConvertKeyEvent(KEY_PGUP, Key_PageUp, elapsedS);
	ConvertKeyEvent(KEY_PGDN, Key_PageDown, elapsedS);

	// Modifier states
	bool Shift = false;
	bool Alt = false;
	bool Ctrl = false;
	bool Command = false;

	// Modifiers
	m_Modifier = ModNone;
	if (Shift = key_shifts & KB_SHIFT_FLAG)
		m_Modifier |= ModShift;
	if (Alt = key_shifts & KB_ALT_FLAG)
		m_Modifier |= ModAlt;
	if (Ctrl = key_shifts & KB_CTRL_FLAG)
		m_Modifier |= ModCtrl;
	if (Command = key_shifts & KB_COMMAND_FLAG)
		m_Modifier |= ModCommand;

	/*
	 * Mouse
	 */

	 // Update mouse, if necessary
	if (!m_OverrideInput)
	{
		if (mouse_needs_poll())
			poll_mouse();
	}
	else {
		mouse_x = m_LastFrameMouseX;
		mouse_y = m_LastFrameMouseY;

		if (m_Player >= 0 && m_Player < 4)
		{
			if (m_NetworkMouseX[m_Player] != 0)
			{
#ifndef GUI_STANDALONE
				if (m_NetworkMouseX[m_Player] < 0) m_NetworkMouseX[m_Player] = 1;
				if (m_NetworkMouseX[m_Player] >= g_FrameMan.GetPlayerFrameBufferWidth(m_Player)) m_NetworkMouseX[m_Player] = g_FrameMan.GetPlayerFrameBufferWidth(m_Player) - 2;
#endif
				mouse_x = m_NetworkMouseX[m_Player];
			}

			if (m_NetworkMouseY[m_Player] != 0)
			{
#ifndef GUI_STANDALONE
				if (m_NetworkMouseY[m_Player] < 0) m_NetworkMouseY[m_Player] = 1;
				if (m_NetworkMouseY[m_Player] >= g_FrameMan.GetPlayerFrameBufferHeight(m_Player)) m_NetworkMouseY[m_Player] = g_FrameMan.GetPlayerFrameBufferHeight(m_Player) - 2;
#endif
				mouse_y = m_NetworkMouseY[m_Player];
			}

			//m_NetworkMouseX[m_Player] = mouse_x;
			//m_NetworkMouseY[m_Player] = mouse_y;
		}
		else {
			if (m_NetworkMouseX[0] != 0)
			{
#ifndef GUI_STANDALONE
				if (m_NetworkMouseX[0] < 0) m_NetworkMouseX[0] = 1;
				if (m_NetworkMouseX[0] >= g_FrameMan.GetPlayerFrameBufferWidth(0)) m_NetworkMouseX[0] = g_FrameMan.GetPlayerFrameBufferWidth(0) - 2;
#endif
				mouse_x = m_NetworkMouseX[0];
			}

			if (m_NetworkMouseY[0] != 0)
			{
#ifndef GUI_STANDALONE
				if (m_NetworkMouseY[0] < 0) m_NetworkMouseY[0] = 1;
				if (m_NetworkMouseY[0] >= g_FrameMan.GetPlayerFrameBufferHeight(0)) m_NetworkMouseY[0] = g_FrameMan.GetPlayerFrameBufferHeight(0) - 2;
#endif
				mouse_y = m_NetworkMouseY[0];
			}
		}

	}

	m_LastFrameMouseX = mouse_x;
	m_LastFrameMouseY = mouse_y;

/* Obsolete
    // Adjust the mouse states
    for(i=0; i<3; i++) {
        if (m_MouseButtonsEvents[i] == Released)
            m_MouseButtonsStates[i] = Up;
        if (m_MouseButtonsEvents[i] == Pushed)
            m_MouseButtonsStates[i] = Down;
    }
*/
    // Update the mouse button events
	if (m_OverrideInput)
	{
		int player = m_Player;
		if (player < 0 || player >= 4)
			player = 0;

		m_NetworkMouseButtonsEvents[player][0] = m_NetworkMouseButtonsStates[player][0] == Down ? (m_PrevNetworkMouseButtonsStates[player][0] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][0] == Down ? Released : None);
		m_NetworkMouseButtonsEvents[player][1] = m_NetworkMouseButtonsStates[player][1] == Down ? (m_PrevNetworkMouseButtonsStates[player][1] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][1] == Down ? Released : None);
		m_NetworkMouseButtonsEvents[player][2] = m_NetworkMouseButtonsStates[player][2] == Down ? (m_PrevNetworkMouseButtonsStates[player][2] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][2] == Down ? Released : None);

		m_PrevNetworkMouseButtonsStates[player][0] = m_NetworkMouseButtonsEvents[player][0];
		m_PrevNetworkMouseButtonsStates[player][1] = m_NetworkMouseButtonsEvents[player][1];
		m_PrevNetworkMouseButtonsStates[player][2] = m_NetworkMouseButtonsEvents[player][2];
	} else {
		if (!m_KeyJoyMouseCursor) {
			if (mouse_b & AMBLEFT) {
				m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Up) ? Pushed : Repeat;
				m_MouseButtonsStates[0] = Down;
			} else {
				m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Down) ? Released : None;
				m_MouseButtonsStates[0] = Up;
			}
		}

		if (mouse_b & AMBMIDDLE) {
			m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Up) ? Pushed : Repeat;
			m_MouseButtonsStates[1] = Down;
		} else {
			m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Down) ? Released : None;
			m_MouseButtonsStates[1] = Up;
		}

		if (mouse_b & AMBRIGHT) {
			m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == Up) ? Pushed : Repeat;
			m_MouseButtonsStates[2] = Down;
		} else {
			m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == Down) ? Released : None;
			m_MouseButtonsStates[2] = Up;
		}

		if (m_Player < 0 || m_Player >= 4) {
			for (int i = 0; i < 4; i++) {
				m_MouseWheelChange = g_UInputMan.MouseWheelMovedByPlayer(i);
				if (m_MouseWheelChange) {
					break;
				}
			}
		} else {
			m_MouseWheelChange = g_UInputMan.MouseWheelMovedByPlayer(m_Player);
		}
	}

	/*if (m_Player >= 0 && m_Player < 4)
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_NetworkMouseButtonsEvents[m_Player][i] != -1)
				m_MouseButtonsEvents[i] = m_NetworkMouseButtonsEvents[m_Player][i];
			if (m_NetworkMouseButtonsStates[m_Player][i] != -1)
				m_MouseButtonsStates[i] = m_NetworkMouseButtonsStates[m_Player][i];
		}
	} else {
		for (int p = 0; p < 4; p++)
		{
			for (int i = 0; i < 3; i++)
			{
				if (m_NetworkMouseButtonsEvents[p][i] != -1)
					m_MouseButtonsEvents[i] = m_NetworkMouseButtonsEvents[p][i];
				if (m_NetworkMouseButtonsStates[p][i] != -1)
					m_MouseButtonsStates[i] = m_NetworkMouseButtonsStates[p][i];
			}
		}
	}*/


/* obsolete
    // Update the mouse position
    //long x, y;
    //m_Input->GetMousePos(&x, &y);
    POINT Point;
    GetCursorPos(&Point);
    ScreenToClient(m_hWnd, &Point);
    m_MouseX = Point.x;
    m_MouseY = Point.y;
*/
#ifndef GUI_STANDALONE
    // Get mouse cursor movement denominator based on window size multiplication
    float mouseDenominator = g_FrameMan.ResolutionMultiplier();

    // If joysticks and keyboard can control the mouse cursor too
    if (m_KeyJoyMouseCursor)
    {
        // Direction change
        Vector joyKeyDirectional = g_UInputMan.GetMenuDirectional() * 5;

        // See how much to accelerate the joystick input based on how long the stick has been pushed around
        if (joyKeyDirectional.GetMagnitude() < 0.95)
            m_pCursorAccelTimer->Reset();

        float acceleration = 0.25 + MIN(m_pCursorAccelTimer->GetElapsedRealTimeS(), 0.5) * 20;

        // Manipulate the mouse position with the joysticks or keys
        mouse_x += joyKeyDirectional.m_X * mouseDenominator * elapsedS * 15.0f * acceleration;
        mouse_y += joyKeyDirectional.m_Y * mouseDenominator * elapsedS * 15.0f * acceleration;
        // Prevent mouse from flying out of the screen
        mouse_x = MAX(0, mouse_x);
        mouse_y = MAX(0, mouse_y);
        // Pull in a bit so cursor doesn't completely disappear
        mouse_x = MIN((g_FrameMan.GetResX() * mouseDenominator) - 3, mouse_x);
        mouse_y = MIN((g_FrameMan.GetResY() * mouseDenominator) - 3, mouse_y);

		// Button states/presses, Primary - ACTUALLY make either button work, we don't have use for secondary in menus
		if (g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Down;
			m_MouseButtonsEvents[0] = Repeat;
		}
		if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Down;
			m_MouseButtonsEvents[0] = Pushed;
		} else if (g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Up;
			m_MouseButtonsEvents[0] = Released;
		} else if (m_MouseButtonsEvents[0] == Released) {
			m_MouseButtonsStates[0] = Up;
			m_MouseButtonsEvents[0] = None;
		}
	}

    // Update the mouse position of this GUIInput, based on the allegro mouse vars (which may have been altered by joystick or keyboard input)
//    m_MouseX = mouse_x;
//    m_MouseY = mouse_y;
    m_MouseX = (((float)mouse_x) / mouseDenominator);
    m_MouseY = (((float)mouse_y) / mouseDenominator);

#else // GUI_STANDALONE
    m_MouseX = mouse_x;
    m_MouseY = mouse_y;
#endif // GUI_STANDALONE

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConvertKeyEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts from allegro's key push to that used by this GUI lib, with
//                  timings for repeats taken into consideration.

void AllegroInput::ConvertKeyEvent(int allegroKey, int guilibKey, float elapsedS)
{
    if (key[allegroKey])
    {
        if (m_KeyboardTimes[guilibKey] < 0)
        {
            m_KeyboardBuffer[guilibKey] = Pushed;
            m_KeyboardTimes[guilibKey] = 0;
        }
        else if (m_KeyboardTimes[guilibKey] < m_KeyDelay)
		{
            m_KeyboardBuffer[guilibKey] = None;
		}
        else
        {
            m_KeyboardBuffer[guilibKey] = Repeat;
            m_KeyboardTimes[guilibKey] = 0;//MAX(m_KeyboardTimes[guilibKey] - m_KeyDelay, 0);
        }
        m_KeyboardTimes[guilibKey] += elapsedS;
    }
    else
    {
        if (m_KeyboardTimes[guilibKey] >= 0)
		{
            m_KeyboardBuffer[guilibKey] = Released;
		}
        else
		{
            m_KeyboardBuffer[guilibKey] = None;
		}
        m_KeyboardTimes[guilibKey] = -1;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateTimer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates the timer.

bool AllegroInput::CreateTimer(void)
{
/*
    LARGE_INTEGER      TicksPerSec;

    // Get the ticks per second
    QueryPerformanceFrequency(&TicksPerSec);

    m_TicksPerSecond = TicksPerSec.QuadPart;
    m_LastElapsedTime = 0;

    // Reset the timer
    LARGE_INTEGER      Time;
    QueryPerformanceCounter(&Time);
    m_TimeStarted = Time.QuadPart;
*/
    m_pTimer->Reset();

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:         GetCurrentTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current time since the timer was created.

float AllegroInput::GetTime(void)
{
/*
    LARGE_INTEGER   Time;

    QueryPerformanceCounter(&Time);

    double Elapsed = (double)(Time.QuadPart - m_TimeStarted) / (double)m_TicksPerSecond;
*/

    return (float)m_pTimer->GetElapsedRealTimeS();
}