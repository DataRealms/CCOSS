#include "SceneLayer.h"

#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(SceneLayerTracked, Entity, 0);
	ConcreteClassInfo(SceneLayer, Entity, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::Clear() {
		m_BitmapFile.Reset();
		m_MainBitmap = nullptr;
		m_BackBitmap = nullptr;
		m_LastClearColor = ColorKeys::g_InvalidColor;
		m_Drawings.clear();
		m_MainBitmapOwned = false;
		m_DrawMasked = true;
		m_WrapX = true;
		m_WrapY = true;
		m_OriginOffset.Reset();
		m_Offset.Reset();
		m_ScrollInfo.SetXY(1.0F, 1.0F);
		m_ScrollRatio.SetXY(1.0F, 1.0F);
		m_ScaleFactor.SetXY(1.0F, 1.0F);
		m_ScaledDimensions.SetXY(1.0F, 1.0F);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::Create(const ContentFile &bitmapFile, bool drawMasked, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo) {
		m_BitmapFile = bitmapFile;
		m_MainBitmap = m_BitmapFile.GetAsBitmap();
		Create(m_MainBitmap, drawMasked, offset, wrapX, wrapY, scrollInfo);

		m_MainBitmapOwned = false;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::Create(BITMAP *bitmap, bool drawMasked, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo) {
		m_MainBitmap = bitmap;
		RTEAssert(m_MainBitmap, "Null bitmap passed in when creating SceneLayerImpl!");

		m_MainBitmapOwned = true;

		m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
		m_LastClearColor = ColorKeys::g_InvalidColor;

		m_DrawMasked = drawMasked;
		m_Offset = offset;
		m_WrapX = wrapX;
		m_WrapY = wrapY;
		m_ScrollInfo = scrollInfo;

		InitScrollRatios();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::Create(const SceneLayerImpl &reference) {
		Entity::Create(reference);

		m_BitmapFile = reference.m_BitmapFile;
		m_DrawMasked = reference.m_DrawMasked;
		m_WrapX = reference.m_WrapX;
		m_WrapY = reference.m_WrapY;
		m_OriginOffset = reference.m_OriginOffset;
		m_ScrollInfo = reference.m_ScrollInfo;
		m_ScrollRatio = reference.m_ScrollRatio;
		m_ScaleFactor = reference.m_ScaleFactor;
		m_ScaledDimensions = reference.m_ScaledDimensions;

		if (reference.m_MainBitmap) {
			// Make a copy of the bitmap because it can be modified in some use cases.
			BITMAP *bitmapToCopy = reference.m_MainBitmap;
			RTEAssert(bitmapToCopy, "Couldn't load the bitmap file specified for SceneLayerImpl!");

			m_MainBitmap = create_bitmap_ex(bitmap_color_depth(bitmapToCopy), bitmapToCopy->w, bitmapToCopy->h);
			RTEAssert(m_MainBitmap, "Failed to allocate BITMAP in SceneLayerImpl::Create");
			blit(bitmapToCopy, m_MainBitmap, 0, 0, 0, 0, bitmapToCopy->w, bitmapToCopy->h);

			m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
			m_LastClearColor = ColorKeys::g_InvalidColor;

			InitScrollRatios();

			m_MainBitmapOwned = true;
		} else {
			// If no bitmap to copy, then it has to be loaded with LoadData.
			m_MainBitmapOwned = false;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::ReadProperty(const std::string_view &propName, Reader &reader) {
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

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("WrapX", m_WrapX);
		writer.NewPropertyWithValue("WrapY", m_WrapY);
		writer.NewPropertyWithValue("BitmapFile", m_BitmapFile);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::Destroy(bool notInherited) {
		if (m_MainBitmapOwned) { destroy_bitmap(m_MainBitmap); }
		if (m_BackBitmap) { destroy_bitmap(m_BackBitmap); }
		if (!notInherited) { Entity::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::InitScrollRatios(bool initForNetworkPlayer, int player) {
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

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::LoadData() {
		// Load from disk and take ownership. Don't cache because the bitmap will be modified.
		m_MainBitmap = m_BitmapFile.GetAsBitmap(COLORCONV_NONE, false);
		m_MainBitmapOwned = true;

		m_BackBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
		m_LastClearColor = ColorKeys::g_InvalidColor;

		InitScrollRatios();
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::SaveData(const std::string &bitmapPath) {
		if (bitmapPath.empty()) {
			return -1;
		}
		if (m_MainBitmap) {
			// Make a copy of the bitmap to pass to the thread because the bitmap may be offloaded mid thread and everything will be on fire.
			BITMAP *outputBitmap = create_bitmap_ex(bitmap_color_depth(m_MainBitmap), m_MainBitmap->w, m_MainBitmap->h);
			blit(m_MainBitmap, outputBitmap, 0, 0, 0, 0, m_MainBitmap->w, m_MainBitmap->h);

			auto saveLayerBitmap = [bitmapPath](BITMAP *bitmapToSave) {
				PALETTE palette;
				get_palette(palette);
				if (save_png(bitmapPath.c_str(), bitmapToSave, palette) != 0) {
					// TODO: This will not kill the main thread. Figure this out!
					RTEAbort(std::string("Failed to save SceneLayerImpl bitmap to path and name: " + bitmapPath));
				}
				destroy_bitmap(bitmapToSave);
			};
			std::thread saveThread(saveLayerBitmap, outputBitmap);
			m_BitmapFile.SetDataPath(bitmapPath);
			// TODO: Move this into some global thread container or a ThreadMan instead of detaching.
			saveThread.detach();
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::ClearData() {
		if (m_MainBitmap && m_MainBitmapOwned) { destroy_bitmap(m_MainBitmap); }
		m_MainBitmap = nullptr;
		m_MainBitmapOwned = false;

		if (m_BackBitmap) { destroy_bitmap(m_BackBitmap); }
		m_BackBitmap = nullptr;
		m_LastClearColor = ColorKeys::g_InvalidColor;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::SetScaleFactor(const Vector &newScale) {
		m_ScaleFactor = newScale;
		if (m_MainBitmap) { m_ScaledDimensions.SetXY(static_cast<float>(m_MainBitmap->w) * newScale.GetX(), static_cast<float>(m_MainBitmap->h) * newScale.GetY()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	int SceneLayerImpl<TRACK_DRAWINGS>::GetPixel(int pixelX, int pixelY) const {
		WrapPosition(pixelX, pixelY);
		return (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) ? MaterialColorKeys::g_MaterialAir : _getpixel(m_MainBitmap, pixelX, pixelY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::SetPixel(int pixelX, int pixelY, int materialID) {
		RTEAssert(m_MainBitmapOwned, "Trying to set a pixel of a SceneLayer's bitmap which isn't owned!");

		WrapPosition(pixelX, pixelY);

		if (pixelX < 0 || pixelX >= m_MainBitmap->w || pixelY < 0 || pixelY >= m_MainBitmap->h) {
			return;
		}
		_putpixel(m_MainBitmap, pixelX, pixelY, materialID);

		RegisterDrawing(pixelX, pixelY, pixelX, pixelY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	bool SceneLayerImpl<TRACK_DRAWINGS>::IsWithinBounds(const int pixelX, const int pixelY, const int margin) const {
		return (m_WrapX || (pixelX >= -margin && pixelX < m_MainBitmap->w + margin)) && (m_WrapY || (pixelY >= -margin && pixelY < m_MainBitmap->h + margin));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::ClearBitmap(ColorKeys clearTo) {
		RTEAssert(m_MainBitmapOwned, "Bitmap not owned! We shouldn't be clearing this!");

		std::scoped_lock<std::mutex> bitmapClearLock(m_BitmapClearMutex);

		if (m_LastClearColor != clearTo) {
			// Note: We're clearing to a different color than expected, which is expensive! We should always aim to clear to the same color to avoid it as much as possible.
			clear_to_color(m_BackBitmap, clearTo);
			m_LastClearColor = clearTo;
		}

		std::swap(m_MainBitmap, m_BackBitmap);

		// Start a new thread to clear the backbuffer bitmap asynchronously.
		std::thread clearBackBitmapThread([this, clearTo](BITMAP *bitmap, std::vector<IntRect> drawings) {
			this->m_BitmapClearMutex.lock();
			ClearDrawings(bitmap, drawings, clearTo);
			this->m_BitmapClearMutex.unlock();
		}, m_BackBitmap, m_Drawings);

		clearBackBitmapThread.detach();

		m_Drawings.clear(); // This was copied into the new thread, so can be safely deleted.
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	bool SceneLayerImpl<TRACK_DRAWINGS>::WrapPosition(int &posX, int &posY) const {
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

	template <bool TRACK_DRAWINGS>
	bool SceneLayerImpl<TRACK_DRAWINGS>::ForceBounds(int &posX, int &posY) const {
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

	template <bool TRACK_DRAWINGS>
	bool SceneLayerImpl<TRACK_DRAWINGS>::ForceBoundsOrWrapPosition(Vector &pos, bool forceBounds) const {
		int posX = pos.GetFloorIntX();
		int posY = pos.GetFloorIntY();
		bool wrapped = forceBounds ? ForceBounds(posX, posY) : WrapPosition(posX, posY);
		pos.SetXY(static_cast<float>(posX) + (pos.GetX() - std::floor(pos.GetX())), static_cast<float>(posY) + (pos.GetY() - std::floor(pos.GetY())));

		return wrapped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::RegisterDrawing(int left, int top, int right, int bottom) {
		if constexpr (TRACK_DRAWINGS) {
			m_Drawings.emplace_back(left, top, right, bottom);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::RegisterDrawing(const Vector &center, float radius) {
		if (radius != 0.0F) {
			RegisterDrawing(static_cast<int>(center.GetX() - radius), static_cast<int>(center.GetY() - radius), static_cast<int>(center.GetX() + radius), static_cast<int>(center.GetY() + radius));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment) {
		RTEAssert(m_MainBitmap, "Data of this SceneLayerImpl has not been loaded before trying to draw!");

		if (offsetNeedsScrollRatioAdjustment) { m_Offset.SetXY(std::floor(m_Offset.GetX() * m_ScrollRatio.GetX()), std::floor(m_Offset.GetY() * m_ScrollRatio.GetY())); }
		if (targetBox.IsEmpty()) { targetBox = Box(Vector(), static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h)); }
		if (!m_WrapX && static_cast<float>(targetBitmap->w) > targetBox.GetWidth()) { m_Offset.SetX(0); }
		if (!m_WrapY && static_cast<float>(targetBitmap->h) > targetBox.GetHeight()) { m_Offset.SetY(0); }

		m_Offset -= m_OriginOffset;
		WrapPosition(m_Offset);

		set_clip_rect(targetBitmap, targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntY(), static_cast<int>(targetBox.GetCorner().GetX() + targetBox.GetWidth()) - 1, static_cast<int>(targetBox.GetCorner().GetY() + targetBox.GetHeight()) - 1);
		bool drawScaled = m_ScaleFactor.GetX() > 1.0F || m_ScaleFactor.GetY() > 1.0F;

		if (m_MainBitmap->w > targetBitmap->w && m_MainBitmap->h > targetBitmap->h) {
			DrawWrapped(targetBitmap, targetBox, drawScaled);
		} else {
			DrawTiled(targetBitmap, targetBox, drawScaled);
		}
		set_clip_rect(targetBitmap, 0, 0, targetBitmap->w - 1, targetBitmap->h - 1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::DrawWrapped(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const {
		if (!drawScaled) {
			std::array<int, 2> sourcePosX = { m_Offset.GetFloorIntX(), 0 };
			std::array<int, 2> sourcePosY = { m_Offset.GetFloorIntY(), 0 };
			std::array<int, 2> sourceWidth = { m_MainBitmap->w - m_Offset.GetFloorIntX(), m_Offset.GetFloorIntX() };
			std::array<int, 2> sourceHeight = { m_MainBitmap->h - m_Offset.GetFloorIntY(), m_Offset.GetFloorIntY() };
			std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX(), targetBox.GetCorner().GetFloorIntX() + m_MainBitmap->w - m_Offset.GetFloorIntX() };
			std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY(), targetBox.GetCorner().GetFloorIntY() + m_MainBitmap->h - m_Offset.GetFloorIntY() };

			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					if (m_DrawMasked) {
						masked_blit(m_MainBitmap, targetBitmap, sourcePosX[j], sourcePosY[i], destPosX[j], destPosY[i], sourceWidth[j], sourceHeight[i]);
					} else {
						blit(m_MainBitmap, targetBitmap, sourcePosX[j], sourcePosY[i], destPosX[j], destPosY[i], sourceWidth[j], sourceHeight[i]);
					}
				}
			}
		} else {
			std::array<int, 2> sourceWidth = { m_MainBitmap->w, m_Offset.GetFloorIntX() / m_ScaleFactor.GetFloorIntX() };
			std::array<int, 2> sourceHeight = { m_MainBitmap->h, m_Offset.GetFloorIntY() / m_ScaleFactor.GetFloorIntY() };
			std::array<int, 2> destPosX = { targetBox.GetCorner().GetFloorIntX() - m_Offset.GetFloorIntX(), targetBox.GetCorner().GetFloorIntX() + m_ScaledDimensions.GetFloorIntX() - m_Offset.GetFloorIntX() };
			std::array<int, 2> destPosY = { targetBox.GetCorner().GetFloorIntY() - m_Offset.GetFloorIntY(), targetBox.GetCorner().GetFloorIntY() + m_ScaledDimensions.GetFloorIntY() - m_Offset.GetFloorIntY() };

			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					if (m_DrawMasked) {
						masked_stretch_blit(m_MainBitmap, targetBitmap, 0, 0, sourceWidth[j], sourceHeight[i], destPosX[j], destPosY[i], sourceWidth[j] * m_ScaleFactor.GetFloorIntX() + 1, sourceHeight[i] * m_ScaleFactor.GetFloorIntY() + 1);
					} else {
						stretch_blit(m_MainBitmap, targetBitmap, 0, 0, sourceWidth[j], sourceHeight[i], destPosX[j], destPosY[i], sourceWidth[j] * m_ScaleFactor.GetFloorIntX() + 1, sourceHeight[i] * m_ScaleFactor.GetFloorIntY() + 1);
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::DrawTiled(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const {
		int bitmapWidth = m_ScaledDimensions.GetFloorIntX();
		int bitmapHeight = m_ScaledDimensions.GetFloorIntY();
		int areaToCoverX = m_Offset.GetFloorIntX() + targetBox.GetCorner().GetFloorIntX() + std::min(targetBitmap->w, static_cast<int>(targetBox.GetWidth()));
		int areaToCoverY = m_Offset.GetFloorIntY() + targetBox.GetCorner().GetFloorIntY() + std::min(targetBitmap->h, static_cast<int>(targetBox.GetHeight()));

		for (int tiledOffsetX = 0; tiledOffsetX < areaToCoverX;) {
			int destX = targetBox.GetCorner().GetFloorIntX() + tiledOffsetX - m_Offset.GetFloorIntX();

			for (int tiledOffsetY = 0; tiledOffsetY < areaToCoverY;) {
				int destY = targetBox.GetCorner().GetFloorIntY() + tiledOffsetY - m_Offset.GetFloorIntY();

				if (!drawScaled) {
					if (m_DrawMasked) {
						masked_blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, bitmapWidth, bitmapHeight);
					} else {
						blit(m_MainBitmap, targetBitmap, 0, 0, destX, destY, bitmapWidth, bitmapHeight);
					}
				} else {
					if (m_DrawMasked) {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <bool TRACK_DRAWINGS>
	void SceneLayerImpl<TRACK_DRAWINGS>::ClearDrawings(BITMAP* bitmap, const std::vector<IntRect> &drawings, ColorKeys clearTo) const {
		if constexpr (TRACK_DRAWINGS) {
			for (const IntRect &rect : drawings) {
				int left = rect.m_Left;
				int top = rect.m_Top;
				int bottom = rect.m_Bottom;
				int right = rect.m_Right;

				rectfill(bitmap, left, top, right, bottom, clearTo);

				if (m_WrapX) {
					if (left < 0) {
						int wrapLeft = left + bitmap->w;
						int wrapRight = bitmap->w - 1;
						rectfill(bitmap, wrapLeft, top, wrapRight, bottom, clearTo);
					}

					if (right >= bitmap->w) {
						int wrapLeft = 0;
						int wrapRight = right - bitmap->w;
						rectfill(bitmap, wrapLeft, top, wrapRight, bottom, clearTo);
					}
				}

				if (m_WrapY) {
					if (top < 0) {
						int wrapTop = top + bitmap->h;
						int wrapBottom = bitmap->h - 1;
						rectfill(bitmap, left, wrapTop, right, wrapBottom, clearTo);
					}

					if (bottom >= bitmap->h) {
						int wrapTop = 0;
						int wrapBottom = bottom - bitmap->h;
						rectfill(bitmap, left, wrapTop, right, wrapBottom, clearTo);
					}
				}
			}
		} else {
			clear_to_color(bitmap, clearTo);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Force instantiation
	template class SceneLayerImpl<false>;
	template class SceneLayerImpl<true>;
}