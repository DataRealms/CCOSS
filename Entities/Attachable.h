#ifndef _RTEATTACHABLE_
#define _RTEATTACHABLE_

#include "MOSRotating.h"

namespace RTE {

	class AEmitter;

	/// <summary>
	/// An articulated, detachable part of an Actor's body.
	/// </summary>
	class Attachable : public MOSRotating {
		friend class MOSRotating;

	public:

		EntityAllocation(Attachable)
		AddScriptFunctionNames(MOSRotating, "OnAttach", "OnDetach")
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Attachable object in system memory. Create() should be called before using the object.
		/// </summary>
		Attachable() { Clear(); }
		
		/// <summary>
		/// Makes the Attachable object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates an Attachable to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Attachable to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Attachable &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an Attachable object before deletion from system memory.
		/// </summary>
		~Attachable() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Attachable object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { MOSRotating::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire Attachable, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); MOSRotating::Reset(); }
#pragma endregion

#pragma region Parent Getters and Setters
		/// <summary>
		/// Gets the MOSRotating which is the parent of this Attachable.
		/// </summary>
		/// <returns>A pointer to the parent of this Attachable.</returns>
		MOSRotating * GetParent() override { return m_Parent; }

		/// <summary>
		/// Gets the MOSRotating which is the parent of this Attachable. 
		/// </summary>
		/// <returns>A pointer to the parent of this Attachable.</returns>
		const MOSRotating * GetParent() const override { return m_Parent; }

		/// <summary>
		/// Indicates whether this Attachable is attached to an MOSRotating parent or not.
		/// </summary>
		/// <returns>Whether it's attached or not.</returns>
		bool IsAttached() const { return m_Parent != nullptr; }

		/// <summary>
		/// Indicates whether this Attachable is attached to the specified MOSRotating or not.
		/// </summary>
		/// <param name="parentToCheck">A pointer to which MOSRotating you want to check for.</param>
		/// <returns>Whether it's attached or not.</returns>
		bool IsAttachedTo(const MOSRotating *parentToCheck) const { return m_Parent == parentToCheck; }

		/// <summary>
		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// </summary>
		/// <returns>A pointer to the highest root parent of this Attachable.</returns>
		MovableObject * GetRootParent() override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// <summary>
		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// </summary>
		/// <returns>A pointer to the highest root parent of this Attachable.</returns>
		const MovableObject * GetRootParent() const override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// <summary>
		/// Gets the stored offset between this Attachable's parent's position and the joint position. This should be maintained by the parent.
		/// </summary>
		/// <returns>A const reference Vector describing the offset from the parent's position to the joint position.</returns>
		const Vector & GetParentOffset() const { return m_ParentOffset; }

		/// <summary>
		/// Sets the stored offset between this Attachable's parent's Pos and the joint position. This should be maintained by the parent.
		/// </summary>
		/// <param name="newParentOffset">A const reference to the new parent offset.</param>
		void SetParentOffset(const Vector &newParentOffset) { m_ParentOffset = newParentOffset; }

		/// <summary>
		/// Gets whether this Attachable is to be drawn after (in front of) or before (behind) its parent.
		/// </summary>
		/// <returns>Whether this Attachable is to be drawn after its parent or not.</returns>
		bool IsDrawnAfterParent() const override { return m_DrawAfterParent; }

		/// <summary>
		/// Sets whether this Attachable is to be drawn after (in front of) or before (behind) its parent.
		/// </summary>
		/// <param name="drawAfterParent">Whether this Attachable is to be drawn after its parent.</param>
		void SetDrawnAfterParent(bool drawAfterParent) { m_DrawAfterParent = drawAfterParent; }

		/// <summary>
		/// Gets whether this Attachable should be drawn normally by its parent.
		/// Some attachables (e.g. AEmitter flashes) require custom handling for when they should or shouldn't draw, to be done by the specific parent class.
		/// </summary>
		/// <returns>Whether this Attachable should be drawn normally by its parent.</returns>
		bool IsDrawnNormallyByParent() const { return m_DrawnNormallyByParent; }

		/// <summary>
		/// Sets whether this Attachable should be drawn normally by its parent.
		/// Some attachables (e.g. AEmitter flashes) require custom handling for when they should or shouldn't draw, to be done by the specific parent class.
		/// </summary>
		/// <param name="drawnNormallyByParent">Whether this Attachable should be drawn normally by its parent.</param>
		void SetDrawnNormallyByParent(bool drawnNormallyByParent) { m_DrawnNormallyByParent = drawnNormallyByParent; }

		/// <summary>
		/// Gets whether this Attachable will be deleted when it's removed from its parent. Has no effect until the Attachable is added to a parent.
		/// </summary>
		/// <returns>Whether this Attachable is marked to be deleted when it's removed from its parent or not.</returns>
		bool GetDeleteWhenRemovedFromParent() const { return m_DeleteWhenRemovedFromParent; }

		/// <summary>
		/// Sets whether this Attachable will be deleted when it's removed from its parent.
		/// </summary>
		/// <param name="deleteWhenRemovedFromParent">Whether this Attachable should be deleted when it's removed from its parent.</param>
		virtual void SetDeleteWhenRemovedFromParent(bool deleteWhenRemovedFromParent) { m_DeleteWhenRemovedFromParent = deleteWhenRemovedFromParent; }

		/// <summary>
		/// Gets whether forces transferred from this Attachable should be applied at its parent's offset (rotated to match the parent) where they will produce torque, or directly at its parent's position.
		/// </summary>
		/// <returns>Whether forces transferred from this Attachable should be applied at an offset.</returns>
		bool GetApplyTransferredForcesAtOffset() const { return m_ApplyTransferredForcesAtOffset; }

		/// <summary>
		/// Sets whether forces transferred from this Attachable should be applied at its parent's offset (rotated to match the parent) where they will produce torque, or directly at its parent's position.
		/// </summary>
		/// <param name="appliesTransferredForcesAtOffset">Whether forces transferred from this Attachable should be applied at an offset.</param>
		void SetApplyTransferredForcesAtOffset(bool appliesTransferredForcesAtOffset) { m_ApplyTransferredForcesAtOffset = appliesTransferredForcesAtOffset; }
#pragma endregion

#pragma region Parent Gib Handling Getters and Setters
		/// <summary>
		/// Gets the percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		/// </summary>
		/// <returns>A float with the percentage chance this Attachable will gib when its parent gibs.</returns>
		float GetGibWithParentChance() const { return m_GibWithParentChance; }

		/// <summary>
		/// Sets the percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		/// </summary>
		/// <param name="gibWithParentChance">A float describing the percentage chance this Attachable will gib when its parent gibs.</param>
		void SetGibWithParentChance(float gibWithParentChance) { m_GibWithParentChance = gibWithParentChance; }

		/// <summary>
		/// Gets the multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs
		/// </summary>
		/// <returns>A float with the parent gib blast strength multiplier of this Attachable.</returns>
		float GetParentGibBlastStrengthMultiplier() const { return m_ParentGibBlastStrengthMultiplier; }

		/// <summary>
		/// Sets the multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs
		/// </summary>
		/// <param name="parentGibBlastStrengthMultiplier">A float describing the parent gib blast strength multiplier of this Attachable.</param>
		void SetParentGibBlastStrengthMultiplier(float parentGibBlastStrengthMultiplier) { m_ParentGibBlastStrengthMultiplier = parentGibBlastStrengthMultiplier; }
#pragma endregion

#pragma region Temporary Handling for Wounds, to be Replaced by a Wound Object in Future
		/// <summary>
		/// Gets whether or not this Attachable is a wound, as determined by its parent MOSR.
		/// </summary>
		/// <returns>Whether or not this Attachable is a wound.</returns>
		bool IsWound() const { return m_IsWound; }

		/// <summary>
		/// Sets whether or not this Attachable is a wound, to be done by its parent MOSR.
		/// </summary>
		/// <param name="isWound">Whether or not this Attachable should be a wound.</param>
		void SetIsWound(bool isWound) { m_IsWound = isWound; }
#pragma endregion

#pragma region Joint Getters and Setters
		/// <summary>
		/// Gets the amount of impulse force the joint of this Attachable can handle before breaking.
		/// </summary>
		/// <returns>A float with the max tolerated impulse force in kg * m/s.</returns>
		float GetJointStrength() const { return m_JointStrength; }

		/// <summary>
		/// Sets the amount of impulse force the joint of this Attachable can handle before breaking.
		/// </summary>
		/// <param name="jointStrength">A float describing the max tolerated impulse force in Newtons (kg * m/s).</param>
		void SetJointStrength(float jointStrength) { m_JointStrength = jointStrength; }

		/// <summary>
		/// Gets the stiffness scalar of the joint of this Attachable, normalized between 0 and 1.0.
		/// 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		/// </summary>
		/// <returns>The normalized stiffness scalar of this Attachable's joint.</returns>
		float GetJointStiffness() const { return m_JointStiffness; }

		/// <summary>
		/// Sets the stiffness scalar of the joint of this Attachable, normalized between 0 and 1.0.
		/// 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		/// </summary>
		/// <param name="jointStiffness">A float describing the normalized stiffness scalar of this Attachable's joint. It will automatically be limited between 0 and 1.0.</param>
		void SetJointStiffness(float jointStiffness) { m_JointStiffness = Limit(jointStiffness, 1.0F, 0.0F); }

		/// <summary>
		/// Gets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// </summary>
		/// <returns>A const reference Vector describing the offset of the joint relative to this Attachable's origin/center of mass position.</returns>
		const Vector & GetJointOffset() const { return m_JointOffset; }

		/// <summary>
		/// Sets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// </summary>
		/// <param name="newJointOffset">A Vector describing the offset of the joint relative to the this Attachable's origin/center of mass position.</param>
		void SetJointOffset(const Vector &newJointOffset) { m_JointOffset = newJointOffset; }
#pragma endregion

#pragma region Force Transferral
		/// <summary>
		/// Bundles up all the accumulated forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// If the accumulated forces exceed the joint strength of this Attachable, the jointForces Vector will be filled to the limit and false will be returned.
		/// Additionally, in this case, the Attachable will remove itself from its parent.
		/// </summary>
		/// <param name="jointForces">A vector that will have the forces affecting the joint ADDED to it.</param>
		/// <returns>False if the Attachable has no parent or its accumulated forces are greater than its joint strength, otherwise true.</returns>
		bool TransferJointForces(Vector &jointForces);

		/// <summary>
		/// Bundles up all the accumulated impulse forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// If the accumulated impulse forces exceed the joint strength or gib impulse limit of this Attachable, the jointImpulses Vector will be filled up to that limit and false will be returned.
		/// Additionally, in this case, the Attachable will remove itself from its parent and gib itself if appropriate.
		/// </summary>
		/// <param name="jointImpulses">A vector that will have the impulse forces affecting the joint ADDED to it.</param>
		/// <param name="jointStiffnessValueToUse">An optional override for the Attachable's joint stiffness for this function call. Primarily used to allow subclasses to perform special behavior.</param>
		/// <param name="jointStrengthValueToUse">An optional override for the Attachable's joint strength for this function call. Primarily used to allow subclasses to perform special behavior.</param>
		/// <param name="gibImpulseLimitValueToUse">An optional override for the Attachable's gib impulse limit for this function call. Primarily used to allow subclasses to perform special behavior.</param>
		/// <returns>False if the Attachable has no parent or its accumulated forces are greater than its joint strength or gib impulse limit, otherwise true.</returns>
		virtual bool TransferJointImpulses(Vector &jointImpulses, float jointStiffnessValueToUse = -1, float jointStrengthValueToUse = -1, float gibImpulseLimitValueToUse = -1);
#pragma endregion

#pragma region Damage and Wound Management
		/// <summary>
		/// Adds the specified number of damage points to this attachable.
		/// </summary>
		/// <param name="damageAmount">The amount of damage to add.</param>
		void AddDamage(float damageAmount) { m_DamageCount += damageAmount; }

		/// <summary>
		/// Calculates the amount of damage this Attachable has sustained since the last time this method was called and returns it, modified by the Attachable's damage multiplier.
		/// This should normally be called AFTER updating this Attachable in order to get the correct damage for a given frame.
		/// </summary>
		/// <returns>A float with the damage accumulated, multiplied by the Attachable's damage multiplier, since the last time this method was called.</returns>
		float CollectDamage();

		/// <summary>
		/// Gets the AEmitter that represents the wound added to this Attachable when it gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>A const pointer to the break wound AEmitter.</returns>
		const AEmitter * GetBreakWound() const { return m_BreakWound; }

		/// <summary>
		/// Sets the AEmitter that represents the wound added to this Attachable when it gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <param name="breakWound">The AEmitter to use for this Attachable's breakwound.</param>
		void SetBreakWound(AEmitter *breakWound) { m_BreakWound = breakWound; }

		/// <summary>
		/// Gets the AEmitter that represents the wound added to this Attachable's parent when this Attachable gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>A const pointer to the parent break wound AEmitter.</returns>
		const AEmitter * GetParentBreakWound() const { return m_ParentBreakWound; }

		/// <summary>
		/// Sets the AEmitter that represents the wound added to this Attachable's parent when this Attachable gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <param name="breakWound">The AEmitter to use for the parent's breakwound.</param>
		void SetParentBreakWound(AEmitter *breakWound) { m_ParentBreakWound = breakWound; }
#pragma endregion

#pragma region Inherited Value Getters and Setters
		/// <summary>
		/// Gets whether or not this Attachable inherits its parent's HFlipped value, i.e. whether it has its HFlipped value reset to match/reverse its parent's every frame, if attached.
		/// -1 (or technically any value that's not 0 or 1) means reversed inheritance (i.e. if the parent's HFlipped value is true, this Attachable's HFlipped value will be false), 0 means no inheritance, 1 means normal inheritance.
		/// </summary>
		/// <returns>Whether or not this Attachable inherits its parent's HFlipped value.</returns>
		int InheritsHFlipped() const { return m_InheritsHFlipped; }

		/// <summary>
		/// Sets whether or not this Attachable inherits its parent's HFlipped value, i.e. whether it has its HFlipped value reset to match/reverse its parent's every frame, if attached.
		/// -1 (or technically any value that's not 0 or 1) means reversed inheritance (i.e. if the parent's HFlipped value is true, this Attachable's HFlipped value will be false), 0 means no inheritance, 1 means normal inheritance.
		/// </summary>
		/// <param name="inheritsRotAngle">Whether or not to inherit its parent's HFlipped value.</param>
		void SetInheritsHFlipped(int inheritsHFlipped) { m_InheritsHFlipped = inheritsHFlipped; }

		/// <summary>
		/// Gets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// </summary>
		/// <returns>Whether or not this Attachable inherits its parent's RotAngle.</returns>
		bool InheritsRotAngle() const { return m_InheritsRotAngle; }

		/// <summary>
		/// Sets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// </summary>
		/// <param name="inheritsRotAngle">Whether or not to inherit its parent's RotAngle.</param>
		void SetInheritsRotAngle(bool inheritsRotAngle) { m_InheritsRotAngle = inheritsRotAngle; }

		/// <summary>
		/// Gets the offset of this Attachable's rotation angle from its parent. Only actually applied if the Attachable is set to inherit its parent's rotation angle.
		/// </summary>
		/// <returns>This Attachable's inherited rotation angle offset.</returns>
		float GetInheritedRotAngleOffset() const { return m_InheritedRotAngleOffset; }

		/// <summary>
		/// Sets the offset of this Attachable's rotation angle from its parent. Only actually applied if the Attachable is set to inherit its parent's rotation angle.
		/// </summary>
		/// <param name="inheritedRotAngleOffset">Thee new rotation angle offset for this Attachable.</param>
		void SetInheritedRotAngleOffset(float inheritedRotAngleOffset) { m_InheritedRotAngleOffset = inheritedRotAngleOffset; }

		/// <summary>
		/// Gets whether or not this Attachable inherits its Frame from its parent, if attached.
		/// </summary>
		/// <returns>Whether or not this Attachable inherits its parent's Frame.</returns>
		bool InheritsFrame() const { return m_InheritsFrame; }

		/// <summary>
		/// Sets whether or not this Attachable inherits its Frame from its parent, if attached.
		/// </summary>
		/// <param name="inheritsFrame">Whether or not to inherit its parent's Frame.</param>
		void SetInheritsFrame(bool inheritsFrame) { m_InheritsFrame = inheritsFrame; }
#pragma endregion

#pragma region Collision Management
		/// <summary>
		/// Gets the subgroup ID of this' Atoms.
		/// </summary>
		/// <returns>The subgroup ID of this' Atoms.</returns>
		long GetAtomSubgroupID() const { return m_AtomSubgroupID; }

		/// <summary>
		/// Sets the subgroup ID of this' Atoms
		/// </summary>
		/// <param name="newID">A long describing the new subgroup id of this' Atoms.</param>
		void SetAtomSubgroupID(long subgroupID = 0) { m_AtomSubgroupID = subgroupID; }

		/// <summary>
		/// Gets whether this Attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// Attachables with Attachable parents that don't collide with terrain will not collide with terrain. This chains up to the root parent.
		/// </summary>
		/// <return>If true, terrain collisions while attached are enabled and atoms are present in parent AtomGroup.</return>
		bool GetCollidesWithTerrainWhileAttached() const { return m_CollidesWithTerrainWhileAttached; }

		/// <summary>
		/// Sets whether this Attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// </summary>
		/// <param name="collidesWithTerrainWhileAttached">Whether this attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.</param>
		void SetCollidesWithTerrainWhileAttached(bool collidesWithTerrainWhileAttached);

		/// <summary>
		/// Gets whether this Attachable is currently able to collide with terrain, taking into account its terrain collision settings and those of its parent and so on.
		/// </summary>
		/// <returns>Whether this Attachable is currently able to collide with terrain, taking into account its terrain collision settings and those of its parent and so on.</returns>
		bool CanCollideWithTerrain() const;
#pragma endregion

#pragma region Override Methods
		/// <summary>
		/// Determines whether a particle which has hit this MO will penetrate, and if so, whether it gets lodged or exits on the other side of this MO.
		/// Appropriate effects will be determined and applied ONLY IF there was penetration! If not, nothing will be affected.
		/// </summary>
		/// <param name="hitData">The HitData describing the collision in detail, the impulses have to have been filled out!</param>
		/// <returns>
		/// Whether the particle managed to penetrate into this MO or not.
		/// If something other than an MOPixel or MOSParticle is being passed in as the hitor, false will trivially be returned here.
		/// </returns>
		bool ParticlePenetration(HitData &hitData) override;

		/// <summary>
		/// Destroys this Attachable and creates its specified Gibs in its place with appropriate velocities.
		/// Any Attachables are removed and also given appropriate velocities.
		/// </summary>
		/// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
		/// <param name="movableObjectToIgnore">A pointer to an MO which the Gibs and Attachables should not be colliding with.</param>
		void GibThis(const Vector &impactImpulse = Vector(), MovableObject *movableObjectToIgnore = nullptr) override;

		/// <summary>
		/// Checks if the given Attachable should affect radius, and handles it if it should.
		/// </summary>
		/// <param name="attachable">The Attachable to check.</param>
		/// <returns>Whether the radius affecting Attachable changed as a result of this call.</returns>
		bool HandlePotentialRadiusAffectingAttachable(const Attachable *attachable) override;

		/// <summary>
		/// Updates this Attachable. Supposed to be done every frame.
		///	NOTE - Attachable subclasses that do things before calling Attachable::Update should make sure to call Attachable::PreUpdate.
		/// </summary>
		void Update() override;
#pragma endregion

		/// <summary>
		/// Pre-update method that should be run by all Attachable sub-classes that do things before calling Attachable::Update.
		/// </summary>
		void PreUpdate();

#pragma region Override Methods for Handling Mass
		/// <summary>
		/// Sets the mass of this Attachable.
		/// </summary>
		/// <param name="newMass">A float specifying the new mass value in Kilograms (kg).</param>
		void SetMass(const float newMass) final;

		/// <summary>
		/// Updates the total mass of Attachables and wounds for this Attachable, intended to be used when Attachables' masses get modified. Simply subtracts the old mass and adds the new one.
		/// </summary>
		/// <param name="oldAttachableOrWoundMass">The mass the Attachable or wound had before its mass was modified.</param>
		/// <param name="newAttachableOrWoundMass">The up-to-date mass of the Attachable or wound after its mass was modified.</param>
		void UpdateAttachableAndWoundMass(float oldAttachableOrWoundMass, float newAttachableOrWoundMass) final;

		/// <summary>
		/// Adds the passed in Attachable the list of Attachables and sets its parent to this Attachable.
		/// </summary>
		/// <param name="attachable">The Attachable to add.</param>
		void AddAttachable(Attachable *attachable) final { MOSRotating::AddAttachable(attachable); }

		/// <summary>
		/// Adds the passed in Attachable the list of Attachables, changes its parent offset to the passed in Vector, and sets its parent to this Attachable.
		/// </summary>
		/// <param name="attachable">The Attachable to add.</param>
		/// <param name="parentOffsetToSet">The Vector to set as the Attachable's parent offset.</param>
		void AddAttachable(Attachable *attachable, const Vector &parentOffsetToSet) final;

		/// <summary>
		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// </summary>
		/// <param name="attachableUniqueID">The UniqueID of the Attachable to remove.</param>
		/// <returns>A pointer to the removed Attachable. Ownership IS transferred!</returns>
		Attachable * RemoveAttachable(long attachableUniqueID) final { return MOSRotating::RemoveAttachable(attachableUniqueID); }

		/// <summary>
		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// </summary>
		/// <param name="attachableUniqueID">The UniqueID of the Attachable to remove.</param>
		/// <param name="addToMovableMan">Whether or not to add the Attachable to MovableMan once it has been removed.</param>
		/// <param name="addBreakWounds">Whether or not to add break wounds to the removed Attachable and this Attachable.</param>
		/// <returns>A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!</returns>
		Attachable * RemoveAttachable(long attachableUniqueID, bool addToMovableMan, bool addBreakWounds) final { return MOSRotating::RemoveAttachable(attachableUniqueID, addToMovableMan, addBreakWounds); }

		/// <summary>
		/// Removes the passed in Attachable and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// </summary>
		/// <param name="attachable">The Attachable to remove.</param>
		/// <returns>A pointer to the removed Attachable. Ownership IS transferred!</returns>
		Attachable * RemoveAttachable(Attachable *attachable) final { return MOSRotating::RemoveAttachable(attachable); }

		/// <summary>
		/// Removes the passed in Attachable and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// </summary>
		/// <param name="attachable">The Attachable to remove.</param>
		/// <param name="addToMovableMan">Whether or not to add the Attachable to MovableMan once it has been removed.</param>
		/// <param name="addBreakWounds">Whether or not to add break wounds to the removed Attachable and this Attachable.</param>
		/// <returns>A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!</returns>
		Attachable * RemoveAttachable(Attachable *attachable, bool addToMovableMan, bool addBreakWounds) final;

		/// <summary>
		/// Adds the passed in wound AEmitter to the list of wounds and changes its parent offset to the passed in Vector.
		/// </summary>
		/// <param name="woundToAdd">The wound AEmitter to add.</param>
		/// <param name="parentOffsetToSet">The vector to set as the wound AEmitter's parent offset.</param>
		/// <param name="checkGibWoundLimit">Whether to gib this Attachable if adding this wound raises its wound count past its gib wound limit. Defaults to true.</param>
		void AddWound(AEmitter *woundToAdd, const Vector &parentOffsetToSet, bool checkGibWoundLimit = true) final;

		/// <summary>
		/// Removes the specified number of wounds from this Attachable, and returns damage caused by these removed wounds.
		/// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
		/// </summary>
		/// <param name="numberOfWoundsToRemove">The number of wounds that should be removed.</param>
		/// <returns>The amount of damage caused by these wounds, taking damage multipliers into account.</returns>
		float RemoveWounds(int numberOfWoundsToRemove) final { return MOSRotating::RemoveWounds(numberOfWoundsToRemove); }

		/// <summary>
		/// Removes the specified number of wounds from this Attachable, and returns damage caused by these removed wounds.
		/// Optionally removes wounds from Attachables (and their Attachables, etc.) that match the conditions set by the provided inclusion parameters.
		/// </summary>
		/// <param name="numberOfWoundsToRemove">The number of wounds that should be removed.</param>
		/// <param name="includePositiveDamageAttachables">Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this Attachable) when wounded.</param>
		/// <param name="includeNegativeDamageAttachables">Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this Attachable) when wounded.</param>
		/// <param name="includeNoDamageAttachables">Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this Attachable) when wounded.</param>
		/// <returns>The amount of damage caused by these wounds, taking damage multipliers into account.</returns>
		float RemoveWounds(int numberOfWoundsToRemove, bool includeAttachablesWithAPositiveDamageMultiplier, bool includeAttachablesWithANegativeDamageMultiplier, bool includeAttachablesWithNoDamageMultiplier) override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		MOSRotating *m_Parent; //!< Pointer to the MOSRotating this attachable is attached to.
		Vector m_ParentOffset; //!< The offset from the parent's Pos to the joint point this Attachable is attached with.
		bool m_DrawAfterParent; //!< Whether to draw this Attachable after (in front of) or before (behind) the parent.
		bool m_DrawnNormallyByParent; //!< Whether this Attachable will be drawn normally when attached, or will require special handling by some non-MOSR parent type.
		bool m_DeleteWhenRemovedFromParent; //!< Whether this Attachable should be deleted when it's removed from its parent.
		bool m_ApplyTransferredForcesAtOffset; //!< Whether forces transferred from this Attachable should be applied at the rotated parent offset (which will produce torque), or directly at the parent's position. Mostly useful to make jetpacks and similar emitters viable.
		
		float m_GibWithParentChance; //!< The percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		float m_ParentGibBlastStrengthMultiplier; //!< The multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs.
		
		//TODO This is a stopgap for a dedicated Wound class, that would be helpful to simplify things like this and default damage multiplier handling.
		bool m_IsWound; //!< Whether or not this Attachable has been added as a wound. Only set and applied for Attachables with parents.

		float m_JointStrength; //!< The amount of impulse force needed on this to detach it from the host Actor, in kg * m/s. A value of 0 means the join is infinitely strong and will never break naturally.
		float m_JointStiffness; //!< The normalized joint stiffness scalar. 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		Vector m_JointOffset; //!< The offset to the joint (the point around which this Attachable and its parent hinge) from its center of mass/origin.
		Vector m_JointPos; //!< The absolute position of the joint that the parent sets upon Update() if this Attachable is attached to it.

		float m_DamageCount; //!< The number of damage points that this Attachable has accumulated since the last time CollectDamage() was called.
		const AEmitter *m_BreakWound; //!< The wound this Attachable will receive when it breaks from its parent.
		const AEmitter *m_ParentBreakWound; //!< The wound this Attachable's parent will receive when the Attachable breaks from its parent.

		int m_InheritsHFlipped; //!< Whether this Attachable should inherit its parent's HFlipped. Defaults to 1 (normal inheritance).
		bool m_InheritsRotAngle; //!< Whether this Attachable should inherit its parent's RotAngle. Defaults to true.
		float m_InheritedRotAngleOffset; //!< The offset by which this Attachable should be rotated when it's set to inherit its parent's rotation angle. Defaults to 0.
		bool m_InheritsFrame; //!< Whether this Attachable should inherit its parent's Frame. Defaults to false.

		long m_AtomSubgroupID; //!< The Atom IDs this' atoms will have when attached and added to a parent's AtomGroup.
		bool m_CollidesWithTerrainWhileAttached; //!< Whether this attachable currently has terrain collisions enabled while it's attached to a parent.

		Vector m_PrevParentOffset; //!< The previous frame's parent offset.
		Vector m_PrevJointOffset; //!< The previous frame's joint offset.
		float m_PrevRotAngleOffset; //!< The previous frame's difference between this Attachable's RotAngle and it's root parent's RotAngle.
		bool m_PreUpdateHasRunThisFrame; //!< Whether or not PreUpdate has run this frame. PreUpdate, like Update, should only run once per frame.

		/// <summary>
		/// Sets this Attachable's parent MOSRotating, and also sets its Team based on its parent and, if the Attachable is set to collide, adds/removes Atoms to its new/old parent.
		/// </summary>
		/// <param name="newParent">A pointer to the MOSRotating to set as the new parent. Ownership is NOT transferred!</param>
		virtual void SetParent(MOSRotating *newParent);

	private:

		/// <summary>
		/// Updates the position of this Attachable based on its parent offset and joint offset. Used during update and when something sets these offsets through setters.
		/// </summary>
		void UpdatePositionAndJointPositionBasedOnOffsets();

		/// <summary>
		/// Turns on/off this Attachable's terrain collisions while it is attached by adding/removing its Atoms to/from its root parent's AtomGroup.
		/// </summary>
		/// <param name="addAtoms">Whether to add this Attachable's Atoms to the root parent's AtomGroup or remove them.</param>
		/// <param name="propagateToChildAttachables">Whether this Atom addition or removal should be propagated to any child Attachables (as appropriate).</param>
		void AddOrRemoveAtomsFromRootParentAtomGroup(bool addAtoms, bool propagateToChildAttachables);

		/// <summary>
		/// Clears all the member variables of this Attachable, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Attachable(const Attachable &reference) = delete;
		Attachable & operator=(const Attachable &rhs) = delete;
	};
}
#endif