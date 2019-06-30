//////////////////////////////////////////////////////////////////////////////////////////
// File:            Controller.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Controller class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Controller.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "DDTTools.h"
#include "Actor.h"

using namespace std;

namespace RTE
{

const int Controller::m_ReleaseDelay = 250;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Controller, effectively
//                  resetting the members of this abstraction level only.

void Controller::Clear()
{
    for (int i = 0; i < CONTROLSTATECOUNT; ++i)
        m_ControlStates[i] = false;

    m_AnalogMove.Reset();
    m_AnalogAim.Reset();
    m_AnalogCursor.Reset();

    m_InputMode = CIM_PLAYER;
    m_pControlled = 0;
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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Controller object ready for use.

int Controller::Create()
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Controller object ready for use.

int Controller::Create(InputMode mode, Actor *pControlled)
{
    m_InputMode = mode;
    m_pControlled = pControlled;
    if (m_pControlled)
        m_Team = m_pControlled->GetTeam();

    return Create();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Controller to be identical to another, by deep copy.

int Controller::Create(const Controller &reference)
{
    for (int i = 0; i < CONTROLSTATECOUNT; ++i)
        m_ControlStates[i] = reference.m_ControlStates[i];

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


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Controller assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Controller equal to another.

Controller & Controller::operator=(const Controller &rhs)
{
    if (this == &rhs)
        return *this;

    Destroy();
    Create(rhs);

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Controller object.

void Controller::Destroy()
{
    Clear();
}

/*
/////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Controller object
//                  identical to an already existing one.

Controller::Controller(const Controller &reference)
{
    for (int i = 0; i < CONTROLSTATECOUNT; ++i)
        m_ControlStates[i] = reference.IsState((ControlState)i);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Team number using this controller.

int Controller::GetTeam() const
{ 
    if (m_pControlled)
        return m_pControlled->GetTeam();
    else
        return m_Team;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RelativeCursorMovement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds relative movement to a passed-in vector. Uses the appropriate
//                  input method currently of this.

bool Controller::RelativeCursorMovement(Vector &cursorPos, float moveScale)
{
    bool altered = false;

    // Try the mouse first for analog input
    if (!GetMouseMovement().IsZero())
    {
        cursorPos += GetMouseMovement() * moveScale;
        altered = true;
    }
    // See if there's other analog input, only if the mouse isn't active (or the cursor will float if mouse is used!)
    else if (GetAnalogCursor().GetLargest() > 0.1 && !IsMouseControlled())
    {
        // See how much to accelerate the joystick input based on how long the stick has been pushed around
        float acceleration = 0.5 + DMin(m_JoyAccelTimer.GetElapsedRealTimeS(), 0.5) * 6;
        cursorPos += GetAnalogCursor() * 10 * moveScale * acceleration;
        altered = true;
    }
    // Digital movement
    else
    {
        // See how much to accelerate the keyboard input based on how long any key has been pressed
        float acceleration = 0.25 + DMin(m_KeyAccelTimer.GetElapsedRealTimeS(), 0.75) * 6;

        if (IsState(HOLD_LEFT))
        {
            cursorPos.m_X -= 10 * moveScale * acceleration;
            altered = true;
        }
        if (IsState(HOLD_RIGHT))
        {
            cursorPos.m_X += 10 * moveScale * acceleration;
            altered = true;
        }
        if (IsState(HOLD_UP))
        {
            cursorPos.m_Y -= 10 * moveScale * acceleration;
            altered = true;
        }
        if (IsState(HOLD_DOWN))
        {
            cursorPos.m_Y += 10 * moveScale * acceleration;
            altered = true;
        }
    }

    return altered;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the team which is controlling this Controller's controlled Actor.

void Controller::SetTeam(int team)
{
    if (m_pControlled)
        m_pControlled->SetTeam(team);

    m_Team = team;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMouseControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this is listening to mouse input at all.

bool Controller::IsMouseControlled() const
{
	if (g_UInputMan.OverrideInput())
		return UInputMan::DEVICE_MOUSE_KEYB;
	else
		return g_UInputMan.GetControlScheme(m_Player)->GetDevice() == UInputMan::DEVICE_MOUSE_KEYB;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Controller. Supposed to be done every frame.

void Controller::Update()
{
    // Reset all command states.
    for (int i = 0; i < CONTROLSTATECOUNT; ++i)
        m_ControlStates[i] = false;

    m_AnalogMove.Reset();
    m_AnalogAim.Reset();
    m_AnalogCursor.Reset();
    m_MouseMovement.Reset();

    // Update team indicator
    if (m_pControlled)
        m_Team = m_pControlled->GetTeam();

    ////////////////////////////////
    // Player Input Mode

    if (m_InputMode == CIM_PLAYER)
    {
        // If the console is open, then disable regular player input and stop updating here
        if (g_ConsoleMan.IsEnabled())
            return;

        // Disabled won't get updates
        if (m_Disabled || m_Player < 0)
            return;

        ///////////////////////////////////////////////////////////
        // Set the control states according to digital key/d-pad states.

        // PIE MENU ACTIVE
        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_PIEMENU))
        {
            m_ControlStates[PIE_MENU_ACTIVE] = true;
            m_ControlStates[MOVE_IDLE] = true;
            m_ReleaseTimer.Reset();
        }
        else
        {
            // Holding of the switch buttons disables aiming later
            if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_NEXT))
            {
                m_ControlStates[ACTOR_NEXT_PREP] = true;
                m_ReleaseTimer.Reset();
            }
			else if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_PREV))
			{
				m_ControlStates[ACTOR_PREV_PREP] = true;
				m_ReleaseTimer.Reset();
			}
			// No actions can be performed while switching actors or pie menu, and short time thereafter
            else if (m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay))
            {
                // WEAPON ACTIVATION
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_FIRE))
                    m_ControlStates[WEAPON_FIRE] = true;

                // SHARP AIM
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM))
                    m_ControlStates[AIM_SHARP] = true;
    /*
                // RELOADING
                if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_PIEMENU))
                    m_ControlStates[WEAPON_RELOAD] = true;
    */
                // JUMP START
                if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_JUMP))
                    m_ControlStates[BODY_JUMPSTART] = true;

                // JUMP EXECUTION
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_JUMP))
                    m_ControlStates[BODY_JUMP] = true;

                // CROUCH/PRONE
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_CROUCH))
                    m_ControlStates[BODY_CROUCH] = true;

                // MOVEMENT LEFT/RIGHT
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_RIGHT))
                    m_ControlStates[MOVE_RIGHT] = true;
                else if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_LEFT))
                    m_ControlStates[MOVE_LEFT] = true;
                else
                    m_ControlStates[MOVE_IDLE] = true;

                // AIM AND MOVE UP AND DOWN
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_UP))
                    m_ControlStates[MOVE_UP] = true;
                else if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_DOWN))
                    m_ControlStates[MOVE_DOWN] = true;

                // AIM UP AND DOWN DIGITALLY
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_UP))
                    m_ControlStates[AIM_UP] = true;
                else if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_DOWN))
                    m_ControlStates[AIM_DOWN] = true;

                // AIM LEFT AND RIGHT DIGITALLY  not really used as aiming, so convert into movement input
                if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_LEFT))
                    m_ControlStates[MOVE_LEFT] = true;
                else if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_RIGHT))
                    m_ControlStates[MOVE_RIGHT] = true;

                if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_FIRE) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_AIM)/* || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_PIEMENU) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_JUMP)*/)
                    m_ControlStates[PRESS_FACEBUTTON] = true;

				if (!m_WeaponChangeNextIgnore && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_WEAPON_CHANGE_NEXT))
				{
					m_ControlStates[WEAPON_CHANGE_NEXT] = true;
					m_WeaponChangeNextIgnore = true;
				}
				if (!m_WeaponChangePrevIgnore && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_WEAPON_CHANGE_PREV))
				{
					m_ControlStates[WEAPON_CHANGE_PREV] = true;
					m_WeaponChangePrevIgnore = true;
				}
				if (!m_WeaponPickupIgnore && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_WEAPON_PICKUP))
				{
					m_ControlStates[WEAPON_PICKUP] = true;
					m_WeaponPickupIgnore = true;
				}
				if (!m_WeaponDropIgnore && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_WEAPON_DROP))
				{
					m_ControlStates[WEAPON_DROP] = true;
					m_WeaponDropIgnore = true;
				}
				if (!m_WeaponReloadIgnore && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_WEAPON_RELOAD))
				{
					m_ControlStates[WEAPON_RELOAD] = true;
					m_WeaponReloadIgnore = true;
				}
            }
        }

        // Only actually switch when the change button(s) are released
        // BRAIN ACTOR
        if ((g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_NEXT) && g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_PREV)) ||
            (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_NEXT) && g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_PREV)))
        {
            m_ControlStates[ACTOR_BRAIN] = true;
            // Ignore the next releases of next and prev buttons so that the brain isnt' switched away form immedailtey after using the brain shortcut
            m_NextIgnore = m_PrevIgnore = true;
        }
        // NEXT ACTOR
        else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_NEXT))
        {
            if (!m_NextIgnore)
                m_ControlStates[ACTOR_NEXT] = true;
            m_NextIgnore = false;
        }
        // PREV ACTOR
        else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_PREV))
        {
            if (!m_PrevIgnore)
                m_ControlStates[ACTOR_PREV] = true;
            m_PrevIgnore = false;
        }
		else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_WEAPON_CHANGE_NEXT))
			m_WeaponChangeNextIgnore = false;
		else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_WEAPON_CHANGE_PREV))
			m_WeaponChangePrevIgnore = false;
		else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_WEAPON_PICKUP))
			m_WeaponPickupIgnore = false;
		else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_WEAPON_DROP))
			m_WeaponDropIgnore = false;
		else if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_WEAPON_RELOAD))
			m_WeaponReloadIgnore = false;

        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_RIGHT) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_RIGHT))
            m_ControlStates[HOLD_RIGHT] = true;
        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_LEFT) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_LEFT))
            m_ControlStates[HOLD_LEFT] = true;
        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_UP) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_UP))
            m_ControlStates[HOLD_UP] = true;
        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_L_DOWN) || g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_AIM_DOWN))
            m_ControlStates[HOLD_DOWN] = true;

        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_L_RIGHT) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_AIM_RIGHT))
            m_ControlStates[PRESS_RIGHT] = true;
        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_L_LEFT) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_AIM_LEFT))
            m_ControlStates[PRESS_LEFT] = true;
        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_L_UP) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_AIM_UP))
            m_ControlStates[PRESS_UP] = true;
        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_L_DOWN) || g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_AIM_DOWN))
            m_ControlStates[PRESS_DOWN] = true;

        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_FIRE))
            m_ControlStates[PRIMARY_ACTION] = true;
        if (g_UInputMan.ElementHeld(m_Player, UInputMan::INPUT_PIEMENU))
            m_ControlStates[SECONDARY_ACTION] = true;
        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_FIRE))
            m_ControlStates[PRESS_PRIMARY] = true;
        if (g_UInputMan.ElementPressed(m_Player, UInputMan::INPUT_PIEMENU))
            m_ControlStates[PRESS_SECONDARY] = true;
        if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_FIRE))
            m_ControlStates[RELEASE_PRIMARY] = true;
        if (g_UInputMan.ElementReleased(m_Player, UInputMan::INPUT_PIEMENU))
            m_ControlStates[RELEASE_SECONDARY] = true;

        ///////////////////////////////////////////
        // ANALOG joystick values

        Vector move = g_UInputMan.AnalogMoveValues(m_Player);
        Vector aim = g_UInputMan.AnalogAimValues(m_Player);
        // Only change aim and move if not holding actor swtich buttons - don't want to mess up AI's aim
        if (!m_ControlStates[PIE_MENU_ACTIVE] && !m_ControlStates[ACTOR_PREV_PREP] && !m_ControlStates[ACTOR_NEXT_PREP] && m_ReleaseTimer.IsPastRealMS(m_ReleaseDelay))
        {
            m_AnalogMove = move;
            m_AnalogAim = aim;
        }
        else
            m_AnalogCursor = move.GetLargest() > aim.GetLargest() ? move : aim;

        // If the joystick-controlled analog cursor is less than at the edge of input range, don't accelerate
        if (GetAnalogCursor().GetMagnitude() < 0.85)
            m_JoyAccelTimer.Reset();
        // If the keyboard inputs for cursor movements is initially pressed, reset the acceleration timer
        if (IsState(ACTOR_NEXT) || IsState(ACTOR_PREV) || (IsState(PRESS_LEFT) || IsState(PRESS_RIGHT) || IsState(PRESS_UP) || IsState(PRESS_DOWN)))
            m_KeyAccelTimer.Reset();

        /////////////////////////////////////////
        // Translate the analog inputs to the discrete control states

        // Sharp Aim
        if (m_AnalogAim.GetMagnitude() > 0.1 && !m_ControlStates[PIE_MENU_ACTIVE])
            m_ControlStates[AIM_SHARP] = true;

        ////////////////////////////////////////////
        // Overrides

        // Sharp aim can't happen when moving around
        // This also helps with keyboard vs mouse fighting when moving and aiming in opposite directions
        if (/*m_AnalogMove.GetMagnitude() > 0.1 || */m_ControlStates[PRESS_RIGHT] || m_ControlStates[PRESS_LEFT] || m_ControlStates[BODY_JUMPSTART] || (m_ControlStates[PIE_MENU_ACTIVE] && !m_ControlStates[SECONDARY_ACTION]))
        {
            // Also stunt the analog aim so that it isn't stuck out in the extreme
            // Reset the aim if we were aiming sharp and not anymore, makes wiggle easier
    //        if (m_ControlStates[AIM_SHARP])
    //            m_AnalogAim.CapMagnitude(0.1);

            if (IsMouseControlled())
                g_UInputMan.SetMouseValueMagnitude(0.05);

            m_ControlStates[AIM_SHARP] = false;
        }

        // Special handing of the mouse input, if applicable
        if (IsMouseControlled())
        {
            m_MouseMovement = g_UInputMan.GetMouseMovement(m_Player);

            if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) < 0)
                m_ControlStates[WEAPON_CHANGE_NEXT] = m_ControlStates[SCROLL_DOWN] = true;
            else if (g_UInputMan.MouseWheelMovedByPlayer(m_Player) > 0)
                m_ControlStates[WEAPON_CHANGE_PREV] = m_ControlStates[SCROLL_UP] = true;

//#if defined(WIN32)
			UInputMan::MouseButtons activeSecondary = UInputMan::MOUSE_RIGHT;
//#elif defined(__APPLE__)
//			UInputMan::MouseButtons activeSecondary = UInputMan::MOUSE_CTRL;
//#endif // defined(WIN32)
			
            if (g_UInputMan.MouseButtonHeld(UInputMan::MOUSE_LEFT, m_Player))
                m_ControlStates[PRIMARY_ACTION] = true;
            if (g_UInputMan.MouseButtonHeld(activeSecondary, m_Player))
                m_ControlStates[SECONDARY_ACTION] = true;
            if (g_UInputMan.MouseButtonPressed(UInputMan::MOUSE_LEFT, m_Player))
                m_ControlStates[PRESS_PRIMARY] = true;
            if (g_UInputMan.MouseButtonPressed(activeSecondary, m_Player))
                m_ControlStates[PRESS_SECONDARY] = true;
            if (g_UInputMan.MouseButtonReleased(UInputMan::MOUSE_LEFT, m_Player))
                m_ControlStates[RELEASE_PRIMARY] = true;
            if (g_UInputMan.MouseButtonReleased(activeSecondary, m_Player))
                m_ControlStates[RELEASE_SECONDARY] = true;
        }

        ////////////////////////////////////
        // DEBUG STUFF
    }

    ////////////////////////////////
    // AI Input Mode

    else if (m_InputMode == CIM_AI)
    {
        // Disabled won't get updates, or when the activity isn't going
        if (m_Disabled || !g_ActivityMan.ActivityRunning())
            return;

        // Update the AI state of the Actor we're controlling
        if (m_pControlled)
        {
            // Try to use any scripted AI defined for this Actor
            if (!m_pControlled->UpdateAIScripted())
                // if can't, fall back on the legacy C++ implementation
                m_pControlled->UpdateAI();
        }
    }
}

} // namespace RTE