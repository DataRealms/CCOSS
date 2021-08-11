#include "SLBackground.h"

namespace RTE {

	ConcreteClassInfo(SLBackground, SceneLayer, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Clear() {
		m_Bitmaps.clear();
		m_FrameCount = 1;
		m_Frame = 0;
		m_SpriteAnimDuration = 0;
		m_SpriteAnimMode = SpriteAnimMode::NOANIM;
		m_SpriteAnimTimer.Reset();
		m_SpriteAnimIsReversingFrames = false;
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
		m_SpriteAnimMode = reference.m_SpriteAnimMode;
		m_SpriteAnimDuration = reference.m_SpriteAnimDuration;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "FrameCount") {
			reader >> m_FrameCount;
		} else if (propName == "SpriteAnimMode") {
			reader >> m_SpriteAnimMode;
			if (m_FrameCount > 1) {
				// If animation mode is set to something other than ALWAYSLOOP but only has 2 frames, override it because it's pointless
				if ((m_SpriteAnimMode == SpriteAnimMode::ALWAYSRANDOM || m_SpriteAnimMode == SpriteAnimMode::ALWAYSPINGPONG) && m_FrameCount == 2) { m_SpriteAnimMode = ALWAYSLOOP; }
			} else {
				m_SpriteAnimMode = SpriteAnimMode::NOANIM;
			}
		} else if (propName == "SpriteAnimDuration") {
			reader >> m_SpriteAnimDuration;
		} else {
			return SceneLayer::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Save(Writer &writer) const {
		SceneLayer::Save(writer);

		writer.NewPropertyWithValue("FrameCount", m_FrameCount);
		writer.NewPropertyWithValue("SpriteAnimMode", m_SpriteAnimMode);
		writer.NewPropertyWithValue("SpriteAnimDuration", m_SpriteAnimDuration);
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Update() {
		if (m_SpriteAnimMode != SpriteAnimMode::NOANIM) {
			int frameTime = m_SpriteAnimDuration / m_FrameCount;
			int prevFrame = m_Frame;

			if (m_SpriteAnimTimer.GetElapsedSimTimeMS() > frameTime) {
				switch (m_SpriteAnimMode) {
					case SpriteAnimMode::ALWAYSLOOP:
						m_Frame = ((m_Frame + 1) % m_FrameCount);
						m_SpriteAnimTimer.Reset();
						break;
					case SpriteAnimMode::ALWAYSRANDOM:
						while (m_Frame == prevFrame) {
							m_Frame = RandomNum<int>(0, m_FrameCount - 1);
						}
						m_SpriteAnimTimer.Reset();
						break;
					case SpriteAnimMode::ALWAYSPINGPONG:
						if (m_Frame == m_FrameCount - 1) {
							m_SpriteAnimIsReversingFrames = true;
						} else if (m_Frame == 0) {
							m_SpriteAnimIsReversingFrames = false;
						}
						m_SpriteAnimIsReversingFrames ? m_Frame-- : m_Frame++;
						m_SpriteAnimTimer.Reset();
						break;
					default:
						break;
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {
		m_MainBitmap = m_Bitmaps.at(m_Frame);

		SceneLayer::Draw(targetBitmap, targetBox, scrollOverride);
	}
}