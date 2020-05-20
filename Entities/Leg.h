#ifndef _RTELEG_
#define _RTELEG_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Leg.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Leg class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Attachable.h"

namespace RTE
{

class HeldDevice;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Leg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A detatchable leg that will be controlled by LimbPath:s.
// Parent(s):       Attachable.
// Class history:   05/30/2002 Leg created.

class Leg:
    public Attachable
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Leg)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Leg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Leg object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Leg() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Leg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Leg object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Leg() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Leg object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Leg to be identical to another, by deep copy.
// Arguments:       A reference to the Leg to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Leg &reference);


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
// Description:     Resets the entire Leg, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Attachable::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Leg to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Leg will save itself with.
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
// Method:          GetAnklePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the position of the ankle of this Leg as an absolute scene coord.
// Arguments:       None.
// Return value:    Vector with the current absolute scene ankle position.

    Vector GetAnklePos() { return m_Pos + m_AnkleOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaxLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the max length this leg can reach from its socket to the foot.
// Arguments:       None.
// Return value:    The max length of reach, in pixels, of this leg.

    float GetMaxLength() const { return m_MaxExtension; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       A MOID specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIdleOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset from the position of this Leg to which its foot will
//                  go when not holding a device and not able to reach a certain target.
// Arguments:       Vector with the new idle offset relative to the position of this Leg.
// Return value:    None.

    void SetIdleOffset(const Vector &newIdleOffset) { m_IdleOffset = newIdleOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableIdle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this to go into idle offset mode if the target appears to be above
//                  the joint of the leg.
// Arguments:       Whetehr to enable the idling if the target is above the joint
// Return value:    None.

    void EnableIdle(bool idle = true) { m_WillIdle = idle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReachToward
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates the leg so that it reaches after a point in scene coordinates.
//                  Must be called AFTER SetPos for this frame if the return value is to
//                  be accurate.
// Arguments:       The point to reach after. If (0, 0), reaching is deactivated.
// Return value:    None.

    void ReachToward(const Vector &scenePoint);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DidReach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Leg actually reached the reach target last
//                  Update().
// Arguments:       None.
// Return value:    Whether.the Leg was able to actually reach the point or not.

    bool DidReach() { return m_DidReach; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BendLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bends the leg to the appropriate position depending on the ankle
//                  offset.
// Arguments:       None.
// Return value:    None.

    void BendLeg();


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
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Leg's current graphical representation to a
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
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;


	/// <summary>
	/// Gets the foot Attachable of this.
	/// </summary>
	/// <returns>A pointer to the foot Attachable of this. Ownership is NOT transferred!</returns>
	Attachable * GetFoot() const { return m_pFoot; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConstrainFoot
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the foot distance is constrained between the mix and max
//                  extension of this Leg.
// Arguments:       None.
// Return value:    Whether.the Foot already was within the reach range of this Leg or not.

    bool ConstrainFoot();


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
//    // The location of the 'foot' in relation to the MovableObject::m_Pos
//    Vector m_FootPos;
    // The foot attachable.
    Attachable *m_pFoot;
    // The offset from the joint where the ankle contracts to in the sprite
    Vector m_ContractedOffset;
    // The offset from the joint where the ankle extends to in the sprite
    Vector m_ExtendedOffset;
    // Precalculated min and max extensions of the leg (from the joint) based ont he contracted and extended offsets
    float m_MinExtension;
    float m_MaxExtension;
    // normalized scalar of where the ankle offset's magnitude is between the min and max extensions
    float m_CurrentNormalizedExtension;
    // Current offset from the joint to the ankle where the foot should be
    Vector m_AnkleOffset;
    // The target offset that this Leg's foot is reaching after.
    // If (0, 0), the Leg is currently not reaching after anything.
    Vector m_TargetOffset;
    // The target offset from m_Pos that this Leg's foot is reaching after when
    // not reaching for or holding anything else.
    Vector m_IdleOffset;
    // How fast the leg moves to a reach target,
    // on a scale from 0.0 (frozen) to 1.0 (instantly there).
    float m_MoveSpeed;
    // Whether this will go to idle position if the target is above the joint
    bool m_WillIdle;
    // Whether this Leg reaached the reach target last Update.
    bool m_DidReach;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Leg, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Leg(const Leg &reference);
    Leg & operator=(const Leg &rhs);

};

} // namespace RTE

#endif // File