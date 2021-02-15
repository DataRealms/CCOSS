#include "Controller.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "Actor.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::Clear() {
		m_ControlStates.fill(false);
		m_AnalogMove.Reset();
		m_AnalogAim.Reset();
		m_AnalogCursor.Reset();

		m_InputMode = CIM_PLAYER;
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
		for (int i = 0; i < CONTROLSTATECOUNT; ++i) {
			m_ControlStates.at(i) = reference.m_ControlStates.at(i);
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

			if (IsState(HOLD_LEFT)) {
				cursorPos.m_X -= 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(HOLD_RIGHT)) {
				cursorPos.m_X += 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(HOLD_UP)) {
				cursorPos.m_Y -= 10 * moveScale * acceleration;
				altered = true;
			}
			if (IsState(HOLD_DOWN)) {
				cursorPos.m_Y += 10 * moveScale * acceleration;
				altered = true;
			}
		}
		return altered;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Controller::IsMouseControlled() const {
		return g_UInputMan.IsInMultiplayerMode() ? DEVICE_MOUSE_KEYB : g_UInputMan.GetControlScheme(m_Player)->GetDevice() == DEVICE_MOUSE_KEYB;
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
		// Reset all command states.
		m_ControlStates.fill(false);
		m_AnalogMove.Reset();
		m_AnalogAim.Reset();
		m_AnalogCursor.Reset();
		m_MouseMovement.Reset();

		// Update team indicator
		if (m_ControlledActor) { m_Team = m_ControlledActor->GetTeam(); }

		// Player Input Mode
		if (m_InputMode == CIM_PLAYER) {
			// Disable player input if the console is open but isn't in read-only mode, or the controller is disabled or has no player
			if ((g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) || m_Disabled || m_Player < 0) {
				return;
			}
			// Update all the player input
			UpdatePlayerInput();

		// AI Input Mode
		} else if (m_InputMode == CIM_AI) {
			// Disabled won't get updates, or when the activity isn't going
			if (m_Disabled || !g_ActivityMan.ActivityRunning()) {
				return;
			}

			// Update the AI state of the Actor we're controlling and to use any scripted AI defined for this Actor.
			if (m_ControlledActor && !m_ControlledActor->UpdateAIScripted()) {
				// If we can't, fall back on the legacy C++ implementation
				m_ControlledActor->UpdateAI();
			}
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
		if ((g_UInputMan.ElementHeld(m_Player, INPUT_NEXT) && g_UInputMan.ElementPressed(m_Player, INPUT_PREV)) ||
			(g_UInputMan.ElementPressed(m_Player, INPUT_NEXT) && g_UInputMan.ElementHeld(m_Player, INPUT_PREV))) {
			m_ControlStates.at(ACTOR_BRAIN) = true;
			// Ignore the next releases of next and previous buttons so that the brain isn't switched away form immediate after using the brain shortcut
			m_NextIgnore = m_PrevIgnore = true;
		// NEXT ACTOR
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_NEXT)) {
			if (!m_NextIgnore) { m_ControlStates.at(ACTOR_NEXT) = true; }
			m_NextIgnore = false;
		// PREV ACTOR
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_PREV)) {
			if (!m_PrevIgnore) { m_ControlStates.at(ACTOR_PREV) = true; }
			m_PrevIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_WEAPON_CHANGE_NEXT)) {
			m_WeaponChangeNextIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_WEAPON_CHANGE_PREV)) {
			m_WeaponChangePrevIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_WEAPON_PICKUP)) {
			m_WeaponPickupIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_WEAPON_DROP)) {
			m_WeaponDropIgnore = false;
		} else if (g_UInputMan.ElementReleased(m_Player, INPUT_WEAPON_RELOAD)) {
			m_WeaponReloadIgnore = false;
		}

		m_ControlStates.at(HOLD_RIGHT) = g_UInputMan.ElementHeld(m_Player, INPUT_L_RIGHT) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_RIGHT);
		m_ControlStates.at(HOLD_LEFT) = g_UInputMan.ElementHeld(m_Player, INPUT_L_LEFT) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_LEFT);
		m_ControlStates.at(HOLD_UP) = g_UInputMan.ElementHeld(m_Player, INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_UP);
		m_ControlStates.at(HOLD_DOWN) = g_UInputMan.ElementHeld(m_Player, INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_DOWN);
		m_ControlStates.at(PRESS_RIGHT) = g_UInputMan.ElementPressed(m_Player, INPUT_L_RIGHT) || g_UInputMan.ElementPressed(m_Player, INPUT_AIM_RIGHT);
		m_ControlStates.at(PRESS_LEFT) = g_UInputMan.ElementPressed(m_Player, INPUT_L_LEFT) || g_UInputMan.ElementPressed(m_Player, INPUT_AIM_LEFT);
		m_ControlStates.at(PRESS_UP) = g_UInputMan.ElementPressed(m_Player, INPUT_L_UP) || g_UInputMan.ElementPressed(m_Player, INPUT_AIM_UP);
		m_ControlStates.at(PRESS_DOWN) = g_UInputMan.ElementPressed(m_Player, INPUT_L_DOWN) || g_UInputMan.ElementPressed(m_Player, INPUT_AIM_DOWN);

		m_ControlStates.at(PRIMARY_ACTION) = g_UInputMan.ElementHeld(m_Player, INPUT_FIRE);
		m_ControlStates.at(SECONDARY_ACTION) = g_UInputMan.ElementHeld(m_Player, INPUT_PIEMENU);
		m_ControlStates.at(PRESS_PRIMARY) = g_UInputMan.ElementPressed(m_Player, INPUT_FIRE);
		m_ControlStates.at(PRESS_SECONDARY) = g_UInputMan.ElementPressed(m_Player, INPUT_PIEMENU);
		m_ControlStates.at(RELEASE_PRIMARY) = g_UInputMan.ElementReleased(m_Player, INPUT_FIRE);
		m_ControlStates.at(RELEASE_SECONDARY) = g_UInputMan.ElementReleased(m_Player, INPUT_PIEMENU);

		UpdatePlayerAnalogInput();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::UpdatePlayerPieMenuInput() {
		// PIE MENU ACTIVE
		if (g_UInputMan.ElementHeld(m_Player, INPUT_PIEMENU)) {
			m_ControlStates.at(PIE_MENU_ACTIVE) = true;
			m_ControlStates.at(MOVE_IDLE) = true;
			m_ReleaseTimer.Reset();
		} else {
			// Holding of the switch buttons disables aiming later
			if (g_UInputMan.ElementHeld(m_Player, INPUT_NEXT)) {
				m_ControlStates.at(ACTOR_NEXT_PREP) = true;
				m_ReleaseTimer.Reset();
			} else if (g_UInputMan.ElementHeld(m_Player, INPUT_PREV)) {
				m_ControlStates.at(ACTOR_PREV_PREP) = true;
				m_ReleaseTimer.Reset();
			// No actions can be performed while switching actors or pie menu, and short time thereafter
			} else if (m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay)) {
				m_ControlStates.at(WEAPON_FIRE) = g_UInputMan.ElementHeld(m_Player, INPUT_FIRE);
				m_ControlStates.at(AIM_SHARP) = g_UInputMan.ElementHeld(m_Player, INPUT_AIM);
				m_ControlStates.at(BODY_JUMPSTART) = g_UInputMan.ElementPressed(m_Player, INPUT_JUMP);
				m_ControlStates.at(BODY_JUMP) = g_UInputMan.ElementHeld(m_Player, INPUT_JUMP);
				m_ControlStates.at(BODY_CROUCH) = g_UInputMan.ElementHeld(m_Player, INPUT_CROUCH);

				// MOVEMENT LEFT/RIGHT
				if (g_UInputMan.ElementHeld(m_Player, INPUT_L_RIGHT)) {
					m_ControlStates.at(MOVE_RIGHT) = true;
				} else if (g_UInputMan.ElementHeld(m_Player, INPUT_L_LEFT)) {
					m_ControlStates.at(MOVE_LEFT) = true;
				} else {
					m_ControlStates.at(MOVE_IDLE) = true;
				}

				// AIM LEFT AND RIGHT DIGITALLY - not really used as aiming, so convert into movement input
				if (g_UInputMan.ElementHeld(m_Player, INPUT_AIM_LEFT)) {
					m_ControlStates.at(MOVE_LEFT) = true;
				} else if (g_UInputMan.ElementHeld(m_Player, INPUT_AIM_RIGHT)) {
					m_ControlStates.at(MOVE_RIGHT) = true;
				}

				// AIM AND MOVE UP AND DOWN
				if (g_UInputMan.ElementHeld(m_Player, INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_UP)) {
					m_ControlStates.at(MOVE_UP) = true;
				} else if (g_UInputMan.ElementHeld(m_Player, INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, INPUT_AIM_DOWN)) {
					m_ControlStates.at(MOVE_DOWN) = true;
				}

				// AIM UP AND DOWN DIGITALLY
				if (g_UInputMan.ElementHeld(m_Player, INPUT_AIM_UP)) {
					m_ControlStates.at(AIM_UP) = true;
				} else if (g_UInputMan.ElementHeld(m_Player, INPUT_AIM_DOWN)) {
					m_ControlStates.at(AIM_DOWN) = true;
				}

				m_ControlStates.at(PRESS_FACEBUTTON) = g_UInputMan.ElementPressed(m_Player, INPUT_FIRE) || g_UInputMan.ElementPressed(m_Player, INPUT_AIM);

				if (!m_WeaponChangeNextIgnore && g_UInputMan.ElementPressed(m_Player, INPUT_WEAPON_CHANGE_NEXT)) {
					m_ControlStates.at(WEAPON_CHANGE_NEXT) = true;
					m_WeaponChangeNextIgnore = true;
				}
				if (!m_WeaponChangePrevIgnore && g_UInputMan.ElementPressed(m_Player, INPUT_WEAPON_CHANGE_PREV)) {
					m_ControlStates.at(WEAPON_CHANGE_PREV) = true;
					m_WeaponChangePrevIgnore = true;
				}
				if (!m_WeaponPickupIgnore && g_UInputMan.ElementPressed(m_Player, INPUT_WEAPON_PICKUP)) {
					m_ControlStates.at(WEAPON_PICKUP) = true;
					m_WeaponPickupIgnore = true;
				}
				if (!m_WeaponDropIgnore && g_UInputMan.ElementPressed(m_Player, INPUT_WEAPON_DROP)) {
					m_ControlStates.at(WEAPON_DROP) = true;
					m_WeaponDropIgnore = true;
				}
				if (!m_WeaponReloadIgnore && g_UInputMan.ElementPressed(m_Player, INPUT_WEAPON_RELOAD)) {
					m_ControlStates.at(WEAPON_RELOAD) = true;
					m_WeaponReloadIgnore = true;
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Controller::UpdatePlayerAnalogInput() {
		// ANALOG joystick values
		Vector move = g_UInputMan.AnalogMoveValues(m_Player);
		Vector aim = g_UInputMan.AnalogAimValues(m_Player);

		// Only change aim and move if not holding actor switch buttons - don't want to mess up AI's aim
		if (!m_ControlStates.at(PIE_MENU_ACTIVE) && !m_ControlStates.at(ACTOR_PREV_PREP) && !m_ControlStates.at(ACTOR_NEXT_PREP) && m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay)) {
			m_AnalogMove = move;
			m_AnalogAim = aim;
		} else {
			m_AnalogCursor = move.GetLargest() > aim.GetLargest() ? move : aim;
		}

		// If the joystick-controlled analog cursor is less than at the edge of input range, don't accelerate
		if (GetAnalogCursor().GetMagnitude() < 0.85F) { m_JoyAccelTimer.Reset(); }
		// If the keyboard inputs for cursor movements is initially pressed, reset the acceleration timer
		if (IsState(ACTOR_NEXT) || IsState(ACTOR_PREV) || (IsState(PRESS_LEFT) || IsState(PRESS_RIGHT) || IsState(PRESS_UP) || IsState(PRESS_DOWN))) {
			m_KeyAccelTimer.Reset();
		}

		// Translate analog aim input into sharp aim control state
		if (m_AnalogAim.GetMagnitude() > 0.1F && !m_ControlStates.at(PIE_MENU_ACTIVE)) { m_ControlStates.at(AIM_SHARP) = true; }

		// Disable sharp aim while moving - this also helps with keyboard vs mouse fighting when moving and aiming in opposite directions
		if (m_ControlStates.at(PRESS_RIGHT) || m_ControlStates.at(PRESS_LEFT) || m_ControlStates.at(BODY_JUMPSTART) || (m_ControlStates.at(PIE_MENU_ACTIVE) && !m_ControlStates.at(SECONDARY_ACTION))) {
			if (IsMouseControlled()) { g_UInputMan.SetMouseValueMagnitude(0.05F); }
			m_ControlStates.at(AIM_SHARP) = false;
		}

		// Special handing of the mouse input, if applicable
		if (IsMouseControlled()) {
			m_MouseMovement = g_UInputMan.GetMouseMovement(m_Player);

			if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) < 0) {
				m_ControlStates.at(WEAPON_CHANGE_NEXT) = m_ControlStates.at(SCROLL_DOWN) = true;
			} else if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) > 0) {
				m_ControlStates.at(WEAPON_CHANGE_PREV) = m_ControlStates.at(SCROLL_UP) = true;
			}
			MouseButtons activeSecondary = MOUSE_RIGHT;

			m_ControlStates.at(PRIMARY_ACTION) = g_UInputMan.MouseButtonHeld(MOUSE_LEFT, m_Player);
			m_ControlStates.at(SECONDARY_ACTION) = g_UInputMan.MouseButtonHeld(activeSecondary, m_Player);
			m_ControlStates.at(PRESS_PRIMARY) = g_UInputMan.MouseButtonPressed(MOUSE_LEFT, m_Player);
			m_ControlStates.at(PRESS_SECONDARY) = g_UInputMan.MouseButtonPressed(activeSecondary, m_Player);
			m_ControlStates.at(RELEASE_PRIMARY) = g_UInputMan.MouseButtonReleased(MOUSE_LEFT, m_Player);
			m_ControlStates.at(RELEASE_SECONDARY) = g_UInputMan.MouseButtonReleased(activeSecondary, m_Player);
		}
	}
}