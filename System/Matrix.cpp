//////////////////////////////////////////////////////////////////////////////////////////
// File:            Matrix.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Matrix class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Matrix.h"
#pragma intrinsic (sin, cos)
#include "DDTTools.h"

namespace RTE {

const string Matrix::ClassName = "Matrix";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Matrix, effectively
//                  resetting the members of this abstraction level only.

void Matrix::Clear()
{
    m_Rotation = 0;
    m_Flipped[X] = false;
    m_Flipped[Y] = false;
    m_ElementsUpdated = false;
    m_Elements[0][0] = 1.0;
    m_Elements[0][1] = 0.0;
    m_Elements[1][0] = 0.0;
    m_Elements[1][1] = 1.0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Matrix object ready for use.

int Matrix::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    m_ElementsUpdated = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Matrix object ready for use.

int Matrix::Create(float angle)
{
    m_Rotation = angle;
    m_ElementsUpdated = true;

    // Inverse angle to make CCW positive direction.
    float const CosAngle = (float)cos(-angle);
    float const SinAngle = (float)sin(-angle);
    m_Elements[0][0] = CosAngle;
    m_Elements[0][1] = -SinAngle;
    m_Elements[1][0] = SinAngle;
    m_Elements[1][1] = CosAngle;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ACRocket to be identical to another, by deep copy.

int Matrix::Create(const Matrix &reference)
{
//    Serializable::Create(reference);

    m_Rotation = reference.m_Rotation;
    m_Flipped[X] = reference.m_Flipped[X];
    m_Flipped[Y] = reference.m_Flipped[Y];
    m_ElementsUpdated = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Matrix::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "AngleDegrees")
    {
        float degAngle;
        reader >> degAngle;
        SetDegAngle(degAngle);
    }
    else if (propName == "AngleRadians")
        reader >> m_Rotation;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Matrix with a Writer for
//                  later recreation with Create(Reader &reader);

int Matrix::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("AngleDegrees");
    writer << GetDegAngle();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Matrix assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Matrix equal to another.

Matrix & Matrix::operator=(const Matrix &rhs)
{
    if (*this == rhs)
        return *this;

    m_Rotation = rhs.m_Rotation;
    m_Flipped[X] = rhs.m_Flipped[X];
    m_Flipped[Y] = rhs.m_Flipped[Y];
    m_ElementsUpdated = false;

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Matrix to represent an angle.

Matrix & Matrix::operator=(const float &rhs)
{
    m_Rotation = rhs;
    m_ElementsUpdated = false;

    return *this;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float conversion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A conversion operator for getting the rotational angle currently
//                  represented by this Matrix.

Matrix::operator float() const
{
    return m_Rotation;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Matrix:es are equal.

bool operator==(const Matrix &lhs, const Matrix &rhs)
{
    return lhs.m_Rotation == rhs.m_Rotation;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Matrix:es are unequal.

bool operator!=(const Matrix &lhs, const Matrix &rhs)
{
    return !operator==(lhs, rhs);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Matrix:es.

Matrix operator+(const Matrix &lhs, const Matrix &rhs)
{
    return Matrix(lhs.m_Rotation + rhs.m_Rotation);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Matrix:es.

Matrix operator-(const Matrix &lhs, const Matrix &rhs)
{
    return Matrix(lhs.m_Rotation - rhs.m_Rotation);
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for a Matrix and a float.

Matrix Matrix::operator+(const float &rhs) const
{
    return Matrix(m_Rotation + rhs);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for a Matrix and a float.

Matrix Matrix::operator-(const float &rhs) const
{
    return Matrix(m_Rotation - rhs);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for a Matrix and a Vector. The vector
//                  will be transformed according to the Matrix's elements.

Vector Matrix::operator*(const Vector &rhs)
{
    if (!m_ElementsUpdated)
        UpdateElements();

    Vector retVec = rhs;
    // Apply flipping as set.
    retVec.m_X = m_Flipped[X] ? -(retVec.m_X) : retVec.m_X;
    retVec.m_Y = m_Flipped[Y] ? -(retVec.m_Y) : retVec.m_Y;
    // Do the matrix multiplication.
    retVec.SetXY(m_Elements[0][0] * retVec.m_X + m_Elements[0][1] * retVec.m_Y,
                 m_Elements[1][0] * retVec.m_X + m_Elements[1][1] * retVec.m_Y);
    return retVec;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for a Matrix and a Vector. The vector
//                  will be transformed according to the Matrix's elements.

Vector Matrix::operator/(const Vector &rhs)
{
    if (!m_ElementsUpdated)
        UpdateElements();

    Vector retVec = rhs;
    // Apply flipping as set.
    retVec.m_X = m_Flipped[X] ? -(retVec.m_X) : retVec.m_X;
    retVec.m_Y = m_Flipped[Y] ? -(retVec.m_Y) : retVec.m_Y;
    // Do the matrix multiplication.
    retVec.SetXY(m_Elements[0][0] * retVec.m_X + m_Elements[1][0] * retVec.m_Y,
                 m_Elements[0][1] * retVec.m_X + m_Elements[1][1] * retVec.m_Y);
    return retVec;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Unary negation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unary negation overload for single Matrix:es.

Matrix Matrix::operator-()
{
    m_Rotation = -m_Rotation;

    if (m_ElementsUpdated)
    {
        // Swap two of the elements to reverse the rotation direction
        float temp = m_Elements[0][1];
        m_Elements[0][1] = m_Elements[1][0];
        m_Elements[1][0] = temp;
    }

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Matrix:es.

Matrix & operator+=(Matrix &lhs, const Matrix &rhs)
{
    lhs.m_Rotation += rhs.m_Rotation;
    lhs.m_ElementsUpdated = false;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Matrix:es.

Matrix & operator-=(Matrix &lhs, const Matrix &rhs)
{
    lhs.m_Rotation -= rhs.m_Rotation;
    lhs.m_ElementsUpdated = false;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Matrix:es.

Matrix & operator*=(Matrix &lhs, const Matrix &rhs)
{
    lhs.m_Rotation *= rhs.m_Rotation;
    lhs.m_ElementsUpdated = false;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Matrix:es.

Matrix & operator/=(Matrix &lhs, const Matrix &rhs)
{
    if (rhs.m_Rotation) {
        lhs.m_Rotation /= rhs.m_Rotation;
        lhs.m_ElementsUpdated = false;
    }
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for a Matrix and a float.

Matrix & Matrix::operator+=(const float &rhs)
{
    m_Rotation += rhs;
    m_ElementsUpdated = false;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for a Matrix and a float.

Matrix & Matrix::operator-=(const float &rhs)
{
    m_Rotation -= rhs;
    m_ElementsUpdated = false;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for a Matrix and a float.

Matrix & Matrix::operator*=(const float &rhs)
{
    m_Rotation *= rhs;
    m_ElementsUpdated = false;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     self-division operator overload for a Matrix and a float.

Matrix & Matrix::operator/=(const float &rhs)
{
    if (rhs) {
        m_Rotation /= rhs;
        m_ElementsUpdated = false;
    }
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRadAngleTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle difference between what this is currently
//                  representing, to another angle in radians. It will wrap and normalize
//                  and give the shortest absolute distance between this and the passed in.

float Matrix::GetRadAngleTo(float otherAngle) const
{
    // Rotate this' angle with the other angle so that the sought after difference angle is between the resulting angle and the x-axis
    float difference = otherAngle - GetRadAngle();

    // "Normalize" difference to range [-PI,PI)
    while(difference < -c_PI)
        difference += c_TwoPI;
    while(difference >= c_PI)
        difference -= c_TwoPI;

    // difference has the signed answer
    return difference;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDegAngleTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle difference between what this is currently
//                  representing, to another angle in degrees. It will wrap and normalize
//                  and give the shortest absolute distance between this and the passed in.

float Matrix::GetDegAngleTo(float otherAngle) const
{
    // Rotate this' angle with the other angle so that the sought after difference angle is between the resulting angle and the x-axis
    float difference = otherAngle - GetDegAngle();

    // "Normalize" difference to range [-180,180)
    while(difference < -180)
        difference += 360;
    while(difference >= 180)
        difference -= 360;

    // difference has the signed answer
    return difference;    
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetXFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this Matrix should also mirror the X component
//                  of any Vector it is multiplied with.

void Matrix::SetXFlipped(bool flipX)
{
//    if (flipX != m_Flipped[X])
//        m_Rotation = c_PI - m_Rotation;

    m_Flipped[X] = flipX;

//    m_ElementsUpdated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetYFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this Matrix should also mirror the Y component
//                  of any Vector it is multiplied with.

void Matrix::SetYFlipped(bool flipY)
{
//    if (flipY != m_Flipped[Y])
//        m_Rotation = -m_Rotation;

    m_Flipped[Y] = flipY;

//    m_ElementsUpdated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateElements
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the elements of this matrix update to represent the set angle.

void Matrix::UpdateElements()
{
    // Inverse angle to make CCW positive direction.
    float const CosAngle = (float)cos(-m_Rotation);
    float const SinAngle = (float)sin(-m_Rotation);
    m_Elements[0][0] = CosAngle;
    m_Elements[0][1] = -SinAngle;
    m_Elements[1][0] = SinAngle;
    m_Elements[1][1] = CosAngle;

    m_ElementsUpdated = true;
}

} // namespace RTE