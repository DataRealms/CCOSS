#ifndef _RTEATOMGROUP_
#define _RTEATOMGROUP_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            AtomGroup.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the AtomGroup class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Atom.h"
#include "LimbPath.h"
#include "Timer.h"

namespace RTE
{

enum { BODY = 0, LIMB = 1 };

class MovableObject;
class MOSRotating;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           AtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A group of Atom:s that move and interact with the terrain in unison.
// Parent(s):       Entity.
// Class history:   05/29/2001 AtomGroup created.

class AtomGroup:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(AtomGroup)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AtomGroup object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    AtomGroup() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a AtomGroup object
//                  identical to an already existing one.
// Arguments:       A AtomGroup object which is passed in by reference.

    AtomGroup(const AtomGroup &reference) { Clear(); Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AtomGroup object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~AtomGroup() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AtomGroup object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup to be identical to another, by deep copy.
// Arguments:       A reference to the AtomGroup to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const AtomGroup &reference);

    /// <summary>
    /// Creates an AtomGroup to be identical to another, by deep copy, with the option to only copy atoms that belong to the reference AtomGroup's owner
    /// </summary>
    /// <param name="reference">A reference to the AtomGroup to deep copy</param>
    /// <param name="onlyCopyOwnerAtoms">Whether or not to only copy atoms that belong to the reference AtomGroup's owner directly</param>
    /// <returns>An error return value signalling success or any particular failure. Anything below 0 is an error signal.</returns>
    virtual int Create(const AtomGroup & reference, bool onlyCopyOwnerAtoms);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup after the silhouette shape of a passed in MOSRotating,
//                  by dotting the outline of the sprite with atoms. The passed in MOSRotating
//                  will also be made the owner of this AtomGroup.
// Arguments:       A pointer to an MOSRotating whose outline will be approximated by Atoms
//                  of this AtomGroup, and that will be set as the owner of this AtomGroup.
//                  Ownership is not transferred!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(MOSRotating *pOwnerMOSRotating);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AtomGroup after the silhouette shape of a passed in MOSRotating,
//                  by dotting the outline of the sprite with atoms. The passed in MOSRotating
//                  will also be made the owner of this AtomGroup.
// Arguments:       A pointer to an MOSRotating whose outline will be approximated by Atoms
//                  of this AtomGroup, and that will be set as the owner of this AtomGroup.
//                  Ownership is not transferred!
//                  The Material that the Atom:s of this AtomGroup should be of.
//                  Resolution, or density of the Atoms in representing the MOSRotating's
//                  silhouette. 1 means higest res, 2 means every other pixel on the
//                  MOSRotating's edge is covered by an atom, 3 mean every third, etc.
//                  The depth into the sprite that the atom:s should be placed.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(MOSRotating *pOwnerMOSRotating, Material const * material, int resolution = 1, int depth = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire AtomGroup, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AtomGroup object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: AtomGroup addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for AtomGroup:s which adds together the
//                  Atom:s of two Groups and merges them into one.
// Arguments:       An AtomGroup reference as the left hand side operand..
//                  An AtomGroup reference as the right hand side operand.
// Return value:    The resulting AtomGroup.

//    friend AtomGroup operator+(const AtomGroup &lhs, const AtomGroup &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: AtomGroup pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for AtomGroup:s pointer which adds together
//                  the Atom:s of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!
// Arguments:       An AtomGroup pointer as the left hand side operand.
//                  An AtomGroup pointer as the right hand side operand.
// Return value:    The resulting dynamically allocated AtomGroup. OWNERSHIP IS TRANSFERRED!

//    friend AtomGroup * operator+(const AtomGroup *lhs, const AtomGroup *rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateMaxRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the longest magnitude of all the atom's offsets.
// Arguments:       None.
// Return value:    The largest magnitude of offset, in pixels.

    float CalculateMaxRadius() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the material of this AtomGroup
// Arguments:       None.
// Return value:    A reference to the material.

	Material const * GetMaterial() const { if (m_pMaterial) return m_pMaterial; else return g_SceneMan.GetMaterialFromID(g_MaterialAir); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtomList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current list of Atom:s that make up the group.
// Arguments:       None.
// Return value:    A reference to the list.

    const std::list<Atom *> & GetAtomList() const { return m_Atoms; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtomCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current number of Atoms that make up the group.
// Arguments:       None.
// Return value:    An int with the count.

    int GetAtomCount() const { return m_Atoms.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMomentOfInertia
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mass moment of inertia of this AtomGroup
// Arguments:       None.
// Return value:    A float with the moment of inertia, in Kg * SceneUnits^2

    float GetMomentOfInertia();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current owner MOSRotating of this AtomGroup.
// Arguments:       None.
// Return value:    A const pointer to the owner.

    MOSRotating * GetOwner() const { return m_pOwnerMO; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Atom is set to collide with MovableObject:s.
// Arguments:       None.
// Return value:    Whether or not this Atom is checking for collisions with MOs or not.

    bool HitsMOs() const { return (*(m_Atoms.begin()))->HitsMOs(); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResolution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the resolution of this AtomGroup. Basically density of the Atoms
//                  in representing a MOSRotating's silhouette. 0 means the Atom config
//                  wasn't genereated by a silhouette, but defined manually.
// Arguments:       None.
// Return value:    The resolution setting. If 0, Atom:s were placed manually.

    const int GetResolution() const { return m_Resolution; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDepth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the depth, or how deep into the bitmap of the owning MO's
//                  graphical representation the Atom:s of this AtomGroup are located.
// Arguments:       None.
// Return value:    The depth, in pixels. If 0, Atom:s are right on the bitmap.

    const int GetDepth() const { return m_Depth; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLimbPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position of this AtomGroup as a limb.
// Arguments:       None.
// Return value:    The absolute limb position in the world

    Vector GetLimbPos(bool hFlipped = false) const
    {
        return m_LimbPos.GetFloored() + m_JointOffset.GetXFlipped(hFlipped);
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLimbPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current position of this AtomGroup as a limb.
// Arguments:       The Vector with the new absolute position
// Return value:    None.

    void SetLimbPos(Vector &newPos, bool hFlipped = false)
    {
        m_LimbPos = newPos - m_JointOffset.GetXFlipped(hFlipped);
    }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current owner MovableObject of this AtomGroup.
// Arguments:       A pointer to the new owner. Ownership is NOT transferred!
// Return value:    None.

    void SetOwner(MOSRotating *newOwner);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this AtomGroup will collide with MovableObject:s or not.
// Arguments:       Whether or not this should check for collisions with MOs or not.
// Return value:    None.

    void SetToHitMOs(bool hitMOs) { for (std::list<Atom *>::iterator itr = m_Atoms.begin(); itr != m_Atoms.end(); ++itr)
                                        (*itr)->SetToHitMOs(hitMOs); }
*/


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AutoGenerate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this AtomGroup's Atom:s are to be automatically generated
//                  based on a bitmap, or manually specified.
// Arguments:       None.
// Return value:    Autogenerate or not.

    bool AutoGenerate() { return m_AutoGenerate; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMOIDToIgnore
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MOID that this Atom should ignore collisions with during its
//                  next travel sequence.
// Arguments:       The MOID to add to the ignore list.
// Return value:    None.

    void AddMOIDToIgnore(MOID ignore);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDIgnoreList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clear the list of MOIDs that this Atom is set to ignore collisions
//                  with during its next travel sequence. This should be done each frame
//                  so that fresh MOIDs can be re-added. (MOIDs are only valid during a
//                  frame)
// Arguments:       None.
// Return value:    None.

    void ClearMOIDIgnoreList();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is set to ignore collisions with a MO of a
///                 specific MOID.
// Arguments:       The MOID to check if it is ignored.
// Return value:    Whether or not this MOID is being ignored.

    bool IsIgnoringMOID(MOID which);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a new Atom onto the internal list that makes up the group.
//                  Ownership IS transferred!
// Arguments:       A pointer to an Atom that will pushed onto the end of the list.
//                  Ownership IS transferred!
//                  The subgroup ID that the new atom will have within the group.
// Return value:    None.

    void AddAtom(Atom *newAtom, int atomID = 0) { newAtom->SetSubID(atomID); m_Atoms.push_back(newAtom); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a list of new Atom:s to the internal list that makes up this group.
//                  Ownership of all Atom:s in the list IS NOT transferred!
// Arguments:       A deque of pointers to a Atom:s whose copies will be pushed onto the
//                  end of this AtomGroup's list. Ownership IS NOT transferred!
//                  The desired subgroup IDs of the atoms to be added.
//                  An offset that should be applied to all added atom:s.
//                  The rotation of the placed atoms around the above offset.
// Return value:    None.

    void AddAtoms(const std::list<Atom *> &atomList, long int subID = 0, const Vector &offset = Vector(), const Matrix &offsetRotation = Matrix());


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSubAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the offsets of a subgroup of Atoms of this. This allows chunks
//                  representing sub parts of the whole to more closely represent the
//                  graphics better.
// Arguments:       The desired subgroup IDs of the atoms to update offsets of.
//                  An offset that should be re-applied applied to atom:s of that subgroup.
//                  The rotation of the updated atoms around the above offset.
// Return value:    Whether any atoms of that group was found and updated.

    bool UpdateSubAtoms(long int subID = 0, const Vector &newOffset = Vector(), const Matrix& newOffsetRotation = Matrix());


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAtoms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all atoms of a specific subgroup ID from this AtomGroup.
// Arguments:       The ID of the subgroup of atoms to remove.
// Return value:    Whether any Atoms of that subgroup ID were found and removed.

    bool RemoveAtoms(long int removeID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together and react when terrain is
//                  hit. Effects are direcly applied to the owning MovableObject.
// Arguments:       The amount of time in s that this Atom is supposed to travel.
//                  Whether to call the parent MO's OnBounce function upon bouncing
//                  against anything, or not.
//                  Whether to call the parent MO's OnSink function upon sinking into
//                  against anything, or not.
//                  Whether the Scene has been pre-locked or not.
// Return value:    The amount of time remaining of the travelTime passed in, in s. This
//                  may only possibly be a nonzero if callOnBounce or callOnSink are true.

    virtual float Travel(float travelTime,
                         bool callOnBounce = false,
                         bool callOnSink = false,
                         bool scenePreLocked = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together and react when terrain is
//                  hit. Effects are applied to the passed in variables.
// Arguments:       A reference to a Vector with the starting position of the AtomGroup
//                  origin. The value of this Vector will be altered accoring to where
//                  the AtomGroup ends up after done travelling.
//                  A Vector with the total desired velocity of the AtomGroup. Will also
//                  be altered according to any collision response.
//                  The current rotation Matrix of the AtomGroup. Will be altered
//                  accordingly as travel happens.
//                  The current desired angular velocity of the owner MO, in r/s. Will
//                  be altered.
//                  A bool that will be set to whether the position change involved a
//                  wrapping of the scene or not.
//                  A float to be filled out with the total magnitudes of all the forces
//                  exerted on this through collisions during this frame.
//                  The designated mass of the AtomGroup at this time.
//                  The amount of time in s that this Atom is supposed to travel.
//                  Whether to call the parent MO's OnBounce function upon bouncing
//                  against anything, or not.
//                  Whether to call the parent MO's OnSink function upon sinking into
//                  against anything, or not.
//                  Whether the Scene has been pre-locked or not.
// Return value:    The amount of time remaining of the travelTime passed in, in s. This
//                  may only possibly be a nonzero if callOnBounce or callOnSink are true.

    virtual float Travel(Vector &position,
                         Vector &velocity,
                         Matrix &rotation,
                         float &angVel,
                         bool &didWrap,
                         Vector &totalImpulse,
                         float mass,
                         float travelTime,
                         bool callOnBounce = false,
                         bool callOnSink = false,
                         bool scenePreLocked = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PushTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together without rotations and react
//                  with the scene by pushing against it.
// Arguments:       A reference to a Vector with the starting position of the AtomGroup
//                  origin. The value of this Vector will be altered accoring to where
//                  the AtomGroup ends up after done travelling.
//                  A Vector with the total desired velocity of the AtomGroup. Will not be
//                  altered.
//                  The maximum force that the push against other stuff in the scene can
//                  have, in kg * m/s^2.
//                  A bool that will be set to whether the position change involved a
//                  wrapping of the scene or not.
//                  The amount of time in s that this Atom is supposed to travel.
//                  Whether to call the parent MO's OnBounce function upon bouncing
//                  against anything, or not.
//                  Whether to call the parent MO's OnSink function upon sinking into
//                  against anything, or not.
//                  Whether the Scene has been pre-locked or not.
// Return value:    The resulting push impulse force is reported back through here, in\
//                  kg * m/s.

    virtual Vector PushTravel(Vector &position,
                              Vector velocity,
                              float pushForce,
                              bool &didWrap,
                              float travelTime,
                              bool callOnBounce = false,
                              bool callOnSink = false,
                              bool scenePreLocked = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PushAsLimb
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together as a pushing entity,
//                  relative to the position of the owning MovableObject. If stuff in the
//                  scene is hit, resulting forces are be added to the owning
//                  MovableObject's impulse forces.
// Arguments:       A reference to a Vector with the world position of the joint of the
//                  of the path along which this AtomGroup is being pushed.
//                  A Vector with the velocity of the owning MovableObject.
//                  A Matrix with the rotation of the owning movableObject.
//                  The vector that the resulting impulse forces of this push will be
//                  added to. IE this will be altered according to any collision response.
//                  A LimbPath which this AtomGroup should travel along.
//                  The amount of time in s that this Atom is supposed to travel.
//                  Pointer to a bool which gets set to true if the limbpath got restarted
//                  during this push. It does NOT get initalized to false!
//                  Whether the forces created by this should have rotational leverage on
//                  the owner, or only have translational effect.
// Return value:    Whether the LimbPath passed in could start free of terrain or not.

    virtual bool PushAsLimb(const Vector &jointPos,
                            const Vector &velocity,
                            const Matrix &rotation,
                            LimbPath &limbPath,
                            const float travelTime,
                            bool *pRestarted = 0,
                            bool affectRotation = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlailAsLimb
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the group of Atom:s travel together as a lifeless limb, constrained
//                  to a radius around the joint pin in the center.
// Arguments:       A Vector with the world position of the owner MovableObject.
//                  A Vector with the rotated offset of the joint that this should flail
//                  around in a radius.
//                  The radius/range of the limb this is to simulate.
//                  A Vector with the velocity of the owning MovableObject.
//                  A float with the angular velocity in rad/s of the owning MovableObject.
//                  The mass of this dead weight limb.
//                  The amount of time in s that this Atom is supposed to travel.
// Return value:    None.

    virtual void FlailAsLimb(const Vector ownerPos,
                             const Vector jointOffset,
                             const float limbRadius,
                             const Vector velocity,
                             const float angVel,
                             const float mass,
                             const float travelTime);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          InTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this AtomGroup are on top of
//                  terrain pixels.
// Arguments:       None.
// Return value:    Whether any Atom of this AtomGroup is on top of a terrain pixel.

    virtual bool InTerrain();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RatioInTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the ratio of how many atoms of this group that are on top of
//                  intact terrain pixels.
// Arguments:       None.
// Return value:    The ratio of atoms on top of terrain pixels, form 0 to 1.0

    virtual float RatioInTerrain();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResolveTerrainIntersection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this AtomGroup are on top of
//                  terrain pixels, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.
// Arguments:       Current position and rotation of the owner MO.
//                  Only consider materials stronger than this specific one.
// Return value:    Whether any intersection was successfully resolved. Will return true
//                  even if there wasn't any intersections to begin with.

    virtual bool ResolveTerrainIntersection(Vector &position, Matrix &rotation, unsigned char strongerThan = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResolveMOSIntersection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this AtomGroup are on top of
//                  other MOSprite:s, and if so, attempt to move the OwnerMO of this out
//                  so none of this' Atoms are inside any other MOS's silhouette anymore.
// Arguments:       Current positiona and rotation of the owner MO.
//                  The number of attempts allowed to clear intersections with MO's. Each
//                  move results in a counted attempt.
// Return value:    Whether all intersections were successfully eliminated in the alloted
//                  number of attempts.

    virtual bool ResolveMOSIntersection(Vector &position, Matrix &rotation);


	/// <summary>
	/// Create and add an atom to this AtomGroup's list of atoms, m_Atoms.
	/// </summary>
	/// <param name="x">X coordinate in the sprite frame.</param>
	/// <param name="y">Y coordinate in the sprite frame.</param>
	/// <param name="spriteOffset">Sprite offset relative to pOwnerMOSRotating.</param>
	/// <param name="pOwnerMOSRotating">MOSRotating whose outline will be approximated by Atoms of this AtomGroup.</param>
	/// <param name="calcNormal">Whether to set a normal for the atom. Should be true for surface atoms.</param>
	void AddAtomToGroup(int x, int y, const Vector &spriteOffset, MOSRotating *pOwnerMOSRotating, bool calcNormal);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AtomGroup's current graphical debug representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Whether to use the limb position of this AtomGroup, or the owner's
//                  position.
//                  The color to draw the atoms' pixels as.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      bool useLimbPos = false,
                      unsigned char color = 34) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AtomGroup, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    static Entity::ClassInfo m_sClass;

    // Whether or not the Atom:s were automatically generated based on a sprite, or manually defined
    bool m_AutoGenerate;
	// Material of this AtomGroup
    Material const * m_pMaterial;
    // Resolution, or density of the Atoms in representing a MOSRotating's silhouette.
    // 0 means the Atom config wasn't genereated by a silhouette, but defined manually.
    int m_Resolution;
    // Depth, or how deep into the bitmap of the owning MO's graphical representation
    // the Atom:s of this AtomGroup are located, in pixels.
    int m_Depth;
    // List of Atoms that constitute the group. Owned by this
    std::list<Atom *> m_Atoms;
    // Sub groupings of atoms, not owned in here. Point to atoms owned in m_Atoms.
	std::unordered_map<long int, std::list<Atom *> > m_SubGroups;
    // Moment of Inertia for this AtomGroup
    float m_MomInertia;
    // The owner of this AtomGroup. The owner is obviously not owned by this AtomGroup.
    MOSRotating *m_pOwnerMO;
    // The last position of this AtomGroup when used as a limb.
    Vector m_LimbPos;
    // The offset of the wrist/ankle relative to this AtomGroup's
    // origin when used as a limb.
    Vector m_JointOffset;
	// ignore hits with MOs of these IDs
	std::list<MOID> m_IgnoreMOIDs;

// TODO: REMOVE THIS")
    Vector m_TestPos;

};

} // namespace RTE

#endif // File
