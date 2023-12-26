#include "MOSParticle.h"

#include "Atom.h"
#include "PostProcessMan.h"
#include "ThreadMan.h"

namespace RTE {

	ConcreteClassInfo(MOSParticle, MovableObject, 1000);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Clear() {
		m_Atom = nullptr;
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
		StartPropertyList(return MOSprite::ReadProperty(propName, reader));
		
		MatchProperty("Atom", {
			if (!m_Atom) { m_Atom = new Atom; }
			reader >> *m_Atom;
			m_Atom->SetOwner(this);
		}); 
		
		EndPropertyList;
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
		m_Atom->SetOwner(this);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::RestDetection() {
		MOSprite::RestDetection();

		// If we seem to be about to settle, make sure we're not still flying in the air
		if ((m_ToSettle || IsAtRest()) && g_SceneMan.OverAltitude(m_Pos, (m_aSprite[m_Frame]->h / 2) + 3, 2)) {
			m_VelOscillations = 0;
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
		int hitCount = 0;
		if (!IsTooFast()) { 
			m_Atom->Travel(g_TimerMan.GetDeltaTimeSecs(), true, g_SceneMan.SceneIsLocked()); 
		}

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

	void MOSParticle::Update() {
		MOSprite::Update();

		if (m_pScreenEffect) { SetPostScreenEffectToDraw(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::Draw(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		RTEAssert(!m_aSprite.empty(), "No sprite bitmaps loaded to draw " + GetPresetName());
		RTEAssert(m_Frame >= 0 && m_Frame < m_FrameCount, "Frame is out of bounds for " + GetPresetName());
		
		BITMAP *currentFrame = m_aSprite[m_Frame];
		if (!currentFrame) {
			RTEAbort("Sprite frame pointer is null when drawing MOSprite!");
		}

		Vector prevSpritePos(m_PrevPos + m_SpriteOffset - targetPos);
		Vector spritePos(m_Pos + m_SpriteOffset - targetPos);
	
		if (mode == g_DrawMOID) {
			g_SceneMan.RegisterMOIDDrawing(m_MOID, spritePos.GetX(), spritePos.GetY(), spritePos.GetX() + currentFrame->w, spritePos.GetY() + currentFrame->h);
			return;
		}

        bool wrapDoubleDraw = m_WrapDoubleDraw;
		char settleMaterial = mode != g_DrawMaterial   ? 0                         :
		                      m_SettleMaterialDisabled ? GetMaterial()->GetIndex() : 
							                             GetMaterial()->GetSettleMaterial();

		auto renderFunc = [=](float interpolationAmount) {
			BITMAP* pTargetBitmap = targetBitmap;
			Vector renderPos = g_SceneMan.Lerp(0.0F, 1.0F, prevSpritePos, spritePos, interpolationAmount);
			if (targetBitmap == nullptr) {
				pTargetBitmap = g_ThreadMan.GetRenderTarget();
				renderPos -= g_ThreadMan.GetRenderOffset();
			}

        	// Take care of wrapping situations
			std::array<Vector, 4> drawPositions = { renderPos };
			int drawPasses = 1;
			if (g_SceneMan.SceneWrapsX()) {
				if (renderPos.IsZero() && wrapDoubleDraw) {
					if (spritePos.GetFloorIntX() < currentFrame->w) {
						drawPositions[drawPasses] = spritePos;
						drawPositions[drawPasses].m_X += static_cast<float>(pTargetBitmap->w);
						drawPasses++;
					} else if (spritePos.GetFloorIntX() > pTargetBitmap->w - currentFrame->w) {
						drawPositions[drawPasses] = spritePos;
						drawPositions[drawPasses].m_X -= static_cast<float>(pTargetBitmap->w);
						drawPasses++;
					}
				} else if (wrapDoubleDraw) {
					if (renderPos.m_X < 0) {
						drawPositions[drawPasses] = drawPositions[0];
						drawPositions[drawPasses].m_X += static_cast<float>(g_SceneMan.GetSceneWidth());
						drawPasses++;
					}
					if (renderPos.GetFloorIntX() + pTargetBitmap->w > g_SceneMan.GetSceneWidth()) {
						drawPositions[drawPasses] = drawPositions[0];
						drawPositions[drawPasses].m_X -= static_cast<float>(g_SceneMan.GetSceneWidth());
						drawPasses++;
					}
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				if (renderPos.IsZero() && wrapDoubleDraw) {
					if (spritePos.GetFloorIntY() < currentFrame->h) {
						drawPositions[drawPasses] = spritePos;
						drawPositions[drawPasses].m_Y += static_cast<float>(pTargetBitmap->h);
						drawPasses++;
					} else if (spritePos.GetFloorIntY() > pTargetBitmap->h - currentFrame->h) {
						drawPositions[drawPasses] = spritePos;
						drawPositions[drawPasses].m_Y -= static_cast<float>(pTargetBitmap->h);
						drawPasses++;
					}
				} else if (wrapDoubleDraw) {
					if (renderPos.m_Y < 0) {
						drawPositions[drawPasses] = drawPositions[0];
						drawPositions[drawPasses].m_Y += static_cast<float>(g_SceneMan.GetSceneHeight());
						drawPasses++;
					}
					if (renderPos.GetFloorIntY() + pTargetBitmap->h > g_SceneMan.GetSceneHeight()) {
						drawPositions[drawPasses] = drawPositions[0];
						drawPositions[drawPasses].m_Y -= static_cast<float>(g_SceneMan.GetSceneHeight());
						drawPasses++;
					}
				}
			}

			for (int i = 0; i < drawPasses; ++i) {
				int spriteX = drawPositions.at(i).GetFloorIntX();
				int spriteY = drawPositions.at(i).GetFloorIntY();
				switch (mode) {
					case g_DrawMaterial:
						draw_character_ex(pTargetBitmap, currentFrame, spriteX, spriteY, settleMaterial, -1);
						break;
					case g_DrawWhite:
						draw_character_ex(pTargetBitmap, currentFrame, spriteX, spriteY, g_WhiteColor, -1);
						break;
					case g_DrawTrans:
						draw_trans_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
						break;
					case g_DrawAlpha:
						set_alpha_blender();
						draw_trans_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
						break;
					default:
						draw_sprite(pTargetBitmap, currentFrame, spriteX, spriteY);
						break;
				}
			}
		};

		if (targetBitmap == nullptr) {
			g_ThreadMan.GetSimRenderQueue().push_back(renderFunc);
		} else {
			renderFunc(1.0F);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOSParticle::SetPostScreenEffectToDraw() const {
		if (m_AgeTimer.GetElapsedSimTimeMS() >= m_EffectStartTime && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime))) {
			if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())) {
				g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, Lerp(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);
			}
		}
	}
}
