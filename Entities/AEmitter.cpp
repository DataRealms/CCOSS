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

ConcreteClassInfo(AEmitter, Attachable, 100)

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AEmitter, effectively
//                  resetting the members of this abstraction level only.

void AEmitter::Clear()
{
	for (list<Emission *>::const_iterator itr = m_EmissionList.begin(); itr != m_EmissionList.end(); ++itr)
		delete (*itr);
    m_EmissionList.clear();
    m_EmissionSound = nullptr;
    m_BurstSound = nullptr;
	m_EndSound = nullptr;
    m_EmitEnabled = false;
    m_WasEmitting = false;
    m_EmitCount = 0;
    m_EmitCountLimit = 0;
    m_MinThrottleRange = -1;
    m_MaxThrottleRange = 1;
    m_Throttle = 0;
    m_EmissionsIgnoreThis = false;
    m_BurstScale = 1.0;
    m_BurstDamage = 0;
	m_EmitterDamageMultiplier = 1.0;
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
    m_LoudnessOnEmit = 1.0F;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a AEmitter to be identical to another, by deep copy.

int AEmitter::Create(const AEmitter &reference) {
    if (reference.m_pFlash) {
        m_ReferenceHardcodedAttachableUniqueIDs.insert(reference.m_pFlash->GetUniqueID());
        SetFlash(dynamic_cast<Attachable *>(reference.m_pFlash->Clone()));
    }
    Attachable::Create(reference);

    for (const Emission *referenceEmission : reference.m_EmissionList) {
        m_EmissionList.push_back(dynamic_cast<Emission *>(referenceEmission->Clone()));
    }

	if (reference.m_EmissionSound) { m_EmissionSound = dynamic_cast<SoundContainer*>(reference.m_EmissionSound->Clone()); }
	if (reference.m_BurstSound) { m_BurstSound = dynamic_cast<SoundContainer*>(reference.m_BurstSound->Clone()); }
	if (reference.m_EndSound) { m_EndSound = dynamic_cast<SoundContainer*>(reference.m_EndSound->Clone()); }
	m_EmitEnabled = reference.m_EmitEnabled;
    m_EmitCount = reference.m_EmitCount;
    m_EmitCountLimit = reference.m_EmitCountLimit;
    m_MinThrottleRange = reference.m_MinThrottleRange;
    m_MaxThrottleRange = reference.m_MaxThrottleRange;
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
    if (propName == "AddEmission") {
        Emission * emission = new Emission();
        reader >> *emission;
        m_EmissionList.push_back(emission);
    } else if (propName == "EmissionSound") {
		m_EmissionSound = new SoundContainer;
        reader >> m_EmissionSound;
    } else if (propName == "BurstSound") {
		m_BurstSound = new SoundContainer;
        reader >> m_BurstSound;
    } else if (propName == "EndSound") {
		m_EndSound = new SoundContainer;
        reader >> m_EndSound;
    } else if (propName == "EmissionEnabled") {
        reader >> m_EmitEnabled;
    } else if (propName == "EmissionCount") {
        reader >> m_EmitCount;
    } else if (propName == "EmissionCountLimit") {
        reader >> m_EmitCountLimit;
    } else if (propName == "ParticlesPerMinute") {
        float ppm;
        reader >> ppm;
        // Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
        for (Emission *emission : m_EmissionList) { emission->m_PPM = ppm / static_cast<float>(m_EmissionList.size()); }
    } else if (propName == "MinThrottleRange") {
        reader >> m_MinThrottleRange;
    } else if (propName == "MaxThrottleRange") {
        reader >> m_MaxThrottleRange;
    } else if (propName == "Throttle") {
        reader >> m_Throttle;
    } else if (propName == "EmissionsIgnoreThis") {
        reader >> m_EmissionsIgnoreThis;
    } else if (propName == "BurstSize") {
        int burstSize;
        reader >> burstSize;
        // Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility.
        for (Emission *emission : m_EmissionList) { emission->m_BurstSize = std::ceil(static_cast<float>(burstSize) / static_cast<float>(m_EmissionList.size())); }
    } else if (propName == "BurstScale") {
        reader >> m_BurstScale;
    } else if (propName == "BurstDamage") {
        reader >> m_BurstDamage;
    } else if (propName == "EmitterDamageMultiplier") {
		reader >> m_EmitterDamageMultiplier;
    } else if (propName == "BurstSpacing") {
        reader >> m_BurstSpacing;
    } else if (propName == "BurstTriggered") {
        reader >> m_BurstTriggered;
    } else if (propName == "EmissionAngle") {
        reader >> m_EmitAngle;
    } else if (propName == "EmissionOffset") {
        reader >> m_EmissionOffset;
    } else if (propName == "EmissionDamage") {
        reader >> m_EmitDamage;
    } else if (propName == "Flash") {
        Attachable iniDefinedObject;
        reader >> &iniDefinedObject;
        SetFlash(dynamic_cast<Attachable *>(iniDefinedObject.Clone()));
    } else if (propName == "FlashScale") {
        reader >> m_FlashScale;
    } else if (propName == "FlashOnlyOnBurst") {
        reader >> m_FlashOnlyOnBurst;
    } else if (propName == "LoudnessOnEmit") {
        reader >> m_LoudnessOnEmit;
    } else {
        return Attachable::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this AEmitter with a Writer for
//                  later recreation with Create(Reader &reader);

int AEmitter::Save(Writer &writer) const
{
    Attachable::Save(writer);

    for (list<Emission *>::const_iterator itr = m_EmissionList.begin(); itr != m_EmissionList.end(); ++itr)
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
    writer.NewProperty("MinThrottleRange");
    writer << m_MinThrottleRange;
    writer.NewProperty("MaxThrottleRange");
    writer << m_MaxThrottleRange;
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
    for (list<Emission *>::iterator eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
        (*eItr)->ResetEmissionTimers();
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
        for (list<Emission *>::iterator eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
        {
            // Only check emissions that push the emitter
            if ((*eItr)->PushesEmitter())
            {
                double emissions = (*eItr)->GetRate() * g_TimerMan.GetDeltaTimeSecs() / 60.0f;
                if (burst)
                    emissions *= (*eItr)->GetBurstSize();
                
                velMin = min((*eItr)->GetMinVelocity(), (*eItr)->GetMaxVelocity());
                velMax = max((*eItr)->GetMinVelocity(), (*eItr)->GetMaxVelocity());
                velRange = (velMax - velMin) * 0.5;
                spread = max(static_cast<float>(c_PI) - (*eItr)->GetSpread(), .0f) / c_PI;     // A large spread will cause the forces to cancel eachother out

                // Add to accumulative recoil impulse generated, F = m * a.
                impulse += (velMin + velRange) * spread * (*eItr)->m_pEmission->GetMass() * emissions;
            }
        }

        if (burst)
            m_AvgBurstImpulse = impulse;
        else
            m_AvgImpulse = impulse;

    }

    // Figure out the throttle factor
    float throttleFactor = 1.0f;
    if (m_Throttle < 0)         // Negative throttle, scale down according to the min throttle range
        throttleFactor += fabs(m_MinThrottleRange) * m_Throttle;
    else if (m_Throttle > 0)    // Positive throttle, scale up
        throttleFactor += fabs(m_MaxThrottleRange) * m_Throttle;
    
    // Apply the throttle factor to the emission rate per update
    if (burst)
        return m_AvgBurstImpulse * throttleFactor;
    
    return m_AvgImpulse * throttleFactor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AEmitter::SetFlash(Attachable *newFlash) {
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
            dynamic_cast<AEmitter *>(parent)->SetFlash(attachable);
        }});

        m_pFlash->SetDrawnNormallyByParent(false);
        m_pFlash->SetInheritsRotAngle(false);
        m_pFlash->SetInheritsHFlipped(0);
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

// TODO: Really hardcode this?
    // Set animation to loop if emitting
    if (m_FrameCount > 1)
    {
        m_SpriteAnimMode = m_EmitEnabled ? ALWAYSLOOP : NOANIM;
        if (!m_EmitEnabled)
            m_Frame = 0;
    }

    // Update and show flash if there is one
    if (m_pFlash && (!m_FlashOnlyOnBurst || m_BurstTriggered)) {
        m_pFlash->SetParentOffset(m_EmissionOffset);
        // Don't set the flipping for the flash because that is wasting resources when drawing, just handle the flipping of the rotation here.
        m_pFlash->SetRotAngle(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle());
        m_pFlash->SetScale(m_FlashScale);
        m_pFlash->SetNextFrame();
    }

    Attachable::Update();

    if (m_EmitEnabled)
    {
        if (!m_WasEmitting)
        {
            // Start playing the sound
			if (m_EmissionSound) { m_EmissionSound->Play(m_Pos); }

            // Reset the timers of all emissions so they will start/stop at the correct relative offsets from now
            for (list<Emission *>::iterator eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
                (*eItr)->ResetEmissionTimers();
        }
        // Update the distance attenuation
        else
			if (m_EmissionSound) { m_EmissionSound->SetPosition(m_Pos); }

        // Get the parent root of this AEmitter
// TODO: Potentially get this once outside instead, like in attach/detach")
        MovableObject *pRootParent = GetRootParent();

        // Figure out the throttle factor
        // Negative throttle, scale down according to the min throttle range
        float throttleFactor = 1.0f;
        if (m_Throttle < 0)
            throttleFactor += fabs(m_MinThrottleRange) * m_Throttle;
        // Positive throttle, scale up
        else if (m_Throttle > 0)
            throttleFactor += fabs(m_MaxThrottleRange) * m_Throttle;

        // Check burst triggering against whether the spacing is fulfilled
        if (m_BurstTriggered && (m_BurstSpacing <= 0 || m_BurstTimer.IsPastSimMS(m_BurstSpacing)))
        {
            // Play burst sound
			if (m_BurstSound) { m_BurstSound->Play(m_Pos); }
            // Start timing until next burst
            m_BurstTimer.Reset();
        }
        // Not enough spacing, cancel the triggering if there was any
        else
            m_BurstTriggered = false;

        int emissions = 0;
        float velMin, velRange, spread;
        double currentPPM, SPE;
        MovableObject *pParticle = 0;
        Vector parentVel, emitVel, pushImpulses;
        // Go through all emissions and emit them according to their respective rates
        for (list<Emission *>::iterator eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
        {
            // Make sure the emissions only happen between the start time and end time
            if ((*eItr)->IsEmissionTime())
            {
                // Apply the throttle factor to the emission rate
                currentPPM = (*eItr)->GetRate() * throttleFactor;
                emissions = 0;

                // Only do all this if the PPM is acutally above zero
                if (currentPPM > 0)
                {
                    // Calculate secs per emission
                    SPE = 60.0 / currentPPM;

                    // Add the last elapsed time to the accumulator
                    (*eItr)->m_Accumulator += m_LastEmitTmr.GetElapsedSimTimeS();

                    // Now figure how many full emissions can fit in the current accumulator
                    emissions = floor((*eItr)->m_Accumulator / SPE);
                    // Deduct the about to be emitted emissions from the accumulator
                    (*eItr)->m_Accumulator -= emissions * SPE;

                    RTEAssert((*eItr)->m_Accumulator >= 0, "Emission accumulator negative!");
                }

                // Add extra emissions if bursting.
                if (m_BurstTriggered)
                    emissions += (*eItr)->GetBurstSize();

                pParticle = 0;
                emitVel.Reset();
                parentVel = pRootParent->GetVel() * (*eItr)->InheritsVelocity();

                for (int i = 0; i < emissions; ++i)
                {
                    velMin = (*eItr)->GetMinVelocity() * (m_BurstTriggered ? m_BurstScale : 1.0);
                    velRange = (*eItr)->GetMaxVelocity() - (*eItr)->GetMinVelocity() * (m_BurstTriggered ? m_BurstScale : 1.0);
                    spread = (*eItr)->GetSpread() * (m_BurstTriggered ? m_BurstScale : 1.0);
                    // Make a copy after the reference particle
                    pParticle = dynamic_cast<MovableObject *>((*eItr)->GetEmissionParticlePreset()->Clone());
                    // Set up its position and velocity according to the parameters of this.
                    // Emission point offset not set

					if ((*eItr)->GetOffset().IsZero())
					{
						if (m_EmissionOffset.IsZero())
							pParticle->SetPos(m_Pos/*Vector(m_Pos.m_X + 5 * NormalRand(), m_Pos.m_Y + 5 * NormalRand())*/);
						else
							pParticle->SetPos(m_Pos + RotateOffset(m_EmissionOffset));
					}
					else 
					{
						pParticle->SetPos(m_Pos + RotateOffset((*eItr)->GetOffset()));
					}
    // TODO: Optimize making the random angles!")
                    emitVel.SetXY(velMin + RandomNum(0.0F, velRange), 0.0F);
					emitVel.RadRotate(m_EmitAngle.GetRadAngle() + spread * RandomNormalNum());
                    emitVel = RotateOffset(emitVel);
                    pParticle->SetVel(parentVel + emitVel);

                    if (pParticle->GetLifetime() != 0)
                        pParticle->SetLifetime(pParticle->GetLifetime() * (1.0F + ((*eItr)->GetLifeVariation() * RandomNormalNum())));
                    pParticle->SetTeam(m_Team);
                    pParticle->SetIgnoresTeamHits(true);

                    // Add to accumulative recoil impulse generated, F = m * a
                    // If enabled, that is
                    if ((*eItr)->PushesEmitter())
                        pushImpulses -= emitVel * pParticle->GetMass();

                    // Set the emitted particle to not hit this emitter's parent, if applicable
                    if (m_EmissionsIgnoreThis)
                        pParticle->SetWhichMOToNotHit(pRootParent);

                    // Scale the particle's lifetime based on the throttle
                    if (throttleFactor != 0)
                        pParticle->SetLifetime(pParticle->GetLifetime() * throttleFactor);

                    // Let particle loose into the world!
//                    g_MovableMan.AddParticle(pParticle);
                    // Might be an Actor...
                    g_MovableMan.AddMO(pParticle);
                    pParticle = 0;
                }
            }
        }
        m_LastEmitTmr.Reset();

        // Apply recoil/push effects. Joint stiffness will take effect when these are transferred to the parent.
        if (!pushImpulses.IsZero()) { AddImpulseForce(pushImpulses); }

        // Count the the damage caused by the emissions, and only if we're not bursting
        if (!m_BurstTriggered)
            m_DamageCount += (float)emissions * m_EmitDamage * m_EmitterDamageMultiplier;
        // Count the the damage caused by the burst
        else
            m_DamageCount += m_BurstDamage * m_EmitterDamageMultiplier;

        // Count the total emissions since enabling, and stop emitting if beyong limit (and limit is also enabled)
        m_EmitCount += emissions;
        if (m_EmitCountLimit > 0 && m_EmitCount > m_EmitCountLimit)
            EnableEmission(false);

        if (m_BurstTriggered)
            m_BurstTriggered = false;

        m_WasEmitting = true;
    }
    // Do stuff to stop emission
	else
	{
		if (m_WasEmitting)
		{
			if (m_EmissionSound) { m_EmissionSound->Stop(); }
			if (m_BurstSound) { m_BurstSound->Stop(); }
			if (m_EndSound) { m_EndSound->Play(m_Pos); }
			m_WasEmitting = false;
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

    // Set the screen flash effect to draw at the final post processing stage
    if (m_EmitEnabled && (!m_FlashOnlyOnBurst || m_BurstTriggered) && m_pFlash && m_pFlash->GetScreenEffect() && mode == g_DrawColor && !onlyPhysical)
    {
        // Fudge the emission pos forward a little bit so the glow aligns nicely
        Vector emitPos(m_pFlash->GetScreenEffect()->w / 4, 0);
        emitPos.RadRotate(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle());
        emitPos = m_Pos + RotateOffset(m_EmissionOffset) + emitPos;
        if(!g_SceneMan.ObscuredPoint(emitPos))
            g_PostProcessMan.RegisterPostEffect(emitPos, m_pFlash->GetScreenEffect(), m_pFlash->GetScreenEffectHash(), 55.0F + RandomNum(0.0F, 200.0F), m_pFlash->GetEffectRotAngle());
//            g_SceneMan.RegisterPostEffect(emitPos, m_pFlash->GetScreenEffect(), 55 + (200 * RandomNum() * ((float)1 - ((float)m_AgeTimer.GetElapsedSimTimeMS() / (float)m_Lifetime))));
    }
}

} // namespace RTE