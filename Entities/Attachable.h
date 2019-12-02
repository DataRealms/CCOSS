#ifndef _RTEATTACHABLE_
#define _RTEATTACHABLE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Attachable.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Attachable class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files
 
#include "MOSRotating.h"

namespace RTE
{

class AEmitter;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Attachable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An articulated, detachable part of an Actor's body.
// Parent(s):       MOSRotating.
// Class history:   05/30/2002 Attachable created.

class Attachable:
    public MOSRotating
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(Attachable)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Attachable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Attachable object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Attachable() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Attachable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Attachable object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Attachable() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Attachable object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Attachable to be identical to another, by deep copy.
// Arguments:       A reference to the Attachable to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Attachable &reference);


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
// Description:     Resets the entire Attachable, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); MOSRotating::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Attachable to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Attachable will save itself with.
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
// Method:          GetRootParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MO which is the ultimate root parent of this Attachable and
//                  its parent.
// Arguments:       None.
// Return value:    A pointer to the highest root parent of this Attachable.

    virtual MovableObject * GetRootParent() { return m_pParent ? m_pParent->GetRootParent() : this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRootParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MO which is the ultimate root parent of this Attachable and
//                  its parent.
// Arguments:       None.
// Return value:    A pointer to the highest root parent of this Attachable.

    virtual const MovableObject * GetRootParent() const { return m_pParent ? m_pParent->GetRootParent() : this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MO which is the parent of this Attachable.
// Arguments:       None.
// Return value:    A pointer to the highest root parent of this Attachable.

	virtual MovableObject * GetParent() { return m_pParent; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MO which is the parent of this Attachable. 
// Arguments:       None.
// Return value:    A pointer to the highest root parent of this Attachable.

	virtual const MovableObject * GetParent() const { return m_pParent; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParentOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the stored offset between this Attachable's parent's Pos and the 
//                  joint position. This should be maintained by the parent.
// Arguments:       None.
// Return value:    A const reference Vector describing the offset from the parent's pos
//                  to the joint point.

    virtual const Vector & GetParentOffset() const { return m_ParentOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJointOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset to the joint (the point around which this Attachable
//                  and its parent hinge) from this Attachable's center of mass/origin.
// Arguments:       None.
// Return value:    A const reference Vector describing the offset of the joint relative
//                  to the this Attachable's origin/center of mass position.

    virtual const Vector & GetJointOffset() const { return m_JointOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset to the joint (the point around which this Attachable
//                  and its parent hinge) from this Attachable's center of mass/origin.
// Arguments:       Vector describing the offset of the joint relative
//                  to the this Attachable's origin/center of mass position.
// Return value:    None.

	virtual void SetJointOffset(Vector offset) { m_JointOffset = offset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJointStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of impulse force the joint of this Attachable can
//                  handle before breaking.
// Arguments:       None.
// Return value:    A float with the max tolerated impulse force in kg * m/s.

    float GetJointStrength() const { return m_JointStrength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJointStiffness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the normalized stiffness scalar of the join of this Attachable.
// Arguments:       None.
// Return value:    A float between 0 and 1.0. 1.0 means 100% of all forces imposed on this
//                  attachable will be transferred through the joint to the parent. 0.5
//                  means 50%, and so on.

    float GetJointStiffness() const { return m_JointStiffness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBreakWound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the AEmitter that represents the wound created when this
//                  Attachable gets detached from its parent.
// Arguments:       None.
// Return value:    A const pointer to the break wound AEmitter.

    const AEmitter * GetBreakWound() const { return m_pBreakWound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRotTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the target rotation that this Attachable should be striving to
//                  match its real rotation with, if it is attached. The joint stiffness
//                  will determine how strong the scalar spring is between the current
//                  rotation and the target set here.
// Arguments:       None.
// Return value:    A float for the current target angle in radians.

    float GetRotTarget() const { return m_RotTarget.GetRadAngle(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtomSubgroupID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the subgroup ID of this' Atoms.
// Arguments:       None.
// Return value:    The the subgroup ID of this' Atoms.

    int GetAtomSubgroupID() const { return m_AtomSubgroupID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOnlyLinearForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this AEmitter only cares about linear forces that it
//                  creates through emissions, ie no torquing due to the parent offset.
// Arguments:       None.
// Return value:    Whether only using linear forces or not.

    bool GetOnlyLinearForces() const { return m_OnlyLinForces; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetParentOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the stored offset between this Attachable's parent's Pos and the 
//                  joint position. This should be maintained by the parent.
// Arguments:       A const reference to the new parent offset.
// Return value:    None.

    void SetParentOffset(const Vector &newParOff) { m_ParentOffset = newParOff; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the position of this Attachable by defining where the joint is.
//                  Upon Update(), this will be translated into what the actual position
//                  of the Attachable origin/center of mass is, depending on its set
//                  rotational angle and joint offset.
// Arguments:       A const reference to the new joint position to set the position with.
// Return value:    None.

    void SetJointPos(const Vector &newJointPos) { m_JointPos = newJointPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of impulse force the joint of this Attachable can
//                  handle before breaking.
// Arguments:       A float with the max tolerated impulse force in kg * m/s.
// Return value:    None.

    void SetJointStrength(float newJointStrength) { m_JointStrength = newJointStrength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJointStiffness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the normalized stiffness scalar of the join of this Attachable.
// Arguments:       A float between 0 and 1.0. 1.0 means 100% of all forces imposed on this
//                  attachable will be transferred through the joint to the parent. 0.5
//                  means 50%, and so on.
// Return value:    None.

    void SetJointStiffness(float newJointStiff) { m_JointStiffness = newJointStiff; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRotTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the target rotation that this Attachable should be striving to
//                  match its real rotation with, if it is attached. The joint stiffness
//                  will determine how strong the scalar spring is between the current
//                  rotation and the target set here.
// Arguments:       A float for the new target angle in radians.
// Return value:    None.

    void SetRotTarget(float newRotTarget) { m_RotTarget.SetRadAngle(newRotTarget); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtomSubgroupID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the subgroup ID of this' Atoms
// Arguments:       The new subgroup id of this' Atoms
// Return value:    None.

    void SetAtomSubgroupID(int newID = 0) { m_AtomSubgroupID = newID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOnlyLinearForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this AEmitter should only care about linear forces that it
//                  creates through emissions, ie no torquing due to the parent offset.
// Arguments:       Whether only use linear forces or not.
// Return value:    None.

    void SetOnlyLinearForces(bool linOnly) { m_OnlyLinForces = linOnly; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAttached
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Attachable is attached to an MOSprite or not.
// Arguments:       None.
// Return value:    Whether it's attached or not.

    bool IsAttached() const { return m_pParent != 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAttachedTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Attachable is attached to a specific actor or
//                  not.
// Arguments:       A pointer to which MOSprite you want to check if this is attached to.
// Return value:    Whether it's attached or not.

    bool IsAttachedTo(const MOSprite *pparent) const { return m_pParent == pparent; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsDrawnAfterParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Attachable is to be drawn after (in front of) or
//                  before (behind) the parent.
// Arguments:       None.
// Return value:    Whether it's to be drawn after parent or not.

    virtual bool IsDrawnAfterParent() const { return m_DrawAfterParent; }


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
// Virtual method:  Attach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attaches this Attachable to a host MOSprite
// Arguments:       Pointer to the MOSprite to attach to. Ownership is NOT transferred!
// Return value:    None.

    virtual void Attach(MOSRotating *pParent);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Attach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attaches this Attachable to a host MOSprite
// Arguments:       Pointer to the MOSprite to attach to. Ownership is NOT transferred!
//                  The offset from the parent's Pos to the joint position.
// Return value:    None.

    virtual void Attach(MOSRotating *pParent, const Vector &parOffset) { Attach(pParent); m_ParentOffset = parOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite
// Arguments:       None.
// Return value:    None.

    virtual void Detach();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TransferJointForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bundles up all the accumulated forces of this Attachable and calcs
//                  how they transfer to the joint, and therefore to the parent.
// Arguments:       A vector that will have with the forces affecting the joint ADDED to it.
// Return value:    If the accumulated forces exceed the strength of the joint, the
//                  attachable will only fill out the forces up to the strength threshold
//                  and then detach itself and return false, and the parent should react
//                  accordingly in that case (null out pointers to the Attachable).

    bool TransferJointForces(Vector &jointForces);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TransferJointImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bundles up all the accumulated impulses of this Attachable and calcs
//                  how they transfer to the joint, and therefore to the parent.
// Arguments:       A vector that will have with the imps affecting the joint ADDED to it.
// Return value:    If the accumulated impulses exceed the strength of the joint, the
//                  attachable will only fill out the impulses up to the strength threshold
//                  and then detach itself and return false, and the parent should react
//                  accordingly in that case (null out pointers to the Attachable).

    bool TransferJointImpulses(Vector &jointImpulses);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollectDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of damage points this Attachable has sustained and
//                  should cause its parent. Calling this will reset the damage count.
//                  This should normally be called AFTER Update() to get the correct
//                  damage for a given frame.
// Arguments:       None.
// Return value:    A float with the damage points accumulated since the last time
//                  this method was called.

    virtual float CollectDamage();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Update(). Always call after
//                  calling Update.
// Arguments:       None.
// Return value:    None.

    virtual void PostTravel();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Attachable's current graphical representation to a
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
// Virtual method:  RemoveWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds.
// Arguments:       Amount of wounds to remove.
// Return value:    Amount of damage, caused by these wounds.

	virtual int RemoveWounds(int amount); 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified number of damage points to this attachable.
// Arguments:       Amount of damage to add.
// Return value:    None.

	virtual void AddDamage(float amount) { m_DamageCount += amount;  };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InheritsRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If true (default old behavior) the parent MOSRotating resets attachables or emitters RotAngle every frame. False to avoid that.
// Arguments:       None.
// Return value:    Whether parent MOSRotating should change this RotAngle to match it's own during MOSRotating::Update

	virtual bool InheritsRotAngle() const { return m_InheritsRotAngle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetInheritsRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets Whether parent MOSRotating should change this RotAngle to match it's own during MOSRotating::Update.
// Arguments:       Whether to inherit RotAngle or not.
// Return value:    None.

	virtual void SetInheritsRotAngle(bool inherit) { m_InheritsRotAngle = inherit; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Pointer to the MOSRotating this attachable is attached to.
    MOSRotating *m_pParent;
    // The offset from the PARENT's Pos to the joint point this Attachable is attached.
    Vector m_ParentOffset;
    // The amount of impulse force needed on this to deatch it from the host Actor. In kg * m/s
    float m_JointStrength;

    // The normalized joint stiffness scalar. 1.0 = all forces are trasferred
    // to parent through joint. 0.5 = half are.
    float m_JointStiffness;

    // The wound this Attachable will cause when it breaks from its parent.
    const AEmitter *m_pBreakWound;

    // The offset to the joint (the point around which this Attachable and its
    // parent hinge) from its center of mass/origin.
    Vector m_JointOffset;
    // The absolute position of the joint that the parent sets upon Update() if this
    // Attachable is attached to it. This position is used to determine the actual
    // position/center of mass of this Attachable on its Update(), using the JointOff
    // and RotAngle of this attachable.
    Vector m_JointPos;

    // This is the desired rotation of this Attachable, the angle that the attachable
    // is striving to achieve through angle springs.
    Matrix m_RotTarget;

    // The Atom ID's this' atoms will have when attached and added to a parent's AtomGroup
    int m_AtomSubgroupID;

    // Whether to draw this Attachable after (in front of) or before (behind) the parent.
    bool m_DrawAfterParent;

    // The number of damage points that this Attachable has accumulated since the
    // last time CollectDamage() was called.
    float m_DamageCount;

    // Whether to only record linear forces, ie no torquing due to the parent offset
    bool m_OnlyLinForces;

	// If true inherits Parent's rot angle, which is set MOSRotating::Update. Default is true to maintain maybe awkward but default behavior
	bool m_InheritsRotAngle;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Attachable, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Attachable(const Attachable &reference);
    Attachable& operator=(const Attachable &rhs);

};

} // namespace RTE

#endif // File