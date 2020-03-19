#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "SoundContainer.h"
#include "GUISound.h"

namespace RTE {
	const std::string AudioMan::m_ClassName = "AudioMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT F_CALLBACK AudioMan::MusicChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *unusedCommandData1, void *unusedCommandData2) {
		if (channelControlType == FMOD_CHANNELCONTROL_CHANNEL && callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
			g_AudioMan.PlayNextStream();
		}
		return FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT F_CALLBACK AudioMan::SoundChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *unusedCommandData1, void *unusedCommandData2) {
		if (channelControlType == FMOD_CHANNELCONTROL_CHANNEL && callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
			FMOD::Channel *channel = (FMOD::Channel *) channelControl;
			int channelIndex;
			FMOD_RESULT result = channel->getIndex(&channelIndex);

			void *userData;
			result == FMOD_OK ? channel->getUserData(&userData) : result;
			SoundContainer *channelSoundContainer = (SoundContainer *)userData;
			//Account for PlaySound that doesn't create a SoundContainer
			if (channelSoundContainer != NULL) {
				channelSoundContainer->RemovePlayingChannel(channelIndex);
			}

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: An error occurred when Ending a sound in SoundContainer " + channelSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
				return result;
			}
		}
		return FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Clear() {
		m_AudioEnabled = false;

		m_MusicPath.clear();
		m_SoundsVolume = 1.0;
		m_MusicVolume = 1.0;
		m_GlobalPitch = 1.0;

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
		FMOD_RESULT soundSystemSetupResult = FMOD::System_Create(&m_AudioSystem);

		//TODO 44.1 kHz came from data, fmod defaults to 48 kHz, see if we can just use this instead??
		m_AudioSystem->setSoftwareFormat(44100, FMOD_SPEAKERMODE_DEFAULT, 0);

		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->init(g_SettingsMan.GetAudioChannels(), FMOD_INIT_NORMAL, 0) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->getMasterChannelGroup(&m_MasterChannelGroup) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->createChannelGroup("Music", &m_MusicChannelGroup) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->createChannelGroup("Sounds", &m_SoundChannelGroup) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_MasterChannelGroup->addGroup(m_MusicChannelGroup) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_MasterChannelGroup->addGroup(m_SoundChannelGroup) : soundSystemSetupResult;
		
		m_AudioEnabled = true;
		if (soundSystemSetupResult != FMOD_OK) {
			m_AudioEnabled = false;
			return -1;
		}
		
		SetGlobalPitch(m_GlobalPitch);
		SetSoundsVolume(m_SoundsVolume);
		SetMusicVolume(m_MusicVolume);

		new GUISound(); //NOTE: Anything that instantiates SoundContainers needs to wait until the Audio System is up and running before they start doing that. It'll fail safely even if Audio is not enabled.

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Destroy() {
		if (m_AudioEnabled) {
			StopAll();
			m_AudioSystem->release();
			Clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Update() {
		if (m_AudioEnabled) {
			m_AudioSystem->update();

			// Done waiting for silence
			if (!IsMusicPlaying() && m_SilenceTimer.IsPastRealTimeLimit()) { PlayNextStream(); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetGlobalPitch(double pitch, bool excludeMusic) {
		if (!m_AudioEnabled) {
			return;
		}

		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_PITCH, 0, 0, std::unordered_set<short int>(), 0, pitch, excludeMusic); }

		m_GlobalPitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down

		if (!excludeMusic) { m_MusicChannelGroup->setPitch(m_GlobalPitch); }

		int numChannels;
		FMOD_RESULT result = m_SoundChannelGroup->getNumChannels(&numChannels);
		if (result == FMOD_OK) {
			FMOD::Channel *soundChannel;
			bool isPlaying;
			for (int i = 0; i < numChannels; i++) {
				result = m_SoundChannelGroup->getChannel(i, &soundChannel);
				result = result == FMOD_OK ? soundChannel->isPlaying(&isPlaying) : result;

				if (result == FMOD_OK && isPlaying) {
					void *userData;
					result == FMOD_OK ? soundChannel->getUserData(&userData) : result;
					SoundContainer *channelSoundContainer = (SoundContainer *)userData;

					if (channelSoundContainer->IsAffectedByGlobalPitch()) { soundChannel->setPitch(pitch); }
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetTempMusicVolume(double volume) {
		if (m_AudioEnabled && IsMusicPlaying()) {
			FMOD::Channel *musicChannel;
			FMOD_RESULT result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = result == FMOD_OK ? musicChannel->setVolume(Limit(volume, 1, 0)) : result;

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set temporary volume for current music track: " + std::string(FMOD_ErrorString(result)));
			}
		}
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetMusicPitch(float pitch) {
		if (!m_AudioEnabled) {
			return false;
		}

		if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_SET_PITCH, 0, 0, 0.0, pitch); }

		pitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down
		FMOD_RESULT result = m_MusicChannelGroup->setPitch(pitch);

		if (result != FMOD_OK) {
			g_ConsoleMan.PrintString("ERROR: Could not set music pitch: " + std::string(FMOD_ErrorString(result)));
		}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double AudioMan::GetMusicPosition() {
		if (m_AudioEnabled || IsMusicPlaying()) {
			FMOD_RESULT result;
			FMOD::Channel *musicChannel;
			unsigned int position;

			result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = result == FMOD_OK ? musicChannel->getPosition(&position, FMOD_TIMEUNIT_MS) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not get music position: " + std::string(FMOD_ErrorString(result)));
			}
			return result == FMOD_OK ? (static_cast<double>(position)) / 1000 : 0;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicPosition(double position) {
		if (m_AudioEnabled || IsMusicPlaying()) {
			FMOD::Channel *musicChannel;
			FMOD_RESULT result = m_MusicChannelGroup->getChannel(0, &musicChannel);

			FMOD::Sound *musicSound;
			result = result == FMOD_OK ? musicChannel->getCurrentSound(&musicSound) : result;

			unsigned int musicLength;
			result = result == FMOD_OK ? musicSound->getLength(&musicLength, FMOD_TIMEUNIT_MS) : result;

			position = static_cast<unsigned int>(Limit(position, musicLength, 0));
			result = result == FMOD_OK ? musicChannel->setPosition(position, FMOD_TIMEUNIT_MS) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set music position: " + std::string(FMOD_ErrorString(result)));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundAttenuation(SoundContainer *pSoundContainer, float distance) {
		if (!m_AudioEnabled || !pSoundContainer) {
			return false;
		}

		distance = Limit(distance, 0.95, 0); //Limit distance so it can't be closer than 0 (no attenuation) or farther than 0.95 (quiet but not quite silent)

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;

		std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
		for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
			result = result == FMOD_OK ? soundChannel->setVolume(1 - distance) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set sound attenuation for the sound being played on channel " + std::to_string(*channelIterator) + " for SoundContainer " + pSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}

		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundPitch(SoundContainer *pSoundContainer, float pitch) {
		if (!m_AudioEnabled || !pSoundContainer || !pSoundContainer->IsBeingPlayed()) {
			return false;
		}

		if (IsInMultiplayerMode() && pSoundContainer) {
			RegisterSoundEvent(-1, SOUND_SET_PITCH, pSoundContainer->GetHash(), 0, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), pitch, pSoundContainer->IsAffectedByGlobalPitch());
		}

		pitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;

		std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
		for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
			if (result == FMOD_OK) {
				soundChannel->setPitch(pitch);
			}
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayMusic(const char *filePath, int loops, double volumeOverrideIfNotMuted) {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_PLAY, filePath, loops, 0.0, 1.0); }

			FMOD_RESULT result = m_MusicChannelGroup->stop();
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not stop existing music to play new music: " + std::string(FMOD_ErrorString(result)));
				return;
			}

			FMOD::Sound *musicStream;

			result = m_AudioSystem->createStream(filePath, (loops == 0 || loops == 1) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL, nullptr, &musicStream);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not open music file " + std::string(filePath) + ": " + std::string(FMOD_ErrorString(result)));
				return;
			}

			result = musicStream->setLoopCount(loops);
			if (result != FMOD_OK && (loops != 0 && loops != 1)) {
				g_ConsoleMan.PrintString("ERROR: Failed to set looping for music file: " + std::string(filePath) + ". This means it will only play 1 time, instead of " + (loops == 0 ? "looping endlessly." : loops + " times.") + std::string(FMOD_ErrorString(result)));
			}

			FMOD::Channel *musicChannel;
			result = m_AudioSystem->playSound(musicStream, m_MusicChannelGroup, true, &musicChannel);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not play music file: " + std::string(filePath));
				return;
			}

			if (volumeOverrideIfNotMuted >= 0 && m_MusicVolume > 0) {
				result = musicChannel->setVolume(volumeOverrideIfNotMuted);
				if (result != FMOD_OK && (loops != 0 && loops != 1)) {
					g_ConsoleMan.PrintString("ERROR: Failed to set volume override for music file: " + std::string(filePath) + ". This means it will stay at " + std::to_string(m_MusicVolume) + ": " + std::string(FMOD_ErrorString(result)));
				}
			}

			m_MusicPath = filePath;

			result = musicChannel->setCallback(MusicChannelEndedCallback);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Failed to set callback for music ending. This means no more music in the music playlist will play after this one is finished: " + std::string(FMOD_ErrorString(result)));
				return;
			}

			result = musicChannel->setPaused(false);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Failed to start playing music after setting it up: " + std::string(FMOD_ErrorString(result)));
				return;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayNextStream() {
		if (m_AudioEnabled && !m_MusicPlayList.empty()) {
			std::string nextString = m_MusicPlayList.front();
			m_MusicPlayList.pop_front();

			// Look for special encoding if we are supposed to have a silence between tracks
			if (nextString.c_str()[0] == '@') {
				// Decipher the number of secs we're supposed to wait
				int seconds = 0;
				sscanf(nextString.c_str(), "@%i", &seconds);
				m_SilenceTimer.SetRealTimeLimitS(seconds > 0 ? seconds : 0);
				m_SilenceTimer.Reset();

				bool isPlaying;
				FMOD_RESULT result = m_MusicChannelGroup->isPlaying(&isPlaying);
				if (result == FMOD_OK && isPlaying) {
					RegisterMusicEvent(-1, MUSIC_SILENCE, 0, seconds, 0.0, 1.0);
					result = m_MusicChannelGroup->stop();
				}
				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("ERROR: Could not set play silence as specified in music queue, when trying to play next stream: " + std::string(FMOD_ErrorString(result)));
				}
			} else {
				PlayMusic(nextString.c_str(), m_MusicPlayList.empty() ? -1 : 0); //Loop music if it's the last track in the playlist, otherwise just go to the next one
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopMusic() {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_STOP, 0, 0, 0.0, 0.0); }

			FMOD_RESULT result = m_MusicChannelGroup->stop();
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not stop music: " + std::string(FMOD_ErrorString(result)));
			}
			m_MusicPlayList.clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::QueueMusicStream(const char *filepath) {
		if (m_AudioEnabled) {
			bool isPlaying;
			FMOD_RESULT result = m_MusicChannelGroup->isPlaying(&isPlaying);

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not queue music stream: " + std::string(FMOD_ErrorString(result)));
			} else if (!isPlaying) {
				PlayMusic(filepath);
			} else {
				m_MusicPlayList.push_back(std::string(filepath));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::QueueSilence(int seconds) {
		if (m_AudioEnabled && seconds > 0) {
			// Encode the silence as number of secs preceded by '@'
			char str[256];
			sprintf_s(str, sizeof(str), "@%i", seconds);
			m_MusicPlayList.push_back(std::string(str));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SoundContainer *AudioMan::PlaySound(const char *filePath, float attenuation, int player, int loops, int priority, double pitchOrAffectedByGlobalPitch) {
		if (!filePath) {
			g_ConsoleMan.PrintString("Error: Null filepath passed to AudioMan::PlaySound!");
			return 0;
		}

		bool affectedByGlobalPitch = pitchOrAffectedByGlobalPitch == -1;
		double pitch = affectedByGlobalPitch ? m_GlobalPitch : pitchOrAffectedByGlobalPitch;

		SoundContainer *newSoundContainer = new SoundContainer();
		newSoundContainer->Create(filePath, loops, affectedByGlobalPitch);
		PlaySound(newSoundContainer, attenuation, player, priority, pitch);

		return newSoundContainer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySound(SoundContainer *pSoundContainer, float attenuation, int player, int priority, double pitch) {
		if (!m_AudioEnabled || !pSoundContainer) {
			return false;
		}

		priority = priority < 0 ? pSoundContainer->GetPriority() : priority;
		pitch = pSoundContainer->IsAffectedByGlobalPitch() ? m_GlobalPitch : pitch;

		FMOD::Channel *channel;
		int channelIndex;
		FMOD_RESULT result = m_AudioSystem->playSound(pSoundContainer->SelectNextSound(), m_SoundChannelGroup, true, &channel);
		result = result == FMOD_OK ? channel->getIndex(&channelIndex) : result;
		result = result == FMOD_OK ? channel->setUserData(pSoundContainer) : result;
		result = result == FMOD_OK ? channel->setCallback(SoundChannelEndedCallback) : result;
		result = result == FMOD_OK ? channel->setLoopCount(pSoundContainer->GetLoopSetting()) : result;
		result = result == FMOD_OK ? channel->setPriority(priority) : result;

		if (result != FMOD_OK) {
			g_ConsoleMan.PrintString("ERROR: Could not play sounds from SoundContainer " + pSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			return false;
		}

		pSoundContainer->AddPlayingChannel(channelIndex);
		SetSoundAttenuation(pSoundContainer, attenuation);
		if (pitch != 1) { SetSoundPitch(pSoundContainer, pitch); }

		result = channel->setPaused(false);
		if (result != FMOD_OK) {
			g_ConsoleMan.PrintString("ERROR: Failed to start playing sounds from SoundContainer " + pSoundContainer->GetPresetName() + " after setting it up: " + std::string(FMOD_ErrorString(result)));
		}

		// Now that the sound is playing we can register an event with the SoundContainer's channels, which can be used by clients to identify the sound being played.
		if (m_IsInMultiplayerMode) {
			RegisterSoundEvent(player, SOUND_PLAY, pSoundContainer->GetHash(), attenuation, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), pitch, pSoundContainer->IsAffectedByGlobalPitch());
		}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(SoundContainer *pSoundContainer) {
		if (!m_AudioEnabled || !pSoundContainer) {
			return false;
		}

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		bool anySoundsPlaying = pSoundContainer->IsBeingPlayed();

		if (anySoundsPlaying) {
			std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
			for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
				result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
				result = result == FMOD_OK ? soundChannel->stop() : result;
				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("Error: Failed to stop playing channel in SoundContainer "+pSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
				}

			}
		}
		return anySoundsPlaying;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(SoundContainer *pSoundContainer, int player) {
		if (m_IsInMultiplayerMode && pSoundContainer) {
			RegisterSoundEvent(player, SOUND_STOP, pSoundContainer->GetHash(), 0, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), 1.0, pSoundContainer->IsAffectedByGlobalPitch());
		}
		return StopSound(pSoundContainer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::FadeOutSound(SoundContainer *pSoundContainer, int fadeOutTime) {
		if (!m_AudioEnabled || !pSoundContainer || !pSoundContainer->IsBeingPlayed()) {
			return;
		}

		int sampleRate;
		m_AudioSystem->getSoftwareFormat(&sampleRate, nullptr, nullptr);
		int fadeOutTimeAsSamples = fadeOutTime * sampleRate / 1000;

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		unsigned long long parentClock;
		float currentVolume;

		std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
		for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
			result = result == FMOD_OK ? soundChannel->getDSPClock(nullptr, &parentClock) : result;
			result = result == FMOD_OK ? soundChannel->getVolume(&currentVolume) : result;
			result = result == FMOD_OK ? soundChannel->addFadePoint(parentClock, currentVolume) : result;
			result = result == FMOD_OK ? soundChannel->addFadePoint(parentClock + fadeOutTimeAsSamples, 0) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not fade out sounds in SoundContainer " + pSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::GetMusicEvents(int player, std::list<NetworkMusicData> &list) {
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

	void AudioMan::RegisterMusicEvent(int player, unsigned char state, const char *filepath, int loops, double position, float pitch) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) {
				RegisterMusicEvent(i, state, filepath, loops, position, pitch);
			}
		} else {
			NetworkMusicData musicData;
			musicData.State = state;
			musicData.Loops = loops;
			musicData.Pitch = pitch;
			musicData.Position = position;
			if (filepath) {
				strncpy(musicData.Path, filepath, 255);
			} else {
				memset(musicData.Path, 0, 255);
			}

			g_SoundEventsListMutex[player].lock();
			m_MusicEvents[player].push_back(musicData);
			g_SoundEventsListMutex[player].unlock();
		}
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

	void AudioMan::RegisterSoundEvent(int player, unsigned char state, size_t hash, short int attenuation, std::unordered_set<short int> const &channels, short int loops, float pitch, bool affectedByPitch) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) {
				RegisterSoundEvent(i, state, hash, attenuation, channels, loops, pitch, affectedByPitch);
			}
		} else {
			if (player >= 0 && player < c_MaxClients) {
				NetworkSoundData soundData;
				soundData.State = state;
				soundData.Distance = attenuation;
				soundData.SoundHash = hash;
				soundData.Channels = channels;
				soundData.Loops = loops;
				soundData.Pitch = pitch;
				soundData.AffectedByPitch = affectedByPitch ? 1 : 0;

				g_SoundEventsListMutex[player].lock();
				m_SoundEvents[player].push_back(soundData);
				g_SoundEventsListMutex[player].unlock();
			}
		}
	}
}
