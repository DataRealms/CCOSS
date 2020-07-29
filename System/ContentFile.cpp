#include "ContentFile.h"
#include "AudioMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

namespace RTE {

	const std::string ContentFile::c_ClassName = "ContentFile";

	std::map<std::string, BITMAP *> ContentFile::s_LoadedBitmaps[BitDepthCount];
	std::map<std::string, FMOD::Sound *> ContentFile::s_LoadedSamples;
	std::map<size_t, std::string> ContentFile::s_PathHashes;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::Clear() {
		m_DataPath.erase();
		m_DataModuleID = 0;
		//m_DataModified = false;
		//m_LoadedData = 0;
		//m_LoadedDataSize = 0;
		//m_DataFile = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const char *filePath) {
		m_DataPath = filePath;
		s_PathHashes[GetHash()] = m_DataPath;
		m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const ContentFile &reference) {
		m_DataPath = reference.m_DataPath;
		m_DataModuleID = reference.m_DataModuleID;
		//m_DataModified = reference.m_DataModified;
		//m_LoadedData = reference.m_LoadedData;
		//m_LoadedDataSize = reference.m_LoadedDataSize;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::FreeAllLoaded() {
		for (int depth = Eight; depth < BitDepthCount; ++depth) {
			for (const std::pair<std::string, BITMAP *> &bitmap : s_LoadedBitmaps[depth]){
				destroy_bitmap(bitmap.second);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "Path" || propName == "FilePath") {
			m_DataPath = reader.ReadPropValue();
			m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
			s_PathHashes[GetHash()] = m_DataPath;
		} else {
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

	int ContentFile::GetDataModuleID() const { return (m_DataModuleID) < 0 ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetDataPath(std::string newDataPath) {
		m_DataPath = newDataPath;
		s_PathHashes[GetHash()] = m_DataPath;

		// Reset the loaded convenience pointer
		//m_LoadedData = 0;
		//m_LoadedDataSize = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}
		BITMAP *returnBitmap = 0;

		// Determine the bit depth this bitmap will be loaded as
		int bitDepth = conversionMode == COLORCONV_8_TO_32 ? ThirtyTwo : Eight;

		// Check if the file has already been read and loaded from the disk and, if so, use that data. Otherwise, load it
		std::map<std::string, BITMAP *>::iterator itr = s_LoadedBitmaps[bitDepth].find(m_DataPath);
		if (itr != s_LoadedBitmaps[bitDepth].end()) {
			returnBitmap = (*itr).second;
		} else {
			returnBitmap = LoadAndReleaseBitmap(conversionMode); //NOTE: This takes ownership of the bitmap file
			RTEAssert(returnBitmap, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

			// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			s_LoadedBitmaps[bitDepth].insert(std::pair<std::string, BITMAP *>(m_DataPath, returnBitmap));
		}
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP *ContentFile::LoadAndReleaseBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}
		BITMAP *returnBitmap = 0;
		set_color_conversion((conversionMode == 0) ? COLORCONV_MOST : conversionMode);

		// Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used
		int separatorPos = m_DataPath.find('#');

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
			// Load the bitmap then close the file stream to clean up
			PALETTE currentPalette;
			get_palette(currentPalette);

			returnBitmap = load_bmp_pf(pFile, (RGB *)currentPalette);
			pack_fclose(pFile);
		} else if (separatorPos != m_DataPath.length() - 1) {
			RTEAbort("Loading bitmaps from allegro datafiles isn't supported yet!");
			// Used for loading from DataFiles, disabled because we don't have this properly implemented right now. 
			/*
			// Split the datapath into the path and the object name and load the datafile from them
			m_DataFile = load_datafile_object(m_DataPath.substr(0, separatorPos).c_str(), m_DataPath.substr(separatorPos + 1).c_str());
			RTEAssert(m_DataFile && m_DataFile->dat && m_DataFile->type == DAT_BITMAP, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

			returnBitmap = (BITMAP *)m_DataFile->dat;
			*/
		}
		RTEAssert(returnBitmap, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP ** ContentFile::GetAsAnimation(int frameCount, int conversionMode) {
		if (m_DataPath.empty()) {
			return 0;
		}
		// Create the array of as many BITMAP pointers as requested frames
		BITMAP **returnBitmaps = new BITMAP *[frameCount];

		// Don't try to append numbers if there's only one frame
		if (frameCount == 1) {
			returnBitmaps[0] = GetAsBitmap(conversionMode);
			return returnBitmaps;
		}
		std::string extension = "";
		// Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used
		int separatorPos = m_DataPath.find('#');
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

			returnBitmaps[i] = GetAsBitmap(conversionMode);
			RTEAssert(returnBitmaps[i], "Could not get a frame of animation with path and name:\n\n" + m_DataPath);
		}
		m_DataPath = originalDataPath + (extensionPos > 0 ? extension : "");
		return returnBitmaps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSample(bool abortGameForInvalidSound) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return 0;
		}
		FMOD::Sound *returnSample = 0;
		std::string errorMessage;

		// Check if the file has already been read and loaded from the disk and, if so, use that data. Otherwise, load it
		std::map<std::string, FMOD::Sound *>::iterator itr = s_LoadedSamples.find(m_DataPath);
		if (itr != s_LoadedSamples.end()) {
			returnSample = (*itr).second;
		} else {
			int separatorPos = m_DataPath.rfind('#'); // Used for handling separators between the datafile name and the object name in .dat datafiles. NOTE: Not currently used
			unsigned long long fileSize;
			char *rawData = 0;

			if (separatorPos == m_DataPath.length()) {
				errorMessage = "There was no object name following first pound sign in the sound ContentFile's datafile path, which means there was no actual object defined. The path was: ";
				if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
				g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
				return returnSample;
			} else if (separatorPos == -1) {
				// Open the file, allocate space for it, read it and load it in as a Sound object
				fileSize = file_size_ex(m_DataPath.c_str());
				PACKFILE *pFile = pack_fopen(m_DataPath.c_str(), F_READ);

				if (!pFile || fileSize <= 0) {
					errorMessage = "Failed to load sound file with following path and name: ";
					if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
					g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
					return returnSample;
				}

				rawData = new char[fileSize];
				int bytesRead = pack_fread(rawData, fileSize, pFile);
				RTEAssert(bytesRead == fileSize, "Tried to read a sound file but couldn't read the same amount of data as the reported file size! The path and name were: \n\n" +m_DataPath);

				// Setup fmod info, and make sure to use mode OPENMEMORY since we're doing the loading with ContentFile instead of fmod, and we're deleting the raw data after loading it
				FMOD_CREATESOUNDEXINFO soundInfo = {};
				soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
				soundInfo.length = fileSize;
				//TODO Consider doing FMOD_CREATESAMPLE for dumping audio files into memory and FMOD_NONBLOCKING to async create sounds
				FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(rawData, FMOD_OPENMEMORY | FMOD_3D, &soundInfo, &returnSample);

				if (result != FMOD_OK) {
					errorMessage = "Unable to create sound because of FMOD error " + std::string(FMOD_ErrorString(result)) + ". Path and name was: ";
					if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
					g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
					return returnSample;
				}

				// Deallocate the intermediary data and close the file stream
				delete[] rawData;
				pack_fclose(pFile);
			} else if (separatorPos != m_DataPath.length() - 1) {
				RTEAbort("Loading sounds from allegro datafiles isn't supported yet!");
				/*
				// Split the datapath into the path and the object name and load the datafile from them
				m_DataFile = load_datafile_object(m_DataPath.substr(0, separatorPos).c_str(), m_DataPath.substr(separatorPos + 1).c_str());
				RTEAssert(m_DataFile && m_DataFile->dat && m_DataFile->type == DAT_BITMAP, "Failed to load datafile object with following path and name:\n\n" + m_DataPath);

				returnSample = (FMOD::Sound *)m_DataFile->dat;
				*/
			}
			if (!returnSample) {
				errorMessage = "Failed to load sound file with following path and name:";
				if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
				g_ConsoleMan.PrintString("Error: " + errorMessage + m_DataPath);
				return returnSample;
			}

			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			s_LoadedSamples.insert(std::pair<std::string, FMOD::Sound *>(m_DataPath, returnSample));
		}
		return returnSample;
	}
}