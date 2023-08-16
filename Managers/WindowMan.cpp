#include "WindowMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PostProcessMan.h"

#include "GLCheck.h"
#include "SDL.h"
#include "glad/gl.h"
#include "Shader.h"

namespace RTE {

	void SDLWindowDeleter::operator()(SDL_Window *window) const { SDL_DestroyWindow(window); }
	void SDLRendererDeleter::operator()(SDL_Renderer *renderer) const { SDL_DestroyRenderer(renderer); }
	void SDLTextureDeleter::operator()(SDL_Texture *texture) const { SDL_DestroyTexture(texture); }

	void SDLContextDeleter::operator()(SDL_GLContext context) const { SDL_GL_DeleteContext(context); }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Clear() {
		m_EventQueue.clear();
		m_FocusEventsDispatchedByMovingBetweenWindows = false;
		m_FocusEventsDispatchedByDisplaySwitchIn = false;

		// m_PrimaryTexture.reset();
		// m_PrimaryRenderer.reset();
		m_PrimaryWindow.reset();
		ClearMultiDisplayData();

		m_AnyWindowHasFocus = false;
		m_ResolutionChanged = false;

		m_NumDisplays = 0;
		m_MaxResX = 0;
		m_MaxResY = 0;
		m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
		m_CanMultiDisplayFullscreen = false;
		m_DisplayArrangmentLeftMostDisplayIndex = -1;
		m_DisplayArrangementLeftMostOffset = 0;
		m_DisplayArrangementLeftMostOffset = 0;

		m_PrimaryWindowDisplayIndex = 0;
		m_PrimaryWindowDisplayWidth = 0;
		m_PrimaryWindowDisplayHeight = 0;
		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;
		m_EnableVSync = true;
		m_IgnoreMultiDisplays = false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ClearMultiDisplayData() {
		#if 0
		m_MultiDisplayTextureOffsets.clear();
		m_MultiDisplayTextures.clear();
		// m_MultiDisplayRenderers.clear();
		m_MultiDisplayWindows.clear();
		#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	WindowMan::WindowMan() {
		Clear();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	WindowMan::~WindowMan() = default;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Initialize() {
		m_NumDisplays = SDL_GetNumVideoDisplays();

		SDL_Rect currentDisplayBounds;
		SDL_GetDisplayBounds(m_PrimaryWindowDisplayIndex, &currentDisplayBounds);

		m_PrimaryWindowDisplayWidth = currentDisplayBounds.w;
		m_PrimaryWindowDisplayHeight = currentDisplayBounds.h;

		MapDisplays(false);

		ValidateResolution(m_ResX, m_ResY, m_ResMultiplier);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		CreatePrimaryWindow();
		InitializeOpenGL();
		CreatePrimaryTexture();
		m_ScreenBlitShader = std::make_unique<Shader>(g_PresetMan.GetFullModulePath("Base.rte/Shaders/ScreenBlit.vert"), g_PresetMan.GetFullModulePath("Base.rte/Shaders/ScreenBlit.frag"));

		// SDL is kinda dumb about the taskbar icon so we need to poll after creating the window for it to show up, otherwise there's no icon till it starts polling in the main menu loop.
		SDL_PollEvent(nullptr);

		m_PrimaryWindowDisplayIndex = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());

		if (FullyCoversAllDisplays()) {
			ChangeResolutionToMultiDisplayFullscreen(m_ResMultiplier);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryWindow() {
		std::string windowTitle = "Cortex Command Community Project";

#ifdef DEBUG_BUILD
		windowTitle += " (Full Debug)";
#elif MIN_DEBUG_BUILD
		windowTitle += " (Min Debug)";
#elif DEBUG_RELEASE_BUILD
		windowTitle += " (Debug Release)";
#endif

#ifdef TARGET_MACHINE_X86
		windowTitle += " (x86)";
#endif

		int windowPosX = (m_ResX * m_ResMultiplier <= m_PrimaryWindowDisplayWidth) ? SDL_WINDOWPOS_CENTERED : (m_MaxResX - (m_ResX * m_ResMultiplier)) / 2;
		int windowPosY = SDL_WINDOWPOS_CENTERED;
		int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;

		if (FullyCoversPrimaryWindowDisplayOnly()) {
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow(windowTitle.c_str(), windowPosX, windowPosY, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, windowFlags), SDLWindowDeleter());
		if (!m_PrimaryWindow) {
			RTEError::ShowMessageBox("Unable to create window because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults!");

			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
			g_SettingsMan.SetSettingsNeedOverwrite();

			m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN), SDLWindowDeleter());
			if (!m_PrimaryWindow) {
				RTEAbort("Failed to create window because:\n" + std::string(SDL_GetError()));
			}
		}
	}

	void WindowMan::InitializeOpenGL() {
		m_GLContext = std::unique_ptr<void, SDLContextDeleter>(SDL_GL_CreateContext(m_PrimaryWindow.get()));

		if (!m_GLContext) {
			RTEAbort("Failed to create OpenGL context because:\n" + std::string(SDL_GetError()));
		}

		std::cout << gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) << std::endl;

		SDL_GL_SetSwapInterval(m_EnableVSync ? 1 : 0);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glGenBuffers(1, &m_ScreenVBO);
		glGenVertexArrays(1, &m_ScreenVAO);
		glBindVertexArray(m_ScreenVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(c_Quad), c_Quad.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryTexture() {
		glDeleteTextures(1, &m_BackBuffer32Texture);
		glGenTextures(1, &m_BackBuffer32Texture);
		glBindTexture(GL_TEXTURE_2D, m_BackBuffer32Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_ResX, m_ResY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreateMultiDisplayTextures() {
#if 0
		m_MultiDisplayTextures.resize(m_MultiDisplayTextureOffsets.size());
		for (size_t i = 0; i < m_MultiDisplayTextures.size(); ++i) {
			m_MultiDisplayTextures[i] = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(SDL_CreateTexture(m_MultiDisplayRenderers[i].get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_MultiDisplayTextureOffsets[i].w, m_MultiDisplayTextureOffsets[i].h));
			if (!m_MultiDisplayTextures[i]) {
				RTEAbort("Failed to create texture for multi-display because:\n" + std::string(SDL_GetError()));
			}
			SDL_RenderSetLogicalSize(m_MultiDisplayRenderers[i].get(), m_MultiDisplayTextureOffsets[i].w, m_MultiDisplayTextureOffsets[i].h);
		}
#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::SetVSyncEnabled(bool enable) {
		m_EnableVSync = enable;

		int sdlEnableVSync = m_EnableVSync ? SDL_TRUE : SDL_FALSE;
		int result = -1;

		SDL_GL_SetSwapInterval(sdlEnableVSync);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::UpdatePrimaryDisplayInfo() {
		m_PrimaryWindowDisplayIndex = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());

		SDL_Rect currentDisplayBounds;
		SDL_GetDisplayBounds(m_PrimaryWindowDisplayIndex, &currentDisplayBounds);

		m_PrimaryWindowDisplayWidth = currentDisplayBounds.w;
		m_PrimaryWindowDisplayHeight = currentDisplayBounds.h;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::MapDisplays(bool updatePrimaryDisplayInfo) {
		auto setSingleDisplayMode = [this](const std::string &errorMsg = "") {
			m_MaxResX = m_PrimaryWindowDisplayWidth;
			m_MaxResY = m_PrimaryWindowDisplayHeight;
			m_NumDisplays = 1;
			m_DisplayArrangementLeftMostOffset = -1;
			m_DisplayArrangementTopMostOffset = -1;
			m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
			m_CanMultiDisplayFullscreen = false;
			m_IgnoreMultiDisplays = true;
			if (!errorMsg.empty()) {
				RTEError::ShowMessageBox("Failed to map displays for multi-display fullscreen because:\n\n" + errorMsg + "!\n\nFullscreen will be limited to the display the window is positioned at!");
			}
		};

		if (updatePrimaryDisplayInfo) {
			UpdatePrimaryDisplayInfo();
		}

		m_NumDisplays = SDL_GetNumVideoDisplays();

		if (m_IgnoreMultiDisplays || m_NumDisplays == 1) {
			setSingleDisplayMode();
			return;
		}

		m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();

		int leftMostOffset = 0;
		int topMostOffset = std::numeric_limits<int>::max();
		int maxHeight = std::numeric_limits<int>::min();
		int totalWidth = 0;

		for (int displayIndex = 0; displayIndex < m_NumDisplays; ++displayIndex) {
			SDL_Rect displayBounds;
			if (SDL_GetDisplayBounds(displayIndex, &displayBounds) == 0) {
				m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.emplace_back(displayIndex, displayBounds);

				leftMostOffset = std::min(leftMostOffset, displayBounds.x);
				topMostOffset = std::min(topMostOffset, displayBounds.y);
				maxHeight = std::max(maxHeight, displayBounds.h);

				totalWidth += displayBounds.w;
			} else {
				setSingleDisplayMode("Failed to get resolution of display " + std::to_string(displayIndex) + "!");
				return;
			}
		}

		if (m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.size() == 1) {
			setSingleDisplayMode("Somehow ended up with only one valid display even though " + std::to_string(m_NumDisplays) + " are available!");
			return;
		}

		std::stable_sort(m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.begin(), m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.end(),
		    [](auto left, auto right) {
			    return left.second.x < right.second.x;
		    });

		for (const auto &[displayIndex, displayBounds]: m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
			// Translate display offsets to backbuffer offsets, where the top left corner is (0,0) to figure out if the display arrangement is unreasonable garbage, i.e not top or bottom edge aligned.
			// If any of the translated offsets ends up negative, or over-positive for the Y offset, disallow going into multi-display fullscreen
			// because we'll just end up with an access violation when trying to read from the backbuffer pixel array during rendering.
			// In odd display size arrangements this is valid as long as the misaligned display is somewhere between the top and bottom edge of the tallest display, as the translated offset will remain in bounds.
			int translatedOffsetX = (displayBounds.x - leftMostOffset);
			int translatedOffsetY = (displayBounds.y - topMostOffset);
			if (translatedOffsetX < 0 || translatedOffsetY < 0 || translatedOffsetY + displayBounds.h > maxHeight) {
				setSingleDisplayMode("Bad display alignment detected!\nMulti-display fullscreen currently supports only horizontal arrangements where all displays are either top or bottom edge aligned!");
				return;
			}
		}

		for (const auto &[displayIndex, displayBounds]: m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
#if SDL_VERSION_ATLEAST(2, 24, 0)
			m_DisplayArrangmentLeftMostDisplayIndex = SDL_GetRectDisplayIndex(&displayBounds);
			if (m_DisplayArrangmentLeftMostDisplayIndex >= 0) {
#else
			// This doesn't return the nearest display index to the point but should still be reliable enough for reasonable display arrangements.
			SDL_Point testPoint = {leftMostOffset + 1, topMostOffset + 1};
			if (SDL_PointInRect(&testPoint, &displayBounds) == SDL_TRUE) {
#endif
				m_DisplayArrangmentLeftMostDisplayIndex = displayIndex;
				break;
			}
		}

		if (m_DisplayArrangmentLeftMostDisplayIndex >= 0) {
			m_MaxResX = totalWidth;
			m_MaxResY = maxHeight;
			m_DisplayArrangementLeftMostOffset = leftMostOffset;
			m_DisplayArrangementTopMostOffset = topMostOffset;
			m_CanMultiDisplayFullscreen = true;
		} else {
			setSingleDisplayMode("Unable to determine left-most display index!");
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ValidateResolution(int &resX, int &resY, int &resMultiplier) const {
		if (resX * resMultiplier > m_MaxResX || resY * resMultiplier > m_MaxResY || resMultiplier < 1 || resMultiplier > 8) {
			resMultiplier = std::clamp<int>(resMultiplier, 1, 8);
			resX = std::min(resX, m_MaxResX / resMultiplier);
			resY = std::min(resY, m_MaxResY / resMultiplier);
			RTEError::ShowMessageBox("Resolution too high to fit display, overriding to fit!");
			g_SettingsMan.SetSettingsNeedOverwrite();
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::AttemptToRevertToPreviousResolution(bool revertToDefaults) {
		auto setDefaultResSettings = [this]() {
			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
			g_SettingsMan.UpdateSettingsFile();
		};

		int windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;

		if ((m_ResX * m_ResMultiplier >= m_MaxResX) && (m_ResY * m_ResMultiplier >= m_MaxResY)) {
			setDefaultResSettings();
			windowFlags = 0;
		}
		SDL_SetWindowSize(m_PrimaryWindow.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);

		if (!FullyCoversPrimaryWindowDisplayOnly()) {
			windowFlags = 0;
			SDL_SetWindowBordered(m_PrimaryWindow.get(), SDL_TRUE);
			SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex));
		}

		bool result = SDL_SetWindowFullscreen(m_PrimaryWindow.get(), windowFlags) == 0;
		if (!result && !revertToDefaults) {
			RTEError::ShowMessageBox("Failed to revert to previous resolution settings!\nAttempting to revert to defaults!");
			setDefaultResSettings();
			AttemptToRevertToPreviousResolution(true);
		} else if (!result) {
			RTEAbort("Failed to revert to previous resolution or defaults because: \n" + std::string(SDL_GetError()) + "!");
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolution(int newResX, int newResY, bool upscaled, bool displaysAlreadyMapped) {
		int newResMultiplier = upscaled ? 2 : 1;

		if (m_ResX == newResX && m_ResY == newResY && m_ResMultiplier == newResMultiplier) {
			return;
		}

		bool onlyResMultiplierChange = (m_ResX == newResX) && (m_ResY == newResY) && (m_ResMultiplier != newResMultiplier);

		ClearMultiDisplayData();

		if (!displaysAlreadyMapped) {
			MapDisplays();
		}
		ValidateResolution(newResX, newResY, newResMultiplier);

		bool newResFullyCoversPrimaryWindowDisplay = (newResX * newResMultiplier == m_PrimaryWindowDisplayWidth) && (newResY * newResMultiplier == m_PrimaryWindowDisplayHeight);
		bool newResFullyCoversAllDisplays = m_CanMultiDisplayFullscreen && (m_NumDisplays > 1) && (newResX * newResMultiplier == m_MaxResX) && (newResY * newResMultiplier == m_MaxResY);

		bool recoveredToPreviousSettings = false;

		if ((newResFullyCoversAllDisplays && !ChangeResolutionToMultiDisplayFullscreen(newResMultiplier)) || (newResFullyCoversPrimaryWindowDisplay && SDL_SetWindowFullscreen(m_PrimaryWindow.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)) {
			RTEError::ShowMessageBox("Failed to switch to new resolution!\nAttempting to revert to previous settings!");
			AttemptToRevertToPreviousResolution();
			recoveredToPreviousSettings = true;
		} else if (!newResFullyCoversPrimaryWindowDisplay && !newResFullyCoversAllDisplays) {
			SDL_SetWindowFullscreen(m_PrimaryWindow.get(), 0);
			SDL_SetWindowSize(m_PrimaryWindow.get(), newResX * newResMultiplier, newResY * newResMultiplier);
			SDL_RestoreWindow(m_PrimaryWindow.get());
			SDL_SetWindowBordered(m_PrimaryWindow.get(), SDL_TRUE);
			SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_PrimaryWindowDisplayIndex));
		}
		if (!recoveredToPreviousSettings) {
			m_ResX = newResX;
			m_ResY = newResY;
			m_ResMultiplier = newResMultiplier;

			g_SettingsMan.UpdateSettingsFile();
		}

		if (onlyResMultiplierChange) {
			if (!newResFullyCoversAllDisplays) {
				CreatePrimaryTexture();
			}
			g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier.");
		} else {
			m_ResolutionChanged = true;
			g_FrameMan.CreateBackBuffers();

			if (newResFullyCoversAllDisplays) {
				CreateMultiDisplayTextures();
			} else {
				CreatePrimaryTexture();
			}
		}

		g_ConsoleMan.PrintString("SYSTEM: " + std::string(!recoveredToPreviousSettings ? "Switched to different resolution." : "Failed to switch to different resolution. Reverted to previous settings."));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolutionMultiplier() {
		int newResMultiplier = (m_ResMultiplier == 1) ? 2 : 1;

		MapDisplays();

		if ((m_ResX * newResMultiplier > m_MaxResX) || (m_ResY * newResMultiplier > m_MaxResY)) {
			RTEError::ShowMessageBox("Requested resolution multiplier will result in game window exceeding display bounds!\nNo change will be made!\n\nNOTE: To toggle fullscreen, use the button in the Options & Controls Menu!");
			return;
		}
		ChangeResolution(m_ResX, m_ResY, newResMultiplier > 1, true);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WindowMan::ChangeResolutionToMultiDisplayFullscreen(int resMultiplier) {
		return false;
#if 0
		if (!m_CanMultiDisplayFullscreen) {
			return false;
		}
		int windowPrevPositionX = 0;
		int windowPrevPositionY = 0;
		SDL_GetWindowPosition(m_PrimaryWindow.get(), &windowPrevPositionX, &windowPrevPositionY);

		// Move the window to the detected leftmost display to avoid all the headaches.
		if (m_PrimaryWindowDisplayIndex != m_DisplayArrangmentLeftMostDisplayIndex) {
			SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_DisplayArrangmentLeftMostDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_DisplayArrangmentLeftMostDisplayIndex));
			m_PrimaryWindowDisplayIndex = m_DisplayArrangmentLeftMostDisplayIndex;
		}

		bool errorSettingFullscreen = false;

		for (const auto &[displayIndex, displayBounds] : m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
			int displayOffsetX = displayBounds.x;
			int displayOffsetY = displayBounds.y;
			int displayWidth = displayBounds.w;
			int displayHeight = displayBounds.h;

			if (displayIndex == m_PrimaryWindowDisplayIndex) {
				m_MultiDisplayWindows.emplace_back(m_PrimaryWindow);
				m_MultiDisplayRenderers.emplace_back(m_PrimaryRenderer);
			} else {
				m_MultiDisplayWindows.emplace_back(SDL_CreateWindow(nullptr, displayOffsetX, displayOffsetY, displayWidth, displayHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR), SDLWindowDeleter());
				if (m_MultiDisplayWindows.back()) {
					m_MultiDisplayRenderers.emplace_back(SDL_CreateRenderer(m_MultiDisplayWindows.back().get(), -1, SDL_RENDERER_ACCELERATED), SDLRendererDeleter());
					if (!m_MultiDisplayRenderers.back()) {
						errorSettingFullscreen = true;
					}
				} else {
					errorSettingFullscreen = true;
				}
				if (errorSettingFullscreen) {
					break;
				}
			}

			int textureOffsetX = (displayOffsetX - m_DisplayArrangementLeftMostOffset);
			int textureOffsetY = (displayOffsetY - m_DisplayArrangementTopMostOffset);

			m_MultiDisplayTextureOffsets.emplace_back(SDL_Rect{
				textureOffsetX / resMultiplier,
				// Sometimes an odd Y offset implodes all the things, depending on the stupidity of the arrangement and what display is primary.
				// Sometimes it needs to be in multiples of 4 for reasons unknown to man, so we're just gonna go with this and hope for the best, for now at least.
				RoundToNearestMultiple(textureOffsetY, 2) / resMultiplier,
				displayWidth / resMultiplier,
				displayHeight / resMultiplier
			});
		}

		if (errorSettingFullscreen) {
			ClearMultiDisplayData();
			SDL_SetWindowPosition(m_PrimaryWindow.get(), windowPrevPositionX, windowPrevPositionY);
			return false;
		}

		CreateMultiDisplayTextures();
		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
		return true;
#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchIn(SDL_Window *windowThatShouldTakeInputFocus) const {
		g_UInputMan.DisableMouseMoving(false);
		g_UInputMan.DisableKeys(false);

		if (!m_MultiDisplayWindows.empty()) {
			for (const auto &window: m_MultiDisplayWindows) {
				SDL_RaiseWindow(window.get());
			}
			SDL_RaiseWindow(windowThatShouldTakeInputFocus);
			SDL_SetWindowInputFocus(windowThatShouldTakeInputFocus);
		} else {
			SDL_RaiseWindow(m_PrimaryWindow.get());
		}

		SDL_ShowCursor(SDL_DISABLE);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchOut() const {
		g_UInputMan.DisableMouseMoving(true);
		g_UInputMan.DisableKeys(true);

		SDL_ShowCursor(SDL_ENABLE);
		// Sometimes the cursor will not be visible after disabling relative mode. Setting it to nullptr forces it to redraw, though this doesn't always work either.
		SDL_SetCursor(nullptr);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::QueueWindowEvent(const SDL_Event &windowEvent) {
		m_EventQueue.emplace_back(windowEvent);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Update() {
		// Some bullshit we have to deal with to correctly focus windows in multi-display fullscreen so mouse binding/unbinding works correctly. Not relevant for single window.
		// This is SDL's fault for not having handling to raise a window so it's top-most without taking focus of it.
		// Don't process any focus events this update if either of these has been set in the previous one.
		// Having two flags is a bit redundant but better be safe than recursively raising windows and taking focus and pretty much locking up your whole shit so the only thing you can do about it is sign out to terminate this.
		// Clearing the queue here is just us not handling the events on our end. Whatever they are and do and wherever they propagate to is handled by SDL_PollEvent earlier.
		if (m_FocusEventsDispatchedByDisplaySwitchIn || m_FocusEventsDispatchedByMovingBetweenWindows) {
			m_EventQueue.clear();

			m_FocusEventsDispatchedByDisplaySwitchIn = false;
			m_FocusEventsDispatchedByMovingBetweenWindows = false;
			return;
		}

		SDL_Event windowEvent;
		for (std::vector<SDL_Event>::const_iterator eventIterator = m_EventQueue.begin(); eventIterator != m_EventQueue.end(); eventIterator++) {
			windowEvent = *eventIterator;
			int windowID = windowEvent.window.windowID;

			switch (windowEvent.window.event) {
				case SDL_WINDOWEVENT_ENTER:
					if (SDL_GetWindowID(SDL_GetMouseFocus()) > 0 && m_AnyWindowHasFocus && FullyCoversAllDisplays()) {
						for (const auto &window: m_MultiDisplayWindows) {
							SDL_RaiseWindow(window.get());
						}
						SDL_RaiseWindow(SDL_GetWindowFromID(windowID));
						SDL_SetWindowInputFocus(SDL_GetWindowFromID(windowID));
						m_AnyWindowHasFocus = true;
						m_FocusEventsDispatchedByMovingBetweenWindows = true;
					}
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					DisplaySwitchIn(SDL_GetWindowFromID(windowID));
					m_AnyWindowHasFocus = true;
					m_FocusEventsDispatchedByDisplaySwitchIn = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					DisplaySwitchOut();
					m_AnyWindowHasFocus = false;
					m_FocusEventsDispatchedByDisplaySwitchIn = false;
					m_FocusEventsDispatchedByMovingBetweenWindows = false;
					break;
				default:
					break;
			}
		}
		m_EventQueue.clear();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ClearRenderer() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		g_FrameMan.ClearBackBuffer32();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::UploadFrame() {
		glDisable(GL_DEPTH_TEST);
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CHECK(glActiveTexture(GL_TEXTURE0));

		int windowW, windowH;
		SDL_GL_GetDrawableSize(m_PrimaryWindow.get(), &windowW, &windowH);
		GL_CHECK(glViewport(0, 0, windowW, windowH));

		glDisable(GL_BLEND);
		if (g_ActivityMan.IsInActivity()) {
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, g_PostProcessMan.GetPostProcessColorBuffer()));
			GL_CHECK(glActiveTexture(GL_TEXTURE1));
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_BackBuffer32Texture));
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_FrameMan.GetBackBuffer32()->w, g_FrameMan.GetBackBuffer32()->h, GL_RGBA, GL_UNSIGNED_BYTE, g_FrameMan.GetBackBuffer32()->line[0]));
		} else {
			glDisable(GL_BLEND);
			glBindTexture(GL_TEXTURE_2D, m_BackBuffer32Texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_FrameMan.GetBackBuffer32()->w, g_FrameMan.GetBackBuffer32()->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_FrameMan.GetBackBuffer32()->line[0]);
		}

		GL_CHECK(glBindVertexArray(m_ScreenVAO));
		m_ScreenBlitShader->Use();
		m_ScreenBlitShader->SetInt(m_ScreenBlitShader->GetTextureUniform(), 0);
		m_ScreenBlitShader->SetMatrix4f(m_ScreenBlitShader->GetProjectionUniform(), glm::mat4(1.0f));
		m_ScreenBlitShader->SetMatrix4f(m_ScreenBlitShader->GetTransformUniform(), glm::mat4(1.0f));
		if(g_ActivityMan.IsInActivity()) {
			m_ScreenBlitShader->SetInt(m_ScreenBlitShader->GetUniformLocation("rteGUITexture"), 1);
		}

		GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));


		SDL_GL_SwapWindow(m_PrimaryWindow.get());
	}

} // namespace RTE