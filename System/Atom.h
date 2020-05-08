#ifndef _RTEATOM_
#define _RTEATOM_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Atom.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Atom class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"
#include "Vector.h"
#include "Matrix.h"
#include "Material.h"
#include "LimbPath.h"
#include "Color.h"

#include "ConsoleMan.h"
#include "SceneMan.h"

namespace RTE
{

class SLTerrain;
class MovableObject;

enum { HITOR = 0, HITEE = 1 };
enum { NormalCheckCount = 16 };


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A point (pixel) that tests for collisions with a BITMAP's drawn
//                  pixels, ie not the key color. Owned and operated by other objects.
// Parent(s):       Serializable.
// Class history:   02/22/2001  Atom created.

class Atom:
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:
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
		virtual void Reset() { Clear(); }

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


    // New and delete operator overloads for the pool allocation of Atom:s
    static void * operator new (size_t size) { return Atom::GetPoolMemory(); }
    static void operator delete (void *pInstance) { Atom::ReturnPoolMemory(pInstance); }


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   FillPool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain number of newly allocated instances to this' pool.
// Arguments:       The number of instances to fill er up with.If 0 is specified, the set
//                  refill amount will be used.
// Return value:    None.

    static void FillPool(int fillAmount = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   GetPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Grabs from the pre-allocated pool, an available chunk of memory the
//                  exact size of an Atom. OWNERSHIP IS TRANSFERRED!
// Arguments:       None.
// Return value:    A pointer to the pre-allocated pool memory. OWNERSHIP IS TRANSFERRED!

    static void * GetPoolMemory();


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   ReturnPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a raw chunk of memory back to the pre-allocated available pool.
// Arguments:       The raw chunk of memory that is being returned. Needs to be the same
//                  size as an Atom. OWNERSHIP IS TRANSFERRED!
// Return value:    The count of outstanding memory chunks after this was returned.

    static int ReturnPoolMemory(void *pReturnedMemory);


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Atom object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Atom() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Convenience constructor to both instantiate a Atom in memory and
//                  and Create it at the same time.
// Arguments:       See Create() below.

    Atom(const Vector &offset,
         unsigned char materialID,/*Material *material,*/
         MovableObject *owner,
         Color trailColor = Color(),
         int trailLength = 0)
    {
        Clear();
		Create(offset, g_SceneMan.GetMaterialFromID(materialID), owner, trailColor, trailLength);
    }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Constructor:     Atom
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Convenience constructor to both instantiate a Atom in memory and
	//                  and Create it at the same time.
	// Arguments:       See Create() below.

	Atom(const Vector &offset,
		Material const *material,
		MovableObject *owner,
		Color trailColor = Color(),
		int trailLength = 0)
	{
		Clear();
		Create(offset, material, owner, trailColor, trailLength);
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Atom object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Atom() { Destroy(true); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Atom object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Atom object ready for use.
// Arguments:       An offset Vector that will be used to offset collision calculations.
//                  A Material that defines what material this Atom is made of.
//                  The owner MovableObject of this Atom. Ownership is NOT transferred!
//                  The trail color.
//                  The trail length. If 0, no trail will be drawn.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Vector &offset,
                       Material const *material,
                       MovableObject *owner,
                       Color trailColor = Color(),
                       int trailLength = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Atom to be identical to another, by deep copy.
// Arguments:       A reference to the Atom to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Atom &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Atom, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Atom to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Atom will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Atom object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Atom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Atom object
//                  identical to an already existing one.
// Arguments:       A Atom object which is passed in by reference.

    Atom(const Atom &reference) { if (this != &reference) { Clear(); Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Atom assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Atom equal to another.
// Arguments:       A Atom reference.
// Return value:    A reference to the changed Atom.

    Atom & operator=(const Atom &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateNormal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the normal of this atom, based on its position on a sprite's
//                  bitmap. It will check piels around it and see if they are inside the
//                  object or not, and infer a collision normal based on that.
//                  THIS ONLY WORKS IF THE ATOM IS ON THE SURFACE OF THE SPRITE!
// Arguments:       The bitmap to check against. Ownership IS NOT transferred!
//                  Where the on the bitmap the center of the object is. This atom's offset
//                  will be applied automatically before checking for it normal.
// Return value:    Whether normal was successfully derived from the bitmap (if not, then
//                  a provisional one is derived from the offset).

    bool CalculateNormal(BITMAP *pSprite, Vector spriteCenter);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset vector.
// Arguments:       None.
// Return value:    The current offset Vector.

    const Vector & GetOffset() const { return m_Offset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOriginalOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset vector that was first set for this Atom. The GetOffset
//                  may have additional offsets baked into it if this is part of an group.
// Arguments:       None.
// Return value:    The original offset Vector.

    const Vector & GetOriginalOffset() const { return m_OriginalOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNormal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the surface normal of this vector, if it has been successfully
//                  calculated. If not, it'll be a 0 vector.
// Arguments:       None.
// Return value:    The current normalized surface normal Vector of this.

    const Vector & GetNormal() const { return m_Normal; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the material of this Atom.
// Arguments:       None.
// Return value:    The the material of this Atom.

	Material const * GetMaterial() const { if (m_pMaterial) return m_pMaterial; else return g_SceneMan.GetMaterialFromID(g_MaterialAir); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSubID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the group ID of this Atom.
// Arguments:       None.
// Return value:    The the group ID of this Atom.

    long int GetSubID() const { return m_SubgroupID; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDensity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the kg/pixel density of this Atom.
// Arguments:       None.
// Return value:    The density in kg/pixel of this Atom.

    float GetDensity() const { return g_SceneMan.GetMaterial(m_pMaterialId).density; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetElasticity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the restitution scalar.
// Arguments:       None.
// Return value:    The current float restitution scalar.

    float GetElasticity() const { return g_SceneMan.GetMaterial(m_pMaterialId).restitution; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFriction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the friction scalar.
// Arguments:       None.
// Return value:    The current float friction scalar.

    float GetFriction() const { return g_SceneMan.GetMaterial(m_pMaterialId).friction; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute current position of this Atom, in scene coords.
//                  This includes the rotated offset, and any steps that may have been
//                  taken on a seg.
// Arguments:       None.
// Return value:    The current position of the Atom, with the offset baked in.

    Vector GetCurrentPos() const { return Vector(m_IntPos[X], m_IntPos[Y]); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEndPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the end position result from a call to Travel() or Feel().
// Arguments:       None.
// Return value:    The latest end position result Vector.

    const Vector & GetEndPos() const { return m_ResultPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRestTrajectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rest trajectory result from a call to Travel() or Feel().
// Arguments:       None.
// Return value:    The latest rest trajectory result Vector.

    const Vector & GetRestTrajectory() const { return m_RestTrajectory; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBounceAcc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bounce acceleration result from a call to Travel() or Feel().
// Arguments:       None.
// Return value:    The latest bounce acceleration result Vector.

    const Vector & GetBounceAcc() const { return m_ResultAcc; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current owner MovableObject of this AtomGroup.
// Arguments:       None.
// Return value:    A const pointer to the owner.

    const MovableObject * GetOwner() const { return m_pOwnerMO; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Atom is set to collide with MovableObject:s.
// Arguments:       None.
// Return value:    Whether or not this Atom is checking for collisions with MOs or not.

    bool HitsMOs() const { return m_HitsMOs; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTrailColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Color of this Atom's trail.
// Arguments:       None.
// Return value:    A Color object describing the trail color.

    Color GetTrailColor() const { return m_TrailColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTrailLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the longest a trail can be drawn, in pixels.
// Arguments:       None.
// Return value:    The new max length, in pixels. If 0, no trail is drawn.

    int GetTrailLength() { return m_TrailLength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStepRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the normalized ratio of how many steps are actually taken to how
//                  many calls to TakeStep are made.
// Arguments:       None.
// Return value:    A normalized float describing the step ratio.

    float GetStepRatio() { return m_StepRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHitData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the stored data struct on the last collision expereinced by this
//                  Atom.
// Arguments:       None.
// Return value:    The Vector describing the velocity of this Atom at the last time it
//                  hit something.

    HitData & GetHitData() { return m_LastHit; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHitVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the stored velocity from the last time this Atom hit something.
// Arguments:       None.
// Return value:    The Vector describing the velocity of this Atom at the last time it
//                  hit something.

    Vector & GetHitVel() { return m_HitVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHitRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the saved rotated radius / offset from the Body's center of mass
//                  at the time of a collision. This is in METERS, not pixels!
// Arguments:       None.
// Return value:    A Vector specifying the rotated hit offset vector from the last hit.

    Vector & GetHitRadius() { return m_HitRadius; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHitImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the saved collision response impulse force vector from the last
//                  collision.
// Arguments:       None.
// Return value:    A Vector specifying the collision response impulse vector in Kg * m/s.

    Vector & GetHitImpulse() { return m_HitImpulse; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNumPenetrations
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of consecutive penetrations of Terrain that this Atom
//                  has sucessfully made, ending with wherever it is now. Resets to 0 as
//                  soon as penetration streak ends.
// Arguments:       None.
// Return value:    The number of consecutive penetrations.

    int GetNumPenetrations() const { return m_NumPenetrations; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStepsLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates how many more steps remain to be taken to traverse the
//                  entire trajectory seg.
// Arguments:       None.
// Return value:    The number of steps that remain to be taken on the set traj seg.

    int GetStepsLeft() { return m_Delta[m_Dom] - m_DomSteps; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSegTraj
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the seg trajectory currently set by SetupSeg
// Arguments:       None.
// Return value:    The currently traversed seg trajectory.

    Vector GetSegTraj() { return m_SegTraj; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSegLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates the total original length of the current seg set by SetupSeg 
// Arguments:       None.
// Return value:    The length of the currently traversed seg.

    float GetSegLength() { return m_SegTraj.GetMagnitude(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new offset vector for the collision calculations.
// Arguments:       A const reference to a Vector that will be used as offset.
// Return value:    None.

    void SetOffset(const Vector &newOffset) { m_Offset = newOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the material of this Atom.
// Arguments:       The new material of this Atom.
// Return value:    None.

    void SetMaterial(Material *newMat) { m_pMaterial = newMat; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSubID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the subgroup ID of this Atom
// Arguments:       The new subgroup id of this Atom
// Return value:    None.

    void SetSubID(long int newID = 0) { m_SubgroupID = newID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current owner MovableObject of this AtomGroup.
// Arguments:       A pointer to the new owner. Ownership is NOT transferred!
// Return value:    None.

    void SetOwner(MovableObject *newOwner) { m_pOwnerMO = newOwner; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this Atom will collide with MovableObject:s or not.
// Arguments:       Whether or not this Atom should check for collisions with MOs or not.
// Return value:    None.

    void SetToHitMOs(bool hitMOs) { m_HitsMOs = hitMOs; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTrailColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the color value of this Atom's trail.
// Arguments:       A Color object specifying the new trail color.
// Return value:    None.

    void SetTrailColor(Color newTrailColor) { m_TrailColor = newTrailColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTrailLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the losgest a trail can be drawn, in pixels.
// Arguments:       The new max length, in pixels. If 0, no trail is drawn.
// Return value:    None.

    void SetTrailLength(const int trailLength) { m_TrailLength = trailLength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStepRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ratio of how many steps are actually taken to how many
//                  calls to TakeStep are made.
// Arguments:       A float specifying the new step ratio.
// Return value:    None.

    void SetStepRatio(float newStepRatio) { m_StepRatio = newStepRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the HitData struct this Atom uses to represent the last hit it
//                  experienced.
// Arguments:       A reference to a HitData struct that will be copied to the Atom's.
// Return value:    None.

    void SetHitData(const HitData &newHitData) { m_LastHit = newHitData; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the saved velocity that this Atom supposedly hit something with.
// Arguments:       A Vector specifying the hit velocity to store.
// Return value:    None.

    void SetHitVel(const Vector &newHitVel) { m_HitVel = newHitVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the saved rotated radius / offset from the Body's center of mass
//                  at the time of a collision.
// Arguments:       A Vector specifying the hit offset vector to store, in METERS, not
//                  pixels!
// Return value:    None.

    void SetHitRadius(const Vector &newHitRad) { m_HitRadius = newHitRad; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the saved collision response impulse force vector from the last
//                  collision.
// Arguments:       A Vector specifying the collision response impulse vector in Kg * m/s.
// Return value:    None.

    void SetHitImpulse(const Vector &newHitImp) { m_HitImpulse = newHitImp; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangedDir
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells this Atom that the previous travel move's fractional error isn't
//                  valid for consecutive travel moves.
// Arguments:       None.
// Return value:    None.

    void ChangedDir() { m_ChangedDir = true; }

/* Obsolete
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetElasticty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the elasticty of this point. This value decides how much kinetic
//                  energy is left in the velocity vector after a collision with terrain.
// Arguments:       A float scalar value that defines the restitution. 1.0 = no kinetic
//                  energy is lost in a collision, 0.0 = all energy is lost (plastic).
// Return value:    None.

    void SetElasticty(float newValue = 0.5) { m_Elasticity = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFriction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the friction of this point. This value decides how much velocity
//                  is lost in the direction of travel when a collision occurs.
// Arguments:       A float scalar value that defines the friction. 1.0 = will snag onto
//                  everything, 0.0 = will glide with no friction.
// Return value:    None.

    void SetFriction(float newValue = 0.5) { m_Friction = newValue; }
*/
/* Obsolete, now done with SceneMan::GetMaterial()
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a coordinate on the terrain is filled with somehting or not.
// Arguments:       The integer 2D coordnates.
// Return value:    A bool telling if there is something there or not (air).

    bool HitCheck(BITMAP *pHitBitmap,
                  int checkX,
                  int checkY) const { return pHitBitmap->GetPixel(checkX, checkY) != pHitBitmap->GetColorKey(); }

    bool HitCheck(BITMAP *pHitBitmap,
                  int checkX,
                  int checkY) const { return pHitBitmap->GetPixel(checkX + 200, checkY + 200) != pHitBitmap->GetColorKey(); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMOIDToIgnore
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MOID that this Atom should ignore collisions with during its
//                  next travel sequence.
// Arguments:       The MOID to add to the ignore list.
// Return value:    None.

    void AddMOIDToIgnore(MOID ignore) { m_IgnoreMOIDs.push_back(ignore); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDIgnoreList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clear the list of MOIDs that this Atom is set to ignore collisions
//                  with during its next travel sequence. This should be done each frame
//                  so that fresh MOIDs can be re-added. (MOIDs are only valid during a
//                  frame)
// Arguments:       None.
// Return value:    None.

    void ClearMOIDIgnoreList() { m_IgnoreMOIDs.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is set to ignore collisions with a MO of a
///                 specific MOID.
// Arguments:       The MOID to check if it is ignored.
// Return value:    Whether or not this MOID is being ignored.

    bool IsIgnoringMOID(MOID which);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringAllMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is temporarily ignoring all MO hits
// Arguments:       None.
// Return value:    Whether or not this is ignoring all MO hits.

    bool IsIgnoringAllMOs() { return m_MOHitsDisabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsIgnoringTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether this Atom is set to ignore collisions with the terrain.
// Arguments:       None.
// Return value:    Whether or not this is ignoring hits the with terrain.

    bool IsIgnoringTerrain() { return m_TerrainHitsDisabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Atom's initial position up for a straight seg of a trajectory
//                  to step through. This is to be done before SetupSeg. It will report
//                  whether the atom's position is inside the terrain or another MO.
//                  It also disables this' terrain and/or MO collisions when travelling,
//                  until the travel encounters an air/noMO pixel again.
// Arguments:       A Vector with the position to start from, in scene coords.
// Return value:    If the start pos is already intersecting the terrain or an MO, then
//                  this will return true. Do collision response as needed.

    bool SetupPos(Vector startPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Atom up for a straight seg of a trajectory to step through.
//                  If this Atom find the startPos to be on an MO, it will ignore any
//                  collisions with that MO for the entire seg. The Scene MUST BE LOCKED
//                  before calling this!
// Arguments:       A Vector with the position to start from, in scene coords.
//                  A Vector with the trajectory to travel during this next seg.
//                  The ratio between the actual steps to be taken and the future calls
//                  to TakeStep().
// Return value:    The total number of steps this will take in the newly set-up segment.

    int SetupSeg(Vector startPos, Vector trajectory, float stepRatio = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StepForward
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes one step along the trajectory seg set up by SetupSeg(). The
//                  Scene MUST BE LOCKED before calling this!
// Arguments:       The number of steps to take.
// Return value:    Whether anything was hit during the steps or not. MO hits will only
//                  be reported if this Atom is set to hit them, and the MO hit is not
//                  ignored by this Atom (if the seg started on that MO).
//                  Also if terrain hits are temporarily disabled, they will not be reported.

    bool StepForward(int numSteps = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StepBack
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes one step back, or undos the step, if any, previously taken along
//                  the trajectory seg set up by SetupSeg(). CAUTION: The previous HitWhat
//                  vars are not reset to what they previously were!
// Arguments:       None.
// Return value:    None.

    void StepBack();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the ID of the MO hit at the previously taken step by TakeStep
//                  This will only potentially return non-g_NoMOID if this Atom is set to
//                  hit MO's and the MO hit isn't marked to be ignored.
// Arguments:       None.
// Return value:    The ID of the non-ignored MO, if any, that this Atom is now
//                  intersecting because of the last step taken.

    MOID HitWhatMOID() { return m_MOIDHit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatTerrMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the terrain material the previously taken step by TakeStep
//                  hit, if any.
// Arguments:       None.
// Return value:    The ID of the material, if any, that this MO hit during the last step.

    unsigned char HitWhatTerrMaterial() { return m_TerrainMatHit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MOHitResponse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision reponse with another MO, if the last step
//                  taken forward resulted in an intersection with a non-ignored MO. Note
//                  that one step backward should be taken after the intersecting step so
//                  that the Atom is not intersecting the hit MO anymore. The hit
//                  response is calculated and appropriate forces are applied to the hit
//                  MO and the impulse force imposed on the owner MO are returned, and
//                  also stored in this Atom to be later retrieved with GetHitImpulse().
// Arguments:       None. The collision data should have already been set by SetHitData().
// Return value:    Whether this collision is valid and should be considered further.

    bool MOHitResponse();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TerrHitResponse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision reponse with the Terrain, if the last step
//                  taken forward resulted in an intersection with the Terrain. Note
//                  that one step backward should be taken after the intersecting step so
//                  that the Atom is not intersecting the Terrain anymore, and the hit
//                  response calculations can be made accurately. The resulting impulse
//                  vector is also stored in this Atom to be later retrieved with
//                  GetHitImpulse().
// Arguments:       None. The collision data should have already been set by SetHitData().
// Return value:    The resulting HitData of this Atom with all the information about the
//                  collision filled out.

    HitData & TerrHitResponse();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses the current state of the owning MovableObject to determine if
//                  if there are any collisions in the path of its travel during this
//                  frame, and if so, apply all collision responses to the MO.
// Arguments:       The amount of time in s that this Atom is allowed to travel.
//                  A bool specifying if the end position result should be moved along the
//                  trajectory if no terrain is hit.
//                  Whether the Scene has been pre-locked or not.
// Return value:    The number of hits against terrain that were made during the travel.

    int Travel(float travelTime,
               bool autoTravel = true,
               bool scenePreLocked = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIgnoreMOIDsByGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     AtomGroup may set this shared list of ignored MOIDs to avoid setting and 
//					removing ignored MOIDs for every atom one by one. The list is maintained only by
//					AtomGroup, Atom never owns it.
// Arguments:       New MOIDs list to ignore.
// Return value:    None.

	void SetIgnoreMOIDsByGroup(std::list<MOID> const * ignoreMOIDsByGroup) { m_pIgnoreMOIDsByGroup = ignoreMOIDsByGroup; };

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static const std::string ClassName;

    // Pool of pre-allocated Atom:s
    static std::vector<void *> m_AllocatedPool;
    // The number of instances to fill up the pool of Atom;s with each time it runs dry
    static int m_PoolAllocBlockCount;
    // The number of allocated instances passed out from the pool
    static int m_InstancesInUse;

    // This forms a circle around the Atom's offset center, to check for key color pixels in order to determine the normal at the Atom's position
    static const int m_sNormalChecks[NormalCheckCount][2];
    Vector m_Offset;
    // This offset is before altering the m_Offset for use in composite groups
    Vector m_OriginalOffset;
    Vector m_Normal;
    Material const * m_pMaterial;
    // Identifying ID for adding and removing atoms from AtomGroups
    int m_SubgroupID;

    // Bresenham line algo vars
    int m_IntPos[2];
    int m_PrevIntPos[2];
    int m_TrailPos[2];
    int m_HitPos[2];
    int m_Delta[2];
    int m_Delta2[2];
    int m_Increment[2];
    int m_Error, m_Dom, m_Sub, m_DomSteps, m_SubSteps;
    bool m_SubStepped;
    // Whether the last call to StepForward actually resulted in a step or not.
    bool m_StepWasTaken;
    float m_StepRatio;
    Vector m_SegTraj;
    float m_SegProgress;

    // Temporary disabling of terrain collisions for this. Will be re-enabled once out of terrain again.
    bool m_MOHitsDisabled;
    // Temporary disabling of terrain collisions for this. Will be re-enabled once out of terrain again.
    bool m_TerrainHitsDisabled;

    // The owner of this Atom. The owner is obviously not owned by this Atom.
    MovableObject *m_pOwnerMO;
    // Whether or not this Atom hits MO:s or not * this is now stored in the MOs *
//    bool m_HitsMOs;
    // Special ignored MOID
    MOID m_IgnoreMOID;
    // ignore hits with MOs of these IDs
    std::list<MOID> m_IgnoreMOIDs;
	// Also ignore hits with MOs of these IDs. This one may be set externally by atom group
	std::list<MOID> const * m_pIgnoreMOIDsByGroup;

    // The MO, if any, this Atom hit on the last step.
    MOID m_MOIDHit;
    // The terrain material, if any, this Atom hit on the last step.
    unsigned char m_TerrainMatHit;

    // Data containing information on the last collision experienced by this Atom.
    HitData m_LastHit;
/*
    // The velocity at which this atom last hit something.
    Vector m_HitVel;
    // The rotated offset or radius from the body/AtomGroup's CoM
    // at the time of the last collision this Atom experienced.
    // Observe that this is in meters, not pixels!
    Vector m_HitRadius;
    // The resulting impulse force of the last collision this Atom experienced
    // In Kg * m/s.
    Vector m_HitImpulse;
*/
    // Trail color
    Color m_TrailColor;
    // The longest the trail should/can get drawn. If 0, no trail is drawn.
    int m_TrailLength;
    // Counts consecutive penetrations in a row.
    // Resets to 0 as soon as penetration streak ends.
    int m_NumPenetrations;
    // This is only true if there was a change in direction of trajectory
    // during the last travel move of this Atom.
    bool m_ChangedDir;
    // This is only true when the resulting position reflects a wrap around the scene.
    bool m_ResultWrapped;
    // This is the stored error (fraction) at the end of the last travel move.
    // To be used when the direction wasn't changed during a hit, and will make
    // sure the continued trajectory is straight.
    int m_PrevError;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Atom, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File
