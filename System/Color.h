#ifndef _RTECOLOR_
#define _RTECOLOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Color.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Color class.
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

namespace RTE
{

    enum { R = 0, G, B};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An class representing a RGB color value.
// Parent(s):       Serializable.
// Class history:   08/26/2004  Color created.

class Color:
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object.
// Arguments:       None.

    Color() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object from RGB values
// Arguments:       Three floats defining the initial Red Green Blue values of this Color.

    Color(unsigned char R, unsigned char G, unsigned char B) { Clear(); Create(R, G, B); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object from an entry
//                  in the current color palette.
// Arguments:       Three floats defining the initial Red Green Blue values of this Color.

    Color(unsigned char index) { Clear(); SetRGBWithIndex(index); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object from RGB values.
// Arguments:       Three floats defining the initial Red Green Blue values of this Color.

    Color(int inputX, int inputY);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Color object
//                  identical to an already existing one.
// Arguments:       A Color object which is passed in by reference.

    Color(const Color &reference) { Clear(); Create(reference.m_R, reference.m_G, reference.m_B); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Color object before deletion.
// Arguments:       None.

    virtual ~Color() { /*Destroy(true);*/  }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Color object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Color object ready for use.
// Arguments:       Three floats defining the initial Red Green Blue values of this Color.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(unsigned char inputR, unsigned char inputG, unsigned char inputB)
    {
        m_R = inputR;
        m_G = inputG;
        m_B = inputB;

        RecalculateIndex();

        return 0;
    }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Color to be identical to another, by deep copy.
// Arguments:       A reference to the Color to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Color &reference);
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
// Method:          Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets RGB of this Color.to zero.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Color to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Color will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) this Color object.
// Arguments:       None.
// Return value:    None.

    virtual void Destroy() { Clear(); }
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Color assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Color equal to another.
// Arguments:       A Color reference.
// Return value:    A reference to the changed Color.

    Color & operator=(const Color &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Color average assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting this Color equal to the average of
//                  an std::deque of Color:s.
// Arguments:       A reference to an std::deque of Color:s that shall be averaged.
// Return value:    A reference to the changed Color (this).

    Color & operator=(const std::deque<Color> &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Color:es are equal.
// Arguments:       A Color reference as the left hand side operand..
//                  A Color reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are equal or not.

    friend bool operator==(const Color &lhs, const Color &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Color:es are unequal.
// Arguments:       A Color reference as the left hand side operand.
//                  A Color reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are unequal or not.

    friend bool operator!=(const Color &lhs, const Color &rhs);
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Vectors.
// Arguments:       A Color reference as the left hand side operand..
//                  A Color reference as the right hand side operand.
// Return value:    The resulting Color.

    friend Color operator+(const Color &lhs, const Color &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Vectors.
// Arguments:       A Color reference as the left hand side operand..
//                  A Color reference as the right hand side operand.
// Return value:    The resulting Color.

    friend Color operator-(const Color &lhs, const Color &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vectors.
// Arguments:       A Color reference as the left hand side operand..
//                  A Color reference as the right hand side operand.
// Return value:    The resulting Color.

    friend Color operator*(const Color &lhs, const Color &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vectors.
// Arguments:       A Color reference as the left hand side operand..
//                  A Color reference as the right hand side operand.
// Return value:    The resulting Color.

    friend Color operator/(const Color &lhs, const Color &rhs);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Color.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Color.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Array subscripting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Array subscripting to access either the X or Y element of this Color.
// Arguments:       An int index indicating which element is requested (X = 0, Y = 1)
// Return value:    The requested element.

    const unsigned char & operator[](const int &rhs) const { return rhs == 0 ? m_R : (rhs == 1 ? m_G : m_B); }

/* Don't want to give write access directly to members without being able to check for changes
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Array subscripting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Array subscripting to access either the X or Y element of this Color.
// Arguments:       An int index indicating which element is requested (X = 0, Y = 1)
// Return value:    The requested element.

    unsigned char & operator[](const int &rhs) { return rhs == 0 ? m_R : (rhs == 1 ? m_G : m_B); }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetR
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the red value of this Color.
// Arguments:       None.
// Return value:    A unsigned char value that represents the R value of this Color. 0 - 255.

    unsigned char GetR() const { return m_R; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetG
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the green value of this Color.
// Arguments:       None.
// Return value:    A unsigned char value that represents the G value of this Color. 0 - 255

    unsigned char GetG() const { return m_G; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetB
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the blue value of this Color.
// Arguments:       None.
// Return value:    A unsigned char value that represents the B value of this Color. 0 - 255

    unsigned char GetB() const { return m_B; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RecalculateIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Causes recalculation of the nearest index even though there might be
//                  one cached or not.
// Arguments:       None.
// Return value:    The new color entry index number.

    unsigned char RecalculateIndex();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the entry in the current color palette that most closely matches
//                  this Color's RGB values.
// Arguments:       None.
// Return value:    The color entry index number.

    unsigned char GetIndex() const { return m_Index; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetR
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the red value of this Color.
// Arguments:       A unsigned char value that the R value will be set to, between 0 and 255.0
// Return value:    None.

//    void SetR(float newR) { m_R = newR < 0 ? 0 : (newR > 255.0 ? 255.0 : newR); m_Index = 0; }
    void SetR(unsigned char newR) { m_R = newR; m_Index = 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetG
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the green value of this Color.
// Arguments:       A unsigned char value that the green value will be set to, between 0 and 255.0
// Return value:    None.

//    void SetG(float newG) { m_G = newG < 0 ? 0 : (newG > 255.0 ? 255.0 : newG); m_Index = 0; }
    void SetG(unsigned char newG) { m_G = newG; m_Index = 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetB
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the blue value of this Color.
// Arguments:       A unsigned char value that the blue value will be set to, between 0 and 255.0
// Return value:    None.

//    void SetB(float newB) { m_B = newB < 0 ? 0 : (newB > 255.0 ? 255.0 : newB); m_Index = 0; }
    void SetB(unsigned char newB) { m_B = newB; m_Index = 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRGB
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all three RGB values of this Color.
// Arguments:       Three unsigned char values that the R G and B values will be set to, respectively.
// Return value:    None.

//    void SetRGB(const float newR, const float newG, const float newB) { SetR(newR); SetG(newG); SetB(newB); m_Index = 0; }
    void SetRGB(unsigned char newR, unsigned char newG, unsigned char newB) { m_R = newR; m_G = newG; m_B = newB; m_Index = 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRGBWithIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all three RGB values of this Color, using an index from the
//                  current color palette.
// Arguments:       The index of the palette entry that this Color object's RGB values
//                  should be set to.
// Return value:    None.

    void SetRGBWithIndex(unsigned char index);


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable and method declarations

public:

    // The components of this color: Red, Green, and Blue.
    unsigned char m_R;
    unsigned char m_G;
    unsigned char m_B;
    // The closest matching index in the current color palette. If 0, this needs to
    // be recalculated and updated.
    unsigned char m_Index;


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
// Description:     Clears all the member variables of this Color, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear() { m_R = m_G = m_B = m_Index = 0; }


};

} // namespace RTE

#endif // File
