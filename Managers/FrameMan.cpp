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

void FrameMan::Clear() {
	m_ResX = 640;
	m_ResY = 480;
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
	m_pPlayerScreen = 0;
	m_PlayerScreenWidth = 0;
	m_PlayerScreenHeight = 0;
	m_pScreendumpBuffer = 0;
	m_pBackBuffer8 = 0;
	m_pBackBuffer32 = 0;
	m_DrawNetworkBackBuffer = false;
	m_StoreNetworkBackBuffer = false;
	m_NetworkFrameCurrent = 0;
	m_NetworkFrameReady = 1;
	m_BPP = 8;
	m_PaletteFile.Reset();
	//m_pPaletteDataFile = 0;
	m_BlackColor = 245;
	m_AlmostBlackColor = 245;
	m_PPM = 0;
	m_pGUIScreen = 0;
	m_pLargeFont = 0;
	m_pSmallFont = 0;
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

		for (short f = 0; f < 2; f++) {
			m_pNetworkBackBufferIntermediate8[f][i] = 0;
			m_pNetworkBackBufferFinal8[f][i] = 0;
			m_pNetworkBackBufferIntermediateGUI8[f][i] = 0;
			m_pNetworkBackBufferFinalGUI8[f][i] = 0;
		}
		m_NetworkBitmapIsLocked[i] = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::Create()
{
    // Init allegro's graphics
    set_color_depth(m_BPP);

//    GFX_MODE_LIST *pList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
//    destroy_gfx_mode_list(pList);

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
				if (set_gfx_mode(m_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) != 0) {
					set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
					allegro_message("Unable to set any graphics mode because %s!", allegro_error);
					return 1;
				}
				// Successfully reverted to VGA, so set that as the current resolution
				m_ResX = m_NewResX = 640;
				m_ResY = m_NewResY = 480;
				m_NxWindowed = 1;
				m_NewNxFullscreen = m_NxFullscreen = 1;
			}
		}
	}

	// Clear the screen buffer so it doesn't flash pink
	clear_to_color(screen, (m_BPP == 8) ? m_BlackColor : 0);

	// Sets the allowed color conversions when loading bitmaps from files
	set_color_conversion(COLORCONV_MOST);

	// Load and set the palette
	if (!LoadPalette(m_PaletteFile.GetDataPath())) {
		return -1;
	}

	// Set the switching mode; what happens when the app window is switched to and fro
	set_display_switch_mode(SWITCH_BACKGROUND);
	//set_display_switch_mode(SWITCH_PAUSE);
	set_display_switch_callback(SWITCH_OUT, DisplaySwitchOut);
	set_display_switch_callback(SWITCH_IN, DisplaySwitchIn);

	// Create transparency color table
	PALETTE ccpal;
	get_palette(ccpal);
	create_trans_table(&m_LessTransTable, ccpal, 192, 192, 192, 0);
	create_trans_table(&m_HalfTransTable, ccpal, 128, 128, 128, 0);
	create_trans_table(&m_MoreTransTable, ccpal, 64, 64, 64, 0);
	// Set the one Allegro currently uses
	color_map = &m_HalfTransTable;

	// Create the back buffer, this is still in 8bpp, we will do any post-processing on the PostProcessing bitmap
	m_pBackBuffer8 = create_bitmap_ex(8, m_ResX, m_ResY);
	clear_to_color(m_pBackBuffer8, m_BlackColor);

	for (short i = 0; i < c_MaxScreenCount; i++) {
		for (short f = 0; f < 2; f++) {
			m_pNetworkBackBufferIntermediate8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
			clear_to_color(m_pNetworkBackBufferIntermediate8[f][i], m_BlackColor);
			m_pNetworkBackBufferIntermediateGUI8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
			clear_to_color(m_pNetworkBackBufferIntermediateGUI8[f][i], g_KeyColor);

			m_pNetworkBackBufferFinal8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
			clear_to_color(m_pNetworkBackBufferFinal8[f][i], m_BlackColor);
			m_pNetworkBackBufferFinalGUI8[f][i] = create_bitmap_ex(8, m_ResX, m_ResY);
			clear_to_color(m_pNetworkBackBufferFinalGUI8[f][i], g_KeyColor);
		}
	}

	// Create the post processing buffer if in 32bpp video mode, it'll be used for glow effects etc
	if (get_color_depth() == 32 && m_BPP == 32) {
		// 32bpp so we can make the cool effects. Everything up to this is 8bpp, including the back buffer
		m_pBackBuffer32 = create_bitmap_ex(32, m_ResX, m_ResY);
		//clear_to_color(m_pBackBuffer32, m_BlackColor);
	}

	m_PlayerScreenWidth = m_pBackBuffer8->w;
	m_PlayerScreenHeight = m_pBackBuffer8->h;

	// Create the splitscreen buffer
	if (m_HSplit || m_VSplit) {
		m_pPlayerScreen = create_bitmap_ex(8, m_ResX / (m_VSplit ? 2 : 1), m_ResY / (m_HSplit ? 2 : 1));
		clear_to_color(m_pPlayerScreen, m_BlackColor);
		set_clip_state(m_pPlayerScreen, 1);

		// Update these to represent the split screens
		m_PlayerScreenWidth = m_pPlayerScreen->w;
		m_PlayerScreenHeight = m_pPlayerScreen->h;
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
	} else if (propName == "TrueColorMode") {
		bool trueColor;
		reader >> trueColor;
		m_BPP = trueColor ? 32 : 8;
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
	writer.NewProperty("TrueColorMode");
	writer << (m_BPP == 32);
	writer.NewProperty("PaletteFile");
	writer << m_PaletteFile;
	writer.NewProperty("Fullscreen");
	writer << m_Fullscreen;
	writer.NewProperty("PixelsPerMeter");
	writer << m_PPM;
	writer.NewProperty("HSplitScreen");
	writer << m_HSplitOverride;
	writer.NewProperty("VSplitScreen");
	writer << m_VSplitOverride;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::Destroy() {
	destroy_bitmap(m_pBackBuffer8);
	for (short i = 0; i < c_MaxScreenCount; i++) {
		for (short f = 0; f < 2; f++) {
			destroy_bitmap(m_pNetworkBackBufferIntermediate8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferIntermediateGUI8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferFinal8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferFinalGUI8[f][i]);
		}
	}
	destroy_bitmap(m_pBackBuffer32);
	destroy_bitmap(m_pPlayerScreen);
	delete m_pGUIScreen;
	delete m_pLargeFont;
	delete m_pSmallFont;

	//if (m_pPaletteDataFile) { unload_datafile_object(m_pPaletteDataFile); }

	g_TimerMan.Destroy();
	Clear();
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

void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit) {
	// Free the previous splitscreen, if any
	if (m_pPlayerScreen)
		release_bitmap(m_pPlayerScreen);

	// Override screen splitting according to settings if needed
	if ((hSplit || vSplit) && !(hSplit && vSplit) && (m_HSplitOverride || m_VSplitOverride)) {
		hSplit = m_HSplitOverride;
		vSplit = m_VSplitOverride;
	}

	m_HSplit = hSplit;
	m_VSplit = vSplit;

	// Create the splitscreen buffer
	if (m_HSplit || m_VSplit) {
		m_pPlayerScreen = create_bitmap_ex(8, g_FrameMan.GetResX() / (m_VSplit ? 2 : 1), g_FrameMan.GetResY() / (m_HSplit ? 2 : 1));
		clear_to_color(m_pPlayerScreen, m_BlackColor);
		set_clip_state(m_pPlayerScreen, 1);

		// Update these to represent the split screens
		m_PlayerScreenWidth = m_pPlayerScreen->w;
		m_PlayerScreenHeight = m_pPlayerScreen->h;
	}
	// No splits, so set the screen dimensions equal to the back buffer
	else {
		m_PlayerScreenWidth = m_pBackBuffer8->w;
		m_PlayerScreenHeight = m_pBackBuffer8->h;
	}

	// Reset the flashes
	for (int i = 0; i < c_MaxScreenCount; ++i) {
		m_FlashScreenColor[i] = -1;
		m_FlashedLastFrame[i] = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIFont * FrameMan::GetLargeFont() {
	if (!m_pLargeFont) {
		if (!m_pGUIScreen) {
			m_pGUIScreen = new AllegroScreen(m_pBackBuffer8);
		}
		m_pLargeFont = new GUIFont("FatFont");
		m_pLargeFont->Load(m_pGUIScreen, "Base.rte/GUIs/Skins/Base/fatfont.bmp");
	}
	return m_pLargeFont;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIFont * FrameMan::GetSmallFont() {
	if (!m_pSmallFont) {
		if (!m_pGUIScreen) {
			m_pGUIScreen = new AllegroScreen(m_pBackBuffer8);
		}
		m_pSmallFont = new GUIFont("SmallFont");
		m_pSmallFont->Load(m_pGUIScreen, "Base.rte/GUIs/Skins/Base/smallfont.bmp");
	}
	return m_pSmallFont;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::CalculateTextHeight(std::string text, int maxWidth, bool isSmall) {
	return (isSmall) ? GetSmallFont()->CalculateHeight(text, maxWidth) : GetLargeFont()->CalculateHeight(text, maxWidth);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::CalculateTextWidth(std::string text, bool isSmall) {
	return (isSmall) ? GetSmallFont()->CalculateWidth(text) : GetLargeFont()->CalculateWidth(text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::SetScreenText(const std::string &msg, int which, int blinkInterval, int displayDuration, bool centered)
{
    if (which >= 0 && which < c_MaxScreenCount)
    {
        // See if we can overwrite the previous message
        if (m_TextDurationTimer[which].IsPastRealMS(m_TextDuration[which]))
        {
            m_ScreenText[which] = msg;
            m_TextDuration[which] = displayDuration;
            m_TextDurationTimer[which].Reset();
            m_TextBlinking[which] = blinkInterval;
            m_TextCentered[which] = centered;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::ClearScreenText(int which = 0) {
	if (which >= 0 && which < c_MaxScreenCount) {
		m_ScreenText[which].clear();
		m_TextDuration[which] = -1;
		m_TextDurationTimer[which].Reset();
		m_TextBlinking[which] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameMan::LoadPalette(std::string palettePath)
{
    // Look for the '#' denoting the divider between the datafile and the datafile object's name is
    // If we find one, that means we're trying to load from a datafile, otherwise it's from an exposed bitmap
    int separatorPos = palettePath.rfind('#');

    // Check whether we're trying to load a palette from an exposed bitmap or from a datafile
    if (separatorPos == -1 || separatorPos >= palettePath.length() - 1)
    {
        // Just going to discard the bitmap, we're only interested in the palette
        BITMAP *tempBitmap;
        PALETTE newPalette;
        if (!(tempBitmap = load_bitmap(palettePath.c_str(), newPalette)))
            RTEAbort(("Failed to load palette from bitmap with following path:\n\n" + palettePath).c_str());

        // Set the current palette
        set_palette(newPalette);

        // Update what black is now with the loaded palette
        m_BlackColor = bestfit_color(newPalette, 0, 0, 0);
        m_AlmostBlackColor = bestfit_color(newPalette, 5, 5, 5);

        // Free the temp bitmap that had the palette
        destroy_bitmap(tempBitmap);
    }
    // Loading from a datafile
    else {
		/*
        // Get the Path only, without the object name, using the separator index as length limiter
        string datafilePath = palettePath.substr(0, separatorPos);
        // Adjusting to the true first character of the datafile object's name string.
        string objectName = palettePath.substr(separatorPos + 1);

        // Try loading the datafile from the specified path + object names.
        DATAFILE *pTempFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

        // Make sure we loaded properly.
        if (!pTempFile || !pTempFile->dat || pTempFile->type != DAT_PALETTE) { RTEAbort(("Failed to load palette datafile object with following path and name:\n\n" + palettePath).c_str()); }      

        // Now when we know it's valid, go ahead and replace the old palette with it
		if (m_pPaletteDataFile) { unload_datafile_object(m_pPaletteDataFile); }        
        m_pPaletteDataFile = pTempFile;

        // Set the current palette
        set_palette(*((PALETTE *)m_pPaletteDataFile->dat));

        // Update what black is now with the loaded palette
        m_BlackColor = bestfit_color(*((PALETTE *)m_pPaletteDataFile->dat), 0, 0, 0);
        m_AlmostBlackColor = bestfit_color(*((PALETTE *)m_pPaletteDataFile->dat), 5, 5, 5);
		*/
    }

    // Indicate success
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::FadeInPalette(int fadeSpeed) {
	PALETTE pal;
	get_palette(pal);
	fadeSpeed = Limit(fadeSpeed, 64, 1);
	fade_in(pal, fadeSpeed);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::FadeOutPalette(int fadeSpeed) {
	fadeSpeed = Limit(fadeSpeed, 64, 1);
	fade_out(fadeSpeed);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::SetTransTable(TransperencyPreset transSetting) {
	if (transSetting == LessTrans) {
		color_map = &m_LessTransTable;
	} else if (transSetting == MoreTrans) {
		color_map = &m_MoreTransTable;
	} else {
		color_map = &m_HalfTransTable;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::SaveScreenToBMP(const char *namebase)
{
    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 name base
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", namebase, filenumber++);
        if (!exists(fullfilename)) {
            break;
        }
    } while (filenumber < maxFileTrys);

    // Save out the screen bitmap, after making a copy of it, faster sometimes
    if (screen) {
        if (!m_pScreendumpBuffer)
            m_pScreendumpBuffer = create_bitmap(screen->w, screen->h);
        blit(screen, m_pScreendumpBuffer, 0, 0, 0, 0, screen->w, screen->h);
        PALETTE palette;
        get_palette(palette);
        save_bmp(fullfilename, m_pScreendumpBuffer, palette);

        g_ConsoleMan.PrintString("SYSTEM: Screen was dumped to: " + string(fullfilename));
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::SaveWorldToBMP(const char *namebase)
{
	if (!g_ActivityMan.ActivityRunning())
		return 0;

    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 name base
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", namebase, filenumber++);
        if (!exists(fullfilename)) {
            break;
        }
    } while (filenumber < maxFileTrys);


	BITMAP * pWorldBitmap = create_bitmap_ex(32, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight());
	Vector targetPos(0,0);
    std::list<PostEffect> postEffects;

	if (pWorldBitmap)
	{
		clear_to_color(pWorldBitmap, makecol32(132, 192, 252)); // Light blue color

		//Draw sky gradient
		for (int i = 0; i < pWorldBitmap->h ; i++)
		{
			hline(pWorldBitmap, 0, i , pWorldBitmap->w - 1, makecol32(64 + (((float)i / (float)pWorldBitmap->h) * (128 - 64)), 64 + (((float)i / (float)pWorldBitmap->h) * (192 - 64)), 96 + ((float)i / (float)pWorldBitmap->h) * (255 - 96)));
		}

		// Draw scene
		draw_sprite(pWorldBitmap, g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0);
		draw_sprite(pWorldBitmap, g_SceneMan.GetTerrain()->GetFGColorBitmap(), 0, 0);

		//Draw objects
		draw_sprite(pWorldBitmap, g_SceneMan.GetMOColorBitmap(), 0, 0);

		g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, pWorldBitmap->w, pWorldBitmap->h, postEffects,-1);

		//Draw post-effects
		BITMAP *pBitmap = 0;
		int effectPosX = 0;
		int effectPosY = 0;
		int strength = 0;
		float angle = 0;

		for (list<PostEffect>::iterator eItr = postEffects.begin(); eItr != postEffects.end(); ++eItr)
		{
			pBitmap = (*eItr).m_pBitmap;
			strength = (*eItr).m_Strength;
			set_screen_blender(strength, strength, strength, strength);
	        effectPosX  = (*eItr).m_Pos.GetFloorIntX() - (pBitmap->w / 2);
			effectPosY  = (*eItr).m_Pos.GetFloorIntY() - (pBitmap->h / 2);
			angle = (*eItr).m_Angle;

			if (angle == 0)
			{
				draw_trans_sprite(pWorldBitmap, pBitmap, effectPosX, effectPosY);
			}
			else 
			{
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

int FrameMan::SaveBitmapToBMP(BITMAP *pBitmap, const char *namebase)
{
    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 name base
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf_s(fullfilename, sizeof(fullfilename), "%s%03i.bmp", namebase, filenumber++);
        if (!exists(fullfilename)) {
            break;
        }
    } while (filenumber < maxFileTrys);

    // Save out the bitmap
    if (pBitmap) {
        PALETTE palette;
        get_palette(palette);
        save_bmp(fullfilename, pBitmap, palette);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::ToggleFullscreen()
{
    // Save the palette so we can re-set it after the change.
    PALETTE pal;
    get_palette(pal);

    // Refuse windowed multiplier if the resolution is too high
    if (m_ResX > 1024)
        m_NxWindowed = 1;
/*
    // Force double virtual fullscreen res if the res is too high
    if (m_ResX >= 1280)// && m_Fullscreen && m_NxFullscreen == 1)
    {
        m_NxFullscreen = 2;
        m_NewResX = m_ResX /= m_NxFullscreen;
        m_NewResY = m_ResY /= m_NxFullscreen;
    }
    // Not oversized resolution
//    else
//        m_NxFullscreen = 1;
*/
    // Need to save these first for recovery attempts to work (screen might be 0)
    int resX = m_ResX;
    int resY = m_ResY;


	int fullscreenGfxDriver = GFX_AUTODETECT_FULLSCREEN;
	int windowedGfxDriver = GFX_AUTODETECT_WINDOWED;

    fullscreenGfxDriver = GFX_DIRECTX_ACCEL;

	if (g_SettingsMan.ForceSoftwareGfxDriver()) 
		fullscreenGfxDriver = GFX_DIRECTX_SOFT;
	if (g_SettingsMan.ForceSafeGfxDriver()) 
		fullscreenGfxDriver = GFX_DIRECTX_SAFE;
	if (g_SettingsMan.ForceOverlayedWindowGfxDriver()) 
		windowedGfxDriver = GFX_DIRECTX_OVL;
	if (g_SettingsMan.ForceNonOverlayedWindowGfxDriver()) 
		windowedGfxDriver = GFX_DIRECTX_WIN;
	if (g_SettingsMan.ForceVirtualFullScreenGfxDriver()) 
		windowedGfxDriver = GFX_DIRECTX_WIN_BORDERLESS;


    // Switch to windowed mode
    if (m_Fullscreen)
    {
        if (set_gfx_mode(windowedGfxDriver, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0)
        {
            // Oops, failed to set windowed mode, so go back to fullscreen
            if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0)
            {
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
    }
    // Switch to fullscreen
    else
    {
        if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0)
        {
            // Oops, failed to set fullscreen mode, try a different fullscreen scaling
            m_NewNxFullscreen = m_NxFullscreen = m_NxFullscreen == 1 ? 2 : 1;
            if (set_gfx_mode(fullscreenGfxDriver, resX * m_NxFullscreen, resY * m_NxFullscreen, 0, 0) != 0)
            {
                // That didn't work either, so go back to windowed
                allegro_message("Unable enter fullscreen mode because: %s!\n\nWill try to revert to windowed mode now...", allegro_error);
                if (set_gfx_mode(windowedGfxDriver, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0)
                {
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
    // If NxFullscreen, adjust the mouse speed accordingly
    if (g_FrameMan.IsFullscreen() && g_FrameMan.NxFullscreen() > 1)
        set_mouse_speed(1, 1);
    else
        set_mouse_speed(2, 2);
    set_mouse_range(0, 0, (GetResX() * mouseDenominator) - 3, (GetResY() * mouseDenominator) - 3);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::SwitchWindowMultiplier(int multiplier)
{
    // Sanity check input
    if (multiplier <= 0 || multiplier > 4 || multiplier == m_NxWindowed)
        return -1;

    // No need to do anything else if we're in fullscreen already
    if (m_Fullscreen)
    {
        m_NxWindowed = multiplier;
        return 0;
    }

    // Refuse windowed multiplier if the resolution is too high
    if (m_ResX > 1024)
        m_NxWindowed = 1;

    // Save the palette so we can re-set it after the switch.
    PALETTE pal;
    get_palette(pal);
    // Need to save these first for recovery attempts to work (screen might be 0)
    int resX = m_ResX;
    int resY = m_ResY;

    // Switch to new windowed mode
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, resX * multiplier, resY * multiplier, 0, 0) != 0)
    {
        // Oops, failed to set windowed mode, so go back to previous multiplier
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, resX * m_NxWindowed, resY * m_NxWindowed, 0, 0) != 0)
        {
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

void FrameMan::FlipFrameBuffers()
{
    if (get_color_depth() == 32 && m_BPP == 32 && m_pBackBuffer32)
    {
        if (g_InActivity)
        {
            if (!m_Fullscreen && m_NxWindowed != 1)
                stretch_blit(g_PostProcessMan.IsPostProcessing() ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else if (m_Fullscreen && m_NxFullscreen != 1)
                stretch_blit(g_PostProcessMan.IsPostProcessing() ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else
                blit(g_PostProcessMan.IsPostProcessing() ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h);
        }
        // Menu is always 32bpp
        else
        {
            if (!m_Fullscreen && m_NxWindowed != 1)
                stretch_blit(m_pBackBuffer32, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else if (m_Fullscreen && m_NxFullscreen != 1)
                stretch_blit(m_pBackBuffer32, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else
                blit(m_pBackBuffer32, screen, 0, 0, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h);            
        }
    }
    // 8bpp video mode
    else
    {
        if (!m_Fullscreen && m_NxWindowed != 1)
            stretch_blit(m_pBackBuffer8, screen, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h, 0, 0, SCREEN_W, SCREEN_H);
        else if (m_Fullscreen && m_NxFullscreen != 1)
            stretch_blit(m_pBackBuffer8, screen, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h, 0, 0, SCREEN_W, SCREEN_H);
        else
            blit(m_pBackBuffer8, screen, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameMan::FlippingWith32BPP() const { return get_color_depth() == 32 && m_BPP == 32 && m_pBackBuffer32 && g_InActivity && g_PostProcessMan.IsPostProcessing(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::DrawText(BITMAP *pTargetBitmap,
                        const string &str,
                        const Vector &pos,
                        bool black)
{
    RTEAbort("Old fonts are not ported yet!");
/*
    BITMAP *pFont = black ? m_pBlackFont : m_pWhiteFont;
    const char *drawChar = str.c_str();
    int charWidth = pFont->GetTile()->GetBlockWidth() - 1;
    int charHeight = pFont->GetTile()->GetBlockHeight() - 1;
    int lineSep = 4;
    int line = 0;

    for (int i = 0, j = 0; i < str.size(); ++i, ++j) {
        if (*drawChar == '\n') {
            i++;
            j = 0;
            line++;
            drawChar++;
        }
        pFont->SetPos(pos.m_X + (j * charWidth), pos.m_Y + (line * (charHeight + lineSep)));
        pFont->SetFrame(*(drawChar++));
        pFont->Draw(pTargetBitmap, 0, 0, CDXBLT_TRANS);
    }
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor, int skip, int skipStart, bool shortestWrap)
{
    RTEAssert(pBitmap, "Trying to draw line to null Bitmap");

//    acquire_bitmap(pBitmap);

    int hitCount = 0, error, dom, sub, domSteps, skipped = skip + (skipStart - skip);
    int intPos[2], delta[2], delta2[2], increment[2];
    bool drawAlt = false;

    // Just make the alt the same color as the main one if no one was specified
    if (altColor == 0)
        altColor = color;
/*
    // Glow intensity
    int glowIntensity = color == g_YellowGlowColor ? 55 + 200 * PosRand() : 0;
*/
    // Calculate the integer values
    intPos[X] = floorf(start.m_X);
    intPos[Y] = floorf(start.m_Y);

    // Wrap line around the scene if it makes it shorter
    if (shortestWrap)
    {
        Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
        delta[X] = floorf(deltaVec.m_X);
        delta[Y] = floorf(deltaVec.m_Y);
    }
    // No wrap
    else
    {
        delta[X] = floorf(end.m_X) - intPos[X];
        delta[Y] = floorf(end.m_Y) - intPos[Y];
    }

    if (delta[X] == 0 &&  delta[Y] == 0)
        return 0;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    Vector glowPoint;
    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only draw pixel if we're not due to skip any
        if (++skipped > skip)
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Slap a regular pixel on there
            putpixel(pBitmap, intPos[X], intPos[Y], drawAlt ? color : altColor);
/*
            // Slap a glow on there in absolute scene coordinates if enabled
            if (glowIntensity > 0)
            {
                glowPoint.SetXY(intPos[X], intPos[Y]);
                g_SceneMan.RegisterGlowDotEffect(glowPoint + targetOffset, m_pYellowEffect, glowIntensity);
            }
*/
            drawAlt = !drawAlt;
            skipped = 0;
        }
    }

//    release_bitmap(pBitmap);
    // Return the end phase state of the skipping
    return skipped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::DrawDotLine(BITMAP *pBitmap, const Vector &start, const Vector &end, BITMAP *pDot, int skip, int skipStart, bool shortestWrap)
{
    RTEAssert(pBitmap, "Trying to draw line to null Bitmap");
    RTEAssert(pDot, "Trying to draw line of dots without specifying a dot Bitmap");

//    acquire_bitmap(pBitmap);

    int hitCount = 0, error, dom, sub, domSteps, skipped = skip + (skipStart - skip);
    int intPos[2], delta[2], delta2[2], increment[2];
    bool drawAlt = false;
    int dotHalfHeight = pDot->h / 2;
    int dotHalfWidth = pDot->w / 2;

    // Calculate the integer values
    intPos[X] = floorf(start.m_X);
    intPos[Y] = floorf(start.m_Y);

    // Wrap line around the scene if it makes it shorter
    if (shortestWrap)
    {
        Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
        delta[X] = floorf(deltaVec.m_X);
        delta[Y] = floorf(deltaVec.m_Y);
    }
    // No wrap
    else
    {
        delta[X] = floorf(end.m_X) - intPos[X];
        delta[Y] = floorf(end.m_Y) - intPos[Y];
    }

    if (delta[X] == 0 &&  delta[Y] == 0)
        return 0;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    Vector glowPoint;
    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only draw pixel if we're not due to skip any
        if (++skipped > skip)
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Slap the dot on there
            masked_blit(pDot, pBitmap, 0, 0, intPos[X] - dotHalfWidth, intPos[Y] - dotHalfHeight, pDot->w, pDot->h);

            drawAlt = !drawAlt;
            skipped = 0;
        }
    }

//    release_bitmap(pBitmap);
    // Return the end phase state of the skipping
    return skipped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::Update() {
	// Update all the performance counters.
	g_PerformanceMan.Update();

	//Remove all scheduled primitives, those will be re-added by updates from other entities.
	g_PrimitiveMan.ClearPrimitivesList();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::Draw()
{
    // Count how many split screens we'll need
    int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);

    RTEAssert(screenCount <= 1 || m_pPlayerScreen, "Splitscreen surface not ready when needed!");
    // Choose which buffer to draw to. If there are no splitscreens and 8bit modes, draw directly to the back buffer, else use a intermediary splitscreen buffer
    char str[512];

    // Clear out the post processing screen effects list
	g_PostProcessMan.Reset();
    // These accumulate the effects for each player's screen area, and are then transferred to the above lists with the player screen offset applied
	list<PostEffect> screenRelativeEffects;
    list<Box> screenRelativeGlowBoxes;
    // Handy handle
    Activity *pActivity = g_ActivityMan.GetActivity();

    for (int whichScreen = 0; whichScreen < screenCount; ++whichScreen)
    {
        // Screen Update
        screenRelativeEffects.clear();
        screenRelativeGlowBoxes.clear();

		BITMAP *pDrawScreen = /*get_color_depth() == 8 && */screenCount == 1 ? m_pBackBuffer8 : m_pPlayerScreen;
		BITMAP *pDrawScreenGUI = pDrawScreen;
		if (m_StoreNetworkBackBuffer)
		{
			pDrawScreen = m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][whichScreen];
			pDrawScreenGUI = m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][whichScreen];
		}

		AllegroBitmap pPlayerGUIBitmap(pDrawScreenGUI);

        // Update the scene view to line up with a specific screen and then draw it onto the intermediate screen
        g_SceneMan.Update(whichScreen);

		// Save scene layer's offsets for each screen, 
		// server will pick them to build the frame state and send to client
		if (m_StoreNetworkBackBuffer)
		{
			int layerCount = 0;

			for (std::list<SceneLayer *>::reverse_iterator itr = g_SceneMan.GetScene()->GetBackLayers().rbegin(); itr != g_SceneMan.GetScene()->GetBackLayers().rend(); ++itr)
			{
				SLOffset[whichScreen][layerCount] = (*itr)->GetOffset();
				layerCount++;

				if (layerCount >= c_MaxLayersStoredForNetwork)
					break;
			}
		}

        Vector targetPos = g_SceneMan.GetOffset(whichScreen);
        // Adjust the drawing position on the target screen for if the target screen is larger than the scene in non-wrapping dimension.
        // Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
        if (!g_SceneMan.SceneWrapsX() && pDrawScreen->w > g_SceneMan.GetSceneWidth())
            targetPos.m_X += (pDrawScreen->w - g_SceneMan.GetSceneWidth()) / 2;
        if (!g_SceneMan.SceneWrapsY() && pDrawScreen->h > g_SceneMan.GetSceneHeight())
            targetPos.m_Y += (pDrawScreen->h - g_SceneMan.GetSceneHeight()) / 2;

		// Try to move at the frame buffer copy time to maybe prevent wonkyness
		m_TargetPos[m_NetworkFrameCurrent][whichScreen] = targetPos;

        // Draw the scene
		if (!m_StoreNetworkBackBuffer/* || g_UInputMan.KeyHeld(KEY_6)*/)
		{
			g_SceneMan.Draw(pDrawScreen, pDrawScreenGUI, targetPos);
		} 
		else 
		{
			clear_to_color(pDrawScreen, g_KeyColor);
			clear_to_color(pDrawScreenGUI, g_KeyColor);
			g_SceneMan.Draw(pDrawScreen, pDrawScreenGUI, targetPos, true, true);
		}
        // Get only the scene-relative post effects that affect this player's screen
        if (g_PostProcessMan.IsPostProcessing() && pActivity)
        {
			g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(whichScreen)));
			g_PostProcessMan.GetGlowAreasWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeGlowBoxes);

			if (IsInMultiplayerMode())
				g_PostProcessMan.SetNetworkPostEffectsList(whichScreen, screenRelativeEffects);
        }

// TODO: Find out what keeps disabling the clipping on the draw bitmap
        // Enable clipping on the draw bitmap
        set_clip_state(pDrawScreen, 1);

		//Always draw seam in debug mode
#ifdef DEBUG_BUILD
		g_PrimitiveMan.DrawLinePrimitive(Vector(0,0),Vector(0,g_SceneMan.GetSceneHeight()), 5);
#endif

        ///////////////////////////////////////
        // Draw screen texts
        int yTextPos = 0;

        // Only draw this stuff for actual human players
        if (whichScreen < g_ActivityMan.GetActivity()->GetHumanCount())
        {
            // Team of current screen's player
//            int team = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));

            yTextPos += 12;

            // Message
            if (!m_ScreenText[whichScreen].empty())
            {
				if (IsInMultiplayerMode())
				{
					if (m_TextCentered[whichScreen])
						yTextPos = (GetPlayerFrameBufferHeight(whichScreen) / 2) - 52;
					int occOffsetX = g_SceneMan.GetScreenOcclusion(whichScreen).m_X;

					// Draw blinking effect, but not of the text message itself, but some characters around it (so it's easier to read the message)
					if (m_TextBlinking[whichScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[whichScreen]))
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerFrameBufferWidth(whichScreen) + occOffsetX) / 2, yTextPos, (">>> " + m_ScreenText[whichScreen] + " <<<").c_str(), GUIFont::Centre);
					else
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerFrameBufferWidth(whichScreen) + occOffsetX) / 2, yTextPos, m_ScreenText[whichScreen].c_str(), GUIFont::Centre);
				}
				else
				{
					if (m_TextCentered[whichScreen])
						yTextPos = (GetPlayerScreenHeight() / 2) - 52;
					int occOffsetX = g_SceneMan.GetScreenOcclusion(whichScreen).m_X;

					// If there's really no room to offset the text into, then don't
					if (GetPlayerScreenWidth() <= GetResX() / 2)
						occOffsetX = 0;
					// Draw blinking effect, but not of the text message itself, but some characters around it (so it's easier to read the message)
					if (m_TextBlinking[whichScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[whichScreen]))
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerScreenWidth() + occOffsetX) / 2, yTextPos, (">>> " + m_ScreenText[whichScreen] + " <<<").c_str(), GUIFont::Centre);
					else
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerScreenWidth() + occOffsetX) / 2, yTextPos, m_ScreenText[whichScreen].c_str(), GUIFont::Centre);
				}
                yTextPos += 12;
            }

			////////////////////////////////////////////////////////////////
            // Performance stats
			g_PerformanceMan.Draw(pPlayerGUIBitmap);
        }
        // If superfluous screen (as in a three-player match), make the fourth the Observer one
        else
        {
//            yTextPos += 12;
            GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, GetPlayerScreenWidth() / 2, yTextPos, "- Observer View -", GUIFont::Centre);
        }

        ////////////////////////////////////////
        // If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer

        // The position of the current draw screen on the final screen
        Vector screenOffset;

        if (screenCount > 1)
        {
            // Always upper left corner
            if (whichScreen == 0)
                screenOffset.SetIntXY(0, 0);
            else if (whichScreen == 1)
            {
                // If both splits, or just VSplit, then in upper right quadrant
                if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit))
                    screenOffset.SetIntXY(GetResX() / 2, 0);
                // If only HSplit, then lower left quadrant
                else
                    screenOffset.SetIntXY(0, GetResY() / 2);
            }
            // Always lower left quadrant
            else if (whichScreen == 2)
                screenOffset.SetIntXY(0, GetResY() / 2);
            // Always lower right quadrant
            else if (whichScreen == 3)
                screenOffset.SetIntXY(GetResX() / 2, GetResY() / 2);
        }

        // Flash the screen if we're supposed to
        if (m_FlashScreenColor[whichScreen] != -1)
        {
            // If set to flash for a period of time, first be solid and then start flashing slower
            float left = m_FlashTimer[whichScreen].LeftTillRealTimeLimitMS();
            if (/*left > 500 || */left < 10 || m_FlashTimer[whichScreen].AlternateReal(50))
            {
                // At most, only flash every other frame
                if (m_FlashedLastFrame[whichScreen])
                    m_FlashedLastFrame[whichScreen] = false;
                else
                {
                    // FLASH!
                    rectfill(pDrawScreenGUI, 0, 0, pDrawScreenGUI->w, pDrawScreenGUI->h, m_FlashScreenColor[whichScreen]);
                    // Show that we did indeed flash this frame
                    m_FlashedLastFrame[whichScreen] = true;
                }
            }
            // Make things go into slight slow-mo - DANGER
//            g_TimerMan.SetOneSimUpdatePerFrame(!m_SloMoTimer.IsPastSimTimeLimit());
//            g_TimerMan.IsOneSimUpdatePerFrame();
            // Stop with the flash after the designated period
            if (m_FlashTimer[whichScreen].IsPastRealTimeLimit())
                m_FlashScreenColor[whichScreen] = -1;
        }

        // Draw the intermediate draw splitscreen to the appropriate spot on the back buffer
		if (!m_StoreNetworkBackBuffer)
	        blit(pDrawScreen, m_pBackBuffer8, 0, 0, screenOffset.GetFloorIntX(), screenOffset.GetFloorIntY(), pDrawScreen->w, pDrawScreen->h);

        // Add the player screen's effects to the total screen effects list so they can be drawn in post processing
        if (g_PostProcessMan.IsPostProcessing() && !IsInMultiplayerMode())
        {
            int occX = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntX();
            int occY = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntY();

			// Copy post effects received by client if in network mode
			if (m_DrawNetworkBackBuffer)
			{
				g_PostProcessMan.GetNetworkPostEffectsList(0, screenRelativeEffects);
			}

            // Adjust for the player screen's position on the final buffer
            for (list<PostEffect>::iterator eItr = screenRelativeEffects.begin(); eItr != screenRelativeEffects.end(); ++eItr)
            {
                // Make sure we won't be adding any effects to a part of the screen that is occluded by menus and such
                if ((*eItr).m_Pos.m_X > occX && (*eItr).m_Pos.m_Y > occY && (*eItr).m_Pos.m_X < pDrawScreen->w + occX && (*eItr).m_Pos.m_Y < pDrawScreen->h + occY)
					g_PostProcessMan.GetPostScreenEffectsList()->push_back( PostEffect((*eItr).m_Pos + screenOffset, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle) );
            }

            // Adjust glow areas for the player screen's position on the final buffer
            for (list<Box>::iterator bItr = screenRelativeGlowBoxes.begin(); bItr != screenRelativeGlowBoxes.end(); ++bItr)
            {
				g_PostProcessMan.GetPostScreenGlowBoxesList()->push_back(*bItr);
                // Adjust each added glow area for the player screen's position on the final buffer
				g_PostProcessMan.GetPostScreenGlowBoxesList()->back().m_Corner += screenOffset;
            }
        }
    }



// Done in MovableMan's Update now
//    g_SceneMan.ClearMOColorLayer();
    // Clears the pixels that have been revealed from the unseen layers
    g_SceneMan.ClearSeenPixels();

	if (!m_StoreNetworkBackBuffer)
	{
		// Draw split screen lines
		acquire_bitmap(m_pBackBuffer8);
		if (m_HSplit)
		{
			// Draw a horizontal separating line
			hline(m_pBackBuffer8, 0, (m_pBackBuffer8->h / 2) - 1, m_pBackBuffer8->w - 1, m_AlmostBlackColor);
			hline(m_pBackBuffer8, 0, (m_pBackBuffer8->h / 2), m_pBackBuffer8->w - 1, m_AlmostBlackColor);
		}
		if (m_VSplit)
		{
			// Draw a vertical separating line
			vline(m_pBackBuffer8, (m_pBackBuffer8->w / 2) - 1, 0, m_pBackBuffer8->h - 1, m_AlmostBlackColor);
			vline(m_pBackBuffer8, (m_pBackBuffer8->w / 2), 0, m_pBackBuffer8->h - 1, m_AlmostBlackColor);
		}

		// Draw the console on top of the 8 bpp buffer if post or 32bpp mode is active
	//    if (!FlippingWith32BPP())
	//        g_ConsoleMan.Draw(m_pBackBuffer32);

		// Replace 8 bit backbuffer contents with network received image before post-processing as it is where this buffer is copied to 32 bit buffer
		if (m_DrawNetworkBackBuffer)
		{
			m_NetworkBitmapIsLocked[0] = true;
			blit(m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0], m_pBackBuffer8, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
			masked_blit(m_pNetworkBackBufferFinalGUI8[m_NetworkFrameReady][0], m_pBackBuffer8, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);

			if (g_UInputMan.FlagAltState() || g_UInputMan.FlagCtrlState() || g_UInputMan.FlagShiftState()) { g_PerformanceMan.DrawCurrentPing(); }

			m_NetworkBitmapIsLocked[0] = false;
		}
	}

	//m_StoreNetworkBackBuffer = false;
	if (m_StoreNetworkBackBuffer) {
		// Blit all four internal player screens onto the backbuffer
		for (int i = 0; i < c_MaxScreenCount; i++) {
			int dx = 0;
			int dy = 0;
			int dw = m_pBackBuffer8->w / 2;
			int dh = m_pBackBuffer8->h / 2;

			if (i == 1) {
				dx = dw;
			} else if (i == 2) {
				dy = dh;
			} else if (i == 3) {
				dx = dw;
				dy = dh;
			}

			//m_TargetPos[i] = g_SceneMan.GetOffset(i);

			m_NetworkBitmapIsLocked[i] = true;
			blit(m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][i], m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->h);
			blit(m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][i], m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->w, m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->h);
			m_NetworkBitmapIsLocked[i] = false;

			// Draw all player's screen into one
			if (g_UInputMan.KeyHeld(KEY_5)) {
				stretch_blit(m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->h, dx, dy, dw, dh);
			}
		}

		if (g_UInputMan.KeyHeld(KEY_1)) {
			stretch_blit(m_pNetworkBackBufferFinal8[0][0], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_2)) {
			stretch_blit(m_pNetworkBackBufferFinal8[1][0], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][1]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][1]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_3)) {
			stretch_blit(m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(KEY_4)) {
			stretch_blit(m_pNetworkBackBufferFinal8[m_NetworkFrameReady][3], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][3]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][3]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}

		// Rendering complete, we can finally mark current frame as ready
		// This is needed to make rendering look totally atomic for the server pulling data in separate threads
		//m_NetworkFrameReady = 1;
		//m_NetworkFrameCurrent = 1;
		m_NetworkFrameReady = m_NetworkFrameCurrent;
		if (m_NetworkFrameCurrent == 0)
			m_NetworkFrameCurrent = 1;
		else 
			m_NetworkFrameCurrent = 0;
		//m_NetworkFrameReady = 1;
		//m_NetworkFrameCurrent = 0;

		/*for (int i = 0; i < c_MaxScreenCount; i++)
		{
			m_NetworkBitmapIsLocked[i] = true;
			blit(m_pBackBuffer8, m_pNetworkBackBuffer8[i], 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
			m_NetworkBitmapIsLocked[i] = false;
		}*/
	}

    // Do post-processing effects, if applicable and enabled
    if (g_PostProcessMan.IsPostProcessing() && g_InActivity && m_BPP == 32) { g_PostProcessMan.PostProcess(); }

    // Draw the console on top of everything
    if (FlippingWith32BPP()) { g_ConsoleMan.Draw(m_pBackBuffer32); }

    release_bitmap(m_pBackBuffer8);

    // Reset the frame timer so we can measure how much it takes until next frame being drawn
	g_PerformanceMan.ResetFrameTimer();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameMan::CreateNewPlayerBackBuffer(int player, int w, int h)
{
	for (int f = 0; f < 2; f++)
	{
		destroy_bitmap(m_pNetworkBackBufferIntermediate8[f][player]);
		m_pNetworkBackBufferIntermediate8[f][player] = create_bitmap_ex(8, w, h);
		destroy_bitmap(m_pNetworkBackBufferIntermediateGUI8[f][player]);
		m_pNetworkBackBufferIntermediateGUI8[f][player] = create_bitmap_ex(8, w, h);

		destroy_bitmap(m_pNetworkBackBufferFinal8[f][player]);
		m_pNetworkBackBufferFinal8[f][player] = create_bitmap_ex(8, w, h);
		destroy_bitmap(m_pNetworkBackBufferFinalGUI8[f][player]);
		m_pNetworkBackBufferFinalGUI8[f][player] = create_bitmap_ex(8, w, h);
	}

	m_PlayerScreenWidth = w;
	m_PlayerScreenHeight = h;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const
{ 
	if (m_StoreNetworkBackBuffer)
	{
		if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount)
		{
			int w = GetResX();
			for (int i = 0; i < c_MaxScreenCount; i++)
				if (m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i] && m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->w < w)
					w = m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->w;
			return w;
		}
		else
		{
			if (m_pNetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer])
				return m_pNetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->w;
		}
	}
	return m_PlayerScreenWidth; 
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FrameMan::GetPlayerFrameBufferHeight(int whichPlayer) const 
{ 
	if (m_StoreNetworkBackBuffer)
	{
		if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount)
		{
			int h = GetResY();
			for (int i = 0; i < c_MaxScreenCount; i++)
				if (m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i] && m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->h < h)
					h = m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->h;
			return h;
		}
		else 
		{
			if (m_pNetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer])
				return m_pNetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->h;
		}
	}
	return m_PlayerScreenHeight; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Callback function for the allegro set_display_switch_callback. It will be called when focus is switched away to the game window.
/// </summary>
void DisplaySwitchOut(void) { g_UInputMan.DisableMouseMoving(true); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Callback function for the allegro set_display_switch_callback. It will be called when focus is switched back to the game window. It will temporarily disable positioning of the mouse.
/// This is so that when focus is switched back to the game window, it avoids having the window fly away because the user clicked the title bar of the window.
/// </summary>
void DisplaySwitchIn(void) { g_UInputMan.DisableMouseMoving(false); }
}