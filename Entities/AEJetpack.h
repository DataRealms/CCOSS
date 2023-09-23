#ifndef _RTEAEJETPACK_
#define _RTEAEJETPACK_

#include "AEmitter.h"

namespace RTE
{

    /// <summary>
    /// A jetpack MO, which can be used to generate thrust
    /// </summary>
    class AEJetpack : public AEmitter {
        friend struct EntityLuaBindings;

    public:

        // Concrete allocation and cloning definitions
        EntityAllocation(AEJetpack);
        SerializableOverrideMethods;
        ClassInfoGetters;

        enum JetpackType {
            Standard,   // Can be intermittently tapped to produce small amounts of thrust
            JumpPack    // Spends all of it's fuel until empty, and cannot fire again until recharged
        };

    #pragma region Creation
        /// <summary>
        /// Constructor method used to instantiate a AEJetpack object in system memory. Create() should be called before using the object.
        /// </summary>
        AEJetpack() { Clear(); }

        /// <summary>
        /// Makes the AEJetpack object ready for use.
        /// </summary>
        /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
        int Create() override;

        /// <summary>
        /// Creates a AEJetpack to be identical to another, by deep copy.
        /// </summary>
        /// <param name="reference">A reference to the AEJetpack to deep copy.</param>
        /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
        int Create(const AEJetpack &reference);
    #pragma endregion

    #pragma region Destruction
        /// <summary>
        /// Destructor method used to clean up a AEJetpack object before deletion from system memory.
        /// </summary>
        ~AEJetpack() override { Destroy(true); }

        /// <summary>
        /// Resets the entire AEJetpack, including its inherited members, to their default settings or values.
        /// </summary>
        void Reset() override { Clear(); AEmitter::Reset(); }
    #pragma endregion

		/// <summary>
		/// Updates this AEJetpack from our parent actor.
		/// </summary>
		void UpdateBurstState(Actor &parentActor);

        /// <summary>
        /// Returns whether or not this jetpack is fully fueled.
        /// </summary>
        /// <returns>Whether or not this jetpack is fully fueled.</returns>
        bool IsFullyFueled() const { return m_JetTimeLeft >= m_JetTimeTotal - (m_JetTimeTotal * std::numeric_limits<float>::epsilon()); }

        /// <summary>
        /// Returns whether or not this jetpack is out of fuel.
        /// </summary>
        /// <returns>Whether or not this jetpack is out of fuel.</returns>
        bool IsOutOfFuel() const { return  m_JetTimeLeft <= std::numeric_limits<float>::epsilon(); }

        /// <summary>
        /// Gets the amount of time this jetpack can fire when filled, in ms.
        /// </summary>
        /// <returns>The amount of time this jetpack can fire when it's at max.</returns>
        float GetJetTimeTotal() const { return m_JetTimeTotal; }

        /// <summary>
        /// Sets the amount of time this' jetpack can fire when filled, in ms.
        /// </summary>
        /// <param name="newValue">The amount of time this jetpack can fire when it's at max.</param>
        void SetJetTimeTotal(float newValue) { m_JetTimeTotal = newValue; }

        /// <summary>
        /// Gets the amount of time this jetpack can still fire until out, in ms.
        /// </summary>
        /// <returns>The amount of time this jetpack can still fire before running out.</returns>
        float GetJetTimeLeft() const { return m_JetTimeLeft; }

        /// <summary>
        /// Sets the amount of time this' jetpack can still fire until out, in ms.
        /// </summary>
        /// <param name="newValue">The amount of time this' jetpack can still fire before running out.</param>
        void SetJetTimeLeft(float newValue) { m_JetTimeLeft = newValue < m_JetTimeTotal ? newValue : m_JetTimeTotal; }

        /// <summary>
        /// Gets the ratio of jetpack time that is left.
        /// </summary>
        /// <returns>The ratio of jetpack time that is left.</returns>
        float GetJetTimeRatio() { return m_JetTimeLeft / m_JetTimeTotal; }

        /// <summary>
        /// Gets the rate at which this AHuman's jetpack is replenished during downtime.
        /// </summary>
        /// <returns>The rate at which the jetpack is replenished.</returns>
        float GetJetReplenishRate() const { return m_JetReplenishRate; }

        /// <summary>
        /// Sets the rate at which this AHuman's jetpack is replenished during downtime.
        /// </summary>
        /// <param name="newValue">The rate at which the jetpack is replenished.</param>
        void SetJetReplenishRate(float newValue) { m_JetReplenishRate = newValue; }

        /// <summary>
        /// Gets the scalar ratio at which this jetpack's thrust angle follows the aim angle of the user.
        /// </summary>
        /// <returns>The ratio at which this jetpack follows the aim angle of the user.</returns>
        float GetJetAngleRange() const { return m_JetAngleRange; }

        /// <summary>
        /// Sets the scalar ratio at which this jetpack's thrust angle follows the aim angle of the user.
        /// </summary>
        /// <param name="newValue">The ratio at which this jetpack follows the aim angle of the user.</param>
        void SetJetAngleRange(float newValue) { m_JetAngleRange = newValue; }

        /// <summary>
        /// Gets the type of this jetpack.
        /// </summary>
        /// <returns>The type of this jetpack.</returns>
        JetpackType GetJetpackType() const { return m_JetpackType; }

        /// <summary>
        /// Sets the type of this jetpack.
        /// </summary>
        /// <param name="newType">The new type of this jetpack.</param>
        void SetJetpackType(JetpackType newType) { m_JetpackType = newType; }

        /// <summary>
        /// Returns whether the angle of this jetpack can adjust while firing, or if it can only be aimed while off.
        /// </summary>
        /// <returns>Whether the angle of this jetpack can adjust while firing.</returns>
        bool GetCanAdjustAngleWhileFiring() const { return m_CanAdjustAngleWhileFiring; }

        /// <summary>
        /// Sets whether the angle of this can adjust while firing, or if it can only be aimed while off.
        /// </summary>
        /// <param name="newValue">The new value for whether the angle of this jetpack can adjust while firing.</param>
        void SetCanAdjustAngleWhileFiring(bool newValue) { m_CanAdjustAngleWhileFiring = newValue; }

        /// <summary>
        /// Returns whether this jetpack adjusts it's throttle to balance for extra weight.
        /// </summary>
        /// <returns>Whether this jetpack adjusts it's throttle to balance for extra weight.</returns>
        bool GetAdjustsThrottleForWeight() const { return m_AdjustsThrottleForWeight; }

        /// <summary>
        /// Sets whether this jetpack adjusts it's throttle to balance for extra weight.
        /// </summary>
        /// <param name="newValue">The new value for whether this jetpack adjusts it's throttle to balance for extra weight.</param>
        void SetAdjustsThrottleForWeight(bool newValue) { m_AdjustsThrottleForWeight = newValue; }

    protected:
        static Entity::ClassInfo m_sClass;

        JetpackType m_JetpackType; //!< The type of jetpack
        float m_JetTimeTotal; //!< The max total time, in ms, that the jetpack can be used without pause
        float m_JetTimeLeft; //!< How much time left the jetpack can go, in ms
        float m_JetThrustBonusMultiplier; //!< A multiplier bonus to our produced thrust, which doesn't cost extra fuel. Used for AI buffs.
        float m_JetReplenishRate; //!< A multiplier affecting how fast the jetpack fuel will replenish when not in use. 1 means that jet time replenishes at 2x speed in relation to depletion.
        float m_JetAngleRange; //!< Ratio at which the jetpack angle follows aim angle
        bool m_CanAdjustAngleWhileFiring; //!< Whether or not the angle of the thrust can change while firing, or if it can only be adjusted while the jetpack is off
        bool m_AdjustsThrottleForWeight; //!< Whether or not the jetpack throttle auto-adjusts for weight, at the cost of fuel usage.

    private:
        /// <summary>
        /// The logic to run when bursting.
        /// </summary>
        /// <param name="parentActor">The parent actor using this jetpack.</param>
        /// <param name="fuelUseMultiplier">The multiplier to fuel usage rate.</param>
        void Burst(Actor& parentActor, float fuelUseMultiplier);

        /// <summary>
        /// The logic to run when thrusting.
        /// </summary>
        /// <param name="parentActor">The parent actor using this jetpack.</param>
        /// <param name="fuelUseMultiplier">The multiplier to fuel usage rate.</param>
        void Thrust(Actor& parentActor, float fuelUseMultiplier);

        /// <summary>
        /// The logic to run when recharging.
        /// </summary>
        /// <param name="parentActor">The parent actor using this jetpack.</param>
        void Recharge(Actor& parentActor);

    	/// <summary>
		/// Clears all the member variables of this AEJetpack, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		AEJetpack(const AEJetpack &reference) = delete;
		AEJetpack & operator=(const AEJetpack &rhs) = delete;
    };
}

#endif