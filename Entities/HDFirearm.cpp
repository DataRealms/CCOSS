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

#include "ActivityMan.h"
#include "CameraMan.h"
#include "FrameMan.h"
#include "PresetMan.h"

#include "Magazine.h"
#include "ThrownDevice.h"
#include "MOPixel.h"
#include "Actor.h"

namespace RTE {

ConcreteClassInfo(HDFirearm, HeldDevice, 50);


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
    m_ReloadEndOffset = -1.0F;
    m_HasPlayedEndReloadSound = false;
    m_RateOfFire = 0;
    m_ActivationDelay = 0;
    m_DeactivationDelay = 0;
    m_Reloading = false;
    m_DoneReloading = false;
    m_BaseReloadTime = 0;
    m_FullAuto = false;
    m_FireIgnoresThis = true;
	m_Reloadable = true;
	m_DualReloadable = false;
	m_OneHandedReloadTimeMultiplier = 1.5F;
	m_ReloadAngle = -0.5F;
	m_OneHandedReloadAngle = -1.0F;
    m_ShakeRange = 0;
    m_SharpShakeRange = 0;
    m_NoSupportFactor = 0;
    m_ParticleSpreadRange = 0;
	m_ShellEjectAngle = 150;
    m_ShellSpreadRange = 0;
    m_ShellAngVelRange = 0;
	m_ShellVelVariation = 0.1F;
    m_RecoilScreenShakeAmount = 0.0F;
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

	m_LegacyCompatibilityRoundsAlwaysFireUnflipped = false;
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
    if (reference.m_pMagazine) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pMagazine->GetUniqueID()); }
    if (reference.m_pFlash) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFlash->GetUniqueID()); }

    HeldDevice::Create(reference);

    if (reference.m_pMagazine) { SetMagazine(dynamic_cast<Magazine *>(reference.m_pMagazine->Clone())); }
    if (reference.m_pFlash) { SetFlash(dynamic_cast<Attachable *>(reference.m_pFlash->Clone())); }

    m_pMagazineReference = reference.m_pMagazineReference;
	if (reference.m_PreFireSound) { m_PreFireSound = dynamic_cast<SoundContainer *>(reference.m_PreFireSound->Clone()); }
	if (reference.m_FireSound) { m_FireSound = dynamic_cast<SoundContainer *>(reference.m_FireSound->Clone()); }
	if (reference.m_FireEchoSound) { m_FireEchoSound = dynamic_cast<SoundContainer*>(reference.m_FireEchoSound->Clone()); }
	if (reference.m_ActiveSound) { m_ActiveSound = dynamic_cast<SoundContainer *>(reference.m_ActiveSound->Clone()); }
	if (reference.m_DeactivationSound) { m_DeactivationSound = dynamic_cast<SoundContainer *>(reference.m_DeactivationSound->Clone()); }
	if (reference.m_EmptySound) { m_EmptySound = dynamic_cast<SoundContainer *>(reference.m_EmptySound->Clone()); }
	if (reference.m_ReloadStartSound) { m_ReloadStartSound = dynamic_cast<SoundContainer *>(reference.m_ReloadStartSound->Clone()); }
	if (reference.m_ReloadEndSound) { m_ReloadEndSound = dynamic_cast<SoundContainer *>(reference.m_ReloadEndSound->Clone()); }
    m_ReloadEndOffset = reference.m_ReloadEndOffset;
	m_RateOfFire = reference.m_RateOfFire;
    m_ActivationDelay = reference.m_ActivationDelay;
    m_DeactivationDelay = reference.m_DeactivationDelay;
    m_Reloading = reference.m_Reloading;
    m_DoneReloading = reference.m_DoneReloading;
    m_BaseReloadTime = reference.m_BaseReloadTime;
	m_LastFireTmr = reference.m_LastFireTmr;
	m_ReloadTmr = reference.m_ReloadTmr;
    m_FullAuto = reference.m_FullAuto;
    m_FireIgnoresThis = reference.m_FireIgnoresThis;
    m_Reloadable = reference.m_Reloadable;
	m_DualReloadable = reference.m_DualReloadable;
	m_OneHandedReloadTimeMultiplier = reference.m_OneHandedReloadTimeMultiplier;
	m_ReloadAngle = reference.m_ReloadAngle;
	m_OneHandedReloadAngle = reference.m_OneHandedReloadAngle;
    m_ShakeRange = reference.m_ShakeRange;
    m_SharpShakeRange = reference.m_SharpShakeRange;
    m_NoSupportFactor = reference.m_NoSupportFactor;
    m_ParticleSpreadRange = reference.m_ParticleSpreadRange;
	m_ShellEjectAngle = reference.m_ShellEjectAngle;
    m_ShellSpreadRange = reference.m_ShellSpreadRange;
    m_ShellAngVelRange = reference.m_ShellAngVelRange;
    m_ShellVelVariation = reference.m_ShellVelVariation;
    m_RecoilScreenShakeAmount = reference.m_RecoilScreenShakeAmount;
    m_MuzzleOff = reference.m_MuzzleOff;
    m_EjectOff = reference.m_EjectOff;
    m_MagOff = reference.m_MagOff;
	m_RoundsFired = reference.m_RoundsFired;
	m_IsAnimatedManually = reference.m_IsAnimatedManually;

	m_LegacyCompatibilityRoundsAlwaysFireUnflipped = reference.m_LegacyCompatibilityRoundsAlwaysFireUnflipped;

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
    StartPropertyList(return HeldDevice::ReadProperty(propName, reader));
    
    MatchProperty("Magazine", { SetMagazine(dynamic_cast<Magazine *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchProperty("Flash", { SetFlash(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchProperty("PreFireSound", {
		m_PreFireSound = new SoundContainer;
		reader >> m_PreFireSound;
	});
	MatchProperty("FireSound", {
		m_FireSound = new SoundContainer;
		reader >> m_FireSound;
	});
	MatchProperty("FireEchoSound", {
		m_FireEchoSound = new SoundContainer;
		reader >> m_FireEchoSound;
		m_FireEchoSound->SetSoundOverlapMode(SoundContainer::SoundOverlapMode::RESTART);
    });
    MatchProperty("ActiveSound", {
		m_ActiveSound = new SoundContainer;
		reader >> m_ActiveSound;
	});
	MatchProperty("DeactivationSound", {
		m_DeactivationSound = new SoundContainer;
        reader >> m_DeactivationSound;
    });
    MatchProperty("EmptySound", {
		m_EmptySound = new SoundContainer;
		reader >> m_EmptySound;
	});
	MatchProperty("ReloadStartSound", {
		m_ReloadStartSound = new SoundContainer;
		reader >> m_ReloadStartSound;
	});
	MatchProperty("ReloadEndSound", {
		m_ReloadEndSound = new SoundContainer;
		reader >> m_ReloadEndSound;
    });
    MatchProperty("ReloadEndOffset", { reader >> m_ReloadEndOffset; });
	MatchProperty("RateOfFire", { reader >> m_RateOfFire; });
    MatchProperty("ActivationDelay", { reader >> m_ActivationDelay; });
    MatchProperty("DeactivationDelay", { reader >> m_DeactivationDelay; });
	MatchForwards("BaseReloadTime") MatchProperty("ReloadTime", { reader >> m_BaseReloadTime; });
    MatchProperty("FullAuto", { reader >> m_FullAuto; });
    MatchProperty("FireIgnoresThis", { reader >> m_FireIgnoresThis; });
    MatchProperty("Reloadable", { reader >> m_Reloadable; });
	MatchProperty("DualReloadable", { reader >> m_DualReloadable; });
	MatchProperty("OneHandedReloadTimeMultiplier", { reader >> m_OneHandedReloadTimeMultiplier; });
	MatchProperty("ReloadAngle", { reader >> m_ReloadAngle; });
	MatchProperty("OneHandedReloadAngle", { reader >> m_OneHandedReloadAngle; });
    MatchProperty("RecoilTransmission", { reader >> m_JointStiffness; });
    MatchProperty("IsAnimatedManually", { reader >> m_IsAnimatedManually; });
    MatchProperty("ShakeRange", {
        reader >> m_ShakeRange;
        m_ShakeRange /= 2;
    });
    MatchProperty("SharpShakeRange", {
        reader >> m_SharpShakeRange;
        m_SharpShakeRange /= 2;
    });
    MatchProperty("NoSupportFactor", { reader >> m_NoSupportFactor; });
    MatchProperty("ParticleSpreadRange", {
        reader >> m_ParticleSpreadRange;
        m_ParticleSpreadRange /= 2;
	});
	MatchProperty("ShellEjectAngle", { reader >> m_ShellEjectAngle; });
    MatchProperty("ShellSpreadRange", {
        reader >> m_ShellSpreadRange;
        m_ShellSpreadRange /= 2;
    });
    MatchProperty("ShellAngVelRange", {
        reader >> m_ShellAngVelRange;
        m_ShellAngVelRange /= 2;
	});
	MatchProperty("ShellVelVariation", { reader >> m_ShellVelVariation; });
    MatchProperty("RecoilScreenShakeAmount", { reader >> m_RecoilScreenShakeAmount; });
    MatchProperty("MuzzleOffset", { reader >> m_MuzzleOff; });
	MatchProperty("EjectionOffset", { reader >> m_EjectOff; });
	MatchProperty("LegacyCompatibilityRoundsAlwaysFireUnflipped", { reader >> m_LegacyCompatibilityRoundsAlwaysFireUnflipped; });
    

    EndPropertyList;
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
    writer.NewPropertyWithValue("ReloadEndOffset", m_ReloadEndOffset);
    writer.NewProperty("RateOfFire");
    writer << m_RateOfFire;
    writer.NewProperty("ActivationDelay");
    writer << m_ActivationDelay;
    writer.NewProperty("DeactivationDelay");
    writer << m_DeactivationDelay;
    writer.NewProperty("ReloadTime");
    writer << m_BaseReloadTime;
    writer.NewProperty("FullAuto");
    writer << m_FullAuto;
    writer.NewProperty("FireIgnoresThis");
    writer << m_FireIgnoresThis;
    writer.NewProperty("Reloadable");
	writer.NewPropertyWithValue("DualReloadable", m_DualReloadable);
	writer.NewPropertyWithValue("OneHandedReloadTimeMultiplier", m_OneHandedReloadTimeMultiplier);
	writer.NewPropertyWithValue("ReloadAngle", m_ReloadAngle);
	writer.NewPropertyWithValue("OneHandedReloadAngle", m_OneHandedReloadAngle);
    writer << m_Reloadable;
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
	writer.NewProperty("ShellEjectAngle");
	writer << m_ShellEjectAngle;
    writer.NewProperty("ShellSpreadRange");
    writer << m_ShellSpreadRange * 2;
    writer.NewProperty("ShellAngVelRange");
    writer << m_ShellAngVelRange * 2;
	writer.NewProperty("ShellVelVariation");
	writer << m_ShellVelVariation;
    writer.NewProperty("RecoilScreenShakeAmount");
    writer << m_RecoilScreenShakeAmount;
    writer.NewProperty("MuzzleOffset");
    writer << m_MuzzleOff;
    writer.NewProperty("EjectionOffset");
    writer << m_EjectOff;

	writer.NewPropertyWithValue("LegacyCompatibilityRoundsAlwaysFireUnflipped", m_LegacyCompatibilityRoundsAlwaysFireUnflipped);

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
    if (m_pMagazine && m_pMagazine->IsAttached()) { RemoveAndDeleteAttachable(m_pMagazine); }
    if (newMagazine == nullptr) {
        m_pMagazine = nullptr;
    } else {
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
    if (m_pFlash && m_pFlash->IsAttached()) { RemoveAndDeleteAttachable(m_pFlash); }
    if (newFlash == nullptr) {
        m_pFlash = nullptr;
    } else {
        // Note - this is done here because setting mass on attached Attachables causes values to be updated on the parent (and its parent, and so on), which isn't ideal. Better to do it before the new flash is attached, so there are fewer calculations.
        newFlash->SetMass(0.0F);

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

std::string HDFirearm::GetNextMagazineName() const {
	return m_pMagazineReference->GetPresetName();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HDFirearm::SetNextMagazineName(std::string magName)
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int HDFirearm::GetRoundInMagCapacity() const {
	if (m_pMagazine) {
		return m_pMagazine->GetCapacity();
	} else if (m_pMagazineReference) {
		return m_pMagazineReference->GetCapacity();
	}
	return 0;
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

        float time = std::max(std::min(LifeTime1, LifeTime2) / 1000.0f, 0.5f);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HDFirearm::RestDetection() {
	HeldDevice::RestDetection();

	if (m_FiredOnce) { m_RestTimer.Reset(); }
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
		if (m_DeactivationSound && m_DeactivationSound->IsBeingPlayed()) { m_DeactivationSound->FadeOut(); }
        if (m_ActiveSound && !m_ActiveSound->IsBeingPlayed() && (m_ActiveSound->GetLoopSetting() == -1 || !wasActivated)) { m_ActiveSound->Play(this->m_Pos); }
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
    if (m_DeactivationSound && wasActivated && m_FiredLastFrame) { m_DeactivationSound->Play(m_Pos); }
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HDFirearm::Reload() {
	if (!m_Reloading && m_Reloadable) {
		bool hadMagazineBeforeReloading = m_pMagazine != nullptr;
        if (hadMagazineBeforeReloading) {
			Vector constrainedMagazineOffset = g_SceneMan.ShortestDistance(m_Pos, m_pMagazine->GetPos(), g_SceneMan.SceneWrapsX()).SetMagnitude(2.0F);
			Vector ejectVector = Vector(2.0F * GetFlipFactor(), 0.0F) + constrainedMagazineOffset.RadRotate(RandomNum(-0.2F, 0.2F));
			m_pMagazine->SetVel(m_Vel + ejectVector);
			m_pMagazine->SetAngularVel(RandomNum(-3.0F, 3.0F));

            if (!m_pMagazine->IsDiscardable()) { m_pMagazine->SetToDelete(); }
            RemoveAttachable(m_pMagazine, m_pMagazine->IsDiscardable(), false);
            m_pMagazine = 0;
        }

		Deactivate();
		if (m_ReloadStartSound) { m_ReloadStartSound->Play(m_Pos); }

		m_ReloadTmr.Reset();
		CorrectReloadTimerForSupportAvailable();

		RunScriptedFunctionInAppropriateScripts("OnReload", false, false, {}, { hadMagazineBeforeReloading ? "true" : "false" });

		m_Reloading = true;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NeedsReloading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the device is curtrently in need of being reloaded.

bool HDFirearm::NeedsReloading() const
{
	if (!m_Reloading && m_Reloadable) {
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

bool HDFirearm::IsFull() const
{
	if (!m_Reloading && m_Reloadable) {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HDFirearm::IsEmpty() const {
	if (m_pMagazine) {
		return m_pMagazine->IsEmpty();
	}
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    Actor *pActor = dynamic_cast<Actor*>(GetRootParent());

    /////////////////////////////////
    // Activation/firing logic

    int roundsFired = 0;
	m_RoundsFired = 0;
    float degAimAngle = m_Rotation.GetDegAngle();
    degAimAngle = m_HFlipped ? (180.0F + degAimAngle) : degAimAngle;
    float totalFireForce = 0.0F;
    m_FireFrame = false;
    m_DoneReloading = false;
    bool playedRoundFireSound = false;

    if (m_pMagazine && !m_pMagazine->IsEmpty())
    {
        if (m_Activated && !(m_PreFireSound && m_PreFireSound->IsBeingPlayed())) {

			double msPerRound = GetMSPerRound();
			if (m_FullAuto) {
				// First round should fly as soon as activated and the delays are taken into account
				if (!m_FiredOnce && (m_LastFireTmr.GetElapsedSimTimeMS() - m_DeactivationDelay - m_ActivationDelay) > msPerRound) {
					roundsFired = 1;
					// Wind back the last fire timer appropriately for the first round, but not farther back than 0
					m_LastFireTmr.SetElapsedSimTimeMS(std::max(m_LastFireTmr.GetElapsedSimTimeMS() - msPerRound, 0.0));
				}
				// How many rounds are going to fly since holding down activation. Make sure gun can't be fired faster by tapping activation fast
				if (m_LastFireTmr.GetElapsedSimTimeMS() > (m_ActivationTimer.GetElapsedSimTimeMS() - m_ActivationDelay)) {
					roundsFired += (m_ActivationTimer.GetElapsedSimTimeMS() - m_ActivationDelay) / msPerRound;
				} else {
					roundsFired += m_LastFireTmr.GetElapsedSimTimeMS() / msPerRound;
				}
            } else {
				// TODO: Confirm that the delays work properly in semi-auto!
				roundsFired = !m_FiredOnce && (m_LastFireTmr.GetElapsedSimTimeMS() - m_ActivationDelay - m_DeactivationDelay) > msPerRound ? 1 : 0;
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

            lethalRange = m_MaxSharpLength * m_SharpAim + std::max(g_FrameMan.GetPlayerFrameBufferWidth(-1), g_FrameMan.GetPlayerFrameBufferHeight(-1)) * 0.51F;
            if (pActor) {
                lethalRange += pActor->GetAimDistance();
            }

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
				int particleCountMax = pRound->ParticleCount();
				float lifeVariation = pRound->GetLifeVariation();

                // Launch all particles in round
                MovableObject *pParticle = 0;
                while (!pRound->IsEmpty())
                {
                    pParticle = pRound->PopNextParticle();

                    // Only make the particles separate back behind the nozzle, not in front. This is to avoid silly penetration firings
					particlePos = tempNozzle + (roundVel.GetNormalized() * (-RandomNum()) * pRound->GetSeparation());
                    pParticle->SetPos(m_Pos + particlePos);

                    particleVel = roundVel;
                    particleSpread = m_ParticleSpreadRange * RandomNormalNum();
                    particleVel.DegRotate(particleSpread);
                    pParticle->SetVel(pRound->GetInheritsFirerVelocity() ? (m_Vel + particleVel) : particleVel);
					if (m_LegacyCompatibilityRoundsAlwaysFireUnflipped) {
						pParticle->SetRotAngle(particleVel.GetAbsRadAngle());
					} else {
						pParticle->SetRotAngle(particleVel.GetAbsRadAngle() + (m_HFlipped ? -c_PI : 0));
						pParticle->SetHFlipped(m_HFlipped);
					}
					if (lifeVariation != 0 && pParticle->GetLifetime() != 0) {
						pParticle->SetLifetime(std::max(static_cast<int>(pParticle->GetLifetime() * (1.0F + (particleCountMax > 1 ? lifeVariation - (lifeVariation * 2.0F * (static_cast<float>(pRound->ParticleCount()) / static_cast<float>(particleCountMax - 1))) : lifeVariation * RandomNormalNum()))), 1));
					}
                    // F = m * a
                    totalFireForce += pParticle->GetMass() * pParticle->GetVel().GetMagnitude();

                    // Remove from parent if it's an attachable
                    Attachable *pAttachable = dynamic_cast<Attachable *>(pParticle);
                    if (pAttachable) {
                        if (pAttachable->IsAttached()) { 
                            pAttachable->GetParent()->RemoveAttachable(pAttachable); 
                        }

                        // Activate if it is some kind of grenade or whatnot.
                        ThrownDevice *pTD = dynamic_cast<ThrownDevice *>(pAttachable);
                        if (pTD) {
                            pTD->Activate();
                        }
                    }

                    // Set the fired particle to not hit this HeldDevice's parent, if applicable
                    if (m_FireIgnoresThis)
                        pParticle->SetWhichMOToNotHit(this, 1.0f);

                    // Set the team so alarm events that happen if these gib won't freak out the guy firing
                    pParticle->SetTeam(m_Team);

                    // Also make this not hit team members
					// TODO: Don't hardcode this???
                    pParticle->SetIgnoresTeamHits(true);

                    // Decide for how long until the bullet tumble and start to lose lethality
                    pPixel = dynamic_cast<MOPixel *>(pParticle);
					if (pPixel) {
						// Stray bullets heavily affected by bullet shake lose lethality quicker, as if missing on an imaginary "Z" axis
						lethalRange *= std::max(1.0F - std::abs(shake) / 20.0F, 0.1F);
						pPixel->SetLethalRange(lethalRange);
					}
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
                    shellVel.SetXY(pRound->GetShellVel() * (1.0F - RandomNum(0.0F, m_ShellVelVariation)), 0);
                    shellVel.DegRotate(degAimAngle + m_ShellEjectAngle * (m_HFlipped ? -1 : 1) + shellSpread);
                    pShell->SetVel(m_Vel + shellVel);
                    pShell->SetRotAngle(m_Rotation.GetRadAngle());
                    pShell->SetAngularVel(pShell->GetAngularVel() + (m_ShellAngVelRange * RandomNormalNum()));
					pShell->SetHFlipped(m_HFlipped);
					// Set the ejected shell to not hit this HeldDevice's parent, if applicable
					if (m_FireIgnoresThis)
						pShell->SetWhichMOToNotHit(this, 1.0f);
                    // Set the team so alarm events that happen if these gib won't freak out the guy firing
                    pShell->SetTeam(m_Team);
					// Set this to ignore team hits in case it's lethal
					// TODO: Don't hardcode this???
					pShell->SetIgnoresTeamHits(true);
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

			if (m_FireFrame) { RunScriptedFunctionInAppropriateScripts("OnFire", false, false); }
		} else {
			m_ActivationTimer.Reset();
		}
	} else if (m_Activated && !m_AlreadyClicked) {
        // Play empty pin click sound.
		if (m_EmptySound) { m_EmptySound->Play(m_Pos); }
        // Indicate that we have clicked once during the current activation. 
        m_AlreadyClicked = true;
    }

    if (m_Reloading && m_ReloadEndSound) {
        // x0.5 the sound length generally just lines up better and leaves less dead air assuming a normal attempt at a ReloadEnd sound
        float offsetMilliseconds = m_ReloadEndOffset == -1.0F ? m_ReloadEndSound->GetLength(SoundContainer::LengthOfSoundType::NextPlayed) * 0.5f : m_ReloadEndOffset;
        bool shouldPlay = !m_HasPlayedEndReloadSound && m_ReloadTmr.LeftTillSimTimeLimitMS() <= offsetMilliseconds;
        if (shouldPlay) {
            m_ReloadEndSound->Play(m_Pos);
            m_HasPlayedEndReloadSound = true;
        }
    }

	if (m_Reloading && !m_pMagazine && m_pMagazineReference && m_ReloadTmr.IsPastSimTimeLimit()) {
		SetMagazine(dynamic_cast<Magazine *>(m_pMagazineReference->Clone()));

		m_ActivationTimer.Reset();
		m_LastFireTmr.Reset();

		if (m_PreFireSound && m_Activated) { 
            m_PreFireSound->Play(); 
        }

        m_HasPlayedEndReloadSound = false;
		m_Reloading = false;
		m_DoneReloading = true;
	}

    // Do stuff to deactivate after being activated
    if (!m_Activated)
    {
        // Reset the click indicator.
        m_AlreadyClicked = false;

        // Stop any looping activation sounds
        if (m_FireSound && m_FireSound->GetLoopSetting() == -1)// && m_FireSound->IsBeingPlayed())
            m_FireSound->Stop();
    }

    //////////////////////////////////////////////
    // Recoil and other activation effects logic.

	// TODO: don't use arbitrary numbers?
	m_RecoilForce.SetMagnitude(std::max(m_RecoilForce.GetMagnitude() * 0.7F - 1.0F, 0.0F));

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
			m_RecoilOffset.SetMagnitude(std::min(m_RecoilOffset.GetMagnitude(), 1.0F));
        }

        // Screen shake
        if (pActor) {
            int controllingPlayer = pActor->GetController()->GetPlayer();
            int screenId = g_ActivityMan.GetActivity()->ScreenOfPlayer(controllingPlayer);
            if (screenId != -1) {
                const float shakiness = g_CameraMan.GetDefaultShakePerUnitOfRecoilEnergy();
                const float maxShakiness = g_CameraMan.GetDefaultShakeFromRecoilMaximum(); // Some weapons fire huge rounds, so restrict the amount
                float screenShakeAmount = m_RecoilScreenShakeAmount == -1.0F ? std::min(totalFireForce * m_JointStiffness * shakiness, maxShakiness) : m_RecoilScreenShakeAmount;
                g_CameraMan.ApplyScreenShake(screenShakeAmount, screenId);
            }
        }

        AddImpulseForce(m_RecoilForce, m_RecoilOffset);

        // Display gun animation
		if (!m_IsAnimatedManually && m_FrameCount > 1) { m_Frame = 1; }

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

		if (m_Loudness > 0) { g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, m_Team, m_Loudness)); }
    } else {
        m_Recoiled = false;
		if (!m_IsAnimatedManually) { m_Frame = 0; }
    }

    // Display and override gun animation if there's a special one
    if (m_FrameCount > 1)
    {
        if (m_SpriteAnimMode == LOOPWHENACTIVE)
        {
            if (m_Activated || m_LastFireTmr.GetElapsedSimTimeMS() < m_DeactivationDelay) {
                // Max rate of the animation when fully activated and firing
                int animDuration = m_SpriteAnimDuration;
                // Spin up - can only spin up if mag is inserted
                if (m_Activated && !m_Reloading && m_ActivationTimer.GetElapsedSimTimeMS() < m_ActivationDelay)
                {
                    animDuration = (int)Lerp(0, m_ActivationDelay, (float)(m_SpriteAnimDuration * 10), (float)m_SpriteAnimDuration, m_ActivationTimer.GetElapsedSimTimeMS());
					if (m_ActiveSound) { m_ActiveSound->SetPitch(Lerp(0, m_ActivationDelay, 0, 1.0, m_ActivationTimer.GetElapsedSimTimeMS())); }
                }
                // Spin down
                if ((!m_Activated || m_Reloading) && m_LastFireTmr.GetElapsedSimTimeMS() < m_DeactivationDelay)
                {
                    animDuration = (int)Lerp(0, m_DeactivationDelay, (float)m_SpriteAnimDuration, (float)(m_SpriteAnimDuration * 10), m_LastFireTmr.GetElapsedSimTimeMS());
					if (m_ActiveSound) { m_ActiveSound->SetPitch(Lerp(0, m_DeactivationDelay, 1.0, 0, m_LastFireTmr.GetElapsedSimTimeMS())); }
                }

                if (animDuration > 0 && !(m_Reloading && m_LastFireTmr.GetElapsedSimTimeMS() >= m_DeactivationDelay)) {
                    float cycleTime = ((long)m_SpriteAnimTimer.GetElapsedSimTimeMS()) % animDuration;
					if (!m_IsAnimatedManually)
	                    m_Frame = std::floor((cycleTime / (float)animDuration) * (float)m_FrameCount);
				} else {
					StopActivationSound();
				}
            } else {
				if (!m_IsAnimatedManually) { m_Frame = 0; }
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

    // Set the screen flash effect to draw at the final post processing stage
    if (m_FireFrame && m_pFlash && m_pFlash->GetScreenEffect()) {
        Vector muzzlePos = m_Pos + RotateOffset(m_MuzzleOff + Vector(m_pFlash->GetSpriteWidth() * 0.3F, 0));
        if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(muzzlePos)) {
            g_PostProcessMan.RegisterPostEffect(muzzlePos, m_pFlash->GetScreenEffect(), m_pFlash->GetScreenEffectHash(), RandomNum(m_pFlash->GetEffectStopStrength(), m_pFlash->GetEffectStartStrength()), m_pFlash->GetEffectRotAngle());
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EstimateDigStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Estimates what material strength the rounds in the magazine can destroy. 

float HDFirearm::EstimateDigStrength() const {
	return m_pMagazine ? m_pMagazine->EstimateDigStrength() : m_pMagazineReference->EstimateDigStrength();
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
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws an aiming aid in front of this HeldDevice.

void HDFirearm::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int whichScreen, bool playerControlled)
{
    if (!m_HUDVisible) {
        return;
    }

    // Only draw if the team viewing this is on the same team OR has seen the space where this is located
    int viewingTeam = g_ActivityMan.GetActivity()->GetTeamOfPlayer(g_ActivityMan.GetActivity()->PlayerOfScreen(whichScreen));
    if (viewingTeam != m_Team && viewingTeam != Activity::NoTeam && g_SceneMan.IsUnseen(m_Pos.m_X, m_Pos.m_Y, viewingTeam)) {
        return;
    }

    HeldDevice::DrawHUD(pTargetBitmap, targetPos, whichScreen);

	if (!m_Parent || IsReloading() || m_MaxSharpLength == 0) {
		return;
	}

	float sharpLength = std::max(m_MaxSharpLength * m_SharpAim, 20.0F);
	int glowStrength = RandomNum(95, 159);
	int pointCount;
	if (playerControlled && sharpLength > 20.0F) {
		pointCount = m_SharpAim > 0.5F ? 4 : 3;
	} else {
		pointCount = 2;
	}

	int pointSpacing = 10 - pointCount;
	sharpLength -= static_cast<float>(pointSpacing * pointCount) * 0.5F;
	Vector muzzleOffset(std::max(m_MuzzleOff.m_X, m_SpriteRadius), m_MuzzleOff.m_Y);

	for (int i = 0; i < pointCount; ++i) {
		Vector aimPoint(sharpLength + static_cast<float>(pointSpacing * i), 0);
		aimPoint = RotateOffset(aimPoint + muzzleOffset) + m_Pos;

		g_PostProcessMan.RegisterGlowDotEffect(aimPoint, YellowDot, glowStrength);
		aimPoint -= targetPos;
		g_SceneMan.WrapPosition(aimPoint);
		putpixel(pTargetBitmap, aimPoint.GetFloorIntX(), aimPoint.GetFloorIntY(), g_YellowGlowColor);
	}
}

} // namespace RTE
