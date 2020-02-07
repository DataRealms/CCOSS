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

namespace RTE {

class MOSParticle;


//////////////////////////////////////////////////////////////////////////////////////////
// Concrete class:  Round
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A round containing a number of projectile particles and a one shell.
// Parent(s):       Entity.
// Class history:   07/06/2002 Round created.
//                  06/14/2012 Round made not nested of Magazine, so it won't cause
//                             problems with LuaMan

class Round:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(Round)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Round
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Round object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Round() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Round
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Round object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Round() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Round to be identical to another, by deep copy.
// Arguments:       A reference to the Round to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Round &reference);


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
// Description:     Resets the entire Round, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Round to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Round will save itself with.
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
// Virtual method:  GetNextParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next particle contained in this Round, ownership is NOT
//                  transferred!
// Arguments:       None.
// Return value:    A pointer to the next particle, or 0 if this Round is empty.

    virtual const MovableObject * GetNextParticle() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PopNextParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next particle contained in this Round, and removes it from
//                  the stack. Owenership IS transferred!
// Arguments:       None.
// Return value:    A pointer to the next particle, or 0 if this Round is empty.

    virtual MovableObject * PopNextParticle();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetShell
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the shell casing preset of this Round. Ownership IS NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the shell casing preset, or 0 if this Round has no shell.

    virtual const MovableObject * GetShell() const { return m_pShell; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetFireVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity at which this round is to be fired.
// Arguments:       None.
// Return value:    A float with the velocity in m/s.

    float GetFireVel() const { return m_FireVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetShellVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity at which this round's shell is to be ejected.
// Arguments:       None.
// Return value:    A float with the shell velocity in m/s.

    float GetShellVel() const { return m_ShellVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSeparation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the separation of particles in this round.
// Arguments:       None.
// Return value:    A float with the separation range in pixels.

    float GetSeparation() const { return m_Separation; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetFireSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the extra firing sound of this Round, which can be played in
//                  addition to the weapon's own firing sound. OINT!
// Arguments:       None.
// Return value:    A sound with the firing sample of this round. OINT!

    Sound * GetFireSound() { return &m_FireSound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HasFireSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this Round has an extra sound sample to play when fired.
// Arguments:       None.
// Return value:    Whether the firing Sound of this has been loaded, or the firing Device
//                  will make the noise alone.

    bool HasFireSound() const { return m_FireSound.HasAnySamples(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ParticleCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how many particles are contained within this Round, not
//                  counting the shell.
// Arguments:       None.
// Return value:    The number of particles.

    int ParticleCount() const { return m_ParticleCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this Round is out of particles or not.
// Arguments:       None.
// Return value:    Whether this Round is out of particles or not.

    bool IsEmpty() const { return m_ParticleCount <= 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAILifeTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the lifetime of the projectile used by the AI when executing the shooting scripts.
// Arguments:       None.
// Return value:    The life time in MS used by the AI

    unsigned long GetAILifeTime() const { return m_AILifeTime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AIFireVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the FireVelocity of the projectile used by the AI when executing the shooting scripts.
// Arguments:       None.
// Return value:    The FireVelocity in m/s used by the AI

    int GetAIFireVel() const { return m_AIFireVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the bullets ability to penetrate material when executing the AI shooting scripts.
// Arguments:       None.
// Return value:    A value equivalent to Mass * Sharpness * Vel

    int GetAIPenetration() const { return m_AIPenetration; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    // How many particles in round (think shotgun shot)
    int m_ParticleCount;
    // Round particle MovableObject preset instance.
    const MovableObject *m_pParticle;
    // Shell MovableObject.
    const MovableObject *m_pShell;
    // The velocity with which this round is fired
    float m_FireVel;
    // The velocity with which this round's shell/casing is launched
    float m_ShellVel;
    // The range of separation between particles in this round, in pixels.
    float m_Separation;
    // The extra firing audio of this Round being fired
    Sound m_FireSound;
    // For overriding LifeTime when executing the AI shooting scripts.
    unsigned long m_AILifeTime;
    // For overriding FireVel when executing the AI shooting scripts.
    int m_AIFireVel;
    // For overriding the bullets ability to penetrate material when executing the AI shooting scripts.
    int m_AIPenetration;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Round, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Round(const Round &reference);
    Round & operator=(const Round &rhs);

};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Magazine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An Attachable ammo magazine that can hold rounds that can be fired
//                  by HDFirearm:s.
// Parent(s):       Attachable.
// Class history:   07/03/2002 Magazine created.

class Magazine:
    public Attachable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(Magazine)


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

    virtual ~Magazine() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Magazine object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Magazine to be identical to another, by deep copy.
// Arguments:       A reference to the Magazine to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Magazine &reference);


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
// Description:     Resets the entire Magazine, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Attachable::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Magazine to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Magazine will save itself with.
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
// Virtual method:  GetNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round preset of ammo in this Magazine, without removing
//                  it. Ownership IS NOT transferred!
// Arguments:       None.
// Return value:    A pointer to the next Round preset of ammo, or 0 if this Magazine is empty.

    virtual const Round * GetNextRound() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PopNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round of ammo in this Magazine, and removes it from the
//                  stack. Ownership IS transferred!
// Arguments:       None.
// Return value:    A pointer to the next Round of ammo, or 0 if this Magazine is empty.

    virtual Round * PopNextRound();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetParentOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current position offset of this Magazine relative to the
//                  parent Actor's position, if attached.
// Arguments:       A const reference to the new parent offset.
// Return value:    None.

    void SetParentOffset(const Vector &newOffset) { m_ParentOffset = newOffset; }
*/

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

    bool IsEmpty() const { return m_FullCapacity > 0 ? m_RoundCount == 0 : !(m_FullCapacity < 0); }


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

    virtual void Update();


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

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      DrawMode mode = g_DrawColor,
                      bool onlyPhysical = false) const;


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
    Magazine(const Magazine &reference);
    Magazine & operator=(const Magazine &rhs);

};

} // namespace RTE

#endif // File