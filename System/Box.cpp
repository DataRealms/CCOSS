//////////////////////////////////////////////////////////////////////////////////////////
// File:            Box.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Box class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Box.h"
#include "DDTTools.h"

namespace RTE {

const string Box::ClassName = "Box";


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.

int Box::Create(const Vector &corner1, const Vector &corner2)
{
    m_Corner = corner1;
    m_Width = corner2.m_X - corner1.m_X;
    m_Height = corner2.m_Y - corner1.m_Y;
    Unflip();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.

int Box::Create(float x1, float y1, float x2, float y2)
{
    m_Corner.SetXY(x1, y1);
    m_Width = x2 - x1;
    m_Height = y2 - y1;
    Unflip();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Box object ready for use.

int Box::Create(const Vector &corner, float width, float height)
{
    m_Corner = corner;
    m_Width = width;
    m_Height = height;
    Unflip();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Box to be identical to another, by deep copy.

int Box::Create(const Box &reference)
{
    m_Corner = reference.m_Corner;
    m_Width = reference.m_Width;
    m_Height = reference.m_Height;
    Unflip();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Box::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Corner")
        reader >> m_Corner;
    else if (propName == "Width")
        reader >> m_Width;
    else if (propName == "Height")
        reader >> m_Height;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Box with a Writer for
//                  later recreation with Create(Reader &reader);

int Box::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("Corner");
    writer << m_Corner;
    writer.NewProperty("Width");
    writer << m_Width;
    writer.NewProperty("Height");
    writer << m_Height;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Box assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Box equal to another.

Box & Box::operator=(const Box &rhs)
{
    if (*this == rhs)
        return *this;

    Create(rhs);

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Box:es are equal.

bool operator==(const Box &lhs, const Box &rhs)
{
    if (lhs.m_Corner == rhs.m_Corner &&
        lhs.m_Width == rhs.m_Width &&
        lhs.m_Height == rhs.m_Height)
        return true;
    else
        return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Box:es are unequal.

bool operator!=(const Box &lhs, const Box &rhs)
{
    if (lhs.m_Corner != rhs.m_Corner ||
        lhs.m_Width != rhs.m_Width ||
        lhs.m_Height != rhs.m_Height)
        return true;
    else
        return false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Boxs.

Box operator+(const Box &lhs, const Box &rhs)
{
    Box returnBox(lhs.m_X + rhs.m_X, lhs.m_Y + rhs.m_Y);
    return returnBox;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Boxs.

Box operator-(const Box &lhs, const Box &rhs)
{
    Box returnBox(lhs.m_X - rhs.m_X, lhs.m_Y - rhs.m_Y);
    return returnBox;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Boxs.

Box & operator+=(Box &lhs, const Box &rhs)
{
    lhs.m_X += rhs.m_X;
    lhs.m_Y += rhs.m_Y;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Box self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Boxs.

Box & operator-=(Box &lhs, const Box &rhs)
{
    lhs.m_X -= rhs.m_X;
    lhs.m_Y -= rhs.m_Y;
    return lhs;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unflip
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the corner of this box represent the upper left corner, and both
//                  width and height will end up positive.

void Box::Unflip()
{
    if (m_Width < 0)
    {
        m_Width = -m_Width;
        m_Corner.m_X -= m_Width;
    }
    if (m_Height < 0)
    {
        m_Height = -m_Height;
        m_Corner.m_Y -= m_Height;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point is within the Box or not.

bool Box::WithinBox(const Vector &point) const
{
    // Can't be within a non-box
    if (m_Width == 0 || m_Height == 0)
        return false;

    // Take potential flipping into account
    return (((m_Width > 0 && point.m_X >= m_Corner.m_X && point.m_X < (m_Corner.m_X + m_Width)) ||
             (m_Width < 0 && point.m_X < m_Corner.m_X && point.m_X >= (m_Corner.m_X + m_Width))) &&
             (m_Height > 0 && point.m_Y >= m_Corner.m_Y && point.m_Y < (m_Corner.m_Y + m_Height)) ||
             (m_Height < 0 && point.m_Y < m_Corner.m_Y && point.m_Y <= (m_Corner.m_Y + m_Height)));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBoxX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether an x coordinate is within the Box's x-range or not.

bool Box::WithinBoxX(float pointX) const
{
    // Can't be within a non-box
    if (m_Width == 0)
        return false;

    // Take potential flipping into account
    return ((m_Width > 0 && pointX >= m_Corner.m_X && pointX < (m_Corner.m_X + m_Width)) ||
            (m_Width < 0 && pointX < m_Corner.m_X && pointX >= (m_Corner.m_X + m_Width)));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinBoxY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether an x coordinate is within the Box's y-range or not.

bool Box::WithinBoxY(float pointY) const
{
    // Can't be within a non-box
    if (m_Height == 0)
        return false;

    // Take potential flipping into account
    return ((m_Height > 0 && pointY >= m_Corner.m_Y && pointY < (m_Corner.m_Y + m_Height)) ||
            (m_Height < 0 && pointY < m_Corner.m_Y && pointY <= (m_Corner.m_Y + m_Height)));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWithinBoxX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an X value constrained inside the Box and returns it.

float Box::GetWithinBoxX(float pointX) const
{
    if (m_Width > 0)
    {
        if (pointX < m_Corner.m_X)
            return m_Corner.m_X;
        else if (pointX >= m_Corner.m_X + m_Width)
            return m_Corner.m_X + m_Width - 1.0;
        else
            return pointX;
    }
    else if (m_Width < 0)
    {
        if (pointX >= m_Corner.m_X)
            return m_Corner.m_X - 1;
        else if (pointX < m_Corner.m_X + m_Width)
            return m_Corner.m_X + m_Width;
        else
            return pointX;
    }
    
    return m_Corner.m_X;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWithinBoxY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an X value constrained inside the Box and returns it.

float Box::GetWithinBoxY(float pointY) const
{
    if (m_Height > 0)
    {
        if (pointY < m_Corner.m_Y)
            return m_Corner.m_Y;
        else if (pointY >= m_Corner.m_Y + m_Height)
            return m_Corner.m_Y + m_Height - 1.0;
        else
            return pointY;
    }
    else if (m_Height < 0)
    {
        if (pointY >= m_Corner.m_Y)
            return m_Corner.m_Y - 1;
        else if (pointY < m_Corner.m_Y + m_Height)
            return m_Corner.m_Y + m_Height;
        else
            return pointY;
    }
    
    return m_Corner.m_Y;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IntersectsUnflippedBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether another box intersects this one.

bool Box::IntersectsBox(const Box &rhs)
{
    // Can't intersect a non-box
    if (m_Width == 0 || m_Height == 0 || rhs.m_Width == 0 || rhs.m_Height == 0)
        return false;

    Box box1 = *this;
    Box box2 = rhs;
    box1.Unflip();
    box2.Unflip();

    return (box1.m_Corner.m_X < box2.m_Corner.m_X + box2.m_Width) && (box1.m_Corner.m_X + box1.m_Width > box2.m_Corner.m_X) &&
           (box1.m_Corner.m_Y < box2.m_Corner.m_Y + box2.m_Height) && (box1.m_Corner.m_Y + box1.m_Height > box2.m_Corner.m_Y);
}

} // namespace RTE