#ifndef _RTEAUDIOMAN_
#define _RTEAUDIOMAN_

#include "RTETools.h"
#include "Entity.h"
#include "Timer.h"
#include "Singleton.h"

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

struct AUDIO_STRUCT;

#define g_AudioMan AudioMan::Instance()

namespace RTE {
	class Sound;

	/// <summary>
	/// The singleton manager of the WAV sound effects and OGG music playback.
	/// </summary>
	class AudioMan : public Singleton<AudioMan> {

	public:
		CLASSINFOGETTERS

		enum PlaybackPriority {
			PRIORITY_LOW = 0,
			PRIORITY_HIGH = 100,
			PRIORITY_NOATTENUATION,
			PRIORITY_COUNT
		};

		enum NetworkSoundState {
			SOUND_PLAY = 0,
			SOUND_STOP,
			SOUND_SET_PITCH,
			SOUND_SET_ATTENUATION
		};

		struct NetworkSoundData {
			unsigned char State;
			size_t SoundHash;
			unsigned char AffectedByPitch;
			short int Distance;
			short int Channel;
			short int Loops;
			float Pitch;
		};

		enum NetworkMusicState {
			MUSIC_PLAY = 0,
			MUSIC_STOP,
			MUSIC_SILENCE,
			MUSIC_SET_PITCH
		};

		struct NetworkMusicData {
			unsigned char State;
			char Path[256];
			int Loops;
			double Position;
			float Pitch;
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a AudioMan object in system memory.
		/// Create() should be called before using the object.
		/// </summary>
		AudioMan() { Clear(); }

		/// <summary>
		/// Makes the AudioMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a AudioMan object before deletion from system memory.
		/// </summary>
		~AudioMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the AudioMan object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Standard Methods
		/// <summary>
		/// Resets the entire AudioMan including it's inherited members to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }

		/// <summary>
		/// Updates the state of this AudioMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Reports whether audio is enabled.
		/// </summary>
		/// <returns>Whether audio is enabled.</returns>
		bool IsAudioEnabled() { return m_AudioEnabled; }

		/// <summary>
		/// Reports whether a certain Sound's last played sample is being played currently.
		/// </summary>
		/// <param name="pSound">A pointer to a Sound object. Ownership IS NOT transferred!</param>
		/// <returns>Whether the LAST sample that was played of the Sound is currently being played by any of the channels.</returns>
		bool IsPlaying(Sound *pSound);

		/// <summary>
		/// Reports whether any music stream is currently playing.
		/// </summary>
		/// <returns>Whether any music stream is currently playing.</returns>
		bool IsMusicPlaying() { return m_AudioEnabled && m_pMusic; }

		/// <summary>
		/// Gets the volume of all sounds. Does not get volume of music.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		double GetSoundsVolume() const { return m_SoundsVolume; }

		/// <summary>
		/// Sets the volume of all sounds to a specific volume. Does not affect music.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetSoundsVolume(double volume = 1.0);

		/// <summary>
		/// Gets the volume of music. Does not get volume of sounds.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		double GetMusicVolume() const { return m_MusicVolume; }

		/// <summary>
		/// Sets the music to a specific volume. Does not affect sounds.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetMusicVolume(double volume = 1.0);

		/// <summary>
		/// Sets the music to a specific volume, but it will only last until a new song is played. Useful for fading etc.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetTempMusicVolume(double volume = 1.0);

		/// <summary>
		/// Gets the global pitch scalar value for all sounds and music.
		/// </summary>
		/// <returns>The current pitch scalar. Will be > 0.</returns>
		double GetGlobalPitch() const { return m_GlobalPitch; }

		/// <summary>
		/// Sets the global pitch multiplier for all sounds, optionally the music too.
		/// </summary>
		/// <param name="pitch">The desired pitch multiplier. Keep it > 0.</param>
		/// <param name="excludeMusic">Whether to exclude the music from pitch modification</param>
		void SetGlobalPitch(double pitch = 1.0, bool excludeMusic = false);

		/// <summary>
		/// Sets/updates the distance attenuation for a specific sound. Will only have an effect if the sound is currently being played.
		/// </summary>
		/// <param name="pSound">A pointer to a Sound object. Ownership IS NOT transferred!</param>
		/// <param name="distance">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not completely inaudible.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the attenuation was successfully set.</returns>
		bool SetSoundAttenuation(Sound *pSound, float distance = 0.0);

		/// <summary>
		/// Sets/updates the frequency/pitch for a specific sound. Will only have an effect if the sound is currently being played.
		/// </summary>
		/// <param name="pSound">A pointer to a Sound object. Ownership IS NOT transferred!</param>
		/// <param name="pitch">New pitch, a multiplier of the original normal frequency. Keep it > 0.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the pitch was successfully set.</returns>
		bool SetSoundPitch(Sound *pSound, float pitch);

		/// <summary>
		/// Sets/updates the frequency/pitch for the music channel.
		/// </summary>
		/// <param name="pitch">New pitch, a multiplier of the original normal frequency. Keep it > 0.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the pitch was successfully set.</returns>
		bool SetMusicPitch(float pitch);

		/// <summary>
		/// Gets the path of the last played music stream.
		/// </summary>
		/// <returns>The file path of the last played music stream.</returns>
		std::string GetMusicPath() const { return m_MusicPath; }

		/// <summary>
		/// Gets the position of playback of the current music stream, in seconds.
		/// </summary>
		/// <returns>The current position of the current stream playing, in seconds.</returns>
		double GetMusicPosition();

		/// <summary>
		/// Sets the music to a specific position in the song.
		/// </summary>
		/// <param name="position">The desired position from the start, in seconds.</param>
		void SetMusicPosition(double position);

		/// <summary>
		/// Returns the number of audio channels currently used.
		/// </summary>
		/// <returns>The number of audio channels currently used.</returns>
		int GetPlayingChannelCount();

		/// <summary>
		/// Returns the number of audio channels available in total.
		/// </summary>
		/// <returns>The number of audio channels available in total.</returns>
		int GetTotalChannelCount();
#pragma endregion

#pragma region Playback Handling
		/// <summary>
		/// Starts playing a certain sound sample.
		/// </summary>
		/// <param name="wavefile">The path to the .wav file to play.</param>
		void PlaySound(const char *wavefile);

		/// <summary>
		/// Starts playing the next sample of a certain Sound.
		/// </summary>
		/// <param name="pSound">Pointer to the Sound to start playing. Ownership is NOT transferred!</param>
		/// <param name="priority">The priority of this sound. Higher gives it a higher likelihood of getting mixed compared to lower-priority samples.</param>
		/// <param name="distance">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not completely inaudible.</param>
		/// <param name="pitch">The pitch modifier for this sound. 1.0 yields unmodified frequency.</param>
		/// <returns>Whether or not playback of the Sound was successful.</returns>
		bool PlaySound(Sound *pSound, int priority = PRIORITY_LOW, float distance = 0.0, double pitch = 1.0);

		/// <summary>
		/// Starts playing the next sample of a certain Sound for a certain player.
		/// </summary>
		/// <param name="player">Which player to play the Sound sample for.</param>
		/// <param name="pSound">Pointer to the Sound to start playing. Ownership is NOT transferred!</param>
		/// <param name="priority">The priority of this sound. Higher gives it a higher likelihood of getting mixed compared to lower-priority samples.</param>
		/// <param name="distance">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.</param>
		/// <param name="pitch">The pitch modifier for this sound. 1.0 yields unmodified frequency.</param>
		/// <returns>Whether or not playback of the Sound was successful.</returns>
		bool PlaySound(int player, Sound *pSound, int priority = PRIORITY_LOW, float distance = 0.0, double pitch = 1.0);

		/// <summary>
		/// Starts playing a certain WAVE sound file.
		/// </summary>
		/// <param name="filepath">The path to the music file to play.</param>
		/// <param name="distance">Normalized distance from 0 to 1 to play the sound with.</param>
		/// <param name="loops">The number of times to loop the sound. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="affectedByPitch">Whether the sound should be affected by pitch.</param>
		/// <param name="player">For which player to play the sound for, -1 for all.</param>
		/// <returns>Returns the new sound object being played. OWNERSHIP IS TRANSFERRED!</returns>
		Sound * PlaySound(const char *filepath, float distance, bool loops, bool affectedByPitch, int player);

		/// <summary>
		/// Starts playing a certain WAVE, MOD, MIDI, OGG, MP3 file in the music channel.
		/// </summary>
		/// <param name="filepath">The path to the music file to play.</param>
		/// <param name="loops">The number of times to loop the song. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="volumeOverride">The volume override for music for this song only. < 0 means no override.</param>
		void PlayMusic(const char *filepath, int loops = -1, double volumeOverride = -1.0);

		/// <summary>
		/// Queues up another path to a stream that will be played after the current one is done. 
		/// Can be done several times to queue up many tracks.
		/// The last track in the list will be looped infinitely.
		/// </summary>
		/// <param name="filepath">The path to the music file to play after the current one.</param>
		void QueueMusicStream(const char *filepath);

		/// <summary>
		/// Plays the next music stream in the queue, if any is queued.
		/// </summary>
		void PlayNextStream();

		/// <summary>
		/// Queues up a period of silence in the music stream playlist.
		/// </summary>
		/// <param name="seconds">The number of secs to wait before going to the next stream.</param>
		void QueueSilence(int seconds);

		/// <summary>
		/// Clears the music queue.
		/// </summary>
		void ClearMusicQueue() { m_MusicPlayList.clear(); }

		/// <summary>
		/// Stops all playback.
		/// </summary>
		void StopAll();

		/// <summary>
		/// Stops playing a the music channel.
		/// </summary>
		void StopMusic();

		/// <summary>
		/// Stops playing a certain Sound sample.
		/// </summary>
		/// <param name="pSound">Pointer to the Sound to stop playing. Ownership is NOT transferred!</param>
		/// <returns>Whether playback of the sound was successfully stopped, or even found.</returns>
		bool StopSound(Sound *pSound);

		/// <summary>
		/// Stops playing a certain Sound sample for a certain player.
		/// </summary>
		/// <param name="player">Which player to stop the Sound sample for.</param>
		/// <param name="pSound">Pointer to the Sound to stop playing. Ownership is NOT transferred!</param>
		/// <returns></returns>
		bool StopSound(int player, Sound *pSound);

		/// <summary>
		/// Fades out playback of a specific sound.
		/// </summary>
		/// <param name="pSound">Pointer to the Sound to fade out playing. Ownership is NOT transferred!</param>
		/// <param name="fadeOutTime"> The amount of time, in ms, to fade out over.</param>
		void FadeOutSound(Sound *pSound, int fadeOutTime = 1000);
#pragma endregion

#pragma region Network Audio Handling
		/// <summary>
		/// Returns true if manager is in multiplayer mode.
		/// </summary>
		/// <returns>True if in multiplayer mode.</returns>
		bool IsInMultiplayerMode() { return m_IsInMultiplayerMode; }

		/// <summary>
		/// Sets the multiplayer mode flag.
		/// </summary>
		/// <param name="value">Whether this manager should operate in multiplayer mode.</param>
		void SetMultiplayerMode(bool value) { m_IsInMultiplayerMode = value; }

		/// <summary>
		/// Fills the list with sound events happened for the specified network player.
		/// </summary>
		/// <param name="player">Player to get events for.</param>
		/// <param name="list">List with events for this player.</param>
		void GetSoundEvents(int player, std::list<NetworkSoundData> & list);

		/// <summary>
		/// Adds the sound event to internal list of sound events for the specified player.
		/// </summary>
		/// <param name="player">Player for which the event happened.</param>
		/// <param name="state">Sound state.</param>
		/// <param name="hash">Sound file hash to transmit to client.</param>
		/// <param name="distance">Sound distance.</param>
		/// <param name="channel">Channel where sound was played.</param>
		/// <param name="loops">Loops counter.</param>
		/// <param name="pitch">Pitch value.</param>
		/// <param name="affectedByPitch">Whether the sound is affected by pitch.</param>
		void RegisterSoundEvent(int player, unsigned char state, size_t hash, short int distance, short int channel, short int loops, float pitch, bool affectedByPitch);

		/// <summary>
		/// Fills the list with music events happened for the specified network player.
		/// </summary>
		/// <param name="player">Player to get events for.</param>
		/// <param name="list">List with events for this player.</param>
		void GetMusicEvents(int player, std::list<NetworkMusicData> & list);

		/// <summary>
		/// Adds the music event to internal list of music events for the specified player.
		/// </summary>
		/// <param name="player">Player for which the event happened.</param>
		/// <param name="state">Music state.</param>
		/// <param name="filepath">Music file path to transmit to client.</param>
		/// <param name="loops">Loops counter.</param>
		/// <param name="position">Music playback position.</param>
		/// <param name="pitch">Pitch value.</param>
		void RegisterMusicEvent(int player, unsigned char state, const char *filepath, int loops, double position, float pitch);
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //! ClassInfo for this class.
		static const std::string m_ClassName; //! A string with the friendly-formatted type name of this object.

		bool m_AudioEnabled; //! Bool to tell whether audio is enabled or not.
		int m_MusicChannel; //! Channel number for Music streams. Typically 0. 

#ifdef __USE_SOUND_FMOD
		static constexpr unsigned short int s_MaxVolume = 255; //! Maximum value to use for volume control.
		FSOUND_STREAM *m_pMusic; //! Pointer to Fmod sound stream data structure.

#elif __USE_SOUND_GORILLA
		static constexpr float s_MaxVolume = 1.0;  //! Maximum value to use for volume control.
		gau_Manager* m_pManager; //! Pointer to Gorilla Audio manager and associated functions.
		ga_Mixer* m_pMixer; //! Pointer to Gorilla Audio mixer data structure.
		ga_StreamManager* m_pStreamManager; //! Pointer to Gorilla Audio stream manager data structure.
		ga_Handle * m_pMusic; //! Pointer to Gorilla Audio playback control handle data structure.
		std::vector<ga_Handle *> m_SoundChannels; //! Vector containing active sound channels.
		std::vector<ga_Sound *> m_SoundInstances; //! Vector containing Sound instances.
#endif
		double m_SoundsVolume; //! Global sounds effects volume.
		double m_MusicVolume; //! Global music volume.

		double m_GlobalPitch; //! Global pitch multiplier.
		std::vector<int> m_NormalFrequencies; //! The 'normal' unpitched frequency of each channel handle we have.
		std::vector<double> m_PitchModifiers; //! How each channel's pitch is modified individually.

		std::string m_MusicPath; //! The path to the last played music stream.
		std::list<std::string> m_MusicPlayList; //! Playlist of paths to music to play after the current non looping one is done.

		Timer m_SilenceTimer; //! Timer for measuring silences between songs.


		bool m_IsInMultiplayerMode; //! If true then the server is in multiplayer mode and will register sound and music events into internal lists.
		std::list<NetworkSoundData> m_SoundEvents[c_MaxClients]; //! Lists of per player sound events.
		std::list<NetworkMusicData> m_MusicEvents[c_MaxClients]; //! Lists of per player music events.

	private:
		/// <summary>
		/// Clears all the member variables of this AudioMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		AudioMan(const AudioMan &reference);
		AudioMan & operator=(const AudioMan &rhs);
	};
}
#endif