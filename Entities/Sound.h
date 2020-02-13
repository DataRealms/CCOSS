#ifndef _RTESOUND_
#define _RTESOUND_

#include "Entity.h"
#include "ContentFile.h"

namespace RTE {

	/// <summary>
	/// One or more sound samples that all represent a specific sound effect.
	/// </summary>
	class Sound : public Entity {
		friend class AudioMan;
		friend class GUISound;
		
	public:
		ENTITYALLOCATION(Sound)
		CLASSINFOGETTERS

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Sound object in system memory. Create() should be called before using the object.
		/// </summary>
		Sound() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a Sound object identical to an already existing one.
		/// </summary>
		/// <param name="reference"></param>
		Sound(const Sound &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Creates a Sound to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Sound to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Sound &reference);

		/// <summary>
		/// Creates a Sound and give it a path to its first sample.
		/// </summary>
		/// <param name="samplePath">A path to the sound sample for this sound to have.</param>
		/// <param name="pitched">Whether this sound's frequency will be pitched at all.</param>
		/// <param name="loops">The number of times this sound will loop. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(std::string samplePath, bool pitched = true, int loops = 0);

		/// <summary>
		/// Adds a new sample to this Sound's collection, loaded from a file.
		/// </summary>
		/// <param name="samplePath">A path to the new sample to add. This will be handled through PresetMan.</param>
		void AddSample(std::string samplePath);
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
		/// Saves the complete state of this Sound to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the Sound will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Sound object before deletion from system memory.
		/// </summary>
		virtual ~Sound() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Sound object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Resets the entire Sound, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current list of samples in the Sound.
		/// </summary>
		/// <returns>A reference to the list.</returns>
		const std::vector<std::pair<ContentFile, AUDIO_STRUCT *> > & GetSampleList() const { return m_Samples; }

		/// <summary>
		/// Shows whether this Sound has been initialized at all yet and loaded with any samples.
		/// </summary>
		/// <returns>Whether this Sound has any samples.</returns>
		bool HasAnySamples() const { return !m_Samples.empty(); }

		/// <summary>
		/// Gets the current number of Samples that make up the Sound.
		/// </summary>
		/// <returns>An int with the count.</returns>
		int GetSampleCount() const { return m_Samples.size(); }

		/// <summary>
		/// Indicates whether this Sound is currently being played.
		/// </summary>
		/// <returns>Whether this Sound is being played on a channel.</returns>
		bool IsBeingPlayed();

		/// <summary>
		/// Gets the currently played sample, if any.
		/// </summary>
		/// <returns>A pointer to the currently played sample, or 0 if no sample of this Sound is currently being played. Ownership is NOT transferred.</returns>
		AUDIO_STRUCT * GetCurrentSample();

		/// <summary>
		/// Gets current channel that one of the samples of this Sound is being played on.
		/// </summary>
		/// <returns>The current channel. -1 if none or not being played.</returns>
		int GetCurrentChannel() const { return m_LastChannel; }

		/// <summary>
		/// Sets the channel that this Sound will be played on.
		/// </summary>
		/// <param name="channel">A number of the new channel. -1 for no channel.</param>
		void SetChannel(int channel) { m_LastChannel = channel; }

		/// <summary>
		/// Gets the looping setting of this Sound.
		/// </summary>
		/// <returns>An int with the loop count.</returns>
		int GetLoopSetting() const { return m_Loops; }

		/// <summary>
		/// Sets the looping setting of this Sound. 
		/// 0 means the sound is set to only play once. -1 means it loops indefinitely.
		/// </summary>
		/// <param name="loops">An int with the loop count.</param>
		void SetLoopSetting(int loops) { m_Loops = loops; }

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
		/// Sets whether this is affected by (global) pitch changes or not.
		/// </summary>
		/// <param name="pitched">Whether this should be affected by pitch or not.</param>
		void SetAffectedByPitch(bool pitched = true) { m_AffectedByPitch = pitched; }

		/// <summary>
		/// Updates the distance attenuation of the sound while it's playing.
		/// </summary>
		/// <param name="distance">
		/// How much distance attenuation to apply to the sound. 0 = full volume 1.0 = max distant, but still won't be completely inaudible.</param>
		/// <returns>Whether this Sound's attenuation setting was successful.</returns>
		bool UpdateAttenuation(float distance = 0);
#pragma endregion	

#pragma region Playback Controls
		/// <summary>
		/// Plays the next sample of this Sound for a specific player.
		/// </summary>
		/// <param name="distance">How much distance attenuation to apply to the sound.</param>
		/// <param name="player">Player to start playback of this sound for.</param>
		/// <returns>Whether this Sound successfully started playing on a channel.</returns>
		bool Play(float distance = 0, int player = -1);

		/// <summary>
		/// Stops playback of this Sound for a specific player.
		/// </summary>
		/// <param name="player">Player to stop playback of this sound for.</param>
		/// <returns>Whether this Sound successfully stopped playing.</returns>
		bool Stop(int player = -1);

		/// <summary>
		/// Gets and sets the next sample of this Sound to be played.
		/// It's a random selection of all the samples of this Sound.
		/// Note that this will change each time this method is called.
		/// </summary>
		/// <returns>A pointer to the sample to be played. Ownership is NOT transferred.</returns>
		AUDIO_STRUCT * StartNextSample();

		/// <summary>
		/// Fades out playback of the currently played sound.
		/// </summary>
		/// <param name="fadeOutTime">The time, in ms, of how long to fade out for.</param>
		void FadeOut(int fadeOutTime = 1000);
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; ///! ClassInfo for this class.

		// All the samples of this Sound, they are NOT owned by this, but owned by the ContentFile static maps 
		std::vector<std::pair<ContentFile, AUDIO_STRUCT *> > m_Samples;

		int m_CurrentSample; ///! Index of the current (or last, if nothing is being played) sample being played
		int m_LastChannel; ///! Current channel the current Sample of this Sound is being played on.
		int m_Loops; ///! Number of loops (repeats) the sound should play when played. 0 means it plays once, -1 means it plays until stopped 
		int m_Priority; ///! The mixing priority of this, the higher the more likely it's to be mixed and heard
		bool m_AffectedByPitch; ///! Whether this should be altered by pitch
		size_t m_Hash; ///! Sound file hash for network transmission.

		/// <summary>
		/// Clears all the member variables of this Sound, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif