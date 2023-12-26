#include "UInputMan.h"

#include "SceneMan.h"
#include "ActivityMan.h"
#include "ThreadMan.h"
#include "MetaMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PerformanceMan.h"
#include "Icon.h"
#include "GameActivity.h"
#include "NetworkServer.h"

#include "SDL.h"
namespace RTE {

	std::array<uint8_t, SDL_NUM_SCANCODES> UInputMan::s_PrevKeyStates;
	std::array<uint8_t, SDL_NUM_SCANCODES> UInputMan::s_ChangedKeyStates;

	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_CurrentMouseButtonStates;
	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_PrevMouseButtonStates;
	std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> UInputMan::s_ChangedMouseButtonStates;

	std::vector<Gamepad> UInputMan::s_PrevJoystickStates(Players::MaxPlayerCount);
	std::vector<Gamepad> UInputMan::s_ChangedJoystickStates(Players::MaxPlayerCount);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::Clear() {
		m_SkipHandlingSpecialInput = false;
		m_TextInput.clear();;
		m_NumJoysticks = 0;
		m_OverrideInput = false;
		m_AbsoluteMousePos.Reset();
		m_RawMouseMovement.Reset();
		m_AnalogMouseData.Reset();
		m_MouseSensitivity = 0.6F;
		m_MouseWheelChange = 0;
		m_TrapMousePos = false;
		m_PlayerScreenMouseBounds = { 0, 0, 0, 0 };
		m_MouseTrapRadius = 350;
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;
		m_DisableKeyboard = false;
		m_DisableMouseMoving = false;
		m_PrepareToEnableMouseMoving = false;

		std::fill(std::begin(m_DeviceIcons), std::end(m_DeviceIcons), nullptr);

		// Init the previous keys, mouse and joy buttons so they don't make it seem like things have changed and also neutralize the changed keys so that no Releases will be detected initially
		std::fill(s_PrevKeyStates.begin(), s_PrevKeyStates.end(), SDL_RELEASED);
		std::fill(s_ChangedKeyStates.begin(), s_ChangedKeyStates.end(), false);
		std::fill(s_CurrentMouseButtonStates.begin(), s_CurrentMouseButtonStates.end(), false);
		std::fill(s_PrevMouseButtonStates.begin(), s_PrevMouseButtonStates.end(), false);
		std::fill(s_ChangedMouseButtonStates.begin(), s_ChangedMouseButtonStates.end(), false);

		for (Gamepad &gamepad: s_PrevJoystickStates) {
			if (gamepad.m_JoystickID != -1) {
				SDL_GameControllerClose(SDL_GameControllerFromInstanceID(gamepad.m_JoystickID));
				gamepad.m_JoystickID = -1;
			}
		}

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ControlScheme[player].Reset();
			m_ControlScheme[player].ResetToPlayerDefaults(static_cast<Players>(player));

			for (int inputState = InputState::Held; inputState < InputState::InputStateCount; inputState++) {
				for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
					m_NetworkServerChangedInputElementState[player][element] = false;
					m_NetworkServerPreviousInputElementState[player][element] = false;
				}
				for (int mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
					m_NetworkServerChangedMouseButtonState[player][mouseButton] = false;
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
		int numKeys;
		const Uint8 *keyboardState = SDL_GetKeyboardState(&numKeys);
		std::copy(keyboardState, keyboardState + numKeys, s_PrevKeyStates.begin());

		int controllerIndex = 0;

		for (size_t index = 0; index < std::min(SDL_NumJoysticks(), static_cast<int>(Players::MaxPlayerCount)); ++index) {
			if (SDL_IsGameController(index)) {
				SDL_GameController *controller = SDL_GameControllerOpen(index);
				if (!controller) {
					g_ConsoleMan.PrintString("ERROR: Failed to connect gamepad " + std::to_string(index) + " " + std::string(SDL_GetError()));
					continue;
				}
				SDL_GameControllerSetPlayerIndex(controller, index);
				SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
				s_PrevJoystickStates[controllerIndex] = Gamepad(index, id, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX);
				s_ChangedJoystickStates[controllerIndex] = Gamepad(index, id, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX);
				controllerIndex++;
				m_NumJoysticks++;
			} else {
				SDL_Joystick *joy = SDL_JoystickOpen(index);
				if (!joy) {
					g_ConsoleMan.PrintString("ERROR: Failed to connect joystick.");
					continue;
				}
				SDL_JoystickID id = SDL_JoystickInstanceID(joy);
				s_PrevJoystickStates[controllerIndex] = Gamepad(index, id, SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy));
				s_ChangedJoystickStates[controllerIndex] = Gamepad(index, id, SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy));
				controllerIndex++;
				m_NumJoysticks++;
			}
		}

		m_PlayerScreenMouseBounds = {
			0,
			0,
			static_cast<int>(g_FrameMan.GetPlayerFrameBufferWidth(Players::NoPlayer) * g_WindowMan.GetResMultiplier()),
			static_cast<int>(g_FrameMan.GetPlayerFrameBufferHeight(Players::NoPlayer) * g_WindowMan.GetResMultiplier())
		};

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

	Vector UInputMan::AnalogMoveValues(int whichPlayer) {
		Vector moveValues(0, 0);
		InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme.at(whichPlayer).GetInputMappings();

			// Assume axes are stretched out over up-down, and left-right.
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
		Vector allInput(0, 0);
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			InputDevice device = m_ControlScheme.at(player).GetDevice();

			switch (device) {
				case InputDevice::DEVICE_KEYB_ONLY:
					if (ElementHeld(player, InputElements::INPUT_L_UP)) {
						allInput.m_Y += -1.0F;
					}
					if (ElementHeld(player, InputElements::INPUT_L_DOWN)) {
						allInput.m_Y += 1.0F;
					}
					if (ElementHeld(player, InputElements::INPUT_L_LEFT)) {
						allInput.m_X += -1.0F;
					}
					if (ElementHeld(player, InputElements::INPUT_L_RIGHT)) {
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

	void UInputMan::SetMousePos(const Vector &newPos, int whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			SDL_WarpMouseInWindow(g_WindowMan.GetWindow(), newPos.GetFloorIntX(), newPos.GetFloorIntY());
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
		if (!IsInMultiplayerMode() && (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			m_TrapMousePos = trap;
			SDL_SetRelativeMouseMode(static_cast<SDL_bool>(trap));
		}
		m_TrapMousePosPerPlayer[whichPlayer] = trap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window.
		if (g_WindowMan.AnyWindowHasFocus() && !m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			int rightMostPos = m_PlayerScreenMouseBounds.x + m_PlayerScreenMouseBounds.w;
			int bottomMostPos = m_PlayerScreenMouseBounds.y + m_PlayerScreenMouseBounds.h;

			if (g_WindowMan.FullyCoversAllDisplays()) {
				int leftPos = std::clamp(m_PlayerScreenMouseBounds.x + x, m_PlayerScreenMouseBounds.x, rightMostPos);
				int topPos = std::clamp(m_PlayerScreenMouseBounds.y + y, m_PlayerScreenMouseBounds.y, bottomMostPos);

				// The max mouse position inside the window is -1 its dimensions so we have to -1 for this to work on the right and bottom edges of the window.
				rightMostPos = std::clamp(leftPos + width, leftPos, rightMostPos - 1);
				bottomMostPos = std::clamp(topPos + height, topPos, bottomMostPos - 1);

				if (!WithinBox(m_AbsoluteMousePos, static_cast<float>(leftPos), static_cast<float>(topPos), static_cast<float>(rightMostPos), static_cast<float>(bottomMostPos))) {
					int limitX = std::clamp(m_AbsoluteMousePos.GetFloorIntX(), leftPos, rightMostPos);
					int limitY = std::clamp(m_AbsoluteMousePos.GetFloorIntY(), topPos, bottomMostPos);
					SDL_WarpMouseInWindow(g_WindowMan.GetWindow(), limitX, limitY);
				}
			} else {
				SDL_Rect newMouseBounds = {
					std::clamp(m_PlayerScreenMouseBounds.x + x, m_PlayerScreenMouseBounds.x, rightMostPos),
					std::clamp(m_PlayerScreenMouseBounds.y + y, m_PlayerScreenMouseBounds.y, bottomMostPos),
					std::clamp(width, 0, rightMostPos - x),
					std::clamp(height, 0, bottomMostPos - y)
				};

				if (newMouseBounds.x >= rightMostPos || newMouseBounds.y >= bottomMostPos) {
					g_ConsoleMan.PrintString("ERROR: Trying to force mouse wihin a box that is outside the player screen bounds!");
					newMouseBounds = m_PlayerScreenMouseBounds;
				}
				SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), &newMouseBounds);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinPlayerScreen(bool force, int whichPlayer) {
		float resMultiplier = g_WindowMan.GetResMultiplier();

		if (force && (whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount)) {
			int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer) * resMultiplier;
			int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer) * resMultiplier;

			switch (g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer)) {
				case 0:
					m_PlayerScreenMouseBounds = { 0, 0, screenWidth, screenHeight };
					break;
				case 1:
					if (g_FrameMan.GetVSplit()) {
						m_PlayerScreenMouseBounds = { screenWidth, 0, screenWidth, screenHeight };
					} else {
						m_PlayerScreenMouseBounds = { 0, screenHeight, screenWidth, screenHeight };
					}
					break;
				case 2:
					m_PlayerScreenMouseBounds = { 0, screenHeight, screenWidth, screenHeight };
					break;
				case 3:
					m_PlayerScreenMouseBounds = { screenWidth, screenHeight, screenWidth, screenHeight };
					break;
				default:
					force = false;
			}
		} else {
			force = false;
		}

		if (force) {
			if (g_WindowMan.FullyCoversAllDisplays()) {
				ForceMouseWithinBox(0, 0, m_PlayerScreenMouseBounds.w, m_PlayerScreenMouseBounds.h);
			} else {
				SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), &m_PlayerScreenMouseBounds);
			}
		} else {
			// Set the mouse bounds to the whole window so ForceMouseWithinBox is not stuck being relative to some player screen, because it can still bind the mouse even if this doesn't.
			m_PlayerScreenMouseBounds = { 0, 0, static_cast<int>(g_WindowMan.GetResX() * resMultiplier), static_cast<int>(g_WindowMan.GetResY() * resMultiplier) };
			SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), nullptr);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::GetJoystickAxisCount(int whichJoy) const {
		if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
			return s_PrevJoystickStates[whichJoy].m_Axis.size();
		}
		return 0;
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
		if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
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
				float analogValue = static_cast<float>(s_PrevJoystickStates[whichJoy].m_Axis[whichAxis]) / 32767.0F;
				return analogValue;
			}
		}
		return  0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyInput(bool checkForPresses) const {
		int gamepadIndex = 0;
		for (const Gamepad &gamepad : s_PrevJoystickStates) {
			for (int button = 0; button < gamepad.m_Buttons.size(); ++button) {
				if (!checkForPresses) {
					if (gamepad.m_Buttons[button]) {
						return true;
					}
				} else if (JoyButtonPressed(gamepadIndex, button)) {
					return true;
				}
			}
			for (int axis = 0; axis < gamepad.m_Axis.size(); ++axis) {
				if (!checkForPresses) {
					if (gamepad.m_Axis[axis] != 0) {
						return true;
					}
				} else if (JoyDirectionPressed(gamepadIndex, axis, JoyDirections::JOYDIR_ONE) || JoyDirectionPressed(gamepadIndex, axis, JoyDirections::JOYDIR_TWO)) {
					return true;
				}
			}
			gamepadIndex++;
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
			return GetNetworkInputElementState(whichPlayer, whichElement, whichState);
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

	bool UInputMan::GetNetworkInputElementState(int whichPlayer, int whichElement, InputState whichState) {
		if (!m_TrapMousePosPerPlayer[whichPlayer] || whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount || whichState < 0 || whichState > InputState::InputStateCount) {
			return false;
		}
		switch (whichState) {
			case InputState::Held:
				return m_NetworkServerPreviousInputElementState[whichPlayer][whichElement];
			case InputState::Pressed:
				return m_NetworkServerPreviousInputElementState[whichPlayer][whichElement] && m_NetworkServerChangedInputElementState[whichPlayer][whichElement];
			case InputState::Released:
				return (!m_NetworkServerPreviousInputElementState[whichPlayer][whichElement]) && m_NetworkServerChangedInputElementState[whichPlayer][whichElement];
			default:
				return false;
		}
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
				buttonState = GetInputElementState(player, InputElements::INPUT_PIEMENU_DIGITAL, whichState) || GetMouseButtonState(player, MouseButtons::MOUSE_RIGHT, whichState);
			}
			if (buttonState) {
				m_LastDeviceWhichControlledGUICursor = device;
				return buttonState;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetKeyboardButtonState(SDL_Scancode scancodeToTest, InputState whichState) const {
		if (m_DisableKeyboard && (scancodeToTest >= SDL_SCANCODE_0 && scancodeToTest < SDL_SCANCODE_ESCAPE)) {
			return false;
		}
		switch (whichState) {
			case InputState::Held:
				return s_PrevKeyStates[scancodeToTest];
			case InputState::Pressed:
				return s_PrevKeyStates[scancodeToTest] && s_ChangedKeyStates[scancodeToTest];
			case InputState::Released:
				return !s_PrevKeyStates[scancodeToTest] && s_ChangedKeyStates[scancodeToTest];
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
			return GetNetworkMouseButtonState(whichPlayer, whichButton, whichState);
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

	bool UInputMan::GetNetworkMouseButtonState(int whichPlayer, int whichButton, InputState whichState) const {
		
		if (whichPlayer == Players::NoPlayer || whichPlayer >= Players::MaxPlayerCount) {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
				if (GetNetworkMouseButtonState(player, whichButton, whichState)) {
					return true;
				}
			}
			return false;
		}

		switch (whichState) {
			case InputState::Held:
				return m_NetworkServerPreviousMouseButtonState[whichPlayer][whichButton];
			case InputState::Pressed:
				return m_NetworkServerPreviousMouseButtonState[whichPlayer][whichButton] && m_NetworkServerChangedMouseButtonState[whichPlayer][whichButton];
			case InputState::Released:
				return !m_NetworkServerPreviousMouseButtonState[whichPlayer][whichButton] && m_NetworkServerChangedMouseButtonState[whichPlayer][whichButton];
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const {
		if (whichJoy < 0 || whichJoy >= s_PrevJoystickStates.size() || whichButton < 0 || whichButton >= s_PrevJoystickStates[whichJoy].m_Buttons.size()) {
			return false;
		}

		bool buttonState = s_PrevJoystickStates[whichJoy].m_Buttons[whichButton];

		switch (whichState) {
			case InputState::Held:
				return buttonState;
			case InputState::Pressed:
				return buttonState && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
			case InputState::Released:
				return !buttonState && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
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
		int axisState = s_PrevJoystickStates[whichJoy].m_DigitalAxis[whichAxis];

		if (whichDir == JoyDirections::JOYDIR_ONE) {
			switch (whichState) {
				case InputState::Held:
					return axisState == -1;
				case InputState::Pressed:
					return axisState == -1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
				case InputState::Released:
					return axisState == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration");
					return false;
			}
		} else {
			switch (whichState) {
				case InputState::Held:
					return axisState == 1;
				case InputState::Pressed:
					return axisState == 1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
				case InputState::Released:
					return axisState == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
					return false;
			}
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::QueueInputEvent(const SDL_Event &inputEvent) {
		m_EventQueue.emplace_back(inputEvent);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Update() {
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;

		// TODO_MULTITHREAD
		// Handle this properly.
		// There should be an intelligent division between inputs and controller states.
		// Probably "stack" all UI updates into the controller for the *last* sim frame (so the state doesn't change mid-frame)
		// But then that introduces a frame of latency. Alternatively, just let it update midframe. 
		// Tbh having things update mid-frame hasn't hurt, unless it's deleting stuff or changing lists being looped through by draw
		std::fill(s_ChangedKeyStates.begin(), s_ChangedKeyStates.end(), false);
		std::fill(s_ChangedMouseButtonStates.begin(), s_ChangedMouseButtonStates.end(), false);
		for (Gamepad &gamepad : s_ChangedJoystickStates) {
			std::fill(gamepad.m_Buttons.begin(), gamepad.m_Buttons.end(), false);
			std::fill(gamepad.m_Axis.begin(), gamepad.m_Axis.end(), 0);
			std::fill(gamepad.m_DigitalAxis.begin(), gamepad.m_DigitalAxis.end(), 0);
		}

		if (IsInMultiplayerMode()) {
			ClearNetworkChangedState();
		}

		m_TextInput.clear();
		m_MouseWheelChange = 0;
		m_RawMouseMovement.Reset();

		SDL_Event inputEvent;
		for (std::vector<SDL_Event>::const_iterator eventIterator = m_EventQueue.begin(); eventIterator != m_EventQueue.end(); eventIterator++) {
			inputEvent = *eventIterator;

			switch (inputEvent.type) {
				case SDL_KEYUP:
				case SDL_KEYDOWN:
					s_ChangedKeyStates[inputEvent.key.keysym.scancode] = (inputEvent.key.state != s_PrevKeyStates[inputEvent.key.keysym.scancode]);
					s_PrevKeyStates[inputEvent.key.keysym.scancode] = inputEvent.key.state;
					break;
				case SDL_TEXTINPUT: {
					char input = inputEvent.text.text[0];
					size_t i = 0;
					while (input != 0 && i < 32) {
						++i;
						if (input <= 127) {
							m_TextInput += input;
						}
						input = inputEvent.text.text[i];
					}
					break;
				}
				case SDL_MOUSEMOTION:
					if (m_DisableMouseMoving) {
						break;
					}
					m_RawMouseMovement += Vector(static_cast<float>(inputEvent.motion.xrel), static_cast<float>(inputEvent.motion.yrel));
					m_AbsoluteMousePos.SetXY(static_cast<float>(inputEvent.motion.x), static_cast<float>(inputEvent.motion.y));
					if (g_WindowMan.FullyCoversAllDisplays()) {
						int windowPosX = 0;
						int windowPosY = 0;
						SDL_GetWindowPosition(SDL_GetWindowFromID(inputEvent.motion.windowID), &windowPosX, &windowPosY);
						Vector windowCoord(static_cast<float>(g_WindowMan.GetDisplayArrangementAbsOffsetX() + windowPosX), static_cast<float>(g_WindowMan.GetDisplayArrangementAbsOffsetY() + windowPosY));
						m_AbsoluteMousePos += windowCoord;
					}
					break;
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
					if (inputEvent.button.button > SDL_BUTTON_RIGHT) {
						break;
					}
					s_ChangedMouseButtonStates[inputEvent.button.button] = (static_cast<bool>(inputEvent.button.state) != s_PrevMouseButtonStates[inputEvent.button.button]);
					s_PrevMouseButtonStates[inputEvent.button.button] = static_cast<bool>(inputEvent.button.state);
					s_CurrentMouseButtonStates[inputEvent.button.button] = static_cast<bool>(inputEvent.button.state);
					break;
				case SDL_MOUSEWHEEL:
					m_MouseWheelChange += inputEvent.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? inputEvent.wheel.y : -inputEvent.wheel.y;
					break;
				case SDL_CONTROLLERAXISMOTION:
				case SDL_JOYAXISMOTION:
					if (std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), inputEvent.type == SDL_CONTROLLERAXISMOTION ? inputEvent.caxis.which : inputEvent.jaxis.which); device != s_PrevJoystickStates.end()) {
						if (SDL_IsGameController(device->m_DeviceIndex) && inputEvent.type == SDL_CONTROLLERAXISMOTION) {
							UpdateJoystickAxis(device, inputEvent.caxis.axis, inputEvent.caxis.value);
						} else if (!SDL_IsGameController(device->m_DeviceIndex)) {
							UpdateJoystickAxis(device, inputEvent.jaxis.axis, inputEvent.jaxis.value);
						}
					}
					break;
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					if (std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), (inputEvent.type == SDL_CONTROLLERBUTTONDOWN || inputEvent.type == SDL_CONTROLLERBUTTONUP) ? inputEvent.cbutton.which : inputEvent.jbutton.which);  device != s_PrevJoystickStates.end()) {
						int button = -1;
						int state = -1;
						if (SDL_IsGameController(device->m_DeviceIndex)) {
							if (inputEvent.type == SDL_CONTROLLERBUTTONUP || inputEvent.type == SDL_CONTROLLERBUTTONDOWN) {
								button = inputEvent.cbutton.button;
								state = inputEvent.cbutton.state;
							} else {
								break;
							}
						} else {
							button = inputEvent.jbutton.button;
							state = inputEvent.jbutton.state;
						}
						size_t index = device - s_PrevJoystickStates.begin();
						s_ChangedJoystickStates[index].m_Buttons[button] = state != device->m_Buttons[button];
						device->m_Buttons[button] = state;
					}
					break;
				case SDL_JOYDEVICEADDED:
					HandleGamepadHotPlug(inputEvent.jdevice.which);
					break;
				case SDL_JOYDEVICEREMOVED:
					if (std::vector<Gamepad>::iterator prevDevice = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), inputEvent.jdevice.which); prevDevice != s_PrevJoystickStates.end()) {
						g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(prevDevice->m_DeviceIndex + 1) + " disconnected!");
						SDL_GameControllerClose(SDL_GameControllerFromInstanceID(prevDevice->m_JoystickID));
						prevDevice->m_JoystickID = -1;
						std::fill(prevDevice->m_Axis.begin(), prevDevice->m_Axis.end(), 0);
						std::fill(prevDevice->m_Buttons.begin(), prevDevice->m_Buttons.end(), false);
						m_NumJoysticks--;
					}
					if (std::vector<Gamepad>::iterator changedDevice = std::find(s_ChangedJoystickStates.begin(), s_ChangedJoystickStates.end(), inputEvent.jdevice.which); changedDevice != s_ChangedJoystickStates.end()) {
						changedDevice->m_JoystickID = -1;
						std::fill(changedDevice->m_Axis.begin(), changedDevice->m_Axis.end(), false);
						std::fill(changedDevice->m_Buttons.begin(), changedDevice->m_Buttons.end(), false);
					}
					break;
				default:
					break;
			}

		}
		m_EventQueue.clear();
		m_RawMouseMovement *= m_MouseSensitivity;

		// NETWORK SERVER: Apply mouse input received from client or collect mouse input
		if (IsInMultiplayerMode()) {
			UpdateNetworkMouseMovement();
		} else {
			m_NetworkAccumulatedRawMouseMovement[Players::PlayerOne] += m_RawMouseMovement;
		}

		UpdateMouseInput();
		UpdateJoystickDigitalAxis();
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
				g_ActivityMan.PauseActivity(true, FlagShiftState());
				return;
			}
			// Ctrl+R or Back button for controllers to reset activity.
			if (!g_MetaMan.GameInProgress() && !g_ActivityMan.ActivitySetToRestart()) {
				g_ActivityMan.SetRestartActivity((FlagCtrlState() && KeyPressed(SDLK_r)) || AnyBackPress());
			}
			if (g_ActivityMan.ActivitySetToRestart()) {
				return;
			}
		}

		if (m_SkipHandlingSpecialInput) {
			return;
		}

		if (FlagCtrlState() && !FlagAltState()) {
			// Ctrl+S to save continuous ScreenDumps
			if (KeyHeld(SDLK_s)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_FrameMan.SaveScreenToPNG("ScreenDump"); });
			// Ctrl+W to save a WorldDump
			} else if (KeyPressed(SDLK_w)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_FrameMan.SaveWorldToPNG("WorldDump"); });
			// Ctrl+M to cycle draw modes
			} else if (KeyPressed(SDLK_m)) {
				g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3);
			// Ctrl+P to toggle performance stats
			} else if (KeyPressed(SDLK_p)) {
				g_PerformanceMan.ShowPerformanceStats(!g_PerformanceMan.IsShowingPerformanceStats());
			} else if (KeyPressed(SDLK_F2)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_PresetMan.QuickReloadEntityPreset(); });
			} else if (KeyPressed(SDLK_F9)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_ActivityMan.LoadAndLaunchGame("AutoSave"); });
			} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
				if (KeyHeld(SDLK_1)) {
					g_TimerMan.SetTimeScale(1.0F);
				} else if (KeyHeld(SDLK_2)) {
					g_TimerMan.SetTimeScale(99999.9F);
				} else if (KeyHeld(SDLK_5)) {
					g_TimerMan.SetDeltaTimeSecs(c_DefaultDeltaTimeS);
				}
			}
		} else if (!FlagCtrlState() && FlagAltState()) {
			if (KeyPressed(SDLK_F2)) {
				ContentFile::ReloadAllBitmaps();
			// Alt+Enter to switch resolution multiplier
			} else if (KeyPressed(SDLK_RETURN)) {
				g_WindowMan.ToggleFullscreen();
			// Alt+W to save ScenePreviewDump (miniature WorldDump)
			} else if (KeyPressed(SDLK_w)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_FrameMan.SaveWorldPreviewToPNG("ScenePreviewDump"); });
			} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
				if (KeyPressed(SDLK_p)) {
					g_PerformanceMan.ShowAdvancedPerformanceStats(!g_PerformanceMan.AdvancedPerformanceStatsEnabled());
				}
			}
		} else {
			if (KeyPressed(SDLK_F1)) {
				g_ConsoleMan.ShowShortcuts();
			} else if (KeyPressed(SDLK_F2)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_PresetMan.ReloadAllScripts(); });
			} else if (KeyPressed(SDLK_F3)) {
				g_ConsoleMan.SaveAllText("Console.dump.log");
			} else if (KeyPressed(SDLK_F4)) {
				g_ConsoleMan.SaveInputLog("Console.input.log");
			} else if (KeyPressed(SDLK_F5)) {
				if (g_ActivityMan.GetActivity() && g_ActivityMan.GetActivity()->CanBeUserSaved()) {
					g_ThreadMan.QueueInSimulationThread([]() { g_ActivityMan.SaveCurrentGame("QuickSave"); });
				} else {
					RTEError::ShowMessageBox("Cannot Save Game - This Activity Does Not Allow QuickSaving!");
				}
			} else if (KeyPressed(SDLK_F9)) {
				g_ThreadMan.QueueInSimulationThread([]() { g_ActivityMan.LoadAndLaunchGame("QuickSave"); });
			} else if (KeyPressed(SDLK_F10)) {
				g_ConsoleMan.ClearLog();
			// F12 to save a single ScreenDump - Note that F12 triggers a breakpoint when the VS debugger is attached, regardless of config - this is by design. Thanks Microsoft.
			} else if (KeyPressed(SDLK_F12)) {
				g_FrameMan.SaveScreenToPNG("ScreenDump");
			}

			if (g_PerformanceMan.IsShowingPerformanceStats()) {
				// Manipulate time scaling
				if (KeyHeld(SDLK_2)) {
					g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() + 0.01F);
				}
				if (KeyHeld(SDLK_1) && g_TimerMan.GetTimeScale() - 0.01F > 0.001F) {
					g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() - 0.01F);
				}

				// Manipulate DeltaTime
				if (KeyHeld(SDLK_6)) {
					g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() + 0.001F);
				}
				if (KeyHeld(SDLK_5) && g_TimerMan.GetDeltaTimeSecs() > 0) {
					g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() - 0.001F);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateMouseInput() {
		// Detect and store mouse movement input, translated to analog stick emulation
		int mousePlayer = MouseUsedByPlayer();
		// TODO: Figure out a less shit solution to updating the mouse in GUIs when there are no mouse players configured, i.e. no player input scheme is using mouse+keyboard. For not just check if we're out of Activity.
		if (!g_ActivityMan.IsInActivity() || mousePlayer != Players::NoPlayer) {
			m_AnalogMouseData.m_X += m_RawMouseMovement.m_X * 3;
			m_AnalogMouseData.m_Y += m_RawMouseMovement.m_Y * 3;
			m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius);

			// Only mess with the mouse pos if the original mouse position is not above the screen and may be grabbing the title bar of the game window
			if (g_WindowMan.AnyWindowHasFocus() && !IsInMultiplayerMode() && !m_DisableMouseMoving && !m_TrapMousePos) {
				// The mouse cursor is visible and can move about the screen/window, but it should still be contained within the mouse player's part of the window
				ForceMouseWithinPlayerScreen(g_ActivityMan.IsInActivity(), mousePlayer);
			}

			// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
			// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
			int mousePosX = m_AbsoluteMousePos.m_X / g_WindowMan.GetResMultiplier();
			int mousePosY = m_AbsoluteMousePos.m_Y / g_WindowMan.GetResMultiplier();
			if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mousePosX >= 0 && mousePosX < g_WindowMan.GetResX() && mousePosY >= 0 && mousePosY < g_WindowMan.GetResY())) {
				m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
			}
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	void UInputMan::UpdateJoystickAxis(std::vector<Gamepad>::iterator device, int axis, int value) {
		if (device != s_PrevJoystickStates.end()) {
			int joystickIndex = device - s_PrevJoystickStates.begin();

			int prevAxisValue = device->m_Axis[axis];

			s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(value - device->m_Axis[axis]);
			device->m_Axis[axis] = value;

			Players joystickPlayer = Players::NoPlayer;
			float deadZone = 0.0F;
			int deadZoneType = DeadZoneType::CIRCLE;

			for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; ++playerToCheck) {
				InputDevice playerInputDevice = m_ControlScheme[playerToCheck].GetDevice();
				int whichJoy = GetJoystickIndex(playerInputDevice);
				if (whichJoy == joystickIndex) {
					deadZone = m_ControlScheme[playerToCheck].GetJoystickDeadzone();
					deadZoneType = m_ControlScheme[playerToCheck].GetJoystickDeadzoneType();
					joystickPlayer = static_cast<Players>(playerToCheck);
				}
			}

			bool isAxisMapped = false;
			if (joystickPlayer != Players::NoPlayer && deadZone > 0.0F) {
				Vector aimValues;
				const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme[joystickPlayer].GetInputMappings();
				std::array<InputElements, 4> elementsToCheck = {InputElements::INPUT_L_LEFT, InputElements::INPUT_L_UP, InputElements::INPUT_R_LEFT, InputElements::INPUT_R_UP};

				for (size_t i = 0; i < elementsToCheck.size(); i += 2) {
					int axisLeft{SDL_CONTROLLER_AXIS_INVALID};
					int axisUp{SDL_CONTROLLER_AXIS_INVALID};
					if (inputElements->at(elementsToCheck[i]).JoyDirMapped()) {
						axisLeft = inputElements->at(elementsToCheck[i]).GetAxis();
						aimValues.m_X = AnalogAxisValue(joystickIndex, axisLeft);
						isAxisMapped = isAxisMapped || (axisLeft == axis);
					}
					if (inputElements->at(elementsToCheck[i + 1]).JoyDirMapped()) {
						axisUp = inputElements->at(elementsToCheck[i + 1]).GetAxis();
						aimValues.m_Y = AnalogAxisValue(joystickIndex, axisUp);
						isAxisMapped = isAxisMapped || (axisUp == axis);
					}
					if (!isAxisMapped) {
						continue;
					}

					if (deadZoneType == DeadZoneType::CIRCLE) {
						if (aimValues.MagnitudeIsLessThan(deadZone)) {
							if (axisLeft != SDL_CONTROLLER_AXIS_INVALID) {
								s_ChangedJoystickStates[joystickIndex].m_Axis[axisLeft] = Sign(axisLeft == axis ? -prevAxisValue : -device->m_Axis[axisLeft]);
								device->m_Axis[axisLeft] = 0;
							}
							if (axisUp != SDL_CONTROLLER_AXIS_INVALID) {
								s_ChangedJoystickStates[joystickIndex].m_Axis[axisUp] = Sign(axisUp == axis ? -prevAxisValue : -device->m_Axis[axisUp]);
								device->m_Axis[axisUp] = 0;
							}
						}
					} else if (deadZoneType == DeadZoneType::SQUARE && deadZone > 0.0F) {
						if (std::abs(static_cast<double>(value) / c_GamepadAxisLimit) < deadZone) {
							s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(-prevAxisValue);
							device->m_Axis[axis] = 0;
						}
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateJoystickDigitalAxis() {
		for (size_t i = 0; i < s_PrevJoystickStates.size(); ++i) {
			for (size_t axis = 0; axis < s_PrevJoystickStates[i].m_DigitalAxis.size(); ++axis) {
				int prevDigitalValue = s_PrevJoystickStates[i].m_DigitalAxis[axis];
				int newDigitalValue = 0;
				if (prevDigitalValue != 0 && std::abs(s_PrevJoystickStates[i].m_Axis[axis]) > c_AxisDigitalReleasedThreshold) {
					newDigitalValue = prevDigitalValue;
				}
				if (s_PrevJoystickStates[i].m_Axis[axis] > c_AxisDigitalPressedThreshold) {
					newDigitalValue = 1;
				} else if (s_PrevJoystickStates[i].m_Axis[axis] < -c_AxisDigitalPressedThreshold) {
					newDigitalValue = -1;
				}
				s_ChangedJoystickStates[i].m_DigitalAxis[axis] = Sign(newDigitalValue - prevDigitalValue);
				s_PrevJoystickStates[i].m_DigitalAxis[axis] = newDigitalValue;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::HandleGamepadHotPlug(int deviceIndex) {
		SDL_Joystick *controller = nullptr;
		int controllerIndex = 0;

		for (controllerIndex = 0; controllerIndex < s_PrevJoystickStates.size(); ++controllerIndex) {
			if (s_PrevJoystickStates[controllerIndex].m_DeviceIndex == deviceIndex || s_PrevJoystickStates[controllerIndex].m_DeviceIndex == -1) {
				if (SDL_IsGameController(deviceIndex)) {
					SDL_GameController *gameController = SDL_GameControllerOpen(deviceIndex);
					if (!gameController) {
						std::string connectString = s_PrevJoystickStates[controllerIndex].m_DeviceIndex == deviceIndex ? "reconnect" : "connect";
						g_ConsoleMan.PrintString("ERROR: Failed to " + connectString + " Gamepad " + std::to_string(controllerIndex + 1));
						break;
					}
					controller = SDL_GameControllerGetJoystick(gameController);
					SDL_GameControllerSetPlayerIndex(gameController, controllerIndex);
				} else {
					controller = SDL_JoystickOpen(deviceIndex);
				}
				if (!controller) {
					std::string connectString = s_PrevJoystickStates[controllerIndex].m_DeviceIndex == deviceIndex ? "reconnect" : "connect";
					g_ConsoleMan.PrintString("ERROR: Failed to " + connectString + " Gamepad " + std::to_string(controllerIndex + 1));
					break;
				}
				std::string connectString = s_PrevJoystickStates[controllerIndex].m_DeviceIndex == deviceIndex ? " reconnected" : " connected";
				g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(controllerIndex + 1) + connectString);
				break;
			}
		}

		if (!controller && controllerIndex == s_PrevJoystickStates.size()) {
			g_ConsoleMan.PrintString("ERROR: Too many gamepads connected!");
		}

		if (controller) {
			SDL_JoystickID id = SDL_JoystickInstanceID(controller);
			int numAxis = 0;
			int numButtons = 0;
			if (SDL_IsGameController(deviceIndex)) {
				numAxis = SDL_CONTROLLER_AXIS_MAX;
				numButtons = SDL_CONTROLLER_BUTTON_MAX;
			} else {
				numAxis = SDL_JoystickNumAxes(controller);
				numButtons = SDL_JoystickNumButtons(controller);
			}
			s_PrevJoystickStates[controllerIndex] = Gamepad(deviceIndex, id, numAxis, numButtons);
			s_ChangedJoystickStates[controllerIndex] = Gamepad(deviceIndex, id, numAxis, numButtons);
			m_NumJoysticks++;
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


			// Reset mouse wheel state to stop over-wheeling
			m_NetworkMouseWheelState[player] = 0;
		}
	}

	void UInputMan::ClearNetworkChangedState() {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
				for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
					m_NetworkServerChangedInputElementState[player][element] = false;
				}
				for (int mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
					m_NetworkServerChangedMouseButtonState[player][mouseButton] = false;
				}
				m_NetworkMouseWheelState[player] = 0;
			}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void UInputMan::SetNetworkInputElementState(int player, int element, bool newState) {
		if (element >= InputElements::INPUT_L_UP && element < InputElements::INPUT_COUNT && player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
			m_NetworkServerChangedInputElementState[player][element] = (newState != m_NetworkServerPreviousInputElementState[player][element]);
			m_NetworkServerPreviousInputElementState[player][element] = newState;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void UInputMan::SetNetworkMouseButtonState(int player, int whichButton, InputState whichState, bool newState) {
		if (whichButton >= MouseButtons::MOUSE_LEFT && whichButton < MouseButtons::MAX_MOUSE_BUTTONS && player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
			m_NetworkServerChangedMouseButtonState[player][whichButton] = (newState != m_NetworkServerPreviousMouseButtonState[player][whichButton]);
			m_NetworkServerPreviousMouseButtonState[player][whichButton] = newState;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void UInputMan::StoreInputEventsForNextUpdate() {
		// Store pressed and released events to be picked by NetworkClient during its update. These will be cleared after update so we don't care about false but we store the result regardless.
		for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++) {
			for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkAccumulatedElementState[element][inputState] = GetInputElementState(Players::PlayerOne, element, static_cast<InputState>(inputState));
			}
		}
	}




}