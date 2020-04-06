#include "PrimitiveMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

	const std::string PrimitiveMan::c_ClassName = "PrimitiveMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(Vector start, Entity * pEntity, float rotAngle, unsigned short frame) {
		const MOSprite *pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS) {
			BITMAP *pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap) { m_Primitives.push_back(new BitmapPrimitive(start, pBitmap, rotAngle)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(short player, Vector start, Entity * pEntity, float rotAngle, unsigned short frame) {
		const MOSprite *pMOS = dynamic_cast<MOSprite *>(pEntity);
		if (pMOS) {
			BITMAP *pBitmap = pMOS->GetSpriteFrame(frame);
			if (pBitmap) { m_Primitives.push_back(new BitmapPrimitive(player, start, pBitmap, rotAngle)); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::ClearPrimitivesList() {
		for (std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it) {
			delete (*it);
		}
		m_Primitives.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawPrimitives(short player, BITMAP *pTargetBitmap, const Vector &targetPos) {
		for (std::list<GraphicalPrimitive *>::const_iterator it = m_Primitives.begin(); it != m_Primitives.end(); ++it) {
			if (player == (*it)->m_Player || (*it)->m_Player == -1) { (*it)->Draw(pTargetBitmap, targetPos); }
		}
	}
}