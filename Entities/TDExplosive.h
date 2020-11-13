#ifndef _RTETDEXPLOSIVE_
#define _RTETDEXPLOSIVE_

#include "ThrownDevice.h"

namespace RTE {

	/// <summary>
	/// A thrown device that explodes after its trigger delay is completed after its activation.
	/// </summary>
	class TDExplosive : public ThrownDevice {

	public:

		EntityAllocation(TDExplosive)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TDExplosive object in system memory. Create() should be called before using the object.
		/// </summary>
		TDExplosive() { Clear(); }

		/// <summary>
		/// Makes the TDExplosive object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a TDExplosive to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the TDExplosive to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const TDExplosive &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a TDExplosive object before deletion from system memory.
		/// </summary>
		~TDExplosive() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { ThrownDevice::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire TDExplosive, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); ThrownDevice::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// If true, then the frame will not be changed automatically during update
		/// </summary>
		/// <returns>Whether or not the TDExplosive's Frame will change automatically during update.</returns>
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// <summary>
		/// Sets whether this TDExplosive is animated manually.
		/// </summary>
		/// <param name="isAnimatedManually">Whether or not to animate manually.</param>
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates this MovableObject. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this' current graphical HUD overlay representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="whichScreen">Which player's screen this is being drawn to. May affect what HUD elements get drawn etc.</param>
		/// <param name="playerControlled">Whether or not this MovableObject is currently player controlled (not applicable for TDExplosive).</param>
		void DrawHUD(BITMAP *targetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		bool m_IsAnimatedManually; //!< If true m_Frame is not changed during an update hence the animation is done by external Lua code.

	private:

		/// <summary>
		/// Clears all the member variables of this TDExplosive, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TDExplosive(const TDExplosive &reference) = delete;
		TDExplosive & operator=(const TDExplosive &rhs) = delete;
	};
}

#endif