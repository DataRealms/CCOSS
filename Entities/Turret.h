#ifndef _RTETURRET_
#define _RTETURRET_

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// <summary>
	/// An Attachable Turret pod that can hold HeldDevices.
	/// </summary>
	class Turret : public Attachable {

	public:

		EntityAllocation(Turret);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Turret object in system memory. Create() should be called before using the object.
		/// </summary>
		Turret() { Clear(); }

		/// <summary>
		/// Creates a Turret to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Turret to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Turret &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Turret object before deletion from system memory.
		/// </summary>
		~Turret() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Turret object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire Turret, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Attachable::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Indicates whether a HeldDevice is mounted or not.
		/// </summary>
		/// <returns>Whether or not a HeldDevice is mounted on this Turret.</returns>
		bool HasMountedDevice() const { return !m_MountedDevices.empty(); }

		/// <summary>
		/// Gets the first mounted HeldDevice of this Turret, mostly here for Lua convenience.
		/// </summary>
		/// <returns>A pointer to mounted HeldDevice of this Turret. Ownership is NOT transferred!</returns>
		HeldDevice * GetFirstMountedDevice() const { return m_MountedDevices.at(0); }

		/// <summary>
		/// Sets the first mounted HeldDevice for this Turret, mostly here for Lua convenience. Ownership IS transferred!
		/// The current first mounted HeldDevice (if there is one) will be dropped and added to MovableMan.
		/// </summary>
		/// <param name="newMountedDevice">The new HeldDevice to use.</param>
		void SetFirstMountedDevice(HeldDevice *newMountedDevice);

		/// <summary>
		/// Gets the vector of mounted HeldDevices for this Turret.
		/// </summary>
		/// <returns>The vector of mounted HeldDevices for this Turret.</returns>
		const std::vector<HeldDevice *> & GetMountedDevices() const { return m_MountedDevices; }

		/// <summary>
		/// Adds a HeldDevice to be mounted on this Turret. Ownership IS transferred!
		/// Will not remove any other HeldDevices mounted on this Turret.
		/// </summary>
		/// <param name="newMountedDevice">The new HeldDevice to be mounted on this Turret.</param>
		void AddMountedDevice(HeldDevice *newMountedDevice);

		/// <summary>
		/// Gets the current rotational offset of the mounted HeldDevice from the rest of the Turret.
		/// </summary>
		/// <returns>The current rotational offset of the mounted HeldDevice from the rest of the Turret.</returns>
		float GetMountedDeviceRotationOffset() const { return m_MountedDeviceRotationOffset; }

		/// <summary>
		/// Sets the current rotational offset of the mounted HeldDevice from the rest of the Turret.
		/// </summary>
		/// <param name="newOffsetAngle">The new offset angle in radians, relative from the rest of the Turret.</param>
		void SetMountedDeviceRotationOffset(float newOffsetAngle) { m_MountedDeviceRotationOffset = newOffsetAngle; }
#pragma endregion

#pragma region Override Methods
		/// <summary>
		/// Updates this MovableObject. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this Turret's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="pTargetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">In which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this MovableObject, indicator arrows or hovering HUD text and so on.</param>
		void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:

		/// <summary>
		/// Sets this Attachable's parent MOSRotating, and also sets its Team based on its parent and, if the Attachable is set to collide, adds/removes Atoms to its new/old parent.
		/// Additionally, deactivates all MountedDevices.
		/// </summary>
		/// <param name="newParent">A pointer to the MOSRotating to set as the new parent. Ownership is NOT transferred!</param>
		void SetParent(MOSRotating *newParent) override;

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

	private:

		//TODO I think things would be cleaner if this (and all hardcoded attachable pointers) used weak_ptrs. It would solve some weird ownership stuff, particularly with this. However, for that to be possible, m_Attachables has to be shared_ptrs though.
		std::vector<HeldDevice *> m_MountedDevices; //!< Vector of pointers to the mounted HeldDevices of this Turret, if any. Owned here.
		float m_MountedDeviceRotationOffset; //!< The relative offset angle (in radians) of the mounted HeldDevice from this Turret's rotation.

		/// <summary>
		/// Removes the HeldDevice from this turret's vector of mounted devices if it's in there. This releases the unique_ptr for it, leaving the caller to take care of it.
		/// </summary>
		/// <param name="mountedDeviceToRemove">A pointer to the mounted device to remove.</param>
		void RemoveMountedDevice(const HeldDevice *mountedDeviceToRemove);

		/// <summary>
		/// Clears all the member variables of this Turret, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Turret(const Turret &reference) = delete;
		Turret & operator=(const Turret &rhs) = delete;
	};
}
#endif