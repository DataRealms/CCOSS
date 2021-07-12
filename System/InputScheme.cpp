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

		for (int inputMapping = 0; inputMapping < InputElements::INPUT_COUNT; ++inputMapping) {
			m_InputMappings.at(inputMapping).Create(reference.m_InputMappings.at(inputMapping));
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
			reader >> m_InputMappings.at(InputElements::INPUT_L_UP);
		} else if (propName == "LeftDown") {
			reader >> m_InputMappings.at(InputElements::INPUT_L_DOWN);
		} else if (propName == "LeftLeft") {
			reader >> m_InputMappings.at(InputElements::INPUT_L_LEFT);
		} else if (propName == "LeftRight") {
			reader >> m_InputMappings.at(InputElements::INPUT_L_RIGHT);
		} else if (propName == "Fire") {
			reader >> m_InputMappings.at(InputElements::INPUT_FIRE);
		} else if (propName == "Aim") {
			reader >> m_InputMappings.at(InputElements::INPUT_AIM);
		} else if (propName == "AimUp") {
			reader >> m_InputMappings.at(InputElements::INPUT_AIM_UP);
		} else if (propName == "AimDown") {
			reader >> m_InputMappings.at(InputElements::INPUT_AIM_DOWN);
		} else if (propName == "AimLeft") {
			reader >> m_InputMappings.at(InputElements::INPUT_AIM_LEFT);
		} else if (propName == "AimRight") {
			reader >> m_InputMappings.at(InputElements::INPUT_AIM_RIGHT);
		} else if (propName == "PieMenu") {
			reader >> m_InputMappings.at(InputElements::INPUT_PIEMENU);
		} else if (propName == "Jump") {
			reader >> m_InputMappings.at(InputElements::INPUT_JUMP);
		} else if (propName == "Crouch") {
			reader >> m_InputMappings.at(InputElements::INPUT_CROUCH);
		} else if (propName == "Next") {
			reader >> m_InputMappings.at(InputElements::INPUT_NEXT);
		} else if (propName == "Prev") {
			reader >> m_InputMappings.at(InputElements::INPUT_PREV);
		} else if (propName == "WeaponChangeNext") {
			reader >> m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT);
		} else if (propName == "WeaponChangePrev") {
			reader >> m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV);
		} else if (propName == "WeaponPickup") {
			reader >> m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP);
		} else if (propName == "WeaponDrop") {
			reader >> m_InputMappings.at(InputElements::INPUT_WEAPON_DROP);
		} else if (propName == "WeaponReload") {
			reader >> m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD);
		} else if (propName == "Start") {
			reader >> m_InputMappings.at(InputElements::INPUT_START);
		} else if (propName == "Back") {
			reader >> m_InputMappings.at(InputElements::INPUT_BACK);
		} else if (propName == "RightUp") {
			reader >> m_InputMappings.at(InputElements::INPUT_R_UP);
		} else if (propName == "RightDown") {
			reader >> m_InputMappings.at(InputElements::INPUT_R_DOWN);
		} else if (propName == "RightLeft") {
			reader >> m_InputMappings.at(InputElements::INPUT_R_LEFT);
		} else if (propName == "RightRight") {
			reader >> m_InputMappings.at(InputElements::INPUT_R_RIGHT);
		} else if (propName == "JoystickDeadzoneType") {
			SetJoystickDeadzoneType(static_cast<DeadZoneType>(std::stoi(reader.ReadPropValue())));
		} else if (propName == "JoystickDeadzone") {
			reader >> m_JoystickDeadzone;
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
			writer.NewPropertyWithValue("LeftUp", m_InputMappings.at(InputElements::INPUT_L_UP));
			writer.NewPropertyWithValue("LeftDown", m_InputMappings.at(InputElements::INPUT_L_DOWN));
			writer.NewPropertyWithValue("LeftLeft", m_InputMappings.at(InputElements::INPUT_L_LEFT));
			writer.NewPropertyWithValue("LeftRight", m_InputMappings.at(InputElements::INPUT_L_RIGHT));
			writer.NewPropertyWithValue("Fire", m_InputMappings.at(InputElements::INPUT_FIRE));
			writer.NewPropertyWithValue("Aim", m_InputMappings.at(InputElements::INPUT_AIM));
			writer.NewPropertyWithValue("AimUp", m_InputMappings.at(InputElements::INPUT_AIM_UP));
			writer.NewPropertyWithValue("AimDown", m_InputMappings.at(InputElements::INPUT_AIM_DOWN));
			writer.NewPropertyWithValue("AimLeft", m_InputMappings.at(InputElements::INPUT_AIM_LEFT));
			writer.NewPropertyWithValue("AimRight", m_InputMappings.at(InputElements::INPUT_AIM_RIGHT));
			writer.NewPropertyWithValue("PieMenu", m_InputMappings.at(InputElements::INPUT_PIEMENU));
			writer.NewPropertyWithValue("Jump", m_InputMappings.at(InputElements::INPUT_JUMP));
			writer.NewPropertyWithValue("Crouch", m_InputMappings.at(InputElements::INPUT_CROUCH));
			writer.NewPropertyWithValue("Next", m_InputMappings.at(InputElements::INPUT_NEXT));
			writer.NewPropertyWithValue("Prev", m_InputMappings.at(InputElements::INPUT_PREV));
			writer.NewPropertyWithValue("WeaponChangeNext", m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT));
			writer.NewPropertyWithValue("WeaponChangePrev", m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV));
			writer.NewPropertyWithValue("WeaponPickup", m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP));
			writer.NewPropertyWithValue("WeaponDrop", m_InputMappings.at(InputElements::INPUT_WEAPON_DROP));
			writer.NewPropertyWithValue("WeaponReload", m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD));
			writer.NewPropertyWithValue("Start", m_InputMappings.at(InputElements::INPUT_START));
			writer.NewPropertyWithValue("Back", m_InputMappings.at(InputElements::INPUT_BACK));
			writer.NewPropertyWithValue("RightUp", m_InputMappings.at(InputElements::INPUT_R_UP));
			writer.NewPropertyWithValue("RightDown", m_InputMappings.at(InputElements::INPUT_R_DOWN));
			writer.NewPropertyWithValue("RightLeft", m_InputMappings.at(InputElements::INPUT_R_LEFT));
			writer.NewPropertyWithValue("RightRight", m_InputMappings.at(InputElements::INPUT_R_RIGHT));
		}

		writer.NewPropertyWithValue("JoystickDeadzoneType", m_JoystickDeadzoneType);
		writer.NewPropertyWithValue("JoystickDeadzone", m_JoystickDeadzone);

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
				m_InputMappings.at(InputElements::INPUT_L_UP).SetKey(KEY_UP);
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetKey(KEY_DOWN);
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetKey(KEY_LEFT);
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetKey(KEY_RIGHT);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetKey(KEY_1_PAD);
				m_InputMappings.at(InputElements::INPUT_AIM).SetKey(KEY_2_PAD);
				m_InputMappings.at(InputElements::INPUT_AIM_UP).SetKey(KEY_UP);
				m_InputMappings.at(InputElements::INPUT_AIM_DOWN).SetKey(KEY_DOWN);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetKey(KEY_3_PAD);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetKey(KEY_ENTER_PAD);
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetKey(KEY_0_PAD);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetKey(KEY_5_PAD);
				m_InputMappings.at(InputElements::INPUT_PREV).SetKey(KEY_4_PAD);
				m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD).SetKey(KEY_DEL_PAD);
				m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP).SetKey(KEY_6_PAD);
				m_InputMappings.at(InputElements::INPUT_WEAPON_DROP).SetKey(KEY_9_PAD);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV).SetKey(KEY_7_PAD);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT).SetKey(KEY_8_PAD);
				break;
			case InputPreset::PresetWASDKeys:
				m_InputMappings.at(InputElements::INPUT_L_UP).SetKey(KEY_W);
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetKey(KEY_S);
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetKey(KEY_A);
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetKey(KEY_D);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetKey(KEY_H);
				m_InputMappings.at(InputElements::INPUT_AIM).SetKey(KEY_J);
				m_InputMappings.at(InputElements::INPUT_AIM_UP).SetKey(KEY_W);
				m_InputMappings.at(InputElements::INPUT_AIM_DOWN).SetKey(KEY_S);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetKey(KEY_K);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetKey(KEY_L);
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetKey(KEY_C);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetKey(KEY_U);
				m_InputMappings.at(InputElements::INPUT_PREV).SetKey(KEY_Y);
				m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD).SetKey(KEY_R);
				m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP).SetKey(KEY_F);
				m_InputMappings.at(InputElements::INPUT_WEAPON_DROP).SetKey(KEY_B);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV).SetKey(KEY_Q);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT).SetKey(KEY_E);
				break;
			case InputPreset::PresetMouseWASDKeys:
				m_InputMappings.at(InputElements::INPUT_L_UP).SetKey(KEY_W);
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetKey(KEY_S);
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetKey(KEY_A);
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetKey(KEY_D);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetMouseButton(MouseButtons::MOUSE_LEFT);
				m_InputMappings.at(InputElements::INPUT_AIM).SetPresetDescription("Mouse Move");
				m_InputMappings.at(InputElements::INPUT_AIM_UP).SetPresetDescription("Mouse Move");
				m_InputMappings.at(InputElements::INPUT_AIM_DOWN).SetPresetDescription("Mouse Move");
				m_InputMappings.at(InputElements::INPUT_AIM_LEFT).SetPresetDescription("Mouse Move");
				m_InputMappings.at(InputElements::INPUT_AIM_RIGHT).SetPresetDescription("Mouse Move");
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetMouseButton(MouseButtons::MOUSE_RIGHT);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetKey(KEY_W);
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetKey(KEY_S);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetKey(KEY_E);
				m_InputMappings.at(InputElements::INPUT_PREV).SetKey(KEY_Q);
				m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD).SetKey(KEY_R);
				m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP).SetKey(KEY_F);
				m_InputMappings.at(InputElements::INPUT_WEAPON_DROP).SetKey(KEY_G);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV).SetKey(KEY_1);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT).SetKey(KEY_2);
				break;
			case InputPreset::PresetGenericDPad:
				// TODO: Don't have any SNES style controllers to test with so no idea what would work or make sense here.
				//m_InputMappings.at(InputElements::INPUT_L_UP).SetKey();
				//m_InputMappings.at(InputElements::INPUT_L_DOWN).SetKey();
				//m_InputMappings.at(InputElements::INPUT_L_LEFT).SetKey();
				//m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetKey();
				m_InputMappings.at(InputElements::INPUT_FIRE).SetJoyButton(JoyButtons::JOY_1);
				m_InputMappings.at(InputElements::INPUT_AIM).SetJoyButton(JoyButtons::JOY_2);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetJoyButton(JoyButtons::JOY_3);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetJoyButton(JoyButtons::JOY_4);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetJoyButton(JoyButtons::JOY_6);
				m_InputMappings.at(InputElements::INPUT_PREV).SetJoyButton(JoyButtons::JOY_5);
				m_InputMappings.at(InputElements::INPUT_START).SetJoyButton(JoyButtons::JOY_8);
				m_InputMappings.at(InputElements::INPUT_BACK).SetJoyButton(JoyButtons::JOY_7);
				//m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD).SetKey();
				//m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP).SetKey();
				//m_InputMappings.at(InputElements::INPUT_WEAPON_DROP).SetKey();
				//m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV).SetKey();
				//m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT).SetKey();
				break;
			case InputPreset::PresetGenericDualAnalog:
				// TODO: Test if this makes sense.
				m_InputMappings.at(InputElements::INPUT_FIRE).SetJoyButton(JoyButtons::JOY_1);
				m_InputMappings.at(InputElements::INPUT_AIM).SetJoyButton(JoyButtons::JOY_2);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetJoyButton(JoyButtons::JOY_3);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetJoyButton(JoyButtons::JOY_4);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetJoyButton(JoyButtons::JOY_6);
				m_InputMappings.at(InputElements::INPUT_PREV).SetJoyButton(JoyButtons::JOY_5);
				m_InputMappings.at(InputElements::INPUT_START).SetJoyButton(JoyButtons::JOY_8);
				m_InputMappings.at(InputElements::INPUT_BACK).SetJoyButton(JoyButtons::JOY_7);

				m_InputMappings.at(InputElements::INPUT_L_UP).SetDirection(0, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetDirection(0, 1, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetDirection(0, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetDirection(0, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_R_UP).SetDirection(1, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_R_DOWN).SetDirection(1, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_R_LEFT).SetDirection(2, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_R_RIGHT).SetDirection(2, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetDirection(0, 2, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetDirection(0, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetDirection(0, 1, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetDirection(0, 2, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_PREV).SetDirection(3, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_WEAPON_CHANGE_NEXT).SetDirection(3, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_WEAPON_RELOAD).SetDirection(3, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_WEAPON_PICKUP).SetDirection(3, 1, JoyDirections::JOYDIR_TWO);
				break;
			case InputPreset::PresetGamepadSNES:
				// TODO: Add a SNES style d-pad preset that makes sense. Use generic for now.
				SetPreset(InputPreset::PresetGenericDPad);
				break;
			case InputPreset::PresetGamepadDS4:
				// TODO: Add a PS4 preset that makes sense. Use generic for now.
				SetPreset(InputPreset::PresetGenericDualAnalog);
				break;
			case InputPreset::PresetGamepadXbox360:
				m_InputMappings.at(InputElements::INPUT_FIRE).SetJoyButton(JoyButtons::JOY_1);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetPresetDescription("A Button");
				// Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping.
				m_InputMappings.at(InputElements::INPUT_AIM).SetJoyButton(JoyButtons::JOY_3);
				m_InputMappings.at(InputElements::INPUT_AIM).SetPresetDescription("X Button");
				// Pie menu also cancels buy menu, which makes sense for the B button.
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetJoyButton(JoyButtons::JOY_2);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetPresetDescription("B Button");
				m_InputMappings.at(InputElements::INPUT_JUMP).SetJoyButton(JoyButtons::JOY_4);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetPresetDescription("Y Button");
				m_InputMappings.at(InputElements::INPUT_NEXT).SetJoyButton(JoyButtons::JOY_6);
				m_InputMappings.at(InputElements::INPUT_NEXT).SetPresetDescription("R. Bumper");
				m_InputMappings.at(InputElements::INPUT_PREV).SetJoyButton(JoyButtons::JOY_5);
				m_InputMappings.at(InputElements::INPUT_PREV).SetPresetDescription("L. Bumper");
				m_InputMappings.at(InputElements::INPUT_START).SetJoyButton(JoyButtons::JOY_8);
				m_InputMappings.at(InputElements::INPUT_START).SetPresetDescription("Start Button");
				m_InputMappings.at(InputElements::INPUT_BACK).SetJoyButton(JoyButtons::JOY_7);
				m_InputMappings.at(InputElements::INPUT_BACK).SetPresetDescription("Back Button");
				// Set up the default xbox joy direction bindings.
				m_InputMappings.at(InputElements::INPUT_L_UP).SetDirection(0, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_L_UP).SetPresetDescription("L. Stick Up");
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetDirection(0, 1, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_L_DOWN).SetPresetDescription("L. Stick Down");
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetDirection(0, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_L_LEFT).SetPresetDescription("L. Stick Left");
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetDirection(0, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_L_RIGHT).SetPresetDescription("L. Stick Right");
				m_InputMappings.at(InputElements::INPUT_R_UP).SetDirection(1, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_R_UP).SetPresetDescription("R. Stick Up");
				m_InputMappings.at(InputElements::INPUT_R_DOWN).SetDirection(1, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_R_DOWN).SetPresetDescription("R. Stick Down");
				m_InputMappings.at(InputElements::INPUT_R_LEFT).SetDirection(2, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_R_LEFT).SetPresetDescription("R. Stick Left");
				m_InputMappings.at(InputElements::INPUT_R_RIGHT).SetDirection(2, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_R_RIGHT).SetPresetDescription("R. Stick Right");
				m_InputMappings.at(InputElements::INPUT_FIRE).SetDirection(0, 2, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_FIRE).SetPresetDescription("R. Trigger");
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetDirection(0, 2, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_PIEMENU).SetPresetDescription("L. Trigger");
				m_InputMappings.at(InputElements::INPUT_JUMP).SetDirection(0, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_JUMP).SetPresetDescription("L. Stick Up");
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetDirection(0, 1, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_CROUCH).SetPresetDescription("L. Stick Down");
				// Fine aiming can be done with the d-pad while holding down X.
				m_InputMappings.at(InputElements::INPUT_AIM_UP).SetDirection(3, 1, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_AIM_UP).SetPresetDescription("D-Pad Up");
				m_InputMappings.at(InputElements::INPUT_AIM_DOWN).SetDirection(3, 1, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_AIM_DOWN).SetPresetDescription("D-Pad Down");
				m_InputMappings.at(InputElements::INPUT_AIM_LEFT).SetDirection(3, 0, JoyDirections::JOYDIR_ONE);
				m_InputMappings.at(InputElements::INPUT_AIM_LEFT).SetPresetDescription("D-Pad Left");
				m_InputMappings.at(InputElements::INPUT_AIM_RIGHT).SetDirection(3, 0, JoyDirections::JOYDIR_TWO);
				m_InputMappings.at(InputElements::INPUT_AIM_RIGHT).SetPresetDescription("D-Pad Right");
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
			std::string keyName = scancode_to_name(inputElement->GetKey());
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
					std::string buttonName = joy[g_UInputMan.GetJoystickIndex(m_ActiveDevice)].button[inputElement->GetJoyButton()].name;
					return (buttonName != "unused") ? buttonName : "";
				} else {
					// If no joysticks connected the joystick handler can't resolve names because it's not installed, so just return generics (which it does anyway).
					return "Button " + std::to_string(inputElement->GetJoyButton() + 1);
				}
			} else if (inputElement->JoyDirMapped()) {
				return "Analog Stick";
			}
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool InputScheme::CaptureKeyMapping(int whichInput) {
		if (keyboard_needs_poll()) { poll_keyboard(); }

		for (int whichKey = KEY_A; whichKey < KEY_MAX; ++whichKey) {
			// Don't allow mapping special keys used by UInputMan.
			if (whichKey == KEY_ESC || whichKey == KEY_TILDE || whichKey == KEY_PRTSCR || whichKey == KEY_F1 || whichKey == KEY_F2 || whichKey == KEY_F3 || whichKey == KEY_F4 || whichKey == KEY_F5) {
				continue;
			}
			if (g_UInputMan.KeyReleased(static_cast<char>(whichKey))) {
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
		int whichButton = g_UInputMan.WhichJoyButtonPressed(whichJoy);
		if (whichButton != JoyButtons::JOY_NONE) {
			m_InputMappings.at(whichInput).Reset();
			SetJoyButtonMapping(whichInput, whichButton);
			return true;
		}
		for (int stick = 0; stick < joy[whichJoy].num_sticks; ++stick) {
			for (int axis = 0; axis < joy[whichJoy].stick[stick].num_axis; ++axis) {
				if (g_UInputMan.JoyDirectionReleased(whichJoy, stick, axis, JoyDirections::JOYDIR_ONE)) {
					m_InputMappings.at(whichInput).Reset();
					m_InputMappings.at(whichInput).SetDirection(stick, axis, JoyDirections::JOYDIR_ONE);
					return true;
				} else if (g_UInputMan.JoyDirectionReleased(whichJoy, stick, axis, JoyDirections::JOYDIR_TWO)) {
					m_InputMappings.at(whichInput).Reset();
					m_InputMappings.at(whichInput).SetDirection(stick, axis, JoyDirections::JOYDIR_TWO);
					return true;
				}
			}
		}
		return false;
	}
}