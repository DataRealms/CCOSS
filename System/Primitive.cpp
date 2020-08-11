#include "Primitive.h"
#include "Matrix.h"
#include "FrameMan.h"
#include "SceneMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GraphicalPrimitive::TranslateCoordinates(Vector targetPos, Vector scenePos, Vector & drawLeftPos, Vector & drawRightPos) const {
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
			line(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), drawEnd.GetIntX(), drawEnd.GetIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			line(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), drawEndLeft.GetIntX(), drawEndLeft.GetIntY(), m_Color);
			line(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), drawEndRight.GetIntX(), drawEndRight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ArcPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			if (m_Thickness > 1) {
				for (short i = 0; i < m_Thickness; i++) {
					// Start and End angles are negative and reversed to compensate for Allegro's way of drawing arcs (counter-clockwise and weird) so we get nice clockwise drawing.
					arc(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
				}
			} else {
				arc(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			if (m_Thickness > 1) {
				for (short i = 0; i < m_Thickness; i++){
					arc(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
					arc(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
				}
			} else {
				arc(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
				arc(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), ftofix(GetAllegroAngle(-m_EndAngle)), ftofix(GetAllegroAngle(-m_StartAngle)), m_Radius, m_Color);
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
			rect(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), drawEnd.GetIntX(), drawEnd.GetIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rect(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), drawEndLeft.GetIntX(), drawEndLeft.GetIntY(), m_Color);
			rect(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), drawEndRight.GetIntX(), drawEndRight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BoxFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rectfill(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), drawEnd.GetIntX(), drawEnd.GetIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rectfill(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), drawEndLeft.GetIntX(), drawEndLeft.GetIntY(), m_Color);
			rectfill(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), drawEndRight.GetIntX(), drawEndRight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RoundedBoxPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			// Draw the top left, bottom left, top right and bottom right corners respectively
			arc(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawStart.GetIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.GetIntX() - m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.GetIntX() - m_CornerRadius, drawStart.GetIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);

			//Draw the top, bottom, left and right planes respectively
			hline(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawStart.GetIntY(), drawEnd.GetIntX() - m_CornerRadius, m_Color);	
			hline(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawEnd.GetIntY(), drawEnd.GetIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStart.GetIntX(), drawStart.GetIntY() - m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, m_Color);
			vline(drawScreen, drawEnd.GetIntX(), drawStart.GetIntY() - m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			arc(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawStartLeft.GetIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.GetIntX() - m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.GetIntX() - m_CornerRadius, drawStartLeft.GetIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawStartLeft.GetIntY(), drawEndLeft.GetIntX() - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawEndLeft.GetIntY(), drawEndLeft.GetIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY() - m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, m_Color);
			vline(drawScreen, drawEndLeft.GetIntX(), drawStartLeft.GetIntY() - m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, m_Color);

			arc(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawStartRight.GetIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.GetIntX() - m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.GetIntX() - m_CornerRadius, drawStartRight.GetIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawStartRight.GetIntY(), drawEndRight.GetIntX() - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawEndRight.GetIntY(), drawEndRight.GetIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY() - m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, m_Color);
			vline(drawScreen, drawEndRight.GetIntX(), drawStartRight.GetIntY() - m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RoundedBoxFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			
			// Draw the top left, bottom left, top right and bottom right corners respectively
			circlefill(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawStart.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.GetIntX() - m_CornerRadius, drawEnd.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.GetIntX() - m_CornerRadius, drawStart.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);

			rectfill(drawScreen, drawStart.GetIntX(), drawStart.GetIntY() - m_CornerRadius, drawEnd.GetIntX(), drawEnd.GetIntY() + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStart.GetIntX() + m_CornerRadius, drawStart.GetIntY(), drawEnd.GetIntX() - m_CornerRadius, drawEnd.GetIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			circlefill(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawStartLeft.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.GetIntX() - m_CornerRadius, drawEndLeft.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.GetIntX() - m_CornerRadius, drawStartLeft.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY() - m_CornerRadius, drawEndLeft.GetIntX(), drawEndLeft.GetIntY() + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.GetIntX() + m_CornerRadius, drawStartLeft.GetIntY(), drawEndLeft.GetIntX() - m_CornerRadius, drawEndLeft.GetIntY(), m_Color);

			circlefill(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawStartRight.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.GetIntX() - m_CornerRadius, drawEndRight.GetIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.GetIntX() - m_CornerRadius, drawStartRight.GetIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY() - m_CornerRadius, drawEndRight.GetIntX(), drawEndRight.GetIntY() + m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.GetIntX() + m_CornerRadius, drawStartRight.GetIntY(), drawEndRight.GetIntX() - m_CornerRadius, drawEndRight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CirclePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circle(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circle(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_Radius, m_Color);
			circle(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CircleFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circlefill(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circlefill(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_Radius, m_Color);
			circlefill(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void EllipsePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipse(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipse(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
			ellipse(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void EllipseFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipsefill(drawScreen, drawStart.GetIntX(), drawStart.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipsefill(drawScreen, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
			ellipsefill(drawScreen, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TrianglePrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			line(drawScreen, drawPointA.GetIntX(), drawPointA.GetIntY(), drawPointB.GetIntX(), drawPointB.GetIntY(), m_Color);
			line(drawScreen, drawPointB.GetIntX(), drawPointB.GetIntY(), drawPointC.GetIntX(), drawPointC.GetIntY(), m_Color);
			line(drawScreen, drawPointC.GetIntX(), drawPointC.GetIntY(), drawPointA.GetIntX(), drawPointA.GetIntY(), m_Color);
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

			line(drawScreen, drawPointALeft.GetIntX(), drawPointALeft.GetIntY(), drawPointBLeft.GetIntX(), drawPointBLeft.GetIntY(), m_Color);
			line(drawScreen, drawPointARight.GetIntX(), drawPointARight.GetIntY(), drawPointBRight.GetIntX(), drawPointBRight.GetIntY(), m_Color);
			line(drawScreen, drawPointBLeft.GetIntX(), drawPointBLeft.GetIntY(), drawPointCLeft.GetIntX(), drawPointCLeft.GetIntY(), m_Color);
			line(drawScreen, drawPointBRight.GetIntX(), drawPointBRight.GetIntY(), drawPointCRight.GetIntX(), drawPointCRight.GetIntY(), m_Color);
			line(drawScreen, drawPointCLeft.GetIntX(), drawPointCLeft.GetIntY(), drawPointALeft.GetIntX(), drawPointALeft.GetIntY(), m_Color);
			line(drawScreen, drawPointCRight.GetIntX(), drawPointCRight.GetIntY(), drawPointARight.GetIntX(), drawPointARight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TriangleFillPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			triangle(drawScreen, drawPointA.GetIntX(), drawPointA.GetIntY(), drawPointB.GetIntX(), drawPointB.GetIntY(), drawPointC.GetIntX(), drawPointC.GetIntY(), m_Color);
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

			triangle(drawScreen, drawPointALeft.GetIntX(), drawPointALeft.GetIntY(), drawPointBLeft.GetIntX(), drawPointBLeft.GetIntY(), drawPointCLeft.GetIntX(), drawPointCLeft.GetIntY(), m_Color);
			triangle(drawScreen, drawPointARight.GetIntX(), drawPointARight.GetIntY(), drawPointBRight.GetIntX(), drawPointBRight.GetIntY(), drawPointCRight.GetIntX(), drawPointCRight.GetIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TextPrimitive::Draw(BITMAP *drawScreen, Vector targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			AllegroBitmap playerGUIBitmap(drawScreen);

			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStart.GetIntX(), drawStart.GetIntY(), m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStart.GetIntX(), drawStart.GetIntY(), m_Text, m_Alignment);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			AllegroBitmap playerGUIBitmap(drawScreen);
			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_Text, m_Alignment);
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.GetIntX(), drawStartLeft.GetIntY(), m_Text, m_Alignment);
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartRight.GetIntX(), drawStartRight.GetIntY(), m_Text, m_Alignment);
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