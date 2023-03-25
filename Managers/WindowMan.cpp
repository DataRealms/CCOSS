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
		m_NumDisplays = SDL_GetNumVideoDisplays();

		SDL_Rect primaryDisplayBounds;
		SDL_GetDisplayBounds(0, &primaryDisplayBounds);

		m_PrimaryDisplayResX = primaryDisplayBounds.w;
		m_PrimaryDisplayResY = primaryDisplayBounds.h;

		for (int i = 0; i < m_NumDisplays; ++i) {
			SDL_Rect res;
			if (SDL_GetDisplayBounds(i, &res) != 0) {
				g_ConsoleMan.PrintString("ERROR: Failed to get resolution of display " + std::to_string(i));
				continue;
			}

			if (res.x + res.w > m_MaxResX) {
				m_MaxResX = res.x + res.w;
			}
			if (res.y + res.h > m_MaxResY) {
				m_MaxResY = res.y + res.h;
			}
		}

		ValidateResolution(m_ResX, m_ResY, m_ResMultiplier);

		CreatePrimaryWindow();
		CreatePrimaryRenderer();
		CreatePrimaryTexture();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryWindow() {
		const char *windowTitle = "Cortex Command Community Project";
		int windowPosX = (m_ResX * m_ResMultiplier <= m_PrimaryDisplayResX) ? SDL_WINDOWPOS_CENTERED : (m_MaxResX - (m_ResX * m_ResMultiplier)) / 2;
		int windowPosY = SDL_WINDOWPOS_CENTERED;
		int windowFlags = SDL_WINDOW_SHOWN;

		if (CoversPrimaryFullscreen()) {
			// SDL_WINDOWPOS_UNDEFINED gives us the same result SDL_WINDOWPOS_CENTERED, so use 0 for proper top left corner of primary screen.
			// TODO: This is temp cause this won't work for any setup where the leftmost screen is not primary.
			//windowPosX = (m_MaxResX - (m_ResX * m_ResMultiplier)) / 2;
			//windowPosY = (m_MaxResY - (m_ResY * m_ResMultiplier)) / 2;
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_PrimaryWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>(SDL_CreateWindow(windowTitle, windowPosX, windowPosY, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, windowFlags));
		if (!m_PrimaryWindow) {
			ShowMessageBox("Unable to create window because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults!");

			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
			g_SettingsMan.SetSettingsNeedOverwrite();

			m_PrimaryWindow = std::unique_ptr<SDL_Window, SDLWindowDeleter>(SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, SDL_WINDOW_SHOWN));
			if (!m_PrimaryWindow) {
				RTEAbort("Failed to create window because:\n" + std::string(SDL_GetError()));
			}
		}

		if (CoversMultiDisplayFullscreen()) {
			SetWindowMultiFullscreen(m_ResX, m_ResY, m_ResMultiplier);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryRenderer() {
		int renderFlags = SDL_RENDERER_ACCELERATED;
		if (m_EnableVSync) {
			renderFlags |= SDL_RENDERER_PRESENTVSYNC;
		}

		m_PrimaryRenderer = std::unique_ptr<SDL_Renderer, SDLRendererDeleter>(SDL_CreateRenderer(m_PrimaryWindow.get(), -1, renderFlags));
		if (!m_PrimaryRenderer) {
			ShowMessageBox("Unable to create hardware accelerated renderer because:\n" + std::string(SDL_GetError()) + "!\n\nTrying to revert to software rendering!");
			m_PrimaryRenderer = std::unique_ptr<SDL_Renderer, SDLRendererDeleter>(SDL_CreateRenderer(m_PrimaryWindow.get(), -1, SDL_RENDERER_SOFTWARE));

			if (!m_PrimaryRenderer) {
				RTEAbort("Failed to initialize renderer!\nAre you sure this is a computer?");
			}
		}

		SDL_RenderSetIntegerScale(m_PrimaryRenderer.get(), SDL_TRUE);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::CreatePrimaryTexture() {
		if (m_MultiScreenTextureOffsets.empty()) {
			m_PrimaryTexture = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(SDL_CreateTexture(m_PrimaryRenderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_ResX, m_ResY));
			SDL_RenderSetLogicalSize(m_PrimaryRenderer.get(), m_ResX, m_ResY);
		} else {
			m_MultiScreenTextures.resize(m_MultiScreenTextureOffsets.size());
			for (size_t i = 0; i < m_MultiScreenTextures.size(); ++i) {
				SDL_Renderer *renderer;
				if (i == 0) {
					renderer = m_PrimaryRenderer.get();
				} else {
					renderer = m_MultiScreenRenderers[i - 1].get();
				}
				m_MultiScreenTextures[i] = std::unique_ptr<SDL_Texture, SDLTextureDeleter>(
					SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_MultiScreenTextureOffsets[i].w, m_MultiScreenTextureOffsets[i].h));
				if (!m_MultiScreenTextures[i]) {
					RTEAbort("Failed to create texture for multi-display: " + std::string(SDL_GetError()));
				}
				SDL_RenderSetLogicalSize(renderer, m_MultiScreenTextureOffsets[i].w, m_MultiScreenTextureOffsets[i].h);
			}
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
		if (settingsNeedOverwrite) { g_SettingsMan.SetSettingsNeedOverwrite(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::SetVSyncEnabled(bool enable) {
		m_EnableVSync = enable;

		int sdlEnable = m_EnableVSync ? SDL_TRUE : SDL_FALSE;
		int result = -1;

#if	SDL_MINOR_VERSION > 0 || (SDL_MINOR_VERSION == 0 && SDL_PATCHLEVEL >= 18)
		// Setting VSync per renderer is introduced in 2.0.18. Check minor version >0 as well because numbering scheme changed with the release of 2.24.0 (would be 2.0.24 with the previous scheme).
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

		ClearFrame();

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
		CreatePrimaryTexture();
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

		ClearFrame();

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

	bool WindowMan::SetWindowMultiFullscreen(int &resX, int &resY, int resMultiplier) {
		m_MultiScreenTextureOffsets.clear();
		m_MultiScreenTextures.clear();
		m_MultiScreenRenderers.clear();
		m_MultiScreenWindows.clear();

		int windowDisplay = SDL_GetWindowDisplayIndex(m_PrimaryWindow.get());

		SDL_Rect windowDisplayBounds;
		SDL_GetDisplayBounds(windowDisplay, &windowDisplayBounds);

		std::vector<std::pair<int, SDL_Rect>> displayBounds(m_NumDisplays);
		for (int i = 0; i < m_NumDisplays; ++i) {
			displayBounds[i].first = i;
			SDL_GetDisplayBounds(i, &displayBounds[i].second);
		}
		std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) {
			return left.second.x < right.second.x;
		});
		std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) {
			return left.second.y < right.second.y;
		});

		std::vector<std::pair<int, SDL_Rect>>::iterator displayPos = std::find_if(displayBounds.begin(), displayBounds.end(), [windowDisplay](auto display) {
			return display.first == windowDisplay;
		});

		int index = displayPos - displayBounds.begin();

		int actualResX = 0;
		int actualResY = 0;
		int topLeftX = windowDisplayBounds.x;
		int topLeftY = windowDisplayBounds.y;

		for (; index < m_NumDisplays && (actualResY < resY * resMultiplier || actualResX < resX * resMultiplier); ++index) {
			if (displayBounds[index].second.x < topLeftX || displayBounds[index].second.y < topLeftY || displayBounds[index].second.x - topLeftX > resX * resMultiplier || displayBounds[index].second.y - topLeftY > resY * resMultiplier) {
				continue;
			}
			if (actualResX < displayBounds[index].second.x - topLeftX + displayBounds[index].second.w) {
				actualResX = displayBounds[index].second.x - topLeftX + displayBounds[index].second.w;
			}
			if (actualResY < displayBounds[index].second.y - topLeftY + displayBounds[index].second.h) {
				actualResY = displayBounds[index].second.y - topLeftY + displayBounds[index].second.h;
			}
			if (index != displayPos - displayBounds.begin()) {
				m_MultiScreenWindows.emplace_back(SDL_CreateWindow("", displayBounds[index].second.x, displayBounds[index].second.y, displayBounds[index].second.w, displayBounds[index].second.h, SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR));
				if (!m_MultiScreenWindows.back()) {
					actualResX = -1;
					actualResY = -1;
					break;
				}
				m_MultiScreenRenderers.emplace_back(SDL_CreateRenderer(m_MultiScreenWindows.back().get(), -1, SDL_RENDERER_ACCELERATED));
				if (!m_MultiScreenRenderers.back()) {
					actualResX = -1;
					actualResY = -1;
					break;
				}
			}
			m_MultiScreenTextureOffsets.emplace_back(SDL_Rect {
				(displayBounds[index].second.x - topLeftX) / resMultiplier,
				(displayBounds[index].second.y - topLeftY) / resMultiplier,
				displayBounds[index].second.w / resMultiplier,
				displayBounds[index].second.h / resMultiplier
			});
		}

		if (actualResX < resX * resMultiplier || actualResY < resY * resMultiplier) {
			int maxResX = displayBounds.back().second.x - topLeftX + displayBounds.back().second.w;
			int maxResY = displayBounds.back().second.y - topLeftY + displayBounds.back().second.h;
			ShowMessageBox("Won't be able to fit the desired resolution onto the displays. Maximum resolution from here is: " + std::to_string(maxResX) + "x" + std::to_string(maxResY) + "\n Please move the window to the display you want to be the top left corner and try again.");
		}

		if (actualResX == -1 || actualResY == -1) {
			m_MultiScreenWindows.clear();
			m_MultiScreenRenderers.clear();
			m_MultiScreenTextures.clear();
			m_MultiScreenTextureOffsets.clear();
			return false;
		}

		// CBA to do figure out letterboxing for multiple displays, so just fix the resolution.
		if (actualResX != resX * resMultiplier || actualResY != resY * resMultiplier) {
			ShowMessageBox("Desired reolution would lead to letterboxing, adjusting to fill entire displays.");
			resX = actualResX / resMultiplier;
			resY = actualResY / resMultiplier;
		}

		SDL_SetWindowFullscreen(m_PrimaryWindow.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);

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
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ClearFrame() const {
		SDL_RenderClear(m_PrimaryRenderer.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::UploadFrame() const {
		const BITMAP *backbuffer = g_FrameMan.GetBackBuffer32();

		if (m_MultiScreenTextureOffsets.empty()) {
			SDL_UpdateTexture(m_PrimaryTexture.get(), nullptr, backbuffer->line[0], backbuffer->w * 4);
			SDL_RenderCopy(m_PrimaryRenderer.get(), m_PrimaryTexture.get(), nullptr, nullptr);
			SDL_RenderPresent(m_PrimaryRenderer.get());
		} else {
			SDL_Renderer *renderer;
			for (size_t i = 0; i < m_MultiScreenTextureOffsets.size(); ++i) {
				renderer = (i == 0) ? m_PrimaryRenderer.get() : m_MultiScreenRenderers[i - 1].get();

				SDL_UpdateTexture(m_MultiScreenTextures[i].get(), nullptr, backbuffer->line[0] + (m_MultiScreenTextureOffsets[i].x + backbuffer->w * m_MultiScreenTextureOffsets[i].y) * 4, backbuffer->w * 4);
				SDL_RenderCopy(renderer, m_MultiScreenTextures[i].get(), nullptr, nullptr);
				SDL_RenderPresent(renderer);
			}
		}
	}
}