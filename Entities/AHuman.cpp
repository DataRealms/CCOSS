//////////////////////////////////////////////////////////////////////////////////////////
// File:            AHuman.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AHuman class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AHuman.h"

#include "AtomGroup.h"
#include "ThrownDevice.h"
#include "Arm.h"
#include "Leg.h"
#include "Controller.h"
#include "MOPixel.h"
#include "FrameMan.h"
#include "AEmitter.h"
#include "HDFirearm.h"
#include "SLTerrain.h"
#include "PresetMan.h"
#include "Scene.h"
#include "SettingsMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(AHuman, Actor, 20);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AHuman, effectively
//                  resetting the members of this abstraction level only.

void AHuman::Clear()
{
    m_pHead = 0;
	m_LookToAimRatio = 0.7F;
    m_pJetpack = 0;
    m_pFGArm = 0;
    m_pBGArm = 0;
    m_pFGLeg = 0;
    m_pBGLeg = 0;
    m_pFGHandGroup = 0;
    m_pBGHandGroup = 0;
    m_pFGFootGroup = 0;
    m_BackupFGFootGroup = nullptr;
    m_pBGFootGroup = 0;
    m_BackupBGFootGroup = nullptr;
    m_StrideSound = nullptr;
    m_ArmsState = WEAPON_READY;
    m_MoveState = STAND;
    m_ProneState = NOTPRONE;
    m_ProneTimer.Reset();
    for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
        m_Paths[FGROUND][i].Reset();
        m_Paths[BGROUND][i].Reset();
        m_Paths[FGROUND][i].Terminate();
        m_Paths[BGROUND][i].Terminate();
        m_RotAngleTargets[i] = 0.0F;
    }
    m_Aiming = false;
    m_ArmClimbing[FGROUND] = false;
    m_ArmClimbing[BGROUND] = false;
    m_StrideStart = false;
    m_JetTimeTotal = 0.0;
    m_JetTimeLeft = 0.0;
	m_JetReplenishRate = 1.0F;
	m_JetAngleRange = 0.25F;
	m_CanActivateBGItem = false;
	m_TriggerPulled = false;
	m_WaitingToReloadOffhand = false;
    m_ThrowTmr.Reset();
    m_ThrowPrepTime = 1000;
	m_SharpAimRevertTimer.Reset();
	m_FGArmFlailScalar = 0.0F;
	m_BGArmFlailScalar = 0.7F;
	m_EquipHUDTimer.Reset();
	m_WalkAngle.fill(Matrix());
	m_ArmSwingRate = 1.0F;
	m_DeviceArmSwayRate = 0.5F;

    m_DeviceState = SCANNING;
    m_SweepState = NOSWEEP;
    m_DigState = NOTDIGGING;
    m_JumpState = NOTJUMPING;
    m_JumpTarget.Reset();
    m_JumpingRight = true;
    m_Crawling = false;
    m_DigTunnelEndPos.Reset();
    m_SweepCenterAimAngle = 0;
    m_SweepRange = c_EighthPI;
    m_DigTarget.Reset();
    m_FireTimer.Reset();
    m_SweepTimer.Reset();
    m_PatrolTimer.Reset();
    m_JumpTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AHuman object ready for use.

int AHuman::Create()
{
	if (Actor::Create() < 0) {
		return -1;
	}

	if (m_AIMode == Actor::AIMODE_NONE) {
		m_AIMode = Actor::AIMODE_BRAINHUNT;
	}

    // Cheat to make sure the FG Arm is always at the end of the Attachables list so it draws last.
    if (m_pFGArm) {
        m_Attachables.erase(std::find(m_Attachables.begin(), m_Attachables.end(), m_pFGArm));
        m_Attachables.push_back(m_pFGArm);
    }

    // Make the limb paths for the background limbs
    for (int i = 0; i < MOVEMENTSTATECOUNT; ++i)
    {
        // If BG path is not initalized, then copy the FG one to it
        if (!m_Paths[BGROUND][i].IsInitialized())
        {
            m_Paths[BGROUND][i].Destroy();
            m_Paths[BGROUND][i].Create(m_Paths[FGROUND][i]);
        }
    }

    // If empty-handed, equip first thing in inventory
    if (m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldDevice()) {
        m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice *>(SwapNextInventory(nullptr, true)));
        m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
    }

    // Initalize the jump time left
    m_JetTimeLeft = m_JetTimeTotal;

    // All AHumans by default avoid hitting each other ont he same team
    m_IgnoresTeamHits = true;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AHuman to be identical to another, by deep copy.

int AHuman::Create(const AHuman &reference) {
    if (reference.m_pBGArm) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGArm->GetUniqueID()); }
    if (reference.m_pBGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGLeg->GetUniqueID()); }
    if (reference.m_pJetpack) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pJetpack->GetUniqueID()); }
    if (reference.m_pHead) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pHead->GetUniqueID()); }
    if (reference.m_pFGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGLeg->GetUniqueID()); }
    if (reference.m_pFGArm) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGArm->GetUniqueID()); }

    Actor::Create(reference);

    //Note - hardcoded attachable copying is organized based on desired draw order here.
    if (reference.m_pBGArm) { SetBGArm(dynamic_cast<Arm *>(reference.m_pBGArm->Clone())); }
    if (reference.m_pBGLeg) { SetBGLeg(dynamic_cast<Leg *>(reference.m_pBGLeg->Clone())); }
    if (reference.m_pJetpack) { SetJetpack(dynamic_cast<AEmitter *>(reference.m_pJetpack->Clone())); }
    if (reference.m_pHead) { SetHead(dynamic_cast<Attachable *>(reference.m_pHead->Clone())); }
    if (reference.m_pFGLeg) { SetFGLeg(dynamic_cast<Leg *>(reference.m_pFGLeg->Clone())); }
    if (reference.m_pFGArm) { SetFGArm(dynamic_cast<Arm *>(reference.m_pFGArm->Clone())); }

	m_LookToAimRatio = reference.m_LookToAimRatio;

	m_ThrowPrepTime = reference.m_ThrowPrepTime;
    m_JetTimeTotal = reference.m_JetTimeTotal;
    m_JetTimeLeft = reference.m_JetTimeLeft;
    m_JetReplenishRate = reference.m_JetReplenishRate;
	m_JetAngleRange = reference.m_JetAngleRange;
	m_WaitingToReloadOffhand = reference.m_WaitingToReloadOffhand;
	m_FGArmFlailScalar = reference.m_FGArmFlailScalar;
	m_BGArmFlailScalar = reference.m_BGArmFlailScalar;
	m_ArmSwingRate = reference.m_ArmSwingRate;
	m_DeviceArmSwayRate = reference.m_DeviceArmSwayRate;

    m_pFGHandGroup = dynamic_cast<AtomGroup *>(reference.m_pFGHandGroup->Clone());
    m_pFGHandGroup->SetOwner(this);
    m_pBGHandGroup = dynamic_cast<AtomGroup *>(reference.m_pBGHandGroup->Clone());
    m_pBGHandGroup->SetOwner(this);

	AtomGroup *atomGroupToUseAsFootGroupFG = reference.m_pFGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pFGFootGroup->Clone()) : m_pFGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupFG, "Failed to fallback to using FGFoot AtomGroup as FGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a FGFootGroup or FGLeg Foot attachable!");

	AtomGroup *atomGroupToUseAsFootGroupBG = reference.m_pBGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pBGFootGroup->Clone()) : m_pBGLeg->GetFootGroupFromFootAtomGroup();;
	RTEAssert(atomGroupToUseAsFootGroupBG, "Failed to fallback to using BGFoot AtomGroup as BGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a BGFootGroup or BGLeg Foot attachable!");

    m_pFGFootGroup = atomGroupToUseAsFootGroupFG;
    m_pFGFootGroup->SetOwner(this);
    m_BackupFGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupFG->Clone());
	m_BackupFGFootGroup->RemoveAllAtoms();
    m_BackupFGFootGroup->SetOwner(this);
    m_BackupFGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupFG->GetLimbPos());
    m_pBGFootGroup = atomGroupToUseAsFootGroupBG;
    m_pBGFootGroup->SetOwner(this);
    m_BackupBGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupBG->Clone());
	m_BackupBGFootGroup->RemoveAllAtoms();
    m_BackupBGFootGroup->SetOwner(this);
    m_BackupBGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupBG->GetLimbPos());

	if (reference.m_StrideSound) { m_StrideSound = dynamic_cast<SoundContainer*>(reference.m_StrideSound->Clone()); }

    m_ArmsState = reference.m_ArmsState;
    m_MoveState = reference.m_MoveState;
    m_ProneState = reference.m_ProneState;

    for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
        m_Paths[FGROUND][i].Create(reference.m_Paths[FGROUND][i]);
        m_Paths[BGROUND][i].Create(reference.m_Paths[BGROUND][i]);
        m_RotAngleTargets[i] = reference.m_RotAngleTargets[i];
    }

    m_DeviceState = reference.m_DeviceState;
    m_SweepState = reference.m_SweepState;
    m_DigState = reference.m_DigState;
    m_JumpState = reference.m_JumpState;
    m_JumpTarget = reference.m_JumpTarget;
    m_JumpingRight = reference.m_JumpingRight;
    m_Crawling = reference.m_Crawling;
    m_DigTunnelEndPos = reference.m_DigTunnelEndPos;
    m_SweepCenterAimAngle = reference.m_SweepCenterAimAngle;
    m_SweepRange = reference.m_SweepRange;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int AHuman::ReadProperty(const std::string_view &propName, Reader &reader) {
    if (propName == "ThrowPrepTime") {
        reader >> m_ThrowPrepTime;
    } else if (propName == "Head") {
        SetHead(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader)));
	} else if (propName == "LookToAimRatio") {
		reader >> m_LookToAimRatio;
    } else if (propName == "Jetpack") {
        SetJetpack(dynamic_cast<AEmitter *>(g_PresetMan.ReadReflectedPreset(reader)));
	} else if (propName == "JumpTime" || propName == "JetTime") {
        reader >> m_JetTimeTotal;
        // Convert to ms
        m_JetTimeTotal *= 1000;
	} else if (propName == "JumpReplenishRate" || propName == "JetReplenishRate") {
		reader >> m_JetReplenishRate;
	} else if (propName == "JumpAngleRange" || propName == "JetAngleRange") {
		reader >> m_JetAngleRange;
	} else if (propName == "FGArmFlailScalar") {
		reader >> m_FGArmFlailScalar;
	} else if (propName == "BGArmFlailScalar") {
		reader >> m_BGArmFlailScalar;
	} else if (propName == "ArmSwingRate") {
		reader >> m_ArmSwingRate;
	} else if (propName == "DeviceArmSwayRate") {
		reader >> m_DeviceArmSwayRate;
    } else if (propName == "FGArm") {
        SetFGArm(dynamic_cast<Arm *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "BGArm") {
        SetBGArm(dynamic_cast<Arm *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "FGLeg") {
        SetFGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "BGLeg") {
        SetBGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader)));
    } else if (propName == "HandGroup") {
        delete m_pFGHandGroup;
        delete m_pBGHandGroup;
        m_pFGHandGroup = new AtomGroup();
        m_pBGHandGroup = new AtomGroup();
        reader >> m_pFGHandGroup;
        m_pBGHandGroup->Create(*m_pFGHandGroup);
        m_pFGHandGroup->SetOwner(this);
        m_pBGHandGroup->SetOwner(this);
    } else if (propName == "FGFootGroup") {
        delete m_pFGFootGroup;
        m_pFGFootGroup = new AtomGroup();
        reader >> m_pFGFootGroup;
        m_pFGFootGroup->SetOwner(this);
        m_BackupFGFootGroup = new AtomGroup(*m_pFGFootGroup);
        m_BackupFGFootGroup->RemoveAllAtoms();
    } else if (propName == "BGFootGroup") {
        delete m_pBGFootGroup;
        m_pBGFootGroup = new AtomGroup();
        reader >> m_pBGFootGroup;
        m_pBGFootGroup->SetOwner(this);
        m_BackupBGFootGroup = new AtomGroup(*m_pBGFootGroup);
        m_BackupBGFootGroup->RemoveAllAtoms();
    } else if (propName == "StrideSound") {
		m_StrideSound = new SoundContainer;
        reader >> m_StrideSound;
    } else if (propName == "StandLimbPath") {
        reader >> m_Paths[FGROUND][STAND];
    } else if (propName == "StandLimbPathBG") {
        reader >> m_Paths[BGROUND][STAND];
    } else if (propName == "WalkLimbPath") {
        reader >> m_Paths[FGROUND][WALK];
    } else if (propName == "CrouchLimbPath") {
        reader >> m_Paths[FGROUND][CROUCH];
    } else if (propName == "CrouchLimbPathBG") {
        reader >> m_Paths[BGROUND][CROUCH];
    } else if (propName == "CrawlLimbPath") {
        reader >> m_Paths[FGROUND][CRAWL];
    } else if (propName == "ArmCrawlLimbPath") {
        reader >> m_Paths[FGROUND][ARMCRAWL];
    } else if (propName == "ClimbLimbPath") {
        reader >> m_Paths[FGROUND][CLIMB];
    } else if (propName == "JumpLimbPath") {
        reader >> m_Paths[FGROUND][JUMP];
    } else if (propName == "DislodgeLimbPath") {
        reader >> m_Paths[FGROUND][DISLODGE];
    } else if (propName == "StandRotAngleTarget") {
        reader >> m_RotAngleTargets[STAND];
    } else if (propName == "WalkRotAngleTarget") {
        reader >> m_RotAngleTargets[WALK];
    } else if (propName == "CrouchRotAngleTarget") {
        reader >> m_RotAngleTargets[CROUCH];
    } else if (propName == "JumpRotAngleTarget") {
        reader >> m_RotAngleTargets[JUMP];
    } else {
        return Actor::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AHuman with a Writer for
//                  later recreation with Create(Reader &reader);

int AHuman::Save(Writer &writer) const
{
    Actor::Save(writer);

	writer.NewProperty("ThrowPrepTime");
	writer << m_ThrowPrepTime;
    writer.NewProperty("Head");
    writer << m_pHead;
    writer.NewProperty("LookToAimRatio");
    writer << m_LookToAimRatio;
    writer.NewProperty("Jetpack");
    writer << m_pJetpack;
    writer.NewProperty("JumpTime");
    // Convert to seconds
    writer << m_JetTimeTotal / 1000;
	writer.NewProperty("JumpReplenishRate");
	writer << m_JetReplenishRate;
	writer.NewProperty("JumpAngleRange");
	writer << m_JetAngleRange;
	writer.NewProperty("FGArmFlailScalar");
	writer << m_FGArmFlailScalar;
	writer.NewProperty("BGArmFlailScalar");
	writer << m_BGArmFlailScalar;
	writer.NewProperty("ArmSwingRate");
	writer << m_ArmSwingRate;
	writer.NewPropertyWithValue("DeviceArmSwayRate", m_DeviceArmSwayRate);
    writer.NewProperty("FGArm");
    writer << m_pFGArm;
    writer.NewProperty("BGArm");
    writer << m_pBGArm;
    writer.NewProperty("FGLeg");
    writer << m_pFGLeg;
    writer.NewProperty("BGLeg");
    writer << m_pBGLeg;
    writer.NewProperty("HandGroup");
    writer << m_pFGHandGroup;
    writer.NewProperty("FGFootGroup");
    writer << m_pFGFootGroup;
    writer.NewProperty("BGFootGroup");
    writer << m_pBGFootGroup;
    writer.NewProperty("StrideSound");
    writer << m_StrideSound;

    writer.NewProperty("StandLimbPath");
    writer << m_Paths[FGROUND][STAND];
    writer.NewProperty("StandLimbPathBG");
    writer << m_Paths[BGROUND][STAND];
    writer.NewProperty("WalkLimbPath");
    writer << m_Paths[FGROUND][WALK];
    writer.NewProperty("CrouchLimbPath");
    writer << m_Paths[FGROUND][CROUCH];
    writer.NewProperty("CrawlLimbPath");
    writer << m_Paths[FGROUND][CRAWL];
    writer.NewProperty("ArmCrawlLimbPath");
    writer << m_Paths[FGROUND][ARMCRAWL];
    writer.NewProperty("ClimbLimbPath");
    writer << m_Paths[FGROUND][CLIMB];
    writer.NewProperty("JumpLimbPath");
    writer << m_Paths[FGROUND][JUMP];
    writer.NewProperty("DislodgeLimbPath");
    writer << m_Paths[FGROUND][DISLODGE];

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AHuman object.

void AHuman::Destroy(bool notInherited) {
    delete m_pFGHandGroup;
    delete m_pBGHandGroup;
    delete m_pFGFootGroup;
    delete m_pBGFootGroup;

	delete m_StrideSound;

    if (!notInherited) { Actor::Destroy(); }
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this Actor and all its carried
//                  gold and inventory.

float AHuman::GetTotalValue(int nativeModule, float foreignMult, float nativeMult) const
{
    float totalValue = Actor::GetTotalValue(nativeModule, foreignMult, nativeMult);

    // If holding something, then add its value, too
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
        totalValue += m_pFGArm->GetHeldDevice()->GetTotalValue(nativeModule, foreignMult, nativeMult);

    return totalValue;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this carries a specifically named object in its inventory.
//                  Also looks through the inventories of potential passengers, as applicable.

bool AHuman::HasObject(std::string objectName) const
{
    bool found = Actor::HasObject(objectName);

    // If holding something, then check that too
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
        found = found || m_pFGArm->GetHeldDevice()->HasObject(objectName);
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldDevice())
        found = found || m_pBGArm->GetHeldDevice()->HasObject(objectName);

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObjectInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically grouped object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.

bool AHuman::HasObjectInGroup(std::string groupName) const
{
    bool found = Actor::HasObjectInGroup(groupName);

    // If holding something, then check that too
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
        found = found || m_pFGArm->GetHeldDevice()->HasObjectInGroup(groupName);
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldDevice())
        found = found || m_pBGArm->GetHeldDevice()->HasObjectInGroup(groupName);

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetCPUPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' brain, or equivalent.

Vector AHuman::GetCPUPos() const
{
    if (m_pHead && m_pHead->IsAttached())
        return m_Pos + ((m_pHead->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation) * 1.5);

    return m_Pos;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.

Vector AHuman::GetEyePos() const
{
	if (m_pHead && m_pHead->IsAttached()) {
		return m_Pos + m_pHead->GetParentOffset() * 1.2F;
	}

    return m_Pos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetHead(Attachable *newHead) {
    if (m_pHead && m_pHead->IsAttached()) { RemoveAndDeleteAttachable(m_pHead); }
    if (newHead == nullptr) {
        m_pHead = nullptr;
    } else {
        m_pHead = newHead;
        AddAttachable(newHead);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newHead->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            dynamic_cast<AHuman *>(parent)->SetHead(attachable);
        }});

        if (m_pHead->HasNoSetDamageMultiplier()) { m_pHead->SetDamageMultiplier(4.0F); }
        if (m_pHead->IsDrawnAfterParent()) { m_pHead->SetDrawnNormallyByParent(false); }
        m_pHead->SetInheritsRotAngle(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetJetpack(AEmitter *newJetpack) {
    if (m_pJetpack && m_pJetpack->IsAttached()) { RemoveAndDeleteAttachable(m_pJetpack); }
    if (newJetpack == nullptr) {
        m_pJetpack = nullptr;
    } else {
        m_pJetpack = newJetpack;
        AddAttachable(newJetpack);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newJetpack->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetJetpack");
            dynamic_cast<AHuman *>(parent)->SetJetpack(castedAttachable);
        }});

        if (m_pJetpack->HasNoSetDamageMultiplier()) { m_pJetpack->SetDamageMultiplier(0.0F); }
        m_pJetpack->SetApplyTransferredForcesAtOffset(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetFGArm(Arm *newArm) {
    if (m_pFGArm && m_pFGArm->IsAttached()) { RemoveAndDeleteAttachable(m_pFGArm); }
    if (newArm == nullptr) {
        m_pFGArm = nullptr;
    } else {
        m_pFGArm = newArm;
        AddAttachable(newArm);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newArm->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Arm *castedAttachable = dynamic_cast<Arm *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetFGArm");
            dynamic_cast<AHuman *>(parent)->SetFGArm(castedAttachable);
        }});

        if (m_pFGArm->HasNoSetDamageMultiplier()) { m_pFGArm->SetDamageMultiplier(1.0F); }
        m_pFGArm->SetDrawnAfterParent(true);
        m_pFGArm->SetDrawnNormallyByParent(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetBGArm(Arm *newArm) {
    if (m_pBGArm && m_pBGArm->IsAttached()) { RemoveAndDeleteAttachable(m_pBGArm); }
    if (newArm == nullptr) {
        m_pBGArm = nullptr;
    } else {
        m_pBGArm = newArm;
        AddAttachable(newArm);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newArm->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Arm *castedAttachable = dynamic_cast<Arm *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetBGArm");
            dynamic_cast<AHuman *>(parent)->SetBGArm(castedAttachable);
        }});

        if (m_pBGArm->HasNoSetDamageMultiplier()) { m_pBGArm->SetDamageMultiplier(1.0F); }
        m_pBGArm->SetDrawnAfterParent(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetFGLeg(Leg *newLeg) {
    if (m_pFGLeg && m_pFGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pFGLeg); }
    if (newLeg == nullptr) {
        m_pFGLeg = nullptr;
    } else {
        m_pFGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetFGLeg");
            dynamic_cast<AHuman *>(parent)->SetFGLeg(castedAttachable);
        }});

        if (m_pFGLeg->HasNoSetDamageMultiplier()) { m_pFGLeg->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AHuman::SetBGLeg(Leg *newLeg) {
    if (m_pBGLeg && m_pBGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pBGLeg); }
    if (newLeg == nullptr) {
        m_pBGLeg = nullptr;
    } else {
        m_pBGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetBGLeg");
            dynamic_cast<AHuman *>(parent)->SetBGLeg(castedAttachable);
        }});

        if (m_pBGLeg->HasNoSetDamageMultiplier()) { m_pBGLeg->SetDamageMultiplier(1.0F); }
        m_pBGLeg->SetDrawnAfterParent(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BITMAP * AHuman::GetGraphicalIcon() const {
	return m_GraphicalIcon ? m_GraphicalIcon : (m_pHead ? m_pHead->GetSpriteFrame(0) : GetSpriteFrame(0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool AHuman::CollideAtPoint(HitData &hd)
{
    return Actor::CollideAtPoint(hd);

/*
    hd.ResImpulse[HITOR].Reset();
    hd.ResImpulse[HITEE].Reset();
    hd.HitRadius[HITEE] = (hd.HitPoint - m_Pos) * c_MPP;
    hd.mass[HITEE] = m_Mass;
    hd.MomInertia[HITEE] = m_pAtomGroup->GetMomentOfInertia();
    hd.HitVel[HITEE] = m_Vel + hd.HitRadius[HITEE].GetPerpendicular() * m_AngularVel;
    hd.VelDiff = hd.HitVel[HITOR] - hd.HitVel[HITEE];
    Vector hitAcc = -hd.VelDiff * (1 + hd.Body[HITOR]->GetMaterial().restitution * GetMaterial().restitution);

    float hittorLever = hd.HitRadius[HITOR].GetPerpendicular().Dot(hd.BitmapNormal);
    float hitteeLever = hd.HitRadius[HITEE].GetPerpendicular().Dot(hd.BitmapNormal);
    hittorLever *= hittorLever;
    hitteeLever *= hitteeLever;
    float impulse = hitAcc.Dot(hd.BitmapNormal) / (((1 / hd.mass[HITOR]) + (1 / hd.mass[HITEE])) +
                    (hittorLever / hd.MomInertia[HITOR]) + (hitteeLever / hd.MomInertia[HITEE]));

    hd.ResImpulse[HITOR] = hd.BitmapNormal * impulse * hd.ImpulseFactor[HITOR];
    hd.ResImpulse[HITEE] = hd.BitmapNormal * -impulse * hd.ImpulseFactor[HITEE];

    ////////////////////////////////////////////////////////////////////////////////
    // If a particle, which does not penetrate, but bounces, do any additional
    // effects of that bounce.
    if (!ParticlePenetration())
// TODO: Add blunt trauma effects here!")
        ;
    }

    m_Vel += hd.ResImpulse[HITEE] / hd.mass[HITEE];
    m_AngularVel += hd.HitRadius[HITEE].GetPerpendicular().Dot(hd.ResImpulse[HITEE]) /
                    hd.MomInertia[HITEE];
*/
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool AHuman::OnBounce(const Vector &pos)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool AHuman::OnSink(const Vector &pos)
{
    return false;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AHuman::HandlePieCommand(PieSlice::SliceType pieSliceIndex) {
    if (pieSliceIndex != PieSlice::SliceType::NoType) {
        if (pieSliceIndex == PieSlice::SliceType::Pickup) {
            m_Controller.SetState(WEAPON_PICKUP);
        } else if (pieSliceIndex == PieSlice::SliceType::Drop) {
            m_Controller.SetState(WEAPON_DROP);
        } else if (pieSliceIndex == PieSlice::SliceType::Reload) {
            m_Controller.SetState(WEAPON_RELOAD);
        } else if (pieSliceIndex == PieSlice::SliceType::NextItem) {
            m_Controller.SetState(WEAPON_CHANGE_NEXT, true);
        } else if (pieSliceIndex == PieSlice::SliceType::PreviousItem) {
            m_Controller.SetState(WEAPON_CHANGE_PREV, true);
        } else if (pieSliceIndex == PieSlice::SliceType::Sentry) {
            m_AIMode = AIMODE_SENTRY;
        } else if (pieSliceIndex == PieSlice::SliceType::Patrol) {
            m_AIMode = AIMODE_PATROL;
        } else if (pieSliceIndex == PieSlice::SliceType::BrainHunt) {
            m_AIMode = AIMODE_BRAINHUNT;
            ClearAIWaypoints();
        } else if (pieSliceIndex == PieSlice::SliceType::GoTo) {
            m_AIMode = AIMODE_GOTO;
            ClearAIWaypoints();
            m_UpdateMovePath = true;
        } else if (pieSliceIndex == PieSlice::SliceType::GoldDig) {
            m_AIMode = AIMODE_GOLDDIG;
        } else {
            return Actor::HandlePieCommand(pieSliceIndex);
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this AHuman. This also puts that item
//                  directly in the hands of this if they are empty.

void AHuman::AddInventoryItem(MovableObject *pItemToAdd) {
    // If we have nothing in inventory, and nothing in our hands, just grab this first thing added to us.
    if (HeldDevice *itemToAddAsHeldDevice = dynamic_cast<HeldDevice *>(pItemToAdd); itemToAddAsHeldDevice && m_Inventory.empty() && m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldDevice()) {
        m_pFGArm->SetHeldDevice(itemToAddAsHeldDevice);
        m_pFGArm->SetHandPos(m_HolsterOffset.GetXFlipped(m_HFlipped));
	} else {
		Actor::AddInventoryItem(pItemToAdd);
	}

    EquipShieldInBGArm();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MovableObject * AHuman::SwapNextInventory(MovableObject *inventoryItemToSwapIn, bool muteSound) {
	MovableObject *swappedInventoryItem = Actor::SwapNextInventory(inventoryItemToSwapIn, muteSound);
	while (!dynamic_cast<HeldDevice *>(swappedInventoryItem) && !m_Inventory.empty()) {
		g_MovableMan.AddMO(swappedInventoryItem);
		swappedInventoryItem = Actor::SwapNextInventory(nullptr, muteSound);
	}

	return swappedInventoryItem;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MovableObject * AHuman::SwapPrevInventory(MovableObject *inventoryItemToSwapIn) {
	MovableObject *swappedInventoryItem = Actor::SwapPrevInventory(inventoryItemToSwapIn);
	while (!dynamic_cast<HeldDevice *>(swappedInventoryItem) && !m_Inventory.empty()) {
		g_MovableMan.AddMO(swappedInventoryItem);
		swappedInventoryItem = Actor::SwapNextInventory(nullptr);
	}

	return swappedInventoryItem;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found firearm
//                  in the inventory. If the held device already is a firearm, or no
//                  firearm is in inventory, nothing happens.

bool AHuman::EquipFirearm(bool doEquip)
{
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

    if (HDFirearm *heldDeviceAsFirearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice()); heldDeviceAsFirearm && heldDeviceAsFirearm->IsWeapon()) {
        return true;
	} else {
		UnequipBGArm();
	}

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pWeapon && pWeapon->IsWeapon())
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice()) {
                    heldDevice->Deactivate();
                    AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldDevice(pWeapon);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm if applicable
                EquipShieldInBGArm();

                // Play the device switching sound
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
            }

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipDeviceInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found device
//                  of the specified group in the inventory. If the held device already
//                  is of that group, or no device is in inventory, nothing happens.

bool AHuman::EquipDeviceInGroup(std::string group, bool doEquip)
{
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && heldDevice->IsInGroup(group)) {
        return true;
    }

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HeldDevice *pDevice = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pDevice && pDevice->IsInGroup(group))
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice())
                {
                    heldDevice->Deactivate();
                    MovableObject *previouslyHeldItem = m_pFGArm->RemoveAttachable(heldDevice);
                    if (previouslyHeldItem) {
                        // Note - This is a fix to deal with an edge case bug when this method is called by a global script.
                        // Because the global script runs before everything has finished traveling, the removed item needs to undraw itself from the MO layer, otherwise it can result in ghost collisions and crashes.
                        if (previouslyHeldItem->GetsHitByMOs()) {
#ifdef DRAW_MOID_LAYER
                            previouslyHeldItem->Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawNoMOID, true);
#else
                            previouslyHeldItem->SetTraveling(true);
#endif
                        }
                        AddToInventoryBack(previouslyHeldItem);
                    }
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldDevice(pDevice);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm if applicable
                EquipShieldInBGArm();

                // Play the device switching sound
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
			}

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipLoadedFirearmInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first loaded HDFirearm
//                  of the specified group in the inventory. If no such weapon is in the
//                  inventory, nothing happens.

bool AHuman::EquipLoadedFirearmInGroup(std::string group, std::string excludeGroup, bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached())) {
        return false;
	}

    if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && !heldDevice->NeedsReloading() && heldDevice->IsInGroup(group) && !heldDevice->IsInGroup(excludeGroup)) {
        return true;
    }

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HDFirearm *pFirearm = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pFirearm && !pFirearm->NeedsReloading() && pFirearm->IsInGroup(group) && !pFirearm->IsInGroup(excludeGroup))
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldDevice(pFirearm);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm if applicable
                EquipShieldInBGArm();

                // Play the device switching sound
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
			}

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipNamedDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found device
//                  of with the specified preset name in the inventory. If the held device already
//                  is of that preset name, or no device is in inventory, nothing happens.

bool AHuman::EquipNamedDevice(const std::string &moduleName, const std::string &presetName, bool doEquip)
{
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	if (const HeldDevice *heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && (moduleName.empty() || heldDevice->GetModuleName() == moduleName) && heldDevice->GetPresetName() == presetName) {
        return true;
    }

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HeldDevice *pDevice = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pDevice && (moduleName.empty() || pDevice->GetModuleName() == moduleName) && pDevice->GetPresetName() == presetName)
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice())
                {
                    heldDevice->Deactivate();
                    AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldDevice(pDevice);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm if applicable
                EquipShieldInBGArm();

                // Play the device switching sound
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
			}

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipThrowable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found ThrownDevice
//                  in the inventory. If the held device already is a ThrownDevice, or no
//                  ThrownDevice  is in inventory, nothing happens.

bool AHuman::EquipThrowable(bool doEquip)
{
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

    if (dynamic_cast<ThrownDevice *>(m_pFGArm->GetHeldDevice())) {
		return true;
    }

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        ThrownDevice *pThrown = dynamic_cast<ThrownDevice *>(*itr);
        // Found proper device to equip, so make the switch!
// TODO: see if thrown is weapon or not, don't want to throw key items etc
        if (pThrown)// && pThrown->IsWeapon())
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice())
                {
                    heldDevice->Deactivate();
                    AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldDevice(pThrown);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm as applicable
                EquipShieldInBGArm();

                // Play the device switching sound
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
			}

            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool AHuman::EquipDiggingTool(bool doEquip) {
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

	const HDFirearm *strongestDigger = nullptr;
	float strongestDiggerDigStrength = 0;
	bool strongestDiggerIsHeld = false;
	if (const HDFirearm *heldDeviceAsFirearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice()); heldDeviceAsFirearm && heldDeviceAsFirearm->IsInGroup("Tools - Diggers")) {
		strongestDigger = heldDeviceAsFirearm;
		strongestDiggerDigStrength = heldDeviceAsFirearm->EstimateDigStrength();
		strongestDiggerIsHeld = true;
    }

	if (doEquip || !strongestDigger) {
		for (MovableObject *inventoryItem : m_Inventory) {
			if (const HDFirearm *inventoryItemAsFirearm = dynamic_cast<HDFirearm *>(inventoryItem); inventoryItemAsFirearm && inventoryItemAsFirearm->IsInGroup("Tools - Diggers") && inventoryItemAsFirearm->EstimateDigStrength() > strongestDiggerDigStrength) {
				strongestDigger = inventoryItemAsFirearm;
				strongestDiggerDigStrength = inventoryItemAsFirearm->EstimateDigStrength();
				strongestDiggerIsHeld = false;
			}
		}
	}

	if (doEquip && strongestDigger && !strongestDiggerIsHeld) {
		EquipNamedDevice(strongestDigger->GetModuleName(), strongestDigger->GetPresetName(), true);
	}

    return strongestDigger != nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////

float AHuman::EstimateDigStrength() const {
    float maxPenetration = Actor::EstimateDigStrength();

	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return maxPenetration;
	}

    if (const HDFirearm *heldDeviceAsHDFirearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice()); heldDeviceAsHDFirearm && heldDeviceAsHDFirearm->IsInGroup("Tools - Diggers")) {
		maxPenetration = std::max(heldDeviceAsHDFirearm->EstimateDigStrength(), maxPenetration);
    }

	for (const MovableObject *inventoryItem : m_Inventory) {
        if (const HDFirearm *inventoryItemAsFirearm = dynamic_cast<const HDFirearm *>(inventoryItem); inventoryItemAsFirearm && inventoryItemAsFirearm->IsInGroup("Tools - Diggers")) {
            maxPenetration = std::max(inventoryItemAsFirearm->EstimateDigStrength(), maxPenetration);
        }
    }

    return maxPenetration;
}

//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipShield
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found shield
//                  in the inventory. If the held device already is a shield, or no
//                  shield is in inventory, nothing happens.

bool AHuman::EquipShield()
{
	if (!(m_pFGArm && m_pFGArm->IsAttached())) {
		return false;
	}

    if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && heldDevice->IsShield()) {
        return true;
    }

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HeldDevice *pShield = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pShield && pShield->IsShield())
        {
            // Erase the inventory entry containing the device we now have switched to
            *itr = 0;
            m_Inventory.erase(itr);

            // Put back into the inventory what we had in our hands, if anything
            if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice())
            {
                heldDevice->Deactivate();
                AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
            }

            // Now put the device we were looking for and found into the hand
            m_pFGArm->SetHeldDevice(pShield);
            // Move the hand to a poisition so it looks like the new device was drawn from inventory
            m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

            // Equip shield in BG arm is applicable
            EquipShieldInBGArm();

            // Play the device switching sound
			if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipShieldInBGArm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tries to equip the first shield in inventory to the background arm;
//                  this only works if nothing is held at all, or the FG arm holds a
//                  one-handed device, or we're in inventory mode.

bool AHuman::EquipShieldInBGArm()
{
	if (!(m_pBGArm && m_pBGArm->IsAttached())) {
		return false;
	}

	if (HeldDevice *heldDevice = m_pBGArm->GetHeldDevice(); heldDevice && (heldDevice->IsShield() || heldDevice->IsDualWieldable())) {
		// If we're holding a shield, but aren't supposed to, because we need to support the FG hand's two-handed device, then let go of the shield and put it back in inventory.
		if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded()) {
			m_pBGArm->GetHeldDevice()->Deactivate();
			AddToInventoryBack(m_pBGArm->RemoveAttachable(heldDevice));
			return false;
		}
		return true;
	}

    // Only equip if the BG hand isn't occupied with supporting a two handed device
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded()) {
        return false;
	}

    // Go through the inventory looking for the proper device
    for (std::deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        HeldDevice *pShield = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pShield && (pShield->IsShield() || pShield->IsDualWieldable()))
        {
            // Erase the inventory entry containing the device we now have switched to
            *itr = 0;
            m_Inventory.erase(itr);

            // Put back into the inventory what we had in our hands, if anything
            if (HeldDevice *heldDevice = m_pBGArm->GetHeldDevice())
            {
                heldDevice->Deactivate();
                AddToInventoryBack(m_pBGArm->RemoveAttachable(heldDevice));
            }

            // Now put the device we were looking for and found into the hand
            m_pBGArm->SetHeldDevice(pShield);
            // Move the hand to a poisition so it looks like the new device was drawn from inventory
            m_pBGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

			if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }

            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool AHuman::UnequipFGArm() {
	if (m_pFGArm) {
		if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice()) {
			heldDevice->Deactivate();
			AddToInventoryBack(m_pFGArm->RemoveAttachable(heldDevice));
			m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool AHuman::UnequipBGArm() {
	if (m_pBGArm) {
		if (HeldDevice *heldDevice = m_pBGArm->GetHeldDevice()) {
			heldDevice->Deactivate();
			AddToInventoryFront(m_pBGArm->RemoveAttachable(heldDevice));
			m_pBGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

float AHuman::GetEquippedMass() const {
	float equippedMass = 0;
	if (MovableObject *fgDevice = GetEquippedItem()) {
		equippedMass += fgDevice->GetMass();
	}
	if (MovableObject *bgDevice = GetEquippedBGItem()) {
		equippedMass += bgDevice->GetMass();
	}
	return equippedMass;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held device's current mag is empty on
//                  ammo or not.

bool AHuman::FirearmIsReady() const
{
    // Check if the currently held device is already the desired type
    if (m_pFGArm && m_pFGArm->IsAttached())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice());
        if (pWeapon && pWeapon->GetRoundInMagCount() != 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  ThrowableIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held ThrownDevice's is ready to go.

bool AHuman::ThrowableIsReady() const
{
    // Check if the currently held thrown device is already the desired type
    if (m_pFGArm && m_pFGArm->IsAttached())
    {
        const ThrownDevice *pThrown = dynamic_cast<ThrownDevice *>(m_pFGArm->GetHeldDevice());
        if (pThrown)// && pThrown->blah() > 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.

bool AHuman::FirearmIsEmpty() const
{
    if (m_pFGArm && m_pFGArm->IsAttached())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice());
        if (pWeapon && pWeapon->GetRoundInMagCount() == 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.

bool AHuman::FirearmNeedsReload() const {
	if (const HDFirearm *fgWeapon = dynamic_cast<HDFirearm *>(GetEquippedItem()); fgWeapon && fgWeapon->NeedsReloading()) {
		return true;
	}
	if (const HDFirearm *bgWeapon = dynamic_cast<HDFirearm *>(GetEquippedBGItem()); bgWeapon && bgWeapon->NeedsReloading()) {
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AHuman::FirearmsAreReloading(bool onlyIfAllFirearmsAreReloading) const {
	int reloadingFirearmCount = 0;
	int totalFirearmCount = 0;

	if (const HDFirearm *fgWeapon = dynamic_cast<HDFirearm *>(GetEquippedItem())) {
		totalFirearmCount++;
		if (fgWeapon->IsReloading()) {
			reloadingFirearmCount++;
		}
	}
	if (reloadingFirearmCount > 0 && !onlyIfAllFirearmsAreReloading) {
		return true;
	}

	if (const HDFirearm *bgWeapon = dynamic_cast<HDFirearm *>(GetEquippedBGItem())) {
		totalFirearmCount++;
		if (bgWeapon->IsReloading()) {
			reloadingFirearmCount++;
		}
	}

	return onlyIfAllFirearmsAreReloading ? reloadingFirearmCount == totalFirearmCount : reloadingFirearmCount > 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.

bool AHuman::FirearmIsSemiAuto() const
{
    if (m_pFGArm && m_pFGArm->IsAttached())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice());
        return pWeapon && !pWeapon->IsFullAuto();
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  ReloadFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the currently held firearm, if any.
// Arguments:       None.
// Return value:    None.

void AHuman::ReloadFirearms(bool onlyReloadEmptyFirearms) {
	for (Arm *arm : { m_pFGArm, m_pBGArm }) {
		if (arm) {
			HDFirearm *heldFirearm = dynamic_cast<HDFirearm *>(arm->GetHeldDevice());
			if (!heldFirearm) {
				continue;
			}
			bool heldFirearmCanReload = heldFirearm->IsReloadable() && !heldFirearm->IsFull() && !heldFirearm->IsReloading() && (!onlyReloadEmptyFirearms || heldFirearm->IsEmpty());
			if (!heldFirearmCanReload) {
				continue;
			}

			Arm *otherArm = arm == m_pFGArm ? m_pBGArm : m_pFGArm;
			HDFirearm *otherHeldFirearm = otherArm ? dynamic_cast<HDFirearm *>(otherArm->GetHeldDevice()) : nullptr;

			bool reloadHeldFirearm = false;
			if (otherHeldFirearm && otherHeldFirearm->IsReloadable()) {
				if (heldFirearm->IsDualReloadable() && otherHeldFirearm->IsDualReloadable()) {
					reloadHeldFirearm = true;
				} else if (!otherHeldFirearm->IsReloading()) {
					reloadHeldFirearm = true;
					if (arm == m_pFGArm) {
						m_WaitingToReloadOffhand = true;
					}
				}
			} else {
				reloadHeldFirearm = true;
			}

			if (reloadHeldFirearm) {
				heldFirearm->Reload();
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
				bool otherArmIsAvailable = otherArm && !otherArm->GetHeldDevice();

				if (otherArmIsAvailable) {
					float delayAtTarget = std::max(static_cast<float>(heldFirearm->GetReloadTime() - 200), 0.0F);
					otherArm->AddHandTarget("Magazine Pos", heldFirearm->GetMagazinePos());
					if (!m_ReloadOffset.IsZero()) {
						otherArm->AddHandTarget("Reload Offset", m_Pos + RotateOffset(m_ReloadOffset), delayAtTarget);
					} else {
						otherArm->AddHandTarget("Holster Offset", m_Pos + RotateOffset(m_HolsterOffset), delayAtTarget);
					}
					otherArm->SetHandPos(heldFirearm->GetMagazinePos());
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the currently held device's delay between pulling the trigger
//                  and activating.

int AHuman::FirearmActivationDelay() const
{
    // Check if the currently held device is already the desired type
    if (m_pFGArm && m_pFGArm->IsAttached())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice());
        if (pWeapon)
            return pWeapon->GetActivationDelay();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsWithinRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is within range of the currently
//                  used device and aiming status, if applicable.

bool AHuman::IsWithinRange(Vector &point) const
{
    if (m_SharpAimMaxedOut)
        return true;

    Vector diff = g_SceneMan.ShortestDistance(m_Pos, point, false);
    float sqrDistance = diff.GetSqrMagnitude();

	// Really close!
	if (sqrDistance <= (m_CharHeight * m_CharHeight)) {
		return true;
	}

    float range = 0;

    if (FirearmIsReady())
    {
        // Start with the default aim distance
        range = m_AimDistance;

        // Add the sharp range of the equipped weapon
        if (m_pFGArm && m_pFGArm->IsAttached())
            range += m_pFGArm->GetHeldDevice()->GetSharpLength() + 150;
    }
    else if (ThrowableIsReady())
    {
// TODO: make proper throw range calc based on the throwable's mass etc
        range += m_CharHeight * 4;
    }

    return sqrDistance <= (range * range);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.

bool AHuman::Look(float FOVSpread, float range)
{
    if (!g_SceneMan.AnythingUnseen(m_Team) || m_CanRevealUnseen == false)
        return false;

    // Set the length of the look vector
    float aimDistance = m_AimDistance + range;
    Vector aimPos = m_Pos;

    // If aiming down the barrel, look through that
    if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
    {
        aimPos = m_pFGArm->GetHeldDevice()->GetPos();
        aimDistance += m_pFGArm->GetHeldDevice()->GetSharpLength();
    }
    // If just looking, use the eyes on the head instead
    else if (m_pHead && m_pHead->IsAttached())
    {
        aimPos = GetEyePos();
    }

    // Create the vector to trace along
    Vector lookVector(aimDistance, 0);
    // Set the rotation to the actual aiming angle
    Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
    aimMatrix.SetXFlipped(m_HFlipped);
    lookVector *= aimMatrix;
    // Add the spread
	lookVector.DegRotate(FOVSpread * RandomNormalNum());

    // TODO: generate an alarm event if we spot an enemy actor?

	Vector ignored;
    // Cast the seeing ray, adjusting the skip to match the resolution of the unseen map
    return g_SceneMan.CastSeeRay(m_Team, aimPos, lookVector, ignored, 25, (int)g_SceneMan.GetUnseenResolution(m_Team).GetSmallest() / 2);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LookForGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts a material detecting ray in the direction of where this is facing.

bool AHuman::LookForGold(float FOVSpread, float range, Vector &foundLocation) const
{
    Vector ray(m_HFlipped ? -range : range, 0);
	ray.DegRotate(FOVSpread * RandomNormalNum());

    return g_SceneMan.CastMaterialRay(m_Pos, ray, g_MaterialGold, foundLocation, 4);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LookForMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an MO detecting ray in the direction of where the head is looking
//                  at the time. Factors including head rotation, sharp aim mode, and
//                  other variables determine how this ray is cast.

MovableObject * AHuman::LookForMOs(float FOVSpread, unsigned char ignoreMaterial, bool ignoreAllTerrain)
{
    MovableObject *pSeenMO = 0;
    Vector aimPos = m_Pos;
    float aimDistance = m_AimDistance + g_FrameMan.GetPlayerScreenWidth() * 0.51;   // Set the length of the look vector

    // If aiming down the barrel, look through that
    if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
    {
        aimPos = m_pFGArm->GetHeldDevice()->GetPos();
        aimDistance += m_pFGArm->GetHeldDevice()->GetSharpLength();
    }
    // If just looking, use the eyes on the head instead
    else if (m_pHead && m_pHead->IsAttached())
    {
        aimPos = GetEyePos();
    }

    // Create the vector to trace along
    Vector lookVector(aimDistance, 0);
    // Set the rotation to the actual aiming angle
    Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
    aimMatrix.SetXFlipped(m_HFlipped);
    lookVector *= aimMatrix;
    // Add the spread
    lookVector.DegRotate(FOVSpread * RandomNormalNum());

    MOID seenMOID = g_SceneMan.CastMORay(aimPos, lookVector, m_MOID, IgnoresWhichTeam(), ignoreMaterial, ignoreAllTerrain, 5);
    pSeenMO = g_MovableMan.GetMOFromID(seenMOID);
    if (pSeenMO)
        return pSeenMO->GetRootParent();

    return pSeenMO;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.

void AHuman::ResetAllTimers()
{
    Actor::ResetAllTimers();

    if (m_pJetpack)
        m_pJetpack->ResetAllTimers();

    if (m_pFGArm && m_pFGArm->GetHeldDevice())
        m_pFGArm->GetHeldDevice()->ResetAllTimers();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the path to move along to the currently set movetarget.

bool AHuman::UpdateMovePath()
{
    // Do the real path calc; abort and pass along the message if it didn't happen due to throttling
    if (!Actor::UpdateMovePath())
        return false;

    // Process the new path we now have, if any
    if (!m_MovePath.empty())
    {
        // Smash all airborne waypoints down to just above the ground, except for when it makes the path intersect terrain or it is the final destination
        std::list<Vector>::iterator finalItr = m_MovePath.end();
        finalItr--;
        Vector smashedPoint;
        Vector previousPoint = *(m_MovePath.begin());
        std::list<Vector>::iterator nextItr = m_MovePath.begin();
        for (std::list<Vector>::iterator lItr = m_MovePath.begin(); lItr != finalItr; ++lItr)
        {
            nextItr++;
            smashedPoint = g_SceneMan.MovePointToGround((*lItr), m_CharHeight*0.2, 7);

            // Only smash if the new location doesn't cause the path to intersect hard terrain ahead or behind of it
            // Try three times to halve the height to see if that won't intersect
            for (int i = 0; i < 3; i++)
            {
                Vector notUsed;
                if (!g_SceneMan.CastStrengthRay(previousPoint, smashedPoint - previousPoint, 5, notUsed, 3, g_MaterialDoor) &&
                    nextItr != m_MovePath.end() && !g_SceneMan.CastStrengthRay(smashedPoint, (*nextItr) - smashedPoint, 5, notUsed, 3, g_MaterialDoor))
                {
                    (*lItr) = smashedPoint;
                    break;
                }
                else
                    smashedPoint.m_Y -= ((smashedPoint.m_Y - (*lItr).m_Y) / 2);
            }

            previousPoint = (*lItr);
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

void AHuman::UpdateWalkAngle(AHuman::Layer whichLayer) {
	if (m_Controller.IsState(BODY_JUMP)) {
		m_WalkAngle[whichLayer] = Matrix(c_QuarterPI * GetFlipFactor());
	} else {
		float rayLength = 15.0F;
		Vector hipPos = m_Pos;
		if (whichLayer == AHuman::Layer::FGROUND && m_pFGLeg) {
			rayLength += m_pFGLeg->GetMaxLength();
			hipPos += RotateOffset(m_pFGLeg->GetParentOffset());
		} else if (m_pBGLeg) {
			rayLength += m_pBGLeg->GetMaxLength();
			hipPos += RotateOffset(m_pBGLeg->GetParentOffset());
		}

		// Cast a ray down from the left and right of us, to determine our angle of ascent
		//TODO Don't use a magic number here, calculate something based on stride length and maybe footgroup width.
		Vector hitPosLeft = hipPos + Vector(-10.0F, 0.0F);
		Vector hitPosRight = hipPos + Vector(10.0F, 0.0F);
		g_SceneMan.CastStrengthRay(hitPosLeft, Vector(0.0F, rayLength), 10.0F, hitPosLeft, 0, g_MaterialGrass);
		g_SceneMan.CastStrengthRay(hitPosRight, Vector(0.0F, rayLength), 10.0F, hitPosRight, 0, g_MaterialGrass);

		// Clamp the max angle, so we don't end up trying to walk at a 80 degree angle up sheer walls
		const float maxAngleDegrees = 40.0F;
		float terrainRotationDegs = std::clamp((hitPosRight - hitPosLeft).GetAbsDegAngle(), -maxAngleDegrees, maxAngleDegrees);

		Matrix walkAngle;
		walkAngle.SetDegAngle(terrainRotationDegs);
		m_WalkAngle[whichLayer] = walkAngle;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.

void AHuman::UpdateAI()
{
    Actor::UpdateAI();

    Vector cpuPos = GetCPUPos();
    MovableObject *pSeenMO = 0;
    Actor *pSeenActor = 0;

    ///////////////////////////////////////////////
    // React to relevant AlarmEvents

	const std::vector<AlarmEvent> &events = g_MovableMan.GetAlarmEvents();
	if (!events.empty()) {
		Vector alarmVec;
		Vector sensorPos = GetEyePos();
		for (const AlarmEvent &alarmEvent : events) {
			// Caused by some other team's activites - alarming!
			if (alarmEvent.m_Team != m_Team) {
				// See how far away the alarm situation is
				alarmVec = g_SceneMan.ShortestDistance(sensorPos, alarmEvent.m_ScenePos);
				// Only react if the alarm is within range and this is perceptive enough to hear it
				if (alarmVec.GetLargest() <= alarmEvent.m_Range * m_Perceptiveness) {
					Vector zero;
					// Now check if we have line of sight to the alarm point
					// Don't check all the way to the target, we are checking for no obstacles, and target will be an obstacle in itself
					if (g_SceneMan.CastObstacleRay(sensorPos, alarmVec * 0.9F, zero, zero, m_RootMOID, IgnoresWhichTeam(), g_MaterialGrass, 5) < 0 && g_SceneMan.ShortestDistance(m_LastAlarmPos, alarmEvent.m_ScenePos).GetLargest() > 10) {
						// If this is the same alarm location as last, then don't repeat the signal
							// Yes! WE ARE ALARMED!
						AlarmPoint(alarmEvent.m_ScenePos);
						break;
					}
				}
			}
		}
	}

    ////////////////////////////////////////////////
    // AI MODES

	// Squad logic
	if (m_AIMode == AIMODE_SQUAD) {
		m_AIMode = AIMODE_GOTO;
	}

    // If alarmed, override all modes, look at the alarming point
    if (!m_AlarmTimer.IsPastSimTimeLimit())
    {
        // Freeze!
        m_LateralMoveState = LAT_STILL;

        // If we're unarmed, hit the deck!
        if (!EquipFirearm() && !EquipThrowable() && !EquipDiggingTool())
        {
            m_Controller.SetState(BODY_CROUCH, true);
            // Also hit the deck completely and crawl away, not jsut sit down in place
            m_Controller.SetState(m_HFlipped ? MOVE_LEFT : MOVE_RIGHT, true);
        }

        // We're not stuck, just waiting and watching
        m_StuckTimer.Reset();
        // If we're not already engaging a target, jsut point in the direciton we heard the alarm come from
        if (m_DeviceState != AIMING && m_DeviceState != FIRING && m_DeviceState != THROWING)
        {
            // Look/point in the direction of alarm, the point should already ahve been set
            m_DeviceState = POINTING;
        }
    }
    // Patrolling
    else if (m_AIMode == AIMODE_PATROL)
    {
        m_SweepCenterAimAngle = 0;
        m_SweepRange = c_EighthPI;

        if (m_LateralMoveState == LAT_STILL)
        {
            // Avoid seeming stuck if we're waiting to turn
            m_StuckTimer.Reset();

            if (m_PatrolTimer.IsPastSimMS(2000))
            {
                m_PatrolTimer.Reset();
                m_LateralMoveState = m_HFlipped ? LAT_RIGHT : LAT_LEFT;
            }
        }
        else
        {
			Vector hitPos;
			Vector trace((m_LateralMoveState == LAT_RIGHT ? GetRadius() : -GetRadius()) * 0.5F, 0);
			// Stop and turn around after a period of time, or if bumped into another actor (like a rocket), or if walking off a ledge.
			if (m_PatrolTimer.IsPastSimMS(8000) || g_SceneMan.CastMORay(m_Pos, trace, m_MOID, IgnoresWhichTeam(), g_MaterialGrass, false, 5) != g_NoMOID || !g_SceneMan.CastStrengthRay(m_Pos + trace, Vector(0, GetRadius()), 5.0F, hitPos, 5, g_MaterialGrass)) {
				m_PatrolTimer.Reset();
				m_LateralMoveState = LAT_STILL;
			}
        }
    }
    // Going to a goal, potentially through a set of waypoints
    else if (m_AIMode == AIMODE_GOTO)
    {
        // Calculate the path to the target brain if need for refresh (note updating each pathfindingupdated causes small chug, maybe space em out with a timer?)
        // Also if we're way off form the path, or haven't made progress toward the current waypoint in a while, update the path to see if we can improve
        // Also if we seem to have completed the path to the current waypoint, we should update to get the path to the next waypoint
        if (m_UpdateMovePath || (m_ProgressTimer.IsPastSimMS(10000) && m_DeviceState != DIGGING) || (m_MovePath.empty() && m_MoveVector.GetLargest() < m_CharHeight * 0.5F))// || (m_MoveVector.GetLargest() > m_CharHeight * 2))// || g_SceneMan.GetScene()->PathFindingUpdated())
        {
            // Also never update while jumping
            if (m_ObstacleState != JUMPING)
                UpdateMovePath();
        }

        // If we used to be pointing at something (probably alarmed), just scan ahead instead
        if (m_DeviceState == POINTING)
            m_DeviceState = SCANNING;

        // Digging has its own advancement modes
        if (m_DeviceState != DIGGING)
        {
			Vector notUsed;
            Vector pathPointVec;
            // See if we are close enough to the next move target that we should grab the next in the path that is out of proximity range
            for (std::list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end();)
            {
                pathPointVec = g_SceneMan.ShortestDistance(m_Pos, *lItr);
                // Make sure we are within range AND have a clear sight to the waypoint we're about to eliminate, or it might be around a corner
                if (pathPointVec.GetLargest() <= m_MoveProximityLimit && !g_SceneMan.CastStrengthRay(m_Pos, pathPointVec, 5, notUsed, 0, g_MaterialDoor))
                {
                    lItr++;
                    // Save the last one before being popped off so we can use it to check if we need to dig (if there's any material between last and current)
                    m_PrevPathTarget = m_MovePath.front();
                    m_MovePath.pop_front();
                }
                else
                    break;
            }

            // If still stuff in the path, get the next point on it
            if (!m_MovePath.empty())
                m_MoveTarget = m_MovePath.front();
        }

        // Determine the direction to walk to get to the next move target, or if to simply stay still
        m_MoveVector = g_SceneMan.ShortestDistance(m_Pos, m_MoveTarget);
        if ((m_MoveVector.m_X > 0 && m_LateralMoveState == LAT_LEFT) || (m_MoveVector.m_X < 0 && m_LateralMoveState == LAT_RIGHT) || (m_LateralMoveState == LAT_STILL && m_DeviceState != AIMING && m_DeviceState != FIRING))
        {
			// Stay still and switch to sentry mode if we're close enough to the final destination.
			if (m_Waypoints.empty() && m_MovePath.empty() && std::abs(m_MoveVector.m_X) < 10.0F) {
				m_LateralMoveState = LAT_STILL;
				m_DeviceState = SCANNING;
				if (!m_pMOMoveTarget) { m_AIMode = AIMODE_SENTRY; }
			}
            // Turns only after a delay to avoid getting stuck on switchback corners in corridors
            else if (m_MoveOvershootTimer.IsPastSimMS(500) || m_LateralMoveState == LAT_STILL)
                m_LateralMoveState = m_LateralMoveState == LAT_RIGHT ? LAT_LEFT : LAT_RIGHT;
        }
        else
            m_MoveOvershootTimer.Reset();

        // Calculate and set the sweep center for the bots to be pointing to the target location
        if (m_DeviceState == SCANNING)
        {
            Vector targetVector(fabs(m_MoveVector.m_X), m_MoveVector.m_Y);
            m_SweepCenterAimAngle = targetVector.GetAbsRadAngle();
            m_SweepRange = c_SixteenthPI;
        }
    }
    // Brain hunting
    else if (m_AIMode == AIMODE_BRAINHUNT)
    {
        // Just set up the closest brain as a target and switch to GOTO mode
        const Actor *pTargetBrain = g_MovableMan.GetClosestBrainActor(m_Team == 0 ? 1 : 0, m_Pos);
        if (pTargetBrain)
        {
            m_UpdateMovePath = true;
            AddAIMOWaypoint(pTargetBrain);
        }
        // Couldn't find any, so stay put
        else
            m_MoveTarget = m_Pos;

        // If we used to be pointing at something (probably alarmed), just scan ahead instead
        if (m_DeviceState == POINTING)
            m_DeviceState = SCANNING;

        m_AIMode = AIMODE_GOTO;
    }
    // Gold digging
    else if (m_AIMode == AIMODE_GOLDDIG)
    {
        m_SweepRange = c_EighthPI;

        // Only dig if we have a tool for it
        if (EquipDiggingTool())
        {
            Vector newGoldPos;
            // Scan for gold, slightly more than the facing direction arc
            if (LookForGold(100, m_SightDistance / 2, newGoldPos))
            {
                // Start digging when gold is spotted and tool is ready
                m_DeviceState = DIGGING;

                // Only replace the target if the one we found is closer, or the old one isn't gold anymore
                Vector newGoldDir = newGoldPos - m_Pos;
                Vector oldGoldDir = m_DigTarget - m_Pos;
                if (newGoldDir.GetSqrMagnitude() < oldGoldDir.GetSqrMagnitude() || g_SceneMan.GetTerrain()->GetMaterialPixel(m_DigTarget.m_X, m_DigTarget.m_Y) != g_MaterialGold)
                {
                    m_DigTarget = newGoldPos;
                    m_StuckTimer.Reset();
                }

                // Turn around if the target is behind us
                m_HFlipped = m_DigTarget.m_X < m_Pos.m_X;
            }
            // If we can't see any gold, and our current target is out of date, then stop pressing the trigger
            else if (g_SceneMan.GetTerrain()->GetMaterialPixel(m_DigTarget.m_X, m_DigTarget.m_Y) != g_MaterialGold)
                m_DeviceState = STILL;

            // Figure out which direction to be digging in.
            Vector goldDir = m_DigTarget - m_Pos;
            m_SweepCenterAimAngle = goldDir.GetAbsRadAngle();

            // Move if there is space or a cavity to move into
            Vector moveRay(m_CharHeight / 2, 0);
            moveRay.AbsRotateTo(goldDir);
// TODO; Consider backstepping implications here, want to override it every time?
            if (g_SceneMan.CastNotMaterialRay(m_Pos, moveRay, g_MaterialAir, 3, false) < 0)
            {
                m_ObstacleState = PROCEEDING;
                m_LateralMoveState = m_HFlipped ? LAT_LEFT : LAT_RIGHT;
            }
            else
            {
                m_ObstacleState = DIGPAUSING;
                m_LateralMoveState = LAT_STILL;
            }
        }
        // Otherwise just stand sentry with a gun
        else
        {
            EquipFirearm();
            m_LateralMoveState = LAT_STILL;
            m_SweepCenterAimAngle = 0;
            m_SweepRange = c_EighthPI;
        }
    }
    // Sentry
    else
    {
        m_LateralMoveState = LAT_STILL;
        m_SweepCenterAimAngle = 0;
        m_SweepRange = c_EighthPI;
    }

    ///////////////////////////////
    // DEVICE LOGIC

    // If there's a digger on the ground and we don't have one, pick it up
    if (m_pItemInReach && m_pItemInReach->IsTool() && !EquipDiggingTool(false))
        m_Controller.SetState(WEAPON_PICKUP, true);

    // Still, pointing at the movetarget
    if (m_DeviceState == STILL)
    {
        m_SweepCenterAimAngle = FacingAngle(g_SceneMan.ShortestDistance(cpuPos, m_MoveTarget).GetAbsRadAngle());
        // Aim to point there
        float aimAngle = GetAimAngle(false);
        if (aimAngle < m_SweepCenterAimAngle && aimAngle < c_HalfPI)
        {
            m_Controller.SetState(AIM_UP, true);
        }
        else if (aimAngle > m_SweepCenterAimAngle && aimAngle > -c_HalfPI)
        {
            m_Controller.SetState(AIM_DOWN, true);
        }
    }
    // Pointing at a specifc target
    else if (m_DeviceState == POINTING)
    {
        Vector targetVector = g_SceneMan.ShortestDistance(GetEyePos(), m_PointingTarget, false);
        m_Controller.m_AnalogAim = targetVector;
        m_Controller.m_AnalogAim.CapMagnitude(1.0);
/* Old digital way, now jsut use analog aim instead
        // Do the actual aiming; first figure out which direction to aim in
        float aimAngleDiff = targetVector.GetAbsRadAngle() - GetLookVector().GetAbsRadAngle();
        // Flip if we're flipped
        aimAngleDiff = IsHFlipped() ? -aimAngleDiff : aimAngleDiff;
        // Now send the command to move aim in the appropriate direction
        m_ControlStates[aimAngleDiff > 0 ? AIM_UP : AIM_DOWN] = true;
*/
/*
        m_SweepCenterAimAngle = FacingAngle(g_SceneMan.ShortestDistance(cpuPos, m_PointingTarget).GetAbsRadAngle());
        // Aim to point there
        float aimAngle = GetAimAngle(false);
        if (aimAngle < m_SweepCenterAimAngle && aimAngle < c_HalfPI)
        {
            m_Controller.SetState(AIM_UP, true);
        }
        else if (aimAngle > m_SweepCenterAimAngle && aimAngle > -c_HalfPI)
        {
            m_Controller.SetState(AIM_DOWN, true);
        }
*/
        // Narrow FOV range scan, 10 degrees each direction
        pSeenMO = LookForMOs(10, g_MaterialGrass, false);
        // Saw something!
        if (pSeenMO)
        {
            pSeenActor = dynamic_cast<Actor *>(pSeenMO->GetRootParent());
            // ENEMY SIGHTED! Switch to a weapon with ammo if we haven't already
            if (pSeenActor && pSeenActor->GetTeam() != m_Team && (EquipFirearm() || EquipThrowable() || EquipDiggingTool()))
            {
                // Start aiming or throwing toward that target, depending on what we have in hands
                if (FirearmIsReady())
                {
                    m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
                    if (IsWithinRange(m_SeenTargetPos))
                    {
                        m_DeviceState = AIMING;
                        m_FireTimer.Reset();
                    }
                }
                else if (ThrowableIsReady())
                {
                    m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
                    // Only throw if within range
                    if (IsWithinRange(m_SeenTargetPos))
                    {
                        m_DeviceState = THROWING;
                        m_FireTimer.Reset();
                    }
                }
            }
        }
    }
    // Digging
    else if (m_DeviceState == DIGGING)
    {
        // Switch to the digger if we have one
        if (EquipDiggingTool())
        {
            // Reload if it's empty
            if (FirearmIsEmpty())
                m_Controller.SetState(WEAPON_RELOAD, true);
            // Everything's ready - dig away!
            else
            {
                // Pull the trigger on the digger, if we're not backstepping or a teammate is in the way!
                m_Controller.SetState(WEAPON_FIRE, m_ObstacleState != BACKSTEPPING && m_TeamBlockState != BLOCKED);

                // Finishing off a tunnel, so aim squarely for the end tunnel positon
                if (m_DigState == FINISHINGDIG)
                    m_SweepCenterAimAngle = FacingAngle(g_SceneMan.ShortestDistance(cpuPos, m_DigTunnelEndPos).GetAbsRadAngle());
                // Tunneling: update the digging direction, aiming exactly between the prev target and the current one
                else
                {
                    Vector digTarget = m_PrevPathTarget + (g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget) * 0.5);
                    // Flip us around if we're facing away from the dig target, also don't dig
                    if (digTarget.m_X > m_Pos.m_X && m_HFlipped)
                    {
                        m_LateralMoveState = LAT_RIGHT;
                        m_Controller.SetState(WEAPON_FIRE, false);
                    }
                    else if (digTarget.m_X < m_Pos.m_X && !m_HFlipped)
                    {
                        m_LateralMoveState = LAT_LEFT;
                        m_Controller.SetState(WEAPON_FIRE, false);
                    }
                    m_SweepCenterAimAngle = FacingAngle(g_SceneMan.ShortestDistance(cpuPos, digTarget).GetAbsRadAngle());
                }

                // Sweep digging up and down
                if (m_SweepState == SWEEPINGUP && m_TeamBlockState != BLOCKED)
                {
                    float aimAngle = GetAimAngle(false);
                    if (aimAngle < m_SweepCenterAimAngle + m_SweepRange && aimAngle < c_HalfPI)
                    {
                        m_Controller.SetState(AIM_UP, true);
                    }
                    else
                    {
                        m_SweepState = SWEEPUPPAUSE;
                        m_SweepTimer.Reset();
                    }
                }
                else if (m_SweepState == SWEEPUPPAUSE && m_SweepTimer.IsPastSimMS(10))
                {
                    m_SweepState = SWEEPINGDOWN;
                }
                else if (m_SweepState == SWEEPINGDOWN && m_TeamBlockState != BLOCKED)
                {
                    float aimAngle = GetAimAngle(false);
                    if (aimAngle > m_SweepCenterAimAngle - m_SweepRange && aimAngle > -c_HalfPI)
                    {
                        m_Controller.SetState(AIM_DOWN, true);
                    }
                    else
                    {
                        m_SweepState = SWEEPDOWNPAUSE;
                        m_SweepTimer.Reset();
                    }
                }
                else if (m_SweepState == SWEEPDOWNPAUSE && m_SweepTimer.IsPastSimMS(10))
                {
                    m_SweepState = SWEEPINGUP;
                }

                // See if we have dug out all that we can in the sweep area without moving closer
// TODO: base the range on the digger's actual range, quereied from teh digger itself
                Vector centerRay(m_CharHeight * 0.45, 0);
                centerRay.RadRotate(GetAimAngle(true));
                if (g_SceneMan.CastNotMaterialRay(cpuPos, centerRay, g_MaterialAir, 3) < 0)
                {
                    // Now check the tunnel's thickness
                    Vector upRay(m_CharHeight * 0.4, 0);
                    upRay.RadRotate(GetAimAngle(true) + m_SweepRange * 0.5);
                    Vector downRay(m_CharHeight * 0.4, 0);
                    downRay.RadRotate(GetAimAngle(true) - m_SweepRange * 0.5);
                    if (g_SceneMan.CastNotMaterialRay(cpuPos, upRay, g_MaterialAir, 3) < 0 &&
                        g_SceneMan.CastNotMaterialRay(cpuPos, downRay, g_MaterialAir, 3) < 0)
                    {
                        // Ok the tunnel section is clear, so start walking forward while still digging
                        m_ObstacleState = PROCEEDING;
                    }
                    // Tunnel cavity not clear yet, so stay put and dig some more
                    else if (m_ObstacleState != BACKSTEPPING)
                        m_ObstacleState = DIGPAUSING;
                }
                // Tunnel cavity not clear yet, so stay put and dig some more
                else if (m_ObstacleState != BACKSTEPPING)
                    m_ObstacleState = DIGPAUSING;

                // When we get close enough to the next point and clear it, advance it and stop again to dig some more
                if (m_DigState != FINISHINGDIG && (fabs(m_PrevPathTarget.m_X - m_Pos.m_X) < (m_CharHeight * 0.33)))
                {
					Vector notUsed;

                    // If we have cleared the buried path segment, advance to the next
                    if (!g_SceneMan.CastStrengthRay(m_PrevPathTarget, g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget), 5, notUsed, 1, g_MaterialDoor))
                    {
                        // Advance to the next one, if there are any
                        if (m_MovePath.size() >= 2)
                        {
                            m_PrevPathTarget = m_MovePath.front();
                            m_MovePath.pop_front();
                            m_MoveTarget = m_MovePath.front();
                        }

                        // WE HAVE BROKEN THROUGH WITH THIS TUNNEL (but not yet cleared it enough for passing through)!
                        // If the path segment is now in the air again, and the tunnel cavity is clear, then go into finishing digging mode
                        if (!g_SceneMan.CastStrengthRay(m_PrevPathTarget, g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget), 5, notUsed, 1, g_MaterialDoor))
                        {
                            m_DigTunnelEndPos = m_MoveTarget;
                            m_DigState = FINISHINGDIG;
                        }
                    }
                }

                // If we have broken through to the end of the tunnel, but not yet cleared it completely, then keep digging until the end tunnel position is hit
                if (m_DigState == FINISHINGDIG && g_SceneMan.ShortestDistance(m_Pos, m_DigTunnelEndPos).m_X < (m_CharHeight * 0.33))
                {
                    // DONE DIGGING THIS FUCKING TUNNEL, PROCEED
                    m_ObstacleState = PROCEEDING;
                    m_DeviceState = SCANNING;
                    m_DigState = NOTDIGGING;
                }
            }
        }
        // If we need to and can, pick up any weapon on the ground
        else if (m_pItemInReach)
        {
            m_Controller.SetState(WEAPON_PICKUP, true);
            // Can't be digging without a tool, fool
            m_DeviceState = SCANNING;
            m_DigState = NOTDIGGING;
        }
    }
    // Look for, aim at, and fire upon enemy Actors
    else if (m_DeviceState == SCANNING)
    {
        if (m_SweepState == NOSWEEP)
            m_SweepState = SWEEPINGUP;

        // Try to switch to, and if necessary, reload a firearm when we are scanning
        if (EquipFirearm())
        {
            // Reload if necessary
            if (FirearmNeedsReload())
                m_Controller.SetState(WEAPON_RELOAD, true);
        }
        // Use digger instead if we have one!
        else if (EquipDiggingTool())
        {
            if (FirearmIsEmpty())
                m_Controller.SetState(WEAPON_RELOAD, true);
        }
        // If we need to and can, pick up any weapon on the ground
        else if (m_pItemInReach)
            m_Controller.SetState(WEAPON_PICKUP, true);

        // Scan aiming up and down
        if (m_SweepState == SWEEPINGUP)
        {
            float aimAngle = GetAimAngle(false);
            if (aimAngle < m_SweepCenterAimAngle + m_SweepRange && aimAngle < c_HalfPI)
            {
                m_Controller.SetState(AIM_UP, true);
            }
            else
            {
                m_SweepState = SWEEPUPPAUSE;
                m_SweepTimer.Reset();
            }
        }
        else if (m_SweepState == SWEEPUPPAUSE && m_SweepTimer.IsPastSimMS(1000))
        {
            m_SweepState = SWEEPINGDOWN;
        }
        else if (m_SweepState == SWEEPINGDOWN)
        {
            float aimAngle = GetAimAngle(false);
            if (aimAngle > m_SweepCenterAimAngle - m_SweepRange && aimAngle > -c_HalfPI)
            {
                m_Controller.SetState(AIM_DOWN, true);
            }
            else
            {
                m_SweepState = SWEEPDOWNPAUSE;
                m_SweepTimer.Reset();
            }
        }
        else if (m_SweepState == SWEEPDOWNPAUSE && m_SweepTimer.IsPastSimMS(1000))
        {
            m_SweepState = SWEEPINGUP;
        }
/*
        // Scan aiming up and down
        if (GetViewPoint().m_Y > m_Pos.m_Y + 2)
            m_ControlStates[AIM_UP] = true;
        else if (GetViewPoint().m_Y < m_Pos.m_Y - 2)
            m_ControlStates[AIM_DOWN] = true;
*/
        // Wide FOV range scan, 25 degrees each direction
        pSeenMO = LookForMOs(25, g_MaterialGrass, false);
        // Saw something!
        if (pSeenMO)
        {
            pSeenActor = dynamic_cast<Actor *>(pSeenMO->GetRootParent());
            // ENEMY SIGHTED! Switch to a weapon with ammo if we haven't already
            if (pSeenActor && pSeenActor->GetTeam() != m_Team && (EquipFirearm() || EquipThrowable() || EquipDiggingTool()))
            {
                // Start aiming or throwing toward that target, depending on what we have in hands
                if (FirearmIsReady())
                {
                    m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
                    if (IsWithinRange(m_SeenTargetPos))
                    {
                        m_DeviceState = AIMING;
                        m_FireTimer.Reset();
                    }
                }
                else if (ThrowableIsReady())
                {
                    m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
                    // Only throw if within range
                    if (IsWithinRange(m_SeenTargetPos))
                    {
                        m_DeviceState = THROWING;
                        m_FireTimer.Reset();
                    }
                }
            }
        }
    }
    // Aiming toward spotted target to confirm enemy presence
    else if (m_DeviceState == AIMING)
    {
        // Aim carefully!
        m_Controller.SetState(AIM_SHARP, true);

        // If we're alarmed, then hit the deck! while aiming
        if (!m_AlarmTimer.IsPastSimTimeLimit())
        {
            m_Controller.SetState(BODY_CROUCH, true);
            // Also hit the deck completely, not jsut sit down in place
            if (m_ProneState != PRONE)
                m_Controller.SetState(m_HFlipped ? MOVE_LEFT : MOVE_RIGHT, true);
        }

        Vector targetVector = g_SceneMan.ShortestDistance(GetEyePos(), m_SeenTargetPos, false);
        m_Controller.m_AnalogAim = targetVector;
        m_Controller.m_AnalogAim.CapMagnitude(1.0);
/* Old digital way, now jsut use analog aim instead
        // Do the actual aiming; first figure out which direction to aim in
        float aimAngleDiff = targetVector.GetAbsRadAngle() - GetLookVector().GetAbsRadAngle();
        // Flip if we're flipped
        aimAngleDiff = IsHFlipped() ? -aimAngleDiff : aimAngleDiff;
        // Now send the command to move aim in the appropriate direction
        m_ControlStates[aimAngleDiff > 0 ? AIM_UP : AIM_DOWN] = true;
*/
        // Narrow focused FOV range scan
        pSeenMO = LookForMOs(10, g_MaterialGrass, false);

        // Saw the enemy actor again through the sights!
        if (pSeenMO)
            pSeenActor = dynamic_cast<Actor *>(pSeenMO->GetRootParent());

        if (pSeenActor && pSeenActor->GetTeam() != m_Team)
        {
            // Adjust aim in case seen target is moving
            m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();

            // If we have something to fire with
            if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldDevice())
            {
                // Don't press the trigger too fast in succession
                if (m_FireTimer.IsPastSimMS(250))
                {
                    // Get the distance to the target and see if we've aimed well enough
                    Vector targetVec = g_SceneMan.ShortestDistance(m_Pos, m_SeenTargetPos, false);
                    float threshold = (m_AimDistance * 6.0F) + (m_pFGArm->GetHeldDevice()->GetSharpLength() * m_SharpAimProgress);
                    // Fire if really close, or if we have aimed well enough
                    if (targetVec.MagnitudeIsLessThan(threshold))
                    {
                        // ENEMY AIMED AT well enough - FIRE!
                        m_DeviceState = FIRING;
                        m_SweepTimer.Reset();
                        m_FireTimer.Reset();
                    }
                    // Stop and aim more carefully
                    else
                        m_LateralMoveState = LAT_STILL;
                }
                // Stop and aim more carefully
                else
                    m_LateralMoveState = LAT_STILL;
            }
        }
        // If we can't see the guy after some time of aiming, then give up and keep scanning
        else if (m_FireTimer.IsPastSimMS(3000))
        {
            m_DeviceState = SCANNING;
        }

        // Make sure we're not detected as being stuck just because we're standing still
        m_StuckTimer.Reset();
    }
    // Firing at seen and aimed at target
    else if (m_DeviceState == FIRING)
    {
        // Keep aiming sharply!
        m_Controller.SetState(AIM_SHARP, true);

        // Pull the trigger repeatedly, so semi-auto weapons are fired properly
        if (!m_SweepTimer.IsPastSimMS(666))
        {
            // Pull the trigger!
            m_Controller.SetState(WEAPON_FIRE, true);
            if (FirearmIsSemiAuto())
                m_SweepTimer.Reset();
        }
        else
        {
            // Let go momentarily
            m_Controller.SetState(WEAPON_FIRE, false);
            m_SweepTimer.Reset();
        }

        // Adjust aim
        Vector targetVector = g_SceneMan.ShortestDistance(GetEyePos(), m_SeenTargetPos, false);
        m_Controller.m_AnalogAim = targetVector;
        m_Controller.m_AnalogAim.CapMagnitude(1.0);

        // Narrow focused FOV range scan
        pSeenMO = LookForMOs(8, g_MaterialGrass, false);
        // Still seeing enemy actor through the sights, keep firing!
        if (pSeenMO)
            pSeenActor = dynamic_cast<Actor *>(pSeenMO->GetRootParent());

        if (pSeenActor && pSeenActor->GetTeam() != m_Team)
        {
            // Adjust aim in case seen target is moving, and keep firing
            m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
            m_FireTimer.Reset();
        }

        // After burst of fire, if we don't still see the guy, then stop firing.
        if (m_FireTimer.IsPastSimMS(500) || FirearmIsEmpty())
        {
            m_DeviceState = SCANNING;
        }
        // Make sure we're not detected as being stuck just because we're standing still
        m_StuckTimer.Reset();
    }
    // Throwing at seen target
    else if (m_DeviceState == THROWING)
    {
        // Keep aiming sharply!
        m_Controller.SetState(AIM_SHARP, true);

        // Adjust aim constantly
        Vector targetVector = g_SceneMan.ShortestDistance(GetEyePos(), m_SeenTargetPos, false);
        // Adjust upward so we aim the throw higer than the target to compensate for gravity in throw trajectory
        targetVector.m_Y -= targetVector.GetMagnitude();
        m_Controller.m_AnalogAim = targetVector;
        m_Controller.m_AnalogAim.CapMagnitude(1.0);

        // Narrow focused FOV range scan
        pSeenMO = LookForMOs(18, g_MaterialGrass, false);
        // Still seeing enemy actor through the sights, keep aiming the throw!
        if (pSeenMO)
            pSeenActor = dynamic_cast<Actor *>(pSeenMO->GetRootParent());

        if (pSeenActor && pSeenActor->GetTeam() != m_Team)
        {
            // Adjust aim in case seen target is moving, and keep aiming thr throw
            m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetCPUPos();
        }

// TODO: make proper throw range calc based on the throwable's mass etc
        float range = m_CharHeight * 4;
        // Figure out how far away the target is based on max range
        float targetScalar = targetVector.GetMagnitude() / range;
        if (targetScalar < 0.01)
            targetScalar = 0.01;

        // Start making the throw, and charge up in proportion to how far away the target is
        if (!m_FireTimer.IsPastSimMS(m_ThrowPrepTime * targetScalar))
        {
            // HOld down the fire button so the throw is charged
            m_Controller.SetState(WEAPON_FIRE, true);
        }
        // Now release the throw and let fly!
        else
        {
            m_Controller.SetState(WEAPON_FIRE, false);
            m_DeviceState = SCANNING;
        }

        // Make sure we're not detected as being stuck just because we're standing still
        m_StuckTimer.Reset();
    }

    /////////////////////////////////////////////////
    // JUMPING LOGIC

    // Already in a jump
    if (m_ObstacleState == JUMPING)
    {
        // Override the lateral control for the precise jump
        // Turn around
        if (m_MoveVector.m_X > 0 && m_LateralMoveState == LAT_LEFT)
            m_LateralMoveState = LAT_RIGHT;
        else if (m_MoveVector.m_X < 0 && m_LateralMoveState == LAT_RIGHT)
            m_LateralMoveState = LAT_LEFT;

        if (m_JumpState == PREUPJUMP)
        {
            // Stand still for a little while to stabilize and look in the right dir, if we're directly under
            m_LateralMoveState = LAT_STILL;
            // Start the actual jump
            if (m_JumpTimer.IsPastSimMS(333))
            {
                // Here we go!
                m_JumpState = UPJUMP;
                m_JumpTimer.Reset();
                m_Controller.SetState(BODY_JUMPSTART, true);
            }
        }
        if (m_JumpState == UPJUMP)
        {
			Vector notUsed;

            // Burn the jetpack
            m_Controller.SetState(BODY_JUMP, true);

            // If we now can see the point we're going to, start adjusting our aim and jet nozzle forward
            if (!g_SceneMan.CastStrengthRay(cpuPos, m_JumpTarget - cpuPos, 5, notUsed, 4))
                m_PointingTarget = m_JumpTarget;

            // if we are a bit over the target, stop firing the jetpack and try to go forward and land
            if (m_Pos.m_Y < m_JumpTarget.m_Y)
            {
                m_DeviceState = POINTING;
                m_JumpState = APEXJUMP;
                m_JumpTimer.Reset();
            }
            // Abort the jump if we're not reaching the target height within reasonable time
            else if (m_JumpTimer.IsPastSimMS(5000) || m_StuckTimer.IsPastRealMS(500))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                if (m_DeviceState == POINTING)
                    m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }

		Vector notUsed;

        // Got the height, now wait until we crest the top and start falling again
        if (m_JumpState == APEXJUMP)
        {
			Vector notUsedInner;

            m_PointingTarget = m_JumpTarget;

            // We are falling again, and we can still see the target! start adjusting our aim and jet nozzle forward
            if (m_Vel.m_Y > 4.0 && !g_SceneMan.CastStrengthRay(cpuPos, m_JumpTarget - cpuPos, 5, notUsedInner, 3))
            {
                m_DeviceState = POINTING;
                m_JumpState = LANDJUMP;
                m_JumpTimer.Reset();
            }

            // Time abortion
            if (m_JumpTimer.IsPastSimMS(3500))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                if (m_DeviceState == POINTING)
                    m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
            // If we've fallen below the target again, then abort the jump
            else if (cpuPos.m_Y > m_JumpTarget.m_Y && g_SceneMan.CastStrengthRay(cpuPos, g_SceneMan.ShortestDistance(cpuPos, m_JumpTarget), 5, notUsedInner, 3))
            {
                // Set the move target back to the ledge, to undo any checked off points we may have seen while hovering oer teh edge
                m_MoveTarget = m_JumpTarget;
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                if (m_DeviceState == POINTING)
                    m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
        // We are high and falling again, now go forward to land on top of the ledge
        if (m_JumpState == LANDJUMP)
        {
			Vector notUsedInner;

            m_PointingTarget = m_JumpTarget;

            // Burn the jetpack for a short while to get forward momentum, but not too much
//            if (!m_JumpTimer.IsPastSimMS(500))
                m_Controller.SetState(BODY_JUMP, true);

            // If we've fallen below the target again, then abort the jump
            // If we're flying past the target too, end the jump
            // Lastly, if we're flying way over the target again, just cut the jets!
            if (m_JumpTimer.IsPastSimMS(3500) || (cpuPos.m_Y > m_JumpTarget.m_Y && g_SceneMan.CastStrengthRay(cpuPos, m_JumpTarget - cpuPos, 5, notUsedInner, 3)) ||
                (m_JumpingRight && m_Pos.m_X > m_JumpTarget.m_X) || (!m_JumpingRight && m_Pos.m_X < m_JumpTarget.m_X) || (cpuPos.m_Y < m_JumpTarget.m_Y - m_CharHeight))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                if (m_DeviceState == POINTING)
                    m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
        else if (m_JumpState == FORWARDJUMP)
        {
            // Burn the jetpack
            m_Controller.SetState(BODY_JUMP, true);

            // Stop firing the jetpack after a period or if we've flown past the target
            if (m_JumpTimer.IsPastSimMS(500) || (m_JumpingRight && m_Pos.m_X > m_JumpTarget.m_X) || (!m_JumpingRight && m_Pos.m_X < m_JumpTarget.m_X))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                if (m_DeviceState == POINTING)
                    m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
    }
    // Not in a jump yet, so check for conditions to trigger a jump
    // Also if the movetarget is szzero, probably first frame , but don't try to chase it
    // Don't start jumping if we are crawling
    else if (!m_MoveTarget.IsZero() && !m_Crawling)
    {
		Vector notUsed;

        // UPWARD JUMP TRIGGERINGS if it's a good time to jump up to a ledge
        if ((-m_MoveVector.m_Y > m_CharHeight * 0.75) && m_DeviceState != AIMING && m_DeviceState != FIRING)// && (fabs(m_MoveVector.m_X) < m_CharHeight))
        {
            // Is there room to jump straight up for as high as we want?
            // ALso, has teh jetpack been given a rest since last attempt?
            if (m_JumpTimer.IsPastSimMS(3500) && !g_SceneMan.CastStrengthRay(cpuPos, Vector(0, m_MoveTarget.m_Y - cpuPos.m_Y), 5, notUsed, 3))
            {
                // Yes, so let's start jump, aim at the target!
                m_ObstacleState = JUMPING;
                m_JumpState = PREUPJUMP;
                m_JumpTarget = m_MoveTarget;
                m_JumpingRight = g_SceneMan.ShortestDistance(m_Pos, m_JumpTarget).m_X > 0;
//                m_JumpState = UPJUMP;
//                m_Controller.SetState(BODY_JUMPSTART, true);
                m_JumpTimer.Reset();
                m_DeviceState = POINTING;
                // Aim straight up
                m_PointingTarget.SetXY(cpuPos.m_X, m_MoveTarget.m_Y);
            }
        }
        // FORWARD JUMP TRIGGERINGS if it's a good time to jump over a chasm; gotto be close to an edge
        else if (m_MovePath.size() > 2 && (fabs(m_PrevPathTarget.m_X - m_Pos.m_X) < (m_CharHeight * 0.25)))
        {
            std::list<Vector>::iterator pItr = m_MovePath.begin();
            std::list<Vector>::iterator prevItr = m_MovePath.begin();
            // Start by looking at the dip between last checked waypoint and the next
// TODO: not wrap safe!
            int dip = m_MoveTarget.GetFloorIntY() - m_PrevPathTarget.GetFloorIntY();
            // See if the next few path points dip steeply
            for (int i = 0; i < 3 && dip < m_CharHeight && pItr != m_MovePath.end(); ++i)
            {
                ++pItr;
                if (pItr == m_MovePath.end())
                    break;
                dip += (*pItr).GetFloorIntY() - (*prevItr).GetFloorIntY();
                ++prevItr;
                if (dip >= m_CharHeight)
                    break;
            }
            // The dip is deep enough to warrant looking for a rise after the dip
            if (dip >= m_CharHeight)
            {
                int rise = 0;
                for (int i = 0; i < 6 && pItr != m_MovePath.end(); ++i)
                {
                    ++pItr;
                    if (pItr == m_MovePath.end())
                        break;
                    rise -= (*pItr).m_Y - (*prevItr).m_Y;
                    ++prevItr;
                    if (rise >= m_CharHeight)
                        break;
                }

				Vector notUsedInner;

                // The rise is high enough to warrant looking across the trench for obstacles in the way of a jump
                if (rise >= m_CharHeight && !g_SceneMan.CastStrengthRay(cpuPos, Vector((*pItr).m_X - cpuPos.m_X, 0), 5, notUsedInner, 3))
                {
                    // JUMP!!!
                    m_Controller.SetState(BODY_JUMPSTART, true);
                    m_ObstacleState = JUMPING;
                    m_JumpState = FORWARDJUMP;
                    m_JumpTarget = *pItr;
                    m_JumpingRight = g_SceneMan.ShortestDistance(m_Pos, m_JumpTarget).m_X > 0;
                    m_JumpTimer.Reset();
                    m_DeviceState = POINTING;
                    m_PointingTarget = *pItr;
                    // Remove the waypoints we're about to jump over
                    std::list<Vector>::iterator pRemItr = m_MovePath.begin();
                    while (pRemItr != m_MovePath.end())
                    {
                        pRemItr++;
                        m_PrevPathTarget = m_MovePath.front();
                        m_MovePath.pop_front();
                        if (pRemItr == pItr)
                            break;
                    }
                    if (!m_MovePath.empty())
                        m_MoveTarget = m_MovePath.front();
                    else
                        m_MoveTarget = m_Pos;
                }
            }
        }
        // See if we can jump over a teammate who's stuck in the way
        else if (m_TeamBlockState != NOTBLOCKED && m_TeamBlockState != FOLLOWWAIT && !g_SceneMan.CastStrengthRay(cpuPos, Vector((m_HFlipped ? -m_CharHeight : m_CharHeight) * 1.5, -m_CharHeight * 1.5), 5, notUsed, 3))
        {
            // JUMP!!!
            m_Controller.SetState(BODY_JUMPSTART, true);
            m_ObstacleState = JUMPING;
            m_JumpState = FORWARDJUMP;
            m_JumpTarget = m_Pos + Vector((m_HFlipped ? -m_CharHeight : m_CharHeight) * 3, -m_CharHeight);
            m_JumpingRight = !m_HFlipped;
            m_JumpTimer.Reset();
            m_DeviceState = POINTING;
            m_PointingTarget = m_JumpTarget;
        }
    }

    ////////////////////////////////////////
    // If falling, use jetpack to land as softly as possible

    // If the height is more than the character's height, do something to soften the landing!
    float thrustLimit = m_CharHeight;

    // If we're already firing jetpack, then see if it's time to stop
    if (m_ObstacleState == SOFTLANDING && (m_Vel.m_Y < 4.0 || GetAltitude(thrustLimit, 5) < thrustLimit))
    {
        m_ObstacleState = PROCEEDING;
        if (m_DeviceState == POINTING)
            m_DeviceState = SCANNING;
    }
    // We're falling, so see if it's time to start firing the jetpack to soften the landing
    if (/*m_FallTimer.IsPastSimMS(300) && */m_Vel.m_Y > 8.0 && m_ObstacleState != SOFTLANDING && m_ObstacleState != JUMPING)
    {
        // Look if we have more than the height limit of air below the controlled
        bool withinLimit = GetAltitude(thrustLimit, 5) < thrustLimit;

        // If the height is more than the limit, do something!
        if (!withinLimit)
        {
            m_ObstacleState = SOFTLANDING;
            m_Controller.SetState(BODY_JUMPSTART, true);
        }
    }
//        else
//            m_FallTimer.Reset();

    ///////////////////////////////////////////
    // Obstacle resolution

    if (m_ObstacleState == PROCEEDING)
    {
        // If we're not caring about blocks for a while, then just see how long until we do again
        if (m_TeamBlockState == IGNORINGBLOCK)
        {
            // Ignored long enough, now we can be blocked again
            if (m_BlockTimer.IsPastSimMS(10000))
                m_TeamBlockState = NOTBLOCKED;
        }
        else
        {
            // Detect a TEAMMATE in the way and hold until he has moved
            Vector lookRay(m_CharHeight * 0.75, 0);
            Vector lookRayDown(m_CharHeight * 0.75, 0);
            lookRay.RadRotate(GetAimAngle(true));
            lookRayDown.RadRotate(GetAimAngle(true) + (m_HFlipped ? c_QuarterPI : -c_QuarterPI));
            MOID obstructionMOID = g_SceneMan.CastMORay(GetCPUPos(), lookRay, m_MOID, IgnoresWhichTeam(), g_MaterialGrass, false, 6);
            obstructionMOID = obstructionMOID == g_NoMOID ? g_SceneMan.CastMORay(cpuPos, lookRayDown, m_MOID, IgnoresWhichTeam(), g_MaterialGrass, false, 6) : obstructionMOID;
            if (obstructionMOID != g_NoMOID)
            {
                // Take a look at the actorness and team of the thing that holds whatever we saw
                obstructionMOID = g_MovableMan.GetRootMOID(obstructionMOID);
                Actor *pActor = dynamic_cast<Actor *>(g_MovableMan.GetMOFromID(obstructionMOID));
                // Oops, a mobile team member is in the way, don't do anything until he moves out of the way!
                if (pActor && pActor != this && pActor->GetTeam() == m_Team && pActor->IsControllable())
                {
                    // If this is the guy we're actually supposed to be following, then indicate that so we jsut wait patiently for him to move
                    if (pActor == m_pMOMoveTarget)
                        m_TeamBlockState = FOLLOWWAIT;
                    else
                    {
                        // If already blocked, see if it's long enough to give up and start to ignore the blockage
                        if (m_TeamBlockState == BLOCKED)
                        {
                            if (m_BlockTimer.IsPastSimMS(10000))
                            {
                                m_TeamBlockState = IGNORINGBLOCK;
                                m_BlockTimer.Reset();
                            }
                        }
                        // Not blocked yet, but will be now, so set it
                        else
                        {
                            m_TeamBlockState = BLOCKED;
                            m_BlockTimer.Reset();
                        }
                    }
                }
                else if (m_BlockTimer.IsPastSimMS(1000))
                    m_TeamBlockState = NOTBLOCKED;
            }
            else if (m_BlockTimer.IsPastSimMS(1000))
                m_TeamBlockState = NOTBLOCKED;
        }

        // Detect MATERIAL blocking the path and start digging through it
        Vector pathSegRay(g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget));
        Vector obstaclePos;
        if (m_TeamBlockState != BLOCKED && m_DeviceState == SCANNING && g_SceneMan.CastStrengthRay(m_PrevPathTarget, pathSegRay, 5, obstaclePos, 1, g_MaterialDoor))
        {
            // Only if we actually have a digging tool!
            if (EquipDiggingTool(false))
            {
                if (m_DigState == NOTDIGGING)
                {
                    // First update the path to make sure a fresh path would still be blocked
                    UpdateMovePath();
                    m_DigState = PREDIG;
                }
    // TODO: base the range on the digger's actual range, quereied from teh digger itself
                // Updated the path, and it's still blocked, so check that we're close enough to START digging
                else if (m_DigState == PREDIG && (fabs(m_PrevPathTarget.m_X - m_Pos.m_X) < (m_CharHeight * 0.5)))
                {
                    m_DeviceState = DIGGING;
                    m_DigState = STARTDIG;
                    m_SweepRange = c_QuarterPI - c_SixteenthPI;
                    m_ObstacleState = DIGPAUSING;
                }
                // If in invalid state of starting to dig but not actually digging, reset
                else if (m_DigState == STARTDIG && m_DeviceState != DIGGING)
                {
                    m_DigState = NOTDIGGING;
                    m_ObstacleState = PROCEEDING;
                }
            }
        }
        else
        {
            m_DigState = NOTDIGGING;
            m_ObstacleState = PROCEEDING;
        }

        // If our path isn't blocked enough to dig, but the headroom is too little, start crawling to get through!
        if (m_DeviceState != DIGGING && m_DigState != PREDIG)
        {
            Vector heading(g_SceneMan.ShortestDistance(m_Pos, m_PrevPathTarget));
            heading.SetMagnitude(m_CharHeight * 0.5);
            // Don't crawl if it's too steep, just let him climb then instead
            if (fabs(heading.m_X) > fabs(heading.m_Y) && m_pHead && m_pHead->IsAttached())
            {
                Vector topHeadPos = m_Pos;
                // Stack up the maximum height the top back of the head can have over the body's position
                topHeadPos.m_X += m_HFlipped ? m_pHead->GetRadius() : -m_pHead->GetRadius();
                topHeadPos.m_Y += m_pHead->GetParentOffset().m_Y - m_pHead->GetJointOffset().m_Y + m_pHead->GetSpriteOffset().m_Y - 3;
                // First check up to the top of the head, and then from there forward
                if (g_SceneMan.CastStrengthRay(m_Pos, topHeadPos - m_Pos, 5, obstaclePos, 4, g_MaterialDoor) ||
                    g_SceneMan.CastStrengthRay(topHeadPos, heading, 5, obstaclePos, 4, g_MaterialDoor))
                {
                    m_Controller.SetState(BODY_CROUCH, true);
                    m_Crawling = true;
                }
                else
                    m_Crawling = false;
            }
            else
                m_Crawling = false;
        }
        else
            m_Crawling = false;
    }
    // We're not proceeding
    else
    {
        // Can't be obstructed if we're not going forward
        m_TeamBlockState = NOTBLOCKED;
        // Disable invalid digging mode
        if ((m_DigState == STARTDIG && m_DeviceState != DIGGING) || (m_ObstacleState == DIGPAUSING && m_DeviceState != DIGGING))
        {
            m_DigState = NOTDIGGING;
            m_ObstacleState = PROCEEDING;
        }
    }

    /////////////////////////////////////
    // Detect and react to being stuck

    if (m_ObstacleState == PROCEEDING)
    {
        // Reset stuck timer if we're moving fine, or we're waiting for teammate to move
        if (m_RecentMovement.MagnitudeIsGreaterThan(2.5F) || m_TeamBlockState)
            m_StuckTimer.Reset();

        if (m_DeviceState == SCANNING)
        {
            // Ok we're actually stuck, so backtrack
            if (m_StuckTimer.IsPastSimMS(1500))
            {
                m_ObstacleState = BACKSTEPPING;
                m_StuckTimer.Reset();
// TEMP hack to pick up weapon, could be stuck on one
                m_Controller.SetState(WEAPON_PICKUP, true);
            }
        }
		else if (m_DeviceState == DIGGING && m_StuckTimer.IsPastSimMS(5000))
		{
			// Ok we're actually stuck, so backtrack.
			m_ObstacleState = BACKSTEPPING;
			m_StuckTimer.Reset();
		}
    }
    if (m_ObstacleState == JUMPING)
    {
        // Reset stuck timer if we're moving fine
        if (m_RecentMovement.MagnitudeIsGreaterThan(2.5F))
            m_StuckTimer.Reset();

        if (m_StuckTimer.IsPastSimMS(250))
        {
            m_JumpState = NOTJUMPING;
            m_ObstacleState = PROCEEDING;
            if (m_DeviceState == POINTING)
                m_DeviceState = SCANNING;
        }
    }
    else if (m_ObstacleState == DIGPAUSING)
    {
        // If we've beeen standing still digging in teh same spot for along time, then backstep to get unstuck
        if (m_DeviceState == DIGGING)
        {
            if (m_StuckTimer.IsPastSimMS(5000))
            {
                m_ObstacleState = BACKSTEPPING;
                m_StuckTimer.Reset();
            }
        }
        else
        {
            m_StuckTimer.Reset();
        }
    }
    // Reset from backstepping
// TODO: better movement detection
    else if (m_ObstacleState == BACKSTEPPING && (m_StuckTimer.IsPastSimMS(2000) || m_RecentMovement.MagnitudeIsGreaterThan(15.0F)))
    {
        m_ObstacleState = PROCEEDING;
        m_StuckTimer.Reset();
    }

    ////////////////////////////////////
    // Set the movement commands now according to what we've decided to do

    // Don't move if there's a teammate in the way (but we can flip)
    if (m_LateralMoveState != LAT_STILL && ((m_TeamBlockState != BLOCKED && m_TeamBlockState != FOLLOWWAIT) || (!m_HFlipped && m_LateralMoveState == LAT_LEFT) || (m_HFlipped && m_LateralMoveState == LAT_RIGHT)))
    {
        if (m_ObstacleState == SOFTLANDING)
        {
            m_Controller.SetState(BODY_JUMP, true);
            // Direct the jetpack blast
            m_Controller.m_AnalogMove = -m_Vel;
            m_Controller.m_AnalogMove.Normalize();
        }
        else if (m_ObstacleState == JUMPING)
        {
            if (m_LateralMoveState == LAT_LEFT)
                m_Controller.SetState(MOVE_LEFT, true);
            else if (m_LateralMoveState == LAT_RIGHT)
                m_Controller.SetState(MOVE_RIGHT, true);
        }
        else if (m_ObstacleState == DIGPAUSING)
        {
            // Only flip if we're commanded to, don't move though, and DON'T FIRE IN THE OPPOSITE DIRECTION
            if (m_LateralMoveState == LAT_LEFT && !m_HFlipped)
            {
                m_Controller.SetState(MOVE_LEFT, true);
                m_Controller.SetState(WEAPON_FIRE, false);
            }
            else if (m_LateralMoveState == LAT_RIGHT && m_HFlipped)
            {
                m_Controller.SetState(MOVE_RIGHT, true);
                m_Controller.SetState(WEAPON_FIRE, false);
            }
        }
        else if (m_ObstacleState == PROCEEDING)
        {
            if (m_LateralMoveState == LAT_LEFT)
                m_Controller.SetState(MOVE_LEFT, true);
            else if (m_LateralMoveState == LAT_RIGHT)
                m_Controller.SetState(MOVE_RIGHT, true);
        }
        else if (m_ObstacleState == BACKSTEPPING)
        {
            if (m_LateralMoveState == LAT_LEFT)
                m_Controller.SetState(MOVE_RIGHT, true);
            else if (m_LateralMoveState == LAT_RIGHT)
                m_Controller.SetState(MOVE_LEFT, true);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void AHuman::PreControllerUpdate()
{
	float deltaTime = g_TimerMan.GetDeltaTimeSecs();
	float rot = m_Rotation.GetRadAngle();

	Vector analogAim = m_Controller.GetAnalogAim();
    const float analogDeadzone = 0.1F;

	m_Paths[FGROUND][m_MoveState].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][m_MoveState].SetHFlip(m_HFlipped);

	////////////////////////////////////
	// Jetpack activation and blast direction

	if (m_pJetpack) {
		if (m_JetTimeTotal > 0) {
			// Jetpack throttle depletes relative to jet time, but only if throttle range values have been defined
			float jetTimeRatio = std::max(m_JetTimeLeft / m_JetTimeTotal, 0.0F);
			m_pJetpack->SetThrottle(jetTimeRatio * 2.0F - 1.0F);
		}
		if (m_Controller.IsState(BODY_JUMPSTART) && m_JetTimeLeft > 0 && m_Status != INACTIVE) {
			m_pJetpack->TriggerBurst();
			m_ForceDeepCheck = true;
			m_pJetpack->EnableEmission(true);
			m_JetTimeLeft = std::max(m_JetTimeLeft - g_TimerMan.GetDeltaTimeMS() * static_cast<float>(std::max(m_pJetpack->GetTotalBurstSize(), 2)) * (m_pJetpack->CanTriggerBurst() ? 1.0F : 0.5F), 0.0F);
		} else if (m_Controller.IsState(BODY_JUMP) && m_JetTimeLeft > 0 && m_Status != INACTIVE) {
			m_pJetpack->EnableEmission(true);
			m_pJetpack->AlarmOnEmit(m_Team);
			m_JetTimeLeft = std::max(m_JetTimeLeft - g_TimerMan.GetDeltaTimeMS(), 0.0F);
			m_MoveState = JUMP;
			m_Paths[FGROUND][JUMP].Restart();
			m_Paths[BGROUND][JUMP].Restart();
		} else {
			m_pJetpack->EnableEmission(false);
			if (m_MoveState == JUMP) { m_MoveState = STAND; }
			m_JetTimeLeft = std::min(m_JetTimeLeft + g_TimerMan.GetDeltaTimeMS() * m_JetReplenishRate, m_JetTimeTotal);
		}

		float maxAngle = c_HalfPI * m_JetAngleRange;
		// If pie menu is on, keep the angle to what it was before.
		if (!m_Controller.IsState(PIE_MENU_ACTIVE)) {
			// Direct the jetpack nozzle according to either analog stick input or aim angle.
			if (m_Controller.GetAnalogMove().MagnitudeIsGreaterThan(analogDeadzone)) {
				float jetAngle = std::clamp(m_Controller.GetAnalogMove().GetAbsRadAngle() - c_HalfPI, -maxAngle, maxAngle);
				m_pJetpack->SetEmitAngle(FacingAngle(jetAngle - c_HalfPI));
			} else {
				// Thrust in the opposite direction when strafing.
				float flip = ((m_HFlipped && m_Controller.IsState(MOVE_RIGHT)) || (!m_HFlipped && m_Controller.IsState(MOVE_LEFT))) ? -1.0F : 1.0F;
				// Halve the jet angle when looking downwards so the actor isn't forced to go sideways
                // TODO: don't hardcode this ratio?
				float jetAngle = (m_AimAngle > 0 ? m_AimAngle * m_JetAngleRange : -m_AimAngle * m_JetAngleRange * 0.5F) - maxAngle;
				// FacingAngle isn't needed because it's already been applied to AimAngle since last update.
				m_pJetpack->SetEmitAngle(jetAngle * flip - c_HalfPI);
			}
		}
	}

	////////////////////////////////////
	// Movement direction

    const float movementThreshold = 1.0F;
	bool isStill = (m_Vel + m_PrevVel).MagnitudeIsLessThan(movementThreshold);
	bool isSharpAiming = m_Controller.IsState(AIM_SHARP);

    // If the pie menu is on, try to preserve whatever move state we had before it going into effect.
    // This is only done for digital input, where the user needs to use the keyboard to choose pie slices.
    // For analog input, this doesn't matter - the mouse or aiming analog stick controls the pie menu.
    bool keepOldState = m_Controller.IsKeyboardOnlyControlled() && m_Controller.IsState(PIE_MENU_ACTIVE);

	if (!keepOldState) {
        bool crouching = m_Controller.IsState(BODY_CROUCH);
        if ((m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT) || m_MoveState == JUMP) && m_Status != INACTIVE) {
            for (int i = WALK; i < MOVEMENTSTATECOUNT; ++i) {
                m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
                m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
            }
            // Only if not jumping, OR if jumping, and apparently stuck on something - then help out with the limbs.
            if (m_MoveState != JUMP || isStill) {
                // Restart the stride if we're just starting to walk or crawl.
                if ((m_MoveState != WALK && !crouching) || (m_MoveState != CRAWL && crouching)) {
                    m_StrideStart = true;
                    MoveOutOfTerrain(g_MaterialGrass);
                }

                m_MoveState = crouching ? CRAWL : WALK;

                // Engage prone state, this makes the body's rotational spring pull it horizontal instead of upright.
                if (m_MoveState == CRAWL && m_ProneState == NOTPRONE) {
                    m_ProneState = GOPRONE;
                    m_ProneTimer.Reset();
                }

                m_Paths[FGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
                m_Paths[BGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
            }

			// Walk backwards if the aiming is already focused in the opposite direction of travel.
			// Note that we check against zero here rather than the deadzone, because using the deadzone makes jetpacking mouse players unable to fly one way and aim the other.
            if (!analogAim.IsZero() || isSharpAiming) {
                m_Paths[FGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
                m_Paths[BGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
            } else if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped)) {
                m_HFlipped = !m_HFlipped;
                m_CheckTerrIntersection = true;
                if (m_ProneState == NOTPRONE) { MoveOutOfTerrain(g_MaterialGrass); }

                for (int i = WALK; i < MOVEMENTSTATECOUNT; ++i) {
                    m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
                    m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
                    m_Paths[FGROUND][i].Terminate();
                    m_Paths[BGROUND][i].Terminate();
                }
                m_StrideStart = true;
                // Stop the going prone spring.
                if (m_ProneState == GOPRONE) { m_ProneState = PRONE; }
            }
        } else {
            m_ArmClimbing[FGROUND] = false;
            m_ArmClimbing[BGROUND] = false;
            if (crouching) {
                // Don't go back to crouching if we're already prone, the player has to let go of the crouch button first. If already laying down, just stay put.
                m_MoveState = m_ProneState == NOTPRONE ? CROUCH : NOMOVE;
            } else {
                m_MoveState = STAND;
            }
        }
        // Disengage the prone state as soon as crouch is released.
        if (!crouching && m_ProneState != NOTPRONE) { 
			EquipShieldInBGArm();
			m_ProneState = NOTPRONE; 
		}
    }

	////////////////////////////////////
	// Standard Reloading

	for (const Arm *arm : { m_pFGArm, m_pBGArm }) {
		if (arm) {
			if (HDFirearm *heldFirearm = dynamic_cast<HDFirearm *>(arm->GetHeldDevice())) {
				Arm *otherArm = arm == m_pFGArm ? m_pBGArm : m_pFGArm;
				bool otherArmIsAvailable = otherArm && !otherArm->GetHeldDevice();
				if (otherArmIsAvailable && heldFirearm->DoneReloading()) { otherArm->SetHandPos(heldFirearm->GetMagazinePos()); };
				heldFirearm->SetSupportAvailable(otherArmIsAvailable);
			}
		}
	}
	if (m_Controller.IsState(ControlState::WEAPON_RELOAD)) {
		ReloadFirearms();
	}
	if (m_WaitingToReloadOffhand) {
		if (HeldDevice *equippedItem = GetEquippedItem(); equippedItem && !equippedItem->IsReloading()) {
			ReloadFirearms();
			m_WaitingToReloadOffhand = false;
		}
	}

    ////////////////////////////////////
    // Change held MovableObjects

	if (m_pFGArm && m_Status != INACTIVE) {
		bool changeNext = m_Controller.IsState(WEAPON_CHANGE_NEXT);
		bool changePrev = m_Controller.IsState(WEAPON_CHANGE_PREV);
		if (changeNext || changePrev) {
			if (changeNext && changePrev) {
				UnequipArms();
			} else if (!m_Inventory.empty() || UnequipBGArm()) {
				if (HDFirearm *firearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice())) {
					firearm->StopActivationSound();
				}
				if (changeNext) {
					m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice *>(SwapNextInventory(m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice()))));
				} else {
					m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice *>(SwapPrevInventory(m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice()))));
				}
				EquipShieldInBGArm();
				m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
			m_EquipHUDTimer.Reset();
			m_SharpAimProgress = 0;
			// Reload empty firearms when we swap to them, for convenience.
			ReloadFirearms(true);
		}
	}

    ////////////////////////////////////
    // Aiming

	if (m_Controller.IsState(AIM_UP) && m_Status != INACTIVE) {
        // Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMUP) { m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300); }
		m_AimState = AIMUP;
		m_AimAngle += isSharpAiming ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();
		if (m_AimAngle > m_AimRange) { m_AimAngle = m_AimRange; }

	} else if (m_Controller.IsState(AIM_DOWN) && m_Status != INACTIVE) {
        // Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMDOWN) {m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300); }
		m_AimState = AIMDOWN;
		m_AimAngle -= isSharpAiming ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();
		if (m_AimAngle < -m_AimRange) { m_AimAngle = -m_AimRange; }

	} else if (analogAim.MagnitudeIsGreaterThan(analogDeadzone) && m_Status != INACTIVE) {
		// Hack to avoid the GetAbsRadAngle from mangling an aim angle straight down.
		if (analogAim.m_X == 0) { analogAim.m_X += 0.01F * GetFlipFactor(); }
		m_AimAngle = analogAim.GetAbsRadAngle();

		if ((analogAim.m_X > 0 && m_HFlipped) || (analogAim.m_X < 0 && !m_HFlipped)) {
			m_HFlipped = !m_HFlipped;
			m_CheckTerrIntersection = true;
			if (m_ProneState == NOTPRONE) { MoveOutOfTerrain(g_MaterialGrass); }
			for (int i = STAND; i < CLIMB; ++i) {
				m_Paths[FGROUND][i].SetHFlip(m_HFlipped);
				m_Paths[BGROUND][i].SetHFlip(m_HFlipped);
				m_Paths[FGROUND][i].Terminate();
				m_Paths[BGROUND][i].Terminate();
			}
			m_StrideStart = true;
			// Stop the going prone spring.
			if (m_ProneState == GOPRONE) { m_ProneState = PRONE; }
		}
		// Correct angle based on flip.
		m_AimAngle = FacingAngle(m_AimAngle);
		// Clamp so it's within the range.
		Clamp(m_AimAngle, m_AimRange, -m_AimRange);
	} else {
		m_AimState = AIMSTILL;
	}
	float adjustedAimAngle = m_AimAngle * GetFlipFactor();

    //////////////////////////////
    // Sharp aim calculation

// TODO: make the delay data driven by both the actor and the device!
    //
	if (isSharpAiming && m_Status == STABLE && (m_MoveState == STAND || m_MoveState == CROUCH || m_MoveState == NOMOVE || m_MoveState == WALK) && m_Vel.MagnitudeIsLessThan(5.0F) && GetEquippedItem()) {
        float aimMag = analogAim.GetMagnitude();

		// If aim sharp is being done digitally, then translate to full analog aim mag
		if (aimMag < 0.1F) { aimMag = 1.0F; }
		if (m_MoveState == WALK) { aimMag *= 0.3F; }

		if (m_SharpAimTimer.IsPastSimMS(m_SharpAimDelay)) {
			// Only go slower outward
			if (m_SharpAimProgress < aimMag) {
				m_SharpAimProgress += (aimMag - m_SharpAimProgress) * 0.035F;
			} else {
				m_SharpAimProgress = aimMag;
			}
			m_SharpAimRevertTimer.Reset();
		} else {
			m_SharpAimProgress *= 0.95F;
			m_SharpAimRevertTimer.SetElapsedSimTimeMS(m_SharpAimDelay - m_SharpAimTimer.GetElapsedSimTimeMS());
		}
	} else {
		m_SharpAimProgress = std::max(m_SharpAimProgress * 0.95F - 0.1F, 0.0F);
		if (m_SharpAimRevertTimer.IsPastSimMS(m_SharpAimDelay)) {
			m_SharpAimTimer.Reset();
		} else {
			m_SharpAimTimer.SetElapsedSimTimeMS(m_SharpAimDelay - m_SharpAimRevertTimer.GetElapsedSimTimeMS());
		}
	}

    ////////////////////////////////////
    // Handle firing/activating/throwing HeldDevices and ThrownDevices.
	// Also deal with certain reload cases and setting sharp aim progress for HeldDevices.

	ThrownDevice *thrownDevice = nullptr;
	if (HeldDevice *device = GetEquippedItem(); device && m_Status != INACTIVE) {
		if (!dynamic_cast<ThrownDevice *>(device)) {
			device->SetSharpAim(m_SharpAimProgress);

			if (HDFirearm *deviceAsFirearm = dynamic_cast<HDFirearm*>(device)) {
				if (m_Controller.IsState(WEAPON_FIRE)) {
					if (!m_CanActivateBGItem) {
						if (deviceAsFirearm->IsFullAuto()) {
							deviceAsFirearm->Activate();
							m_CanActivateBGItem = deviceAsFirearm->FiredOnce() && deviceAsFirearm->HalfwayToNextRound();
						} else if (!m_TriggerPulled) {
							deviceAsFirearm->Activate();
							if (deviceAsFirearm->FiredOnce()) {
								m_CanActivateBGItem = true;
								m_TriggerPulled = true;
							} else {
								m_CanActivateBGItem = !deviceAsFirearm->CanFire();
							}
						}
					}
				} else {
					deviceAsFirearm->Deactivate();
					m_TriggerPulled = false;
				}
			} else {
				m_CanActivateBGItem = true;
				if (m_Controller.IsState(WEAPON_FIRE)) {
					device->Activate();
					if (device->IsEmpty()) {
						ReloadFirearms(true);
					}
				} else {
					device->Deactivate();
				}
			}
			// If reloading 2 guns one-at-a-time, the automatic reload when firing empty won't trigger, so this makes sure it happens automatically.
			if (device->IsEmpty()) {
				ReloadFirearms(true);
			}

			if (device->IsReloading()) {
				m_CanActivateBGItem = true;
				m_SharpAimTimer.Reset();
				m_SharpAimProgress = 0;
				device->SetSharpAim(m_SharpAimProgress);
			}
		} else {
			m_CanActivateBGItem = true;
			if (thrownDevice = dynamic_cast<ThrownDevice *>(device)) {
				thrownDevice->SetSharpAim(isSharpAiming ? 1.0F : 0);
				if (m_Controller.IsState(WEAPON_FIRE)) {
					if (m_ArmsState != THROWING_PREP) {
						m_ThrowTmr.Reset();
						if (!thrownDevice->ActivatesWhenReleased()) { thrownDevice->Activate(); }
					}
					float throwProgress = GetThrowProgress();
					m_ArmsState = THROWING_PREP;
                    m_pFGArm->SetHandPos(m_pFGArm->GetJointPos() + (thrownDevice->GetStartThrowOffset().GetXFlipped(m_HFlipped) * throwProgress + thrownDevice->GetStanceOffset() * (1.0F - throwProgress)).RadRotate(adjustedAimAngle));
				} else if (m_ArmsState == THROWING_PREP) {
					m_ArmsState = THROWING_RELEASE;
                    m_pFGArm->SetHandPos(m_pFGArm->GetJointPos() + thrownDevice->GetEndThrowOffset().RadRotate(adjustedAimAngle).GetXFlipped(m_HFlipped));

					float maxThrowVel = thrownDevice->GetCalculatedMaxThrowVelIncludingArmThrowStrength();
					if (MovableObject *pMO = m_pFGArm->RemoveAttachable(thrownDevice)) {
						pMO->SetPos(m_pFGArm->GetJointPos() + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
						float minThrowVel = thrownDevice->GetMinThrowVel();
						if (minThrowVel == 0) { minThrowVel = maxThrowVel * 0.2F; }

						Vector tossVec(minThrowVel + (maxThrowVel - minThrowVel) * GetThrowProgress(), 0.5F * RandomNormalNum());
						pMO->SetVel(m_Vel * 0.5F + tossVec.RadRotate(m_AimAngle).GetXFlipped(m_HFlipped));
						pMO->SetAngularVel(m_AngularVel + RandomNum(-5.0F, 2.5F) * GetFlipFactor());
						pMO->SetRotAngle(adjustedAimAngle);

						if (HeldDevice *moAsHeldDevice = dynamic_cast<HeldDevice *>(pMO)) {
							moAsHeldDevice->SetTeam(m_Team);
							moAsHeldDevice->SetIgnoresTeamHits(true);
							g_MovableMan.AddItem(moAsHeldDevice);
						}
						pMO = 0;
					}
					if (thrownDevice->ActivatesWhenReleased()) { thrownDevice->Activate(); }
					m_ThrowTmr.Reset();
				}
			} else if (m_ArmsState == THROWING_RELEASE && m_ThrowTmr.GetElapsedSimTimeMS() > 100) {
				m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice *>(SwapNextInventory()));
				m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
				EquipShieldInBGArm();
				m_ArmsState = WEAPON_READY;
			} else if (m_ArmsState == THROWING_RELEASE) {
				m_pFGArm->AddHandTarget("Adjusted Aim Angle", m_Pos + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
			}
		}
	} else if (m_ArmsState == THROWING_RELEASE && m_ThrowTmr.GetElapsedSimTimeMS() > 100) {
		if (m_pFGArm) {
			m_pFGArm->SetHeldDevice(dynamic_cast<HeldDevice *>(SwapNextInventory()));
			m_pFGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
		}
		EquipShieldInBGArm();
		m_ArmsState = WEAPON_READY;
	} else if (m_ArmsState == THROWING_RELEASE && m_pFGArm) {
		m_pFGArm->AddHandTarget("Adjusted Aim Angle", m_Pos + Vector(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F).RadRotate(adjustedAimAngle));
	} else {
		m_CanActivateBGItem = true;
	}

	if (HeldDevice *device = GetEquippedBGItem(); device && m_Status != INACTIVE) {
		if (HDFirearm *deviceAsFirearm = dynamic_cast<HDFirearm*>(device)) {
			if (m_Controller.IsState(WEAPON_FIRE)) {
				if (m_CanActivateBGItem && (!m_TriggerPulled || (deviceAsFirearm->IsFullAuto() && deviceAsFirearm->HalfwayToNextRound()))) {
					deviceAsFirearm->Activate();
					if (deviceAsFirearm->FiredOnce()) {
						m_CanActivateBGItem = false;
						m_TriggerPulled = true;
					} else {
						m_CanActivateBGItem = deviceAsFirearm->CanFire();
					}
				}
			} else {
				deviceAsFirearm->Deactivate();
				m_TriggerPulled = false;
			}
		} else {
			m_CanActivateBGItem = false;
			if (m_Controller.IsState(WEAPON_FIRE)) {
				device->Activate();
			} else {
				device->Deactivate();
			}
		}
		// If reloading 2 guns one-at-a-time, the automatic reload when firing empty won't trigger, so this makes sure it happens automatically.
		if (device->IsEmpty()) {
			ReloadFirearms(true);
		}
		device->SetSharpAim(m_SharpAimProgress);

		if (device->IsReloading()) {
			m_CanActivateBGItem = false;
			m_SharpAimTimer.Reset();
			m_SharpAimProgress = 0;
			device->SetSharpAim(m_SharpAimProgress);
		}
	} else {
		m_CanActivateBGItem = false;
	}

	if (m_ArmsState == THROWING_PREP && !thrownDevice) {
		m_ArmsState = WEAPON_READY;
	}

    // Controller disabled
    if (m_Controller.IsDisabled())
    {
        m_MoveState = STAND;
        if (m_pJetpack && m_pJetpack->IsAttached())
            m_pJetpack->EnableEmission(false);
    }

//    m_aSprite->SetAngle((m_AimAngle / 180) * 3.141592654);
//    m_aSprite->SetScale(2.0);

    ////////////////////////////////////////
    // Item dropping logic

	if (m_Controller.IsState(WEAPON_DROP) && m_Status != INACTIVE) {
		Arm *dropperArm = nullptr;
		for (Arm *arm : { m_pFGArm, m_pBGArm }) {
			if (arm && arm->GetHeldDevice()) {
				HeldDevice *heldDevice = arm->GetHeldDevice();
				arm->RemoveAttachable(heldDevice, true, false);
				if (dropperArm) {
					if (heldDevice) {
						dropperArm->SetHeldDevice(heldDevice);
						arm->SetHandPos(dropperArm->GetPos());
					}
				} else {
					heldDevice->SetPos(arm->GetJointPos() + Vector(arm->GetMaxLength() * GetFlipFactor(), 0).RadRotate(adjustedAimAngle));
					Vector tossVec(1.0F + std::sqrt(std::abs(arm->GetThrowStrength()) / std::sqrt(std::abs(heldDevice->GetMass()) + 1.0F)), RandomNormalNum());
					heldDevice->SetVel(heldDevice->GetVel() * 0.5F + tossVec.RadRotate(m_AimAngle).GetXFlipped(m_HFlipped));
					heldDevice->SetAngularVel(heldDevice->GetAngularVel() + m_AngularVel * 0.5F + 3.0F * RandomNormalNum());

					arm->SetHandPos(heldDevice->GetPos());
				}
				dropperArm = arm;
			} else if (dropperArm && !m_Inventory.empty()) {
				dropperArm->SetHeldDevice(dynamic_cast<HeldDevice*>(SwapNextInventory()));
				dropperArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
		}
		if (!dropperArm && !m_Inventory.empty() && !m_pFGArm) {
			DropAllInventory();
			if (m_pBGArm) {
				m_pBGArm->SetHandPos(m_Pos + RotateOffset(m_HolsterOffset));
			}
		}
		EquipShieldInBGArm();
		m_SharpAimProgress = 0;
		m_EquipHUDTimer.Reset();
	}

    ////////////////////////////////////////
    // Item pickup logic

	float reach = m_SpriteRadius;
	Vector reachPoint = m_Pos;

	// Try to detect a new item
	if ((m_pFGArm || m_pBGArm) && m_Status == STABLE) {
		reach += m_pFGArm ? m_pFGArm->GetMaxLength() : m_pBGArm->GetMaxLength();
		reachPoint = m_pFGArm ? m_pFGArm->GetJointPos() : m_pBGArm->GetJointPos();

		MOID itemMOID = g_SceneMan.CastMORay(reachPoint, Vector(reach * RandomNum(0.5F, 1.0F) * GetFlipFactor(), 0).RadRotate(m_pItemInReach ? adjustedAimAngle : RandomNum(-(c_HalfPI + c_EighthPI), m_AimAngle * 0.75F + c_EighthPI) * GetFlipFactor()), m_MOID, Activity::NoTeam, g_MaterialGrass, true, 3);

		if (MovableObject *foundMO = g_MovableMan.GetMOFromID(itemMOID)) {
			if (HeldDevice *foundDevice = dynamic_cast<HeldDevice *>(foundMO->GetRootParent())) {
				m_pItemInReach = (m_pFGArm || foundDevice->IsOneHanded()) ? foundDevice : nullptr;
			}
		}
	}

	// Item currently set to be within reach has expired or is now out of range
	if (m_pItemInReach && (!m_pItemInReach->IsPickupableBy(this) || !g_MovableMan.IsDevice(m_pItemInReach) || g_SceneMan.ShortestDistance(reachPoint, m_pItemInReach->GetPos(), g_SceneMan.SceneWrapsX()).MagnitudeIsGreaterThan(reach + m_pItemInReach->GetRadius()))) {
		m_pItemInReach = nullptr;
	}

	if (m_pItemInReach && (m_pFGArm || m_pBGArm) && m_Controller.IsState(WEAPON_PICKUP) && m_Status != INACTIVE && g_MovableMan.RemoveMO(m_pItemInReach)) {
		Arm *armToUse = m_pFGArm ? m_pFGArm : m_pBGArm;
        Attachable *pMO = armToUse->RemoveAttachable(armToUse->GetHeldDevice());
		AddToInventoryBack(pMO);
		armToUse->SetHandPos(m_pItemInReach->GetJointPos());
		armToUse->SetHeldDevice(m_pItemInReach);
		m_pItemInReach = nullptr;

		if (armToUse != m_pBGArm) {
			EquipShieldInBGArm();
		}
		m_SharpAimProgress = 0;
		if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }

		m_EquipHUDTimer.Reset();
    }

    ///////////////////////////////////////////////////
    // Travel the limb AtomGroup:s

	if (m_Status == STABLE && !m_LimbPushForcesAndCollisionsDisabled && m_MoveState != NOMOVE)
    {
        // This exists to support disabling foot collisions if the limbpath has that flag set.
        if ((m_pFGFootGroup->GetAtomCount() == 0 && m_BackupFGFootGroup->GetAtomCount() > 0) != m_Paths[FGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupFGFootGroup->SetLimbPos(m_pFGFootGroup->GetLimbPos());
            std::swap(m_pFGFootGroup, m_BackupFGFootGroup);
        }
        if ((m_pBGFootGroup->GetAtomCount() == 0 && m_BackupBGFootGroup->GetAtomCount() > 0) != m_Paths[BGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupBGFootGroup->SetLimbPos(m_pBGFootGroup->GetLimbPos());
            std::swap(m_pBGFootGroup, m_BackupBGFootGroup);
        }

        // WALKING, OR WE ARE JETPACKING AND STUCK
        if (m_MoveState == WALK || (m_MoveState == JUMP && isStill)) {
            m_Paths[FGROUND][STAND].Terminate();
            m_Paths[BGROUND][STAND].Terminate();

//            float FGLegProg = MAX(m_Paths[FGROUND][WALK].GetRegularProgress(), m_Paths[FGROUND][WALK].GetTotalTimeProgress());
//            float BGLegProg = MAX(m_Paths[BGROUND][WALK].GetRegularProgress(), m_Paths[BGROUND][WALK].GetTotalTimeProgress());
            float FGLegProg = m_Paths[FGROUND][WALK].GetRegularProgress();
            float BGLegProg = m_Paths[BGROUND][WALK].GetRegularProgress();

            bool restarted = false;

			// Make sure we are starting a stride if we're basically stopped.
			if (isStill) { m_StrideStart = true; }

			if (m_pFGLeg && (!m_pBGLeg || !(m_Paths[FGROUND][WALK].PathEnded() && BGLegProg < 0.5F) || m_StrideStart)) {
				// Reset the stride timer if the path is about to restart.
				if (m_Paths[FGROUND][WALK].PathEnded() || m_Paths[FGROUND][WALK].PathIsAtStart()) { m_StrideTimer.Reset(); }
				m_ArmClimbing[BGROUND] = !m_pFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pFGLeg->GetParentOffset()), m_Vel, m_WalkAngle[FGROUND], m_Paths[FGROUND][WALK], deltaTime, &restarted, false, Vector(0.0F, m_Paths[FGROUND][WALK].GetLowestY()));
				if (restarted) { UpdateWalkAngle(FGROUND); }
			} else {
				m_ArmClimbing[BGROUND] = false;
			}
			if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][WALK].PathEnded() && FGLegProg < 0.5F))) {
				m_StrideStart = false;
				// Reset the stride timer if the path is about to restart.
				if (m_Paths[BGROUND][WALK].PathEnded() || m_Paths[BGROUND][WALK].PathIsAtStart()) { m_StrideTimer.Reset(); }
				m_ArmClimbing[FGROUND] = !m_pBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pBGLeg->GetParentOffset()), m_Vel, m_WalkAngle[BGROUND], m_Paths[BGROUND][WALK], deltaTime, &restarted, false, Vector(0.0F, m_Paths[BGROUND][WALK].GetLowestY()));
				if (restarted) { UpdateWalkAngle(BGROUND); }
			} else {
				if (m_pBGLeg) { m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime); }
				m_ArmClimbing[FGROUND] = false;
			}
			bool climbing = m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND];

			if (restarted) {
				if (!climbing) {
					if (m_StrideSound) { m_StrideSound->Play(m_Pos); }
					RunScriptedFunctionInAppropriateScripts("OnStride");
				} else {
					m_WalkAngle[FGROUND] = Matrix();
					m_WalkAngle[BGROUND] = Matrix();
				}
			}

            ////////////////////////////////////////
            // Arm Climbing if the leg paths failed to find clear spot to restart

//            float FGArmProg = MAX(m_Paths[FGROUND][CLIMB].GetRegularProgress(), m_Paths[FGROUND][CLIMB].GetTotalTimeProgress());
//            float BGArmProg = MAX(m_Paths[BGROUND][CLIMB].GetRegularProgress(), m_Paths[BGROUND][CLIMB].GetTotalTimeProgress());
            float FGArmProg = m_Paths[FGROUND][CLIMB].GetRegularProgress();
            float BGArmProg = m_Paths[BGROUND][CLIMB].GetRegularProgress();

			// TODO: Figure out what this comment means, and then rephrase it better!
            // Slightly negative BGArmProg makes sense because any progress on the starting segments are reported as negative,
            // and there's many starting segments on properly formed climbing paths
			if (climbing) {
				if (m_pFGArm && !m_pFGArm->GetHeldDevice() && !(m_Paths[FGROUND][CLIMB].PathEnded() && BGArmProg > 0.1F)) {	// < 0.5F
					m_ArmClimbing[FGROUND] = true;
					m_Paths[FGROUND][WALK].Terminate();
					m_StrideStart = true;
					// Reset the stride timer if the path is about to restart.
					if (m_Paths[FGROUND][CLIMB].PathEnded() || m_Paths[FGROUND][CLIMB].PathIsAtStart()) { m_StrideTimer.Reset(); }
					m_pFGHandGroup->PushAsLimb(m_Pos + Vector(0, m_pFGArm->GetParentOffset().m_Y).RadRotate(-rot), m_Vel, Matrix(), m_Paths[FGROUND][CLIMB], deltaTime, 0, false);
				} else {
					m_ArmClimbing[FGROUND] = false;
					m_Paths[FGROUND][CLIMB].Terminate();
				}
				if (m_pBGArm) {
					m_ArmClimbing[BGROUND] = true;
					m_Paths[BGROUND][WALK].Terminate();
					m_StrideStart = true;
					// Reset the stride timer if the path is about to restart.
					if (m_Paths[BGROUND][CLIMB].PathEnded() || m_Paths[BGROUND][CLIMB].PathIsAtStart()) { m_StrideTimer.Reset(); }
					m_pBGHandGroup->PushAsLimb(m_Pos + Vector(0, m_pBGArm->GetParentOffset().m_Y).RadRotate(-rot), m_Vel, Matrix(), m_Paths[BGROUND][CLIMB], deltaTime, 0, false);
				} else {
					m_ArmClimbing[BGROUND] = false;
					m_Paths[BGROUND][CLIMB].Terminate();
				}
			}

			// Restart the climbing stroke if the current one seems to be taking too long with no movement.
			if (climbing && isStill && m_StrideTimer.IsPastSimMS(static_cast<double>(m_Paths[BGROUND][CLIMB].GetTotalPathTime() * 0.5F))) {
                m_StrideStart = true;
                m_Paths[FGROUND][CLIMB].Terminate();
                m_Paths[BGROUND][CLIMB].Terminate();
			} else if (m_StrideTimer.IsPastSimMS(static_cast<double>(m_Paths[FGROUND][WALK].GetTotalPathTime() * 1.1F))) {
				// Reset the walking stride if it's taking longer than it should.
                m_StrideStart = true;
                m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
            }
		} else if (m_MoveState == CRAWL) {
			// Start crawling only once we are fully prone.
			if (m_ProneState == PRONE) {

				float FGLegProg = m_Paths[FGROUND][CRAWL].GetRegularProgress();
				float BGLegProg = m_Paths[BGROUND][CRAWL].GetRegularProgress();

				if (m_pFGLeg && (!m_pBGLeg || (!(m_Paths[FGROUND][CRAWL].PathEnded() && BGLegProg < 0.5F) || m_StrideStart))) {
					if (m_Paths[FGROUND][CRAWL].PathEnded() || m_Paths[FGROUND][CRAWL].PathIsAtStart()) { m_StrideTimer.Reset(); }
					m_pFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pFGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[FGROUND][CRAWL], deltaTime);
				} else {
					m_Paths[FGROUND][CRAWL].Terminate();
				}
				if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][CRAWL].PathEnded() && FGLegProg < 0.5F))) {
					m_StrideStart = false;
					if (m_Paths[BGROUND][CRAWL].PathEnded() || m_Paths[BGROUND][CRAWL].PathIsAtStart()) { m_StrideTimer.Reset(); }
					m_pBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pBGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[BGROUND][CRAWL], deltaTime);
				} else {
					m_Paths[BGROUND][CRAWL].Terminate();
				}
				if (m_pBGArm) {
					m_ArmClimbing[BGROUND] = true;
					m_pBGHandGroup->PushAsLimb(m_Pos + RotateOffset(Vector(0, m_pBGArm->GetParentOffset().m_Y)), m_Vel, m_Rotation, m_Paths[BGROUND][ARMCRAWL], deltaTime);
				}
				if (m_pFGArm && !m_pFGArm->GetHeldDevice() && !(m_Paths[FGROUND][ARMCRAWL].PathEnded() && m_Paths[BGROUND][ARMCRAWL].GetRegularProgress() < 0.5F)) {
					m_ArmClimbing[FGROUND] = true;
					m_pFGHandGroup->PushAsLimb(m_Pos + RotateOffset(Vector(0, m_pFGArm->GetParentOffset().m_Y)), m_Vel, m_Rotation, m_Paths[FGROUND][ARMCRAWL], deltaTime);
				}
				// Restart the stride if the current one seems to be taking too long.
				if (m_StrideTimer.IsPastSimMS(m_Paths[FGROUND][CRAWL].GetTotalPathTime())) {
					m_StrideStart = true;
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();
				}
			} else {
				if (m_pFGLeg) { m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pFGLeg->GetMass(), deltaTime); }

				if (m_pBGLeg) { m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime); }
			}
		} else if (m_pFGLeg || m_pBGLeg) {
			if (m_MoveState == JUMP) {
				// TODO: Utilize jump paths in an intuitive way!
				if (m_pFGLeg) { m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pFGLeg->GetMass(), deltaTime); }

				if (m_pBGLeg) { m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pBGLeg->GetMass(), deltaTime); }

				if (m_JetTimeLeft <= 0) {
					m_MoveState = STAND;
					m_Paths[FGROUND][JUMP].Terminate();
					m_Paths[BGROUND][JUMP].Terminate();
					m_Paths[FGROUND][STAND].Terminate();
					m_Paths[BGROUND][STAND].Terminate();
					m_Paths[FGROUND][WALK].Terminate();
					m_Paths[BGROUND][WALK].Terminate();
				}
			} else {
				m_Paths[FGROUND][JUMP].Terminate();
				m_Paths[BGROUND][JUMP].Terminate();
				if (m_MoveState == CROUCH) {
					m_Paths[FGROUND][WALK].Terminate();
					m_Paths[BGROUND][WALK].Terminate();
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();

					if (m_pFGLeg) { m_pFGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, Matrix(), m_Paths[FGROUND][CROUCH], deltaTime); }

					if (m_pBGLeg) { m_pBGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, Matrix(), m_Paths[BGROUND][CROUCH], deltaTime); }

				} else {
					m_Paths[FGROUND][WALK].Terminate();
					m_Paths[BGROUND][WALK].Terminate();
					m_Paths[FGROUND][CRAWL].Terminate();
					m_Paths[BGROUND][CRAWL].Terminate();
					m_Paths[FGROUND][ARMCRAWL].Terminate();
					m_Paths[BGROUND][ARMCRAWL].Terminate();

					if (m_pFGLeg) { m_pFGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, m_WalkAngle[FGROUND], m_Paths[FGROUND][STAND], deltaTime, nullptr, !m_pBGLeg, Vector(0.0F, m_Paths[FGROUND][STAND].GetLowestY())); }

					if (m_pBGLeg) { m_pBGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped), m_Vel, m_WalkAngle[BGROUND], m_Paths[BGROUND][STAND], deltaTime, nullptr, !m_pFGLeg, Vector(0.0F, m_Paths[FGROUND][STAND].GetLowestY())); }
				}
			}
		}
	} else {
		// Not stable/standing, so make sure the end of limbs are moving around limply in a ragdoll fashion.
		// TODO: Make the limb atom groups fly around and react to terrain, without getting stuck etc.
		if (m_pFGArm) { m_pFGHandGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGArm->GetParentOffset()), m_pFGArm->GetMaxLength(), m_PrevVel * m_pFGArm->GetJointStiffness(), m_AngularVel, m_pFGArm->GetMass(), deltaTime); }

		if (m_pBGArm) { m_pBGHandGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGArm->GetParentOffset()), m_pBGArm->GetMaxLength(), m_PrevVel * m_pBGArm->GetJointStiffness(), m_AngularVel, m_pBGArm->GetMass(), deltaTime); }

		if (m_pFGLeg) { m_pFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pFGLeg->GetParentOffset()), m_pFGLeg->GetMaxLength(), m_PrevVel * m_pFGLeg->GetJointStiffness(), m_AngularVel, m_pFGLeg->GetMass(), deltaTime); }

        if (m_pBGLeg) { m_pBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pBGLeg->GetParentOffset()), m_pBGLeg->GetMaxLength(), m_PrevVel * m_pBGLeg->GetJointStiffness(), m_AngularVel, m_pBGLeg->GetMass(), deltaTime); }
	}

    /////////////////////////////////
    // Manage Attachables

    if (m_pHead) {
        float toRotate = 0;
        // Only rotate the head to match the aim angle if body is stable and upright
        if (m_Status == STABLE && std::abs(rot) < (c_HalfPI + c_QuarterPI)) {
            toRotate = m_pHead->GetRotMatrix().GetRadAngleTo((adjustedAimAngle) * m_LookToAimRatio + rot * (0.9F - m_LookToAimRatio)) * 0.15F;
		} else {
			// Rotate the head loosely along with the body if upside down, unstable or dying.
			toRotate = m_pHead->GetRotMatrix().GetRadAngleTo(rot) * m_pHead->GetJointStiffness() * c_QuarterPI;
		}
		m_pHead->SetRotAngle(m_pHead->GetRotAngle() + toRotate);
	}

    if (m_pFGLeg) {
        m_pFGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
        m_pFGLeg->SetTargetPosition(m_pFGFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pBGLeg) {
        m_pBGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
        m_pBGLeg->SetTargetPosition(m_pBGFootGroup->GetLimbPos(m_HFlipped));
    }

	// FG Arm rotating and climbing
	if (m_pFGArm) {
		float affectingBodyAngle = m_Status < INACTIVE ? m_FGArmFlailScalar : 1.0F;
		if (affectingBodyAngle != 0 && m_SharpAimDelay != 0) {
			float aimScalar = std::min(static_cast<float>(m_SharpAimTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			float revertScalar = std::min(static_cast<float>(m_SharpAimRevertTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			aimScalar = (aimScalar > revertScalar) ? aimScalar : 1.0F - revertScalar;

			affectingBodyAngle *= std::abs(std::sin(rot)) * rot * (1.0F - aimScalar);
		}
		m_pFGArm->SetRotAngle(affectingBodyAngle + adjustedAimAngle);

        if (m_Status == STABLE) {
            if (m_ArmClimbing[FGROUND]) {
                m_pFGArm->AddHandTarget("Hand AtomGroup Limb Pos", m_pFGHandGroup->GetLimbPos(m_HFlipped));
			}
        } else if (!m_pFGArm->GetHeldDevice()) {
			m_pFGArm->ClearHandTargets();
			m_pFGArm->AddHandTarget("Arm Flail", m_pFGHandGroup->GetLimbPos(m_HFlipped));
        }
    }

	// BG Arm rotating, climbing, throw animations, supporting fg weapon
    if (m_pBGArm) {
		float affectingBodyAngle = m_Status < INACTIVE ? m_BGArmFlailScalar : 1.0F;
		m_pBGArm->SetRotAngle(std::abs(std::sin(rot)) * rot * affectingBodyAngle + adjustedAimAngle);

        if (m_Status == STABLE) {
			if (m_ArmClimbing[BGROUND]) {
				// Can't climb or crawl with the shield
				if (m_MoveState != CRAWL || m_ProneState == PRONE) {
					UnequipBGArm();
				}
				m_pBGArm->AddHandTarget("Hand AtomGroup Limb Pos", m_pBGHandGroup->GetLimbPos(m_HFlipped));
			} else {
				HeldDevice *heldDevice = GetEquippedItem();
				ThrownDevice *thrownDevice = dynamic_cast<ThrownDevice *>(heldDevice);
				if (thrownDevice && (m_ArmsState == THROWING_PREP || isSharpAiming)) {
					m_pBGArm->AddHandTarget("End Throw Offset", m_pBGArm->GetJointPos() + thrownDevice->GetEndThrowOffset().GetXFlipped(m_HFlipped).RadRotate(adjustedAimAngle));
				} else if (heldDevice) {
					if (HeldDevice *bgDevice = GetEquippedBGItem(); bgDevice && !heldDevice->IsOneHanded()) {
						UnequipBGArm();
					} else if (!bgDevice && !heldDevice->IsReloading() && heldDevice->IsSupportable()) {
						m_pBGArm->SetHeldDeviceThisArmIsTryingToSupport(heldDevice);

						if (!m_pBGArm->HasAnyHandTargets() && m_pBGArm->GetHandHasReachedCurrentTarget()) {
							heldDevice->SetSupported(true);
							m_pBGArm->SetRecoil(heldDevice->GetRecoilForce(), heldDevice->GetRecoilOffset(), heldDevice->IsRecoiled());
						} else {
							// BGArm did not reach to support the device. Count device as supported anyway, if crouching or prone.
							heldDevice->SetSupported(m_MoveState == CROUCH || m_ProneState == PRONE);
							m_pBGArm->SetRecoil(Vector(), Vector(), false);
						}
					}
				}
			}
        } else {
			m_pBGArm->ClearHandTargets();
			m_pBGArm->AddHandTarget("Arm Flail", m_pBGHandGroup->GetLimbPos(m_HFlipped));
        }
	} else if (HeldDevice *heldDevice = GetEquippedItem()) {
		heldDevice->SetSupported(false);
    }
	// Make sure the bg arm doesn't think it's supporting something when it isn't.
	if (m_pBGArm && (!m_pFGArm || !m_pFGArm->GetHeldDevice() || m_pBGArm->GetHeldDevice())) {
		m_pBGArm->SetHeldDeviceThisArmIsTryingToSupport(nullptr);
	}

	/////////////////////////////////
	// Arm swinging or device swaying walking animations

	if (m_MoveState != MovementState::STAND && (m_ArmSwingRate != 0 || m_DeviceArmSwayRate != 0)) {
		for (Arm *arm : { m_pFGArm, m_pBGArm }) {
			if (arm && !arm->GetHeldDeviceThisArmIsTryingToSupport()) {
				Leg *legToSwingWith = arm == m_pFGArm ? m_pBGLeg : m_pFGLeg;
				Leg *otherLeg = legToSwingWith == m_pBGLeg ? m_pFGLeg : m_pBGLeg;
				if (!legToSwingWith || m_MoveState == JUMP || m_MoveState == CROUCH) {
					std::swap(legToSwingWith, otherLeg);
				}

				if (legToSwingWith) {
					float armMovementRateToUse = m_ArmSwingRate;
					if (HeldDevice *heldDevice = arm->GetHeldDevice()) {
						armMovementRateToUse = m_DeviceArmSwayRate * (1.0F - m_SharpAimProgress) * std::sin(std::abs(heldDevice->GetStanceOffset().GetAbsRadAngle()));
					}
					float angleToSwingTo = std::sin(legToSwingWith->GetRotAngle() + (c_HalfPI * GetFlipFactor()));
					arm->SetHandIdleRotation(angleToSwingTo * armMovementRateToUse);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

void AHuman::Update()
{
    float rot = m_Rotation.GetRadAngle(); // eugh, for backwards compat to be the same behaviour as with multithreaded AI

    Actor::Update();

    ////////////////////////////////////
    // Update viewpoint

    // Set viewpoint based on how we are aiming etc.
    Vector aimSight(m_AimDistance, 0);
    Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
    aimMatrix.SetXFlipped(m_HFlipped);
    // Reset this each frame
    m_SharpAimMaxedOut = false;

	if (HeldDevice *heldDevice = GetEquippedItem()) {
		float maxLength = heldDevice->GetSharpLength();
		if (maxLength == 0) {
			m_SharpAimProgress = 0;
			m_SharpAimMaxedOut = true;
		} else if (m_MoveState == WALK) {
			maxLength *= 0.7F;
		}
		// Use a non-terrain check ray to cap the magnitude, so we can't see into objects etc
		if (m_SharpAimProgress > 0) {
			// TODO: make an uniform function to get the total GripStrength of an AHuman?
			float totalGripStrength = m_pFGArm->GetGripStrength();
			if (m_pBGArm) {
				if (m_pBGArm->GetHeldDevice()) {
					HeldDevice *heldBGDevice = m_pBGArm->GetHeldDevice();
					if (heldBGDevice->IsRecoiled()) {
						m_SharpAimProgress *= 1.0F - std::min(heldBGDevice->GetRecoilForce().GetMagnitude() / std::max(m_pBGArm->GetGripStrength() * heldBGDevice->GetGripStrengthMultiplier(), 1.0F), 1.0F);
					}
				} else if (heldDevice->GetSupported()) {
					totalGripStrength += m_pBGArm->GetGripStrength();
				}
			}
			if (heldDevice->IsRecoiled()) {
				m_SharpAimProgress *= 1.0F - std::min(heldDevice->GetRecoilForce().GetMagnitude() / std::max(totalGripStrength * heldDevice->GetGripStrengthMultiplier(), 1.0F), 1.0F);
			}
			Vector notUsed;
            Vector sharpAimVector(maxLength, 0);
            sharpAimVector *= aimMatrix;

            // See how far along the sharp aim vector there is opaque air
            float result = g_SceneMan.CastObstacleRay(heldDevice->GetMuzzlePos(), sharpAimVector, notUsed, notUsed, GetRootID(), IgnoresWhichTeam(), g_MaterialAir, 5);
            // If we didn't find anything but air before the sharpdistance, then don't alter the sharp distance
            if (result >= 0 && result < (maxLength * m_SharpAimProgress))
            {
                m_SharpAimProgress = result / maxLength;
                m_SharpAimMaxedOut = true;
            }
        }
		if (m_SharpAimProgress > 0.9F) { m_SharpAimMaxedOut = true; }
        aimSight.m_X += maxLength * m_SharpAimProgress;
    }

    // Rotate the aiming spot vector and add it to the view point
    aimSight *= aimMatrix;
    m_ViewPoint = m_Pos.GetFloored() + aimSight;

	// Add velocity also so the viewpoint moves ahead at high speeds
	if (m_Vel.MagnitudeIsGreaterThan(10.0F)) { m_ViewPoint += m_Vel * std::sqrt(m_Vel.GetMagnitude() * 0.1F); }

    ////////////////////////////////////////
    // Balance stuff

    // Eliminate full rotations
    while (std::abs(rot) > c_TwoPI) {
        rot -= rot > 0 ? c_TwoPI : -c_TwoPI;
    }
    // Eliminate rotations over half a turn
    if (std::abs(rot) > c_PI) {
        rot = (rot > 0 ? -c_PI : c_PI) + (rot - (rot > 0 ? c_PI : -c_PI));
        // If we're upside down, we're unstable damnit
		if (m_Status == STABLE) { m_Status = UNSTABLE; }
        m_StableRecoverTimer.Reset();
    }

    // Rotational balancing spring calc
	if (m_Status == STABLE) {

        // If we're supposed to be laying down on the ground, make the spring pull the body that way until we reach that angle
        if (m_ProneState != NOTPRONE)
        {
            float rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
            float rotDiff = rotTarget - rot;

			if (m_ProneState == GOPRONE) {
				if (!m_ProneTimer.IsPastSimMS(333)) {
					if (std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
						m_AngularVel += rotDiff * 0.4F;
						m_Vel.m_X += (m_HFlipped ? -std::abs(rotDiff) : std::abs(rotDiff)) / std::max(m_Vel.GetMagnitude(), 4.0F);
					}
                } else {
					// Done going down, now stay down without spring.
                    m_AngularVel *= 0.5F;
					m_ProneState = PRONE;
                }
			} else if (m_ProneState == PRONE) {
				// If down, try to keep flat against the ground.
				if (std::abs(rotDiff) > c_SixteenthPI && std::abs(rotDiff) < c_HalfPI) {
					m_AngularVel += rotDiff * 0.65F;
				} else if (std::abs(m_AngularVel) > 0.3F) {
					m_AngularVel *= 0.85F;
				}
			}
		} else {
			// Upright body posture
			float rotDiff = rot - (GetRotAngleTarget(m_MoveState) * (m_AimAngle > 0 ? 1.0F - (m_AimAngle / c_HalfPI) : 1.0F) * GetFlipFactor());
			m_AngularVel = m_AngularVel * (0.98F - 0.06F * (m_Health / m_MaxHealth)) - (rotDiff * 0.5F);
		}
    }
	else if (m_Status == UNSTABLE) {
		float rotTarget = 0;
		// If traveling at speed, always start falling forward.
		if (std::abs(m_Vel.m_X) > 1.0F) {
			rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
		} else {
			// Otherwise, go whichever way we're already rotated.
			rotTarget = rot > 0 ? c_HalfPI : -c_HalfPI;
		}

		float rotDiff = rotTarget - rot;
		if (std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
			m_AngularVel += rotDiff * 0.05F;
		}
	} else if (m_Status == DYING) {
		float rotTarget = m_Vel.m_X - (rot + m_AngularVel) > 0 ? -c_HalfPI : c_HalfPI;
		float rotDiff = rotTarget - rot;
		if (!m_DeathTmr.IsPastSimMS(125) && std::abs(rotDiff) > 0.1F && std::abs(rotDiff) < c_PI) {
			// TODO: finetune this for situations like low gravity!
			float velScalar = 0.5F; //* (g_SceneMan.GetGlobalAcc().GetY() * m_GlobalAccScalar) / c_PPM;
			m_AngularVel += rotDiff * velScalar;
			m_Vel.m_X += (rotTarget > 0 ? -std::abs(rotDiff) : std::abs(rotDiff)) * velScalar * 0.5F;
		} else {
			m_Status = DEAD;
		}
	}
    m_Rotation.SetRadAngle(rot);

    ///////////////////////////////////////////////////
    // Death detection and handling

	if (!m_pHead && m_Status != DYING && m_Status != DEAD) {
		m_Health -= m_MaxHealth + 1.0F;
	} else if (!m_pFGArm && !m_pBGArm && !m_pFGLeg && !m_pBGLeg && m_Status != DYING && m_Status != DEAD) {
		m_Health -= 0.1F;
	}

    if (m_Status == DYING) {
		if (m_pFGArm) { m_pFGArm->RemoveAttachable(m_pFGArm->GetHeldDevice(), true, false); }
		if (m_pBGArm) { m_pBGArm->RemoveAttachable(m_pBGArm->GetHeldDevice(), true, false); }
    }

    /////////////////////////////////////////
    // Misc.

//    m_DeepCheck = true/*m_Status == DEAD*/;
}


//////////////////////////////////////////////////////////////////////////////////////////

void AHuman::DrawThrowingReticle(BITMAP *targetBitmap, const Vector &targetPos, float progressScalar) const {
	const int pointCount = 9;
	Vector points[pointCount];

	for (int index = 0; index < pointCount; index++) {
		points[index].SetXY(static_cast<float>(index * 4), 0.0F);
	}
	Vector outOffset(m_pFGArm->GetMaxLength() * GetFlipFactor(), -m_pFGArm->GetMaxLength() * 0.5F);
	float adjustedAimAngle = m_AimAngle * GetFlipFactor();

	acquire_bitmap(targetBitmap);

	for (int i = 0; i < pointCount * progressScalar; ++i) {
		points[i].FlipX(m_HFlipped);
		points[i] += outOffset;
		points[i].RadRotate(adjustedAimAngle);
		points[i] += m_pFGArm->GetJointPos();

		g_PostProcessMan.RegisterGlowDotEffect(points[i], YellowDot, RandomNum(63, 127));
		putpixel(targetBitmap, points[i].GetFloorIntX() - targetPos.GetFloorIntX(), points[i].GetFloorIntY() - targetPos.GetFloorIntY(), g_YellowGlowColor);
	}

	release_bitmap(targetBitmap);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AHuman's current graphical representation to a
//                  BITMAP of choice.

void AHuman::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    Actor::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    DrawMode realMode = (mode == g_DrawColor && m_FlashWhiteMS) ? g_DrawWhite : mode;
    // Note: For some reason the ordering of the attachables list can get messed up. The most important thing here is that the FGArm is on top of everything else.
    if (m_pHead && m_pHead->IsDrawnAfterParent()) { m_pHead->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical); }
    if (m_pFGArm) { m_pFGArm->Draw(pTargetBitmap, targetPos, realMode, onlyPhysical); }

    // Draw background Arm's hand after the HeldDevice of FGArm is drawn if the FGArm is holding a weapon.
    if (m_pFGArm && m_pBGArm && !onlyPhysical && mode == g_DrawColor && m_pBGArm->GetHandHasReachedCurrentTarget() && !GetEquippedBGItem()) {
		if (HeldDevice *heldDevice = m_pFGArm->GetHeldDevice(); heldDevice && !dynamic_cast<ThrownDevice *>(heldDevice) && !heldDevice->IsReloading() && !heldDevice->IsShield()) {
			m_pBGArm->DrawHand(pTargetBitmap, targetPos, realMode);
		}
    }

    if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawHandAndFootGroupVisualizations()) {
        m_pFGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pBGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pFGHandGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pBGHandGroup->Draw(pTargetBitmap, targetPos, true, 13);
    }

    if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawLimbPathVisualizations()) {
        m_Paths[m_HFlipped][WALK].Draw(pTargetBitmap, targetPos, 122);
        m_Paths[m_HFlipped][CRAWL].Draw(pTargetBitmap, targetPos, 122);
        m_Paths[m_HFlipped][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
        m_Paths[m_HFlipped][CLIMB].Draw(pTargetBitmap, targetPos, 165);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void AHuman::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
	m_HUDStack = -m_CharHeight / 2;

    // Only do HUD if on a team
    if (m_Team < 0)
        return;

	// Only draw if the team viewing this is on the same team OR has seen the space where this is located.
	int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && (!g_SettingsMan.ShowEnemyHUD() || g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam))) {
		return;
	}

    Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);

	if (!m_HUDVisible) {
		return;
	}

#ifdef DEBUG_BUILD
    // Limbpath debug drawing
    m_Paths[FGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
    m_Paths[FGROUND][CRAWL].Draw(pTargetBitmap, targetPos, 122);
    m_Paths[FGROUND][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
    m_Paths[FGROUND][CLIMB].Draw(pTargetBitmap, targetPos, 98);

    m_Paths[BGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
    m_Paths[BGROUND][CRAWL].Draw(pTargetBitmap, targetPos, 122);
    m_Paths[BGROUND][ARMCRAWL].Draw(pTargetBitmap, targetPos, 13);
    m_Paths[BGROUND][CLIMB].Draw(pTargetBitmap, targetPos, 98);

    // Draw the AI paths
    std::list<Vector>::iterator last = m_MovePath.begin();
    Vector waypoint, lastPoint, lineVec;
    for (std::list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end(); ++lItr)
    {
        lastPoint = (*last) - targetPos;
        waypoint = lastPoint + g_SceneMan.ShortestDistance(lastPoint, (*lItr) - targetPos);
        line(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, waypoint.m_X, waypoint.m_Y, g_RedColor);
        last = lItr;
    }
    waypoint = m_MoveTarget - targetPos;
    circlefill(pTargetBitmap, waypoint.m_X, waypoint.m_Y, 3, g_RedColor);
    lastPoint = m_PrevPathTarget - targetPos;
    circlefill(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, 2, g_YellowGlowColor);
    lastPoint = m_DigTunnelEndPos - targetPos;
    circlefill(pTargetBitmap, lastPoint.m_X, lastPoint.m_Y, 2, g_YellowGlowColor);
    // Radius
//    waypoint = m_Pos - targetPos;
//    circle(pTargetBitmap, waypoint.m_X, waypoint.m_Y, m_MoveProximityLimit, g_RedColor);
#endif

    // Player AI drawing

	if (m_pFGArm && m_pFGArm->GetHeldDevice()) {
		// Draw the aiming dots for the currently held device.
		if (m_ArmsState == THROWING_PREP) {
			DrawThrowingReticle(pTargetBitmap, targetPos, GetThrowProgress());
		} else if (m_Controller.IsState(AIM_SHARP) || (m_Controller.IsPlayerControlled() && !m_Controller.IsState(PIE_MENU_ACTIVE))) {
			m_pFGArm->GetHeldDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen, m_Controller.IsState(AIM_SHARP) && m_Controller.IsPlayerControlled());
		}
	}

    //////////////////////////////////////
    // Draw stat info HUD
    char str[64];

    GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();

    // Only show extra HUD if this guy is controlled by the same player that this screen belongs to
    if (m_Controller.IsPlayerControlled() && g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller.GetPlayer()) == whichScreen && pSmallFont && pSymbolFont)
    {
        AllegroBitmap allegroBitmap(pTargetBitmap);
/*
        // Device aiming reticle
        if (m_Controller.IsState(AIM_SHARP) &&
            m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
            m_pFGArm->GetHeldDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen);*/

        Vector drawPos = m_Pos - targetPos;

        // Adjust the draw position to work if drawn to a target screen bitmap that is straddling a scene seam
        if (!targetPos.IsZero())
        {
            // Spans vertical scene seam
            int sceneWidth = g_SceneMan.GetSceneWidth();
            if (g_SceneMan.SceneWrapsX() && pTargetBitmap->w < sceneWidth)
            {
                if ((targetPos.m_X < 0) && (m_Pos.m_X > (sceneWidth - pTargetBitmap->w)))
                    drawPos.m_X -= sceneWidth;
                else if (((targetPos.m_X + pTargetBitmap->w) > sceneWidth) && (m_Pos.m_X < pTargetBitmap->w))
                    drawPos.m_X += sceneWidth;
            }
            // Spans horizontal scene seam
            int sceneHeight = g_SceneMan.GetSceneHeight();
            if (g_SceneMan.SceneWrapsY() && pTargetBitmap->h < sceneHeight)
            {
                if ((targetPos.m_Y < 0) && (m_Pos.m_Y > (sceneHeight - pTargetBitmap->h)))
                    drawPos.m_Y -= sceneHeight;
                else if (((targetPos.m_Y + pTargetBitmap->h) > sceneHeight) && (m_Pos.m_Y < pTargetBitmap->h))
                    drawPos.m_Y += sceneHeight;
            }
        }

		if (m_pFGArm || m_pBGArm) {
			// Held-related GUI stuff
            HDFirearm *fgHeldFirearm = dynamic_cast<HDFirearm *>(GetEquippedItem());
			HDFirearm *bgHeldFirearm = dynamic_cast<HDFirearm *>(GetEquippedBGItem());

            if (fgHeldFirearm || bgHeldFirearm) {
                str[0] = -56;
				str[1] = 0;

				std::string fgWeaponString = "EMPTY";
				if (fgHeldFirearm) {
					if (fgHeldFirearm->IsReloading()) {
						fgWeaponString = "Reloading";
						int barColorIndex = 77;
						if (!fgHeldFirearm->GetSupportAvailable()) {
							float reloadMultiplier = fgHeldFirearm->GetOneHandedReloadTimeMultiplier();
							if (reloadMultiplier != 1.0F) {
								// Add a hand icon next to the ammo icon when reloading without supporting hand.
								str[0] = -37; str[1] = -49; str[2] = -56; str[3] = 0;
								if (reloadMultiplier > 1.0F) {
									if (m_IconBlinkTimer.AlternateSim(250)) { barColorIndex = 13; }
								} else {
									barColorIndex = 133;
								}
							}
						}
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 13, drawPos.GetFloorIntX() + 29, drawPos.GetFloorIntY() + m_HUDStack + 14, 245);
						rectfill(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 12, drawPos.GetFloorIntX() + static_cast<int>(28.0F * fgHeldFirearm->GetReloadProgress() + 0.5F), drawPos.GetFloorIntY() + m_HUDStack + 13, barColorIndex);
					} else {
						fgWeaponString = fgHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(fgHeldFirearm->GetRoundInMagCount());
					}
				}

				std::string bgWeaponString;
                if (bgHeldFirearm) {
					if (bgHeldFirearm->IsReloading()) {
						bgWeaponString = "Reloading";
						int barColorIndex = 77;
						if (!bgHeldFirearm->GetSupportAvailable()) {
							float reloadMultiplier = bgHeldFirearm->GetOneHandedReloadTimeMultiplier();
							if (reloadMultiplier != 1.0F) {
								// Add a hand icon next to the ammo icon when reloading without supporting hand.
								str[0] = -37; str[1] = -49; str[2] = -56; str[3] = 0;
								if (reloadMultiplier > 1.0F) {
									if (m_IconBlinkTimer.AlternateSim(250)) { barColorIndex = 13; }
								} else {
									barColorIndex = 133;
								}
							}
						}
						int totalTextWidth = pSmallFont->CalculateWidth(fgWeaponString) + 6;
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, drawPos.GetFloorIntX() + 29 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 14, 245);
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 12, drawPos.GetFloorIntX() + static_cast<int>(28.0F * bgHeldFirearm->GetReloadProgress() + 0.5F) + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, barColorIndex);
					} else {
						bgWeaponString = bgHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(bgHeldFirearm->GetRoundInMagCount());
					}
                }
				pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - pSymbolFont->CalculateWidth(str) - 3, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Left);
				std::snprintf(str, sizeof(str), bgHeldFirearm ? "%s | %s" : "%s", fgWeaponString.c_str(), bgWeaponString.c_str());
                pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 3, str, GUIFont::Left);

				m_HUDStack -= 9;
            }
			if (m_Controller.IsState(PIE_MENU_ACTIVE) || !m_EquipHUDTimer.IsPastRealMS(700)) {
				HeldDevice *fgEquippedItem = GetEquippedItem();
				HeldDevice *bgEquippedItem = GetEquippedBGItem();
				std::string equippedItemsString = (fgEquippedItem ? fgEquippedItem->GetPresetName() : "EMPTY") + (bgEquippedItem ? " | " + bgEquippedItem->GetPresetName() : "");
				pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 3, equippedItemsString, GUIFont::Centre);
				m_HUDStack -= 9;
			}
        }
        else
        {
            std::snprintf(str, sizeof(str), "NO ARM!");
            pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
            m_HUDStack -= 9;
        }

		if (m_pJetpack && m_Status != INACTIVE && !m_Controller.IsState(PIE_MENU_ACTIVE) && (m_Controller.IsState(BODY_JUMP) || m_JetTimeLeft < m_JetTimeTotal)) {
			if (m_JetTimeLeft < 100.0F) {
				str[0] = m_IconBlinkTimer.AlternateSim(100) ? -26 : -25;
			} else if (m_pJetpack->IsEmitting()) {
				float acceleration = m_pJetpack->EstimateImpulse(false) / std::max(GetMass(), 0.1F);
				if (acceleration > 0.41F) {
					str[0] = acceleration > 0.47F ? -31 : -30;
				} else {
					str[0] = acceleration > 0.35F ? -29 : -28;
					if (m_IconBlinkTimer.AlternateSim(200)) { str[0] = -27; }
				}
			} else {
				str[0] = -27;
			}
			str[1] = 0;
			pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - 7, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Centre);

			rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1, drawPos.GetFloorIntY() + m_HUDStack + 7, drawPos.GetFloorIntX() + 15, drawPos.GetFloorIntY() + m_HUDStack + 8, 245);
			if (m_JetTimeTotal > 0) {
				float jetTimeRatio = m_JetTimeLeft / m_JetTimeTotal;
				int gaugeColor;
				if (jetTimeRatio > 0.75F) {
					gaugeColor = 149;
				} else if (jetTimeRatio > 0.5F) {
					gaugeColor = 133;
				} else if (jetTimeRatio > 0.375F) {
					gaugeColor = 77;
				} else if (jetTimeRatio > 0.25F) {
					gaugeColor = 48;
				} else {
					gaugeColor = 13;
				}
				rectfill(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 6, drawPos.GetFloorIntX() + static_cast<int>(15.0F * jetTimeRatio), drawPos.GetFloorIntY() + m_HUDStack + 7, gaugeColor);
			}
			m_HUDStack -= 9;
		}

        // Pickup GUI
        if (!m_Controller.IsState(PIE_MENU_ACTIVE) && m_pItemInReach) {
            std::snprintf(str, sizeof(str), " %c %s", -49, m_pItemInReach->GetPresetName().c_str());
            pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 3, str, GUIFont::Centre);
			m_HUDStack -= 9;
        }
/*
        // AI Mode select GUI HUD
        if (m_Controller.IsState(AI_MODE_SET))
        {
            int iconOff = m_apAIIcons[0]->w + 2;
            int iconColor = m_Team == Activity::TeamOne ? AIICON_RED : AIICON_GREEN;
            Vector iconPos = GetCPUPos() - targetPos;

            if (m_AIMode == AIMODE_SENTRY)
            {
                std::snprintf(str, sizeof(str), "%s", "Sentry");
                pSmallFont->DrawAligned(&allegroBitmap, iconPos.m_X, iconPos.m_Y - 18, str, GUIFont::Centre);
            }
            else if (m_AIMode == AIMODE_PATROL)
            {
                std::snprintf(str, sizeof(str), "%s", "Patrol");
                pSmallFont->DrawAligned(&allegroBitmap, iconPos.m_X - 9, iconPos.m_Y - 5, str, GUIFont::Right);
            }
            else if (m_AIMode == AIMODE_BRAINHUNT)
            {
                std::snprintf(str, sizeof(str), "%s", "Brainhunt");
                pSmallFont->DrawAligned(&allegroBitmap, iconPos.m_X + 9, iconPos.m_Y - 5, str, GUIFont::Left);
            }
            else if (m_AIMode == AIMODE_GOLDDIG)
            {
                std::snprintf(str, sizeof(str), "%s", "Gold Dig");
                pSmallFont->DrawAligned(&allegroBitmap, iconPos.m_X, iconPos.m_Y + 8, str, GUIFont::Centre);
            }

            // Draw the mode alternatives if they are not the current one
            if (m_AIMode != AIMODE_SENTRY)
            {
                draw_sprite(pTargetBitmap, m_apAIIcons[AIMODE_SENTRY], iconPos.m_X - 6, iconPos.m_Y - 6 - iconOff);
            }
            if (m_AIMode != AIMODE_PATROL)
            {
                draw_sprite(pTargetBitmap, m_apAIIcons[AIMODE_PATROL], iconPos.m_X - 6 - iconOff, iconPos.m_Y - 6);
            }
            if (m_AIMode != AIMODE_BRAINHUNT)
            {
                draw_sprite(pTargetBitmap, m_apAIIcons[AIMODE_BRAINHUNT], iconPos.m_X - 6 + iconOff, iconPos.m_Y - 6);
            }
            if (m_AIMode != AIMODE_GOLDDIG)
            {
                draw_sprite(pTargetBitmap, m_apAIIcons[AIMODE_GOLDDIG], iconPos.m_X - 6, iconPos.m_Y - 6 + iconOff);
            }
        }
*/
    }

    // AI mode state debugging
#ifdef DEBUG_BUILD

    AllegroBitmap allegroBitmap(pTargetBitmap);
    Vector drawPos = m_Pos - targetPos;

    // Dig state
    if (m_DigState == PREDIG)
        std::snprintf(str, sizeof(str), "PREDIG");
    else if (m_DigState == STARTDIG)
        std::snprintf(str, sizeof(str), "STARTDIG");
    else if (m_DigState == TUNNELING)
        std::snprintf(str, sizeof(str), "TUNNELING");
    else if (m_DigState == FINISHINGDIG)
        std::snprintf(str, sizeof(str), "FINISHINGDIG");
    else if (m_DigState == PAUSEDIGGER)
        std::snprintf(str, sizeof(str), "PAUSEDIGGER");
    else
        std::snprintf(str, sizeof(str), "NOTDIGGING");
    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
    m_HUDStack += -9;

    // Device State
    if (m_DeviceState == POINTING)
        std::snprintf(str, sizeof(str), "POINTING");
    else if (m_DeviceState == SCANNING)
        std::snprintf(str, sizeof(str), "SCANNING");
    else if (m_DeviceState == AIMING)
        std::snprintf(str, sizeof(str), "AIMING");
    else if (m_DeviceState == FIRING)
        std::snprintf(str, sizeof(str), "FIRING");
    else if (m_DeviceState == THROWING)
        std::snprintf(str, sizeof(str), "THROWING");
    else if (m_DeviceState == DIGGING)
        std::snprintf(str, sizeof(str), "DIGGING");
    else
        std::snprintf(str, sizeof(str), "STILL");
    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
    m_HUDStack += -9;

    // Jump State
    if (m_JumpState == FORWARDJUMP)
        std::snprintf(str, sizeof(str), "FORWARDJUMP");
    else if (m_JumpState == PREUPJUMP)
        std::snprintf(str, sizeof(str), "PREUPJUMP");
    else if (m_JumpState == UPJUMP)
        std::snprintf(str, sizeof(str), "UPJUMP");
    else if (m_JumpState == APEXJUMP)
        std::snprintf(str, sizeof(str), "APEXJUMP");
    else if (m_JumpState == LANDJUMP)
        std::snprintf(str, sizeof(str), "LANDJUMP");
    else
        std::snprintf(str, sizeof(str), "NOTJUMPING");
    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
    m_HUDStack += -9;

    if (m_Status == STABLE)
        std::snprintf(str, sizeof(str), "STABLE");
    else if (m_Status == UNSTABLE)
        std::snprintf(str, sizeof(str), "UNSTABLE");
    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
    m_HUDStack += -9;

#endif

}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get walking limb path speed for the specified preset.

float AHuman::GetLimbPathSpeed(int speedPreset) const
{
	return m_Paths[FGROUND][WALK].GetSpeed(speedPreset);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set walking limb path speed for the specified preset.

void AHuman::SetLimbPathSpeed(int speedPreset, float speed)
{
	m_Paths[FGROUND][WALK].OverrideSpeed(speedPreset, speed);
	m_Paths[BGROUND][WALK].OverrideSpeed(speedPreset, speed);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with.

float AHuman::GetLimbPathPushForce() const
{
	return m_Paths[FGROUND][WALK].GetDefaultPushForce();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with.

void AHuman::SetLimbPathPushForce(float force)
{
	m_Paths[FGROUND][WALK].OverridePushForce(force);
	m_Paths[BGROUND][WALK].OverridePushForce(force);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AHuman::WhilePieMenuOpenListener(const PieMenu *pieMenu) {
	int result = Actor::WhilePieMenuOpenListener(pieMenu);

	for (PieSlice *pieSlice : GetPieMenu()->GetPieSlices()) {
		switch (pieSlice->GetType()) {
			case PieSlice::SliceType::Pickup:
			case PieSlice::SliceType::Reload:
				pieSlice->SetType(m_pItemInReach ? PieSlice::SliceType::Pickup : PieSlice::SliceType::Reload);
				pieSlice->SetIcon(dynamic_cast<Icon *>(g_PresetMan.GetEntityPreset("Icon", m_pItemInReach ? "Pick Up" : "Refresh")->Clone()));

				if (pieSlice->GetType() == PieSlice::SliceType::Pickup) {
					if (m_pFGArm || (m_pBGArm && m_pItemInReach->IsOneHanded())) {
						pieSlice->SetEnabled(m_Status != INACTIVE);
						pieSlice->SetDescription("Pick Up " + m_pItemInReach->GetPresetName());
					} else {
						pieSlice->SetEnabled(false);
						pieSlice->SetDescription("No Arm");
					}
				} else {
					const HeldDevice *fgHeldDevice = dynamic_cast<const HeldDevice *>(GetEquippedItem());
					const HeldDevice *bgHeldDevice = dynamic_cast<const HeldDevice *>(GetEquippedBGItem());
					if (fgHeldDevice || bgHeldDevice) {
						pieSlice->SetEnabled(m_Status != INACTIVE && ((fgHeldDevice && !fgHeldDevice->IsFull()) || (bgHeldDevice && !bgHeldDevice->IsFull())));
						pieSlice->SetDescription("Reload");
					} else {
						pieSlice->SetEnabled(false);
						pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
					}
				}
				break;
			case PieSlice::SliceType::NextItem:
				if (!IsInventoryEmpty() && m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Next Item");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
				}
				break;
			case PieSlice::SliceType::PreviousItem:
				if (!IsInventoryEmpty() && m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Prev Item");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription(m_pFGArm ? "Not Holding Anything" : "No Arm");
				}
				break;
			case PieSlice::SliceType::Drop:
				if (const MovableObject *equippedFGItem = GetEquippedItem()) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Drop " + equippedFGItem->GetPresetName());
				} else if (const MovableObject *equippedBGItem = GetEquippedBGItem()) {
					pieSlice->SetDescription("Drop " + equippedBGItem->GetPresetName());
					pieSlice->SetEnabled(m_Status != INACTIVE);
				} else if (!IsInventoryEmpty() && !m_pFGArm) {
					pieSlice->SetEnabled(m_Status != INACTIVE);
					pieSlice->SetDescription("Drop Inventory");
				} else {
					pieSlice->SetEnabled(false);
					pieSlice->SetDescription((m_pFGArm || m_pBGArm) ? "Not Holding Anything" : "No Arm");
				}
				break;
		}
	}
	return result;
}


} // namespace RTE
