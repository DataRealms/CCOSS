#include "PrimitiveMan.h"

#include "SceneMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

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

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *bitmap = moSprite->GetSpriteFrame(frame);
			if (bitmap) { m_ScheduledPrimitives.push_back(std::make_unique<BitmapPrimitive>(player, centerPos, bitmap, rotAngle, hFlipped, vFlipped)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawIconPrimitive(int player, const Vector &centerPos, Entity *entity) {
		if (const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity)) { m_ScheduledPrimitives.push_back(std::make_unique<BitmapPrimitive>(player, centerPos, moSprite->GetGraphicalIcon(), 0, false, false)); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(int player, BITMAP *targetBitmap, const Vector &targetPos) const {
		for (const std::unique_ptr<GraphicalPrimitive> &primitive : m_ScheduledPrimitives) {
			if (primitive->m_Player == player || primitive->m_Player == -1) { primitive->Draw(targetBitmap, targetPos); }
		}
	}
}