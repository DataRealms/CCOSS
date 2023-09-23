#include "SLBackground.h"
#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(SLBackground, SceneLayer, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Clear() {
		m_Bitmaps.clear();
		m_FrameCount = 1;
		m_Frame = 0;
		m_SpriteAnimMode = SpriteAnimMode::NOANIM;
		m_SpriteAnimDuration = 1000;
		m_SpriteAnimIsReversingFrames = false;
		m_SpriteAnimTimer.Reset();
		m_IsAnimatedManually = false;
		m_CanAutoScrollX = false;
		m_CanAutoScrollY = false;
		m_AutoScrollStep.Reset();
		m_AutoScrollStepInterval = 0;
		m_AutoScrollStepTimer.Reset();
		m_AutoScrollOffset.Reset();
		m_FillColorLeft = ColorKeys::g_MaskColor;
		m_FillColorRight = ColorKeys::g_MaskColor;
		m_FillColorUp = ColorKeys::g_MaskColor;
		m_FillColorDown = ColorKeys::g_MaskColor;

		m_IgnoreAutoScale = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Create() {
		SceneLayer::Create();

		m_Bitmaps.clear();
		m_BitmapFile.GetAsAnimation(m_Bitmaps, m_FrameCount);
		m_MainBitmap = m_Bitmaps[0];

		if (m_FrameCount == 1) {
			m_SpriteAnimMode = SpriteAnimMode::NOANIM;
		} else if (m_FrameCount == 2 && m_SpriteAnimMode != SpriteAnimMode::NOANIM) {
			m_SpriteAnimMode = SpriteAnimMode::ALWAYSLOOP;
		}

		if (!m_WrapX) {
			m_FillColorLeft = _getpixel(m_MainBitmap, 0, m_MainBitmap->h / 2);
			m_FillColorRight = _getpixel(m_MainBitmap, m_MainBitmap->w - 1, m_MainBitmap->h / 2);
		}
		if (!m_WrapY) {
			m_FillColorUp = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, 0);
			m_FillColorDown = _getpixel(m_MainBitmap, m_MainBitmap->w / 2, m_MainBitmap->h - 1);
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
		m_MainBitmap = m_Bitmaps[0];

		m_FillColorLeft = reference.m_FillColorLeft;
		m_FillColorRight = reference.m_FillColorRight;
		m_FillColorUp = reference.m_FillColorUp;
		m_FillColorDown = reference.m_FillColorDown;

		m_FrameCount = reference.m_FrameCount;
		m_SpriteAnimMode = reference.m_SpriteAnimMode;
		m_SpriteAnimDuration = reference.m_SpriteAnimDuration;

		m_CanAutoScrollX = reference.m_CanAutoScrollX;
		m_CanAutoScrollY = reference.m_CanAutoScrollY;
		m_AutoScrollStep = reference.m_AutoScrollStep;
		m_AutoScrollStepInterval = reference.m_AutoScrollStepInterval;

		m_IgnoreAutoScale = reference.m_IgnoreAutoScale;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return SceneLayer::ReadProperty(propName, reader));
		
		MatchProperty("FrameCount", { reader >> m_FrameCount; });
		MatchProperty("SpriteAnimMode", {
			m_SpriteAnimMode = static_cast<SpriteAnimMode>(std::stoi(reader.ReadPropValue()));
			if (m_SpriteAnimMode < SpriteAnimMode::NOANIM || m_SpriteAnimMode > SpriteAnimMode::ALWAYSPINGPONG) { reader.ReportError("Invalid SLBackground sprite animation mode!"); }
		});
		MatchProperty("SpriteAnimDuration", { reader >> m_SpriteAnimDuration; });
		MatchProperty("IsAnimatedManually", { reader >> m_IsAnimatedManually; });
		MatchProperty("DrawTransparent", { reader >> m_DrawMasked; });
		MatchProperty("ScrollRatio", {
			// Actually read the ScrollInfo, not the ratio. The ratios will be initialized later.
			reader >> m_ScrollInfo;
		});
		MatchProperty("ScaleFactor", {
			reader >> m_ScaleFactor;
			SetScaleFactor(m_ScaleFactor);
		});
		MatchProperty("IgnoreAutoScaling", { reader >> m_IgnoreAutoScale; });
		MatchProperty("OriginPointOffset", { reader >> m_OriginOffset; });
		MatchProperty("CanAutoScrollX", { reader >> m_CanAutoScrollX; });
		MatchProperty("CanAutoScrollY", { reader >> m_CanAutoScrollY; });
		MatchProperty("AutoScrollStepInterval", { reader >> m_AutoScrollStepInterval; });
		MatchProperty("AutoScrollStep", { reader >> m_AutoScrollStep; });
		
		
		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SLBackground::Save(Writer &writer) const {
		SceneLayer::Save(writer);

		writer.NewPropertyWithValue("FrameCount", m_FrameCount);
		writer.NewPropertyWithValue("SpriteAnimMode", m_SpriteAnimMode);
		writer.NewPropertyWithValue("SpriteAnimDuration", m_SpriteAnimDuration);
		writer.NewPropertyWithValue("IsAnimatedManually", m_IsAnimatedManually);
		writer.NewPropertyWithValue("DrawTransparent", m_DrawMasked);
		writer.NewPropertyWithValue("ScrollRatio", m_ScrollInfo);
		writer.NewPropertyWithValue("ScaleFactor", m_ScaleFactor);
		writer.NewPropertyWithValue("IgnoreAutoScaling", m_IgnoreAutoScale);
		writer.NewPropertyWithValue("OriginPointOffset", m_OriginOffset);
		writer.NewPropertyWithValue("CanAutoScrollX", m_CanAutoScrollX);
		writer.NewPropertyWithValue("CanAutoScrollY", m_CanAutoScrollY);
		writer.NewPropertyWithValue("AutoScrollStepInterval", m_AutoScrollStepInterval);
		writer.NewPropertyWithValue("AutoScrollStep", m_AutoScrollStep);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::InitScaleFactors() {
		if (!m_IgnoreAutoScale) {
			float fitScreenScaleFactor = std::clamp(static_cast<float>(std::min(g_SceneMan.GetSceneHeight(), g_FrameMan.GetPlayerScreenHeight())) / static_cast<float>(m_MainBitmap->h), 1.0F, 2.0F);

			switch (g_SettingsMan.GetSceneBackgroundAutoScaleMode()) {
				case LayerAutoScaleMode::FitScreen:
					SetScaleFactor(Vector(fitScreenScaleFactor, fitScreenScaleFactor));
					break;
				case LayerAutoScaleMode::AlwaysUpscaled:
					SetScaleFactor(Vector(2.0F, 2.0F));
					break;
				default:
					SetScaleFactor(m_ScaleFactor);
					break;
			}
			m_ScrollInfo *= m_ScaleFactor;
			InitScrollRatios();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Update() {
		if (!m_IsAnimatedManually && m_SpriteAnimMode != SpriteAnimMode::NOANIM) {
			int prevFrame = m_Frame;

			if (m_SpriteAnimTimer.GetElapsedSimTimeMS() > (m_SpriteAnimDuration / m_FrameCount)) {
				switch (m_SpriteAnimMode) {
					case SpriteAnimMode::ALWAYSLOOP:
						m_Frame = (m_Frame + 1) % m_FrameCount;
						break;
					case SpriteAnimMode::ALWAYSRANDOM:
						while (m_Frame == prevFrame) {
							m_Frame = RandomNum(0, m_FrameCount - 1);
						}
						break;
					case SpriteAnimMode::ALWAYSPINGPONG:
						if (m_Frame == m_FrameCount - 1) {
							m_SpriteAnimIsReversingFrames = true;
						} else if (m_Frame == 0) {
							m_SpriteAnimIsReversingFrames = false;
						}
						m_SpriteAnimIsReversingFrames ? m_Frame-- : m_Frame++;
						break;
					default:
						break;
				}
				m_SpriteAnimTimer.Reset();
			}
		}
		m_MainBitmap = m_Bitmaps.at(m_Frame);

		if (IsAutoScrolling()) {
			if (m_AutoScrollStepTimer.GetElapsedSimTimeMS() > m_AutoScrollStepInterval) {
				if (m_WrapX && m_CanAutoScrollX) { m_AutoScrollOffset.SetX(m_AutoScrollOffset.GetX() + m_AutoScrollStep.GetX()); }
				if (m_WrapY && m_CanAutoScrollY) { m_AutoScrollOffset.SetY(m_AutoScrollOffset.GetY() + m_AutoScrollStep.GetY()); }
				WrapPosition(m_AutoScrollOffset);
				m_AutoScrollStepTimer.Reset();
			}
			m_Offset.SetXY(std::floor((m_Offset.GetX() * m_ScrollRatio.GetX()) + m_AutoScrollOffset.GetX()), std::floor((m_Offset.GetY() * m_ScrollRatio.GetY()) + m_AutoScrollOffset.GetY()));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SLBackground::Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment) {
		SceneLayer::Draw(targetBitmap, targetBox, !IsAutoScrolling());

		int bitmapWidth = m_ScaledDimensions.GetFloorIntX();
		int bitmapHeight = m_ScaledDimensions.GetFloorIntY();
		int targetBoxCornerX = targetBox.GetCorner().GetFloorIntX();
		int targetBoxCornerY = targetBox.GetCorner().GetFloorIntY();
		int targetBoxWidth = static_cast<int>(targetBox.GetWidth());
		int targetBoxHeight = static_cast<int>(targetBox.GetHeight());

		set_clip_rect(targetBitmap, targetBoxCornerX, targetBoxCornerY, targetBoxCornerX + targetBoxWidth - 1, targetBoxCornerY + targetBoxHeight - 1);

		// Detect if non-wrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge.
		if (!m_WrapX && bitmapWidth <= targetBoxWidth) {
			if (m_FillColorLeft != ColorKeys::g_MaskColor && m_Offset.GetFloorIntX() != 0) { rectfill(targetBitmap, targetBoxCornerX, targetBoxCornerY, targetBoxCornerX - m_Offset.GetFloorIntX(), targetBoxCornerY + targetBoxHeight, m_FillColorLeft); }
			if (m_FillColorRight != ColorKeys::g_MaskColor) { rectfill(targetBitmap, targetBoxCornerX + bitmapWidth - m_Offset.GetFloorIntX(), targetBoxCornerY, targetBoxCornerX + targetBoxWidth, targetBoxCornerY + targetBoxHeight, m_FillColorRight); }
		}
		if (!m_WrapY && bitmapHeight <= targetBoxHeight) {
			if (m_FillColorUp != ColorKeys::g_MaskColor && m_Offset.GetFloorIntY() != 0) { rectfill(targetBitmap, targetBoxCornerX, targetBoxCornerY, targetBoxCornerX + targetBoxWidth, targetBoxCornerY - m_Offset.GetFloorIntY(), m_FillColorUp); }
			if (m_FillColorDown != ColorKeys::g_MaskColor) { rectfill(targetBitmap, targetBoxCornerX, targetBoxCornerY + bitmapHeight - m_Offset.GetFloorIntY(), targetBoxCornerX + targetBoxWidth, targetBoxCornerY + targetBoxHeight, m_FillColorDown); }
		}
		set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
	}
}