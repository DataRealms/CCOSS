#ifndef _RTEACROCKET_
#define _RTEACROCKET_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACRocket.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ACRocket class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ACraft.h"
#include "LimbPath.h"

namespace RTE
{

class Attachable;
class Arm;
class Leg;
//class LimbPath;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ACRocket
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A rocket craft, with main engine and rotation of the whole body as a
//                  means of steering.
// Parent(s):       ACraft.
// Class history:   09/02/2004 ARocket created.
//                  12/13/2006 ARocket changed names to ACRocket, parent changed to ACraft

class ACRocket:
    public ACraft
{

enum LandingGearState
{
    RAISED = 0,
    LOWERED,
    LOWERING,
    RAISING,
    GearStateCount
};


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(ACRocket)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ACRocket
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ACRocket object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ACRocket() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ACRocket
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ACRocket object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ACRocket() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACRocket object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACRocket to be identical to another, by deep copy.
// Arguments:       A reference to the ACRocket to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const ACRocket &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ACRocket, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); ACraft::Reset(); }


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
// Description:     Gets the mass value of this ACRocket, including the mass of its
//                  currently attached body parts and inventory.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.
// Arguments:       The max altitude you care to check for. 0 Means check the whole scene's height.
//                  The accuracy within which measurement is acceptable. Higher number
//                  here means less calculation.
// Return value:    The rough altitude over the terrain, in pixels.

    virtual float GetAltitude(int max = 0, int accuracy = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       A MOID specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The position where the bounce-hit occurred.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this bounce.

    virtual bool OnBounce(const Vector &pos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The position where the sink-hit occurred.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this sinkage.

    virtual bool OnSink(const Vector &pos);


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
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateAI();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       Nosssssssne.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetEmissionTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reset the timers of all emissions so they will start/stop at the 
//                  correct relative offsets from now.
// Arguments:       None.
// Return value:    None.

    virtual void ResetEmissionTimers();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTotalWoundCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns total wound count of this actor and all vital attachables.
// Arguments:       None.
// Return value:    Returns total number of wounds of this actor.

	virtual int GetTotalWoundCount() const; 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTotalWoundLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns total wound limit of this actor and all vital attachables.
// Arguments:       None.
// Return value:    Returns total wound limit of this actor.

	virtual int GetTotalWoundLimit() const; 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ACRocket's current graphical representation to a
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
// Virtual method:	GetMaxPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The recomended, not absolute, maximum number of actors that fit in the
//                  invetory. Used by the activity AI.
// Arguments:       None.
// Return value:    An integer with the recomended number of actors that fit in the craft.
//                  Default is two.

    virtual int GetMaxPassengers() const { return m_MaxPassengers > -1 ? m_MaxPassengers : 2; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAnyRandomWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds from the actor and all standard attachables.
// Arguments:       Amount of wounds to remove.
// Return value:    Damage taken from removed wounds.

	virtual int RemoveAnyRandomWounds(int amount);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetMThruster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main thruster.
// Arguments:       None.
// Return value:    An AEmitter pointer.

	virtual AEmitter * GetMThruster() const { return m_pMThruster; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetRThruster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the right thruster.
// Arguments:       None.
// Return value:    An AEmitter pointer.

	virtual AEmitter * GetRThruster() const { return m_pRThruster; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetLThruster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the left thruster.
// Arguments:       None.
// Return value:    An AEmitter pointer.

	virtual AEmitter * GetLThruster() const { return m_pLThruster; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetURThruster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the right side secondary thruster.
// Arguments:       None.
// Return value:    An AEmitter pointer.

	virtual AEmitter * GetURThruster() const { return m_pURThruster; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetULThruster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the left side secondary thruster.
// Arguments:       None.
// Return value:    An AEmitter pointer.

	virtual AEmitter * GetULThruster() const { return m_pULThruster; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetGearState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the the landing gear state
// Arguments:       None.
// Return value:    Current landing gear state.

	virtual unsigned int GetGearState() const { return m_GearState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

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
    // Attached nosecone/capsule.
//    Attachable *m_pCapsule;
    // Right landing gear.
    Leg *m_pRLeg;
    // Left landing gear.
    Leg *m_pLLeg;
    // Body AtomGroups.
    AtomGroup *m_pBodyAG;
    // Limb AtomGroups.
    AtomGroup *m_pRFootGroup;
    AtomGroup *m_pLFootGroup;
    // Thruster emitters.
    AEmitter *m_pMThruster;
    AEmitter *m_pRThruster;
    AEmitter *m_pLThruster;
    AEmitter *m_pURThruster;
    AEmitter *m_pULThruster;
    // Current landing gear action state.
    unsigned int m_GearState;
    // Limb paths for different movement states.
    // [0] is for the right limbs, and [1] is for left.
    LimbPath m_Paths[2][GearStateCount];
    // The craft explodes if it has been on its side for more than this many MS (default 4000). Disable by setting to -1.
    float m_ScuttleIfFlippedTime;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACRocket, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
    ACRocket(const ACRocket &reference);
    ACRocket & operator=(const ACRocket &rhs);

};

} // namespace RTE

#endif // File