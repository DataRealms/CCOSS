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
		/// Creates a SoundContainer and gives it a path to its first sample.
		/// </summary>
		/// <param name="soundPath">A path to the sound for this sound to have.</param>
		/// <param name="affectedByPitch">Whether this SoundContainer's sounds' frequency will be affected by global pitch</param>
		/// <param name="loops">The number of times this SoundContainer's sounds will loop. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(std::string soundPath, bool affectedByPitch = true, int loops = 0);

		/// <summary>
		/// Adds a new Sound to this SoundContainer, loaded from a file.
		/// </summary>
		/// <param name="soundPath">A path to the new sound to add. This will be handled through PresetMan.</param>
		void AddSound(std::string soundPath);
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
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SoundContainer object before deletion from system memory.
		/// </summary>
		virtual ~SoundContainer() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SoundContainer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Resets the entire SoundContainer, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current list of sounds in the SoundContainer.
		/// </summary>
		/// <returns>A reference to the list.</returns>
		std::vector<std::pair<ContentFile, FMOD::Sound *>> GetSounds() const { return m_Sounds; }

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
		/// Gets the index in the internal sound group of the currently playing sound in this SoundContainer.
		/// </summary>
		/// <returns>The currently playing sound.</returns>
		int GetCurrentSound() { return m_CurrentSound; }

		/// <summary>
		/// Gets the channels playing sounds from this SoundContainer.
		/// </summary>
		/// <returns>The channels currently being used.</returns>
		std::unordered_set<short int> GetPlayingChannels() { return m_PlayingChannels; }

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
		void SetPriority(int priority) { m_Priority = priority; }

		/// <summary>
		/// Gets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <returns>Whether or not the Sounds in this SoundContainer affected by global pitch changes.</returns>
		bool IsAffectedByPitch() const { return m_AffectedByPitch; }

		/// <summary>
		/// Sets whether the sounds in this SoundContainer are affected by global pitch changes or not.
		/// </summary>
		/// <param name="pitched">Whether the sounds in this SoundContainer should be affected by global pitch or not.</param>
		void SetAffectedByPitch(bool pitched = true) { m_AffectedByPitch = pitched; }

		/// <summary>
		/// Updates the distance attenuation of the SoundContainer's sounds while they're playing.
		/// </summary>
		/// <param name="distance">
		/// How much distance attenuation to apply to the sound. 0 = full volume 1.0 = max distant, but still won't be completely inaudible.</param>
		/// <returns>Whether this SoundContainer's attenuation setting was successful.</returns>
		bool UpdateAttenuation(float distance = 0) { return g_AudioMan.SetSoundAttenuation(this, distance); }

		/// <summary>
		/// Gets the hash for this SoundContainer, used for multiplayer data
		/// </summary>
		/// <returns>The hash for this SoundContainer</returns>
		size_t GetHash() { return m_Hash; }
#pragma endregion

#pragma region Playback Controls
		/// <summary>
		/// Plays the next sound of this SoundContainer for a specific player.
		/// </summary>
		/// <param name="distance">How much distance attenuation to apply to the SoundContainer.</param>
		/// <param name="player">Player to start playback of this SoundContainer for.</param>
		/// <returns>Whether this SoundContainer successfully started playing on a channel.</returns>
		bool Play(float distance = 0, int player = -1) { return HasAnySounds() ? g_AudioMan.PlaySound(this, distance, player) : false; }

		/// <summary>
		/// Stops playback of this SoundContainer for a specific player.
		/// </summary>
		/// <param name="player">Player to stop playback of this SoundContainer for.</param>
		/// <returns>Whether this SoundContainer successfully stopped playing.</returns>
		bool Stop(int player = -1) { return HasAnySounds() ? g_AudioMan.StopSound(player, this) : false; }

		/// <summary>
		/// Selects, saves and returns the next sound of this SoundContainer to be played.
		/// </summary>
		/// <returns>A pointer to the sound to be played. Ownership is NOT transferred.</returns>
		FMOD::Sound * SelectNextSound();

		/// <summary>
		/// Fades out playback of the SoundContainer to 0 volume.
		/// </summary>
		/// <param name="fadeOutTime">How long the fadeout should take.</param>
		void FadeOut(int fadeOutTime = 1000);
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::vector<std::pair<ContentFile, FMOD::Sound *>> m_Sounds; // All the FMOD Sound objects in this SoundContainer, paired with their appropriate ContentFile. The sound objects within are owned by the ContentFile static maps

		//TODO change this to be a list/vector of sounds
		int m_CurrentSound; //!< Sound group index of the current (or last played if nothing is being played) sound being played
		std::unordered_set<short int> m_PlayingChannels; //!< The channels this SoundContainer is currently using

		int m_Loops; //!< Number of loops (repeats) the SoundContainer's sounds should play when played. 0 means it plays once, -1 means it plays until stopped 
		int m_Priority; //!< The mixing priority of this SoundContainer's sounds. Higher values are more likely to be heard
		bool m_AffectedByPitch; //!< Whether this SoundContainer's sounds should be able to be altered by pitch changes

		size_t m_Hash; //!< Sound file hash for network transmission.

	private:
		/// <summary>
		/// Clears all the member variables of this SoundContainer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif