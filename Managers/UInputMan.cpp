#include "UInputMan.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "MetaMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PerformanceMan.h"
#include "GUIInput.h"
#include "Icon.h"
#include "GameActivity.h"
#include "NetworkServer.h"

#include "SDL2/SDL.h"

namespace RTE {

	GUIInput *UInputMan::s_GUIInputInstanceToCaptureKeyStateFrom = nullptr;

	std::array<uint8_t, SDL_NUM_SCANCODES> UInputMan::s_PrevKeyStates;
	std::array<uint8_t, SDL_NUM_SCANCODES> UInputMan::s_ChangedKeyStates;

	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_CurrentMouseButtonStates;
	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_PrevMouseButtonStates;
	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_ChangedMouseButtonStates;

	std::vector<Gamepad> UInputMan::s_PrevJoystickStates(Players::MaxPlayerCount);
	std::vector<Gamepad> UInputMan::s_ChangedJoystickStates(Players::MaxPlayerCount);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::Clear() {
		m_TextInput = "";
		m_NumJoysticks = 0;
		m_OverrideInput = false;
		m_GameHasAnyFocus = false;
		m_FrameLostFocus= false;
		m_AbsoluteMousePos.Reset();
		m_RawMouseMovement.Reset();
		m_AnalogMouseData.Reset();
		m_MouseSensitivity = 0.6F;
		m_MouseWheelChange = 0;
		m_TrapMousePos = false;
		m_MouseTrapRadius = 350;
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;
		m_DisableKeyboard = false;
		m_DisableMouseMoving = false;
		m_PrepareToEnableMouseMoving = false;

		std::fill(std::begin(m_DeviceIcons), std::end(m_DeviceIcons), nullptr);

		// Init the previous keys, mouse and joy buttons so they don't make it seem like things have changed and also neutralize the changed keys so that no Releases will be detected initially
		std::fill(s_PrevKeyStates.begin(), s_PrevKeyStates.end(), SDL_RELEASED);
		// std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);
		std::fill(s_ChangedKeyStates.begin(), s_ChangedKeyStates.end(), false);
		std::fill(s_CurrentMouseButtonStates.begin(), s_CurrentMouseButtonStates.end(), false);
		std::fill(s_PrevMouseButtonStates.begin(), s_PrevMouseButtonStates.end(), false);
		std::fill(s_ChangedMouseButtonStates.begin(), s_ChangedMouseButtonStates.end(), false);

		for (Gamepad& gp: s_PrevJoystickStates) {
			if (gp.m_JoystickID != -1) {
				SDL_GameControllerClose(SDL_GameControllerFromInstanceID(gp.m_JoystickID));
				gp.m_JoystickID = -1;
			}
		}

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ControlScheme[player].Reset();
			m_ControlScheme[player].ResetToPlayerDefaults(static_cast<Players>(player));

			for (int inputState = InputState::Held; inputState < InputState::InputStateCount; inputState++) {
				for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
					m_NetworkInputElementState[player][element][inputState] = false;
				}
				for (int mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
					m_NetworkMouseButtonState[player][mouseButton][inputState] = false;
				}
			}
			m_NetworkAccumulatedRawMouseMovement[player].Reset();
			m_NetworkAnalogMoveData[player].Reset();
			m_NetworkMouseWheelState[player] = 0;
			m_TrapMousePosPerPlayer[player] = false;
		}

		for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++) {
			for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkAccumulatedElementState[element][inputState] = false;
			}
		}

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Initialize() {
		int nKeys;
		const Uint8* keyboardState = SDL_GetKeyboardState(&nKeys);
		std::copy(keyboardState, keyboardState + nKeys, s_PrevKeyStates.begin());

		int controllerIndex = 0;

		for(size_t index = 0; index < std::min(SDL_NumJoysticks(), static_cast<int>(Players::MaxPlayerCount)); ++index) {
			if (SDL_IsGameController(index)) {
				SDL_GameController* controller = SDL_GameControllerOpen(index);
				if (!controller) {
					g_ConsoleMan.PrintString("ERROR: Failed to open controller " + std::to_string(index) + " " + std::string(SDL_GetError()));
					continue;
				}
				SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
				s_PrevJoystickStates[controllerIndex] = Gamepad(index, id, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX);
				s_ChangedJoystickStates[controllerIndex] = Gamepad(index, id, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX);
				controllerIndex++;
				m_NumJoysticks++;
			} else {
				SDL_Joystick* joy =  SDL_JoystickOpen(index);
				if (!joy) {
					g_ConsoleMan.PrintString("ERROR: Failed to open joystick.");
					continue;
				}
				SDL_JoystickID id = SDL_JoystickInstanceID(joy);
				s_PrevJoystickStates[controllerIndex] = Gamepad(index, id, SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy));
				s_ChangedJoystickStates[controllerIndex] = Gamepad(index, id, SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy));
				controllerIndex++;
				m_NumJoysticks++;
			}
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::LoadDeviceIcons() {
		m_DeviceIcons[InputDevice::DEVICE_KEYB_ONLY] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
		m_DeviceIcons[InputDevice::DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));

		for (int gamepad = InputDevice::DEVICE_GAMEPAD_1; gamepad < InputDevice::DEVICE_COUNT; gamepad++) {
			m_DeviceIcons[gamepad] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad " + std::to_string(gamepad - 1)));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetGUIInputInstanceToCaptureKeyStateFrom(GUIInput *inputClass) const {
		s_GUIInputInstanceToCaptureKeyStateFrom = inputClass;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::AnalogMoveValues(int whichPlayer) {
		Vector moveValues(0, 0);
		InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme.at(whichPlayer).GetInputMappings();

			// Assume axes are stretched out over up-down, and left-right
			if (inputElements->at(InputElements::INPUT_L_LEFT).JoyDirMapped()) {
				moveValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_LEFT).GetAxis()));
			}
			if (inputElements->at(InputElements::INPUT_L_UP).JoyDirMapped()) {
				moveValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_UP).GetAxis()));
			}
		}
		return moveValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::AnalogAimValues(int whichPlayer) {
		InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();

		if (IsInMultiplayerMode()) { device = InputDevice::DEVICE_MOUSE_KEYB; }

		Vector aimValues(0, 0);
		if (device == InputDevice::DEVICE_MOUSE_KEYB) {
			aimValues = (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) ? (m_NetworkAnalogMoveData[whichPlayer] / m_MouseTrapRadius) : (m_AnalogMouseData / m_MouseTrapRadius);
		}
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme.at(whichPlayer).GetInputMappings();

			// Assume axes are stretched out over up-down, and left-right
			if (inputElements->at(InputElements::INPUT_R_LEFT).JoyDirMapped()) {
				aimValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_LEFT).GetAxis()));
			}
			if (inputElements->at(InputElements::INPUT_R_UP).JoyDirMapped()) {
				aimValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_UP).GetAxis()));
			}
		}
		return aimValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetMenuDirectional() {
		Vector allInput(0,0);
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			InputDevice device = m_ControlScheme.at(player).GetDevice();

			switch (device) {
				case InputDevice::DEVICE_KEYB_ONLY:
					if (ElementHeld(player, InputElements::INPUT_L_UP)) {
						allInput.m_Y += -1.0F;
					} else if (ElementHeld(player, InputElements::INPUT_L_DOWN)) {
						allInput.m_Y += 1.0F;
					} else if (ElementHeld(player, InputElements::INPUT_L_LEFT)) {
						allInput.m_X += -1.0F;
					} else if (ElementHeld(player, InputElements::INPUT_L_RIGHT)) {
						allInput.m_X += 1.0F;
					}
					break;
				case InputDevice::DEVICE_MOUSE_KEYB:
					// Mouse player shouldn't be doing anything here, he should be using the mouse!
					break;
				default:
					if (device == InputDevice::DEVICE_COUNT) {
						RTEAbort("Trying to control the menu cursor with an invalid input device!");
						break;
					}

					// Analog enabled device (DEVICE_GAMEPAD_1-4)
					if (AnalogMoveValues(player).GetLargest() > 0.05F) {
						allInput += AnalogMoveValues(player);
						m_LastDeviceWhichControlledGUICursor = device;
					}
					if (AnalogAimValues(player).GetLargest() > 0.05F) {
						allInput += AnalogAimValues(player);
						m_LastDeviceWhichControlledGUICursor = device;
					}
					break;
			}
		}
		allInput.CapMagnitude(1.0F);

		return allInput;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyKeyOrJoyInput() const {
		bool input = AnyKeyPress();
		if (!input) { input = AnyJoyInput(); }
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyPress() const {
		bool pressed = false;

		if (!pressed) { pressed = AnyKeyPress(); }
		if (!pressed) { pressed = AnyMouseButtonPress(); }
		if (!pressed) { pressed = AnyJoyPress(); }

		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyStartPress(bool includeSpacebar) {
		if (KeyPressed(SDLK_ESCAPE) || (includeSpacebar && KeyPressed(SDLK_SPACE))) {
			return true;
		}
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (ElementPressed(player, InputElements::INPUT_START)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyBackPress() {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (ElementPressed(player, InputElements::INPUT_BACK)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyKeyPress() const {
		for (size_t testKey = SDL_SCANCODE_A; testKey < SDL_NUM_SCANCODES; ++testKey) {
			if (s_PrevKeyStates[testKey] && s_ChangedKeyStates[testKey]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::MouseUsedByPlayer() const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				return player;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::DisableMouseMoving(bool disable) {
		if (disable) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			m_DisableMouseMoving = true;
			m_PrepareToEnableMouseMoving = false;
		} else {
			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(m_TrapMousePos));
			m_PrepareToEnableMouseMoving = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetMouseMovement(int whichPlayer) const {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			return m_NetworkAccumulatedRawMouseMovement[whichPlayer];
		}
		if (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			return m_RawMouseMovement;
		}
		return Vector(0, 0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMouseValueMagnitude(float magCap, int whichPlayer) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			m_NetworkAnalogMoveData[whichPlayer].CapMagnitude(m_MouseTrapRadius * magCap);
		}
		m_AnalogMouseData.SetMagnitude(m_MouseTrapRadius * magCap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMouseValueAngle(float angle, int whichPlayer) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			m_NetworkAnalogMoveData[whichPlayer].SetAbsRadAngle(angle);
		}
		m_AnalogMouseData.SetAbsRadAngle(angle);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMousePos(Vector &newPos, int whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			SDL_WarpMouseInWindow(g_FrameMan.GetWindow(), newPos.GetX(), newPos.GetY());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyMouseButtonPress() const {
		for (int button = MouseButtons::MOUSE_LEFT; button < MouseButtons::MAX_MOUSE_BUTTONS; ++button) {
			if (MouseButtonPressed(button, -1)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::TrapMousePos(bool trap, int whichPlayer) {
		if (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			m_TrapMousePos = trap;
		}
		m_TrapMousePosPerPlayer[whichPlayer] = trap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			int limitX = std::clamp(static_cast<int>(m_AbsoluteMousePos.m_X), x, x + width);
			int limitY = std::clamp(static_cast<int>(m_AbsoluteMousePos.m_Y), y, y + height);
			SDL_WarpMouseInWindow(g_FrameMan.GetWindow(), limitX, limitY);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinPlayerScreen(int whichPlayer) const {
		if (whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer) * g_FrameMan.GetResMultiplier();
			int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer) * g_FrameMan.GetResMultiplier();

			switch (g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer)) {
				case 0:
					ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
					break;
				case 1:
					if (g_FrameMan.GetVSplit()) {
						ForceMouseWithinBox(screenWidth, 0, screenWidth, screenHeight, whichPlayer);
					} else {
						ForceMouseWithinBox(0, screenHeight, screenWidth, screenHeight, whichPlayer);
					}
					break;
				case 2:
					ForceMouseWithinBox(0, screenHeight, screenWidth, screenHeight, whichPlayer);
					break;
				case 3:
					ForceMouseWithinBox(screenWidth, screenHeight, screenWidth, screenHeight, whichPlayer);
					break;
				default:
					// ScreenOfPlayer will return -1 for inactive player so do nothing.
					break;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonHeld(int whichJoy) const {
		if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
			for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
				if (s_PrevJoystickStates[whichJoy].m_Buttons[button]) {
					return button;
				}
			}
		}

		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonPressed(int whichJoy) const {
		if (whichJoy >=0 && whichJoy < s_PrevJoystickStates.size()) {
			for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
				if (s_PrevJoystickStates[whichJoy].m_Buttons[button] && s_ChangedJoystickStates[whichJoy].m_Buttons[button]) {
					return button;
				}
			}
		}
		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float UInputMan::AnalogAxisValue(int whichJoy, int whichAxis) const {
		if (whichJoy < s_PrevJoystickStates.size() && whichAxis < s_PrevJoystickStates[whichJoy].m_Axis.size()) {
			if (s_PrevJoystickStates[whichJoy].m_JoystickID != -1) {
				float analogValue = static_cast<float>(s_PrevJoystickStates[whichJoy].m_Axis[whichAxis]) / 32767.0f;
				return analogValue;
			}
		}
		return  0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyInput(bool checkForPresses) const {
		size_t gpIndex = 0;
		for (const Gamepad &gp: s_PrevJoystickStates) {
			for (int button = 0; button < gp.m_Buttons.size(); ++button) {
				if (!checkForPresses) {
					if (gp.m_Buttons[button]) {
						return true;
					}
				} else if (JoyButtonPressed(gpIndex, button)) {
						return true;
					}
			}
			for (int axis = 0; axis < gp.m_Axis.size(); ++axis){
				if(!checkForPresses) {
					if (gp.m_Axis[axis] != 0){
						return true;
					}
				} else if(JoyDirectionPressed(gpIndex, axis, JoyDirections::JOYDIR_ONE) || JoyDirectionPressed(gpIndex, axis, JoyDirections::JOYDIR_TWO)){
					return true;
				}
			}

			gpIndex++;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyButtonPress(int whichJoy) const {
		for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
			if (JoyButtonPressed(whichJoy, button)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetNetworkAccumulatedRawMouseMovement(int player) {
		Vector accumulatedMovement = m_NetworkAccumulatedRawMouseMovement[player];
		m_NetworkAccumulatedRawMouseMovement[player].Reset();
		return accumulatedMovement;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ClearNetworkAccumulatedStates() {
		for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++) {
			for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkAccumulatedElementState[element][inputState] = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetInputElementState(int whichPlayer, int whichElement, InputState whichState) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementState[whichPlayer][whichElement][whichState] : false;
		}
		bool elementState = false;
		InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();
		const InputMapping *element = &(m_ControlScheme.at(whichPlayer).GetInputMappings()->at(whichElement));

		if (!elementState && device == InputDevice::DEVICE_KEYB_ONLY || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN))) {
			elementState = GetKeyboardButtonState(static_cast<SDL_Scancode>(element->GetKey()), whichState);
		}
		if (!elementState && device == InputDevice::DEVICE_MOUSE_KEYB && m_TrapMousePos) { elementState = GetMouseButtonState(whichPlayer, element->GetMouseButton(), whichState); }

		if (!elementState && device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			elementState = GetJoystickButtonState(whichJoy, element->GetJoyButton(), whichState);
			if (!elementState && element->JoyDirMapped()) { elementState = GetJoystickDirectionState(whichJoy, element->GetAxis(), element->GetDirection(), whichState); }
		}
		return elementState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetMenuButtonState(int whichButton, InputState whichState) {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			bool buttonState = false;
			InputDevice device = m_ControlScheme[player].GetDevice();
			if (!buttonState && whichButton >= MenuCursorButtons::MENU_PRIMARY) {
				buttonState = GetInputElementState(player, InputElements::INPUT_FIRE, whichState) || GetMouseButtonState(player, MouseButtons::MOUSE_LEFT, whichState);
			}
			if (!buttonState && whichButton >= MenuCursorButtons::MENU_SECONDARY) {
				buttonState = GetInputElementState(player, InputElements::INPUT_PIEMENU, whichState) || GetMouseButtonState(player, MouseButtons::MOUSE_RIGHT, whichState);
			}
			if (buttonState) {
				m_LastDeviceWhichControlledGUICursor = device;
				return buttonState;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetKeyboardButtonState(SDL_Scancode keyToTest, InputState whichState) const {
		if (m_DisableKeyboard && (keyToTest >= SDL_SCANCODE_0 && keyToTest < SDL_SCANCODE_ESCAPE)) {
			return false;
		}
		switch (whichState) {
			case InputState::Held:
				return s_PrevKeyStates[keyToTest];
			case InputState::Pressed:
				return s_GUIInputInstanceToCaptureKeyStateFrom ? (s_GUIInputInstanceToCaptureKeyStateFrom->GetScanCodeState(keyToTest) == GUIInput::Pushed) : (s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest]);
			case InputState::Released:
				return !s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest];
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetMouseButtonState(int whichPlayer, int whichButton, InputState whichState) const {
		if (whichButton < MouseButtons::MOUSE_LEFT || whichButton >= MouseButtons::MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
				for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (m_NetworkMouseButtonState[player][whichButton][whichState]) {
						return m_NetworkMouseButtonState[player][whichButton][whichState];
					}
				}
				return m_NetworkMouseButtonState[Players::PlayerOne][whichButton][whichState];
			} else {
				return m_NetworkMouseButtonState[whichPlayer][whichButton][whichState];
			}
		}

		switch (whichState) {
			case InputState::Held:
				return s_CurrentMouseButtonStates[whichButton];
			case InputState::Pressed:
				return s_CurrentMouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];
			case InputState::Released:
				return !s_CurrentMouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int UInputMan::GetJoystickAxisCount(int whichJoy) const {
		if(whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
			return s_PrevJoystickStates[whichJoy].m_Axis.size();
		}
		return 0;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const {
		if (whichJoy < 0 || whichJoy >= s_PrevJoystickStates.size() || whichButton < 0 || whichButton >= s_PrevJoystickStates[whichJoy].m_Buttons.size()) {
			return false;
		}

		bool button = s_PrevJoystickStates[whichJoy].m_Buttons[whichButton];

		switch (whichState) {
			case InputState::Held:
				return button;
			case InputState::Pressed:
				return button && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
			case InputState::Released:
				return !button && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetJoystickDirectionState(int whichJoy, int whichAxis, int whichDir, InputState whichState) const {
		if (whichJoy < 0 || whichJoy >= s_PrevJoystickStates.size() || whichAxis < 0 || whichAxis >= s_PrevJoystickStates[whichJoy].m_DigitalAxis.size()) {
			return false;
		}
		int axis = s_PrevJoystickStates[whichJoy].m_DigitalAxis[whichAxis];

		if (whichDir == JoyDirections::JOYDIR_ONE) {
			switch (whichState) {
				case InputState::Held:
					return axis == -1;
				case InputState::Pressed:
					return axis == -1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
				case InputState::Released:
					return axis == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration");
					return false;
			}
		} else {
			switch (whichState) {
				case InputState::Held:
					return axis == 1;
				case InputState::Pressed:
					return axis == 1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
				case InputState::Released:
					return axis == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration");
					return false;
			}
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Update() {
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;

		std::fill(s_ChangedKeyStates.begin(), s_ChangedKeyStates.end(), false);
		std::fill(s_ChangedMouseButtonStates.begin(), s_ChangedMouseButtonStates.end(), false);
		for (Gamepad& pad: s_ChangedJoystickStates) {
			std::fill(pad.m_Buttons.begin(), pad.m_Buttons.end(), false);
			std::fill(pad.m_Axis.begin(), pad.m_Axis.end(), 0);
			std::fill(pad.m_DigitalAxis.begin(), pad.m_DigitalAxis.end(), 0);
		}
		m_TextInput.clear();
		m_MouseWheelChange = 0;
		m_RawMouseMovement.Reset();

		SDL_Event e;

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				System::SetQuit(true);
			}
			if (e.type == SDL_KEYUP || e.type == SDL_KEYDOWN) {
				s_ChangedKeyStates[e.key.keysym.scancode] = (e.key.state != s_PrevKeyStates[e.key.keysym.scancode]);
				s_PrevKeyStates[e.key.keysym.scancode] = e.key.state;
			}
			if (e.type == SDL_TEXTINPUT) {
				char input = e.text.text[0];
				size_t i = 0;
				while (input != 0 && i < 32) {
					++i;
					if (input <= 127) {
						m_TextInput += input;
					}
					input = e.text.text[i];
				}
			}
			if (e.type == SDL_MOUSEMOTION) {
				m_RawMouseMovement.SetXY(e.motion.xrel, e.motion.yrel);
				m_AbsoluteMousePos.SetXY(e.motion.x, e.motion.y);
				if (g_FrameMan.IsWindowFullscreen() && SDL_GetNumVideoDisplays() > 1) {
					int x{0};
					int y{0};
					SDL_GetWindowPosition(SDL_GetWindowFromID(e.motion.windowID), &x, &y);
					Vector windowCoord(x, y);
					m_AbsoluteMousePos += windowCoord;
				}
			}
			if (e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEBUTTONDOWN) {
				if (e.button.button > SDL_BUTTON_RIGHT)
					continue;
				s_ChangedMouseButtonStates[e.button.button] = (e.button.state != s_PrevMouseButtonStates[e.button.button]);
				s_PrevMouseButtonStates[e.button.button] = e.button.state;
				s_CurrentMouseButtonStates[e.button.button] = e.button.state;
			}
			if (e.type == SDL_MOUSEWHEEL) {
				m_MouseWheelChange = e.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? e.wheel.y : -e.wheel.y;
			}
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
				if (!m_FrameLostFocus)
					g_FrameMan.DisplaySwitchIn();
				m_GameHasAnyFocus = true;
			}
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
				m_GameHasAnyFocus = false;
				m_FrameLostFocus = true;
				g_FrameMan.DisplaySwitchOut();
			}
			if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_ENTER) {
				if (m_GameHasAnyFocus && g_FrameMan.IsWindowFullscreen() && SDL_GetNumVideoDisplays() > 1) {
					SDL_RaiseWindow(SDL_GetWindowFromID(e.window.windowID));
					SDL_SetWindowInputFocus(SDL_GetWindowFromID(e.window.windowID));
					m_GameHasAnyFocus = true;
				}
			}
			if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_RESIZED)) {
				if (!g_FrameMan.IsWindowFullscreen()) {
					g_FrameMan.WindowResizedCallback(e.window.data1, e.window.data2);
				}
			}
			if (e.type == SDL_CONTROLLERAXISMOTION || e.type == SDL_JOYAXISMOTION) {
				SDL_JoystickID id = e.type == SDL_CONTROLLERAXISMOTION ? e.caxis.which : e.jaxis.which;
				std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), id);
				if (device != s_PrevJoystickStates.end()) {
					if (SDL_IsGameController(device->m_DeviceIndex) && e.type == SDL_CONTROLLERAXISMOTION) {
						UpdateJoystickAxis(device, e.caxis.axis, e.caxis.value);
					} else if (!SDL_IsGameController(device->m_DeviceIndex)) {
						UpdateJoystickAxis(device, e.jaxis.axis, e.jaxis.value);
					}
				}
			}
			if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP || e.type == SDL_JOYBUTTONDOWN || e.type == SDL_JOYBUTTONUP) {
				SDL_JoystickID id = e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP ? e.cbutton.which : e.jbutton.which;
				std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), id);
				if (device != s_PrevJoystickStates.end()) {
					int button = -1;
					int state = -1;
					if (SDL_IsGameController(device->m_DeviceIndex)) {
						if (e.type == SDL_CONTROLLERBUTTONUP || e.type == SDL_CONTROLLERBUTTONDOWN) {
							button = e.cbutton.button;
							state = e.cbutton.state;
						} else {
							continue;
						}
					} else {
						button = e.jbutton.button;
						state = e.jbutton.state;
					}
					int index = device - s_PrevJoystickStates.begin();
					s_ChangedJoystickStates[index].m_Buttons[button] = state != device->m_Buttons[button];
					device->m_Buttons[button] = state;
				}
			}
			if (e.type == SDL_JOYDEVICEADDED) {
				OpenJoystick(e.jdevice.which);
			}
			if (e.type == SDL_JOYDEVICEREMOVED) {
				std::vector<Gamepad>::iterator prevDevice = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), e.jdevice.which);
				if (prevDevice != s_PrevJoystickStates.end()) {
					g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(prevDevice->m_DeviceIndex + 1) + " disconnected!");
					SDL_GameControllerClose(SDL_GameControllerFromInstanceID(prevDevice->m_JoystickID));
					prevDevice->m_JoystickID = -1;
					std::fill(prevDevice->m_Axis.begin(), prevDevice->m_Axis.end(), 0);
					std::fill(prevDevice->m_Buttons.begin(), prevDevice->m_Buttons.end(), false);
					m_NumJoysticks--;
				}
				std::vector<Gamepad>::iterator changedDevice = std::find(s_ChangedJoystickStates.begin(), s_ChangedJoystickStates.end(), e.jdevice.which);
				if (changedDevice != s_ChangedJoystickStates.end()) {
					changedDevice->m_JoystickID = -1;
					std::fill(changedDevice->m_Axis.begin(), changedDevice->m_Axis.end(), false);
					std::fill(changedDevice->m_Buttons.begin(), changedDevice->m_Buttons.end(), false);
				}
			}
		}
		// TODO: Add sensitivity slider to settings menu
		m_RawMouseMovement *= m_MouseSensitivity;
		m_FrameLostFocus = false;
		// NETWORK SERVER: Apply mouse input received from client or collect mouse input
		if (IsInMultiplayerMode()) {
			UpdateNetworkMouseMovement();
		} else {
			m_NetworkAccumulatedRawMouseMovement[Players::PlayerOne] += m_RawMouseMovement;
		}
		UpdateMouseInput();
		HandleSpecialInput();
		StoreInputEventsForNextUpdate();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::HandleSpecialInput() {
		// If we launched into editor directly, skip the logic and quit quickly.
		if (g_ActivityMan.IsSetToLaunchIntoEditor() && KeyPressed(SDLK_ESCAPE)) {
			System::SetQuit();
			return;
		}

		if (g_ActivityMan.IsInActivity()) {
			const GameActivity *gameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity());
			// Don't allow pausing and returning to main menu when running in server mode to not disrupt the simulation for the clients
			if (!g_NetworkServer.IsServerModeEnabled() && AnyStartPress(false) && (!gameActivity || !gameActivity->IsBuyGUIVisible(-1))) {
				g_ActivityMan.PauseActivity();
				return;
			}
			// Ctrl+R or Back button for controllers to reset activity.
			if (!g_MetaMan.GameInProgress() && !g_ActivityMan.ActivitySetToRestart()) {
				g_ActivityMan.SetRestartActivity(FlagCtrlState() && KeyPressed(SDLK_r) || AnyBackPress());
			}
			if (g_ActivityMan.ActivitySetToRestart()) {
				return;
			}
		}
		// If InputClass is set for input capture pressing F1 will open console and hard-lock during manual input configuration, so just skip processing all special input until it's nullptr again.
		if (s_GUIInputInstanceToCaptureKeyStateFrom) {
			return;
		}
		if (FlagCtrlState() && !FlagAltState()) {
			// Ctrl+S to save continuous ScreenDumps
			if (KeyHeld(SDLK_s)) {
				g_FrameMan.SaveScreenToPNG("ScreenDump");
			// Ctrl+W to save a WorldDump
			} else if (KeyPressed(SDLK_w)) {
				g_FrameMan.SaveWorldToPNG("WorldDump");
			// Ctrl+M to cycle draw modes
			} else if (KeyPressed(SDLK_m)) {
				g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3);
			// Ctrl+P to toggle performance stats
			} else if (KeyPressed(SDLK_p)) {
				g_PerformanceMan.ShowPerformanceStats(!g_PerformanceMan.IsShowingPerformanceStats());
			// Ctrl+O to toggle one sim update per frame
			} else if (KeyPressed(SDLK_o)) {
				g_TimerMan.SetOneSimUpdatePerFrame(!g_TimerMan.IsOneSimUpdatePerFrame());
			} else if (KeyPressed(SDLK_F2)) {
				g_PresetMan.QuickReloadEntityPreset();
			} else if (KeyPressed(KEY_F9)) {
				g_ActivityMan.LoadAndLaunchGame("AutoSave");
			} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
				if (KeyHeld(KEY_1)) {
					g_TimerMan.SetTimeScale(1.0F);
				} else if (KeyHeld(KEY_3)) {
					g_TimerMan.SetRealToSimCap(c_DefaultRealToSimCap);
				} else if (KeyHeld(KEY_5)) {
					g_TimerMan.SetDeltaTimeSecs(c_DefaultDeltaTimeS);
				}
			}
		} else if (!FlagCtrlState() && FlagAltState()) {
			// Alt+Enter to switch resolution multiplier
			if (KeyPressed(SDLK_RETURN)) {
				g_FrameMan.ChangeResolutionMultiplier((g_FrameMan.GetResMultiplier() >= 2) ? 1 : 2);
			// Alt+W to save ScenePreviewDump (miniature WorldDump)
			} else if (KeyPressed(SDLK_w)) {
				g_FrameMan.SaveWorldPreviewToPNG("ScenePreviewDump");
			} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
				if (KeyPressed(KEY_P)) {
					g_PerformanceMan.ShowAdvancedPerformanceStats(!g_PerformanceMan.AdvancedPerformanceStatsEnabled());
				}
			}
		} else {
			// PrntScren to save a single ScreenDump
			if (KeyPressed(SDLK_PRINTSCREEN)) {
				g_FrameMan.SaveScreenToPNG("ScreenDump");
			} else if (KeyPressed(SDLK_F1)) {
				g_ConsoleMan.ShowShortcuts();
			} else if (KeyPressed(SDLK_F2)) {
				g_PresetMan.ReloadAllScripts();
			} else if (KeyPressed(SDLK_F3)) {
				g_ConsoleMan.SaveAllText("Console.dump.log");
			} else if (KeyPressed(SDLK_F4)) {
				g_ConsoleMan.SaveInputLog("Console.input.log");
			} else if (KeyPressed(SDLK_F5)) {
				g_ActivityMan.SaveCurrentGame("QuickSave");
			} else if (KeyPressed(SDLK_F9)) {
				g_ActivityMan.LoadAndLaunchGame("QuickSave");
			} else if (KeyPressed(SDLK_F10)) {
				g_ConsoleMan.ClearLog();
			}

			if (g_PerformanceMan.IsShowingPerformanceStats()) {
				// Manipulate time scaling
				if (KeyHeld(SDLK_2)) { g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() + 0.01F); }
				if (KeyHeld(SDLK_1) && g_TimerMan.GetTimeScale() - 0.01F > 0.001F) { g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() - 0.01F); }

				// Manipulate real to sim cap
				if (KeyHeld(SDLK_4)) { g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() + 0.001F); }
				if (KeyHeld(SDLK_3) && g_TimerMan.GetRealToSimCap() > 0) { g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() - 0.001F); }

				// Manipulate DeltaTime
				if (KeyHeld(SDLK_6)) { g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() + 0.001F); }
				if (KeyHeld(SDLK_5) && g_TimerMan.GetDeltaTimeSecs() > 0) { g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() - 0.001F); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateNetworkMouseMovement() {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (!m_NetworkAccumulatedRawMouseMovement[player].IsZero()) {
				// TODO: Figure out why we're multiplying by 3 here. Possibly related to mouse sensitivity.
				m_NetworkAnalogMoveData[player].m_X += m_NetworkAccumulatedRawMouseMovement[player].m_X * 3;
				m_NetworkAnalogMoveData[player].m_Y += m_NetworkAccumulatedRawMouseMovement[player].m_Y * 3;
				m_NetworkAnalogMoveData[player].CapMagnitude(m_MouseTrapRadius);
			}
			m_NetworkAccumulatedRawMouseMovement[player].Reset();

			// Clear mouse events and inputs as they should've been already processed during by recipients.
			// This is important so mouse readings are correct, e.g. to ensure events don't trigger multiple times on a single press.
			for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++) {
				for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
					m_NetworkInputElementState[player][element][inputState] = false;
				}
				for (int mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
					m_NetworkMouseButtonState[player][mouseButton][inputState] = false;
				}
			}

			// Reset mouse wheel state to stop over-wheeling
			m_NetworkMouseWheelState[player] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateMouseInput() {
		// Detect and store mouse movement input, translated to analog stick emulation
		int mousePlayer = MouseUsedByPlayer();
		if (mousePlayer != Players::NoPlayer) {
			// Multiplying by 30 for sensitivity. TODO: Make sensitivity slider 1-50;
			m_AnalogMouseData.m_X += m_RawMouseMovement.m_X * 30;
			m_AnalogMouseData.m_Y += m_RawMouseMovement.m_Y * 30;
			m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius);

			// Only mess with the mouse pos if the original mouse position is not above the screen and may be grabbing the title bar of the game window
			if (!m_DisableMouseMoving && !IsInMultiplayerMode()) {
				if (m_TrapMousePos) {
					// Trap the (invisible) mouse cursor in the middle of the screen, so it doesn't fly out in windowed mode and some other window gets clicked
					SDL_WarpMouseInWindow(g_FrameMan.GetWindow(), g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
				} else if (g_ActivityMan.IsInActivity()) {
					// The mouse cursor is visible and can move about the screen/window, but it should still be contained within the mouse player's part of the window
					ForceMouseWithinPlayerScreen(mousePlayer);
				}
			}

			// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
			// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
			int mousePosX = m_AbsoluteMousePos.m_X / g_FrameMan.GetResMultiplier();
			int mousePosY = m_AbsoluteMousePos.m_Y / g_FrameMan.GetResMultiplier();
			if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mousePosX >= 0 && mousePosX < g_FrameMan.GetResX() && mousePosY >= 0 && mousePosY < g_FrameMan.GetResY())) {
				m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateJoystickAxis(std::vector<Gamepad>::iterator device, int axis, int value) {
		if (device != s_PrevJoystickStates.end()) {
			int joystickIndex = device - s_PrevJoystickStates.begin();

			int prevAxisValue = device->m_Axis[axis];
			int prevDigitalValue = device->m_DigitalAxis[axis];

			int newDigitalState = 0;
			if (value > 8912) {
				newDigitalState = 1;
			} else if (value < -8912) {
				newDigitalState = -1;
			}

			s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(value - device->m_Axis[axis]);
			s_ChangedJoystickStates[joystickIndex].m_DigitalAxis[axis] = Sign(newDigitalState - prevDigitalValue);
			device->m_Axis[axis] = value;
			device->m_DigitalAxis[axis] = newDigitalState;

			Players joystickPlayer = Players::NoPlayer;
			float deadZone = 0.0f;
			int deadZoneType = DeadZoneType::CIRCLE;

			for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; ++playerToCheck) {
				InputDevice device = m_ControlScheme[playerToCheck].GetDevice();
				int whichJoy = GetJoystickIndex(device);
				if (whichJoy == joystickIndex) {
					deadZone = m_ControlScheme[playerToCheck].GetJoystickDeadzone();
					deadZoneType = m_ControlScheme[playerToCheck].GetJoystickDeadzoneType();
					joystickPlayer = static_cast<Players>(playerToCheck);
				}
			}

			bool isAxisMapped{false};
			if (joystickPlayer != Players::NoPlayer && deadZone > 0.0f) {
				Vector aimValues;
				const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme[joystickPlayer].GetInputMappings();
				std::array<InputElements, 4> elementsToCheck = {InputElements::INPUT_L_LEFT, InputElements::INPUT_L_UP, InputElements::INPUT_R_LEFT, InputElements::INPUT_R_UP};

				for (size_t i = 0; i < elementsToCheck.size(); i += 2) {
					int axisLeft{SDL_CONTROLLER_AXIS_INVALID};
					int axisUp{SDL_CONTROLLER_AXIS_INVALID};
					if (inputElements->at(elementsToCheck[i]).JoyDirMapped()) {
						axisLeft = inputElements->at(elementsToCheck[i]).GetAxis();

						aimValues.m_X = AnalogAxisValue(joystickIndex, axisLeft);
						isAxisMapped |= axisLeft == axis;
					}
					if (inputElements->at(elementsToCheck[i + 1]).JoyDirMapped()) {
						axisUp = inputElements->at(elementsToCheck[i + 1]).GetAxis();
						aimValues.m_Y = AnalogAxisValue(joystickIndex, axisUp);
						isAxisMapped |= axisUp == axis;
					}
					if (!isAxisMapped) {
						continue;
					}

					if (deadZoneType == DeadZoneType::CIRCLE) {
						if (aimValues.MagnitudeIsLessThan(deadZone)) {
							if (axisLeft != SDL_CONTROLLER_AXIS_INVALID) {
								s_ChangedJoystickStates[joystickIndex].m_Axis[axisLeft] = Sign(axisLeft == axis ? -prevAxisValue : -device->m_Axis[axisLeft]);
								s_ChangedJoystickStates[joystickIndex].m_DigitalAxis[axisLeft] = Sign(axisLeft == axis ? -prevDigitalValue : -device->m_DigitalAxis[axisLeft]);
								device->m_Axis[axisLeft] = 0;
								device->m_DigitalAxis[axisLeft] = 0;
							}
							if (axisUp != SDL_CONTROLLER_AXIS_INVALID) {
								s_ChangedJoystickStates[joystickIndex].m_Axis[axisUp] = Sign(axisUp == axis ? -prevAxisValue : -device->m_Axis[axisUp]);
								s_ChangedJoystickStates[joystickIndex].m_DigitalAxis[axisUp] = Sign(axisLeft == axis ? -prevDigitalValue : -device->m_DigitalAxis[axisUp]);
								device->m_Axis[axisUp] = 0;
								device->m_DigitalAxis[axisUp] = 0;
							}
						}
					}
				}
			}
			if (!isAxisMapped && deadZoneType == DeadZoneType::SQUARE && deadZone > 0.f) {
				if (std::abs(value / 32767.0) < deadZone) {
					s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(-prevAxisValue);
					s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(-prevDigitalValue);
					device->m_Axis[axis] = 0;
					device->m_DigitalAxis[axis] = 0;
				}
			}
		}
	}

	void UInputMan::OpenJoystick(int deviceIndex) {

		SDL_Joystick *controller = nullptr;
		size_t controllerIndex = 0;
		for (; controllerIndex < s_PrevJoystickStates.size(); ++controllerIndex) {
			if (s_PrevJoystickStates[controllerIndex].m_DeviceIndex == deviceIndex) {
				if (SDL_IsGameController(deviceIndex)) {
					SDL_GameController *gameController = SDL_GameControllerOpen(deviceIndex);
					if (!gameController) {
						g_ConsoleMan.PrintString("ERROR: Failed to reconnect Gamepad " + std::to_string(controllerIndex + 1));
						break;
					}
					controller = SDL_GameControllerGetJoystick(gameController);
				} else {
					controller = SDL_JoystickOpen(deviceIndex);
				}
				if (!controller) {
					g_ConsoleMan.PrintString("ERROR: Failed to reconnect Gamepad " + std::to_string(controllerIndex + 1));
					break;
				}
				g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(controllerIndex + 1) + " reconnected");
				break;
			} else if (s_PrevJoystickStates[controllerIndex].m_DeviceIndex == -1) {
				if (SDL_IsGameController(deviceIndex)) {
					SDL_GameController *gameController = SDL_GameControllerOpen(deviceIndex);
					if (!gameController) {
						g_ConsoleMan.PrintString("ERROR: Failed to connect Gamepad " + std::to_string(controllerIndex + 1));
						break;
					}
					controller = SDL_GameControllerGetJoystick(gameController);
				} else {
					controller = SDL_JoystickOpen(deviceIndex);
				}
				if (!controller) {
					g_ConsoleMan.PrintString("ERROR: Failed to connect Gamepad " + std::to_string(controllerIndex + 1));
					break;
				}
				g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(controllerIndex + 1) + " connected");
				break;
			}
		}
		if (!controller && controllerIndex == s_PrevJoystickStates.size()) {
			controllerIndex = 0;
			for (; controllerIndex < s_PrevJoystickStates.size(); ++controllerIndex) {
				if (s_PrevJoystickStates[controllerIndex].m_JoystickID == -1) {
					if (SDL_IsGameController(deviceIndex)) {
						SDL_GameController *gameController = SDL_GameControllerOpen(deviceIndex);
						if (!gameController) {
							g_ConsoleMan.PrintString("ERROR: Failed to connect controller.");
							break;
						}
						controller = SDL_GameControllerGetJoystick(gameController);
					} else {
						controller = SDL_JoystickOpen(deviceIndex);
					}
					if (!controller) {
						g_ConsoleMan.PrintString("ERROR: Failed to connect controller!");
						break;
					}
					g_ConsoleMan.PrintString("INFO: New Gamepad connected as Gamepad " + std::to_string(controllerIndex + 1));
					break;
				}
			}
			g_ConsoleMan.PrintString("ERROR: Too many gamepads connected!");
		}
		if (controller) {
			SDL_JoystickID id = SDL_JoystickInstanceID(controller);
			int nAxis = 0;
			int nButtons = 0;
			if (SDL_IsGameController(deviceIndex)) {
				nAxis = SDL_CONTROLLER_AXIS_MAX;
				nButtons = SDL_CONTROLLER_BUTTON_MAX;
			} else {
				nAxis = SDL_JoystickNumAxes(controller);
				nButtons = SDL_JoystickNumButtons(controller);
			}
			s_PrevJoystickStates[controllerIndex] = Gamepad(deviceIndex, id, nAxis, nButtons);
			s_ChangedJoystickStates[controllerIndex] = Gamepad(deviceIndex, id, nAxis, nButtons);
			m_NumJoysticks++;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::StoreInputEventsForNextUpdate() {
		// Store pressed and released events to be picked by NetworkClient during it's update. These will be cleared after update so we don't care about false but we store the result regardless.
		for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++) {
			for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkAccumulatedElementState[element][inputState] = GetInputElementState(Players::PlayerOne, element, static_cast<InputState>(inputState));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
