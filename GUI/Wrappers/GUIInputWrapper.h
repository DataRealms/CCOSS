#ifndef _GUIINPUTWRAPPER_
#define _GUIINPUTWRAPPER_

#include "GUIInput.h"
#include "SDL_scancode.h"

namespace RTE {

	class Timer;

	/// <summary>
	/// Wrapper class to translate input handling of whatever library is currently used to valid GUI library input.
	/// </summary>
	class GUIInputWrapper : public GUIInput {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GUIInputWrapper object in system memory.
		/// </summary>
		/// <param name="whichPlayer">Which player this GUIInputWrapper will handle input for. -1 means no specific player and will default to player 1.</param>
		/// <param name="keyJoyMouseCursor">Whether the keyboard and joysticks also can control the mouse cursor.</param>
		GUIInputWrapper(int whichPlayer, bool keyJoyMouseCursor = false);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GUIInputWrapper object before deletion from system memory.
		/// </summary>
		~GUIInputWrapper() override = default;
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the input.
		/// </summary>
		void Update() override;
#pragma endregion

	private:

		const float m_KeyRepeatDelay = 0.300F; //!< The delay a key needs to be held to be considered a repeating input.
		std::array<float, GUIInput::Constants::KEYBOARD_BUFFER_SIZE> m_KeyHoldDuration; //!< How long each key has been held in order to set repeating inputs.

		std::unique_ptr<Timer> m_KeyTimer; //!< Timer for checking key hold duration.
		std::unique_ptr<Timer> m_CursorAccelTimer; //!< Timer to calculate the mouse cursor acceleration when it is controller with the keyboard or joysticks.

		/// <summary>
		/// Converts from SDL's key push to that used by this GUI lib, with timings for repeats taken into consideration.
		/// </summary>
		/// <param name="sdlKey">The key scancode.</param>
		/// <param name="guilibKey">The corresponding GUIlib scancode</param>
		/// <param name="elapsedS">The elapsed time since the last update, in seconds.</param>
		void ConvertKeyEvent(SDL_Scancode sdlKey, int guilibKey, float elapsedS);

#pragma region Update Breakdown
		/// <summary>
		/// Updates the keyboard input.
		/// </summary>
		void UpdateKeyboardInput(float keyElapsedTime);

		/// <summary>
		/// Updates the mouse input.
		/// </summary>
		void UpdateMouseInput();

		/// <summary>
		/// Updates the mouse input using the joystick or keyboard.
		/// </summary>
		void UpdateKeyJoyMouseInput(float keyElapsedTime);
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIInputWrapper(const GUIInputWrapper &reference) = delete;
		GUIInputWrapper & operator=(const GUIInputWrapper &rhs) = delete;
	};
};
#endif
