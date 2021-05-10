#ifndef _RTEACTOR_
#define _RTEACTOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Actor.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Actor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSRotating.h"
#include "PieMenuGUI.h"

namespace RTE
{

class AtomGroup;
class HeldDevice;
class PieMenuGUI;

#define AILINEDOTSPACING 16

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Actor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A sprite movable object that is autonomous.
// Parent(s):       MOSRotating.
// Class history:   04/13/2001 Actor created.

class Actor:
    public MOSRotating
{
    friend class LuaMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    enum Status
    {
        STABLE = 0,
        UNSTABLE,
        INACTIVE,
        DYING,
        DEAD,
        StatusCount
    };

    enum AIMode
    {
        AIMODE_NONE = 0,
        AIMODE_SENTRY,
        AIMODE_PATROL,
        AIMODE_GOTO,
        AIMODE_BRAINHUNT,
        AIMODE_GOLDDIG,
        AIMODE_RETURN,
        AIMODE_STAY,
        AIMODE_SCUTTLE,
        AIMODE_DELIVER,
        AIMODE_BOMB,
        AIMODE_SQUAD,
        AIMODE_COUNT
    };

// Concrete allocation and cloning definitions
EntityAllocation(Actor)
AddScriptFunctionNames(MOSRotating, "UpdateAI")
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Actor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Actor object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Actor() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Actor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Actor object before deletion
//                  from system memory.
// Arguments:       None.

	~Actor() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Actor object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Actor to be identical to another, by deep copy.
// Arguments:       A reference to the Actor to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const Actor &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Actor, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); MOSRotating::Reset(); m_MOType = MovableObject::TypeActor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;

    /// <summary>
    /// Loads the script at the given script path onto the object, checking for appropriately named functions within it.
    /// </summary>
    /// <param name="scriptPath">The path to the script to load.</param>
    /// <param name="loadAsEnabledScript">Whether or not the script should load as enabled. Defaults to true.</param>
    /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	int LoadScript(std::string const &scriptPath, bool loadAsEnabledScript = false) override;

    /// <summary>
    /// Gets the mass of this Actor's inventory. Does not include any equipped item (for actor subtypes that have that).
    /// </summary>
    /// <returns>The mass of this Actor's inventory.</returns>
    float GetInventoryMass() const;

    /// <summary>
    /// Gets the mass of this Actor, including the mass of its Attachables, wounds and inventory.
    /// </summary>
    /// <returns>The mass of this Actor, its inventory and all its Attachables and wounds in Kilograms (kg).</returns>
    float GetMass() const override { return MOSRotating::GetMass() + GetInventoryMass() + (m_GoldCarried * g_SceneMan.GetKgPerOz()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's Controller. Ownership IS NOT transferred!
// Arguments:       None.
// Return value:    A const pointer to this Actor's Controller.

    Controller * GetController() { return &m_Controller; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlayerControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a player is currently controlling this.
// Arguments:       None.
// Return value:    Whether a player is controlling this.

    bool IsPlayerControlled() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsControllable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells wheter the player can switch control to this at all
// Arguments:       None.
// Return value:    Whether a player can control this at all.

    virtual bool IsControllable() const { return true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStatus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current Status of this.
// Arguments:       None.
// Return value:    The status.

    int GetStatus() const { return m_Status; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHealth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's health value.
// Arguments:       None.
// Return value:    A const int describing this Actor's health.

    int GetHealth() const { return m_Health; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaxHealth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's maximum health value.
// Arguments:       None.
// Return value:    A const int describing this Actor's max health.

    int GetMaxHealth() const { return m_MaxHealth; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaxHealth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's maximum health value.
// Arguments:       New max health value.
// Return value:    None.

    void SetMaxHealth(int newValue) { m_MaxHealth = newValue; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAimDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the distance between the actor and the view point when not
//                  sharp aiming.
// Arguments:       None.
// Return value:    A const int describing how far this actor aims/looks by default.

    int GetAimDistance() const { return m_AimDistance; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAimDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the distance between the actor and the view point when not
//                  sharp aiming.
// Arguments:       None.
// Return value:    A const int describing how far this actor aims/looks by default.

	void SetAimDistance( int newValue ) { m_AimDistance = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldCarried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many ounces of gold this Actor is carrying.
// Arguments:       None.
// Return value:    The current amount of carried gold, in Oz.

    virtual float GetGoldCarried() const { return m_GoldCarried; }


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
// Method:          GetTotalValueOld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     DOES THE SAME THING AS GetTotalValue, USED ONLY TO PRESERVE LUA COMPATIBILITY

	float GetTotalValueOld(int nativeModule = 0, float foreignMult = 1.0) const override { return GetTotalValue(nativeModule, foreignMult, 1.0); }


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
// Method:          GetAimAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's aim angle.
// Arguments:       Whether to adjust the angle for flipping or not.
// Return value:    The angle, in radians.

    float GetAimAngle(bool adjustForFlipped = true) const { return adjustForFlipped ? FacingAngle(m_AimAngle) : m_AimAngle; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPassengerSlots
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this Actor's passenger slots.
// Arguments:       None.
// Return value:    The Actor's passenger plots

    int GetPassengerSlots() const { return m_PassengerSlots; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetCPUPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' brain, or equivalent.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' brain.

    virtual Vector GetCPUPos() const { return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' eye or view point.

    virtual Vector GetEyePos() const { return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAboveHUDPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of the top of this' HUD stack.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' HUD stack top point.

	Vector GetAboveHUDPos() const override { return m_Pos + Vector(0, m_HUDStack + 6); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetViewPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the point at which this actor is viewing, or that the scene frame
//                  should be centered on if tracking this Actor's view. In absolute scene
//                  coordinates.
// Arguments:       None.
// Return value:    The point in absolute scene coordinates.

    Vector GetViewPoint() const { return m_ViewPoint.IsZero() ? m_Pos : m_ViewPoint; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLookVector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the direction where this is looking/aiming.
// Arguments:       None.
// Return value:    A Vector with the direction in which this is looking along.

	Vector GetLookVector() const { return m_ViewPoint - GetEyePos(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSharpAimProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the normalized amount of sharp aim that has been achieved by this. 
// Arguments:       None.
// Return value:    Sharp aim progress between 0 - 1.0. 1.0 is fully aimed.

    float GetSharpAimProgress() const { return m_SharpAimProgress; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the approximate height of this Actor, standing up.
// Arguments:       None.
// Return value:    A float with the approximate height, in pixels.

	float GetHeight() const { return m_CharHeight; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetControllerMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's new Controller input mode.
// Arguments:       The new input mode.
//                  The player which will control this if the input mode was set to player.
// Return value:    None.

    void SetControllerMode(Controller::InputMode newMode, int newPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapControllerModes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's Controller mode and gives back what it used to be.
// Arguments:       The new mode to set to.
//                  The player which will control this if the input mode was set to player.
// Return value:    The old mode that it had before.

    Controller::InputMode SwapControllerModes(Controller::InputMode newMode, int newPlayer = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStatus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's status.
// Arguments:       A Status enumeration.
// Return value:    None.

    void SetStatus(Actor::Status newStatus) { m_Status = newStatus; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this Actor belongs to.
// Arguments:       The assigned team number.
// Return value:    None.

	void SetTeam(int team) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGoldCarried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how many ounces of gold this Actor is carrying.
// Arguments:       The new amount of carried gold, in Oz.
// Return value:    None.

    void SetGoldCarried(float goldOz) { m_GoldCarried = goldOz; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAimAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's aim angle.
// Arguments:       A new angle, in radians.
// Return value:    None.

    void SetAimAngle(float newAngle) { m_AimAngle = newAngle; Clamp(m_AimAngle, m_AimRange, -m_AimRange); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPassengerSlots
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's passenger slots.
// Arguments:       A new amount of passenger slots.
// Return value:    None.

    void SetPassengerSlots(int newPassengerSlots) { m_PassengerSlots = newPassengerSlots;; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetViewPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the point at which this actor is viewing, or that the scene frame
//                  should be centered on if tracking this Actor's view. In absolute scene
//                  coordinates.
// Arguments:       A new point in absolute scene coords.
// Return value:    None.

    void SetViewPoint(Vector newPoint) { m_ViewPoint = newPoint; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetItemInReach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the item that is within reach of the Actor at this frame, so that
//                  it may be picked up. Ownership is NOT transferred!
// Arguments:       A pointer to the item that has been determined to be within reach of
//                  this Actor. Ownership is NOT transferred!
// Return value:    None.

    void SetItemInReach(HeldDevice *pItem) { m_pItemInReach = pItem; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsWithinRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is within range of the currently
//                  used device and aiming status, if applicable.
// Arguments:       A Vector witht he aboslute coordinates of a point to check.
// Return value:    Whether the point is within range of this.

    virtual bool IsWithinRange(Vector &point) const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  The range, in pixels, that the ray will have.
// Return value:    Whether any unseen pixels were revealed by this look.

    virtual bool Look(float FOVSpread, float range);

/* Old version, we don't let the actors carry gold anymore, goes directly to the team funds instead
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain amount of ounces of gold to teh currently carried
//                  amount.
// Arguments:       The amount in Oz with which to change the current gol dtally of this
//                  Actor.
// Return value:    None.

    void AddGold(float goldOz) { m_GoldCarried += std::ceil(goldOz); m_GoldPicked = true; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain amount of ounces of gold to this' team's total funds.
// Arguments:       The amount in Oz with which to change this' team's gold tally.
// Return value:    None.

    void AddGold(float goldOz);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    void RestDetection() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddHealth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds health points to this Actor's current health value.
// Arguments:       An int specifying the value to add.
// Return value:    The resulting total health of this Actor.

    const int AddHealth(const int addedHealth) { return m_Health += addedHealth; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHealth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's current health value.
// Arguments:       An int specifying the value to set to.
// Return value:    The resulting total health of this Actor.

    void SetHealth(const int setHealth) { m_Health = setHealth; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsStatus
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if this Actor is in a specific status.
// Arguments:       Which status to check for.
// Return value:    A bool with the answer.

    bool IsStatus(Status which) const { return m_Status == which; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDead
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if this Actor is dead.
// Arguments:       None.
// Return value:    A const bool with the answer.

    bool IsDead() const { return m_Status == DEAD; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PieNeedsUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates that the pie menu associated with this Actor needs updating.
// Arguments:       None.
// Return value:    Whether the pie menu needs updating.

    bool PieNeedsUpdate() { return m_PieNeedsUpdate; };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.
// Arguments:       The pie menu to add slices to. Ownership is NOT transferred!
// Return value:    Whether any slices were added.

	virtual bool AddPieMenuSlices(PieMenuGUI *pPieMenu);


    /// <summary>
    /// Handles and does whatever a specific activated Pie Menu slice does to this.
    /// </summary>
    /// <param name="pieSliceIndex">The pie menu command to handle. See the enum in PieSlice.</param>
    /// <returns>Whether any slice was handled. False if no matching slice handler was found, or there was no slice currently activated by the pie menu.</returns>
    virtual bool HandlePieCommand(PieSlice::PieSliceIndex pieSliceIndex) { return false; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the AI states for this.
// Arguments:       None.
// Return value:    None.

    void ResetAI() { m_AIMode; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAIMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this' AI mode.
// Arguments:       None.
// Return value:    The current AI mode.

	int GetAIMode() const { return m_AIMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAIModeIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the icon bitmap associated with this' current AI mode and team.
// Arguments:       None.
// Return value:    The current AI mode icon of this. Ownership is NOT transferred!

	BITMAP * GetAIModeIcon();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetAIMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this' AI mode.
// Arguments:       The new AI mode.
// Return value:    None.

	void SetAIMode(AIMode newMode = AIMODE_SENTRY) { m_AIMode = newMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddAISceneWaypoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an absolute scene point to the list of waypoints this is going to
//                  go to, in order
// Arguments:       The new scene point this should try to get to after all other waypoints
//                  are reached.
// Return value:    None.

	void AddAISceneWaypoint(const Vector &waypoint) { m_Waypoints.push_back(std::pair<Vector, MovableObject *>(waypoint, (MovableObject*)NULL)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddAIMOWaypoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an MO in the scene as the next waypoint for this to go to, in order
// Arguments:       The new MO this should try to get to after all other waypoints are reached.
//                  OWNERSHIP IS NOT TRANSFERRED!
// Return value:    None.

	void AddAIMOWaypoint(const MovableObject *pMOWaypoint);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearAIWaypoints
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all AI waypoints and clears the current path to the current
//                  waypoint. The AI Actor will stop in its tracks.
// Arguments:       None.
// Return value:    None.

	void ClearAIWaypoints() { m_pMOMoveTarget = 0; m_Waypoints.clear(); m_MovePath.clear(); m_MoveTarget = m_Pos; m_MoveVector.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLastAIWaypoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the last or furthest set AI waypoint of this. If none, this' pos
//                  is returned.
// Arguments:       None.
// Return value:    The furthest set AI waypoint of this.

	Vector GetLastAIWaypoint() { if (!m_Waypoints.empty()) { return m_Waypoints.back().first; } else if (!m_MovePath.empty()) { return m_MovePath.back(); } return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLastMOWaypointID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of the last set AI MO waypoint of this. If none, g_NoMOID is returned.
// Arguments:       None.
// Return value:    The furthest set AI MO waypoint of this.

	MOID GetAIMOWaypointID();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetWaypointsSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many waypoints this actor have.
// Arguments:       None.
// Return value:    How many waypoints.

	int GetWaypointsSize() { return m_Waypoints.size(); };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of coordinates in this' current MovePath, ie the path
//                  to the next Waypoint.
// Arguments:       None.
// Return value:    None.

	void ClearMovePath() { m_MovePath.clear(); m_MoveTarget = m_Pos; m_MoveVector.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddToMovePathBeginning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a coordinate to the beginning of the MovePath, meaning the one
//                  closest to this Actor.
// Arguments:       The new coordinate to add to the front of the MovePath.
// Return value:    None.

	void AddToMovePathBeginning(Vector newCoordinate) { m_MovePath.push_front(newCoordinate); m_MoveTarget = newCoordinate; m_MoveVector.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddToMovePathEnd
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a coordinate to the end of the MovePath, meaning the one
//                  closest to this Actor's next waypoint.
// Arguments:       The new coordinate to add to the end of the MovePath.
// Return value:    None.

	void AddToMovePathEnd(Vector newCoordinate) { m_MovePath.push_back(newCoordinate); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  RemoveMovePathBeginning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a coordinate from the beginning of the MovePath, meaning the
//                  one closest to this Actor.
// Arguments:       None.
// Return value:    Whether there was any coordinate to remove. If false, the MovePath
//                  is empty.

	bool RemoveMovePathBeginning() { if (!m_MovePath.empty()) { m_MovePath.pop_front(); m_MoveTarget = m_MovePath.empty() ? m_Pos : m_MovePath.front(); m_MoveVector.Reset(); return true; } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveMovePathEnd
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a coordinate from the end of the MovePath, meaning the
//                  one farthest from this Actor.
// Arguments:       None.
// Return value:    Whether there was any coordinate to remove. If false, the MovePath
//                  is empty.

	bool RemoveMovePathEnd() { if (!m_MovePath.empty()) { m_MovePath.pop_back(); return true; } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetPerceptiveness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this' perceptiveness to alarming events going on around him.
// Arguments:       The current perceptiveness, 0.0 - 1.0
// Return value:    None.

	void SetPerceptiveness(float newPerceptiveness) { m_Perceptiveness = newPerceptiveness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetPerceptiveness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets this' perceptiveness to alarming events going on around him.
// Arguments:       None.
// Return value:    The current perceptiveness, 0.0 - 1.0

	float GetPerceptiveness() const { return m_Perceptiveness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  AlarmPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this alarmed about a certian point on in the scene, overriding
//                  the current AI mode until a certain time has passed.
// Arguments:       The new scene point this should look at and see if anything dangerous
//                  is there.
// Return value:    None.

	void AlarmPoint(const Vector &alarmPoint) { if (m_AlarmTimer.GetElapsedSimTimeMS() > 50) { m_AlarmTimer.Reset(); m_LastAlarmPos = m_PointingTarget = alarmPoint; if (m_AlarmSound) { m_AlarmSound->Play(alarmPoint); } } }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetAlarmPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any point on the scene this actor should be alarmed about this frame.
// Arguments:       None.
// Return value:    The new scene point this should look at and see if anything dangerous
//                  is there or (0,0) if nothing is alarming.

	Vector GetAlarmPoint() { if (m_AlarmTimer.GetElapsedSimTimeMS() > g_TimerMan.GetDeltaTimeMS()) { return Vector(); } return m_LastAlarmPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this Actor.
// Arguments:       An pointer to the new item to add. Ownership IS TRANSFERRED!
// Return value:    None..

    virtual void AddInventoryItem(MovableObject *pItemToAdd) { if (pItemToAdd) { m_Inventory.emplace_back(pItemToAdd); } }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:  RemoveInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified item from the actor's inventory. Only one item is removed at a time.
// Arguments:       Preset name of an item to remove.
// Return value:    None.

	void RemoveInventoryItem(string presetName);

    /// <summary>
    /// Removes and returns the inventory item at the given index. Ownership IS transferred.
    /// </summary>
    /// <param name="inventoryIndex">The index of the inventory item to remove.</param>
    /// <returns>An owning pointer to the removed inventory item.</returns>
    MovableObject * RemoveInventoryItemAtIndex(int inventoryIndex);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SwapNextInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Swaps the next MovableObject carried by this Actor and puts one not
//                  currently carried into the into the back of the inventory of this.
// Arguments:       A pointer to the external MovableObject to trade in. Ownership IS xferred!
//                  If 0 is passed in, nothing will be added to the inventory.
//                  Whether to mute the sound on this event. Override for the loading screen hack.
// Return value:    The next MovableObject in this Actor's inventory. Ownership IS xferred!
//                  If there are no MovableObject:s in inventory, 0 will be returned.

	MovableObject * SwapNextInventory(MovableObject *pSwapIn = 0, bool muteSound = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SwapPrevInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Swaps the prev MovableObject carried by this Actor and puts one not
//                  currently carried into the into the back of the inventory of this.
// Arguments:       A pointer to the external MovableObject to trade in. Ownership IS xferred!
//                  If 0 is passed in, nothing will be added to the inventory.
// Return value:    The prev MovableObject in this Actor's inventory. Ownership IS xferred!
//                  If there are no MovableObject:s in inventory, 0 will be returned.

	MovableObject * SwapPrevInventory(MovableObject *pSwapIn = 0);

    /// <summary>
    /// Swaps the inventory items at the given indices. Will return false if a given index is invalid.
    /// </summary>
    /// <param name="inventoryIndex1">The index of one item.</param>
    /// <param name="inventoryIndex2">The index of the other item.</param>
    /// <returns>Whether or not the swap was successful.</returns>
    bool SwapInventoryItemsByIndex(int inventoryIndex1, int inventoryIndex2);

    /// <summary>
    /// Sets the inventory item at the given index as the new inventory item, and gives back the one that was originally there.
    /// If an invalid index is given, the new item will be put in the back of the inventory, and nullptr will be returned.
    /// </summary>
    /// <param name="newInventoryItem">The new item that should be at the given inventory index. Cannot be a nullptr. Ownership IS transferred.</param>
    /// <param name="inventoryIndex">The inventory index the new item should be placed at.</param>
    /// <returns>The inventory item that used to be at the inventory index. Ownership IS transferred.</returns>
    MovableObject * SetInventoryItemAtIndex(MovableObject *newInventoryItem, int inventoryIndex);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DropAllInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Ejects all inventory items that this is carrying. It may not happen
//                  instantaneously, so check for ejection being complete with
//                  IsInventoryEmpty().
// Arguments:       None.
// Return value:    None.

    virtual void DropAllInventory();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetInventorySize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells how many things are in the invetory
// Arguments:       None.
// Return value:    The number of things in the inventory

	int GetInventorySize() const { return m_Inventory.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  IsInventoryEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether inventory is completely empty
// Arguments:       None.
// Return value:    Whether inventory is completely empty.

	bool IsInventoryEmpty() { return m_Inventory.empty(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the deque of inventory of this. Ownership is NOT transferred.
// Arguments:       None.
// Return value:    A const pointer to the inventory deque of this. OWNERSHIP IS NOT TRANSFERRED!

	const std::deque<MovableObject *> * GetInventory() { return &m_Inventory; }

	/// <summary>
	/// Returns the maximum total mass this Actor can carry in its inventory.
	/// </summary>
	/// <returns>The maximum carriable mass of this Actor.</returns>
	float GetMaxInventoryMass() const { return m_MaxInventoryMass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetAimRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The limit of this actors aiming angle, in each direction, in radians.
// Arguments:       None.
// Return value:    The arc range of the aiming angle in radians.
//                  Eg if HalfPI, it means full 180 degree range

	float GetAimRange() const { return m_AimRange; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetAimRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the limit of this actors aiming angle, in each direction, in radians.
// Arguments:       The arc range of the aiming angle in radians.
//                  Eg if HalfPI, it means full 180 degree range
// Return value:    None.

	void SetAimRange(float range) { m_AimRange = range; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlashWhite
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells to make this and all children get drawn as completely white, but
//                  only for a specified amount of time.
// Arguments:       For how long to flash the whiteness, in MS.
// Return value:    None.

    void FlashWhite(int howLongMS = 32) { m_FlashWhiteMS = howLongMS; m_WhiteFlashTimer.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawWaypoints
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this draw its current waypoints and related data on the scene in
//                  its HUD drawing stage.
// Arguments:       Whether to enable or disable the drawing of the waypoints.
// Return value:    None.

    void DrawWaypoints(bool drawWaypoints = true) { m_DrawWaypoints = drawWaypoints; }


    /// <summary>
    /// Destroys this MOSRotating and creates its specified Gibs in its place with appropriate velocities.
    /// Any Attachables are removed and also given appropriate velocities.
    /// </summary>
    /// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
    /// <param name="movableObjectToIgnore">A pointer to an MO which the Gibs and Attachables should not be colliding with.</param>
    void GibThis(const Vector &impactImpulse = Vector(), MovableObject *movableObjectToIgnore = nullptr) override;


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

    bool CollideAtPoint(HitData &hitData) override;


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

	bool ParticlePenetration(HitData &hd) override;


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
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

	void PreTravel() override { MOSRotating::PreTravel(); m_GoldPicked = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMovePathToUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this' AI's move path to be updated. Will update the path to the
//                  current waypoint, if any.
// Arguments:       None.
// Return value:    None.

	void SetMovePathToUpdate() { m_UpdateMovePath = true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMovePathSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many waypoints there are in the MovePath currently
// Arguments:       None.
// Return value:    The number of waypoints in the MovePath.

	int GetMovePathSize() const { return m_MovePath.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' move path RIGHT NOW. Will update the path to the
//                  current waypoint, if any. CAVEAT: this only actually updates if a queue
//                  index number passed in is sufficiently close to 0 to allow this to
//                  compute, based on an internal global assessment of how often this very
//                  expensive computation is allowed to run.
// Arguments:       The queue number this was given the last time
// Return value:    Whether the update was performed, or if it should be tried again next
//                  frame.

    virtual bool UpdateMovePath();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  UpdateAIScripted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state with the provided scripted AI Update function.
// Arguments:       None.
// Return value:    Whether there was an AI Update function defined for this in its script,
//                  and if it was executed successfully.

	bool UpdateAIScripted();


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
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDeploymentID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Sets deployment ID for this actor
// Arguments:       New deployment id.
// Return value:    None.

	void SetDeploymentID(unsigned int newID) { m_DeploymentID = newID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetDeploymentID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Gets deployment ID of this actor
// Arguments:       None.
// Return value:    Returns deployment id of this actor.

	unsigned int GetDeploymentID() const { return m_DeploymentID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSightDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns actor's sight distance.
// Arguments:       None.
// Return value:    Returns actor's sight distance.

	float GetSightDistance() const { return m_SightDistance; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSightDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Sets actor's sight distance.
// Arguments:       New sight distance value.
// Return value:    None.

	void SetSightDistance(float newValue) { m_SightDistance = newValue; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


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
// Method:          VerifyMOIDIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Verifieis whether all actor's MO has correct IDs. Should be used in Debug mode only.
// Arguments:       None.
// Return value:    None.

	void VerifyMOIDs();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTravelImpulseDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns Threshold for taking damage from travel impulses, in kg * m/s
// Arguments:       None.
// Return value:    Threshold for taking damage from travel impulses, in kg * m/s

	float GetTravelImpulseDamage() const { return m_TravelImpulseDamage; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTravelImpulseDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets Threshold for taking damage from travel impulses, in kg * m/s
// Arguments:       Threshold for taking damage from travel impulses, in kg * m/s
// Return value:    None.

	void SetTravelImpulseDamage(float value) { m_TravelImpulseDamage = value; }


	/// <summary>
	/// Gets this Actor's body hit sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this Actor's body hit sound.</returns>
	SoundContainer * GetBodyHitSound() const { return m_BodyHitSound; }

	/// <summary>
	/// Sets this Actor's body hit sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this Actor's body hit sound.</param>
	void SetBodyHitSound(SoundContainer *newSound) { m_BodyHitSound = newSound; }

	/// <summary>
	/// Gets this Actor's alarm sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this Actor's alarm sound.</returns>
	SoundContainer * GetAlarmSound() const { return m_AlarmSound; }

	/// <summary>
	/// Sets this Actor's alarm sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this Actor's alarm sound.</param>
	void SetAlarmSound(SoundContainer *newSound) { m_AlarmSound = newSound; }

	/// <summary>
	/// Gets this Actor's pain sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this Actor's pain sound.</returns>
	SoundContainer * GetPainSound() const { return m_PainSound; }

	/// <summary>
	/// Sets this Actor's pain sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this Actor's pain sound.</param>
	void SetPainSound(SoundContainer *newSound) { m_PainSound = newSound; }

	/// <summary>
	/// Gets this Actor's death sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this Actor's death sound.</returns>
	SoundContainer * GetDeathSound() const { return m_DeathSound; }

	/// <summary>
	/// Sets this Actor's death sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this Actor's death sound.</param>
	void SetDeathSound(SoundContainer *newSound) { m_DeathSound = newSound; }

	/// <summary>
	/// Gets this Actor's device switch sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this Actor's device switch sound.</returns>
	SoundContainer * GetDeviceSwitchSound() const { return m_DeviceSwitchSound; }

	/// <summary>
	/// Sets this Actor's device switch sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this Actor's device switch sound.</param>
	void SetDeviceSwitchSound(SoundContainer *newSound) { m_DeviceSwitchSound = newSound; }

	/// <summary>
	/// Gets the X and Y thresholds for how fast the actor can travel before losing stability.
	/// </summary>
	/// <returns>A Vector with the X and Y thresholds for how fast the actor can travel before losing stability.</returns>
	Vector GetStableVel() const { return m_StableVel; }

	/// <summary>
	/// Sets the X and Y thresholds for how fast the actor can travel before losing stability.
	/// </summary>
	/// <param name="newVelX">New value for how fast the actor can travel before losing stability on X axis.</param>
	/// <param name="newVelY">New value for how fast the actor can travel before losing stability on Y axis.</param>
	void SetStableVel(float newVelX, float newVelY) { m_StableVel.SetXY(newVelX, newVelY); }

	/// <summary>
	/// Sets the X and Y thresholds for how fast the actor can travel before losing stability.
	/// </summary>
	/// <param name="newVelVector">Vector with new values for how fast the actor can travel before losing stability on both axis.</param>
	void SetStableVel(Vector newVelVector) { m_StableVel = newVelVector; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    enum ActionState
    {
        MOVING = 0,
        MOVING_FAST,
        FIRING,
        ActionStateCount
    };

    enum AimState
    {
        AIMSTILL = 0,
        AIMUP,
        AIMDOWN,
        AimStateCount
    };

    AtomGroup *m_pHitBody;
    Controller m_Controller;

    // Sounds
    SoundContainer *m_BodyHitSound;
    SoundContainer *m_AlarmSound;
    SoundContainer *m_PainSound;
    SoundContainer *m_DeathSound;
    SoundContainer *m_DeviceSwitchSound;

//    bool m_FacingRight;
    int m_Status;
    float m_Health;
    // Maximum health
    float m_MaxHealth;
    // The health of the previous frame, so we can track damage
    float m_PrevHealth;
    // Not owned by this!
    const Icon *m_pTeamIcon;
	// Not owned by this!
	const Icon *m_pControllerIcon;
    // Timing the last second to store the position each second so we can determine larger movement
    Timer m_LastSecondTimer;
    // This' position up to a second ago
    Vector m_LastSecondPos;
    // Movement since last whole second
    Vector m_RecentMovement;
    float m_RecentMovementMag;
    // Threshold for taking damage from travel impulses, in kg * m/s
    float m_TravelImpulseDamage;
    // Timer for timing the delay before regaining stability after losing it
    Timer m_StableRecoverTimer;
    // Thresholds in both x and y for how fast the actor can travel before losing stability. Meters per second (m/s).
    Vector m_StableVel;
    // Timer for the heartbeat of this Actor
    Timer m_HeartBeat;
    // Timer for timing how long this has been under Control
    Timer m_NewControlTmr;
    // Death timing timer
    Timer m_DeathTmr;
    // Amount of Gold carried, in ounces.
    float m_GoldCarried;
    // Whether or not any gold was picked up this frame.
    bool m_GoldPicked;
    // Aiming state
    char m_AimState;
    // The arc range of the aiming angle, in each direction, in radians. Eg if HalfPI, it means full 180 degree range
    float m_AimRange;
    // Current Aim angle within the AimRange
    float m_AimAngle;
    // How far the actor aims/looks by default
    float m_AimDistance;
    // Aiming timing timer
    Timer m_AimTmr;
    // For timing the transition between regular aim and sharp aim
    Timer m_SharpAimTimer;
    // The time it takes to achieve complete full sharp aiming
    int m_SharpAimDelay;
    // The velocity
    float m_SharpAimSpeed;
    // Normalzied scalar showing storing much sharp aim progress has been made
    float m_SharpAimProgress;
    // If sharp aim has been maxed out, ie it's either at its max, or being limited by some obstruction
    bool m_SharpAimMaxedOut;
    // Point at this target when devicestate is in POINTING mode
    Vector m_PointingTarget;
    // Last seen enemy target
    Vector m_SeenTargetPos;
    // Timer measuring how long this has been alarmed by a nearby gunshot etc.
    Timer m_AlarmTimer;
    // Position of the last thing that alarmed us
    Vector m_LastAlarmPos;
    // How far this guy's AI can see when he's just looking ahead
    float m_SightDistance;
    // How perceptive this is of alarming events going on around him, 0.0 - 1.0
    float m_Perceptiveness;
    // About How tall is the Actor, in pixels?
    float m_CharHeight;
    // Speed at which the m_AimAngle will change, in radians/s.
//    float
    // The offset position of the holster where this Actor draws his devices from.
    Vector m_HolsterOffset;
    // The point at which this actor is viewing, or the scene frame
    // should be centered on if tracking this Actor's view.
    // In absolute scene coordinates.
    Vector m_ViewPoint;
    // The inventory of carried MovableObjects of this Actor. They are also Owned by this.
    std::deque<MovableObject *> m_Inventory;
    float m_MaxInventoryMass; //!< The mass limit for this Actor's inventory. -1 means there's no limit.
    // The device that can/will be picked up
    HeldDevice *m_pItemInReach;
    // Whether the pie menu associated with this needs updating
    bool m_PieNeedsUpdate;
    // HUD positioning aid
    int m_HUDStack;
    // For how much longer to draw this as white. 0 means don't draw as white
    int m_FlashWhiteMS;
    // The timer that measures and deducts past time from the remaining white flash time
    Timer m_WhiteFlashTimer;
    // Extra pie menu options that this should add to any Pie Menu that focuses on this
    std::list<PieSlice> m_PieSlices;
    // What material strength this actor is capable of digging trough.
    float m_DigStrength;
	// ID of deployment which spawned this actor
	unsigned int m_DeploymentID;
    // How many passenger slots this actor will take in a craft
    int m_PassengerSlots;


    ////////////////////
    // AI States

    enum LateralMoveState
    {
        LAT_STILL = 0,
        LAT_LEFT,
        LAT_RIGHT
    };

    enum ObstacleState
    {
        PROCEEDING = 0,
        BACKSTEPPING,
        DIGPAUSING,
        JUMPING,
        SOFTLANDING
    };

    enum TeamBlockState
    {
         NOTBLOCKED = 0,
         BLOCKED,
         IGNORINGBLOCK,
         FOLLOWWAIT
    };
    // Unknown team icon
    static BITMAP **m_apNoTeamIcon;
    // The AI mode icons
    static BITMAP *m_apAIIcons[AIMODE_COUNT];
    // Selection arrow
    static BITMAP **m_apSelectArrow;
    // Selection arrow
    static BITMAP **m_apAlarmExclamation;
    // Whether the static icons have been loaded yet or not
    static bool m_sIconsLoaded;
    // Whether a Lua update AI function was provided in this' script file
    bool m_ScriptedAIUpdate;
    // The current mode the AI is set to perform as
    AIMode m_AIMode;
    // The list of waypoints remaining between which the paths are made. If this is empty, the last path is in teh MovePath
    // The MO pointer in the pair is nonzero if the waypoint is tied to an MO in the scene, and gets updated each UpdateAI. This needs to be checked for validity/existence each UpdateAI
    std::list<std::pair<Vector, const MovableObject *> > m_Waypoints;
    // Whether to draw the waypoints or not in the HUD
    bool m_DrawWaypoints;
    // Absolute target to move to on the scene; this is usually the point at the front of the movepath list
    Vector m_MoveTarget;
    // The MO we're currently following, if any. If still valid, this' position will update the MoveTarget each UpdateAI.
    const MovableObject *m_pMOMoveTarget;
    // The point previous on the path to the one currently assigned the move target
    Vector m_PrevPathTarget;
    // The relative, scene-wrapped difference between the current m_Pos and the m_MoveTarget.
    Vector m_MoveVector;
    // The calculated path to get to that move-to target
    std::list<Vector> m_MovePath;
    // Whether it's time to update the path
    bool m_UpdateMovePath;
    // The minimum range to consider having reached a move target is considered
    float m_MoveProximityLimit;
    // Whether the AI is trying to progress to the right, left, or stand still
    LateralMoveState m_LateralMoveState;
    // Timer for how long to keep going before switching directions when moving along a path
    Timer m_MoveOvershootTimer;
    // Whether the AI is in the process of proceeding, backstepping to get out of being stuck, or jumping over stuff
    ObstacleState m_ObstacleState;
    // Teammate is in the way of whatever we are doing; stop until he moves
    TeamBlockState m_TeamBlockState;
    // Times how long after an obstruction is cleared to start proceeding again
    Timer m_BlockTimer;
    // The closest the actor has ever come to the current waypoint it's going for. Used to checking if we shuold re-update the movepath
    // It's useful for when the path seems to be broken or unreachable
    float m_BestTargetProximity;
    // Timer used to check on larger movement progress toward the goal
    Timer m_ProgressTimer;
    // Timer used to time how long we've been stuck in the same spot.
    Timer m_StuckTimer;
    // Timer for measuring interval between height checks
    Timer m_FallTimer;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Actor, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
	Actor(const Actor &reference) = delete;
	Actor & operator=(const Actor &rhs) = delete;

};

} // namespace RTE

#endif // File