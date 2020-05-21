#ifndef _RTESOUND_
#define _RTESOUND_

#include "Entity.h"
#include "ContentFile.h"
#include "AudioMan.h"

namespace RTE {
	class Vector;

	/// <summary>
	/// A container for sounds that represent a specific sound effect.
	/// </summary>
	class SoundContainer : public Entity {
		
	public:
		EntityAllocation(SoundContainer)
		ClassInfoGetters
		
		/// <summary>
		/// How the SoundContainer should choose the next SoundSet to play when SelectNextSoundSet is called
		/// </summary>
		enum SoundCycleMode {
			MODE_RANDOM = 0,
			MODE_FORWARDS = 1
		};

		/// <summary>
		/// Self-contained struct defining an individual sound in a SoundSet
		/// </summary>
		struct SoundData {
			ContentFile SoundFile;
			FMOD::Sound *SoundObject;
			Vector Offset = Vector();
			float MinimumAudibleDistance = 0;
			float AttenuationStartDistance = -1;
			FMOD_VECTOR CustomRolloffPoints[2];
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SoundContainer object in system memory. Create() should be called before using the object.
		/// </summary>
		SoundContainer() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a SoundContainer object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A reference to the SoundContainer to deep copy.</param>
		SoundContainer(const SoundContainer &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Creates a SoundContainer to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SoundContainer to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SoundContainer &reference);

		/// <summary>
		/// Creates an empty SoundContainer with no sounds.
		/// </summary>
		/// <param name="loops">The number of times this SoundContainer's sounds will loop. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="affectedByGlobalPitch">Whether this SoundContainer's sounds' frequency will be affected by the global pitch.</param>
		/// <param name="attenuationStartDistance">The distance at which this SoundContainer's sounds should start attenuating away.</param>
		/// <param name="immobile">Whether this SoundContainer's sounds will be treated as immobile, i.e. they won't be affected by 3D sound manipulation.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(int loops = 0, bool affectedByGlobalPitch = true, float attenuationStartDistance = -1, bool immobile = false) { SetLoopSetting(loops); m_AffectedByGlobalPitch = affectedByGlobalPitch; SetAttenuationStartDistance(attenuationStartDistance); m_Immobile = immobile; return 0; }

		/// <summary>
		/// Creates a SoundContainer and gives it a path to its first sound.
		/// </summary>
		/// <param name="soundFilePath">A path to a sound to add to the first SoundSet of this SoundContainer.</param>
		/// <param name="loops">The number of times this SoundContainer's sounds will loop. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="affectedByGlobalPitch">Whether this SoundContainer's sounds' frequency will be affected by the global pitch.</param>
		/// <param name="attenuationStartDistance">The distance at which this SoundContainer's sounds should start attenuating away.</param>
		/// <param name="immobile">Whether this SoundContainer's sounds will be treated as immobile, i.e. they won't be affected by 3D sound manipulation.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const std::string soundFilePath, int loops = 0, bool affectedByGlobalPitch = true, float attenuationStartDistance = -1, bool immobile = false) { int result = Create(loops, affectedByGlobalPitch, attenuationStartDistance, immobile); AddSound(soundFilePath); return result; }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SoundContainer object before deletion from system memory.
		/// </summary>
		virtual ~SoundContainer() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SoundContainer object. It doesn't delete the Sound files, since they're owned by ContentFile static maps.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false) { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire SoundContainer, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Reads a property value from a Reader stream.
		/// If the name isn't recognized by this class, then ReadProperty of the parent class is called.
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
		/// Handles reading a SoundSet from INI. If the Reader is trying to read a line adding a sound, it'll call ReadSound and add the resulting sound to a new SoundSet.
		/// </summary>
		/// <param name="propName">The name of the property to be read.</param>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>An error return value signaling whether the property was successfully read or not. 0 means it was succesful, any nonzero value means it failed.</returns>
		int ReadSoundOrSoundSet(const std::string &propName, Reader &reader);

		/// <summary>
		/// Handles reading a SoundData from INI, loading it in as a ContentFile and into FMOD, and reading any of its subproperties. Does not add the created SoundData to a SoundContainer.
		/// </summary>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>SoundData for the newly read sound.</returns>
		SoundData ReadSound(Reader &reader);

		/// <summary>
		/// Saves the complete state of this SoundContainer to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the SoundContainer will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const { return 0; }
#pragma endregion

#pragma region Sound Addition
		/// <summary>
		/// Adds a new sound to this SoundContainer, spitting out a lua error if it fails.
		/// The Sound will have default configuration and be added to a new SoundSet.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		void AddSound(const std::string &soundFilePath) { return AddSound(soundFilePath, false); }

		/// <summary>
		/// Adds a new sound to this SoundContainer, either spitting out a lua error or aborting if it fails.
		/// The sound will have default configuration and be added to a new SoundSet.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error.</param>
		void AddSound(const std::string &soundFilePath, bool abortGameForInvalidSound) { return AddSound(soundFilePath, Vector(), c_DefaultAttenuationStartDistance, abortGameForInvalidSound); }

		/// <summary>
		/// Adds a new sound to this SoundContainer, either spitting out a lua error or aborting if it fails.
		/// The sound will be configured based on parameters, and be added to a new SoundSet.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="offset">The offset position to play this sound at, where (0, 0) is no offset.</param>
		/// <param name="attenuationStartDistance">The attenuation start distance for this sound, -1 is default.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error.</param>
		void AddSound(const std::string &soundFilePath, const Vector &offset, float attenuationStartDistance, bool abortGameForInvalidSound) { return AddSound(soundFilePath, m_SoundSets.size() + 1, Vector(), 0, c_DefaultAttenuationStartDistance, abortGameForInvalidSound); }

		/// <summary>
		/// Adds a new sound to this SoundContainer, either spitting out a lua error or aborting if it fails.
		/// The sound will be configured based on parameters, and be added to the SoundSet at the given index, or a new one if there is none at that index.
		/// </summary>
		/// <param name="soundFilePath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="soundSetIndex">The SoundSet index to add this new Sound to. If it's not an existing index, a new SoundSet will be added with this Sound.</param>
		/// <param name="offset">The offset position to play this sound at, where (0, 0) is no offset.</param>
		/// <param name="minimumAudibleDistance">The minimum distance at which this sound will be audible. 0 means there is none, which is normally the case.</param>
		/// <param name="attenuationStartDistance">The attenuation start distance for this sound, -1 sets it to default.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error.</param>
		void AddSound(const std::string &soundFilePath, unsigned int soundSetIndex, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound);
#pragma endregion

#pragma region Sound Management Getters and Setters
		/// <summary>
		/// Gets the current list of sounds in the SoundContainer.
		/// </summary>
		/// <returns>A reference to the list.</returns>
		const std::vector<std::vector<SoundData>> *GetSounds() const { return &m_SoundSets; }

		/// <summary>
		/// Shows whether this SoundContainer has been initialized at all yet and loaded with any samples.
		/// </summary>
		/// <returns>Whether this sound has any samples.</returns>
		bool HasAnySounds() const { return m_SoundSets.size() > 0; }

		/// <summary>
		/// Gets the channels playing sounds from this SoundContainer.
		/// </summary>
		/// <returns>The channels currently being used.</returns>
		std::unordered_set<unsigned short> *GetPlayingChannels() { return &m_PlayingChannels; }

		/// <summary>
		/// Indicates whether any sound in this SoundContainer is currently being played.
		/// </summary>
		/// <returns>Whether any sounds are playing.</returns>
		bool IsBeingPlayed() const { return m_PlayingChannels.size() > 0; }

		/// <summary>
		/// Adds a channel index to the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to add.</param>
		void AddPlayingChannel(unsigned short channel) { m_PlayingChannels.insert(channel); }

		/// <summary>
		/// Removes a channel index from the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to remove.</param>
		void RemovePlayingChannel(unsigned short channel) { m_PlayingChannels.erase(channel); }

		/// <summary>
		/// Gets the current sound selection cycle mode, which is used to determine what SoundSet to select next time SelectNextSoundSet is called.
		/// </summary>
		/// <returns>The current sound selection cycle mode.</returns>
		SoundCycleMode GetSoundSelectionCycleMode() const { return m_SoundSelectionCycleMode; }

		/// <summary>
		/// Sets the current sound selection cycle mode, which is used to determine what SoundSet to select next time SelectNextSoundSet is called.
		/// </summary>
		/// <param name="soundSelectionCycleMode">The sound selection cycle mode to use.</param>
		void SetSoundSelectionCycleMode(SoundCycleMode soundSelectionCycleMode) { m_SoundSelectionCycleMode = soundSelectionCycleMode; }

		/// <summary>
		/// Gets the selected SoundSet for this SoundContainer. The selected SoundSet is changed with SelectNextSoundSet.
		/// </summary>
		/// <returns>The selected SoundSet.</returns>
		std::vector<SoundData> GetSelectedSoundSet() const { return m_SoundSets[m_SelectedSoundSet]; }

		/// <summary>
		/// Gets a vector of hashes of the sounds selected to be played next in this SoundContainer.
		/// </summary>
		/// <returns>The currently playing sounds hashes.</returns>
		std::vector<size_t> GetSelectedSoundHashes() const;

		/// <summary>
		/// Gets the SoundData object that corresponds to the given FMOD::Sound. If the sound can't be found, it returns a null pointer.
		/// </summary>
		/// <param name="sound">The FMOD::Sound to search for.</param>
		/// <returns>A pointer to the corresponding SoundData or a null pointer.</returns>
		const SoundData *GetSoundDataForSound(const FMOD::Sound *sound) const;
#pragma endregion

#pragma region Sound Property Getters and Setters
		/// <summary>
		/// Updates the position of the SoundContainer's sounds while they're playing.
		/// </summary>
		/// <param name="position">The new position to play the SoundContainer's sounds.</param>
		/// <returns>Whether this SoundContainer's attenuation setting was successful.</returns>
		bool SetPosition(const Vector &position) { return g_AudioMan.SetSoundPosition(this, position); }

		/// <summary>
		/// Gets the attenuation start distance of this SoundContainer.
		/// </summary>
		/// <returns>A float with the attenuation start distance.</returns>
		float GetAttenuationStartDistance() const { return m_AttenuationStartDistance; }

		/// <summary>
		/// Sets the attenuation start distance of this SoundContainer. Values < 0 set it to default. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="attenuationStartDistance">The new attenuation start distance.</param>
		void SetAttenuationStartDistance(float attenuationStartDistance) { m_AttenuationStartDistance = (attenuationStartDistance < 0) ? c_DefaultAttenuationStartDistance : attenuationStartDistance; m_AllSoundPropertiesUpToDate = false; }

		/// <summary>
		/// Gets the looping setting of this SoundContainer.
		/// </summary>
		/// <returns>An int with the loop count.</returns>
		int GetLoopSetting() const { return m_Loops; }

		/// <summary>
		/// Sets the looping setting of this SoundContainer. Does not affect currently playing sounds.
		/// 0 means the sound is set to only play once. -1 means it loops indefinitely.
		/// </summary>
		/// <param name="loops">The new loop count.</param>
		void SetLoopSetting(int loops) { m_Loops = loops; m_AllSoundPropertiesUpToDate = false; }

		/// <summary>
		/// Gets the current playback priority.
		/// </summary>
		/// <returns>The playback priority.</returns>
		int GetPriority() const { return m_Priority; }

		/// <summary>
		/// Sets the current playback priority. Higher priority (lower value) will make this more likely to make it into mixing on playback. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="priority">The new priority. See AudioMan::PRIORITY_* enumeration.</param>
		void SetPriority(int priority) { m_Priority = Limit(priority, 255, 0); }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer are affected by global pitch changes.</returns>
		bool IsAffectedByGlobalPitch() const { return m_AffectedByGlobalPitch; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer are affected by global pitch changes or not. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="pitched">The new affected by global pitch setting.</param>
		void SetAffectedByGlobalPitch(bool affectedByGlobalPitch) { m_AffectedByGlobalPitch = affectedByGlobalPitch; }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer should be considered immobile, i.e. always play at the listener's position.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer are immobile.</returns>
		bool IsImmobile() const { return m_Immobile; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer should be considered immobile, i.e. always play at the listener's position. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="immobile">The new immobile setting.</param>
		void SetImmobile(bool immobile) { m_Immobile = immobile; m_AllSoundPropertiesUpToDate = false; }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer have all had all their properties set appropriately. Used to account for issues with ordering in INI loading.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer have their properties set appropriately.</returns>
		bool AllSoundPropertiesUpToDate() const { return m_AllSoundPropertiesUpToDate; }
#pragma endregion

#pragma region Playback Controls
		/// <summary>
		/// Plays the next sound of this SoundContainer with at (0, 0) for all players.
		/// </summary>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play() { return Play(Vector(), -1); }

		/// <summary>
		/// Plays the next sound of this SoundContainer at the given position for all players.
		/// </summary>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(const Vector &position) { return Play(position, -1); }

		/// <summary>
		/// Plays the next sound of this container at (0, 0) for the given player. Mostly useful for immobile sounds.
		/// </summary>
		/// <param name="player">The player to start playback of this SoundContainer's sounds for.</param>
		/// <returns>Whether there were sounds to play and they were able to be played.</returns>
		bool Play(int player) { return Play(Vector(), player); }

		/// <summary>
		/// Plays the next sound of this SoundContainer with the given attenuation for a specific player.
		/// </summary>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">The player to start playback of this SoundContainer's sounds for.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(const Vector &position, int player) { return HasAnySounds() ? g_AudioMan.PlaySound(this, (m_Immobile ? Vector() : position), player) : false; }

		/// <summary>
		/// Stops playback of this SoundContainer for all players.
		/// </summary>
		/// <returns>Whether this SoundContainer successfully stopped playing.</returns>
		bool Stop() { return Stop(-1); }

		/// <summary>
		/// Stops playback of this SoundContainer for a specific player.
		/// </summary>
		/// <param name="player">Player to stop playback of this SoundContainer for.</param>
		/// <returns>Whether this SoundContainer successfully stopped playing.</returns>
		bool Stop(int player) { return (HasAnySounds() && IsBeingPlayed()) ? g_AudioMan.StopSound(this, player) : false; }

		/// <summary>
		/// Selects the next sounds of this SoundContainer to be played.
		/// </summary>
		bool SelectNextSoundSet();

		/// <summary>
		/// Fades out playback of the SoundContainer to 0 volume.
		/// </summary>
		/// <param name="fadeOutTime">How long the fadeout should take.</param>
		void FadeOut(int fadeOutTime = 1000) { if (IsBeingPlayed()) { return g_AudioMan.FadeOutSound(this, fadeOutTime); } }
#pragma endregion

#pragma region Miscellaneous
		/// <summary>
		/// Updates all sound properties to match this SoundContainer's settings.
		/// Necessary because sounds loaded from ini seem to be used directly instead of loaded from PresetMan, so their correctness can't be guaranteed when they're played.
		/// </summary>
		/// <returns>The FMOD_RESULT for updating all of the SoundContainer's sounds' properties. If it's not FMOD_OK, something went wrong.</returns>
		FMOD_RESULT UpdateSoundProperties();
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static const std::unordered_map<std::string, SoundCycleMode> c_CycleModeMap; //!< A map of to support string parsing for the CycleMode enum. Populated in the implementing cpp file.

		std::vector<std::vector<SoundData>> m_SoundSets; //The vector of SoundSets in this SoundContainer, wherein a SoundSet is a vector containing one or more SoundData structs.
		size_t m_SelectedSoundSet; //!< The selected SoundSet for this SoundContainer, used to determine what sounds will play when Play is called.
		SoundCycleMode m_SoundSelectionCycleMode; //!< The sound cycle mode for this sound container, used to determine what will play next, each time play is called.

		std::unordered_set<unsigned short> m_PlayingChannels; //!< The channels this SoundContainer is currently using

		float m_AttenuationStartDistance; //!< The distance away from the AudioSystem listener to start attenuating this sound. Attenuation follows FMOD 3D Inverse roll-off model.
		int m_Loops; //!< Number of loops (repeats) the SoundContainer's sounds should play when played. 0 means it plays once, -1 means it plays until stopped 
		int m_Priority; //!< The mixing priority of this SoundContainer's sounds. Higher values are more likely to be heard
		bool m_AffectedByGlobalPitch; //!< Whether this SoundContainer's sounds should be able to be altered by global pitch changes
		bool m_Immobile; //!< Whether this SoundContainer's sounds should be treated as immobile, i.e. not affected by 3D sound effects. Mostly used for GUI sounds and the like.

		bool m_AllSoundPropertiesUpToDate = false; //!< Whether this SoundContainer's sounds' modes and properties are up to date. Used primarily to handle discrepancies that can occur when loading from ini if the line ordering isn't ideal.

	private:

		/// <summary>
		/// TODO This is currently not used in favor of a simpler 2-point method but is kept in case it should be changed back. Examine this and remove or use it in future.
		/// </summary>
		/// <param name="soundDataToCalculateFor"></param>
		/// <param name="rolloffPoints"></param>
		/// <param name="numRolloffPoints"></param>
		void CalculateCustomRolloffPoints(const SoundData &soundDataToCalculateFor, FMOD_VECTOR *rolloffPoints, int numRolloffPoints);

		/// <summary>
		/// Clears all the member variables of this SoundContainer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif