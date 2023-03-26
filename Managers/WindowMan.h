#ifndef _RTEWINDOWMAN_
#define _RTEWINDOWMAN_

#include "Singleton.h"
#include "glm/glm.hpp"

#define g_WindowMan WindowMan::Instance()

extern "C" {
	struct SDL_Window;
	struct SDL_Renderer;
	struct SDL_Texture;
	struct SDL_Rect;
	union SDL_Event;
}

namespace RTE {

	struct SDLWindowDeleter {
		void operator() (SDL_Window *window) const;
	};

	struct SDLRendererDeleter {
		void operator()(SDL_Renderer *renderer) const;
	};

	struct SDLTextureDeleter {
		void operator()(SDL_Texture *texture) const;
	};

	/// <summary>
	/// The singleton manager over the game window and display of frames.
	/// </summary>
	class WindowMan : public Singleton<WindowMan> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a WindowMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		WindowMan();

		/// <summary>
		/// Makes the WindowMan object ready for use.
		/// </summary>
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a WindowMan object before deletion from system memory.
		/// </summary>
		~WindowMan() = default;
#pragma endregion

#pragma region Display Getters and Setters
		/// <summary>
		/// Gets the horizontal resolution of the display the primary window is positioned at.
		/// </summary>
		/// <returns>The horizontal resolution of the display the primary window is positioned at.</returns>
		int GetDisplayPrimaryWindowIsAtResX() const { return m_DisplayPrimaryWindowIsAtResX; }

		/// <summary>
		/// Gets the vertical resolution of the display the primary window is positioned at.
		/// </summary>
		/// <returns>The vertical resolution of the display the primary window is positioned at.</returns>
		int GetDisplayPrimaryWindowIsAtResY() const { return m_DisplayPrimaryWindowIsAtResY; }

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
#pragma endregion

#pragma region Game Window Getters and Setters
		/// <summary>
		/// Gets the primary game window pointer. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>Pointer to the primary game window.</returns>
		SDL_Window * GetPrimaryWindow() const { return m_PrimaryWindow.get(); }

		/// <summary>
		/// Gets whether any of the game windows is currently in focus.
		/// </summary>
		/// <returns>Whether any of the game windows is currently in focus.</returns>
		bool AnyWindowHasFocus() const { return m_AnyWindowHasFocus; }

		/// <summary>
		/// Gets the current horizontal resolution of the game window.
		/// </summary>
		/// <returns>The current horizontal resolution of the window, in pixels.</returns>
		int GetResX() const { return m_ResX; }

		/// <summary>
		/// Gets the current vertical resolution of the game window.
		/// </summary>
		/// <returns>The current vertical resolution of the window, in pixels.</returns>
		int GetResY() const { return m_ResY; }

		/// <summary>
		/// Gets how many times the window resolution is being multiplied and the backbuffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the window resolution is run in.</returns>
		int GetResMultiplier() const { return m_ResMultiplier; }

		/// <summary>
		/// Gets whether the game resolution was changed.
		/// </summary>
		/// <returns>Whether the game resolution was changed.</returns>
		bool ResolutionChanged() const { return m_ResolutionChanged; }

		/// <summary>
		/// Checks whether the current resolution settings fully cover the display the window is positioned at.
		/// </summary>
		/// <returns>Whether the current resolution settings fully cover the display the window is positioned at.</returns>
		bool FullyCoversDisplayPrimaryWindowIsAtOnly() const { return (m_ResX * m_ResMultiplier == m_DisplayPrimaryWindowIsAtResX) && (m_ResY * m_ResMultiplier == m_DisplayPrimaryWindowIsAtResY); }

		/// <summary>
		/// Checks whether the current resolution settings fully cover all the available displays.
		/// </summary>
		/// <returns>Whether the current resolution settings fully cover all the available displays.</returns>
		bool FullyCoversAllDisplays() const { return m_NumDisplays > 1 && (m_ResX * m_ResMultiplier == m_MaxResX) && (m_ResY * m_ResMultiplier == m_MaxResY); }

		/// <summary>
		/// Gets whether VSync is enabled.
		/// </summary>
		/// <returns>Whether VSync is enabled.</returns>
		bool GetVSyncEnabled() const { return m_EnableVSync; }

		/// <summary>
		/// Sets whether VSync is enabled or not.
		/// </summary>
		/// <param name="enable">Whether to enable VSync.</param>
		void SetVSyncEnabled(bool enable);
#pragma endregion

#pragma region Display Switch Handling
		/// <summary>
		/// Handles focus loss when switching away from the game window.
		/// Will temporarily disable positioning of the mouse so that when focus is switched back to the game window, the game window won't fly away because the user clicked the title bar of the window.
		/// </summary>
		void DisplaySwitchOut() const;

		/// <summary>
		/// Handles focus gain when switching back to the game window.
		/// </summary>
		void DisplaySwitchIn() const;
#pragma endregion

#pragma region Resolution Handling
		/// <summary>
		/// Switches the game window resolution to the specified dimensions.
		/// </summary>
		/// <param name="newResX">New width to set window to.</param>
		/// <param name="newResY">New height to set window to.</param>
		/// <param name="upscaled">Whether the new resolution is upscaled.</param>
		void ChangeResolution(int newResX, int newResY, bool upscaled);

		/// <summary>
		/// Sets and switches to a new resolution multiplier.
		/// </summary>
		void ChangeResolutionMultiplier();

		/// <summary>
		/// Completes the resolution change by resetting the flag.
		/// </summary>
		void CompleteResolutionChange();
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		///
		/// </summary>
		void Update();

		/// <summary>
		///
		/// </summary>
		/// <param name="windowEvent"></param>
		void HandleWindowEvent(const SDL_Event &windowEvent);

		/// <summary>
		/// Clear the primary renderer, or all the renderers if in multi-display fullscreen.
		/// </summary>
		void ClearRenderer() const;

		/// <summary>
		/// Copies the BackBuffer32 content to GPU and shows it on screen.
		/// </summary>
		void UploadFrame() const;
#pragma endregion

	private:

		std::shared_ptr<SDL_Window> m_PrimaryWindow; //!< The main Window.
		std::vector<std::shared_ptr<SDL_Window>> m_MultiScreenWindows; //!< Additional windows for multi display fullscreen.

		std::shared_ptr<SDL_Renderer> m_PrimaryRenderer; //!< The Main Window Renderer, draws to the main window.
		std::vector<std::shared_ptr<SDL_Renderer>> m_MultiScreenRenderers; //!< Additional Renderers for multi display fullscreen.

		std::unique_ptr<SDL_Texture, SDLTextureDeleter> m_PrimaryTexture;
		std::vector<std::unique_ptr<SDL_Texture, SDLTextureDeleter>> m_MultiScreenTextures; //!< Additional Textures when drawing to multiple displays.
		std::vector<SDL_Rect> m_MultiScreenTextureOffsets; //!< Texture offsets for multi-display fullscreen.

		int m_NumDisplays; //!< Number of physical displays.
		int m_DisplayPrimaryWindowIsAtResX; //!< Width of the display the primary window is currently positioned at.
		int m_DisplayPrimaryWindowIsAtResY; //!< Height of the display the primary window is currently positioned at.
		int m_MaxResX; //!< Maximum width the game window can be (desktop width).
		int m_MaxResY; //!< Maximum height the game window can be (desktop height).
		int m_LeftMostDisplayIndex; //!<
		int m_LeftMostOffset; //!<
		int m_TopMostOffset; //!<
		std::vector<std::pair<int, SDL_Rect>> m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen; //!<

		int m_PrimaryWindowDisplayIndex; //!<
		bool m_AnyWindowHasFocus; //!< Whether any game window might have focus.
		bool m_FrameLostFocus; //!< Whether the focus lost event was due to moving between screens.

		int m_ResX; //!< Game window width.
		int m_ResY; //!< Game window height.
		int m_ResMultiplier; //!< The number of times the game window and image should be multiplied and stretched across for better visibility.
		bool m_ResolutionChanged; //!< Whether the resolution was changed through the settings.

		bool m_EnableVSync; //!< Whether vertical synchronization is enabled.

		bool m_IgnoreMultiDisplays; //!<

#pragma region Initialize Breakdown
		/// <summary>
		///
		/// </summary>
		void CreatePrimaryWindow();

		/// <summary>
		///
		/// </summary>
		void CreatePrimaryRenderer();

		/// <summary>
		///
		/// </summary>
		void CreatePrimaryTexture();

		/// <summary>
		///
		/// </summary>
		void CreateMultiDisplayTextures();
#pragma endregion

#pragma region Resolution Handling
		/// <summary>
		/// Checks whether the passed in resolution settings make sense. If not, overrides them to prevent crashes or unexpected behavior. This is called during Initialize().
		/// </summary>
		/// <param name="resX">Game window width to check.</param>
		/// <param name="resY">Game window height to check.</param>
		/// <param name="resMultiplier">Game window resolution multiplier to check.</param>
		void ValidateResolution(int &resX, int &resY, int &resMultiplier) const;

		/// <summary>
		///
		/// </summary>
		/// <param name="revertToDefaults"></param>
		/// <returns></returns>
		void AttemptToRevertToPreviousResolution(bool revertToDefaults = false);
#pragma endregion

#pragma region Multi Display Handling
		/// <summary>
		///
		/// </summary>
		void ClearMultiDisplayData();

		/// <summary>
		/// Get a GL viewport with letterboxing for a window resolutions with different aspect ratio.
		/// </summary>
		/// <param name="resX">Desired horizontal resolution.</param>
		/// <param name="resY">Desired vertical resolution.</param>
		/// <param name="windowW">Window horizontal resolution.</param>
		/// <param name="windowH">Window vertical resolution.</param>
		/// <returns></returns>
		glm::vec4 GetViewportLetterbox(int resX, int resY, int windowW, int windowH);

		/// <summary>
		///
		/// </summary>
		/// <returns></returns>
		bool MapDisplays();

		/// <summary>
		/// Resize the window to enable fullscreen on multiple monitors. This will fill as many screens as necessary to fulfill resX*resY*resMultiplier resolution.
		/// </summary>
		/// <param name="resMultiplier">Requested resolution multiplier.</param>
		/// <returns>Whether all displays were created successfully.</returns>
		bool ChangeResolutionToMultiDisplayFullscreen(int resMultiplier);
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