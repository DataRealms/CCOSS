#ifndef _RTELEG_
#define _RTELEG_

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// <summary>
	/// A detatchable leg that will be controlled by LimbPaths.
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
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a Leg to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Leg to deep copy.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
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
		Attachable *GetFoot() const { return m_Foot; }

		/// <summary>
		/// Sets the foot for this Leg. Ownership IS transferred!
		/// </summary>
		/// <param name="newFoot">The new foot to use.</param>
		void SetFoot(Attachable *newFoot);

		/// <summary>
		/// Gets the max length this Leg can reach from its socket to the foot.
		/// </summary>
		/// <returns>The max length of reach, in pixels, of this Leg.</returns>
		float GetMaxLength() const { return m_MaxExtension; }

		//TODO I think this is in absolute scene coordinates actually, since it works based off of joint pos not joint offset. Look into this and update accordingly. Also look at the member variable, the (0, 0) thing seems wrong.
		/// <summary>
		/// Sets the point the Leg should move towards, in relative coordinates.
		/// </summary>
		/// <param name="targetOffset">The offset the Leg should move towards.</param>
		void SetTargetOffset(const Vector &targetOffset) { m_TargetOffset = targetOffset; }

		/// <summary>
		/// Sets this to go into idle offset mode if the target appears to be above the joint of the Leg.
		/// </summary>
		/// <param name="idle">Whether to enable idling if the target offset is above the joint.</param>
		void EnableIdle(bool idle = true) { m_WillIdle = idle; }
#pragma endregion

		/// <summary>
		/// Updates this Leg. Supposed to be done every frame.
		/// </summary>
		void Update() override;

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Attachable *m_Foot; //!< Pointer to the foot attachable of this Leg.
		
		Vector m_ContractedOffset; //!< The offset from the joint where the ankle contracts to in the sprite.
		Vector m_ExtendedOffset; //!< The offset from the joint where the ankle extends to in the sprite.

		float m_MinExtension; //!< Precalculated min extension of the Leg (from the joint) based on the contracted offset.
		float m_MaxExtension; //!< Precalculated max extension of the Leg (from the joint) based on the extended offset.
		float m_CurrentNormalizedExtension; //!< Normalized scalar of where the ankle offset's magnitude is between the min and max extensions
		
		Vector m_TargetOffset; //!< The target offset that this Leg's foot is moving towards. If (0, 0), the Leg will not try  towards anything.
		Vector m_IdleOffset; //!< The target offset from m_Pos that this Leg's foot is reaching towards when not reaching for or anything else.

		Vector m_CurrentAnkleOffset; //!< Current offset from the joint to the ankle where the foot should be.
		
		bool m_WillIdle; //!< Whether the Leg will go to idle position if the target is above the joint.
		float m_MoveSpeed; //!< How fast the Leg moves to a reach target, 0 means it doesn't and 1 means it moves instantly.

	private:

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
		/// Sets the Leg's frame to the appropriate one based on its current extension, and updates the normalized extension value. Should only be called from Update.
		/// Works the same whether attached or not.
		/// </summary>
		void UpdateFrameAndNormalizedExtension();

		/// <summary>
		/// Updates the frame, joint position and rotation of the Leg's foot Attachable. Should only be called from Update.
		/// If the Leg is attached, the rotation and frame depend on the target offset, otherwise the rotation is set to be perpendicular to the Leg.
		/// The joint position is always updated to include the current ankle offset so the foot Attachable stays in the appropriate position.
		/// </summary>
		void UpdateFootFrameJointPositionAndRotation();

		/// <summary>
		/// Clears all the member variables of this Leg, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Leg(const Leg &reference) {}
		Leg &operator=(const Leg &rhs) {}
	};
}
#endif