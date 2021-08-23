#include "SceneLayer.h"

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
		m_WrapX = true;
		m_WrapY = true;
		m_OriginOffset.Reset();
		m_Offset.Reset();
		m_ScrollInfo.SetXY(1.0F, 1.0F);
		m_ScrollRatio.SetXY(1.0F, 1.0F);
		m_ScaleFactor.SetXY(1.0F, 1.0F);
		m_ScaleInverse.SetXY(1.0F, 1.0F);
		m_ScaledDimensions.SetXY(1.0F, 1.0F);
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
		m_WrapX = reference.m_WrapX;
		m_WrapY = reference.m_WrapY;
		m_OriginOffset = reference.m_OriginOffset;
		m_ScrollInfo = reference.m_ScrollInfo;
		m_ScrollRatio = reference.m_ScrollRatio;
		m_ScaleFactor = reference.m_ScaleFactor;
		m_ScaleInverse = reference.m_ScaleInverse;
		m_ScaledDimensions = reference.m_ScaledDimensions;

		if (reference.m_MainBitmap) {
			// Make a copy of the bitmap because it can be modified in some use cases.
			BITMAP *bitmapToCopy = reference.m_MainBitmap;
			RTEAssert(bitmapToCopy, "Couldn't load the bitmap file specified for SceneLayer!");

			m_MainBitmap = create_bitmap_ex(8, bitmapToCopy->w, bitmapToCopy->h);
			RTEAssert(m_MainBitmap, "Failed to allocate BITMAP in SceneLayer::Create");
			blit(bitmapToCopy, m_MainBitmap, 0, 0, 0, 0, bitmapToCopy->w, bitmapToCopy->h);

			InitScrollRatios();

			m_MainBitmapOwned = true;
		} else {
			// If no bitmap to copy, then it has to be loaded with LoadData.
			m_MainBitmapOwned = false;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "WrapX") {
			reader >> m_WrapX;
		} else if (propName == "WrapY") {
			reader >> m_WrapY;
		} else if (propName == "BitmapFile") {
			reader >> m_BitmapFile;
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SceneLayer::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("WrapX", m_WrapX);
		writer.NewPropertyWithValue("WrapY", m_WrapY);
		writer.NewPropertyWithValue("BitmapFile", m_BitmapFile);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::Destroy(bool notInherited) {
		if (m_MainBitmapOwned) { destroy_bitmap(m_MainBitmap); }
		if (!notInherited) { Entity::Destroy(); }
		Clear();
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

	int SceneLayer::LoadData() {
		// Re-load directly from disk each time; don't do any caching of these bitmaps.
		m_MainBitmap = m_BitmapFile.GetAsBitmap(COLORCONV_NONE, false);
		m_MainBitmapOwned = true;
		InitScrollRatios();
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
			// Set the new path to point to the new file location - only if there was a successful save of the bitmap.
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

	bool SceneLayer::WrapPosition(int &posX, int &posY) const {
		bool wrapped = false;
		int width = m_ScaledDimensions.GetFloorIntX();
		int height = m_ScaledDimensions.GetFloorIntY();

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

	bool SceneLayer::ForceBounds(int &posX, int &posY) const {
		bool wrapped = false;
		int width = m_ScaledDimensions.GetFloorIntX();
		int height = m_ScaledDimensions.GetFloorIntY();

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

	bool SceneLayer::ForceBoundsOrWrapPosition(Vector &pos, bool forceBounds) const {
		int posX = pos.GetFloorIntX();
		int posY = pos.GetFloorIntY();
		bool wrapped = forceBounds ? ForceBounds(posX, posY) : WrapPosition(posX, posY);
		pos.SetXY(static_cast<float>(posX) + (pos.GetX() - std::floor(pos.GetX())), static_cast<float>(posY) + (pos.GetY() - std::floor(pos.GetY())));

		return wrapped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment) {
		RTEAssert(m_MainBitmap, "Data of this SceneLayer has not been loaded before trying to draw!");

		if (offsetNeedsScrollRatioAdjustment) { m_Offset.SetXY(std::floor(m_Offset.GetX() * m_ScrollRatio.GetX()), std::floor(m_Offset.GetY() * m_ScrollRatio.GetY())); }
		if (targetBox.IsEmpty()) { targetBox = Box(Vector(), static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h)); }
		if (!m_WrapX && static_cast<float>(targetBitmap->w) > targetBox.GetWidth()) { m_Offset.SetX(0); }
		if (!m_WrapY && static_cast<float>(targetBitmap->h) > targetBox.GetHeight()) { m_Offset.SetY(0); }

		m_Offset -= m_OriginOffset;
		WrapPosition(m_Offset);

		set_clip_rect(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()) - 1, static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()) - 1);
		bool drawScaled = m_ScaleFactor.GetX() > 1.0F || m_ScaleFactor.GetY() > 1.0F;

		// If this SceneLayer is wider AND higher than the target bitmap, use simple wrapping logic - otherwise need to tile.
		if (m_MainBitmap->w > targetBitmap->w && m_MainBitmap->h > targetBitmap->h) {
			DrawWrapped(targetBitmap, targetBox, drawScaled);
		} else {
			DrawTiled(targetBitmap, targetBox, drawScaled);
		}
		// Reset the clip rect back to the entire target bitmap.
		set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawWrapped(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const {
		if (!drawScaled) {
			std::array<int, 2> sourcePosX = { m_Offset.GetFloorIntX(), 0 };
			std::array<int, 2> sourcePosY = { m_Offset.GetFloorIntY(), 0 };
			std::array<int, 2> sourceWidth = { m_MainBitmap->w - m_Offset.GetFloorIntX(), m_Offset.GetFloorIntX() };
			std::array<int, 2> sourceHeight = { m_MainBitmap->h - m_Offset.GetFloorIntY(), m_Offset.GetFloorIntY() };
			std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntX() + m_MainBitmap->w - m_Offset.GetFloorIntX() };
			std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY(), targetBox.GetCorner().GetFloorIntY() + m_MainBitmap->h - m_Offset.GetFloorIntY() };

			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					if (m_DrawTrans) {
						masked_blit(m_MainBitmap, targetBitmap, sourcePosX.at(j), sourcePosY.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j), sourceHeight.at(i));
					} else {
						blit(m_MainBitmap, targetBitmap, sourcePosX.at(j), sourcePosY.at(i), destPosX.at(j), destPosY.at(i), sourceWidth.at(j), sourceHeight.at(i));
					}
				}
			}
		} else {
			std::array<int, 2> sourceWidth = { m_MainBitmap->w, m_Offset.GetFloorIntX() * m_ScaleInverse.GetFloorIntX() };
			std::array<int, 2> sourceHeight = { m_MainBitmap->h, m_Offset.GetFloorIntY() * m_ScaleInverse.GetFloorIntY() };
			std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX() - m_Offset.GetFloorIntX(), targetBox.GetCorner().GetFloorIntX() + m_ScaledDimensions.GetFloorIntX() - m_Offset.GetFloorIntX() };
			std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY() - m_Offset.GetFloorIntY(), targetBox.GetCorner().GetFloorIntY() + m_ScaledDimensions.GetFloorIntY() - m_Offset.GetFloorIntY() };

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
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SceneLayer::DrawTiled(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const {
		int bitmapWidth = m_ScaledDimensions.GetFloorIntX();
		int bitmapHeight = m_ScaledDimensions.GetFloorIntY();
		int areaToCoverX = m_Offset.GetFloorIntX() + targetBox.GetCorner().GetFloorIntX() + std::min(targetBitmap->w, static_cast<int>(targetBox.GetWidth()));
		int areaToCoverY = m_Offset.GetFloorIntY() + targetBox.GetCorner().GetFloorIntY() + std::min(targetBitmap->h, static_cast<int>(targetBox.GetHeight()));

		for (int tiledOffsetX = 0; tiledOffsetX < areaToCoverX;) {
			int destX = targetBox.GetCorner().GetFloorIntX() + tiledOffsetX - m_Offset.GetFloorIntX();

			for (int tiledOffsetY = 0; tiledOffsetY < areaToCoverY;) {
				int destY = targetBox.GetCorner().GetFloorIntY() + tiledOffsetY - m_Offset.GetFloorIntY();

				if (!drawScaled) {
					if (m_DrawTrans) {
						masked_blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, bitmapWidth, bitmapHeight);
					} else {
						blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, bitmapWidth, bitmapHeight);
					}
				} else {
					if (m_DrawTrans) {
						masked_stretch_blit(m_MainBitmap, targetBitmap, 0, 0, m_MainBitmap->w, m_MainBitmap->h, destX, destY, bitmapWidth, bitmapHeight);
					} else {
						stretch_blit(m_MainBitmap, targetBitmap, 0, 0, m_MainBitmap->w, m_MainBitmap->h, destX, destY, bitmapWidth, bitmapHeight);
					}
				}
				if (!m_WrapY) {
					break;
				}
				tiledOffsetY += bitmapHeight;
			}
			if (!m_WrapX) {
				break;
			}
			tiledOffsetX += bitmapWidth;
		}
	}
}