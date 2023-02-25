#ifndef _RTECONTENTFILE_
#define _RTECONTENTFILE_

#include "Serializable.h"

namespace FMOD { class Sound; }
struct BITMAP;

namespace RTE {

	/// <summary>
	/// A representation of a content file that is stored directly on disk.
	/// </summary>
	class ContentFile : public Serializable {

	public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory. Create() should be called before using the object.
		/// </summary>
		ContentFile() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory, and also do a Create() in the same line.
		/// </summary>
		/// <param name="filePath">A string defining the path to where the content file itself is located.</param>
		explicit ContentFile(const char *filePath) { Clear(); Create(filePath); }

		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory from a hash value of the file path, and also do a Create() in the same line.
		/// </summary>
		/// <param name="hash">A hash value containing the path to where the content file itself is located.</param>
		explicit ContentFile(size_t hash) { Clear(); Create(GetPathFromHash(hash).c_str()); }

		/// <summary>
		/// Makes the ContentFile object ready for use.
		/// </summary>
		/// <param name="filePath">A string defining the path to where the content file itself is located.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const char *filePath);

		/// <summary>
		/// Creates a ContentFile to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the ContentFile to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const ContentFile &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a ContentFile object before deletion from system memory.
		/// </summary>
		~ContentFile() override = default;

		/// <summary>
		/// Resets the entire ContentFile, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }

		/// <summary>
		/// Frees all loaded data used by all ContentFile instances. This should ONLY be done when quitting the app, or after everything else is completely destroyed.
		/// </summary>
		static void FreeAllLoaded();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the ID of the Data Module this file is inside.
		/// </summary>
		/// <returns>The ID of the DataModule containing this' file.</returns>
		int GetDataModuleID() const;

		/// <summary>
		/// Gets the file path of the content file represented by this ContentFile object.
		/// </summary>
		/// <returns>A string with the file name path.</returns>
		const std::string & GetDataPath() const { return m_DataPath; }

		/// <summary>
		/// Sets the file path of the content file represented by this ContentFile object.
		/// </summary>
		/// <param name="newDataPath">A string with the new file name path.</param>
		void SetDataPath(const std::string &newDataPath);

		/// <summary>
		/// Creates a hash value out of a path to a ContentFile.
		/// This uses boost::hash for compiler independent hashing.
		/// </summary>
		/// <returns>Hash value of a path to a ContentFile.</returns>
		size_t GetHash() const;

		/// <summary>
		/// Converts hash values into file paths to ContentFiles.
		/// </summary>
		/// <param name="hash">Hash value to get file path from.</param>
		/// <returns>Path to ContentFile.</returns>
		static std::string GetPathFromHash(size_t hash) { return (s_PathHashes.find(hash) == s_PathHashes.end()) ? "" : s_PathHashes[hash]; }
#pragma endregion

#pragma region Logging
		/// <summary>
		/// Gets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// </summary>
		/// <returns>A string containing the currently read file path and the line being read.</returns>
		const std::string & GetFormattedReaderPosition() const { return m_FormattedReaderPosition; }

		/// <summary>
		/// Sets the file and line that are currently being read. Formatted to be used for logging warnings and errors.
		/// </summary>
		/// <param name="newPosition">A string containing the currently read file path and the line being read.</returns>
		void SetFormattedReaderPosition(const std::string &newPosition) override;
#pragma endregion

#pragma region Image Info Getters
		/// <summary>
		/// Gets whether the data file at this ContentFile's path is a supported image format.
		/// </summary>
		/// <returns>Whether the data file at this ContentFile's path is a supported image format.</returns>
		bool DataPathIsImageFile() const { return m_DataPathIsImageFile; }

		/// <summary>
		/// Gets the bit depth of the image file at this ContentFile's path.
		/// </summary>
		/// <returns>The bit depth of the image file at this ContentFile's path, or -1 if the file is not an image format.</returns>
		int GetImageBitDepth() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageBitDepth) : -1; }

		/// <summary>
		/// Gets the width of the image file at this ContentFile's path.
		/// </summary>
		/// <returns>The width of the image file at this ContentFile's path, in pixels, or -1 if the file is not an image format.</returns>
		int GetImageWidth() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageWidth) : -1; }

		/// <summary>
		/// Gets the height of the image file at this ContentFile's path.
		/// </summary>
		/// <returns>The height of the image file at this ContentFile's path, in pixels, or -1 if the file is not an image format.</returns>
		int GetImageHeight() { return m_DataPathIsImageFile ? GetImageFileInfo(ImageFileInfoType::ImageHeight) : -1; }
#pragma endregion

#pragma region Data Handling
		/// <summary>
		/// Reloads all BITMAPs in the cache from disk, allowing any changes to be reflected at runtime.
		/// </summary>
		static void ReloadAllBitmaps();

		/// <summary>
		/// Gets the data represented by this ContentFile object as an Allegro BITMAP, loading it into the static maps if it's not already loaded. Note that ownership of the BITMAP is NOT transferred!
		/// </summary>
		/// <param name="conversionMode">The Allegro color conversion mode to use when loading this bitmap.</param>
		/// <param name="storeBitmap">Whether to store the BITMAP in the relevant static map after loading it or not. If this is false, ownership of the BITMAP IS transferred!</param>
		/// <param name="dataPathToSpecificFrame">Path to a specific frame when loading an animation to avoid overwriting the original preset DataPath when loading each frame.</param>
		/// <returns>Pointer to the BITMAP loaded from disk.</returns>
		BITMAP * GetAsBitmap(int conversionMode = 0, bool storeBitmap = true, const std::string &dataPathToSpecificFrame = "");

		/// <summary>
		/// Fills an existing vector of Allegro BITMAPs representing each frame in the animation with the data represented by this ContentFile object.
		/// It loads the BITMAPs into the static maps if they're not already loaded. Ownership of the BITMAPs is NOT transferred!
		/// </summary>
		/// <param name="vectorToFill">The existing vector of Allegro BITMAPs to fill.</param>
		/// <param name="frameCount">The number of frames to attempt to load, more than 1 frame will mean 00# is appended to DataPath to handle naming conventions.</param>
		/// <param name="conversionMode">The Allegro color conversion mode to use when loading this bitmap.</param>
		void GetAsAnimation(std::vector<BITMAP *> &vectorToFill, int frameCount = 1, int conversionMode = 0);

		/// <summary>
		/// Gets the data represented by this ContentFile object as a vector of Allegro BITMAPs, each representing a frame in the animation.
		/// It loads the BITMAPs into the static maps if they're not already loaded. Ownership of the BITMAPs is NOT transferred!
		/// </summary>
		/// <param name="frameCount">The number of frames to attempt to load, more than 1 frame will mean 00# is appended to DataPath to handle naming conventions.</param>
		/// <param name="conversionMode">The Allegro color conversion mode to use when loading this bitmap.</param>
		/// <returns>A vector of BITMAP pointers loaded from the disk.</returns>
		std::vector<BITMAP *> GetAsAnimation(int frameCount = 1, int conversionMode = 0) { std::vector<BITMAP *> returnBitmaps; GetAsAnimation(returnBitmaps, frameCount, conversionMode); return returnBitmaps; }

		/// <summary>
		/// Gets the data represented by this ContentFile object as an FMOD FSOUND_SAMPLE, loading it into the static maps if it's not already loaded. Ownership of the FSOUND_SAMPLE is NOT transferred!
		/// </summary>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error. Default true.</param>
		/// <param name="asyncLoading">Whether to enable FMOD asynchronous loading or not. Should be disabled for loading audio files with Lua AddSound.
		/// <returns>Pointer to the FSOUND_SAMPLE loaded from disk.</returns>
		FMOD::Sound * GetAsSound(bool abortGameForInvalidSound = true, bool asyncLoading = true);
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for loading BITMAPs by bit depth. NOTE: This can't be lower down because s_LoadedBitmaps relies on this definition.
		/// </summary>
		enum BitDepths { Eight = 0, ThirtyTwo, BitDepthCount };

		/// <summary>
		/// Enumeration for the image file information types that can be stored.
		/// </summary>
		enum ImageFileInfoType { ImageBitDepth, ImageWidth, ImageHeight, ImageInfoTypeCount };

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		static std::unordered_map<size_t, std::string> s_PathHashes; //!< Static map containing the hash values of paths of all loaded data files.
		static std::array<std::unordered_map<std::string, BITMAP *>, BitDepths::BitDepthCount> s_LoadedBitmaps; //!< Static map containing all the already loaded BITMAPs and their paths for each bit depth.
		static std::unordered_map<std::string, FMOD::Sound *> s_LoadedSamples; //!< Static map containing all the already loaded FSOUND_SAMPLEs and their paths.

		std::string m_DataPath; //!< The path to this ContentFile's data file. In the case of an animation, this filename/name will be appended with 000, 001, 002 etc.
		std::string m_DataPathExtension; //!< The extension of the data file of this ContentFile's path.
		std::string m_DataPathWithoutExtension; //!< The path to this ContentFile's data file without the file's extension.

		bool m_DataPathIsImageFile; //!< Whether the data file at this ContentFile's path is a supported image format.
		std::array<int, ImageFileInfoType::ImageInfoTypeCount> m_ImageFileInfo; //!< Array that holds image file information read directly from the data file on disk.

		std::string m_FormattedReaderPosition; //!< A string containing the currently read file path and the line being read. Formatted to be used for logging.
		std::string m_DataPathAndReaderPosition; //!< The path to this ContentFile's data file combined with the ini file and line it is being read from. This is used for logging.

		int m_DataModuleID; //!< Data Module ID of where this was loaded from.

#pragma region Image Info Getters
		/// <summary>
		/// Gets the specified image info from this ContentFile's data file on disk.
		/// </summary>
		/// <param name="infoTypeToGet">The image info type to get. See ImageFileInfoType enumeration.</param>
		/// <returns>An integer value with the requested image info.</returns>
		int GetImageFileInfo(ImageFileInfoType infoTypeToGet);

		/// <summary>
		/// Reads a PNG file from disk and stores the relevant information without actually loading the whole file into memory.
		/// </summary>
		/// <param name="imageFile">Pointer to the file stream to read.</param>
		void ReadAndStorePNGFileInfo(FILE *imageFile);

		/// <summary>
		/// Reads a BMP file from disk and stores the relevant information without actually loading the whole file into memory.
		/// </summary>
		/// <param name="imageFile">Pointer to the file stream to read.</param>
		void ReadAndStoreBMPFileInfo(FILE *imageFile);
#pragma endregion

#pragma region Data Handling
		/// <summary>
		/// Loads and transfers the data represented by this ContentFile object as an Allegro BITMAP. Ownership of the BITMAP IS transferred!
		/// Note that this is relatively slow since it reads the data from disk each time.
		/// </summary>
		/// <param name="conversionMode">The Allegro color conversion mode to use when loading this bitmap. Only applies the first time a bitmap is loaded from the disk.</param>
		/// <param name="dataPathToSpecificFrame">Path to a specific frame when loading an animation to avoid overwriting the original preset DataPath when loading each frame.</param>
		/// <returns>Pointer to the BITMAP loaded from disk.</returns>
		BITMAP * LoadAndReleaseBitmap(int conversionMode = 0, const std::string &dataPathToSpecificFrame = "");

		/// <summary>
		/// Loads and transfers the data represented by this ContentFile object as an FMOD FSOUND_SAMPLE. Ownership of the FSOUND_SAMPLE is NOT transferred!
		/// </summary>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error. Default true.</param>
		/// <param name="asyncLoading">Whether to enable FMOD asynchronous loading or not. Should be disabled for loading audio files with Lua AddSound.</param>
		/// <returns>Pointer to the FSOUND_SAMPLE loaded from disk.</returns>
		FMOD::Sound * LoadAndReleaseSound(bool abortGameForInvalidSound = true, bool asyncLoading = true);

		/// <summary>
		/// Reloads a specific BITMAP in the cache from disk, allowing any changes to be reflected at runtime.
		/// </summary>
		/// <param name="filePath">The filepath to the bitmap we want to reload.</param>
		/// <param name="conversionMode">The Allegro color conversion mode to use when reloading this bitmap.</param>
		static void ReloadBitmap(const std::string &filePath, int conversionMode = 0);
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ContentFile, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif
