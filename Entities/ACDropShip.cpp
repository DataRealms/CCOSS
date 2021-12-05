//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACDropShip.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ACDropShip class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ACDropShip.h"
#include "AtomGroup.h"
#include "Controller.h"
#include "Matrix.h"
#include "AEmitter.h"
#include "PresetMan.h"

namespace RTE {

ConcreteClassInfo(ACDropShip, ACraft, 10);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACDropShip, effectively
//                  resetting the members of this abstraction level only.

void ACDropShip::Clear()
{
    m_pBodyAG = 0;
    m_pRThruster = 0;
    m_pLThruster = 0;
    m_pURThruster = 0;
    m_pULThruster = 0;
    m_pRHatch = 0;
    m_pLHatch = 0;
    m_HatchSwingRange.SetDegAngle(90);
    m_HatchOpeness = 0;
    m_LateralControl = 0;
	m_LateralControlSpeed = 6.0f;
	m_AutoStabilize = 1;
	m_MaxEngineAngle = 20.0f;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACDropShip object ready for use.

int ACDropShip::Create()
{
    if (ACraft::Create() < 0)
        return -1;

    // Save the AtomGroup read in by MOSRotating, as we are going to make it
    // into a composite group, and want to have the base body stored for reference.
    m_pBodyAG = dynamic_cast<AtomGroup *>(m_pAtomGroup->Clone());

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACDropShip to be identical to another, by deep copy.

int ACDropShip::Create(const ACDropShip &reference) {
    if (reference.m_pRThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRThruster->GetUniqueID()); }
    if (reference.m_pLThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLThruster->GetUniqueID()); }
    if (reference.m_pURThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pURThruster->GetUniqueID()); }
    if (reference.m_pULThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pULThruster->GetUniqueID()); }
    if (reference.m_pRHatch) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRHatch->GetUniqueID()); }
    if (reference.m_pLHatch) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLHatch->GetUniqueID()); }

    ACraft::Create(reference);

    if (reference.m_pRThruster) { SetRightThruster(dynamic_cast<AEmitter *>(reference.m_pRThruster->Clone())); }
    if (reference.m_pLThruster) { SetLeftThruster(dynamic_cast<AEmitter *>(reference.m_pLThruster->Clone())); }
    if (reference.m_pURThruster) { SetURightThruster(dynamic_cast<AEmitter *>(reference.m_pURThruster->Clone())); }
    if (reference.m_pULThruster) { SetULeftThruster(dynamic_cast<AEmitter *>(reference.m_pULThruster->Clone())); }
    if (reference.m_pRHatch) { SetRightHatch(dynamic_cast<Attachable *>(reference.m_pRHatch->Clone())); }
    if (reference.m_pLHatch) { SetLeftHatch(dynamic_cast<Attachable *>(reference.m_pLHatch->Clone())); }

    m_pBodyAG = dynamic_cast<AtomGroup *>(reference.m_pBodyAG->Clone());
    m_pBodyAG->SetOwner(this);
    m_HatchSwingRange = reference.m_HatchSwingRange;
    m_HatchOpeness = reference.m_HatchOpeness;

    m_LateralControl = reference.m_LateralControl;
    m_LateralControlSpeed = reference.m_LateralControlSpeed;
    m_AutoStabilize = reference.m_AutoStabilize;

	m_MaxEngineAngle = reference.m_MaxEngineAngle;

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ACDropShip::ReadProperty(const std::string_view &propName, Reader &reader) {
    if (propName == "RThruster" || propName == "RightThruster" || propName == "RightEngine") {
        SetRightThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "LThruster" || propName == "LeftThruster" || propName == "LeftEngine") {
        SetLeftThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "URThruster" || propName == "UpRightThruster") {
        SetURightThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "ULThruster" || propName == "UpLeftThruster") {
        SetULeftThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "RHatchDoor" || propName == "RightHatchDoor") {
        SetRightHatch(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "LHatchDoor" || propName == "LeftHatchDoor") {
        SetLeftHatch(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "HatchDoorSwingRange") {
        reader >> m_HatchSwingRange;
    } else if (propName == "AutoStabilize") {
        reader >> m_AutoStabilize;
    } else if (propName == "MaxEngineAngle") {
        reader >> m_MaxEngineAngle;
    } else if (propName == "LateralControlSpeed") {
        reader >> m_LateralControlSpeed;
    } else {
        return ACraft::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ACDropShip with a Writer for
//                  later recreation with Create(Reader &reader);

int ACDropShip::Save(Writer &writer) const
{
    ACraft::Save(writer);

    writer.NewProperty("RThruster");
    writer << m_pRThruster;
    writer.NewProperty("LThruster");
    writer << m_pLThruster;
    writer.NewProperty("URThruster");
    writer << m_pURThruster;
    writer.NewProperty("ULThruster");
    writer << m_pULThruster;
    writer.NewProperty("RHatchDoor");
    writer << m_pRHatch;
    writer.NewProperty("LHatchDoor");
    writer << m_pLHatch;
    writer.NewProperty("HatchDoorSwingRange");
    writer << m_HatchSwingRange;
    writer.NewProperty("AutoStabilize");
    writer << m_AutoStabilize;
	writer.NewProperty("MaxEngineAngle");
	writer << m_MaxEngineAngle;
	writer.NewProperty("LateralControlSpeed");
	writer << m_LateralControlSpeed;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ACDropShip object.

void ACDropShip::Destroy(bool notInherited)
{
    delete m_pBodyAG;

    if (!notInherited)
        ACraft::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.

float ACDropShip::GetAltitude(int max, int accuracy)
{
    // Check altitude both thrusters, and report the one closest to the ground.
    Vector rPos, lPos;

    if (m_pRThruster && m_pRThruster->IsAttached())
        rPos = m_Pos + RotateOffset(m_pRThruster->GetParentOffset());// + Vector(m_pRThruster->GetRadius(), 0));
    else
        rPos = m_Pos;

    if (m_pLThruster && m_pLThruster->IsAttached())
        lPos = m_Pos + RotateOffset(m_pLThruster->GetParentOffset());// + Vector(-m_pLThruster->GetRadius(), 0));
    else
        lPos = m_Pos;

    // Wrap the engine positions
    g_SceneMan.WrapPosition(lPos);
    g_SceneMan.WrapPosition(rPos);

    // Check center too
    float cAlt = g_SceneMan.FindAltitude(m_Pos, max, accuracy);
    float rAlt = g_SceneMan.FindAltitude(rPos, max, accuracy);
    float lAlt = g_SceneMan.FindAltitude(lPos, max, accuracy);

    // Return the lowest of the three
    return MIN(cAlt, MIN(rAlt, lAlt));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DetectObstacle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for obstacles in the travel direction.

MOID ACDropShip::DetectObstacle(float distance)
{
    // Check altitude both thrusters, and report the one closest to the ground.
    Vector rPos, lPos;

    if (m_pRThruster && m_pRThruster->IsAttached())
        rPos = m_Pos + RotateOffset(m_pRThruster->GetParentOffset() + Vector(m_pRThruster->GetRadius(), 0));
    else
        rPos = m_Pos;

    if (m_pLThruster && m_pLThruster->IsAttached())
        lPos = m_Pos + RotateOffset(m_pLThruster->GetParentOffset() + Vector(-m_pLThruster->GetRadius(), 0));
    else
        lPos = m_Pos;

    // Wrap the engine positions
    g_SceneMan.WrapPosition(lPos);
    g_SceneMan.WrapPosition(rPos);

    // Make the ray to check along point in an appropriate direction
    Vector checkRay;
    if (m_AltitudeMoveState == DESCEND)
        checkRay.m_Y = distance;
    else if (m_AltitudeMoveState == ASCEND)
        checkRay.m_Y = -distance;
    // Just rotate it to align iwth the velocity
    else
    {
        checkRay.m_X = distance;
        checkRay.AbsRotateTo(m_Vel);
    }

    MOID detected = g_NoMOID;

    // Check center too?
    if ((detected = g_SceneMan.CastMORay(m_Pos, checkRay, m_RootMOID, Activity::NoTeam, 0, true, 30)) != g_NoMOID)
        return detected;
    if ((detected = g_SceneMan.CastMORay(rPos, checkRay, m_RootMOID, Activity::NoTeam, 0, true, 30)) != g_NoMOID)
        return detected;
    if ((detected = g_SceneMan.CastMORay(lPos, checkRay, m_RootMOID, Activity::NoTeam, 0, true, 30)) != g_NoMOID)
        return detected;

    return false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool ACDropShip::OnBounce(const Vector &pos)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool ACDropShip::OnSink(const Vector &pos)
{
    return false;
}
*/


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.

void ACDropShip::UpdateAI()
{
    float angle = m_Rotation.GetRadAngle();

    // This is the altitude at which the craft will hover and unload its cargo
    float hoverAltitude = m_CharHeight * 2;
    // The gutter range threshold for how much above and below the hovering altitude is ok to stay in
    float hoverRange = m_CharHeight / 4;
    // Get the altitude reading, within 25 pixels precision
    float altitude = GetAltitude(g_SceneMan.GetSceneHeight() / 2, 25);

    //////////////////////////////////////////
    // Try to avoid lateral drift by default

    if (m_Vel.m_X > 2.0)
        m_LateralMoveState = LAT_LEFT;
    else if (m_Vel.m_X < -2.0)
        m_LateralMoveState = LAT_RIGHT;

    ////////////////////////////
    // Delivery Sequence logic

    if (m_DeliveryState == FALL)
    {
        m_AltitudeMoveState = DESCEND;

        // Check for something in the way of our descent, and hover to the side to avoid it
        MOID detected = g_NoMOID;
        //TODO since GetRadius includes attachables, this could probably be lessened. Maybe could just use GetDiameter directly.
        if ((detected = DetectObstacle(GetRadius() * 4)) != g_NoMOID)
        {
            // Only check other craft in the way
            ACraft *pCraft = dynamic_cast<ACraft *>(g_MovableMan.GetMOFromID(detected));
            if (pCraft)
            {
                // Start ASCENDing until descent stops
                m_AltitudeMoveState = m_Vel.m_Y > 1.0f ? ASCEND : HOVER;
                m_LateralMoveState = pCraft->GetPos().m_X > m_Pos.m_X ? LAT_LEFT : LAT_RIGHT;
            }
        }

        // If we passed the hover altitude upper limit, start unloading
        if (altitude < hoverAltitude + hoverRange * 2)
        {
            // Start unloading if there's something to unload
            if (!IsInventoryEmpty() && m_AIMode != AIMODE_STAY)
            {
                // Randomly choose a direction to be going when unloading
                m_LateralMoveState = RandomNum() > 0.5F ? LAT_LEFT : LAT_RIGHT;
                DropAllInventory();
                m_DeliveryState = UNLOAD;
                // Start ascending since we're probably doing downward at max already
                m_AltitudeMoveState = ASCEND;
            }
            // Otherwise, just hover in place and wait for manual control
            else
            {
                m_LateralMoveState = LAT_STILL;
                m_DeliveryState = STANDBY;
            }
        }

        // Don't descend if we have nothing to deliver
        if (IsInventoryEmpty() && HasDelivered() && m_AIMode != AIMODE_STAY)
        {
            m_DeliveryState = LAUNCH;
        }
    }
    else if (m_DeliveryState == STANDBY || m_DeliveryState == UNLOAD)
    {
        // Adjust altitude to keep the appropriate altitude over the ground
        if (altitude < hoverAltitude - hoverRange)
            m_AltitudeMoveState = ASCEND;
        else if (altitude > hoverAltitude + hoverRange)
            m_AltitudeMoveState = DESCEND;
        else
            m_AltitudeMoveState = HOVER;

        // If we're unloading, and done, then launch off again
        if (m_DeliveryState == UNLOAD && IsInventoryEmpty())
        {
            m_LateralMoveState = LAT_STILL;
            m_DeliveryState = LAUNCH;
            m_StuckTimer.Reset();
        }
    }
    else if (m_DeliveryState == LAUNCH)
    {
        if (m_HatchTimer.IsPastSimMS(1000))
            CloseHatch();
        m_AltitudeMoveState = ASCEND;

        // Check for something in the way of our ascent, and hover to the side to avoid it
        MOID detected = g_NoMOID;
        //TODO since GetRadius includes attachables, this could probably be lessened. Maybe could just use GetDiameter directly.
        if ((detected = DetectObstacle(GetRadius() * 4)) != g_NoMOID)
        {
            // Only check other craft in the way
            ACraft *pCraft = dynamic_cast<ACraft *>(g_MovableMan.GetMOFromID(detected));
            if (pCraft)
            {
                // Start ASCENDing until descent stops
                m_AltitudeMoveState = m_Vel.m_Y < -1.0f ? DESCEND : HOVER;
                m_LateralMoveState = pCraft->GetPos().m_X > m_Pos.m_X ? LAT_LEFT : LAT_RIGHT;
            }
        }
    }

    /////////////////////////
    // If we are hopelessly stuck, self destruct

    if (m_RecentMovementMag > 10)
        m_StuckTimer.Reset();
    if (m_StuckTimer.IsPastSimMS(10000))
        GibThis();

    /////////////////////////
    // INPUT TRANSLATION

    if (m_AltitudeMoveState == ASCEND)
        m_Controller.SetState(MOVE_UP, true);
    else if (m_AltitudeMoveState == DESCEND)
        m_Controller.SetState(MOVE_DOWN, true);

    if (m_LateralMoveState == LAT_RIGHT)
    {
//        m_Controller.SetState(MOVE_RIGHT, true);
        // Move slower
        m_Controller.m_AnalogMove.m_X = 0.35;
    }
    else if (m_LateralMoveState == LAT_LEFT)
    {
//        m_Controller.SetState(MOVE_LEFT, true);
        m_Controller.m_AnalogMove.m_X = -0.35;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ACDropShip. Supposed to be done every frame.

void ACDropShip::Update()
{
	/////////////////////////////////
	// Controller update and handling

// TODO: Improve and make optional thrusters more robust!
	if (m_Status != DEAD && m_Status != DYING)
	{
		float targetYVel = 0;
		float throttleRange = 7.5f;

		if (m_Controller.IsState(PRESS_UP))
		{
			// This is to make sure se get loose from being sideways stuck
			m_ForceDeepCheck = true;
		}
		// TODO: make framerate independent!
				// Altitude control, check analog first
		if (fabs(m_Controller.GetAnalogMove().m_Y) > 0.1)
		{
			targetYVel = -m_Controller.GetAnalogMove().m_Y * throttleRange;
		}
		// Fall back to digital altitude control
		else if (m_Controller.IsState(MOVE_UP) || m_Controller.IsState(AIM_UP))
			targetYVel = throttleRange;
		else if (m_Controller.IsState(MOVE_DOWN) || m_Controller.IsState(AIM_DOWN))
			targetYVel = -throttleRange;

		//////////////////////////////////////////////////////
		// Main thruster throttling to stay hovering

		// This is to trim the hover so it's perfectly still altitude-wise
		float trimming = -1.75f;
		float throttle = (targetYVel + (m_Vel.m_Y + trimming)) / throttleRange;
		//AEmitter do this already: throttle = throttle > 1.0f ? 1.0f : (throttle < -1.0f ? -1.0f : throttle);

		// Right main thruster
		if (m_pRThruster && m_pRThruster->IsAttached())
		{
			float rightThrottle = throttle;
			// Throttle override control for correcting heavy tilt, only applies if both engines are present
			if (m_pLThruster && m_pLThruster->IsAttached())
			{
				if (m_Rotation.GetRadAngle() > c_SixteenthPI)
					rightThrottle = -0.8f;
				else if (m_Rotation.GetRadAngle() < -c_SixteenthPI)
					rightThrottle = 0.8f;
			}

			if (rightThrottle > m_pRThruster->GetThrottle())
				rightThrottle = rightThrottle * 0.3f + m_pRThruster->GetThrottle() * 0.7f;  // Increase throttle slowly

			m_pRThruster->EnableEmission(m_Status == STABLE);
			m_pRThruster->SetThrottle(rightThrottle);
			m_pRThruster->SetFlashScale((rightThrottle + 1.5f) / 2);
			// Engines are noisy! Make AI aware of them
			m_pRThruster->AlarmOnEmit(m_Team);
		}
		// Left main thruster
		if (m_pLThruster && m_pLThruster->IsAttached())
		{
			float leftThrottle = throttle;
			// Throttle override control for correcting heavy tilt, only applies if both engines are present
			if (m_pRThruster && m_pRThruster->IsAttached())
			{
				if (m_Rotation.GetRadAngle() > c_SixteenthPI)
					leftThrottle = 0.8f;
				else if (m_Rotation.GetRadAngle() < -c_SixteenthPI)
					leftThrottle = -0.8f;
			}

			if (leftThrottle > m_pLThruster->GetThrottle())
				leftThrottle = leftThrottle * 0.3f + m_pLThruster->GetThrottle() * 0.7f;  // Increase throttle slowly

			m_pLThruster->EnableEmission(m_Status == STABLE);
			m_pLThruster->SetThrottle(leftThrottle);
			m_pLThruster->SetFlashScale((leftThrottle + 1.5f) / 2);
			// Engines are noisy! Make AI aware of them
			m_pLThruster->AlarmOnEmit(m_Team);
		}

		///////////////////////////////////////////////
		// Lateral control

		// Check analog first
		if (fabs(m_Controller.GetAnalogMove().m_X) > 0.1)
		{
			if (m_LateralControl < -m_Controller.GetAnalogMove().m_X)
				m_LateralControl += m_LateralControlSpeed * g_TimerMan.GetDeltaTimeSecs();   // 0.1 per update at 60fps
			else if (m_LateralControl > -m_Controller.GetAnalogMove().m_X)
				m_LateralControl -= m_LateralControlSpeed * g_TimerMan.GetDeltaTimeSecs();
		}
		// Fall back to digital lateral control
		else
		{
			if (m_Controller.IsState(MOVE_RIGHT))
				m_LateralControl -= m_LateralControlSpeed * g_TimerMan.GetDeltaTimeSecs();
			else if (m_Controller.IsState(MOVE_LEFT))
				m_LateralControl += m_LateralControlSpeed * g_TimerMan.GetDeltaTimeSecs();
			else if (m_LateralControl != 0.0)
				m_LateralControl *= 54.0f * g_TimerMan.GetDeltaTimeSecs();  // 90% per update at 60fps
		}

		// Clamp the lateral control
		if (m_LateralControl > 1.0)
			m_LateralControl = 1.0;
		else if (m_LateralControl < -1.0)
			m_LateralControl = -1.0;

		if (m_Controller.IsState(PRESS_FACEBUTTON))
		{
			if (m_HatchState == CLOSED)
				DropAllInventory();
			else if (m_HatchState == OPEN)
				CloseHatch();
		}
	}
	// No Controller present, or dead
	else
	{
		if (m_pRThruster && m_pRThruster->IsAttached())
			m_pRThruster->EnableEmission(false);
		if (m_pLThruster && m_pLThruster->IsAttached())
			m_pLThruster->EnableEmission(false);
		/*
				if (m_pURThruster && m_pURThruster->IsAttached())
					m_pURThruster->EnableEmission(false);
				if (m_pULThruster && m_pULThruster->IsAttached())
					m_pULThruster->EnableEmission(false);
		*/
	}

	////////////////////////////////////////
	// Hatch Operation

	if (m_HatchState == OPENING)
	{
		if (m_HatchDelay > 0 && !m_HatchTimer.IsPastSimMS(m_HatchDelay))
			m_HatchOpeness = (float)m_HatchTimer.GetElapsedSimTimeMS() / (float)m_HatchDelay;
		else
		{
			m_HatchOpeness = 1.0;
			m_HatchState = OPEN;
			DropAllInventory();
		}
	}
	else if (m_HatchState == CLOSING)
	{
		if (m_HatchDelay > 0 && !m_HatchTimer.IsPastSimMS(m_HatchDelay))
			m_HatchOpeness = 1.0 - ((float)m_HatchTimer.GetElapsedSimTimeMS() / (float)m_HatchDelay);
		else
		{
			m_HatchOpeness = 0;
			m_HatchState = CLOSED;
		}
	}

    /////////////////////////////////
    // Manage Attachable:s
    Matrix engineRot = 0;
    if (m_Rotation.GetDegAngle() > m_MaxEngineAngle) {
        engineRot.SetDegAngle(m_Rotation.GetDegAngle() - m_MaxEngineAngle);
    } else if (m_Rotation.GetDegAngle() < -m_MaxEngineAngle) {
        engineRot.SetDegAngle(m_Rotation.GetDegAngle() + m_MaxEngineAngle);
    } else {
        // Lateral control application
        engineRot.SetDegAngle(m_MaxEngineAngle * m_LateralControl);
    }

    if (m_pRThruster && m_pRThruster->IsAttached()) {
        m_pRThruster->SetRotAngle(engineRot.GetRadAngle());
        m_pRThruster->SetAngularVel(0.0F);
    }

    if (m_pLThruster && m_pLThruster->IsAttached()) {
        m_pLThruster->SetRotAngle(engineRot.GetRadAngle());
        m_pLThruster->SetAngularVel(0.0F);
    }

    // Auto balancing with the up thrusters
    if (m_pURThruster && m_pURThruster->IsAttached() && m_pULThruster && m_pULThruster->IsAttached()) {
        if (m_AutoStabilize) {
            // Use a PD-controller for balance
            float change = 0.9F * m_AngularVel + 0.8F * m_Rotation.GetRadAngle();
            if (change > 0.2F) {
                if (!m_pURThruster->IsEmitting()) {
                    m_pURThruster->TriggerBurst();
                }
                m_pURThruster->EnableEmission(true);
            } else {
                m_pURThruster->EnableEmission(false);
            }

            if (change < -0.2F) {
                if (!m_pULThruster->IsEmitting()) {
                    m_pULThruster->TriggerBurst();
                }
                m_pULThruster->EnableEmission(true);
            } else {
                m_pULThruster->EnableEmission(false);
            }
        }
    }

    // Hatch door pieces
    if (m_pRHatch && m_pRHatch->IsAttached()) {
        m_pRHatch->SetRotAngle(m_Rotation.GetRadAngle() + m_HatchSwingRange.GetRadAngle() * m_HatchOpeness);
    }

    if (m_pLHatch && m_pLHatch->IsAttached()) {
        m_pLHatch->SetRotAngle(m_Rotation.GetRadAngle() - m_HatchSwingRange.GetRadAngle() * m_HatchOpeness);
    }

	/////////////////////////////////////////////////
	// Update MovableObject, adds on the forces etc, updated viewpoint
	ACraft::Update();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetRightThruster(AEmitter *newThruster) {
    if (m_pRThruster && m_pRThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pRThruster); }
    if (newThruster == nullptr) {
        m_pRThruster = nullptr;
    } else {
        m_pRThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightThruster");
            dynamic_cast<ACDropShip *>(parent)->SetRightThruster(castedAttachable);
        }});

        if (m_pRThruster->HasNoSetDamageMultiplier()) { m_pRThruster->SetDamageMultiplier(1.0F); }
        m_pRThruster->SetInheritsRotAngle(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetLeftThruster(AEmitter *newThruster) {
    if (m_pLThruster && m_pLThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pLThruster); }
    if (newThruster == nullptr) {
        m_pLThruster = nullptr;
    } else {
        m_pLThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftThruster");
            dynamic_cast<ACDropShip *>(parent)->SetLeftThruster(castedAttachable);
        }});

        if (m_pLThruster->HasNoSetDamageMultiplier()) { m_pLThruster->SetDamageMultiplier(1.0F); }
        m_pLThruster->SetInheritsRotAngle(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetURightThruster(AEmitter *newThruster) {
    if (m_pURThruster && m_pURThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pURThruster); }
    if (newThruster == nullptr) {
        m_pURThruster = nullptr;
    } else {
        m_pURThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetURightThruster");
            dynamic_cast<ACDropShip *>(parent)->SetURightThruster(castedAttachable);
        }});

        if (m_pURThruster->HasNoSetDamageMultiplier()) { m_pURThruster->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetULeftThruster(AEmitter *newThruster) {
    if (m_pULThruster && m_pULThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pULThruster); }
    if (newThruster == nullptr) {
        m_pULThruster = nullptr;
    } else {
        m_pULThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetULeftThruster");
            dynamic_cast<ACDropShip *>(parent)->SetULeftThruster(castedAttachable);
        }});

        if (m_pULThruster->HasNoSetDamageMultiplier()) { m_pULThruster->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetRightHatch(Attachable *newHatch) {
    if (m_pRHatch && m_pRHatch->IsAttached()) { RemoveAndDeleteAttachable(m_pRHatch); }
    if (newHatch == nullptr) {
        m_pRHatch = nullptr;
    } else {
        m_pRHatch = newHatch;
        AddAttachable(newHatch);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newHatch->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            dynamic_cast<ACDropShip *>(parent)->SetRightHatch(attachable);
        }});

        if (m_pRHatch->HasNoSetDamageMultiplier()) { m_pRHatch->SetDamageMultiplier(1.0F); }
        m_pRHatch->SetInheritsRotAngle(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDropShip::SetLeftHatch(Attachable *newHatch) {
    if (m_pLHatch && m_pLHatch->IsAttached()) { RemoveAndDeleteAttachable(m_pLHatch); }
    if (newHatch == nullptr) {
        m_pLHatch = nullptr;
    } else {
        m_pLHatch = newHatch;
        AddAttachable(newHatch);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newHatch->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            dynamic_cast<ACDropShip *>(parent)->SetLeftHatch(attachable);
        }});

        if (m_pLHatch->HasNoSetDamageMultiplier()) { m_pLHatch->SetDamageMultiplier(1.0F); }
        m_pLHatch->SetInheritsRotAngle(false);
    }
}

} // namespace RTE
