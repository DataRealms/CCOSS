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

		EntityAllocation(Icon)
		ClassInfoGetters

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
		virtual int Create();

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
		virtual ~Icon() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Icon object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false);

		/// <summary>
		/// Resets the entire Icon, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); Entity::Reset(); }
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
		/// Saves the complete state of this Icon to an output stream for later recreation with Create(istream &stream).
		/// </summary>
		/// <param name="writer">A Writer that the Icon will save itself to.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the number of frames in this Icon's animation.
		/// </summary>
		/// <returns>The number of frames in the animation.</returns>
		unsigned short GetFrameCount() const { return m_FrameCount; }

		/// <summary>
		/// Gets the array of 8-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// </summary>
		/// <returns>The BITMAPs in 8bpp of this Icon.</returns>
		BITMAP ** GetBitmaps8() const { return m_BitmapsIndexed; }

		/// <summary>
		/// Gets the array of 32-bit bitmaps of this Icon, as many as GetFrameCount says. Neither the array nor the BITMAPs are transferred ownership!
		/// </summary>
		/// <returns>The BITMAPs in 32bpp of this Icon.</returns>
		BITMAP ** GetBitmaps32() const { return m_BitmapsTrueColor; }
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
		unsigned short m_FrameCount; //!< Number of frames in this Icon's animation.

		BITMAP **m_BitmapsIndexed; //!< The 8bpp BITMAPs of this Icon. The dynamic array IS owned here, but NOT the BITMAPs!
		BITMAP **m_BitmapsTrueColor; //!< The 32bpp BITMAPs of this Icon. The dynamic array IS owned here, but NOT the BITMAPs!

	private:

		/// <summary>
		/// Clears all the member variables of this Icon, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif