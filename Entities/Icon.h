#ifndef _RTEICON_
#define _RTEICON_

#include "Entity.h"
#include "ContentFile.h"

namespace RTE {

	/// <summary>
	/// Represents an Icon in the interface that can be loaded and stored from different data modules etc.
	/// </summary>
	class Icon : public Entity {

	public:

		EntityAllocation(Icon);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an Icon object in system memory. Create() should be called before using the object.
		/// </summary>
		Icon() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate an Icon object identical to an already existing one.
		/// </summary>
		/// <param name="reference">An Icon object which is passed in by reference.</param>
		Icon(const Icon &reference) { if (this != &reference) { Clear(); Create(reference); } }

		/// <summary>
		/// Makes the Icon object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates an Icon to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Icon to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Icon &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an Icon object before deletion from system memory.
		/// </summary>
		~Icon() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Icon object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire Icon, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the number of frames in this Icon's animation.
		/// </summary>
		/// <returns>The number of frames in the animation.</returns>
		unsigned int GetFrameCount() const { return m_FrameCount; }

		/// <summary>
		/// Gets the array of 8-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// </summary>
		/// <returns>The BITMAPs in 8bpp of this Icon.</returns>
		std::vector<BITMAP *> GetBitmaps8() const { return m_BitmapsIndexed; }

		/// <summary>
		/// Gets the array of 32-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// </summary>
		/// <returns>The BITMAPs in 32bpp of this Icon.</returns>
		std::vector<BITMAP *> GetBitmaps32() const { return m_BitmapsTrueColor; }
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// An assignment operator for setting one Icon equal to another.
		/// </summary>
		/// <param name="rhs">An Icon reference.</param>
		/// <returns>A reference to the changed Icon.</returns>
		Icon & operator=(const Icon &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ContentFile m_BitmapFile; //!< ContentFile containing the bitmap file of this Icon.
		unsigned int m_FrameCount; //!< Number of frames in this Icon's animation.

		std::vector<BITMAP *> m_BitmapsIndexed; //!< Vector containing the 8bpp BITMAPs of this Icon. BITMAPs are NOT owned!
		std::vector<BITMAP *> m_BitmapsTrueColor; //!< Vector containing the 32bpp BITMAPs of this Icon. BITMAPs are NOT owned!

	private:

		/// <summary>
		/// Clears all the member variables of this Icon, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif