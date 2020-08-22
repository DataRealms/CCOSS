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

namespace RTE {

ConcreteClassInfo(ACDropShip, ACraft, 10)


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
// TODO: don't hardcode
    m_HatchSwingRange.SetDegAngle(90);
    m_HatchOpeness = 0;
    m_LateralControl = 0;
	m_LateralControlSpeed = 6.0f;
	m_AutoStabilize = 1;
    m_ScuttleIfFlippedTime = 4000;
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

int ACDropShip::Create(const ACDropShip &reference)
{
    ACraft::Create(reference);

    m_pBodyAG = dynamic_cast<AtomGroup *>(reference.m_pBodyAG->Clone());
    m_pBodyAG->SetOwner(this);

    if (reference.m_pRThruster)
    {
        m_pRThruster = dynamic_cast<AEmitter *>(reference.m_pRThruster->Clone());
		m_pRThruster->SetCanCollideWithTerrainWhenAttached(true);
        AddAttachable(m_pRThruster, true);
    }
    if (reference.m_pLThruster)
    {
        m_pLThruster = dynamic_cast<AEmitter *>(reference.m_pLThruster->Clone());
		m_pLThruster->SetCanCollideWithTerrainWhenAttached(true);
        AddAttachable(m_pLThruster, true);
    }
    if (reference.m_pURThruster)
    {
        m_pURThruster = dynamic_cast<AEmitter *>(reference.m_pURThruster->Clone());
        AddAttachable(m_pURThruster, true);
    }
    if (reference.m_pULThruster)
    {
        m_pULThruster = dynamic_cast<AEmitter *>(reference.m_pULThruster->Clone());
        AddAttachable(m_pULThruster, true);
    }
    if (reference.m_pRHatch)
    {
        m_pRHatch = dynamic_cast<Attachable *>(reference.m_pRHatch->Clone());
		m_pRHatch->SetCanCollideWithTerrainWhenAttached(true);
        AddAttachable(m_pRHatch, true);
    }
    if (reference.m_pLHatch)
    {
        m_pLHatch = dynamic_cast<Attachable *>(reference.m_pLHatch->Clone());
		m_pLHatch->SetCanCollideWithTerrainWhenAttached(true);
        AddAttachable(m_pLHatch, true);
    }
    m_HatchSwingRange = reference.m_HatchSwingRange;
    m_HatchOpeness = reference.m_HatchOpeness;

    m_LateralControl = reference.m_LateralControl;
    m_LateralControlSpeed = reference.m_LateralControlSpeed;
    m_AutoStabilize = reference.m_AutoStabilize;
    m_ScuttleIfFlippedTime = reference.m_ScuttleIfFlippedTime;

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

int ACDropShip::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "RThruster")
    {
        delete m_pRThruster;
        m_pRThruster = new AEmitter;
        reader >> m_pRThruster;
    }
    else if (propName == "LThruster")
    {
        delete m_pLThruster;
        m_pLThruster = new AEmitter;
        reader >> m_pLThruster;
    }
    else if (propName == "URThruster")
    {
        delete m_pURThruster;
        m_pURThruster = new AEmitter;
        reader >> m_pURThruster;
    }
    else if (propName == "ULThruster")
    {
        delete m_pULThruster;
        m_pULThruster = new AEmitter;
        reader >> m_pULThruster;
    }
    else if (propName == "RHatchDoor")
    {
        delete m_pRHatch;
        m_pRHatch = new Attachable;
        reader >> m_pRHatch;
    }
    else if (propName == "LHatchDoor")
    {
        delete m_pLHatch;
        m_pLHatch = new Attachable;
        reader >> m_pLHatch;
    }
    else if (propName == "HatchDoorSwingRange")
        reader >> m_HatchSwingRange;
    else if (propName == "AutoStabilize")
        reader >> m_AutoStabilize;
    else if (propName == "ScuttleIfFlippedTime")
        reader >> m_ScuttleIfFlippedTime;
	else if (propName == "MaxEngineAngle")
		reader >> m_MaxEngineAngle;
	else if (propName == "LateralControlSpeed")
		reader >> m_LateralControlSpeed;
	else
        return ACraft::ReadProperty(propName, reader);

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
    writer.NewProperty("ScuttleIfFlippedTime");
    writer << m_ScuttleIfFlippedTime;
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
    delete m_pRThruster;
    delete m_pLThruster;
    delete m_pURThruster;
    delete m_pULThruster;
    delete m_pRHatch;
    delete m_pLHatch;

    if (!notInherited)
        ACraft::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACDropShip, including the mass of its
//                  currently attached body parts and inventory.

float ACDropShip::GetMass() const
{
    float totalMass = ACraft::GetMass();

    if (m_pRThruster)
        totalMass += m_pRThruster->GetMass();
    if (m_pLThruster)
        totalMass += m_pLThruster->GetMass();
    if (m_pRHatch)
        totalMass += m_pRHatch->GetMass();
    if (m_pLHatch)
        totalMass += m_pLHatch->GetMass();

    return totalMass;
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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.

void ACDropShip::SetID(const MOID newID)
{
    MovableObject::SetID(newID);
    if (m_pRThruster)
        m_pRThruster->SetID(newID);
    if (m_pLThruster)
        m_pLThruster->SetID(newID);
    if (m_pRHatch)
        m_pRHatch->SetID(newID);
    if (m_pLHatch)
        m_pLHatch->SetID(newID);
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
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void ACDropShip::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    // TODO: maybe make hardcoded attachables gib if their gib list isn't empty
    // Detach all limbs and let loose
    if (m_pRThruster && m_pRThruster->IsAttached())
    {
        RemoveAttachable(m_pRThruster);
        SetAttachableVelocitiesForGibbing(m_pRThruster, impactImpulse, internalBlast);
        m_pRThruster->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pRThruster);
        m_pRThruster = 0;
    }
    if (m_pLThruster && m_pLThruster->IsAttached())
    {
        RemoveAttachable(m_pLThruster);
        SetAttachableVelocitiesForGibbing(m_pLThruster, impactImpulse, internalBlast);
        m_pLThruster->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pLThruster);
        m_pLThruster = 0;
    }
    if (m_pRHatch && m_pRHatch->IsAttached())
    {
        RemoveAttachable(m_pRHatch);
        SetAttachableVelocitiesForGibbing(m_pRHatch, impactImpulse, internalBlast);
        m_pRHatch->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pRHatch);
        m_pRHatch = 0;
    }
    if (m_pLHatch && m_pLHatch->IsAttached())
    {
        RemoveAttachable(m_pLHatch);
        SetAttachableVelocitiesForGibbing(m_pLHatch, impactImpulse, internalBlast);
        m_pLHatch->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pLHatch);
        m_pLHatch = 0;
    }
    if (m_pURThruster && m_pURThruster->IsAttached())
    {
        RemoveAttachable(m_pURThruster);
        SetAttachableVelocitiesForGibbing(m_pURThruster, impactImpulse, internalBlast);
        m_pURThruster->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pURThruster);
        m_pURThruster = 0;
    }
    if (m_pULThruster && m_pULThruster->IsAttached())
    {
        RemoveAttachable(m_pULThruster);
        SetAttachableVelocitiesForGibbing(m_pULThruster, impactImpulse, internalBlast);
        m_pULThruster->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pULThruster);
        m_pULThruster = 0;
    }

    Actor::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool ACDropShip::IsOnScenePoint(Vector &scenePoint) const
{
    return ((m_pLThruster && m_pLThruster->IsOnScenePoint(scenePoint)) ||
            (m_pRThruster && m_pRThruster->IsOnScenePoint(scenePoint)) ||
            MOSRotating::IsOnScenePoint(scenePoint) ||
            (m_pLHatch && m_pLHatch->IsOnScenePoint(scenePoint)) ||
            (m_pRHatch && m_pRHatch->IsOnScenePoint(scenePoint)));
}


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
	float mass = GetMass();

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

	/////////////////////////////////////////////////
	// Update MovableObject, adds on the forces etc, updated viewpoint

	ACraft::Update();


	/////////////////////////////////
	// Update Attachable:s

// TODO: don't hardcode the 20 deg tilt range!
	Matrix engineRot = 0;
	// Clamp engine rotation to within +-20 of body rotation
	if (m_Rotation.GetDegAngle() > m_MaxEngineAngle)
		engineRot.SetDegAngle(m_Rotation.GetDegAngle() - m_MaxEngineAngle);
	else if (m_Rotation.GetDegAngle() < -m_MaxEngineAngle)
		engineRot.SetDegAngle(m_Rotation.GetDegAngle() + m_MaxEngineAngle);
	// Lateral control application
	else
		engineRot.SetDegAngle(m_MaxEngineAngle * m_LateralControl);

    if (m_pRThruster && m_pRThruster->IsAttached())
    {
        m_pRThruster->SetRotAngle(engineRot.GetRadAngle());
        m_pRThruster->SetJointPos(m_Pos + m_pRThruster->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pRThruster->Update();

        // Update the Atoms' offsets in the parent group
        Matrix atomRot(FacingAngle(m_pRThruster->GetRotMatrix().GetRadAngle()) - FacingAngle(m_Rotation.GetRadAngle()));
        m_pAtomGroup->UpdateSubAtoms(m_pRThruster->GetAtomSubgroupID(), m_pRThruster->GetParentOffset() - (m_pRThruster->GetJointOffset() * atomRot), atomRot);

        m_Health -= m_pRThruster->CollectDamage();
    }

    if (m_pLThruster && m_pLThruster->IsAttached())
    {
        m_pLThruster->SetRotAngle(engineRot.GetRadAngle());
        m_pLThruster->SetJointPos(m_Pos + m_pLThruster->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pLThruster->Update();

        // Update the Atoms' offsets in the parent group
        Matrix atomRot(FacingAngle(m_pLThruster->GetRotMatrix().GetRadAngle()) - FacingAngle(m_Rotation.GetRadAngle()));
        m_pAtomGroup->UpdateSubAtoms(m_pLThruster->GetAtomSubgroupID(), m_pLThruster->GetParentOffset() - (m_pLThruster->GetJointOffset() * atomRot), atomRot);

        m_Health -= m_pLThruster->CollectDamage();
    }

    // Auto balancing with the up thrusters

    if (m_pURThruster && m_pURThruster->IsAttached() && m_pULThruster && m_pULThruster->IsAttached())
    {
        if (m_AutoStabilize)
        {
            // Use a PD-controller for balance
            float change = 0.9 * m_AngularVel + 0.8 * m_Rotation.GetRadAngle();
            if (change > 0.2)
            {
                if (!m_pURThruster->IsEmitting())
                    m_pURThruster->TriggerBurst();
                m_pURThruster->EnableEmission(true);
            }
            else
                m_pURThruster->EnableEmission(false);

            if (change < -0.2)
            {
                if (!m_pULThruster->IsEmitting())
                    m_pULThruster->TriggerBurst();
                m_pULThruster->EnableEmission(true);
            }
            else
                m_pULThruster->EnableEmission(false);
        }

        m_pURThruster->SetRotAngle(m_Rotation.GetRadAngle());
        m_pURThruster->SetJointPos(m_Pos + m_pURThruster->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pURThruster->Update();

        m_pULThruster->SetRotAngle(m_Rotation.GetRadAngle());
        m_pULThruster->SetJointPos(m_Pos + m_pULThruster->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pULThruster->Update();
    }

    // Hatch door pieces

    if (m_pRHatch && m_pRHatch->IsAttached())
    {
        m_pRHatch->SetRotAngle(m_Rotation.GetRadAngle() + m_HatchSwingRange.GetRadAngle() * m_HatchOpeness);
        m_pRHatch->SetJointPos(m_Pos + m_pRHatch->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pRHatch->Update();

		// Update the Atoms' offsets in the parent group
		Matrix atomRot(FacingAngle(m_pRHatch->GetRotMatrix().GetRadAngle()) - FacingAngle(m_Rotation.GetRadAngle()));
		m_pAtomGroup->UpdateSubAtoms(m_pRHatch->GetAtomSubgroupID(), m_pRHatch->GetParentOffset() - (m_pRHatch->GetJointOffset() * atomRot), atomRot);

        m_Health -= m_pRHatch->CollectDamage();
    }

    if (m_pLHatch && m_pLHatch->IsAttached())
    {
        m_pLHatch->SetRotAngle(m_Rotation.GetRadAngle() - m_HatchSwingRange.GetRadAngle() * m_HatchOpeness);
        m_pLHatch->SetJointPos(m_Pos + m_pLHatch->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pLHatch->Update();

		// Update the Atoms' offsets in the parent group
		Matrix atomRot(FacingAngle(m_pLHatch->GetRotMatrix().GetRadAngle()) - FacingAngle(m_Rotation.GetRadAngle()));
		m_pAtomGroup->UpdateSubAtoms(m_pLHatch->GetAtomSubgroupID(), m_pLHatch->GetParentOffset() - (m_pLHatch->GetJointOffset() * atomRot), atomRot);

        m_Health -= m_pLHatch->CollectDamage();
    }

    ///////////////////////////////////////////////////////////
    // Apply forces transferred from the attachables and
    // add detachment wounds to this if applicable

    if (!ApplyAttachableForces(m_pRHatch))
        m_pRHatch = 0;
    if (!ApplyAttachableForces(m_pLHatch))
        m_pLHatch = 0;
    if (!ApplyAttachableForces(m_pRThruster))
        m_pRThruster = 0;
    if (!ApplyAttachableForces(m_pLThruster))
        m_pLThruster = 0;
    if (!ApplyAttachableForces(m_pURThruster))
        m_pURThruster = 0;
    if (!ApplyAttachableForces(m_pULThruster))
        m_pULThruster = 0;

// TODO: add hatch damage here

    ///////////////////////////////////
    // Explosion logic

    if (m_Status == DEAD)
        GibThis(Vector(), 50);

    ////////////////////////////////////////
    // Balance stuff

    // Get the rotation in radians.
    float rot = m_Rotation.GetRadAngle();
/* If the dropship starts rolling, it's over man
    // Eliminate full rotations
    while (fabs(rot) > c_TwoPI)
        rot -= rot > 0 ? c_TwoPI : -c_TwoPI;

    // Eliminate rotations over half a turn
    if (fabs(rot) > c_PI)
        rot = (rot > 0 ? -c_PI : c_PI) + (rot - (rot > 0 ? c_PI : -c_PI));
*/
    // If tipped too far for too long, die
    if (rot < c_HalfPI && rot > -c_HalfPI)
    {
        m_FlippedTimer.Reset();
    }
    // Start death process if tipped over for too long
    else if (m_ScuttleIfFlippedTime >= 0 && m_FlippedTimer.IsPastSimMS(m_ScuttleIfFlippedTime) && m_Status != DYING) // defult is 4s
    {
        m_Status = DYING;
        m_DeathTmr.Reset();
    }

    // Flash if dying, warning of impending explosion
    if (m_Status == DYING)
    {
        if (m_DeathTmr.IsPastSimMS(500) && m_DeathTmr.AlternateSim(100))
            FlashWhite(10);
    }

/*
//        rot = fabs(rot) < c_QuarterPI ? rot : (rot > 0 ? c_QuarterPI : -c_QuarterPI);

    // Rotational balancing spring calc
    if (m_Status == STABLE) {
        // Break the spring if close to target angle.
        if (fabs(rot) > 0.1)
            m_AngularVel -= rot * fabs(rot);
        else if (fabs(m_AngularVel) > 0.1)
            m_AngularVel *= 0.5;
    }
    // Unstable, or without balance
    else if (m_Status == DYING) {
//        float rotTarget = rot > 0 ? c_HalfPI : -c_HalfPI;
        float rotTarget = c_HalfPI;
        float rotDiff = rotTarget - rot;
        if (fabs(rotDiff) > 0.1)
            m_AngularVel += rotDiff * rotDiff;
        else
            m_Status = DEAD;

//        else if (fabs(m_AngularVel) > 0.1)
//            m_AngularVel *= 0.5;
    }
    m_Rotation.SetRadAngle(rot);
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetEmissionTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reset the timers of all emissions so they will start/stop at the 
//                  correct relative offsets from now.

void ACDropShip::ResetEmissionTimers()
{
    if (m_pRThruster && m_pRThruster->IsAttached())
        m_pRThruster->ResetEmissionTimers();

    if (m_pLThruster && m_pLThruster->IsAttached())
        m_pLThruster->ResetEmissionTimers();

    if (m_pURThruster && m_pURThruster->IsAttached())
        m_pURThruster->ResetEmissionTimers();

    if (m_pULThruster && m_pULThruster->IsAttached())
        m_pULThruster->ResetEmissionTimers();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAnyRandomWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds from the actor and all standard attachables.

int ACDropShip::RemoveAnyRandomWounds(int amount)
{
	float damage = 0;

	for (int i = 0; i < amount; i++)
	{
		// Fill the list of damaged bodyparts
		std::vector<MOSRotating *> bodyParts;
		if (GetWoundCount() > 0)
			bodyParts.push_back(this);

		if (m_pRHatch && m_pRHatch->GetWoundCount())
			bodyParts.push_back(m_pRHatch);
		if (m_pLHatch && m_pLHatch->GetWoundCount())
			bodyParts.push_back(m_pLHatch);
		if (m_pRThruster && m_pRThruster->GetWoundCount())
			bodyParts.push_back(m_pRThruster);
		if (m_pLThruster && m_pLThruster->GetWoundCount())
			bodyParts.push_back(m_pLThruster);

		// Stop removing wounds if there are not any left
		if (bodyParts.size() == 0)
			break;

		int partIndex = RandomNum<int>(0, bodyParts.size() - 1);
		MOSRotating * part = bodyParts[partIndex];
		damage += part->RemoveWounds(1);
	}

	return damage;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTotalWoundCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns total wound count of this actor and all vital attachables.

int ACDropShip::GetTotalWoundCount() const
{
	int count = ACraft::GetWoundCount();

    if (m_pRHatch)
        count += m_pRHatch->GetWoundCount();
    if (m_pLHatch)
        count += m_pLHatch->GetWoundCount();
    if (m_pRThruster)
        count += m_pRThruster->GetWoundCount();
    if (m_pLThruster)
        count += m_pLThruster->GetWoundCount();

	return count;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTotalWoundLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns total wound limit of this actor and all vital attachables.

int ACDropShip::GetTotalWoundLimit() const
{ 
	int count = ACraft::GetGibWoundLimit();

    if (m_pRHatch)
        count += m_pRHatch->GetGibWoundLimit();
    if (m_pLHatch)
        count += m_pLHatch->GetGibWoundLimit();
    if (m_pRThruster)
        count += m_pRThruster->GetGibWoundLimit();
    if (m_pLThruster)
        count += m_pLThruster->GetGibWoundLimit();

	return count;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void ACDropShip::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                                 MOID rootMOID,
                                 bool makeNewMOID)
{
    if (m_pRHatch)
        m_pRHatch->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);
    if (m_pLHatch)
        m_pLHatch->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);
    if (m_pRThruster)
        m_pRThruster->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);
    if (m_pLThruster)
        m_pLThruster->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);

    ACraft::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void ACDropShip::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	if (m_pRHatch)
		m_pRHatch->GetMOIDs(MOIDs);
	if (m_pLHatch)
		m_pLHatch->GetMOIDs(MOIDs);
	if (m_pRThruster)
		m_pRThruster->GetMOIDs(MOIDs);
	if (m_pLThruster)
		m_pLThruster->GetMOIDs(MOIDs);

	ACraft::GetMOIDs(MOIDs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ACDropShip's current graphical representation to a
//                  BITMAP of choice.

void ACDropShip::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    if (m_pRHatch)
        m_pRHatch->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    if (m_pLHatch)
        m_pLHatch->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pRThruster && !m_pRThruster->IsDrawnAfterParent())
        m_pRThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    if (m_pLThruster && !m_pLThruster->IsDrawnAfterParent())
        m_pLThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    ACraft::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pRThruster && m_pRThruster->IsDrawnAfterParent())
        m_pRThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    if (m_pLThruster && m_pLThruster->IsDrawnAfterParent())
        m_pLThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (mode == g_DrawColor || mode == g_DrawMaterial)
    {
        if (m_pURThruster)
            m_pURThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pULThruster)
            m_pULThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }

    if (mode == g_DrawColor)
    {
#ifdef DEBUG_BUILD
        acquire_bitmap(pTargetBitmap);
        putpixel(pTargetBitmap, floorf(m_Pos.m_X),
                              floorf(m_Pos.m_Y),
                              64);
        putpixel(pTargetBitmap, floorf(m_Pos.m_X),
                              floorf(m_Pos.m_Y),
                              64);
        release_bitmap(pTargetBitmap);

        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false, 122);
//        m_pDeepGroup->Draw(pTargetBitmap, targetPos, false, 13);
#endif
    }
}

} // namespace RTE