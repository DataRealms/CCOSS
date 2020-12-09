#include "PrimitiveMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *bitmap = moSprite->GetSpriteFrame(frame);
			if (bitmap) { m_Primitives.push(std::make_unique<BitmapPrimitive>(player, centerPos, bitmap, rotAngle, hFlipped, vFlipped)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::ClearPrimitivesQueue() {
		std::queue<std::unique_ptr<GraphicalPrimitive>> emptyQueue;
		std::swap(m_Primitives, emptyQueue);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(int player, BITMAP *targetBitmap, const Vector &targetPos) {
		while (!m_Primitives.empty()) {
			if (m_Primitives.front()->m_Player == player || m_Primitives.front()->m_Player == -1) { m_Primitives.front()->Draw(targetBitmap, targetPos); }
			m_Primitives.pop();
		}
	}
}