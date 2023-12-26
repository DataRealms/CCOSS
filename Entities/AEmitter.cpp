//////////////////////////////////////////////////////////////////////////////////////////
// File:            AEmitter.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the AEmitter class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AEmitter.h"
#include "Atom.h"
#include "Emission.h"
#include "PresetMan.h"

namespace RTE {

ConcreteClassInfo(AEmitter, Attachable, 100);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AEmitter, effectively
//                  resetting the members of this abstraction level only.

void AEmitter::Clear()
{
    m_EmissionList.clear();
    m_EmissionSound = nullptr;
    m_BurstSound = nullptr;
	m_EndSound = nullptr;
    m_EmitEnabled = false;
    m_WasEmitting = false;
    m_EmitCount = 0;
    m_EmitCountLimit = 0;
	m_NegativeThrottleMultiplier = 1.0F;
	m_PositiveThrottleMultiplier = 1.0F;
    m_Throttle = 0;
    m_EmissionsIgnoreThis = false;
    m_BurstScale = 1.0F;
    m_BurstDamage = 0;
	m_EmitterDamageMultiplier = 1.0F;
    m_BurstTriggered = false;
    m_BurstSpacing = 0;
    // Set this to really long so an initial burst will be possible
    m_BurstTimer.SetElapsedSimTimeS(50000);
    m_BurstTimer.SetElapsedRealTimeS(50000);
    m_EmitAngle.Reset();
    m_EmissionOffset.Reset();
    m_EmitDamage = 0;
    m_LastEmitTmr.Reset();
    m_pFlash = 0;
    m_FlashScale = 1.0F;
    m_AvgBurstImpulse = -1.0F;
    m_AvgImpulse = -1.0F;
    m_FlashOnlyOnBurst = true;
    m_SustainBurstSound = false;
    m_BurstSoundFollowsEmitter = true;
    m_LoudnessOnEmit = 1.0F;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AEmitter to be identical to another, by deep copy.

int AEmitter::Create(const AEmitter &reference) {
    if (reference.m_pFlash) { m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFlash->GetUniqueID()); }

    Attachable::Create(reference);

    if (reference.m_pFlash) { SetFlash(dynamic_cast<Attachable *>(reference.m_pFlash->Clone())); }

    for (auto itr = reference.m_EmissionList.begin(); itr != reference.m_EmissionList.end(); ++itr) {
		m_EmissionList.push_back(*itr);
	}
	if (reference.m_EmissionSound) { m_EmissionSound = dynamic_cast<SoundContainer*>(reference.m_EmissionSound->Clone()); }
	if (reference.m_BurstSound) { m_BurstSound = dynamic_cast<SoundContainer*>(reference.m_BurstSound->Clone()); }
	if (reference.m_EndSound) { m_EndSound = dynamic_cast<SoundContainer*>(reference.m_EndSound->Clone()); }
	m_EmitEnabled = reference.m_EmitEnabled;
    m_EmitCount = reference.m_EmitCount;
    m_EmitCountLimit = reference.m_EmitCountLimit;
    m_NegativeThrottleMultiplier = reference.m_NegativeThrottleMultiplier;
    m_PositiveThrottleMultiplier = reference.m_PositiveThrottleMultiplier;
    m_Throttle = reference.m_Throttle;
    m_EmissionsIgnoreThis = reference.m_EmissionsIgnoreThis;
    m_BurstScale = reference.m_BurstScale;
    m_BurstDamage = reference.m_BurstDamage;
	m_EmitterDamageMultiplier = reference.m_EmitterDamageMultiplier;
    m_BurstSpacing = reference.m_BurstSpacing;
    m_BurstTriggered = reference.m_BurstTriggered;
    m_EmitAngle = reference.m_EmitAngle;
    m_EmissionOffset = reference.m_EmissionOffset;
    m_EmitDamage = reference.m_EmitDamage;
    m_FlashScale = reference.m_FlashScale;
    m_FlashOnlyOnBurst = reference.m_FlashOnlyOnBurst;
    m_SustainBurstSound = reference.m_SustainBurstSound;
    m_BurstSoundFollowsEmitter = reference.m_BurstSoundFollowsEmitter;
    m_LoudnessOnEmit = reference.m_LoudnessOnEmit;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int AEmitter::ReadProperty(const std::string_view &propName, Reader &reader) {
    StartPropertyList(return Attachable::ReadProperty(propName, reader));
    
    MatchProperty("AddEmission", {
        Emission emission;
        reader >> emission;
        m_EmissionList.push_back(emission);
    });
    MatchProperty("EmissionSound", {
		m_EmissionSound = new SoundContainer;
        reader >> m_EmissionSound;
    });
    MatchProperty("BurstSound", {
		m_BurstSound = new SoundContainer;
        reader >> m_BurstSound;
    });
    MatchProperty("EndSound", {
		m_EndSound = new SoundContainer;
        reader >> m_EndSound;
    });
    MatchProperty("EmissionEnabled", { reader >> m_EmitEnabled; });
    MatchProperty("EmissionCount", { reader >> m_EmitCount; });
    MatchProperty("EmissionCountLimit", { reader >> m_EmitCountLimit; });
    MatchProperty("ParticlesPerMinute", {
        float ppm;
        reader >> ppm;
        // Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
        for (Emission &emission : m_EmissionList) { emission.m_PPM = ppm / static_cast<float>(m_EmissionList.size()); }
    });
    MatchProperty("NegativeThrottleMultiplier", { reader >> m_NegativeThrottleMultiplier; });
    MatchProperty("PositiveThrottleMultiplier", { reader >> m_PositiveThrottleMultiplier; });
    MatchProperty("Throttle", { reader >> m_Throttle; });
    MatchProperty("EmissionsIgnoreThis", { reader >> m_EmissionsIgnoreThis; });
    MatchProperty("BurstSize", {
        int burstSize;
        reader >> burstSize;
        // Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
        for (Emission &emission : m_EmissionList) { emission.m_BurstSize = std::ceil(static_cast<float>(burstSize) / static_cast<float>(m_EmissionList.size())); }
    });
    MatchProperty("BurstScale", { reader >> m_BurstScale; });
    MatchProperty("BurstDamage", { reader >> m_BurstDamage; });
    MatchProperty("EmitterDamageMultiplier", { reader >> m_EmitterDamageMultiplier; });
    MatchProperty("BurstSpacing", { reader >> m_BurstSpacing; });
    MatchProperty("BurstTriggered", { reader >> m_BurstTriggered; });
    MatchProperty("EmissionAngle", { reader >> m_EmitAngle; });
    MatchProperty("EmissionOffset", { reader >> m_EmissionOffset; });
    MatchProperty("EmissionDamage", { reader >> m_EmitDamage; });
    MatchProperty("Flash", { SetFlash(dynamic_cast<Attachable *>(g_PresetMan.ReadReflectedPreset(reader))); });
    MatchProperty("FlashScale", { reader >> m_FlashScale; });
    MatchProperty("FlashOnlyOnBurst", { reader >> m_FlashOnlyOnBurst; });
    MatchProperty("SustainBurstSound", { reader >> m_SustainBurstSound; });
    MatchProperty("BurstSoundFollowsEmitter", { reader >> m_BurstSoundFollowsEmitter; });
    MatchProperty("LoudnessOnEmit", { reader >> m_LoudnessOnEmit; });

    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AEmitter with a Writer for
//                  later recreation with Create(Reader &reader);

int AEmitter::Save(Writer &writer) const
{
    Attachable::Save(writer);

    for (auto itr = m_EmissionList.begin(); itr != m_EmissionList.end(); ++itr)
    {
        writer.NewProperty("AddEmission");
        writer << *itr;
    }
    writer.NewProperty("EmissionSound");
    writer << m_EmissionSound;
    writer.NewProperty("BurstSound");
    writer << m_BurstSound;
    writer.NewProperty("EndSound");
    writer << m_EndSound;
    writer.NewProperty("EmissionEnabled");
    writer << m_EmitEnabled;
    writer.NewProperty("EmissionCount");
    writer << m_EmitCount;
    writer.NewProperty("EmissionCountLimit");
    writer << m_EmitCountLimit;
    writer.NewProperty("EmissionsIgnoreThis");
    writer << m_EmissionsIgnoreThis;
    writer.NewProperty("NegativeThrottleMultiplier");
    writer << m_NegativeThrottleMultiplier;
    writer.NewProperty("PositiveThrottleMultiplier");
    writer << m_PositiveThrottleMultiplier;
    writer.NewProperty("Throttle");
    writer << m_Throttle;
    writer.NewProperty("BurstScale");
    writer << m_BurstScale;
    writer.NewProperty("BurstDamage");
    writer << m_BurstDamage;
	writer.NewProperty("EmitterDamageMultiplier");
	writer << m_EmitterDamageMultiplier;
    writer.NewProperty("BurstSpacing");
    writer << m_BurstSpacing;
    writer.NewProperty("BurstTriggered");
    writer << m_BurstTriggered;
    writer.NewProperty("EmissionAngle");
    writer << m_EmitAngle;
    writer.NewProperty("EmissionOffset");
    writer << m_EmissionOffset;
    writer.NewProperty("EmissionDamage");
    writer << m_EmitDamage;
    writer.NewProperty("Flash");
    writer << m_pFlash;
    writer.NewProperty("FlashScale");
    writer << m_FlashScale;
    writer.NewProperty("FlashOnlyOnBurst");
    writer << m_FlashOnlyOnBurst;
    writer.NewProperty("SustainBurstSound");
	writer << m_SustainBurstSound;
    writer.NewProperty("BurstSoundFollowsEmitter");
	writer << m_BurstSoundFollowsEmitter;
    writer.NewProperty("LoudnessOnEmit");
    writer << m_LoudnessOnEmit;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AEmitter object.

void AEmitter::Destroy(bool notInherited)
{
    // Stop playback of sounds gracefully
	if (m_EmissionSound) {
		if (m_EndSound) { m_EmissionSound->IsBeingPlayed() ? m_EndSound->Play(m_Pos) : m_EndSound->Stop(); }
		m_EmissionSound->Stop();
	}

	delete m_EmissionSound;
	delete m_BurstSound;
	delete m_EndSound;

//    m_BurstSound.Stop();

    if (!notInherited)
        Attachable::Destroy();
    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetEmissionTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reset the timers of all emissions so they will start/stop at the
//                  correct relative offsets from now.

void AEmitter::ResetEmissionTimers()
{
    m_LastEmitTmr.Reset();
    for (auto eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
        (*eItr).ResetEmissionTimers();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableEmission
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this AEmitter to start emitting at the set rate, or to stop.

void AEmitter::EnableEmission(bool enable)
{
    if (!m_EmitEnabled && enable)
    {
        m_LastEmitTmr.Reset();
        // Reset counter
        m_EmitCount = 0;
        // Reset animation
        m_Frame = 0;
    }
    m_EmitEnabled = enable;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EstimateImpulse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the forces this emitter applies on any parent.

float AEmitter::EstimateImpulse(bool burst)
{
    // Calculate the impulse generated by the emissions, once and store the result
    if ((!burst && m_AvgImpulse < 0) || (burst && m_AvgBurstImpulse < 0))
    {
        float impulse = 0;
        float velMin, velMax, velRange, spread;

        // Go through all emissions and emit them according to their respective rates
        for (auto eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
        {
            // Only check emissions that push the emitter
            if ((*eItr).PushesEmitter())
            {
                double emissions = (*eItr).GetRate() * g_TimerMan.GetDeltaTimeSecs() / 60.0f;
                if (burst) {
                    emissions *= (*eItr).GetBurstSize();
                }
                
                velMin = std::min((*eItr).GetMinVelocity(), (*eItr).GetMaxVelocity());
                velMax = std::max((*eItr).GetMinVelocity(), (*eItr).GetMaxVelocity());
                velRange = (velMax - velMin) * 0.5;
                spread = std::max(static_cast<float>(c_PI) - (*eItr).GetSpread(), .0f) / c_PI;     // A large spread will cause the forces to cancel eachother out

                // Add to accumulative recoil impulse generated, F = m * a.
                impulse += (velMin + velRange) * spread * (*eItr).m_pEmission->GetMass() * emissions;
            }
        }

        if (burst)
            m_AvgBurstImpulse = impulse;
        else
            m_AvgImpulse = impulse;

    }

	// Scale the emission rate up or down according to the appropriate throttle multiplier.
	float throttleFactor = GetThrottleFactor();
    // Apply the throttle factor to the emission rate per update
	if (burst) { return m_AvgBurstImpulse * throttleFactor; }

	return m_AvgImpulse * throttleFactor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AEmitter::GetTotalParticlesPerMinute() const {
	float totalPPM = 0;
	for (const Emission &emission : m_EmissionList) {
		totalPPM += emission.m_PPM;
	}
	return totalPPM;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AEmitter::GetTotalBurstSize() const {
	int totalBurstSize = 0;
	for (const Emission &emission : m_EmissionList) {
		totalBurstSize += emission.m_BurstSize;
	}
	return totalBurstSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AEmitter::GetScaledThrottle(float throttle, float multiplier) const {
    float throttleFactor = LERP(-1.0f, 1.0f, m_NegativeThrottleMultiplier, m_PositiveThrottleMultiplier, throttle);
    return LERP(m_NegativeThrottleMultiplier, m_PositiveThrottleMultiplier, -1.0f, 1.0f, throttleFactor * multiplier);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AEmitter::SetFlash(Attachable *newFlash) {
    if (m_pFlash && m_pFlash->IsAttached()) { RemoveAndDeleteAttachable(m_pFlash); }
    if (newFlash == nullptr) {
        m_pFlash = nullptr;
    } else {
        // Note - this is done here because setting mass on attached Attachables causes values to be updated on the parent (and its parent, and so on), which isn't ideal. Better to do it before the new flash is attached, so there are fewer calculations.
        newFlash->SetMass(0.0F);

        m_pFlash = newFlash;
        AddAttachable(newFlash);

        m_HardcodedAttachableUniqueIDsAndSetters.insert({newFlash->GetUniqueID(), [](MOSRotating *parent, Attachable *attachable) {
            dynamic_cast<AEmitter *>(parent)->SetFlash(attachable);
        }});

        m_pFlash->SetDrawnNormallyByParent(false);
        m_pFlash->SetInheritsRotAngle(false);
        m_pFlash->SetDeleteWhenRemovedFromParent(true);
        m_pFlash->SetCollidesWithTerrainWhileAttached(false);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this AEmitter. Supposed to be done every frame.

void AEmitter::Update()
{
    Attachable::PreUpdate();

    if (m_FrameCount > 1) {
        if (m_EmitEnabled && m_SpriteAnimMode == NOANIM) {
            m_SpriteAnimMode = ALWAYSLOOP;
        } else if (!m_EmitEnabled) {
            m_SpriteAnimMode = NOANIM;
            m_Frame = 0;
        }
    }

    // Update and show flash if there is one
    if (m_pFlash && (!m_FlashOnlyOnBurst || m_BurstTriggered)) {
        m_pFlash->SetParentOffset(m_EmissionOffset);
		m_pFlash->SetRotAngle(m_Rotation.GetRadAngle() + (m_EmitAngle.GetRadAngle() * GetFlipFactor()));
        m_pFlash->SetScale(m_FlashScale);
        m_pFlash->SetNextFrame();
    }

    Attachable::Update();

    if (m_BurstSoundFollowsEmitter && m_BurstSound) {
        m_BurstSound->SetPosition(m_Pos);
    }

    if (m_EmitEnabled)
    {
        if (!m_WasEmitting)
        {
            // Start playing the sound
			if (m_EmissionSound) { m_EmissionSound->Play(m_Pos); }

            // Reset the timers of all emissions so they will start/stop at the correct relative offsets from now
            for (Emission &emission : m_EmissionList)
                emission.ResetEmissionTimers();
        }
        // Update the distance attenuation
        else
			if (m_EmissionSound) { m_EmissionSound->SetPosition(m_Pos); }

        // Get the parent root of this AEmitter
// TODO: Potentially get this once outside instead, like in attach/detach")
        MovableObject *pRootParent = GetRootParent();

		float throttleFactor = GetThrottleFactor();
		m_FlashScale = throttleFactor;
        // Check burst triggering against whether the spacing is fulfilled
		if (m_BurstTriggered && CanTriggerBurst()) {
            // Play burst sound
			if (m_BurstSound) { m_BurstSound->Play(m_Pos); }
            // Start timing until next burst
            m_BurstTimer.Reset();
        }
        // Not enough spacing, cancel the triggering if there was any
        else
            m_BurstTriggered = false;

		int emissionCountTotal = 0;
        float velMin, velRange, spread;
        double currentPPM, SPE;
        MovableObject *pParticle = 0;
        Vector parentVel, emitVel, pushImpulses;
        // Go through all emissions and emit them according to their respective rates
        for (Emission &emission : m_EmissionList)
        {
            // Make sure the emissions only happen between the start time and end time
            if (emission.IsEmissionTime())
            {
                // Apply the throttle factor to the emission rate
                currentPPM = emission.GetRate() * throttleFactor;
				int emissionCount = 0;

				// Only do all this if the PPM is actually above zero
                if (currentPPM > 0)
                {
                    // Calculate secs per emission
                    SPE = 60.0 / currentPPM;

                    // Add the last elapsed time to the accumulator
                    emission.m_Accumulator += m_LastEmitTmr.GetElapsedSimTimeS();

                    // Now figure how many full emissions can fit in the current accumulator
					emissionCount = std::floor(emission.m_Accumulator / SPE);
                    // Deduct the about to be emitted emissions from the accumulator
                    emission.m_Accumulator -= emissionCount * SPE;

                    RTEAssert(emission.m_Accumulator >= 0, "Emission accumulator negative!");
                }
				else {
					emission.m_Accumulator = 0;
				}
				float scale = 1.0F;
                // Add extra emissions if bursting.
				if (m_BurstTriggered) {
					emissionCount += emission.GetBurstSize() * std::floor(throttleFactor);
					scale = m_BurstScale;
				}
				emissionCountTotal += emissionCount;
                pParticle = 0;
                emitVel.Reset();
                parentVel = pRootParent->GetVel() * emission.InheritsVelocity();

                for (int i = 0; i < emissionCount; ++i)
                {
                    velMin = emission.GetMinVelocity() * scale;
                    velRange = emission.GetMaxVelocity() - emission.GetMinVelocity() * scale;
                    spread = emission.GetSpread() * scale;
                    // Make a copy after the reference particle
                    pParticle = dynamic_cast<MovableObject *>(emission.GetEmissionParticlePreset()->Clone());
                    // Set up its position and velocity according to the parameters of this.
                    // Emission point offset not set

					if (emission.GetOffset().IsZero()) {
						if (m_EmissionOffset.IsZero()) {
							pParticle->SetPrevPos(m_Pos);
							pParticle->SetPos(m_Pos);
						} else {
                            pParticle->SetPrevPos(m_Pos);
							pParticle->SetPos(m_Pos + RotateOffset(m_EmissionOffset));
						}
					} else {
                        pParticle->SetPrevPos(m_Pos);
						pParticle->SetPos(m_Pos + RotateOffset((*eItr)->GetOffset()));
					}
    // TODO: Optimize making the random angles!")
                    emitVel.SetXY(velMin + RandomNum(0.0F, velRange), 0.0F);
					emitVel.RadRotate(m_EmitAngle.GetRadAngle() + spread * RandomNormalNum());
                    emitVel = RotateOffset(emitVel);
                    pParticle->SetVel(parentVel + emitVel);
					pParticle->SetRotAngle(emitVel.GetAbsRadAngle() + (m_HFlipped ? -c_PI : 0));
					pParticle->SetHFlipped(m_HFlipped);

					//Scale the particle's lifetime based on life variation and throttle, as long as it's not 0
					if (pParticle->GetLifetime() != 0) {
						pParticle->SetLifetime(std::max(static_cast<int>(static_cast<float>(pParticle->GetLifetime()) * (1.0F + (emission.GetLifeVariation() * RandomNormalNum()))), 1));
						pParticle->SetLifetime(std::max(static_cast<int>(pParticle->GetLifetime() * throttleFactor), 1));
					}
                    pParticle->SetTeam(m_Team);
                    pParticle->SetIgnoresTeamHits(true);

                    // Add to accumulative recoil impulse generated, F = m * a
                    // If enabled, that is
                    if (emission.PushesEmitter() && (GetParent() || GetMass() > 0)) { pushImpulses -= emitVel * pParticle->GetMass(); }

                    // Set the emitted particle to not hit this emitter's parent, if applicable
                    if (m_EmissionsIgnoreThis)
                        pParticle->SetWhichMOToNotHit(pRootParent);

                    // Let particle loose into the world!
                    g_MovableMan.AddMO(pParticle);
                    pParticle = 0;
                }
            }
        }
		m_LastEmitTmr.Reset();

        // Apply recoil/push effects. Joint stiffness will take effect when these are transferred to the parent.
        if (!pushImpulses.IsZero()) { AddImpulseForce(pushImpulses); }

        // Count the the damage caused by the emissions, and only if we're not bursting
		if (!m_BurstTriggered) {
			m_DamageCount += static_cast<float>(emissionCountTotal) * m_EmitDamage * m_EmitterDamageMultiplier;
		} else {	// Count the the damage caused by the burst
			m_DamageCount += m_BurstDamage * m_EmitterDamageMultiplier;
		}

		// Count the total emissions since enabling, and stop emitting if beyond limit (and limit is also enabled)
		m_EmitCount += emissionCountTotal;
		if (m_EmitCountLimit > 0 && m_EmitCount > m_EmitCountLimit) { EnableEmission(false); }

		if (m_BurstTriggered) { m_BurstTriggered = false; }

        m_WasEmitting = true;
    }
    // Do stuff to stop emission
	else
	{
		if (m_WasEmitting)
		{
			if (m_EmissionSound) { m_EmissionSound->Stop(); }
			if (m_BurstSound && !m_SustainBurstSound) { m_BurstSound->Stop(); }
			if (m_EndSound) { m_EndSound->Play(m_Pos); }
			m_WasEmitting = false;
		}
	}

    // Set the screen flash effect to draw at the final post processing stage
    if (m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered) && m_pFlash && m_pFlash->GetScreenEffect()) {
        // Fudge the glow pos forward a bit so it aligns nicely with the flash
        Vector emitPos(m_pFlash->GetScreenEffect()->w * 0.3F * m_FlashScale, 0);
        emitPos.RadRotate(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle());
        emitPos = m_Pos + RotateOffset(m_EmissionOffset) + emitPos;
        if (m_EffectAlwaysShows || !g_SceneMan.ObscuredPoint(emitPos)) {
            g_PostProcessMan.RegisterPostEffect(emitPos, m_pFlash->GetScreenEffect(), m_pFlash->GetScreenEffectHash(), RandomNum(m_pFlash->GetEffectStopStrength(), m_pFlash->GetEffectStartStrength()) * std::clamp(m_FlashScale, 0.0F, 1.0F), m_pFlash->GetEffectRotAngle());
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this AEmitter's current graphical representation to a
//                  BITMAP of choice.

void AEmitter::Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos,
                      DrawMode mode,
                      bool onlyPhysical) const
{
    // Draw flash if there is one
    if (m_pFlash && !m_pFlash->IsDrawnAfterParent() &&
        !onlyPhysical && mode == g_DrawColor && m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered))
        m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    Attachable::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);

    // Update and Draw flash if there is one
    if (m_pFlash && m_pFlash->IsDrawnAfterParent() &&
        !onlyPhysical && mode == g_DrawColor && m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered))
        m_pFlash->Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
}

} // namespace RTE