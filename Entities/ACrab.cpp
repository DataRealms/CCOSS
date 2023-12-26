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
#include "AEJetpack.h"
#include "HDFirearm.h"
#include "Scene.h"
#include "SettingsMan.h"
#include "PresetMan.h"
#include "FrameMan.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "tracy/Tracy.hpp"

namespace RTE {

ConcreteClassInfo(ACrab, Actor, 20);


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
    m_pJetpack = nullptr;
    m_MoveState = STAND;
    m_StrideFrame = false;
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
    m_LockMouseAimInput = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.

int ACrab::Create()
{
	// Read all the properties
	if (Actor::Create() < 0) {
		return -1;
	}

	if (m_AIMode == Actor::AIMODE_NONE) {
		m_AIMode = Actor::AIMODE_BRAINHUNT;
	}

    // Create the background paths copied from the foreground ones which were already read in
    for (int side = 0; side < SIDECOUNT; ++side)
    {
        for (int i = 0; i < MOVEMENTSTATECOUNT; ++i)
        {
            m_Paths[side][BGROUND][i].Destroy();
            m_Paths[side][BGROUND][i].Create(m_Paths[side][FGROUND][i]);
        }
    }

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
    if (reference.m_pLBGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLBGLeg->GetUniqueID()); }
    if (reference.m_pRBGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRBGLeg->GetUniqueID()); }
    if (reference.m_pJetpack) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pJetpack->GetUniqueID()); }
    if (reference.m_pTurret) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pTurret->GetUniqueID()); }
    if (reference.m_pLFGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pLFGLeg->GetUniqueID()); }
    if (reference.m_pRFGLeg) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pRFGLeg->GetUniqueID()); }

    Actor::Create(reference);

    //Note - hardcoded attachable copying is organized based on desired draw order here.
    if (reference.m_pLBGLeg) { SetLeftBGLeg(dynamic_cast<Leg *>(reference.m_pLBGLeg->Clone())); }
    if (reference.m_pRBGLeg) { SetRightBGLeg(dynamic_cast<Leg *>(reference.m_pRBGLeg->Clone())); }
    if (reference.m_pJetpack) { SetJetpack(dynamic_cast<AEJetpack *>(reference.m_pJetpack->Clone())); }
    if (reference.m_pTurret) { SetTurret(dynamic_cast<Turret *>(reference.m_pTurret->Clone())); }
    if (reference.m_pLFGLeg) { SetLeftFGLeg(dynamic_cast<Leg *>(reference.m_pLFGLeg->Clone())); }
    if (reference.m_pRFGLeg) { SetRightFGLeg(dynamic_cast<Leg *>(reference.m_pRFGLeg->Clone())); }

	AtomGroup *atomGroupToUseAsFootGroupLFG = reference.m_pLFGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pLFGFootGroup->Clone()) : m_pLFGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupLFG, "Failed to fallback to using LFGFoot AtomGroup as LFGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a LFGFootGroup or LFGLeg Foot attachable!");

	AtomGroup *atomGroupToUseAsFootGroupLBG = reference.m_pLBGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pLBGFootGroup->Clone()) : m_pLBGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupLBG, "Failed to fallback to using LBGFoot AtomGroup as LBGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a LBGFootGroup or LBGLeg Foot attachable!");

	AtomGroup *atomGroupToUseAsFootGroupRFG = reference.m_pRFGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pRFGFootGroup->Clone()) : m_pRFGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupRFG, "Failed to fallback to using RFGFoot AtomGroup as RFGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a RFGFootGroup or RFGLeg Foot attachable!");

	AtomGroup *atomGroupToUseAsFootGroupRBG = reference.m_pRBGFootGroup ? dynamic_cast<AtomGroup *>(reference.m_pRBGFootGroup->Clone()) : m_pRBGLeg->GetFootGroupFromFootAtomGroup();
	RTEAssert(atomGroupToUseAsFootGroupRBG, "Failed to fallback to using RBGFoot AtomGroup as RBGFootGroup in preset " + this->GetModuleAndPresetName() + "!\nPlease define a RBGFootGroup or RBGLeg Foot attachable!");

    m_pLFGFootGroup = atomGroupToUseAsFootGroupLFG;
    m_pLFGFootGroup->SetOwner(this);
    m_BackupLFGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupLFG->Clone());
	m_BackupLFGFootGroup->RemoveAllAtoms();
    m_BackupLFGFootGroup->SetOwner(this);
    m_BackupLFGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupLFG->GetLimbPos());
    m_pLBGFootGroup = atomGroupToUseAsFootGroupLBG;
    m_pLBGFootGroup->SetOwner(this);
    m_BackupLBGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupLBG->Clone());
	m_BackupLBGFootGroup->RemoveAllAtoms();
    m_BackupLBGFootGroup->SetOwner(this);
    m_BackupLBGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupLFG->GetLimbPos());
    m_pRFGFootGroup = atomGroupToUseAsFootGroupRFG;
    m_pRFGFootGroup->SetOwner(this);
    m_BackupRFGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupRFG->Clone());
	m_BackupRFGFootGroup->RemoveAllAtoms();
    m_BackupRFGFootGroup->SetOwner(this);
    m_BackupRFGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupLFG->GetLimbPos());
    m_pRBGFootGroup = atomGroupToUseAsFootGroupRBG;
    m_pRBGFootGroup->SetOwner(this);
    m_BackupRBGFootGroup = dynamic_cast<AtomGroup *>(atomGroupToUseAsFootGroupRBG->Clone());
	m_BackupRBGFootGroup->RemoveAllAtoms();
    m_BackupRBGFootGroup->SetOwner(this);
    m_BackupRBGFootGroup->SetLimbPos(atomGroupToUseAsFootGroupLFG->GetLimbPos());

	if (reference.m_StrideSound) { m_StrideSound = dynamic_cast<SoundContainer*>(reference.m_StrideSound->Clone()); }

    m_MoveState = reference.m_MoveState;

    for (int side = 0; side < SIDECOUNT; ++side) {
        for (int i = 0; i < MOVEMENTSTATECOUNT; ++i) {
            m_Paths[side][FGROUND][i].Create(reference.m_Paths[side][FGROUND][i]);
            m_Paths[side][BGROUND][i].Create(reference.m_Paths[side][BGROUND][i]);
        }
    }

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
    m_LockMouseAimInput = reference.m_LockMouseAimInput;

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
    StartPropertyList(return Actor::ReadProperty(propName, reader));

    MatchProperty("Turret", { SetTurret(dynamic_cast<Turret *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchProperty("Jetpack", { SetJetpack(dynamic_cast<AEJetpack *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchForwards("LFGLeg") MatchProperty("LeftFGLeg", { SetLeftFGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchForwards("LBGLeg") MatchProperty("LeftBGLeg", { SetLeftBGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchForwards("RFGLeg") MatchProperty("RightFGLeg", { SetRightFGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchForwards("RBGLeg") MatchProperty("RightBGLeg", { SetRightBGLeg(dynamic_cast<Leg *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchForwards("LFootGroup") MatchProperty("LeftFootGroup", {
        delete m_pLFGFootGroup;
        delete m_pLBGFootGroup;
        delete m_BackupLFGFootGroup;
        delete m_BackupLBGFootGroup;
        m_pLFGFootGroup = new AtomGroup();
        m_pLBGFootGroup = new AtomGroup();
        reader >> m_pLFGFootGroup;
        m_pLBGFootGroup->Create(*m_pLFGFootGroup);
        m_pLFGFootGroup->SetOwner(this);
        m_pLBGFootGroup->SetOwner(this);
        m_BackupLFGFootGroup = new AtomGroup(*m_pLFGFootGroup);
        m_BackupLFGFootGroup->RemoveAllAtoms();
        m_BackupLBGFootGroup = new AtomGroup(*m_BackupLFGFootGroup);
    }); 
    MatchForwards("RFootGroup") MatchProperty("RightFootGroup", {
        delete m_pRFGFootGroup;
        delete m_pRBGFootGroup;
        delete m_BackupRFGFootGroup;
        delete m_BackupRBGFootGroup;
        m_pRFGFootGroup = new AtomGroup();
        m_pRBGFootGroup = new AtomGroup();
        reader >> m_pRFGFootGroup;
        m_pRBGFootGroup->Create(*m_pRFGFootGroup);
        m_pRFGFootGroup->SetOwner(this);
        m_pRBGFootGroup->SetOwner(this);
        m_BackupRFGFootGroup = new AtomGroup(*m_pRFGFootGroup);
        m_BackupRFGFootGroup->RemoveAllAtoms();
        m_BackupRBGFootGroup = new AtomGroup(*m_BackupRFGFootGroup);
    }); 
    MatchForwards("LFGFootGroup") MatchProperty("LeftFGFootGroup", {
        delete m_pLFGFootGroup;
        delete m_BackupLFGFootGroup;
        m_pLFGFootGroup = new AtomGroup();
        reader >> m_pLFGFootGroup;
        m_pLFGFootGroup->SetOwner(this);
        m_BackupLFGFootGroup = new AtomGroup(*m_pLFGFootGroup);
        m_BackupLFGFootGroup->RemoveAllAtoms();
    }); 
    MatchForwards("LBGFootGroup") MatchProperty("LeftBGFootGroup", {
        delete m_pLBGFootGroup;
        delete m_BackupLBGFootGroup;
        m_pLBGFootGroup = new AtomGroup();
        reader >> m_pLBGFootGroup;
        m_pLBGFootGroup->SetOwner(this);
        m_BackupLBGFootGroup = new AtomGroup(*m_pLBGFootGroup);
        m_BackupLBGFootGroup->RemoveAllAtoms();
    }); 
    MatchForwards("RFGFootGroup") MatchProperty("RightFGFootGroup", {
        delete m_pRFGFootGroup;
        delete m_BackupRFGFootGroup;
        m_pRFGFootGroup = new AtomGroup();
        reader >> m_pRFGFootGroup;
        m_pRFGFootGroup->SetOwner(this);
        m_BackupRFGFootGroup = new AtomGroup(*m_pRFGFootGroup);
        m_BackupRFGFootGroup->RemoveAllAtoms();
    }); 
    MatchForwards("RBGFootGroup") MatchProperty("RightBGFootGroup", {
        delete m_pRBGFootGroup;
        delete m_BackupRBGFootGroup;
        m_pRBGFootGroup = new AtomGroup();
        reader >> m_pRBGFootGroup;
        m_pRBGFootGroup->SetOwner(this);
        m_BackupRBGFootGroup = new AtomGroup(*m_pRBGFootGroup);
        m_BackupRBGFootGroup->RemoveAllAtoms();
    });
    MatchProperty("StrideSound", {
		m_StrideSound = new SoundContainer;
        reader >> m_StrideSound;
    }); 
    MatchForwards("LStandLimbPath") MatchProperty("LeftStandLimbPath", { reader >> m_Paths[LEFTSIDE][FGROUND][STAND]; });
    MatchForwards("LWalkLimbPath") MatchProperty("LeftWalkLimbPath", { reader >> m_Paths[LEFTSIDE][FGROUND][WALK]; });
    MatchForwards("LDislodgeLimbPath") MatchProperty("LeftDislodgeLimbPath", { reader >> m_Paths[LEFTSIDE][FGROUND][DISLODGE]; });
    MatchForwards("RStandLimbPath") MatchProperty("RightStandLimbPath", { reader >> m_Paths[RIGHTSIDE][FGROUND][STAND]; });
    MatchForwards("RWalkLimbPath") MatchProperty("RightWalkLimbPath", { reader >> m_Paths[RIGHTSIDE][FGROUND][WALK]; });
    MatchForwards("RDislodgeLimbPath") MatchProperty("RightDislodgeLimbPath", { reader >> m_Paths[RIGHTSIDE][FGROUND][DISLODGE]; });
    MatchProperty("AimRangeUpperLimit", { reader >> m_AimRangeUpperLimit; });
    MatchProperty("AimRangeLowerLimit", { reader >> m_AimRangeLowerLimit; });
    MatchProperty("LockMouseAimInput", { reader >> m_LockMouseAimInput; });
    
    EndPropertyList;
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
    writer.NewProperty("LockMouseAimInput");
    writer << m_LockMouseAimInput;

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
        return m_pTurret->GetFirstMountedDevice()->GetPos();

    return m_Pos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ACrab::SetTurret(Turret *newTurret) {
    if (m_pTurret && m_pTurret->IsAttached()) { RemoveAndDeleteAttachable(m_pTurret); }
    if (newTurret == nullptr) {
        m_pTurret = nullptr;
    } else {
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

void ACrab::SetJetpack(AEJetpack *newJetpack) {
    if (m_pJetpack && m_pJetpack->IsAttached()) { RemoveAndDeleteAttachable(m_pJetpack); }
    if (newJetpack == nullptr) {
        m_pJetpack = nullptr;
    } else {
        m_pJetpack = newJetpack;
        AddAttachable(newJetpack);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newJetpack->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            AEJetpack *castedAttachable = dynamic_cast<AEJetpack *>(attachable);
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
    if (m_pLFGLeg && m_pLFGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pLFGLeg); }
    if (newLeg == nullptr) {
        m_pLFGLeg = nullptr;
    } else {
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
    if (m_pLBGLeg && m_pLBGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pLBGLeg); }
    if (newLeg == nullptr) {
        m_pLBGLeg = nullptr;
    } else {
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
    if (m_pRFGLeg && m_pRFGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pRFGLeg); }
    if (newLeg == nullptr) {
        m_pRFGLeg = nullptr;
    } else {
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
    if (m_pRBGLeg && m_pRBGLeg->IsAttached()) { RemoveAndDeleteAttachable(m_pRBGLeg); }
    if (newLeg == nullptr) {
        m_pRBGLeg = nullptr;
    } else {
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

BITMAP * ACrab::GetGraphicalIcon() const {
	return m_GraphicalIcon ? m_GraphicalIcon : (m_pTurret ? m_pTurret->GetSpriteFrame(0) : GetSpriteFrame(0));
}

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ACrab::HandlePieCommand(PieSlice::SliceType pieSliceIndex) {
    if (pieSliceIndex != PieSlice::SliceType::NoType) {
        if (pieSliceIndex == PieSlice::SliceType::Reload) {
            m_Controller.SetState(WEAPON_RELOAD);
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
        } else {
            return Actor::HandlePieCommand(pieSliceIndex);
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the turret, if anything. OWNERSHIP IS NOT TRANSFERRED!

MovableObject * ACrab::GetEquippedItem() const
{
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        return m_pTurret->GetFirstMountedDevice();
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held device's current mag is empty on
//                  ammo or not.

bool ACrab::FirearmIsReady() const {
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice()) {
        for (const HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
            if (const HDFirearm *mountedFirearm = dynamic_cast<const HDFirearm *>(mountedDevice); mountedFirearm && mountedFirearm->GetRoundInMagCount() != 0) {
                return true;
            }
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.

bool ACrab::FirearmIsEmpty() const {
    return !FirearmIsReady() && m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.

bool ACrab::FirearmsAreFull() const {
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice()) {
        for (const HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
            if (const HDFirearm *mountedFirearm = dynamic_cast<const HDFirearm *>(mountedDevice); mountedFirearm && !mountedFirearm->IsFull()) {
                return false;
            }
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////

bool ACrab::FirearmNeedsReload() const {
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice()) {
        for (const HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
            if (const HDFirearm *mountedFirearm = dynamic_cast<const HDFirearm *>(mountedDevice); mountedFirearm && mountedFirearm->NeedsReloading()) {
                return true;
            }
        }
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
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetFirstMountedDevice());
        return pWeapon && !pWeapon->IsFullAuto();
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  ReloadFirearms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the currently held firearms, if any.
// Arguments:       None.
// Return value:    None.

void ACrab::ReloadFirearms() {
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice()) {
        for (HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
            if (HDFirearm *mountedFirearm = dynamic_cast<HDFirearm *>(mountedDevice)) {
                mountedFirearm->Reload();
            }
        }
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
        HDFirearm *pWeapon = dynamic_cast<HDFirearm *>(m_pTurret->GetFirstMountedDevice());
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
    float sqrDistance = diff.GetSqrMagnitude();

	// Really close!
	if (sqrDistance <= (m_CharHeight * m_CharHeight)) {
		return true;
	}

    // Start with the default aim distance
    float range = m_AimDistance;

    // Add the sharp range of the equipped weapon
    if (m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice())
    {
        range += m_pTurret->GetFirstMountedDevice()->GetSharpLength() * m_SharpAimProgress;
    }

    return sqrDistance <= (range * range);
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
        aimPos = m_pTurret->GetFirstMountedDevice()->GetPos();
        aimDistance += m_pTurret->GetFirstMountedDevice()->GetSharpLength();
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
        aimPos = m_pTurret->GetFirstMountedDevice()->GetPos();
        aimDistance += m_pTurret->GetFirstMountedDevice()->GetSharpLength();
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

void ACrab::OnNewMovePath()
{
    Actor::OnNewMovePath();

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
}

//////////////////////////////////////////////////////////////////////////////////////////

void ACrab::PreControllerUpdate()
{
    ZoneScoped;

    Actor::PreControllerUpdate();

    float deltaTime = g_TimerMan.GetDeltaTimeSecs();
    float mass = GetMass();

	Vector analogAim = m_Controller.GetAnalogAim();
    const float analogAimDeadzone = 0.1F;

    // Set Default direction of all the paths!
    for (int side = 0; side < SIDECOUNT; ++side)
    {
        for (int layer = 0; layer < LAYERCOUNT; ++layer)
        {
            m_Paths[side][layer][WALK].SetHFlip(m_HFlipped);
            m_Paths[side][layer][STAND].SetHFlip(m_HFlipped);
        }
    }

    if (m_pJetpack && m_pJetpack->IsAttached()) {
		m_pJetpack->UpdateBurstState(*this);
	}

    ////////////////////////////////////
    // Movement direction
    const float movementThreshold = 1.0F;
	bool isStill = (m_Vel + m_PrevVel).MagnitudeIsLessThan(movementThreshold);

	// If the pie menu is on, try to preserve whatever move state we had before it going into effect.
	// This is only done for digital input, where the user needs to use the keyboard to choose pie slices.
	// For analog input, this doesn't matter - the mouse or aiming analog stick controls the pie menu.
	bool keepOldState = m_Controller.IsKeyboardOnlyControlled() && m_Controller.IsState(PIE_MENU_ACTIVE);

	if (!keepOldState) {
		if (m_Controller.IsState(MOVE_RIGHT) || m_Controller.IsState(MOVE_LEFT) || m_MoveState == JUMP && m_Status != INACTIVE) {
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

			// Walk backwards if the aiming is already focused in the opposite direction of travel.
			if (std::abs(analogAim.m_X) > 0 || m_Controller.IsState(AIM_SHARP)) {
				for (int side = 0; side < SIDECOUNT; ++side) {
					m_Paths[side][FGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
					m_Paths[side][BGROUND][m_MoveState].SetHFlip(m_Controller.IsState(MOVE_LEFT));
				}
			} else if ((m_Controller.IsState(MOVE_RIGHT) && m_HFlipped) || (m_Controller.IsState(MOVE_LEFT) && !m_HFlipped)) {
				m_HFlipped = !m_HFlipped;
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
		} else {
			m_MoveState = STAND;
		}
	}

    ////////////////////////////////////
    // Reload held MO, if applicable

	if (m_Controller.IsState(WEAPON_RELOAD) && !FirearmsAreFull() && m_Status != INACTIVE) {
        ReloadFirearms();

        if (m_DeviceSwitchSound) { m_DeviceSwitchSound->Play(m_Pos); }

        // Interrupt sharp aiming
        m_SharpAimTimer.Reset();
        m_SharpAimProgress = 0;
    }

    ////////////////////////////////////
    // Aiming

    // Get rotation angle of crab
    float rotAngle = GetRotAngle();

    // Adjust AimRange limits to crab rotation
    float adjustedAimRangeUpperLimit = (m_HFlipped) ? m_AimRangeUpperLimit - rotAngle : m_AimRangeUpperLimit + rotAngle;
    float adjustedAimRangeLowerLimit = (m_HFlipped) ? -m_AimRangeLowerLimit - rotAngle : -m_AimRangeLowerLimit + rotAngle;

	if (m_Controller.IsState(AIM_UP) && m_Status != INACTIVE) {
		// Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMUP) { m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300); }
		m_AimState = AIMUP;
		m_AimAngle += m_Controller.IsState(AIM_SHARP) ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();

	} else if (m_Controller.IsState(AIM_DOWN) && m_Status != INACTIVE) {
		// Set the timer to a base number so we don't get a sluggish feeling at start.
		if (m_AimState != AIMDOWN) { m_AimTmr.SetElapsedSimTimeMS(m_AimState == AIMSTILL ? 150 : 300); }
		m_AimState = AIMDOWN;
		m_AimAngle -= m_Controller.IsState(AIM_SHARP) ? std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00005F, 0.05F) : std::min(static_cast<float>(m_AimTmr.GetElapsedSimTimeMS()) * 0.00015F, 0.15F) * m_Controller.GetDigitalAimSpeed();

	} else if (analogAim.MagnitudeIsGreaterThan(analogAimDeadzone) && m_Status != INACTIVE) {
        // Hack to avoid the GetAbsRadAngle to mangle an aim angle straight down
		if (analogAim.m_X == 0) { analogAim.m_X += 0.01F * GetFlipFactor(); }
        m_AimAngle = analogAim.GetAbsRadAngle();

        // Check for flip change
		if ((analogAim.m_X > 0 && m_HFlipped) || (analogAim.m_X < 0 && !m_HFlipped)) {
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

        // Clamp the analog aim too, so it doesn't feel "sticky" at the edges of the aim limit
        if (m_Controller.IsPlayerControlled() && m_LockMouseAimInput) {
            float mouseAngle = g_UInputMan.AnalogAimValues(m_Controller.GetPlayer()).GetAbsRadAngle();
            Clamp(mouseAngle, FacingAngle(adjustedAimRangeUpperLimit), FacingAngle(adjustedAimRangeLowerLimit));
            g_UInputMan.SetMouseValueAngle(mouseAngle, m_Controller.GetPlayer());
        }
    }
    else
        m_AimState = AIMSTILL;

    // Clamp aim angle so it's within adjusted limit ranges, for all control types
    Clamp(m_AimAngle, adjustedAimRangeUpperLimit, adjustedAimRangeLowerLimit);

    //////////////////////////////
    // Sharp aim calculation

	if (m_Controller.IsState(AIM_SHARP) && m_Status == STABLE && m_Vel.MagnitudeIsLessThan(5.0F)) {
        float aimMag = analogAim.GetMagnitude();
        
		// If aim sharp is being done digitally, then translate to full magnitude.
		if (aimMag < 0.1F) { aimMag = 1.0F; }
		if (m_MoveState == WALK) { aimMag *= 0.3F; }

		if (m_SharpAimTimer.IsPastSimMS(m_SharpAimDelay)) {
			// Only go slower outward.
			if (m_SharpAimProgress < aimMag) {
				m_SharpAimProgress += (aimMag - m_SharpAimProgress) * 0.035F;
			} else {
				m_SharpAimProgress = aimMag;
			}
		} else {
			m_SharpAimProgress *= 0.95F;
		}
	} else {    
		m_SharpAimProgress = std::max(m_SharpAimProgress * 0.95F - 0.1F, 0.0F);
	}

    ////////////////////////////////////
    // Fire/Activate held devices

    if (m_pTurret && m_pTurret->IsAttached() && m_Status != INACTIVE) {
        for (HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
            mountedDevice->SetSharpAim(m_SharpAimProgress);
            if (m_Controller.IsState(WEAPON_FIRE)) {
                mountedDevice->Activate();
				if (mountedDevice->IsEmpty()) { mountedDevice->Reload(); }
            } else {
                mountedDevice->Deactivate();
            }
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


    ///////////////////////////////////////////////////
    // Travel the limb AtomGroup:s

    m_StrideFrame = false;

    if (m_Status == STABLE && !m_LimbPushForcesAndCollisionsDisabled)
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

            bool restarted = false;
			Matrix walkAngle(rotAngle * 0.5F);

			// Make sure we are starting a stride if we're basically stopped.
			if (isStill) { m_StrideStart[LEFTSIDE] = true; }

            //////////////////
            // LEFT LEGS

			if (m_pLFGLeg && (!m_pLBGLeg || (!(m_Paths[LEFTSIDE][FGROUND][WALK].PathEnded() && LBGLegProg < 0.5F) || m_StrideStart[LEFTSIDE]))) {
				m_StrideTimer[LEFTSIDE].Reset();
				m_pLFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pLFGLeg->GetParentOffset()), m_Vel, walkAngle, m_Paths[LEFTSIDE][FGROUND][WALK], deltaTime, &restarted);
			}

			if (m_pLBGLeg) {
				if (!m_pLFGLeg || !(m_Paths[LEFTSIDE][BGROUND][WALK].PathEnded() && LFGLegProg < 0.5F)) {
					m_StrideStart[LEFTSIDE] = false;
					m_StrideTimer[LEFTSIDE].Reset();
					m_pLBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pLBGLeg->GetParentOffset()), m_Vel, walkAngle, m_Paths[LEFTSIDE][BGROUND][WALK], deltaTime);
				} else {
					m_pLBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pLBGLeg->GetParentOffset()), m_pLBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pLBGLeg->GetMass(), deltaTime);
				}
			}

			// Reset the left-side walking stride if it's taking longer than it should.
			if (m_StrideTimer[LEFTSIDE].IsPastSimMS(static_cast<double>(m_Paths[LEFTSIDE][FGROUND][WALK].GetTotalPathTime() * 1.1F))) { m_StrideStart[LEFTSIDE] = true; }

            ///////////////////
            // RIGHT LEGS

			if (m_pRFGLeg) {
				if (!m_pRBGLeg || !(m_Paths[RIGHTSIDE][FGROUND][WALK].PathEnded() && RBGLegProg < 0.5F)) {
					m_StrideStart[RIGHTSIDE] = false;
					m_StrideTimer[RIGHTSIDE].Reset();
					m_pRFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRFGLeg->GetParentOffset()), m_Vel, walkAngle, m_Paths[RIGHTSIDE][FGROUND][WALK], deltaTime, &restarted);
				} else {
					m_pRFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pRFGLeg->GetParentOffset()), m_pRFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pRFGLeg->GetMass(), deltaTime);
				}
			}

			if (m_pRBGLeg && (!m_pRFGLeg || (!(m_Paths[RIGHTSIDE][BGROUND][WALK].PathEnded() && RFGLegProg < 0.5F) || m_StrideStart[RIGHTSIDE]))) {
				m_StrideTimer[RIGHTSIDE].Reset();
				m_pRBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRBGLeg->GetParentOffset()), m_Vel, walkAngle, m_Paths[RIGHTSIDE][BGROUND][WALK], deltaTime);
			}

			// Reset the right-side walking stride if it's taking longer than it should.
			if (m_StrideTimer[RIGHTSIDE].IsPastSimMS(static_cast<double>(m_Paths[RIGHTSIDE][FGROUND][WALK].GetTotalPathTime() * 1.1F))) { m_StrideStart[RIGHTSIDE] = true; }
            
			if (m_StrideSound) {
				m_StrideSound->SetPosition(m_Pos);
				if (m_StrideSound->GetLoopSetting() < 0) {
					if (!m_StrideSound->IsBeingPlayed()) { m_StrideSound->Play(); }
				} else if (restarted) {
					m_StrideSound->Play();
				}
			}

            if (restarted) {
                m_StrideFrame = true;
                RunScriptedFunctionInAppropriateScripts("OnStride");
            }
		} else if (m_pLFGLeg || m_pLBGLeg || m_pRFGLeg || m_pRBGLeg) {
			if (m_MoveState == JUMP) {
				// TODO: Utilize jump paths in an intuitive way?
				if (m_pLFGLeg) { m_pLFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pLFGLeg->GetParentOffset()), m_pLFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pLFGLeg->GetMass(), deltaTime); }
				if (m_pLBGLeg) { m_pLBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pLBGLeg->GetParentOffset()), m_pLBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pLBGLeg->GetMass(), deltaTime); }
				if (m_pRFGLeg) { m_pRFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pRFGLeg->GetParentOffset()), m_pRFGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pRFGLeg->GetMass(), deltaTime); }
				if (m_pRBGLeg) { m_pRBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pRBGLeg->GetParentOffset()), m_pRBGLeg->GetMaxLength(), m_PrevVel, m_AngularVel, m_pRBGLeg->GetMass(), deltaTime); }

				if (m_pJetpack == nullptr || m_pJetpack->IsOutOfFuel()) {
					m_MoveState = STAND;
					m_Paths[LEFTSIDE][FGROUND][JUMP].Terminate();
					m_Paths[LEFTSIDE][BGROUND][JUMP].Terminate();
					m_Paths[LEFTSIDE][FGROUND][STAND].Terminate();
					m_Paths[LEFTSIDE][BGROUND][STAND].Terminate();
					m_Paths[LEFTSIDE][FGROUND][WALK].Terminate();
					m_Paths[LEFTSIDE][BGROUND][WALK].Terminate();
					m_Paths[RIGHTSIDE][FGROUND][JUMP].Terminate();
					m_Paths[RIGHTSIDE][BGROUND][JUMP].Terminate();
					m_Paths[RIGHTSIDE][FGROUND][STAND].Terminate();
					m_Paths[RIGHTSIDE][BGROUND][STAND].Terminate();
					m_Paths[RIGHTSIDE][FGROUND][WALK].Terminate();
					m_Paths[RIGHTSIDE][BGROUND][WALK].Terminate();
				}
			} else {
				for (int side = 0; side < SIDECOUNT; ++side) {
					for (int layer = 0; layer < LAYERCOUNT; ++layer) {
						m_Paths[side][layer][WALK].Terminate();
					}
				}
				if (m_pLFGLeg) { m_pLFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pLFGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[LEFTSIDE][FGROUND][STAND], deltaTime, nullptr, !m_pRFGLeg); }

				if (m_pLBGLeg) { m_pLBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pLBGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[LEFTSIDE][BGROUND][STAND], deltaTime); }

				if (m_pRFGLeg) { m_pRFGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRFGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[RIGHTSIDE][FGROUND][STAND], deltaTime, nullptr, !m_pLFGLeg); }

				if (m_pRBGLeg) { m_pRBGFootGroup->PushAsLimb(m_Pos + RotateOffset(m_pRBGLeg->GetParentOffset()), m_Vel, m_Rotation, m_Paths[RIGHTSIDE][BGROUND][STAND], deltaTime); }
			}
		}
	} else {
		// Not stable/standing, so make sure the end of limbs are moving around limply in a ragdoll fashion.
		// TODO: Make the limb atom groups fly around and react to terrain, without getting stuck etc.
		if (m_pLFGLeg) { m_pLFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pLFGLeg->GetParentOffset()), m_pLFGLeg->GetMaxLength(), m_PrevVel * m_pLFGLeg->GetJointStiffness(), m_AngularVel, m_pLFGLeg->GetMass(), deltaTime); }

		if (m_pLBGLeg) { m_pLBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pLBGLeg->GetParentOffset()), m_pLBGLeg->GetMaxLength(), m_PrevVel * m_pLBGLeg->GetJointStiffness(), m_AngularVel, m_pLBGLeg->GetMass(), deltaTime); }

		if (m_pRFGLeg) { m_pRFGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pRFGLeg->GetParentOffset()), m_pRFGLeg->GetMaxLength(), m_PrevVel * m_pRFGLeg->GetJointStiffness(), m_AngularVel, m_pRFGLeg->GetMass(), deltaTime); }

		if (m_pRBGLeg) { m_pRBGFootGroup->FlailAsLimb(m_Pos, RotateOffset(m_pRBGLeg->GetParentOffset()), m_pRBGLeg->GetMaxLength(), m_PrevVel * m_pRBGLeg->GetJointStiffness(), m_AngularVel, m_pRBGLeg->GetMass(), deltaTime); }
	}
    if (m_MoveState != WALK && m_StrideSound && m_StrideSound->GetLoopSetting() < 0) {
        m_StrideSound->Stop();
    }

    /////////////////////////////////
    // Manage Attachable:s
    if (m_pTurret && m_pTurret->IsAttached()) {
        m_pTurret->SetMountedDeviceRotationOffset((m_AimAngle * GetFlipFactor()) - m_Rotation.GetRadAngle());
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
}

//////////////////////////////////////////////////////////////////////////////////////////

void ACrab::Update() 
{
    ZoneScoped;

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
        float maxLength = m_pTurret->GetFirstMountedDevice()->GetSharpLength();

        // Use a non-terrain check ray to cap the magnitude, so we can't see into objects etc
        if (m_SharpAimProgress > 0)
        {
			Vector notUsed;
            Vector sharpAimVector(maxLength, 0);
            sharpAimVector *= aimMatrix;

            // See how far along the sharp aim vector there is opaque air
//            float result = g_SceneMan.CastNotMaterialRay(m_pLFGLeg->GetFirstMountedDevice()->GetMuzzlePos(), sharpAimVector, g_MaterialAir, 5);
            float result = g_SceneMan.CastObstacleRay(m_pTurret->GetFirstMountedDevice()->GetMuzzlePos(), sharpAimVector, notUsed, notUsed, GetRootID(), IgnoresWhichTeam(), g_MaterialAir, 5);
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
    if (m_Vel.MagnitudeIsGreaterThan(10.0F))
        m_ViewPoint += m_Vel * std::sqrt(m_Vel.GetMagnitude() * 0.1F);

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
		if (m_Status == STABLE) { m_Status = UNSTABLE; }
        m_StableRecoverTimer.Reset();
    }

    // Rotational balancing spring calc
    if (m_Status == STABLE)
    {
        // Upright body posture
		m_AngularVel = m_AngularVel * 0.9F - (rot * 0.3F);
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
        m_Paths[LEFTSIDE][BGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
        m_Paths[LEFTSIDE][FGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
        m_Paths[RIGHTSIDE][BGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
        m_Paths[RIGHTSIDE][FGROUND][WALK].Draw(pTargetBitmap, targetPos, 122);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.

void ACrab::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled) {
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

    // Player AI drawing

	if ((m_Controller.IsState(AIM_SHARP) || (m_Controller.IsPlayerControlled() && !m_Controller.IsState(PIE_MENU_ACTIVE))) && m_pTurret && m_pTurret->IsAttached() && m_pTurret->HasMountedDevice()) {
		m_pTurret->GetFirstMountedDevice()->DrawHUD(pTargetBitmap, targetPos, whichScreen, m_Controller.IsState(AIM_SHARP) && m_Controller.IsPlayerControlled());
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

        // Held-related GUI stuff
		if (m_pTurret) {
            std::string textString;
            for (const HeldDevice *mountedDevice : m_pTurret->GetMountedDevices()) {
                if (const HDFirearm *mountedFirearm = dynamic_cast<const HDFirearm *>(mountedDevice)) {
                    if (!textString.empty()) { textString += " | "; }
					int totalTextWidth = pSmallFont->CalculateWidth(textString);
                    if (mountedFirearm->IsReloading()) {
                        textString += "Reloading";
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + 1 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, drawPos.GetFloorIntX() + 29 + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 14, 245);
						rectfill(pTargetBitmap, drawPos.GetFloorIntX() + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 12, drawPos.GetFloorIntX() + static_cast<int>(28.0F * mountedFirearm->GetReloadProgress() + 0.5F) + totalTextWidth, drawPos.GetFloorIntY() + m_HUDStack + 13, 77);
                    } else {
						textString += mountedFirearm->GetRoundInMagCount() < 0 ? "Infinite" : std::to_string(mountedFirearm->GetRoundInMagCount());
                    }
                }
            }
            if (!textString.empty()) {
                str[0] = -56; str[1] = 0;
                pSymbolFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - 10, drawPos.GetFloorIntY() + m_HUDStack, str, GUIFont::Left);
                pSmallFont->DrawAligned(&allegroBitmap, drawPos.GetFloorIntX() - 0, drawPos.GetFloorIntY() + m_HUDStack + 3, textString, GUIFont::Left);
				m_HUDStack -= 9;
            }
        } else {
            std::snprintf(str, sizeof(str), "NO TURRET!");
            pSmallFont->DrawAligned(&allegroBitmap, drawPos.m_X + 2, drawPos.m_Y + m_HUDStack + 3, str, GUIFont::Centre);
            m_HUDStack += -9;
        }

		if (m_pJetpack && m_Status != INACTIVE && !m_Controller.IsState(PIE_MENU_ACTIVE) && (m_Controller.IsState(BODY_JUMP) || !m_pJetpack->IsFullyFueled())) {
			if (m_pJetpack->GetJetTimeLeft() < 100.0F) {
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
			if (m_pJetpack->GetJetTimeTotal() > 0.0F) {
				float jetTimeRatio = m_pJetpack->GetJetTimeRatio();
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ACrab::WhilePieMenuOpenListener(const PieMenu *pieMenu) {
	int result = Actor::WhilePieMenuOpenListener(pieMenu);

	for (PieSlice *pieSlice : GetPieMenu()->GetPieSlices()) {
		if (pieSlice->GetType() == PieSlice::SliceType::Reload) {
			if (m_pTurret && m_pTurret->HasMountedDevice()) {
				pieSlice->SetDescription("Reload");
				pieSlice->SetEnabled(!FirearmsAreFull());
			} else {
				pieSlice->SetDescription(m_pTurret ? "No Weapons" : "No Turret");
				pieSlice->SetEnabled(false);
			}
			break;
		}
	}
	return result;
}


} // namespace RTE
