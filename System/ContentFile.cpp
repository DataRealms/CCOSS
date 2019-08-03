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

#ifdef __USE_SOUND_FMOD
#include "fmod.h"
#elif __USE_SOUND_SDLMIXER
#include "SDL.h"
#include "SDL_mixer.h"
#elif __USE_SOUND_GORILLA
#include "gorilla/ga.h"
#include "gorilla/gau.h"
#endif

#include "ContentFile.h"
#include "PresetMan.h"

#include "allegro.h"

using namespace std;

namespace RTE
{

const string ContentFile::m_ClassName = "ContentFile";
map<string, BITMAP *> ContentFile::m_sLoadedBitmaps[BitDepthCount];
map<size_t, std::string> ContentFile::m_PathHashes;

#ifdef __USE_SOUND_FMOD
map<string, FSOUND_SAMPLE *> ContentFile::m_sLoadedSamples;
#elif __USE_SOUND_SDLMIXER
map<string, Mix_Chunk *> ContentFile::m_sLoadedSamples;
#elif __USE_SOUND_GORILLA
map<string, ga_Sound *> ContentFile::m_sLoadedSamples;
#endif // __USE_SOUND_FMOD


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ContentFile, effectively
//                  resetting the members of this abstraction level only.

void ContentFile::Clear()
{
    m_DataPath.erase();
    m_DataModuleID = 0;
    m_DataModified = false;
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

#ifndef WIN32
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *fcase( const char *path )
{
	DIR *folder = opendir( "." );
	if ( !folder ) return 0;

	char *fixed = ( char * ) calloc( strlen( path ) + 1, sizeof( char ) );
	char *copy = strdup( path );
	char *current = strtok( copy, "/\\" );

	while ( current )
	{
		fixed[ strlen( fixed ) ] = '/';
		dirent *entry;
		bool found = false;

		char *next = strtok( 0, "/" );

		while ( entry = readdir( folder ) )
		{
			if ( !strcasecmp( entry->d_name, current ) )
			{
				memcpy( fixed + ( current - copy ), entry->d_name, strlen( entry->d_name ) );

				if ( next )
				{
					// We're still looking for a directory; make sure this is one
					// (in case there's a file named the same as a dir)
					DIR *test = opendir( fixed );
					if ( test != 0 )
					{
						closedir( test );
						found = true;
						break;
					}
				}
				else
				{
					found = true;
					break;
				}
			}
		}

		if ( !found )
		{
			free( copy );
			closedir( folder );
			return 0;
		}

		current = next;
		if ( current )
		{
			closedir( folder );
			folder = opendir( fixed );
			if ( !folder )
			{
				free( copy );
				return 0;
			}
		}
	}

	free( copy );
	fixed[ strlen( path ) ] = 0;
	closedir( folder );
	return fixed;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ContentFile object ready for use.

int ContentFile::Create(const char *filePath)
{
    m_DataPath = filePath;
	m_PathHashes[GetHash()] = m_DataPath;

#ifndef WIN32
    char *fixed = fcase( m_DataPath.c_str() );
    if ( fixed )
    {
        m_DataPath.assign( fixed );
        free( fixed );
    }
#endif
    m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);

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
#ifndef WIN32
    char *fixed = fcase( m_DataPath.c_str() );
    if ( fixed )
    {
        m_DataPath.assign( fixed );
        free( fixed );
    }
#endif
    m_DataModuleID = reference.m_DataModuleID;
    m_DataModified = reference.m_DataModified;
//    m_pLoadedData = reference.m_pLoadedData;
//    m_LoadedDataSize = reference.m_LoadedDataSize;
//    m_pDataFile = reference.m_pDataFile; Don't copy this; only the creator should keep it

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int ContentFile::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "Path" || propName == "FilePath")
    {
        m_DataPath = reader.ReadPropValue();
#ifndef WIN32
        char *fixed = fcase( m_DataPath.c_str() );
        if ( fixed )
        {
            m_DataPath.assign( fixed );
            free( fixed );
        }
#endif
        m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
		m_PathHashes[GetHash()] = m_DataPath;
    }
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ContentFile with a Writer for
//                  later recreation with Create(Reader &reader);

int ContentFile::Save(Writer &writer) const
{
    Serializable::Save(writer);

    if (!m_DataPath.empty())
    {
        writer.NewProperty("FilePath");
        writer << m_DataPath;
    }

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
    for (int depth = Eight; depth < BitDepthCount; ++depth)
    {
        for (map<string, BITMAP *>::iterator lbItr = m_sLoadedBitmaps[depth].begin(); lbItr != m_sLoadedBitmaps[depth].end(); ++lbItr)
            destroy_bitmap((*lbItr).second);
    }

#ifdef __USE_SOUND_FMOD
	for (map<string, FSOUND_SAMPLE *>::iterator lcItr = m_sLoadedSamples.begin(); lcItr != m_sLoadedSamples.end(); ++lcItr)
        FSOUND_Sample_Free((*lcItr).second);
#endif // __USE_SOUND_FMOD
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDataPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the file path of the content file represented by this ContentFile
//                  object.

void ContentFile::SetDataPath(std::string newDataPath)
{
    m_DataPath = newDataPath;
	m_PathHashes[GetHash()] = m_DataPath;
#ifndef WIN32
    char *fixed = fcase( m_DataPath.c_str() );
    if ( fixed )
    {
        m_DataPath.assign( fixed );
        free( fixed );
    }
#endif
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
    return m_DataModuleID < 0 ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID;
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
// Virtual method:  GetAsContour
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as a
//                  binary chunk of data, representing a Terrian Contour. Note that
//                  ownership of the DATA IS NOT TRANSFERRED!
//                  Also, this should only be done once upon startup, since getting the
//                  data again is slow.

int * ContentFile::GetAsContour()
{
    if (m_DataPath.empty())
        return 0;

    int *pReturnContour = 0;
    int length = 0;

    // Don't bother with storing loaded contours and retrieveing them; dupes will seldom be loaded.

    // Find where the '#' denoting the divider between the datafile and the datafile object's name is
    int separatorPos = m_DataPath.rfind('#');

    // If there is none, that means we're told to load an exposed file outside of a .dat datafile.
    if (separatorPos == -1)
    {
        PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);

        // Make sure we opened properly.
        if (!pFile)
            DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

        // Read the length header
        if (pack_fread(&length, sizeof(int), pFile) < sizeof(int))
            DDTAbort(("Failed to read contour size header of contour file:\n\n" + m_DataPath).c_str());

        // Make the memory buffer, including room for the length header
        pReturnContour = new int[length + 1];

        // Set the length header
        pReturnContour[0] = length;

        // Now read the rest of the contour into the memory, after the header
        if (pack_fread(&(pReturnContour[1]), length * sizeof(int), pFile) < length)
            DDTAbort(("Failed to read entire contour:\n\n" + m_DataPath).c_str());

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
        if (!m_pDataFile || !m_pDataFile->dat/* || m_pDataFile->type != DAT_BITMAP*/)
            DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

        // Get the loaded data
        pReturnContour = (int *)m_pDataFile->dat;
    }
    else
        DDTAbort("No object name following first #-sign in ContentFile's datafile object path!");

    if (!pReturnContour)
        DDTAbort(("Failed to load Content File with following path and name:\n\n" + m_DataPath).c_str());

    return pReturnContour;
}


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

    // Determine the bit depth this bitmap will be loaded as
    int bitDepth = conversionMode == COLORCONV_8_TO_32 ? ThirtyTwo : Eight;

    // Check if this file has already been read and loaded from disk.
    map<string, BITMAP *>::iterator itr = m_sLoadedBitmaps[bitDepth].find(m_DataPath);
    if (itr != m_sLoadedBitmaps[bitDepth].end())
    {
        // Yes, has been loaded previously, then use that data from memory.
        pReturnBitmap = (*itr).second;
    }
    // Hasn't been loaded previously, so go ahead and do so now.
    else
    {
        // Load the BITMAP from file and take ownership of it
        pReturnBitmap = LoadAndReleaseBitmap(conversionMode);

        if (!pReturnBitmap)
            DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

        // Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
        m_sLoadedBitmaps[bitDepth].insert(pair<string, BITMAP *>(m_DataPath, pReturnBitmap));
    }

    // Return without transferring ownership
    return pReturnBitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAsAnimation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as an
//                  of array Allegro BITMAP:s, each represnting a frame in an animation.
//                  Note that ownership of the BITMAP:S ARE NOT TRANSFERRED, BUT THE ARRAY
//                  ITSELF, IS!

BITMAP ** ContentFile::GetAsAnimation(int frameCount, int conversionMode)
{
    if (m_DataPath.empty())
        return 0;

    // Create the array of as many BITMAP pointers as requested frames
    BITMAP **aReturnBitmaps = new BITMAP *[frameCount];

    // Don't mess with numbers if there's only one frame
    if (frameCount == 1)
    {
        aReturnBitmaps[0] = GetAsBitmap(conversionMode);
        return aReturnBitmaps;
    }

    // The file extension string, if any.
    string extension = "";

    // Find where the '#' denoting the divider between the datafile and the datafile object's name is
    int separatorPos = m_DataPath.find('#');

    // Position of the dot in the file extension, if any.
    int extensionPos = 0;

    // If there is none, that means we're told to load an exposed file outside of a .dat datafile.
    // This also means we need to handle file extensions
    if (separatorPos == -1)
    {
        // Find the filename extension
        extensionPos = m_DataPath.rfind('.');
        AAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps!");

        // Save our extension from the datapath
        extension.assign(m_DataPath, extensionPos, m_DataPath.length() - extensionPos);
        // Truncate the datapath
        m_DataPath.resize(extensionPos);
    }

    string originalDataPath = m_DataPath;
    char framePath[1024];
    for (int i = 0; i < frameCount; i++)
    {
        // Create the temporary frame datapath
        sprintf(framePath, "%s%03i%s", originalDataPath.c_str(), i, extension.c_str());
        // Temporarily assign it to the datapath member var so that GetAsBitmap uses it
        m_DataPath = framePath;
        // Get the frame bitmap
        aReturnBitmaps[i] = GetAsBitmap(conversionMode);
        AAssert(aReturnBitmaps[i], "Could not get a frame of animation");
    }

    // Set the data path back to its original state (without any numbers after it)
    m_DataPath = originalDataPath;

    // Append back the extension, if there was any
    if (separatorPos > 0 || extensionPos > 0)
        m_DataPath += extension;

    // Return the loaded BITMAP:s
    return aReturnBitmaps;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAsSample
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as an
//                  FMOD FSOUND_SAMPLE. Note that ownership of the SAMPLE IS NOT TRANSFERRED!
//                  Also, this should only be done once upon startup, since getting the
//                  Sample again is slow.

#ifdef __USE_SOUND_FMOD
FSOUND_SAMPLE * ContentFile::GetAsSample()
{
    if (m_DataPath.empty())
        return 0;

    FSOUND_SAMPLE *pReturnSample = 0;

    // Check if this file has already been read and loaded from disk.
    map<string, FSOUND_SAMPLE *>::iterator itr = m_sLoadedSamples.find(m_DataPath);
    if (itr != m_sLoadedSamples.end())
    {
        // Yes, has been loaded previously, then use that data from memory.
        pReturnSample = (*itr).second;
    }
    // Hasn't been loaded previously, so go ahead and do so now.
    else
    {
        // Find where the '#' denoting the divider between the datafile and the datafile object's name is
        int separatorPos = m_DataPath.rfind('#');

        // Size of the entire file to be read
        long fileSize;
        // Holder of the raw data read from the pack file
        char *pRawData = 0;

        // If there is none, that means we're told to load an exposed file outside of a .dat datafile.
        if (separatorPos == -1)
        {
            fileSize = file_size(m_DataPath.c_str());
            PACKFILE *pFile = pack_fopen(GetPlatformPath().c_str(), F_READ);

            // Make sure we opened properly.
            if (!pFile || fileSize <= 0)
                DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

            // Allocte the raw data space in memory
            pRawData = new char[fileSize];

            // Read the raw data from the file
            int bytesRead = pack_fread(pRawData, fileSize, pFile);
            AAssert(bytesRead == fileSize, "Tried to read a file but couldn't read the same amount of data as the reported file size!");

            // Load the sample from the memory we've read from the file. FSOUND_UNMANAGED because we want to manage the freeing of the sample ourselves
            pReturnSample = FSOUND_Sample_Load(FSOUND_UNMANAGED, pRawData, FSOUND_LOADMEMORY, 0, fileSize);

            // Deallocate the intermediary data
            delete [] pRawData;

            // Close the file stream
            pack_fclose(pFile);
        }
        // If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
        else if (separatorPos != m_DataPath.length() - 1)
        {
            DDTAbort("Loading sound samples from allegro datafiles isn't supported yet!");
// TODO loading SDL_mixer chunks from allegro datafiles! this!
/*
            // Get the Path only, without the object name, using the separator index as length limiter
            string datafilePath = m_DataPath.substr(0, separatorPos);
            // Adjusting to the true first character of the datafile object's name string.
            string objectName = m_DataPath.substr(separatorPos + 1);

            // Try loading the datafile from the specified path + object names.
            m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

            // Make sure we loaded properly.
            if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_SAMPLE)
                DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

            // Get the loaded data
            pReturnSample = (FSOUND_SAMPLE *)m_pDataFile->dat;
*/
        }
        else
            DDTAbort("No object name following first #-sign in ContentFile's datafile object path!");

        if (!pReturnSample)
            DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

        // Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
        m_sLoadedSamples.insert(pair<string, FSOUND_SAMPLE *>(m_DataPath, pReturnSample));
    }

    return pReturnSample;
}
#elif __USE_SOUND_SDLMIXER
Mix_Chunk * ContentFile::GetAsSample()
{
	if (m_DataPath.empty())
		return 0;

	Mix_Chunk *pReturnSample = 0;

	// Check if this file has already been read and loaded from disk.
	map<string, Mix_Chunk *>::iterator itr = m_sLoadedSamples.find(m_DataPath);
	if (itr != m_sLoadedSamples.end())
	{
		// Yes, has been loaded previously, then use that data from memory.
		pReturnSample = (*itr).second;
	}
	// Hasn't been loaded previously, so go ahead and do so now.
	else
	{
		// Find where the '#' denoting the divider between the datafile and the datafile object's name is
		int separatorPos = m_DataPath.rfind('#');

		// Size of the entire file to be read
		long fileSize;
		// Holder of the raw data read from the pack file
		char *pRawData = 0;

		// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
		if (separatorPos == -1)
		{
			fileSize = file_size(m_DataPath.c_str());
			PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);

			// Make sure we opened properly.
			if (!pFile || fileSize <= 0)
				DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

			// Allocte the raw data space in memory
			pRawData = new char[fileSize];

			// Read the raw data from the file
			int bytesRead = pack_fread(pRawData, fileSize, pFile);
			AAssert(bytesRead == fileSize, "Tried to read a file but couldn't read the same amount of data as the reported file size!");

			// Load the sample from the memory we've read from the file. FSOUND_UNMANAGED because we want to manage the freeing of the sample ourselves
			pReturnSample = Mix_LoadWAV_RW(SDL_RWFromMem(pRawData, fileSize), 0);
			if (pReturnSample == 0)
				DDTAbort(SDL_GetError());

			// Deallocate the intermediary data
			delete[] pRawData;

			// Close the file stream
			pack_fclose(pFile);
		}
		// If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
		else if (separatorPos != m_DataPath.length() - 1)
		{
			DDTAbort("Loading sound samples from allegro datafiles isn't supported yet!");
			// TODO loading SDL_mixer chunks from allegro datafiles! this!
			/*
			// Get the Path only, without the object name, using the separator index as length limiter
			string datafilePath = m_DataPath.substr(0, separatorPos);
			// Adjusting to the true first character of the datafile object's name string.
			string objectName = m_DataPath.substr(separatorPos + 1);

			// Try loading the datafile from the specified path + object names.
			m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

			// Make sure we loaded properly.
			if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_SAMPLE)
			DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

			// Get the loaded data
			pReturnSample = (FSOUND_SAMPLE *)m_pDataFile->dat;
			*/
		}
		else
			DDTAbort("No object name following first #-sign in ContentFile's datafile object path!");

		if (!pReturnSample)
			DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

		// Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
		m_sLoadedSamples.insert(pair<string, Mix_Chunk *>(m_DataPath, pReturnSample));
	}

	return pReturnSample;
}
#elif __USE_SOUND_GORILLA
ga_Sound * ContentFile::GetAsSample()
{
	if (m_DataPath.empty())
		return 0;

	ga_Sound *pReturnSample = 0;

	// Check if this file has already been read and loaded from disk.
	map<string, ga_Sound *>::iterator itr = m_sLoadedSamples.find(m_DataPath);
	if (itr != m_sLoadedSamples.end())
	{
		// Yes, has been loaded previously, then use that data from memory.
		pReturnSample = (*itr).second;
	}
	// Hasn't been loaded previously, so go ahead and do so now.
	else
	{
		// Find where the '#' denoting the divider between the datafile and the datafile object's name is
		int separatorPos = m_DataPath.rfind('#');

		// Size of the entire file to be read
		long fileSize;
		// Holder of the raw data read from the pack file
		char *pRawData = 0;

		// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
		if (separatorPos == -1)
		{
            std::string platform_path = GetPlatformPath();
			fileSize = file_size(platform_path.c_str());
			PACKFILE *pFile = pack_fopen(platform_path.c_str(), F_READ);

			// Make sure we opened properly.
			if (!pFile || fileSize <= 0)
				DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

			// Allocte the raw data space in memory
			pRawData = new char[fileSize];

			// Read the raw data from the file
			int bytesRead = pack_fread(pRawData, fileSize, pFile);
			AAssert(bytesRead == fileSize, "Tried to read a file but couldn't read the same amount of data as the reported file size!");

			// Load the sample from the memory we've read from the file.


			ga_Memory * mem = ga_memory_create(pRawData, fileSize);
			ga_DataSource * data = gau_data_source_create_memory(mem);
			ga_SampleSource * samples = gau_sample_source_create_wav(data);
			pReturnSample = ga_sound_create_sample_source(samples);

			//pReturnSample = gau_load_sound_file(m_DataPath.c_str(), "wav");

			if (pReturnSample == 0)
				DDTAbort(("Unable to create ga_sound " + m_DataPath).c_str());

			// Deallocate the intermediary data
			delete[] pRawData;

			// Close the file stream
			pack_fclose(pFile);
		}
		// If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
		else if (separatorPos != m_DataPath.length() - 1)
		{
			DDTAbort("Loading sound samples from allegro datafiles isn't supported yet!");
			// TODO loading SDL_mixer chunks from allegro datafiles! this!
			/*
			// Get the Path only, without the object name, using the separator index as length limiter
			string datafilePath = m_DataPath.substr(0, separatorPos);
			// Adjusting to the true first character of the datafile object's name string.
			string objectName = m_DataPath.substr(separatorPos + 1);

			// Try loading the datafile from the specified path + object names.
			m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

			// Make sure we loaded properly.
			if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_SAMPLE)
			DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

			// Get the loaded data
			pReturnSample = (FSOUND_SAMPLE *)m_pDataFile->dat;
			*/
		}
		else
			DDTAbort("No object name following first #-sign in ContentFile's datafile object path!");

		if (!pReturnSample)
			DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

		// Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
		m_sLoadedSamples.insert(pair<string, ga_Sound *>(m_DataPath, pReturnSample));
	}

	return pReturnSample;
}
#endif // __USE_SOUND_FMOD

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
        PACKFILE *pFile = pack_fopen(GetPlatformPath().c_str(), F_READ);
        // Make sure we opened properly, or try to add 000 before the extension if it's part of an animation naming
        if (!pFile)
        {
            // Truncate away the extension
            int extensionPos = m_DataPath.rfind('.');
            m_DataPath.resize(extensionPos);
#ifdef WIN32
            pFile = pack_fopen((m_DataPath + "000.bmp").c_str(), F_READ);
#else
            std::string newpath = m_DataPath + "000.bmp";
            const char *fixed = fcase( newpath.c_str() );
            if ( !fixed ) fixed = newpath.c_str();
            pFile = pack_fopen( fixed, F_READ );
            if ( !pFile )
            {
                newpath = m_DataPath + ".bmp";
                fixed = fcase( newpath.c_str() );
                if ( !fixed ) fixed = newpath.c_str();
                pFile = pack_fopen( fixed, F_READ );
            }
#endif
            if (!pFile)
                DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());
            // Add the extension back in so it can get saved properly later
            m_DataPath = m_DataPath + ".bmp";
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
        if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_BITMAP)
            DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

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
    else
        DDTAbort("No object name following first #-sign in ContentFile's datafile object path!");

    if (!pReturnBitmap)
        DDTAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str());

    return pReturnBitmap;
}

std::string ContentFile::GetPathFromHash(size_t hash)
{
	std::string result;

	if (m_PathHashes.count(hash) == 1)
		result = m_PathHashes[hash];

	return result;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadAndReleaseAnimation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and transfers the data represtented by this ConentFile object as
//                  a series of Allegro BITMAPs. Note that ownership of both the ARRAY and
//                  the BITMAPS ARE TRANSFERRED! Also, this is relatively slow since it
//                  reads the data from disk each time.

BITMAP ** ContentFile::LoadAndReleaseAnimation(int frameCount, int conversionMode)
{
    if (m_DataPath.empty())
        return 0;

    // Create the array of as many BITMAP pointers as requested frames
    BITMAP **aReturnBitmaps = new BITMAP *[frameCount];

    // Don't mess with numbers if there's only one frame
    if (frameCount == 1)
    {
        aReturnBitmaps[0] = LoadAndReleaseBitmap(conversionMode);
        return aReturnBitmaps;
    }

    // The file extension string, if any.
    string extension = "";

    // Find where the '#' denoting the divider between the datafile and the datafile object's name is
    int separatorPos = m_DataPath.find('#');

    // Position of the dot in the file extension, if any.
    int extensionPos = 0;

    // If there is none, that means we're told to load an exposed file outside of a .dat datafile.
    // This also means we need to handle file extensions
    if (separatorPos == -1)
    {
        // Find the filename extension
        extensionPos = m_DataPath.rfind('.');
        AAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps!");

        // Save our extension from the datapath
        extension.assign(m_DataPath, extensionPos, m_DataPath.length() - extensionPos);
        // Truncate the datapath
        m_DataPath.resize(extensionPos);
    }

    string originalDataPath = m_DataPath;
    char framePath[1024];
    for (int i = 0; i < frameCount; i++)
    {
        // Create the temporary frame datapath
        sprintf(framePath, "%s%03i%s", originalDataPath.c_str(), i, extension.c_str());
        // Temporarily assign it to the datapath member var so that GetAsBitmap uses it
        m_DataPath = framePath;
        // Get the frame bitmap
        aReturnBitmaps[i] = LoadAndReleaseBitmap(conversionMode);
        AAssert(aReturnBitmaps[i], "Could not get a frame of animation");
    }

    // Set the data path back to its original state (without any numbers after it)
    m_DataPath = originalDataPath;

    // Append back the extension, if there was any
    if (separatorPos > 0 || extensionPos > 0)
        m_DataPath += extension;

    // Return the loaded BITMAP:s
    return aReturnBitmaps;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetPlatformPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns data path in a platform specific representation
// Arguments:       
// Return value:    Platform specific representation of path
    
std::string ContentFile::GetPlatformPath()
{
    std::string rv;
#ifdef WIN32
    for (auto it = 0; it < m_DataPath.length(); ++it) {
        if (m_DataPath[it] == '/') {
            rv.push_back('\\');
        } else {
            rv.push_back(m_DataPath[it]);
        }
    }
#else
    for (auto it = 0; it < m_DataPath.length(); ++it) {
        if (m_DataPath[it] == '\\') {
            rv.push_back('/');
        } else {
            rv.push_back(m_DataPath[it]);
        }
    }
#endif
    return rv;
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
