#ifndef _RTEWINDOWMAN_
#define _RTEWINDOWMAN_

#include "Singleton.h"
#include "glm/glm.hpp"

#define g_WindowMan WindowMan::Instance()

extern "C" {
	struct SDL_Window;
}

namespace RTE {

	struct SDLWindowDeleter {
		void operator() (SDL_Window *window) const;
	};

	/// <summary>
	///
	/// </summary>
	class WindowMan : public Singleton<WindowMan> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a WindowMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		WindowMan() { Clear(); }

		/// <summary>
		/// Makes the WindowMan object ready for use, which is to be used with SettingsMan first.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure.</returns>
		bool Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a WindowMan object before deletion from system memory.
		/// </summary>
		~WindowMan() = default;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Returns the main window pointer. OWNERSHIP IS NOT TRANSFERRED!.
		/// </summary>
		/// <returns>The pointer to the main window.</returns>
		SDL_Window * GetWindow() const { return m_Window.get(); }

		/// <summary>
		/// Gets whether VSync is enabled.
		/// </summary>
		/// <returns>Whether VSync is enabled.</returns>
		bool VSyncEnabled() const { return m_EnableVSync; }


#pragma endregion

#pragma region Display Switch Handling
		/// <summary>
		/// Callback function for the Allegro set_display_switch_callback. It will be called when focus is switched away from the game window.
		/// It will temporarily disable positioning of the mouse so that when focus is switched back to the game window, the game window won't fly away because the user clicked the title bar of the window.
		/// </summary>
		void DisplaySwitchOut();

		/// <summary>
		/// Callback function for the Allegro set_display_switch_callback. It will be called when focus is switched back to the game window.
		/// </summary>
		void DisplaySwitchIn();
#pragma endregion

#pragma region Concrete Methods



#pragma endregion





		/// <summary>
		/// Checks whether the passed in resolution settings make sense. If not, overrides them to prevent crashes or unexpected behavior. This is called during Initialize().
		/// </summary>
		/// <param name="resX">Game window width to check.</param>
		/// <param name="resY">Game window height to check.</param>
		/// <param name="resMultiplier">Game window resolution multiplier to check.</param>
		void ValidateResolution(int &resX, int &resY, int &resMultiplier, bool &newFullscreen) const;

		/// <summary>
		/// Checks whether the passed in multi-screen resolution settings make sense. If not, overrides them to prevent crashes or unexpected behavior. This is called during ValidateResolution().
		/// </summary>
		/// <param name="resX">Game window width to check.</param>
		/// <param name="resY">Game window height to check.</param>
		/// <param name="resMultiplier">Game window resolution multiplier to check.</param>
		/// <returns>Whether multi-screen validation performed any overrides to settings.</returns>
		bool ValidateMultiScreenResolution(int &resX, int &resY, int resMultiplier) const;





		/// <summary>
/// Gets the graphics driver that is used for rendering.
/// </summary>
/// <returns>The graphics driver that is used for rendering.</returns>
		int GetGraphicsDriver() const { return m_Fullscreen; }

		/// <summary>
		/// Gets the maximum horizontal resolution the game window can be (desktop width).
		/// </summary>
		/// <returns>The maximum horizontal resolution the game window can be (desktop width).</returns>
		int GetMaxResX() const { return m_MaxResX; }

		/// <summary>
		/// Gets the maximum vertical resolution the game window can be (desktop height).
		/// </summary>
		/// <returns>The maximum vertical resolution the game window can be (desktop height).</returns>
		int GetMaxResY() const { return m_MaxResY; }

		/// <summary>
		/// Gets the horizontal resolution of the primary display.
		/// </summary>
		/// <returns>The horizontal resolution of the primary display.</returns>
		int GetPrimaryScreenResX() const { return m_PrimaryScreenResX; }

		/// <summary>
		/// Gets the vertical resolution of the primary display.
		/// </summary>
		/// <returns>The vertical resolution of the primary display.</returns>
		int GetPrimaryScreenResY() const { return m_PrimaryScreenResY; }

		/// <summary>
		/// Gets the horizontal resolution of the screen.
		/// </summary>
		/// <returns>An int describing the horizontal resolution of the screen in pixels.</returns>
		int GetResX() const { return m_ResX; }

		/// <summary>
		/// Gets the vertical resolution of the screen.
		/// </summary>
		/// <returns>An int describing the vertical resolution of the screen in pixels.</returns>
		int GetResY() const { return m_ResY; }

		/// <summary>
		/// Gets how many times the screen resolution is being multiplied and the backbuffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the screen resolution is run in.</returns>
		int GetResMultiplier() const { return m_ResMultiplier; }

		/// <summary>
		/// Gets whether the game window resolution was changed.
		/// </summary>
		/// <returns>Whether the game window resolution was changed.</returns>
		bool ResolutionChanged() const { return m_ResChanged; }

		/// <summary>
		/// Checks whether the game window is currently in fullscreen mode.
		/// </summary>
		/// <returns>Whether the window is fullscreen.</returns>
		bool IsWindowFullscreen() const { return m_Fullscreen; }





		/// <summary>
		/// Sets and switches to a new windowed mode resolution multiplier.
		/// </summary>
		/// <param name="newMultiplier">The multiplier to switch to.</param>
		void ChangeResolutionMultiplier(int newMultiplier = 1);

		/// <summary>
		/// Switches the game window resolution to the specified dimensions.
		/// </summary>
		/// <param name="newResX">New width to set window to.</param>
		/// <param name="newResY">New height to set window to.</param>
		/// <param name="upscaled">Whether the new resolution is upscaled.</param>
		/// <param name="newFullscreen">Whether to put the game into fullscreen mode.</param>
		void ChangeResolution(int newResX, int newResY, bool upscaled, bool newFullscreen);

		/// <summary>
		/// Apply resolution change after window resize.
		/// </summary>
		/// <param name="newResX">The new horizontal resolution.</param>
		/// <param name="newResY">The new vertical resolution.</param>
		void WindowResizedCallback(int newResX, int newResY);



		/// <summary>
		/// Resize the window to enable fullscreen on multiple monitors. This will fill as many screens as necessary to fulfill resX*resY*resMultiplier resolution.
		/// </summary>
		/// <param name="resX">Requested horizontal resolution (not including scaling). May be adjusted to avoid letterboxing.</param>
		/// <param name="resY">Requested vertical resolution (not including scaling). May be adjusted to avoid letterboxing.</param>
		/// <param name="resMultiplier">Requested resolution multiplier.</param>
		/// <returns>Whether all displays were created successfully.</returns>
		bool SetWindowMultiFullscreen(int &resX, int &resY, int resMultiplier);


		/// <summary>
		/// Get a GL viewport with letterboxing for a window resolutions with different aspect ratio.
		/// </summary>
		/// <param name="resX">Desired horizontal resolution.</param>
		/// <param name="resY">Desired vertical resolution.</param>
		/// <param name="windowW">Window horizontal resolution.</param>
		/// <param name="windowH">Window vertical resolution.</param>
		/// <returns></returns>
		glm::vec4 GetViewportLetterbox(int resX, int resY, int windowW, int windowH);




	private:

		std::unique_ptr<SDL_Window, SDLWindowDeleter> m_Window; //!< The main Window.
		std::vector<std::unique_ptr<SDL_Window, SDLWindowDeleter>> m_MultiWindows; //!< Additional windows for multi display fullscreen.

		int m_NumScreens; //!< Number of physical screens.
		int m_MaxResX; //!< Width of the primary or all physical screens combined if more than one available (desktop resolution).
		int m_MaxResY; //!< Height of the primary or tallest screen if more than one available (desktop resolution).
		int m_PrimaryScreenResX; //!< Width of the primary physical screen only.
		int m_PrimaryScreenResY; //!< Height of the primary physical screen only.

		int m_ResX; //!< Game window width.
		int m_ResY; //!< Game window height.
		int m_ResMultiplier; //!< The number of times the game window and image should be multiplied and stretched across for better visibility.

		bool m_EnableVSync; //!<
		bool m_Fullscreen; //!< The graphics driver that will be used for rendering.
		bool m_DisableMultiScreenResolutionValidation; //!< Whether to disable resolution validation when running multi-screen mode or not. Allows setting whatever crazy resolution that may or may not crash.

		bool m_ResChanged; //!< Whether the resolution was changed through the settings fullscreen/upscaled fullscreen buttons.

#pragma region



#pragma endregion

		/// <summary>
		/// Clears all the member variables of this WindowMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		WindowMan(const WindowMan &reference) = delete;
		WindowMan & operator=(const WindowMan &rhs) = delete;
	};
}
#endif