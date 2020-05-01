#include "SoundContainer.h"
#include "RTETools.h"

namespace RTE {

	CONCRETECLASSINFO(SoundContainer, Entity, 0);

	const std::unordered_map<std::string, SoundContainer::SoundCycleMode> SoundContainer::c_CycleModeMap = {
		{"Random", SoundContainer::SoundCycleMode::MODE_RANDOM},
		{"Forwards", SoundContainer::SoundCycleMode::MODE_FORWARDS}
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundContainer::Clear() {
		m_SoundSets.clear();
		m_SelectedSoundSet = 0;
		m_SoundSelectionCycleMode = MODE_RANDOM;

		m_PlayingChannels.clear();

		m_AttenuationStartDistance = c_DefaultAttenuationStartDistance;
		m_Loops = 0;
		m_Priority = AudioMan::PRIORITY_NORMAL;
		m_AffectedByGlobalPitch = true;
		m_Immobile = false;

		m_AllSoundPropertiesUpToDate = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::Create(const SoundContainer &reference) {
		Entity::Create(reference);

		for (std::vector<SoundData> referenceSoundSet : reference.m_SoundSets) {
			std::vector<SoundData> soundSet;
			for (SoundData referenceData : referenceSoundSet) {
				soundSet.push_back(SoundData {referenceData.SoundFile, referenceData.SoundObject, referenceData.Offset, referenceData.MinimumAudibleDistance, referenceData.AttenuationStartDistance});
			}
			m_SoundSets.push_back(soundSet);
		}
		m_SelectedSoundSet = reference.m_SelectedSoundSet;
		m_SoundSelectionCycleMode = reference.m_SoundSelectionCycleMode;

		m_PlayingChannels.clear();

		m_AttenuationStartDistance = reference.m_AttenuationStartDistance;
		m_Loops = reference.m_Loops;
		m_Priority = reference.m_Priority;
		m_AffectedByGlobalPitch = reference.m_AffectedByGlobalPitch;
		m_Immobile = reference.m_Immobile;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "AddSound" || propName == "AddSoundSet") {
			return ReadSoundSet(propName, reader);
		} else if (propName == "CycleMode") {
			std::string cycleModeString = reader.ReadPropValue();
			if (c_CycleModeMap.find(cycleModeString) != c_CycleModeMap.end()) {
				m_SoundSelectionCycleMode = c_CycleModeMap.find(cycleModeString)->second;
			} else {
				reader.ReportError("Cycle mode " + cycleModeString + " is invalid.");
			}
		} else if (propName == "AttenuationStartDistance") {
			reader >> m_AttenuationStartDistance;
		} else if (propName == "LoopSetting") {
			reader >> m_Loops;
		} else if (propName == "Priority") {
			reader >> m_Priority;
			if (m_Priority < 0 || m_Priority > 256) { reader.ReportError("SoundContainer priority must be between 256 (lowest priority) and 0 (highest priority)."); }
		} else if (propName == "AffectedByGlobalPitch") {
			reader >> m_AffectedByGlobalPitch;
		} else if (propName == "Immobile") {
			reader >> m_Immobile;
		} else {
			// See if the base class(es) can find a match instead
			return Entity::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	int SoundContainer::ReadSoundSet(std::string propName, Reader &reader) {
		vector<SoundData> soundSet;
		if (propName == "AddSound") {
			soundSet.push_back(ReadSound(reader));
		} else if (propName == "AddSoundSet") {
			reader.ReadPropValue();
			while (reader.NextProperty()) {
				std::string soundSetSubPropertyName = reader.ReadPropName();
				if (soundSetSubPropertyName == "AddSound") {
					soundSet.push_back(ReadSound(reader));
				} else {
					reader.ReportError(soundSetSubPropertyName + " is not a valid property of SoundSets.");
				}
			}
		}

		m_SoundSets.push_back(soundSet);
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SoundContainer::SoundData SoundContainer::ReadSound(Reader &reader) {
		std::string propValue = reader.ReadPropValue();
		SoundData soundData;

		/// <summary>
		/// This lambda exists to have some easy, private reused code.
		/// It loads an audio file by path in as a ContentFile, which in turn loads it into FMOD, then returns SoundData for it in the outParam outSoundData.
		/// </summary>
		/// <param name="soundPath">The path to the sound file.</param>
		/// <param name="parentReader">The reader being used when calling this. Used to report errors if loading the sound fails.</param>
		/// <param name="outSoundData">The outParam for the SoundData struct produced by reading the given sound.</param>
		auto readSound = [](const std::string &soundPath, Reader &parentReader, SoundData *outSoundData) {
			ContentFile soundFile(soundPath.c_str());
			FMOD::Sound *soundObject = soundFile.GetAsSample();
			if (g_AudioMan.IsAudioEnabled() && !soundObject) { parentReader.ReportError(std::string("Failed to load the sound from the file")); }

			outSoundData->SoundFile = soundFile;
			outSoundData->SoundObject = soundObject;
		};

		if (propValue != "Sound" && propValue != "ContentFile") {
			readSound(propValue, reader, &soundData);
			return soundData;
		}

		while (reader.NextProperty()) {
			std::string soundSubPropertyName = reader.ReadPropName();
			if (soundSubPropertyName == "FilePath" || soundSubPropertyName == "Path") {
				readSound(reader.ReadPropValue(), reader, &soundData);
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

	void SoundContainer::AddSound(std::string const &soundFilePath, unsigned int soundSetIndex, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound) {
		vector<SoundData> soundSet;
		if (soundSetIndex < m_SoundSets.size()) { soundSet = m_SoundSets[soundSetIndex]; }

		ContentFile soundFile(soundFilePath.c_str());
		FMOD::Sound *soundObject = soundFile.GetAsSample(abortGameForInvalidSound);
		if (!soundObject) {
			return;
		}

		attenuationStartDistance = attenuationStartDistance < 0 ? c_DefaultAttenuationStartDistance : attenuationStartDistance;
		soundSet.push_back({soundFile, soundObject, offset, minimumAudibleDistance, attenuationStartDistance});
		if (soundSetIndex >= m_SoundSets.size()) { m_SoundSets.push_back(soundSet); }

		m_AllSoundPropertiesUpToDate = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	std::vector<size_t> SoundContainer::GetSelectedSoundHashes() {
		std::vector<size_t> soundHashes;
		for (SoundData selectedSoundData : m_SoundSets[m_SelectedSoundSet]) {
			soundHashes.push_back(selectedSoundData.SoundFile.GetHash());
		}
		return soundHashes;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	const SoundContainer::SoundData *SoundContainer::GetSoundDataForSound(const FMOD::Sound *sound) const {
		for (std::vector<SoundData> const &soundSet : m_SoundSets) {
			for (SoundData const &soundData : soundSet) {
				if (sound == soundData.SoundObject) {
					return &soundData;
				}
			}
		}
		return NULL;
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
				auto selectRandomSound = [](int soundSetSize, size_t *outSelectedSoundSet) {
					size_t soundToSelect = floorf(static_cast<float>(soundSetSize) * PosRand());
					while (soundToSelect == *outSelectedSoundSet) {
						soundToSelect = floorf(static_cast<float>(soundSetSize) * PosRand());
					}
					*outSelectedSoundSet = soundToSelect;
				};

				switch (m_SoundSelectionCycleMode) {
					case MODE_RANDOM:
						selectRandomSound(soundSetCount, &m_SelectedSoundSet);
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
					soundMode |= FMOD_3D_CUSTOMROLLOFF;
				}

				result = (result == FMOD_OK) ? soundData.SoundObject->setMode(soundMode) : result;
				result = (result == FMOD_OK) ? soundData.SoundObject->setLoopCount(m_Loops) : result;
				if (m_Immobile) {
					result = (result == FMOD_OK) ? soundData.SoundObject->set3DMinMaxDistance(m_AttenuationStartDistance, c_SoundMaxAudibleDistance) : result;
				} else {
					//FMOD_VECTOR customRolloffPoints[10];
					//CalculateCustomRolloffPoints(soundData, customRolloffPoints, 10);

					//TODO Consider replacing this with normal min and max distance (but keep custom rolloff mode) so we can save some pointing issues. Might need to store min audible distance in audioman if fmod is strict about min and max distance sizes wrt each other.
					soundData.CustomRolloffPoints[0] = FMOD_VECTOR{soundData.MinimumAudibleDistance, 0, 0};
					soundData.CustomRolloffPoints[1] = FMOD_VECTOR{soundData.AttenuationStartDistance < 0 ? m_AttenuationStartDistance : soundData.AttenuationStartDistance, 1, 0};
					result = (result == FMOD_OK) ? soundData.SoundObject->set3DCustomRolloff(soundData.CustomRolloffPoints, 2) : result;
				}
			}
		}
		m_AllSoundPropertiesUpToDate = result == FMOD_OK;

		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//TODO this needs to be used or be deleted
	void SoundContainer::CalculateCustomRolloffPoints(const SoundData &soundDataToCalculateFor, FMOD_VECTOR *rolloffPoints, int numRolloffPoints) {
		int attenuationStartDistance = soundDataToCalculateFor.AttenuationStartDistance < 0 ? m_AttenuationStartDistance : soundDataToCalculateFor.AttenuationStartDistance;
		float currentDistance = attenuationStartDistance;
		float currentVolumeLevel = 1;

		int i = 0;
		if (soundDataToCalculateFor.MinimumAudibleDistance > 0) {
			i = 2;
			currentDistance += soundDataToCalculateFor.MinimumAudibleDistance;
			rolloffPoints[0] = FMOD_VECTOR{0, 0, 0};
			rolloffPoints[1] = FMOD_VECTOR{soundDataToCalculateFor.MinimumAudibleDistance - 0.1F, 0, 0};
		}

		for (i = (soundDataToCalculateFor.MinimumAudibleDistance > 0 ? 2 : 0); i < numRolloffPoints - 1; i++) {
			rolloffPoints[i] = FMOD_VECTOR{currentDistance, currentVolumeLevel, 0};
			currentDistance += attenuationStartDistance;
			currentVolumeLevel = currentDistance < c_SoundMaxAudibleDistance ? currentVolumeLevel * 0.5 : 0;
		}
		rolloffPoints[numRolloffPoints - 1] = FMOD_VECTOR{currentDistance, 0, 0};
	}
}
