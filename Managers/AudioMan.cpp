#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "SoundContainer.h"
#include "GUISound.h"

namespace RTE {

	const std::string AudioMan::c_ClassName = "AudioMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Clear() {
		m_AudioEnabled = false;

		m_SoundChannelRolloffs.clear();
		m_MusicVolume = 1.0;
		m_SoundsVolume = 1.0;
		m_GlobalPitch = 1.0;

		m_MusicPath.clear();
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
		FMOD_RESULT audioSystemSetupResult = FMOD::System_Create(&m_AudioSystem);
		
		FMOD_ADVANCEDSETTINGS audioSystemAdvancedSettings;
		memset(&audioSystemAdvancedSettings, 0, sizeof(audioSystemAdvancedSettings));
		audioSystemAdvancedSettings.cbSize = sizeof(FMOD_ADVANCEDSETTINGS);
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->getAdvancedSettings(&audioSystemAdvancedSettings) : audioSystemSetupResult;
		audioSystemAdvancedSettings.vol0virtualvol = 0.001F;
		audioSystemAdvancedSettings.randomSeed = RandomNum(0, INT_MAX);

		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->setAdvancedSettings(&audioSystemAdvancedSettings) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->set3DSettings(1, c_PPM, 1) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->setSoftwareChannels(c_MaxSoftwareChannels) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->init(c_MaxVirtualChannels, FMOD_INIT_VOL0_BECOMES_VIRTUAL, 0) : audioSystemSetupResult;

		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->getMasterChannelGroup(&m_MasterChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->createChannelGroup("Music", &m_MusicChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->createChannelGroup("Sounds", &m_SoundChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->createChannelGroup("MobileSounds", &m_MobileSoundChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_AudioSystem->createChannelGroup("ImmobileSounds", &m_ImmobileSoundChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_MasterChannelGroup->addGroup(m_MusicChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_MasterChannelGroup->addGroup(m_SoundChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_SoundChannelGroup->addGroup(m_MobileSoundChannelGroup) : audioSystemSetupResult;
		audioSystemSetupResult = (audioSystemSetupResult == FMOD_OK) ? m_SoundChannelGroup->addGroup(m_ImmobileSoundChannelGroup) : audioSystemSetupResult;
		
		m_AudioEnabled = audioSystemSetupResult == FMOD_OK;

		if (!m_AudioEnabled) {
			return -1;
		}

		SetGlobalPitch(m_GlobalPitch, false, false);
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

			//TODO allow setting vel for AEmitter and PEmitter. Also maybe consider setting vel on listener when SceneMan target scrolling is happening.

			FMOD_RESULT status = FMOD_OK;

			if (g_ActivityMan.ActivityRunning()) {
				const Activity *currentActivity = g_ActivityMan.GetActivity();

				if (m_CurrentActivityHumanCount != (m_IsInMultiplayerMode ? 1 : currentActivity->GetHumanCount())) {
					m_CurrentActivityHumanCount = m_IsInMultiplayerMode ? 1 : currentActivity->GetHumanCount();
					status = status == FMOD_OK ? m_AudioSystem->set3DNumListeners(m_CurrentActivityHumanCount) : status;
				}

				int audioSystemPlayerNumber = 0;
				for (short player = Players::PlayerOne; player < currentActivity->GetPlayerCount() && audioSystemPlayerNumber < m_CurrentActivityHumanCount; player++) {
					if (currentActivity->PlayerHuman(player)) {
						status = status == FMOD_OK ? m_AudioSystem->set3DListenerAttributes(audioSystemPlayerNumber, &GetAsFMODVector(g_SceneMan.GetScrollTarget(currentActivity->ScreenOfPlayer(player)), g_SettingsMan.c_ListenerZOffset()), NULL, &c_FMODForward, &c_FMODUp) : status;
						audioSystemPlayerNumber++; 
					}
				}

				if (g_SettingsMan.SoundPanningEffectStrength() < 1) { UpdateCalculated3DEffectsForMobileSoundChannels(); }
			} else {
				if (m_CurrentActivityHumanCount != 1) {
					m_CurrentActivityHumanCount = 1;
					status = status == FMOD_OK ? m_AudioSystem->set3DNumListeners(1) : status;
				}
				status = status == FMOD_OK ? m_AudioSystem->set3DListenerAttributes(0, &GetAsFMODVector(g_SceneMan.GetScrollTarget(), g_SettingsMan.c_ListenerZOffset()), NULL, &c_FMODForward, &c_FMODUp) : status;
			}

			status = status == FMOD_OK ? m_AudioSystem->update() : status;

			if (!IsMusicPlaying() && m_SilenceTimer.IsPastRealTimeLimit()) { PlayNextStream(); }
			if (status != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not update AudioMan due to FMOD error: " + std::string(FMOD_ErrorString(status))); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetGlobalPitch(float pitch, bool includeImmobileSounds, bool includeMusic) {
		if (!m_AudioEnabled) {
			return;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_GLOBAL_PITCH, nullptr, nullptr, Vector(), 0, 0.0F, pitch); }

		m_GlobalPitch = std::clamp(pitch, 0.125F, 8.0F); 
		if (includeMusic) { m_MusicChannelGroup->setPitch(m_GlobalPitch); }

		FMOD::ChannelGroup *channelGroupToUse = includeImmobileSounds ? m_SoundChannelGroup : m_MobileSoundChannelGroup;
		channelGroupToUse->setPitch(m_GlobalPitch);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetTempMusicVolume(float volume) {
		if (m_AudioEnabled && IsMusicPlaying()) {
			FMOD::Channel *musicChannel;
			FMOD_RESULT result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = (result == FMOD_OK) ? musicChannel->setVolume(std::clamp(volume, 0.0F, 1.0F)) : result;

			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not set temporary volume for current music track: " + std::string(FMOD_ErrorString(result))); }
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

		if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not set music pitch: " + std::string(FMOD_ErrorString(result))); }

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float AudioMan::GetMusicPosition() const {
		if (m_AudioEnabled && IsMusicPlaying()) {
			FMOD_RESULT result;
			FMOD::Channel *musicChannel;
			unsigned int position;

			result = m_MusicChannelGroup->getChannel(0, &musicChannel);
			result = (result == FMOD_OK) ? musicChannel->getPosition(&position, FMOD_TIMEUNIT_MS) : result;
			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not get music position: " + std::string(FMOD_ErrorString(result))); }

			return (result == FMOD_OK) ? (static_cast<float>(position)) / 1000.0F : 0;
		}
		return 0.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::SetMusicPosition(float position) {
		if (m_AudioEnabled && IsMusicPlaying()) {
			FMOD::Channel *musicChannel;
			FMOD_RESULT result = m_MusicChannelGroup->getChannel(0, &musicChannel);

			FMOD::Sound *musicSound;
			result = (result == FMOD_OK) ? musicChannel->getCurrentSound(&musicSound) : result;

			unsigned int musicLength;
			result = (result == FMOD_OK) ? musicSound->getLength(&musicLength, FMOD_TIMEUNIT_MS) : result;

			position = std::clamp(position, 0.0F, static_cast<float>(musicLength)) / 1000.0F;
			result = (result == FMOD_OK) ? musicChannel->setPosition(static_cast<unsigned int>(position), FMOD_TIMEUNIT_MS) : result;

			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not set music position: " + std::string(FMOD_ErrorString(result))); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::PlayMusic(const char *filePath, int loops, float volumeOverrideIfNotMuted) {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_PLAY, filePath, loops); }

			FMOD_RESULT result = m_MusicChannelGroup->stop();
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not stop existing music to play new music: " + std::string(FMOD_ErrorString(result)));
				return;
			}

			FMOD::Sound *musicStream;

			result = m_AudioSystem->createStream(filePath, FMOD_3D_HEADRELATIVE | ((loops == 0 || loops == 1) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL), nullptr, &musicStream);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not open music file " + std::string(filePath) + ": " + std::string(FMOD_ErrorString(result)));
				return;
			}
			
			result = musicStream->setLoopCount(loops);
			if (result != FMOD_OK && (loops != 0 && loops != 1)) {
				g_ConsoleMan.PrintString("ERROR: Failed to set looping for music file: " + std::string(filePath) + ". This means it will only play 1 time, instead of " + (loops == 0 ? "looping endlessly." : loops + " times.") + std::string(FMOD_ErrorString(result)));
			}

			FMOD::Channel *musicChannel;
			result = musicStream->set3DMinMaxDistance(c_SoundMaxAudibleDistance, c_SoundMaxAudibleDistance);
			result = (result == FMOD_OK) ? m_AudioSystem->playSound(musicStream, m_MusicChannelGroup, true, &musicChannel) : result;
			result = (result == FMOD_OK) ? musicChannel->set3DAttributes(&GetAsFMODVector(Vector()), nullptr) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not play music file: " + std::string(filePath) + ": " + std::string(FMOD_ErrorString(result)));
				return;
			}
			result = musicChannel->setPriority(PRIORITY_HIGH);
			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Failed to set music as high priority when playing music file."); }

			if (volumeOverrideIfNotMuted >= 0.0F && m_MusicVolume > 0.0F) {
				volumeOverrideIfNotMuted = std::clamp((volumeOverrideIfNotMuted > 1.0F ? volumeOverrideIfNotMuted / 100.0F : volumeOverrideIfNotMuted), 0.0F, 1.0F);
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

			if (!nextString.empty() && nextString.at(0) == '@') {
				try {
					int seconds = std::stoi(nextString.substr(1, nextString.size()));
					m_SilenceTimer.SetRealTimeLimitS((seconds > 0) ? seconds : 0);
					m_SilenceTimer.Reset();

					bool isPlaying;
					FMOD_RESULT result = m_MusicChannelGroup->isPlaying(&isPlaying);
					if (result == FMOD_OK && isPlaying) {
						if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_SILENCE, nullptr, seconds); }
						result = m_MusicChannelGroup->stop();
					}
					if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not play silence as specified in music queue, when trying to play next stream: " + std::string(FMOD_ErrorString(result))); }
				} catch (const std::invalid_argument &) {
					g_ConsoleMan.PrintString("ERROR: Could invalid silence specification when trying to play next stream.");
				}
			} else {
				PlayMusic(nextString.c_str(), m_MusicPlayList.empty() ? -1 : 0); 
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::StopMusic() {
		if (m_AudioEnabled) {
			if (m_IsInMultiplayerMode) { RegisterMusicEvent(-1, MUSIC_STOP, 0, 0, 0.0, 0.0); }

			FMOD_RESULT result = m_MusicChannelGroup->stop();
			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not stop music: " + std::string(FMOD_ErrorString(result))); }
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

	SoundContainer *AudioMan::PlaySound(const std::string &filePath, const Vector &position, int player) {
		SoundContainer *newSoundContainer = new SoundContainer();
		newSoundContainer->SetPosition(position);
		newSoundContainer->AddSound(filePath);
		if (newSoundContainer->HasAnySounds()) {
			PlaySoundContainer(newSoundContainer, player);
		} else {
			delete newSoundContainer;
		}
		return newSoundContainer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::StopSound(SoundContainer *soundContainer, int player) {
		if (!m_AudioEnabled || !soundContainer) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(player, SOUND_STOP, soundContainer->GetPlayingChannels()); }

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		bool anySoundsPlaying = soundContainer->IsBeingPlayed();

		if (anySoundsPlaying) {
			const std::unordered_set<int> *channels = soundContainer->GetPlayingChannels();
			for (std::unordered_set<int>::const_iterator channelIterator = channels->begin(); channelIterator != channels->end();) {
				result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
				++channelIterator; // NOTE - stopping the sound will remove the channel, screwing things up if we don't move to the next iterator preemptively
				result = (result == FMOD_OK) ? soundChannel->stop() : result;
				if (result != FMOD_OK) { g_ConsoleMan.PrintString("Error: Failed to stop playing channel in SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result))); }
			}
		}
		return anySoundsPlaying;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::FadeOutSound(SoundContainer *soundContainer, int fadeOutTime) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_FADE_OUT, soundContainer->GetPlayingChannels(), &soundContainer->GetSelectedSoundHashes(), Vector(), 0, 0, 0, false, 0, false, fadeOutTime); }

		int sampleRate;
		m_AudioSystem->getSoftwareFormat(&sampleRate, nullptr, nullptr);
		int fadeOutTimeAsSamples = fadeOutTime * sampleRate / 1000;

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		unsigned long long parentClock;
		float currentVolume;

		const std::unordered_set<int> channels = *soundContainer->GetPlayingChannels();
		for (int channel : channels) {
			result = m_AudioSystem->getChannel(channel, &soundChannel);
			result = (result == FMOD_OK) ? soundChannel->getDSPClock(nullptr, &parentClock) : result;
			result = (result == FMOD_OK) ? soundChannel->getVolume(&currentVolume) : result;
			result = (result == FMOD_OK) ? soundChannel->addFadePoint(parentClock, currentVolume) : result;
			result = (result == FMOD_OK) ? soundChannel->addFadePoint(parentClock + fadeOutTimeAsSamples, 0) : result;

			if (result != FMOD_OK) { g_ConsoleMan.PrintString("ERROR: Could not fade out sounds in SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result))); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::GetMusicEvents(int player, std::list<NetworkMusicData> &list) {
		if (player < 0 || player >= c_MaxClients) {
			return;
		}
		list.clear();
		g_SoundEventsListMutex[player].lock();

		for (const NetworkMusicData &musicEvent : m_MusicEvents[player]) { list.push_back(musicEvent); }
		m_MusicEvents[player].clear();
		g_SoundEventsListMutex[player].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::RegisterMusicEvent(int player, NetworkMusicState state, const char *filepath, int loops, float position, float pitch) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) { RegisterMusicEvent(i, state, filepath, loops, position, pitch); }
		} else {
			NetworkMusicData musicData;
			musicData.State = state;
			musicData.Loops = loops;
			musicData.Pitch = pitch;
			musicData.Position = position;
			if (filepath) {
				std::strncpy(musicData.Path, filepath, 255);
			} else {
				std::memset(musicData.Path, 0, 255);
			}
			g_SoundEventsListMutex[player].lock();
			m_MusicEvents[player].push_back(musicData);
			g_SoundEventsListMutex[player].unlock();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::GetSoundEvents(int player, std::list<NetworkSoundData> &list) {
		if (player < 0 || player >= c_MaxClients) {
			return;
		}
		list.clear();
		g_SoundEventsListMutex[player].lock();

		for (const NetworkSoundData &soundEvent : m_SoundEvents[player]) { list.push_back(soundEvent); }
		m_SoundEvents[player].clear();
		g_SoundEventsListMutex[player].unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::RegisterSoundEvent(int player, NetworkSoundState state, const std::unordered_set<int> *channels, const std::vector<size_t> *soundFileHashes, const Vector &position, int loops, float volume, float pitch, bool affectedByGlobalPitch, float attenuationStartDistance, bool immobile, int fadeOutTime) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) { RegisterSoundEvent(i, state, channels, soundFileHashes, position, loops, volume, pitch, affectedByGlobalPitch, attenuationStartDistance, immobile, fadeOutTime); }
		} else {
			if (player >= 0 && player < c_MaxClients) {
				NetworkSoundData soundData;
				soundData.State = state;

				std::fill_n(soundData.Channels, c_MaxPlayingSoundsPerContainer, c_MaxVirtualChannels + 1);
				if (channels) { std::copy(channels->begin(), channels->end(), soundData.Channels); }

				std::fill_n(soundData.SoundFileHashes, c_MaxPlayingSoundsPerContainer, 0);
				if (soundFileHashes) { std::copy(soundFileHashes->begin(), soundFileHashes->end(), soundData.SoundFileHashes); }

				soundData.Position[0] = position.m_X;
				soundData.Position[1] = position.m_Y;
				soundData.Loops = loops;
				soundData.Volume = volume;
				soundData.Pitch = pitch;
				soundData.AffectedByGlobalPitch = affectedByGlobalPitch;
				soundData.AttenuationStartDistance = attenuationStartDistance;
				soundData.Immobile = immobile;
				soundData.FadeOutTime = fadeOutTime;

				g_SoundEventsListMutex[player].lock();
				m_SoundEvents[player].push_back(soundData);
				g_SoundEventsListMutex[player].unlock();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::PlaySoundContainer(SoundContainer *soundContainer, int player) {
		if (!m_AudioEnabled || !soundContainer || soundContainer->GetPlayingChannels()->size() >= c_MaxPlayingSoundsPerContainer) {
			return false;
		}
		FMOD_RESULT result = FMOD_OK;

		if (!soundContainer->SoundPropertiesUpToDate()) {
			result = soundContainer->UpdateSoundProperties();
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not update sound properties for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
				return false;
			}
		}
		if (!soundContainer->SelectNextSoundSet()) {
			g_ConsoleMan.PrintString("Unable to select new sounds to play for SoundContainer " + soundContainer->GetPresetName());
			return false;
		}
		FMOD::ChannelGroup *channelGroupToPlayIn = soundContainer->IsImmobile() ? m_ImmobileSoundChannelGroup : m_MobileSoundChannelGroup;
		FMOD::Channel *channel;
		int channelIndex;
		unsigned short channelIndexAsUnsignedShort;
		for (SoundContainer::SoundData soundData : soundContainer->GetSelectedSoundSet()) {
			result = (result == FMOD_OK) ? m_AudioSystem->playSound(soundData.SoundObject, channelGroupToPlayIn, true, &channel) : result;
			result = (result == FMOD_OK) ? channel->getIndex(&channelIndex) : result;
			channelIndexAsUnsignedShort = static_cast<unsigned short>(channelIndex);

			result = (result == FMOD_OK) ? channel->setUserData(soundContainer) : result;
			result = (result == FMOD_OK) ? channel->setCallback(SoundChannelEndedCallback) : result;
			result = (result == FMOD_OK) ? channel->set3DAttributes(&GetAsFMODVector(soundContainer->GetPosition() + soundData.Offset), NULL) : result;
			result = (result == FMOD_OK) ? channel->set3DLevel(g_SettingsMan.SoundPanningEffectStrength()) : result;
			result = (result == FMOD_OK) ? channel->setPriority(soundContainer->GetPriority()) : result;
			result = (result == FMOD_OK) ? channel->setVolume(soundContainer->GetVolume()) : result;
			result = (result == FMOD_OK) ? channel->setPitch(soundContainer->GetPitch()) : result;

			if (!soundContainer->IsImmobile()) {
				m_SoundChannelRolloffs.insert({static_cast<unsigned short>(channelIndex), {FMOD_VECTOR(soundData.CustomRolloffPoints[0]), FMOD_VECTOR(soundData.CustomRolloffPoints[1])}});
				result = (result == FMOD_OK) ? channel->set3DCustomRolloff(m_SoundChannelRolloffs.at(channelIndexAsUnsignedShort).data(), 2) : result;
				result = (result == FMOD_OK) ? UpdateMobileSoundChannelCalculated3DEffects(channel) : result;
			}

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not play sounds from SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
				return false;
			}

			result = channel->setPaused(false);
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Failed to start playing sounds from SoundContainer " + soundContainer->GetPresetName() + " after setting it up: " + std::string(FMOD_ErrorString(result)));
				return false;
			}

			soundContainer->AddPlayingChannel(channelIndexAsUnsignedShort);
		}

		// Now that the sound is playing we can register an event with the SoundContainer's channels, which can be used by clients to identify the sound being played.
		if (m_IsInMultiplayerMode) {
			RegisterSoundEvent(player, SOUND_PLAY, soundContainer->GetPlayingChannels(), &soundContainer->GetSelectedSoundHashes(), soundContainer->GetPosition(), soundContainer->GetLoopSetting(), soundContainer->GetVolume(), soundContainer->GetPitch(), soundContainer->IsAffectedByGlobalPitch(), soundContainer->GetAttenuationStartDistance(), soundContainer->IsImmobile());
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsPosition(SoundContainer *soundContainer) {
		if (!m_AudioEnabled || !soundContainer) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_POSITION, soundContainer->GetPlayingChannels(), &soundContainer->GetSelectedSoundHashes(), soundContainer->GetPosition()); }

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;
		FMOD::Sound *sound;

		const std::unordered_set<int> *playingChannels = soundContainer->GetPlayingChannels();
		for (int channelIndex : *playingChannels) {
			result = m_AudioSystem->getChannel(channelIndex, &soundChannel);
			result = (result == FMOD_OK) ? soundChannel->getCurrentSound(&sound) : result;
			const SoundContainer::SoundData *soundData = soundContainer->GetSoundDataForSound(sound);

			result = (result == FMOD_OK) ? soundChannel->set3DAttributes(&GetAsFMODVector(position + ((soundData == NULL) ? Vector() : soundData->Offset)), NULL) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set sound position for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
			result = (result == FMOD_OK) ? UpdateMobileSoundChannelCalculated3DEffects(soundChannel) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not update attenuation for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsVolume(SoundContainer *soundContainer) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_VOLUME, soundContainer->GetPlayingChannels(), &soundContainer->GetSelectedSoundHashes(), Vector(), 0, soundContainer->GetVolume()); }

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;

		const std::unordered_set<int> *playingChannels = soundContainer->GetPlayingChannels();
		for (int channelIndex : *playingChannels) {
			result = m_AudioSystem->getChannel(channelIndex, &soundChannel);
			result = result == FMOD_OK ? soundChannel->setVolume(soundContainer->GetVolume()) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not update sound volume for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsPitch(SoundContainer *soundContainer) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_PITCH, soundContainer->GetPlayingChannels(), &soundContainer->GetSelectedSoundHashes(), Vector(), 0, 0.0F, soundContainer->GetPitch()); }

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;

		const std::unordered_set<int> *playingChannels = soundContainer->GetPlayingChannels();
		for (int channelIndex : *playingChannels) {
			result = m_AudioSystem->getChannel(channelIndex, &soundChannel);
			result = result == FMOD_OK ? soundChannel->setPitch(soundContainer->GetPitch()) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not update sound pitch for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




			if (result != FMOD_OK) {
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	void AudioMan::UpdateCalculated3DEffectsForMobileSoundChannels() {
		int numberOfPlayingChannels;
		FMOD::Channel *channel;

		FMOD_RESULT result = m_MobileSoundChannelGroup->getNumChannels(&numberOfPlayingChannels);
		if (result == FMOD_OK) {
			for (int i = 0; i < numberOfPlayingChannels; i++) {
				result = m_MobileSoundChannelGroup->getChannel(i, &channel);
				result = result == FMOD_OK ? UpdateMobileSoundChannelCalculated3DEffects(channel) : result;
				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("ERROR: An error occurred when manually attenuating all playing channels, for channel index " + std::to_string(i) + ": " + std::string(FMOD_ErrorString(result)));
				}
			}
		} else {
			g_ConsoleMan.PrintString("ERROR: Failed to get the number of playing channels when manually attenuating all playing channels: " + std::string(FMOD_ErrorString(result)));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT AudioMan::UpdateMobileSoundChannelCalculated3DEffects(FMOD::Channel *channel) {
		FMOD_VECTOR channelPosition;
		FMOD_RESULT result = channel->get3DAttributes(&channelPosition, NULL);
		if (result != FMOD_OK) {
			return result;
	FMOD_RESULT F_CALLBACK AudioMan::MusicChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *unusedCommandData1, void *unusedCommandData2) {
		if (channelControlType == FMOD_CHANNELCONTROL_CHANNEL && callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
			g_AudioMan.PlayNextStream();
		}
		return FMOD_OK;
	}

		const Activity *currentActivity = g_ActivityMan.GetActivity();
		float shortestDistance = g_SceneMan.GetSceneDim().GetMagnitude();
		float longestDistance = 0;
		for (unsigned short player = Players::PlayerOne; player < currentActivity->GetPlayerCount(); player++) {
			if (currentActivity->PlayerHuman(player)) {
				float distance = g_SceneMan.ShortestDistance(GetAsVector(channelPosition), g_SceneMan.GetScrollTarget(currentActivity->ScreenOfPlayer(player)), g_SceneMan.SceneWrapsX()).GetMagnitude();
				shortestDistance = min(shortestDistance, distance);
				longestDistance = max(longestDistance, distance);
			}
		}

		float channel3dLevel;
		result = (result == FMOD_OK) ? channel->get3DLevel(&channel3dLevel) : result;
		if (result == FMOD_OK && m_CurrentActivityHumanCount == 1) {
			if (shortestDistance < g_SettingsMan.c_MinimumDistanceForPanning()) {
				channel->set3DLevel(0);
			} else if (shortestDistance < g_SettingsMan.c_MinimumDistanceForPanning() * 2) {
				channel->set3DLevel(LERP(0, 1, 0, g_SettingsMan.SoundPanningEffectStrength(), channel3dLevel));
			} else {
				channel->set3DLevel(g_SettingsMan.SoundPanningEffectStrength());
			}
		}
	FMOD_RESULT F_CALLBACK AudioMan::SoundChannelEndedCallback(FMOD_CHANNELCONTROL *channelControl, FMOD_CHANNELCONTROL_TYPE channelControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType, void *unusedCommandData1, void *unusedCommandData2) {
		if (channelControlType == FMOD_CHANNELCONTROL_CHANNEL && callbackType == FMOD_CHANNELCONTROL_CALLBACK_END) {
			FMOD::Channel *channel = reinterpret_cast<FMOD::Channel *>(channelControl);
			int channelIndex;
			FMOD_RESULT result = channel->getIndex(&channelIndex);

			// Remove this playing sound index from the SoundContainer if it has any playing sounds, i.e. it hasn't been reset before this callback happened.
			void *userData;
			result = (result == FMOD_OK) ? channel->getUserData(&userData) : result;
			if (result == FMOD_OK) {
				SoundContainer *channelSoundContainer = static_cast<SoundContainer *>(userData);
				if (channelSoundContainer->IsBeingPlayed()) { channelSoundContainer->RemovePlayingChannel(channelIndex); }
				result = (result == FMOD_OK) ? channel->setUserData(nullptr) : result;

		FMOD_VECTOR *rolloffPoints; // NOTE: Rolloff points are - 1. Minimum audible distance, 2. Attenuation start distance
		result = (result == FMOD_OK) ? channel->get3DCustomRolloff(&rolloffPoints, NULL) : result;

		if (result == FMOD_OK && channel3dLevel < 1 && rolloffPoints) {
			float attenuatedVolume;
			if (longestDistance < rolloffPoints[0].x || shortestDistance > c_SoundMaxAudibleDistance) {
				attenuatedVolume = 0;
			} else if (shortestDistance < rolloffPoints[0].x + rolloffPoints[1].x) {
				attenuatedVolume = 1;
				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("ERROR: An error occurred when Ending a sound in SoundContainer " + channelSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
					return result;
				}
			} else {
				attenuatedVolume = rolloffPoints[1].x / (shortestDistance - rolloffPoints[0].x);
				g_ConsoleMan.PrintString("ERROR: An error occurred when Ending a sound: " + std::string(FMOD_ErrorString(result)));
			}

			void *userData;
			result = (result == FMOD_OK) ? channel->getUserData(&userData) : result;
			result = (result == FMOD_OK) ? channel->setVolume(attenuatedVolume * static_cast<SoundContainer *>(userData)->GetVolume()) : result;
		}

		return result;
		return FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_VECTOR AudioMan::GetAsFMODVector(const Vector &vector, float zValue) const {
		Vector sceneDimensions = g_SceneMan.GetScene() ? g_SceneMan.GetSceneDim() : Vector();
		return sceneDimensions.IsZero() ? FMOD_VECTOR{0, 0, zValue} : FMOD_VECTOR{vector.m_X, sceneDimensions.m_Y - vector.m_Y, zValue};
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Vector AudioMan::GetAsVector(FMOD_VECTOR fmodVector) const {
		Vector sceneDimensions = g_SceneMan.GetScene() ? g_SceneMan.GetSceneDim() : Vector();
		return sceneDimensions.IsZero() ? Vector() : Vector(fmodVector.x, sceneDimensions.m_Y - fmodVector.y);
	}
}
