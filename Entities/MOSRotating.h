#ifndef _RTEMOSROTATING_
#define _RTEMOSROTATING_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSRotating.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MOSRotating class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSprite.h"
#include "Gib.h"
#include "PostProcessMan.h"
#include "SoundContainer.h"

namespace RTE
{

class AtomGroup;
struct HitData;
class AEmitter;
class Attachable;

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A sprite movable object that can rotate.
// Parent(s):       MOSprite.
// Class history:   05/30/2002 MOSRotating created.

class MOSRotating:
    public MOSprite
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


friend class AtomGroup;
friend class SLTerrain;
friend struct EntityLuaBindings;


// Concrete allocation and cloning definitions
EntityAllocation(MOSRotating)
SerializableOverrideMethods
ClassInfoGetters


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MOSRotating object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MOSRotating() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MOSRotating
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MOSRotating object before deletion
//                  from system memory.
// Arguments:       None.

	~MOSRotating() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSRotating object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSRotating object ready for use.
// Arguments:       A pointer to ContentFile that represents the bitmap file that will be
//                  used to create the Sprite.
//                  The number of frames in the Sprite's animation.
//                  A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector &position = Vector(0, 0), const Vector &velocity = Vector(0, 0), const unsigned long lifetime = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOSRotating to be identical to another, by deep copy.
// Arguments:       A reference to the MOSRotating to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const MOSRotating &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MOSRotating, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); MOSprite::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;

    /// <summary>
    /// Gets the radius of this MOSRotating, not including any Attachables.
    /// </summary>
    /// <returns></returns>
    float GetIndividualRadius() const { return m_SpriteRadius; }

    /// <summary>
    /// Gets the radius of this MOSRotating, including any Attachables.
    /// </summary>
    /// <returns>The radius of this MOSRotating, including any Attachables.</returns>
    float GetRadius() const override { return std::max(m_SpriteRadius, m_FarthestAttachableDistanceAndRadius); }

    /// <summary>
    /// Gets the diameter of this MOSRotating, not including any Attachables.
    /// </summary>
    /// <returns></returns>
    float GetIndividualDiameter() const { return m_SpriteDiameter; }

    /// <summary>
    /// Gets the diameter of this MOSRotating, including any Attachables.
    /// </summary>
    /// <returns>The diameter of this MOSRotating, including any Attachables.</returns>
    float GetDiameter() const override { return GetRadius() * 2.0F; }

    /// <summary>
    /// Checks if the given Attachable should affect radius, and handles it if it should.
    /// </summary>
    /// <param name="attachable">The Attachable to check.</param>
    /// <returns>Whether the radius affecting Attachable changed as a result of this call.</returns>
    virtual bool HandlePotentialRadiusAffectingAttachable(const Attachable *attachable);

    /// <summary>
    /// Gets the mass value of this MOSRotating, not including any Attachables or wounds.
    /// </summary>
    /// <returns>The mass of this MOSRotating.</returns>
    float GetIndividualMass() const { return MovableObject::GetMass(); }

    /// <summary>
    /// Gets the mass value of this MOSRotating, including the mass of all its Attachables and wounds, and their Attachables and so on.
    /// </summary>
    /// <returns>The mass of this MOSRotating and all of its Attachables and wounds in Kilograms (kg).</returns>
    float GetMass() const override { return MovableObject::GetMass() + m_AttachableAndWoundMass; }

    /// <summary>
    /// Updates the total mass of Attachables and wounds for this MOSRotating, intended to be used when Attachables' masses get modified. Simply subtracts the old mass and adds the new one.
    /// </summary>
    /// <param name="oldAttachableOrWoundMass">The mass the Attachable or wound had before its mass was modified.</param>
    /// <param name="newAttachableOrWoundMass">The up-to-date mass of the Attachable or wound after its mass was modified.</param>
    virtual void UpdateAttachableAndWoundMass(float oldAttachableOrWoundMass, float newAttachableOrWoundMass) { m_AttachableAndWoundMass += newAttachableOrWoundMass - oldAttachableOrWoundMass; }

    /// <summary>
    /// Gets the MOIDs of this MOSRotating and all its Attachables and Wounds, putting them into the MOIDs vector.
    /// </summary>
    /// <param name="MOIDs">The vector that will store all the MOIDs of this MOSRotating.</param>
    void GetMOIDs(std::vector<MOID> &MOIDs) const override;

    /// <summary>
    /// Sets the MOID of this MOSRotating and any Attachables on it to be g_NoMOID (255) for this frame.
    /// </summary>
    void SetAsNoID() override;

    /// <summary>
    /// Sets this MOSRotating to not hit a specific other MO and all its children even though MO hitting is enabled on this MOSRotating.
    /// </summary>
    /// <param name="moToNotHit">A pointer to the MO to not be hitting. Null pointer means don't ignore anything. Ownership is NOT transferred!</param>
    /// <param name="forHowLong">How long, in seconds, to ignore the specified MO. A negative number means forever.</param>
    void SetWhichMOToNotHit(MovableObject *moToNotHit = nullptr, float forHowLong = -1) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtomGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current AtomGroup of this MOSRotating.
// Arguments:       None.
// Return value:    A const reference to the current AtomGroup.

    AtomGroup * GetAtomGroup() { return m_pAtomGroup; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main Material  of this MOSRotating.
// Arguments:       None.
// Return value:    The the Material of this MOSRotating.

	Material const * GetMaterial() const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.
// Arguments:       None.
// Return value:    The the priority  of this MovableObject. Higher number, the higher
//                  priority.

	int GetDrawPriority() const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRecoilForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current recoil impulse force Vector of this MOSprite.
// Arguments:       None.
// Return value:    A const reference to the current recoil impulse force in kg * m/s.

    const Vector & GetRecoilForce() const { return m_RecoilForce; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRecoilOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current recoil offset Vector of this MOSprite.
// Arguments:       None.
// Return value:    A const reference to the current recoil offset.

    const Vector & GetRecoilOffset() const { return m_RecoilOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGibList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets direct access to the list of object this is to generate upon gibbing.
// Arguments:       None.
// Return value:    A pointer to the list of gibs. Ownership is NOT transferred!

    std::list<Gib> * GetGibList() { return &m_Gibs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddRecoil
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds graphical recoil offset to this MOSprite according to its angle.
// Arguments:       None.
// Return value:    None.

    void AddRecoil();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRecoil
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds recoil offset to this MOSprite.
// Arguments:       A vector with the recoil impulse force in kg * m/s.
//                  A vector with the recoil offset in pixels.
//                  Whether recoil should be activated or not for the next Draw().
// Return value:    None.

    void SetRecoil(const Vector &force, const Vector &offset, bool recoil = true)
    {
        m_RecoilForce = force;
        m_RecoilOffset = offset;
        m_Recoiled = recoil;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsRecoiled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this MOSprite is currently under the effects of
//                  recoil.
// Arguments:       None.
// Return value:    None.

    bool IsRecoiled() { return m_Recoiled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableDeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this MOSRotating should check for deep penetrations
//                  the terrain or not.
// Arguments:       Whether to enable deep penetration checking or not.
// Return value:    None.

    void EnableDeepCheck(const bool enable = true) { m_DeepCheck = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceDeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets to force a deep checking of this' silhouette against the terrain
//                  and create an outline hole in the terrain, generating particles of the
//                  intersecting pixels in the terrain.
// Arguments:       Whether to force a deep penetration check for this sim frame or not..
// Return value:    None.

    void ForceDeepCheck(const bool enable = true) { m_ForceDeepCheck = enable; }


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


    /// <summary>
    /// Defines what should happen when this MovableObject hits and then bounces off of something.
    /// This is called by the owned Atom/AtomGroup of this MovableObject during travel.
    /// </summary>
    /// <param name="hd">The HitData describing the collision in detail.</param>
    /// <return>Whether the MovableObject should immediately halt any travel going on after this bounce.</return>
	bool OnBounce(HitData &hd) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this sinkage.

	bool OnSink(HitData &hd) override;


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

    virtual bool ParticlePenetration(HitData &hd);


    /// <summary>
    /// Destroys this MOSRotating and creates its specified Gibs in its place with appropriate velocities. Any Attachables are removed and also given appropriate velocities.
    /// </summary>
    /// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
    /// <param name="movableObjectToIgnore">A pointer to an MO which the Gibs and Attachables should not be colliding with.</param>
    virtual void GibThis(const Vector &impactImpulse = Vector(), MovableObject *movableObjectToIgnore = nullptr);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MoveOutOfTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this MovableObject are on top of
//                  terrain pixels, and if so, attempt to move this out so none of this'
//                  Atoms are on top of the terrain any more.
// Arguments:       Only consider materials stronger than this in the terrain for
//                  intersections.
// Return value:    Whether any intersection was successfully resolved. Will return true
//                  even if there wasn't any intersections to begin with.

	bool MoveOutOfTerrain(unsigned char strongerThan = g_MaterialAir) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the global and accumulated forces. Then it clears
//                  out the force list.Note that this does NOT apply the accumulated
//                  impulses (impulse forces)!
// Arguments:       None.
// Return value:    None.

	void ApplyForces() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the accumulated impulse forces. Then it clears
//                  out the impulse list.Note that this does NOT apply the accumulated
//                  regular forces (non-impulse forces)!
// Arguments:       None.
// Return value:    None.

	void ApplyImpulses() override;

    /// <summary>
    /// Adds the passed in Attachable the list of Attachables and sets its parent to this MOSRotating.
    /// </summary>
    /// <param name="attachable">The Attachable to add.</param>
	virtual void AddAttachable(Attachable *attachable);

    /// <summary>
    /// Adds the passed in Attachable the list of Attachables, changes its parent offset to the passed in Vector, and sets its parent to this MOSRotating.
    /// </summary>
    /// <param name="attachable">The Attachable to add.</param>
    /// <param name="parentOffsetToSet">The Vector to set as the Attachable's parent offset.</param>
	virtual void AddAttachable(Attachable *attachable, const Vector &parentOffsetToSet);

    /// <summary>
    /// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
    /// </summary>
    /// <param name="attachableUniqueID">The UniqueID of the Attachable to remove.</param>
    /// <returns>A pointer to the removed Attachable. Ownership IS transferred!</returns>
    virtual Attachable * RemoveAttachable(long attachableUniqueID) { return RemoveAttachable(attachableUniqueID, false, false); }

    /// <summary>
    /// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
    /// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
    /// </summary>
    /// <param name="attachableUniqueID">The UniqueID of the Attachable to remove.</param>
    /// <param name="addToMovableMan">Whether or not to add the Attachable to MovableMan once it has been removed.</param>
    /// <param name="addBreakWounds">Whether or not to add break wounds to the removed Attachable and this MOSRotating.</param>
    /// <returns>A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!</returns>
    virtual Attachable * RemoveAttachable(long attachableUniqueID, bool addToMovableMan, bool addBreakWounds);

    /// <summary>
    /// Removes the passed in Attachable and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
    /// </summary>
    /// <param name="attachable">The Attachable to remove.</param>
    /// <returns>A pointer to the removed Attachable. Ownership IS transferred!</returns>
    virtual Attachable * RemoveAttachable(Attachable *attachable) { return RemoveAttachable(attachable, false, false); }

    /// <summary>
    /// Removes the passed in Attachable and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
    /// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
    /// </summary>
    /// <param name="attachable">The Attachable to remove.</param>
    /// <param name="addToMovableMan">Whether or not to add the Attachable to MovableMan once it has been removed.</param>
    /// <param name="addBreakWounds">Whether or not to add break wounds to the removed Attachable and this MOSRotating.</param>
    /// <returns>A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!</returns>
    virtual Attachable * RemoveAttachable(Attachable *attachable, bool addToMovableMan, bool addBreakWounds);

    /// <summary>
    /// Removes the passed in Attachable, and sets it to delete so it will go straight to MovableMan and be handled there.
    /// </summary>
    /// <param name="attachable">The Attacahble to remove and delete.</param>
    void RemoveAndDeleteAttachable(Attachable *attachable);

    /// <summary>
    /// Either removes or deletes all of this MOSRotating's Attachables.
    /// </summary>
    /// <param name="destroy">Whether to remove or delete the Attachables. Setting this to true deletes them, setting it to false removes them.</param>
	void RemoveOrDestroyAllAttachables(bool destroy);


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
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    void RestDetection() override;


    /// <summary>
    /// Indicates whether this MOSRotating's current graphical representation, including its Attachables, overlaps a point in absolute scene coordinates.
    /// </summary>
    /// <param name="scenePoint">The point in absolute scene coordinates to check for overlap with.</param>
    /// <returns>Whether or not this MOSRotating's graphical representation overlaps the given scene point.</returns>
	bool IsOnScenePoint(Vector &scenePoint) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EraseFromTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Cuts this' silhouette out from the terrain's material and color layers.
// Arguments:       None.
// Return value:    None.

	void EraseFromTerrain();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DeepCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if any of this' deep group atmos are on top of the terrain, and
//                  if so, erases this' silhouette from the terrain.
// Arguments:       Whether to make any MOPixels from erased terrain pixels at all.
//                  The size of the gaps between MOPixels knocked loose by the terrain erasure.
//                  The max number of MOPixel:s to generate as dislodged particles from the
//                  erased terrain.
// Return value:    Whether deep penetration was detected and erasure was done.

	bool DeepCheck(bool makeMOPs = true, int skipMOP = 2, int maxMOP = 100);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

	void PreTravel() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MOSRotatin, using its physical representation.
// Arguments:       None.
// Return value:    None.

	void Travel() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Update(). Always call after
//                  calling Update.
// Arguments:       None.
// Return value:    None.

	void PostTravel() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawMOIDIfOverlapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the MOID representation of this to the SceneMan's MOID layer if
//                  this is found to potentially overlap another MovableObject.
// Arguments:       The MovableObject to check this for overlap against.
// Return value:    Whether it was drawn or not.

	bool DrawMOIDIfOverlapping(MovableObject *pOverlapMO) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSRotating's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


    /// <summary>
    /// Gets the gib impulse limit for this MOSRotating, i.e. the amount of impulse force required in a frame to gib this MOSRotating.
    /// </summary>
    /// <returns>The gib impulse limit of this MOSRotating.</returns>
	float GetGibImpulseLimit() const { return m_GibImpulseLimit; }

    /// <summary>
    /// Sets the gib impulse limit for this MOSRotating, i.e. the amount of impulse force required in a frame to gib this MOSRotating.
    /// </summary>
    /// <param name="newGibImpulseLimit">The new gib impulse limit to use.</param>
    void SetGibImpulseLimit(float newGibImpulseLimit) { m_GibImpulseLimit = newGibImpulseLimit; }

    /// <summary>
    /// Gets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating. Does not include any Attachables.
    /// </summary>
    /// <returns></returns>
    int GetGibWoundLimit() const { return GetGibWoundLimit(false, false, false); }

    /// <summary>
    /// Gets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating.
    /// Optionally adds the gib wound limits of Attachables (and their Attachables, etc.) that match the conditions set by the provided parameters.
    /// </summary>
    /// <param name="includePositiveDamageAttachables">Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNegativeDamageAttachables">Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNoDamageAttachables">Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.</param>
    /// <returns>The wound limit of this MOSRotating and, optionally, its Attachables.</returns>
    int GetGibWoundLimit(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const;

    /// <summary>
    /// Sets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating.
    /// This will not directly trigger gibbing, even if the limit is lower than the current number of wounds.
    /// </summary>
    /// <param name="newLimit">The new gib wound limit to use.</param>
    void SetGibWoundLimit(int newGibWoundLimit) { m_GibWoundLimit = newGibWoundLimit; }

    /// <summary>
    /// Gets the gib blast strength this MOSRotating, i.e. the strength with which Gibs and Attachables will be launched when this MOSRotating is gibbed.
    /// </summary>
    /// <returns>The gib blast strength of this MOSRotating.</returns>
	float GetGibBlastStrength() const { return m_GibBlastStrength; }

    /// <summary>
    /// Sets the gib blast strength this MOSRotating, i.e. the strength with which Gibs and Attachables will be launched when this MOSRotating is gibbed.
    /// </summary>
    /// <param name="newGibBlastStrength">The new gib blast strength to use.</param>
    void SetGibBlastStrength(float newGibBlastStrength) { m_GibBlastStrength = newGibBlastStrength; }

    /// <summary>
    /// Gets the number of wounds attached to this MOSRotating.
    /// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
    /// <returns>The number of wounds on this MOSRotating.</returns>
    /// </summary>
    int GetWoundCount() const { return GetWoundCount(true, false, false); }

    /// <summary>
    /// Gets the number of wounds attached to this MOSRotating.
    /// Optionally adds the wound counts of Attachables (and their Attachables, etc.) that match the conditions set by the provided parameters.
    /// <param name="includePositiveDamageAttachables">Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNegativeDamageAttachables">Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNoDamageAttachables">Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.</param>
    /// <returns>The number of wounds on this MOSRotating and, optionally, its Attachables.</returns>
    /// </summary>
    int GetWoundCount(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const;

	/// <summary>
	/// Adds the passed in wound AEmitter to the list of wounds and changes its parent offset to the passed in Vector.
	/// </summary>
	/// <param name="woundToAdd">The wound AEmitter to add.</param>
	/// <param name="parentOffsetToSet">The vector to set as the wound AEmitter's parent offset.</param>
	/// <param name="checkGibWoundLimit">Whether to gib this MOSRotating if adding this wound raises its wound count past its gib wound limit. Defaults to true.</param>
    virtual void AddWound(AEmitter *woundToAdd, const Vector &parentOffsetToSet, bool checkGibWoundLimit = true);

    /// <summary>
    /// Removes the specified number of wounds from this MOSRotating, and returns damage caused by these removed wounds.
    /// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
    /// </summary>
    /// <param name="numberOfWoundsToRemove">The number of wounds that should be removed.</param>
    /// <returns>The amount of damage caused by these wounds, taking damage multipliers into account.</returns>
    virtual float RemoveWounds(int numberOfWoundsToRemove) { return RemoveWounds(numberOfWoundsToRemove, true, false, false); }

    /// <summary>
    /// Removes the specified number of wounds from this MOSRotating, and returns damage caused by these removed wounds.
    /// Optionally removes wounds from Attachables (and their Attachables, etc.) that match the conditions set by the provided inclusion parameters.
    /// </summary>
    /// <param name="numberOfWoundsToRemove">The number of wounds that should be removed.</param>
    /// <param name="includePositiveDamageAttachables">Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNegativeDamageAttachables">Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.</param>
    /// <param name="includeNoDamageAttachables">Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.</param>
    /// <returns>The amount of damage caused by these wounds, taking damage multipliers into account.</returns>
    virtual float RemoveWounds(int numberOfWoundsToRemove, bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the string value associated with the specified key or "" if it does not exist.
// Arguments:       Key to retrieve value.
// Return value:    String value.

	std::string GetStringValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number value associated with the specified key or 0 if it does not exist.
// Arguments:       Key to retrieve value.
// Return value:    Number (double) value.

	double GetNumberValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the object value associated with the specified key or 0 if it does not exist.
// Arguments:       None.
// Return value:    Object (Entity *) value.

	Entity * GetObjectValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	void SetStringValue(std::string key, std::string value);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	void SetNumberValue(std::string key, double value);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the string value associated with the specified key.
// Arguments:       String key and value to set.
// Return value:    None.

	void SetObjectValue(std::string key, Entity * value);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveStringValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the string value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	void RemoveStringValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveNumberValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the number value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	void RemoveNumberValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RemoveObjectValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove the object value associated with the specified key.
// Arguments:       String key to remove.
// Return value:    None.

	void RemoveObjectValue(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  StringValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	bool StringValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NumberValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	bool NumberValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ObjectValueExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the value associated with the specified key exists.
// Arguments:       String key to check.
// Return value:    True if value exists.

	bool ObjectValueExists(std::string key);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets damage multiplier of this attachable.
// Arguments:       New multiplier value.
// Return value:    None.

    void SetDamageMultiplier(float newValue) { m_DamageMultiplier = newValue; m_NoSetDamageMultiplier = false; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns damage multiplier of this attachable.
// Arguments:       None.
// Return value:    Current multiplier value.

	float GetDamageMultiplier() const { return m_DamageMultiplier; }

    /// <summary>
    /// Gets whether the damage multiplier for this MOSRotating has been directly set, or is at its default value.
    /// </summary>
    /// <returns>Whether the damage multiplier for this MOSRotating has been set.</returns>
    bool HasNoSetDamageMultiplier() const { return m_NoSetDamageMultiplier; }

	/// <summary>
	/// Gets the velocity orientation scalar of this MOSRotating.
	/// </summary>
	/// <returns>New scalar value.</returns>
	float GetOrientToVel() const { return m_OrientToVel; }

	/// <summary>
	/// Sets the velocity orientation scalar of this MOSRotating.
	/// </summary>
	/// <param name="newValue">New scalar value.</param>
	void SetOrientToVel(float newValue) { m_OrientToVel = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTravelImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Retrurns the amount of impulse force exerted on this during the last frame.
// Arguments:       None.
// Return value:    The amount of impulse force exerted on this during the last frame.

	Vector GetTravelImpulse() const { return m_TravelImpulse; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTravelImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of impulse force exerted on this during the last frame.
// Arguments:       New impulse value
// Return value:    None.

	void SetTravelImpulse(Vector impulse) { m_TravelImpulse = impulse; }

	/// <summary>
	/// Gets this MOSRotating's gib sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this MOSRotating's gib sound.</returns>
	SoundContainer * GetGibSound() const { return m_GibSound; }

	/// <summary>
	/// Sets this MOSRotating's gib sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this MOSRotating's gib sound.</param>
	void SetGibSound(SoundContainer *newSound) { m_GibSound = newSound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    /// <summary>
    /// Transfers forces and impulse forces from the given Attachable to this MOSRotating, gibbing and/or removing the Attachable if needed.
    /// </summary>
    /// <param name="attachable">A pointer to the Attachable to apply forces from. Ownership is NOT transferred!</param>
    /// <returns>Whether or not the Attachable has been removed, in which case it'll usually be passed to MovableMan.</returns>
    bool TransferForcesFromAttachable(Attachable *attachable);


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

    void UpdateChildMOIDs(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true) override;

    /// <summary>
    /// Creates the particles specified by this MOSRotating's list of Gibs and adds them to MovableMan with appropriately randomized velocities, based on this MOSRotating's gib blast strength.
    /// </summary>
    /// <param name="impactImpulse">The impulse (kg * m/s) of the impact that caused the gibbing to happen.</param>
    /// <param name="movableObjectToIgnore">A pointer to an MO which the Attachables should not be colliding with.</param>
    void CreateGibsWhenGibbing(const Vector &impactImpulse, MovableObject *movableObjectToIgnore);

    /// <summary>
    /// Removes all Attachables from this MOSR, deleting them or adding them to MovableMan as appropriate, and giving them randomized velocities based on their properties and this MOSRotating's gib blast strength.
    /// </summary>
    /// <param name="impactImpulse">The impulse (kg * m/s) of the impact that caused the gibbing to happen.</param>
    /// <param name="movableObjectToIgnore">A pointer to an MO which the Attachables should not be colliding with.</param>
    void RemoveAttachablesWhenGibbing(const Vector &impactImpulse, MovableObject *movableObjectToIgnore);

    // Member variables
    static Entity::ClassInfo m_sClass;
//    float m_Torque; // In kg * r/s^2 (Newtons).
//    float m_ImpulseTorque; // In kg * r/s.
    // The group of Atom:s that will be the physical reperesentation of this MOSRotating.
    AtomGroup *m_pAtomGroup;
    // The group of Atom:s that will serve as a means to detect deep terrain penetration.
    AtomGroup *m_pDeepGroup;
    // Whether or not to check for deep penetrations.
    bool m_DeepCheck;
    // A trigger for forcing a deep check to happen
    bool m_ForceDeepCheck;
    // Whether deep penetration happaned in the last frame or not, and how hard it was.
    float m_DeepHardness;
    // The amount of impulse force exerted on this during the last frame.
    Vector m_TravelImpulse;
    // The precomupted center location of the sprite relative to the MovableObject::m_Pos.
    Vector m_SpriteCenter;
    // How much to orient the rotation of this to match the velocity vector each frame 0 = none, 1.0 = immediately align with vel vector
    float m_OrientToVel;
    // Whether the SpriteMO is currently pushed back by recoil or not.
    bool m_Recoiled;
    // The impulse force in kg * m/s that represents the recoil.
    Vector m_RecoilForce;
    // The vector that the recoil offsets the sprite when m_Recoiled is true.
    Vector m_RecoilOffset;
    // The list of wound AEmitters currently attached to this MOSRotating, and owned here as well.
    std::list<AEmitter *> m_Wounds;
    // The list of Attachables currently attached and Owned by this.
    std::list<Attachable *> m_Attachables;
    std::unordered_set<unsigned long> m_ReferenceHardcodedAttachableUniqueIDs; //!< An unordered set is filled with the Unique IDs of all of the reference object's hardcoded Attachables when using the copy Create.
    std::unordered_map<unsigned long, std::function<void (MOSRotating*, Attachable*)>> m_HardcodedAttachableUniqueIDsAndSetters; //!< An unordered map of Unique IDs to setter lambda functions, used to call the appropriate hardcoded Attachable setter when a hardcoded Attachable is removed.
    std::unordered_map<unsigned long, std::function<void(MOSRotating *, Attachable *)>> m_HardcodedAttachableUniqueIDsAndRemovers; //!< An unordered map of Unique IDs to remove lambda functions, used to call the appropriate hardcoded Attachable remover when a hardcoded Attachable is removed and calling the setter with nullptr won't work.
    const Attachable *m_RadiusAffectingAttachable; //!< A pointer to the Attachable that is currently affecting the radius. Used for some efficiency benefits.
    float m_FarthestAttachableDistanceAndRadius; //!< The distance + radius of the radius affecting Attachable.
    float m_AttachableAndWoundMass; //!< The mass of all Attachables and wounds on this MOSRotating. Used in combination with its actual mass and any other affecting factors to get its total mass.
    // The list of Gib:s this will create when gibbed
    std::list<Gib> m_Gibs;
    // The amount of impulse force required to gib this, in kg * (m/s). 0 means no limit
    float m_GibImpulseLimit;
    // The number of wound emitters allowed before this gets gibbed. 0 means this can't get gibbed
    int m_GibWoundLimit;
    float m_GibBlastStrength; //!< The strength with which Gibs and Attachables will get launched when this MOSRotating is gibbed.
    // Gib sound effect
    SoundContainer *m_GibSound;
    // Whether to flash effect on gib
    bool m_EffectOnGib;
    // How far this is audiable (in screens) when gibbing
    float m_LoudnessOnGib;
	// Map to store any generic strings
	std::map<std::string, std::string> m_StringValueMap;
	// Map to store any generic numbers
	std::map<std::string, double> m_NumberValueMap;
	// Map to store any object pointers
	std::map<std::string, Entity *> m_ObjectValueMap;

	float m_DamageMultiplier; //!< Damage multiplier for this MOSRotating.
    bool m_NoSetDamageMultiplier; //!< Whether or not the damage multiplier for this MOSRotating was set.

    // Intermediary drawing bitmap used to flip rotating bitmaps. Owned!
    BITMAP *m_pFlipBitmap;
    BITMAP *m_pFlipBitmapS;
    // Intermediary drawing bitmap used to draw sihouettes and other effects. Not owned; points to the shared static bitmaps
    BITMAP *m_pTempBitmap;
    // Temp drawing bitmaps shared between all MOSRotatings
    static BITMAP *m_spTempBitmap16;
    static BITMAP *m_spTempBitmap32;
    static BITMAP *m_spTempBitmap64;
    static BITMAP *m_spTempBitmap128;
    static BITMAP *m_spTempBitmap256;
    static BITMAP *m_spTempBitmap512;

    // Intermediary drawing bitmap used to draw MO silhouettes. Not owned; points to the shared static bitmaps
    BITMAP *m_pTempBitmapS;
    // Temp drawing bitmaps shared between all MOSRotatings
    static BITMAP *m_spTempBitmapS16;
    static BITMAP *m_spTempBitmapS32;
    static BITMAP *m_spTempBitmapS64;
    static BITMAP *m_spTempBitmapS128;
    static BITMAP *m_spTempBitmapS256;
    static BITMAP *m_spTempBitmapS512;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSRotating, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    /// <summary>
    /// Handles reading for custom values, dealing with the various types of custom values.
    /// </summary>
    /// <param name="reader">A Reader lined up to the custom value type to be read.</param>
    void ReadCustomValueProperty(Reader &reader);


    // Disallow the use of some implicit methods.
	MOSRotating(const MOSRotating &reference) = delete;
	MOSRotating& operator=(const MOSRotating &rhs) = delete;

};

} // namespace RTE

#endif // File