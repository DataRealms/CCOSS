#include "Controller.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "MovableMan.h"
#include "Actor.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::Clear() {
		m_ControlStates.fill(false);
		m_AnalogMove.Reset();
		m_AnalogAim.Reset();
		m_AnalogCursor.Reset();

		m_InputMode = InputMode::CIM_PLAYER;
		m_ControlledActor = nullptr;
		m_Team = 0;
		m_Player = 0;
		m_Disabled = false;
		m_NextIgnore = false;
		m_PrevIgnore = false;
		m_WeaponChangeNextIgnore = false;
		m_WeaponChangePrevIgnore = false;
		m_WeaponPickupIgnore = false;
		m_WeaponDropIgnore = false;
		m_WeaponReloadIgnore = false;
		m_MouseMovement.Reset();
		m_AnalogCursorAngleLimits = { {0, 0}, false };
		m_ReleaseTimer.Reset();
		m_JoyAccelTimer.Reset();
		m_KeyAccelTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Controller::Create(InputMode mode, Actor *controlledActor) {
		m_InputMode = mode;
		m_ControlledActor = controlledActor;
		if (m_ControlledActor) { m_Team = m_ControlledActor->GetTeam(); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Controller::Create(const Controller &reference) {
		for (int i = 0; i < ControlState::CONTROLSTATECOUNT; ++i) {
			m_ControlStates[i] = reference.m_ControlStates[i];
		}
		m_AnalogMove = reference.m_AnalogMove;
		m_AnalogAim = reference.m_AnalogAim;
		m_AnalogCursor = reference.m_AnalogCursor;

		m_InputMode = reference.m_InputMode;
		m_Team = reference.m_Team;
		m_Player = reference.m_Player;
		m_Disabled = reference.m_Disabled;

		m_WeaponChangeNextIgnore = reference.m_WeaponChangeNextIgnore;
		m_WeaponChangePrevIgnore = reference.m_WeaponChangePrevIgnore;
		m_WeaponPickupIgnore = reference.m_WeaponPickupIgnore;
		m_WeaponDropIgnore = reference.m_WeaponDropIgnore;
		m_WeaponReloadIgnore = reference.m_WeaponReloadIgnore;

		m_AnalogCursorAngleLimits = reference.m_AnalogCursorAngleLimits;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Controller::RelativeCursorMovement(Vector &cursorPos, float moveScale) const {
		bool altered = false;

		// Try the mouse first for analog input
		if (!GetMouseMovement().IsZero()) {
			cursorPos += GetMouseMovement() * moveScale;
			altered = true;

		// See if there's other analog input, only if the mouse isn't active (or the cursor will float if mouse is used!)
		} else if (GetAnalogCursor().GetLargest() > 0.1F && !IsMouseControlled()) {
			// See how much to accelerate the joystick input based on how long the stick has been pushed around
			float acceleration = static_cast<float>(0.5 + std::min(m_JoyAccelTimer.GetElapsedRealTimeS(), 0.5) * 6);
			cursorPos += GetAnalogCursor() * 10 * moveScale * acceleration;
			altered = true;

		// Digital movement
		} else {
			// See how much to accelerate the keyboard input based on how long any key has been pressed
			float acceleration = static_cast<float>(0.25 + std::min(m_KeyAccelTimer.GetElapsedRealTimeS(), 0.75) * 6);

			if (IsState(ControlState::HOLD_LEFT)) {
				cursorPos.m_X -= 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(ControlState::HOLD_RIGHT)) {
				cursorPos.m_X += 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(ControlState::HOLD_UP)) {
				cursorPos.m_Y -= 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(ControlState::HOLD_DOWN)) {
				cursorPos.m_Y += 10 * moveScale * acceleration;
				altered = true;
			}
		}
		return altered;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Controller::GetDigitalAimSpeed() const {
		return m_Player != Players::NoPlayer ? g_UInputMan.GetControlScheme(m_Player)->GetDigitalAimSpeed() : 1.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Controller::IsMouseControlled() const {
		return m_Player != Players::NoPlayer && g_UInputMan.GetControlScheme(m_Player)->GetDevice() == InputDevice::DEVICE_MOUSE_KEYB;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Controller::IsGamepadControlled() const {
		bool isGamepadControlled = false;
		if (m_Player != Players::NoPlayer) {
			InputDevice inputDevice = g_UInputMan.GetControlScheme(m_Player)->GetDevice();
			if (inputDevice >= InputDevice::DEVICE_GAMEPAD_1 && inputDevice <= InputDevice::DEVICE_GAMEPAD_4) {
				isGamepadControlled = true;
			}
		}
		return isGamepadControlled;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Controller::GetTeam() const {
		return m_ControlledActor ? m_ControlledActor->GetTeam() : m_Team;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::SetTeam(short team) {
		if (m_ControlledActor) { m_ControlledActor->SetTeam(team); }
		m_Team = team;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::Update() {
		if (IsDisabled()) {
			return;
		}

		if (m_ControlledActor) { m_Team = m_ControlledActor->GetTeam(); }

		switch (m_InputMode) {
			case InputMode::CIM_PLAYER:
				GetInputFromPlayer();
				break;
			case InputMode::CIM_AI:
				GetInputFromAI();
				break;
			default:
				ResetCommandState();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::ResetCommandState() {
		// Reset all command states.
		m_ControlStates.fill(false);
		m_AnalogMove.Reset();
		m_AnalogAim.Reset();
		m_AnalogCursor.Reset();
		m_MouseMovement.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::GetInputFromPlayer() {
		ResetCommandState();

		if ((g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) || m_Player < 0) {
			return;
		}

		UpdatePlayerInput();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::GetInputFromAI() {
		// Throttle the AI to only update every X sim updates.
		// We want to spread the updates around (so, half the actors on odd frames, the other half on even frames, etc), so we check their contiguous ID against the frame number.
		const int simTicksPerUpdate = g_SettingsMan.GetAIUpdateInterval();
		if (m_ControlledActor && g_MovableMan.GetContiguousActorID(m_ControlledActor) % simTicksPerUpdate != g_TimerMan.GetSimUpdateCount() % simTicksPerUpdate) {
			// Don't reset our command state, so we give the same input as last frame.
			return;
		}

		ResetCommandState();

		// Try to run the scripted AI for the controlled Actor. If it doesn't work, fall back on the legacy C++ implementation.
		if (m_ControlledActor && m_ControlledActor->ObjectScriptsInitialized() && !m_ControlledActor->UpdateAIScripted()) {
			m_ControlledActor->UpdateAI();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Controller & Controller::operator=(const Controller &rhs) {
		if (this == &rhs) {
			return *this;
		}
		Reset();
		Create(rhs);
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::UpdatePlayerInput() {
		UpdatePlayerPieMenuInput();

		// Only actually switch when the change button(s) are released
		// BRAIN ACTOR
		if ((g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_NEXT) && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_PREV)) ||
			(g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_NEXT) && g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_PREV))) {
			m_ControlStates[ControlState::ACTOR_BRAIN] = true;
			// Ignore the next releases of next and previous buttons so that the brain isn't switched away form immediate after using the brain shortcut
			m_NextIgnore = m_PrevIgnore = true;
		// NEXT ACTOR
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_NEXT)) {
			if (!m_NextIgnore) { m_ControlStates[ControlState::ACTOR_NEXT] = true; }
			m_NextIgnore = false;
		// PREV ACTOR
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_PREV)) {
			if (!m_PrevIgnore) { m_ControlStates[ControlState::ACTOR_PREV] = true; }
			m_PrevIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_WEAPON_CHANGE_NEXT)) {
			m_WeaponChangeNextIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_WEAPON_CHANGE_PREV)) {
			m_WeaponChangePrevIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_WEAPON_PICKUP)) {
			m_WeaponPickupIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_WEAPON_DROP)) {
			m_WeaponDropIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_WEAPON_RELOAD)) {
			m_WeaponReloadIgnore = false;
		}

		m_ControlStates[ControlState::HOLD_RIGHT] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_RIGHT) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_RIGHT);
		m_ControlStates[ControlState::HOLD_LEFT] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_LEFT) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_LEFT);
		m_ControlStates[ControlState::HOLD_UP] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_UP);
		m_ControlStates[ControlState::HOLD_DOWN] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_DOWN);
		m_ControlStates[ControlState::PRESS_RIGHT] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_L_RIGHT) || g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_AIM_RIGHT);
		m_ControlStates[ControlState::PRESS_LEFT] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_L_LEFT) || g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_AIM_LEFT);
		m_ControlStates[ControlState::PRESS_UP] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_L_UP) || g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_AIM_UP);
		m_ControlStates[ControlState::PRESS_DOWN] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_L_DOWN) || g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_AIM_DOWN);

		m_ControlStates[ControlState::PRIMARY_ACTION] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_FIRE);
		m_ControlStates[ControlState::SECONDARY_ACTION] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_PIEMENU);
		m_ControlStates[ControlState::PRESS_PRIMARY] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_FIRE);
		m_ControlStates[ControlState::PRESS_SECONDARY] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_PIEMENU);
		m_ControlStates[ControlState::RELEASE_PRIMARY] = g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_FIRE);
		m_ControlStates[ControlState::RELEASE_SECONDARY] = g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_PIEMENU);

		UpdatePlayerAnalogInput();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::UpdatePlayerPieMenuInput() {		
		// Holding of the switch buttons disables aiming later
		if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_NEXT)) {
			m_ControlStates[ControlState::ACTOR_NEXT_PREP] = true;
			m_ReleaseTimer.Reset();
		} else if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_PREV)) {
			m_ControlStates[ControlState::ACTOR_PREV_PREP] = true;
			m_ReleaseTimer.Reset();
		// No actions can be performed while switching actors, and short time thereafter
		} else if (m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay)) {
			m_ControlStates[ControlState::WEAPON_FIRE] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_FIRE);
			m_ControlStates[ControlState::AIM_SHARP] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM);
			m_ControlStates[ControlState::BODY_JUMPSTART] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_JUMP);
			m_ControlStates[ControlState::BODY_JUMP] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_JUMP);
			m_ControlStates[ControlState::BODY_CROUCH] = g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_CROUCH);

			// MOVEMENT LEFT/RIGHT
			if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_RIGHT)) {
				m_ControlStates[ControlState::MOVE_RIGHT] = true;
			} else if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_LEFT)) {
				m_ControlStates[ControlState::MOVE_LEFT] = true;
			} else {
				m_ControlStates[ControlState::MOVE_IDLE] = true;
			}

			// AIM LEFT AND RIGHT DIGITALLY - not really used as aiming, so convert into movement input
			if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_LEFT)) {
				m_ControlStates[ControlState::MOVE_LEFT] = true;
			} else if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_RIGHT)) {
				m_ControlStates[ControlState::MOVE_RIGHT] = true;
			}

			// AIM AND MOVE UP AND DOWN
			if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_UP)) {
				m_ControlStates[ControlState::MOVE_UP] = true;
			} else if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_DOWN)) {
				m_ControlStates[ControlState::MOVE_DOWN] = true;
			}

			// AIM UP AND DOWN DIGITALLY
			if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_UP)) {
				m_ControlStates[ControlState::AIM_UP] = true;
			} else if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_AIM_DOWN)) {
				m_ControlStates[ControlState::AIM_DOWN] = true;
			}

			m_ControlStates[ControlState::PRESS_FACEBUTTON] = g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_FIRE) || g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_AIM);
			m_ControlStates[ControlState::RELEASE_FACEBUTTON] = g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_FIRE) || g_UInputMan.ElementReleased(m_Player, InputElements::INPUT_AIM);

			if (!m_WeaponChangeNextIgnore && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_WEAPON_CHANGE_NEXT)) {
				m_ControlStates[ControlState::WEAPON_CHANGE_NEXT] = true;
				m_WeaponChangeNextIgnore = true;
			}
			if (!m_WeaponChangePrevIgnore && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_WEAPON_CHANGE_PREV)) {
				m_ControlStates[ControlState::WEAPON_CHANGE_PREV] = true;
				m_WeaponChangePrevIgnore = true;
			}
			if (!m_WeaponPickupIgnore && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_WEAPON_PICKUP)) {
				m_ControlStates[ControlState::WEAPON_PICKUP] = true;
				m_WeaponPickupIgnore = true;
			}
			if (!m_WeaponDropIgnore && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_WEAPON_DROP)) {
				m_ControlStates[ControlState::WEAPON_DROP] = true;
				m_WeaponDropIgnore = true;
			}
			if (!m_WeaponReloadIgnore && g_UInputMan.ElementPressed(m_Player, InputElements::INPUT_WEAPON_RELOAD)) {
				m_ControlStates[ControlState::WEAPON_RELOAD] = true;
				m_WeaponReloadIgnore = true;
			}
		}

		// PIE MENU ACTIVE
		if (g_UInputMan.ElementHeld(m_Player, InputElements::INPUT_PIEMENU)) {
			m_ControlStates[ControlState::PIE_MENU_ACTIVE] = true;
			// Make sure that firing and aiming are ignored while the pie menu is open, since it consumes those inputs.
			m_ControlStates[ControlState::WEAPON_FIRE] = false;
			m_ControlStates[ControlState::AIM_UP] = false;
			m_ControlStates[ControlState::AIM_DOWN] = false;
		} 
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::UpdatePlayerAnalogInput() {
		// ANALOG joystick values
		Vector move = g_UInputMan.AnalogMoveValues(m_Player);
		Vector aim = g_UInputMan.AnalogAimValues(m_Player);

		bool pieMenuActive = m_ControlStates[ControlState::PIE_MENU_ACTIVE];

		// Only change aim and move if not holding actor switch buttons - don't want to mess up AI's aim
		if (!m_ControlStates[ControlState::ACTOR_PREV_PREP] && !m_ControlStates[ControlState::ACTOR_NEXT_PREP] && m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay)) {
			m_AnalogMove = move;
		} 
		
		if (!pieMenuActive) {
			m_AnalogAim = aim;
		} else {
			m_AnalogCursor = aim;
			if (m_AnalogCursorAngleLimits.second) {
				m_AnalogCursor.SetAbsRadAngle(ClampAngle(m_AnalogCursor.GetAbsRadAngle(), m_AnalogCursorAngleLimits.first.first, m_AnalogCursorAngleLimits.first.second));
			}
		}

		// If the joystick-controlled analog cursor is less than at the edge of input range, don't accelerate
		if (GetAnalogCursor().MagnitudeIsLessThan(0.85F)) { 
			m_JoyAccelTimer.Reset(); 
		}
		
		// If the keyboard inputs for cursor movements is initially pressed, reset the acceleration timer
		if (IsState(ControlState::ACTOR_NEXT) || IsState(ControlState::ACTOR_PREV) || (IsState(ControlState::PRESS_LEFT) || IsState(ControlState::PRESS_RIGHT) || IsState(ControlState::PRESS_UP) || IsState(ControlState::PRESS_DOWN))) {
			m_KeyAccelTimer.Reset();
		}

		// Translate analog aim input into sharp aim control state
		if (m_AnalogAim.MagnitudeIsGreaterThan(0.1F) && !pieMenuActive) { 
			m_ControlStates[ControlState::AIM_SHARP] = true; 
		}

		// Disable sharp aim while moving - this also helps with keyboard vs mouse fighting when moving and aiming in opposite directions
		if (m_ControlStates[ControlState::BODY_JUMP] || (pieMenuActive && !m_ControlStates[ControlState::SECONDARY_ACTION])) {
			if (IsMouseControlled()) {
				g_UInputMan.SetMouseValueMagnitude(0.3F);
			}
			m_ControlStates[ControlState::AIM_SHARP] = false;
		}

		// Special handing of the mouse input, if applicable
		if (IsMouseControlled()) {
			m_MouseMovement = g_UInputMan.GetMouseMovement(m_Player);

			if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) < 0) {
				m_ControlStates[ControlState::WEAPON_CHANGE_NEXT] = m_ControlStates[ControlState::SCROLL_DOWN] = true;
			} else if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) > 0) {
				m_ControlStates[ControlState::WEAPON_CHANGE_PREV] = m_ControlStates[ControlState::SCROLL_UP] = true;
			}
			MouseButtons activeSecondary = MouseButtons::MOUSE_RIGHT;

			m_ControlStates[ControlState::PRIMARY_ACTION] = g_UInputMan.MouseButtonHeld(MouseButtons::MOUSE_LEFT, m_Player);
			m_ControlStates[ControlState::SECONDARY_ACTION] = g_UInputMan.MouseButtonHeld(activeSecondary, m_Player);
			m_ControlStates[ControlState::PRESS_PRIMARY] = g_UInputMan.MouseButtonPressed(MouseButtons::MOUSE_LEFT, m_Player);
			m_ControlStates[ControlState::PRESS_SECONDARY] = g_UInputMan.MouseButtonPressed(activeSecondary, m_Player);
			m_ControlStates[ControlState::RELEASE_PRIMARY] = g_UInputMan.MouseButtonReleased(MouseButtons::MOUSE_LEFT, m_Player);
			m_ControlStates[ControlState::RELEASE_SECONDARY] = g_UInputMan.MouseButtonReleased(activeSecondary, m_Player);
		}
	}
}
