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
		if (reference.m_MountedDevice) {
			m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_MountedDevice->GetUniqueID());
			SetMountedDevice(dynamic_cast<Attachable *>(reference.m_MountedDevice->Clone()));
		}
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
				m_MountedDevice->SetDeleteWhenRemovedFromParent(true);
				//Force weapons mounted on turrets to never be removed due to forces. This doesn't affect them gibbing from hitting their impulse limits though.
				m_MountedDevice->SetJointStrength(0.0F);
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
				m_HardcodedAttachableUniqueIDsAndSetters.insert({castedNewMountedDevice->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) { dynamic_cast<Turret *>(parent)->SetMountedDevice(attachable); }});
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Update() {
		if (m_MountedDevice) { m_MountedDevice->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedDeviceRotOffset); }
		Attachable::Update();
	}
}