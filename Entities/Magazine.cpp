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
#include "PresetMan.h"
#include "AEmitter.h"

namespace RTE {

ConcreteClassInfo(Magazine, Attachable, 50);


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

    // NOTE: This special override of a parent class member variable avoids needing an extra variable to avoid overwriting INI values.
    m_CollidesWithTerrainWhileAttached = false;
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

int Magazine::ReadProperty(const std::string_view &propName, Reader &reader)
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
