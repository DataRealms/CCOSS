#include "ContentFile.h"
#include "AudioMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

#include "png.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "boost/functional/hash.hpp"

namespace RTE {

	const std::string ContentFile::c_ClassName = "ContentFile";

	std::array<std::unordered_map<std::string, BITMAP *>, ContentFile::BitDepths::BitDepthCount> ContentFile::s_LoadedBitmaps;
	std::unordered_map<std::string, FMOD::Sound *> ContentFile::s_LoadedSamples;
	std::unordered_map<size_t, std::string> ContentFile::s_PathHashes;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::Clear() {
		m_DataPath.clear();
		m_DataPathExtension.clear();
		m_DataPathWithoutExtension.clear();
		m_DataPathIsImageFile = false;
		m_FormattedReaderPosition.clear();
		m_DataPathAndReaderPosition.clear();
		m_DataModuleID = 0;

		m_ImageFileInfo.fill(-1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::Create(const char *filePath) {
		SetDataPath(filePath);

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
			for (const auto &[bitmapPath, bitmapPtr] : s_LoadedBitmaps[depth]) {
				destroy_bitmap(bitmapPtr);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::ReadProperty(const std::string_view &propName, Reader &reader) {
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

		if (!m_DataPath.empty()) { writer.NewPropertyWithValue("FilePath", m_DataPath); }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::GetDataModuleID() const {
		return (m_DataModuleID < 0) ? g_PresetMan.GetModuleIDFromPath(m_DataPath) : m_DataModuleID;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetDataPath(const std::string &newDataPath) {
		m_DataPath = CorrectBackslashesInPath(newDataPath);
		m_DataPathExtension = std::filesystem::path(m_DataPath).extension().string();

		RTEAssert(!m_DataPathExtension.empty(), "Failed to find file extension when trying to find file with path and name:\n" + m_DataPath + "\n" + GetFormattedReaderPosition());

		m_DataPathIsImageFile = m_DataPathExtension == ".png" || m_DataPathExtension == ".bmp";

		m_DataPathWithoutExtension = m_DataPath.substr(0, m_DataPath.length() - m_DataPathExtension.length());
		s_PathHashes[GetHash()] = m_DataPath;
		m_DataModuleID = g_PresetMan.GetModuleIDFromPath(m_DataPath);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t ContentFile::GetHash() const {
		// Use boost::hash for compiler independent hashing.
		return boost::hash<std::string>()(m_DataPath);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::SetFormattedReaderPosition(const std::string &newPosition) {
		m_FormattedReaderPosition = newPosition;
		m_DataPathAndReaderPosition = m_DataPath + "\n" + newPosition;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ContentFile::GetImageFileInfo(ImageFileInfoType infoTypeToGet) {
		bool fetchFileInfo = false;
		for (const int &fileInfoEntry : m_ImageFileInfo) {
			if (fileInfoEntry == -1) {
				fetchFileInfo = true;
				break;
			}
		}
		if (fetchFileInfo) {
			FILE *imageFile = fopen(m_DataPath.c_str(), "rb");
			RTEAssert(imageFile, "Failed to open file prior to reading info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may not exist or be corrupt.");

			if (m_DataPathExtension == ".png") {
				ReadAndStorePNGFileInfo(imageFile);
			} else if (m_DataPathExtension == ".bmp") {
				ReadAndStoreBMPFileInfo(imageFile);
			} else {
				RTEAbort("Somehow ended up attempting to read image file info for an unsupported image file type.\nThe file path and name were:\n\n" + m_DataPath);
			}
			fclose(imageFile);
		}
		return m_ImageFileInfo[infoTypeToGet];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::ReadAndStorePNGFileInfo(FILE *imageFile) {
		std::array<uint8_t, 8> fileSignature = {};

		// Read the first 8 bytes to then verify they match the PNG file signature which is { 137, 80, 78, 71, 13, 10, 26, 10 } or { '\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n' }.
		fread(fileSignature.data(), sizeof(uint8_t), fileSignature.size(), imageFile);
		if (png_sig_cmp(fileSignature.data(), 0, fileSignature.size()) == 0) {
			png_structp pngReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			png_infop pngInfo = png_create_info_struct(pngReadStruct);

			png_init_io(pngReadStruct, imageFile);
			// Set the PNG reader to skip the first 8 bytes since we already handled them.
			png_set_sig_bytes(pngReadStruct, fileSignature.size());
			png_read_info(pngReadStruct, pngInfo);

			m_ImageFileInfo[ImageFileInfoType::ImageBitDepth] = static_cast<int>(png_get_bit_depth(pngReadStruct, pngInfo));
			m_ImageFileInfo[ImageFileInfoType::ImageWidth] = static_cast<int>(png_get_image_width(pngReadStruct, pngInfo));
			m_ImageFileInfo[ImageFileInfoType::ImageHeight] = static_cast<int>(png_get_image_height(pngReadStruct, pngInfo));

			png_destroy_read_struct(&pngReadStruct, &pngInfo, nullptr);
		} else {
			RTEAbort("Encountered invalid PNG file signature while attempting to read info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may be corrupt or is not a PNG file.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::ReadAndStoreBMPFileInfo(FILE *imageFile) {
		std::array<uint8_t, 2> bmpSignature = { 0x42, 0x4D }; // { 'B', 'M' }.
		std::array<uint8_t, 2> fileSignature = {};

		// Read the first 2 bytes to then verify they match the BMP file signature.
		fread(fileSignature.data(), sizeof(uint8_t), fileSignature.size(), imageFile);
		if (fileSignature == bmpSignature) {
			std::array<uint8_t, 4> bmpData = {};

			const auto toInt32 = [](uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
				return (static_cast<int32_t>(b0)) | (static_cast<int32_t>(b1) << 8) | (static_cast<int32_t>(b2) << 16) | (static_cast<int32_t>(b3) << 24);
			};

			// Skip the next 16 bytes. Dimensions data starts at the 18th byte.
			fseek(imageFile, 16, SEEK_CUR);

			fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
			m_ImageFileInfo[ImageFileInfoType::ImageWidth] = toInt32(bmpData[0], bmpData[1], bmpData[2], bmpData[3]);

			fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
			m_ImageFileInfo[ImageFileInfoType::ImageHeight] = toInt32(bmpData[0], bmpData[1], bmpData[2], bmpData[3]);

			// Skip the next 2 bytes. Bit depth data starts at the 28th byte.
			fseek(imageFile, 2, SEEK_CUR);

			// Bit depth is stored as 2 bytes, so ignore the last 2 in the array when converting to int32.
			fread(bmpData.data(), sizeof(uint8_t), bmpData.size(), imageFile);
			m_ImageFileInfo[ImageFileInfoType::ImageBitDepth] = toInt32(bmpData[0], bmpData[1], 0, 0);
		} else {
			RTEAbort("Encountered invalid BMP file signature while attempting to read info of image file with following path and name:\n\n" + m_DataPath + "\n\nThe file may be corrupt or is not a BMP file.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::ReloadAllBitmaps() {
		for (const std::unordered_map<std::string, BITMAP *> &bitmapCache : s_LoadedBitmaps) {
			for (const auto &[filePath, oldBitmap] : bitmapCache) {
				ReloadBitmap(filePath);
			}
		}
		g_ConsoleMan.PrintString("SYSTEM: Sprites reloaded");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::GetAsBitmap(int conversionMode, bool storeBitmap, const std::string &dataPathToSpecificFrame) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		BITMAP *returnBitmap = nullptr;
		const int bitDepth = conversionMode == COLORCONV_8_TO_32 ? BitDepths::ThirtyTwo : BitDepths::Eight;
		std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;

		if (g_PresetMan.GetReloadEntityPresetCalledThisUpdate()) {
			ReloadBitmap(dataPathToLoad, conversionMode);
		}

		// Check if the file has already been read and loaded from the disk and, if so, use that data.
		std::unordered_map<std::string, BITMAP *>::iterator foundBitmap = s_LoadedBitmaps[bitDepth].find(dataPathToLoad);
		if (foundBitmap != s_LoadedBitmaps[bitDepth].end()) {
			returnBitmap = (*foundBitmap).second;
		} else {
			if (!System::PathExistsCaseSensitive(dataPathToLoad)) {
				const std::string dataPathWithoutExtension = dataPathToLoad.substr(0, dataPathToLoad.length() - m_DataPathExtension.length());
				const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

				if (System::PathExistsCaseSensitive(dataPathWithoutExtension + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + altFileExtension);
					dataPathToLoad = dataPathWithoutExtension + altFileExtension;
				} else {
					RTEAbort("Failed to find image file with following path and name:\n\n" + dataPathToLoad + " or " + altFileExtension + "\n" + m_FormattedReaderPosition);
				}
			}
			returnBitmap = LoadAndReleaseBitmap(conversionMode, dataPathToLoad); // NOTE: This takes ownership of the bitmap file

			// Insert the bitmap into the map, PASSING OVER OWNERSHIP OF THE LOADED DATAFILE
			if (storeBitmap) { s_LoadedBitmaps[bitDepth].try_emplace(dataPathToLoad, returnBitmap); }
		}
		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::GetAsAnimation(std::vector<BITMAP *> &vectorToFill, int frameCount, int conversionMode) {
		if (m_DataPath.empty() || frameCount < 1) {
			return;
		}
		vectorToFill.reserve(frameCount);

		if (frameCount == 1) {
			// Check for 000 in the file name in case it is part of an animation but the FrameCount was set to 1. Do not warn about this because it's normal operation, but warn about incorrect extension.
			if (!System::PathExistsCaseSensitive(m_DataPath)) {
				const std::string altFileExtension = (m_DataPathExtension == ".png") ? ".bmp" : ".png";

				if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + m_DataPathExtension)) {
					SetDataPath(m_DataPathWithoutExtension + "000" + m_DataPathExtension);
				} else if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + "000" + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + "000" + altFileExtension);
				}
			}
			vectorToFill.emplace_back(GetAsBitmap(conversionMode));
		} else {
			char framePath[1024];
			for (int frameNum = 0; frameNum < frameCount; ++frameNum) {
				std::snprintf(framePath, sizeof(framePath), "%s%03i%s", m_DataPathWithoutExtension.c_str(), frameNum, m_DataPathExtension.c_str());
				vectorToFill.emplace_back(GetAsBitmap(conversionMode, true, framePath));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * ContentFile::LoadAndReleaseBitmap(int conversionMode, const std::string &dataPathToSpecificFrame) {
		if (m_DataPath.empty()) {
			return nullptr;
		}
		const std::string dataPathToLoad = dataPathToSpecificFrame.empty() ? m_DataPath : dataPathToSpecificFrame;

		BITMAP *returnBitmap = nullptr;

		PALETTE currentPalette;
		get_palette(currentPalette);

		set_color_conversion((conversionMode == COLORCONV_NONE) ? COLORCONV_MOST : conversionMode);
		returnBitmap = load_bitmap(dataPathToLoad.c_str(), currentPalette);
		RTEAssert(returnBitmap, "Failed to load image file with following path and name:\n\n" + m_DataPathAndReaderPosition + "\nThe file may be corrupt, incorrectly converted or saved with unsupported parameters.");

		return returnBitmap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::GetAsSound(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		std::unordered_map<std::string, FMOD::Sound *>::iterator foundSound = s_LoadedSamples.find(m_DataPath);
		if (foundSound != s_LoadedSamples.end()) {
			returnSample = (*foundSound).second;
		} else {
			returnSample = LoadAndReleaseSound(abortGameForInvalidSound, asyncLoading); //NOTE: This takes ownership of the sample file

			// Insert the Sound object into the map, PASSING OVER OWNERSHIP OF THE LOADED FILE
			s_LoadedSamples.try_emplace(m_DataPath, returnSample);
		}
		return returnSample;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD::Sound * ContentFile::LoadAndReleaseSound(bool abortGameForInvalidSound, bool asyncLoading) {
		if (m_DataPath.empty() || !g_AudioMan.IsAudioEnabled()) {
			return nullptr;
		}

		if (!System::PathExistsCaseSensitive(m_DataPath)) {
			bool foundAltExtension = false;
			for (const std::string &altFileExtension : c_SupportedAudioFormats) {
				if (System::PathExistsCaseSensitive(m_DataPathWithoutExtension + altFileExtension)) {
					g_ConsoleMan.AddLoadWarningLogExtensionMismatchEntry(m_DataPath, m_FormattedReaderPosition, altFileExtension);
					SetDataPath(m_DataPathWithoutExtension + altFileExtension);
					foundAltExtension = true;
					break;
				}
			}
			if (!foundAltExtension) {
				std::string errorMessage = "Failed to find audio file with following path and name:\n\n" + m_DataPath + " or any alternative supported file type";
				RTEAssert(!abortGameForInvalidSound, errorMessage + "\n" + m_FormattedReaderPosition);
				g_ConsoleMan.PrintString(errorMessage + ". The file was not loaded!");
				return nullptr;
			}
		}
		if (std::filesystem::file_size(m_DataPath) == 0) {
			const std::string errorMessage = "Failed to create sound because the file was empty. The path and name were: ";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
			g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
			return nullptr;
		}
		FMOD::Sound *returnSample = nullptr;

		FMOD_MODE fmodFlags = FMOD_CREATESAMPLE | FMOD_3D | (asyncLoading ? FMOD_NONBLOCKING : FMOD_DEFAULT);
		FMOD_RESULT result = g_AudioMan.GetAudioSystem()->createSound(m_DataPath.c_str(), fmodFlags, nullptr, &returnSample);

		if (result != FMOD_OK) {
			const std::string errorMessage = "Failed to create sound because of FMOD error:\n" + std::string(FMOD_ErrorString(result)) + "\nThe path and name were: ";
			RTEAssert(!abortGameForInvalidSound, errorMessage + "\n\n" + m_DataPathAndReaderPosition);
			g_ConsoleMan.PrintString("ERROR: " + errorMessage + m_DataPath);
			return returnSample;
		}
		return returnSample;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ContentFile::ReloadBitmap(const std::string &filePath, int conversionMode) {
		const int bitDepth = (conversionMode == COLORCONV_8_TO_32) ? BitDepths::ThirtyTwo : BitDepths::Eight;

		auto bmpItr = s_LoadedBitmaps[bitDepth].find(filePath);
		if (bmpItr == s_LoadedBitmaps[bitDepth].end()) {
			return;
		}

		PALETTE currentPalette;
		get_palette(currentPalette);
		set_color_conversion((conversionMode == COLORCONV_NONE) ? COLORCONV_MOST : conversionMode);

		BITMAP *loadedBitmap = (*bmpItr).second;
		BITMAP *newBitmap = load_bitmap(filePath.c_str(), currentPalette);
		BITMAP swap;

		std::memcpy(&swap, loadedBitmap, sizeof(BITMAP));
		std::memcpy(loadedBitmap, newBitmap, sizeof(BITMAP));
		std::memcpy(newBitmap, &swap, sizeof(BITMAP));

		destroy_bitmap(newBitmap);
	}
}