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
	/// The singleton manager of sound effect and music playback.
	/// </summary>
	class AudioMan : public Singleton<AudioMan> {
		friend class SettingsMan;
		friend class SoundContainer;

	public:

		/// <summary>
		/// Hardcoded playback priorities for sounds. Note that sounds don't have to use these specifically; their priority can be anywhere between high and low.
		/// </summary>
		enum PlaybackPriority {
			PRIORITY_HIGH = 0,
			PRIORITY_NORMAL = 128,
			PRIORITY_LOW = 256
		};

		/// <summary>
		/// Music event states for sending music data from the server to clients during multiplayer games.
		/// </summary>
		enum NetworkMusicState {
			MUSIC_PLAY = 0,
			MUSIC_STOP,
			MUSIC_SILENCE,
			MUSIC_SET_PITCH
		};

		/// <summary>
		/// The data struct used to send music data from the server to clients during multiplayer games.
		/// </summary>
		struct NetworkMusicData {
			unsigned char State;
			char Path[256];
			int LoopsOrSilence;
			float Position;
			float Pitch;
		};

		/// <summary>
		/// Sound event states for sending sound data from the server to clients during multiplayer games.
		/// </summary>
		enum NetworkSoundState {
			SOUND_SET_GLOBAL_PITCH = 0,
			SOUND_PLAY,
			SOUND_STOP,
			SOUND_SET_POSITION,
			SOUND_SET_VOLUME,
			SOUND_SET_PITCH,
			SOUND_FADE_OUT
		};

		/// <summary>
		/// The data struct used to send sound data from the server to clients during multiplayer games.
		/// </summary>
		struct NetworkSoundData {
			unsigned char State;
			std::size_t SoundFileHash;
			int Channel;
			bool Immobile;
			float AttenuationStartDistance;
			int Loops;
			int Priority;
			bool AffectedByGlobalPitch;
			float Position[2];
			float Volume;
			float Pitch;
			int FadeOutTime;
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
		/// <returns>Whether the audio system was initialized successfully. If not, no audio will be available.</returns>
		bool Initialize();
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
		/// Gets the virtual and real playing channel counts, filling in the passed-in out-parameters.
		/// </summary>
		/// <param name="outVirtualChannelCount">The out-parameter that will hold the virtual channel count.</param>
		/// <param name="outRealChannelCount">The out-parameter that will hold the real channel count.</param>
		/// <returns>Whether or not the playing channel count was succesfully gotten.</returns>
		bool GetPlayingChannelCount(int *outVirtualChannelCount, int *outRealChannelCount) const { return m_AudioSystem->getChannelsPlaying(outVirtualChannelCount, outRealChannelCount) == FMOD_OK; }

		/// <summary>
		/// Returns the total number of virtual audio channels available.
		/// </summary>
		/// <returns>The number of virtual audio channels available.</returns>
		int GetTotalVirtualChannelCount() const { return c_MaxVirtualChannels; }

		/// <summary>
		/// Returns the total number of real audio channels available.
		/// </summary>
		/// <returns>The number of real audio channels available.</returns>
		int GetTotalRealChannelCount() const { int channelCount; return m_AudioSystem->getSoftwareChannels(&channelCount) == FMOD_OK ? channelCount : 0; }

		/// <summary>
		/// Gets whether all audio is muted or not.
		/// </summary>
		/// <returns>Whether all the audio is muted or not.</returns>
		bool GetMasterMuted() const { return m_MuteMaster; }

		/// <summary>
		/// Mutes or unmutes all audio.
		/// </summary>
		/// <param name="muteOrUnmute">Whether to mute or unmute all the audio.</param>
		void SetMasterMuted(bool muteOrUnmute = true) { m_MuteMaster = muteOrUnmute; if (m_AudioEnabled) { m_MasterChannelGroup->setMute(m_MuteMaster); } }

		/// <summary>
		/// Gets the volume of all audio. Does not get music or sounds individual volumes.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		float GetMasterVolume() const { return m_MasterVolume; }

		/// <summary>
		/// Sets all the audio to a specific volume. Does not affect music or sounds individual volumes.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetMasterVolume(float volume = 1.0F) { m_MasterVolume = std::clamp(volume, 0.0F, 1.0F); if (m_AudioEnabled) { m_MasterChannelGroup->setVolume(m_MasterVolume); } }

		/// <summary>
		/// Gets the global pitch scalar value for all sounds and music.
		/// </summary>
		/// <returns>The current pitch scalar. Will be > 0.</returns>
		float GetGlobalPitch() const { return m_GlobalPitch; }

		/// <summary>
		/// Sets the global pitch multiplier for mobile sounds, optionally setting it for immobile sounds and music.
		/// </summary>
		/// <param name="pitch">New global pitch, limited to 8 octaves up or down (i.e. 0.125 - 8). Defaults to 1.</param>
		/// <param name="includeImmobileSounds">Whether to include immobile sounds (normally used for GUI and so on) in global pitch modification. Defaults to false.</param>
		/// <param name="includeMusic">Whether to include the music in global pitch modification. Defaults to false.</param>
		void SetGlobalPitch(float pitch = 1.0F, bool includeImmobileSounds = false, bool includeMusic = false);

		/// <summary>
		/// The strength of the sound panning effect.
		/// </summary>
		/// <returns>0 - 1, where 0 is no panning and 1 is fully panned.</returns>
		float GetSoundPanningEffectStrength() const { return m_SoundPanningEffectStrength; }
#pragma endregion

#pragma region Music Getters and Setters
		/// <summary>
		/// Reports whether any music stream is currently playing.
		/// </summary>
		/// <returns>Whether any music stream is currently playing.</returns>
		bool IsMusicPlaying() const { bool isPlayingMusic; return m_AudioEnabled && m_MusicChannelGroup->isPlaying(&isPlayingMusic) == FMOD_OK ? isPlayingMusic : false; }

		/// <summary>
		/// Gets whether the music channel is muted or not.
		/// </summary>
		/// <returns>Whether the music channel is muted or not.</returns>
		bool GetMusicMuted() const { return m_MuteMusic; }

		/// <summary>
		/// Mutes or unmutes the music channel.
		/// </summary>
		/// <param name="muteOrUnmute">Whether to mute or unmute the music channel.</param>
		void SetMusicMuted(bool muteOrUnmute = true) { m_MuteMusic = muteOrUnmute; if (m_AudioEnabled) { m_MusicChannelGroup->setMute(m_MuteMusic); } }

		/// <summary>
		/// Gets the volume of music. Does not get volume of sounds.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		float GetMusicVolume() const { return m_MusicVolume; }

		/// <summary>
		/// Sets the music to a specific volume. Does not affect sounds.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetMusicVolume(float volume = 1.0F) { m_MusicVolume = std::clamp(volume, 0.0F, 1.0F); if (m_AudioEnabled) { m_MusicChannelGroup->setVolume(m_MusicVolume); } }

		/// <summary>
		/// Sets the music to a specific volume, but it will only last until a new song is played. Useful for fading etc.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetTempMusicVolume(float volume = 1.0F);

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
		float GetMusicPosition() const;

		/// <summary>
		/// Sets the music to a specific position in the song.
		/// </summary>
		/// <param name="position">The desired position from the start, in seconds.</param>
		void SetMusicPosition(float position);
#pragma endregion

#pragma region Overall Sound Getters and Setters
		/// <summary>
		/// Gets whether all the sound effects channels are muted or not.
		/// </summary>
		/// <returns>Whether all the sound effects channels are muted or not.</returns>
		bool GetSoundsMuted() const { return m_MuteSounds; }

		/// <summary>
		/// Mutes or unmutes all the sound effects channels.
		/// </summary>
		/// <param name="muteOrUnmute">Whether to mute or unmute all the sound effects channels.</param>
		void SetSoundsMuted(bool muteOrUnmute = true) { m_MuteSounds = muteOrUnmute; if (m_AudioEnabled) { m_SoundChannelGroup->setMute(m_MuteSounds); } }

		/// <summary>
		/// Gets the volume of all sounds. Does not get volume of music.
		/// </summary>
		/// <returns>Current volume scalar value. 0.0-1.0.</returns>
		float GetSoundsVolume() const { return m_SoundsVolume; }

		/// <summary>
		/// Sets the volume of all sounds to a specific volume. Does not affect music.
		/// </summary>
		/// <param name="volume">The desired volume scalar. 0.0-1.0.</param>
		void SetSoundsVolume(float volume = 1.0F) { m_SoundsVolume = volume; if (m_AudioEnabled) { m_SoundChannelGroup->setVolume(m_SoundsVolume); } }
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
		void PlayMusic(const char *filePath, int loops = -1, float volumeOverrideIfNotMuted = -1.0F);

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
		void QueueSilence(int seconds) { if (m_AudioEnabled && seconds > 0) { m_MusicPlayList.push_back("@" + std::to_string(seconds)); } }

		/// <summary>
		/// Clears the music queue.
		/// </summary>
		void ClearMusicQueue() { m_MusicPlayList.clear(); }
#pragma endregion

#pragma region Mobile Sound Playback and Handling
		/// <summary>
		/// Pauses all sound playback.
		/// <param name="pause">Whether to pause sounds or resume them.</param>
		/// </summary>
		void PauseAllMobileSounds(bool pause = true) { if (m_AudioEnabled) { m_MobileSoundChannelGroup->setPaused(pause); } }
#pragma endregion

#pragma region Lua Sound File Playing
		/// <summary>
		/// Starts playing a certain sound file.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const std::string &filePath) { return PlaySound(filePath, Vector(), -1); }

		/// <summary>
		/// Starts playing a certain sound file at a certain position for all players.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const std::string &filePath, const Vector &position) { return PlaySound(filePath, position, -1); }

		/// <summary>
		/// Starts playing a certain sound file at a certain position for a certain player.
		/// </summary>
		/// <param name="filePath">The path to the sound file to play.</param>
		/// <param name="position">The position at which to play the SoundContainer's sounds.</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players.</param>
		/// <returns>The new SoundContainer being played. OWNERSHIP IS TRANSFERRED!</returns>
		SoundContainer *PlaySound(const std::string &filePath, const Vector &position, int player);
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
		/// <param name="loops">LoopsOrSilence counter or, if state is silence, the length of the silence.</param>
		/// <param name="position">Music playback position.</param>
		/// <param name="pitch">Pitch value.</param>
		void RegisterMusicEvent(int player, NetworkMusicState state, const char *filepath, int loopsOrSilence = 0, float position = 0, float pitch = 1.0F);

		/// <summary>
		/// Clears the list of current Music events for the target player.
		/// </summary>
		/// <param name="player">Player to clear music events for. -1 clears for all players</param>
		void ClearMusicEvents(int player);

		/// <summary>
		/// Fills the list with sound events happened for the specified network player.
		/// </summary>
		/// <param name="player">Player to get events for.</param>
		/// <param name="list">List with events for this player.</param>
		void GetSoundEvents(int player, std::list<NetworkSoundData> &list);

		/// <summary>
		/// Adds the sound event to the internal list of sound events for the specified player.
		/// </summary>
		/// <param name="player">Player(s) for which the event happened.</param>
		/// <param name="state">NetworkSoundState for the event.</param>
		/// <param name="soundContainer">A pointer to the SoundContainer this event is happening to, or a null pointer for global events.</param>
		/// <param name="fadeOutTime">THe amount of time, in MS, to fade out over. This data isn't contained in SoundContainer, so it needs to be passed in separately.</param>
		void RegisterSoundEvent(int player, NetworkSoundState state, const SoundContainer *soundContainer, int fadeOutTime = 0);

		/// <summary>
		/// Clears the list of current Sound events for the target player.
		/// </summary>
		/// <param name="player">Player to clear sound events for. -1 clears for all players.</param>
		void ClearSoundEvents(int player);
#pragma endregion

	protected:

		const FMOD_VECTOR c_FMODForward = FMOD_VECTOR{0, 0, 1}; //!< An FMOD_VECTOR defining the Forwards direction. Necessary for 3D Sounds.
		const FMOD_VECTOR c_FMODUp = FMOD_VECTOR{0, 1, 0}; //!< An FMOD_VECTOR defining the Up direction. Necessary for 3D Sounds.

		FMOD::System *m_AudioSystem; //!< The FMOD Sound management object.
		FMOD::ChannelGroup *m_MasterChannelGroup; //!< The top-level FMOD ChannelGroup that holds everything.
		FMOD::ChannelGroup *m_MusicChannelGroup; //!< The FMOD ChannelGroup for music.
		FMOD::ChannelGroup *m_SoundChannelGroup; //!< The FMOD ChannelGroup for sounds.
		FMOD::ChannelGroup *m_MobileSoundChannelGroup; //!< The FMOD ChannelGroup for mobile sounds.
		FMOD::ChannelGroup *m_ImmobileSoundChannelGroup; //!< The FMOD ChannelGroup for immobile sounds.

		bool m_AudioEnabled; //!< Bool to tell whether audio is enabled or not.
		std::vector<std::unique_ptr<const Vector>> m_CurrentActivityHumanPlayerPositions; //!< The stored positions of each human player in the current activity. Only filled when there's an activity running.
		std::unordered_map<int, float> m_SoundChannelMinimumAudibleDistances; //!<  An unordered map of sound channel indices to floats representing each Sound Channel's minimum audible distances. This is necessary to keep safe data in case the SoundContainer is destroyed while the sound is still playing, as happens often with TDExplosives.

		bool m_MuteMaster; //!< Whether all the audio is muted.
		bool m_MuteMusic; //!< Whether the music channel is muted.
		bool m_MuteSounds; //!< Whether all the sound effects channels are muted.
		float m_MasterVolume; //!< Global volume of all audio.
		float m_MusicVolume; //!< Global music volume.
		float m_SoundsVolume; //!< Global sounds effects volume.
		float m_GlobalPitch; //!< Global pitch multiplier.

		float m_SoundPanningEffectStrength; //!< The strength of the sound panning effect, 0 (no panning) - 1 (full panning).

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		float m_ListenerZOffset;
		float m_MinimumDistanceForPanning;
		//////////////////////////////////////////////////

		std::string m_MusicPath; //!< The path to the last played music stream.
		std::list<std::string> m_MusicPlayList; //!< Playlist of paths to music to play after the current non looping one is done.
		Timer m_SilenceTimer; //!< Timer for measuring silences between songs.

		bool m_IsInMultiplayerMode; //!< If true then the server is in multiplayer mode and will register sound and music events into internal lists.
		std::list<NetworkSoundData> m_SoundEvents[c_MaxClients]; //!< Lists of per player sound events.
		std::list<NetworkMusicData> m_MusicEvents[c_MaxClients]; //!< Lists of per player music events.

		std::mutex g_SoundEventsListMutex[c_MaxClients]; //!< A list for locking sound events for multiplayer to avoid race conditions and other such problems.

	private:

#pragma region Sound Container Actions and Modifications
		/// <summary>
		/// Starts playing the next SoundSet of the given SoundContainer for the give player.
		/// </summary>
		/// <param name="soundContainer">Pointer to the SoundContainer to start playing. Ownership is NOT transferred!</param>
		/// <param name="player">Which player to play the SoundContainer's sounds for, -1 means all players. Defaults to -1.</param>
		/// <returns>Whether or not playback of the Sound was successful.</returns>
		bool PlaySoundContainer(SoundContainer *soundContainer, int player = -1);

		/// <summary>
		/// Sets/updates the position of a SoundContainer's playing sounds.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership IS NOT transferred!</param>
		/// <returns>Whether the position was successfully set.</returns>
		bool ChangeSoundContainerPlayingChannelsPosition(const SoundContainer *soundContainer);

		/// <summary>
		/// Changes the volume of a SoundContainer's playing sounds.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership IS NOT transferred!</param>
		/// <param name="newVolume">The new volume to play sounds at, between 0 and 1.</param>
		/// <returns>Whether the volume was successfully updated.</returns>
		bool ChangeSoundContainerPlayingChannelsVolume(const SoundContainer *soundContainer, float newVolume);

		/// <summary>
		/// Changes the frequency/pitch of a SoundContainer's playing sounds.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership IS NOT transferred!</param>
		/// <returns>Whether the pitch was successfully updated.</returns>
		bool ChangeSoundContainerPlayingChannelsPitch(const SoundContainer *soundContainer);

		/// <summary>
		/// Stops playing a SoundContainer's playing sounds for a certain player.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object6. Ownership is NOT transferred!</param>
		/// <param name="player">Which player to stop playing the SoundContainer for.</param>
		/// <returns></returns>
		bool StopSoundContainerPlayingChannels(SoundContainer *soundContainer, int player);

		/// <summary>
		/// Fades out playback a SoundContainer.
		/// </summary>
		/// <param name="soundContainer">A pointer to a SoundContainer object. Ownership is NOT transferred!</param>
		/// <param name="fadeOutTime">The amount of time, in ms, to fade out over.</param>
		void FadeOutSoundContainerPlayingChannels(SoundContainer *soundContainer, int fadeOutTime);
#pragma endregion

#pragma region 3D Effect Handling
		/// <summary>
		/// Updates 3D effects calculations for all sound channels whose SoundContainers isn't immobile.
		/// </summary>
		void Update3DEffectsForMobileSoundChannels();

		/// <summary>
		/// Sets or updates the position of the given sound channel so it handles scene wrapping correctly. Also handles volume attenuation and minimum audible distance.
		/// </summary>
		/// <param name="soundChannel">The channel whose position should be set or updated.</param>
		/// <param name="positionToUse">An optional position to set for this sound channel. Done this way to save setting and resetting data in FMOD.</param>
		/// <returns>Whether the channel's position was succesfully set.</returns>
		FMOD_RESULT UpdatePositionalEffectsForSoundChannel(FMOD::Channel *soundChannel, const FMOD_VECTOR *positionToUse = nullptr) const;
#pragma endregion

#pragma region FMOD Callbacks
		/// <summary>
		/// A static callback function for FMOD to invoke when the music channel finishes playing. See fmod docs - FMOD_CHANNELCONTROL_CALLBACK for details
		/// </summary>
		static FMOD_RESULT F_CALLBACK MusicChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);

		/// <summary>
		/// A static callback function for FMOD to invoke when a sound channel finished playing. See fmod docs - FMOD_CHANNELCONTROL_CALLBACK for details
		/// </summary>
		static FMOD_RESULT F_CALLBACK SoundChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *commandData1, void *commandData2);
#pragma endregion

#pragma region Utility Methods
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
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this AudioMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		AudioMan(const AudioMan &reference) = delete;
		AudioMan & operator=(const AudioMan &rhs) = delete;
	};
}
#endif
