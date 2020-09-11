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
		if (reference.m_MountedDevice && reference.m_MountedDevice->IsDevice()) { CloneHardcodedAttachable(dynamic_cast<Attachable *>(reference.m_MountedDevice), this, static_cast<std::function<void(Turret &, Attachable *)>>((void (Turret:: *)(Attachable * newMountedMO)) & Turret::SetMountedDevice)); }
		Attachable::Create(reference);

		m_MountedDeviceRotOffset = reference.m_MountedDeviceRotOffset;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Turret::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "MountedDevice") {
			const Entity *entity = g_PresetMan.GetEntityPreset(reader);
			if (entity) {
				m_MountedDevice = dynamic_cast<HeldDevice *>(entity->Clone());
				m_MountedDevice->SetInheritsRotAngle(false);
			}
			entity = 0;
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

	void Turret::SetMountedDevice(HeldDevice *newMountedDevice) {
		DropMountedDevice();

		if (newMountedDevice) {
			if (newMountedDevice->IsAttached()) { dynamic_cast<MOSRotating *>(newMountedDevice->GetParent())->RemoveAttachable(newMountedDevice); }
			g_MovableMan.RemoveMO(newMountedDevice);
			AddAttachable(newMountedDevice);
		}
		m_MountedDevice = newMountedDevice;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HeldDevice *Turret::ReleaseMountedDevice() {
		HeldDevice *previouslyMountedDevice = m_MountedDevice;
		if (m_MountedDevice) { RemoveAttachable(m_MountedDevice); }
		m_MountedDevice = 0;
		return previouslyMountedDevice;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HeldDevice *Turret::DropMountedDevice() {
		HeldDevice *previouslyMountedDevice = ReleaseMountedDevice();

		if (previouslyMountedDevice) {
			g_MovableMan.AddItem(previouslyMountedDevice);
		}

		return previouslyMountedDevice;
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
		//TODO consider expanding the stuff used in clone create to allow for clearing hardcoded attachables members (might be as simple as updating unique id entries and calling things accordingly)
		if (std::find(m_Attachables.begin(), m_Attachables.end(), m_MountedDevice) == m_Attachables.end()) { m_MountedDevice = nullptr; }
	}
}