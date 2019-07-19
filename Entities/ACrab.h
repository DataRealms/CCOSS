#ifndef _RTEACRAB_
#define _RTEACRAB_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACrab.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ACrab class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"
#include "LimbPath.h"
#include <deque>

struct BITMAP;

namespace RTE
{

class Attachable;
class Turret;
class Leg;
class AEmitter;
//class LimbPath;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ACrab
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A crab-like actor with four legs.
// Parent(s):       Actor.
// Class history:   10/24/2007 ACrab created.

class ACrab:
    public Actor
{


enum MovementState
{
    STAND = 0,
    WALK,
    JUMP,
    DISLODGE,
    MOVEMENTSTATECOUNT
};

enum
{
    LEFTSIDE = 0,
    RIGHTSIDE,
    SIDECOUNT
};

enum
{
    FGROUND = 0,
    BGROUND,
    LAYERCOUNT
};


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(ACrab)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ACrab
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ACrab object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ACrab() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ACrab
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ACrab object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ACrab() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.
// Arguments:       A pointer to a BITMAP that this ACrab will own and destroy.
//                  A pointer to a Controller that this ACrab will own and destroy.
//                  A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  A AtomGroup that will make up the collision 'cage' of this mass object.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
//                  An initial Status.
//                  An int with the initial health value of this ACrab.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(BITMAP *pSprite,
                       Controller *pController,
                       const float mass,
                       const Vector &position = Vector(0, 0),
                       const Vector &velocity = Vector(0, 0),
                       AtomGroup *hitBody = new AtomGroup(),
                       const unsigned long lifetime = 0,
                       Status status = ACTIVE,
                       const int health = 100);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACrab to be identical to another, by deep copy.
// Arguments:       A reference to the ACrab to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const ACrab &reference);


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
// Description:     Resets the entire ACrab, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Actor::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ACrab to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ACrab will save itself with.
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
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACrab, including the mass of its
//                  currently attached body parts and inventory.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldCarried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many ounces of gold this Actor is carrying.
// Arguments:       None.
// Return value:    The current amount of carried gold, in Oz.

    virtual float GetGoldCarried() const { return m_GoldCarried + m_GoldInInventoryChunk; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' eye or view point.

    virtual Vector GetEyePos() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLFGLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the left FG Leg as an Attachable. This is for Lua binding mostly.
// Arguments:       None.
// Return value:    A pointer to the Leg Attachable. Ownership is NOT transferred!

    Attachable * GetLFGLeg() const { return (Attachable *)m_pLFGLeg; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLBGLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the left BG Leg as an Attachable. This is for Lua binding mostly.
// Arguments:       None.
// Return value:    A pointer to the Leg Attachable. Ownership is NOT transferred!

    Attachable * GetLBGLeg() const { return (Attachable *)m_pLBGLeg; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRFGLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the right FG Leg as an Attachable. This is for Lua binding mostly.
// Arguments:       None.
// Return value:    A pointer to the Leg Attachable. Ownership is NOT transferred!

    Attachable * GetRFGLeg() const { return (Attachable *)m_pRFGLeg; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLFGLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the right BG Leg as an Attachable. This is for Lua binding mostly.
// Arguments:       None.
// Return value:    A pointer to the Leg Attachable. Ownership is NOT transferred!

    Attachable * GetRBGLeg() const { return (Attachable *)m_pRBGLeg; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJetpack
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the jetpack as an emitter. This is for Lua binding mostly.
// Arguments:       None.
// Return value:    A pointer to jetpack emitter. Ownership is NOT transferred!

    AEmitter * GetJetpack() const { return (AEmitter *)m_pJetpack; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJetTimeTotal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time this' jetpack can fire when filled, in ms.
// Arguments:       None.
// Return value:    The amount of time this' jetpack can fire when it's at max.

    float GetJetTimeTotal() const { return m_JetTimeTotal; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetJetTimeTotal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of time this' jetpack can fire when filled, in ms.
// Arguments:       The amount of time this' jetpack can fire when it's at max.
// Return value:    None.

    void SetJetTimeTotal(float newValue) { m_JetTimeTotal = newValue; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetJetTimeLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time this' jetpack can still fire until out, in ms.
// Arguments:       None.
// Return value:    The amount of time this' jetpack can still fire before running out.

    float GetJetTimeLeft() const { return m_JetTimeLeft; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       A MOID specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);


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

    virtual bool CollideAtPoint(HitData &hitData);

/*
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
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.
// Arguments:       The pie menu to add slices to. Ownership is NOT transferred!
// Return value:    Whether any slices were added.

    virtual bool AddPieMenuSlices(PieMenuGUI *pPieMenu);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HandlePieCommand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles and does whatever a specific activated Pie Menu slice does to
//                  this.
// Arguments:       The pie menu command to handle. See the PieSliceIndex enum.
// Return value:    Whetehr any slice was handled. False if no matching slice handler was
//                  found, or there was no slice currently activated by the pie menu.

    virtual bool HandlePieCommand(int pieSliceIndex);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns any equipped turret.
// Arguments:       None.
// Return value:    A pointer to an attachable.

    virtual Attachable * GetTurret() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the turret, if anything. OINT.
// Arguments:       None.
// Return value:    The currently equipped item, if any.

    virtual MovableObject * GetEquippedItem() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is ready for use, and has
//                  ammo etc.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is ready for use.

    virtual bool FirearmIsReady() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is out of ammo.

    virtual bool FirearmIsEmpty() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) has less than half of ammo left.

    virtual bool FirearmNeedsReload() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is a semi auto device.

    virtual bool FirearmIsSemiAuto() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the currently held device's delay between pulling the trigger
//                  and activating.
// Arguments:       None.
// Return value:    Delay in ms or zero if not a HDFirearm.

    virtual int FirearmActivationDelay() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  ReloadFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the currently held firearm, if any.
// Arguments:       None.
// Return value:    None.

    virtual void ReloadFirearm();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsWithinRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is within close range of the currently
//                  used device and aiming status, if applicable.
// Arguments:       A Vector witht he aboslute coordinates of a point to check.
// Return value:    Whether the point is within close range of this.

    virtual bool IsWithinRange(Vector &point) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  The range, in pixels, beyond the actors sharp aim that the ray will have.
// Return value:    Whether any unseen pixels were revealed by this look.

    virtual bool Look(float FOVSpread, float range);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LookForMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an MO detecting ray in the direction of where the head is looking
//                  at the time. Factors including head rotation, sharp aim mode, and
//                  other variables determine how this ray is cast.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  A specific material ID to ignore (see through)
//                  Whether to ignore all terrain or not (true means 'x-ray vision').
// Return value:    A pointer to the MO seen while looking.

    virtual MovableObject * LookForMOs(float FOVSpread = 45, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false);


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
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

    virtual bool IsOnScenePoint(Vector &scenePoint) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the path to move along to the currently set movetarget.
// Arguments:       None.
// Return value:    None.

    virtual bool UpdateMovePath();


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

    virtual void Update();


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
// Description:     Draws this ACrab's current graphical representation to a
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
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false);


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
// Virtual method:  GetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST
// Return value:    Limb path speed for the specified preset in m/s.

	virtual float GetLimbPathSpeed(int speedPreset) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST. New speed value in m/s.
// Return value:    None.

	virtual void SetLimbPathSpeed(int speedPreset, float speed);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 
// Arguments:       None.
// Return value:    The default set force maximum, in kg * m/s^2.

	virtual float GetLimbPathPushForce() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 
// Arguments:       The default set force maximum, in kg * m/s^2.
// Return value:    None

	virtual void SetLimbPathPushForce(float force);


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

    // Turret which can be mounted with a weapon
    Turret *m_pTurret;
    // Left Foreground leg.
    Leg *m_pLFGLeg;
    // Left Background leg.
    Leg *m_pLBGLeg;
    // Right Foreground leg.
    Leg *m_pRFGLeg;
    // Right Background leg.
    Leg *m_pRBGLeg;
    // Limb AtomGroups.
    AtomGroup *m_pLFGFootGroup;
    AtomGroup *m_pLBGFootGroup;
    AtomGroup *m_pRFGFootGroup;
    AtomGroup *m_pRBGFootGroup;
    // The sound of the actor taking a step (think robot servo)
    Sound m_StrideSound;
    // Jetpack booster.
    AEmitter *m_pJetpack;
    // The max total time, in ms, that the jetpack can be used without pause
    float m_JetTimeTotal;
    // How much time left the jetpack can go, in ms
    float m_JetTimeLeft;
    // Blink timer
    Timer m_IconBlinkTimer;
    // Current movement state.
    MovementState m_MoveState;
    // Limb paths for different movement states.
    // First which side, then which background/foreground, then the movement state
    LimbPath m_Paths[SIDECOUNT][LAYERCOUNT][MOVEMENTSTATECOUNT];
    // Whether was aiming during the last frame too.
    bool m_Aiming;
    // Controls the start of leg synch.
    bool m_StrideStart[SIDECOUNT];
    // Times the strides to make sure they get restarted if they end up too long
    Timer m_StrideTimer[SIDECOUNT];
    // How much gold is carried in an MovableObject in inventory, separate from the actor gold tally.
    int m_GoldInInventoryChunk;
    // The maximum angle MountedMO can be aimed up, positive values only, in radians
    float m_AimRangeUpperLimit;
    // The maximum angle MountedMO can be aimed down, positive values only, in radians
    float m_AimRangeLowerLimit;

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


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACrab, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
    ACrab(const ACrab &reference);
    ACrab & operator=(const ACrab &rhs);

};

} // namespace RTE

#endif // File