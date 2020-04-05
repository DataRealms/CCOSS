#include "PrimitiveMan.h"
#include "FrameMan.h"
#include "SceneMan.h"
#include "Entity.h"
#include "MOSprite.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

	const std::string PrimitiveMan::c_ClassName = "PrimitiveMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::GraphicalPrimitive::TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos) {
		drawLeftPos = scenePos;
		drawRightPos = scenePos;

		if (g_SceneMan.SceneWrapsX()) {
			int sceneWidth = g_SceneMan.GetSceneWidth();
			if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth / 2) { targetPos.m_X = targetPos.m_X - sceneWidth; }
			drawLeftPos.m_X = (drawLeftPos.m_X > 0) ? drawLeftPos.m_X -= sceneWidth : drawLeftPos.m_X -= sceneWidth + targetPos.m_X;
		}
		drawLeftPos.m_X -= targetPos.m_X;
		drawRightPos.m_X -= targetPos.m_X;

		if (g_SceneMan.SceneWrapsY()) {
			int sceneHeight = g_SceneMan.GetSceneHeight();
			if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight / 2) { targetPos.m_Y = targetPos.m_Y - sceneHeight; }
			drawLeftPos.m_Y = (drawLeftPos.m_Y > 0) ? drawLeftPos.m_Y -= sceneHeight : drawLeftPos.m_Y -= sceneHeight + targetPos.m_Y;
		}
		drawLeftPos.m_Y -= targetPos.m_Y;
		drawRightPos.m_Y -= targetPos.m_Y;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::LinePrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			line(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			line(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			line(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::BoxPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rect(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rect(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			rect(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::BoxFillPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rectfill(pDrawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rectfill(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			rectfill(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::CirclePrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circle(pDrawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circle(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
			circle(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::CircleFillPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circlefill(pDrawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circlefill(pDrawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
			circlefill(pDrawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::TextPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			AllegroBitmap pPlayerGUIBitmap(pDrawScreen);

			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			AllegroBitmap pPlayerGUIBitmap(pDrawScreen);
			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
				g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
				g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::BitmapPrimitive::Draw(BITMAP *pDrawScreen, Vector targetPos) {
		if (!m_pBitmap) {
			return;
		}
		Vector pos[2];

		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			pos[0] = drawStart;
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			pos[0] = drawStartLeft;
			pos[1] = drawStartRight;
		}
		Matrix rotation = Matrix(m_RotAngle);

		for (unsigned short i = 0; i < 2; i++) {
			// Take into account the h-flipped pivot point
			pivot_scaled_sprite(pDrawScreen, m_pBitmap, pos[i].GetFloorIntX(), pos[i].GetFloorIntY(), m_pBitmap->w / 2, m_pBitmap->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(Vector start, Entity * pEntity, float rotAngle, unsigned short frame) {
		const MOSprite *pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS) {
			BITMAP *pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap) { m_Primitives.push_back(new BitmapPrimitive(start, pBitmap, rotAngle)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(short player, Vector start, Entity * pEntity, float rotAngle, unsigned short frame) {
		const MOSprite *pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS) {
			BITMAP *pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap) { m_Primitives.push_back(new BitmapPrimitive(player, start, pBitmap, rotAngle)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::ClearPrimitivesList() {
		for (std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it) {
			delete (*it);
		}
		m_Primitives.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(short player, BITMAP *pTargetBitmap, const Vector &targetPos) {
		for (std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it) {
			if (player == (*it)->m_Player || (*it)->m_Player == -1) { (*it)->Draw(pTargetBitmap, targetPos); }
		}
	}
}