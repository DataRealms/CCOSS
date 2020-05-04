#ifndef _RTEAUDIOMAN_
#define _RTEAUDIOMAN_

#include "Constants.h"
#include "Entity.h"
#include "Timer.h"
#include "Vector.h"
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

		// TODO: Add comments to all these.
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
			SOUND_SET_POSITION,
			SOUND_SET_PITCH,
			SOUND_SET_GLOBAL_PITCH,
			SOUND_FADE_OUT
		};

		struct NetworkSoundData {
			unsigned char State;
			unsigned short Channels[c_MaxPlayingSoundsPerContainer];
			size_t SoundFileHashes[c_MaxPlayingSoundsPerContainer];
			float Position[2];
			short Loops;
			float Pitch;
			bool AffectedByGlobalPitch;
			float AttenuationStartDistance;
			bool Immobile;
			short FadeOutTime;
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

		/// <summary>
		/// Resets the entire AudioMan including it's inherited members to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this AudioMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update();
#pragma endregion

#pragma region General Getters and Setters
		/// <summary>
		/// Gets the audio management system object used for playing all audio.
		/// </summary>
		/// <returns>The audio management system object used by AudioMan for playing audio.</returns>
		FMOD::System *GetAudioSystem() const { return m_AudioSystem; }

		/// <summary>
		/// Reports whether audio is enabled.
		/// </summary>
		/// <returns>Whether audio is enabled.</returns>
		bool IsAudioEnabled() const { return m_AudioEnabled; }

		/// <summary>
		/// Returns the number of audio channels currently used.
		/// </summary>
		/// <returns>The number of audio channels currently used.</returns>
		int GetPlayingChannelCount() const { int channelCount; return m_AudioSystem->getChannelsPlaying(&channelCount, NULL) == FMOD_OK ? channelCount : 0; }

		/// <summary>
		/// Returns the number of audio channels available in total.
		/// </summary>
		/// <returns>The number of audio channels available in total.</returns>
		int GetTotalChannelCount() const { int channelCount; return m_AudioSystem->getSoftwareChannels(&channelCount) == FMOD_OK ? channelCount : 0; }

		/// <summary>
		/// Gets the global pitch scalar value for all sounds and music.
		/// </summary>
		/// <returns>The current pitch scalar. Will be > 0.</returns>
		double GetGlobalPitch() const { return m_GlobalPitch; }

		/// <summary>
		/// Sets the global pitch multiplier for mobile sounds, optionally setting it for immobile sounds and music.
		/// </summary>
		/// <param name="pitch">New global pitch, limited to 8 octaves up or down (i.e. 0.125 - 8). Defaults to 1.</param>
		/// <param name="includeImmobileSounds">Whether to include immobile sounds (normally used for GUI and so on) in global pitch modification. Defaults to false.</param>
		/// <param name="includeMusic">Whether to include the music in global pitch modification. Defaults to false.</param>
		void SetGlobalPitch(double pitch = 1.0, bool includeImmobileSounds = false, bool includeMusic = false);
#pragma endregion

#pragma region Music Getters and Setters
		/// <summary>
		/// Reports whether any music stream is currently playing.
		/// </summary>
		/// <returns>Whether any music stream is currently playing.</returns>
		bool IsMusicPlaying() const { bool isPlayingMusic; return m_AudioEnabled && m_MusicChannelGroup->isPlaying(&isPlayingMusic) == FMOD_OK ? isPlayingMusic : false; }

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
		/// <returns>Whether the music channel's pitch was successfully updated.</returns>
		bool SetMusicPitch(float pitch);

		/// <summary>
		/// Gets the position of playback of the current music stream, in seconds.
		/// </summary>
		/// <returns>The current position of the current stream playing, in seconds.</returns>
		double GetMusicPosition() const;

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
		/// Sets/updates the position of a SoundContainer's playing sounds.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership IS NOT transferred!</param>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <returns>Whether the position was successfully set.</returns>
		bool SetSoundPosition(SoundContainer *soundContainer, const Vector &position);

		/// <summary>
		/// Sets/updates the frequency/pitch of a SoundContainer's playing sounds.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership IS NOT transferred!</param>
		/// <param name="pitch">New pitch to play sounds at, limited to 8 octaves up or down (i.e. 0.125 - 8).</param>
		/// <returns>Whether the pitch was successfully set.</returns>
		bool SetSoundPitch(SoundContainer *soundContainer, float pitch);
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
		/// Can be done several times to queue up many tracks. The last track in the queue will be looped infinitely.
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
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath) { return PlaySound(filePath, Vector(), -1); }

		/// <summary>
		/// Starts playing a certain sound file at a certain position for all players.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath, const Vector &position) { return PlaySound(filePath, position, -1); }

		/// <summary>
		/// Starts playing a certain sound file at a certain position for a certain player.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players.</param>
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath, const Vector &position, int player) { return PlaySound(filePath, position, player, 0, PRIORITY_NORMAL, -1, c_DefaultAttenuationStartDistance, false); }

		/// <summary>
		/// Starts playing a certain sound file with various configuration settings.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players.</param>
		/// <param name="loops">The number of times to loop the SoundContainer's sounds. 0 means play once. -1 means play infinitely until stopped.</param>
		/// <param name="priority">The priority of this sound from 256 (lowest) to 0 (highest). Higher priority sounds are more likely to be heard.</param>
		/// <param name="pitchOrAffectedByGlobalPitch">
		/// The pitch to play this SoundContainer's at where 1 is unmodified frequency and each multiple of 2 is an octave up or down.
		/// -1 means the SoundContainer will be affected by global pitch instead of setting handling its pitch manually.
		/// </param>
		/// <param name="attenuationStartDistance">The distance at which this SoundContainer's sounds should start attenuating away. -1 means default.</param>
		/// </param name="immobile">Whether this SoundContainer's sounds will be treated as immobile, i.e. they won't be affected by 3D sound manipulation.</param>
		/// <returns>Returns the new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const char *filePath, const Vector &position, int player, int loops, int priority, double pitchOrAffectedByGlobalPitch, float attenuationStartDistance, bool immobile);

		/// <summary>
		/// Starts playing the next sample of a certain SoundContainer for a certain player.
		/// </summary>
		/// <param name="soundContainer">Pointer to the SoundContainer to start playing. Ownership is NOT transferred!</param>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players. Defaults to -1.</param>
		/// <param name="priority">The priority of this sound - higher priority sounds are more likely to be heard. -1 means it'll use the SoundContainer's value. Defaults to -1.</param>
		/// <param name="pitch">The pitch to play this SoundContainer's at where 1 is unmodified frequency and each multiple of 2 is an octave up or down. Defaults to 1.</param>
		/// <returns>Whether or not playback of the Sound was successful.</returns>
		bool PlaySound(SoundContainer *soundContainer, const Vector &position, int player = -1, int priority = -1, double pitch = 1);

		/// <summary>
		/// Stops playing all sounds in a given SoundContainer.
		/// </summary>
		/// <param name="soundContainer">Pointer to the SoundContainer to stop playing. Ownership is NOT transferred!</param>
		/// <returns>True if sounds were playing and were stopped, false otherwise.</returns>
		bool StopSound(SoundContainer *soundContainer) { return StopSound(soundContainer, -1); }

		/// <summary>
		/// Stops playing all sounds in a given SoundContainer for a certain player.
		/// </summary>
		/// <param name="player">Which player to stop the SoundContainer  for.</param>
		/// <param name="soundContainer">Pointer to the SoundContainer to stop playing. Ownership is NOT transferred!</param>
		/// <returns></returns>
		bool StopSound(SoundContainer *soundContainer, int player);

		/// <summary>
		/// Fades out playback of all sounds in a specific SoundContainer.
		/// </summary>
		/// <param name="soundContainer">Pointer to the Sound to fade out playing. Ownership is NOT transferred!</param>
		/// <param name="fadeOutTime">The amount of time, in ms, to fade out over.</param>
		void FadeOutSound(SoundContainer *soundContainer, int fadeOutTime = 1000);
#pragma endregion

#pragma region Network Audio Handling
		/// <summary>
		/// Returns true if manager is in multiplayer mode.
		/// </summary>
		/// <returns>True if in multiplayer mode.</returns>
		bool IsInMultiplayerMode() const { return m_IsInMultiplayerMode; }

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
		/// <param name="position">Sound position.</param>
		/// <param name="loops">Loops counter.</param>
		/// <param name="pitch">Pitch value.</param>
		/// <param name="attenuationStartDistance">The distance at which the sound will start attenuating away.</param>
		/// <param name="affectedByGlobalPitch">Whether the sound is affected by pitch.</param>
		/// <param name="fadeOutTime">The amount of time, in ms, to fade out over.</param>
		void RegisterSoundEvent(int player, NetworkSoundState state, const std::unordered_set<unsigned short> *channels = NULL, const std::vector<size_t> *soundFileHashes = NULL, const Vector &position = Vector(), short loops = 0, float pitch = 1, bool affectedByGlobalPitch = false, float attenuationStartDistance = 0, bool immobile = false, short fadeOutTime = 0);
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static const std::string m_ClassName; //!< A string with the friendly-formatted type name of this object.

		const FMOD_VECTOR c_FMODForward = FMOD_VECTOR{0, 0, 1}; //!< An FMOD_VECTOR defining the Forwards direction. Necessary for 3D Sounds.
		const FMOD_VECTOR c_FMODUp = FMOD_VECTOR{0, 1, 0}; //!< An FMOD_VECTOR defining the Up direction. Necessary for 3D Sounds.

		FMOD::System *m_AudioSystem; //!< The FMOD Sound management object.
		FMOD::ChannelGroup *m_MasterChannelGroup; //!< The top-level FMOD ChannelGroup that holds everything.
		FMOD::ChannelGroup *m_MusicChannelGroup; //!< The FMOD ChannelGroup for music.
		FMOD::ChannelGroup *m_SoundChannelGroup; //!< The FMOD ChannelGroup for sounds.
		FMOD::ChannelGroup *m_MobileSoundChannelGroup; //!< The FMOD ChannelGroup for mobile sounds.
		FMOD::ChannelGroup *m_ImmobileSoundChannelGroup; //!< The FMOD ChannelGroup for immobile sounds.
		
		bool m_AudioEnabled; //!< Bool to tell whether audio is enabled or not.
		int m_CurrentActivityHumanCount; //!< The stored number of humans in the current activity, used for audio splitscreen handling. Only updated when there's an activity running.

		std::unordered_map<unsigned short, std::vector<FMOD_VECTOR>> soundChannelRolloffs; //!< An unordered map of Sound Channel indices to a std::vector of FMOD_VECTORs representing each Sound Channel's custom attenuation rolloff. This is necessary to keep safe data in case the SoundContainer is destroyed while the sound is still playing.

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
		/// A static callback function for Digital Signal Processing that does extra work with Panning and Attenuation to make them better for CC.
		/// </summary>
		static FMOD_RESULT F_CALLBACK AudioMan::PanAndAttenuationDSPCallback(FMOD_DSP_STATE *dspState, float *inBuffer, float *outBuffer, unsigned int length, int inChannels, int *outChannels);

		/// <summary>
		/// Updates 3D effects calculations for all sound channels whose SoundContainers isn't immobile.
		/// </summary>
		void UpdateCalculated3DEffectsForMobileSoundChannels();

		/// <summary>
		/// Updates 3D effects calculations on a given sound channel whose SoundContainer isn't immobile.
		/// </summary>
		/// <returns>FMOD_OK if the 3D effects were successfully updated, otherwise an FMOD_ERROR.</returns>
		FMOD_RESULT UpdateMobileSoundChannelCalculated3DEffects(FMOD::Channel *channel);

		/// <summary>
		/// Gets the corresponding FMOD_VECTOR for a given RTE Vector.
		/// </summary>
		/// <param name="vector">The RTE Vector to get as an FMOD_VECTOR.</param>
		/// <returns>The FMOD_VECTOR that corresponds to the given RTE Vector.</returns>
		FMOD_VECTOR GetAsFMODVector(const Vector &vector, float zValue = 0) const;

		/// <summary>
		/// Gets the corresponding RTE Vector for a given FMOD_VECTOR.
		/// </summary>
		/// <param name="fmodVector">The FMOD_VECTOR to get as an RTE Vector.</param>
		/// <returns>The RTE Vector that corresponds to the given FMOD_VECTOR.</returns>
		Vector GetAsVector(FMOD_VECTOR fmodVector) const;

		/// <summary>
		/// Clears all the member variables of this AudioMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		AudioMan(const AudioMan &reference);
		AudioMan & operator=(const AudioMan &rhs) {}
	};
}
#endif