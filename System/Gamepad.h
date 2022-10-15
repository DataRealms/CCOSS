#ifndef RTEGAMEPAD_H
#define RTEGAMEPAD_H
#include "SDL2/SDL_gamecontroller.h"
#include "Vector.h"

namespace RTE {
	struct Gamepad {
	public:
		Gamepad():m_DeviceIndex(-1), m_JoystickID(-1) {};
		Gamepad(int deviceIndex, SDL_JoystickID id, size_t numAxis, size_t numButtons) :
		    m_DeviceIndex(deviceIndex), m_JoystickID{id}, m_Axis(numAxis), m_DigitalAxis(numAxis), m_Buttons(numButtons) {}
		~Gamepad() = default;

		int m_DeviceIndex;
		SDL_JoystickID m_JoystickID;
		std::vector<int> m_Axis;
		std::vector<int> m_DigitalAxis;
		std::vector<bool> m_Buttons;

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
