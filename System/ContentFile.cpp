#include "ContentFile.h"
#include "AudioMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

namespace RTE {

	const std::string ContentFile::c_ClassName = "ContentFile";

	std::unordered_map<std::string, BITMAP *> ContentFile::s_LoadedBitmaps[BitDepthCount];
	std::unordered_map<std::string, FMOD::Sound *> ContentFile::s_LoadedSamples;
	std::unordered_map<size_t, std::string> ContentFile::s_PathHashes;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::Clear() {
		m_DataPath.clear();
		m_DataPathExtension.clear();
		m_DataPathWithoutExtension.clear();
		m_FormattedReaderPosition.clear();
		m_DataPathAndReaderPosition.clear();
		m_DataModuleID = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const char *filePath) {
		SetDataPath(filePath);
		SetFormattedReaderPosition(GetFormattedReaderPosition());

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const ContentFile &reference) {
		m_DataPath = reference.m_DataPath;
		m_DataPathExtension = reference.m_DataPathExtension;
		m_DataPathWithoutExtension = reference.m_DataPathWithoutExtension;
		m_DataModuleID = reference.m_DataModuleID;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::FreeAllLoaded() {
		for (int depth = BitDepths::Eight; depth < BitDepths::BitDepthCount; ++depth) {
			for (const std::pair<std::string, BITMAP *> &bitmap : s_LoadedBitmaps[depth]) {
				destroy_bitmap(bitmap.second);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "FilePath" || propName == "Path") {
			SetDataPath(reader.ReadPropValue());
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

	void ContentFile::SetDataPath(const std::string &newDataPath) {
		m_DataPath = newDataPath;
		m_DataPathExtension = std::filesystem::path(newDataPath).extension().string();

		RTEAssert(!m_DataPathExtension.empty(), "Failed to find file extension when trying to find file with path and name:\n\n" + m_DataPath + "\n" + GetFormattedReaderPosition());

		m_DataPathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - m_DataPathExtension.length());
		s_PathHashes[GetHash()] = m_DataPath;
		m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetFormattedReaderPosition(const std::string &newPosition) {
		m_FormattedReaderPosition = newPosition;
		m_DataPathAndReaderPosition = m_DataPath + "\n" + newPosition;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		BITMAP *returnBitmap = nullptr;
		int bitDepth = (conversionMode == COLORCONV_8_TO_32) ? BitDepths::ThirtyTwo : BitDepths::Eight;

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::unordered_map<std::string, BITMAP *>::iterator foundBitmap = s_LoadedBitmaps[bitDepth].find(m_DataPath);
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
		char framePath[1024];
		// For each frame in the animation, temporarily assign it to the datapath member var so that GetAsBitmap and then load it with GetBitmap
		for (int frameNum = 0; frameNum < frameCount; frameNum++) {
			sprintf_s(framePath, sizeof(framePath), "%s%03i%s", m_DataPathWithoutExtension.c_str(), frameNum, m_DataPathExtension.c_str());
			m_DataPath = framePath;
			returnBitmaps[frameNum] = GetAsBitmap(conversionMode);
		}
		m_DataPath = m_DataPathWithoutExtension + m_DataPathExtension;
		return returnBitmaps;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		SetFormattedReaderPosition(GetFormattedReaderPosition());
		const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

		if (!std::filesystem::exists(m_DataPath)) {
			// Check for 000 in the file name in case it is part of an animation but the FrameCount was set to 1. Do not warn about this because it's normal operation.
			if (std::filesystem::exists(m_DataPathWithoutExtension + "000" + m_DataPathExtension)) {
				SetDataPath(m_DataPathWithoutExtension + "000" + m_DataPathExtension);
			} else {
				if (std::filesystem::exists(m_DataPathWithoutExtension + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + altFileExtension);
				} else if (std::filesystem::exists(m_DataPathWithoutExtension + "000" + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + "000" + altFileExtension);
				} else {
					RTEAbort("Failed to find image file with following path and name:\n\n" + m_DataPath + " or " + altFileExtension + "\n" + m_FormattedReaderPosition);
				}
			}
		}
		BITMAP *returnBitmap = nullptr;

		PALETTE currentPalette;
		get_palette(currentPalette);

		set_color_conversion((conversionMode == 0) ? COLORCONV_MOST : conversionMode);
		returnBitmap = load_bitmap(m_DataPath.c_str(), currentPalette);
		RTEAssert(returnBitmap, "Failed to load image file with following path and name:\n\n" + m_DataPathAndReaderPosition + "\nThe file may be corrupt, incorrectly converted or saved with unsupported parameters.");
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSample(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::unordered_map<std::string, FMOD::Sound *>::iterator foundSound = s_LoadedSamples.find(m_DataPath);
		if (foundSound != s_LoadedSamples.end()) {
			returnSample = (*foundSound).second;
		} else {
			returnSample = LoadAndReleaseSample(abortGameForInvalidSound, asyncLoading); //NOTE: This takes ownership of the sample file

			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED FILE
			s_LoadedSamples.insert(std::pair<std::string, FMOD::Sound *>(m_DataPath, returnSample));
		}
		return returnSample;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::LoadAndReleaseSample(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}
		const std::string altFileExtension = (m_DataPathExtension == ".wav") ? ".ogg" : ".wav";

		if (!std::filesystem::exists(m_DataPath)) {
			if (std::filesystem::exists(m_DataPathWithoutExtension + altFileExtension)) {
				g_ConsoleMan.AddLoadWarningLogEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
				SetDataPath(m_DataPathWithoutExtension + altFileExtension);
			} else {
				if (abortGameForInvalidSound) {
					RTEAbort("Failed to find audio file with following path and name:\n\n" + m_DataPath + " or " + altFileExtension + "\n" + m_FormattedReaderPosition);
				} else {
					g_ConsoleMan.PrintString("Failed to find audio file with following path and name:\n\n" + m_DataPath + " or " + altFileExtension + ". The file was not loaded!");
					return nullptr;
				}
			}
		}
		FMOD::Sound *returnSample = nullptr;

		FMOD_MODE fmodFlags = FMOD_CREATESAMPLE | FMOD_3D | (asyncLoading ? FMOD_NONBLOCKING : FMOD_DEFAULT);
		FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(m_DataPath.c_str(), fmodFlags, nullptr, &returnSample);

		if (result != FMOD_OK) {
			const std::string errorMessage = "Failed to create sound because of FMOD error: " + std::string(FMOD_ErrorString(result)) + "\nThe path and name were: ";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
			g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
			return returnSample;
		}
		return returnSample;
	}
}