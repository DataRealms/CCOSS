#ifndef _RTEARM_
#define _RTEARM_

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// <summary>
	/// A detachable arm that can hold HeldDevices.
	/// </summary>
	class Arm : public Attachable {

	public:

		EntityAllocation(Arm);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an Arm object in system memory. Create() should be called before using the object.
		/// </summary>
		Arm() { Clear(); }

		/// <summary>
		/// Makes the Arm object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates an Arm to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Arm to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Arm &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an Arm object before deletion from system memory.
		/// </summary>
		~Arm() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Arm object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Attachable::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire Arm, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Attachable::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the max length of this Arm when fully extended, i.e. the farthest possible length from its joint position to the hand.
		/// </summary>
		/// <returns>The max length of this Arm.</returns>
		float GetMaxLength() const { return m_MaxLength; }

		/// <summary>
		/// Gets the move speed of this Arm, where 1.0 is instant and 0.0 is none.
		/// </summary>
		/// <returns>The move speed of this Arm.</returns>
		float GetMoveSpeed() const { return m_MoveSpeed; }

		/// <summary>
		/// Sets the move speed of this Arm, where 1.0 is instant and 0.0 is none.
		/// </summary>
		/// <returns>The new move speed of this Arm.</returns>
		void SetMoveSpeed(float newMoveSpeed) { m_MoveSpeed = newMoveSpeed; }

		/// <summary>
		/// Gets the default idle offset of this Arm's hand, i.e. the default offset from the joint position that this Arm will try to move to when not moving towards a position.
		/// </summary>
		/// <returns>The idle offset of this Arm's hand.</returns>
		Vector GetHandDefaultIdleOffset() const { return m_HandDefaultIdleOffset; }

		/// <summary>
		/// Sets the default idle offset of this Arm's hand, i.e. the default offset from the joint position that this Arm will try to move to when not moving towards a position.
		/// </summary>
		/// <param name="newDefaultIdleOffset">The new idle offset of this Arm's hand.</param>
		void SetHandDefaultIdleOffset(const Vector &newDefaultIdleOffset) { m_HandDefaultIdleOffset = newDefaultIdleOffset; }

		/// <summary>
		/// Gets the rotation that is being applied to this Arm's hand, if it's using an idle offset.
		/// </summary>
		/// <returns>The idle rotation of this Arm's hand.</returns>
		float GetHandIdleRotation() const { return m_HandIdleRotation; }

		/// <summary>
		/// Sets the rotation that is being applied to this Arm's hand, if it's using an idle offset. Note that this value is reset to 0 every update.
		/// </summary>
		/// <param name="newHandIdleRotation">The new idle rotation of this Arm's hand.</param>
		void SetHandIdleRotation(float newHandIdleRotation) { m_HandIdleRotation = newHandIdleRotation; }

		/// <summary>
		/// Gets the current offset of this Arm's hand, i.e. its distance from the joint position.
		/// </summary>
		/// <returns>This current offset of this Arm's hand.</returns>
		Vector GetHandCurrentOffset() const { return m_HandCurrentOffset; }

		/// <summary>
		/// Sets the current offset of this Arm's hand, i.e. its distance from the joint position. The value is capped to the max length of the Arm.
		/// </summary>
		/// <param name="newHandOffset">The new current offset of this Arm's hand.</param>
		//TODO maybe don't want this in favor of SetHandPos?
		void SetHandCurrentOffset(const Vector &newHandOffset) { m_HandCurrentOffset = newHandOffset; m_HandCurrentOffset.CapMagnitude(m_MaxLength); }

		/// <summary>
		/// Gets the current position of this Arm's hand in absolute Scene coordinates.
		/// </summary>
		/// <returns>The current position of this Arm's hand in absolute Scene coordinates.</returns>
		Vector GetHandCurrentPos() const { return m_JointPos + m_HandCurrentOffset; }

		/// <summary>
		/// Sets the current position of this Arm's hand to an absolute scene coordinate. If needed, the set position is modified so its distance from the joint position of the Arm is capped to the max length of the Arm.
		/// </summary>
		/// <param name="newHandPos">The new current position of this Arm's hand as absolute scene coordinate.</param>
		void SetHandCurrentPos(const Vector &newHandPos);

		/// <summary>
		/// Gets the the strength with which this Arm will grip its HeldDevice.
		/// </summary>
		/// <returns>The grip strength of this Arm.</returns>
		float GetGripStrength() const { return m_GripStrength; }

		/// <summary>
		/// Sets the strength with which this Arm will grip its HeldDevice.
		/// </summary>
		/// <param name="newGripStrength">The new grip strength for this Arm to use.</param>
		void SetGripStrength(float newGripStrength) { m_GripStrength = newGripStrength; }

		/// <summary>
		/// Gets the the strength with which this Arm will throw a ThrownDevice.
		/// </summary>
		/// <returns>The throw strength of this Arm.</returns>
		float GetThrowStrength() const { return m_ThrowStrength; }

		/// <summary>
		/// Sets the strength with which this Arm will throw a ThrownDevice.
		/// </summary>
		/// <param name="newThrowStrength">The new throw strength for this Arm to use.</param>
		void SetThrowStrength(float newThrowStrength) { m_ThrowStrength = newThrowStrength; }
#pragma endregion

#pragma region Hand Animation Handling
		/// <summary>
		/// Adds a HandTarget position, in absolute scene coordinates, to the queue for the Arm to move its hand towards. Target positions are removed from the queue when they're reached (or as close to reached as is possible).
		/// </summary>
		/// <param name="description">The description of this HandTarget, for easy identification.</param>
		/// <param name="handTargetPositionToAdd">The position, in absolute scene coordinates, to add the queue of hand targets.</param>
		void AddHandTarget(const std::string &description, const Vector &handTargetPositionToAdd) { AddHandTarget(description, handTargetPositionToAdd, 0); }

		/// <summary>
		/// Adds a HandTarget position, in absolute scene coordinates, to the queue for the Arm to move its hand towards. Target positions are removed from the queue when they're reached (or as close to reached as is possible).
		/// If the target position is very close to the last element in the queue, or has the same name as it, the last element in the queue is updated to avoid filling the queue with similar values.
		/// </summary>
		/// <param name="description">The description of this HandTarget, for easy identification.</param>
		/// <param name="handTargetPositionToAdd">The position, in absolute scene coordinates, to add the queue of hand targets.</param>
		/// <param name="delayAtTarget">The amount of time, in MS, that the hand should wait when it reaches the newly added HandTarget.</param>
		void AddHandTarget(const std::string &description, const Vector &handTargetPositionToAdd, float delayAtTarget);

		/// <summary>
		/// Removes this Arm's next HandTarget, if there is one.
		/// </summary>
		void RemoveNextHandTarget() { if (!m_HandTargets.empty()) { m_HandTargets.pop(); m_HandHasReachedCurrentTarget = false; } }

		/// <summary>
		/// Gets whether or not this Arm has any HandTargets.
		/// </summary>
		/// <returns>Whether or not this Arm has any HandTargets.</returns>
		bool HasAnyHandTargets() const { return !m_HandTargets.empty(); }

		/// <summary>
		/// Gets the number of HandTargets this Arm has.
		/// </summary>
		/// <returns>The number of HandTargets this Arm has.</returns>
		int GetNumberOfHandTargets() const { return m_HandTargets.size(); }

		/// <summary>
		/// Gets the name of this Arm's next HandTarget.
		/// </summary>
		/// <returns>The name of this Arm's next HandTarget.</returns>
		std::string GetNextHandTargetDescription() const { return m_HandTargets.empty() ? "" : m_HandTargets.front().Description; }

		/// <summary>
		/// Gets the position, in absolute scene coordinates, of this Arm's next HandTarget.
		/// </summary>
		/// <returns>The position of this Arm's next HandTarget.</returns>
		Vector GetNextHandTargetPosition() const { return m_HandTargets.empty() ? Vector() : m_HandTargets.front().TargetOffset + m_JointPos; }

		/// <summary>
		/// Gets whether or not the hand has reached its current target. This is either the front of the HandTarget queue, or the offset it's currently trying to move to to when it has no HandTargets specified.
		/// </summary>
		/// <returns>Whether or not the hand has reached its current target.</returns>
		bool GetHandHasReachedCurrentTarget() const { return m_HandHasReachedCurrentTarget; }

		/// <summary>
		/// Empties the queue of HandTargets. With the queue empty, the hand will move to its appropriate idle offset.
		/// </summary>
		void ClearHandTargets() { m_HandTargets = {}; }
#pragma endregion

#pragma region HeldDevice Management
		/// <summary>
		/// Gets the HeldDevice currently held by this Arm.
		/// </summary>
		/// <returns>The HeldDevice currently held by this Arm. Ownership is NOT transferred.</returns>
		HeldDevice * GetHeldDevice() const { return m_HeldDevice; }

		/// <summary>
		/// Sets the HeldDevice held by this Arm.
		/// </summary>
		/// <param name="newHeldDevice">The new HeldDevice to be held by this Arm. Ownership IS transferred.</param>
		void SetHeldDevice(HeldDevice *newHeldDevice);

		/// <summary>
		/// Gets the HeldDevice this Arm is trying to support.
		/// </summary>
		/// <returns>The HeldDevice this Arm is trying to support. Ownership is NOT transferred.</returns>
		HeldDevice * GetHeldDeviceThisArmIsTryingToSupport() const { return m_HeldDeviceThisArmIsTryingToSupport; }

		/// <summary>
		/// Sets the HeldDevice being this Arm is trying to support.
		/// </summary>
		/// <param name="newHeldDeviceForThisArmToTryToSupport">The new HeldDevice this Arm should try to support. Ownership is NOT transferred.</param>
		void SetHeldDeviceThisArmIsTryingToSupport(HeldDevice *newHeldDeviceThisArmShouldTryToSupport) { m_HeldDeviceThisArmIsTryingToSupport = newHeldDeviceThisArmShouldTryToSupport; }

		/// <summary>
		/// Replaces the HeldDevice currently held by this Arm with a new one, and returns the old one. Ownership IS transferred both ways.
		/// </summary>
		/// <param name="newHeldDevice">The new HeldDevice to be held by this Arm. Ownership IS transferred.</param>
		/// <returns>The HeldDevice that was held by this Arm. Ownership IS transferred.</returns>
		HeldDevice * SwapHeldDevice(HeldDevice *newHeldDevice);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Draws this Arm's hand's graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">Which mode to draw in. See the DrawMode enumeration for available modes.</param>
		void DrawHand(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor) const;
#pragma endregion

#pragma region Override Methods
		/// <summary>
		/// Does stuff that needs to be done after Update().
		/// </summary>
		void PostTravel() override { if (IsAttached()) { m_AngularVel = 0; } MOSRotating::PostTravel(); }

		/// <summary>
		/// Updates this Arm. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this Arm's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">Which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this Arm. In this case, that means the hand sprite.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	private:

		/// <summary>
		/// Struct for storing data about each target in the Arm's queue of HandTargets.
		/// </summary>
		struct HandTarget {
			/// <summary>
			/// Constructor method used to instantiate a HandTarget object in system memory.
			/// </summary>
			HandTarget(const std::string_view &description, const Vector &targetOffset, float delayAtTarget, bool hFlippedWhenTargetWasCreated) : Description(description), TargetOffset(targetOffset), DelayAtTarget(delayAtTarget), HFlippedWhenTargetWasCreated(hFlippedWhenTargetWasCreated) {}

			std::string Description = "";
			Vector TargetOffset;
			float DelayAtTarget = 0;
			bool HFlippedWhenTargetWasCreated = false;
		};

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		float m_MaxLength; //!< The maximum length of this Arm when fully extended, i.e. the length of the straight Arm sprite.
		float m_MoveSpeed; //!< How quickly this Arm moves between targets. 0.0 means it doesn't move at all, 1.0 means it moves instantly.

		Vector m_HandDefaultIdleOffset; //!< The default offset that this Arm's hand should move to when not moving towards anything else, relative to its joint position. Other offsets are used under certain circumstances.
		float m_HandIdleRotation; //!< The rotation to be applied to the idle offset, when it's being used. Resets every update to avoid locking it.

		Vector m_HandCurrentOffset; //!< The current offset of this Arm's hand, relative to its joint position.

		std::queue<HandTarget> m_HandTargets; // A queue of target positions this Arm's hand is reaching towards. If it's empty, the Arm isn't reaching towards anything.
		Timer m_HandMovementDelayTimer; //!< A Timer for making the hand wait at its current HandTarget.
		bool m_HandHasReachedCurrentTarget; //!< A flag for whether or not the hand has reached its current target. The target is either the front of the HandTarget queue, or the appropriate target to move to if the queue is empty.

		ContentFile m_HandSpriteFile; //!< The ContentFile containing this Arm's hand bitmap.
		BITMAP *m_HandSpriteBitmap; //!< An unowned pointer to the Bitmap held by the hand sprite ContentFile.

		float m_GripStrength; //!< The strength with which this Arm will grip its HeldDevice. Effectively supersedes the HeldDevice's JointStrength.
		float m_ThrowStrength; //!< The strength with which this Arm will throw a ThrownDevice. Effectively supersedes the ThrownDevice's ThrowVelocity values.

		HeldDevice *m_HeldDevice; //!< A pointer to the HeldDevice this Arm is currently holding. Owned in the MOSRotating Attachables list, kept here for convenience.
		HeldDevice *m_HeldDeviceThisArmIsTryingToSupport; //!< A pointer to the HeldDevice being supported by this Arm (i.e. this is the background Arm for another HeldDevice).

		/// <summary>
		/// Gets whether or not the hand is close to the given offset.
		/// </summary>
		/// <param name="targetOffset">The offset to check for closeness to the hand.</param>
		/// <returns>Whether or not the hand is close to the given offset.</returns>
		bool HandIsCloseToTargetOffset(const Vector &targetOffset) const;

#pragma region Update Breakdown
		/// <summary>
		/// Updates the current hand offset for this Arm. Should only be called from Update.
		/// If the Arm is attached, the current hand offset is based on the target offset and move speed, and whether the Arm should idle or not, otherwise it puts it in a reasonable position.
		/// </summary>
		/// <param name="hasParent">Whether or not this Arm has a parent. Passed in for convenient reuse.</param>
		/// <param name="heldDeviceIsAThrownDevice">Whether or not this Arm's HeldDevice is a ThrownDevice. Passed in for convenient reuse.</param>
		void UpdateHandCurrentOffset(bool armHasParent, bool heldDeviceIsAThrownDevice);

		/// <summary>
		/// To be used in UpdateHandCurrentOffset. Applies any recoil accumulated on the HeldDevice to the passed in target offset, so it can be used to affect the hand's target position.
		/// </summary>
		/// <param name="heldDevice">The held MO as a HeldDevice, for convenience.</param>
		/// <param name="targetOffset">The target offset to have recoil applied to it.</param>
		void AccountForHeldDeviceRecoil(const HeldDevice *heldDevice, Vector &targetOffset);

		/// <summary>
		/// To be used in UpdateHandCurrentOffset. Ensures the HeldDevice won't clip through terrain by modifying the passed in target offset.
		/// </summary>
		/// <param name="heldDevice">The held MO as a HeldDevice, for convenience.</param>
		/// <param name="targetOffset">The target offset to be modified to avoid any terrain clipping.</param>
		void AccountForHeldDeviceTerrainClipping(const HeldDevice *heldDevice, Vector &targetOffset) const;

		/// <summary>
		/// Updates the frame for this Arm. Should only be called from Update.
		/// </summary>
		void UpdateArmFrame();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this Arm, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Arm(const Arm &reference) = delete;
		Arm & operator=(const Arm &rhs) = delete;
	};
}
#endif