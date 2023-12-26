//////////////////////////////////////////////////////////////////////////////////////////
// File:            Actor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Actor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"

#include "UInputMan.h"
#include "ActivityMan.h"
#include "CameraMan.h"
#include "Singleton.h"
#include "GameActivity.h"
#include "ACrab.h"
#include "ACraft.h"
#include "AtomGroup.h"
#include "Controller.h"
#include "RTETools.h"
#include "SceneMan.h"
#include "HeldDevice.h"
#include "PresetMan.h"
#include "AEmitter.h"
#include "Material.h"
#include "MOPixel.h"
#include "Scene.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "PerformanceMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "tracy/Tracy.hpp"

namespace RTE {

ConcreteClassInfo(Actor, MOSRotating, 20);

std::vector<BITMAP *> Actor::m_apNoTeamIcon;
BITMAP *Actor::m_apAIIcons[AIMODE_COUNT];
std::vector<BITMAP *> Actor::m_apSelectArrow;
std::vector<BITMAP *> Actor::m_apAlarmExclamation;
bool Actor::m_sIconsLoaded = false;

#define ARROWTIME 1000



//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     LuaBindRegister
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registration function for exposing this' members to a LuaBind module.


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Actor, effectively
//                  resetting the members of this abstraction level only.

void Actor::Clear() {
    m_Controller.Reset();
	m_PlayerControllable = true;
    m_BodyHitSound = nullptr;
    m_AlarmSound = nullptr;
    m_PainSound = nullptr;
    m_DeathSound = nullptr;
    m_DeviceSwitchSound = nullptr;
    m_Status = STABLE;
    m_Health = m_PrevHealth = m_MaxHealth = 100.0F;
	m_pTeamIcon = nullptr;
	m_pControllerIcon = nullptr;
    m_LastSecondTimer.Reset();
    m_LastSecondPos.Reset();
    m_RecentMovement.Reset();
    m_TravelImpulseDamage = 750.0F;
    m_StableVel.SetXY(15.0F, 25.0F);
	m_StableRecoverDelay = 1000;
    m_HeartBeat.Reset();
    m_NewControlTmr.Reset();
    m_DeathTmr.Reset();
	m_GoldCarried = 0;
    m_GoldPicked = false;
    m_AimState = AIMSTILL;
	m_AimAngle = 0;
    m_AimRange = c_HalfPI;
	m_AimDistance = 0;
    m_AimTmr.Reset();
    m_SharpAimTimer.Reset();
    m_SharpAimDelay = 250;
	m_SharpAimProgress = 0;
    m_SharpAimMaxedOut = false;
    m_PointingTarget.Reset();
    m_SeenTargetPos.Reset();
    // Set the limit to soemthing reasonable, if the timer is over it, there's no alarm
    m_AlarmTimer.SetSimTimeLimitMS(3000);
    m_AlarmTimer.SetElapsedSimTimeMS(4000);
    m_LastAlarmPos.Reset();
    m_SightDistance = 450.0F;
    m_Perceptiveness = 0.5F;
    m_PainThreshold = 15.0F;
	m_CanRevealUnseen = true;
    m_CharHeight = 0;
    m_HolsterOffset.Reset();
	m_ReloadOffset.Reset();
    m_ViewPoint.Reset();
    m_Inventory.clear();
	m_MaxInventoryMass = -1.0F;
	m_pItemInReach = nullptr;
    m_HUDStack = 0;
	m_DeploymentID = 0;
    m_PassengerSlots = 1;

    m_AIMode = AIMODE_NONE;
    m_Waypoints.clear();
    m_DrawWaypoints = false;
    m_MoveTarget.Reset();
	m_pMOMoveTarget = nullptr;
    m_PrevPathTarget.Reset();
    m_MoveVector.Reset();
    m_MovePath.clear();
    m_UpdateMovePath = true;
    m_MoveProximityLimit = 20.0F;
    m_AIBaseDigStrength = c_PathFindingDefaultDigStrength;
    m_BaseMass = std::numeric_limits<float>::infinity();

    m_DamageMultiplier = 1.0F;

	m_Organic = false;
	m_Mechanical = false;

	m_LimbPushForcesAndCollisionsDisabled = false;

	m_PieMenu.reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Actor object ready for use.

int Actor::Create()
{
    if (MOSRotating::Create() < 0) {
        return -1;
    }

    // Set MO Type.
    m_MOType = MovableObject::TypeActor;

    // Default to an interesting AI controller mode
    m_Controller.SetInputMode(Controller::CIM_AI);
    m_Controller.SetControlledActor(this);
    m_UpdateMovePath = true;

    m_ViewPoint = m_Pos;
    m_HUDStack = -m_CharHeight / 2;

    // Sets up the team icon
    SetTeam(m_Team);

    if (const Actor *presetActor = static_cast<const Actor *>(GetPreset())) {
        m_BaseMass = presetActor->GetMass();
    }

	// All brain actors by default avoid hitting each other on the same team
	if (IsInGroup("Brains")) {
		m_IgnoresTeamHits = true;
    }

	if (!m_PieMenu) {
		SetPieMenu(static_cast<PieMenu *>(g_PresetMan.GetEntityPreset("PieMenu", GetDefaultPieMenuName())->Clone()));
	} else {
		m_PieMenu->SetOwner(this);
	}

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Actor to be identical to another, by deep copy.

int Actor::Create(const Actor &reference)
{
    MOSRotating::Create(reference);

    // Set MO Type.
    m_MOType = MovableObject::TypeActor;

    m_Controller = reference.m_Controller;
    m_Controller.SetInputMode(Controller::CIM_AI);
    m_Controller.SetControlledActor(this);
	m_PlayerControllable = reference.m_PlayerControllable;

	if (reference.m_BodyHitSound) { m_BodyHitSound = dynamic_cast<SoundContainer *>(reference.m_BodyHitSound->Clone()); }
	if (reference.m_AlarmSound) { m_AlarmSound = dynamic_cast<SoundContainer*>(reference.m_AlarmSound->Clone()); }
	if (reference.m_PainSound) { m_PainSound = dynamic_cast<SoundContainer *>(reference.m_PainSound->Clone()); }
	if (reference.m_DeathSound) { m_DeathSound = dynamic_cast<SoundContainer *>(reference.m_DeathSound->Clone()); }
	if (reference.m_DeviceSwitchSound) { m_DeviceSwitchSound = dynamic_cast<SoundContainer *>(reference.m_DeviceSwitchSound->Clone()); }
//    m_FacingRight = reference.m_FacingRight;
    m_Status = reference.m_Status;
    m_Health = m_PrevHealth = reference.m_Health;
	m_MaxHealth = reference.m_MaxHealth;
    m_pTeamIcon = reference.m_pTeamIcon;
//    m_LastSecondTimer.Reset();
//    m_LastSecondPos.Reset();
//    m_RecentMovement.Reset();
    m_LastSecondPos = reference.m_LastSecondPos;
    m_TravelImpulseDamage = reference.m_TravelImpulseDamage;
    m_StableVel = reference.m_StableVel;
    m_StableRecoverDelay = reference.m_StableRecoverDelay;
    m_GoldCarried = reference.m_GoldCarried;
    m_AimState = reference.m_AimState;
    m_AimRange = reference.m_AimRange;
    m_AimAngle = reference.m_AimAngle;
    m_AimDistance = reference.m_AimDistance;
    m_SharpAimDelay = reference.m_SharpAimDelay;
    m_SharpAimProgress = reference.m_SharpAimProgress;
    m_PointingTarget = reference.m_PointingTarget;
    m_SeenTargetPos = reference.m_SeenTargetPos;
    m_SightDistance = reference.m_SightDistance;
    m_Perceptiveness = reference.m_Perceptiveness;
    m_PainThreshold = reference.m_PainThreshold;
	m_CanRevealUnseen = reference.m_CanRevealUnseen;
    m_CharHeight = reference.m_CharHeight;
    m_HolsterOffset = reference.m_HolsterOffset;
	m_ReloadOffset = reference.m_ReloadOffset;

    for (std::deque<MovableObject*>::const_iterator itr = reference.m_Inventory.begin(); itr != reference.m_Inventory.end(); ++itr) {
        m_Inventory.push_back(dynamic_cast<MovableObject*>((*itr)->Clone()));
    }

    m_MaxInventoryMass = reference.m_MaxInventoryMass;

    // Only load the static AI mode icons once
    if (!m_sIconsLoaded)
    {
        ContentFile("Base.rte/GUIs/TeamIcons/NoTeam.png").GetAsAnimation(m_apNoTeamIcon, 2);

        ContentFile iconFile("Base.rte/GUIs/PieMenus/PieIcons/Blank000.png");
        m_apAIIcons[AIMODE_NONE] = iconFile.GetAsBitmap();
        m_apAIIcons[AIMODE_BOMB] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Eye000.png");
        m_apAIIcons[AIMODE_SENTRY] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Cycle000.png");
        m_apAIIcons[AIMODE_PATROL] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/GoTo000.png");
        m_apAIIcons[AIMODE_GOTO] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Brain000.png");
        m_apAIIcons[AIMODE_BRAINHUNT] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Dig000.png");
        m_apAIIcons[AIMODE_GOLDDIG] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Return000.png");
        m_apAIIcons[AIMODE_RETURN] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Land000.png");
        m_apAIIcons[AIMODE_STAY] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Launch000.png");
        m_apAIIcons[AIMODE_DELIVER] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Death000.png");
        m_apAIIcons[AIMODE_SCUTTLE] = iconFile.GetAsBitmap();
        iconFile.SetDataPath("Base.rte/GUIs/PieMenus/PieIcons/Follow000.png");
        m_apAIIcons[AIMODE_SQUAD] = iconFile.GetAsBitmap();

        ContentFile("Base.rte/GUIs/Indicators/SelectArrow.png").GetAsAnimation(m_apSelectArrow, 4);
        ContentFile("Base.rte/GUIs/Indicators/AlarmExclamation.png").GetAsAnimation(m_apAlarmExclamation, 2);

        m_sIconsLoaded = true;
    }
	m_DeploymentID = reference.m_DeploymentID;
    m_PassengerSlots = reference.m_PassengerSlots;

    m_AIMode = reference.m_AIMode;
    m_Waypoints = reference.m_Waypoints;
    m_DrawWaypoints = reference.m_DrawWaypoints;
    m_MoveTarget = reference.m_MoveTarget;
    m_pMOMoveTarget = reference.m_pMOMoveTarget;
    m_PrevPathTarget = reference.m_PrevPathTarget;
    m_MoveVector = reference.m_MoveVector;
	if (!m_Waypoints.empty()) {
		UpdateMovePath();
	}
//    m_MovePath.clear(); will recalc on its own
    m_UpdateMovePath = reference.m_UpdateMovePath;
    m_MoveProximityLimit = reference.m_MoveProximityLimit;
    m_AIBaseDigStrength = reference.m_AIBaseDigStrength;
    m_BaseMass = reference.m_BaseMass;

	m_Organic = reference.m_Organic;
	m_Mechanical = reference.m_Mechanical;

	m_LimbPushForcesAndCollisionsDisabled = reference.m_LimbPushForcesAndCollisionsDisabled;

	RTEAssert(reference.m_PieMenu != nullptr, "Tried to clone actor with no pie menu.");
	SetPieMenu(static_cast<PieMenu *>(reference.m_PieMenu->Clone()));
	m_PieMenu->AddWhilePieMenuOpenListener(this, std::bind(&Actor::WhilePieMenuOpenListener, this, m_PieMenu.get()));

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Actor::ReadProperty(const std::string_view &propName, Reader &reader)
{
    StartPropertyList(return MOSRotating::ReadProperty(propName, reader));

	MatchProperty("PlayerControllable", { reader >> m_PlayerControllable; });
	MatchProperty("BodyHitSound", {
		m_BodyHitSound = new SoundContainer;
		reader >> m_BodyHitSound;
	});
	MatchProperty("AlarmSound", {
		m_AlarmSound = new SoundContainer;
		reader >> m_AlarmSound;
	});
	MatchProperty("PainSound", {
		m_PainSound = new SoundContainer;
		reader >> m_PainSound;
	});
	MatchProperty("DeathSound", {
		m_DeathSound = new SoundContainer;
		reader >> m_DeathSound;
	});
	MatchProperty("DeviceSwitchSound", {
		m_DeviceSwitchSound = new SoundContainer;
		reader >> m_DeviceSwitchSound;
	});
	MatchProperty("Status", { reader >> m_Status; });
    MatchProperty("DeploymentID", { reader >> m_DeploymentID; });
    MatchProperty("PassengerSlots", { reader >> m_PassengerSlots; });
    MatchProperty("Health",
    {
        reader >> m_Health;
        m_PrevHealth = m_Health;
		if (m_Health > m_MaxHealth)
			m_MaxHealth = m_Health;
    });
    MatchProperty("MaxHealth",
	{
		reader >> m_MaxHealth;
		if (m_MaxHealth < m_Health)
		{
			m_Health = m_MaxHealth;
	        m_PrevHealth = m_Health;
		}
	});
    MatchProperty("ImpulseDamageThreshold", { reader >> m_TravelImpulseDamage; });
    MatchProperty("StableVelocityThreshold", { reader >> m_StableVel; });
    MatchProperty("StableRecoveryDelay", { reader >> m_StableRecoverDelay; });
    MatchProperty("AimAngle", { reader >> m_AimAngle; });
    MatchProperty("AimRange", { reader >> m_AimRange; });
    MatchProperty("AimDistance", { reader >> m_AimDistance; });
    MatchProperty("SharpAimDelay", { reader >> m_SharpAimDelay; });
    MatchProperty("SightDistance", { reader >> m_SightDistance; });
    MatchProperty("Perceptiveness", { reader >> m_Perceptiveness; });
    MatchProperty("PainThreshold", { reader >> m_PainThreshold; });
	MatchProperty("CanRevealUnseen", { reader >> m_CanRevealUnseen; });
    MatchProperty("CharHeight", { reader >> m_CharHeight; });
    MatchProperty("HolsterOffset", { reader >> m_HolsterOffset; });
	MatchProperty("ReloadOffset", { reader >> m_ReloadOffset; });
    MatchForwards("AddInventoryDevice") MatchProperty("AddInventory",
    {
        MovableObject *pInvMO = dynamic_cast<MovableObject *>(g_PresetMan.ReadReflectedPreset(reader));
		if (!pInvMO) { reader.ReportError("Object added to inventory is broken."); }
        AddToInventoryBack(pInvMO);
    });
    MatchProperty("MaxInventoryMass", { reader >> m_MaxInventoryMass; });
	MatchProperty("AIMode", {
		int mode;
		reader >> mode;
		m_AIMode = static_cast<AIMode>(mode);
	});
	MatchProperty("SpecialBehaviour_AddAISceneWaypoint", {
		Vector waypointToAdd;
		reader >> waypointToAdd;
		AddAISceneWaypoint(waypointToAdd);
	});
	MatchProperty("PieMenu", {
		m_PieMenu = std::unique_ptr<PieMenu>(dynamic_cast<PieMenu *>(g_PresetMan.ReadReflectedPreset(reader)));
		if (!m_PieMenu) { reader.ReportError("Failed to set Actor's pie menu. Doublecheck your name and everything is correct."); }
		m_PieMenu->Create(this);
    });
    MatchProperty("Organic", { reader >> m_Organic; });
    MatchProperty("Mechanical", { reader >> m_Mechanical; });
    MatchProperty("AIBaseDigStrength", { reader >> m_AIBaseDigStrength; });
    

    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Actor with a Writer for
//                  later recreation with Create(Reader &reader);

int Actor::Save(Writer &writer) const
{
    MOSRotating::Save(writer);

	writer.NewPropertyWithValue("PlayerControllable", m_PlayerControllable);
    writer.NewProperty("BodyHitSound");
    writer << m_BodyHitSound;
    writer.NewProperty("AlarmSound");
    writer << m_AlarmSound;
    writer.NewProperty("PainSound");
    writer << m_PainSound;
    writer.NewProperty("DeathSound");
    writer << m_DeathSound;
    writer.NewProperty("DeviceSwitchSound");
    writer << m_DeviceSwitchSound;
    writer.NewProperty("Status");
    writer << m_Status;
    writer.NewProperty("Health");
    writer << m_Health;
    writer.NewProperty("MaxHealth");
    writer << m_MaxHealth;
	if (m_DeploymentID)
	{
		writer.NewProperty("DeploymentID");
		writer << m_DeploymentID;
	}
	writer.NewProperty("ImpulseDamageThreshold");
    writer << m_TravelImpulseDamage;
    writer.NewProperty("StableVelocityThreshold");
    writer << m_StableVel;
    writer.NewProperty("StableRecoveryDelay");
    writer << m_StableRecoverDelay;
    writer.NewProperty("AimAngle");
    writer << m_AimAngle;
    writer.NewProperty("AimRange");
    writer << m_AimRange;
    writer.NewProperty("AimDistance");
    writer << m_AimDistance;
    writer.NewProperty("SharpAimDelay");
    writer << m_SharpAimDelay;
    writer.NewProperty("SightDistance");
    writer << m_SightDistance;
    writer.NewProperty("Perceptiveness");
    writer << m_Perceptiveness;
    writer.NewProperty("PainThreshold");
    writer << m_PainThreshold;
	writer.NewProperty("CanRevealUnseen");
	writer << m_CanRevealUnseen;
    writer.NewProperty("CharHeight");
    writer << m_CharHeight;
    writer.NewProperty("HolsterOffset");
    writer << m_HolsterOffset;
	writer.NewPropertyWithValue("ReloadOffset", m_ReloadOffset);
    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        writer.NewProperty("AddInventory");
        writer << **itr;
    }
    writer.NewProperty("MaxInventoryMass");
    writer << m_MaxInventoryMass;
    writer.NewProperty("AIMode");
    writer << m_AIMode;
    writer.NewProperty("PieMenu");
    writer << m_PieMenu.get();

	writer.NewPropertyWithValue("Organic", m_Organic);
    writer.NewPropertyWithValue("Mechanical", m_Mechanical);
    writer.NewPropertyWithValue("AIBaseDigStrength", m_AIBaseDigStrength);

    return 0;
}

void Actor::DestroyScriptState() {
    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
        (*itr)->DestroyScriptState();
    }

    MOSRotating::DestroyScriptState();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Actor object.

void Actor::Destroy(bool notInherited)
{
	delete m_DeviceSwitchSound;
	delete m_BodyHitSound;
	delete m_PainSound;
	delete m_DeathSound;
	delete m_AlarmSound;

    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr) {
        delete (*itr);
    }

    if (!notInherited) {
        MOSRotating::Destroy();
    }
    
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Actor::GetInventoryMass() const {
    float inventoryMass = 0.0F;
    for (const MovableObject *inventoryItem : m_Inventory) {
        inventoryMass += inventoryItem->GetMass();
    }
    return inventoryMass;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Actor::GetBaseMass() {
    if (m_BaseMass == std::numeric_limits<float>::infinity()) {
        if (const Actor* presetActor = static_cast<const Actor*>(GetPreset())) {
            m_BaseMass = presetActor->GetMass();
        } else {
            m_BaseMass = GetMass();
        }
    }

    return m_BaseMass;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlayerControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a player is currently controlling this.

bool Actor::IsPlayerControlled() const
{
    return m_Controller.GetInputMode() == Controller::CIM_PLAYER && m_Controller.GetPlayer() >= 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this Actor and all its carried
//                  gold and inventory.

float Actor::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const
{
	float totalValue = (GetGoldValue(nativeModule, foreignMult, nativeMult) / 2) + ((GetGoldValue(nativeModule, foreignMult, nativeMult) / 2) * (GetHealth() / GetMaxHealth()));
    totalValue += GetGoldCarried();

    MOSprite *pItem = 0;
    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
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

bool Actor::HasObject(std::string objectName) const
{
    if (MOSRotating::HasObject(objectName))
        return true;

    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
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

bool Actor::HasObjectInGroup(std::string groupName) const
{
    if (MOSRotating::HasObjectInGroup(groupName))
        return true;

    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        if ((*itr) && (*itr)->HasObjectInGroup(groupName))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this Actor belongs to.

void Actor::SetTeam(int team)
{
	MovableObject::SetTeam(team);

    // Change the Team Icon to display
    m_pTeamIcon = 0;
    if (g_ActivityMan.GetActivity())
        m_pTeamIcon = g_ActivityMan.GetActivity()->GetTeamIcon(m_Team);

    // Also set all actors in the inventory
    Actor *pActor = 0;
    for (std::deque<MovableObject *>::const_iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pActor = dynamic_cast<Actor *>(*itr);
        if (pActor)
            pActor->SetTeam(team);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetControllerMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's new Controller input mode.

void Actor::SetControllerMode(Controller::InputMode newMode, int newPlayer)
{

	Controller::InputMode previousControllerMode = m_Controller.GetInputMode();
	int previousControllingPlayer = m_Controller.GetPlayer();

    m_Controller.SetInputMode(newMode);
    m_Controller.SetPlayer(newPlayer);

	RunScriptedFunctionInAppropriateScripts("OnControllerInputModeChange", false, false, {}, {std::to_string(previousControllerMode), std::to_string(previousControllingPlayer) });

    m_NewControlTmr.Reset();
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapControllerModes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this Actor's Controller mode and gives back what it used to be.

Controller::InputMode Actor::SwapControllerModes(Controller::InputMode newMode, int newPlayer)
{
    Controller::InputMode returnMode = m_Controller.GetInputMode();
	SetControllerMode(newMode, newPlayer);
    return returnMode;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.

bool Actor::Look(float FOVSpread, float range)
{
    if (!g_SceneMan.AnythingUnseen(m_Team) || m_CanRevealUnseen == false)
        return false;

    // Use the 'eyes' on the 'head', if applicable
    Vector aimPos = GetEyePos();
/*
    Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
    aimMatrix.SetXFlipped(m_HFlipped);
    // Get the langth of the look vector
    Vector aimDistance = m_ViewPoint - aimPos;
    // Add half the screen width
    Vector lookVector(fabs(aimDistance.m_X) + range, 0);
    // Set the rotation to the acutal aiming angle
    lookVector *= aimMatrix;
    // Add the spread
    lookVector.DegRotate(FOVSpread * NormalRand());
// TEST: Really need so far?
    lookVector /= 2;
*/
    Vector lookVector = m_Vel;
    // If there is no vel, just look in all directions
    if (lookVector.GetLargest() < 0.01)
    {
        lookVector.SetXY(range, 0);
		lookVector.DegRotate(RandomNum(-180.0F, 180.0F));
    }
    else
    {
        // Set the distance in the look direction
        lookVector.SetMagnitude(range);
        // Add the spread from the directed look
        lookVector.DegRotate(FOVSpread * RandomNormalNum());
    }

	Vector ignored;
    return g_SceneMan.CastSeeRay(m_Team, aimPos, lookVector, ignored, 25, g_SceneMan.GetUnseenResolution(m_Team).GetSmallest() / 2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Actor::AddGold(float goldOz) {
	bool isHumanTeam = g_ActivityMan.GetActivity()->IsHumanTeam(m_Team);
	if (g_SettingsMan.GetAutomaticGoldDeposit() || !isHumanTeam) {
		// TODO: Allow AI to reliably deliver gold via craft
		g_ActivityMan.GetActivity()->ChangeTeamFunds(goldOz, m_Team);
	} else {
		m_GoldCarried += goldOz;
		m_GoldPicked = true;
		if (isHumanTeam) {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				if (g_ActivityMan.GetActivity()->GetTeamOfPlayer(player) == m_Team) { g_GUISound.FundsChangedSound()->Play(player); }
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Actor::RestDetection() {
	MOSRotating::RestDetection();

	if (m_Status != DEAD) {
		m_AngOscillations = 0;
		m_VelOscillations = 0;
		m_RestTimer.Reset();
		m_ToSettle = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddAIMOWaypoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an MO in the scene as the next waypoint for this to go to, in order

void Actor::AddAIMOWaypoint(const MovableObject *pMOWaypoint)
{
    if (g_MovableMan.ValidMO(pMOWaypoint))
        m_Waypoints.push_back(std::pair<Vector, const MovableObject *>(pMOWaypoint->GetPos(), pMOWaypoint));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapNextInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Swaps the next MovableObject carried by this Actor and puts one not
//                  currently carried into the into the back of the inventory of this.

MovableObject * Actor::SwapNextInventory(MovableObject *pSwapIn, bool muteSound)
{
    MovableObject *pRetDev = 0;
    bool playSound = false;
    if (!m_Inventory.empty()) {
        pRetDev = m_Inventory.front();
        // Reset all the timers of the object being taken out of inventory so it doesn't emit a bunch of particles that have been backed up while dormant in inventory
        pRetDev->ResetAllTimers();
        m_Inventory.pop_front();
        playSound = true;
    }
    if (pSwapIn)
    {
		pSwapIn->SetAsNoID();
        AddToInventoryBack(pSwapIn);
        playSound = true;
    }

    if (m_DeviceSwitchSound && playSound && !muteSound)
        m_DeviceSwitchSound->Play(m_Pos);

    return pRetDev;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Actor::RemoveInventoryItem(const std::string &moduleName, const std::string &presetName) {
	for (std::deque<MovableObject*>::iterator inventoryIterator = m_Inventory.begin(); inventoryIterator != m_Inventory.end(); ++inventoryIterator) {
		if ((moduleName.empty() || (*inventoryIterator)->GetModuleName() == moduleName) && (*inventoryIterator)->GetPresetName() == presetName) {
            (*inventoryIterator)->DestroyScriptState();
            delete (*inventoryIterator);
			m_Inventory.erase(inventoryIterator);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MovableObject * Actor::RemoveInventoryItemAtIndex(int inventoryIndex) {
    if (inventoryIndex >= 0 && inventoryIndex < m_Inventory.size()) {
        MovableObject *itemAtIndex = m_Inventory[inventoryIndex];
        m_Inventory.erase(m_Inventory.begin() + inventoryIndex);
        return itemAtIndex;
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwapPrevInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Swaps the prev MovableObject carried by this Actor and puts one not
//                  currently carried into the into the back of the inventory of this.

MovableObject * Actor::SwapPrevInventory(MovableObject *pSwapIn)
{
    MovableObject *pRetDev = 0;
    bool playSound = false;
    if (!m_Inventory.empty()) {
        pRetDev = m_Inventory.back();
        m_Inventory.pop_back();
        playSound = true;
    }
    if (pSwapIn)
    {
		pSwapIn->SetAsNoID();
        AddToInventoryFront(pSwapIn);
        playSound = true;
    }

    if (m_DeviceSwitchSound && playSound)
        m_DeviceSwitchSound->Play(m_Pos);

    return pRetDev;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Actor::SwapInventoryItemsByIndex(int inventoryIndex1, int inventoryIndex2) {
    if (inventoryIndex1 < 0 || inventoryIndex2 < 0 || inventoryIndex1 >= m_Inventory.size() || inventoryIndex2 >= m_Inventory.size()) {
        return false;
    }

    std::swap(m_Inventory.at(inventoryIndex1), m_Inventory.at(inventoryIndex2));
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MovableObject * Actor::SetInventoryItemAtIndex(MovableObject *newInventoryItem, int inventoryIndex) {
    if (!newInventoryItem) {
        return RemoveInventoryItemAtIndex(inventoryIndex);
    }
	newInventoryItem->SetAsNoID();

    if (inventoryIndex < 0 || inventoryIndex >= m_Inventory.size()) {
        AddToInventoryBack(newInventoryItem);
        return nullptr;
    }
    MovableObject *currentInventoryItemAtIndex = m_Inventory.at(inventoryIndex);
    m_Inventory.at(inventoryIndex) = newInventoryItem;
    return currentInventoryItemAtIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DropAllInventory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Ejects all inventory items that this is carrying. It may not happen
//                  instantaneously, so check for ejection being complete with InventoryEmpty().

void Actor::DropAllInventory()
{
    MovableObject *pObject = 0;
    Actor *pPassenger = 0;
    float velMin, velMax, angularVel;
    Vector gibROffset, gibVel;
    for (std::deque<MovableObject *>::iterator gItr = m_Inventory.begin(); gItr != m_Inventory.end(); ++gItr)
    {
        // Get handy handle to the object we're putting
        pObject = *gItr;
		if (pObject)
		{
			// Generate the velocities procedurally
			velMin = 3.0F;
			velMax = velMin + std::sqrt(m_SpriteRadius);

			// Randomize the offset from center to be within the original object
			gibROffset.SetXY(m_SpriteRadius * 0.35F * RandomNum(), 0);
			gibROffset.RadRotate(c_PI * RandomNormalNum());
			// Set up its position and velocity according to the parameters of this AEmitter.
			pObject->SetPos(m_Pos + gibROffset);
			pObject->SetRotAngle(m_Rotation.GetRadAngle() + pObject->GetRotMatrix().GetRadAngle());
			// Rotational angle
			pObject->SetAngularVel((pObject->GetAngularVel() * 0.35F) + (pObject->GetAngularVel() * 0.65F / (pObject->GetMass() != 0 ? pObject->GetMass() : 0.0001F)) * RandomNum());
			// Make it rotate away in the appropriate direction depending on which side of the object it is on
			// If the object is far to the relft or right of the center, make it always rotate outwards to some degree
			if (gibROffset.m_X > m_aSprite[0]->w / 3)
			{
				float offCenterRatio = gibROffset.m_X / (m_aSprite[0]->w / 2);
				angularVel = std::abs(pObject->GetAngularVel() * 0.5F);
				angularVel += std::abs(pObject->GetAngularVel() * 0.5F * offCenterRatio);
				pObject->SetAngularVel(angularVel * (gibROffset.m_X > 0.0F ? -1 : 1));
			}
			// Gib is too close to center to always make it rotate in one direction, so give it a baseline rotation and then randomize
			else
			{
				pObject->SetAngularVel((pObject->GetAngularVel() * RandomNum(0.5F, 1.5F)) * (RandomNum() < 0.5F ? 1.0F : -1.0F));
			}

			// TODO: Optimize making the random angles!")
			gibVel = gibROffset;
			if (gibVel.IsZero()) {
				gibVel.SetXY(RandomNum(velMin, velMax), 0.0F);
				gibVel.RadRotate(c_PI * RandomNormalNum());
			} else {
				gibVel.SetMagnitude(RandomNum(velMin, velMax));
			}
			// Distribute any impact implse out over all the gibs
			//            gibVel += (impactImpulse / m_Gibs.size()) / pObject->GetMass();
			pObject->SetVel(m_Vel + gibVel);
			// Reset all the timers of the object being shot out so it doesn't emit a bunch of particles that have been backed up while dormant in inventory
			pObject->ResetAllTimers();

			// Detect whether we're dealing with a passenger and add it as Actor instead
			if (pPassenger = dynamic_cast<Actor *>(pObject))
			{
				pPassenger->SetRotAngle(c_HalfPI * RandomNormalNum());
				pPassenger->SetAngularVel(pPassenger->GetAngularVel() * 5.0F);
				pPassenger->SetHFlipped(RandomNum() > 0.5F);
				pPassenger->SetStatus(UNSTABLE);
				g_MovableMan.AddActor(pPassenger);
			}
			// Add the gib to the scene, passing ownership from the inventory
			else
				g_MovableMan.AddParticle(pObject);

			pPassenger = 0;
			pObject = 0;
		}
    }

    // We have exhausted all teh inventory into the scene, passing ownership
    m_Inventory.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////

void Actor::DropAllGold() {
	const Material *goldMaterial = g_SceneMan.GetMaterialFromID(g_MaterialGold);
	float velMin = 3.0F;
	float velMax = velMin + std::sqrt(m_SpriteRadius);

	for (int i = 0; i < static_cast<int>(std::floor(m_GoldCarried)); i++) {
		Vector dropOffset(m_SpriteRadius * 0.3F * RandomNum(), 0);
		dropOffset.RadRotate(c_PI * RandomNormalNum());
		
		Vector dropVelocity(dropOffset);
		dropVelocity.SetMagnitude(RandomNum(velMin, velMax));

		Atom *goldMOPixelAtom = new Atom(Vector(), g_MaterialGold, nullptr, goldMaterial->GetColor(), 2);

		MOPixel *goldMOPixel = new MOPixel(goldMaterial->GetColor(), goldMaterial->GetPixelDensity(), m_Pos + dropOffset, dropVelocity, goldMOPixelAtom);
		goldMOPixel->SetToHitMOs(false);
		g_MovableMan.AddParticle(goldMOPixel);
	}
	m_GoldCarried = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Actor::AddToInventoryFront(MovableObject *itemToAdd) {
	// This function is called often to add stuff we just removed from our hands, which may be set to delete so we need to guard against that lest we crash.
	if (!itemToAdd || itemToAdd->IsSetToDelete()) {
		return false;
	}

	m_Inventory.push_front(itemToAdd);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool Actor::AddToInventoryBack(MovableObject *itemToAdd) {
	// This function is called often to add stuff we just removed from our hands, which may be set to delete so we need to guard against that lest we crash.
	if (!itemToAdd || itemToAdd->IsSetToDelete()) {
		return false;
	}

	m_Inventory.push_back(itemToAdd);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.

void Actor::GibThis(const Vector &impactImpulse, MovableObject *movableObjectToIgnore)
{
    // Play death sound
// TODO: Don't attenuate since death is pretty important.. maybe only make this happen for teh brains
	if (m_DeathSound) { m_DeathSound->Play(m_Pos); }

    // Gib all the regular gibs
    MOSRotating::GibThis(impactImpulse, movableObjectToIgnore);

    // Throw out all the inventory with the appropriate force and directions
    MovableObject *pObject = 0;
    Actor *pPassenger = 0;
    float velMin, velRange, angularVel;
    Vector gibROffset, gibVel;
    for (std::deque<MovableObject *>::iterator gItr = m_Inventory.begin(); gItr != m_Inventory.end(); ++gItr)
    {
        // Get handy handle to the object we're putting
        pObject = *gItr;

        // Generate the velocities procedurally
		velMin = m_GibBlastStrength / (pObject->GetMass() != 0 ? pObject->GetMass() : 0.0001F);
        velRange = 10.0F;

        // Randomize the offset from center to be within the original object
        gibROffset.SetXY(m_SpriteRadius * 0.35F * RandomNormalNum(), m_SpriteRadius * 0.35F * RandomNormalNum());
        // Set up its position and velocity according to the parameters of this AEmitter.
        pObject->SetPos(m_Pos + gibROffset/*Vector(m_Pos.m_X + 5 * NormalRand(), m_Pos.m_Y + 5 * NormalRand())*/);
        pObject->SetRotAngle(m_Rotation.GetRadAngle() + pObject->GetRotMatrix().GetRadAngle());
        // Rotational angle
        pObject->SetAngularVel((pObject->GetAngularVel() * 0.35F) + (pObject->GetAngularVel() * 0.65F / (pObject->GetMass() != 0 ? pObject->GetMass() : 0.0001F)) * RandomNum());
        // Make it rotate away in the appropriate direction depending on which side of the object it is on
        // If the object is far to the relft or right of the center, make it always rotate outwards to some degree
        if (gibROffset.m_X > m_aSprite[0]->w / 3)
        {
            float offCenterRatio = gibROffset.m_X / (m_aSprite[0]->w / 2);
            angularVel = fabs(pObject->GetAngularVel() * 0.5F);
            angularVel += fabs(pObject->GetAngularVel() * 0.5F * offCenterRatio);
            pObject->SetAngularVel(angularVel * (gibROffset.m_X > 0 ? -1 : 1));
        }
        // Gib is too close to center to always make it rotate in one direction, so give it a baseline rotation and then randomize
        else
        {
            pObject->SetAngularVel((pObject->GetAngularVel() * 0.5F + pObject->GetAngularVel() * RandomNum()) * (RandomNormalNum() > 0.0F ? 1.0F : -1.0F));
        }

// TODO: Optimize making the random angles!")
        gibVel = gibROffset;
        if (gibVel.IsZero())
            gibVel.SetXY(velMin + RandomNum(0.0F, velRange), 0.0F);
        else
            gibVel.SetMagnitude(velMin + RandomNum(0.0F, velRange));
        gibVel.RadRotate(impactImpulse.GetAbsRadAngle());
// Don't! the offset was already rotated!
//            gibVel = RotateOffset(gibVel);
        // Distribute any impact implse out over all the gibs
//            gibVel += (impactImpulse / m_Gibs.size()) / pObject->GetMass();
        pObject->SetVel(m_Vel + gibVel);
        // Reset all the timers of the object being shot out so it doesn't emit a bunch of particles that have been backed up while dormant in inventory
        pObject->ResetAllTimers();

        // Set the gib to not hit a specific MO
        if (movableObjectToIgnore)
            pObject->SetWhichMOToNotHit(movableObjectToIgnore);

        // Detect whether we're dealing with a passenger and add it as Actor instead
        if (pPassenger = dynamic_cast<Actor *>(pObject))
        {
            pPassenger->SetRotAngle(c_HalfPI * RandomNormalNum());
            pPassenger->SetAngularVel(pPassenger->GetAngularVel() * 5.0F);
            pPassenger->SetHFlipped(RandomNum() > 0.5F);
            pPassenger->SetStatus(UNSTABLE);
            g_MovableMan.AddActor(pPassenger);
        }
        // Add the gib to the scene, passing ownership from the inventory
        else
            g_MovableMan.AddParticle(pObject);

        pPassenger = 0;
        pObject = 0;
    }

    // We have exhausted all teh inventory into the scene, passing ownership
    m_Inventory.clear();

    // If this is the actual brain of any player, flash that player's screen when he's now dead
	if (g_SettingsMan.FlashOnBrainDamage() && g_ActivityMan.IsInActivity())
	{
		int brainOfPlayer = g_ActivityMan.GetActivity()->IsBrainOfWhichPlayer(this);
		// Only flash if player is human (AI players don't have screens!)
		if (brainOfPlayer != Players::NoPlayer && g_ActivityMan.GetActivity()->PlayerHuman(brainOfPlayer))
		{
			// Croaked.. flash for a longer period
			if (m_ToDelete || m_Status == DEAD)
				g_FrameMan.FlashScreen(g_ActivityMan.GetActivity()->ScreenOfPlayer(brainOfPlayer), g_WhiteColor, 500);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool Actor::CollideAtPoint(HitData &hd)
{
    return MOSRotating::CollideAtPoint(hd);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ParticlePenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Determines whether a particle which has hit this MO will penetrate,
//                  and if so, whether it gets lodged or exits on the other side of this
//                  MO. Appropriate effects will be determined and applied ONLY IF there
//                  was penetration! If not, nothing will be affected.

bool Actor::ParticlePenetration(HitData &hd) {
    bool penetrated = MOSRotating::ParticlePenetration(hd);

    MovableObject *hitor = hd.Body[HITOR];
    float damageToAdd = hitor->DamageOnCollision();
    damageToAdd += penetrated ? hitor->DamageOnPenetration() : 0;
    if (hitor->GetApplyWoundDamageOnCollision()) { damageToAdd += m_pEntryWound->GetEmitDamage() * hitor->WoundDamageMultiplier(); }
    if (hitor->GetApplyWoundBurstDamageOnCollision()) { damageToAdd += m_pEntryWound->GetBurstDamage() * hitor->WoundDamageMultiplier(); }

	if (damageToAdd != 0) { m_Health = std::min(m_Health - (damageToAdd * m_DamageMultiplier), m_MaxHealth); }
	if ((penetrated || damageToAdd != 0) && m_Perceptiveness > 0 && m_Health > 0) {
		Vector extruded(hd.HitVel[HITOR]);
		extruded.SetMagnitude(m_CharHeight);
		extruded = m_Pos - extruded;
		g_SceneMan.WrapPosition(extruded);
		AlarmPoint(extruded);
	}

    return penetrated;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAIModeIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the icon bitmap associated with this' current AI mode and team.

BITMAP * Actor::GetAIModeIcon()
{
    return m_apAIIcons[m_AIMode];
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLastMOWaypointID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of the last set AI MO waypoint of this. If none, g_NoMOID is returned.
// Arguments:       None.
// Return value:    The furthest set AI MO waypoint of this.

MOID Actor::GetAIMOWaypointID() const
{
	if (g_MovableMan.ValidMO(m_pMOMoveTarget))
		return m_pMOMoveTarget->GetID();
	else
		return g_NoMOID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the path to move along to the currently set movetarget.

void Actor::UpdateMovePath()
{
	if (g_SceneMan.GetScene() == nullptr) {
		return;
	}

    // Estimate how much material this actor can dig through
    float digStrength = EstimateDigStrength();

    // If we're following someone/thing, then never advance waypoints until that thing disappears
    if (g_MovableMan.ValidMO(m_pMOMoveTarget)) {
        m_PathRequest = g_SceneMan.GetScene()->CalculatePathAsync(g_SceneMan.MovePointToGround(m_Pos, m_CharHeight*0.2, 10), m_pMOMoveTarget->GetPos(), digStrength, static_cast<Activity::Teams>(m_Team));
    } else {
        // Do we currently have a path to a static target we would like to still pursue?
        if (m_MovePath.empty())
        {
            // Ok no path going, so get a new path to the next waypoint, if there is a next waypoint
            if (!m_Waypoints.empty())
            {
                // Make sure the path starts from the ground and not somewhere up in the air if/when dropped out of ship
                m_PathRequest = g_SceneMan.GetScene()->CalculatePathAsync(g_SceneMan.MovePointToGround(m_Pos, m_CharHeight*0.2, 10), m_Waypoints.front().first, digStrength, static_cast<Activity::Teams>(m_Team));
                
                // If the waypoint was tied to an MO to pursue, then load it into the current MO target
                if (g_MovableMan.ValidMO(m_Waypoints.front().second)) {
                    m_pMOMoveTarget = m_Waypoints.front().second;
                } else {
                    m_pMOMoveTarget = 0;
                }

                // We loaded the waypoint, no need to keep it
                m_Waypoints.pop_front();
            }
            // Just try to get to the last Move Target
            else {
                m_PathRequest = g_SceneMan.GetScene()->CalculatePathAsync(g_SceneMan.MovePointToGround(m_Pos, m_CharHeight*0.2, 10), m_MoveTarget, digStrength, static_cast<Activity::Teams>(m_Team));
            }
        }
        // We had a path before trying to update, so use its last point as the final destination
        else {
            m_PathRequest = g_SceneMan.GetScene()->CalculatePathAsync(g_SceneMan.MovePointToGround(m_Pos, m_CharHeight*0.2, 10), Vector(m_MovePath.back()), digStrength, static_cast<Activity::Teams>(m_Team));
        }
    }
    
    m_UpdateMovePath = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Actor::EstimateDigStrength() const {
    return m_AIBaseDigStrength;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          VerifyMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Verifieis whether all actor's MO has correct IDs. Should be used in Debug mode only.

void Actor::VerifyMOIDs()
{
	std::vector<MOID> MOIDs;
	GetMOIDs(MOIDs);

	for (std::vector<MOID>::iterator it = MOIDs.begin(); it != MOIDs.end(); it++)
	{
		RTEAssert(*it == g_NoMOID || *it < g_MovableMan.GetMOIDCount(), "Invalid MOID in actor");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

void Actor::OnNewMovePath() {
    if (!m_MovePath.empty()) {
        // Remove the first one; it's our position
        m_PrevPathTarget = m_MovePath.front();
        m_MovePath.pop_front();
        // Also remove the one after that; it may move in opposite direction since it heads to the nearest PathNode center
        // Unless it is the last one, in which case it shouldn't be removed
        if (m_MovePath.size() > 1) {
            m_PrevPathTarget = m_MovePath.front();
            m_MovePath.pop_front();
        }
    } else if (m_pMOMoveTarget) {
        m_MoveTarget = m_pMOMoveTarget->GetPos();
    } else {
        // Nowhere to gooooo
        m_MoveTarget = m_PrevPathTarget = m_Pos;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void Actor::PreControllerUpdate() {
    if (m_PathRequest && m_PathRequest->complete) {
        m_MovePath = const_cast<std::list<Vector> &>(m_PathRequest->path);
        m_PathRequest.reset();
        OnNewMovePath();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void Actor::Update()
{
    ZoneScoped;

    /////////////////////////////////
    // Hit Body update and handling
    MOSRotating::Update();

	m_PieMenu->Update();

    // Update the viewpoint to be at least what the position is
    m_ViewPoint = m_Pos;

    // "See" the location and surroundings of this actor on the unseen map
    if (m_Status != Actor::INACTIVE)
        Look(45 * m_Perceptiveness, g_FrameMan.GetPlayerScreenWidth() * 0.51 * m_Perceptiveness);

	//Check if the MO we're following still exists, and if not, then clear the destination
    if (m_pMOMoveTarget && !g_MovableMan.ValidMO(m_pMOMoveTarget))
		m_pMOMoveTarget = 0;

    ///////////////////////////////////////////////////////////////////////////////
    // Check for manual player-made progress made toward the set AI goal

    if ((m_AIMode == AIMODE_GOTO || m_AIMode == AIMODE_SQUAD) && m_Controller.IsPlayerControlled() && !m_Controller.IsDisabled())
    {
		Vector notUsed;
        // See if we are close enough to the next move target that we should grab the next in the path that is out of proximity range
        Vector pathPointVec;
        for (std::list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end();)
        {
            pathPointVec = g_SceneMan.ShortestDistance(m_Pos, *lItr);
            // Make sure we are within range AND have a clear sight to the path point we're about to eliminate, or it might be around a corner
            if (pathPointVec.MagnitudeIsLessThan(m_MoveProximityLimit) && !g_SceneMan.CastStrengthRay(m_Pos, pathPointVec, 5, notUsed, 0))
            {
                lItr++;
                // Save the last one before being popped off so we can use it to check if we need to dig (if there's any material between last and current)
                m_PrevPathTarget = m_MovePath.front();
                m_MovePath.pop_front();
            } else {
                break;
            }
        }

        if (!m_MovePath.empty())
        {
			Vector notUsed;

            // See if we are close enough to the last point in the current path, in which case we can toss teh whole current path and start ont he next
            pathPointVec = g_SceneMan.ShortestDistance(m_Pos, m_MovePath.back());
            // Clear out the current path, the player apparently took a shortcut
            if (pathPointVec.MagnitudeIsLessThan(m_MoveProximityLimit) && !g_SceneMan.CastStrengthRay(m_Pos, pathPointVec, 5, notUsed, 0, g_MaterialDoor)) {
                m_MovePath.clear();
            }
        }

        // If still stuff in the path, get the next point on it
        if (!m_MovePath.empty())
            m_MoveTarget = m_MovePath.front();
        // No more path, so check if any more waypoints to make a new path to? This doesn't apply if we're following something
        else if (m_MovePath.empty() && !m_Waypoints.empty() && !m_pMOMoveTarget)
            UpdateMovePath();
        // Nope, so just conclude that we must have reached the ultimate AI target set and exit the goto mode
        else if (!m_pMOMoveTarget)
            m_AIMode = AIMODE_SENTRY;
    }
	// Save health state so we can compare next update
	m_PrevHealth = m_Health;
    /////////////////////////////////////
    // Take damage/heal from wounds and wounds on Attachables
    for (AEmitter *wound : m_Wounds) {
        m_Health -= wound->CollectDamage() * m_DamageMultiplier;
    }
    for (Attachable *attachable : m_Attachables) {
        m_Health -= attachable->CollectDamage();
    }
    m_Health = std::min(m_Health, m_MaxHealth);

    /////////////////////////////
    // Stability logic

    if (m_Status == STABLE) {
        // If moving really fast, we're not able to be stable
        if (std::abs(m_Vel.m_X) > std::abs(m_StableVel.m_X) || std::abs(m_Vel.m_Y) > std::abs(m_StableVel.m_Y)) { m_Status = UNSTABLE; }

        m_StableRecoverTimer.Reset();
    }
    else if (m_Status == UNSTABLE) {
        // Only regain stability if we're not moving too fast and it's been a while since we lost it
        if (m_StableRecoverTimer.IsPastSimMS(m_StableRecoverDelay) && !(std::abs(m_Vel.m_X) > std::abs(m_StableVel.m_X) || std::abs(m_Vel.m_Y) > std::abs(m_StableVel.m_Y))) { m_Status = STABLE; }
    }

    /////////////////////////////////////////////
    // Take damage from large hits during travel

	const float travelImpulseMagnitudeSqr = m_TravelImpulse.GetSqrMagnitude();

    // If we're travelling at least half the speed to hurt ourselves, play the body hit noise
    float halfTravelImpulseDamage = m_TravelImpulseDamage * 0.5F;
	if (m_BodyHitSound && travelImpulseMagnitudeSqr > (halfTravelImpulseDamage * halfTravelImpulseDamage)) { m_BodyHitSound->Play(m_Pos); }

    // But only actually damage ourselves if we're unstable
	if (m_Status == Actor::UNSTABLE && travelImpulseMagnitudeSqr > (m_TravelImpulseDamage * m_TravelImpulseDamage)) {
		const float impulse = std::sqrt(travelImpulseMagnitudeSqr) - m_TravelImpulseDamage;
		const float damage = std::max(impulse / (m_GibImpulseLimit - m_TravelImpulseDamage) * m_MaxHealth, 0.0F);
		m_Health -= damage;
		m_ForceDeepCheck = true;
	}

    // Spread the carried items and gold around before death.
    if (m_Status == DYING || m_Status == DEAD) {
		// Actor may die for a long time, no need to call this more than once
		if (m_Inventory.size() > 0) { DropAllInventory(); }
		if (m_GoldCarried > 0) { DropAllGold(); }
    }

    ////////////////////////////////
    // Death logic

	if (m_Status != DYING && m_Status != DEAD && m_Health <= 0) {
		if (m_DeathSound) { m_DeathSound->Play(m_Pos); }
        DropAllInventory();
        m_Status = DYING;
		m_DeathTmr.Reset();
	}

	// Prevent dead actors from rotating like mad
	if (m_Status == DYING || m_Status == DEAD) { m_AngularVel = m_AngularVel * 0.98F; }

	if (m_Status == DYING && m_DeathTmr.GetElapsedSimTimeMS() > 1000) { m_Status = DEAD; }

    //////////////////////////////////////////////////////
    // Save previous second's position so we can detect larger movement

    if (m_LastSecondTimer.IsPastSimMS(1000))
    {
        m_RecentMovement = m_Pos - m_LastSecondPos;
        m_LastSecondPos = m_Pos;
        m_LastSecondTimer.Reset();
    }

    ////////////////////////////////////////
    // Animate the sprite, if applicable

    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENACTIVE)
        {
            if (m_Controller.IsState(MOVE_LEFT) || m_Controller.IsState(MOVE_RIGHT) || m_Controller.GetAnalogMove().GetLargest() > 0.1)
            {
// TODO: improve; make this
                float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % m_SpriteAnimDuration;
                m_Frame = std::floor((cycleTime / (float)m_SpriteAnimDuration) * (float)m_FrameCount);
            }
        }
    }

    /////////////////////////////////
    // Misc

    // If in AI setting mode prior to actor switch, made the team rosters get sorted so the lines are drawn correctly
    if (m_Controller.IsState(PIE_MENU_ACTIVE))
    {
        g_MovableMan.SortTeamRoster(m_Team);
    }

    // Play PainSound if damage this frame exceeded PainThreshold
    if (m_PainThreshold > 0 && m_PrevHealth - m_Health > m_PainThreshold && m_Health > 1 && m_PainSound) { m_PainSound->Play(m_Pos); }

	int brainOfPlayer = g_ActivityMan.GetActivity()->IsBrainOfWhichPlayer(this);
	if (brainOfPlayer != Players::NoPlayer && g_ActivityMan.GetActivity()->PlayerHuman(brainOfPlayer)) {
		if (m_PrevHealth - m_Health > 1.5F) {
			// If this is a brain that's under attack, broadcast an alarm event so that the enemy AI won't dawdle in trying to kill it.
			g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, 0.5F));
			if (g_SettingsMan.FlashOnBrainDamage()) {
				g_FrameMan.FlashScreen(g_ActivityMan.GetActivity()->ScreenOfPlayer(brainOfPlayer), g_RedColor, 10);
			}
		}
		if ((m_ToDelete || m_Status == DEAD) && g_SettingsMan.FlashOnBrainDamage()) {
			g_FrameMan.FlashScreen(g_ActivityMan.GetActivity()->ScreenOfPlayer(brainOfPlayer), g_WhiteColor, 500);
		}
	}
}

void Actor::FullUpdate() {
    PreControllerUpdate();
    m_Controller.Update();
    Update();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void Actor::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
	// This should probably be a local var and not alter a member one in a draw func. Would let us make these functions const
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

    // Draw stat info HUD
    char str[64];

    GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();
    Vector drawPos = m_Pos - targetPos;
    Vector cpuPos = GetCPUPos() - targetPos;

    // If we have something to draw, adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
    if ((m_HUDVisible || m_PieMenu->IsVisible()) && !targetPos.IsZero()) {
        // Spans vertical scene seam
        int sceneWidth = g_SceneMan.GetSceneWidth();
        if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth) {
            if ((targetPos.m_X < 0) && (m_Pos.m_X > (sceneWidth - pTargetBitmap->w))) {
                drawPos.m_X -= sceneWidth;
                cpuPos.m_X -= sceneWidth;
            } else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_Pos.m_X < pTargetBitmap->w)) {
                drawPos.m_X += sceneWidth;
                cpuPos.m_X += sceneWidth;
            }
        }

        // Spans horizontal scene seam
        int sceneHeight = g_SceneMan.GetSceneHeight();
        if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight) {
            if ((targetPos.m_Y < 0) && (m_Pos.m_Y > (sceneHeight - pTargetBitmap->h))) {
                drawPos.m_Y -= sceneHeight;
                cpuPos.m_Y -= sceneHeight;
            } else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_Pos.m_Y < pTargetBitmap->h)) {
                drawPos.m_Y += sceneHeight;
                cpuPos.m_Y += sceneHeight;
            }
        }
    }

	int actorScreen = g_ActivityMan.GetActivity() ? g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller.GetPlayer()) : -1;
	bool screenTeamIsSameAsActorTeam = g_ActivityMan.GetActivity() ? g_ActivityMan.GetActivity()->GetTeamOfPlayer(whichScreen) == m_Team : true;
	if (m_PieMenu->IsVisible() && screenTeamIsSameAsActorTeam && (!m_PieMenu->IsInNormalAnimationMode() || (m_Controller.IsPlayerControlled() && actorScreen == whichScreen))) {
		m_PieMenu->Draw(pTargetBitmap, targetPos);
	}

	if (!m_HUDVisible) {
		return;
	}

    // Draw the selection arrow, if controlled and under the arrow's time limit
    if (m_Controller.IsPlayerControlled() && m_NewControlTmr.GetElapsedSimTimeMS() < ARROWTIME) {
        draw_sprite(pTargetBitmap, m_apSelectArrow[m_Team], cpuPos.m_X, EaseOut(drawPos.m_Y + m_HUDStack - 60, drawPos.m_Y + m_HUDStack - 20, m_NewControlTmr.GetElapsedSimTimeMS() / (float)ARROWTIME));
    }

    // Draw the alarm exclamation mark if we are alarmed!
    if (m_AlarmTimer.SimTimeLimitProgress() < 0.25) {
        draw_sprite(pTargetBitmap, m_apAlarmExclamation[m_AgeTimer.AlternateSim(100)], cpuPos.m_X - 3, EaseOut(drawPos.m_Y + m_HUDStack - 10, drawPos.m_Y + m_HUDStack - 25, m_AlarmTimer.SimTimeLimitProgress() / 0.25f));
    }

    if (pSmallFont && pSymbolFont) {
        AllegroBitmap bitmapInt(pTargetBitmap);

        if (!m_Controller.IsState(PIE_MENU_ACTIVE) || actorScreen != whichScreen) {
            // If we're still alive, show the team colors
            if (m_Health > 0) {
				if (IsPlayerControlled() && g_FrameMan.IsInMultiplayerMode()) {
					m_pControllerIcon = 0;

					if (m_Team == 0) {
						m_pControllerIcon = g_UInputMan.GetDeviceIcon(DEVICE_GAMEPAD_1);
                    } else if (m_Team == 1) {
						m_pControllerIcon = g_UInputMan.GetDeviceIcon(DEVICE_GAMEPAD_2);
                    } else if (m_Team == 2) {
						m_pControllerIcon = g_UInputMan.GetDeviceIcon(DEVICE_GAMEPAD_3);
                    } else if (m_Team == 3) {
						m_pControllerIcon = g_UInputMan.GetDeviceIcon(DEVICE_GAMEPAD_4);
                    }

					if (m_pControllerIcon) {
						std::vector<BITMAP *> apControllerBitmaps = m_pControllerIcon->GetBitmaps8();
						masked_blit(apControllerBitmaps[0], pTargetBitmap, 0, 0, drawPos.m_X - apControllerBitmaps[0]->w - 2 + 10, drawPos.m_Y + m_HUDStack - (apControllerBitmaps[0]->h / 2) + 8, apControllerBitmaps[0]->w, apControllerBitmaps[0]->h);
					}
				}

                // Get the Icon bitmaps of this Actor's team, if any
                std::vector<BITMAP *> apIconBitmaps;
                if (m_pTeamIcon) {
                    apIconBitmaps = m_pTeamIcon->GetBitmaps8();
                }

                // Team Icon could not be found, or of no team, so use the static noteam Icon instead
                if (apIconBitmaps.empty()) {
                    apIconBitmaps = m_apNoTeamIcon;
                }

                // Now draw the Icon if we can
                if (!apIconBitmaps.empty() && m_pTeamIcon && m_pTeamIcon->GetFrameCount() > 0) {
                    // Make team icon blink faster as the health goes down
                    int f = m_HeartBeat.AlternateReal(200 + 800 * (m_Health / 100)) ? 0 : 1;
                    f = MIN(f, m_pTeamIcon ? m_pTeamIcon->GetFrameCount() - 1 : 1);
                    masked_blit(apIconBitmaps.at(f), pTargetBitmap, 0, 0, drawPos.m_X - apIconBitmaps.at(f)->w - 2, drawPos.m_Y + m_HUDStack - (apIconBitmaps.at(f)->h / 2) + 8, apIconBitmaps.at(f)->w, apIconBitmaps.at(f)->h);
                }
            } else {
                // Draw death icon
                str[0] = -39;
                str[1] = 0;
                pSymbolFont->DrawAligned(&bitmapInt, drawPos.m_X - 10, drawPos.m_Y + m_HUDStack, str, GUIFont::Left);
            }

			std::snprintf(str, sizeof(str), "%.0f", std::ceil(m_Health));
            pSymbolFont->DrawAligned(&bitmapInt, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack, str, GUIFont::Left);

            m_HUDStack += -12;

			if (IsPlayerControlled()) {
				if (GetGoldCarried() > 0) {
					str[0] = m_GoldPicked ? -57 : -58; str[1] = 0;
					pSymbolFont->DrawAligned(&bitmapInt, drawPos.GetFloorIntX() - 11, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Left);
					std::snprintf(str, sizeof(str), "%.0f oz", GetGoldCarried());
					pSmallFont->DrawAligned(&bitmapInt, drawPos.GetFloorIntX() - 0, drawPos.GetFloorIntY() + m_HUDStack + 2, str, GUIFont::Left);

					m_HUDStack -= 11;
				}
				// Player name
				if (g_FrameMan.IsInMultiplayerMode()) {
					if (GameActivity * gameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetActivity())) {
						pSmallFont->DrawAligned(&bitmapInt, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 2, gameActivity->GetNetworkPlayerName(m_Controller.GetPlayer()).c_str(), GUIFont::Centre);
						m_HUDStack -= 11;
					}
				}
			}
        }
    }

    // Don't proceed to draw all the secret stuff below if this screen is for a player on the other team!
    if (g_ActivityMan.GetActivity() && g_ActivityMan.GetActivity()->GetTeamOfPlayer(whichScreen) != m_Team) {
        return;
    }

    // AI waypoints or points of interest
    if (m_DrawWaypoints && m_PlayerControllable && (m_AIMode == AIMODE_GOTO || m_AIMode == AIMODE_SQUAD)) {
        // Draw the AI paths, from the ultimate destination back up to the actor's position.
        // We do this backwards so the lines won't crawl and the dots can be evenly spaced throughout
        Vector waypoint;
        std::list<std::pair<Vector, const MovableObject *> >::reverse_iterator vLast, vItr;
        std::list<Vector>::reverse_iterator lLast, lItr;
        int skipPhase = 0;

        // Draw the line between the end of the movepath and the first waypoint after that, if any
        if (!m_Waypoints.empty()) {
            // Draw the additional waypoint points beyond the first one
            vLast = m_Waypoints.rbegin();
            vItr = m_Waypoints.rbegin();
            for (; vItr != m_Waypoints.rend(); ++vItr)
            {
                // Draw the line
                g_FrameMan.DrawLine(pTargetBitmap, (*vLast).first - targetPos, (*vItr).first - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, 0, true);
                vLast = vItr;

                // Draw the points
                waypoint = (*vItr).first - targetPos;
                circlefill(pTargetBitmap, waypoint.m_X, waypoint.m_Y, 2, g_YellowGlowColor);

                // Add pixel glow area around it, in scene coordinates
				g_PostProcessMan.RegisterGlowArea((*vItr).first, 5);
            }

            // Draw line from the last movetarget on the current path to the first waypoint in queue after that
            if (!m_MovePath.empty()) {
                g_FrameMan.DrawLine(pTargetBitmap, m_MovePath.back() - targetPos, m_Waypoints.front().first - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, 0, true);
            } else {
                g_FrameMan.DrawLine(pTargetBitmap, m_MoveTarget - targetPos, m_Waypoints.front().first - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, 0, true);
            }
        }

        // Draw the current movepath, but backwards so the dot spacing can be even and they don't crawl as the guy approaches
        if (!m_MovePath.empty()) {
            lLast = m_MovePath.rbegin();
            lItr = m_MovePath.rbegin();
            for (; lItr != m_MovePath.rend(); ++lItr) {
                // Draw these backwards so the skip phase works
                skipPhase = g_FrameMan.DrawLine(pTargetBitmap, (*lLast) - targetPos, (*lItr) - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, skipPhase, true);
                lLast = lItr;
            }

            // Draw the line between the current position and to the start of the movepath, backwards so the dotted lines doesn't crawl
            skipPhase = g_FrameMan.DrawLine(pTargetBitmap, m_MovePath.front() - targetPos, m_Pos - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, skipPhase, true);

            // Draw the first destination/waypoint point
            waypoint = m_MovePath.back() - targetPos;
            circlefill(pTargetBitmap, waypoint.m_X, waypoint.m_Y, 2, g_YellowGlowColor);

            // Add pixel glow area around it, in scene coordinates
			g_PostProcessMan.RegisterGlowArea(m_MovePath.back(), 5);
        } else {
            // No points left on movepath, so draw straight line to the movetarget

            // Draw it backwards so the dotted lines doesn't crawl
            skipPhase = g_FrameMan.DrawLine(pTargetBitmap, m_MoveTarget - targetPos, m_Pos - targetPos, g_YellowGlowColor, 0, AILINEDOTSPACING, skipPhase, true);

            // Draw the first destination/waypoint point
            waypoint = m_MoveTarget - targetPos;
            circlefill(pTargetBitmap, waypoint.m_X, waypoint.m_Y, 2, g_YellowGlowColor);

            // Add pixel glow area around it, in scene coordinates
			g_PostProcessMan.RegisterGlowArea(m_MoveTarget, 5);
        }
    }

    // AI Mode team roster HUD lines
	if (m_PlayerControllable && g_ActivityMan.GetActivity()->GetViewState(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen)) == Activity::ViewState::ActorSelect && g_SceneMan.ShortestDistance(m_Pos, g_CameraMan.GetScrollTarget(whichScreen), g_SceneMan.SceneWrapsX()).GetMagnitude() < 100) {
		draw_sprite(pTargetBitmap, GetAIModeIcon(), cpuPos.m_X - 6, cpuPos.m_Y - 6);
	} else if (m_Controller.IsState(ACTOR_NEXT_PREP) || m_Controller.IsState(ACTOR_PREV_PREP)) {
        int prevColor = m_Controller.IsState(ACTOR_PREV_PREP) ? 122 : (m_Team == Activity::TeamOne ? 13 : 147);
        int nextColor = m_Controller.IsState(ACTOR_NEXT_PREP) ? 122 : (m_Team == Activity::TeamOne ? 13 : 147);
        int prevSpacing = m_Controller.IsState(ACTOR_PREV_PREP) ? 3 : 9;
        int nextSpacing = m_Controller.IsState(ACTOR_NEXT_PREP) ? 3 : 9;
        int altColor = m_Team == Activity::TeamOne ? 11 : 160;

        Actor *pPrevAdj = 0;
        Actor *pNextAdj = 0;
        std::list<Actor *> *pRoster = g_MovableMan.GetTeamRoster(m_Team);

        if (pRoster->size() > 1) {
            // Find this in the list, both ways
            std::list<Actor *>::reverse_iterator selfRItr = find(pRoster->rbegin(), pRoster->rend(), this);
            RTEAssert(selfRItr != pRoster->rend(), "Actor couldn't find self in Team roster!");
            std::list<Actor *>::iterator selfItr = find(pRoster->begin(), pRoster->end(), this);
            RTEAssert(selfItr != pRoster->end(), "Actor couldn't find self in Team roster!");

            // Find the adjacent actors
            if (selfItr != pRoster->end()) {
                // Get the previous available actor in the list (not controlled by another player)
                std::list<Actor *>::reverse_iterator prevItr = selfRItr;
                do {
                    if (++prevItr == pRoster->rend()) {
                        prevItr = pRoster->rbegin();
                    }

                    if ((*prevItr) == (*selfItr)) {
                        break;
                    }
                } while (!(*prevItr)->IsPlayerControllable() || (*prevItr)->GetController()->IsPlayerControlled() || g_ActivityMan.GetActivity()->IsOtherPlayerBrain((*prevItr), m_Controller.GetPlayer()));

                // Get the next actor in the list (not controlled by another player)
                std::list<Actor *>::iterator nextItr = selfItr;
                do {
                    if (++nextItr == pRoster->end()) {
                        nextItr = pRoster->begin();
                    }

                    if ((*nextItr) == (*selfItr)) {
                        break;
                    }
                } while (!(*nextItr)->IsPlayerControllable() || (*nextItr)->GetController()->IsPlayerControlled() || g_ActivityMan.GetActivity()->IsOtherPlayerBrain((*prevItr), m_Controller.GetPlayer()));

                Vector iconPos = cpuPos;
                
                // Only continue if there are available adjacent Actors
                if ((*prevItr) != (*selfItr) && (*nextItr) != (*selfItr)) {
                    pPrevAdj = *prevItr;
                    pNextAdj = *nextItr;
                    if (pPrevAdj != pNextAdj) {
                        // Only draw both lines if they're not pointing to the same thing
                        g_FrameMan.DrawLine(pTargetBitmap, cpuPos, pPrevAdj->GetCPUPos() - targetPos, prevColor, prevColor, prevSpacing, 0, true);
                        g_FrameMan.DrawLine(pTargetBitmap, cpuPos, pNextAdj->GetCPUPos() - targetPos, nextColor, nextColor, nextSpacing, 0, true);
                    }
                    else {
                        // If only one other available Actor, only draw one yellow line to it
                        g_FrameMan.DrawLine(pTargetBitmap, cpuPos, pNextAdj->GetCPUPos() - targetPos, 122, 122, 3, 0, true);
                    }

                    // Prev selected icon
                    iconPos = pPrevAdj->GetCPUPos() - targetPos;
                    draw_sprite(pTargetBitmap, pPrevAdj->GetAIModeIcon(), iconPos.m_X - 6, iconPos.m_Y - 6);

                    // Next selected icon
                    iconPos = pNextAdj->GetCPUPos() - targetPos;
                    draw_sprite(pTargetBitmap, pNextAdj->GetAIModeIcon(), iconPos.m_X - 6, iconPos.m_Y - 6);
                }

                // Self selected icon
                iconPos = cpuPos;
                draw_sprite(pTargetBitmap, GetAIModeIcon(), iconPos.m_X - 6, iconPos.m_Y - 6);
            }
        }
    }
}


} // namespace RTE
