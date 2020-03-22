#ifndef _RTEFRAMEMAN_
#define _RTEFRAMEMAN_

#include "RTETools.h"
#include "Singleton.h"
#include "Serializable.h"
#include "ContentFile.h"
#include "Timer.h"
#include "Box.h"

#define g_FrameMan FrameMan::Instance()

namespace RTE {

	class AllegroScreen;
	class GUIFont;

enum TransperencyPreset { LessTrans = 0, HalfTrans, MoreTrans };

/// <summary>
/// The singleton manager over the composition and display of frames.
/// </summary>
class FrameMan : public Singleton<FrameMan>, public Serializable {

public:

#pragma region Creation
	/// <summary>
	/// Constructor method used to instantiate a FrameMan object in system memory. Create() should be called before using the object.
	/// </summary>
	FrameMan() { Clear(); }

	/// <summary>
	/// Makes the FrameMan object ready for use, which is to be used with SettingsMan first.
	/// </summary>
	/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	virtual int Create();
#pragma endregion

#pragma region Destruction
	/// <summary>
	/// Destructor method used to clean up a FrameMan object before deletion from system memory.
	/// </summary>
	virtual ~FrameMan() { Destroy(); }

	/// <summary>
	/// Destroys and resets (through Clear()) the FrameMan object.
	/// </summary>
	void Destroy();

	/// <summary>
	/// Resets the entire FrameMan, including its inherited members, to their default settings or values.
	/// </summary>
	virtual void Reset() { Clear(); }
#pragma endregion

#pragma region INI Handling
	/// <summary>
	/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
	/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
	/// </summary>
	/// <param name="propName">The name of the property to be read.</param>
	/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
	/// <returns>
	/// An error return value signaling whether the property was successfully read or not.
	/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
	/// </returns>
	virtual int ReadProperty(std::string propName, Reader &reader);

	/// <summary>
	/// Saves the complete state of this FrameMan to an output stream for later recreation with Create(Reader &reader).
	/// </summary>
	/// <param name="writer">A Writer that the FrameMan will save itself with.</param>
	/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	virtual int Save(Writer &writer) const;
#pragma endregion


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text height
// Arguments:       Text, max width, whether to use small or large font
// Return value:    None.

	int CalculateTextHeight(std::string text, int maxWidth, bool isSmall);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateTextWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns max text width
// Arguments:       Text, whether to use small or large font
// Return value:    None.

	int CalculateTextWidth(std::string text, bool isSmall);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetSplitScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets new values for the split screen configuration.
// Arguments:       Whether the new setting should be horizontally split (over and under).
//                  Whether the new setting should be vertically split (side by side)
// Return value:    None.

    void ResetSplitScreens(bool hSplit = false, bool vSplit = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPM
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between on-screen pixels and the physics engine's
//                  meters.
// Arguments:       None.
// Return value:    A float describing the current PPM ratio.

    float GetPPM() const { return m_PPM; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the physics engine's meters and on-screen
//                  pixels.
// Arguments:       None.
// Return value:    A float describing the current MPP ratio.

    float GetMPP() const { return m_MPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPPL
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the on-screen pixels and the physics engine's
//                  Litres.
// Arguments:       None.
// Return value:    A float describing the current PPL ratio.

    float GetPPL() const { return m_PPL; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio between the physics engine's Litres and on-screen
//                  pixels.
// Arguments:       None.
// Return value:    A float describing the current LPP ratio.

    float GetLPP() const { return m_LPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the horizontal resolution of the screen.
// Arguments:       None.
// Return value:    An int describing the horizontal resolution of the screen in pixels.

    int GetResX() const { return m_ResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the vertical resolution of the screen.
// Arguments:       None.
// Return value:    An int describing the vertical resolution of the screen in pixels.

    int GetResY() const { return m_ResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the horizontal resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       None.
// Return value:    An int describing the horizontal resolution of the new screen in pixels.

    int GetNewResX() const { return m_NewResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the vertical resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       None.
// Return value:    An int describing the vertical resolution of the new screen in pixels.

    int GetNewResY() const { return m_NewResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewResX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the horizontal resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       An int describing the horizontal resolution of the new screen in pixels.
// Return value:    None.

    void SetNewResX(int newResX) { m_NewResX = newResX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewResY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the vertical resolution of the screen that will be used next
//                  time this FrameMan is Created.
// Arguments:       An int describing the vertical resolution of the new screen in pixels.
// Return value:    None.

    void SetNewResY(int newResY) { m_NewResY = newResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsNewResSet
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whetehr a new resolution has been set for the next time
//                  this FrameMan is created.
// Arguments:       None.
// Return value:    Whether the new resolution set differs from the current one.

    bool IsNewResSet() const { return m_NewResX != m_ResX || m_NewResY != m_ResY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bits per pixel color depth
// Arguments:       None.
// Return value:    An int describing the number of bits per pixel of the current color depth.

    int GetBPP() const { return m_BPP; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPaletteFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ContentFile describing the locaiton of the color palette.
// Arguments:       None.
// Return value:    An reference to a ContentFile which described the palette location.

    const ContentFile & GetPaletteFile() const { return m_PaletteFile; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 8bpp back buffer bitmap
// Arguments:       None.
// Return value:    A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

    BITMAP * GetBackBuffer8() const { return m_pBackBuffer8; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNetworkClinteBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 8bpp back buffer bitmap used to draw network transmitted image on top of everything
// Arguments:       None.
// Return value:    A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

	BITMAP * GetNetworkBackBufferIntermediateGUI8Ready(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBufferGUI8Ready(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBufferIntermediate8Ready(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameReady][player]; }

	BITMAP * GetNetworkBackBuffer8Ready(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameReady][player]; }


	BITMAP * GetNetworkBackBufferIntermediateGUI8Current(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBufferGUI8Current(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBufferIntermediate8Current(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][player]; }

	BITMAP * GetNetworkBackBuffer8Current(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][player]; }




	bool IsNetworkBitmapLocked(int player) const { return m_NetworkBitmapIsLocked[player]; }

	bool GetDrawNetworkBackBuffer() { return m_DrawNetworkBackBuffer; }

	void SetDrawNetworkBackBuffer(bool value) { m_DrawNetworkBackBuffer = value; }

	bool GetStoreNetworkBackBuffer() { return m_StoreNetworkBackBuffer; }

	void SetStoreNetworkBackBuffer(bool value) { m_StoreNetworkBackBuffer = value; }

	void CreateNewPlayerBackBuffer(int player, int w, int h);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBackBuffer32
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the 32bpp back buffer bitmap, if available. If not, the 8bpp is
//                  returned. Make sure you don't do any blending stuff to the 8bpp one!
// Arguments:       None.
// Return value:    A pointer to the BITMAP 32bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!

    BITMAP * GetBackBuffer32() const { return m_pBackBuffer32 ? m_pBackBuffer32 : m_pBackBuffer8; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of currently active screens, counting all splits.
// Arguments:       None.
// Return value:    The number of currently active screens.

    int GetScreenCount() const { return m_HSplit || m_VSplit ? (m_HSplit && m_VSplit ? 4 : 2) : 1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether the screen is split horizontally across the screen, ie
//                  as two splitscreens one above the other.
// Arguments:       None.
// Return value:    Whether or not screen has a horizontal split.

    bool GetHSplit() const { return m_HSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetVSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether the screen is split vertically across the screen, ie
//                  as two splitscreens side by side.
// Arguments:       None.
// Return value:    Whether screen has a vertical split.

    bool GetVSplit() const { return m_VSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The width of the player screens.

	int GetPlayerScreenWidth() const { return GetPlayerFrameBufferWidth(-1); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The width of the player screens.

	int GetPlayerFrameBufferWidth(int whichPlayer) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       None.
// Return value:    The height of the player screens.

	int GetPlayerScreenHeight() const { return GetPlayerFrameBufferHeight(-1); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerScreenHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the individual player screens. This will only be less
//                  than the backbuffer resolution if there are split screens.
// Arguments:       Player to get screen width for, only used by multiplayer parts.
// Return value:    The height of the player screens.

	int GetPlayerFrameBufferHeight(int whichPlayer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBlackIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a font from the GUI engine's current skin. Ownership is NOT
//                  transferred!
// Arguments:       The font filename in the currently used skin, NOT including the skin
//                  path.
// Return value:    A pointer to the requested font, or 0 if no font with that filename
//                  is in the current skin was found. Ownership is NOT transferred!

    unsigned char GetBlackIndex() const { return m_BlackColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLargeFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the requested font, or 0 if no large font was found.
//                  Ownership is NOT transferred!

    GUIFont * GetLargeFont();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSmallFont
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the large font from the GUI engine's current skin. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the requested font, or 0 if no large font was found.
//                  Ownership is NOT transferred!

    GUIFont * GetSmallFont();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether the screen is split horizontally across the screen, ie
//                  as two splitscreens one above the other.
// Arguments:       Whether or not to have a horizontal split.
// Return value:    None.

    void SetHSplit(bool hSplit) { m_HSplit = hSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetVSplit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether the screen is split vertically across the screen, ie
//                  as two splitscreens side by side.
// Arguments:       Whether or not to have a vertical split.
// Return value:    None.

    void SetVSplit(bool vSplit) { m_VSplit = vSplit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPPM
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ratio between on-screen pixels and the physics engine's
//                  meters.
// Arguments:       A float specifying the new PPM ratio.
// Return value:    None.

    void SetPPM(const float newPPM) { m_PPM = newPPM; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the message to be displayed on top of each player's screen
// Arguments:       An std::string that specifies what should be displayed.
//                  Which screen you want to set text to.
//                  The interval with which the screen will be blinking, in ms. 0 means no
//                  blinking.
//                  The duration, in MS to force this message to display. No other message
//                  can be displayed before this expires. ClearScreenText overrides it though.
//                  Vertically centered on the screen
// Return value:    None.

    void SetScreenText(const std::string &msg, int which = 0, int blinkInterval = 0, int displayDuration = -1, bool centered = false);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the message to be displayed on top of each player's screen
// Arguments:       None.
// Return value:    Current message shown to player.

	std::string GetScreenText(int which = 0) const { return (which >= 0 && which < c_MaxScreenCount) ? m_ScreenText[which] : ""; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearScreenText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the message to be displayed on top of each player's screen
// Arguments:       Which screen you want to set text to.
// Return value:    None.

    void ClearScreenText(int which = 0) { if (which >= 0 && which < c_MaxScreenCount) { m_ScreenText[which].clear(); m_TextDuration[which] = -1; m_TextDurationTimer[which].Reset(); m_TextBlinking[which] = 0; } }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether we're in fullscreen mode or not.
// Arguments:       None.
// Return value:    Whether we're in fullscreen mode.

    bool IsFullscreen() const { return m_Fullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NxWindowed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how many times the windowed screen resolution is being multiplied
//                  and the back buffer stretched across for better readibility.
// Arguments:       None.
// Return value:    What multiple the windowed mode screen resolution is run in. (1 normal)

    int NxWindowed() const { return m_NxWindowed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how many times the fullscreen resolution is being multiplied
//                  and the back buffer stretched across for better readibility.
// Arguments:       None.
// Return value:    What multiple the fullscreen mode screen resolution is run in. (1 normal)

    int NxFullscreen() const { return m_NxFullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNewNxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how many times the fullscreen resolution will be multiplied ON
//                  NEXT RESTART and the back buffer stretched.
// Arguments:       What multiple the fullscreen mode screen resolution will be run in.
//                  on next restart of game.
// Return value:    None.

    void SetNewNxFullscreen(int newMultiple) { m_NewNxFullscreen = newMultiple; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewNxFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many times the fullscreen resolution will be multiplied ON
//                  NEXT RESTART and the back buffer stretched.
// Arguments:       None.
// Return value:    What multiple the fullscreen mode screen resolution will be run in.
//                  on next restart of game.

    int GetNewNxFullscreen() const { return m_NewNxFullscreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a palette from a .dat file and sets it as the currently used
//                  screen palette.
// Arguments:       String with the data path to the palette data object within a .dat.
// Return value:    Whether palette laoded successfully or not.

    bool LoadPalette(std::string palettePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeInPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette in from black at a specified speed.
// Arguments:       Speed specififed from (slowest) 1 - 64 (fastest)
// Return value:    None.

    void FadeInPalette(int fadeSpeed = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FadeOutPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fades the palette out to black at a specified speed.
// Arguments:       Speed specififed from (slowest) 1 - 64 (fastest)
// Return value:    None.

    void FadeOutPalette(int fadeSpeed = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTransTable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific precalculated transperency table which is used for
//                  any subsequent transparency drawing.
// Arguments:       The transperency setting, see the enum.
// Return value:    None.

    void SetTransTable(TransperencyPreset transSetting);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveScreenToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap of the screen back buffer to a 8bpp BMP file.
// Arguments:       The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveScreenToBMP(const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveWorldToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap of everything on the scene to the BMP file.
// Arguments:       The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveWorldToBMP(const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveBitmapToBMP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Dumps a bitmap to a 8bpp BMP file.
// Arguments:       A pointer to a BITMAP to save.
//                  The filename of the file to save to, WITHOUT EXTENSION. eg, If "Test"
//                  is passed in, this func will save to Test000.bmp, if that file does
//                  not already exist. If it does exist, it will attempt 001, and so on.
// Return value:    Success >=0, or failure < 0.

    int SaveBitmapToBMP(BITMAP *pBitmap, const char *namebase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleFullscreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Toggles to and from fullscreen and windowed mode.
// Arguments:       None.
// Return value:    Error code, anything other than 0 is error..

    int ToggleFullscreen();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchWindowMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets and switches to a new windowed mode multiplier.
// Arguments:       The multiplier to switch to.
// Return value:    Error code, anything other than 0 is error..

    int SwitchWindowMultiplier(int multiplier = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearBackBuffer8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the 8bpp backbuffer with black.
// Arguments:       None.
// Return value:    None.

    void ClearBackBuffer8() { clear_to_color(m_pBackBuffer8, m_BlackColor); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearBackBuffer32
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the 32bpp backbuffer with black.
// Arguments:       None.
// Return value:    None.

    void ClearBackBuffer32() { if (m_pBackBuffer32) clear_to_color(m_pBackBuffer32, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlipFrameBuffers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flips the framebuffer.
// Arguments:       None.
// Return value:    None.

    void FlipFrameBuffers();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlippingWith32BPP
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the 32bpp framebuffer is currnetly being used or not
//                  when flipping the frame buffers.
// Arguments:       None.
// Return value:    Whether the 32bpp is used or not (8bpp one is).

	bool FlippingWith32BPP() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlashScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flashes any of the players' screen red for this frame.
// Arguments:       Which screen to flash.
//                  What color to flash it. -1 means no color or flash.
//                  How long a period to fill the frame with color. If 0, a single-frame
//                  flash will happen.
// Return value:    None.

    void FlashScreen(int screen, int color, float periodMS = 0) { m_FlashScreenColor[screen] = color; m_FlashTimer[screen].SetRealTimeLimitMS(periodMS); m_FlashTimer[screen].Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a text string to the bitmap of choice, using the internal
//                  fontsets.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The string containing the text to draw.
//                  the position the upper right corner of the first char of the text
//                  string will have on the bitmap.
//                  Whether to draw text with black chars or not (white).
// Return value:    None.

    void DrawText(BITMAP *pTargetBitmap,
                  const std::string &str,
                  const Vector &pos,
                  bool black = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted or with other effects.
// Arguments:       The Bitmap to draw to. Ownership is NOT transferred.
//                  The absolute Start point.
//                  The absolute end point.
//                  The color value of the line.
//                  A color to alternate with every other pixel drawn will have this if !0.
//                  How many pixels to skip drawing between drawn ones. 0 means solid line
//                  2 means there's a gap of two pixels between each drawn one.
//                  The start of the skipping phase. If skip is 10 and this is 5, the first
//                  dot will will be drawn after 5 pixels.
//                  Whether the line should take the shortest possible route across scene
//                  wraps.
// Return value:    The end state of the skipping phase. Eg if 4 is returned here the last
//                  dot was placed 4 pixels ago.

    int DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawDotLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line that can be dotted with bitmaps.
// Arguments:       The Bitmap to draw to. Ownership is NOT transferred.
//                  The absolute Start point.
//                  The absolute end point.
//                  The bitmap to be used for dots; will be centered.
//                  How many pixels to gap between drawing dots. Should be more than 0
//                  The start of the skipping phase. If skip is 10 and this is 5, the first
//                  dot will will be drawn after 5 pixels.
//                  Whether the line should take the shortest possible route across scene
//                  wraps.
// Return value:    The end state of the skipping phase. Eg if 4 is returned here the last
//                  dot was placed 4 pixels ago.

    int DrawDotLine(BITMAP *pBitmap, const Vector &start, const Vector &end, BITMAP *pDot, int skip = 0, int skipStart = 0, bool shortestWrap = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this FrameMan. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the current frame to the screen.
// Arguments:       None.
// Return value:    None.

    void Draw();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsValidResolution	
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if this resolution is supported
// Arguments:       Resolution width and height
// Return value:    True if resolution is supported
	bool IsValidResolution(int width, int height) const;

	Vector GetTargetPos(int screen) { return m_TargetPos[m_NetworkFrameReady][screen]; }

	Vector SLOffset[c_MaxScreenCount][c_MaxLayersStoredForNetwork];

	bool IsInMultiplayerMode() { return m_StoreNetworkBackBuffer; }


#pragma region Class Info
	/// <summary>
	/// Gets the class name of this Entity.
	/// </summary>
	/// <returns>A string with the friendly-formatted type name of this object.</returns>
	virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

protected:

    // Member variables
    static const std::string c_ClassName; //!< The friendly-formatted type name of this object.

    // Resolution
    int m_ResX;
    int m_ResY;
    // Color depth (bits per pixel)
    int m_BPP;

    // These are the new resolution settings that will take effect next time the FrameMan is started
    int m_NewResX;
    int m_NewResY;

    // Screen back buffer, always 8bpp, gets copied to the 32bpp buffer if post processing is used
    BITMAP *m_pBackBuffer8;
    // 32Bits per pixel back buffer, only used if player elects, and only if in 32bpp video mode
    BITMAP *m_pBackBuffer32;

	// Per-player allocated frame buffer to draw upon during frameman draw
	BITMAP *m_pNetworkBackBufferIntermediate8[2][c_MaxScreenCount];

	// Per-player allocated frame buffer to draw upon during frameman draw used to draw UI only
	BITMAP *m_pNetworkBackBufferIntermediateGUI8[2][c_MaxScreenCount];


	// Per-player allocated frame buffer to copy Intermediate before sending
	BITMAP *m_pNetworkBackBufferFinal8[2][c_MaxScreenCount];

	// Per-player allocated frame buffer to copy Intermediate before sending used to draw UI only
	BITMAP *m_pNetworkBackBufferFinalGUI8[2][c_MaxScreenCount];


	// If true, draws the contents of the m_pNetworkBackBuffer8 on top of m_pBackBuffer8 every frame in FrameMan.Draw
	bool m_DrawNetworkBackBuffer;

	// If true, dumps the contents of the m_pBackBuffer8 to the m_pNetworkBackBuffer8 every frame
	bool m_StoreNetworkBackBuffer;

    // Temporary buffer for making quick screencaps
    BITMAP *m_pScreendumpBuffer;

    // Data file of the screen palette
    ContentFile m_PaletteFile;
    DATAFILE *m_pPaletteDataFile;

    // Transparency color tables
    COLOR_MAP m_LessTransTable;
    COLOR_MAP m_HalfTransTable;
    COLOR_MAP m_MoreTransTable;

    // Color black index
    unsigned char m_BlackColor;
    unsigned char m_AlmostBlackColor;

    // Whether in fullscreen mode or not
    bool m_Fullscreen;
    // The number of times the windowed mode resoltion should be multiplied and streched across for better visibility
    int m_NxWindowed;
    // The number of times the fullscreen mode resolution should be multiplied and streched across for better visibility
    // The internal virtual resolution (m_ResX, m_ResY) is 1/n of the acutal fullscreen res that the graphics card outputs
    int m_NxFullscreen;
    // This is the new fullscreen multiple that will take effect next time the FrameMan is started
    int m_NewNxFullscreen;

    // Whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.
    bool m_HSplit;
    // Whether the screen is split vertically across the screen, ie as two splitscreens side by side.
    bool m_VSplit;
    // Whether the screen is set to split horizontally in settings
    bool m_HSplitOverride;
    // Whether the screen is set to split vertically in settings
    bool m_VSplitOverride;
    // Intermediary split screen bitmap.
    BITMAP *m_pPlayerScreen;
    // Dimensions of each of the screens of each player. Will be smaller than resolution only if the screen is split
    int m_PlayerScreenWidth;
    int m_PlayerScreenHeight;

    // Pixels Per Meter constant
    float m_PPM;
    // Meters Per Pixel constant.
    float m_MPP;
    // Pixels per Litre contstant
    float m_PPL;
    // Litres Per Pixel contstant
    float m_LPP;

    // The GUI control managers for all teams
//    std::vector<GUIControlManager *> m_BuyGUIs;

    // GUI screen object kept and owned just for the fonts
    AllegroScreen *m_pGUIScreen;
    // Standard fonts for quick access.
    GUIFont *m_pLargeFont;
    GUIFont *m_pSmallFont;
    // The text to be displayed on each player's screen
    std::string m_ScreenText[c_MaxScreenCount];
    // The minimum duration the current message is supposed to show vefore it can be overwritten
    int m_TextDuration[c_MaxScreenCount];
    // Screen text display duration time
    Timer m_TextDurationTimer[c_MaxScreenCount];
    // Screen text messages blinking interval in ms. 0 is no blink at all, just show message.
    int m_TextBlinking[c_MaxScreenCount];
    // Whether screen text is centered vertically
    bool m_TextCentered[c_MaxScreenCount];
    // Screen text blink timer
    Timer m_TextBlinkTimer;
    // Whether to flash a player's screen a specific color this frame. -1 means no flash
    int m_FlashScreenColor[c_MaxScreenCount];
    // Whether we flashed last frame or not
    bool m_FlashedLastFrame[c_MaxScreenCount];
    // Flash screen timer
    Timer m_FlashTimer[c_MaxScreenCount];
	// Frame target pos for network players
	Vector m_TargetPos[2][c_MaxScreenCount];
	// Which frame index is being rendered, 0 or 1
	int m_NetworkFrameCurrent;
	// Which frame is rendered and ready for transmission, 0 or 1
	int m_NetworkFrameReady;

	// If true then the network bitmap is being updated
	bool m_NetworkBitmapIsLocked[c_MaxScreenCount];
	//std::mutex m_NetworkBitmapIsLocked[c_MaxScreenCount];

private:

	/// <summary>
	/// Clears all the member variables of this FrameMan, effectively resetting the members of this abstraction level only.
	/// </summary>
    void Clear();

    // Disallow the use of some implicit methods.
	FrameMan(const FrameMan &reference) {}
	FrameMan & operator=(const FrameMan &rhs) {}
};
}
#endif