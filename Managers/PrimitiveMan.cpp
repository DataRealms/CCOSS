#include "PrimitiveMan.h"
#include "Entity.h"
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

	void PrimitiveMan::SchedulePrimitivesForTransparentDrawing(int transValue, const std::vector<GraphicalPrimitive *> &primitives) {
		if (transValue < TransparencyPreset::Trans100) {
			for (GraphicalPrimitive *primitive : primitives) {
				primitive->m_Transparency = std::clamp(transValue, static_cast<int>(TransparencyPreset::Trans0), static_cast<int>(TransparencyPreset::Trans100));
				m_ScheduledPrimitives.emplace_back(MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(primitive));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *bitmap = moSprite->GetSpriteFrame(frame);
			if (bitmap) { m_ScheduledPrimitives.emplace_back(std::make_unique<BitmapPrimitive>(player, centerPos, bitmap, rotAngle, hFlipped, vFlipped)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawIconPrimitive(int player, const Vector &centerPos, Entity *entity) {
		if (const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity)) { m_ScheduledPrimitives.emplace_back(std::make_unique<BitmapPrimitive>(player, centerPos, moSprite->GetGraphicalIcon(), 0, false, false)); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(int player, BITMAP *targetBitmap, const Vector &targetPos) const {
		for (const std::unique_ptr<GraphicalPrimitive> &primitive : m_ScheduledPrimitives) {
			if (int playerToDrawFor = primitive->m_Player; playerToDrawFor == player || playerToDrawFor == -1) {
				if (int transparencySetting = primitive->m_Transparency; transparencySetting > TransparencyPreset::Trans0) {
					g_FrameMan.SetTransTable(transparencySetting);
					drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
				} else {
					drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
				}
				primitive->Draw(targetBitmap, targetPos);
			}
		}
		drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
	}
}