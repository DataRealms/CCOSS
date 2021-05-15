//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACrab.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ACrab class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ACrab.h"
#include "AtomGroup.h"
#include "Attachable.h"
#include "ThrownDevice.h"
#include "Turret.h"
#include "Leg.h"
#include "Controller.h"
#include "Matrix.h"
#include "AEmitter.h"
#include "HDFirearm.h"
#include "PieMenuGUI.h"
#include "Scene.h"
#include "SettingsMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

ConcreteClassInfo(ACrab, Actor, 20)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACrab, effectively
//                  resetting the members of this abstraction level only.

void ACrab::Clear()
{
    m_pTurret = 0;
    m_pLFGLeg = 0;
    m_pLBGLeg = 0;
    m_pRFGLeg = 0;
    m_pRBGLeg = 0;
    m_pLFGFootGroup = 0;
    m_BackupLFGFootGroup = nullptr;
    m_pLBGFootGroup = 0;
    m_BackupLBGFootGroup = nullptr;
    m_pRFGFootGroup = 0;
    m_BackupRFGFootGroup = nullptr;
    m_pRBGFootGroup = 0;
    m_BackupRBGFootGroup = nullptr;
    m_StrideSound = nullptr;
    m_pJetpack = 0;
    m_JetTimeTotal = 0.0;
    m_JetTimeLeft = 0.0;
    m_MoveState = STAND;
    for (int side = 0; side < SIDECOUNT; ++side)
    {
        for (int layer = 0; layer < LAYERCOUNT; ++layer)
        {
            for (int state = 0; state < MOVEMENTSTATECOUNT; ++state)
            {
                m_Paths[side][layer][state].Reset();
                m_Paths[side][layer][state].Terminate();
            }
        }
        m_StrideStart[side] = false;
//        m_StrideTimer[side].Reset();
    }
    m_Aiming = false;
    m_GoldInInventoryChunk = 0;

    m_DeviceState = SCANNING;
    m_SweepState = NOSWEEP;
    m_DigState = NOTDIGGING;
    m_JumpState = NOTJUMPING;
    m_JumpTarget.Reset();
    m_JumpingRight = true;
    m_DigTunnelEndPos.Reset();
    m_SweepCenterAimAngle = 0;
    m_SweepRange = c_EighthPI;
    m_DigTarget.Reset();
    m_FireTimer.Reset();
    m_SweepTimer.Reset();
    m_PatrolTimer.Reset();
    m_JumpTimer.Reset();
    m_AimRangeUpperLimit = -1;
    m_AimRangeLowerLimit = -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.

int ACrab::Create()
{
    // Read all the properties
    if (Actor::Create() < 0)
        return -1;

    // Create the background paths copied from the foreground ones which were already read in
    for (int side = 0; side < SIDECOUNT; ++side)
    {
        for (int i = 0; i < MOVEMENTSTATECOUNT; ++i)
        {
            m_Paths[side][BGROUND][i].Destroy();
            m_Paths[side][BGROUND][i].Create(m_Paths[side][FGROUND][i]);
        }
    }

    // Initalize the jump time left
    m_JetTimeLeft = m_JetTimeTotal;

    // All ACrabs by default avoid hitting each other ont he same team
    m_IgnoresTeamHits = true;

    // Check whether UpperLimit and LowerLimit are defined, if not, copy general AimRange value to preserve compatibility
    if (m_AimRangeUpperLimit == -1 || m_AimRangeLowerLimit == -1)
    {
        m_AimRangeUpperLimit = m_AimRange;
        m_AimRangeLowerLimit = m_AimRange;
    }

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.

int ACrab::Create(BITMAP *pSprite,
                   Controller *pController,
                   const float mass,
                   const Vector &position,
                   const Vector &velocity,
                   AtomGroup *hitBody,
                   const unsigned long lifetime,
                   Status status,
                   const int health)
{
    

    return Actor::Create(pSprite,
                         pController,
                         mass,
                         position,
                         velocity,
                         hitBody,
                         lifetime,
                         status,
                         health);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACrab to be identical to another, by deep copy.

int ACrab::Create(const ACrab &reference) {
    //Note - hardcoded attachable copying is organized based on desired draw order here.
    if (reference.m_pLBGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLBGLeg->GetUniqueID());
        SetLeftBGLeg(dynamic_cast<Leg *>(reference.m_pLBGLeg->Clone()));
    }
    if (reference.m_pRBGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRBGLeg->GetUniqueID());
        SetRightBGLeg(dynamic_cast<Leg *>(reference.m_pRBGLeg->Clone()));
    }
    if (reference.m_pJetpack) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pJetpack->GetUniqueID());
        SetJetpack(dynamic_cast<AEmitter *>(reference.m_pJetpack->Clone()));
    }
    if (reference.m_pTurret) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pTurret->GetUniqueID());
        SetTurret(dynamic_cast<Turret *>(reference.m_pTurret->Clone()));
    }
    if (reference.m_pLFGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLFGLeg->GetUniqueID());
        SetLeftFGLeg(dynamic_cast<Leg *>(reference.m_pLFGLeg->Clone()));
    }
    if (reference.m_pRFGLeg) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRFGLeg->GetUniqueID());
        SetRightFGLeg(dynamic_cast<Leg *>(reference.m_pRFGLeg->Clone()));
    }
    Actor::Create(reference);

    m_JetTimeTotal = reference.m_JetTimeTotal;
    m_JetTimeLeft = reference.m_JetTimeLeft;

    m_pLFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pLFGFootGroup->Clone());
    m_pLFGFootGroup->SetOwner(this);
    m_BackupLFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupLFGFootGroup->Clone());
    m_BackupLFGFootGroup->SetOwner(this);
    m_BackupLFGFootGroup->SetLimbPos(reference.m_BackupLFGFootGroup->GetLimbPos());
    m_pLBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pLBGFootGroup->Clone());
    m_pLBGFootGroup->SetOwner(this);
    m_BackupLBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupLBGFootGroup->Clone());
    m_BackupLBGFootGroup->SetOwner(this);
    m_BackupLBGFootGroup->SetLimbPos(reference.m_BackupLBGFootGroup->GetLimbPos());
    m_pRFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pRFGFootGroup->Clone());
    m_pRFGFootGroup->SetOwner(this);
    m_BackupRFGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupRFGFootGroup->Clone());
    m_BackupRFGFootGroup->SetOwner(this);
    m_BackupRFGFootGroup->SetLimbPos(reference.m_BackupRFGFootGroup->GetLimbPos());
    m_pRBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_pRBGFootGroup->Clone());
    m_pRBGFootGroup->SetOwner(this);
    m_BackupRBGFootGroup = dynamic_cast<AtomGroup *>(reference.m_BackupRBGFootGroup->Clone());
    m_BackupRBGFootGroup->SetOwner(this);
    m_BackupRBGFootGroup->SetLimbPos(reference.m_BackupRBGFootGroup->GetLimbPos());

	if (reference.m_StrideSound) { m_StrideSound = dynamic_cast<SoundContainer*>(reference.m_StrideSound->Clone()); }

    m_MoveState = reference.m_MoveState;

    for (int side = 0; side < SIDECOUNT; ++side) {
        for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
            m_Paths[side][FGROUND][i].Create(reference.m_Paths[side][FGROUND][i]);
            m_Paths[side][BGROUND][i].Create(reference.m_Paths[side][BGROUND][i]);
        }
    }

    m_GoldInInventoryChunk = reference.m_GoldInInventoryChunk;

    m_DeviceState = reference.m_DeviceState;
    m_SweepState = reference.m_SweepState;
    m_DigState = reference.m_DigState;
    m_JumpState = reference.m_JumpState;
    m_JumpTarget = reference.m_JumpTarget;
    m_JumpingRight = reference.m_JumpingRight;
    m_DigTunnelEndPos = reference.m_DigTunnelEndPos;
    m_SweepCenterAimAngle = reference.m_SweepCenterAimAngle;
    m_SweepRange = reference.m_SweepRange;
    m_AimRangeUpperLimit = reference.m_AimRangeUpperLimit;
    m_AimRangeLowerLimit = reference.m_AimRangeLowerLimit;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ACrab::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "Turret") {
        Turret iniDefinedObject;
        reader >> &iniDefinedObject;
        SetTurret(dynamic_cast<Turret *>(iniDefinedObject.Clone()));
    } else if (propName == "Jetpack") {
        AEmitter iniDefinedObject;
        reader >> &iniDefinedObject;
        SetJetpack(dynamic_cast<AEmitter *>(iniDefinedObject.Clone()));
    } else if (propName == "JumpTime") {
        reader >> m_JetTimeTotal;
        m_JetTimeTotal *= 1000;
    } else if (propName == "LFGLeg" || propName == "LeftFGLeg") {
        Leg iniDefinedObject;
        reader >> &iniDefinedObject;
        SetLeftFGLeg(dynamic_cast<Leg *>(iniDefinedObject.Clone()));
    } else if (propName == "LBGLeg" || propName == "LeftFGLeg") {
        Leg iniDefinedObject;
        reader >> &iniDefinedObject;
        SetLeftBGLeg(dynamic_cast<Leg *>(iniDefinedObject.Clone()));
    } else if (propName == "RFGLeg" || propName == "LeftFGLeg") {
        Leg iniDefinedObject;
        reader >> &iniDefinedObject;
        SetRightFGLeg(dynamic_cast<Leg *>(iniDefinedObject.Clone()));
    } else if (propName == "RBGLeg" || propName == "LeftFGLeg") {
        Leg iniDefinedObject;
        reader >> &iniDefinedObject;
        SetRightBGLeg(dynamic_cast<Leg *>(iniDefinedObject.Clone()));
    } else if (propName == "LFootGroup" || propName == "LeftFootGroup") {
        delete m_pLFGFootGroup;
        delete m_pLBGFootGroup;
        m_pLFGFootGroup = new AtomGroup();
        m_pLBGFootGroup = new AtomGroup();
        reader >> m_pLFGFootGroup;
        m_pLBGFootGroup->Create(*m_pLFGFootGroup);
        m_pLFGFootGroup->SetOwner(this);
        m_pLBGFootGroup->SetOwner(this);
        m_BackupLFGFootGroup = new AtomGroup(*m_pLFGFootGroup);
        m_BackupLFGFootGroup->RemoveAllAtoms();
        m_BackupLBGFootGroup = new AtomGroup(*m_BackupLFGFootGroup);
    } else if (propName == "RFootGroup" || propName == "RightFootGroup") {
        delete m_pRFGFootGroup;
        delete m_pRBGFootGroup;
        m_pRFGFootGroup = new AtomGroup();
        m_pRBGFootGroup = new AtomGroup();
        reader >> m_pRFGFootGroup;
        m_pRBGFootGroup->Create(*m_pRFGFootGroup);
        m_pRFGFootGroup->SetOwner(this);
        m_pRBGFootGroup->SetOwner(this);
        m_BackupRFGFootGroup = new AtomGroup(*m_pRFGFootGroup);
        m_BackupRFGFootGroup->RemoveAllAtoms();
        m_BackupRBGFootGroup = new AtomGroup(*m_BackupRFGFootGroup);
    } else if (propName == "StrideSound") {
		m_StrideSound = new SoundContainer;
        reader >> m_StrideSound;
    } else if (propName == "LStandLimbPath" || propName == "LeftStandLimbPath") {
        reader >> m_Paths[LEFTSIDE][FGROUND][STAND];
    } else if (propName == "LWalkLimbPath" || propName == "LeftWalkLimbPath") {
        reader >> m_Paths[LEFTSIDE][FGROUND][WALK];
    } else if (propName == "LDislodgeLimbPath" || propName == "LeftDislodgeLimbPath") {
        reader >> m_Paths[LEFTSIDE][FGROUND][DISLODGE];
    } else if (propName == "RStandLimbPath" || propName == "RightStandLimbPath") {
        reader >> m_Paths[RIGHTSIDE][FGROUND][STAND];
    } else if (propName == "RWalkLimbPath" || propName == "RightWalkLimbPath") {
        reader >> m_Paths[RIGHTSIDE][FGROUND][WALK];
    } else if (propName == "RDislodgeLimbPath" || propName == "RightDislodgeLimbPath") {
        reader >> m_Paths[RIGHTSIDE][FGROUND][DISLODGE];
    } else if (propName == "AimRangeUpperLimit") {
        reader >> m_AimRangeUpperLimit;
    } else if (propName == "AimRangeLowerLimit") {
        reader >> m_AimRangeLowerLimit;
    } else {
        return Actor::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ACrab with a Writer for
//                  later recreation with Create(Reader &reader);

int ACrab::Save(Writer &writer) const
{
    Actor::Save(writer);

    writer.NewProperty("Turret");
    writer << m_pTurret;
    writer.NewProperty("Jetpack");
    writer << m_pJetpack;
    writer.NewProperty("JumpTime");
    // Convert to seconds
    writer << m_JetTimeTotal / 1000;
    writer.NewProperty("LFGLeg");
    writer << m_pLFGLeg;
    writer.NewProperty("LBGLeg");
    writer << m_pLBGLeg;
    writer.NewProperty("RFGLeg");
    writer << m_pRFGLeg;
    writer.NewProperty("RBGLeg");
    writer << m_pRBGLeg;
    writer.NewProperty("LFGFootGroup");
    writer << m_pLFGFootGroup;
    writer.NewProperty("LBGFootGroup");
    writer << m_pLBGFootGroup;
    writer.NewProperty("RFGFootGroup");
    writer << m_pRFGFootGroup;
    writer.NewProperty("RBGFootGroup");
    writer << m_pRBGFootGroup;
    writer.NewProperty("StrideSound");
    writer << m_StrideSound;

    writer.NewProperty("LStandLimbPath");
    writer << m_Paths[LEFTSIDE][FGROUND][STAND];
    writer.NewProperty("LWalkLimbPath");
    writer << m_Paths[LEFTSIDE][FGROUND][WALK];
    writer.NewProperty("LDislodgeLimbPath");
    writer << m_Paths[LEFTSIDE][FGROUND][DISLODGE];
    writer.NewProperty("RStandLimbPath");
    writer << m_Paths[RIGHTSIDE][FGROUND][STAND];
    writer.NewProperty("RWalkLimbPath");
    writer << m_Paths[RIGHTSIDE][FGROUND][WALK];
    writer.NewProperty("RDislodgeLimbPath");
    writer << m_Paths[RIGHTSIDE][FGROUND][DISLODGE];

    writer.NewProperty("AimRangeUpperLimit");
    writer << m_AimRangeUpperLimit;
    writer.NewProperty("AimRangeLowerLimit");
    writer << m_AimRangeLowerLimit;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ACrab object.

void ACrab::Destroy(bool notInherited)
{
    delete m_pLFGFootGroup;
    delete m_pLBGFootGroup;
    delete m_pRFGFootGroup;
    delete m_pRBGFootGroup;

	delete m_StrideSound;
//    for (deque<LimbPath *>::iterator itr = m_WalkPaths.begin();
//         itr != m_WalkPaths.end(); ++itr)
//        delete *itr;

    if (!notInherited)
        Actor::Destroy();
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this Actor and all its carried
//                  gold and inventory.

float ACrab::GetTotalValue(int nativeModule, float foreignMult) const
{
    float totalValue = Actor::GetTotalValue(nativeModule, foreignMult);

    return totalValue;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetCPUPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' brain, or equivalent.

Vector ACrab::GetCPUPos() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->GetMountedMO())
        return m_pTurret->GetMountedMO()->GetPos();

    return m_Pos;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.

Vector ACrab::GetEyePos() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
        return m_pTurret->GetMountedDevice()->GetPos();

    return m_Pos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetTurret(Turret *newTurret) {
    if (newTurret == nullptr) {
        if (m_pTurret && m_pTurret->IsAttached()) { RemoveAttachable(m_pTurret); }
        m_pTurret = nullptr;
    } else {
        if (m_pTurret && m_pTurret->IsAttached()) { RemoveAttachable(m_pTurret); }
        m_pTurret = newTurret;
        AddAttachable(newTurret);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newTurret->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Turret *castedAttachable = dynamic_cast<Turret *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetTurret");
            dynamic_cast<ACrab *>(parent)->SetTurret(castedAttachable);
        }});

        if (m_pTurret->HasNoSetDamageMultiplier()) { m_pTurret->SetDamageMultiplier(5.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetJetpack(AEmitter *newJetpack) {
    if (newJetpack == nullptr) {
        if (m_pJetpack && m_pJetpack->IsAttached()) { RemoveAttachable(m_pJetpack); }
        m_pJetpack = nullptr;
    } else {
        if (m_pJetpack && m_pJetpack->IsAttached()) { RemoveAttachable(m_pJetpack); }
        m_pJetpack = newJetpack;
        AddAttachable(newJetpack);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newJetpack->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEmitter *castedAttachable = dynamic_cast<AEmitter *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetJetpack");
            dynamic_cast<ACrab *>(parent)->SetJetpack(castedAttachable);
        }});

        if (m_pJetpack->HasNoSetDamageMultiplier()) { m_pJetpack->SetDamageMultiplier(0.0F); }
        m_pJetpack->SetApplyTransferredForcesAtOffset(false);
        m_pJetpack->SetDeleteWhenRemovedFromParent(true);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetLeftFGLeg(Leg *newLeg) {
    if (newLeg == nullptr) {
        if (m_pLFGLeg && m_pLFGLeg->IsAttached()) { RemoveAttachable(m_pLFGLeg); }
        m_pLFGLeg = nullptr;
    } else {
        if (m_pLFGLeg && m_pLFGLeg->IsAttached()) { RemoveAttachable(m_pLFGLeg); }
        m_pLFGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftFGLeg");
            dynamic_cast<ACrab *>(parent)->SetLeftFGLeg(castedAttachable);
        }});

        if (m_pLFGLeg->HasNoSetDamageMultiplier()) { m_pLFGLeg->SetDamageMultiplier(1.0F); }
        m_pLFGLeg->SetInheritsHFlipped(-1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetLeftBGLeg(Leg *newLeg) {
    if (newLeg == nullptr) {
        if (m_pLBGLeg && m_pLBGLeg->IsAttached()) { RemoveAttachable(m_pLBGLeg); }
        m_pLBGLeg = nullptr;
    } else {
        if (m_pLBGLeg && m_pLBGLeg->IsAttached()) { RemoveAttachable(m_pLBGLeg); }
        m_pLBGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetLeftBGLeg");
            dynamic_cast<ACrab *>(parent)->SetLeftBGLeg(castedAttachable);
        }});

        if (m_pLBGLeg->HasNoSetDamageMultiplier()) { m_pLBGLeg->SetDamageMultiplier(1.0F); }
        m_pLBGLeg->SetInheritsHFlipped(-1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetRightFGLeg(Leg *newLeg) {
    if (newLeg == nullptr) {
        if (m_pRFGLeg && m_pRFGLeg->IsAttached()) { RemoveAttachable(m_pRFGLeg); }
        m_pRFGLeg = nullptr;
    } else {
        if (m_pRFGLeg && m_pRFGLeg->IsAttached()) { RemoveAttachable(m_pRFGLeg); }
        m_pRFGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightFGLeg");
            dynamic_cast<ACrab *>(parent)->SetRightFGLeg(castedAttachable);
        }});

        if (m_pRFGLeg->HasNoSetDamageMultiplier()) { m_pRFGLeg->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetRightBGLeg(Leg *newLeg) {
    if (newLeg == nullptr) {
        if (m_pRBGLeg && m_pRBGLeg->IsAttached()) { RemoveAttachable(m_pRBGLeg); }
        m_pRBGLeg = nullptr;
    } else {
        if (m_pRBGLeg && m_pRBGLeg->IsAttached()) { RemoveAttachable(m_pRBGLeg); }
        m_pRBGLeg = newLeg;
        AddAttachable(newLeg);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newLeg->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Leg *castedAttachable = dynamic_cast<Leg *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetRightBGLeg");
            dynamic_cast<ACrab *>(parent)->SetRightBGLeg(castedAttachable);
        }});

        if (m_pRBGLeg->HasNoSetDamageMultiplier()) { m_pRBGLeg->SetDamageMultiplier(1.0F); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.

bool ACrab::CollideAtPoint(HitData &hd)
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

bool ACrab::OnBounce(const Vector &pos)
{
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.

bool ACrab::OnSink(const Vector &pos)
{
    return false;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddPieMenuSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all slices this needs on a pie menu.

bool ACrab::AddPieMenuSlices(PieMenuGUI *pPieMenu)
{
	PieSlice reloadSlice("Reload", PieSlice::PieSliceIndex::PSI_RELOAD, PieSlice::SliceDirection::UP);
    pPieMenu->AddSlice(reloadSlice);

	PieSlice sentryAISlice("Sentry AI Mode", PieSlice::PieSliceIndex::PSI_SENTRY, PieSlice::SliceDirection::DOWN);
    pPieMenu->AddSlice(sentryAISlice);

    if (!HasObjectInGroup("Turrets"))
    {
	    PieSlice aiModeSlice("Go-To AI Mode", PieSlice::PieSliceIndex::PSI_GOTO, PieSlice::SliceDirection::DOWN);
        pPieMenu->AddSlice(aiModeSlice);
    }

	PieSlice patrolAISlice("Patrol AI Mode", PieSlice::PieSliceIndex::PSI_PATROL, PieSlice::SliceDirection::DOWN);
	pPieMenu->AddSlice(patrolAISlice);

    if (!HasObjectInGroup("Turrets"))
    {
	    PieSlice formSquadSlice("Form Squad", PieSlice::PieSliceIndex::PSI_FORMSQUAD, PieSlice::SliceDirection::UP);
        pPieMenu->AddSlice(formSquadSlice);
    }

//    pPieMenu->AddSlice(PieSlice("Gold Dig AI Mode", PieMenuGUI::PSI_GOLDDIG, PieSlice::DOWN));

    Actor::AddPieMenuSlices(pPieMenu);

    // Add any custom slices from a currently held device
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
        m_pTurret->GetMountedDevice()->AddPieMenuSlices(pPieMenu);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HandlePieCommand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles and does whatever a specific activated Pie Menu slice does to
//                  this.

bool ACrab::HandlePieCommand(PieSlice::PieSliceIndex pieSliceIndex)
{
    if (pieSliceIndex != PieSlice::PieSliceIndex::PSI_NONE)
    {
        if (pieSliceIndex == PieSlice::PieSliceIndex::PSI_RELOAD)
            m_Controller.SetState(WEAPON_RELOAD);
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
//        else if (pieSliceIndex == PieMenuGUI::PSI_GOLDDIG)
//            m_AIMode = AIMODE_GOLDDIG;
        else
            return Actor::HandlePieCommand(pieSliceIndex);
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the turret, if anything. OWNERSHIP IS NOT TRANSFERRED!

MovableObject * ACrab::GetEquippedItem() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        return m_pTurret->GetMountedDevice();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held device's current mag is empty on
//                  ammo or not.

bool ACrab::FirearmIsReady() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
        if (pWeapon && pWeapon->GetRoundInMagCount() != 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.

bool ACrab::FirearmIsEmpty() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
        if (pWeapon && pWeapon->GetRoundInMagCount() == 0)
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.

bool ACrab::FirearmNeedsReload() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
        if (pWeapon && pWeapon->NeedsReloading())
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.

bool ACrab::FirearmIsSemiAuto() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
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

void ACrab::ReloadFirearm()
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
        if (pWeapon)
            pWeapon->Reload();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the currently held device's delay between pulling the trigger
//                  and activating.

int ACrab::FirearmActivationDelay() const
{
    // Check if the currently held device is already the desired type
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());
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

bool ACrab::IsWithinRange(Vector &point) const
{
    if (m_SharpAimMaxedOut)
        return true;

    Vector diff = g_SceneMan.ShortestDistance(m_Pos, point, false);
    float distance = diff.GetMagnitude();

    // Really close!
    if (distance <= m_CharHeight)
        return true;

    // Start with the default aim distance
    float range = m_AimDistance;

    // Add the sharp range of the equipped weapon
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        range += m_pTurret->GetMountedDevice()->GetSharpLength() * m_SharpAimProgress;
    }

    return distance <= range;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.

bool ACrab::Look(float FOVSpread, float range)
{
    if (!g_SceneMan.AnythingUnseen(m_Team))
        return false;

    // Set the length of the look vector
    float aimDistance = m_AimDistance + range;
    Vector aimPos = GetCPUPos();

    // If aiming down the barrel, look through that
    if (m_Controller.IsState(AIM_SHARP) && m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        aimPos = m_pTurret->GetMountedDevice()->GetPos();
        aimDistance += m_pTurret->GetMountedDevice()->GetSharpLength();
    }
    // If just looking, use the sensors on the turret instead
    else if (m_pTurret && m_pTurret->IsAttached())
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
// Virtual method:  LookForMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an MO detecting ray in the direction of where the head is looking
//                  at the time. Factors including head rotation, sharp aim mode, and
//                  other variables determine how this ray is cast.

MovableObject * ACrab::LookForMOs(float FOVSpread, unsigned char ignoreMaterial, bool ignoreAllTerrain)
{
    MovableObject *pSeenMO = 0;
    Vector aimPos = m_Pos;
    float aimDistance = m_AimDistance + g_FrameMan.GetPlayerScreenWidth() * 0.51;   // Set the length of the look vector

    // If aiming down the barrel, look through that
    if (m_Controller.IsState(AIM_SHARP) && m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        aimPos = m_pTurret->GetMountedDevice()->GetPos();
        aimDistance += m_pTurret->GetMountedDevice()->GetSharpLength();
    }
    // If just looking, use the sensors on the turret instead
    else if (m_pTurret && m_pTurret->IsAttached())
        aimPos = GetEyePos();
    // If no turret...
    else
        aimPos = GetCPUPos();

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
// Method:          UpdateMovePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the path to move along to the currently set movetarget.

bool ACrab::UpdateMovePath()
{
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
// Virtual method:  UpdateAI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' AI state. Supposed to be done every frame that this has
//                  a CAI controller controlling it.

void ACrab::UpdateAI()
{
    Actor::UpdateAI();

    Vector cpuPos = GetCPUPos();
    MovableObject *pSeenMO = 0;
    Actor *pSeenActor = 0;

    ///////////////////////////////////////////////
    // React to relevant AlarmEvents

    const list<AlarmEvent> &events = g_MovableMan.GetAlarmEvents();
    if (!events.empty())
    {
        Vector alarmVec;
        Vector sensorPos = GetEyePos();
        for (list<AlarmEvent>::const_iterator aeItr = events.begin(); aeItr != events.end(); ++aeItr)
        {
            // Caused by some other team's activites - alarming!
            if (aeItr->m_Team != m_Team)
            {
                // See how far away the alarm situation is
                alarmVec = g_SceneMan.ShortestDistance(sensorPos, aeItr->m_ScenePos);
                // Only react if the alarm is within range and this is perceptive enough to hear it
                if (alarmVec.GetLargest() <= aeItr->m_Range * m_Perceptiveness)
                {
	 	    Vector zero;
					
                    // Now check if we have line of sight to the alarm point
                    // Don't check all the way to the target, we are checking for no obstacles, and target will be an abstacle in itself
                    if (g_SceneMan.CastObstacleRay(sensorPos, alarmVec * 0.9, zero, zero, m_RootMOID, IgnoresWhichTeam(), g_MaterialGrass, 5) < 0)
                    {
                        // If this is the same alarm as last, then don't repeat the signal
                        if (g_SceneMan.ShortestDistance(m_LastAlarmPos, aeItr->m_ScenePos).GetLargest() > 10)
                        {
                            // Yes! WE ARE ALARMED!
                            AlarmPoint(aeItr->m_ScenePos);
                            break;
                        }
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
/*
        // If we're unarmed, hit the deck!
        if (!EquipFirearm() && !EquipThrowable() && !EquipDiggingTool())
        {
            m_Controller.SetState(BODY_CROUCH, true);
            // Also hit the deck completely and crawl away, not jsut sit down in place
            m_Controller.SetState(m_HFlipped ? MOVE_LEFT : MOVE_RIGHT, true);
        }
*/
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
        if (m_UpdateMovePath || (m_ProgressTimer.IsPastSimMS(10000) && m_DeviceState != DIGGING) || (m_MovePath.empty() && m_MoveVector.GetLargest() < m_CharHeight * 0.25f))// || (m_MoveVector.GetLargest() > m_CharHeight * 2))// || g_SceneMan.GetScene()->PathFindingUpdated())
        {
            // Also never update while jumping
            if (m_DeviceState != JUMPING)
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
        if ((m_MoveVector.m_X > 0 && m_LateralMoveState == LAT_LEFT) || (m_MoveVector.m_X < 0 && m_LateralMoveState == LAT_RIGHT) || m_LateralMoveState == LAT_STILL)
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
        // Just set up the closest brain target and switch to GOTO mode
        Actor *pTargetBrain = g_MovableMan.GetClosestBrainActor(m_Team == 0 ? 1 : 0, m_Pos);
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
/*
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
*/
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
            if (pSeenActor && pSeenActor->GetTeam() != m_Team)
            {
                // Start aiming toward that target
                m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetPos();
                m_DeviceState = AIMING;
                m_FireTimer.Reset();
            }
        }
    }
    // Digging
    else if (m_DeviceState == DIGGING)
    {
        m_DeviceState = SCANNING;
        m_DigState = NOTDIGGING;
/*
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
                    // If we have cleared the buried path segment, advance to the next
                    if (!g_SceneMan.CastStrengthRay(m_PrevPathTarget, g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget), 5, Vector(), 1, g_MaterialDoor))
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
                        if (!g_SceneMan.CastStrengthRay(m_PrevPathTarget, g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget), 5, Vector(), 1, g_MaterialDoor))
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
*/
    }
    // Look for, aim at, and fire upon enemy Actors
    else if (m_DeviceState == SCANNING)
    {
        if (m_SweepState == NOSWEEP)
            m_SweepState = SWEEPINGUP;

        // Try to switch to, and if necessary, reload a firearm when we are scanning
        // Reload if necessary
        if (FirearmNeedsReload())
            m_Controller.SetState(WEAPON_RELOAD, true);

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
            if (pSeenActor && pSeenActor->GetTeam() != m_Team)
            {
                // Start aiming toward that target
                m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetPos();
                m_DeviceState = AIMING;
                m_FireTimer.Reset();
            }
        }
    }
    // Aiming toward spotted target to confirm enemy presence
    else if (m_DeviceState == AIMING)
    {
        // Aim carefully!
        m_Controller.SetState(AIM_SHARP, true);

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
            m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetPos();

            // Is the seen target within range? Keep aiming
            if (IsWithinRange(m_SeenTargetPos) || m_FireTimer.IsPastSimMS(2500))
            {
                // ENEMY AIMED AT and within good range - FIRE!
                m_DeviceState = FIRING;
                m_FireTimer.Reset();
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
            m_SeenTargetPos = g_SceneMan.GetLastRayHitPos();//pSeenActor->GetPos();
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
            else if (m_JumpTimer.IsPastSimMS(5000))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
        // Got the height, now wait until we crest the top and start falling again
        if (m_JumpState == APEXJUMP)
        {
			Vector notUsed;
			
            m_PointingTarget = m_JumpTarget;

            // We are falling again, and we can still see the target! start adjusting our aim and jet nozzle forward
            if (m_Vel.m_Y > 4.0 && !g_SceneMan.CastStrengthRay(cpuPos, m_JumpTarget - cpuPos, 5, notUsed, 3))
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
                m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
            // If we've fallen below the target again, then abort the jump
            else if (cpuPos.m_Y > m_JumpTarget.m_Y && g_SceneMan.CastStrengthRay(cpuPos, g_SceneMan.ShortestDistance(cpuPos, m_JumpTarget), 5, notUsed, 3))
            {
                // Set the move target back to the ledge, to undo any checked off points we may have seen while hovering oer teh edge
                m_MoveTarget = m_JumpTarget;
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
                m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
        // We are high and falling again, now go forward to land on top of the ledge
        if (m_JumpState == LANDJUMP)
        {
			Vector notUsed;
			
            m_PointingTarget = m_JumpTarget;

            // Burn the jetpack for a short while to get forward momentum, but not too much
//            if (!m_JumpTimer.IsPastSimMS(500))
                m_Controller.SetState(BODY_JUMP, true);

            // If we've fallen below the target again, then abort the jump
            // If we're flying past the target too, end the jump
            // Lastly, if we're flying way over the target again, just cut the jets!
            if (m_JumpTimer.IsPastSimMS(3500) || (cpuPos.m_Y > m_JumpTarget.m_Y && g_SceneMan.CastStrengthRay(cpuPos, m_JumpTarget - cpuPos, 5, notUsed, 3)) ||
                (m_JumpingRight && m_Pos.m_X > m_JumpTarget.m_X) || (!m_JumpingRight && m_Pos.m_X < m_JumpTarget.m_X) || (cpuPos.m_Y < m_JumpTarget.m_Y - m_CharHeight))
            {
                m_JumpState = NOTJUMPING;
                m_ObstacleState = PROCEEDING;
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
                m_DeviceState = SCANNING;
                m_JumpTimer.Reset();
            }
        }
    }
    // Not in a jump yet, so check for conditions to trigger a jump
    // Also if the movetarget is szzero, probably first frame , but don't try to chase it
    else if (!m_MoveTarget.IsZero())
    {
        // UPWARD JUMP TRIGGERINGS if it's a good time to jump up to a ledge
        if ((-m_MoveVector.m_Y > m_CharHeight * 0.66))// && (fabs(m_MoveVector.m_X) < m_CharHeight))
        {
			Vector notUsed;
			
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
            int dip = m_MoveTarget.m_Y - m_PrevPathTarget.m_Y;
            // See if the next few path points dip steeply
            for (int i = 0; i < 3 && dip < m_CharHeight && pItr != m_MovePath.end(); ++i)
            {
                ++pItr;
                if (pItr == m_MovePath.end())
                    break;
                dip += (*pItr).m_Y - (*prevItr).m_Y;
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
				
				Vector notUsed;
				
                // The rise is high enough to warrant looking across the trench for obstacles in the way of a jump
                if (rise >= m_CharHeight && !g_SceneMan.CastStrengthRay(cpuPos, Vector((*pItr).m_X - cpuPos.m_X, 0), 5, notUsed, 3))
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
    }

    ////////////////////////////////////////
    // If falling, use jetpack to land as softly as possible

    // If the height is more than the character's height, do something to soften the landing!
    float thrustLimit = m_CharHeight;

    // If we're already firing jetpack, then see if it's time to stop
    if (m_ObstacleState == SOFTLANDING && (m_Vel.m_Y < 4.0 || GetAltitude(thrustLimit, 5) < thrustLimit))
    {
        m_ObstacleState = PROCEEDING;
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
            obstructionMOID = obstructionMOID == g_NoMOID ? g_SceneMan.CastMORay(GetCPUPos(), lookRayDown, m_MOID, IgnoresWhichTeam(), g_MaterialGrass, false, 6) : obstructionMOID;
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

/* Can't dig, yet, so this is moot
        // Detect MATERIAL blocking the path and start digging through it!
        Vector pathSegRay(g_SceneMan.ShortestDistance(m_PrevPathTarget, m_MoveTarget));
        Vector obstaclePos;
        if (m_TeamBlockState != BLOCKED && m_DeviceState != DIGGING && g_SceneMan.CastStrengthRay(m_PrevPathTarget, pathSegRay, 5, obstaclePos, 1, g_MaterialDoor))
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
                m_DigState = NOTDIGGING;
        }
*/
    }
    // Can't be obstructed if we're not going forward
    else
        m_TeamBlockState = NOTBLOCKED;

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
        else if (m_DeviceState == DIGGING)
        {
            // Ok we're actually stuck, so backtrack
            if (m_StuckTimer.IsPastSimMS(5000))
            {
                m_ObstacleState = BACKSTEPPING;
                m_StuckTimer.Reset();
            }
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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this ACrab. Supposed to be done every frame.

void ACrab::Update()
{
    float deltaTime = g_TimerMan.GetDeltaTimeSecs();
    float mass = GetMass();

    // Set Default direction of all the paths!
    for (int side = 0; side < SIDECOUNT; ++side)
    {
        for (int layer = 0; layer < LAYERCOUNT; ++layer)
        {
            m_Paths[side][layer][WALK].SetHFlip(m_HFlipped);
            m_Paths[side][layer][STAND].SetHFlip(m_HFlipped);
        }
    }

    ////////////////////////////////////
    // Jetpack activation and blast direction

    if (m_pJetpack && m_pJetpack->IsAttached())
    {
        // Start Jetpack burn
        if (m_Controller.IsState(BODY_JUMPSTART) && m_JetTimeLeft > 0)
        {
            m_pJetpack->TriggerBurst();
            // This is to make sure se get loose from being stuck
            m_ForceDeepCheck = true;
            m_pJetpack->EnableEmission(true);
            // Quadruple this for the burst
            m_JetTimeLeft -= g_TimerMan.GetDeltaTimeMS() * 10;
            if (m_JetTimeLeft < 0)
                m_JetTimeLeft = 0;
        }
        // Jetpack is burning
        else if (m_Controller.IsState(BODY_JUMP) && m_JetTimeLeft > 0)
        {
            m_pJetpack->EnableEmission(true);
            // Jetpacks are noisy!
            m_pJetpack->AlarmOnEmit(m_Team);
            // Deduct from the jetpack time
            m_JetTimeLeft -= g_TimerMan.GetDeltaTimeMS();
            m_MoveState = JUMP;
        }
        // Jetpack is off/turning off
        else
        {
            m_pJetpack->EnableEmission(false);
            if (m_MoveState == JUMP)
                m_MoveState = STAND;

            // Replenish the jetpack time, twice as fast
            m_JetTimeLeft += g_TimerMan.GetDeltaTimeMS() * 2;
            if (m_JetTimeLeft >= m_JetTimeTotal)
                m_JetTimeLeft = m_JetTimeTotal;
        }

        // Direct the jetpack nozzle according to movement stick if analog input is present
        if (m_Controller.GetAnalogMove().GetMagnitude() > 0.1)
        {
            float jetAngle = m_Controller.GetAnalogMove().GetAbsRadAngle() + c_PI;
            // Clamp the angle to 45 degrees down cone with centr straight down on body
            if (jetAngle > c_PI + c_HalfPI + c_QuarterPI)// - c_SixteenthPI)
                jetAngle = c_PI + c_HalfPI + c_QuarterPI;// - c_SixteenthPI;
            else if (jetAngle < c_PI + c_QuarterPI)// + c_SixteenthPI)
                jetAngle = c_PI + c_QuarterPI;// + c_SixteenthPI;

            m_pJetpack->SetEmitAngle(FacingAngle(jetAngle));
        }
        // Or just use the aim angle if we're getting digital input
        else
        {
            float jetAngle = m_AimAngle >= 0 ? (m_AimAngle * 0.25) : 0;
            jetAngle = c_PI + c_QuarterPI + c_EighthPI + jetAngle;
            // Don't need to use FacingAngle on this becuase it's already applied to the AimAngle since last update.
            m_pJetpack->SetEmitAngle(jetAngle);
        }
    }

    ////////////////////////////////////
    // Movement direction

    if (m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT) || m_MoveState == JUMP)
    {
        if (m_MoveState != JUMP)
        {
            // Restart the stride if we're just starting to walk or crawl
            if (m_MoveState != WALK)
            {
                m_StrideStart[LEFTSIDE] = true;
                m_StrideStart[RIGHTSIDE] = true;
                MoveOutOfTerrain(g_MaterialGrass);
            }

            m_MoveState = WALK;

            for (int side = 0; side < SIDECOUNT; ++side)
            {
                m_Paths[side][FGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
                m_Paths[side][BGROUND][m_MoveState].SetSpeed(m_Controller.IsState(MOVE_FAST) ? FAST : NORMAL);
            }
        }

        // Walk backwards if the aiming is done in the opposite direction of travel
        if (fabs(m_Controller.GetAnalogAim().m_X) > 0.1)
        {
            // Walk backwards if necessary
            for (int side = 0; side < SIDECOUNT; ++side)
            {
                m_Paths[side][FGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
                m_Paths[side][BGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
            }
        }
        // Flip if we're moving in the opposite direction
        else if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped))
        {
            m_HFlipped = !m_HFlipped;
//                // Instead of simply carving out a silhouette of the now flipped actor, isntead disable any atoms which are embedded int eh terrain until they emerge again
//                m_ForceDeepCheck = true;
            m_CheckTerrIntersection = true;
            MoveOutOfTerrain(g_MaterialGrass);
            for (int side = 0; side < SIDECOUNT; ++side)
            {
                for (int layer = 0; layer < LAYERCOUNT; ++layer)
                {
                    m_Paths[side][layer][m_MoveState].SetHFlip(m_HFlipped);
                    m_Paths[side][layer][WALK].Terminate();
                    m_Paths[side][layer][STAND].Terminate();
                }
                m_StrideStart[side] = true;
            }
        }
    }
    else
        m_MoveState = STAND;

    ////////////////////////////////////
    // Reload held MO, if applicable

    if (m_pTurret && m_pTurret->IsAttached())
    {
        HeldDevice *pDevice = m_pTurret->GetMountedDevice();

        // Holds device, check if we are commanded to reload, or do other related stuff
        if (pDevice)
        {
            // Only reload if no other pickuppable item is in reach
            if (!pDevice->IsFull() && m_Controller.IsState(WEAPON_RELOAD))
            {
                pDevice->Reload();
				if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }

                // Interrupt sharp aiming
                m_SharpAimTimer.Reset();
                m_SharpAimProgress = 0;
            }
        }
    }

    ////////////////////////////////////
    // Aiming

    // Get rotation angle of crab
    float rotAngle = GetRotAngle();

    // Adjust AimRange limits to crab rotation
    float adjustedAimRangeUpperLimit = (m_HFlipped) ? m_AimRangeUpperLimit - rotAngle : m_AimRangeUpperLimit + rotAngle;
    float adjustedAimRangeLowerLimit = (m_HFlipped) ? -m_AimRangeLowerLimit - rotAngle : -m_AimRangeLowerLimit + rotAngle;

    if (m_Controller.IsState(AIM_UP))
    {
        // Set the timer to some base number so we don't get a sluggish feeling at start of aim
        if (m_AimState != AIMUP)
            m_AimTmr.SetElapsedSimTimeMS(150);
        m_AimState = AIMUP;
        m_AimAngle += m_Controller.IsState(AIM_SHARP) ? MIN(m_AimTmr.GetElapsedSimTimeMS() * 0.00005, 0.05) : MIN(m_AimTmr.GetElapsedSimTimeMS() * 0.00015, 0.1);
    }
    else if (m_Controller.IsState(AIM_DOWN))
    {
        // Set the timer to some base number so we don't get a sluggish feeling at start of aim
        if (m_AimState != AIMDOWN)
            m_AimTmr.SetElapsedSimTimeMS(150);
        m_AimState = AIMDOWN;
        m_AimAngle -= m_Controller.IsState(AIM_SHARP) ? MIN(m_AimTmr.GetElapsedSimTimeMS() * 0.00005, 0.05) : MIN(m_AimTmr.GetElapsedSimTimeMS() * 0.00015, 0.1);
    }
    // Analog aim
    else if (m_Controller.GetAnalogAim().GetMagnitude() > 0.1)
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
            // Instead of simply carving out a silhouette of the now flipped actor, isntead disable any atoms which are embedded int eh terrain until they emerge again
            //m_ForceDeepCheck = true;
            m_CheckTerrIntersection = true;
            MoveOutOfTerrain(g_MaterialGrass);
            for (int side = 0; side < SIDECOUNT; ++side)
            {
                for (int layer = 0; layer < LAYERCOUNT; ++layer)
                {
                    m_Paths[side][layer][m_MoveState].SetHFlip(m_HFlipped);
                    m_Paths[side][layer][WALK].Terminate();
                    m_Paths[side][layer][STAND].Terminate();
                }
                m_StrideStart[side] = true;
            }
        }
        // Correct angle based on flip
        m_AimAngle = FacingAngle(m_AimAngle);
    }
    else
        m_AimState = AIMSTILL;

    // Clamp aim angle so it's within adjusted limit ranges, for all control types
    Clamp(m_AimAngle, adjustedAimRangeUpperLimit, adjustedAimRangeLowerLimit);

    //////////////////////////////
    // Sharp aim calculation

// TODO: make the delay data driven by both the actor and the device!
    // 
    if (m_Controller.IsState(AIM_SHARP) && m_MoveState == STAND && m_Vel.GetMagnitude() < 5.0)
    {
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
        if (aimMag < 0.1)
            aimMag = 1.0;

        if (m_SharpAimTimer.IsPastSimMS(m_SharpAimDelay))
        {
            // Only go slower outward
            if (m_SharpAimProgress < aimMag)
                m_SharpAimProgress += (aimMag - m_SharpAimProgress) * 0.035;
            else
                m_SharpAimProgress = aimMag;
        }
        else
            m_SharpAimProgress = 0;
    }
    else
    {
        m_SharpAimProgress = 0;
        m_SharpAimTimer.Reset();
    }

    ////////////////////////////////////
    // Fire/Activate held devices

    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        // Activate held device, if a device is held.
        m_pTurret->GetMountedDevice()->SetSharpAim(m_SharpAimProgress);
        if (m_Controller.IsState(WEAPON_FIRE))
            m_pTurret->GetMountedDevice()->Activate();
        else
            m_pTurret->GetMountedDevice()->Deactivate();
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


    ///////////////////////////////////////////////////
    // Travel the limb AtomGroup:s

    if (m_Status == STABLE)
    {
        // This exists to support disabling foot collisions if the limbpath has that flag set.
        if ((m_pLFGFootGroup->GetAtomCount() == 0 && m_BackupLFGFootGroup->GetAtomCount() > 0) != m_Paths[LEFTSIDE][FGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupLFGFootGroup->SetLimbPos(m_pLFGFootGroup->GetLimbPos());
            std::swap(m_pLFGFootGroup, m_BackupLFGFootGroup);
        }
        if ((m_pLBGFootGroup->GetAtomCount() == 0 && m_BackupLBGFootGroup->GetAtomCount() > 0) != m_Paths[LEFTSIDE][BGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupLBGFootGroup->SetLimbPos(m_pLBGFootGroup->GetLimbPos());
            std::swap(m_pLBGFootGroup, m_BackupLBGFootGroup);
        }
        if ((m_pRFGFootGroup->GetAtomCount() == 0 && m_BackupRFGFootGroup->GetAtomCount() > 0) != m_Paths[RIGHTSIDE][FGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupRFGFootGroup->SetLimbPos(m_pRFGFootGroup->GetLimbPos());
            std::swap(m_pRFGFootGroup, m_BackupRFGFootGroup);
        }
        if ((m_pRBGFootGroup->GetAtomCount() == 0 && m_BackupRBGFootGroup->GetAtomCount() > 0) != m_Paths[RIGHTSIDE][BGROUND][m_MoveState].FootCollisionsShouldBeDisabled()) {
            m_BackupRBGFootGroup->SetLimbPos(m_pRBGFootGroup->GetLimbPos());
            std::swap(m_pRBGFootGroup, m_BackupRBGFootGroup);
        }

        // WALKING
        if (m_MoveState == WALK)
        {
            for (int side = 0; side < SIDECOUNT; ++side)
                for (int layer = 0; layer < LAYERCOUNT; ++layer)
                    m_Paths[side][layer][STAND].Terminate();

            float LFGLegProg = m_Paths[LEFTSIDE][FGROUND][WALK].GetRegularProgress();
            float LBGLegProg = m_Paths[LEFTSIDE][BGROUND][WALK].GetRegularProgress();
            float RFGLegProg = m_Paths[RIGHTSIDE][FGROUND][WALK].GetRegularProgress();
            float RBGLegProg = m_Paths[RIGHTSIDE][BGROUND][WALK].GetRegularProgress();

            bool playStride = false;

            // Make sure we are starting a stride if we're basically stopped
            if (fabs(m_Vel.GetLargest()) < 0.25)
                m_StrideStart[LEFTSIDE] = true;

            //////////////////
            // LEFT LEGS

            if (m_pLFGLeg && (!m_pLBGLeg || (!(m_Paths[LEFTSIDE][FGROUND][WALK].PathEnded() && LBGLegProg < 0.5) || m_StrideStart[LEFTSIDE])))
            {
//                m_StrideStart[LEFTSIDE] = false;
                m_StrideTimer[LEFTSIDE].Reset();
                m_pLFGFootGroup->PushAsLimb(m_Pos +
                                            RotateOffset(m_pLFGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[LEFTSIDE][FGROUND][WALK],
//                                            mass,
                                            deltaTime,
                                            &playStride);
            }

            if (m_pLBGLeg && (!m_pLFGLeg || !(m_Paths[LEFTSIDE][BGROUND][WALK].PathEnded() && LFGLegProg < 0.5)))
            {
                m_StrideStart[LEFTSIDE] = false;
                m_StrideTimer[LEFTSIDE].Reset();
                m_pLBGFootGroup->PushAsLimb(m_Pos +
                                            RotateOffset(m_pLBGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[LEFTSIDE][BGROUND][WALK],
//                                            mass,
                                            deltaTime);
            }

            // Restart the left stride if the current one seems to be taking too long
            if (m_StrideTimer[LEFTSIDE].IsPastSimMS(m_Paths[LEFTSIDE][FGROUND][WALK].GetTotalPathTime()))
                m_StrideStart[LEFTSIDE] = true;

            ///////////////////
            // RIGHT LEGS

            if (m_pRFGLeg && (!m_pRBGLeg || !(m_Paths[RIGHTSIDE][FGROUND][WALK].PathEnded() && RBGLegProg < 0.5)))
            {
                m_StrideStart[RIGHTSIDE] = false;
                m_StrideTimer[RIGHTSIDE].Reset();
                m_pRFGFootGroup->PushAsLimb(m_Pos +
                                            RotateOffset(m_pRFGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[RIGHTSIDE][FGROUND][WALK],
//                                            mass,
                                            deltaTime,
                                            &playStride);
            }

            if (m_pRBGLeg && (!m_pRFGLeg || (!(m_Paths[RIGHTSIDE][BGROUND][WALK].PathEnded() && RFGLegProg < 0.5) || m_StrideStart[RIGHTSIDE])))
            {
//                m_StrideStart[RIGHTSIDE] = false;
                m_StrideTimer[RIGHTSIDE].Reset();
                m_pRBGFootGroup->PushAsLimb(m_Pos +
                                            RotateOffset(m_pRBGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[RIGHTSIDE][BGROUND][WALK],
//                                            mass,
                                            deltaTime);
            }

            // Restart the right stride if the current one seems to be taking too long
            if (m_StrideTimer[RIGHTSIDE].IsPastSimMS(m_Paths[RIGHTSIDE][FGROUND][WALK].GetTotalPathTime()))
                m_StrideStart[RIGHTSIDE] = true;

            // Play the stride sound, if applicable
            if (playStride)
				if (m_StrideSound) { m_StrideSound->Play(m_Pos); }
        }
        // JUMPING
        else if ((m_pRFGLeg || m_pRBGLeg) && m_MoveState == JUMP)
        {
/*
            if (m_pRFGLeg && (!m_Paths[FGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal))
            {
                m_pRFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRFGLeg->GetParentOffset()),
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[FGROUND][m_MoveState],
    //                                  mass / 2,
                                      deltaTime);
            }
            if (m_pRBGLeg && (!m_Paths[BGROUND][m_MoveState].PathEnded() || m_JetTimeLeft == m_JetTimeTotal))
            {
                m_pRBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRBGLeg->GetParentOffset()),
                                      m_Vel,
                                      m_Rotation,
                                      m_Paths[BGROUND][m_MoveState],
    //                                mass / 2,
                                      deltaTime);
            }

            if (m_JetTimeLeft <= 0)
            {
                m_MoveState = STAND;
                m_Paths[FGROUND][JUMP].Terminate();
                m_Paths[BGROUND][JUMP].Terminate();
                m_Paths[FGROUND][STAND].Terminate();
                m_Paths[BGROUND][STAND].Terminate();
                m_Paths[FGROUND][WALK].Terminate();
                m_Paths[BGROUND][WALK].Terminate();
            }
*/
        }
        // STANDING
        else if (m_pLFGLeg || m_pLBGLeg || m_pRFGLeg || m_pRBGLeg)
        {
            for (int side = 0; side < SIDECOUNT; ++side)
                for (int layer = 0; layer < LAYERCOUNT; ++layer)
                    m_Paths[side][layer][WALK].Terminate();

            if (m_pLFGLeg)
                m_pLFGFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pLFGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[LEFTSIDE][FGROUND][STAND],
//                                            mass / 2,
                                            deltaTime);

            if (m_pLBGLeg)
                m_pLBGFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pLBGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[LEFTSIDE][BGROUND][STAND],
//                                            mass / 2,
                                            deltaTime);

            if (m_pRFGLeg)
                m_pRFGFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pRFGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[RIGHTSIDE][FGROUND][STAND],
//                                            mass / 2,
                                            deltaTime);

            if (m_pRBGLeg)
                m_pRBGFootGroup->PushAsLimb(m_Pos.GetFloored() + RotateOffset(m_pRBGLeg->GetParentOffset()),
                                            m_Vel,
                                            m_Rotation,
                                            m_Paths[RIGHTSIDE][BGROUND][STAND],
//                                            mass / 2,
                                            deltaTime);

        }
    }
    // Not stable/standing, so make sure the end of limbs are moving around limply in a ragdoll fashion
    else
    {
// TODO: Make the limb atom groups fly around and react to terrain, without getting stuck etc
        bool wrapped = false;
        Vector limbPos;
        if (m_pLFGLeg)
        {
//            m_pLFGFootGroup->SetLimbPos(m_pLFGLeg->GetAnklePos(), m_HFlipped);
            m_pLFGFootGroup->FlailAsLimb(m_Pos,
                                         m_pLFGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                         m_pLFGLeg->GetMaxLength(),
                                         g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                         m_AngularVel,
                                         m_pLFGLeg->GetMass(),
                                         g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pLBGLeg)
        {
//            m_pLBGFootGroup->SetLimbPos(m_pLBGLeg->GetAnklePos(), m_HFlipped);
            m_pLBGFootGroup->FlailAsLimb(m_Pos,
                                         m_pLBGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                         m_pLBGLeg->GetMaxLength(),
                                         g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                         m_AngularVel,
                                         m_pLBGLeg->GetMass(),
                                         g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pRFGLeg)
        {
//            m_pRFGFootGroup->SetLimbPos(m_pRFGLeg->GetAnklePos(), m_HFlipped);
            m_pRFGFootGroup->FlailAsLimb(m_Pos,
                                         m_pRFGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                         m_pRFGLeg->GetMaxLength(),
                                         g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                         m_AngularVel,
                                         m_pRFGLeg->GetMass(),
                                         g_TimerMan.GetDeltaTimeSecs());
        }
        if (m_pRBGLeg)
        {
//            m_pRBGFootGroup->SetLimbPos(m_pRBGLeg->GetAnklePos(), m_HFlipped);
            m_pRBGFootGroup->FlailAsLimb(m_Pos,
                                         m_pRBGLeg->GetParentOffset().GetXFlipped(m_HFlipped) * m_Rotation,
                                         m_pRBGLeg->GetMaxLength(),
                                         g_SceneMan.GetGlobalAcc() * g_TimerMan.GetDeltaTimeSecs(),
                                         m_AngularVel,
                                         m_pRBGLeg->GetMass(),
                                         g_TimerMan.GetDeltaTimeSecs());
        }
    }

    /////////////////////////////////
    // Manage Attachable:s
    if (m_pTurret && m_pTurret->IsAttached()) {
        m_pTurret->SetMountedDeviceRotOffset((m_AimAngle * static_cast<float>(GetFlipFactor())) - m_Rotation.GetRadAngle());
    }

    if (m_pLFGLeg && m_pLFGLeg->IsAttached()) {
        m_pLFGLeg->EnableIdle(m_Status != UNSTABLE);
        m_pLFGLeg->SetTargetPosition(m_pLFGFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pLBGLeg && m_pLBGLeg->IsAttached()) {
        m_pLBGLeg->EnableIdle(m_Status != UNSTABLE);
        m_pLBGLeg->SetTargetPosition(m_pLBGFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pRFGLeg && m_pRFGLeg->IsAttached()) {
        m_pRFGLeg->EnableIdle(m_Status != UNSTABLE);
        m_pRFGLeg->SetTargetPosition(m_pRFGFootGroup->GetLimbPos(m_HFlipped));
    }

    if (m_pRBGLeg && m_pRBGLeg->IsAttached()) {
        m_pRBGLeg->EnableIdle(m_Status != UNSTABLE);
        m_pRBGLeg->SetTargetPosition(m_pRBGFootGroup->GetLimbPos(m_HFlipped));
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

    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        float maxLength = m_pTurret->GetMountedDevice()->GetSharpLength();

        // Use a non-terrain check ray to cap the magnitude, so we can't see into objects etc
        if (m_SharpAimProgress > 0)
        {
			Vector notUsed;
            Vector sharpAimVector(maxLength, 0);
            sharpAimVector *= aimMatrix;

            // See how far along the sharp aim vector there is opaque air
//            float result = g_SceneMan.CastNotMaterialRay(m_pLFGLeg->GetMountedDevice()->GetMuzzlePos(), sharpAimVector, g_MaterialAir, 5);
            float result = g_SceneMan.CastObstacleRay(m_pTurret->GetMountedDevice()->GetMuzzlePos(), sharpAimVector, notUsed, notUsed, GetRootID(), IgnoresWhichTeam(), g_MaterialAir, 5);
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
    if (m_Vel.GetMagnitude() > 10.0)
        m_ViewPoint += m_Vel * 6;

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

    ////////////////////////////////////////
    // Balance stuff

    // Get the rotation in radians.
    float rot = m_Rotation.GetRadAngle();
//        rot = fabs(rot) < c_QuarterPI ? rot : (rot > 0 ? c_QuarterPI : -c_QuarterPI);
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
        // Upright body posture
        // Break the spring if close to target angle.
        if (fabs(rot) > (c_HalfPI - c_SixteenthPI))
            m_AngularVel -= rot * 0.5;//fabs(rot);
        else if (fabs(m_AngularVel) > 0.3)
            m_AngularVel *= 0.85;
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

    // Losing all limbs should kill... eventually
    if (!m_pLFGLeg && !m_pLBGLeg && !m_pRFGLeg && !m_pRBGLeg && m_Status != DYING && m_Status != DEAD)
        m_Health -= 0.1;

    /////////////////////////////////////////
    // Misc.

//    m_DeepCheck = true/*m_Status == DEAD*/;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    Actor::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawHandAndFootGroupVisualizations()) {
        m_pLFGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pLBGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pRFGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
        m_pRBGFootGroup->Draw(pTargetBitmap, targetPos, true, 13);
    }

    if (mode == g_DrawColor && !onlyPhysical && g_SettingsMan.DrawLimbPathVisualizations()) {
        m_Paths[LEFTSIDE][WALK]->Draw(pTargetBitmap, targetPos, 122);
        m_Paths[RIGHTSIDE][WALK]->Draw(pTargetBitmap, targetPos, 122);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void ACrab::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible)
        return;

    // Only do HUD if on a team
    if (m_Team < 0)
        return;

    // Only draw if the team viewing this is on the same team OR has seen the space where this is located
    int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam)
    {
        if (g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam))
            return;
    }

    Actor::DrawHUD(pTargetBitmap, targetPos, whichScreen);
/*
// TODO: REMOVE< THIS IS TEMP
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
// TODO: REMOVE THIS IS TEMP
*/

    // Player AI drawing

    // Device aiming reticule
    if (m_Controller.IsState(AIM_SHARP) && m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
        m_pTurret->GetMountedDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen, m_Controller.IsPlayerControlled());

    //////////////////////////////////////
    // Draw stat info HUD
    char str[64];

    GUIFont *pSymbolFont = g_FrameMan.GetLargeFont();
    GUIFont *pSmallFont = g_FrameMan.GetSmallFont();

    // Only show extra HUD if this guy is controlled by the same player that this screen belongs to
    if (m_Controller.IsPlayerControlled() && g_ActivityMan.GetActivity()->ScreenOfPlayer(m_Controller.GetPlayer()) == whichScreen && pSmallFont && pSymbolFont)
    {
        AllegroBitmap allegroBitmap(pTargetBitmap);

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
        if (m_pJetpack && m_pJetpack->IsAttached() && m_MoveState == JUMP)
        {
            float mass = GetMass();
// TODO: Don't hardcode the mass indicator! Figure out how to calculate the jetpack threshold values
            str[0] = mass < 135 ? -31 : (mass < 160 ? -30 : -29); str[1] = 0;
            // Do the blinky blink
            if ((str[0] == -29 || str[0] == -30) && m_IconBlinkTimer.AlternateSim(250))
                str[0] = -28;
            pSymbolFont->DrawAligned(&allegroBitmap, drawPos.m_X - 11, drawPos.m_Y + m_HUDStack, str, GUIFont::Centre);

            float jetTimeRatio = m_JetTimeLeft / m_JetTimeTotal;
// TODO: Don't hardcode this shit
            char gaugeColor = jetTimeRatio > 0.6 ? 149 : (jetTimeRatio > 0.3 ? 77 : 13);
            rectfill(pTargetBitmap, drawPos.m_X, drawPos.m_Y + m_HUDStack + 6, drawPos.m_X + (16 * jetTimeRatio), drawPos.m_Y + m_HUDStack + 7, gaugeColor);
//                    rect(pTargetBitmap, drawPos.m_X, drawPos.m_Y + m_HUDStack - 2, drawPos.m_X + 24, drawPos.m_Y + m_HUDStack - 4, 238);
//                    std::snprintf(str, sizeof(str), "%.0f Kg", mass);
//                    pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);

            m_HUDStack += -10;
        }
        // Held-related GUI stuff
        else if (m_pTurret && m_pTurret->IsAttached())
        {
            HDFirearm *pHeldFirearm = dynamic_cast<HDFirearm *>(m_pTurret->GetMountedDevice());

            // Ammo
            if (pHeldFirearm)
            {
                str[0] = -56; str[1] = 0;
                pSymbolFont->DrawAligned(&allegroBitmap, drawPos.m_X - 10, drawPos.m_Y + m_HUDStack, str, GUIFont::Left);
                if (pHeldFirearm->IsReloading())
                    std::snprintf(str, sizeof(str), "%s", "Reloading...");
                else if (pHeldFirearm->GetRoundInMagCount() >= 0)
                    std::snprintf(str, sizeof(str), "%i", pHeldFirearm->GetRoundInMagCount());
                else
                    std::snprintf(str, sizeof(str), "%s", "INF");
                pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Left);

                m_HUDStack += -10;
            }
        }
        else
        {
            std::snprintf(str, sizeof(str), "NO TURRET!");
            pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
            m_HUDStack += -9;
        }


		// Print aim angle and rot angle stoff
		/*{
			std::snprintf(str, sizeof(str), "Aim %.2f Rot %.2f Lim %.2f", m_AimAngle, GetRotAngle(), m_AimRange + GetRotAngle());
			pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X - 0, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);

			m_HUDStack += -10;
		}*/

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
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get walking limb path speed for the specified preset.

float ACrab::GetLimbPathSpeed(int speedPreset) const
{
	return m_Paths[LEFTSIDE][FGROUND][WALK].GetSpeed(speedPreset);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set walking limb path speed for the specified preset.

void ACrab::SetLimbPathSpeed(int speedPreset, float speed)
{
	m_Paths[LEFTSIDE][FGROUND][WALK].OverrideSpeed(speedPreset, speed);
	m_Paths[RIGHTSIDE][FGROUND][WALK].OverrideSpeed(speedPreset, speed);

	m_Paths[LEFTSIDE][BGROUND][WALK].OverrideSpeed(speedPreset, speed);
	m_Paths[RIGHTSIDE][BGROUND][WALK].OverrideSpeed(speedPreset, speed);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 

float ACrab::GetLimbPathPushForce() const
{
	return m_Paths[LEFTSIDE][FGROUND][WALK].GetDefaultPushForce();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with. 

void ACrab::SetLimbPathPushForce(float force)
{
	m_Paths[LEFTSIDE][FGROUND][WALK].OverridePushForce(force);
	m_Paths[RIGHTSIDE][FGROUND][WALK].OverridePushForce(force);

	m_Paths[LEFTSIDE][BGROUND][WALK].OverridePushForce(force);
	m_Paths[RIGHTSIDE][BGROUND][WALK].OverridePushForce(force);
}



} // namespace RTE
