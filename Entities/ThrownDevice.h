#ifndef _RTETHROWNDEVICE_
#define _RTETHROWNDEVICE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ThrownDevice.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ThrownDevice class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "HeldDevice.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  ThrownDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A device that is carried and thrown by Actor:s
// Parent(s):       HeldDevice.
// Class history:   8/8/2004 ThrownDevice created.
//                  9/18/2008 ThrownDevice subordinated to HeldDevice

class ThrownDevice:
    public HeldDevice
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(ThrownDevice)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ThrownDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ThrownDevice object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ThrownDevice() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ThrownDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ThrownDevice object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ThrownDevice() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ThrownDevice object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ThrownDevice to be identical to another, by deep copy.
// Arguments:       A reference to the ThrownDevice to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const ThrownDevice &reference);


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
// Description:     Resets the entire ThrownDevice, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Attachable::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ThrownDevice to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ThrownDevice will save itself with.
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
// Virtual method:  GetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this ThrownDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current stance parent offset.

    Vector GetStanceOffset() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStartThrowOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Start throw offset of this ThrownDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current start throw parent offset.

    Vector GetStartThrowOffset() const { return m_StartThrowOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEndThrowOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the End throw offset of this ThrownDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current end throw parent offset.

    Vector GetEndThrowOffset() const { return m_EndThrowOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMinThrowVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the minimum throw velocity of this when thrown.
// Arguments:       None.
// Return value:    The minimum throw velocity of this, in m/s.

    float GetMinThrowVel() const { return m_MinThrowVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMinThrowVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the minimum throw velocity of this when thrown.
// Arguments:       The minimum throw velocity of this, in m/s.
// Return value:    None.

    void SetMinThrowVel(float Vel) { m_MinThrowVel = Vel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaxThrowVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the maximum throw velocity of this when thrown.
// Arguments:       None.
// Return value:    The maximum throw velocity of this, in m/s.

    float GetMaxThrowVel() const { return m_MaxThrowVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMaxThrowVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the maximum throw velocity of this when thrown.
// Arguments:       None.
// Return value:    The maximum throw velocity of this, in m/s.

    void SetMaxThrowVel(float Vel) { m_MaxThrowVel = Vel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.
// Arguments:       None.
// Return value:    None.

    virtual void ResetAllTimers() { Attachable::ResetAllTimers(); m_ActivationTmr.Reset(); m_ThrownTmr.Reset(); }


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
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite
// Arguments:       None.
// Return value:    None.

    virtual void Detach() { Attachable::Detach(); m_ThrownTmr.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates one of this Device's features. Analogous to starting the throw
// Arguments:       None.
// Return value:    None.

    virtual void Activate();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this Device's features. Analogous to starting the
//                  throw
// Arguments:       None.
// Return value:    None.

    virtual void Deactivate();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.
// Arguments:       The other MO hit. Ownership is not transferred.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this hit.

    virtual bool OnMOHit(MovableObject *pOtherMO);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this, using its physical representation.
// Arguments:       None.
// Return value:    None.

    virtual void Travel();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ThrownDevice's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  Whether the bitmap passed in is a screen sized bitmap and thus
//                  whatever is to be drawn on it should be subtracted by the SceneMan's
//                  view scroll offset.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      DrawMode mode = g_DrawColor,
                      bool onlyPhysical = false) const = 0;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Activation sound
    Sound m_ActivationSound;
    // The position offset at which a throw of this Device begins.
    Vector m_StartThrowOffset;
    // The position offset at which a throw of this Device ends.
    Vector m_EndThrowOffset;
    // The minimum throw velocity this gets when thrown
    float m_MinThrowVel;
    // The maximum throw velocity this gets when thrown
    float m_MaxThrowVel;

    // Time in millisecs from the time of being thrown to triggering
    // whatever it is that this ThrownDevice does.
    long m_TriggerDelay;
    // Timer for timing how long ago this ThrownDevice was thrown.
    Timer m_ThrownTmr;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ThrownDevice, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ThrownDevice(const ThrownDevice &reference);
    ThrownDevice & operator=(const ThrownDevice &rhs);

};

} // namespace RTE

#endif // File