#ifndef _RTECONTROLLER_
#define _RTECONTROLLER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Controller.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Controller class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Timer.h"
#include "RTETools.h"
#include "Vector.h"

namespace RTE
{

class Actor;


// Enumerate control states.
enum ControlState
{
    PRIMARY_ACTION = 0,
    SECONDARY_ACTION,
    MOVE_IDLE,
    MOVE_RIGHT,
    MOVE_LEFT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_FAST,
    BODY_JUMPSTART,
    BODY_JUMP,
    BODY_CROUCH,
    AIM_UP,
    AIM_DOWN,
    AIM_SHARP,
    WEAPON_FIRE,
    WEAPON_RELOAD,
    PIE_MENU_ACTIVE,
    WEAPON_CHANGE_NEXT,
    WEAPON_CHANGE_PREV,
    WEAPON_PICKUP,
    WEAPON_DROP,
    ACTOR_NEXT,
    ACTOR_PREV,
    ACTOR_BRAIN,
    ACTOR_NEXT_PREP,
    ACTOR_PREV_PREP,
    HOLD_RIGHT,
    HOLD_LEFT,
    HOLD_UP,
    HOLD_DOWN,
    // These will only register once for keypresses
    PRESS_PRIMARY,
    PRESS_SECONDARY,
    PRESS_RIGHT,
    PRESS_LEFT,
    PRESS_UP,
    PRESS_DOWN,
    // When the buttons are released
    RELEASE_PRIMARY,
    RELEASE_SECONDARY,
    // Any of the four action buttons, fire, aim, inventory and jump (not next and prev actor!)
    PRESS_FACEBUTTON,
    SCROLL_UP,
    SCROLL_DOWN,
    DEBUG_ONE,
    CONTROLSTATECOUNT
};


//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A class controlling MovableObject:s through either player input,
//                  networking, scripting, AI, etc.
// Parent(s):       Entity.
// Class history:   04/09/2001 Controller created.
//                  09/14/2007 All derived classes consolidated to Controller, and
//                             Controller not made an Object.

class Controller
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

// The different input modes
enum InputMode
{
    CIM_DISABLED = 0,
    CIM_PLAYER,
    CIM_AI,
    CIM_NETWORK,
    CIM_INPUTMODECOUNT
};


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Controller object in system
//                  memory. Create() should be called before using the object.
// Arguments:       The Actor this is supposed to control. Ownership is NOT transferred!

    Controller() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Controller object in system
//                  memory. Create() should be called before using the object.
// Arguments:       The Actor this is supposed to control. Ownership is NOT transferred!

    Controller(InputMode mode, Actor *pControlled) { Clear(); Create(mode, pControlled); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Controller object in system
//                  memory. Create() should be called before using the object.
// Arguments:       Which human player is controlling this.

    Controller(InputMode mode, int player = 0) { Clear(); Create(mode, player); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Controller object
//                  identical to an already existing one.
// Arguments:       A Controller object which is passed in by reference.

    Controller(const Controller &reference) { if (this != &reference) { Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Controller
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Controller object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Controller() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Controller object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Controller object ready for use.
// Arguments:       The Actor this is supposed to control. Ownership is NOT transferred!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(InputMode mode, Actor *pControlled);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Controller object ready for use.
// Arguments:       Which player is controlling this.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(InputMode mode, int player) { m_InputMode = mode; m_Player = player; return Create(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Controller to be identical to another, by deep copy.
// Arguments:       A reference to the Controller to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Controller &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Controller assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Controller equal to another.
// Arguments:       A Controller reference.
// Return value:    A reference to the changed Controller.

    virtual Controller & operator=(const Controller &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Controller, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Controller object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetInputMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the mode of input for this Controller.
// Arguments:       The new InputMode for this controller to use.
// Return value:    None.

    void SetInputMode(InputMode newMode) { if (m_InputMode != newMode) { m_ReleaseTimer.Reset(); } m_InputMode = newMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetInputMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mode of input for this Controller.
// Arguments:       None.
// Return value:    The InputMode that this controller is currently using.

    InputMode GetInputMode() const { return m_InputMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlayerControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shortcut to indicate if in player input mode.
// Arguments:       If you want to check if it's controlled by a player, AND that player
//                  is someone else than a specific one, pass in that player number here.
// Return value:    Whether input mode is set to player input.

    bool IsPlayerControlled(int otherThanPlayer = -1) { return (m_InputMode == CIM_PLAYER && (otherThanPlayer < 0 || m_Player != otherThanPlayer)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetControlledActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which Actor is being controlled by this. 0 if none.
// Arguments:       A pointer to the Actor which is being controlled by this. Ownership
//                  is NOT transferred!
// Return value:    None.

    virtual Actor * GetControlledActor() const { return m_pControlled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Team number using this controller.
// Arguments:       None.
// Return value:    An int representing the team which this Controller belongs to. 0 is the
//                  first team. 0 if no team is using it.

    int GetTeam() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAnalogMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the analog movement vector state of this.
// Arguments:       The new analog movement vector.
// Return value:    None.

    void SetAnalogMove(Vector &newMove) { m_AnalogMove = newMove; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAnalogMove
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog movement input data.
// Arguments:       None.
// Return value:    A vector with the analog movement data, both axes ranging form -1.0 to 1.0.

    Vector GetAnalogMove() const { return m_AnalogMove; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAnalogAim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the analog aiming vector state of this.
// Arguments:       The new analog aiming vector.
// Return value:    None.

    void SetAnalogAim(Vector &newAim) { m_AnalogAim = newAim; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAnalogAim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog aiming input data.
// Arguments:       None.
// Return value:    A vector with the analog aiming data, both axes ranging form -1.0 to 1.0.

    Vector GetAnalogAim() const { return m_AnalogAim; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAnalogCursor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog menu input data.
// Arguments:       None.
// Return value:    A vector with the analog menu data, both axes ranging form -1.0 to 1.0.

    Vector GetAnalogCursor() const { return m_AnalogCursor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RelativeCursorMovement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds relative movement to a passed-in vector. Uses the appropriate
//                  input method currently of this.
// Arguments:       The vector to alter.
//                  The scale of the input. 1.0 is 'normal'.
// Return value:    Whether the vector was altered or not.

    bool RelativeCursorMovement(Vector &cursorPos, float moveScale = 1.0f);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetControlledActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which Actor is supposed to be controlled by this.
// Arguments:       A pointer to a an Actor which is being controlled by this. Ownership
//                  is NOT transferred!
// Return value:    None.

    virtual void SetControlledActor(Actor *pControlled = 0) { m_pControlled = pControlled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the team which is controlling this Controller's controlled Actor.
// Arguments:       The team number. 0 is the first team.
// Return value:    None.

    void SetTeam(int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which player's input this is listening to, and will enable player
//                  input mode.
// Arguments:       The player number.
// Return value:    None.

    void SetPlayer(int player) { m_Player = player; if (m_Player >= 0) { m_InputMode = CIM_PLAYER; } }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which player's input this is listening to, if in player input mode.
// Arguments:       None.
// Return value:    The player number, or -1 if not in player input mode.

    int GetPlayer() const { return m_InputMode == CIM_PLAYER ? m_Player : -1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMouseControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this is listening to mouse input at all.
// Arguments:       None.
// Return value:    Whether this is using mouse input at all.

	bool IsMouseControlled() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseMovement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the relative movement of the mouse since last update. Only returns
//                  true if this' player is actually set up to be using the mouse.
// Arguments:       None.
// Return value:    The relative mouse movements, in both axes.

    const Vector & GetMouseMovement() const { return m_MouseMovement; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDisabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this is disabled controller that doesn't give any new 
//                  output.
// Arguments:       Disabled or not.
// Return value:    None.

    void SetDisabled(bool disabled = true) { if (m_Disabled != disabled) { m_ReleaseTimer.Reset(); } m_Disabled = disabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDisabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is disabled.
// Arguments:       None.
// Return value:    Whether disabled.

    bool IsDisabled() const { return m_InputMode == CIM_DISABLED || m_Disabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets one of this controller's states.
// Arguments:       Which state to set and which value to set it to.
// Return value:    None.

    void SetState(ControlState which, bool setting = true) { RTEAssert(which >= 0 && which < CONTROLSTATECOUNT, "Control state out of whack"); m_ControlStates[which] = setting; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the current controller is in a specific state.
// Arguments:       Which state to check.
// Return value:    Whether the controller is in the specified state.

    bool IsState(ControlState which) const { return m_ControlStates[which]; };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Controller. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable and method declarations

public:


    // Analog values for movement
    Vector m_AnalogMove;
    // Analog values for aiming
    Vector m_AnalogAim;
    // Analog values for Pie Menu operation
    Vector m_AnalogCursor;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Control states
    bool m_ControlStates[CONTROLSTATECOUNT];
    // The current controller input mode, like AI, player etc
    InputMode m_InputMode;
    // The actor controled by this
    Actor *m_pControlled;
    // The last team this controlled. This is necesary so we stil have some control after controlled's death
    int m_Team;
    // The last player this controlled. This is necesary so we stil have some control after controlled's death
    // If this is -1, no player is controlling/ed, even if in player control input mode
    int m_Player;
    // Quick and easy disable to prevent updates from being made
    bool m_Disabled;
    // These are hacks to make the switch to brain shortcut work without immediately switching away by detecting the release of the prev and next buttons after pressing them both down to get to the brain
    bool m_NextIgnore;
    bool m_PrevIgnore;
	// These are used to track just a single press on  shortcut button
	bool m_WeaponChangeNextIgnore;
	bool m_WeaponChangePrevIgnore;
	bool m_WeaponPickupIgnore;
	bool m_WeaponDropIgnore;
	bool m_WeaponReloadIgnore;
    // Relative mouse movement, if this player uses the mouse
    Vector m_MouseMovement;
    // Timer for measuring release delays
    Timer m_ReleaseTimer;
    // Timer for measuring analog joystick-controlled cursor acceleration
    Timer m_JoyAccelTimer;
    // Timer for measuring keyboard-controlled cursor acceleration
    Timer m_KeyAccelTimer;
    // The delay between releasing a menu button and activating the regular controls, to avoid accidental input
    static const int m_ReleaseDelay;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Controller, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File