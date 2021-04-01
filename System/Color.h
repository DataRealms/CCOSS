#ifndef _RTECOLOR_
#define _RTECOLOR_

#include "Serializable.h"

namespace RTE {

	/// <summary>
	/// A class representing a RGB color value.
	/// </summary>
	class Color : public Serializable {

	public:

		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Color object.
		/// </summary>
		Color() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Color object from RGB values.
		/// </summary>
		/// <param name="R">Initial Red value of this color.</param>
		/// <param name="G">Initial Green value of this color.</param>
		/// <param name="B">Initial Blue value of this color.</param>
		Color(int R, int G, int B) { Clear(); Create(R, G, B); }

		/// <summary>
		/// Constructor method used to instantiate a Color object from an entry in the current color palette.
		/// </summary>
		/// <param name="index">Palette index entry to create this color from.</param>
		Color(int index) { Clear(); SetRGBWithIndex(index); }

		/// <summary>
		/// Copy constructor method used to instantiate a Color object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Color object which is passed in by reference.</param>
		Color(const Color &reference) { Clear(); Create(reference.m_R, reference.m_G, reference.m_B); }

		/// <summary>
		/// Makes the Color object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Makes the Color object ready for use.
		/// </summary>
		/// <param name="R">Initial Red value of this color.</param>
		/// <param name="G">Initial Green value of this color.</param>
		/// <param name="B">Initial Blue value of this color.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(int inputR, int inputG, int inputB);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Sets RGB of this Color to zero.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the entry in the current color palette that most closely matches this Color's RGB values.
		/// </summary>
		/// <returns>The color entry index number.</returns>
		int GetIndex() const { return m_Index; }

		/// <summary>
		/// Sets all three RGB values of this Color, using an index from the current color palette.
		/// </summary>
		/// <param name="index">The index of the palette entry that this Color object's RGB values should be set to.</param>
		void SetRGBWithIndex(int index);

		/// <summary>
		/// Gets the red value of this Color.
		/// </summary>
		/// <returns>An integer value that represents the R value of this Color. 0 - 255.</returns>
		int GetR() const { return m_R; }

		/// <summary>
		/// Sets the red value of this Color.
		/// </summary>
		/// <param name="newR">An integer value that the R value will be set to, between 0 and 255.</param>
		void SetR(int newR) { m_R = std::clamp(newR, 0, 255); m_Index = 0; }

		/// <summary>
		/// Gets the green value of this Color.
		/// </summary>
		/// <returns>An integer value that represents the green value of this Color. 0 - 255.</returns>
		int GetG() const { return m_G; }

		/// <summary>
		/// Sets the green value of this Color.
		/// </summary>
		/// <param name="newG">An integer value that the green value will be set to, between 0 and 255.</param>
		void SetG(int newG) { m_G = std::clamp(newG, 0, 255); m_Index = 0; }

		/// <summary>
		/// Gets the blue value of this Color.
		/// </summary>
		/// <returns>An integer value that represents the blue value of this Color. 0 - 255.</returns>
		int GetB() const { return m_B; }

		/// <summary>
		/// Sets the blue value of this Color.
		/// </summary>
		/// <param name="newB">An integer value that the blue value will be set to, between 0 and 255.</param>
		void SetB(int newB) { m_B = std::clamp(newB, 0, 255); m_Index = 0; }

		/// <summary>
		/// Sets all three RGB values of this Color.
		/// </summary>
		/// <param name="newR">Integer value that the Red value will be set to, between 0 and 255.</param>
		/// <param name="newG">Integer value that the Green value will be set to, between 0 and 255.</param>
		/// <param name="newB">Integer value that the Blue value will be set to, between 0 and 255.</param>
		void SetRGB(int newR, int newG, int newB) { SetR(newR); SetG(newG); SetB(newB); m_Index = 0; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Causes recalculation of the nearest index even though there might be one cached or not.
		/// </summary>
		/// <returns>The new color entry index number.</returns>
		int RecalculateIndex();
#pragma endregion

	protected:

		int m_R; //!< Red value of this color.
		int m_G; //!< Green value of this color.
		int m_B; //!< Blue value of this color.
		int m_Index; //!< The closest matching index in the current color palette. If 0, this needs to be recalculated and updated.

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// <summary>
		/// Clears all the member variables of this Color, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear() { m_R = m_G = m_B = m_Index = 0; }
	};
}
#endif