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
		m_TransfersDamageToParent = false;
		m_DeleteWithParent = false;

		m_JointStrength = 10;
		m_JointStiffness = 1.0;
		m_JointOffset.Reset();
		m_JointPos.Reset();

		m_OnlyLinearForces = false;

		m_DamageCount = 0;
		m_BreakWound = 0;
		m_ParentBreakWound = 0;

		m_InheritsRotAngle = true;

		m_AtomSubgroupID = -1;
		m_CollidesWithTerrainWhileAttached = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::Create(const Attachable &reference) {
		MOSRotating::Create(reference);

		m_Parent = reference.m_Parent;
		m_ParentOffset = reference.m_ParentOffset;
		m_DrawAfterParent = reference.m_DrawAfterParent;
		m_DrawnNormallyByParent = reference.m_DrawnNormallyByParent;
		m_TransfersDamageToParent = reference.m_TransfersDamageToParent;
		m_DeleteWithParent = reference.m_DeleteWithParent;

		m_JointStrength = reference.m_JointStrength;
		m_JointStiffness = reference.m_JointStiffness;
		m_JointOffset = reference.m_JointOffset;
		m_JointPos = reference.m_JointPos;

		m_OnlyLinearForces = reference.m_OnlyLinearForces;

		m_DamageCount = reference.m_DamageCount;
		m_BreakWound = reference.m_BreakWound;
		m_ParentBreakWound = reference.m_ParentBreakWound;

		m_InheritsRotAngle = reference.m_InheritsRotAngle;

		m_AtomSubgroupID = reference.m_AtomSubgroupID;
		m_CollidesWithTerrainWhileAttached = reference.m_CollidesWithTerrainWhileAttached;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "ParentOffset") {
			reader >> m_ParentOffset;
		} else if (propName == "DrawAfterParent") {
			reader >> m_DrawAfterParent;
		} else if (propName == "TransfersDamageToParent") {
			reader >> m_TransfersDamageToParent;
		} else if (propName == "DeleteWithParent") {
			reader >> m_DeleteWithParent;
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
		} else if (propName == "InheritsRotAngle") {
			reader >> m_InheritsRotAngle;
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
		writer.NewProperty("TransfersDamageToParent");
		writer << m_TransfersDamageToParent;
		writer.NewProperty("DeleteWithParent");
		writer << m_DeleteWithParent;

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

		writer.NewProperty("InheritsRotAngle");
		writer << m_InheritsRotAngle;
		writer.NewProperty("CollidesWithTerrainWhileAttached");
		writer << m_CollidesWithTerrainWhileAttached;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::TransferJointForces(Vector &jointForces) {
		if (!m_Parent) {
			return false;
		}

		Vector forces;
		for (const std::pair<Vector, Vector> &force : m_Forces) {
			forces += force.first;
		}
		m_Forces.clear();

		jointForces += forces;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::TransferJointImpulses(Vector &jointImpulses) {
		if (!m_Parent) {
			return false;
		}

		Vector impulseForces;
		for (const std::pair<Vector, Vector> &impulseForce : m_ImpulseForces) {
			impulseForces += impulseForce.first;
		}
		impulseForces *= m_JointStiffness;

		if (impulseForces.GetMagnitude() > m_JointStrength) {
			impulseForces.SetMagnitude(m_JointStrength);
			jointImpulses += impulseForces;
			if (m_BreakWound) {
				AEmitter *breakWound = dynamic_cast<AEmitter *>(m_BreakWound->Clone());
				if (breakWound) {
					breakWound->SetEmitAngle(m_JointOffset.GetAbsRadAngle());
					AddWound(breakWound, m_JointOffset, false);
					breakWound = 0;
				}
			}
			m_Parent->RemoveAttachable(this);
			g_MovableMan.AddParticle(this);
			return false;
		}

		for (const std::pair<Vector, Vector> &impulseForce : m_ImpulseForces) {
			if (!impulseForce.second.IsZero()) {
				// Rough explanation of what this is doing:
				// The first part is getting the Dot/Scalar product of the perpendicular of the offset vector for the force onto the force vector itself (dot product is the amount two vectors are pointing in the same direction).
				// The second part is dividing that Dot product by the moment of inertia, i.e. the torque needed to make it turn. All of this is multiplied by 1 - JointStiffness, because max stiffness joints transfer all force to parents and min stiffness transfer none.
				m_AngularVel += (impulseForce.second.GetPerpendicular().Dot(impulseForce.first) / m_pAtomGroup->GetMomentOfInertia()) * (1.0F - m_JointStiffness);
			}
		}
		m_ImpulseForces.clear();

		jointImpulses += impulseForces;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Attachable::CollectDamage() {
		float totalDamage = m_DamageCount;
		m_DamageCount = 0;

		for (AEmitter *wound : m_Wounds) {
			totalDamage += wound->CollectDamage();
		}
		return totalDamage * m_DamageMultiplier;
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

	//TODO change impactImpulse to a const reference Vector
	void Attachable::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO) {
		if (m_Parent) {
			m_Parent->RemoveAttachable(this);
		} else {
			SetParent(nullptr);
		}
		MOSRotating::GibThis(impactImpulse, internalBlast, pIgnoreMO);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::Update() {
		if (!m_Parent) {
			//TODO right now parents are responsible for setting JointPos. The only one that seems to do much with this is Leg, everything else looks much the same. I'd rather attachables do this internally cause that makes way more sense. Will take some cleanup though.
			m_JointPos = m_Pos + RotateOffset(m_JointOffset);
		} else {
			m_PrevPos = m_Pos;
			m_PrevVel = m_Vel;

			if (!m_JointPos.IsZero()) {
				m_Pos = m_JointPos - RotateOffset(m_JointOffset);
			} else {
				m_Pos = m_Parent->GetPos() - RotateOffset(m_JointOffset);
			}
			m_Vel = m_Parent->GetVel();
			m_Team = m_Parent->GetTeam();

			//TODO RotAngle should be set here based on whether or not this InheritsRotAngle. It's currently done by MOSR which is dumdum. Alternatively, see below.

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