#ifndef _RTEHDFIREARM_
#define _RTEHDFIREARM_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            HDFirearm.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the HDFirearm class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "HeldDevice.h"

namespace RTE
{

class Magazine;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           HDFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A firearm device that fires projectile MO's and discharges shell MO's.
// Parent(s):       HeldDevice.
// Class history:   07/1/2002 HDFirearm created.

class HDFirearm:
    public HeldDevice
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(HDFirearm)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     HDFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a HDFirearm object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    HDFirearm() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~HDFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a HDFirearm object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~HDFirearm() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the HDFirearm object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a HDFirearm to be identical to another, by deep copy.
// Arguments:       A reference to the HDFirearm to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const HDFirearm &reference);


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
// Description:     Resets the entire HDFirearm, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); HeldDevice::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this HDFirearm to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the HDFirearm will save itself with.
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
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this HDFirearm, including the mass of Magazine
//                  may have inserted.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRateOfFire
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the rate of fire of this. This applies even if semi-auto. it
//                  limits how quickly a new round can be fired after the last.
// Arguments:       None.
// Return value:    The rate of fire, in rounds per min.

    virtual int GetRateOfFire() const { return m_RateOfFire; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetRateOfFire
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the rate of fire of this. This applies even if semi-auto. it
//                  limits how quickly a new round can be fired after the last.
// Arguments:       The new rate of fire, in rounds per min.
// Return value:    None.

    virtual void SetRateOfFire(int newRate) { m_RateOfFire = newRate; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMagazine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently attached Magazine, if any.
// Arguments:       None.
// Return value:    The Magazine, if any is attached.

    virtual Magazine * GetMagazine() const { return m_pMagazine; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetNextMagazineName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Preset name of the next Magazine that will be loaded into
//                  this gun. This changes all future mags that will be reloaded.
// Arguments:       The preset name of the new Magazine to load into this from now on.
// Return value:    Whether the specified magazine was found and successfully prepared.

    virtual bool SetNextMagazineName(std::string magName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundInMagCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of rounds still in the loaded magazine. Negative value
//                  means infinite ammo.
// Arguments:       None.
// Return value:    An int with the number of rounds in the magazine currently in this
//                  HDFirearm. Negative means infinite ammo.

    int GetRoundInMagCount() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the delay before firing.
// Arguments:       None.
// Return value:    An int with the activation delay in ms.

    int GetActivationDelay() const { return m_ActivationDelay; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the delay before firing.
// Arguments:       An int with the activation delay in ms.
// Return value:    None.

    void SetActivationDelay(int delay) { m_ActivationDelay = delay; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeactivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the delay between release of activation and another can be started.
// Arguments:       None.
// Return value:    An int with the delay in ms.

    int GetDeactivationDelay() const { return m_DeactivationDelay; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDeactivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the delay between release of activation and another can be started.
// Arguments:       An int with the delay in ms.
// Return value:    None.

    void SetDeactivationDelay(int delay) { m_DeactivationDelay = delay; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetReloadTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the reload time in ms.
// Arguments:       None.
// Return value:    An int in ms.

    int GetReloadTime() const { return m_ReloadTime; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetReloadTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the reload time in ms.
// Arguments:       An int in ms.
// Return value:    None.

    void SetReloadTime(int delay) { m_ReloadTime = delay; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetShakeRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the range of normal shaking of entire weapon.
// Arguments:       None.
// Return value:    A float with the range in degrees.

    float GetShakeRange() const { return m_ShakeRange; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetShakeRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the range of normal shaking of entire weapon.
// Arguments:       A float with the range in degrees.
// Return value:    None.

    void SetShakeRange(float range) { m_ShakeRange = range; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSharpShakeRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the range of shaking of entire weapon during sharp aiming.
// Arguments:       None.
// Return value:    A float with the range in degrees.

    float GetSharpShakeRange() const { return m_SharpShakeRange; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSharpShakeRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the range of shaking of entire weapon during sharp aiming.
// Arguments:       A float with the range in degrees.
// Return value:    None.

    void SetSharpShakeRange(float range) { m_SharpShakeRange = range; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNoSupportFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the factor for how much more weapon shakes if it isn't supported
//                  by a second hand.
// Arguments:       None.
// Return value:    A float with the factor.

    float GetNoSupportFactor() const { return m_NoSupportFactor; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNoSupportFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the factor for how much more weapon shakes if it isn't supported
//                  by a second hand.
// Arguments:       A float with the factor.
// Return value:    None.

    void SetNoSupportFactor(float factor) { m_NoSupportFactor = factor; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParticleSpreadRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the range of spread angle of fired particles, in one direction.
// Arguments:       None.
// Return value:    A float with the range in degrees.

    float GetParticleSpreadRange() const { return m_ParticleSpreadRange; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetParticleSpreadRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the range of spread angle of fired particles, in one direction.
// Arguments:       A float with the range in degrees.
// Return value:    None.

    void SetParticleSpreadRange(float range) { m_ParticleSpreadRange = range; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIFireVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity the AI use when aiming this weapon.
// Arguments:       None.
// Return value:    A float with the velocity in m/s.

    float GetAIFireVel();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIBulletLifeTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet life time the AI use when aiming this weapon.
// Arguments:       None.
// Return value:    A float with the life time in ms.

    unsigned long GetAIBulletLifeTime();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBulletAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet acceleration scalar the AI use when aiming this weapon.
// Arguments:       None.
// Return value:    A float with the scalar.

    float GetBulletAccScalar();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIBlastRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the blast radius the AI use when aiming this weapon.
// Arguments:       None.
// Return value:    A float with the blast radius in pixels.

    float GetAIBlastRadius() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how much material the projectiles from this weapon can destory.
// Arguments:       None.
// Return value:    A float with the material strength.

    float GetAIPenetration() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CompareTrajectories
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates how close the projectiles from two weapons will land.
// Arguments:       A HDFirearm pointer to compare with.
// Return value:    A float with the distance in pixels.

    float CompareTrajectories(HDFirearm * pWeapon);


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

    virtual Vector GetMuzzlePos() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMuzzleOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the unrotated relative offset from the position to the muzzle or
//                  other equivalent point of this.
// Arguments:       None.
// Return value:    A unrotated vector describing the relative for the muzzle point of
//                  this from this' position.

    virtual Vector GetMuzzleOffset() const { return m_MuzzleOff; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMuzzleOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the unrotated relative offset from the position to the muzzle or
//                  other equivalent point of this.
// Arguments:       Bew ofsset value.
// Return value:    None.

	virtual void SetMuzzleOffset(Vector newOffset) { m_MuzzleOff = newOffset; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParentOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this HDFirearm relative to the
//                  parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current parent offset.

    const Vector & GetParentOffset() const { return m_ParentOffset; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       An moid specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);
    

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Attach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attaches this Attachable to a host MOSprite
// Arguments:       Pointer to the MOSprite to attach to. Ownership is NOT transferred!
// Return value:    None.

    virtual void Attach(MOSRotating *pParent) { HeldDevice::Attach(pParent); m_Reloading = false; m_ReloadTmr.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite
// Arguments:       None.
// Return value:    None.

    virtual void Detach() { HeldDevice::Detach(); m_Activated = m_Reloading = false; m_ReloadTmr.Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.
// Arguments:       None.
// Return value:    None.

    virtual void ResetAllTimers() { HeldDevice::ResetAllTimers(); m_LastFireTmr.Reset(); m_ReloadTmr.Reset(); }


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
// Method:			StopActivationSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Aborts playing of active sound no matter what. Used to silence spinning 
//                  weapons when weapons swapped
// Arguments:       None.
// Return value:    None.

	void StopActivationSound();

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Throws out the currently used Magazine, if any, and puts in a new one
//                  after the reload delay is up.
// Arguments:       None.
// Return value:    None.

    virtual void Reload();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently being reloaded.
// Arguments:       None.
// Return value:    Whetehr being reloaded.

    virtual bool IsReloading() const override { return m_Reloading; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DoneReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device just finished reloading this frame.
// Arguments:       None.
// Return value:    Whether just done reloading this frame.

    virtual bool DoneReloading() const override { return m_DoneReloading; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NeedsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently in need of being reloaded.
// Arguments:       None.
// Return value:    Whetehr in need of reloading (ie not full).

    virtual bool NeedsReloading() const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFull
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently full and reloading won't have
//                  any effect.
// Arguments:       None.
// Return value:    Whetehr magazine is full or not.

    virtual bool IsFull();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFullAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is fully automatic or not.
// Arguments:       None.
// Return value:    Whether the player can hold down fire and this will fire repeatedly.

    virtual bool IsFullAuto() const { return m_FullAuto; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetFullAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether the device is fully automatic or not.
// Arguments:       New value.
// Return value:    None.

    virtual void SetFullAuto(bool newValue)  { m_FullAuto = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this HDFirearm's current graphical representation to a
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
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws an aiming aid in front of this HeldDevice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength one round in the magazine can destroy.
// Arguments:       None.
// Return value:    The maximum material strength the regular or the tracer round can destroy.

    float EstimateDigStrenght ();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FiredOnce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether at least one round has already been	fired during the current activation.
// Arguments:       None.
// Return value:    Returns true if at least one round has already been fired during the current activation.

	bool FiredOnce() const { return m_FiredOnce; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FiredFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether at least one round has already been fired during the current frame.
// Arguments:       None.
// Return value:    Returns true at least one round has already been fired during the current frame.

	bool FiredFrame() const { return m_FireFrame; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  RoundsFired
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     How many rounds were fired during this frame.
// Arguments:       None.
// Return value:    Returns the number of rounds fired during this frame.

	int RoundsFired() const { return m_RoundsFired; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  IsAnimatedManually
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If true then the m_Frame property is not changed bye the Update function
// Arguments:       None.
// Return value:    Whether this HDFirearm is animated manually.

	bool IsAnimatedManually() const { return m_IsAnimatedManually; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetAnimatedManually
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets Whether this HDFirearm is animated manually.
// Arguments:       Manual animation flag value.
// Return value:    None.

	void SetAnimatedManually(bool newValue)  { m_IsAnimatedManually = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetRecoilTransmission
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the calar of how much of the fire recoil force is transmitted to 
//					who/whatever is holding this weapon
// Arguments:       New transmission value.
// Return value:    None.

	void SetRecoilTransmission(float recoilTransmission) { m_RecoilTransmission = recoilTransmission; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetRecoilTransmission
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the calar of how much of the fire recoil force is transmitted to 
//					who/whatever is holding this weapon
// Arguments:       None.
// Return value:    Transmission value.

	float GetRecoilTransmission() const { return m_RecoilTransmission; }


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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;

    // Member variables.
    static Entity::ClassInfo m_sClass;

    // The mag reference which all current mags are generated from.
    // NOT owned by this, owned by PresetMan
    const Magazine *m_pMagazineReference;
    // Magazine MovableObject. Owned
    Magazine *m_pMagazine;
    // Muzzle Flash Attachable. Owned
    Attachable *m_pFlash;

    // The audio of this FireArm being fired.
    SoundContainer m_FireSound;
    // The audio that is played immediately upon activation, but perhaps before actual first firing, if there's a pre-delay
    SoundContainer m_ActiveSound;
    // The audio that is played immediately upon cease of activation
    SoundContainer m_DeactivationSound;
    // The audio of this FireArm being fired empty.
    SoundContainer m_EmptySound;
    // The audio of this FireArm being reloaded.
    SoundContainer m_ReloadStartSound;
    SoundContainer m_ReloadEndSound;

    // Rate of fire, in rounds per min.
    // If 0, firearm is semi-automatic (ie only one discharge per activation).
    int m_RateOfFire;
    // Delay between activation and full round output is achieved, in ms
    int m_ActivationDelay;
    // Delay between release of activation and another can be started, in ms
    int m_DeactivationDelay;
    // Reloading or not
    bool m_Reloading;
    // Just done reloading this frame
    bool m_DoneReloading;
    // Reload time in millisecs.
    int m_ReloadTime;
    // Whether this HDFirearm is full or semi-auto.
    bool m_FullAuto;
    // Whether particles fired from this HDFirearm will ignore hits with itself,
    // and the root parent of this HDFirearm, regardless if they are set to hit MOs.
    bool m_FireIgnoresThis;
    // Scalar of how much of the fire recoil force is transmitted to who/whatever is holding this weapon
    float m_RecoilTransmission;

    // Timer for timing how long ago the last round was fired.
    Timer m_LastFireTmr;
    // Timer for timing reload times.
    Timer m_ReloadTmr;

    // The point from where the projectiles appear.
    Vector m_MuzzleOff;
    // The point from where the discharged shells appear.
    Vector m_EjectOff;
    // Offset to magazine.
    Vector m_MagOff;
    // Range of normal shaking of entire weapon.
    float m_ShakeRange;
    // Range of shaking of entire weapon during sharp aiming.
    float m_SharpShakeRange;
    // Factor for how much more weapon shakes if it isn't supported by a second hand.
    float m_NoSupportFactor;
    // Range of spread angle of fired particles, in one direction
    float m_ParticleSpreadRange;
    // Range of spread angle of ejected shells, in one direction
    float m_ShellSpreadRange;
    // Range of spread in ang vel of ejected shells, in one direction
    float m_ShellAngVelRange;
    // The muzzle velocity the AI use when aiming this weapon
    float m_AIFireVel;
    // The bullet life time the AI use when aiming this weapon
    unsigned long m_AIBulletLifeTime;
    // The bullet acc scalar the AI use when aiming this weapon
    float m_AIBulletAccScalar;

    
    // Whether at least one round has already been
    // fired during the current activation.
    bool m_FiredOnce;
    // Whether at least one round has already been
    // fired during the current frame.
    bool m_FireFrame;
    // Whether at least one round was fired during the last frame
    bool m_FiredLastFrame;
    // Whether, if this HDFireArm is empty, pin has already clicked once during
    // current acticvation.
    bool m_AlreadyClicked;
	// How many rounds were fired during this frame
	int m_RoundsFired;
	// If true m_Frame is not changed during an update hence the animation 
	// is done by external Lua code
	bool m_IsAnimatedManually;

/* TODO
    // Path the the script file that contains the ballistic solution function of this
    std::string m_BallisticScriptPath;
    // The name of the function that 
    std::string m_BallisticScriptFunction;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HDFirearm, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    HDFirearm(const HDFirearm &reference);
    HDFirearm & operator=(const HDFirearm &rhs);

};

} // namespace RTE

#endif // File