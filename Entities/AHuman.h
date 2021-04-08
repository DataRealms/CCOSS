#ifndef _RTEAHUMAN_
#define _RTEAHUMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            AHuman.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the AHuman class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"
#include "Arm.h"
#include "Leg.h"
#include "LimbPath.h"

struct BITMAP;

namespace RTE
{

class AEmitter;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           AHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A humanoid actor.
// Parent(s):       Actor.
// Class history:   05/24/2001 AHuman created.

class AHuman:
    public Actor
{


enum UpperBodyState
{
    WEAPON_READY = 0,
    AIMING_SHARP,
    HOLSTERING_BACK,
    HOLSTERING_BELT,
    DEHOLSTERING_BACK,
    DEHOLSTERING_BELT,
    THROWING_PREP,
    THROWING_RELEASE
};

enum MovementState
{
    NOMOVE = 0,
    STAND,
    WALK,
    CROUCH,
    CRAWL,
    ARMCRAWL,
    CLIMB,
    JUMP,
    DISLODGE,
    MOVEMENTSTATECOUNT
};

enum ProneState
{
    NOTPRONE = 0,
    GOPRONE,
    PRONE,
    PRONESTATECOUNT
};

enum Layer
{
    FGROUND = 0,
    BGROUND
};


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(AHuman)
AddScriptFunctionNames(Actor, "OnStride")
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AHuman object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    AHuman() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AHuman object before deletion
//                  from system memory.
// Arguments:       None.

	~AHuman() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AHuman object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AHuman to be identical to another, by deep copy.
// Arguments:       A reference to the AHuman to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const AHuman &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire AHuman, including its inherited members, to their
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
// Method:          GetGoldCarried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many ounces of gold this Actor is carrying.
// Arguments:       None.
// Return value:    The current amount of carried gold, in Oz.

    float GetGoldCarried() const override { return m_GoldCarried + m_GoldInInventoryChunk; }


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
// Virtual method:  GetCPUPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' brain, or equivalent.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' brain.

	Vector GetCPUPos() const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' eye or view point.

    Vector GetEyePos() const override;


    /// <summary>
    /// Gets the head of this AHuman.
    /// </summary>
    /// <returns>A pointer to the head of this AHuman. Ownership is NOT transferred.</returns>
    Attachable * GetHead() const { return m_pHead; }

    /// <summary>
    /// Sets the head for this AHuman.
    /// </summary>
    /// <param name="newHead">The new head to use.</param>
    void SetHead(Attachable *newHead);

    /// <summary>
    /// Gets the jetpack of this AHuman.
    /// </summary>
    /// <returns>A pointer to the jetpack of this AHuman. Ownership is NOT transferred.</returns>
    AEmitter * GetJetpack() const { return m_pJetpack; }

    /// <summary>
    /// Sets the jetpack for this AHuman.
    /// </summary>
    /// <param name="newJetpack">The new jetpack to use.</param>
    void SetJetpack(AEmitter *newJetpack);

    /// <summary>
    /// Gets the foreground Arm of this AHuman.
    /// </summary>
    /// <returns>A pointer to the foreground Arm of this AHuman. Ownership is NOT transferred.</returns>
    Arm * GetFGArm() const { return m_pFGArm; }

    /// <summary>
    /// Sets the foreground Arm for this AHuman.
    /// </summary>
    /// <param name="newArm">The new Arm to use.</param>
    void SetFGArm(Arm *newArm);

    /// <summary>
    /// Gets the background arm of this AHuman.
    /// </summary>
    /// <returns>A pointer to the background arm of this AHuman. Ownership is NOT transferred.</returns>
    Arm * GetBGArm() const { return m_pBGArm; }

    /// <summary>
    /// Sets the background Arm for this AHuman.
    /// </summary>
    /// <param name="newArm">The new Arm to use.</param>
    void SetBGArm(Arm *newArm);

    /// <summary>
    /// Gets the foreground Leg of this AHuman.
    /// </summary>
    /// <returns>A pointer to the foreground Leg of this AHuman. Ownership is NOT transferred.</returns>
    Leg * GetFGLeg() const { return m_pFGLeg; }

    /// <summary>
    /// Sets the foreground Leg for this AHuman.
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetFGLeg(Leg *newLeg);

    /// <summary>
    /// Gets the background Leg of this AHuman.
    /// </summary>
    /// <returns>A pointer to the background Leg of this AHuman. Ownership is NOT transferred.</returns>
    Leg * GetBGLeg() const { return m_pBGLeg; }

    /// <summary>
    /// Sets the background Leg for this AHuman.
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetBGLeg(Leg *newLeg);

	/// <summary>
	/// Gets the foot Attachable of this AHuman's foreground Leg.
	/// </summary>
	/// <returns>A pointer to the foot Attachable of this AHuman's foreground Leg. Ownership is NOT transferred!</returns>
    Attachable * GetFGFoot() const { return m_pFGLeg ? m_pFGLeg->GetFoot() : nullptr; }

    /// <summary>
    /// Sets the foot Attachable of this AHuman's foreground Leg.
    /// </summary>
    /// <param name="newFoot">The new foot for this AHuman's foreground Leg to use.</param>
    void SetFGFoot(Attachable *newFoot) { if (m_pFGLeg && m_pFGLeg->IsAttached()) { m_pFGLeg->SetFoot(newFoot); } }

	/// <summary>
    /// Gets the foot Attachable of this AHuman's background Leg.
	/// </summary>
    /// <returns>A pointer to the foot Attachable of this AHuman's background Leg. Ownership is NOT transferred!</returns>
	Attachable * GetBGFoot() const { return m_pBGLeg ? m_pBGLeg->GetFoot() : nullptr; }

    /// <summary>
    /// Sets the foot Attachable of this AHuman's background Leg.
    /// </summary>
    /// <param name="newFoot">The new foot for this AHuman's background Leg to use.</param>
    void SetBGFoot(Attachable *newFoot) { if (m_pBGLeg && m_pBGLeg->IsAttached()) { m_pBGLeg->SetFoot(newFoot); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeadBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the sprite representing the head of this.
// Arguments:       None.
// Return value:    A pointer to the bitmap of with the head of this. Ownership is NOT
//                  transferred!

    BITMAP *GetHeadBitmap() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJetTimeTotal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time this' jetpack can fire when filled, in ms.
// Arguments:       None.
// Return value:    The amount of time this' jetpack can fire when it's at max.

    float GetJetTimeTotal() const { return m_JetTimeTotal; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJetTimeLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time this' jetpack can still fire until out, in ms.
// Arguments:       None.
// Return value:    The amount of time this' jetpack can still fire before running out.

    float GetJetTimeLeft() const { return m_JetTimeLeft; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJetTimeTotal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of time this' jetpack can fire when filled, in ms.
// Arguments:       The amount of time this' jetpack can fire when it's at max.
// Return value:    None.

    void SetJetTimeTotal(float newValue) { m_JetTimeTotal = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJetTimeLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of time this' jetpack can still fire until out, in ms.
// Arguments:       The amount of time this' jetpack can still fire before running out.
// Return value:    None.

	void SetJetTimeLeft(float newValue) { m_JetTimeLeft = newValue < m_JetTimeTotal ? newValue : m_JetTimeTotal; }


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

    bool HandlePieCommand(int pieSliceIndex) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this AHuman. This also puts that item
//                  directly in the hands of this if they are empty.
// Arguments:       An pointer to the new item to add. Ownership IS TRANSFERRED!
// Return value:    None.

	void AddInventoryItem(MovableObject *pItemToAdd) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found firearm
//                  in the inventory. If the held device already is a firearm, or no
//                  firearm is in inventory, nothing happens.
// Arguments:       Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a firearm was successfully switched to, or already held.

	bool EquipFirearm(bool doEquip = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipDeviceInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found device
//                  of the specified group in the inventory. If the held device already 
//                  is of that group, or no device is in inventory, nothing happens.
// Arguments:       The group the device must belong to.
//                  Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a firearm was successfully switched to, or already held.

    bool EquipDeviceInGroup(string group, bool doEquip = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipLoadedFirearmInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first loaded HDFirearm
//                  of the specified group in the inventory. If no such weapon is in the 
//                  inventory, nothing happens.
// Arguments:       The group the HDFirearm must belong to. "Any" for all groups.
//                  The group the HDFirearm must *not* belong to. "None" for no group.
//                  Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a firearm was successfully switched to, or already held.

    bool EquipLoadedFirearmInGroup(string group, string exludeGroup, bool doEquip = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipNamedDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found device
//                  of with the specified preset name in the inventory. If the held device already 
//                  is of that preset name, or no device is in inventory, nothing happens.
// Arguments:       The preset name the device must have.
//                  Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a device was successfully switched to, or already held.

    bool EquipNamedDevice(const string name, bool doEquip);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipThrowable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found ThrownDevice
//                  in the inventory. If the held device already is a ThrownDevice, or no
//                  ThrownDevice  is in inventory, nothing happens.
// Arguments:       Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a ThrownDevice was successfully switched to, or already held.

	bool EquipThrowable(bool doEquip = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipDiggingTool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found digging
//                  tool in the inventory. If the held device already is a digging tool,
//                  or no digging tool is in inventory, nothing happens.
// Arguments:       Whether to actually equip any matching item found in the inventory,
//                  or just report that it's there or not.
// Return value:    Whether a digging tool was successfully switched to.

	bool EquipDiggingTool(bool doEquip = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength any digger this actor is carrying
//                  can penetrate.
// Arguments:       None.
// Return value:    One or the maximum material strength this actor's digger can penetrate.

    float EstimateDigStrenght();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipShield
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found shield
//                  in the inventory. If the held device already is a shield, or no
//                  shield is in inventory, nothing happens.
// Arguments:       None.
// Return value:    Whether a shield was successfully switched to, or already held.

	bool EquipShield();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipShieldInBGArm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tries to equip the first shield in inventory to the background arm;
//                  this only works if nothing is held at all, or the FG arm holds a
//                  one-handed device, or we're in inventory mode.
// Arguments:       None.
// Return value:    Whether a shield was successfully equipped in the background arm.

	bool EquipShieldInBGArm();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipDualWieldableInBGArm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tries to equip the first dual-wieldable in inventory to the background arm;
//                  this only works if nothing is held at all, or the FG arm holds a
//                  one-handed device, or we're in inventory mode.
// Arguments:       None.
// Return value:    Whether a shield was successfully equipped in the background arm.

//	bool EquipDualWieldableInBGArm();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  UnequipBGArm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unequips whatever is in the BG arm and puts it into the inventory.
// Arguments:       None.
// Return value:    Whether there was anything to unequip.

	bool UnequipBGArm();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the FG Arm, if anything. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       None.
// Return value:    The currently equipped item, if any.

	MovableObject * GetEquippedItem() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetEquippedBGItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the BG Arm, if anything. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       None.
// Return value:    The currently equipped item, if any.

	MovableObject * GetEquippedBGItem() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is ready for use, and has
//                  ammo etc.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is ready for use.

	bool FirearmIsReady() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ThrowableIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held ThrownDevice's is ready to go.
// Arguments:       None.
// Return value:    Whether a currently held ThrownDevice (if any) is ready for use.

	bool ThrowableIsReady() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is out of ammo.

	bool FirearmIsEmpty() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) has less than half of ammo left.

	bool FirearmNeedsReload() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is a semi auto device.

	bool FirearmIsSemiAuto() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the currently held device's delay between pulling the trigger
//                  and activating.
// Arguments:       None.
// Return value:    Delay in ms or zero if not a HDFirearm.

	int FirearmActivationDelay() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ReloadFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the currently held firearm, if any.
// Arguments:       None.
// Return value:    None.

	void ReloadFirearm() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsWithinRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is within close range of the currently
//                  used device and aiming status, if applicable.
// Arguments:       A Vector with the aboslute coordinates of a point to check.
// Return value:    Whether the point is within close range of this.

	bool IsWithinRange(Vector &point) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  The range, in pixels, beyond the actors sharp aim that the ray will have.
// Return value:    Whether any unseen pixels were revealed by this look.

	bool Look(float FOVSpread, float range) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LookForGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts a material detecting ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  The range, in pixels, that the ray will have.
//                  A Vector which will be filled with the absolute coordinates of any
//                  found gold. It will be unaltered if false is returned.
// Return value:    Whether gold was spotted by this ray cast. If so, foundLocation
//                  has been filled out with the absolute location of the gold.

	bool LookForGold(float FOVSpread, float range, Vector &foundLocation) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LookForMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an MO detecting ray in the direction of where the head is looking
//                  at the time. Factors including head rotation, sharp aim mode, and
//                  other variables determine how this ray is cast.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  A specific material ID to ignore (see through)
//                  Whether to ignore all terrain or not (true means 'x-ray vision').
// Return value:    A pointer to the MO seen while looking.

	MovableObject * LookForMOs(float FOVSpread = 45, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

    BITMAP * GetGraphicalIcon() override { return GetHeadBitmap(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.
// Arguments:       None.
// Return value:    None.

    void ResetAllTimers() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the path to move along to the currently set movetarget.
// Arguments:       None.
// Return value:    None.

	bool UpdateMovePath() override;


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
// Arguments:       None.
// Return value:    None.

	void Update() override;

    /// <summary>
    /// Executes the Lua-defined OnPieMenu event handler for this AHuman.
    /// </summary>
    /// <param name="pieMenuActor">The actor which triggered the pie menu event.</param>
    /// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
	int OnPieMenu(Actor *pieMenuActor) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AHuman's current graphical representation to a
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


    /// <summary>
    /// Gets the LimbPath corresponding to the passed in Layer and MovementState values.
    /// </summary>
    /// <param name="layer">Whether to get foreground or background LimbPath.</param>
    /// <param name="movementState">Which movement state to get the LimbPath for.</param>
    /// <returns>The LimbPath corresponding to the passed in Layer and MovementState values.</returns>
    LimbPath * GetLimbPath(Layer layer, MovementState movementState) { return &m_Paths[layer][movementState]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST
// Return value:    Limb path speed for the specified preset in m/s.

	float GetLimbPathSpeed(int speedPreset) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST. New speed value in m/s.
// Return value:    None.

	void SetLimbPathSpeed(int speedPreset, float speed);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 
// Arguments:       None.
// Return value:    The default set force maximum, in kg * m/s^2.

	float GetLimbPathPushForce() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 
// Arguments:       The default set force maximum, in kg * m/s^2.
// Return value:    None

	void SetLimbPathPushForce(float force);

    /// <summary>
    /// Gets the target rot angle for the given MovementState.
    /// </summary>
    /// <param name="movementState">The MovementState to get the rot angle target for.</param>
    /// <returns>The target rot angle for the given MovementState.</returns>
    float GetRotAngleTarget(MovementState movementState) { return m_RotAngleTargets.at(movementState); }

    /// <summary>
    /// Sets the target rot angle for the given MovementState.
    /// </summary>
    /// <param name="movementState">The MovementState to get the rot angle target for.</param>
    /// <param name="newRotAngleTarget">The new rot angle target to use.</param>
    void SetRotAngleTarget(MovementState movementState, float newRotAngleTarget) { m_RotAngleTargets.at(movementState) = newRotAngleTarget; }

	/// <summary>
	/// Gets the duration it takes this AHuman to fully charge a throw.
	/// </summary>
	/// <returns>The duration it takes to fully charge a throw in MS.</returns>
	long GetThrowPrepTime() const { return m_ThrowPrepTime; }

	/// <summary>
	/// Sets the duration it takes this AHuman to fully charge a throw.
	/// </summary>
	/// <param name="newPrepTime">New duration to fully charge a throw in MS.</param>
	void SetThrowPrepTime(long newPrepTime) { m_ThrowPrepTime = newPrepTime; }

	/// <summary>
	/// Gets this AHuman's stride sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this AHuman's stride sound.</returns>
	SoundContainer * GetStrideSound() const { return m_StrideSound; }

	/// <summary>
	/// Sets this AHuman's stride sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this AHuman's stride sound.</param>
	void SetStrideSound(SoundContainer *newSound) { m_StrideSound = newSound; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChunkGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts an appropriate amount of gold tracked by Actor, and puts it
//                  in a MovableObject which is put into inventory.
// Arguments:       None.
// Return value:    None.

    void ChunkGold();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  DrawThrowingReticule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws an aiming aid in front of this HeldDevice for throwing.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  A normalized scalar that determines how much of the magnitude of the
//                  reticule should be drawn, to indicate force in the throw.
// Return value:    None.

	void DrawThrowingReticule(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), double amount = 1.0) const;


    // Member variables
    static Entity::ClassInfo m_sClass;
    // Articulated head.
    Attachable *m_pHead;
    // Foreground arm.
    Arm *m_pFGArm;
    // Background arm.
    Arm *m_pBGArm;
    // Foreground leg.
    Leg *m_pFGLeg;
    // Background leg.
    Leg *m_pBGLeg;
    // Limb AtomGroups.
    AtomGroup *m_pFGHandGroup;
    AtomGroup *m_pBGHandGroup;
    AtomGroup *m_pFGFootGroup;
    AtomGroup *m_BackupFGFootGroup;
    AtomGroup *m_pBGFootGroup;
    AtomGroup *m_BackupBGFootGroup;
    // The sound of the actor taking a step (think robot servo)
    SoundContainer *m_StrideSound;
    // Jetpack booster.
    AEmitter *m_pJetpack;
    // The max total time, in ms, that the jetpack can be used without pause
    float m_JetTimeTotal;
    // How much time left the jetpack can go, in ms
    float m_JetTimeLeft;
    // Blink timer
    Timer m_IconBlinkTimer;
    // Current upper body state.
    UpperBodyState m_ArmsState;
    // Current movement state.
    MovementState m_MoveState;
    // Whether the guy is currently lying down on the ground, rotational spring pulling him that way
    // This is engaged if the player first crouches (still upright spring), and then presses left/right
    // It is disengaged as soon as the crouch button/direction is released
    ProneState m_ProneState;
    // Timer for the going prone procedural animation
    Timer m_ProneTimer;
    // Limb paths for different movement states.
    // [0] is for the foreground limbs, and [1] is for BG.
    LimbPath m_Paths[2][MOVEMENTSTATECOUNT];
    std::array<float, MOVEMENTSTATECOUNT> m_RotAngleTargets; //!< An array of rot angle targets for different movement states.
    // Whether was aiming during the last frame too.
    bool m_Aiming;
    // Whether the BG Arm is helping with locomotion or not.
    bool m_ArmClimbing[2];
    // Controls the start of leg synch.
    bool m_StrideStart;
    // Times the stride to see if it is taking too long and needs restart
    Timer m_StrideTimer;
    // How much gold is carried in an MovableObject in inventory, separate from the actor gold tally.
    int m_GoldInInventoryChunk;
    // For timing throws
    Timer m_ThrowTmr;
    
    long m_ThrowPrepTime; //!< The duration it takes this AHuman to fully charge a throw.

    ////////////////
    // AI States

    enum DeviceHandlingState
    {
        STILL = 0,
        POINTING,
        SCANNING,
        AIMING,
        FIRING,
        THROWING,
        DIGGING
    };

    enum SweepState
    {
        NOSWEEP = 0,
        SWEEPINGUP,
        SWEEPUPPAUSE,
        SWEEPINGDOWN,
        SWEEPDOWNPAUSE
    };

    enum DigState
    {
        NOTDIGGING = 0,
        PREDIG,
        STARTDIG,
        TUNNELING,
        FINISHINGDIG,
        PAUSEDIGGER
    };

    enum JumpState
    {
        NOTJUMPING = 0,
        FORWARDJUMP,
        PREUPJUMP,
        UPJUMP,
        APEXJUMP,
        LANDJUMP
    };

    // What the AI is doing with its held devices
    DeviceHandlingState m_DeviceState;
    // What we are doing with a device sweeping
    SweepState m_SweepState;
    // The current digging state
    DigState m_DigState;
    // The current jumping state
    JumpState m_JumpState;
    // Jumping target, overshoot this and the jump is completed
    Vector m_JumpTarget;
    // Jumping left or right
    bool m_JumpingRight;
    // AI is crawling
    bool m_Crawling;
    // The position of the end of the current tunnel being dug. When it is reached, digging can stop.
    Vector m_DigTunnelEndPos;
    // The center angle (in rads) for the sweeping motion done duing scannign and digging
    float m_SweepCenterAimAngle;
    // The range to each direction of the center that the sweeping motion will be done in
    float m_SweepRange;
    // The absolute coordinates of the last detected gold deposits
    Vector m_DigTarget;
    // Timer for how long to be shooting at a seen enemy target
    Timer m_FireTimer;
    // Timer for how long to be shooting at a seen enemy target
    Timer m_SweepTimer;
    // Timer for how long to be patrolling in a direction
    Timer m_PatrolTimer;
    // Timer for how long to be firing the jetpack in a direction
    Timer m_JumpTimer;

	// April 1 prank
	bool m_GotHat;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AHuman, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
	AHuman(const AHuman &reference) = delete;
	AHuman & operator=(const AHuman &rhs) = delete;

};

} // namespace RTE

#endif // File