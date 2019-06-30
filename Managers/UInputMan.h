#ifndef _RTEUINPUTMAN_
#define _RTEUINPUTMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            UInputMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the UInputMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DDTTools.h"
#include "Singleton.h"
#define g_UInputMan UInputMan::Instance()
#include "Serializable.h"
#include "Timer.h"

#include "FrameMan.h"
//#include "SceneMan.h"
//#include "MovableMan.h"

namespace RTE
{

class GUIInput;
class Icon;
	
//////////////////////////////////////////////////////////////////////////////////////////
// Class:           UInputMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager over the user input.
// Parent(s):       Singleton, Serializable.
// Class history:   12/26/2001 UInputMan created.

class UInputMan:
    public Singleton<UInputMan>,
    public Serializable
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


enum Players
{
    PLAYER_NONE = -1,
    PLAYER_ONE = 0,
    PLAYER_TWO,
    PLAYER_THREE,
    PLAYER_FOUR,
    MAX_PLAYERS
};

enum InputDevice
{
    DEVICE_KEYB_ONLY = 0,
    DEVICE_MOUSE_KEYB,
    DEVICE_GAMEPAD_1,
    DEVICE_GAMEPAD_2,
    DEVICE_GAMEPAD_3,
    DEVICE_GAMEPAD_4,
    DEVICE_COUNT
};

enum InputPreset
{
    PRESET_NONE = 0,
    PRESET_WASDKEYS,
    PRESET_CURSORKEYS,
    PRESET_XBOX360,
    PRESET_COUNT
    
};

enum InputElements
{
    INPUT_L_UP = 0,
    INPUT_L_DOWN,
    INPUT_L_LEFT,
    INPUT_L_RIGHT,
    INPUT_R_UP,
    INPUT_R_DOWN,
    INPUT_R_LEFT,
    INPUT_R_RIGHT,
    INPUT_FIRE,
    INPUT_AIM,
    INPUT_AIM_UP,
    INPUT_AIM_DOWN,
    INPUT_AIM_LEFT,
    INPUT_AIM_RIGHT,
    INPUT_PIEMENU,
    INPUT_JUMP,
    INPUT_CROUCH,
    INPUT_NEXT,
    INPUT_PREV,
    INPUT_START,
    INPUT_BACK,
	INPUT_WEAPON_CHANGE_NEXT,
	INPUT_WEAPON_CHANGE_PREV,
	INPUT_WEAPON_PICKUP,
	INPUT_WEAPON_DROP,
	INPUT_WEAPON_RELOAD,
    INPUT_COUNT
};

enum MouseButtons
{
    MOUSE_NONE = -1,
    MOUSE_LEFT = 0,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MAX_MOUSE_BUTTONS
};

enum JoyButtons
{
    JOY_NONE = -1,
    JOY_1 = 0,
    JOY_2,
    JOY_3,
    JOY_4,
    JOY_5,
    JOY_6,
    JOY_7,
    JOY_8,
    JOY_9,
    JOY_10,
    JOY_11,
    JOY_12,
    MAX_JOY_BUTTONS
};

enum JoyDirections
{
    JOYDIR_ONE = 0,
    JOYDIR_TWO
};

enum MenuCursorButtons
{
    MENU_PRIMARY = 0,
    MENU_SECONDARY,
    MENU_EITHER
};

// Specifies joystick dead zone type square or circle. 
// Square deadzone cuts-off any input from every axis separately. For example if x-axis has less than 20% input and y-axis has more, x-axis input is ignored.
// Circle uses a round zone to capture stick position on both axis then cut-off if this position is inside the round dead zone.
enum DeadZoneType
{
	CIRCLE = 0,
	SQUARE = 1
};

/*
struct JoyDirMap
{
    // Whether this mapping is active or not
    bool active;
    // Which stick on the joystick
    int stick;
    // Which axis
    int axis;
    // Which direction, on that axis, JOYDIR_ONE or JOYDIR_TWO
    int direction;

    JoyDirMap() { active = false; stick = axis = direction = 0; }
};
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable, method and friend function declarations

private:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    InputScheme
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     A complete input configuration scheme description for a single player.
    // Parent(s):       Serializable.
    // Class history:   1/12/2007 InputScheme created.

    class InputScheme:
        public Serializable
    {

    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

        //////////////////////////////////////////////////////////////////////////////////////////
        // Nested class:    InputMapping
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     A map between an input element and specific input device elements.
        // Parent(s):       Serializable.
        // Class history:   11/25/2006 InputMapping created.
        //                  1/12/2007 Nested inside InputScheme

        class InputMapping:
            public Serializable
        {


        //////////////////////////////////////////////////////////////////////////////////////////
        // Public member variable, method and friend function declarations

        public:


        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual method:  Reset
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Resets the entire Serializable, including its inherited members, to their
        //                  default settings or values.
        // Arguments:       None.
        // Return value:    None.

            virtual void Reset() { Clear(); }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          Clear
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Clears all the member variables of this InputMapping, effectively
        //                  resetting the members of this abstraction level only.
        // Arguments:       None.
        // Return value:    None.

            void Clear();


        //////////////////////////////////////////////////////////////////////////////////////////
        // Constructor:     InputMapping
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Constructor method used to instantiate a InputMapping object in system
        //                  memory. Create() should be called before using the object.
        // Arguments:       None.

            InputMapping() { Clear(); }

/*
        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual method:  Create
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Makes the InputMapping object ready for use.
        // Arguments:       None.
        // Return value:    An error return value signaling sucess or any particular failure.
        //                  Anything below 0 is an error signal.

            virtual int Create();
*/

        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          Create
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Creates a InputMapping to be identical to another, by deep copy.
        // Arguments:       A reference to the InputMapping to deep copy.
        // Return value:    An error return value signaling sucess or any particular failure.
        //                  Anything below 0 is an error signal.

            int Create(const InputMapping &reference);


        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual method:  ReadProperty
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Reads a property value from a Reader stream. If the name isn't
        //                  recognized by this class, then ReadProperty of the parent class
        //                  is called. If the property isn't recognized by any of the base classes,
        //                  false is returned, and the Reader's position is untouched.
        // Arguments:       The name of the property to be read.
        //                  A Reader lined up to the value of the property to be read.
        // Return value:    An error return value signaling whether the property was successfully
        //                  read or not. 0 means it was read successfully, and any nonzero indicates
        //                  that a property of that name could not be found in this or base classes.

            virtual int ReadProperty(std::string propName, Reader &reader);


        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual method:  Save
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Saves the complete state of this InputMapping to an output stream for
        //                  later recreation with Create(Reader &reader);
        // Arguments:       A Writer that the InputMapping will save itself with.
        // Return value:    An error return value signaling sucess or any particular failure.
        //                  Anything below 0 is an error signal.

            virtual int Save(Writer &writer) const;


        //////////////////////////////////////////////////////////////////////////////////////////
        // Virtual method:  GetClassName
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the class name of this Entity.
        // Arguments:       None.
        // Return value:    A string with the friendly-formatted type name of this object.

            virtual const std::string & GetClassName() const { return m_sClassName; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          SetKey
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Sets the keyboard key this mapped to.
        // Arguments:       The scancode of the new key to map to.
        // Return value:    None.

            void SetKey(int newKey) { m_KeyMap = newKey; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          SetMouseButton
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Sets the mouse button mapping.
        // Arguments:       The number of the mouse button this should be mapped to.
        // Return value:    None.

            void SetMouseButton(int newButton) { m_MouseButtonMap = newButton; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          SetJoyButton
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Sets the joystick button mapping.
        // Arguments:       The number of the joystick button this should be mapped to.
        // Return value:    None.

            void SetJoyButton(int newButton) { m_JoyButtonMap = newButton; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          SetDirection
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Sets the joystick direction mapping.
        // Arguments:       The stick, axis, and direction to map this to.
        // Return value:    None.

            void SetDirection(int newStick, int newAxis, int newDirection) { m_DirectionMapped = true; m_StickMap = newStick; m_AxisMap = newAxis; m_DirectionMap = newDirection; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          SetPresetDesc
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Sets the description of the input scheme preset that this element is part of,
        //                  if any preset has been set for this element's scheme.
        // Arguments:       The desc associated with this element by the scheme preset, if any
        //                  has been set. This string should be empty otherwise.
        // Return value:    None.

            void SetPresetDesc(std::string presetDesc) { m_PresetDesc = presetDesc; }

			
        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetKey
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the keyboard key mapping.
        // Arguments:       None.
        // Return value:    The keyboard key this is mapped to.

            int GetKey() const { return m_KeyMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetMouseButton
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the mouse button mapping.
        // Arguments:       None.
        // Return value:    The number of the mouse button this is mapped to.

            int GetMouseButton() const { return m_MouseButtonMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetJoyButton
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the joystick button mapping.
        // Arguments:       None.
        // Return value:    The number of the joystick button this is mapped to.

            int GetJoyButton() const { return m_JoyButtonMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          JoyDirMapped
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Shows whether this is mapped to a joy direciton or not.
        // Arguments:       None.
        // Return value:    Joy direction mapped or not.

            bool JoyDirMapped() const { return m_DirectionMapped; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetStick
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the joystick stick number that this is mapped to.
        // Arguments:       None.
        // Return value:    The joystick stick number.

            int GetStick() const { return m_StickMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetAxis
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the joystick axis number that this is mapped to.
        // Arguments:       None.
        // Return value:    The joystick axis number.

            int GetAxis() const { return m_AxisMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetStick
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the joystick stick number that this is mapped to.
        // Arguments:       None.
        // Return value:    The direction, UInputMan::JOYDIR_ONE or UInputMan::JOYDIR_TWO.

            int GetDirection() const { return m_DirectionMap; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          GetPresetDesc
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Gets the description of the input scheme preset that this element is part of,
        //                  if any preset has been set for this element's scheme.
        // Arguments:       None.
        // Return value:    The desc associated with this element by the scheme preset, if any
        //                  has been set. This string is empty otherwise.

        std::string GetPresetDesc() const { return m_PresetDesc; }

        //////////////////////////////////////////////////////////////////////////////////////////
        // Protected member variable and method declarations

        protected:

            // Member variables
            static const std::string m_sClassName;
            // The keyboard key mapping
            int m_KeyMap;
            // The mouse button mapping
            int m_MouseButtonMap;
            // The joystick button mapping
            int m_JoyButtonMap;
            // Whether joystick direction mapping is enabled
            bool m_DirectionMapped;
            // The joystick stick mapping, if any
            int m_StickMap;
            // The joystick axis mapping
            int m_AxisMap;
            // The joystick direction mapping
            int m_DirectionMap;
            // The friendly description that is associated with the scheme preset element, if any is set
            std::string m_PresetDesc;

        };


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        virtual void Reset() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this InputScheme, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     InputScheme
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a InputScheme object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        InputScheme() { Clear(); }

/*
    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the InputScheme object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create();
*/

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a InputScheme to be identical to another, by deep copy.
    // Arguments:       A reference to the InputScheme to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        int Create(const InputScheme &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  ReadProperty
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Reads a property value from a Reader stream. If the name isn't
    //                  recognized by this class, then ReadProperty of the parent class
    //                  is called. If the property isn't recognized by any of the base classes,
    //                  false is returned, and the Reader's position is untouched.
    // Arguments:       The name of the property to be read.
    //                  A Reader lined up to the value of the property to be read.
    // Return value:    An error return value signaling whether the property was successfully
    //                  read or not. 0 means it was read successfully, and any nonzero indicates
    //                  that a property of that name could not be found in this or base classes.

        virtual int ReadProperty(std::string propName, Reader &reader);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Save
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Saves the complete state of this InputScheme to an output stream for
    //                  later recreation with Create(Reader &reader);
    // Arguments:       A Writer that the InputScheme will save itself with.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Save(Writer &writer) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetClassName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the class name of this Entity.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted type name of this object.

        virtual const std::string & GetClassName() const { return m_sClassName; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetDevice
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets the device this scheme is supposed to use.
    // Arguments:       The number of the device this scheme should use. See InputDevice enum
    // Return value:    None.

        void SetDevice(int activeDevice = 0) { m_ActiveDevice = activeDevice; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetDevice
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the number of the device that this scheme is using.
    // Arguments:       None.
    // Return value:    The device number of this scheme. See InputDevice enum

        int GetDevice() const { return m_ActiveDevice; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetPreset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets up a specific preset scheme that is sensible and recommended.
    // Arguments:       The preset number to set the scheme to match. See InputPreset enum
    // Return value:    None.

        void SetPreset(int schemePreset = 0);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetPreset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the number of the last preset that this was set to.
    // Arguments:       None.
    // Return value:    The last preset number set of this scheme. See InputPreset enum

        int GetPreset() const { return m_SchemePreset; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetInputMappings
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the InputMapping:s for this.
    // Arguments:       None.
    // Return value:    The input mappings array, which is INPUT_COUNT large.

        InputMapping * GetInputMappings() { return m_aInputMapping; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetupDefaults
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets some sensible default bindings for this.
    // Arguments:       None.
    // Return value:    None.

        void SetupDefaults();


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetJoystickDeadzone
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Get the deadzone value for this control scheme.
	// Arguments:       None.
	// Return value:    Joystick dead zone from 0.0 to 1.0.

	float GetJoystickDeadzone()
	{
		return m_JoystickDeadzone;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetJoystickDeadzone
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     set the deadzone value for this control scheme.
	// Arguments:       Joystick dead zone from 0.0 to 1.0.
	// Return value:    None.

	void SetJoystickDeadzone(float val)
	{
		m_JoystickDeadzone = val;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetJoystickDeadzoneType
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Get the deadzone type for this control scheme.
	// Arguments:       None.
	// Return value:    Joystick dead zone InputScheme::DeadZoneType::CIRCLE or InputScheme::DeadZoneType::SQUARE

	int GetJoystickDeadzoneType()
	{
		return m_JoystickDeadzoneType;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetJoystickDeadzoneType
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Set the deadzone type for this control scheme.
	// Arguments:       Joystick dead zone InputScheme::DeadZoneType::CIRCLE or InputScheme::DeadZoneType::SQUARE
	// Return value:    None.

	void SetJoystickDeadzoneType(int val)
	{
		m_JoystickDeadzoneType = val;
	}





    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // Member variables
        static const std::string m_sClassName;
        // The currently active device for this scheme 
        int m_ActiveDevice;
        // The preset this scheme was last set to, if any
        int m_SchemePreset;
		// How much of the input to treat like a deadzone input, not registered by the game
		float m_JoystickDeadzone;
		// Which deadzone type is used
		int m_JoystickDeadzoneType;
        // The device input element mappings
        InputMapping m_aInputMapping[INPUT_COUNT];

    };


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     UInputMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a UInputMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    UInputMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~UInputMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a UInputMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~UInputMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the UInputMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire UInputMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this UInputMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the UInputMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the UInputMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReInitKeyboard
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Re-initalizes the keyboard for when windows regains focus. This is
//                  really used to work around an Allegro bug.
// Arguments:       None.
// Return value:    None.

    void ReInitKeyboard();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetInputClass
//////////////////////////////////////////////////////////////////////////////////////////
//
// 
//
	void SetInputClass(GUIInput* pInputClass);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlScheme
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Access a specific players' control scheme.
// Arguments:       Which player to get the scheme for.
// Return value:    A pointer to the requested player's control scheme. Ownership is NOT
//                  transferred!

	InputScheme * GetControlScheme(int whichPlayer) { if (m_OverrideInput) return &m_aControlScheme[0]; else return &(m_aControlScheme[whichPlayer]); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSchemeIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the current device Icon of a specific player's scheme
// Arguments:       Which player to get the scheme device icon of.
// Return value:    A const pointer to the requested player's control scheme icon.
//                  Ownership is NOT transferred!

    const Icon * GetSchemeIcon(int whichPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeviceIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the current device Icon of a specific device
// Arguments:       Which device to get the icon of.
// Return value:    A const pointer to the requested device's control scheme icon.
//                  Ownership is NOT transferred!

	const Icon * GetDeviceIcon(int whichDevice);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears a all mappings for a specific input element of a specific player.
// Arguments:       Which player to affect.
//                  Which input element to clear all mappings of.
// Return value:    None.

    void ClearMapping(int whichPlayer, int whichInput) { m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKeyMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which keyboard key is mapped to a specific input element.
// Arguments:       Which player to look up.
//                  Which input element to look up.
// Return value:    Which keyboard key is mapped to the specified player and element.

    int GetKeyMapping(int whichPlayer, int whichInput) { return m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].GetKey(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetButtonMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which joystick button is mapped to a specific input element.
// Arguments:       Which player to look up.
//                  Which input element to look up.
// Return value:    Which joystick button is mapped to the specified player and element.

    int GetButtonMapping(int whichPlayer, int whichInput) { return m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].GetJoyButton(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMappingName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the key/mouse/joy button/key/direction that a
//                  particular input element is mapped to.
// Arguments:       Which player to look up.
//                  Which input element to look up.
// Return value:    A string with the appropriate clear text description of the mapped thing.

    std::string GetMappingName(int whichPlayer, int whichElement);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetKeyMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a keyboard key mapped to a specific input element.
// Arguments:       Which player to do this for.
//                  Which input element to map to.
//                  The scancode of which keyboard key to map to above input element.
// Return value:    None.

    void SetKeyMapping(int whichPlayer, int whichInput, int whichKey) { m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].SetKey(whichKey); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DisableKeys
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     For temporary disabling of most keyboard keys for when typing into a
//                  dialog is required for example.
// Arguments:       Whether to disable most keys or not.
// Return value:    None.

    void DisableKeys(bool disable = true) { m_DisableKeyboard = disable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetButtonMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a joystick button mapped to a specific input element.
// Arguments:       Which player to do this for.
//                  Which input element to map to.
//                  Which joystick button to map to above input element.
// Return value:    None.

    void SetButtonMapping(int whichPlayer, int whichInput, int whichButton) { m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].SetJoyButton(whichButton); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureKeyMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any key press this frame, and creates an input mapping
//                  for a specific player accordingly.
// Arguments:       Which player to do create a map for.
//                  Which input element to map to for that player.
// Return value:    Whether there were any key presses this frame, and therefore whether
//                  a mapping was successfully captured or not.

    bool CaptureKeyMapping(int whichPlayer, int whichInput);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureButtonMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any button press this frame, and creates an input mapping
//                  for a specific player accordingly.
// Arguments:       Which player to do create a map for.
//                  Which joystick to scan for button presses.
//                  Which input element to map to for that player.
// Return value:    Whether there were any button presses this frame, and therefore whether
//                  a mapping was successfully captured or not.

    bool CaptureButtonMapping(int whichPlayer, int whichJoy, int whichInput);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureDirectionMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any joystick pad or stick direction press this frame,
//                  and creates an input mapping for a specific player accordingly.
// Arguments:       Which player to do create a map for.
//                  Which joystick to scan for pad and stick direction presses.
//                  Which input element to map to for that player.
// Return value:    Whether there were any direction presses this frame, and therefore whether
//                  a mapping was successfully captured or not.

    bool CaptureDirectionMapping(int whichPlayer, int whichJoy, int whichInput);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureJoystickMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any button or direction press this frame, and creates
//                  an input mapping for a specific player accordingly.
// Arguments:       Which player to do create a map for.
//                  Which joystick to scan for button and stick presses.
//                  Which input element to map to for that player.
// Return value:    Whether there were any button or stick presses this frame, and
//                  therefore whether a mapping was successfully captured or not.

    bool CaptureJoystickMapping(int whichPlayer, int whichJoy, int whichInput);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was depressed between the last
//                  update and the one previous to it.
// Arguments:       Which player to check, and which element of that player.
// Return value:    Pressed or not.

    bool ElementPressed(int whichPlayer, int whichElement);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was released between the last
//                  update and the one previous to it.
// Arguments:       Which player to check, and which element of that player.
// Return value:    Pressed or not.

    bool ElementReleased(int whichPlayer, int whichElement);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was held during the last update.
// Arguments:       Which player to check, and which element of that player.
// Return value:    Pressed or not.

    bool ElementHeld(int whichPlayer, int whichElement);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key was depressed between the last update and the one
//                  previous to it.
// Arguments:       A const char with the Allegro-defined key enumeration to test.
// Return value:    Pressed or not.

    bool KeyPressed(const char keyToTest);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key was released between the last update and the one
//                  previous to it.
// Arguments:       A const char with the Allegro-defined key enumeration to test.
// Return value:    Released or not.

    bool KeyReleased(const char keyToTest);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key is being held right now.
// Arguments:       A const char with the Allegro-defined key enumeration to test.
// Return value:    Held or not.

    bool KeyHeld(const char keyToTest);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichKeyHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the scancode of the keyboard key which is currently down.
// Arguments:       None.
// Return value:    The scancode (KEY_SPACE, etc) of the first keyboard key in the keyboard
//                  buffer. 0 means none.

    int WhichKeyHeld();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mosue button was depressed between the last update and
//                  the one previous to it.
// Arguments:       Which button. Which network player pressed the button.
// Return value:    Pressed or not.

    bool MouseButtonPressed(int whichButton, int whichPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mosue button was depressed between the last update and
//                  the one previous to it.
// Arguments:       Which button.
// Return value:    Pressed or not.

	bool MouseButtonPressed(int whichButton) { return MouseButtonPressed(whichButton, 0); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mosue button was released between the last update and
//                  the one previous to it.
// Arguments:       Which button. Which network player released the button.
// Return value:    Released or not.

    bool MouseButtonReleased(int whichButton, int whichPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mosue button was released between the last update and
//                  the one previous to it.
// Arguments:       Which button.
// Return value:    Released or not.

	bool MouseButtonReleased(int whichButton) { return MouseButtonReleased(whichButton, 0); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mouse button is being held down right now.
// Arguments:       Which button. Which network player holds the button.
// Return value:    Held or not.

    bool MouseButtonHeld(int whichButton, int whichPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     (ONLY FOR LUA BACKWARD COMPATIBILITY) Shows whether a mouse button is being held down right now.
// Arguments:       Which button.
// Return value:    Held or not.

	bool MouseButtonHeld(int whichButton) { return MouseButtonHeld(whichButton, 0); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseWheelMoved
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a the mouse wheel has been moved past the threshold
//                  limit in either direction this frame.
// Arguments:       None.
// Return value:    The direction the mouse wheel has been moved which is past tht threshold
//                  0 means not past, negative means moved down, positive means moved up.

    int MouseWheelMoved() { return m_MouseWheelChange; }


	int MouseWheelMovedByPlayer(int player)
	{
		if (m_OverrideInput && player >= 0 && player < MAX_PLAYERS)
		{
			return m_aNetworkMouseWheelState[player];
		}
		return m_MouseWheelChange;
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joy button was depressed between the last update and
//                  the one previous to it.
// Arguments:       Which joystick to check, and which button on that joystick.
// Return value:    Pressed or not.

    bool JoyButtonPressed(int whichJoy, int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joy button was released between the last update and
//                  the one previous to it.
// Arguments:       Which joystick to check, and which button on that joystick.
// Return value:    Released or not.

    bool JoyButtonReleased(int whichJoy, int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick button is being held down right now.
// Arguments:       Which joystick to check, and which button on that joystick.
// Return value:    Held or not.

    bool JoyButtonHeld(int whichJoy, int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichJoyButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the first joystick button which is currently down.
// Arguments:       Which joystick to check.
// Return value:    The first button in the sequence of button enumerations (JOY_1, JOY_2,
//                  JOY_3) that is held at the time of calling this. JOY_NONE means none.

    int WhichJoyButtonHeld(int whichJoy);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichJoyButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the first joystick button which was pressed down since last frame.
// Arguments:       Which joystick to check.
// Return value:    The first button in the sequence of button enumerations (JOY_1, JOY_2,
//                  JOY_3) that is pressed since the previous frame. JOY_NONE means none.

    int WhichJoyButtonPressed(int whichJoy);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick axis direction was depressed between the
//                  last update and the one previous to it.
// Arguments:       Which joystick, stick, axis and direction to check.
// Return value:    Pressed or not.

    bool JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick d-pad direction was released between the
//                  last update and the one previous to it.
// Arguments:       Which joystick, stick, axis and direction to check.
// Return value:    Released or not.

    bool JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick axis is being held down in a specific
//                  direction right now. Two adjacent directions can be held down to produce
//                  diagonals.
// Arguments:       Which joystick, stick, axis and direction to check.
// Return value:    Held or not.

    bool JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogMoveValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog moving values of a specific player's control scheme.
// Arguments:       Which player to get em for.
// Return value:    The analog axis values ranging between -1.0 to 1.0, in both axes.

    Vector AnalogMoveValues(int whichPlayer = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogAimValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog aiming values of a specific player's control scheme.
// Arguments:       Which player to get em for.
// Return value:    The analog axis values ranging between -1.0 to 1.0, in both axes.

    Vector AnalogAimValues(int whichPlayer = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMouseValueMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set the mouse's analog emualtion output to be of a specific normalized
//                  magnitude.
// Arguments:       The normalized magnitude, between 0 and 1.0.
// Return value:    None.

    void SetMouseValueMagnitude(float magCap) { m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius * magCap); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogAxisValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the normalized value of a certain joystick's stick's axis.
// Arguments:       Which joystick, stick and axis to check.
// Return value:    The analog axis value ranging between -1.0 to 1.0, or 0.0 to 1.0 if
//                  it's a throttle type control.

    float AnalogAxisValue(int whichJoy = 0, int whichStick = 0, int whichAxis = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogStickValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog values of a certain joystick device stick.
// Arguments:       Which joystick to check, and which stick on that joystick device.
// Return value:    The analog axis values ranging between -1.0 to 1.0.

    Vector AnalogStickValues(int whichJoy = 0, int whichStick = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseUsedByPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports if any, and which player is using the mouse for control at
//                  this time.
// Arguments:       None.
// Return value:    Which player is using the mouse. If noone is, then -1 is returned;

    int MouseUsedByPlayer() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyMouseButtonPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any mouse button presses at all
// Arguments:       None.
// Return value:    Whether any mouse buttons have been pressed at all since last frame

    bool AnyMouseButtonPress();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrapMousePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the mouse to be trapped in the middle of the screen, so it doesn't
//                  go out and click on other windows etc. This is usually used when the
//                  cursor is invisible and only relative mouse movements are used.
// Arguments:       Whether to trap the mouse or not.
//                  Which player is trying ot control the mouse. Only the player with
//                  actual control over the mouse will affect its trapping here. -1 means
//                  change mouse trapping regardless of player.
// Return value:    None.

	void TrapMousePos(bool trap = true, int whichPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseMovement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the relative movement of the mouse since last update. Only returns
//                  true if the selected player actuall is using the mouse.
// Arguments:       Which player to get em for. If the player doesn't use the mouse, then
//                  this always returns a zero vector.
// Return value:    The relative mouse movements, in both axes.

    Vector GetMouseMovement(int whichPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DisableMouseMoving
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Will temporarily disable positioniong of the mouse. This is so that
//                  when focus is swtiched back to the game window, it avoids having the
//                  window fly away because the user clicked the title bar of the window.
// Arguments:       Whether to disable or not.
// Return value:    None.

    void DisableMouseMoving(bool disable = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMousePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the absolute screen position of the mouse cursor.
// Arguments:       Where to place the mouse.
//                  Which player is trying ot control the mouse. Only the player with
//                  actual control over the mouse will affect its trapping here. -1 means
//                  do it regardless of player.
// Return value:    None.

    void SetMousePos(Vector &newPos, int whichPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceMouseWithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the mouse within a box on the screen.
// Arguments:       The top left corner of the screen box to keep the mouse within, in
//                  screen coodinates.
//                  The dimensions of the box, width/height.
//                  Which player is trying ot control the mouse. Only the player with
//                  actual control over the mouse will affect its trapping here. -1 means
//                  do it regardless of player.
// Return value:    None.

    void ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceMouseWithinPlayerScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the mouse within a specific player's screen area.
// Arguments:       Which player's screen's are to constrain the mouse to. Only the player
//                  with actual control over the mouse will affect its trapping here.
// Return value:    None.

    void ForceMouseWithinPlayerScreen(int whichPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick input at all, buttons or d-pad.
// Arguments:       None.
// Return value:    Whether any buttons of pads are pressed at all.

    bool AnyJoyInput();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick presses at all, buttons or d-pad.
// Arguments:       None.
// Return value:    Whether any buttons or pads have been pressed at all since last frame.

    bool AnyJoyPress();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyButtonPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick button presses at all, but not
//                  d-pad input, for a specific joystick.
// Arguments:       None.
// Return value:    Whether any joy buttons have been pressed at all since last frame, of
//                  a specific joystick.

    bool AnyJoyButtonPress(int whichJoy);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMenuDirectional
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the generic direction input from any and all players which can
//                  affect a shared menu cursor. Normalized to 1.0 max
// Arguments:       None.
// Return value:    The vector with the directional input from any or all players.

    Vector GetMenuDirectional();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor was pressed
//                  between previous update and this.
// Arguments:       Which generic menu cursor button to check for.
// Return value:    Pressed or not.

    bool MenuButtonPressed(int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor is held down.
// Arguments:       Which generic menu cursor button to check for.
// Return value:    Released or not.

    bool MenuButtonReleased(int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor is held down.
// Arguments:       Which generic menu cursor button to check for.
// Return value:    Held or not.

    bool MenuButtonHeld(int whichButton);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any input at all, keyboard or buttons or d-pad.
// Arguments:       None.
// Return value:    Whether any buttons of pads are pressed at all.

    bool AnyInput();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any key, button, or d-pad presses at all.
//                  MUST call Update before calling this for it to work properly!
// Arguments:       None.
// Return value:    Whether any buttons of pads have been pressed at all since last frame.

    bool AnyPress();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyStartPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any start key/button presses at all.
//                  MUST call Update before calling this for it to work properly!
// Arguments:       None.
// Return value:    Whether any start buttons or keys have been pressed at all since last frame.

    bool AnyStartPress();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WaitForSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Halts thread and waits for space to be pressed. This is for debug
//                  purposes mostly.
// Arguments:       None.
// Return value:    None.

    void WaitForSpace();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this UInputMan. Supposed to be done every frame.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMouseSensitivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets mouse sensitivity
// Arguments:       New sensitivty value.
// Return value:    None.

	void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseSensitivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets mouse sensitivity
// Arguments:       None.
// Return value:    Returns current mouse sensitivity.

	float GetMouseSensitivity() const { return m_MouseSensitivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLastDeviceWhichControlledGUICursor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns last device which affected GUI cursor position.
// Arguments:       None.
// Return value:    Returns last device which affected GUI cursor position.

	int GetLastDeviceWhichControlledGUICursor() const { return m_LastDeviceWhichControlledGUICursor; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoystickActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified joystick is active
// Arguments:       None.
// Return value:    Returns true if specified joystick is active

	bool JoystickActive(int joystickNumber) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJoystickCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of active joysticks
// Arguments:       None.
// Return value:    Returns the number of active joysticks

	int GetJoystickCount() const;

	Vector GetNetworkAccumulatedRawMouseMovement(int player);

	void SetNetworkMouseInput(int player, Vector input);

	void SetNetworkInputElementHeldState(int player, int element, bool state);

	void SetNetworkInputElementPressedState(int player, int element, bool state);

	void SetNetworkInputElementReleasedState(int player, int element, bool state);

	void SetNetworkMouseButtonPressedState(int player, int whichButton, bool state);

	void SetNetworkMouseButtonReleasedState(int player, int whichButton, bool state);

	void SetNetworkMouseButtonHeldState(int player, int whichButton, bool state);

	void SetNetworkMouseWheelState(int player, int state);

	bool OverrideInput() { return m_OverrideInput; };

	void ClearAccumulatedStates();

	bool AccumulatedElementPressed(int element);

	bool AccumulatedElementReleased(int element);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlagAltState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the state of the Alt key.
// Arguments:       None.
// Return value:    Returns the state of the Alt key.

	bool FlagAltState() const { return (key_shifts & KB_ALT_FLAG) > 0 ? true : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlagCtrlState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the state of the Ctrl key.
// Arguments:       None.
// Return value:    Returns the state of the Ctrl key.

	bool FlagCtrlState() const { return (key_shifts & KB_CTRL_FLAG) > 0 ? true : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlagShiftState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the state of the Shift key.
// Arguments:       None.
// Return value:    Returns the state of the Shift key.

	bool FlagShiftState() const { return (key_shifts & KB_SHIFT_FLAG) > 0 ? true : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMultiplayerMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this instance is in multiplayer mode or not
// Arguments:       Whether multiplayer mode is on.
// Return value:    None.

	void SetMultiplayerMode(bool isMultiplayer) { m_IsInMultiplayerMode = isMultiplayer; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

    bool m_DebugArmed;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this UInputMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Key states of the previous update
    static char *s_aLastKeys;
    // Key states that have changed
    static char *s_aChangedKeys;
	// Current input class if available
	static GUIInput* s_InputClass; 
	
    // Temporarily disable all keyboard input reading
    bool m_DisableKeyboard;

    // Which control scheme is being used by each player
    InputScheme m_aControlScheme[MAX_PLAYERS];

    // The Icons representing all different devices
    const Icon *m_apDeviceIcons[DEVICE_COUNT];

    // The raw absolute movement of the mouse between the last two Updates
    Vector m_RawMouseMovement;
    // The emulated analog stick position of the mouse
    Vector m_AnalogMouseData;
    // Whetehr the mouse is trapped in the middle of the screen each update or not.
    bool m_TrapMousePos;
    // The radius (in pixels) of the circle trapping the mouse for analog mouse data
    float m_MouseTrapRadius;
    // The relative mouse wheel pos since last reset of it
    int m_MouseWheelChange;
    // Temporary disable for positioniong the mouse, for when the game window is not in focus
    bool m_DisableMouseMoving;
    // This is set when focus is swtiched back to the game window, an will cause the m_DisableMouseMoving to switch to
    // false when the mouse button is RELEASED. This is to avoid having the window fly away because the user clicked the title bar.
    bool m_PrepareToEnableMouseMoving;
	// Mouse sensitivity, to replace hardcoded 0.6 value in Update
	float m_MouseSensitivity;

	// Indicates which device controlled the cursor last time
	int m_LastDeviceWhichControlledGUICursor;

    // Mouse button states
    static bool m_aMouseButtonState[MAX_MOUSE_BUTTONS];
    static bool m_aMousePrevButtonState[MAX_MOUSE_BUTTONS];
    static bool m_aMouseChangedButtonState[MAX_MOUSE_BUTTONS];

    // Joystick states as they were the previous update
    static JOYSTICK_INFO s_aaPrevJoyState[MAX_PLAYERS];
    // Joystick states that have changed
    static JOYSTICK_INFO s_aaChangedJoyState[MAX_PLAYERS];
	// Input of this manager is overriden by netowrk input
	bool m_OverrideInput;
	// If true then this instance operates in multiplayer mode
	bool m_IsInMultiplayerMode;

	Vector m_NetworkAccumulatedRawMouseMovement[MAX_PLAYERS];

	bool m_aNetworkAccumulatedElementPressed[INPUT_COUNT];
	bool m_aNetworkAccumulatedElementReleased[INPUT_COUNT];

	bool m_aNetworkInputElementPressed[MAX_PLAYERS][INPUT_COUNT];
	bool m_aNetworkInputElementReleased[MAX_PLAYERS][INPUT_COUNT];
	bool m_aNetworkInputElementHeld[MAX_PLAYERS][INPUT_COUNT];

	bool m_aNetworkMouseButtonPressedState[MAX_PLAYERS][MAX_MOUSE_BUTTONS];
	bool m_aNetworkMouseButtonReleasedState[MAX_PLAYERS][MAX_MOUSE_BUTTONS];
	bool m_aNetworkMouseButtonHeldState[MAX_PLAYERS][MAX_MOUSE_BUTTONS];

	Vector m_aNetworkAnalogMoveData[MAX_PLAYERS];

	int m_aNetworkMouseWheelState[MAX_PLAYERS];

	bool m_TrapMousePosPerPlayer[MAX_PLAYERS];

    // Disallow the use of some implicit methods.
    UInputMan(const UInputMan &reference);
    UInputMan & operator=(const UInputMan &rhs);

};

} // namespace RTE

#endif // File