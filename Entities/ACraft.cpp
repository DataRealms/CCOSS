//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACraft.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ACraft class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ACraft.h"

#include "AtomGroup.h"
#include "Leg.h"
#include "Controller.h"
#include "Matrix.h"
#include "AEmitter.h"
#include "HDFirearm.h"
#include "Controller.h"
#include "SceneMan.h"
#include "Scene.h"
#include "SettingsMan.h"
#include "FrameMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

AbstractClassInfo(ACraft, Actor);
const std::string ACraft::Exit::c_ClassName = "Exit";

bool ACraft::s_CrabBombInEffect = false;

#define EXITLINESPACING 7
#define EXITSUCKDELAYMS 1500


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Exit, effectively
//                  resetting the members of this abstraction level only.

void ACraft::Exit::Clear()
{
    m_Offset.Reset();
    m_Velocity.Reset();
    m_VelSpread = 0.2f;
    m_Radius = 10;
    m_Range = 35;
    m_Clear = true;
    m_pIncomingMO = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Exit to be identical to another, by deep copy.

int ACraft::Exit::Create(const Exit &reference)
{
    m_Offset = reference.m_Offset;
    m_Velocity = reference.m_Velocity;
    m_VelSpread = reference.m_VelSpread;
    m_Radius = reference.m_Radius;
    m_Range = reference.m_Range;
    m_Clear = reference.m_Clear;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Exit object ready for use.

int ACraft::Exit::Create()
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

int ACraft::Exit::ReadProperty(const std::string_view &propName, Reader &reader)
{
    StartPropertyList(return Serializable::ReadProperty(propName, reader));

    MatchProperty("Offset", { reader >> m_Offset; });
    MatchProperty("Velocity", { reader >> m_Velocity; });
    MatchProperty("VelocitySpread", { reader >> m_VelSpread; });
    MatchProperty("Radius", { reader >> m_Radius; });
    MatchProperty("Range", { reader >> m_Range; });

    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Exit with a Writer for
//                  later recreation with Create(Reader &reader);

int ACraft::Exit::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("Offset");
    writer << m_Offset;
    writer.NewProperty("Velocity");
    writer << m_Velocity;
    writer.NewProperty("VelocitySpread");
    writer << m_VelSpread;
    writer.NewProperty("Radius");
    writer << m_Radius;
    writer.NewProperty("Range");
    writer << m_Range;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CheckIfClear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates whether this exit is currently clear enough of terrain to
//                  safely put things through without them ending up in the terrain.

bool ACraft::Exit::CheckIfClear(const Vector &pos, Matrix &rot, float size)
{
	Vector notUsed;
    Vector ray = m_Velocity;
    ray.SetMagnitude(size);
    return m_Clear = !g_SceneMan.CastNotMaterialRay(pos + (m_Offset * rot), ray * rot, g_MaterialAir, notUsed);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SuckInMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses cast MO rays to see if anyhting is able to be drawn into this
//                  exit. If so, it will alter the positiona nd velocity of the objet so
//                  it flies into the exit until it is sufficiently inside and then it'll
//                  return the MO here, OWNERHIP NOT TRANSFERRED! It is still in MovableMan!

MOSRotating * ACraft::Exit::SuckInMOs(ACraft *pExitOwner)
{
    if (!pExitOwner || !m_Clear)
        return 0;

    Vector exitPos = pExitOwner->GetPos() + pExitOwner->RotateOffset(m_Offset);
    Vector exitRadius;
    Vector exitCorner;
    Vector rayVec = pExitOwner->RotateOffset(m_Velocity);

    // If we're sucking on an MO already
    if (m_pIncomingMO)
    {
        const float suckageRange = m_Range * 1.5F;

        // Check that it's still active and valid (not destroyed)
        if (!(g_MovableMan.IsDevice(m_pIncomingMO) || g_MovableMan.IsActor(m_pIncomingMO)))
        {
            m_pIncomingMO = 0;
        }
        // See if it's now out of range of suckage
        else if ((exitPos - m_pIncomingMO->GetPos()).MagnitudeIsGreaterThan(suckageRange))
        {
            m_pIncomingMO = 0;
        }
        // See if the object has been sucked in far enough to be considered picked up by the exit
        else if ((m_pIncomingMO->GetPos() - exitPos).Dot(rayVec) < 0)
        {
            // Yes, the object has been sucked in beyond the offset position of the exit itself, so we're done picking it up
            // Zero out the incoming member pointer to show that we're not sucking on anyhting anymore
            MOSRotating *pReturnMO = m_pIncomingMO;
            m_pIncomingMO = 0;
            return pReturnMO;
        }
        // Ok it's not quite there yet, so keep sucking it in
        else
        {
            // Figure the distance left for the object to go to reach the exit
            Vector toGo = exitPos - m_pIncomingMO->GetPos();
            // If the object is still a bit away from the exit goal, override velocity of the object to head straight into the exit
            const float threshold = 1.0F;
            if (toGo.MagnitudeIsGreaterThan(threshold))
            {
                m_pIncomingMO->SetVel(toGo.SetMagnitude(m_Velocity.GetMagnitude()));
            }

            // Turn off collisions between the object and the craft sucking it in
            m_pIncomingMO->SetWhichMOToNotHit(pExitOwner, 3);
            pExitOwner->SetWhichMOToNotHit(m_pIncomingMO, 3);
        }
    }

    // Not sucking in anything, so see if there's anyhting to start doing it to
    if (!m_pIncomingMO)
    {
        exitRadius = pExitOwner->RotateOffset(m_Velocity.GetPerpendicular().SetMagnitude(m_Radius));
        exitCorner = exitPos + exitRadius;
        rayVec = pExitOwner->RotateOffset(m_Velocity);
        rayVec.SetMagnitude(m_Range);

        MOID itemMOID = g_SceneMan.CastMORay(exitCorner, rayVec, pExitOwner->GetRootID(), Activity::NoTeam, g_MaterialGrass, true, 4);
        // Try the other side if we didn't find anything
        if (itemMOID == g_NoMOID)
        {
            exitCorner -= exitRadius * 2;
            itemMOID = g_SceneMan.CastMORay(exitCorner, rayVec, pExitOwner->GetRootID(), Activity::NoTeam, g_MaterialGrass, true, 4);
        }
        // Try center beam if we STILL didn't find anything
        if (itemMOID == g_NoMOID)
        {
            exitCorner += exitRadius;
            itemMOID = g_SceneMan.CastMORay(exitCorner, rayVec, pExitOwner->GetRootID(), Activity::NoTeam, g_MaterialGrass, true, 4);
        }

        // See if we caught anything
        MovableObject *pItem = g_MovableMan.GetMOFromID(itemMOID);
        if (pItem)
        {
            // We did! Now start sucking it in (next frame)
            m_pIncomingMO = dynamic_cast<MOSRotating *>(pItem->GetRootParent());
            // Don't suck in other ACraft!
            if (dynamic_cast<ACraft *>(m_pIncomingMO))
                m_pIncomingMO = 0;
        }
    }

    // Nothing was sucked in far enough to be returned as done
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACraft, effectively
//                  resetting the members of this abstraction level only.

void ACraft::Clear()
{
    m_AIMode = AIMODE_DELIVER;

    m_MoveState = 0;
    m_HatchState = CLOSED;
    m_HatchTimer.Reset();
    m_HatchDelay = 0;
    m_HatchOpenSound = nullptr;
    m_HatchCloseSound = nullptr;
    m_CollectedInventory.clear();
    m_Exits.clear();
    m_CurrentExit = m_Exits.begin();
    m_ExitInterval = 1000;
    m_ExitTimer.Reset();
    m_ExitLinePhase = 0;
    m_HasDelivered = false;
    m_LandingCraft = true;
    m_FlippedTimer.Reset();
    m_CrashTimer.Reset();
    m_CrashSound = nullptr;

    m_DeliveryState = FALL;
    m_AltitudeMoveState = HOVER;
    m_AltitudeControl = 0;
    m_MaxPassengers = -1;

	m_DeliveryDelayMultiplier = 1.0;
	m_ScuttleIfFlippedTime = 4000;
	m_ScuttleOnDeath = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACraft object ready for use.

int ACraft::Create()
{
    // Read all the properties
    if (Actor::Create() < 0)
        return -1;

	if (m_AIMode == Actor::AIMODE_NONE) {
		m_AIMode = Actor::AIMODE_DELIVER;
	}

    m_CurrentExit = m_Exits.begin();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACraft to be identical to another, by deep copy.

int ACraft::Create(const ACraft &reference)
{
    Actor::Create(reference);

    m_MoveState = reference.m_MoveState;
    m_HatchState = reference.m_HatchState;
    m_HatchDelay = reference.m_HatchDelay;
	if (reference.m_HatchOpenSound) { m_HatchOpenSound = dynamic_cast<SoundContainer *>(reference.m_HatchOpenSound->Clone()); }
	if (reference.m_HatchCloseSound) {
		m_HatchCloseSound = dynamic_cast<SoundContainer *>(reference.m_HatchCloseSound->Clone());
	} else if (reference.m_HatchOpenSound) {
		m_HatchCloseSound = dynamic_cast<SoundContainer *>(reference.m_HatchOpenSound->Clone());
	}
	for (std::deque<MovableObject *>::const_iterator niItr = reference.m_CollectedInventory.begin(); niItr != reference.m_CollectedInventory.end(); ++niItr)
        m_CollectedInventory.push_back(dynamic_cast<MovableObject *>((*niItr)->Clone()));
    for (std::list<Exit>::const_iterator eItr = reference.m_Exits.begin(); eItr != reference.m_Exits.end(); ++eItr)
        m_Exits.push_back(*eItr);
    m_CurrentExit = m_Exits.begin();
    m_ExitInterval = reference.m_ExitInterval;
    m_HasDelivered = reference.m_HasDelivered;
    m_LandingCraft = reference.m_LandingCraft;
	if (reference.m_CrashSound) { m_CrashSound = dynamic_cast<SoundContainer *>(reference.m_CrashSound->Clone()); }

    m_DeliveryState = reference.m_DeliveryState;
    m_AltitudeMoveState = reference.m_AltitudeMoveState;
    m_AltitudeControl = reference.m_AltitudeControl;
    m_MaxPassengers = reference.m_MaxPassengers;

	m_DeliveryDelayMultiplier = reference.m_DeliveryDelayMultiplier;
	m_ScuttleIfFlippedTime = reference.m_ScuttleIfFlippedTime;
	m_ScuttleOnDeath = reference.m_ScuttleOnDeath;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ACraft::ReadProperty(const std::string_view &propName, Reader &reader)
{
    StartPropertyList(return Actor::ReadProperty(propName, reader)); 
    
    MatchProperty("HatchDelay", { reader >> m_HatchDelay; });
	MatchProperty("HatchOpenSound", {
		m_HatchOpenSound = new SoundContainer;
		reader >> m_HatchOpenSound;
	});
	MatchProperty("HatchCloseSound", {
		m_HatchCloseSound = new SoundContainer;
		reader >> m_HatchCloseSound;
	});
	MatchProperty("CrashSound", {
		m_CrashSound = new SoundContainer;
		reader >> m_CrashSound;
	});
	MatchProperty("AddExit",
    {
        Exit exit;
        reader >> exit;
        m_Exits.push_back(exit);
    });
    MatchProperty("DeliveryDelayMultiplier", { reader >> m_DeliveryDelayMultiplier; });
	MatchProperty("ExitInterval", { reader >> m_ExitInterval; });
	MatchProperty("CanLand", { reader >> m_LandingCraft; });
    MatchProperty("MaxPassengers", { reader >> m_MaxPassengers; });
	MatchProperty("ScuttleIfFlippedTime", { reader >> m_ScuttleIfFlippedTime; });
    MatchProperty("ScuttleOnDeath", { reader >> m_ScuttleOnDeath; });

    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ACraft with a Writer for
//                  later recreation with Create(Reader &reader);

int ACraft::Save(Writer &writer) const
{
    Actor::Save(writer);

    writer.NewProperty("HatchDelay");
    writer << m_HatchDelay;
    writer.NewProperty("HatchOpenSound");
    writer << m_HatchOpenSound;
    writer.NewProperty("HatchCloseSound");
    writer << m_HatchCloseSound;
    for (std::list<Exit>::const_iterator itr = m_Exits.begin(); itr != m_Exits.end(); ++itr)
    {
        writer.NewProperty("AddExit");
        writer << (*itr);
    }
    writer.NewProperty("DeliveryDelayMultiplier");
    writer << m_DeliveryDelayMultiplier;
	writer.NewProperty("ExitInterval");
	writer << m_ExitInterval;
	writer.NewProperty("CanLand");
    writer << m_LandingCraft;

    writer.NewProperty("CrashSound");
    writer << m_CrashSound;

    writer.NewProperty("MaxPassengers");
    writer << m_MaxPassengers;
	writer.NewProperty("ScuttleIfFlippedTime");
	writer << m_ScuttleIfFlippedTime;
	writer.NewProperty("ScuttleOnDeath");
	writer << m_ScuttleOnDeath;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ACraft object.

void ACraft::Destroy(bool notInherited)
{
	delete m_HatchOpenSound;
	delete m_HatchCloseSound;
	delete m_CrashSound;

    if (!notInherited)
        Actor::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this Actor and all its carried
//                  gold and inventory.

float ACraft::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const
{
    float totalValue = Actor::GetTotalValue(nativeModule, foreignMult, nativeMult);

    MOSprite *pItem = 0;
    for (std::deque<MovableObject *>::const_iterator itr = m_CollectedInventory.begin(); itr != m_CollectedInventory.end(); ++itr)
    {
        pItem = dynamic_cast<MOSprite *>(*itr);
        if (pItem)
            totalValue += pItem->GetTotalValue(nativeModule, foreignMult, nativeMult);
    }

    return totalValue;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this carries a specifically named object in its inventory.
//                  Also looks through the inventories of potential passengers, as applicable.

bool ACraft::HasObject(std::string objectName) const
{
    if (Actor::HasObject(objectName))
        return true;

    for (std::deque<MovableObject *>::const_iterator itr = m_CollectedInventory.begin(); itr != m_CollectedInventory.end(); ++itr)
    {
        if ((*itr) && (*itr)->HasObject(objectName))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObjectInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically grouped object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.

bool ACraft::HasObjectInGroup(std::string groupName) const
{
    if (Actor::HasObjectInGroup(groupName))
        return true;

    for (std::deque<MovableObject *>::const_iterator itr = m_CollectedInventory.begin(); itr != m_CollectedInventory.end(); ++itr)
    {
        if ((*itr) && (*itr)->HasObjectInGroup(groupName))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this belongs to, and all its inventory too.

void ACraft::SetTeam(int team)
{
    Actor::SetTeam(team);

    // Also set all actors in the new inventory
    Actor *pActor = 0;
    for (std::deque<MovableObject *>::iterator itr = m_CollectedInventory.begin(); itr != m_CollectedInventory.end(); ++itr)
    {
        pActor = dynamic_cast<Actor *>(*itr);
        if (pActor)
            pActor->SetTeam(team);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACraft::HandlePieCommand(PieSlice::SliceType pieSliceIndex) {
    if (pieSliceIndex != PieSlice::SliceType::NoType) {
        if (pieSliceIndex == PieSlice::SliceType::Deliver) {
            m_AIMode = AIMODE_DELIVER;
            m_DeliveryState = FALL;
            m_HasDelivered = false;
        } else if (pieSliceIndex == PieSlice::SliceType::Return) {
            m_AIMode = AIMODE_RETURN;
            m_DeliveryState = LAUNCH;
        } else if (pieSliceIndex == PieSlice::SliceType::Stay) {
            m_AIMode = AIMODE_STAY;
            m_DeliveryState = FALL;
        } else if (pieSliceIndex == PieSlice::SliceType::Sentry) {
            m_AIMode = AIMODE_SENTRY;
            m_DeliveryState = FALL;
        } else if (pieSliceIndex == PieSlice::SliceType::Return) {
            m_AIMode = AIMODE_RETURN;
            m_DeliveryState = LAUNCH;
        } else if (pieSliceIndex == PieSlice::SliceType::GoTo) {
            m_AIMode = AIMODE_GOTO;
            m_DeliveryState = FALL;
            ClearAIWaypoints();
            m_UpdateMovePath = true;
        } else if (pieSliceIndex == PieSlice::SliceType::Scuttle) {
            m_AIMode = AIMODE_SCUTTLE;
        } else {
            return Actor::HandlePieCommand(pieSliceIndex);
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenHatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the hatch doors, if they're closed.
// Arguments:       None.
// Return value:    None.

void ACraft::OpenHatch()
{
    if (m_HatchState == CLOSED || m_HatchState == CLOSING)
    {
        m_HatchState = OPENING;
        m_HatchTimer.Reset();

        // PSCHHT
		if (m_HatchOpenSound) { m_HatchOpenSound->Play(m_Pos); }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CloseHatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes the hatch doors, if they're open.
// Arguments:       None.
// Return value:    None.

void ACraft::CloseHatch()
{
    if (m_HatchState == OPEN || m_HatchState == OPENING)
    {
        m_HatchState = CLOSING;
        m_HatchTimer.Reset();

        // When closing, move all newly added inventory to the regular inventory list so it'll be ejected next time doors open
        for (std::deque<MovableObject *>::const_iterator niItr = m_CollectedInventory.begin(); niItr != m_CollectedInventory.end(); ++niItr) {
            AddToInventoryBack(*niItr);
        }

        // Clear the new inventory hold, it's all been moved to the regular inventory
        m_CollectedInventory.clear();

        // PSCHHT
		if (m_HatchCloseSound) { m_HatchCloseSound->Play(m_Pos); }
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this Actor.
// Arguments:       An pointer to the new item to add. Ownership IS TRANSFERRED!
// Return value:    None..

void ACraft::AddInventoryItem(MovableObject *pItemToAdd)
{
    if (pItemToAdd)
    {
        // If the hatch is open, then only add the new item to the intermediate new inventory list
        // so that it doesn't get chucked out right away again
		if (m_HatchState == OPEN || m_HatchState == OPENING) {
			m_CollectedInventory.push_back(pItemToAdd);
		} else {
			// If doors are already closed, it's safe to put the item directly the regular inventory
			AddToInventoryBack(pItemToAdd);
		}
		if (Actor *itemAsActor = dynamic_cast<Actor*>(pItemToAdd); itemAsActor && itemAsActor->GetGoldCarried() > 0) {
			m_GoldCarried += itemAsActor->GetGoldCarried();
			itemAsActor->SetGoldCarried(0);
			m_GoldPicked = true;
			if (g_ActivityMan.GetActivity()->IsHumanTeam(m_Team)) {
				for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (g_ActivityMan.GetActivity()->GetTeamOfPlayer(player) == m_Team) { g_GUISound.FundsChangedSound()->Play(player); }
				}
			}
		}
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DropAllInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the hatches and makes everything in the Rocket fly out, including
//                  the passenger Actors, one after another. It may not happen
//                  instantaneously, so check for ejection being complete with
//                  IsInventoryEmpty().

void ACraft::DropAllInventory()
{
    if (m_HatchState == OPEN && !m_Exits.empty())
    {
        // Cancel if we're not due to release
        if (!m_ExitTimer.IsPastSimMS(m_ExitInterval))
            return;

        bool exitExists = false;
        std::list<Exit>::iterator exit = m_Exits.begin();
        // Check which exits are clear of the terrain, if any
        for (; exit != m_Exits.end(); ++exit)
        {
            if (exit->CheckIfClear(m_Pos, m_Rotation, 18))
                exitExists = true;
        }

        // Cancel if no exits are clear
        if (!exitExists)
        {
// TODO: give some kind of audio feedback to user
            return;
        }

        // Eject inventory and passengers, through alternating clear exits
        Vector exitVel, exitVelNorm, gravityNorm;
        float antiGravBoost = 0;
        Actor *pPassenger = 0;
        bool droppedSomething = false;
        for (std::deque<MovableObject *>::iterator exitee = m_Inventory.begin(); exitee != m_Inventory.end(); ++exitee)
        {
            // Select next clear exit
            do
            {
                if (++m_CurrentExit == m_Exits.end())
                    m_CurrentExit = m_Exits.begin();
            }
            while (!m_CurrentExit->IsClear());

            //(*exitee)->SetPos(m_Pos + m_CurrentExit->GetOffset() * m_Rotation);
            (*exitee)->SetPos(m_Pos + RotateOffset(m_CurrentExit->GetOffset()));
            // Reset all the timers of the object being shot out so it doesn't emit a bunch of particles that have been backed up while dormant in inventory
            (*exitee)->ResetAllTimers();
            //exitVel = m_CurrentExit->GetVelocity() * m_Rotation;
            exitVel = RotateOffset(m_CurrentExit->GetVelocity());

            // Boost against gravity
            // Normalize the gravity and exit velocity vectors
            exitVelNorm = exitVel;
            exitVelNorm.SetMagnitude(1.0);
            gravityNorm = g_SceneMan.GetGlobalAcc();
            gravityNorm.SetMagnitude(1.0);
            // Make the gravity boost be proportional to counteract gravity
            antiGravBoost = 1.0 + 1.0 * exitVelNorm.Dot(-gravityNorm);
            if (antiGravBoost < 1.0)
                antiGravBoost = 1.0;

            // Detect whether we're dealing with a passenger and add it as Actor instead
            pPassenger = dynamic_cast<Actor *>(*exitee);
            if (pPassenger && m_ExitTimer.IsPastSimMS(m_ExitInterval))
            {
                pPassenger->SetVel(m_Vel + exitVel * antiGravBoost);
//                pPassenger->SetRotAngle(m_Rotation + exitVel.GetAbsRadAngle() (ejectDir > 0 ? -c_HalfPI : c_HalfPI));
//                pPassenger->SetHFlipped(ejectDir <= 0);
                // Avoid having immediate collisions with this
                pPassenger->SetWhichMOToNotHit(this, 0.5f);
                // Avoid this immediate collisions with it
                SetWhichMOToNotHit(pPassenger, 0.5f);
                // Add to scene
                g_MovableMan.AddActor(pPassenger);

                // If this craft is being directly controlled by a player, and has landed, switch control to the first guy out
                if (pPassenger->GetTeam() == m_Team && m_Controller.IsPlayerControlled() && g_ActivityMan.GetActivity()->GetControlledActor(m_Controller.GetPlayer()) == this && m_LandingCraft)
                {
                    g_ActivityMan.GetActivity()->SwitchToActor(pPassenger, m_Controller.GetPlayer(), m_Team);
                    // To avoid jump in the view, Update the passenger so its viewpoint is next to it and not at 0,0
                    pPassenger->Update();
                }

                // Remove from inventory
                m_Inventory.erase(exitee);
                // Reset timer interval and quit until next one is due
                m_ExitTimer.Reset();
                break;
            }
            else
            {
                (*exitee)->SetVel(m_Vel + exitVel * antiGravBoost);
                (*exitee)->SetAngularVel(5.0F * RandomNormalNum());
                // Avoid it having immediate collisions with this
                (*exitee)->SetWhichMOToNotHit(this, 0.5f);
                // Avoid this immediate collisions with it
                SetWhichMOToNotHit(*exitee, 0.5f);
                // Add to scene
                g_MovableMan.AddMO(*exitee);
                // Remove passenger from inventory
                m_Inventory.erase(exitee);
                // Reset timer interval and quit until next one is due
                m_ExitTimer.Reset();
                break;
            }
            droppedSomething = true;
        }

        if (m_Inventory.empty())
        {
            m_HasDelivered = true;

			// Kill craft if it is lying down.
			if (std::fabs(m_Rotation.GetRadAngle()) > c_HalfPI && m_Status != DYING) {
                m_Status = DYING;
                m_DeathTmr.Reset();
            }
            // Reset exit timer so we can time until when we should start sucking things in again
            // Don't reset it if inventory is empty but we didn't drop anyhting, it means that doors just opened and nothing was inside, so immediately start sucking
            if (droppedSomething)
                m_ExitTimer.Reset();
        }
    }
    else
    {
        if (m_HatchState != OPENING)
        {
			if (m_HatchOpenSound) { m_HatchOpenSound->Play(m_Pos); }
			g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, 0.4));
            m_HatchTimer.Reset();
        }
        m_HatchState = OPENING;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ACraft::GetCollectedInventoryMass() const {
	float inventoryMass = 0.0F;
	for (const MovableObject *inventoryItem : m_CollectedInventory) {
		inventoryMass += inventoryItem->GetMass();
	}
	return inventoryMass;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACraft::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore) {
	if (g_SettingsMan.CrabBombsEnabled() && !s_CrabBombInEffect) {
		s_CrabBombInEffect = true;
		int crabCount = 0;
		for (const MovableObject *inventoryEntry : m_Inventory) {
			if (inventoryEntry->GetPresetName() == "Crab") { crabCount++; }
		}
		if (crabCount >= g_SettingsMan.GetCrabBombThreshold()) {
			for (int moid = 1; moid < g_MovableMan.GetMOIDCount() - 1; moid++) {
				Actor *actor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(moid));
				if (actor && actor != this && actor->GetClassName() != "ADoor" && !actor->IsInGroup("Brains")) { actor->GibThis(); }
			}
		}
		s_CrabBombInEffect = false;
	}
	Actor::GibThis(impactImpulse, movableObjectToIgnore);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACraft::ResetAllTimers() {
	MOSRotating::ResetAllTimers();

	m_FlippedTimer.Reset();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ACraft. Supposed to be done every frame.

void ACraft::Update()
{
    Actor::Update();

    ////////////////////////////////////
    // Update viewpoint

    // Set viewpoint based on how we are aiming etc.
    m_ViewPoint = m_Pos.GetFloored();
	// Add velocity also so the viewpoint moves ahead at high speeds
	if (m_Vel.MagnitudeIsGreaterThan(10.0F)) { m_ViewPoint += m_Vel * std::sqrt(m_Vel.GetMagnitude() * 0.1F); }

    ///////////////////////////////////////////////////
    // Crash detection and handling
    const float crashSpeedThreshold = 1.0F;
    if (m_DeepHardness > 5 && m_Vel.MagnitudeIsGreaterThan(crashSpeedThreshold))
    {
        m_Health -= m_DeepHardness * 0.03;
// TODO: HELLA GHETTO, REWORK
        if (m_CrashTimer.GetElapsedSimTimeMS() > 500)
        {
			if (m_CrashSound) { m_CrashSound->Play(m_Pos); } 
            m_CrashTimer.Reset();
        }
    }

    ///////////////////////////////////////////////////
    // Doors open logic

    if (m_HatchState == OPEN)
    {
        // Keep ejecting things if hatch is open and inventory isn't empty
        if (!IsInventoryEmpty())
            DropAllInventory();
        // If doors are open and all inventory is ejected (and they have had a chance to fall away), then actively look for things to suck in through the exits.
        else if (m_Status == STABLE && m_ExitTimer.IsPastSimMS(EXITSUCKDELAYMS))
        {
            // See if any of the exits have sucked in an MO
            for (std::list<Exit>::iterator exit = m_Exits.begin(); exit != m_Exits.end(); ++exit)
            {
                // If exit sucked in an MO, add it to invetory
                MOSRotating *pNewObject = exit->SuckInMOs(this);
                if (pNewObject && !pNewObject->IsSetToDelete())
                {
                    // Did we catch an Actor? If so, we need to do special controller switching in case it's player controlled atm
                    Actor *pCaughtActor = 0;
                    if (pCaughtActor = dynamic_cast<Actor *>(pNewObject))
                    {
                        // Switch control to this craft if the Actor we just caught is on our team and currently player controlled
                        // Set AI controller of the Actor going into the ship
                        if (pCaughtActor->GetTeam() == m_Team && g_ActivityMan.GetActivity() && pCaughtActor->GetController()->IsPlayerControlled())
                            g_ActivityMan.GetActivity()->SwitchToActor(this, pCaughtActor->GetController()->GetPlayer(), pCaughtActor->GetTeam());
                        // Add (copy) of caught Actor to this' inventory
                        AddInventoryItem(dynamic_cast<MovableObject *>(pCaughtActor->Clone()));
                        // Delete the original from scene - this is safer than 'removing' or handing over ownership halfway through MovableMan's update
                        pCaughtActor->SetToDelete();
                        // Negate the team 'loss' that will be reported when the deletion is done
                        g_ActivityMan.GetActivity()->ReportDeath(pCaughtActor->GetTeam(), -1);
                    }
                    // Any other MO has been caught, jsut add it to inventory
// TODO: do we need special case for Attachables?? (so we can detach them first? - probably no because SuckInMOs is unlikely to return an attahced thing)
                    else
                    {
                        // Add (copy) of caught Actor to this' inventory
                        AddInventoryItem(dynamic_cast<MovableObject *>(pNewObject->Clone()));
                        // Delete the original from scene - this is safer than 'removing' or handing over ownership halfway through MovableMan's update
                        pNewObject->SetToDelete();
                    }
                    pNewObject = 0;
                }
            }
        }
    }

    /////////////////////////////////////////
    // Check for having gone into orbit

    if (m_Pos.m_Y < -m_CharHeight || m_Pos.m_Y > g_SceneMan.GetSceneHeight() + m_CharHeight)
    {
        g_ActivityMan.GetActivity()->HandleCraftEnteringOrbit(this);
        // Play fading away thruster sound
//        if (m_pMThruster && m_pMThruster->IsEmitting())
//            m_pMThruster->(pTargetBitmap, targetPos, mode, onlyPhysical);
        m_ToDelete = true;
    }

	if (g_ActivityMan.GetActivity()->GetCraftOrbitAtTheEdge())
	{
		if (g_SceneMan.GetScene() && !g_SceneMan.GetScene()->WrapsX())
		{
			if (m_Pos.m_X < -GetSpriteWidth() || m_Pos.m_X > g_SceneMan.GetSceneWidth() + GetSpriteWidth())
			{
				g_ActivityMan.GetActivity()->HandleCraftEnteringOrbit(this);
				m_ToDelete = true;
			}
		}
	}

	if (m_Status == DEAD) {
		if (m_ScuttleOnDeath || m_AIMode == AIMODE_SCUTTLE) { GibThis(); }
	} else if (m_Status == DYING) {
		if ((m_ScuttleOnDeath || m_AIMode == AIMODE_SCUTTLE) && m_DeathTmr.IsPastSimMS(500) && m_DeathTmr.AlternateSim(100)) { FlashWhite(10); }
	}  else if (m_Health <= 0 || m_AIMode == AIMODE_SCUTTLE || (m_ScuttleIfFlippedTime >= 0 && m_FlippedTimer.IsPastSimMS(m_ScuttleIfFlippedTime))) {
		m_Status = DYING;
		m_DeathTmr.Reset();
		DropAllInventory();
	}

	// Get the rotation in radians.
	float rot = m_Rotation.GetRadAngle();

	// Eliminate rotations over half a turn.
	if (std::abs(rot) > c_PI) {
		rot += (rot > 0) ? -c_TwoPI : c_TwoPI;
		m_Rotation.SetRadAngle(rot);
	}
	if (rot < c_HalfPI && rot > -c_HalfPI) {
		m_FlippedTimer.Reset();
	}

    /////////////////////////////////////////
    // Misc.

    m_DeepCheck = true/*m_Status == DEAD*/;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void ACraft::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
	m_HUDStack = -m_CharHeight / 2;

    // Only do HUD if on a team
    if (m_Team < 0) {
        return;
    }
	
	// Only draw if the team viewing this is on the same team OR has seen the space where this is located.
	int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
	if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && (!g_SettingsMan.ShowEnemyHUD() || g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam))) {
		return;
	}

    Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);

	if (!m_HUDVisible) {
		return;
	}

    GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();

    // Draw hud guides for the Exits, depending on whether the doors are open
    if (m_HatchState == OPEN) {
        // Doors open and inventory not empty yet, so show arrows pointing out of the exits since things are still coming out
        if (!IsInventoryEmpty()) {
            //  --------
            //  |  \  \
            // -+-  |  |
            //  |  /  /
            //  --------
            // Make the dotted lines crawl out of the exit, indicating that things are still coming out
            if (--m_ExitLinePhase < 0) {
                m_ExitLinePhase = EXITLINESPACING - 1;
            }
        }
        // Inventory empty and doors open, so show arrows pointing into the exits IF the delay to allow for things to eject away all the way has passed
        else if (m_ExitTimer.IsPastSimMS(EXITSUCKDELAYMS)) {
            // Make the dotted lines crawl back into the exit, inviting people to jump in
            if (++m_ExitLinePhase >= EXITLINESPACING) {
                m_ExitLinePhase = 0;
            }
        }

        // Draw the actual dotted lines
        for (std::list<Exit>::iterator exit = m_Exits.begin(); exit != m_Exits.end(); ++exit) {
            if (!exit->CheckIfClear(m_Pos, m_Rotation, 18)) {
                continue;
            }

            Vector exitRadius = RotateOffset(exit->GetVelocity().GetPerpendicular().SetMagnitude(exit->GetRadius()));
            Vector exitCorner = m_Pos - targetPos + RotateOffset(exit->GetOffset()) + exitRadius;
            Vector arrowVec = RotateOffset(exit->GetVelocity().SetMagnitude(exit->GetRange()));
            g_FrameMan.DrawLine(pTargetBitmap, exitCorner, exitCorner + arrowVec, 120, 120, EXITLINESPACING, m_ExitLinePhase);
            exitCorner -= exitRadius * 2;
            g_FrameMan.DrawLine(pTargetBitmap, exitCorner, exitCorner + arrowVec, 120, 120, EXITLINESPACING, m_ExitLinePhase);
        }
    }
}

} // namespace RTE