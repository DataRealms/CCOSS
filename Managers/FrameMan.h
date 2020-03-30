#ifndef _RTEFRAMEMAN_
#define _RTEFRAMEMAN_

#include "RTETools.h"
#include "ContentFile.h"
#include "Timer.h"
#include "Box.h"

#define g_FrameMan FrameMan::Instance()

namespace RTE {

	class AllegroScreen;
	class GUIFont;

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
		/// Gets the 8bpp back buffer bitmap.
		/// </summary>
		/// <returns>A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer8() const { return m_pBackBuffer8; }

		/// <summary>
		/// Gets the 32bpp back buffer bitmap, if available. If not, the 8bpp is returned. Make sure you don't do any blending stuff to the 8bpp one!
		/// </summary>
		/// <returns>A pointer to the BITMAP 32bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer32() const { return m_pBackBuffer32 ? m_pBackBuffer32 : m_pBackBuffer8; }

		/// <summary>
		/// Indicates whether the 32bpp frame buffer is currently being used or not when flipping the frame buffers.
		/// </summary>
		/// <returns>Whether the 32bpp is used or not (8bpp one is).</returns>
		bool FlippingWith32BPP() const;

		/// <summary>
		/// Gets the bits per pixel color depth.
		/// </summary>
		/// <returns>An int describing the number of bits per pixel of the current color depth.</returns>
		int GetBPP() const { return m_BPP; }

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
		/// Indicates whether we're in fullscreen mode or not.
		/// </summary>
		/// <returns>Whether we're in fullscreen mode.</returns>
		bool IsFullscreen() const { return m_Fullscreen; }

		/// <summary>
		/// Toggles to and from fullscreen and windowed mode.
		/// </summary>
		/// <returns>Error code, anything other than 0 is error.</returns>
		int ToggleFullscreen();

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
		/// Tells how many times the windowed screen resolution is being multiplied and the back buffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the windowed mode screen resolution is run in (1 normal).</returns>
		int NxWindowed() const { return m_NxWindowed; }

		/// <summary>
		/// Sets and switches to a new windowed mode multiplier.
		/// </summary>
		/// <param name="multiplier">The multiplier to switch to.</param>
		/// <returns>Error code, anything other than 0 is error.</returns>
		int SwitchWindowMultiplier(int multiplier = 1);

		/// <summary>
		/// Tells how many times the fullscreen resolution is being multiplied and the back buffer stretched across for better readability.
		/// </summary>
		/// <returns>What multiple the fullscreen mode screen resolution is run in (1 normal).</returns>
		int NxFullscreen() const { return m_NxFullscreen; }

		/// <summary>
		/// Gets how many times the fullscreen resolution will be multiplied ON NEXT RESTART and the back buffer stretched.
		/// </summary>
		/// <returns>What multiple the fullscreen mode screen resolution will be run in on next restart of game.</returns>
		int GetNewNxFullscreen() const { return m_NewNxFullscreen; }

		/// <summary>
		/// Sets how many times the fullscreen resolution will be multiplied ON NEXT RESTART and the back buffer stretched.
		/// </summary>
		/// <param name="newMultiple">What multiple the fullscreen mode screen resolution will be run in on next restart of game.</param>
		void SetNewNxFullscreen(int newMultiple) { m_NewNxFullscreen = newMultiple; }
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
		/// Gets the width of the individual player screens. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <param name="whichPlayer">Player to get screen width for, only used by multiplayer parts.</param>
		/// <returns>The width of the player screens.</returns>
		int GetPlayerFrameBufferWidth(int whichPlayer) const;

		/// <summary>
		/// Gets the height of the individual player screens. This will only be less than the backbuffer resolution if there are split screens.
		/// </summary>
		/// <param name="whichPlayer">Player to get screen width for, only used by multiplayer parts.</param>
		/// <returns>The height of the player screens.</returns>
		int GetPlayerFrameBufferHeight(int whichPlayer) const;
#pragma endregion

#pragma region Text Handling
		/// <summary>
		/// Gets the large font from the GUI engine's current skin. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the requested font, or 0 if no large font was found.</returns>
		GUIFont * GetSmallFont();

		/// <summary>
		/// Gets the large font from the GUI engine's current skin. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the requested font, or 0 if no large font was found.</returns>
		GUIFont * GetLargeFont();

		/// <summary>
		/// Returns max text width.
		/// </summary>
		/// <param name="text">Text string.</param>
		/// <param name="isSmall">Whether to use small or large font.</param>
		/// <returns>Max text width.</returns>
		int CalculateTextWidth(std::string text, bool isSmall);

		/// <summary>
		/// Returns max text height.
		/// </summary>
		/// <param name="text">Text string.</param>
		/// <param name="maxWidth">Max width.</param>
		/// <param name="isSmall">Whether to use small or large font.</param>
		/// <returns>Max text height.</returns>
		int CalculateTextHeight(std::string text, int maxWidth, bool isSmall);

		/// <summary>
		/// Gets the message to be displayed on top of each player's screen.
		/// </summary>
		/// <param name="which">Which player screen to get message from.</param>
		/// <returns>Current message shown to player.</returns>
		std::string GetScreenText(int which = 0) const { return (which >= 0 && which < c_MaxScreenCount) ? m_ScreenText[which] : ""; }

		/// <summary>
		/// Sets the message to be displayed on top of each player's screen
		/// </summary>
		/// <param name="msg">An std::string that specifies what should be displayed.</param>
		/// <param name="which">Which screen you want to set text to.</param>
		/// <param name="blinkInterval">The interval with which the screen will be blinking, in ms. 0 means no blinking.</param>
		/// <param name="displayDuration">The duration, in MS to force this message to display. No other message can be displayed before this expires. ClearScreenText overrides it though.</param>
		/// <param name="centered">Vertically centered on the screen.</param>
		void SetScreenText(const std::string &msg, int which = 0, int blinkInterval = 0, int displayDuration = -1, bool centered = false);

		/// <summary>
		/// Clears the message to be displayed on top of each player's screen.
		/// </summary>
		/// <param name="which">Which screen message to clear..</param>
		void ClearScreenText(int which = 0);
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// Flips the frame buffers.
		/// </summary>
		void FlipFrameBuffers();

		/// <summary>
		/// Clears the 8bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer8() { clear_to_color(m_pBackBuffer8, m_BlackColor); }

		/// <summary>
		/// Clears the 32bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer32() { if (m_pBackBuffer32) clear_to_color(m_pBackBuffer32, 0); }

		/// <summary>
		/// Sets a specific recalculated transparency table which is used for any subsequent transparency drawing.
		/// </summary>
		/// <param name="transSetting">The transparency setting, see the enumerator.</param>
		void SetTransTable(TransperencyPreset transSetting);

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
		/// <param name="altColor">A color to alternate with every other pixel drawn will have this if !0.</param>
		/// <param name="skip">How many pixels to skip drawing between drawn ones. 0 means solid line 2 means there's a gap of two pixels between each drawn one.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int DrawLine(BITMAP *pBitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false);

		/// <summary>
		/// Draws a line that can be dotted with bitmaps.
		/// </summary>
		/// <param name="pBitmap">The Bitmap to draw to. Ownership is NOT transferred.</param>
		/// <param name="start">The absolute Start point.</param>
		/// <param name="end">The absolute end point.</param>
		/// <param name="pDot">The bitmap to be used for dots (will be centered).</param>
		/// <param name="skip">How many pixels to gap between drawing dots. Should be more than 0.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int DrawDotLine(BITMAP *pBitmap, const Vector &start, const Vector &end, BITMAP *pDot, int skip = 0, int skipStart = 0, bool shortestWrap = false);
#pragma endregion

#pragma region Network Handling
		/// <summary>
		/// Gets the 8bpp back buffer bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get back buffer bitmap from.</param>
		/// <returns>A pointer to the BITMAP 8bpp back buffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBuffer8Ready(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameReady][player]; }
		BITMAP * GetNetworkBackBuffer8Current(int player) const { return m_pNetworkBackBufferFinal8[m_NetworkFrameCurrent][player]; }
		BITMAP * GetNetworkBackBufferIntermediate8Ready(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameReady][player]; }
		BITMAP * GetNetworkBackBufferIntermediate8Current(int player) const { return m_pNetworkBackBufferIntermediate8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <returns></returns>
		BITMAP * GetNetworkBackBufferGUI8Ready(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameReady][player]; }
		BITMAP * GetNetworkBackBufferGUI8Current(int player) const { return m_pNetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][player]; }
		BITMAP * GetNetworkBackBufferIntermediateGUI8Ready(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameReady][player]; }
		BITMAP * GetNetworkBackBufferIntermediateGUI8Current(int player) const { return m_pNetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][player]; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool IsInMultiplayerMode() { return m_StoreNetworkBackBuffer; }

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
		/// <param name="value"></param>
		void SetStoreNetworkBackBuffer(bool value) { m_StoreNetworkBackBuffer = value; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="w"></param>
		/// <param name="h"></param>
		void CreateNewPlayerBackBuffer(int player, int w, int h);

		/// <summary>
		/// 
		/// </summary>
		Vector SLOffset[c_MaxScreenCount][c_MaxLayersStoredForNetwork];
#pragma endregion

#pragma region Palette Routines
		/// <summary>
		/// Loads a palette from a .dat file and sets it as the currently used screen palette.
		/// </summary>
		/// <param name="palettePath">String with the data path to the palette data object within a .dat.</param>
		/// <returns>Whether palette loaded successfully or not.</returns>
		bool LoadPalette(std::string palettePath);

		/// <summary>
		/// Gets the ContentFile describing the location of the color palette.
		/// </summary>
		/// <returns>An reference to a ContentFile which described the palette location.</returns>
		const ContentFile & GetPaletteFile() const { return m_PaletteFile; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned char GetBlackIndex() const { return m_BlackColor; }

		/// <summary>
		/// Fades the palette in from black at a specified speed.
		/// </summary>
		/// <param name="fadeSpeed">Speed specified from (slowest) 1 - 64 (fastest).</param>
		void FadeInPalette(int fadeSpeed = 1);

		/// <summary>
		/// Fades the palette out to black at a specified speed.
		/// </summary>
		/// <param name="fadeSpeed">Speed specified from (slowest) 1 - 64 (fastest).</param>
		void FadeOutPalette(int fadeSpeed = 1);
#pragma endregion

#pragma region Screen Capture
		/// <summary>
		/// Dumps a bitmap of the screen back buffer to a 8bpp BMP file.
		/// </summary>
		/// <param name="namebase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveScreenToBMP(const char *namebase);

		/// <summary>
		/// Dumps a bitmap of everything on the scene to the BMP file.
		/// </summary>
		/// <param name="namebase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveWorldToBMP(const char *namebase);

		/// <summary>
		/// Dumps a bitmap to a 8bpp BMP file.
		/// </summary>
		/// <param name="namebase">
		/// The filename of the file to save to, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveBitmapToBMP(BITMAP *pBitmap, const char *namebase);
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

		// Resolution
		int m_ResX;
		int m_ResY;

		// These are the new resolution settings that will take effect next time the FrameMan is started
		int m_NewResX;
		int m_NewResY;

		bool m_Fullscreen; //!< Whether in fullscreen mode or not.	
		int m_NxWindowed; //!< The number of times the windowed mode resolution should be multiplied and stretched across for better visibility.

		// The number of times the fullscreen mode resolution should be multiplied and stretched across for better visibility
		// The internal virtual resolution (m_ResX, m_ResY) is 1/n of the actual fullscreen res that the graphics card outputs
		int m_NxFullscreen;

		int m_NewNxFullscreen; //!< This is the new fullscreen multiple that will take effect next time the FrameMan is started.

		bool m_HSplit; //!< Whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.		
		bool m_VSplit; //!< Whether the screen is split vertically across the screen, ie as two splitscreens side by side.	
		bool m_HSplitOverride; //!< Whether the screen is set to split horizontally in settings.		
		bool m_VSplitOverride; //!< Whether the screen is set to split vertically in settings.



		// Color depth (bits per pixel)
		int m_BPP;

		//!< Data file of the screen palette
		ContentFile m_PaletteFile;
		//DATAFILE *m_pPaletteDataFile;

		// Transparency color tables
		COLOR_MAP m_LessTransTable;
		COLOR_MAP m_HalfTransTable;
		COLOR_MAP m_MoreTransTable;

		// Color black index
		unsigned char m_BlackColor;
		unsigned char m_AlmostBlackColor;
		

		BITMAP *m_pPlayerScreen; //!< Intermediary split screen bitmap.

		// Dimensions of each of the screens of each player. Will be smaller than resolution only if the screen is split
		int m_PlayerScreenWidth;
		int m_PlayerScreenHeight;
	
		float m_PPM; //!< Pixels Per Meter constant.		
		float m_MPP; //!< Meters Per Pixel constant.	
		float m_PPL; //!< Pixels per Liter constant.	
		float m_LPP; //!< Liters Per Pixel constant.

		// The GUI control managers for all teams
	//    std::vector<GUIControlManager *> m_BuyGUIs;

		//!< GUI screen object kept and owned just for the fonts.
		AllegroScreen *m_pGUIScreen;
		//!< Standard fonts for quick access.
		GUIFont *m_pLargeFont;
		GUIFont *m_pSmallFont;
		
		std::string m_ScreenText[c_MaxScreenCount]; //!< The text to be displayed on each player's screen.		
		int m_TextDuration[c_MaxScreenCount]; //!< The minimum duration the current message is supposed to show before it can be overwritten.		
		Timer m_TextDurationTimer[c_MaxScreenCount]; //!< Screen text display duration time.
		int m_TextBlinking[c_MaxScreenCount]; //!< Screen text messages blinking interval in ms. 0 is no blink at all, just show message.		
		bool m_TextCentered[c_MaxScreenCount]; //!< Whether screen text is centered vertically.		
		Timer m_TextBlinkTimer; //!< Screen text blink timer.
		
		int m_FlashScreenColor[c_MaxScreenCount]; //!< Whether to flash a player's screen a specific color this frame. -1 means no flash.	
		bool m_FlashedLastFrame[c_MaxScreenCount]; //!< Whether we flashed last frame or not.		
		Timer m_FlashTimer[c_MaxScreenCount]; //!< Flash screen timer.
		

		Vector m_TargetPos[2][c_MaxScreenCount]; //!< Frame target pos for network players.


		BITMAP *m_pBackBuffer8; //!< Screen back buffer, always 8bpp, gets copied to the 32bpp buffer if post processing is used.
		BITMAP *m_pBackBuffer32; //!< 32Bits per pixel back buffer, only used if player elects, and only if in 32bpp video mode.
		BITMAP *m_pScreendumpBuffer; //!< Temporary buffer for making quick screencaps.			
		BITMAP *m_pNetworkBackBufferIntermediate8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw.
		BITMAP *m_pNetworkBackBufferIntermediateGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw used to draw UI only.
		BITMAP *m_pNetworkBackBufferFinal8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending
		BITMAP *m_pNetworkBackBufferFinalGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending used to draw UI only.


		bool m_DrawNetworkBackBuffer; //!< If true, draws the contents of the m_pNetworkBackBuffer8 on top of m_pBackBuffer8 every frame in FrameMan.Draw.
		bool m_StoreNetworkBackBuffer; //!< If true, dumps the contents of the m_pBackBuffer8 to the m_pNetworkBackBuffer8 every frame.

		int m_NetworkFrameCurrent; //!< Which frame index is being rendered, 0 or 1.	
		int m_NetworkFrameReady; //!< Which frame is rendered and ready for transmission, 0 or 1.

		bool m_NetworkBitmapIsLocked[c_MaxScreenCount]; //!< If true then the network bitmap is being updated
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