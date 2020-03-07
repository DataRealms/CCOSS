#include "ContentFile.h"
#include "PresetMan.h"

namespace RTE {

	const std::string ContentFile::m_ClassName = "ContentFile";

	std::map<std::string, BITMAP *> ContentFile::m_sLoadedBitmaps[BitDepthCount];
	std::map<std::string, AUDIO_STRUCT *> ContentFile::m_sLoadedSamples;
	std::map<size_t, std::string> ContentFile::m_PathHashes;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::Clear() {
		m_DataPath.erase();
		m_DataModuleID = 0;
		//m_DataModified = false;
		//m_pLoadedData = 0;
		//m_LoadedDataSize = 0;
		m_pDataFile = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const char *filePath) {
		m_DataPath = filePath;
		m_PathHashes[GetHash()] = m_DataPath;

		m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const ContentFile &reference) {
		m_DataPath = reference.m_DataPath;

		m_DataModuleID = reference.m_DataModuleID;
		//m_DataModified = reference.m_DataModified;
		//m_pLoadedData = reference.m_pLoadedData;
		//m_LoadedDataSize = reference.m_LoadedDataSize;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::FreeAllLoaded() {
		for (int depth = Eight; depth < BitDepthCount; ++depth) {
			for (std::map<std::string, BITMAP *>::iterator lbItr = m_sLoadedBitmaps[depth].begin(); lbItr != m_sLoadedBitmaps[depth].end(); ++lbItr) { destroy_bitmap((*lbItr).second); }
		}
#ifdef __USE_SOUND_FMOD
		for (std::map<std::string, FSOUND_SAMPLE *>::iterator lcItr = m_sLoadedSamples.begin(); lcItr != m_sLoadedSamples.end(); ++lcItr) { FSOUND_Sample_Free((*lcItr).second); }
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "Path" || propName == "FilePath") {
			m_DataPath = reader.ReadPropValue();
			m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
			m_PathHashes[GetHash()] = m_DataPath;
		} else {
			// See if the base class(es) can find a match instead
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Save(Writer &writer) const {
		Serializable::Save(writer);

		if (!m_DataPath.empty()) {
			writer.NewProperty("FilePath");
			writer << m_DataPath;
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::GetDataModuleID() { return m_DataModuleID < 0 ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	unsigned long ContentFile::GetDataSize() {
		if (!m_pLoadedData) { GetContent(); }
		// Now that we have a data, return the size.
		return m_LoadedDataSize;
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	int ContentFile::GetDataType() {
		if (!m_pDataFile) { GetContent(); }
		// Now that we have a datafile, return the Allegro type.
		return m_pDataFile->type;
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetDataPath(std::string newDataPath) {
		m_DataPath = newDataPath;
		m_PathHashes[GetHash()] = m_DataPath;

		// Reset the loaded convenience pointer
		//m_pLoadedData = 0;
		//m_LoadedDataSize = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string ContentFile::GetPathFromHash(size_t hash) {
		std::string result;
		if (m_PathHashes.count(hash) == 1) { result = m_PathHashes[hash]; }
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode) {
		if (m_DataPath.empty()) { return 0; }
		BITMAP *pReturnBitmap = 0;

		// Determine the bit depth this bitmap will be loaded as
		int bitDepth = conversionMode == COLORCONV_8_TO_32 ? ThirtyTwo : Eight;

		// Check if this file has already been read and loaded from disk.
		std::map<std::string, BITMAP *>::iterator itr = m_sLoadedBitmaps[bitDepth].find(m_DataPath);
		if (itr != m_sLoadedBitmaps[bitDepth].end()) {
			// Yes, has been loaded previously, then use that data from memory.
			pReturnBitmap = (*itr).second;
		} else {
			// Hasn't been loaded previously, so go ahead and do so now.
			// Load the BITMAP from file and take ownership of it
			pReturnBitmap = LoadAndReleaseBitmap(conversionMode);

			if (!pReturnBitmap) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

			// Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			m_sLoadedBitmaps[bitDepth].insert(pair<std::string, BITMAP *>(m_DataPath, pReturnBitmap));
		}
		// Return without transferring ownership
		return pReturnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP ** ContentFile::GetAsAnimation(int frameCount, int conversionMode) {
		if (m_DataPath.empty()) { return 0; }

		// Create the array of as many BITMAP pointers as requested frames
		BITMAP **aReturnBitmaps = new BITMAP *[frameCount];

		// Don't mess with numbers if there's only one frame
		if (frameCount == 1) {
			aReturnBitmaps[0] = GetAsBitmap(conversionMode);
			return aReturnBitmaps;
		}

		// The file extension string, if any.
		std::string extension = "";

		// Find where the '#' denoting the divider between the datafile and the datafile object's name is
		int separatorPos = m_DataPath.find('#');

		// Position of the dot in the file extension, if any.
		int extensionPos = 0;

		// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
		// This also means we need to handle file extensions
		if (separatorPos == -1) {
			// Find the filename extension
			extensionPos = m_DataPath.rfind('.');
			RTEAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps!");

			// Save our extension from the datapath
			extension.assign(m_DataPath, extensionPos, m_DataPath.length() - extensionPos);
			// Truncate the datapath
			m_DataPath.resize(extensionPos);
		}

		std::string originalDataPath = m_DataPath;
		char framePath[1024];
		for (int i = 0; i < frameCount; i++) {
			// Create the temporary frame datapath
			sprintf_s(framePath, sizeof(framePath), "%s%03i%s", originalDataPath.c_str(), i, extension.c_str());
			// Temporarily assign it to the datapath member var so that GetAsBitmap uses it
			m_DataPath = framePath;
			// Get the frame bitmap
			aReturnBitmaps[i] = GetAsBitmap(conversionMode);
			RTEAssert(aReturnBitmaps[i], "Could not get a frame of animation");
		}

		// Set the data path back to its original state (without any numbers after it)
		m_DataPath = originalDataPath;

		// Append back the extension, if there was any
		if (separatorPos > 0 || extensionPos > 0) { m_DataPath += extension; }

		// Return the loaded BITMAPs
		return aReturnBitmaps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AUDIO_STRUCT * ContentFile::GetAsSample() {

		if (m_DataPath.empty()) { return 0; }
		AUDIO_STRUCT *pReturnSample = 0;

		// Check if this file has already been read and loaded from disk.
		std::map<std::string, AUDIO_STRUCT *>::iterator itr = m_sLoadedSamples.find(m_DataPath);
		if (itr != m_sLoadedSamples.end()) {
			// Yes, has been loaded previously, then use that data from memory.
			pReturnSample = (*itr).second;
			// Hasn't been loaded previously, so go ahead and do so now.
		} else {
			// Find where the '#' denoting the divider between the datafile and the datafile object's name is
			int separatorPos = m_DataPath.rfind('#');

			// Size of the entire file to be read
			long fileSize;
			// Holder of the raw data read from the pack file
			char *pRawData = 0;

			// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
			if (separatorPos == -1) {
				fileSize = file_size(m_DataPath.c_str());
				PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);

				// Make sure we opened properly.
				if (!pFile || fileSize <= 0) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

				// Allocate the raw data space in memory
				pRawData = new char[fileSize];

				// Read the raw data from the file
				int bytesRead = pack_fread(pRawData, fileSize, pFile);
				RTEAssert(bytesRead == fileSize, "Tried to read a file but couldn't read the same amount of data as the reported file size!");

				// Load the sample from the memory we've read from the file.
#ifdef __USE_SOUND_FMOD
				// FSOUND_UNMANAGED because we want to manage the freeing of the sample ourselves.
				pReturnSample = FSOUND_Sample_Load(FSOUND_UNMANAGED, pRawData, FSOUND_LOADMEMORY, 0, fileSize);

#elif __USE_SOUND_GORILLA
				ga_Memory * mem = ga_memory_create(pRawData, fileSize);
				ga_DataSource * data = gau_data_source_create_memory(mem);
				ga_SampleSource * samples = gau_sample_source_create_wav(data);
				pReturnSample = ga_sound_create_sample_source(samples);
#endif
				if (pReturnSample == 0) { RTEAbort(("Unable to create sound " + m_DataPath).c_str()); }

				// Deallocate the intermediary data
				delete[] pRawData;

				// Close the file stream
				pack_fclose(pFile);

				// If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
			} else if (separatorPos != m_DataPath.length() - 1) {
				RTEAbort("Loading sound samples from allegro datafiles isn't supported yet!");
				// TODO loading SDL_mixer chunks from allegro datafiles! this!
				/*
				// Get the Path only, without the object name, using the separator index as length limiter
				std::string datafilePath = m_DataPath.substr(0, separatorPos);
				// Adjusting to the true first character of the datafile object's name string.
				std::string objectName = m_DataPath.substr(separatorPos + 1);

				// Try loading the datafile from the specified path + object names.
				m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

				// Make sure we loaded properly.
				if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_SAMPLE) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

				// Get the loaded data
				pReturnSample = (FSOUND_SAMPLE *)m_pDataFile->dat;
				*/
			} else {
				RTEAbort("No object name following first #-sign in ContentFile's datafile object path!");
			}

			if (!pReturnSample) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

			// Now when loaded for the first time, enter into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			m_sLoadedSamples.insert(pair<std::string, AUDIO_STRUCT *>(m_DataPath, pReturnSample));
		}
		return pReturnSample;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode) {
		if (m_DataPath.empty()) { return 0; }
		BITMAP *pReturnBitmap = 0;

		// Set the conversion mode, default to COLORCONV_MOST if none was set
		set_color_conversion(conversionMode == 0 ? COLORCONV_MOST : conversionMode);

		// Find where the '#' denoting the divider between the datafile and the datafile object's name is
		int separatorPos = m_DataPath.rfind('#');

		// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
		if (separatorPos == -1) {
			PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);
			// Make sure we opened properly, or try to add 000 before the extension if it's part of an animation naming
			if (!pFile) {
				// Truncate away the extension
				int extensionPos = m_DataPath.rfind('.');
				m_DataPath.resize(extensionPos);

				pFile = pack_fopen((m_DataPath + "000.bmp").c_str(), F_READ);
				if (!pFile) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

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
		
		// If we found a pound sign, make sure it's not on the very end. If not, then go ahead and load from packed stream.
		} else if (separatorPos != m_DataPath.length() - 1) {
			// Get the Path only, without the object name, using the separator index as length limiter
			std::string datafilePath = m_DataPath.substr(0, separatorPos);
			// Adjusting to the true first character of the datafile object's name string.
			std::string objectName = m_DataPath.substr(separatorPos + 1);

			// Try loading the datafile from the specified path + object names.
			m_pDataFile = load_datafile_object(datafilePath.c_str(), objectName.c_str());

			// Make sure we loaded properly.
			if (!m_pDataFile || !m_pDataFile->dat || m_pDataFile->type != DAT_BITMAP) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

			// Get the loaded data
			pReturnBitmap = (BITMAP *)m_pDataFile->dat;
		} else {
			RTEAbort("No object name following first #-sign in ContentFile's datafile object path!");
		}
		if (!pReturnBitmap) { RTEAbort(("Failed to load datafile object with following path and name:\n\n" + m_DataPath).c_str()); }

		return pReturnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP ** ContentFile::LoadAndReleaseAnimation(int frameCount, int conversionMode) {
		if (m_DataPath.empty()) { return 0; }

		// Create the array of as many BITMAP pointers as requested frames
		BITMAP **aReturnBitmaps = new BITMAP *[frameCount];

		// Don't mess with numbers if there's only one frame
		if (frameCount == 1) {
			aReturnBitmaps[0] = LoadAndReleaseBitmap(conversionMode);
			return aReturnBitmaps;
		}

		// The file extension string, if any.
		std::string extension = "";

		// Find where the '#' denoting the divider between the datafile and the datafile object's name is
		int separatorPos = m_DataPath.find('#');

		// Position of the dot in the file extension, if any.
		int extensionPos = 0;

		// If there is none, that means we're told to load an exposed file outside of a .dat datafile.
		// This also means we need to handle file extensions
		if (separatorPos == -1) {
			// Find the filename extension
			extensionPos = m_DataPath.rfind('.');
			RTEAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps!");

			// Save our extension from the datapath
			extension.assign(m_DataPath, extensionPos, m_DataPath.length() - extensionPos);
			// Truncate the datapath
			m_DataPath.resize(extensionPos);
		}

		std::string originalDataPath = m_DataPath;
		char framePath[1024];
		for (int i = 0; i < frameCount; i++) {
			// Create the temporary frame datapath
			sprintf_s(framePath, sizeof(framePath), "%s%03i%s", originalDataPath.c_str(), i, extension.c_str());
			// Temporarily assign it to the datapath member var so that GetAsBitmap uses it
			m_DataPath = framePath;
			// Get the frame bitmap
			aReturnBitmaps[i] = LoadAndReleaseBitmap(conversionMode);
			RTEAssert(aReturnBitmaps[i], "Could not get a frame of animation");
		}

		// Set the data path back to its original state (without any numbers after it)
		m_DataPath = originalDataPath;

		// Append back the extension, if there was any
		if (separatorPos > 0 || extensionPos > 0) { m_DataPath += extension; }

		// Return the loaded BITMAP:s
		return aReturnBitmaps;
	}
}