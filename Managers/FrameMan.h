#ifndef _RTEFRAMEMAN_
#define _RTEFRAMEMAN_

#include "RTETools.h"
#include "ContentFile.h"
#include "Timer.h"
#include "Box.h"

#define g_FrameMan FrameMan::Instance()

namespace RTE {

	class AllegroScreen;
	class AllegroBitmap;
	class GUIFont;

	/// <summary>
	/// The singleton manager over the composition and display of frames.
	/// </summary>
	class FrameMan : public Singleton<FrameMan>, public Serializable {

	public:

		Vector SLOffset[c_MaxScreenCount][c_MaxLayersStoredForNetwork]; //!< SceneLayer offsets for each screen in online multiplayer.

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

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this FrameMan. Supposed to be done every frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the current frame to the screen.
		/// </summary>
		void Draw();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the 8bpp back-buffer bitmap.
		/// </summary>
		/// <returns>A pointer to the BITMAP 8bpp back-buffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer8() const { return m_BackBuffer8; }

		/// <summary>
		/// Gets the 32bpp back-buffer bitmap. Make sure you don't do any blending stuff to the 8bpp one!
		/// </summary>
		/// <returns>A pointer to the BITMAP 32bpp back-buffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer32() const { return m_BackBuffer32; }

		/// <summary>
		/// Gets the ratio between the physics engine's meters and on-screen pixels.
		/// </summary>
		/// <returns>A float describing the current MPP ratio.</returns>
		float GetMPP() const { return m_MPP; }

		/// <summary>
		/// Gets the ratio between on-screen pixels and the physics engine's meters.
		/// </summary>
		/// <returns>A float describing the current PPM ratio.</returns>
		float GetPPM() const { return m_PPM; }

		/// <summary>
		/// Sets the ratio between on-screen pixels and the physics engine's meters.
		/// </summary>
		/// <param name="newPPM">A float specifying the new PPM ratio.</param>
		void SetPPM(const float newPPM) { m_PPM = newPPM; }

		/// <summary>
		/// Gets the ratio between the physics engine's Liters and on-screen pixels.
		/// </summary>
		/// <returns>A float describing the current LPP ratio.</returns>
		float GetLPP() const { return m_LPP; }

		/// <summary>
		/// Gets the ratio between the on-screen pixels and the physics engine's Liters.
		/// </summary>
		/// <returns>A float describing the current PPL ratio.</returns>
		float GetPPL() const { return m_PPL; }
#pragma endregion

#pragma region Resolution Handling
		/// <summary>
		/// Gets the horizontal resolution of the screen.
		/// </summary>
		/// <returns>An int describing the horizontal resolution of the screen in pixels.</returns>
		int GetResX() const { return m_ResX; }

		/// <summary>
		/// Gets the vertical resolution of the screen.
		/// </summary>
		/// <returns>An int describing the vertical resolution of the screen in pixels.</returns>
		int GetResY() const { return m_ResY; }

		/// <summary>
		/// Gets the horizontal resolution of the screen that will be used next time this FrameMan is Created.
		/// </summary>
		/// <returns>An int describing the horizontal resolution of the new screen in pixels.</returns>
		int GetNewResX() const { return m_NewResX; }

		/// <summary>
		/// Sets the horizontal resolution of the screen that will be used next time this FrameMan is Created.
		/// </summary>
		/// <param name="newResX">An int describing the horizontal resolution of the new screen in pixels.</param>
		void SetNewResX(int newResX) { m_NewResX = newResX; }

		/// <summary>
		/// Gets the vertical resolution of the screen that will be used next time this FrameMan is Created.
		/// </summary>
		/// <returns>An int describing the vertical resolution of the new screen in pixels.</returns>
		int GetNewResY() const { return m_NewResY; }

		/// <summary>
		/// Sets the vertical resolution of the screen that will be used next time this FrameMan is Created.
		/// </summary>
		/// <param name="newResY">An int describing the vertical resolution of the new screen in pixels.</param>
		void SetNewResY(int newResY) { m_NewResY = newResY; }

		/// <summary>
		/// Indicates whether a new resolution has been set for the next time this FrameMan is created.
		/// </summary>
		/// <returns>Whether the new resolution set differs from the current one.</returns>
		bool IsNewResSet() const { return m_NewResX != m_ResX || m_NewResY != m_ResY; }

		/// <summary>
		/// Returns true if this resolution is supported.
		/// </summary>
		/// <param name="width">Resolution width.</param>
		/// <param name="height">Resolution height.</param>
		/// <returns>True if resolution is supported.</returns>
		bool IsValidResolution(int width, int height) const;

		/// <summary>
		/// Tells how many times the screen resolution is being multiplied and the back-buffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the screen resolution is run in (1 normal).</returns>
		int ResolutionMultiplier() const { return m_ResMultiplier; }

		/// <summary>
		/// Sets and switches to a new windowed mode resolution multiplier.
		/// </summary>
		/// <param name="multiplier">The multiplier to switch to.</param>
		/// <returns>Error code, anything other than 0 is error.</returns>
		int SwitchResolutionMultiplier(unsigned char multiplier = 1);
#pragma endregion

#pragma region Split-Screen Handling
		/// <summary>
		/// Gets whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.
		/// </summary>
		/// <returns>Whether or not screen has a horizontal split.</returns>
		bool GetHSplit() const { return m_HSplit; }

		/// <summary>
		/// Sets whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.
		/// </summary>
		/// <param name="hSplit">Whether or not to have a horizontal split.</param>
		void SetHSplit(bool hSplit) { m_HSplit = hSplit; }

		/// <summary>
		/// Gets whether the screen is split vertically across the screen, ie as two splitscreens side by side.
		/// </summary>
		/// <returns>Whether screen has a vertical split.</returns>
		bool GetVSplit() const { return m_VSplit; }

		/// <summary>
		/// Sets whether the screen is split vertically across the screen, ie as two splitscreens side by side.
		/// </summary>
		/// <param name="vSplit">Whether or not to have a vertical split.</param>
		void SetVSplit(bool vSplit) { m_VSplit = vSplit; }

		/// <summary>
		/// Sets new values for the split screen configuration.
		/// </summary>
		/// <param name="hSplit">Whether the new setting should be horizontally split (over and under).</param>
		/// <param name="vSplit">Whether the new setting should be vertically split (side by side).</param>
		void ResetSplitScreens(bool hSplit = false, bool vSplit = false);

		/// <summary>
		/// Gets the number of currently active screens, counting all splits.
		/// </summary>
		/// <returns>The number of currently active screens.</returns>
		int GetScreenCount() const { return m_HSplit || m_VSplit ? (m_HSplit && m_VSplit ? 4 : 2) : 1; }

		/// <summary>
		/// Gets the width of the individual player screens. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <returns>The width of the player screens.</returns>
		int GetPlayerScreenWidth() const { return GetPlayerFrameBufferWidth(-1); }

		/// <summary>
		/// Gets the height of the individual player screens. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <returns>The height of the player screens.</returns>
		int GetPlayerScreenHeight() const { return GetPlayerFrameBufferHeight(-1); }

		/// <summary>
		/// Gets the width of the specified player screen. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <param name="whichPlayer">Player to get screen width for, only used by multiplayer parts.</param>
		/// <returns>The width of the specified player screen.</returns>
		int GetPlayerFrameBufferWidth(int whichPlayer) const;

		/// <summary>
		/// Gets the height of the specified player screen. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <param name="whichPlayer">Player to get screen width for, only used by multiplayer parts.</param>
		/// <returns>The height of the specified player screen.</returns>
		int GetPlayerFrameBufferHeight(int whichPlayer) const;
#pragma endregion

#pragma region Text Handling
		/// <summary>
		/// Gets the small font from the GUI engine's current skin. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the requested font, or 0 if no small font was found.</returns>
		GUIFont * GetSmallFont() { return GetFont(true); }

		/// <summary>
		/// Gets the large font from the GUI engine's current skin. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the requested font, or 0 if no large font was found.</returns>
		GUIFont * GetLargeFont() { return GetFont(false); }

		/// <summary>
		/// Calculates the width of a text string using the given font size.
		/// </summary>
		/// <param name="text">Text string.</param>
		/// <param name="isSmall">Whether to use small or large font.</param>
		/// <returns>Width of the text string.</returns>
		int CalculateTextWidth(std::string text, bool isSmall);

		/// <summary>
		/// Calculates the height of a text string using the given font size.
		/// </summary>
		/// <param name="text">Text string.</param>
		/// <param name="maxWidth">Maximum width of the text string.</param>
		/// <param name="isSmall">Whether to use small or large font.</param>
		/// <returns>Height of the text string.</returns>
		int CalculateTextHeight(std::string text, int maxWidth, bool isSmall);

		/// <summary>
		/// Gets the message to be displayed on top of each player's screen.
		/// </summary>
		/// <param name="whichScreen">Which player screen to get message from.</param>
		/// <returns>Current message shown to player.</returns>
		std::string GetScreenText(int whichScreen = 0) const { return (whichScreen >= 0 && whichScreen < c_MaxScreenCount) ? m_ScreenText[whichScreen] : ""; }

		/// <summary>
		/// Sets the message to be displayed on top of each player's screen
		/// </summary>
		/// <param name="message">An std::string that specifies what should be displayed.</param>
		/// <param name="whichScreen">Which screen you want to set text to.</param>
		/// <param name="blinkInterval">The interval with which the screen will be blinking, in ms. 0 means no blinking.</param>
		/// <param name="displayDuration">The duration, in MS to force this message to display. No other message can be displayed before this expires. ClearScreenText overrides it though.</param>
		/// <param name="centered">Vertically centered on the screen.</param>
		void SetScreenText(const std::string &message, int whichScreen = 0, int blinkInterval = 0, int displayDuration = -1, bool centered = false);

		/// <summary>
		/// Clears the message to be displayed on top of each player's screen.
		/// </summary>
		/// <param name="whichScreen">Which screen message to clear.</param>
		void ClearScreenText(int whichScreen = 0);
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// Flips the frame buffers, showing the back-buffer on the current display.
		/// </summary>
		void FlipFrameBuffers();

		/// <summary>
		/// Clears the 8bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer8() { clear_to_color(m_BackBuffer8, m_BlackColor); }

		/// <summary>
		/// Clears the 32bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer32() { clear_to_color(m_BackBuffer32, 0); }

		/// <summary>
		/// Sets a specific recalculated transparency table which is used for any subsequent transparency drawing.
		/// </summary>
		/// <param name="transSetting">The transparency setting, see the enumerator.</param>
		void SetTransTable(TransparencyPreset transSetting);

		/// <summary>
		/// Flashes any of the players' screen with the specified color for this frame.
		/// </summary>
		/// <param name="screen">Which screen to flash.</param>
		/// <param name="color">What color to flash it. -1 means no color or flash.</param>
		/// <param name="periodMS">How long a period to fill the frame with color. If 0, a single-frame flash will happen.</param>
		void FlashScreen(int screen, int color, float periodMS = 0) { m_FlashScreenColor[screen] = color; m_FlashTimer[screen].SetRealTimeLimitMS(periodMS); m_FlashTimer[screen].Reset(); }

		/// <summary>
		/// Draws a line that can be dotted or with other effects.
		/// </summary>
		/// <param name="pBitmap">The Bitmap to draw to. Ownership is NOT transferred.</param>
		/// <param name="start">The absolute Start point.</param>
		/// <param name="end">The absolute end point.</param>
		/// <param name="color">The color value of the line.</param>
		/// <param name="altColor">A color to alternate with. Every other pixel drawn will have this if !0.</param>
		/// <param name="skip">How many pixels to skip drawing between drawn ones. 0 means solid line 2 means there's a gap of two pixels between each drawn one.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false);

		/// <summary>
		/// Draws a line that can be dotted with bitmaps.
		/// </summary>
		/// <param name="bitmap">The Bitmap to draw to. Ownership is NOT transferred.</param>
		/// <param name="start">The absolute Start point.</param>
		/// <param name="end">The absolute end point.</param>
		/// <param name="dot">The bitmap to be used for dots (will be centered).</param>
		/// <param name="skip">How many pixels to gap between drawing dots. Should be more than 0.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int DrawDotLine(BITMAP *bitmap, const Vector &start, const Vector &end, BITMAP *dot, int skip = 0, int skipStart = 0, bool shortestWrap = false);
#pragma endregion

#pragma region Network Handling
		/// <summary>
		/// Returns true if this manager is in multiplayer mode, storing the 8bpp back-buffer for network transmission.
		/// </summary>
		/// <returns>True if in multiplayer mode.</returns>
		bool IsInMultiplayerMode() const { return m_StoreNetworkBackBuffer; }

		/// <summary>
		/// Sets the multiplayer mode flag, telling the manager to store the 8bpp back-buffer for network transmission.
		/// </summary>
		/// <param name="value">Whether this manager should operate in multiplayer mode.</param>
		void SetMultiplayerMode(bool value) { m_StoreNetworkBackBuffer = value; }

		/// <summary>
		/// Gets the 8bpp back-buffer bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get back-buffer bitmap from.</param>
		/// <returns>A pointer to the 8bpp back-buffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBuffer8Ready(int player) const { return m_NetworkBackBufferFinal8[m_NetworkFrameReady][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player">Which player screen to get GUI back-buffer bitmap from.</param>
		/// <returns>A pointer to the 8bpp GUI back-buffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferGUI8Ready(int player) const { return m_NetworkBackBufferFinalGUI8[m_NetworkFrameReady][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		BITMAP * GetNetworkBackBuffer8Current(int player) const { return m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		BITMAP * GetNetworkBackBufferGUI8Current(int player) const { return m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// Gets the 8bpp intermediate bitmap used to send network transmitted image. 
		/// </summary>
		/// <param name="player">Which player screen to get intermediate bitmap from.</param>
		/// <returns>A pointer to the 8bpp intermediate BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediate8Ready(int player) const { return m_NetworkBackBufferIntermediate8[m_NetworkFrameReady][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player">Which player screen to get intermediate GUI bitmap from.</param>
		/// <returns>A pointer to the 8bpp intermediate GUI BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediate8Current(int player) const { return m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		BITMAP * GetNetworkBackBufferIntermediateGUI8Ready(int player) const { return m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameReady][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		BITMAP * GetNetworkBackBufferIntermediateGUI8Current(int player) const { return m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		bool IsNetworkBitmapLocked(int player) const { return m_NetworkBitmapIsLocked[player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="screen"></param>
		/// <returns></returns>
		Vector GetTargetPos(int screen) { return m_TargetPos[m_NetworkFrameReady][screen]; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetDrawNetworkBackBuffer() { return m_DrawNetworkBackBuffer; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="value"></param>
		void SetDrawNetworkBackBuffer(bool value) { m_DrawNetworkBackBuffer = value; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetStoreNetworkBackBuffer() { return m_StoreNetworkBackBuffer; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="w"></param>
		/// <param name="h"></param>
		void CreateNewPlayerBackBuffer(int player, int w, int h);
#pragma endregion

#pragma region Palette Routines
		/// <summary>
		/// Loads a palette from a bitmap file and sets it as the currently used screen palette.
		/// </summary>
		/// <param name="palettePath">String with the path to the palette bitmap file.</param>
		/// <returns>Whether palette loaded successfully or not.</returns>
		bool LoadPalette(std::string palettePath);

		/// <summary>
		/// Gets the ContentFile describing the location of the color palette.
		/// </summary>
		/// <returns>An reference to a ContentFile which described the palette location.</returns>
		const ContentFile & GetPaletteFile() const { return m_PaletteFile; }

		/// <summary>
		/// Fades the palette in from black at a specified speed.
		/// </summary>
		/// <param name="fadeSpeed">Speed specified from (slowest) 1 - 64 (fastest).</param>
		void FadeInPalette(int fadeSpeed = 1) { PALETTE pal; get_palette(pal); fade_in(pal, Limit(fadeSpeed, 64, 1)); }

		/// <summary>
		/// Fades the palette out to black at a specified speed.
		/// </summary>
		/// <param name="fadeSpeed">Speed specified from (slowest) 1 - 64 (fastest).</param>
		void FadeOutPalette(int fadeSpeed = 1) { fade_out(Limit(fadeSpeed, 64, 1)); }
#pragma endregion

#pragma region Screen Capture
		/// <summary>
		/// Dumps a bitmap of the screen back-buffer to a 8bpp BMP file.
		/// </summary>
		/// <param name="nameBase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveScreenToBMP(const char *nameBase);

		/// <summary>
		/// Dumps a bitmap of everything on the scene to the BMP file.
		/// </summary>
		/// <param name="nameBase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveWorldToBMP(const char *nameBase);

		/// <summary>
		/// Dumps a bitmap to a 8bpp BMP file.
		/// </summary>
		/// <param name="nameBase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveBitmapToBMP(BITMAP *bitmap, const char *nameBase);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Entity.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< The friendly-formatted type name of this object.

		static constexpr unsigned short m_BPP = 32; //!< Color depth (bits per pixel).

		int m_GfxDriver; //!< The graphics driver that will be used for rendering.

		unsigned short m_ScreenResX; //!< Width of the physical screen (desktop resolution). 
		unsigned short m_ScreenResY; //!< Height of the physical screen (desktop resolution).

		unsigned short m_ResX; //!< Game window width.
		unsigned short m_ResY; //!< Game window height.
		unsigned short m_NewResX; //!< New game window width that will take effect next time the FrameMan is started.
		unsigned short m_NewResY; //!< New game window height that will take effect next time the FrameMan is started.

		unsigned char m_ResMultiplier; //!< The number of times the game window and image should be multiplied and stretched across for better visibility.
		unsigned char m_NewResMultiplier; //!< This is the new multiple that will take effect next time the FrameMan is started.

		bool m_Fullscreen; //!< Whether in fullscreen mode (borderless window) or not.

		bool m_HSplit; //!< Whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.		
		bool m_VSplit; //!< Whether the screen is split vertically across the screen, ie as two splitscreens side by side.	
		bool m_HSplitOverride; //!< Whether the screen is set to split horizontally in settings.		
		bool m_VSplitOverride; //!< Whether the screen is set to split vertically in settings.
	
		ContentFile m_PaletteFile; //!< File of the screen palette.

		unsigned char m_BlackColor; //!< Palette index for the black color.
		unsigned char m_AlmostBlackColor; //!< Palette index for the closest to black color.

		COLOR_MAP m_LessTransTable; //!< Color table for low transparency.
		COLOR_MAP m_HalfTransTable; //!< Color table for medium transparency.
		COLOR_MAP m_MoreTransTable; //!< Color table for high transparency.
		
		BITMAP *m_PlayerScreen; //!< Intermediary split screen bitmap.
		int m_PlayerScreenWidth; //!< Width of the screen of each player. Will be smaller than resolution only if the screen is split.
		int m_PlayerScreenHeight; //!< Height of the screen of each player. Will be smaller than resolution only if the screen is split.
	
		float m_PPM; //!< Pixels Per Meter constant.		
		float m_MPP; //!< Meters Per Pixel constant.	
		float m_PPL; //!< Pixels per Liter constant.	
		float m_LPP; //!< Liters Per Pixel constant.
	
		AllegroScreen *m_GUIScreen; //!< GUI screen object kept and owned just for the fonts.
		GUIFont *m_SmallFont; //!< Pointer to the standard small font for quick access.	
		GUIFont *m_LargeFont; //!< Pointer to the standard large font for quick access.
		
		std::string m_ScreenText[c_MaxScreenCount]; //!< The text to be displayed on each player's screen.		
		int m_TextDuration[c_MaxScreenCount]; //!< The minimum duration the current message is supposed to show before it can be overwritten.		
		Timer m_TextDurationTimer[c_MaxScreenCount]; //!< Screen text display duration time.
		int m_TextBlinking[c_MaxScreenCount]; //!< Screen text messages blinking interval in ms. 0 is no blink at all, just show message.		
		bool m_TextCentered[c_MaxScreenCount]; //!< Whether screen text is centered vertically.		
		Timer m_TextBlinkTimer; //!< Screen text blink timer.
		
		int m_FlashScreenColor[c_MaxScreenCount]; //!< Whether to flash a player's screen a specific color this frame. -1 means no flash.	
		bool m_FlashedLastFrame[c_MaxScreenCount]; //!< Whether we flashed last frame or not.		
		Timer m_FlashTimer[c_MaxScreenCount]; //!< Flash screen timer.
		
		BITMAP *m_BackBuffer8; //!< Screen back-buffer, always 8bpp, gets copied to the 32bpp buffer for post-processing.
		BITMAP *m_BackBuffer32; //!< 32bpp back-buffer, only used for post-processing.
		BITMAP *m_ScreendumpBuffer; //!< Temporary buffer for making quick screencaps.	

		BITMAP *m_NetworkBackBufferIntermediate8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw.
		BITMAP *m_NetworkBackBufferIntermediateGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw. Used to draw UI only.
		BITMAP *m_NetworkBackBufferFinal8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending.
		BITMAP *m_NetworkBackBufferFinalGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending. Used to draw UI only.

		Vector m_TargetPos[2][c_MaxScreenCount]; //!< Frame target position for network players.

		bool m_DrawNetworkBackBuffer; //!< If true, draws the contents of the m_NetworkBackBuffer8 on top of m_BackBuffer8 every frame in FrameMan.Draw.
		bool m_StoreNetworkBackBuffer; //!< If true, dumps the contents of the m_BackBuffer8 to the m_NetworkBackBuffer8 every frame.

		int m_NetworkFrameCurrent; //!< Which frame index is being rendered, 0 or 1.	
		int m_NetworkFrameReady; //!< Which frame is rendered and ready for transmission, 0 or 1.

		bool m_NetworkBitmapIsLocked[c_MaxScreenCount]; //!< If true then the network bitmap is being updated.
		// TODO: Test if this destroys the whole multiplayer and use instead of the one above if it doesn't.
		//std::mutex m_NetworkBitmapIsLocked[c_MaxScreenCount];

	private:

#pragma region Create Breakdown
		/// <summary>
		/// Checks whether a specific driver has been requested and if not uses the default Allegro windowed magic driver. This is called during Create().
		/// </summary>
		void SetGraphicsDriver();

		/// <summary>
		/// Checks whether the passed in resolution settings make sense. If not, overrides them to prevent crashes or unexpected behavior. This is called during Create().
		/// </summary>
		/// <param name="resX">Game window width (m_ResX or m_NewResX).</param>
		/// <param name="resY">Game window height (m_ResY or m_NewResY).</param>
		void ValidateResolution(unsigned short &resX, unsigned short &resY);

		/// <summary>
		/// Creates all the frame buffer bitmaps to be used by FrameMan. This is called during Create().
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int CreateBackBuffers();
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Draws all the text messages to the specified player screen. This is called during Draw().
		/// </summary>
		/// <param name="playerScreenToFlash">The player screen the text will be shown on.</param>
		/// <param name="guiBitmap">The bitmap the text will be drawn on.</param>
		void DrawScreenText(short playerScreen, AllegroBitmap playerGUIBitmap);

		/// <summary>
		/// Draws the screen flash effect to the specified player screen with parameters set by FlashScreen(). This is called during Draw().
		/// </summary>
		/// <param name="playerScreenToFlash">The player screen the flash effect will be shown to.</param>
		/// <param name="guiBitmap">The bitmap the flash effect will be drawn on.</param>
		void DrawScreenFlash(short playerScreen, BITMAP *playerGUIBitmap);
#pragma endregion

		/// <summary>
		/// Gets the requested font from the GUI engine's current skin. Ownership is NOT transferred!
		/// </summary>
		/// <param name="isSmall">Size of font to get. True for small font, false for large font.</param>
		/// <returns>A pointer to the requested font, or 0 if no font was found.</returns>
		GUIFont * GetFont(bool isSmall);

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