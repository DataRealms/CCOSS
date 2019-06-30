#ifndef _RTEBOX_
#define _RTEBOX_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Box.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Box class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <iostream>
#include <cmath>
#include <string>
#include <deque>
#include "Serializable.h"
#include "Vector.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A useful 2D axis-aligned rectangle class.
// Parent(s):       Serializable.
// Class history:   09/25/2007  Box created.

class Box:
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Box object.
// Arguments:       None.

    Box() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Box object from two points.
// Arguments:       Two Vector:s defining the initial corners of this Box.

    Box(const Vector &corner1, const Vector &corner2) { /*Clear();*/ Create(corner1, corner2); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Box object from two points.
// Arguments:       Four floats defining the initial corners of this Box.

    Box(float x1, float y1, float x2, float y2) { /*Clear();*/ Create(x1, y1, x2, y2); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Box object from one point
//                  and two dimensions.
// Arguments:       A Vector and two dimensions for width and height. They can be negative
//                  but it will affect the interpretation of which corner is defined. The
//                  Box will always return positive values for width and height.

    Box(const Vector &corner, float width, float height) { /*Clear();*/ Create(corner, width, height); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Box object
//                  identical to an already existing one.
// Arguments:       A Box object which is passed in by reference.

    Box(const Box &reference) { /*Clear();*/ Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Box
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Box object before deletion.
// Arguments:       None.

    virtual ~Box() { /*Destroy(true);*/  }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.
// Arguments:       Two Vector:s defining the initial corners of this Box.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Vector &corner1, const Vector &corner2);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.
// Arguments:       Four floats defining the initial corners of this Box.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(float x1, float y1, float x2, float y2);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.
// Arguments:       A Vector and two dimensions for width and height. They can be negative
//                  but it will affect the interpretation of which corner is defined. The
//                  Box will always return positive values for width and height.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Vector &corner, float width, float height);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Box to be identical to another, by deep copy.
// Arguments:       A reference to the Box to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Box &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Serializable, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Box to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Box will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) this Box object.
// Arguments:       None.
// Return value:    None.

    virtual void Destroy() { Clear(); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Box assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Box equal to another.
// Arguments:       A Box reference.
// Return value:    A reference to the changed Box.

    Box & operator=(const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Box:es are equal.
// Arguments:       A Box reference as the left hand side operand..
//                  A Box reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are equal or not.

    friend bool operator==(const Box &lhs, const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Box:es are unequal.
// Arguments:       A Box reference as the left hand side operand.
//                  A Box reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are unequal or not.

    friend bool operator!=(const Box &lhs, const Box &rhs);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Box. Will make a larger box which
//                  contains both operands.
// Arguments:       A Box reference as the left hand side operand.
//                  A Box reference as the right hand side operand.
// Return value:    The resulting Box.

    friend Box operator+(const Box &lhs, const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Boxs.
// Arguments:       A Box reference as the left hand side operand..
//                  A Box reference as the right hand side operand.
// Return value:    The resulting Box.

    friend Box operator-(const Box &lhs, const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for Boxs.
// Arguments:       A Box reference as the left hand side operand.
//                  A Box reference as the right hand side operand.
// Return value:    A reference to the resulting Box (the left one)

    friend Box & operator+=(Box &lhs, const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for Boxs.
// Arguments:       A Box reference as the left hand side operand.
//                  A Box reference as the right hand side operand.
// Return value:    A reference to the resulting Box (the left one)

    friend Box & operator-=(Box &lhs, const Box &rhs);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Box.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Box.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCorner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the primary corner of this box
// Arguments:       None.
// Return value:    A Vector with the primary corner of this box.

    Vector GetCorner() const { return m_Corner; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCorner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the primary corner of this box.
// Arguments:       A Vector with the new primary corner.
// Return value:    None.

    void SetCorner(const Vector &newCorner) { m_Corner = newCorner; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCenter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the primary corner of this box, by specifying where the center
//                  ought to be.
// Arguments:       A Vector with the new center point.
// Return value:    None.

    void SetCenter(const Vector &newCenter) { m_Corner.SetXY(newCenter.m_X - (m_Width / 2), newCenter.m_Y - (m_Height / 2)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of this box. Note that this can be negative if the box
//                  hasn't been righted with Unflip().
// Arguments:       None.
// Return value:    A float value that represents the width value of this Box.

    float GetWidth() const { return m_Width; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of this box. Note that this can be negative if the box
//                  hasn't been righted with Unflip().
// Arguments:       None.
// Return value:    A float value that represents the height value of this Box.

    float GetHeight() const { return m_Height; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the width of this box. Note that this can be negative if the box
//                  hasn't been righted with Unflip().
// Arguments:       A float value that represents the width value of this Box.
// Return value:    None.

    void SetWidth(float width) { m_Width = width; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the height of this box. Note that this can be negative if the box
//                  hasn't been righted with Unflip().
// Arguments:       A float value that represents the height value of this Box.
// Return value:    None.

    void SetHeight(float height) { m_Height = height; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width times the height.
// Arguments:       None.
// Return value:    The width times the height.

    float GetArea() const { return m_Width * m_Height; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCenter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the center point of this Box' area, in absolute Scene coordinates.
// Arguments:       None.
// Return value:    The center point.

    Vector GetCenter() const { return Vector(m_Corner.m_X + (m_Width / 2), m_Corner.m_Y + (m_Height / 2)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a random point within this box.
// Arguments:       None.
// Return value:    The random point within the box.

    Vector GetRandomPoint() const { return Vector(m_Corner.m_X + m_Width * PosRand(), m_Corner.m_Y + m_Height * PosRand()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unflip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the corner of this box represent the upper left corner, and both
//                  width and height will end up positive.
// Arguments:       None.
// Return value:    None.

    void Unflip();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows if this box has 0 width OR 0 height, ie can't contain anything.
// Arguments:       None.
// Return value:    Whether either width OR height are 0.

    bool IsEmpty() const { return (m_Width == 0 || m_Height == 0); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within the Box or not.
// Arguments:       The Vector describing the point to test for insideness.
// Return value:    Inside the box or not.

    bool WithinBox(const Vector &point) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBoxX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether an x coordinate is within the Box's x-range or not.
// Arguments:       The coordinate describing the x value to test for insideness.
// Return value:    Inside the box or not in the x axis.

    bool WithinBoxX(float pointX) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBoxY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether an y coordinate is within the Box's y-range or not.
// Arguments:       The coordinate describing the y value to test for insideness.
// Return value:    Inside the box or not in the y axis.

    bool WithinBoxY(float pointY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWithinBoxX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an X value constrained inside the Box and returns it.
// Arguments:       The X value to constrain inside the Box.
// Return value:    The constrained value.

    float GetWithinBoxX(float pointX) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWithinBoxY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an X value constrained inside the Box and returns it.
// Arguments:       The X value to constrain inside the Box.
// Return value:    The constrained value.

    float GetWithinBoxY(float pointY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a copy of a point constrained inside this box.
// Arguments:       The Vector describing the point to constrain inside the box.
// Return value:    The resulting point isnide the box.

    Vector GetWithinBox(const Vector &point) const { return Vector(GetWithinBoxX(point.m_X), GetWithinBoxY(point.m_Y)); } 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IntersectsBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether another box intersects this one.
// Arguments:       The other Box to check for intersection with.
// Return value:    Intersecting the other box or not.

    bool IntersectsBox(const Box &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable and method declarations

public:

    Vector m_Corner;
    float m_Width;
    float m_Height;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static const std::string ClassName;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Box, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear() { m_Corner.Reset(); m_Width = m_Height = 0; }


};

} // namespace RTE

#endif // File
