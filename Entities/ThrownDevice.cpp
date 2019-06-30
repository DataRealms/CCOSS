//////////////////////////////////////////////////////////////////////////////////////////
// File:            ThrownDevice.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ThrownDevice class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ThrownDevice.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "ACraft.h"
#include "RTEManagers.h"
#include "DDTTools.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(ThrownDevice, Attachable, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ThrownDevice, effectively
//                  resetting the members of this abstraction level only.

void ThrownDevice::Clear()
{
    m_ActivationSound.Reset();
    m_StartThrowOffset.Reset();
    m_EndThrowOffset.Reset();
    m_MinThrowVel = 5;
    m_MaxThrowVel = 15;
    m_TriggerDelay = 0;
    m_ThrownTmr.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int ThrownDevice::Create()
{
    if (HeldDevice::Create() < 0)
        return -1;

    // Set MO Type.
    m_MOType = MovableObject::TypeThrownDevice;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ThrownDevice to be identical to another, by deep copy.

int ThrownDevice::Create(const ThrownDevice &reference)
{
    HeldDevice::Create(reference);

    // Set MO Type.
    m_MOType = MovableObject::TypeThrownDevice;

    m_ActivationSound = reference.m_ActivationSound;

    m_StartThrowOffset = reference.m_StartThrowOffset;
    m_EndThrowOffset = reference.m_EndThrowOffset;
    m_MinThrowVel = reference.m_MinThrowVel;
    m_MaxThrowVel = reference.m_MaxThrowVel;
    m_TriggerDelay = reference.m_TriggerDelay;
    m_ThrownTmr = reference.m_ThrownTmr;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ThrownDevice::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ActivationSound")
        reader >> m_ActivationSound;
    else if (propName == "StartThrowOffset")
        reader >> m_StartThrowOffset;
    else if (propName == "EndThrowOffset")
        reader >> m_EndThrowOffset;
    else if (propName == "MinThrowVel")
        reader >> m_MinThrowVel;
    else if (propName == "MaxThrowVel")
        reader >> m_MaxThrowVel;
    else if (propName == "TriggerDelay")
        reader >> m_TriggerDelay;
    else
        // See if the base class(es) can find a match instead
        return HeldDevice::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ThrownDevice with a Writer for
//                  later recreation with Create(Reader &reader);

int ThrownDevice::Save(Writer &writer) const
{
    HeldDevice::Save(writer);

    writer.NewProperty("ActivationSound");
    writer << m_ActivationSound;
    writer.NewProperty("StartThrowOffset");
    writer << m_StartThrowOffset;
    writer.NewProperty("EndThrowOffset");
    writer << m_EndThrowOffset;
    writer.NewProperty("MinThrowVel");
    writer << m_MinThrowVel;
    writer.NewProperty("MaxThrowVel");
    writer << m_MaxThrowVel;
    writer.NewProperty("TriggerDelay");
    writer << m_TriggerDelay;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ThrownDevice object.

void ThrownDevice::Destroy(bool notInherited)
{

    if (!notInherited)
        HeldDevice::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetStanceOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this ThrownDevice's joint relative
//                  from the parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current stance parent offset.

Vector ThrownDevice::GetStanceOffset() const
{
    return m_StanceOffset.GetXFlipped(m_HFlipped);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void ThrownDevice::RestDetection()
{
    HeldDevice::RestDetection();

    if (m_Activated)
        m_RestTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates one of this HDFirearm's features. Analogous to 'pulling
//                  the trigger'.

void ThrownDevice::Activate()
{
    if (!m_Activated)
        m_ActivationTmr.Reset();
    m_Activated = true;
    m_ActivationSound.Play(g_SceneMan.TargetDistanceScalar(m_Pos));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this HDFirearm's features. Analogous to 'releasing
//                  the trigger'.

void ThrownDevice::Deactivate()
{
    m_Activated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.

bool ThrownDevice::OnMOHit(MovableObject *pOtherMO)
{
/* The ACraft now actively suck things in with cast rays instead
    // See if we hit any craft with open doors to get sucked into
    ACraft *pCraft = dynamic_cast<ACraft *>(pOtherMO);

    // Don't let activated things get sucked into craft (like live grenades!)
    if (!m_Activated && !IsSetToDelete() && pCraft && (pCraft->GetHatchState() == ACraft::OPEN || pCraft->GetHatchState() == ACraft::OPENING))
    {
        // Detach from whomever holds this
        Detach();
        // Add (copy) to the ship's inventory
        pCraft->AddInventoryItem(dynamic_cast<MovableObject *>(this->Clone()));
        // Delete the original from scene - this is safer than 'removing' or handing over ownership halfway through MovableMan's update
        this->SetToDelete();
        // Terminate; we got sucked into the craft; so communicate this out
        return true;
    }
*/
    // Don't terminate, continue travel
    return false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this, using its physical representation.
// Arguments:       None.
// Return value:    None.

void ThrownDevice::Travel()
{
    HeldDevice::Travel();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ThrownDevice. Supposed to be done every frame.

void ThrownDevice::Update()
{
    HeldDevice::Update();

    if (!m_pParent) {

    }
    else {
        /////////////////////////////////
        // Update and apply rotations and scale

        // Taken care of by holder/owner Arm.
//        m_Pos += m_ParentOffset;
// Don't apply state changes to BITMAP anywhere else than Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
    }
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ThrownDevice's current graphical representation to a
//                  BITMAP of choice.

void ThrownDevice::Draw(BITMAP *pTargetBitmap,
                        const Vector &targetPos,
                        DrawMode mode,
                        bool onlyPhysical) const
{
    HeldDevice::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}
*/
} // namespace RTE