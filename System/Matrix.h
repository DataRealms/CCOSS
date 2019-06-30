#ifndef _RTEMATRIX_
#define _RTEMATRIX_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Matrix.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Matrix class.
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
#include "Vector.h"
#include "Serializable.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Matrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A 2x2 matrix to rotate 2D Vector:s with.
// Parent(s):       Serializable.
// Class history:   02/01/2004  Matrix created.

class Matrix:
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Matrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Matrix object.
// Arguments:       None.

    Matrix() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Matrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Matrix object from an angle.
// Arguments:       A float of an angle in radians that this Matrix should be set to
//                  represent.

    Matrix(float radAng) { Clear(); Create(radAng); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Matrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Matrix object
//                  identical to an already existing one.
// Arguments:       A Matrix object which is passed in by reference.

    Matrix(const Matrix &reference) { Clear(); Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Matrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Matrix object before deletion.
// Arguments:       None.

    virtual ~Matrix() { /*Destroy(true);*/  }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Matrix object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Matrix object ready for use.
// Arguments:       The float angle in radians which this rotational matrix should
//                  represent.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(float angle);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Matrix to be identical to another, by deep copy.
// Arguments:       A reference to the Matrix to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Matrix &reference);


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
// Description:     Saves the complete state of this Matrix to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Matrix will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) this Matrix object.
// Arguments:       None.
// Return value:    None.

    virtual void Destroy() { Clear(); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Matrix assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Matrix equal to another.
// Arguments:       A Matrix reference.
// Return value:    A reference to the changed Matrix.

    Matrix & operator=(const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Matrix to represent an angle.
// Arguments:       A float in radians to set this rotational Matrix to.
// Return value:    A reference to the changed Matrix.

    Matrix & operator=(const float &rhs);

/* This is VERY dangerously ambigious when multiplying a vector by a matrix 
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float conversion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A conversion operator for getting the rotational angle currently
//                  represented by this Matrix.
// Arguments:       None.
// Return value:    A float with the represented angle in radians.

    operator float() const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Matrix:es are equal.
// Arguments:       A Matrix reference as the left hand side operand..
//                  A Matrix reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are equal or not.

    friend bool operator==(const Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Matrix:es are unequal.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are unequal or not.

    friend bool operator!=(const Matrix &lhs, const Matrix &rhs);

/* Causes operator conflicts, now handled by operator float()
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand..
//                  A Matrix reference as the right hand side operand.
// Return value:    The resulting Matrix.

    friend Matrix operator+(const Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand..
//                  A Matrix reference as the right hand side operand.
// Return value:    The resulting Matrix.

    friend Matrix operator-(const Matrix &lhs, const Matrix &rhs);
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Matrix.

    Matrix operator+(const float &rhs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    The resulting Matrix.

    Matrix operator-(const float &rhs) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for a Matrix and a Vector. The vector
//                  will be transformed according to the Matrix's elements.
// Arguments:       A Vector reference as the right hand side operand.
// Return value:    The resulting transformed Vector.

    Vector operator*(const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix-Vector multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vector:s with Matrix:es.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    friend Vector operator*(const Vector &lhs, const Matrix &rhs) 
	{ 
		Matrix m(rhs);
		return m * lhs; 
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector-Matrix self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for Vector with a Matrix.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator*=(Vector &lhs, Matrix &rhs) { return lhs = rhs * lhs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Vector division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for a Matrix and a Vector. The vector
//                  will be transformed according to the Matrix's elements.
// Arguments:       A Vector reference as the right hand side operand.
// Return value:    The resulting transformed Vector.

    Vector operator/(const Vector &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix-Vector division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vector:s with Matrix:es.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Vector.

    friend Vector operator/(const Vector &lhs, Matrix &rhs) { return rhs / lhs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Vector-Matrix self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-division operator overload for Vector with a Matrix.
// Arguments:       A Vector reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Vector (the left one)

    friend Vector & operator/=(Vector &lhs, Matrix &rhs) { return lhs = rhs / lhs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Unary negation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unary negation overload for single Matrix:es.
// Arguments:       None. (implies unary operator)
// Return value:    The resulting Matrix.

    Matrix operator-();


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix (the left one)

    friend Matrix & operator+=(Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix (the left one)

    friend Matrix & operator-=(Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix (the left one)

    friend Matrix & operator*=(Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-division operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix (the left one)

    friend Matrix & operator/=(Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Matrix self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-division operator overload for Matrix:es.
// Arguments:       A Matrix reference as the left hand side operand.
//                  A Matrix reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix (the left one)

    friend Matrix & operator/=(Matrix &lhs, const Matrix &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-addition operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix.

    Matrix & operator+=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-subtraction operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix.

    Matrix & operator-=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Self-multiplication operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix.

    Matrix & operator*=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Float self-division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     self-division operator overload for a Matrix and a float.
// Arguments:       A float reference as the right hand side operand.
// Return value:    A reference to the resulting Matrix.

    Matrix & operator/=(const float &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Matrix.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Matrix.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRadAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle this rotational Matrix is currently representing.
// Arguments:       None.
// Return value:    A float with the represented angle in radians.

    float GetRadAngle() const { return m_Rotation; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDegAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle this rotational Matrix is currently representing.
// Arguments:       None.
// Return value:    A float with the represented angle in degrees.

    float GetDegAngle() const { return (m_Rotation / PI) * 180; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRadAngleTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle difference between what this is currently
//                  representing, to another angle in radians. It will wrap and normalize
//                  and give the shortest absolute distance between this and the passed in.
// Arguments:       A float with the angle to get the difference to from this, in radians.
// Return value:    A float with the difference angle between this and the passed-in angle.

    float GetRadAngleTo(float otherAngle) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDegAngleTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle difference between what this is currently
//                  representing, to another angle in degrees. It will wrap and normalize
//                  and give the shortest absolute distance between this and the passed in.
// Arguments:       A float with the angle to get the difference to from this, in degrees.
// Return value:    A float with the difference angle between this and the passed-in angle.

    float GetDegAngleTo(float otherAngle) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllegroAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the angle this rotational Matrix is currently representing.
// Arguments:       None.
// Return value:    A float with the represented angle as full rotations being 256.

    float GetAllegroAngle() const { return (m_Rotation / PI) * -128; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetXFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether or not this Matrix also mirrors the X component of any
//                  Vector it is multiplied with.
// Arguments:       None.
// Return value:    A bool with the setting whether flipping the X components or not.

    bool GetXFlipped() { return m_Flipped[X]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetYFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether or not this Matrix also mirrors the Y component of any
//                  Vector it is multiplied with.
// Arguments:       None.
// Return value:    A bool with the setting whether flipping the Y components or not.

    bool GetYFlipped() { return m_Flipped[Y]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRadAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the angle that this rotational Matrix should represent.
// Arguments:       A float with the new angle, in radians.
// Return value:    None.

    void SetRadAngle(float newAngle) { m_Rotation = newAngle; m_ElementsUpdated = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDegAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the angle that this rotational Matrix should represent.
// Arguments:       A float with the new angle, in degrees.
// Return value:    None.

    void SetDegAngle(float newAngle) { m_Rotation = (newAngle / 180) * PI;
                                       m_ElementsUpdated = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetXFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this Matrix should also mirror the X component
//                  of any Vector it is multiplied with.
// Arguments:       A bool with the setting whether to flip the X components or not.
// Return value:    None.

    void SetXFlipped(bool flipX = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetYFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether or not this Matrix should also mirror the Y component
//                  of any Vector it is multiplied with.
// Arguments:       A bool with the setting whether to flip the Y components or not.
// Return value:    None.

    void SetYFlipped(bool flipY = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets this Matrix to an identity Matrix, representing a 0 angle.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable and method declarations

public:

    // The angle represented, in radians.
    float m_Rotation;
    // Whether or not this Matrix also mirrors the X axis of its invoked Vector:s
    bool m_Flipped[2];
    // Whether the elements are currently updated to the set angle.
    bool m_ElementsUpdated;
    // The elements of the matrix, which represent the angle.
    float m_Elements[2][2];


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
// Description:     Clears all the member variables of this Matrix, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateElements
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the elements of this matrix update to represent the set angle.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateElements();


};

} // namespace RTE

#endif // File
