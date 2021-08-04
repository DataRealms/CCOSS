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
		m_Parent = nullptr;
		m_ParentOffset.Reset();
		m_DrawAfterParent = true;
		m_DrawnNormallyByParent = true;
		m_DeleteWhenRemovedFromParent = false;
		m_ApplyTransferredForcesAtOffset = true;

		m_GibWithParentChance = 0.0F;
		m_ParentGibBlastStrengthMultiplier = 1.0F;

		m_IsWound = false;

		m_JointStrength = 10.0F;
		m_JointStiffness = 1.0F;
		m_JointOffset.Reset();
		m_JointPos.Reset();

		m_DamageCount = 0.0F;
		m_BreakWound = nullptr;
		m_ParentBreakWound = nullptr;

		m_InheritsHFlipped = 1;
		m_InheritsRotAngle = true;
		m_InheritedRotAngleOffset = 0.0F;
		m_InheritsFrame = false;

		m_AtomSubgroupID = -1L;
		m_CollidesWithTerrainWhileAttached = true;

		m_PrevRotAngleOffset = 0.0F;
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

		m_ParentOffset = reference.m_ParentOffset;
		m_DrawAfterParent = reference.m_DrawAfterParent;
		m_DrawnNormallyByParent = reference.m_DrawnNormallyByParent;
		m_DeleteWhenRemovedFromParent = reference.m_DeleteWhenRemovedFromParent;
		m_ApplyTransferredForcesAtOffset = reference.m_ApplyTransferredForcesAtOffset;

		m_GibWithParentChance = reference.m_GibWithParentChance;
		m_ParentGibBlastStrengthMultiplier = reference.m_ParentGibBlastStrengthMultiplier;

		m_IsWound = reference.m_IsWound;

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
		m_InheritsFrame = reference.m_InheritsFrame;

		m_AtomSubgroupID = GetUniqueID();
		m_CollidesWithTerrainWhileAttached = reference.m_CollidesWithTerrainWhileAttached;

		m_PrevRotAngleOffset = reference.m_PrevRotAngleOffset;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Attachable::ReadProperty(const std::string_view &propName, Reader &reader) {
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
		} else if (propName == "ParentBreakWound") {
			m_ParentBreakWound = dynamic_cast<const AEmitter *>(g_PresetMan.GetEntityPreset(reader));
		} else if (propName == "InheritsHFlipped") {
			reader >> m_InheritsHFlipped;
			if (m_InheritsHFlipped != 0 && m_InheritsHFlipped != 1) { m_InheritsHFlipped = -1; }
		} else if (propName == "InheritsRotAngle") {
			reader >> m_InheritsRotAngle;
		} else if (propName == "InheritedRotAngleRadOffset" || propName == "InheritedRotAngleOffset") {
			reader >> m_InheritedRotAngleOffset;
		} else if (propName == "InheritedRotAngleDegOffset") {
			m_InheritedRotAngleOffset = DegreesToRadians(std::stof(reader.ReadPropValue()));
		} else if (propName == "InheritsFrame") {
			reader >> m_InheritsFrame;
		} else if (propName == "CollidesWithTerrainWhileAttached") {
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
		if (m_Forces.empty()) {
			return true;
		}

		Vector totalForce;
		for (const auto &[force, forceOffset] : m_Forces) {
			totalForce += force;
		}

		jointForces += totalForce;
		m_Forces.clear();
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::TransferJointImpulses(Vector &jointImpulses, float jointStiffnessValueToUse, float jointStrengthValueToUse, float gibImpulseLimitValueToUse) {
		if (!m_Parent) {
			return false;
		}
		if (m_ImpulseForces.empty()) {
			return true;
		}
		jointStiffnessValueToUse = jointStiffnessValueToUse > 0 ? jointStiffnessValueToUse : m_JointStiffness;
		jointStrengthValueToUse = jointStrengthValueToUse > 0 ? jointStrengthValueToUse : m_JointStrength;
		gibImpulseLimitValueToUse = gibImpulseLimitValueToUse > 0 ? gibImpulseLimitValueToUse : m_GibImpulseLimit;
		if (gibImpulseLimitValueToUse > 0) { gibImpulseLimitValueToUse = std::max(gibImpulseLimitValueToUse, jointStrengthValueToUse); }

		Vector totalImpulseForce;
		for (const auto &[impulseForce, impulseForceOffset] : m_ImpulseForces) {
			totalImpulseForce += impulseForce;
		}
		totalImpulseForce *= jointStiffnessValueToUse;

		if (gibImpulseLimitValueToUse > 0 && totalImpulseForce.GetMagnitude() > gibImpulseLimitValueToUse) {
			jointImpulses += totalImpulseForce.SetMagnitude(gibImpulseLimitValueToUse);
			GibThis();
			return false;
		} else if (jointStrengthValueToUse > 0 && totalImpulseForce.GetMagnitude() > jointStrengthValueToUse) {
			jointImpulses += totalImpulseForce.SetMagnitude(jointStrengthValueToUse);
			m_Parent->RemoveAttachable(this, true, true);
			return false;
		} else {
			jointImpulses += totalImpulseForce;
		}

		// Rough explanation of what this is doing:
		// The first part is getting the Dot/Scalar product of the perpendicular of the offset vector for the force onto the force vector itself (dot product is the amount two vectors are pointing in the same direction).
		// The second part is dividing that Dot product by the moment of inertia, i.e. the torque needed to make it turn. All of this is multiplied by 1 - JointStiffness, because max stiffness joints transfer all force to parents and min stiffness transfer none.
		if (!m_InheritsRotAngle) {
			for (const auto &[impulseForce, impulseForceOffset] : m_ImpulseForces) {
				if (!impulseForceOffset.IsZero()) {
					m_AngularVel += (impulseForceOffset.GetPerpendicular().Dot(impulseForce) / m_pAtomGroup->GetMomentOfInertia()) * (1.0F - jointStiffnessValueToUse);
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
			bool previousTerrainCollisionValue = CanCollideWithTerrain();
			m_CollidesWithTerrainWhileAttached = collidesWithTerrainWhileAttached;

			if (previousTerrainCollisionValue != CanCollideWithTerrain()) {
				AddOrRemoveAtomsFromRootParentAtomGroup(collidesWithTerrainWhileAttached, true);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::CanCollideWithTerrain() const {
		if (m_CollidesWithTerrainWhileAttached && IsAttached() && GetParent() != GetRootParent()) {
			if (const Attachable *parentAsAttachable = dynamic_cast<const Attachable *>(GetParent())) { return parentAsAttachable->CanCollideWithTerrain(); }
		}
		return m_CollidesWithTerrainWhileAttached;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::ParticlePenetration(HitData &hd) {
		bool penetrated = MOSRotating::ParticlePenetration(hd);

		if (m_Parent) {
			MovableObject *hitor = hd.Body[HITOR];
			float damageToAdd = hitor->DamageOnCollision();
			damageToAdd += penetrated ? hitor->DamageOnPenetration() : 0;
			if (hitor->GetApplyWoundDamageOnCollision()) { damageToAdd += m_pEntryWound->GetEmitDamage() * hitor->WoundDamageMultiplier(); }
			if (hitor->GetApplyWoundBurstDamageOnCollision()) { damageToAdd += m_pEntryWound->GetBurstDamage() * hitor->WoundDamageMultiplier(); }

			if (damageToAdd != 0) {
				AddDamage(damageToAdd);

				if (Actor *parentAsActor = dynamic_cast<Actor *>(GetRootParent()); parentAsActor && parentAsActor->GetPerceptiveness() > 0) {
					Vector extruded(hd.HitVel[HITOR]);
					extruded.SetMagnitude(parentAsActor->GetHeight());
					extruded = m_Pos - extruded;
					g_SceneMan.WrapPosition(extruded);
					parentAsActor->AlarmPoint(extruded);
				}
			}
		}

		return penetrated;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
		if (m_Parent) { m_Parent->RemoveAttachable(this, true, true); }
		MOSRotating::GibThis(impactImpulse, movableObjectToIgnore);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Attachable::HandlePotentialRadiusAffectingAttachable(const Attachable *attachable) {
		if (MOSRotating::HandlePotentialRadiusAffectingAttachable(attachable)) {
			if (IsAttached()) { m_Parent->HandlePotentialRadiusAffectingAttachable(this); }
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::Update() {
		if (!m_PreUpdateHasRunThisFrame) { PreUpdate(); }
		if (m_Parent) {
			UpdatePositionAndJointPositionBasedOnOffsets();
			if (m_ParentOffset != m_PrevParentOffset || m_JointOffset != m_PrevJointOffset) { m_Parent->HandlePotentialRadiusAffectingAttachable(this); }
			m_Vel = m_Parent->GetVel();
			m_Team = m_Parent->GetTeam();

			MOSRotating *rootParentAsMOSR = dynamic_cast<MOSRotating *>(GetRootParent());
			float currentRotAngleOffset = (GetRotAngle() * GetFlipFactor()) - rootParentAsMOSR->GetRotAngle();
			if (rootParentAsMOSR && CanCollideWithTerrain()) {
				// Note: This safety check exists to ensure the parent's AtomGroup contains this Attachable's Atoms in a subgroup. Hardcoded Attachables need this in order to work, since they're cloned before their parent's AtomGroup exists.
				if (!rootParentAsMOSR->GetAtomGroup()->ContainsSubGroup(m_AtomSubgroupID)) { AddOrRemoveAtomsFromRootParentAtomGroup(true, false); }

				if (std::abs(currentRotAngleOffset - m_PrevRotAngleOffset) > 0.01745F) { // Update for 1 degree differences
					Matrix atomRotationForSubgroup(rootParentAsMOSR->FacingAngle(GetRotAngle()) - rootParentAsMOSR->FacingAngle(rootParentAsMOSR->GetRotAngle()));
					Vector atomOffsetForSubgroup(g_SceneMan.ShortestDistance(rootParentAsMOSR->GetPos(), m_Pos, g_SceneMan.SceneWrapsX()).FlipX(rootParentAsMOSR->IsHFlipped()));
					Matrix rootParentAngleToUse(rootParentAsMOSR->GetRotAngle() * static_cast<float>(rootParentAsMOSR->GetFlipFactor()));
					atomOffsetForSubgroup /= rootParentAngleToUse;
					rootParentAsMOSR->GetAtomGroup()->UpdateSubAtoms(GetAtomSubgroupID(), atomOffsetForSubgroup, atomRotationForSubgroup);
				}
			}
			m_DeepCheck = false;
			m_PrevRotAngleOffset = currentRotAngleOffset;
		} else {
			UpdatePositionAndJointPositionBasedOnOffsets();
		}

		MOSRotating::Update();

		if (m_Parent && m_InheritsFrame) { SetFrame(m_Parent->GetFrame()); }

		// If we're attached to something, MovableMan doesn't own us, and therefore isn't calling our UpdateScripts method (and neither is our parent), so we should here.
		if (m_Parent && GetRootParent()->HasEverBeenAddedToMovableMan()) { UpdateScripts(); }

		m_PrevPos = m_Pos;
		m_PrevVel = m_Vel;
		m_PrevParentOffset = m_ParentOffset;
		m_PrevJointOffset = m_JointOffset;
		m_PreUpdateHasRunThisFrame = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::PreUpdate() {
		if (!m_PreUpdateHasRunThisFrame) {
			if (m_Parent) {
				if (InheritsHFlipped() != 0) { m_HFlipped = m_InheritsHFlipped == 1 ? m_Parent->IsHFlipped() : !m_Parent->IsHFlipped(); }
				if (InheritsRotAngle()) {
					SetRotAngle(m_Parent->GetRotAngle() + m_InheritedRotAngleOffset * m_Parent->GetFlipFactor());
					m_AngularVel = 0.0F;
				}
			}
			m_PreUpdateHasRunThisFrame = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::SetMass(const float newMass) {
		float currentMass = GetMass();
		if (newMass != currentMass) {
			float previousMassForUpdatingParent = m_Parent ? currentMass : 0.0F;
			MovableObject::SetMass(newMass);
			if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::UpdateAttachableAndWoundMass(float oldAttachableOrWoundMass, float newAttachableOrWoundMass) {
		float previousMassForUpdatingParent = m_Parent ? GetMass() : 0.0F;
		MOSRotating::UpdateAttachableAndWoundMass(oldAttachableOrWoundMass, newAttachableOrWoundMass);
		if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::AddAttachable(Attachable *attachable, const Vector &parentOffsetToSet) {
		float previousMassForUpdatingParent = m_Parent ? GetMass() : 0.0F;
		MOSRotating::AddAttachable(attachable, parentOffsetToSet);
		if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Attachable * Attachable::RemoveAttachable(Attachable *attachable, bool addToMovableMan, bool addBreakWounds) {
		float previousMassForUpdatingParent = m_Parent ? GetMass() : 0.0F;
		Attachable *removedAttachable = MOSRotating::RemoveAttachable(attachable, addToMovableMan, addBreakWounds);
		if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
		return removedAttachable;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::AddWound(AEmitter *woundToAdd, const Vector &parentOffsetToSet, bool checkGibWoundLimit) {
		float previousMassForUpdatingParent = m_Parent ? GetMass() : 0.0F;
		MOSRotating::AddWound(woundToAdd, parentOffsetToSet, checkGibWoundLimit);
		if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Attachable::RemoveWounds(int numberOfWoundsToRemove, bool includeAttachablesWithAPositiveDamageMultiplier, bool includeAttachablesWithANegativeDamageMultiplier, bool includeAttachablesWithNoDamageMultiplier) {
		float previousMassForUpdatingParent = m_Parent ? GetMass() : 0.0F;
		float result = MOSRotating::RemoveWounds(numberOfWoundsToRemove, includeAttachablesWithAPositiveDamageMultiplier, includeAttachablesWithANegativeDamageMultiplier, includeAttachablesWithNoDamageMultiplier);
		if (m_Parent) { m_Parent->UpdateAttachableAndWoundMass(previousMassForUpdatingParent, GetMass()); }
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::SetParent(MOSRotating *newParent) {
		if (newParent == m_Parent) {
			return;
		}
		RTEAssert(!(m_Parent && newParent), "Tried to set an Attachable's " + GetModuleAndPresetName() + " parent without first unsetting its old parent, " + (IsAttached() ? GetParent()->GetModuleAndPresetName() : "ERROR") + ".");
		MOSRotating *parentToUseForScriptCall = newParent ? newParent : m_Parent;

		//TODO Get rid of the need for calling ResetAllTimers, if something like inventory swapping needs timers reset it should do it itself! This blanket handling probably has side-effects.
		// Timers are reset here as a precaution, so that if something was sitting in an inventory, it doesn't cause backed up emissions.
		ResetAllTimers();
		
		if (newParent) {
			m_Parent = newParent;
			m_Team = newParent->GetTeam();
			if (InheritsHFlipped() != 0) { m_HFlipped = m_InheritsHFlipped == 1 ? m_Parent->IsHFlipped() : !m_Parent->IsHFlipped(); }
			if (InheritsRotAngle()) {
				SetRotAngle(m_Parent->GetRotAngle() + m_InheritedRotAngleOffset * static_cast<float>(m_Parent->GetFlipFactor()));
				m_AngularVel = 0.0F;
			}
			UpdatePositionAndJointPositionBasedOnOffsets();
			if (CanCollideWithTerrain()) { AddOrRemoveAtomsFromRootParentAtomGroup(true, true); }
		} else {
			m_RootMOID = m_MOID;
			m_RestTimer.Reset();
			m_Team = -1;
			m_IsWound = false;

			if (MovableObject *rootParent = GetRootParent()) {
				const MovableObject *whichMOToNotHit = GetWhichMOToNotHit();
				const MovableObject *rootParentMOToNotHit = rootParent->GetWhichMOToNotHit();
				if ((whichMOToNotHit && whichMOToNotHit != rootParent) || (rootParentMOToNotHit && rootParentMOToNotHit != this)) {
					m_pMOToNotHit = nullptr;
				} else {
					m_pMOToNotHit = rootParent;
					rootParent->SetWhichMOToNotHit(this);
				}
			}

			if (CanCollideWithTerrain()) { AddOrRemoveAtomsFromRootParentAtomGroup(false, true); }
			m_Parent = newParent;
			for (Attachable *attachable : m_Attachables) {
				if (attachable->m_CollidesWithTerrainWhileAttached) { attachable->AddOrRemoveAtomsFromRootParentAtomGroup(true, true); }
			}
		}

		if (parentToUseForScriptCall && parentToUseForScriptCall->GetRootParent()->HasEverBeenAddedToMovableMan()) {
			RunScriptedFunctionInAppropriateScripts(newParent ? "OnAttach" : "OnDetach", false, false, {parentToUseForScriptCall});
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::UpdatePositionAndJointPositionBasedOnOffsets() {
		if (m_Parent) {
			m_JointPos = m_Parent->GetPos() + m_Parent->RotateOffset(GetParentOffset());
			m_Pos = m_JointPos - RotateOffset(m_JointOffset);
		} else {
			m_JointPos = m_Pos + RotateOffset(m_JointOffset);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Attachable::AddOrRemoveAtomsFromRootParentAtomGroup(bool addAtoms, bool propagateToChildAttachables) {
		if (IsAttached()) {
			MOSRotating *rootParentAsMOSR = dynamic_cast<MOSRotating *>(GetRootParent());
			AtomGroup *rootParentAtomGroup = rootParentAsMOSR ? rootParentAsMOSR->GetAtomGroup() : nullptr;
			if (rootParentAtomGroup) {
				if (addAtoms && !rootParentAtomGroup->ContainsSubGroup(GetAtomSubgroupID())) {
					Vector atomOffsetForSubgroup = g_SceneMan.ShortestDistance(rootParentAsMOSR->GetPos(), m_Pos, g_SceneMan.SceneWrapsX());
					atomOffsetForSubgroup.FlipX(rootParentAsMOSR->IsHFlipped());
					Matrix atomRotationForSubgroup(rootParentAsMOSR->FacingAngle(GetRotAngle()) - rootParentAsMOSR->FacingAngle(rootParentAsMOSR->GetRotAngle()));
					rootParentAtomGroup->AddAtoms(GetAtomGroup()->GetAtomList(), GetAtomSubgroupID(), atomOffsetForSubgroup, atomRotationForSubgroup);
				} else if (!addAtoms && rootParentAtomGroup->ContainsSubGroup(GetAtomSubgroupID())) {
					rootParentAtomGroup->RemoveAtoms(GetAtomSubgroupID());
				}

				if (propagateToChildAttachables) {
					for (Attachable *attachable : m_Attachables) {
						if (attachable->m_CollidesWithTerrainWhileAttached) { attachable->AddOrRemoveAtomsFromRootParentAtomGroup(addAtoms, propagateToChildAttachables); }
					}
				}
			}
		}
	}
}
