#include "Attachable.h"
#include "AtomGroup.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "AEmitter.h"
#include "Actor.h"

namespace RTE {

	ConcreteClassInfo(Attachable, MOSRotating, 0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::Clear() {
		m_Parent = 0;
		m_ParentOffset.Reset();
		m_DrawAfterParent = true;
		m_DrawnNormallyByParent = true;
		m_DeleteWhenRemovedFromParent = false;
		m_ApplyTransferredForcesAtOffset = true;

		m_GibWithParentChance = 0;
		m_ParentGibBlastStrengthMultiplier = 1;

		m_JointStrength = 10;
		m_JointStiffness = 1.0;
		m_JointOffset.Reset();
		m_JointPos.Reset();

		m_DamageCount = 0;
		m_BreakWound = 0;
		m_ParentBreakWound = 0;

		m_InheritsHFlipped = 1;
		m_InheritsRotAngle = true;
		m_InheritedRotAngleOffset = 0;

		m_AtomSubgroupID = -1;
		m_CollidesWithTerrainWhileAttached = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::Create() {
		MOSRotating::Create();

		m_AtomSubgroupID = GetUniqueID();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::Create(const Attachable &reference) {
		MOSRotating::Create(reference);

		m_Parent = reference.m_Parent;
		m_ParentOffset = reference.m_ParentOffset;
		m_DrawAfterParent = reference.m_DrawAfterParent;
		m_DrawnNormallyByParent = reference.m_DrawnNormallyByParent;
		m_DeleteWhenRemovedFromParent = reference.m_DeleteWhenRemovedFromParent;
		m_ApplyTransferredForcesAtOffset = reference.m_ApplyTransferredForcesAtOffset;

		m_GibWithParentChance = reference.m_GibWithParentChance;
		m_ParentGibBlastStrengthMultiplier = reference.m_ParentGibBlastStrengthMultiplier;

		m_JointStrength = reference.m_JointStrength;
		m_JointStiffness = reference.m_JointStiffness;
		m_JointOffset = reference.m_JointOffset;
		m_JointPos = reference.m_JointPos;

		m_DamageCount = reference.m_DamageCount;
		m_BreakWound = reference.m_BreakWound;
		m_ParentBreakWound = reference.m_ParentBreakWound;

		m_InheritsHFlipped = reference.m_InheritsHFlipped;
		m_InheritsRotAngle = reference.m_InheritsRotAngle;
		m_InheritedRotAngleOffset = reference.m_InheritedRotAngleOffset;

		m_CollidesWithTerrainWhileAttached = reference.m_CollidesWithTerrainWhileAttached;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "ParentOffset") {
			reader >> m_ParentOffset;
		} else if (propName == "DrawAfterParent") {
			reader >> m_DrawAfterParent;
		} else if (propName == "DeleteWhenRemovedFromParent") {
			reader >> m_DeleteWhenRemovedFromParent;
		} else if (propName == "ApplyTransferredForcesAtOffset") {
			reader >> m_ApplyTransferredForcesAtOffset;
		} else if (propName == "GibWithParentChance") {
			reader >> m_GibWithParentChance;
		} else if (propName == "ParentGibBlastStrengthMultiplier") {
			reader >> m_ParentGibBlastStrengthMultiplier;
		} else if (propName == "JointStrength" || propName == "Strength") {
			reader >> m_JointStrength;
		} else if (propName == "JointStiffness" || propName == "Stiffness") {
			reader >> m_JointStiffness;
		} else if (propName == "JointOffset") {
			reader >> m_JointOffset;
		} else if (propName == "BreakWound") {
			m_BreakWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
			if (!m_ParentBreakWound) { m_ParentBreakWound = m_BreakWound; }
		} else if (propName == "ParentBreakWound") {
			m_ParentBreakWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
		} else if (propName == "InheritsHFlipped") {
			reader >> m_InheritsHFlipped;
			if (m_InheritsHFlipped != 0 && m_InheritsHFlipped != 1) { m_InheritsHFlipped = 2; }
		} else if (propName == "InheritsRotAngle") {
			reader >> m_InheritsRotAngle;
		} else if (propName == "InheritedRotAngleRadOffset" || propName == "InheritedRotAngleOffset") {
			reader >> m_InheritedRotAngleOffset;
		} else if (propName == "InheritedRotAngleDegOffset") {
			m_InheritedRotAngleOffset = DegreesToRadians(std::stof(reader.ReadPropValue()));
		} else if (propName == "CollidesWithTerrainWhenAttached") {
			reader >> m_CollidesWithTerrainWhileAttached;
		} else {
			return MOSRotating::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::Save(Writer &writer) const {
		MOSRotating::Save(writer);

		writer.NewProperty("ParentOffset");
		writer << m_ParentOffset;
		writer.NewProperty("DrawAfterParent");
		writer << m_DrawAfterParent;
		writer.NewProperty("DeleteWhenRemovedFromParent");
		writer << m_DeleteWhenRemovedFromParent;
		writer.NewProperty("ApplyTransferredForcesAtOffset");
		writer << m_ApplyTransferredForcesAtOffset;

		writer.NewProperty("JointStrength");
		writer << m_JointStrength;
		writer.NewProperty("JointStiffness");
		writer << m_JointStiffness;
		writer.NewProperty("JointOffset");
		writer << m_JointOffset;

		writer.NewProperty("BreakWound");
		writer << m_BreakWound;
		writer.NewProperty("ParentBreakWound");
		writer << m_ParentBreakWound;

		writer.NewProperty("InheritsHFlipped");
		writer << ((m_InheritsHFlipped == 0 || m_InheritsHFlipped == 1) ? m_InheritsHFlipped : 2);
		writer.NewProperty("InheritsRotAngle");
		writer << m_InheritsRotAngle;
		writer.NewProperty("InheritedRotAngleOffset");
		writer << m_InheritedRotAngleOffset;

		writer.NewProperty("CollidesWithTerrainWhileAttached");
		writer << m_CollidesWithTerrainWhileAttached;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::TransferJointForces(Vector &jointForces) {
		if (!m_Parent) {
			return false;
		}

		Vector totalForce;
		for (const std::pair<Vector, Vector> &force : m_Forces) {
			totalForce += force.first;
		}

		jointForces += totalForce;
		m_Forces.clear();
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::TransferJointImpulses(Vector &jointImpulses) {
		if (!m_Parent) {
			return false;
		}

		Vector totalImpulseForce;
		for (const std::pair<Vector, Vector> &impulseForce : m_ImpulseForces) {
			totalImpulseForce += impulseForce.first;
		}

		if (m_GibImpulseLimit > 0 && totalImpulseForce.GetMagnitude() > m_GibImpulseLimit) {
			jointImpulses += (totalImpulseForce.SetMagnitude(totalImpulseForce.GetMagnitude() - m_GibImpulseLimit)) * m_JointStiffness;
			GibThis();
			return false;
		} else if (m_JointStrength > 0 && totalImpulseForce.GetMagnitude() > m_JointStrength) {
			jointImpulses += (totalImpulseForce.SetMagnitude(totalImpulseForce.GetMagnitude() - m_JointStiffness)) * m_JointStiffness;
			m_Parent->RemoveAttachable(this, true, true);
			return false;
		} else {
			jointImpulses += totalImpulseForce * m_JointStiffness;
		}

		// Rough explanation of what this is doing:
		// The first part is getting the Dot/Scalar product of the perpendicular of the offset vector for the force onto the force vector itself (dot product is the amount two vectors are pointing in the same direction).
		// The second part is dividing that Dot product by the moment of inertia, i.e. the torque needed to make it turn. All of this is multiplied by 1 - JointStiffness, because max stiffness joints transfer all force to parents and min stiffness transfer none.
		if (!m_InheritsRotAngle) {
			for (const std::pair<Vector, Vector> &impulseForce : m_ImpulseForces) {
				if (!impulseForce.second.IsZero()) {
					m_AngularVel += (impulseForce.second.GetPerpendicular().Dot(impulseForce.first) / m_pAtomGroup->GetMomentOfInertia()) * (1.0F - m_JointStiffness);
				}
			}
		}

		m_ImpulseForces.clear();
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Attachable::CollectDamage() {
		if (m_DamageMultiplier != 0) {
			float totalDamage = m_DamageCount;
			m_DamageCount = 0;

			for (AEmitter *wound : m_Wounds) {
				totalDamage += wound->CollectDamage();
			}
			for (Attachable *attachable : m_Attachables) {
				totalDamage += attachable->CollectDamage();
			}
			return totalDamage * m_DamageMultiplier;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::SetCollidesWithTerrainWhileAttached(bool collidesWithTerrainWhileAttached) {
		if (m_CollidesWithTerrainWhileAttached != collidesWithTerrainWhileAttached) {
			OrganizeAtomsInParent(collidesWithTerrainWhileAttached);
			m_CollidesWithTerrainWhileAttached = collidesWithTerrainWhileAttached;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::ParticlePenetration(HitData &hd) {
		bool penetrated = MOSRotating::ParticlePenetration(hd);

		if (hd.Body[HITOR]->DamageOnCollision() != 0) {
			AddDamage(hd.Body[HITOR]->DamageOnCollision());
		}

		if (penetrated && m_Parent) {
			if (hd.Body[HITOR]->DamageOnPenetration() != 0) {
				AddDamage(hd.Body[HITOR]->DamageOnPenetration());
			}

			// If the parent is an actor, generate an alarm point for them, moving it slightly away from the body (in the direction they got hit from) to get a good reaction.
			Actor *parentAsActor = dynamic_cast<Actor *>(GetRootParent());
			if (parentAsActor) {
				Vector extruded(hd.HitVel[HITOR]);
				extruded.SetMagnitude(parentAsActor->GetHeight());
				extruded = m_Pos - extruded;
				g_SceneMan.WrapPosition(extruded);
				parentAsActor->AlarmPoint(extruded);
			}
		}

		return penetrated;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
		m_ToDelete = true; // Note: ToDelete must be set to true ahead of time to avoid oddities with breakwounds
		if (m_Parent) {
			m_Parent->RemoveAttachable(this, true, true);
		} else {
			SetParent(nullptr);
		}
		MOSRotating::GibThis(impactImpulse, movableObjectToIgnore);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::Update() {
		if (!m_Parent) {
			m_JointPos = m_Pos + RotateOffset(m_JointOffset);
		} else {
			m_PrevPos = m_Pos;
			m_PrevVel = m_Vel;

			m_JointPos = m_Parent->GetPos() + m_Parent->RotateOffset(GetParentOffset());
			m_Pos = m_JointPos - RotateOffset(m_JointOffset);
			m_Vel = m_Parent->GetVel();
			m_Team = m_Parent->GetTeam();
			if (InheritsHFlipped() != 0) { m_HFlipped = m_InheritsHFlipped == 1 ? m_Parent->IsHFlipped() : !m_Parent->IsHFlipped(); }
			if (InheritsRotAngle()) { SetRotAngle(m_Parent->GetRotAngle() + m_InheritedRotAngleOffset); }

			if (m_CollidesWithTerrainWhileAttached) {
				float facingAngle = (m_HFlipped ? c_PI : 0) + GetRotAngle() * static_cast<float>(GetFlipFactor());
				float parentFacingAngle = (m_Parent->IsHFlipped() ? c_PI : 0) + m_Parent->GetRotAngle() * static_cast<float>(m_Parent->GetFlipFactor());

				Matrix atomRot(facingAngle - parentFacingAngle);
				m_pAtomGroup->UpdateSubAtoms(GetAtomSubgroupID(), GetParentOffset() - (GetJointOffset() * atomRot), atomRot);
			}

			m_DeepCheck = false;
		}

		MOSRotating::Update();

		// If we're attached to something, MovableMan doesn't own us, and therefore isn't calling our UpdateScripts method (and neither is our parent), so we should here.
		if (m_Parent != nullptr && GetRootParent()->HasEverBeenAddedToMovableMan()) { UpdateScripts(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::SetParent(MOSRotating *newParent) {
		if (newParent == m_Parent) {
			return;
		}

		m_Team = newParent ? newParent->GetTeam() : -1;
		MOSRotating *parentToUseForScriptCall = newParent ? newParent : m_Parent;
		
		if (newParent) {
			//TODO see if this is reasonable. Seems like inventory swapping should do this cleanup internally. Also note that it used to be done regardless of newParent existing (i.e. any time Attach was called) but I changed that, might be bad?
			
			// Timers are reset here as a precaution, so that if something was sitting in an inventory, it doesn't cause backed up emissions.
			ResetAllTimers();

			m_Parent = newParent;
			OrganizeAtomsInParent(newParent != nullptr);
		} else {
			m_RootMOID = m_MOID;
			m_RestTimer.Reset();

			OrganizeAtomsInParent(newParent != nullptr);
			m_Parent = newParent;
		}

		if (parentToUseForScriptCall && parentToUseForScriptCall->GetRootParent()->HasEverBeenAddedToMovableMan()) {
			RunScriptedFunctionInAppropriateScripts(newParent ? "OnAttach" : "OnDetach", false, false, {parentToUseForScriptCall});
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::OrganizeAtomsInParent(bool addToParent) {
		if (IsAttached() && m_Parent->GetAtomGroup()) {
			if (addToParent) {
				m_Parent->GetAtomGroup()->AddAtoms(GetAtomGroup()->GetAtomList(), GetAtomSubgroupID(), GetParentOffset() - GetJointOffset());
			} else {
				m_Parent->GetAtomGroup()->RemoveAtoms(GetAtomSubgroupID());
			}
		}
	}
}