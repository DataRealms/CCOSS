#ifndef _ALLEGROINPUT_
#define _ALLEGROINPUT_

#include "GUIInput.h"

namespace RTE {

	class Timer;

	/// <summary>
	/// Wrapper class to translate Allegro input to valid GUI library input.
	/// </summary>
	class AllegroInput : public GUIInput {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a AllegroInput object in system memory.
		/// </summary>
		/// <param name="whichPlayer">Which player this AllegroInput will handle input for. -1 means no specific player and will default to player 1.</param>
		/// <param name="keyJoyMouseCursor">Whether the keyboard and joysticks also can control the mouse cursor.</param>
		AllegroInput(int whichPlayer, bool keyJoyMouseCursor = false);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Changes the mouse movement speed based on the graphics driver in use. The default (2) movement speed is slow and sluggish in dedicated fullscreen so this is used to change between default and faster (1) when switching drivers.
		/// </summary>
		/// <param name="graphicsDriver">The graphics driver to adjust mouse movement speed to.</param>
		void AdjustMouseMovementSpeedToGraphicsDriver(int graphicsDriver) const;
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the input.
		/// </summary>
		void Update() override;
#pragma endregion

	private:

		/// <summary>
		/// Enumeration containing the codes in the Allegro mouse button state.
		/// </summary>
		enum AllegroMouseButtons {
			ButtonLeft = 1,
			ButtonRight = 2,
			ButtonMiddle = 4
		};

		const float m_KeyRepeatDelay = 0.300F; //!< The delay a key needs to be held to be considered a repeating input.
		std::array<float, 256> m_KeyHoldDuration; //!< How long each key has been held in order to set repeating inputs.

		std::unique_ptr<Timer> m_KeyTimer; //!< Timer for checking key hold duration.
		std::unique_ptr<Timer> m_CursorAccelTimer; //!< Timer to calculate the mouse cursor acceleration when it is controller with the keyboard or joysticks.

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

		/// <summary>
		/// Converts from Allegro's key push to that used by this GUI lib, with timings for repeats taken into consideration.
		/// </summary>
		/// <param name="allegroKey">The Allegro scancode.</param>
		/// <param name="guilibKey">The corresponding GUIlib scancode</param>
		/// <param name="elapsedS">The elapsed time since the last update, in seconds.</param>
		void ConvertKeyEvent(int allegroKey, int guilibKey, float elapsedS);

		// Disallow the use of some implicit methods.
		AllegroInput(const AllegroInput &reference) = delete;
		AllegroInput & operator=(const AllegroInput &rhs) = delete;
	};
};
#endif