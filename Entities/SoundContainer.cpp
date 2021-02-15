#include "SoundContainer.h"
#include "SettingsMan.h"

namespace RTE {

	ConcreteClassInfo(SoundContainer, Entity, 50);

	const std::unordered_map<std::string, SoundContainer::SoundOverlapMode> SoundContainer::c_SoundOverlapModeMap = {
		{"Overlap", SoundContainer::SoundOverlapMode::OVERLAP},
		{"Restart", SoundContainer::SoundOverlapMode::RESTART},
		{"Ignore Play", SoundContainer::SoundOverlapMode::IGNORE_PLAY}
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundContainer::Clear() {
		m_TopLevelSoundSet.Destroy();

		m_PlayingChannels.clear();
		m_SoundOverlapMode = SoundOverlapMode::OVERLAP;

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

		m_TopLevelSoundSet.Create(reference.m_TopLevelSoundSet);

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

	int SoundContainer::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "AddSound") {
			m_TopLevelSoundSet.AddSoundData(SoundSet::ReadAndGetSoundData(reader));
		} else if (propName == "AddSoundSet") {
			SoundSet soundSetToAdd;
			reader >> soundSetToAdd;
			m_TopLevelSoundSet.AddSoundSet(soundSetToAdd);
		} else if (propName == "SoundSelectionCycleMode" || propName == "CycleMode") {
			m_TopLevelSoundSet.SetSoundSelectionCycleMode(SoundSet::ReadSoundSelectionCycleMode(reader));
		} else if (propName == "SoundOverlapMode") {
			std::string soundOverlapModeString = reader.ReadPropValue();
			if (c_SoundOverlapModeMap.find(soundOverlapModeString) != c_SoundOverlapModeMap.end()) {
				m_SoundOverlapMode = c_SoundOverlapModeMap.find(soundOverlapModeString)->second;
			} else {
				try {
					m_SoundOverlapMode = static_cast<SoundOverlapMode>(std::stoi(soundOverlapModeString));
				} catch (const std::exception &) {
					reader.ReportError("Cycle mode " + soundOverlapModeString + " is invalid.");
				}
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

	int SoundContainer::Save(Writer &writer) const {
		Entity::Save(writer);

		writer << m_TopLevelSoundSet;

		writer.NewProperty("SoundSelectionCycleMode");
		SoundSet::SaveSoundSelectionCycleMode(writer, m_TopLevelSoundSet.GetSoundSelectionCycleMode());

		writer.NewProperty("SoundOverlapMode");
		auto overlapModeMapEntry = std::find_if(c_SoundOverlapModeMap.begin(), c_SoundOverlapModeMap.end(), [&soundOverlapMode = m_SoundOverlapMode](auto element) { return element.second == soundOverlapMode; });
		if (overlapModeMapEntry != c_SoundOverlapModeMap.end()) {
			writer << overlapModeMapEntry->first;
		} else {
			RTEAbort("Tried to write invalid SoundOverlapMode when saving SoundContainer.");
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

	std::vector<std::size_t> SoundContainer::GetSelectedSoundHashes() const {
		std::vector<size_t> soundHashes;
		std::vector<const SoundSet::SoundData *> flattenedSoundData;
		m_TopLevelSoundSet.GetFlattenedSoundData(flattenedSoundData, false);
		for (const SoundSet::SoundData *selectedSoundData : flattenedSoundData) {
			soundHashes.push_back(selectedSoundData->SoundFile.GetHash());
		}
		return soundHashes;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	const SoundSet::SoundData * SoundContainer::GetSoundDataForSound(const FMOD::Sound *sound) const {
		std::vector<const SoundSet::SoundData *> flattenedSoundData;
		m_TopLevelSoundSet.GetFlattenedSoundData(flattenedSoundData, false);
		for (const SoundSet::SoundData *soundData : flattenedSoundData) {
			if (sound == soundData->SoundObject) {
				return soundData;
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundContainer::Play(int player) {
		if (HasAnySounds()) {
			if (IsBeingPlayed()) {
				if (m_SoundOverlapMode == SoundOverlapMode::RESTART) {
					Restart(player);
				} else if (m_SoundOverlapMode == SoundOverlapMode::IGNORE_PLAY) {
					return false;
				}
			}
			return g_AudioMan.PlaySoundContainer(this, player);

		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT SoundContainer::UpdateSoundProperties() {
		FMOD_RESULT result = FMOD_OK;

		std::vector<SoundSet::SoundData *> flattenedSoundData;
		m_TopLevelSoundSet.GetFlattenedSoundData(flattenedSoundData, false);
		for (SoundSet::SoundData *soundData : flattenedSoundData) {
			FMOD_MODE soundMode = (m_Loops == 0) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL;
			if (m_Immobile) {
				soundMode |= FMOD_3D_HEADRELATIVE;
				m_AttenuationStartDistance = c_SoundMaxAudibleDistance;
			} else if (g_AudioMan.GetSoundPanningEffectStrength() == 1.0F) {
				soundMode |= FMOD_3D_INVERSEROLLOFF;
			} else {
				soundMode |= FMOD_3D_CUSTOMROLLOFF;
			}

			result = (result == FMOD_OK) ? soundData->SoundObject->setMode(soundMode) : result;
			result = (result == FMOD_OK) ? soundData->SoundObject->setLoopCount(m_Loops) : result;
			m_AttenuationStartDistance = std::clamp(m_AttenuationStartDistance, 0.0F, static_cast<float>(c_SoundMaxAudibleDistance) - soundData->MinimumAudibleDistance);
			result = (result == FMOD_OK) ? soundData->SoundObject->set3DMinMaxDistance(soundData->MinimumAudibleDistance + m_AttenuationStartDistance, c_SoundMaxAudibleDistance) : result;
		}
		m_SoundPropertiesUpToDate = result == FMOD_OK;

		return result;
	}
}
