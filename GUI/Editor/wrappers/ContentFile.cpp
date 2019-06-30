//////////////////////////////////////////////////////////////////////////////////////////
// File:            ContentFile.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the ContentFile class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ContentFile.h"

#include "allegro.h"
#include "winalleg.h"


using namespace std;
//using namespace zip;

namespace RTE
{

const string ContentFile::m_ClassName = "ContentFile";
map<string, BITMAP *> ContentFile::m_sLoadedBitmaps;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ContentFile, effectively
//                  resetting the members of this abstraction level only.

void ContentFile::Clear()
{
    m_DataPath.erase();
    m_DataModuleID = 0;
    m_pLoadedData = 0;
    m_LoadedDataSize = 0;
    m_pDataFile = 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ContentFile object ready for use.

int ContentFile::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ContentFile object ready for use.

int ContentFile::Create(const char *filePath)
{
    m_DataPath = filePath;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ContentFile to be identical to another, by deep copy.

int ContentFile::Create(const ContentFile &reference)
{
//    Entity::Create(reference);

    m_DataPath = reference.m_DataPath;
    m_DataModuleID = reference.m_DataModuleID;
//    m_pLoadedData = reference.m_pLoadedData;
//    m_LoadedDataSize = reference.m_LoadedDataSize;
//    m_pDataFile = reference.m_pDataFile; Don't copy this; only the creator should keep it

    return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ContentFile object.

void ContentFile::Destroy(bool notInherited)
{
/* No, we're leaving all data loaded until engine shutdown - NO WE"RE NOT, LOOK at FreeAllLoaded
    if (m_pDataFile)
        unload_datafile_object(m_pDataFile);
*/
    // Don't delete this guy, just for conveneice, and is not and owner of the data
//    delete m_pLoadedData;

//    if (!notInherited)
//        Serializable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   FreeAllLoaded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Frees all loaded data used by all ContentFile instances. This should
//                  ONLY be done when quitting the app, or after everything else is
//                  completly destroyed

void ContentFile::FreeAllLoaded()
{
    for (map<string, BITMAP *>::iterator lbItr = m_sLoadedBitmaps.begin(); lbItr != m_sLoadedBitmaps.end(); ++lbItr)
        destroy_bitmap((*lbItr).second);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDataPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the file path of the content file represented by this ContentFile
//                  object.

void ContentFile::SetDataPath(std::string newDataPath)
{
    m_DataPath = newDataPath;

    // Reset the loaded convenience pointer
    m_pLoadedData = 0;
    m_LoadedDataSize = 0;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of the Data Module this file is inside.
// Arguments:       None.
// Return value:    The ID of the Data Module containing this' file.

int ContentFile::GetDataModuleID()
{
    return m_DataModuleID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the file size of the content file represented by this ContentFile
//                  object, in bytes.

unsigned long ContentFile::GetDataSize()
{
/*
    if (!m_pLoadedData)
        GetContent();
*/
    // Now that we have a data, return the size.
    return m_LoadedDataSize;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Allegro DATAFILE type of the DATAFILE represented by this
//                  ContentFile.

int ContentFile::GetDataType()
{
    if (!m_pDataFile)
        GetContent();

    // Now that we have a datafile, return the Allegro type.
    return m_pDataFile->type;
}
*/


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAsBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as an
//                  Allegro BITMAP. Note that ownership of the BITMAP IS NOT TRANSFERRED!

BITMAP * ContentFile::GetAsBitmap(int conversionMode)
{
    if (m_DataPath.empty())
        return 0;

    BITMAP *pReturnBitmap = 0;

    // Check if this file has already been read and loaded from disk.
    map<string, BITMAP *>::iterator itr = m_sLoadedBitmaps.find(m_DataPath);
    if (itr != m_sLoadedBitmaps.end())
    {
        // Yes, has been loaded previously, then use that data from memory.
        pReturnBitmap = (*itr).second;
    }
    // Hasn't been loaded previously, so go ahead and do so now.
    else
    {
        // Load the BITMAP from file and take ownership of it
        pReturnBitmap = LoadAndReleaseBitmap(conversionMode);

        // Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
        m_sLoadedBitmaps.insert(pair<string, BITMAP *>(m_DataPath, pReturnBitmap));
    }

    // Return without transferring ownership
    return pReturnBitmap;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadAndReleaseBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and transfers the data represtented by this ConentFile object as
//                  an Allegro BITMAP. Note that ownership of the BITMAP IS TRANSFERRED!
//                  Also, this is relatively slow since it reads the data from disk each time.

BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode)
{
    if (m_DataPath.empty())
        return 0;

    BITMAP *pReturnBitmap = 0;
    // Set the conversion mode, default to COLORCONV_MOST if none was set
    set_color_conversion(conversionMode == 0 ? COLORCONV_MOST : conversionMode);

    // Find where the '#' denoting the divider between the datafile and the datafile object's name is
    int separatorPos = m_DataPath.rfind('#');

    // If there is none, that means we're told to load an exposed file outside of a .dat datafile.
    if (separatorPos == -1)
    {
        PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);
        // Make sure we opened properly, or try to add 000 before the extension if it's part of an animation naming
        if (!pFile)
        {
            // Truncate away the extension
            int extensionPos = m_DataPath.rfind('.');
            m_DataPath.resize(extensionPos);
            pFile = pack_fopen((m_DataPath + "000.bmp").c_str(), F_READ);
            
        }

        // Get the current color palette; the load_bmp function needs it
        PALETTE currentPalette;
        get_palette(currentPalette);

        // Now load the actual BITMAP
        pReturnBitmap = load_bmp_pf(pFile, (RGB *)currentPalette);

        // Close the file stream
        pack_fclose(pFile);
    }
    // If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
    else if (separatorPos != m_DataPath.length() - 1)
    {
        // Get the Path only, without the object name, using the separator index as length limiter
        string datafilePath = m_DataPath.substr(0, separatorPos);
        // Adjusting to the true first character of the datafile object's name string.
        string objectName = m_DataPath.substr(separatorPos + 1);

        // Try loading the datafile from the specified path + object names.
        m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

        // Make sure we loaded properly.

/* Don't do this because there is no easy way to copy SAMPLE:s. Just save the datafile and unload it upon destruction.
        // Create temporary poitner to laoded bitmap; it will be destroyed along with the datafile soon
        BITMAP *pTempBitmap = (BITMAP *)m_pDataFile->dat;

        // Copy the loaded bitmap
        if (!(pReturnBitmap = create_bitmap_ex(bitmap_color_depth(pTempBitmap), pTempBitmap->w, pTempBitmap->h)))
            DDTAbort("Failed to create copy of the loaded datafile's data!");
        blit(pTempBitmap, pReturnBitmap, 0, 0, 0, 0, pTempBitmap->w, pTempBitmap->h);

        // Now unload the datafile, also destroying the loaded bitmap;
        unload_datafile_object(m_pDataFile);
        pTempBitmap = 0;
*/
        // Get the loaded data
        pReturnBitmap = (BITMAP *)m_pDataFile->dat;
    }
    
    return pReturnBitmap;
}



/* This is foolish
//////////////////////////////////////////////////////////////////////////////////////////
// Static method:  ClearAllLoadedData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the previously loaded data shared among all ContentFile
//                  instances. NEVER USE THIS UNLESS ALL INSTANCES OF CONTENTFILE ARE GONE!

static void ContentFile::ClearAllLoadedData()
{
    for (map<string, DATAFILE *>::iterator itr = m_sLoadedDataMap.begin(); itr != m_sLoadedDataMap.end(); ++itr) {
        unload_datafile_object(itr->second);
        m_sLoadedDataMap.erase(itr);
    }
}
*/
} // namespace RTE