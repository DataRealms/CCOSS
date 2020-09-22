#ifndef _RTEATOMGROUP_
#define _RTEATOMGROUP_

#include "Atom.h"

// Defined in Winbase.h and conflicts with our method so we need to undefine it manually.
#undef AddAtom

namespace RTE {

	class MOSRotating;
	class LimbPath;

	/// <summary>
	/// A group of Atoms that move and interact with the terrain in unison.
	/// </summary>
	class AtomGroup : public Entity {

	public:

		EntityAllocation(AtomGroup)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an AtomGroup object in system memory. Create() should be called before using the object.
		/// </summary>
		AtomGroup() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a AtomGroup object identical to an already existing one.
		/// </summary>
		/// <param name="reference">An AtomGroup object which is passed in by reference.</param>
		AtomGroup(const AtomGroup &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Makes the AtomGroup object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a AtomGroup to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the AtomGroup to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const AtomGroup &reference) { return Create(reference, false); }

		/// <summary>
		/// Creates an AtomGroup to be identical to another, by deep copy, with the option to only copy atoms that belong to the reference AtomGroup's owner.
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
		/// <param name="resolution">
		/// Resolution, or density of the Atoms in representing the MOSRotating's silhouette.
		/// 1 means highest resolution, 2 means every other pixel on the MOSRotating's edge is covered by an Atom, 3 mean every third, etc.</param>
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
		MOSRotating * GetOwner() const { return m_OwnerMO; }

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
		bool AutoGenerate() { return m_AutoGenerate; }

		/// <summary>
		/// Gets the resolution of this AtomGroup. Basically density of the Atoms in representing a MOSRotating's silhouette.
		/// </summary>
		/// <returns>The resolution setting. 0 means the Atom config wasn't generated by a silhouette, but defined manually.</returns>
		int GetResolution() const { return m_Resolution; }

		/// <summary>
		/// Gets the depth, or how deep into the bitmap of the owning MO's graphical representation the Atoms of this AtomGroup are located.
		/// </summary>
		/// <returns>The depth, in pixels. If 0, Atoms are right on the bitmap.</returns>
		int GetDepth() const { return m_Depth; }

		/// <summary>
		/// Gets the current position of this AtomGroup as a limb.
		/// </summary>
		/// <param name="hFlipped"></param>
		/// <returns>The absolute limb position in the world.</returns>
		Vector GetLimbPos(bool hFlipped = false) const { return m_LimbPos.GetFloored() + m_JointOffset.GetXFlipped(hFlipped); }

		/// <summary>
		/// Sets the current position of this AtomGroup as a limb.
		/// </summary>
		/// <param name="newPos">The Vector with the new absolute position.</param>
		/// <param name="hFlipped"></param>
		void SetLimbPos(const Vector &newPos, bool hFlipped = false) { m_LimbPos = newPos - m_JointOffset.GetXFlipped(hFlipped); }

		/// <summary>
		/// Gets the current mass moment of inertia of this AtomGroup
		/// </summary>
		/// <returns>A float with the moment of inertia, in Kg * SceneUnits^2.</returns>
		float GetMomentOfInertia();
#pragma endregion

#pragma region Atom Management
		/// <summary>
		/// Adds a new Atom into the internal list that makes up this AtomGroup. Ownership of the Atom IS transferred!
		/// </summary>
		/// <param name="newAtom">A pointer to an Atom that will pushed onto the end of the list. Ownership IS transferred!</param>
		/// <param name="atomID">The subgroup ID that the new Atom will have within the group.</param>
		void AddAtom(Atom *newAtom, long atomID = 0) { newAtom->SetSubID(atomID); m_Atoms.push_back(newAtom); }

		/// <summary>
		/// Adds a list of new Atoms to the internal list that makes up this AtomGroup. Ownership of all Atoms in the list IS NOT transferred!
		/// </summary>
		/// <param name="atomList">A list of pointers to a Atoms whose copies will be pushed onto the end of this AtomGroup's list. Ownership IS NOT transferred!</param>
		/// <param name="subID">The desired subgroup IDs of the atoms to be added.</param>
		/// <param name="offset">An offset that should be applied to all added Atoms.</param>
		/// <param name="offsetRotation">The rotation of the placed Atoms around the specified offset.</param>
		void AddAtoms(const std::list<Atom *> &atomList, long subID = 0, const Vector &offset = Vector(), const Matrix &offsetRotation = Matrix());

		/// <summary>
		/// Removes all Atoms of a specific subgroup ID from this AtomGroup.
		/// </summary>
		/// <param name="removeID">The ID of the subgroup of atoms to remove.</param>
		/// <returns>Whether any Atoms of that subgroup ID were found and removed.</returns>
		bool RemoveAtoms(long removeID);

		/// <summary>
		/// Updates the offsets of a subgroup of Atoms in this AtomGroup. This allows chunks representing sub parts of the whole to more closely represent the graphics better.
		/// </summary>
		/// <param name="subID">The desired subgroup IDs of the Atoms to update offsets of.</param>
		/// <param name="newOffset">An offset that should be re-applied to Atoms of that subgroup.</param>
		/// <param name="newOffsetRotation">The rotation of the updated Atoms around the specified offset.</param>
		/// <returns>Whether any Atoms of that group was found and updated.</returns>
		bool UpdateSubAtoms(long subID = 0, const Vector &newOffset = Vector(), const Matrix& newOffsetRotation = Matrix());
#pragma endregion

#pragma region Travel
		/// <summary>
		/// Makes this AtomGroup travel and react when terrain is hit. Effects are directly applied to the owning MOSRotating.
		/// </summary>
		/// <param name="travelTime">The amount of time in s that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MO's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MO's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.</returns>
		float Travel(float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel and react when terrain is hit. Effects are applied to the passed in variables.
		/// </summary>
		/// <param name="position">A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after done traveling.</param>
		/// <param name="velocity">A Vector with the total desired velocity of the AtomGroup. Will also be altered according to any collision response.</param>
		/// <param name="rotation">The current rotation Matrix of the AtomGroup. Will be altered accordingly as travel happens.</param>
		/// <param name="angVel">The current desired angular velocity of the owner MO, in r/s. Will be altered.</param>
		/// <param name="didWrap">A bool that will be set to whether the position change involved a wrapping of the scene or not.</param>
		/// <param name="totalImpulse">A float to be filled out with the total magnitudes of all the forces exerted on this through collisions during this frame.</param>
		/// <param name="mass">The designated mass of the AtomGroup at this time.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MO's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MO's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.</returns>
		/// <remarks>
		/// Pseudocode explaining how this works can be found at: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Notes-on-AtomGroup::Travel
		/// </remarks>
		float Travel(Vector &position, Vector &velocity, Matrix &rotation, float &angVel, bool &didWrap, Vector &totalImpulse, float mass, float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel without rotation and react with the scene by pushing against it.
		/// </summary>
		/// <param name="position">A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after done traveling.</param>
		/// <param name="velocity">A Vector with the total desired velocity of the AtomGroup. Will not be altered.</param>
		/// <param name="pushForce">The maximum force that the push against other stuff in the scene can have, in kg * m/s^2.</param>
		/// <param name="didWrap">A bool that will be set to whether the position change involved a wrapping of the scene or not.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		/// <param name="callOnBounce">Whether to call the parent MO's OnBounce function upon bouncing against anything or not.</param>
		/// <param name="callOnSink">Whether to call the parent MO's OnSink function upon sinking into anything or not.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>A Vector with the resulting push impulse force, in kg * m/s.</returns>
		Vector PushTravel(Vector &position, Vector velocity, float pushForce, bool &didWrap, float travelTime, bool callOnBounce = false, bool callOnSink = false, bool scenePreLocked = false);

		/// <summary>
		/// Makes this AtomGroup travel as a pushing entity relative to the position of the owning MOSRotating.
		/// If stuff in the scene is hit, resulting forces are be added to the owning MOSRotating's impulse forces.
		/// </summary>
		/// <param name="jointPos">A reference to a Vector with the world position of the joint of the of the path along which this AtomGroup is being pushed.</param>
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
		/// <param name="angVel">A float with the angular velocity in rad/s of the owning MOSRotating.</param>
		/// <param name="mass">The mass of this dead weight limb.</param>
		/// <param name="travelTime">The amount of time in seconds that this AtomGroup is supposed to travel.</param>
		void FlailAsLimb(const Vector &ownerPos, const Vector &jointOffset, const float limbRadius, const Vector &velocity, const float angVel, const float mass, const float travelTime);
#pragma endregion

#pragma region Collision
		/// <summary>
		/// Adds a MOID that this AtomGroup should ignore collisions with during its next Travel sequence.
		/// </summary>
		/// <param name="moidToIgnore">The MOID to add to the ignore list.</param>
		void AddMOIDToIgnore(MOID moidToIgnore) { m_IgnoreMOIDs.push_back(moidToIgnore); }

		/// <summary>
		/// Checks whether this AtomGroup is set to ignore collisions with a MO of a specific MOID.
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
		/// Gets the ratio of how many Atoms of this AtomGroup that are on top of intact terrain pixels.
		/// </summary>
		/// <returns>The ratio of Atoms on top of terrain pixels, form 0 to 1.0.</returns>
		float RatioInTerrain() const;

		/// <summary>
		/// Checks whether any of the Atoms in this AtomGroup are on top of terrain pixels, and if so, attempt to move the OwnerMO out so none of the Atoms are inside any terrain pixels anymore.
		/// </summary>
		/// <param name="position">Current position of the owner MO.</param>
		/// <param name="strongerThan">Only attempt to move out of materials stronger than this specific ID.</param>
		/// <returns>Whether any intersection was successfully resolved. Will return true even if there wasn't any intersections to begin with.</returns>
		bool ResolveTerrainIntersection(Vector &position, unsigned char strongerThan = 0) const;

		/// <summary>
		/// Checks whether any of the Atoms in this AtomGroup are on top of MOSprites, and if so, attempt to move the OwnerMO out so none of the Atoms are inside the other MOSprite's silhouette anymore.
		/// </summary>
		/// <param name="position">Current position of the owner MO.</param>
		/// <param name="rotation">Current rotation of the owner MO.</param>
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
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), bool useLimbPos = false, unsigned char color = 34) const;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::list<Atom *> m_Atoms; //!< List of Atoms that constitute the group. Owned by this.
		std::unordered_map<long, std::list<Atom *>> m_SubGroups; //!< Sub groupings of Atoms. Points to atoms owned in m_Atoms. Not owned.

		MOSRotating *m_OwnerMO; //!< The owner of this AtomGroup. The owner is obviously not owned by this AtomGroup.
		const Material *m_Material; //!< Material of this AtomGroup

		bool m_AutoGenerate; //!< // Whether or not the Atoms were automatically generated based on a sprite, or manually defined.
		int m_Resolution; //!< Resolution, or density of the Atoms in representing a MOSRotating's silhouette. 0 means the Atom config wasn't generated by a silhouette, but defined manually.
		int m_Depth; //!< Depth, or how deep into the bitmap of the owning MO's graphical representation the Atoms of this AtomGroup are located, in pixels.

		Vector m_JointOffset; //!< The offset of the wrist/ankle relative to this AtomGroup's origin when used as a limb.
		Vector m_LimbPos; //!< The last position of this AtomGroup when used as a limb.

		float m_MomInertia; //!< Moment of Inertia for this AtomGroup.

		std::list<MOID> m_IgnoreMOIDs; //!< Ignore hits with MOs of these IDs.

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