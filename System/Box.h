#ifndef _RTEBOX_
#define _RTEBOX_

#include "Serializable.h"
#include "Vector.h"

namespace RTE {

	/// <summary>
	/// A useful 2D axis-aligned rectangle class.
	/// </summary>
	class Box : public Serializable {

	public:

		Vector m_Corner; //! Vector position of the upper left corner of this box.
		float m_Width; //! Width of this box.
		float m_Height; //! Height of this box.

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Box object.
		/// </summary>
		Box() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Box object from two points.
		/// </summary>
		/// <param name="corner1">Vector position of the upper left corner of this box.</param>
		/// <param name="corner2">Vector position of the lower right corner of this box.</param>
		Box(const Vector &corner1, const Vector &corner2) { Create(corner1, corner2); }

		/// <summary>
		/// Constructor method used to instantiate a Box object from four float values defining the initial corners of this Box.
		/// </summary>
		/// <param name="x1">X position of box upper left corner.</param>
		/// <param name="y1">Y position of box upper left corner.</param>
		/// <param name="x2">X position of box lower right corner.</param>
		/// <param name="y2">Y position of box lower right corner.</param>
		Box(float x1, float y1, float x2, float y2) { Create(x1, y1, x2, y2); }

		/// <summary>
		/// Constructor method used to instantiate a Box object from one point and two dimensions.
		/// They can be negative but it will affect the interpretation of which corner is defined. The Box will always return positive values for width and height.
		/// </summary>
		/// <param name="corner">Vector position of the upper left corner of this box.</param>
		/// <param name="width">Width of this box.</param>
		/// <param name="height">Height of this box.</param>
		Box(const Vector &corner, float width, float height) { Create(corner, width, height); }

		/// <summary>
		/// Copy constructor method used to instantiate a Box object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Box object which is passed in by reference.</param>
		Box(const Box &reference) { Create(reference); }

		/// <summary>
		/// Makes the Box object ready for use.
		/// </summary>
		/// <param name="corner1">Vector position of the upper left corner of this box.</param>
		/// <param name="corner2">Vector position of the lower right corner of this box.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const Vector &corner1, const Vector &corner2);

		/// <summary>
		/// Makes the Box object ready for use.
		/// </summary>
		/// <param name="x1">X position of box upper left corner.</param>
		/// <param name="y1">Y position of box upper left corner.</param>
		/// <param name="x2">X position of box lower right corner.</param>
		/// <param name="y2">Y position of box lower right corner.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(float x1, float y1, float x2, float y2);

		/// <summary>
		/// Makes the Box object ready for use.
		/// </summary>
		/// <param name="corner">Vector position of the upper left corner of this box.</param>
		/// <param name="width">Width of this box.</param>
		/// <param name="height">Height of this box.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const Vector &corner, float width, float height);

		/// <summary>
		/// Creates a Box to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Box to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const Box &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Box object before deletion.
		/// </summary>
		virtual ~Box() { ; }

		/// <summary>
		/// Resets the entire Serializable, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
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
		/// Saves the complete state of this Box to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the Box will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Makes the corner of this box represent the upper left corner, and both width and height will end up positive.
		/// </summary>
		void Unflip();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the class name of this Box.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this Box.</returns>
		virtual const std::string & GetClassName() const { return ClassName; }

		/// <summary>
		/// Shows if this box has 0 width OR 0 height, ie can't contain anything.
		/// </summary>
		/// <returns>Whether either width OR height are 0.</returns>
		bool IsEmpty() const { return (m_Width == 0 || m_Height == 0); };

		/// <summary>
		/// Gets the primary corner of this box.
		/// </summary>
		/// <returns>A Vector with the primary corner of this box.</returns>
		Vector GetCorner() const { return m_Corner; }

		/// <summary>
		/// Sets the primary corner of this box.
		/// </summary>
		/// <param name="newCorner">A Vector with the new primary corner.</param>
		void SetCorner(const Vector &newCorner) { m_Corner = newCorner; }

		/// <summary>
		/// Gets the center point of this Box' area, in absolute Scene coordinates.
		/// </summary>
		/// <returns>The center point.</returns>
		Vector GetCenter() const { return Vector(m_Corner.m_X + (m_Width / 2), m_Corner.m_Y + (m_Height / 2)); }

		/// <summary>
		/// Sets the primary corner of this box, by specifying where the center ought to be.
		/// </summary>
		/// <param name="newCenter">A Vector with the new center point.</param>
		void SetCenter(const Vector &newCenter) { m_Corner.SetXY(newCenter.m_X - (m_Width / 2), newCenter.m_Y - (m_Height / 2)); }

		/// <summary>
		/// Gets the width of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// </summary>
		/// <returns>A float value that represents the width value of this Box.</returns>
		float GetWidth() const { return m_Width; }

		/// <summary>
		/// Sets the width of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// </summary>
		/// <param name="width">A float value that represents the width value of this Box.</param>
		void SetWidth(float width) { m_Width = width; }

		/// <summary>
		/// Gets the height of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// </summary>
		/// <returns>A float value that represents the height value of this Box.</returns>
		float GetHeight() const { return m_Height; }

		/// <summary>
		/// Sets the height of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// </summary>
		/// <param name="height">A float value that represents the height value of this Box.</param>
		void SetHeight(float height) { m_Height = height; }

		/// <summary>
		/// Gets the width times the height.
		/// </summary>
		/// <returns>The width times the height.</returns>
		float GetArea() const { return m_Width * m_Height; }

		/// <summary>
		/// Gets a random point within this box.
		/// </summary>
		/// <returns>The random point within the box.</returns>
		Vector GetRandomPoint() const { return Vector(m_Corner.m_X + m_Width * PosRand(), m_Corner.m_Y + m_Height * PosRand()); }
#pragma endregion

#pragma region Detection
		/// <summary>
		/// Tells whether another box intersects this one.
		/// </summary>
		/// <param name="rhs">The other Box to check for intersection with.</param>
		/// <returns>Intersecting the other box or not.</returns>
		bool IntersectsBox(const Box &rhs);

		/// <summary>
		/// Tells whether a point is within the Box or not.
		/// </summary>
		/// <param name="point">The Vector describing the point to test for within box bounds.</param>
		/// <returns>Inside the box or not.</returns>
		bool WithinBox(const Vector &point) const;

		/// <summary>
		/// Returns a copy of a point constrained inside this box.
		/// </summary>
		/// <param name="point">The Vector describing the point to constrain inside the box.</param>
		/// <returns>The resulting point inside the box.</returns>
		Vector GetWithinBox(const Vector &point) const { return Vector(GetWithinBoxX(point.m_X), GetWithinBoxY(point.m_Y)); }

		/// <summary>
		/// Tells whether an X coordinate is within the Box's X-range or not.
		/// </summary>
		/// <param name="pointX">The coordinate describing the X value to test for within box bounds.</param>
		/// <returns>Inside the box or not in the X axis.</returns>
		bool WithinBoxX(float pointX) const;

		/// <summary>
		/// Returns an X value constrained inside the Box and returns it.
		/// </summary>
		/// <param name="pointX">The X value to constrain inside the Box.</param>
		/// <returns>The constrained value.</returns>
		float GetWithinBoxX(float pointX) const;

		/// <summary>
		/// Tells whether an Y coordinate is within the Box's Y-range or not.
		/// </summary>
		/// <param name="pointY">The coordinate describing the Y value to test for within box bounds.</param>
		/// <returns>Inside the box or not in the Y axis.</returns>
		bool WithinBoxY(float pointY) const;

		/// <summary>
		/// Returns an Y value constrained inside the Box and returns it.
		/// </summary>
		/// <param name="pointY">The Y value to constrain inside the Box.</param>
		/// <returns>The constrained value.</returns>
		float GetWithinBoxY(float pointY) const;
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// An assignment operator for setting one Box equal to another.
		/// </summary>
		/// <param name="rhs">A Box reference.</param>
		/// <returns>A reference to the changed Box.</returns>
		Box & operator=(const Box &rhs);

		/// <summary>
		/// An equality operator for testing if any two Boxes are equal.
		/// </summary>
		/// <param name="lhs">A Box reference as the left hand side operand.</param>
		/// <param name="rhs">A Box reference as the right hand side operand.</param>
		/// <returns>A boolean indicating whether the two operands are equal or not.</returns>
		friend bool operator==(const Box &lhs, const Box &rhs) { if (lhs.m_Corner == rhs.m_Corner && lhs.m_Width == rhs.m_Width && lhs.m_Height == rhs.m_Height) { return true; } else { return false; } }

		/// <summary>
		/// An inequality operator for testing if any two Boxes are unequal.
		/// </summary>
		/// <param name="lhs">A Box reference as the left hand side operand.</param>
		/// <param name="rhs">A Box reference as the right hand side operand.</param>
		/// <returns>A boolean indicating whether the two operands are unequal or not.</returns>
		friend bool operator!=(const Box &lhs, const Box &rhs) { if (lhs.m_Corner != rhs.m_Corner || lhs.m_Width != rhs.m_Width || lhs.m_Height != rhs.m_Height) { return true; } else { return false; } }
#pragma endregion

	protected:

		static const std::string ClassName; //! A string with the friendly-formatted type name of this.

	private:

		/// <summary>
		/// Clears all the member variables of this Box, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear() { m_Corner.Reset(); m_Width = m_Height = 0; }
	};
}
#endif