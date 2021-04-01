#ifndef _RTESOUNDSET_
#define _RTESOUNDSET_

#include "Vector.h"
#include "ContentFile.h"
#include "LuaMan.h"

namespace RTE {

	/// <summary>
	/// A set of sounds, and their selection data.
	/// </summary>
	class SoundSet : public Serializable {
		friend class LuaMan;

	public:

		SerializableOverrideMethods

		/// <summary>
		/// How the SoundSet should choose the next sound or SoundSet to play when SelectNextSound is called.
		/// </summary>
		enum SoundSelectionCycleMode {
			RANDOM = 0,
			FORWARDS,
			ALL
		};

		/// <summary>
		/// Self-contained struct defining an individual sound in a SoundSet.
		/// </summary>
		struct SoundData {
			ContentFile SoundFile;
			FMOD::Sound *SoundObject;
			Vector Offset = Vector();
			float MinimumAudibleDistance = 0.0F;
			float AttenuationStartDistance = -1.0F;
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SoundSet object in system memory. Create() should be called before using the object.
		/// </summary>
		SoundSet() { Clear(); }

		/// <summary>
		/// Creates a SoundSet to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SoundSet to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SoundSet &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SoundSet object before deletion from system memory.
		/// </summary>
		~SoundSet() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SoundSet object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Handles reading a SoundData from INI, loading it in as a ContentFile and into FMOD, and reading any of its subproperties.
		/// Does not add the created SoundData to a this SoundSet.
		/// </summary>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>SoundData for the newly read sound.</returns>
		static SoundData ReadAndGetSoundData(Reader &reader);

		/// <summary>
		/// Handles turning a SoundCelectionCycleMode from its user-friendly name in INI to its enum value, using the static SoundSelectionCycleMap.
		/// </summary>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>The appropriate SoundSelectionCycleMode for the given INI value.</returns>
		static SoundSelectionCycleMode ReadSoundSelectionCycleMode(Reader &reader);

		/// <summary>
		/// Handles writing the given SoundSelectionCycleMode out to the given Writer, using the static SoundSelectionCycleMap.
		/// </summary>
		/// <param name="writer">A Writer filled in with the property to write to.</param>
		/// <param name="soundSelectionCycleMode">The SoundSelectionCycleMode to write.</param>
		static void SaveSoundSelectionCycleMode(Writer &writer, SoundSelectionCycleMode soundSelectionCycleMode);
#pragma endregion

#pragma region SoundData and SoundSet Addition
		/// <summary>
		/// Adds a new sound to this SoundSet, spitting out a Lua error if it fails. The sound will have default configuration.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		void AddSound(const std::string &soundFilePath) { AddSound(soundFilePath, false); }

		/// <summary>
		/// Adds a new sound to this SoundSet, either spitting out a Lua error or aborting if it fails. The sound will have default configuration.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error.</param>
		void AddSound(const std::string &soundFilePath, bool abortGameForInvalidSound) { AddSound(soundFilePath, Vector(), 0, -1, abortGameForInvalidSound); }

		/// <summary>
		/// Adds a new sound to this SoundSet, spitting out a Lua error if it fails. The sound will be configured based on parameters.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="offset">The offset position to play this sound at, where (0, 0) is no offset.</param>
		/// <param name="minimumAudibleDistance">The minimum distance at which this sound will be audible. 0 means there is none, which is normally the case.</param>
		/// <param name="attenuationStartDistance">The attenuation start distance for this sound, -1 sets it to default.</param>
		void AddSound(const std::string &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance) { AddSound(soundFilePath, offset, minimumAudibleDistance, attenuationStartDistance, false); }

		/// <summary>
		/// Adds a new sound to this SoundSet, either spitting out a Lua error or aborting if it fails. The sound will be configured based on parameters.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="offset">The offset position to play this sound at, where (0, 0) is no offset.</param>
		/// <param name="minimumAudibleDistance">The minimum distance at which this sound will be audible. 0 means there is none, which is normally the case.</param>
		/// <param name="attenuationStartDistance">The attenuation start distance for this sound, -1 sets it to default.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error.</param>
		void AddSound(const std::string &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound);

		/// <summary>
		/// Adds a copy of the given SoundData to this SoundSet.
		/// </summary>
		/// <param name="soundDataToAdd">The SoundData to copy to this SoundSet.</param>
		void AddSoundData(SoundData soundDataToAdd) { m_SoundData.push_back(soundDataToAdd); }

		/// <summary>
		/// Adds a copy of the passed in SoundSet as a sub SoundSet of this SoundSet. Ownership IS transferred!
		/// </summary>
		/// <param name="soundSetToAdd">A reference to the SoundSet to be copied in as a sub SoundSet of this SoundSet. Ownership IS transferred!</param>
		void AddSoundSet(const SoundSet &soundSetToAdd) { m_SubSoundSets.push_back(soundSetToAdd); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Shows whether this SoundSet has any sounds in it or, optionally its SubSoundSets.
		/// </summary>
		/// <returns>Whether this SoundSet has any sounds, according to the conditions.</returns>
		bool HasAnySounds(bool includeSubSoundSets = true) const;

		/// <summary>
		/// Gets the current SoundSelectionCycleMode for this SoundSet, which is used to determine what SoundSet to select next time SelectNextSounds is called.
		/// </summary>
		/// <returns>The current sound selection cycle mode.</returns>
		SoundSelectionCycleMode GetSoundSelectionCycleMode() const { return m_SoundSelectionCycleMode; }

		/// <summary>
		/// Sets the SoundSelectionCycleMode for this SoundSet, which is used to determine what SoundSet to select next time SelectNextSounds is called.
		/// </summary>
		/// <param name="newSoundSelectionCycleMOde">The new SoundSelectionCycleMode for this SoundSet.</param>
		void SetSoundSelectionCycleMode(SoundSelectionCycleMode newSoundSelectionCycleMode) { m_SoundSelectionCycleMode = newSoundSelectionCycleMode; if (m_SoundSelectionCycleMode == SoundSelectionCycleMode::FORWARDS) { m_CurrentSelection.second = -1; } }

		/// <summary>
		/// Fills the passed in vector with the flattened SoundData in the SoundSet, optionally only getting currently selected SoundData.
		/// </summary>
		/// <param name="flattenedSoundData">A reference vector of SoundData references to be filled with this SoundSet's flattened SoundData.</param>
		/// <param name="onlyGetSelectedSoundData">Whether to only get SoundData that is currently selected, or to get all SoundData in this SoundSet.</param>
		void GetFlattenedSoundData(std::vector<SoundData *> &flattenedSoundData, bool onlyGetSelectedSoundData);

		/// <summary>
		/// Fills the passed in vector with the flattened SoundData in the SoundSet, optionally only getting currently selected SoundData.
		/// </summary>
		/// <param name="flattenedSoundData">A reference vector of SoundData references to be filled with this SoundSet's flattened SoundData.</param>
		/// <param name="onlyGetSelectedSoundData">Whether to only get SoundData that is currently selected, or to get all SoundData in this SoundSet.</param>
		void GetFlattenedSoundData(std::vector<const SoundData *> &flattenedSoundData, bool onlyGetSelectedSoundData) const;

		/// <summary>
		/// Gets the vector of SubSoundSets for this SoundSet.
		/// </summary>
		/// <returns>The vector of SubSoundSets for this SoundSet.</returns>
		std::vector<SoundSet> & GetSubSoundSets() { return m_SubSoundSets; }
#pragma endregion

#pragma region Miscellaneous
		/// <summary>
		/// Selects the next sounds of this SoundSet to be played, also selecting them for sub SoundSets as appropriate.
		/// </summary>
		/// <returns>False if this SoundSet or any of its sub SoundSets failed to select sounds, or true if everything worked.</returns>
		bool SelectNextSounds();
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Serializable.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		const std::string &GetClassName() const override { return m_sClassName; }
#pragma endregion

	private:

		static const std::string m_sClassName; //!< A string with the friendly-formatted type name of this object.
		static const std::unordered_map<std::string, SoundSet::SoundSelectionCycleMode> c_SoundSelectionCycleModeMap; //!< A map of strings to SoundSelectionCycleModes to support string parsing for the SoundCycleMode enum. Populated in the implementing cpp file.

		SoundSelectionCycleMode m_SoundSelectionCycleMode; //!< The SoundSelectionCycleMode for this SoundSet.
		std::pair<bool, int> m_CurrentSelection; //!< Whether the currently selection is in the SoundData (false) or SoundSet (true) vector, and its index in the appropriate vector.

		std::vector<SoundData> m_SoundData; //!< The SoundData available for selection in this SoundSet.
		std::vector<SoundSet> m_SubSoundSets; //!< The sub SoundSets available for selection in this SoundSet.

		/// <summary>
		/// Clears all the member variables of this SoundSet, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif