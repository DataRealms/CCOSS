#ifndef _RTEATTACHABLE_
#define _RTEATTACHABLE_

#include "MOSRotating.h"

namespace RTE {

	class AEmitter;

	/// <summary>
	/// An articulated, detachable part of an Actor's body.
	/// </summary>
	class Attachable : public MOSRotating {

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
		/// Creates a Attachable to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Attachable to deep copy.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Attachable &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Attachable object before deletion from system memory.
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
		/// Gets the MO which is the parent of this Attachable.
		/// </summary>
		/// <returns>A pointer to the parent of this Attachable.</returns>
		MovableObject *GetParent() override { return m_Parent; }

		/// <summary>
		/// Gets the MO which is the parent of this Attachable. 
		/// </summary>
		/// <returns>A pointer to the parent of this Attachable.</returns>
		const MovableObject *GetParent() const override { return m_Parent; }

		/// <summary>
		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// </summary>
		/// <returns>A pointer to the highest root parent of this Attachable.</returns>
		MovableObject *GetRootParent() override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// <summary>
		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// </summary>
		/// <returns>A pointer to the highest root parent of this Attachable.</returns>
		const MovableObject *GetRootParent() const override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// <summary>
		/// Gets the stored offset between this Attachable's parent's position and the joint position. This should be maintained by the parent.
		/// </summary>
		/// <returns>A const reference Vector describing the offset from the parent's position to the joint position.</returns>
		const Vector &GetParentOffset() const { return m_ParentOffset; }

		/// <summary>
		/// Sets the stored offset between this Attachable's parent's Pos and the joint position. This should be maintained by the parent.
		/// </summary>
		/// <param name="newParOff">A const reference to the new parent offset.</param>
		void SetParentOffset(const Vector &newParentOffset) { m_ParentOffset = newParentOffset; }

		/// <summary>
		/// Indicates whether this Attachable is to be drawn after (in front of) or before (behind) the parent.
		/// </summary>
		/// <returns>Whether it's to be drawn after parent or not.</returns>
		virtual bool IsDrawnAfterParent() const { return m_DrawAfterParent; }

		/// <summary>
		/// Gets whether this attachable should be deleted automatically when its parent is being deleted or not.
		/// </summary>
		/// <returns>Whether this attachable is marked to be deleted along with it's parent or not.</returns>
		bool ShouldDeleteWithParent() const { return m_DeleteWithParent; }
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
		void SetJointStiffness(float jointStiffness) { m_JointStiffness = Limit(jointStiffness, 1, 0); }

		/// <summary>
		/// Gets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// </summary>
		/// <returns>A const reference Vector describing the offset of the joint relative to this Attachable's origin/center of mass position.</returns>
		const Vector &GetJointOffset() const { return m_JointOffset; }

		/// <summary>
		/// Sets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// </summary>
		/// <param name="offset">A Vector describing the offset of the joint relative to the this Attachable's origin/center of mass position.</param>
		void SetJointOffset(Vector offset) { m_JointOffset = offset; }

		/// <summary>
		/// Sets the position of this Attachable by defining where the joint is.
		/// Upon Update(), this will be translated into what the actual position of the Attachable origin/center of mass is, depending on its set rotational angle and joint offset.
		/// </summary>
		/// <param name="newJointPos">A const reference to the new joint position to set the position with.</param>
		void SetJointPos(const Vector &jointPos) { m_JointPos = jointPos; }
#pragma endregion

#pragma region Force Managment
		/// <summary>
		/// Indicates whether this Attachable only cares about linear forces that it creates through emissions, ie no torquing due to the parent offset.
		/// </summary>
		/// <returns>Whether this is only using linear forces or not.</returns>
		bool GetOnlyLinearForces() const { return m_OnlyLinearForces; }

		/// <summary>
		/// Sets whether this AEmitter should only care about linear forces that it creates through emissions, ie no torquing due to the parent offset.
		/// </summary>
		/// <param name="onlyLinearForces">Whether to only use linear forces or not.</param>
		void SetOnlyLinearForces(bool onlyLinearForces) { m_OnlyLinearForces = onlyLinearForces; }

		/// <summary>
		/// Bundles up all the accumulated forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// </summary>
		/// <param name="jointForces">A vector that will have the forces affecting the joint ADDED to it.</param>
		/// <returns>
		/// If the accumulated forces exceed the strength of the joint, the attachable will only fill out the forces up to the strength threshold and then detach itself and return false.
		/// The parent should react accordingly in that case, by nulling out pointers to the Attachable.
		/// </returns>
		bool TransferJointForces(Vector &jointForces);

		/// <summary>
		/// Bundles up all the accumulated impulse forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// </summary>
		/// <param name="jointImpulses">A vector that will have with the impulse forces affecting the joint ADDED to it.</param>
		/// <returns>
		/// If the accumulated impulse forces exceed the strength of the joint, the attachable will only fill out the forces up to the strength threshold and then detach itself and return false.
		/// The parent should react accordingly in that case, by nulling out pointers to the Attachable.
		/// </returns>
		bool TransferJointImpulses(Vector &jointImpulses);
#pragma endregion

#pragma region Damage and Wound Management
		/// <summary>
		/// Adds the specified number of damage points to this attachable.
		/// </summary>
		/// <param name="damageAmount">The amount of damage to add.</param>
		void AddDamage(float damageAmount) { m_DamageCount += damageAmount; }

		/// <summary>
		/// Gets the amount of damage points this Attachable has sustained and should cause its parent, then resets the damage count.
		/// This should normally be called AFTER Update() to get the correct damage for a given frame.
		/// </summary>
		/// <returns>A float with the damage points accumulated since the last time this method was called.</returns>
		float CollectDamage();

		/// <summary>
		/// Gets the AEmitter that represents the wound created when this Attachable gets detached from its parent, if it's set as Vital. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>A const pointer to the break wound AEmitter.</returns>
		const AEmitter *GetBreakWound() const { return m_BreakWound; }

		/// <summary>
		/// Sets the AEmitter that represents the wound created when this Attachable gets detached from its parent, if it's set as Vital. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <param name="breakWound"></param>
		void SetBreakWound(const AEmitter *breakWound) { m_BreakWound = breakWound; } //TODO I added this for consistency but do we want it? Maybe should have a string version that does the presetman lookup, cause we need to have a working pointer to the breakwound.
#pragma endregion

#pragma region Rotation Getters and Setters
		/// <summary>
		/// Gets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// </summary>
		/// <returns>Whether or not this Attachable inherits its RotAngle from its parent.</returns>
		bool InheritsRotAngle() const { return m_InheritsRotAngle; }

		/// <summary>
		/// Sets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// </summary>
		/// <param name="inheritsRotAngle">Whether or not to inherit its parent's RotAngle.</param>
		void SetInheritsRotAngle(bool inheritsRotAngle) { m_InheritsRotAngle = inheritsRotAngle; }

		/// <summary>
		/// Gets the target rotation that this Attachable should be striving to match its real rotation with, if it is attached.
		/// The joint stiffness will determine how strong the scalar spring is between the current rotation and the target set here.
		/// </summary>
		/// <returns>A float for the current target angle in radians.</returns>
		float GetRotTarget() const { return m_RotTarget.GetRadAngle(); }

		/// <summary>
		/// Sets the target rotation angle in radians that this Attachable should be striving to match its real rotation with, if it is attached.
		/// The joint stiffness will determine how strong the scalar spring is between the current rotation and the target set here.
		/// </summary>
		/// <param name="rotRadAngle">A float describing the target angle in radians.</param>
		void SetRotTarget(float rotRadAngle) { m_RotTarget.SetRadAngle(rotRadAngle); }
#pragma endregion

#pragma region Collision Management
		/// <summary>
		/// Gets the subgroup ID of this' Atoms.
		/// </summary>
		/// <returns>The subgroup ID of this' Atoms.</returns>
		long int GetAtomSubgroupID() const { return m_AtomSubgroupID; }

		/// <summary>
		/// Sets the subgroup ID of this' Atoms
		/// </summary>
		/// <param name="newID">A long int describing the new subgroup id of this' Atoms.</param>
		void SetAtomSubgroupID(long int subgroupID = 0) { m_AtomSubgroupID = subgroupID; }

		/// <summary>
		/// Whether this attachable is capable of having terrain collisions enabled/disabled when attached to a parent.
		/// </summary>
		/// <return>If true, can have terrain collisions enabled/disabled when attached.</return>
		virtual bool CanCollideWithTerrainWhenAttached() const { return m_CanCollideWithTerrainWhenAttached; }

		/// <summary>
		/// Sets whether this attachable is capable of having terrain collisions enabled/disabled when attached to a parent.
		/// </summary>
		/// <param name="canCollide">Whether this attachable can have terrain collisions enabled/disabled when attached.</param>
		virtual void SetCanCollideWithTerrainWhenAttached(bool canCollide) { m_CanCollideWithTerrainWhenAttached = canCollide; }

		/// <summary>
		/// Whether this attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// </summary>
		/// <return>If true, terrain collisions while attached are enabled and atoms are present in parent AtomGroup.</return>
		virtual bool IsCollidingWithTerrainWhileAttached() const { return m_IsCollidingWithTerrainWhileAttached; }

		/// <summary>
		/// Sets whether this attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// </summary>
		/// <param name="collide">Whether this attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.</param>
		virtual void SetIsCollidingWithTerrainWhileAttached(bool isColliding) { m_IsCollidingWithTerrainWhileAttached = isColliding; }

		/// <summary>
		/// Turns on/off this Attachable's terrain collisions while it is attached by adding/removing its atoms to/from its parent AtomGroup.
		/// </summary>
		/// <param name="enable">Whether or not to add or remove this Attachable's atoms to/from the parent's AtomGroup.</param>
		virtual void EnableTerrainCollisions(bool enable);
#pragma endregion

#pragma region Attachment Managment
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
		/// Attaches this Attachable to a new parent MOSRotating.
		/// </summary>
		/// <param name="newParent">A pointer to the MOSRotating to attach to. Ownership is NOT transferred!</param>
		virtual void Attach(MOSRotating *newParent);

		/// <summary>
		/// Attaches this Attachable to a new parent MOSRotating, with the specified parent offset.
		/// </summary>
		/// <param name="newParent">A pointer to the MOSRotating to attach to. Ownership is NOT transferred!</param>
		/// <param name="parentOffset">The offset from the parent's Pos to the joint Pos.</param>
		void Attach(MOSRotating *newParent, const Vector &parentOffset) { Attach(newParent); m_ParentOffset = parentOffset; }

		/// <summary>
		/// Detaches this Attachable from its host MOSprite
		/// </summary>
		virtual void Detach();
#pragma endregion

#pragma region Override Methods
		/// <summary>
		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// </summary>
		/// <param name="hitData">Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.</param>
		/// <returns>Whether the collision has been deemed valid. If false, then disregard any impulses in the Hitdata.</returns>
		bool CollideAtPoint(HitData &hitData) override;

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
		/// Gibs this, effectively destroying it and creating gibs in its place.
		/// </summary>
		/// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
		/// <param name="internalBlast">The internal blast impulse which will push the gibs away from the center.</param>
		/// <param name="MOToIgnore">A pointer to an MO which the gibs shuold not be colliding with!</param>
		void GibThis(Vector impactImpulse = Vector(), float internalBlast = 10, MovableObject *MOToIgnore = 0) override;

		/// <summary>
		/// Updates this Attachable. Supposed to be done every frame.
		/// </summary>
		void Update() override;
#pragma endregion
		//TODO This is crap but MOSR needs to account for damage multiplier
		virtual int RemoveWounds(int amount) { return MOSRotating::RemoveWounds(amount) * m_DamageMultiplier; }

	private:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		MOSRotating *m_Parent; //!< Pointer to the MOSRotating this attachable is attached to.
		Vector m_ParentOffset; //!< The offset from the parent's Pos to the joint point this Attachable is attached with.
		bool m_DrawAfterParent; //!< Whether to draw this Attachable after (in front of) or before (behind) the parent.
		bool m_DeleteWithParent; //!< Whether this attachable should be deleted when its parent is set ToDelete.
		
		float m_JointStrength; //!< The amount of impulse force needed on this to deatch it from the host Actor, in kg * m/s.
		float m_JointStiffness; //!< The normalized joint stiffness scalar. 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		Vector m_JointOffset; //!< The offset to the joint (the point around which this Attachable and its parent hinge) from its center of mass/origin.
		Vector m_JointPos; //!< The absolute position of the joint that the parent sets upon Update() if this Attachable is attached to it.

		bool m_OnlyLinearForces; //!< Whether to only record linear forces, ie no torquing due to the parent offset.

		float m_DamageCount; //!< The number of damage points that this Attachable has accumulated since the last time CollectDamage() was called.
		const AEmitter *m_BreakWound; //!< The wound this Attachable will cause when it breaks from its parent.

		bool m_InheritsRotAngle; //!< Whether this Attachable should inherit its parent's RotAngle.
		Matrix m_RotTarget; //!< The desired rotation of this Attachable, the angle it's trying to achieve through angle springs.

		long int m_AtomSubgroupID; //!< The Atom IDs this' atoms will have when attached and added to a parent's AtomGroup.
		bool m_CanCollideWithTerrainWhenAttached; //!< Whether this attachable is capable of having terrain collisions enabled/disabled when attached to a parent.
		bool m_IsCollidingWithTerrainWhileAttached; //!< Whether this attachable currently has terrain collisions enabled while it's attached to a parent.

		/// <summary>
		/// Clears all the member variables of this Attachable, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Attachable(const Attachable &reference) {}
		Attachable &operator=(const Attachable &rhs) {}
	};

}
#endif