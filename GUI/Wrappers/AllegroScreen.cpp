#include "GUI.h"
#include "AllegroScreen.h"

#include "allegro.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIBitmap * AllegroScreen::CreateBitmap(const std::string &fileName) {
		std::unique_ptr<AllegroBitmap> newAllegroBitmap;
		newAllegroBitmap.reset(new AllegroBitmap());

		newAllegroBitmap->Create(fileName);
		return newAllegroBitmap.release();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIBitmap * AllegroScreen::CreateBitmap(int width, int height) {
		std::unique_ptr<AllegroBitmap> newAllegroBitmap;
		newAllegroBitmap.reset(new AllegroBitmap());

		newAllegroBitmap->Create(width, height, bitmap_color_depth(m_BackBufferBitmap->GetBitmap()));
		return newAllegroBitmap.release();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroScreen::DrawBitmap(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) {
		if (!guiBitmap) {
			return;
		}
		if (BITMAP *sourceBitmap = dynamic_cast<AllegroBitmap *>(guiBitmap)->GetBitmap()) {
			if (srcPosAndSizeRect) {
				blit(sourceBitmap, m_BackBufferBitmap->GetBitmap(), srcPosAndSizeRect->left, srcPosAndSizeRect->top, destX, destY, srcPosAndSizeRect->right - srcPosAndSizeRect->left, srcPosAndSizeRect->bottom - srcPosAndSizeRect->top);
			} else {
				blit(sourceBitmap, m_BackBufferBitmap->GetBitmap(), 0, 0, destX, destY, sourceBitmap->w, sourceBitmap->h);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroScreen::DrawBitmapTrans(GUIBitmap *guiBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) {
		if (!guiBitmap) {
			return;
		}
		if (BITMAP *sourceBitmap = dynamic_cast<AllegroBitmap *>(guiBitmap)->GetBitmap()) {
			if (srcPosAndSizeRect) {
				masked_blit(sourceBitmap, m_BackBufferBitmap->GetBitmap(), srcPosAndSizeRect->left, srcPosAndSizeRect->top, destX, destY, srcPosAndSizeRect->right - srcPosAndSizeRect->left, srcPosAndSizeRect->bottom - srcPosAndSizeRect->top);
			} else {
				masked_blit(sourceBitmap, m_BackBufferBitmap->GetBitmap(), 0, 0, destX, destY, sourceBitmap->w, sourceBitmap->h);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned long AllegroScreen::ConvertColor(unsigned long color, int targetColorDepth) {
		if (targetColorDepth == 0) { targetColorDepth = get_color_depth(); }

		if (targetColorDepth == 8) {
			// Isn't indexed, don't convert
			if (!(color >= 0 && color <= 255)) { color = makecol8(getr32(color), getg32(color), getb32(color)); }
		} else {
			if (color >= 0 && color <= 255) {
				RGB rgbEntry;
				get_color(color, &rgbEntry);
				// Times 4 because RGB struct's elements are in range 0-63, and makecol needs 0-255.
				color = makecol(rgbEntry.r * 4, rgbEntry.g * 4, rgbEntry.b * 4);
			}
		}
		return color;
	}
}