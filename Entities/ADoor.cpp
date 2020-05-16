//////////////////////////////////////////////////////////////////////////////////////////
// File:            ADoor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ADoor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ADoor.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "RTETools.h"
#include "Matrix.h"
#include "SLTerrain.h"
#include "PresetMan.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(ADoor, Actor, 0)
const string ADoor::Sensor::m_sClassName = "Sensor";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Sensor, effectively
//                  resetting the members of this abstraction level only.

void ADoor::Sensor::Clear()
{
    m_StartOffset.Reset();
    m_SensorRay.Reset();
    m_Skip = 3;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Sensor to be identical to another, by deep copy.

int ADoor::Sensor::Create(const Sensor &reference)
{
    m_StartOffset = reference.m_StartOffset;
    m_SensorRay = reference.m_SensorRay;
    m_Skip = reference.m_Skip;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Sensor object ready for use.

int ADoor::Sensor::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ADoor::Sensor::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "StartOffset")
        reader >> m_StartOffset;
    else if (propName == "SensorRay")
        reader >> m_SensorRay;
    else if (propName == "SkipPixels")
        reader >> m_Skip;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Sensor with a Writer for
//                  later recreation with Create(Reader &reader);

int ADoor::Sensor::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("StartOffset");
    writer << m_StartOffset;
    writer.NewProperty("SensorRay");
    writer << m_SensorRay;
    writer.NewProperty("SkipPixels");
    writer << m_Skip;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SenseActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts the ray along the sensor vector and returns any MO that was found
//                  along it.

Actor * ADoor::Sensor::SenseActor(Vector &doorPos, Matrix &doorRot, bool doorHFlipped, MOID ignoreMOID)
{
    Actor *pSensedActor = 0;
    MOID foundMOID = g_SceneMan.CastMORay(doorPos + m_StartOffset.GetXFlipped(doorHFlipped) * doorRot, m_SensorRay.GetXFlipped(doorHFlipped) * doorRot, ignoreMOID, Activity::NOTEAM, 0, true, m_Skip);

    // Convert the found MOID into actor, if it indeed is an Actor
    if (foundMOID && (foundMOID = g_MovableMan.GetRootMOID(foundMOID)))
    {
        pSensedActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(foundMOID));

        // If we found an invalid MO casting form that direction, then reverse the ray and see if we hit anything else that is relevant
        if (!pSensedActor || (pSensedActor && !pSensedActor->IsControllable()))
        {
            // Cast reverse ray
            foundMOID = g_SceneMan.CastMORay(doorPos + (m_StartOffset.GetXFlipped(doorHFlipped) + m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, (-m_SensorRay.GetXFlipped(doorHFlipped)) * doorRot, ignoreMOID, Activity::NOTEAM, 0, true, m_Skip);
            if (foundMOID && (foundMOID = g_MovableMan.GetRootMOID(foundMOID)))
                pSensedActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(foundMOID));
        }
    }

    return pSensedActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ADoor, effectively
//                  resetting the members of this abstraction level only.

void ADoor::Clear()
{
    m_pDoor = 0;
    m_OpenOffset.Reset();
    m_ClosedOffset.Reset();
    m_OpenAngle = 0;
    m_ClosedAngle = 0;

    m_DoorState = CLOSED;
    m_DoorMoveTimer.Reset();
    m_DoorMoveTime = 0;
    m_ClosedByDefault = true;
    m_ResetDefaultDelay = 8000;
    m_ResetDefaultTimer.Reset();
    m_Sensors.clear();
    m_SensorInterval = 1000;
    m_SensorTimer.Reset();
    m_DrawWhenOpen = true;
    m_DrawWhenClosed = true;
    m_LastDoorMaterialPos.Reset();
    m_DoorMaterialDrawn = false;
    m_MaterialDrawOverride = false;

	m_DoorMoveStartSound.Reset();
	m_DoorMoveSound.Reset();
	m_DoorMoveEndSound.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ADoor object ready for use.

int ADoor::Create()
{
    // Read all the properties
    if (Actor::Create() < 0)
        return -1;

    // Draw the initial door material
    if (m_DrawWhenClosed)
        DrawDoorMaterial();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ADoor to be identical to another, by deep copy.

int ADoor::Create(const ADoor &reference)
{
    Actor::Create(reference);

    if (reference.m_pDoor)
    {
        m_pDoor = dynamic_cast<Attachable *>(reference.m_pDoor->Clone());
        AddAttachable(m_pDoor, m_ClosedOffset, true);
    }
    m_OpenOffset = reference.m_OpenOffset;
    m_ClosedOffset = reference.m_ClosedOffset;
    m_OpenAngle = reference.m_OpenAngle;
    m_ClosedAngle = reference.m_ClosedAngle;

    m_DoorState = reference.m_DoorState;
    m_DoorMoveTime = reference.m_DoorMoveTime;
	m_DoorMoveStartSound = reference.m_DoorMoveStartSound;
	m_DoorMoveSound = reference.m_DoorMoveSound;
	m_DoorMoveEndSound = reference.m_DoorMoveEndSound;
    m_ClosedByDefault = reference.m_ClosedByDefault;
    // Set the inital door state to the opposite of the default, so it'll go to the default first thing
    // This will look cool, demonstrate that it's a movable door, and will force the drawing of the default state of door material
    if (!m_ClosedByDefault)
        m_DoorState = CLOSED;
    else
        m_DoorState = OPEN;

    m_ResetDefaultDelay = reference.m_ResetDefaultDelay;
    for (list<Sensor>::const_iterator eItr = reference.m_Sensors.begin(); eItr != reference.m_Sensors.end(); ++eItr)
        m_Sensors.push_back(*eItr);
    m_SensorInterval = reference.m_SensorInterval;
    m_DrawWhenOpen = reference.m_DrawWhenOpen;
    m_DrawWhenClosed = reference.m_DrawWhenClosed;
    m_LastDoorMaterialPos = reference.m_LastDoorMaterialPos;
    m_DoorMaterialDrawn = reference.m_DoorMaterialDrawn;
    m_MaterialDrawOverride = reference.m_MaterialDrawOverride;
    m_DoorMaterialID = reference.m_DoorMaterialID;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ADoor::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Door")
    {
        delete m_pDoor;
        m_pDoor = new Attachable;
        reader >> m_pDoor;
        m_DoorMaterialID = m_pDoor->GetMaterial()->GetIndex();
    }
    else if (propName == "OpenOffset")
        reader >> m_OpenOffset;
    else if (propName == "ClosedOffset")
        reader >> m_ClosedOffset;
    else if (propName == "OpenClosedOffset")
    {
        reader >> m_OpenOffset;
        m_ClosedOffset = m_OpenOffset;
    }
    else if (propName == "OpenAngle")
    {
        Matrix rotation;
        reader >> rotation;
        m_OpenAngle = rotation.GetRadAngle();
    }
    else if (propName == "ClosedAngle")
    {
        Matrix rotation;
        reader >> rotation;
        m_ClosedAngle = rotation.GetRadAngle();
    }
    else if (propName == "OpenClosedAngle")
    {
        Matrix rotation;
        reader >> rotation;
        m_OpenAngle = rotation.GetRadAngle();
        m_ClosedAngle = rotation.GetRadAngle();
    }
    else if (propName == "DoorMoveTime")
        reader >> m_DoorMoveTime;
	else if (propName == "DoorMoveStartSound") {
		reader >> m_DoorMoveStartSound;
	} else if (propName == "DoorMoveSound") {
		reader >> m_DoorMoveSound;
	} else if (propName == "DoorMoveEndSound")
		reader >> m_DoorMoveEndSound;
    else if (propName == "ClosedByDefault")
    {
        reader >> m_ClosedByDefault;
        // Set the inital door state to the opposite of the default, so it'll go to the default first thing
        // This will look cool, demonstrate that it's a movable door, and will force the drawing of the default state of door material
        if (!m_ClosedByDefault)
            m_DoorState = CLOSED;
        else
            m_DoorState = OPEN;
    }
    else if (propName == "ResetDefaultDelay")
        reader >> m_ResetDefaultDelay;
    else if (propName == "AddSensor")
    {
        Sensor exit;
        reader >> exit;
        m_Sensors.push_back(exit);
    }
    else if (propName == "SensorInterval")
        reader >> m_SensorInterval;
    else if (propName == "DrawWhenOpen")
        reader >> m_DrawWhenOpen;
    else if (propName == "DrawWhenClosed")
        reader >> m_DrawWhenClosed;
    else
        // See if the base class(es) can find a match instead
        return Actor::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ADoor with a Writer for
//                  later recreation with Create(Reader &reader);

int ADoor::Save(Writer &writer) const
{
    Actor::Save(writer);

    writer.NewProperty("Door");
    writer << m_pDoor;
    writer.NewProperty("OpenOffset");
    writer << m_OpenOffset;
    writer.NewProperty("ClosedOffset");
    writer << m_ClosedOffset;
    writer.NewProperty("OpenAngle");
    writer << Matrix(m_OpenAngle);
    writer.NewProperty("ClosedAngle");
    writer << Matrix(m_ClosedAngle);
    writer.NewProperty("DoorDelay");
    writer << m_DoorMoveTime;
	writer.NewProperty("DoorMoveStartSound");
	writer << m_DoorMoveStartSound;
	writer.NewProperty("DoorMoveSound");
	writer << m_DoorMoveSound;
	writer.NewProperty("DoorMoveEndSound");
	writer << m_DoorMoveEndSound;
    writer.NewProperty("ClosedByDefault");
    writer << m_ClosedByDefault;
    writer.NewProperty("ResetDefaultDelay");
    writer << m_ResetDefaultDelay;
    for (list<Sensor>::const_iterator itr = m_Sensors.begin(); itr != m_Sensors.end(); ++itr)
    {
        writer.NewProperty("AddSensor");
        writer << (*itr);
    }
    writer.NewProperty("SensorInterval");
    writer << m_SensorInterval;
    writer.NewProperty("DrawWhenOpen");
    writer << m_DrawWhenOpen;
    writer.NewProperty("DrawWhenClosed");
    writer << m_DrawWhenClosed;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ADoor object.

void ADoor::Destroy(bool notInherited)
{
//    g_MovableMan.RemoveEntityPreset(this);

//    EraseDoorMaterial();

	m_DoorMoveStartSound.Stop();
	m_DoorMoveSound.Stop();
	m_DoorMoveEndSound.Stop();
    delete m_pDoor;

    if (!notInherited)
        Actor::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ADoor, including the mass of its
//                  currently attached body parts and inventory.

float ADoor::GetMass() const
{
    float totalMass = Actor::GetMass();

    if (m_pDoor)
        totalMass += m_pDoor->GetMass();

    return totalMass;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.

void ADoor::SetID(const MOID newID)
{
    Actor::SetID(newID);

    if (m_pDoor)
        m_pDoor->SetID(newID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenDoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the door, if it's closed.
// Arguments:       None.
// Return value:    None.

void ADoor::OpenDoor()
{
	if (m_DoorState == CLOSED) {
		if (!m_DoorMoveStartSound.IsBeingPlayed()) { m_DoorMoveStartSound.Play(m_Pos); }
        m_DoorState = OPENING;
        m_DoorMoveTimer.Reset();
        // Clear the material rep of the door to the terrain from the closed state
        EraseDoorMaterial();
        if (m_pDoor)
            m_pDoor->DeepCheck(true);
    }
    else if (m_DoorState == CLOSING)
    {
        m_DoorState = OPENING;
        // Set the time left as the time now elapsed from a fully open position
        m_DoorMoveTimer.SetElapsedSimTimeMS(m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS());
    }

    m_ResetDefaultTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CloseDoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes the door, if it's open.

void ADoor::CloseDoor()
{
	if (m_DoorState == OPEN) {
		if (!m_DoorMoveStartSound.IsBeingPlayed()) { m_DoorMoveStartSound.Play(m_Pos); }
        m_DoorState = CLOSING;
        m_DoorMoveTimer.Reset();
        // Clear the material rep of the door to the terrain from the open state
        EraseDoorMaterial();
        if (m_pDoor)
            m_pDoor->DeepCheck(true);
    }
    else if (m_DoorState == OPENING)
    {
        m_DoorState = CLOSING;
        // Set the time left as the time now elapsed from a fully open position
        m_DoorMoveTimer.SetElapsedSimTimeMS(m_DoorMoveTime - m_DoorMoveTimer.GetElapsedSimTimeMS());
    }

    m_ResetDefaultTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void ADoor::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
{
    // Detach all limbs and let loose
    if (m_pDoor && m_pDoor->IsAttached())
    {
        // Remove the material representation of the door, if any
        EraseDoorMaterial();
        m_pDoor->DeepCheck(true);
        m_pDoor->SetPinStrength(0);
        m_pDoor->Detach();
        m_pDoor->SetVel(m_Vel + m_pDoor->GetParentOffset() * PosRand());
        m_pDoor->SetAngularVel(NormalRand());
        g_MovableMan.AddParticle(m_pDoor);
        m_pDoor = 0;
    }

    Actor::GibThis(impactImpulse, internalBlast, pIgnoreMO);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool ADoor::IsOnScenePoint(Vector &scenePoint) const
{
    return ((m_pDoor && m_pDoor->IsOnScenePoint(scenePoint)) ||
            Actor::IsOnScenePoint(scenePoint));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MaterialDrawOverride
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to temporarily remove or add back the material drawing of this
//                  in the scene. Used for making pathfinding work through doors.

void ADoor::MaterialDrawOverride(bool enable)
{
    if (!g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap())
        return;

    // We are enabling the override, meaning we will erase any existing material door
    if (enable)
    {
        // Erase the material temporarily if we have drawn it and the override isn't already in effect
        if (m_DoorMaterialDrawn && m_MaterialDrawOverride != enable)
        {
            int fillX = m_LastDoorMaterialPos.GetFloorIntX();
            int fillY = m_LastDoorMaterialPos.GetFloorIntY();
            if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir)
            {
                floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
                // Register that we changed the material layer of the terrain
                g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_pDoor->GetBoundingBox());
            }
        }
        m_MaterialDrawOverride = enable;
    }
    // We are disabling the override, so we need to draw back the door if it is supposed to be drawn!
    else
    {
        // Draw the door back if we were indeed temporarily supressing it before
        if (m_DoorMaterialDrawn && m_MaterialDrawOverride != enable)
        {
            Vector notUsed;
            m_pDoor->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), notUsed, g_DrawMaterial, true);
            // Register that we changed the material layer of the terrain
            g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_pDoor->GetBoundingBox());
        }
        m_MaterialDrawOverride = enable;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ADoor. Supposed to be done every frame.

void ADoor::Update()
{
    /////////////////////////////////////////////////
    // Update MovableObject, adds on the forces etc

    Actor::Update();

    /////////////////////////////////////////
    // Open or close the door interpolation

    if (m_pDoor && m_pDoor->IsAttached())
    {
        m_pDoor->SetHFlipped(m_HFlipped);

		if (m_DoorState == OPENING) {
			if ((m_DoorMoveSound.GetLoopSetting() == -1) && !m_DoorMoveSound.IsBeingPlayed()) { m_DoorMoveSound.Play(m_Pos); }
			// Reached open state
			if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
				m_DoorState = OPEN;
				m_DoorMoveSound.Stop();
				if (!m_DoorMoveEndSound.IsBeingPlayed()) { m_DoorMoveEndSound.Play(m_Pos); }
                m_ResetDefaultTimer.Reset();
                m_pDoor->SetJointPos(m_Pos + m_OpenOffset.GetXFlipped(m_HFlipped) * m_Rotation);
                m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -m_OpenAngle : m_OpenAngle));
                // Draw the material rep of the door to the terrain in the open state
                if (m_DrawWhenOpen)
                    DrawDoorMaterial();
            }
            // Still opening
            else
            {
                // Find interpolated positions and angles for opening
                Vector openingOffset(LERP(0, m_DoorMoveTime, m_ClosedOffset.m_X, m_OpenOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()),
                                     LERP(0, m_DoorMoveTime, m_ClosedOffset.m_Y, m_OpenOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
// TODO: Make this work across rotation 0
                float openingAngle = LERP(0, m_DoorMoveTime, m_ClosedAngle, m_OpenAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

                m_pDoor->SetJointPos(m_Pos + openingOffset.GetXFlipped(m_HFlipped) * m_Rotation);
                m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -openingAngle : openingAngle));
                // Do the deep check so clear away any terrain debris when the door is moving,
                // but only after a lil bit so it doesn't take a chunk out of the ground before being snapped into place
                if (m_DoorMoveTimer.IsPastSimMS(50))
                    m_pDoor->DeepCheck(true);
            }
        }
		else if (m_DoorState == CLOSING) {
			if ((m_DoorMoveSound.GetLoopSetting() == -1) && !m_DoorMoveSound.IsBeingPlayed()) { m_DoorMoveSound.Play(m_Pos); }
			// Reached closed state
			if (m_DoorMoveTimer.IsPastSimMS(m_DoorMoveTime)) {
				m_DoorState = CLOSED;
				m_DoorMoveSound.Stop();
				if (!m_DoorMoveEndSound.IsBeingPlayed()) { m_DoorMoveEndSound.Play(m_Pos); }
                m_ResetDefaultTimer.Reset();
                m_pDoor->SetJointPos(m_Pos + m_ClosedOffset.GetXFlipped(m_HFlipped) * m_Rotation);
                m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -m_ClosedAngle : m_ClosedAngle));
                // Draw the material rep of the door to the terrain in the closed state
                if (m_DrawWhenClosed)
                    DrawDoorMaterial();
            }
            // Still closing
            else
            {
                // Find interpolated positions and angles for closing
                Vector closingOffset(LERP(0, m_DoorMoveTime, m_OpenOffset.m_X, m_ClosedOffset.m_X, m_DoorMoveTimer.GetElapsedSimTimeMS()),
                                     LERP(0, m_DoorMoveTime, m_OpenOffset.m_Y, m_ClosedOffset.m_Y, m_DoorMoveTimer.GetElapsedSimTimeMS()));
// TODO: Make this work across rotation 0
                float closingAngle = LERP(0, m_DoorMoveTime, m_OpenAngle, m_ClosedAngle, m_DoorMoveTimer.GetElapsedSimTimeMS());

                m_pDoor->SetJointPos(m_Pos + closingOffset.GetXFlipped(m_HFlipped) * m_Rotation);
                m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -closingAngle : closingAngle));
                // Do the deep check so clear away any terrain debris when the door is moving
                // but only after a lil bit so it doesn't take a chunk out of the ground before being snapped into place
                if (m_DoorMoveTimer.IsPastSimMS(50))
                    m_pDoor->DeepCheck(true);
            }
        }
        else if (m_DoorState == OPEN)
        {
            m_pDoor->SetJointPos(m_Pos + m_OpenOffset.GetXFlipped(m_HFlipped) * m_Rotation);
            m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -m_OpenAngle : m_OpenAngle));
        }
        else if (m_DoorState == CLOSED)
        {
            m_pDoor->SetJointPos(m_Pos + m_ClosedOffset.GetXFlipped(m_HFlipped) * m_Rotation);
            m_pDoor->SetRotAngle(m_Rotation.GetRadAngle() + (m_HFlipped ? -m_ClosedAngle : m_ClosedAngle));
        }

        m_pDoor->Update();
        // Update the Atoms' offsets in the parent group
//        Matrix atomRot(FacingAngle(m_pDoor->GetRotMatrix().GetRadAngle()) - FacingAngle(m_Rotation.GetRadAngle()));
//        m_pAtomGroup->UpdateSubAtoms(m_pDoor->GetAtomSubgroupID(), m_pDoor->GetParentOffset() - (m_pDoor->GetJointOffset() * atomRot), atomRot);

//        m_Health -= m_pDoor->CollectDamage();
    }

    // See if the door has been detached/gibbed
    if (m_pDoor && !ApplyAttachableForces(m_pDoor))
    {
        // Remove the material representation if the door fell off
        EraseDoorMaterial();
        // Start the spinning out of control anim for the motor, start it slow
        m_SpriteAnimDuration *= 4;
        m_pDoor = 0;
    }

    /////////////////////////////////////////
    // Sense Actors with all sensors and open/close accordingly

    // Check if it's time to do a sensor sweep
    if (m_SensorTimer.IsPastSimMS(m_SensorInterval))
    {
        Actor *pFoundActor = 0;
        bool anySensorInput = false;

        // Go through all the sensors, checking for Actor:s breaking the beams
        for (list<Sensor>::iterator itr = m_Sensors.begin(); itr != m_Sensors.end(); ++itr)
        {
            pFoundActor = (*itr).SenseActor(m_Pos, m_Rotation, m_HFlipped, m_MOID);

            // If valid actor found, act accordingly
            if (pFoundActor && pFoundActor->IsControllable())
            {
                anySensorInput = true;

                // Close the door if it's an actor of other team
                if (pFoundActor->GetTeam() != m_Team)
                {
                    CloseDoor();
                    break;
                }
                // Open the door if it's an actor of the same team
                else if (pFoundActor->GetTeam() == m_Team)
                    OpenDoor();
            }
        }

        // If no sensor input, return to default state if it's time
        if (!anySensorInput && m_ResetDefaultTimer.IsPastSimMS(m_ResetDefaultDelay))
        {
            // This won't override any actions that the sensors started
            if (m_ClosedByDefault && m_DoorState == OPEN)
                CloseDoor();
            else if (!m_ClosedByDefault && m_DoorState == CLOSED)
                OpenDoor();
        }

        // Start timing for next sensor sweep
        m_SensorTimer.Reset();
    }

    /////////////////////////////////
    // Animate the body sprite

    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENOPENCLOSE)
        {
            if (m_DoorState == OPENING)
            {
                float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % m_SpriteAnimDuration;
                m_Frame = floorf((cycleTime / (float)m_SpriteAnimDuration) * (float)m_FrameCount);
            }
            else if (m_DoorState == CLOSING)
            {
                float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % m_SpriteAnimDuration;
                m_Frame = floorf((cycleTime / (float)m_SpriteAnimDuration) * (float)m_FrameCount);
            }
        }
    }

    //////////////////////////////////////
    // Death logic

    // Lose health when door is lost, spinning out of control until grinds to halt
    if (!m_pDoor && m_Status != DYING && m_Status != DEAD)
    {
        m_SpriteAnimMode = ALWAYSLOOP;
        // Make it go faster and faster
        if (m_SpriteAnimDuration > 650)
            m_SpriteAnimDuration -= 1;
        m_Health -= 0.4;
    }

    // Blow up when dead
    if (m_Status == DEAD)
        GibThis(Vector(), 50);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.

void ADoor::UpdateChildMOIDs(vector<MovableObject *> &MOIDIndex,
                             MOID rootMOID,
                             bool makeNewMOID)
{
    if (m_pDoor)
        m_pDoor->UpdateMOID(MOIDIndex, m_RootMOID, makeNewMOID);

    Actor::UpdateChildMOIDs(MOIDIndex, m_RootMOID, makeNewMOID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

void ADoor::GetMOIDs(std::vector<MOID> &MOIDs) const
{
	if (m_pDoor)
		m_pDoor->GetMOIDs(MOIDs);

	Actor::GetMOIDs(MOIDs);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAnyRandomWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds from the actor and all standard attachables.

int ADoor::RemoveAnyRandomWounds(int amount)
{
	float damage = 0;

	for (int i = 0; i < amount; i++)
	{
		// Fill the list of damaged bodyparts
		std::vector<MOSRotating *> bodyParts;
		if (GetWoundCount() > 0)
			bodyParts.push_back(this);

		if (m_pDoor && m_pDoor->GetWoundCount())
			bodyParts.push_back(m_pDoor);

		// Stop removing wounds if there are not any left
		if (bodyParts.size() == 0)
			break;

		int partIndex = RangeRand(0, bodyParts.size() - 1);
		MOSRotating * part = bodyParts[partIndex];
		damage += part->RemoveWounds(1);
	}

	return damage;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ADoor's current graphical representation to a
//                  BITMAP of choice.

void ADoor::Draw(BITMAP *pTargetBitmap,
                   const Vector &targetPos,
                   DrawMode mode,
                   bool onlyPhysical) const
{
    // Override color drawing with flash, if requested.
    DrawMode realMode = (mode == g_DrawColor && m_FlashWhiteMS) ? g_DrawWhite : mode;

    if (m_pDoor && m_pDoor->IsAttached() && !m_pDoor->IsDrawnAfterParent())
        m_pDoor->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical);

    Actor::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pDoor && m_pDoor->IsAttached() && m_pDoor->IsDrawnAfterParent())
        m_pDoor->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void ADoor::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible)
        return;

    // Only draw if the team viewing this is on the same team OR has seen the space where this is located
    int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NOTEAM)
    {
        if (g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam))
            return;
    }

//    Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawDoorMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the material under the pos of the door attachable, to create
//                  terrain collision detection for the doors.

void ADoor::DrawDoorMaterial()
{
    if (!m_pDoor || m_MaterialDrawOverride || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap())
        return;

    // Erase the previous drawing if it already exists
    if (m_DoorMaterialDrawn)
    {
        int fillX = m_LastDoorMaterialPos.GetFloorIntX();
        int fillY = m_LastDoorMaterialPos.GetFloorIntY();
        if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir)
            floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
    }

    // Now draw the material silhouette to the terrain
	Vector notUsed;
    m_pDoor->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), notUsed, g_DrawMaterial, true);
    m_LastDoorMaterialPos = m_pDoor->GetPos();
    m_DoorMaterialDrawn = true;

    // Register that we changed the material layer of the terrain
    g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_pDoor->GetBoundingBox());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EraseDoorMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Floodfills the material area under the pos of the door attachable
//                  hat matches the material index of it. This is to get rid of the
//                  material footprint when the door part starts to move.

bool ADoor::EraseDoorMaterial()
{
    m_DoorMaterialDrawn = false;

    if (!m_pDoor || !g_SceneMan.GetTerrain() || !g_SceneMan.GetTerrain()->GetMaterialBitmap())
        return false;

    int fillX = m_LastDoorMaterialPos.GetFloorIntX();
    int fillY = m_LastDoorMaterialPos.GetFloorIntY();

    if (g_SceneMan.GetTerrMatter(fillX, fillY) != g_MaterialAir)
    {
        floodfill(g_SceneMan.GetTerrain()->GetMaterialBitmap(), fillX, fillY, g_MaterialAir);
        // Register that we changed the material layer of the terrain
        g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(m_pDoor->GetBoundingBox());
        return true;
    }

    return false;
}

} // namespace RTE