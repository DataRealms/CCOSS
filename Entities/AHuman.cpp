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
#include "AEmitter.h"
#include "HDFirearm.h"
#include "SLTerrain.h"
#include "PresetMan.h"
#include "PieMenuGUI.h"
#include "Scene.h"
#include "SettingsMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(AHuman, Actor, 20)


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
	m_JetAngleRange = 0.25F;
    m_GoldInInventoryChunk = 0;
    m_ThrowTmr.Reset();
    m_ThrowPrepTime = 1000;
	m_SharpAimRevertTimer.Reset();
	m_FGArmFlailScalar = 0.0F;
	m_BGArmFlailScalar = 0.7F;

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
    // Read all the properties
    if (Actor::Create() < 0)
        return -1;

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
    if (m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldMO())
    {
        m_pFGArm->SetHeldMO(SwapNextInventory(0, true));
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
    //Note - hardcoded attachable copying is organized based on desired draw order here.
    if (reference.m_pBGArm) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGArm->GetUniqueID());
        SetBGArm(dynamic_cast<Arm *>(reference.m_pBGArm->Clone()));
    }
    if (reference.m_pBGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pBGLeg->GetUniqueID());
        SetBGLeg(dynamic_cast<Leg *>(reference.m_pBGLeg->Clone()));
    }
    if (reference.m_pJetpack) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pJetpack->GetUniqueID());
        SetJetpack(dynamic_cast<AEmitter *>(reference.m_pJetpack->Clone()));
    }
    if (reference.m_pHead) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pHead->GetUniqueID());
        SetHead(dynamic_cast<Attachable *>(reference.m_pHead->Clone()));
    }
    if (reference.m_pFGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGLeg->GetUniqueID());
        SetFGLeg(dynamic_cast<Leg *>(reference.m_pFGLeg->Clone()));
    }
    if (reference.m_pFGArm) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFGArm->GetUniqueID());
        SetFGArm(dynamic_cast<Arm *>(reference.m_pFGArm->Clone()));
    }
    Actor::Create(reference);

	m_LookToAimRatio = reference.m_LookToAimRatio;

	m_ThrowPrepTime = reference.m_ThrowPrepTime;
    m_JetTimeTotal = reference.m_JetTimeTotal;
    m_JetTimeLeft = reference.m_JetTimeLeft;
	m_JetAngleRange = reference.m_JetAngleRange;
	m_FGArmFlailScalar = reference.m_FGArmFlailScalar;
	m_BGArmFlailScalar = reference.m_BGArmFlailScalar;

    m_pFGHandGroup = dynamic_cast<AtomGroup *>(reference.m_pFGHandGroup->Clone());
    m_pFGHandGroup->SetOwner(this);
    m_pBGHandGroup = dynamic_cast<AtomGroup *>(reference.m_pBGHandGroup->Clone());
    m_pBGHandGroup->SetOwner(this);
    m_pFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pFGFootGroup->Clone());
    m_pFGFootGroup->SetOwner(this);
    m_BackupFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupFGFootGroup->Clone());
    m_BackupFGFootGroup->SetOwner(this);
    m_BackupFGFootGroup->SetLimbPos(reference.m_BackupFGFootGroup->GetLimbPos());
    m_pBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pBGFootGroup->Clone());
    m_pBGFootGroup->SetOwner(this);
    m_BackupBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupBGFootGroup->Clone());
    m_BackupBGFootGroup->SetOwner(this);
    m_BackupBGFootGroup->SetLimbPos(reference.m_BackupBGFootGroup->GetLimbPos());

	if (reference.m_StrideSound) { m_StrideSound = dynamic_cast<SoundContainer*>(reference.m_StrideSound->Clone()); }

    m_ArmsState = reference.m_ArmsState;
    m_MoveState = reference.m_MoveState;
    m_ProneState = reference.m_ProneState;

    for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
        m_Paths[FGROUND][i].Create(reference.m_Paths[FGROUND][i]);
        m_Paths[BGROUND][i].Create(reference.m_Paths[BGROUND][i]);
        m_RotAngleTargets[i] = reference.m_RotAngleTargets[i];
    }

    m_GoldInInventoryChunk = reference.m_GoldInInventoryChunk;

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
    } else if (propName == "JumpTime") {
        reader >> m_JetTimeTotal;
        // Convert to ms
        m_JetTimeTotal *= 1000;
	} else if (propName == "JumpAngleRange") {
		reader >> m_JetAngleRange;
	} else if (propName == "FGArmFlailScalar") {
		reader >> m_FGArmFlailScalar;
	} else if (propName == "BGArmFlailScalar") {
		reader >> m_BGArmFlailScalar;
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
	writer.NewProperty("JumpAngleRange");
	writer << m_JetAngleRange;
	writer.NewProperty("FGArmFlailScalar");
	writer << m_FGArmFlailScalar;
	writer.NewProperty("BGArmFlailScalar");
	writer << m_BGArmFlailScalar;
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
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldMO())
        totalValue += m_pFGArm->GetHeldMO()->GetTotalValue(nativeModule, foreignMult, nativeMult);

    return totalValue;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this carries a specifically named object in its inventory.
//                  Also looks through the inventories of potential passengers, as applicable.

bool AHuman::HasObject(string objectName) const
{
    bool found = Actor::HasObject(objectName);

    // If holding something, then check that too
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldMO())
        found = found || m_pFGArm->GetHeldMO()->HasObject(objectName);
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldMO())
        found = found || m_pBGArm->GetHeldMO()->HasObject(objectName);

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
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->GetHeldMO())
        found = found || m_pFGArm->GetHeldMO()->HasObjectInGroup(groupName);
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->GetHeldMO())
        found = found || m_pBGArm->GetHeldMO()->HasObjectInGroup(groupName);

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

        if (m_pHead->HasNoSetDamageMultiplier()) { m_pHead->SetDamageMultiplier(5.0F); }
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

BITMAP *AHuman::GetHeadBitmap() const {
    return (m_pHead && m_pHead->IsAttached()) ? m_pHead->GetSpriteFrame(0) : nullptr;
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChunkGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts an appropriate amount of gold tracked by Actor, and puts it
//                  in a MovableObject which is put into inventory.

void AHuman::ChunkGold()
{
    MovableObject *pGoldMO = 0;
    if (m_GoldCarried >= 24) {
        pGoldMO = dynamic_cast<MovableObject *>(
            g_PresetMan.GetEntityPreset("MOSParticle", "24 oz Gold Brick")->Clone());
        m_Inventory.push_front(pGoldMO);
        m_GoldCarried -= 24;
        m_GoldInInventoryChunk = 24;
    }
    else if (m_GoldCarried >= 10) {
        pGoldMO = dynamic_cast<MovableObject *>(
//            g_PresetMan.GetEntityPreset("MOSRotating", "10 Gold Brick")->Clone());
            g_PresetMan.GetEntityPreset("MOSParticle", "10 oz Gold Brick")->Clone());
        m_Inventory.push_front(pGoldMO);
        m_GoldCarried -= 10;
        m_GoldInInventoryChunk = 10;
    }
/*
    else if (m_GoldCarried >= 1) {
        pGoldMO = dynamic_cast<MovableObject *>(
            g_PresetMan.GetEntityPreset("MOPixel", "Gold Particle")->Clone());
        m_Inventory.push_front(pGoldMO);
        m_GoldCarried -= 1;
        m_GoldInInventoryChunk = 1;
    }
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

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.

bool AHuman::AddPieMenuSlices(PieMenuGUI *pPieMenu)
{
	if (m_pItemInReach) {
		PieSlice pickUpSlice(m_pFGArm ? "Pick Up " + m_pItemInReach->GetPresetName() : "NO ARM!", PieSlice::PieSliceIndex::PSI_PICKUP, PieSlice::SliceDirection::UP, m_pFGArm && m_Status != INACTIVE);
		pPieMenu->AddSlice(pickUpSlice);
	} else {
		PieSlice reloadSlice(GetEquippedItem() || GetEquippedBGItem() ? "Reload" : (m_pFGArm ? "Not holding anything!" : "NO ARM!"), PieSlice::PieSliceIndex::PSI_RELOAD, PieSlice::SliceDirection::UP, ((m_pFGArm && m_pFGArm->GetHeldDevice() && !m_pFGArm->GetHeldDevice()->IsFull()) || (m_pBGArm && m_pBGArm->GetHeldDevice() && !m_pBGArm->GetHeldDevice()->IsFull())) && m_Status != INACTIVE);
		pPieMenu->AddSlice(reloadSlice);
	}
	//To-do: don't oneline this?
	PieSlice dropSlice(m_pFGArm && m_pFGArm->GetHeldMO() ? "Drop " + m_pFGArm->GetHeldMO()->GetPresetName() : m_pBGArm ? (m_pBGArm->GetHeldMO() ? "Drop " + m_pBGArm->GetHeldMO()->GetPresetName() : (!m_Inventory.empty() ? "Drop Inventory" : "Not holding anything!")) : (m_pFGArm ? "Not holding anything!" : "NO ARM!"), PieSlice::PieSliceIndex::PSI_DROP, PieSlice::SliceDirection::DOWN, ((m_pFGArm && m_pFGArm->GetHeldMO()) || (m_pBGArm && (m_pBGArm->GetHeldMO() || !m_Inventory.empty()))) && m_Status != INACTIVE);
	pPieMenu->AddSlice(dropSlice);

	PieSlice nextItemSlice(m_pFGArm ? "Next Item" : "NO ARM!", PieSlice::PieSliceIndex::PSI_NEXTITEM, PieSlice::SliceDirection::RIGHT, m_pFGArm && (!m_Inventory.empty() || GetEquippedBGItem()) && m_Status != INACTIVE);
	pPieMenu->AddSlice(nextItemSlice);
	PieSlice prevItemSlice(m_pFGArm ? "Prev Item" : "NO ARM!", PieSlice::PieSliceIndex::PSI_PREVITEM, PieSlice::SliceDirection::LEFT, m_pFGArm && (!m_Inventory.empty() || GetEquippedBGItem()) && m_Status != INACTIVE);
	pPieMenu->AddSlice(prevItemSlice);

	PieSlice sentryAISlice("Sentry AI Mode", PieSlice::PieSliceIndex::PSI_SENTRY, PieSlice::SliceDirection::DOWN);
    pPieMenu->AddSlice(sentryAISlice);
    PieSlice patrolAISlice("Patrol AI Mode", PieSlice::PieSliceIndex::PSI_PATROL, PieSlice::SliceDirection::DOWN);
	pPieMenu->AddSlice(patrolAISlice);
	PieSlice formSquadSlice("Form Squad", PieSlice::PieSliceIndex::PSI_FORMSQUAD, PieSlice::SliceDirection::UP);
    pPieMenu->AddSlice(formSquadSlice);
	
	PieSlice goToSlice("Go-To AI Mode", PieSlice::PieSliceIndex::PSI_GOTO, PieSlice::SliceDirection::DOWN);
    pPieMenu->AddSlice(goToSlice);
    PieSlice goldAISlice("Gold Dig AI Mode", PieSlice::PieSliceIndex::PSI_GOLDDIG, PieSlice::SliceDirection::DOWN);
	pPieMenu->AddSlice(goldAISlice);

    PieSlice huntAISlice("Brain Hunt AI Mode", PieSlice::PieSliceIndex::PSI_BRAINHUNT, PieSlice::SliceDirection::RIGHT);
	pPieMenu->AddSlice(huntAISlice);

    PieSlice fullInventorySlice("Show Full Inventory", PieSlice::PieSliceIndex::PSI_FULLINVENTORY, PieSlice::SliceDirection::RIGHT, GetEquippedItem() || GetEquippedBGItem() || !m_Inventory.empty());
    pPieMenu->AddSlice(fullInventorySlice);

    // Add any custom slices from a currently held device
	if (m_Status != INACTIVE) {
		// Add any custom added slices after we've added the hardcoded ones, so they are lower priorty and don't hog the cardinal axes
		Actor::AddPieMenuSlices(pPieMenu);

		// Add any custom slices from a currently held device
		if (m_pFGArm && m_pFGArm->HoldsDevice()) { m_pFGArm->GetHeldDevice()->AddPieMenuSlices(pPieMenu); }
	} else {
		m_PieNeedsUpdate = false;
	}
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HandlePieCommand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles and does whatever a specific activated Pie Menu slice does to
//                  this.

bool AHuman::HandlePieCommand(PieSlice::PieSliceIndex pieSliceIndex)
{
    if (pieSliceIndex != PieSlice::PieSliceIndex::PSI_NONE)
    {
        if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_PICKUP)
            m_Controller.SetState(WEAPON_PICKUP);
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_DROP)
            m_Controller.SetState(WEAPON_DROP);
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_RELOAD)
            m_Controller.SetState(WEAPON_RELOAD);
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_NEXTITEM)
            m_Controller.SetState(WEAPON_CHANGE_NEXT, true);
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_PREVITEM)
            m_Controller.SetState(WEAPON_CHANGE_PREV, true);
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_SENTRY)
            m_AIMode = AIMODE_SENTRY;
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_PATROL)
            m_AIMode = AIMODE_PATROL;
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_BRAINHUNT)
        {
            m_AIMode = AIMODE_BRAINHUNT;
            // Clear out the waypoints; player will set new ones with UI in gameactivity
            ClearAIWaypoints();
        }
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_GOTO)
        {
            m_AIMode = AIMODE_GOTO;
            // Clear out the waypoints; player will set new ones with UI in gameactivity
            ClearAIWaypoints();
            m_UpdateMovePath = true;
        }
        else if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_GOLDDIG)
            m_AIMode = AIMODE_GOLDDIG;
        else
            return Actor::HandlePieCommand(pieSliceIndex);
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddInventoryItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an inventory item to this AHuman. This also puts that item
//                  directly in the hands of this if they are empty.

void AHuman::AddInventoryItem(MovableObject *pItemToAdd)
{
    // If we have nothing in inventory, and nothing in our hands, just grab this first things added to us
    if (pItemToAdd && m_Inventory.empty() && m_pFGArm && m_pFGArm->IsAttached() && !m_pFGArm->GetHeldMO())
    {
        m_pFGArm->SetHeldMO(pItemToAdd);
        m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
    }
    else
        Actor::AddInventoryItem(pItemToAdd);

    // Equip shield in BG arm is applicable
    EquipShieldInBGArm();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipFirearm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found firearm
//                  in the inventory. If the held device already is a firearm, or no
//                  firearm is in inventory, nothing happens.

bool AHuman::EquipFirearm(bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HDFirearm *pWeapon = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pWeapon && pWeapon->IsWeapon())
            return true;
	} else {
		UnequipBGArm();
	}

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pWeapon = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pWeapon && pWeapon->IsWeapon())
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pWeapon);
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

bool AHuman::EquipDeviceInGroup(string group, bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HeldDevice *pDevice = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pDevice = dynamic_cast<HeldDevice *>(m_pFGArm->GetHeldMO());
        if (pDevice && pDevice->IsInGroup(group))
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pDevice = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pDevice && pDevice->IsInGroup(group))
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    MovableObject *previouslyHeldItem = m_pFGArm->ReleaseHeldMO();
                    // Note - This is a fix to deal with an edge case bug when this method is called by a global script.
                    // Because the global script runs before everything has finished traveling, the removed item needs to undraw itself from the MO layer, otherwise it can result in ghost collisions and crashes.
                    if (previouslyHeldItem->GetsHitByMOs()) { previouslyHeldItem->Draw(g_SceneMan.GetMOIDBitmap(), Vector(), g_DrawNoMOID, true); }
                    m_Inventory.emplace_back(previouslyHeldItem);
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pDevice);
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

bool AHuman::EquipLoadedFirearmInGroup(string group, string excludeGroup, bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HDFirearm *pFirearm = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pFirearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pFirearm && !pFirearm->NeedsReloading() && pFirearm->IsInGroup(group) && !pFirearm->IsInGroup(excludeGroup))
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pFirearm = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pFirearm && !pFirearm->NeedsReloading() && pFirearm->IsInGroup(group) && !pFirearm->IsInGroup(excludeGroup))
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pFirearm);
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

bool AHuman::EquipNamedDevice(const string name, bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HeldDevice *pDevice = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pDevice = dynamic_cast<HeldDevice *>(m_pFGArm->GetHeldMO());
        if (pDevice && pDevice->GetPresetName() == name)
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pDevice = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pDevice && pDevice->GetPresetName() == name)
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pDevice);
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
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    ThrownDevice *pThrown = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pThrown = dynamic_cast<ThrownDevice *>(m_pFGArm->GetHeldMO());
// TODO: see if thrown is weapon or not, don't want to throw key items etc
        if (pThrown)// && pThrown->IsWeapon())
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pThrown = dynamic_cast<ThrownDevice *>(*itr);
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
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pThrown);
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
// Virtual Method:  EquipDiggingTool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found digging
//                  tool in the inventory. If the held device already is a digging tool,
//                  or no digging tool is in inventory, nothing happens.

bool AHuman::EquipDiggingTool(bool doEquip)
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HDFirearm *pTool = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pTool = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pTool && pTool->IsInGroup("Tools - Diggers"))
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pTool = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pTool && pTool->IsInGroup("Tools - Diggers"))
        {
            if (doEquip)
            {
                // Erase the inventory entry containing the device we now have switched to
                *itr = 0;
                m_Inventory.erase(itr);

                // Put back into the inventory what we had in our hands, if anything
                if (m_pFGArm->HoldsSomething())
                {
                    m_pFGArm->GetHeldDevice()->Deactivate();
                    m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
                }

                // Now put the device we were looking for and found into the hand
                m_pFGArm->SetHeldMO(pTool);
                // Move the hand to a poisition so it looks like the new device was drawn from inventory
                m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

                // Equip shield in BG arm is applicable
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
// Method:          EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength this actor can move through.

float AHuman::EstimateDigStrenght()
{
    float maxPenetration = 1;
    
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return maxPenetration;
    
    HDFirearm *pTool = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pTool = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pTool && pTool->IsInGroup("Tools - Diggers"))
            return pTool->EstimateDigStrenght();
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pTool = dynamic_cast<HDFirearm *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pTool && pTool->IsInGroup("Tools - Diggers"))
            maxPenetration = max(pTool->EstimateDigStrenght(), maxPenetration);
    }
    
    return maxPenetration;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EquipShield
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches the currently held device (if any) to the first found shield
//                  in the inventory. If the held device already is a shield, or no
//                  shield is in inventory, nothing happens.

bool AHuman::EquipShield()
{
    if (!(m_pFGArm && m_pFGArm->IsAttached()))
        return false;

    HeldDevice *pShield = 0;

    // Check if the currently held device is already the desired type
    if (m_pFGArm->HoldsSomething())
    {
        pShield = dynamic_cast<HeldDevice *>(m_pFGArm->GetHeldMO());
        if (pShield && pShield->IsShield())
            return true;
    }

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pShield = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pShield && pShield->IsShield())
        {
            // Erase the inventory entry containing the device we now have switched to
            *itr = 0;
            m_Inventory.erase(itr);

            // Put back into the inventory what we had in our hands, if anything
            if (m_pFGArm->HoldsSomething())
            {
                m_pFGArm->GetHeldDevice()->Deactivate();
                m_Inventory.push_back(m_pFGArm->ReleaseHeldMO());
            }

            // Now put the device we were looking for and found into the hand
            m_pFGArm->SetHeldMO(pShield);
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
    if (!(m_pBGArm && m_pBGArm->IsAttached()))
        return false;

    HeldDevice *pShield = 0;

    // Check if the currently held device is already the desired type
    if (m_pBGArm->HoldsSomething())
    {
        pShield = dynamic_cast<HeldDevice *>(m_pBGArm->GetHeldMO());
		if (pShield && (pShield->IsShield() || pShield->IsDualWieldable()))
        {
            // If we're holding a shield, but aren't supposed to, because we need to support the FG hand's two-handed device,
            // then let go of the shield and put it back in inventory
            if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded())
            {
                m_pBGArm->GetHeldDevice()->Deactivate();
                m_Inventory.push_back(m_pBGArm->ReleaseHeldMO());
                return false;
            }
            return true;
        }
    }

    // Only equip if the BG hand isn't occupied with supporting a two handed device
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice() && !m_pFGArm->GetHeldDevice()->IsOneHanded())
        return false;

    // Go through the inventory looking for the proper device
    for (deque<MovableObject *>::iterator itr = m_Inventory.begin(); itr != m_Inventory.end(); ++itr)
    {
        pShield = dynamic_cast<HeldDevice *>(*itr);
        // Found proper device to equip, so make the switch!
        if (pShield && (pShield->IsShield() || pShield->IsDualWieldable()))
        {
            // Erase the inventory entry containing the device we now have switched to
            *itr = 0;
            m_Inventory.erase(itr);

            // Put back into the inventory what we had in our hands, if anything
            if (m_pBGArm->HoldsSomething())
            {
                m_pBGArm->GetHeldDevice()->Deactivate();
                m_Inventory.push_back(m_pBGArm->ReleaseHeldMO());
            }

            // Now put the device we were looking for and found into the hand
            m_pBGArm->SetHeldMO(pShield);
            // Move the hand to a poisition so it looks like the new device was drawn from inventory
            m_pBGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

            // Play the device switching sound only if activity is running
			if (g_ActivityMan.ActivityRunning())
			{
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
			}

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  UnequipBGArm
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unequips whatever is in the BG arm and puts it into the inventory.
// Arguments:       None.
// Return value:    Whether there was anything to unequip.

bool AHuman::UnequipBGArm()
{
    if (!(m_pBGArm && m_pBGArm->IsAttached()))
        return false;

    // Put back into the inventory what we had in our hand, if anything
    if (m_pBGArm->HoldsSomething())
    {
        m_pBGArm->GetHeldDevice()->Deactivate();
        m_Inventory.push_back(m_pBGArm->ReleaseHeldMO());
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the FG Arm, if anything. OWNERSHIP IS NOT TRANSFERRED!

MovableObject * AHuman::GetEquippedItem() const
{
    // Check if the currently held device is already the desired type
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsSomething())
    {
        return m_pFGArm->GetHeldMO();
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedBGItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the FG Arm, if anything. OWNERSHIP IS NOT TRANSFERRED!

MovableObject * AHuman::GetEquippedBGItem() const
{
	// Check if the currently held device is already the desired type
	if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->HoldsSomething())
	{
		return m_pBGArm->GetHeldMO();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held device's current mag is empty on
//                  ammo or not.

bool AHuman::FirearmIsReady() const
{
    // Check if the currently held device is already the desired type
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsSomething())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
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
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsSomething())
    {
        const ThrownDevice *pThrown = dynamic_cast<ThrownDevice *>(m_pFGArm->GetHeldMO());
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
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pWeapon && pWeapon->GetRoundInMagCount() == 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.

bool AHuman::FirearmNeedsReload() const
{
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
        if (pWeapon && pWeapon->NeedsReloading())
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.

bool AHuman::FirearmIsSemiAuto() const
{
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
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

void AHuman::ReloadFirearms() const
{
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
		if (pWeapon) {
			pWeapon->Reload();
		}
    }
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->HoldsHeldDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pBGArm->GetHeldMO());
        if (pWeapon) {
            pWeapon->Reload();
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
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsSomething())
    {
        const HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
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
    float distance = diff.GetMagnitude();

    // Really close!
    if (distance <= m_CharHeight)
        return true;

    float range = 0;

    if (FirearmIsReady())
    {
        // Start with the default aim distance
        range = m_AimDistance;

        // Add the sharp range of the equipped weapon
        if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
            range += m_pFGArm->GetHeldDevice()->GetSharpLength() + 150;
    }
    else if (ThrowableIsReady())
    {
// TODO: make proper throw range calc based on the throwable's mass etc
        range += m_CharHeight * 4;
    }

    return distance <= range;
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
    if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
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
    if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
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
    // Estimate how much material this actor can dig through
    m_DigStrength = EstimateDigStrenght();
    
    // Do the real path calc; abort and pass along the message if it didn't happen due to throttling
    if (!Actor::UpdateMovePath())
        return false;

    // Process the new path we now have, if any
    if (!m_MovePath.empty())
    {
        // Smash all airborne waypoints down to just above the ground, except for when it makes the path intersect terrain or it is the final destination
        list<Vector>::iterator finalItr = m_MovePath.end();
        finalItr--;
        Vector smashedPoint;
        Vector previousPoint = *(m_MovePath.begin());
        list<Vector>::iterator nextItr = m_MovePath.begin();
        for (list<Vector>::iterator lItr = m_MovePath.begin(); lItr != finalItr; ++lItr)
        {
            nextItr++;
            smashedPoint = g_SceneMan.MovePointToGround((*lItr), m_CharHeight*0.2, 7);

            Vector notUsed;

            // Only smash if the new location doesn't cause the path to intersect hard terrain ahead or behind of it
            // Try three times to halve the height to see if that won't intersect
            for (int i = 0; i < 3; i++)
            {
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

	const list<AlarmEvent> &events = g_MovableMan.GetAlarmEvents();
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
            // Stop and then turn around after a period of time, or if bumped into another actor (like a rocket)
            if (m_PatrolTimer.IsPastSimMS(8000) ||
                /*g_SceneMan.CastNotMaterialRay(m_Pos, Vector(m_CharHeight / 4, 0), g_MaterialAir, Vector(), 4, false)*/
                g_SceneMan.CastMORay(m_Pos, Vector((m_LateralMoveState == LAT_RIGHT ? m_CharHeight : -m_CharHeight) / 3, 0), m_MOID, IgnoresWhichTeam(), g_MaterialGrass, false, 4) != g_NoMOID)
            {
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
            for (list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end();)
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
            // If not following an MO, stay still and switch to sentry mode if we're close enough to final static destination
            if (!m_pMOMoveTarget && m_Waypoints.empty() && m_MovePath.empty() && fabs(m_MoveVector.m_X) <= 10)
            {
                // DONE MOVING TOWARD TARGET
                m_LateralMoveState = LAT_STILL;
                m_AIMode = AIMODE_SENTRY;
                m_DeviceState = SCANNING;
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
                if (newGoldDir.GetMagnitude() < oldGoldDir.GetMagnitude() || g_SceneMan.GetTerrain()->GetMaterialPixel(m_DigTarget.m_X, m_DigTarget.m_Y) != g_MaterialGold)
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
            if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
            {
                // Don't press the trigger too fast in succession
                if (m_FireTimer.IsPastSimMS(250))
                {
                    // Get the distance to the target and see if we've aimed well enough
                    Vector targetVec = g_SceneMan.ShortestDistance(m_Pos, m_SeenTargetPos, false);
                    float distance = targetVec.GetMagnitude();
                    // Fire if really close, or if we have aimed well enough
                    if (distance < m_AimDistance * 6 + m_pFGArm->GetHeldDevice()->GetSharpLength() * m_SharpAimProgress)
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
            list<Vector>::iterator pItr = m_MovePath.begin();
            list<Vector>::iterator prevItr = m_MovePath.begin();
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
                    list<Vector>::iterator pRemItr = m_MovePath.begin();
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
                topHeadPos.m_Y += m_pHead->GetParentOffset().m_Y - m_pHead->GetJointOffset().m_Y + m_pHead->GetSpriteOffset().m_Y - 6;
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
        if (m_RecentMovementMag > 2.5 || m_TeamBlockState)
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
        if (m_RecentMovementMag > 2.5)
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
    else if (m_ObstacleState == BACKSTEPPING && (m_StuckTimer.IsPastSimMS(2000) || m_RecentMovementMag > 15.0))
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AHuman::OnPieMenu(Actor *pieMenuActor) {
	int status = Actor::OnPieMenu(pieMenuActor);

    // Note: This is a bit ugly, but it should make this function output different error statuses based on whether the AHuman's OnPieMenuFunction fails, or its weapons' do, though the specifics can't be sussed out by the error alone.
    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsDevice()) {
        status += m_pFGArm->GetHeldDevice()->OnPieMenu(pieMenuActor);
    }
    if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->HoldsDevice()) {
        status += m_pBGArm->GetHeldDevice()->OnPieMenu(pieMenuActor);
    }

	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this AHuman. Supposed to be done every frame.

void AHuman::Update()
{
	float deltaTime = g_TimerMan.GetDeltaTimeSecs();
	// Get the rotation in radians.
	float rot = m_Rotation.GetRadAngle();

	// Set Default direction of all the paths!
	m_Paths[FGROUND][WALK].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][WALK].SetHFlip(m_HFlipped);
	m_Paths[FGROUND][CROUCH].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][CROUCH].SetHFlip(m_HFlipped);
	m_Paths[FGROUND][CRAWL].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][CRAWL].SetHFlip(m_HFlipped);
	m_Paths[FGROUND][ARMCRAWL].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][ARMCRAWL].SetHFlip(m_HFlipped);
	m_Paths[FGROUND][CLIMB].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][CLIMB].SetHFlip(m_HFlipped);
	m_Paths[FGROUND][STAND].SetHFlip(m_HFlipped);
	m_Paths[BGROUND][STAND].SetHFlip(m_HFlipped);

	////////////////////////////////////
	// Jetpack activation and blast direction

	if (m_pJetpack)
	{
		if (m_JetTimeTotal > 0) {
			// Jetpack throttle depletes relative to jet time, but only if throttle range values have been defined
			float jetTimeRatio = std::max(m_JetTimeLeft / m_JetTimeTotal, 0.0F);
			m_pJetpack->SetThrottle(jetTimeRatio * 2.0F - 1.0F);
			float minScale = 1.0F - m_pJetpack->GetMinThrottle();
			m_pJetpack->SetFlashScale(minScale + (1.0F + m_pJetpack->GetMaxThrottle() - minScale) * jetTimeRatio);
		}
		// Start Jetpack burn
		if (m_Controller.IsState(BODY_JUMPSTART) && m_JetTimeLeft > 0 && m_Status != INACTIVE)
		{
			m_pJetpack->TriggerBurst();
			// This is to make sure se get loose from being stuck
			m_ForceDeepCheck = true;
			m_pJetpack->EnableEmission(true);
			// Quadruple this for the burst
			m_JetTimeLeft = std::max(m_JetTimeLeft - g_TimerMan.GetDeltaTimeMS() * 10.0F, 0.0F);
		}
		// Jetpack is ordered to be burning, or the pie menu is on and was burning before it went on
		else if ((m_Controller.IsState(BODY_JUMP) || (m_MoveState == JUMP && m_Controller.IsState(PIE_MENU_ACTIVE))) && m_JetTimeLeft > 0)
		{
			m_pJetpack->EnableEmission(true);
			// Jetpacks are noisy!
			m_pJetpack->AlarmOnEmit(m_Team);
			// Deduct from the jetpack time
			m_JetTimeLeft = std::max(m_JetTimeLeft - g_TimerMan.GetDeltaTimeMS(), 0.0F);
			m_MoveState = JUMP;
			m_Paths[FGROUND][JUMP].Restart();
			m_Paths[BGROUND][JUMP].Restart();
		}
		// Jetpack is off/turning off
		else {
			m_pJetpack->EnableEmission(false);
			if (m_MoveState == JUMP) { m_MoveState = STAND; }
			// Replenish the jetpack time, twice as fast
			m_JetTimeLeft = std::min(m_JetTimeLeft + g_TimerMan.GetDeltaTimeMS() * 2.0F, m_JetTimeTotal);
		}

		// If pie menu is on, keep the angle to what it was before
		if (m_Controller.IsState(PIE_MENU_ACTIVE))
		{
			// Don't change anything
		}
		// Direct the jetpack nozzle according to movement stick if analog input is present
		else if (m_Controller.GetAnalogMove().GetMagnitude() > 0.1F)
		{
			//To-do: test whether this works properly
			float jetAngle = (m_Controller.GetAnalogMove().GetAbsRadAngle() - c_PI);
			if (jetAngle > c_PI) { jetAngle -= c_TwoPI; }
			m_pJetpack->SetEmitAngle(FacingAngle(jetAngle * m_JetAngleRange));
		}
		// Or just use the aim angle if we're getting digital input
		else {
			// Halve the jet angle when looking downwards so the actor isn't forced to go sideways (To-do: don't hardcode this value?)
			float jetAngle = m_AimAngle > 0 ? m_AimAngle * m_JetAngleRange : -m_AimAngle * m_JetAngleRange * 0.5F;
			jetAngle -= (c_HalfPI * m_JetAngleRange + c_HalfPI);
			// Don't need to use FacingAngle on this because it's already applied to the AimAngle since last update.
			m_pJetpack->SetEmitAngle(jetAngle);
		}
	}

	////////////////////////////////////
	// Movement direction

	// If the pie menu is on, try to preserve whatever move state we had before it going into effect
	if (m_Controller.IsState(PIE_MENU_ACTIVE))
	{
		// Just keep the previous movestate, don't stand up or stop walking or stop jumping
	}
	else if (m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT) || m_MoveState == JUMP && m_Status != INACTIVE)
	{
		// Only if not jumping, OR if jumping, and apparently stuck on something - then help out with the limbs
		if (m_MoveState != JUMP || (m_MoveState == JUMP && m_Vel.GetLargest() < 0.1))
		{
			// Restart the stride if we're just starting to walk or crawl
			if ((m_MoveState != WALK && !m_Controller.IsState(BODY_CROUCH)) ||
				(m_MoveState != CRAWL && m_Controller.IsState(BODY_CROUCH)))
			{
				m_StrideStart = true;
				MoveOutOfTerrain(g_MaterialGrass);
			}

			// Crawling or walking?
			m_MoveState = m_Controller.IsState(BODY_CROUCH) ? CRAWL : WALK;

			// Engage prone state, this makes the body's rotational spring pull it horizontal instead of upright
			if (m_MoveState == CRAWL && m_ProneState == NOTPRONE)
			{
				m_ProneState = GOPRONE;
				m_ProneTimer.Reset();
			}

			m_Paths[FGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
			m_Paths[BGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
		}

		// Walk backwards if the aiming is done in the opposite direction of travel
		if (std::abs(m_Controller.GetAnalogAim().m_X) > 0.1F)
		{
			// Walk backwards if necessary
			m_Paths[FGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
			m_Paths[BGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
		}
		// Flip if we're moving in the opposite direction
		else if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped))
		{
			m_HFlipped = !m_HFlipped;
			//                // Instead of simply carving out a silhouette of the now flipped actor, isntead disable any atoms which are embedded int eh terrain until they emerge again
			//                m_ForceDeepCheck = true;
			m_CheckTerrIntersection = true;
			if (m_ProneState == NOTPRONE) { MoveOutOfTerrain(g_MaterialGrass); }
			m_Paths[FGROUND][m_MoveState].SetHFlip(m_HFlipped);
			m_Paths[BGROUND][m_MoveState].SetHFlip(m_HFlipped);

			m_Paths[FGROUND][WALK].Terminate();
			m_Paths[BGROUND][WALK].Terminate();
			m_Paths[FGROUND][CROUCH].Terminate();
			m_Paths[BGROUND][CROUCH].Terminate();
			m_Paths[FGROUND][CLIMB].Terminate();
			m_Paths[BGROUND][CLIMB].Terminate();
			m_Paths[FGROUND][CRAWL].Terminate();
			m_Paths[BGROUND][CRAWL].Terminate();
			m_Paths[FGROUND][ARMCRAWL].Terminate();
			m_Paths[BGROUND][ARMCRAWL].Terminate();
			m_Paths[FGROUND][STAND].Terminate();
			m_Paths[BGROUND][STAND].Terminate();
			m_StrideStart = true;
			// Stop the going prone spring
			if (m_ProneState == GOPRONE) { m_ProneState = PRONE; }
		}
	// Not moving, so check if we need to be crouched or not
	} else if (m_Controller.IsState(BODY_CROUCH)) {
		// Don't go back to crouching if we're already prone, player has to let go of the crouch button first
		// If already laying down, just don't do anything and keep laying there
		m_MoveState = m_ProneState == NOTPRONE ? CROUCH : NOMOVE;
	} else {
		m_MoveState = STAND;
	}

    // Disengage the prone state as soon as the crouch is released when the pie menu isn't active
    if (!m_Controller.IsState(BODY_CROUCH) && !m_Controller.IsState(PIE_MENU_ACTIVE))
        m_ProneState = NOTPRONE;

    ////////////////////////////////////
    // Change and reload held MovableObjects

	bool reloadFG = false;
	if (m_pFGArm && m_Status != INACTIVE) {
		HDFirearm * pFireArm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldMO());
		if (m_Controller.IsState(WEAPON_CHANGE_NEXT)) {
			if (!m_Inventory.empty() || GetEquippedBGItem()) {
				if (pFireArm) { pFireArm->StopActivationSound(); }
				if (m_Inventory.empty()) { UnequipBGArm(); }

				m_pFGArm->SetHeldMO(SwapNextInventory(m_pFGArm->ReleaseHeldMO()));
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
				m_PieNeedsUpdate = true;

				EquipShieldInBGArm();
			}
		}
		if (m_Controller.IsState(WEAPON_CHANGE_PREV)) {
			if (!m_Inventory.empty() || GetEquippedBGItem()) {
				if (pFireArm) { pFireArm->StopActivationSound(); }
				if (m_Inventory.empty()) { UnequipBGArm(); }

				m_pFGArm->SetHeldMO(SwapPrevInventory(m_pFGArm->ReleaseHeldMO()));
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
				m_PieNeedsUpdate = true;

				EquipShieldInBGArm();
			}
        }

		// Reload held MO, if applicable
		if (pFireArm) {
			if (pFireArm->IsReloading()) {
				// Interrupt sharp aiming while reloading
				m_SharpAimTimer.Reset();
				m_SharpAimProgress = 0;
				// Move BG hand accordingly
				if (m_pBGArm && m_pBGArm->IsAttached() && !GetEquippedBGItem()) { m_pBGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped)); }
			}
			if (!pFireArm->IsFull() && m_Controller.IsState(WEAPON_RELOAD)) {
				if (m_pBGArm && m_pBGArm->IsAttached() && !GetEquippedBGItem()) { m_pBGArm->SetHandPos(pFireArm->GetMagazinePos()); }
				pFireArm->Reload();
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
				reloadFG = true;

				// Interrupt sharp aiming
				m_SharpAimTimer.Reset();
				m_SharpAimProgress = 0;
			}
			// Detect reloading being completed and move hand accordingly
			if (pFireArm->DoneReloading() && m_pBGArm && m_pBGArm->IsAttached() && !GetEquippedBGItem()) { m_pBGArm->SetHandPos(pFireArm->GetMagazinePos()); }
		}
    }

    ////////////////////////////////////
    // Aiming

    if (m_Controller.IsState(AIM_UP) && m_Status != INACTIVE)
    {
// TODO: Improve these!")
        // Set the timer to some base number so we don't
        // get a sluggish feeling at start of aim
        if (m_AimState != AIMUP)
            m_AimTmr.SetElapsedSimTimeMS(150);
        m_AimState = AIMUP; 
        m_AimAngle += m_Controller.IsState(AIM_SHARP) ?
                      std::min(m_AimTmr.GetElapsedSimTimeMS() * 0.00005, 0.05) :
                      std::min(m_AimTmr.GetElapsedSimTimeMS() * 0.00015, 0.1);
        if (m_AimAngle > m_AimRange)
            m_AimAngle = m_AimRange;
    }
    else if (m_Controller.IsState(AIM_DOWN) && m_Status != INACTIVE)
    {
        // Set the timer to some base number so we don't
        // get a sluggish feeling at start of aim
        if (m_AimState != AIMDOWN)
            m_AimTmr.SetElapsedSimTimeMS(150);
        m_AimState = AIMDOWN;
        m_AimAngle -= m_Controller.IsState(AIM_SHARP) ?
                      std::min(m_AimTmr.GetElapsedSimTimeMS() * 0.00005, 0.05) :
                      std::min(m_AimTmr.GetElapsedSimTimeMS() * 0.00015, 0.1);
        if (m_AimAngle < -m_AimRange)
            m_AimAngle = -m_AimRange;
    }
    // Analog aim
    else if (m_Controller.GetAnalogAim().GetMagnitude() > 0.1 && m_Status != INACTIVE)
    {
        Vector aim = m_Controller.GetAnalogAim();
        // Hack to avoid the GetAbsRadAngle to mangle an aim angle straight down
        if (aim.m_X == 0)
            aim.m_X += m_HFlipped ? -0.01 : 0.01;
        m_AimAngle = aim.GetAbsRadAngle();

        // Check for flip change
        if ((aim.m_X > 0 && m_HFlipped) || (aim.m_X < 0 && !m_HFlipped))
        {
            m_HFlipped = !m_HFlipped;
//                // Instead of simply carving out a silhouette of the now flipped actor, isntead disable any atoms which are embedded int eh terrain until they emerge again
//                m_ForceDeepCheck = true;
            m_CheckTerrIntersection = true;
            if (m_ProneState == NOTPRONE)
                MoveOutOfTerrain(g_MaterialGrass);
            m_Paths[FGROUND][WALK].Terminate();
            m_Paths[BGROUND][WALK].Terminate();
            m_Paths[FGROUND][CLIMB].Terminate();
            m_Paths[BGROUND][CLIMB].Terminate();
            m_Paths[FGROUND][CRAWL].Terminate();
            m_Paths[BGROUND][CRAWL].Terminate();
            m_Paths[FGROUND][ARMCRAWL].Terminate();
            m_Paths[BGROUND][ARMCRAWL].Terminate();
            m_Paths[FGROUND][STAND].Terminate();
            m_Paths[BGROUND][STAND].Terminate();
            m_StrideStart = true;
            // Stop the going prone spring
            if (m_ProneState == GOPRONE)
                m_ProneState = PRONE;
        }
        // Correct angle based on flip
        m_AimAngle = FacingAngle(m_AimAngle);
        // Clamp so it's within the range
        Clamp(m_AimAngle, m_AimRange, -m_AimRange);
    }
    else
        m_AimState = AIMSTILL;

    //////////////////////////////
    // Sharp aim calculation

// TODO: make the delay data driven by both the actor and the device!
    // 
    if (m_Controller.IsState(AIM_SHARP) && (m_MoveState == STAND || m_MoveState == CROUCH || m_MoveState == NOMOVE || m_MoveState == WALK) && m_Vel.GetMagnitude() < 5.0F) {
/*
        float halfDelay = m_SharpAimDelay / 2;
        // Accelerate for first half
        if (!m_SharpAimTimer.IsPastSimMS(halfDelay))
            m_SharpAimProgress = (float)m_SharpAimTimer.GetElapsedSimTimeMS() / (float)m_SharpAimDelay;
        // Decelerate for second half
        else if (!m_SharpAimTimer.IsPastSimMS(m_SharpAimDelay)
            m_SharpAimProgress
        // At max
        else
            m_SharpAimProgress = 1.0;
*/
        float aimMag = m_Controller.GetAnalogAim().GetMagnitude();

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
    // Fire/Activate held devices

	ThrownDevice *pThrown = nullptr;
	if (m_pFGArm && m_pFGArm->IsAttached()) {
		// DOn't reach toward anything
		m_pFGArm->ReachToward(Vector());

		// Activate held device, if it's not a thrown device.
		if (m_pFGArm->HoldsHeldDevice() && !m_pFGArm->HoldsThrownDevice()) {
			m_pFGArm->GetHeldDevice()->SetSharpAim(m_SharpAimProgress);
			if (m_Controller.IsState(WEAPON_FIRE)) {
				m_pFGArm->GetHeldDevice()->Activate();
			} else {
				m_pFGArm->GetHeldDevice()->Deactivate();
			}
		}
		// Throw whatever is held if it's a thrown device
		else if (m_pFGArm->GetHeldMO()) {
			pThrown = dynamic_cast<ThrownDevice *>(m_pFGArm->GetHeldMO());
			if (pThrown) {
				if (m_Controller.IsState(WEAPON_FIRE)) {
					if (m_ArmsState != THROWING_PREP/* || m_ThrowTmr.GetElapsedSimTimeMS() > m_ThrowPrepTime*/) {
						m_ThrowTmr.Reset();
						if (!pThrown->ActivatesWhenReleased()) {
							pThrown->Activate();
						}
					}
					m_ArmsState = THROWING_PREP;
					m_pFGArm->ReachToward(m_Pos + pThrown->GetStartThrowOffset().GetXFlipped(m_HFlipped));
				} else if (m_ArmsState == THROWING_PREP) {
					m_ArmsState = THROWING_RELEASE;

					m_pFGArm->SetHandPos(m_Pos + pThrown->GetEndThrowOffset().GetXFlipped(m_HFlipped));

					MovableObject *pMO = m_pFGArm->ReleaseHeldMO();

					if (pThrown->ActivatesWhenReleased()) {
						pThrown->Activate();
					}
					if (pMO) {
						pMO->SetPos(m_Pos + m_pFGArm->GetParentOffset().GetXFlipped(m_HFlipped) + Vector(m_HFlipped ? -15 : 15, -8));
						float throwScalar = static_cast<float>(std::min(m_ThrowTmr.GetElapsedSimTimeMS(), static_cast<double>(m_ThrowPrepTime)) / m_ThrowPrepTime);
						Vector tossVec(pThrown->GetMinThrowVel() + ((pThrown->GetMaxThrowVel() - pThrown->GetMinThrowVel()) * throwScalar), 0.5F * RandomNormalNum());
						tossVec.RadRotate(m_AimAngle);
						pMO->SetVel(tossVec.GetXFlipped(m_HFlipped) * m_Rotation);
						pMO->SetAngularVel(5.0F * RandomNormalNum());

						if (pMO->IsHeldDevice()) {
							// Set the grenade or whatever to ignore hits with same team
							pMO->SetTeam(m_Team);
							pMO->SetIgnoresTeamHits(true);
							g_MovableMan.AddItem(pMO);
						} else {
							if (pMO->IsGold()) {
								m_GoldInInventoryChunk = 0;
								ChunkGold();
							}
							g_MovableMan.AddParticle(pMO);
						}
						pMO = 0;
					}
					m_ThrowTmr.Reset();
				}
			}
		} else if (m_ArmsState == THROWING_RELEASE && m_ThrowTmr.GetElapsedSimTimeMS() > 100) {
			m_pFGArm->SetHeldMO(SwapNextInventory());
			m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
			m_ArmsState = WEAPON_READY;
		} else if (m_ArmsState == THROWING_RELEASE) {
			m_pFGArm->SetHandPos(m_Pos + (m_HolsterOffset + Vector(15, -15)).GetXFlipped(m_HFlipped));
		}
	}

	if (m_ArmsState == THROWING_PREP && !pThrown) {
		m_ArmsState = WEAPON_READY;
	}

	if (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->HoldsHeldDevice()) {
		HeldDevice *pDevice = m_pBGArm->GetHeldDevice();

		if (pDevice->IsReloading()) {
			m_SharpAimTimer.Reset();
			m_SharpAimProgress = 0;
			if (m_pFGArm && m_pFGArm->IsAttached() && !GetEquippedItem()) { m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped)); }
		}
		if (reloadFG == false && !pDevice->IsFull() && m_Controller.IsState(WEAPON_RELOAD) && !m_pItemInReach) {
			if (m_pFGArm && m_pFGArm->IsAttached() && !GetEquippedItem()) { m_pFGArm->SetHandPos(pDevice->GetMagazinePos()); }
			pDevice->Reload();
			if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
		}
		if (pDevice->DoneReloading() && m_pFGArm && m_pFGArm->IsAttached() && !GetEquippedItem()) { m_pFGArm->SetHandPos(pDevice->GetMagazinePos()); }
		pDevice->SetSharpAim(m_SharpAimProgress);
		if (m_Controller.IsState(WEAPON_FIRE)) {
			pDevice->Activate();
		} else {
			pDevice->Deactivate();
		}
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
		if (m_pFGArm && m_pFGArm->HoldsSomething()) {
			if (MovableObject *pMO = m_pFGArm->ReleaseHeldMO()) {
				pMO->SetPos(m_Pos + Vector(m_HFlipped ? -10 : 10, -8));
				Vector tossVec(RandomNum(3.0F, 6.0F), RandomNum(-3.0F, -1.5F));
				tossVec.RadRotate(m_AimAngle);
				pMO->SetVel(m_Vel * 0.5F + tossVec.GetXFlipped(m_HFlipped) * m_Rotation);
				pMO->SetAngularVel(m_AngularVel * 0.5F + 3.0F * RandomNormalNum());
				if (pMO->IsDevice()) {
					g_MovableMan.AddItem(pMO);
				} else {
					if (pMO->IsGold()) {
						m_GoldInInventoryChunk = 0;
						ChunkGold();
					}
					g_MovableMan.AddParticle(pMO);
				}
			}
			if (!m_Inventory.empty()) {
				m_pFGArm->SetHeldMO(SwapNextInventory());
				m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
			}
		} else if (m_pBGArm) {
			if (MovableObject *pMO = m_pBGArm->ReleaseHeldMO()) {
				pMO->SetPos(m_Pos + Vector(m_HFlipped ? -10 : 10, -8));
				Vector tossVec(RandomNum(3.0F, 6.0F), RandomNum(-3.0F, -1.5F));
				tossVec.RadRotate(m_AimAngle);
				pMO->SetVel(m_Vel * 0.5F + tossVec.GetXFlipped(m_HFlipped) * m_Rotation);
				pMO->SetAngularVel(m_AngularVel * 0.5F + 3.0F * RandomNormalNum());
				if (pMO->IsDevice()) {
					g_MovableMan.AddItem(pMO);
				}
				else {
					if (pMO->IsGold()) {
						m_GoldInInventoryChunk = 0;
						ChunkGold();
					}
					g_MovableMan.AddParticle(pMO);
				}
			} else {
				DropAllInventory();
				m_pBGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));
			}
		}
		m_PieNeedsUpdate = true;
	}

    ////////////////////////////////////////
    // Item pickup logic

	float reach = m_SpriteRadius;
	Vector reachPoint = m_Pos;

	// Try to detect a new item
	if (m_pFGArm && m_Status == STABLE) {
		reach += m_pFGArm->GetMaxLength();
		reachPoint = m_pFGArm->GetPos();
		if (!m_pItemInReach) {
			MOID itemMOID = g_SceneMan.CastMORay(reachPoint, Vector((m_HFlipped ? -reach : reach) * RandomNum(), RandomNum(0.0F, reach)), m_MOID, Activity::NoTeam, g_MaterialGrass, true, 2);

			MovableObject *pItem = g_MovableMan.GetMOFromID(itemMOID);
			if (pItem) {
				m_pItemInReach = pItem ? dynamic_cast<HeldDevice *>(pItem->GetRootParent()) : 0;
				if (m_pItemInReach) { m_PieNeedsUpdate = true; }
			}
		}
	}

    // Item currently set to be within reach has expired or is now out of range
    if (m_pItemInReach && (m_pItemInReach->IsUnPickupable() || (m_pItemInReach->HasPickupLimitations() && !m_pItemInReach->IsPickupableBy(this)) || !g_MovableMan.IsDevice(m_pItemInReach) || g_SceneMan.ShortestDistance(reachPoint, m_pItemInReach->GetPos(), g_SceneMan.SceneWrapsX()).GetMagnitude() > reach + m_pItemInReach->GetRadius())) {
        m_pItemInReach = 0;
        m_PieNeedsUpdate = true;
    }

    // Pick up the designated item
    if (m_pItemInReach && m_pFGArm && m_Controller.IsState(WEAPON_PICKUP) && m_Status != INACTIVE)
    {
        // Remove the item from the scene, it's gong into the hands of this
        if (g_MovableMan.RemoveMO(m_pItemInReach))
        {
            // Replace whatever's in the FG arm (if anything) with what we are picking up
            MovableObject *pMO = m_pFGArm->ReleaseHeldMO();
			if (pMO) { m_Inventory.push_back(pMO); }
            m_pFGArm->SetHeldMO(m_pItemInReach);
            m_pFGArm->SetHandPos(m_Pos + m_HolsterOffset.GetXFlipped(m_HFlipped));

            m_PieNeedsUpdate = true;
			if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }
		}
    }

    ///////////////////////////////////////////////////
    // Travel the limb AtomGroup:s

    if (m_Status == STABLE && m_MoveState != NOMOVE)
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
        if (m_MoveState == WALK || (m_MoveState == JUMP && m_Vel.GetLargest() < 1.0))
        {
            m_Paths[FGROUND][STAND].Terminate();
            m_Paths[BGROUND][STAND].Terminate();

//            float FGLegProg = MAX(m_Paths[FGROUND][WALK].GetRegularProgress(), m_Paths[FGROUND][WALK].GetTotalTimeProgress());
//            float BGLegProg = MAX(m_Paths[BGROUND][WALK].GetRegularProgress(), m_Paths[BGROUND][WALK].GetTotalTimeProgress());
            float FGLegProg = m_Paths[FGROUND][WALK].GetRegularProgress();
            float BGLegProg = m_Paths[BGROUND][WALK].GetRegularProgress();

            bool playStride = false;

            // Make sure we are starting a stride if we're basically stopped
            if (fabs(m_Vel.GetLargest()) < 0.5)
                m_StrideStart = true;

            if (m_pFGLeg && (!m_pBGLeg || (!(m_Paths[FGROUND][WALK].PathEnded() && BGLegProg < 0.5) || m_StrideStart)))
            {
//                m_StrideStart = false;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[FGROUND][WALK].PathEnded() || m_Paths[FGROUND][WALK].PathIsAtStart())
                    m_StrideTimer.Reset();
                m_ArmClimbing[BGROUND] = !m_pFGFootGroup->PushAsLimb(m_Pos +
                                                                     m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                                                     m_Vel,
                                                                     Matrix(),
                                                                     m_Paths[FGROUND][WALK],
                                                                     deltaTime,
                                                                     &playStride,
                                                                     false);
            }
            else
                m_ArmClimbing[BGROUND] = false;

            if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][WALK].PathEnded() && FGLegProg < 0.5)))
            {
                m_StrideStart = false;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[BGROUND][WALK].PathEnded() || m_Paths[BGROUND][WALK].PathIsAtStart())
                    m_StrideTimer.Reset();
                m_ArmClimbing[FGROUND] = !m_pBGFootGroup->PushAsLimb(m_Pos +
                                                                     m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                                                     m_Vel,
                                                                     Matrix(),
                                                                     m_Paths[BGROUND][WALK],
                                                                     deltaTime,
                                                                     &playStride,
                                                                     false);
            }
            else
                m_ArmClimbing[FGROUND] = false;

            // Play the stride sound, if applicable
            if (playStride && !m_ArmClimbing[FGROUND] && !m_ArmClimbing[BGROUND]) {
				if (m_StrideSound) { m_StrideSound->Play(m_Pos); }
                RunScriptedFunctionInAppropriateScripts("OnStride");
            }

            ////////////////////////////////////////
            // Arm Climbing if the leg paths failed to find clear spot to restart

//            float FGArmProg = MAX(m_Paths[FGROUND][CLIMB].GetRegularProgress(), m_Paths[FGROUND][CLIMB].GetTotalTimeProgress());
//            float BGArmProg = MAX(m_Paths[BGROUND][CLIMB].GetRegularProgress(), m_Paths[BGROUND][CLIMB].GetTotalTimeProgress());
            float FGArmProg = m_Paths[FGROUND][CLIMB].GetRegularProgress();
            float BGArmProg = m_Paths[BGROUND][CLIMB].GetRegularProgress();

            // Slightly negative BGArmProg makes sense because any progress on the starting segments are reported as negative,
            // and there's many starting segments on properly formed climbing paths
            if (m_pFGArm && (m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND]) && (!m_pBGArm || !m_pFGLeg || BGArmProg > 0.1))
            {
                m_ArmClimbing[FGROUND] = true;
                m_Paths[FGROUND][WALK].Terminate();
    //            m_Paths[BGROUND][WALK].Terminate();
                m_StrideStart = true;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[FGROUND][CLIMB].PathEnded() || m_Paths[FGROUND][CLIMB].PathIsAtStart())
                    m_StrideTimer.Reset();
                m_pFGHandGroup->PushAsLimb(m_Pos +
                                           m_pFGArm->GetParentOffset().GetXFlipped(m_HFlipped),
                                           m_Vel,
                                           m_Rotation,
                                           m_Paths[FGROUND][CLIMB],
                                           deltaTime);
            }
            else
            {
                m_ArmClimbing[FGROUND] = false;
                m_Paths[FGROUND][CLIMB].Terminate();
            }

            if (m_pBGArm && (m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND]))
            {
                m_ArmClimbing[BGROUND] = true;
    //            m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
                m_StrideStart = true;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[BGROUND][CLIMB].PathEnded() || m_Paths[BGROUND][CLIMB].PathIsAtStart())
                    m_StrideTimer.Reset();
                m_pBGHandGroup->PushAsLimb(m_Pos +
                                           m_pBGArm->GetParentOffset().GetXFlipped(m_HFlipped),
                                           m_Vel,
                                           m_Rotation,
                                           m_Paths[BGROUND][CLIMB],
                                           deltaTime);
            }
            else
            {
                m_ArmClimbing[BGROUND] = false;
                m_Paths[BGROUND][CLIMB].Terminate();
            }

            // Restart the climbing stroke if the current one seems to be taking too long without movement
            if ((m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND]) && fabs(m_Vel.GetLargest()) < 0.5 && m_StrideTimer.IsPastSimMS(m_Paths[BGROUND][CLIMB].GetTotalPathTime() / 4))
            {
                m_StrideStart = true;
                m_Paths[FGROUND][CLIMB].Terminate();
                m_Paths[BGROUND][CLIMB].Terminate();
            }
            // Reset the walking stride if it's taking too long
            else if (m_StrideTimer.IsPastSimMS(m_Paths[FGROUND][WALK].GetTotalPathTime()))
            {
                m_StrideStart = true;
                m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
            }
        }
        // CRAWLING
        else if (m_MoveState == CRAWL)
        {
            // LEG Crawls
            float FGLegProg = m_Paths[FGROUND][CRAWL].GetRegularProgress();
            float BGLegProg = m_Paths[BGROUND][CRAWL].GetRegularProgress();

            // FG Leg crawl
            if (m_pFGLeg && (!m_pBGLeg || (!(m_Paths[FGROUND][CRAWL].PathEnded() && BGLegProg < 0.5) || m_StrideStart)))
            {
//                m_StrideStart = false;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[FGROUND][CRAWL].PathEnded() || m_Paths[FGROUND][CRAWL].PathIsAtStart())
                    m_StrideTimer.Reset();
                m_pFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pFGLeg->GetParentOffset()),
                                                                   m_Vel,
                                                                   m_Rotation,
                                                                   m_Paths[FGROUND][CRAWL],
                                                                   deltaTime,
                                                                   0,
                                                                   true);
            }
            else
                m_Paths[FGROUND][CRAWL].Terminate();

            // BG Leg crawl
            if (m_pBGLeg && (!m_pFGLeg || !(m_Paths[BGROUND][CRAWL].PathEnded() && FGLegProg < 0.5)))
            {
                m_StrideStart = false;
                // Reset the stride timer if the path is about to restart
                if (m_Paths[BGROUND][CRAWL].PathEnded() || m_Paths[BGROUND][CRAWL].PathIsAtStart())
                    m_StrideTimer.Reset();
                // If both legs can't find free resrtart, ti's time to use the arm!
                m_pBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pBGLeg->GetParentOffset()),
                                                                   m_Vel,
                                                                   m_Rotation,
                                                                   m_Paths[BGROUND][CRAWL],
                                                                   deltaTime,
                                                                   0,
                                                                   true);
            }
            else
                m_Paths[BGROUND][CRAWL].Terminate();

            // ARMS using rotated path to help crawl
            if (m_pBGArm)
            {
                m_ArmClimbing[BGROUND] = true;
                // Reset the stride timer if the path is about to restart
//                if (m_Paths[BGROUND][ARMCRAWL].PathEnded() || m_Paths[BGROUND][ARMCRAWL].PathIsAtStart())
//                    m_StrideStart = true;
                m_pBGHandGroup->PushAsLimb(m_Pos + RotateOffset(m_pBGArm->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[BGROUND][ARMCRAWL],
                                            deltaTime,
                                            0,
                                            true);
            }
/*
            if (m_pFGArm)
            {
                m_ArmClimbing[FGROUND] = true;
//                m_StrideStart = true;
                m_pFGHandGroup->PushAsLimb(m_Pos + RotateOffset(m_pFGArm->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[FGROUND][ARMCRAWL],
            //                              mass,
                                            deltaTime,
                                            0,
                                            true);
            }
*/

            // Restart the stride if the current one seems to be taking too long
            if (m_StrideTimer.IsPastSimMS(m_Paths[FGROUND][CRAWL].GetTotalPathTime()))
            {
                m_StrideStart = true;
                m_Paths[FGROUND][CRAWL].Terminate();
                m_Paths[BGROUND][CRAWL].Terminate();
            }
        }
        // JUMPING
        else if ((m_pFGLeg || m_pBGLeg) && m_MoveState == JUMP) {
            //TODO 4zK Uncomment this section to keep the limb held static
            /*
            if (m_pFGLeg) {
                m_pFGFootGroup->SetLimbPos(m_Pos + RotateOffset(m_Paths[FGROUND][STAND].GetStartOffset()));
            }
            if (m_pBGLeg) {
                m_pBGFootGroup->SetLimbPos(m_Pos + RotateOffset(m_Paths[BGROUND][STAND].GetStartOffset()));
            }
            */

            //TODO 4zK Uncomment this section to make the limb follow its jump path. I believe this was data's original intention but
            // 1. The existing standard jump limbpath is awful, the actor spends all its time squatting
            // 2. I'm not sure of the details, but this push as limb doesn't seem to be advancing the jump limbpath, so it doesn't work very well, even with my efforts to properly reset it
            /*
            if (m_pFGLeg && (!m_Paths[FGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal)) {
                m_pFGFootGroup->PushAsLimb(
                    m_Pos + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                    m_Vel,
                    Matrix(),
                    m_Paths[FGROUND][m_MoveState],
                    deltaTime);
            }
            if (m_pBGLeg && (!m_Paths[BGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal)) {
                m_pBGFootGroup->PushAsLimb(
                    m_Pos + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                    m_Vel,
                    Matrix(),
                    m_Paths[BGROUND][m_MoveState],
                    deltaTime);
            }
            */
            if (m_pFGLeg && (!m_Paths[FGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal)) {
                m_pFGFootGroup->FlailAsLimb(
                    m_Pos,
                    m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                    m_pFGLeg->GetMaxLength(),
                    g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                    m_AngularVel,
                    m_pFGLeg->GetMass(),
                    g_TimerMan.GetDeltaTimeSecs());
            }
            if (m_pBGLeg && (!m_Paths[BGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal)) {
                m_pBGFootGroup->FlailAsLimb(
                    m_Pos,
                    m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped) *m_Rotation,
                    m_pBGLeg->GetMaxLength(),
                    g_SceneMan.GetGlobalAcc() *g_TimerMan.GetDeltaTimeSecs(),
                    m_AngularVel,
                    m_pBGLeg->GetMass(),
                    g_TimerMan.GetDeltaTimeSecs());
            }

            if (m_JetTimeLeft <= 0) {
                m_MoveState = STAND;
                m_Paths[FGROUND][JUMP].Terminate();
                m_Paths[BGROUND][JUMP].Terminate();
                m_Paths[FGROUND][STAND].Terminate();
                m_Paths[BGROUND][STAND].Terminate();
                m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
            }
        }
        // CROUCHING
        else if ((m_pFGLeg || m_pBGLeg) && m_MoveState == CROUCH)
        {
            m_Paths[FGROUND][WALK].Terminate();
            m_Paths[BGROUND][WALK].Terminate();
            m_Paths[FGROUND][CRAWL].Terminate();
            m_Paths[BGROUND][CRAWL].Terminate();

            if (m_pFGLeg)
                m_pFGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                           m_Vel,
                                           Matrix(),
                                           m_Paths[FGROUND][CROUCH],
                                           deltaTime);

            if (m_pBGLeg)
                m_pBGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                           m_Vel,
                                           Matrix(),
                                           m_Paths[BGROUND][CROUCH],
                                           deltaTime);
        }
        // STANDING
        else if (m_pFGLeg || m_pBGLeg)
        {
            m_Paths[FGROUND][WALK].Terminate();
            m_Paths[BGROUND][WALK].Terminate();
            m_Paths[FGROUND][CRAWL].Terminate();
            m_Paths[BGROUND][CRAWL].Terminate();

            if (m_pFGLeg)
                m_pFGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                      m_Vel,
                                      Matrix(),
                                      m_Paths[FGROUND][STAND],
                                      deltaTime,
                                      0,
                                      false);

            if (m_pBGLeg)
                m_pBGFootGroup->PushAsLimb(m_Pos.GetFloored() + m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped),
                                      m_Vel,
                                      Matrix(),
                                      m_Paths[BGROUND][STAND],
                                      deltaTime,
                                      0,
                                      false);
        }
    }
    // Not stable/standing, so make sure the end of limbs are moving around limply in a ragdoll fashion
    else
    {

// TODO: Make the limb atom groups fly around and react to terrain, without getting stuck etc
        bool wrapped = false;
        Vector limbPos;
        if (m_pFGArm)
        {
//            m_pFGHandGroup->SetLimbPos(m_pFGArm->GetHandPos(), m_HFlipped);
            m_pFGHandGroup->FlailAsLimb(m_Pos,
                                        m_pFGArm->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                        m_pFGArm->GetMaxLength(),
                                        g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                        m_AngularVel,
                                        m_pFGArm->GetMass(),
                                        g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pBGArm)
        {
//            m_pBGHandGroup->SetLimbPos(m_pBGArm->GetHandPos(), m_HFlipped);
            m_pBGHandGroup->FlailAsLimb(m_Pos,
                                        m_pBGArm->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                        m_pBGArm->GetMaxLength(),
                                        g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                        m_AngularVel,
                                        m_pBGArm->GetMass(),
                                        g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pFGLeg)
        {
//            m_pFGFootGroup->SetLimbPos(m_pFGLeg->GetAnklePos(), m_HFlipped);
            m_pFGFootGroup->FlailAsLimb(m_Pos,
                                        m_pFGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                        m_pFGLeg->GetMaxLength(),
                                        g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                        m_AngularVel,
                                        m_pFGLeg->GetMass(),
                                        g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pBGLeg)
        {
//            m_pBGFootGroup->SetLimbPos(m_pBGLeg->GetAnklePos(), m_HFlipped);
            m_pBGFootGroup->FlailAsLimb(m_Pos,
                                        m_pBGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                        m_pBGLeg->GetMaxLength(),
                                        g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                        m_AngularVel,
                                        m_pBGLeg->GetMass(),
                                        g_TimerMan.GetDeltaTimeSecs());
        }
    }

    /////////////////////////////////
    // Manage Attachable:s
    if (m_pHead && m_pHead->IsAttached()) {
        float toRotate = 0;
        // Only rotate the head to match the aim angle if body is stable and upright
        if (m_Status == STABLE && std::fabs(m_Rotation.GetRadAngle()) < (c_HalfPI + c_QuarterPI)) {
            toRotate = m_pHead->GetRotMatrix().GetRadAngleTo((m_AimAngle * static_cast<float>(GetFlipFactor())) * m_LookToAimRatio + m_Rotation.GetRadAngle() * (0.9F - m_LookToAimRatio));
            toRotate *= 0.15F;
        }
        // If dying upright, make head slump forward or back depending on body lean
// TODO: Doesn't work too well, but probably could
//        else if ((m_Status == DEAD || m_Status == DYING) && fabs(m_Rotation.GetRadAngle()) < c_QuarterPI)
//        {
//            toRotate = m_pHead->GetRotMatrix().GetRadAngleTo(m_Rotation.GetRadAngle() + ((m_HFlipped && m_Rotation.GetRadAngle() > 0) || (!m_HFlipped && m_Rotation.GetRadAngle() > 0) ? c_PI : -c_PI) * 0.6);
//            toRotate *= 0.10;
//        }
        // Make head just keep rotating loosely with the body if unstable or upside down
        else {
            toRotate = m_pHead->GetRotMatrix().GetRadAngleTo(m_Rotation.GetRadAngle());
            toRotate *= 0.10F;
        }
        // Now actually rotate by the amount calculated above
        m_pHead->SetRotAngle(m_pHead->GetRotAngle() + toRotate);
    }

    if (m_pFGLeg && m_pFGLeg->IsAttached()) {
        m_pFGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
        m_pFGLeg->SetTargetPosition(m_pFGFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pBGLeg && m_pBGLeg->IsAttached()) {
        m_pBGLeg->EnableIdle(m_ProneState == NOTPRONE && m_Status != UNSTABLE);
        m_pBGLeg->SetTargetPosition(m_pBGFootGroup->GetLimbPos(m_HFlipped));
    }

	if (m_pFGArm) {
		float affectingBodyAngle = 0.0F;
		if (m_FGArmFlailScalar != 0 && m_SharpAimDelay != 0) {
			float aimScalar = std::min(static_cast<float>(m_SharpAimTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			float revertScalar = std::min(static_cast<float>(m_SharpAimRevertTimer.GetElapsedSimTimeMS()) / static_cast<float>(m_SharpAimDelay), 1.0F);
			aimScalar = (aimScalar > revertScalar) ? aimScalar : 1.0F - revertScalar;

			affectingBodyAngle = std::abs(std::sin(rot)) * rot * m_FGArmFlailScalar * (1.0F - aimScalar);
		}
		m_pFGArm->SetRotAngle(affectingBodyAngle + m_AimAngle * static_cast<float>(GetFlipFactor()));

        if (m_Status == STABLE) {
            if (m_ArmClimbing[FGROUND]) {
                m_pFGArm->ReachToward(m_pFGHandGroup->GetLimbPos(m_HFlipped));
            } else if (!m_pFGArm->IsReaching()) {
				// Use an unreachable position to force this arm to idle, so it wont bug out where the AtomGroup was left off
				m_pFGArm->Reach(Vector());
            }
        } else {
            // Unstable, so just drop the arm limply
            m_pFGArm->ReachToward(m_pFGHandGroup->GetLimbPos(m_HFlipped));
        }
    }

    if (m_pBGArm) {
		m_pBGArm->SetRotAngle(std::abs(std::sin(rot)) * rot * m_BGArmFlailScalar + (m_AimAngle * static_cast<float>(GetFlipFactor())));
        if (m_Status == STABLE) { 
			if (m_ArmClimbing[BGROUND]) {
				// Can't climb or crawl with the shield
				if (m_MoveState == CLIMB || (m_MoveState == CRAWL && m_ProneState == PRONE)) { UnequipBGArm(); }
				m_pBGArm->ReachToward(m_pBGHandGroup->GetLimbPos(m_HFlipped));

			} else {
				// Re-equip shield in BG arm after climbing
				EquipShieldInBGArm();
				if (m_pFGArm && m_pFGArm->HoldsHeldDevice() && !m_pBGArm->HoldsHeldDevice()) {
					m_pBGArm->Reach(m_pFGArm->GetHeldDevice()->GetSupportPos());

					// BGArm does reach to support the device held by FGArm.
					if (m_pBGArm->DidReach()) {
						m_pFGArm->GetHeldDevice()->SetSupported(true);
						m_pBGArm->SetRecoil(m_pFGArm->GetHeldDevice()->GetRecoilForce(), m_pFGArm->GetHeldDevice()->GetRecoilOffset(), m_pFGArm->GetHeldDevice()->IsRecoiled());
					} else {
						// BGArm did not reach to support the device. Count device as supported anyway, if crouching
						m_pFGArm->GetHeldDevice()->SetSupported(m_MoveState == CROUCH || m_ProneState == PRONE);
						m_pBGArm->SetRecoil(Vector(), Vector(), false);
					}
				} else {
					// Use an unreachable position to force this arm to idle, so it wont bug out where the AtomGroup was left off
					m_pBGArm->Reach(Vector());
				}
			}
        } else {
            // Unstable, so just drop the arm limply
            m_pBGArm->ReachToward(m_pBGHandGroup->GetLimbPos(m_HFlipped));
        }
	} else if (m_pFGArm && m_pFGArm->HoldsHeldDevice()) {
		m_pFGArm->GetHeldDevice()->SetSupported(false);
    }

    /////////////////////////////////////////////////
    // Update MovableObject, adds on the forces etc
    // NOTE: this also updates the controller, so any setstates of it will be wiped!
    Actor::Update();

    ////////////////////////////////////
    // Update viewpoint

    // Set viewpoint based on how we are aiming etc.
    Vector aimSight(m_AimDistance, 0);
    Matrix aimMatrix(m_HFlipped ? -m_AimAngle : m_AimAngle);
    aimMatrix.SetXFlipped(m_HFlipped);
    // Reset this each frame
    m_SharpAimMaxedOut = false;

    if (m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice())
    {
        float maxLength = m_pFGArm->GetHeldDevice()->GetSharpLength();
		if (maxLength == 0) {
			m_SharpAimProgress = 0;
			m_SharpAimMaxedOut = true;
		} else if (m_MoveState == WALK) {
			maxLength *= 0.7F;
		}
		// Use a non-terrain check ray to cap the magnitude, so we can't see into objects etc
		if (m_SharpAimProgress > 0) {
			if (m_pFGArm->GetHeldDevice()->IsRecoiled()) {
				float totalGripStrength = m_pFGArm->GetGripStrength() + (m_pBGArm && m_pBGArm->IsAttached() && m_pBGArm->HoldsHeldDevice() ? m_pBGArm->GetGripStrength() : 0);
				m_SharpAimProgress *= 1 - std::min(m_pFGArm->GetHeldDevice()->GetRecoilForce().GetMagnitude() / std::max(totalGripStrength * m_pFGArm->GetHeldDevice()->GetGripStrengthMultiplier(), 1.0F), 1.0F);
			}
			Vector notUsed;
            Vector sharpAimVector(maxLength, 0);
            sharpAimVector *= aimMatrix;

            // See how far along the sharp aim vector there is opaque air
//            float result = g_SceneMan.CastNotMaterialRay(m_pFGArm->GetHeldDevice()->GetMuzzlePos(), sharpAimVector, g_MaterialAir, 5);
            float result = g_SceneMan.CastObstacleRay(m_pFGArm->GetHeldDevice()->GetMuzzlePos(), sharpAimVector, notUsed, notUsed, GetRootID(), IgnoresWhichTeam(), g_MaterialAir, 5);
            // If we didn't find anything but air before the sharpdistance, then don't alter the sharp distance
            if (result >= 0 && result < (maxLength * m_SharpAimProgress))
            {
                m_SharpAimProgress = result / maxLength;
                m_SharpAimMaxedOut = true;
            }
        }
        // Indicate maxed outedness if we really are, too
        if (m_SharpAimProgress > 0.9)
            m_SharpAimMaxedOut = true;

//        sharpDistance *= m_Controller.GetAnalogAim().GetMagnitude();
        aimSight.m_X += maxLength * m_SharpAimProgress;
    }

    // Rotate the aiming spot vector and add it to the view point
    aimSight *= aimMatrix;
    m_ViewPoint = m_Pos.GetFloored() + aimSight;

	// Add velocity also so the viewpoint moves ahead at high speeds
	if (m_Vel.GetMagnitude() > 10.0F) { m_ViewPoint += m_Vel * std::sqrt(m_Vel.GetMagnitude() * 0.1F); }


/* Done by pie menu now, see HandlePieCommand()
    ////////////////////////////////////////
    // AI mode setting
    
    if (m_Controller.IsState(AI_MODE_SET))
    {
        if (m_Controller.IsState(PRESS_RIGHT))
        {
            m_AIMode = AIMODE_BRAINHUNT;
            m_UpdateMovePath = true;
        }
        else if (m_Controller.IsState(PRESS_LEFT))
        {
            m_AIMode = AIMODE_PATROL;
        }
        else if (m_Controller.IsState(PRESS_UP))
        {
            m_AIMode = AIMODE_SENTRY;
        }
        else if (m_Controller.IsState(PRESS_DOWN))
        {
            m_AIMode = AIMODE_GOLDDIG;
        }

        m_DeviceState = SCANNING;
    }
*/
    /////////////////////////////////////////
    // Gold Chunk inventroy management

    if (m_GoldInInventoryChunk <= 0) {
        ChunkGold();
    }


    ////////////////////////////////////////
    // Balance stuff

    // Eliminate full rotations
    while (fabs(rot) > c_TwoPI) {
        rot -= rot > 0 ? c_TwoPI : -c_TwoPI;
    }
    // Eliminate rotations over half a turn
    if (fabs(rot) > c_PI)
    {
        rot = (rot > 0 ? -c_PI : c_PI) + (rot - (rot > 0 ? c_PI : -c_PI));
        // If we're upside down, we're unstable damnit
		if (m_Status != DYING && m_Status != DEAD)
			m_Status = UNSTABLE;
        m_StableRecoverTimer.Reset();
    }

    // Rotational balancing spring calc
    if (m_Status == STABLE)
    {
        // If we're supposed to be laying down on the ground, make the spring pull the body that way until we reach that angle
        if (m_ProneState != NOTPRONE)
        {
            float rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
            float rotDiff = rotTarget - rot;

            if (m_ProneState == GOPRONE)
            {
                if (!m_ProneTimer.IsPastSimMS(333))
                {
                    if (fabs(rotDiff) > 0.1 && fabs(rotDiff) < c_PI)
                    {
                        m_AngularVel += rotDiff * 0.45;// * fabs(rotDiff);
                        m_Vel.m_X += (m_HFlipped ? -fabs(rotDiff) : fabs(rotDiff)) * 0.25;
                    }
                }
                // Done going down, now stay down without spring
                else
                {
                    m_AngularVel *= 0.5;
                    m_ProneState = PRONE;
                }
/*
                // Break the spring if close to target angle.
                if (-c_HalfPI + fabs(rot) > 0.1)
                    m_AngularVel -= rot * fabs(rot);
                else if (fabs(m_AngularVel) > 0.1)
                    m_AngularVel *= 0.5;
*/
            }
            // If down, try to keep flat against the ground
            else if (m_ProneState == PRONE)
            {
                if (fabs(rotDiff) > c_SixteenthPI && fabs(rotDiff) < c_HalfPI)
                    m_AngularVel += rotDiff * 0.65;// * fabs(rotDiff);
                else if (fabs(m_AngularVel) > 0.3)
                    m_AngularVel *= 0.85;
            }
		} else {
			// Upright body posture
			float rotDiff = rot - (GetRotAngleTarget(m_MoveState) * (m_AimAngle > 0 ? 1 - (m_AimAngle / c_HalfPI) : 1) * static_cast<float>(GetFlipFactor()));
			m_AngularVel = m_AngularVel * (0.98F - 0.08F * (m_Health / m_MaxHealth)) - (rotDiff * 0.5F);
		}
    }
    // Keel over
    else if (m_Status == UNSTABLE && fabs(m_AngularVel) < 5)
    {
        float rotTarget = 0;
        // If traveling at speed, then always start falling forward
        if (fabs(m_Vel.m_X) > 1.0)
            rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
        // Go whichever way we're already rotated
        else
            rotTarget = rot > 0 ? c_HalfPI : -c_HalfPI;

        float rotDiff = rotTarget - rot;
        if (fabs(rotDiff) > 0.1 && fabs(rotDiff) < c_PI)
        {
            m_AngularVel += rotDiff * 0.05;
//            m_Vel.m_X += (rotTarget > 0 ? -fabs(rotDiff) : fabs(rotDiff)) * 0.35;
        }
    }
    // While dying, pull body quickly toward down toward horizontal
    else if (m_Status == DYING)
    {
        float rotTarget = rot > 0 ? c_HalfPI : -c_HalfPI;
//        float rotTarget = m_HFlipped ? c_HalfPI : -c_HalfPI;
        float rotDiff = rotTarget - rot;
        if (!m_DeathTmr.IsPastSimMS(125) && fabs(rotDiff) > 0.1 && fabs(rotDiff) < c_PI)
        {
            m_AngularVel += rotDiff * 0.5;//fabs(rotDiff);
//            m_Vel.m_X += (m_HFlipped ? -fabs(rotDiff) : fabs(rotDiff)) * 0.35;
            m_Vel.m_X += (rotTarget > 0 ? -fabs(rotDiff) : fabs(rotDiff)) * 0.35;
        }
        else
            m_Status = DEAD;

//        else if (fabs(m_AngularVel) > 0.1)
//            m_AngularVel *= 0.5;
    }
    m_Rotation.SetRadAngle(rot);

    ///////////////////////////////////////////////////
    // Death detection and handling

    // Losing head should kill
    if (!m_pHead && m_Status != DYING && m_Status != DEAD)
        m_Health -= m_MaxHealth + 1;
    // Losing all limbs should kill... eventually
    else if (!m_pFGArm && !m_pBGArm && !m_pFGLeg && !m_pBGLeg && m_Status != DYING && m_Status != DEAD)
        m_Health -= 0.1;

    if (m_Status == DYING)
    {
        if (m_pFGArm && m_pFGArm->IsAttached())
            m_pFGArm->DropEverything();
        if (m_pBGArm && m_pBGArm->IsAttached())
            m_pBGArm->DropEverything();
    }

    /////////////////////////////////////////
    // Misc.

//    m_DeepCheck = true/*m_Status == DEAD*/;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawThrowingReticule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws an aiming aid in front of this HeldDevice for throwing.

void AHuman::DrawThrowingReticule(BITMAP *pTargetBitmap, const Vector &targetPos, double amount) const
{
    const int pointCount = 9;
    Vector points[pointCount];
    //Color colors[pointCount];

	for (int index = 0; index < pointCount; index++) {
		points[index].SetXY(static_cast<float>(index * 4), 0.0F);
		//colors[index].SetRGB(255 - index * 3, 225 - index * 20, index);
	}

    Vector outOffset(m_HFlipped ? -15 : 15, -4);
//    Matrix aimMatrix(m_AimAngle);
//    aimMatrix.SetXFlipped(m_HFlipped);

    acquire_bitmap(pTargetBitmap);

    for (int i = 0; i < pointCount * amount; ++i) {
        points[i].FlipX(m_HFlipped);
        points[i] += outOffset;
        points[i].RadRotate(m_HFlipped ? -m_AimAngle : m_AimAngle);
        points[i] += m_Pos;
        if (m_pFGArm && m_pFGArm->IsAttached())
            points[i] += m_pFGArm->GetParentOffset();

        // Put the flickering glows on the reticule dots, in absolute scene coordinates
		g_PostProcessMan.RegisterGlowDotEffect(points[i], YellowDot, 55 + RandomNum(0, 100));

        putpixel(pTargetBitmap, points[i].GetFloorIntX() - targetPos.GetFloorIntX(), points[i].GetFloorIntY() - targetPos.GetFloorIntY(), g_YellowGlowColor);
    }

    release_bitmap(pTargetBitmap);
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

    //TODO simplify this complex if check when arm is cleaned up like turret so all it can hold are HeldDevices and children
    // Draw background Arm's hand after the HeldDevice of FGArm is drawn if the FGArm is holding a weapon.
    if (m_pFGArm && m_pBGArm && !onlyPhysical && mode == g_DrawColor && m_pBGArm->DidReach() && m_pFGArm->HoldsHeldDevice() && !m_pFGArm->HoldsThrownDevice() && !m_pFGArm->GetHeldDevice()->IsReloading() && !m_pFGArm->GetHeldDevice()->IsShield()) {
        m_pBGArm->DrawHand(pTargetBitmap, targetPos, realMode);
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
    if (!m_HUDVisible)
        return;

    // Only do HUD if on a team
    if (m_Team < 0)
        return;

    // Only draw if the team viewing this is on the same team OR has seen the space where this is located
    int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && g_SceneMan.IsUnseen(m_Pos.GetFloorIntX(), m_Pos.GetFloorIntY(), viewingTeam)) {
		return;
    }

    Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);

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
    list<Vector>::iterator last = m_MovePath.begin();
    Vector waypoint, lastPoint, lineVec;
    for (list<Vector>::iterator lItr = m_MovePath.begin(); lItr != m_MovePath.end(); ++lItr)
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
    // Raidus
//    waypoint = m_Pos - targetPos;
//    circle(pTargetBitmap, waypoint.m_X, waypoint.m_Y, m_MoveProximityLimit, g_RedColor);  
#endif

    // Player AI drawing

    // Device aiming reticule
	if (m_Controller.IsState(AIM_SHARP) && m_pFGArm && m_pFGArm->IsAttached() && m_pFGArm->HoldsHeldDevice()) {
		m_pFGArm->GetHeldDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen, m_Controller.IsPlayerControlled());
	}
        

    // Throwing reticule
	if (m_ArmsState == THROWING_PREP) {
		DrawThrowingReticule(pTargetBitmap, targetPos, std::min(m_ThrowTmr.GetElapsedSimTimeMS() / m_ThrowPrepTime, 1.0));
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
        // Device aiming reticule
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

        // Weight and jetpack energy
        if (m_pJetpack && m_pJetpack->IsAttached() && m_Controller.IsState(BODY_JUMP))
        {
            // Draw empty fuel indicator
            if (m_JetTimeLeft < 100)
                str[0] = m_IconBlinkTimer.AlternateSim(100) ? -26 : -25;
            // Display normal jet icons
            else
            {
                float acceleration = m_pJetpack->EstimateImpulse(false) / max(GetMass(), 0.1F);
				if (acceleration > 0.47F) {
					str[0] = -31;
				} else {
					str[0] = acceleration > 0.41F ? -30 : (acceleration > 0.35F ? -29 : -28);
				}
				// Do the blinky blink
				if ((str[0] == -28 || str[0] == -29) && m_IconBlinkTimer.AlternateSim(250)) { str[0] = -27; }
            }
            // null-terminate
            str[1] = 0;
            pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - 11, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Centre);

            float jetTimeRatio = m_JetTimeLeft / m_JetTimeTotal;
// TODO: Don't hardcode this shit
            int gaugeColor = jetTimeRatio > 0.6F ? 149 : (jetTimeRatio > 0.3F ? 77 : 13);
            rectfill(pTargetBitmap, drawPos.GetFloorIntX(), drawPos.GetFloorIntY() + m_HUDStack + 6, drawPos.GetFloorIntX() + (16 * jetTimeRatio), drawPos.GetFloorIntY() + m_HUDStack + 7, gaugeColor);
//                    rect(pTargetBitmap, drawPos.m_X, drawPos.m_Y + m_HUDStack - 2, drawPos.m_X + 24, drawPos.m_Y + m_HUDStack - 4, 238);
//                    std::snprintf(str, sizeof(str), "%.0f Kg", mass);
//                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);

            m_HUDStack += -10;
        }
        // Held-related GUI stuff
        else if (m_pFGArm && m_pFGArm->IsAttached())
        {
            HDFirearm *pHeldFirearm = dynamic_cast<HDFirearm *>(m_pFGArm->GetHeldDevice());

            // Ammo
            if (pHeldFirearm)
            {
                MovableObject *bgHeldItem = GetEquippedBGItem();
                HDFirearm const *bgHeldFirearm = bgHeldItem == NULL ? NULL : dynamic_cast<HDFirearm *>(bgHeldItem);

                str[0] = -56; str[1] = 0;
                pSymbolFont->DrawAligned(&allegroBitmap, drawPos.m_X - 10, drawPos.m_Y + m_HUDStack, str, GUIFont::Left);
                std::string fgWeaponString = pHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(pHeldFirearm->GetRoundInMagCount());
                fgWeaponString = pHeldFirearm->IsReloading() ? "Reloading" : fgWeaponString;

                if (bgHeldItem && bgHeldFirearm) {
                    std::string bgWeaponString = bgHeldFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(bgHeldFirearm->GetRoundInMagCount());
                    bgWeaponString = bgHeldFirearm->IsReloading() ? "Reloading" : bgWeaponString;
                    std::snprintf(str, sizeof(str), "%s | %s", fgWeaponString.c_str(), bgWeaponString.c_str());
                } else {
                    std::snprintf(str, sizeof(str), "%s", fgWeaponString.c_str());
                }
                pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);

                m_HUDStack += -10;
            }

            // Device changing GUI
            if (m_Controller.IsState(PIE_MENU_ACTIVE))
            {
/*
                // Display Gold tally if gold chunk is in hand
                if (m_pFGArm->HoldsSomething() && m_pFGArm->GetHeldMO()->IsGold() && GetGoldCarried() > 0)
                {
                    str[0] = m_GoldPicked ? -57 : -58; str[1] = 0;
                    pSymbolFont->DrawAligned(&allegroBitmap, drawPos.m_X - 11, drawPos.m_Y + m_HUDStack, str, GUIFont::Left);
                    std::snprintf(str, sizeof(str), "%.0f oz", GetGoldCarried());
                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 2, str, GUIFont::Left);

                    m_HUDStack += -11;
                }
*/
                if (m_pFGArm->HoldsSomething())
                {
/*
                    std::snprintf(str, sizeof(str), " Œ Drop");
                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 12, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);
                    m_HUDStack += -9;
*/
//                    std::snprintf(str, sizeof(str), "   %s", m_pFGArm->GetHeldMO()->GetPresetName().c_str());
                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X, drawPos.m_Y + m_HUDStack + 3, m_pFGArm->GetHeldMO()->GetPresetName().c_str(), GUIFont::Centre);
                    m_HUDStack += -9;
                }
                else
                {
//                    std::snprintf(str, sizeof(str), "æ  EMPTY  ø");
                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X, drawPos.m_Y + m_HUDStack + 3, "EMPTY", GUIFont::Centre);
                    m_HUDStack += -9;
                }
/*
                // Reload GUI, only show when there's nothing to pick up
                if (!m_pItemInReach && m_pFGArm->HoldsSomething() && pHeldFirearm && !pHeldFirearm->IsFull())
                {
                    std::snprintf(str, sizeof(str), " œ Reload", pHeldFirearm);
                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 12, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);
                }
*/
            }
        }
        else
        {
            std::snprintf(str, sizeof(str), "NO ARM!");
            pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
            m_HUDStack += -9;
        }

        // Pickup GUI
        if (!m_Controller.IsState(PIE_MENU_ACTIVE))
        {
            if (m_pItemInReach && g_MovableMan.IsDevice(m_pItemInReach) && m_pFGArm && m_pFGArm->IsAttached())
            {
                std::snprintf(str, sizeof(str), " %c %s", -49, m_pItemInReach->GetPresetName().c_str());
                pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 12, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);
            }
            else
                m_pItemInReach = 0;
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


} // namespace RTE
