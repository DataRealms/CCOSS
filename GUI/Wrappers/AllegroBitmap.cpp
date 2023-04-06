#include "GUI.h"
#include "AllegroBitmap.h"
#include "RTEError.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::Clear() {
		m_Bitmap = nullptr;
		m_BitmapFile.Reset();
		m_SelfCreated = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::Create(const std::string &fileName) {
		m_BitmapFile.Create(fileName.c_str());
		m_Bitmap = m_BitmapFile.GetAsBitmap();

		RTEAssert(m_Bitmap, "Could not load bitmap from \"" + fileName + "\" into AllegroBitmap!");

		m_SelfCreated = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::Create(int width, int height, int colorDepth) {
		m_BitmapFile.Reset();
		m_Bitmap = create_bitmap_ex(colorDepth, width, height);

		RTEAssert(m_Bitmap, "Could not create new bitmap in AllegroBitmap!");

		clear_to_color(m_Bitmap, 0);
		m_SelfCreated = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::Destroy() {
		if (m_SelfCreated && m_Bitmap) { destroy_bitmap(m_Bitmap); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AllegroBitmap::GetWidth() const {
		return m_Bitmap ? m_Bitmap->w : 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AllegroBitmap::GetHeight() const {
		return m_Bitmap ? m_Bitmap->h : 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AllegroBitmap::GetColorDepth() const {
		return m_Bitmap ? bitmap_color_depth(m_Bitmap) : 8;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned long AllegroBitmap::GetPixel(int posX, int posY) const {
		return m_Bitmap ? getpixel(m_Bitmap, posX, posY) : 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::SetPixel(int posX, int posY, unsigned long pixelColor) {
		RTEAssert(m_Bitmap, "Trying to set a pixel on a null bitmap!");
		putpixel(m_Bitmap, posX, posY, pixelColor);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::GetClipRect(GUIRect *clippingRect) const {
		if (m_Bitmap && clippingRect) {
			int x1;
			int y1;
			int x2;
			int y2;
			get_clip_rect(m_Bitmap, &x1, &y1, &x2, &y2);
			clippingRect->left = x1;
			clippingRect->top = y1;
			clippingRect->right = x2;
			clippingRect->bottom = y2;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::SetClipRect(GUIRect *clippingRect) {
		if (!m_Bitmap) {
			return;
		}
		set_clip_state(m_Bitmap, 1);

		if (!clippingRect) {
			set_clip_rect(m_Bitmap, 0, 0, m_Bitmap->w - 1, m_Bitmap->h - 1);
		} else {
			set_clip_rect(m_Bitmap, clippingRect->left, clippingRect->top, clippingRect->right, clippingRect->bottom);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::AddClipRect(GUIRect *clippingRect) {
		if (!m_Bitmap) {
			return;
		}
		set_clip_state(m_Bitmap, 1);

		if (!clippingRect) {
			set_clip_rect(m_Bitmap, 0, 0, m_Bitmap->w - 1, m_Bitmap->h - 1);
		} else {
			add_clip_rect(m_Bitmap, clippingRect->left, clippingRect->top, clippingRect->right, clippingRect->bottom);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::Draw(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) {
		if (!m_Bitmap) {
			return;
		}
		RTEAssert(destBitmap && dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

		if (srcPosAndSizeRect) {
			blit(m_Bitmap, dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), srcPosAndSizeRect->left, srcPosAndSizeRect->top, destX, destY, srcPosAndSizeRect->right - srcPosAndSizeRect->left, srcPosAndSizeRect->bottom - srcPosAndSizeRect->top);
		} else {
			blit(m_Bitmap, dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), 0, 0, destX, destY, destBitmap->GetWidth(), destBitmap->GetHeight());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::DrawTrans(GUIBitmap *destBitmap, int destX, int destY, GUIRect *srcPosAndSizeRect) {
		if (!m_Bitmap) {
			return;
		}
		RTEAssert(destBitmap && dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

		if (srcPosAndSizeRect) {
			masked_blit(m_Bitmap, dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), srcPosAndSizeRect->left, srcPosAndSizeRect->top, destX, destY, srcPosAndSizeRect->right - srcPosAndSizeRect->left, srcPosAndSizeRect->bottom - srcPosAndSizeRect->top);
		} else {
			masked_blit(m_Bitmap, dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), 0, 0, destX, destY, destBitmap->GetWidth(), destBitmap->GetHeight());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::DrawTransScaled(GUIBitmap *destBitmap, int destX, int destY, int width, int height) {
		if (!m_Bitmap) {
			return;
		}
		RTEAssert(destBitmap && dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

		stretch_sprite(dynamic_cast<AllegroBitmap *>(destBitmap)->GetBitmap(), m_Bitmap, destX, destY, width, height);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::DrawLine(int x1, int y1, int x2, int y2, unsigned long color) {
		if (!m_Bitmap) {
			return;
		}
		line(m_Bitmap, x1, y1, x2, y2, color);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AllegroBitmap::DrawRectangle(int posX, int posY, int width, int height, unsigned long color, bool filled) {
		if (!m_Bitmap) {
			return;
		}
		if (filled) {
			rectfill(m_Bitmap, posX, posY, posX + width - 1, posY + height - 1, color);
		} else {
			rect(m_Bitmap, posX, posY, posX + width - 1, posY + height - 1, color);
		}
	}
}