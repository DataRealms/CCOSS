#include "InputScheme.h"
#include "UInputMan.h"

namespace RTE {

	const std::string InputScheme::c_ClassName = "InputScheme";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputScheme::Clear() {
		m_ActiveDevice = InputDevice::DEVICE_KEYB_ONLY;
		m_SchemePreset = InputPreset::NoPreset;
		m_JoystickDeadzoneType = DeadZoneType::CIRCLE;
		m_JoystickDeadzone = 0.01F;
		m_DigitalAimSpeed = 1.0F;

		for (InputMapping &inputMapping : m_InputMappings) {
			inputMapping.Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::Create(const InputScheme &reference) {
		m_ActiveDevice = reference.m_ActiveDevice;
		m_SchemePreset = reference.m_SchemePreset;
		m_JoystickDeadzoneType = reference.m_JoystickDeadzoneType;
		m_JoystickDeadzone = reference.m_JoystickDeadzone;
		m_DigitalAimSpeed = reference.m_DigitalAimSpeed;

		for (int inputMapping = 0; inputMapping < InputElements::INPUT_COUNT; ++inputMapping) {
			m_InputMappings[inputMapping].Create(reference.m_InputMappings[inputMapping]);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Device") {
			SetDevice(static_cast<InputDevice>(std::stoi(reader.ReadPropValue())));
		} else if (propName == "Preset") {
			SetPreset(static_cast<InputPreset>(std::stoi(reader.ReadPropValue())));
		} else if (propName == "LeftUp") {
			reader >> m_InputMappings[InputElements::INPUT_L_UP];
		} else if (propName == "LeftDown") {
			reader >> m_InputMappings[InputElements::INPUT_L_DOWN];
		} else if (propName == "LeftLeft") {
			reader >> m_InputMappings[InputElements::INPUT_L_LEFT];
		} else if (propName == "LeftRight") {
			reader >> m_InputMappings[InputElements::INPUT_L_RIGHT];
		} else if (propName == "Fire") {
			reader >> m_InputMappings[InputElements::INPUT_FIRE];
		} else if (propName == "Aim") {
			reader >> m_InputMappings[InputElements::INPUT_AIM];
		} else if (propName == "AimUp") {
			reader >> m_InputMappings[InputElements::INPUT_AIM_UP];
		} else if (propName == "AimDown") {
			reader >> m_InputMappings[InputElements::INPUT_AIM_DOWN];
		} else if (propName == "AimLeft") {
			reader >> m_InputMappings[InputElements::INPUT_AIM_LEFT];
		} else if (propName == "AimRight") {
			reader >> m_InputMappings[InputElements::INPUT_AIM_RIGHT];
		} else if (propName == "PieMenu") {
			reader >> m_InputMappings[InputElements::INPUT_PIEMENU];
		} else if (propName == "Jump") {
			reader >> m_InputMappings[InputElements::INPUT_JUMP];
		} else if (propName == "Crouch") {
			reader >> m_InputMappings[InputElements::INPUT_CROUCH];
		} else if (propName == "Next") {
			reader >> m_InputMappings[InputElements::INPUT_NEXT];
		} else if (propName == "Prev") {
			reader >> m_InputMappings[InputElements::INPUT_PREV];
		} else if (propName == "WeaponChangeNext") {
			reader >> m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT];
		} else if (propName == "WeaponChangePrev") {
			reader >> m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV];
		} else if (propName == "WeaponPickup") {
			reader >> m_InputMappings[InputElements::INPUT_WEAPON_PICKUP];
		} else if (propName == "WeaponDrop") {
			reader >> m_InputMappings[InputElements::INPUT_WEAPON_DROP];
		} else if (propName == "WeaponReload") {
			reader >> m_InputMappings[InputElements::INPUT_WEAPON_RELOAD];
		} else if (propName == "Start") {
			reader >> m_InputMappings[InputElements::INPUT_START];
		} else if (propName == "Back") {
			reader >> m_InputMappings[InputElements::INPUT_BACK];
		} else if (propName == "RightUp") {
			reader >> m_InputMappings[InputElements::INPUT_R_UP];
		} else if (propName == "RightDown") {
			reader >> m_InputMappings[InputElements::INPUT_R_DOWN];
		} else if (propName == "RightLeft") {
			reader >> m_InputMappings[InputElements::INPUT_R_LEFT];
		} else if (propName == "RightRight") {
			reader >> m_InputMappings[InputElements::INPUT_R_RIGHT];
		} else if (propName == "JoystickDeadzoneType") {
			SetJoystickDeadzoneType(static_cast<DeadZoneType>(std::stoi(reader.ReadPropValue())));
		} else if (propName == "JoystickDeadzone") {
			reader >> m_JoystickDeadzone;
		} else if (propName == "DigitalAimSpeed") {
			reader >> m_DigitalAimSpeed;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewPropertyWithValue("Device", m_ActiveDevice);
		writer.NewPropertyWithValue("Preset", m_SchemePreset);

		if (m_SchemePreset == InputPreset::NoPreset) {
			writer.NewPropertyWithValue("LeftUp", m_InputMappings[InputElements::INPUT_L_UP]);
			writer.NewPropertyWithValue("LeftDown", m_InputMappings[InputElements::INPUT_L_DOWN]);
			writer.NewPropertyWithValue("LeftLeft", m_InputMappings[InputElements::INPUT_L_LEFT]);
			writer.NewPropertyWithValue("LeftRight", m_InputMappings[InputElements::INPUT_L_RIGHT]);
			writer.NewPropertyWithValue("Fire", m_InputMappings[InputElements::INPUT_FIRE]);
			writer.NewPropertyWithValue("Aim", m_InputMappings[InputElements::INPUT_AIM]);
			writer.NewPropertyWithValue("AimUp", m_InputMappings[InputElements::INPUT_AIM_UP]);
			writer.NewPropertyWithValue("AimDown", m_InputMappings[InputElements::INPUT_AIM_DOWN]);
			writer.NewPropertyWithValue("AimLeft", m_InputMappings[InputElements::INPUT_AIM_LEFT]);
			writer.NewPropertyWithValue("AimRight", m_InputMappings[InputElements::INPUT_AIM_RIGHT]);
			writer.NewPropertyWithValue("PieMenu", m_InputMappings[InputElements::INPUT_PIEMENU]);
			writer.NewPropertyWithValue("Jump", m_InputMappings[InputElements::INPUT_JUMP]);
			writer.NewPropertyWithValue("Crouch", m_InputMappings[InputElements::INPUT_CROUCH]);
			writer.NewPropertyWithValue("Next", m_InputMappings[InputElements::INPUT_NEXT]);
			writer.NewPropertyWithValue("Prev", m_InputMappings[InputElements::INPUT_PREV]);
			writer.NewPropertyWithValue("WeaponChangeNext", m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT]);
			writer.NewPropertyWithValue("WeaponChangePrev", m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV]);
			writer.NewPropertyWithValue("WeaponPickup", m_InputMappings[InputElements::INPUT_WEAPON_PICKUP]);
			writer.NewPropertyWithValue("WeaponDrop", m_InputMappings[InputElements::INPUT_WEAPON_DROP]);
			writer.NewPropertyWithValue("WeaponReload", m_InputMappings[InputElements::INPUT_WEAPON_RELOAD]);
			writer.NewPropertyWithValue("Start", m_InputMappings[InputElements::INPUT_START]);
			writer.NewPropertyWithValue("Back", m_InputMappings[InputElements::INPUT_BACK]);
			writer.NewPropertyWithValue("RightUp", m_InputMappings[InputElements::INPUT_R_UP]);
			writer.NewPropertyWithValue("RightDown", m_InputMappings[InputElements::INPUT_R_DOWN]);
			writer.NewPropertyWithValue("RightLeft", m_InputMappings[InputElements::INPUT_R_LEFT]);
			writer.NewPropertyWithValue("RightRight", m_InputMappings[InputElements::INPUT_R_RIGHT]);
		}

		writer.NewPropertyWithValue("JoystickDeadzoneType", m_JoystickDeadzoneType);
		writer.NewPropertyWithValue("JoystickDeadzone", m_JoystickDeadzone);
		writer.NewPropertyWithValue("DigitalAimSpeed", m_DigitalAimSpeed);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputScheme::ResetToPlayerDefaults(Players player) {
		switch (player) {
			case Players::PlayerOne:
				m_ActiveDevice = InputDevice::DEVICE_MOUSE_KEYB;
				SetPreset(InputPreset::PresetMouseWASDKeys);
				break;
			case Players::PlayerTwo:
				m_ActiveDevice = InputDevice::DEVICE_KEYB_ONLY;
				SetPreset(InputPreset::PresetArrowKeys);
				break;
			case Players::PlayerThree:
				m_ActiveDevice = InputDevice::DEVICE_GAMEPAD_1;
				SetPreset(InputPreset::PresetGenericDualAnalog);
				break;
			case Players::PlayerFour:
				m_ActiveDevice = InputDevice::DEVICE_GAMEPAD_2;
				SetPreset(InputPreset::PresetGenericDualAnalog);
				break;
			default:
				RTEAbort("Invalid Player passed into InputScheme::ResetToDefault!");
				break;
		}
		m_JoystickDeadzoneType = DeadZoneType::CIRCLE;
		m_JoystickDeadzone = 0.01F;
		m_DigitalAimSpeed = 1.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputScheme::SetPreset(InputPreset schemePreset) {
		m_SchemePreset = schemePreset;

		if (schemePreset == InputPreset::NoPreset || schemePreset == InputPreset::InputPresetCount) {
			return;
		}
		for (InputMapping &inputMapping : m_InputMappings) {
			inputMapping.Reset();
		}
		switch (m_SchemePreset) {
			case InputPreset::PresetArrowKeys:
				m_InputMappings[InputElements::INPUT_L_UP].SetKey(SDL_SCANCODE_UP);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetKey(SDL_SCANCODE_DOWN);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetKey(SDL_SCANCODE_LEFT);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetKey(SDL_SCANCODE_RIGHT);
				m_InputMappings[InputElements::INPUT_FIRE].SetKey(SDL_SCANCODE_KP_1);
				m_InputMappings[InputElements::INPUT_AIM].SetKey(SDL_SCANCODE_KP_2);
				m_InputMappings[InputElements::INPUT_AIM_UP].SetKey(SDL_SCANCODE_UP);
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetKey(SDL_SCANCODE_DOWN);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetKey(SDL_SCANCODE_KP_3);
				m_InputMappings[InputElements::INPUT_JUMP].SetKey(SDL_SCANCODE_KP_ENTER);
				m_InputMappings[InputElements::INPUT_CROUCH].SetKey(SDL_SCANCODE_KP_0);
				m_InputMappings[InputElements::INPUT_NEXT].SetKey(SDL_SCANCODE_KP_5);
				m_InputMappings[InputElements::INPUT_PREV].SetKey(SDL_SCANCODE_KP_4);
				m_InputMappings[InputElements::INPUT_WEAPON_RELOAD].SetKey(SDL_SCANCODE_KP_DECIMAL);
				m_InputMappings[InputElements::INPUT_WEAPON_PICKUP].SetKey(SDL_SCANCODE_KP_6);
				m_InputMappings[InputElements::INPUT_WEAPON_DROP].SetKey(SDL_SCANCODE_KP_9);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV].SetKey(SDL_SCANCODE_KP_7);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT].SetKey(SDL_SCANCODE_KP_8);
				break;
			case InputPreset::PresetWASDKeys:
				m_InputMappings[InputElements::INPUT_L_UP].SetKey(SDL_SCANCODE_W);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetKey(SDL_SCANCODE_S);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetKey(SDL_SCANCODE_A);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetKey(SDL_SCANCODE_D);
				m_InputMappings[InputElements::INPUT_FIRE].SetKey(SDL_SCANCODE_H);
				m_InputMappings[InputElements::INPUT_AIM].SetKey(SDL_SCANCODE_J);
				m_InputMappings[InputElements::INPUT_AIM_UP].SetKey(SDL_SCANCODE_W);
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetKey(SDL_SCANCODE_S);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetKey(SDL_SCANCODE_K);
				m_InputMappings[InputElements::INPUT_JUMP].SetKey(SDL_SCANCODE_L);
				m_InputMappings[InputElements::INPUT_CROUCH].SetKey(SDL_SCANCODE_C);
				m_InputMappings[InputElements::INPUT_NEXT].SetKey(SDL_SCANCODE_U);
				m_InputMappings[InputElements::INPUT_PREV].SetKey(SDL_SCANCODE_Y);
				m_InputMappings[InputElements::INPUT_WEAPON_RELOAD].SetKey(SDL_SCANCODE_R);
				m_InputMappings[InputElements::INPUT_WEAPON_PICKUP].SetKey(SDL_SCANCODE_F);
				m_InputMappings[InputElements::INPUT_WEAPON_DROP].SetKey(SDL_SCANCODE_B);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV].SetKey(SDL_SCANCODE_Q);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT].SetKey(SDL_SCANCODE_E);
				break;
			case InputPreset::PresetMouseWASDKeys:
				m_InputMappings[InputElements::INPUT_L_UP].SetKey(SDL_SCANCODE_W);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetKey(SDL_SCANCODE_S);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetKey(SDL_SCANCODE_A);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetKey(SDL_SCANCODE_D);
				m_InputMappings[InputElements::INPUT_FIRE].SetMouseButton(MouseButtons::MOUSE_LEFT);
				m_InputMappings[InputElements::INPUT_AIM].SetPresetDescription("Mouse Move");
				m_InputMappings[InputElements::INPUT_AIM_UP].SetPresetDescription("Mouse Move");
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetPresetDescription("Mouse Move");
				m_InputMappings[InputElements::INPUT_AIM_LEFT].SetPresetDescription("Mouse Move");
				m_InputMappings[InputElements::INPUT_AIM_RIGHT].SetPresetDescription("Mouse Move");
				m_InputMappings[InputElements::INPUT_PIEMENU].SetMouseButton(MouseButtons::MOUSE_RIGHT);
				m_InputMappings[InputElements::INPUT_JUMP].SetKey(SDL_SCANCODE_W);
				m_InputMappings[InputElements::INPUT_CROUCH].SetKey(SDL_SCANCODE_S);
				m_InputMappings[InputElements::INPUT_NEXT].SetKey(SDL_SCANCODE_E);
				m_InputMappings[InputElements::INPUT_PREV].SetKey(SDL_SCANCODE_Q);
				m_InputMappings[InputElements::INPUT_WEAPON_RELOAD].SetKey(SDL_SCANCODE_R);
				m_InputMappings[InputElements::INPUT_WEAPON_PICKUP].SetKey(SDL_SCANCODE_F);
				m_InputMappings[InputElements::INPUT_WEAPON_DROP].SetKey(SDL_SCANCODE_G);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV].SetKey(SDL_SCANCODE_1);
				m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT].SetKey(SDL_SCANCODE_2);
				break;
			case InputPreset::PresetGenericDPad:
				// TODO: Don't have any SNES style controllers to test with so no idea what would work or make sense here.
				m_InputMappings[InputElements::INPUT_L_UP].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
				m_InputMappings[InputElements::INPUT_AIM_UP].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				m_InputMappings[InputElements::INPUT_FIRE].SetJoyButton(SDL_CONTROLLER_BUTTON_B);
				m_InputMappings[InputElements::INPUT_AIM].SetJoyButton(SDL_CONTROLLER_BUTTON_Y);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetJoyButton(SDL_CONTROLLER_BUTTON_A);
				m_InputMappings[InputElements::INPUT_JUMP].SetJoyButton(SDL_CONTROLLER_BUTTON_X);
				m_InputMappings[InputElements::INPUT_NEXT].SetJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
				m_InputMappings[InputElements::INPUT_PREV].SetJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
				m_InputMappings[InputElements::INPUT_START].SetJoyButton(SDL_CONTROLLER_BUTTON_START);
				m_InputMappings[InputElements::INPUT_BACK].SetJoyButton(SDL_CONTROLLER_BUTTON_BACK);
				//m_InputMappings[InputElements::INPUT_WEAPON_RELOAD].SetKey();
				//m_InputMappings[InputElements::INPUT_WEAPON_PICKUP].SetKey();
				//m_InputMappings[InputElements::INPUT_WEAPON_DROP].SetKey();
				//m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_PREV].SetKey();
				//m_InputMappings[InputElements::INPUT_WEAPON_CHANGE_NEXT].SetKey();
				break;
			case InputPreset::PresetGenericDualAnalog:
				SetPreset(InputPreset::PresetGamepadXbox360);
				break;
			case InputPreset::PresetGamepadSNES:
				// TODO: Add a SNES style d-pad preset that makes sense. Use generic for now.
				SetPreset(InputPreset::PresetGenericDPad);
				break;
			case InputPreset::PresetGamepadDS4:
				m_InputMappings[InputElements::INPUT_FIRE].SetJoyButton(SDL_CONTROLLER_BUTTON_A);
				m_InputMappings[InputElements::INPUT_FIRE].SetPresetDescription("Cross Button");
				// Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping.
				m_InputMappings[InputElements::INPUT_AIM].SetJoyButton(SDL_CONTROLLER_BUTTON_X);
				m_InputMappings[InputElements::INPUT_AIM].SetPresetDescription("Square Button");
				// Pie menu also cancels buy menu, which makes sense for the B button.
				m_InputMappings[InputElements::INPUT_PIEMENU].SetJoyButton(SDL_CONTROLLER_BUTTON_B);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetPresetDescription("Circle Button");
				m_InputMappings[InputElements::INPUT_JUMP].SetJoyButton(SDL_CONTROLLER_BUTTON_Y);
				m_InputMappings[InputElements::INPUT_JUMP].SetPresetDescription("Triangle Button");
				m_InputMappings[InputElements::INPUT_NEXT].SetJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
				m_InputMappings[InputElements::INPUT_NEXT].SetPresetDescription("R. Bumper");
				m_InputMappings[InputElements::INPUT_PREV].SetJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
				m_InputMappings[InputElements::INPUT_PREV].SetPresetDescription("L. Bumper");
				m_InputMappings[InputElements::INPUT_START].SetJoyButton(SDL_CONTROLLER_BUTTON_START);
				m_InputMappings[InputElements::INPUT_START].SetPresetDescription("Start Button");
				m_InputMappings[InputElements::INPUT_BACK].SetJoyButton(SDL_CONTROLLER_BUTTON_BACK);
				m_InputMappings[InputElements::INPUT_BACK].SetPresetDescription("Select Button");
				// Set up the default xbox joy direction bindings.
				m_InputMappings[InputElements::INPUT_L_UP].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_L_UP].SetPresetDescription("L. Stick Up");
				m_InputMappings[InputElements::INPUT_L_DOWN].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetPresetDescription("L. Stick Down");
				m_InputMappings[InputElements::INPUT_L_LEFT].SetDirection(SDL_CONTROLLER_AXIS_LEFTX, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetPresetDescription("L. Stick Left");
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetDirection(SDL_CONTROLLER_AXIS_LEFTX, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetPresetDescription("L. Stick Right");
				m_InputMappings[InputElements::INPUT_R_UP].SetDirection(SDL_CONTROLLER_AXIS_RIGHTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_R_UP].SetPresetDescription("R. Stick Up");
				m_InputMappings[InputElements::INPUT_R_DOWN].SetDirection(SDL_CONTROLLER_AXIS_RIGHTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_R_DOWN].SetPresetDescription("R. Stick Down");
				m_InputMappings[InputElements::INPUT_R_LEFT].SetDirection(SDL_CONTROLLER_AXIS_RIGHTX, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_R_LEFT].SetPresetDescription("R. Stick Left");
				m_InputMappings[InputElements::INPUT_R_RIGHT].SetDirection(SDL_CONTROLLER_AXIS_RIGHTX, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_R_RIGHT].SetPresetDescription("R. Stick Right");
				m_InputMappings[InputElements::INPUT_FIRE].SetDirection(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_FIRE].SetPresetDescription("R. Trigger");
				m_InputMappings[InputElements::INPUT_PIEMENU].SetDirection(SDL_CONTROLLER_AXIS_TRIGGERLEFT, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetPresetDescription("L. Trigger");
				m_InputMappings[InputElements::INPUT_JUMP].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_JUMP].SetPresetDescription("L. Stick Up");
				m_InputMappings[InputElements::INPUT_CROUCH].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_CROUCH].SetPresetDescription("L. Stick Down");
				// Fine aiming can be done with the d-pad while holding down X.
				m_InputMappings[InputElements::INPUT_AIM_UP].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
				m_InputMappings[InputElements::INPUT_AIM_UP].SetPresetDescription("D-Pad Up");
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetPresetDescription("D-Pad Down");
				m_InputMappings[InputElements::INPUT_AIM_LEFT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				m_InputMappings[InputElements::INPUT_AIM_LEFT].SetPresetDescription("D-Pad Left");
				m_InputMappings[InputElements::INPUT_AIM_RIGHT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
				m_InputMappings[InputElements::INPUT_AIM_RIGHT].SetPresetDescription("D-Pad Right");
				break;
			case InputPreset::PresetGamepadXbox360:
				m_InputMappings[InputElements::INPUT_FIRE].SetJoyButton(SDL_CONTROLLER_BUTTON_A);
				m_InputMappings[InputElements::INPUT_FIRE].SetPresetDescription("A Button");
				// Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping.
				m_InputMappings[InputElements::INPUT_AIM].SetJoyButton(SDL_CONTROLLER_BUTTON_X);
				m_InputMappings[InputElements::INPUT_AIM].SetPresetDescription("X Button");
				// Pie menu also cancels buy menu, which makes sense for the B button.
				m_InputMappings[InputElements::INPUT_PIEMENU].SetJoyButton(SDL_CONTROLLER_BUTTON_B);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetPresetDescription("B Button");
				m_InputMappings[InputElements::INPUT_JUMP].SetJoyButton(SDL_CONTROLLER_BUTTON_Y);
				m_InputMappings[InputElements::INPUT_JUMP].SetPresetDescription("Y Button");
				m_InputMappings[InputElements::INPUT_NEXT].SetJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
				m_InputMappings[InputElements::INPUT_NEXT].SetPresetDescription("R. Bumper");
				m_InputMappings[InputElements::INPUT_PREV].SetJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
				m_InputMappings[InputElements::INPUT_PREV].SetPresetDescription("L. Bumper");
				m_InputMappings[InputElements::INPUT_START].SetJoyButton(SDL_CONTROLLER_BUTTON_START);
				m_InputMappings[InputElements::INPUT_START].SetPresetDescription("Start Button");
				m_InputMappings[InputElements::INPUT_BACK].SetJoyButton(SDL_CONTROLLER_BUTTON_BACK);
				m_InputMappings[InputElements::INPUT_BACK].SetPresetDescription("Back Button");
				// Set up the default xbox joy direction bindings.
				m_InputMappings[InputElements::INPUT_L_UP].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_L_UP].SetPresetDescription("L. Stick Up");
				m_InputMappings[InputElements::INPUT_L_DOWN].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_L_DOWN].SetPresetDescription("L. Stick Down");
				m_InputMappings[InputElements::INPUT_L_LEFT].SetDirection(SDL_CONTROLLER_AXIS_LEFTX, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_L_LEFT].SetPresetDescription("L. Stick Left");
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetDirection(SDL_CONTROLLER_AXIS_LEFTX, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_L_RIGHT].SetPresetDescription("L. Stick Right");
				m_InputMappings[InputElements::INPUT_R_UP].SetDirection(SDL_CONTROLLER_AXIS_RIGHTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_R_UP].SetPresetDescription("R. Stick Up");
				m_InputMappings[InputElements::INPUT_R_DOWN].SetDirection(SDL_CONTROLLER_AXIS_RIGHTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_R_DOWN].SetPresetDescription("R. Stick Down");
				m_InputMappings[InputElements::INPUT_R_LEFT].SetDirection(SDL_CONTROLLER_AXIS_RIGHTX, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_R_LEFT].SetPresetDescription("R. Stick Left");
				m_InputMappings[InputElements::INPUT_R_RIGHT].SetDirection(SDL_CONTROLLER_AXIS_RIGHTX, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_R_RIGHT].SetPresetDescription("R. Stick Right");
				m_InputMappings[InputElements::INPUT_FIRE].SetDirection(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_FIRE].SetPresetDescription("R. Trigger");
				m_InputMappings[InputElements::INPUT_PIEMENU].SetDirection(SDL_CONTROLLER_AXIS_TRIGGERLEFT, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_PIEMENU].SetPresetDescription("L. Trigger");
				m_InputMappings[InputElements::INPUT_JUMP].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_ONE);
				m_InputMappings[InputElements::INPUT_JUMP].SetPresetDescription("L. Stick Up");
				m_InputMappings[InputElements::INPUT_CROUCH].SetDirection(SDL_CONTROLLER_AXIS_LEFTY, JoyDirections::JOYDIR_TWO);
				m_InputMappings[InputElements::INPUT_CROUCH].SetPresetDescription("L. Stick Down");
				// Fine aiming can be done with the d-pad while holding down X.
				m_InputMappings[InputElements::INPUT_AIM_UP].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
				m_InputMappings[InputElements::INPUT_AIM_UP].SetPresetDescription("D-Pad Up");
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
				m_InputMappings[InputElements::INPUT_AIM_DOWN].SetPresetDescription("D-Pad Down");
				m_InputMappings[InputElements::INPUT_AIM_LEFT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				m_InputMappings[InputElements::INPUT_AIM_LEFT].SetPresetDescription("D-Pad Left");
				m_InputMappings[InputElements::INPUT_AIM_RIGHT].SetJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
				m_InputMappings[InputElements::INPUT_AIM_RIGHT].SetPresetDescription("D-Pad Right");
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string InputScheme::GetMappingName(int whichElement) const {
		const InputMapping *inputElement = &(m_InputMappings.at(whichElement));
		if (m_SchemePreset != InputScheme::InputPreset::NoPreset && !inputElement->GetPresetDescription().empty()) {
			return inputElement->GetPresetDescription();
		}
		if (m_ActiveDevice == InputDevice::DEVICE_KEYB_ONLY || (m_ActiveDevice == InputDevice::DEVICE_MOUSE_KEYB && (!(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN || whichElement == InputElements::INPUT_AIM_LEFT || whichElement == InputElements::INPUT_AIM_RIGHT))) && inputElement->GetKey() != 0) {
			std::string keyName = SDL_GetScancodeName(static_cast<SDL_Scancode>(inputElement->GetKey()));
			return (keyName != "(none)") ? keyName : "";
		} else if (m_ActiveDevice == InputDevice::DEVICE_MOUSE_KEYB && inputElement->GetMouseButton() != MouseButtons::MOUSE_NONE) {
			switch (inputElement->GetMouseButton()) {
				case MouseButtons::MOUSE_LEFT:
					return "Mouse Left";
				case MouseButtons::MOUSE_RIGHT:
					return "Mouse Right";
				case MouseButtons::MOUSE_MIDDLE:
					return "Mouse Middle";
				default:
					return "";
			}
		} else if (m_ActiveDevice >= InputDevice::DEVICE_GAMEPAD_1) {
			if (inputElement->GetJoyButton() != JoyButtons::JOY_NONE) {
				if (g_UInputMan.GetJoystickCount() > 0) {
					std::string buttonName = SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(inputElement->GetJoyButton()));
					return (buttonName != "unused") ? buttonName : "";
				} else {
					// If no joysticks connected the joystick handler can't resolve names because it's not installed, so just return generics (which it does anyway).
					return "Button " + std::to_string(inputElement->GetJoyButton() + 1);
				}
			} else if (inputElement->JoyDirMapped()) {
				std::string axisName = SDL_GameControllerGetStringForAxis(static_cast<SDL_GameControllerAxis>(inputElement->GetAxis()));
				axisName += inputElement->GetDirection() == JoyDirections::JOYDIR_ONE ? '-' : '+';
				return axisName;
			}
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool InputScheme::CaptureKeyMapping(int whichInput) {
		for (int whichKey = SDL_SCANCODE_A; whichKey < SDL_NUM_SCANCODES; ++whichKey) {
			// Don't allow mapping special keys used by UInputMan.
			if (whichKey == SDL_SCANCODE_ESCAPE || whichKey == SDL_SCANCODE_GRAVE || whichKey == SDL_SCANCODE_PRINTSCREEN || whichKey == SDL_SCANCODE_F1 || whichKey == SDL_SCANCODE_F2 || whichKey == SDL_SCANCODE_F3 || whichKey == SDL_SCANCODE_F4 || whichKey == SDL_SCANCODE_F5) {
				continue;
			}
			if (g_UInputMan.KeyReleased(static_cast<SDL_Scancode>(whichKey))) {
				m_InputMappings.at(whichInput).Reset();
				SetKeyMapping(whichInput, whichKey);
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool InputScheme::CaptureJoystickMapping(int whichJoy, int whichInput) {
		if (whichJoy < 0) {
			return false;
		}
		if (int whichButton = g_UInputMan.WhichJoyButtonPressed(whichJoy); whichButton != JoyButtons::JOY_NONE) {
			m_InputMappings.at(whichInput).Reset();
			SetJoyButtonMapping(whichInput, whichButton);
			return true;
		}

		for (int axis = 0; axis < g_UInputMan.GetJoystickAxisCount(whichJoy); ++axis) {
			if (g_UInputMan.JoyDirectionReleased(whichJoy, axis, JoyDirections::JOYDIR_ONE)) {
				m_InputMappings.at(whichInput).Reset();
				m_InputMappings.at(whichInput).SetDirection(axis, JoyDirections::JOYDIR_ONE);
				return true;
			} else if (g_UInputMan.JoyDirectionReleased(whichJoy, axis, JoyDirections::JOYDIR_TWO)) {
				m_InputMappings.at(whichInput).Reset();
				m_InputMappings.at(whichInput).SetDirection(axis, JoyDirections::JOYDIR_TWO);
				return true;
			}
		}
		return false;
	}
}
