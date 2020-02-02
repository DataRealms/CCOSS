#ifndef _RTEVECTOR_
#define _RTEVECTOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Vector.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Vector class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"

#if defined(__clang__)
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif 

namespace RTE
{

enum { X = 0, Y = 1 };


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A useful 2D float vector.
// Parent(s):       Serializable.
// Class history:   02/22/2001  Vector created.

class Vector:
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object.
// Arguments:       None.

    Vector() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object from X and
//                  Y values.
// Arguments:       Two floats defining the initial X and Y values of this Vector.

    Vector(float inputX, float inputY) { /*Clear();*/ Create(inputX, inputY); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Vector object from X and
//                  Y values.
// Arguments:       Two int defining the initial X and Y values of this Vector.

    Vector(int inputX, int inputY);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Vector object
//                  identical to an already existing one.
// Arguments:       A Vector object which is passed in by reference.

    Vector(const Vector &reference) { /*Clear();*/ Create(reference.m_X, reference.m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Vector
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Vector object before deletion.
// Arguments:       None.

    virtual ~Vector() { /*Destroy(true);*/  }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Vector object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Vector object ready for use.
// Arguments:       Two floats defining the initial X and Y values of this Vector.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(float inputX, float inputY)
    {
        m_X = inputX;
        m_Y = inputY;
        return 0;
    }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Vector to be identical to another, by deep copy.
// Arguments:       A reference to the Vector to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Vector &reference);
*/


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
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Vector to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Vector will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Vector object ready for use.
// Arguments:       An input stream that the Vector will create itself from.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(std::istream &stream, bool checkType = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Vector to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       An output stream that the Vector will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(std::ostream &stream) const;
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) this Vector object.
// Arguments:       None.
// Return value:    None.

    virtual void Destroy() { Clear(); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Vector equal to another.
// Arguments:       A Vector reference.
// Return value:    A reference to the changed Vector.

    Vector & operator=(const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector average assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting this Vector equal to the average of
//                  an std::deque of Vector:s.
// Arguments:       A reference to an std::deque of Vector:s that shall be averaged.
// Return value:    A reference to the changed Vector (this).

    Vector & operator=(const std::deque<Vector> &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Vector:es are equal.
// Arguments:       A Vector reference as the left hand side operand..
//                  A Vector reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are equal or not.

    friend bool operator==(const Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Vector:es are unequal.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are unequal or not.

    friend bool operator!=(const Vector &lhs, const Vector &rhs);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector stream extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A stream extraction operator for filling a Vector from an input
//                  stream.
// Arguments:       An istream reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    An istream reference for further use in an expression.

    friend std::istream & operator>>(std::istream &stream, Vector &operand) { operand.Create(stream);
                                                                              return stream; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector stream insertion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A stream insertion operator for sending a Vector to an output
//                  stream.
// Arguments:       An ostream reference as the left hand side operand.
//                  A CSimpleString reference as the right hand side operand.
// Return value:    An ostream reference for further use in an expression.

    friend std::ostream & operator<<(std::ostream &stream, const Vector &operand) { stream << "{" << operand.m_X << ", " << operand.m_Y << "}";
                                                                                    return stream; }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand..
//                  A Vector reference as the right hand side operand.
// Return value:    The resulting Vector.

    friend Vector operator+(const Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand..
//                  A Vector reference as the right hand side operand.
// Return value:    The resulting Vector.

    friend Vector operator-(const Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand..
//                  A Vector reference as the right hand side operand.
// Return value:    The resulting Vector.

    friend Vector operator*(const Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand..
//                  A Vector reference as the right hand side operand.
// Return value:    The resulting Vector.

    friend Vector operator/(const Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Vector.

    Vector operator+(const float &rhs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Vector.

    Vector operator-(const float &rhs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Vector.

    Vector operator*(const float &rhs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Vector.

    Vector operator/(const float &rhs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Unary negation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unary negation overload for single Vectors.
// Arguments:       None. (implies unary operator)
// Return value:    The resulting Vector.

    Vector operator-();


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator+=(Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator-=(Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator*=(Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-division operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator/=(Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-division operator overload for Vectors.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Vector reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator/=(Vector &lhs, const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    Vector & operator+=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    Vector & operator-=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    Vector & operator*=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     self-division operator overload for a Vector and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    Vector & operator/=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Vector.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Vector.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Array subscripting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Array subscripting to access either the X or Y element of this Vector.
// Arguments:       An int index indicating which element is requested (X = 0, Y = 1)
// Return value:    The requested element.

    const float & operator[](const int &rhs) const { return rhs == 0 ? m_X : m_Y; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Array subscripting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Array subscripting to access either the X or Y element of this Vector.
// Arguments:       An int index indicating which element is requested (X = 0, Y = 1)
// Return value:    The requested element.

    float & operator[](const int &rhs) { return rhs == 0 ? m_X : m_Y; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the X value of this Vector.
// Arguments:       None.
// Return value:    A float value that represents the X value of this Vector.

    float GetX() const { return m_X; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Y value of this Vector.
// Arguments:       None.
// Return value:    A float value that represents the Y value of this Vector.

    float GetY() const { return m_Y; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundIntX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the rounded integer X value of this Vector.
// Arguments:       None.
// Return value:    A int value that represents the X value of this Vector.

    int GetRoundIntX() const { return static_cast<int>(floorf(m_X + 0.5)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundIntY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the rounded integer Y value of this Vector.
// Arguments:       None.
// Return value:    A int value that represents the Y value of this Vector.

    int GetRoundIntY() const { return static_cast<int>(floorf(m_Y + 0.5)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRounded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a rounded copy of this Vector. Does not alter this
//                  Vector.
// Arguments:       None.
// Return value:    A rounded copy of this Vector.

    Vector GetRounded() const { Vector returnVector(GetRoundIntX(), GetRoundIntY());
                                return returnVector; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFloorIntX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the greatest integer that is not greater than the X value of
//                  this Vector.
// Arguments:       None.
// Return value:    A int value that represents the X value of this Vector.

    int GetFloorIntX() const { return static_cast<int>(floorf(m_X)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFloorIntY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the greatest integer that is not greater than the Y value of
//                  this Vector.
// Arguments:       None.
// Return value:    A int value that represents the Y value of this Vector.

    int GetFloorIntY() const { return static_cast<int>(floorf(m_Y)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFloored
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a floored copy of this Vector. Does not alter this
//                  Vector.
// Arguments:       None.
// Return value:    A floored copy of this Vector.

    Vector GetFloored() const { Vector returnVector(GetFloorIntX(), GetFloorIntY());
                                return returnVector; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCeilingIntX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the lowest integer that is not less than the X value of this
//                  Vector.
// Arguments:       None.
// Return value:    A int value that represents the X value of this Vector.

    int GetCeilingIntX() const { return static_cast<int>(ceil(m_X)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCeilingIntY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the lowest integer that is not less than the Y value of this
//                  Vector.
// Arguments:       None.
// Return value:    A int value that represents the Y value of this Vector.

    int GetCeilingIntY() const { return static_cast<int>(ceil(m_Y)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCeilinged
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a ceiling:ed copy of this Vector. Does not alter this
//                  Vector.
// Arguments:       None.
// Return value:    A ceiling:ed copy of this Vector.

    Vector GetCeilinged() const { Vector returnVector(GetCeilingIntX(), GetCeilingIntY());
                                  return returnVector; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the magnitude of this Vector.
// Arguments:       None.
// Return value:    A float describing the magnitude.

    float GetMagnitude() const { return sqrt((m_X * m_X) + (m_Y * m_Y)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLargest
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute largest of the two elements. Will always be positive.
// Arguments:       None.
// Return value:    A float describing the largest value of the two, but not the magnitude.

    float GetLargest() const { return fabs(fabs(m_X) > fabs(m_Y) ? m_X : m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSmallest
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute smallest of the two elements. Will always be positive.
// Arguments:       None.
// Return value:    A float describing the smallest value of the two, but not the magnitude.

    float GetSmallest() const { return fabs(fabs(m_X) > fabs(m_Y) ? m_Y : m_X); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNormalized
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a Vector that has the same direction as this but with a
//                  magnitude of 1.0.
// Arguments:       None.
// Return value:    None.

    Vector GetNormalized() const { return *this / GetMagnitude(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPerpendicular
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a Vector that is perpendicular to this, rotated PI/2.
// Arguments:       None.
// Return value:    None.

    Vector GetPerpendicular() const { return Vector(m_Y, -m_X); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetXFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a Vector identical to this except that its X component is flipped.
// Arguments:       Whether to flip the X axis of the return vector or not.
// Return value:    A copy of this vector with flipped X axis.

    Vector GetXFlipped(bool xFlip = true) const { Vector retVec((xFlip ? -m_X : m_X), m_Y);
                                                  return retVec; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetYFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a Vector identical to this except that its Y component is flipped.
// Arguments:       Whether to flip the Y axis of the return vector or not.
// Return value:    A copy of this vector with flipped Y axis.

    Vector GetYFlipped(bool yFlip = true) const { Vector retVec(m_X, (yFlip ? -m_Y : m_Y));
                                                  return retVec; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsRadAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in radians. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields -pi/2 here.
// Arguments:       None.
// Return value:    The absolute angle in radians.

    float GetAbsRadAngle() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsDegAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get this Vector's absolute angle in degrees. e.g: when x = 1, y = 0,
//                  the value returned here will be 0. x = 0, y = 1 yields -90 here.
// Arguments:       None.
// Return value:    The absolute angle in degrees.

    float GetAbsDegAngle() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIntX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the X value of this Vector.
// Arguments:       A int value that the X value will be set to.
// Return value:    None.

    void SetIntX(const int newX) { m_X = (float)newX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIntY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Y value of this Vector.
// Arguments:       A int value that the Y value will be set to.
// Return value:    None.

    void SetIntY(const int newY) { m_Y = (float)newY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIntXY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets both the X and Y values of this Vector.
// Arguments:       Two int values that the X and Y value will be set to, respectively.
// Return value:    None.

    void SetIntXY(const int newX, const int newY) { m_X = (float)newX; m_Y = (float)newY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the X value of this Vector.
// Arguments:       A float value that the X value will be set to.
// Return value:    None.

    void SetX(const float newX) { m_X = newX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Y value of this Vector.
// Arguments:       A float value that the Y value will be set to.
// Return value:    None.

    void SetY(const float newY) { m_Y = newY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetXY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets both the X and Y values of this Vector.
// Arguments:       Two float values that the X and Y value will be set to, respectively.
// Return value:    None.

    void SetXY(const float newX, const float newY) { m_X = newX; m_Y = newY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the magnitude of this Vector and keeps its angle intact.
// Arguments:       A float value that the magnitude will be set to.
// Return value:    A reference to this after the change.

    Vector & SetMagnitude(float newMag);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CapMagnitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Caps the magnitude of this Vector to a max value and keeps its angle
//                  intact.
// Arguments:       A float value that the magnitude will be capped by.
// Return value:    A reference to this after the change.

    Vector & CapMagnitude(float capMag);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlipX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flips the X element of this Vector.
// Arguments:       Whether or not to flip the X element or not.
// Return value:    None.

    void FlipX(bool flipX = true) { m_X = flipX ? -m_X : m_X; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FlipY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Flips the Y element of this Vector.
// Arguments:       Whether or not to flip the Y element or not.
// Return value:    None.

    void FlipY(bool flipY = true) { m_Y = flipY ? -m_Y : m_Y; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsZero
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether both x and y components of this Vector are 0;
// Arguments:       None.
// Return value:    Whether both x and y components of this Vector are 0.

    bool IsZero() const { return m_X == 0 && m_Y == 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOpposedTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the x and y components of this Vector each have
//                  opposite signs to their corresponding components of a passed in Vector.
// Arguments:       The Vector to compare with.
// Return value:    Whether both x and y components of this Vector are 0.

    bool IsOpposedTo(const Vector &opp) { return (!m_X && !opp.m_X) || (m_X < 0 && opp.m_X > 0) || (m_X > 0 && opp.m_X < 0) &&
                                                 (!m_Y && !opp.m_Y) || (m_Y < 0 && opp.m_Y > 0) || (m_Y > 0 && opp.m_Y < 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Dot
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the dot product of this Vector and the passed in Vector.
// Arguments:       The Vector which will be the rhs operand of the dot product operation.
// Return value:    The resulting dot product scalar float.

    float Dot(const Vector &rhs) { return (m_X * rhs.m_X) + (m_Y * rhs.m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Cross
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the 2D cross product of this Vector and the passed in Vector.
//                  This is really the area of the parallellogram that the two vectors form.
// Arguments:       The Vector which will be the rhs operand of the cross product operation.
// Return value:    The resulting 2d cross product parallellogram area.

    float Cross(const Vector &rhs) { return (m_X * rhs.m_Y) - (rhs.m_X * m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Round
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rounds the X and Y values of this Vector upwards. E.g. 0.49 -> 0.0
//                  and 0.5 -> 1.0.
// Arguments:       None.
// Return value:    None.

    void Round() { m_X = floorf(m_X + 0.5);
                   m_Y = floorf(m_Y + 0.5); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToHalf
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the X and Y of this Vector to the nearest half value.
//                  E.g. 1.0 -> 1.5 and 0.9 -> 0.5.
// Arguments:       None.
// Return value:    None.

    void ToHalf() { m_X = floorf(m_X) + 0.5;
                    m_Y = floorf(m_Y) + 0.5; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Floor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the X and Y of this Vector.to the greatest integers that are
//                  not greater than their original values. E.g. -1.02 becomes -2.0.
// Arguments:       None.
// Return value:    None.

    void Floor() { m_X = floorf(m_X);
                   m_Y = floorf(m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Ceiling
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the X and Y of this Vector.to the lowest integers that are
//                  not less than their original values. E.g. -1.02 becomes -1.0.
// Arguments:       None.
// Return value:    None.

    void Ceiling() { m_X = ceil(m_X);
                     m_Y = ceil(m_Y); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Normalize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Scales this vector to have the same direction but a magnitude of 1.0.
// Arguments:       None.
// Return value:    Vector reference to this after the operation.

    Vector & Normalize() { return *this /= GetMagnitude(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Perpendicularize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this vector perpendicular to its previous state, rotated PI/2.
//                  Much faster than RadRotate by PI/2.
// Arguments:       None.
// Return value:    Vector reference to this after the operation.

    Vector & Perpendicularize() { float temp = -m_X; m_X = m_Y; m_Y = temp; return *this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets both the X and Y of this Vector.to zero.
// Arguments:       None.
// Return value:    None.

    void Reset() { m_X = m_Y = 0.0f; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RadRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector relatively by an angle in radians.
// Arguments:       The angle in radians to rotate by. Positive angles rotate counter-
//                  clockwise, and negative angles clockwise.
// Return value:    This vector, rotated.

    Vector & RadRotate(float angle);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DegRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector relatively by an angle in degrees.
// Arguments:       The angle in degrees to rotate by. Positive angles rotate counter-
//                  clockwise, and negative angles clockwise.
// Return value:    This vector, rotated.

    Vector & DegRotate(float angle);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AbsRotateTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set this Vector to an absolute rotation based on the absolute rotation
//                  of another Vector.
// Arguments:       The reference Vector whose absolute angle from positive X (0 degrees)
//                  this Vector will be rotated to.
// Return value:    This vector, rotated.

    Vector & AbsRotateTo(const Vector &refVector);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DegRotate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotate this Vector around itself by a relative rotation in degrees.
// Arguments:       The angle in degrees to rotate. Positive is counter clockwise, and
//                  negative is clockwise rotation.
// Return value:    None.

    void DegRotate(float angle);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable and method declarations

public:

    float m_X;
    float m_Y;


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
// Description:     Clears all the member variables of this Vector, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear() { m_X = m_Y = 0; }


};

} // namespace RTE

#endif // File
