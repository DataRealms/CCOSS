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
#include "SettingsMan.h"
#include "PresetMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(ACRocket, ACraft, 10);


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
    for (int i = 0; i < GearStateCount; ++i) {
        m_Paths[RIGHT][i].Reset();
        m_Paths[LEFT][i].Reset();
        m_Paths[RIGHT][i].Terminate();
        m_Paths[LEFT][i].Terminate();
    }
	m_MaxGimbalAngle = 0;
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

int ACRocket::Create(const ACRocket &reference) {
    if (reference.m_pRLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRLeg->GetUniqueID()); }
    if (reference.m_pLLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLLeg->GetUniqueID()); }
    if (reference.m_pMThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pMThruster->GetUniqueID()); }
    if (reference.m_pRThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRThruster->GetUniqueID()); }
    if (reference.m_pLThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLThruster->GetUniqueID()); }
    if (reference.m_pURThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pURThruster->GetUniqueID()); }
    if (reference.m_pULThruster) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pULThruster->GetUniqueID()); }

    ACraft::Create(reference);

    if (reference.m_pRLeg) { SetRightLeg(dynamic_cast<Leg *>(reference.m_pRLeg->Clone())); }
    if (reference.m_pLLeg) { SetLeftLeg(dynamic_cast<Leg *>(reference.m_pLLeg->Clone())); }
    if (reference.m_pMThruster) { SetMainThruster(dynamic_cast<AEmitter *>(reference.m_pMThruster->Clone())); }
    if (reference.m_pRThruster) { SetRightThruster(dynamic_cast<AEmitter *>(reference.m_pRThruster->Clone())); }
    if (reference.m_pLThruster) { SetLeftThruster(dynamic_cast<AEmitter *>(reference.m_pLThruster->Clone())); }
    if (reference.m_pURThruster) { SetURightThruster(dynamic_cast<AEmitter *>(reference.m_pURThruster->Clone())); }
    if (reference.m_pULThruster) { SetULeftThruster(dynamic_cast<AEmitter *>(reference.m_pULThruster->Clone())); }

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

    m_GearState = reference.m_GearState;

    for (int i = 0; i < GearStateCount; ++i) {
        m_Paths[RIGHT][i].Create(reference.m_Paths[RIGHT][i]);
        m_Paths[LEFT][i].Create(reference.m_Paths[LEFT][i]);
    }

	m_MaxGimbalAngle = reference.m_MaxGimbalAngle;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ACRocket::ReadProperty(const std::string_view &propName, Reader &reader) {
    if (propName == "RLeg" || propName == "RightLeg") {
        SetRightLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "LLeg" || propName == "LeftLeg") {
        SetLeftLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "RFootGroup" || propName == "RightFootGroup") {
        delete m_pRFootGroup;
        m_pRFootGroup = new AtomGroup();
        reader >> m_pRFootGroup;
        m_pRFootGroup->SetOwner(this);
    } else if (propName == "LFootGroup" || propName == "LeftFootGroup") {
        delete m_pLFootGroup;
        m_pLFootGroup = new AtomGroup();
        reader >> m_pLFootGroup;
        m_pLFootGroup->SetOwner(this);
    } else if (propName == "MThruster" || propName == "MainThruster") {
        SetMainThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "RThruster" || propName == "RightThruster") {
        SetRightThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "LThruster" || propName == "LeftThruster") {
        SetLeftThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "URThruster" || propName == "UpRightThruster") {
        SetURightThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "ULThruster" || propName == "UpLeftThruster") {
        SetULeftThruster(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "RaisedGearLimbPath") {
        reader >> m_Paths[RIGHT][RAISED];
    } else if (propName == "LoweredGearLimbPath") {
        reader >> m_Paths[RIGHT][LOWERED];
    } else if (propName == "LoweringGearLimbPath") {
        reader >> m_Paths[RIGHT][LOWERING];
    } else if (propName == "RaisingGearLimbPath") {
        reader >> m_Paths[RIGHT][RAISING];
    } else if (propName == "MaxGimbalAngle") {
		reader >> m_MaxGimbalAngle;
		m_MaxGimbalAngle *= (c_PI / 180.0F);
    } else {
        return ACraft::ReadProperty(propName, reader);
    }

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
	writer.NewProperty("MaxGimbalAngle");
	writer << m_MaxGimbalAngle / (c_PI / 180.0F);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ACRocket object.

void ACRocket::Destroy(bool notInherited)
{
    delete m_pBodyAG;
    delete m_pRFootGroup;
    delete m_pLFootGroup;
    
//    for (deque<LimbPath *>::iterator itr = m_WalkPaths.begin();
//         itr != m_WalkPaths.end(); ++itr)
//        delete *itr;

    if (!notInherited)
        ACraft::Destroy();
    Clear();
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
    float deltaTime = g_TimerMan.GetDeltaTimeSecs();

    // Look/aim update, make the scanner point aftward if the rocket is falling
    m_AimAngle = m_Vel.m_Y < 0 ? c_HalfPI : -c_HalfPI;

    /////////////////////////////////
    // Controller update and handling

    if ((m_Status == STABLE || m_Status == UNSTABLE) && !m_Controller.IsDisabled()) {
		if (m_pMThruster) { 
			if (m_MaxGimbalAngle != 0) { m_pMThruster->SetInheritedRotAngleOffset(std::sin(m_Rotation.GetRadAngle()) * m_MaxGimbalAngle - c_HalfPI); }

			if (m_Controller.IsState(MOVE_UP) || m_Controller.IsState(AIM_UP)) {
				if (!m_pMThruster->IsEmitting()) {
					m_pMThruster->TriggerBurst();
					m_ForceDeepCheck = true;
				}
				m_pMThruster->EnableEmission(true);
				m_pMThruster->AlarmOnEmit(m_Team);

				if (m_HatchState == OPEN) {
					CloseHatch();
					m_HatchTimer.Reset();
				}
			} else {
				m_pMThruster->EnableEmission(false);
			}
        }
		if (m_pULThruster || m_pURThruster) {
			if (m_Controller.IsState(MOVE_DOWN) || m_Controller.IsState(AIM_DOWN)) {
				if (m_pURThruster) {
					if (!m_pURThruster->IsEmitting()) { m_pURThruster->TriggerBurst(); }
					m_pURThruster->EnableEmission(true);
				}
				if (m_pULThruster) {
					if (!m_pULThruster->IsEmitting()) { m_pULThruster->TriggerBurst(); }
					m_pULThruster->EnableEmission(true);
				}
			} else {
				if (m_pURThruster) { m_pURThruster->EnableEmission(false); }
				if (m_pULThruster) { m_pULThruster->EnableEmission(false); }
			}
		}
		if (m_pLThruster) {
			if (m_Controller.IsState(MOVE_RIGHT)) {
				if (!m_pLThruster->IsEmitting()) { m_pLThruster->TriggerBurst(); }
				m_pLThruster->EnableEmission(true);
			} else {
				m_pLThruster->EnableEmission(false);
			}
		}
		if (m_pRThruster) {
			if (m_Controller.IsState(MOVE_LEFT)) {
				if (!m_pRThruster->IsEmitting()) { m_pRThruster->TriggerBurst(); }
				m_pRThruster->EnableEmission(true);
			} else {
				m_pRThruster->EnableEmission(false);
			}
		}
		if (m_Controller.IsState(PRESS_FACEBUTTON)) {
			if (m_HatchState == CLOSED) {
				DropAllInventory();
			} else if (m_HatchState == OPEN) {
				CloseHatch();
			}
		}
	} else {
		if (m_pMThruster) { m_pMThruster->EnableEmission(false); }
		if (m_pRThruster) { m_pRThruster->EnableEmission(false); }
		if (m_pLThruster) { m_pLThruster->EnableEmission(false); }
		if (m_pURThruster) { m_pURThruster->EnableEmission(false); }
		if (m_pULThruster) { m_pULThruster->EnableEmission(false); }
    }

    ///////////////////////////////////////////////////
    // Travel the landing gear AtomGroup:s

	if (m_pMThruster) {
        m_GearState = m_pMThruster->IsEmitting() ? LandingGearState::RAISED : LandingGearState::LOWERED;

        m_Paths[RIGHT][m_GearState].SetHFlip(m_HFlipped);
        m_Paths[LEFT][m_GearState].SetHFlip(!m_HFlipped);
        
        if (m_pRLeg) { m_pRFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pRLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[RIGHT][m_GearState], deltaTime, nullptr, true); }
        if (m_pLLeg) { m_pLFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pLLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[LEFT][m_GearState], deltaTime, nullptr, true); }
	}

    /////////////////////////////////
    // Manage Attachable:s
    if (m_pRLeg && m_pRLeg->IsAttached()) {
        m_pRLeg->SetTargetPosition(m_pRFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pLLeg && m_pLLeg->IsAttached()) {
        m_pLLeg->SetTargetPosition(m_pLFootGroup->GetLimbPos(!m_HFlipped));
    }

    /////////////////////////////////////////////////
    // Update MovableObject, adds on the forces etc, updated viewpoint
    ACraft::Update();

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetRightLeg(Leg *newLeg) {
    if (m_pRLeg && m_pRLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pRLeg); }
    if (newLeg == nullptr) {
        m_pRLeg = nullptr;
    } else {
        m_pRLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightLeg");
            dynamic_cast<ACRocket *>(parent)->SetRightLeg(castedAttachable);
        }});

        if (m_pRLeg->HasNoSetDamageMultiplier()) { m_pRLeg->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetLeftLeg(Leg *newLeg) {
    if (m_pLLeg && m_pLLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pLLeg); }
    if (newLeg == nullptr) {
        m_pLLeg = nullptr;
    } else {
        m_pLLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftLeg");
            dynamic_cast<ACRocket *>(parent)->SetLeftLeg(castedAttachable);
        }});

        if (m_pLLeg->HasNoSetDamageMultiplier()) { m_pLLeg->SetDamageMultiplier(1.0F); }
        m_pLLeg->SetInheritsHFlipped(-1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetMainThruster(AEmitter *newThruster) {
    if (m_pMThruster && m_pMThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pMThruster); }
    if (newThruster == nullptr) {
        m_pMThruster = nullptr;
    } else {
        m_pMThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetMainThruster");
            dynamic_cast<ACRocket *>(parent)->SetMainThruster(castedAttachable);
        }});

        if (m_pMThruster->HasNoSetDamageMultiplier()) { m_pMThruster->SetDamageMultiplier(1.0F); }
        m_pMThruster->SetInheritedRotAngleOffset(-c_HalfPI);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetRightThruster(AEmitter *newThruster) {
    if (m_pRThruster && m_pRThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pRThruster); }
    if (newThruster == nullptr) {
        m_pRThruster = nullptr;
    } else {
        m_pRThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightThruster");
            dynamic_cast<ACRocket *>(parent)->SetRightThruster(castedAttachable);
        }});

        if (m_pRThruster->HasNoSetDamageMultiplier()) { m_pRThruster->SetDamageMultiplier(1.0F); }
        m_pRThruster->SetInheritedRotAngleOffset(c_EighthPI);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetLeftThruster(AEmitter *newThruster) {
    if (m_pLThruster && m_pLThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pLThruster); }
    if (newThruster == nullptr) {
        m_pLThruster = nullptr;
    } else {
        m_pLThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftThruster");
            dynamic_cast<ACRocket *>(parent)->SetLeftThruster(castedAttachable);
        }});

        if (m_pLThruster->HasNoSetDamageMultiplier()) { m_pLThruster->SetDamageMultiplier(1.0F); }
        m_pLThruster->SetInheritedRotAngleOffset(c_PI - c_EighthPI);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetURightThruster(AEmitter *newThruster) {
    if (m_pURThruster && m_pURThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pURThruster); }
    if (newThruster == nullptr) {
        m_pURThruster = nullptr;
    } else {
        m_pURThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetURightThruster");
            dynamic_cast<ACRocket *>(parent)->SetURightThruster(castedAttachable);
        }});

        if (m_pURThruster->HasNoSetDamageMultiplier()) { m_pURThruster->SetDamageMultiplier(1.0F); }
        m_pURThruster->SetInheritedRotAngleOffset(c_HalfPI - c_EighthPI);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::SetULeftThruster(AEmitter *newThruster) {
    if (m_pULThruster && m_pULThruster->IsAttached()) { RemoveAndDeleteAttachable(m_pULThruster); }
    if (newThruster == nullptr) {
        m_pULThruster = nullptr;
    } else {
        m_pULThruster = newThruster;
        AddAttachable(newThruster);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newThruster->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetULeftThruster");
            dynamic_cast<ACRocket *>(parent)->SetULeftThruster(castedAttachable);
        }});

        if (m_pULThruster->HasNoSetDamageMultiplier()) { m_pULThruster->SetDamageMultiplier(1.0F); }
        m_pULThruster->SetInheritedRotAngleOffset(c_HalfPI + c_EighthPI);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACRocket::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    ACraft::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawHandAndFootGroupVisualizations()) {
        m_pRFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pLFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
    }
}

} // namespace RTE
