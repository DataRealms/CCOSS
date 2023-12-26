#include "MOPixel.h"

#include "Atom.h"
#include "PostProcessMan.h"
#include "ThreadMan.h"

namespace RTE {

	ConcreteClassInfo(MOPixel, MovableObject, 2000);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::Clear() {
		m_Atom = 0;
		m_Color.Reset();
		m_LethalRange = std::max(g_FrameMan.GetPlayerScreenWidth(), g_FrameMan.GetPlayerScreenHeight());
		m_MinLethalRange = 1;
		m_MaxLethalRange = 1;
		m_LethalSharpness = 1;
		m_Staininess = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::Create() {
		if (MovableObject::Create() < 0) {
			return -1;
		}
		if (!m_Atom) { m_Atom = new Atom; }

		if (m_MinLethalRange < m_MaxLethalRange) { m_LethalRange *= RandomNum(m_MinLethalRange, m_MaxLethalRange); }
		m_LethalSharpness = m_Sharpness * 0.5F;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::Create(Color color, const float mass, const Vector &position, const Vector &velocity, Atom *atom, const unsigned long lifetime) {
		m_Color = color;
		m_Atom = atom;
		m_Atom->SetOwner(this);

		if (m_MinLethalRange < m_MaxLethalRange) { m_LethalRange *= RandomNum(m_MinLethalRange, m_MaxLethalRange); }
		m_LethalSharpness = m_Sharpness * 0.5F;

		return MovableObject::Create(mass, position, velocity, lifetime);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::Create(const MOPixel &reference) {
		MovableObject::Create(reference);

		m_Atom = new Atom(*(reference.m_Atom));
		m_Atom->SetOwner(this);
		m_Color = reference.m_Color;
		m_LethalRange = reference.m_LethalRange;
		m_MinLethalRange = reference.m_MinLethalRange;
		m_MaxLethalRange = reference.m_MaxLethalRange;
		m_LethalSharpness = reference.m_LethalSharpness;
		m_Staininess = reference.m_Staininess;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return MovableObject::ReadProperty(propName, reader));
		
		MatchProperty("Atom", {
			if (!m_Atom) { m_Atom = new Atom; }
			reader >> m_Atom;
			m_Atom->SetOwner(this);
		});
		MatchProperty("Color", { reader >> m_Color; });
		MatchProperty("MinLethalRange", { reader >> m_MinLethalRange; });
		MatchProperty("MaxLethalRange", { reader >> m_MaxLethalRange; });
		MatchProperty("Staininess", { reader >> m_Staininess; });

		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::Save(Writer &writer) const {
		MovableObject::Save(writer);

		writer.NewProperty("Atom");
		writer << m_Atom;
		writer.NewProperty("Color");
		writer << m_Color;
		writer.NewProperty("MinLethalRange");
		writer << m_MinLethalRange;
		writer.NewProperty("MaxLethalRange");
		writer << m_MaxLethalRange;
		writer.NewProperty("Staininess");
		writer << m_Staininess;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::Destroy(bool notInherited) {
		delete m_Atom;

		if (!notInherited) { MovableObject::Destroy(); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::GetDrawPriority() const { return m_Atom->GetMaterial()->GetPriority(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Material * MOPixel::GetMaterial() const { return m_Atom->GetMaterial(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::SetAtom(Atom *newAtom) {
		delete m_Atom;
		m_Atom = newAtom;
		m_Atom->SetOwner(this);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::SetLethalRange(float range) {
		m_LethalRange = range;
		if (m_MinLethalRange < m_MaxLethalRange) { m_LethalRange *= RandomNum(m_MinLethalRange, m_MaxLethalRange); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MOPixel::GetTrailLength() const {
		return m_Atom->GetTrailLength();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::SetTrailLength(int trailLength) {
		m_Atom->SetTrailLength(trailLength);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MOPixel::HitTestAtPixel(int pixelX, int pixelY) const {
		if (!GetsHitByMOs() || GetRootParent()->GetTraveling()) {
			return false;
		}

		return m_Pos.GetFloorIntX() == pixelX && m_Pos.GetFloorIntY() == pixelY;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::Travel() {
		MovableObject::Travel();

		if (m_PinStrength) {
			return;
		}

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
		if (!IsTooFast()) { hitCount = m_Atom->Travel(g_TimerMan.GetDeltaTimeSecs(), true, g_SceneMan.SceneIsLocked()); }

		m_Atom->ClearMOIDIgnoreList();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MOPixel::CollideAtPoint(HitData &hd) {
		RTEAssert(hd.HitPoint.GetFloored() == m_Pos.GetFloored(), "Collision mismatch in MOPixel::CollideAtPoint!");
		RTEAssert(hd.Body[HITOR], "Valid MO not passed into MOPixel::CollideAtPoint!");

		hd.TotalMass[HITEE] = m_Mass;

		// See if we were already hit by this MO earlier during this frame update.
		if (m_AlreadyHitBy.find(hd.Body[HITOR]->GetID()) != m_AlreadyHitBy.end()) {
			// TODO: Figure out why we're removing the already hit flag if we got hit
			m_AlreadyHitBy.erase(hd.Body[HITOR]->GetID());
		} else {
			m_AlreadyHitBy.insert(hd.Body[HITOR]->GetID());
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::RestDetection() {
		MovableObject::RestDetection();

		// If we seem to be about to settle, make sure we're not still flying in the air
		if ((m_ToSettle || IsAtRest()) && g_SceneMan.OverAltitude(m_Pos, 2, 0)) {
			m_VelOscillations = 0;
			m_RestTimer.Reset();
			m_ToSettle = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::Update() {
		MovableObject::Update();

		// TODO: Rework this once we figure out how we want to handle it
		if (m_HitsMOs && m_Sharpness > 0) {
			if (m_DistanceTravelled > m_LethalRange) {
				if (m_Sharpness < m_LethalSharpness) {
					m_Sharpness = std::max(m_Sharpness * (1.0F - (20.0F * g_TimerMan.GetDeltaTimeSecs())) - 0.1F, 0.0F);
					if (m_LethalRange > 0) {
						float randomNum = RandomNum(0.0F, 0.5F);
						m_Atom->SetTrailLength(static_cast<int>(static_cast<float>(m_Atom->GetTrailLength()) * (1.0F - randomNum)));
						m_Lifetime -= static_cast<unsigned long>(static_cast<float>(m_Lifetime - static_cast<int>(m_AgeTimer.GetElapsedSimTimeMS())) * randomNum);
						m_HitsMOs = RandomNum() < 0.5F;
					}
				} else {
					m_Sharpness *= 1.0F - (10.0F * g_TimerMan.GetDeltaTimeSecs());
				}
			}
		}

		if (m_pScreenEffect) {
			SetPostScreenEffectToDraw();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::Draw(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		int drawColor = -1;

		if (mode == g_DrawMOID) {
			g_SceneMan.RegisterMOIDDrawing(m_MOID, m_Pos, 1);
			return;
		}

		switch (mode) {
			case g_DrawMaterial:
				drawColor = m_Atom->GetMaterial()->GetSettleMaterial();
				break;
			default:
				drawColor = m_Color.GetIndex();
				break;
		}

		if (drawColor != g_MaskColor) {
			Vector prevSpritePos = m_PrevPos - targetPos;
			Vector spritePos = m_Pos - targetPos;

			auto renderFunc = [=](float interpolationAmount) {
				BITMAP* pTargetBitmap = targetBitmap;
				Vector renderPos = Lerp(0.0F, 1.0F, prevSpritePos, spritePos, interpolationAmount);
				if (targetBitmap == nullptr) {
					pTargetBitmap = g_ThreadMan.GetRenderTarget();
					renderPos -= g_ThreadMan.GetRenderOffset();
				}

				putpixel(pTargetBitmap, renderPos.GetFloorIntX(), renderPos.GetFloorIntY(), drawColor); 
			};

			if (targetBitmap == nullptr) {
				g_ThreadMan.GetSimRenderQueue().push_back(renderFunc);
			} else {
				renderFunc(1.0F);
			}
		}
		
		if (mode == g_DrawColor && m_pScreenEffect && !onlyPhysical) {
			SetPostScreenEffectToDraw();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MOPixel::SetPostScreenEffectToDraw() const {
		if (m_AgeTimer.GetElapsedSimTimeMS() >= m_EffectStartTime && (m_EffectStopTime == 0 || !m_AgeTimer.IsPastSimMS(m_EffectStopTime))) {
			if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY())) {
				g_PostProcessMan.RegisterPostEffect(m_Pos, m_pScreenEffect, m_ScreenEffectHash, Lerp(m_EffectStartTime, m_EffectStopTime, m_EffectStartStrength, m_EffectStopStrength, m_AgeTimer.GetElapsedSimTimeMS()), m_EffectRotAngle);
			}
		}
	}
}
