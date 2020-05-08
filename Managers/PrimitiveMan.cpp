#include "PrimitiveMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

	const std::string PrimitiveMan::c_ClassName = "PrimitiveMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(short player, Vector start, Entity *entity, float rotAngle, unsigned short frame) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *pBitmap = moSprite->GetSpriteFrame(frame);
			if (pBitmap) { m_Primitives.push_back(new BitmapPrimitive(player, start, pBitmap, rotAngle)); }
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

	void PrimitiveMan::DrawPrimitives(short player, BITMAP *pTargetBitmap, const Vector &targetPos) {
		for (GraphicalPrimitive *primitive : m_Primitives) {
			if (player == primitive->m_Player || primitive->m_Player == -1) { primitive->Draw(pTargetBitmap, targetPos); }
		}
	}
}