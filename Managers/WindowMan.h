#ifndef _RTEWINDOWMAN_
#define _RTEWINDOWMAN_

#include "Singleton.h"
#include "System/Shader.h"
#include "glad/gl.h"
#define g_WindowMan WindowMan::Instance()

extern "C" {
	struct SDL_Window;
	struct SDL_Renderer;
	struct SDL_Texture;
	struct SDL_Rect;
	union SDL_Event;
}

namespace RTE {

	class Shader;

	struct SDLWindowDeleter {
		void operator() (SDL_Window *window) const;
	};

	struct SDLRendererDeleter {
		void operator()(SDL_Renderer *renderer) const;
	};

	struct SDLTextureDeleter {
		void operator()(SDL_Texture *texture) const;
	};

	struct SDLContextDeleter {
		void operator()(void *context) const;
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
		~WindowMan();

		/// <summary>
		/// Clean up GL pointers.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets a pointer to the primary game window. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>Pointer to the primary game window.</returns>
		SDL_Window * GetWindow() const { return m_PrimaryWindow.get(); }

		/// <summary>
		/// Gets whether any of the game windows is currently in focus.
		/// </summary>
		/// <returns>Whether any of the game windows is currently in focus.</returns>
		bool AnyWindowHasFocus() const { return m_AnyWindowHasFocus; }

		/// <summary>
		/// Gets the maximum horizontal resolution the game can be resized to.
		/// </summary>
		/// <returns>The maximum horizontal resolution the game can be resized to.</returns>
		int GetMaxResX() const { return m_MaxResX; }

		/// <summary>
		/// Gets the maximum vertical resolution the game can be resized to.
		/// </summary>
		/// <returns>The maximum vertical resolution the game can be resized to.</returns>
		int GetMaxResY() const { return m_MaxResY; }

		/// <summary>
		/// Gets the horizontal resolution the game is currently sized at.
		/// </summary>
		/// <returns>The horizontal resolution the game is currently sized at, in pixels.</returns>
		int GetResX() const { return m_ResX; }

		/// <summary>
		/// Gets the vertical resolution the game is currently sized at.
		/// </summary>
		/// <returns>The vertical resolution the game is currently sized at, in pixels.</returns>
		int GetResY() const { return m_ResY; }

		/// <summary>
		/// Gets the horizontal resolution the game window is currently sized at, in pixels.
		/// </summary>
		/// <returns>The horizontal resolution the game window is currently sized at, in pixels.</returns>
		int GetWindowResX();

		
		/// <summary>
		/// Gets the vertical resolution the game window is currently sized at, in pixels.
		/// </summary>
		/// <returns>The vertical resolution the game window is currently sized at, in pixels.</returns>
		int GetWindowResY();

		/// <summary>
		/// Gets how many times the game resolution is currently being multiplied and the backbuffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the game resolution is currently sized at.</returns>
		float GetResMultiplier() const { return m_ResMultiplier; }

		/// <summary>
		/// Gets whether VSync is enabled.
		/// </summary>
		/// <returns>Whether VSync is enabled.</returns>
		bool GetVSyncEnabled() const { return m_EnableVSync; }

		/// <summary>
		/// Sets whether VSync is enabled.
		/// </summary>
		/// <param name="enable">Whether to enable VSync.</param>
		void SetVSyncEnabled(bool enable);

		/// <summary>
		/// Gets whether the game window is currently in fullscreen.
		/// </summary>
		/// <returns>Whether the game window is currently in fullscreen.</returns>
		bool IsFullscreen() { return m_Fullscreen; }

		/// <summary>
		/// Gets whether the multi-display arrangement should be used or whether only the display the main window is currently positioned at should be used for fullscreen.
		/// </summary>
		/// <returns>Whether the multi-display arrangement is used.</returns>
		bool GetUseMultiDisplays() const { return m_UseMultiDisplays; }

		/// <summary>
		/// Sets whether the multi-display arrangement should be used or whether only the display the main window is currently positioned at should be used for fullscreen.
		/// </summary>
		/// <param name="use">Whether the multi-display arrangement should be used</param>
		void SetUseMultiDisplays(bool use) { m_UseMultiDisplays = use; }

		/// <summary>
		/// Checks whether the current resolution settings fully cover all the available displays.
		/// </summary>
		/// <returns>Whether the current resolution settings fully cover all the available displays.</returns>
		bool FullyCoversAllDisplays() const { return m_NumDisplays > 1 && (m_ResX * m_ResMultiplier == m_MaxResX) && (m_ResY * m_ResMultiplier == m_MaxResY); }

		/// <summary>
		/// Gets the absolute left-most position in the OS display arrangement. Used for correcting mouse position in multi-display fullscreen when the left-most display is not primary.
		/// </summary>
		/// <returns>The absolute left-most position in the OS display arrangement.</returns>
		int GetDisplayArrangementAbsOffsetX() const { return std::abs(m_DisplayArrangementLeftMostOffset); }

		/// <summary>
		/// Gets the absolute top-most position in the OS display arrangement. Used for correcting mouse position in multi-display fullscreen when the left-most display is not primary.
		/// </summary>
		/// <returns>The absolute top-most position in the OS display arrangement.</returns>
		int GetDisplayArrangementAbsOffsetY() const { return std::abs(m_DisplayArrangementTopMostOffset); }

		/// <summary>
		/// Get the screen buffer texture.
		/// </summary>
		/// <returns>The screen buffer texture.</returns>
		GLuint GetScreenBufferTexture() const { return m_ScreenBufferTexture; }
#pragma endregion

#pragma region Resolution Change Handling
		/// <summary>
		/// Attempts to figure our what the hell the OS display arrangement is and what are the resolution capabilities for single or multi-display fullscreen.
		/// </summary>
		/// <param name="updatePrimaryDisplayInfo">Whether to update the stored info of the display the primary window is currently positioned at.</param>
		void MapDisplays(bool updatePrimaryDisplayInfo = true);

		/// <summary>
		/// Gets the horizontal resolution of the display the primary game window is currently positioned at.
		/// </summary>
		/// <returns>The horizontal resolution of the display the primary game window is currently positioned at.</returns>
		int GetPrimaryWindowDisplayWidth() const { return m_PrimaryWindowDisplayWidth; }

		/// <summary>
		/// Gets the vertical resolution of the display the primary game window is currently positioned at.
		/// </summary>
		/// <returns>The vertical resolution of the display the primary game window is currently positioned at.</returns>
		int GetPrimaryWindowDisplayHeight() const { return m_PrimaryWindowDisplayHeight; }

		/// <summary>
		/// Gets whether the game resolution was changed.
		/// </summary>
		/// <returns>Whether the game resolution was changed.</returns>
		bool ResolutionChanged() const { return m_ResolutionChanged; }

		/// <summary>
		/// Switches the game resolution to the specified dimensions.
		/// </summary>
		/// <param name="newResX">New width to resize to.</param>
		/// <param name="newResY">New height to resize to.</param>
		/// <param name="upscaled">Whether the new resolution should be upscaled.</param>
		/// <param name="displaysAlreadyMapped">Whether to skip mapping displays because they were already mapped elsewhere.</param>
		void ChangeResolution(int newResX, int newResY, float newResMultiplier = 1.0f, bool fullscreen = false, bool displaysAlreadyMapped = false);

		/// <summary>
		/// Toggles between windowed and fullscreen mode (single display).
		/// </summary>
		void ToggleFullscreen();

		/// <summary>
		/// Completes the resolution change by resetting the flag.
		/// </summary>
		void CompleteResolutionChange() { m_ResolutionChanged = false; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Adds an SDL_Event to the Event queue for processing on Update.
		/// </summary>
		/// <param name="windowEvent">The SDL window event to queue.</param>
		void QueueWindowEvent(const SDL_Event &windowEvent);

		/// <summary>
		/// Updates the state of this WindowMan.
		/// </summary>
		void Update();

		/// <summary>
		/// Clears the primary renderer, or all the renderers if in multi-display fullscreen.
		/// </summary>
		void ClearRenderer();

		/// <summary>
		/// Set this Frame to draw the game. To be set before UploadFrame. Resets on ClearRenderer.
		/// </summary>
		void DrawPostProcessBuffer() { m_DrawPostProcessBuffer = true; }

		/// <summary>
		/// Copies the BackBuffer32 content to GPU and shows it on screen.
		/// </summary>
		void UploadFrame();
#pragma endregion

	private:

		std::vector<SDL_Event> m_EventQueue; //!< List of incoming window events.

		bool m_FocusEventsDispatchedByMovingBetweenWindows; //!< Whether queued events were dispatched due to raising windows when moving between windows in multi-display fullscreen in the previous update.
		bool m_FocusEventsDispatchedByDisplaySwitchIn; //!< Whether queued events were dispatched due to raising windows when taking focus of any game window in the previous update.

		std::shared_ptr<SDL_Window> m_PrimaryWindow; //!< The main window.
		GLuint m_BackBuffer32Texture; //!< Streaming texture for the software rendered stuff.
		GLuint m_ScreenBufferTexture; //!< Internal backbuffer for the final blit and sceenshots, only clear immediately before drawing.
		GLuint m_ScreenBufferFBO; //!< Framebuffer object for the screen buffer texture.
		glm::mat4 m_PrimaryWindowProjection; //!< Projection Matrix for the main window.
		std::unique_ptr<SDL_Rect> m_PrimaryWindowViewport; //!< Viewport for the main window.

		std::vector<std::shared_ptr<SDL_Window>> m_MultiDisplayWindows; //!< Additional windows for multi-display fullscreen.
		std::vector<glm::mat4> m_MultiDisplayProjections; //!< Projection Matrices for MultiDisplay.
		std::vector<glm::mat4> m_MultiDisplayTextureOffsets; //!< Texture offsets for multi-display fullscreen.

		std::unique_ptr<void, SDLContextDeleter> m_GLContext; //!< OpenGL context.
		GLuint m_ScreenVAO; //!< Vertex Array Object for the screen quad.
		GLuint m_ScreenVBO; //!< Vertex Buffer Object for the screen quad.

		std::unique_ptr<Shader> m_ScreenBlitShader; //!< Blit shader to combine the menu layer and post process layers and show them on screen.

		bool m_AnyWindowHasFocus; //!< Whether any game window has focus.
		bool m_ResolutionChanged; //!< Whether the resolution was changed through the settings.

		int m_NumDisplays; //!< Number of physical displays.
		int m_MaxResX; //!< Maximum width the game window can be (desktop width).
		int m_MaxResY; //!< Maximum height the game window can be (desktop height).
		std::vector<std::pair<int, SDL_Rect>> m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen; //!< Display indices and bounds that can be used for multi-display fullscreen.
		bool m_CanMultiDisplayFullscreen; //!< Whether the display arrangement allows switching to multi-display fullscreen.

		int m_DisplayArrangmentLeftMostDisplayIndex; //!< The index of the left-most screen in the OS display arrangement.
		int m_DisplayArrangementLeftMostOffset; //!< The left-most position in the OS display arrangement.
		int m_DisplayArrangementTopMostOffset; //!< The top-most position in the OS display arrangement.

		int m_PrimaryWindowDisplayIndex; //!< The index of the display the main window is currently positioned at.
		int m_PrimaryWindowDisplayWidth; //!< The width of the display the main window is currently positioned at.
		int m_PrimaryWindowDisplayHeight; //!< The height of the display the main window is currently positioned at.

		int m_ResX; //!< Game window width.
		int m_ResY; //!< Game window height.
		float m_ResMultiplier; //!< The number of times the game window and image should be multiplied and stretched across for better visibility.
		float m_MaxResMultiplier; //!< The maximum resolution multiplier before the game starts breaking.

		bool m_Fullscreen; //!< Whether the game window is currently in fullscreen.

		bool m_EnableVSync; //!< Whether vertical synchronization is enabled.
		bool m_UseMultiDisplays; //!< Whether the multi-display arrangement should be ignored and only the display the main window is currently positioned at should be used for fullscreen.

		bool m_DrawPostProcessBuffer; //!< Whether to draw the PostProcessBuffer while not in Activity. Resets on Frame Clear.

#pragma region Initialize Breakdown
		/// <summary>
		/// Creates the main game window.
		/// </summary>
		void CreatePrimaryWindow();

		/// <summary>
		/// Initializes all opengl objects (textures, vbo, vao, and fbo).
		/// </summary>
		void InitializeOpenGL();

		/// <summary>
		/// Creates the main game window renderer's drawing surface.
		/// </summary>
		void CreateBackBufferTexture();
#pragma endregion

#pragma region Resolution Handling
		void SetViewportLetterboxed();

		/// <summary>
		/// Updates the stored info of the display the primary window is currently positioned at.
		/// </summary>
		void UpdatePrimaryDisplayInfo();

		/// <summary>
		/// Gets the maximum available window bounds for a decorated window on the specified display.
		/// May not provide accurate results if the window is in fullscreen or has just been created.
		/// </summary>
		/// <param name="display">The display to get the bounds for.</param>
		/// <returns>The maximum available window bounds for a decorated window on the specified display.</returns>
		SDL_Rect GetUsableBoundsWithDecorations(int display);

		/// <summary>
		/// Calculates whether the given resolution and multiplier would create a maximized window.
		/// </summary>
		/// <param name="resX">Game window width to check.</param>
		/// <param name="resY">Game window height to check.</param>
		/// <param name="resMultiplier">Game window resolution multiplier to check.</param>
		/// <returns>Whether the given resolution and multiplier create a maximized window.</returns>
		bool IsResolutionMaximized(int resX, int resY, float resMultiplier);

		/// <summary>
		/// Checks whether the passed in resolution settings make sense. If not, overrides them to prevent crashes or unexpected behavior.
		/// </summary>
		/// <param name="resX">Game window width to check.</param>
		/// <param name="resY">Game window height to check.</param>
		/// <param name="resMultiplier">Game window resolution multiplier to check.</param>
		void ValidateResolution(int &resX, int &resY, float &resMultiplier) const;

		/// <summary>
		/// Attempts to revert to the previous resolution settings if the new ones failed for whatever reason. Will recursively attempt to revert to defaults if previous settings fail as well.
		/// </summary>
		/// <param name="revertToDefaults">Whether to attempt to revert to defaults. Will be set by this.</param>
		void AttemptToRevertToPreviousResolution(bool revertToDefaults = false);
#pragma endregion

#pragma region Multi-Display Handling
		/// <summary>
		/// Clears all the multi-display data, resetting the game to a single-window-single-display state.
		/// </summary>
		void ClearMultiDisplayData();

		/// <summary>
		/// Resize the window to enable fullscreen on multiple displays, using the arrangement info gathered during display mapping.
		/// </summary>
		/// <param name="resMultiplier">Requested resolution multiplier.</param>
		/// <returns>Whether all displays were created successfully.</returns>
		bool ChangeResolutionToMultiDisplayFullscreen(float resMultiplier);
#pragma endregion

#pragma region Display Switch Handling
		/// <summary>
		/// Handles focus gain when switching back to the game window.
		/// </summary>
		/// <param name="windowThatShouldTakeInputFocus">The window that should take focus of input after all the windows are raised. This is only relevant in multi-display fullscreen.</param>
		void DisplaySwitchIn(SDL_Window *windowThatShouldTakeInputFocus) const;

		/// <summary>
		/// Handles focus loss when switching away from the game window.
		/// Will temporarily disable positioning of the mouse so that when focus is switched back to the game window, the game window won't fly away because the user clicked the title bar of the window.
		/// </summary>
		void DisplaySwitchOut() const;
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