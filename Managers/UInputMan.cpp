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
	GUIInput* UInputMan::s_InputClass = NULL;

	char *UInputMan::s_PrevKeyStates = new char[KEY_MAX];
	char *UInputMan::s_ChangedKeyStates = new char[KEY_MAX];

	bool UInputMan::s_MouseButtonStates[MAX_MOUSE_BUTTONS];
	bool UInputMan::s_PrevMouseButtonStates[MAX_MOUSE_BUTTONS];
	bool UInputMan::s_ChangedMouseButtonStates[MAX_MOUSE_BUTTONS];

	JOYSTICK_INFO UInputMan::s_PrevJoystickStates[MAX_PLAYERS];
	JOYSTICK_INFO UInputMan::s_ChangedJoystickStates[MAX_PLAYERS];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::Clear() {
		// Init the previous keys so they don't make it seem like things have changed
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);

		// Neutralize the changed keys so that no Releases will be detected initially
		for (int i = 0; i < KEY_MAX; ++i) {
			s_ChangedKeyStates[i] = false;
		}
		m_DisableKeyboard = false;

		m_OverrideInput = false;
		m_LastDeviceWhichControlledGUICursor = 0;

		// Reset mouse button data
		s_MouseButtonStates[MOUSE_LEFT] = s_PrevMouseButtonStates[MOUSE_LEFT] = s_ChangedMouseButtonStates[MOUSE_LEFT] = false;
		s_MouseButtonStates[MOUSE_RIGHT] = s_PrevMouseButtonStates[MOUSE_RIGHT] = s_ChangedMouseButtonStates[MOUSE_RIGHT] = false;
		s_MouseButtonStates[MOUSE_MIDDLE] = s_PrevMouseButtonStates[MOUSE_MIDDLE] = s_ChangedMouseButtonStates[MOUSE_MIDDLE] = false;

		// Mouse wheel init
		position_mouse_z(0);
		m_MouseWheelChange = 0;

		// Set Mouse analog stick emulation data defaults
		m_RawMouseMovement.Reset();
		m_AnalogMouseData.Reset();
		m_TrapMousePos = false;
		m_MouseTrapRadius = 350;
		m_MouseSensitivity = 0.6F;

		m_DisableMouseMoving = false;
		m_PrepareToEnableMouseMoving = false;

		// Reset all schemes
		for (int player = 0; player < MAX_PLAYERS; ++player) {
			m_ControlScheme[player].Reset();
			m_NetworkAccumulatedRawMouseMovement[player].Reset();

			for (int element = 0; element < INPUT_COUNT; element++) {
				m_NetworkInputElementHeld[player][element] = false;
				m_NetworkInputElementPressed[player][element] = false;
				m_NetworkInputElementReleased[player][element] = false;
			}
			for (int mouseButton = 0; mouseButton < MAX_MOUSE_BUTTONS; mouseButton++) {
				m_NetworkMouseButtonHeldState[player][mouseButton] = false;
				m_NetworkMouseButtonPressedState[player][mouseButton] = false;
				m_NetworkMouseButtonReleasedState[player][mouseButton] = false;
			}
			m_NetworkAnalogMoveData[player].Reset();
			m_TrapMousePosPerPlayer[player] = false;
		}
		for (int element = 0; element < INPUT_COUNT; element++) {
			m_NetworkAccumulatedElementPressed[element] = false;
			m_NetworkAccumulatedElementReleased[element] = false;
		}
		for (int device = 0; device < DEVICE_COUNT; device++) {
			m_DeviceIcons[device] = 0;
		}

		// Init the previous joy buttons so they don't make it seem like things have changed. Also neutralize the changed keys so that no Releases will be detected initially.
		// This has to use MAX_PLAYERS instead of num_joysticks, because the latter may not have been initialized yet
		for (int joystick = 0; joystick < MAX_PLAYERS; ++joystick) {
			// Init button change state detection data
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

		// Set up the default key mappings for each player
		m_ControlScheme[PLAYER_ONE].SetDevice(DEVICE_MOUSE_KEYB);
		m_ControlScheme[PLAYER_ONE].SetPreset(PRESET_P1DEFAULT);
		m_ControlScheme[PLAYER_TWO].SetDevice(DEVICE_KEYB_ONLY);
		m_ControlScheme[PLAYER_TWO].SetPreset(PRESET_P2DEFAULT);
		m_ControlScheme[PLAYER_THREE].SetDevice(DEVICE_GAMEPAD_1);
		m_ControlScheme[PLAYER_THREE].SetPreset(PRESET_P3DEFAULT);
		m_ControlScheme[PLAYER_FOUR].SetDevice(DEVICE_GAMEPAD_2);
		m_ControlScheme[PLAYER_FOUR].SetPreset(PRESET_P4DEFAULT);
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
		} else if (propName == "P1Scheme") {
			reader >> m_ControlScheme[PLAYER_ONE];
		} else if (propName == "P2Scheme") {
			reader >> m_ControlScheme[PLAYER_TWO];
		} else if (propName == "P3Scheme") {
			reader >> m_ControlScheme[PLAYER_THREE];
		} else if (propName == "P4Scheme") {
			reader >> m_ControlScheme[PLAYER_FOUR];
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

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Player 1", false);
		writer.NewLine(false);
		writer.NewProperty("P1Scheme");
		writer << m_ControlScheme[PLAYER_ONE];

		writer.NewLine(false);
		writer.NewDivider(false);
		writer.NewLineString("// Player 2", false);
		writer.NewLine(false);
		writer.NewProperty("P2Scheme");
		writer << m_ControlScheme[PLAYER_TWO];

		writer.NewLine(false);
		writer.NewDivider(false);
		writer.NewLineString("// Player 3", false);
		writer.NewLine(false);
		writer.NewProperty("P3Scheme");
		writer << m_ControlScheme[PLAYER_THREE];

		writer.NewLine(false);
		writer.NewDivider(false);
		writer.NewLineString("// Player 4", false);
		writer.NewLine(false);
		writer.NewProperty("P4Scheme");
		writer << m_ControlScheme[PLAYER_FOUR];

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::LoadDeviceIcons() {
		m_DeviceIcons[DEVICE_KEYB_ONLY] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
		m_DeviceIcons[DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));
		m_DeviceIcons[DEVICE_GAMEPAD_1] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 1"));
		m_DeviceIcons[DEVICE_GAMEPAD_2] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 2"));
		m_DeviceIcons[DEVICE_GAMEPAD_3] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 3"));
		m_DeviceIcons[DEVICE_GAMEPAD_4] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 4"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetInputClass(GUIInput* inputClass) { s_InputClass = inputClass; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Icon * UInputMan::GetSchemeIcon(short whichPlayer) const {
		return (whichPlayer < Activity::PLAYER_1 || whichPlayer >= Activity::MAXPLAYERCOUNT) ? 0 : m_DeviceIcons[m_ControlScheme[whichPlayer].GetDevice()];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string UInputMan::GetMappingName(short whichPlayer, int whichElement) {
		if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS) {
			return "";
		}

		int device = m_ControlScheme[whichPlayer].GetDevice();
		int preset = m_ControlScheme[whichPlayer].GetPreset();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		// If there is a preset, just return the element name set by the preset previously
		if (preset != PRESET_NONE && !element->GetPresetDesc().empty()) {
			return element->GetPresetDesc();
		}

		// Joystick input is used, more important to show than keyboard
		if (device >= DEVICE_GAMEPAD_1) {
			// Translate the device selection to the joystick numbering (0-3)
			int whichJoy = device - DEVICE_GAMEPAD_1;

			// Check joystick button presses
			if (element->GetJoyButton() != JOY_NONE) {
				return joy[whichJoy].button[element->GetJoyButton()].name;
			}
			// Check joystick axis directions
			if (element->JoyDirMapped()) {
				return "Joystick";
			}
		}
		// Mouse input is used, more important to show than keyboard
		if (device == DEVICE_MOUSE_KEYB && element->GetMouseButton() != MOUSE_NONE) {
			int button = element->GetMouseButton();

			switch (button) {
				case MOUSE_LEFT:
					return "Left Mouse";
				case MOUSE_RIGHT:
					return "Right Mouse";
				case MOUSE_MIDDLE:
					return "Middle Mouse";
			}
		}
		// Keyboard defaults - don't check certain elements which don't make sense when in mouse mode
		if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN)) && element->GetKey() != 0) {
			return scancode_to_name(element->GetKey());
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureKeyMapping(short whichPlayer, int whichInput) {
		if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS) {
			return false;
		}
		// Update the key array
		if (keyboard_needs_poll()) { poll_keyboard(); }

		for (int whichKey = 0; whichKey < KEY_MAX; ++whichKey) {
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
		if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS) {
			return false;
		}
		int whichButton = WhichJoyButtonPressed(whichJoy);

		if (whichButton != JOY_NONE) {
			m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset();
			SetButtonMapping(whichPlayer, whichInput, whichButton);
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::CaptureDirectionMapping(short whichPlayer, int whichJoy, int whichInput) {
		if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS) {
			return false;
		}

		// Go through all the sticks on this joystick
		for (int stick = 0; stick < joy[whichJoy].num_sticks; ++stick) {
			// Go through all the axes on this joystick
			for (int axis = 0; axis < joy[whichJoy].stick[stick].num_axis; ++axis) {
				// See if there is direction press in the first direction
				if (joy[whichJoy].stick[stick].axis[axis].d1 && s_ChangedJoystickStates[whichJoy].stick[stick].axis[axis].d1) {
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
		int device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = m_ControlScheme[whichPlayer].GetInputMappings();
		Vector moveValues;

		// Joystick input is applicable
		if (device >= DEVICE_GAMEPAD_1) {
			// Translate the device selection to the joystick numbering (0-3)
			int whichJoy = device - DEVICE_GAMEPAD_1;

			// Assume axes are stretched out over up-down, and left-right
			if (element[INPUT_L_LEFT].JoyDirMapped()) { moveValues.m_X = AnalogAxisValue(whichJoy, element[INPUT_L_LEFT].GetStick(), element[INPUT_L_LEFT].GetAxis()); }
			if (element[INPUT_L_UP].JoyDirMapped()) { moveValues.m_Y = AnalogAxisValue(whichJoy, element[INPUT_L_UP].GetStick(), element[INPUT_L_UP].GetAxis()); }
		}
		return moveValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::AnalogAimValues(short whichPlayer) {
		int device = m_ControlScheme[whichPlayer].GetDevice();

		if (IsInMultiplayerMode()) { device = DEVICE_MOUSE_KEYB; }

		const InputMapping *element = m_ControlScheme[whichPlayer].GetInputMappings();
		Vector aimValues;

		if (device == DEVICE_MOUSE_KEYB) {
			// Return the normalized mouse analog stick emulation value
			aimValues = (IsInMultiplayerMode() && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS) ? (m_NetworkAnalogMoveData[whichPlayer] / m_MouseTrapRadius) : (m_AnalogMouseData / m_MouseTrapRadius);
		}
		if (device >= DEVICE_GAMEPAD_1) {
			// Translate the device selection to the joystick numbering (0-3)
			int whichJoy = device - DEVICE_GAMEPAD_1;

			// Assume axes are stretched out over up-down, and left-right
			if (element[INPUT_R_LEFT].JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(whichJoy, element[INPUT_R_LEFT].GetStick(), element[INPUT_R_LEFT].GetAxis()); }
			if (element[INPUT_R_UP].JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(whichJoy, element[INPUT_R_UP].GetStick(), element[INPUT_R_UP].GetAxis()); }
		}
		return aimValues;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementHeld(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementHeld[whichPlayer][whichElement] : false;
		}
		bool held = false;
		int device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		// Don't check certain elements which don't make sense when in mouse mode
		if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN))) { held = held ? true : KeyHeld(element->GetKey()); }

		if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos) { held = held ? true : MouseButtonHeld(element->GetMouseButton(), whichPlayer); }

		if (device >= DEVICE_GAMEPAD_1) {
			// Translate the device selection to the joystick numbering (0-3)
			int whichJoy = device - DEVICE_GAMEPAD_1;

			// Check joystick button holds
			held = held ? true : JoyButtonHeld(whichJoy, element->GetJoyButton());

			// Check joystick axis direction holds
			if (!held && element->JoyDirMapped()) { held = JoyDirectionHeld(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return held;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementPressed(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementPressed[whichPlayer][whichElement] : false;
		}
		bool pressed = false;
		int device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN))) { pressed = pressed ? true : KeyPressed(element->GetKey()); }
		if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos) { pressed = pressed ? true : MouseButtonPressed(element->GetMouseButton(), whichPlayer); }
		if (device >= DEVICE_GAMEPAD_1) {
			int whichJoy = device - DEVICE_GAMEPAD_1;
			pressed = pressed ? true : JoyButtonPressed(whichJoy, element->GetJoyButton());
			if (!pressed && element->JoyDirMapped()) { pressed = JoyDirectionPressed(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::ElementReleased(short whichPlayer, int whichElement) {
		if (IsInMultiplayerMode() && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS) {
			return m_TrapMousePosPerPlayer[whichPlayer] ? m_NetworkInputElementReleased[whichPlayer][whichElement] : false;
		}
		bool released = false;
		int device = m_ControlScheme[whichPlayer].GetDevice();
		const InputMapping *element = &(m_ControlScheme[whichPlayer].GetInputMappings()[whichElement]);

		if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN))) { released = released ? true : KeyReleased(element->GetKey()); }
		if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos) { released = released ? true : MouseButtonReleased(element->GetMouseButton(), whichPlayer); }
		if (device >= DEVICE_GAMEPAD_1) {
			int whichJoy = device - DEVICE_GAMEPAD_1;
			released = released ? true : JoyButtonReleased(whichJoy, element->GetJoyButton());
			if (!released && element->JoyDirMapped()) { released = JoyDirectionReleased(whichJoy, element->GetStick(), element->GetAxis(), element->GetDirection()); }
		}
		return released;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetMenuDirectional() {
		Vector allInput;
		for (int player = 0; player < MAX_PLAYERS; ++player) {
			int device = m_ControlScheme[player].GetDevice();

			switch (device) {
				case DEVICE_KEYB_ONLY:
					if (ElementHeld(player, INPUT_L_UP)) {
						allInput.m_Y += -1.0F;
					} else if (ElementHeld(player, INPUT_L_DOWN)) {
						allInput.m_Y += 1.0F;
					} else if (ElementHeld(player, INPUT_L_LEFT)) {
						allInput.m_X += -1.0F;
					} else if (ElementHeld(player, INPUT_L_RIGHT)) {
						allInput.m_X += 1.0F;
					}
					break;
				case DEVICE_MOUSE_KEYB:
					// Mouse player shouldn't be doing anything here, he should be using the mouse!
					break;
				default:
					if (device == DEVICE_COUNT) {
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
		for (int player = 0; player < MAX_PLAYERS && !button; ++player) {
			int device = m_ControlScheme[player].GetDevice();

			if (whichButton >= MENU_PRIMARY) { button = ElementHeld(player, INPUT_FIRE) || MouseButtonHeld(MOUSE_LEFT, player) || button; }
			if (whichButton >= MENU_SECONDARY) { button = ElementHeld(player, INPUT_PIEMENU) || MouseButtonHeld(MOUSE_RIGHT, player) || button; }

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
		for (int player = 0; player < MAX_PLAYERS && !button; ++player) {
			int device = m_ControlScheme[player].GetDevice();

			if (whichButton >= MENU_PRIMARY) { button = ElementPressed(player, INPUT_FIRE) || MouseButtonPressed(MOUSE_LEFT, player) || button; }
			if (whichButton >= MENU_SECONDARY) { button = ElementPressed(player, INPUT_PIEMENU) || MouseButtonPressed(MOUSE_RIGHT, player) || button; }

			if (button) { m_LastDeviceWhichControlledGUICursor = device; }
		}
		return button;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MenuButtonReleased(int whichButton) {
		bool button = false;
		for (int player = 0; player < MAX_PLAYERS && !button; ++player) {
			if (whichButton >= MENU_PRIMARY) { button = ElementReleased(player, INPUT_FIRE) || MouseButtonReleased(MOUSE_LEFT, player) || button; }
			if (whichButton >= MENU_SECONDARY) { button = ElementReleased(player, INPUT_PIEMENU) || MouseButtonReleased(MOUSE_RIGHT, player) || button; }
		}
		return button;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::KeyPressed(const char keyToTest) {
		if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) {
			return false;
		}
		bool pressed = false;
		pressed = (s_InputClass) ? (s_InputClass->GetScanCodeState(keyToTest) == GUIInput::Pushed) : (s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest]);
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::KeyReleased(const char keyToTest) {
		if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) {
			return false;
		}
		bool released = !s_PrevKeyStates[keyToTest] && s_ChangedKeyStates[keyToTest];
		return released;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyInput() {
		// Check keyboard
		bool input = keypressed();
		// Check all joysticks
		if (!input) { input = AnyJoyInput(); }
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyPress() {
		bool pressed = false;
		// Check keyboard for presses
		for (int testKey = 0; testKey < KEY_MAX; ++testKey) {
			pressed = s_PrevKeyStates[testKey] && s_ChangedKeyStates[testKey] ? true : pressed;
		}
		// Check mouse buttons for presses
		if (!pressed) { pressed = AnyMouseButtonPress(); }
		// Check all joysticks
		if (!pressed) { pressed = AnyJoyPress(); }
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyStartPress() {
		bool pressed = false;
		if (KeyPressed(KEY_ESC) || KeyPressed(KEY_SPACE)) { pressed = true; }

		// Check all user bound start buttons
		for (int player = 0; player < MAX_PLAYERS && !pressed; ++player) {
			pressed = pressed || ElementPressed(player, INPUT_START);
			pressed = pressed || ElementPressed(player, INPUT_BACK);
		}
		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::MouseUsedByPlayer() const {
		if (m_ControlScheme[PLAYER_ONE].GetDevice() == DEVICE_MOUSE_KEYB) {
			return PLAYER_ONE;
		} else if (m_ControlScheme[PLAYER_TWO].GetDevice() == DEVICE_MOUSE_KEYB) {
			return PLAYER_TWO;
		} else if (m_ControlScheme[PLAYER_THREE].GetDevice() == DEVICE_MOUSE_KEYB) {
			return PLAYER_THREE;
		} else if (m_ControlScheme[PLAYER_FOUR].GetDevice() == DEVICE_MOUSE_KEYB) {
			return PLAYER_FOUR;
		}
		return PLAYER_NONE;
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

	Vector UInputMan::GetMouseMovement(short whichPlayer) {
		Vector mouseMovement;
		if (IsInMultiplayerMode() && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS) {
			mouseMovement = m_NetworkAccumulatedRawMouseMovement[whichPlayer];
			return mouseMovement;
		}
		if (whichPlayer == -1 || m_ControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB) { mouseMovement = m_RawMouseMovement; }

		return mouseMovement;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetMousePos(Vector &newPos, short whichPlayer) {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == -1 || m_ControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB)) { position_mouse(newPos.m_X, newPos.m_Y); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonHeld(int whichButton, short whichPlayer) {
		if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= MAX_PLAYERS) {
				for (int player = 0; player < MAX_PLAYERS; player++) {
					if (m_NetworkMouseButtonHeldState[player][whichButton]) {
						return m_NetworkMouseButtonHeldState[player][whichButton];
					}
				}
				return m_NetworkMouseButtonHeldState[0][whichButton];
			} else {
				return m_NetworkMouseButtonHeldState[whichPlayer][whichButton];
			}
		}
		return s_MouseButtonStates[whichButton];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonPressed(int whichButton, short whichPlayer) {
		if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= MAX_PLAYERS) {
				for (int i = 0; i < MAX_PLAYERS; i++) {
					if (m_NetworkMouseButtonPressedState[i][whichButton]) {
						return m_NetworkMouseButtonPressedState[i][whichButton];
					}
				}
				return m_NetworkMouseButtonPressedState[0][whichButton];
			} else {
				return m_NetworkMouseButtonPressedState[whichPlayer][whichButton];
			}
		}
		bool pressed = s_MouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];

		return pressed;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::MouseButtonReleased(int whichButton, short whichPlayer) {
		if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS) {
			return false;
		}
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= MAX_PLAYERS) {
				for (int i = 0; i < MAX_PLAYERS; i++) {
					if (m_NetworkMouseButtonReleasedState[i][whichButton]) {
						return m_NetworkMouseButtonReleasedState[i][whichButton];
					}
				}
				return m_NetworkMouseButtonReleasedState[0][whichButton];
			} else {
				return m_NetworkMouseButtonReleasedState[whichPlayer][whichButton];
			}
		}
		bool released = !s_MouseButtonStates[whichButton] && s_ChangedMouseButtonStates[whichButton];

		return released;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyMouseButtonPress() {
		bool input = false;
		for (int button = 0; button < MAX_MOUSE_BUTTONS && !input; ++button) {
			input = MouseButtonPressed(button, -1) ? true : input;
		}
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::TrapMousePos(bool trap, short whichPlayer) {
		if (whichPlayer == -1 || m_ControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB) {
			m_TrapMousePos = trap;
		}
		m_TrapMousePosPerPlayer[whichPlayer] = trap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, short whichPlayer) {
		// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
		if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == -1 || m_ControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB)) {
			int mouseX = std::max(x, static_cast<int>(mouse_x));
			int mouseY = std::max(y, static_cast<int>(mouse_y));
			mouseX = std::min(mouseX, x + width * g_FrameMan.ResolutionMultiplier());
			mouseY = std::min(mouseY, y + height * g_FrameMan.ResolutionMultiplier());

			position_mouse(mouseX, mouseY);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ForceMouseWithinPlayerScreen(short whichPlayer) {
		if (whichPlayer < PLAYER_ONE || whichPlayer >= PLAYER_FOUR) {
			return;
		}
		int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer);
		int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer);

		// If we are dealing with split screens, then draw the intermediate draw splitscreen to the appropriate spot on the back buffer
		if (g_FrameMan.GetScreenCount() > 1) {
			if (whichPlayer == 0) {
				// Always upper left corner
				ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
			} else if (whichPlayer == 1) {
				// If both splits, or just V-Split, then in upper right quadrant
				if ((g_FrameMan.GetVSplit() && !g_FrameMan.GetHSplit()) || (g_FrameMan.GetVSplit() && g_FrameMan.GetHSplit())) {
					ForceMouseWithinBox(g_FrameMan.GetResX() / 2, 0, screenWidth, screenHeight, whichPlayer);
				} else {
					ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
				}
			} else if (whichPlayer == 2) {
				// Always lower left quadrant
				ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);

			} else if (whichPlayer == 3) {
				// Always lower right quadrant
				ForceMouseWithinBox(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
			}
		} else {
			ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoystickActive(int joystickNumber) const {
		if (joystickNumber < 0 || joystickNumber >= MAX_PLAYERS) {
			return false;
		}
		if (joystickNumber < num_joysticks) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonHeld(int whichJoy, int whichButton) {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return false;
		}
		if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons) {
			return false;
		}
		return joy[whichJoy].button[whichButton].b;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::WhichJoyButtonHeld(int whichJoy) {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return JOY_NONE;
		}
		for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
			if (joy[whichJoy].button[button].b) {
				return button;
			}
		}
		// No button is held down
		return JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonPressed(int whichJoy, int whichButton) {
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

	int UInputMan::WhichJoyButtonPressed(int whichJoy) {
		if (whichJoy < 0 || whichJoy >= num_joysticks) {
			return JOY_NONE;
		}
		for (int button = 0; button < joy[whichJoy].num_buttons; ++button) {
			if (joy[whichJoy].button[button].b && JoyButtonPressed(whichJoy, button)) {
				return button;
			}
		}
		// No button was pressed down
		return JOY_NONE;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyButtonReleased(int whichJoy, int whichButton) {
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

	bool UInputMan::JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir) {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JOYDIR_TWO) {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir) {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JOYDIR_TWO) {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir) {
		if (whichJoy < 0 || whichJoy >= num_joysticks || whichStick < 0 || whichStick >= joy[whichJoy].num_sticks || whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis) {
			return false;
		}
		if (whichDir == JOYDIR_TWO) {
			return !joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d2;
		} else {
			return !joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_ChangedJoystickStates[whichJoy].stick[whichStick].axis[whichAxis].d1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float UInputMan::AnalogAxisValue(int whichJoy, int whichStick, int whichAxis) {
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

	bool UInputMan::AnyJoyInput() {
		bool input = false;
		int joystick;
		int button;
		int stick;
		int axis;

		poll_joystick();

		for (joystick = 0; joystick < MAX_PLAYERS && !input; ++joystick) {
			for (button = 0; button < MAX_JOY_BUTTONS && !input; ++button) {
				input = joy[joystick].button[button].b ? true : input;
			}
			if (!input) {
				for (stick = 0; stick < joy[joystick].num_sticks; ++stick) {
					for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
						input = JoyDirectionHeld(joystick, stick, axis, JOYDIR_ONE) || JoyDirectionHeld(joystick, stick, axis, JOYDIR_TWO);
					}
				}
			}
		}
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyPress() {
		bool input = false;
		int joystick;
		int button;
		int stick;
		int axis;

		for (joystick = 0; joystick < MAX_PLAYERS && !input; ++joystick) {
			for (button = 0; button < MAX_JOY_BUTTONS && !input; ++button) {
				input = JoyButtonPressed(joystick, button) ? true : input;
			}
			if (!input) {
				for (stick = 0; stick < joy[joystick].num_sticks; ++stick) {
					for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
						input = JoyDirectionPressed(joystick, stick, axis, JOYDIR_ONE) || JoyDirectionPressed(joystick, stick, axis, JOYDIR_TWO);
					}
				}
			}
		}
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool UInputMan::AnyJoyButtonPress(int whichJoy) {
		bool input = false;

		for (int button = 0; button < MAX_JOY_BUTTONS && !input; ++button) {
			input = JoyButtonPressed(whichJoy, button) ? true : input;
		}
		return input;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector UInputMan::GetNetworkAccumulatedRawMouseMovement(short player) {
		Vector accumulatedMovement = m_NetworkAccumulatedRawMouseMovement[player];
		m_NetworkAccumulatedRawMouseMovement[player].Reset();
		return accumulatedMovement;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkInputElementHeldState(short player, int element, bool state) {
		if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS) { m_NetworkInputElementHeld[player][element] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkInputElementPressedState(short player, int element, bool state) {
		if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS) { m_NetworkInputElementPressed[player][element] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkInputElementReleasedState(short player, int element, bool state) {
		if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS) { m_NetworkInputElementReleased[player][element] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkMouseWheelState(short player, int state) {
		if (player >= 0 && player < MAX_PLAYERS) { m_NetworkMouseWheelState[player] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkMouseButtonHeldState(short player, int whichButton, bool state) {
		if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS) { m_NetworkMouseButtonHeldState[player][whichButton] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkMouseButtonPressedState(short player, int whichButton, bool state) {
		if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS) { m_NetworkMouseButtonPressedState[player][whichButton] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::SetNetworkMouseButtonReleasedState(short player, int whichButton, bool state) {
		if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS) { m_NetworkMouseButtonReleasedState[player][whichButton] = state; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::ClearAccumulatedStates() {
		for (int element = 0; element < INPUT_COUNT; element++) {
			m_NetworkAccumulatedElementPressed[element] = false;
			m_NetworkAccumulatedElementReleased[element] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int UInputMan::Update() {
		m_LastDeviceWhichControlledGUICursor = 0;

		if (keyboard_needs_poll()) { poll_keyboard(); }
		if (mouse_needs_poll()) { poll_mouse(); }
		poll_joystick();

		// Detect and store key changes since last Update()
		for (int i = 0; i < KEY_MAX; ++i) {
			s_ChangedKeyStates[i] = key[i] != s_PrevKeyStates[i];
		}

		// Store mouse movement
		int mickeyX;
		int mickeyY;
		get_mouse_mickeys(&mickeyX, &mickeyY);
		m_RawMouseMovement.SetIntXY(mickeyX, mickeyY);

		// TODO: Have proper mouse sensitivity controls somewhere
		m_RawMouseMovement *= m_MouseSensitivity;

		// NETWORK SERVER: Apply mouse input received from client or collect mouse input
		IsInMultiplayerMode() ? UpdateNetworkMouseMovement() : m_NetworkAccumulatedRawMouseMovement[0] += m_RawMouseMovement;

		UpdateMouseInput();
		if (num_joysticks > 0) { UpdateJoystickInput(); }

		// If Escape is pressed, go to the main menu or close the app
		if (KeyPressed(KEY_ESC)) {
			// If we launched into editor directly, skip the logic and quit quickly.
			if (g_LaunchIntoEditor) { g_Quit = true; }

			// If in activity pause and exit to menu on esc
			if (g_InActivity) {
				g_ActivityMan.PauseActivity();
				g_InActivity = false;
			}
		}

		if (g_InActivity) {
			// Reset Activity if Ctrl+R is pressed
			if (FlagCtrlState() && KeyPressed(KEY_R)) { g_ResetActivity = true; }

			// Check for resets and start button presses on controllers of all active players
			if (g_ActivityMan.GetActivity()) {
				for (int player = PLAYER_ONE; player < MAX_PLAYERS; ++player) {
					if (g_ActivityMan.GetActivity()->PlayerActive(player)) {
						g_ResetActivity = g_ResetActivity || ElementPressed(PLAYER_ONE, INPUT_BACK);

						if (ElementPressed(player, INPUT_START)) {
							g_ActivityMan.PauseActivity();
							g_InActivity = false;
						}
					}
				}
			}
			if (g_ResetActivity) { g_ConsoleMan.PrintString("SYSTEM: Activity was reset!"); }
		}

		// Alt+Enter to switch resolution multiplier between 1X and 2X
		if (FlagAltState() && KeyPressed(KEY_ENTER)) { g_FrameMan.SwitchResolutionMultiplier((g_FrameMan.ResolutionMultiplier() >= 2) ? 1 : 2); }

		// Only allow performance tweaking if showing the stats
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

		// Screenshot, Ctrl+S
		if ((FlagCtrlState() && KeyHeld(KEY_S)) || KeyHeld(KEY_PRTSCR)) { g_FrameMan.SaveScreenToBMP("ScreenDump"); }

		// Screenshot entire scene, Ctrl+W
		if (FlagCtrlState() && KeyHeld(KEY_W)) { g_FrameMan.SaveWorldToBMP("WorldDump"); }

		// Dump scene preview (scaled down WorldDump), Alt+W
		if (FlagAltState() && KeyHeld(KEY_W)) { g_FrameMan.SaveWorldToPreviewBMP("ScenePreviewDump"); }

		// Material draw toggle, Ctrl+M
		if (FlagCtrlState() && KeyPressed(KEY_M)) { g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3); }

		// Performance stats display toggle, Ctrl+P
		if (FlagCtrlState() && KeyPressed(KEY_P)) { g_PerformanceMan.ShowPerformanceStats(!g_PerformanceMan.IsShowingPerformanceStats()); }

		// Force one sim update per graphics frame, Ctrl+O
		if (FlagCtrlState() && KeyPressed(KEY_O)) { g_TimerMan.SetOneSimUpdatePerFrame(!g_TimerMan.IsOneSimUpdatePerFrame()); }

		if (KeyPressed(KEY_F1)) { g_ConsoleMan.ShowShortcuts(); }
		if (KeyPressed(KEY_F2)) {
			g_PresetMan.ReloadAllScripts();
			g_ConsoleMan.PrintString("SYSTEM: Scripts reloaded!");
		}
		if (KeyPressed(KEY_F3)) { g_ConsoleMan.SaveAllText("Console.dump.log"); }
		if (KeyPressed(KEY_F4)) { g_ConsoleMan.SaveInputLog("Console.input.log"); }
		if (KeyPressed(KEY_F5)) { g_ConsoleMan.ClearLog(); }

		StoreInputEventsForNextUpdate();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateNetworkMouseMovement() {
		for (int player = 0; player < MAX_PLAYERS; player++) {
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
			for (int el = 0; el < INPUT_COUNT; el++) {
				m_NetworkInputElementPressed[player][el] = false;
				m_NetworkInputElementReleased[player][el] = false;
			}

			// Reset mouse movement
			m_NetworkAccumulatedRawMouseMovement[player].Reset();

			// Reset mouse button states to stop double clicking
			for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
				m_NetworkMouseButtonPressedState[player][i] = false;
				m_NetworkMouseButtonReleasedState[player][i] = false;
			}

			// Reset mouse wheel state to stop over-wheeling
			m_NetworkMouseWheelState[player] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UInputMan::UpdateMouseInput() {
		// Detect and store mouse button input
		s_MouseButtonStates[MOUSE_LEFT] = mouse_b & 1;
		s_MouseButtonStates[MOUSE_RIGHT] = mouse_b & 2;
		s_MouseButtonStates[MOUSE_MIDDLE] = mouse_b & 4;

		// Detect changes in mouse button input
		s_ChangedMouseButtonStates[MOUSE_LEFT] = s_MouseButtonStates[MOUSE_LEFT] != s_PrevMouseButtonStates[MOUSE_LEFT];
		s_ChangedMouseButtonStates[MOUSE_RIGHT] = s_MouseButtonStates[MOUSE_RIGHT] != s_PrevMouseButtonStates[MOUSE_RIGHT];
		s_ChangedMouseButtonStates[MOUSE_MIDDLE] = s_MouseButtonStates[MOUSE_MIDDLE] != s_PrevMouseButtonStates[MOUSE_MIDDLE];

		// Detect and store mouse movement input, translated to analog stick emulation
		int mousePlayer = MouseUsedByPlayer();
		if (mousePlayer != PLAYER_NONE) {
			m_AnalogMouseData.m_X += m_RawMouseMovement.m_X * 3;
			m_AnalogMouseData.m_Y += m_RawMouseMovement.m_Y * 3;
			// Cap the mouse input in a circle
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
				// Save the direction
				m_MouseWheelChange = mouse_z;
				// Reset the position
				position_mouse_z(0);
			} else {
				m_MouseWheelChange = 0;
			}

			// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
			// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
			int mX = mouse_x / g_FrameMan.ResolutionMultiplier();
			int mY = mouse_y / g_FrameMan.ResolutionMultiplier();
			if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mX >= 0 && mX < g_FrameMan.GetResX() && mY >= 0 && mY < g_FrameMan.GetResY())) {
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

			// Determine deadzone settings
			float deadZone = 0.0F;
			int deadZoneType = DeadZoneType::CIRCLE;
			int player = -1;

			// Retrieve deadzone settings from player's input scheme
			for (int p = 0; p < UInputMan::MAX_PLAYERS; p++) {
				int device = m_ControlScheme[p].GetDevice() - DEVICE_GAMEPAD_1;

				if (device == joystick) {
					deadZone = m_ControlScheme[p].GetJoystickDeadzone();
					deadZoneType = m_ControlScheme[p].GetJoystickDeadzoneType();
					player = p;
					break;
				}
			}

			// Disable input if it's in circle deadzone
			if (player > -1 && deadZoneType == DeadZoneType::CIRCLE && deadZone > 0.0F) {
				const InputMapping *element = m_ControlScheme[player].GetInputMappings();
				Vector aimValues;

				// Left stick
				if (element[INPUT_L_LEFT].JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(joystick, element[INPUT_L_LEFT].GetStick(), element[INPUT_L_LEFT].GetAxis()); }
				if (element[INPUT_L_UP].JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(joystick, element[INPUT_L_UP].GetStick(), element[INPUT_L_UP].GetAxis()); }

				if (aimValues.GetMagnitude() < deadZone * 2) {
					if (element[INPUT_L_LEFT].JoyDirMapped()) {
						if (joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].pos = 128;
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].pos = 0;
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_L_LEFT].GetStick()].axis[element[INPUT_L_LEFT].GetAxis()].d2 = 0;
						}
					}
					if (element[INPUT_L_UP].JoyDirMapped()) {
						if (joy[joystick].stick[element[INPUT_L_UP].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].pos = 128;
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].pos = 0;
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_L_UP].GetStick()].axis[element[INPUT_L_UP].GetAxis()].d2 = 0;
						}
					}
				}

				aimValues.Reset();

				// Right stick
				if (element[INPUT_R_LEFT].JoyDirMapped()) { aimValues.m_X = AnalogAxisValue(joystick, element[INPUT_R_LEFT].GetStick(), element[INPUT_R_LEFT].GetAxis()); }
				if (element[INPUT_R_UP].JoyDirMapped()) { aimValues.m_Y = AnalogAxisValue(joystick, element[INPUT_R_UP].GetStick(), element[INPUT_R_UP].GetAxis()); }

				if (aimValues.GetMagnitude() < deadZone * 2) {
					if (element[INPUT_R_LEFT].JoyDirMapped()) {
						if (joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].pos = 128;
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].pos = 0;
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_R_LEFT].GetStick()].axis[element[INPUT_R_LEFT].GetAxis()].d2 = 0;
						}
					}
					if (element[INPUT_R_UP].JoyDirMapped()) {
						if (joy[joystick].stick[element[INPUT_R_UP].GetStick()].flags & JOYFLAG_UNSIGNED) {
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].pos = 128;
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].d2 = 0;
						} else {
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].pos = 0;
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].d1 = 0;
							joy[joystick].stick[element[INPUT_R_UP].GetStick()].axis[element[INPUT_R_UP].GetAxis()].d2 = 0;
						}
					}
				}
			}

			// Stick, axis, directions
			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					// Adjust joystick values to eliminate values in deadzone
					if (player > -1 && deadZoneType == DeadZoneType::SQUARE && deadZone > 0.0F) {
						// This one heavily relies on AnalogAxiesValue method of processing joystick data.
						if (joy[joystick].stick[stick].flags & JOYFLAG_UNSIGNED) {
							if (std::abs(joy[joystick].stick[stick].axis[axis].pos - 128) > 0.0F && std::abs(joy[joystick].stick[stick].axis[axis].pos - 128) / 128 < deadZone) {
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
		// Save the current state of the keyboard so that we can compare it next frame and see which key states have been changed in the mean time.
		std::memcpy(s_PrevKeyStates, const_cast<const char *>(key), KEY_MAX);

		// Save the mouse button states so that we can compare it next frame and see which buttons have changed.
		for (int mouseButton = 0; mouseButton < MAX_MOUSE_BUTTONS; ++mouseButton) {
			s_PrevMouseButtonStates[mouseButton] = s_MouseButtonStates[mouseButton];
		}

		// Save the current state of the joysticks
		for (int joystick = 0; joystick < num_joysticks; ++joystick) {
			for (int button = 0; button < joy[joystick].num_buttons; ++button) {
				s_PrevJoystickStates[joystick].button[button].b = joy[joystick].button[button].b;
			}
			for (int stick = 0; stick < joy[joystick].num_sticks; ++stick) {
				for (int axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis) {
					s_PrevJoystickStates[joystick].stick[stick].axis[axis].d1 = joy[joystick].stick[stick].axis[axis].d1;
					s_PrevJoystickStates[joystick].stick[stick].axis[axis].d2 = joy[joystick].stick[stick].axis[axis].d2;
				}
			}
		}

		// Store pressed and released events to be picked by NetworkClient during it's on-timer update
		for (int element = 0; element < INPUT_COUNT; element++) {
			// Only store press and release events if they happened, Client will clear those after update so we don't care about false
			if (ElementPressed(0, element)) { m_NetworkAccumulatedElementPressed[element] = true; }
			if (ElementReleased(0, element)) { m_NetworkAccumulatedElementReleased[element] = true; }
		}
	}
}