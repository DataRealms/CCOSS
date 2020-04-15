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

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"

extern bool g_ResetActivity;
extern bool g_InActivity;

namespace RTE {

	const std::string FrameMan::c_ClassName = "FrameMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Callback function for the allegro set_display_switch_callback. It will be called when focus is switched away from the game window.
	/// </summary>
	void DisplaySwitchOut(void) { g_UInputMan.DisableMouseMoving(true); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Callback function for the allegro set_display_switch_callback. It will be called when focus is switched back to the game window. It will temporarily disable positioning of the mouse.
	/// This is so that when focus is switched back to the game window, it avoids having the window fly away because the user clicked the title bar of the window.
	/// </summary>
	void DisplaySwitchIn(void) { g_UInputMan.DisableMouseMoving(false); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Clear() {
		m_ResX = 960;
		m_ResY = 540;
		m_NewResX = m_ResX;
		m_NewResY = m_ResY;
		m_Fullscreen = false;
		m_NxWindowed = 1;
		m_NxFullscreen = 1;
		m_NewNxFullscreen = 1;
		m_HSplit = false;
		m_VSplit = false;
		m_HSplitOverride = false;
		m_VSplitOverride = false;
		m_PlayerScreen = 0;
		m_PlayerScreenWidth = 0;
		m_PlayerScreenHeight = 0;
		m_ScreendumpBuffer = 0;
		m_BackBuffer8 = 0;
		m_BackBuffer32 = 0;
		m_DrawNetworkBackBuffer = false;
		m_StoreNetworkBackBuffer = false;
		m_NetworkFrameCurrent = 0;
		m_NetworkFrameReady = 1;
		m_PaletteFile.Reset();
		m_BlackColor = 245;
		m_AlmostBlackColor = 245;
		m_PPM = 0;
		m_GUIScreen = 0;
		m_LargeFont = 0;
		m_SmallFont = 0;
		m_TextBlinkTimer.Reset();

		for (short i = 0; i < c_MaxScreenCount; ++i) {
			m_ScreenText[i].clear();
			m_TextDuration[i] = -1;
			m_TextDurationTimer[i].Reset();
			m_TextBlinking[i] = 0;
			m_TextCentered[i] = false;
			m_FlashScreenColor[i] = -1;
			m_FlashedLastFrame[i] = false;
			m_FlashTimer[i].Reset();
			m_NetworkBitmapIsLocked[i] = false;

			for (short f = 0; f < 2; f++) {
				m_NetworkBackBufferIntermediate8[f][i] = 0;
				m_NetworkBackBufferFinal8[f][i] = 0;
				m_NetworkBackBufferIntermediateGUI8[f][i] = 0;
				m_NetworkBackBufferFinalGUI8[f][i] = 0;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::Create() {
		// Init allegro's graphics
		set_color_depth(m_BPP);

		// Refuse windowed multiplier if the resolution is too high
		if (m_ResX > 1024) { m_NxWindowed = 1; }

		int fullscreenGfxDriver = GFX_AUTODETECT_FULLSCREEN;
		int windowedGfxDriver = GFX_AUTODETECT_WINDOWED;

		fullscreenGfxDriver = GFX_DIRECTX_ACCEL;

		if (g_SettingsMan.ForceSoftwareGfxDriver()) { fullscreenGfxDriver = GFX_DIRECTX_SOFT; }
		if (g_SettingsMan.ForceSafeGfxDriver()) { fullscreenGfxDriver = GFX_DIRECTX_SAFE; }
		if (g_SettingsMan.ForceOverlayedWindowGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_OVL; }
		if (g_SettingsMan.ForceNonOverlayedWindowGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_WIN; }
		if (g_SettingsMan.ForceVirtualFullScreenGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_WIN_BORDERLESS; }
		
		if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0) {
			g_ConsoleMan.PrintString("Failed to set gfx mode, trying different windowed scaling.");
			// If player somehow managed to set up a windowed 2X mode and then set a resolution higher than physical, then disable 2X resolution
			m_NxWindowed = (m_NxWindowed == 2) ? 1 : m_NxWindowed;
			if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0) {
				g_ConsoleMan.PrintString("Failed to set gfx mode, trying different fullscreen scaling.");

				// TODO: this is whack if we're attempting windowed
				// Oops, failed to set fullscreen mode, try a different fullscreen scaling
				m_NewNxFullscreen = m_NxFullscreen = (m_NxFullscreen == 1) ? 2 : 1;
				if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0) {
					// Oops, failed to set the resolution specified in the setting file, so default to a safe one instead
					allegro_message("Unable to set specified graphics mode because: %s!\n\nNow trying to default back to VGA...", allegro_error);
					if (set_gfx_mode(m_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, 960, 540, 0, 0) != 0) {
						set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
						allegro_message("Unable to set any graphics mode because %s!", allegro_error);
						return 1;
					}
					// Successfully reverted to VGA, so set that as the current resolution
					m_ResX = m_NewResX = 960;
					m_ResY = m_NewResY = 540;
					m_NxWindowed = 1;
					m_NewNxFullscreen = m_NxFullscreen = 1;
				}
			}
		}

		// Clear the screen buffer so it doesn't flash pink
		clear_to_color(screen, 0);

		// Sets the allowed color conversions when loading bitmaps from files
		set_color_conversion(COLORCONV_MOST);

		// Load and set the palette
		if (!LoadPalette(m_PaletteFile.GetDataPath())) {
			return -1;
		}

		// Set the switching mode; what happens when the app window is switched to and from
		set_display_switch_mode(SWITCH_BACKGROUND);
		//set_display_switch_mode(SWITCH_PAUSE);
		set_display_switch_callback(SWITCH_OUT, DisplaySwitchOut);
		set_display_switch_callback(SWITCH_IN, DisplaySwitchIn);

		// Create transparency color table
		PALETTE ccPalette;
		get_palette(ccPalette);
		create_trans_table(&m_LessTransTable, ccPalette, 192, 192, 192, 0);
		create_trans_table(&m_HalfTransTable, ccPalette, 128, 128, 128, 0);
		create_trans_table(&m_MoreTransTable, ccPalette, 64, 64, 64, 0);
		// Set the one Allegro currently uses
		color_map = &m_HalfTransTable;

		// Create the back buffer, this is still in 8bpp, we will do any post-processing on the PostProcessing bitmap
		m_BackBuffer8 = create_bitmap_ex(8, m_ResX, m_ResY);
		ClearBackBuffer8();

		// Create the post-processing buffer, it'll be used for glow effects etc
		m_BackBuffer32 = create_bitmap_ex(32, m_ResX, m_ResY);
		ClearBackBuffer32();

		// Create all the network 8bpp back buffers
		for (short i = 0; i < c_MaxScreenCount; i++) {
			for (short f = 0; f < 2; f++) {
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
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "ResolutionX") {
			reader >> m_ResX;
			m_NewResX = m_ResX;
		} else if (propName == "ResolutionY") {
			reader >> m_ResY;
			m_NewResY = m_ResY;
		} else if (propName == "Fullscreen") {
			reader >> m_Fullscreen;
		} else if (propName == "NxWindowed") {
			reader >> m_NxWindowed;
		} else if (propName == "NxFullscreen") {
			reader >> m_NxFullscreen;
			m_NewNxFullscreen = m_NxFullscreen;
		} else if (propName == "HSplitScreen") {
			reader >> m_HSplitOverride;
		} else if (propName == "VSplitScreen") {
			reader >> m_VSplitOverride;
		} else if (propName == "PaletteFile") {
			reader >> m_PaletteFile;
		} else if (propName == "PixelsPerMeter") {
			reader >> m_PPM;
			m_MPP = 1 / m_PPM;
			// Calculate the Pixel per Liter and Liter per Pixel
			float cmPP = 100 / m_PPM;
			float LVolume = 10 * 10 * 10;
			m_PPL = LVolume / (cmPP * cmPP * cmPP);
			m_LPP = 1 / m_PPL;
		} else {
			// See if the base class(es) can find a match instead
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("ResolutionX");
		writer << m_ResX;
		writer.NewProperty("ResolutionY");
		writer << m_ResY;
		writer.NewProperty("Fullscreen");
		writer << m_Fullscreen;
		writer.NewProperty("HSplitScreen");
		writer << m_HSplitOverride;
		writer.NewProperty("VSplitScreen");
		writer << m_VSplitOverride;
		writer.NewProperty("PaletteFile");
		writer << m_PaletteFile;
		writer.NewProperty("PixelsPerMeter");
		writer << m_PPM;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Destroy() {
		destroy_bitmap(m_BackBuffer8);
		for (short i = 0; i < c_MaxScreenCount; i++) {
			for (short f = 0; f < 2; f++) {
				destroy_bitmap(m_NetworkBackBufferIntermediate8[f][i]);
				destroy_bitmap(m_NetworkBackBufferIntermediateGUI8[f][i]);
				destroy_bitmap(m_NetworkBackBufferFinal8[f][i]);
				destroy_bitmap(m_NetworkBackBufferFinalGUI8[f][i]);
			}
		}
		destroy_bitmap(m_BackBuffer32);
		destroy_bitmap(m_PlayerScreen);
		delete m_GUIScreen;
		delete m_LargeFont;
		delete m_SmallFont;

		g_TimerMan.Destroy();
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Update() {
		g_PerformanceMan.Update();

		// Remove all scheduled primitives, those will be re-added by updates from other entities.
		g_PrimitiveMan.ClearPrimitivesList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::ToggleFullscreen() {
		// Save the palette so we can re-set it after the change
		PALETTE pal;
		get_palette(pal);

		// Refuse windowed multiplier if the resolution is too high
		if (m_ResX > 1024) { m_NxWindowed = 1; }

		// Need to save these first for recovery attempts to work (screen might be 0)
		int resX = m_ResX;
		int resY = m_ResY;

		int fullscreenGfxDriver = GFX_AUTODETECT_FULLSCREEN;
		int windowedGfxDriver = GFX_AUTODETECT_WINDOWED;

		fullscreenGfxDriver = GFX_DIRECTX_ACCEL;

		if (g_SettingsMan.ForceSoftwareGfxDriver()) { fullscreenGfxDriver = GFX_DIRECTX_SOFT; }
		if (g_SettingsMan.ForceSafeGfxDriver()) { fullscreenGfxDriver = GFX_DIRECTX_SAFE; }
		if (g_SettingsMan.ForceOverlayedWindowGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_OVL; }
		if (g_SettingsMan.ForceNonOverlayedWindowGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_WIN; }
		if (g_SettingsMan.ForceVirtualFullScreenGfxDriver()) { windowedGfxDriver = GFX_DIRECTX_WIN_BORDERLESS; }

		// Switch to windowed mode
		if (m_Fullscreen) {
			if (set_gfx_mode(windowedGfxDriver, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0) {
				// Oops, failed to set windowed mode, so go back to fullscreen
				if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0) {
					// Can't go back to fullscreen either! total fail
					set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
					allegro_message("Unable to set back to fullscreen mode because: %s!", allegro_error);
					return 1;
				}
				// Successfully reverted back to fullscreen
				g_ConsoleMan.PrintString("ERROR: Failed to switch to windowed mode, reverted back to fullscreen!");
				m_Fullscreen = true;
				// Reset the palette.
				set_palette(pal);
				// Make sure everything gets caught up after the switch
				rest(2000);
				return 1;
			}
			// Make sure everything gets caught up after the switch
			rest(2000);
			g_ConsoleMan.PrintString("SYSTEM: Switched to windowed mode");
			m_Fullscreen = false;
			// Switch to fullscreen
		} else {
			if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0) {
				// Oops, failed to set fullscreen mode, try a different fullscreen scaling
				m_NewNxFullscreen = m_NxFullscreen = m_NxFullscreen == 1 ? 2 : 1;
				if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0) {
					// That didn't work either, so go back to windowed
					allegro_message("Unable enter fullscreen mode because: %s!\n\nWill try to revert to windowed mode now...", allegro_error);
					if (set_gfx_mode(windowedGfxDriver, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0) {
						// Can't go back to windowed either! total fail
						set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
						allegro_message("Unable to set back to windowed mode because: %s!", allegro_error);
						return 1;
					}
					// Successfully reverted back to windowed
					g_ConsoleMan.PrintString("ERROR: Failed to switch to fullscreen mode, reverted back to windowed!");
					m_Fullscreen = false;
					// Make sure everything gets caught up after the switch
					rest(1500);
					// Reset the palette.
					set_palette(pal);
					return 1;
				}
			}
			// Make sure everything gets caught up after the switch
			rest(1500);
			g_ConsoleMan.PrintString("SYSTEM: Switched to fullscreen mode");
			m_Fullscreen = true;
		}
		// Reset the palette.
		set_palette(pal);

		// Adjust the speed of the mouse according to 2x of screen
		float mouseDenominator = IsFullscreen() ? NxFullscreen() : NxWindowed();
		set_mouse_range(0, 0, (GetResX() * mouseDenominator) - 3, (GetResY() * mouseDenominator) - 3);

		// If NxFullscreen, adjust the mouse speed accordingly
		unsigned char mouseSpeedMultiplier = (g_FrameMan.IsFullscreen() && g_FrameMan.NxFullscreen() > 1) ? 1 : 2;
		set_mouse_speed(mouseSpeedMultiplier, mouseSpeedMultiplier);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::IsValidResolution(int width, int height) const {
		int actualWidth = width;
		int actualHeight = height;

		// If width is greater than 1280, the game will switch itself in 2X mode lowering actual resolution twice.
		if (width >= 1280) {
			actualWidth = width / 2;
			actualHeight = height / 2;
		}
		return (actualWidth < 360 || actualHeight < 360) ? false : true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SwitchWindowMultiplier(int multiplier) {
		if (multiplier <= 0 || multiplier > 4 || multiplier == m_NxWindowed) {
			return -1;
		}
		if (m_Fullscreen) {
			m_NxWindowed = multiplier;
			return 0;
		}

		// Refuse windowed multiplier if the resolution is too high
		if (m_ResX > 1024) { m_NxWindowed = 1; }

		// Save the palette so we can re-set it after the switch.
		PALETTE pal;
		get_palette(pal);
		// Need to save these first for recovery attempts to work (screen might be 0)
		int resX = m_ResX;
		int resY = m_ResY;

		// Switch to new windowed mode
		if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, resX * multiplier, resY * multiplier, 0, 0) != 0) {
			// Oops, failed to set windowed mode, so go back to previous multiplier
			if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0) {
				// Can't go back to previous mode either! total fail
				set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
				allegro_message("Unable to set back to previous windowed mode multiplier because: %s!", allegro_error);
				return 1;
			}
			// Successfully reverted back to previous setting
			g_ConsoleMan.PrintString("ERROR: Failed to switch to new windowed mode multiplier, reverted back to previous setting!");
			// Reset the palette.
			set_palette(pal);
			// Make sure everything gets caught up after the switch
			rest(2000);
			return 1;
		}
		// Make sure everything gets caught up after the switch
		rest(2000);
		g_ConsoleMan.PrintString("SYSTEM: Switched to different windowed mode multiplier");

		// Save the new multiplier
		m_NxWindowed = multiplier;
		m_Fullscreen = false;

		// Reset the palette.
		set_palette(pal);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit) {
		// Free the previous splitscreen, if any
		if (m_PlayerScreen) { release_bitmap(m_PlayerScreen); }

		// Override screen splitting according to settings if needed
		if ((hSplit || vSplit) && !(hSplit && vSplit) && (m_HSplitOverride || m_VSplitOverride)) {
			hSplit = m_HSplitOverride;
			vSplit = m_VSplitOverride;
		}
		m_HSplit = hSplit;
		m_VSplit = vSplit;

		// Create the splitscreen buffer
		if (m_HSplit || m_VSplit) {
			m_PlayerScreen = create_bitmap_ex(8, g_FrameMan.GetResX() / (m_VSplit ? 2 : 1), g_FrameMan.GetResY() / (m_HSplit ? 2 : 1));
			clear_to_color(m_PlayerScreen, m_BlackColor);
			set_clip_state(m_PlayerScreen, 1);

			// Update these to represent the split screens
			m_PlayerScreenWidth = m_PlayerScreen->w;
			m_PlayerScreenHeight = m_PlayerScreen->h;

		// No splits, so set the screen dimensions equal to the back buffer
		} else {
			m_PlayerScreenWidth = m_BackBuffer8->w;
			m_PlayerScreenHeight = m_BackBuffer8->h;
		}
		// Reset the flashes
		for (short i = 0; i < c_MaxScreenCount; ++i) {
			m_FlashScreenColor[i] = -1;
			m_FlashedLastFrame[i] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const {
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount) {
				unsigned short w = GetResX();
				for (short i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w < w)) {
						w = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w;
					}
				}
				return w;
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
				unsigned short h = GetResY();
				for (short i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h < h)) { 
						h = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h;
					}
				}
				return h;
			} else {
				if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]) {
					return m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->h;
				}
			}
		}
		return m_PlayerScreenHeight;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextHeight(std::string text, int maxWidth, bool isSmall) {
		return isSmall ? GetSmallFont()->CalculateHeight(text, maxWidth) : GetLargeFont()->CalculateHeight(text, maxWidth);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextWidth(std::string text, bool isSmall) {
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

	void FrameMan::FlipFrameBuffers() {
		if ((!m_Fullscreen && m_NxWindowed != 1) || (m_Fullscreen && m_NxFullscreen != 1)) {
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
				RTEAbort("Undefined transparency preset value passed in. See TransparencyPreset enumeration for defined values.")
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::DrawLine(BITMAP *bitmap, const Vector &start, const Vector &end, int color, int altColor, int skip, int skipStart, bool shortestWrap) {
		RTEAssert(bitmap, "Trying to draw line to null Bitmap");

		//acquire_bitmap(bitmap);

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

		// Just make the alt the same color as the main one if no one was specified
		if (altColor == 0) { altColor = color; }
		
		intPos[X] = floorf(start.m_X);
		intPos[Y] = floorf(start.m_Y);

		// Wrap line around the scene if it makes it shorter
		if (shortestWrap) {
			Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
			delta[X] = floorf(deltaVec.m_X);
			delta[Y] = floorf(deltaVec.m_Y);
		} else {
			delta[X] = floorf(end.m_X) - intPos[X];
			delta[Y] = floorf(end.m_Y) - intPos[Y];
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

				// Slap a regular pixel on there
				putpixel(bitmap, intPos[X], intPos[Y], drawAlt ? color : altColor);
				drawAlt = !drawAlt;
				skipped = 0;
			}
		}
		//release_bitmap(bitmap);

		// Return the end phase state of the skipping
		return skipped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::DrawDotLine(BITMAP *bitmap, const Vector &start, const Vector &end, BITMAP *dot, int skip, int skipStart, bool shortestWrap) {
		RTEAssert(bitmap, "Trying to draw line to null Bitmap");
		RTEAssert(dot, "Trying to draw line of dots without specifying a dot Bitmap");

		//acquire_bitmap(bitmap);

		int	error = 0;
		int	dom = 0;
		int	sub = 0;
		int	domSteps = 0;
		int	skipped = skip + (skipStart - skip);
		int intPos[2];
		int delta[2];
		int delta2[2];
		int increment[2];
		bool drawAlt = false;
		int dotHalfHeight = dot->h / 2;
		int dotHalfWidth = dot->w / 2;

		// Calculate the integer values
		intPos[X] = floorf(start.m_X);
		intPos[Y] = floorf(start.m_Y);

		// Wrap line around the scene if it makes it shorter
		if (shortestWrap) {
			Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
			delta[X] = floorf(deltaVec.m_X);
			delta[Y] = floorf(deltaVec.m_Y);	
		} else {
			// No wrap
			delta[X] = floorf(end.m_X) - intPos[X];
			delta[Y] = floorf(end.m_Y) - intPos[Y];
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

				// Slap the dot on there
				masked_blit(dot, bitmap, 0, 0, intPos[X] - dotHalfWidth, intPos[Y] - dotHalfHeight, dot->w, dot->h);

				drawAlt = !drawAlt;
				skipped = 0;
			}
		}
		//release_bitmap(bitmap);

		// Return the end phase state of the skipping
		return skipped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::CreateNewPlayerBackBuffer(int player, int w, int h) {
		for (int f = 0; f < 2; f++) {
			destroy_bitmap(m_NetworkBackBufferIntermediate8[f][player]);
			m_NetworkBackBufferIntermediate8[f][player] = create_bitmap_ex(8, w, h);

			destroy_bitmap(m_NetworkBackBufferIntermediateGUI8[f][player]);
			m_NetworkBackBufferIntermediateGUI8[f][player] = create_bitmap_ex(8, w, h);

			destroy_bitmap(m_NetworkBackBufferFinal8[f][player]);
			m_NetworkBackBufferFinal8[f][player] = create_bitmap_ex(8, w, h);

			destroy_bitmap(m_NetworkBackBufferFinalGUI8[f][player]);
			m_NetworkBackBufferFinalGUI8[f][player] = create_bitmap_ex(8, w, h);
		}
		m_PlayerScreenWidth = w;
		m_PlayerScreenHeight = h;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::LoadPalette(std::string palettePath) {
		PALETTE newPalette;
		BITMAP *tempBitmap = load_bitmap(palettePath.c_str(), newPalette);
		RTEAssert(tempBitmap, ("Failed to load palette from bitmap with following path:\n\n" + palettePath).c_str());

		set_palette(newPalette);

		// Update what black is now with the loaded palette
		m_BlackColor = bestfit_color(newPalette, 0, 0, 0);
		m_AlmostBlackColor = bestfit_color(newPalette, 5, 5, 5);

		destroy_bitmap(tempBitmap);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveScreenToBMP(const char *nameBase) {
		int filenumber = 0;
		char fullfilename[256];
		int maxFileTrys = 1000;

		if (nameBase == 0 || strlen(nameBase) <= 0) {
			return -1;
		}

		do {
			// Check for the file namebase001.bmp; if it exists, try 002, etc.
			sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", nameBase, filenumber++);
			if (!std::experimental::filesystem::exists(fullfilename)) {
				break;
			}
		} while (filenumber < maxFileTrys);

		// Save out the screen bitmap, after making a copy of it, faster sometimes
		if (screen) {
			if (!m_ScreendumpBuffer) { m_ScreendumpBuffer = create_bitmap(screen->w, screen->h); }

			blit(screen, m_ScreendumpBuffer, 0, 0, 0, 0, screen->w, screen->h);
			PALETTE palette;
			get_palette(palette);
			save_bmp(fullfilename, m_ScreendumpBuffer, palette);

			g_ConsoleMan.PrintString("SYSTEM: Screen was dumped to: " + string(fullfilename));
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveWorldToBMP(const char *nameBase) {
		if (!g_ActivityMan.ActivityRunning()) {
			return 0;
		}

		int filenumber = 0;
		char fullfilename[256];
		int maxFileTrys = 1000;

		// Make sure its not a 0 name base
		if (nameBase == 0 || strlen(nameBase) <= 0) {
			return -1;
		}

		do {
			// Check for the file namebase001.bmp; if it exists, try 002, etc.
			sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", nameBase, filenumber++);
			if (!std::experimental::filesystem::exists(fullfilename)) {
				break;
			}
		} while (filenumber < maxFileTrys);


		BITMAP * pWorldBitmap = create_bitmap_ex(32, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight());
		Vector targetPos(0, 0);
		std::list<PostEffect> postEffects;

		if (pWorldBitmap) {
			clear_to_color(pWorldBitmap, makecol32(132, 192, 252)); // Light blue color

			//Draw sky gradient
			for (int i = 0; i < pWorldBitmap->h; i++) {
				hline(pWorldBitmap, 0, i, pWorldBitmap->w - 1, makecol32(64 + (((float)i / (float)pWorldBitmap->h) * (128 - 64)), 64 + (((float)i / (float)pWorldBitmap->h) * (192 - 64)), 96 + ((float)i / (float)pWorldBitmap->h) * (255 - 96)));
			}

			// Draw scene
			draw_sprite(pWorldBitmap, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0);
			draw_sprite(pWorldBitmap, g_SceneMan.GetTerrain()->GetFGColorBitmap(), 0, 0);

			//Draw objects
			draw_sprite(pWorldBitmap, g_SceneMan.GetMOColorBitmap(), 0, 0);

			g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, pWorldBitmap->w, pWorldBitmap->h, postEffects, -1);

			//Draw post-effects
			BITMAP *pBitmap = 0;
			int effectPosX = 0;
			int effectPosY = 0;
			int strength = 0;
			float angle = 0;

			for (list<PostEffect>::iterator eItr = postEffects.begin(); eItr != postEffects.end(); ++eItr) {
				pBitmap = (*eItr).m_Bitmap;
				strength = (*eItr).m_Strength;
				set_screen_blender(strength, strength, strength, strength);
				effectPosX = (*eItr).m_Pos.GetFloorIntX() - (pBitmap->w / 2);
				effectPosY = (*eItr).m_Pos.GetFloorIntY() - (pBitmap->h / 2);
				angle = (*eItr).m_Angle;

				if (angle == 0) {
					draw_trans_sprite(pWorldBitmap, pBitmap, effectPosX, effectPosY);
				} else {
					BITMAP * pTargetBitmap;

					if (pBitmap->w < 16 && pBitmap->h < 16) {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(16);
					} else if (pBitmap->w < 32 && pBitmap->h < 32) {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(32);
					} else if (pBitmap->w < 64 && pBitmap->h < 64) {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(64);
					} else if (pBitmap->w < 128 && pBitmap->h < 128) {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(128);
					} else if (pBitmap->w < 256 && pBitmap->h < 256) {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(256);
					} else {
						pTargetBitmap = g_PostProcessMan.GetTempEffectBitmap(512);
					}
					clear_to_color(pTargetBitmap, 0);

					fixed fAngle;
					fAngle = fixmul(angle, radtofix_r);

					rotate_sprite(pTargetBitmap, pBitmap, 0, 0, fAngle);
					draw_trans_sprite(pWorldBitmap, pTargetBitmap, effectPosX, effectPosY);
				}
			}

			PALETTE palette;
			get_palette(palette);
			save_bmp(fullfilename, pWorldBitmap, palette);

			g_ConsoleMan.PrintString("SYSTEM: World was dumped to: " + string(fullfilename));

			destroy_bitmap(pWorldBitmap);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveBitmapToBMP(BITMAP *bitmap, const char *nameBase) {
		int filenumber = 0;
		char fullfilename[256];
		int maxFileTrys = 1000;

		// Make sure its not a 0 name base
		if (nameBase == 0 || strlen(nameBase) <= 0) {
			return -1;
		}

		do {
			// Check for the file namebase001.bmp; if it exists, try 002, etc.
			sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", nameBase, filenumber++);
			if (!std::experimental::filesystem::exists(fullfilename)) {
				break;
			}
		} while (filenumber < maxFileTrys);

		// Save out the bitmap
		if (bitmap) {
			PALETTE palette;
			get_palette(palette);
			save_bmp(fullfilename, bitmap, palette);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIFont * FrameMan::GetFont(bool isSmall) {
		if (!m_GUIScreen) { m_GUIScreen = new AllegroScreen(m_BackBuffer8); }

		switch (isSmall) {
			case false:
				if (!m_LargeFont) {
					m_LargeFont = new GUIFont("FatFont");
					m_LargeFont->Load(m_GUIScreen, "Base.rte/GUIs/Skins/Base/fatfont.bmp");
				}
				return m_LargeFont;
			case true:
				if (!m_SmallFont) {
					m_SmallFont = new GUIFont("SmallFont");
					m_SmallFont->Load(m_GUIScreen, "Base.rte/GUIs/Skins/Base/smallfont.bmp");
				}
				return m_SmallFont;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Draw() {
		// Count how many split screens we'll need
		int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
		RTEAssert(screenCount <= 1 || m_PlayerScreen, "Splitscreen surface not ready when needed!");

		g_PostProcessMan.Reset();

		// These accumulate the effects for each player's screen area, and are then transferred to the above lists with the player screen offset applied
		list<PostEffect> screenRelativeEffects;
		list<Box> screenRelativeGlowBoxes;

		const Activity *pActivity = g_ActivityMan.GetActivity();

		for (int whichScreen = 0; whichScreen < screenCount; ++whichScreen) {
			screenRelativeEffects.clear();
			screenRelativeGlowBoxes.clear();

			BITMAP *pDrawScreen = (screenCount == 1) ? m_BackBuffer8 : m_PlayerScreen;
			BITMAP *pDrawScreenGUI = pDrawScreen;
			if (IsInMultiplayerMode()) {
				pDrawScreen = m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][whichScreen];
				pDrawScreenGUI = m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][whichScreen];
			}

			AllegroBitmap pPlayerGUIBitmap(pDrawScreenGUI);

			// Update the scene view to line up with a specific screen and then draw it onto the intermediate screen
			g_SceneMan.Update(whichScreen);

			// Save scene layer's offsets for each screen, server will pick them to build the frame state and send to client
			if (IsInMultiplayerMode()) {
				int layerCount = 0;

				for (std::list<SceneLayer *>::reverse_iterator itr = g_SceneMan.GetScene()->GetBackLayers().rbegin(); itr != g_SceneMan.GetScene()->GetBackLayers().rend(); ++itr) {
					SLOffset[whichScreen][layerCount] = (*itr)->GetOffset();
					layerCount++;

					if (layerCount >= c_MaxLayersStoredForNetwork) {
						break;
					}
				}
			}
			Vector targetPos = g_SceneMan.GetOffset(whichScreen);

			// Adjust the drawing position on the target screen for if the target screen is larger than the scene in non-wrapping dimension.
			// Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
			if (!g_SceneMan.SceneWrapsX() && pDrawScreen->w > g_SceneMan.GetSceneWidth()) { targetPos.m_X += (pDrawScreen->w - g_SceneMan.GetSceneWidth()) / 2; }
			if (!g_SceneMan.SceneWrapsY() && pDrawScreen->h > g_SceneMan.GetSceneHeight()) { targetPos.m_Y += (pDrawScreen->h - g_SceneMan.GetSceneHeight()) / 2; }

			// Try to move at the frame buffer copy time to maybe prevent wonkyness
			m_TargetPos[m_NetworkFrameCurrent][whichScreen] = targetPos;

			// Draw the scene
			if (!IsInMultiplayerMode()) {
				g_SceneMan.Draw(pDrawScreen, pDrawScreenGUI, targetPos);
			} else {
				clear_to_color(pDrawScreen, g_MaskColor);
				clear_to_color(pDrawScreenGUI, g_MaskColor);
				g_SceneMan.Draw(pDrawScreen, pDrawScreenGUI, targetPos, true, true);
			}

			// Get only the scene-relative post effects that affect this player's screen
			if (pActivity) {
				g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(whichScreen)));
				g_PostProcessMan.GetGlowAreasWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeGlowBoxes);

				if (IsInMultiplayerMode()) { g_PostProcessMan.SetNetworkPostEffectsList(whichScreen, screenRelativeEffects); }
			}

			// TODO: Find out what keeps disabling the clipping on the draw bitmap
			// Enable clipping on the draw bitmap
			set_clip_state(pDrawScreen, 1);

			DrawScreenText(whichScreen, pPlayerGUIBitmap);

			// If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer
			// The position of the current draw screen on the final screen
			Vector screenOffset;

			if (screenCount > 1) {
				switch (whichScreen) {
					case 1:
						// If both splits, or just VSplit, then in upper right quadrant
						if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit)) {
							screenOffset.SetIntXY(GetResX() / 2, 0);
						} else {
							// If only HSplit, then lower left quadrant
							screenOffset.SetIntXY(0, GetResY() / 2);
						}
						break;
					case 2:
						// Always lower left quadrant
						screenOffset.SetIntXY(0, GetResY() / 2);
						break;
					case 3:
						// Always lower right quadrant
						screenOffset.SetIntXY(GetResX() / 2, GetResY() / 2);
						break;
					default:
						// Always upper left corner
						screenOffset.SetIntXY(0, 0);
						break;
				}
			}

			DrawScreenFlash(whichScreen, pDrawScreenGUI);

			// Draw the intermediate draw splitscreen to the appropriate spot on the back buffer
			if (!IsInMultiplayerMode()) { blit(pDrawScreen, m_BackBuffer8, 0, 0, screenOffset.GetFloorIntX(), screenOffset.GetFloorIntY(), pDrawScreen->w, pDrawScreen->h); }

			// Add the player screen's effects to the total screen effects list so they can be drawn in post processing
			if (!IsInMultiplayerMode()) {
				int occX = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntX();
				int occY = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntY();

				// Copy post effects received by client if in network mode
				if (m_DrawNetworkBackBuffer) { g_PostProcessMan.GetNetworkPostEffectsList(0, screenRelativeEffects); }

				// Adjust for the player screen's position on the final buffer
				for (list<PostEffect>::iterator eItr = screenRelativeEffects.begin(); eItr != screenRelativeEffects.end(); ++eItr) {
					// Make sure we won't be adding any effects to a part of the screen that is occluded by menus and such
					if ((*eItr).m_Pos.m_X > occX && (*eItr).m_Pos.m_Y > occY && (*eItr).m_Pos.m_X < pDrawScreen->w + occX && (*eItr).m_Pos.m_Y < pDrawScreen->h + occY) {
						g_PostProcessMan.GetPostScreenEffectsList()->push_back(PostEffect((*eItr).m_Pos + screenOffset, (*eItr).m_Bitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
					}
				}

				// Adjust glow areas for the player screen's position on the final buffer
				for (list<Box>::iterator bItr = screenRelativeGlowBoxes.begin(); bItr != screenRelativeGlowBoxes.end(); ++bItr) {
					g_PostProcessMan.GetPostScreenGlowBoxesList()->push_back(*bItr);
					// Adjust each added glow area for the player screen's position on the final buffer
					g_PostProcessMan.GetPostScreenGlowBoxesList()->back().m_Corner += screenOffset;
				}
			}
		}

		// Clears the pixels that have been revealed from the unseen layers
		g_SceneMan.ClearSeenPixels();

		if (!IsInMultiplayerMode()) {
			// Draw separating lines for split-screens
			acquire_bitmap(m_BackBuffer8);
			if (m_HSplit) {
				hline(m_BackBuffer8, 0, (m_BackBuffer8->h / 2) - 1, m_BackBuffer8->w - 1, m_AlmostBlackColor);
				hline(m_BackBuffer8, 0, (m_BackBuffer8->h / 2), m_BackBuffer8->w - 1, m_AlmostBlackColor);
			}
			if (m_VSplit) {
				vline(m_BackBuffer8, (m_BackBuffer8->w / 2) - 1, 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
				vline(m_BackBuffer8, (m_BackBuffer8->w / 2), 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
			}

			// Replace 8 bit backbuffer contents with network received image before post-processing as it is where this buffer is copied to 32 bit buffer
			if (m_DrawNetworkBackBuffer) {
				m_NetworkBitmapIsLocked[0] = true;
				blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][0], m_BackBuffer8, 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
				masked_blit(m_NetworkBackBufferFinalGUI8[m_NetworkFrameReady][0], m_BackBuffer8, 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);

				if (g_UInputMan.FlagAltState() || g_UInputMan.FlagCtrlState() || g_UInputMan.FlagShiftState()) { g_PerformanceMan.DrawCurrentPing(); }

				m_NetworkBitmapIsLocked[0] = false;
			}
		}

		if (IsInMultiplayerMode()) {
			// Blit all four internal player screens onto the backbuffer
			for (short i = 0; i < c_MaxScreenCount; i++) {
				int dx = 0;
				int dy = 0;
				int dw = m_BackBuffer8->w / 2;
				int dh = m_BackBuffer8->h / 2;

				switch (i) {
					case 1:
						dx = dw;
						break;
					case 2:
						dy = dh;
						break;
					case 3:
						dx = dw;
						dy = dh;
						break;
					default:
						break;
				}

				m_NetworkBitmapIsLocked[i] = true;
				blit(m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][i], m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->h);
				blit(m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][i], m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->h);
				m_NetworkBitmapIsLocked[i] = false;

#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD
				// Draw all player's screen into one
				if (g_UInputMan.KeyHeld(KEY_5)) {
					stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i], m_BackBuffer8, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h, dx, dy, dw, dh);
				}
#endif
			}

#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD
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

			// Rendering complete, we can finally mark current frame as ready
			// This is needed to make rendering look totally atomic for the server pulling data in separate threads
			m_NetworkFrameReady = m_NetworkFrameCurrent;
			m_NetworkFrameCurrent = (m_NetworkFrameCurrent == 0) ? 1 : 0;
		}

		if (g_InActivity) { g_PostProcessMan.PostProcess(); }

		// Draw the console on top of everything
		g_ConsoleMan.Draw(m_BackBuffer32);

#ifdef DEBUG_BUILD
		// Draw scene seam
		vline(m_BackBuffer8, 0, 0, g_SceneMan.GetSceneHeight(), 5);
#endif

		release_bitmap(m_BackBuffer8);

		// Reset the frame timer so we can measure how much it takes until next frame being drawn
		g_PerformanceMan.ResetFrameTimer();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawScreenText(short playerScreen, AllegroBitmap playerGUIBitmap) {
		int textPosY = 0;
		// Only draw screen text to actual human players
		if (playerScreen < g_ActivityMan.GetActivity()->GetHumanCount()) {
			textPosY += 12;

			if (!m_ScreenText[playerScreen].empty()) {
				unsigned short bufferOrScreenWidth = IsInMultiplayerMode() ? GetPlayerFrameBufferWidth(playerScreen) : GetPlayerScreenWidth();
				unsigned short bufferOrScreenHeight = IsInMultiplayerMode() ? GetPlayerFrameBufferHeight(playerScreen) : GetPlayerScreenHeight();

				if (m_TextCentered[playerScreen]) { textPosY = (bufferOrScreenHeight / 2) - 52; }

				int screenOcclusionOffsetX = g_SceneMan.GetScreenOcclusion(playerScreen).m_X;
				// If there's really no room to offset the text into, then don't
				if (GetPlayerScreenWidth() <= GetResX() / 2) { screenOcclusionOffsetX = 0; }

				// Draw text and handle blinking by turning on and off extra surrounding characters. Text is always drawn to keep it readable.
				if (m_TextBlinking[playerScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[playerScreen])) {
					GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, (">>> " + m_ScreenText[playerScreen] + " <<<").c_str(), GUIFont::Centre);
				} else {
					GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, m_ScreenText[playerScreen].c_str(), GUIFont::Centre);
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

	void FrameMan::DrawScreenFlash(short playerScreen, BITMAP *playerGUIBitmap) {
		if (m_FlashScreenColor[playerScreen] != -1) {
			// If set to flash for a period of time, first be solid and then start flashing slower
			float timeTillLimit = m_FlashTimer[playerScreen].LeftTillRealTimeLimitMS();

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
}