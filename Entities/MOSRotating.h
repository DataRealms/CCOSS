#ifndef _RTEMOSROTATING_
#define _RTEMOSROTATING_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSRotating.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MOSRotating class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSprite.h"

namespace RTE
{

class AtomGroup;
struct HitData;
class AEmitter;
class Attachable;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A sprite movable object that can rotate.
// Parent(s):       MOSprite.
// Class history:   05/30/2002 MOSRotating created.

class MOSRotating:
    public MOSprite
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


friend class AtomGroup;
friend class SLTerrain;
friend class LuaMan;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    Gib
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Something to bundle the properties of Gib piece together.
    // Parent(s):       Serializable.
    // Class history:   10/24/2006 Gib created.

    class Gib:
        public Serializable
    {

    friend class GibEditor;
    friend class TDExplosive;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Gib
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Gib object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Gib() { Clear(); }

/*
    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Gib object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create();
*/

    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a Gib to be identical to another, by deep copy.
    // Arguments:       A reference to the Gib to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(const Gib &reference);


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
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        virtual void Reset() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Save
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Saves the complete state of this Gib to an output stream for
    //                  later recreation with Create(Reader &reader);
    // Arguments:       A Writer that the Gib will save itself with.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Save(Writer &writer) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetClassName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the class name of this Entity.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted type name of this object.

        virtual const std::string & GetClassName() const { return m_sClassName; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetParticlePreset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the reference particle to be used as a gib. Owenership is NOT transferred!
    // Arguments:       None.
    // Return value:    A pointer to the particle to be emitted. Not transferred!

        virtual const MovableObject * GetParticlePreset() { return m_pGibParticle; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetOffset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the spawn offset from the parent's position.
    // Arguments:       None.
    // Return value:    The offset in pixels from the parent's position where this gets spawned.

        virtual Vector GetOffset() const { return m_Offset; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetCount
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the number of emissions to make of this emission type in a burst.
    // Arguments:       None.
    // Return value:    The number of emissions there should be of this type in an emission.

        virtual int GetCount() const { return m_Count; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetSpread
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the angle spread of velocity of the emitted MO's to each side of
    //                  the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
    //                  one side only, with the m_Rotation defining the middle of that half circle.
    // Arguments:       None.
    // Return value:    The emission spread in radians.

        virtual float GetSpread() const { return m_Spread; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetMinVelocity
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the specified minimum velocity an emitted MO can have when emitted.
    // Arguments:       None.
    // Return value:    The min emission velocity in m/s.

        virtual float GetMinVelocity() const { return DMin(m_MinVelocity, m_MaxVelocity); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetMaxVelocity
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the specified maximum velocity an emitted MO can have when emitted.
    // Arguments:       None.
    // Return value:    The max emission velocity in m/s.

        virtual float GetMaxVelocity() const { return DMax(m_MinVelocity, m_MaxVelocity); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetLifeVariation
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the specified variation in lifetime of the gibbed object.
    // Arguments:       None.
    // Return value:    The life variation rationally expressed.. 0.1 = up to 10% varitaion.

        virtual float GetLifeVariation() const { return m_LifeVariation; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  InheritsVel
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows whether this's gibs should inherit the velocity of the gibbing
    //                  parent.
    // Arguments:       None.
    // Return value:    Whetehr this inherits velocity or not.

        virtual bool InheritsVel() const { return m_InheritsVel; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // Member variables
        static const std::string m_sClassName;
        // The pointer to the preset instance, that copies of which will be emitted. Not Owned
        const MovableObject *m_pGibParticle;
        // Offset spawn position from owner/parent's position
        Vector m_Offset;
        // The number of emissions of this type should be emitted
        int m_Count;
        // The angle spread of velocity of the emitted MO's to each
        // side of the m_EmitAngle angle. in radians.
        // PI/2 would mean that MO's fly out to one side only, with the
        // m_Rotation defining the middle of that half circle.
        float m_Spread;
        // The minimum velocity an emitted MO can have when emitted
        float m_MinVelocity;
        // The maximum velocity an emitted MO can have when emitted
        float m_MaxVelocity;
        // The per-gib variation in life time, in percentage of the existing life time of the gib
        float m_LifeVariation;
        // Whether this gib should inherit the velocity of the exploding parent or not
        bool m_InheritsVel;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Gib, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


// Concrete allocation and cloning definitions
ENTITYALLOCATION(MOSRotating)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MOSRotating object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MOSRotating() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MOSRotating object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~MOSRotating() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSRotating object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSRotating object ready for use.
// Arguments:       A pointer to ContentFile that represents the bitmap file that will be
//                  used to create the Sprite.
//                  The number of frames in the Sprite's animation.
//                  A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(ContentFile spriteFile,
                       const int frameCount = 1,
                       const float mass = 1,
                       const Vector &position = Vector(0, 0),
                       const Vector &velocity = Vector(0, 0),
                       const unsigned long lifetime = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOSRotating to be identical to another, by deep copy.
// Arguments:       A reference to the MOSRotating to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const MOSRotating &reference);


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
// Description:     Resets the entire MOSRotating, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); MOSprite::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MOSRotating to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the MOSRotating will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACDropShip, including the mass of its
//                  currently attached body parts and inventory.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current AtomGroup of this MOSRotating.
// Arguments:       None.
// Return value:    A const reference to the current AtomGroup.

    AtomGroup * GetAtomGroup() { return m_pAtomGroup; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main Material  of this MOSRotating.
// Arguments:       None.
// Return value:    The the Material of this MOSRotating.

    virtual Material const * GetMaterial() const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during travel.
// Arguments:       None.
// Return value:    Whether to hit other MO's during travel, or not.

    virtual bool HitsMOs() const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.
// Arguments:       None.
// Return value:    The the priority  of this MovableObject. Higher number, the higher
//                  priority.

    virtual int GetDrawPriority() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRecoilForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current recoil impulse force Vector of this MOSprite.
// Arguments:       None.
// Return value:    A const reference to the current recoil impulse force in kg * m/s.

    const Vector & GetRecoilForce() const { return m_RecoilForce; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRecoilOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current recoil offset Vector of this MOSprite.
// Arguments:       None.
// Return value:    A const reference to the current recoil offset.

    const Vector & GetRecoilOffset() const { return m_RecoilOffset; }
/*
// TODO: Improve this one! Really crappy fit
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetBoundingBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the oriented bounding box which is guaranteed to contain this,
//                  taking rotation etc into account. It's not guaranteed to be fit
//                  perfectly though. TODO: MAKE FIT BETTER
// Arguments:       None.
// Return value:    A Box which is guaranteed to contain this. Does nto take wrapping into
//                  account, and parts of this box may be out of bounds!

    virtual Box GetBoundingBox() const { return Box(m_Pos + Vector(-m_MaxRadius, -m_MaxRadius), m_MaxDiameter, m_MaxDiameter); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGibList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets direct access to the list of object this is to generate upon gibbing.
// Arguments:       None.
// Return value:    A pointer to the list of gibs. Ownership is NOT transferred!

    std::list<Gib> * GetGibList() { return &m_Gibs; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the current Atom of this MOSRotating with a new one.
// Arguments:       A reference to the new Atom.
// Return value:    None.

    void SetAtom(Atom *newAtom);
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.
// Arguments:       Whether to hit other MO's during travel, or not.
// Return value:    None.

    virtual void SetToHitMOs(bool hitMOs = true);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is made of Gold or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Gold or not.

    virtual bool IsGold() const { return m_MOType == TypeGeneric && GetMaterial()->id == GOLDMATID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddRecoil
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds graphical recoil offset to this MOSprite according to its angle.
// Arguments:       None.
// Return value:    None.

    void AddRecoil();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRecoil
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds recoil offset to this MOSprite.
// Arguments:       A vector with the recoil impulse force in kg * m/s.
//                  A vector with the recoil offset in pixels.
//                  Whether recoil should be activated or not for the next Draw().
// Return value:    None.

    void SetRecoil(const Vector &force, const Vector &offset, bool recoil = true)
    {
        m_RecoilForce = force;
        m_RecoilOffset = offset;
        m_Recoiled = recoil;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsRecoiled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this MOSprite is currently under the effects of
//                  recoil.
// Arguments:       None.
// Return value:    None.

    bool IsRecoiled() { return m_Recoiled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableDeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this MOSRotating should check for deep penetrations
//                  the terrain or not.
// Arguments:       Whether to enable deep penetration checking or not.
// Return value:    None.

    void EnableDeepCheck(const bool enable = true) { m_DeepCheck = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceDeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets to force a deep checking of this' silhouette against the terrain
//                  and create an outline hole in the terrain, generating particles of the
//                  intersecting pixels in the terrain.
// Arguments:       Whether to force a deep penetration check for this sim frame or not..
// Return value:    None.

    void ForceDeepCheck(const bool enable = true) { m_ForceDeepCheck = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.
// Arguments:       Reference to the HitData struct which describes the collision. This
//                  will be modified to represent the results of the collision.
// Return value:    Whether the collision has been deemed valid. If false, then disregard
//                  any impulses in the Hitdata.

    virtual bool CollideAtPoint(HitData &hitData);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this bounce.

    virtual bool OnBounce(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this sinkage.

    virtual bool OnSink(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ParticlePenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Determines whether a particle which has hit this MO will penetrate,
//                  and if so, whether it gets lodged or exits on the other side of this
//                  MO. Appropriate effects will be determined and applied ONLY IF there
//                  was penetration! If not, nothing will be affected.
// Arguments:       The HitData describing the collision in detail, the impulses have to
//                  have been filled out!
// Return value:    Whether the particle managed to penetrate into this MO or not. If
//                  somehting but a MOPixel or MOSParticle is being passed in as hitor,
//                  false will trivially be returned here.

    virtual bool ParticlePenetration(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.
// Arguments:       The impulse (kg * m/s) of the impact causing the gibbing to happen.
//					The internal blast impulse which will push the gibs away from the center.
//                  A pointer to an MO which the gibs shuold not be colliding with!
// Return value:    None.

    virtual void GibThis(Vector impactImpulse = Vector(), float internalBlast = 10, MovableObject *pIgnoreMO = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MoveOutOfTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this MovableObject are on top of
//                  terrain pixels, and if so, attempt to move this out so none of this'
//                  Atoms are on top of the terrain any more.
// Arguments:       Only consider materials stronger than this in the terrain for
//                  intersections.
// Return value:    Whether any intersection was successfully resolved. Will return true
//                  even if there wasn't any intersections to begin with.

    virtual bool MoveOutOfTerrain(unsigned char strongerThan = g_MaterialAir);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the global and accumulated forces. Then it clears
//                  out the force list.Note that this does NOT apply the accumulated
//                  impulses (impulse forces)!
// Arguments:       None.
// Return value:    None.

    virtual void ApplyForces();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the accumulated impulse forces. Then it clears
//                  out the impulse list.Note that this does NOT apply the accumulated
//                  regular forces (non-impulse forces)!
// Arguments:       None.
// Return value:    None.

    virtual void ApplyImpulses();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AttachEmitter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attaches an AEmitter to this MOSRotating.
// Arguments:       The AEmitter to attach, ownership IS transferred!
//                  The offset from the origin of this MOSRotating to where the AEmitter's
//                  joint is attached.
//                  Whether to check if this added emitter triggers the GibWoundLimit..
//                  sometimes that's a bad idea if potentially gibbing this would cause
//                  the m_Attachables list to be messed with while it's being iterated.
// Return value:    None.

    virtual void AttachEmitter(AEmitter *pEmitter, Vector emitOffset, bool checkGibWoundLimit = true);

    /// <summary>
    /// Attaches the passed in attachable and adds it to the list of attachables
    /// </summary>
    /// <param name="pAttachable"></param>
    void AddAttachable(Attachable *pAttachable);

    void AddAttachable(Attachable *pAttachable, const Vector& parentOffsetToSet);

    void AddAttachable(Attachable *pAttachable, bool isHardcodedAttachable);

    void AddAttachable(Attachable *pAttachable, const Vector& parentOffsetToSet, bool isHardcodedAttachable);

    bool RemoveAttachable(long attachableUniqueId);

    bool RemoveAttachable(Attachable *pAttachable);

    void DetachOrDestroyAll(bool destroy);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.
// Arguments:       None.
// Return value:    None.

    virtual void ResetAllTimers();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    virtual void RestDetection();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

    virtual bool IsOnScenePoint(Vector &scenePoint) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EraseFromTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Cuts this' silhouette out from the terrain's material and color layers.
// Arguments:       None.
// Return value:    None.

    virtual void EraseFromTerrain();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if any of this' deep group atmos are on top of the terrain, and
//                  if so, erases this' silhouette from the terrain.
// Arguments:       Whether to make any MOPixels from erased terrain pixels at all.
//                  The size of the gaps between MOPixels knocked loose by the terrain erasure.
//                  The max number of MOPixel:s to generate as dislodged particles from the
//                  erased terrain.
// Return value:    Whether deep penetration was detected and erasure was done.

    virtual bool DeepCheck(bool makeMOPs = true, int skipMOP = 2, int maxMOP = 100);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

    virtual void PreTravel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MOSRotatin, using its physical representation.
// Arguments:       None.
// Return value:    None.

    virtual void Travel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Update(). Always call after
//                  calling Update.
// Arguments:       None.
// Return value:    None.

    virtual void PostTravel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawMOIDIfOverlapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the MOID representation of this to the SceneMan's MOID layer if
//                  this is found to potentially overlap another MovableObject.
// Arguments:       The MovableObject to check this for overlap against.
// Return value:    Whether it was drawn or not.

    virtual bool DrawMOIDIfOverlapping(MovableObject *pOverlapMO);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSRotating's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      DrawMode mode = g_DrawColor,
                      bool onlyPhysical = false) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetGibWoundLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return wound limit for this object.
// Arguments:       None.
// Return value:    Wound limit of the object.

	virtual int GetGibWoundLimit() const { return m_GibWoundLimit; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetGibImpulseLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set new impulse limit.
// Arguments:       New impulse limit.
// Return value:    None.

	virtual void SetGibImpulseLimit(int newLimit) { m_GibImpulseLimit = newLimit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetGibImpulseLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return impulse limit for this object.
// Arguments:       None.
// Return value:    Impulse limit of the object.

	virtual int GetGibImpulseLimit() const { return m_GibImpulseLimit; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetGibWoundLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set new wound limit, current wounds are not affected.
// Arguments:       New wound limit.
// Return value:    None.

	virtual void SetGibWoundLimit(int newLimit) { m_GibWoundLimit = newLimit; }



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds.
// Arguments:       Amount of wounds to remove.
// Return value:    Amount of damage, caused by these wounds.

	virtual int RemoveWounds(int amount); 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetWoundCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the amount of wound attached to this.
// Arguments:       Key to retrieve value.
// Return value:    Wound amount.

	virtual int GetWoundCount() const { return m_Emitters.size(); }; 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the string value associated with the specified key or "" if it does not exist.
// Arguments:       Key to retrieve value.
// Return value:    String value.

	virtual std::string GetStringValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number value associated with the specified key or 0 if it does not exist.
// Arguments:       Key to retrieve value.
// Return value:    Number (double) value.

	virtual double GetNumberValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the object value associated with the specified key or 0 if it does not exist.
// Arguments:       None.
// Return value:    Object (Entity *) value.

	virtual Entity * GetObjectValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	virtual void SetStringValue(std::string key, std::string value);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	virtual void SetNumberValue(std::string key, double value);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	virtual void SetObjectValue(std::string key, Entity * value);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the string value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	virtual void RemoveStringValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the number value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	virtual void RemoveNumberValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the object value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	virtual void RemoveObjectValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  StringValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	virtual bool StringValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NumberValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	virtual bool NumberValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ObjectValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	virtual bool ObjectValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets damage multiplier of this attachable.
// Arguments:       New multiplier value.
// Return value:    None.

	virtual void SetDamageMultiplier(float newValue) { m_DamageMultiplier = newValue; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns damage multiplier of this attachable.
// Arguments:       None.
// Return value:    Current multiplier value.

	virtual float GetDamageMultiplier() const { return m_DamageMultiplier; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDamageMultiplierRedefined
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the damage multiplier was altered in the .INI definition. 
//					If not, CC will apply default values during actor construction.
// Arguments:       None.
// Return value:    Current multiplier value.

	virtual bool IsDamageMultiplierRedefined() const { return m_DamageMultiplierRedefined; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTravelImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Retrurns the amount of impulse force exerted on this during the last frame.
// Arguments:       None.
// Return value:    The amount of impulse force exerted on this during the last frame.

	virtual Vector GetTravelImpulse() const { return m_TravelImpulse; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTravelImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of impulse force exerted on this during the last frame.
// Arguments:       New impulse value
// Return value:    None.

	virtual void SetTravelImpulse(Vector impulse) { m_TravelImpulse = impulse; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyAttachableForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the joint force transfer stuff for an attachable. Convencinece
//                  method. If this returns false, it means the attachable has been knocked
//                  off and has been passed to MovableMan OR deleted. In either case,
//                  if false is returned just set the pointer to 0 and be done with it.
// Arguments:       A pointer to the attachable to mess with. Ownership isn't transferred,
//                  but if the return is false, then the object has been deleted!
//					If isCritical is true, then if attachable is gibbed created break wound
//					emits indefenitely to guarantee actor's death.
// Return value:    Whether or not the joint held up to the forces and impulses which had
//                  accumulated on the Attachable during this Update(). If false, the passed
//                  in instance is now deleted and invalid!

    bool ApplyAttachableForces(Attachable *pAttachable, bool isCritical = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.
// Arguments:       The MOID index to register itself and its children in.
//                  The MOID of the root MO of this MO, ie the highest parent of this MO.
//                  0 means that this MO is the root, ie it is owned by MovableMan.
//                  Whether this MO should make a new MOID to use for itself, or to use
//                  the same as the last one in the index (presumably its parent),
// Return value:    None.

    virtual void UpdateChildMOIDs(std::vector<MovableObject *> &MOIDIndex,
                                 MOID rootMOID = g_NoMOID,
                                 bool makeNewMOID = true);

    // Member variables
    static Entity::ClassInfo m_sClass;
//    float m_Torque; // In kg * r/s^2 (Newtons).
//    float m_ImpulseTorque; // In kg * r/s.
    // The group of Atom:s that will be the physical reperesentation of this MOSRotating.
    AtomGroup *m_pAtomGroup;
    // The group of Atom:s that will serve as a means to detect deep terrain penetration.
    AtomGroup *m_pDeepGroup;
    // Whether or not to check for deep penetrations.
    bool m_DeepCheck;
    // A trigger for forcing a deep check to happen
    bool m_ForceDeepCheck;
    // Whether deep penetration happaned in the last frame or not, and how hard it was.
    float m_DeepHardness;
    // The amount of impulse force exerted on this during the last frame.
    Vector m_TravelImpulse;
    // The precomupted center location of the sprite relative to the MovableObject::m_Pos.
    Vector m_SpriteCenter;
    // How much to orient the rotation of this to match the velocity vector each frame 0 = none, 1.0 = immediately align with vel vector
    float m_OrientToVel;
    // Whether the SpriteMO is currently pushed back by recoil or not.
    bool m_Recoiled;
    // The impulse force in kg * m/s that represents the recoil.
    Vector m_RecoilForce;
    // The vector that the recoil offsets the sprite when m_Recoiled is true.
    Vector m_RecoilOffset;
    // The list of AEmitters currently attached to this MOSRotating, and owned here as well
    std::list<AEmitter *> m_Emitters;
    // The list of general Attachables currently attached and Owned by this.
    std::list<Attachable *> m_Attachables;
    // The list of all Attachables, including both hardcoded attachables and those added through ini or lua
    std::list<Attachable *> m_AllAttachables;
    // The list of Gib:s this will create when gibbed
    std::list<Gib> m_Gibs;
    // The amount of impulse force required to gib this, in kg * (m/s). 0 means no limit
    float m_GibImpulseLimit;
    // The number of emitters allowed before this gets gibbed. 0 means this can't get gibbed
    int m_GibWoundLimit;
    // Gib sound effect
    Sound m_GibSound;
    // Whether to flash effect on gib
    bool m_EffectOnGib;
    // How far this is audiable (in screens) when gibbing
    float m_LoudnessOnGib;
	// Map to store any generic strings
	std::map<std::string, std::string> m_StringValueMap;
	// Map to store any generic numbers
	std::map<std::string, double> m_NumberValueMap;
	// Map to store any object pointers
	std::map<std::string, Entity *> m_ObjectValueMap;

	// Damage mutliplier for this attachable
	float m_DamageMultiplier;
	// Whether damage multiplier for this attachable was redefined in .ini
	bool m_DamageMultiplierRedefined;

    // Intermediary drawing bitmap used to flip rotating bitmaps. Owned!
    BITMAP *m_pFlipBitmap;
    BITMAP *m_pFlipBitmapS;
    // Intermediary drawing bitmap used to draw sihouettes and other effects. Not owned; points to the shared static bitmaps
    BITMAP *m_pTempBitmap;
    // Temp drawing bitmaps shared between all MOSRotatings
    static BITMAP *m_spTempBitmap16;
    static BITMAP *m_spTempBitmap32;
    static BITMAP *m_spTempBitmap64;
    static BITMAP *m_spTempBitmap128;
    static BITMAP *m_spTempBitmap256;
    static BITMAP *m_spTempBitmap512;

    // Intermediary drawing bitmap used to draw MO silhouettes. Not owned; points to the shared static bitmaps
    BITMAP *m_pTempBitmapS;
    // Temp drawing bitmaps shared between all MOSRotatings
    static BITMAP *m_spTempBitmapS16;
    static BITMAP *m_spTempBitmapS32;
    static BITMAP *m_spTempBitmapS64;
    static BITMAP *m_spTempBitmapS128;
    static BITMAP *m_spTempBitmapS256;
    static BITMAP *m_spTempBitmapS512;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSRotating, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    MOSRotating(const MOSRotating &reference);
    MOSRotating& operator=(const MOSRotating &rhs);

};

} // namespace RTE

#endif // File