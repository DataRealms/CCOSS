#include "TDExplosive.h"

namespace RTE {
	ConcreteClassInfo(TDExplosive, ThrownDevice, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TDExplosive::Clear() {
		m_IsAnimatedManually = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TDExplosive::Create(const TDExplosive &reference) {
		ThrownDevice::Create(reference);

		m_IsAnimatedManually = reference.m_IsAnimatedManually;

		// All Explosives should hit against other objects etc, like grenades flying and hitting actors etc EXCEPT when they are laying on the ground etc
		m_IgnoresAGHitsWhenSlowerThan = 1.0;

		if (IsInGroup("Bombs - Payloads")) {
			m_HUDVisible = false;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TDExplosive::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "DetonationSound") //TODO Consider removing this as GibSound already exists and could be used in its place
			reader >> m_GibSound;
		else if (propName == "IsAnimatedManually")
			reader >> m_IsAnimatedManually;
		else
			return ThrownDevice::ReadProperty(propName, reader);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TDExplosive::Save(Writer &writer) const {
		ThrownDevice::Save(writer);

		writer.NewProperty("IsAnimatedManually");
		writer << m_IsAnimatedManually;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TDExplosive::Destroy(bool notInherited) {
		if (!notInherited) {
			ThrownDevice::Destroy();
		}

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TDExplosive::Update() {
		ThrownDevice::Update();

		if (m_Activated) {
			// If not animated manually, play 'fuse lit' animation
			if (!m_IsAnimatedManually) {
				m_SpriteAnimMode = ALWAYSLOOP;
			} else {
				// If animated manually, stay on starting frame
				m_Frame = 0;
				m_SpriteAnimMode = NOANIM;
			}
			m_RestTimer.Reset();
			m_ToSettle = false;
		}

		// Blow up if the activation timer has reached the trigger delay limit
		if (m_Activated && m_ActivationTmr.GetElapsedSimTimeMS() >= m_TriggerDelay) {
			m_Activated = false;
			GibThis();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TDExplosive::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
		if (!m_HUDVisible) {
			return;
		}

		// Only draw the pickup HUD if not activated
		if (!m_Activated) {
			ThrownDevice::DrawHUD(pTargetBitmap, targetPos, whichScreen);
		}
	}
}