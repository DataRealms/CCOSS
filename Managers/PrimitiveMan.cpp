#include "PrimitiveMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

	const std::string PrimitiveMan::c_ClassName = "PrimitiveMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(short player, Vector centerPos, Entity *entity, float rotAngle, unsigned short frame, bool hFlipped, bool vFlipped) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *bitmap = moSprite->GetSpriteFrame(frame);
			if (bitmap) { m_Primitives.push_back(new BitmapPrimitive(player, centerPos, bitmap, rotAngle, hFlipped, vFlipped)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::ClearPrimitivesList() {
		for (const GraphicalPrimitive *primitive : m_Primitives) {
			delete primitive;
		}
		m_Primitives.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(short player, BITMAP *targetBitmap, const Vector &targetPos) const {
		for (GraphicalPrimitive *primitive : m_Primitives) {
			if (player == primitive->m_Player || primitive->m_Player == -1) { primitive->Draw(targetBitmap, targetPos); }
		}
	}
}