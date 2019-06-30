//////////////////////////////////////////////////////////////////////////////////////////
// File:            Vector.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Vector class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Vector.h"
#pragma intrinsic (sin, cos)
#ifdef _WIN32
#pragma float_control(precise, on)
#endif
#include "DDTTools.h"

using namespace std;

namespace RTE
{

const string Vector::ClassName = "Vector";

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object.

Vector::Vector():
    m_X(0.0),
    m_Y(0.0)
{
    
}
*/
/*
/////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object from X and
//                  Y values.

Vector::Vector(float inputX, float inputY):
    m_X(inputX),
    m_Y(inputY)
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object from X and
//                  Y values.

Vector::Vector(int inputX, int inputY):
    m_X(static_cast<float>(inputX)),
    m_Y(static_cast<float>(inputY))
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Vector object
//                  identical to an already existing one.

Vector::Vector(const Vector &reference):
    m_X(reference.m_X),
    m_Y(reference.m_Y)
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up the Vector object before
//                  termination.

Vector::~Vector()
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Vector object ready for use.

int Vector::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Vector::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "X")
        reader >> m_X;
    else if (propName == "Y")
        reader >> m_Y;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Vector with a Writer for
//                  later recreation with Create(Reader &reader);

int Vector::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("X");
    writer << m_X;
    writer.NewProperty("Y");
    writer << m_Y;

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Vector object ready for use.

int Vector::Create(istream &stream, bool checkType)
{
    if (checkType)
    {
        string name;
        stream >> name;
        if (name != ClassName)
        {
           DDTAbort("Wrong type in stream when passed to Create");
           return -1;
        }
    }

    stream >> m_X;
    stream >> m_Y;

// TODO: Exception checking/handling")
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Vector to an output stream for
//                  later recreation with Create(istream &stream);

int Vector::Save(ostream &stream) const
{
// TODO: Exception checking/handling")
    stream << ClassName << " ";

    stream << m_X << " ";
    stream << m_Y;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Vector equal to another.

Vector & Vector::operator=(const Vector &rhs)
{
    if (*this == rhs)
        return *this;

    m_X = rhs.m_X;
    m_Y = rhs.m_Y;

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector average assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting this Vector equal to the average of
//                  an std::deque of Vector:s.

Vector & Vector::operator=(const deque<Vector> &rhs)
{
    Clear();
    if (rhs.empty())
        return *this;

    for (deque<Vector>::const_iterator itr = rhs.begin(); itr != rhs.end(); ++itr)
        *this += *itr;
    *this /= rhs.size();
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Vector:es are equal.

bool operator==(const Vector &lhs, const Vector &rhs)
{
    if (lhs.m_X == rhs.m_X &&
        lhs.m_Y == rhs.m_Y)
        return true;
    else
        return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Vector:es are unequal.

bool operator!=(const Vector &lhs, const Vector &rhs)
{
    if (lhs.m_X != rhs.m_X ||
        lhs.m_Y != rhs.m_Y)
        return true;
    else
        return false;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector stream extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A stream extraction operator for filling a Vector from an input
//                  stream.

istream & operator>>(istream &stream, Vector &operand)
{
    operand.Destroy();

    stream >> operand.m_X;
    stream >> operand.m_Y;
    return stream;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector stream insertion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A stream insertion operator for sending a Vector to an output
//                  stream.

ostream & operator<<(ostream &stream, const Vector &operand)
{
// TODO: Error checking/handling")
    stream << "[Vector] ";
    stream << operand.m_X << " ";
    stream << operand.m_Y;
    return stream;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Vectors.

Vector operator+(const Vector &lhs, const Vector &rhs)
{
    Vector returnVector(lhs.m_X + rhs.m_X, lhs.m_Y + rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Vectors.

Vector operator-(const Vector &lhs, const Vector &rhs)
{
    Vector returnVector(lhs.m_X - rhs.m_X, lhs.m_Y - rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vectors.

Vector operator*(const Vector &lhs, const Vector &rhs)
{
    Vector returnVector(lhs.m_X * rhs.m_X, lhs.m_Y * rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vectors.

Vector operator/(const Vector &lhs, const Vector &rhs)
{
    Vector returnVector(0, 0);
    if (rhs.m_X && rhs.m_Y)
        returnVector.SetXY(lhs.m_X / rhs.m_X, lhs.m_Y / rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for a Vector and a float.

Vector Vector::operator+(const float &rhs) const
{
    Vector returnVector(m_X + rhs, m_Y + rhs);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for a Vector and a float.

Vector Vector::operator-(const float &rhs) const
{
    Vector returnVector(m_X - rhs, m_Y - rhs);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for a Vector and a float.

Vector Vector::operator*(const float &rhs) const
{
    Vector returnVector(m_X * rhs, m_Y * rhs);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for a Vector and a float.

Vector Vector::operator/(const float &rhs) const
{
    Vector returnVector(0, 0);
    if (rhs)
        returnVector.SetXY(m_X / rhs, m_Y / rhs);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Unary negation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unary negation overload for single Vectors.

Vector Vector::operator-()
{
    Vector returnVector(-m_X, -m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Vectors.

Vector & operator+=(Vector &lhs, const Vector &rhs)
{
    lhs.m_X += rhs.m_X;
    lhs.m_Y += rhs.m_Y;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Vectors.

Vector & operator-=(Vector &lhs, const Vector &rhs)
{
    lhs.m_X -= rhs.m_X;
    lhs.m_Y -= rhs.m_Y;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vectors.

Vector & operator*=(Vector &lhs, const Vector &rhs)
{
    lhs.m_X *= rhs.m_X;
    lhs.m_Y *= rhs.m_Y;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vectors.

Vector & operator/=(Vector &lhs, const Vector &rhs)
{
    if (rhs.m_X)
        lhs.m_X /= rhs.m_X;
    if (rhs.m_Y)
        lhs.m_Y /= rhs.m_Y;
    return lhs;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for a Vector and a float.

Vector & Vector::operator+=(const float &rhs)
{
    m_X += rhs;
    m_Y += rhs;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for a Vector and a float.

Vector & Vector::operator-=(const float &rhs)
{
    m_X -= rhs;
    m_Y -= rhs;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for a Vector and a float.

Vector & Vector::operator*=(const float &rhs)
{
    m_X *= rhs;
    m_Y *= rhs;
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     self-division operator overload for a Vector and a float.

Vector & Vector::operator/=(const float &rhs)
{
    if (rhs) {
        m_X /= rhs;
        m_Y /= rhs;
    }
    return *this;
}

/* This is with clockwise is positive rotation
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsRadAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in radians. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields pi/2 here.

float Vector::GetAbsRadAngle()
{
    if (m_X == 0)
        return m_Y > 0 ? HalfPI : (m_Y < 0 ? -HalfPI : 0);
    if (m_Y == 0)
        return m_X > 0 ? 0 : (m_X < 0 ? PI : 0);

// TODO: Confirm that this is correct!")
    float rawAngle = atan(m_Y / m_X);
    if (m_X < 0 && m_Y > 0 || m_X < 0 && m_Y < 0)
        rawAngle += PI;
    return rawAngle;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsDegAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in degrees. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields 90 here.

float Vector::GetAbsDegAngle()
{
    if (m_X == 0)
        return m_Y > 0 ? 90 : (m_Y < 0 ? -90 : 0);
    if (m_Y == 0)
        return m_X > 0 ? 0 : (m_X < 0 ? 180 : 0);

// TODO: Confirm that this is correct!")
    float rawAngle = (atan(m_Y / m_X) / PI) * 180;
    if (m_X < 0 && m_Y > 0 || m_X < 0 && m_Y < 0)
        rawAngle += 180;
    return rawAngle;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsRadAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in radians. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields -pi/2 here.

float Vector::GetAbsRadAngle() const
{
    if (m_X == 0)
        return m_Y > 0 ? -HalfPI : (m_Y < 0 ? HalfPI : 0);
    if (m_Y == 0)
        return m_X > 0 ? 0 : (m_X < 0 ? PI : 0);

// TODO: Confirm that this is correct!")
    float rawAngle = -atan(m_Y / m_X);
    if (m_X < 0)
        rawAngle += PI;
    return rawAngle;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsDegAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in degrees. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields -90 here.

float Vector::GetAbsDegAngle() const
{
    if (m_X == 0)
        return m_Y > 0 ? -90 : (m_Y < 0 ? 90 : 0);
    if (m_Y == 0)
        return m_X > 0 ? 0 : (m_X < 0 ? 180 : 0);

    float rawAngle = -(atan(m_Y / m_X) / PI) * 180;
    if (m_X < 0)
        rawAngle += 180;
    return rawAngle;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the magnitude of this Vector and keeps its angle intact.

Vector & Vector::SetMagnitude(float newMag)
{
    Vector temp(*this);
    SetXY(newMag, 0);
    AbsRotateTo(temp);
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CapMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Caps the magnitude of this Vector to a max value and keeps its angle
//                  intact.

Vector & Vector::CapMagnitude(float capMag)
{
    if (capMag == 0)
        Reset();

    if (GetMagnitude() > capMag)
        SetMagnitude(capMag);
    return *this;
}

/* This is prior to changing angle sign convention from CW rotation being positive
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RadRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set this Vector to an absolute rotation in radians.

void Vector::RadRotate(float angle)
{
    float tempX = m_X * cos(angle) - m_Y * sin(angle);
    float tempY = m_X * sin(angle) + m_Y * cos(angle);
    m_X = tempX;
    m_Y = tempY;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DegRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set this Vector to an absolute rotation in degrees.

void Vector::DegRotate(float angle)
{
    // Convert to radians.
    angle /= 180;
    angle *= PI;

    float tempX = m_X * cos(angle) - m_Y * sin(angle);
    float tempY = m_X * sin(angle) + m_Y * cos(angle);
    m_X = tempX;
    m_Y = tempY;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RadRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector relatively by an angle in radians.

Vector & Vector::RadRotate(float angle)
{
    angle = -angle;
    float tempX = m_X * cos(angle) - m_Y * sin(angle);
    float tempY = m_X * sin(angle) + m_Y * cos(angle);
    m_X = tempX;
    m_Y = tempY;

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DegRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector relatively by an angle in degrees.

Vector & Vector::DegRotate(float angle)
{
    angle = -angle;

    // Convert to radians.
    angle /= 180;
    angle *= PI;

    float tempX = m_X * cos(angle) - m_Y * sin(angle);
    float tempY = m_X * sin(angle) + m_Y * cos(angle);
    m_X = tempX;
    m_Y = tempY;

    return *this;
}

/* before angle sign convention change
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AbsRotateTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set this Vector to an absolute rotation based on the absolute rotation
//                  of another Vector.

void Vector::AbsRotateTo(const Vector &refVector)
{
    float rawAngle;
    if (refVector.m_X == 0)
        rawAngle = refVector.m_Y > 0 ? HalfPI : (refVector.m_Y < 0 ? -HalfPI : 0);
    else if (refVector.m_Y == 0)
        rawAngle = refVector.m_X > 0 ? 0 : (refVector.m_X < 0 ? PI : 0);
    else {
        rawAngle = atan(refVector.m_Y / refVector.m_X);
        if (refVector.m_X < 0 && refVector.m_Y > 0 || refVector.m_X < 0 && refVector.m_Y <= 0)
            rawAngle += PI;
    }

    float tempX = m_X * cos(rawAngle) - m_Y * sin(rawAngle);
    float tempY = m_X * sin(rawAngle) + m_Y * cos(rawAngle);
    m_X = tempX;
    m_Y = tempY;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AbsRotateTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set this Vector to an absolute rotation based on the absolute rotation
//                  of another Vector.

Vector & Vector::AbsRotateTo(const Vector &refVector)
{
    float rawAngle;
    if (refVector.m_X == 0)
        rawAngle = refVector.m_Y > 0 ? -HalfPI : (refVector.m_Y < 0 ? HalfPI : 0);
    else if (refVector.m_Y == 0)
        rawAngle = refVector.m_X > 0 ? 0 : (refVector.m_X < 0 ? PI : 0);
    else {
        rawAngle = -atan(refVector.m_Y / refVector.m_X);
        if (refVector.m_X < 0)
            rawAngle += PI;
    }
    rawAngle = -rawAngle;

    m_X = GetMagnitude();
    m_Y = 0.0;

    float tempX = m_X * cos(rawAngle) - m_Y * sin(rawAngle);
    float tempY = m_X * sin(rawAngle) + m_Y * cos(rawAngle);
    m_X = tempX;
    m_Y = tempY;

    return *this;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DegRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector around itself by a relative rotation in degrees.

void Vector::DegRotate(float angle)
{
    // Convert to radians.
    angle /= 180;
    angle *= 3.1416;

    float tempX = m_X * cos(angle) - m_Y * sin(angle);
    float tempY = m_X * sin(angle) + m_Y * cos(angle);
    m_X = tempX;
    m_Y = tempY;
}
*/
} // namespace RTE