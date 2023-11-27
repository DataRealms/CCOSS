//////////////////////////////////////////////////////////////////////////////////////////
// File:            PEmitter.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the PEmitter class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "PEmitter.h"
#include "Atom.h"
#include "RTETools.h"
#include "PresetMan.h"
#include "Emission.h"

namespace RTE {

	ConcreteClassInfo(PEmitter, MOSParticle, 100);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the member variables of this PEmitter, effectively
		//                  resetting the members of this abstraction level only.

		void PEmitter::Clear()
	{
		m_EmissionList.clear();
		m_EmissionSound.Reset();
		m_BurstSound.Reset();
		m_EndSound.Reset();
		m_EmitEnabled = false;
		m_WasEmitting = false;
		m_EmitCount = 0;
		m_EmitCountLimit = 0;
		m_NegativeThrottleMultiplier = 1.0F;
		m_PositiveThrottleMultiplier = 1.0F;
		m_Throttle = 0;
		m_EmissionsIgnoreThis = false;
		m_BurstScale = 1.0;
		m_BurstTriggered = false;
		m_BurstSpacing = 0;
		// Set this to really long so an initial burst will be possible
		m_BurstTimer.SetElapsedSimTimeS(50000);
		m_BurstTimer.SetElapsedRealTimeS(50000);
		m_EmitAngle.Reset();
		m_EmissionOffset.Reset();
		m_LastEmitTmr.Reset();
		m_FlashScale = 1.0f;
		m_AvgBurstImpulse = -1.0f;
		m_AvgImpulse = -1.0f;
		m_FlashOnlyOnBurst = true;
		m_SustainBurstSound = false;
		m_BurstSoundFollowsEmitter = true;
		m_LoudnessOnEmit = 1.0f;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the Emission object ready for use.

	int PEmitter::Create()
	{
		if (MOSParticle::Create() < 0)
			return -1;

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a PEmitter to be identical to another, by deep copy.

	int PEmitter::Create(const PEmitter &reference)
	{
		MOSParticle::Create(reference);

		for (auto itr = reference.m_EmissionList.begin(); itr != reference.m_EmissionList.end(); ++itr) {
			m_EmissionList.push_back(*itr);
		}
		m_EmissionSound = reference.m_EmissionSound;
		m_BurstSound = reference.m_BurstSound;
		m_EndSound = reference.m_EndSound;
		m_EmitEnabled = reference.m_EmitEnabled;
		m_EmitCount = reference.m_EmitCount;
		m_EmitCountLimit = reference.m_EmitCountLimit;
		m_NegativeThrottleMultiplier = reference.m_NegativeThrottleMultiplier;
		m_PositiveThrottleMultiplier = reference.m_PositiveThrottleMultiplier;
		m_Throttle = reference.m_Throttle;
		m_EmissionsIgnoreThis = reference.m_EmissionsIgnoreThis;
		m_BurstScale = reference.m_BurstScale;
		m_BurstSpacing = reference.m_BurstSpacing;
		m_BurstTriggered = reference.m_BurstTriggered;
		m_EmitAngle = reference.m_EmitAngle;
		m_EmissionOffset = reference.m_EmissionOffset;
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

	int PEmitter::ReadProperty(const std::string_view &propName, Reader &reader)
	{
		StartPropertyList(return MOSParticle::ReadProperty(propName, reader));
		
		MatchProperty("AddEmission",
		{
			Emission emission;
			reader >> emission;
			m_EmissionList.push_back(emission);
		});
		MatchProperty("EmissionSound", { reader >> m_EmissionSound; });
		MatchProperty("BurstSound", { reader >> m_BurstSound; });
		MatchProperty("EndSound", { reader >> m_EndSound; });
		MatchProperty("EmissionEnabled", { reader >> m_EmitEnabled; });
		MatchProperty("EmissionCount", { reader >> m_EmitCount; });
		MatchProperty("EmissionCountLimit", { reader >> m_EmitCountLimit; });
		MatchProperty("ParticlesPerMinute",
		{
			float ppm;
			reader >> ppm;
			// Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility
			for (auto eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
				(*eItr).m_PPM = ppm / m_EmissionList.size();
		});
		MatchProperty("NegativeThrottleMultiplier", { reader >> m_NegativeThrottleMultiplier; });
		MatchProperty("PositiveThrottleMultiplier", { reader >> m_PositiveThrottleMultiplier; });
		MatchProperty("Throttle", { reader >> m_Throttle; });
		MatchProperty("EmissionsIgnoreThis", { reader >> m_EmissionsIgnoreThis; });
		MatchProperty("BurstSize",
		{
			int burstSize;
			reader >> burstSize;
			// Go through all emissions and set the rate so that it emulates the way it used to work, for mod backwards compatibility
			for (auto eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
				(*eItr).m_BurstSize = std::ceil((float)burstSize / (float)m_EmissionList.size());
		});
		MatchProperty("BurstScale", { reader >> m_BurstScale; });
		MatchProperty("BurstSpacing", { reader >> m_BurstSpacing; });
		MatchProperty("BurstTriggered", { reader >> m_BurstTriggered; });
		MatchProperty("EmissionAngle", { reader >> m_EmitAngle; });
		MatchProperty("EmissionOffset", { reader >> m_EmissionOffset; });
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
	// Description:     Saves the complete state of this PEmitter with a Writer for
	//                  later recreation with Create(Reader &reader);

	int PEmitter::Save(Writer &writer) const
	{
		MOSParticle::Save(writer);

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
		writer.NewProperty("BurstSpacing");
		writer << m_BurstSpacing;
		writer.NewProperty("BurstTriggered");
		writer << m_BurstTriggered;
		writer.NewProperty("EmissionAngle");
		writer << m_EmitAngle;
		writer.NewProperty("EmissionOffset");
		writer << m_EmissionOffset;
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
	// Description:     Destroys and resets (through Clear()) the PEmitter object.

	void PEmitter::Destroy(bool notInherited)
	{
		// Stop playback of sounds gracefully
		if (m_EmissionSound.IsBeingPlayed())
			m_EndSound.Play(m_Pos);
		else
			m_EndSound.Stop();

		m_EmissionSound.Stop();
		//    m_BurstSound.Stop();

		if (!notInherited)
			MOSParticle::Destroy();
		Clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          ResetEmissionTimers
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Reset the timers of all emissions so they will start/stop at the 
	//                  correct relative offsets from now.

	void PEmitter::ResetEmissionTimers()
	{
		m_LastEmitTmr.Reset();
		for (auto eItr = m_EmissionList.begin(); eItr != m_EmissionList.end(); ++eItr)
			(*eItr).ResetEmissionTimers();
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          EnableEmission
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets this PEmitter to start emitting at the set rate, or to stop.

	void PEmitter::EnableEmission(bool enable)
	{
		if (!m_EmitEnabled && enable)
		{
			m_LastEmitTmr.Reset();
			// Reset counter
			m_EmitCount = 0;
			// Reset animation
			//m_Frame = 0;
		}
		m_EmitEnabled = enable;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          EstimateImpulse
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Calculates the forces this emitter applies on any parent.

	float PEmitter::EstimateImpulse(bool burst)
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
				if (eItr->PushesEmitter())
				{
					double emissions = eItr->GetRate() * g_TimerMan.GetDeltaTimeSecs() / 60.0f;
					if (burst)
						emissions *= eItr->GetBurstSize();

					velMin = std::min(eItr->GetMinVelocity(), eItr->GetMaxVelocity());
					velMax = std::max(eItr->GetMinVelocity(), eItr->GetMaxVelocity());
					velRange = (velMax - velMin) * 0.5;
					spread = std::max(static_cast<float>(c_PI)-eItr->GetSpread(), .0f) / c_PI;     // A large spread will cause the forces to cancel eachother out

					// Add to accumulative recoil impulse generated, F = m * a.
					impulse += (velMin + velRange) * spread * eItr->m_pEmission->GetMass() * emissions;
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
		if (burst)
			return m_AvgBurstImpulse * throttleFactor;

		return m_AvgImpulse * throttleFactor;
	}


	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GibThis
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gibs this, effectively destroying it and creating multiple gibs or
	//                  pieces in its place.

	void PEmitter::GibThis(Vector impactImpulse, float internalBlast, MovableObject *pIgnoreMO)
	{


	Attachable::GibThis(impactImpulse, internalBlast, pIgnoreMO);
	}
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Update
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates this PEmitter. Supposed to be done every frame.

	void PEmitter::Update()
	{
		MOSParticle::Update();

		if (m_BurstSoundFollowsEmitter) {
			m_BurstSound.SetPosition(m_Pos);
		}

		if (m_EmitEnabled)
		{
			if (!m_WasEmitting)
			{
				// Start playing the sound
				m_EmissionSound.Play(m_Pos);

				// Reset the timers of all emissions so they will start/stop at the correct relative offsets from now
				for (Emission& emission : m_EmissionList) {
					emission.ResetEmissionTimers();
				}
			}
			// Update the distance attenuation
			else
				m_EmissionSound.SetPosition(m_Pos);

			// Get the parent root of this PEmitter
			// TODO: Potentially get this once outside instead, like in attach/detach")
			MovableObject *pRootParent = GetRootParent();

			// Scale the emission rate up or down according to the appropriate throttle multiplier.
			float throttleFactor = GetThrottleFactor();
			m_FlashScale = throttleFactor;
			// Check burst triggering against whether the spacing is fulfilled
			if (m_BurstTriggered && (m_BurstSpacing <= 0 || m_BurstTimer.IsPastSimMS(m_BurstSpacing)))
			{
				// Play burst sound
				m_BurstSound.Play(m_Pos);
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
			for (Emission &emission : m_EmissionList)
			{
				// Make sure the emissions only happen between the start time and end time
				if (emission.IsEmissionTime())
				{
					// Apply the throttle factor to the emission rate
					currentPPM = emission.GetRate() * throttleFactor;
					emissions = 0;

					// Only do all this if the PPM is acutally above zero
					if (currentPPM > 0)
					{
						// Calculate secs per emission
						SPE = 60.0 / currentPPM;

						// Add the last elapsed time to the accumulator
						emission.m_Accumulator += m_LastEmitTmr.GetElapsedSimTimeS();

						// Now figure how many full emissions can fit in the current accumulator
						emissions = std::floor(emission.m_Accumulator / SPE);
						// Deduct the about to be emitted emissions from the accumulator
						emission.m_Accumulator -= emissions * SPE;

						RTEAssert(emission.m_Accumulator >= 0, "Emission accumulator negative!");
					}

					// Add extra emissions if bursting.
					if (m_BurstTriggered)
						emissions += emission.GetBurstSize() * std::floor(throttleFactor);

					pParticle = 0;
					emitVel.Reset();
					parentVel = pRootParent->GetVel() * emission.InheritsVelocity();

					for (int i = 0; i < emissions; ++i)
					{
						velMin = emission.GetMinVelocity() * (m_BurstTriggered ? m_BurstScale : 1.0);
						velRange = emission.GetMaxVelocity() - emission.GetMinVelocity() * (m_BurstTriggered ? m_BurstScale : 1.0);
						spread = emission.GetSpread() * (m_BurstTriggered ? m_BurstScale : 1.0);
						// Make a copy after the reference particle
						pParticle = dynamic_cast<MovableObject *>(emission.GetEmissionParticlePreset()->Clone());
						// Set up its position and velocity according to the parameters of this.
						// Emission point offset not set
						if (m_EmissionOffset.IsZero())
							pParticle->SetPos(m_Pos/*Vector(m_Pos.m_X + 5 * NormalRand(), m_Pos.m_Y + 5 * NormalRand())*/);
						else
							pParticle->SetPos(m_Pos + RotateOffset(m_EmissionOffset));
						// TODO: Optimize making the random angles!")
						emitVel.SetXY(velMin + RandomNum(0.0F, velRange), 0);
						emitVel.RadRotate(m_EmitAngle.GetRadAngle() + spread * RandomNormalNum());
						emitVel = RotateOffset(emitVel);
						pParticle->SetVel(parentVel + emitVel);

						if (pParticle->GetLifetime() != 0) { pParticle->SetLifetime(std::max(static_cast<int>(pParticle->GetLifetime() * (1.0F + (emission.GetLifeVariation() * RandomNormalNum()))), 1)); }
						pParticle->SetTeam(m_Team);
						pParticle->SetIgnoresTeamHits(true);

						// Add to accumulative recoil impulse generated, F = m * a
						// If enabled, that is
						if (emission.PushesEmitter())
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

			// Count the total emissions since enabling, and stop emitting if beyong limit (and limit is also enabled)
			m_EmitCount += emissions;
			if (m_EmitCountLimit > 0 && m_EmitCount > m_EmitCountLimit)
				EnableEmission(false);

			if (m_BurstTriggered)
				m_BurstTriggered = false;

			m_WasEmitting = true;
		}
		// Do stuff to stop emission
		else if (m_WasEmitting)
		{
			m_EmissionSound.Stop();
			if (!m_SustainBurstSound) { m_BurstSound.Stop(); }
			m_EndSound.Play(m_Pos);
			m_WasEmitting = false;
		}
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Draw
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws this PEmitter's current graphical representation to a
	//                  BITMAP of choice.

	void PEmitter::Draw(BITMAP *pTargetBitmap,
		const Vector &targetPos,
		DrawMode mode,
		bool onlyPhysical) const
	{
		MOSParticle::Draw(pTargetBitmap, targetPos, mode, onlyPhysical);
	}

} // namespace RTE