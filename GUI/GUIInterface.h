#ifndef _GUIINTERFACE_
#define _GUIINTERFACE_

// Header file for abstract classes used by the GUI library.

struct BITMAP;

namespace RTE {

#pragma region GUIBitmap
	/// <summary>
	/// An interface class inherited by the different types of bitmap methods.
	/// </summary>
	class GUIBitmap {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GUIBitmap object in system memory.
		/// </summary>
		GUIBitmap() = default;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GUIBitmap object before deletion from system memory.
		/// </summary>
		virtual ~GUIBitmap() = default;

		/// <summary>
		/// Destroys and resets the GUIBitmap object.
		/// </summary>
		virtual void Destroy() = 0;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the path to the data file this GUIBitmap uses.
		/// </summary>
		/// <returns>>Path to the data file this GUIBitmap uses.</returns>
		virtual std::string GetDataPath() const = 0;

		/// <summary>
		/// Gets the underlying BITMAP of this GUIBitmap.
		/// </summary>
		/// <returns>The underlying BITMAP of this GUIBitmap.</returns>
		virtual BITMAP * GetBitmap() const = 0;

		/// <summary>
		/// Sets the underlying BITMAP for this GUIBitmap.
		/// </summary>
		/// <param name="newBitmap">A pointer to the new BITMAP for this GUIBitmap.</param>
		virtual void SetBitmap(BITMAP *newBitmap) = 0;

		/// <summary>
		/// Gets the width of the bitmap.
		/// </summary>
		/// <returns>The width of the bitmap.</returns>
		virtual int GetWidth() const = 0;

		/// <summary>
		/// Gets the height of the bitmap.
		/// </summary>
		/// <returns>The height of the bitmap.</returns>
		virtual int GetHeight() const = 0;

		/// <summary>
		/// Gets the number of bits per pixel color depth of the bitmap.
		/// </summary>
		/// <returns>The color depth of the bitmap.</returns>
		virtual int GetColorDepth() const = 0;

		/// <summary>
		/// Gets the color of a pixel at a specific point on the bitmap.
		/// </summary>
		/// <param name="posX">X position on bitmap.</param>
		/// <param name="posY">Y position on bitmap.</param>
		/// <returns>The color of the pixel at the specified point.</returns>
		virtual unsigned long GetPixel(int posX, int posY) const = 0;

		/// <summary>
		/// Sets the color of a pixel at a specific point on the bitmap.
		/// </summary>
		/// <param name="posX">X position on bitmap.</param>
		/// <param name="posY">Y position on bitmap.</param>
		/// <param name="pixelColor">The color to set the pixel to.</param>
		virtual void SetPixel(int posX, int posY, unsigned long pixelColor) = 0;

		/// <summary>
		/// Sets the color key (mask color) of the bitmap to the color of the pixel in the upper right corner of the bitmap.
		/// </summary>
		virtual void SetColorKey() {}

		/// <summary>
		/// Sets the color key (mask color) of the bitmap.
		/// </summary>
		/// <param name="colorKey">Color key (mask color).</param>
		virtual void SetColorKey(unsigned long colorKey) {}
#pragma endregion

#pragma region Clipping
		/// <summary>
		/// Gets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="clippingRect">Pointer to a GUIRect to fill out.</param>
		virtual void GetClipRect(GUIRect *clippingRect) const = 0;

		/// <summary>
		/// Sets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="clippingRect">Pointer to a GUIRect to use as the clipping rectangle, or nullptr for no clipping.</param>
		virtual void SetClipRect(GUIRect *clippingRect) = 0;

		/// <summary>
		/// Sets the clipping rectangle of the specified bitmap as the intersection of its current clipping rectangle and the rectangle described by the passed-in GUIRect. 
		/// </summary>
		/// <param name="rect">Rectangle pointer.</param>
		virtual void AddClipRect(GUIRect *rect) = 0;
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// Draw a section of this bitmap onto another bitmap.
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		virtual void Draw(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) = 0;

		/// <summary>
		/// Draw a section of this bitmap onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		virtual void DrawTrans(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) = 0;

		/// <summary>
		/// Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="width">Target width of the bitmap.</param>
		/// <param name="height">Target height of the bitmap.</param>
		virtual void DrawTransScaled(GUIBitmap *destBitmap, int destX, int destY, int width, int height) = 0;
#pragma endregion

#pragma region Primitive Drawing
		/// <summary>
		/// Draws a line on this bitmap.
		/// </summary>
		/// <param name="x1">Start position on X axis.</param>
		/// <param name="y1">Start position on Y axis.</param>
		/// <param name="x2">End position on X axis.</param>
		/// <param name="y2">End position on Y axis.</param>
		/// <param name="color">Color to draw this line with.</param>
		virtual void DrawLine(int x1, int y1, int x2, int y2, unsigned long color) = 0;

		/// <summary>
		/// Draws a rectangle on this bitmap.
		/// </summary>
		/// <param name="posX">Position on X axis.</param>
		/// <param name="posY">Position on Y axis.</param>
		/// <param name="width">Width of rectangle.</param>
		/// <param name="height">Height of rectangle.</param>
		/// <param name="color">Color to draw this rectangle with.</param>
		/// <param name="filled">Whether to fill the rectangle with the set color or not.</param>
		virtual void DrawRectangle(int posX, int posY, int width, int height, unsigned long color, bool filled) = 0;
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIBitmap & operator=(const GUIBitmap &rhs) = delete;
	};
#pragma endregion

#pragma region GUIScreen
	/// <summary>
	/// An interface class inherited by the different types of graphics methods.
	/// </summary>
	class GUIScreen {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GUIScreen object in system memory.
		/// </summary>
		GUIScreen() = default;

		/// <summary>
		/// Creates a bitmap from a file.
		/// </summary>
		/// <param name="fileName">File name to create bitmap from.</param>
		/// <returns>Pointer to the created bitmap.</returns>
		virtual GUIBitmap * CreateBitmap(const std::string &fileName) = 0;

		/// <summary>
		/// Creates an empty bitmap.
		/// </summary>
		/// <param name="width">Bitmap width.</param>
		/// <param name="height">Bitmap height.</param>
		/// <returns>Pointer to the created bitmap.</returns>
		virtual GUIBitmap * CreateBitmap(int width, int height) = 0;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GUIScreen object before deletion from system memory.
		/// </summary>
		virtual ~GUIScreen() = default;

		/// <summary>
		/// Destroys and resets the GUIScreen object.
		/// </summary>
		virtual void Destroy() = 0;
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the bitmap representing the screen.
		/// </summary>
		/// <returns>Pointer to the bitmap representing the screen.</returns>
		virtual GUIBitmap * GetBitmap() const = 0;
#pragma endregion

#pragma region Pure Virtual Methods
		/// <summary>
		/// Draws a bitmap onto the back buffer.
		/// </summary>
		/// <param name="guiBitmap">The bitmap to draw to this AllegroScreen.</param>
		/// <param name="destX">Destination X position</param>
		/// <param name="destY">Destination Y position</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		virtual void DrawBitmap(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) = 0;

		/// <summary>
		/// Draws a bitmap onto the back buffer ignoring color-keyed pixels.
		/// </summary>
		/// <param name="guiBitmap">The bitmap to draw to this AllegroScreen.</param>
		/// <param name="destX">Destination X position</param>
		/// <param name="destY">Destination Y position</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		virtual void DrawBitmapTrans(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) = 0;

		/// <summary>
		/// Converts an 8bit palette index to a valid pixel format color.
		/// </summary>
		/// <param name="color">Color value in any bit depth. Will be converted to the format specified.</param>
		/// <param name="targetColorDepth">An optional target color depth that will determine what format the color should be converted to. If this is 0, then the current video color depth will be used as target.</param>
		/// <returns>The converted color.</returns>
		virtual unsigned long ConvertColor(unsigned long color, int targetColorDepth = 0) = 0;
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIScreen & operator=(const GUIScreen &rhs) = delete;
	};
#pragma endregion
}
#endif