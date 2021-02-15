#ifndef _RTEACDROPSHIP_
#define _RTEACDROPSHIP_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACDropShip.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ACDropShip class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ACraft.h"

namespace RTE
{

class Attachable;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ACDropShip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A hovering craft, with two engines on each attached on each end which
//                  tilt independently of the body to achieve steering.
// Parent(s):       ACraft.
// Class history:   12/13/2006 ACDropShip created.

class ACDropShip:
    public ACraft
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(ACDropShip)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ACDropShip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ACDropShip object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ACDropShip() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ACDropShip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ACDropShip object before deletion
//                  from system memory.
// Arguments:       None.

	~ACDropShip() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACDropShip object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACDropShip to be identical to another, by deep copy.
// Arguments:       A reference to the ACDropShip to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const ACDropShip &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ACDropShip, including its inherited members, to their
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
// Virtual method:  DetectObstacle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for obstacles in the travel direction.
// Arguments:       How far ahead of travel direction to check for obstacles.
// Return value:    Which MOID was detected as obstacle. g_NoMOID means nothing was detected.

	MOID DetectObstacle(float distance);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AutoStabilizing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this has the means and will try to right itself, or if
//                  that's up to the Controller to do.
// Arguments:       None.
// Return value:    Wheter this will try to auto stabilize.

	bool AutoStabilizing() override { return true; }


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
// Virtual method:	GetMaxPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The recomended, not absolute, maximum number of actors that fit in the
//                  invetory. Used by the activity AI.
// Arguments:       None.
// Return value:    An integer with the recomended number of actors that fit in the craft.
//                  Default is four.

	int GetMaxPassengers() const override { return m_MaxPassengers > -1 ? m_MaxPassengers : 4; }


    /// <summary>
    /// Gets the right side thruster of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the right side thruster of this ACDropship. Ownership is NOT transferred.</returns>
    AEmitter * GetRightThruster() const { return m_pRThruster; }

    /// <summary>
    /// Sets the right side thruster for this ACDropship.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetRightThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the left side thruster of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the left side thruster of this ACDropship. Ownership is NOT transferred.</returns>
    AEmitter * GetLeftThruster() const { return m_pLThruster; }

    /// <summary>
    /// Sets the left side thruster for this ACDropship.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetLeftThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the right side secondary thruster of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the right side secondary thruster of this ACDropship. Ownership is NOT transferred.</returns>
    AEmitter * GetURightThruster() const { return m_pURThruster; }

    /// <summary>
    /// Sets the right side secondary thruster for this ACDropship.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetURightThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the left side secondary thruster of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the left side secondary thruster of this ACDropship. Ownership is NOT transferred.</returns>
    AEmitter * GetULeftThruster() const { return m_pULThruster; }

    /// <summary>
    /// Sets the left side secondary thruster for this ACDropship.
    /// </summary>
    /// <param name="newThruster">The new thruster to use.</param>
    void SetULeftThruster(AEmitter *newThruster);

    /// <summary>
    /// Gets the left side hatch of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the left side hatch of this ACDropship. Ownership is NOT transferred.</returns>
    Attachable * GetLeftHatch() const { return m_pLHatch; }

    /// <summary>
    /// Sets the left side hatch for this ACDropship.
    /// </summary>
    /// <param name="newHatch">The new hatch to use.</param>
    void SetLeftHatch(Attachable *newHatch);

    /// <summary>
    /// Gets the right side hatch of this ACDropship.
    /// </summary>
    /// <returns>A pointer to the right side hatch of this ACDropship. Ownership is NOT transferred.</returns>
    Attachable * GetRightHatch() const { return m_pRHatch; }

    /// <summary>
    /// Sets the right side hatch for this ACDropship.
    /// </summary>
    /// <param name="newHatch">The new hatch to use.</param>
    void SetRightHatch(Attachable *newHatch);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetEmissionTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reset the timers of all emissions so they will start/stop at the 
//                  correct relative offsets from now.
// Arguments:       None.
// Return value:    None.

    void ResetEmissionTimers() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaxEngineAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get max engine rotation angle in degrees.
// Arguments:       None.
// Return value:    Max engine angle in degrees.

	float GetMaxEngineAngle() const { return m_MaxEngineAngle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMaxEngineAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets max engine rotation angle in degrees.
// Arguments:       Max engine angle in degrees.
// Return value:    None.

	void SetMaxEngineAngle(float newAngle) { m_MaxEngineAngle = newAngle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLateralControlSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the abstract rate of LateralControl change. Default is 6
// Arguments:       None.
// Return value:    Current lateral control speed value.

	float GetLateralControlSpeed() const { return m_LateralControlSpeed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLateralControlSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the abstract rate of LateralControl change. Default is 6
// Arguments:       New lateral control speed value.
// Return value:    None.

	void SetLateralControlSpeed(float newSpeed) { m_LateralControl = newSpeed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLateralControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets lateral control value -1.0 to 1.0 control of sideways movement. 0 means try to stand still in X.
// Arguments:       None.
// Return value:    Current lateral control value.

	float GetLateralControl() const { return m_LateralControl; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


    // Member variables
    static Entity::ClassInfo m_sClass;
    // Body AtomGroups.
    AtomGroup *m_pBodyAG;
    // Thruster emitters.
    //TODO when this class is cleaned up, these and their getters and setters should probably be renamed (I'd argue the lua bindings should be broken to match but that's debatable). L and R should be Left and Right and they should probably be Primary and Secondary.
    AEmitter *m_pRThruster;
    AEmitter *m_pLThruster;
    AEmitter *m_pURThruster;
    AEmitter *m_pULThruster;

    // Hatch doors
    Attachable *m_pRHatch;
    Attachable *m_pLHatch;
    // How much the hatch doors rotate to open
    Matrix m_HatchSwingRange;
    // From 0 to 1.0, the state of hatch door openness
    float m_HatchOpeness;

    // -1.0 to 1.0 control of sideways movement. 0 means try to stand still in X.
    float m_LateralControl;
	// Abstract speed at which Lateralcontroll is changed
	float m_LateralControlSpeed;

    // Automatically stabilize the craft with the upper thrusters? Defaults to yes.
    int m_AutoStabilize;
    // The craft explodes if it has been on its side for more than this many MS (default 4000). Disable by setting to -1.
    float m_ScuttleIfFlippedTime;

	// Maximum engine rotation in degrees
	float m_MaxEngineAngle;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACDropShip, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	ACDropShip(const ACDropShip &reference) = delete;
	ACDropShip & operator=(const ACDropShip &rhs) = delete;

};

} // namespace RTE

#endif // File