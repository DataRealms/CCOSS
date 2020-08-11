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

			if (bitmap) {
				if (!hFlipped && !vFlipped) {
					m_Primitives.push_back(new BitmapPrimitive(player, centerPos, bitmap, rotAngle));
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
					m_Primitives.push_back(new BitmapPrimitive(player, centerPos, flipBitmap, rotAngle));
				}
			}
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