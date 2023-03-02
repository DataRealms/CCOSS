#include "GUI.h"
#include "AllegroInput.h"
#include "Timer.h"

#ifndef GUI_STANDALONE
#include "FrameMan.h"
#include "UInputMan.h"
#include "SDL.h"
#endif

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AllegroInput::AllegroInput(int whichPlayer, bool keyJoyMouseCursor) : GUIInput(whichPlayer, keyJoyMouseCursor) {
		m_KeyTimer = std::make_unique<Timer>();
		m_CursorAccelTimer = std::make_unique<Timer>();

		memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		memset(m_ScanCodeState, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);

		m_KeyHoldDuration.fill(-1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::ConvertKeyEvent(SDL_Scancode sdlKey, int guilibKey, float elapsedS) {
		int nKeys;
		const Uint8* sdlKeyState = SDL_GetKeyboardState(&nKeys);
		if (sdlKeyState[sdlKey]) {
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
		Vector mousePos = g_UInputMan.GetAbsoluteMousePosition();
		m_MouseX = static_cast<int>(static_cast<float>(mousePos.m_X) / static_cast<float>(g_FrameMan.GetResMultiplier()));
		m_MouseY = static_cast<int>(static_cast<float>(mousePos.m_Y) / static_cast<float>(g_FrameMan.GetResMultiplier()));
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

		for (size_t k = 0; k < KEYBOARD_BUFFER_SIZE; ++k) {
			if(g_UInputMan.KeyPressed(static_cast<SDL_Scancode>(k))){
				m_ScanCodeState[k] = Pushed;
				uint8_t keyName = static_cast<uint8_t>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(k)));
				m_KeyboardBuffer[keyName] = Pushed;
			}
		}
		m_HasTextInput = g_UInputMan.GetTextInput(m_TextInput);

		ConvertKeyEvent(SDL_SCANCODE_SPACE, ' ', keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_BACKSPACE, Key_Backspace, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_TAB, Key_Tab, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_RETURN, Key_Enter, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_KP_ENTER, Key_Enter, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_ESCAPE, Key_Escape, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_LEFT, Key_LeftArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_RIGHT, Key_RightArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_UP, Key_UpArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_DOWN, Key_DownArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_INSERT, Key_Insert, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_DELETE, Key_Delete, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_HOME, Key_Home, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_END, Key_End, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_PAGEUP, Key_PageUp, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_PAGEDOWN, Key_PageDown, keyElapsedTime);

		m_Modifier = ModNone;

		SDL_Keymod keyShifts = SDL_GetModState();

#ifndef GUI_STANDALONE
		if (keyShifts & KMOD_SHIFT) { m_Modifier |= ModShift; }
		if (keyShifts & KMOD_ALT) { m_Modifier |= ModAlt; }
		if (keyShifts & KMOD_CTRL) { m_Modifier |= ModCtrl; }
		if (keyShifts & KMOD_GUI) { m_Modifier |= ModCommand; }
#else
		if (key[KEY_LSHIFT] || key[KEY_RSHIFT]) { m_Modifier |= ModShift; }
		if (key[KEY_ALT]) { m_Modifier |= ModAlt; }
		if (key[KEY_LCONTROL] || key[KEY_RCONTROL]) { m_Modifier |= ModCtrl; }
		if (key[KEY_COMMAND]) { m_Modifier |= ModCommand; }
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroInput::UpdateMouseInput() {
		int discard;
		Uint32 buttonState = SDL_GetMouseState(&discard, &discard);
		Vector mousePos = g_UInputMan.GetAbsoluteMousePosition();

		if (!m_OverrideInput) {
#ifndef GUI_STANDALONE
		} else {

			mousePos.m_X = m_LastFrameMouseX;
			mousePos.m_Y = m_LastFrameMouseY;

			if (m_Player >= 0 && m_Player < 4) {
				if (m_NetworkMouseX[m_Player] != 0) {
					if (m_NetworkMouseX[m_Player] < 0) m_NetworkMouseX[m_Player] = 1;
					if (m_NetworkMouseX[m_Player] >= g_FrameMan.GetPlayerFrameBufferWidth(m_Player)) m_NetworkMouseX[m_Player] = g_FrameMan.GetPlayerFrameBufferWidth(m_Player) - 2;
					mousePos.m_X = m_NetworkMouseX[m_Player];
				}
				if (m_NetworkMouseY[m_Player] != 0) {
					if (m_NetworkMouseY[m_Player] < 0) m_NetworkMouseY[m_Player] = 1;
					if (m_NetworkMouseY[m_Player] >= g_FrameMan.GetPlayerFrameBufferHeight(m_Player)) m_NetworkMouseY[m_Player] = g_FrameMan.GetPlayerFrameBufferHeight(m_Player) - 2;
					mousePos.m_Y = m_NetworkMouseY[m_Player];
				}
			} else {
				if (m_NetworkMouseX[0] != 0) {
					if (m_NetworkMouseX[0] < 0) m_NetworkMouseX[0] = 1;
					if (m_NetworkMouseX[0] >= g_FrameMan.GetPlayerFrameBufferWidth(0)) m_NetworkMouseX[0] = g_FrameMan.GetPlayerFrameBufferWidth(0) - 2;
					mousePos.m_X = m_NetworkMouseX[0];
				}
				if (m_NetworkMouseY[0] != 0) {
					if (m_NetworkMouseY[0] < 0) m_NetworkMouseY[0] = 1;
					if (m_NetworkMouseY[0] >= g_FrameMan.GetPlayerFrameBufferHeight(0)) m_NetworkMouseY[0] = g_FrameMan.GetPlayerFrameBufferHeight(0) - 2;
					mousePos.m_Y = m_NetworkMouseY[0];
				}
			}
			g_UInputMan.SetAbsoluteMousePosition(mousePos);
#endif
		}

		m_LastFrameMouseX = mousePos.m_X;
		m_LastFrameMouseY = mousePos.m_Y;

		if (!m_OverrideInput) {
			if (!m_KeyJoyMouseCursor) {
				if (buttonState & SDL_BUTTON_LMASK) {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Up) ? Pushed : Repeat;
					m_MouseButtonsStates[0] = Down;
				} else {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == Down) ? Released : None;
					m_MouseButtonsStates[0] = Up;
				}
			}
			if (buttonState & SDL_BUTTON_MMASK) {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Up) ? Pushed : Repeat;
				m_MouseButtonsStates[1] = Down;
			} else {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == Down) ? Released : None;
				m_MouseButtonsStates[1] = Up;
			}
			if (buttonState & SDL_BUTTON_RMASK) {
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
		if (joyKeyDirectional.MagnitudeIsLessThan(0.95F)) { m_CursorAccelTimer->Reset(); }

		float acceleration = 0.25F + static_cast<float>(std::min(m_CursorAccelTimer->GetElapsedRealTimeS(), 0.5)) * 20.0F;
		Vector newMousePos = g_UInputMan.GetAbsoluteMousePosition();

		// Manipulate the mouse position with the joysticks or keys
		newMousePos.m_X += joyKeyDirectional.GetX() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration;
		newMousePos.m_Y += joyKeyDirectional.GetY() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration;
		// Prevent mouse from flying out of the screen
		newMousePos.m_X = std::max(0.0f, newMousePos.m_X);
		newMousePos.m_Y = std::max(0.0f, newMousePos.m_Y);
		// Pull in a bit so cursor doesn't completely disappear
		newMousePos.m_X = std::min((g_FrameMan.GetResX() * mouseDenominator) - 3.0f, newMousePos.m_X);
		newMousePos.m_Y = std::min((g_FrameMan.GetResY() * mouseDenominator) - 3.0f, newMousePos.m_Y);

		g_UInputMan.SetAbsoluteMousePosition(newMousePos);

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
