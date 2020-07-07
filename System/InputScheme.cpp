#include "InputScheme.h"
#include "allegro.h"

namespace RTE {

	const std::string InputScheme::c_ClassName = "InputScheme";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputScheme::Clear() {
		m_ActiveDevice = DEVICE_KEYB_ONLY;
		m_SchemePreset = PRESET_NONE;
		m_JoystickDeadzone = 0.01F;
		m_JoystickDeadzoneType = DeadZoneType::CIRCLE;

		for (int mapping = 0; mapping < INPUT_COUNT; ++mapping) {
			m_InputMapping[mapping].Reset();
		}

		// Set up the default mouse button bindings - do these here because there are no config steps for them yet
		m_InputMapping[INPUT_FIRE].SetMouseButton(MOUSE_LEFT);
		m_InputMapping[INPUT_PIEMENU].SetMouseButton(MOUSE_RIGHT);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::Create(const InputScheme &reference) {
		m_ActiveDevice = reference.m_ActiveDevice;
		m_SchemePreset = reference.m_SchemePreset;
		m_JoystickDeadzone = reference.m_JoystickDeadzone;
		m_JoystickDeadzoneType = reference.m_JoystickDeadzoneType;

		for (int mapping = 0; mapping < INPUT_COUNT; ++mapping) {
			m_InputMapping[mapping].Create(reference.m_InputMapping[mapping]);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "Device") {
			reader >> m_ActiveDevice;
		} else if (propName == "Preset") {
			reader >> m_SchemePreset;
			SetPreset(m_SchemePreset);
		} else if (propName == "LeftUp") {
			reader >> m_InputMapping[INPUT_L_UP];
		} else if (propName == "LeftDown") {
			reader >> m_InputMapping[INPUT_L_DOWN];
		} else if (propName == "LeftLeft") {
			reader >> m_InputMapping[INPUT_L_LEFT];
		} else if (propName == "LeftRight") {
			reader >> m_InputMapping[INPUT_L_RIGHT];
		} else if (propName == "RightUp") {
			reader >> m_InputMapping[INPUT_R_UP];
		} else if (propName == "RightDown") {
			reader >> m_InputMapping[INPUT_R_DOWN];
		} else if (propName == "RightLeft") {
			reader >> m_InputMapping[INPUT_R_LEFT];
		} else if (propName == "RightRight") {
			reader >> m_InputMapping[INPUT_R_RIGHT];
		} else if (propName == "Fire") {
			reader >> m_InputMapping[INPUT_FIRE];
		} else if (propName == "Aim") {
			reader >> m_InputMapping[INPUT_AIM];
		} else if (propName == "AimUp") {
			reader >> m_InputMapping[INPUT_AIM_UP];
		} else if (propName == "AimDown") {
			reader >> m_InputMapping[INPUT_AIM_DOWN];
		} else if (propName == "AimLeft") {
			reader >> m_InputMapping[INPUT_AIM_LEFT];
		} else if (propName == "AimRight") {
			reader >> m_InputMapping[INPUT_AIM_RIGHT];
		} else if (propName == "PieMenu") {
			reader >> m_InputMapping[INPUT_PIEMENU];
		} else if (propName == "Jump") {
			reader >> m_InputMapping[INPUT_JUMP];
		} else if (propName == "Crouch") {
			reader >> m_InputMapping[INPUT_CROUCH];
		} else if (propName == "Next") {
			reader >> m_InputMapping[INPUT_NEXT];
		} else if (propName == "Prev") {
			reader >> m_InputMapping[INPUT_PREV];
		} else if (propName == "Start") {
			reader >> m_InputMapping[INPUT_START];
		} else if (propName == "Back") {
			reader >> m_InputMapping[INPUT_BACK];
		} else if (propName == "WeaponChangeNext") {
			reader >> m_InputMapping[INPUT_WEAPON_CHANGE_NEXT];
		} else if (propName == "WeaponChangePrev") {
			reader >> m_InputMapping[INPUT_WEAPON_CHANGE_PREV];
		} else if (propName == "WeaponPickup") {
			reader >> m_InputMapping[INPUT_WEAPON_PICKUP];
		} else if (propName == "WeaponDrop") {
			reader >> m_InputMapping[INPUT_WEAPON_DROP];
		} else if (propName == "WeaponReload") {
			reader >> m_InputMapping[INPUT_WEAPON_RELOAD];
		} else if (propName == "JoystickDeadzone") {
			reader >> m_JoystickDeadzone;
		} else if (propName == "JoystickDeadzoneType") {
			reader >> m_JoystickDeadzoneType;
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int InputScheme::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("Device");
		writer << m_ActiveDevice;
		if (m_SchemePreset > PRESET_NONE) {
			writer.NewProperty("Preset");
			writer << m_SchemePreset;
		}
		writer.NewProperty("LeftUp");
		writer << m_InputMapping[INPUT_L_UP];
		writer.NewProperty("LeftDown");
		writer << m_InputMapping[INPUT_L_DOWN];
		writer.NewProperty("LeftLeft");
		writer << m_InputMapping[INPUT_L_LEFT];
		writer.NewProperty("LeftRight");
		writer << m_InputMapping[INPUT_L_RIGHT];
		writer.NewProperty("RightUp");
		writer << m_InputMapping[INPUT_R_UP];
		writer.NewProperty("RightDown");
		writer << m_InputMapping[INPUT_R_DOWN];
		writer.NewProperty("RightLeft");
		writer << m_InputMapping[INPUT_R_LEFT];
		writer.NewProperty("RightRight");
		writer << m_InputMapping[INPUT_R_RIGHT];
		writer.NewProperty("Fire");
		writer << m_InputMapping[INPUT_FIRE];
		writer.NewProperty("Aim");
		writer << m_InputMapping[INPUT_AIM];
		writer.NewProperty("AimUp");
		writer << m_InputMapping[INPUT_AIM_UP];
		writer.NewProperty("AimDown");
		writer << m_InputMapping[INPUT_AIM_DOWN];
		writer.NewProperty("AimLeft");
		writer << m_InputMapping[INPUT_AIM_LEFT];
		writer.NewProperty("AimRight");
		writer << m_InputMapping[INPUT_AIM_RIGHT];
		writer.NewProperty("PieMenu");
		writer << m_InputMapping[INPUT_PIEMENU];
		writer.NewProperty("Jump");
		writer << m_InputMapping[INPUT_JUMP];
		writer.NewProperty("Crouch");
		writer << m_InputMapping[INPUT_CROUCH];
		writer.NewProperty("Next");
		writer << m_InputMapping[INPUT_NEXT];
		writer.NewProperty("Prev");
		writer << m_InputMapping[INPUT_PREV];
		writer.NewProperty("Start");
		writer << m_InputMapping[INPUT_START];
		writer.NewProperty("Back");
		writer << m_InputMapping[INPUT_BACK];
		writer.NewProperty("WeaponChangeNext");
		writer << m_InputMapping[INPUT_WEAPON_CHANGE_NEXT];
		writer.NewProperty("WeaponChangePrev");
		writer << m_InputMapping[INPUT_WEAPON_CHANGE_PREV];
		writer.NewProperty("WeaponPickup");
		writer << m_InputMapping[INPUT_WEAPON_PICKUP];
		writer.NewProperty("WeaponDrop");
		writer << m_InputMapping[INPUT_WEAPON_DROP];
		writer.NewProperty("WeaponReload");
		writer << m_InputMapping[INPUT_WEAPON_RELOAD];
		writer.NewProperty("JoystickDeadzone");
		writer << m_JoystickDeadzone;
		writer.NewProperty("JoystickDeadzoneType");
		writer << m_JoystickDeadzoneType;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void InputScheme::SetPreset(int schemePreset) {
		m_SchemePreset = schemePreset;
		switch (m_SchemePreset) {
			case PRESET_XBOX360:
				// Set up the default xbox 360 button bindings
				m_InputMapping[INPUT_FIRE].SetJoyButton(JOY_1);
				m_InputMapping[INPUT_FIRE].SetPresetDesc("A Button");
				// Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping
				m_InputMapping[INPUT_AIM].SetJoyButton(JOY_3);
				m_InputMapping[INPUT_AIM].SetPresetDesc("X Button");
				// Pie menu also cancels buy menu, which makes sense for the B button
				m_InputMapping[INPUT_PIEMENU].SetJoyButton(JOY_2);
				m_InputMapping[INPUT_PIEMENU].SetPresetDesc("B Button");
				// Jump on top button of diamond makes sense
				m_InputMapping[INPUT_JUMP].SetJoyButton(JOY_4);
				m_InputMapping[INPUT_JUMP].SetPresetDesc("Y Button");
				m_InputMapping[INPUT_NEXT].SetJoyButton(JOY_6);
				m_InputMapping[INPUT_NEXT].SetPresetDesc("R Bumper Button");
				m_InputMapping[INPUT_PREV].SetJoyButton(JOY_5);
				m_InputMapping[INPUT_PREV].SetPresetDesc("L Bumper Button");
				m_InputMapping[INPUT_START].SetJoyButton(JOY_8);
				m_InputMapping[INPUT_START].SetPresetDesc("Start Button");
				m_InputMapping[INPUT_BACK].SetJoyButton(JOY_7);
				m_InputMapping[INPUT_BACK].SetPresetDesc("Back Button");
				// Set up the default xbox joy direction bindings
				m_InputMapping[INPUT_L_UP].SetDirection(0, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_L_UP].SetPresetDesc("L Thumbstick Up");
				m_InputMapping[INPUT_L_DOWN].SetDirection(0, 1, JOYDIR_TWO);
				m_InputMapping[INPUT_L_DOWN].SetPresetDesc("L Thumbstick Down");
				m_InputMapping[INPUT_L_LEFT].SetDirection(0, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_L_LEFT].SetPresetDesc("L Thumbstick Left");
				m_InputMapping[INPUT_L_RIGHT].SetDirection(0, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_L_RIGHT].SetPresetDesc("L Thumbstick Right");
				m_InputMapping[INPUT_R_UP].SetDirection(1, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_R_UP].SetPresetDesc("R Thumbstick Up");
				m_InputMapping[INPUT_R_DOWN].SetDirection(1, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_R_DOWN].SetPresetDesc("R Thumbstick Down");
				m_InputMapping[INPUT_R_LEFT].SetDirection(2, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_R_LEFT].SetPresetDesc("R Thumbstick Left");
				m_InputMapping[INPUT_R_RIGHT].SetDirection(2, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_R_RIGHT].SetPresetDesc("R Thumbstick Right");
				m_InputMapping[INPUT_FIRE].SetDirection(0, 2, JOYDIR_ONE);
				m_InputMapping[INPUT_FIRE].SetPresetDesc("R Trigger");
				m_InputMapping[INPUT_PIEMENU].SetDirection(0, 2, JOYDIR_TWO);
				m_InputMapping[INPUT_PIEMENU].SetPresetDesc("L Trigger");
				m_InputMapping[INPUT_JUMP].SetDirection(0, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_JUMP].SetPresetDesc("L Thumbstick Up");
				m_InputMapping[INPUT_CROUCH].SetDirection(0, 1, JOYDIR_TWO);
				m_InputMapping[INPUT_CROUCH].SetPresetDesc("L Thumbstick Down");
				// So fine aiming can be done with the d-pad while holding down X
				m_InputMapping[INPUT_AIM_UP].SetDirection(3, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_AIM_UP].SetPresetDesc("D-Pad Up");
				m_InputMapping[INPUT_AIM_DOWN].SetDirection(3, 1, JOYDIR_TWO);
				m_InputMapping[INPUT_AIM_DOWN].SetPresetDesc("D-Pad Down");
				m_InputMapping[INPUT_AIM_LEFT].SetDirection(3, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_AIM_LEFT].SetPresetDesc("D-Pad Left");
				m_InputMapping[INPUT_AIM_RIGHT].SetDirection(3, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_AIM_RIGHT].SetPresetDesc("D-Pad Right");
				break;

			case PRESET_P1DEFAULT:
				m_InputMapping[INPUT_L_UP].SetKey(KEY_W);
				m_InputMapping[INPUT_L_DOWN].SetKey(KEY_S);
				m_InputMapping[INPUT_L_LEFT].SetKey(KEY_A);
				m_InputMapping[INPUT_L_RIGHT].SetKey(KEY_D);
				m_InputMapping[INPUT_R_UP].SetKey(KEY_W);
				m_InputMapping[INPUT_R_DOWN].SetKey(KEY_S);
				m_InputMapping[INPUT_R_LEFT].SetKey(KEY_A);
				m_InputMapping[INPUT_R_RIGHT].SetKey(KEY_D);
				m_InputMapping[INPUT_FIRE].SetMouseButton(0);
				m_InputMapping[INPUT_PIEMENU].SetMouseButton(1);
				m_InputMapping[INPUT_JUMP].SetKey(KEY_W);
				m_InputMapping[INPUT_CROUCH].SetKey(KEY_S);
				m_InputMapping[INPUT_NEXT].SetKey(KEY_E);
				m_InputMapping[INPUT_PREV].SetKey(KEY_Q);
				m_InputMapping[INPUT_WEAPON_RELOAD].SetKey(KEY_R);
				m_InputMapping[INPUT_WEAPON_DROP].SetKey(KEY_G);
				m_InputMapping[INPUT_WEAPON_PICKUP].SetKey(KEY_F);
				m_InputMapping[INPUT_WEAPON_CHANGE_NEXT].SetKey(KEY_C);
				break;

			case PRESET_P2DEFAULT:
				m_InputMapping[INPUT_L_UP].SetKey(KEY_UP);
				m_InputMapping[INPUT_L_DOWN].SetKey(KEY_DOWN);
				m_InputMapping[INPUT_L_LEFT].SetKey(KEY_LEFT);
				m_InputMapping[INPUT_L_RIGHT].SetKey(KEY_RIGHT);
				m_InputMapping[INPUT_FIRE].SetKey(KEY_1_PAD);
				m_InputMapping[INPUT_AIM].SetKey(KEY_2_PAD);
				m_InputMapping[INPUT_AIM_UP].SetKey(KEY_UP);
				m_InputMapping[INPUT_AIM_DOWN].SetKey(KEY_DOWN);
				m_InputMapping[INPUT_PIEMENU].SetKey(KEY_3_PAD);
				m_InputMapping[INPUT_JUMP].SetKey(KEY_ENTER_PAD);
				m_InputMapping[INPUT_CROUCH].SetKey(KEY_DEL_PAD);
				m_InputMapping[INPUT_NEXT].SetKey(KEY_5_PAD);
				m_InputMapping[INPUT_PREV].SetKey(KEY_4_PAD);
				m_InputMapping[INPUT_WEAPON_RELOAD].SetKey(KEY_0_PAD);
				m_InputMapping[INPUT_WEAPON_DROP].SetKey(KEY_6_PAD);
				m_InputMapping[INPUT_WEAPON_PICKUP].SetKey(KEY_9_PAD);
				m_InputMapping[INPUT_WEAPON_CHANGE_PREV].SetKey(KEY_7_PAD);
				m_InputMapping[INPUT_WEAPON_CHANGE_NEXT].SetKey(KEY_8_PAD);
				break;

			case PRESET_P3DEFAULT:
				//TODO: Replace these with gamepad bindings.
				m_InputMapping[INPUT_L_UP].SetKey(KEY_W);
				m_InputMapping[INPUT_L_DOWN].SetKey(KEY_S);
				m_InputMapping[INPUT_L_LEFT].SetKey(KEY_A);
				m_InputMapping[INPUT_L_RIGHT].SetKey(KEY_D);
				m_InputMapping[INPUT_R_UP].SetKey(KEY_W);
				m_InputMapping[INPUT_R_DOWN].SetKey(KEY_S);
				m_InputMapping[INPUT_R_LEFT].SetKey(KEY_A);
				m_InputMapping[INPUT_R_RIGHT].SetKey(KEY_D);
				m_InputMapping[INPUT_FIRE].SetKey(KEY_H);
				m_InputMapping[INPUT_AIM].SetKey(KEY_J);
				m_InputMapping[INPUT_AIM_UP].SetKey(KEY_W);
				m_InputMapping[INPUT_AIM_DOWN].SetKey(KEY_S);
				m_InputMapping[INPUT_PIEMENU].SetKey(KEY_K);
				m_InputMapping[INPUT_JUMP].SetKey(KEY_L);
				m_InputMapping[INPUT_CROUCH].SetKey(KEY_STOP);
				m_InputMapping[INPUT_NEXT].SetKey(KEY_U);
				m_InputMapping[INPUT_PREV].SetKey(KEY_Y);
				break;

			case PRESET_P4DEFAULT:
				//TODO: Replace these with gamepad bindings.
				m_InputMapping[INPUT_L_UP].SetKey(KEY_UP);
				m_InputMapping[INPUT_L_DOWN].SetKey(KEY_DOWN);
				m_InputMapping[INPUT_L_LEFT].SetKey(KEY_LEFT);
				m_InputMapping[INPUT_L_RIGHT].SetKey(KEY_RIGHT);
				m_InputMapping[INPUT_R_UP].SetKey(KEY_UP);
				m_InputMapping[INPUT_R_DOWN].SetKey(KEY_DOWN);
				m_InputMapping[INPUT_R_LEFT].SetKey(KEY_LEFT);
				m_InputMapping[INPUT_R_RIGHT].SetKey(KEY_RIGHT);
				m_InputMapping[INPUT_FIRE].SetKey(KEY_1_PAD);
				m_InputMapping[INPUT_AIM].SetKey(KEY_2_PAD);
				m_InputMapping[INPUT_AIM_UP].SetKey(KEY_UP);
				m_InputMapping[INPUT_AIM_DOWN].SetKey(KEY_DOWN);
				m_InputMapping[INPUT_PIEMENU].SetKey(KEY_3_PAD);
				m_InputMapping[INPUT_JUMP].SetKey(KEY_DEL_PAD);
				m_InputMapping[INPUT_CROUCH].SetKey(KEY_STOP);
				m_InputMapping[INPUT_NEXT].SetKey(KEY_5_PAD);
				m_InputMapping[INPUT_PREV].SetKey(KEY_4_PAD);
				break;

			// Some generic defaults; no real preset is set
			default:
				m_SchemePreset = PRESET_NONE;
				// Set up the default mouse button bindings
				m_InputMapping[INPUT_FIRE].SetMouseButton(MOUSE_LEFT);
				m_InputMapping[INPUT_PIEMENU].SetMouseButton(MOUSE_RIGHT);
				// Set up the default joystick button bindings
				m_InputMapping[INPUT_FIRE].SetJoyButton(JOY_1);
				m_InputMapping[INPUT_AIM].SetJoyButton(JOY_2);
				m_InputMapping[INPUT_PIEMENU].SetJoyButton(JOY_3);
				m_InputMapping[INPUT_JUMP].SetJoyButton(JOY_4);
				m_InputMapping[INPUT_NEXT].SetJoyButton(JOY_6);
				m_InputMapping[INPUT_PREV].SetJoyButton(JOY_5);
				m_InputMapping[INPUT_START].SetJoyButton(JOY_8);
				m_InputMapping[INPUT_BACK].SetJoyButton(JOY_7);
				// Set up the default joystick direction bindings
				m_InputMapping[INPUT_L_UP].SetDirection(0, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_L_DOWN].SetDirection(0, 1, JOYDIR_TWO);
				m_InputMapping[INPUT_L_LEFT].SetDirection(0, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_L_RIGHT].SetDirection(0, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_R_UP].SetDirection(2, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_R_DOWN].SetDirection(2, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_R_LEFT].SetDirection(1, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_R_RIGHT].SetDirection(1, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_FIRE].SetDirection(0, 2, JOYDIR_ONE);
				m_InputMapping[INPUT_JUMP].SetDirection(0, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_CROUCH].SetDirection(0, 1, JOYDIR_TWO);
				m_InputMapping[INPUT_PIEMENU].SetDirection(0, 2, JOYDIR_TWO);
				m_InputMapping[INPUT_WEAPON_CHANGE_PREV].SetDirection(3, 0, JOYDIR_ONE);
				m_InputMapping[INPUT_WEAPON_CHANGE_NEXT].SetDirection(3, 0, JOYDIR_TWO);
				m_InputMapping[INPUT_WEAPON_RELOAD].SetDirection(3, 1, JOYDIR_ONE);
				m_InputMapping[INPUT_WEAPON_PICKUP].SetDirection(3, 1, JOYDIR_TWO);
		}
	}
}