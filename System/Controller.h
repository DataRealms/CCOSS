#ifndef _RTECONTROLLER_
#define _RTECONTROLLER_

#include "Timer.h"
#include "Vector.h"

namespace RTE {

	class Actor;

	//!< Enumerate control states.
	enum ControlState {
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
		// These will only register once for key presses.
		PRESS_PRIMARY,
		PRESS_SECONDARY,
		PRESS_RIGHT,
		PRESS_LEFT,
		PRESS_UP,
		PRESS_DOWN,
		// When the buttons are released.
		RELEASE_PRIMARY,
		RELEASE_SECONDARY,
		// Any of the four action buttons, fire, aim, inventory and jump (not next and previous actor!).
		PRESS_FACEBUTTON,
		SCROLL_UP,
		SCROLL_DOWN,
		DEBUG_ONE,
		CONTROLSTATECOUNT
	};

	/// <summary>
	/// A class controlling MovableObjects through either player input, networking, scripting, AI, etc.
	/// </summary>
	class Controller {

	public:

		Vector m_AnalogMove; //!< Analog values for movement.
		Vector m_AnalogAim; //!< Analog values for aiming.
		Vector m_AnalogCursor; //!< Analog values for Pie Menu operation.

		// Enumerate different input modes.
		enum InputMode {
			CIM_DISABLED = 0,
			CIM_PLAYER,
			CIM_AI,
			CIM_NETWORK,
			CIM_INPUTMODECOUNT
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		/// </summary>
		Controller() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		/// </summary>
		/// <param name="mode">The controller input mode, like AI, player etc.</param>
		/// <param name="pControlled">The Actor this is supposed to control. Ownership is NOT transferred!</param>
		Controller(InputMode mode, Actor *pControlled) { Clear(); Create(mode, pControlled); }

		/// <summary>
		/// Constructor method used to instantiate a Controller object in system memory. Create() should be called before using the object.
		/// </summary>
		/// <param name="mode">The controller input mode, like AI, player etc.</param>
		/// <param name="player">Which human player is controlling this.</param>
		Controller(InputMode mode, int player = 0) { Clear(); Create(mode, player); }

		/// <summary>
		/// Copy constructor method used to instantiate a Controller object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Controller object which is passed in by reference.</param>
		Controller(const Controller &reference) { if (this != &reference) { Create(reference); } }

		/// <summary>
		/// Makes the Controller object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create() { return 0; }

		/// <summary>
		/// Makes the Controller object ready for use.
		/// </summary>
		/// <param name="mode">The controller input mode, like AI, player etc.</param>
		/// <param name="pControlled">The Actor this is supposed to control. Ownership is NOT transferred!</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(InputMode mode, Actor *pControlled);

		/// <summary>
		/// Makes the Controller object ready for use.
		/// </summary>
		/// <param name="mode">The controller input mode, like AI, player etc.</param>
		/// <param name="player">Which player is controlling this.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(InputMode mode, int player) { m_InputMode = mode; m_Player = player; return Create(); }

		/// <summary>
		/// Creates a Controller to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Controller to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Controller &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Controller object before deletion from system memory.
		/// </summary>
		virtual ~Controller() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Controller object.
		/// </summary>
		virtual void Destroy() { Clear(); }

		/// <summary>
		/// Resets the entire Controller, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Shortcut to indicate if in player input mode.
		/// </summary>
		/// <param name="otherThanPlayer">If you want to check if it's controlled by a player, AND that player is someone else than a specific one, pass in that player number here.</param>
		/// <returns>Whether input mode is set to player input.</returns>
		bool IsPlayerControlled(int otherThanPlayer = -1) { return (m_InputMode == CIM_PLAYER && (otherThanPlayer < 0 || m_Player != otherThanPlayer)); }

		/// <summary>
		/// Shows whether this controller is disabled.
		/// </summary>
		/// <returns>Whether disabled.</returns>
		bool IsDisabled() const { return m_InputMode == CIM_DISABLED || m_Disabled; }

		/// <summary>
		/// Sets whether this is a disabled controller that doesn't give any new output.
		/// </summary>
		/// <param name="disabled">Disabled or not.</param>
		void SetDisabled(bool disabled = true) { if (m_Disabled != disabled) { m_ReleaseTimer.Reset(); } m_Disabled = disabled; }

		/// <summary>
		/// Shows whether the current controller is in a specific state.
		/// </summary>
		/// <returns>Whether the controller is in the specified state.</returns>
		bool IsState(ControlState which) const { return m_ControlStates[which]; };

		/// <summary>
		/// Sets one of this controller's states.
		/// </summary>
		/// <param name="which>Which state to set.</param>
		/// <param name="setting">Value of the state being set.</param>
		void SetState(ControlState which, bool setting = true) { RTEAssert(which >= 0 && which < CONTROLSTATECOUNT, "Control state out of whack"); m_ControlStates[which] = setting; };

		/// <summary>
		/// Gets the current mode of input for this Controller.
		/// </summary>
		/// <returns>The InputMode that this controller is currently using.</returns>
		InputMode GetInputMode() const { return m_InputMode; }

		/// <summary>
		/// Sets the mode of input for this Controller.
		/// </summary>
		/// <param name="newMode">The new InputMode for this controller to use.</param>
		void SetInputMode(InputMode newMode) { if (m_InputMode != newMode) { m_ReleaseTimer.Reset(); } m_InputMode = newMode; }

		/// <summary>
		/// Gets the analog movement input data.
		/// </summary>
		/// <returns>A vector with the analog movement data, both axes ranging form -1.0 to 1.0.</returns>
		Vector GetAnalogMove() const { return m_AnalogMove; }

		/// <summary>
		/// Sets the analog movement vector state of this.
		/// </summary>
		/// <param name="newMove">The new analog movement vector.</param>
		void SetAnalogMove(Vector &newMove) { m_AnalogMove = newMove; }

		/// <summary>
		/// Gets the analog aiming input data.
		/// </summary>
		/// <returns>A vector with the analog aiming data, both axes ranging form -1.0 to 1.0.</returns>
		Vector GetAnalogAim() const { return m_AnalogAim; }

		/// <summary>
		/// Sets the analog aiming vector state of this.
		/// </summary>
		/// <param name="newAim">The new analog aiming vector.</param>
		void SetAnalogAim(Vector &newAim) { m_AnalogAim = newAim; }

		/// <summary>
		/// Gets the analog menu input data.
		/// </summary>
		/// <returns>A vector with the analog menu data, both axes ranging form -1.0 to 1.0.</returns>
		Vector GetAnalogCursor() const { return m_AnalogCursor; }

		/// <summary>
		/// Adds relative movement to a passed-in vector. Uses the appropriate input method currently of this.
		/// </summary>
		/// <param name="cursorPos"> The vector to alter.</param>
		/// <param name="moveScale">The scale of the input. 1.0 is 'normal'.</param>
		/// <returns>Whether the vector was altered or not.</returns>
		bool RelativeCursorMovement(Vector &cursorPos, float moveScale = 1.0F);

		/// <summary>
		/// Indicates whether this is listening to mouse input at all.
		/// </summary>
		/// <returns>Whether this is using mouse input at all.</returns>
		bool IsMouseControlled() const;

		/// <summary>
		/// Gets the relative movement of the mouse since last update.
		/// </summary>
		/// <returns>The relative mouse movements, in both axes.</returns>
		const Vector & GetMouseMovement() const { return m_MouseMovement; }

		/// <summary>
		/// Gets which player's input this is listening to, if in player input mode.
		/// </summary>
		/// <returns>The player number, or -1 if not in player input mode.</returns>
		int GetPlayer() const { return m_InputMode == CIM_PLAYER ? m_Player : -1; }

		/// <summary>
		/// Sets which player's input this is listening to, and will enable player input mode.
		/// </summary>
		/// <param name="player">The player number.</param>
		void SetPlayer(int player) { m_Player = player; if (m_Player >= 0) { m_InputMode = CIM_PLAYER; } }

		/// <summary>
		/// Gets the Team number using this controller.
		/// </summary>
		/// <returns>An int representing the team which this Controller belongs to. 0 is the first team. 0 if no team is using it.</returns>
		int GetTeam() const;

		/// <summary>
		/// Sets the team which is controlling this Controller's controlled Actor.
		/// </summary>
		/// <param name="team">The team number. 0 is the first team.</param>
		void SetTeam(int team);

		/// <summary>
		/// Gets which Actor is being controlled by this. 0 if none.
		/// </summary>
		/// <returns>A pointer to the Actor which is being controlled by this. Ownership is NOT transferred!</returns>
		virtual Actor * GetControlledActor() const { return m_pControlled; }

		/// <summary>
		/// Sets which Actor is supposed to be controlled by this.
		/// </summary>
		/// <param name="pControlled">A pointer to a an Actor which is being controlled by this. Ownership is NOT transferred!</param>
		virtual void SetControlledActor(Actor *pControlled = 0) { m_pControlled = pControlled; }
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates this Controller. Supposed to be done every frame.
		/// </summary>
		virtual void Update();
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// An assignment operator for setting one Controller equal to another.
		/// </summary>
		/// <param name="rhs">A Controller reference.</param>
		/// <returns>A reference to the changed Controller.</returns>
		virtual Controller & operator=(const Controller &rhs);
#pragma endregion

	protected:

		bool m_ControlStates[CONTROLSTATECOUNT]; //!< Control states.
		bool m_Disabled; //!< Quick and easy disable to prevent updates from being made.

		InputMode m_InputMode; //!< The current controller input mode, like AI, player etc.

		Actor *m_pControlled; //!< The actor controlled by this.

		/// <summary>
		/// The last player this controlled. This is necessary so we still have some control after controlled's death.
		/// If this is -1, no player is controlling/ed, even if in player control input mode.
		/// </summary>
		int m_Player;

		int m_Team; //!< The last team this controlled. This is necessary so we still have some control after controlled's death.

		/// <summary>
		/// These are hacks to make the switch to brain shortcut work without immediately switching away by 
		/// detecting the release of the previous and next buttons after pressing them both down to get to the brain.
		/// </summary>		
		bool m_NextIgnore;
		bool m_PrevIgnore;

		//!< These are used to track just a single press on  shortcut button.
		bool m_WeaponChangeNextIgnore;
		bool m_WeaponChangePrevIgnore;
		bool m_WeaponPickupIgnore;
		bool m_WeaponDropIgnore;
		bool m_WeaponReloadIgnore;

		static const int m_ReleaseDelay; //!< The delay between releasing a menu button and activating the regular controls, to avoid accidental input.
		Timer m_ReleaseTimer; //!< Timer for measuring release delays.  
		Timer m_JoyAccelTimer; //!< Timer for measuring analog joystick-controlled cursor acceleration.   
		Timer m_KeyAccelTimer; //!< Timer for measuring keyboard-controlled cursor acceleration.

		Vector m_MouseMovement; //!< Relative mouse movement, if this player uses the mouse.

	private:

#pragma region Update Breakdown
		/// <summary>
		/// Updates the player's inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		/// This method will call both UpdatePlayerPieMenuInput and UpdatePlayerAnalogInput.
		/// </summary>
		void UpdatePlayerInput();

		/// <summary>
		/// Updates the player's PieMenu inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		/// </summary>
		void UpdatePlayerPieMenuInput();

		/// <summary>
		/// Updates the player's analog inputs portion of this Controller. For breaking down Update into more comprehensible chunks.
		/// </summary>
		void UpdatePlayerAnalogInput();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this Controller, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif