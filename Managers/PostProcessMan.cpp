#include "PostProcessMan.h"
#include "FrameMan.h"
#include "Scene.h"
#include "ContentFile.h"
#include "Matrix.h"

namespace RTE {

	const std::string PostProcessMan::c_ClassName = "PostProcessMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::Clear() {
		m_PostScreenEffects.clear();
		m_PostSceneEffects.clear();
		m_YellowGlow = 0;
		m_YellowGlowHash = 0;
		m_RedGlow = 0;
		m_RedGlowHash = 0;
		m_BlueGlow = 0;
		m_BlueGlowHash = 0;
		for (short i = 0; i < c_MaxScreenCount; ++i) {
			m_ScreenRelativeEffects->clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PostProcessMan::Create() {
		// TODO: Make more robust and load more glows!
		ContentFile glowFile("Base.rte/Effects/Glows/YellowTiny.bmp");
		m_YellowGlow = glowFile.GetAsBitmap();
		m_YellowGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/RedTiny.bmp");
		m_RedGlow = glowFile.GetAsBitmap();
		m_RedGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/BlueTiny.bmp");
		m_BlueGlow = glowFile.GetAsBitmap();
		m_BlueGlowHash = glowFile.GetHash();

		m_TempEffectBitmap_16 = create_bitmap_ex(32, 16, 16);
		m_TempEffectBitmap_32 = create_bitmap_ex(32, 32, 32);
		m_TempEffectBitmap_64 = create_bitmap_ex(32, 64, 64);
		m_TempEffectBitmap_128 = create_bitmap_ex(32, 128, 128);
		m_TempEffectBitmap_256 = create_bitmap_ex(32, 256, 256);
		m_TempEffectBitmap_512 = create_bitmap_ex(32, 512, 512);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::Destroy() {
		ClearScreenPostEffects();
		ClearScenePostEffects();
		Clear();
		destroy_bitmap(m_TempEffectBitmap_16);
		destroy_bitmap(m_TempEffectBitmap_32);
		destroy_bitmap(m_TempEffectBitmap_64);
		destroy_bitmap(m_TempEffectBitmap_128);
		destroy_bitmap(m_TempEffectBitmap_256);
		destroy_bitmap(m_TempEffectBitmap_512);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::AdjustEffectsPosToPlayerScreen(char playerScreen, BITMAP *targetBitmap, Vector targetBitmapOffset, std::list<PostEffect> &screenRelativeEffectsList, std::list<Box> &screenRelativeGlowBoxesList) {
		int screenOcclusionOffsetX = g_SceneMan.GetScreenOcclusion(playerScreen).GetFloorIntX();
		int screenOcclusionOffsetY = g_SceneMan.GetScreenOcclusion(playerScreen).GetFloorIntY();
		int occludedOffsetX = targetBitmap->w + screenOcclusionOffsetX;
		int occludedOffsetY = targetBitmap->h + screenOcclusionOffsetY;

		// Copy post effects received by client if in network mode
		if (g_FrameMan.GetDrawNetworkBackBuffer()) { g_PostProcessMan.GetNetworkPostEffectsList(0, screenRelativeEffectsList); }

		// Adjust for the player screen's position on the final buffer
		for (list<PostEffect>::iterator eItr = screenRelativeEffectsList.begin(); eItr != screenRelativeEffectsList.end(); ++eItr) {
			// Make sure we won't be adding any effects to a part of the screen that is occluded by menus and such
			if ((*eItr).m_Pos.m_X > screenOcclusionOffsetX && (*eItr).m_Pos.m_Y > screenOcclusionOffsetY && (*eItr).m_Pos.m_X < occludedOffsetX && (*eItr).m_Pos.m_Y < occludedOffsetY) {
				g_PostProcessMan.GetPostScreenEffectsList()->push_back(PostEffect((*eItr).m_Pos + targetBitmapOffset, (*eItr).m_Bitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
			}
		}
		// Adjust glow areas for the player screen's position on the final buffer
		for (list<Box>::iterator bItr = screenRelativeGlowBoxesList.begin(); bItr != screenRelativeGlowBoxesList.end(); ++bItr) {
			g_PostProcessMan.GetPostScreenGlowBoxesList()->push_back(*bItr);
			// Adjust each added glow area for the player screen's position on the final buffer
			g_PostProcessMan.GetPostScreenGlowBoxesList()->back().m_Corner += targetBitmapOffset;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterPostEffect(const Vector &effectPos, BITMAP *effect, size_t hash, unsigned char strength, float angle) {
		// These effects get applied when there's a drawn frame that followed one or more sim updates.
		// They are not only registered on drawn sim updates; flashes and stuff could be missed otherwise if they occur on undrawn sim updates.
		if (effect && g_TimerMan.SimUpdatesSinceDrawn() >= 0) { m_PostSceneEffects.push_back(PostEffect(effectPos, effect, hash, strength, angle)); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffectsWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team) {
		bool found = false;

		// Do the first unwrapped rect
		found = GetPostScreenEffects(boxPos, boxWidth, boxHeight, effectsList, team);

		int left = boxPos.m_X;
		int top = boxPos.m_Y;
		int right = left + boxWidth;
		int bottom = top + boxHeight;

		if (g_SceneMan.SceneWrapsX()) {
			int sceneWidth = g_SceneMan.GetScene()->GetWidth();
			if (left < 0) { found = GetPostScreenEffects(left + sceneWidth, top, right + sceneWidth, bottom, effectsList, team) || found; }
			if (right >= sceneWidth) { found = GetPostScreenEffects(left - sceneWidth, top, right - sceneWidth, bottom, effectsList, team) || found; }
		}
		if (g_SceneMan.SceneWrapsY()) {
			int sceneHeight = g_SceneMan.GetScene()->GetHeight();
			if (top < 0) { found = GetPostScreenEffects(left, top + sceneHeight, right, bottom + sceneHeight, effectsList, team) || found; }
			if (bottom >= sceneHeight) { found = GetPostScreenEffects(left, top - sceneHeight, right, bottom - sceneHeight, effectsList, team) || found; }
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP *PostProcessMan::GetTempEffectBitmap(unsigned short bitmapSize) const {
		if (bitmapSize <= 16) {
			return m_TempEffectBitmap_16;
		} else if (bitmapSize <= 32) {
			return m_TempEffectBitmap_32;
		} else if (bitmapSize <= 64) {
			return m_TempEffectBitmap_64;
		} else if (bitmapSize <= 128) {
			return m_TempEffectBitmap_128;
		} else if (bitmapSize <= 256) {
			return m_TempEffectBitmap_256;
		} else {
			return m_TempEffectBitmap_512;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterGlowDotEffect(const Vector &effectPos, DotGlowColor color, int strength) {
		// These effects only apply only once per drawn sim update, and only on the first frame drawn after one or more sim updates
		if (color != NoDot && g_TimerMan.DrawnSimUpdate() && g_TimerMan.SimUpdatesSinceDrawn() >= 0) {
			RegisterPostEffect(effectPos, GetDotGlowEffect(color), GetDotGlowEffectHash(color), strength);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetGlowAreasWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<Box> &areaList) {
		bool foundAny = false;
		Vector intRectPosRelativeToBox;

		// Account for wrapping in any registered glow IntRects, as well as on the box we're testing against
		std::list<IntRect> wrappedGlowRects;
		for (std::list<IntRect>::iterator grItr = m_GlowAreas.begin(); grItr != m_GlowAreas.end(); ++grItr) {
			g_SceneMan.WrapRect((*grItr), wrappedGlowRects);
		}
		std::list<IntRect> wrappedTestRects;
		g_SceneMan.WrapRect(IntRect(boxPos.m_X, boxPos.m_Y, boxPos.m_X + boxWidth, boxPos.m_Y + boxHeight), wrappedTestRects);

		// Check for intersections. If any are found, cut down the intersecting IntRect to the bounds of the IntRect we're testing against, then make and store a Box out of it
		for (IntRect wrappedTestRect : wrappedTestRects) {
			for (IntRect wrappedGlowRect : wrappedGlowRects) {
				if (wrappedTestRect.Intersects(wrappedGlowRect)) {
					IntRect cutRect(wrappedGlowRect);
					cutRect.IntersectionCut(wrappedTestRect);
					intRectPosRelativeToBox = Vector(cutRect.m_Left - boxPos.m_X, cutRect.m_Top - boxPos.m_Y);
					areaList.push_back(Box(intRectPosRelativeToBox, cutRect.m_Right - cutRect.m_Left, cutRect.m_Bottom - cutRect.m_Top));
					foundAny = true;
				}
			}
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::GetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> & outputList) {
		ScreenRelativeEffectsMutex[whichScreen].lock();
		outputList.clear();
		for (std::list<PostEffect>::iterator eItr = m_ScreenRelativeEffects[whichScreen].begin(); eItr != m_ScreenRelativeEffects[whichScreen].end(); ++eItr) {
			outputList.push_back(PostEffect((*eItr).m_Pos, (*eItr).m_Bitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
		}
		ScreenRelativeEffectsMutex[whichScreen].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> & inputList) {
		ScreenRelativeEffectsMutex[whichScreen].lock();
		m_ScreenRelativeEffects[whichScreen].clear();
		for (std::list<PostEffect>::iterator eItr = inputList.begin(); eItr != inputList.end(); ++eItr) {
			m_ScreenRelativeEffects[whichScreen].push_back(PostEffect((*eItr).m_Pos, (*eItr).m_Bitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
		}
		ScreenRelativeEffectsMutex[whichScreen].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffects(Vector boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team) {
		bool found = false;
		bool unseen = false;
		Vector postEffectPosRelativeToBox;

		for (std::list<PostEffect>::iterator itr = m_PostSceneEffects.begin(); itr != m_PostSceneEffects.end(); ++itr) {
			if (team != Activity::NOTEAM) { unseen = g_SceneMan.IsUnseen((*itr).m_Pos.m_X, (*itr).m_Pos.m_Y, team); }

			if (WithinBox((*itr).m_Pos, boxPos, boxWidth, boxHeight) && !unseen) {
				found = true;
				postEffectPosRelativeToBox = (*itr).m_Pos - boxPos;
				effectsList.push_back(PostEffect(postEffectPosRelativeToBox, (*itr).m_Bitmap, (*itr).m_BitmapHash, (*itr).m_Strength, (*itr).m_Angle));
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffects(int left, int top, int right, int bottom, std::list<PostEffect> &effectsList, int team) {
		bool found = false;
		bool unseen = false;
		Vector postEffectPosRelativeToBox;

		for (std::list<PostEffect>::iterator itr = m_PostSceneEffects.begin(); itr != m_PostSceneEffects.end(); ++itr) {
			if (team != Activity::NOTEAM) { unseen = g_SceneMan.IsUnseen((*itr).m_Pos.m_X, (*itr).m_Pos.m_Y, team); }
				
			if (WithinBox((*itr).m_Pos, left, top, right, bottom) && !unseen) {
				found = true;
				postEffectPosRelativeToBox = Vector((*itr).m_Pos.m_X - left, (*itr).m_Pos.m_Y - top);
				effectsList.push_back(PostEffect(postEffectPosRelativeToBox, (*itr).m_Bitmap, (*itr).m_BitmapHash, (*itr).m_Strength, (*itr).m_Angle));
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * PostProcessMan::GetDotGlowEffect(DotGlowColor whichColor) const {
		switch (whichColor) {
			case NoDot:
				return 0;
			case YellowDot:
				return m_YellowGlow;
			case RedDot:
				return m_RedGlow;
			case BlueDot:
				return m_BlueGlow;
			default:
				RTEAbort("Undefined glow dot color value passed in. See DotGlowColor enumeration for defined values.");
				return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t PostProcessMan::GetDotGlowEffectHash(DotGlowColor whichColor) const {
		switch (whichColor) {
			case NoDot:
				return 0;
			case YellowDot:
				return m_YellowGlowHash;
			case RedDot:
				return m_RedGlowHash;
			case BlueDot:
				return m_BlueGlowHash;
			default:
				RTEAbort("Undefined glow dot color value passed in. See DotGlowColor enumeration for defined values.");
				return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::PostProcess() {
		// First copy the current 8bpp backbuffer to the 32bpp buffer; we'll add effects to it
		blit(g_FrameMan.GetBackBuffer8(), g_FrameMan.GetBackBuffer32(), 0, 0, 0, 0, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h);

		// Set the screen blender mode for glows
		set_screen_blender(128, 128, 128, 128);

		// Reference. Do not remove.
		//acquire_bitmap(m_BackBuffer8);
		//acquire_bitmap(m_BackBuffer32);

		DrawDotGlowEffects();
		DrawPostScreenEffects();

		// Reference. Do not remove.
		//release_bitmap(m_BackBuffer32);
		//release_bitmap(m_BackBuffer8);
		// Set blender mode back??
		//set_trans_blender(128, 128, 128, 128);

		// Clear the effects list for this frame
		m_PostScreenEffects.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::DrawDotGlowEffects() {
		int x = 0;
		int y = 0;
		int startX = 0;
		int startY = 0;
		int endX = 0;
		int endY = 0;
		unsigned short testpixel = 0;

		// Randomly sample the entire backbuffer, looking for pixels to put a glow on.
		// NOTE THIS IS SLOW, especially on higher resolutions!
		for (std::list<Box>::iterator bItr = m_PostScreenGlowBoxes.begin(); bItr != m_PostScreenGlowBoxes.end(); ++bItr) {
			startX = (*bItr).m_Corner.m_X;
			startY = (*bItr).m_Corner.m_Y;
			endX = startX + (*bItr).m_Width;
			endY = startY + (*bItr).m_Height;
			testpixel = 0;

			// Sanity check a little at least
			if (startX < 0 || startX >= g_FrameMan.GetBackBuffer8()->w || startY < 0 || startY >= g_FrameMan.GetBackBuffer8()->h ||
				endX < 0 || endX >= g_FrameMan.GetBackBuffer8()->w || endY < 0 || endY >= g_FrameMan.GetBackBuffer8()->h) {
				continue;
			}

#ifdef DEBUG_BUILD
			// Draw a rectangle around the glow box so we see it's position and size
			rect(m_BackBuffer32, startX, startY, endX, endY, g_RedColor);
#endif
			
			for (y = startY; y < endY; ++y) {
				for (x = startX; x < endX; ++x) {
					testpixel = _getpixel(g_FrameMan.GetBackBuffer8(), x, y);

					// YELLOW
					if ((testpixel == g_YellowGlowColor && PosRand() < 0.9) || testpixel == 98 || (testpixel == 120 && PosRand() < 0.7)) {
						draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_YellowGlow, x - 2, y - 2);
					}
					// TODO: Enable and add more colors once we actually have something that needs these.
					// RED
					/*
					if (testpixel == 13) {
						draw_trans_sprite(m_BackBuffer32, m_RedGlow, x - 2, y - 2);
					}
					// BLUE
					if (testpixel == 166) {
						draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_BlueGlow, x - 2, y - 2);
					}
					*/
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::DrawPostScreenEffects() {
		BITMAP *pBitmap = 0;
		int effectPosX = 0;
		int effectPosY = 0;
		int strength = 0;
		float angle = 0;

		for (std::list<PostEffect>::iterator eItr = m_PostScreenEffects.begin(); eItr != m_PostScreenEffects.end(); ++eItr) {
			if ((*eItr).m_Bitmap) {
				pBitmap = (*eItr).m_Bitmap;
				strength = (*eItr).m_Strength;
				set_screen_blender(strength, strength, strength, strength);
				effectPosX = (*eItr).m_Pos.GetFloorIntX() - (pBitmap->w / 2);
				effectPosY = (*eItr).m_Pos.GetFloorIntY() - (pBitmap->h / 2);
				angle = (*eItr).m_Angle;

				// Draw all the scene screen effects accumulated this frame
				if (angle == 0) {
					draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pBitmap, effectPosX, effectPosY);
				} else {
					BITMAP * pTargetBitmap;

					if (pBitmap->w < 16 && pBitmap->h < 16) {
						pTargetBitmap = m_TempEffectBitmap_16;
					} else if (pBitmap->w < 32 && pBitmap->h < 32) {
						pTargetBitmap = m_TempEffectBitmap_32;
					} else if (pBitmap->w < 64 && pBitmap->h < 64) {
						pTargetBitmap = m_TempEffectBitmap_64;
					} else if (pBitmap->w < 128 && pBitmap->h < 128) {
						pTargetBitmap = m_TempEffectBitmap_128;
					} else if (pBitmap->w < 256 && pBitmap->h < 256) {
						pTargetBitmap = m_TempEffectBitmap_256;
					} else {
						pTargetBitmap = m_TempEffectBitmap_512;
					}

					clear_to_color(pTargetBitmap, 0);

					Matrix newAngle;
					newAngle.SetRadAngle(angle);

					rotate_sprite(pTargetBitmap, pBitmap, 0, 0, ftofix(newAngle.GetAllegroAngle()));
					draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pTargetBitmap, effectPosX, effectPosY);
				}
			}
		}
	}
}