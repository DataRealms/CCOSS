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
		m_pYellowGlow = 0;
		m_YellowGlowHash = 0;
		m_pRedGlow = 0;
		m_RedGlowHash = 0;
		m_pBlueGlow = 0;
		m_BlueGlowHash = 0;
		for (short i = 0; i < c_MaxScreenCount; ++i) {
			m_ScreenRelativeEffects->clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PostProcessMan::Create() {
		// TODO: Make more robust and load more glows!
		ContentFile glowFile("Base.rte/Effects/Glows/YellowTiny.bmp");
		m_pYellowGlow = glowFile.GetAsBitmap();
		m_YellowGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/RedTiny.bmp");
		m_pRedGlow = glowFile.GetAsBitmap();
		m_RedGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/BlueTiny.bmp");
		m_pBlueGlow = glowFile.GetAsBitmap();
		m_BlueGlowHash = glowFile.GetHash();

		m_pTempEffectBitmap_16 = create_bitmap_ex(32, 16, 16);
		m_pTempEffectBitmap_32 = create_bitmap_ex(32, 32, 32);
		m_pTempEffectBitmap_64 = create_bitmap_ex(32, 64, 64);
		m_pTempEffectBitmap_128 = create_bitmap_ex(32, 128, 128);
		m_pTempEffectBitmap_256 = create_bitmap_ex(32, 256, 256);
		m_pTempEffectBitmap_512 = create_bitmap_ex(32, 512, 512);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::PostProcess() {
		// First copy the current 8bpp backbuffer to the 32bpp buffer; we'll add effects to it
		blit(g_FrameMan.GetBackBuffer8(), g_FrameMan.GetBackBuffer32(), 0, 0, 0, 0, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h);

		// Set the screen blender mode for glows
		set_screen_blender(128, 128, 128, 128);

		//acquire_bitmap(m_pBackBuffer8);
		//acquire_bitmap(m_pBackBuffer32);

		// Randomly sample the entire backbuffer, looking for pixels to put a glow on
		// NOTE THIS IS SLOW, especially on higher resolutions!
		int x = 0;
		int y = 0;
		int startX = 0;
		int startY = 0;
		int endX = 0;
		int endY = 0;
		unsigned short testpixel = 0;

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
			//rect(m_pBackBuffer32, startX, startY, endX, endY, g_RedColor);
#endif

			for (y = startY; y < endY; ++y) {
				for (x = startX; x < endX; ++x) {
					testpixel = _getpixel(g_FrameMan.GetBackBuffer8(), x, y);

					// YELLOW
					if ((testpixel == g_YellowGlowColor && PosRand() < 0.9) || testpixel == 98 || (testpixel == 120 && PosRand() < 0.7)) {
						draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_pYellowGlow, x - 2, y - 2);
					}
					// RED
					/*
					if (testpixel == 13) {
						draw_trans_sprite(m_pBackBuffer32, m_pRedGlow, x - 2, y - 2);
					}
					*/
					// BLUE
					if (testpixel == 166) {
						draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_pBlueGlow, x - 2, y - 2);
					}
				}
			}
		}

		// Draw all the scene screen effects accumulated this frame
		BITMAP *pBitmap = 0;
		int effectPosX = 0;
		int effectPosY = 0;
		int strength = 0;
		float angle = 0;

		for (std::list<PostEffect>::iterator eItr = m_PostScreenEffects.begin(); eItr != m_PostScreenEffects.end(); ++eItr) {
			if ((*eItr).m_pBitmap) {
				pBitmap = (*eItr).m_pBitmap;
				strength = (*eItr).m_Strength;
				set_screen_blender(strength, strength, strength, strength);
				effectPosX = (*eItr).m_Pos.GetFloorIntX() - (pBitmap->w / 2);
				effectPosY = (*eItr).m_Pos.GetFloorIntY() - (pBitmap->h / 2);
				angle = (*eItr).m_Angle;
				//draw_trans_sprite(m_pBackBuffer32, pBitmap, effectPosX, effectPosY);

				if (angle == 0) {
					draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pBitmap, effectPosX, effectPosY);
				} else {
					BITMAP * pTargetBitmap;

					if (pBitmap->w < 16 && pBitmap->h < 16) {
						pTargetBitmap = m_pTempEffectBitmap_16;
					} else if (pBitmap->w < 32 && pBitmap->h < 32) {
						pTargetBitmap = m_pTempEffectBitmap_32;
					} else if (pBitmap->w < 64 && pBitmap->h < 64) {
						pTargetBitmap = m_pTempEffectBitmap_64;
					} else if (pBitmap->w < 128 && pBitmap->h < 128) {
						pTargetBitmap = m_pTempEffectBitmap_128;
					} else if (pBitmap->w < 256 && pBitmap->h < 256) {
						pTargetBitmap = m_pTempEffectBitmap_256;
					} else {
						pTargetBitmap = m_pTempEffectBitmap_512;
					}

					clear_to_color(pTargetBitmap, 0);

					//fixed fAngle;
					//fAngle = fixmul(angle, radtofix_r);

					Matrix m;
					m.SetRadAngle(angle);

					rotate_sprite(pTargetBitmap, pBitmap, 0, 0, ftofix(m.GetAllegroAngle()));
					draw_trans_sprite(g_FrameMan.GetBackBuffer32(), pTargetBitmap, effectPosX, effectPosY);
				}
			}
		}
		//release_bitmap(m_pBackBuffer32);
		//release_bitmap(m_pBackBuffer8);

		// Set blender mode back??
		//set_trans_blender(128, 128, 128, 128);

		// Clear the effects list for this frame
		m_PostScreenEffects.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterPostEffect(const Vector &effectPos, BITMAP *pEffect, size_t hash, int strength, float angle) {
		// These effects get applied when there's a drawn frame that followed one or more sim updates
		// They are not only registered on drawn sim updates; flashes and stuff could be missed otherwise if they occur on undrawn sim updates
		if (pEffect && /*g_TimerMan.DrawnSimUpdate()) && */g_TimerMan.SimUpdatesSinceDrawn() >= 0) {
			m_PostSceneEffects.push_back(PostEffect(effectPos, pEffect, hash, strength, angle));
		}
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

		// Check wrapped rectangles
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
			return m_pTempEffectBitmap_16;
		} else if (bitmapSize <= 32) {
			return m_pTempEffectBitmap_32;
		} else if (bitmapSize <= 64) {
			return m_pTempEffectBitmap_64;
		} else if (bitmapSize <= 128) {
			return m_pTempEffectBitmap_128;
		} else if (bitmapSize <= 256) {
			return m_pTempEffectBitmap_256;
		} else {
			return m_pTempEffectBitmap_512;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterGlowArea(const IntRect &glowArea) { if (g_TimerMan.DrawnSimUpdate() && g_TimerMan.SimUpdatesSinceDrawn() >= 0) m_GlowAreas.push_back(glowArea); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterGlowArea(const Vector &center, float radius) {
		if (g_TimerMan.DrawnSimUpdate() && g_TimerMan.SimUpdatesSinceDrawn() >= 0) {
			RegisterGlowArea(IntRect(center.m_X - radius, center.m_Y - radius, center.m_X + radius, center.m_Y + radius));
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
			outputList.push_back(PostEffect((*eItr).m_Pos, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
		}
		ScreenRelativeEffectsMutex[whichScreen].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> & inputList) {
		ScreenRelativeEffectsMutex[whichScreen].lock();
		m_ScreenRelativeEffects[whichScreen].clear();
		for (std::list<PostEffect>::iterator eItr = inputList.begin(); eItr != inputList.end(); ++eItr) {
			m_ScreenRelativeEffects[whichScreen].push_back(PostEffect((*eItr).m_Pos, (*eItr).m_pBitmap, (*eItr).m_BitmapHash, (*eItr).m_Strength, (*eItr).m_Angle));
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
				effectsList.push_back(PostEffect(postEffectPosRelativeToBox, (*itr).m_pBitmap, (*itr).m_BitmapHash, (*itr).m_Strength, (*itr).m_Angle));
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
				effectsList.push_back(PostEffect(postEffectPosRelativeToBox, (*itr).m_pBitmap, (*itr).m_BitmapHash, (*itr).m_Strength, (*itr).m_Angle));
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * PostProcessMan::GetDotGlowEffect(DotGlowColor which) const {
		switch (which) {
			case NoDot:
				return 0;
			case YellowDot:
				return m_pYellowGlow;
			case RedDot:
				return m_pRedGlow;
			case BlueDot:
				return m_pBlueGlow;
			default:
				RTEAbort("Undefined glow dot color value passed in. See DotGlowColor enumeration for defined values.");
				return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t PostProcessMan::GetDotGlowEffectHash(DotGlowColor which) const {
		switch (which) {
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
}