#ifndef RTEGAMEPAD_H
#define RTEGAMEPAD_H
#include "SDL2/SDL_gamecontroller.h"
#include "Vector.h"

namespace RTE {
	struct Gamepad {
	public:
		Gamepad() = default;
		Gamepad(SDL_JoystickID id, size_t numAxis, size_t numButtons):m_JoystickID{id}, m_Axis(numAxis),m_Buttons(numButtons) {}
		~Gamepad() = default;

		SDL_JoystickID m_JoystickID;
		std::vector<Vector> m_Axis;
		std::vector<bool> m_Buttons;

		bool operator==(const Gamepad& rhs) {
			return m_JoystickID == rhs.m_JoystickID;
		}
		bool operator<(const Gamepad& rhs) {
			return m_JoystickID < rhs.m_JoystickID;
		}
	};
}

#endif
