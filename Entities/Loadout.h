#ifndef _LOADOUT_
#define _LOADOUT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Loadout.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loadout class
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Entity.h"

namespace RTE
{

class SceneObject;
class MovableObject;
class ACraft;
class Actor;


//////////////////////////////////////////////////////////////////////////////////////////
// Nested class:    Loadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines a delivery of Actors, with all their equipment etc.
// Parent(s):       Entity.
// Class history:   2/21/2012 Loadout created.

class Loadout : public Entity {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Loadout);
SerializableOverrideMethods;
ClassInfoGetters;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Loadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Loadout object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Loadout() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Loadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Loadout object
//                  identical to an already existing one.
// Arguments:       A Loadout object which is passed in by reference.

    Loadout(const Loadout &reference) { if (this != &reference) { Clear(); Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Loadout assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Loadout equal to another.
// Arguments:       A Loadout reference.
// Return value:    A reference to the changed Loadout.

    Loadout & operator=(const Loadout &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Loadout to be identical to another, by deep copy.
// Arguments:       A reference to the Loadout to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const Loadout &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Entity, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  IsComplete
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this loadout is complete, or some Entity within could
//                  not be found on load.
// Arguments:       None.
// Return value:    Complete or not.

    bool IsComplete() { return m_Complete; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  CreateFirstActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Actor that this Loadout has and equips.
//                  Ownership IS transferred!! All items of the Loadout of this Deployment
//                  will be added to the Actor's inventory as well (and also owned by it)
// Arguments:       Which in-game player to create the Actor for.
//                  A float which will be added to with the cost of the stuff returned here.
// Return value:    The Actor instance, if any, that is first defined in this Loadout.
//                  OWNERSHIP IS TRANSFERRED!

	Actor * CreateFirstActor(int nativeModule, float foreignMult, float nativeMult, float &costTally) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  CreateFirstDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that is defined in this Loadout.
//                  Ownership IS transferred!! Only the first Device is created.
// Arguments:       Which in-game player to create the device for.
//                  A float which will be added to with the cost of the stuff returned here.
// Return value:    The SceneObject instance, if any, that this Loadout defines first.
//                  OWNERSHIP IS TRANSFERRED!

	SceneObject * CreateFirstDevice(int nativeModule, float foreignMult, float nativeMult,  float &costTally) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetDeliveryCraft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the preset of the delivery craft set for this loadout. Owenership
//                  is NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the ACraft preset instance. OWNERSHIP IS NOT TRANSFERRED!

	const ACraft * GetDeliveryCraft() const { return m_pDeliveryCraft; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetDeliveryCraft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the preset of the delivery craft set for this loadout. Owenership
//                  is NOT transferred!
// Arguments:       A pointer to the ACraft preset instance. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    None.

	void SetDeliveryCraft(const ACraft *pCraft) { m_pDeliveryCraft = pCraft; m_Complete = m_Complete && m_pDeliveryCraft; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetCargoList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of cargo Entity items this Loadout represents.
// Arguments:       None.
// Return value:    A pointer to the list of cargo Entity items. OWNERSHIP IS NOT TRANSFERRED!

	std::list<const SceneObject *> * GetCargoList() { return &m_CargoItems; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  AddToCargoList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a new Preset to the list of cargo items to be included in this.
// Arguments:       A const pointer to the ScneObject preset we want to add to this loadout.
// Return value:    None.

	void AddToCargoList(const SceneObject *pNewItem) { if (pNewItem) m_CargoItems.push_back(pNewItem); }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Whether this Loadout is the full thing, or something is missing due to unavailable entities on load
    bool m_Complete;
    // Preset instance of the delivery craft, not owned by this.
    const ACraft *m_pDeliveryCraft;
    // The cargo of this loadout, all preset instances not owned by this
    std::list<const SceneObject *> m_CargoItems;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Loadout, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif  // File