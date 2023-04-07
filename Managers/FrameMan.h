#ifndef _RTEFRAMEMAN_
#define _RTEFRAMEMAN_

#include "ContentFile.h"
#include "Timer.h"
#include "Box.h"

#define g_FrameMan FrameMan::Instance()

namespace RTE {

	class AllegroScreen;
	class AllegroBitmap;
	class GUIFont;
	class ScreenShader;

	struct BitmapDeleter {
		void operator() (BITMAP *bitmap) const;
	};

	/// <summary>
	/// The singleton manager over the composition of frames.
	/// </summary>
	class FrameMan : public Singleton<FrameMan> {
		friend class SettingsMan;
		friend class WindowMan;

	public:

		Vector SLOffset[c_MaxScreenCount][c_MaxLayersStoredForNetwork]; //!< SceneLayer offsets for each screen in online multiplayer.

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a FrameMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		FrameMan() { Clear(); }

		/// <summary>
		/// Makes the FrameMan object ready for use, which is to be used with SettingsMan first.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a FrameMan object before deletion from system memory.
		/// </summary>
		~FrameMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the FrameMan object.
		/// </summary>
		void Destroy();
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

#pragma region Getters
		/// <summary>
		/// Gets the 8bpp backbuffer bitmap.
		/// </summary>
		/// <returns>A pointer to the BITMAP 8bpp backbuffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer8() const { return m_BackBuffer8.get(); }

		/// <summary>
		/// Gets the 32bpp backbuffer bitmap. Make sure you don't do any blending stuff to the 8bpp one!
		/// </summary>
		/// <returns>A pointer to the BITMAP 32bpp backbuffer. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetBackBuffer32() const { return m_BackBuffer32.get(); }

		/// <summary>
		/// Gets the 32bpp bitmap that is used for overlaying the screen.
		/// </summary>
		/// <returns>A pointer to the overlay BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetOverlayBitmap32() const { return m_OverlayBitmap32.get(); }
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
		/// Gets whether the screen is being split vertically when in two player splitscreen, or is default split horizontally.
		/// </summary>
		/// <returns>Whether the screen is being split vertically when in two player splitscreen or not.</returns>
		bool GetTwoPlayerVSplit() const { return m_TwoPlayerVSplit; }

		/// <summary>
		/// Sets whether the screen should be split vertically when in two player splitscreen or default to the horizontal split.
		/// </summary>
		/// <param name="vSplit">Whether the screen should be split vertically when in two player splitscreen or default to the horizontal split.</param>
		void SetTwoPlayerVSplit(bool vSplit) { m_TwoPlayerVSplit = vSplit; }

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
		/// Gets a Vector with the absolute position of the middle of the specified player's screen.
		/// </summary>
		/// <param name="whichPlayer">Player to get the middle of the screen for. -1 will give the middle of the entire game window.</param>
		/// <returns>The middle of the screen for the specified player, or the middle of the game window if that player is -1.</returns>
		Vector GetMiddleOfPlayerScreen(int whichPlayer);

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
		int CalculateTextWidth(const std::string &text, bool isSmall);

		/// <summary>
		/// Calculates the height of a text string using the given font size.
		/// </summary>
		/// <param name="text">Text string.</param>
		/// <param name="maxWidth">Maximum width of the text string.</param>
		/// <param name="isSmall">Whether to use small or large font.</param>
		/// <returns>Height of the text string.</returns>
		int CalculateTextHeight(const std::string &text, int maxWidth, bool isSmall);

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
		/// Clears the 8bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer8() { clear_to_color(m_BackBuffer8.get(), m_BlackColor); }

		/// <summary>
		/// Clears the 32bpp backbuffer with black.
		/// </summary>
		void ClearBackBuffer32() { clear_to_color(m_BackBuffer32.get(), 0); }

		/// <summary>
		/// Sets a specific color table which is used for any subsequent blended drawing in indexed color modes.
		/// </summary>
		/// <param name="blendMode">The blending mode that will be used in drawing.</param>
		/// <param name="colorChannelBlendAmounts">The color channel blend amounts that will be used to select or create the correct table in the specified blending mode.</param>
		void SetColorTable(DrawBlendMode blendMode, std::array<int, 4> colorChannelBlendAmounts);

		/// <summary>
		/// Sets a specific pre-calculated transparency table which is used for any subsequent transparency drawing in indexed color modes.
		/// </summary>
		/// <param name="transValue">The transparency preset value. See the TransparencyPreset enumeration for values.</param>
		void SetTransTableFromPreset(TransparencyPreset transValue);

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
		/// <param name="bitmap">The Bitmap to draw to. Ownership is NOT transferred.</param>
		/// <param name="start">The absolute Start point.</param>
		/// <param name="end">The absolute end point.</param>
		/// <param name="color">The color value of the line.</param>
		/// <param name="altColor">A color to alternate with. Every other pixel drawn will have this if !0.</param>
		/// <param name="skip">How many pixels to skip drawing between drawn ones. 0 means solid line 2 means there's a gap of two pixels between each drawn one.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int DrawLine(BITMAP *bitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false) const {
			return SharedDrawLine(bitmap, start, end, color, altColor, skip, skipStart, shortestWrap, false, nullptr);
		}

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
		int DrawDotLine(BITMAP *bitmap, const Vector &start, const Vector &end, BITMAP *dot, int skip = 0, int skipStart = 0, bool shortestWrap = false) const {
			return SharedDrawLine(bitmap, start, end, 0, 0, skip, skipStart, shortestWrap, true, dot);
		}
#pragma endregion

#pragma region Network Handling
		/// <summary>
		/// Returns true if this manager is in multiplayer mode, storing the 8bpp backbuffer for network transmission.
		/// </summary>
		/// <returns>True if in multiplayer mode.</returns>
		bool IsInMultiplayerMode() const { return m_StoreNetworkBackBuffer; }

		/// <summary>
		/// Sets the multiplayer mode flag, telling the manager to store the 8bpp backbuffer for network transmission.
		/// </summary>
		/// <param name="value">Whether this manager should operate in multiplayer mode.</param>
		void SetMultiplayerMode(bool value) { m_StoreNetworkBackBuffer = value; }

		/// <summary>
		/// Gets the ready 8bpp backbuffer bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get backbuffer bitmap for.</param>
		/// <returns>A pointer to the 8bpp backbuffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBuffer8Ready(int player) const { return m_NetworkBackBufferFinal8[m_NetworkFrameReady][player].get(); }

		/// <summary>
		/// Gets the ready 8bpp backbuffer GUI bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get GUI backbuffer bitmap for.</param>
		/// <returns>A pointer to the 8bpp GUI backbuffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferGUI8Ready(int player) const { return m_NetworkBackBufferFinalGUI8[m_NetworkFrameReady][player].get(); }

		/// <summary>
		/// Gets the current 8bpp backbuffer bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get backbuffer bitmap for.</param>
		/// <returns>A pointer to the 8bpp backbuffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBuffer8Current(int player) const { return m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][player].get(); }

		/// <summary>
		/// Gets the current 8bpp backbuffer GUI bitmap used to draw network transmitted image on top of everything.
		/// </summary>
		/// <param name="player">Which player screen to get backbuffer bitmap for.</param>
		/// <returns>A pointer to the 8bpp GUI backbuffer BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferGUI8Current(int player) const { return m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][player].get(); }

		/// <summary>
		/// Gets the ready 8bpp intermediate backbuffer bitmap used to copy network transmitted image to before sending.
		/// </summary>
		/// <param name="player">Which player screen to get intermediate bitmap for.</param>
		/// <returns>A pointer to the 8bpp intermediate BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediate8Ready(int player) const { return m_NetworkBackBufferIntermediate8[m_NetworkFrameReady][player].get(); }

		/// <summary>
		/// Gets the ready 8bpp intermediate backbuffer GUI bitmap used to copy network transmitted image to before sending.
		/// </summary>
		/// <param name="player">Which player screen to get intermediate GUI bitmap for.</param>
		/// <returns>A pointer to the 8bpp intermediate GUI BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediate8Current(int player) const { return m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][player].get(); }

		/// <summary>
		/// Gets the current 8bpp intermediate backbuffer bitmap used to copy network transmitted image to before sending.
		/// </summary>
		/// <param name="player">Which player screen to get intermediate bitmap for.</param>
		/// <returns>A pointer to the 8bpp intermediate BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediateGUI8Ready(int player) const { return m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameReady][player].get(); }

		/// <summary>
		/// Gets the current 8bpp intermediate backbuffer GUI bitmap used to copy network transmitted image to before sending.
		/// </summary>
		/// <param name="player">Which player screen to get intermediate GUI bitmap for.</param>
		/// <returns>A pointer to the 8bpp intermediate GUI BITMAP. OWNERSHIP IS NOT TRANSFERRED!</returns>
		BITMAP * GetNetworkBackBufferIntermediateGUI8Current(int player) const { return m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][player].get(); }

		// TODO: Figure out.
		/// <summary>
		///
		/// </summary>
		/// <param name="screen"></param>
		/// <returns></returns>
		Vector GetTargetPos(int screen) const { return m_TargetPos[m_NetworkFrameReady][screen]; }

		/// <summary>
		/// Gets whether we are drawing the contents of the network backbuffers on top of m_BackBuffer8 every frame.
		/// </summary>
		/// <returns>Whether we are drawing the contents of the network backbuffers on top of m_BackBuffer8 every frame or not.</returns>
		bool GetDrawNetworkBackBuffer() const { return m_DrawNetworkBackBuffer; }

		/// <summary>
		/// Sets whether to draw the contents of the network backbuffers on top of m_BackBuffer8 every frame.
		/// </summary>
		/// <param name="value">Whether to draw the contents of the network backbuffers on top of m_BackBuffer8 every frame or not.</param>
		void SetDrawNetworkBackBuffer(bool value) { m_DrawNetworkBackBuffer = value; }

		/// <summary>
		/// Gets whether we are dumping the contents of the m_BackBuffer8 to the network backbuffers every frame.
		/// </summary>
		/// <returns>Whether the contents of the m_BackBuffer8 are being dumped to the network backbuffers every frame.</returns>
		bool GetStoreNetworkBackBuffer() const { return m_StoreNetworkBackBuffer; }

		/// <summary>
		/// Creates a new set of network backbuffers for the specified player.
		/// </summary>
		/// <param name="player">Player to create new backbuffer for.</param>
		/// <param name="width">Width of new backbuffer.</param>
		/// <param name="height">Height of new backbuffer</param>
		void CreateNewNetworkPlayerBackBuffer(int player, int width, int height);
#pragma endregion

#pragma region Palette Routines
		/// <summary>
		/// Loads a palette from a bitmap file and sets it as the currently used screen palette.
		/// </summary>
		/// <param name="palettePath">String with the path to the palette bitmap file.</param>
		/// <returns>Whether palette loaded successfully or not.</returns>
		bool LoadPalette(const std::string &palettePath);

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
		/// Dumps a bitmap to a 8bpp PNG file.
		/// </summary>
		/// <param name="bitmap">The individual bitmap that will be dumped.</param>
		/// <param name="nameBase">The filename of the file to save to, WITHOUT EXTENSION.</param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveBitmapToPNG(BITMAP *bitmap, const char *nameBase) { return SaveBitmap(SingleBitmap, nameBase, bitmap); }

		/// <summary>
		/// Dumps a bitmap of the screen backbuffer to a 8bpp PNG file.
		/// </summary>
		/// <param name="nameBase">The filename of the file to save to, WITHOUT EXTENSION.</param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveScreenToPNG(const char *nameBase) { return SaveBitmap(ScreenDump, nameBase); }

		/// <summary>
		/// Dumps a bitmap of everything on the scene to a PNG file.
		/// </summary>
		/// <param name="nameBase">The filename of the file to save to, WITHOUT EXTENSION.</param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveWorldToPNG(const char *nameBase) { return SaveBitmap(WorldDump, nameBase); }

		/// <summary>
		/// Dumps a miniature screenshot of the whole scene to be used as a preview to a PNG file.
		/// </summary>
		/// <param name="nameBase">The filename of the file to save to, WITHOUT EXTENSION.</param>
		/// <returns>0 for success, anything below 0 is a sign of failure.</returns>
		int SaveWorldPreviewToPNG(const char *nameBase) { return SaveBitmap(ScenePreviewDump, nameBase); }
#pragma endregion

	private:

		/// <summary>
		/// Enumeration with different settings for the SaveBitmap() method.
		/// </summary>
		enum SaveBitmapMode { SingleBitmap, ScreenDump, WorldDump, ScenePreviewDump };

		static constexpr int m_BPP = 32; //!< Color depth (bits per pixel).

		static const std::array<std::function<void(int r, int g, int b, int a)>, DrawBlendMode::BlendModeCount> c_BlenderSetterFunctions; //!< Array of function references to Allegro blender setters for convenient access when creating new color tables.

		bool m_HSplit; //!< Whether the screen is split horizontally across the screen, ie as two splitscreens one above the other.
		bool m_VSplit; //!< Whether the screen is split vertically across the screen, ie as two splitscreens side by side.
		bool m_TwoPlayerVSplit; //!< Whether the screen is set to be split vertically when in two player splitscreen, or is default split horizontally.

		ContentFile m_PaletteFile; //!< File of the screen palette.
		PALETTE m_Palette; //!< The current array of RGB entries read from the palette file.
		PALETTE m_DefaultPalette; //!< The default array of RGB entries read from the palette file at initialization.
		RGB_MAP m_RGBTable; //!< RGB mapping table to speed up calculation of Allegro color maps.

		int m_BlackColor; //!< Palette index for the black color.
		int m_AlmostBlackColor; //!< Palette index for the closest to black color.

		/// <summary>
		/// Color tables for blended drawing in indexed color mode.
		/// The key is an array of the RGBA values. The value is a pair of the color table itself and a time stamp of when it was last accessed for use during color table pruning.
		/// </summary>
		std::array<std::unordered_map<std::array<int, 4>, std::pair<COLOR_MAP, long long>>, DrawBlendMode::BlendModeCount> m_ColorTables;
		Timer m_ColorTablePruneTimer; //!< Timer for pruning unused color tables to prevent ridiculous memory usage.

		std::unique_ptr<BITMAP, BitmapDeleter> m_PlayerScreen; //!< Intermediary split screen bitmap.
		int m_PlayerScreenWidth; //!< Width of the screen of each player. Will be smaller than resolution only if the screen is split.
		int m_PlayerScreenHeight; //!< Height of the screen of each player. Will be smaller than resolution only if the screen is split.

		AllegroScreen *m_GUIScreen; //!< GUI screen object kept and owned just for the fonts.
		GUIFont *m_SmallFont; //!< Pointer to the standard small font for quick access.
		GUIFont *m_LargeFont; //!< Pointer to the standard large font for quick access.

		std::string m_ScreenText[c_MaxScreenCount]; //!< The text to be displayed on each player's screen.
		bool m_TextCentered[c_MaxScreenCount]; //!< Whether screen text is centered vertically.
		int m_TextDuration[c_MaxScreenCount]; //!< The minimum duration the current message is supposed to show before it can be overwritten.
		Timer m_TextDurationTimer[c_MaxScreenCount]; //!< Screen text display duration time.
		int m_TextBlinking[c_MaxScreenCount]; //!< Screen text messages blinking interval in ms. 0 is no blink at all, just show message.
		Timer m_TextBlinkTimer; //!< Screen text blink timer.

		int m_FlashScreenColor[c_MaxScreenCount]; //!< Whether to flash a player's screen a specific color this frame. -1 means no flash.
		bool m_FlashedLastFrame[c_MaxScreenCount]; //!< Whether we flashed last frame or not.
		Timer m_FlashTimer[c_MaxScreenCount]; //!< Flash screen timer.

		std::string m_ScreenDumpName; //!< The filename of the screenshot to save.
		std::unique_ptr<BITMAP, BitmapDeleter> m_BackBuffer8; //!< Screen backbuffer, always 8bpp, gets copied to the 32bpp buffer for post-processing.
		std::unique_ptr<BITMAP, BitmapDeleter> m_BackBuffer32; //!< 32bpp backbuffer, only used for post-processing.
		std::unique_ptr<BITMAP, BitmapDeleter> m_OverlayBitmap32; //!< 32bpp bitmap used for overlaying (fading in/out or darkening) the screen.
		std::unique_ptr<BITMAP, BitmapDeleter> m_ScreenDumpBuffer; //!< Temporary buffer for making quick screencaps.
		std::unique_ptr<BITMAP, BitmapDeleter> m_WorldDumpBuffer; //!< Temporary buffer for making whole scene screencaps.
		std::unique_ptr<BITMAP, BitmapDeleter> m_ScenePreviewDumpGradient; //!< BITMAP for the scene preview sky gradient (easier to load from a pre-made file because it's dithered).
		std::unique_ptr<BITMAP, BitmapDeleter> m_ScreenDumpNamePlaceholder; //!< Dummy BITMAP for keeping naming continuity when saving ScreenDumps with multi-threading.

		std::unique_ptr<BITMAP, BitmapDeleter> m_NetworkBackBufferIntermediate8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw.
		std::unique_ptr<BITMAP, BitmapDeleter> m_NetworkBackBufferIntermediateGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to draw upon during FrameMan draw. Used to draw UI only.
		std::unique_ptr<BITMAP, BitmapDeleter> m_NetworkBackBufferFinal8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending.
		std::unique_ptr<BITMAP, BitmapDeleter> m_NetworkBackBufferFinalGUI8[2][c_MaxScreenCount]; //!< Per-player allocated frame buffer to copy Intermediate before sending. Used to draw UI only.

		Vector m_TargetPos[2][c_MaxScreenCount]; //!< Frame target position for network players.

		bool m_StoreNetworkBackBuffer; //!< If true, dumps the contents of the m_BackBuffer8 to the network backbuffers every frame.
		bool m_DrawNetworkBackBuffer; //!< If true, draws the contents of the network backbuffers on top of m_BackBuffer8 every frame in FrameMan.Draw.

		int m_NetworkFrameCurrent; //!< Which frame index is being rendered, 0 or 1.
		int m_NetworkFrameReady; //!< Which frame is rendered and ready for transmission, 0 or 1.

		std::mutex m_NetworkBitmapLock[c_MaxScreenCount]; //!< Mutex lock for thread safe updating of the network backbuffer bitmaps.

#pragma region Initialize Breakdown
		/// <summary>
		/// Creates all the frame buffer bitmaps to be used by FrameMan. This is called during Initialize().
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int CreateBackBuffers();

		/// <summary>
		/// Creates the RGB lookup table and color table presets for drawing with transparency in indexed color mode. This is called during Initialize().
		/// </summary>
		void CreatePresetColorTables();
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Updates the drawing position of each player screen on the backbuffer when split screen is active. This is called during Draw().
		/// </summary>
		/// <param name="playerScreen">The player screen to update offset for.</param>
		/// <param name="screenOffset">Vector representing the screen offset.</param>
		void UpdateScreenOffsetForSplitScreen(int playerScreen, Vector &screenOffset) const;

		/// <summary>
		/// Draws all the text messages to the specified player screen. This is called during Draw().
		/// </summary>
		/// <param name="playerScreen">The player screen the text will be shown on.</param>
		/// <param name="playerGUIBitmap">The bitmap the text will be drawn on.</param>
		void DrawScreenText(int playerScreen, AllegroBitmap playerGUIBitmap);

		/// <summary>
		/// Draws the screen flash effect to the specified player screen with parameters set by FlashScreen(). This is called during Draw().
		/// </summary>
		/// <param name="playerScreen">The player screen the flash effect will be shown to.</param>
		/// <param name="playerGUIBitmap">The bitmap the flash effect will be drawn on.</param>
		void DrawScreenFlash(int playerScreen, BITMAP *playerGUIBitmap);

		/// <summary>
		/// Renders current frame and marks it ready for network transmission. This is called during Draw().
		/// </summary>
		void PrepareFrameForNetwork();
#pragma endregion

#pragma region Screen Capture
		/// <summary>
		/// Draws the current frame of the whole scene to a temporary buffer that is later saved as a screenshot.
		/// </summary>
		/// <param name="drawForScenePreview">If true will skip drawing objects, post-effects and sky gradient in the WorldDump. To be used for dumping scene preview images.</param>
		void DrawWorldDump(bool drawForScenePreview = false) const;

		/// <summary>
		/// Shared method for saving screenshots or individual bitmaps.
		/// </summary>
		/// <param name="modeToSave">What is being saved. See SaveBitmapMode enumeration for a list of modes.</param>
		/// <param name="nameBase">
		/// The name of the file that is being saved, WITHOUT EXTENSION.
		/// Eg, If "Test" is passed in, this function will save to Test000.bmp, if that file does not already exist. If it does exist, it will attempt 001, and so on.
		/// </param>
		/// <param name="bitmapToSave">The individual bitmap that will be dumped. 0 or nullptr if not in SingleBitmap mode.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int SaveBitmap(SaveBitmapMode modeToSave, const std::string &nameBase, BITMAP *bitmapToSave = nullptr);

		/// <summary>
		/// Saves a BITMAP as an 8bpp bitmap file that is indexed with the specified palette.
		/// </summary>
		/// <param name="fileName">The full name of the file that is being saved. Path and everything included.</param>
		/// <param name="bitmapToSave">The BITMAP that is being saved into a file.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		/// <remarks>
		/// This method is a workaround to Allegro being unable to set a color conversion mode when saving files.
		/// It works by first saving the 32bpp bitmap as is, then loading it back under the REDUCE_TO_256 color conversion mode, blitting it to a fresh bitmap and saving it again with the passed in palette.
		/// The re-blitted bitmap is properly 8bpp and will be indexed correctly. The old saved file is deleted in the process before the new one is saved.
		/// </remarks>
		int SaveIndexedPNG(const char *fileName, BITMAP *bitmapToSave) const;
#pragma endregion

		/// <summary>
		/// Shared method for drawing lines to avoid duplicate code. Will by called by either DrawLine() or DrawDotLine().
		/// </summary>
		/// <param name="bitmap">The Bitmap to draw to. Ownership is NOT transferred.</param>
		/// <param name="start">The absolute Start point.</param>
		/// <param name="end">The absolute end point.</param>
		/// <param name="color">The color value of the line.</param>
		/// <param name="altColor">A color to alternate with. Every other pixel drawn will have this if !0.</param>
		/// <param name="skip">How many pixels to skip drawing between drawn ones. 0 means solid line 2 means there's a gap of two pixels between each drawn one. Should be more than 0 for dots.</param>
		/// <param name="skipStart">The start of the skipping phase. If skip is 10 and this is 5, the first dot will be drawn after 5 pixels.</param>
		/// <param name="shortestWrap">Whether the line should take the shortest possible route across scene wraps.</param>
		/// <param name="drawDot">Whether to draw a regular line or a dot line. True for dot line.</param>
		/// <param name="dot">The bitmap to be used for dots (will be centered).</param>
		/// <returns>The end state of the skipping phase. Eg if 4 is returned here the last dot was placed 4 pixels ago.</returns>
		int SharedDrawLine(BITMAP *bitmap, const Vector &start, const Vector &end, int color, int altColor = 0, int skip = 0, int skipStart = 0, bool shortestWrap = false, bool drawDot = false, BITMAP *dot = nullptr) const;

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
		FrameMan(const FrameMan &reference) = delete;
		FrameMan & operator=(const FrameMan &rhs) = delete;
	};
}
#endif
