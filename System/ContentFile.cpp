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

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::FreeAllLoaded() {
		for (int depth = BitDepths::Eight; depth < BitDepths::BitDepthCount; ++depth) {
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

	int ContentFile::GetDataModuleID() const { return (m_DataModuleID < 0) ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		BITMAP *returnBitmap = nullptr;
		set_color_conversion((conversionMode == 0) ? COLORCONV_MOST : conversionMode);

		int extensionPos = m_DataPath.rfind('.');
		RTEAssert(extensionPos > 0, "Could not find file extension when trying to find image file with path and name:\n\n" + m_DataPath);

		std::string pathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - 4);
		std::string fileExtension = std::experimental::filesystem::path(m_DataPath).extension().string();
		std::string altFileExtension = (fileExtension == ".png") ? ".bmp" : ".png";

		if (!std::experimental::filesystem::exists(m_DataPath)) {
			if (std::experimental::filesystem::exists(pathWithoutExtension + "000" + fileExtension)) {
				SetDataPath(pathWithoutExtension + "000" + fileExtension);
			} else {
				if (std::experimental::filesystem::exists(pathWithoutExtension + altFileExtension)) {
					SetDataPath(pathWithoutExtension + altFileExtension);
				} else if (std::experimental::filesystem::exists(pathWithoutExtension + "000" + altFileExtension)) {
					SetDataPath(pathWithoutExtension + "000" + altFileExtension);
				} else {
					RTEAbort("Failed to find image file with following path and name:\n\n" + m_DataPath + "\nor\n" + pathWithoutExtension + altFileExtension + "\n\nLoading has been aborted!");
				}
			}
		}
		PALETTE currentPalette;
		get_palette(currentPalette);

		returnBitmap = load_bitmap(m_DataPath.c_str(), currentPalette);
		RTEAssert(returnBitmap, "Failed to load image file with following path and name:\n\n" + m_DataPath + "\nThe image file may be corrupt, incorrectly converted or saved with unsupported parameters.");
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		BITMAP *returnBitmap = nullptr;

		int bitDepth = (conversionMode == COLORCONV_8_TO_32) ? BitDepths::ThirtyTwo : BitDepths::Eight;

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::map<std::string, BITMAP *>::iterator foundBitmap = s_LoadedBitmaps[bitDepth].find(m_DataPath);
		if (foundBitmap != s_LoadedBitmaps[bitDepth].end()) {
			returnBitmap = (*foundBitmap).second;
		} else {
			returnBitmap = LoadAndReleaseBitmap(conversionMode); //NOTE: This takes ownership of the bitmap file

			// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			s_LoadedBitmaps[bitDepth].insert(std::pair<std::string, BITMAP *>(m_DataPath, returnBitmap));
		}
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP ** ContentFile::GetAsAnimation(int frameCount, int conversionMode) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		// Create the array of as many BITMAP pointers as requested frames
		BITMAP **returnBitmaps = new BITMAP *[frameCount];

		// Don't try to append numbers if there's only one frame
		if (frameCount == 1) {
			returnBitmaps[0] = GetAsBitmap(conversionMode);
			return returnBitmaps;
		}

		int extensionPos = m_DataPath.rfind('.');
		RTEAssert(extensionPos > 0, "Could not find file extension when trying to load an animation from external bitmaps with path and name:\n\n" + m_DataPath);
		std::string fileExtension = std::experimental::filesystem::path(m_DataPath).extension().string();
		std::string pathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - 4);
		
		char framePath[1024];
		// For each frame in the animation, temporarily assign it to the datapath member var so that GetAsBitmap and then load it with GetBitmap
		for (int frameNum = 0; frameNum < frameCount; frameNum++) {
			sprintf_s(framePath, sizeof(framePath), "%s%03i%s", pathWithoutExtension.c_str(), frameNum, fileExtension.c_str());
			m_DataPath = framePath;
			returnBitmaps[frameNum] = GetAsBitmap(conversionMode);
		}
		m_DataPath = pathWithoutExtension + fileExtension;
		return returnBitmaps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSample(bool abortGameForInvalidSound) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::map<std::string, FMOD::Sound *>::iterator foundSound = s_LoadedSamples.find(m_DataPath);
		if (foundSound != s_LoadedSamples.end()) {
			returnSample = (*foundSound).second;
		} else {
			std::string errorMessage;
			unsigned int fileSize = static_cast<unsigned int>(std::experimental::filesystem::file_size(m_DataPath));
			PACKFILE *fileToLoad = pack_fopen(m_DataPath.c_str(), F_READ);
			
			if (!fileToLoad || fileSize <= 0) {
				errorMessage = "ERROR: Failed to load sound file with following path and name: ";
				if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
				g_ConsoleMan.PrintString(errorMessage + m_DataPath);
				return returnSample;
			}
			char *rawData = new char[fileSize];
			int bytesRead = pack_fread(rawData, fileSize, fileToLoad);
			RTEAssert(bytesRead == fileSize, "Tried to read a sound file but couldn't read the same amount of data as the reported file size! The path and name were: \n\n" +m_DataPath);
			pack_fclose(fileToLoad);

			// Setup fmod info, and make sure to use mode OPENMEMORY since we're doing the loading with ContentFile instead of fmod, and we're deleting the raw data after loading it
			FMOD_CREATESOUNDEXINFO soundInfo = {};
			soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
			soundInfo.length = fileSize;
			//TODO Consider doing FMOD_CREATESAMPLE for dumping audio files into memory and FMOD_NONBLOCKING to async create sounds
			FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(rawData, FMOD_OPENMEMORY | FMOD_3D, &soundInfo, &returnSample);
			delete[] rawData;

			if (result != FMOD_OK) {
				errorMessage = "ERROR: Unable to create sound because of FMOD error " + std::string(FMOD_ErrorString(result)) + ". Path and name was: ";
				if (abortGameForInvalidSound) { RTEAbort(errorMessage + "\n\n" + m_DataPath); }
				g_ConsoleMan.PrintString(errorMessage + m_DataPath);
				return returnSample;
			}
			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED FILE
			s_LoadedSamples.insert(std::pair<std::string, FMOD::Sound *>(m_DataPath, returnSample));
		}
		return returnSample;
	}
}