#ifndef _RTEGAMEPAD_
#define _RTEGAMEPAD_

#include "Vector.h"
#include "SDL_gamecontroller.h"

namespace RTE {

	/// <summary>
	/// Structure for storing SDL_GameController or SDL_Joystick states.
	/// </summary>
	struct Gamepad {
		int m_DeviceIndex = -1; //!< The SDL device index, used when reopening devices as SDL will attempt to reassign joysticks to the same index.
		SDL_JoystickID m_JoystickID = -1; //!< The joystick ID for event handling.
		std::vector<int> m_Axis; //!< Array of analog axis states.
		std::vector<int> m_DigitalAxis; //!< Array of digital axis states. Should be updated when analog axis crosses half value 8192.
		std::vector<bool> m_Buttons; //!< Array of button states.

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Gamepad object in system memory and make it ready for use.
		/// </summary>
		Gamepad() = default;

		/// <summary>
		/// Constructor method used to instantiate a Gamepad object in system memory and make it ready for use.
		/// </summary>
		/// <param name="deviceIndex">The SDL device index.</param>
		/// <param name="id">The joystick ID for event handling.</param>
		/// <param name="numAxis">Number of analog axis.</param>
		/// <param name="numButtons">Number of buttons.</param>
		Gamepad(int deviceIndex, SDL_JoystickID id, int numAxis, int numButtons) : m_DeviceIndex(deviceIndex), m_JoystickID(id), m_Axis(numAxis), m_DigitalAxis(numAxis), m_Buttons(numButtons) {}
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// Equality operator for testing if any two Gamepads are equal by ID.
		/// </summary>
		/// <param name="joystickID">The ID to check equality with.</param>
		/// <returns>A boolean indicating whether the two operands are equal or not.</returns>
		bool operator==(int joystickID) const { return m_JoystickID == joystickID; }

		/// <summary>
		/// Equality operator for testing if any two Gamepads are equal.
		/// </summary>
		/// <param name="rhs">A Gamepad reference as the right hand side operand.</param>
		/// <returns>A boolean indicating whether the two operands are equal or not.</returns>
		bool operator==(const Gamepad &rhs) const { return m_JoystickID == rhs.m_JoystickID; }

		/// <summary>
		/// Comparison operator for sorting Gamepads by ID.
		/// </summary>
		/// <param name="rhs">A Gamepad reference as the right hand side operand.</param>
		/// <returns>A boolean indicating the comparison result.</returns>
		bool operator<(const Gamepad &rhs) const { return m_JoystickID < rhs.m_JoystickID; }
	};
}
#endif