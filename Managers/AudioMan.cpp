#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "SoundContainer.h"

namespace RTE {
	const std::string AudioMan::m_ClassName = "AudioMan";

	// I know this is a crime, but if I include it in FrameMan.h the whole thing will collapse due to int redefinitions in Allegro
	std::mutex g_SoundEventsListMutex[c_MaxClients];

	FMOD_RESULT F_CALLBACK AudioMan::MusicChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *unusedCommandData1, void *unusedCommandData2) {
		if (channelControlType == FMOD_CHANNELCONTROL_CHANNEL && callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
			FMOD::Channel *channel = (FMOD::Channel *) channelControl;

			FMOD_RESULT result = channel->setVolume(g_AudioMan.m_MusicVolume);
			g_AudioMan.PlayNextStream();

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: An error occured in the End Music callback.");
				return result;
			}
		}
		return FMOD_OK;
	}

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

			//TODO this stuff below might be unnecessary
			result = result == FMOD_OK ? channel->setPitch(1.0) : result;
			result = result == FMOD_OK ? channel->removeFadePoints(0, 18446744073709551615) : result; //Remove all possible fade points
			result = result == FMOD_OK ? channel->setVolume(Instance().m_SoundsVolume) : result;

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: An error occured in the End Sound callback for a sound in SoundContainer " + channelSoundContainer->GetPresetName() + ". This is probably bad!");
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
		FMOD_RESULT soundSystemSetupResult = FMOD::System_Create(&m_AudioSystem);

		m_AudioSystem->setSoftwareChannels(g_SettingsMan.GetAudioChannels());
		//TODO 44.1 kHz came from data, fmod defaults to 48 kHz, see if we can just use this instead??
		m_AudioSystem->setSoftwareFormat(44100, FMOD_SPEAKERMODE_DEFAULT, 0);

		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->init(512, FMOD_INIT_NORMAL, 0) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->createChannelGroup("Music", &m_MusicChannelGroup) : soundSystemSetupResult;
		soundSystemSetupResult = soundSystemSetupResult == FMOD_OK ? m_AudioSystem->createChannelGroup("Sounds", &m_SoundChannelGroup) : soundSystemSetupResult;
		
		m_AudioEnabled = true;
		if (soundSystemSetupResult != FMOD_OK) {
			m_AudioEnabled = false;
			return -1;
		}

		//TODO figure out if we want this/if it can be done more automatically with fmod?
		SetGlobalPitch(m_GlobalPitch);
		SetSoundsVolume(m_SoundsVolume);
		SetMusicVolume(m_MusicVolume);

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
			if (!IsMusicPlaying() && m_SilenceTimer.IsPastRealTimeLimit()) {
				PlayNextStream();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetSoundsVolume(double volume) {
		m_SoundsVolume = volume;

		if (m_AudioEnabled) {
			m_SoundChannelGroup->setVolume(volume);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicVolume(double volume) {
		m_MusicVolume = volume;

		if (m_AudioEnabled) {
			m_MusicChannelGroup->setVolume(volume);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetTempMusicVolume(double volume) {
		if (m_AudioEnabled && IsMusicPlaying()) {
			FMOD::Channel *musicChannel;
			FMOD_RESULT result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = result == FMOD_OK ? musicChannel->setVolume(volume) : result;

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set temporary volume for current music track.");
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetGlobalPitch(double pitch, bool excludeMusic) {
		if (!m_AudioEnabled) {
			return;
		}

		if (m_IsInMultiplayerMode) {
			RegisterSoundEvent(-1, SOUND_SET_PITCH, 0, 0, std::unordered_set<short int>(), 0, pitch, excludeMusic);
		}

		m_GlobalPitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down

		if (!excludeMusic) {
			m_MusicChannelGroup->setPitch(m_GlobalPitch);
		}

		int numChannels;
		FMOD_RESULT result = m_SoundChannelGroup->getNumChannels(&numChannels);
		if (result == FMOD_OK) {
			FMOD::Channel *soundChannel;
			bool isPlaying;
			for (int i = 0; i < numChannels; i++) {
				result = m_SoundChannelGroup->getChannel(i, &soundChannel);
				result = result == FMOD_OK ? soundChannel->isPlaying(&isPlaying) : result;

				if (result == FMOD_OK && isPlaying) {
					SoundContainer *soundContainer = m_ChannelsToSoundContainers.at(i);

					if (soundContainer->IsAffectedByPitch()) {
						soundChannel->setPitch(pitch);
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundPitch(SoundContainer *pSoundContainer, float pitch) {
		if (!m_AudioEnabled || !pSoundContainer || !pSoundContainer->IsAffectedByPitch() || !IsPlaying(pSoundContainer)) {
			return false;
		}
		if (IsInMultiplayerMode() && pSoundContainer) {
			RegisterSoundEvent(-1, SOUND_SET_PITCH, pSoundContainer->GetHash(), 0, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), pitch, pSoundContainer->IsAffectedByPitch());
		}

		pitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;

		std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
		for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_SoundChannelGroup->getChannel((*channelIterator), &soundChannel);
			if (result == FMOD_OK) {
				soundChannel->setPitch(pitch);
			}
		}
		return true;
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetMusicPitch(float pitch) {
		if (m_IsInMultiplayerMode) {
			RegisterMusicEvent(-1, MUSIC_SET_PITCH, 0, 0, 0.0, pitch);
		}
		if (!m_AudioEnabled) {
			return false;
		}

		pitch = Limit(pitch, 8, 0.125); //Limit pitch change to 8 octaves up or down
		m_MusicChannelGroup->setPitch(pitch);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::SetSoundAttenuation(SoundContainer *pSoundContainer, float distance) {
		if (!m_AudioEnabled || !pSoundContainer) {
			return false;
		}

		distance = Limit(distance, 0.95, 0); //Limit distance so it can't be closer than 0 (no attenuation) or farther than 0.95 (quiet but not quite silent)

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;

		std::unordered_set<short int> channels = pSoundContainer->GetPlayingChannels();
		for (std::unordered_set<short int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_SoundChannelGroup->getChannel((*channelIterator), &soundChannel);
			if (result == FMOD_OK) {
				soundChannel->setVolume(1 - distance);
			}
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::IsPlaying(SoundContainer *pSoundContainer) {
		return pSoundContainer->IsBeingPlayed();
		/*if (!m_AudioEnabled || !pSoundContainer || pSoundContainer->GetSoundCount() <= 0 || pSoundContainer->GetCurrentChannels().size() == 0) {
			return false;
		}

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		bool isPlaying;

		vector<int> channels = pSoundContainer->GetCurrentChannels();
		for (std::vector<int>::iterator channelIterator = channels.begin(); channelIterator != channels.end(); ++channelIterator) {
			result = m_SoundChannelGroup->getChannel((*channelIterator), &soundChannel);
			if (result == FMOD_OK) {
				result = soundChannel->isPlaying(&isPlaying);
				if (result == FMOD_OK && isPlaying) {
					return true;
				}
			}
		}
		return false;*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopAll() {
		if (m_AudioEnabled) {
			m_MusicChannelGroup->stop();
			m_SoundChannelGroup->stop();
		}
		m_MusicPlayList.clear();
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
				result = m_SoundChannelGroup->getChannel((*channelIterator), &soundChannel);
				result = result == FMOD_OK ? soundChannel->stop() : result;
				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("Error: Failed to stop playing channel in SoundContainer "+pSoundContainer->GetPresetName());
				}

			}
		}
		return anySoundsPlaying;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(int player, SoundContainer *pSoundContainer) {
		if (m_IsInMultiplayerMode && pSoundContainer) {
			RegisterSoundEvent(player, SOUND_STOP, pSoundContainer->GetHash(), 0, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), 1.0, pSoundContainer->IsAffectedByPitch());
		}
		return StopSound(pSoundContainer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopMusic() {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) {
				RegisterMusicEvent(-1, MUSIC_STOP, 0, 0, 0.0, 0.0);
			}

			m_MusicChannelGroup->stop();
			m_MusicPlayList.clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double AudioMan::GetMusicPosition() {
		if (m_AudioEnabled || IsMusicPlaying()) {
			FMOD_RESULT result;
			FMOD::Channel *musicChannel;
			unsigned int position;

			result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = result == FMOD_OK ? musicChannel->getPosition(&position, FMOD_TIMEUNIT_MS) : result;
			return result == FMOD_OK ? ((double)position) / 1000 : 0;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicPosition(double position) {
		if (m_AudioEnabled || IsMusicPlaying()) {
			FMOD_RESULT result;
			FMOD::Channel *musicChannel;

			result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = result == FMOD_OK ? musicChannel->setPosition((unsigned int)(position * 1000), FMOD_TIMEUNIT_MS) : result;
		}
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
			result = m_SoundChannelGroup->getChannel((*channelIterator), &soundChannel);
			if (result == FMOD_OK) {
				result = soundChannel->getDSPClock(nullptr, &parentClock);
				result = result == FMOD_OK ? soundChannel->getVolume(&currentVolume) : result;
				result = result == FMOD_OK ? soundChannel->addFadePoint(parentClock, currentVolume) : result;
				result = result == FMOD_OK ? soundChannel->addFadePoint(parentClock + fadeOutTimeAsSamples, 0) : result;
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

	void AudioMan::QueueMusicStream(const char *filepath) {
		if (m_AudioEnabled) {
			bool isPlaying;
			FMOD_RESULT result = m_MusicChannelGroup->isPlaying(&isPlaying);

			if (result == FMOD_OK && !isPlaying) {
				PlayMusic(filepath);
			} else {
				m_MusicPlayList.push_back(std::string(filepath));
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
					m_MusicChannelGroup->stop();
				}
			} else {
				PlayMusic(nextString.c_str(), m_MusicPlayList.empty() ? -1 : 0); //Loop music if it's the last track in the playlist, otherwise just go to the next one
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlaySound(const char *filePath) {
		FMOD::Sound *sound;
		m_AudioSystem->createSound(filePath, FMOD_DEFAULT, nullptr, &sound);
		m_AudioSystem->playSound(sound, m_SoundChannelGroup, false, nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SoundContainer * AudioMan::PlaySound(const char *filePath, float distance, bool loops, bool affectedByPitch, int player) {
		if (!filePath) {
			g_ConsoleMan.PrintString("Error: Null filepath passed to AudioMan::PlaySound!");
			return 0;
		}

		SoundContainer *newSoundContainer = new SoundContainer();
		newSoundContainer->Create(filePath, affectedByPitch, loops ? 1 : 0);
		PlaySound(player, newSoundContainer, PRIORITY_LOW, distance, affectedByPitch ? m_GlobalPitch : 1.0);

		return newSoundContainer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySound(int player, SoundContainer *pSoundContainer, int priority, float distance, double pitch) {
		// We need to play sound before registering an event because it will assign a channel or channels to the sound being played. Clients then use this number to identify the sound being played
		bool success = PlaySound(pSoundContainer, priority, distance, pitch);

		if (m_IsInMultiplayerMode && pSoundContainer) {
			RegisterSoundEvent(player, SOUND_PLAY, pSoundContainer->GetHash(), distance, pSoundContainer->GetPlayingChannels(), pSoundContainer->GetLoopSetting(), pitch, pSoundContainer->IsAffectedByPitch());
		}
		return success;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySound(SoundContainer *pSoundContainer, int priority, float distance, double pitch) {
		if (!m_AudioEnabled || !pSoundContainer) {
			return false;
		}

		FMOD::Channel *channel;
		int channelIndex;
		FMOD_RESULT result = m_AudioSystem->playSound(pSoundContainer->SelectNextSound(), m_SoundChannelGroup, false, &channel);
		result = result == FMOD_OK ? channel->getIndex(&channelIndex) : result;
		result = result == FMOD_OK ? channel->setUserData(pSoundContainer) : result;
		result = result == FMOD_OK ? channel->setCallback(SoundChannelEndedCallback) : result;
		result = result == FMOD_OK ? channel->setLoopCount(pSoundContainer->GetLoopSetting()) : result;
		result = result == FMOD_OK ? channel->setPriority(pSoundContainer->GetPriority()) : result;

		if (result != FMOD_OK) {
			g_ConsoleMan.PrintString("ERROR: Could not play sounds from SoundContainer "+pSoundContainer->GetPresetName());
			return false;
		}

		pSoundContainer->AddPlayingChannel(channelIndex);
		SetSoundAttenuation(pSoundContainer, distance);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayMusic(const char *filePath, int loops, double volumeOverrideIfNotMuted) {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) {
				RegisterMusicEvent(-1, MUSIC_PLAY, filePath, loops, 0.0, 1.0);
			}

			FMOD_RESULT result = m_MusicChannelGroup->stop();
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not stop existing music to play new music.");
				return;
			}

			FMOD::Sound *musicStream;

			result = m_AudioSystem->createStream(filePath, (loops == 0 || loops == 1) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL, nullptr, &musicStream);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not open music file: " + std::string(filePath));
				return;
			}

			result = musicStream->setLoopCount(loops);
			if (result != FMOD_OK && (loops != 0 && loops != 1)) {
				g_ConsoleMan.PrintString("ERROR: Failed to set looping for music file: " + std::string(filePath) + ". This means it will only play 1 time, instead of " + (loops == 0 ? "looping endlessly." : loops + " times."));
			}

			FMOD::Channel *musicChannel;
			result = m_AudioSystem->playSound(musicStream, m_MusicChannelGroup, false, &musicChannel);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not play music file: " + std::string(filePath));
				return;
			}

			if (volumeOverrideIfNotMuted >= 0 && m_MusicVolume > 0) {
				result = musicChannel->setVolume(volumeOverrideIfNotMuted);
				if (result != FMOD_OK && (loops != 0 && loops != 1)) {
					g_ConsoleMan.PrintString("ERROR: Failed to set vollume override for music file: " + std::string(filePath) + ". This means it will at the current music volume.");
				}
			}

			m_MusicPath = filePath;

			result = musicChannel->setCallback(MusicChannelEndedCallback);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Failed to set callback for music ending. This means no more music in the music playlist will play after this one is finished.");
				return;
			}
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

	void AudioMan::RegisterSoundEvent(int player, unsigned char state, size_t hash, short int distance, std::unordered_set<short int> channels, short int loops, float pitch, bool affectedByPitch) {

		// Send event to all players
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) {
				RegisterSoundEvent(i, state, hash, distance, channels, loops, pitch, affectedByPitch);
			}
		} else {
			if (player >= 0 && player < c_MaxClients) {
				NetworkSoundData d;
				d.State = state;
				d.Distance = distance;
				d.SoundHash = hash;
				d.Channels = channels;
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
