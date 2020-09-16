#include "Turret.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(Turret, Attachable, 20)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Clear() {
		m_MountedDevice = 0;
		m_MountedDeviceRotOffset = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Turret::Create(const Turret &reference) {
		if (reference.m_MountedDevice) { m_HardcodedAttachableUniqueIDsAndSetters.insert({reference.m_MountedDevice->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) { dynamic_cast<Turret *>(parent)->SetMountedDevice(attachable); }}); }
		Attachable::Create(reference);

		m_MountedDeviceRotOffset = reference.m_MountedDeviceRotOffset;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Turret::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "MountedDevice") {
			RemoveAttachable(m_MountedDevice);
			const Entity *mountedDeviceEntity = g_PresetMan.GetEntityPreset(reader);
			if (mountedDeviceEntity) {
				m_MountedDevice = dynamic_cast<HeldDevice *>(mountedDeviceEntity->Clone());
				AddAttachable(m_MountedDevice);
				m_MountedDevice->SetInheritsRotAngle(false);
			}
		} else {
			return Attachable::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Turret::Save(Writer &writer) const {
		Attachable::Save(writer);

		writer.NewProperty("MountedDevice");
		writer << m_MountedDevice;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::SetMountedDevice(Attachable *newMountedDevice) {
		if (newMountedDevice == nullptr) {
			if (m_MountedDevice && m_MountedDevice->IsAttachedTo(this)) { RemoveAttachable(m_MountedDevice); }
			m_MountedDevice = nullptr;
		} else {
			HeldDevice *castedNewMountedDevice = dynamic_cast<HeldDevice *>(newMountedDevice);
			if (castedNewMountedDevice) {
				RemoveAttachable(m_MountedDevice);
				m_MountedDevice = castedNewMountedDevice;
				AddAttachable(castedNewMountedDevice);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Update() {
		if (m_MountedDevice) {
			m_MountedDevice->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedDeviceRotOffset);
			if (m_Parent) {
				//if (m_MountedDevice->IsRecoiled()) {
				//	m_Parent->AddImpulseForce(m_MountedDevice->GetRecoilForce());
				//} else {
					m_Recoiled = false;
				//}
			}
		}

		Attachable::Update();
	}
}