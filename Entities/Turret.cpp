//////////////////////////////////////////////////////////////////////////////////////////
// File:            Turret.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Turret class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Turret.h"
#include "HeldDevice.h"
#include "PresetMan.h"

namespace RTE {

ConcreteClassInfo(Turret, Attachable, 20)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Turret, effectively
//                  resetting the members of this abstraction level only.

void Turret::Clear()
{
    m_pMountedMO = 0;
    m_MountedRotOffset = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int Turret::Create()
{
    if (Attachable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Turret to be identical to another, by deep copy.

int Turret::Create(const Turret &reference)
{
    Attachable::Create(reference);

    if (reference.m_pMountedMO) {
        m_pMountedMO = dynamic_cast<MovableObject *>(reference.m_pMountedMO->Clone());
        if (m_pMountedMO->IsDevice())
            dynamic_cast<Attachable *>(m_pMountedMO)->Attach(this,
                dynamic_cast<Attachable *>(m_pMountedMO)->GetParentOffset());
    }

    m_MountedRotOffset = reference.m_MountedRotOffset;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Turret::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "MountedMO")
    {
        const Entity *pEntity = g_PresetMan.GetEntityPreset(reader);
        if (pEntity)
        {
            m_pMountedMO = dynamic_cast<MovableObject *>(pEntity->Clone());
            if (m_pMountedMO->IsDevice())
                dynamic_cast<HeldDevice *>(m_pMountedMO)->Attach(this, dynamic_cast<HeldDevice *>(m_pMountedMO)->GetParentOffset());
        }
        pEntity = 0;
    }
    else
        return Attachable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Turret with a Writer for
//                  later recreation with Create(Reader &reader);

int Turret::Save(Writer &writer) const
{
    Attachable::Save(writer);

    writer.NewProperty("MountedMO");
    writer << m_pMountedMO;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Turret object.

void Turret::Destroy(bool notInherited)
{
//    g_MovableMan.RemoveEntityPreset(this);

    delete m_pMountedMO;

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this Turret, including the mass of any device it
//                  may be holding.

float Turret::GetMass() const
{
    return m_Mass + (m_pMountedMO ? m_pMountedMO->GetMass() : 0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMountedDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the HeldDevice currently held by this Turret, IF the thing held is
//                  a HeldDevice, that is. Ownership is NOT transferred.
// Arguments:       None.
// Return value:    A pointer to the currently held HeldDevice. 0 is returned if no
//                  HeldDevice is currently held (even though an MO may be held).

HeldDevice * Turret::GetMountedDevice() const
{
    HeldDevice *pRetDev = 0;
    if (m_pMountedMO && m_pMountedMO->IsHeldDevice())
        pRetDev = dynamic_cast<HeldDevice *>(m_pMountedMO);
    return pRetDev;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.

void Turret::SetID(const MOID newID)
{
    MovableObject::SetID(newID);
    if (m_pMountedMO)
        m_pMountedMO->SetID(newID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Turret with a new
//                  one. Ownership IS transferred. The currently held MovableObject
//                  (if there is one) will be dropped and become a detached MovableObject,

void Turret::SetMountedMO(MovableObject *newHeldMO)
{
    if (m_pMountedMO && m_pMountedMO->IsHeldDevice() && dynamic_cast<HeldDevice *>(m_pMountedMO)->IsAttachedTo(this)) {
        HeldDevice *pHeldDev = dynamic_cast<HeldDevice *>(m_pMountedMO);
        pHeldDev->Detach();
// TODO: Refine throwing force to dropped device here?")
		pHeldDev->SetVel(Vector(RandomNum(0.0F, 10.0F), -RandomNum(0.0F, 15.0F)));
		pHeldDev->SetAngularVel(-RandomNum(0.0F, 10.0F));
        g_MovableMan.AddItem(pHeldDev);
        m_pMountedMO = pHeldDev = 0;
    }

    if (newHeldMO && newHeldMO->IsHeldDevice()) {
        HeldDevice *pNewDev = dynamic_cast<HeldDevice *>(newHeldMO);
        pNewDev->Detach();
        g_MovableMan.RemoveMO(pNewDev);
        pNewDev->Attach(this, pNewDev->GetParentOffset());
        pNewDev = 0;
    }
    m_pMountedMO = newHeldMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReleaseMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of the HeldDevice currently held. Ownership IS
//                  transferred!
// Arguments:       None.
// Return value:    A pointer to the up to this point held HeldDevice. 0 is returned if no
//                  HeldDevice is currently held. Ownership IS transferred!

MovableObject * Turret::ReleaseMountedMO()
{
    MovableObject *pReturnMO = m_pMountedMO;
    if (m_pMountedMO && m_pMountedMO->IsDevice())
        dynamic_cast<Attachable *>(m_pMountedMO)->Detach();
    m_pMountedMO = 0;
    return pReturnMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DropEverything
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of anyhthing it holds and give it to the
//                  MovableMan. Ownership is transferred to MovableMan.
// Arguments:       None.

MovableObject * Turret::DropEverything()
{
    MovableObject *pReturnMO = m_pMountedMO;

    if (m_pMountedMO && m_pMountedMO->IsDevice()) {
        dynamic_cast<Attachable *>(m_pMountedMO)->Detach();
        g_MovableMan.AddItem(m_pMountedMO);
    }
    else if (m_pMountedMO)
        g_MovableMan.AddParticle(m_pMountedMO);

    m_pMountedMO = 0;

    return pReturnMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapMountedMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Turret with a new
//                  one, and returns the replaced one. Ownership IS transferred both ways.

MovableObject * Turret::SwapMountedMO(MovableObject *newMO)
{
    MovableObject *oldMO = m_pMountedMO;
    if (oldMO && oldMO->IsDevice())
        dynamic_cast<Attachable *>(oldMO)->Detach();

    m_pMountedMO = newMO;
    if (newMO && newMO->IsDevice()) {
        dynamic_cast<Attachable *>(newMO)->Attach(this,
            dynamic_cast<Attachable *>(newMO)->GetParentOffset());
    }

    return oldMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void Turret::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    Attachable::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Turret. Supposed to be done every frame.

void Turret::Update()
{
    // Update basic metrics from parent.
    Attachable::Update();

    if (!m_pParent)
    {
        if (m_pMountedMO)
        {
/*
            m_pMountedMO->SetVel(m_Vel + Vector(-10 * RandomNum(), -15 * RandomNum()));
            m_pMountedMO->SetAngularVel(-7);
            if (m_pMountedMO->IsDevice())
                dynamic_cast<Attachable *>(m_pMountedMO)->Detach();
            g_MovableMan.AddItem(m_pMountedMO);
            m_pMountedMO = 0;
*/
            HeldDevice *pHeldDev = dynamic_cast<HeldDevice *>(m_pMountedMO);
            if (pHeldDev && pHeldDev->IsHeldDevice())
            {
                pHeldDev->SetJointPos(m_Pos + pHeldDev->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
                pHeldDev->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedRotOffset);
                pHeldDev->Update();
            }
        }
    }
    else
    {
        // Attached, so act like it

        // If a device is held, update it and arm configuration accordingly.
        HeldDevice *pHeldDev = dynamic_cast<HeldDevice *>(m_pMountedMO);
        if (m_pMountedMO && m_pMountedMO->IsHeldDevice() && pHeldDev)
        {
            pHeldDev->SetHFlipped(m_HFlipped);

            pHeldDev->SetJointPos(m_Pos + pHeldDev->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
            pHeldDev->SetRotAngle(m_Rotation.GetRadAngle() + m_MountedRotOffset);
            pHeldDev->Update();
// Looks strange
//            if (pHeldDev->IsRecoiled())
//                m_pParent->AddImpulseForce(pHeldDev->GetRecoilForce());
//            else
                m_Recoiled = false;

            // Apply forces and detach if necessary
            // OBSERVE the memeber pointer is what gets set to 0!$@#$@
            if (!ApplyAttachableForces(pHeldDev))
                m_pMountedMO = 0;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void Turret::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                              MOID rootMOID,
                              bool makeNewMOID)
{
    if (m_pMountedMO && m_pMountedMO->GetsHitByMOs())
        m_pMountedMO->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);

    Attachable::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void Turret::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	if (m_pMountedMO && m_pMountedMO->GetsHitByMOs())
		m_pMountedMO->GetMOIDs(MOIDs);

	Attachable::GetMOIDs(MOIDs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Turret's current graphical representation to a
//                  BITMAP of choice.

void Turret::Draw(BITMAP *pTargetBitmap,
               const Vector &targetPos,
               DrawMode mode,
               bool onlyPhysical) const
{
    if (m_pMountedMO && !m_pMountedMO->IsDrawnAfterParent())
        m_pMountedMO->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pMountedMO && m_pMountedMO->IsDrawnAfterParent())
        m_pMountedMO->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}

} // namespace RTE