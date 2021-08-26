//////////////////////////////////////////////////////////////////////////////////////////
// File:            Arm.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Arm class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Arm.h"
#include "HDFirearm.h"
#include "ThrownDevice.h"
#include "PresetMan.h"

namespace RTE {

ConcreteClassInfo(Arm, Attachable, 50);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Arm, effectively
//                  resetting the members of this abstraction level only.

void Arm::Clear()
{
    m_pHeldMO = nullptr;
    m_GripStrength = 0;
    m_ThrowStrength = 25.0F;
    m_HandFile.Reset();
    m_pHand = nullptr;
    m_MaxLength = 0;
    m_HandOffset.Reset();
    m_TargetPosition.Reset();
    m_IdleOffset.Reset();
    m_MoveSpeed = 0;
    m_WillIdle = true;
    m_DidReach = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Arm object ready for use.

int Arm::Create() {
    if (Attachable::Create() < 0) {
        return -1;
    }

    // Ensure Arms don't get flagged as inheriting RotAngle, since they never do and always set their RotAngle for themselves.
    m_InheritsRotAngle = false;

    // Ensure Arms don't collide with terrain when attached since their expansion/contraction is frame based so atom group doesn't know how to account for it.
    SetCollidesWithTerrainWhileAttached(false);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Arm to be identical to another, by deep copy.

int Arm::Create(const Arm &reference) {
    if (reference.m_pHeldMO) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pHeldMO->GetUniqueID());
        SetHeldMO(dynamic_cast<Attachable *>(reference.m_pHeldMO->Clone()));
    }
    Attachable::Create(reference);

    m_GripStrength = reference.m_GripStrength;
    m_ThrowStrength = reference.m_ThrowStrength;
    m_HandFile = reference.m_HandFile;
    m_pHand = m_HandFile.GetAsBitmap();
    RTEAssert(m_pHand, "Failed to load hand bitmap in Arm::Create")

    m_MaxLength = reference.m_MaxLength;
    m_HandOffset = reference.m_HandOffset;
    m_TargetPosition = reference.m_TargetPosition;
    m_IdleOffset = reference.m_IdleOffset;
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

int Arm::ReadProperty(const std::string_view &propName, Reader &reader) {
    if (propName == "HeldDevice") {
        SetHeldMO(dynamic_cast<MovableObject *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "GripStrength") {
        reader >> m_GripStrength;
    } else if (propName == "ThrowStrength") {
        reader >> m_ThrowStrength;
    } else if (propName == "Hand") {
        reader >> m_HandFile;
        m_pHand = m_HandFile.GetAsBitmap();
    } else if (propName == "MaxLength") {
        reader >> m_MaxLength;
    } else if (propName == "IdleOffset") {
        reader >> m_IdleOffset;
    } else if (propName == "WillIdle") {
        reader >> m_WillIdle;
    } else if (propName == "MoveSpeed") {
        reader >> m_MoveSpeed;
    } else {
        return Attachable::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Arm with a Writer for
//                  later recreation with Create(Reader &reader);

int Arm::Save(Writer &writer) const
{
    Attachable::Save(writer);

    writer.NewProperty("HeldDevice");
    writer << m_pHeldMO;
    writer.NewProperty("GripStrength");
    writer << m_GripStrength;
    writer.NewProperty("ThrowStrength");
    writer << m_ThrowStrength;
    writer.NewProperty("HandGroup");
    writer << m_HandFile;
    writer.NewProperty("MaxLength");
    writer << m_MaxLength;
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
// Description:     Destroys and resets (through Clear()) the Arm object.

void Arm::Destroy(bool notInherited)
{
//    g_MovableMan.RemoveEntityPreset(this);

// Not owned by this
//    destroy_bitmap(m_pHand);

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeldDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the HeldDevice currently held by this Arm, IF the thing held is
//                  a HeldDevice, that is. Ownership is NOT transferred.
// Arguments:       None.
// Return value:    A pointer to the currently held HeldDevice. 0 is returned if no
//                  HeldDevice is currently held (even though an MO may be held).

HeldDevice * Arm::GetHeldDevice() const
{
    HeldDevice *pRetDev = 0;
    if (m_pHeldMO && m_pHeldMO->IsHeldDevice())
        pRetDev = dynamic_cast<HeldDevice *>(m_pHeldMO);
    return pRetDev;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHeldMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Arm with a new
//                  one. Ownership IS transferred. The currently held MovableObject
//                  (if there is one) will be deleted.

void Arm::SetHeldMO(MovableObject *newHeldMO) {
    Attachable *oldHeldMOAsAttachable = dynamic_cast<Attachable *>(m_pHeldMO);
    if (oldHeldMOAsAttachable && oldHeldMOAsAttachable->IsAttached()) { RemoveAndDeleteAttachable(oldHeldMOAsAttachable); }
    if (newHeldMO == nullptr) {
        m_pHeldMO = nullptr;
    } else {
        m_pHeldMO = newHeldMO;
        if (Attachable *heldMOAsAttachable = dynamic_cast<Attachable *>(newHeldMO)) {
            AddAttachable(heldMOAsAttachable);

            m_HardcodedAttachableUniqueIDsAndSetters.insert({heldMOAsAttachable->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
                dynamic_cast<Arm *>(parent)->SetHeldMO(attachable);
            }});
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReleaseHeldMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of the HeldDevice currently held. Ownership IS
//                  transferred!
// Arguments:       None.
// Return value:    A pointer to the up to this point held HeldDevice. 0 is returned if no
//                  HeldDevice is currently held. Ownership IS transferred!

MovableObject * Arm::ReleaseHeldMO()
{
    MovableObject *pReturnMO = m_pHeldMO;
    if (m_pHeldMO)
    {
        // Clear forces so it doesn't blow up immediately due to accumulated crap when being held
        m_pHeldMO->ClearForces();
        m_pHeldMO->ClearImpulseForces();
		if (m_pHeldMO->IsDevice())
		{
			// Once detached may have incorrect ID value. Detach will take care m_RootID. New ID will be assigned on next frame.
            m_pHeldMO->SetAsNoID();
            RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO));
		}
    }
    m_pHeldMO = nullptr;
    return pReturnMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DropEverything
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this arm let go of anyhthing it holds and give it to the
//                  MovableMan. Ownership is transferred to MovableMan.
// Arguments:       None.

MovableObject * Arm::DropEverything()
{
    MovableObject *pReturnMO = m_pHeldMO;

    if (m_pHeldMO && m_pHeldMO->IsDevice()) {
        RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO), true, false);
    }
    else if (m_pHeldMO)
        g_MovableMan.AddParticle(m_pHeldMO);

    m_pHeldMO = 0;

    return pReturnMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapHeldMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the MovableObject currently held by this Arm with a new
//                  one, and returns the replaced one. Ownership IS transferred both ways.

MovableObject * Arm::SwapHeldMO(MovableObject *newMO)
{
    MovableObject *oldMO = m_pHeldMO;
    if (m_pHeldMO && m_pHeldMO->IsDevice()) { RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO)); }

    m_pHeldMO = newMO;
    if (newMO && newMO->IsDevice()) {
        AddAttachable(dynamic_cast<Attachable *>(newMO));
    }

    return oldMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Reach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates the arm so that it reaches after a point in scene coordinates.
//                  Must be called AFTER SetPos for this frame if the return value is to
//                  be accurate. If the target is not reached, the idle position of the
//                  will be assumed.

void Arm::Reach(const Vector &scenePoint)
{
    m_TargetPosition = scenePoint;
    m_WillIdle = true;
/*
    if (m_HFlipped) {
        m_Pos.m_X -= m_ParentOffset.m_X;
        m_Pos.m_Y += m_ParentOffset.m_Y;
    }
    else
        m_Pos += m_ParentOffset;

    Vector reachVec(m_TargetPosition - m_Pos);
    return reachVec.GetMagnitude() <= m_MaxLength &&
           reachVec.GetMagnitude() >= (m_MaxLength / 2);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReachToward
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates the arm so that it reaches after a point in scene coordinates.
//                  Must be called AFTER SetPos for this frame if the return value is to
//                  be accurate. Arm will reach towards target regardless of wheter it
//                  is within this Arm's length or not.

void Arm::ReachToward(const Vector &scenePoint)
{
    m_TargetPosition = scenePoint;
    m_WillIdle = false;
/*
    if (m_HFlipped) {
        m_Pos.m_X -= m_ParentOffset.m_X;
        m_Pos.m_Y += m_ParentOffset.m_Y;
    }
    else
        m_Pos += m_ParentOffset;

    Vector reachVec(m_TargetPosition - m_Pos);
    return reachVec.GetMagnitude() <= m_MaxLength &&
           reachVec.GetMagnitude() >= (m_MaxLength / 2);
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arm::Update() {
    Attachable::PreUpdate();

    if (!IsAttached()) {
        RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO), true, false);
    } else {
        m_AngularVel = 0.0F;
    }

    UpdateCurrentHandOffset();

    HeldDevice *heldDevice = m_pHeldMO ? dynamic_cast<HeldDevice *>(m_pHeldMO) : nullptr;
    const ThrownDevice *thrownDevice = heldDevice ? dynamic_cast<ThrownDevice *>(heldDevice) : nullptr;

    // HeldDevices need to use the aim angle for their positioning and rotating, while ThrownDevices need to aim and position themselves based on the hand offset, so this done here for TDs and below for HDs.
    if (thrownDevice || !heldDevice) { m_Rotation = m_HandOffset.GetAbsRadAngle() + (m_HFlipped ? c_PI : 0); }

    if (heldDevice) {
        // In order to keep the HeldDevice in the right place, we need to convert its offset (the hand offset) to work as the ParentOffset for the HeldDevice.
        // The HeldDevice will then use this to set its JointPos when it's updated. Unfortunately UnRotateOffset doesn't work for this, since it's Vector/Matrix division, which isn't commutative.
        Vector handOffsetAsParentOffset = RotateOffset(m_JointOffset) + m_HandOffset;
        handOffsetAsParentOffset.RadRotate(-m_Rotation.GetRadAngle()).FlipX(m_HFlipped);
        heldDevice->SetParentOffset(handOffsetAsParentOffset);
    }

    Attachable::Update();

    m_Recoiled = heldDevice && heldDevice->IsRecoiled();

    if (heldDevice && !thrownDevice) {
        m_Rotation = m_HandOffset.GetAbsRadAngle() + (m_HFlipped ? c_PI : 0);
        m_Pos = m_JointPos - RotateOffset(m_JointOffset);
    }

    UpdateArmFrame();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arm::UpdateCurrentHandOffset() {
    if (IsAttached()) {
        Vector targetOffset;
        if (m_pHeldMO && !dynamic_cast<ThrownDevice *>(m_pHeldMO)) {
            m_DidReach = false;
			HeldDevice *heldDevice = dynamic_cast<HeldDevice *>(m_pHeldMO);
			// TODO: calculate total grip strength from both arms? (also: fine-tune this shit, and move it elsewhere)
			float totalGripStrength = (m_GripStrength || heldDevice->GetJointStrength()) * (heldDevice->GetSupported() ? 2.0F : 1.0F);
			targetOffset = heldDevice->GetStanceOffset();
			// Diminish recoil effect when body is horizontal so that the device doesn't get pushed into terrain when prone.
			float rotAngleScalar = std::abs(std::cos(m_Parent->GetRotAngle()));
			float recoilScalar = std::min((heldDevice->GetRecoilForce() / totalGripStrength).GetMagnitude() * 0.4F, 0.8F) * rotAngleScalar;
			targetOffset.SetX(targetOffset.GetX() * (1.0F - recoilScalar));
			// Shift Y offset slightly so the device is more likely to go under the shoulder rather than over it. (otherwise it looks goofy)
			if (targetOffset.GetY() <= 0) { targetOffset.SetY(targetOffset.GetY() * (1.0F - recoilScalar) + recoilScalar); }
			targetOffset *= m_Rotation;
            // In order to keep the held device from clipping through terrain, we need to determine where its muzzle position will be, and use that to figure out where its midpoint will be, as well as the distance between the two.
            Vector newMuzzlePos = (m_JointPos + targetOffset) - RotateOffset(heldDevice->GetJointOffset()) + RotateOffset(heldDevice->GetMuzzleOffset());
            Vector midToMuzzle = RotateOffset({heldDevice->GetRadius(), 0});
            Vector midOfDevice = newMuzzlePos - midToMuzzle;

            Vector terrainOrMuzzlePosition;
            g_SceneMan.CastStrengthRay(midOfDevice, midToMuzzle, 5, terrainOrMuzzlePosition, 0, false);
            targetOffset += g_SceneMan.ShortestDistance(newMuzzlePos, terrainOrMuzzlePosition, g_SceneMan.SceneWrapsX());
        } else {
            if (m_TargetPosition.IsZero()) {
                targetOffset = m_IdleOffset.GetXFlipped(m_HFlipped);
                m_DidReach = false;
            } else {
                targetOffset = g_SceneMan.ShortestDistance(m_JointPos, m_TargetPosition, g_SceneMan.SceneWrapsX());
                m_DidReach = m_WillIdle;
                if (m_WillIdle && targetOffset.GetMagnitude() > m_MaxLength) {
                    targetOffset = m_IdleOffset.GetXFlipped(m_HFlipped);
                    m_DidReach = false;
                }
            }
        }

        Vector distanceFromTargetOffsetToHandOffset(targetOffset - m_HandOffset);
        m_HandOffset += distanceFromTargetOffsetToHandOffset * m_MoveSpeed;
        m_HandOffset.ClampMagnitude(m_MaxLength, m_MaxLength / 2 + 0.1F);
    } else {
        m_HandOffset.SetXY(m_MaxLength * 0.65F, 0);
        m_HandOffset.RadRotate((m_HFlipped ? c_PI : 0) + m_Rotation.GetRadAngle());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arm::UpdateArmFrame() {
    if (IsAttached()) {
        float halfMax = m_MaxLength / 2.0F;
        unsigned int newFrame = static_cast<unsigned int>(std::floor(((m_HandOffset.GetMagnitude() - halfMax) / halfMax) * static_cast<float>(m_FrameCount)));
        m_Frame = std::clamp(newFrame, 0U, m_FrameCount - 1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arm::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (!onlyPhysical && (mode == g_DrawColor || mode == g_DrawWhite || mode == g_DrawTrans) && (!m_Parent || m_pHeldMO || (!m_pHeldMO && !m_DidReach))) {
        DrawHand(pTargetBitmap, targetPos, mode);
        if (m_pHeldMO && m_pHeldMO->IsDrawnAfterParent()) { m_pHeldMO->Draw(pTargetBitmap, targetPos, mode, onlyPhysical); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arm::DrawHand(BITMAP *targetBitmap, const Vector &targetPos, DrawMode mode) const {
    Vector handPos(m_JointPos + m_HandOffset + (m_Recoiled ? m_RecoilOffset : Vector()) - targetPos);
    handPos.m_X -= static_cast<float>((m_pHand->w / 2) + 1);
    handPos.m_Y -= static_cast<float>((m_pHand->h / 2) + 1);

    if (!m_HFlipped) {
        if (mode == g_DrawWhite) {
            draw_character_ex(targetBitmap, m_pHand, handPos.GetFloorIntX(), handPos.GetFloorIntY(), g_WhiteColor, -1);
        } else {
            draw_sprite(targetBitmap, m_pHand, handPos.GetFloorIntX(), handPos.GetFloorIntY());
        }
    } else {
        //TODO this draw_character_ex won't draw flipped. It should draw onto a temp bitmap and then draw that flipped. Maybe it can reuse a temp bitmap from MOSR, maybe not?
        if (mode == g_DrawWhite) {
            draw_character_ex(targetBitmap, m_pHand, handPos.GetFloorIntX(), handPos.GetFloorIntY(), g_WhiteColor, -1);
        } else {
            draw_sprite_h_flip(targetBitmap, m_pHand, handPos.GetFloorIntX(), handPos.GetFloorIntY());
        }
    }
}

} // namespace RTE