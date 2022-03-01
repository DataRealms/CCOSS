#ifndef _RTEACRAFT_
#define _RTEACRAFT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACraft.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ACraft class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"
#include "LimbPath.h"

struct BITMAP;

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ACraft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A flying Actor which carries other things and can drop them.
// Parent(s):       Actor.
// Class history:   12/13/2006 ACraft created.

class ACraft : public Actor {
	friend struct EntityLuaBindings;


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods;
	ClassInfoGetters;


enum HatchState
{
    CLOSED = 0,
    OPENING,
    OPEN,
    CLOSING,
    HatchStateCount
};

enum Side
{
    RIGHT = 0,
    LEFT
};

    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    Exit
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Something to bundle the properties of ACraft exits together.
    // Parent(s):       Serializable.
    // Class history:   12/19/2006 Exit created.

    class Exit:
        public Serializable
    {

    friend class ACraft;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Exit
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Exit object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Exit() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Exit object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

		int Create() override;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a Exit to be identical to another, by deep copy.
    // Arguments:       A reference to the Exit to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

		int Create(const Exit &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        void Reset() override { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  GetOffset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the position offset of this exit from the position of its ACraft.
    // Arguments:       None.
    // Return value:    The coordinates relative to the m_Pos of this' ACraft.

		Vector GetOffset() const { return m_Offset; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  GetVelocity
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the velocity of anything that exits through this.
    // Arguments:       None.
    // Return value:    The velocity vector for anything exiting through this.

        Vector GetVelocity() const { return m_Velocity * (1.0F + m_VelSpread * RandomNormalNum()); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetRadius
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the width from the center tanget created by the velocity vector
    //                  out from the offet point. This times two gives the total width of the
    //                  opening.
    // Arguments:       None.
    // Return value:    Half the total width of the opening.

		float GetRadius() const { return m_Radius; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  GetRange
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the distance this exit can suck in objects from.
    // Arguments:       None.
    // Return value:    The sucking range of this.

		float GetRange() const { return m_Range; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  CheckIfClear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Calculates whether this exit is currently clear enough of terrain to
    //                  safely put things through without them ending up in the terrain.
    // Arguments:       The position of the parent ACraft of this.
    //                  The rotation of the parent ACraft of this.
    //                  How large (radius) the item is that is supposed to fit.
    // Return value:    If this has been determined clear to put anything through.

		bool CheckIfClear(const Vector &pos, Matrix &rot, float size = 20);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  IsClear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Tells if this is clear of the terrain to put things through. Faster than
    //                  CheckIfClear().
    // Arguments:       None.
    // Return value:    If this has been determined clear to put anything through.

		bool IsClear() const { return m_Clear; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:  SuckInMOs
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Uses cast MO rays to see if anyhting is able to be drawn into this
    //                  exit. If so, it will alter the positiona nd velocity of the objet so
    //                  it flies into the exit until it is sufficiently inside and then it'll
    //                  return the MO here, OWNERHIP NOT TRANSFERRED! It is still in MovableMan!
    // Arguments:       A pointer to the ACraft owner of this Exit. OWNERSHIP IS NOT TRANSFERRED!
    // Return value:    If an MO has been fully drawn into the exit, it will be returned here,
    //                  OWNERSHIP NOT TRANSFERRED!

		MOSRotating * SuckInMOs(ACraft *pExitOwner);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // The offset of this exit relative the position of its ACraft
        Vector m_Offset;
        // The exiting velocity of anyhting exiting through this
        Vector m_Velocity;
        // The spread in velocity, ratio
        float m_VelSpread;
        // The width from the center tanget created by the velocity vector out from the offet point. This times two gives the total width of the opening.
        float m_Radius;
        // How far away the exit cna suck objects in from
        float m_Range;
        // Temporary var to check if this is clear of terrain for putting things through
        bool m_Clear;
        // Movable Object that is being drawn into this exit
        MOSRotating *m_pIncomingMO;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Exit, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


// Concrete allocation and cloning definitions
//EntityAllocation(ACraft)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ACraft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ACraft object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ACraft() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ACraft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ACraft object before deletion
//                  from system memory.
// Arguments:       None.

	~ACraft() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACraft object ready for use.
// Arguments:       A Reader that the ACraft will create itself with.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACraft to be identical to another, by deep copy.
// Arguments:       A reference to the ACraft to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const ACraft &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ACraft, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Actor::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this Actor and all its carried
//                  gold and inventory.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
// Return value:    The current value of this Actor and all his carried assets.

	float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically named object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.
// Arguments:       The Preset name of the object to look for.
// Return value:    Whetehr the object was found carried by this.

	bool HasObject(std::string objectName) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObjectInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically grouped object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.
// Arguments:       The name of the group to look for.
// Return value:    Whetehr the object in the group was found carried by this.

	bool HasObjectInGroup(std::string groupName) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHatchState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current state of the hatch.
// Arguments:       None.
// Return value:    An int encoding the hatch state. See the HatchState enum.

    unsigned int GetHatchState() const { return m_HatchState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this belongs to, and all its inventory too.
// Arguments:       The assigned team number.
// Return value:    None.

	void SetTeam(int team) override;



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.
// Arguments:       The pie menu to add slices to. Ownership is NOT transferred!
// Return value:    Whether any slices were added.

   bool AddPieMenuSlices(PieMenuGUI *pPieMenu) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HandlePieCommand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles and does whatever a specific activated Pie Menu slice does to
//                  this.
// Arguments:       The pie menu command to handle. See the PieSliceIndex enum.
// Return value:    Whetehr any slice was handled. False if no matching slice handler was
//                  found, or there was no slice currently activated by the pie menu.

    bool HandlePieCommand(PieSlice::PieSliceIndex pieSliceIndex) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AutoStabilizing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this has the means and will try to right itself, or if
//                  that's up to the Controller to do.
// Arguments:       None.
// Return value:    Wheter this will try to auto stabilize.

    virtual bool AutoStabilizing() { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenHatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the hatch doors, if they're closed or closing.
// Arguments:       None.
// Return value:    None.

	void OpenHatch();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CloseHatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes the hatch doors, if they're open or opening.
// Arguments:       None.
// Return value:    None.

	void CloseHatch();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this Actor.
// Arguments:       An pointer to the new item to add. Ownership IS TRANSFERRED!
// Return value:    None..

	void AddInventoryItem(MovableObject *pItemToAdd) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DropAllInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the hatches and makes everything in the Rocket fly out, including
//                  the passenger Actors, one after another. It may not happen
//                  instantaneously, so check for ejection being complete with
//                  IsInventoryEmpty().
// Arguments:       None.
// Return value:    None.

	void DropAllInventory() override;

	/// <summary>
	/// Gets the mass of this ACraft's inventory of newly collected items.
	/// </summary>
	/// <returns>The mass of this ACraft's newly collected inventory.</returns>
	float GetCollectedInventoryMass() const;

	/// <summary>
	/// Gets the mass of this ACraft, including the mass of its Attachables, wounds and inventory.
	/// </summary>
	/// <returns>The mass of this ACraft, its inventory and all its Attachables and wounds in Kilograms (kg).</returns>
	float GetMass() const override { return Actor::GetMass() + GetCollectedInventoryMass(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasDelivered
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this has landed and delivered yet on its current run.
// Arguments:       None.
// Return value:    Whether this has delivered yet.

    bool HasDelivered() { return m_HasDelivered; }

	/// <summary>
	/// Resets all the timers related to this, including the scuttle timer.
	/// </summary>
	void ResetAllTimers() override;

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.
// Arguments:       The other MO hit. Ownership is not transferred.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this hit.

	bool OnMOHit(MovableObject *pOtherMO) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:	GetMaxPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The recomended, not absolute, maximum number of actors that fit in the
//                  invetory. Used by the activity AI.
// Arguments:       None.
// Return value:    An integer with the recomended number of actors that fit in the craft.
//                  Default is -1 (unknown).

	virtual int GetMaxPassengers() const { return m_MaxPassengers; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:	SetMaxPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the recomended, not absolute, maximum number of actors that fit in the
//                  invetory. Used by the activity AI.
// Arguments:       An integer with the recomended number of actors that fit in the craft.
//                  Default is -1 (unknown).
// Return value:    None.

	virtual void SetMaxPassengers(int max) { m_MaxPassengers = max; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetDeliveryDelayMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns delivery delay multiplier. 
// Arguments:       None.
// Return value:    Delivery delay multiplier. 

	float GetDeliveryDelayMultiplier() const { return m_DeliveryDelayMultiplier; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetDeliveryDelayMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Sets delivery delay multiplier. 
// Arguments:       Delivery delay multiplier. 
// Return value:    None.

	void SetDeliveryDelayMultiplier(float newValue) { m_DeliveryDelayMultiplier = newValue; }


	/// <summary>
	/// Gets whether this ACraft will scuttle automatically on death.
	/// </summary>
	/// <returns>Whether this ACraft will scuttle automatically on death.</returns>
	bool GetScuttleOnDeath() const { return m_ScuttleOnDeath; }

	/// <summary>
	/// Sets whether this ACraft will scuttle automatically on death.
	/// </summary>
	/// <param name="scuttleOnDeath">Whether this ACraft will scuttle automatically on death.</param>
	void SetScuttleOnDeath(bool scuttleOnDeath) { m_ScuttleOnDeath = scuttleOnDeath; }

	/// <summary>
	/// Gets the hatch opening/closing delay of this ACraft.
	/// </summary>
	/// <returns>The hatch delay of this ACraft.</returns>
	int GetHatchDelay() const { return m_HatchDelay; }

	/// <summary>
	/// Sets the hatch opening/closing delay of this ACraft.
	/// </summary>
	/// <param name="newDelay">The new hatch delay of this ACraft.</param>
	void SetHatchDelay(int newDelay) { m_HatchDelay = newDelay; }

	/// <summary>
	/// Destroys this ACraft and creates its specified Gibs in its place with appropriate velocities. Any Attachables are removed and also given appropriate velocities.
	/// </summary>
	/// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
	/// <param name="movableObjectToIgnore">A pointer to an MO which the Gibs and Attachables should not be colliding with.</param>
	void GibThis(const Vector &impactImpulse = Vector(), MovableObject *movableObjectToIgnore = nullptr) override;

	/// <summary>
	/// Gets this ACraft's hatch opening sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this ACraft's hatch opening sound.</returns>
	SoundContainer * GetHatchOpenSound() const { return m_HatchOpenSound; }

	/// <summary>
	/// Sets this ACraft's hatch opening sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this ACraft's hatch opening sound.</param>
	void SetHatchOpenSound(SoundContainer *newSound) { m_HatchOpenSound = newSound; }

	/// <summary>
	/// Gets this ACraft's hatch closing sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this ACraft's hatch closing sound.</returns>
	SoundContainer * GetHatchCloseSound() const { return m_HatchCloseSound; }

	/// <summary>
	/// Sets this ACraft's hatch closing sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this ACraft's hatch closing sound.</param>
	void SetHatchCloseSound(SoundContainer *newSound) { m_HatchCloseSound = newSound; }

	/// <summary>
	/// Gets this ACraft's crash sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this ACraft's crash sound.</returns>
	SoundContainer * GetCrashSound() const { return m_CrashSound; }

	/// <summary>
	/// Sets this ACraft's crash sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this ACraft's crash sound.</param>
	void SetCrashSound(SoundContainer *newSound) { m_CrashSound = newSound; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Current movement state.
    unsigned int m_MoveState;
    // Current hatch action state.
    unsigned int m_HatchState;
    // Timer for opening and closing hatches
    Timer m_HatchTimer;
    // The time it takes to open or close the hatch, in ms.
    int m_HatchDelay;
    // Sound for opening the hatch
    SoundContainer *m_HatchOpenSound;
	// Sound for closing the hatch
	SoundContainer *m_HatchCloseSound;
    std::deque<MovableObject *> m_CollectedInventory;	//!< A separate inventory to temporarily store newly collected items, so that they don't get immediately ejected from the main inventory while the hatch is still open.
    // All the possible exits for when ejecting stuff out of this.
    std::list<Exit> m_Exits;
    // Last used exit so we can alternate/cycle
    std::list<Exit>::iterator m_CurrentExit;
    // The delay between each exiting passenger Actor
    long m_ExitInterval;
    // Times the exit interval
    Timer m_ExitTimer;
    // The phase of the exit lines animation
    int m_ExitLinePhase;
    // Whether this has landed and delivered yet on its current run
    bool m_HasDelivered;
    // Whether this is capable of landing on the ground at all
    bool m_LandingCraft;
    // Timer for checking if craft is hopelessly flipped and should die
    Timer m_FlippedTimer;
    // Timer to measure how long ago a crash sound was played
    Timer m_CrashTimer;
    // Crash sound
    SoundContainer *m_CrashSound;
    // The maximum number of actors that fit in the inventory
    int m_MaxPassengers;
	int m_ScuttleIfFlippedTime; //!< The time after which the craft will scuttle automatically, if tipped over.
	bool m_ScuttleOnDeath; //!< Whether the craft will self-destruct at zero health.

	static bool s_CrabBombInEffect; //!< Flag to determine if a craft is triggering the Crab Bomb effect.

    ////////
    // AI states

    enum CraftDeliverySequence
    {
        FALL = 0,
        LAND,
        STANDBY,
        UNLOAD,
        LAUNCH,
        UNSTICK
    };

    enum AltitudeMoveState
    {
        HOVER = 0,
        DESCEND,
        ASCEND
    };

    // What the rocket/ship is currently doing in an AI landing sequence
    int m_DeliveryState;
    // Whether the AI is trying to go higher, lower, or stand still in altitude
    int m_AltitudeMoveState;
    // Controls the altitude gain/loss the AI is trying to achieve. Normalized -1.0 (max rise) to 1.0 (max drop). 0 is approximate hover.
    float m_AltitudeControl;
	// Mutliplier to apply to default delivery time
	float m_DeliveryDelayMultiplier;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACraft, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	ACraft(const ACraft &reference) = delete;
	ACraft & operator=(const ACraft &rhs) = delete;

};

} // namespace RTE

#endif // File