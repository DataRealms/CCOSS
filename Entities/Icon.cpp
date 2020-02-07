//////////////////////////////////////////////////////////////////////////////////////////
// File:            Icon.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Icon class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Icon.h"
#include "DDTTools.h"
#include "FrameMan.h"

namespace RTE {

CONCRETECLASSINFO(Icon, Entity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Icon, effectively
//                  resetting the members of this abstraction level only.

void Icon::Clear()
{
    m_BitmapFile.Reset();
    m_FrameCount = 0;
    m_apBitmaps8 = 0;
    m_apBitmaps32 = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Icon object ready for use.

int Icon::Create()
{
/*
    // Read all the properties
    if (Entity::Create() < 0)
        return -1;
*/
    // Load the bitmaps
    if (!m_apBitmaps8 || !m_apBitmaps32)
    {
        // If no file specified, load the default icon bitmap
        if (m_BitmapFile.GetDataPath().empty())
            m_BitmapFile.SetDataPath("Base.rte/GUIs/PieIcons/Blank001.bmp");
        m_apBitmaps8 = m_BitmapFile.GetAsAnimation(m_FrameCount, COLORCONV_REDUCE_TO_256);
        m_apBitmaps32 = m_BitmapFile.GetAsAnimation(m_FrameCount, COLORCONV_8_TO_32);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Arm to be identical to another, by deep copy.

int Icon::Create(const Icon &reference)
{
    Entity::Create(reference);

    m_BitmapFile = reference.m_BitmapFile;
    m_FrameCount = reference.m_FrameCount;
    if (reference.m_apBitmaps8 && reference.m_apBitmaps32)
    {
        m_apBitmaps8 = new BITMAP *[m_FrameCount];
        m_apBitmaps32 = new BITMAP *[m_FrameCount];
        for (int frame = 0; frame < m_FrameCount; ++frame)
        {
            m_apBitmaps8[frame] = reference.m_apBitmaps8[frame];
            m_apBitmaps32[frame] = reference.m_apBitmaps32[frame];
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int Icon::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "BitmapFile")
        reader >> m_BitmapFile;
    else if (propName == "FrameCount")
        reader >> m_FrameCount;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Icon with a Writer for
//                  later recreation with Create(Reader &reader);

int Icon::Save(Writer &writer) const
{
    Entity::Save(writer);
    writer.NewProperty("BitmapFile");
    writer << m_BitmapFile;
    writer.NewProperty("FrameCount");
    writer << m_FrameCount;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Icon object.

void Icon::Destroy(bool notInherited)
{
    delete [] m_apBitmaps8;
    delete [] m_apBitmaps32;

    if (!notInherited)
        Entity::Destroy();
    Clear();
}

} // namespace RTE