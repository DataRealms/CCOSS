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

class ACRocket : public ACraft {
	friend struct EntityLuaBindings;

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
EntityAllocation(ACRocket);
SerializableOverrideMethods;
ClassInfoGetters;

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

	~ACRocket() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACRocket object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACRocket to be identical to another, by deep copy.
// Arguments:       A reference to the ACRocket to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const ACRocket &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ACRocket, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); ACraft::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.
// Arguments:       The max altitude you care to check for. 0 Means check the whole scene's height.
//                  The accuracy within which measurement is acceptable. Higher number
//                  here means less calculation.
// Return value:    The rough altitude over the terrain, in pixels.

	float GetAltitude(int max = 0, int accuracy = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.
// Arguments:       None.
// Return value:    None.

	void UpdateAI() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       Nosssssssne.
// Return value:    None.

	void Update() override;


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

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:	GetMaxPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The recomended, not absolute, maximum number of actors that fit in the
//                  invetory. Used by the activity AI.
// Arguments:       None.
// Return value:    An integer with the recomended number of actors that fit in the craft.
//                  Default is two.

	int GetMaxPassengers() const override { return m_MaxPassengers > -1 ? m_MaxPassengers : 2; }

    /// <summary>
    /// Gets the right leg of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the right Leg of this ACRocket. Ownership is NOT transferred.</returns>
    Leg * GetRightLeg() const { return m_pRLeg; }

    /// <summary>
    /// Sets the right Leg for this ACRocket.
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetRightLeg(Leg *newLeg);

    /// <summary>
    /// Gets the left Leg of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the left Leg of this ACRocket. Ownership is NOT transferred.</returns>
    Leg * GetLeftLeg() const { return m_pLLeg; }

    /// <summary>
    /// Sets the left Leg for this ACRocket.
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetLeftLeg(Leg *newLeg);

    /// <summary>
    /// Gets the main thruster of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the main thruster of this ACRocket. Ownership is NOT transferred.</returns>
	AEmitter * GetMainThruster() const { return m_pMThruster; }

    /// <summary>
    /// Sets the main thruster for this ACRocket.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetMainThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the right side thruster of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the right side thruster of this ACRocket. Ownership is NOT transferred.</returns>
	AEmitter * GetRightThruster() const { return m_pRThruster; }

    /// <summary>
    /// Sets the right side thruster for this ACRocket.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetRightThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the left side thruster of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the left side thruster of this ACRocket. Ownership is NOT transferred.</returns>
    AEmitter * GetLeftThruster() const { return m_pLThruster; }

    /// <summary>
    /// Sets the left side thruster for this ACRocket.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetLeftThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the right side secondary thruster of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the right side secondary thruster of this ACRocket. Ownership is NOT transferred.</returns>
    AEmitter * GetURightThruster() const { return m_pURThruster; }

    /// <summary>
    /// Sets the right side secondary thruster for this ACRocket.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetURightThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the left side secondary thruster of this ACRocket.
    /// </summary>
    /// <returns>A pointer to the left side secondary thruster of this ACRocket. Ownership is NOT transferred.</returns>
    AEmitter * GetULeftThruster() const { return m_pULThruster; }

    /// <summary>
    /// Sets the left side secondary thruster for this ACRocket.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetULeftThruster(AEmitter *newThruster);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetGearState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the the landing gear state
// Arguments:       None.
// Return value:    Current landing gear state.

	unsigned int GetGearState() const { return m_GearState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


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
	ACRocket(const ACRocket &reference) = delete;
	ACRocket & operator=(const ACRocket &rhs) = delete;

};

} // namespace RTE

#endif // File