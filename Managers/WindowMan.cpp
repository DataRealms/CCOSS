#include "WindowMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"

#include "SDL.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_query.hpp"

namespace RTE {

	void SDLWindowDeleter::operator()(SDL_Window *window) const { SDL_DestroyWindow(window); }
	void SDLRendererDeleter::operator()(SDL_Renderer *renderer) const { SDL_DestroyRenderer(renderer); }
	void SDLTextureDeleter::operator()(SDL_Texture *texture) const { SDL_DestroyTexture(texture); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Clear() {
		m_PrimaryWindow.reset();
		m_MultiScreenWindows.clear();
		m_PrimaryRenderer.reset();
		m_MultiScreenRenderers.clear();
		m_PrimaryTexture.reset();
		m_MultiScreenTextures.clear();
		m_MultiScreenTextureOffsets.clear();

		m_NumDisplays = 0;
		m_PrimaryDisplayResX = 0;
		m_PrimaryDisplayResY = 0;
		m_MaxResX = 0;
		m_MaxResY = 0;
		m_CanMultiDisplayFullscreen = false;
		m_LeftMostDisplayIndex = -1;
		m_LeftMostOffset = -1;
		m_LeftMostOffset = -1;
		m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
		m_PrimaryWindowDisplayIndex = 0;
		m_AnyWindowHasFocus = false;
		m_FrameLostFocus = false;
		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;
		m_ResChanged = false;
		m_EnableVSync = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	WindowMan::WindowMan() {
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Initialize() {
		MapDisplays();

		ValidateResolution(m_ResX, m_ResY, m_ResMultiplier);

		CreatePrimaryWindow();
		CreatePrimaryRenderer();
		CreatePrimaryTexture();

		m_PrimaryWindowDisplayIndex = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());

		if (CoversMultiDisplayFullscreen()) {
			ChangeResolutionToMultiDisplayFullscreen(m_ResMultiplier);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryWindow() {
		const char *windowTitle = "Cortex Command Community Project";
		int windowPosX = (m_ResX * m_ResMultiplier <= m_PrimaryDisplayResX) ? SDL_WINDOWPOS_CENTERED : (m_MaxResX - (m_ResX * m_ResMultiplier)) / 2;
		int windowPosY = SDL_WINDOWPOS_CENTERED;
		int windowFlags = SDL_WINDOW_SHOWN;

		if (CoversPrimaryFullscreen()) {
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow(windowTitle, windowPosX, windowPosY, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, windowFlags), SDLWindowDeleter());
		if (!m_PrimaryWindow) {
			ShowMessageBox("Unable to create window because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults!");

			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
			g_SettingsMan.SetSettingsNeedOverwrite();

			m_PrimaryWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, SDL_WINDOW_SHOWN), SDLWindowDeleter());
			if (!m_PrimaryWindow) {
				RTEAbort("Failed to create window because:\n" + std::string(SDL_GetError()));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryRenderer() {
		int renderFlags = SDL_RENDERER_ACCELERATED;
		if (m_EnableVSync) {
			renderFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		m_PrimaryRenderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(m_PrimaryWindow.get(), -1, renderFlags), SDLRendererDeleter());
		if (!m_PrimaryRenderer) {
			ShowMessageBox("Unable to create hardware accelerated renderer because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to software rendering!");
			m_PrimaryRenderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(m_PrimaryWindow.get(), -1, SDL_RENDERER_SOFTWARE), SDLRendererDeleter());

			if (!m_PrimaryRenderer) {
				RTEAbort("Failed to initialize renderer!\nAre you sure this is a computer?");
			}
		}

		SDL_RenderSetIntegerScale(m_PrimaryRenderer.get(), SDL_TRUE);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryTexture() {
		m_PrimaryTexture = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(SDL_CreateTexture(m_PrimaryRenderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_ResX, m_ResY));
		SDL_RenderSetLogicalSize(m_PrimaryRenderer.get(), m_ResX, m_ResY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreateMultiDisplayTextures() {
		m_MultiScreenTextures.resize(m_MultiScreenTextureOffsets.size());
		for (size_t i = 0; i < m_MultiScreenTextures.size(); ++i) {
			m_MultiScreenTextures[i] = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(SDL_CreateTexture(m_MultiScreenRenderers[i].get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_MultiScreenTextureOffsets[i].w, m_MultiScreenTextureOffsets[i].h));
			if (!m_MultiScreenTextures[i]) {
				RTEAbort("Failed to create texture for multi-display: " + std::string(SDL_GetError()));
			}
			SDL_RenderSetLogicalSize(m_MultiScreenRenderers[i].get(), m_MultiScreenTextureOffsets[i].w, m_MultiScreenTextureOffsets[i].h);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ValidateResolution(int &resX, int &resY, int &resMultiplier) const {
		bool settingsNeedOverwrite = false;

		if (resX * resMultiplier > m_MaxResX || resY * resMultiplier > m_MaxResY) {
			settingsNeedOverwrite = true;
			resX = m_MaxResX / resMultiplier;
			resY = m_MaxResY / resMultiplier;
			ShowMessageBox("Resolution too high to fit display, overriding to fit!");
		}
		if (settingsNeedOverwrite) {
			g_SettingsMan.SetSettingsNeedOverwrite();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::SetVSyncEnabled(bool enable) {
		m_EnableVSync = enable;

		int sdlEnable = m_EnableVSync ? SDL_TRUE : SDL_FALSE;
		int result = -1;

#if	SDL_VERSION_ATLEAST(2, 0, 18)
		// Setting VSync per renderer is introduced in 2.0.18.
		if (!m_MultiScreenRenderers.empty()) {
			for (const auto &renderer : m_MultiScreenRenderers) {
				result = SDL_RenderSetVSync(renderer.get(), sdlEnable);

				if (result != 0) {
					break;
				}
			}
		} else {
			result = SDL_RenderSetVSync(m_PrimaryRenderer.get(), sdlEnable);
		}
#endif

		if (result != 0) {
			g_ConsoleMan.PrintString("ERROR: Unable to change VSync mode at runtime! The change will be applied after restarting!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchOut() const {
		g_UInputMan.DisableMouseMoving(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchIn() const {
		g_UInputMan.DisableMouseMoving(false);
		if (!m_MultiScreenWindows.empty()) {
			SDL_RaiseWindow(m_PrimaryWindow.get());
			for (auto &window : m_MultiScreenWindows) {
				SDL_RaiseWindow(window.get());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::AttemptToRevertToPreviousResolution(bool revertToDefaults) {
		SDL_SetWindowSize(m_PrimaryWindow.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);

		int result = SDL_SetWindowFullscreen(m_PrimaryWindow.get(), CoversPrimaryFullscreen() ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0;

		if (!revertToDefaults && result != 0) {
			ShowMessageBox("Failed to switch to new resolution, reverted back to previous setting! Attempting to revert to defaults!");

			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
			g_SettingsMan.UpdateSettingsFile();

			AttemptToRevertToPreviousResolution(true);
		} else if (result != 0) {
			RTEAbort("Unable to revert to previous resolution or defaults because : \n" + std::string(SDL_GetError()) + "!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolution(int newResX, int newResY, bool upscaled) {
		int newResMultiplier = upscaled ? 2 : 1;

		if (m_ResX == newResX && m_ResY == newResY && m_ResMultiplier == newResMultiplier) {
			return;
		}

		m_MultiScreenTextureOffsets.clear();
		m_MultiScreenTextures.clear();
		m_MultiScreenRenderers.clear();
		m_MultiScreenWindows.clear();


		ValidateResolution(newResX, newResY, newResMultiplier);

		bool newResSettingsCoverPrimaryFullscreen = (newResX * newResMultiplier == m_PrimaryDisplayResX) && (newResY * newResMultiplier == m_PrimaryDisplayResY);
		bool newResSettingsCoverMultiScreenFullscreen = (m_NumDisplays > 1) && (newResX * newResMultiplier == m_MaxResX) && (newResY * newResMultiplier == m_MaxResY);
		bool switchToFullscreen = newResSettingsCoverPrimaryFullscreen || newResSettingsCoverMultiScreenFullscreen;

		if (switchToFullscreen && ((m_NumDisplays > 1 && !SetWindowMultiFullscreen(newResX, newResY, newResMultiplier)) || SDL_SetWindowFullscreen(m_PrimaryWindow.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)) {
			AttemptToRevertToPreviousResolution();
			return;
		} else if (!switchToFullscreen) {
			SDL_SetWindowFullscreen(m_PrimaryWindow.get(), 0);
			SDL_SetWindowSize(m_PrimaryWindow.get(), newResX * newResMultiplier, newResY * newResMultiplier);
			SDL_RestoreWindow(m_PrimaryWindow.get());
			SDL_SetWindowBordered(m_PrimaryWindow.get(), SDL_TRUE);
			int displayIndex = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());
			SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
		}

		m_ResX = newResX;
		m_ResY = newResY;
		m_ResMultiplier = newResMultiplier;
		m_ResChanged = true;

		g_SettingsMan.UpdateSettingsFile();

		g_FrameMan.RecreateBackBuffers();

		if (newResSettingsCoverMultiScreenFullscreen) {
			CreateMultiDisplayTextures();
		} else {
			CreatePrimaryTexture();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolutionMultiplier() {
		int newResMultiplier = (m_ResMultiplier == 1) ? 2 : 1;

		if (m_MaxResX / newResMultiplier < m_ResX || m_MaxResY / newResMultiplier < m_ResY) {
			ShowMessageBox("Requested resolution multiplier will result in game window exceeding display bounds!\nNo change will be made!\n\nNOTE: To toggle fullscreen, use the button in the Options & Controls Menu!");
			return;
		}

		m_MultiScreenTextureOffsets.clear();
		m_MultiScreenTextures.clear();
		m_MultiScreenRenderers.clear();
		m_MultiScreenWindows.clear();


		bool newResSettingsCoverPrimaryFullscreen = (m_ResX * newResMultiplier == m_PrimaryDisplayResX) && (m_ResY * newResMultiplier == m_PrimaryDisplayResY);
		bool newResSettingsCoverMultiScreenFullscreen = (m_NumDisplays > 1) && (m_ResX * newResMultiplier == m_MaxResX) && (m_ResY * newResMultiplier == m_MaxResY);

		if (newResSettingsCoverPrimaryFullscreen || newResSettingsCoverMultiScreenFullscreen) {
			SDL_SetWindowSize(m_PrimaryWindow.get(), m_ResX * newResMultiplier, m_ResY * newResMultiplier);
			SDL_SetWindowPosition(m_PrimaryWindow.get(), 0, 0);
		} else {
			int windowPosX = (m_ResX * newResMultiplier <= m_PrimaryDisplayResX) ? SDL_WINDOWPOS_CENTERED : (m_MaxResX - (m_ResX * newResMultiplier)) / 2;

			SDL_SetWindowSize(m_PrimaryWindow.get(), m_ResX * newResMultiplier, m_ResY * newResMultiplier);
			SDL_SetWindowPosition(m_PrimaryWindow.get(), windowPosX, SDL_WINDOWPOS_CENTERED);
		}

		m_ResMultiplier = newResMultiplier;

		g_SettingsMan.UpdateSettingsFile();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier.");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CompleteResolutionChange() {
		g_FrameMan.DestroyTempBackBuffers();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different resolution.");
		m_ResChanged = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glm::vec4 WindowMan::GetViewportLetterbox(int resX, int resY, int windowW, int windowH) {
		float aspectRatio = resX / static_cast<float>(resY);
		int width = windowW;
		int height = width / aspectRatio + 0.5F;

		if (height > windowH) {
			height = windowH;
			width = height * aspectRatio + 0.5F;
		}

		int offsetX = (windowW / 2) - (width / 2);
		int offsetY = (windowH / 2) - (height / 2);
		return glm::vec4(offsetX, windowH - offsetY - height, width, height);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::MapDisplays() {
		m_NumDisplays = SDL_GetNumVideoDisplays();

		SDL_Rect primaryDisplayBounds;
		SDL_GetDisplayBounds(0, &primaryDisplayBounds);

		m_PrimaryDisplayResX = primaryDisplayBounds.w;
		m_PrimaryDisplayResY = primaryDisplayBounds.h;

		m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();

		int leftMostOffset = 0;
		int topMostOffset = 0;
		int maxHeight = 0;
		int maxUsableHeight = m_ResY;
		int minHeightDisplayIndex = -1;
		int totalWidth = 0;

		for (int displayIndex = 0; displayIndex < m_NumDisplays; ++displayIndex) {
			SDL_Rect displayBounds;
			if (SDL_GetDisplayBounds(displayIndex, &displayBounds) == 0) {
				m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.emplace_back(displayIndex, displayBounds);

				leftMostOffset = std::min(leftMostOffset, displayBounds.x);
				topMostOffset = std::min(topMostOffset, displayBounds.y);
				maxHeight = std::max(maxHeight, displayBounds.h);

				int prevMinHeight = maxUsableHeight;
				maxUsableHeight = std::min(maxUsableHeight, displayBounds.h);

				if (maxUsableHeight < prevMinHeight) {
					minHeightDisplayIndex = displayIndex;
				}

				totalWidth += displayBounds.w;
			} else {
				g_ConsoleMan.PrintString("ERROR: Failed to get resolution of display " + std::to_string(displayIndex));
			}
		}

		bool mappingErrorOrOnlyOneDisplay = false;

		if (m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.size() > 1) {
			std::stable_sort(m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.begin(), m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.end(),
				[](auto left, auto right) {
					return left.second.x < right.second.x;
				}
			);

			// TODO: Probably need this for vertical setups, but that's more headache to deal with. Currently a vertical arrangement works but extends horizontally so it's useless.
			//std::stable_sort(m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.begin(), m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.end(),
			//	[](auto left, auto right) {
			//		return left.second.y < right.second.y;
			//	}
			//);

			for (const auto &[displayIndex, displayBounds] : m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
				SDL_Point testPoint = { leftMostOffset + 1, topMostOffset + 1 };
				if (SDL_PointInRect(&testPoint, &displayBounds) == SDL_TRUE) {
					m_LeftMostDisplayIndex = displayIndex;
					break;
				}
			}
			if (m_LeftMostDisplayIndex >= 0) {
					m_CanMultiDisplayFullscreen = true;
					m_VerticalMultiDisplay = verticalDisplaySetup;
					m_MaxResX = totalWidth;
					m_MaxResY = maxHeight;
					m_LeftMostOffset = leftMostOffset;
					m_TopMostOffset = topMostOffset;
			} else {
				mappingErrorOrOnlyOneDisplay = true;
			}
		} else {
			mappingErrorOrOnlyOneDisplay = true;
		}

		if (mappingErrorOrOnlyOneDisplay) {
			m_CanMultiDisplayFullscreen = false;
			m_MaxResX = m_PrimaryDisplayResX;
			m_MaxResY = m_PrimaryDisplayResY;
			m_LeftMostOffset = -1;
			m_TopMostOffset = -1;
			m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen.clear();
			g_ConsoleMan.PrintString("");
			return;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WindowMan::ChangeResolutionToMultiDisplayFullscreen(int resMultiplier) {
		MapDisplays();

		if (!m_CanMultiDisplayFullscreen) {
			return false;
		}

		int windowPrevPositionX = 0;
		int windowPrevPositionY = 0;
		SDL_GetWindowPosition(m_PrimaryWindow.get(), &windowPrevPositionX, &windowPrevPositionY);

		// Move the window to the detected leftmost display to avoid all the headaches.
		if (m_PrimaryWindowDisplayIndex != m_LeftMostDisplayIndex) {
			SDL_SetWindowPosition(m_PrimaryWindow.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(m_LeftMostDisplayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(m_LeftMostDisplayIndex));
		}

		m_MultiScreenTextureOffsets.clear();
		m_MultiScreenTextures.clear();
		m_MultiScreenRenderers.clear();
		m_MultiScreenWindows.clear();

		bool errorSettingFullscreen = false;

		for (const auto &[displayIndex, displayBounds] : m_ValidDisplayIndicesAndBoundsForMultiDisplayFullscreen) {
			int displayOffsetX = displayBounds.x;
			int displayOffsetY = displayBounds.y;
			int displayWidth = displayBounds.w;
			int displayHeight = displayBounds.h;

			if (displayIndex == m_PrimaryWindowDisplayIndex) {
				m_MultiScreenWindows.emplace_back(m_PrimaryWindow);
				m_MultiScreenRenderers.emplace_back(m_PrimaryRenderer);
			} else {
				m_MultiScreenWindows.emplace_back(SDL_CreateWindow(nullptr, displayOffsetX, displayOffsetY, displayWidth, displayHeight, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR), SDLWindowDeleter());
				if (m_MultiScreenWindows.back()) {
					m_MultiScreenRenderers.emplace_back(SDL_CreateRenderer(m_MultiScreenWindows.back().get(), -1, SDL_RENDERER_ACCELERATED), SDLRendererDeleter());
					if (!m_MultiScreenRenderers.back()) {
						errorSettingFullscreen = true;
					}
				} else {
					errorSettingFullscreen = true;
				}
				if (errorSettingFullscreen) {
					break;
				}
			}
			m_MultiScreenTextureOffsets.emplace_back(SDL_Rect {
				(m_LeftMostOffset + displayOffsetX) / resMultiplier,
				(m_TopMostOffset + displayOffsetY) / resMultiplier,
				displayWidth / resMultiplier,
				displayHeight / resMultiplier
			});
		}

		if (errorSettingFullscreen) {
			SDL_SetWindowPosition(m_PrimaryWindow.get(), windowPrevPositionX, windowPrevPositionY);
			m_MultiScreenWindows.clear();
			m_MultiScreenRenderers.clear();
			m_MultiScreenTextures.clear();
			m_MultiScreenTextureOffsets.clear();
			return false;
		}

		CreateMultiDisplayTextures();
		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);

		// CBA to do figure out letterboxing for multiple displays, so just fix the resolution.
		//if (actualResX != resX * resMultiplier || actualResY != resY * resMultiplier) {
		//	ShowMessageBox("Desired reolution would lead to letterboxing, adjusting to fill entire displays.");
		//	resX = actualResX / resMultiplier;
		//	resY = actualResY / resMultiplier;
		//}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Update() {
		m_FrameLostFocus = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::HandleWindowEvent(const SDL_Event &windowEvent) {
		switch (windowEvent.window.event) {
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				if (!m_FrameLostFocus) {
					DisplaySwitchIn();
				}
				m_AnyWindowHasFocus = true;
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				m_AnyWindowHasFocus = false;
				m_FrameLostFocus = true;
				DisplaySwitchOut();
				break;
			case SDL_WINDOWEVENT_ENTER:
				if (m_AnyWindowHasFocus && CoversMultiDisplayFullscreen()) {
					SDL_RaiseWindow(SDL_GetWindowFromID(windowEvent.window.windowID));
					SDL_SetWindowInputFocus(SDL_GetWindowFromID(windowEvent.window.windowID));
					m_AnyWindowHasFocus = true;
				}
				break;
			case SDL_WINDOWEVENT_MOVED:
				if (windowEvent.window.windowID == SDL_GetWindowID(m_PrimaryWindow.get())) {
					m_PrimaryWindowDisplayIndex = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ClearRenderer() const {
		if (m_MultiScreenRenderers.empty()) {
			SDL_RenderClear(m_PrimaryRenderer.get());
		} else {
			for (const auto &renderer : m_MultiScreenRenderers) {
				SDL_RenderClear(renderer.get());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::UploadFrame() const {
		const BITMAP *backbuffer = g_FrameMan.GetBackBuffer32();

		if (m_MultiScreenTextureOffsets.empty()) {
			SDL_UpdateTexture(m_PrimaryTexture.get(), nullptr, backbuffer->line[0], backbuffer->w * 4);
			SDL_RenderCopy(m_PrimaryRenderer.get(), m_PrimaryTexture.get(), nullptr, nullptr);
			SDL_RenderPresent(m_PrimaryRenderer.get());
		} else {
			for (size_t i = 0; i < m_MultiScreenTextureOffsets.size(); ++i) {
				int displayOffsetX = m_MultiScreenTextureOffsets[i].x * 4;
				SDL_UpdateTexture(m_MultiScreenTextures[i].get(), nullptr, backbuffer->line[m_MultiScreenTextureOffsets[i].y] + displayOffsetX, backbuffer->w * 4);
				SDL_RenderCopy(m_MultiScreenRenderers[i].get(), m_MultiScreenTextures[i].get(), nullptr, nullptr);
				SDL_RenderPresent(m_MultiScreenRenderers[i].get());
			}
		}
	}
}