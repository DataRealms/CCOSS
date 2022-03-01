#include "FrameMan.h"
#include "PostProcessMan.h"
#include "PrimitiveMan.h"
#include "PerformanceMan.h"
#include "ActivityMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "UInputMan.h"

#include "SLTerrain.h"
#include "Scene.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"

#ifdef _WIN32
#include "winalleg.h"
#elif __unix__
#include <xalleg.h>
#endif

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DisplaySwitchOut() {
		g_UInputMan.DisableMouseMoving(true);

#ifdef __unix__
		// In fullscreen regrab focus because the window is lost otherwise. Only applies to X11 since XWayland handles this differently.
		if (_xwin.fs_window) { XSetInputFocus(_xwin.display, _xwin.window, RevertToPointerRoot, CurrentTime); }
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DisplaySwitchIn() {
		g_UInputMan.DisableMouseMoving(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Clear() {
		m_GfxDriverMessage.clear();
		m_GfxDriver = GFX_AUTODETECT_WINDOWED;
		m_ForceVirtualFullScreenGfxDriver = false;
		m_ForceDedicatedFullScreenGfxDriver = false;
		m_DisableMultiScreenResolutionValidation = false;
#ifdef _WIN32
		m_NumScreens = GetSystemMetrics(SM_CMONITORS);
		m_MaxResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		m_MaxResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		m_PrimaryScreenResX = GetSystemMetrics(SM_CXSCREEN);
		m_PrimaryScreenResY = GetSystemMetrics(SM_CYSCREEN);
#elif __unix__
		m_NumScreens = 1;
		m_MaxResX = m_PrimaryScreenResX = DisplayWidth(_xwin.display, _xwin.screen);
		m_MaxResY = m_PrimaryScreenResY = DisplayHeight(_xwin.display, _xwin.screen);
#endif
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

	void FrameMan::SetInitialGraphicsDriver() {
#ifdef _WIN32
		if (m_ForceVirtualFullScreenGfxDriver) {
			m_GfxDriver = GFX_DIRECTX_WIN_BORDERLESS;
			m_GfxDriverMessage = "SYSTEM: Using DirectX borderless window driver!";
		} else if (m_ForceDedicatedFullScreenGfxDriver) {
			m_GfxDriver = GFX_DIRECTX_ACCEL;
			m_GfxDriverMessage = "SYSTEM: Using DirectX dedicated fullscreen driver!";
		} else {
			m_GfxDriver = GFX_AUTODETECT_WINDOWED;
		}
#else
		m_GfxDriver = (m_ResX * m_ResMultiplier == m_MaxResX && m_ResY * m_ResMultiplier == m_MaxResY) ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::PrintForcedGfxDriverMessage() const {
		if (!m_GfxDriverMessage.empty()) { g_ConsoleMan.PrintString(m_GfxDriverMessage); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetDisplaySwitchMode() const {
		set_display_switch_mode(SWITCH_BACKGROUND);
		set_display_switch_callback(SWITCH_OUT, DisplaySwitchOut);
		set_display_switch_callback(SWITCH_IN, DisplaySwitchIn);

#ifdef __unix__
		// Release the mouse and keyboard to keep passing certain keys to the system and avoid effective system hard-locks. This effectively makes a borderless fullscreen window.
		if (_xwin.keyboard_grabbed) { XUngrabKeyboard(_xwin.display, CurrentTime); }
		if (_xwin.mouse_grabbed) { XUngrabPointer(_xwin.display, CurrentTime); }
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ValidateResolution(int &resX, int &resY, int &resMultiplier) const {
		bool settingsNeedOverwrite = false;

		if (resX * resMultiplier > m_MaxResX || resY * resMultiplier > m_MaxResY) {
			settingsNeedOverwrite = true;
			resX = m_MaxResX / resMultiplier;
			resY = m_MaxResY / resMultiplier;
			ShowMessageBox("Resolution too high to fit display, overriding to fit!");

#ifndef __unix__
		} else if (!m_ForceDedicatedFullScreenGfxDriver && resX * resMultiplier == 1366 && resY * resMultiplier == 768) {
			settingsNeedOverwrite = true;
			resX = 1360 / resMultiplier;
			resY = 768 / resMultiplier;
			const char *invalidResolutionMessage = {
				"Unfortunately, 1366x768 resolution is not supported in windowed or borderless mode. 1360x768 will be used instead!\n"
				"To enable the use of this resolution, please force the dedicated fullscreen driver through \"Settings.ini\" or through the in-game custom resolution settings."
			};
			ShowMessageBox(invalidResolutionMessage);
		} else if (!m_ForceDedicatedFullScreenGfxDriver && (resX * resMultiplier) % 4 > 0) {
			settingsNeedOverwrite = true;
			resX = static_cast<int>(std::floor(static_cast<float>(resX) / 4.0F) * 4.0F);
			std::string invalidResolutionMessage = {
				"Resolution width that is not divisible by 4 is not supported in windowed or borderless mode!\nOverriding to closest valid width!\n"
				"To enable the use of this resolution, please force the dedicated fullscreen driver through \"Settings.ini\" or through the in-game custom resolution settings."
			};
			ShowMessageBox(invalidResolutionMessage);
#endif

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
			if (resX * resMultiplier > m_PrimaryScreenResX || resY * resMultiplier > m_PrimaryScreenResY) { settingsNeedOverwrite = ValidateMultiScreenResolution(resX, resY, resMultiplier); }
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
#ifdef _WIN32
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
		ValidateResolution(m_ResX, m_ResY, m_ResMultiplier);
		SetInitialGraphicsDriver();
		set_color_depth(m_BPP);

		if (set_gfx_mode(m_GfxDriver, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, 0, 0) != 0) {
			// If a bad resolution somehow slipped past the validation, revert to defaults.
			ShowMessageBox("Unable to set specified graphics mode because: " + std::string(allegro_error) + "!\n\nTrying to revert to defaults...");
			if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, c_DefaultResX, c_DefaultResY, 0, 0) != 0) {
				RTEAbort("Unable to set any graphics mode because " + std::string(allegro_error) + "!");
				return 1;
			}
			m_ResX = c_DefaultResX;
			m_ResY = c_DefaultResY;
			m_ResMultiplier = 1;
		}

		// Clear the screen buffer so it doesn't flash pink
		clear_to_color(screen, 0);

		SetDisplaySwitchMode();

		// Sets the allowed color conversions when loading bitmaps from files
		set_color_conversion(COLORCONV_MOST);

		LoadPalette(m_PaletteFile.GetDataPath());

		// Create transparency color table
		PALETTE ccPalette;
		get_palette(ccPalette);
		create_trans_table(&m_LessTransTable, ccPalette, 192, 192, 192, nullptr);
		create_trans_table(&m_HalfTransTable, ccPalette, 128, 128, 128, nullptr);
		create_trans_table(&m_MoreTransTable, ccPalette, 64, 64, 64, nullptr);
		// Set the one Allegro currently uses
		color_map = &m_HalfTransTable;

		CreateBackBuffers();

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

		m_ScreenDumpBuffer = create_bitmap_ex(24, screen->w, screen->h);

		return 0;
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
		g_PerformanceMan.Update();

		// Remove all scheduled primitives, those will be re-added by updates from other entities.
		// This needs to happen here, otherwise if there are multiple sim updates during a single frame duplicates will be added to the primitive queue.
		g_PrimitiveMan.ClearPrimitivesQueue();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ChangeResolutionMultiplier(int newMultiplier) {
		if (newMultiplier <= 0 || newMultiplier > 4 || newMultiplier == m_ResMultiplier) {
			return;
		}

#ifdef _WIN32
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

#ifdef __unix__
		m_GfxDriver = (m_ResX * newMultiplier == m_MaxResX && m_ResY * newMultiplier == m_MaxResY) ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;
#elif _WIN32
		m_GfxDriver = (m_ResX * newMultiplier == m_MaxResX && m_ResY * newMultiplier == m_MaxResY) ? GFX_DIRECTX_WIN_BORDERLESS : GFX_AUTODETECT_WINDOWED;
#endif

		// Set the GFX_TEXT driver to hack around Allegro's window resizing limitations (specifically reducing window size) when switching from 2X mode to 1X mode.
		// This will force a state where there is no actual game window between multiplier switches and the next set_gfx_mode call will recreate it correctly.
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

		if (set_gfx_mode(m_GfxDriver, m_ResX * newMultiplier, m_ResY * newMultiplier, 0, 0) != 0) {
			if (set_gfx_mode(m_GfxDriver, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, 0, 0) != 0) {
				RTEAbort("Unable to set back to previous windowed mode multiplier because: " + std::string(allegro_error) + "!");
			}
			g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
			set_palette(m_Palette);
			SetDisplaySwitchMode();
			return;
		}
		m_ResMultiplier = newMultiplier;

		set_palette(m_Palette);
		SetDisplaySwitchMode();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier.");
		g_SettingsMan.UpdateSettingsFile();

		FlipFrameBuffers();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ChangeResolution(int newResX, int newResY, bool upscaled, int newGfxDriver) {
		int newResMultiplier = upscaled ? 2 : 1;

		if (m_ResX == newResX && m_ResY == newResY && m_ResMultiplier == newResMultiplier && m_GfxDriver == newGfxDriver) {
			return;
		}
		bool prevForceDedicatedDriver = m_ForceDedicatedFullScreenGfxDriver;
		m_ForceDedicatedFullScreenGfxDriver = newGfxDriver == GFX_AUTODETECT_FULLSCREEN || newGfxDriver == GFX_DIRECTX_ACCEL;

		ValidateResolution(newResX, newResY, newResMultiplier);

		// Set the GFX_TEXT driver to hack around Allegro's window resizing limitations (specifically reducing window size) when switching from 2X mode to 1X mode.
		// This will force a state where there is no actual game window between multiplier switches and the next set_gfx_mode call will recreate it correctly.
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

		if (set_gfx_mode(newGfxDriver, newResX * newResMultiplier, newResY * newResMultiplier, 0, 0) != 0) {
			if (set_gfx_mode(m_GfxDriver, m_ResX * m_ResMultiplier, m_ResY * m_ResMultiplier, 0, 0) != 0) {
				RTEAbort("Unable to set back to previous resolution because: " + std::string(allegro_error) + "!");
			}
			g_ConsoleMan.PrintString("ERROR: Failed to switch to new resolution, reverted back to previous setting!");
			m_ForceDedicatedFullScreenGfxDriver = prevForceDedicatedDriver;
			set_palette(m_Palette);
			SetDisplaySwitchMode();
			return;
		}
		m_GfxDriver = newGfxDriver;
		m_ResX = newResX;
		m_ResY = newResY;
		m_ResMultiplier = newResMultiplier;

		set_palette(m_Palette);
		RecreateBackBuffers();
		SetDisplaySwitchMode();

		g_ConsoleMan.PrintString("SYSTEM: Switched to different resolution.");
		g_SettingsMan.UpdateSettingsFile();

		m_ResChanged = true;
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

	void FrameMan::FlipFrameBuffers() const {
		if (m_ResMultiplier > 1) {
			stretch_blit(m_BackBuffer32, screen, 0, 0, m_BackBuffer32->w, m_BackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
		} else {
			blit(m_BackBuffer32, screen, 0, 0, 0, 0, m_BackBuffer32->w, m_BackBuffer32->h);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetTransTable(TransparencyPreset transSetting) {
		switch (transSetting) {
			case LessTrans:
				color_map = &m_LessTransTable;
				break;
			case MoreTrans:
				color_map = &m_MoreTransTable;
				break;
			case HalfTrans:
				color_map = &m_HalfTransTable;
				break;
			default:
				RTEAbort("Undefined transparency preset value passed in. See TransparencyPreset enumeration for defined values.");
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

	int FrameMan::SaveBitmap(SaveBitmapMode modeToSave, const char *nameBase, BITMAP *bitmapToSave) {
		if ((modeToSave == WorldDump || modeToSave == ScenePreviewDump) && !g_ActivityMan.ActivityRunning()) {
			return 0;
		}
		if (nameBase == nullptr || strlen(nameBase) <= 0) {
			return -1;
		}

		int fileNumber = 0;
		int maxFileTrys = 1000;
		char fullFileName[256];

		while (fileNumber < maxFileTrys) {
			// Check for the file namebase001.png; if it exists, try 002, etc.
			std::snprintf(fullFileName, sizeof(fullFileName), "%s/%s%03i%s", System::GetScreenshotDirectory().c_str(), nameBase, fileNumber++, ".png");
			if (!std::filesystem::exists(fullFileName)) {
				break;
			}
		}

		bool saveSuccess = false;

		switch (modeToSave) {
			case SingleBitmap:
				if (bitmapToSave && save_png(nameBase, bitmapToSave, m_Palette) == 0) {
					g_ConsoleMan.PrintString("SYSTEM: Bitmap was dumped to: " + std::string(nameBase));
					saveSuccess = true;
				}
				break;
			case ScreenDump:
				if (screen) {
					if (m_ScreenDumpBuffer->w != screen->w || m_ScreenDumpBuffer->h != screen->h) {
						destroy_bitmap(m_ScreenDumpBuffer);
						m_ScreenDumpBuffer = create_bitmap_ex(24, screen->w, screen->h);
					}
					blit(screen, m_ScreenDumpBuffer, 0, 0, 0, 0, screen->w, screen->h);
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
		list<PostEffect> screenRelativeEffects;
		list<Box> screenRelativeGlowBoxes;

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
			Vector targetPos = g_SceneMan.GetOffset(playerScreen);

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
				clear_to_color(drawScreen, g_MaskColor);
				clear_to_color(drawScreenGUI, g_MaskColor);
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

		// Draw the console on top of everything
		g_ConsoleMan.Draw(m_BackBuffer32);

#ifdef DEBUG_BUILD
		// Draw scene seam
		vline(m_BackBuffer8, 0, 0, g_SceneMan.GetSceneHeight(), 5);
#endif

		// Reset the frame timer so we can measure how much it takes until next frame being drawn
		g_PerformanceMan.ResetFrameTimer();
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

				int screenOcclusionOffsetX = g_SceneMan.GetScreenOcclusion(playerScreen).GetRoundIntX();
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
				case g_LayerMOID:
					GetSmallFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, GetPlayerScreenHeight() - 12, "Viewing MovableObject ID layer\nHit Ctrl+M to cycle modes", GUIFont::Centre, GUIFont::Bottom);
					break;
				default:
					break;
			}
			g_PerformanceMan.Draw(playerGUIBitmap);

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
