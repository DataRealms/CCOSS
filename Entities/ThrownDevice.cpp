#include "ThrownDevice.h"

namespace RTE {
	CONCRETECLASSINFO(ThrownDevice, HeldDevice, 0);
	
////////////////////////////////////////////////////////////////////////////////////////////////////

	void ThrownDevice::Clear() {
		m_ActivationSound.Reset();
		m_StartThrowOffset.Reset();
		m_EndThrowOffset.Reset();
		m_MinThrowVel = 5;
		m_MaxThrowVel = 15;
		m_TriggerDelay = 0;
		m_ActivatesWhenReleased = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ThrownDevice::Create() {
		if (HeldDevice::Create() < 0) {
			return -1;
		}

		m_MOType = MovableObject::TypeThrownDevice;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ThrownDevice::Create(const ThrownDevice &reference) {
		HeldDevice::Create(reference);

		m_MOType = MovableObject::TypeThrownDevice;

		m_ActivationSound = reference.m_ActivationSound;

		m_StartThrowOffset = reference.m_StartThrowOffset;
		m_EndThrowOffset = reference.m_EndThrowOffset;
		m_MinThrowVel = reference.m_MinThrowVel;
		m_MaxThrowVel = reference.m_MaxThrowVel;
		m_TriggerDelay = reference.m_TriggerDelay;
		m_ActivatesWhenReleased = reference.m_ActivatesWhenReleased;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ThrownDevice::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "ActivationSound")
			reader >> m_ActivationSound;
		else if (propName == "StartThrowOffset")
			reader >> m_StartThrowOffset;
		else if (propName == "EndThrowOffset")
			reader >> m_EndThrowOffset;
		else if (propName == "MinThrowVel")
			reader >> m_MinThrowVel;
		else if (propName == "MaxThrowVel")
			reader >> m_MaxThrowVel;
		else if (propName == "TriggerDelay")
			reader >> m_TriggerDelay;
		else if (propName == "ActivatesWhenReleased")
			reader >> m_ActivatesWhenReleased;
		else
			return HeldDevice::ReadProperty(propName, reader);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ThrownDevice::Save(Writer &writer) const {
		HeldDevice::Save(writer);

		writer.NewProperty("ActivationSound");
		writer << m_ActivationSound;
		writer.NewProperty("StartThrowOffset");
		writer << m_StartThrowOffset;
		writer.NewProperty("EndThrowOffset");
		writer << m_EndThrowOffset;
		writer.NewProperty("MinThrowVel");
		writer << m_MinThrowVel;
		writer.NewProperty("MaxThrowVel");
		writer << m_MaxThrowVel;
		writer.NewProperty("TriggerDelay");
		writer << m_TriggerDelay;
		writer.NewProperty("ActivatesWhenReleased");
		writer << m_ActivatesWhenReleased;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ThrownDevice::Destroy(bool notInherited) {
		if (!notInherited) {
			HeldDevice::Destroy();
		}

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ThrownDevice::ResetAllTimers() {
		double elapsedTime;
		if (m_Activated) {
			elapsedTime = m_ActivationTmr.GetElapsedSimTimeMS();
		}
		HeldDevice::ResetAllTimers();
		if (m_Activated) {
			m_ActivationTmr.SetElapsedSimTimeMS(elapsedTime);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ThrownDevice::Activate() {
		if (!m_Activated) {
			m_ActivationTmr.Reset();
			m_ActivationSound.Play(g_SceneMan.TargetDistanceScalar(m_Pos));
			m_Activated = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ThrownDevice::RestDetection() {
		HeldDevice::RestDetection();

		if (m_Activated) {
			m_RestTimer.Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector ThrownDevice::GetStanceOffset() const {
		return m_StanceOffset.GetXFlipped(m_HFlipped);
	}
}