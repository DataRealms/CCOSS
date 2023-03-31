#ifndef _ALLEGROBITMAP_
#define _ALLEGROBITMAP_

#include "GUIInterface.h"
#include "ContentFile.h"

namespace RTE {

	/// <summary>
	/// Wrapper class to convert raw Allegro BITMAPs to GUI library bitmaps.
	/// </summary>
	class AllegroBitmap : public GUIBitmap {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an AllegroBitmap object in system memory.
		/// </summary>
		AllegroBitmap() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate an AllegroBitmap object in system memory and make it ready for use.
		/// </summary>
		/// <param name="bitmap">The underlaying BITMAP of this AllegroBitmap. Ownership is NOT transferred!</param>
		explicit AllegroBitmap(BITMAP *bitmap) { Clear(); m_Bitmap = bitmap; }

		/// <summary>
		/// Creates an AllegroBitmap from a file.
		/// </summary>
		/// <param name="fileName">File name to get the underlaying BITMAP from. Ownership is NOT transferred!</param>
		void Create(const std::string &fileName);

		/// <summary>
		/// Creates an empty BITMAP that is owned by this AllegroBitmap.
		/// </summary>
		/// <param name="width">Bitmap width.</param>
		/// <param name="height">Bitmap height.</param>
		/// <param name="colorDepth">Bitmap color depth (8 or 32).</param>
		void Create(int width, int height, int colorDepth = 8);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a AllegroBitmap object before deletion from system memory.
		/// </summary>
		~AllegroBitmap() override { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the AllegroBitmap object.
		/// </summary>
		void Destroy() override;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the path to the data file this AllegroBitmap uses.
		/// </summary>
		/// <returns>Path to the data file this AllegroBitmap uses.</returns>
		std::string GetDataPath() const override { return m_BitmapFile.GetDataPath(); }

		/// <summary>
		/// Gets the underlying BITMAP of this AllegroBitmap.
		/// </summary>
		/// <returns>The underlying BITMAP of this AllegroBitmap.</returns>
		BITMAP * GetBitmap() const override { return m_Bitmap; }

		/// <summary>
		/// Sets the underlying BITMAP for this AllegroBitmap. Ownership is NOT transferred.
		/// </summary>
		/// <param name="newBitmap">A pointer to the new BITMAP for this AllegroBitmap.</param>
		void SetBitmap(BITMAP *newBitmap) override { Destroy(); m_Bitmap = newBitmap; }

		/// <summary>
		/// Gets the width of the bitmap.
		/// </summary>
		/// <returns>The width of the bitmap.</returns>
		int GetWidth() const override;

		/// <summary>
		/// Gets the height of the bitmap.
		/// </summary>
		/// <returns>The height of the bitmap.</returns>
		int GetHeight() const override;

		/// <summary>
		/// Gets the number of bits per pixel color depth of the bitmap.
		/// </summary>
		/// <returns>The color depth of the bitmap.</returns>
		int GetColorDepth() const override;

		/// <summary>
		/// Gets the color of a pixel at a specific point on the bitmap.
		/// </summary>
		/// <param name="posX">X position on bitmap.</param>
		/// <param name="posY">Y position on bitmap.</param>
		/// <returns>The color of the pixel at the specified point.</returns>
		unsigned long GetPixel(int posX, int posY) const override;

		/// <summary>
		/// Sets the color of a pixel at a specific point on the bitmap.
		/// </summary>
		/// <param name="posX">X position on bitmap.</param>
		/// <param name="posY">Y position on bitmap.</param>
		/// <param name="pixelColor">The color to set the pixel to.</param>
		void SetPixel(int posX, int posY, unsigned long pixelColor) override;
#pragma endregion

#pragma region Clipping
		/// <summary>
		/// Gets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="clippingRect">Pointer to a GUIRect to fill out.</param>
		void GetClipRect(GUIRect *clippingRect) const override;

		/// <summary>
		/// Sets the clipping rectangle of the bitmap.
		/// </summary>
		/// <param name="clippingRect">Pointer to a GUIRect to use as the clipping rectangle, or nullptr for no clipping.</param>
		void SetClipRect(GUIRect *clippingRect) override;

		/// <summary>
		/// Sets the clipping rectangle of the bitmap as the intersection of its current clipping rectangle and the passed-in rectangle.
		/// </summary>
		/// <param name="clippingRect">Pointer to a GUIRect to add to the existing clipping rectangle.</param>
		void AddClipRect(GUIRect *clippingRect) override;
#pragma endregion

#pragma region Drawing
		/// <summary>
		/// Draw a section of this bitmap onto another bitmap
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		void Draw(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) override;

		/// <summary>
		/// Draw a section of this bitmap onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="srcPosAndSizeRect">Source bitmap position and size rectangle.</param>
		void DrawTrans(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) override;

		/// <summary>
		/// Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.
		/// </summary>
		/// <param name="destBitmap">Bitmap to draw onto.</param>
		/// <param name="destX">Destination X position.</param>
		/// <param name="destY">Destination Y position.</param>
		/// <param name="width">Target width of the bitmap.</param>
		/// <param name="height">Target height of the bitmap.</param>
		void DrawTransScaled(GUIBitmap *destBitmap, int destX, int destY, int width, int height) override;
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
		void DrawLine(int x1, int y1, int x2, int y2, unsigned long color) override;

		/// <summary>
		/// Draws a rectangle on this bitmap.
		/// </summary>
		/// <param name="posX">Position on X axis.</param>
		/// <param name="posY">Position on Y axis.</param>
		/// <param name="width">Width of rectangle.</param>
		/// <param name="height">Height of rectangle.</param>
		/// <param name="color">Color to draw this rectangle with.</param>
		/// <param name="filled">Whether to fill the rectangle with the set color or not.</param>
		void DrawRectangle(int posX, int posY, int width, int height, unsigned long color, bool filled) override;
#pragma endregion

	private:

		BITMAP *m_Bitmap; //!< The underlaying BITMAP.
		ContentFile m_BitmapFile; //!< The ContentFile the underlaying BITMAP was created from, if created from a file.
		bool m_SelfCreated; //!< Whether the underlaying BITMAP was created by this and is owned.

		/// <summary>
		/// Clears all the member variables of this AllegroBitmap, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		AllegroBitmap & operator=(const AllegroBitmap &rhs) = delete;
	};
};
#endif