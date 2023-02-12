#include "PrimitiveMan.h"

#include "FrameMan.h"
#include "SceneMan.h"
#include "ConsoleMan.h"
#include "MOSprite.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::unique_ptr<GraphicalPrimitive> PrimitiveMan::MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(GraphicalPrimitive *primitive) {
		switch (primitive->GetPrimtiveType()) {
			case GraphicalPrimitive::PrimitiveType::Line:
				return std::unique_ptr<LinePrimitive>(static_cast<LinePrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Arc:
				return std::unique_ptr<ArcPrimitive>(static_cast<ArcPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Spline:
				return std::unique_ptr<SplinePrimitive>(static_cast<SplinePrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Box:
				return std::unique_ptr<BoxPrimitive>(static_cast<BoxPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::BoxFill:
				return std::unique_ptr<BoxFillPrimitive>(static_cast<BoxFillPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::RoundedBox:
				return std::unique_ptr<RoundedBoxPrimitive>(static_cast<RoundedBoxPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::RoundedBoxFill:
				return std::unique_ptr<RoundedBoxFillPrimitive>(static_cast<RoundedBoxFillPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Circle:
				return std::unique_ptr<CirclePrimitive>(static_cast<CirclePrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::CircleFill:
				return std::unique_ptr<CircleFillPrimitive>(static_cast<CircleFillPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Ellipse:
				return std::unique_ptr<EllipsePrimitive>(static_cast<EllipsePrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::EllipseFill:
				return std::unique_ptr<EllipseFillPrimitive>(static_cast<EllipseFillPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Triangle:
				return std::unique_ptr<TrianglePrimitive>(static_cast<TrianglePrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::TriangleFill:
				return std::unique_ptr<TriangleFillPrimitive>(static_cast<TriangleFillPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Text:
				return std::unique_ptr<TextPrimitive>(static_cast<TextPrimitive *>(primitive));
			case GraphicalPrimitive::PrimitiveType::Bitmap:
				return std::unique_ptr<BitmapPrimitive>(static_cast<BitmapPrimitive *>(primitive));
			default:
				return nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::SchedulePrimitive(std::unique_ptr<GraphicalPrimitive>&& primitive) {
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_ScheduledPrimitives.emplace_back(std::move(primitive));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::SchedulePrimitivesForBlendedDrawing(DrawBlendMode blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const std::vector<GraphicalPrimitive *> &primitives) {
		if (blendMode < DrawBlendMode::NoBlend || blendMode >= DrawBlendMode::BlendModeCount) {
			g_ConsoleMan.PrintString("ERROR: Encountered invalid blending mode when attempting to draw primitives! Drawing will be skipped! See the DrawBlendMode enumeration for valid modes.");
			return;
		}
		blendAmountR = std::clamp(blendAmountR, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend));
		blendAmountG = std::clamp(blendAmountG, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend));
		blendAmountB = std::clamp(blendAmountB, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend));
		blendAmountA = std::clamp(blendAmountA, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend));

		for (GraphicalPrimitive *primitive : primitives) {
			primitive->m_BlendMode = blendMode;
			primitive->m_ColorChannelBlendAmounts = { blendAmountR, blendAmountG, blendAmountB, blendAmountA };
			SchedulePrimitive(MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(primitive));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawLinePrimitive(int player, const Vector &startPos, const Vector &endPos, unsigned char color, int thickness) {
		if (thickness > 1) {
			Vector dirVector = g_SceneMan.ShortestDistance(startPos, endPos, g_SceneMan.SceneWrapsX()).SetMagnitude(static_cast<float>(thickness - 1) / 2.0F).Perpendicularize();
			Vector pointA = startPos + dirVector;
			Vector pointB = startPos - dirVector;
			Vector pointC = endPos + dirVector;
			Vector pointD = endPos - dirVector;

			DrawTriangleFillPrimitive(player, pointA, pointB, pointC, color);
			DrawTriangleFillPrimitive(player, pointC, pointD, pointB, color);
		} else {
			SchedulePrimitive(std::make_unique<LinePrimitive>(player, startPos, endPos, color));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color) { 
		SchedulePrimitive(std::make_unique<ArcPrimitive>(-1, centerPos, startAngle, endAngle, radius, 1, color));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawArcPrimitive(const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness) { 
		SchedulePrimitive(std::make_unique<ArcPrimitive>(-1, centerPos, startAngle, endAngle, radius, thickness, color));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color) { 
		SchedulePrimitive(std::make_unique<ArcPrimitive>(player, centerPos, startAngle, endAngle, radius, 1, color));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawArcPrimitive(int player, const Vector &centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness) { 
		SchedulePrimitive(std::make_unique<ArcPrimitive>(player, centerPos, startAngle, endAngle, radius, thickness, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawSplinePrimitive(const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color) { 
		SchedulePrimitive(std::make_unique<SplinePrimitive>(-1, startPos, guideA, guideB, endPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawSplinePrimitive(int player, const Vector &startPos, const Vector &guideA, const Vector &guideB, const Vector &endPos, unsigned char color) { 
		SchedulePrimitive(std::make_unique<SplinePrimitive>(player, startPos, guideA, guideB, endPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
		SchedulePrimitive(std::make_unique<BoxPrimitive>(-1, topLeftPos, bottomRightPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
		SchedulePrimitive(std::make_unique<BoxPrimitive>(player, topLeftPos, bottomRightPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
		SchedulePrimitive(std::make_unique<BoxFillPrimitive>(-1, topLeftPos, bottomRightPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, unsigned char color) {
		SchedulePrimitive(std::make_unique<BoxFillPrimitive>(player, topLeftPos, bottomRightPos, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawRoundedBoxPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<RoundedBoxPrimitive>(-1, topLeftPos, bottomRightPos, cornerRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawRoundedBoxPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<RoundedBoxPrimitive>(player, topLeftPos, bottomRightPos, cornerRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawRoundedBoxFillPrimitive(const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<RoundedBoxFillPrimitive>(-1, topLeftPos, bottomRightPos, cornerRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawRoundedBoxFillPrimitive(int player, const Vector &topLeftPos, const Vector &bottomRightPos, int cornerRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<RoundedBoxFillPrimitive>(player, topLeftPos, bottomRightPos, cornerRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawCirclePrimitive(const Vector &centerPos, int radius, unsigned char color) {
		SchedulePrimitive(std::make_unique<CirclePrimitive>(-1, centerPos, radius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawCirclePrimitive(int player, const Vector &centerPos, int radius, unsigned char color) {
		SchedulePrimitive(std::make_unique<CirclePrimitive>(player, centerPos, radius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawCircleFillPrimitive(const Vector &centerPos, int radius, unsigned char color) {
		SchedulePrimitive(std::make_unique<CircleFillPrimitive>(-1, centerPos, radius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawCircleFillPrimitive(int player, const Vector &centerPos, int radius, unsigned char color) {
		SchedulePrimitive(std::make_unique<CircleFillPrimitive>(player, centerPos, radius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawEllipsePrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<EllipsePrimitive>(-1, centerPos, horizRadius, vertRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawEllipsePrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<EllipsePrimitive>(player, centerPos, horizRadius, vertRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawEllipseFillPrimitive(const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<EllipseFillPrimitive>(-1, centerPos, horizRadius, vertRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawEllipseFillPrimitive(int player, const Vector &centerPos, int horizRadius, int vertRadius, unsigned char color) {
		SchedulePrimitive(std::make_unique<EllipseFillPrimitive>(player, centerPos, horizRadius, vertRadius, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTrianglePrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) {
		SchedulePrimitive(std::make_unique<TrianglePrimitive>(-1, pointA, pointB, pointC, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTrianglePrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) {
		SchedulePrimitive(std::make_unique<TrianglePrimitive>(player, pointA, pointB, pointC, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTriangleFillPrimitive(const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) {
		SchedulePrimitive(std::make_unique<TriangleFillPrimitive>(-1, pointA, pointB, pointC, color));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTriangleFillPrimitive(int player, const Vector &pointA, const Vector &pointB, const Vector &pointC, unsigned char color) {
		SchedulePrimitive(std::make_unique<TriangleFillPrimitive>(player, pointA, pointB, pointC, color));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPolygonOrPolygonFillPrimitive(int player, const Vector &startPos, unsigned char color, const std::vector<Vector *> &vertices, bool filled) {
		if (vertices.size() < 2) {
			g_ConsoleMan.PrintString("ERROR: Polygon primitive should have at least 2 vertices! Drawing will be skipped!");
			return;
		}
		if (filled) {
			SchedulePrimitive(std::make_unique<PolygonFillPrimitive>(player, startPos, color, vertices));
		} else {
			SchedulePrimitive(std::make_unique<PolygonPrimitive>(player, startPos, color, vertices));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTextPrimitive(const Vector &start, const std::string &text, bool isSmall, int alignment) {
		SchedulePrimitive(std::make_unique<TextPrimitive>(-1, start, text, isSmall, alignment, 0));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTextPrimitive(const Vector &start, const std::string &text, bool isSmall, int alignment, float rotAngle) {
		SchedulePrimitive(std::make_unique<TextPrimitive>(-1, start, text, isSmall, alignment, rotAngle));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTextPrimitive(int player, const Vector &start, const std::string &text, bool isSmall, int alignment) {
		SchedulePrimitive(std::make_unique<TextPrimitive>(player, start, text, isSmall, alignment, 0));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawTextPrimitive(int player, const Vector &start, const std::string &text, bool isSmall, int alignment, float rotAngle) {
		SchedulePrimitive(std::make_unique<TextPrimitive>(player, start, text, isSmall, alignment, rotAngle));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, const MOSprite *moSprite, float rotAngle, int frame, bool hFlipped, bool vFlipped) {
		SchedulePrimitive(std::make_unique<BitmapPrimitive>(player, centerPos, moSprite, rotAngle, frame, hFlipped, vFlipped));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, const std::string &filePath, float rotAngle, bool hFlipped, bool vFlipped) {
		SchedulePrimitive(std::make_unique<BitmapPrimitive>(player, centerPos, filePath, rotAngle, hFlipped, vFlipped));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawIconPrimitive(int player, const Vector &centerPos, Entity *entity) {
		if (const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity)) { 
			SchedulePrimitive(std::make_unique<BitmapPrimitive>(player, centerPos, moSprite->GetGraphicalIcon(), 0, false, false)); 
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(int player, BITMAP *targetBitmap, const Vector &targetPos) const {
		if (m_ScheduledPrimitives.empty()) {
			return;
		}
		int lastDrawMode = DRAW_MODE_SOLID;
		DrawBlendMode lastBlendMode = DrawBlendMode::NoBlend;
		std::array<int, 4> lastBlendAmounts = { BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend };

		for (const std::unique_ptr<GraphicalPrimitive> &primitive : m_ScheduledPrimitives) {
			if (int playerToDrawFor = primitive->m_Player; playerToDrawFor == player || playerToDrawFor == -1) {
				if (DrawBlendMode blendMode = primitive->m_BlendMode; blendMode > DrawBlendMode::NoBlend) {
					if (const std::array<int, 4> &blendAmounts = primitive->m_ColorChannelBlendAmounts; blendMode != lastBlendMode || blendAmounts != lastBlendAmounts) {
						g_FrameMan.SetColorTable(blendMode, blendAmounts);
						lastBlendMode = blendMode;
						lastBlendAmounts = blendAmounts;
					}
					if (lastDrawMode != DRAW_MODE_TRANS) {
						// Drawing mode is set so blending effects apply to true primitives. For bitmap based primitives it has no effect.
						drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
						lastDrawMode = DRAW_MODE_TRANS;
					}
				} else {
					if (lastDrawMode != DRAW_MODE_SOLID) {
						drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
						lastDrawMode = DRAW_MODE_SOLID;
					}
					lastBlendMode = DrawBlendMode::NoBlend;
				}
				primitive->Draw(targetBitmap, targetPos);
			}
		}
		drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
	}
}