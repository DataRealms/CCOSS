#include "GUI.h"
#include "AllegroInput.h"
#include "Timer.h"

#ifndef GUI_STANDALONE
#include "FrameMan.h"
#include "UInputMan.h"
#endif

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AllegroInput::AllegroInput(int whichPlayer, bool keyJoyMouseCursor) : GUIInput(whichPlayer, keyJoyMouseCursor) {
		install_keyboard();
		install_mouse();

#ifndef GUI_STANDALONE
		set_mouse_range(0, 0, (g_FrameMan.GetResX() * g_FrameMan.GetResMultiplier()) - 3, (g_FrameMan.GetResY() * g_FrameMan.GetResMultiplier()) - 3);
		AdjustMouseMovementSpeedToGraphicsDriver(g_FrameMan.GetGraphicsDriver());
#endif

		m_KeyTimer = std::make_unique<Timer>();
		m_CursorAccelTimer = std::make_unique<Timer>();

		memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		memset(m_ScanCodeState, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);

		m_KeyHoldDuration.fill(-1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::AdjustMouseMovementSpeedToGraphicsDriver(int graphicsDriver) const {
#ifndef GUI_STANDALONE
		if (graphicsDriver == GFX_AUTODETECT_WINDOWED || graphicsDriver == GFX_DIRECTX_WIN_BORDERLESS) {
			set_mouse_speed(2, 2);
		} else {
			set_mouse_speed(1, 1);
		}
#else
		set_mouse_speed(2, 2);
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::ConvertKeyEvent(int allegroKey, int guilibKey, float elapsedS) {
		if (key[allegroKey]) {
			if (m_KeyHoldDuration.at(guilibKey) < 0) {
				m_KeyboardBuffer[guilibKey] = Pushed;
				m_KeyHoldDuration.at(guilibKey) = 0;
			} else if (m_KeyHoldDuration.at(guilibKey) < m_KeyRepeatDelay) {
				m_KeyboardBuffer[guilibKey] = None;
			} else {
				m_KeyboardBuffer[guilibKey] = Repeat;
				m_KeyHoldDuration.at(guilibKey) = 0;
			}
			m_KeyHoldDuration.at(guilibKey) += elapsedS;
		} else {
			if (m_KeyHoldDuration.at(guilibKey) >= 0) {
				m_KeyboardBuffer[guilibKey] = Released;
			} else {
				m_KeyboardBuffer[guilibKey] = None;
			}
			m_KeyHoldDuration.at(guilibKey) = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::Update() {
		float keyElapsedTime = static_cast<float>(m_KeyTimer->GetElapsedRealTimeS());
		m_KeyTimer->Reset();

		UpdateKeyboardInput(keyElapsedTime);
		UpdateMouseInput();

		// If joysticks and keyboard can control the mouse cursor too
		if (m_KeyJoyMouseCursor) { UpdateKeyJoyMouseInput(keyElapsedTime); }

		// Update the mouse position of this GUIInput, based on the Allegro mouse vars (which may have been altered by joystick or keyboard input)
#ifndef GUI_STANDALONE
		m_MouseX = static_cast<int>(static_cast<float>(mouse_x) / static_cast<float>(g_FrameMan.GetResMultiplier()));
		m_MouseY = static_cast<int>(static_cast<float>(mouse_y) / static_cast<float>(g_FrameMan.GetResMultiplier()));
#else
		m_MouseX = mouse_x;
		m_MouseY = mouse_y;
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::UpdateKeyboardInput(float keyElapsedTime) {
		// Clear the keyboard buffer, we need it to check for changes
		memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		memset(m_ScanCodeState, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		while (keypressed()) {
			uint32_t rawkey = readkey();
			uint8_t ascii = rawkey & 0xff;
			uint8_t scancode = (rawkey >> 8);

			m_ScanCodeState[scancode] = Pushed;
			m_KeyboardBuffer[ascii] = Pushed;
		}

		ConvertKeyEvent(KEY_SPACE, ' ', keyElapsedTime);
		ConvertKeyEvent(KEY_BACKSPACE, Key_Backspace, keyElapsedTime);
		ConvertKeyEvent(KEY_TAB, Key_Tab, keyElapsedTime);
		ConvertKeyEvent(KEY_ENTER, Key_Enter, keyElapsedTime);
		ConvertKeyEvent(KEY_ESC, Key_Escape, keyElapsedTime);
		ConvertKeyEvent(KEY_LEFT, Key_LeftArrow, keyElapsedTime);
		ConvertKeyEvent(KEY_RIGHT, Key_RightArrow, keyElapsedTime);
		ConvertKeyEvent(KEY_UP, Key_UpArrow, keyElapsedTime);
		ConvertKeyEvent(KEY_DOWN, Key_DownArrow, keyElapsedTime);
		ConvertKeyEvent(KEY_INSERT, Key_Insert, keyElapsedTime);
		ConvertKeyEvent(KEY_DEL, Key_Delete, keyElapsedTime);
		ConvertKeyEvent(KEY_HOME, Key_Home, keyElapsedTime);
		ConvertKeyEvent(KEY_END, Key_End, keyElapsedTime);
		ConvertKeyEvent(KEY_PGUP, Key_PageUp, keyElapsedTime);
		ConvertKeyEvent(KEY_PGDN, Key_PageDown, keyElapsedTime);

		m_Modifier = ModNone;

#ifndef GUI_STANDALONE
		if (key_shifts & KB_SHIFT_FLAG) { m_Modifier |= ModShift; }
		if (key_shifts & KB_ALT_FLAG) { m_Modifier |= ModAlt; }
		if (key_shifts & KB_CTRL_FLAG) { m_Modifier |= ModCtrl; }
		if (key_shifts & KB_COMMAND_FLAG) { m_Modifier |= ModCommand; }
#else
		if (key[KEY_LSHIFT] || key[KEY_RSHIFT]) { m_Modifier |= ModShift; }
		if (key[KEY_ALT]) { m_Modifier |= ModAlt; }
		if (key[KEY_LCONTROL] || key[KEY_RCONTROL]) { m_Modifier |= ModCtrl; }
		if (key[KEY_COMMAND]) { m_Modifier |= ModCommand; }
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::UpdateMouseInput() {
		if (!m_OverrideInput) {
			if (mouse_needs_poll()) { poll_mouse(); }

#ifndef GUI_STANDALONE
		} else {
			mouse_x = m_LastFrameMouseX;
			mouse_y = m_LastFrameMouseY;

			if (m_Player >= 0 && m_Player < 4) {
				if (m_NetworkMouseX[m_Player] != 0) {
					if (m_NetworkMouseX[m_Player] < 0) m_NetworkMouseX[m_Player] = 1;
					if (m_NetworkMouseX[m_Player] >= g_FrameMan.GetPlayerFrameBufferWidth(m_Player)) m_NetworkMouseX[m_Player] = g_FrameMan.GetPlayerFrameBufferWidth(m_Player) - 2;
					mouse_x = m_NetworkMouseX[m_Player];
				}
				if (m_NetworkMouseY[m_Player] != 0) {
					if (m_NetworkMouseY[m_Player] < 0) m_NetworkMouseY[m_Player] = 1;
					if (m_NetworkMouseY[m_Player] >= g_FrameMan.GetPlayerFrameBufferHeight(m_Player)) m_NetworkMouseY[m_Player] = g_FrameMan.GetPlayerFrameBufferHeight(m_Player) - 2;
					mouse_y = m_NetworkMouseY[m_Player];
				}
			} else {
				if (m_NetworkMouseX[0] != 0) {
					if (m_NetworkMouseX[0] < 0) m_NetworkMouseX[0] = 1;
					if (m_NetworkMouseX[0] >= g_FrameMan.GetPlayerFrameBufferWidth(0)) m_NetworkMouseX[0] = g_FrameMan.GetPlayerFrameBufferWidth(0) - 2;
					mouse_x = m_NetworkMouseX[0];
				}
				if (m_NetworkMouseY[0] != 0) {
					if (m_NetworkMouseY[0] < 0) m_NetworkMouseY[0] = 1;
					if (m_NetworkMouseY[0] >= g_FrameMan.GetPlayerFrameBufferHeight(0)) m_NetworkMouseY[0] = g_FrameMan.GetPlayerFrameBufferHeight(0) - 2;
					mouse_y = m_NetworkMouseY[0];
				}
			}
#endif
		}

		m_LastFrameMouseX = mouse_x;
		m_LastFrameMouseY = mouse_y;

		if (!m_OverrideInput) {
			if (!m_KeyJoyMouseCursor) {
				if (mouse_b & AllegroMouseButtons::ButtonLeft) {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Up) ? Pushed : Repeat;
					m_MouseButtonsStates[0] = Down;
				} else {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Down) ? Released : None;
					m_MouseButtonsStates[0] = Up;
				}
			}
			if (mouse_b & AllegroMouseButtons::ButtonMiddle) {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Up) ? Pushed : Repeat;
				m_MouseButtonsStates[1] = Down;
			} else {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Down) ? Released : None;
				m_MouseButtonsStates[1] = Up;
			}
			if (mouse_b & AllegroMouseButtons::ButtonRight) {
				m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == Up) ? Pushed : Repeat;
				m_MouseButtonsStates[2] = Down;
			} else {
				m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == Down) ? Released : None;
				m_MouseButtonsStates[2] = Up;
			}

#ifndef GUI_STANDALONE
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
		} else {
			int player = m_Player;
			if (player < 0 || player >= 4) { player = 0; }

			m_NetworkMouseButtonsEvents[player][0] = m_NetworkMouseButtonsStates[player][0] == Down ? (m_PrevNetworkMouseButtonsStates[player][0] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][0] == Down ? Released : None);
			m_NetworkMouseButtonsEvents[player][1] = m_NetworkMouseButtonsStates[player][1] == Down ? (m_PrevNetworkMouseButtonsStates[player][1] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][1] == Down ? Released : None);
			m_NetworkMouseButtonsEvents[player][2] = m_NetworkMouseButtonsStates[player][2] == Down ? (m_PrevNetworkMouseButtonsStates[player][2] == Up ? Pushed : Repeat) : (m_PrevNetworkMouseButtonsStates[player][2] == Down ? Released : None);

			m_PrevNetworkMouseButtonsStates[player][0] = m_NetworkMouseButtonsEvents[player][0];
			m_PrevNetworkMouseButtonsStates[player][1] = m_NetworkMouseButtonsEvents[player][1];
			m_PrevNetworkMouseButtonsStates[player][2] = m_NetworkMouseButtonsEvents[player][2];
#endif
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::UpdateKeyJoyMouseInput(float keyElapsedTime) {
#ifndef GUI_STANDALONE
		int mouseDenominator = g_FrameMan.GetResMultiplier();
		Vector joyKeyDirectional = g_UInputMan.GetMenuDirectional() * 5;

		// See how much to accelerate the joystick input based on how long the stick has been pushed around
		if (joyKeyDirectional.GetMagnitude() < 0.95F) { m_CursorAccelTimer->Reset(); }

		float acceleration = 0.25F + static_cast<float>(std::min(m_CursorAccelTimer->GetElapsedRealTimeS(), 0.5)) * 20.0F;

		// Manipulate the mouse position with the joysticks or keys
		mouse_x = mouse_x + static_cast<volatile int>(joyKeyDirectional.GetX() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration);
		mouse_y = mouse_y + static_cast<volatile int>(joyKeyDirectional.GetY() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration);
		// Prevent mouse from flying out of the screen
		mouse_x = std::max(0, static_cast<int>(mouse_x));
		mouse_y = std::max(0, static_cast<int>(mouse_y));
		// Pull in a bit so cursor doesn't completely disappear
		mouse_x = std::min((g_FrameMan.GetResX() * mouseDenominator) - 3, static_cast<int>(mouse_x));
		mouse_y = std::min((g_FrameMan.GetResY() * mouseDenominator) - 3, static_cast<int>(mouse_y));

		// Button states/presses, Primary - ACTUALLY make either button work, we don't have use for secondary in menus
		if (g_UInputMan.MenuButtonHeld(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Down;
			m_MouseButtonsEvents[0] = Repeat;
		}
		if (g_UInputMan.MenuButtonPressed(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Down;
			m_MouseButtonsEvents[0] = Pushed;
		} else if (g_UInputMan.MenuButtonReleased(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = Up;
			m_MouseButtonsEvents[0] = Released;
		} else if (m_MouseButtonsEvents[0] == Released) {
			m_MouseButtonsStates[0] = Up;
			m_MouseButtonsEvents[0] = None;
		}
#endif
	}
}