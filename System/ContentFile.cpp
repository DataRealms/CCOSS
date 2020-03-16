#include "ContentFile.h"
#include "PresetMan.h"
#include "AudioMan.h"

namespace RTE {
	const std::string ContentFile::m_ClassName = "ContentFile";

	std::map<std::string, BITMAP *> ContentFile::m_sLoadedBitmaps[BitDepthCount];
	std::map<std::string, FMOD::Sound *> ContentFile::m_sLoadedSamples;
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
			for (std::map<std::string, BITMAP *>::iterator lbItr = m_sLoadedBitmaps[depth].begin(); lbItr != m_sLoadedBitmaps[depth].end(); ++lbItr) {
				destroy_bitmap((*lbItr).second);
			}
		}
		for (std::map<std::string, FMOD::Sound *>::iterator lcItr = m_sLoadedSamples.begin(); lcItr != m_sLoadedSamples.end(); ++lcItr) {
			(*lcItr).second->release();
		}
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

	void ContentFile::SetDataPath(std::string newDataPath) {
		m_DataPath = newDataPath;
		m_PathHashes[GetHash()] = m_DataPath;

		// Reset the loaded convenience pointer
		//m_pLoadedData = 0;
		//m_LoadedDataSize = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}

		BITMAP *pReturnBitmap = 0;

		// Determine the bit depth this bitmap will be loaded as
		int bitDepth = conversionMode == COLORCONV_8_TO_32 ? ThirtyTwo : Eight;

		// Check if the file has already been read and loaded from the disk and, if so, use that data. Otherwise, load it
		std::map<std::string, BITMAP *>::iterator itr = m_sLoadedBitmaps[bitDepth].find(m_DataPath);
		if (itr != m_sLoadedBitmaps[bitDepth].end()) {
			pReturnBitmap = (*itr).second;
		} else {
			pReturnBitmap = LoadAndReleaseBitmap(conversionMode); //NOTE: This takes ownership of the bitmap file
			RTEAssert(pReturnBitmap, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

			// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			m_sLoadedBitmaps[bitDepth].insert(pair<std::string, BITMAP *>(m_DataPath, pReturnBitmap));
		}

		return pReturnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP *ContentFile::LoadAndReleaseBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}
		BITMAP *pReturnBitmap = 0;

		set_color_conversion(conversionMode == 0 ? COLORCONV_MOST : conversionMode);

		int separatorPos = m_DataPath.find('#'); // Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used

		if (separatorPos == m_DataPath.length()) {
			RTEAbort("There was no object name following first pound sign in the ContentFile's datafile path, which means there was no actual object defined. The path was:\n\n" + m_DataPath);
		} else if (separatorPos == -1) {
			PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);
			// If the file didn't open, try using animation naming scheme of adding 000 before the extension
			if (!pFile) {
				int extensionPos = m_DataPath.rfind('.');
				RTEAssert(extensionPos > 0, "Could not find file extension when trying to load and release bitmap with path and name:\n\n" + m_DataPath);
				std::string pathWithoutExtension = m_DataPath;
				pathWithoutExtension.resize(extensionPos);

				pFile = pack_fopen((pathWithoutExtension + "000.bmp").c_str(), F_READ);
				RTEAssert(pFile, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);
			}

			// Load the bitmap then close the filestream to clean up
			PALETTE currentPalette;
			get_palette(currentPalette);

			pReturnBitmap = load_bmp_pf(pFile, (RGB *)currentPalette);
			pack_fclose(pFile);
		} else if (separatorPos != m_DataPath.length() - 1) {
			// Split the datapath into the path and the object name and load the datafile from them
			m_pDataFile = load_datafile_object(m_DataPath.substr(0, separatorPos).c_str(), m_DataPath.substr(separatorPos + 1).c_str());
			RTEAssert(m_pDataFile && m_pDataFile->dat && m_pDataFile->type == DAT_BITMAP, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

			pReturnBitmap = (BITMAP *)m_pDataFile->dat;
		}
		RTEAssert(pReturnBitmap, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

		return pReturnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP ** ContentFile::GetAsAnimation(int frameCount, int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}

		// Create the array of as many BITMAP pointers as requested frames
		BITMAP **aReturnBitmaps = new BITMAP *[frameCount];

		// Don't try to append numbers if there's only one frame
		if (frameCount == 1) {
			aReturnBitmaps[0] = GetAsBitmap(conversionMode);
			return aReturnBitmaps;
		}

		std::string extension = "";
		int separatorPos = m_DataPath.find('#'); // Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used
		int extensionPos = 0;

		// No separator, need to separate file extension from datapath
		if (separatorPos == -1) {
			extensionPos = m_DataPath.rfind('.');
			RTEAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps with path and name:\n\n" + m_DataPath);
			extension.assign(m_DataPath, extensionPos, m_DataPath.length() - extensionPos);
			m_DataPath.resize(extensionPos);
		}

		std::string originalDataPath = m_DataPath;
		char framePath[1024];
		// For each frame in the animation, temporarily assign it to the datapath member var so that GetAsBitmap and then load it with GetBitmap
		for (int i = 0; i < frameCount; i++) {
			sprintf_s(framePath, sizeof(framePath), "%s%03i%s", originalDataPath.c_str(), i, extension.c_str());
			m_DataPath = framePath;

			aReturnBitmaps[i] = GetAsBitmap(conversionMode);
			RTEAssert(aReturnBitmaps[i], "Could not get a frame of animation with path and name:\n\n" + m_DataPath);
		}
		m_DataPath = originalDataPath + (extensionPos > 0 ? extension : "");

		return aReturnBitmaps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSample() {
		if (m_DataPath.empty()) {
			return 0;
		}
		FMOD::Sound *pReturnSample = 0;

		// Check if the file has already been read and loaded from the disk and, if so, use that data. Otherwise, load it
		std::map<std::string, FMOD::Sound *>::iterator itr = m_sLoadedSamples.find(m_DataPath);
		if (itr != m_sLoadedSamples.end()) {
			pReturnSample = (*itr).second;
		} else {
			int separatorPos = m_DataPath.rfind('#'); // Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used
			long fileSize;
			char *pRawData = 0;

			if (separatorPos == m_DataPath.length()) {
				RTEAbort("There was no object name following first pound sign in the ContentFile's datafile path, which means there was no actual object defined. The path was:\n\n" + m_DataPath);
			} else if (separatorPos == -1) {
				// Open the file, allocate space for it, read it and load it in as a Sound object
				fileSize = file_size(m_DataPath.c_str());
				PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);
				RTEAssert(pFile && fileSize > 0, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

				pRawData = new char[fileSize];
				int bytesRead = pack_fread(pRawData, fileSize, pFile);
				RTEAssert(bytesRead == fileSize, "Tried to read a file but couldn't read the same amount of data as the reported file size!");

				// Setup fmod info, and make sure to use mode OPENMEMORY_POINT since we're doing the loading ContentFile instead of fmod
				FMOD_CREATESOUNDEXINFO soundInfo = {}; 
				soundInfo.cbsize = fileSize;
				//TODO Consider doing FMOD_CREATESAMPLE for dumping audio files into memory and FMOD_NONBLOCKING to async create sounds
				FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(pRawData, FMOD_OPENMEMORY_POINT, &soundInfo, &pReturnSample);

				RTEAssert(result == FMOD_OK, "Unable to create sound " + m_DataPath);

				// Deallocate the intermediary data and close the file stream
				delete[] pRawData;
				pack_fclose(pFile);
			} else if (separatorPos != m_DataPath.length() - 1) {
				RTEAbort("Loading sound samples from allegro datafiles isn't supported yet!");
				/*
				// Split the datapath into the path and the object name and load the datafile from them
				m_pDataFile = load_datafile_object(m_DataPath.substr(0, separatorPos).c_str(), m_DataPath.substr(separatorPos + 1).c_str());
				RTEAssert(m_pDataFile && m_pDataFile->dat && m_pDataFile->type == DAT_BITMAP, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

				pReturnSample = (FMOD::Sound *)m_pDataFile->dat;
				*/
			}

			RTEAssert(pReturnSample, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			m_sLoadedSamples.insert(std::pair<std::string, FMOD::Sound *>(m_DataPath, pReturnSample));
		}
		return pReturnSample;
	}
}