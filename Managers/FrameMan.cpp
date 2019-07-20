//////////////////////////////////////////////////////////////////////////////////////////
// File:            FrameMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the FrameMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <mutex>

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "LicenseMan.h"
#include "ConsoleMan.h"
#include "AudioMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"
#include "MovableMan.h"
#include "SLTerrain.h"
#include "MOSprite.h"
#include "Scene.h"


#include "UInputMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"

#ifdef __USE_SOUND_FMOD
#include "fmod.h"
#endif

// I know this is a crime, but if I include it in FrameMan.h the whole thing will collapse due to int redefinitions in Allegro
std::mutex ScreenRelativeEffectsMutex[MAXSCREENCOUNT];

using std::list;
using std::pair;
using std::deque;

#define MSPFAVERAGESAMPLESIZE 10

extern bool g_ResetActivity;
extern bool g_InActivity;

namespace RTE
{

const string FrameMan::m_ClassName = "FrameMan";

//////////////////////////////////////////////////////////////////////////////////////////
// Callback function for the allegro set_display_switch_callback. It will be called when
// focus is swtiched away to the game window. 

void DisplaySwitchOut(void)
{
    g_UInputMan.DisableMouseMoving(true);
//    g_ActivityMan.PauseActivity();
//    g_InActivity = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Callback function for the allegro set_display_switch_callback. It will be called when
// focus is swtiched back to the game window. It will temporarily disable positioniong of
// the mouse. This is so that when focus is swtiched back to the game window, it avoids
// having the window fly away because the user clicked the title bar of the window.

void DisplaySwitchIn(void)
{
    g_UInputMan.DisableMouseMoving(false);
//    g_UInputMan.ReInitKeyboard();
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:			Translate coordinates
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Translates coordinats from scene to this bitmap
//                  
// Arguments:       bitmap offset, scene coordinates
// Return value:    Vector with translated coordinates
Vector FrameMan::GraphicalPrimitive::TranslateCoordinates(Vector targetPos, Vector scenePos)
{
	return scenePos - targetPos;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			TranslateCoordinates
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Translates coordinats from scene to this bitmap offset producing two coordinates 
//					for 'left' scene bitmap with negative values as if scene seam is 0,0 and
//					for 'right' dcini bitmap with positive values.
//                  
// Arguments:       Bitmap to draw on, bitmap offset, scene coordinates, translated coordinates
// Return value:    Vector with translated coordinates
void FrameMan::GraphicalPrimitive::TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos)
{
	// Unfortunately it's hard to explain how this works. It tries to represent scene bitmap as two parts
	// with center in 0,0. Right part is just plain visible part with coordinates from [0, scenewidth]
	// and left part is imaginary bitmap as if we traversed it across the seam right-to-left with coordinates [0, -scenewidth].
	// So in order to be drawn each screen coordinates calculated twice for left and right 'bitmaps' and then one of them
	// either flies away offscreen or gets drawn on the screen.
	// When we cross the seam either left or right part is actually drawn in the bitmap, and negative coords of right
	// part are compensated by view point offset coordinates when we cross the seam right to left. I really don't know
	// how to make it simpler, becuase it has so many special cases and simply wrapping all out-of-the scene coordinates
	// don't work because this way nithing will be ever draw across the seam. You're welcom to rewrite this nightmare if you can
	// I wasted a whole week on this (I can admit that I'm just too dumb for this) )))
	drawLeftPos = scenePos;
	drawRightPos = scenePos;

	if (g_SceneMan.SceneWrapsX()) 
	{
		int sceneWidth = g_SceneMan.GetSceneWidth(); 

		if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth  / 2)
			targetPos.m_X = targetPos.m_X - sceneWidth;
		 
		if (drawLeftPos.m_X > 0) 
			drawLeftPos.m_X -= sceneWidth;
		else
			drawLeftPos.m_X -= sceneWidth + targetPos.m_X;
	}

	drawLeftPos.m_X -= targetPos.m_X; 
	drawRightPos.m_X -= targetPos.m_X; 

	if (g_SceneMan.SceneWrapsY()) 
	{
		int sceneHeight = g_SceneMan.GetSceneHeight(); 

		if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight  / 2)
			targetPos.m_Y = targetPos.m_Y - sceneHeight;
		 
		if (drawLeftPos.m_Y > 0) 
			drawLeftPos.m_Y -= sceneHeight;
		else
			drawLeftPos.m_Y -= sceneHeight + targetPos.m_Y;
	}

	drawLeftPos.m_Y -= targetPos.m_Y; 
	drawRightPos.m_Y -= targetPos.m_Y; 
} 



void FrameMan::LinePrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		Vector drawEnd = m_End - targetPos; 
		 
		line(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;

		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_End, drawEndLeft, drawEndRight);

		line(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
		line(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this primitive on provided bitmap
//                  
// Arguments:       Bitmap to draw on, bitmap offset
// Return value:    None
void FrameMan::BoxPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		Vector drawEnd = m_End - targetPos; 
		 
		rect(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;

		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_End, drawEndLeft, drawEndRight);

		rect(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
		rect(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this primitive on provided bitmap
//                  
// Arguments:       Bitmap to draw on, bitmap offset
// Return value:    None
void FrameMan::BoxFillPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		Vector drawEnd = m_End - targetPos; 
		 
		rectfill(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;

		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_End, drawEndLeft, drawEndRight);

		rectfill(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
		rectfill(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
	}
}


void FrameMan::CirclePrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		 
		circle(pDrawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);

		circle(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
		circle(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
	}
}


void FrameMan::CircleFillPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		 
		circlefill(pDrawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);

		circlefill(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
		circlefill(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
	}
}

void FrameMan::TextPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos; 
		AllegroBitmap pPlayerGUIBitmap(pDrawScreen);

		if (m_IsSmall)
			g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
		else
			g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);

		AllegroBitmap pPlayerGUIBitmap(pDrawScreen);
		if (m_IsSmall)
		{
			g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
			g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
		}
		else
		{
			g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
			g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
		}
	}
}


void FrameMan::BitmapPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) 
{
	if (!m_pBitmap)
		return;

	Vector pos[2];
	int passes = 0;

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) 
	{ 
		Vector drawStart = m_Start - targetPos;

		pos[0] = drawStart;
		passes = 1;

	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_Start, drawStartLeft, drawStartRight);

		pos[0] = drawStartLeft;
		pos[1] = drawStartRight;
		passes = 2;
	}

	Matrix rotation = Matrix(m_RotAngle);

	for (int i = 0 ; i < 2 ; i++)
	{
		// Take into account the h-flipped pivot point
		pivot_scaled_sprite(pDrawScreen,
							m_pBitmap,
							pos[i].GetFloorIntX(),
							pos[i].GetFloorIntY(),
							m_pBitmap->w / 2,
							m_pBitmap->h / 2,
							ftofix(rotation.GetAllegroAngle()),
							ftofix(1.0));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmapPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a bitmap primitive.
// Arguments:       Position of primitive in scene coordintaes, an entity to sraw sprite from, 
//					rotation angle in radians, frame to draw
// Return value:    None.

	void FrameMan::DrawBitmapPrimitive(Vector start, Entity * pEntity, float rotAngle, int frame)
	{
		MOSprite * pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS)
		{
			BITMAP * pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap)
				m_Primitives.push_back(new BitmapPrimitive(start, pBitmap, rotAngle));
		}
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmapPrimitive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Schedule to draw a bitmap primitive.
// Arguments:       Player screen to draw primitive on. Position of primitive in scene coordintaes, an entity to sraw sprite from, 
//					rotation angle in radians, frame to draw
// Return value:    None.

	void FrameMan::DrawBitmapPrimitive(int player, Vector start, Entity * pEntity, float rotAngle, int frame)
	{
		MOSprite * pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS)
		{
			BITMAP * pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap)
				m_Primitives.push_back(new BitmapPrimitive(player, start, pBitmap, rotAngle));
		}
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this FrameMan, effectively
//                  resetting the members of this abstraction level only.

void FrameMan::Clear()
{
    m_ResX = 640;
    m_ResY = 480;
    m_BPP = 8;
    m_NewResX = m_ResX;
    m_NewResY = m_ResY;
    m_pBackBuffer8 = 0;
	m_DrawNetworkBackBuffer = false;
	m_StoreNetworkBackBuffer = false;
	m_pBackBuffer32 = 0;
    m_pScreendumpBuffer = 0;
    m_PaletteFile.Reset();
    m_pPaletteDataFile = 0;
    m_BlackColor = 245;
    m_AlmostBlackColor = 245;
    m_Fullscreen = false;
    m_NxWindowed = 1;
    m_NxFullscreen = 1;
    m_NewNxFullscreen = 1;
    m_PostProcessing = false;
    m_PostPixelGlow = false;
    m_pYellowGlow = 0;
    m_YellowGlowHash = 0;
    m_pRedGlow = 0;
    m_RedGlowHash = 0;
    m_pBlueGlow = 0;
    m_BlueGlowHash = 0;
    m_PostScreenEffects.clear();
    m_HSplit = false;
    m_VSplit = false;
    m_HSplitOverride = false;
    m_VSplitOverride = false;
    m_pPlayerScreen = 0;
    m_PlayerScreenWidth = 0;
    m_PlayerScreenHeight = 0;
    m_PPM = 0;
    m_pFrameTimer = 0;
    m_MSPFs.clear();
    m_MSPFAverage = 0;
    m_SimSpeed = 1.0;
    m_ResetRTE = false;
    m_pGUIScreen = 0;
    m_pLargeFont = 0;
    m_pSmallFont = 0;
    m_ShowPerfStats = false;
	m_CurrentPing = 0;

	m_NetworkFrameCurrent = 0;
	m_NetworkFrameReady = 1;

	// Delete all allocated primitives and clear the list
	ClearPrimitivesList();

    for (int i = 0; i < MAXSCREENCOUNT; ++i)
    {
        m_ScreenText[i].clear();
        m_TextDuration[i] = -1;
        m_TextDurationTimer[i].Reset();
        m_TextBlinking[i] = 0;
        m_TextCentered[i] = false;
        m_FlashScreenColor[i] = -1;
        m_FlashedLastFrame[i] = false;
        m_FlashTimer[i].Reset();

		for (int f = 0; f < 2; f++)
		{
			m_pNetworkBackBufferIntermediate8[f][i] = 0;
			m_pNetworkBackBufferFinal8[f][i] = 0;
			m_pNetworkBackBufferIntermediateGUI8[f][i] = 0;
			m_pNetworkBackBufferFinalGUI8[f][i] = 0;
		}
		m_NetworkBitmapIsLocked[i] = false;

		m_ScreenRelativeEffects->clear();
    }
    m_TextBlinkTimer.Reset();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the FrameMan object ready for use.

int FrameMan::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return Create();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the FrameMan object ready for use, which is to be used with
//                  SettingsMan first.

int FrameMan::Create()
{
    m_pFrameTimer = new Timer();

    // Init allegro's graphics
    set_color_depth(m_BPP);

//    GFX_MODE_LIST *pList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
//    destroy_gfx_mode_list(pList);

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
    else
        m_NxFullscreen = 1;
*/

	int fullscreenGfxDriver = GFX_AUTODETECT_FULLSCREEN;
	int windowedGfxDriver = GFX_AUTODETECT_WINDOWED;

#if defined(__APPLE__)
	//fullscreenGfxDriver = GFX_QUARTZ_FULLSCREEN;
	//windowedGfxDriver = GFX_QUARTZ_WINDOW
#elif defined(__unix__)
	//fullscreenGfxDriver = GFX_XWINDOWS_FULLSCREEN;
	//windowedGfxDriver = GFX_XWINDOWS;
#else
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
#endif // defined(__APPLE__)


    if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0)
    {
		g_ConsoleMan.PrintString("Failed to set gfx mode, trying different windowed scaling.");

		// If player somehow managed to set up a windowed 2X mode and then set a resoultion higher than physical, then disable 2X resolution
		m_NxWindowed = m_NxWindowed == 2 ? 1 : m_NxWindowed;
		if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0)
	    {
			g_ConsoleMan.PrintString("Failed to set gfx mode, trying different fullscreen scaling.");

			// TODO: this is whack if we're attempting windowed
			// Oops, failed to set fullscreen mode, try a different fullscreen scaling
			m_NewNxFullscreen = m_NxFullscreen = m_NxFullscreen == 1 ? 2 : 1;
			if (set_gfx_mode(m_Fullscreen ? fullscreenGfxDriver : windowedGfxDriver, m_Fullscreen ? m_ResX * m_NxFullscreen : m_ResX * m_NxWindowed, m_Fullscreen ? m_ResY * m_NxFullscreen : m_ResY * m_NxWindowed, 0, 0) != 0)
			{
				// Oops, failed to set the resolution specified in the setting file, so default to a safe one instead
				allegro_message("Unable to set specified graphics mode because: %s!\n\nNow trying to default back to VGA...", allegro_error);
				if (set_gfx_mode(m_Fullscreen ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) != 0)
				{
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
    if (m_BPP == 8)
        clear_to_color(screen, m_BlackColor);
    else
        clear_to_color(screen, 0);

    // Sets the allowed color conversions when loading bitmaps from files
    set_color_conversion(COLORCONV_MOST);

    // Load and set the palette
    if (!LoadPalette(m_PaletteFile.GetDataPath()))
        return -1;

    // Set the switching mode; what happens when the app window is switched to and fro
    set_display_switch_mode(SWITCH_BACKGROUND);
//    set_display_switch_mode(SWITCH_PAUSE);
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

    // Create the back buffer, this is still in 8bpp, we will do any postprocessing on the PostProcessing bitmap
    m_pBackBuffer8 = create_bitmap_ex(8, m_ResX, m_ResY);
    clear_to_color(m_pBackBuffer8, m_BlackColor);

	for (int i = 0; i < MAXSCREENCOUNT; i++)
	{
		for (int f = 0; f < 2; f++)
		{
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
    if (get_color_depth() == 32 && m_BPP == 32)
    {
        // 32bpp so we can make ze cool effects. Everyhting up to this is 8bpp, including the back buffer
        m_pBackBuffer32 = create_bitmap_ex(32, m_ResX, m_ResY);
//        clear_to_color(m_pBackBuffer32, m_BlackColor);

// TODO: Make more robust and load more glows!
        ContentFile glowFile("Base.rte/Effects/Glows/YellowTiny.bmp");
        m_pYellowGlow = glowFile.GetAsBitmap();
		m_YellowGlowHash = glowFile.GetHash();
        glowFile.SetDataPath("Base.rte/Effects/Glows/RedTiny.bmp");
        m_pRedGlow = glowFile.GetAsBitmap();
        m_RedGlowHash = glowFile.GetHash();
        glowFile.SetDataPath("Base.rte/Effects/Glows/BlueTiny.bmp");
        m_pBlueGlow = glowFile.GetAsBitmap();
        m_BlueGlowHash = glowFile.GetHash();

		m_pTempEffectBitmap_16 = create_bitmap_ex(32, 16, 16);
		m_pTempEffectBitmap_32 = create_bitmap_ex(32, 32, 32);
		m_pTempEffectBitmap_64 = create_bitmap_ex(32, 64, 64);
		m_pTempEffectBitmap_128 = create_bitmap_ex(32, 128, 128);
		m_pTempEffectBitmap_256 = create_bitmap_ex(32, 256, 256);
	}

    m_PlayerScreenWidth = m_pBackBuffer8->w;
    m_PlayerScreenHeight = m_pBackBuffer8->h;

    // Create the splitscreen buffer
    if (m_HSplit || m_VSplit)
    {
// TODO: Make this a Video-only memory bitmap: create_video_bitmap"
        m_pPlayerScreen = create_bitmap_ex(8, m_ResX / (m_VSplit ? 2 : 1), m_ResY / (m_HSplit ? 2 : 1));
        clear_to_color(m_pPlayerScreen, m_BlackColor);
        set_clip_state(m_pPlayerScreen, 1);

        // Update these to represent the split screens
        m_PlayerScreenWidth = m_pPlayerScreen->w;
        m_PlayerScreenHeight = m_pPlayerScreen->h;
    }

	m_Sample = 0;

	for (int c = 0; c < PERF_COUNT; ++c)
	{
		for (int i = 0; i < MAXSAMPLES; ++i)
		{
			m_PerfData[c][i] = 0;
			m_PerfPercentages[c][i] = 0;
		}
		m_PerfMeasureStart[c] = 0;
		m_PerfMeasureStop[c] = 0;
	}

	//Set up performance counter's names
	m_PerfCounterNames[PERF_SIM_TOTAL] = "Total";
	m_PerfCounterNames[PERF_ACTORS_PASS1] = "Act Travel";
    m_PerfCounterNames[PERF_PARTICLES_PASS1] = "Prt Travel";
	m_PerfCounterNames[PERF_ACTORS_PASS2] = "Act Update";
    m_PerfCounterNames[PERF_PARTICLES_PASS2] = "Prt Update";
	m_PerfCounterNames[PERF_ACTORS_AI] = "Act AI";
    m_PerfCounterNames[PERF_ACTIVITY] = "Activity";
#if __USE_SOUND_GORILLA
	m_PerfCounterNames[PERF_SOUND] = "Sound";
#endif
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsValidResolution	
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if this resolution is supported
bool FrameMan::IsValidResolution(int width, int height) const
{
	int actualWidth = width;
	int actualHeight = height;

	// If width is greater than 1280, the game will switch itself in 2X mode
	// lowering actual resolution twice
	if (width >= 1280)
	{
		actualWidth = width / 2;
		actualHeight = height / 2;
	}

	if (actualWidth < 360 || actualHeight < 360)
		return false;
	else
		return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int FrameMan::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ResolutionX")
    {
        reader >> m_ResX;
        m_NewResX = m_ResX;
    }
    else if (propName == "ResolutionY")
    {
        reader >> m_ResY;
        m_NewResY = m_ResY;
    }
    else if (propName == "TrueColorMode")
    {
        bool trueColor;
        reader >> trueColor;
        m_BPP = trueColor ? 32 : 8;
    }
    else if (propName == "PaletteFile")
        reader >> m_PaletteFile;
    else if (propName == "Fullscreen")
    {
        reader >> m_Fullscreen;
#ifdef FULLSCREEN_OVERRIDE
        m_Fullscreen = true;
#endif // FULLSCREEN_OVERRIDE
    }
    else if (propName == "NxWindowed")
        reader >> m_NxWindowed;
    else if (propName == "NxFullscreen")
    {
        reader >> m_NxFullscreen;
        m_NewNxFullscreen = m_NxFullscreen;
    }
    else if (propName == "PostProcessing")
        reader >> m_PostProcessing;
    else if (propName == "PostPixelGlow")
        reader >> m_PostPixelGlow;
    else if (propName == "PixelsPerMeter")
    {
        reader >> m_PPM;
        m_MPP = 1 / m_PPM;
        // Calculate the Pixle per Litre and Litre per Pixel
        float cmPP = 100 / m_PPM;
        float LVolume = 10 * 10 * 10;
        m_PPL = LVolume / (cmPP * cmPP * cmPP);
        m_LPP = 1 / m_PPL;
    }
    else if (propName == "HSplitScreen")
        reader >> m_HSplitOverride;
    else if (propName == "VSplitScreen")
        reader >> m_VSplitOverride;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetSplitScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets new values for the split screen configuration.

void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit)
{
    // Free the previous splitscreen, if any
    if (m_pPlayerScreen)
        release_bitmap(m_pPlayerScreen);

	// Override screen splitting according to settings if needed
	if ((hSplit || vSplit) && !(hSplit && vSplit) && (m_HSplitOverride || m_VSplitOverride))
	{
		hSplit = m_HSplitOverride;
		vSplit = m_VSplitOverride;
	}

    m_HSplit = hSplit;
    m_VSplit = vSplit;

    // Create the splitscreen buffer
    if (m_HSplit || m_VSplit)
    {
// TODO: Make this a Video-only memory bitmap: create_video_bitmap"
        m_pPlayerScreen = create_bitmap_ex(8, g_FrameMan.GetResX() / (m_VSplit ? 2 : 1), g_FrameMan.GetResY() / (m_HSplit ? 2 : 1));
        clear_to_color(m_pPlayerScreen, m_BlackColor);
        set_clip_state(m_pPlayerScreen, 1);

        // Update these to represent the split screens
        m_PlayerScreenWidth = m_pPlayerScreen->w;
        m_PlayerScreenHeight = m_pPlayerScreen->h;
    }
    // No splits, so set the screen dimensions equal to the back buffer
    else
    {
        m_PlayerScreenWidth = m_pBackBuffer8->w;
        m_PlayerScreenHeight = m_pBackBuffer8->h;
    }

    // Reset the flashes
    for (int i = 0; i < MAXSCREENCOUNT; ++i)
    {
        m_FlashScreenColor[i] = -1;
        m_FlashedLastFrame[i] = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this FrameMan with a Writer for
//                  later recreation with Create(Reader &reader);

int FrameMan::Save(Writer &writer) const
{
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
    writer.NewProperty("PostProcessing");
    writer << m_PostProcessing;
    writer.NewProperty("PostPixelGlow");
    writer << m_PostPixelGlow;
    writer.NewProperty("PixelsPerMeter");
    writer << m_PPM;
    writer.NewProperty("HSplitScreen");
    writer << m_HSplitOverride;
    writer.NewProperty("VSplitScreen");
    writer << m_VSplitOverride;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the FrameMan object.

void FrameMan::Destroy()
{
    destroy_bitmap(m_pBackBuffer8);
	for (int i = 0; i < MAXSCREENCOUNT; i++)
	{
		for (int f = 0; f < 2; f++)
		{
			destroy_bitmap(m_pNetworkBackBufferIntermediate8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferIntermediateGUI8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferFinal8[f][i]);
			destroy_bitmap(m_pNetworkBackBufferFinalGUI8[f][i]);
		}
	}
    destroy_bitmap(m_pBackBuffer32);
    destroy_bitmap(m_pPlayerScreen);
    if (m_pPaletteDataFile)
        unload_datafile_object(m_pPaletteDataFile);
    delete m_pGUIScreen;
    delete m_pLargeFont;
    delete m_pSmallFont;
    delete m_pFrameTimer;

    g_TimerMan.Destroy();

    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLargeFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!

GUIFont * FrameMan::GetLargeFont()
{
    if (!m_pLargeFont)
    {
        if (!m_pGUIScreen)
        {
            m_pGUIScreen = new AllegroScreen(m_pBackBuffer8);
        }
        m_pLargeFont = new GUIFont("FatFont");
        m_pLargeFont->Load(m_pGUIScreen, "Base.rte/GUIs/Skins/Base/fatfont.bmp");
    }
    return m_pLargeFont;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSmallFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!

GUIFont * FrameMan::GetSmallFont()
{
    if (!m_pSmallFont)
    {
        if (!m_pGUIScreen)
        {
            m_pGUIScreen = new AllegroScreen(m_pBackBuffer8);
        }
        m_pSmallFont = new GUIFont("SmallFont");
        m_pSmallFont->Load(m_pGUIScreen, "Base.rte/GUIs/Skins/Base/smallfont.bmp");
    }
    return m_pSmallFont;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text height

int FrameMan::CalculateTextHeight(std::string text, int maxWidth, bool isSmall)
{
	if (isSmall)
		return GetSmallFont()->CalculateHeight(text, maxWidth);
	else
		return GetLargeFont()->CalculateHeight(text, maxWidth);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text height

int FrameMan::CalculateTextWidth(std::string text, bool isSmall)
{
	if (isSmall)
		return GetSmallFont()->CalculateWidth(text);
	else
		return GetLargeFont()->CalculateWidth(text);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the message to be displayed on top of each player's screen

void FrameMan::SetScreenText(const std::string &msg, int which, int blinkInterval, int displayDuration, bool centered)
{
    if (which >= 0 && which < MAXSCREENCOUNT)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the message to be displayed on top of each player's screen

std::string FrameMan::GetScreenText(int which) const
{
    if (which >= 0 && which < MAXSCREENCOUNT)
		return m_ScreenText[which];
	else
		return "";
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a palette from a .dat file and sets it as the currently used
//                  screen palette.

bool FrameMan::LoadPalette(std::string palettePath)
{
    // Look for the '#' denoting the divider between the datafile and the datafile object's name is
    // If we find one, that means we're trying to load from a datafile, otehrwise it's from an exposed bitmap
    int separatorPos = palettePath.rfind('#');

    // Check whether we're trying to load a palette from an exposed bitmap or from a datafile
    if (separatorPos == -1 || separatorPos >= palettePath.length() - 1)
    {
        // Just going to discard the bitmap, we're only interested in the palette
        BITMAP *tempBitmap;
        PALETTE newPalette;
        if (!(tempBitmap = load_bitmap(palettePath.c_str(), newPalette)))
            DDTAbort(("Failed to load palette from bitmap with following path:\n\n" + palettePath).c_str());

        // Set the current palette
        set_palette(newPalette);

        // Update what black is now with the loaded palette
        m_BlackColor = bestfit_color(newPalette, 0, 0, 0);
        m_AlmostBlackColor = bestfit_color(newPalette, 5, 5, 5);

        // Free the temp bitmap that had the palette
        destroy_bitmap(tempBitmap);
    }
    // Loading from a datafile
    else
    {
        // Get the Path only, without the object name, using the separator index as length limiter
        string datafilePath = palettePath.substr(0, separatorPos);
        // Adjusting to the true first character of the datafile object's name string.
        string objectName = palettePath.substr(separatorPos + 1);

        // Try loading the datafile from the specified path + object names.
        DATAFILE *pTempFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

        // Make sure we loaded properly.
        if (!pTempFile || !pTempFile->dat || pTempFile->type != DAT_PALETTE)
            DDTAbort(("Failed to load palette datafile object with following path and name:\n\n" + palettePath).c_str());

        // Now when we know it's valid, go ahead and replace the old palette with it
        if (m_pPaletteDataFile)
            unload_datafile_object(m_pPaletteDataFile);
        m_pPaletteDataFile = pTempFile;

        // Set the current palette
        set_palette(*((PALETTE *)m_pPaletteDataFile->dat));

        // Update what black is now with the loaded palette
        m_BlackColor = bestfit_color(*((PALETTE *)m_pPaletteDataFile->dat), 0, 0, 0);
        m_AlmostBlackColor = bestfit_color(*((PALETTE *)m_pPaletteDataFile->dat), 5, 5, 5);
    }

    // Indicate success
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeInPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette in from black at a specified speed.

void FrameMan::FadeInPalette(int fadeSpeed)
{
    if (fadeSpeed < 1)
        fadeSpeed = 1;
    if (fadeSpeed > 64)
        fadeSpeed = 64;

    PALETTE pal;
    get_palette(pal);
    fade_in(pal, fadeSpeed);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOutPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette out to black at a specified speed.

void FrameMan::FadeOutPalette(int fadeSpeed)
{
    if (fadeSpeed < 1)
        fadeSpeed = 1;
    if (fadeSpeed > 64)
        fadeSpeed = 64;

    fade_out(fadeSpeed);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTransTable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific precalculated transperency table which is used for
//                  any subsequent transparency drawing.

void FrameMan::SetTransTable(TransperencyPreset transSetting)
{
    if (transSetting == LessTrans)
        color_map = &m_LessTransTable;
    else if (transSetting == MoreTrans)
        color_map = &m_MoreTransTable;
    else
        color_map = &m_HalfTransTable;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScreenToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap to a 8bpp BMP file.

int FrameMan::SaveScreenToBMP(const char *namebase)
{
    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 namebase
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf(fullfilename, "%s%03i.bmp", namebase, filenumber++);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveWorldToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap of everything on the scene to the BMP file.

int FrameMan::SaveWorldToBMP(const char *namebase)
{
	if (!g_ActivityMan.ActivityRunning())
		return 0;

    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 namebase
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf(fullfilename, "%s%03i.bmp", namebase, filenumber++);
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

		g_SceneMan.GetPostScreenEffectsWrapped(targetPos, pWorldBitmap->w, pWorldBitmap->h, postEffects,-1);

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

				if (pBitmap->w < 16 && pBitmap->h < 16)
					pTargetBitmap = m_pTempEffectBitmap_16;
				else if (pBitmap->w < 32 && pBitmap->h < 32)
					pTargetBitmap = m_pTempEffectBitmap_32;
				else if (pBitmap->w < 64 && pBitmap->h < 64)
					pTargetBitmap = m_pTempEffectBitmap_64;
				else if (pBitmap->w < 128 && pBitmap->h < 128)
					pTargetBitmap = m_pTempEffectBitmap_128;
				else 
					pTargetBitmap = m_pTempEffectBitmap_256;

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



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveBitmapToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap to a 8bpp BMP file.

int FrameMan::SaveBitmapToBMP(BITMAP *pBitmap, const char *namebase)
{
    int filenumber = 0;
    char fullfilename[256];
    int maxFileTrys = 1000;

    // Make sure its not a 0 namebase
    if (namebase == 0 || strlen(namebase) <= 0)
        return -1;

    do {
        // Check for the file namebase001.bmp; if it exists, try 002, etc.
        sprintf(fullfilename, "%s%03i.bmp", namebase, filenumber++);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Toggles to and from fullscreen and windowed mode.

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

#if defined(__APPLE__)
	//fullscreenGfxDriver = GFX_QUARTZ_FULLSCREEN;
	//windowedGfxDriver = GFX_QUARTZ_WINDOW
#elif defined(__unix__)
	//fullscreenGfxDriver = GFX_XWINDOWS_FULLSCREEN;
	//windowedGfxDriver = GFX_XWINDOWS;
#else
    fullscreenGfxDriver = GFX_DIRECTX_ACCEL;
	//windowedGfxDriver = GFX_DIRECTX_WIN;

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
#endif // defined(__APPLE__)


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
    // If Nx fullscreen, adjust the mouse speed accordingly
    if (g_FrameMan.IsFullscreen() && g_FrameMan.NxFullscreen() > 1)
        set_mouse_speed(1, 1);
    else
        set_mouse_speed(2, 2);
    set_mouse_range(0, 0, (GetResX() * mouseDenominator) - 3, (GetResY() * mouseDenominator) - 3);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchWindowMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets and switches to a new windowed mode multiplier.

int FrameMan::SwitchWindowMultiplier(int multiplier)
{
    // Sanity check input
    if (multiplier <= 0 || multiplier > 4 || multiplier == m_NxWindowed)
        return -1;

    // No need to do anyhting else if we're in fullscreen already
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
            // Can't go back to prev mode either! total fail
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PostProcess
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current state of the back buffer, copies it, and adds post
//                  processing effects on top like glows etc. Only works in 32bpp mode.

void FrameMan::PostProcess()
{
    SLICK_PROFILE(0xFF354556);

    if (!m_PostProcessing)
        return;

    // First copy the current 8bpp backbuffer to the 32bpp buffer; we'll add effects to it
    blit(m_pBackBuffer8, m_pBackBuffer32, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);

	// Set the screen blender mode for glows
//    set_alpha_blender();
    set_screen_blender(128, 128, 128, 128);

//    acquire_bitmap(m_pBackBuffer8);
//    acquire_bitmap(m_pBackBuffer32);

    // Randomly sample the entire backbuffer, looking for pixels to put a glow on
    // NOTE THIS IS SLOW, especially on higher resolutions!
    if (m_PostPixelGlow)
    {
        int x = 0, y = 0, startX = 0, startY = 0, endX = 0, endY = 0, testpixel = 0;

        for (list<Box>::iterator bItr = m_PostScreenGlowBoxes.begin(); bItr != m_PostScreenGlowBoxes.end(); ++bItr)
        {
            startX = (*bItr).m_Corner.m_X;
            startY = (*bItr).m_Corner.m_Y;
            endX = startX + (*bItr).m_Width;
            endY = startY + (*bItr).m_Height;
            testpixel = 0;

            // Sanity check a little at least
            if (startX < 0 || startX >= m_pBackBuffer8->w || startY < 0 || startY >= m_pBackBuffer8->h ||
                endX < 0 || endX >= m_pBackBuffer8->w || endY < 0 || endY >= m_pBackBuffer8->h)
                continue;

// TODO: REMOVE TEMP DEBUG
//            rect(m_pBackBuffer32, startX, startY, endX, endY, g_RedColor);

            for (y = startY; y < endY; ++y)
            {
                for (x = startX; x < endX; ++x)
                {
                    testpixel = _getpixel(m_pBackBuffer8, x, y);

                    // YELLOW
                    if ((testpixel == g_YellowGlowColor && PosRand() < 0.9) || testpixel == 98 || (testpixel == 120 && PosRand() < 0.7))// || testpixel == 39 || testpixel == 86 || testpixel == 47 || testpixel == 48 || testpixel == 116)
                        draw_trans_sprite(m_pBackBuffer32, m_pYellowGlow, x - 2, y - 2);
                    // RED
        //            if (testpixel == 13)
        //                draw_trans_sprite(m_pBackBuffer32, m_pRedGlow, x - 2, y - 2);
                    // BLUE
                    if (testpixel == 166)
                        draw_trans_sprite(m_pBackBuffer32, m_pBlueGlow, x - 2, y - 2);
                }
            }            
        }
    }
/* This one is even slower than above method
    // How many samples to make
    int samples = 640 * 480 * 0.7;

    for (int i = 0; i < samples; ++i)
    {
        x = m_pBackBuffer8->w * PosRand();
//        y = m_pBackBuffer8->h * PosRand();
        testpixel = _getpixel(m_pBackBuffer8, x, y);
        // YELLOW
        if (testpixel == g_YellowGlowColor || testpixel == 98 || testpixel == 120)// || testpixel == 39 || testpixel == 86 || testpixel == 47 || testpixel == 48 || testpixel == 116)
            draw_trans_sprite(m_pBackBuffer32, m_pYellowGlow, x - 2, y - 2);
        // RED
//            if (testpixel == 13)
//                draw_trans_sprite(m_pBackBuffer32, m_pRedGlow, x - 2, y - 2);
        // BLUE
        if (testpixel == 166)
            draw_trans_sprite(m_pBackBuffer32, m_pBlueGlow, x - 2, y - 2);
    }
*/

    // Draw all the scene screen effects accumulated this frame
    BITMAP *pBitmap = 0;
    int effectPosX = 0;
    int effectPosY = 0;
    int strength = 0;
	float angle = 0;

    for (list<PostEffect>::iterator eItr = m_PostScreenEffects.begin(); eItr != m_PostScreenEffects.end(); ++eItr)
    {
		if ((*eItr).m_pBitmap)
		{
			pBitmap = (*eItr).m_pBitmap;
			strength = (*eItr).m_Strength;
			set_screen_blender(strength, strength, strength, strength);
			effectPosX = (*eItr).m_Pos.GetFloorIntX() - (pBitmap->w / 2);
			effectPosY = (*eItr).m_Pos.GetFloorIntY() - (pBitmap->h / 2);
			angle = (*eItr).m_Angle;
			//draw_trans_sprite(m_pBackBuffer32, pBitmap, effectPosX, effectPosY);

			if (angle == 0)
			{
				draw_trans_sprite(m_pBackBuffer32, pBitmap, effectPosX, effectPosY);
			}
			else
			{
				BITMAP * pTargetBitmap;

				if (pBitmap->w < 16 && pBitmap->h < 16)
					pTargetBitmap = m_pTempEffectBitmap_16;
				else if (pBitmap->w < 32 && pBitmap->h < 32)
					pTargetBitmap = m_pTempEffectBitmap_32;
				else if (pBitmap->w < 64 && pBitmap->h < 64)
					pTargetBitmap = m_pTempEffectBitmap_64;
				else if (pBitmap->w < 128 && pBitmap->h < 128)
					pTargetBitmap = m_pTempEffectBitmap_128;
				else
					pTargetBitmap = m_pTempEffectBitmap_256;

				clear_to_color(pTargetBitmap, 0);

				//fixed fAngle;
				//fAngle = fixmul(angle, radtofix_r);

				Matrix m;
				m.SetRadAngle(angle);

				rotate_sprite(pTargetBitmap, pBitmap, 0, 0, ftofix(m.GetAllegroAngle()));
				draw_trans_sprite(m_pBackBuffer32, pTargetBitmap, effectPosX, effectPosY);
			}
		}
    }

//    release_bitmap(m_pBackBuffer32);
//    release_bitmap(m_pBackBuffer8);

    // Set blender mode back??
//    set_trans_blender(128, 128, 128, 128);

    // Clear the effects list for this frame
    m_PostScreenEffects.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlipFrameBuffers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flips the framebuffer.

void FrameMan::FlipFrameBuffers()
{
    SLICK_PROFILE(0xFF886532);

    if (get_color_depth() == 32 && m_BPP == 32 && m_pBackBuffer32)
    {
        if (g_InActivity)
        {
            if (!m_Fullscreen && m_NxWindowed != 1)
                stretch_blit(m_PostProcessing ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else if (m_Fullscreen && m_NxFullscreen != 1)
                stretch_blit(m_PostProcessing ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h, 0, 0, SCREEN_W, SCREEN_H);
            else
                blit(m_PostProcessing ? m_pBackBuffer32 : m_pBackBuffer8, screen, 0, 0, 0, 0, m_pBackBuffer32->w, m_pBackBuffer32->h);            
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlippingWith32BPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the 32bpp framebuffer is currnetly being used or not
//                  when flipping the frame buffers.

bool FrameMan::FlippingWith32BPP() const
{
    return get_color_depth() == 32 && m_BPP == 32 && m_pBackBuffer32 && g_InActivity && m_PostProcessing;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a text string to the bitmap of choice, using the internal
//                  fontsets.

void FrameMan::DrawText(BITMAP *pTargetBitmap,
                        const string &str,
                        const Vector &pos,
                        bool black)
{
    DDTAbort("Old fonts are not ported yet!");
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted or with other effects.

int FrameMan::DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor, int skip, int skipStart, bool shortestWrap)
{
    AAssert(pBitmap, "Trying to draw line to null Bitmap");

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
            // Slap a glow on there in absoltue scene coordinates if enabled
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawDotLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted with bitmaps.

int FrameMan::DrawDotLine(BITMAP *pBitmap, const Vector &start, const Vector &end, BITMAP *pDot, int skip, int skipStart, bool shortestWrap)
{
    AAssert(pBitmap, "Trying to draw line to null Bitmap");
    AAssert(pDot, "Trying to draw line of dots without specifying a dot Bitmap");

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this FrameMan. Supposed to be done every frame.

void FrameMan::Update()
{
    // Time and add the millisecs per frame reading to the buffer
    m_MSPFs.push_back(m_pFrameTimer->GetElapsedRealTimeMS());
    // Keep the buffer trimmed
    while (m_MSPFs.size() > MSPFAVERAGESAMPLESIZE)
        m_MSPFs.pop_front();

    // Calculate the average millsecs per frame over the last sampleSize frames
    m_MSPFAverage = 0;
    for (deque<int>::iterator fItr = m_MSPFs.begin(); fItr != m_MSPFs.end(); ++fItr)
        m_MSPFAverage += *fItr;
    m_MSPFAverage /= m_MSPFs.size();

    // If one sim update per frame mode, adjust the pitch of most sound effects to match the sim time over real time ratio as it fluctuates!
    if (g_TimerMan.IsOneSimUpdatePerFrame())
    {
        // Calculate the sim speed over the actual real time
        m_SimSpeed = g_TimerMan.GetDeltaTimeMS() / (float)m_MSPFAverage;
//        float simSpeed = g_TimerMan.GetDeltaTimeSecs() / m_pFrameTimer->GetElapsedRealTimeS();
        // If limited, only allow pitch to go slower, not faster
        if (g_TimerMan.IsSimSpeedLimited() && m_SimSpeed > 1.0)
            m_SimSpeed = 1.0;
        // Soften the ratio of the pitch adjustment so it's not such an extreme effect on the audio
// TODO: Don't hardcode this coefficient - although it's a good defualt
        float pitch = m_SimSpeed + (1.0f - m_SimSpeed) * 0.35;
        // Set the pitch for all other applicable sounds other than music
        g_AudioMan.SetGlobalPitch(pitch, true);
// MUSIC PITCHING IS SUCK.. ruins the songs
        // Only affect the music if it's really slow, so it doesn't sound funny and fluctuating
// TODO: Don't hardcode this threshold - although it's a good defualt
//        g_AudioMan.SetMusicPitch(pitch >= 0.50 ? 1.0 : pitch);
    }
    else
        m_SimSpeed = 1.0;

    // Clear the back buffers
//    m_pScreen->GetBack()->Fill(0); // don't do this to avoid the black lines...look into that later.

//    g_UInputMan.DisableMouseMoving(false);

	//Remove all scheduled primitives, those will be readded by updates from other entities
	ClearPrimitivesList();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawPrimitives
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws all stored primitives on the screen
// Arguments:       Bitmap to draw on, poistion to draw.
// Return value:    None.

void FrameMan::DrawPrimitives(int player, BITMAP *pTargetBitmap, const Vector &targetPos)
{
    // Count how many split screens we'll need
    //int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
    //BITMAP *pDrawScreen = /*get_color_depth() == 8 && */screenCount == 1 ? m_pBackBuffer8 : m_pPlayerScreen;

	//Draw primitives
	for (std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it)
	{
		if (player == (*it)->m_Player || (*it)->m_Player == -1)
			(*it)->Draw(pTargetBitmap, targetPos);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the current frame to the screen.

void FrameMan::Draw()
{
    SLICK_PROFILE(0xFF684822);

    // Count how many split screens we'll need
    int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);

    AAssert(screenCount <= 1 || m_pPlayerScreen, "Splitscreen surface not ready when needed!");
    // Choose which buffer to draw to. If there are no splitscreens and 8bit modes, draw directly to the back buffer, else use a intermediary splitscreen buffer
    char str[512];

    // Clear out the post processing screen effects list
    m_PostScreenEffects.clear();
    m_PostScreenGlowBoxes.clear();
    // These accumulate the effects for each player's screen area, and are then transferred to the above lists with the player screen offset applied
	list<PostEffect> screenRelativeEffects;
    list<Box> screenRelativeGlowBoxes;
    // Handy handle
    Activity *pActivity = g_ActivityMan.GetActivity();

    for (int whichScreen = 0; whichScreen < screenCount; ++whichScreen)
    {
        SLICK_PROFILENAME("Screen Update", 0xFF321546);
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

				if (layerCount >= MAX_LAYERS_STORED_FOR_NETWORK)
					break;
			}
		}

        Vector targetPos = g_SceneMan.GetOffset(whichScreen);
        // Adjust the drawing position on the target screen for if the target screen is larger than the scene in nonwrapping dimension.
        // Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
        if (!g_SceneMan.SceneWrapsX() && pDrawScreen->w > g_SceneMan.GetSceneWidth())
            targetPos.m_X += (pDrawScreen->w - g_SceneMan.GetSceneWidth()) / 2;
        if (!g_SceneMan.SceneWrapsY() && pDrawScreen->h > g_SceneMan.GetSceneHeight())
            targetPos.m_Y += (pDrawScreen->h - g_SceneMan.GetSceneHeight()) / 2;

		// Try to move at the framebuffer copy time to maybe prevent wonkyness
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
        if (m_PostProcessing && pActivity)
        {
            g_SceneMan.GetPostScreenEffectsWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(whichScreen)));
            g_SceneMan.GetGlowAreasWrapped(targetPos, pDrawScreen->w, pDrawScreen->h, screenRelativeGlowBoxes);

			if (IsInMultiplayerMode())
				SetPostEffectsList(whichScreen, screenRelativeEffects);
        }

// TODO: Find out what keeps disabling the clipping on the draw bitmap
        // Enable clipping on the draw bitmap
        set_clip_state(pDrawScreen, 1);

		//Always draw seam in debug mode
#ifdef _DEBUG
		//DrawLinePrimitive(Vector(0,0),Vector(0,g_SceneMan.GetSceneHeight()), 5);
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

					// Draw blinking effect, but not of the text message itself, but some characters around it (so it's easier to read the msg)
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
					// Draw blinking effect, but not of the text message itself, but some characters around it (so it's easier to read the msg)
					if (m_TextBlinking[whichScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[whichScreen]))
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerScreenWidth() + occOffsetX) / 2, yTextPos, (">>> " + m_ScreenText[whichScreen] + " <<<").c_str(), GUIFont::Centre);
					else
						GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, (GetPlayerScreenWidth() + occOffsetX) / 2, yTextPos, m_ScreenText[whichScreen].c_str(), GUIFont::Centre);
				}
                yTextPos += 12;
            }

			////////////////////////////////////////////////////////////////
            // Performance stats
            if (m_ShowPerfStats && whichScreen == 0)
            {
                int sampleSize = 10;
                // Time and add the millisecs per frame reading to the buffer
                m_MSPFs.push_back(m_pFrameTimer->GetElapsedRealTimeMS());
                m_pFrameTimer->Reset();
                // Keep the buffer trimmed
                while (m_MSPFs.size() > sampleSize)
                    m_MSPFs.pop_front();

                // Calculate the average millsecs per frame over the last sampleSize frames
                int m_MSPFAverage = 0;
                for (deque<int>::iterator fItr = m_MSPFs.begin(); fItr != m_MSPFs.end(); ++fItr)
                    m_MSPFAverage += *fItr;
                m_MSPFAverage /= m_MSPFs.size();

                // Calcualte teh fps from the average
                float fps = 1.0f / ((float)m_MSPFAverage / 1000.0f);
                sprintf(str, "FPS: %.0f", fps);
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 14, str, GUIFont::Left);

                // Display the average
                sprintf(str, "MSPF: %i", m_MSPFAverage);
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 24, str, GUIFont::Left);

                sprintf(str, "Time Scale: x%.2f ([1]-, [2]+)", g_TimerMan.IsOneSimUpdatePerFrame() ? m_SimSpeed : g_TimerMan.GetTimeScale());
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 34, str, GUIFont::Left);

                sprintf(str, "Real to Sim Cap: %.2f ms ([3]-, [4]+)", g_TimerMan.GetRealToSimCap() * 1000.0f);
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 44, str, GUIFont::Left);

                float dt = g_TimerMan.GetDeltaTimeMS();
                sprintf(str, "DeltaTime: %.2f ms ([5]-, [6]+)", dt);
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 54, str, GUIFont::Left);

                sprintf(str, "Particles: %i", g_MovableMan.GetParticleCount());
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 64, str, GUIFont::Left);

				sprintf(str, "Objects: %i", g_MovableMan.GetKnownObjectsCount());
				GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 74, str, GUIFont::Left);

                sprintf(str, "MOIDs: %i", g_MovableMan.GetMOIDCount());
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 84, str, GUIFont::Left);

                sprintf(str, "Sim Updates Since Last Drawn: %i", g_TimerMan.SimUpdatesSinceDrawn());
                GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 94, str, GUIFont::Left);

                if (g_TimerMan.IsOneSimUpdatePerFrame())
                    GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 104, "ONE Sim Update Per Frame!", GUIFont::Left);

				sprintf(str, "Sound channels: %d / %d ", g_AudioMan.GetPlayingChannelCount(), g_AudioMan.GetTotalChannelCount());
				GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, 17, 114, str, GUIFont::Left);

				int xOffset = 17;
				int yOffset = 134;
				int blockHeight = 34;
				int graphHeight = 20;
				int graphOffset = 14;

				//Update current sample percentage
				g_FrameMan.CalculateSamplePercentages();

				//Draw advanced performance counters
				for(int pc = 0 ; pc < FrameMan::PERF_COUNT; ++pc)
				{
					int blockStart = yOffset + pc * blockHeight;

					GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, xOffset, blockStart , m_PerfCounterNames[pc], GUIFont::Left);

					// Print percentage from PerformanceCounters::PERF_SIM_TOTAL
					int perc = (int)((float)GetPerormanceCounterAverage(static_cast<PerformanceCounters>(pc)) / (float)GetPerormanceCounterAverage(PERF_SIM_TOTAL) * 100);
					sprintf(str, "%%: %i", perc);
		            GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, xOffset + 60, blockStart, str, GUIFont::Left);
					
					// Print average processing time in ms
					sprintf(str, "T: %i", GetPerormanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
		            GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, xOffset + 96, blockStart, str, GUIFont::Left);
					
					int graphStart = blockStart + graphOffset;

					//Draw graph
					//Draw graph backgrounds
					pPlayerGUIBitmap.DrawRectangle(xOffset, graphStart , MAXSAMPLES, graphHeight , 240, true);
					//pPlayerGUIBitmap.DrawLine(xOffset, graphStart, xOffset + MAXSAMPLES, graphStart, 48);
					pPlayerGUIBitmap.DrawLine(xOffset, graphStart + graphHeight / 2, xOffset + MAXSAMPLES, graphStart + graphHeight / 2, 96);
					//pPlayerGUIBitmap.DrawLine(xOffset, graphStart + graphHeight, xOffset + MAXSAMPLES, graphStart + graphHeight , 48);

					int smpl = m_Sample;

					//Custom graph for Total counter which shows update time
					// Not used because graphs now show data in ms, but can be enabled if you need percentages
					/*if (pc == PerformanceCounters::SIM_TOTAL)
					{
						m_PerfPercentages[pc][smpl] = (int)((float)m_PerfData[pc][smpl] / (1000000 / 30) * 100);
						if (m_PerfPercentages[pc][smpl] > 100)
							m_PerfPercentages[pc][smpl] = 100;
					}*/

					//Reset peak value
					int peak = 0;

					//Draw sample dots
					for (int i = 0; i < MAXSAMPLES; i++)
					{
						if (smpl < 0)
							smpl = MAXSAMPLES - 1;
						
						// Show percentages in graphs
						//int dotHeight = (int)((float)graphHeight / 100.0 * (float)m_PerfPercentages[pc][smpl]);

						// Show microseconds in graphs, assume that 33333 microseconds (one frame of 30 fps) is the highest value on the graph
						int value = (int)((float)m_PerfData[pc][smpl] / (1000000 / 30) * 100);
						if (value > 100)
							value = 100;
						// Calculate dot height on the graph
						int dotHeight = (int)((float)graphHeight / 100.0 * (float)value);
						pPlayerGUIBitmap.SetPixel(xOffset + MAXSAMPLES - i, graphStart + graphHeight - dotHeight, 13);

						if (peak < m_PerfData[pc][smpl])
							peak = m_PerfData[pc][smpl];

						//Move to previous sample
						smpl--;
					}

					// Print peak values
					sprintf(str, "Peak: %i", peak / 1000);
		            GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, xOffset + 130, blockStart, str, GUIFont::Left);
				}
            }

        }
        // If superflous screen (as in a three-player match), make the fourth the Observer one
        else
        {
//            yTextPos += 12;
            GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, GetPlayerScreenWidth() / 2, yTextPos, "- Observer View -", GUIFont::Centre);
        }

        ////////////////////////////////////////
        // If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer

        // The position of the current drawscreen on the final screen
        Vector screenOffset;

        if (screenCount > 1)
        {
            // Always upper left corner
            if (whichScreen == 0)
                screenOffset.SetIntXY(0, 0);
            else if (whichScreen == 1)
            {
                // If both splits, or just Vsplit, then in upper right quadrant
                if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit))
                    screenOffset.SetIntXY(GetResX() / 2, 0);
                // If only hsplit, then lower left quadrant
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
            // Make things go into slight slomo - DANGER
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
        if (m_PostProcessing && !IsInMultiplayerMode())
        {
            int occX = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntX();
            int occY = g_SceneMan.GetScreenOcclusion(whichScreen).GetFloorIntY();

			// Copy post effects received by client if in network mode
			if (m_DrawNetworkBackBuffer)
			{
				GetPostEffectsList(0, screenRelativeEffects);
			}

            // Adjust for the player screen's position on the final buffer
            for (list<PostEffect>::iterator eItr = screenRelativeEffects.begin(); eItr != screenRelativeEffects.end(); ++eItr)
            {
                // Make sure we won't be adding any effects to a part of the screen that is occluded by menus and such
                if ((*eItr).m_Pos.m_X > occX && (*eItr).m_Pos.m_Y > occY && (*eItr).m_Pos.m_X < pDrawScreen->w + occX && (*eItr).m_Pos.m_Y < pDrawScreen->h + occY)
                    m_PostScreenEffects.push_back( PostEffect((*eItr).m_Pos + screenOffset, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle) );
            }

            // Adjust glow areas for the player screen's position on the final buffer
            for (list<Box>::iterator bItr = screenRelativeGlowBoxes.begin(); bItr != screenRelativeGlowBoxes.end(); ++bItr)
            {
                m_PostScreenGlowBoxes.push_back(*bItr);
                // Adjust each added glow area for the player screen's position on the final buffer
                m_PostScreenGlowBoxes.back().m_Corner += screenOffset;
            }
        }
    }



// Done in MovableMan's Update now
//    g_SceneMan.ClearMOColorLayer();
    // Clears the pixels that have been revealed from the unseen layers
    g_SceneMan.ClearSeenPixels();

	if (!m_StoreNetworkBackBuffer)
	{
		// Draw demo timer countdown if not registered
		if (g_ActivityMan.GetActivity() && !g_LicenseMan.HasValidatedLicense())
		{
			int demoSecsLeft = g_ActivityMan.GetActivity()->GetDemoTimeLeft();
			// Blink if under 30s
			if (demoSecsLeft >= 0 && (demoSecsLeft > 30 || m_TextBlinkTimer.AlternateReal(333)))
			{
				AllegroBitmap pScreenGUIBitmap(m_pBackBuffer8);
				// Minutes or seconds
				if (demoSecsLeft > 60)
					sprintf(str, "Demo Time Left: %imin %is", demoSecsLeft / 60, (demoSecsLeft % 60));
				else
					sprintf(str, "Demo Time Left: %is", demoSecsLeft);
				GetLargeFont()->DrawAligned(&pScreenGUIBitmap, GetResX() / 2, GetResY() - 25, str, GUIFont::Centre);
			}
		}

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

		// Draw the console on top of teh 8 bpp buffer if post or 32bpp mode is active
	//    if (!FlippingWith32BPP())
	//        g_ConsoleMan.Draw(m_pBackBuffer32);

		// Replace 8 bit backbuffer contents with network received image before postprocessing as it is where this buffer is copied to 32 bit buffer
		if (m_DrawNetworkBackBuffer)
		{
			m_NetworkBitmapIsLocked[0] = true;
			blit(m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0], m_pBackBuffer8, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
			masked_blit(m_pNetworkBackBufferFinalGUI8[m_NetworkFrameReady][0], m_pBackBuffer8, 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);

			if (g_UInputMan.FlagAltState() || g_UInputMan.FlagCtrlState() || g_UInputMan.FlagShiftState())
			{
				AllegroBitmap allegroBitmap(m_pBackBuffer8);
				char buf[32];
				sprintf(buf, "PING: %u", m_CurrentPing);
				GetLargeFont()->DrawAligned(&allegroBitmap, m_pBackBuffer8->w - 25, m_pBackBuffer8->h - 14, buf, GUIFont::Right);
			}

			m_NetworkBitmapIsLocked[0] = false;
		}
	}

	//m_StoreNetworkBackBuffer = false;
	if (m_StoreNetworkBackBuffer)
	{
		// Blit all four internal player screens onto the backbuffer
		for (int i = 0; i < MAXSCREENCOUNT; i++)
		{
			int dx = 0;
			int dy = 0;
			int dw = m_pBackBuffer8->w / 2;
			int dh = m_pBackBuffer8->h / 2;

			if (i == 1)
			{
				dx = dw;
			} 
			else if (i == 2)
			{
				dy = dh;
			}
			else if (i == 3)
			{
				dx = dw;
				dy = dh;
			}

			//m_TargetPos[i] = g_SceneMan.GetOffset(i);

			m_NetworkBitmapIsLocked[i] = true;
			blit(m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][i], m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->h);
			blit(m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][i], m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i], 0, 0, 0, 0, m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->w, m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->h);
			m_NetworkBitmapIsLocked[i] = false;

			// Draw all player's screen into one
			if (g_UInputMan.KeyHeld(KEY_5))
				stretch_blit(m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][i], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][i]->h, dx, dy, dw, dh);
		}

		if (g_UInputMan.KeyHeld(KEY_1))
		{
			stretch_blit(m_pNetworkBackBufferFinal8[0][0], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][0]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}

		if (g_UInputMan.KeyHeld(KEY_2))
		{
			stretch_blit(m_pNetworkBackBufferFinal8[1][0], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][1]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][1]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}

		if (g_UInputMan.KeyHeld(KEY_3))
		{
			stretch_blit(m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2], m_pBackBuffer8, 0, 0, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2]->w, m_pNetworkBackBufferFinal8[m_NetworkFrameReady][2]->h, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
		}

		if (g_UInputMan.KeyHeld(KEY_4))
		{
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

		/*for (int i = 0; i < MAXSCREENCOUNT; i++)
		{
			m_NetworkBitmapIsLocked[i] = true;
			blit(m_pBackBuffer8, m_pNetworkBackBuffer8[i], 0, 0, 0, 0, m_pBackBuffer8->w, m_pBackBuffer8->h);
			m_NetworkBitmapIsLocked[i] = false;
		}*/
	}

    // Do postprocessing effects, if applicable and enabled
    if (m_PostProcessing && g_InActivity && m_BPP == 32)
        PostProcess();

    // Draw the console on top of everything
    if (FlippingWith32BPP())
        g_ConsoleMan.Draw(m_pBackBuffer32);

    release_bitmap(m_pBackBuffer8);

    // Reset the frame timer so we can measure how much it takes until next frame being drawn
    m_pFrameTimer->Reset();
}

void FrameMan::GetPostEffectsList(int whichScreen, list<PostEffect> & outputList)
{
	ScreenRelativeEffectsMutex[whichScreen].lock();
	outputList.clear();
	for (list<PostEffect>::iterator eItr = m_ScreenRelativeEffects[whichScreen].begin(); eItr != m_ScreenRelativeEffects[whichScreen].end(); ++eItr)
		outputList.push_back(PostEffect((*eItr).m_Pos, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));

	ScreenRelativeEffectsMutex[whichScreen].unlock();
}

void FrameMan::SetPostEffectsList(int whichScreen, list<PostEffect> & inputList)
{
	ScreenRelativeEffectsMutex[whichScreen].lock();
	m_ScreenRelativeEffects[whichScreen].clear();
	for (list<PostEffect>::iterator eItr = inputList.begin(); eItr != inputList.end(); ++eItr)
		m_ScreenRelativeEffects[whichScreen].push_back(PostEffect((*eItr).m_Pos, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));

	ScreenRelativeEffectsMutex[whichScreen].unlock();
}


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The width of the player screens.

int FrameMan::GetPlayerScreenWidth() const
{
	return GetPlayerFrameBufferWidth(-1);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The width of the player screens.

int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const
{ 
	if (m_StoreNetworkBackBuffer)
	{
		if (whichPlayer < 0 || whichPlayer >= MAXSCREENCOUNT)
		{
			int w = GetResX();
			for (int i = 0; i < MAXSCREENCOUNT; i++)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The height of the player screens.

int FrameMan::GetPlayerScreenHeight() const
{
	return GetPlayerFrameBufferHeight(-1);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The height of the player screens.

int FrameMan::GetPlayerFrameBufferHeight(int whichPlayer) const 
{ 
	if (m_StoreNetworkBackBuffer)
	{
		if (whichPlayer < 0 || whichPlayer >= MAXSCREENCOUNT)
		{
			int h = GetResY();
			for (int i = 0; i < MAXSCREENCOUNT; i++)
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


} // namespace RTE
