#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "Sound.h"
#include "DDTError.h"

namespace RTE {
	const std::string AudioMan::m_ClassName = "AudioMan";

	// I know this is a crime, but if I include it in FrameMan.h the whole thing will collapse due to int redefinitions in Allegro
	std::mutex g_SoundEventsListMutex[c_MaxClients];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Callback for catching music streams that end
	signed char F_CALLBACKAPI PlayNextCallback(FSOUND_STREAM *stream, void *buff, int len, void *userdata) { g_AudioMan.PlayNextStream(); return 0; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Clear() {
		m_AudioEnabled = false;
		m_pMusic = 0;

		m_MusicChannel = -1;
		m_MusicPath.clear();
		m_SoundsVolume = 1.0;
		m_MusicVolume = 1.0;
		m_GlobalPitch = 1.0;
		m_NormalFrequencies.clear();
		m_PitchModifiers.clear();
		m_MusicPlayList.clear();
		m_SilenceTimer.Reset();
		m_SilenceTimer.SetRealTimeLimitS(-1);
		m_IsInMultiplayerMode = false;

		for (int i = 0; i < c_MaxClients; i++) {
			m_SoundEvents[i].clear();
			m_MusicEvents[i].clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AudioMan::Create() {
		// Basic parameters for the audio
		int audioBitrate = 44100;
		int maxChannels = g_SettingsMan.GetAudioChannels();

		if (!FSOUND_Init(audioBitrate, maxChannels, 0)) {
			// Audio failed to initialize, so just disable it
			m_AudioEnabled = false;
			return -1;
		} else {
			m_AudioEnabled = true;
		}

		// initialize the array of normal frequencies
		int channelCount = FSOUND_GetMaxChannels();
		for (int channel = 0; channel < channelCount; ++channel) {
			m_NormalFrequencies.push_back(FSOUND_GetFrequency(channel));
			m_PitchModifiers.push_back(1.0);
		}

		// Set the global sound channels' volume
		FSOUND_SetSFXMasterVolume(s_MaxVolume * m_SoundsVolume);
		// initialize the global pitch
		SetGlobalPitch(m_GlobalPitch);
		// initialize Music volume
		SetMusicVolume(m_MusicVolume);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Destroy() {

		if (!m_AudioEnabled) {
			return;
		}
		StopAll();
		FSOUND_Close();
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Update() {
		if (!m_AudioEnabled) {
			return;
		}

		// Done waiting for silence
		if (!m_pMusic && m_SilenceTimer.IsPastRealTimeLimit()) {
			PlayNextStream();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetSoundsVolume(double volume) {
		m_SoundsVolume = volume;

		if (!m_AudioEnabled) {
			return;
		}
		FSOUND_SetSFXMasterVolume(s_MaxVolume * m_SoundsVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicVolume(double volume) {
		m_MusicVolume = volume;

		if (!m_AudioEnabled) {
			return;
		}

		if (m_pMusic >= 0) {
			FSOUND_SetVolumeAbsolute(m_MusicChannel, s_MaxVolume * m_MusicVolume);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetTempMusicVolume(double volume) {
		if (!m_AudioEnabled) {
			return;
		}

		if (m_pMusic >= 0) {
			FSOUND_SetVolumeAbsolute(m_MusicChannel, s_MaxVolume * volume);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetGlobalPitch(double pitch, bool excludeMusic) {
		if (!m_AudioEnabled) {
			return;
		}

		if (m_IsInMultiplayerMode) {
			RegisterSoundEvent(-1, SOUND_SET_PITCH, 0, 0, 0, 0, pitch, excludeMusic);
		}

		// Keep the pitch value sane
		m_GlobalPitch = pitch > 0.1 ? pitch : (pitch < 16.0 ? pitch : 16.0);

		// The channel index is stored in the lower 12 bits of the channel handle
		// It works to pass the channel index to IsPlaying and SetFreq because the lower 12 bits are enough to show which channel we're talking about
		int musicChannelIndex = excludeMusic ? m_MusicChannel & 0x00000FFF : -1;

		int channelCount = FSOUND_GetMaxChannels();

		// Go through all active channels and set the pitch on each, except for the music one
		for (int channel = 0; channel < channelCount; ++channel) {

			// Adjust the music too, it works better for gameplay
			if (channel != musicChannelIndex && FSOUND_IsPlaying(channel)) {

				// Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
				if (m_NormalFrequencies[channel] > 0) {
					FSOUND_SetFrequency(channel, m_NormalFrequencies[channel] * m_PitchModifiers[channel] * m_GlobalPitch);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundPitch(Sound *pSound, float pitch) {
		if (!m_AudioEnabled || !pSound || !IsPlaying(pSound)) {
			return false;
		}
		if (IsInMultiplayerMode() && pSound) {
			RegisterSoundEvent(-1, SOUND_SET_PITCH, pSound->m_Hash, 0, pSound->m_LastChannel, pSound->m_Loops, pitch, pSound->m_AffectedByPitch);
		}

		// Keep the pitch value sane
		if (pitch < 0.1F) {
			pitch = 0.1F;
		}
		if (pitch > 16.0F) {
			pitch = 16.0F;
		}

		// Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
		// The channel index is stored in the lower 12 bits of the channel handle
		int channelIndex = pSound->m_LastChannel & 0x00000FFF;

		// Update the individual pitch modifier
		m_PitchModifiers[channelIndex] = pitch;

		if (m_NormalFrequencies[channelIndex] > 0) {
			FSOUND_SetFrequency(pSound->m_LastChannel, m_NormalFrequencies[channelIndex] * m_PitchModifiers[channelIndex] * m_GlobalPitch);
		}
		return true;
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetMusicPitch(float pitch) {
		if (m_IsInMultiplayerMode) {
			RegisterMusicEvent(-1, MUSIC_SET_PITCH, 0, 0, 0.0, pitch);
		}
		if (!m_AudioEnabled || !m_pMusic) {
			return false;
		}

		// Keep the pitch value sane
		if (pitch < 0.1F) {
			pitch = 0.1F;
		}
		if (pitch > 16.0F) {
			pitch = 16.0F;
		}

		// The channel index is stored in the lower 12 bits of the channel handle
		int channelIndex = m_MusicChannel & 0x00000FFF;

		// Update the individual pitch modifier
		m_PitchModifiers[channelIndex] = pitch;

		// Only set the frequency of those whose normal frequency values are normal (not set to <= 0 because they shouldn't be pitched)
		if (m_NormalFrequencies[channelIndex] > 0) {
			FSOUND_SetFrequency(m_MusicChannel, m_NormalFrequencies[channelIndex] * m_PitchModifiers[channelIndex] * m_GlobalPitch);
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundAttenuation(Sound *pSound, float distance) {
		if (!m_AudioEnabled || !pSound) {
			return false;
		}
		// TODO: Use the fmod 3D attenuation method for this instead

		// Set the distance attenuation effect of the just started sound
		if (distance > 0) {
			if (distance > 1.0F) {
				distance = 1.0F;
			}
			// Multiply by 0.95 because we don't want to go completely quiet if max distance
			distance *= 0.95F;
			FSOUND_SetVolume(pSound->m_LastChannel, s_MaxVolume * (1.0F - distance));
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AudioMan::GetTotalChannelCount() {
		return FSOUND_GetMaxChannels();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int AudioMan::GetPlayingChannelCount() {
		return FSOUND_GetChannelsPlaying();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::IsPlaying(Sound *pSound) {
		if (!m_AudioEnabled || !pSound) {
			return false;
		}
		if (pSound->GetSampleCount() <= 0 || pSound->m_LastChannel < 0) {
			return false;
		}

		// TODO: This doesn't check all the samples of the Sound?!
		if (FSOUND_GetCurrentSample(pSound->m_LastChannel) != pSound->GetCurrentSample()) {
			return false;
		}
		return FSOUND_IsPlaying(pSound->m_LastChannel);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopAll() {
		if (!m_AudioEnabled) {
			return;
		}
		// Halt sound playback on all channels
		FSOUND_StopSound(FSOUND_ALL);

		// If music is playing, stop it
		if (m_pMusic) {
			FSOUND_Stream_Stop(m_pMusic);
			FSOUND_Stream_Close(m_pMusic);
		}
		m_pMusic = 0;
		// Clear out playlist, it doesn't apply anymore
		m_MusicPlayList.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(Sound *pSound) {
		if (!m_AudioEnabled || !pSound) {
			return false;
		}

		if (IsPlaying(pSound)) {
			FSOUND_StopSound(pSound->m_LastChannel);
			pSound->m_LastChannel = -1;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(int player, Sound *pSound) {
		if (m_IsInMultiplayerMode && pSound) {
			RegisterSoundEvent(player, SOUND_STOP, pSound->m_Hash, 0, pSound->m_LastChannel, pSound->m_Loops, 1.0, pSound->m_AffectedByPitch);
		}
		return StopSound(pSound);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopMusic() {
		if (!m_AudioEnabled || !m_pMusic) {
			return;
		}
		if (m_IsInMultiplayerMode) {
			RegisterMusicEvent(-1, MUSIC_STOP, 0, 0, 0.0, 0.0);
		}

		FSOUND_Stream_Stop(m_pMusic);
		FSOUND_Stream_Close(m_pMusic);
		m_pMusic = 0;
		// Clear out playlist, it doesn't apply anymore
		m_MusicPlayList.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double AudioMan::GetMusicPosition() {
		if (!m_AudioEnabled || !m_pMusic) {
			return 0;
		}
		return ((double)FSOUND_Stream_GetTime(m_pMusic)) / 1000.0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicPosition(double position) {
		if (!m_AudioEnabled || !m_pMusic) {
			return;
		}
		FSOUND_Stream_SetTime(m_pMusic, position * 1000);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::FadeOutSound(Sound *pSound, int fadeOutTime) {
		if (!m_AudioEnabled || !pSound) {
			return;
		}
		//TODO: This
	/*	if (IsPlaying(pSound)) {
			Mix_FadeOutChannel(pSound->m_LastChannel, fadeOutTime);
		}
	*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::QueueSilence(int seconds) {
		if (seconds > 0) {
			// Encode the silence as number of secs preceded by '@'
			char str[256];
			sprintf_s(str, sizeof(str), "@%i", seconds);
			m_MusicPlayList.push_back(std::string(str));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::QueueMusicStream(const char *filepath) {
		if (!m_pMusic) {
			PlayMusic(filepath);
		} else {
			m_MusicPlayList.push_back(std::string(filepath));
			// Set the callback so that it will switch to next song when current one is done.
			FSOUND_Stream_SetEndCallback(m_pMusic, PlayNextCallback, 0);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayNextStream() {

		if (!m_MusicPlayList.empty()) {
			std::string nextString = m_MusicPlayList.front();
			m_MusicPlayList.pop_front();

			// Look for special encoding if we are supposed to have a silence between tracks
			if (nextString.c_str()[0] == '@') {
				// Decipher the number of secs we're supposed to wait
				int seconds = 0;
				sscanf(nextString.c_str(), "@%i", &seconds);
				m_SilenceTimer.SetRealTimeLimitS(seconds > 0 ? seconds : 0);
				m_SilenceTimer.Reset();

				// Stop music playback
				if (m_pMusic) {
					RegisterMusicEvent(-1, MUSIC_SILENCE, 0, seconds, 0.0, 1.0);
					FSOUND_Stream_Stop(m_pMusic);
					FSOUND_Stream_Close(m_pMusic);
					m_pMusic = 0;
				}
			} else {
				// Only loop the last one, otherwise play playlist entries only once
				PlayMusic(nextString.c_str(), m_MusicPlayList.empty() ? -1 : 0);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlaySound(const char *wavefilepath) {
		Sound newSound;
		newSound.Create(wavefilepath, false);
		newSound.Play();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Sound * AudioMan::PlaySound(const char *filepath, float distance, bool loops, bool affectedByPitch, int player) {
		if (!filepath) {
			g_ConsoleMan.PrintString("Error: Null filepath passed to AudioMan::PlaySound!");
			return 0;
		}

		Sound * newSound = new Sound();
		newSound->Create(filepath, affectedByPitch, loops ? 1 : 0);
		PlaySound(player, newSound, PRIORITY_LOW, distance, affectedByPitch ? m_GlobalPitch : 1.0);

		return newSound;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySound(int player, Sound *pSound, int priority, float distance, double pitch) {

		// We need to play sound before registering an event because it will assign channel number to the sound being played
		// clients then uses this number to identify the sound being played
		bool ret = PlaySound(pSound, priority, distance, pitch);

		if (m_IsInMultiplayerMode && pSound) {
			RegisterSoundEvent(player, SOUND_PLAY, pSound->m_Hash, distance, pSound->m_LastChannel, pSound->m_Loops, pitch, pSound->m_AffectedByPitch);
		}
		return ret;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySound(Sound *pSound, int priority, float distance, double pitch) {
		if (!m_AudioEnabled || !pSound) {
			return false;
		}

		pSound->m_LastChannel = FSOUND_PlaySound(FSOUND_FREE, pSound->StartNextSample());

		if (pSound->m_LastChannel == -1) {
			g_ConsoleMan.PrintString("ERROR: Could not play a sound sample!");
			return false;
		}

		// Set sample's channel looping setting
		if (pSound->m_Loops == 0) {
			FSOUND_SetLoopMode(pSound->m_LastChannel, FSOUND_LOOP_OFF);
		} else {
			FSOUND_SetLoopMode(pSound->m_LastChannel, FSOUND_LOOP_NORMAL);
		}

		// Set sample's channel priority setting
		if (pSound->m_Priority > 0) {
			FSOUND_SetPriority(pSound->m_LastChannel, pSound->m_Priority);
		}

		// Set the distance attenuation effect of the just started sound
		SetSoundAttenuation(pSound, distance);

		// The channel index is stored in the lower 12 bits of the channel handle
		int channelIndex = pSound->m_LastChannel & 0x00000FFF;

		// Save the 'normal' frequency so we can pitch it later
		DAssert(channelIndex < m_NormalFrequencies.size(), "Channel index higher than normal freq array?");
		if (channelIndex < m_NormalFrequencies.size()) {
			// If this sound isn't supposed to be pitched, then set a <= 0 normal frequency to indicate it
			if (!pSound->m_AffectedByPitch) {
				m_NormalFrequencies[channelIndex] = 0;
			} else {
				m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(pSound->m_LastChannel);
			}
		}

		// Store the individual pitch modifier
		m_PitchModifiers[channelIndex] = pitch;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayMusic(const char *filepath, int loops, double volumeOverride) {
		if (!m_AudioEnabled) {
			return;
		}
		if (m_IsInMultiplayerMode) {
			RegisterMusicEvent(-1, MUSIC_PLAY, filepath, loops, 0.0, 1.0);
		}

		// If music is already playing, first stop it
		if (m_pMusic) {
			FSOUND_Stream_Stop(m_pMusic);
			FSOUND_Stream_Close(m_pMusic);
			m_pMusic = 0;
		}

		// Open the stream
		m_pMusic = FSOUND_Stream_Open(filepath, 0, 0, 0);
		if (!m_pMusic) {
			g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + std::string(filepath));
			return;
		}

		// Start playing the stream
		m_MusicChannel = FSOUND_Stream_Play(FSOUND_FREE, m_pMusic);
		if (m_MusicChannel < 0) {
			g_ConsoleMan.PrintString("ERROR: Could not open and play music file:" + std::string(filepath));
			return;
		}

		// Save the path of the last played music stream
		m_MusicPath = filepath;

		// Set the callback so that it will switch to next song when current one is done.
		FSOUND_Stream_SetEndCallback(m_pMusic, PlayNextCallback, 0);

		// Set loop mode
		if (loops != 0) {
			FSOUND_Stream_SetMode(m_pMusic, FSOUND_LOOP_NORMAL);
		}

		// Set loop count
		FSOUND_Stream_SetLoopCount(m_pMusic, loops);

		// Set the volume of the music stream's channel, and override if asked to, but not if the normal music volume is muted
		if (volumeOverride >= 0 && m_MusicVolume > 0) {
			FSOUND_SetVolumeAbsolute(m_MusicChannel, s_MaxVolume * volumeOverride);
		} else {
			FSOUND_SetVolumeAbsolute(m_MusicChannel, s_MaxVolume * m_MusicVolume);
		}

		// The channel index is stored in the lower 12 bits of the channel handle
		int channelIndex = m_MusicChannel & 0x00000FFF;

		// Save the 'normal' frequency so we can pitch it later
		if (channelIndex < m_NormalFrequencies.size()) {
			m_NormalFrequencies[channelIndex] = FSOUND_GetFrequency(m_MusicChannel);
		}
		m_PitchModifiers[channelIndex] = 1.0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::GetSoundEvents(int player, std::list<NetworkSoundData> & list) {
		if (player < 0 || player >= c_MaxClients) {
			return;
		}

		list.clear();
		g_SoundEventsListMutex[player].lock();

		for (std::list<NetworkSoundData>::iterator eItr = m_SoundEvents[player].begin(); eItr != m_SoundEvents[player].end(); ++eItr) {
			list.push_back((*eItr));
		}

		m_SoundEvents[player].clear();
		g_SoundEventsListMutex[player].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::GetMusicEvents(int player, std::list<NetworkMusicData> & list) {
		if (player < 0 || player >= c_MaxClients) {
			return;
		}

		list.clear();
		g_SoundEventsListMutex[player].lock();

		for (std::list<NetworkMusicData>::iterator eItr = m_MusicEvents[player].begin(); eItr != m_MusicEvents[player].end(); ++eItr) {
			list.push_back((*eItr));
		}

		m_MusicEvents[player].clear();
		g_SoundEventsListMutex[player].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::RegisterSoundEvent(int player, unsigned char state, size_t hash, short int distance, short int channel, short int loops, float pitch, bool affectedByPitch) {

		// Send event to all players
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) {
				RegisterSoundEvent(i, state, hash, distance, channel, loops, pitch, affectedByPitch);
			}
		} else {
			if (player >= 0 && player < c_MaxClients) {
				NetworkSoundData d;
				d.State = state;
				d.Distance = distance;
				d.SoundHash = hash;
				d.Channel = channel;
				d.Loops = loops;
				d.Pitch = pitch;
				d.AffectedByPitch = affectedByPitch ? 1 : 0;

				g_SoundEventsListMutex[player].lock();
				m_SoundEvents[player].push_back(d);
				g_SoundEventsListMutex[player].unlock();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::RegisterMusicEvent(int player, unsigned char state, const char *filepath, int loops, double position, float pitch) {

		// Send event to all players
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) {
				RegisterMusicEvent(i, state, filepath, loops, position, pitch);
			}
		} else {
			NetworkMusicData d;
			d.State = state;
			d.Loops = loops;
			d.Pitch = pitch;
			d.Position = position;
			if (filepath) {
				strncpy(d.Path, filepath, 255);
			} else {
				memset(d.Path, 0, 255);
			}

			g_SoundEventsListMutex[player].lock();
			m_MusicEvents[player].push_back(d);
			g_SoundEventsListMutex[player].unlock();
		}
	}
}
