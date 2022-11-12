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
		blendAmountA = std::clamp(blendAmountB, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend));

		for (GraphicalPrimitive *primitive : primitives) {
			primitive->m_BlendMode = blendMode;

			if (blendAmountR == BlendAmountLimits::MaxBlend && blendAmountG == BlendAmountLimits::MaxBlend && blendAmountB == BlendAmountLimits::MaxBlend) {
				continue;
			} else if (primitive->m_BlendMode == DrawBlendMode::BlendInvert && blendAmountB == BlendAmountLimits::MaxBlend) {
				// Invert does nothing with the RGA channel values, it will always be fully inverted on all channels. The B channel controls transparency, so don't schedule if it's set to max.
				continue;
			}
			primitive->m_ColorChannelBlendAmounts = { blendAmountR, blendAmountG, blendAmountB, blendAmountA };
			m_ScheduledPrimitives.emplace_back(MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(primitive));
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
		int lastDrawMode = NULL;
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