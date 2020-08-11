#include "Primitive.h"
#include "Matrix.h"
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

	void ArcPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			if (m_Thickness > 1) {
				for (short i = 0; i < m_Thickness; i++) {
					arc(drawScreen, drawStart.m_X, drawStart.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
					// Start and End angles are negative and reversed to compensate for Allegro's way of drawing arcs (counter-clockwise and weird) so we get nice clockwise drawing.
				}
			} else {
				arc(drawScreen, drawStart.m_X, drawStart.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			if (m_Thickness > 1) {
				for (short i = 0; i < m_Thickness; i++){
					arc(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
					arc(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
				}
			} else {
				arc(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
				arc(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SplinePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawGuideA = m_GuidePointAPos - targetPos;
			Vector drawGuideB = m_GuidePointBPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			int guidePoints[8] = { drawStart.GetIntX(), drawStart.GetIntY(), drawGuideA.GetIntX(), drawGuideA.GetIntY(), drawGuideB.GetIntX(), drawGuideB.GetIntY(), drawEnd.GetIntX(), drawEnd.GetIntY() };
			spline(drawScreen, guidePoints, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawGuideALeft;
			Vector drawGuideBLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawGuideARight;
			Vector drawGuideBRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_GuidePointAPos, drawGuideALeft, drawGuideARight);
			TranslateCoordinates(targetPos, m_GuidePointBPos, drawGuideBLeft, drawGuideBRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			int guidePointsLeft[8] = { drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), drawGuideALeft.GetIntX(), drawGuideALeft.GetIntY(), drawGuideBLeft.GetIntX(), drawGuideBLeft.GetIntY(), drawEndLeft.GetIntX(), drawEndLeft.GetIntY() };
			int guidePointsRight[8] = { drawStartRight.GetIntX(), drawStartRight.GetIntY(), drawGuideARight.GetIntX(), drawGuideARight.GetIntY(), drawGuideBRight.GetIntX(), drawGuideBRight.GetIntY(), drawEndRight.GetIntX(), drawEndRight.GetIntY() };

			spline(drawScreen, guidePointsLeft, m_Color);
			spline(drawScreen, guidePointsRight, m_Color);
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

	void RoundedBoxPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			// Draw the top left, bottom left, top right and bottom right corners respectively
			arc(drawScreen, drawStart.m_X + m_CornerRadius, drawEnd.m_Y + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStart.m_X + m_CornerRadius, drawStart.m_Y - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.m_X - m_CornerRadius, drawEnd.m_Y + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.m_X - m_CornerRadius, drawStart.m_Y - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);

			//Draw the top, bottom, left and right planes respectively
			hline(drawScreen, drawStart.m_X + m_CornerRadius, drawStart.m_Y, drawEnd.m_X - m_CornerRadius, m_Color);	
			hline(drawScreen, drawStart.m_X + m_CornerRadius, drawEnd.m_Y, drawEnd.m_X - m_CornerRadius, m_Color);
			vline(drawScreen, drawStart.m_X, drawStart.m_Y - m_CornerRadius, drawEnd.m_Y + m_CornerRadius, m_Color);
			vline(drawScreen, drawEnd.m_X, drawStart.m_Y - m_CornerRadius, drawEnd.m_Y + m_CornerRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			arc(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawStartLeft.m_Y - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.m_X - m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.m_X - m_CornerRadius, drawStartLeft.m_Y - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawStartLeft.m_Y, drawEndLeft.m_X - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawEndLeft.m_Y, drawEndLeft.m_X - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y - m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, m_Color);
			vline(drawScreen, drawEndLeft.m_X, drawStartLeft.m_Y - m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, m_Color);

			arc(drawScreen, drawStartRight.m_X + m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartRight.m_X + m_CornerRadius, drawStartRight.m_Y - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.m_X - m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.m_X - m_CornerRadius, drawStartRight.m_Y - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.m_X + m_CornerRadius, drawStartRight.m_Y, drawEndRight.m_X - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.m_X + m_CornerRadius, drawEndRight.m_Y, drawEndRight.m_X - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartRight.m_X, drawStartRight.m_Y - m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, m_Color);
			vline(drawScreen, drawEndRight.m_X, drawStartRight.m_Y - m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RoundedBoxFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			
			// Draw the top left, bottom left, top right and bottom right corners respectively
			circlefill(drawScreen, drawStart.m_X + m_CornerRadius, drawEnd.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStart.m_X + m_CornerRadius, drawStart.m_Y - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.m_X - m_CornerRadius, drawEnd.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.m_X - m_CornerRadius, drawStart.m_Y - m_CornerRadius, m_CornerRadius, m_Color);

			rectfill(drawScreen, drawStart.m_X, drawStart.m_Y - m_CornerRadius, drawEnd.m_X, drawEnd.m_Y + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStart.m_X + m_CornerRadius, drawStart.m_Y, drawEnd.m_X - m_CornerRadius, drawEnd.m_Y, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			circlefill(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawStartLeft.m_Y - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.m_X - m_CornerRadius, drawEndLeft.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.m_X - m_CornerRadius, drawStartLeft.m_Y - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y - m_CornerRadius, drawEndLeft.m_X, drawEndLeft.m_Y + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.m_X + m_CornerRadius, drawStartLeft.m_Y, drawEndLeft.m_X - m_CornerRadius, drawEndLeft.m_Y, m_Color);

			circlefill(drawScreen, drawStartRight.m_X + m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartRight.m_X + m_CornerRadius, drawStartRight.m_Y - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.m_X - m_CornerRadius, drawEndRight.m_Y + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.m_X - m_CornerRadius, drawStartRight.m_Y - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.m_X, drawStartRight.m_Y - m_CornerRadius, drawEndRight.m_X, drawEndRight.m_Y + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.m_X + m_CornerRadius, drawStartRight.m_Y, drawEndRight.m_X - m_CornerRadius, drawEndRight.m_Y, m_Color);
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

	void EllipsePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipse(drawScreen, drawStart.m_X, drawStart.m_Y, m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipse(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_HorizRadius, m_VertRadius, m_Color);
			ellipse(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void EllipseFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipsefill(drawScreen, drawStart.m_X, drawStart.m_Y, m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipsefill(drawScreen, drawStartLeft.m_X, drawStartLeft.m_Y, m_HorizRadius, m_VertRadius, m_Color);
			ellipsefill(drawScreen, drawStartRight.m_X, drawStartRight.m_Y, m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TrianglePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			line(drawScreen, drawPointA.m_X, drawPointA.m_Y, drawPointB.m_X, drawPointB.m_Y, m_Color);
			line(drawScreen, drawPointB.m_X, drawPointB.m_Y, drawPointC.m_X, drawPointC.m_Y, m_Color);
			line(drawScreen, drawPointC.m_X, drawPointC.m_Y, drawPointA.m_X, drawPointA.m_Y, m_Color);
		} else {
			Vector drawPointALeft;
			Vector drawPointBLeft;
			Vector drawPointCLeft;
			Vector drawPointARight;
			Vector drawPointBRight;
			Vector drawPointCRight;

			TranslateCoordinates(targetPos, m_PointAPos, drawPointALeft, drawPointARight);
			TranslateCoordinates(targetPos, m_PointBPos, drawPointBLeft, drawPointBRight);
			TranslateCoordinates(targetPos, m_PointCPos, drawPointCLeft, drawPointCRight);

			line(drawScreen, drawPointALeft.m_X, drawPointALeft.m_Y, drawPointBLeft.m_X, drawPointBLeft.m_Y, m_Color);
			line(drawScreen, drawPointARight.m_X, drawPointARight.m_Y, drawPointBRight.m_X, drawPointBRight.m_Y, m_Color);
			line(drawScreen, drawPointBLeft.m_X, drawPointBLeft.m_Y, drawPointCLeft.m_X, drawPointCLeft.m_Y, m_Color);
			line(drawScreen, drawPointBRight.m_X, drawPointBRight.m_Y, drawPointCRight.m_X, drawPointCRight.m_Y, m_Color);
			line(drawScreen, drawPointCLeft.m_X, drawPointCLeft.m_Y, drawPointALeft.m_X, drawPointALeft.m_Y, m_Color);
			line(drawScreen, drawPointCRight.m_X, drawPointCRight.m_Y, drawPointARight.m_X, drawPointARight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TriangleFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			triangle(drawScreen, drawPointA.m_X, drawPointA.m_Y, drawPointB.m_X, drawPointB.m_Y, drawPointC.m_X, drawPointC.m_Y, m_Color);
		} else {
			Vector drawPointALeft;
			Vector drawPointBLeft;
			Vector drawPointCLeft;
			Vector drawPointARight;
			Vector drawPointBRight;
			Vector drawPointCRight;

			TranslateCoordinates(targetPos, m_PointAPos, drawPointALeft, drawPointARight);
			TranslateCoordinates(targetPos, m_PointBPos, drawPointBLeft, drawPointBRight);
			TranslateCoordinates(targetPos, m_PointCPos, drawPointCLeft, drawPointCRight);

			triangle(drawScreen, drawPointALeft.m_X, drawPointALeft.m_Y, drawPointBLeft.m_X, drawPointBLeft.m_Y, drawPointCLeft.m_X, drawPointCLeft.m_Y, m_Color);
			triangle(drawScreen, drawPointARight.m_X, drawPointARight.m_Y, drawPointBRight.m_X, drawPointBRight.m_Y, drawPointCRight.m_X, drawPointCRight.m_Y, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TextPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			AllegroBitmap playerGUIBitmap(drawScreen);

			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStart.m_X, drawStart.m_Y, m_Text, m_Alignment);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			AllegroBitmap playerGUIBitmap(drawScreen);
			switch (m_IsSmall) {
				case false:
					g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
					g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
					break;
				case true:
					g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.m_X, drawStartLeft.m_Y, m_Text, m_Alignment);
					g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartRight.m_X, drawStartRight.m_Y, m_Text, m_Alignment);
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