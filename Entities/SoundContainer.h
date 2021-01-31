#ifndef _RTESOUNDCONTAINER_
#define _RTESOUNDCONTAINER_

#include "Entity.h"
#include "SoundSet.h"
#include "AudioMan.h"

namespace RTE {
	class Vector;

	/// <summary>
	/// A container for sounds that represent a specific sound effect.
	/// </summary>
	class SoundContainer : public Entity {
		
	public:

		EntityAllocation(SoundContainer)
		SerializableOverrideMethods
		ClassInfoGetters

		/// <summary>
		/// How the SoundContainer should behave when it tries to play again while already playing.
		/// </summary>
		enum SoundOverlapMode {
			OVERLAP = 0,
			RESTART = 1,
			IGNORE_PLAY = 2
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
		/// Creates a SoundContainer and adds a sound, optionally setting whether it's immobile or affected by global pitch.
		/// </summary>
		/// <param name="soundFilePath">The path to a sound to add to the first SoundSet of this SoundContainer.</param>
		/// <param name="immobile">Whether this SoundContainer's sounds will be treated as immobile, i.e. they won't be affected by 3D sound manipulation.</param>
		/// <param name="affectedByGlobalPitch">Whether this SoundContainer's sounds' frequency will be affected by the global pitch.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const std::string &soundFilePath, bool immobile = false, bool affectedByGlobalPitch = true) { m_TopLevelSoundSet.AddSound(soundFilePath, true); SetImmobile(immobile); SetAffectedByGlobalPitch(affectedByGlobalPitch); return 0; }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SoundContainer object before deletion from system memory.
		/// </summary>
		~SoundContainer() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SoundContainer object. It doesn't delete the Sound files, since they're owned by ContentFile static maps.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire SoundContainer, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Sound Management Getters and Setters
		/// <summary>
		/// Shows whether this SoundContainer's top level SoundSet has any SoundData or SoundSets.
		/// </summary>
		/// <returns>Whether this SoundContainer has any sounds.</returns>
		bool HasAnySounds() const { return m_TopLevelSoundSet.HasAnySounds(); }

		/// <summary>
		/// Gets a reference to the top level SoundSet of this SoundContainer, to which all SoundData and sub SoundSets belong.
		/// </summary>
		/// <returns>A reference to the top level SoundSet of this SoundContainer.</returns>
		SoundSet & GetTopLevelSoundSet() { return m_TopLevelSoundSet; }

		/// <summary>
		/// Gets a vector of hashes of the sounds selected to be played next in this SoundContainer.
		/// </summary>
		/// <returns>The currently playing sounds hashes.</returns>
		std::vector<std::size_t> GetSelectedSoundHashes() const;

		/// <summary>
		/// Gets the SoundData object that corresponds to the given FMOD::Sound. If the sound can't be found, it returns a null pointer.
		/// </summary>
		/// <param name="sound">The FMOD::Sound to search for.</param>
		/// <returns>A pointer to the corresponding SoundData or a null pointer.</returns>
		const SoundSet::SoundData * GetSoundDataForSound(const FMOD::Sound *sound) const;

		/// <summary>
		/// Gets the channels playing sounds from this SoundContainer.
		/// </summary>
		/// <returns>The channels currently being used.</returns>
		std::unordered_set<int> const * GetPlayingChannels() const { return &m_PlayingChannels; }

		/// <summary>
		/// Indicates whether any sound in this SoundContainer is currently being played.
		/// </summary>
		/// <returns>Whether any sounds are playing.</returns>
		bool IsBeingPlayed() const { return !m_PlayingChannels.empty(); }

		/// <summary>
		/// Adds a channel index to the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to add.</param>
		void AddPlayingChannel(int channel) { m_PlayingChannels.insert(channel); }

		/// <summary>
		/// Removes a channel index from the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to remove.</param>
		void RemovePlayingChannel(int channel) { m_PlayingChannels.erase(channel); }

		/// <summary>
		/// Gets the SoundOverlapMode of this SoundContainer, which is used to determine how it should behave when it's told to play while already playing.
		/// </summary>
		/// <returns>The SoundOverlapMode of this SoundContainer.</returns>
		SoundOverlapMode GetSoundOverlapMode() const { return m_SoundOverlapMode; }

		/// <summary>
		/// Sets the SoundOverlapMode of this SoundContainer, which is used to determine how it should behave when it's told to play while already playing. 
		/// </summary>
		/// <param name="newSoundOverlapMode">The new SoundOverlapMode this SoundContainer should use.</param>
		void SetSoundOverlapMode(SoundOverlapMode newSoundOverlapMode) { m_SoundOverlapMode = newSoundOverlapMode; }
#pragma endregion

#pragma region Sound Property Getters and Setters
		/// <summary>
		/// Gets whether the sounds in this SoundContainer should be considered immobile, i.e. always play at the listener's position.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer are immobile.</returns>
		bool IsImmobile() const { return m_Immobile; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer should be considered immobile, i.e. always play at the listener's position. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="immobile">The new immobile setting.</param>
		void SetImmobile(bool immobile) { m_Immobile = immobile; m_SoundPropertiesUpToDate = false; }

		/// <summary>
		/// Gets the attenuation start distance of this SoundContainer.
		/// </summary>
		/// <returns>A float with the attenuation start distance.</returns>
		float GetAttenuationStartDistance() const { return m_AttenuationStartDistance; }

		/// <summary>
		/// Sets the attenuation start distance of this SoundContainer. Values < 0 set it to default. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="attenuationStartDistance">The new attenuation start distance.</param>
		void SetAttenuationStartDistance(float attenuationStartDistance) { m_AttenuationStartDistance = (attenuationStartDistance < 0) ? c_DefaultAttenuationStartDistance : attenuationStartDistance; m_SoundPropertiesUpToDate = false; }

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
		void SetLoopSetting(int loops) { m_Loops = loops; m_SoundPropertiesUpToDate = false; }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer have all had all their properties set appropriately. Used to account for issues with ordering in INI loading.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer have their properties set appropriately.</returns>
		bool SoundPropertiesUpToDate() const { return m_SoundPropertiesUpToDate; }

		/// <summary>
		/// Gets the current playback priority.
		/// </summary>
		/// <returns>The playback priority.</returns>
		int GetPriority() const { return m_Priority; }

		/// <summary>
		/// Sets the current playback priority. Higher priority (lower value) will make this more likely to make it into mixing on playback. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="priority">The new priority. See AudioMan::PRIORITY_* enumeration.</param>
		void SetPriority(int priority) { m_Priority = std::clamp(priority, 0, 256); }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <returns>Whether or not the sounds in this SoundContainer are affected by global pitch changes.</returns>
		bool IsAffectedByGlobalPitch() const { return m_AffectedByGlobalPitch; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer are affected by global pitch changes or not. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="affectedByGlobalPitch">The new affected by global pitch setting.</param>
		void SetAffectedByGlobalPitch(bool affectedByGlobalPitch) { m_AffectedByGlobalPitch = affectedByGlobalPitch; }

		/// <summary>
		/// Gets the position at which this SoundContainer's sound will be played. Note that its individual sounds can be offset from this.
		/// </summary>
		/// <returns>The position of this SoundContainer.</returns>
		const Vector & GetPosition() const { return m_Pos; }

		/// <summary>
		/// Sets the position of the SoundContainer's sounds while they're playing.
		/// </summary>
		/// <param name="position">The new position to play the SoundContainer's sounds.</param>
		/// <returns>Whether this SoundContainer's attenuation setting was successful.</returns>
		void SetPosition(const Vector &newPosition) { if (!m_Immobile && newPosition != m_Pos) { m_Pos = newPosition; if (IsBeingPlayed()) { g_AudioMan.ChangeSoundContainerPlayingChannelsPosition(this); } } }

		/// <summary>
		/// Gets the volume the sounds in this SoundContainer are played at. Note that this does not factor volume changes due to the SoundContainer's position.
		/// </summary>
		/// <returns>The volume the sounds in this SoundContainer are played at.</returns>
		float GetVolume() const { return m_Volume; }

		/// <summary>
		/// Sets the volume sounds in this SoundContainer should be played at. Note that this does not factor volume changes due to the SoundContainer's position. Does not affect currently playing sounds.
		/// </summary>
		/// <param name="newVolume">The new volume sounds in this SoundContainer should be played at. Limited between 0 and 10.</param>
		void SetVolume(float newVolume) { newVolume = std::clamp(newVolume, 0.0F, 10.0F); if (IsBeingPlayed()) { g_AudioMan.ChangeSoundContainerPlayingChannelsVolume(this, newVolume); } m_Volume = newVolume; }

		/// <summary>
		/// Gets the pitch the sounds in this SoundContainer are played at. Note that this does not factor in global pitch.
		/// </summary>
		/// <returns>The pitch the sounds in this SoundContainer are played at.</returns>
		float GetPitch() const { return m_Pitch; }

		/// <summary>
		/// Sets the pitch sounds in this SoundContainer should be played at and updates any playing instances accordingly.
		/// </summary>
		/// <param name="newPitch">The new pitch sounds in this SoundContainer should be played at. Limited between 0.125 and 8 (8 octaves up or down).</param>
		void SetPitch(float newPitch) { m_Pitch = std::clamp(newPitch, 0.125F, 8.0F); if (IsBeingPlayed()) { g_AudioMan.ChangeSoundContainerPlayingChannelsPitch(this); } }
#pragma endregion

#pragma region Playback Controls
		/// <summary>
		/// Plays the next sound of this SoundContainer at its current position for all players.
		/// </summary>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play() { return Play(-1); }

		/// <summary>
		/// Plays the next sound of this container at its current position.
		/// </summary>
		/// <param name="player">The player to start playback of this SoundContainer's sounds for.</param>
		/// <returns>Whether there were sounds to play and they were able to be played.</returns>
		bool Play(int player);

		/// <summary>
		/// Plays the next sound of this SoundContainer at the given position for all players.
		/// </summary>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(const Vector &position) { return Play(position, -1); }

		/// <summary>
		/// Plays the next sound of this SoundContainer with the given attenuation for a specific player.
		/// </summary>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">The player to start playback of this SoundContainer's sounds for.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(const Vector &position, int player) { SetPosition(position); return Play(player); }

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
		bool Stop(int player) { return (HasAnySounds() && IsBeingPlayed()) ? g_AudioMan.StopSoundContainerPlayingChannels(this, player) : false; }

		/// <summary>
		/// Restarts playback of this SoundContainer for all players.
		/// </summary>
		/// <returns>Whether this SoundContainer successfully restarted its playback.</returns>
		bool Restart() { return Restart(-1); }

		/// <summary>
		/// Restarts playback of this SoundContainer for a specific player.
		/// </summary>
		/// <param name="player">Player to restart playback of this SoundContainer for.</param>
		/// <returns>Whether this SoundContainer successfully restarted its playback.</returns>
		bool Restart(int player) { return (HasAnySounds() && IsBeingPlayed()) ? g_AudioMan.StopSoundContainerPlayingChannels(this, player) && g_AudioMan.PlaySoundContainer(this, player) : false; }

		/// <summary>
		/// Fades out playback of the SoundContainer to 0 volume.
		/// </summary>
		/// <param name="fadeOutTime">How long the fadeout should take.</param>
		void FadeOut(int fadeOutTime = 1000) { if (IsBeingPlayed()) { return g_AudioMan.FadeOutSoundContainerPlayingChannels(this, fadeOutTime); } }
#pragma endregion

#pragma region Miscellaneous
		/// <summary>
		/// Updates all sound properties to match this SoundContainer's settings.
		/// Necessary because sounds loaded from ini seem to be used directly instead of loaded from PresetMan, so their correctness can't be guaranteed when they're played.
		/// </summary>
		/// <returns>The FMOD_RESULT for updating all of the SoundContainer's sounds' properties. If it's not FMOD_OK, something went wrong.</returns>
		FMOD_RESULT UpdateSoundProperties();
#pragma endregion

	private:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static const std::unordered_map<std::string, SoundOverlapMode> c_SoundOverlapModeMap; //!< A map of strings to SoundOverlapModes to support string parsing for the SoundOverlapMode enum. Populated in the implementing cpp file.

		SoundSet m_TopLevelSoundSet; //The top level SoundSet that handles all SoundData and sub SoundSets in this SoundContainer.

		std::unordered_set<int> m_PlayingChannels; //!< The channels this SoundContainer is currently using.
		SoundOverlapMode m_SoundOverlapMode; //!< The SoundOverlapMode for this SoundContainer, used to determine how it should handle overlapping play calls.

		bool m_Immobile; //!< Whether this SoundContainer's sounds should be treated as immobile, i.e. not affected by 3D sound effects. Mostly used for GUI sounds and the like.
		float m_AttenuationStartDistance; //!< The distance away from the AudioSystem listener to start attenuating this sound. Attenuation follows FMOD 3D Inverse roll-off model.
		int m_Loops; //!< Number of loops (repeats) the SoundContainer's sounds should play when played. 0 means it plays once, -1 means it plays until stopped.
		bool m_SoundPropertiesUpToDate = false; //!< Whether this SoundContainer's sounds' modes and properties are up to date. Used primarily to handle discrepancies that can occur when loading from ini if the line ordering isn't ideal.
		
		int m_Priority; //!< The mixing priority of this SoundContainer's sounds. Higher values are more likely to be heard.
		bool m_AffectedByGlobalPitch; //!< Whether this SoundContainer's sounds should be able to be altered by global pitch changes.
		
		Vector m_Pos; //!< The current position of this SoundContainer's sounds.
		float m_Pitch; //!< The current natural pitch of this SoundContainer's sounds.
		float m_Volume; //!< The current natural volume of this SoundContainer's sounds.

		/// <summary>
		/// Clears all the member variables of this SoundContainer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif