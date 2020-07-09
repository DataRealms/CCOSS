//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIInput.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the GUIInput class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"

using namespace RTE;


bool GUIInput::m_OverrideInput = false;

int GUIInput::m_NetworkMouseButtonsEvents[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }};
int GUIInput::m_NetworkMouseButtonsStates[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };
int GUIInput::m_PrevNetworkMouseButtonsStates[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };

int GUIInput::m_NetworkMouseX[4] = { 0, 0, 0, 0 };
int GUIInput::m_NetworkMouseY[4] = { 0, 0, 0, 0 };

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIInput object in system
//                  memory.

GUIInput::GUIInput(int whichPlayer, bool keyJoyMouseCursor)
{
    // Clear all the states
    memset(m_KeyboardBuffer, 0, sizeof(unsigned char) * KEYBOARD_BUFFER_SIZE);
	memset(m_ScanCodeState, 0, sizeof(unsigned char) * KEYBOARD_BUFFER_SIZE);
    memset(m_MouseButtonsEvents, 0, sizeof(int) * 3);
    memset(m_MouseButtonsStates, 0, sizeof(int) * 3);

	//memset(m_NetworkMouseButtonsEvents, -1, sizeof(int) * 3);
	//memset(m_NetworkMouseButtonsStates, -1, sizeof(int) * 3);


    m_MouseX = 0;
    m_MouseY = 0;
	m_LastFrameMouseX = 0;
	m_LastFrameMouseY = 0;

    m_MouseOffsetX = 0;
    m_MouseOffsetY = 0;
    m_Modifier = ModNone;

    m_KeyJoyMouseCursor = keyJoyMouseCursor;

	m_Player = whichPlayer;

	m_MouseWheelChange = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroy the screen

void GUIInput::Destroy(void)
{
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKeyboard
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copies the keyboard buffer into an array

void GUIInput::GetKeyboard(unsigned char *Buffer)
{
    if (Buffer)
        memcpy(Buffer, m_KeyboardBuffer, sizeof(unsigned char) * KEYBOARD_BUFFER_SIZE);
}


unsigned char GUIInput::GetAsciiState(unsigned char ascii)
{
	return m_KeyboardBuffer[ascii];
}

unsigned char GUIInput::GetScanCodeState(unsigned char scancode)
{
	return m_ScanCodeState[scancode];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseButtons
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copies the mouse button states into an array

void GUIInput::GetMouseButtons(int *Buttons, int *States)
{
	if (!m_OverrideInput)
	{
		if (Buttons)
			memcpy(Buttons, m_MouseButtonsEvents, sizeof(int) * 3);
		if (States)
			memcpy(States, m_MouseButtonsStates, sizeof(int) * 3);
	}
	else 
	{
		for (int i = 0; i < 3; i++)
		{
			Buttons[i] = -1;
			States[i] = -1;
		}

		if (m_Player >= 0 && m_Player < 4)
		{
			for (int b = 0; b < 3; b++)
			{
				//if (m_NetworkMouseButtonsEvents[m_Player][b] > Buttons[b])
					Buttons[b] = m_NetworkMouseButtonsEvents[m_Player][b];

				//if (m_NetworkMouseButtonsStates[m_Player][b] > States[b])
					States[b] = m_NetworkMouseButtonsStates[m_Player][b];

				// After the events are read immediately move to next event to avoid reading the same press and release event twice or more
				//m_NetworkMouseButtonsEvents[m_Player][b] = m_NetworkMouseButtonsStates[m_Player][b] == Down ? 
				//	(m_NetworkMouseButtonsStates[m_Player][b] == Up ? Pushed : Repeat) : 
				//	(m_NetworkMouseButtonsStates[m_Player][b] == Down ? Released : None);
			}
		}
		else 
		{
			//for (int p = 0; p < 4; p++)
			{
				for (int b = 0; b < 3; b++)
				{
					//if (m_NetworkMouseButtonsEvents[0][b] > Buttons[b])
						Buttons[b] = m_NetworkMouseButtonsEvents[0][b];

					//if (m_NetworkMouseButtonsStates[0][b] > States[b])
						States[b] = m_NetworkMouseButtonsStates[0][b];

					// After the events are read immediately move to next to avoid reading the same press and release event twice or more
					//m_NetworkMouseButtonsEvents[0][b] = m_NetworkMouseButtonsStates[0][b] == Down ?
					//	(m_NetworkMouseButtonsStates[0][b] == Up ? Pushed : Repeat) :
					//	(m_NetworkMouseButtonsStates[0][b] == Down ? Released : None);
				}
			}
		}
	}
}


void GUIInput::SetNetworkMouseButton(int whichPlayer, int state1, int state2, int state3)
{
	if (whichPlayer >= 0 && whichPlayer < 4)
	{
		m_OverrideInput = true;

		m_PrevNetworkMouseButtonsStates[whichPlayer][0] = m_NetworkMouseButtonsStates[whichPlayer][0];
		m_PrevNetworkMouseButtonsStates[whichPlayer][1] = m_NetworkMouseButtonsStates[whichPlayer][1];
		m_PrevNetworkMouseButtonsStates[whichPlayer][2] = m_NetworkMouseButtonsStates[whichPlayer][2];

		m_NetworkMouseButtonsStates[whichPlayer][0] = state1;
		m_NetworkMouseButtonsStates[whichPlayer][1] = state2;
		m_NetworkMouseButtonsStates[whichPlayer][2] = state3;
	}
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMousePosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mouse position

void GUIInput::GetMousePosition(int *X, int *Y)
{
	if (m_OverrideInput)
	{
		if (m_Player >= 0 && m_Player < 4)
		{
			if (X)
				*X = (m_NetworkMouseX[m_Player] + m_MouseOffsetX);
			if (Y)
				*Y = (m_NetworkMouseY[m_Player] + m_MouseOffsetY);
		}
		else 
		{
				if (X)
					*X = (m_NetworkMouseX[0] + m_MouseOffsetX);
				if (Y)
					*Y = (m_NetworkMouseY[0] + m_MouseOffsetY);
		}
	}
	else 
	{
		if (X)
			*X = (m_MouseX + m_MouseOffsetX);
		if (Y)
			*Y = (m_MouseY + m_MouseOffsetY);
	}
}

void GUIInput::SetNetworkMouseMovement(int whichPlayer, int x, int y)
{
	if (whichPlayer >= 0 && whichPlayer < 4)
	{
		m_OverrideInput = true;
		m_NetworkMouseX[whichPlayer] += x;
		m_NetworkMouseY[whichPlayer] += y;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Input.

void GUIInput::Update(void)
{
    // Do nothing
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModifier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the key modifiers.

int GUIInput::GetModifier(void)
{
    return m_Modifier;
}
