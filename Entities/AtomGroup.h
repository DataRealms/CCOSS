#ifndef _RTEATOMGROUP_
#define _RTEATOMGROUP_

#include "Atom.h"

namespace RTE {

	class MOSRotating;
	class LimbPath;

	/// <summary>
	/// A group of Atoms that move and interact with the terrain in unison.
	/// </summary>
	class AtomGroup : public Entity {

	public:

		EntityAllocation(AtomGroup);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an AtomGroup object in system memory. Create() should be called before using the object.
		/// </summary>
		AtomGroup() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate an AtomGroup object identical to an already existing one.
		/// </summary>
		/// <param name="reference">An AtomGroup object which is passed in by reference.</param>
		AtomGroup(const AtomGroup &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Makes the AtomGroup object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates an AtomGroup to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the AtomGroup to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const AtomGroup &reference) { return Create(reference, false); }

		/// <summary>
		/// Creates an AtomGroup to be identical to another, by deep copy, with the option to only copy Atoms that belong to the reference AtomGroup's owner thereby excluding any Atom subgroups.
		/// </summary>
		/// <param name="reference">A reference to the AtomGroup to deep copy.</param>
		/// <param name="onlyCopyOwnerAtoms">Whether or not to only copy Atoms that belong to the reference AtomGroup's owner directly.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const AtomGroup &reference, bool onlyCopyOwnerAtoms);

		/// <summary>
		/// Creates an AtomGroup after the silhouette shape of a passed in MOSRotating by dotting the outline of the sprite with Atoms. 
		/// The passed in MOSRotating will also be made the owner of this AtomGroup! Ownership of the MOSRotating is NOT transferred!
		/// </summary>
		/// <param name="ownerMOSRotating">A pointer to a MOSRotating whose outline will be approximated by Atoms of this AtomGroup, and that will be set as the owner of this AtomGroup.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(MOSRotating *ownerMOSRotating) { return Create(ownerMOSRotating, m_Material, m_Resolution, m_Depth); }

		/// <summary>
		/// Creates an AtomGroup after the silhouette shape of a passed in MOSRotating by dotting the outline of the sprite with Atoms.
		/// The passed in MOSRotating will also be made the owner of this AtomGroup! Ownership of the MOSRotating is NOT transferred!
		/// </summary>
		/// <param name="ownerMOSRotating">A pointer to an MOSRotating whose outline will be approximated by Atoms of this AtomGroup, and that will be set as the owner of this AtomGroup.</param>
		/// <param name="material">The Material that the Atoms of this AtomGroup should be.</param>
		/// <param name="resolution">Resolution, or density of the Atoms in representing the MOSRotating's outline. Lower value equals higher density.</param>
		/// <param name="depth">The depth into the sprite that the Atoms should be placed.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(MOSRotating *ownerMOSRotating, Material const *material, int resolution = 1, int depth = 0);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an AtomGroup object before deletion from system memory.
		/// </summary>
		~AtomGroup() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the AtomGroup object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire AtomGroup, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current list of Atoms that make up the group.
		/// </summary>
		/// <returns>A const reference to the Atom list.</returns>
		const std::list<Atom *> & GetAtomList() const { return m_Atoms; }

		/// <summary>
		/// Gets the current number of Atoms that make up the group.
		/// </summary>
		/// <returns>The number of Atoms that make up the group.</returns>
		int GetAtomCount() const { return m_Atoms.size(); }

		/// <summary>
		/// Gets max radius of the AtomGroup through the longest magnitude of all the Atom's offsets.
		/// </summary>
		/// <returns>The largest magnitude of Atom's offsets, in pixels.</returns>
		float CalculateMaxRadius() const;

		/// <summary>
		/// Gets the current owner MOSRotating of this AtomGroup.
		/// </summary>
		/// <returns>A pointer to the owner.</returns>
		MOSRotating * GetOwner() const { return m_OwnerMOSR; }

		/// <summary>
		/// Sets the current owner MOSRotating of this AtomGroup.
		/// </summary>
		/// <param name="newOwner">A pointer to the new owner. Ownership is NOT transferred!</param>
		void SetOwner(MOSRotating *newOwner);

		/// <summary>
		/// Gets the Material of this AtomGroup.
		/// </summary>
		/// <returns>A const pointer to the Material.</returns>
		const Material * GetMaterial() const { return (m_Material) ? m_Material : g_SceneMan.GetMaterialFromID(g_MaterialAir); }

		/// <summary>
		/// Gets whether this AtomGroup's Atoms are to be automatically generated based on a bitmap, or manually specified.
		/// </summary>
		/// <returns>Whether this AtomGroup is auto generated from a bitmap or not.</returns>
		bool AutoGenerate() const { return m_AutoGenerate; }

		/// <summary>
		/// Gets the resolution (density of Atoms) of this AtomGroup. Higher values mean a less dense and less accurate physical representation of the owner MOSR's graphical representation.
		/// </summary>
		/// <returns>The resolution value of this AtomGroup. 0 means the Atoms in this AtomGroup were defined manually.</returns>
		int GetResolution() const { return m_Resolution; }

		/// <summary>
		/// Gets the depth Atoms in this AtomGroup are placed off the edge of the owning MOSR's graphical representation outline towards it's center.
		/// </summary>
		/// <returns>The depth, in pixels. If 0, Atoms are placed right on the edge of the MOSR outline.</returns>
		int GetDepth() const { return m_Depth; }

		/// <summary>
		/// Gets the offset of an Atom in this AtomGroup adjusted to the Owner MOSRotating horizontal flip and rotation.
		/// </summary>
		/// <param name="atom">The individual Atom to get the offset for.</param>
		/// <returns>The offset of an Atom in this AtomGroup adjusted to the Owner MOSRotating horizontal flip and rotation.</returns>
		Vector GetAdjustedAtomOffset(const Atom *atom) const;

		/// <summary>
		/// Gets the current position of this AtomGroup as a limb.
		/// </summary>
		/// <param name="hFlipped">Whether to adjust the position for horizontal flip or not.</param>
		/// <returns>The absolute limb position in the world.</returns>
		Vector GetLimbPos(bool hFlipped = false) const { return m_LimbPos.GetFloored() + m_JointOffset.GetXFlipped(hFlipped); }

		/// <summary>
		/// Sets the current position of this AtomGroup as a limb.
		/// </summary>
		/// <param name="newPos">The Vector with the new absolute position.</param>
		/// <param name="hFlipped">Whether to adjust the new position for horizontal flip or not.</param>
		void SetLimbPos(const Vector &newPos, bool hFlipped = false) { m_LimbPos = newPos - m_JointOffset.GetXFlipped(hFlipped); }

		/// <summary>
		/// Gets the current mass moment of inertia of this AtomGroup.
		/// </summary>
		/// <returns>A float with the moment of inertia, in Kg * meter^2.</returns>
		float GetMomentOfInertia();
#pragma endregion

#pragma region Atom Management
		/// <summary>
		/// Adds a new Atom into the internal list that makes up this AtomGroup. Ownership of the Atom IS transferred!
		/// Note, this resets the moment of inertia, which then has to be recalculated.
		/// </summary>
		/// <param name="newAtom">A pointer to an Atom that will pushed onto the end of the list. Ownership IS transferred!</param>
		/// <param name="subgroupID">The subgroup ID that the new Atom will have within the group.</param>
		void AddAtom(Atom *newAtom, long subgroupID = 0) { newAtom->SetSubID(subgroupID); m_Atoms.push_back(newAtom); m_MomentOfInertia = 0.0F; }

		/// <summary>
		/// Adds a list of new Atoms to the internal list that makes up this AtomGroup. Ownership of all Atoms in the list IS NOT transferred!
		/// </summary>
		/// <param name="atomList">A list of pointers to Atoms whose copies will be pushed onto the end of this AtomGroup's list. Ownership IS NOT transferred!</param>
		/// <param name="subgroupID">The desired subgroup ID for the Atoms being added.</param>
		/// <param name="offset">An offset that should be applied to all added Atoms.</param>
		/// <param name="offsetRotation">The rotation of the placed Atoms around the specified offset.</param>
		void AddAtoms(const std::list<Atom *> &atomList, long subgroupID = 0, const Vector &offset = Vector(), const Matrix &offsetRotation = Matrix());

		/// <summary>
		/// Removes all Atoms of a specific subgroup ID from this AtomGroup.
		/// </summary>
		/// <param name="removeID">The ID of the subgroup of Atoms to remove.</param>
		/// <returns>Whether any Atoms of that subgroup ID were found and removed.</returns>
		bool RemoveAtoms(long removeID);

		/// <summary>
		/// Removes all atoms in this AtomGroup, leaving it empty of Atoms.
		/// </summary>
		void RemoveAllAtoms() { m_Atoms.clear(); m_SubGroups.clear(); m_MomentOfInertia = 0.0F; m_StoredOwnerMass = 0.0F; }

		/// <summary>
		/// Gets whether the AtomGroup contains a subgroup with the given subgroupID.
		/// </summary>
		/// <param name="subgroupID">The subgroupID to check for.</param>
		/// <returns>Whether this AtomGroup contains a subgroup with the given subgroupID.</returns>
		bool ContainsSubGroup(long subgroupID) const { return m_SubGroups.count(subgroupID) != 0; }

		/// <summary>
		/// Updates the offsets of a subgroup of Atoms in this AtomGroup. This allows repositioning a subgroup to match the position and rotation of the graphical representation of it's owner MOSR.
		/// </summary>
		/// <param name="subgroupID">The desired subgroup ID of the Atoms to update offsets for.</param>
		/// <param name="newOffset">The change in offset for the Atoms of the specified subgroup.</param>
		/// <param name="newOffsetRotation">The rotation of the updated Atoms around the specified offset.</param>
		/// <returns>Whether any Atoms were found and updated for the specified subgroup.</returns>
		bool UpdateSubAtoms(long subgroupID = 0, const Vector &newOffset = Vector(), const Matrix& newOffsetRotation = Matrix());
#pragma endregion

#pragma region Travel
		/// <summary>
		/// Makes this AtomGroup travel and react when terrain is hit. Effects are directly applied to the owning MOSRotating.
		/// </summary>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MOSR's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.</returns>
		float Travel(float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel and react when terrain is hit. Effects are applied to the passed in variables.
		/// </summary>
		/// <param name="position">A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after it's done traveling.</param>
		/// <param name="velocity">A Vector with the total desired velocity of the AtomGroup. Will also be altered according to any collision response.</param>
		/// <param name="rotation">The current rotation Matrix of the AtomGroup. Will be altered accordingly as travel happens.</param>
		/// <param name="angularVel">The current desired angular velocity of the owner MOSR, in rad/sec. Will be altered.</param>
		/// <param name="didWrap">A bool that will be set to whether the position change involved a wrapping of the scene or not.</param>
		/// <param name="totalImpulse">A float to be filled out with the total magnitudes of all the forces exerted on this through collisions during this frame.</param>
		/// <param name="mass">The designated mass of the AtomGroup at this time.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MOSR's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.</returns>
		/// <remarks>
		/// Pseudocode explaining how this works can be found at: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Notes-on-AtomGroup::Travel.
		/// </remarks>
		float Travel(Vector &position, Vector &velocity, Matrix &rotation, float &angularVel, bool &didWrap, Vector &totalImpulse, float mass, float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel without rotation and react with the scene by pushing against it.
		/// </summary>
		/// <param name="position">A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after done traveling.</param>
		/// <param name="velocity">A Vector with the total desired velocity of the AtomGroup.</param>
		/// <param name="pushForce">The maximum force that the push against other stuff in the scene can have, in Newtons (N).</param>
		/// <param name="didWrap">A bool that will be set to whether the position change involved a wrapping of the scene or not.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MOSR's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>A Vector with the resulting push impulse force, in Newton-second (Ns).</returns>
		Vector PushTravel(Vector &position, const Vector &velocity, float pushForce, bool &didWrap, float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel as a pushing entity relative to the position of the owning MOSRotating.
		/// If stuff in the scene is hit, resulting forces are be added to the owning MOSRotating's impulse forces.
		/// </summary>
		/// <param name="jointPos">A reference to a Vector with the world position of the limb joint which this AtomGroup is being pushed along.</param>
		/// <param name="velocity">A Vector with the velocity of the owning MOSRotating.</param>
		/// <param name="rotation">A Matrix with the rotation of the owning MOSRotating.</param>
		/// <param name="limbPath">A LimbPath which this AtomGroup should travel along.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="restarted">Pointer to a bool which gets set to true if the LimbPath got restarted during this push. It does NOT get initialized to false!</param>
		/// <param name="affectRotation">Whether the forces created by this should have rotational leverage on the owner or only have translational effect.</param>
		/// <returns>Whether the LimbPath passed in could start free of terrain or not.</returns>
		bool PushAsLimb(const Vector &jointPos, const Vector &velocity, const Matrix &rotation, LimbPath &limbPath, const float travelTime, bool *restarted = nullptr, bool affectRotation = true);

		/// <summary>
		/// Makes this AtomGroup travel as a lifeless limb, constrained to a radius around the joint pin in the center.
		/// </summary>
		/// <param name="ownerPos">A Vector with the world position of the owner MOSRotating.</param>
		/// <param name="jointOffset">A Vector with the rotated offset of the joint that this should flail around in a radius.</param>
		/// <param name="limbRadius">The radius/range of the limb this is to simulate.</param>
		/// <param name="velocity">A Vector with the velocity of the owning MOSRotating.</param>
		/// <param name="angularVel">A float with the angular velocity in rad/sec of the owning MOSRotating.</param>
		/// <param name="mass">The mass of this dead weight limb.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		void FlailAsLimb(const Vector &ownerPos, const Vector &jointOffset, const float limbRadius, const Vector &velocity, const float angularVel, const float mass, const float travelTime);
#pragma endregion

#pragma region Collision
		/// <summary>
		/// Adds a MOID that this AtomGroup should ignore collisions with during its next Travel sequence.
		/// </summary>
		/// <param name="moidToIgnore">The MOID to add to the ignore list.</param>
		void AddMOIDToIgnore(MOID moidToIgnore) { m_IgnoreMOIDs.push_back(moidToIgnore); }

		/// <summary>
		/// Checks whether this AtomGroup is set to ignore collisions with a MOSR of a specific MOID.
		/// </summary>
		/// <param name="whichMOID">The MOID to check if it is ignored.</param>
		/// <returns>Whether or not this MOID is being ignored.</returns>
		bool IsIgnoringMOID(MOID whichMOID) { return (*(m_Atoms.begin()))->IsIgnoringMOID(whichMOID); }

		/// <summary>
		/// Clears the list of MOIDs that this AtomGroup is set to ignore collisions with during its next Travel sequence.
		/// This should be done each frame so that fresh MOIDs can be re-added. (MOIDs are only valid during a frame).
		/// </summary>
		void ClearMOIDIgnoreList() { m_IgnoreMOIDs.clear(); }

		/// <summary>
		/// Gets whether any of the Atoms in this AtomGroup are on top of terrain pixels.
		/// </summary>
		/// <returns>Whether any Atom of this AtomGroup is on top of a terrain pixel.</returns>
		bool InTerrain() const;

		/// <summary>
		/// Gets the ratio of how many Atoms of this AtomGroup are on top of intact terrain pixels.
		/// </summary>
		/// <returns>The ratio of Atoms on top of terrain pixels, from 0 to 1.0.</returns>
		float RatioInTerrain() const;

		/// <summary>
		/// Checks whether any of the Atoms in this AtomGroup are on top of terrain pixels, and if so, attempt to move the OwnerMO out so none of the Atoms are inside any terrain pixels anymore.
		/// </summary>
		/// <param name="position">Current position of the owner MOSR.</param>
		/// <param name="strongerThan">Only attempt to move out of materials stronger than this specific ID.</param>
		/// <returns>Whether any intersection was successfully resolved. Will return true even if there wasn't any intersections to begin with.</returns>
		bool ResolveTerrainIntersection(Vector &position, unsigned char strongerThan = 0) const;

		/// <summary>
		/// Checks whether any of the Atoms in this AtomGroup are on top of MOSprites, and if so, attempt to move the OwnerMO out so none of the Atoms are inside the other MOSprite's silhouette anymore.
		/// </summary>
		/// <param name="position">Current position of the owner MOSR.</param>>
		/// <returns>Whether all intersections were successfully resolved.</returns>
		bool ResolveMOSIntersection(Vector &position);
#pragma endregion

#pragma region Debug
		/// <summary>
		/// Draws this AtomGroup's current graphical debug representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="useLimbPos">Whether to use the limb position of this AtomGroup, or the owner's position.</param>
		/// <param name="color">The color to draw the Atoms' pixels as.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos, bool useLimbPos = false, unsigned char color = 34) const;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		// TODO: It's probably worth trying out changing this from a list to a vector. m_Atoms is iterated over often and we could probably get some big gainz by doing this swap.
		// The downside is anytime attachables with atoms get added we may have the cost of resizing the vector but that's an uncommon use case while iterating over atoms happens multiple times per frame.
		std::list<Atom *> m_Atoms; //!< List of Atoms that constitute the group. Owned by this.
		std::unordered_map<long, std::list<Atom *>> m_SubGroups; //!< Sub groupings of Atoms. Points to Atoms owned in m_Atoms. Not owned.

		MOSRotating *m_OwnerMOSR; //!< The owner of this AtomGroup. The owner is obviously not owned by this AtomGroup.
		float m_StoredOwnerMass; //!< The stored mass for the owner MOSR. Used to figure out when the moment of inertia needs to be recalculated due to significant mass changes.
		const Material *m_Material; //!< Material of this AtomGroup.

		bool m_AutoGenerate; //!< Whether the Atoms in this AtomGroup were automatically generated based on a sprite, or manually defined.

		/// <summary>
		/// The density of Atoms in this AtomGroup along the outline of the owning MOSR's graphical representation. Higher values mean more pixels are skipped along the outline when placing Atoms.
		/// For example: a box that is 20x20px will have an outline of 80px, with a resolution value of 10 an Atom will be placed every 10 pixels on this outline, resulting in an AtomGroup that
		/// consists of 8 Atoms total with 2 Atoms on each plane. Note that the outline isn't actually "unwrapped" and higher values may result in slightly less accurate Atom placement on complex sprites.
		/// 0 means the Atoms in this AtomGroup were defined manually. 1 means the whole outline will be populated with Atoms, resulting in the most accurate physical representation.
		/// </summary>
		int m_Resolution;

		int m_Depth; //!< The depth Atoms in this AtomGroup are placed off the edge of the owning MOSR's graphical representation outline towards it's center, in pixels.

		Vector m_JointOffset; //!< The offset of the wrist/ankle relative to this AtomGroup's origin when used as a limb.
		Vector m_LimbPos; //!< The last position of this AtomGroup when used as a limb.

		float m_MomentOfInertia; //!< Moment of Inertia for this AtomGroup.

		std::list<MOID> m_IgnoreMOIDs; //!< List of MOIDs this AtomGroup will ignore collisions with.

	private:

#pragma region Create Breakdown
		/// <summary>
		/// Generates an AtomGroup using the owner MOSRotating's sprite outline.
		/// </summary>
		/// <param name="ownerMOSRotating">MOSRotating whose outline will be approximated by Atoms of this AtomGroup.</param>
		void GenerateAtomGroup(MOSRotating *ownerMOSRotating);

		/// <summary>
		/// Create and add an Atom to this AtomGroup's list of Atoms. This is called during GenerateAtomGroup().
		/// </summary>
		/// <param name="ownerMOSRotating">MOSRotating whose outline will be approximated by Atoms of this AtomGroup.</param>
		/// <param name="spriteOffset">Sprite offset relative to the owner MOSRotating.</param>
		/// <param name="x">X coordinate in the sprite frame.</param>
		/// <param name="y">Y coordinate in the sprite frame.</param>
		/// <param name="calcNormal">Whether to set a normal for the Atom. Should be true for surface Atoms.</param>
		void AddAtomToGroup(MOSRotating *ownerMOSRotating, const Vector &spriteOffset, int x, int y, bool calcNormal);
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this AtomGroup, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif