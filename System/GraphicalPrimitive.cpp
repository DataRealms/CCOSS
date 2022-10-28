#include "GraphicalPrimitive.h"
#include "Matrix.h"
#include "FrameMan.h"
#include "SceneMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GraphicalPrimitive::TranslateCoordinates(Vector targetPos, const Vector &scenePos, Vector &drawLeftPos, Vector &drawRightPos) const {
		drawLeftPos = scenePos;
		drawRightPos = scenePos;

		if (g_SceneMan.SceneWrapsX()) {
			float sceneWidth = static_cast<float>(g_SceneMan.GetSceneWidth());
			if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth / 2) { targetPos.m_X -= sceneWidth; }
			drawLeftPos.m_X = (drawLeftPos.m_X > 0) ? (drawLeftPos.m_X -= sceneWidth) : (drawLeftPos.m_X -= sceneWidth + targetPos.m_X);
		}
		drawLeftPos.m_X -= targetPos.m_X;
		drawRightPos.m_X -= targetPos.m_X;

		if (g_SceneMan.SceneWrapsY()) {
			float sceneHeight = static_cast<float>(g_SceneMan.GetSceneHeight());
			if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight / 2) { targetPos.m_Y -= sceneHeight; }
			drawLeftPos.m_Y = (drawLeftPos.m_Y > 0) ? (drawLeftPos.m_Y -= sceneHeight) : (drawLeftPos.m_Y -= sceneHeight + targetPos.m_Y);
		}
		drawLeftPos.m_Y -= targetPos.m_Y;
		drawRightPos.m_Y -= targetPos.m_Y;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LinePrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			line(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			line(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY(), m_Color);
			line(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ArcPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			if (m_Thickness > 1) {
				for (int i = 0; i < m_Thickness; i++) {
					arc(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
				}
			} else {
				arc(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), m_Radius, m_Color);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			if (m_Thickness > 1) {
				for (int i = 0; i < m_Thickness; i++){
					arc(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
					arc(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), (m_Radius - (m_Thickness / 2)) + i, m_Color);
				}
			} else {
				arc(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), m_Radius, m_Color);
				arc(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), ftofix(GetAllegroAngle(m_StartAngle)), ftofix(GetAllegroAngle(m_EndAngle)), m_Radius, m_Color);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SplinePrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawGuideA = m_GuidePointAPos - targetPos;
			Vector drawGuideB = m_GuidePointBPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			std::array<int, 8> guidePoints = { drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawGuideA.GetFloorIntX(), drawGuideA.GetFloorIntY(), drawGuideB.GetFloorIntX(), drawGuideB.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY() };
			spline(drawScreen, guidePoints.data(), m_Color);
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

			std::array<int, 8> guidePointsLeft = { drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawGuideALeft.GetFloorIntX(), drawGuideALeft.GetFloorIntY(), drawGuideBLeft.GetFloorIntX(), drawGuideBLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY() };
			std::array<int, 8> guidePointsRight = { drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawGuideARight.GetFloorIntX(), drawGuideARight.GetFloorIntY(), drawGuideBRight.GetFloorIntX(), drawGuideBRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY() };
			spline(drawScreen, guidePointsLeft.data(), m_Color);
			spline(drawScreen, guidePointsRight.data(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BoxPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rect(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rect(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY(), m_Color);
			rect(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BoxFillPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;
			rectfill(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			rectfill(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY(), m_Color);
			rectfill(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RoundedBoxPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (m_StartPos.m_X > m_EndPos.m_X) { std::swap(m_StartPos.m_X, m_EndPos.m_X); }
		if (m_StartPos.m_Y > m_EndPos.m_Y) { std::swap(m_StartPos.m_Y, m_EndPos.m_Y); }

		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			arc(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.GetFloorIntX() - m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);

			hline(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - m_CornerRadius, m_Color);
			hline(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY(), drawEnd.GetFloorIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_Color);
			vline(drawScreen, drawEnd.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			arc(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.GetFloorIntX() - m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndLeft.GetFloorIntX() - m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawEndLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_Color);
			vline(drawScreen, drawEndLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_Color);

			arc(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, itofix(64), itofix(128), m_CornerRadius, m_Color);
			arc(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, itofix(128), itofix(-64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.GetFloorIntX() - m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, itofix(0), itofix(64), m_CornerRadius, m_Color);
			arc(drawScreen, drawEndRight.GetFloorIntX() - m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, itofix(-64), itofix(0), m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - m_CornerRadius, m_Color);
			hline(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawEndRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - m_CornerRadius, m_Color);
			vline(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_Color);
			vline(drawScreen, drawEndRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RoundedBoxFillPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (m_StartPos.m_X > m_EndPos.m_X) { std::swap(m_StartPos.m_X, m_EndPos.m_X); }
		if (m_StartPos.m_Y > m_EndPos.m_Y) { std::swap(m_StartPos.m_Y, m_EndPos.m_Y); }

		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			Vector drawEnd = m_EndPos - targetPos;

			circlefill(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.GetFloorIntX() - m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);

			rectfill(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY() - m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY(), m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawEndLeft;
			Vector drawStartRight;
			Vector drawEndRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

			circlefill(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.GetFloorIntX() - m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndLeft.GetFloorIntX() - m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY() - m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - m_CornerRadius, drawEndLeft.GetFloorIntY(), m_Color);

			circlefill(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.GetFloorIntX() - m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, m_CornerRadius, m_Color);
			circlefill(drawScreen, drawEndRight.GetFloorIntX() - m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY() - m_CornerRadius, m_Color);
			rectfill(drawScreen, drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - m_CornerRadius, drawEndRight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CirclePrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circle(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circle(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Radius, m_Color);
			circle(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CircleFillPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			circlefill(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			circlefill(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Radius, m_Color);
			circlefill(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Radius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void EllipsePrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipse(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipse(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
			ellipse(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void EllipseFillPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			ellipsefill(drawScreen, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			ellipsefill(drawScreen, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
			ellipsefill(drawScreen, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_HorizRadius, m_VertRadius, m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TrianglePrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			line(drawScreen, drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), m_Color);
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

			line(drawScreen, drawPointALeft.GetFloorIntX(), drawPointALeft.GetFloorIntY(), drawPointBLeft.GetFloorIntX(), drawPointBLeft.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointARight.GetFloorIntX(), drawPointARight.GetFloorIntY(), drawPointBRight.GetFloorIntX(), drawPointBRight.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointBLeft.GetFloorIntX(), drawPointBLeft.GetFloorIntY(), drawPointCLeft.GetFloorIntX(), drawPointCLeft.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointBRight.GetFloorIntX(), drawPointBRight.GetFloorIntY(), drawPointCRight.GetFloorIntX(), drawPointCRight.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointCLeft.GetFloorIntX(), drawPointCLeft.GetFloorIntY(), drawPointALeft.GetFloorIntX(), drawPointALeft.GetFloorIntY(), m_Color);
			line(drawScreen, drawPointCRight.GetFloorIntX(), drawPointCRight.GetFloorIntY(), drawPointARight.GetFloorIntX(), drawPointARight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TriangleFillPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawPointA = m_PointAPos - targetPos;
			Vector drawPointB = m_PointBPos - targetPos;
			Vector drawPointC = m_PointCPos - targetPos;
			triangle(drawScreen, drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), m_Color);
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

			triangle(drawScreen, drawPointALeft.GetFloorIntX(), drawPointALeft.GetFloorIntY(), drawPointBLeft.GetFloorIntX(), drawPointBLeft.GetFloorIntY(), drawPointCLeft.GetFloorIntX(), drawPointCLeft.GetFloorIntY(), m_Color);
			triangle(drawScreen, drawPointARight.GetFloorIntX(), drawPointARight.GetFloorIntY(), drawPointBRight.GetFloorIntX(), drawPointBRight.GetFloorIntY(), drawPointCRight.GetFloorIntX(), drawPointCRight.GetFloorIntY(), m_Color);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TextPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			AllegroBitmap playerGUIBitmap(drawScreen);

			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Text, m_Alignment);
			}
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			AllegroBitmap playerGUIBitmap(drawScreen);
			if (m_IsSmall) {
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Text, m_Alignment);
				g_FrameMan.GetSmallFont()->DrawAligned(&playerGUIBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Text, m_Alignment);
			} else {
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Text, m_Alignment);
				g_FrameMan.GetLargeFont()->DrawAligned(&playerGUIBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Text, m_Alignment);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void BitmapPrimitive::Draw(BITMAP *drawScreen, const Vector &targetPos) {
		if (!m_Bitmap) {
			return;
		}

		BITMAP *bitmapToDraw = create_bitmap_ex(8, m_Bitmap->w, m_Bitmap->h);
		clear_to_color(bitmapToDraw, 0);
		draw_sprite(bitmapToDraw, m_Bitmap, 0, 0);

		if (m_HFlipped || m_VFlipped) {
			BITMAP *flipBitmap = create_bitmap_ex(8, bitmapToDraw->w, bitmapToDraw->h);
			clear_to_color(flipBitmap, 0);

			if (m_HFlipped && !m_VFlipped) {
				draw_sprite_h_flip(flipBitmap, bitmapToDraw, 0, 0);
			} else if (!m_HFlipped && m_VFlipped) {
				draw_sprite_v_flip(flipBitmap, bitmapToDraw, 0, 0);
			} else if (m_HFlipped && m_VFlipped) {
				draw_sprite_vh_flip(flipBitmap, bitmapToDraw, 0, 0);
			}

			blit(flipBitmap, bitmapToDraw, 0, 0, 0, 0, bitmapToDraw->w, bitmapToDraw->h);
			destroy_bitmap(flipBitmap);
		}

		Matrix rotation = Matrix(m_RotAngle);

		if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
			Vector drawStart = m_StartPos - targetPos;
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		} else {
			Vector drawStartLeft;
			Vector drawStartRight;

			TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

			// Take into account the h-flipped pivot point
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		}

		destroy_bitmap(bitmapToDraw);
	}
}