//////////////////////////////////////////////////////////////////////////////////////////
// File:            UInputMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the UInputMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "UInputMan.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "SLTerrain.h"
#include "GUIInput.h"
#include "Icon.h"

extern volatile bool g_Quit;
extern bool g_ResetActivity;
extern bool g_InActivity;
extern int g_IntroState;
//extern int g_TempXOff;
//extern int g_TempYOff;

namespace RTE
{

const string UInputMan::m_ClassName = "UInputMan";
const string UInputMan::InputScheme::m_sClassName = "InputScheme";
const string UInputMan::InputScheme::InputMapping::m_sClassName = "InputMapping";
char *UInputMan::s_aLastKeys = new char[KEY_MAX];
char *UInputMan::s_aChangedKeys = new char[KEY_MAX];
GUIInput* UInputMan::s_InputClass = NULL; 
bool UInputMan::m_aMouseButtonState[MAX_MOUSE_BUTTONS];
bool UInputMan::m_aMousePrevButtonState[MAX_MOUSE_BUTTONS];
bool UInputMan::m_aMouseChangedButtonState[MAX_MOUSE_BUTTONS];
JOYSTICK_INFO UInputMan::s_aaPrevJoyState[MAX_PLAYERS];
JOYSTICK_INFO UInputMan::s_aaChangedJoyState[MAX_PLAYERS];


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this InputMapping, effectively
//                  resetting the members of this abstraction level only.

void UInputMan::InputScheme::InputMapping::Clear()
{
    m_KeyMap = 0;
    m_MouseButtonMap = -1;
    m_JoyButtonMap = -1;
    m_DirectionMapped = false;
    m_StickMap = 0;
    m_AxisMap = 0;
    m_DirectionMap = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the InputMapping object ready for use.

int UInputMan::InputScheme::InputMapping::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a InputMapping to be identical to another, by deep copy.

int UInputMan::InputScheme::InputMapping::Create(const InputMapping &reference)
{
    m_KeyMap = reference.m_KeyMap;
    m_MouseButtonMap = reference.m_MouseButtonMap;
    m_JoyButtonMap = reference.m_JoyButtonMap;
    m_DirectionMapped = reference.m_DirectionMapped;
    m_StickMap = reference.m_StickMap;
    m_AxisMap = reference.m_AxisMap;
    m_DirectionMap = reference.m_DirectionMap;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int UInputMan::InputScheme::InputMapping::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "KeyMap")
    {
        int key;
        reader >> key;
        if (key != 0)
            m_KeyMap = key;
    }
    else if (propName == "MouseButtonMap")
        reader >> m_MouseButtonMap;
    else if (propName == "JoyButtonMap")
        reader >> m_JoyButtonMap;
    else if (propName == "StickMap")
    {
        reader >> m_StickMap;
        m_DirectionMapped = true;
    }
    else if (propName == "AxisMap")
    {
        reader >> m_AxisMap;
        m_DirectionMapped = true;
    }
    else if (propName == "DirectionMap")
    {
        reader >> m_DirectionMap;
        m_DirectionMapped = true;
    }
	else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this InputMapping with a Writer for
//                  later recreation with Create(Reader &reader);

int UInputMan::InputScheme::InputMapping::Save(Writer &writer) const
{
    Serializable::Save(writer);

// Gotto write somehting
//    if (m_KeyMap != 0)
//    {
        writer.NewProperty("KeyMap");
        writer << m_KeyMap;
//    }
    if (m_MouseButtonMap >= 0)
    {
        writer.NewProperty("MouseButtonMap");
        writer << m_MouseButtonMap;
    }
    if (m_JoyButtonMap >= 0)
    {
        writer.NewProperty("JoyButtonMap");
        writer << m_JoyButtonMap;
    }
    // Only save direction map if it's enabled
    if (m_DirectionMapped)
    {
        writer.NewProperty("StickMap");
        writer << m_StickMap;
        writer.NewProperty("AxisMap");
        writer << m_AxisMap;
        writer.NewProperty("DirectionMap");
        writer << m_DirectionMap;
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this InputScheme, effectively
//                  resetting the members of this abstraction level only.

void UInputMan::InputScheme::Clear()
{
    m_ActiveDevice = DEVICE_KEYB_ONLY;
    m_SchemePreset = PRESET_NONE;
	m_JoystickDeadzone = 0.01;
	m_JoystickDeadzoneType = DeadZoneType::CIRCLE;

    // Clear all mappings
    for (int mapping = 0; mapping < INPUT_COUNT; ++mapping)
    {
        m_aInputMapping[mapping].Clear();
    }

    // Set up the default mouse button bindings - do these here becuase there are no config steps for them yet
    m_aInputMapping[INPUT_FIRE].SetMouseButton(MOUSE_LEFT);
    m_aInputMapping[INPUT_PIEMENU].SetMouseButton(MOUSE_RIGHT);
}


/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the InputScheme object ready for use.

int UInputMan::InputScheme::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPreset
//////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets up a specific preset scheme that is sensible and recommended.

void UInputMan::InputScheme::SetPreset(int schemePreset)
{
    m_SchemePreset = schemePreset;

    if (m_SchemePreset == PRESET_XBOX360)
    {
		/*
        // Set up the default xbox 360 button bindings
        m_aInputMapping[INPUT_FIRE].SetJoyButton(JOY_1);
        m_aInputMapping[INPUT_FIRE].SetPresetDesc("A Button");
        // Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping
        m_aInputMapping[INPUT_AIM].SetJoyButton(JOY_3);
        m_aInputMapping[INPUT_AIM].SetPresetDesc("X Button");
        // Pie menu also cancels buy menu, which makes sense for the B button
        m_aInputMapping[INPUT_PIEMENU].SetJoyButton(JOY_2);
        m_aInputMapping[INPUT_PIEMENU].SetPresetDesc("B Button");
        // Jump on top button of diamond makes sense
        m_aInputMapping[INPUT_JUMP].SetJoyButton(JOY_4);
        m_aInputMapping[INPUT_JUMP].SetPresetDesc("Y Button");
        m_aInputMapping[INPUT_NEXT].SetJoyButton(JOY_6);
        m_aInputMapping[INPUT_NEXT].SetPresetDesc("R Bumper Button");
        m_aInputMapping[INPUT_PREV].SetJoyButton(JOY_5);
        m_aInputMapping[INPUT_PREV].SetPresetDesc("L Bumper Button");
        m_aInputMapping[INPUT_START].SetJoyButton(JOY_8);
        m_aInputMapping[INPUT_START].SetPresetDesc("Start Button");
        m_aInputMapping[INPUT_BACK].SetJoyButton(JOY_7);
        m_aInputMapping[INPUT_BACK].SetPresetDesc("Back Button");

        // Set up the default xbox joy direction bindings
        m_aInputMapping[INPUT_L_UP].SetDirection(0, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_L_UP].SetPresetDesc("L Thumbstick Up");
        m_aInputMapping[INPUT_L_DOWN].SetDirection(0, 1, JOYDIR_TWO);
        m_aInputMapping[INPUT_L_DOWN].SetPresetDesc("L Thumbstick Down");
        m_aInputMapping[INPUT_L_LEFT].SetDirection(0, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_L_LEFT].SetPresetDesc("L Thumbstick Left");
        m_aInputMapping[INPUT_L_RIGHT].SetDirection(0, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_L_RIGHT].SetPresetDesc("L Thumbstick Right");
        
		m_aInputMapping[INPUT_R_UP].SetDirection(2, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_R_UP].SetPresetDesc("R Thumbstick Up");
        m_aInputMapping[INPUT_R_DOWN].SetDirection(2, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_R_DOWN].SetPresetDesc("R Thumbstick Down");
        m_aInputMapping[INPUT_R_LEFT].SetDirection(1, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_R_LEFT].SetPresetDesc("R Thumbstick Left");
        m_aInputMapping[INPUT_R_RIGHT].SetDirection(1, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_R_RIGHT].SetPresetDesc("R Thumbstick Right");
        
		m_aInputMapping[INPUT_FIRE].SetDirection(0, 2, JOYDIR_ONE);
        m_aInputMapping[INPUT_FIRE].SetPresetDesc("R Trigger");
        m_aInputMapping[INPUT_JUMP].SetDirection(0, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_JUMP].SetPresetDesc("L Thumbstick Up");
        m_aInputMapping[INPUT_CROUCH].SetDirection(0, 1, JOYDIR_TWO);
        m_aInputMapping[INPUT_CROUCH].SetPresetDesc("L Thumbstick Down");
        m_aInputMapping[INPUT_PIEMENU].SetDirection(0, 2, JOYDIR_TWO);
        m_aInputMapping[INPUT_PIEMENU].SetPresetDesc("L Trigger");
        // So fine aiming can be done with the d-pad while holding down X
        m_aInputMapping[INPUT_AIM_UP].SetDirection(3, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_AIM_UP].SetPresetDesc("D-Pad Up");
        m_aInputMapping[INPUT_AIM_DOWN].SetDirection(3, 1, JOYDIR_TWO);
        m_aInputMapping[INPUT_AIM_DOWN].SetPresetDesc("D-Pad Down");
        m_aInputMapping[INPUT_AIM_LEFT].SetDirection(3, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_AIM_LEFT].SetPresetDesc("D-Pad Left");
        m_aInputMapping[INPUT_AIM_RIGHT].SetDirection(3, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_AIM_RIGHT].SetPresetDesc("D-Pad Right");*/


		// Set up the default xbox 360 button bindings
		m_aInputMapping[INPUT_FIRE].SetJoyButton(JOY_1);
		m_aInputMapping[INPUT_FIRE].SetPresetDesc("A Button");
		
		// Hold down X to enter aim mode, then can use d-pad up/down to finely aim while sniping
		m_aInputMapping[INPUT_AIM].SetJoyButton(JOY_3);
		m_aInputMapping[INPUT_AIM].SetPresetDesc("X Button");
		
		// Pie menu also cancels buy menu, which makes sense for the B button
		m_aInputMapping[INPUT_PIEMENU].SetJoyButton(JOY_2);
		m_aInputMapping[INPUT_PIEMENU].SetPresetDesc("B Button");
		
		// Jump on top button of diamond makes sense
		m_aInputMapping[INPUT_JUMP].SetJoyButton(JOY_4);
		m_aInputMapping[INPUT_JUMP].SetPresetDesc("Y Button");
		
		m_aInputMapping[INPUT_NEXT].SetJoyButton(JOY_6);
		m_aInputMapping[INPUT_NEXT].SetPresetDesc("R Bumper Button");
		m_aInputMapping[INPUT_PREV].SetJoyButton(JOY_5);
		m_aInputMapping[INPUT_PREV].SetPresetDesc("L Bumper Button");
		
		m_aInputMapping[INPUT_START].SetJoyButton(JOY_8);
		m_aInputMapping[INPUT_START].SetPresetDesc("Start Button");
		m_aInputMapping[INPUT_BACK].SetJoyButton(JOY_7);
		m_aInputMapping[INPUT_BACK].SetPresetDesc("Back Button");

		// Set up the default xbox joy direction bindings
		m_aInputMapping[INPUT_L_UP].SetDirection(0, 1, JOYDIR_ONE);
		m_aInputMapping[INPUT_L_UP].SetPresetDesc("L Thumbstick Up");
		m_aInputMapping[INPUT_L_DOWN].SetDirection(0, 1, JOYDIR_TWO);
		m_aInputMapping[INPUT_L_DOWN].SetPresetDesc("L Thumbstick Down");
		m_aInputMapping[INPUT_L_LEFT].SetDirection(0, 0, JOYDIR_ONE);
		m_aInputMapping[INPUT_L_LEFT].SetPresetDesc("L Thumbstick Left");
		m_aInputMapping[INPUT_L_RIGHT].SetDirection(0, 0, JOYDIR_TWO);
		m_aInputMapping[INPUT_L_RIGHT].SetPresetDesc("L Thumbstick Right");

		m_aInputMapping[INPUT_R_UP].SetDirection(1, 0, JOYDIR_ONE);
		m_aInputMapping[INPUT_R_UP].SetPresetDesc("R Thumbstick Up");
		m_aInputMapping[INPUT_R_DOWN].SetDirection(1, 0, JOYDIR_TWO);
		m_aInputMapping[INPUT_R_DOWN].SetPresetDesc("R Thumbstick Down");
		m_aInputMapping[INPUT_R_LEFT].SetDirection(2, 0, JOYDIR_ONE);
		m_aInputMapping[INPUT_R_LEFT].SetPresetDesc("R Thumbstick Left");
		m_aInputMapping[INPUT_R_RIGHT].SetDirection(2, 0, JOYDIR_TWO);
		m_aInputMapping[INPUT_R_RIGHT].SetPresetDesc("R Thumbstick Right");

		m_aInputMapping[INPUT_FIRE].SetDirection(0, 2, JOYDIR_ONE);
		m_aInputMapping[INPUT_FIRE].SetPresetDesc("R Trigger");
		m_aInputMapping[INPUT_PIEMENU].SetDirection(0, 2, JOYDIR_TWO);
		m_aInputMapping[INPUT_PIEMENU].SetPresetDesc("L Trigger");

		m_aInputMapping[INPUT_JUMP].SetDirection(0, 1, JOYDIR_ONE);
		m_aInputMapping[INPUT_JUMP].SetPresetDesc("L Thumbstick Up");
		m_aInputMapping[INPUT_CROUCH].SetDirection(0, 1, JOYDIR_TWO);
		m_aInputMapping[INPUT_CROUCH].SetPresetDesc("L Thumbstick Down");

		// So fine aiming can be done with the d-pad while holding down X
		m_aInputMapping[INPUT_AIM_UP].SetDirection(3, 1, JOYDIR_ONE);
		m_aInputMapping[INPUT_AIM_UP].SetPresetDesc("D-Pad Up");
		m_aInputMapping[INPUT_AIM_DOWN].SetDirection(3, 1, JOYDIR_TWO);
		m_aInputMapping[INPUT_AIM_DOWN].SetPresetDesc("D-Pad Down");
		m_aInputMapping[INPUT_AIM_LEFT].SetDirection(3, 0, JOYDIR_ONE);
		m_aInputMapping[INPUT_AIM_LEFT].SetPresetDesc("D-Pad Left");
		m_aInputMapping[INPUT_AIM_RIGHT].SetDirection(3, 0, JOYDIR_TWO);
		m_aInputMapping[INPUT_AIM_RIGHT].SetPresetDesc("D-Pad Right");


    }
    // Some generic defaults; no real preset is set
    else
    {
        m_SchemePreset = PRESET_NONE;

        // Set up the default mouse button bindings
        m_aInputMapping[INPUT_FIRE].SetMouseButton(MOUSE_LEFT);
        m_aInputMapping[INPUT_PIEMENU].SetMouseButton(MOUSE_RIGHT);

        // Set up the default joystick button bindings
        m_aInputMapping[INPUT_FIRE].SetJoyButton(JOY_1);
        m_aInputMapping[INPUT_AIM].SetJoyButton(JOY_2);
        m_aInputMapping[INPUT_PIEMENU].SetJoyButton(JOY_3);
        m_aInputMapping[INPUT_JUMP].SetJoyButton(JOY_4);
        m_aInputMapping[INPUT_NEXT].SetJoyButton(JOY_6);
        m_aInputMapping[INPUT_PREV].SetJoyButton(JOY_5);
        m_aInputMapping[INPUT_START].SetJoyButton(JOY_8);
        m_aInputMapping[INPUT_BACK].SetJoyButton(JOY_7);

        // Set up the default joystick direction bindings
        m_aInputMapping[INPUT_L_UP].SetDirection(0, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_L_DOWN].SetDirection(0, 1, JOYDIR_TWO);
        m_aInputMapping[INPUT_L_LEFT].SetDirection(0, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_L_RIGHT].SetDirection(0, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_R_UP].SetDirection(2, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_R_DOWN].SetDirection(2, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_R_LEFT].SetDirection(1, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_R_RIGHT].SetDirection(1, 0, JOYDIR_TWO);
        m_aInputMapping[INPUT_FIRE].SetDirection(0, 2, JOYDIR_ONE);
        m_aInputMapping[INPUT_JUMP].SetDirection(0, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_CROUCH].SetDirection(0, 1, JOYDIR_TWO);
        m_aInputMapping[INPUT_PIEMENU].SetDirection(0, 2, JOYDIR_TWO);


        m_aInputMapping[INPUT_WEAPON_CHANGE_PREV].SetDirection(3, 0, JOYDIR_ONE);
        m_aInputMapping[INPUT_WEAPON_CHANGE_NEXT].SetDirection(3, 0, JOYDIR_TWO);
		m_aInputMapping[INPUT_WEAPON_RELOAD].SetDirection(3, 1, JOYDIR_ONE);
        m_aInputMapping[INPUT_WEAPON_PICKUP].SetDirection(3, 1, JOYDIR_TWO);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a InputScheme to be identical to another, by deep copy.

int UInputMan::InputScheme::Create(const InputScheme &reference)
{
    m_ActiveDevice = reference.m_ActiveDevice;
    m_SchemePreset = reference.m_SchemePreset;
	m_JoystickDeadzone = reference.m_JoystickDeadzone;
	m_JoystickDeadzoneType = reference.m_JoystickDeadzoneType;

    for (int mapping = 0; mapping < INPUT_COUNT; ++mapping)
        m_aInputMapping[mapping].Create(reference.m_aInputMapping[mapping]);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int UInputMan::InputScheme::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Device")
        reader >> m_ActiveDevice;
    else if (propName == "Preset")
    {
        reader >> m_SchemePreset;
        SetPreset(m_SchemePreset);
    }
    else if (propName == "LeftUp")
        reader >> m_aInputMapping[INPUT_L_UP];
    else if (propName == "LeftDown")
        reader >> m_aInputMapping[INPUT_L_DOWN];
    else if (propName == "LeftLeft")
        reader >> m_aInputMapping[INPUT_L_LEFT];
    else if (propName == "LeftRight")
        reader >> m_aInputMapping[INPUT_L_RIGHT];
    else if (propName == "RightUp")
        reader >> m_aInputMapping[INPUT_R_UP];
    else if (propName == "RightDown")
        reader >> m_aInputMapping[INPUT_R_DOWN];
    else if (propName == "RightLeft")
        reader >> m_aInputMapping[INPUT_R_LEFT];
    else if (propName == "RightRight")
        reader >> m_aInputMapping[INPUT_R_RIGHT];
    else if (propName == "Fire")
        reader >> m_aInputMapping[INPUT_FIRE];
    else if (propName == "Aim")
        reader >> m_aInputMapping[INPUT_AIM];
    else if (propName == "AimUp")
        reader >> m_aInputMapping[INPUT_AIM_UP];
    else if (propName == "AimDown")
        reader >> m_aInputMapping[INPUT_AIM_DOWN];
    else if (propName == "AimLeft")
        reader >> m_aInputMapping[INPUT_AIM_LEFT];
    else if (propName == "AimRight")
        reader >> m_aInputMapping[INPUT_AIM_RIGHT];
    else if (propName == "PieMenu")
        reader >> m_aInputMapping[INPUT_PIEMENU];
    else if (propName == "Jump")
        reader >> m_aInputMapping[INPUT_JUMP];
    else if (propName == "Crouch")
        reader >> m_aInputMapping[INPUT_CROUCH];
    else if (propName == "Next")
        reader >> m_aInputMapping[INPUT_NEXT];
    else if (propName == "Prev")
        reader >> m_aInputMapping[INPUT_PREV];
    else if (propName == "Start")
        reader >> m_aInputMapping[INPUT_START];
    else if (propName == "Back")
        reader >> m_aInputMapping[INPUT_BACK];
	else if (propName == "WeaponChangeNext")
		reader >> m_aInputMapping[INPUT_WEAPON_CHANGE_NEXT];
	else if (propName == "WeaponChangePrev")
		reader >> m_aInputMapping[INPUT_WEAPON_CHANGE_PREV];
	else if (propName == "WeaponPickup")
		reader >> m_aInputMapping[INPUT_WEAPON_PICKUP];
	else if (propName == "WeaponDrop")
		reader >> m_aInputMapping[INPUT_WEAPON_DROP];
	else if (propName == "WeaponReload")
		reader >> m_aInputMapping[INPUT_WEAPON_RELOAD];
	else if (propName == "JoystickDeadzone")
		reader >> m_JoystickDeadzone;
	else if (propName == "JoystickDeadzoneType")
		reader >> m_JoystickDeadzoneType;
	else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this InputScheme with a Writer for
//                  later recreation with Create(Reader &reader);

int UInputMan::InputScheme::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewLine();
    writer << "// 0 = Keyboard only, 1 = Mouse + Keyboard, 2 = Joystick One, 3 = Joystick Two, , 4 = Joystick Three, 5 = Joystick Four";
    writer.NewProperty("Device");
    writer << m_ActiveDevice;
    if (m_SchemePreset > PRESET_NONE)
    {
        writer << "// 0 = No Preset, 1 = WASD, 2 = Cursor Keys, 3 = XBox 360 Controller";
        writer.NewProperty("Preset");
        writer << m_SchemePreset;
    }
    writer.NewProperty("LeftUp");
    writer << m_aInputMapping[INPUT_L_UP];
    writer.NewProperty("LeftDown");
    writer << m_aInputMapping[INPUT_L_DOWN];
    writer.NewProperty("LeftLeft");
    writer << m_aInputMapping[INPUT_L_LEFT];
    writer.NewProperty("LeftRight");
    writer << m_aInputMapping[INPUT_L_RIGHT];
    writer.NewProperty("RightUp");
    writer << m_aInputMapping[INPUT_R_UP];
    writer.NewProperty("RightDown");
    writer << m_aInputMapping[INPUT_R_DOWN];
    writer.NewProperty("RightLeft");
    writer << m_aInputMapping[INPUT_R_LEFT];
    writer.NewProperty("RightRight");
    writer << m_aInputMapping[INPUT_R_RIGHT];
    writer.NewProperty("Fire");
    writer << m_aInputMapping[INPUT_FIRE];
    writer.NewProperty("Aim");
    writer << m_aInputMapping[INPUT_AIM];
    writer.NewProperty("AimUp");
    writer << m_aInputMapping[INPUT_AIM_UP];
    writer.NewProperty("AimDown");
    writer << m_aInputMapping[INPUT_AIM_DOWN];
    writer.NewProperty("AimLeft");
    writer << m_aInputMapping[INPUT_AIM_LEFT];
    writer.NewProperty("AimRight");
    writer << m_aInputMapping[INPUT_AIM_RIGHT];
    writer.NewProperty("PieMenu");
    writer << m_aInputMapping[INPUT_PIEMENU];
    writer.NewProperty("Jump");
    writer << m_aInputMapping[INPUT_JUMP];
    writer.NewProperty("Crouch");
    writer << m_aInputMapping[INPUT_CROUCH];
    writer.NewProperty("Next");
    writer << m_aInputMapping[INPUT_NEXT];
    writer.NewProperty("Prev");
    writer << m_aInputMapping[INPUT_PREV];
    writer.NewProperty("Start");
    writer << m_aInputMapping[INPUT_START];
    writer.NewProperty("Back");
    writer << m_aInputMapping[INPUT_BACK];
	writer.NewProperty("WeaponChangeNext");
	writer << m_aInputMapping[INPUT_WEAPON_CHANGE_NEXT];
	writer.NewProperty("WeaponChangePrev");
	writer << m_aInputMapping[INPUT_WEAPON_CHANGE_PREV];
	writer.NewProperty("WeaponPickup");
	writer << m_aInputMapping[INPUT_WEAPON_PICKUP];
	writer.NewProperty("WeaponDrop");
	writer << m_aInputMapping[INPUT_WEAPON_DROP];
	writer.NewProperty("WeaponReload");
	writer << m_aInputMapping[INPUT_WEAPON_RELOAD];
	writer.NewProperty("JoystickDeadzone");
	writer << m_JoystickDeadzone;
	writer.NewProperty("JoystickDeadzoneType");
	writer << m_JoystickDeadzoneType;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this UInputMan, effectively
//                  resetting the members of this abstraction level only.

void UInputMan::Clear()
{
    m_DebugArmed = false;

    // Init the previous keys so they don't make it seem like things have changed
    memcpy(s_aLastKeys, const_cast<const char *>(key), KEY_MAX);

    // Neutralize the changed keys so that no Releases will be detected initially
    for (int i = 0; i < KEY_MAX; ++i)
    {
        s_aChangedKeys[i] = false;
    }

    m_DisableKeyboard = false;

    // Reset mouse button data
    m_aMouseButtonState[MOUSE_LEFT] = m_aMousePrevButtonState[MOUSE_LEFT] = m_aMouseChangedButtonState[MOUSE_LEFT] = false;
    m_aMouseButtonState[MOUSE_RIGHT] = m_aMousePrevButtonState[MOUSE_RIGHT] = m_aMouseChangedButtonState[MOUSE_RIGHT] = false;
    m_aMouseButtonState[MOUSE_MIDDLE] = m_aMousePrevButtonState[MOUSE_MIDDLE] = m_aMouseChangedButtonState[MOUSE_MIDDLE] = false;
	
    // Set Mouse analog stick emualtion data defaults
    m_RawMouseMovement.Reset();
    m_AnalogMouseData.Reset();
    m_TrapMousePos = false;
    m_MouseTrapRadius = 350;

    // Mouse wheel init
    position_mouse_z(0);
    m_MouseWheelChange = 0;

    m_DisableMouseMoving = false;
    m_PrepareToEnableMouseMoving = false;

    // Reset all schemes
    int player;
    for (player = 0; player < MAX_PLAYERS; ++player)
    {
		m_TrapMousePosPerPlayer[player] = false;

        m_aControlScheme[player].Clear();
		m_NetworkAccumulatedRawMouseMovement[player].Reset();

		for (int element = 0; element < INPUT_COUNT; element++)
		{
			m_aNetworkInputElementHeld[player][element] = false;
			m_aNetworkInputElementPressed[player][element] = false;
			m_aNetworkInputElementReleased[player][element] = false;
		}

		for (int btn = 0; btn < MAX_MOUSE_BUTTONS; btn++)
		{
			m_aNetworkMouseButtonHeldState[player][btn] = false;
			m_aNetworkMouseButtonPressedState[player][btn] = false;
			m_aNetworkMouseButtonReleasedState[player][btn] = false;
		}
		m_aNetworkAnalogMoveData[player].Reset();
    }

	for (int element = 0; element < INPUT_COUNT; element++)
	{
		m_aNetworkAccumulatedElementPressed[element];
		m_aNetworkAccumulatedElementReleased[element];
	}

    for (int device = 0; device < DEVICE_COUNT; ++device)
    {
        m_apDeviceIcons[device] = 0;
    }

	m_MouseSensitivity = 0.6;

    // Set up the default Mouse+keyboard key mappings for player one
    InputScheme::InputMapping * pMappings = m_aControlScheme[PLAYER_ONE].GetInputMappings();
    m_aControlScheme[PLAYER_ONE].SetDevice(DEVICE_MOUSE_KEYB);
    pMappings[INPUT_L_UP].SetKey(KEY_W);
    pMappings[INPUT_L_DOWN].SetKey(KEY_S);
    pMappings[INPUT_L_LEFT].SetKey(KEY_A);
    pMappings[INPUT_L_RIGHT].SetKey(KEY_D);
    pMappings[INPUT_R_UP].SetKey(KEY_W);
    pMappings[INPUT_R_DOWN].SetKey(KEY_S);
    pMappings[INPUT_R_LEFT].SetKey(KEY_A);
    pMappings[INPUT_R_RIGHT].SetKey(KEY_D);
//    pMappings[INPUT_FIRE].SetKey(KEY_H);
    pMappings[INPUT_FIRE].SetMouseButton(0);
//    pMappings[INPUT_AIM].SetKey(KEY_J);
//    pMappings[INPUT_AIM_UP].SetKey(KEY_W);
//    pMappings[INPUT_AIM_DOWN].SetKey(KEY_S);
//    pMappings[INPUT_PIEMENU].SetKey(KEY_K);
    pMappings[INPUT_PIEMENU].SetMouseButton(1);
    pMappings[INPUT_JUMP].SetKey(KEY_W);
    pMappings[INPUT_CROUCH].SetKey(KEY_S);
    pMappings[INPUT_NEXT].SetKey(KEY_E);
    pMappings[INPUT_PREV].SetKey(KEY_Q);

	pMappings[INPUT_WEAPON_RELOAD].SetKey(KEY_R);
	pMappings[INPUT_WEAPON_DROP].SetKey(KEY_G);
	pMappings[INPUT_WEAPON_PICKUP].SetKey(KEY_F);
	pMappings[INPUT_WEAPON_CHANGE_NEXT].SetKey(KEY_C);


    // Set up the default keyboard key bindings for player two
    pMappings = m_aControlScheme[PLAYER_TWO].GetInputMappings();
    pMappings[INPUT_L_UP].SetKey(84);
    pMappings[INPUT_L_DOWN].SetKey(85);
    pMappings[INPUT_L_LEFT].SetKey(82);
    pMappings[INPUT_L_RIGHT].SetKey(83);
//    pMappings[INPUT_R_UP].SetKey(KEY_UP);
//    pMappings[INPUT_R_DOWN].SetKey(KEY_DOWN);
//    pMappings[INPUT_R_LEFT].SetKey(KEY_LEFT);
//    pMappings[INPUT_R_RIGHT].SetKey(KEY_RIGHT);
    pMappings[INPUT_FIRE].SetKey(38);
    pMappings[INPUT_AIM].SetKey(39);
    pMappings[INPUT_AIM_UP].SetKey(84);
    pMappings[INPUT_AIM_DOWN].SetKey(85);
    pMappings[INPUT_PIEMENU].SetKey(40);
    pMappings[INPUT_JUMP].SetKey(91);
    pMappings[INPUT_CROUCH].SetKey(90);
    pMappings[INPUT_NEXT].SetKey(42);
    pMappings[INPUT_PREV].SetKey(41);

	pMappings[INPUT_WEAPON_RELOAD].SetKey(KEY_0_PAD);
	pMappings[INPUT_WEAPON_DROP].SetKey(KEY_6_PAD);
	pMappings[INPUT_WEAPON_PICKUP].SetKey(KEY_9_PAD);
	pMappings[INPUT_WEAPON_CHANGE_PREV].SetKey(KEY_7_PAD);
	pMappings[INPUT_WEAPON_CHANGE_NEXT].SetKey(KEY_8_PAD);


    // Player three. These key mappings are for when 
    pMappings = m_aControlScheme[PLAYER_THREE].GetInputMappings();
    m_aControlScheme[PLAYER_THREE].SetDevice(DEVICE_GAMEPAD_1);
    pMappings[INPUT_L_UP].SetKey(KEY_W);
    pMappings[INPUT_L_DOWN].SetKey(KEY_S);
    pMappings[INPUT_L_LEFT].SetKey(KEY_A);
    pMappings[INPUT_L_RIGHT].SetKey(KEY_D);
    pMappings[INPUT_R_UP].SetKey(KEY_W);
    pMappings[INPUT_R_DOWN].SetKey(KEY_S);
    pMappings[INPUT_R_LEFT].SetKey(KEY_A);
    pMappings[INPUT_R_RIGHT].SetKey(KEY_D);
    pMappings[INPUT_FIRE].SetKey(KEY_H);
    pMappings[INPUT_AIM].SetKey(KEY_J);
    pMappings[INPUT_AIM_UP].SetKey(KEY_W);
    pMappings[INPUT_AIM_DOWN].SetKey(KEY_S);
    pMappings[INPUT_PIEMENU].SetKey(KEY_K);
    pMappings[INPUT_JUMP].SetKey(KEY_L);
    pMappings[INPUT_CROUCH].SetKey(KEY_STOP);
    pMappings[INPUT_NEXT].SetKey(KEY_U);
    pMappings[INPUT_PREV].SetKey(KEY_Y);

    // Player four
    pMappings = m_aControlScheme[PLAYER_FOUR].GetInputMappings();
    m_aControlScheme[PLAYER_FOUR].SetDevice(DEVICE_GAMEPAD_2);
    pMappings[INPUT_L_UP].SetKey(KEY_UP);
    pMappings[INPUT_L_DOWN].SetKey(KEY_DOWN);
    pMappings[INPUT_L_LEFT].SetKey(KEY_LEFT);
    pMappings[INPUT_L_RIGHT].SetKey(KEY_RIGHT);
    pMappings[INPUT_R_UP].SetKey(KEY_UP);
    pMappings[INPUT_R_DOWN].SetKey(KEY_DOWN);
    pMappings[INPUT_R_LEFT].SetKey(KEY_LEFT);
    pMappings[INPUT_R_RIGHT].SetKey(KEY_RIGHT);
    pMappings[INPUT_FIRE].SetKey(KEY_1_PAD);
    pMappings[INPUT_AIM].SetKey(KEY_2_PAD);
    pMappings[INPUT_AIM_UP].SetKey(KEY_UP);
    pMappings[INPUT_AIM_DOWN].SetKey(KEY_DOWN);
    pMappings[INPUT_PIEMENU].SetKey(KEY_3_PAD);
    pMappings[INPUT_JUMP].SetKey(KEY_DEL_PAD);
    pMappings[INPUT_CROUCH].SetKey(KEY_STOP);
    pMappings[INPUT_NEXT].SetKey(KEY_5_PAD);
    pMappings[INPUT_PREV].SetKey(KEY_4_PAD);

    pMappings = 0;

    // Init the previous joybuttons so they don't make it seem like things have changed
    // Also neutralize the changed keys so that no Releases will be detected initially
    int joystick, button, stick, axis;
    // This has to use MAX_PLAYERS instead of num_joysticks, because the latter may not have been inited yet
    for (joystick = 0; joystick < MAX_PLAYERS; ++joystick)
    {
        // Init button change state detection data
        for (button = 0; button < joy[joystick].num_buttons; ++button)
        {
            s_aaPrevJoyState[joystick].button[button].b = false;
            s_aaChangedJoyState[joystick].button[button].b = false;
        }

        // Init stick-axis-direction change state detection data
        for (stick = 0; stick < joy[joystick].num_sticks; ++stick)
        {
            for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis)
            {
                s_aaPrevJoyState[joystick].stick[stick].axis[axis].d1 = 0;
                s_aaPrevJoyState[joystick].stick[stick].axis[axis].d2 = 0;
                s_aaChangedJoyState[joystick].stick[stick].axis[axis].d1 = 0;
                s_aaChangedJoyState[joystick].stick[stick].axis[axis].d2 = 0;
            }
        }
    }

	m_LastDeviceWhichControlledGUICursor = 0;
	m_OverrideInput = false;
	m_IsInMultiplayerMode = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the UInputMan object ready for use.

int UInputMan::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    // This is a hack to fix the keyboard and mouse losing focus when in windowed mode.. it's a known allegro issue
//    if (!g_FrameMan.IsFullscreen())
//        rest(500);

    // Get the Allegro keyboard going
    install_keyboard();
    // Hack to not have the keyboard lose focus permanently when window is started without focus
    //win_grab_input();

    // Get the Joysticks going
#ifdef WIN32
	if (install_joystick(JOY_TYPE_WIN32) != 0)
	{
		//DDTAbort("Error initialising joysticks!");
		// No win 32 joysticks? Try DX then
		if (num_joysticks == 0)
		{
			remove_joystick();
			if (install_joystick(JOY_TYPE_DIRECTX) != 0)
			{
				//DDTAbort("Error initialising joysticks!");

				// No dx either? Try whatever is possible
				if (num_joysticks   == 0)
				{
					remove_joystick();
					if (install_joystick(JOY_TYPE_AUTODETECT) != 0)
						DDTAbort("Error initialising joysticks!");
				}
			}
		}
	}
#else
    if (install_joystick(JOY_TYPE_AUTODETECT) != 0)
        DDTAbort("Error initialising joysticks!");
#endif

    poll_joystick();
/* Can't do this now, the data modules aren't loaded yet.. this is done lazily as the first one is gotten
    // Get the device Icons
    m_apDeviceIcons[DEVICE_KEYB_ONLY] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
    m_apDeviceIcons[DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));
    m_apDeviceIcons[DEVICE_GAMEPAD_1] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 1"));
    m_apDeviceIcons[DEVICE_GAMEPAD_2] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 2"));
    m_apDeviceIcons[DEVICE_GAMEPAD_3] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 3"));
    m_apDeviceIcons[DEVICE_GAMEPAD_4] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 4"));
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int UInputMan::ReadProperty(std::string propName, Reader &reader)
{
    int mappedButton = 0;


    if (propName == "P1Scheme")
        reader >> m_aControlScheme[PLAYER_ONE];
    else if (propName == "P2Scheme")
        reader >> m_aControlScheme[PLAYER_TWO];
    else if (propName == "P3Scheme")
        reader >> m_aControlScheme[PLAYER_THREE];
    else if (propName == "P4Scheme")
        reader >> m_aControlScheme[PLAYER_FOUR];
    else if (propName == "MouseSensitivity")
        reader >> m_MouseSensitivity;
	else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this UInputMan with a Writer for
//                  later recreation with Create(Reader &reader);

int UInputMan::Save(Writer &writer) const
{
    writer.NewProperty("MouseSensitivity");
    writer << m_MouseSensitivity;
    writer.NewProperty("P1Scheme");
    writer << m_aControlScheme[PLAYER_ONE];
    writer.NewProperty("P2Scheme");
    writer << m_aControlScheme[PLAYER_TWO];
    writer.NewProperty("P3Scheme");
    writer << m_aControlScheme[PLAYER_THREE];
    writer.NewProperty("P4Scheme");
    writer << m_aControlScheme[PLAYER_FOUR];

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the UInputMan object.

void UInputMan::Destroy()
{
    Clear();
}
	

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReInitKeyboard
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Re-initalizes the keyboard for when windows regains focus. This is
//                  really used to work around an Allegro bug.

void UInputMan::ReInitKeyboard()
{
//    remove_keyboard();
    install_keyboard();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetInputClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the input class for use if one is available
void UInputMan::SetInputClass(GUIInput* pInputClass)
{
	s_InputClass = pInputClass;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSchemeIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the current device Icon of a specific player's scheme

const Icon * UInputMan::GetSchemeIcon(int whichPlayer)
{
    if (whichPlayer < Activity::PLAYER_1 || whichPlayer >= Activity::MAXPLAYERCOUNT)
        return 0;

    // Lazy load of these.. can't load earlier
    if (!m_apDeviceIcons[DEVICE_KEYB_ONLY])
    {
        m_apDeviceIcons[DEVICE_KEYB_ONLY] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
        m_apDeviceIcons[DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));
        m_apDeviceIcons[DEVICE_GAMEPAD_1] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 1"));
        m_apDeviceIcons[DEVICE_GAMEPAD_2] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 2"));
        m_apDeviceIcons[DEVICE_GAMEPAD_3] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 3"));
        m_apDeviceIcons[DEVICE_GAMEPAD_4] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 4"));
    }
    return m_apDeviceIcons[m_aControlScheme[whichPlayer].GetDevice()];
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeviceIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get the current Icon of a specific device's scheme

const Icon * UInputMan::GetDeviceIcon(int whichDevice)
{
	if (whichDevice < DEVICE_KEYB_ONLY || whichDevice > DEVICE_GAMEPAD_4)
		return 0;

	// Lazy load of these.. can't load earlier
	if (!m_apDeviceIcons[DEVICE_KEYB_ONLY])
		GetSchemeIcon(0);

	return m_apDeviceIcons[whichDevice];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoystickActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified joystick is active

bool UInputMan::JoystickActive(int joystickNumber) const
{
	if (joystickNumber < 0 || joystickNumber >= MAX_PLAYERS)
		return false;

	/*if (s_aaPrevJoyState[joystickNumber].num_buttons > 0)
		return true;

	if (s_aaChangedJoyState[joystickNumber].num_buttons > 0)
		return true;*/

	if (joystickNumber < num_joysticks)
		return true;

	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJoystickCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of active joysticks

int UInputMan::GetJoystickCount() const
{
	if (num_joysticks > MAX_PLAYERS)
		return MAX_PLAYERS;
	else
		return num_joysticks;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMappingName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the key/mouse/joy button/key/direction that a
//                  particular input element is mapped to.

string UInputMan::GetMappingName(int whichPlayer, int whichElement)
{
    if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS)
        return "";

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();

    // Which scheme preset, if any, this adheres to
    int preset = m_aControlScheme[whichPlayer].GetPreset();

    // Get handy pointer to the relevant input element
    InputScheme::InputMapping * pElement = &(m_aControlScheme[whichPlayer].GetInputMappings()[whichElement]);

    // If there is a preset, just return the element name set by the preset previously
    if (preset != PRESET_NONE && !pElement->GetPresetDesc().empty())
        return pElement->GetPresetDesc();

    // Joystick input is used, more important to show than keyboard
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Check joysitck button presses
        if (pElement->GetJoyButton() != JOY_NONE)
            return joy[whichJoy].button[pElement->GetJoyButton()].name;

        // Check joystick axis directions
        if (pElement->JoyDirMapped())
        {
            return "Joystick";
//            pressed = JoyDirectionPressed(whichJoy, pElement->GetStick(), pElement->GetAxis(), pElement->GetDirection());
        }
    }

    // Mouse input is used, more important to show than keyboard
    if (device == DEVICE_MOUSE_KEYB && pElement->GetMouseButton() != MOUSE_NONE)
    {
        // Check mouse
        int button = pElement->GetMouseButton();
        if (button == MOUSE_LEFT)
            return "Left Mouse";
        else if (button == MOUSE_RIGHT)
            return "Right Mouse";
        else if (button == MOUSE_MIDDLE)
            return "Middle Mouse";
    }

    // Keyboard defualts - don't check certain elements which don't make sense when in mouse mode
    if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN)))
    {
        // Return keyboard mapping name
        if (pElement->GetKey() != 0)
            return scancode_to_name(pElement->GetKey());
    }

    return "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureKeyMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any key press this frame, and creates an input mapping
//                  for a specific player accordingly.

bool UInputMan::CaptureKeyMapping(int whichPlayer, int whichInput)
{	
    if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS)
        return false;

    // Update the key array
	if (keyboard_needs_poll())
        poll_keyboard();

    for (int whichKey = 0; whichKey < KEY_MAX; ++whichKey)
    {	
        if (KeyPressed(whichKey) /* s_aLastKeys[whichKey] && s_aChangedKeys[whichKey]*/)
        {
            // Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
            m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].Clear();
            SetKeyMapping(whichPlayer, whichInput, whichKey);
            return true;
        }

    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureButtonMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any button press this frame, and creates an input mapping
//                  for a specific player accordingly.

bool UInputMan::CaptureButtonMapping(int whichPlayer, int whichJoy, int whichInput)
{
    if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS)
        return false;

    int whichButton = WhichJoyButtonPressed(whichJoy);

    if (whichButton != JOY_NONE)
    {
        // Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
        m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].Clear();
        SetButtonMapping(whichPlayer, whichInput, whichButton);
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureDirectionMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any joystick pad or stick direction press this frame,
//                  and creates an input mapping for a specific player accordingly.

bool UInputMan::CaptureDirectionMapping(int whichPlayer, int whichJoy, int whichInput)
{
    if (whichPlayer < PLAYER_ONE || whichPlayer >= MAX_PLAYERS)
        return false;

    int stick, axis;
    // Go through all the sticks on this joystick
    for (stick = 0; stick < joy[whichJoy].num_sticks; ++stick)
    {
        // Go through all the axes on this joystick
        for (axis = 0; axis < joy[whichJoy].stick[stick].num_axis; ++axis)
        {
            // See if there is direction press in the first direction
            if (joy[whichJoy].stick[stick].axis[axis].d1 && s_aaChangedJoyState[whichJoy].stick[stick].axis[axis].d1)
            {
                // Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
                m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].Clear();
                // Capture the mapping!
                m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].SetDirection(stick, axis, JOYDIR_ONE);
                return true;
            }
            // Check the other direction
            else if (joy[whichJoy].stick[stick].axis[axis].d2 && s_aaChangedJoyState[whichJoy].stick[stick].axis[axis].d2)
            {
                // Clear out all the mappings for this input first, because otherwise old device mappings may linger and interfere
                m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].Clear();
                // Capture the mapping!
                m_aControlScheme[whichPlayer].GetInputMappings()[whichInput].SetDirection(stick, axis, JOYDIR_TWO);
                return true;
            }
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CaptureJoystickMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the any button or direction press this frame, and creates
//                  an input mapping for a specific player accordingly.

bool UInputMan::CaptureJoystickMapping(int whichPlayer, int whichJoy, int whichInput)
{
    // Try buttons first
    if (CaptureButtonMapping(whichPlayer, whichJoy, whichInput))
        return true;
    // Then directions
    if (CaptureDirectionMapping(whichPlayer, whichJoy, whichInput))
        return true;

    return false;
}

Vector UInputMan::GetNetworkAccumulatedRawMouseMovement(int player)
{
	Vector tmp = m_NetworkAccumulatedRawMouseMovement[player];
	m_NetworkAccumulatedRawMouseMovement[player].Reset();
	return tmp;
	//return m_NetworkAccumulatedRawMouseMovement[player];
}

void UInputMan::SetNetworkMouseInput(int player, Vector input)
{
	m_OverrideInput = true;
	m_NetworkAccumulatedRawMouseMovement[player] = input;
}

void UInputMan::SetNetworkInputElementHeldState(int player, int element, bool state)
{
	m_OverrideInput = true;
	if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS)
	{
		m_aNetworkInputElementHeld[player][element] = state;
		/*if (state && element == INPUT_L_LEFT)
			g_ConsoleMan.PrintString("L #");
		if (!state && element == INPUT_L_LEFT)
			g_ConsoleMan.PrintString("L O");*/
	}
}

void UInputMan::SetNetworkInputElementPressedState(int player, int element, bool state)
{
	m_OverrideInput = true;
	if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS)
		m_aNetworkInputElementPressed[player][element] = state;
}

void UInputMan::SetNetworkInputElementReleasedState(int player, int element, bool state)
{
	m_OverrideInput = true;
	if (element >= 0 && element < INPUT_COUNT && player >= 0 && player < MAX_PLAYERS)
		m_aNetworkInputElementReleased[player][element] = state;
}



void UInputMan::SetNetworkMouseButtonPressedState(int player, int whichButton, bool state)
{
	m_OverrideInput = true;
	if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS)
		m_aNetworkMouseButtonPressedState[player][whichButton] = state;
}

void UInputMan::SetNetworkMouseButtonReleasedState(int player, int whichButton, bool state)
{
	m_OverrideInput = true;
	if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS)
		m_aNetworkMouseButtonReleasedState[player][whichButton] = state;
}


void UInputMan::SetNetworkMouseButtonHeldState(int player, int whichButton, bool state)
{
	m_OverrideInput = true;
	if (whichButton >= 0 && whichButton < MAX_MOUSE_BUTTONS && player >= 0 && player < MAX_PLAYERS)
		m_aNetworkMouseButtonHeldState[player][whichButton] = state;
}

void UInputMan::SetNetworkMouseWheelState(int player, int state)
{
	m_OverrideInput = true;
	if (player >= 0 && player < MAX_PLAYERS)
		m_aNetworkMouseWheelState[player] = state;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was depressed between the last
//                  update and the one previous to it.

bool UInputMan::ElementPressed(int whichPlayer, int whichElement)
{
	if (m_OverrideInput && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS)
	{
		if (m_TrapMousePosPerPlayer[whichPlayer])
			return m_aNetworkInputElementPressed[whichPlayer][whichElement];
		else
			return false;
	}

    bool pressed = false;

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();

    // Get handy pointer to the relevant input element
    InputScheme::InputMapping * pElement = &(m_aControlScheme[whichPlayer].GetInputMappings()[whichElement]);

    // Keyboard is involved
    // Don't check certain elements which don't make sense when in mouse mode
    if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN)))
    {
        // Check keyboard
        pressed = pressed ? true : KeyPressed(pElement->GetKey());
    }

    // Mouse is involved
    if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos)
    {
        // Check mouse
        pressed = pressed ? true : MouseButtonPressed(pElement->GetMouseButton(), whichPlayer);
    }

    // Joystick input is applicable
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Check joysitck button presses
        pressed = pressed ? true : JoyButtonPressed(whichJoy, pElement->GetJoyButton());

        // Check joystick axis directions
        if (!pressed && pElement->JoyDirMapped())
            pressed = JoyDirectionPressed(whichJoy, pElement->GetStick(), pElement->GetAxis(), pElement->GetDirection());
    }

    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was released between the last
//                  update and the one previous to it.

bool UInputMan::ElementReleased(int whichPlayer, int whichElement)
{
	if (m_OverrideInput && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS)
	{
		if (m_TrapMousePosPerPlayer[whichPlayer])
			return m_aNetworkInputElementReleased[whichPlayer][whichElement];
		else
			return false;
	}

    bool released = false;

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();

    // Get handy pointer to the relevant input element
    InputScheme::InputMapping * pElement = &(m_aControlScheme[whichPlayer].GetInputMappings()[whichElement]);

    // Keyboard is involved
    // Don't check certain elements which don't make sense when in mouse mode
    if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN)))
    {
        // Check keyboard
        released = released ? true : KeyReleased(pElement->GetKey());
    }

    // Mouse is involved
    if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos)
    {
        // Check mouse
        released = released ? true : MouseButtonReleased(pElement->GetMouseButton(), whichPlayer);
    }

    // Joystick input is applicable
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Check joysitck button presses
        released = released ? true : JoyButtonReleased(whichJoy, pElement->GetJoyButton());

        // Check joystick axis directions
        if (!released && pElement->JoyDirMapped())
            released = JoyDirectionReleased(whichJoy, pElement->GetStick(), pElement->GetAxis(), pElement->GetDirection());
    }

    return released;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ElementHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific input element was held during the last update.

bool UInputMan::ElementHeld(int whichPlayer, int whichElement)
{
	if (m_OverrideInput && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS)
	{
		/*if (whichElement == INPUT_L_LEFT)
		{
			if (m_aNetworkInputElementPressed[whichPlayer][whichElement])
				g_ConsoleMan.PrintString("L +");
			else
				g_ConsoleMan.PrintString("L -");
		}*/

		if (m_TrapMousePosPerPlayer[whichPlayer])
			return m_aNetworkInputElementHeld[whichPlayer][whichElement];
		else
			return false;
	}

    bool held = false;

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();

    // Get handy pointer to the relevant input element
    InputScheme::InputMapping * pElement = &(m_aControlScheme[whichPlayer].GetInputMappings()[whichElement]);

    // Keyboard is involved
    // Don't check certain elements which don't make sense when in mouse mode
    if (device == DEVICE_KEYB_ONLY || (device == DEVICE_MOUSE_KEYB && !(whichElement == INPUT_AIM_UP || whichElement == INPUT_AIM_DOWN)))
    {
        // Check keyboard
        held = held ? true : KeyHeld(pElement->GetKey());
    }

    // Mouse is involved
    if (device == DEVICE_MOUSE_KEYB && m_TrapMousePos)
    {
        // Check mouse
        held = held ? true : MouseButtonHeld(pElement->GetMouseButton(), whichPlayer);
    }

    // Joystick input is applicable
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Check joysitck button holds
        held = held ? true : JoyButtonHeld(whichJoy, pElement->GetJoyButton());

        // Check joystick axis direction holds
        if (!held && pElement->JoyDirMapped())
            held = JoyDirectionHeld(whichJoy, pElement->GetStick(), pElement->GetAxis(), pElement->GetDirection());
    }

    return held;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key was depressed between the last update and the one
//                  previous to it.

bool UInputMan::KeyPressed(const char keyToTest)
{
    // Keyboard disabled
    if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC))
        return false;

	bool pressed = false;
	
	if (s_InputClass)
	{		
		pressed = (s_InputClass->GetScanCodeState(keyToTest) == GUIInput::Pushed);
	}
	else
	{
		pressed = s_aLastKeys[keyToTest] && s_aChangedKeys[keyToTest];
	}
	
//    s_aChangedKeys[keyToTest] = false;
    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key was released between the last update and the one
//                  previous to it.

bool UInputMan::KeyReleased(const char keyToTest)
{
    // Keyboard disabled
    if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC))
        return false;

    bool released = !s_aLastKeys[keyToTest] && s_aChangedKeys[keyToTest];
//    s_aChangedKeys[keyToTest] = false;
    return released;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeyHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a key is being held right now.

bool UInputMan::KeyHeld(const char keyToTest)
{
    // Keyboard disabled
    if (m_DisableKeyboard && (keyToTest >= KEY_A && keyToTest < KEY_ESC))
        return false;

    return s_aLastKeys[keyToTest];
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichKeyHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the first keyboard key button which is currently down.

int UInputMan::WhichKeyHeld()
{
    int key = readkey();
    // decode the scancode and return it
    return key >> 8;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mosue button was depressed between the last update and
//                  the one previous to it.

bool UInputMan::MouseButtonPressed(int whichButton, int whichPlayer)
{
    if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS)
        return false;

	if (m_OverrideInput)
	{
		if (whichPlayer < 0 || whichPlayer >= MAX_PLAYERS)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_aNetworkMouseButtonPressedState[i][whichButton])
					return m_aNetworkMouseButtonPressedState[i][whichButton];
			}
			return m_aNetworkMouseButtonPressedState[0][whichButton];
		}
		else
		{
			return m_aNetworkMouseButtonPressedState[whichPlayer][whichButton];
		}
	}

    bool pressed = m_aMouseButtonState[whichButton] && m_aMouseChangedButtonState[whichButton];

    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mosue button was released between the last update and
//                  the one previous to it.

bool UInputMan::MouseButtonReleased(int whichButton, int whichPlayer)
{
    if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS)
        return false;

	if (m_OverrideInput)
	{
		if (whichPlayer <0 || whichPlayer >= MAX_PLAYERS)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_aNetworkMouseButtonReleasedState[i][whichButton])
					return m_aNetworkMouseButtonReleasedState[i][whichButton];
			}
			return m_aNetworkMouseButtonReleasedState[0][whichButton];
		
		}
		else 
		{
			return m_aNetworkMouseButtonReleasedState[whichPlayer][whichButton];
		}
	}

    bool released = !m_aMouseButtonState[whichButton] && m_aMouseChangedButtonState[whichButton];

    return released;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a mouse button is being held down right now.

bool UInputMan::MouseButtonHeld(int whichButton, int whichPlayer)
{
    if (whichButton < 0 || whichButton >= MAX_MOUSE_BUTTONS)
        return false;

	if (m_OverrideInput)
	{
		if (whichPlayer < 0 || whichPlayer >= MAX_PLAYERS)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				if (m_aNetworkMouseButtonHeldState[i][whichButton])
					return m_aNetworkMouseButtonHeldState[i][whichButton];
			}
			return m_aNetworkMouseButtonHeldState[0][whichButton];

		}
		else
		{
			return m_aNetworkMouseButtonHeldState[whichPlayer][whichButton];
		}
	}

    return m_aMouseButtonState[whichButton];
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joy button was depressed between the last update and
//                  the one previous to it.

bool UInputMan::JoyButtonPressed(int whichJoy, int whichButton)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks)
        return false;

    if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons)
        return false;

    bool pressed = joy[whichJoy].button[whichButton].b && s_aaChangedJoyState[whichJoy].button[whichButton].b;
//    s_aaChangedJoyState[whichJoy].button[whichButton].b = false;
    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joy button was released between the last update and
//                  the one previous to it.

bool UInputMan::JoyButtonReleased(int whichJoy, int whichButton)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks)
        return false;

    if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons)
        return false;

    bool released = !joy[whichJoy].button[whichButton].b && s_aaChangedJoyState[whichJoy].button[whichButton].b;
//    s_aaChangedJoyState[whichJoy].button[whichButton].b = false;
    return released;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick button is being held down right now.

bool UInputMan::JoyButtonHeld(int whichJoy, int whichButton)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks)
        return false;

    if (whichButton < 0 || whichButton >= joy[whichJoy].num_buttons)
        return false;

    return joy[whichJoy].button[whichButton].b;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichJoyButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the first joystick button which is currently down.

int UInputMan::WhichJoyButtonHeld(int whichJoy)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks)
        return JOY_NONE;

    for (int button = 0; button < joy[whichJoy].num_buttons; ++button)
    {
        if (joy[whichJoy].button[button].b)
            return button;
    }

    // No button is held down
    return JOY_NONE;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichJoyButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the first joystick button which was pressed down since last frame.

int UInputMan::WhichJoyButtonPressed(int whichJoy)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks)
        return JOY_NONE;

    for (int button = 0; button < joy[whichJoy].num_buttons; ++button)
    {
        if (joy[whichJoy].button[button].b)
        {
            if (JoyButtonPressed(whichJoy, button))
                return button;
        }
    }

    // No button was pressed down
    return JOY_NONE;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick axis direction was depressed between the
//                  last update and the one previous to it.

bool UInputMan::JoyDirectionPressed(int whichJoy, int whichStick, int whichAxis, int whichDir)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks ||
        whichStick < 0 || whichStick >= joy[whichJoy].num_sticks ||
        whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis)
        return false;

    if (whichDir == JOYDIR_TWO)
        return joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_aaChangedJoyState[whichJoy].stick[whichStick].axis[whichAxis].d2;
    else
        return joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_aaChangedJoyState[whichJoy].stick[whichStick].axis[whichAxis].d1;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick axis direction was released between the
//                  last update and the one previous to it.

bool UInputMan::JoyDirectionReleased(int whichJoy, int whichStick, int whichAxis, int whichDir)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks ||
        whichStick < 0 || whichStick >= joy[whichJoy].num_sticks ||
        whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis)
        return false;

    if (whichDir == JOYDIR_TWO)
        return !joy[whichJoy].stick[whichStick].axis[whichAxis].d2 && s_aaChangedJoyState[whichJoy].stick[whichStick].axis[whichAxis].d2;
    else
        return !joy[whichJoy].stick[whichStick].axis[whichAxis].d1 && s_aaChangedJoyState[whichJoy].stick[whichStick].axis[whichAxis].d1;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          JoyDirectionHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a joystick axis is being held down in a specific
//                  direction right now. Two adjacent direcitons can be held down to produce
//                  diagonals.

bool UInputMan::JoyDirectionHeld(int whichJoy, int whichStick, int whichAxis, int whichDir)
{
    if (whichJoy < 0 || whichJoy >= num_joysticks ||
        whichStick < 0 || whichStick >= joy[whichJoy].num_sticks ||
        whichAxis < 0 || whichAxis >= joy[whichJoy].stick[whichStick].num_axis)
        return false;

    if (whichDir == JOYDIR_TWO)
        return joy[whichJoy].stick[whichStick].axis[whichAxis].d2;
    else
        return joy[whichJoy].stick[whichStick].axis[whichAxis].d1;

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogMoveValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog moving values of a specific player's control scheme.

Vector UInputMan::AnalogMoveValues(int whichPlayer)
{
    Vector moveValues;

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();

    // Get handy pointer to the relevant input elements
    InputScheme::InputMapping * pElement = m_aControlScheme[whichPlayer].GetInputMappings();
/*
    // Mouse is involved
    if (device == DEVICE_MOUSE_KEYB)
    {
        // Check mouse
        
    }
*/
    // Joystick input is applicable
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Assume axes are stretched out over up-down, and left-right
		if (pElement[INPUT_L_LEFT].JoyDirMapped())
			moveValues.m_X = AnalogAxisValue(whichJoy, pElement[INPUT_L_LEFT].GetStick(), pElement[INPUT_L_LEFT].GetAxis());
		if (pElement[INPUT_L_UP].JoyDirMapped())
			moveValues.m_Y = AnalogAxisValue(whichJoy, pElement[INPUT_L_UP].GetStick(), pElement[INPUT_L_UP].GetAxis());
    }

    return moveValues;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogAimValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog aiming values of a specific player's control scheme.

Vector UInputMan::AnalogAimValues(int whichPlayer)
{
    Vector aimValues;

    // Which Device is used by this player
    int device = m_aControlScheme[whichPlayer].GetDevice();
	if (m_OverrideInput)
		device = UInputMan::DEVICE_MOUSE_KEYB;

    // Get handy pointer to the relevant input elements
    InputScheme::InputMapping * pElement = m_aControlScheme[whichPlayer].GetInputMappings();

    // Mouse is involved
    if (device == DEVICE_MOUSE_KEYB)
    {
        // Return the normalized mouse analog stick emulation value
		if (m_OverrideInput && whichPlayer >= 0 && whichPlayer < MAX_PLAYERS)
			aimValues = m_aNetworkAnalogMoveData[whichPlayer] / m_MouseTrapRadius;
		else
			aimValues = m_AnalogMouseData / m_MouseTrapRadius;
    }

    // Joystick input is applicable
    if (device >= DEVICE_GAMEPAD_1)
    {
        // Translate the device selection to the joystick numbering (0-3)
        int whichJoy = device - DEVICE_GAMEPAD_1;

        // Assume axes are stretched out over up-down, and left-right
		if (pElement[INPUT_R_LEFT].JoyDirMapped())
			aimValues.m_X = AnalogAxisValue(whichJoy, pElement[INPUT_R_LEFT].GetStick(), pElement[INPUT_R_LEFT].GetAxis());
		if (pElement[INPUT_R_UP].JoyDirMapped())
			aimValues.m_Y = AnalogAxisValue(whichJoy, pElement[INPUT_R_UP].GetStick(), pElement[INPUT_R_UP].GetAxis());
    }

    return aimValues;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogAxisValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the normalized value of a certain joystick's stick's axis.

float UInputMan::AnalogAxisValue(int whichJoy, int whichStick, int whichAxis)
{
    if (whichJoy < num_joysticks && whichStick < joy[whichJoy].num_sticks && whichAxis < joy[whichJoy].stick[whichStick].num_axis)
    {
        // Treat unsigned (throttle axes) as rudders, with a range of 0-255 and midpoint of 128
        if (joy[whichJoy].stick[whichStick].flags & JOYFLAG_UNSIGNED)
            return (float)(joy[whichJoy].stick[whichStick].axis[whichAxis].pos - 128) / (float)128;
        // Regular signed axis with range of -128 to 128
        else
            return (float)joy[whichJoy].stick[whichStick].axis[whichAxis].pos / (float)128;
    }
    return  0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnalogStickValues
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the analog values of a certain joystick device stick.

Vector UInputMan::AnalogStickValues(int whichJoy, int whichStick)
{
/*
// HACK! make work with 360 controller
    if (whichStick == 1)
    {
        return Vector((float)(joy[whichJoy].stick[1].axis[0].pos - 128) / (float)128,
                      (float)(joy[whichJoy].stick[2].axis[0].pos - 128) / (float)128);
    }
*/
    return Vector(AnalogAxisValue(whichJoy, whichStick, 0), AnalogAxisValue(whichJoy, whichStick, 1));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMouseMovement
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the relative movement of the mouse since last update. Only returns
//                  true if the selected player actuall is using the mouse.

Vector UInputMan::GetMouseMovement(int whichPlayer)
{
    Vector mouseMovement;
	if (m_OverrideInput)
	{
		if (whichPlayer >= 0 && whichPlayer < MAX_PLAYERS)
		{
			mouseMovement = m_NetworkAccumulatedRawMouseMovement[whichPlayer];
			return mouseMovement;
		}
	}

    if (whichPlayer == -1 || m_aControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB)
        mouseMovement = m_RawMouseMovement;

    return mouseMovement;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DisableMouseMoving
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Will temporarily disable positioniong of the mouse. This is so that
//                  when focus is swtiched back to the game window, it avoids having the
//                  window fly away because the user clicked the title bar of the window.
// Arguments:       Whether to disable or not.
// Return value:    None.

void UInputMan::DisableMouseMoving(bool disable)
{
    if (disable)
    {
        m_DisableMouseMoving = true;
        m_PrepareToEnableMouseMoving = false;
        // Set these to outside the screen so the mouse has to be updated first before checking if they're in the screen or not
        mouse_x = mouse_y = -1;
    }
    else
        m_PrepareToEnableMouseMoving = true;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMousePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the absolute screen position of the mouse cursor.
// Arguments:       Where to place the mouse.

void UInputMan::SetMousePos(Vector &newPos, int whichPlayer)
{
    // Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
    if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == -1 || m_aControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB))
        position_mouse(newPos.m_X, newPos.m_Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceMouseWithinPlayerScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the mouse within a specific player's screen area.

void UInputMan::ForceMouseWithinPlayerScreen(int whichPlayer)
{
    if (whichPlayer < PLAYER_ONE || whichPlayer >= PLAYER_FOUR)
        return;

    int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer);
    int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer);

    // If we are dealing with split screens, then draw the intermediate draw splitscreen to the appropriate spot on the back buffer
    if (g_FrameMan.GetScreenCount() > 1)
    {
        // Always upper left corner
        if (whichPlayer == 0)
            ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
        else if (whichPlayer == 1)
        {
            // If both splits, or just Vsplit, then in upper right quadrant
            if ((g_FrameMan.GetVSplit() && !g_FrameMan.GetHSplit()) || (g_FrameMan.GetVSplit() && g_FrameMan.GetHSplit()))
                ForceMouseWithinBox(g_FrameMan.GetResX() / 2, 0, screenWidth, screenHeight, whichPlayer);
            // If only hsplit, then lower left quadrant
            else
                ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
        }
        // Always lower left quadrant
        else if (whichPlayer == 2)
            ForceMouseWithinBox(0, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
        // Always lower right quadrant
        else if (whichPlayer == 3)
            ForceMouseWithinBox(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2, screenWidth, screenHeight, whichPlayer);
    }
    // No splitscreen, jsut constrain hte mouse to the full screen
    else
        ForceMouseWithinBox(0, 0, screenWidth, screenHeight, whichPlayer);
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceMouseWithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the mouse within a box on the screen.

void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer)
{
    // Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
    if (!m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == -1 || m_aControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB))
    {
        float windowResMultiplier = g_FrameMan.IsFullscreen() ? g_FrameMan.NxFullscreen() : g_FrameMan.NxWindowed();
        int mouseX = MAX(x, mouse_x);
        int mouseY = MAX(y, mouse_y);
        mouseX = MIN(mouseX, x + width * windowResMultiplier);
        mouseY = MIN(mouseY, y + height * windowResMultiplier);
		
#if !defined(__APPLE__)
		// [CHRISK] OSX really doesn't like this
        position_mouse(mouseX, mouseY);
#endif // !defined(__APPLE__)
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick input at all, buttons or d-pad.

bool UInputMan::AnyJoyInput()
{
    bool input = false;

    poll_joystick();

    int joystick, button, stick, axis;
    for (joystick = 0; joystick < MAX_PLAYERS && !input; ++joystick)
    {
        // Check all buttons
        for (button = 0; button < MAX_JOY_BUTTONS && !input; ++button)
        {
            input = joy[joystick].button[button].b ? true : input;
        }

        if (!input)
        {
            // Stick, axis, directions
            for (stick = 0; stick < joy[joystick].num_sticks; ++stick)
            {
                for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis)
                {
                    input = JoyDirectionHeld(joystick, stick, axis, JOYDIR_ONE) || JoyDirectionHeld(joystick, stick, axis, JOYDIR_TWO);
                }
            }
        }
    }

    return input;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick presses at all, buttons or d-pad.

bool UInputMan::AnyJoyPress()
{
    bool input = false;

    int joystick, button, stick, axis;
    for (joystick = 0; joystick < MAX_PLAYERS && !input; ++joystick)
    {
        // Check for button presses
        for (button = 0; button < MAX_JOY_BUTTONS && !input; ++button)
        {
            input = JoyButtonPressed(joystick, button) ? true : input;
        }

        if (!input)
        {
            // Stick, axis, directions
            for (stick = 0; stick < joy[joystick].num_sticks; ++stick)
            {
                for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis)
                {
                    input = JoyDirectionPressed(joystick, stick, axis, JOYDIR_ONE) || JoyDirectionPressed(joystick, stick, axis, JOYDIR_TWO);
                }
            }
        }
    }

    return input;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyMouseButtonPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any mouse button presses at all

bool UInputMan::AnyMouseButtonPress()
{
    bool input = false;

    for (int button = 0; button < MAX_MOUSE_BUTTONS && !input; ++button)
    {
        input = MouseButtonPressed(button, -1) ? true : input;
    }

    return input;
}


void UInputMan::TrapMousePos(bool trap, int whichPlayer)
{ 
	if (whichPlayer == -1 || m_aControlScheme[whichPlayer].GetDevice() == DEVICE_MOUSE_KEYB) 
	{ 
		m_TrapMousePos = trap; 
	} 
	m_TrapMousePosPerPlayer[whichPlayer] = trap;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MouseUsedByPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports if any, and which player is using the mouse for control at
//                  this time.

int UInputMan::MouseUsedByPlayer() const
{
    if (m_aControlScheme[PLAYER_ONE].GetDevice() == DEVICE_MOUSE_KEYB)
        return PLAYER_ONE;
    else if (m_aControlScheme[PLAYER_TWO].GetDevice() == DEVICE_MOUSE_KEYB)
        return PLAYER_TWO;
    else if (m_aControlScheme[PLAYER_THREE].GetDevice() == DEVICE_MOUSE_KEYB)
        return PLAYER_THREE;
    else if (m_aControlScheme[PLAYER_FOUR].GetDevice() == DEVICE_MOUSE_KEYB)
        return PLAYER_FOUR;

    return PLAYER_NONE;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyJoyButtonPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any joystick button presses at all, but not
//                  d-pad input.

bool UInputMan::AnyJoyButtonPress(int whichJoy)
{
    bool input = false;

    for (int button = 0; button < MAX_JOY_BUTTONS && !input; ++button)
    {
        input = JoyButtonPressed(whichJoy, button) ? true : input;
    }

    return input;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMenuDirectional
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the generic direction input from any and all players which can
//                  affect a shared menu cursor. Normalized to 1.0 max

Vector UInputMan::GetMenuDirectional()
{
    Vector allInput;

	//m_LastDeviceWhichControlledGUICursor = 0;

    // Go through all players
    for (int player = 0; player < MAX_PLAYERS; ++player)
    {
        // Which Device is used by this player
        int device = m_aControlScheme[player].GetDevice();

        // Keyboard input
        if (device == DEVICE_KEYB_ONLY)
        {
            if (ElementHeld(player, INPUT_L_UP))
                allInput.m_Y += -1.0;
            else if (ElementHeld(player, INPUT_L_DOWN))
                allInput.m_Y += 1.0;
            if (ElementHeld(player, INPUT_L_LEFT))
                allInput.m_X += -1.0;
            else if (ElementHeld(player, INPUT_L_RIGHT))
                allInput.m_X += 1.0;
        }
        // Mouse player shouldn't be doing anything here, he should be using the mouse!
        else if (device == DEVICE_MOUSE_KEYB)
        {
            
        }
        // Analog enabled device (joystick, really)
        else if (device >= DEVICE_GAMEPAD_1)
        {
			if (AnalogMoveValues(player).GetLargest() > 0.05)
			{
				allInput += AnalogMoveValues(player);
				m_LastDeviceWhichControlledGUICursor = device;
			}
			if (AnalogAimValues(player).GetLargest() > 0.05)
			{
				allInput += AnalogAimValues(player);
				m_LastDeviceWhichControlledGUICursor = device;
			}
        }
    }

    // Normalize/cap
    allInput.CapMagnitude(1.0);

    return allInput;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonPressed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor was pressed
//                  between previous update and this.

bool UInputMan::MenuButtonPressed(int whichButton)
{
    bool button = false;

    // Go through all players
    for (int player = 0; player < MAX_PLAYERS && !button; ++player)
    {
        // Which Device is used by this player
        int device = m_aControlScheme[player].GetDevice();

        // Check for primary/secondary button presses
		if (whichButton >= MENU_PRIMARY)
			button = ElementPressed(player, INPUT_FIRE) || MouseButtonPressed(MOUSE_LEFT, player) || button;
		if (whichButton >= MENU_SECONDARY)
			button = ElementPressed(player, INPUT_PIEMENU) || MouseButtonPressed(MOUSE_RIGHT, player) || button;

		if (button/* && m_aControlScheme[player].GetDevice() >= DEVICE_GAMEPAD_1*/)
			m_LastDeviceWhichControlledGUICursor = device;
    }

    return button;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonReleased
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor is held down.

bool UInputMan::MenuButtonReleased(int whichButton)
{
    bool button = false;

    // Go through all players
    for (int player = 0; player < MAX_PLAYERS && !button; ++player)
    {
        // Which Device is used by this player
        int device = m_aControlScheme[player].GetDevice();

        // Check for primary/secondary button presses
		if (whichButton >= MENU_PRIMARY)
			button = ElementReleased(player, INPUT_FIRE) || MouseButtonReleased(MOUSE_LEFT, player) || button;
        if (whichButton >= MENU_SECONDARY)
            button = ElementReleased(player, INPUT_PIEMENU) || MouseButtonReleased(MOUSE_RIGHT, player) || button;
    }

    return button;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MenuButtonHeld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether any generic button with the menu cursor is held down.

bool UInputMan::MenuButtonHeld(int whichButton)
{
    bool button = false;

    // Go through all players
    for (int player = 0; player < MAX_PLAYERS && !button; ++player)
    {
        // Which Device is used by this player
        int device = m_aControlScheme[player].GetDevice();

        // Check for primary/secondary button presses
		if (whichButton >= MENU_PRIMARY)
			button = ElementHeld(player, INPUT_FIRE) || MouseButtonHeld(MOUSE_LEFT, player) || button;
		if (whichButton >= MENU_SECONDARY)
			button = ElementHeld(player, INPUT_PIEMENU) || MouseButtonHeld(MOUSE_RIGHT, player) || button;

		if (button/* && m_aControlScheme[player].GetDevice() >= DEVICE_GAMEPAD_1*/)
		{
			m_LastDeviceWhichControlledGUICursor = device;
			break;
		}
    }

    return button;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any input at all, keyboard or buttons or d-pad.

bool UInputMan::AnyInput()
{
    // Check keyboard
    bool input = keypressed();

    // Check all joysticks
    if (!input)
        input = AnyJoyInput();

    return input;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any key, button, or d-pad presses at all.

bool UInputMan::AnyPress()
{
    bool pressed = false;

    // Check keyboard for presses
    for (int testKey = 0; testKey < KEY_MAX; ++testKey)
        pressed = s_aLastKeys[testKey] && s_aChangedKeys[testKey] ? true : pressed;

    // Check mouse buttons for presses
    if (!pressed)
        pressed = AnyMouseButtonPress();

    // Check all joysticks
    if (!pressed)
        pressed = AnyJoyPress();

    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyStartPress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return true if there is any start key/button presses at all.
//                  MUST call Update before calling this for it to work properly!

bool UInputMan::AnyStartPress()
{
    bool pressed = false;

    // Check keyboard for presses
    if (KeyPressed(KEY_ESC) || KeyPressed(KEY_SPACE))
        pressed = true;

    // Check all user bound start buttons
    for (int player = 0; player < MAX_PLAYERS && !pressed; ++player)
    {
        pressed = pressed || ElementPressed(player, INPUT_START);
        pressed = pressed || ElementPressed(player, INPUT_BACK);
    }

    return pressed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WaitForSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Halts thread and waits for space to be pressed. This is for debug
//                  purposes mostly.

void UInputMan::WaitForSpace()
{
/*
    if (m_DebugArmed) {
        while (KeyPressed(KEY_SPACE) != CDXKEY_PRESS)
            m_pInput->Update();

        m_pInput->Update();
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this UInputMan. Supposed to be done every frame.

int UInputMan::Update()
{
	m_LastDeviceWhichControlledGUICursor = 0;

    // Only update inputs on drawn frames
//    if (!g_TimerMan.DrawnSimUpdate())
//        return 0;

//    clear_keybuf();

    if (keyboard_needs_poll())
        poll_keyboard();

    if (mouse_needs_poll())
        poll_mouse();

    poll_joystick();

    // Detect and store key changes since last Update()
    for (int i = 0; i < KEY_MAX; ++i)
        s_aChangedKeys[i] = key[i] != s_aLastKeys[i];

    // Store mouse movement
    int mickeyX, mickeyY;
    get_mouse_mickeys(&mickeyX, &mickeyY);
    m_RawMouseMovement.SetXY(mickeyX, mickeyY);
/* Figured out that it was a acceleration setting in AllegroConfig.txt that caused this problem
    // Adjust for the wackiness that happens to the mickeys when 2x fullscreen screened
    if (g_FrameMan.IsFullscreen() && g_FrameMan.NxFullscreen() > 1)
    {
        m_RawMouseMovement *= 0.2 / g_FrameMan.NxFullscreen();
        m_RawMouseMovement.Ceiling();
    }
*/
// TODO: Have proper mouse sensitivity controls somewhere
    m_RawMouseMovement *= m_MouseSensitivity;

	// NETWORK SERVER: Apply mouse input received from client
	if (m_OverrideInput)
	{ 
		for (int p = 0; p < MAX_PLAYERS; p++)
		{
			if (m_NetworkAccumulatedRawMouseMovement[p].GetX() != 0 || m_NetworkAccumulatedRawMouseMovement[p].GetY() != 0)
			{
				Vector mouseMovement = m_NetworkAccumulatedRawMouseMovement[p];

				m_aNetworkAnalogMoveData[p].m_X += mouseMovement.m_X * 3;
				m_aNetworkAnalogMoveData[p].m_Y += mouseMovement.m_Y * 3;
				m_aNetworkAnalogMoveData[p].CapMagnitude(m_MouseTrapRadius);

				//m_NetworkAccumulatedRawMouseMovement[p].Reset();
			}

			// By the time we reach here all events should've been processed by recipients during the last update
			// Wee need to clear press and release event or otherwise it will look like player clicks buttons every frame till
			// the nex frame arrives after 33 ms with those states are off.

			// Clear pressed and released events as they should've been already processed during ActivityUpdate
			// It is vital that press and release events are processed just once or multiple events will be 
			// triggered on a single press
			for (int el = 0; el < INPUT_COUNT; el++)
			{
				m_aNetworkInputElementPressed[p][el] = false;
				m_aNetworkInputElementReleased[p][el] = false;
			}

			// Reset mouse movement
			m_NetworkAccumulatedRawMouseMovement[p].m_X = 0;
			m_NetworkAccumulatedRawMouseMovement[p].m_Y = 0;

			// Reset mouse button states to stop double clicking
			m_aNetworkMouseButtonPressedState[p][0] = false;
			m_aNetworkMouseButtonPressedState[p][1] = false;
			m_aNetworkMouseButtonPressedState[p][2] = false;

			m_aNetworkMouseButtonReleasedState[p][0] = false;
			m_aNetworkMouseButtonReleasedState[p][1] = false;
			m_aNetworkMouseButtonReleasedState[p][2] = false;

			// Reset mouse wheel state to stop overwheeling
			m_aNetworkMouseWheelState[p] = 0;
		}
	} 
	else
	{
		// This one is for client collecting mouse input
		m_NetworkAccumulatedRawMouseMovement[0] += m_RawMouseMovement;
	}

    // Detect and store mouse button input
    m_aMouseButtonState[MOUSE_LEFT] = mouse_b & 1;
    m_aMouseButtonState[MOUSE_RIGHT] = mouse_b & 2;
    m_aMouseButtonState[MOUSE_MIDDLE] = mouse_b & 4;

    // Detect changes in mouse button input
    m_aMouseChangedButtonState[MOUSE_LEFT] = m_aMouseButtonState[MOUSE_LEFT] != m_aMousePrevButtonState[MOUSE_LEFT];
    m_aMouseChangedButtonState[MOUSE_RIGHT] = m_aMouseButtonState[MOUSE_RIGHT] != m_aMousePrevButtonState[MOUSE_RIGHT];
    m_aMouseChangedButtonState[MOUSE_MIDDLE] = m_aMouseButtonState[MOUSE_MIDDLE] != m_aMousePrevButtonState[MOUSE_MIDDLE];

    // Detect and store mouse movement input, translated to analog stick emulation
    int mousePlayer;
    if ((mousePlayer = MouseUsedByPlayer()) != PLAYER_NONE)
    {
// TODO: temporary? Make framerate independent!
        // Mouse analog emulation input returns to 0;
//        m_AnalogMouseData *= 0.9;

// TODO: Make proper sensitivty setting and GUI controls
		m_AnalogMouseData.m_X += m_RawMouseMovement.m_X * 3;
		m_AnalogMouseData.m_Y += m_RawMouseMovement.m_Y * 3;
        // Cap the mouse input in a circle
        m_AnalogMouseData.CapMagnitude(m_MouseTrapRadius);

        // Only mess with the mouse pos if the original mouse position is not above the screen and may be grabbing the title bar of the game window
        if (!m_DisableMouseMoving)
        {
			if (!m_OverrideInput && !m_IsInMultiplayerMode)
			{
				// Trap the (invisible) mouse cursor in the middle of the screen, so it doens't fly out in windowed mode and some other window gets clicked
				if (m_TrapMousePos)
					position_mouse(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
				// The mouse cursor is visible and can move about the screen/window, but it shuold still be contained within the mouse player's part of the window
				else if (g_InActivity)
					ForceMouseWithinPlayerScreen(mousePlayer);
			}
        }

        // Mouse wheel update, translate motion into discrete ticks
        if (abs(mouse_z) >= 1)
        {
            // Save the direction
            m_MouseWheelChange = mouse_z;
            // Reset the position
            position_mouse_z(0);
        }
        // Or just leave as no change
        else
            m_MouseWheelChange = 0;

        // Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
        // won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
        float mouseDenominator = g_FrameMan.IsFullscreen() ? g_FrameMan.NxFullscreen() : g_FrameMan.NxWindowed();
        int mX = (float)mouse_x / mouseDenominator;
        int mY = (float)mouse_y / mouseDenominator;
        if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mX >= 0 && mX < g_FrameMan.GetResX() && mY >= 0 && mY < g_FrameMan.GetResY()))
            m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
    }

    // Detect and store joystick button changed since last Update()
    int joystick, button, stick, axis;
    for (joystick = 0; joystick < num_joysticks; ++joystick)
    {
        // Buttons
        for (button = 0; button < joy[joystick].num_buttons; ++button)
            s_aaChangedJoyState[joystick].button[button].b = joy[joystick].button[button].b != s_aaPrevJoyState[joystick].button[button].b;

		// Determine deadzone settings
		float deadzone = 0.0;
		int deadzonetype = UInputMan::DeadZoneType::CIRCLE;
		int player = -1;

		// Retreive deadzone settings from player's input scheme
		for (int p = 0; p < UInputMan::MAX_PLAYERS; p++)
		{
			int device = m_aControlScheme[p].GetDevice() - UInputMan::DEVICE_GAMEPAD_1;

			if (device == joystick)
			{
				deadzone = m_aControlScheme[p].GetJoystickDeadzone();
				deadzonetype = m_aControlScheme[p].GetJoystickDeadzoneType();
				player = p;
				break;
			}
		}


		// Disable input if it's in circle deadzone
		if (player > -1 && deadzonetype == UInputMan::DeadZoneType::CIRCLE && deadzone > 0.0)
		{
			InputScheme::InputMapping * pElement = m_aControlScheme[player].GetInputMappings();

			Vector aimValues;

			// Assume axes are stretched out over up-down, and left-right
			/*if (pElement[INPUT_R_LEFT].JoyDirMapped())
				if (joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED)
					aimValues.m_X = joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].pos - 128;
				else
					aimValues.m_X = joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].pos;

			if (pElement[INPUT_R_UP].JoyDirMapped())
				if (joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].flags & JOYFLAG_UNSIGNED)
					aimValues.m_Y = joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].pos - 128;
				else
					aimValues.m_Y = joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].pos;*/


			// Left stick
			if (pElement[INPUT_L_LEFT].JoyDirMapped())
				aimValues.m_X = AnalogAxisValue(joystick, pElement[INPUT_L_LEFT].GetStick(), pElement[INPUT_L_LEFT].GetAxis());
			if (pElement[INPUT_L_UP].JoyDirMapped())
				aimValues.m_Y = AnalogAxisValue(joystick, pElement[INPUT_L_UP].GetStick(), pElement[INPUT_L_UP].GetAxis());

			if (aimValues.GetMagnitude() < deadzone * 2)
			{
				if (pElement[INPUT_L_LEFT].JoyDirMapped())
				{
					if (joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED)
					{
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].pos = 128;
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].d2 = 0;
					}
					else {
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].pos = 0;
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_L_LEFT].GetStick()].axis[pElement[INPUT_L_LEFT].GetAxis()].d2 = 0;
					}
				}

				if (pElement[INPUT_L_UP].JoyDirMapped())
				{
					if (joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].flags & JOYFLAG_UNSIGNED)
					{
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].pos = 128;
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].d2 = 0;
					}
					else {
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].pos = 0;
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_L_UP].GetStick()].axis[pElement[INPUT_L_UP].GetAxis()].d2 = 0;
					}
				}
			}

			aimValues.SetX(0);
			aimValues.SetY(0);

			// Right stick
			if (pElement[INPUT_R_LEFT].JoyDirMapped())
				aimValues.m_X = AnalogAxisValue(joystick, pElement[INPUT_R_LEFT].GetStick(), pElement[INPUT_R_LEFT].GetAxis());
			if (pElement[INPUT_R_UP].JoyDirMapped())
				aimValues.m_Y = AnalogAxisValue(joystick, pElement[INPUT_R_UP].GetStick(), pElement[INPUT_R_UP].GetAxis());

			if (aimValues.GetMagnitude() < deadzone * 2)
			{
				if (pElement[INPUT_R_LEFT].JoyDirMapped())
				{
					if (joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].flags & JOYFLAG_UNSIGNED)
					{
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].pos = 128;
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].d2 = 0;
					}
					else {
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].pos = 0;
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_R_LEFT].GetStick()].axis[pElement[INPUT_R_LEFT].GetAxis()].d2 = 0;
					}
				}

				if (pElement[INPUT_R_UP].JoyDirMapped())
				{
					if (joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].flags & JOYFLAG_UNSIGNED)
					{
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].pos = 128;
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].d2 = 0;
					}
					else {
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].pos = 0;
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].d1 = 0;
						joy[joystick].stick[pElement[INPUT_R_UP].GetStick()].axis[pElement[INPUT_R_UP].GetAxis()].d2 = 0;
					}
				}
			}
		}

        // Stick, axis, directions
        for (stick = 0; stick < joy[joystick].num_sticks; ++stick)
        {
            for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis)
            {

				// Adjust joystick values to eliminate values in deadzone
				if (player > -1 && deadzonetype == UInputMan::DeadZoneType::SQUARE && deadzone > 0.0)
				{
					// !!! REFERENCE CODE DO NOT UNCOMMENT !!!
					// Taken from AnalogAxiesValue
					// Treat unsigned (throttle axes) as rudders, with a range of 0-255 and midpoint of 128
					//if (joy[whichJoy].stick[whichStick].flags & JOYFLAG_UNSIGNED)
					//	return (float)(joy[whichJoy].stick[whichStick].axis[whichAxis].pos - 128) / (float)128;
					// Regular signed axis with range of -128 to 128
					//else
					//	return (float)joy[whichJoy].stick[whichStick].axis[whichAxis].pos / (float)128;

					// This one heavily relies on AnalogAxiesValue method of processing joystick data. Code above for the reference
					if (joy[joystick].stick[stick].flags & JOYFLAG_UNSIGNED)
					{
						if (abs(joy[joystick].stick[stick].axis[axis].pos - 128) > 0.0 && abs(joy[joystick].stick[stick].axis[axis].pos - 128) / (float)128 < deadzone)
						{
							joy[joystick].stick[stick].axis[axis].pos = 128;
							joy[joystick].stick[stick].axis[axis].d1 = 0;
							joy[joystick].stick[stick].axis[axis].d2 = 0;
						}

					}
					else {
						if (abs(joy[joystick].stick[stick].axis[axis].pos) > 0 && abs(joy[joystick].stick[stick].axis[axis].pos) / (float)128 < deadzone)
						{
							joy[joystick].stick[stick].axis[axis].pos = 0;
							joy[joystick].stick[stick].axis[axis].d1 = 0;
							joy[joystick].stick[stick].axis[axis].d2 = 0;
						}
					}
				}

                s_aaChangedJoyState[joystick].stick[stick].axis[axis].d1 = joy[joystick].stick[stick].axis[axis].d1 != s_aaPrevJoyState[joystick].stick[stick].axis[axis].d1;
                s_aaChangedJoyState[joystick].stick[stick].axis[axis].d2 = joy[joystick].stick[stick].axis[axis].d2 != s_aaPrevJoyState[joystick].stick[stick].axis[axis].d2;
            }
        }
    }

    // If Escape is pressed, go to the mainmenu or close the app
    if (KeyPressed(KEY_ESC))
    {
// TODO: Make this more robust and purty!")
        // If in the game pause and exit to menu on esc
        if (g_InActivity)
        {
            g_ActivityMan.PauseActivity();
            g_InActivity = false;
        }
        // Do nothing if the intro is playing, has own handling
// Now handled in MainMenuGUI
//        else if (g_IntroState <= 0 || g_IntroState >= 17)
//            g_Quit = true;
    }
/*
    if (KeyPressed(KEY_Q)) {
        m_DebugArmed = true;
    }
*/
    // Reset RTE if back is pressed
    if (g_InActivity)
    {
        // Ctrl-R resets
        if ((key_shifts & KB_CTRL_FLAG) && KeyPressed(KEY_R))
        {
            g_ResetActivity = true;
        }

        // Check for resets and start button presses on controllers of all active players
        if (g_ActivityMan.GetActivity())
        {
            for (int player = PLAYER_ONE; player < MAX_PLAYERS; ++player)
            {
                if (g_ActivityMan.GetActivity()->PlayerActive(player))
                {
                    g_ResetActivity = g_ResetActivity || ElementPressed(PLAYER_ONE, INPUT_BACK);

                    if (ElementPressed(player, INPUT_START))
                    {
                        g_ActivityMan.PauseActivity();
                        g_InActivity = false;
                    }
                }
            }
        }

        if (g_ResetActivity)
            g_ConsoleMan.PrintString("SYSTEM: Activity was reset!");
    }

    // Toggle fullscreen / windowed
    if (key_shifts & KB_ALT_FLAG && KeyPressed(KEY_ENTER)) {
        g_FrameMan.ToggleFullscreen();
    }

    // Only allow performance tweaking if showing the stats
    if (g_FrameMan.IsShowingPerformanceStats())
    {
        // Manipulate timescaling
        if (KeyHeld(KEY_2))
            g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() + 0.01);
        if (KeyHeld(KEY_1) && g_TimerMan.GetTimeScale() > 0.01)
            g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() - 0.01);

        // Increase real to sim cap
        if (KeyHeld(KEY_4))
            g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() + 0.001);
        // Decrease frame delay
        if (KeyHeld(KEY_3) && g_TimerMan.GetRealToSimCap() > 0)
            g_TimerMan.SetRealToSimCap(g_TimerMan.GetRealToSimCap() - 0.001);
        // Manipulate deltatime
        if (KeyHeld(KEY_6))
            g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() + 0.001);
        // Decrease frame delay
        if (KeyHeld(KEY_5) && g_TimerMan.GetDeltaTimeSecs() > 0)
            g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() - 0.001);
    }

    // Screendump - Ctrl+S
    if ((((key_shifts & KB_CTRL_FLAG)/* && (key_shifts & KB_SHIFT_FLAG)*/ && KeyHeld(KEY_S)) || KeyHeld(KEY_PRTSCR)))// && g_TimerMan.DrawnSimUpdate())
    {
        g_FrameMan.SaveScreenToBMP("ScreenDump");
// TEMP!!
//        g_FrameMan.SaveBitmapToBMP(g_SceneMan.GetTerrain()->GetBGColorBitmap(), "SceneBG");
    }

	// Dump entire map, Ctrl+W
    if ((key_shifts & KB_CTRL_FLAG) && KeyHeld(KEY_W))
    {
        g_FrameMan.SaveWorldToBMP("WorldDump");
    }

    // Material draw toggle, Ctrl + M
    if ((key_shifts & KB_CTRL_FLAG) && KeyPressed(KEY_M))
        g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3);

    // Perf stats display toggle
    if ((key_shifts & KB_CTRL_FLAG) && KeyPressed(KEY_P))
        g_FrameMan.ShowPerformanceStats(!g_FrameMan.IsShowingPerformanceStats());

    // Force one sim update per graphics frame
    if ((key_shifts & KB_CTRL_FLAG) && KeyPressed(KEY_O))
        g_TimerMan.SetOneSimUpdatePerFrame(!g_TimerMan.IsOneSimUpdatePerFrame());

	// Dump all shortcuts to console window
	if (KeyPressed(KEY_F1))
	{
		if (!g_ConsoleMan.IsEnabled())
			g_ConsoleMan.SetEnabled();

		g_ConsoleMan.PrintString("--- SHORTCUTS ---");
		g_ConsoleMan.PrintString("~ - This console. CTRL + ~ - Console without input capture");
		g_ConsoleMan.PrintString("CTRL + S / Prnt Scrn - Make a screenshot");
		g_ConsoleMan.PrintString("CTRL + W - Make a screenshot of the entire level");
		g_ConsoleMan.PrintString("CTRL + P - Show performance stats");
		g_ConsoleMan.PrintString("CTRL + R - Reset activity");
		g_ConsoleMan.PrintString("CTRL + M - Switch display mode: Draw -> Material -> MO");
		g_ConsoleMan.PrintString("CTRL + O - Toggle one sim update per frame");

		g_ConsoleMan.PrintString("F2 - Reload all scripts");
		g_ConsoleMan.PrintString("F3 - Save Console.log");
		g_ConsoleMan.PrintString("F4 - Save input log");
		g_ConsoleMan.PrintString("F5 - Clear console log ");
	}

	if (KeyPressed(KEY_F2))
	{
		g_PresetMan.ReloadAllScripts();
		g_ConsoleMan.PrintString("Scripts reloaded");
	}

	if (KeyPressed(KEY_F3))
	{
		g_ConsoleMan.SaveAllText("Console.dump.log");
	}

	if (KeyPressed(KEY_F4))
	{
		g_ConsoleMan.SaveInputLog("Console.input.log");
	}

	if (KeyPressed(KEY_F5))
	{
		g_ConsoleMan.ClearLog();
	}

    ///////////////////////////////////////////////////////////
    // Scrolling test
/*
    if (KeyHeld(KEY_LEFT))
        g_TempXOff--;
    if (KeyHeld(KEY_RIGHT))
        g_TempXOff++;
    if (KeyHeld(KEY_DOWN))
        g_TempYOff++;
    if (KeyHeld(KEY_UP))
        g_TempYOff--;
*/
    // Save the current state of the keyboard so that we can compare it
    // next frame and see which key states have been changed in the mean time.
    memcpy(s_aLastKeys, const_cast<const char *>(key), KEY_MAX);

    // Save the mouse button states so taht we can compare it next frame and see which buttons have changed.
    for (int mbutton = 0; mbutton < MAX_MOUSE_BUTTONS; ++mbutton)
    {
        m_aMousePrevButtonState[mbutton] = m_aMouseButtonState[mbutton];
    }

    // Save the current state of the joysticks
    for (joystick = 0; joystick < num_joysticks; ++joystick)
    {
        // Buttons
        for (button = 0; button < joy[joystick].num_buttons; ++button)
            s_aaPrevJoyState[joystick].button[button].b = joy[joystick].button[button].b;

        // Stick, axis, directions
        for (stick = 0; stick < joy[joystick].num_sticks; ++stick)
        {
            for (axis = 0; axis < joy[joystick].stick[stick].num_axis; ++axis)
            {
                s_aaPrevJoyState[joystick].stick[stick].axis[axis].d1 = joy[joystick].stick[stick].axis[axis].d1;
                s_aaPrevJoyState[joystick].stick[stick].axis[axis].d2 = joy[joystick].stick[stick].axis[axis].d2;
            }
        }
    }


	// Store pressed and released events to be picked by NetworkClient during it's on-timer update
	for (int element = 0; element < INPUT_COUNT; element++)
	{
		// Only store press and release events if they happened, Client will clear those after update so we don't care about false
		if (ElementPressed(0, element))
			m_aNetworkAccumulatedElementPressed[element] = true;
		if (ElementReleased(0, element))
			m_aNetworkAccumulatedElementReleased[element] = true;
	}

    return 0;
}


void UInputMan::ClearAccumulatedStates()
{
	for (int element = 0; element < INPUT_COUNT; element++)
	{
		m_aNetworkAccumulatedElementPressed[element] = false;
		m_aNetworkAccumulatedElementReleased[element] = false;
	}
}

bool UInputMan::AccumulatedElementPressed(int element)
{
	if (element < 0 || element >= INPUT_COUNT)
		return false;

	return m_aNetworkAccumulatedElementPressed[element];
}


bool UInputMan::AccumulatedElementReleased(int element)
{
	if (element < 0 || element >= INPUT_COUNT)
		return false;

	return m_aNetworkAccumulatedElementReleased[element];
}




} // namespace RTE
