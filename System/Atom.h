#ifndef _RTEATOM_
#define _RTEATOM_

#include "Matrix.h"
#include "Material.h"
#include "SceneMan.h"

namespace RTE {

	class SLTerrain;
	class MovableObject;

	enum { HITOR = 0, HITEE = 1 };

#pragma region HitData
	/// <summary>
	/// A struct to keep all data about a rigid body collision in one package.
	/// </summary>
	struct HitData {

		MovableObject *Body[2]; //!< Pointers to the two hitting bodies. If hitee is 0, that means collision with terrain. The HitData struct doesn't own these.
		MovableObject *RootBody[2]; //!< Pointers to root parents of the two hitting bodies. If hitee is 0, that means collision with terrain. The HitData struct doesn't own these.

		Vector BitmapNormal; //!< The approximated normal vector of the bitmap that the hitee is presenting to the hittor at the collision point. The inverse of this is the one representing the hittor bitmap.

		const Material *HitMaterial[2]; //!< The material of the respective bodies at the hit point.

		float TotalMass[2]; //!< Total mass of each body.
		float MomInertia[2]; //!< Moment of inertia. If 0, assume to be a point mass.
		float SquaredMIHandle[2]; //!< The torque handle used to calculate the moment of inertia's effects, pre-squared.

		Vector HitPoint; //!< The hit point of the collision in absolute scene units.
		Vector HitRadius[2]; //!< The vector IN METERS between the CoM and the HitPoint.
		Vector HitVel[2]; //!< The collision velocity of the respective bodies at the HitPoint, including rotations.
		Vector VelDiff; //!< The difference in velocity between the two bodies HitPoint velocities.

		Vector PreImpulse[2]; //!< The impulse force of each body at the hit point just prior to collision.
		Vector ResImpulse[2]; //!< The resulting impulse as response to the collision.

		float HitDenominator; //!< The impulse equation's denominator for this collision.
		float ImpulseFactor[2]; //!< The factor by which the final impulse vector should be scaled to account for multiple hits in the same step.
		bool Terminate[2]; //!< Whether either of the bodies should be stopped and deleted as a result of the collision.

		/// <summary>
		/// Constructor method used to instantiate a HitData object in system memory.
		/// </summary>
		HitData() { Clear(); }

		/// <summary>
		/// Resets the entire HitData object to the default settings or values.
		/// </summary>
		void Reset() { Clear(); }

		/// <summary>
		/// An assignment operator for setting one HitData equal to another.
		/// </summary>
		/// <param name="rhs">A HitData reference.</param>
		/// <returns>A reference to the changed HitData.</returns>
		HitData & operator=(const HitData &rhs);

		/// <summary>
		/// Clears all the member variables of this HitData, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
#pragma endregion

	/// <summary>
	/// A point (pixel) that tests for collisions with a BITMAP's drawn pixels, ie not the mask color. Owned and operated by other objects.
	/// </summary>
	class Atom : public Serializable {

	public:

		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an Atom object in system memory. Create() should be called before using the object.
		/// </summary>
		Atom() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate an Atom object identical to an already existing one.
		/// </summary>
		/// <param name="reference">An Atom object which is passed in by reference.</param>
		Atom(const Atom &reference) { if (this != &reference) { Clear(); Create(reference); } }

		/// <summary>
		/// Convenience constructor to both instantiate an Atom in memory and Create it at the same time.
		/// </summary>
		/// <param name="offset">An offset Vector that will be used to offset collision calculations.</param>
		/// <param name="material">A Material that defines what material this Atom is made of.</param>
		/// <param name="owner">The owner MovableObject of this Atom. Ownership is NOT transferred!</param>
		/// <param name="trailColor">The trail color.</param>
		/// <param name="trailLength">The trail length. If 0, no trail will be drawn.</param>
		Atom(const Vector &offset, Material const *material, MovableObject *owner, Color trailColor = Color(), int trailLength = 0) { Clear(); Create(offset, material, owner, trailColor, trailLength); }

		/// <summary>
		/// Convenience constructor to both instantiate an Atom in memory and Create it at the same time.
		/// </summary>
		/// <param name="offset">An offset Vector that will be used to offset collision calculations.</param>
		/// <param name="materialID">The material ID of the Material that defines what this Atom is made of.</param>
		/// <param name="owner">The owner MovableObject of this Atom. Ownership is NOT transferred!</param>
		/// <param name="trailColor">The trail color.</param>
		/// <param name="trailLength">The trail length. If 0, no trail will be drawn.</param>
		Atom(const Vector &offset, unsigned char materialID, MovableObject *owner, Color trailColor = Color(), int trailLength = 0) { Clear(); Create(offset, g_SceneMan.GetMaterialFromID(materialID), owner, trailColor, trailLength); }

		/// <summary>
		/// Creates an Atom to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Atom to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Atom &reference);

		/// <summary>
		/// Makes the Atom object ready for use.
		/// </summary>
		/// <param name="offset">An offset Vector that will be used to offset collision calculations.</param>
		/// <param name="material">A Material that defines what material this Atom is made of.</param>
		/// <param name="owner">The owner MovableObject of this Atom. Ownership is NOT transferred!</param>
		/// <param name="trailColor">The trail color.</param>
		/// <param name="trailLength">The trail length. If 0, no trail will be drawn.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Vector &offset, Material const *material, MovableObject *owner, Color trailColor = Color(), int trailLength = 0);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an Atom object before deletion from system memory.
		/// </summary>
		~Atom() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Atom object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Memory Management
		/// <summary>
		/// Grabs from the pre-allocated pool, an available chunk of memory the exact size of an Atom. OWNERSHIP IS TRANSFERRED!
		/// </summary>
		/// <returns>A pointer to the pre-allocated pool memory. OWNERSHIP IS TRANSFERRED!</returns>
		static void * GetPoolMemory();

		/// <summary>
		/// Adds a certain number of newly allocated instances to this' pool.
		/// </summary>
		/// <param name="fillAmount">The number of instances to fill with. If 0 is specified, the set refill amount will be used.</param>
		static void FillPool(int fillAmount = 0);

		/// <summary>
		/// Returns a raw chunk of memory back to the pre-allocated available pool.
		/// </summary>
		/// <param name="returnedMemory">The raw chunk of memory that is being returned. Needs to be the same size as an Atom. OWNERSHIP IS TRANSFERRED!</param>
		/// <returns>The count of outstanding memory chunks after this was returned.</returns>
		static int ReturnPoolMemory(void *returnedMemory);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current owner MovableObject of this AtomGroup.
		/// </summary>
		/// <returns>A const pointer to the owner.</returns>
		const MovableObject * GetOwner() const { return m_OwnerMO; }

		/// <summary>
		/// Sets the current owner MovableObject of this AtomGroup.
		/// </summary>
		/// <param name="newOwner">A pointer to the new owner. Ownership is NOT transferred!</param>
		void SetOwner(MovableObject *newOwner) { m_OwnerMO = newOwner; }

		/// <summary>
		/// Gets the group ID of this Atom.
		/// </summary>
		/// <returns>The group ID of this Atom.</returns>
		long GetSubID() const { return m_SubgroupID; }

		/// <summary>
		/// Sets the subgroup ID of this Atom.
		/// </summary>
		/// <param name="newID">The new subgroup ID of this Atom.</param>
		void SetSubID(long newID = 0) { m_SubgroupID = newID; }

		/// <summary>
		/// Gets the material of this Atom.
		/// </summary>
		/// <returns>The material of this Atom.</returns>
		Material const * GetMaterial() const { return m_Material ? m_Material : g_SceneMan.GetMaterialFromID(g_MaterialAir); }

		/// <summary>
		/// Sets the material of this Atom.
		/// </summary>
		/// <param name="newMat">The new material of this Atom.</param>
		void SetMaterial(Material *newMat) { m_Material = newMat; }

		/// <summary>
		/// Gets the Color of this Atom's trail.
		/// </summary>
		/// <returns>A Color object describing the trail color.</returns>
		Color GetTrailColor() const { return m_TrailColor; }

		/// <summary>
		/// Sets the color value of this Atom's trail.
		/// </summary>
		/// <param name="newTrailColor">A Color object specifying the new trail color.</param>
		void SetTrailColor(Color newTrailColor) { m_TrailColor = newTrailColor; }

		/// <summary>
		/// Gets the longest a trail can be drawn, in pixels.
		/// </summary>
		/// <returns>The new max length, in pixels. If 0, no trail is drawn.</returns>
		int GetTrailLength() const { return m_TrailLength; }

		/// <summary>
		/// Sets the longest a trail can be drawn, in pixels.
		/// </summary>
		/// <param name="trailLength">The new max length, in pixels. If 0, no trail is drawn.</param>
		void SetTrailLength(const int trailLength) { m_TrailLength = trailLength; }

		/// <summary>
		/// Gets the length variation of this Atom's trail.
		/// </summary>
		/// <returns>The length variation of this Atom's trail.</returns>
		float GetTrailLengthVariation() const { return m_TrailLengthVariation; }

		/// <summary>
		/// Sets the length variation scalar of a trail.
		/// </summary>
		/// <param name="trailLengthVariation">The new length variation scalar, 0 meaning no variation and 1 meaning full variation.</param>
		void SetTrailLengthVariation(float trailLengthVariation) { m_TrailLengthVariation = trailLengthVariation; }

		/// <summary>
		/// Gets the offset vector that was first set for this Atom. The GetOffset may have additional offsets baked into it if this is part of an group.
		/// </summary>
		/// <returns>The original offset Vector.</returns>
		const Vector & GetOriginalOffset() const { return m_OriginalOffset; }

		/// <summary>
		/// Gets the offset vector.
		/// </summary>
		/// <returns>The current offset Vector.</returns>
		const Vector & GetOffset() const { return m_Offset; }

		/// <summary>
		/// Sets a new offset vector for the collision calculations.
		/// </summary>
		/// <param name="newOffset">A const reference to a Vector that will be used as offset.</param>
		void SetOffset(const Vector &newOffset) { m_Offset = newOffset; }

		/// <summary>
		/// Gets the surface normal of this vector, if it has been successfully calculated. If not, it'll be a 0 vector.
		/// </summary>
		/// <returns>The current normalized surface normal Vector of this.</returns>
		const Vector & GetNormal() const { return m_Normal; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Sets up the normal of this atom, based on its position on a sprite's bitmap.
		/// It will check pixels around it and see if they are inside the object or not, and infer a collision normal based on that. THIS ONLY WORKS IF THE ATOM IS ON THE SURFACE OF THE SPRITE!
		/// </summary>
		/// <param name="sprite">The bitmap to check against. Ownership IS NOT transferred!</param>
		/// <param name="spriteCenter">Where on the bitmap the center of the object is. This atom's offset will be applied automatically before checking for its normal.</param>
		/// <returns>Whether normal was successfully derived from the bitmap. If not, then a provisional one is derived from the offset.</returns>
		bool CalculateNormal(BITMAP *sprite, Vector spriteCenter);
#pragma endregion

#pragma region Collision
		/// <summary>
		/// Gets the stored data struct on the last collision experienced by this Atom.
		/// </summary>
		/// <returns>The Vector describing the velocity of this Atom at the last time it hit something.</returns>
		HitData & GetHitData() { return m_LastHit; }

		/// <summary>
		/// Sets the HitData struct this Atom uses to represent the last hit it experienced.
		/// </summary>
		/// <param name="newHitData">A reference to a HitData struct that will be copied to the Atom's.</param>
		void SetHitData(const HitData &newHitData) { m_LastHit = newHitData; }

		/// <summary>
		/// Checks whether this Atom is set to ignore collisions with the terrain.
		/// </summary>
		/// <returns>Whether or not this is ignoring hits the with terrain.</returns>
		bool IsIgnoringTerrain() const { return m_TerrainHitsDisabled; }

		/// <summary>
		/// Checks whether this Atom is temporarily ignoring all MO hits.
		/// </summary>
		/// <returns>Whether or not this is ignoring all MO hits.</returns>
		bool IsIgnoringAllMOs() const { return m_MOHitsDisabled; }

		/// <summary>
		/// Checks whether this Atom is set to ignore collisions with a MO of a specific MOID.
		/// </summary>
		/// <param name="whichMOID">The MOID to check if it is ignored.</param>
		/// <returns>Whether or not this MOID is being ignored.</returns>
		bool IsIgnoringMOID(MOID whichMOID);

		/// <summary>
		/// Adds a MOID that this Atom should ignore collisions with during its next travel sequence.
		/// </summary>
		/// <param name="ignore">The MOID to add to the ignore list.</param>
		void AddMOIDToIgnore(MOID ignore) { m_IgnoreMOIDs.push_back(ignore); }

		/// <summary>
		/// AtomGroup may set this shared list of ignored MOIDs to avoid setting and removing ignored MOIDs for every atom one by one. The list is maintained only by AtomGroup, Atom never owns it.
		/// </summary>
		/// <param name="ignoreMOIDsByGroup">New MOIDs list to ignore.</param>
		void SetIgnoreMOIDsByGroup(std::list<MOID> const * ignoreMOIDsByGroup) { m_IgnoreMOIDsByGroup = ignoreMOIDsByGroup; };

		/// <summary>
		/// Clear the list of MOIDs that this Atom is set to ignore collisions with during its next travel sequence.
		/// This should be done each frame so that fresh MOIDs can be re-added. (MOIDs are only valid during a frame).
		/// </summary>
		void ClearMOIDIgnoreList() { m_IgnoreMOIDs.clear(); }

		/// <summary>
		/// Gets the number of consecutive penetrations of Terrain that this Atom has successfully made, ending with wherever it is now.
		/// </summary>
		/// <returns>The number of consecutive penetrations. Resets to 0 as soon as penetration streak ends.</returns>
		int GetNumPenetrations() const { return m_NumPenetrations; }

		/// <summary>
		/// Returns the ID of the MO hit at the previously taken step by TakeStep. This will potentially return g_NoMOID if this Atom is not set to hit MO's or the MO hit is marked to be ignored.
		/// </summary>
		/// <returns>The ID of the non-ignored MO, if any, that this Atom is now intersecting because of the last step taken.</returns>
		MOID HitWhatMOID() const { return m_MOIDHit; }

		/// <summary>
		/// Calculates the collision response with another MO, if the last step taken forward resulted in an intersection with a non-ignored MO.
		/// Note that one step backward should be taken after the intersecting step so that the Atom is not intersecting the hit MO anymore.
		/// The hit response is calculated and appropriate forces are applied to the hit MO and the impulse force imposed on the owner MO are returned, and also stored in this Atom to be later retrieved with GetHitImpulse().
		/// </summary>
		/// <remarks>Collision data should already be set by SetHitData().</remarks>
		/// <returns>Whether this collision is valid and should be considered further.</returns>
		bool MOHitResponse();

		/// <summary>
		/// Returns the terrain material the previously taken step by TakeStep hit, if any.
		/// </summary>
		/// <returns>The ID of the material, if any, that this MO hit during the last step.</returns>
		unsigned char HitWhatTerrMaterial() { return m_TerrainMatHit; }

		/// <summary>
		/// Calculates the collision response with the Terrain, if the last step taken forward resulted in an intersection with the Terrain.
		/// Note that one step backward should be taken after the intersecting step so that the Atom is not intersecting the Terrain anymore, and the hit response calculations can be made accurately.
		/// The resulting impulse vector is also stored in this Atom to be later retrieved with GetHitImpulse().
		/// </summary>
		/// <remarks>Collision data should already be set by SetHitData().</remarks>
		/// <returns>The resulting HitData of this Atom with all the information about the collision filled out.</returns>
		HitData & TerrHitResponse();
#pragma endregion

#pragma region Travel
		/// <summary>
		/// Sets this Atom's initial position up for a straight segment of a trajectory to step through. This is to be done before SetupSeg.
		/// It will report whether the atom's position is inside the terrain or another MO. It also disables this' terrain and/or MO collisions when traveling, until the travel encounters an air/noMO pixel again.
		/// </summary>
		/// <param name="startPos">A Vector with the position to start from, in scene coordinates.</param>
		/// <returns>If the start pos is already intersecting the terrain or an MO, then this will return true. Do collision response as needed.</returns>
		bool SetupPos(Vector startPos);

		/// <summary>
		/// Gets the absolute current position of this Atom, in scene coordinates. This includes the rotated offset, and any steps that may have been taken on a segment.
		/// </summary>
		/// <returns>The current position of the Atom, with the offset baked in.</returns>
		Vector GetCurrentPos() const { return Vector(m_IntPos[X], m_IntPos[Y]); }

		/// <summary>
		/// Sets this Atom up for a straight segment of a trajectory to step through. If this Atom find the startPos to be on an MO, it will ignore any collisions with that MO for the entire segment.
		/// The Scene MUST BE LOCKED before calling this!
		/// </summary>
		/// <param name="startPos">A Vector with the position to start from, in scene coordinates.</param>
		/// <param name="trajectory">A Vector with the trajectory to travel during this next segment.</param>
		/// <param name="stepRatio">The ratio between the actual steps to be taken and the future calls to TakeStep().</param>
		/// <returns>The total number of steps this will take in the newly set-up segment.</returns>
		int SetupSeg(Vector startPos, Vector trajectory, float stepRatio = 1.0);

		/// <summary>
		/// Gets the segment trajectory currently set by SetupSeg.
		/// </summary>
		/// <returns>The currently traversed segment trajectory.</returns>
		Vector GetSegTraj() const { return m_SegTraj; }

		/// <summary>
		/// Indicates the total original length of the current segment set by SetupSeg.
		/// </summary>
		/// <returns>The length of the currently traversed segment.</returns>
		float GetSegLength() const { return m_SegTraj.GetMagnitude(); }

		/// <summary>
		/// Takes one step along the trajectory segment set up by SetupSeg(). The Scene MUST BE LOCKED before calling this!
		/// </summary>
		/// <param name="numSteps">The number of steps to take.</param>
		/// <returns>
		/// Whether anything was hit during the steps or not. MO hits will only be reported if this Atom is set to hit them, and the MO hit is not ignored by this Atom (if the segment started on that MO).
		/// Also if terrain hits are temporarily disabled, they will not be reported.
		/// </returns>
		bool StepForward(int numSteps = 1);

		/// <summary>
		/// Takes one step back, or undos the step, if any, previously taken along the trajectory segment set up by SetupSeg().
		/// </summary>
		/// <remarks>CAUTION: The previous HitWhat vars are not reset to what they previously were!</remarks>
		void StepBack();

		/// <summary>
		/// Gets the normalized ratio of how many steps are actually taken to how many calls to TakeStep are made.
		/// </summary>
		/// <returns>A normalized float describing the step ratio.</returns>
		float GetStepRatio() const { return m_StepRatio; }

		/// <summary>
		/// Sets the ratio of how many steps are actually taken to how many calls to TakeStep are made.
		/// </summary>
		/// <param name="newStepRatio">A float specifying the new step ratio.</param>
		void SetStepRatio(float newStepRatio) { m_StepRatio = newStepRatio; }

		/// <summary>
		/// Indicates how many more steps remain to be taken to traverse the entire trajectory segment.
		/// </summary>
		/// <returns>The number of steps that remain to be taken on the set trajectory segment.</returns>
		int GetStepsLeft() const { return m_Delta[m_Dom] - m_DomSteps; }

		/// <summary>
		/// Tells this Atom that the previous travel move's fractional error isn't valid for consecutive travel moves.
		/// </summary>
		void ChangedDir() { m_ChangedDir = true; }

		/// <summary>
		/// Uses the current state of the owning MovableObject to determine if there are any collisions in the path of its travel during this frame, and if so, apply all collision responses to the MO.
		/// </summary>
		/// <param name="travelTime">The amount of time in s that this Atom is allowed to travel.</param>
		/// <param name="autoTravel">A bool specifying if the end position result should be moved along the trajectory if no terrain is hit.</param>
		/// <param name="scenePreLocked">Whether the Scene has been pre-locked or not.</param>
		/// <returns>The number of hits against terrain that were made during the travel.</returns>
		int Travel(float travelTime, bool autoTravel = true, bool scenePreLocked = false);
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// New operator overload for the pool allocation of Atoms.
		/// </summary>
		/// <param name="size"></param>
		/// <returns></returns>
		static void *operator new (size_t size) { return Atom::GetPoolMemory(); }

		/// <summary>
		/// Delete operator overload for the pool deallocation of Atoms.
		/// </summary>
		/// <param name="instance"></param>
		static void operator delete (void *instance) { Atom::ReturnPoolMemory(instance); }

		/// <summary>
		/// An assignment operator for setting one Atom equal to another.
		/// </summary>
		/// <param name="rhs">An Atom reference.</param>
		/// <returns>A reference to the changed Atom.</returns>
		Atom & operator=(const Atom &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }
#pragma endregion

	protected:

		static constexpr int c_NormalCheckCount = 16; //!< Array size for offsets to form circle in s_NormalChecks.

		static std::vector<void *> s_AllocatedPool; //!< Pool of pre-allocated Atoms.
		static int s_PoolAllocBlockCount; //!< The number of instances to fill up the pool of Atoms with each time it runs dry.
		static int s_InstancesInUse; //!< The number of allocated instances passed out from the pool.
		static const int s_NormalChecks[c_NormalCheckCount][2]; //!< This forms a circle around the Atom's offset center, to check for key color pixels in order to determine the normal at the Atom's position.

		Vector m_Offset; //!< The offset of this Atom for collision calculations.
		Vector m_OriginalOffset; //!< This offset is before altering the m_Offset for use in composite groups.
		Vector m_Normal; //!< The current normalized surface normal Vector of this Atom.
		Material const * m_Material; //!< The material this Atom is made of.
		int m_SubgroupID; //!< Identifying ID for adding and removing atoms from AtomGroups.

		bool m_StepWasTaken; //!< Whether the last call to StepForward actually resulted in a step or not.
		float m_StepRatio; //!< The normalized ratio of how many steps are actually taken to how many calls to TakeStep are made.
		Vector m_SegTraj; //!< The segment trajectory currently set by SetupSeg.
		float m_SegProgress; //!< The segment progress while taking steps.

		bool m_ChangedDir; //!< This is only true if there was a change in direction of trajectory during the last travel move of this Atom.
		int m_PrevError; //!< This is the stored error (fraction) at the end of the last travel move. To be used when the direction wasn't changed during a hit, and will make sure the continued trajectory is straight.
		bool m_ResultWrapped; //!< This is only true when the resulting position reflects a wrap around the scene.

		bool m_MOHitsDisabled; //!< Temporary disabling of MO collisions for this.
		bool m_TerrainHitsDisabled; //!< Temporary disabling of terrain collisions for this. Will be re-enabled once out of terrain again.

		MovableObject *m_OwnerMO; //!< The owner of this Atom. The owner is obviously not owned by this Atom.
		MOID m_IgnoreMOID; //!< Special ignored MOID.
		std::list<MOID> m_IgnoreMOIDs; //!< ignore hits with MOs of these IDs.
		std::list<MOID> const * m_IgnoreMOIDsByGroup; //!< Also ignore hits with MOs of these IDs. This one may be set externally by atom group.

		HitData m_LastHit; //!< Data containing information on the last collision experienced by this Atom.
		MOID m_MOIDHit; //!< The MO, if any, this Atom hit on the last step.
		unsigned char m_TerrainMatHit; //!< The terrain material, if any, this Atom hit on the last step.

		int m_NumPenetrations; //!< Counts consecutive penetrations in a row. Resets to 0 as soon as penetration streak ends.

		/*
		Vector m_HitVel; //!< The velocity at which this atom last hit something.
		Vector m_HitRadius; //!< The rotated offset or radius from the body/AtomGroup's CoM at the time of the last collision this Atom experienced. Observe that this is in meters, not pixels!
		Vector m_HitImpulse; //!< The resulting impulse force of the last collision this Atom experienced in Kg * m/s.
		*/

		Color m_TrailColor; //!< Trail color
		int m_TrailLength; //!< The longest the trail should/can get drawn. If 0, no trail is drawn.
		float m_TrailLengthVariation; //!< What percentage the trail length of this Atom can vary each frame it's drawn. 0 means no variance, 1 means 100% variance between 0 and its TrailLength.

		// Bresenham line algorithm variables
		int m_IntPos[2];
		int m_PrevIntPos[2];
		int m_TrailPos[2];
		int m_HitPos[2];
		int m_Delta[2];
		int m_Delta2[2];
		int m_Increment[2];
		int m_Error;
		int m_Dom;
		int m_Sub;
		int m_DomSteps;
		int m_SubSteps;
		bool m_SubStepped;

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// <summary>
		/// Clears all the member variables of this Atom, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif