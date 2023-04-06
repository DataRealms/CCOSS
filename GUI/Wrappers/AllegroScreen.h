#ifndef _ALLEGROSCREEN_
#define _ALLEGROSCREEN_

#include "AllegroBitmap.h"

namespace RTE {

	/// <summary>
	/// Wrapper class to convert raw Allegro BITMAPs to GUI library backbuffer bitmaps.
	/// </summary>
	class AllegroScreen : public GUIScreen {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an AllegroScreen object in system memory and make it ready for use.
		/// </summary>
		/// <param name="backBuffer">A bitmap that represents the back buffer. Ownership is NOT transferred!</param>
		explicit AllegroScreen(BITMAP *backBuffer) { m_BackBufferBitmap = std::make_unique<AllegroBitmap>(backBuffer); }

		/// <summary>
		/// Creates a bitmap from a file.
		/// </summary>
		/// <param name="fileName">File name to create bitmap from.</param>
		/// <returns>Pointer to the created bitmap. Ownership IS transferred!</returns>
		GUIBitmap * CreateBitmap(const std::string &fileName) override;

		/// <summary>
		/// Creates an empty bitmap.
		/// </summary>
		/// <param name="width">Bitmap width.</param>
		/// <param name="height">Bitmap height.</param>
		/// <returns>Pointer to the created bitmap. Ownership IS transferred!</returns>
		GUIBitmap * CreateBitmap(int width, int height) override;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a AllegroScreen object before deletion from system memory.
		/// </summary>
		~AllegroScreen() override { Destroy(); }

		/// <summary>
		/// Destroys the AllegroScreen object.
		/// </summary>
		void Destroy() override { m_BackBufferBitmap.reset(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the bitmap representing the screen.
		/// </summary>
		/// <returns>Pointer to the bitmap representing the screen. Ownership is NOT transferred!</returns>
		GUIBitmap * GetBitmap() const override { return m_BackBufferBitmap.get(); }
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// Draws a bitmap onto the back buffer.
		/// </summary>
		/// <param name="guiBitmap">The bitmap to draw to this AllegroScreen.</param>
		/// <param name="destX">Destination X position</param>
		/// <param name="destY">Destination Y position</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		void DrawBitmap(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) override;

		/// <summary>
		/// Draws a bitmap onto the back buffer ignoring color-keyed pixels.
		/// </summary>
		/// <param name="guiBitmap">The bitmap to draw to this AllegroScreen.</param>
		/// <param name="destX">Destination X position</param>
		/// <param name="destY">Destination Y position</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		void DrawBitmapTrans(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) override;
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Converts an 8bit palette index to a valid pixel format color.
		/// </summary>
		/// <param name="color">Color value in any bit depth. Will be converted to the format specified.</param>
		/// <param name="targetColorDepth">An optional target color depth that will determine what format the color should be converted to. If this is 0, then the current video color depth will be used as target.</param>
		/// <returns>The converted color.</returns>
		unsigned long ConvertColor(unsigned long color, int targetColorDepth = 0) override;
#pragma endregion

	private:

		std::unique_ptr<AllegroBitmap> m_BackBufferBitmap; //!< The AllegroBitmap that makes this AllegroScreen.

		// Disallow the use of some implicit methods.
		AllegroScreen & operator=(const AllegroScreen &rhs) = delete;
	};
};
#endif