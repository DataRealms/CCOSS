#ifndef _RTECOLOR_
#define _RTECOLOR_

#include "Serializable.h"

namespace RTE {

	enum { R = 0, G, B };

	/// <summary>
	/// A class representing a RGB color value.
	/// </summary>
	class Color : public Serializable {

	public:

		unsigned char m_R; //! Red value of this color.
		unsigned char m_G; //! Green value of this color.
		unsigned char m_B; //! Blue value of this color.
		unsigned char m_Index; //! The closest matching index in the current color palette. If 0, this needs to be recalculated and updated.

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Color object.
		/// </summary>
		Color() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Color object from RGB values
		/// </summary>
		/// <param name="R">Initial Red value of this color.</param>
		/// <param name="G">Initial Green value of this color.</param>
		/// <param name="B">Initial Blue value of this color.</param>
		Color(unsigned char R, unsigned char G, unsigned char B) { Clear(); Create(R, G, B); }

		/// <summary>
		/// Constructor method used to instantiate a Color object from an entry in the current color palette.
		/// </summary>
		/// <param name="index">Palette index entry to create this color from.</param>
		Color(unsigned char index) { Clear(); SetRGBWithIndex(index); }

		/// <summary>
		/// Copy constructor method used to instantiate a Color object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Color object which is passed in by reference.</param>
		Color(const Color &reference) { Clear(); Create(reference.m_R, reference.m_G, reference.m_B); }

		/// <summary>
		/// Makes the Color object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();

		/// <summary>
		/// Makes the Color object ready for use.
		/// </summary>
		/// <param name="R">Initial Red value of this color.</param>
		/// <param name="G">Initial Green value of this color.</param>
		/// <param name="B">Initial Blue value of this color.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(unsigned char inputR, unsigned char inputG, unsigned char inputB) {
			m_R = inputR;
			m_G = inputG;
			m_B = inputB;
			RecalculateIndex();
			return 0;
		}
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Color object before deletion.
		/// </summary>
		virtual ~Color() { ; }

		/// <summary>
		/// Sets RGB of this Color to zero.
		/// </summary>
		void Reset() { Clear(); }
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
		/// Saves the complete state of this Color to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the Color will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the entry in the current color palette that most closely matches this Color's RGB values.
		/// </summary>
		/// <returns>The color entry index number.</returns>
		unsigned char GetIndex() const { return m_Index; }

		/// <summary>
		/// Sets all three RGB values of this Color, using an index from the current color palette.
		/// </summary>
		/// <param name="index">The index of the palette entry that this Color object's RGB values should be set to.</param>
		void SetRGBWithIndex(unsigned char index);

		/// <summary>
		/// Gets the red value of this Color.
		/// </summary>
		/// <returns>An unsigned char value that represents the R value of this Color. 0 - 255.</returns>
		unsigned char GetR() const { return m_R; }

		/// <summary>
		/// Sets the red value of this Color.
		/// </summary>
		/// <param name="newR">An unsigned char value that the R value will be set to, between 0 and 255.</param>
		void SetR(unsigned char newR) { m_R = newR; m_Index = 0; }

		/// <summary>
		/// Gets the green value of this Color.
		/// </summary>
		/// <returns>An unsigned char value that represents the G value of this Color. 0 - 255.</returns>
		unsigned char GetG() const { return m_G; }

		/// <summary>
		/// Sets the green value of this Color.
		/// </summary>
		/// <param name="newG">An unsigned char value that the green value will be set to, between 0 and 255.</param>
		void SetG(unsigned char newG) { m_G = newG; m_Index = 0; }

		/// <summary>
		/// Gets the blue value of this Color.
		/// </summary>
		/// <returns>An unsigned char value that represents the B value of this Color. 0 - 255.</returns>
		unsigned char GetB() const { return m_B; }

		/// <summary>
		/// Sets the blue value of this Color.
		/// </summary>
		/// <param name="newB">An unsigned char value that the blue value will be set to, between 0 and 255.</param>
		void SetB(unsigned char newB) { m_B = newB; m_Index = 0; }

		/// <summary>
		/// Sets all three RGB values of this Color.
		/// </summary>
		/// <param name="newR">Unsigned char value that the Red value will be set to, between 0 and 255.</param>
		/// <param name="newG">Unsigned char value that the Green value will be set to, between 0 and 255.</param>
		/// <param name="newB">Unsigned char value that the Blue value will be set to, between 0 and 255.</param>
		void SetRGB(unsigned char newR, unsigned char newG, unsigned char newB) { m_R = newR; m_G = newG; m_B = newB; m_Index = 0; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Causes recalculation of the nearest index even though there might be one cached or not.
		/// </summary>
		/// <returns>The new color entry index number.</returns>
		unsigned char RecalculateIndex();
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Color.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this Color.</returns>
		virtual const std::string & GetClassName() const { return m_ClassName; }
#pragma endregion

	protected:

		static const std::string m_ClassName; //! A string with the friendly-formatted type name of this.

	private:

		/// <summary>
		/// Clears all the member variables of this Color, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear() { m_R = m_G = m_B = m_Index = 0; }
	};
}
#endif