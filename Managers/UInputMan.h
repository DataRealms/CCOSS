#ifndef _RTEUINPUTMAN_
#define _RTEUINPUTMAN_

#include "Singleton.h"
#include "Vector.h"
#include "InputScheme.h"

#define g_UInputMan UInputMan::Instance()

namespace RTE {

	class GUIInput;
	class Icon;

	/// <summary>
	/// The singleton manager over the user input.
	/// </summary>
	class UInputMan : public Singleton<UInputMan>, public Serializable {

	public:

		SerializableOverrideMethods

		/// <summary>
		/// Enumeration for the player controllers.
		/// </summary>
		enum Players {
			PLAYER_NONE = -1,
			PLAYER_ONE = 0,
			PLAYER_TWO,
			PLAYER_THREE,
			PLAYER_FOUR,
			MAX_PLAYERS
		};

		/// <summary>
		/// Enumeration for the mouse button actions.
		/// </summary>
		enum MenuCursorButtons {
			MENU_PRIMARY = 0,
			MENU_SECONDARY,
			MENU_EITHER
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a UInputMan object in system memory. Create() should be called before using the object.
		/// </summary>
		UInputMan() { Clear(); }

		/// <summary>
		/// Makes the UInputMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a UInputMan object before deletion from system memory.
		/// </summary>
		~UInputMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the UInputMan object.
		/// </summary>
		void Destroy() { Clear(); }

		/// <summary>
		/// Resets the entire UInputMan, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Loads the input device icons from loaded presets. This will be called from LoadingGUI after modules are loaded. Can't do this during Create() because the presets don't exist.
		/// </summary>
		void LoadDeviceIcons();

		/// <summary>
		/// Re-initializes the keyboard for when windows regains focus. This is really used to work around an Allegro bug.
		/// </summary>
		void ReInitKeyboard() { install_keyboard(); }

		/// <summary>
		/// Updates the state of this UInputMan. Supposed to be done every frame.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Update();
#pragma endregion

#pragma region Control Scheme and Input Mapping Handling
		/// <summary>
		/// Sets the input class for use if one is available.
		/// </summary>
		/// <param name="inputClass">The input class to set.</param>
		void SetInputClass(GUIInput *inputClass);

		/// <summary>
		/// Access a specific players' control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to get the scheme for.</param>
		/// <returns>A pointer to the requested player's control scheme. Ownership is NOT transferred!</returns>
		InputScheme * GetControlScheme(short whichPlayer) { return m_OverrideInput ? &m_ControlScheme[0] : &m_ControlScheme[whichPlayer]; }

		/// <summary>
		/// Get the current device Icon of a specific player's scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to get the scheme device icon of.</param>
		/// <returns>A const pointer to the requested player's control scheme icon. Ownership is NOT transferred!</returns>
		const Icon * GetSchemeIcon(short whichPlayer) const;

		/// <summary>
		/// Get the current device Icon of a specific device.
		/// </summary>
		/// <param name="whichDevice">Which device to get the icon of.</param>
		/// <returns>A const pointer to the requested device's control scheme icon. Ownership is NOT transferred!</returns>
		const Icon * GetDeviceIcon(int whichDevice) { return (whichDevice < DEVICE_KEYB_ONLY || whichDevice > DEVICE_GAMEPAD_4) ? 0 : m_DeviceIcons[whichDevice]; }

		/// <summary>
		/// Clears a all mappings for a specific input element of a specific player.
		/// </summary>
		/// <param name="whichPlayer">Which player to affect.</param>
		/// <param name="whichInput">Which input element to clear all mappings of.</param>
		void ClearMapping(short whichPlayer, int whichInput) { m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].Reset(); }

		/// <summary>
		/// Gets the name of the key/mouse/joystick button/direction that a particular input element is mapped to.
		/// </summary>
		/// <param name="whichPlayer">Which player to look up.</param>
		/// <param name="whichElement">Which input element to look up.</param>
		/// <returns>A string with the appropriate clear text description of the mapped thing.</returns>
		std::string GetMappingName(short whichPlayer, int whichElement);

		/// <summary>
		/// Gets which keyboard key is mapped to a specific input element.
		/// </summary>
		/// <param name="whichPlayer">Which player to look up.</param>
		/// <param name="whichInput">Which input element to look up.</param>
		/// <returns>Which keyboard key is mapped to the specified player and element.</returns>
		int GetKeyMapping(short whichPlayer, int whichInput) { return m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].GetKey(); }

		/// <summary>
		/// Sets a keyboard key mapped to a specific input element.
		/// </summary>
		/// <param name="whichPlayer">Which player to do this for.</param>
		/// <param name="whichInput">Which input element to map to.</param>
		/// <param name="whichKey">The scancode of which keyboard key to map to above input element.</param>
		void SetKeyMapping(short whichPlayer, int whichInput, int whichKey) { m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].SetKey(whichKey); }

		/// <summary>
		/// Gets which joystick button is mapped to a specific input element.
		/// </summary>
		/// <param name="whichPlayer">Which player to look up.</param>
		/// <param name="whichInput">Which input element to look up.</param>
		/// <returns>Which joystick button is mapped to the specified player and element.</returns>
		int GetButtonMapping(short whichPlayer, int whichInput) { return m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].GetJoyButton(); }

		/// <summary>
		/// Sets a joystick button mapped to a specific input element.
		/// </summary>
		/// <param name="whichPlayer">Which player to do this for.</param>
		/// <param name="whichInput">Which input element to map to.</param>
		/// <param name="whichButton">Which joystick button to map to above input element.</param>
		void SetButtonMapping(short whichPlayer, int whichInput, int whichButton) { m_ControlScheme[whichPlayer].GetInputMappings()[whichInput].SetJoyButton(whichButton); }

		/// <summary>
		/// Checks for the any key press this frame and creates an input mapping for a specific player accordingly.
		/// </summary>
		/// <param name="whichPlayer">Which player to do create a map for.</param>
		/// <param name="whichInput">Which input element to map to for that player.</param>
		/// <returns>Whether there were any key presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureKeyMapping(short whichPlayer, int whichInput);

		/// <summary>
		/// Checks for the any button press this frame and creates an input mapping for a specific player accordingly.
		/// </summary>
		/// <param name="whichPlayer">Which player to do create a map for.</param>
		/// <param name="whichJoy">Which joystick to scan for button presses.</param>
		/// <param name="whichInput">Which input element to map to for that player.</param>
		/// <returns>Whether there were any button presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureButtonMapping(short whichPlayer, int whichJoy, int whichInput);

		/// <summary>
		/// Checks for the any joystick pad or stick direction press this frame and creates an input mapping for a specific player accordingly.
		/// </summary>
		/// <param name="whichPlayer">Which player to do create a map for.</param>
		/// <param name="whichJoy">Which joystick to scan for pad and stick direction presses.</param>
		/// <param name="whichInput">Which input element to map to for that player.</param>
		/// <returns>Whether there were any direction presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureDirectionMapping(short whichPlayer, int whichJoy, int whichInput);

		/// <summary>
		/// Checks for the any button or direction press this frame and creates an input mapping for a specific player accordingly.
		/// </summary>
		/// <param name="whichPlayer">Which player to do create a map for.</param>
		/// <param name="whichJoy">Which joystick to scan for button and stick presses.</param>
		/// <param name="whichInput">Which input element to map to for that player.</param>
		/// <returns>Whether there were any button or stick presses this frame and therefore whether a mapping was successfully captured or not.</returns>
		bool CaptureJoystickMapping(short whichPlayer, int whichJoy, int whichInput);
#pragma endregion

#pragma region General Input Handling
		/// <summary>
		/// Returns last device which affected GUI cursor position.
		/// </summary>
		/// <returns>The last device which affected GUI cursor position.</returns>
		int GetLastDeviceWhichControlledGUICursor() const { return m_LastDeviceWhichControlledGUICursor; }

		/// <summary>
		/// Gets the analog moving values of a specific player's control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0, in both axes.</returns>
		Vector AnalogMoveValues(short whichPlayer = 0);

		/// <summary>
		/// Gets the analog aiming values of a specific player's control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0, in both axes.</returns>
		Vector AnalogAimValues(short whichPlayer = 0);

		/// <summary>
		/// Shows whether a specific input element was held during the last update.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for..</param>
		/// <returns>Element held or not.</returns>
		bool ElementHeld(short whichPlayer, int whichElement);

		/// <summary>
		/// Shows whether a specific input element was depressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for..</param>
		/// <returns>Element pressed or not.</returns>
		bool ElementPressed(short whichPlayer, int whichElement);

		/// <summary>
		/// Shows whether a specific input element was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for..</param>
		/// <returns>Element released or not.</returns>
		bool ElementReleased(short whichPlayer, int whichElement);

		/// <summary>
		/// Gets the generic direction input from any and all players which can affect a shared menu cursor. Normalized to 1.0 max.
		/// </summary>
		/// <returns>The vector with the directional input from any or all players.</returns>
		Vector GetMenuDirectional();

		/// <summary>
		/// Shows whether any generic button with the menu cursor is held down.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Held or not.</returns>
		bool MenuButtonHeld(int whichButton);

		/// <summary>
		/// Shows whether any generic button with the menu cursor was pressed between previous update and this.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Pressed or not.</returns>
		bool MenuButtonPressed(int whichButton);

		/// <summary>
		/// Shows whether any generic button with the menu cursor was released between previous update and this.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Released or not.</returns>
		bool MenuButtonReleased(int whichButton);

		/// <summary>
		/// Returns the state of the Ctrl key.
		/// </summary>
		/// <returns>The state of the Ctrl key.</returns>
		bool FlagCtrlState() const { return ((key_shifts & KB_CTRL_FLAG) > 0) ? true : false; }

		/// <summary>
		/// Returns the state of the Alt key.
		/// </summary>
		/// <returns>The state of the Alt key.</returns>
		bool FlagAltState() const { return ((key_shifts & KB_ALT_FLAG) > 0) ? true : false; }

		/// <summary>
		/// Returns the state of the Shift key.
		/// </summary>
		/// <returns>The state of the Shift key.</returns>
		bool FlagShiftState() const { return ((key_shifts & KB_SHIFT_FLAG) > 0) ? true : false; }
#pragma endregion

#pragma region Keyboard Handling
		/// <summary>
		/// For temporarily disabling of most keyboard keys when typing into a dialog is required for example.
		/// </summary>
		/// <param name="disable">Whether to disable most keys or not.</param>
		void DisableKeys(bool disable = true) { m_DisableKeyboard = disable; }

		/// <summary>
		/// Shows whether a key is being held right now.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Held or not.</returns>
		bool KeyHeld(const char keyToTest) { return (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC)) ? false : s_PrevKeyStates[keyToTest]; }

		/// <summary>
		/// Shows the scancode of the keyboard key which is currently down.
		/// </summary>
		/// <returns>The scancode of the first keyboard key in the keyboard buffer. 0 means none.</returns>
		int WhichKeyHeld() { int key = readkey(); return key >> 8; }

		/// <summary>
		/// Shows whether a key was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Pressed or not.</returns>
		bool KeyPressed(const char keyToTest);

		/// <summary>
		/// Shows whether a key was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Released or not.</returns>
		bool KeyReleased(const char keyToTest);

		/// <summary>
		/// Return true if there is any input at all, keyboard or buttons or D-pad.
		/// </summary>
		/// <returns>Whether any buttons of pads are pressed at all.</returns>
		bool AnyInput();

		/// <summary>
		/// Return true if there is any key, button, or D-pad presses at all. MUST call Update before calling this for it to work properly!
		/// </summary>
		/// <returns>Whether any buttons of pads have been pressed at all since last frame.</returns>
		bool AnyPress();

		/// <summary>
		/// Return true if there is any start key/button presses at all. MUST call Update before calling this for it to work properly!
		/// </summary>
		/// <returns>Whether any start buttons or keys have been pressed at all since last frame.</returns>
		bool AnyStartPress();
#pragma endregion

#pragma region Mouse Handling
		/// <summary>
		/// Reports which player is using the mouse for control at this time if any.
		/// </summary>
		/// <returns>Which player is using the mouse. If no one is then -1 is returned.</returns>
		int MouseUsedByPlayer() const;

		/// <summary>
		/// Will temporarily disable positioning of the mouse.
		/// This is so that when focus is switched back to the game window, it avoids having the window fly away because the user clicked the title bar of the window.
		/// </summary>
		/// <param name="disable">Whether to disable or not.</param>
		void DisableMouseMoving(bool disable = true);

		/// <summary>
		/// Gets the relative movement of the mouse since last update. Only returns true if the selected player actual is using the mouse.
		/// </summary>
		/// <param name="whichPlayer">Which player to get movement for. If the player doesn't use the mouse this always returns a zero vector.</param>
		/// <returns>The relative mouse movements, in both axes.</returns>
		Vector GetMouseMovement(short whichPlayer = -1);

		/// <summary>
		/// Set the mouse's analog emulation output to be of a specific normalized magnitude.
		/// </summary>
		/// <param name="magCap">The normalized magnitude, between 0 and 1.0.</param>
		void SetMouseValueMagnitude(float magCap) { m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius * magCap); }

		/// <summary>
		/// Sets the absolute screen position of the mouse cursor.
		/// </summary>
		/// <param name="newPos">Where to place the mouse.</param>
		/// <param name="whichPlayer">Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.</param>
		void SetMousePos(Vector &newPos, short whichPlayer = -1);

		/// <summary>
		/// Gets mouse sensitivity.
		/// </summary>
		/// <returns>The current mouse sensitivity.</returns>
		float GetMouseSensitivity() const { return m_MouseSensitivity; }

		/// <summary>
		/// Sets mouse sensitivity.
		/// </summary>
		/// <param name="sensitivity">New sensitivity value.</param>
		void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }

		/// <summary>
		/// Shows whether a mouse button is being held down right now.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Held or not.</returns>
		bool MouseButtonHeld(int whichButton, short whichPlayer);

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mouse button is being held down right now.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Held or not.</returns>
		bool MouseButtonHeld(int whichButton) { return MouseButtonHeld(whichButton, 0); };

		/// <summary>
		/// Shows whether a mouse button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Pressed or not.</returns>
		bool MouseButtonPressed(int whichButton, short whichPlayer);

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mouse button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Pressed or not.</returns>
		bool MouseButtonPressed(int whichButton) { return MouseButtonPressed(whichButton, 0); };

		/// <summary>
		/// Shows whether a mouse button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Released or not.</returns>
		bool MouseButtonReleased(int whichButton, short whichPlayer);

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mouse button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Released or not.</returns>
		bool MouseButtonReleased(int whichButton) { return MouseButtonReleased(whichButton, 0); };

		/// <summary>
		/// Shows whether the mouse wheel has been moved past the threshold limit in either direction this frame.
		/// </summary>
		/// <returns>The direction the mouse wheel has been moved which is past that threshold. 0 means not past, negative means moved down, positive means moved up.</returns>
		int MouseWheelMoved() const { return m_MouseWheelChange; }

		/// <summary>
		/// Gets the relative mouse wheel position for the specified player.
		/// </summary>
		/// <param name="player">The player to get mouse wheel position for.</param>
		/// <returns>The relative mouse wheel position for the specified player.</returns>
		int MouseWheelMovedByPlayer(short player) const { return (m_OverrideInput && player >= 0 && player < MAX_PLAYERS) ? m_NetworkMouseWheelState[player] : m_MouseWheelChange; }

		/// <summary>
		/// Return true if there is any mouse button presses at all.
		/// </summary>
		/// <returns>Whether any mouse buttons have been pressed at all since last frame.</returns>
		bool AnyMouseButtonPress();

		/// <summary>
		/// Sets the mouse to be trapped in the middle of the screen so it doesn't go out and click on other windows etc.
		/// This is usually used when the cursor is invisible and only relative mouse movements are used.
		/// </summary>
		/// <param name="trap">Whether to trap the mouse or not.</param>
		/// <param name="whichPlayer">
		/// Which player is trying to control the mouse.
		/// Only the player with actual control over the mouse will affect its trapping here. -1 means change mouse trapping regardless of player.
		/// </param>
		void TrapMousePos(bool trap = true, short whichPlayer = -1);

		/// <summary>
		/// Forces the mouse within a box on the screen.
		/// </summary>
		/// <param name="x">X value of the top left corner of the screen box to keep the mouse within, in screen coordinates.</param>
		/// <param name="y">Y value of the top left corner of the screen box to keep the mouse within, in screen coordinates.</param>
		/// <param name="width">The width of the box.</param>
		/// <param name="height">The height of the box.</param>
		/// <param name="whichPlayer">Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.</param>
		void ForceMouseWithinBox(int x, int y, int width, int height, short whichPlayer = -1);

		/// <summary>
		/// Forces the mouse within a specific player's screen area.
		/// </summary>
		/// <param name="whichPlayer">Which player's screen to constrain the mouse to. Only the player with actual control over the mouse will be affected.</param>
		void ForceMouseWithinPlayerScreen(short whichPlayer);
#pragma endregion

#pragma region Joystick Handling
		/// <summary>
		/// Returns the number of active joysticks.
		/// </summary>
		/// <returns>The number of active joysticks.</returns>
		int GetJoystickCount() const { return (num_joysticks > MAX_PLAYERS) ? MAX_PLAYERS : num_joysticks; }

		/// <summary>
		/// Returns true if specified joystick is active.
		/// </summary>
		/// <param name="joystickNumber">Joystick to check for.</param>
		/// <returns>Whether the specified joystick is active.</returns>
		bool JoystickActive(int joystickNumber) const;

		/// <summary>
		/// Shows whether a joystick button is being held down right now.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Held or not.</returns>
		bool JoyButtonHeld(int whichJoy, int whichButton);

		/// <summary>
		/// Shows the first joystick button which is currently down.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>The first button in the sequence of button enumerations that is held at the time of calling this. JOY_NONE means none.</returns>
		int WhichJoyButtonHeld(int whichJoy);

		/// <summary>
		/// Shows whether a joystick button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Pressed or not.</returns>
		bool JoyButtonPressed(int whichJoy, int whichButton);

		/// <summary>
		/// Shows the first joystick button which was pressed down since last frame.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>The first button in the sequence of button enumerations that is pressed since the previous frame. JOY_NONE means none.</returns>
		int WhichJoyButtonPressed(int whichJoy);

		/// <summary>
		/// Shows whether a joystick button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Released or not.</returns>
		bool JoyButtonReleased(int whichJoy, int whichButton);

		/// <summary>
		/// Shows whether a joystick axis is being held down in a specific direction right now. Two adjacent directions can be held down to produce diagonals.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Held or not.</returns>
		bool JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir);

		/// <summary>
		/// Shows whether a joystick axis direction was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Pressed or not.</returns>
		bool JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir);

		/// <summary>
		/// Shows whether a joystick axis direction was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Released or not.</returns>
		bool JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir);

		/// <summary>
		/// Gets the normalized value of a certain joystick's stick's axis.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <returns>The analog axis value ranging between -1.0 to 1.0, or 0.0 to 1.0 if it's a throttle type control.</returns>
		float AnalogAxisValue(int whichJoy = 0, int whichStick = 0, int whichAxis = 0);

		/// <summary>
		/// Gets the analog values of a certain joystick device stick.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0.</returns>
		Vector AnalogStickValues(int whichJoy = 0, int whichStick = 0) { return Vector(AnalogAxisValue(whichJoy, whichStick, 0), AnalogAxisValue(whichJoy, whichStick, 1)); }

		/// <summary>
		/// Return true if there is any joystick input at all, buttons or D-pad.
		/// </summary>
		/// <returns>Whether any buttons of pads are pressed at all.</returns>
		bool AnyJoyInput();

		/// <summary>
		/// Return true if there is any joystick presses at all, buttons or D-pad.
		/// </summary>
		/// <returns>Whether any buttons or pads have been pressed at all since last frame.</returns>
		bool AnyJoyPress();

		/// <summary>
		/// Return true if there is any joystick button presses at all, but not D-pad input, for a specific joystick.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>Whether any joystick buttons have been pressed at all since last frame, of a specific joystick.</returns>
		bool AnyJoyButtonPress(int whichJoy);
#pragma endregion

#pragma region Network Handling
		/// <summary>
		/// Returns true if manager is in multiplayer mode.
		/// </summary>
		/// <returns>True if in multiplayer mode.</returns>
		bool IsInMultiplayerMode() const { return m_OverrideInput; }

		/// <summary>
		/// Sets the multiplayer mode flag.
		/// </summary>
		/// <param name="value">Whether this manager should operate in multiplayer mode.</param>
		void SetMultiplayerMode(bool value) { m_OverrideInput = value; }

		/// <summary>
		/// Gets the position of the mouse for a player during network multiplayer.
		/// </summary>
		/// <param name="player">The player to get for.</param>
		/// <returns>The position of the mouse for the specified player</returns>
		Vector GetNetworkAccumulatedRawMouseMovement(short player);

		/// <summary>
		/// Sets the position of the mouse for a player during network multiplayer.
		/// </summary>
		/// <param name="player">The player to set for.</param>
		/// <param name="input">The new position of the mouse.</param>
		void SetNetworkMouseMovement(short player, Vector input) { m_NetworkAccumulatedRawMouseMovement[player] = input; }

		/// <summary>
		/// Sets whether an input element is held by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementHeldState(short player, int element, bool state);

		/// <summary>
		/// Sets whether an input element is pressed by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementPressedState(short player, int element, bool state);

		/// <summary>
		/// Sets whether an input element is released by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementReleasedState(short player, int element, bool state);

		/// <summary>
		/// Sets whether a mouse button is held by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonHeldState(short player, int whichButton, bool state);

		/// <summary>
		/// Sets whether a mouse button is pressed by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonPressedState(short player, int whichButton, bool state);

		/// <summary>
		/// Sets whether a mouse button is released by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonReleasedState(short player, int whichButton, bool state);

		/// <summary>
		/// Sets the state of the mouse wheel for a player during network multiplayer.
		/// </summary>
		/// <param name="player">The player to set for.</param>
		/// <param name="state">The new state of the mouse wheel.</param>
		void SetNetworkMouseWheelState(short player, int state);

		/// <summary>
		/// Gets whether the specified input element is pressed during network multiplayer.
		/// </summary>
		/// <param name="element">The input element to check for.</param>
		/// <returns>Whether the specified input element is pressed or not.</returns>
		bool AccumulatedElementPressed(int element) { return (element < 0 || element >= INPUT_COUNT) ? false : m_NetworkAccumulatedElementPressed[element]; }

		/// <summary>
		/// Gets whether the specified input element is released during network multiplayer.
		/// </summary>
		/// <param name="element">The input element to check for.</param>
		/// <returns>Whether the specified input element is released or not.</returns>
		bool AccumulatedElementReleased(int element) const { return (element < 0 || element >= INPUT_COUNT) ? false : m_NetworkAccumulatedElementReleased[element]; }

		/// <summary>
		/// Clears all the accumulated input element states.
		/// </summary>
		void ClearAccumulatedStates();
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		static GUIInput* s_InputClass; //!< Current input class if available.

		static char *s_PrevKeyStates; //!< Key states as they were the previous update.
		static char *s_ChangedKeyStates; //!< Key states that have changed.

		//TODO: Figure out why we need this for mouse but not for other input devices.
		static bool s_MouseButtonStates[MAX_MOUSE_BUTTONS]; //!< Current? mouse states.

		static bool s_PrevMouseButtonStates[MAX_MOUSE_BUTTONS]; //!< Mouse states as they were the previous update.
		static bool s_ChangedMouseButtonStates[MAX_MOUSE_BUTTONS]; //!< Mouse states that have changed.

		static JOYSTICK_INFO s_PrevJoystickStates[MAX_PLAYERS]; //!< Joystick states as they were the previous update.
		static JOYSTICK_INFO s_ChangedJoystickStates[MAX_PLAYERS]; //!< Joystick states that have changed.

		bool m_OverrideInput; //!< If true then this instance operates in multiplayer mode and the input is overridden by network input.

		InputScheme m_ControlScheme[MAX_PLAYERS]; //!< Which control scheme is being used by each player.
		const Icon *m_DeviceIcons[DEVICE_COUNT]; //!< The Icons representing all different devices.
	
		Vector m_RawMouseMovement; //!< The raw absolute movement of the mouse between the last two Updates.
		Vector m_AnalogMouseData; //!< The emulated analog stick position of the mouse.
		float m_MouseSensitivity; //!< Mouse sensitivity, to replace hardcoded 0.6 value in Update.
		int m_MouseWheelChange; //!< The relative mouse wheel position since last reset of it.
		
		bool m_TrapMousePos; //!< Whether the mouse is trapped in the middle of the screen each update or not.
		float m_MouseTrapRadius; //!< The radius (in pixels) of the circle trapping the mouse for analog mouse data.

		int m_LastDeviceWhichControlledGUICursor; //!< Indicates which device controlled the cursor last time.

		bool m_DisableKeyboard; //!< Temporarily disable all keyboard input reading.
		bool m_DisableMouseMoving; //!< Temporary disable for positioning the mouse, for when the game window is not in focus.

		/// <summary>
		/// This is set when focus is switched back to the game window and will cause the m_DisableMouseMoving to switch to false when the mouse button is RELEASED.
		/// This is to avoid having the window fly away because the user clicked the title bar.
		/// </summary>
		bool m_PrepareToEnableMouseMoving;
 
		bool m_NetworkAccumulatedElementPressed[INPUT_COUNT]; //!< Whether an input element is pressed or not during network multiplayer.
		bool m_NetworkAccumulatedElementReleased[INPUT_COUNT]; //!< Whether an input element is released or not during network multiplayer.

		bool m_NetworkInputElementHeld[MAX_PLAYERS][INPUT_COUNT]; //!< Whether an input element is held by a player during network multiplayer.
		bool m_NetworkInputElementPressed[MAX_PLAYERS][INPUT_COUNT]; //!< Whether an input element is pressed by a player during network multiplayer.
		bool m_NetworkInputElementReleased[MAX_PLAYERS][INPUT_COUNT]; //!< Whether an input element is released by a player during network multiplayer.

		bool m_NetworkMouseButtonHeldState[MAX_PLAYERS][MAX_MOUSE_BUTTONS]; //!< Whether a mouse button is held by a player during network multiplayer.
		bool m_NetworkMouseButtonPressedState[MAX_PLAYERS][MAX_MOUSE_BUTTONS]; //!< Whether a mouse button is pressed by a player during network multiplayer.
		bool m_NetworkMouseButtonReleasedState[MAX_PLAYERS][MAX_MOUSE_BUTTONS]; //!< Whether a mouse button is released by a player during network multiplayer.

		Vector m_NetworkAccumulatedRawMouseMovement[MAX_PLAYERS]; //!< The position of the mouse for each player during network multiplayer.
		Vector m_NetworkAnalogMoveData[MAX_PLAYERS]; //!< Mouse analog movement data for each player during network multiplayer. 
		int m_NetworkMouseWheelState[MAX_PLAYERS]; //!< The position of a player's mouse wheel during network multiplayer.

		bool m_TrapMousePosPerPlayer[MAX_PLAYERS]; //!< Whether to trap the mouse position to the middle of the screen for each player during network multiplayer.

	private:

#pragma region Update Breakdown
		/// <summary>
		/// Handles the mouse input in network multiplayer. This is called from Update().
		/// </summary>
		void UpdateNetworkMouseMovement();

		/// <summary>
		/// Handles the mouse input. This is called from Update().
		/// </summary>
		void UpdateMouseInput();

		/// <summary>
		/// Handles the joysticks input. This is called from Update().
		/// </summary>
		void UpdateJoystickInput();

		/// <summary>
		/// Stores all the input events that happened during this update to be compared to in the next update. This is called from Update().
		/// </summary>
		void StoreInputEventsForNextUpdate();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this UInputMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		UInputMan(const UInputMan &reference) {}
		UInputMan & operator=(const UInputMan &rhs) {}
	};
}
#endif