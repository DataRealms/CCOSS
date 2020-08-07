#ifndef _RTETURRET_
#define _RTETURRET_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Turret.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Turret class.
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
// Class:           Turret
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A detatchable turret pod that can hold HeldDevices (weapons, tools)
// Parent(s):       Attachable.
// Class history:   10/24/2007 Turret created.

class Turret:
    public Attachable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Turret)
SerializableOverrideMethods
ClassInfoGetters


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Turret
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Turret object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Turret() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Turret
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Turret object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Turret() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Turret to be identical to another, by deep copy.
// Arguments:       A reference to the Turret to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Turret &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Turret, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Attachable::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this Turret, including the mass of any device it
//                  may be holding.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMountedDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the HeldDevice currently held by this Turret, IF the thing held is
//                  a HeldDevice, that is. Ownership is NOT transferred.
// Arguments:       None.
// Return value:    A pointer to the currently held HeldDevice. 0 is returned if no
//                  HeldDevice is currently held (even though an MO may be held).

    HeldDevice * GetMountedDevice() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDeviceMounted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a device is being held or not. Faster than using
//                  GetMountedDevice() for the purpose.
// Arguments:       None.
// Return value:    A bool indicating whether any device is held by this Turret.

    bool IsDeviceMounted() const { return m_pMountedMO && m_pMountedMO->IsDevice(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsHeldDeviceMounted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a HeldDevice is held or not. Faster than using
//                  GetMountedDevice() for the purpose.
// Arguments:       None.
// Return value:    A bool indicating whether a HeldDevice is held by this Turret.

    bool IsHeldDeviceMounted() const { return m_pMountedMO && m_pMountedMO->IsHeldDevice(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsThrownDeviceMounted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a ThrownDevice is held or not. Faster than using
//                  GetMountedDevice() for the purpose.
// Arguments:       None.
// Return value:    A bool indicating whether a ThrownDevice is held by this Turret.

    bool IsThrownDeviceMounted() const { return m_pMountedMO && m_pMountedMO->IsThrownDevice(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MovableObject currently held by this Turret. Ownership is NOT
//                  transferred.
// Arguments:       None.
// Return value:    A pointer to the currently held MovableObject. 0 is returned if no
//                  MovableObject is currently held.

    MovableObject * GetMountedMO() const { return m_pMountedMO; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       A MOID specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Turret with a new
//                  one. Ownership IS transferred. The currently held MovableObject
//                  (if there is one) will be dropped and become a detached MovableObject,
// Arguments:       A pointer to the new MovableObject to hold. Ownership IS transferred.
// Return value:    None.

    void SetMountedMO(MovableObject *newHeldMO);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReleaseMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of the MovableObject currently held. Ownership
//                  IS transferred!
// Arguments:       None.
// Return value:    A pointer to the up to this point held MovableObject. 0 is returned
//                  if no MovableObject is currently held. Ownership IS transferred!

    MovableObject * ReleaseMountedMO();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DropEverything
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of anyhthing it holds and give it to the
//                  MovableMan. Ownership is transferred to MovableMan.
// Arguments:       None.
// Return value:    A pointer to the up to this point held anything. 0 is returned if
//                  nothing is currently held. Ownership is NOT transferred, but given
//                  to MovableMan.

    MovableObject * DropEverything();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Turret with a new
//                  one, and returns the replaced one. Ownership IS transferred both ways.
// Arguments:       A pointer to the new MovableObject to hold. Ownership IS transferred.
// Return value:    A pointer to the previously held MO. Ownership IS transferred.

    MovableObject * SwapMountedMO(MovableObject *newMO);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDeviceMounted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Turret is holding a HeldDevice or not.
// Arguments:       None.
// Return value:    Whether this Turret is holding a HeldDevice or not.

    bool IsDeviceMounted() { return m_pMountedMO && m_pMountedMO->IsDevice(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsSomethingMounted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this Turret is holding an MO or not.
// Arguments:       None.
// Return value:    Whether this Turret is holding anyhting.

    bool IsSomethingMounted() { return m_pMountedMO != 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMountedRotOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current rotational offset of the mounted device from the rest
//                  of the turret
// Arguments:       The new offset angle in radians, relative from the rest of the turret.
// Return value:    None.

    virtual void SetMountedRotOffset(float newOffsetAngle) { m_MountedRotOffset = newOffsetAngle; }


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

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Turret's current graphical representation to a
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


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame,
//                  and then finally draws its and its childrens' MOID silhouette onto a
//                  BITMAP of choice.
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
    // The MovableObject held, updated and held by this Turret, if any.
    MovableObject *m_pMountedMO;
    // The relative offset in angle (radians) of the mounted device from this' rotation
    float m_MountedRotOffset;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Turret, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Turret(const Turret &reference);
    Turret & operator=(const Turret &rhs);

};

} // namespace RTE

#endif // File