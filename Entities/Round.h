#ifndef _RTEROUND_
#define _RTEROUND_

#include "SoundContainer.h"

namespace RTE {

	class MovableObject;

	/// <summary>
	/// A round containing a number of projectile particles and one shell.
	/// </summary>
	class Round : public Entity {

	public:

		EntityAllocation(Round)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Round object in system memory. Create() should be called before using the object.
		/// </summary>
		Round() { Clear(); }

		/// <summary>
		/// Makes the Round object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a Round to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Round to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Round &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Round object before deletion from system memory.
		/// </summary>
		~Round() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Round object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire Round, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Returns how many particles are contained within this Round, not counting the shell.
		/// </summary>
		/// <returns>The number of particles.</returns>
		int ParticleCount() const { return m_ParticleCount; }

		/// <summary>
		/// Returns whether this Round is out of particles or not.
		/// </summary>
		/// <returns>Whether this Round is out of particles or not.</returns>
		bool IsEmpty() const { return m_ParticleCount <= 0; }

		/// <summary>
		/// Gets the next particle contained in this Round. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the next particle, or 0 if this Round is empty.</returns>
		const MovableObject * GetNextParticle() const { return (m_ParticleCount > 0) ? m_Particle : 0; }

		/// <summary>
		/// Gets the next particle contained in this Round, and removes it from the stack. Ownership IS transferred!
		/// </summary>
		/// <returns>A pointer to the next particle, or 0 if this Round is empty.</returns>
		MovableObject * PopNextParticle();

		/// <summary>
		/// Gets the velocity at which this round is to be fired.
		/// </summary>
		/// <returns>A float with the velocity in m/s.</returns>
		float GetFireVel() const { return m_FireVel; }

		/// <summary>
		/// Gets the separation of particles in this round.
		/// </summary>
		/// <returns>A float with the separation range in pixels.</returns>
		float GetSeparation() const { return m_Separation; }

		/// <summary>
		/// Gets the shell casing preset of this Round. Ownership IS NOT transferred!
		/// </summary>
		/// <returns>A pointer to the shell casing preset, or 0 if this Round has no shell.</returns>
		const MovableObject * GetShell() const { return m_Shell; }

		/// <summary>
		/// Gets the velocity at which this round's shell is to be ejected.
		/// </summary>
		/// <returns>A float with the shell velocity in m/s.</returns>
		float GetShellVel() const { return m_ShellVel; }

		/// <summary>
		/// Shows whether this Round has an extra sound sample to play when fired.
		/// </summary>
		/// <returns>Whether the firing Sound of this has been loaded, or the firing Device will make the noise alone.</returns>
		bool HasFireSound() const { return m_FireSound.HasAnySounds(); }

		/// <summary>
		/// Gets the extra firing sound of this Round, which can be played in addition to the weapon's own firing sound. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <returns>A sound with the firing sample of this round.</returns>
		SoundContainer * GetFireSound() { return &m_FireSound; }
#pragma endregion

#pragma region AI Properties
		/// <summary>
		/// Returns the lifetime of the projectile used by the AI when executing the shooting scripts.
		/// </summary>
		/// <returns>The life time in MS used by the AI.</returns>
		unsigned long GetAILifeTime() const { return m_AILifeTime; }

		/// <summary>
		/// Returns the FireVelocity of the projectile used by the AI when executing the shooting scripts.
		/// </summary>
		/// <returns>The FireVelocity in m/s used by the AI.</returns>
		int GetAIFireVel() const { return m_AIFireVel; }

		/// <summary>
		/// Returns the bullet's ability to penetrate material when executing the AI shooting scripts.
		/// </summary>
		/// <returns>A value equivalent to Mass * Sharpness * Vel.</returns>
		int GetAIPenetration() const { return m_AIPenetration; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		const MovableObject *m_Particle; //!< Round particle MovableObject preset instance.
		int m_ParticleCount; //!< How many particle copies there are in this Round.
		float m_FireVel; //!< The velocity with which this Round is fired.
		float m_Separation; //!< The range of separation between particles in this Round, in pixels.

		const MovableObject *m_Shell; //!< Shell particle MovableObject preset instance.
		float m_ShellVel; //!< The velocity with which this Round's shell/casing is launched.

		SoundContainer m_FireSound; //!< The extra firing audio of this Round being fired.

		unsigned long m_AILifeTime; //!< For overriding LifeTime when executing the AI shooting scripts.
		int m_AIFireVel; //!< For overriding FireVel when executing the AI shooting scripts.
		int m_AIPenetration; //!< For overriding the bullets ability to penetrate material when executing the AI shooting scripts.

	private:

		/// <summary>
		/// Clears all the member variables of this Round, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Round(const Round &reference) = delete;
		Round & operator=(const Round &rhs) = delete;
	};
}
#endif