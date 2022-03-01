#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIInput::m_OverrideInput = false;

int GUIInput::m_NetworkMouseButtonsEvents[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };
int GUIInput::m_NetworkMouseButtonsStates[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };
int GUIInput::m_PrevNetworkMouseButtonsStates[4][3] = { { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 }, { -1, -1, -1 } };

int GUIInput::m_NetworkMouseX[4] = { 0, 0, 0, 0 };
int GUIInput::m_NetworkMouseY[4] = { 0, 0, 0, 0 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIInput::GUIInput(int whichPlayer, bool keyJoyMouseCursor) {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::Destroy() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::GetKeyboard(unsigned char *Buffer) const {
	if (Buffer) { memcpy(Buffer, m_KeyboardBuffer, sizeof(unsigned char) * KEYBOARD_BUFFER_SIZE); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char GUIInput::GetAsciiState(unsigned char ascii) const {
	return m_KeyboardBuffer[ascii];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char GUIInput::GetScanCodeState(unsigned char scancode) const {
	return m_ScanCodeState[scancode];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::GetMouseButtons(int *Buttons, int *States) const {
	if (!m_OverrideInput) {
		if (Buttons) { memcpy(Buttons, m_MouseButtonsEvents, sizeof(int) * 3); }
		if (States) { memcpy(States, m_MouseButtonsStates, sizeof(int) * 3); }
	} else {
		for (int i = 0; i < 3; i++) {
			Buttons[i] = -1;
			States[i] = -1;
		}
		if (m_Player >= 0 && m_Player < 4) {
			for (int b = 0; b < 3; b++) {
				Buttons[b] = m_NetworkMouseButtonsEvents[m_Player][b];
				States[b] = m_NetworkMouseButtonsStates[m_Player][b];
			}
		} else {
			for (int b = 0; b < 3; b++) {
				Buttons[b] = m_NetworkMouseButtonsEvents[0][b];
				States[b] = m_NetworkMouseButtonsStates[0][b];
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::SetNetworkMouseButton(int whichPlayer, int state1, int state2, int state3) {
	if (whichPlayer >= 0 && whichPlayer < 4) {
		m_OverrideInput = true;

		m_PrevNetworkMouseButtonsStates[whichPlayer][0] = m_NetworkMouseButtonsStates[whichPlayer][0];
		m_PrevNetworkMouseButtonsStates[whichPlayer][1] = m_NetworkMouseButtonsStates[whichPlayer][1];
		m_PrevNetworkMouseButtonsStates[whichPlayer][2] = m_NetworkMouseButtonsStates[whichPlayer][2];

		m_NetworkMouseButtonsStates[whichPlayer][0] = state1;
		m_NetworkMouseButtonsStates[whichPlayer][1] = state2;
		m_NetworkMouseButtonsStates[whichPlayer][2] = state3;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::GetMousePosition(int *X, int *Y) const {
	if (m_OverrideInput) {
		if (m_Player >= 0 && m_Player < 4) {
			if (X) { *X = (m_NetworkMouseX[m_Player] + m_MouseOffsetX); }
			if (Y) { *Y = (m_NetworkMouseY[m_Player] + m_MouseOffsetY); }
		} else {
			if (X) { *X = (m_NetworkMouseX[0] + m_MouseOffsetX); }
			if (Y) { *Y = (m_NetworkMouseY[0] + m_MouseOffsetY); }
		}
	} else {
		if (X) { *X = (m_MouseX + m_MouseOffsetX); }
		if (Y) { *Y = (m_MouseY + m_MouseOffsetY); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::SetNetworkMouseMovement(int whichPlayer, int x, int y) {
	if (whichPlayer >= 0 && whichPlayer < 4) {
		m_OverrideInput = true;
		m_NetworkMouseX[whichPlayer] += x;
		m_NetworkMouseY[whichPlayer] += y;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIInput::Update() {
	// Do nothing
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIInput::GetModifier() const {
	return m_Modifier;
}
