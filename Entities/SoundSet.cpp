#include "SoundSet.h"

namespace RTE {

	const std::string SoundSet::m_sClassName = "SoundSet";

	const std::unordered_map<std::string, SoundContainer::SoundCycleMode> SoundContainer::c_SoundCycleModeMap = {
		{"Random", SoundSet::SoundCycleMode::RANDOM},
		{"Forwards", SoundSet::SoundCycleMode::FORWARDS},
		{"All", SoundSet::SoundCycleMode::ALL}
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundSet::Clear() {
		m_SoundSelectionCycleMode = SoundSelectionCycleMode::RANDOM;
		m_CurrentSelection.clear();

		m_SoundData.clear();
		m_SubSoundSets.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundSet::Create(const SoundSet &reference) {
		m_SoundSelectionCycleMode = reference.m_SoundSelectionCycleMode;
		m_CurrentSelection = reference.m_CurrentSelection;
		for (SoundData referenceSoundData : reference.m_SoundData) {
			m_SoundData.push_back(referenceSoundData);
		}
		for (const SoundSet &referenceSoundSet : reference.m_SoundSets) {
			SoundSet soundSet;
			soundSet.Create(referenceSoundSet);
			m_SubSoundSets.push_back(soundSet);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundSet::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "SoundSelectionCycleMode" || propName == "CycleMode") {
			m_SoundSelectionCycleMode = ReadSoundSelectionCycleMode(reader);
		} else if (propName == "AddSound") {
			AddSoundData(ReadAndGetSound(reader));
		} else if (propName == "AddSoundSet" || propName == "AddSubSoundSet") {
			SoundSet subSoundSet;
			reader >> subSoundSet;
			AddSubSoundSet(subSoundSet);
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static SoundData ReadAndGetSound(Reader &reader) {
		SoundData soundData;

		/// <summary>
		/// Internal lambda function to load an audio file by path in as a ContentFile, which in turn loads it into FMOD, then returns SoundData for it in the outParam outSoundData.
		/// </summary>
		/// <param name="soundPath">The path to the sound file.</param>
		auto readSoundDirectlyFromPath = [&soundData, &reader](const std::string &soundPath) {
			ContentFile soundFile(soundPath.c_str());
			soundFile.SetFormattedReaderPosition("in file " + reader.GetCurrentFilePath() + " on line " + std::to_string(reader.GetCurrentFileLine()));
			FMOD::Sound *soundObject = soundFile.GetAsSound();
			if (g_AudioMan.IsAudioEnabled() && !soundObject) { reader.ReportError(std::string("Failed to load the sound from the file")); }

			soundData.SoundFile = soundFile;
			soundData.SoundObject = soundObject;
		};

		std::string propValue = reader.ReadPropValue();
		if (propValue != "Sound" && propValue != "ContentFile") {
			readSoundDirectlyFromPath(propValue);
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

	static SoundSelectionCycleMode SoundSet::ReadSoundSelectionCycleMode(const Reader &reader) {
		std::string cycleModeString = reader.ReadPropValue();
		SoundSelectionCycleMode soundSelectionCycleMode = SoundSelectionCycleMode::RANDOM;
		if (c_SoundCycleModeMap.find(cycleModeString) != c_SoundCycleModeMap.end()) {
			soundSelectionCycleMode = c_SoundCycleModeMap.find(cycleModeString)->second;
		} else {
			reader.ReportError("Cycle mode " + cycleModeString + " is invalid.");
		}
		return soundSelectionCycleMode;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundSet::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewProperty("SoundSelectionCycleMode");
		SaveSoundSelectionCycleMode(m_SoundSelectionCycleMode);

		for (const SoundData &soundData : m_SoundData) {
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

		for (const SoundSet &subSoundSet : m_SubSoundSets) {
			writer.NewProperty("AddSoundSet");
			writer.ObjectStart("SoundSet");
			writer << subSoundSet;
			writer.ObjectEnd();
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	static void SoundSet::SaveSoundSelectionCycleMode(Writer &writer, SoundSelectionCycleMode soundSelectionCycleMode) {
		std::list<std::pair<const std::string, SoundCycleMode>>::const_iterator cycleModeMapEntry = std::find_if(c_SoundCycleModeMap.begin(), c_SoundCycleModeMap.end(), [&soundSelectionCycleMode = soundSelectionCycleMode](auto element) { return element.second == soundSelectionCycleMode; });
		if (cycleModeMapEntry != c_SoundCycleModeMap.end()) {
			writer << cycleModeMapEntry->first;
		} else {
			RTEAbort("Tried to write invalid SoundSelectionCycleMode when saving SoundContainer/SoundSet.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundSet::AddSound(const std::string &soundFilePath, const Vector &offset, float minimumAudibleDistance, float attenuationStartDistance, bool abortGameForInvalidSound) {
		ContentFile soundFile(soundFilePath.c_str());
		FMOD::Sound *soundObject = soundFile.GetAsSound(abortGameForInvalidSound, false);
		if (!soundObject) {
			return;
		}

		m_SoundData.push_back({soundFile, soundObject, offset, minimumAudibleDistance, attenuationStartDistance});
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundSet::HasAnySounds(bool includeSubSoundSets) const {
		bool hasAnySounds = !m_SoundSets.empty();
		if (!hasAySounds && includeSubSoundSets) {
			for (const SoundSet &subSoundSet : m_SubSoundSets) {
				hasAnySounds = subSoundSet.HasAnySounds();
				if (hasAnySounds) { break; }
			}
		}
		return hasAnySounds;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundSet::GetFlattenedSoundData(std::vector<SoundData *> &flattenedSoundData, bool onlyGetSelectedSoundData) const {
		if (!onlyGetSelectedSoundData || m_SoundSelectionCycleMode == SoundSelectionCycleMode::ALL) {
			for (SoundData &soundData : m_SoundData) { flattenedSoundData.push_back(&soundData); }
			for (const SoundSet &subSoundSet : m_SubSoundSets) { subSoundSet.GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData); }
		} else {
			if (m_CurrentSelection.first == false) {
				flattenedSoundData.push_back(&m_SoundData[m_CurrentSelection.second]);
			} else {
				m_CurrentSelection.second.GetFlattenedSoundData(flattenedSoundData, onlyGetSelectedSoundData);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundSet::SelectNextSoundSet() const {
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
					case RANDOM:
						selectRandomSound();
						break;
					case FORWARDS:
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
}