#ifndef _RTEMAGAZINE_
#define _RTEMAGAZINE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Magazine.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Magazine class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Attachable.h"
#include "Round.h"

namespace RTE {


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Magazine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An Attachable ammo magazine that can hold rounds that can be fired
//                  by HDFirearm:s.
// Parent(s):       Attachable.
// Class history:   07/03/2002 Magazine created.

class Magazine : public Attachable {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Magazine);
SerializableOverrideMethods;
ClassInfoGetters;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Magazine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Magazine object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Magazine() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Magazine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Magazine object before deletion
//                  from system memory.
// Arguments:       None.

	~Magazine() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Magazine object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Magazine to be identical to another, by deep copy.
// Arguments:       A reference to the Magazine to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Magazine &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Magazine, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Attachable::Reset(); m_CollidesWithTerrainWhileAttached = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round preset of ammo in this Magazine, without removing
//                  it. Ownership IS NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the next Round preset of ammo, or 0 if this Magazine is empty.

	const Round * GetNextRound() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PopNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round of ammo in this Magazine, and removes it from the
//                  stack. Ownership IS transferred!
// Arguments:       None.
// Return value:    A pointer to the next Round of ammo, or 0 if this Magazine is empty.

	Round * PopNextRound();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how many rounds are left in this Magazine.
// Arguments:       None.
// Return value:    The number of rounds left. Negative value means infinite ammo left!

    int GetRoundCount() const { return m_RoundCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRoundCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how many rounds are left in this Magazine.
// Arguments:       The new number of rounds left. Negative value means infinite ammo!
// Return value:    None.

    void SetRoundCount(int newCount) { m_RoundCount = newCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this Magazine is out of rounds.
// Arguments:       None.
// Return value:    Whether this Magazine is out of rounds or not.

	bool IsEmpty() const { return m_FullCapacity >= 0 && m_RoundCount == 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsFull
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this Magazine has not used up any rounds yet.
// Arguments:       None.
// Return value:    Whether this Magazine has not used any rounds yet.

    bool IsFull() const { return m_FullCapacity > 0 ? (m_RoundCount == m_FullCapacity || m_RoundCount < 0) : m_FullCapacity < 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOverHalfFull
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this Magazine has not used up half of the rounds yet.
// Arguments:       None.
// Return value:    Whether this Magazine has not used half of its rounds yet.

    bool IsOverHalfFull() const { return m_FullCapacity > 0 ? ((m_RoundCount > (m_FullCapacity / 2)) || m_RoundCount < 0) : m_FullCapacity < 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCapacity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns teh number of rounds this can hold when it's full.
// Arguments:       None.
// Return value:    The number of rounds this can hold. Negative value means infinite ammo.

    int GetCapacity() const { return m_FullCapacity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDiscardable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether this Magazine should be released into the scene when discarded
//                  or just deleted.
// Arguments:       None.
// Return value:    Whether this Magazine should be relesed into scene or deleted when released.

    bool IsDiscardable() const { return m_Discardable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength the rounds in the magazine can destroy.
// Arguments:       None.
// Return value:    The material strength.

    float EstimateDigStrenght();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAimVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the AI what muzzle velocity to assume when aiming this weapon.
// Arguments:       None.
// Return value:    Velocity in m/s.

    float GetAIAimVel() const { return m_AIAimVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIAimBlastRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the AI what distance in pixels from the rounds in this mag round
//                  are mostly safe.
// Arguments:       None.
// Return value:    Distance in pixels.

    int GetAIAimBlastRadius() const { return m_AIBlastRadius; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIAimPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the AI how much material this projectile can penetrate.
// Arguments:       None.
// Return value:    The material strenght.

    float GetAIAimPenetration() const { return m_AIAimPenetration; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBulletAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet acceleration scalar the AI use when aiming this weapon.
// Arguments:       None.
// Return value:    A float with the scalar.

    float GetBulletAccScalar();


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
// Description:     Draws this Magazine's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    // How many rounds in mag. Negative value means infinite ammo
    int m_RoundCount;
    // The number of rounds that this mag holds. negative menas infinite ammo capacity
    int m_FullCapacity;
    // The ratio between regular and tracer rounds. 0 means no tracers.
    // e.g. 3 means every third round will be a tracer. ie Round To Tracer (RTT) ratio.
    int m_RTTRatio;
    // Round reference instances.
    const Round *m_pRegularRound;
    const Round *m_pTracerRound;
    // Whether this magazine should be released into the scene when discarded, or just be deleted instead
    bool m_Discardable;
    // The muzzle velocity the AI use when aiming this gun. calculated when the magazine is created
    float m_AIAimVel;
    // The estimated maximum distance in pixels the projectiles can hit from
    float m_AIAimMaxDistance;
    // The half of the theoretical upper limit for the amount of material strength this weapon can destroy with one projectile
    float m_AIAimPenetration;
    // Tells the AI what distance in pixels from this round is mostly safe.
    int m_AIBlastRadius;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Magazine, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	Magazine(const Magazine &reference) = delete;
	Magazine & operator=(const Magazine &rhs) = delete;

};

} // namespace RTE

#endif // File