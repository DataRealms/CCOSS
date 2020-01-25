#ifndef _RTETDEXPLOSIVE_
#define _RTETDEXPLOSIVE_

#include "ThrownDevice.h"

namespace RTE {
	/// <summary>
	/// [Concrete Class] -  A thrown device that explodes after its trigger delay is completed after its activation.
	/// </summary>
	class TDExplosive : public ThrownDevice {

	public:
		ENTITYALLOCATION(TDExplosive);
		CLASSINFOGETTERS();

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TDExplosive object in system memory. Create() should be called before using the object.
		/// </summary>
		TDExplosive() { Clear(); }

		/// <summary>
		/// Creates a TDExplosive to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the TDExplosive to deep copy.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const TDExplosive &reference);
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
		/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
		/// </summary>
		/// <param name="propName">The name of the property to be read.</param>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>
		/// An error return value signaling whether the property was successfully read or not.
		/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
		/// </returns>
		virtual int ReadProperty(std::string propName, Reader &reader);

		/// <summary>
		/// Saves the complete state of this TDExplosive to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the TDExplosive will save itself with.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a TDExplosive object before deletion from system memory.
		/// </summary>
		virtual ~TDExplosive() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Resets the entire TDExplosive, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); ThrownDevice::Reset(); }

		/// <summary>
		/// Updates this MovableObject. Supposed to be done every frame.
		/// </summary>
		virtual void Update();

		/// <summary>
		/// Draws this' current graphical HUD overlay representation to a BITMAP of choice.
		/// </summary>
		/// <param name="pTargetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="whichScreen">Which player's screen this is being drawn to. May affect what HUD elements get drawn etc.</param>
		/// <param name="playerControlled">Whether or not this MovableObject is currently player controlled (not applicable for TDExplosive)</param>
		virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// If true, then the frame will not be changed automatically during update
		/// </summary>
		/// <returns>Whether or not the TDExplosive's Frame will change automatically during update</returns>
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// <summary>
		/// Sets whether this TDExplosive is animated manually.
		/// </summary>
		/// <param name="isAnimatedManually">Whether or not to animate manually</param>
		void SetAnimatedManually(bool isAnimatedManually) { m_IsAnimatedManually = isAnimatedManually; }
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //! ClassInfo for this class

		bool m_IsAnimatedManually; //! If true m_Frame is not changed during an update hence the animation is done by external Lua code

	private:
		/// <summary>
		/// Clears all the member variables of this TDExplosive, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TDExplosive(const TDExplosive &reference);
		TDExplosive & operator=(const TDExplosive &rhs);
	};
}

#endif