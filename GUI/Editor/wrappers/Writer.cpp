//////////////////////////////////////////////////////////////////////////////////////////
// File:            Writer.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Writer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Writer.h"
#include <fstream>

using namespace std;
//using namespace zip;

namespace RTE
{

const string Writer::ClassName = "Writer";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Writer, effectively
//                  resetting the members of this abstraction level only.

void Writer::Clear()
{
    m_pStream = 0;
//    m_Package = 0;
    m_Indent = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Writer object ready for use.

int Writer::Create(const char *filename, bool append)
{
    m_pStream = new ofstream(filename, append ? (ios_base::out | ios_base::app | ios_base::ate) : (ios_base::out | ios_base::trunc));

    if (!m_pStream->good())
        return -1;
/*
    if (!*m_pStream) {
        delete m_pStream;

        // Get the packagename from the path, since the conventional path didn't work.
        char packageName[128];
        for (int i = 0; i < strlen(filename) && filename[i] != '/'; ++i)
            packageName[i] = filename[i];
        packageName[i] = '\0';
//        strcpy(&packageName[i], g_WritePackageExtension);

        m_Package = new ozipfile(packageName);
        if (!m_Package->isOk())
            return -1;

        m_pStream = new ozipstream(*m_Package, &filename[i + 1]);
        if (!*m_pStream)
            return -1;
    }
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Writer object.

void Writer::Destroy(bool notInherited)
{
    delete m_pStream;
//    delete m_Package;

//    if (!notInherited)
//        Serializable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  WriterOK
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the writer is ok to start accepting data streamed to it.

bool Writer::WriterOK()
{
    if (m_pStream && !m_pStream->fail() && m_pStream->is_open())
        return true;

    return false;
}

} // namespace RTE