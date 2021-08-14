#include "SLBackground.h"
#include "FrameMan.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(SLBackground, SceneLayer, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Clear() {
		m_Bitmaps.clear();
		m_FrameCount = 1;
		m_Frame = 0;
		m_SpriteAnimMode = SpriteAnimMode::NOANIM;
		m_SpriteAnimDuration = 1000;
		m_SpriteAnimIsReversingFrames = false;
		m_SpriteAnimTimer.Reset();
		m_AutoScrollX = false;
		m_AutoScrollY = false;
		m_AutoScrollStep.Reset();
		m_AutoScrollStepInterval = 0;
		m_AutoScrollStepTimer.Reset();
		m_AutoScrollOffset.Reset();

		m_LayerScaleFactors = { Vector(1.0F, 1.0F), Vector(), Vector(2.0F, 2.0F) };
		m_IgnoreAutoScale = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Create() {
		SceneLayer::Create();

		m_BitmapFile.GetAsAnimation(m_Bitmaps, m_FrameCount);
		m_MainBitmap = m_Bitmaps.at(0);

		InitScaleFactors();

		// Sampled color at the edges of the layer that can be used to fill gap if the layer isn't large enough to cover a target bitmap.
		if (!m_WrapX) {
			m_FillLeftColor = _getpixel(m_MainBitmap, 0, m_MainBitmap->h / 2);
			m_FillRightColor = _getpixel(m_MainBitmap, m_MainBitmap->w - 1, m_MainBitmap->h / 2);
		}
		if (!m_WrapY) {
			m_FillUpColor = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, 0);
			m_FillDownColor = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, m_MainBitmap->h - 1);
		}
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

		m_FillLeftColor = reference.m_FillLeftColor;
		m_FillRightColor = reference.m_FillRightColor;
		m_FillUpColor = reference.m_FillUpColor;
		m_FillDownColor = reference.m_FillDownColor;

		m_FrameCount = reference.m_FrameCount;
		m_SpriteAnimMode = reference.m_SpriteAnimMode;
		m_SpriteAnimDuration = reference.m_SpriteAnimDuration;

		m_AutoScrollX = reference.m_AutoScrollX;
		m_AutoScrollY = reference.m_AutoScrollY;
		m_AutoScrollStep = reference.m_AutoScrollStep;
		m_AutoScrollStepInterval = reference.m_AutoScrollStepInterval;

		m_LayerScaleFactors = reference.m_LayerScaleFactors;
		m_IgnoreAutoScale = reference.m_IgnoreAutoScale;

		InitScaleFactors();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::InitScaleFactors() {
		m_LayerScaleFactors.at(LayerAutoScaleMode::AutoScaleOff) = m_ScaleFactor;

		float fitScreenScaleFactor = std::clamp(static_cast<float>(g_FrameMan.GetPlayerScreenHeight()) / static_cast<float>(m_MainBitmap->h), 1.0F, 2.0F);
		m_LayerScaleFactors.at(LayerAutoScaleMode::FitScreen).SetXY(fitScreenScaleFactor, fitScreenScaleFactor);

		if (!m_IgnoreAutoScale) {
			switch (g_SettingsMan.GetSceneBackgroundAutoScaleMode()) {
				case LayerAutoScaleMode::FitScreen:
					SetScaleFactor(m_LayerScaleFactors.at(LayerAutoScaleMode::FitScreen));
					break;
				case LayerAutoScaleMode::AlwaysUpscaled:
					SetScaleFactor(m_LayerScaleFactors.at(LayerAutoScaleMode::AlwaysUpscaled));
					break;
				default:
					SetScaleFactor(m_LayerScaleFactors.at(LayerAutoScaleMode::AutoScaleOff));
					break;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "FrameCount") {
			reader >> m_FrameCount;
		} else if (propName == "SpriteAnimMode") {
			reader >> m_SpriteAnimMode;
			if (m_FrameCount > 1) {
				// If animation mode is set to something other than ALWAYSLOOP but only has 2 frames, override it because it's pointless
				if ((m_SpriteAnimMode == SpriteAnimMode::ALWAYSRANDOM || m_SpriteAnimMode == SpriteAnimMode::ALWAYSPINGPONG) && m_FrameCount == 2) { m_SpriteAnimMode = SpriteAnimMode::ALWAYSLOOP; }
			} else {
				m_SpriteAnimMode = SpriteAnimMode::NOANIM;
			}
		} else if (propName == "SpriteAnimDuration") {
			reader >> m_SpriteAnimDuration;
		} else if (propName == "AutoScrollX") {
			reader >> m_AutoScrollX;
		} else if (propName == "AutoScrollY") {
			reader >> m_AutoScrollY;
		} else if (propName == "AutoScrollStepInterval") {
			reader >> m_AutoScrollStepInterval;
		} else if (propName == "AutoScrollStep") {
			reader >> m_AutoScrollStep;
		} else if (propName == "IgnoreAutoScaling") {
			reader >> m_IgnoreAutoScale;
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
		writer.NewPropertyWithValue("AutoScrollX", m_AutoScrollX);
		writer.NewPropertyWithValue("AutoScrollY", m_AutoScrollY);
		writer.NewPropertyWithValue("AutoScrollStepInterval", m_AutoScrollStepInterval);
		writer.NewPropertyWithValue("AutoScrollStep", m_AutoScrollStep);
		writer.NewPropertyWithValue("IgnoreAutoScaling", m_IgnoreAutoScale);

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
		if (IsAutoScrolling() && m_AutoScrollStepTimer.GetElapsedSimTimeMS() > m_AutoScrollStepInterval) {
			m_AutoScrollOffset += m_AutoScrollStep;
			WrapPosition(m_AutoScrollOffset);
			m_AutoScrollStepTimer.Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {
		Vector ratioAdjustedAutoScrolledOffset;
		if (IsAutoScrolling()) {
			ratioAdjustedAutoScrolledOffset.SetXY(std::floor((m_Offset.GetX() * m_ScrollRatio.GetX()) + m_AutoScrollOffset.GetX()), std::floor((m_Offset.GetY() * m_ScrollRatio.GetY()) + m_AutoScrollOffset.GetY()));
			WrapPosition(ratioAdjustedAutoScrolledOffset);
		}
		m_MainBitmap = m_Bitmaps.at(m_Frame);
		SceneLayer::Draw(targetBitmap, targetBox, IsAutoScrolling() ? ratioAdjustedAutoScrolledOffset : scrollOverride);
	}
}