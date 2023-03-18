#include "WindowMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"

#include "SDL.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_query.hpp"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::Clear() {
		m_Window.reset();
		m_MultiWindows.clear();

		m_NumScreens = SDL_GetNumVideoDisplays();
		m_MaxResX = 0;
		m_MaxResY = 0;
		m_PrimaryScreenResX = 0;
		m_PrimaryScreenResY = 0;
		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;

		m_EnableVSync = false;
		m_Fullscreen = false;
		m_DisableMultiScreenResolutionValidation = false;

		m_ResChanged = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WindowMan::Initialize() {
		m_NumScreens = SDL_GetNumVideoDisplays();

		SDL_Rect primaryDisplayBounds;
		SDL_GetDisplayBounds(0, &primaryDisplayBounds);

		m_PrimaryScreenResX = primaryDisplayBounds.w;
		m_PrimaryScreenResY = primaryDisplayBounds.h;

		for (int i = 0; i < m_NumScreens; ++i) {
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

		ValidateResolution(m_ResX, m_ResY, m_ResMultiplier, m_Fullscreen);

		int windowFlags = SDL_WINDOW_SHOWN;
		if (m_Fullscreen) {
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_Window = std::unique_ptr<SDL_Window, SDLWindowDeleter>(
			SDL_CreateWindow("Cortex Command Community Project",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			m_ResX * m_ResMultiplier,
			m_ResY * m_ResMultiplier,
			windowFlags));

		if (!m_Window) {
			ShowMessageBox("Unable to create window because: " + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults");
			m_Window = std::unique_ptr<SDL_Window, SDLWindowDeleter>(SDL_CreateWindow(
				"Cortex Command Community Project",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				c_DefaultResX,
				c_DefaultResY,
				SDL_WINDOW_SHOWN));

			if (!m_Window) {
				RTEAbort("Failed to create a window because: " + std::string(SDL_GetError()));
			}
			m_Fullscreen = false;
			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
		}

		SDL_SetWindowMinimumSize(m_Window.get(), 640, 480);

		if (m_Fullscreen) {
			if (m_NumScreens == 1) {
				SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
			} else {
				if (!SetWindowMultiFullscreen(m_ResX, m_ResY, m_ResMultiplier)) {
					SDL_SetWindowFullscreen(m_Window.get(), SDL_FALSE);
					m_Fullscreen = false;
					m_ResX = c_DefaultResX;
					m_ResY = c_DefaultResY;
					SDL_SetWindowSize(m_Window.get(), m_ResX, m_ResY);
					SDL_SetWindowPosition(m_Window.get(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);
				}
			}
		}

		return true;
	}











/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WindowMan::SetWindowMultiFullscreen(int &resX, int &resY, int resMultiplier) {
		//m_MultiRenderers.clear();
		//m_MultiWindows.clear();
		//m_TextureOffsets.clear();

		//int windowW = resX * resMultiplier;
		//int windowH = resY * resMultiplier;
		//int windowDisplay = SDL_GetWindowDisplayIndex(m_Window.get());
		//SDL_Rect windowDisplayBounds;
		//SDL_GetDisplayBounds(windowDisplay, &windowDisplayBounds);
		//std::vector<std::pair<int, SDL_Rect>> displayBounds(m_NumScreens);
		//for (int i = 0; i < m_NumScreens; ++i) {
		//	displayBounds[i].first = i;
		//	SDL_GetDisplayBounds(i, &displayBounds[i].second);
		//}
		//std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) { return left.second.x < right.second.x; });
		//std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) { return left.second.y < right.second.y; });
		//std::vector<std::pair<int, SDL_Rect>>::iterator displayPos = std::find_if(displayBounds.begin(), displayBounds.end(), [windowDisplay](auto display) {return display.first == windowDisplay; });

		//int index = displayPos - displayBounds.begin();

		//int actualResX = 0;
		//int actualResY = 0;
		//int topLeftX = windowDisplayBounds.x;
		//int topLeftY = windowDisplayBounds.y;


		//for (; index < m_NumScreens && (actualResY < resY * resMultiplier || actualResX < resX * resMultiplier); ++index) {
		//	if (displayBounds[index].second.x < topLeftX || displayBounds[index].second.y < topLeftY ||
		//		displayBounds[index].second.x - topLeftX > resX * resMultiplier || displayBounds[index].second.y - topLeftY > resY * resMultiplier) {
		//		continue;
		//	}
		//	if (actualResX < displayBounds[index].second.x - topLeftX + displayBounds[index].second.w) {
		//		actualResX = displayBounds[index].second.x - topLeftX + displayBounds[index].second.w;
		//	}
		//	if (actualResY < displayBounds[index].second.y - topLeftY + displayBounds[index].second.h) {
		//		actualResY = displayBounds[index].second.y - topLeftY + displayBounds[index].second.h;
		//	}
		//	if (index != displayPos - displayBounds.begin()) {
		//		m_MultiWindows.emplace_back(SDL_CreateWindow("",
		//			displayBounds[index].second.x,
		//			displayBounds[index].second.y,
		//			displayBounds[index].second.w,
		//			displayBounds[index].second.h,
		//			SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR));
		//		if (!m_MultiWindows.back()) {
		//			actualResX = -1;
		//			actualResY = -1;
		//			break;
		//		}
		//		m_MultiRenderers.emplace_back(SDL_CreateRenderer(
		//			m_MultiWindows.back().get(),
		//			-1,
		//			SDL_RENDERER_ACCELERATED));
		//		if (!m_MultiRenderers.back()) {
		//			actualResX = -1;
		//			actualResY = -1;
		//			break;
		//		}
		//	}
		//	m_TextureOffsets.emplace_back(SDL_Rect{
		//		(displayBounds[index].second.x - topLeftX) / resMultiplier,
		//		(displayBounds[index].second.y - topLeftY) / resMultiplier,
		//		displayBounds[index].second.w / resMultiplier,
		//		displayBounds[index].second.h / resMultiplier });
		//}
		//if (actualResX < resX * resMultiplier || actualResY < resY * resMultiplier) {
		//	int maxResX = displayBounds.back().second.x - topLeftX + displayBounds.back().second.w;
		//	int maxResY = displayBounds.back().second.y - topLeftY + displayBounds.back().second.h;
		//	ShowMessageBox("Won't be able to fit the desired resolution onto the displays. Maximum resolution from here is: " + std::to_string(maxResX) + "x" + std::to_string(maxResY) + "\n Please move the window to the display you want to be the top left corner and try again.");
		//}
		//if (actualResX == -1 || actualResY == -1) {
		//	m_MultiWindows.clear();
		//	m_MultiRenderers.clear();
		//	m_TextureOffsets.clear();
		//	return false;
		//}
		////CBA to do figure out letterboxing for multiple displays, so just fix the resolution.
		//if (actualResX != resX * resMultiplier || actualResY != resY * resMultiplier) {
		//	ShowMessageBox("Desired reolution would lead to letterboxing, adjusting to fill entire displays.");
		//	resX = actualResX / resMultiplier;
		//	resY = actualResY / resMultiplier;
		//}

		//SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);

		return true;
	}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glm::vec4 WindowMan::GetViewportLetterbox(int resX, int resY, int windowW, int windowH) {
		float aspectRatio = resX / static_cast<float>(resY);
		int width = windowW;
		int height = width / aspectRatio + 0.5f;

		if (height > windowH) {
			height = windowH;
			width = height * aspectRatio + 0.5f;
		}

		int offsetX = (windowW / 2) - (width / 2);
		int offsetY = (windowH / 2) - (height / 2);
		return glm::vec4(offsetX, windowH - offsetY - height, width, height);
	}








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolutionMultiplier(int newMultiplier) {
		//if (newMultiplier <= 0 || newMultiplier > 4 || newMultiplier == m_ResMultiplier) {
		//	return;
		//}

		//if (m_ResX > m_MaxResX / newMultiplier || m_ResY > m_MaxResY / newMultiplier) {
		//	ShowMessageBox("Requested resolution multiplier will result in game window exceeding display bounds!\nNo change will be made!\n\nNOTE: To toggle fullscreen, use the button in the Options & Controls Menu!");
		//	return;
		//}

		//m_MultiDisplayTextures.clear();
		//m_TextureOffsets.clear();
		//m_MultiRenderers.clear();
		//m_MultiWindows.clear();

		//m_Fullscreen = (m_ResX * newMultiplier == m_MaxResX && m_ResY * newMultiplier == m_MaxResY);

		//if (m_Fullscreen) {
		//	if (m_NumScreens > 1) {
		//		if (!SetWindowMultiFullscreen(m_ResX, m_ResY, newMultiplier)) {
		//			m_Fullscreen = (m_ResX * m_ResMultiplier == m_MaxResX && m_ResY * m_ResMultiplier == m_MaxResY);
		//			SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
		//			if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
		//				RTEAbort("Unable to set back to previous windowed mode multiplier because: " + std::string(SDL_GetError()) + "!");
		//			}
		//			g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
		//			set_palette(m_Palette);
		//			return;
		//		}
		//	} else if (SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
		//		m_Fullscreen = (m_ResX * m_ResMultiplier == m_MaxResX && m_ResY * m_ResMultiplier == m_MaxResY);
		//		SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
		//		if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
		//			RTEAbort("Unable to set back to previous windowed mode multiplier because: " + std::string(SDL_GetError()) + "!");
		//		}
		//		g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
		//		set_palette(m_Palette);
		//		return;
		//	}
		//} else {
		//	SDL_SetWindowFullscreen(m_Window.get(), 0);
		//	SDL_RestoreWindow(m_Window.get());
		//	SDL_SetWindowBordered(m_Window.get(), SDL_TRUE);
		//	SDL_SetWindowSize(m_Window.get(), m_ResX * newMultiplier, m_ResY * newMultiplier);
		//	int displayIndex = SDL_GetWindowDisplayIndex(m_Window.get());
		//	SDL_SetWindowPosition(m_Window.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
		//}
		//m_ResMultiplier = newMultiplier;

		//set_palette(m_Palette);
		//// RecreateBackBuffers();

		//g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier.");
		//g_SettingsMan.UpdateSettingsFile();

		//ClearFrame();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ChangeResolution(int newResX, int newResY, bool upscaled, bool newFullscreen) {
		//int newResMultiplier = upscaled ? 2 : 1;

		//if (m_ResX == newResX && m_ResY == newResY && m_ResMultiplier == newResMultiplier && m_Fullscreen == newFullscreen) {
		//	return;
		//}
		//m_MultiDisplayTextures.clear();
		//m_TextureOffsets.clear();
		//m_MultiRenderers.clear();
		//m_MultiWindows.clear();

		//if (!newFullscreen) {
		//	SDL_RestoreWindow(m_Window.get());
		//}

		//ValidateResolution(newResX, newResY, newResMultiplier, newFullscreen);

		//if (newFullscreen &&
		//	((m_NumScreens > 1 && !SetWindowMultiFullscreen(newResX, newResY, newResMultiplier)) ||
		//	SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)) {

		//	SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
		//	if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
		//		RTEAbort("Unable to set back to previous resolution because: " + std::string(SDL_GetError()) + "!");
		//	}
		//	g_ConsoleMan.PrintString("ERROR: Failed to switch to new resolution, reverted back to previous setting!");
		//	set_palette(m_Palette);
		//	return;
		//} else if (!newFullscreen) {
		//	SDL_SetWindowFullscreen(m_Window.get(), 0);
		//	SDL_RestoreWindow(m_Window.get());
		//	SDL_SetWindowBordered(m_Window.get(), SDL_TRUE);
		//	SDL_SetWindowSize(m_Window.get(), newResX * newResMultiplier, newResY * newResMultiplier);
		//	int displayIndex = SDL_GetWindowDisplayIndex(m_Window.get());
		//	SDL_SetWindowPosition(m_Window.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
		//}
		//m_Fullscreen = newFullscreen;
		//m_ResX = newResX;
		//m_ResY = newResY;
		//m_ResMultiplier = newResMultiplier;

		//set_palette(m_Palette);
		//RecreateBackBuffers();

		//g_ConsoleMan.PrintString("SYSTEM: Switched to different resolution.");
		//g_SettingsMan.UpdateSettingsFile();

		//m_ResChanged = true;
		//ClearFrame();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::WindowResizedCallback(int newResX, int newResY) {
		//RTEAssert(!m_Fullscreen, "ERROR: Somehow the fullscreen window was resized!");
		//m_Fullscreen = false;
		//m_ResX = newResX / m_ResMultiplier;
		//m_ResY = newResY / m_ResMultiplier;

		//if (newResX < 640 || newResY < 480) {
		//	SDL_SetWindowSize(m_Window.get(), 640, 480);
		//	m_ResX = 640 / m_ResMultiplier;
		//	m_ResY = 480 / m_ResMultiplier;
		//}

		//set_palette(m_Palette);
		//RecreateBackBuffers();

		//m_ResChanged = true;

		//ClearFrame();
	}











/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchOut() {
		g_UInputMan.DisableMouseMoving(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::DisplaySwitchIn() {
		g_UInputMan.DisableMouseMoving(false);
		if (m_MultiWindows.size() > 0) {
			SDL_RaiseWindow(m_Window.get());
			for (auto &window : m_MultiWindows) {
				SDL_RaiseWindow(window.get());
			}
		}
	}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void WindowMan::ValidateResolution(int &resX, int &resY, int &resMultiplier, bool &newFullscreen) const {
		bool settingsNeedOverwrite = false;

		if (resX * resMultiplier > m_MaxResX || resY * resMultiplier > m_MaxResY) {
			settingsNeedOverwrite = true;
			resX = m_MaxResX / resMultiplier;
			resY = m_MaxResY / resMultiplier;
			newFullscreen = false;
			ShowMessageBox("Resolution too high to fit display, overriding to fit!");

		}
		if (m_NumScreens == 1) {
			float currentAspectRatio = static_cast<float>(resX) / static_cast<float>(resY);
			if (currentAspectRatio < 1 || currentAspectRatio > 4) {
				settingsNeedOverwrite = true;
				resX = c_DefaultResX;
				resY = c_DefaultResY;
				resMultiplier = 1;
				newFullscreen = false;
				ShowMessageBox("Abnormal aspect ratio detected! Reverting to defaults!");
			}
		}
		if (settingsNeedOverwrite) { g_SettingsMan.SetSettingsNeedOverwrite(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool WindowMan::ValidateMultiScreenResolution(int &resX, int &resY, int resMultiplier) const {
#if 0
		POINT pointOnScreen;
		HMONITOR screenHandle;
		MONITORINFO screenInfo = { sizeof(MONITORINFO) };

		pointOnScreen = { -1 , 0 };
		screenHandle = MonitorFromPoint(pointOnScreen, MONITOR_DEFAULTTONULL);
		if (screenHandle != NULL) {
			resX = m_PrimaryScreenResX / resMultiplier;
			resY = m_PrimaryScreenResY / resMultiplier;

			std::string leftNotPrimaryMessage = {
				"Due to limitations in Cortex Command's graphics API it is impossible to properly run multi-screen mode when the left-most screen is not set as primary.\n"
				"Please configure your left-most screen to be primary to utilize all screens, as the game window will extend right but will not extend left, leaving any screen left of the primary unused.\n\n"
				"You can disable multi-screen resolution validation in \"Settings.ini\" and run at your own risk!\n\nResolution settings will be overridden to fit primary screen only!"
			};
			ShowMessageBox(leftNotPrimaryMessage);
			return true;
		}

		pointOnScreen = { m_PrimaryScreenResX + 1 , 0 };
		screenHandle = MonitorFromPoint(pointOnScreen, MONITOR_DEFAULTTONULL);
		GetMonitorInfo(screenHandle, &screenInfo);

		if (m_PrimaryScreenResY != screenInfo.rcMonitor.bottom) {
			resX = m_PrimaryScreenResX / resMultiplier;
			resY = m_PrimaryScreenResY / resMultiplier;
			ShowMessageBox("Center screen height is not identical to primary screen, overriding to fit primary screen only!\n\nYou can disable multi-screen resolution validation in \"Settings.ini\" and run at your own risk!");
			return true;
		}

		if (m_NumScreens == 3) {
			pointOnScreen = { screenInfo.rcMonitor.right + 1 , 0 };
			screenHandle = MonitorFromPoint(pointOnScreen, MONITOR_DEFAULTTONULL);
			screenInfo = { sizeof(MONITORINFO) };
			GetMonitorInfo(screenHandle, &screenInfo);

			if (m_PrimaryScreenResY != screenInfo.rcMonitor.bottom) {
				resX = (m_MaxResX - (screenInfo.rcMonitor.right - screenInfo.rcMonitor.left)) / resMultiplier;
				resY = m_PrimaryScreenResY / resMultiplier;
				ShowMessageBox("Right screen height is not identical to primary screen, overriding to extend to center screen only!\n\nYou can disable multi-screen resolution validation in \"Settings.ini\" and run at your own risk!");
				return true;
			}
		}
#endif

		return false;
	}








/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SDLWindowDeleter::operator()(SDL_Window *window) const {
		SDL_DestroyWindow(window);
	}
}