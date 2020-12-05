#include "PrimitiveMan.h"
#include "Entity.h"
#include "MOSprite.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PrimitiveMan::DrawBitmapPrimitive(int player, const Vector &centerPos, Entity *entity, float rotAngle, int frame, bool hFlipped, bool vFlipped) {
		const MOSprite *moSprite = dynamic_cast<MOSprite *>(entity);
		if (moSprite) {
			BITMAP *bitmap = moSprite->GetSpriteFrame(frame);

			if (bitmap) {
				if (!hFlipped && !vFlipped) {
					m_Primitives.push(std::make_unique<BitmapPrimitive>(player, centerPos, bitmap, rotAngle));
				} else {
					BITMAP *flipBitmap = create_bitmap_ex(8, bitmap->w, bitmap->h);
					clear_to_color(flipBitmap, 0);

					if (hFlipped && !vFlipped) {
						draw_sprite_h_flip(flipBitmap, bitmap, 0, 0);
					} else if (!hFlipped && vFlipped) {
						draw_sprite_v_flip(flipBitmap, bitmap, 0, 0);
					} else if (hFlipped && vFlipped) {
						draw_sprite_vh_flip(flipBitmap, bitmap, 0, 0);
					}
					m_Primitives.push(std::make_unique<BitmapPrimitive>(player, centerPos, flipBitmap, rotAngle));
				}
			}
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