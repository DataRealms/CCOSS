#ifndef _RTEHELDDEVICE_
#define _RTEHELDDEVICE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            HeldDevice.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the HeldDevice class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Attachable.h"
#include "Actor.h"
#include "PieMenuGUI.h"

namespace RTE
{

enum HeldDeviceType
{
    WEAPON = 0,
    TOOL,
    SHIELD,
    BOMB,
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           HeldDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An articulated device that can be weilded by an Actor.
// Parent(s):       Attachable.
// Class history:   06/2/2002 HeldDevice created.
//                  01/31/2007 Made concrete so Shields can be jsut HeldDevice:s

class HeldDevice:
    public Attachable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(HeldDevice)
SerializableOverrideMethods
ClassInfoGetters


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     HeldDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a HeldDevice object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    HeldDevice() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~HeldDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a HeldDevice object before deletion
//                  from system memory.
// Arguments:       None.

	~HeldDevice() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the HeldDevice object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a HeldDevice to be identical to another, by deep copy.
// Arguments:       A reference to the HeldDevice to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const HeldDevice &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire HeldDevice, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Attachable::Reset(); m_MOType = MovableObject::TypeHeldDevice; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAboveHUDPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of the top of this' HUD stack.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' HUD stack top point.

	Vector GetAboveHUDPos() const override { return m_Pos + Vector(0, -32); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSupportPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the support handhold that this HeldDevice
//                  offers.
// Arguments:       None.
// Return value:    A vector describing the absolute world coordinates for the support
//                  position of this HeldDevice.

    Vector GetSupportPos() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMagazinePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the magazine or other equivalent point of
//                  this.
// Arguments:       None.
// Return value:    A vector describing the absolute world coordinates for the magazine
//                  attachment point of this

    virtual Vector GetMagazinePos() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMuzzlePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the muzzle or other equivalent point of
//                  this.
// Arguments:       None.
// Return value:    A vector describing the absolute world coordinates for the muzzle point
//                  of this

    virtual Vector GetMuzzlePos() const { return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMuzzleOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the unrotated relative offset from the position to the muzzle or
//                  other equivalent point of this.
// Arguments:       None.
// Return value:    A unrotated vector describing the relative for the muzzle point of
//                  this from this' position.

    virtual Vector GetMuzzleOffset() const { return Vector(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMuzzleOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the unrotated relative offset from the position to the muzzle or
//                  other equivalent point of this.
// Arguments:       Bew ofsset value.
// Return value:    None.

	virtual void SetMuzzleOffset(Vector newOffset) { /* Actually does something in inherited classes */ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current stance parent offset.

    virtual Vector GetStanceOffset() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       New value.
// Return value:    None.

	void SetStanceOffset(Vector newValue) { m_StanceOffset = newValue; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSharpStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       New value.
// Return value:    None.

	Vector GetSharpStanceOffset() const { return m_SharpStanceOffset; } 

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSharpStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current position offset of this HeldDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       New value.
// Return value:    None.

	void SetSharpStanceOffset(Vector newValue) { m_SharpStanceOffset = newValue; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSharpLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how much farther an Actor which holds this device can see when
//                  aiming this HeldDevice sharply.
// Arguments:       None.
// Return value:    The length in world pixel units.

    float GetSharpLength() const { return m_MaxSharpLength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSharpLength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much farther an Actor which holds this device can see when
//                  aiming this HeldDevice sharply.
// Arguments:       The length in world pixel units.
// Return value:    None.

    void SetSharpLength(float newLength) { m_MaxSharpLength = newLength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSupported
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this HeldDevice is currently supported by a second hand
//                  or not.
// Arguments:       If it should be supported or not.
// Return value:    None.

    void SetSupported(bool supported) { m_Supported = supported; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSupportOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns support offset.
// Arguments:       None.
// Return value:    Support offset value.

	Vector GetSupportOffset() const { return m_SupportOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSupportOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets support offset.
// Arguments:       New support offset value.
// Return value:    None.

	void SetSupportOffset(Vector newOffset) { m_SupportOffset = newOffset; }

    /// <summary>
    /// Gets whether this HeldDevice has any limitations on what can pick it up.
    /// </summary>
    /// <returns>Whether this HeldDevice has any limitations on what can pick it up.</returns>
    bool HasPickupLimitations() const { return IsUnPickupable() || !m_PickupableByPresetNames.empty(); }

    /// <summary>
    /// Gets whether this HeldDevice cannot be picked up at all.
    /// </summary>
    /// <returns>Whether this HeldDevice cannot be picked up at all.</returns>
    bool IsUnPickupable() const { return m_IsUnPickupable; }

    /// <summary>
    /// Sets whether this HeldDevice cannot be picked up at all.
    /// </summary>
    /// <param name="shouldBeUnPickupable">Whether this HeldDevice cannot be picked up at all. True means it cannot, false means any other limitations will apply normally.</param>
    void SetUnPickupable(bool shouldBeUnPickupable) { m_IsUnPickupable = shouldBeUnPickupable; }

    /// <summary>
    /// Checks whether the given Actor can pick up this HeldDevice.
    /// </summary>
    /// <param name="actor">The Actor to check. Ownership is NOT transferred.</param>
    /// <returns>Whether the given Actor can pick up this HeldDevice.</returns>
    bool IsPickupableBy(const Actor *actor) const { return !HasPickupLimitations() || m_PickupableByPresetNames.find(actor->GetPresetName()) != m_PickupableByPresetNames.end(); }

    /// <summary>
    /// Specify that objects with the given PresetName can pick up this HeldDevice.
    /// </summary>
    /// <param name="presetName">The PresetName of an object that should be able to pick up this HeldDevice.</param>
    void AddPickupableByPresetName(const std::string &presetName) { SetUnPickupable(false); m_PickupableByPresetNames.insert(presetName); }

    /// <summary>
    /// Remove allowance for objects with the given PresetName to pick up this HeldDevice.
    /// Note that if the last allowance is removed, the HeldDevice will no longer have pickup limitations, rather than setting itself as unpickupable.
    /// </summary>
    /// <param name="actorPresetName">The PresetName of an object that should no longer be able to pick up this HeldDevice.</param>
    void RemovePickupableByPresetName(const std::string &actorPresetName);

    /// <summary>
    /// Gets the multiplier for how well this HeldDevice can be gripped by Arms.
    /// </summary>
    /// <returns>The grip strength multiplier for this HeldDevice.</returns>
    float GetGripStrengthMultiplier() const { return m_GripStrengthMultiplier; }

    /// <summary>
    /// Sets the multiplier for how well this HeldDevice can be gripped by Arms.
    /// </summary>
    /// <param name="gripStrengthMultiplier">The new grip strength multiplier for this HeldDevice.</param>
    void SetGripStrengthMultiplier(float gripStrengthMultiplier) { m_GripStrengthMultiplier = gripStrengthMultiplier; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSharpAim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the degree to which this is being aimed sharp. This will
//                  affect the accuracy and what GetParentOffset returns.
// Arguments:       A normalized scalar between 0 (no sharp aim) to 1.0 (best aim).
// Return value:    None.

    void SetSharpAim(float sharpAim) { m_SharpAim = sharpAim; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsWeapon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this is an offensive weapon or not.
// Arguments:       None.
// Return value:    Offensive weapon or not.

    bool IsWeapon() { return m_HeldDeviceType == WEAPON; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsTool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this is a tool or not.
// Arguments:       None.
// Return value:    Tool or not.

    bool IsTool() { return m_HeldDeviceType == TOOL; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsShield
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this is a shield or not.
// Arguments:       None.
// Return value:    Shield or not.

    bool IsShield() { return m_HeldDeviceType == SHIELD; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDualWieldable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this is a dual wieldable weapon or not.
// Arguments:       None.
// Return value:    Dual wieldable or not.

	bool IsDualWieldable() const { return m_DualWieldable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDualWieldable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this is a dual wieldable weapon or not.
// Arguments:       Dual wieldable or not.
// Return value:    None.

	void SetDualWieldable(bool isDualWieldable) { m_DualWieldable = isDualWieldable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOneHanded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this can be held and operated effectively with one
//                  hand or not.
// Arguments:       None.
// Return value:    One handed device or not.

    bool IsOneHanded() const { return m_OneHanded; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOneHanded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this can be held and operated effectively with one
//                  hand or not.
// Arguments:       New value.
// Return value:    None.

    void SetOneHanded(bool newValue) { m_OneHanded = newValue; }
	
	
	
//////////////////////////////////////////////////////////////////////////////////////////
// Method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.
// Arguments:       The pie menu to add slices to. Ownership is NOT transferred!
// Return value:    Whether any slices were added.

   bool AddPieMenuSlices(PieMenuGUI *pPieMenu);


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
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates one of this HDFirearm's features. Analogous to 'pulling
//                  the trigger'.
// Arguments:       None.
// Return value:    None.

    virtual void Activate();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this HDFirearm's features. Analogous to 'releasing
//                  the trigger'.
// Arguments:       None.
// Return value:    None.

    virtual void Deactivate();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Throws out the currently used Magazine, if any, and puts in a new one
//                  after the reload delay is up.
// Arguments:       None.
// Return value:    None.

    virtual void Reload() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsActivated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently being activated.
// Arguments:       None.
// Return value:    Whether being activated.

    virtual bool IsActivated() const { return m_Activated; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently being reloaded.
// Arguments:       None.
// Return value:    Whetehr being reloaded.

    virtual bool IsReloading() const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DoneReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device just finished reloading this frame.
// Arguments:       None.
// Return value:    Whether just done reloading this frame.

    virtual bool DoneReloading() const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NeedsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently in need of being reloaded.
// Arguments:       None.
// Return value:    Whetehr in need of reloading (ie not full).

    virtual bool NeedsReloading() const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFull
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently full and reloading won't have
//                  any effect.
// Arguments:       None.
// Return value:    Whetehr magazine is full or not.

    virtual bool IsFull() const { return false; }


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
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this HeldDevice's current graphical representation to a
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
// Description:     Draws this' current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;

	/// <summary>
	/// Resest all the timers used by this. Can be emitters, etc. This is to prevent backed up emissions to come out all at once while this has been held dormant in an inventory.
	/// </summary>
	void ResetAllTimers() override { Attachable::ResetAllTimers(); m_ActivationTimer.Reset(); }

#pragma region Force Transferral
    /// <summary>
    /// Bundles up all the accumulated impulse forces of this HeldDevice and calculates how they transfer to the joint, and therefore to the parent.
    /// If the accumulated impulse forces exceed the joint strength or gib impulse limit of this HeldDevice, the jointImpulses Vector will be filled up to that limit and false will be returned.
    /// Additionally, in this case, the HeldDevice will remove itself from its parent, destabilizing said parent if it's an Actor, and gib itself if appropriate.
    /// </summary>
    /// <param name="jointImpulses">A vector that will have the impulse forces affecting the joint ADDED to it.</param>
    /// <param name="jointStiffnessValueToUse">An optional override for the HeldDevice's joint stiffness for this function call. Primarily used to allow subclasses to perform special behavior.</param>
    /// <param name="jointStrengthValueToUse">An optional override for the HeldDevice's joint strength for this function call. Primarily used to allow subclasses to perform special behavior.</param>
    /// <param name="gibImpulseLimitValueToUse">An optional override for the HeldDevice's gib impulse limit for this function call. Primarily used to allow subclasses to perform special behavior.</param>
    /// <returns>False if the HeldDevice has no parent or its accumulated forces are greater than its joint strength or gib impulse limit, otherwise true.</returns>
    bool TransferJointImpulses(Vector &jointImpulses, float jointStiffnessValueToUse = -1, float jointStrengthValueToUse = -1, float gibImpulseLimitValueToUse = -1) override;
#pragma endregion


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // Indicates what kind of held device this is, see the HeldDeviceType enum
    int m_HeldDeviceType;
    // Is this HeldDevice that are currently activated?
    bool m_Activated;
    // Timer for timing how long a feature has been activated.
    Timer m_ActivationTimer;
    // Can be weilded well with one hand or not
    bool m_OneHanded;
	// Can be weilded with bg hand or not
	bool m_DualWieldable;
    // Position offset from the parent's own position to this HeldDevice's joint, which
    // defines the normal stance that an arm that is holding this device should have.
    Vector m_StanceOffset;
    // The alternative parent offset stance that is used when the device is carefully aimed.
    Vector m_SharpStanceOffset;
    // The point at which the other arm of the holder can support this HeldDevice.
    // Relative to the m_Pos. This is like a seconday handle position.
    Vector m_SupportOffset;
    // The degree as to this is being aimed carefully. 0 means no sharp aim, and 1.0 means best aim.
    float m_SharpAim;
    // How much farther the player can see when aiming this sharply.
    float m_MaxSharpLength;
    // If this HeldDevice is currently being supported by a second hand.
    bool m_Supported;
    bool m_IsUnPickupable; //!< Whether or not this HeldDevice should be able to be picked up at all.
    std::unordered_set<std::string> m_PickupableByPresetNames; //!< The unordered set of PresetNames that can pick up this HeldDevice if it's dropped. An empty set means there are no PresetName limitations.
    float m_GripStrengthMultiplier; //!< The multiplier for how well this HeldDevice can be gripped by Arms.
    // Blink timer for the icon
    Timer m_BlinkTimer;
    // Extra pie menu options that this should add to any actor who holds this device
    std::list<PieSlice> m_PieSlices;
    // How loud this device is when activated. 0 means perfectly quiet 0.5 means half of normal (normal equals audiable from ~half a screen)
    float m_Loudness;
    // If this weapon belongs to the "Explosive Weapons" group or not
    bool m_IsExplosiveWeapon;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HeldDevice, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	HeldDevice(const HeldDevice &reference) = delete;
	HeldDevice & operator=(const HeldDevice &rhs) = delete;

};

} // namespace RTE

#endif // File