//////////////////////////////////////////////////////////////////////////////////////////
// File:            HDFirearm.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the HDFirearm class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "HDFirearm.h"
#include "PresetMan.h"
#include "Magazine.h"
#include "ThrownDevice.h"
#include "MOPixel.h"
#include "Actor.h"

namespace RTE {

ConcreteClassInfo(HDFirearm, HeldDevice, 50)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this HDFirearm, effectively
//                  resetting the members of this abstraction level only.

void HDFirearm::Clear()
{
    m_pMagazineReference = 0;
    m_pMagazine = 0;

    m_pFlash = 0;
    m_PreFireSound = nullptr;
    m_FireSound = nullptr;
    m_FireEchoSound = nullptr;
    m_ActiveSound = nullptr;
    m_DeactivationSound = nullptr;
    m_EmptySound = nullptr;
	m_ReloadStartSound = nullptr;
    m_ReloadEndSound = nullptr;
    m_RateOfFire = 0;
    m_ActivationDelay = 0;
    m_DeactivationDelay = 0;
    m_Reloading = false;
    m_DoneReloading = false;
    m_ReloadTime = 0;
    m_FullAuto = false;
    m_FireIgnoresThis = true;
    m_ShakeRange = 0;
    m_SharpShakeRange = 0;
    m_NoSupportFactor = 0;
    m_ParticleSpreadRange = 0;
    m_ShellSpreadRange = 0;
    m_ShellAngVelRange = 0;
    m_AIFireVel = -1;
    m_AIBulletLifeTime = 0;
    m_AIBulletAccScalar = -1;
    m_LastFireTmr.Reset();
    m_ReloadTmr.Reset();
    m_MuzzleOff.Reset();
    m_EjectOff.Reset();
    m_MagOff.Reset();
    m_FiredOnce = false;
    m_FireFrame = false;
    m_FiredLastFrame = false;
    m_AlreadyClicked = false;
	m_RoundsFired = 0;
	m_IsAnimatedManually = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Round object ready for use.

int HDFirearm::Create()
{
    if (HeldDevice::Create() < 0)
        return -1;

    if (m_pFlash)
        m_pFlash->SetParentOffset(m_MuzzleOff);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a HDFirearm to be identical to another, by deep copy.

int HDFirearm::Create(const HDFirearm &reference) {
    if (reference.m_pMagazine) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pMagazine->GetUniqueID());
        SetMagazine(dynamic_cast<Magazine *>(reference.m_pMagazine->Clone()));
    }
    if (reference.m_pFlash) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFlash->GetUniqueID());
        SetFlash(dynamic_cast<Attachable *>(reference.m_pFlash->Clone()));
    }
    HeldDevice::Create(reference);

    m_pMagazineReference = reference.m_pMagazineReference;
	if (reference.m_PreFireSound) { m_PreFireSound = dynamic_cast<SoundContainer *>(reference.m_PreFireSound->Clone()); }
	if (reference.m_FireSound) { m_FireSound = dynamic_cast<SoundContainer *>(reference.m_FireSound->Clone()); }
	if (reference.m_FireEchoSound) { m_FireEchoSound = dynamic_cast<SoundContainer*>(reference.m_FireEchoSound->Clone()); }
	if (reference.m_ActiveSound) { m_ActiveSound = dynamic_cast<SoundContainer *>(reference.m_ActiveSound->Clone()); }
	if (reference.m_DeactivationSound) { m_DeactivationSound = dynamic_cast<SoundContainer *>(reference.m_DeactivationSound->Clone()); }
	if (reference.m_EmptySound) { m_EmptySound = dynamic_cast<SoundContainer *>(reference.m_EmptySound->Clone()); }
	if (reference.m_ReloadStartSound) { m_ReloadStartSound = dynamic_cast<SoundContainer *>(reference.m_ReloadStartSound->Clone()); }
	if (reference.m_ReloadEndSound) { m_ReloadEndSound = dynamic_cast<SoundContainer *>(reference.m_ReloadEndSound->Clone()); }
	m_RateOfFire = reference.m_RateOfFire;
    m_ActivationDelay = reference.m_ActivationDelay;
    m_DeactivationDelay = reference.m_DeactivationDelay;
    m_Reloading = reference.m_Reloading;
    m_DoneReloading = reference.m_DoneReloading;
    m_ReloadTime = reference.m_ReloadTime;
    m_FullAuto = reference.m_FullAuto;
    m_FireIgnoresThis = reference.m_FireIgnoresThis;
    m_ShakeRange = reference.m_ShakeRange;
    m_SharpShakeRange = reference.m_SharpShakeRange;
    m_NoSupportFactor = reference.m_NoSupportFactor;
    m_ParticleSpreadRange = reference.m_ParticleSpreadRange;
    m_ShellSpreadRange = reference.m_ShellSpreadRange;
    m_ShellAngVelRange = reference.m_ShellAngVelRange;
    m_MuzzleOff = reference.m_MuzzleOff;
    m_EjectOff = reference.m_EjectOff;
    m_MagOff = reference.m_MagOff;
	m_RoundsFired = reference.m_RoundsFired;
	m_IsAnimatedManually = reference.m_IsAnimatedManually;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int HDFirearm::ReadProperty(const std::string_view &propName, Reader &reader) {
    if (propName == "Magazine") {
        Magazine iniDefinedObject;
        reader >> &iniDefinedObject;
        SetMagazine(dynamic_cast<Magazine *>(iniDefinedObject.Clone()));
    } else if (propName == "Flash") {
        Attachable iniDefinedObject;
        reader >> &iniDefinedObject;
        SetFlash(dynamic_cast<Attachable *>(iniDefinedObject.Clone()));
    } else if (propName == "PreFireSound") {
		m_PreFireSound = new SoundContainer;
		reader >> m_PreFireSound;
	} else if (propName == "FireSound") {
		m_FireSound = new SoundContainer;
		reader >> m_FireSound;
	} else if (propName == "FireEchoSound") {
		m_FireEchoSound = new SoundContainer;
		reader >> m_FireEchoSound;
		m_FireEchoSound->SetSoundOverlapMode(SoundContainer::SoundOverlapMode::RESTART);
    } else if (propName == "ActiveSound") {
		m_ActiveSound = new SoundContainer;
		reader >> m_ActiveSound;
	} else if (propName == "DeactivationSound") {
		m_DeactivationSound = new SoundContainer;
        reader >> m_DeactivationSound;
        m_DeactivationSound->SetSoundOverlapMode(SoundContainer::SoundOverlapMode::IGNORE_PLAY);
    } else if (propName == "EmptySound") {
		m_EmptySound = new SoundContainer;
		reader >> m_EmptySound;
	} else if (propName == "ReloadStartSound") {
		m_ReloadStartSound = new SoundContainer;
		reader >> m_ReloadStartSound;
	} else if (propName == "ReloadEndSound") {
		m_ReloadEndSound = new SoundContainer;
		reader >> m_ReloadEndSound;
	} else if (propName == "RateOfFire") {
        reader >> m_RateOfFire;
    } else if (propName == "ActivationDelay") {
        reader >> m_ActivationDelay;
    } else if (propName == "DeactivationDelay") {
        reader >> m_DeactivationDelay;
    } else if (propName == "ReloadTime") {
        reader >> m_ReloadTime;
    } else if (propName == "FullAuto") {
        reader >> m_FullAuto;
    } else if (propName == "FireIgnoresThis") {
        reader >> m_FireIgnoresThis;
    } else if (propName == "RecoilTransmission") {
        reader >> m_JointStiffness;
    } else if (propName == "IsAnimatedManually") {
		reader >> m_IsAnimatedManually;
    } else if (propName == "ShakeRange") {
        reader >> m_ShakeRange;
        m_ShakeRange /= 2;
    } else if (propName == "SharpShakeRange") {
        reader >> m_SharpShakeRange;
        m_SharpShakeRange /= 2;
    } else if (propName == "NoSupportFactor") {
        reader >> m_NoSupportFactor;
    } else if (propName == "ParticleSpreadRange") {
        reader >> m_ParticleSpreadRange;
        m_ParticleSpreadRange /= 2;
    } else if (propName == "ShellSpreadRange") {
        reader >> m_ShellSpreadRange;
        m_ShellSpreadRange /= 2;
    } else if (propName == "ShellAngVelRange") {
        reader >> m_ShellAngVelRange;
        m_ShellAngVelRange /= 2;
    } else if (propName == "MuzzleOffset") {
        reader >> m_MuzzleOff;
    } else if (propName == "EjectionOffset") {
        reader >> m_EjectOff;
    } else {
        return HeldDevice::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this HDFirearm with a Writer for
//                  later recreation with Create(Reader &reader);

int HDFirearm::Save(Writer &writer) const
{
    HeldDevice::Save(writer);

    writer.NewProperty("Magazine");
    writer << m_pMagazine;
    writer.NewProperty("Flash");
    writer << m_pFlash;
    writer.NewProperty("PreFireSound");
    writer << m_PreFireSound;
    writer.NewProperty("FireSound");
    writer << m_FireSound;
    writer.NewProperty("FireEchoSound");
    writer << m_FireEchoSound;
    writer.NewProperty("ActiveSound");
    writer << m_ActiveSound;
    writer.NewProperty("DeactivationSound");
    writer << m_DeactivationSound;
    writer.NewProperty("EmptySound");
    writer << m_EmptySound;
    writer.NewProperty("ReloadStartSound");
    writer << m_ReloadStartSound;
    writer.NewProperty("ReloadEndSound");
    writer << m_ReloadEndSound;
    writer.NewProperty("RateOfFire");
    writer << m_RateOfFire;
    writer.NewProperty("ActivationDelay");
    writer << m_ActivationDelay;
    writer.NewProperty("DeactivationDelay");
    writer << m_DeactivationDelay;
    writer.NewProperty("ReloadTime");
    writer << m_ReloadTime;
    writer.NewProperty("FullAuto");
    writer << m_FullAuto;
    writer.NewProperty("FireIgnoresThis");
    writer << m_FireIgnoresThis;
    writer.NewProperty("RecoilTransmission");
    writer << m_JointStiffness;
	writer.NewProperty("IsAnimatedManually");
	writer << m_IsAnimatedManually;
	writer.NewProperty("ShakeRange");
    writer << m_ShakeRange * 2;
    writer.NewProperty("SharpShakeRange");
    writer << m_SharpShakeRange * 2;
    writer.NewProperty("NoSupportFactor");
    writer << m_NoSupportFactor;
    writer.NewProperty("ParticleSpreadRange");
    writer << m_ParticleSpreadRange * 2;
    writer.NewProperty("ShellSpreadRange");
    writer << m_ShellSpreadRange * 2;
    writer.NewProperty("ShellAngVelRange");
    writer << m_ShellAngVelRange * 2;
    writer.NewProperty("MuzzleOffset");
    writer << m_MuzzleOff;
    writer.NewProperty("EjectionOffset");
    writer << m_EjectOff;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the HDFirearm object.

void HDFirearm::Destroy(bool notInherited)
{
	if (m_PreFireSound) {
		m_PreFireSound->Stop();
	}
	if (m_FireSound) {
		m_FireSound->Stop();
	}
	if (m_FireEchoSound) {
		m_FireEchoSound->Stop();
	}
	if (m_ActiveSound) {
		m_ActiveSound->Stop();
	}
	if (m_DeactivationSound) {
		m_DeactivationSound->Stop();
	}
	if (m_EmptySound) {
		m_EmptySound->Stop();
	}
	if (m_ReloadStartSound) {
		m_ReloadStartSound->Stop();
	}
	if (m_ReloadEndSound) {
		m_ReloadEndSound->Stop();
	}

	delete m_PreFireSound;
	delete m_FireSound;
	delete m_FireEchoSound;
	delete m_ActiveSound;
	delete m_DeactivationSound;
	delete m_EmptySound;
	delete m_ReloadStartSound;
	delete m_ReloadEndSound;

    if (!notInherited)
        HeldDevice::Destroy();
    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HDFirearm::SetMagazine(Magazine *newMagazine) {
    if (newMagazine == nullptr) {
        if (m_pMagazine && m_pMagazine->IsAttached()) { RemoveAttachable(m_pMagazine); }
        m_pMagazine = nullptr;
    } else {
        if (m_pMagazine && m_pMagazine->IsAttached()) { RemoveAttachable(m_pMagazine); }
        m_pMagazine = newMagazine;
        AddAttachable(newMagazine);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newMagazine->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            Magazine *castedAttachable = dynamic_cast<Magazine *>(attachable);
            RTEAssert(!attachable || castedAttachable, "Tried to pass incorrect Attachable subtype " + (attachable ? attachable->GetClassName() : "") + " to SetMagazine");
            dynamic_cast<HDFirearm *>(parent)->SetMagazine(castedAttachable);
        }});

        const Entity *newMagazineReference = g_PresetMan.GetEntityPreset(newMagazine->GetClassName(), newMagazine->GetPresetName(), newMagazine->GetModuleID());
        if (newMagazineReference) { m_pMagazineReference = dynamic_cast<const Magazine *>(newMagazineReference); }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HDFirearm::SetFlash(Attachable *newFlash) {
    if (newFlash == nullptr) {
        if (m_pFlash && m_pFlash->IsAttached()) { RemoveAttachable(m_pFlash); }
        m_pFlash = nullptr;
    } else {
        // Note - this is done here because setting mass on attached Attachables causes values to be updated on the parent (and its parent, and so on), which isn't ideal. Better to do it before the new flash is attached, so there are fewer calculations.
        newFlash->SetMass(0.0F);

        if (m_pFlash && m_pFlash->IsAttached()) { RemoveAttachable(m_pFlash); }
		m_pFlash = newFlash;
        AddAttachable(newFlash);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newFlash->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            dynamic_cast<HDFirearm *>(parent)->SetFlash(attachable);
        }});

        m_pFlash->SetDrawnNormallyByParent(false);
        m_pFlash->SetDeleteWhenRemovedFromParent(true);
        m_pFlash->SetCollidesWithTerrainWhileAttached(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HDFirearm::SetNextMagazineName(string magName)
{
    const Magazine * pNewMag = dynamic_cast<const Magazine *>(g_PresetMan.GetEntityPreset("Magazine", magName));
    if (pNewMag)
    {
        m_pMagazineReference = pNewMag;
        m_AIFireVel = -1;
        m_AIBulletLifeTime = 0;
        m_AIBulletAccScalar = -1;

        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRoundInMagCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of rounds still in the loaded magazine.

int HDFirearm::GetRoundInMagCount() const
{
    return m_pMagazine ? m_pMagazine->GetRoundCount() : 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIFireVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity the AI use when aiming this weapon

float HDFirearm::GetAIFireVel()
{
    if (m_AIFireVel < 0 && m_pMagazine)
        m_AIFireVel = m_pMagazine->GetAIAimVel();

    return m_AIFireVel;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIBulletLifeTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet life time the AI use when aiming this weapon.

unsigned long HDFirearm::GetAIBulletLifeTime()
{
    if (m_AIBulletLifeTime == 0 && m_pMagazine)
    {
        const Round * pRound = m_pMagazine->GetNextRound();
        if (pRound)
        {
            m_AIBulletLifeTime = pRound->GetAILifeTime();

            // Set a default if the lifetime is zero (i.e. infinite)
            if (m_AIBulletLifeTime == 0)
                m_AIBulletLifeTime = 20000;
        }
    }

    return m_AIBulletLifeTime;   
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBulletAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bullet acceleration scalar the AI use when aiming this weapon.

float HDFirearm::GetBulletAccScalar()
{
    if (m_AIBulletAccScalar < 0 && m_pMagazine)
        m_AIBulletAccScalar = m_pMagazine->GetBulletAccScalar();

    return m_AIBulletAccScalar;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIBlastRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the blast radius the AI use when aiming this weapon

float HDFirearm::GetAIBlastRadius() const
{
    int radius = -1;
    if (m_pMagazine)
        radius = m_pMagazine->GetAIAimBlastRadius();

    if (radius < 0)
    {
        // Set default value
        if (m_IsExplosiveWeapon)
            radius = 100;
        else
            radius = 0;
    }

    return radius;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAIPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how much material the projectiles from this weapon can destory.

float HDFirearm::GetAIPenetration() const
{
    if (m_pMagazine)
        return m_pMagazine->GetAIAimPenetration();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CompareTrajectories
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates how close the projectiles from two weapons will land.

float HDFirearm::CompareTrajectories(HDFirearm * pWeapon)
{
    if (pWeapon)
    {
        // Get AI aim data and cap life time to one second
        unsigned long LifeTime1 = GetAIBulletLifeTime();
        if (LifeTime1 == 0 || LifeTime1 > 1000)
            LifeTime1 = 1000;

        unsigned long LifeTime2 = GetAIBulletLifeTime();
        if (LifeTime2 == 0 || LifeTime2 > 1000)
            LifeTime2 = 1000;

        float time = max(min(LifeTime1, LifeTime2) / 1000.0f, 0.5f);
        Vector Vel1 = Vector(GetAIFireVel(), 0);
        Vector Vel2 = Vector(pWeapon->GetAIFireVel(), 0);

        // Estimate the hit pos according to: FuturePos=Pos+Vel+Accel*(t*t*0.5)
        time = time * time * 0.5;
        Vector FuturePos1 = GetMuzzlePos();
        g_SceneMan.WrapPosition(FuturePos1);
        FuturePos1 = FuturePos1 * c_MPP + RotateOffset(Vel1) + g_SceneMan.GetGlobalAcc() * GetBulletAccScalar() * time;

        Vector FuturePos2 = GetMuzzlePos();
        g_SceneMan.WrapPosition(FuturePos2);
        FuturePos2 = pWeapon->GetMuzzlePos() * c_MPP + RotateOffset(Vel2) + g_SceneMan.GetGlobalAcc() * pWeapon->GetBulletAccScalar() * time;

        return (FuturePos2 - FuturePos1).GetMagnitude() * c_PPM;
    }

    return 100000;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMagazinePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the magazine or other equivalent point of
//                  this.

Vector HDFirearm::GetMagazinePos() const
{
    return m_Pos + RotateOffset(m_MagOff);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  GetMuzzlePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of the muzzle or other equivalent point of
//                  this.

Vector HDFirearm::GetMuzzlePos() const
{
    return m_Pos + RotateOffset(m_MuzzleOff);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.

void HDFirearm::RestDetection()
{
    HeldDevice::RestDetection();

    if (m_FiredOnce)
        m_RestTimer.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates one of this HDFirearm's features. Analogous to 'pulling
//                  the trigger'.

void HDFirearm::Activate() {
    bool wasActivated = m_Activated;
    HeldDevice::Activate();

    if (!IsReloading()) {
        if (m_ActiveSound && !m_ActiveSound->IsBeingPlayed()) { m_ActiveSound->Play(this->m_Pos); }
        if (m_PreFireSound && !wasActivated && !m_PreFireSound->IsBeingPlayed()) { m_PreFireSound->Play(this->m_Pos); }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates one of this HDFirearm's features. Analogous to 'releasing
//                  the trigger'.

void HDFirearm::Deactivate() {
    bool wasActivated = m_Activated;
    HeldDevice::Deactivate();
    m_FiredOnce = false;

	if (m_PreFireSound) { m_PreFireSound->Stop(); }
    if (m_FireSound && m_FireSound->GetLoopSetting() == -1) { m_FireSound->Stop(); }
    if (m_DeactivationSound && wasActivated && m_pMagazine && !m_pMagazine->IsEmpty()) { m_DeactivationSound->Play(m_Pos); }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			StopActivationSound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Aborts playing of active sound no matter what. Used to silence spinning 
//                  weapons when weapons swapped

void HDFirearm::StopActivationSound()
{
    if (m_ActiveSound && m_ActiveSound->IsBeingPlayed())
        m_ActiveSound->Stop();

	//TODO: Also stop any animation
	//Those don't work really, at least we stopped it from making noise
	//m_Frame = 0;
	//m_Activated = false;
	//m_LastFireTmr.SetElapsedSimTimeMS(m_DeactivationDelay + 1);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Throws out the currently used Magazine, if any, and puts in a new one
//                  after the reload delay is up.

void HDFirearm::Reload()
{
    if (!m_Reloading)
    {
        if (m_pMagazine)
        {
            m_pMagazine->SetVel(m_Vel + Vector(m_HFlipped ? -3 : 3, 0.3));
            m_pMagazine->SetAngularVel(6.0F + (-RandomNum(0.0F, 6.0F)));
            if (!m_pMagazine->IsDiscardable()) { m_pMagazine->SetToDelete(); }
            RemoveAttachable(m_pMagazine, m_pMagazine->IsDiscardable(), false);
            m_pMagazine = 0;
        }

        // Stop any activation
        m_Activated = false;
        if (m_FireSound && m_FireSound->GetLoopSetting() == -1 && m_FireSound->IsBeingPlayed())
            m_FireSound->Stop();

		if (m_ReloadStartSound) { m_ReloadStartSound->Play(m_Pos); }
        m_ReloadTmr.Reset();
        m_Reloading = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NeedsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently in need of being reloaded.

bool HDFirearm::NeedsReloading() const
{
    if (!m_Reloading)
    {
        if (m_pMagazine)
        {
            // If we've used over half the rounds, we can profitably go ahead and reload
            return !m_pMagazine->IsOverHalfFull();
        }
        return true;
    }
    // We're currently reloading
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFull
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently full and reloading won't have
//                  any effect.

bool HDFirearm::IsFull()
{
    if (!m_Reloading)
    {
        if (m_pMagazine)
        {
            // If we've used over half the rounds, we can profitably go ahead and reload
            return m_pMagazine->GetRoundCount() == m_pMagazine->GetCapacity() || m_pMagazine->GetCapacity() < 0;
        }
        return false;
    }
    // We're currently reloading
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this HDFirearm. Supposed to be done every frame.

void HDFirearm::Update()
{
    HeldDevice::Update();

    if (m_PreFireSound && m_PreFireSound->IsBeingPlayed()) { m_PreFireSound->SetPosition(m_Pos); }
    if (m_FireSound && m_FireSound->IsBeingPlayed()) { m_FireSound->SetPosition(m_Pos); }
    if (m_ActiveSound && m_ActiveSound->IsBeingPlayed()) { m_ActiveSound->SetPosition(m_Pos); }
    if (m_DeactivationSound && m_DeactivationSound->IsBeingPlayed()) { m_DeactivationSound->SetPosition(m_Pos); }

    /////////////////////////////////
    // Activation/firing logic

    int roundsFired = 0;
	m_RoundsFired = 0;
    float degAimAngle = m_Rotation.GetDegAngle();
    degAimAngle = m_HFlipped ? (180 + degAimAngle) : degAimAngle;
    float totalFireForce = 0;
    m_FireFrame = false;
    m_DoneReloading = false;
    bool playedRoundFireSound = false;

    if (m_pMagazine && !m_pMagazine->IsEmpty())
    {
        if (m_Activated && !(m_PreFireSound && m_PreFireSound->IsBeingPlayed())) {

            // Get the parent root of this AEmitter
// TODO: Potentially get this once outside instead, like in attach/detach")
            MovableObject *pRootParent = GetRootParent();

            // Full auto
            if (m_FullAuto)
            {
                // ms per Round.
                double mspr = (long double)60000 / (long double)m_RateOfFire;
                // First round should fly as soon as activated and the delays are taken into account
                if (!m_FiredOnce && (m_LastFireTmr.GetElapsedSimTimeMS() - m_DeactivationDelay - m_ActivationDelay) > mspr)
                {
                    roundsFired = 1;
                    // Wind back the last fire timer appropriately for the first round, but not farther back than 0
                    m_LastFireTmr.SetElapsedSimTimeMS(MAX(m_LastFireTmr.GetElapsedSimTimeMS() - mspr, 0));
                }
                // How many rounds are going to fly since holding down activation. Make sure gun can't be fired faster by tapping activation fast
                if (m_LastFireTmr.GetElapsedSimTimeMS() > (m_ActivationTimer.GetElapsedSimTimeMS() - m_ActivationDelay))
                    roundsFired += (m_ActivationTimer.GetElapsedSimTimeMS() - m_ActivationDelay) / mspr;
                else
                    roundsFired += m_LastFireTmr.GetElapsedSimTimeMS() / mspr;
            }
            // Semi-auto
            else
            {
                double mspr = (long double)60000 / (long double)m_RateOfFire;
// TODO: Confirm that the delays work properly in semi-auto!
                if (!m_FiredOnce && (m_LastFireTmr.GetElapsedSimTimeMS() - m_ActivationDelay - m_DeactivationDelay) > mspr)
                    roundsFired = 1;
                else
                    roundsFired = 0;
            }

            if (roundsFired >= 1)
            {
				m_FiredOnce = true;
				m_FireFrame = true;
                m_LastFireTmr.Reset();
            }

            Vector roundVel;
            Vector shellVel;

            Round *pRound = 0;
            Vector tempNozzle;
            Vector tempEject;
            MOPixel *pPixel;
            float shake, particleSpread, shellSpread, lethalRange;

            lethalRange = m_MaxSharpLength + max(g_FrameMan.GetPlayerFrameBufferWidth(-1), g_FrameMan.GetPlayerFrameBufferHeight(-1)) * 0.52;
            Actor *pUser = dynamic_cast<Actor *>(pRootParent);
            if (pUser)
                lethalRange += pUser->GetAimDistance();

            // Fire all rounds that were fired this frame.
            for (int i = 0; i < roundsFired && !m_pMagazine->IsEmpty(); ++i)
            {
				m_RoundsFired++;

                pRound = m_pMagazine->PopNextRound();
                shake = (m_ShakeRange - ((m_ShakeRange - m_SharpShakeRange) * m_SharpAim)) *
                        (m_Supported ? 1.0F : m_NoSupportFactor) * RandomNormalNum();
                tempNozzle = m_MuzzleOff.GetYFlipped(m_HFlipped);
                tempNozzle.DegRotate(degAimAngle + shake);
                roundVel.SetXY(pRound->GetFireVel(), 0);
                roundVel.DegRotate(degAimAngle + shake);

                Vector particlePos;
                Vector particleVel;

                // Launch all particles in round
                MovableObject *pParticle = 0;
                while (!pRound->IsEmpty())
                {
                    pParticle = pRound->PopNextParticle();

                    // Only make the particles separate back behind the nozzle, not in front. THis is to avoid silly penetration firings
					particlePos = tempNozzle + (roundVel.GetNormalized() * (-RandomNum()) * pRound->GetSeparation());
                    pParticle->SetPos(m_Pos + particlePos);

                    particleVel = roundVel;
                    particleSpread = m_ParticleSpreadRange * RandomNormalNum();
                    particleVel.DegRotate(particleSpread);
                    pParticle->SetVel(m_Vel + particleVel);
                    pParticle->SetRotAngle(particleVel.GetAbsRadAngle());
                    // F = m * a
                    totalFireForce += pParticle->GetMass() * pParticle->GetVel().GetMagnitude();

                    // Detach if it's an attachable
                    Attachable *pAttachable = dynamic_cast<Attachable *>(pParticle);
                    if (pAttachable)
                    {
                        if (pAttachable->IsAttached()) { pAttachable->GetParent()->RemoveAttachable(pAttachable); }
                        // Activate if it is some kind of grenade or whatnot.
                        ThrownDevice *pTD = dynamic_cast<ThrownDevice *>(pAttachable);
                        if (pTD)
                            pTD->Activate();
                    }

                    // Set the fired particle to not hit this HeldDevice's parent, if applicable
                    if (m_FireIgnoresThis)
                        pParticle->SetWhichMOToNotHit(this, 1.0f);

                    // Set the team so alarm events that happen if these gib won't freak out the guy firing
                    pParticle->SetTeam(m_Team);

                    // Also make this not hit team members
                    pParticle->SetIgnoresTeamHits(true);

                    // Decide for how long until the bullet tumble and start to lose lethality
                    pPixel = dynamic_cast<MOPixel *>(pParticle);
                    if (pPixel)
                        pPixel->SetLethalRange(lethalRange);

                    g_MovableMan.AddParticle(pParticle);
                }
                pParticle = 0;

                // Launch shell, if there is one.
                MovableObject *pShell = pRound->GetShell() ? dynamic_cast<MovableObject *>(pRound->GetShell()->Clone()) : 0;
                if (pShell)
                {
                    tempEject = m_EjectOff.GetYFlipped(m_HFlipped);
                    shellSpread = m_ShellSpreadRange * RandomNormalNum();
                    tempEject.DegRotate(degAimAngle + shellSpread);
                    pShell->SetPos(m_Pos + tempEject);

                    // ##@#@@$ TEMP
                    shellVel.SetXY(pRound->GetShellVel(), 0);
                    shellVel.DegRotate(degAimAngle + 150 * (m_HFlipped ? -1 : 1) + shellSpread);
                    pShell->SetVel(m_Vel + shellVel);
                    pShell->SetRotAngle(m_Rotation.GetRadAngle());
                    pShell->SetAngularVel(pShell->GetAngularVel() + (m_ShellAngVelRange * RandomNormalNum()));
					pShell->SetHFlipped(m_HFlipped);
//                  // Set the ejected shell to not hit this HeldDevice's parent, if applicable
//                  if (m_FireIgnoresThis)
//                      pParticle->SetWhichMOToNotHit(pRootParent, 1.0f);
                    // Set the team so alarm events that happen if these gib won't freak out the guy firing
                    pShell->SetTeam(m_Team);
                    g_MovableMan.AddParticle(pShell);
                    pShell = 0;
                }

                // Sound the extra Round firing sound, if any is defined
                if (!playedRoundFireSound && pRound->HasFireSound())
                {
                    pRound->GetFireSound()->Play(m_Pos);
                    playedRoundFireSound = true;
                }

                delete pRound;
            }
            pRound = 0;
        }
    }

    // No or empty magazine, so just click.
    else if (((m_pMagazine && m_pMagazine->IsEmpty()) || !m_pMagazine) && m_Activated && !m_AlreadyClicked )
    {
        // Play empty pin click sound.
		if (m_EmptySound) { m_EmptySound->Play(m_Pos); }
        // Indicate that we have clicked once during the current activation. 
        m_AlreadyClicked = true;

        // Auto-reload
        Reload();
    }

    // No magazine, have started to reload, so put new mag in when done
    if (m_Reloading && !m_pMagazine && m_pMagazineReference && m_ReloadTmr.IsPastSimMS(m_ReloadTime)) {
        SetMagazine(dynamic_cast<Magazine *>(m_pMagazineReference->Clone()));
		if (m_ReloadEndSound) { m_ReloadEndSound->Play(m_Pos); }

        m_ActivationTimer.Reset();
        m_LastFireTmr.Reset();

        if (m_PreFireSound && m_Activated) { m_PreFireSound->Play(); }

        m_Reloading = false;
        m_DoneReloading = true;
    }

    // Do stuff to deactivate after being activated
    if (!m_Activated)
    {
		if (m_AlreadyClicked && m_FiredLastFrame && m_DeactivationSound) { m_DeactivationSound->Play(m_Pos); }
        // Reset the click indicator.
        m_AlreadyClicked = false;

		if (m_PreFireSound) { m_PreFireSound->Stop(); }
        // Stop any looping activation sounds
        if (m_FireSound && m_FireSound->GetLoopSetting() == -1)// && m_FireSound->IsBeingPlayed())
            m_FireSound->Stop();
    }

    //////////////////////////////////////////////
    // Recoil and other activation effects logic.

    if (roundsFired > 0) {
        // Alternate to get that shake effect!
        m_Recoiled = !m_Recoiled;

        // Set up the recoil force and shake offsets
        if (m_Recoiled)
        {
            m_RecoilForce.SetXY(totalFireForce * m_JointStiffness, 0);
            m_RecoilForce = RotateOffset(m_RecoilForce);
            m_RecoilForce = -m_RecoilForce;

            // Set up the recoil shake offset
            m_RecoilOffset = m_RecoilForce;
            m_RecoilOffset.SetMagnitude(1.25);
        }

        AddImpulseForce(m_RecoilForce, m_RecoilOffset);

        // Display gun animation
		if (!m_IsAnimatedManually)
		{
			if (m_FrameCount > 1)
				m_Frame = 1;
		}

        // Display gun flame frame.
        if (m_pFlash) {
            m_pFlash->SetParentOffset(m_MuzzleOff);
            m_pFlash->SetFrame(RandomNum(0, m_pFlash->GetFrameCount() - 1));
        }

        // Play firing sound
        // Only start playing if it's not a looping fire sound that is already playing, and if there's a mag
        if (m_pMagazine) {
            if (m_FireSound && !(m_FireSound->GetLoopSetting() == -1 && m_FireSound->IsBeingPlayed())) {
                m_FireSound->Play(m_Pos);
            }
			if (m_FireEchoSound) { m_FireEchoSound->Play(m_Pos); }
        }
    }
    else {
        m_Recoiled = false;
		if (!m_IsAnimatedManually)
	        m_Frame = 0;
    }

    // Display and override gun animation if there's a special one
    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENMOVING)
        {
            if (m_Activated || m_LastFireTmr.GetElapsedSimTimeMS() < m_DeactivationDelay) {
                // Max rate of the animation when fully activated and firing
                int animDuration = m_SpriteAnimDuration;
                // Spin up - can only spin up if mag is inserted
                if (m_Activated && !m_Reloading && m_ActivationTimer.GetElapsedSimTimeMS() < m_ActivationDelay)
                {
                    animDuration = (int)LERP(0, m_ActivationDelay, (float)(m_SpriteAnimDuration * 10), (float)m_SpriteAnimDuration, m_ActivationTimer.GetElapsedSimTimeMS());
					if (m_ActiveSound) { m_ActiveSound->SetPitch(LERP(0, m_ActivationDelay, 0, 1.0, m_ActivationTimer.GetElapsedSimTimeMS())); }
                }
                // Spin down
                if ((!m_Activated || m_Reloading) && m_LastFireTmr.GetElapsedSimTimeMS() < m_DeactivationDelay)
                {
                    animDuration = (int)LERP(0, m_DeactivationDelay, (float)m_SpriteAnimDuration, (float)(m_SpriteAnimDuration * 10), m_LastFireTmr.GetElapsedSimTimeMS());
					if (m_ActiveSound) { m_ActiveSound->SetPitch(LERP(0, m_DeactivationDelay, 1.0, 0, m_LastFireTmr.GetElapsedSimTimeMS())); }
                }

                if (animDuration > 0 && !(m_Reloading && m_LastFireTmr.GetElapsedSimTimeMS() >= m_DeactivationDelay)) {
                    float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % animDuration;
					if (!m_IsAnimatedManually)
	                    m_Frame = std::floor((cycleTime / (float)animDuration) * (float)m_FrameCount);
				} else {
					StopActivationSound();
				}
            } else {
				if (!m_IsAnimatedManually)
					m_Frame = 0;
				StopActivationSound();
            }
        }
    }

    /////////////////////////////////
    // Update fitted Magazine.

    if (m_pMagazine) {
        // Recoil offset has to be applied after the Update or it'll get reset within the update
        m_pMagazine->SetRecoil(m_RecoilForce, m_RecoilOffset, m_Recoiled);
    }

    m_FiredLastFrame = m_FireFrame;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EstimateDigStrenght
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength the rounds in the magazine can destroy. 

float HDFirearm::EstimateDigStrenght()
{
    if (m_pMagazine)
        return m_pMagazine->EstimateDigStrenght();

    return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this HDFirearm's current graphical representation to a
//                  BITMAP of choice.

void HDFirearm::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const {
    if (m_pFlash && m_FireFrame && !m_pFlash->IsDrawnAfterParent() && mode == g_DrawColor && !onlyPhysical) {
        m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }

    HeldDevice::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    if (m_pFlash && m_FireFrame && m_pFlash->IsDrawnAfterParent() && mode == g_DrawColor && !onlyPhysical) {
        m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
    }

    // Fudge the muzzle pos forward a little bit so the glow aligns nicely
    Vector muzzlePos = m_MuzzleOff;
    muzzlePos.m_X += 4;
    muzzlePos = m_Pos + RotateOffset(muzzlePos);
    // Set the screen flash effect to draw at the final post processing stage
    if (m_FireFrame && m_pFlash && m_pFlash->GetScreenEffect() && mode == g_DrawColor && !onlyPhysical && !g_SceneMan.ObscuredPoint(muzzlePos)) {
        g_PostProcessMan.RegisterPostEffect(muzzlePos, m_pFlash->GetScreenEffect(), m_pFlash->GetScreenEffectHash(), 55.0F + RandomNum(0.0F, 200.0F), m_pFlash->GetEffectRotAngle());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws an aiming aid in front of this HeldDevice.

void HDFirearm::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible)
        return;

    // Only draw if the team viewing this is on the same team OR has seen the space where this is located
    int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam)
    {
        if (g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam))
            return;
    }

    HeldDevice::DrawHUD(pTargetBitmap, targetPos, whichScreen);

    // Don't bother if the aim distance is really short, or not held
    if (!m_Parent || m_SharpAim < 0.15)
        return;

    float sharpLength = m_MaxSharpLength * m_SharpAim;

    if (playerControlled)
    {
        Vector aimPoint1(sharpLength - 9, 0);
        Vector aimPoint2(sharpLength - 3, 0);
        Vector aimPoint3(sharpLength + 3, 0);
        Vector aimPoint4(sharpLength + 9, 0);
        aimPoint1 += m_MuzzleOff;
        aimPoint2 += m_MuzzleOff;
        aimPoint3 += m_MuzzleOff;
        aimPoint4 += m_MuzzleOff;
        Matrix aimMatrix(m_Rotation);
        aimMatrix.SetXFlipped(m_HFlipped);
        aimPoint1 *= aimMatrix;
        aimPoint2 *= aimMatrix;
        aimPoint3 *= aimMatrix;
        aimPoint4 *= aimMatrix;
        aimPoint1 += m_Pos;
        aimPoint2 += m_Pos;
        aimPoint3 += m_Pos;
        aimPoint4 += m_Pos;

        // Put the flickering glows on the reticule dots, in absolute scene coordinates
		int glow = (155 + RandomNum(0, 100));
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint1, YellowDot, glow);
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint2, YellowDot, glow);
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint3, YellowDot, glow);
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint4, YellowDot, glow);

        // Make into target frame coordinates
        aimPoint1 -= targetPos;
        aimPoint2 -= targetPos;
        aimPoint3 -= targetPos;
        aimPoint4 -= targetPos;

        // Wrap the points
        g_SceneMan.WrapPosition(aimPoint1);
        g_SceneMan.WrapPosition(aimPoint2);
        g_SceneMan.WrapPosition(aimPoint3);
        g_SceneMan.WrapPosition(aimPoint4);

        acquire_bitmap(pTargetBitmap);
        putpixel(pTargetBitmap, aimPoint1.m_X, aimPoint1.m_Y, g_YellowGlowColor);
        putpixel(pTargetBitmap, aimPoint2.m_X, aimPoint2.m_Y, g_YellowGlowColor);
        putpixel(pTargetBitmap, aimPoint3.m_X, aimPoint3.m_Y, g_YellowGlowColor);
        putpixel(pTargetBitmap, aimPoint4.m_X, aimPoint4.m_Y, g_YellowGlowColor);
        release_bitmap(pTargetBitmap);
    }
    else
    {
        Vector aimPoint2(sharpLength - 3, 0);
        Vector aimPoint3(sharpLength + 3, 0);
        aimPoint2 += m_MuzzleOff;
        aimPoint3 += m_MuzzleOff;
        Matrix aimMatrix(m_Rotation);
        aimMatrix.SetXFlipped(m_HFlipped);
        aimPoint2 *= aimMatrix;
        aimPoint3 *= aimMatrix;
        aimPoint2 += m_Pos;
        aimPoint3 += m_Pos;

        // Put the flickering glows on the reticule dots, in absolute scene coordinates
        int glow = (55 + RandomNum(0, 100));
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint2, YellowDot, glow);
		g_PostProcessMan.RegisterGlowDotEffect(aimPoint3, YellowDot, glow);

        // Make into target frame coordinates
        aimPoint2 -= targetPos;
        aimPoint3 -= targetPos;

        // Wrap the points
        g_SceneMan.WrapPosition(aimPoint2);
        g_SceneMan.WrapPosition(aimPoint3);

        acquire_bitmap(pTargetBitmap);
        putpixel(pTargetBitmap, aimPoint2.m_X, aimPoint2.m_Y, g_YellowGlowColor);
        putpixel(pTargetBitmap, aimPoint3.m_X, aimPoint3.m_Y, g_YellowGlowColor);
        release_bitmap(pTargetBitmap);
    }
}

} // namespace RTE
