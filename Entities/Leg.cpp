//////////////////////////////////////////////////////////////////////////////////////////
// File:            Leg.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Leg class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Leg.h"
#include "RTEManagers.h"
#include "DDTTools.h"
#include "HeldDevice.h"
#include "ContentFile.h"

using namespace std;

namespace RTE
{

CONCRETECLASSINFO(Leg, Attachable, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Leg, effectively
//                  resetting the members of this abstraction level only.

void Leg::Clear()
{
//    m_AnkleOffset.Reset();
    m_pFoot = 0;
    m_ContractedOffset.Reset();
    m_ExtendedOffset.Reset();
    m_MinExtension = 0;
    m_MaxExtension = 0;
    m_CurrentNormalizedExtension = 0;
    m_AnkleOffset.Reset();
    m_TargetOffset.Reset();
    m_IdleOffset.Reset();
    m_MoveSpeed = 0;
    m_WillIdle = false;
    m_DidReach = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int Leg::Create()
{
    if (Attachable::Create() < 0)
        return -1;

    // Make sure the contracted offset is the one closer to the joint
    if (m_ContractedOffset.GetMagnitude() > m_ExtendedOffset.GetMagnitude())
    {
        Vector tempOffset = m_ContractedOffset;
        m_ContractedOffset = m_ExtendedOffset;
        m_ExtendedOffset = tempOffset;
    }

    m_MinExtension = m_ContractedOffset.GetMagnitude();
    m_MaxExtension = m_ExtendedOffset.GetMagnitude();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Leg to be identical to another, by deep copy.

int Leg::Create(const Leg &reference)
{
    Attachable::Create(reference);

    if (reference.m_pFoot) {
        m_pFoot = dynamic_cast<Attachable *>(reference.m_pFoot->Clone());
        AddAttachable(m_pFoot, true);
    }
    m_ContractedOffset = reference.m_ContractedOffset;
    m_ExtendedOffset = reference.m_ExtendedOffset;
    m_MinExtension = reference.m_MinExtension;
    m_MaxExtension = reference.m_MaxExtension;
    m_CurrentNormalizedExtension = reference.m_CurrentNormalizedExtension;
    m_AnkleOffset = reference.m_AnkleOffset;
    m_TargetOffset = reference.m_TargetOffset;
    m_IdleOffset = reference.m_IdleOffset;
    m_WillIdle = reference.m_WillIdle;
    m_MoveSpeed = reference.m_MoveSpeed;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Leg::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Foot")
    {
        const Entity *pObj = g_PresetMan.GetEntityPreset(reader);
        if (pObj)
        {
            m_pFoot = dynamic_cast<Attachable *>(pObj->Clone());
        }
    }
    else if (propName == "ContractedOffset")
    {
        reader >> m_ContractedOffset;
        m_MinExtension = m_ContractedOffset.GetMagnitude();
    }
    else if (propName == "ExtendedOffset")
    {
        reader >> m_ExtendedOffset;
        m_MaxExtension = m_ExtendedOffset.GetMagnitude();
    }
    else if (propName == "MaxLength")
    {
        // For backward compatibiltiy with before
        float maxLength;
        reader >> maxLength;

        m_MinExtension = maxLength / 2;
        m_ContractedOffset.SetXY(m_MinExtension, 0);
        m_MaxExtension = maxLength;
        m_ExtendedOffset.SetXY(m_MaxExtension, 0);
    }
    else if (propName == "IdleOffset")
        reader >> m_IdleOffset;
    else if (propName == "WillIdle")
        reader >> m_WillIdle;
    else if (propName == "MoveSpeed")
        reader >> m_MoveSpeed;
    else
        // See if the base class(es) can find a match instead
        return Attachable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Leg with a Writer for
//                  later recreation with Create(Reader &reader);

int Leg::Save(Writer &writer) const
{
    Attachable::Save(writer);

    writer.NewProperty("Foot");
    writer << m_pFoot;
    writer.NewProperty("ContractedOffset");
    writer << m_ContractedOffset;
    writer.NewProperty("ExtendedOffset");
    writer << m_ExtendedOffset;
    writer.NewProperty("IdleOffset");
    writer << m_IdleOffset;
    writer.NewProperty("WillIdle");
    writer << m_WillIdle;
    writer.NewProperty("MoveSpeed");
    writer << m_MoveSpeed;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Leg object.

void Leg::Destroy(bool notInherited)
{
    delete m_pFoot;
    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.

void Leg::SetID(const MOID newID)
{
    MovableObject::SetID(newID);
    if (m_pFoot)
        m_pFoot->SetID(newID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReachToward
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates the leg so that it reaches after a point in scene coordinates.

void Leg::ReachToward(const Vector &scenePoint)
{
    m_TargetOffset = scenePoint;
/*
    if (m_HFlipped) {
        m_Pos.m_X -= m_ParentOffset.m_X;
        m_Pos.m_Y += m_ParentOffset.m_Y;
    }
    else
        m_Pos += m_ParentOffset;

    Vector reachVec(m_TargetOffset - m_Pos);
    return reachVec.GetMagnitude() <= m_MaxExtension &&
           reachVec.GetMagnitude() >= (m_MaxExtension / 2);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BendLeg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Bends the leg to the appropriate position depending on the ankle
//                  offset.

void Leg::BendLeg()
{
    if (m_FrameCount == 1)
    {
        m_Frame = 0;
        return;
    }

    // Set correct frame for leg bend.
    float range = m_MaxExtension - m_MinExtension;
    m_CurrentNormalizedExtension = (m_AnkleOffset.GetMagnitude() - (m_MaxExtension - range)) / range;

    if (m_CurrentNormalizedExtension < 0)
        m_CurrentNormalizedExtension = 0;
    else if (m_CurrentNormalizedExtension > 1.0)
        m_CurrentNormalizedExtension = 1.0;

    m_Frame = floorf(m_CurrentNormalizedExtension * m_FrameCount);

    // Clamp
    if (m_Frame >= m_FrameCount)
        m_Frame = m_FrameCount - 1;

    DAssert(m_Frame >= 0 && m_Frame < m_FrameCount, "Frame is out of bounds!");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void Leg::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    // Detach foot and let loose
    if (m_pFoot)
    {
        RemoveAttachable(m_pFoot);
        g_MovableMan.AddParticle(m_pFoot);
        m_pFoot = 0;
    }

    Attachable::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Leg. Supposed to be done every frame.

void Leg::Update()
{
    // Update basic metrics from parent.
    Attachable::Update();

    if (!m_pParent)
    {
        if (m_pFoot)
        {
            m_AnkleOffset.SetXY(m_MaxExtension * 0.60, 0);
            m_AnkleOffset.RadRotate((m_HFlipped ? PI : 0) + m_Rotation.GetRadAngle());
            BendLeg();
            m_pFoot->SetJointPos(m_JointPos.GetFloored() + m_AnkleOffset);
            m_pFoot->SetRotAngle(m_Rotation.GetRadAngle() + PI / 2);
            m_pFoot->Update();
        }
    }
    else
    {
        // Attached, so act like it

        Vector target = g_SceneMan.ShortestDistance(m_JointPos, m_TargetOffset);// m_Pos + RotateOffset(m_JointOffset));
        // Check if target is within leg's length.
        if (target.GetMagnitude() <= m_MaxExtension && target.m_Y >= -3/* && target.GetFloored() != m_AnkleOffset.GetFloored()*/)
        {
            Vector moveVec(target - m_AnkleOffset);
            m_AnkleOffset += moveVec * m_MoveSpeed;
            m_DidReach = true;
        }
        else /*if (m_IdleOffset.GetXFlipped(m_HFlipped).GetFloored() != m_AnkleOffset.GetFloored())*/
        {
            if (target.m_Y < -3 && m_WillIdle)
            {
                Vector moveVec(m_IdleOffset.GetXFlipped(m_HFlipped) - m_AnkleOffset);
                m_AnkleOffset += moveVec * m_MoveSpeed;
//                m_AnkleOffset = m_IdleOffset.GetXFlipped(m_HFlipped);
            }
            else
                m_AnkleOffset = target;
            m_DidReach = false;
        }

        // Cap foot distance to what the Leg allows
        ConstrainFoot();

        // Set the basic rotation
        m_Rotation = (m_HFlipped ? PI : 0) + m_AnkleOffset.GetAbsRadAngle();

        // Apply the extra rotation needed to line up the sprite with the leg extension line

        // Get normalized scalar for how much of the difference in angle between the contracted and extrended offsets should be applied
        // EaseOut is used to get the sine effect needed
        float extraRotationRatio = (EaseOut(m_MinExtension, m_MaxExtension, m_CurrentNormalizedExtension) - m_MinExtension) / (m_MaxExtension - m_MinExtension);
        // The contracted offset's inverse angle is the base for the rotation correction
        float extraRotation = -(m_ContractedOffset.GetAbsRadAngle());
        // Get the actual amount of extra rotation correction needed from the ratio, somewhere on the arc between contracted and extended angles
        // This is negative because it's a correction, the bitmap needs to rotate back to align the ankle with where it's supposed to be in the sprite
        extraRotation -= (m_ExtendedOffset.GetAbsRadAngle() - m_ContractedOffset.GetAbsRadAngle()) * extraRotationRatio;
        // Apply the extra rotation
        m_Rotation.SetRadAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -extraRotation : extraRotation));

// Don't apply state changes to BITMAP anywhere else than Draw().
//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);

        BendLeg();

        if (m_pFoot)
        {
            m_pFoot->SetHFlipped(m_HFlipped);
//            Vector rotatedOff(m_pFoot->GetParentOffset());
//            if (m_HFlipped)
//                rotatedOff.m_X = -rotatedOff.m_X;
//            rotatedOff.RadRotate(m_Rotation);
//            m_pFoot->SetPos(m_Pos + rotatedOff.GetFloored());
//            m_pFoot->SetJointPos(m_Pos.GetFloored() + RotateOffset(m_JointOffset) + m_AnkleOffset);
            m_pFoot->SetJointPos(m_JointPos + m_AnkleOffset);
            if (!m_HFlipped && target.m_X < -10 ||
                m_HFlipped && target.m_X > 10) {
                m_pFoot->SetFrame(3);
            }
            else if (!m_HFlipped && target.m_X < -6 ||
                     m_HFlipped && target.m_X > 6) {
                m_pFoot->SetFrame(2);
                m_pFoot->SetRotAngle(0);
            }
            else if (!m_HFlipped && target.m_X > 6 ||
                     m_HFlipped && target.m_X < -6) {
                m_pFoot->SetFrame(1);
                m_pFoot->SetRotAngle(0);
//                m_pFoot->SetRotAngle(m_Rotation + (m_HFlipped ? -HalfPI : HalfPI));
            }
            else {
                m_pFoot->SetFrame(0);
                m_pFoot->SetRotAngle(0.0);
            }
            m_pFoot->Update();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void Leg::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                           MOID rootMOID,
                           bool makeNewMOID)
{
//    if (m_pFoot)
//        m_pFoot->UpdateMOID(MOIDIndex, false);

    Attachable::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void Leg::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	Attachable::GetMOIDs(MOIDs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Leg's current graphical representation to a
//                  BITMAP of choice.

void Leg::Draw(BITMAP *pTargetBitmap,
               const Vector &targetPos,
               DrawMode mode,
               bool onlyPhysical) const
{
    if (m_pFoot && mode != g_DrawMOID && !m_pFoot->IsDrawnAfterParent())
        m_pFoot->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pFoot && mode != g_DrawMOID && m_pFoot->IsDrawnAfterParent())
        m_pFoot->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConstrainFoot
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the foot distance is constrained between the mix and max
//                  extension of this Leg.

bool Leg::ConstrainFoot()
{
    if (m_AnkleOffset.GetMagnitude() > m_MaxExtension)
    {
        m_AnkleOffset.SetMagnitude(m_MaxExtension);
        return false;
    }
    else if (m_AnkleOffset.GetMagnitude() < m_MinExtension) {
        m_AnkleOffset.SetMagnitude(m_MinExtension + 0.1);
        return true;
    }
    else
        return true;
}

} // namespace RTE