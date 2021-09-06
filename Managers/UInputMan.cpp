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

#ifdef _WIN32
#include "joystickapi.h"
#endif

namespace RTE {

	GUIInput *UInputMan::s_GUIInputInstanceToCaptureKeyStateFrom = nullptr;

	char *UInputMan::s_PrevKeyStates = new char[KEY_MAX];
	char *UInputMan::s_ChangedKeyStates = new char[KEY_MAX];

	bool UInputMan::s_CurrentMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS];
	bool UInputMan::s_PrevMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS];
	bool UInputMan::s_ChangedMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS];

	JOYSTICK_INFO UInputMan::s_PrevJoystickStates[Players::MaxPlayerCount];
	JOYSTICK_INFO UInputMan::s_ChangedJoystickStates[Players::MaxPlayerCount];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::Clear() {
		m_OverrideInput = false;
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
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);
		std::fill_n(s_ChangedKeyStates, KEY_MAX, false);
		std::fill_n(s_CurrentMouseButtonStates, MouseButtons::MAX_MOUSE_BUTTONS, false);
		std::fill_n(s_PrevMouseButtonStates, MouseButtons::MAX_MOUSE_BUTTONS, false);
		std::fill_n(s_ChangedMouseButtonStates, MouseButtons::MAX_MOUSE_BUTTONS, false);
		position_mouse_z(0);

		// This has to use Players::MaxPlayerCount instead of num_joysticks, because the latter may not have been initialized yet
		for (int joystick = Players::PlayerOne; joystick < Players::MaxPlayerCount; ++joystick) {
			for (int button = 0; button < joy[joystick].num_buttons; ++button) {
				s_PrevJoystickStates[joystick].button[button].b = false;
				s_ChangedJoystickStates[joystick].button[button].b = false;
			}
			// Init stick-axis-direction change state detection data
			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					s_PrevJoystickStates[joystick].stick[stick].axis[axis].d1 = 0;
					s_PrevJoystickStates[joystick].stick[stick].axis[axis].d2 = 0;
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d1 = 0;
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d2 = 0;
				}
			}
		}

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ControlScheme.at(player).Reset();
			m_ControlScheme.at(player).ResetToPlayerDefaults(static_cast<Players>(player));

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
		if (install_keyboard() != 0) { RTEAbort("Failed to initialize keyboard!"); }

#ifdef _WIN32
		// JOY_TYPE_AUTODETECT is failing to select the correct joystick driver, so a dual analog ends up with a non-functional right stick and the triggers being treated as one instead.
		// This overrides the setting to force it to use the correct driver (without modifying AllegroConfig). Not sure why this is happening but appears to have started after updating Allegro.
		const char *overrideData = "[joystick]\njoytype=W32";
		override_config_data(overrideData, ustrsize(overrideData));
#endif

		if (install_joystick(JOY_TYPE_AUTODETECT) != 0) { RTEAbort("Failed to initialize joysticks!"); }
		poll_joystick();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::DetectJoystickHotPlug() const {
		// TODO: Add Linux implementation.
#ifdef _WIN32
		int numDevices = joyGetNumDevs();
		if (numDevices > 0) {
			static JOYINFOEX joyInfo = { sizeof(JOYINFOEX), JOY_RETURNBUTTONS };
			int numDetectedJoysticks = 0;

			for (int i = 0; i < numDevices; ++i) {
				if (joyGetPosEx(i, &joyInfo) == JOYERR_NOERROR) { numDetectedJoysticks++; }
			}
			if (numDetectedJoysticks != num_joysticks) {
				remove_joystick();
				if (numDetectedJoysticks > 0 && install_joystick(JOY_TYPE_AUTODETECT) != 0) { RTEAbort("Failed to initialize joysticks!"); }
				return true;
			}
		}
#endif
		return false;
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
			if (inputElements->at(InputElements::INPUT_L_LEFT).JoyDirMapped()) { moveValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_LEFT).GetStick(), inputElements->at(InputElements::INPUT_L_LEFT).GetAxis())); }
			if (inputElements->at(InputElements::INPUT_L_UP).JoyDirMapped()) { moveValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_UP).GetStick(), inputElements->at(InputElements::INPUT_L_UP).GetAxis())); }
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
			if (inputElements->at(InputElements::INPUT_R_LEFT).JoyDirMapped()) { aimValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_LEFT).GetStick(), inputElements->at(InputElements::INPUT_R_LEFT).GetAxis())); }
			if (inputElements->at(InputElements::INPUT_R_UP).JoyDirMapped()) { aimValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_UP).GetStick(), inputElements->at(InputElements::INPUT_R_UP).GetAxis())); }
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
		if (KeyPressed(KEY_ESC) || (includeSpacebar && KeyPressed(KEY_SPACE))) {
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
		for (char testKey = KEY_A; testKey < KEY_MAX; ++testKey) {
			if (s_PrevKeyStates[testKey] && s_ChangedKeyStates[testKey]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::MouseUsedByPlayer() const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (m_ControlScheme.at(player).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				return player;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::DisableMouseMoving(bool disable) {
		if (disable) {
			m_DisableMouseMoving = true;
			m_PrepareToEnableMouseMoving = false;
			// Set these to outside the screen so the mouse has to be updated first before checking if they're in the screen or not
			mouse_x = mouse_y = -1;
		} else {
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
		m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius * magCap);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMousePos(Vector &newPos, int whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			position_mouse(static_cast<int>(newPos.GetX()), static_cast<int>(newPos.GetY()));
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
			position_mouse(Limit(mouse_x, x + width * g_FrameMan.GetResMultiplier(), x), Limit(mouse_y, y + height * g_FrameMan.GetResMultiplier(), y));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinPlayerScreen(int whichPlayer) const {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::PlayerFour) {
			return;
		}
		unsigned int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer);
		unsigned int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer);

		if (g_FrameMan.GetScreenCount() > 1) {
			switch (whichPlayer) {
				case Players::PlayerOne:
					ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
					break;
				case Players::PlayerTwo:
					if ((g_FrameMan.GetVSplit() && !g_FrameMan.GetHSplit()) || (g_FrameMan.GetVSplit() && g_FrameMan.GetHSplit())) {
						ForceMouseWithinBox(g_FrameMan.GetResX() / 2, 0, screenWidth, screenHeight, whichPlayer);
					} else {
						ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
					}
					break;
				case Players::PlayerThree:
					ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
					break;
				case Players::PlayerFour:
					ForceMouseWithinBox(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
					break;
				default:
					RTEAbort("Undefined player value passed in. See Players enumeration for defined values.")
			}
		} else {
			ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonHeld(int whichJoy) const {
		if (whichJoy >= 0 || whichJoy < num_joysticks) {
			for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
				if (joy[whichJoy].button[button].b) {
					return button;
				}
			}
		}
		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonPressed(int whichJoy) const {
		if (whichJoy >= 0 || whichJoy < num_joysticks) {
			for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
				if (joy[whichJoy].button[button].b && JoyButtonPressed(whichJoy, button)) {
					return button;
				}
			}
		}
		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float UInputMan::AnalogAxisValue(int whichJoy, int whichStick, int whichAxis) const {
		if (whichJoy < num_joysticks && whichStick < joy[whichJoy].num_sticks && whichAxis < joy[whichJoy].stick[whichStick].num_axis) {
			if (joy[whichJoy].stick[whichStick].flags & JOYFLAG_UNSIGNED) {
				// Treat unsigned (throttle axes) as rudders, with a range of 0-255 and midpoint of 128
				return static_cast<float>(joy[whichJoy].stick[whichStick].axis[whichAxis].pos - 128) / 128.0F;
			} else {
				// Regular signed axis with range of -128 to 128
				return static_cast<float>(joy[whichJoy].stick[whichStick].axis[whichAxis].pos) / 128.0F;
			}
		}
		return  0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyInput(bool checkForPresses) const {
		poll_joystick();

		for (int joystick = Players::PlayerOne; joystick < Players::MaxPlayerCount; ++joystick) {
			for (int button = JoyButtons::JOY_1; button < JoyButtons::MAX_JOY_BUTTONS; ++button) {
				if (!checkForPresses) {
					if (joy[joystick].button[button].b) {
						return true;
					}
				} else {
					if (JoyButtonPressed(joystick, button)) {
						return true;
					}
				}
			}
			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					if (!checkForPresses){
						if (JoyDirectionHeld(joystick, stick, axis, JoyDirections::JOYDIR_ONE) || JoyDirectionHeld(joystick, stick, axis, JoyDirections::JOYDIR_TWO)) {
							return true;
						}
					} else {
						if (JoyDirectionPressed(joystick, stick, axis, JoyDirections::JOYDIR_ONE) || JoyDirectionPressed(joystick, stick, axis, JoyDirections::JOYDIR_TWO)) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyButtonPress(int whichJoy) const {
		for (int button = JoyButtons::JOY_1; button < JoyButtons::MAX_JOY_BUTTONS; ++button) {
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
			elementState = GetKeyboardButtonState(static_cast<char>(element->GetKey()),whichState);
		}
		if (!elementState && device == InputDevice::DEVICE_MOUSE_KEYB && m_TrapMousePos) { elementState = GetMouseButtonState(whichPlayer, element->GetMouseButton(), whichState); }

		if (!elementState && device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			elementState = GetJoystickButtonState(whichJoy, element->GetJoyButton(), whichState);
			if (!elementState && element->JoyDirMapped()) { elementState = GetJoystickDirectionState(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection(), whichState); }
		}
		return elementState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetMenuButtonState(int whichButton, InputState whichState) {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			bool buttonState = false;
			InputDevice device = m_ControlScheme.at(player).GetDevice();
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

	bool UInputMan::GetKeyboardButtonState(const char keyToTest, InputState whichState) const {
		if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) {
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

	bool UInputMan::GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichButton < 0 || whichButton >= joy[whichJoy].num_buttons) {
			return false;
		}
		switch (whichState) {
			case InputState::Held:
				return joy[whichJoy].button[whichButton].b;
			case InputState::Pressed:
				return joy[whichJoy].button[whichButton].b && s_ChangedJoystickStates[whichJoy].button[whichButton].b;
			case InputState::Released:
				return !joy[whichJoy].button[whichButton].b && s_ChangedJoystickStates[whichJoy].button[whichButton].b;
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::GetJoystickDirectionState(int whichJoy, int whichStick, int whichAxis, int whichDir, InputState whichState) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		const JOYSTICK_AXIS_INFO *joystickAxis = &joy[whichJoy].stick[whichStick].axis[whichAxis];

		if (whichDir == JoyDirections::JOYDIR_ONE) {
			switch (whichState) {
				case InputState::Held:
					return joystickAxis->d1;
				case InputState::Pressed:
					return joystickAxis->d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
				case InputState::Released:
					return !joystickAxis->d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
					return false;
			}
		} else {
			switch (whichState) {
				case InputState::Held:
					return joystickAxis->d2;
				case InputState::Pressed:
					return joystickAxis->d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
				case InputState::Released:
					return !joystickAxis->d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
				default:
					RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
					return false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Update() {
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;

		if (keyboard_needs_poll()) { poll_keyboard(); }
		if (mouse_needs_poll()) { poll_mouse(); }
		poll_joystick();

		for (char keyToCheck = KEY_A; keyToCheck < KEY_MAX; ++keyToCheck) {
			s_ChangedKeyStates[keyToCheck] = key[keyToCheck] != s_PrevKeyStates[keyToCheck];
		}

		// Store mouse movement
		int mickeyX;
		int mickeyY;
		get_mouse_mickeys(&mickeyX, &mickeyY);
		m_RawMouseMovement.SetXY(mickeyX, mickeyY);

		// TODO: Add sensitivity slider to settings menu
		m_RawMouseMovement *= m_MouseSensitivity;

		// NETWORK SERVER: Apply mouse input received from client or collect mouse input
		if (IsInMultiplayerMode()) {
			UpdateNetworkMouseMovement();
		} else {
			m_NetworkAccumulatedRawMouseMovement[Players::PlayerOne] += m_RawMouseMovement;
		}

		UpdateMouseInput();
		if (num_joysticks > 0) { UpdateJoystickInput(); }
		HandleSpecialInput();
		StoreInputEventsForNextUpdate();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::HandleSpecialInput() {
		// If we launched into editor directly, skip the logic and quit quickly.
		if (g_ActivityMan.IsSetToLaunchIntoEditor() && KeyPressed(KEY_ESC)) {
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
			if (!g_MetaMan.GameInProgress() && !g_ActivityMan.ActivitySetToRestart()) { g_ActivityMan.SetRestartActivity(FlagCtrlState() && KeyPressed(KEY_R) || AnyBackPress()); }
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
			if (KeyHeld(KEY_S)) {
				g_FrameMan.SaveScreenToPNG("ScreenDump");
			// Ctrl+W to save a WorldDump
			} else if (KeyPressed(KEY_W)) {
				g_FrameMan.SaveWorldToPNG("WorldDump");
			// Ctrl+M to cycle draw modes
			} else if (KeyPressed(KEY_M)) {
				g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3);
			// Ctrl+P to toggle performance stats
			} else if (KeyPressed(KEY_P)) {
				g_PerformanceMan.ShowPerformanceStats(!g_PerformanceMan.IsShowingPerformanceStats());
			// Ctrl+O to toggle one sim update per frame
			} else if (KeyPressed(KEY_O)) {
				g_TimerMan.SetOneSimUpdatePerFrame(!g_TimerMan.IsOneSimUpdatePerFrame());
			}
		} else if (!FlagCtrlState() && FlagAltState()) {
			// Alt+Enter to switch resolution multiplier
			if (KeyPressed(KEY_ENTER)) {
				g_FrameMan.ChangeResolutionMultiplier((g_FrameMan.GetResMultiplier() >= 2) ? 1 : 2);
			// Alt+W to save ScenePreviewDump (miniature WorldDump)
			} else if (KeyPressed(KEY_W)) {
				g_FrameMan.SaveWorldPreviewToPNG("ScenePreviewDump");
			}
		} else {
			// PrntScren to save a single ScreenDump
			if (KeyPressed(KEY_PRTSCR)) {
				g_FrameMan.SaveScreenToPNG("ScreenDump");
			} else if (KeyPressed(KEY_F1)) {
				g_ConsoleMan.ShowShortcuts();
			} else if (KeyPressed(KEY_F2)) {
				g_PresetMan.ReloadAllScripts();
				g_ConsoleMan.PrintString("SYSTEM: Scripts reloaded!");
			} else if (KeyPressed(KEY_F3)) {
				g_ConsoleMan.SaveAllText("Console.dump.log");
			} else if (KeyPressed(KEY_F4)) {
				g_ConsoleMan.SaveInputLog("Console.input.log");
			} else if (KeyPressed(KEY_F5)) {
				g_ConsoleMan.ClearLog();
			}

			if (g_PerformanceMan.IsShowingPerformanceStats()) {
				// Manipulate time scaling
				if (KeyHeld(KEY_2)) { g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() + 0.01F); }
				if (KeyHeld(KEY_1) && g_TimerMan.GetTimeScale() - 0.01F > 0.001F) { g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() - 0.01F); }

				// Manipulate real to sim cap
				if (KeyHeld(KEY_4)) { g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() + 0.001F); }
				if (KeyHeld(KEY_3) && g_TimerMan.GetRealToSimCap() > 0) { g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() - 0.001F); }

				// Manipulate DeltaTime
				if (KeyHeld(KEY_6)) { g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() + 0.001F); }
				if (KeyHeld(KEY_5) && g_TimerMan.GetDeltaTimeSecs() > 0) { g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() - 0.001F); }
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
		// Detect and store mouse button inputs and their changes from last frame
		s_CurrentMouseButtonStates[MouseButtons::MOUSE_LEFT] = mouse_b & 1;
		s_CurrentMouseButtonStates[MouseButtons::MOUSE_RIGHT] = mouse_b & 2;
		s_CurrentMouseButtonStates[MouseButtons::MOUSE_MIDDLE] = mouse_b & 4;
		for (int mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
			s_ChangedMouseButtonStates[mouseButton] = s_CurrentMouseButtonStates[mouseButton] != s_PrevMouseButtonStates[mouseButton];
		}

		// Detect and store mouse movement input, translated to analog stick emulation
		int mousePlayer = MouseUsedByPlayer();
		if (mousePlayer != Players::NoPlayer) {
			// TODO: Figure out why we're multiplying by 3 here. Possibly related to mouse sensitivity.
			m_AnalogMouseData.m_X += m_RawMouseMovement.m_X * 3;
			m_AnalogMouseData.m_Y += m_RawMouseMovement.m_Y * 3;
			m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius);

			// Only mess with the mouse pos if the original mouse position is not above the screen and may be grabbing the title bar of the game window
			if (!m_DisableMouseMoving && !IsInMultiplayerMode()) {
				if (m_TrapMousePos) {
					// Trap the (invisible) mouse cursor in the middle of the screen, so it doesn't fly out in windowed mode and some other window gets clicked
					position_mouse(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
				} else if (g_ActivityMan.IsInActivity()) {
					// The mouse cursor is visible and can move about the screen/window, but it should still be contained within the mouse player's part of the window
					ForceMouseWithinPlayerScreen(mousePlayer);
				}
			}

			// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
			// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
			int mousePosX = mouse_x / g_FrameMan.GetResMultiplier();
			int mousePosY = mouse_y / g_FrameMan.GetResMultiplier();
			if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mousePosX >= 0 && mousePosX < g_FrameMan.GetResX() && mousePosY >= 0 && mousePosY < g_FrameMan.GetResY())) {
				m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
			}
		}
		if (mousePlayer != Players::NoPlayer || !g_ActivityMan.IsInActivity()) {
			// Mouse wheel update happens while a device is kb+mouse and while in the menus regardless of player devices.
			// Translate motion into discrete ticks.
			if (std::abs(mouse_z) >= 1) {
				m_MouseWheelChange = mouse_z;
				position_mouse_z(0);
			} else {
				m_MouseWheelChange = 0;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateJoystickInput() {
		for (int joystick = 0; joystick < num_joysticks; ++joystick) {
			// Detect and store joystick button changed since last Update()
			for (int button = 0; button < joy[joystick].num_buttons; ++button) {
				s_ChangedJoystickStates[joystick].button[button].b = joy[joystick].button[button].b != s_PrevJoystickStates[joystick].button[button].b;
			}

			Players joystickPlayer = Players::NoPlayer;

			float deadZone = 0.0F;
			int deadZoneType = DeadZoneType::CIRCLE;
			for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; playerToCheck++) {
				InputDevice device = m_ControlScheme.at(playerToCheck).GetDevice();
				int whichJoy = GetJoystickIndex(device);
				if (whichJoy == joystick) {
					deadZone = m_ControlScheme.at(playerToCheck).GetJoystickDeadzone();
					deadZoneType = m_ControlScheme.at(playerToCheck).GetJoystickDeadzoneType();
					joystickPlayer = static_cast<Players>(playerToCheck);
					break;
				}
			}
			if (joystickPlayer > Players::NoPlayer && deadZoneType == DeadZoneType::CIRCLE && deadZone > 0.0F) {
				Vector aimValues;
				const std::array<InputMapping, InputElements::INPUT_COUNT> *inputElements = m_ControlScheme.at(joystickPlayer).GetInputMappings();
				std::array<InputElements, 4> elementsToCheck = { InputElements::INPUT_L_LEFT, InputElements::INPUT_L_UP, InputElements::INPUT_R_LEFT, InputElements::INPUT_R_UP };

				for (int i = 0; i < elementsToCheck.size() - 1; i += 2) {
					if (inputElements->at(elementsToCheck.at(i)).JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(joystick, inputElements->at(elementsToCheck.at(i)).GetStick(), inputElements->at(elementsToCheck.at(i)).GetAxis()); }
					if (inputElements->at(elementsToCheck.at(i + 1)).JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(joystick, inputElements->at(elementsToCheck.at(i + 1)).GetStick(), inputElements->at(elementsToCheck.at(i + 1)).GetAxis()); }

					if (aimValues.GetMagnitude() < deadZone * 2) {
						for (int j = 0; j < 2; j++) {
							InputElements whichElementDirection = elementsToCheck[i + j];
							if (inputElements->at(whichElementDirection).JoyDirMapped()) {
								JOYSTICK_AXIS_INFO *joystickAxis = &joy[joystick].stick[inputElements->at(whichElementDirection).GetStick()].axis[inputElements->at(whichElementDirection).GetAxis()];
								if (joy[joystick].stick[inputElements->at(whichElementDirection).GetStick()].flags & JOYFLAG_UNSIGNED) {
									joystickAxis->pos = 128;
									joystickAxis->d1 = joystickAxis->d2 = 0;
								} else {
									joystickAxis->pos = joystickAxis->d1 = joystickAxis->d2 = 0;
								}
							}
						}
					}
					aimValues.Reset();
				}
			}
			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					JOYSTICK_AXIS_INFO *joystickAxis = &joy[joystick].stick[stick].axis[axis];

					if (joystickPlayer > Players::NoPlayer && deadZoneType == DeadZoneType::SQUARE && deadZone > 0.0F) {
						// Adjust joystick values to eliminate values in deadzone. This heavily relies on AnalogAxisValue method of processing joystick data.
						if (joy[joystick].stick[stick].flags & JOYFLAG_UNSIGNED) {
							if (std::abs(joystickAxis->pos - 128) > 0 && std::fabs(joystickAxis->pos - 128) / 128 < deadZone) {
								joystickAxis->pos = 128;
								joystickAxis->d1 = joystickAxis->d2 = 0;
							}
						} else {
							if (std::abs(joystickAxis->pos) > 0 && std::fabs(joystickAxis->pos) / 128 < deadZone) { joystickAxis->pos = joystickAxis->d1 = joystickAxis->d2 = 0; }
						}
					}
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d1 = joystickAxis->d1 != s_PrevJoystickStates[joystick].stick[stick].axis[axis].d1;
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d2 = joystickAxis->d2 != s_PrevJoystickStates[joystick].stick[stick].axis[axis].d2;
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::StoreInputEventsForNextUpdate() {
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);
		std::memcpy(s_PrevMouseButtonStates, s_CurrentMouseButtonStates, MouseButtons::MAX_MOUSE_BUTTONS);
		for (int joystick = 0; joystick < num_joysticks; ++joystick) {
			std::memcpy(s_PrevJoystickStates, &joy[joystick], sizeof(joy[joystick]));
		}
		// Store pressed and released events to be picked by NetworkClient during it's update. These will be cleared after update so we don't care about false but we store the result regardless.
		for (int inputState = InputState::Pressed; inputState < InputState::InputStateCount; inputState++){
			for (int element = InputElements::INPUT_L_UP; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkAccumulatedElementState[element][inputState] = GetInputElementState(Players::PlayerOne, element, static_cast<InputState>(inputState));
			}
		}
	}
}
