#include "Leg.h"
#include "PresetMan.h"

namespace RTE {

	ConcreteClassInfo(Leg, Attachable, 50)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//TODO Organize these
	void Leg::Clear() {
		m_Foot = 0;
		m_ContractedOffset.Reset();
		m_ExtendedOffset.Reset();
		m_MinExtension = 0;
		m_MaxExtension = 0;
		m_CurrentNormalizedExtension = 0;
		m_TargetOffset.Reset();
		m_IdleOffset.Reset();
		m_CurrentAnkleOffset.Reset();
		m_WillIdle = false;
		m_MoveSpeed = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Leg::Create() {
		if (MOSRotating::Create() < 0) {
			return -1;
		}

		if (m_ContractedOffset.GetMagnitude() > m_ExtendedOffset.GetMagnitude()) {
			std::swap(m_ContractedOffset, m_ExtendedOffset); //TODO dunno if this'll work, swapping if needed so contracted always < extended
		}

		m_MinExtension = m_ContractedOffset.GetMagnitude();
		m_MaxExtension = m_ExtendedOffset.GetMagnitude();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Leg::Create(const Leg &reference) {
		if (reference.m_Foot) { CloneHardcodedAttachable(reference.m_Foot, this, static_cast<std::function<void(Leg &, Attachable *)>>(&Leg::SetFoot)); }
		Attachable::Create(reference);

		m_ContractedOffset = reference.m_ContractedOffset;
		m_ExtendedOffset = reference.m_ExtendedOffset;
		m_MinExtension = reference.m_MinExtension;
		m_MaxExtension = reference.m_MaxExtension;
		m_CurrentNormalizedExtension = reference.m_CurrentNormalizedExtension;
		m_TargetOffset = reference.m_TargetOffset;
		m_IdleOffset = reference.m_IdleOffset;
		m_CurrentAnkleOffset = reference.m_CurrentAnkleOffset;
		m_WillIdle = reference.m_WillIdle;
		m_MoveSpeed = reference.m_MoveSpeed;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Leg::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "Foot") {
			const Entity *pObj = g_PresetMan.GetEntityPreset(reader);
			if (pObj) {
				m_Foot = dynamic_cast<Attachable *>(pObj->Clone());
			}
		} else if (propName == "ContractedOffset") {
			reader >> m_ContractedOffset;
			m_MinExtension = m_ContractedOffset.GetMagnitude();
		} else if (propName == "ExtendedOffset") {
			reader >> m_ExtendedOffset;
			m_MaxExtension = m_ExtendedOffset.GetMagnitude();
		} else if (propName == "MaxLength") {
			//TODO Get rid of this, it's used only for rocket leg I think
			float maxLength;
			reader >> maxLength;

			m_MinExtension = maxLength / 2;
			m_ContractedOffset.SetXY(m_MinExtension, 0);
			m_MaxExtension = maxLength;
			m_ExtendedOffset.SetXY(m_MaxExtension, 0);
		} else if (propName == "IdleOffset") {
			reader >> m_IdleOffset;
		} else if (propName == "WillIdle") {
			reader >> m_WillIdle;
		} else if (propName == "MoveSpeed") {
			reader >> m_MoveSpeed;
		} else {
			return Attachable::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Leg::Save(Writer &writer) const {
		Attachable::Save(writer);

		writer.NewProperty("Foot");
		writer << m_Foot;
		writer.NewProperty("ContractedOffset");
		writer << m_ContractedOffset;
		writer.NewProperty("ExtendedOffset");
		writer << m_ExtendedOffset;
		writer.NewProperty("IdleOffset");
		writer << m_IdleOffset;
		writer.NewProperty("WillIdle");
		writer << m_WillIdle;
		writer.NewProperty("MoveSpeed");
		writer << m_MoveSpeed;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::SetFoot(Attachable *newFoot) {
		if (newFoot) {
			RemoveAttachable(m_Foot);
			m_Foot = newFoot;
			AddAttachable(newFoot);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::Update() {
		Attachable::Update();

		UpdateCurrentAnkleOffset();

		UpdateLegRotation();

		UpdateFrameAndNormalizedExtension();

		UpdateFootFrameJointPositionAndRotation();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::UpdateCurrentAnkleOffset() {
		if (IsAttached()) {
			Vector targetPosition = g_SceneMan.ShortestDistance(m_JointPos, m_TargetOffset, g_SceneMan.SceneWrapsX());
			if (m_WillIdle && targetPosition.m_Y < -3) {
				targetPosition = m_IdleOffset.GetXFlipped(m_HFlipped);
			}

			//TODO DELETE IF NEVER TRIGGERED ELSE INTEGRATE
			if (targetPosition.GetMagnitude() > m_MaxExtension) {
				int a = 0;
			}

			Vector distanceToTargetPosition(targetPosition - m_CurrentAnkleOffset);
			distanceToTargetPosition.SetMagnitude(std::min(distanceToTargetPosition.GetMagnitude(), m_MaxExtension));
			m_CurrentAnkleOffset += distanceToTargetPosition * m_MoveSpeed;

			//TODO DELETE IF NEVER TRIGGERED ELSE INTEGRATE BETTER
			if (m_CurrentAnkleOffset.GetMagnitude() > m_MaxExtension) {
				m_CurrentAnkleOffset.SetMagnitude(m_MaxExtension);
			} else if (m_CurrentAnkleOffset.GetMagnitude() < m_MinExtension) {
				m_CurrentAnkleOffset.SetMagnitude(m_MinExtension + 0.1F);
			}
		} else {
			m_CurrentAnkleOffset.SetXY(m_MaxExtension * 0.60F, 0);
			m_CurrentAnkleOffset.RadRotate((m_HFlipped ? c_PI : 0) + m_Rotation.GetRadAngle());
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::UpdateLegRotation() {
		if (IsAttached()) {
			m_Rotation = m_CurrentAnkleOffset.GetAbsRadAngle() + (m_HFlipped ? c_PI : 0);

			// Get normalized scalar for how much of the difference in angle between the contracted and extrended offsets should be applied. EaseOut is used to get the sine effect needed
			float extraRotationRatio = (EaseOut(m_MinExtension, m_MaxExtension, m_CurrentNormalizedExtension) - m_MinExtension) / (m_MaxExtension - m_MinExtension);

			// The contracted offset's inverse angle is the base for the rotation correction
			float extraRotation = -(m_ContractedOffset.GetAbsRadAngle());

			// Get the actual amount of extra rotation correction needed from the ratio, somewhere on the arc between contracted and extended angles.
			// This is negative because it's a correction, the bitmap needs to rotate back to align the ankle with where it's supposed to be in the sprite
			extraRotation -= (m_ExtendedOffset.GetAbsRadAngle() - m_ContractedOffset.GetAbsRadAngle()) * extraRotationRatio;

			m_Rotation.SetRadAngle(m_Rotation.GetRadAngle() + extraRotation * static_cast<float>(GetFlipFactor()));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::UpdateFrameAndNormalizedExtension() {
		if (m_FrameCount == 1) {
			m_Frame = 0;
			return;
		}

		//TODO Test out the math here and with frame setting stuff, I don't trust/understand this and I wonder if we can maybe change the frame stuff to build the clamping in
		float extensionRange = m_MaxExtension - m_MinExtension;
		m_CurrentNormalizedExtension = (m_CurrentAnkleOffset.GetMagnitude() - (m_MaxExtension - extensionRange)) / extensionRange;
		Clamp(m_CurrentNormalizedExtension, 1.0F, 0.0F);

		m_Frame = std::min(m_FrameCount - 1, static_cast<unsigned int>(floorf(m_CurrentNormalizedExtension * static_cast<float>(m_FrameCount))));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Leg::UpdateFootFrameJointPositionAndRotation() {
		if (m_Foot) {
			m_Foot->SetJointPos(m_JointPos + m_CurrentAnkleOffset); //TODO parentoffset for feet is left at (0,0). It may be able to be used here (need to understand how rotation is done right now!) so jointpos stuff can live in Attachable entirely.

			if (IsAttached()) {
				//TODO I wonder if we could use m_CurrentAnkleOffset for this instead?
				float targetHorizontalDistanceAccountingForFlipping = g_SceneMan.ShortestDistance(m_JointPos, m_TargetOffset, g_SceneMan.SceneWrapsX()).m_X * static_cast<float>(GetFlipFactor());
				if (targetHorizontalDistanceAccountingForFlipping < -10) {
					m_Foot->SetFrame(3);
					//TODO this should probably be setting rotangle to 0 too, check if it's changing at any point in time and if not, unify
				} else if (targetHorizontalDistanceAccountingForFlipping < -6) {
					m_Foot->SetFrame(2);
					m_Foot->SetRotAngle(0.0F);
				} else if (targetHorizontalDistanceAccountingForFlipping > 6) {
					m_Foot->SetFrame(1);
					m_Foot->SetRotAngle(0.0F);
				} else {
					m_Foot->SetFrame(0);
					m_Foot->SetRotAngle(0.0F);
				}
			} else {
				m_Foot->SetRotAngle(m_Rotation.GetRadAngle() + c_PI / 2);
			}
			//m_Foot->Update(); //TODO this may need reorganizing
		}
	}
}