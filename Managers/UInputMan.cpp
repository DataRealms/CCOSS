#include "UInputMan.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PerformanceMan.h"
#include "GUIInput.h"
#include "Icon.h"

extern volatile bool g_Quit;
extern bool g_ResetActivity;
extern bool g_InActivity;
extern bool g_LaunchIntoEditor;

namespace RTE {

	const std::string UInputMan::c_ClassName = "UInputMan";
	GUIInput* UInputMan::s_InputClass = nullptr;

	char *UInputMan::s_PrevKeyStates = new char[KEY_MAX];
	char *UInputMan::s_ChangedKeyStates = new char[KEY_MAX];

	bool UInputMan::s_MouseButtonStates[MAX_MOUSE_BUTTONS];
	bool UInputMan::s_PrevMouseButtonStates[MAX_MOUSE_BUTTONS];
	bool UInputMan::s_ChangedMouseButtonStates[MAX_MOUSE_BUTTONS];

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

		// Reset mouse wheel and buttons
		position_mouse_z(0);
		s_MouseButtonStates[MOUSE_LEFT] = s_PrevMouseButtonStates[MOUSE_LEFT] = s_ChangedMouseButtonStates[MOUSE_LEFT] = false;
		s_MouseButtonStates[MOUSE_RIGHT] = s_PrevMouseButtonStates[MOUSE_RIGHT] = s_ChangedMouseButtonStates[MOUSE_RIGHT] = false;
		s_MouseButtonStates[MOUSE_MIDDLE] = s_PrevMouseButtonStates[MOUSE_MIDDLE] = s_ChangedMouseButtonStates[MOUSE_MIDDLE] = false;

		// Init the previous keys and joy buttons so they don't make it seem like things have changed and also neutralize the changed keys so that no Releases will be detected initially
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);
		std::fill_n(s_ChangedKeyStates, KEY_MAX, false);

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

		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ControlScheme[player].Reset();

			for (short element = 0; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkInputElementHeld[player][element] = false;
				m_NetworkInputElementPressed[player][element] = false;
				m_NetworkInputElementReleased[player][element] = false;
			}
			for (short mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
				m_NetworkMouseButtonHeldState[player][mouseButton] = false;
				m_NetworkMouseButtonPressedState[player][mouseButton] = false;
				m_NetworkMouseButtonReleasedState[player][mouseButton] = false;
			}
			m_NetworkAccumulatedRawMouseMovement[player].Reset();
			m_NetworkAnalogMoveData[player].Reset();
			m_NetworkMouseWheelState[player] = 0;	
			m_TrapMousePosPerPlayer[player] = false;
		}

		for (short element = 0; element < InputElements::INPUT_COUNT; element++) {
			m_NetworkAccumulatedElementPressed[element] = false;
			m_NetworkAccumulatedElementReleased[element] = false;
		}

		// Set up the default key mappings for each player
		m_ControlScheme[Players::PlayerOne].SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
		m_ControlScheme[Players::PlayerOne].SetPreset(InputPreset::PRESET_P1DEFAULT);
		m_ControlScheme[Players::PlayerTwo].SetDevice(InputDevice::DEVICE_KEYB_ONLY);
		m_ControlScheme[Players::PlayerTwo].SetPreset(InputPreset::PRESET_P2DEFAULT);
		m_ControlScheme[Players::PlayerThree].SetDevice(InputDevice::DEVICE_GAMEPAD_1);
		m_ControlScheme[Players::PlayerThree].SetPreset(InputPreset::PRESET_P3DEFAULT);
		m_ControlScheme[Players::PlayerFour].SetDevice(InputDevice::DEVICE_GAMEPAD_2);
		m_ControlScheme[Players::PlayerFour].SetPreset(InputPreset::PRESET_P4DEFAULT);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Create() {
		if (Serializable::Create() < 0) {
			return -1;
		}

		if (install_keyboard() != 0) { RTEAbort("Failed to initialize keyboard!"); }
		if (install_joystick(JOY_TYPE_AUTODETECT) != 0) { RTEAbort("Failed to initialize joysticks!"); }

		poll_joystick();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "MouseSensitivity") {
			reader >> m_MouseSensitivity;
		} else if (propName == "Player1Scheme" || propName == "Player2Scheme" || propName == "Player3Scheme" || propName == "Player4Scheme") {
			for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "Player" + playerNum + "Scheme") {
					reader >> m_ControlScheme[player];
					break;
				}
			}
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Save(Writer &writer) const {
		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Input Mapping", false);
		writer.NewLine(false);

		writer.NewProperty("MouseSensitivity");
		writer << m_MouseSensitivity;

		writer.NewLine(false);
		writer.NewLineString("// Input Devices:  0 = Keyboard Only, 1 = Mouse + Keyboard, 2 = Joystick One, 3 = Joystick Two, , 4 = Joystick Three, 5 = Joystick Four");
		writer.NewLineString("// Scheme Presets: 0 = No Preset, 1 = WASD, 2 = Cursor Keys, 3 = XBox 360 Controller");

		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player + 1);
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Player " + playerNum, false);
			writer.NewLine(false);
			writer.NewProperty("Player" + playerNum + "Scheme");
			writer << m_ControlScheme[player];
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::LoadDeviceIcons() {
		m_DeviceIcons[InputDevice::DEVICE_KEYB_ONLY] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
		m_DeviceIcons[InputDevice::DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));

		for (short gamepad = InputDevice::DEVICE_GAMEPAD_1; gamepad < InputDevice::DEVICE_COUNT; gamepad++) {
			std::string gamepadNum = std::to_string(gamepad - 1);
			m_DeviceIcons[gamepad] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad " + gamepadNum));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetInputClass(GUIInput* inputClass) const { s_InputClass = inputClass; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Icon * UInputMan::GetSchemeIcon(short whichPlayer) const {
		return (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) ? nullptr : m_DeviceIcons[m_ControlScheme[whichPlayer].GetDevice()];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string UInputMan::GetMappingName(short whichPlayer, int whichElement) {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
			return "";
		}

		InputPreset preset = m_ControlScheme[whichPlayer].GetPreset();
		const InputMapping *element = &m_ControlScheme[whichPlayer].GetInputMappings()[whichElement];
		if (preset != InputPreset::PRESET_NONE && !element->GetPresetDescription().empty()) {
			return element->GetPresetDescription();
		}
		
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);

			// Check joystick button presses and axis directions
			if (element->GetJoyButton() != JoyButtons::JOY_NONE) {
				return joy[whichJoy].button[element->GetJoyButton()].name;
			}
			if (element->JoyDirMapped()) {
				return "Joystick";
			}
		} else if (device == InputDevice::DEVICE_MOUSE_KEYB && element->GetMouseButton() != MouseButtons::MOUSE_NONE) {
			int button = element->GetMouseButton();

			switch (button) {
				case MouseButtons::MOUSE_LEFT:
					return "Left Mouse";
				case MouseButtons::MOUSE_RIGHT:
					return "Right Mouse";
				case MouseButtons::MOUSE_MIDDLE:
					return "Middle Mouse";
				default:
					return "";
			}
		} else if (device == InputDevice::DEVICE_KEYB_ONLY || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN)) && element->GetKey() != 0) {
			return scancode_to_name(element->GetKey());
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureKeyMapping(short whichPlayer, int whichInput) {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
			return false;
		}
		if (keyboard_needs_poll()) { poll_keyboard(); }

		for (char whichKey = 0; whichKey < KEY_MAX; ++whichKey) {
			if (KeyPressed(whichKey)) {
				// Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
				m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset();
				SetKeyMapping(whichPlayer, whichInput, whichKey);
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureButtonMapping(short whichPlayer, int whichJoy, int whichInput) {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
			return false;
		}
		int whichButton = WhichJoyButtonPressed(whichJoy);

		if (whichButton != JoyButtons::JOY_NONE) {
			// Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
			m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset();
			SetButtonMapping(whichPlayer, whichInput, whichButton);
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureDirectionMapping(short whichPlayer, int whichJoy, int whichInput) {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
			return false;
		}

		for (int stick = 0; stick < joy[whichJoy].num_sticks; ++stick) {
			for (int axis = 0; axis < joy[whichJoy].stick[stick].num_axis; ++axis) {
				if (joy[whichJoy].stick[stick].axis[axis].d1 && s_ChangedJoystickStates[whichJoy].stick[stick].axis[axis].d1) {
					// Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
					m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset();
					m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].SetDirection(stick, axis, JOYDIR_ONE);
					return true;
				} else if (joy[whichJoy].stick[stick].axis[axis].d2 && s_ChangedJoystickStates[whichJoy].stick[stick].axis[axis].d2) {
					m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset();
					m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].SetDirection(stick, axis, JOYDIR_TWO);
					return true;
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureJoystickMapping(short whichPlayer, int whichJoy, int whichInput) {
		if (CaptureButtonMapping(whichPlayer, whichJoy, whichInput)) {
			return true;
		}
		if (CaptureDirectionMapping(whichPlayer, whichJoy, whichInput)) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::AnalogMoveValues(short whichPlayer) {
		Vector moveValues(0,0);
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);

			const InputMapping *element = m_ControlScheme[whichPlayer].GetInputMappings();
			// Assume axes are stretched out over up-down, and left-right
			if (element[InputElements::INPUT_L_LEFT].JoyDirMapped()) { moveValues.SetX(AnalogAxisValue(whichJoy, element[InputElements::INPUT_L_LEFT].GetStick(), element[InputElements::INPUT_L_LEFT].GetAxis())); }
			if (element[InputElements::INPUT_L_UP].JoyDirMapped()) { moveValues.SetY(AnalogAxisValue(whichJoy, element[InputElements::INPUT_L_UP].GetStick(), element[InputElements::INPUT_L_UP].GetAxis())); }
		}
		return moveValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::AnalogAimValues(short whichPlayer) {
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();

		if (IsInMultiplayerMode()) { device = InputDevice::DEVICE_MOUSE_KEYB; }

		Vector aimValues(0,0);
		if (device == InputDevice::DEVICE_MOUSE_KEYB) {
			aimValues = (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) ? (m_NetworkAnalogMoveData[whichPlayer] / m_MouseTrapRadius) : (m_AnalogMouseData / m_MouseTrapRadius);
		} else if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);

			const InputMapping *element = m_ControlScheme[whichPlayer].GetInputMappings();
			// Assume axes are stretched out over up-down, and left-right
			if (element[InputElements::INPUT_R_LEFT].JoyDirMapped()) { aimValues.SetX(AnalogAxisValue(whichJoy, element[InputElements::INPUT_R_LEFT].GetStick(), element[InputElements::INPUT_R_LEFT].GetAxis())); }
			if (element[InputElements::INPUT_R_UP].JoyDirMapped()) { aimValues.SetY(AnalogAxisValue(whichJoy, element[InputElements::INPUT_R_UP].GetStick(), element[InputElements::INPUT_R_UP].GetAxis())); }
		}
		return aimValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementHeld(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementHeld[whichPlayer][whichElement] : false;
		}
		bool held = false;
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		if (device == InputDevice::DEVICE_KEYB_ONLY || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN))) {
			held = held ? true : KeyHeld(element->GetKey());
		}
		if (device == InputDevice::DEVICE_MOUSE_KEYB && m_TrapMousePos) {
			held = held ? true : MouseButtonHeld(element->GetMouseButton(), whichPlayer);
		}
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			held = held ? true : JoyButtonHeld(whichJoy, element->GetJoyButton());
			if (!held && element->JoyDirMapped()) { held = JoyDirectionHeld(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return held;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementPressed(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementPressed[whichPlayer][whichElement] : false;
		}
		bool pressed = false;
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		if (device == InputDevice::DEVICE_KEYB_ONLY || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN))) {
			pressed = pressed ? true : KeyPressed(element->GetKey());
		}
		if (device == InputDevice::DEVICE_MOUSE_KEYB && m_TrapMousePos) {
			pressed = pressed ? true : MouseButtonPressed(element->GetMouseButton(), whichPlayer);
		}
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			pressed = pressed ? true : JoyButtonPressed(whichJoy, element->GetJoyButton());
			if (!pressed && element->JoyDirMapped()) { pressed = JoyDirectionPressed(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementReleased(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementReleased[whichPlayer][whichElement] : false;
		}
		bool released = false;
		InputDevice device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		if (device == InputDevice::DEVICE_KEYB_ONLY || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN))) {
			released = released ? true : KeyReleased(element->GetKey());
		}
		if (device == InputDevice::DEVICE_MOUSE_KEYB && m_TrapMousePos) {
			released = released ? true : MouseButtonReleased(element->GetMouseButton(), whichPlayer);
		}
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int whichJoy = GetJoystickIndex(device);
			released = released ? true : JoyButtonReleased(whichJoy, element->GetJoyButton());
			if (!released && element->JoyDirMapped()) { released = JoyDirectionReleased(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return released;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetMenuDirectional() {
		Vector allInput(0,0);
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			InputDevice device = m_ControlScheme[player].GetDevice();

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

	bool UInputMan::MenuButtonHeld(int whichButton) {
		bool button = false;
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount && !button; ++player) {
			InputDevice device = m_ControlScheme[player].GetDevice();

			if (whichButton >= MenuCursorButtons::MENU_PRIMARY) { button = ElementHeld(player, InputElements::INPUT_FIRE) || MouseButtonHeld(MouseButtons::MOUSE_LEFT, player) || button; }
			if (whichButton >= MenuCursorButtons::MENU_SECONDARY) { button = ElementHeld(player, InputElements::INPUT_PIEMENU) || MouseButtonHeld(MouseButtons::MOUSE_RIGHT, player) || button; }

			if (button) {
				m_LastDeviceWhichControlledGUICursor = device;
				break;
			}
		}
		return button;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MenuButtonPressed(int whichButton) {
		bool button = false;
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount && !button; ++player) {
			InputDevice device = m_ControlScheme[player].GetDevice();

			if (whichButton >= MenuCursorButtons::MENU_PRIMARY) { button = ElementPressed(player, InputElements::INPUT_FIRE) || MouseButtonPressed(MouseButtons::MOUSE_LEFT, player) || button; }
			if (whichButton >= MenuCursorButtons::MENU_SECONDARY) { button = ElementPressed(player, InputElements::INPUT_PIEMENU) || MouseButtonPressed(MouseButtons::MOUSE_RIGHT, player) || button; }

			if (button) { m_LastDeviceWhichControlledGUICursor = device; }
		}
		return button;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MenuButtonReleased(int whichButton) {
		bool button = false;
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount && !button; ++player) {
			if (whichButton >= MenuCursorButtons::MENU_PRIMARY) { button = ElementReleased(player, InputElements::INPUT_FIRE) || MouseButtonReleased(MouseButtons::MOUSE_LEFT, player) || button; }
			if (whichButton >= MenuCursorButtons::MENU_SECONDARY) { button = ElementReleased(player, InputElements::INPUT_PIEMENU) || MouseButtonReleased(MouseButtons::MOUSE_RIGHT, player) || button; }
		}
		return button;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyKeyOrJoyInput() const {
		bool input = keypressed();
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

	bool UInputMan::AnyStartPress() {
		bool pressed = false;
		if (KeyPressed(KEY_ESC) || KeyPressed(KEY_SPACE)) { pressed = true; }

		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount && !pressed; ++player) {
			pressed = pressed || ElementPressed(player, InputElements::INPUT_START);
			pressed = pressed || ElementPressed(player, InputElements::INPUT_BACK);
		}
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::KeyPressed(const char keyToTest) const {
		if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) {
			return false;
		}
		return (s_InputClass) ? (s_InputClass->GetScanCodeState(keyToTest) == GUIInput::Pushed) : (s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest]);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::KeyReleased(const char keyToTest) const {
		if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) {
			return false;
		}
		return !s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyKeyPress() const {
		for (int testKey = 0; testKey < KEY_MAX; ++testKey) {
			if (s_PrevKeyStates[testKey] && s_ChangedKeyStates[testKey]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	short UInputMan::MouseUsedByPlayer() const {
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
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

	Vector UInputMan::GetMouseMovement(short whichPlayer) const {
		Vector mouseMovement(0,0);
		if (IsInMultiplayerMode() && whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount) {
			mouseMovement = m_NetworkAccumulatedRawMouseMovement[whichPlayer];
			return mouseMovement;
		}
		if (whichPlayer == Players::NoPlayer || m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) { mouseMovement = m_RawMouseMovement; }

		return mouseMovement;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMousePos(Vector &newPos, short whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB)) {
			position_mouse(static_cast<int>(newPos.GetX()), static_cast<int>(newPos.GetY()));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonHeld(int whichButton, short whichPlayer) const {
		if (whichButton < MouseButtons::MOUSE_LEFT || whichButton >= MouseButtons::MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
				for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (m_NetworkMouseButtonHeldState[player][whichButton]) {
						return m_NetworkMouseButtonHeldState[player][whichButton];
					}
				}
				return m_NetworkMouseButtonHeldState[Players::PlayerOne][whichButton];
			} else {
				return m_NetworkMouseButtonHeldState[whichPlayer][whichButton];
			}
		}
		return s_MouseButtonStates[whichButton];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonPressed(int whichButton, short whichPlayer) const {
		if (whichButton < MouseButtons::MOUSE_LEFT || whichButton >= MouseButtons::MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
				for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (m_NetworkMouseButtonPressedState[player][whichButton]) {
						return m_NetworkMouseButtonPressedState[player][whichButton];
					}
				}
				return m_NetworkMouseButtonPressedState[Players::PlayerOne][whichButton];
			} else {
				return m_NetworkMouseButtonPressedState[whichPlayer][whichButton];
			}
		}
		return s_MouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonReleased(int whichButton, short whichPlayer) const {
		if (whichButton < MouseButtons::MOUSE_LEFT || whichButton >= MouseButtons::MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) {
				for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (m_NetworkMouseButtonReleasedState[player][whichButton]) {
						return m_NetworkMouseButtonReleasedState[player][whichButton];
					}
				}
				return m_NetworkMouseButtonReleasedState[Players::PlayerOne][whichButton];
			} else {
				return m_NetworkMouseButtonReleasedState[whichPlayer][whichButton];
			}
		}
		return !s_MouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyMouseButtonPress() const {
		for (short button = MouseButtons::MOUSE_LEFT; button < MouseButtons::MAX_MOUSE_BUTTONS; ++button) {
			if (MouseButtonPressed(button, -1)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::TrapMousePos(bool trap, short whichPlayer) {
		if (whichPlayer == Players::NoPlayer || m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			m_TrapMousePos = trap;
		}
		m_TrapMousePosPerPlayer[whichPlayer] = trap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, short whichPlayer) const {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || m_ControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB)) {
			//int mouseX = std::max(x, const_cast<int &>(mouse_x));
			//int mouseY = std::max(y, const_cast<int &>(mouse_y));
			//mouseX = std::min(mouseX, x + width * g_FrameMan.ResolutionMultiplier());
			//mouseY = std::min(mouseY, y + height * g_FrameMan.ResolutionMultiplier());

			//position_mouse(mouseX, mouseY);

			int mouseX = const_cast<int &>(mouse_x);
			int mouseY = const_cast<int &>(mouse_y);

			position_mouse(Limit(mouseX, x + width * g_FrameMan.ResolutionMultiplier(), x), Limit(mouseY, y + height * g_FrameMan.ResolutionMultiplier(), y));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinPlayerScreen(short whichPlayer) const {
		if (whichPlayer < Players::PlayerOne || whichPlayer >= Players::PlayerFour) {
			return;
		}
		unsigned short screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer);
		unsigned short screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer);

		if (g_FrameMan.GetScreenCount() > 1) {
			switch (whichPlayer) {
				case Players::PlayerOne:
					// Always upper left corner
					ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
					break;
				case Players::PlayerTwo:
					// If both splits, or just V-Split, then in upper right quadrant
					if ((g_FrameMan.GetVSplit() && !g_FrameMan.GetHSplit()) || (g_FrameMan.GetVSplit() && g_FrameMan.GetHSplit())) {
						ForceMouseWithinBox(g_FrameMan.GetResX() / 2, 0, screenWidth, screenHeight, whichPlayer);
					} else {
						ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
					}
					break;
				case Players::PlayerThree:
					// Always lower left quadrant
					ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
					break;
				case Players::PlayerFour:
					// Always lower right quadrant
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

	bool UInputMan::JoystickActive(int joystickNumber) const {
		if (joystickNumber < Players::PlayerOne || joystickNumber >= Players::MaxPlayerCount) {
			return false;
		}
		if (joystickNumber < num_joysticks) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonHeld(int whichJoy, int whichButton) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return false;
		}
		if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons) {
			return false;
		}
		return joy[whichJoy].button[whichButton].b;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonHeld(int whichJoy) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return JoyButtons::JOY_NONE;
		}
		for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
			if (joy[whichJoy].button[button].b) {
				return button;
			}
		}
		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonPressed(int whichJoy, int whichButton) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return false;
		}
		if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons) {
			return false;
		}
		bool pressed = joy[whichJoy].button[whichButton].b && s_ChangedJoystickStates[whichJoy].button[whichButton].b;

		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonPressed(int whichJoy) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return JoyButtons::JOY_NONE;
		}
		for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
			if (joy[whichJoy].button[button].b && JoyButtonPressed(whichJoy, button)) {
				return button;
			}
		}
		// No button was pressed down
		return JoyButtons::JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonReleased(int whichJoy, int whichButton) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return false;
		}
		if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons) {
			return false;
		}
		bool released = !joy[whichJoy].button[whichButton].b && s_ChangedJoystickStates[whichJoy].button[whichButton].b;

		return released;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JoyDirections::JOYDIR_TWO) {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JoyDirections::JOYDIR_TWO) {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir) const {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JoyDirections::JOYDIR_TWO) {
			return !joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return !joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
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
			for (int button = 0; button < JoyButtons::MAX_JOY_BUTTONS; ++button) {
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
		for (int button = 0; button < JoyButtons::MAX_JOY_BUTTONS; ++button) {
			if (JoyButtonPressed(whichJoy, button)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetNetworkAccumulatedRawMouseMovement(short player) {
		Vector accumulatedMovement = m_NetworkAccumulatedRawMouseMovement[player];
		m_NetworkAccumulatedRawMouseMovement[player].Reset();
		return accumulatedMovement;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ClearNetworkAccumulatedStates() {
		for (int element = 0; element < INPUT_COUNT; element++) {
			m_NetworkAccumulatedElementPressed[element] = false;
			m_NetworkAccumulatedElementReleased[element] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Update() {
		m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;

		if (keyboard_needs_poll()) { poll_keyboard(); }
		if (mouse_needs_poll()) { poll_mouse(); }
		poll_joystick();

		for (char keyToCheck = 0; keyToCheck < KEY_MAX; ++keyToCheck) {
			s_ChangedKeyStates[keyToCheck] = key[keyToCheck] != s_PrevKeyStates[keyToCheck];
		}

		// Store mouse movement
		int mickeyX;
		int mickeyY;
		get_mouse_mickeys(&mickeyX, &mickeyY);
		m_RawMouseMovement.SetIntXY(mickeyX, mickeyY);

		// TODO: Have proper mouse sensitivity controls somewhere
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
		if (KeyPressed(KEY_ESC)) {
			// If we launched into editor directly, skip the logic and quit quickly.
			if (g_LaunchIntoEditor) { 
				g_Quit = true;
				return;
			}
			if (g_InActivity) {
				g_ActivityMan.PauseActivity();
			}
		}
		if (g_InActivity) {
			// Ctrl+R to reset Activity
			if (FlagCtrlState() && KeyPressed(KEY_R)) { g_ResetActivity = true; }

			// Check for resets and start button presses on controllers of all active players
			if (g_ActivityMan.GetActivity()) {
				for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
					if (g_ActivityMan.GetActivity()->PlayerActive(player)) {
						g_ResetActivity = g_ResetActivity || ElementPressed(Players::PlayerOne, INPUT_BACK);

						if (ElementPressed(player, INPUT_START)) {
							g_ActivityMan.PauseActivity();
						}
					}
				}
			}
			if (g_ResetActivity) { g_ConsoleMan.PrintString("SYSTEM: Activity was reset!"); }
		}

		if (FlagCtrlState() && !FlagAltState()) {
			// Ctrl+S to save continuous ScreenDumps
			if (KeyHeld(KEY_S)) {
				g_FrameMan.SaveScreenToBMP("ScreenDump");
			// Ctrl+W to save a WorldDump
			} else if (KeyPressed(KEY_W)) {
				g_FrameMan.SaveWorldToBMP("WorldDump");
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
				g_FrameMan.SwitchResolutionMultiplier((g_FrameMan.ResolutionMultiplier() >= 2) ? 1 : 2);
			// Alt+W to save ScenePreviewDump (miniature WorldDump)
			} else if (KeyPressed(KEY_W)) {
				g_FrameMan.SaveWorldToPreviewBMP("ScenePreviewDump");
			}
		} else {
			// PrntScren to save a single ScreenDump
			if (KeyPressed(KEY_PRTSCR)) {
				g_FrameMan.SaveScreenToBMP("ScreenDump");
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
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (m_NetworkAccumulatedRawMouseMovement[player].GetX() != 0 || m_NetworkAccumulatedRawMouseMovement[player].GetY() != 0) {
				Vector networkMouseMovement = m_NetworkAccumulatedRawMouseMovement[player];

				m_NetworkAnalogMoveData[player].m_X += networkMouseMovement.m_X * 3;
				m_NetworkAnalogMoveData[player].m_Y += networkMouseMovement.m_Y * 3;
				m_NetworkAnalogMoveData[player].CapMagnitude(m_MouseTrapRadius);
			}

			// By the time we reach here all events should've been processed by recipients during the last update.
			// We need to clear press and release event or otherwise it will look like player clicks buttons every frame till the next frame arrives after 33 ms with those states are off.

			// Clear pressed and released events as they should've been already processed during ActivityUpdate.
			// It is vital that press and release events are processed just once or multiple events will be triggered on a single press.
			for (short element = 0; element < InputElements::INPUT_COUNT; element++) {
				m_NetworkInputElementPressed[player][element] = false;
				m_NetworkInputElementReleased[player][element] = false;
			}
			m_NetworkAccumulatedRawMouseMovement[player].Reset();

			// Reset mouse button states to stop double clicking
			for (short mouseButton = MouseButtons::MOUSE_LEFT; mouseButton < MouseButtons::MAX_MOUSE_BUTTONS; mouseButton++) {
				m_NetworkMouseButtonPressedState[player][mouseButton] = false;
				m_NetworkMouseButtonReleasedState[player][mouseButton] = false;
			}

			// Reset mouse wheel state to stop over-wheeling
			m_NetworkMouseWheelState[player] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateMouseInput() {
		// Detect and store mouse button inputs and their changes from last frame
		s_MouseButtonStates[MOUSE_LEFT] = mouse_b & 1;
		s_MouseButtonStates[MOUSE_RIGHT] = mouse_b & 2;
		s_MouseButtonStates[MOUSE_MIDDLE] = mouse_b & 4;
		s_ChangedMouseButtonStates[MOUSE_LEFT] = s_MouseButtonStates[MOUSE_LEFT] != s_PrevMouseButtonStates[MOUSE_LEFT];
		s_ChangedMouseButtonStates[MOUSE_RIGHT] = s_MouseButtonStates[MOUSE_RIGHT] != s_PrevMouseButtonStates[MOUSE_RIGHT];
		s_ChangedMouseButtonStates[MOUSE_MIDDLE] = s_MouseButtonStates[MOUSE_MIDDLE] != s_PrevMouseButtonStates[MOUSE_MIDDLE];

		// Detect and store mouse movement input, translated to analog stick emulation
		short mousePlayer = MouseUsedByPlayer();
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
				} else if (g_InActivity) {
					// The mouse cursor is visible and can move about the screen/window, but it should still be contained within the mouse player's part of the window
					ForceMouseWithinPlayerScreen(mousePlayer);
				}
			}

			// Mouse wheel update, translate motion into discrete ticks
			if (std::abs(mouse_z) >= 1) {
				m_MouseWheelChange = mouse_z;
				position_mouse_z(0);
			} else {
				m_MouseWheelChange = 0;
			}

			// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
			// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
			int mousePosX = mouse_x / g_FrameMan.ResolutionMultiplier();
			int mousePosY = mouse_y / g_FrameMan.ResolutionMultiplier();
			if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mousePosX >= 0 && mousePosX < g_FrameMan.GetResX() && mousePosY >= 0 && mousePosY < g_FrameMan.GetResY())) {
				m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateJoystickInput() {
		// Detect and store joystick button changed since last Update()
		for (int joystick = 0; joystick < num_joysticks; ++joystick) {
			for (int button = 0; button < joy[joystick].num_buttons; ++button) {
				s_ChangedJoystickStates[joystick].button[button].b = joy[joystick].button[button].b != s_PrevJoystickStates[joystick].button[button].b;
			}

			int joystickPlayer = Players::NoPlayer;

			float deadZone = 0.0F;
			int deadZoneType = DeadZoneType::CIRCLE;
			for (short playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; playerToCheck++) {
				InputDevice device = m_ControlScheme[playerToCheck].GetDevice();
				int whichJoy = GetJoystickIndex(device);
				if (whichJoy == joystick) {
					deadZone = m_ControlScheme[playerToCheck].GetJoystickDeadzone();
					deadZoneType = m_ControlScheme[playerToCheck].GetJoystickDeadzoneType();
					joystickPlayer = playerToCheck;
					break;
				}
			}

			// Disable input if it's in circle deadzone
			if (joystickPlayer > Players::NoPlayer && deadZoneType == DeadZoneType::CIRCLE && deadZone > 0.0F) {
				const InputMapping *element = m_ControlScheme[joystickPlayer].GetInputMappings();
				Vector aimValues;

				// Left stick
				if (element[InputElements::INPUT_L_LEFT].JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(joystick, element[InputElements::INPUT_L_LEFT].GetStick(), element[InputElements::INPUT_L_LEFT].GetAxis()); }
				if (element[InputElements::INPUT_L_UP].JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(joystick, element[InputElements::INPUT_L_UP].GetStick(), element[InputElements::INPUT_L_UP].GetAxis()); }

				if (aimValues.GetMagnitude() < deadZone * 2) {
					if (element[InputElements::INPUT_L_LEFT].JoyDirMapped()) {
						if (joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].pos = 128;
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].pos = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_LEFT].GetStick()].axis[element[InputElements::INPUT_L_LEFT].GetAxis()].d2 = 0;
						}
					}
					if (element[InputElements::INPUT_L_UP].JoyDirMapped()) {
						if (joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].pos = 128;
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].pos = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_L_UP].GetStick()].axis[element[InputElements::INPUT_L_UP].GetAxis()].d2 = 0;
						}
					}
				}

				aimValues.Reset();

				// Right stick
				if (element[InputElements::INPUT_R_LEFT].JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(joystick, element[InputElements::INPUT_R_LEFT].GetStick(), element[InputElements::INPUT_R_LEFT].GetAxis()); }
				if (element[InputElements::INPUT_R_UP].JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(joystick, element[InputElements::INPUT_R_UP].GetStick(), element[InputElements::INPUT_R_UP].GetAxis()); }

				if (aimValues.GetMagnitude() < deadZone * 2) {
					if (element[InputElements::INPUT_R_LEFT].JoyDirMapped()) {
						if (joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].pos = 128;
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].pos = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_LEFT].GetStick()].axis[element[InputElements::INPUT_R_LEFT].GetAxis()].d2 = 0;
						}
					}
					if (element[InputElements::INPUT_R_UP].JoyDirMapped()) {
						if (joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].pos = 128;
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].pos = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[InputElements::INPUT_R_UP].GetStick()].axis[element[InputElements::INPUT_R_UP].GetAxis()].d2 = 0;
						}
					}
				}
			}

			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					// Adjust joystick values to eliminate values in deadzone
					if (joystickPlayer > Players::NoPlayer && deadZoneType == DeadZoneType::SQUARE && deadZone > 0.0F) {
						// This one heavily relies on AnalogAxiesValue method of processing joystick data.
						if (joy[joystick].stick[stick].flags & JOYFLAG_UNSIGNED) {
							if (std::abs(joy[joystick].stick[stick].axis[axis].pos - 128) > 0 && std::abs(joy[joystick].stick[stick].axis[axis].pos - 128) / 128 < deadZone) {
								joy[joystick].stick[stick].axis[axis].pos = 128;
								joy[joystick].stick[stick].axis[axis].d1 = 0;
								joy[joystick].stick[stick].axis[axis].d2 = 0;
							}
						} else {
							if (std::abs(joy[joystick].stick[stick].axis[axis].pos) > 0 && std::abs(joy[joystick].stick[stick].axis[axis].pos) / 128 < deadZone) {
								joy[joystick].stick[stick].axis[axis].pos = 0;
								joy[joystick].stick[stick].axis[axis].d1 = 0;
								joy[joystick].stick[stick].axis[axis].d2 = 0;
							}
						}
					}
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d1 = joy[joystick].stick[stick].axis[axis].d1 != s_PrevJoystickStates[joystick].stick[stick].axis[axis].d1;
					s_ChangedJoystickStates[joystick].stick[stick].axis[axis].d2 = joy[joystick].stick[stick].axis[axis].d2 != s_PrevJoystickStates[joystick].stick[stick].axis[axis].d2;
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::StoreInputEventsForNextUpdate() {
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);
		std::copy(std::begin(s_MouseButtonStates), std::end(s_MouseButtonStates), std::begin(s_PrevMouseButtonStates));

		for (int joystick = 0; joystick < num_joysticks; ++joystick) {
			std::memcpy(s_PrevJoystickStates, &joy[joystick], sizeof(joy[joystick]));
		}

		// Store pressed and released events to be picked by NetworkClient during it's on-timer update. These will be cleared after update so we don't care about false but we store the result regardless.
		for (short element = 0; element < InputElements::INPUT_COUNT; element++) {
			m_NetworkAccumulatedElementPressed[element] = ElementPressed(0, element);
			m_NetworkAccumulatedElementReleased[element] = ElementReleased(0, element);
		}
	}
}