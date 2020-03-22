#ifndef _RTESOUND_
#define _RTESOUND_

#include "Entity.h"
#include "ContentFile.h"
#include "AudioMan.h"

namespace RTE {

	/// <summary>
	/// A container for sounds that represent a specific sound effect.
	/// </summary>
	class SoundContainer : public Entity {
		
	public:
		ENTITYALLOCATION(SoundContainer)
		CLASSINFOGETTERS

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
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(int loops = 0, bool affectedByGlobalPitch = true) { m_Loops = loops; m_AffectedByGlobalPitch = affectedByGlobalPitch; return 0; }

		/// <summary>
		/// Creates a SoundContainer and gives it a path to its first sound.
		/// </summary>
		/// <param name="soundPath">A path to the sound for this sound to have.</param>
		/// <param name="loops">The number of times this SoundContainer's sounds will loop. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="affectedByGlobalPitch">Whether this SoundContainer's sounds' frequency will be affected by the global pitch.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(std::string const soundPath, int loops = 0, bool affectedByGlobalPitch = true) { int result = Create(loops, affectedByGlobalPitch); AddSound(soundPath); return result; }

		/// <summary>
		/// Adds a new Sound to this SoundContainer, loaded from a file.
		/// </summary>
		/// <param name="soundPath">A path to the new sound to add. This will be handled through PresetMan.</param>
		/// <param name="abortGameForInvalidSound">Whether to abort the game if the sound couldn't be added, or just show a console error. Default true.</param>
		void AddSound(std::string const soundPath, bool abortGameForInvalidSound = true);
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
		/// Saves the complete state of this SoundContainer to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the SoundContainer will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const { return 0; }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current list of sounds in the SoundContainer.
		/// </summary>
		/// <returns>A reference to the list.</returns>
		const std::vector<std::pair<ContentFile, FMOD::Sound *>> *GetSounds() const { return &m_Sounds; }

		/// <summary>
		/// Shows whether this SoundContainer has been initialized at all yet and loaded with any samples.
		/// </summary>
		/// <returns>Whether this sound has any samples.</returns>
		bool HasAnySounds() const { return GetSoundCount() > 0; }

		/// <summary>
		/// Gets the current number of sounds in this SoundContainer.
		/// </summary>
		/// <returns>An int with the count.</returns>
		int GetSoundCount() const { return m_Sounds.size(); }

		/// <summary>
		/// Indicates whether any sound in this SoundContainer is currently being played.
		/// </summary>
		/// <returns>Whether any sounds are playing.</returns>
		bool IsBeingPlayed() { return GetPlayingSoundCount() > 0; }

		/// <summary>
		/// Gets the number of currently playing sounds in this SoundContainer.
		/// </summary>
		/// <returns>Number of playing sounds in this SoundContainer.</returns>
		int GetPlayingSoundCount() { return m_PlayingChannels.size(); }

		/// <summary>
		/// Gets a vector of hashes of the sounds selected to be played next in this SoundContainer.
		/// </summary>
		/// <returns>The currently playing sounds hashes.</returns>
		std::vector<std::size_t> GetSelectedSoundHashes();

		/// <summary>
		/// Gets a vector of the sounds objects selected to be played next in this SoundContainer.
		/// </summary>
		/// <returns>The currently playing sound objects.</returns>
		std::vector<FMOD::Sound *> GetSelectedSoundObjects();

		/// <summary>
		/// Gets the channels playing sounds from this SoundContainer.
		/// </summary>
		/// <returns>The channels currently being used.</returns>
		std::unordered_set<unsigned short int> *GetPlayingChannels() { return &m_PlayingChannels; }

		/// <summary>
		/// Adds a channel index to the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to add.</param>
		void AddPlayingChannel(unsigned short int channel) { m_PlayingChannels.insert(channel); RTEAssert(m_PlayingChannels.size() <= c_MaxPlayingSoundsPerContainer, "Tried to play more than " + std::to_string(c_MaxPlayingSoundsPerContainer) + " sounds in SoundContainer " + GetPresetName()); }

		/// <summary>
		/// Removes a channel index from the SoundContainer's collection of playing channels.
		/// </summary>
		/// <param name="channel">The channel index to remove.</param>
		void RemovePlayingChannel(unsigned short int channel) { m_PlayingChannels.erase(channel); }

		/// <summary>
		/// Updates the distance attenuation of the SoundContainer's sounds while they're playing.
		/// </summary>
		/// <param name="attenuation">How much distance attenuation to apply to the sound. 0 = full volume 1.0 = max distant, but still won't be completely inaudible.</param>
		/// <returns>Whether this SoundContainer's attenuation setting was successful.</returns>
		bool UpdateAttenuation(float attenuation) { return g_AudioMan.SetSoundAttenuation(this, attenuation); }

		/// <summary>
		/// Gets the looping setting of this SoundContainer.
		/// </summary>
		/// <returns>An int with the loop count.</returns>
		int GetLoopSetting() const { return m_Loops; }

		/// <summary>
		/// Sets the looping setting of this SoundContainer. 
		/// 0 means the sound is set to only play once. -1 means it loops indefinitely.
		/// </summary>
		/// <param name="loops">An int with the loop count.</param>
		void SetLoopSetting(int loops);

		/// <summary>
		/// Gets the current playback priority.
		/// </summary>
		/// <returns>The playback priority.</returns>
		int GetPriority() const { return m_Priority; }

		/// <summary>
		/// Sets the current playback priority. Higher value will make this more likely to make it into mixing on playback.
		/// </summary>
		/// <param name="priority">The new priority. See AudioMan::PRIORITY_* enumeration.</param>
		void SetPriority(int priority) { m_Priority = Limit(priority, 255, 0); }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <returns>Whether or not the Sounds in this SoundContainer affected by global pitch changes.</returns>
		bool IsAffectedByGlobalPitch() const { return m_AffectedByGlobalPitch; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <param name="pitched">Whether the sounds in this SoundContainer should be affected by global pitch or not.</param>
		void SetAffectedByGlobalPitch(bool affectedByGlobalPitch) { m_AffectedByGlobalPitch = affectedByGlobalPitch; }
#pragma endregion

#pragma region Playback Controls
		/// <summary>
		/// Plays the next sound of this SoundContainer with default attenuation for all players.
		/// </summary>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play() { return Play(0, -1); }

		/// <summary>
		/// Plays the next sound of this SoundContainer with the given attenuation for all players.
		/// </summary>
		/// <param name="attenuation">How much distance attenuation to apply to the SoundContainer.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(float attenuation) { return Play(attenuation, -1); }

		/// <summary>
		/// Plays the next sound of this SoundContainer with the given attenuation for a specific player.
		/// </summary>
		/// <param name="attenuation">How much distance attenuation to apply to the SoundContainer.</param>
		/// <param name="player">Player to start playback of this SoundContainer for.</param>
		/// <returns>Whether this SoundContainer successfully started playing on any channels.</returns>
		bool Play(float attenuation, int player) { return HasAnySounds() ? g_AudioMan.PlaySound(this, attenuation, player) : false; }

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
		bool SelectNextSounds();

		/// <summary>
		/// Fades out playback of the SoundContainer to 0 volume.
		/// </summary>
		/// <param name="fadeOutTime">How long the fadeout should take.</param>
		void FadeOut(int fadeOutTime = 1000) { if (IsBeingPlayed()) { return g_AudioMan.FadeOutSound(this, fadeOutTime); } }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::vector<std::pair<ContentFile, FMOD::Sound *>> m_Sounds; // All the FMOD Sound objects in this SoundContainer, paired with their appropriate ContentFile. The sound objects within are owned by the ContentFile static maps
		std::vector<size_t> m_SelectedSounds; //!< Vector of the indices of all selected sounds

		std::unordered_set<unsigned short int> m_PlayingChannels; //!< The channels this SoundContainer is currently using

		int m_Loops; //!< Number of loops (repeats) the SoundContainer's sounds should play when played. 0 means it plays once, -1 means it plays until stopped 
		int m_Priority; //!< The mixing priority of this SoundContainer's sounds. Higher values are more likely to be heard
		bool m_AffectedByGlobalPitch; //!< Whether this SoundContainer's sounds should be able to be altered by global pitch changes

	private:
		/// <summary>
		/// Clears all the member variables of this SoundContainer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif