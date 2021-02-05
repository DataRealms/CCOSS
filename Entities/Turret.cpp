#include "Turret.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(Turret, Attachable, 20)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Clear() {
		m_MountedDevice = nullptr;
		m_MountedDeviceRotOffset = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Turret::Create(const Turret &reference) {
		if (reference.m_MountedDevice) {
			m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_MountedDevice->GetUniqueID());
			SetMountedDevice(dynamic_cast<HeldDevice *>(reference.m_MountedDevice->Clone()));
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
				m_MountedDevice->SetUnPickupable(true);
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

	void Turret::SetMountedDevice(HeldDevice *newMountedDevice) {
		if (newMountedDevice == nullptr) {
			if (m_MountedDevice && m_MountedDevice->IsAttached()) { RemoveAttachable(m_MountedDevice); }
			m_MountedDevice = nullptr;
		} else {
			if (m_MountedDevice && m_MountedDevice->IsAttached()) { RemoveAttachable(m_MountedDevice); }
			m_MountedDevice = newMountedDevice;
			AddAttachable(newMountedDevice);

			m_HardcodedAttachableUniqueIDsAndSetters.insert({newMountedDevice->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
				HeldDevice *castedAttachable = dynamic_cast<HeldDevice *>(attachable);
				RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetMountedDevice");
				dynamic_cast<Turret *>(parent)->SetMountedDevice(castedAttachable);
			}});
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Update() {
		if (m_MountedDevice) { m_MountedDevice->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedDeviceRotOffset); }
		Attachable::Update();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Turret::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
		Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
		//TODO replace this with a relative draw order property or something that lets you organize attachable drawing so it doesn't need special hardcoding crap. Use this for ahuman limbs and arm held mo if possible.
		if (m_MountedDevice && m_MountedDevice->IsDrawnAfterParent()) { m_MountedDevice->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
	}
}