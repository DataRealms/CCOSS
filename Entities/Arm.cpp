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

ConcreteClassInfo(Arm, Attachable, 50)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Arm, effectively
//                  resetting the members of this abstraction level only.

void Arm::Clear()
{
//    m_HandOffset.Reset();
    m_pHeldMO = 0;
    m_HandFile.Reset();
    m_pHand = 0;
    m_MaxLength = 0;
    m_HandOffset.Reset();
    m_TargetPoint.Reset();
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
    if (MOSRotating::Create() < 0) {
        return -1;
    }

    // Ensure Arms don't get flagged as inheriting RotAngle, since they never do and always set their RotAngle for themselves.
    m_InheritsRotAngle = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Arm to be identical to another, by deep copy.

int Arm::Create(const Arm &reference) {
    if (reference.m_pHeldMO) { CloneHardcodedAttachable(dynamic_cast<Attachable *>(reference.m_pHeldMO), this, static_cast<std::function<void(Arm &, Attachable *)>>((void (Arm:: *)(Attachable *newHeldMO)) &Arm::SetHeldMO)); }
    Attachable::Create(reference);

    m_HandFile = reference.m_HandFile;
    m_pHand = m_HandFile.GetAsBitmap();
    RTEAssert(m_pHand, "Failed to load hand bitmap in Arm::Create")

    m_MaxLength = reference.m_MaxLength;
    m_HandOffset = reference.m_HandOffset;
    m_TargetPoint = reference.m_TargetPoint;
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

int Arm::ReadProperty(std::string propName, Reader &reader) {
    if (propName == "HeldDevice") {
        const Entity *pEntity;
        pEntity = g_PresetMan.GetEntityPreset(reader);
        if (pEntity) {
            m_pHeldMO = dynamic_cast<MovableObject *>(pEntity->Clone());
        }
        pEntity = 0;
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
//                  (if there is one) will be dropped and become a detached MovableObject,

void Arm::SetHeldMO(MovableObject *newHeldMO)
{
// TODO: NEED TO REWORK THIS TO WORK WITH THROWNDEVICES!!
    if (m_pHeldMO && m_pHeldMO->IsHeldDevice() && dynamic_cast<HeldDevice *>(m_pHeldMO)->IsAttachedTo(this)) {
        HeldDevice *pHeldDev = dynamic_cast<HeldDevice *>(m_pHeldMO);
        if (pHeldDev->IsAttached()) { dynamic_cast<MOSRotating *>(pHeldDev->GetParent())->RemoveAttachable(pHeldDev); }
// TODO: Refine throwing force to dropped device here?")
        pHeldDev->SetVel(Vector(10 * PosRand(), -15 * PosRand()));
        pHeldDev->SetAngularVel(-10 * PosRand());
        g_MovableMan.AddItem(pHeldDev);
        m_pHeldMO = pHeldDev = 0;
    }

    if (newHeldMO && (newHeldMO->IsHeldDevice() || newHeldMO->IsThrownDevice())) {
        Attachable *pNewDev = dynamic_cast<Attachable *>(newHeldMO);
        if (pNewDev->IsAttached()) { dynamic_cast<MOSRotating *>(pNewDev->GetParent())->RemoveAttachable(pNewDev); }
        AddAttachable(pNewDev);
        pNewDev->SetTeam(m_Team);
        pNewDev = 0;
    }
    m_pHeldMO = newHeldMO;
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
    m_pHeldMO = 0;
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
        RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO));
        g_MovableMan.AddItem(m_pHeldMO);
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
    m_TargetPoint = scenePoint;
    m_WillIdle = true;
/*
    if (m_HFlipped) {
        m_Pos.m_X -= m_ParentOffset.m_X;
        m_Pos.m_Y += m_ParentOffset.m_Y;
    }
    else
        m_Pos += m_ParentOffset;

    Vector reachVec(m_TargetPoint - m_Pos);
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
    m_TargetPoint = scenePoint;
    m_WillIdle = false;
/*
    if (m_HFlipped) {
        m_Pos.m_X -= m_ParentOffset.m_X;
        m_Pos.m_Y += m_ParentOffset.m_Y;
    }
    else
        m_Pos += m_ParentOffset;

    Vector reachVec(m_TargetPoint - m_Pos);
    return reachVec.GetMagnitude() <= m_MaxLength &&
           reachVec.GetMagnitude() >= (m_MaxLength / 2);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConstrainHand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the hand distance is constrained between the max length of
//                  this Arm and half the max length.

bool Arm::ConstrainHand()
{
        float halfMax = m_MaxLength / 2;
        if (m_HandOffset.GetMagnitude() > m_MaxLength) {
            m_HandOffset.SetMagnitude(m_MaxLength);
            return false;
        }
        else if (m_HandOffset.GetMagnitude() < halfMax) {
            m_HandOffset.SetMagnitude(halfMax + 0.1);
            return true;
        }
        else
            return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void Arm::GibThis(const Vector &impactImpulse, float internalBlast, MovableObject *pIgnoreMO) {
    DropEverything();
    Attachable::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Arm. Supposed to be done every frame.

void Arm::Update() {


    Attachable::Update();

    if (!m_Parent) {
        // When arm is detached, let go of whatever it is holding 
        if (m_pHeldMO) {
            m_pHeldMO->SetVel(m_Vel + Vector(-10 * PosRand(), -15 * PosRand()));
            m_pHeldMO->SetAngularVel(-7);
            if (m_pHeldMO->IsDevice()) { RemoveAttachable(dynamic_cast<Attachable *>(m_pHeldMO)); }
            g_MovableMan.AddItem(m_pHeldMO);
            m_pHeldMO = 0;
        }
        // Update hand
        m_HandOffset.SetXY(m_MaxLength * 0.65, 0);
        m_HandOffset.RadRotate((m_HFlipped ? c_PI : 0) + m_Rotation.GetRadAngle());
    } else {
        // Attached, so act like it

        // If a Firearm or Shield device is held, but not a throwable, update it and arm configuration accordingly.
        HeldDevice *pHeldDev = dynamic_cast<HeldDevice *>(m_pHeldMO);
        if (pHeldDev && !dynamic_cast<ThrownDevice *>(m_pHeldMO))
        {
            // Indicate that the arm wasn't reaching for anything this frame.
            m_DidReach = false;
            pHeldDev->SetHFlipped(m_HFlipped);

            Vector handTarget(pHeldDev->GetStanceOffset());
            handTarget *= m_Rotation/* + m_pParent->GetRotMatrix()*/;
//            handTarget.RadRotate(m_pParent->GetRotMatrix());

            // Predict where the new muzzle position will be if we don't try to clear the muzzle of terrain
            Vector newMuzzlePos = (m_JointPos + handTarget) - RotateOffset(pHeldDev->GetJointOffset()) + RotateOffset(pHeldDev->GetMuzzleOffset());
            // Adjust the hand offset back if necessary so that the weapon's muzzle doesn't poke into terrain
            Vector midToMuzzle(pHeldDev->GetRadius(), 0);
            midToMuzzle = RotateOffset(midToMuzzle);

            // Figure out where the back butt of the device will be without adjustment
            Vector midOfDevice = newMuzzlePos - midToMuzzle;

            // Trace from the back toward the muzzle, finding first pixel of impassable hardness
            Vector freeMuzzlePos;
            g_SceneMan.CastStrengthRay(midOfDevice, midToMuzzle, 5, freeMuzzlePos, 0, false);
            Vector muzzleAdjustment = g_SceneMan.ShortestDistance(newMuzzlePos, freeMuzzlePos);
            // Only apply if it's large enough
            if (muzzleAdjustment.GetMagnitude() > 2.0f)
                handTarget += muzzleAdjustment;

            // Interpolate the hand offset to the hand target
            handTarget -= m_HandOffset;
            m_HandOffset += handTarget * m_MoveSpeed;

            // Make sure the weapon cannot be extended beyond the reach of the arm.
            ConstrainHand();

            float handAngle = m_HandOffset.GetAbsRadAngle();

            // In order to keep the HeldDevice in the right place, we need to convert its offset (the hand offset) to work as the ParentOffset for the HeldDevice.
            // The HeldDevice will then use this to set its JointPos when it's updated. Unfortunately UnRotateOffset doesn't work for this, since it's Vector/Matrix division, which isn't commutative.
            Vector handOffsetAsParentOffset = RotateOffset(m_JointOffset) + m_HandOffset;
            handOffsetAsParentOffset.RadRotate(-m_Rotation.GetRadAngle()).FlipX(m_HFlipped);
            pHeldDev->SetParentOffset(handOffsetAsParentOffset);
            
            if (pHeldDev->IsRecoiled())
                m_Parent->AddImpulseForce(pHeldDev->GetRecoilForce());
            else
                m_Recoiled = false;

            m_Rotation = (m_HFlipped ? c_PI : 0) + handAngle;

            // Redo the positioning of the arm now since the rotation has changed and RotateOffset will return different results
            m_Pos = m_JointPos - RotateOffset(m_JointOffset);

            // If it blew up or whatever, releaes it from hand and put into scene so it'll be cleaned up properly
            if (m_pHeldMO && m_pHeldMO->IsSetToDelete())
                g_MovableMan.AddItem(ReleaseHeldMO());
        }
        // Adjust rotation and hand distance if reaching toward something.
        else
        {
            // Not reaching toward anything
            if (m_TargetPoint.IsZero())
            {
                Vector moveVec(m_IdleOffset.GetXFlipped(m_HFlipped) - m_HandOffset);
                m_HandOffset += moveVec * m_MoveSpeed;
                m_DidReach = false;
            }
            else
            {
                Vector handTarget = g_SceneMan.ShortestDistance(m_JointPos, m_TargetPoint);

                // Check if handTarget is within arm's length.
    // TEMP the +3 is a hack! improve
                if (handTarget.GetMagnitude() <= m_MaxLength || !m_WillIdle/* && handTarget.GetFloored() != m_HandOffset.GetFloored()*/)
                {
                    Vector moveVec(handTarget - m_HandOffset);
                    m_HandOffset += moveVec * m_MoveSpeed;
                    m_DidReach = m_WillIdle;
                }
                else /*if (m_IdleOffset.GetXFlipped(m_HFlipped).GetFloored() != m_HandOffset.GetFloored())*/
                {
                    Vector moveVec(m_IdleOffset.GetXFlipped(m_HFlipped) - m_HandOffset);
                    m_HandOffset += moveVec * m_MoveSpeed;
                    m_DidReach = false;
                }
            }
            // Cap hand distance to what the Arm allows
            ConstrainHand();
            m_Rotation = (m_HFlipped ? c_PI : 0) + m_HandOffset.GetAbsRadAngle();

            // Redo the positioning of the arm now since the rotation has changed and RotateOffset will return different results
            m_Pos = m_JointPos - RotateOffset(m_JointOffset);

            // If holding something other than a FireArm, then update it
            if (m_pHeldMO) {
                Attachable *pHeldDev = dynamic_cast<Attachable *>(m_pHeldMO);

                // In order to keep the HeldDevice in the right place, we need to convert its offset (the hand offset) to work as the ParentOffset for the HeldDevice.
                // The HeldDevice will then use this to set its JointPos when it's updated. Unfortunately UnRotateOffset doesn't work for this, since it's Vector/Matrix division, which isn't commutative.
                Vector handOffsetAsParentOffset = RotateOffset(m_JointOffset) + m_HandOffset;
                handOffsetAsParentOffset.RadRotate(-m_Rotation.GetRadAngle()).FlipX(m_HFlipped);
                pHeldDev->SetParentOffset(handOffsetAsParentOffset);

                // If it blew up or whatever, releaes it from hand and put into scene so it'll be cleaned up properly
                if (m_pHeldMO->IsSetToDelete())
                    g_MovableMan.AddItem(ReleaseHeldMO());
            }
        }

// TODO: improve!")
        // Set correct frame for arm bend.
        float halfMax = m_MaxLength / 2;
        float balle = m_HandOffset.GetMagnitude() - halfMax;
        float temp = (m_HandOffset.GetMagnitude() - halfMax) / halfMax;
        temp *= m_FrameCount;
        int newFrame = floorf(temp);
        newFrame -= newFrame >= m_FrameCount ? 1 : 0;
        m_Frame = newFrame;

//        m_aSprite->SetAngle(m_Rotation);
//        m_aSprite->SetScale(m_Scale);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Arm's current graphical representation to a
//                  BITMAP of choice.

void Arm::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    if (m_pHeldMO || (!m_pHeldMO && !m_DidReach) || !m_Parent) {
        if (!onlyPhysical && (mode == g_DrawColor || mode == g_DrawWhite || mode == g_DrawTrans)) {
            DrawHand(pTargetBitmap, targetPos, mode);
        }
        if (m_pHeldMO && m_pHeldMO->IsDrawnAfterParent()) {
            m_pHeldMO->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Arm's hand's graphical representation to a BITMAP of
//                  choice.

void Arm::DrawHand(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode) const {
    Vector handPos(m_JointPos + m_HandOffset + (m_Recoiled ? m_RecoilOffset : Vector()) - targetPos);
    handPos.m_X -= (m_pHand->w / 2) + 1;
    handPos.m_Y -= (m_pHand->h / 2) + 1;

    if (!m_HFlipped) {
        if (mode == g_DrawWhite) {
            draw_character_ex(pTargetBitmap, m_pHand, handPos.GetRoundIntX(), handPos.GetRoundIntY(), g_WhiteColor, -1);
        } else {
            draw_sprite(pTargetBitmap, m_pHand, handPos.GetRoundIntX(), handPos.GetRoundIntY());
        }
    } else {
        //TODO this won't draw flipped. It should draw onto a temp bitmap and then draw that flipped. Maybe it can reuse a temp bitmap from MOSR, maybe not?
        if (mode == g_DrawWhite) {
            draw_character_ex(pTargetBitmap, m_pHand, handPos.GetRoundIntX(), handPos.GetRoundIntY(), g_WhiteColor, -1);
        } else {
            draw_sprite_h_flip(pTargetBitmap, m_pHand, handPos.GetRoundIntX(), handPos.GetRoundIntY());
        }
    }
}

} // namespace RTE