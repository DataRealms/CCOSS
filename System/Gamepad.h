#ifndef RTEGAMEPAD_H
#define RTEGAMEPAD_H
#include "SDL_gamecontroller.h"
#include "Vector.h"

namespace RTE {
	/// <summary>
	/// Structure for storing SDL_GameController or SDL_Joystick states.
	/// </summary>
	struct Gamepad {
	public:
		Gamepad():m_DeviceIndex(-1), m_JoystickID(-1), m_Axis{}, m_DigitalAxis{}, m_Buttons{} {};
		Gamepad(int deviceIndex, SDL_JoystickID id, size_t numAxis, size_t numButtons) :
		    m_DeviceIndex(deviceIndex), m_JoystickID{id}, m_Axis(numAxis), m_DigitalAxis(numAxis), m_Buttons(numButtons) {}
		~Gamepad() = default;

		int m_DeviceIndex; //!< the SDL device index, used when reopening devices as SDL will attempt to reassign joysticks to the same index
		SDL_JoystickID m_JoystickID; //!< The joystick ID for event handling
		std::vector<int> m_Axis; //!< Array of analog axis states
		std::vector<int> m_DigitalAxis; //!< Array of digital axis states. Should be updated when analog axis crosses half value 8192.
		std::vector<bool> m_Buttons; //!< Array of button states.

		bool operator==(int joystickID) {
			return m_JoystickID == joystickID;
		}
		bool operator==(const Gamepad &rhs) {
			return m_JoystickID == rhs.m_JoystickID;
		}
		bool operator<(const Gamepad &rhs) {
			return m_JoystickID < rhs.m_JoystickID;
		}
	};
} // namespace RTE

#endif
