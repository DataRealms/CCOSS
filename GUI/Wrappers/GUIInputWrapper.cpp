#include "GUI.h"
#include "GUIInputWrapper.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "Timer.h"
#include "SDL.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIInputWrapper::GUIInputWrapper(int whichPlayer, bool keyJoyMouseCursor) : GUIInput(whichPlayer, keyJoyMouseCursor) {
		m_KeyTimer = std::make_unique<Timer>();
		m_CursorAccelTimer = std::make_unique<Timer>();

		memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		memset(m_ScanCodeState, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);

		m_KeyHoldDuration.fill(-1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIInputWrapper::ConvertKeyEvent(SDL_Scancode sdlKey, int guilibKey, float elapsedS) {
		int nKeys;
		const Uint8 *sdlKeyState = SDL_GetKeyboardState(&nKeys);
		if (sdlKeyState[sdlKey]) {
			if (m_KeyHoldDuration[guilibKey] < 0) {
				m_KeyboardBuffer[guilibKey] = GUIInput::Pushed;
				m_KeyHoldDuration[guilibKey] = 0;
			} else if (m_KeyHoldDuration[guilibKey] < m_KeyRepeatDelay) {
				m_KeyboardBuffer[guilibKey] = GUIInput::None;
			} else {
				m_KeyboardBuffer[guilibKey] = GUIInput::Repeat;
				m_KeyHoldDuration[guilibKey] = 0;
			}
			m_KeyHoldDuration[guilibKey] += elapsedS;
		} else {
			if (m_KeyHoldDuration[guilibKey] >= 0) {
				m_KeyboardBuffer[guilibKey] = GUIInput::Released;
			} else {
				m_KeyboardBuffer[guilibKey] = GUIInput::None;
			}
			m_KeyHoldDuration[guilibKey] = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIInputWrapper::Update() {
		float keyElapsedTime = static_cast<float>(m_KeyTimer->GetElapsedRealTimeS());
		m_KeyTimer->Reset();

		UpdateKeyboardInput(keyElapsedTime);
		UpdateMouseInput();

		// If joysticks and keyboard can control the mouse cursor too.
		if (m_KeyJoyMouseCursor) { UpdateKeyJoyMouseInput(keyElapsedTime); }

		// Update the mouse position of this GUIInput, based on the SDL mouse vars (which may have been altered by joystick or keyboard input).
		Vector mousePos = g_UInputMan.GetAbsoluteMousePosition();
		m_MouseX = static_cast<int>(mousePos.GetX() / static_cast<float>(g_WindowMan.GetResMultiplier()));
		m_MouseY = static_cast<int>(mousePos.GetY() / static_cast<float>(g_WindowMan.GetResMultiplier()));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIInputWrapper::UpdateKeyboardInput(float keyElapsedTime) {
		// Clear the keyboard buffer, we need it to check for changes.
		memset(m_KeyboardBuffer, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);
		memset(m_ScanCodeState, 0, sizeof(uint8_t) * GUIInput::Constants::KEYBOARD_BUFFER_SIZE);

		for (size_t k = 0; k < GUIInput::Constants::KEYBOARD_BUFFER_SIZE; ++k) {
			if (g_UInputMan.KeyPressed(static_cast<SDL_Scancode>(k))) {
				m_ScanCodeState[k] = GUIInput::Pushed;
				uint8_t keyName = static_cast<uint8_t>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(k)));
				m_KeyboardBuffer[keyName] = GUIInput::Pushed;
			}
		}
		m_HasTextInput = g_UInputMan.GetTextInput(m_TextInput);

		ConvertKeyEvent(SDL_SCANCODE_SPACE, ' ', keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_BACKSPACE, GUIInput::Key_Backspace, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_TAB, GUIInput::Key_Tab, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_RETURN, GUIInput::Key_Enter, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_KP_ENTER, GUIInput::Key_Enter, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_ESCAPE, GUIInput::Key_Escape, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_LEFT, GUIInput::Key_LeftArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_RIGHT, GUIInput::Key_RightArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_UP, GUIInput::Key_UpArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_DOWN, GUIInput::Key_DownArrow, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_INSERT, GUIInput::Key_Insert, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_DELETE, GUIInput::Key_Delete, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_HOME, GUIInput::Key_Home, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_END, GUIInput::Key_End, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_PAGEUP, GUIInput::Key_PageUp, keyElapsedTime);
		ConvertKeyEvent(SDL_SCANCODE_PAGEDOWN, GUIInput::Key_PageDown, keyElapsedTime);

		m_Modifier = GUIInput::ModNone;
		SDL_Keymod keyShifts = SDL_GetModState();

		if (keyShifts & KMOD_SHIFT) { m_Modifier |= GUIInput::ModShift; }
		if (keyShifts & KMOD_ALT) { m_Modifier |= GUIInput::ModAlt; }
		if (keyShifts & KMOD_CTRL) { m_Modifier |= GUIInput::ModCtrl; }
		if (keyShifts & KMOD_GUI) { m_Modifier |= GUIInput::ModCommand; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIInputWrapper::UpdateMouseInput() {
		int discard;
		Uint32 buttonState = SDL_GetMouseState(&discard, &discard);
		Vector mousePos = g_UInputMan.GetAbsoluteMousePosition();

		if (m_OverrideInput) {
			mousePos.SetXY(static_cast<float>(m_LastFrameMouseX), static_cast<float>(m_LastFrameMouseY));

			if (m_Player >= 0 && m_Player < 4) {
				if (m_NetworkMouseX[m_Player] != 0) {
					if (m_NetworkMouseX[m_Player] < 0) {
						m_NetworkMouseX[m_Player] = 1;
					}
					if (m_NetworkMouseX[m_Player] >= g_FrameMan.GetPlayerFrameBufferWidth(m_Player)) {
						m_NetworkMouseX[m_Player] = g_FrameMan.GetPlayerFrameBufferWidth(m_Player) - 2;
					}
					mousePos.SetX(static_cast<float>(m_NetworkMouseX[m_Player]));
				}
				if (m_NetworkMouseY[m_Player] != 0) {
					if (m_NetworkMouseY[m_Player] < 0) {
						m_NetworkMouseY[m_Player] = 1;
					}
					if (m_NetworkMouseY[m_Player] >= g_FrameMan.GetPlayerFrameBufferHeight(m_Player)) {
						m_NetworkMouseY[m_Player] = g_FrameMan.GetPlayerFrameBufferHeight(m_Player) - 2;
					}
					mousePos.SetY(static_cast<float>(m_NetworkMouseY[m_Player]));
				}
			} else {
				if (m_NetworkMouseX[0] != 0) {
					if (m_NetworkMouseX[0] < 0) {
						m_NetworkMouseX[0] = 1;
					}
					if (m_NetworkMouseX[0] >= g_FrameMan.GetPlayerFrameBufferWidth(0)) {
						m_NetworkMouseX[0] = g_FrameMan.GetPlayerFrameBufferWidth(0) - 2;
					}
					mousePos.SetX(static_cast<float>(m_NetworkMouseX[0]));
				}
				if (m_NetworkMouseY[0] != 0) {
					if (m_NetworkMouseY[0] < 0) {
						m_NetworkMouseY[0] = 1;
					}
					if (m_NetworkMouseY[0] >= g_FrameMan.GetPlayerFrameBufferHeight(0)) {
						m_NetworkMouseY[0] = g_FrameMan.GetPlayerFrameBufferHeight(0) - 2;
					}
					mousePos.SetY(static_cast<float>(m_NetworkMouseY[0]));
				}
			}
			g_UInputMan.SetAbsoluteMousePosition(mousePos);
		}

		m_LastFrameMouseX = mousePos.GetFloorIntX();
		m_LastFrameMouseY = mousePos.GetFloorIntY();

		if (!m_OverrideInput) {
			if (!m_KeyJoyMouseCursor) {
				if (buttonState & SDL_BUTTON_LMASK) {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == GUIInput::Up) ? GUIInput::Pushed : GUIInput::Repeat;
					m_MouseButtonsStates[0] = GUIInput::Down;
				} else {
					m_MouseButtonsEvents[0] = (m_MouseButtonsStates[0] == GUIInput::Down) ? GUIInput::Released : GUIInput::None;
					m_MouseButtonsStates[0] = GUIInput::Up;
				}
			}
			if (buttonState & SDL_BUTTON_MMASK) {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == GUIInput::Up) ? GUIInput::Pushed : GUIInput::Repeat;
				m_MouseButtonsStates[1] = GUIInput::Down;
			} else {
				m_MouseButtonsEvents[1] = (m_MouseButtonsStates[1] == GUIInput::Down) ? GUIInput::Released : GUIInput::None;
				m_MouseButtonsStates[1] = GUIInput::Up;
			}
			if (buttonState & SDL_BUTTON_RMASK) {
				m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == GUIInput::Up) ? GUIInput::Pushed : GUIInput::Repeat;
				m_MouseButtonsStates[2] = GUIInput::Down;
			} else {
				m_MouseButtonsEvents[2] = (m_MouseButtonsStates[2] == GUIInput::Down) ? GUIInput::Released : GUIInput::None;
				m_MouseButtonsStates[2] = GUIInput::Up;
			}

			if (m_Player <= Players::NoPlayer || m_Player >= Players::MaxPlayerCount) {
				for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					m_MouseWheelChange = g_UInputMan.MouseWheelMovedByPlayer(player);
					if (m_MouseWheelChange) {
						break;
					}
				}
			} else {
				m_MouseWheelChange = g_UInputMan.MouseWheelMovedByPlayer(m_Player);
			}
		} else {
			int player = (m_Player <= Players::NoPlayer || m_Player >= Players::MaxPlayerCount) ? 0 : m_Player;

			m_NetworkMouseButtonsEvents[player][0] = m_NetworkMouseButtonsStates[player][0] == GUIInput::Down ? (m_PrevNetworkMouseButtonsStates[player][0] == GUIInput::Up ? GUIInput::Pushed : GUIInput::Repeat) : (m_PrevNetworkMouseButtonsStates[player][0] == GUIInput::Down ? GUIInput::Released : GUIInput::None);
			m_NetworkMouseButtonsEvents[player][1] = m_NetworkMouseButtonsStates[player][1] == GUIInput::Down ? (m_PrevNetworkMouseButtonsStates[player][1] == GUIInput::Up ? GUIInput::Pushed : GUIInput::Repeat) : (m_PrevNetworkMouseButtonsStates[player][1] == GUIInput::Down ? GUIInput::Released : GUIInput::None);
			m_NetworkMouseButtonsEvents[player][2] = m_NetworkMouseButtonsStates[player][2] == GUIInput::Down ? (m_PrevNetworkMouseButtonsStates[player][2] == GUIInput::Up ? GUIInput::Pushed : GUIInput::Repeat) : (m_PrevNetworkMouseButtonsStates[player][2] == GUIInput::Down ? GUIInput::Released : GUIInput::None);

			m_PrevNetworkMouseButtonsStates[player][0] = m_NetworkMouseButtonsEvents[player][0];
			m_PrevNetworkMouseButtonsStates[player][1] = m_NetworkMouseButtonsEvents[player][1];
			m_PrevNetworkMouseButtonsStates[player][2] = m_NetworkMouseButtonsEvents[player][2];
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIInputWrapper::UpdateKeyJoyMouseInput(float keyElapsedTime) {
		//TODO Try to not use magic numbers throughout this method.
		int mouseDenominator = g_WindowMan.GetResMultiplier();
		Vector joyKeyDirectional = g_UInputMan.GetMenuDirectional() * 5;

		// See how much to accelerate the joystick input based on how long the stick has been pushed around.
		if (joyKeyDirectional.MagnitudeIsLessThan(0.95F)) { m_CursorAccelTimer->Reset(); }

		float acceleration = 0.25F + static_cast<float>(std::min(m_CursorAccelTimer->GetElapsedRealTimeS(), 0.5)) * 20.0F;
		Vector newMousePos = g_UInputMan.GetAbsoluteMousePosition();

		// Manipulate the mouse position with the joysticks or keys.
		newMousePos.m_X += joyKeyDirectional.GetX() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration;
		newMousePos.m_Y += joyKeyDirectional.GetY() * static_cast<float>(mouseDenominator) * keyElapsedTime * 15.0F * acceleration;

		// Keep the mouse within the bounds of the screen. Give it a bit of leeway on the right side, to account for the cursor sprite size.
		newMousePos.m_X = std::clamp(newMousePos.m_X, 0.0F, static_cast<float>(g_WindowMan.GetResX() * mouseDenominator) - 3.0F);
		newMousePos.m_Y = std::clamp(newMousePos.m_Y, 0.0F, static_cast<float>(g_WindowMan.GetResY() * mouseDenominator) - 3.0F);

		g_UInputMan.SetAbsoluteMousePosition(newMousePos);

		// Update mouse button states and presses. In the menu, either left or mouse button works.
		if (g_UInputMan.MenuButtonHeld(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = GUIInput::Down;
			m_MouseButtonsEvents[0] = GUIInput::Repeat;
		}
		if (g_UInputMan.MenuButtonPressed(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = GUIInput::Down;
			m_MouseButtonsEvents[0] = GUIInput::Pushed;
		} else if (g_UInputMan.MenuButtonReleased(UInputMan::MenuCursorButtons::MENU_EITHER)) {
			m_MouseButtonsStates[0] = GUIInput::Up;
			m_MouseButtonsEvents[0] = GUIInput::Released;
		} else if (m_MouseButtonsEvents[0] == GUIInput::Released) {
			m_MouseButtonsStates[0] = GUIInput::Up;
			m_MouseButtonsEvents[0] = GUIInput::None;
		}
	}
}