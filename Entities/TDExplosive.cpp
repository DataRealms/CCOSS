//////////////////////////////////////////////////////////////////////////////////////////
// File:            TDExplosive.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the TDExplosive class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "TDExplosive.h"
#include "Magazine.h"
#include "Atom.h"
#include "RTEManagers.h"
#include "DDTTools.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(TDExplosive, ThrownDevice, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TDExplosive, effectively
//                  resetting the members of this abstraction level only.

void TDExplosive::Clear()
{
    m_NumberToAdd = 10;
	m_ActivatesWhenReleased = false;
	m_IsAnimatedManually = false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int TDExplosive::Create()
{
    if (ThrownDevice::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TDExplosive to be identical to another, by deep copy.

int TDExplosive::Create(const TDExplosive &reference)
{
    ThrownDevice::Create(reference);

//    m_DetonationSound = reference.m_DetonationSound;
    m_ActivatesWhenReleased = reference.m_ActivatesWhenReleased;
	m_IsAnimatedManually = reference.m_IsAnimatedManually;

    // All Explosives should hit against other objects etc, like grenades flying and hitting actors etc
    // EXCEPT when they are laying on the ground etc
    m_IgnoresAGHitsWhenSlowerThan = 1.0;

	if (IsInGroup("Bombs - Payloads"))
		m_HUDVisible = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int TDExplosive::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ParticleNumberToAdd")
        reader >> m_NumberToAdd;
    else if (propName == "AddParticles")
    {
        // Read in the particle we'll convert to a gib for backward compatibility
        const MovableObject *pParticle = dynamic_cast<const MovableObject *>(g_PresetMan.GetEntityPreset(reader));
        AAssert(pParticle, "Stream suggests allocating an unallocatable type in TDExplosive::Create!");

        Gib gib;
        // Assign the particle read in
        gib.m_pGibParticle = pParticle;
        gib.m_Count = m_NumberToAdd;
        // hardcoded junk for backwards compatibility
        gib.m_MinVelocity = 25;
        gib.m_MaxVelocity = 50;
        // Add as gib!
        m_Gibs.push_back(gib);
    }
    // Also for backwads compatibility
    else if (propName == "DetonationSound")
        reader >> m_GibSound;
    else if (propName == "ActivatesWhenReleased")
        reader >> m_ActivatesWhenReleased;
	else if (propName == "IsAnimatedManually")
		reader >> m_IsAnimatedManually;
    else
        // See if the base class(es) can find a match instead
        return ThrownDevice::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TDExplosive with a Writer for
//                  later recreation with Create(Reader &reader);

int TDExplosive::Save(Writer &writer) const
{
    ThrownDevice::Save(writer);

//    writer.NewProperty("DetonationSound");
//    writer << m_DetonationSound;
//	writer.NewProperty("IsAnimatedManually");
//	writer << m_IsAnimatedManually;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TDExplosive object.

void TDExplosive::Destroy(bool notInherited)
{

    if (!notInherited)
        ThrownDevice::Destroy();
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this TDExplosive, including the mass of Magazine
//                  may have inserted.

float TDExplosive::GetMass() const
{
    return m_pMagazine ? m_Mass + m_pMagazine->GetMass() : m_Mass;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void TDExplosive::RestDetection()
{
    ThrownDevice::RestDetection();

    if (m_FiredOnce)
        m_RestTimer.Reset();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite

void TDExplosive::Detach()
{
    // If this only really activated when released, now is the time to do so!
    if (m_pParent && m_ActivatesWhenReleased)
    {
        if (!m_Activated)
            m_ActivationTmr.Reset();
        m_Activated = true;
        m_ActivationSound.Play(g_SceneMan.TargetDistanceScalar(m_Pos));
    }

    ThrownDevice::Detach();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates this Device's features. Depending on whether it can activate
//                  when being held, it will activate and start countdown to boom.

void TDExplosive::Activate()
{
    // Only activate if it can when being held
    if (!m_ActivatesWhenReleased || !m_pParent)
    {
        if (!m_Activated)
            m_ActivationTmr.Reset();
        m_Activated = true;
        m_ActivationSound.Play(g_SceneMan.TargetDistanceScalar(m_Pos));
    }
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void TDExplosive::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    ThrownDevice::GibThis(impactImpulse, internalBlast, pIgnoreMO);


}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this TDExplosive. Supposed to be done every frame.

void TDExplosive::Update()
{
	ThrownDevice::Update();

	if (m_Activated)
	{
		// If not animated manually, play 'fuse lit' animation
		if (!m_IsAnimatedManually)
		{
			m_SpriteAnimMode = ALWAYSLOOP;
		}
		m_RestTimer.Reset();
		m_ToSettle = false;
	}
	else
	{
		if (!m_IsAnimatedManually)
		{
			m_Frame = 0;
			m_SpriteAnimMode = NOANIM;
		}
	}

	// Blow up if the timer since being activated has reached the trigger delay limit
	if (m_Activated && m_ActivationTmr.GetElapsedSimTimeMS() >= m_TriggerDelay)
	{
		m_Activated = false;
		// Detonate!
		GibThis();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this TDExplosive's current graphical representation to a
//                  BITMAP of choice.

void TDExplosive::Draw(BITMAP *pTargetBitmap,
                       const Vector &targetPos,
                       DrawMode mode,
                       bool onlyPhysical) const
{
    ThrownDevice::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void TDExplosive::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible)
        return;

    // Only draw the pickup HUD if not activated
    if (!m_Activated)
        ThrownDevice::DrawHUD(pTargetBitmap, targetPos, whichScreen);
}

} // namespace RTE