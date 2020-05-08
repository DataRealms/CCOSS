#include "Primitive.h"
#include "FrameMan.h"
#include "SceneMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GraphicalPrimitive::TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos) {
		drawLeftPos = scenePos;
		drawRightPos = scenePos;

		if (g_SceneMan.SceneWrapsX()) {
			int sceneWidth = g_SceneMan.GetSceneWidth();
			if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth / 2) { targetPos.m_X -= sceneWidth; }
			drawLeftPos.m_X = (drawLeftPos.m_X > 0) ? (drawLeftPos.m_X -= sceneWidth) : (drawLeftPos.m_X -= sceneWidth + targetPos.m_X);
		}
		drawLeftPos.m_X -= targetPos.m_X;
		drawRightPos.m_X -= targetPos.m_X;

		if (g_SceneMan.SceneWrapsY()) {
			int sceneHeight = g_SceneMan.GetSceneHeight();
			if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight / 2) { targetPos.m_Y -= sceneHeight; }
			drawLeftPos.m_Y = (drawLeftPos.m_Y > 0) ? (drawLeftPos.m_Y -= sceneHeight) : (drawLeftPos.m_Y -= sceneHeight + targetPos.m_Y);
		}
		drawLeftPos.m_Y -= targetPos.m_Y;
		drawRightPos.m_Y -= targetPos.m_Y;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LinePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			line(drawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			line(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			line(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BoxPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rect(drawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rect(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			rect(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BoxFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rectfill(drawScreen, drawStart.m_X, drawStart.m_Y, drawEnd.m_X, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rectfill(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, drawEndLeft.m_X, drawEndLeft.m_Y, m_Color);
			rectfill(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, drawEndRight.m_X, drawEndRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CirclePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circle(drawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circle(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
			circle(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CircleFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circlefill(drawScreen, drawStart.m_X, drawStart.m_Y, m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circlefill(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_Radius, m_Color);
			circlefill(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TextPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			AllegroBitmap pPlayerGUIBitmap(drawScreen);

			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			AllegroBitmap pPlayerGUIBitmap(drawScreen);
			switch (m_IsSmall) {
				case false:
					g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
					g_FrameMan.GetLargeFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
					break;
				case true:
					g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
					g_FrameMan.GetSmallFont()->DrawAligned(&pPlayerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
					break;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BitmapPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!m_Bitmap) {
			return;
		}
		Matrix rotation = Matrix(m_RotAngle);

		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			pivot_scaled_sprite(drawScreen, m_Bitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Bitmap->w / 2, m_Bitmap->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			// Take into account the h-flipped pivot point
			pivot_scaled_sprite(drawScreen, m_Bitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Bitmap->w / 2, m_Bitmap->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
			pivot_scaled_sprite(drawScreen, m_Bitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Bitmap->w / 2, m_Bitmap->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		}
	}
}