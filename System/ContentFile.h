#ifndef _RTECONTENTFILE_
#define _RTECONTENTFILE_

#include "Serializable.h"

#ifdef __USE_SOUND_FMOD
#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"
#define AUDIO_STRUCT FSOUND_SAMPLE
struct FSOUND_STREAM;

#elif __USE_SOUND_GORILLA
#include "gorilla/ga.h"
#include "gorilla/gau.h"
#define AUDIO_STRUCT ga_Sound
struct ga_Handle;
#endif

struct DATAFILE;
struct BITMAP;

namespace RTE {

	/// <summary>
	/// A representation of a content file that is stored either directly on disk or packaged in another file.
	/// </summary>
	class ContentFile : public Serializable {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory. Create() should be called before using the object.
		/// </summary>
		ContentFile() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory, and also do a Create() in the same line.
		/// Create() should therefore not be called after using this constructor.
		/// </summary>
		/// <param name="filePath">A string defining the path to where the content file itself is located, either within the package file, or directly on the disk.</param>
		ContentFile(const char *filePath) { Clear(); Create(filePath); }

		/// <summary>
		/// Constructor method used to instantiate a ContentFile object in system memory from a hash value of the file path, and also do a Create() in the same line.
		/// Create() should therefore not be called after using this constructor.
		/// </summary>
		/// <param name="hash">A hash value containing the path to where the content file itself is located.</param>
		ContentFile(size_t hash) { Clear(); if (m_PathHashes.count(hash) == 1) { Create(m_PathHashes[hash].c_str()); } else { Create(""); } }

		/// <summary>
		/// Makes the ContentFile object ready for use.
		/// </summary>
		/// <param name="filePath">A string defining the path to where the content file itself is located, either within the package file, or directly on the disk.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const char *filePath);

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
		virtual ~ContentFile() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ContentFile object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false) { Clear(); }

		/// <summary>
		/// Resets the entire ContentFile, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }

		/// <summary>
		/// Frees all loaded data used by all ContentFile instances. This should ONLY be done when quitting the app, or after everything else is completely destroyed.
		/// </summary>
		static void FreeAllLoaded();
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
		/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
		/// </summary>
		/// <param name="propName">The name of the property to be read.</param>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>
		/// An error return value signaling whether the property was successfully read or not.
		/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
		/// </returns>
		virtual int ReadProperty(std::string propName, Reader &reader);

		/// <summary>
		/// Saves the complete state of this ContentFile to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the ContentFile will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the ID of the Data Module this file is inside.
		/// </summary>
		/// <returns>The ID of the Data Module containing this' file.</returns>
		int GetDataModuleID();

		/// <summary>
		/// Gets the file size of the content file represented by this ContentFile object, in bytes. This should be called AFTER using any of the GetAs methods.
		/// </summary>
		/// <returns>A long describing the file size of the content file.</returns>
		//virtual unsigned long GetDataSize();

		/// <summary>
		/// Gets the Allegro DATAFILE type of the DATAFILE represented by this ContentFile.
		/// </summary>
		/// <returns>A DATAFILE type as described in the Allegro docs.</returns>
		//virtual int GetDataType();

		/// <summary>
		/// Gets the file path of the content file represented by this ContentFile object.
		/// </summary>
		/// <returns>A string with the datafile object name path, like "datafile.dat#mydataobject".</returns>
		virtual const std::string & GetDataPath() const { return m_DataPath; };

		/// <summary>
		/// Sets the file path of the content file represented by this ContentFile object.
		/// </summary>
		/// <param name="newDataPath">A string with the datafile object name path, like: "datafile.dat#mydataobject".</param>
		virtual void SetDataPath(std::string newDataPath);

		/// <summary>
		/// Shows whether the data of this has been modified since load, and should therefore be saved out again to the path if this.
		/// </summary>
		/// <returns>Whether the data of this has been modified in memory since load.</returns>
		//bool IsDataModified() const { return m_DataModified; };

		/// <summary>
		/// Tells this whether the data loaded by this into memory has been modified and should be saved out to this' path upon writing.
		/// </summary>
		/// <param name="modified">Whether the loaded data of this has been modified or not.</param>
		//virtual void SetDataModified(bool modified = true) { m_DataModified = modified; };

		/// <summary>
		/// Creates a hash value out of a path to a ContentFile.
		/// </summary>
		/// <returns>Hash value of a path to a ContentFile.</returns>
		virtual size_t GetHash() const { return std::hash<std::string>()(m_DataPath); }

		/// <summary>
		/// Converts hash values into file paths to ContentFiles.
		/// </summary>
		/// <param name="hash">Hash value to get file path from.</param>
		/// <returns>Path to ContentFile.</returns>
		static std::string GetPathFromHash(size_t hash);
#pragma endregion

#pragma region Data Handling
		/// <summary>
		/// Loads and gets the data represented by this ConentFile object as an Allegro BITMAP. Note that ownership of the BITMAP IS NOT TRANSFERRED!
		/// Also, this should only be done once upon startup, since getting the BITMAP again is slow.
		/// </summary>
		/// <param name="conversionMode">
		/// The Allegro color conversion mode to use when loading this bitmap.
		/// Note it will only apply the first time you get a bitmap since it is only loaded from disk the first time. See allegro docs for the modes.
		/// </param>
		/// <returns>The pointer to the beginning of the data object loaded from the allegro .dat datafile. file. Ownership is NOT transferred! If 0, the file could not be found/loaded.</returns>
		virtual BITMAP * GetAsBitmap(int conversionMode = 0);

		/// <summary>
		/// Loads and gets the data represented by this ConentFile object as an of array Allegro BITMAPs, each representing a frame in an animation.
		/// Note that ownership of the BITMAPS ARE NOT TRANSFERRED, BUT THE ARRAY ITSELF, IS!
		/// Also, this should only be done once upon startup, since getting the BITMAPs again is slow.
		/// </summary>
		/// <param name="frameCount">
		/// The number of frames to attempt to load.
		/// The name of the file/object specified by the data path will be appended with 000, 001. 002 etc up to the frameCount number - 1.
		/// </param>
		/// <param name="conversionMode">
		/// The Allegro color conversion mode to use when loading this bitmap.
		/// Note it will only apply the first time you get a bitmap since it is only loaded from disk the first time. See allegro docs for the modes.
		/// </param>
		/// <returns>
		/// The pointer to the beginning of the array of BITMAP pointers loaded from the allegro .dat datafile. file, the length of which is specified with the frameCount argument.
		/// Ownership of the array IS transferred! Ownership of the BITMAPS is NOT transferred! If 0, the file could not be found/loaded.
		/// </returns>
		virtual BITMAP ** GetAsAnimation(int frameCount = 1, int conversionMode = 0);

		/// <summary>
		/// Loads and gets the data represented by this ConentFile object as an FMOD FSOUND_SAMPLE. Note that ownership of the SAMPLE IS NOT TRANSFERRED!
		/// Also, this should only be done once upon startup, since getting the Sample again is slow.
		/// </summary>
		/// <returns>The pointer to the beginning of the data object loaded from the file. Ownership is NOT transferred! If 0, the file could not be found/loaded.</returns>
		virtual AUDIO_STRUCT * GetAsSample();

		/// <summary>
		/// Loads and transfers the data represented by this ConentFile object as an Allegro BITMAP. Note that ownership of the BITMAP IS TRANSFERRED!
		/// Also, this is relatively slow since it reads the data from disk each time.
		/// </summary>
		/// <param name="conversionMode">
		/// The Allegro color conversion mode to use when loading this bitmap.
		/// Note it will only apply the first time you get a bitmap since it is only loaded from disk the first time. See allegro docs for the modes.
		/// </param>
		/// <returns>The pointer to the BITMAP loaded from disk Ownership IS transferred! If 0, the file could not be found/loaded.</returns>
		virtual BITMAP * LoadAndReleaseBitmap(int conversionMode = 0);

		/// <summary>
		/// Loads and transfers the data represented by this ConentFile object as a series of Allegro BITMAPs. Note that ownership of both the ARRAY and the BITMAPS ARE TRANSFERRED!
		/// Also, this is relatively slow since it reads the data from disk each time.
		/// </summary>
		/// <param name="frameCount"> 
		/// The number of frames to attempt to load. 
		/// The name of the file/object specified by the data path will be appended with 000, 001. 002 etc up to the frameCount number - 1.
		/// </param>
		/// <param name="conversionMode">The Allegro color conversion mode to use when loading this bitmap.</param>
		/// <returns>The pointer to the BITMAP loaded from disk Ownership IS transferred! If 0, the file could not be found/loaded.</returns>
		virtual BITMAP ** LoadAndReleaseAnimation(int frameCount = 1, int conversionMode = 0);

		/// <summary>
		/// Loads and gets the data represented by this ConentFile object as a binary chunk of data. Note that ownership of the DATA IS NOT TRANSFERRED!
		/// Also, this should only be done once upon startup, since getting the data again is slow.
		/// </summary>
		/// <returns>The pointer to the beginning of the raw data loaded from the Allegro .dat datafile. file. Ownership is NOT transferred! If 0, the file could not be found/loaded.</returns>
		//virtual char * GetAsRawBinary();
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Entity.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return m_ClassName; }
#pragma endregion

	protected:

		static const std::string m_ClassName; //!< A string with the friendly-formatted type name of this object.

		enum BitDepths { Eight = 0, ThirtyTwo, BitDepthCount }; //!< Enumeration for loading BITMAPs by bit depth.

		std::string m_DataPath; //!< Path to this ContentFile's Datafile Object's path. "datafile.dat#objectname". In the case of an animation, this filename/name will be appended with 000, 001, 002 etc.
		static std::map<size_t, std::string> m_PathHashes; //!< Hash value of the path to this ContentFile's Datafile Object.

		static std::map<std::string, BITMAP *> m_sLoadedBitmaps[BitDepthCount]; //!< Static map containing all the already loaded BITMAPs and their paths, and there's two maps, for each bit depth.
		static std::map<std::string, AUDIO_STRUCT *> m_sLoadedSamples; //!< Static map containing all the already loaded FSOUND_SAMPLEs and their paths.
		//static std::map<std::string, std::pair<char *, long> > m_sLoadedBinary; //!< Static map containing all the already loaded binary data. First in pair is the data, second is size in bytes.

		int m_DataModuleID; //!< Data Module ID of where this was loaded from.

		void *m_pLoadedData; //!< Non-ownership pointer to the loaded data for convenience. Do not release/delete this.   
		//long m_LoadedDataSize; //!< Size of loaded data.
		//bool m_DataModified; //!< Whether the data itself has been modified since load, and should be saved out again to the path on write.

		DATAFILE *m_pDataFile; //!< This is only if the data is loaded from a datafile; needs to be saved so that it can be unloaded as some point.

	private:

		/// <summary>
		/// Clears all the member variables of this ContentFile, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif