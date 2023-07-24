#include "AEJetpack.h"

#include "Actor.h"
#include "Controller.h"

namespace RTE {

    ConcreteClassInfo(AEJetpack, AEmitter, 20);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AEJetpack::Clear() {
		m_JetpackType = JetpackType::Standard;
		m_JetTimeTotal = 0.0F;
        m_JetTimeLeft = 0.0F;
		m_JetThrustBonusMultiplier = 1.0F;
        m_JetReplenishRate = 1.0F;
        m_JetAngleRange = 0.25F;
		m_CanAdjustAngleWhileFiring = true;
		m_AdjustsThrottleForWeight = true;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int AEJetpack::Create() {
        if (Attachable::Create() < 0) {
		    return -1;
	    }

        // Initalize the jump time left
        m_JetTimeLeft = m_JetTimeTotal;
		EnableEmission(false);

		return 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int AEJetpack::Create(const AEJetpack &reference)
    {
        AEmitter::Create(reference);

		m_JetpackType = reference.m_JetpackType;
        m_JetTimeTotal = reference.m_JetTimeTotal;
        m_JetTimeLeft = reference.m_JetTimeLeft;
        m_JetReplenishRate = reference.m_JetReplenishRate;
        m_JetAngleRange = reference.m_JetAngleRange;
		m_CanAdjustAngleWhileFiring = reference.m_CanAdjustAngleWhileFiring;
		m_AdjustsThrottleForWeight = reference.m_AdjustsThrottleForWeight;

		return 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AEJetpack::ReadProperty(const std::string_view& propName, Reader& reader) {
		if (propName == "JetpackType") {
			std::string jetpackType;
			reader >> jetpackType;
			if (jetpackType == "Standard") {
				m_JetpackType = JetpackType::Standard;
			} else if (jetpackType == "JumpPack") {
				m_JetpackType = JetpackType::JumpPack;
			} else {
				reader.ReportError("Unknown JetpackType '" + jetpackType + "'!");
			}
		} else if (propName == "JumpTime" || propName == "JetTime") {
            reader >> m_JetTimeTotal;
            m_JetTimeTotal *= 1000.0f; // Convert to ms
	    } else if (propName == "JumpReplenishRate" || propName == "JetReplenishRate") {
            reader >> m_JetReplenishRate;
        } else if (propName == "JumpAngleRange" || propName == "JetAngleRange") {
            reader >> m_JetAngleRange; 
        } else if (propName == "CanAdjustAngleWhileFiring") {
            reader >> m_CanAdjustAngleWhileFiring; 
		} else if (propName == "AdjustsThrottleForWeight") {
            reader >> m_AdjustsThrottleForWeight; 
        } else {
			return AEmitter::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AEJetpack::Save(Writer& writer) const {
		AEmitter::Save(writer);

		writer.NewProperty("JetpackType");
		switch (m_JetpackType) {
		default:
		case JetpackType::Standard:
			writer << "Standard";
			break;
		case JetpackType::JumpPack:
			writer << "JumpPack";
			break;
		}

		writer.NewPropertyWithValue("JumpTime", m_JetTimeTotal / 1000.0f); // Convert to seconds
		writer.NewPropertyWithValue("JumpReplenishRate", m_JetReplenishRate);
		writer.NewPropertyWithValue("JumpAngleRange", m_JetAngleRange);
		writer.NewPropertyWithValue("CanAdjustAngleWhileFiring", m_CanAdjustAngleWhileFiring);
		writer.NewPropertyWithValue("AdjustsThrottleForWeight", m_AdjustsThrottleForWeight);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void AEJetpack::UpdateBurstState(Actor &parentActor) {
        const Controller &controller = *parentActor.GetController();

		if (parentActor.GetStatus() == Actor::INACTIVE) {
			Recharge(parentActor);
			return;
		}

		float fuelUseMultiplier = 1.0F;
		if (m_AdjustsThrottleForWeight) {
			// Adjust force based on weight, so we have greater thrust with a heavier inventory (but spend fuel faster)
			float thrustAdjustment = parentActor.GetMass() / parentActor.GetBaseMass();
			float adjustedThrottle = GetScaledThrottle(0.0F, thrustAdjustment);
			SetThrottle(adjustedThrottle);
			fuelUseMultiplier = GetThrottleFactor();
		} else {
			// Jetpack throttle depletes relative to jet time, but only if throttle range values have been defined
			float jetTimeRatio = GetJetTimeRatio();
			SetThrottle((jetTimeRatio * 2.0F) - 1.0F);
		}

		bool wasEmittingLastFrame = IsEmitting();

		// We don't want the jetpack to rapidly toggle on/off at zero fuel if the player is holding down the jetpack button
		// So enforce a minimum time we must be able to thrust for (in ms)
		const float minimumTimeToBeginThrusting = 250.0f * fuelUseMultiplier;
		if (m_JetTimeLeft > minimumTimeToBeginThrusting || wasEmittingLastFrame || IsFullyFueled()) {
			switch (m_JetpackType)
			{
			case JetpackType::Standard:
				if (controller.IsState(BODY_JUMPSTART) && !IsOutOfFuel() && IsFullyFueled()) {
					Burst(parentActor, fuelUseMultiplier);
				} else if (controller.IsState(BODY_JUMP) && !IsOutOfFuel()) {
					Thrust(parentActor, fuelUseMultiplier);
				} else {
					Recharge(parentActor);
				}
				break;

			case JetpackType::JumpPack:
				if (wasEmittingLastFrame && !IsOutOfFuel()) {
					Thrust(parentActor, fuelUseMultiplier);
				} else if (controller.IsState(BODY_JUMPSTART) && IsFullyFueled()) {
					Burst(parentActor, fuelUseMultiplier);
				} else {
					Recharge(parentActor);
				}
				break;
			}
		} else {
			Recharge(parentActor);
		}

		float maxAngle = c_HalfPI * m_JetAngleRange;

		// If pie menu is on, keep the angle to what it was before.
		bool canAdjustAngle = !controller.IsState(PIE_MENU_ACTIVE) && (m_CanAdjustAngleWhileFiring || !IsEmitting());
		if (canAdjustAngle) {
			// Direct the jetpack nozzle according to either analog stick input or aim angle.
            const float analogDeadzone = 0.1F;
			if (controller.GetAnalogMove().MagnitudeIsGreaterThan(analogDeadzone)) {
				float jetAngle = std::clamp(controller.GetAnalogMove().GetAbsRadAngle() - c_HalfPI, -maxAngle, maxAngle);
				SetEmitAngle(parentActor.FacingAngle(jetAngle - c_HalfPI));
			} else {
				// Thrust in the opposite direction when strafing.
				float flip = ((parentActor.IsHFlipped() && controller.IsState(MOVE_RIGHT)) || (!parentActor.IsHFlipped() && controller.IsState(MOVE_LEFT))) ? -1.0F : 1.0F;
				// Halve the jet angle when looking downwards so the actor isn't forced to go sideways
                // TODO: don't hardcode this ratio?
                float aimAngle = parentActor.GetAimAngle(false);
				float jetAngle = (aimAngle > 0 ? aimAngle * m_JetAngleRange : -aimAngle * m_JetAngleRange * 0.5F) - maxAngle;
				// FacingAngle isn't needed because it's already been applied to AimAngle since last update.
				SetEmitAngle(jetAngle * flip - c_HalfPI);
			}
		}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AEJetpack::Burst(Actor& parentActor, float fuelUseMultiplier) {
		parentActor.SetMovementState(Actor::JUMP);
		
		TriggerBurst();
		EnableEmission(true);
		AlarmOnEmit(m_Team); // Jetpacks are noisy!

		float fuelUsage = g_TimerMan.GetDeltaTimeMS() * static_cast<float>(std::max(GetTotalBurstSize(), 2)) * (CanTriggerBurst() ? 1.0F : 0.5F) * fuelUseMultiplier; // burst fuel
		fuelUsage += g_TimerMan.GetDeltaTimeMS() * fuelUseMultiplier; // emit fuel
		m_JetTimeLeft -= fuelUsage;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AEJetpack::Thrust(Actor& parentActor, float fuelUseMultiplier) {
		parentActor.SetMovementState(Actor::JUMP);

		EnableEmission(true);
		AlarmOnEmit(m_Team); // Jetpacks are noisy!

		float fuelUsage = g_TimerMan.GetDeltaTimeMS() * fuelUseMultiplier;
		m_JetTimeLeft -= fuelUsage;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AEJetpack::Recharge(Actor& parentActor) {
		EnableEmission(false);
		if (parentActor.GetMovementState() == Actor::JUMP) {
			parentActor.SetMovementState(Actor::STAND);
		}
		m_JetTimeLeft = std::min(m_JetTimeLeft + g_TimerMan.GetDeltaTimeMS() * m_JetReplenishRate, m_JetTimeTotal);
	}

}