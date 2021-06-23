#ifndef _GUIINTERFACE_
#define _GUIINTERFACE_

// Header file for abstract classes used by the GUI library.

//#include "GUI.h"

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
		GUIBitmap() {};
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GUIBitmap object before deletion from system memory.
		/// </summary>
		virtual ~GUIBitmap() { Destroy(); }

		/// <summary>
		///  Destroys and frees the bitmap.
		/// </summary>
		virtual void Destroy() {}
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Returns the path to the datafile object this GUIBitmap uses.
		/// </summary>
		/// <returns></returns>
		virtual std::string GetDataPath() = 0;

		/// <summary>
		/// Indicates whether this GUI bitmap even contains loaded bitmap data.
		/// </summary>
		/// <returns>Whether this contains bitmap data or not.</returns>
		virtual bool HasBitmap() = 0;

		/// <summary>
		/// Gets the underlying BITMAP of this GUIBitmap.
		/// </summary>
		/// <returns>The underlying BITMAP of this GUIBitmap.</returns>
		virtual BITMAP *GetBitmap() = 0;

		/// <summary>
		/// Sets the underlying Bitmap for this GUIBitmap. Ownership is NOT transferred.
		/// </summary>
		/// <param name="newBitmap">A pointer to the new Bitmap for this GUIBitmap.</param>
		virtual void SetBitmap(BITMAP *newBitmap) = 0;

		/// <summary>
		/// Gets the Width of the bitmap.
		/// </summary>
		virtual int GetWidth() { return 0; }

		/// <summary>
		/// Gets the Height of the bitmap.
		/// </summary>
		virtual int GetHeight() { return 0; }

		/// <summary>
		/// Gets the number of bits per pixel color depth of this bitmap.
		/// </summary>
		/// <returns>Color depth. 8, 16, 32.</returns>
		virtual int GetColorDepth() { return 0; }

		/// <summary>
		/// Sets the color key of the bitmap.
		/// </summary>
		/// <param name="Key">Color key.</param>
		virtual void SetColorKey(unsigned long Key) {}

		/// <summary>
		/// Sets the color key of the bitmap to the color of the pixel in the upper right corner of the bitmap.
		/// </summary>
		/// <param name="Key">Color key.</param>
		virtual void SetColorKey() {}

		/// <summary>
		/// Gets the color of a pixel at a specific point.
		/// </summary>
		virtual unsigned long GetPixel(int X, int Y) { return 0; }

		/// <summary>
		/// Sets the color of a pixel at a specific point.
		/// </summary>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Color">Color of this pixel.</param>
		virtual void SetPixel(int X, int Y, unsigned long Color) {}

		/// <summary>
		/// Gets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="Rect">Pointer to GUIRect struct to fill out.</param>
		virtual void GetClipRect(GUIRect *Rect) {}

		/// <summary>
		/// Sets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="Rect">Rectangle pointer. 0 for no clipping.</param>
		virtual void SetClipRect(GUIRect *Rect) {}

		/// <summary>
		/// Sets the clipping rectangle of the specified bitmap as the intersection of its current clipping rectangle and the rectangle described by the passed-in GUIRect. 
		/// </summary>
		/// <param name="Rect">Rectangle pointer.</param>
		virtual void AddClipRect(GUIRect *Rect) {}
#pragma endregion

#pragma region Pure Virtual Methods
		/// <summary>
		/// Draw a section of this bitmap onto another bitmap
		/// </summary>
		/// <param name="pDestBitmap>Destination Bitmap.</param>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Rect">Size of bitmap we are drawing.</param>
		virtual void Draw(GUIBitmap *pDestBitmap, int X, int Y, GUIRect *Rect) {}

		/// <summary>
		/// Draw a section of this bitmap onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="pDestBitmap>Destination Bitmap.</param>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Rect">Size of this bitmap.</param>
		virtual void DrawTrans(GUIBitmap *pDestBitmap, int X, int Y, GUIRect *Rect) {}

		/// <summary>
		/// Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="pDestBitmap>Destination Bitmap.</param>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="width">Width of this bitmap.</param>
		/// <param name="height">Height of this bitmap.</param>
		virtual void DrawTransScaled(GUIBitmap *pDestBitmap, int X, int Y, int width, int height) {}

		/// <summary>
		/// Draws a line.
		/// </summary>
		/// <param name="x1">Start position on X axis.</param>
		/// <param name="y1">Start position on Y axis.</param>
		/// <param name="x2">End position on X axis.</param>
		/// <param name="y2">End position on Y axis.</param>
		/// <param name="Color">Color to draw this line with.</param>
		virtual void DrawLine(int x1, int y1, int x2, int y2, unsigned long Color) = 0;

		/// <summary>
		/// Draws a rectangle.
		/// </summary>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Width">Width of rectangle.</param>
		/// <param name="Height">Height of rectangle.</param>
		/// <param name="Color">Color to draw this rectangle with.</param>
		/// <param name="Filled">Whether to fill the rectangle with the set color or not.</param>
		virtual void DrawRectangle(int X, int Y, int Width, int Height, unsigned long Color, bool Filled) = 0;
#pragma endregion
	};
#pragma endregion

#pragma region GUIScreen
	/// <summary>
	/// An interface class inherited by the different types of graphics methods
	/// </summary>
	class GUIScreen {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GUIScreen object in system memory.
		/// </summary>
		GUIScreen() {}

		/// <summary>
		/// Creates a bitmap from a file.
		/// </summary>
		/// <param name="Filename">Filename.</param>
		/// <returns>Created bitmap.</returns>
		virtual GUIBitmap * CreateBitmap(const std::string Filename) = 0;

		/// <summary>
		/// Creates an empty bitmap.
		/// </summary>
		/// <param name="Width">Width of bitmap.</param>
		/// <param name="Height">Height of bitmap.</param>
		/// <returns>Created bitmap.</returns>
		virtual GUIBitmap * CreateBitmap(int Width, int Height) = 0;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a GUIScreen object in system memory.
		/// </summary>
		virtual ~GUIScreen() { Destroy(); }

		/// <summary>
		/// Destroy the screen.
		/// </summary>
		virtual void Destroy() {}
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets a bitmap representing the screen.
		/// </summary>
		/// <returns>The bitmap representing the screen.</returns>
		virtual GUIBitmap *GetBitmap() = 0;
#pragma endregion

#pragma region Pure Virtual Methods
		/// <summary>
		/// Draws a bitmap onto the back buffer.
		/// </summary>
		/// <param name="Bitmap">Bitmap to draw.</param>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Rect">Source rectangle.</param>
		virtual void DrawBitmap(GUIBitmap *Bitmap, int X, int Y, GUIRect *Rect) = 0;

		/// <summary>
		/// Draws a bitmap onto the back buffer using the color key.
		/// </summary>
		/// <param name="Bitmap">Bitmap to draw.</param>
		/// <param name="X">Position on X axis.</param>
		/// <param name="Y">Position on Y axis.</param>
		/// <param name="Rect">Source rectangle.</param>
		virtual void DrawBitmapTrans(GUIBitmap *Bitmap, int X, int Y, GUIRect *Rect) = 0;

		/// <summary>
		/// Converts an 8bit palette index to a valid pixel format. Primarily used for development in windowed mode.
		/// </summary>
		/// <param name="color">Color value in any bit depth. Will be converted to the format specified.</param>
		/// <param name="targetDepth">
		/// An optional target color depth that will determine what format the color should be converted to. 
		/// If this is 0, then the current video color depth will be used as target.</param>
		/// <returns>Converted color.</returns>
		virtual unsigned long ConvertColor(unsigned long color, int targetDepth = 0) = 0;
#pragma endregion
	};
#pragma endregion
}
#endif