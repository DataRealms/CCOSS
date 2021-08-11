#include "SLBackground.h"

namespace RTE {

	ConcreteClassInfo(SLBackground, SceneLayer, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Clear() {
		m_Bitmaps.clear();
		m_FrameCount = 1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Create() {
		m_BitmapFile.GetAsAnimation(m_Bitmaps, m_FrameCount);
		m_MainBitmap = m_Bitmaps.at(0);
		// Sampled color at the edges of the layer that can be used to fill gap if the layer isn't large enough to cover a target bitmap.
		m_FillLeftColor = m_WrapX ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, 0, m_MainBitmap->h / 2);
		m_FillRightColor = m_WrapX ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w - 1, m_MainBitmap->h / 2);
		m_FillUpColor = m_WrapY ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w / 2, 0);
		m_FillDownColor = m_WrapY ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w / 2, m_MainBitmap->h - 1);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Create(const SLBackground &reference) {
		SceneLayer::Create(reference);

		// The main bitmap is created and owned by SceneLayer because it can be modified. We need to destroy it to avoid a leak because the bitmaps we'll be using here are owned by ContentFile static maps and are unmodifiable.
		destroy_bitmap(m_MainBitmap);
		m_MainBitmapOwned = false;

		m_Bitmaps.clear();
		m_Bitmaps = reference.m_Bitmaps;
		m_MainBitmap = m_Bitmaps.at(0);

		m_FrameCount = reference.m_FrameCount;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::ReadProperty(const std::string_view &propName, Reader &reader) {
			return SceneLayer::ReadProperty(propName, reader);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Save(Writer &writer) const {
		SceneLayer::Save(writer);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Update() {

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {

		SceneLayer::Draw(targetBitmap, targetBox, scrollOverride);
	}
}