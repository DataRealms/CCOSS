//////////////////////////////////////////////////////////////////////////////////////////
// File:            Magazine.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Magazine class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Magazine.h"
#include "Atom.h"
#include "MOSParticle.h"
#include "RTEManagers.h"
#include "RTETools.h"
#include "MOPixel.h"
#include "AEmitter.h"
#include "float.h"

namespace RTE {

ConcreteClassInfo(Magazine, Attachable, 0)
ConcreteClassInfo(Round, Entity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Round, effectively
//                  resetting the members of this abstraction level only.

void Round::Clear()
{
    m_ParticleCount = 0;
    m_pParticle = 0;
    m_pShell = 0;
    m_FireVel = 0;
    m_ShellVel = 0;
    m_Separation = 0;
    m_AILifeTime = 0;
    m_AIFireVel = -1;
    m_AIPenetration = -1;
    m_FireSound.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int Round::Create()
{
    if (Entity::Create() < 0)
        return -1;
    
    // Check if LifeTime is overriden
    if (m_AILifeTime == 0)
    {
        // Read the life time from the real projectile
        const MovableObject * pBullet = GetNextParticle();
        if (pBullet)
            m_AILifeTime = pBullet->GetLifetime();
    }
    
    // Check if FireVel is overriden
    if (m_AIFireVel < 0)
        m_AIFireVel = m_FireVel;    // Read the muzzle velocity from this round
    
    // Check if Penetration is overriden
    if (m_AIPenetration < 0)
    {
        // Calculate the Penetration from the real projectiles data, if MOPixel
        const MovableObject * pBullet = GetNextParticle();
        if(pBullet && dynamic_cast<const MOPixel *>(pBullet))
            m_AIPenetration = pBullet->GetMass() * pBullet->GetSharpness() * m_AIFireVel;
        else
            m_AIPenetration = 0;
    }
    
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Round to be identical to another, by deep copy.

int Round::Create(const Round &reference)
{
    Entity::Create(reference);

    m_ParticleCount = reference.m_ParticleCount;
    // Copy particle preset instance
    m_pParticle = reference.m_pParticle;

    // Copy shell preset instance
    m_pShell = reference.m_pShell;
//    if (reference.m_pShell) Don't own anymore
//        m_pShell = dynamic_cast<MovableObject *>(reference.m_pShell->Clone());
    m_FireVel = reference.m_FireVel;
    m_ShellVel = reference.m_ShellVel;
    m_Separation = reference.m_Separation;
    m_FireSound = reference.m_FireSound;
    m_AILifeTime = reference.m_AILifeTime;
    m_AIFireVel = reference.m_AIFireVel;
    m_AIPenetration = reference.m_AIPenetration;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Round::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ParticleCount")
        reader >> m_ParticleCount;
    else if (propName == "Particle")
    {
        m_pParticle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
        RTEAssert(m_pParticle, "Stream suggests allocating an unallocatable type in Round::Create!");
    }
    else if (propName == "Shell")
        m_pShell = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
    else if (propName == "FireVelocity")
        reader >> m_FireVel;
    else if (propName == "ShellVelocity")
        reader >> m_ShellVel;
    else if (propName == "Separation")
        reader >> m_Separation;
    else if (propName == "FireSound")
        reader >> m_FireSound;
    else if (propName == "AILifeTime")
        reader >> m_AILifeTime;
    else if (propName == "AIFireVel")
        reader >> m_AIFireVel;
    else if (propName == "AIPenetration")
        reader >> m_AIPenetration;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Round with a Writer for
//                  later recreation with Create(Reader &reader);

int Round::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("ParticleCount");
    writer << m_ParticleCount;
    writer.NewProperty("Particle");
    writer << m_pParticle;
    writer.NewProperty("Shell");
    writer << m_pShell;
    writer.NewProperty("FireVelocity");
    writer << m_FireVel;
    writer.NewProperty("ShellVelocity");
    writer << m_ShellVel;
    writer.NewProperty("Separation");
    writer << m_Separation;
    writer.NewProperty("FireSound");
    writer << m_FireSound;
    writer.NewProperty("AILifeTime");
    writer << m_AILifeTime;
    writer.NewProperty("AIFireVel");
    writer << m_AIFireVel;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Round object.

void Round::Destroy(bool notInherited)
{
//    delete m_pParticle; not owned anymore
//    delete m_pShell; ditto.

    if (!notInherited)
        Entity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetNextParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next particle contained in this Round, ownership is NOT
//                  transferred!

const MovableObject * Round::GetNextParticle() const
{
    return m_ParticleCount > 0 ? m_pParticle : 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PopNextParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next particle contained in this Round, and removes it from
//                  the stack. Owenership IS transferred!

MovableObject * Round::PopNextParticle()
{
    MovableObject *temp = m_ParticleCount > 0 ? dynamic_cast<MovableObject *>(m_pParticle->Clone()) : 0;
    m_ParticleCount--;
    return temp;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Magazine, effectively
//                  resetting the members of this abstraction level only.

void Magazine::Clear()
{
    m_RoundCount = 0;
    m_FullCapacity = 0;
    m_RTTRatio = 0;
    m_pRegularRound = 0;
    m_pTracerRound = 0;
    m_Discardable = true;
    m_AIAimVel = 100;
    m_AIAimMaxDistance = -1;
    m_AIAimPenetration = 0;
    m_AIBlastRadius = -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Magazine object ready for use.

int Magazine::Create()
{
    if (Attachable::Create() < 0)
        return -1;
    
    // Read projectile properties for AI aim caluculations
    const Round * pNextRound = GetNextRound();
    if (pNextRound)
    {
        // What muzzle velocity should the AI use when aiming?
        m_AIAimVel = pNextRound->GetAIFireVel() < 0 ? pNextRound->GetFireVel() : pNextRound->GetAIFireVel();

        // How much material can this projectile penetrate?
        m_AIAimPenetration = pNextRound->GetAIPenetration();
        
        if (pNextRound->GetAIFireVel() < 0)
        {
            const MovableObject * pBullet = pNextRound->GetNextParticle();
            if(pBullet)
            {
                // Also get FireVel on emitters from sharpness to assure backwards compability with mods
                const AEmitter * pEmitter = dynamic_cast<const AEmitter *>(pBullet);
                if (pEmitter)
                    m_AIAimVel = max(m_AIAimVel, pEmitter->GetSharpness());
            }
        }
    }
    
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Magazine to be identical to another, by deep copy.

int Magazine::Create(const Magazine &reference)
{
    Attachable::Create(reference);

    m_RoundCount = reference.m_RoundCount;
    m_FullCapacity = reference.m_FullCapacity;
    m_RTTRatio = reference.m_RTTRatio;
    m_pRegularRound = reference.m_pRegularRound;
    m_pTracerRound = reference.m_pTracerRound;
    m_Discardable = reference.m_Discardable;
    m_AIBlastRadius = reference.m_AIBlastRadius;
	m_AIAimPenetration = reference.m_AIAimPenetration;
	m_AIAimVel = reference.m_AIAimVel;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Magazine::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "RoundCount")
    {
        reader >> m_RoundCount;
        m_FullCapacity = m_RoundCount;
    }
    else if (propName == "RTTRatio")
        reader >> m_RTTRatio;
    else if (propName == "RegularRound")
        m_pRegularRound = dynamic_cast<const Round *>(g_PresetMan.GetEntityPreset(reader));
    else if (propName == "TracerRound")
        m_pTracerRound = dynamic_cast<const Round *>(g_PresetMan.GetEntityPreset(reader));
    else if (propName == "Discardable")
        reader >> m_Discardable;
    else if (propName == "AIBlastRadius")
        reader >> m_AIBlastRadius;
    else
        // See if the base class(es) can find a match instead
        return Attachable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Magazine with a Writer for
//                  later recreation with Create(Reader &reader);

int Magazine::Save(Writer &writer) const
{
    Attachable::Save(writer);

    writer.NewProperty("RoundCount");
    writer << m_RoundCount;
    writer.NewProperty("RTTRatio");
    writer << m_RTTRatio;
    writer.NewProperty("RegularRound");
    writer << m_pRegularRound;
    writer.NewProperty("TracerRound");
    writer << m_pTracerRound;
    writer.NewProperty("Discardable");
    writer << m_Discardable;
    writer.NewProperty("AIBlastRadius");
    writer << m_AIBlastRadius;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Magazine object.

void Magazine::Destroy(bool notInherited)
{

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round preset of ammo in this Magazine, without removing
//                  it. Ownership IS NOT transferred!

const Round * Magazine::GetNextRound() const
{
    const Round *tempRound = 0;
    if (m_RoundCount != 0)
    {
        if (m_RTTRatio && m_pTracerRound && m_RoundCount % m_RTTRatio == 0)
            tempRound = m_pTracerRound;
        else
            tempRound = m_pRegularRound;
    }
    return tempRound;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PopNextRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Round of ammo in this Magazine, and removes it from the
//                  stack. Ownership IS transferred!

Round * Magazine::PopNextRound()
{
    Round *tempRound = 0;
    if (m_RoundCount != 0)
    {
        if (m_RTTRatio && m_pTracerRound && m_RoundCount % m_RTTRatio == 0)
            tempRound = dynamic_cast<Round *>(m_pTracerRound->Clone());
        else
            tempRound = dynamic_cast<Round *>(m_pRegularRound->Clone());
        // Negative roundcount means infinite ammo
        if (m_FullCapacity > 0)
            m_RoundCount--;
    }
    return tempRound;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength the rounds in the magazine can destroy. 

float Magazine::EstimateDigStrenght()
{
    float maxPenetration = 1;
    if (m_pTracerRound)
    {
        // Find the next tracer
        const MovableObject * pBullet = m_pTracerRound->GetNextParticle();
        if (pBullet)
        {
            if (m_pTracerRound->GetAIFireVel() > 0)
                maxPenetration = max(maxPenetration, m_pTracerRound->GetAIFireVel() * abs(pBullet->GetMass()) * max(pBullet->GetSharpness(), 0.0f));
            else
                maxPenetration = max(maxPenetration, m_pTracerRound->GetFireVel() * abs(pBullet->GetMass()) * max(pBullet->GetSharpness(), 0.0f));
        }
    }
    
    if (m_pRegularRound)
    {
        // Find the next regular bullet
        const MovableObject * pBullet = m_pRegularRound->GetNextParticle();
        if (pBullet)
        {
            if (m_pRegularRound->GetAIFireVel() > 0)
                maxPenetration = max(maxPenetration, m_pRegularRound->GetAIFireVel() * abs(pBullet->GetMass()) * max(pBullet->GetSharpness(), 0.0f));
            else
                maxPenetration = max(maxPenetration, m_pRegularRound->GetFireVel() * abs(pBullet->GetMass()) * max(pBullet->GetSharpness(), 0.0f));
        }
    }
    
    return maxPenetration;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBulletAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet acceleration scalar the AI use when aiming this weapon.

float Magazine::GetBulletAccScalar()
{
    const Round * pRound = GetNextRound();
    if (pRound)
    {
        const MovableObject * pBullet = pRound->GetNextParticle();
        if (pBullet)
            return pBullet->GetGlobalAccScalar();
    }

    return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Magazine. Supposed to be done every frame.

void Magazine::Update()
{
    Attachable::Update();

    /*if (!m_pParent) {

    }
    else {
        /////////////////////////////////
        // Update rotations and scale

        // Taken care of by holder/owner Arm.
//        m_Pos += m_ParentOffset;
// Only apply in Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
    }*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Magazine's current graphical representation to a
//                  BITMAP of choice.

void Magazine::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}

} // namespace RTE
