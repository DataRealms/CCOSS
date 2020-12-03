#include "SoundContainer.h"

namespace RTE {

	ConcreteClassInfo(SoundContainer, Entity, 50);

	const std::unordered_map<std::string, SoundContainer::SoundCycleMode> SoundContainer::c_SoundCycleModeMap = {
		{"Random", SoundContainer::SoundCycleMode::MODE_RANDOM},
		{"Forwards", SoundContainer::SoundCycleMode::MODE_FORWARDS}
	};
	const std::unordered_map<std::string, SoundContainer::SoundOverlapMode> SoundContainer::c_SoundOverlapModeMap = {
		{"Overlap", SoundContainer::SoundOverlapMode::MODE_OVERLAP},
		{"Restart", SoundContainer::SoundOverlapMode::MODE_RESTART},
		{"Ignore Play", SoundContainer::SoundOverlapMode::MODE_IGNORE_PLAY}
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundContainer::Clear() {
		m_SoundSets.clear();
		m_SelectedSoundSet = 0;
		m_SoundSelectionCycleMode = MODE_RANDOM;

		m_PlayingChannels.clear();
		m_SoundOverlapMode = MODE_OVERLAP;

		m_Immobile = false;
		m_AttenuationStartDistance = c_DefaultAttenuationStartDistance;
		m_Loops = 0;
		m_SoundPropertiesUpToDate = false;

		m_Priority = AudioMan::PRIORITY_NORMAL;
		m_AffectedByGlobalPitch = true;

		m_Pos = Vector();
		m_Volume = 1.0F;
		m_Pitch = 1.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::Create(const SoundContainer &reference) {
		Entity::Create(reference);

		for (const std::vector<SoundData> &referenceSoundSet : reference.m_SoundSets) {
			std::vector<SoundData> soundSet;
			for (const SoundData &referenceData : referenceSoundSet) {
				soundSet.push_back(SoundData {referenceData.SoundFile, referenceData.SoundObject, referenceData.Offset, referenceData.MinimumAudibleDistance, referenceData.AttenuationStartDistance});
			}
			m_SoundSets.push_back(soundSet);
		}
		m_SelectedSoundSet = reference.m_SelectedSoundSet;
		m_SoundSelectionCycleMode = reference.m_SoundSelectionCycleMode;

		m_PlayingChannels.clear();
		m_SoundOverlapMode = reference.m_SoundOverlapMode;

		m_Immobile = reference.m_Immobile;
		m_AttenuationStartDistance = reference.m_AttenuationStartDistance;
		m_Loops = reference.m_Loops;

		m_Priority = reference.m_Priority;
		m_AffectedByGlobalPitch = reference.m_AffectedByGlobalPitch;

		m_Pos = reference.m_Pos;
		m_Volume = reference.m_Volume;
		m_Pitch = reference.m_Pitch;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "AddSound" || propName == "AddSoundSet") {
			return ReadSoundOrSoundSet(propName, reader);
		} else if (propName == "CycleMode") {
			std::string cycleModeString = reader.ReadPropValue();
			if (c_SoundCycleModeMap.find(cycleModeString) != c_SoundCycleModeMap.end()) {
				m_SoundSelectionCycleMode = c_SoundCycleModeMap.find(cycleModeString)->second;
			} else {
				reader.ReportError("Cycle mode " + cycleModeString + " is invalid.");
			}
		} else if (propName == "OverlapMode") {
			std::string overlapModeString = reader.ReadPropValue();
			if (c_SoundOverlapModeMap.find(overlapModeString) != c_SoundOverlapModeMap.end()) {
				m_SoundOverlapMode = c_SoundOverlapModeMap.find(overlapModeString)->second;
			} else {
				reader.ReportError("Cycle mode " + overlapModeString + " is invalid.");
			}
		} else if (propName == "Immobile") {
			reader >> m_Immobile;
		} else if (propName == "AttenuationStartDistance") {
			reader >> m_AttenuationStartDistance;
		} else if (propName == "LoopSetting") {
			reader >> m_Loops;
		} else if (propName == "Priority") {
			reader >> m_Priority;
			if (m_Priority < 0 || m_Priority > 256) { reader.ReportError("SoundContainer priority must be between 256 (lowest priority) and 0 (highest priority)."); }
		} else if (propName == "AffectedByGlobalPitch") {
			reader >> m_AffectedByGlobalPitch;
		} else if (propName == "Position") {
			reader >> m_Pos;
		} else if (propName == "Volume") {
			reader >> m_Volume;
		} else if (propName == "Pitch") {
			reader >> m_Pitch;
		} else {
			return Entity::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	int SoundContainer::ReadSoundOrSoundSet(const std::string &propName, Reader &reader) {
		vector<SoundData> soundSet;
		if (propName == "AddSound") {
			soundSet.push_back(ReadAndGetSound(reader));
		} else if (propName == "AddSoundSet") {
			reader.ReadPropValue();
			while (reader.NextProperty()) {
				std::string soundSetSubPropertyName = reader.ReadPropName();
				if (soundSetSubPropertyName == "AddSound") {
					soundSet.push_back(ReadAndGetSound(reader));
				} else {
					reader.ReportError(soundSetSubPropertyName + " is not a valid property of SoundSets.");
				}
			}
		}

		m_SoundSets.push_back(soundSet);
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SoundContainer::SoundData SoundContainer::ReadAndGetSound(Reader &reader) const {
		std::string propValue = reader.ReadPropValue();
		SoundData soundData;

		/// <summary>
		/// Internal lambda function to load an audio file by path in as a ContentFile, which in turn loads it into FMOD, then returns SoundData for it in the outParam outSoundData.
		/// </summary>
		/// <param name="soundPath">The path to the sound file.</param>
		auto readSound = [&soundData, &reader](const std::string &soundPath) {
			ContentFile soundFile(soundPath.c_str());
			soundFile.SetFormattedReaderPosition("in file " + reader.GetCurrentFilePath() + " on line " + std::to_string(reader.GetCurrentFileLine()));
			FMOD::Sound *soundObject = soundFile.GetAsSound();
			if (g_AudioMan.IsAudioEnabled() && !soundObject) { reader.ReportError(std::string("Failed to load the sound from the file")); }

			soundData.SoundFile = soundFile;
			soundData.SoundObject = soundObject;
		};

		if (propValue != "Sound" && propValue != "ContentFile") {
			readSound(propValue);
			return soundData;
		}

		while (reader.NextProperty()) {
			std::string soundSubPropertyName = reader.ReadPropName();
			if (soundSubPropertyName == "FilePath" || soundSubPropertyName == "Path") {
				readSound(reader.ReadPropValue());
			} else if (soundSubPropertyName == "Offset") {
				reader >> soundData.Offset;
			} else if (soundSubPropertyName == "MinimumAudibleDistance") {
				reader >> soundData.MinimumAudibleDistance;
			} else if (soundSubPropertyName == "AttenuationStartDistance") {
				reader >> soundData.AttenuationStartDistance;
			}
		}

		return soundData;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::Save(Writer &writer) const {
		Entity::Save(writer);

		for (const std::vector<SoundData> &soundSet : m_SoundSets) {
			writer.NewProperty("AddSoundSet");
			writer.ObjectStart("SoundSet");

			for (const SoundData &soundData : soundSet) {
				writer.NewProperty("AddSound");
				writer.ObjectStart("ContentFile");

				writer.NewProperty("FilePath");
				writer << soundData.SoundFile.GetDataPath();
				writer.NewProperty("Offset");
				writer << soundData.Offset;
				writer.NewProperty("MinimumAudibleDistance");
				writer << soundData.MinimumAudibleDistance;
				writer.NewProperty("AttenuationStartDistance");
				writer << soundData.AttenuationStartDistance;

				writer.ObjectEnd();
			}
			writer.ObjectEnd();
		}

		writer.NewProperty("CycleMode");
		std::list<std::pair<const std::string, SoundCycleMode>>::const_iterator cycleModeMapEntry = std::find_if(c_SoundCycleModeMap.begin(), c_SoundCycleModeMap.end(), [&soundSelectionCycleMode = m_SoundSelectionCycleMode](auto element) { return element.second == soundSelectionCycleMode; });
		if (cycleModeMapEntry != c_SoundCycleModeMap.end()) {
			writer << cycleModeMapEntry->first;
		} else {
			writer << m_SoundSelectionCycleMode;
		}

		writer.NewProperty("OverlapMode");
		std::list<std::pair<const std::string, SoundOverlapMode>>::const_iterator overlapModeMapEntry = std::find_if(c_SoundOverlapModeMap.begin(), c_SoundOverlapModeMap.end(), [&soundOverlapMode = m_SoundOverlapMode](auto element) { return element.second == soundOverlapMode; });
		if (overlapModeMapEntry != c_SoundOverlapModeMap.end()) {
			writer << overlapModeMapEntry->first;
		} else {
			writer << m_SoundOverlapMode;
		}

		writer.NewProperty("Immobile");
		writer << m_Immobile;
		writer.NewProperty("AttenuationStartDistance");
		writer << m_AttenuationStartDistance;
		writer.NewProperty("LoopSetting");
		writer << m_Loops;

		writer.NewProperty("Priority");
		writer << m_Priority;
		writer.NewProperty("AffectedByGlobalPitch");
		writer << m_AffectedByGlobalPitch;

		writer.NewProperty("Pos");
		writer << m_Pos;
		writer.NewProperty("Volume");
		writer << m_Volume;
		writer.NewProperty("Pitch");
		writer << m_Pitch;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundContainer::AddSound(const std::string &soundFilePath, unsigned int soundSetIndex, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound) {
		std::vector<SoundData> soundSet;
		if (soundSetIndex < m_SoundSets.size()) { soundSet = m_SoundSets[soundSetIndex]; }

		ContentFile soundFile(soundFilePath.c_str());
		FMOD::Sound *soundObject = soundFile.GetAsSound(abortGameForInvalidSound, false);
		if (!soundObject) {
			return;
		}

		soundSet.push_back({soundFile, soundObject, offset, minimumAudibleDistance, attenuationStartDistance});
		if (soundSetIndex >= m_SoundSets.size()) { m_SoundSets.push_back(soundSet); }

		m_SoundPropertiesUpToDate = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	std::vector<size_t> SoundContainer::GetSelectedSoundHashes() const {
		std::vector<size_t> soundHashes;
		for (const SoundData &selectedSoundData : m_SoundSets[m_SelectedSoundSet]) {
			soundHashes.push_back(selectedSoundData.SoundFile.GetHash());
		}
		return soundHashes;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	const SoundContainer::SoundData *SoundContainer::GetSoundDataForSound(const FMOD::Sound *sound) const {
		for (const std::vector<SoundData> &soundSet : m_SoundSets) {
			for (const SoundData &soundData : soundSet) {
				if (sound == soundData.SoundObject) {
					return &soundData;
				}
			}
		}
		return NULL;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundContainer::Play(int player) {
		if (HasAnySounds()) {
			if (IsBeingPlayed()) {
				if (m_SoundOverlapMode == MODE_RESTART) {
					Restart(player);
				} else if (m_SoundOverlapMode == MODE_IGNORE_PLAY) {
					return false;
				}
			}
			return g_AudioMan.PlaySoundContainer(this, player);

		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundContainer::SelectNextSoundSet() {
		int soundSetCount = m_SoundSets.size();
		switch (soundSetCount) {
			case 0:
				return false;
			case 1:
				return true;
			case 2:
				m_SelectedSoundSet = (m_SelectedSoundSet + 1) % 2;
				break;
			default:
				/// <summary>
				/// Internal lambda function to pick a random sound that's not the previously played sound. Done to avoid scoping issues inside the switch below.
				/// </summary>
				auto selectRandomSound = [&soundSetCount, this]() {
					size_t soundToSelect = RandomNum(0, soundSetCount - 1);
					while (soundToSelect == m_SelectedSoundSet) {
						soundToSelect = RandomNum(0, soundSetCount - 1);
					}
					m_SelectedSoundSet = soundToSelect;
				};

				switch (m_SoundSelectionCycleMode) {
					case MODE_RANDOM:
						selectRandomSound();
						break;
					case MODE_FORWARDS:
						m_SelectedSoundSet = (m_SelectedSoundSet + 1) % soundSetCount;
						break;
					default:
						RTEAbort("Invalid sound cycle mode " + m_SoundSelectionCycleMode);
						break;
				}
				RTEAssert(m_SelectedSoundSet >= 0 && m_SelectedSoundSet < soundSetCount, "Failed to select next sound, either none was selected or the selected sound was invalid.");
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT SoundContainer::UpdateSoundProperties() {
		FMOD_RESULT result = FMOD_OK;

		for (std::vector<SoundData> &soundSet : m_SoundSets) {
			for (SoundData &soundData : soundSet) {
				FMOD_MODE soundMode = (m_Loops == 0) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL;
				if (m_Immobile) {
					soundMode |= FMOD_3D_HEADRELATIVE;
					m_AttenuationStartDistance = c_SoundMaxAudibleDistance;
				} else {
					soundMode |= FMOD_3D_INVERSEROLLOFF;
				}

				result = (result == FMOD_OK) ? soundData.SoundObject->setMode(soundMode) : result;
				result = (result == FMOD_OK) ? soundData.SoundObject->setLoopCount(m_Loops) : result;
				result = (result == FMOD_OK) ? soundData.SoundObject->set3DMinMaxDistance(soundData.MinimumAudibleDistance + std::max(0.0F, m_AttenuationStartDistance), c_SoundMaxAudibleDistance) : result;
			}
		}
		m_SoundPropertiesUpToDate = result == FMOD_OK;

		return result;
	}
}
