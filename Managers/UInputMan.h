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
	/// The singleton manager responsible for handling user input.
	/// </summary>
	class UInputMan : public Singleton<UInputMan> {
		friend class SettingsMan;

	public:

		/// <summary>
		/// Enumeration for the mouse cursor actions in menus.
		/// </summary>
		enum MenuCursorButtons { MENU_PRIMARY, MENU_SECONDARY, MENU_EITHER };

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a UInputMan object in system memory. Create() should be called before using the object.
		/// </summary>
		UInputMan() { Clear(); }

		/// <summary>
		/// Makes the UInputMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
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
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Workaround for Allegro being unable to detect joystick plugging/unplugging at runtime to enable/disable them accordingly. Uses OS functions to check if the number of connected joysticks changed and reinitializes the joystick handler if necessary.
		/// </summary>
		/// <returns>Whether plugging/unplugging was detected and the joystick handler was reinitialized.</returns>
		bool DetectJoystickHotPlug() const;

		/// <summary>
		/// Loads the input device icons from loaded presets. Can't do this during Create() because the presets don't exist so this will be called from MenuMan::Initialize() after modules are loaded.
		/// </summary>
		void LoadDeviceIcons();

		/// <summary>
		/// Updates the state of this UInputMan. Supposed to be done every frame.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Update();
#pragma endregion

#pragma region Control Scheme and Input Mapping Handling
		/// <summary>
		/// Sets the GUIInput instance to capture key state from. This is used for better key detection during input mapping input capture.
		/// </summary>
		/// <param name="guiInputInstance">Pointer to the GUIInput instance to capture key state from, or nullptr if using UInputMan input capture.</param>
		void SetGUIInputInstanceToCaptureKeyStateFrom(GUIInput *guiInputInstance) const;

		/// <summary>
		/// Gets the currently used input device of the specified player.
		/// </summary>
		/// <param name="whichPlayer">Which player to get input device for.</param>
		/// <returns>A number value representing the currently used input device of this player. See InputDevice enumeration for values.</returns>
		int GetInputDevice(int whichPlayer) const { return m_ControlScheme.at(whichPlayer).GetDevice(); }

		/// <summary>
		/// Access a specific player's control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to get the scheme for.</param>
		/// <returns>A pointer to the requested player's control scheme. Ownership is NOT transferred!</returns>
		InputScheme * GetControlScheme(int whichPlayer) { return IsInMultiplayerMode() ? &m_ControlScheme.at(Players::PlayerOne) : &m_ControlScheme.at(whichPlayer); }

		/// <summary>
		/// Get the current device Icon of a specific player's scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to get the scheme device icon of.</param>
		/// <returns>A const pointer to the requested player's control scheme icon. Ownership is NOT transferred!</returns>
		const Icon * GetSchemeIcon(int whichPlayer) const { return (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) ? nullptr : m_DeviceIcons[m_ControlScheme.at(whichPlayer).GetDevice()]; }

		/// <summary>
		/// Get the current device Icon of a specific device.
		/// </summary>
		/// <param name="whichDevice">Which device to get the icon of.</param>
		/// <returns>A const pointer to the requested device's control scheme icon. Ownership is NOT transferred!</returns>
		const Icon * GetDeviceIcon(int whichDevice) const { return (whichDevice < InputDevice::DEVICE_KEYB_ONLY || whichDevice > InputDevice::DEVICE_GAMEPAD_4) ? nullptr : m_DeviceIcons[whichDevice]; }
#pragma endregion

#pragma region General Input Handling
		/// <summary>
		/// Gets the last device which affected GUI cursor position.
		/// </summary>
		/// <returns>The last device which affected GUI cursor position.</returns>
		InputDevice GetLastDeviceWhichControlledGUICursor() const { return m_LastDeviceWhichControlledGUICursor; }

		/// <summary>
		/// Gets the analog moving values of a specific player's control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0, in both axes.</returns>
		Vector AnalogMoveValues(int whichPlayer = 0);

		/// <summary>
		/// Gets the analog aiming values of a specific player's control scheme.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0, in both axes.</returns>
		Vector AnalogAimValues(int whichPlayer = 0);

		/// <summary>
		/// Gets whether a specific input element was held during the last update.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for.</param>
		/// <returns>Whether the element is held or not.</returns>
		bool ElementHeld(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Held); }

		/// <summary>
		/// Gets whether a specific input element was depressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for.</param>
		/// <returns>Whether the element is pressed or not.</returns>
		bool ElementPressed(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Pressed); }

		/// <summary>
		/// Gets whether a specific input element was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <param name="whichElement">Which element to check for.</param>
		/// <returns>Whether the element is released or not.</returns>
		bool ElementReleased(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Released); }

		/// <summary>
		/// Gets the generic direction input from any and all players which can affect a shared menu cursor. Normalized to 1.0 max.
		/// </summary>
		/// <returns>The vector with the directional input from any or all players.</returns>
		Vector GetMenuDirectional();

		/// <summary>
		/// Gets whether any generic button with the menu cursor is held down.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Whether the button is held or not.</returns>
		bool MenuButtonHeld(int whichButton) { return GetMenuButtonState(whichButton, InputState::Held); }

		/// <summary>
		/// Gets whether any generic button with the menu cursor was pressed between previous update and this.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Whether the button is pressed or not.</returns>
		bool MenuButtonPressed(int whichButton) { return GetMenuButtonState(whichButton, InputState::Pressed); }

		/// <summary>
		/// Gets whether any generic button with the menu cursor was released between previous update and this.
		/// </summary>
		/// <param name="whichButton">Which generic menu cursor button to check for.</param>
		/// <returns>Whether the button is released or not.</returns>
		bool MenuButtonReleased(int whichButton) { return GetMenuButtonState(whichButton, InputState::Released); }

		/// <summary>
		/// Gets whether there is any input at all, keyboard or buttons or D-pad.
		/// </summary>
		/// <returns>Whether any buttons of pads are pressed at all.</returns>
		bool AnyKeyOrJoyInput() const;

		/// <summary>
		/// Gets whether there are any key, button, or D-pad presses at all. MUST call Update before calling this for it to work properly!
		/// </summary>
		/// <returns>Whether any buttons of pads have been pressed at all since last frame.</returns>
		bool AnyPress() const;

		/// <summary>
		/// Gets whether there are any start key/button presses at all. MUST call Update before calling this for it to work properly!
		/// </summary>
		/// <param="includeSpacebar">Whether to check for space bar presses or not. 
		/// <returns>Whether any start buttons or keys have been pressed at all since last frame.</returns>
		bool AnyStartPress(bool includeSpacebar = true);

		/// <summary>
		/// Gets whether there are any back button presses at all. MUST call Update before calling this for it to work properly!
		/// </summary>
		/// <returns>Whether any back buttons have been pressed at all since last frame.</returns>
		bool AnyBackPress();

		/// <summary>
		/// Gets the state of the Ctrl key.
		/// </summary>
		/// <returns>The state of the Ctrl key.</returns>
		bool FlagCtrlState() const { return ((key_shifts & KB_CTRL_FLAG) > 0) ? true : false; }

		/// <summary>
		/// Gets the state of the Alt key.
		/// </summary>
		/// <returns>The state of the Alt key.</returns>
		bool FlagAltState() const { return ((key_shifts & KB_ALT_FLAG) > 0) ? true : false; }

		/// <summary>
		/// Gets the state of the Shift key.
		/// </summary>
		/// <returns>The state of the Shift key.</returns>
		bool FlagShiftState() const { return ((key_shifts & KB_SHIFT_FLAG) > 0) ? true : false; }
#pragma endregion

#pragma region Keyboard Handling
		/// <summary>
		/// Temporarily disables most of the keyboard keys. This is used when typing into a dialog box is required.
		/// </summary>
		/// <param name="disable">Whether to disable most keys or not.</param>
		void DisableKeys(bool disable = true) { m_DisableKeyboard = disable; }

		/// <summary>
		/// Gets whether a key is being held right now.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Whether the key is held or not.</returns>
		bool KeyHeld(const char keyToTest) const { return GetKeyboardButtonState(keyToTest, InputState::Held); }

		/// <summary>
		/// Shows the scancode of the keyboard key which is currently down.
		/// </summary>
		/// <returns>The scancode of the first keyboard key in the keyboard buffer. 0 means none.</returns>
		int WhichKeyHeld() const { int key = readkey(); return key >> 8; }

		/// <summary>
		/// Gets whether a key was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Whether the key is pressed or not.</returns>
		bool KeyPressed(const char keyToTest) const { return GetKeyboardButtonState(keyToTest, InputState::Pressed); }

		/// <summary>
		/// Gets whether a key was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <returns>Whether the key is released or not.</returns>
		bool KeyReleased(const char keyToTest) const { return GetKeyboardButtonState(keyToTest, InputState::Released); }

		/// <summary>
		/// Return true if there are any keyboard button presses at all.
		/// </summary>
		/// <returns>Whether any keyboard buttons have been pressed at all since last frame.</returns>
		bool AnyKeyPress() const;
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
		/// <param name="disable">Whether to disable mouse positioning or not.</param>
		void DisableMouseMoving(bool disable = true);

		/// <summary>
		/// Gets the relative movement of the mouse since last update. Only returns true if the selected player is actually using the mouse.
		/// </summary>
		/// <param name="whichPlayer">Which player to get movement for. If the player doesn't use the mouse this always returns a zero vector.</param>
		/// <returns>The relative mouse movements, in both axes.</returns>
		Vector GetMouseMovement(int whichPlayer = -1) const;

		/// <summary>
		/// Set the mouse's analog emulation output to be of a specific normalized magnitude.
		/// </summary>
		/// <param name="magCap">The normalized magnitude, between 0 and 1.0.</param>
		/// <param name="whichPlayer">Which player to set magnitude for. Only relevant when in online multiplayer mode.</param>
		void SetMouseValueMagnitude(float magCap, int whichPlayer = Players::NoPlayer);

		/// <summary>
		/// Sets the absolute screen position of the mouse cursor.
		/// </summary>
		/// <param name="newPos">Where to place the mouse.</param>
		/// <param name="whichPlayer">Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.</param>
		void SetMousePos(Vector &newPos, int whichPlayer = -1) const;

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
		/// Gets whether a mouse button is being held down right now.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Whether the mouse button is held or not.</returns>
		bool MouseButtonHeld(int whichButton, int whichPlayer) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Held); }

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Gets whether a mouse button is being held down right now.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Whether the mouse button is held or not.</returns>
		bool MouseButtonHeld(int whichButton) const { return GetMouseButtonState(Players::PlayerOne, whichButton, InputState::Held); }

		/// <summary>
		/// Gets whether a mouse button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Whether the mouse button is pressed or not.</returns>
		bool MouseButtonPressed(int whichButton, int whichPlayer) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Pressed); }

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Gets whether a mouse button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Whether the mouse button is pressed or not.</returns>
		bool MouseButtonPressed(int whichButton) const { return GetMouseButtonState(Players::PlayerOne, whichButton, InputState::Pressed); }

		/// <summary>
		/// Gets whether a mouse button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <param name="whichPlayer">Which player to check for.</param>
		/// <returns>Whether the mouse button is released or not.</returns>
		bool MouseButtonReleased(int whichButton, int whichPlayer) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Released); }

		/// <summary>
		/// (ONLY FOR LUA BACKWARD COMPATIBILITY) Gets whether a mouse button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichButton">Which button to check for.</param>
		/// <returns>Whether the mouse button is released or not.</returns>
		bool MouseButtonReleased(int whichButton) const { return GetMouseButtonState(Players::PlayerOne, whichButton, InputState::Released); }

		/// <summary>
		/// Gets whether the mouse wheel has been moved past the threshold limit in either direction this frame.
		/// </summary>
		/// <returns>The direction the mouse wheel has been moved which is past that threshold. 0 means not past, negative means moved down, positive means moved up.</returns>
		int MouseWheelMoved() const { return m_MouseWheelChange; }

		/// <summary>
		/// Gets the relative mouse wheel position for the specified player.
		/// </summary>
		/// <param name="player">The player to get mouse wheel position for.</param>
		/// <returns>The relative mouse wheel position for the specified player.</returns>
		int MouseWheelMovedByPlayer(int player) const {
			return (IsInMultiplayerMode() && player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_NetworkMouseWheelState[player] : m_MouseWheelChange;
		}

		/// <summary>
		/// Return true if there are any mouse button presses at all.
		/// </summary>
		/// <returns>Whether any mouse buttons have been pressed at all since last frame.</returns>
		bool AnyMouseButtonPress() const;

		/// <summary>
		/// Sets the mouse to be trapped in the middle of the screen so it doesn't go out and click on other windows etc.
		/// This is usually used when the cursor is invisible and only relative mouse movements are used.
		/// </summary>
		/// <param name="trap">Whether to trap the mouse or not.</param>
		/// <param name="whichPlayer">
		/// Which player is trying to control the mouse.
		/// Only the player with actual control over the mouse will affect its trapping here. -1 means change mouse trapping regardless of player.
		/// </param>
		void TrapMousePos(bool trap = true, int whichPlayer = -1);

		/// <summary>
		/// Forces the mouse within a box on the screen.
		/// </summary>
		/// <param name="x">X value of the top left corner of the screen box to keep the mouse within, in screen coordinates.</param>
		/// <param name="y">Y value of the top left corner of the screen box to keep the mouse within, in screen coordinates.</param>
		/// <param name="width">The width of the box.</param>
		/// <param name="height">The height of the box.</param>
		/// <param name="whichPlayer">Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.</param>
		void ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer = -1) const;

		/// <summary>
		/// Forces the mouse within a specific player's screen area.
		/// Player 1 will always be in the upper-left corner, Player 3 will always be in the lower-left corner, Player 4 will always be in the lower-right quadrant.
		/// Player 2 will either be in the lower-left corner or the upper-right corner depending on vertical/horizontal splitting.
		/// </summary>
		/// <param name="whichPlayer">Which player's screen to constrain the mouse to. Only the player with actual control over the mouse will be affected.</param>
		void ForceMouseWithinPlayerScreen(int whichPlayer) const;
#pragma endregion

#pragma region Joystick Handling
		/// <summary>
		/// Gets the number of active joysticks.
		/// </summary>
		/// <returns>The number of active joysticks.</returns>
		int GetJoystickCount() const { return (num_joysticks > Players::MaxPlayerCount) ? Players::MaxPlayerCount : num_joysticks; }

		/// <summary>
		/// Gets the index number of a joystick from InputDevice. Basically just subtract 2 from the passed in value because the Allegro joystick indices are 0-3 and ours are 2-5.
		/// </summary>
		/// <param name="device">The InputDevice to get index from.</param>
		/// <returns>The corrected index. A non-joystick device will result in an out of range value returned which will not affect any active joysticks.</returns>
		int GetJoystickIndex(InputDevice device) const { return (device >= InputDevice::DEVICE_GAMEPAD_1 && device < InputDevice::DEVICE_COUNT) ? device - InputDevice::DEVICE_GAMEPAD_1 : InputDevice::DEVICE_COUNT ; }

		/// <summary>
		/// Gets whether the specified joystick is active. The joystick number does not correspond to the player number.
		/// </summary>
		/// <param name="joystickNumber">Joystick to check for.</param>
		/// <returns>Whether the specified joystick is active.</returns>
		bool JoystickActive(int joystickNumber) const { return joystickNumber >= Players::PlayerOne && joystickNumber < Players::MaxPlayerCount && joystickNumber < num_joysticks; }

		/// <summary>
		/// Gets whether a joystick button is being held down right now.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Whether the joystick button is held or not.</returns>
		bool JoyButtonHeld(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Held); }

		/// <summary>
		/// Shows the first joystick button which is currently down.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>The first button in the sequence of button enumerations that is held at the time of calling this. JOY_NONE means none.</returns>
		int WhichJoyButtonHeld(int whichJoy) const;

		/// <summary>
		/// Gets whether a joystick button was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Whether the joystick button is pressed or not.</returns>
		bool JoyButtonPressed(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Pressed); }

		/// <summary>
		/// Shows the first joystick button which was pressed down since last frame.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>The first button in the sequence of button enumerations that is pressed since the previous frame. JOY_NONE means none.</returns>
		int WhichJoyButtonPressed(int whichJoy) const;

		/// <summary>
		/// Gets whether a joystick button was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for.</param>
		/// <returns>Whether the joystick button is released or not.</returns>
		bool JoyButtonReleased(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Released); }

		/// <summary>
		/// Gets whether a joystick axis is being held down in a specific direction right now. Two adjacent directions can be held down to produce diagonals.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Whether the stick axis is held in the specified direction or not.</returns>
		bool JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichStick, whichAxis, whichDir, InputState::Held); }

		/// <summary>
		/// Gets whether a joystick axis direction was pressed between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Whether the stick axis is pressed or not.</returns>
		bool JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichStick, whichAxis, whichDir, InputState::Pressed); }

		/// <summary>
		/// Gets whether a joystick axis direction was released between the last update and the one previous to it.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for.</param>
		/// <returns>Whether the stick axis is released or not.</returns>
		bool JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichStick, whichAxis, whichDir, InputState::Released); }

		/// <summary>
		/// Gets the normalized value of a certain joystick's stick's axis.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <returns>The analog axis value ranging between -1.0 to 1.0, or 0.0 to 1.0 if it's a throttle type control.</returns>
		float AnalogAxisValue(int whichJoy = 0, int whichStick = 0, int whichAxis = 0) const;

		/// <summary>
		/// Gets the analog values of a certain joystick device stick.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <returns>The analog axis values ranging between -1.0 to 1.0.</returns>
		Vector AnalogStickValues(int whichJoy = 0, int whichStick = 0) const { return Vector(AnalogAxisValue(whichJoy, whichStick, 0), AnalogAxisValue(whichJoy, whichStick, 1)); }

		/// <summary>
		/// Gets whether there is any joystick input at all, buttons or D-pad.
		/// </summary>
		/// <param name="checkForPresses">Whether to check specifically for presses since last frame.</param>
		/// <returns>Whether any buttons of pads are pressed at all or since the last frame.</returns>
		bool AnyJoyInput(bool checkForPresses = false) const;

		/// <summary>
		/// Return true if there are any joystick presses at all, buttons or D-pad.
		/// </summary>
		/// <returns>Whether any buttons or pads have been pressed at all since last frame.</returns>
		bool AnyJoyPress() const { return AnyJoyInput(true); }

		/// <summary>
		/// Gets whether there are any joystick button presses at all, but not D-pad input, for a specific joystick.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <returns>Whether any joystick buttons have been pressed at all since last frame, of a specific joystick.</returns>
		bool AnyJoyButtonPress(int whichJoy) const;
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
		Vector GetNetworkAccumulatedRawMouseMovement(int player);

		/// <summary>
		/// Sets the position of the mouse for a player during network multiplayer.
		/// </summary>
		/// <param name="player">The player to set for.</param>
		/// <param name="input">The new position of the mouse.</param>
		void SetNetworkMouseMovement(int player, const Vector &input) { m_NetworkAccumulatedRawMouseMovement[player] = input; }

		/// <summary>
		/// Sets whether an input element is held by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementHeldState(int player, int element, bool state) { SetNetworkInputElementState(player, element, InputState::Held, state); }

		/// <summary>
		/// Sets whether an input element is pressed by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementPressedState(int player, int element, bool state) { SetNetworkInputElementState(player, element, InputState::Pressed, state); }

		/// <summary>
		/// Sets whether an input element is released by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="element">Which input element to set for.</param>
		/// <param name="state">The new state of the input element. True or false.</param>
		void SetNetworkInputElementReleasedState(int player, int element, bool state) { SetNetworkInputElementState(player, element, InputState::Released, state); }

		/// <summary>
		/// Sets whether a mouse button is held by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonHeldState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Held, state); }

		/// <summary>
		/// Sets whether a mouse button is pressed by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonPressedState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Pressed, state); }

		/// <summary>
		/// Sets whether a mouse button is released by a player during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for.</param>
		/// <param name="whichButton">Which mouse button to set for.</param>
		/// <param name="state">The new state of the mouse button. True or false.</param>
		void SetNetworkMouseButtonReleasedState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Released, state); }

		/// <summary>
		/// Sets the state of the mouse wheel for a player during network multiplayer.
		/// </summary>
		/// <param name="player">The player to set for.</param>
		/// <param name="state">The new state of the mouse wheel.</param>
		void SetNetworkMouseWheelState(int player, int state) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) { m_NetworkMouseWheelState[player] = state; } }

		/// <summary>
		/// Gets whether the specified input element is pressed during network multiplayer.
		/// </summary>
		/// <param name="element">The input element to check for.</param>
		/// <returns>Whether the specified input element is pressed or not.</returns>
		bool NetworkAccumulatedElementPressed(int element) const { return NetworkAccumulatedElementState(element, InputState::Pressed); }

		/// <summary>
		/// Gets whether the specified input element is released during network multiplayer.
		/// </summary>
		/// <param name="element">The input element to check for.</param>
		/// <returns>Whether the specified input element is released or not.</returns>
		bool NetworkAccumulatedElementReleased(int element) const { return NetworkAccumulatedElementState(element, InputState::Released); }

		/// <summary>
		/// Clears all the accumulated input element states.
		/// </summary>
		void ClearNetworkAccumulatedStates();
#pragma endregion

	protected:

		/// <summary>
		/// Enumeration for the different states an input element or button can be in.
		/// </summary>
		enum InputState { Held, Pressed, Released, InputStateCount };

		static GUIInput *s_GUIInputInstanceToCaptureKeyStateFrom; //!< Pointer to the GUIInput instance to capture key state from, if any. This is used for better key detection during input mapping input capture.

		static char *s_PrevKeyStates; //!< Key states as they were the previous update.
		static char *s_ChangedKeyStates; //!< Key states that have changed.

		static bool s_CurrentMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS]; //!< Current mouse button states.
		static bool s_PrevMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS]; //!< Mouse button states as they were the previous update.
		static bool s_ChangedMouseButtonStates[MouseButtons::MAX_MOUSE_BUTTONS]; //!< Mouse button states that have changed since previous update.

		static JOYSTICK_INFO s_PrevJoystickStates[Players::MaxPlayerCount]; //!< Joystick states as they were the previous update.
		static JOYSTICK_INFO s_ChangedJoystickStates[Players::MaxPlayerCount]; //!< Joystick states that have changed.

		bool m_OverrideInput; //!< If true then this instance operates in multiplayer mode and the input is overridden by network input.

		std::array<InputScheme, Players::MaxPlayerCount> m_ControlScheme; //!< Which control scheme is being used by each player.
		const Icon *m_DeviceIcons[InputDevice::DEVICE_COUNT]; //!< The Icons representing all different devices.

		Vector m_RawMouseMovement; //!< The raw absolute movement of the mouse between the last two Updates.
		Vector m_AnalogMouseData; //!< The emulated analog stick position of the mouse.
		float m_MouseSensitivity; //!< Mouse sensitivity, to replace hardcoded 0.6 value in Update.
		int m_MouseWheelChange; //!< The relative mouse wheel position since last reset of it.

		bool m_TrapMousePos; //!< Whether the mouse is trapped in the middle of the screen each update or not.
		float m_MouseTrapRadius; //!< The radius (in pixels) of the circle trapping the mouse for analog mouse data.

		InputDevice m_LastDeviceWhichControlledGUICursor; //!< Indicates which device controlled the cursor last time.

		bool m_DisableKeyboard; //!< Temporarily disable all keyboard input reading.
		bool m_DisableMouseMoving; //!< Temporary disable for positioning the mouse, for when the game window is not in focus.

		/// <summary>
		/// This is set when focus is switched back to the game window and will cause the m_DisableMouseMoving to switch to false when the mouse button is RELEASED.
		/// This is to avoid having the window fly away because the user clicked the title bar.
		/// </summary>
		bool m_PrepareToEnableMouseMoving;
 
		bool m_NetworkAccumulatedElementState[InputElements::INPUT_COUNT][InputState::InputStateCount]; //!< The state of an input element during network multiplayer.
		bool m_NetworkInputElementState[Players::MaxPlayerCount][InputElements::INPUT_COUNT][InputState::InputStateCount]; //!< The state of a player's input element during network multiplayer.
		bool m_NetworkMouseButtonState[Players::MaxPlayerCount][MouseButtons::MAX_MOUSE_BUTTONS][InputState::InputStateCount]; //!< The state of a player's mouse button during network multiplayer. 

		Vector m_NetworkAccumulatedRawMouseMovement[Players::MaxPlayerCount]; //!< The position of the mouse for each player during network multiplayer.
		Vector m_NetworkAnalogMoveData[Players::MaxPlayerCount]; //!< Mouse analog movement data for each player during network multiplayer. 
		int m_NetworkMouseWheelState[Players::MaxPlayerCount]; //!< The position of a player's mouse wheel during network multiplayer.

		bool m_TrapMousePosPerPlayer[Players::MaxPlayerCount]; //!< Whether to trap the mouse position to the middle of the screen for each player during network multiplayer.

	private:

#pragma region Input State Handling
		/// <summary>
		/// Gets whether an input element is in the specified state.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for. See Players enumeration.</param>
		/// <param name="whichElement">Which element to check for. See InputElements enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the element is in the specified state or not.</returns>
		bool GetInputElementState(int whichPlayer, int whichElement, InputState whichState);

		/// <summary>
		/// Gets whether any generic button with the menu cursor is in the specified state.
		/// </summary>
		/// <param name="whichButton">Which menu button to check for. See MenuButtons enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the menu button is in the specified state or not.</returns>
		bool GetMenuButtonState(int whichButton, InputState whichState) ;

		/// <summary>
		/// Gets whether a keyboard key is in the specified state.
		/// </summary>
		/// <param name="keyToTest">A const char with the Allegro-defined key enumeration to test.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the keyboard key is in the specified state or not.</returns>
		bool GetKeyboardButtonState(const char keyToTest, InputState whichState) const;

		/// <summary>
		/// Gets whether a mouse button is in the specified state.
		/// </summary>
		/// <param name="whichPlayer">Which player to check for. See Players enumeration.</param>
		/// <param name="whichButton">Which mouse button to check for. See MouseButtons enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the mouse button is in the specified state or not.</returns>
		bool GetMouseButtonState(int whichPlayer, int whichButton, InputState whichState) const;

		/// <summary>
		/// Gets whether a joystick button is in the specified state.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichButton">Which joystick button to check for. See JoyButtons enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the joystick button is in the specified state or not.</returns>
		bool GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const;

		/// <summary>
		/// Gets whether a joystick axis direction is in the specified state or not.
		/// </summary>
		/// <param name="whichJoy">Which joystick to check for.</param>
		/// <param name="whichStick">Which joystick stick to check for.</param>
		/// <param name="whichAxis">Which joystick stick axis to check for.</param>
		/// <param name="whichDir">Which direction to check for. See JoyDirections enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the joystick stick axis is in the specified state or not.</returns>
		bool GetJoystickDirectionState(int whichJoy, int whichStick, int whichAxis, int whichDir, InputState whichState) const;

		/// <summary>
		/// Sets an input element of a player to the specified state during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for. See Players enumeration.</param>
		/// <param name="element">Which element to set. See InputElements enumeration.</param>
		/// <param name="whichState">Which input state to set. See InputState enumeration.</param>
		/// <param name="newState">The new state of the specified InputState. True or false.</param>
		void SetNetworkInputElementState(int player, int element, InputState whichState, bool newState) {
			if (element >= InputElements::INPUT_L_UP && element < InputElements::INPUT_COUNT && player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
				m_NetworkInputElementState[player][element][whichState] = newState;
			}
		}

		/// <summary>
		/// Sets a mouse button for a player to the specified state during network multiplayer.
		/// </summary>
		/// <param name="player">Which player to set for. See Players enumeration.</param>
		/// <param name="whichButton">Which mouse button to set for. See MouseButtons enumeration.</param>
		/// <param name="whichState">Which input state to set. See InputState enumeration.</param>
		/// <param name="newState">The new state of the specified InputState. True or false.</param>
		void SetNetworkMouseButtonState(int player, int whichButton, InputState whichState, bool newState) {
			if (whichButton >= MouseButtons::MOUSE_LEFT && whichButton < MouseButtons::MAX_MOUSE_BUTTONS && player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
				m_NetworkMouseButtonState[player][whichButton][whichState] = newState;
			}
		}

		/// <summary>
		/// Gets whether an input element is in the specified state during network multiplayer.
		/// </summary>
		/// <param name="element">Which element to check for. See InputElements enumeration.</param>
		/// <param name="whichState">Which state to check for. See InputState enumeration.</param>
		/// <returns>Whether the element is in the specified state or not.</returns>
		bool NetworkAccumulatedElementState(int element, InputState whichState) const {
			return (element < InputElements::INPUT_L_UP || element >= InputElements::INPUT_COUNT) ? false : m_NetworkAccumulatedElementState[element][whichState];
		}
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Capture and handle special key shortcuts and combinations. This is called from Update().
		/// </summary>
		void HandleSpecialInput();

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
		UInputMan(const UInputMan &reference) = delete;
		UInputMan & operator=(const UInputMan &rhs) = delete;
	};
}
#endif
