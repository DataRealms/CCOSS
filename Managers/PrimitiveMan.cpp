#include "PrimitiveMan.h"
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
			m_ScheduledPrimitives.emplace_back(MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(primitive));
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
			m_ScheduledPrimitives.emplace_back(std::make_unique<LinePrimitive>(player, startPos, endPos, color));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPolygonOrPolygonFillPrimitive(int player, const Vector &startPos, unsigned char color, const std::vector<Vector *> &vertices, bool filled) {
		if (vertices.size() < 2) {
			g_ConsoleMan.PrintString("ERROR: Polygon primitive should have at least 2 vertices! Drawing will be skipped!");
			return;
		}
		if (filled) {
			m_ScheduledPrimitives.emplace_back(std::make_unique<PolygonFillPrimitive>(player, startPos, color, vertices));
		} else {
			m_ScheduledPrimitives.emplace_back(std::make_unique<PolygonPrimitive>(player, startPos, color, vertices));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawIconPrimitive(int player, const Vector &centerPos, Entity *entity) {
		if (const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity)) { m_ScheduledPrimitives.emplace_back(std::make_unique<BitmapPrimitive>(player, centerPos, moSprite->GetGraphicalIcon(), 0, false, false)); }
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