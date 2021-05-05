#include "AudioMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "SoundContainer.h"
#include "GUISound.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::Clear() {
		m_AudioEnabled = false;
		m_CurrentActivityHumanPlayerPositions.clear();
		m_SoundChannelMinimumAudibleDistances.clear();

		m_MusicVolume = 1.0F;
		m_SoundsVolume = 1.0F;
		m_GlobalPitch = 1.0F;

		m_SoundPanningEffectStrength = 0.5F;

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		m_ListenerZOffset = 400;
		m_MinimumDistanceForPanning = 30.0F;
		//////////////////////////////////////////////////

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

	int AudioMan::Initialize() {
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
				int currentActivityHumanCount = m_IsInMultiplayerMode ? 1 : currentActivity->GetHumanCount();

				if (m_CurrentActivityHumanPlayerPositions.size() != currentActivityHumanCount) { status = status == FMOD_OK ? m_AudioSystem->set3DNumListeners(currentActivityHumanCount) : status; }

				if (m_CurrentActivityHumanPlayerPositions.empty() || (m_CurrentActivityHumanPlayerPositions.at(0) == nullptr || m_CurrentActivityHumanPlayerPositions.at(0)->GetFloorIntX() > g_SceneMan.GetSceneWidth() || m_CurrentActivityHumanPlayerPositions.at(0)->GetX() < 0.0F)) {
					m_CurrentActivityHumanPlayerPositions.clear();
					for (short player = Players::PlayerOne; player < Players::MaxPlayerCount && m_CurrentActivityHumanPlayerPositions.size() < currentActivityHumanCount; player++) {
						if (currentActivity->PlayerActive(player) && currentActivity->PlayerHuman(player)) {
							const Vector &humanPlayerPosition = g_SceneMan.GetScrollTarget(currentActivity->ScreenOfPlayer(player));
							m_CurrentActivityHumanPlayerPositions.push_back(&humanPlayerPosition);
						}
					}
				}

				int listenerNumber = 0;
				for (const Vector *humanPlayerPosition : m_CurrentActivityHumanPlayerPositions) {
					if (status == FMOD_OK) {
						FMOD_VECTOR playerPosition = GetAsFMODVector(*humanPlayerPosition, m_ListenerZOffset);
						status = m_AudioSystem->set3DListenerAttributes(listenerNumber, &playerPosition, nullptr, &c_FMODForward, &c_FMODUp);
					}
					listenerNumber++;
				}

				Update3DEffectsForMobileSoundChannels();
			} else {
				if (!m_CurrentActivityHumanPlayerPositions.empty()) {
					m_CurrentActivityHumanPlayerPositions.clear();
					status = status == FMOD_OK ? m_AudioSystem->set3DNumListeners(1) : status;
				}
				if (status == FMOD_OK) {
					FMOD_VECTOR scrollTarget = GetAsFMODVector(g_SceneMan.GetScrollTarget(), m_ListenerZOffset);
					status = m_AudioSystem->set3DListenerAttributes(0, &scrollTarget, nullptr, &c_FMODForward, &c_FMODUp);
				}
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
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_GLOBAL_PITCH, nullptr); }

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
			if (result == FMOD_OK) {
				FMOD_VECTOR zero_vector = GetAsFMODVector(Vector());
				result = musicChannel->set3DAttributes(&zero_vector, nullptr);
			}
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
		newSoundContainer->GetTopLevelSoundSet().AddSound(filePath);
		if (newSoundContainer->HasAnySounds()) {
			PlaySoundContainer(newSoundContainer, player);
		}
		return newSoundContainer;
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

	void AudioMan::RegisterMusicEvent(int player, NetworkMusicState state, const char *filepath, int loopsOrSilence, float position, float pitch) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) { RegisterMusicEvent(i, state, filepath, loopsOrSilence, position, pitch); }
		} else {
			NetworkMusicData musicData;
			musicData.State = state;
			musicData.LoopsOrSilence = loopsOrSilence;
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
		const NetworkSoundData *lastSetGlobalPitchEvent = nullptr;
		for (const NetworkSoundData &soundEvent : m_SoundEvents[player]) {
			if (soundEvent.State == SOUND_SET_GLOBAL_PITCH) {
				lastSetGlobalPitchEvent = &soundEvent;
			} else {
				list.push_back(soundEvent);
			}
		}
		if (lastSetGlobalPitchEvent) { list.push_back(*lastSetGlobalPitchEvent); }
		g_SoundEventsListMutex[player].unlock();

		m_SoundEvents[player].clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::RegisterSoundEvent(int player, NetworkSoundState state, const SoundContainer *soundContainer, int fadeoutTime) {
		if (player == -1) {
			for (int i = 0; i < c_MaxClients; i++) { RegisterSoundEvent(i, state, soundContainer, fadeoutTime); }
		} else {
			FMOD_RESULT result = FMOD_OK;
			std::vector<NetworkSoundData> soundDataVector;

			if (state == SOUND_SET_GLOBAL_PITCH) {
				NetworkSoundData soundData;
				soundData.State = state;
				soundData.Pitch = m_GlobalPitch;
				soundDataVector.push_back(soundData);
			} else {
				for (int playingChannel : *soundContainer->GetPlayingChannels()) {
					FMOD::Channel *soundChannel;
					result = m_AudioSystem->getChannel(playingChannel, &soundChannel);
					FMOD::Sound *sound;
					result = (result == FMOD_OK) ? soundChannel->getCurrentSound(&sound) : result;

					if (result != FMOD_OK) {
						continue;
					}
					NetworkSoundData soundData;
					soundData.State = state;
					soundData.SoundFileHash = soundContainer->GetSoundDataForSound(sound)->SoundFile.GetHash();
					soundData.Channel = playingChannel;
					soundData.Immobile = soundContainer->IsImmobile();
					soundData.AttenuationStartDistance = soundContainer->GetAttenuationStartDistance();
					soundData.Loops = soundContainer->GetLoopSetting();
					soundData.Priority = soundContainer->GetPriority();
					soundData.AffectedByGlobalPitch = soundContainer->IsAffectedByGlobalPitch();
					soundData.Position[0] = soundContainer->GetPosition().m_X;
					soundData.Position[1] = soundContainer->GetPosition().m_Y;
					soundData.Volume = soundContainer->GetVolume();
					soundData.Pitch = soundContainer->GetPitch();
					soundData.FadeOutTime = fadeoutTime;
					soundDataVector.push_back(soundData);
				}
			}

			g_SoundEventsListMutex[player].lock();
			m_SoundEvents[player].insert(m_SoundEvents[player].end(), soundDataVector.begin(), soundDataVector.end());
			g_SoundEventsListMutex[player].unlock();
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
		if (!soundContainer->GetTopLevelSoundSet().SelectNextSounds()) {
			g_ConsoleMan.PrintString("Unable to select new sounds to play for SoundContainer " + soundContainer->GetPresetName());
			return false;
		}
		FMOD::ChannelGroup *channelGroupToPlayIn = soundContainer->IsImmobile() ? m_ImmobileSoundChannelGroup : m_MobileSoundChannelGroup;
		FMOD::Channel *channel;
		int channelIndex;
		std::vector<const SoundSet::SoundData *> selectedSoundData;
		soundContainer->GetTopLevelSoundSet().GetFlattenedSoundData(selectedSoundData, true);
		for (const SoundSet::SoundData *soundData : selectedSoundData) {
			result = (result == FMOD_OK) ? m_AudioSystem->playSound(soundData->SoundObject, channelGroupToPlayIn, true, &channel) : result;
			result = (result == FMOD_OK) ? channel->getIndex(&channelIndex) : result;

			result = (result == FMOD_OK) ? channel->setUserData(soundContainer) : result;
			result = (result == FMOD_OK) ? channel->setCallback(SoundChannelEndedCallback) : result;
			result = (result == FMOD_OK) ? channel->setPriority(soundContainer->GetPriority()) : result;
			result = (result == FMOD_OK) ? channel->setPitch(soundContainer->GetPitch()) : result;
			if (soundContainer->IsImmobile()) {
				result = (result == FMOD_OK) ? channel->set3DLevel(0.0F) : result;
				result = (result == FMOD_OK) ? channel->setVolume(soundContainer->GetVolume()) : result;
			} else {
				m_SoundChannelMinimumAudibleDistances.insert({ channelIndex, soundData->MinimumAudibleDistance });
				result = (result == FMOD_OK) ? channel->set3DLevel(m_SoundPanningEffectStrength) : result;

				FMOD_VECTOR soundContainerPosition = GetAsFMODVector(soundContainer->GetPosition() + soundData->Offset);
				UpdatePositionalEffectsForSoundChannel(channel, &soundContainerPosition);
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

			soundContainer->AddPlayingChannel(channelIndex);
		}

		if (m_IsInMultiplayerMode) { RegisterSoundEvent(player, SOUND_PLAY, soundContainer); }
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsPosition(const SoundContainer *soundContainer) {
		if (!m_AudioEnabled || !soundContainer) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_POSITION, soundContainer); }

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;
		FMOD::Sound *sound;

		const std::unordered_set<int> *playingChannels = soundContainer->GetPlayingChannels();
		for (int channelIndex : *playingChannels) {
			result = m_AudioSystem->getChannel(channelIndex, &soundChannel);
			result = (result == FMOD_OK) ? soundChannel->getCurrentSound(&sound) : result;
			const SoundSet::SoundData *soundData = soundContainer->GetSoundDataForSound(sound);

			FMOD_VECTOR soundPosition = GetAsFMODVector(soundContainer->GetPosition() + ((soundData == nullptr) ? Vector() : soundData->Offset));
			result = (result == FMOD_OK) ? UpdatePositionalEffectsForSoundChannel(soundChannel, &soundPosition) : result;
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not set sound position for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsVolume(const SoundContainer *soundContainer, float newVolume) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_VOLUME, soundContainer); }

		FMOD_RESULT result = FMOD_OK;
		FMOD::Channel *soundChannel;
		float soundContainerOldVolume = soundContainer->GetVolume() == 0 ? 1.0F : soundContainer->GetVolume();
		float soundChannelCurrentVolume;

		const std::unordered_set<int> *playingChannels = soundContainer->GetPlayingChannels();
		for (int channelIndex : *playingChannels) {
			result = m_AudioSystem->getChannel(channelIndex, &soundChannel);
			result = result == FMOD_OK ? soundChannel->getVolume(&soundChannelCurrentVolume) : result;

			if (newVolume == 0.0F) {
				result = result == FMOD_OK ? soundChannel->setMute(true) : result;
				result = result == FMOD_OK ? soundChannel->setVolume(soundChannelCurrentVolume / soundContainerOldVolume) : result;
			} else {
				result = result == FMOD_OK ? soundChannel->setMute(false) : result;
				result = result == FMOD_OK ? soundChannel->setVolume(newVolume / soundContainerOldVolume * soundChannelCurrentVolume) : result;
			}
			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: Could not update sound volume for the sound being played on channel " + std::to_string(channelIndex) + " for SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
			}
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool AudioMan::ChangeSoundContainerPlayingChannelsPitch(const SoundContainer *soundContainer) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_SET_PITCH, soundContainer); }

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

	bool AudioMan::StopSoundContainerPlayingChannels(SoundContainer *soundContainer, int player) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return false;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(player, SOUND_STOP, soundContainer); }

		FMOD_RESULT result;
		FMOD::Channel *soundChannel;
		
		const std::unordered_set<int> *channels = soundContainer->GetPlayingChannels();
		for (std::unordered_set<int>::const_iterator channelIterator = channels->begin(); channelIterator != channels->end();) {
			result = m_AudioSystem->getChannel((*channelIterator), &soundChannel);
			++channelIterator; // NOTE - stopping the sound will remove the channel, screwing things up if we don't move to the next iterator preemptively
			result = (result == FMOD_OK) ? soundChannel->stop() : result;
			if (result != FMOD_OK) { g_ConsoleMan.PrintString("Error: Failed to stop playing channel in SoundContainer " + soundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result))); }
		}
		return result == FMOD_OK;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AudioMan::FadeOutSoundContainerPlayingChannels(SoundContainer *soundContainer, int fadeOutTime) {
		if (!m_AudioEnabled || !soundContainer || !soundContainer->IsBeingPlayed()) {
			return;
		}
		if (m_IsInMultiplayerMode) { RegisterSoundEvent(-1, SOUND_FADE_OUT, soundContainer, fadeOutTime); }

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

	void AudioMan::Update3DEffectsForMobileSoundChannels() {
		int numberOfPlayingChannels;
		FMOD::Channel *soundChannel;

		FMOD_RESULT result = m_MobileSoundChannelGroup->getNumChannels(&numberOfPlayingChannels);
		if (result != FMOD_OK) {
			g_ConsoleMan.PrintString("ERROR: Failed to get the number of playing channels when updating calculated sound effects for all playing channels: " + std::string(FMOD_ErrorString(result)));
			return;
		}

		for (int i = 0; i < numberOfPlayingChannels; i++) {
			result = m_MobileSoundChannelGroup->getChannel(i, &soundChannel);
			FMOD_VECTOR channelPosition;
			result = result == FMOD_OK ? soundChannel->get3DAttributes(&channelPosition, nullptr) : result;
			result = result == FMOD_OK ? UpdatePositionalEffectsForSoundChannel(soundChannel, &channelPosition) : result;

			float channel3dLevel;
			result = (result == FMOD_OK) ? soundChannel->get3DLevel(&channel3dLevel) : result;
			if (result == FMOD_OK && m_CurrentActivityHumanPlayerPositions.size() == 1) {
				float distanceToPlayer = (*m_CurrentActivityHumanPlayerPositions.at(0) - GetAsVector(channelPosition)).GetMagnitude();
				if (distanceToPlayer < m_MinimumDistanceForPanning) {
					soundChannel->set3DLevel(0);
				} else if (distanceToPlayer < m_MinimumDistanceForPanning * 2) {
					soundChannel->set3DLevel(LERP(0, 1, 0, m_SoundPanningEffectStrength, channel3dLevel));
				} else {
					soundChannel->set3DLevel(m_SoundPanningEffectStrength);
				}
			}

			if (result != FMOD_OK) {
				g_ConsoleMan.PrintString("ERROR: An error occurred updating calculated sound effects for playing channel with index " + std::to_string(i) + ": " + std::string(FMOD_ErrorString(result)));
				continue;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT AudioMan::UpdatePositionalEffectsForSoundChannel(FMOD::Channel *soundChannel, const FMOD_VECTOR *positionOverride) const {
		FMOD_RESULT result = FMOD_OK;
		bool sceneWraps = g_SceneMan.SceneWrapsX();

		FMOD_VECTOR channelPosition;
		if (positionOverride) {
			channelPosition = *positionOverride;
		} else if (sceneWraps) {
			//NOTE If the scene doesn't wrap and the position hasn't changed, this method doesn't set the channel position below, so there's no need to get it here.
			result = soundChannel->get3DAttributes(&channelPosition, nullptr);
			if (result != FMOD_OK) {
				return result;
			}
		}

		float halfSceneWidth = static_cast<float>(g_SceneMan.GetSceneWidth()) / 2.0F;
		std::array<FMOD_VECTOR, 2> wrappedChannelPositions;
		if (!sceneWraps) {
			wrappedChannelPositions = {channelPosition};
		} else {
			wrappedChannelPositions = (channelPosition.x <= halfSceneWidth) ?
				wrappedChannelPositions = {channelPosition, {channelPosition.x + g_SceneMan.GetSceneWidth(), channelPosition.y}} :
				wrappedChannelPositions = {FMOD_VECTOR({channelPosition.x - g_SceneMan.GetSceneWidth(), channelPosition.y}), channelPosition};
		}

		float shortestDistance = c_SoundMaxAudibleDistance;
		float longestDistance = 0;
		for (const Vector *humanPlayerPosition : m_CurrentActivityHumanPlayerPositions) {
			for (const FMOD_VECTOR &wrappedChannelPosition : wrappedChannelPositions) {
				float distanceToChannelPosition = (*humanPlayerPosition - GetAsVector(wrappedChannelPosition)).GetMagnitude();
				if (distanceToChannelPosition < shortestDistance) {
					shortestDistance = distanceToChannelPosition;
					channelPosition = wrappedChannelPosition;
				}
				if (distanceToChannelPosition > longestDistance) { longestDistance = distanceToChannelPosition; }
				if (!sceneWraps) {
					break;
				}
			}
		}

		int soundChannelIndex;
		result = result == FMOD_OK ? soundChannel->getIndex(&soundChannelIndex) : result;

		float attenuationStartDistance = c_DefaultAttenuationStartDistance;
		float soundMaxDistance;
		result = result == FMOD_OK ? soundChannel->get3DMinMaxDistance(&attenuationStartDistance, &soundMaxDistance) : result;
		
		float attenuatedVolume = (shortestDistance <= attenuationStartDistance) ? 1.0F : attenuationStartDistance / shortestDistance;
		if (shortestDistance >= soundMaxDistance) {
			attenuatedVolume = 0.0F;
		} else if (m_SoundChannelMinimumAudibleDistances.empty() || m_SoundChannelMinimumAudibleDistances.find(soundChannelIndex) == m_SoundChannelMinimumAudibleDistances.end()) {
			g_ConsoleMan.PrintString("ERROR: An error occurred when checking to see if the sound at channel " + std::to_string(soundChannelIndex) + " was less than its minimum audible distance away from the farthest listener.");
		} else if (longestDistance < m_SoundChannelMinimumAudibleDistances.at(soundChannelIndex)) {
			attenuatedVolume = 0.0F;
		}

		void *userData;
		result = result == FMOD_OK ? soundChannel->getUserData(&userData) : result;
		float panLevel;
		result = result == FMOD_OK ? soundChannel->get3DLevel(&panLevel) : result;
		if (result == FMOD_OK && (panLevel < 1.0F || attenuatedVolume == 0.0F)) {
			const SoundContainer *channelSoundContainer = static_cast<SoundContainer *>(userData);
			result = soundChannel->setVolume(attenuatedVolume * channelSoundContainer->GetVolume());
		}

		result = (result == FMOD_OK && (sceneWraps || positionOverride)) ? soundChannel->set3DAttributes(&channelPosition, nullptr) : result;

		return result;
	}

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

				if (g_AudioMan.m_SoundChannelMinimumAudibleDistances.find(channelIndex) != g_AudioMan.m_SoundChannelMinimumAudibleDistances.end()) { g_AudioMan.m_SoundChannelMinimumAudibleDistances.erase(channelIndex); }

				if (result != FMOD_OK) {
					g_ConsoleMan.PrintString("ERROR: An error occurred when Ending a sound in SoundContainer " + channelSoundContainer->GetPresetName() + ": " + std::string(FMOD_ErrorString(result)));
					return result;
				}
			} else {
				g_ConsoleMan.PrintString("ERROR: An error occurred when Ending a sound: " + std::string(FMOD_ErrorString(result)));
			}
		}
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
