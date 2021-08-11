#include "SceneLayer.h"
#include "ContentFile.h"

#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(SceneLayer, Entity, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::Clear() {
		m_BitmapFile.Reset();
		m_MainBitmap = nullptr;
		m_MainBitmapOwned = false;
		m_DrawTrans = true;
		m_Offset.Reset();
		m_ScrollInfo.SetXY(1.0F, 1.0F);
		m_ScrollRatio.SetXY(1.0F, 1.0F);
		m_ScaleFactor.SetXY(1.0F, 1.0F);
		m_ScaleInverse.SetXY(1.0F, 1.0F);
		m_ScaledDimensions.SetXY(1.0F, 1.0F);
		m_WrapX = true;
		m_WrapY = true;
		m_FillLeftColor = ColorKeys::g_MaskColor;
		m_FillRightColor = ColorKeys::g_MaskColor;
		m_FillUpColor = ColorKeys::g_MaskColor;
		m_FillDownColor = ColorKeys::g_MaskColor;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::Create(const ContentFile &bitmapFile, bool drawTrans, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo) {
		m_BitmapFile = bitmapFile;
		m_MainBitmap = m_BitmapFile.GetAsBitmap();
		Create(m_MainBitmap, drawTrans, offset, wrapX, wrapY, scrollInfo);
		m_MainBitmapOwned = false;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::Create(BITMAP *bitmap, bool drawTrans, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo) {
		m_MainBitmap = bitmap;
		RTEAssert(m_MainBitmap, "Null bitmap passed in when creating SceneLayer");

		m_MainBitmapOwned = true;

		m_DrawTrans = drawTrans;
		m_Offset = offset;
		m_WrapX = wrapX;
		m_WrapY = wrapY;
		m_ScrollInfo = scrollInfo;

		InitScrollRatios();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::Create(const SceneLayer &reference) {
		Entity::Create(reference);

		m_BitmapFile = reference.m_BitmapFile;
		m_DrawTrans = reference.m_DrawTrans;
		m_Offset = reference.m_Offset;
		m_WrapX = reference.m_WrapX;
		m_WrapY = reference.m_WrapY;
		m_ScrollInfo = reference.m_ScrollInfo;
		// The ratios need to be calculated from the ScrollInfo with InitScrollRatios later in LoadData.
		m_ScrollRatio = reference.m_ScrollRatio;
		m_ScaleFactor = reference.m_ScaleFactor;
		m_ScaleInverse = reference.m_ScaleInverse;
		m_ScaledDimensions = reference.m_ScaledDimensions;
		m_FillLeftColor = reference.m_FillLeftColor;
		m_FillRightColor = reference.m_FillRightColor;
		m_FillUpColor = reference.m_FillUpColor;
		m_FillDownColor = reference.m_FillDownColor;

		if (reference.m_MainBitmap) {
			// Copy the bitmap from the ContentFile, because we're going to be changing it!
			BITMAP *bitmapToCopy = reference.m_MainBitmap;
			RTEAssert(bitmapToCopy, "Couldn't load the bitmap file specified for SceneLayer!");

			m_MainBitmap = create_bitmap_ex(8, bitmapToCopy->w, bitmapToCopy->h);
			RTEAssert(m_MainBitmap, "Failed to allocate BITMAP in SceneLayer::Create");
			blit(bitmapToCopy, m_MainBitmap, 0, 0, 0, 0, bitmapToCopy->w, bitmapToCopy->h);

			InitScrollRatios();

			m_MainBitmapOwned = true;
		} else {
			// If no bitmap to copy, has to load it with LoadData to create this in the copied SL.
			m_MainBitmapOwned = false;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::InitScrollRatios(bool initForNetworkPlayer, int player) {
		float mainBitmapWidth = static_cast<float>(m_MainBitmap->w);
		float mainBitmapHeight = static_cast<float>(m_MainBitmap->h);
		float playerScreenWidth = static_cast<float>(initForNetworkPlayer ? g_FrameMan.GetPlayerFrameBufferWidth(player) : g_FrameMan.GetPlayerScreenWidth());
		float playerScreenHeight = static_cast<float>(initForNetworkPlayer ? g_FrameMan.GetPlayerFrameBufferHeight(player) : g_FrameMan.GetPlayerScreenHeight());

		if (m_WrapX) {
			m_ScrollRatio.SetX(m_ScrollInfo.GetX());
		} else {
			if (m_ScrollInfo.GetX() == -1.0F || m_ScrollInfo.GetX() == 1.0F) {
				m_ScrollRatio.SetX(1.0F);
			} else if (m_ScrollInfo.GetX() == playerScreenWidth) {
				m_ScrollRatio.SetX(mainBitmapWidth - playerScreenWidth);
			} else if (mainBitmapWidth == playerScreenWidth) {
				m_ScrollRatio.SetX(1.0F / (m_ScrollInfo.GetX() - playerScreenWidth));
			} else {
				m_ScrollRatio.SetX((mainBitmapWidth - playerScreenWidth) / (m_ScrollInfo.GetX() - playerScreenWidth));
			}
		}
		if (m_WrapY) {
			m_ScrollRatio.SetY(m_ScrollInfo.GetY());
		} else {
			if (m_ScrollInfo.GetY() == -1.0F || m_ScrollInfo.GetY() == 1.0) {
				m_ScrollRatio.SetY(1.0F);
			} else if (m_ScrollInfo.GetY() == playerScreenHeight) {
				m_ScrollRatio.SetY(mainBitmapHeight - playerScreenHeight);
			} else if (mainBitmapHeight == playerScreenHeight) {
				m_ScrollRatio.SetY(1.0F / (m_ScrollInfo.GetY() - playerScreenHeight));
			} else {
				m_ScrollRatio.SetY((mainBitmapHeight - playerScreenHeight) / (m_ScrollInfo.GetY() - playerScreenHeight));
			}
		}
		m_ScaledDimensions.SetXY(mainBitmapWidth * m_ScaleFactor.GetX(), mainBitmapHeight * m_ScaleFactor.GetY());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "BitmapFile") {
			reader >> m_BitmapFile;
		} else if (propName == "DrawTransparent") {
			reader >> m_DrawTrans;
		} else if (propName == "Offset") {
			reader >> m_Offset;
		} else if (propName == "WrapX") {
			reader >> m_WrapX;
		} else if (propName == "WrapY") {
			reader >> m_WrapY;
		} else if (propName == "ScrollRatio") {
			// Actually read the ScrollInfo, not the ratio. The ratios will be initialized later.
			reader >> m_ScrollInfo;
		} else if (propName == "ScaleFactor") {
			reader >> m_ScaleFactor;
			SetScaleFactor(m_ScaleFactor);
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("BitmapFile", m_BitmapFile);
		writer.NewPropertyWithValue("DrawTransparent", m_DrawTrans);
		writer.NewPropertyWithValue("Offset", m_Offset);
		writer.NewPropertyWithValue("WrapX", m_WrapX);
		writer.NewPropertyWithValue("WrapY", m_WrapY);
		writer.NewPropertyWithValue("ScrollRatio", m_ScrollInfo);
		writer.NewPropertyWithValue("ScaleFactor", m_ScaleFactor);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::Destroy(bool notInherited) {
		if (m_MainBitmapOwned) { destroy_bitmap(m_MainBitmap); }
		if (!notInherited) { Entity::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::LoadData() {
		// Re-load directly from disk each time; don't do any caching of these bitmaps
		m_MainBitmap = m_BitmapFile.GetAsBitmap(COLORCONV_NONE, false);

		m_MainBitmapOwned = true;

		InitScrollRatios();

		// Sampled color at the edges of the layer that can be used to fill gap if the layer isn't large enough to cover a target bitmap
		m_FillLeftColor = m_WrapX ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, 0, m_MainBitmap->h / 2);
		m_FillRightColor = m_WrapX ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w - 1, m_MainBitmap->h / 2);
		m_FillUpColor = m_WrapY ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w / 2, 0);
		m_FillDownColor = m_WrapY ? ColorKeys::g_MaskColor : _getpixel(m_MainBitmap, m_MainBitmap->w / 2, m_MainBitmap->h - 1);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::SaveData(const std::string &bitmapPath) {
		if (bitmapPath.empty()) {
			return -1;
		}
		if (m_MainBitmap) {
			PALETTE palette;
			get_palette(palette);
			if (save_png(bitmapPath.c_str(), m_MainBitmap, palette) != 0) {
				return -1;
			}
			// Set the new path to point to the new file location - only if there was a successful save of the bitmap
			m_BitmapFile.SetDataPath(bitmapPath);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::ClearData() {
		if (m_MainBitmap && m_MainBitmapOwned) { destroy_bitmap(m_MainBitmap); }
		m_MainBitmap = nullptr;
		m_MainBitmapOwned = false;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::SetScaleFactor(const Vector &newScale) {
		m_ScaleFactor = newScale;
		m_ScaleInverse.SetXY(1.0F / newScale.GetX(), 1.0F / newScale.GetY());
		if (m_MainBitmap) { m_ScaledDimensions.SetXY(static_cast<float>(m_MainBitmap->w) * newScale.GetX(), static_cast<float>(m_MainBitmap->h) * newScale.GetY()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::GetPixel(const int pixelX, const int pixelY) {
		return (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) ? 0 : _getpixel(m_MainBitmap, pixelX, pixelY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::SetPixel(const int pixelX, const int pixelY, const int value) {
		RTEAssert(m_MainBitmapOwned, "Trying to set a pixel of a SceneLayer's bitmap which isn't owned!");

		if (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) {
			return;
		}
		putpixel(m_MainBitmap, pixelX, pixelY, value);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneLayer::IsWithinBounds(const int pixelX, const int pixelY, const int margin) const {
		return (m_WrapX || (pixelX >= -margin) && pixelX < (m_MainBitmap->w + margin)) && (pixelY >= -1000) && (pixelY < (m_MainBitmap->h + margin));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneLayer::WrapPosition(int &posX, int &posY, bool scaled) const {
		bool wrapped = false;
		int width = scaled ? m_ScaledDimensions.GetFloorIntX() : m_MainBitmap->w;
		int height = scaled ? m_ScaledDimensions.GetFloorIntY() : m_MainBitmap->h;

		if (m_WrapX) {
			if (posX < 0) {
				while (posX < 0) {
					posX += width;
				}
				wrapped = true;
			} else if (posX >= width) {
				posX %= width;
				wrapped = true;
			}
		}
		if (m_WrapY) {
			if (posY < 0) {
				while (posY < 0) {
					posY += height;
				}
				wrapped = true;
			} else if (posY >= height) {
				posY %= height;
				wrapped = true;
			}
		}
		return wrapped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneLayer::ForceBounds(int &posX, int &posY, bool scaled) const {
		bool wrapped = false;
		int width = scaled ? m_ScaledDimensions.GetFloorIntX() : m_MainBitmap->w;
		int height = scaled ? m_ScaledDimensions.GetFloorIntY() : m_MainBitmap->h;

		if (posX < 0) {
			if (m_WrapX) {
				while (posX < 0) {
					posX += width;
				}
				wrapped = true;
			} else {
				posX = 0;
			}
		}
		if (posY < 0) {
			if (m_WrapY) {
				while (posY < 0) {
					posY += height;
				}
				wrapped = true;
			} else {
				posY = 0;
			}
		}
		if (posX >= width) {
			if (m_WrapX) {
				posX %= width;
				wrapped = true;
			} else {
				posX = width - 1;
			}
		}
		if (posY >= height) {
			if (m_WrapY) {
				posY %= height;
				wrapped = true;
			} else {
				posY = height - 1;
			}
		}
		return wrapped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneLayer::ForceBoundsOrWrapPosition(Vector &pos, bool scaled, bool forceBounds) const {
		int posX = pos.GetFloorIntX();
		int posY = pos.GetFloorIntY();
		bool wrapped = forceBounds ? ForceBounds(posX, posY, scaled) : WrapPosition(posX, posY, scaled);
		pos.SetXY(static_cast<float>(posX) + (pos.GetX() - std::floor(pos.GetX())), static_cast<float>(posY) + (pos.GetY() - std::floor(pos.GetY())));

		return wrapped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {
		RTEAssert(m_MainBitmap, "Data of this SceneLayer has not been loaded before trying to draw!");

		int offsetX = 0;
		int offsetY = 0;
		bool scrollOverridden = !(scrollOverride.GetX() == -1.0F && scrollOverride.GetY() == -1.0F);

		if (scrollOverridden) {
			offsetX = scrollOverride.GetFloorIntX();
			offsetY = scrollOverride.GetFloorIntY();
		} else {
			offsetX = static_cast<int>(std::floor(m_Offset.GetX() * m_ScrollRatio.GetX()));
			offsetY = static_cast<int>(std::floor(m_Offset.GetY() * m_ScrollRatio.GetY()));
			// Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
			WrapPosition(offsetX, offsetY);
		}

		// Make target box valid size if it's empty
		if (targetBox.IsEmpty()) {
			targetBox.SetCorner(Vector(0, 0));
			targetBox.SetWidth(static_cast<float>(targetBitmap->w));
			targetBox.SetHeight(static_cast<float>(targetBitmap->h));
		}

		// Set the clipping rectangle of the target bitmap to match the specified target box
		set_clip_rect(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()) - 1, static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()) - 1);

		// See if this SceneLayer is wider AND higher than the target bitmap; then use simple wrapping logic - otherwise need to tile
		if (m_MainBitmap->w >= targetBitmap->w && m_MainBitmap->h >= targetBitmap->h) {
			DrawWrapped(targetBitmap, targetBox, offsetX, offsetY);
		} else {
			DrawTiled(targetBitmap, targetBox, scrollOverridden, offsetX, offsetY);
		}
		// Reset the clip rect back to the entire target bitmap
		set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawWrapped(BITMAP *targetBitmap, const Box &targetBox, int offsetX, int offsetY) const {
		std::array<int, 2> sourcePosX = { offsetX, 0 };
		std::array<int, 2> sourcePosY = { offsetY, 0 };
		std::array<int, 2> sourceWidth = { m_MainBitmap->w - offsetX, offsetX };
		std::array<int, 2> sourceHeight = { m_MainBitmap->h - offsetY, offsetY };
		std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntX() + m_MainBitmap->w - offsetX };
		std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY(), targetBox.GetCorner().GetFloorIntY() + m_MainBitmap->h - offsetY };

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				if (m_DrawTrans) {
					masked_blit(m_MainBitmap, targetBitmap, sourcePosX.at(j), sourcePosY.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j), sourceHeight.at(i));
				} else {
					blit(m_MainBitmap, targetBitmap, sourcePosX.at(j), sourcePosY.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j), sourceHeight.at(i));
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawTiled(BITMAP *targetBitmap, const Box &targetBox, bool scrollOverridden, int offsetX, int offsetY) const {
		// Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
		int tiledOffsetX = 0;
		int tiledOffsetY = 0;
		// Use the dimensions of the target box, if it has any area at all
		int toCoverX = offsetX + targetBox.GetCorner().GetFloorIntX() + std::min(targetBitmap->w, static_cast<int>(targetBox.GetWidth()));
		int toCoverY = offsetY + targetBox.GetCorner().GetFloorIntY() + std::min(targetBitmap->h, static_cast<int>(targetBox.GetHeight()));

		// Check for special case adjustment when the screen is larger than the scene
		bool screenLargerThanSceneX = false;
		bool screenLargerThanSceneY = false;
		if (!scrollOverridden && g_SceneMan.GetSceneWidth() > 0) {
			screenLargerThanSceneX = targetBitmap->w > g_SceneMan.GetSceneWidth();
			screenLargerThanSceneY = targetBitmap->h > g_SceneMan.GetSceneHeight();
		}

		do {
			do {
				// If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
				int destX = (!m_WrapX && screenLargerThanSceneX) ? ((targetBitmap->w / 2) - (m_MainBitmap->w / 2)) : (targetBox.GetCorner().GetFloorIntX() + tiledOffsetX - offsetX);
				int destY = (!m_WrapY && screenLargerThanSceneY) ? ((targetBitmap->h / 2) - (m_MainBitmap->h / 2)) : (targetBox.GetCorner().GetFloorIntY() + tiledOffsetY - offsetY);
				if (m_DrawTrans) {
					masked_blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, m_MainBitmap->w, m_MainBitmap->h);
				} else {
					blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, m_MainBitmap->w, m_MainBitmap->h);
				}
				tiledOffsetX += m_MainBitmap->w;
			} while (m_WrapX && toCoverX > tiledOffsetX);

			tiledOffsetY += m_MainBitmap->h;
		} while (m_WrapY && toCoverY > tiledOffsetY);

		// Detect if non-wrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge
		if (!m_WrapX && !screenLargerThanSceneX && m_ScrollRatio.GetX() < 0) {
			if (m_FillLeftColor != g_MaskColor && offsetX != 0) { rectfill(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), targetBox.GetCorner().GetFloorIntX() - offsetX, static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()), m_FillLeftColor); }
			if (m_FillRightColor != g_MaskColor) { rectfill(targetBitmap, (targetBox.GetCorner().GetFloorIntX() - offsetX) + m_MainBitmap->w, targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()), static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()), m_FillRightColor); }
		}
		if (!m_WrapY && !screenLargerThanSceneY && m_ScrollRatio.GetY() < 0) {
			if (m_FillUpColor != g_MaskColor && offsetY != 0) { rectfill(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()), targetBox.GetCorner().GetFloorIntY() - offsetY, m_FillUpColor); }
			if (m_FillDownColor != g_MaskColor) { rectfill(targetBitmap, targetBox.GetCorner().GetFloorIntX(), (targetBox.GetCorner().GetFloorIntY() - offsetY) + m_MainBitmap->h, static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()), static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()), m_FillDownColor); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawScaled(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride) {
		if (m_ScaleFactor.GetX() == 1.0F && m_ScaleFactor.GetY() == 1.0F) {
			Draw(targetBitmap, targetBox, scrollOverride);
			return;
		}
		RTEAssert(m_MainBitmap, "Data of this SceneLayer has not been loaded before trying to draw!");

		int offsetX = 0;
		int offsetY = 0;
		bool scrollOverridden = !(scrollOverride.m_X == -1 && scrollOverride.m_Y == -1);

		if (scrollOverridden) {
			offsetX = scrollOverride.GetFloorIntX();
			offsetY = scrollOverride.GetFloorIntY();
		} else {
			offsetX = static_cast<int>(std::floor(m_Offset.GetX() * m_ScrollRatio.GetX()));
			offsetY = static_cast<int>(std::floor(m_Offset.GetY() * m_ScrollRatio.GetY()));
			// Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
			WrapPosition(offsetX, offsetY);
		}

		// Make target box valid size if it's empty
		if (targetBox.IsEmpty()) {
			targetBox.SetCorner(Vector(0, 0));
			targetBox.SetWidth(static_cast<float>(targetBitmap->w));
			targetBox.SetHeight(static_cast<float>(targetBitmap->h));
		}

		// Set the clipping rectangle of the target bitmap to match the specified target box
		set_clip_rect(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()) - 1, static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()) - 1);

		// See if this SceneLayer is wider AND higher than the target bitmap when scaled; then use simple wrapping logic - otherwise need to tile
		if (m_ScaledDimensions.GetFloorIntX() >= targetBitmap->w && m_ScaledDimensions.GetFloorIntY() >= targetBitmap->h) {
			DrawWrappedScaled(targetBitmap, targetBox, offsetX, offsetY);
		} else {
			DrawTiledScaled(targetBitmap, targetBox, scrollOverridden, offsetX, offsetY);
		}
		// Reset the clip rect back to the entire target bitmap
		set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawWrappedScaled(BITMAP *targetBitmap, const Box &targetBox, int offsetX, int offsetY) const {
		Vector scaledSourceOffset(static_cast<float>(offsetX) * m_ScaleInverse.GetX(), static_cast<float>(offsetY) * m_ScaleInverse.GetY());

		std::array<int, 2> sourceWidth = { m_MainBitmap->w, scaledSourceOffset.GetFloorIntX() };
		std::array<int, 2> sourceHeight = { m_MainBitmap->h, scaledSourceOffset.GetFloorIntY() };
		std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX() - offsetX, targetBox.GetCorner().GetFloorIntX() + m_ScaledDimensions.GetFloorIntX() - offsetX };
		std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY() - offsetY, targetBox.GetCorner().GetFloorIntY() + m_ScaledDimensions.GetFloorIntY() - offsetY };

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				if (m_DrawTrans) {
					masked_stretch_blit(m_MainBitmap, targetBitmap, 0, 0, sourceWidth.at(j), sourceHeight.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j) * m_ScaleFactor.GetFloorIntX() + 1, sourceHeight.at(i) * m_ScaleFactor.GetFloorIntY() + 1);
				} else {
					stretch_blit(m_MainBitmap, targetBitmap, 0, 0, sourceWidth.at(j), sourceHeight.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j) * m_ScaleFactor.GetFloorIntX() + 1, sourceHeight.at(i) * m_ScaleFactor.GetFloorIntY() + 1);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawTiledScaled(BITMAP *targetBitmap, const Box &targetBox, bool scrollOverridden, int offsetX, int offsetY) const {
		// Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
		int tiledOffsetX = 0;
		int tiledOffsetY = 0;
		// Use the dimensions of the target box, if it has any area at all
		int toCoverX = offsetX + targetBox.GetCorner().GetFloorIntX() + std::min(targetBitmap->w, static_cast<int>(targetBox.GetWidth()));
		int toCoverY = offsetY + targetBox.GetCorner().GetFloorIntY() + std::min(targetBitmap->h, static_cast<int>(targetBox.GetHeight()));

		// Check for special case adjustment when the screen is larger than the scene
		bool screenLargerThanSceneX = false;
		bool screenLargerThanSceneY = false;
		if (!scrollOverridden && g_SceneMan.GetSceneWidth() > 0) {
			screenLargerThanSceneX = targetBitmap->w > g_SceneMan.GetSceneWidth();
			screenLargerThanSceneY = targetBitmap->h > g_SceneMan.GetSceneHeight();
		}

		do {
			do {
				// If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
				int destX = (!m_WrapX && screenLargerThanSceneX) ? ((targetBitmap->w / 2) - (m_ScaledDimensions.GetFloorIntX() / 2)) : (targetBox.GetCorner().GetFloorIntX() + tiledOffsetX - offsetX);
				int destY = (!m_WrapY && screenLargerThanSceneY) ? ((targetBitmap->h / 2) - (m_ScaledDimensions.GetFloorIntY() / 2)) : (targetBox.GetCorner().GetFloorIntY() + tiledOffsetY - offsetY);
				if (m_DrawTrans) {
					masked_stretch_blit(m_MainBitmap, targetBitmap, 0, 0, m_MainBitmap->w, m_MainBitmap->h, destX, destY, m_ScaledDimensions.GetFloorIntX(), m_ScaledDimensions.GetFloorIntY());
				} else {
					stretch_blit(m_MainBitmap, targetBitmap, 0, 0, m_MainBitmap->w, m_MainBitmap->h, destX, destY, m_ScaledDimensions.GetFloorIntX(), m_ScaledDimensions.GetFloorIntY());
				}
				tiledOffsetX += m_ScaledDimensions.GetFloorIntX();
			}
			while (m_WrapX && toCoverX > tiledOffsetX);

			tiledOffsetY += m_ScaledDimensions.GetFloorIntY();
		}
		while (m_WrapY && toCoverY > tiledOffsetY);
	}
}