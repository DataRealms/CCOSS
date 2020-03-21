#ifndef _RTEAUDIOMAN_
#define _RTEAUDIOMAN_

#include "Constants.h"
#include "RTETools.h"
#include "Entity.h"
#include "Timer.h"
#include "Singleton.h"

#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

#define g_AudioMan AudioMan::Instance()

namespace RTE {
	class SoundContainer;

	/// <summary>
	/// The singleton manager of the WAV sound effects and OGG music playback.
	/// </summary>
	class AudioMan : public Singleton<AudioMan> {

	public:
		CLASSINFOGETTERS

		enum PlaybackPriority {
			PRIORITY_HIGH = 0,
			PRIORITY_NORMAL = 128,
			PRIORITY_LOW = 256,
			PRIORITY_COUNT
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

		enum NetworkSoundState {
			SOUND_PLAY = 0,
			SOUND_STOP,
			SOUND_SET_ATTENUATION,
			SOUND_SET_PITCH,
			SOUND_SET_GLOBAL_PITCH,
			SOUND_FADE_OUT
		};

		struct NetworkSoundData {
			unsigned char State;
			unsigned short int Channels[c_MaxPlayingSoundsPerContainer]{c_NumberOfAudioChannels + 1};
			size_t SoundFileHashes[c_NumberOfAudioChannels]{0};
			short int Distance;
			short int Loops;
			float Pitch;
			bool AffectedByGlobalPitch;
			short int FadeOutTime;
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

#pragma region General Getters and Setters
		/// <summary>
		/// Gets the sound management system object used for playing every sound
		/// </summary>
		/// <returns>The sound management system object used by AudioMan for playing audio</returns>
		FMOD::System *GetAudioSystem() { return m_AudioSystem; }

		/// <summary>
		/// Reports whether audio is enabled.
		/// </summary>
		/// <returns>Whether audio is enabled.</returns>
		bool IsAudioEnabled() { return m_AudioEnabled; }

		/// <summary>
		/// Returns the number of audio channels currently used.
		/// </summary>
		/// <returns>The number of audio channels currently used.</returns>
		int GetPlayingChannelCount() { int channelCount; return m_AudioSystem->getChannelsPlaying(&channelCount) == FMOD_OK ? channelCount : 0; }

		/// <summary>
		/// Returns the number of audio channels available in total.
		/// </summary>
		/// <returns>The number of audio channels available in total.</returns>
		int GetTotalChannelCount() { int channelCount; return m_AudioSystem->getSoftwareChannels(&channelCount) == FMOD_OK ? channelCount : 0; }

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
#pragma endregion

#pragma region Music Getters and Setters
		/// <summary>
		/// Reports whether any music stream is currently playing.
		/// </summary>
		/// <returns>Whether any music stream is currently playing.</returns>
		bool IsMusicPlaying() { bool isPlayingMusic; return m_AudioEnabled && m_MusicChannelGroup->isPlaying(&isPlayingMusic) == FMOD_OK ? isPlayingMusic : false; }

		/// <summary>
		/// Gets the volume of music. Does not get volume of sounds.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		double GetMusicVolume() const { return m_MusicVolume; }

		/// <summary>
		/// Sets the music to a specific volume. Does not affect sounds.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetMusicVolume(double volume = 1.0) { m_MusicVolume = Limit(volume, 1, 0); if (m_AudioEnabled) { m_MusicChannelGroup->setVolume(m_MusicVolume); } }

		/// <summary>
		/// Sets the music to a specific volume, but it will only last until a new song is played. Useful for fading etc.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetTempMusicVolume(double volume = 1.0);

		/// <summary>
		/// Gets the path of the last played music stream.
		/// </summary>
		/// <returns>The file path of the last played music stream.</returns>
		std::string GetMusicPath() const { return m_MusicPath; }

		/// <summary>
		/// Sets/updates the frequency/pitch for the music channel.
		/// </summary>
		/// <param name="pitch">New pitch, a multiplier of the original normal frequency. Keep it > 0.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the pitch was successfully set.</returns>
		bool SetMusicPitch(float pitch);

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
#pragma endregion

#pragma region Sound Getters and Setters
		/// <summary>
		/// Gets the volume of all sounds. Does not get volume of music.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		double GetSoundsVolume() const { return m_SoundsVolume; }

		/// <summary>
		/// Sets the volume of all sounds to a specific volume. Does not affect music.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetSoundsVolume(double volume = 1.0) { m_SoundsVolume = volume; if (m_AudioEnabled) { m_SoundChannelGroup->setVolume(volume); } }

		/// <summary>
		/// Sets/updates the distance attenuation for a specific SoundContainer. Will only have an effect if the sound is currently being played.
		/// </summary>
		/// <param name="pSound">A pointer to a Sound object. Ownership IS NOT transferred!</param>
		/// <param name="distance">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not completely inaudible.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the attenuation was successfully set.</returns>
		bool SetSoundAttenuation(SoundContainer *pSound, float distance = 0.0);

		/// <summary>
		/// Sets/updates the frequency/pitch for a specific sound. Will only have an effect if the sound is currently being played.
		/// </summary>
		/// <param name="pSound">A pointer to a Sound object. Ownership IS NOT transferred!</param>
		/// <param name="pitch">New pitch, a multiplier of the original normal frequency. Keep it > 0.</param>
		/// <returns>Whether a sample of the Sound is currently being played by any of the channels, and the pitch was successfully set.</returns>
		bool SetSoundPitch(SoundContainer *pSound, float pitch);
#pragma endregion

#pragma region Global Playback and Handling
		/// <summary>
		/// Stops all playback and clears the music playlist.
		/// </summary>
		void StopAll() { if (m_AudioEnabled) { m_MasterChannelGroup->stop(); } m_MusicPlayList.clear(); }
#pragma endregion

#pragma region Music Playback and Handling
		/// <summary>
		/// Starts playing a certain WAVE, MOD, MIDI, OGG, MP3 file in the music channel.
		/// </summary>
		/// <param name="filePath">The path to the music file to play.</param>
		/// <param name="loops">The number of times to loop the song. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="volumeOverrideIfNotMuted">The volume override for music for this song only, if volume is not muted. < 0 means no override.</param>
		void PlayMusic(const char *filePath, int loops = -1, double volumeOverrideIfNotMuted = -1.0);

		/// <summary>
		/// Plays the next music stream in the queue, if any is queued.
		/// </summary>
		void PlayNextStream();

		/// <summary>
		/// Stops playing a the music channel.
		/// </summary>
		void StopMusic();

		/// <summary>
		/// Queues up another path to a stream that will be played after the current one is done. 
		/// Can be done several times to queue up many tracks.
		/// The last track in the list will be looped infinitely.
		/// </summary>
		/// <param name="filePath">The path to the music file to play after the current one.</param>
		void QueueMusicStream(const char *filePath);

		/// <summary>
		/// Queues up a period of silence in the music stream playlist.
		/// </summary>
		/// <param name="seconds">The number of secs to wait before going to the next stream.</param>
		void QueueSilence(int seconds);

		/// <summary>
		/// Clears the music queue.
		/// </summary>
		void ClearMusicQueue() { m_MusicPlayList.clear(); }
#pragma endregion

#pragma region Sound Playback and Handling
		/// <summary>
		/// Starts playing a certain sound file.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <returns>Returns the new sound object being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath) { return PlaySound(filePath, 0, -1); }

		/// <summary>
		/// Starts playing a certain sound file with a certain attenuation for a certain player.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <param name="attenuation">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players.</param>
		/// <returns></returns>
		SoundContainer *PlaySound(const char *filePath, float attenuation, int player) { return PlaySound(filePath, attenuation, player, 0, PRIORITY_NORMAL, -1); }

		/// <summary>
		/// Starts playing a certain sound file with various configuration settings.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <param name="attenuation">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players.</param>
		/// <param name="loops">The number of times to loop the SoundContainer's sounds. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="priority">The priority of this sound from 256 (lowest) to 0 (highest). Higher priority sounds are more likely to be heard.</param>
		/// <param name="pitchOrAffectedByGlobalPitch">
		/// The pitch to play this SoundContainer's at where 1 is unmodified frequency and each multiple of 2 is an octave up or down.
		/// -1 means the SoundContainer will be affected by global pitch instead of setting handling its pitch manually. Defaults to -1.
		/// </param>
		/// <returns>Returns the new sound object being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath, float attenuation, int player, int loops, int priority, double pitchOrAffectedByGlobalPitch);

		/// <summary>
		/// Starts playing the next sample of a certain SoundContainer for a certain player.
		/// </summary>
		/// <param name="pSound">Pointer to the SoundContainer to start playing. Ownership is NOT transferred!</param>
		/// <param name="attenuation">Distance attenuation scalar: 0 = full volume, 1.0 = max distant, but not silent. Defaults to 0.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players. Defaults to -1.</param>
		/// <param name="priority">The priority of this sound - higher priority sounds are more likely to be heard. -1 means it'll use the SoundContainer's value. Defaults to -1.</param>
		/// <param name="pitch">/// The pitch to play this SoundContainer's at where 1 is unmodified frequency and each multiple of 2 is an octave up or down. Defaults to 1.</param>
		/// <returns>Whether or not playback of the Sound was successful.</returns>
		bool PlaySound(SoundContainer *pSoundContainer, float attenuation = 0, int player = -1, int priority = -1, double pitch = 1);

		/// <summary>
		/// Stops playing all sounds in a given SoundContainer.
		/// </summary>
		/// <param name="pSound">Pointer to the SoundContainer to stop playing. Ownership is NOT transferred!</param>
		/// <returns>True if sounds were playing and were stopped, false otherwise.</returns>
		bool StopSound(SoundContainer *pSound) { return StopSound(pSound, -1); }

		/// <summary>
		/// Stops playing all sounds in a given SoundContainer for a certain player.
		/// </summary>
		/// <param name="player">Which player to stop the SoundContainer  for.</param>
		/// <param name="pSound">Pointer to the SoundContainer to stop playing. Ownership is NOT transferred!</param>
		/// <returns></returns>
		bool StopSound(SoundContainer *pSound, int player);

		/// <summary>
		/// Fades out playback of all sounds in a specific SoundContainer.
		/// </summary>
		/// <param name="pSound">Pointer to the Sound to fade out playing. Ownership is NOT transferred!</param>
		/// <param name="fadeOutTime">The amount of time, in ms, to fade out over.</param>
		void FadeOutSound(SoundContainer *pSound, int fadeOutTime = 1000);
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
		/// Fills the list with music events happened for the specified network player.
		/// </summary>
		/// <param name="player">Player to get events for.</param>
		/// <param name="list">List with events for this player.</param>
		void GetMusicEvents(int player, std::list<NetworkMusicData> &list);

		/// <summary>
		/// Adds the music event to internal list of music events for the specified player.
		/// </summary>
		/// <param name="player">Player(s) for which the event happened.</param>
		/// <param name="state">NetworkMusicState for the event.</param>
		/// <param name="filepath">Music file path to transmit to client.</param>
		/// <param name="loops">Loops counter.</param>
		/// <param name="position">Music playback position.</param>
		/// <param name="pitch">Pitch value.</param>
		void RegisterMusicEvent(int player, NetworkMusicState state, const char *filepath, int loops = 0, double position = 0, float pitch = 1);

		/// <summary>
		/// Fills the list with sound events happened for the specified network player.
		/// </summary>
		/// <param name="player">Player to get events for.</param>
		/// <param name="list">List with events for this player.</param>
		void GetSoundEvents(int player, std::list<NetworkSoundData> & list);

		/// <summary>
		/// Adds the sound event to internal list of sound events for the specified player.
		/// </summary>
		/// <param name="player">Player(s) for which the event happened.</param>
		/// <param name="state">NetworkSoundState for the event .</param>
		/// <param name="channels">Pointer to an unordered_set of channels this sound event applies to on the server.</param>
		/// <param name="soundFileHashes">Pointer to a vector of hashes describing sound file locations to transmit to client.</param>
		/// <param name="attenuation">Sound attenuation.</param>
		/// <param name="loops">Loops counter.</param>
		/// <param name="pitch">Pitch value.</param>
		/// <param name="affectedByGlobalPitch">Whether the sound is affected by pitch.</param>
		/// <param name="fadeOutTime">The amount of time, in ms, to fade out over.</param>
		void RegisterSoundEvent(int player, NetworkSoundState state, std::unordered_set<unsigned short int> const *channels = NULL, std::vector<size_t> const *soundFileHashes = NULL, short int attenuation = 0, short int loops = 0, float pitch = 1, bool affectedByGlobalPitch = false, short int fadeOutTime = 0);
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static const std::string m_ClassName; //!< A string with the friendly-formatted type name of this object.

		FMOD::System *m_AudioSystem; //!< The FMOD Sound management object
		FMOD::ChannelGroup *m_MasterChannelGroup; //!< The top-level FMOD ChannelGroup that holds everything
		FMOD::ChannelGroup *m_MusicChannelGroup; //!< The FMOD ChannelGroup for music
		FMOD::ChannelGroup *m_SoundChannelGroup; //!< The FMOD ChannelGroup for sounds

		bool m_AudioEnabled; //!< Bool to tell whether audio is enabled or not.

		double m_SoundsVolume; //!< Global sounds effects volume.
		double m_MusicVolume; //!< Global music volume.
		double m_GlobalPitch; //!< Global pitch multiplier.

		std::string m_MusicPath; //!< The path to the last played music stream.
		std::list<std::string> m_MusicPlayList; //!< Playlist of paths to music to play after the current non looping one is done.
		Timer m_SilenceTimer; //!< Timer for measuring silences between songs.

		bool m_IsInMultiplayerMode; //!< If true then the server is in multiplayer mode and will register sound and music events into internal lists.
		std::list<NetworkSoundData> m_SoundEvents[c_MaxClients]; //!< Lists of per player sound events.
		std::list<NetworkMusicData> m_MusicEvents[c_MaxClients]; //!< Lists of per player music events.

		std::mutex g_SoundEventsListMutex[c_MaxClients]; //!< A list for locking sound events for multiplayer to avoid race conditions and other such problems.

	private:

		/// <summary>
		/// A static callback function for FMOD to invoke when the music channel finishes playing. See fmod docs - FMOD_SYSTEM_CALLBACK for details
		/// </summary>
		static FMOD_RESULT F_CALLBACK MusicChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);

		/// <summary>
		/// A static callback function for FMOD to invoke when a sound channel finished playing. See fmod docs - FMOD_SYSTEM_CALLBACK for details
		/// </summary>
		static FMOD_RESULT F_CALLBACK SoundChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);

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