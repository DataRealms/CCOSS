//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACRocket.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ACRocket class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files


#include "ACRocket.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "Leg.h"
#include "Controller.h"
#include "Matrix.h"
#include "AEmitter.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(ACRocket, ACraft, 10)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACRocket, effectively
//                  resetting the members of this abstraction level only.

void ACRocket::Clear()
{
//    m_pCapsule = 0;
    m_pRLeg = 0;
    m_pLLeg = 0;
    m_pBodyAG = 0;
    m_pRFootGroup = 0;
    m_pLFootGroup = 0;
    m_pMThruster = 0;
    m_pRThruster = 0;
    m_pLThruster = 0;
    m_pURThruster = 0;
    m_pULThruster = 0;
    m_GearState = RAISED;
    m_ScuttleIfFlippedTime = 4000;
    for (int i = 0; i < GearStateCount; ++i) {
        m_Paths[RIGHT][i].Reset();
        m_Paths[LEFT][i].Reset();
        m_Paths[RIGHT][i].Terminate();
        m_Paths[LEFT][i].Terminate();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACRocket object ready for use.

int ACRocket::Create()
{
    // Read all the properties
    if (ACraft::Create() < 0)
        return -1;

    // Save the AtomGroup read in by MOSRotating, as we are going to make it
    // into a composite group, and want to have the base body stored for reference.
    m_pBodyAG = dynamic_cast<AtomGroup *>(m_pAtomGroup->Clone());

    // Mirror the limb paths
    for (int i = 0; i < GearStateCount; ++i)
    {
        m_Paths[LEFT][i].Create(m_Paths[RIGHT][i]);
    }

    // Override the body animation mode
    m_SpriteAnimMode = PINGPONGOPENCLOSE;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACRocket to be identical to another, by deep copy.

int ACRocket::Create(const ACRocket &reference)
{
    ACraft::Create(reference);

    if (reference.m_pRLeg) {
        m_pRLeg = dynamic_cast<Leg *>(reference.m_pRLeg->Clone());
        AddAttachable(m_pRLeg, true);
    }

    if (reference.m_pLLeg) {
        m_pLLeg = dynamic_cast<Leg *>(reference.m_pLLeg->Clone());
        AddAttachable(m_pLLeg, true);
    }

    m_pBodyAG = dynamic_cast<AtomGroup *>(reference.m_pBodyAG->Clone());
    m_pBodyAG->SetOwner(this);

    if (reference.m_pRFootGroup)
    {
        m_pRFootGroup = dynamic_cast<AtomGroup *>(reference.m_pRFootGroup->Clone());
        m_pRFootGroup->SetOwner(this);
    }
    if (reference.m_pLFootGroup)
    {
        m_pLFootGroup = dynamic_cast<AtomGroup *>(reference.m_pLFootGroup->Clone());
        m_pLFootGroup->SetOwner(this);
    }

    if (reference.m_pMThruster)
    {
        m_pMThruster = dynamic_cast<AEmitter *>(reference.m_pMThruster->Clone());
        AddAttachable(m_pMThruster, true);
    }
    if (reference.m_pRThruster)
    {
        m_pRThruster = dynamic_cast<AEmitter *>(reference.m_pRThruster->Clone());
        AddAttachable(m_pRThruster, true);
    }
    if (reference.m_pLThruster)
    {
        m_pLThruster = dynamic_cast<AEmitter *>(reference.m_pLThruster->Clone());
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

    m_GearState = reference.m_GearState;

    for (int i = 0; i < GearStateCount; ++i) {
        m_Paths[RIGHT][i].Create(reference.m_Paths[RIGHT][i]);
        m_Paths[LEFT][i].Create(reference.m_Paths[LEFT][i]);
    }

    m_ScuttleIfFlippedTime = reference.m_ScuttleIfFlippedTime;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ACRocket::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "RLeg")
    {
        delete m_pRLeg;
        m_pRLeg = new Leg;
        reader >> m_pRLeg;
    }
    else if (propName == "LLeg")
    {
        delete m_pLLeg;
        m_pLLeg = new Leg;
        reader >> m_pLLeg;
        m_pLLeg->SetHFlipped(true);
    }
    else if (propName == "RFootGroup")
    {
        delete m_pRFootGroup;
        m_pRFootGroup = new AtomGroup();
        reader >> m_pRFootGroup;
        m_pRFootGroup->SetOwner(this);
    }
    else if (propName == "LFootGroup")
    {
        delete m_pLFootGroup;
        m_pLFootGroup = new AtomGroup();
        reader >> m_pLFootGroup;
        m_pLFootGroup->SetOwner(this);
    }
    else if (propName == "MThruster")
    {   
        delete m_pMThruster;
        m_pMThruster = new AEmitter;
        reader >> m_pMThruster;
    }
    else if (propName == "RThruster")
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
    else if (propName == "RaisedGearLimbPath")
        reader >> m_Paths[RIGHT][RAISED];
    else if (propName == "LoweredGearLimbPath")
        reader >> m_Paths[RIGHT][LOWERED];
    else if (propName == "LoweringGearLimbPath")
        reader >> m_Paths[RIGHT][LOWERING];
    else if (propName == "RaisingGearLimbPath")
        reader >> m_Paths[RIGHT][RAISING];
    else if (propName == "ScuttleIfFlippedTime")
        reader >> m_ScuttleIfFlippedTime;
    else
        return ACraft::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ACRocket with a Writer for
//                  later recreation with Create(Reader &reader);

int ACRocket::Save(Writer &writer) const
{
    ACraft::Save(writer);

    writer.NewProperty("RLeg");
    writer << m_pRLeg;
    writer.NewProperty("LLeg");
    writer << m_pLLeg;
    writer.NewProperty("RFootGroup");
    writer << m_pRFootGroup;
    writer.NewProperty("LFootGroup");
    writer << m_pLFootGroup;
    writer.NewProperty("MThruster");
    writer << m_pMThruster;
    writer.NewProperty("RThruster");
    writer << m_pRThruster;
    writer.NewProperty("LThruster");
    writer << m_pLThruster;
    writer.NewProperty("URThruster");
    writer << m_pURThruster;
    writer.NewProperty("ULThruster");
    writer << m_pULThruster;
    writer.NewProperty("RaisedGearLimbPath");
    writer << m_Paths[RIGHT][RAISED];
    writer.NewProperty("LoweredGearLimbPath");
    writer << m_Paths[RIGHT][LOWERED];
    writer.NewProperty("LoweringGearLimbPath");
    writer << m_Paths[RIGHT][LOWERING];
    writer.NewProperty("RaisingGearLimbPath");
    writer << m_Paths[RIGHT][RAISING];
    writer.NewProperty("ScuttleIfFlippedTime");
    writer << m_ScuttleIfFlippedTime;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ACRocket object.

void ACRocket::Destroy(bool notInherited)
{
    delete m_pRLeg;
    delete m_pLLeg;
    delete m_pBodyAG;
    delete m_pRFootGroup;
    delete m_pLFootGroup;

    delete m_pMThruster;
    delete m_pRThruster;
    delete m_pLThruster;
    delete m_pURThruster;
    delete m_pULThruster;
    
//    for (deque<LimbPath *>::iterator itr = m_WalkPaths.begin();
//         itr != m_WalkPaths.end(); ++itr)
//        delete *itr;

    if (!notInherited)
        ACraft::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACRocket, including the mass of its
//                  currently attached body parts and inventory.

float ACRocket::GetMass() const
{
    float totalMass = ACraft::GetMass();
//    if (m_pCapsule)
//        totalMass += m_pCapsule->GetMass();
    if (m_pRLeg)
        totalMass += m_pRLeg->GetMass();
    if (m_pLLeg)
        totalMass += m_pLLeg->GetMass();
    if (m_pMThruster)
        totalMass += m_pMThruster->GetMass();
    return totalMass;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.

float ACRocket::GetAltitude(int max, int accuracy)
{
    // Use the main thruster's position as the position ot measure from
    Vector pos;
    if (m_pMThruster && m_pMThruster->IsAttached())
        pos = m_Pos + RotateOffset(m_pMThruster->GetParentOffset());
    else
        pos = m_Pos;

    return g_SceneMan.FindAltitude(pos, max, accuracy);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.

void ACRocket::SetID(const MOID newID)
{
    MovableObject::SetID(newID);
//    if (m_pCapsule)
//        m_pCapsule->SetID(newID);
    if (m_pRLeg)
        m_pRLeg->SetID(newID);
    if (m_pLLeg)
        m_pLLeg->SetID(newID);
    if (m_pMThruster)
        m_pMThruster->SetID(newID);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool ACRocket::OnBounce(const Vector &pos)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool ACRocket::OnSink(const Vector &pos)
{
    return false;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void ACRocket::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    // TODO: maybe make hardcoded attachables gib if their gib list isn't empty
    // Detach all limbs and let loose
    if (m_pRLeg && m_pRLeg->IsAttached())
    {
        RemoveAttachable(m_pRLeg);
        SetAttachableVelocitiesForGibbing(m_pRLeg, impactImpulse, internalBlast);
        m_pRLeg->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pRLeg);
        m_pRLeg = 0;
    }
    if (m_pLLeg && m_pLLeg->IsAttached())
    {
        RemoveAttachable(m_pLLeg);
        SetAttachableVelocitiesForGibbing(m_pLLeg, impactImpulse, internalBlast);
        m_pLLeg->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pLLeg);
        m_pLLeg = 0;
    }
    if (m_pMThruster && m_pMThruster->IsAttached())
    {
        RemoveAttachable(m_pMThruster);
        SetAttachableVelocitiesForGibbing(m_pMThruster, impactImpulse, internalBlast);
        m_pMThruster->SetToGetHitByMOs(false);
        g_MovableMan.AddParticle(m_pMThruster);
        m_pMThruster = 0;
    }
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
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.

void ACRocket::UpdateAI()
{
    float velMag = m_Vel.GetMagnitude();
    float angle = m_Rotation.GetRadAngle();

    // This is the limit where increased thrust should be applied to assure a soft landing
    float landLimit = m_CharHeight * 2;
    // This is the lower limit on when thrust should be applied - anyhting higher, blast away!
    float thrustLimit = m_CharHeight / 4;
    // Get the altitude reading, within 10 pixels precision
    float altitude = GetAltitude(g_SceneMan.GetSceneHeight() / 2, 10);

    // Stuck detection
    if (velMag > 1.0)
        m_StuckTimer.Reset();

    /////////////////////////////
    // AI Modes affect the delivery state

    if (m_AIMode == AIMODE_DELIVER)
    {
        
    }

    ////////////////////////////
    // Delivery Sequence logic

    if (m_DeliveryState == FALL)
    {
        m_AltitudeControl = 0.95;
        if (altitude < landLimit || m_StuckTimer.IsPastSimMS(3000))
            m_DeliveryState = LAND;
    }
    else if (m_DeliveryState == LAND)
    {
        m_AltitudeControl = 0.3;
        // Unload only if there's something to unload, and we're on the ground, or deem controlled is stuck
        if (!IsInventoryEmpty() && (altitude < thrustLimit || m_StuckTimer.IsPastSimMS(3000)) && m_AIMode != AIMODE_STAY)
        {
            DropAllInventory();
            m_FallTimer.Reset();
            m_DeliveryState = UNLOAD;
        }

        // Don't try to land if we have already delivered
        if (IsInventoryEmpty() && m_HasDelivered && m_AIMode != AIMODE_STAY)
        {
            m_StuckTimer.Reset();
            m_DeliveryState = LAUNCH;
        }
    }
    else if (m_DeliveryState == UNLOAD)
    {
        m_AltitudeControl = 0;
        // Add delay after last unload before taking off
        if (m_FallTimer.IsPastSimMS(1000) && IsInventoryEmpty())
        {
            m_StuckTimer.Reset();
            m_DeliveryState = LAUNCH;
        }
        // Reset only the timer if we're not empty yet
        else if (!IsInventoryEmpty())
            m_FallTimer.Reset();
    }
    else if (m_DeliveryState == LAUNCH)
    {
        m_ObstacleState = PROCEEDING;
        m_AltitudeControl = -1.0;

        if (m_StuckTimer.IsPastSimMS(3000))
        {
            m_StuckTimer.Reset();
            m_DeliveryState = UNSTICK;
        }
    }
    else if (m_DeliveryState == UNSTICK)
    {
        m_ObstacleState = BACKSTEPPING;

        if (m_StuckTimer.IsPastSimMS(1500))
        {
            m_StuckTimer.Reset();
            m_DeliveryState = LAUNCH;
        }
    }

    // Adjustement of turn on/off thresholds based on the altitude control
// TODO: DOn't hardcode adjustments
    float altVelSpread = 8.0 / 2;
    float startVel = 1 + 10 * m_AltitudeControl + altVelSpread;
    float stopVel = 1 + 10 * m_AltitudeControl - altVelSpread;

    /////////////////////////
    // LIFT

    // Don't do anyhting else if using the retrorockets to get out of stuckness
    if (m_ObstacleState == BACKSTEPPING)
        m_ObstacleState = BACKSTEPPING;
    // Don't when very close to the ground, unless taking off, or if really rotated
    else if (m_DeliveryState != LAUNCH && altitude < thrustLimit || m_AltitudeControl >= 1.0/* || fabs(angle) > c_SixteenthPI */)
        m_ObstacleState = PROCEEDING;
    // Always fire if alt control is maxed out
    else if (m_AltitudeControl <= -1.0)
        m_ObstacleState = SOFTLANDING;
    else
    {
        // START - We're falling, so see if it's time to start firing the main thruster to hover
        if (m_Vel.m_Y > startVel)
        {
            m_ObstacleState = SOFTLANDING;
        }
        // STOP main lift thruster
        else if (m_Vel.m_Y < stopVel)
        {
            m_ObstacleState = PROCEEDING;
        }
    }

    /////////////////////////
    // STABILIZATION

    // Don't mess if we're unloading or automatically stabilizing
    if (AutoStabilizing() || (m_DeliveryState == UNLOAD && velMag < 5.0))
        m_LateralMoveState = LAT_STILL;
    else
    {
        if (angle > c_SixteenthPI / 2)
        {
            m_LateralMoveState = LAT_LEFT;
        }
        else if (angle < -c_SixteenthPI / 2)
        {
            m_LateralMoveState = LAT_RIGHT;
        }
        else
            m_LateralMoveState = LAT_STILL;
    }

    /////////////////////////
    // UNLODGING
/*
    if (m_DeliveryState != UNLOAD && m_StuckTimer.IsPastSimMS(1500))
    {

    }
    if ()
*/
    /////////////////////////
    // INPUT TRANSLATION

    if (m_ObstacleState == SOFTLANDING)
    {
        m_Controller.SetState(BODY_JUMP, true);
        m_Controller.SetState(MOVE_UP, true);
    }
    else if (m_ObstacleState == BACKSTEPPING)
        m_Controller.SetState(MOVE_DOWN, true);

    if (m_LateralMoveState == LAT_LEFT)
        m_Controller.SetState(MOVE_RIGHT, true);
    else if (m_LateralMoveState == LAT_RIGHT)
        m_Controller.SetState(MOVE_LEFT, true);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ACRocket. Supposed to be done every frame.

void ACRocket::Update()
{
    float mass = GetMass();
    float deltaTime = g_TimerMan.GetDeltaTimeSecs();

    // Look/aim update, make the scanner point aftward if the rocket is falling
    m_AimAngle = m_Vel.m_Y < 0 ? c_HalfPI : -c_HalfPI;

    /////////////////////////////////
    // Controller update and handling

// TODO: Improve and make optional thrusters more robust!
    // Make sure we have all thrusters
    if (m_pMThruster && m_pRThruster && m_pLThruster && m_pURThruster && m_pULThruster)
    {
        if (m_Status != DEAD && m_Status != DYING)
        {
            // Fire main thrusters
            if (m_Controller.IsState(MOVE_UP) || m_Controller.IsState(AIM_UP))
            {
                if (!m_pMThruster->IsEmitting())
                {
                    m_pMThruster->TriggerBurst();
                    // This is to make sure se get loose from being sideways stuck
                    m_ForceDeepCheck = true;
                }
                m_pMThruster->EnableEmission(true);
                // Engines are noisy!
                m_pMThruster->AlarmOnEmit(m_Team);

                if (m_HatchState == OPEN) {
                    CloseHatch();
                    m_HatchTimer.Reset();
                }
            }
            else
            {
                m_pMThruster->EnableEmission(false);

                // Fire reverse thrusters
                if (m_Controller.IsState(MOVE_DOWN) || m_Controller.IsState(AIM_DOWN))
                {
                    if (!m_pURThruster->IsEmitting())
                        m_pURThruster->TriggerBurst();
                    if (!m_pULThruster->IsEmitting())
                        m_pULThruster->TriggerBurst();
                    m_pURThruster->EnableEmission(true);
                    m_pULThruster->EnableEmission(true);
                }
                else
                {
                    m_pURThruster->EnableEmission(false);
                    m_pULThruster->EnableEmission(false);
                }
            }

            // Fire left thrusters
            if (m_Controller.IsState(MOVE_RIGHT)/* || m_Rotation > 0.1*/)
            {
                if (!m_pLThruster->IsEmitting())
                    m_pLThruster->TriggerBurst();
                m_pLThruster->EnableEmission(true);
            }
            else
                m_pLThruster->EnableEmission(false);

            // Fire right thrusters
            if (m_Controller.IsState(MOVE_LEFT)/* || m_Rotation < 0.1*/)
            {
                if (!m_pRThruster->IsEmitting())
                    m_pRThruster->TriggerBurst();
                m_pRThruster->EnableEmission(true);
            }
            else
                m_pRThruster->EnableEmission(false);
            /*
            if (m_Controller.IsState(PRESS_FACEBUTTON))
            {
                if (m_GearState == RAISED)
                    m_GearState = LOWERING;
                else if (m_GearState == LOWERED)
                    m_GearState = RAISING;
            }
            */
            if (m_Controller.IsState(PRESS_FACEBUTTON))
            {
                if (m_HatchState == CLOSED)
                    DropAllInventory();
                else if (m_HatchState == OPEN)
                    CloseHatch();
            }
        }
        else
        {
            m_pMThruster->EnableEmission(false);
            m_pRThruster->EnableEmission(false);
            m_pLThruster->EnableEmission(false);
            m_pURThruster->EnableEmission(false);
            m_pULThruster->EnableEmission(false);
        }

/*
        else if (m_Controller && m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT))
        {
            if (m_MoveState != WALK && m_MoveState != CROUCH)
                m_StrideStart = true;

            if (m_MoveState == BODY_JUMPSTART || m_MoveState == BODY_JUMPSTART)
                m_MoveState = CROUCH;
            else
                m_MoveState = WALK;

            if (m_Controller.IsState(MOVE_FAST))
            {
                m_Paths[FGROUND][WALK].SetSpeed(FAST);
                m_Paths[BGROUND][WALK].SetSpeed(FAST);
            }
            else
            {
                m_Paths[FGROUND][WALK].SetSpeed(NORMAL);
                m_Paths[BGROUND][WALK].SetSpeed(NORMAL);
            }

            if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped))
            {
                m_HFlipped = !m_HFlipped;
                m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
                m_Paths[FGROUND][CLIMB].Terminate();
                m_Paths[BGROUND][CLIMB].Terminate();
                m_Paths[FGROUND][STAND].Terminate();
                m_Paths[BGROUND][STAND].Terminate();
                m_StrideStart = true;
            }
        }
        else
            m_MoveState = STAND;

        if (m_Controller.IsState(WEAPON_CHANGE_NEXT))
        {
            if (m_pFGArm && m_pFGArm->IsAttached())
            {
                m_pFGArm->SetDevice(SwapNextDevice(m_pFGArm->ReleaseDevice()));
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
            }
        }
*/
        // No Controller present
        if (m_Controller.IsDisabled())
        {
            m_pMThruster->EnableEmission(false);
            m_pRThruster->EnableEmission(false);
            m_pLThruster->EnableEmission(false);
            m_pURThruster->EnableEmission(false);
            m_pULThruster->EnableEmission(false);
        }
    }
//    m_aSprite->SetAngle((m_AimAngle / 180) * 3.141592654);
//    m_aSprite->SetScale(2.0);


    ///////////////////////////////////////////////////
    // Travel the landing gear AtomGroup:s


    // RAISE the gears
    if (m_pMThruster && m_pMThruster->IsEmitting())// && m_pMThruster->IsSetToBurst())
    {
        m_Paths[RIGHT][RAISED].SetHFlip(m_HFlipped);
        m_Paths[LEFT][RAISED].SetHFlip(!m_HFlipped);

        if (m_pRLeg)
            m_pRFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pRLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[RIGHT][RAISED],
                                      deltaTime,
                                      0,
                                      true);
        if (m_pLLeg)
            m_pLFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pLLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[LEFT][RAISED],
                                      deltaTime,
                                      0,
                                      true);

        m_GearState = RAISED;
    }
    // LOWER the gears
    else if (m_pMThruster && !m_pMThruster->IsEmitting())// && m_GearState != LOWERED)
    {
        m_Paths[RIGHT][LOWERED].SetHFlip(m_HFlipped);
        m_Paths[LEFT][LOWERED].SetHFlip(!m_HFlipped);

        if (m_pRLeg)
            m_pRFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pRLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[RIGHT][LOWERED],
                                      deltaTime,
                                      0,
                                      true);
        if (m_pLLeg)
            m_pLFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pLLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[LEFT][LOWERED],
                                      deltaTime,
                                      0,
                                      true);
        m_GearState = LOWERED;
    }

    /////////////////////////////////////////////////
    // Update MovableObject, adds on the forces etc, updated viewpoint

    ACraft::Update();


    /////////////////////////////////
    // Update Attachable:s
/*
    if (m_pCapsule && m_pCapsule->IsAttached())
    {
        m_pCapsule->SetJointPos(m_Pos + m_pCapsule->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
        m_pCapsule->SetRotAngle(m_Rotation);
        m_pCapsule->Update();
        m_Health -= m_pCapsule->CollectDamage();
    }
*/
    if (m_pRLeg && m_pRLeg->IsAttached())
    {
        m_pRLeg->SetJointPos(m_Pos + m_pRLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
/* Obsolete
        if (!m_pMThruster->IsEmitting())
            m_pRLeg->ReachToward(m_Pos + Vector(18, 40) * m_Rotation);
        else
            m_pRLeg->ReachToward(m_Pos + Vector(13, 40) * m_Rotation);
*/
        m_pRLeg->ReachToward(m_pRFootGroup->GetLimbPos(m_HFlipped));
        m_pRLeg->Update();
        m_Health -= m_pRLeg->CollectDamage();
    }

    if (m_pLLeg && m_pLLeg->IsAttached())
    {
        m_pLLeg->SetJointPos(m_Pos + m_pLLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation);
/* Obsolete
        if (!m_pMThruster->IsEmitting())
            m_pLLeg->ReachToward(m_Pos + Vector(-18, 40) * m_Rotation);
        else
            m_pLLeg->ReachToward(m_Pos + Vector(-13, 40) * m_Rotation);
*/
        m_pLLeg->ReachToward(m_pLFootGroup->GetLimbPos(!m_HFlipped));
        m_pLLeg->Update();
        m_Health -= m_pLLeg->CollectDamage();
    }

    // Apply forces transferred from the attachables and
    // add detachment wounds to this if applicable

//    if (!ApplyAttachableForces(m_pCapsule))
//        m_pCapsule = 0;
    if (!ApplyAttachableForces(m_pRLeg))
        m_pRLeg = 0;
    if (!ApplyAttachableForces(m_pLLeg))
        m_pLLeg = 0;

    if (m_pMThruster) {
        m_pMThruster->SetJointPos(m_Pos + RotateOffset(m_pMThruster->GetParentOffset()));
//        m_pMThruster->SetVel(m_Vel);
        m_pMThruster->SetRotAngle(m_Rotation.GetRadAngle() - c_HalfPI);
//        m_pMThruster->SetEmitAngle(m_Rotation - c_HalfPI);
        m_pMThruster->Update();
    }

    if (m_pRThruster) {
        m_pRThruster->SetJointPos(m_Pos + RotateOffset(m_pRThruster->GetParentOffset()));
        m_pRThruster->SetVel(m_Vel);
        m_pRThruster->SetRotAngle(m_Rotation.GetRadAngle() + c_EighthPI);
//        m_pRThruster->SetEmitAngle(m_Rotation);
        m_pRThruster->Update();
    }

    if (m_pLThruster) {
        m_pLThruster->SetJointPos(m_Pos + RotateOffset(m_pLThruster->GetParentOffset()));
        m_pLThruster->SetVel(m_Vel);
        m_pLThruster->SetRotAngle(m_Rotation.GetRadAngle() + c_PI - c_EighthPI);
//        m_pLThruster->SetEmitAngle(m_Rotation + c_PI);
        m_pLThruster->Update();
    }

    if (m_pURThruster) {
        m_pURThruster->SetJointPos(m_Pos + RotateOffset(m_pURThruster->GetParentOffset()));
        m_pURThruster->SetVel(m_Vel);
        m_pURThruster->SetRotAngle(m_Rotation.GetRadAngle() + c_HalfPI - c_QuarterPI / 2);
//        m_pURThruster->SetEmitAngle(-c_QuarterPI / 2);
        m_pURThruster->Update();
    }

    if (m_pULThruster) {
        m_pULThruster->SetJointPos(m_Pos + RotateOffset(m_pULThruster->GetParentOffset()));
        m_pULThruster->SetVel(m_Vel);
        m_pULThruster->SetRotAngle(m_Rotation.GetRadAngle() + c_HalfPI + c_QuarterPI / 2);
//        m_pULThruster->SetEmitAngle(c_QuarterPI / 2);
        m_pULThruster->Update();
    }

    ///////////////////////////////////
    // Explosion logic

    if (m_Status == DEAD)
        GibThis(Vector(), 50);

    ////////////////////////////////////////
    // Balance stuff

    // Get the rotation in radians.
    float rot = m_Rotation.GetRadAngle();

    // Eliminate full rotations
    while (fabs(rot) > c_TwoPI)
        rot -= rot > 0 ? c_TwoPI : -c_TwoPI;

    // Eliminate rotations over half a turn
    if (fabs(rot) > c_PI)
        rot = (rot > 0 ? -c_PI : c_PI) + (rot - (rot > 0 ? c_PI : -c_PI));

    // If tipped too far for too long, die
    if (rot < c_HalfPI && rot > -c_HalfPI)
	{
        m_FlippedTimer.Reset();
	}
    // Start death process if tipped over for too long
    else if (m_ScuttleIfFlippedTime >= 0 && m_FlippedTimer.IsPastSimMS(m_ScuttleIfFlippedTime) && m_Status != DYING)
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

    ////////////////////////////////////////
    // Hatch Operation

    unsigned int lastFrame = m_FrameCount - 1;

    if (m_HatchState == OPENING) {
        if (m_HatchTimer.GetElapsedSimTimeMS() <= m_HatchDelay && m_HatchDelay)
            m_Frame = static_cast<unsigned int>(static_cast<double>(lastFrame) * (m_HatchTimer.GetElapsedSimTimeMS() / static_cast<double>(m_HatchDelay)));
        else
        {
            m_Frame = lastFrame;
            m_HatchState = OPEN;
            DropAllInventory();
        }
    }
    else if (m_HatchState == CLOSING) {
        if (m_HatchTimer.GetElapsedSimTimeMS() <= m_HatchDelay && m_HatchDelay)

            m_Frame = lastFrame - static_cast<unsigned int>(static_cast<double>(lastFrame) * (m_HatchTimer.GetElapsedSimTimeMS() / static_cast<double>(m_HatchDelay)));
        else
        {
            m_Frame = 0;
            m_HatchState = CLOSED;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void ACRocket::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                                MOID rootMOID,
                                bool makeNewMOID)
{
    if (m_pRLeg)
        m_pRLeg->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);
    if (m_pLLeg)
        m_pLLeg->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);

    ACraft::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void ACRocket::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	if (m_pRLeg)
		m_pRLeg->GetMOIDs(MOIDs);
	if (m_pLLeg)
		m_pLLeg->GetMOIDs(MOIDs);

	ACraft::GetMOIDs(MOIDs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetEmissionTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reset the timers of all emissions so they will start/stop at the 
//                  correct relative offsets from now.

void ACRocket::ResetEmissionTimers()
{
    if (m_pMThruster && m_pMThruster->IsAttached())
        m_pMThruster->ResetEmissionTimers();

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

int ACRocket::RemoveAnyRandomWounds(int amount)
{
	float damage = 0;

	for (int i = 0; i < amount; i++)
	{
		// Fill the list of damaged bodyparts
		std::vector<MOSRotating *> bodyParts;
		if (GetWoundCount() > 0)
			bodyParts.push_back(this);

		if (m_pRLeg && m_pRLeg->GetWoundCount())
			bodyParts.push_back(m_pRLeg);
		if (m_pLLeg && m_pLLeg->GetWoundCount())
			bodyParts.push_back(m_pLLeg);

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

int ACRocket::GetTotalWoundCount() const
{
	int count = ACraft::GetWoundCount();

//    if (m_pMThruster)
//        count += m_pMThruster->GetWoundCount();
    if (m_pRLeg)
        count += m_pRLeg->GetWoundCount();
    if (m_pLLeg)
        count += m_pLLeg->GetWoundCount();
//    if (m_pCapsule)
//        count += m_pCapsule->GetWoundCount();

	return count;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTotalWoundLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns total wound limit of this actor and all vital attachables.

int ACRocket::GetTotalWoundLimit() const
{ 
	int count = ACraft::GetGibWoundLimit();

//    if (m_pMThruster)
//        count += m_pMThruster->GetGibWoundLimit();
    if (m_pRLeg)
        count += m_pRLeg->GetGibWoundLimit();
    if (m_pLLeg)
        count += m_pLLeg->GetGibWoundLimit();
//    if (m_pCapsule)
//        count += m_pCapsule->GetGibWoundLimit();

	return count;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ACRocket's current graphical representation to a
//                  BITMAP of choice.

void ACRocket::Draw(BITMAP *pTargetBitmap,
                   const Vector &targetPos,
                   DrawMode mode,
                   bool onlyPhysical) const
{
    if (m_pMThruster && !m_pMThruster->IsDrawnAfterParent() && (mode == g_DrawColor || mode == g_DrawMaterial))
        m_pMThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pRLeg && !m_pRLeg->IsDrawnAfterParent())
        m_pRLeg->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);    
    if (m_pLLeg && !m_pLLeg->IsDrawnAfterParent())
        m_pLLeg->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (mode == g_DrawColor || mode == g_DrawMaterial) {
        if (m_pRThruster && !m_pRThruster->IsDrawnAfterParent())
            m_pRThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pLThruster && !m_pLThruster->IsDrawnAfterParent())
            m_pLThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pURThruster && !m_pURThruster->IsDrawnAfterParent())
            m_pURThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pULThruster && !m_pULThruster->IsDrawnAfterParent())
            m_pULThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }

    ACraft::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pMThruster && m_pMThruster->IsDrawnAfterParent() && (mode == g_DrawColor || mode == g_DrawMaterial))
        m_pMThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pRLeg && m_pRLeg->IsDrawnAfterParent())
        m_pRLeg->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    if (m_pLLeg && m_pLLeg->IsDrawnAfterParent())
        m_pLLeg->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (mode == g_DrawColor || mode == g_DrawMaterial) {
        if (m_pRThruster && m_pRThruster->IsDrawnAfterParent())
            m_pRThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pLThruster && m_pLThruster->IsDrawnAfterParent())
            m_pLThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pURThruster && m_pURThruster->IsDrawnAfterParent())
            m_pURThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
        if (m_pULThruster && m_pULThruster->IsDrawnAfterParent())
            m_pULThruster->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }
/*
    if (m_pCapsule)
        m_pCapsule->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
*/
    if (mode == g_DrawColor) {
#ifdef DEBUG_BUILD
        acquire_bitmap(pTargetBitmap);
        putpixel(pTargetBitmap, floorf(m_Pos.m_X),
                              floorf(m_Pos.m_Y),
                              64);
        putpixel(pTargetBitmap, floorf(m_Pos.m_X),
                              floorf(m_Pos.m_Y),
                              64);
        release_bitmap(pTargetBitmap);

        m_pRFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pLFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
//        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false, 122);
//        m_pDeepGroup->Draw(pTargetBitmap, targetPos, false, 13);
#endif
//        m_pAtomGroup->Draw(pTargetBitmap, targetPos, false);
//        m_pFGFootGroup->Draw(pTargetBitmap, targetPos, true);
//        m_pBGFootGroup->Draw(pTargetBitmap, targetPos, true);
//        m_pBGHandGroup->Draw(pTargetBitmap, targetPos, true);
    }
}

} // namespace RTE
