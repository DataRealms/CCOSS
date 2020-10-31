#ifndef _RTETHROWNDEVICE_
#define _RTETHROWNDEVICE_

#include "HeldDevice.h"

namespace RTE {

	/// <summary>
	/// A device that is carried and thrown by Actors.
	/// </summary>
	class ThrownDevice : public HeldDevice {

	public:

		EntityAllocation(ThrownDevice)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ThrownDevice object in system memory. Create should be called before using the object.
		/// </summary>
		ThrownDevice() { Clear(); }

		/// <summary>
		/// Makes the ThrownDevice object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a ThrownDevice to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the ThrownDevice to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const ThrownDevice &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a ThrownDevice object before deletion from system memory.
		/// </summary>
		~ThrownDevice() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { HeldDevice::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire ThrownDevice, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Attachable::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the Start throw offset of this ThrownDevice's joint relative from the parent Actor's position, if attached.
		/// </summary>
		/// <returns>A const reference to the current start throw parent offset.</returns>
		Vector GetStartThrowOffset() const { return m_StartThrowOffset; }

		/// <summary>
		/// Gets the End throw offset of this ThrownDevice's joint relative from the parent Actor's position, if attached.
		/// </summary>
		/// <returns>A const reference to the current end throw parent offset.</returns>
		Vector GetEndThrowOffset() const { return m_EndThrowOffset; }

		/// <summary>
		/// Gets the minimum throw velocity of this when thrown.
		/// </summary>
		/// <returns>The minimum throw velocity of this, in m/s.</returns>
		float GetMinThrowVel() const { return m_MinThrowVel; }

		/// <summary>
		/// Sets the minimum throw velocity of this when thrown.
		/// </summary>
		/// <param name="minThrowVel">The minimum throw velocity of this, in m/s.</param>
		void SetMinThrowVel(float minThrowVel) { m_MinThrowVel = minThrowVel; }

		/// <summary>
		/// Gets the maximum throw velocity of this when thrown.
		/// </summary>
		/// <returns>The maximum throw velocity of this, in m/s.</returns>
		float GetMaxThrowVel() const { return m_MaxThrowVel; }

		/// <summary>
		/// Sets the maximum throw velocity of this when thrown.
		/// </summary>
		/// <param name="maxThrowVel">The maximum throw velocity of this, in m/s.</param>
		void SetMaxThrowVel(float maxThrowVel) { m_MaxThrowVel = maxThrowVel; }

		/// <summary>
		/// If true then the explosive will not activate until it's released.
		/// </summary>
		/// <returns>Whether this ThrownDevice is supposed to only activate when it's released.</returns>
		bool ActivatesWhenReleased() const { return m_ActivatesWhenReleased; }
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Gets the current position offset of this ThrownDevice's joint relative from the parent Actor's position, if attached.
		/// </summary>
		/// <returns>A const reference to the current stance parent offset.</returns>
		Vector GetStanceOffset() const override { return m_StanceOffset.GetXFlipped(m_HFlipped); }

		/// <summary>
		/// Resets all the timers used by this (e.g. emitters, etc). This is to prevent backed up emissions from coming out all at once while this has been held dormant in an inventory.
		/// </summary>
		void ResetAllTimers() override;

		/// <summary>
		/// Activates this Device as long as it's not set to activate when released or it has no parent.
		/// </summary>
		void Activate() override;

		/// <summary>
		/// Does the calculations necessary to detect whether this MO appears to have has settled in the world and is at rest or not. IsAtRest() retrieves the answer.
		/// </summary>
		void RestDetection() override { HeldDevice::RestDetection(); if (m_Activated) { m_RestTimer.Reset(); } }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		SoundContainer m_ActivationSound; //!< Activation sound.

		Vector m_StartThrowOffset; //!< The position offset at which a throw of this Device begins.
		Vector m_EndThrowOffset; //!< The position offset at which a throw of this Device ends.
		float m_MinThrowVel; //!< The minimum throw velocity this gets when thrown.
		float m_MaxThrowVel; //!< The maximum throw velocity this gets when thrown.
		long m_TriggerDelay; //!< Time in millisecs from the time of being thrown to triggering whatever it is that this ThrownDevice does.
		bool m_ActivatesWhenReleased; //!< Whether this activates when its throw is started, or waits until it is released from the arm that is throwing it.

	private:

		/// <summary>
		/// Clears all the member variables of this ThrownDevice, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ThrownDevice(const ThrownDevice &reference) = delete;
		ThrownDevice & operator=(const ThrownDevice &rhs) = delete;
	};
}
#endif