#include "MOSParticle.h"
#include "Atom.h"
#include "PostProcessMan.h"

namespace RTE {

	ConcreteClassInfo(MOSParticle, MovableObject, 1000);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Clear() {
		m_Atom = 0;
		m_SpriteAnimMode = OVERLIFETIME;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOSParticle::Create() {
		if (MOSprite::Create() < 0) {
			return -1;
		}
		if (!m_Atom) { m_Atom = new Atom(); }
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOSParticle::Create(const MOSParticle &reference) {
		MOSprite::Create(reference);

		m_Atom = new Atom(*(reference.m_Atom));
		m_Atom->SetOwner(this);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOSParticle::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Atom") {
			if (!m_Atom) { m_Atom = new Atom; }
			reader >> *m_Atom;
			m_Atom->SetOwner(this);
		} else {
			return MOSprite::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOSParticle::Save(Writer &writer) const {
		MOSprite::Save(writer);

		// TODO: Make proper save system that knows not to save redundant data!
		/*
		writer.NewProperty("Atom");
		writer << m_Atom;
		*/

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Destroy(bool notInherited) {
		delete m_Atom;

		if (!notInherited) { MOSprite::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOSParticle::GetDrawPriority() const { return m_Atom->GetMaterial()->GetPriority(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Material * MOSParticle::GetMaterial() const { return m_Atom->GetMaterial(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::SetAtom(Atom *newAtom) {
		delete m_Atom;
		m_Atom = newAtom;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::RestDetection() {
		MOSprite::RestDetection();

		// If we seem to be about to settle, make sure we're not still flying in the air
		if ((m_ToSettle || IsAtRest()) && g_SceneMan.OverAltitude(m_Pos, (m_aSprite[m_Frame]->h / 2) + 3, 2)) {
			m_RestTimer.Reset();
			m_ToSettle = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Travel() {
		MOSprite::Travel();

		if (m_PinStrength) {
			return;
		}

		float deltaTime = g_TimerMan.GetDeltaTimeSecs();
		float velMag = m_Vel.GetMagnitude();

		// Set the atom to ignore a certain MO, if set and applicable.
		if (m_HitsMOs && m_pMOToNotHit && g_MovableMan.ValidMO(m_pMOToNotHit) && !m_MOIgnoreTimer.IsPastSimTimeLimit()) {
			std::vector<MOID> MOIDsNotToHit;
			m_pMOToNotHit->GetMOIDs(MOIDsNotToHit);
			for (const MOID &MOIDNotToHit : MOIDsNotToHit) {
				m_Atom->AddMOIDToIgnore(MOIDNotToHit);
			}
		}
		// Do static particle bounce calculations.
		int hitCount = m_Atom->Travel(g_TimerMan.GetDeltaTimeSecs(), true, g_SceneMan.SceneIsLocked());

		m_Atom->ClearMOIDIgnoreList();

		if (m_SpriteAnimMode == ONCOLLIDE) {
			// Change angular velocity after collision.
			if (hitCount >= 1) {
				m_AngularVel *= 0.5F * velMag * RandomNormalNum();
				m_AngularVel = -m_AngularVel;
			}

			// TODO: Rework this so it's less incomprehensible black magic math and not driven by AngularVel.
			double newFrame = m_Rotation.GetRadAngle();

			newFrame -= std::floor(m_Rotation.GetRadAngle() / (2.0F * c_PI)) * (2.0F * c_PI);
			newFrame /= (2.0F * c_PI);
			newFrame *= m_FrameCount;
			m_Frame = std::floor(newFrame);
			m_Rotation += m_AngularVel * deltaTime;

			if (m_Frame >= m_FrameCount) { m_Frame = m_FrameCount - 1; }
		}			
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Draw(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		Vector spritePos(m_Pos + m_SpriteOffset - targetPos);

		int spriteX = 0;
		int spriteY = 0;

		switch (mode) {
			case g_DrawMaterial:
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), m_SettleMaterialDisabled ? GetMaterial()->GetIndex() : GetMaterial()->GetSettleMaterial(), -1);
				break;
			case g_DrawAir:
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_MaterialAir, -1);
				break;
			case g_DrawMask:
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_MaskColor, -1);
				break;
			case g_DrawWhite:
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_WhiteColor, -1);
				break;
			case g_DrawMOID:
				spriteX = spritePos.GetFloorIntX();
				spriteY = spritePos.GetFloorIntY();
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spriteX, spriteY, m_MOID, -1);
				g_SceneMan.RegisterMOIDDrawing(spriteX, spriteY, spriteX + m_aSprite[m_Frame]->w, spriteY + m_aSprite[m_Frame]->h);
				break;
			case g_DrawNoMOID:
				draw_character_ex(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY(), g_NoMOID, -1);
				break;
			case g_DrawTrans:
				draw_trans_sprite(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());
				break;
			case g_DrawAlpha:
				set_alpha_blender();
				draw_trans_sprite(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());
				break;
			default:
				draw_sprite(targetBitmap, m_aSprite[m_Frame], spritePos.GetFloorIntX(), spritePos.GetFloorIntY());
				break;
		}
		
		if (m_pScreenEffect && mode == g_DrawColor && !onlyPhysical) { SetPostScreenEffectToDraw(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::SetPostScreenEffectToDraw() const {
		if (m_AgeTimer.GetElapsedSimTimeMS() >= m_EffectStartTime && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime))) {
			if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())) {
				g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, LERP(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);
			}
		}
	}
}