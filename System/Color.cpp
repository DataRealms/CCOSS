//////////////////////////////////////////////////////////////////////////////////////////
// File:            Color.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Color class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Color.h"
#pragma intrinsic (sin, cos)
#include "RTETools.h"
#include "FrameMan.h"

namespace RTE {

const string Color::ClassName = "Color";

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object.

Color::Color():
    m_X(0.0),
    m_Y(0.0)
{
    
}
*/
/*
/////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object from X and
//                  Y values.

Color::Color(float inputX, float inputY):
    m_X(inputX),
    m_Y(inputY)
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Color object from X and
//                  Y values.

Color::Color(int inputX, int inputY):
    m_X(static_cast<float>(inputX)),
    m_Y(static_cast<float>(inputY))
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Color object
//                  identical to an already existing one.

Color::Color(const Color &reference):
    m_X(reference.m_X),
    m_Y(reference.m_Y)
{
    
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Color
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up the Color object before
//                  termination.

Color::~Color()
{
    
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Color object ready for use.

int Color::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    RecalculateIndex();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Color::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "R")
        reader >> m_R;
    else if (propName == "G")
        reader >> m_G;
    else if (propName == "B")
        reader >> m_B;
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Color with a Writer for
//                  later recreation with Create(Reader &reader);

int Color::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("R");
    writer << m_R;
    writer.NewProperty("G");
    writer << m_G;
    writer.NewProperty("B");
    writer << m_B;

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Color assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Color equal to another.

Color & Color::operator=(const Color &rhs)
{
    if (*this == rhs)
        return *this;

    m_X = rhs.m_X;
    m_Y = rhs.m_Y;

    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Color average assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting this Color equal to the average of
//                  an std::deque of Color:s.

Color & Color::operator=(const deque<Color> &rhs)
{
    Clear();
    if (rhs.empty())
        return *this;

    for (deque<Color>::const_iterator itr = rhs.begin(); itr != rhs.end(); ++itr)
        *this += *itr;
    *this /= rhs.size();
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two Color:es are equal.

bool operator==(const Color &lhs, const Color &rhs)
{
    if (lhs.m_X == rhs.m_X &&
        lhs.m_Y == rhs.m_Y)
        return true;
    else
        return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two Color:es are unequal.

bool operator!=(const Color &lhs, const Color &rhs)
{
    if (lhs.m_X != rhs.m_X ||
        lhs.m_Y != rhs.m_Y)
        return true;
    else
        return false;
}
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for Vectors.

Color operator+(const Color &lhs, const Color &rhs)
{
    Color returnVector(lhs.m_X + rhs.m_X, lhs.m_Y + rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color subtraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Subtraction operator overload for Vectors.

Color operator-(const Color &lhs, const Color &rhs)
{
    Color returnVector(lhs.m_X - rhs.m_X, lhs.m_Y - rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color multiplication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Multiplication operator overload for Vectors.

Color operator*(const Color &lhs, const Color &rhs)
{
    Color returnVector(lhs.m_X * rhs.m_X, lhs.m_Y * rhs.m_Y);
    return returnVector;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Color division
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Division operator overload for Vectors.

Color operator/(const Color &lhs, const Color &rhs)
{
    Color returnVector(0, 0);
    if (rhs.m_X && rhs.m_Y)
        returnVector.SetXY(lhs.m_X / rhs.m_X, lhs.m_Y / rhs.m_Y);
    return returnVector;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RecalculateIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Causes recalculation of the nearest index even though there might be
//                  one cached or not.

unsigned char Color::RecalculateIndex()
{
    return m_Index = makecol8(m_R, m_G, m_B);
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the entry in the current color palette that most closely matches
//                  this Color's RGB values.

unsigned char Color::GetIndex() const
{
    return m_Index;

    m_Index = makecol8(m_R, m_G, m_B);

//    OR OLD WAY:

    float bestDist = 10000000.0f;
    unsigned char bytePalette[256 * 4];
    unsigned char entRed, entGreen, entBlue;
    int lastIndex = (ignoreLastIndex ? 255 : 256);

    // Get the current palette
//    LPDIRECTDRAWPALETTE palette = g_FrameMan.GetScreen()->GetPalette();
//    if (!palette)
//        return 0;
    LPDIRECTDRAWPALETTE palette;
    g_FrameMan.GetScreen()->GetFront()->GetDDS()->GetPalette(&palette);
    palette->GetEntries(0, 0, 256, (tagPALETTEENTRY *)&(bytePalette[0]));
    palette->Release();

    float vTarget = sqrt((float)(m_R * m_R + m_G * m_G + m_B * m_B)) * 0.57735027;

    for (int i = 0; i < lastIndex; ++i) {
        entRed = bytePalette[i * 4];
        entGreen = bytePalette[i * 4 + 1];
        entBlue = bytePalette[i * 4 + 2];

        float vPalette = sqrt((float)(entRed * entRed + entGreen * entGreen + entBlue * entBlue)) * 0.57735027;

        float dR = entRed - m_R;
        float dG = entGreen - m_G;
        float dB = entBlue - m_B;
        float dV = vPalette - vTarget;
        float dist = dR * dR * 0.3 + dG * dG * 0.59 + dB * dB * 0.11 + dV * dV * 0.7;

        if (dist < bestDist) {
                bestDist = dist;
                m_Index = i;
        }
    }
    return m_Index;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRGBWithIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all three RGB values of this Color, using an index from the
//                  current color palette.

void Color::SetRGBWithIndex(unsigned char index)
{
    m_Index = index;

    RGB color;
    get_color((int)m_Index, &color);

    m_R = color.r * 4;
    m_G = color.g * 4;
    m_B = color.b * 4;

/*
    unsigned char bytePalette[256 * 4];
    // Get the current palette
//    LPDIRECTDRAWPALETTE palette = g_FrameMan.GetScreen()->GetPalette();
    LPDIRECTDRAWPALETTE palette;
    g_FrameMan.GetScreen()->GetFront()->GetDDS()->GetPalette(&palette);
    RTEAssert(palette, "Couldn't get palette!");
    palette->GetEntries(0, 0, 256, (tagPALETTEENTRY *)&(bytePalette[0]));
    palette->Release();

    m_R = bytePalette[m_Index * 4];
    m_G = bytePalette[m_Index * 4 + 1];
    m_B = bytePalette[m_Index * 4 + 2];
*/
}

} // namespace RTE