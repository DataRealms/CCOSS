#include "FrameMan.h"

#include "PostProcessMan.h"
#include "PrimitiveMan.h"
#include "PerformanceMan.h"
#include "ActivityMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "UInputMan.h"

#include "SLTerrain.h"
#include "SLBackground.h"
#include "Scene.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"

#include <SDL.h>
#include "ScreenShader.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_query.hpp"

namespace RTE {

	void SdlWindowDeleter::operator()(SDL_Window* window) {
		SDL_DestroyWindow(window);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SdlContextDeleter::operator()(SDL_GLContext context) {
		SDL_GL_DeleteContext(context);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::s_DisableFrameBufferFlip = false;

	const std::array<std::function<void(int r, int g, int b, int a)>, DrawBlendMode::BlendModeCount> FrameMan::c_BlenderSetterFunctions = {
		nullptr, // NoBlend obviously has no blender, but we want to keep the indices matching with the enum.
		&set_burn_blender,
		&set_color_blender,
		&set_difference_blender,
		&set_dissolve_blender,
		&set_dodge_blender,
		&set_invert_blender,
		&set_luminance_blender,
		&set_multiply_blender,
		&set_saturation_blender,
		&set_screen_blender,
		nullptr // Transparency does not rely on the blender setting, it creates a map with the dedicated function instead of with the generic one.
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DisplaySwitchOut() {
		g_UInputMan.DisableMouseMoving(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DisplaySwitchIn() {
		g_UInputMan.DisableMouseMoving(false);
		if (m_MultiWindows.size() > 0) {
			SDL_RaiseWindow(m_Window.get());
			for (auto &window: m_MultiWindows) {
				SDL_RaiseWindow(window.get());
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FrameMan::FrameMan() { Clear(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FrameMan::~FrameMan() { Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Clear() {
		m_ScreenVertices = {
			1.0f, 1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f,};
		m_ScreenShader.reset();
		m_WindowView.resize(1);
		m_WindowView[0] = glm::mat4(1);
		m_WindowTransforms.resize(1);
		m_WindowTransforms[0] = glm::mat4(1);
		m_WindowViewport.resize(1);
		m_WindowViewport[0] = glm::vec4(1);
		m_ScreenTexture = 0;
		m_ScreenVBO = 0;
		m_ScreenVAO = 0;
		m_EnableVsync = -1;

		// GL Context needs to be destroyed after all GL objects and before the main window.
		m_GLContext.reset();
		m_Window.reset();
		m_MultiWindows.clear();

		m_GfxDriverMessage.clear();
		m_Fullscreen = false;
		m_DisableMultiScreenResolutionValidation = false;
		m_NumScreens = SDL_GetNumVideoDisplays();
		m_MaxResX = m_PrimaryScreenResX = 0;
		m_MaxResY = m_PrimaryScreenResY = 0;
		m_ResX = c_DefaultResX;
		m_ResY = c_DefaultResY;
		m_ResMultiplier = 1;
		m_ResChanged = false;
		m_HSplit = false;
		m_VSplit = false;
		m_TwoPlayerVSplit = false;
		m_PlayerScreen = nullptr;
		m_PlayerScreenWidth = 0;
		m_PlayerScreenHeight = 0;
		m_ScreenDumpBuffer = nullptr;
		m_WorldDumpBuffer = nullptr;
		m_ScenePreviewDumpGradient = nullptr;
		m_BackBuffer8 = nullptr;
		m_BackBuffer32 = nullptr;
		m_OverlayBitmap32 = nullptr;
		m_DrawNetworkBackBuffer = false;
		m_StoreNetworkBackBuffer = false;
		m_NetworkFrameCurrent = 0;
		m_NetworkFrameReady = 1;
		m_PaletteFile = ContentFile("Base.rte/palette.bmp");
		m_BlackColor = 245;
		m_AlmostBlackColor = 245;
		m_ColorTablePruneTimer.Reset();
		m_GUIScreen = nullptr;
		m_LargeFont = nullptr;
		m_SmallFont = nullptr;
		m_TextBlinkTimer.Reset();

		m_TempBackBuffer8 = nullptr;
		m_TempBackBuffer32 = nullptr;
		m_TempOverlayBitmap32 = nullptr;
		m_TempPlayerScreen = nullptr;

		for (int screenCount = 0; screenCount < c_MaxScreenCount; ++screenCount) {
			m_ScreenText[screenCount].clear();
			m_TextDuration[screenCount] = -1;
			m_TextDurationTimer[screenCount].Reset();
			m_TextBlinking[screenCount] = 0;
			m_TextCentered[screenCount] = false;
			m_FlashScreenColor[screenCount] = -1;
			m_FlashedLastFrame[screenCount] = false;
			m_FlashTimer[screenCount].Reset();

			for (int bufferFrame = 0; bufferFrame < 2; bufferFrame++) {
				m_NetworkBackBufferIntermediate8[bufferFrame][screenCount] = nullptr;
				m_NetworkBackBufferFinal8[bufferFrame][screenCount] = nullptr;
				m_NetworkBackBufferIntermediateGUI8[bufferFrame][screenCount] = nullptr;
				m_NetworkBackBufferFinalGUI8[bufferFrame][screenCount] = nullptr;

				m_TempNetworkBackBufferIntermediate8[bufferFrame][screenCount] = nullptr;
				m_TempNetworkBackBufferIntermediateGUI8[bufferFrame][screenCount] = nullptr;
				m_TempNetworkBackBufferFinal8[bufferFrame][screenCount] = nullptr;
				m_TempNetworkBackBufferFinalGUI8[bufferFrame][screenCount] = nullptr;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::PrintForcedGfxDriverMessage() const {
		if (!m_GfxDriverMessage.empty()) { g_ConsoleMan.PrintString(m_GfxDriverMessage); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ValidateResolution(int &resX, int &resY, int &resMultiplier) const {
		bool settingsNeedOverwrite = false;

		if (resX * resMultiplier > m_MaxResX || resY * resMultiplier > m_MaxResY) {
			settingsNeedOverwrite = true;
			resX = m_MaxResX / resMultiplier;
			resY = m_MaxResY / resMultiplier;
			ShowMessageBox("Resolution too high to fit display, overriding to fit!");

		}
		if (m_NumScreens == 1) {
			float currentAspectRatio = static_cast<float>(resX) / static_cast<float>(resY);
			if (currentAspectRatio < 1 || currentAspectRatio > 4) {
				settingsNeedOverwrite = true;
				resX = c_DefaultResX;
				resY = c_DefaultResY;
				resMultiplier = 1;
				ShowMessageBox("Abnormal aspect ratio detected! Reverting to defaults!");
			}
		} else if (!m_DisableMultiScreenResolutionValidation && m_NumScreens > 1 && m_NumScreens < 4) {
			if (resX * resMultiplier > m_PrimaryScreenResX || resY * resMultiplier > m_PrimaryScreenResY) {
				settingsNeedOverwrite = ValidateMultiScreenResolution(resX, resY, resMultiplier);
			}
		} else if (!m_DisableMultiScreenResolutionValidation && m_NumScreens > 3) {
			settingsNeedOverwrite = true;
			resX = c_DefaultResX;
			resY = c_DefaultResY;
			resMultiplier = 1;
			ShowMessageBox("Number of screens is too damn high! Overriding to defaults!\n\nPlease disable multi-screen resolution validation in \"Settings.ini\" and run at your own risk!");
		}

		if (settingsNeedOverwrite) { g_SettingsMan.SetSettingsNeedOverwrite(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::ValidateMultiScreenResolution(int &resX, int &resY, int resMultiplier) const {
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

	int FrameMan::Initialize() {

		m_NumScreens = SDL_GetNumVideoDisplays();
		m_MaxResX = 0;
		m_MaxResY = 0;
		SDL_Rect primaryDisplayBounds;
		SDL_GetDisplayBounds(0, &primaryDisplayBounds);
		m_PrimaryScreenResX = primaryDisplayBounds.w;
		m_PrimaryScreenResY = primaryDisplayBounds.h;
		for (int i = 0; i < m_NumScreens; ++i) {
			SDL_Rect res;
			if (SDL_GetDisplayBounds(i, &res) != 0) {
				g_ConsoleMan.PrintString("Failed to get resolution of display " + std::to_string(i));
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

		int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
		if (m_Fullscreen) {
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		m_Window = std::unique_ptr<SDL_Window, SdlWindowDeleter>(
		    SDL_CreateWindow("Cortex Command Community Project",
		        SDL_WINDOWPOS_CENTERED,
		        SDL_WINDOWPOS_CENTERED,
		        m_ResX * m_ResMultiplier,
		        m_ResY * m_ResMultiplier,
		        windowFlags));

		if (!m_Window) {
			ShowMessageBox("Unable to create window because: " + std::string(SDL_GetError()) + "!\n\nTrying to revert to defaults");
			m_Window = std::unique_ptr<SDL_Window, SdlWindowDeleter>(SDL_CreateWindow(
			    "Cortex Command Community Project",
			    SDL_WINDOWPOS_CENTERED,
			    SDL_WINDOWPOS_CENTERED,
			    c_DefaultResX,
			    c_DefaultResY,
			    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN));

			if (!m_Window) {
				RTEAbort("Failed to create a window because: " + std::string(SDL_GetError()));
			}
			m_Fullscreen = false;
			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		m_GLContext = std::unique_ptr<void, SdlContextDeleter>(static_cast<void *>(SDL_GL_CreateContext(m_Window.get())));
		int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

		if (version == 0 || (GLAD_VERSION_MAJOR(version) < 3 && GLAD_VERSION_MINOR(version) < 3)) {
			RTEAbort("Failed to load OpenGL");
		}

		if (SDL_GL_SetSwapInterval(m_EnableVsync)) {
			g_ConsoleMan.PrintString("Unsupported Vsync value, falling back to basic Vsync.");
			m_EnableVsync = 1;
			SDL_GL_SetSwapInterval(1);
		}

		m_ScreenShader = std::make_unique<ScreenShader>();
		GL_CHECK(glGenTextures(1, &m_ScreenTexture););
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_ScreenTexture););
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT););
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT););
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST););
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST););
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0););

		GL_CHECK(glGenBuffers(1, &m_ScreenVBO););
		GL_CHECK(glGenVertexArrays(1, &m_ScreenVAO););
		GL_CHECK(glBindVertexArray(m_ScreenVAO););
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO););
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_ScreenVertices.size(), m_ScreenVertices.data(), GL_STATIC_DRAW););
		GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), 0););
		GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), (void *)(2 * sizeof(float))););
		GL_CHECK(glEnableVertexAttribArray(0););
		GL_CHECK(glEnableVertexAttribArray(1););
		GL_CHECK(glBindVertexArray(0););
		set_color_depth(m_BPP);

		int windowW = m_ResX * m_ResMultiplier;
		int windowH = m_ResY * m_ResMultiplier;
		if (m_Fullscreen) {
			SDL_GL_GetDrawableSize(m_Window.get(), &windowW, &windowH);
		}
		GL_CHECK(glViewport(0, 0, windowW, windowH););

		m_WindowViewport[0] = GetViewportLetterbox(m_ResX, m_ResY, windowW, windowH);
		m_WindowView[0] = glm::ortho<float>(0.0f, m_WindowViewport[0].z, 0.0f, m_WindowViewport[0].w, -1.0f, 1.0f);
		m_WindowTransforms[0] = glm::mat4(1.0f);
		
		GL_CHECK(glEnable(GL_DEPTH_TEST););

		if (m_NumScreens > 1 && m_Fullscreen) {
			CreateFullscreenMultiWindows(m_ResX, m_ResY, m_ResMultiplier);
		}

		// Sets the allowed color conversions when loading bitmaps from files
		set_color_conversion(COLORCONV_MOST);

		LoadPalette(m_PaletteFile.GetDataPath());

		// Store the default palette for re-use when creating new color tables for different blend modes because the palette can be changed via scripts, and handling per-palette per-mode color tables is too much headache.
		get_palette(m_DefaultPalette);

		CreatePresetColorTables();
		SetTransTableFromPreset(TransparencyPreset::HalfTrans);
		CreateBackBuffers();
		ClearFrame();

		ContentFile scenePreviewGradientFile("Base.rte/GUIs/PreviewSkyGradient.png");
		m_ScenePreviewDumpGradient = scenePreviewGradientFile.GetAsBitmap(COLORCONV_8_TO_32, false);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CreateBackBuffers() {
		// Create the back buffer, this is still in 8bpp, we will do any post-processing on the PostProcessing bitmap
		m_BackBuffer8 = create_bitmap_ex(8, m_ResX, m_ResY);
		ClearBackBuffer8();

		// Create the post-processing buffer, it'll be used for glow effects etc
		m_BackBuffer32 = create_bitmap_ex(32, m_ResX, m_ResY);
		ClearBackBuffer32();

		m_OverlayBitmap32 = create_bitmap_ex(32, m_ResX, m_ResY);
		clear_to_color(m_OverlayBitmap32, 0);

		// Create all the network 8bpp back buffers
		for (int i = 0; i < c_MaxScreenCount; i++) {
			for (int f = 0; f < 2; f++) {
				m_NetworkBackBufferIntermediate8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
				clear_to_color(m_NetworkBackBufferIntermediate8[f][i], m_BlackColor);

				m_NetworkBackBufferIntermediateGUI8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
				clear_to_color(m_NetworkBackBufferIntermediateGUI8[f][i], g_MaskColor);

				m_NetworkBackBufferFinal8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
				clear_to_color(m_NetworkBackBufferFinal8[f][i], m_BlackColor);

				m_NetworkBackBufferFinalGUI8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
				clear_to_color(m_NetworkBackBufferFinalGUI8[f][i], g_MaskColor);
			}
		}

		m_PlayerScreenWidth = m_BackBuffer8->w;
		m_PlayerScreenHeight = m_BackBuffer8->h;

		// Create the splitscreen buffer
		if (m_HSplit || m_VSplit) {
			m_PlayerScreen = create_bitmap_ex(8, m_ResX / (m_VSplit ? 2 : 1), m_ResY / (m_HSplit ? 2 : 1));
			clear_to_color(m_PlayerScreen, m_BlackColor);
			set_clip_state(m_PlayerScreen, 1);

			// Update these to represent the split screens
			m_PlayerScreenWidth = m_PlayerScreen->w;
			m_PlayerScreenHeight = m_PlayerScreen->h;
		}

		m_ScreenDumpBuffer = create_bitmap_ex(24, m_BackBuffer32->w, m_BackBuffer32->h);

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_ScreenTexture););
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_BackBuffer32->w, m_BackBuffer32->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL););
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0););

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::CreatePresetColorTables() {
		// Create RGB lookup table that supposedly speeds up calculation of other color tables.
		create_rgb_table(&m_RGBTable, m_DefaultPalette, nullptr);
		rgb_map = &m_RGBTable;

		// Create transparency color tables. Tables for other blend modes will be created on demand.
		int transparencyPresetCount = BlendAmountLimits::MaxBlend / c_BlendAmountStep;
		for (int index = 0; index <= transparencyPresetCount; ++index) {
			int presetBlendAmount = index * c_BlendAmountStep;
			std::array<int, 4> colorChannelBlendAmounts = { presetBlendAmount, presetBlendAmount, presetBlendAmount, BlendAmountLimits::MinBlend };
			int adjustedBlendAmount = 255 - (static_cast<int>(255.0F * (1.0F / static_cast<float>(transparencyPresetCount) * static_cast<float>(index))));

			m_ColorTables.at(DrawBlendMode::BlendTransparency).try_emplace(colorChannelBlendAmounts);
			create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedBlendAmount, adjustedBlendAmount, adjustedBlendAmount, nullptr);
			m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::RecreateBackBuffers() {
		m_TempBackBuffer8 = m_BackBuffer8;
		m_TempBackBuffer32 = m_BackBuffer32;
		m_TempOverlayBitmap32 = m_OverlayBitmap32;

		for (int i = 0; i < c_MaxScreenCount; i++) {
			for (int f = 0; f < 2; f++) {
				m_TempNetworkBackBufferIntermediate8[f][i] = m_NetworkBackBufferIntermediate8[f][i];
				m_TempNetworkBackBufferIntermediateGUI8[f][i] = m_NetworkBackBufferIntermediateGUI8[f][i];
				m_TempNetworkBackBufferFinal8[f][i] = m_NetworkBackBufferFinal8[f][i];
				m_TempNetworkBackBufferFinalGUI8[f][i] = m_NetworkBackBufferFinalGUI8[f][i];
			}
		}
		if (m_HSplit || m_VSplit) { m_TempPlayerScreen = m_PlayerScreen; }

		CreateBackBuffers();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glm::vec4 FrameMan::GetViewportLetterbox(int resX, int resY, int windowW, int windowH) {
		float aspectRatio = resX / static_cast<float>(resY);
		int width = windowW;
		int height = width / aspectRatio + 0.5f;

		if (height > windowH) {
			height = windowH;
			width = height * aspectRatio +0.5f;
		}

		int offsetX = (windowW / 2) - (width / 2);
		int offsetY = (windowH / 2) - (height / 2);
		return glm::vec4(offsetX, windowH - offsetY - height, width, height);
	}

	bool FrameMan::CreateFullscreenMultiWindows(int resX, int resY, int resMultiplier) {
		std::vector<std::pair<int, SDL_Rect>> displayBounds(m_NumScreens);
		for (int i = 0; i < m_NumScreens; ++i) {
			displayBounds[i].first = i;
			SDL_GetDisplayBounds(i, &displayBounds[i].second);
		}
		std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) { return left.second.x < right.second.x; });
		std::stable_sort(displayBounds.begin(), displayBounds.end(), [](auto left, auto right) { return left.second.y < right.second.y; });
		// Move main window to primary display (at 0,0).
		SDL_SetWindowFullscreen(m_Window.get(), 0);
		SDL_SetWindowPosition(m_Window.get(), 0, 0);
		SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP);
		int mainWindowDisplay = SDL_GetWindowDisplayIndex(m_Window.get());
		int actualResX = 0;
		int actualResY = 0;
		int index = 0;
		while (index < m_NumScreens && (actualResY < resY * resMultiplier || actualResX < resX * resMultiplier)) {
			if (displayBounds[index].second.x > resX * resMultiplier || displayBounds[index].second.y > resY * resMultiplier) {
				++index;
				continue;
			}
			if (actualResX < displayBounds[index].second.x + displayBounds[index].second.w) {
				actualResX = displayBounds[index].second.x + displayBounds[index].second.w;
			}
			if (actualResY < displayBounds[index].second.y + displayBounds[index].second.h) {
				actualResY = displayBounds[index].second.y + displayBounds[index].second.h;
			}

			glm::mat4 projection = glm::ortho<float>(0.0f, displayBounds[index].second.w, 0.0f, displayBounds[index].second.h, -1.0f, 1.0f);
			float width = resX * resMultiplier - displayBounds[index].second.x;
			float height = resY * resMultiplier - displayBounds[index].second.y;

			width = std::clamp<float>(width, 0.0f, displayBounds[index].second.w);
			height = std::clamp<float>(height, 0.0f, displayBounds[index].second.h);

			glm::mat4 uvTransform(1.0f);
			uvTransform = glm::translate<float>(uvTransform, {displayBounds[index].second.x / static_cast<float>(resX * resMultiplier), displayBounds[index].second.y / static_cast<float>(resY * resMultiplier), 0.0f});
			uvTransform = glm::scale(uvTransform, {width / static_cast<float>(resX * resMultiplier), height / static_cast<float>(resY * resMultiplier), 1.0f});

			if (displayBounds[index].first != mainWindowDisplay) {
				m_MultiWindows.emplace_back(SDL_CreateWindow("",
				    displayBounds[index].second.x,
				    displayBounds[index].second.y,
				    displayBounds[index].second.w,
				    displayBounds[index].second.h,
				    SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SKIP_TASKBAR));
				if (m_MultiWindows.back() == nullptr) {
					actualResX = -1;
					actualResY = -1;
					break;
				}

				m_WindowView.emplace_back(projection);
				m_WindowTransforms.emplace_back(uvTransform);
				m_WindowViewport.emplace_back(glm::vec4(0.0f,0.0f,displayBounds[index].second.w, displayBounds[index].second.h));
			} else {
				m_WindowView[0] = projection;
				m_WindowTransforms[0] = uvTransform;
				m_WindowViewport[0] = glm::vec4(0.0f, 0.0f, displayBounds[index].second.w, displayBounds[index].second.h);
			}
			index++;
		}
		if (actualResX != -1 || actualResX != -1) {
			return true;
		}
		m_MultiWindows.clear();
		m_WindowView.resize(1);
		m_WindowTransforms.resize(1);
		m_WindowView[0] = glm::ortho<float>(0.0f, displayBounds[0].second.w, 0.0f, displayBounds[0].second.h, -1.0f, 1.0f);
		m_WindowTransforms[0] = glm::mat4(1);
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Destroy() {
		destroy_bitmap(m_BackBuffer8);
		destroy_bitmap(m_BackBuffer32);
		destroy_bitmap(m_OverlayBitmap32);
		destroy_bitmap(m_PlayerScreen);
		destroy_bitmap(m_ScreenDumpBuffer);
		destroy_bitmap(m_WorldDumpBuffer);
		destroy_bitmap(m_ScenePreviewDumpGradient);

		for (int i = 0; i < c_MaxScreenCount; i++) {
			for (int f = 0; f < 2; f++) {
				destroy_bitmap(m_NetworkBackBufferIntermediate8[f][i]);
				destroy_bitmap(m_NetworkBackBufferIntermediateGUI8[f][i]);
				destroy_bitmap(m_NetworkBackBufferFinal8[f][i]);
				destroy_bitmap(m_NetworkBackBufferFinalGUI8[f][i]);
			}
		}

		delete m_GUIScreen;
		delete m_LargeFont;
		delete m_SmallFont;

		GL_CHECK(glDeleteTextures(1, &m_ScreenTexture););
		GL_CHECK(glDeleteVertexArrays(1, &m_ScreenVAO););
		GL_CHECK(glDeleteBuffers(1, &m_ScreenVBO););
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DestroyTempBackBuffers() {
		destroy_bitmap(m_TempBackBuffer8);
		destroy_bitmap(m_TempBackBuffer32);
		destroy_bitmap(m_TempOverlayBitmap32);
		destroy_bitmap(m_TempPlayerScreen);

		for (int i = 0; i < c_MaxScreenCount; i++) {
			for (int f = 0; f < 2; f++) {
				destroy_bitmap(m_TempNetworkBackBufferIntermediate8[f][i]);
				destroy_bitmap(m_TempNetworkBackBufferIntermediateGUI8[f][i]);
				destroy_bitmap(m_TempNetworkBackBufferFinal8[f][i]);
				destroy_bitmap(m_TempNetworkBackBufferFinalGUI8[f][i]);
			}
		}

		m_ResChanged = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Update() {
		// Remove all scheduled primitives, those will be re-added by updates from other entities.
		// This needs to happen here, otherwise if there are multiple sim updates during a single frame duplicates will be added to the primitive queue.
		g_PrimitiveMan.ClearPrimitivesQueue();

		// Prune unused color tables every 5 real minutes to prevent ridiculous memory usage over time.
		if (m_ColorTablePruneTimer.IsPastRealMS(300000)) {
			long long currentTime = g_TimerMan.GetAbsoluteTime() / 10000;
			for (std::unordered_map<std::array<int, 4>, std::pair<COLOR_MAP, long long>> &colorTableMap : m_ColorTables) {
				if (colorTableMap.size() >= 100) {
					std::vector<std::array<int, 4>> markedForDelete;
					markedForDelete.reserve(colorTableMap.size());
					for (const auto &[tableKey, tableData] : colorTableMap) {
						long long lastAccessTime = tableData.second;
						// Mark tables that haven't been accessed in the last minute for deletion. Avoid marking the transparency table presets, those will have lastAccessTime set to -1.
						if (lastAccessTime != -1 && (currentTime - lastAccessTime > 60)) { markedForDelete.emplace_back(tableKey); }
					}
					for (const std::array<int, 4> &keyToDelete : markedForDelete) {
						colorTableMap.erase(keyToDelete);
					}
				}
			}
			m_ColorTablePruneTimer.Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ChangeResolutionMultiplier(int newMultiplier) {
		if (newMultiplier <= 0 || newMultiplier > 4 || newMultiplier == m_ResMultiplier) {
			return;
		}

#if 0
		// This can be made to work but it doesn't really make any sense because regardless of the resolution or the multiplier it's still full screen.
		// It just switches back and forth between a crisp upscaled image and a blurry badly interpolated by the monitor image (if the un-upscaled resolution is even supported).
		// Windows only for now because Linux switches to dedicated fullscreen because lack of borderless and this won't allow it to switch back to windowed.
		if (IsUsingDedicatedGraphicsDriver()) {
			ShowMessageBox("Quick resolution multiplier change while running in dedicated fullscreen mode is not supported!\nNo change will be made!");
			return;
		}
#endif

		if (m_ResX > m_MaxResX / newMultiplier || m_ResY > m_MaxResY / newMultiplier) {
			ShowMessageBox("Requested resolution multiplier will result in game window exceeding display bounds!\nNo change will be made!\n\nNOTE: To toggle fullscreen, use the button in the Options & Controls Menu!");
			return;
		}

		m_MultiWindows.clear();
		m_WindowView.resize(1);
		m_WindowTransforms.resize(1);
		m_WindowViewport.resize(1);
		m_Fullscreen = (m_ResX * newMultiplier == m_MaxResX && m_ResY * newMultiplier == m_MaxResY);

		if (m_Fullscreen) {
			if (m_NumScreens > 1) {
				if (!CreateFullscreenMultiWindows(m_ResX, m_ResY, newMultiplier)) {
					m_Fullscreen = (m_ResX * m_ResMultiplier == m_MaxResX && m_ResY * m_ResMultiplier == m_MaxResY);
					SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
					if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
						RTEAbort("Unable to set back to previous windowed mode multiplier because: " + std::string(SDL_GetError()) + "!");
					}
					g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
					set_palette(m_Palette);
					return;
				}
			} else if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
				m_Fullscreen = (m_ResX * m_ResMultiplier == m_MaxResX && m_ResY * m_ResMultiplier == m_MaxResY);
				SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
				if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
					RTEAbort("Unable to set back to previous windowed mode multiplier because: " + std::string(SDL_GetError()) + "!");
				}
				g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
				set_palette(m_Palette);
				return;
			}
		} else {
			SDL_SetWindowFullscreen(m_Window.get(), 0);
			SDL_RestoreWindow(m_Window.get());
			SDL_SetWindowSize(m_Window.get(), m_ResX * newMultiplier, m_ResY * newMultiplier);
			int displayIndex = SDL_GetWindowDisplayIndex(m_Window.get());
			SDL_SetWindowPosition(m_Window.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
		}
		m_ResMultiplier = newMultiplier;

		int windowW;
		int windowH;
		SDL_GL_GetDrawableSize(m_Window.get(), &windowW, &windowH);
		GL_CHECK(glViewport(0, 0, windowW, windowH););
		if (!m_Fullscreen || m_MultiWindows.empty()) {
			m_WindowViewport[0] = GetViewportLetterbox(m_ResX, m_ResY, windowW, windowH);
			m_WindowView[0] = glm::ortho<float>(0.0f, m_WindowViewport[0].z, 0.0f, m_WindowViewport[0].w, -1.0f, 1.0f);
			m_WindowTransforms[0] = glm::mat4(1.0f);
		}
		set_palette(m_Palette);
		// RecreateBackBuffers();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier.");
		g_SettingsMan.UpdateSettingsFile();

		ClearFrame();
		SwapWindow();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ChangeResolution(int newResX, int newResY, bool upscaled, int newFullscreen) {
		int newResMultiplier = upscaled ? 2 : 1;

		if (m_ResX == newResX && m_ResY == newResY && m_ResMultiplier == newResMultiplier && m_Fullscreen == newFullscreen) {
			return;
		}
		m_MultiWindows.clear();
		m_WindowView.resize(1);
		m_WindowTransforms.resize(1);
		m_WindowViewport.resize(1);

		if (!newFullscreen) {
			SDL_RestoreWindow(m_Window.get());
		}

		ValidateResolution(newResX, newResY, newResMultiplier);

		if (newFullscreen &&
		    ((m_NumScreens > 1 && !CreateFullscreenMultiWindows(newResX, newResY, newResMultiplier)) ||
		        SDL_SetWindowFullscreen(m_Window.get(), SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)) {

			SDL_SetWindowSize(m_Window.get(), m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier);
			if (SDL_SetWindowFullscreen(m_Window.get(), m_Fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0) {
				RTEAbort("Unable to set back to previous resolution because: " + std::string(SDL_GetError()) + "!");
			}
			g_ConsoleMan.PrintString("ERROR: Failed to switch to new resolution, reverted back to previous setting!");
			set_palette(m_Palette);
			return;
		} else if (!newFullscreen) {
			SDL_SetWindowFullscreen(m_Window.get(), 0);
			SDL_RestoreWindow(m_Window.get());
			SDL_SetWindowSize(m_Window.get(), newResX * newResMultiplier, newResY * newResMultiplier);
			int displayIndex = SDL_GetWindowDisplayIndex(m_Window.get());
			SDL_SetWindowPosition(m_Window.get(), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex));
		}
		m_Fullscreen = newFullscreen;
		m_ResX = newResX;
		m_ResY = newResY;
		m_ResMultiplier = newResMultiplier;

		int windowW = m_ResX * m_ResMultiplier;
		int windowH = m_ResY * m_ResMultiplier;
		SDL_GL_GetDrawableSize(m_Window.get(), &windowW, &windowH);
		if (!m_Fullscreen || m_MultiWindows.empty()) {
			m_WindowViewport[0] = GetViewportLetterbox(m_ResX, m_ResY, windowW, windowH);
			m_WindowView[0] = glm::ortho<float>(0.0f, m_WindowViewport[0].z, 0.0f, m_WindowViewport[0].w, -1.0f, 1.0f);
			m_WindowTransforms[0] = glm::mat4(1.0f);
		}
		GL_CHECK(glViewport(0, 0, windowW, windowH););

		set_palette(m_Palette);
		RecreateBackBuffers();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different resolution.");
		g_SettingsMan.UpdateSettingsFile();

		m_ResChanged = true;
		ClearFrame();
		SwapWindow();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::WindowResizedCallback(int newResX, int newResY) {
		RTEAssert(!m_Fullscreen, "ERROR: Somehow the fullscreen window was resized!");
		m_Fullscreen = false;
		m_ResX = newResX / m_ResMultiplier;
		m_ResY = newResY / m_ResMultiplier;

		if ( newResX < 640 || newResY < 480) {
			SDL_SetWindowSize(m_Window.get(), 640, 480);
			m_ResX = 640 / m_ResMultiplier;
			m_ResY = 480 / m_ResMultiplier;
		}

		int windowW;
		int windowH;
		SDL_GL_GetDrawableSize(m_Window.get(), &windowW, &windowH);

		m_WindowView[0] = glm::ortho<float>(0.0f, windowW, 0.0f, windowH, -1.0f, 1.0f);
		m_WindowTransforms[0] = glm::mat4(1.0f);
		m_WindowViewport[0] = glm::vec4(0, 0, windowW, windowH);
		GL_CHECK(glViewport(0, 0, windowW, windowH););

		set_palette(m_Palette);
		RecreateBackBuffers();

		m_ResChanged = true;

		ClearFrame();
		SwapWindow();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit) {
		if (m_PlayerScreen) { release_bitmap(m_PlayerScreen); }

		// Override screen splitting according to settings if needed
		if ((hSplit || vSplit) && !(hSplit && vSplit) && m_TwoPlayerVSplit) {
			hSplit = false;
			vSplit = m_TwoPlayerVSplit;
		}
		m_HSplit = hSplit;
		m_VSplit = vSplit;

		// Create the splitscreen buffer
		if (m_HSplit || m_VSplit) {
			m_PlayerScreen = create_bitmap_ex(8, g_FrameMan.GetResX() / (m_VSplit ? 2 : 1), g_FrameMan.GetResY() / (m_HSplit ? 2 : 1));
			clear_to_color(m_PlayerScreen, m_BlackColor);
			set_clip_state(m_PlayerScreen, 1);

			m_PlayerScreenWidth = m_PlayerScreen->w;
			m_PlayerScreenHeight = m_PlayerScreen->h;
		} else {
			// No splits, so set the screen dimensions equal to the back buffer
			m_PlayerScreenWidth = m_BackBuffer8->w;
			m_PlayerScreenHeight = m_BackBuffer8->h;
		}
		for (int i = 0; i < c_MaxScreenCount; ++i) {
			m_FlashScreenColor[i] = -1;
			m_FlashedLastFrame[i] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector FrameMan::GetMiddleOfPlayerScreen(int whichPlayer) {
		Vector middleOfPlayerScreen;

		if (whichPlayer == -1 || IsInMultiplayerMode()) {
			middleOfPlayerScreen.SetXY(static_cast<float>(m_ResX / 2), static_cast<float>(m_ResY / 2));
		} else {
			int playerScreen = g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer);

			middleOfPlayerScreen.SetXY(static_cast<float>(m_PlayerScreenWidth / 2), static_cast<float>(m_PlayerScreenHeight / 2));
			if ((playerScreen == 1 && g_FrameMan.GetVSplit()) || playerScreen == 3) {
				middleOfPlayerScreen.SetX(middleOfPlayerScreen.GetX() + static_cast<float>(m_PlayerScreenWidth));
			}
			if ((playerScreen == 1 && g_FrameMan.GetHSplit()) || playerScreen >= 2) {
				middleOfPlayerScreen.SetY(middleOfPlayerScreen.GetY() + static_cast<float>(m_PlayerScreenHeight));
			}
		}
		return middleOfPlayerScreen;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const {
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount) {
				int width = GetResX();
				for (int i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w < width)) {
						width = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w;
					}
				}
				return width;
			} else {
				if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]) {
					return m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->w;
				}
			}
		}
		return m_PlayerScreenWidth;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::GetPlayerFrameBufferHeight(int whichPlayer) const {
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount) {
				int height = GetResY();
				for (int i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h < height)) {
						height = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h;
					}
				}
				return height;
			} else {
				if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]) {
					return m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->h;
				}
			}
		}
		return m_PlayerScreenHeight;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextHeight(const std::string &text, int maxWidth, bool isSmall) {
		return isSmall ? GetSmallFont()->CalculateHeight(text, maxWidth) : GetLargeFont()->CalculateHeight(text, maxWidth);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextWidth(const std::string &text, bool isSmall) {
		return isSmall ? GetSmallFont()->CalculateWidth(text) : GetLargeFont()->CalculateWidth(text);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetScreenText(const std::string &message, int whichScreen, int blinkInterval, int displayDuration, bool centered) {
		// See if we can overwrite the previous message
		if (whichScreen >= 0 && whichScreen < c_MaxScreenCount && m_TextDurationTimer[whichScreen].IsPastRealMS(m_TextDuration[whichScreen])) {
			m_ScreenText[whichScreen] = message;
			m_TextDuration[whichScreen] = displayDuration;
			m_TextDurationTimer[whichScreen].Reset();
			m_TextBlinking[whichScreen] = blinkInterval;
			m_TextCentered[whichScreen] = centered;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ClearScreenText(int whichScreen) {
		if (whichScreen >= 0 && whichScreen < c_MaxScreenCount) {
			m_ScreenText[whichScreen].clear();
			m_TextDuration[whichScreen] = -1;
			m_TextDurationTimer[whichScreen].Reset();
			m_TextBlinking[whichScreen] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void FrameMan::ClearFrame() {
		GL_CHECK(glClearColor(0.0, 0.0, 0.0, 1.0););
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT););
		for (auto &window: m_MultiWindows) {
			SDL_GL_MakeCurrent(window.get(), m_GLContext.get());
			GL_CHECK(glClearColor(0.0, 0.0, 0.0, 1.0););
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT););
		}
		SDL_GL_MakeCurrent(m_Window.get(), m_GLContext.get());
	}

	void FrameMan::SwapWindow(){
		SDL_GL_SwapWindow(m_Window.get());
		for (const std::unique_ptr<SDL_Window, SdlWindowDeleter> &window: m_MultiWindows) {
			SDL_GL_SwapWindow(window.get());
		}
	}

	void FrameMan::FlipFrameBuffers() {
		if (s_DisableFrameBufferFlip) {
			return;
		}

		if(m_WantScreenDump) {
			SaveBitmap(ScreenDump, m_ScreenDumpName);
			m_WantScreenDump = false;
		}
		GL_CHECK(glActiveTexture(GL_TEXTURE0););
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_ScreenTexture););
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_BackBuffer32->w, m_BackBuffer32->h, GL_RGBA, GL_UNSIGNED_BYTE, m_BackBuffer32->line[0]););

		glm::mat4 preScaleProjection(1.0f);
		if (m_MultiWindows.size() > 0) {
			for (int i = 0; i < m_MultiWindows.size(); ++i) {
				SDL_GL_MakeCurrent(m_MultiWindows[i].get(), m_GLContext.get());
				GL_CHECK(glViewport(m_WindowViewport[i+1].x, m_WindowViewport[i+1].y, m_WindowViewport[i+1].z, m_WindowViewport[i+1].w););

				preScaleProjection = glm::translate(m_WindowView[i + 1], {m_WindowViewport[i+1].z / 2, m_WindowViewport[i+1].w / 2, 0.0f});
				preScaleProjection = glm::scale<float>(preScaleProjection, {m_WindowViewport[i+1].z / 2,m_WindowViewport[i+1].w / 2, 1.0f});

				GL_CHECK(glActiveTexture(GL_TEXTURE0););
				GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_ScreenTexture););

				m_ScreenShader->Use();
				m_ScreenShader->SetInt("rteTex0", 0);
				m_ScreenShader->SetMatrix("rteUVTransform", m_WindowTransforms[i + 1]);
				m_ScreenShader->SetMatrix("rteProjMatrix", preScaleProjection);

				GL_CHECK(glBindVertexArray(m_ScreenVAO););
				GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4););
				SDL_GL_SwapWindow(m_MultiWindows[i].get());
			}
			SDL_GL_MakeCurrent(m_Window.get(), m_GLContext.get());
		}

		GL_CHECK(glViewport(m_WindowViewport[0].x, m_WindowViewport[0].y, m_WindowViewport[0].z, m_WindowViewport[0].w););
		preScaleProjection = m_WindowView[0];
		preScaleProjection = glm::translate(m_WindowView[0], {m_WindowViewport[0].z / 2, m_WindowViewport[0].w / 2, 0.0f});
		preScaleProjection = glm::scale<float>(preScaleProjection, {m_WindowViewport[0].z / 2, m_WindowViewport[0].w / 2, 1.0f});

		GL_CHECK(glActiveTexture(GL_TEXTURE0););
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_ScreenTexture););

		m_ScreenShader->Use();
		m_ScreenShader->SetInt("rteTex0", 0);
		m_ScreenShader->SetMatrix("rteUVTransform", m_WindowTransforms[0]);
		m_ScreenShader->SetMatrix("rteProjMatrix", preScaleProjection);

		GL_CHECK(glBindVertexArray(m_ScreenVAO););
		GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4););
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetColorTable(DrawBlendMode blendMode, std::array<int, 4> colorChannelBlendAmounts) {
		RTEAssert(blendMode > DrawBlendMode::NoBlend && blendMode < DrawBlendMode::BlendModeCount, "Invalid DrawBlendMode or DrawBlendMode::NoBlend passed into FrameMan::SetColorTable. See DrawBlendMode enumeration for defined values.");

		for (int &colorChannelBlendAmount : colorChannelBlendAmounts) {
			colorChannelBlendAmount = RoundToNearestMultiple(std::clamp(colorChannelBlendAmount, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend)), c_BlendAmountStep);
		}

		bool usedPresetTransparencyTable = false;

		switch (blendMode) {
			case DrawBlendMode::NoBlend:
				RTEAbort("Somehow ended up attempting to set a color table for DrawBlendMode::NoBlend in FrameMan::SetColorTable! This should never happen!");
				return;
			case DrawBlendMode::BlendInvert:
			case DrawBlendMode::BlendDissolve:
				// Invert and Dissolve do nothing with the RGB channels values, so set all channels to Alpha channel value to avoid creating pointless variants.
				colorChannelBlendAmounts.fill(colorChannelBlendAmounts[3]);
				break;
			case DrawBlendMode::BlendTransparency:
				// Indexed transparency has dedicated maps that don't use alpha, so min it to attempt to load one of the presets, and in case there isn't one avoid creating a variant for each alpha value.
				colorChannelBlendAmounts[3] = BlendAmountLimits::MinBlend;
				usedPresetTransparencyTable = (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[1]) && (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[2]);
				break;
			default:
				break;
		}

		// New color tables will be created using the default palette loaded at FrameMan initialization because handling per-palette per-mode color tables is too much headache, even if it may possibly produce better blending results.
		if (m_ColorTables[blendMode].find(colorChannelBlendAmounts) == m_ColorTables[blendMode].end()) {
			m_ColorTables[blendMode].try_emplace(colorChannelBlendAmounts);

			std::array<int, 4> adjustedColorChannelBlendAmounts = { BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend };
			for (int index = 0; index < adjustedColorChannelBlendAmounts.size(); ++index) {
				adjustedColorChannelBlendAmounts[index] = 255 - (static_cast<int>(255.0F * 0.01F * static_cast<float>(colorChannelBlendAmounts[index])));
			}

			if (blendMode == DrawBlendMode::BlendTransparency) {
				// Paletted transparency has dedicated tables so better create one instead of generic for best result. Alpha is ignored here.
				create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], nullptr);
			} else {
				c_BlenderSetterFunctions[blendMode](adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], adjustedColorChannelBlendAmounts[3]);
				create_blender_table(&m_ColorTables[blendMode].at(colorChannelBlendAmounts).first, m_DefaultPalette, nullptr);
				// Reset the blender to avoid potentially screwing some true-color draw operation. Hopefully.
				c_BlenderSetterFunctions[blendMode](255, 255, 255, 255);
			}
		}
		color_map = &m_ColorTables[blendMode].at(colorChannelBlendAmounts).first;
		m_ColorTables[blendMode].at(colorChannelBlendAmounts).second = usedPresetTransparencyTable ? -1 : (g_TimerMan.GetAbsoluteTime() / 10000);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetTransTableFromPreset(TransparencyPreset transPreset) {
		RTEAssert(transPreset == TransparencyPreset::LessTrans || transPreset == TransparencyPreset::HalfTrans || transPreset == TransparencyPreset::MoreTrans, "Undefined transparency preset value passed in. See TransparencyPreset enumeration for defined values.");
		std::array<int, 4> colorChannelBlendAmounts = { transPreset, transPreset, transPreset, BlendAmountLimits::MinBlend };
		if (m_ColorTables[DrawBlendMode::BlendTransparency].find(colorChannelBlendAmounts) != m_ColorTables[DrawBlendMode::BlendTransparency].end()) {
			color_map = &m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first;
			m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::CreateNewNetworkPlayerBackBuffer(int player, int width, int height) {
		for (int f = 0; f < 2; f++) {
			destroy_bitmap(m_NetworkBackBufferIntermediate8[f][player]);
			m_NetworkBackBufferIntermediate8[f][player] = create_bitmap_ex(8, width, height);

			destroy_bitmap(m_NetworkBackBufferIntermediateGUI8[f][player]);
			m_NetworkBackBufferIntermediateGUI8[f][player] = create_bitmap_ex(8, width, height);

			destroy_bitmap(m_NetworkBackBufferFinal8[f][player]);
			m_NetworkBackBufferFinal8[f][player] = create_bitmap_ex(8, width, height);

			destroy_bitmap(m_NetworkBackBufferFinalGUI8[f][player]);
			m_NetworkBackBufferFinalGUI8[f][player] = create_bitmap_ex(8, width, height);
		}
		m_PlayerScreenWidth = width;
		m_PlayerScreenHeight = height;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::LoadPalette(const std::string &palettePath) {
		BITMAP *tempBitmap = load_bitmap(palettePath.c_str(), m_Palette);
		RTEAssert(tempBitmap, ("Failed to load palette from bitmap with following path:\n\n" + palettePath).c_str());

		set_palette(m_Palette);

		// Update what black is now with the loaded palette
		m_BlackColor = bestfit_color(m_Palette, 0, 0, 0);
		m_AlmostBlackColor = bestfit_color(m_Palette, 5, 5, 5);

		destroy_bitmap(tempBitmap);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveBitmap(SaveBitmapMode modeToSave, const std::string& nameBase, BITMAP *bitmapToSave) {
		if ((modeToSave == WorldDump || modeToSave == ScenePreviewDump) && !g_ActivityMan.ActivityRunning()) {
			return 0;
		}
		if (nameBase.empty() || nameBase.size() <= 0) {
			return -1;
		}

		int fileNumber = 0;
		int maxFileTrys = 1000;
		char fullFileName[256];

		while (fileNumber < maxFileTrys) {
			// Check for the file namebase001.png; if it exists, try 002, etc.
			std::snprintf(fullFileName, sizeof(fullFileName), "%s/%s%03i%s", System::GetScreenshotDirectory().c_str(), nameBase.c_str(), fileNumber++, ".png");
			if (!std::filesystem::exists(fullFileName)) {
				break;
			}
		}

		bool saveSuccess = false;

		switch (modeToSave) {
			case SingleBitmap:
				if (bitmapToSave && save_png(nameBase.c_str(), bitmapToSave, m_Palette) == 0) {
					g_ConsoleMan.PrintString("SYSTEM: Bitmap was dumped to: " + std::string(nameBase));
					saveSuccess = true;
				}
				break;
			case ScreenDump:
				if (m_BackBuffer32) {
					if (!m_ScreenDumpBuffer) {
						destroy_bitmap(m_ScreenDumpBuffer);
						m_ScreenDumpBuffer = create_bitmap_ex(24, m_BackBuffer32->w, m_BackBuffer32->h);
					}
					blit(m_BackBuffer32, m_ScreenDumpBuffer, 0, 0, 0, 0, m_BackBuffer32->w, m_BackBuffer32->h);
					// nullptr for the PALETTE parameter here because we're saving a 24bpp file and it's irrelevant.
					if (save_png(fullFileName, m_ScreenDumpBuffer, nullptr) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: Screen was dumped to: " + std::string(fullFileName));
						saveSuccess = true;
					}
				}
				break;
			case ScenePreviewDump:
			case WorldDump:
				if (!m_WorldDumpBuffer || (m_WorldDumpBuffer->w != g_SceneMan.GetSceneWidth() || m_WorldDumpBuffer->h != g_SceneMan.GetSceneHeight())) {
					if (m_WorldDumpBuffer) { destroy_bitmap(m_WorldDumpBuffer); }
					m_WorldDumpBuffer = create_bitmap_ex(32, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight());
				}
				if (modeToSave == ScenePreviewDump) {
					DrawWorldDump(true);

					BITMAP *scenePreviewDumpBuffer = create_bitmap_ex(32, c_ScenePreviewWidth, c_ScenePreviewHeight);
					blit(m_ScenePreviewDumpGradient, scenePreviewDumpBuffer, 0, 0, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);
					masked_stretch_blit(m_WorldDumpBuffer, scenePreviewDumpBuffer, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);

					if (SaveIndexedPNG(fullFileName, scenePreviewDumpBuffer) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: Scene Preview was dumped to: " + std::string(fullFileName));
						saveSuccess = true;
					}
					destroy_bitmap(scenePreviewDumpBuffer);
				} else {
					DrawWorldDump();

					BITMAP *depthConvertBitmap = create_bitmap_ex(24, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);
					blit(m_WorldDumpBuffer, depthConvertBitmap, 0, 0, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);

					if (save_png(fullFileName, depthConvertBitmap, nullptr) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: World was dumped to: " + std::string(fullFileName));
						saveSuccess = true;
					}
					destroy_bitmap(depthConvertBitmap);
				}
				break;
			default:
				g_ConsoleMan.PrintString("ERROR: Wrong bitmap save mode passed in, no bitmap was saved!");
				return -1;
		}
		if (!saveSuccess) {
			g_ConsoleMan.PrintString("ERROR: Unable to save bitmap to: " + std::string(fullFileName));
			return -1;
		} else {
			return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveIndexedPNG(const char *fileName, BITMAP *bitmapToSave) const {
		// nullptr for the PALETTE parameter here because the bitmap is 32bpp and whatever we index it with will end up wrong anyway.
		save_png(fileName, bitmapToSave, nullptr);

		int lastColorConversionMode = get_color_conversion();
		set_color_conversion(COLORCONV_REDUCE_TO_256);
		// nullptr for the PALETTE parameter here because we don't need the bad palette from it and don't want it to overwrite anything.
		BITMAP *tempLoadBitmap = load_bitmap(fileName, nullptr);
		std::remove(fileName);

		BITMAP *tempConvertingBitmap = create_bitmap_ex(8, bitmapToSave->w, bitmapToSave->h);
		blit(tempLoadBitmap, tempConvertingBitmap, 0, 0, 0, 0, tempConvertingBitmap->w, tempConvertingBitmap->h);

		int saveResult = save_png(fileName, tempConvertingBitmap, m_Palette);

		set_color_conversion(lastColorConversionMode);
		destroy_bitmap(tempLoadBitmap);
		destroy_bitmap(tempConvertingBitmap);

		return saveResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SharedDrawLine(BITMAP *bitmap, const Vector &start, const Vector &end, int color, int altColor, int skip, int skipStart, bool shortestWrap, bool drawDot, BITMAP *dot) const {
		RTEAssert(bitmap, "Trying to draw line to null Bitmap");
		if (drawDot) { RTEAssert(dot, "Trying to draw line of dots without specifying a dot Bitmap"); }

		int error = 0;
		int dom = 0;
		int sub = 0;
		int domSteps = 0;
		int skipped = skip + (skipStart - skip);
		int intPos[2];
		int delta[2];
		int delta2[2];
		int increment[2];
		bool drawAlt = false;

		int dotHeight = drawDot ? dot->h : 0;
		int dotWidth = drawDot ? dot->w : 0;

		//acquire_bitmap(bitmap);

		// Just make the alt the same color as the main one if no one was specified
		if (altColor == 0) { altColor = color; }

		intPos[X] = start.GetFloorIntX();
		intPos[Y] = start.GetFloorIntY();

		// Wrap line around the scene if it makes it shorter
		if (shortestWrap) {
			Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
			delta[X] = deltaVec.GetFloorIntX();
			delta[Y] = deltaVec.GetFloorIntY();
		} else {
			delta[X] = end.GetFloorIntX() - intPos[X];
			delta[Y] = end.GetFloorIntY() - intPos[Y];
		}
		if (delta[X] == 0 && delta[Y] == 0) {
			return 0;
		}

		// Bresenham's line drawing algorithm preparation
		if (delta[X] < 0) {
			increment[X] = -1;
			delta[X] = -delta[X];
		} else {
			increment[X] = 1;
		}
		if (delta[Y] < 0) {
			increment[Y] = -1;
			delta[Y] = -delta[Y];
		} else {
			increment[Y] = 1;
		}

		// Scale by 2, for better accuracy of the error at the first pixel
		delta2[X] = delta[X] << 1;
		delta2[Y] = delta[Y] << 1;

		// If X is dominant, Y is submissive, and vice versa.
		if (delta[X] > delta[Y]) {
			dom = X;
			sub = Y;
		} else {
			dom = Y;
			sub = X;
		}
		error = delta2[sub] - delta[dom];

		// Bresenham's line drawing algorithm execution
		for (domSteps = 0; domSteps < delta[dom]; ++domSteps) {
			intPos[dom] += increment[dom];
			if (error >= 0) {
				intPos[sub] += increment[sub];
				error -= delta2[dom];
			}
			error += delta2[sub];

			// Only draw pixel if we're not due to skip any
			if (++skipped > skip) {
				// Scene wrapping, if necessary
				g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

				if (drawDot) {
					masked_blit(dot, bitmap, 0, 0, intPos[X] - (dotWidth / 2), intPos[Y] - (dotHeight / 2), dot->w, dot->h);
				} else {
					putpixel(bitmap, intPos[X], intPos[Y], drawAlt ? color : altColor);
				}
				drawAlt = !drawAlt;
				skipped = 0;
			}
		}

		//release_bitmap(bitmap);

		// Return the end phase state of the skipping
		return skipped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIFont * FrameMan::GetFont(bool isSmall) {
		if (!m_GUIScreen) { m_GUIScreen = new AllegroScreen(m_BackBuffer8); }

		if (isSmall) {
			if (!m_SmallFont) {
				m_SmallFont = new GUIFont("SmallFont");
				m_SmallFont->Load(m_GUIScreen, "Base.rte/GUIs/Skins/FontSmall.png");
			}
			return m_SmallFont;
		}
		if (!m_LargeFont) {
			m_LargeFont = new GUIFont("FatFont");
			m_LargeFont->Load(m_GUIScreen, "Base.rte/GUIs/Skins/FontLarge.png");
		}
		return m_LargeFont;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::UpdateScreenOffsetForSplitScreen(int playerScreen, Vector &screenOffset) const {
		switch (playerScreen) {
			case Players::PlayerTwo:
				// If both splits, or just VSplit, then in upper right quadrant
				if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit)) {
					screenOffset.SetXY(GetResX() / 2, 0);
				} else {
					// If only HSplit, then lower left quadrant
					screenOffset.SetXY(0, GetResY() / 2);
				}
				break;
			case Players::PlayerThree:
				// Always lower left quadrant
				screenOffset.SetXY(0, GetResY() / 2);
				break;
			case Players::PlayerFour:
				// Always lower right quadrant
				screenOffset.SetXY(GetResX() / 2, GetResY() / 2);
				break;
			default:
				// Always upper left corner
				screenOffset.SetXY(0, 0);
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Draw() {
		// Count how many split screens we'll need
		int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
		RTEAssert(screenCount <= 1 || m_PlayerScreen, "Splitscreen surface not ready when needed!");

		g_PostProcessMan.ClearScreenPostEffects();

		// These accumulate the effects for each player's screen area, and are then transferred to the post-processing lists with the player screen offset applied
		std::list<PostEffect> screenRelativeEffects;
		std::list<Box> screenRelativeGlowBoxes;

		const Activity *pActivity = g_ActivityMan.GetActivity();

		for (int playerScreen = 0; playerScreen < screenCount; ++playerScreen) {
			screenRelativeEffects.clear();
			screenRelativeGlowBoxes.clear();

			BITMAP *drawScreen = (screenCount == 1) ? m_BackBuffer8 : m_PlayerScreen;
			BITMAP *drawScreenGUI = drawScreen;
			if (IsInMultiplayerMode()) {
				drawScreen = m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][playerScreen];
				drawScreenGUI = m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][playerScreen];
			}
			// Need to clear the backbuffers because Scene background layers can be too small to fill the whole backbuffer or drawn masked resulting in artifacts from the previous frame.
			clear_to_color(drawScreenGUI, ColorKeys::g_MaskColor);
			// If in online multiplayer mode clear to mask color otherwise the scene background layers will get drawn over.
			clear_to_color(drawScreen, IsInMultiplayerMode() ? ColorKeys::g_MaskColor : m_BlackColor);

			AllegroBitmap playerGUIBitmap(drawScreenGUI);

			// Update the scene view to line up with a specific screen and then draw it onto the intermediate screen
			g_SceneMan.Update(playerScreen);

			// Save scene layer's offsets for each screen, server will pick them to build the frame state and send to client
			if (IsInMultiplayerMode()) {
				int layerCount = 0;

				for (const SceneLayer *sceneLayer : g_SceneMan.GetScene()->GetBackLayers()) {
					SLOffset[playerScreen][layerCount] = sceneLayer->GetOffset();
					layerCount++;

					if (layerCount >= c_MaxLayersStoredForNetwork) {
						break;
					}
				}
			}

			Vector targetPos = g_CameraMan.GetOffset(playerScreen);

			// Adjust the drawing position on the target screen for if the target screen is larger than the scene in non-wrapping dimension.
			// Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
			if (!g_SceneMan.SceneWrapsX() && drawScreen->w > g_SceneMan.GetSceneWidth()) { targetPos.m_X += (drawScreen->w - g_SceneMan.GetSceneWidth()) / 2; }
			if (!g_SceneMan.SceneWrapsY() && drawScreen->h > g_SceneMan.GetSceneHeight()) { targetPos.m_Y += (drawScreen->h - g_SceneMan.GetSceneHeight()) / 2; }

			// Try to move at the frame buffer copy time to maybe prevent wonkyness
			m_TargetPos[m_NetworkFrameCurrent][playerScreen] = targetPos;

			// Draw the scene
			if (!IsInMultiplayerMode()) {
				g_SceneMan.Draw(drawScreen, drawScreenGUI, targetPos);
			} else {
				g_SceneMan.Draw(drawScreen, drawScreenGUI, targetPos, true, true);
			}

			// Get only the scene-relative post effects that affect this player's screen
			if (pActivity) {
				g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(playerScreen)));
				g_PostProcessMan.GetGlowAreasWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeGlowBoxes);

				if (IsInMultiplayerMode()) { g_PostProcessMan.SetNetworkPostEffectsList(playerScreen, screenRelativeEffects); }
			}

			// TODO: Find out what keeps disabling the clipping on the draw bitmap
			// Enable clipping on the draw bitmap
			set_clip_state(drawScreen, 1);

			DrawScreenText(playerScreen, playerGUIBitmap);

			// The position of the current draw screen on the backbuffer
			Vector screenOffset;

			// If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer
			if (screenCount > 1) { UpdateScreenOffsetForSplitScreen(playerScreen, screenOffset); }

			DrawScreenFlash(playerScreen, drawScreenGUI);

			if (!IsInMultiplayerMode()) {
				// Draw the intermediate draw splitscreen to the appropriate spot on the back buffer
				blit(drawScreen, m_BackBuffer8, 0, 0, screenOffset.GetFloorIntX(), screenOffset.GetFloorIntY(), drawScreen->w, drawScreen->h);

				g_PostProcessMan.AdjustEffectsPosToPlayerScreen(playerScreen, drawScreen, screenOffset, screenRelativeEffects, screenRelativeGlowBoxes);
			}
		}

		// Clears the pixels that have been revealed from the unseen layers
		g_SceneMan.ClearSeenPixels();

		if (!IsInMultiplayerMode()) {
			// Draw separating lines for split-screens
			if (m_HSplit) {
				hline(m_BackBuffer8, 0, (m_BackBuffer8->h / 2) - 1, m_BackBuffer8->w - 1, m_AlmostBlackColor);
				hline(m_BackBuffer8, 0, (m_BackBuffer8->h / 2), m_BackBuffer8->w - 1, m_AlmostBlackColor);
			}
			if (m_VSplit) {
				vline(m_BackBuffer8, (m_BackBuffer8->w / 2) - 1, 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
				vline(m_BackBuffer8, (m_BackBuffer8->w / 2), 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
			}

			// Replace 8 bit backbuffer contents with network received image before post-processing as it is where this buffer is copied to 32 bit buffer
			if (GetDrawNetworkBackBuffer()) {
				m_NetworkBitmapLock[0].lock();

				blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][0], m_BackBuffer8, 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
				masked_blit(m_NetworkBackBufferFinalGUI8[m_NetworkFrameReady][0], m_BackBuffer8, 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);

				if (g_UInputMan.FlagAltState() || g_UInputMan.FlagCtrlState() || g_UInputMan.FlagShiftState()) { g_PerformanceMan.DrawCurrentPing(); }

				m_NetworkBitmapLock[0].unlock();
			}
		}

		if (IsInMultiplayerMode()) { PrepareFrameForNetwork(); }

		if (g_ActivityMan.IsInActivity()) { g_PostProcessMan.PostProcess(); }

		// Draw the performance stats and console on top of everything.
		g_PerformanceMan.Draw(m_BackBuffer32);
		g_ConsoleMan.Draw(m_BackBuffer32);

#ifdef DEBUG_BUILD
		// Draw scene seam
		vline(m_BackBuffer8, 0, 0, g_SceneMan.GetSceneHeight(), 5);
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawScreenText(int playerScreen, AllegroBitmap playerGUIBitmap) {
		int textPosY = 0;
		// Only draw screen text to actual human players
		if (playerScreen < g_ActivityMan.GetActivity()->GetHumanCount()) {
			textPosY += 12;

			if (!m_ScreenText[playerScreen].empty()) {
				int bufferOrScreenWidth = IsInMultiplayerMode() ? GetPlayerFrameBufferWidth(playerScreen) : GetPlayerScreenWidth();
				int bufferOrScreenHeight = IsInMultiplayerMode() ? GetPlayerFrameBufferHeight(playerScreen) : GetPlayerScreenHeight();

				if (m_TextCentered[playerScreen]) { textPosY = (bufferOrScreenHeight / 2) - 52; }

				int screenOcclusionOffsetX = g_CameraMan.GetScreenOcclusion(playerScreen).GetRoundIntX();
				// If there's really no room to offset the text into, then don't
				if (GetPlayerScreenWidth() <= GetResX() / 2) { screenOcclusionOffsetX = 0; }

				// Draw text and handle blinking by turning on and off extra surrounding characters. Text is always drawn to keep it readable.
				if (m_TextBlinking[playerScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[playerScreen])) {
					GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, ">>> " + m_ScreenText[playerScreen] + " <<<", GUIFont::Centre);
				} else {
					GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, m_ScreenText[playerScreen], GUIFont::Centre);
				}
				textPosY += 12;
			}

			// Draw info text when in MOID or material layer draw mode
			switch (g_SceneMan.GetLayerDrawMode()) {
				case g_LayerTerrainMatter:
					GetSmallFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, GetPlayerScreenHeight() - 12, "Viewing terrain material layer\nHit Ctrl+M to cycle modes", GUIFont::Centre, GUIFont::Bottom);
					break;
#ifdef DRAW_MOID_LAYER
				case g_LayerMOID:
					GetSmallFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, GetPlayerScreenHeight() - 12, "Viewing MovableObject ID layer\nHit Ctrl+M to cycle modes", GUIFont::Centre, GUIFont::Bottom);
					break;
#endif
				default:
					break;
			}
		} else {
			// If superfluous screen (as in a three-player match), make the fourth the Observer one
			GetLargeFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, textPosY, "- Observer View -", GUIFont::Centre);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawScreenFlash(int playerScreen, BITMAP *playerGUIBitmap) {
		if (m_FlashScreenColor[playerScreen] != -1) {
			// If set to flash for a period of time, first be solid and then start flashing slower
			double timeTillLimit = m_FlashTimer[playerScreen].LeftTillRealTimeLimitMS();

			if (timeTillLimit < 10 || m_FlashTimer[playerScreen].AlternateReal(50)) {
				if (m_FlashedLastFrame[playerScreen]) {
					m_FlashedLastFrame[playerScreen] = false;
				} else {
					rectfill(playerGUIBitmap, 0, 0, playerGUIBitmap->w, playerGUIBitmap->h, m_FlashScreenColor[playerScreen]);
					m_FlashedLastFrame[playerScreen] = true;
				}
			}
			if (m_FlashTimer[playerScreen].IsPastRealTimeLimit()) { m_FlashScreenColor[playerScreen] = -1; }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawWorldDump(bool drawForScenePreview) const {
		float worldBitmapWidth = static_cast<float>(m_WorldDumpBuffer->w);
		float worldBitmapHeight = static_cast<float>(m_WorldDumpBuffer->h);

		// Draw sky gradient if we're not dumping a scene preview
		if (!drawForScenePreview) {
			clear_to_color(m_WorldDumpBuffer, makecol32(132, 192, 252)); // Light blue color
			for (int i = 0; i < m_WorldDumpBuffer->h; i++) {
				int lineColor = makecol32(64 + ((static_cast<float>(i) / worldBitmapHeight) * (128 - 64)), 64 + ((static_cast<float>(i) / worldBitmapHeight) * (192 - 64)), 96 + ((static_cast<float>(i) / worldBitmapHeight) * (255 - 96)));
				hline(m_WorldDumpBuffer, 0, i, worldBitmapWidth - 1, lineColor);
			}
		} else {
			clear_to_color(m_WorldDumpBuffer, makecol32(255, 0, 255)); // Magenta
		}

		// Draw scene
		draw_sprite(m_WorldDumpBuffer, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0);
		draw_sprite(m_WorldDumpBuffer, g_SceneMan.GetTerrain()->GetFGColorBitmap(), 0, 0);

		// If we're not dumping a scene preview, draw objects and post-effects.
		if (!drawForScenePreview) {
			std::list<PostEffect> postEffectsList;
			BITMAP *effectBitmap = nullptr;
			int effectPosX = 0;
			int effectPosY = 0;
			int effectStrength = 0;
			Vector targetPos(0, 0);

			// Draw objects
			draw_sprite(m_WorldDumpBuffer, g_SceneMan.GetMOColorBitmap(), 0, 0);

			// Draw post-effects
			g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, worldBitmapWidth, worldBitmapHeight, postEffectsList, -1);

			for (const PostEffect &postEffect : postEffectsList) {
				effectBitmap = postEffect.m_Bitmap;
				effectStrength = postEffect.m_Strength;
				set_screen_blender(effectStrength, effectStrength, effectStrength, effectStrength);
				effectPosX = postEffect.m_Pos.GetFloorIntX() - (effectBitmap->w / 2);
				effectPosY = postEffect.m_Pos.GetFloorIntY() - (effectBitmap->h / 2);

				if (postEffect.m_Angle == 0.0F) {
					draw_trans_sprite(m_WorldDumpBuffer, effectBitmap, effectPosX, effectPosY);
				} else {
					BITMAP *targetBitmap = g_PostProcessMan.GetTempEffectBitmap(effectBitmap);
					clear_to_color(targetBitmap, 0);

					Matrix newAngle(postEffect.m_Angle);
					rotate_sprite(targetBitmap, effectBitmap, 0, 0, ftofix(newAngle.GetAllegroAngle()));
					draw_trans_sprite(m_WorldDumpBuffer, targetBitmap, effectPosX, effectPosY);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::PrepareFrameForNetwork() {
		int dx = 0;
		int dy = 0;
		int dw = m_BackBuffer8->w / 2;
		int dh = m_BackBuffer8->h / 2;

		// Blit all four internal player screens onto the backbuffer
		for (int i = 0; i < c_MaxScreenCount; i++) {
			dx = (i == 1 || i == 3) ? dw : dx;
			dy = (i == 2 || i == 3) ? dh : dy;

			m_NetworkBitmapLock[i].lock();
			blit(m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][i], m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->h);
			blit(m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][i], m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->h);
			m_NetworkBitmapLock[i].unlock();

#ifndef RELEASE_BUILD
			// Draw all player's screen into one
			if (g_UInputMan.KeyHeld(KEY_5)) {
				stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h, dx, dy, dw, dh);
			}
#endif
		}

#ifndef RELEASE_BUILD
		if (g_UInputMan.KeyHeld(KEY_1)) {
			stretch_blit(m_NetworkBackBufferFinal8[0][0], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][0]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][0]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_2)) {
			stretch_blit(m_NetworkBackBufferFinal8[1][0], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][1]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][1]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_3)) {
			stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][2], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][2]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][2]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_4)) {
			stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][3], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][3]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][3]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
#endif
		// Rendering complete, we can finally mark current frame as ready. This is needed to make rendering look totally atomic for the server pulling data in separate threads.
		m_NetworkFrameReady = m_NetworkFrameCurrent;
		m_NetworkFrameCurrent = (m_NetworkFrameCurrent == 0) ? 1 : 0;
	}
}
