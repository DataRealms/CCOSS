#ifndef _RTELEG_
#define _RTELEG_

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// <summary>
	/// A detachable Leg that will be controlled by LimbPaths.
	/// </summary>
	class Leg : public Attachable {

	public:
		EntityAllocation(Leg)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Leg object in system memory. Create() should be called before using the object.
		/// </summary>
		Leg() { Clear(); }

		/// <summary>
		/// Makes the Leg object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a Leg to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Leg to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Leg &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Leg object before deletion from system memory.
		/// </summary>
		~Leg() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Leg object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Attachable::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire Leg, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Attachable::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the foot of this Leg.
		/// </summary>
		/// <returns>A pointer to foot of this Leg. Ownership is NOT transferred!</returns>
		Attachable * GetFoot() const { return m_Foot; }

		/// <summary>
		/// Sets the foot for this Leg. Ownership IS transferred!
		/// </summary>
		/// <param name="newFoot">The new foot to use.</param>
		void SetFoot(Attachable *newFoot);

		/// <summary>
		/// Gets the min length this of Leg, the minimum allowed length from its joint to its ankle's position.
		/// </summary>
		/// <returns>The min length, in pixels, of this Leg.</returns>
		float GetMinLength() const { return m_MinExtension; }

		/// <summary>
		/// Gets the max length this Leg, the maximum allowed length from its joint to its ankle's position.
		/// </summary>
		/// <returns>The max length, in pixels, of this Leg.</returns>
		float GetMaxLength() const { return m_MaxExtension; }

		/// <summary>
		/// Sets the position this Leg should move towards, in absolute coordinates.
		/// </summary>
		/// <param name="targetPosition">The position the Leg should move towards.</param>
		void SetTargetPosition(const Vector &targetPosition) { m_TargetPosition = targetPosition; }

		/// <summary>
		/// Sets whether this Leg will go into idle offset mode if the target appears to be above the joint of the Leg.
		/// </summary>
		/// <param name="idle">Whether to enable idling if the target offset is above the joint.</param>
		void EnableIdle(bool idle = true) { m_WillIdle = idle; }
#pragma endregion

#pragma region Override Methods
		/// <summary>
		/// Updates this Leg. Supposed to be done every frame.
		/// </summary>
		void Update() override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Attachable *m_Foot; //!< Pointer to the foot attachable of this Leg.
		
		Vector m_ContractedOffset; //!< The offset from the joint where the ankle contracts to in the sprite.
		Vector m_ExtendedOffset; //!< The offset from the joint where the ankle extends to in the sprite.

		float m_MinExtension; //!< Precalculated min extension of the Leg (from the joint) based on the contracted offset.
		float m_MaxExtension; //!< Precalculated max extension of the Leg (from the joint) based on the extended offset.
		float m_NormalizedExtension; //!< Normalized scalar of where the ankle offset's magnitude is between the min and max extensions.
		
		Vector m_TargetPosition; //!< The absolute position that this Leg's foot is moving towards.
		Vector m_IdleOffset; //!< The target offset from m_Pos that this Leg's foot is moving towards when allowed to idle and the target position is not acceptable.

		Vector m_AnkleOffset; //!< Current offset from the joint to the ankle where the foot should be.
		
		bool m_WillIdle; //!< Whether the Leg will go to idle position if the target position is above the Leg's joint position.
		float m_MoveSpeed; //!< How fast the Leg moves to a reach target, 0 means it doesn't and 1 means it moves instantly.

	private:

#pragma region Update Breakdown
		/// <summary>
		/// Updates the current ankle offset for this Leg. Should only be called from Update.
		/// If the Leg is attached, the current ankle offset is based on the target offset and move speed, and whether the Leg should idle or not, otherwise it puts it in a reasonable position.
		/// </summary>
		void UpdateCurrentAnkleOffset();

		/// <summary>
		/// Updates the rotation of the Leg. Should only be called from Update.
		/// If the Leg is attached, this applies extra rotation to line up the Leg's sprite with its extension line, otherwise it does nothing.
		/// </summary>
		void UpdateLegRotation();

		/// <summary>
		/// Updates the frame and rotation of the Leg's foot Attachable. Should only be called from Update.
		/// If the Leg is attached, the foot's rotation and frame depend on the ankle offset, otherwise the foot's rotation is set to be perpendicular to the Leg's rotation.
		/// </summary>
		void UpdateFootFrameAndRotation();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this Leg, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Leg(const Leg &reference) = delete;
		Leg & operator=(const Leg &rhs) = delete;
	};
}
#endif