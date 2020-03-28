#include "SoundContainer.h"
#include "RTETools.h"

namespace RTE {

	CONCRETECLASSINFO(SoundContainer, Entity, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SoundContainer::Clear() {
		m_Sounds.clear();
		m_SelectedSounds.clear();
		m_PlayingChannels.clear();
		m_AttenuationStartDistance = 1;
		m_Loops = 0;
		m_Priority = AudioMan::PRIORITY_NORMAL;
		m_AffectedByGlobalPitch = true;
		m_Immobile = false;

		m_AllSoundPropertiesUpToDate = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SoundContainer::Create(const SoundContainer &reference) {
		Entity::Create(reference);

		for (std::vector<std::pair<ContentFile, FMOD::Sound *>>::const_iterator itr = reference.m_Sounds.begin(); itr != reference.m_Sounds.end(); ++itr) {
			m_Sounds.push_back(*itr);
		}
		m_SelectedSounds = reference.m_SelectedSounds;
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
		if (propName == "AddSound") {
			ContentFile newFile;
			reader >> newFile;
			FMOD::Sound *pNewSample = newFile.GetAsSample();
			if (!pNewSample) { reader.ReportError(std::string("Failed to load the sample from the file")); }
			m_Sounds.push_back(std::pair<ContentFile, FMOD::Sound *>(newFile, pNewSample));
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

	void SoundContainer::AddSound(std::string const soundPath, bool abortGameForInvalidSound) {
		ContentFile newFile(soundPath.c_str());

		FMOD::Sound *newSample = newFile.GetAsSample(abortGameForInvalidSound);
		if (newSample) {
			FMOD_RESULT result = newSample->setMode((m_Loops == 0) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL);
			result = (result == FMOD_OK) ? newSample->setLoopCount(m_Loops) : result;
			m_Sounds.push_back(std::pair<ContentFile, FMOD::Sound *>(newFile, newSample));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	std::vector<size_t> SoundContainer::GetSelectedSoundHashes() {
		std::vector<size_t> soundHashes;
		for (size_t selectedSoundIndex : m_SelectedSounds) {
			soundHashes.push_back(m_Sounds[selectedSoundIndex].first.GetHash());
		}
		return soundHashes;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<FMOD::Sound *> SoundContainer::GetSelectedSoundObjects() {
 		std::vector<FMOD::Sound *> soundObjects;
		for (size_t selectedSoundIndex : m_SelectedSounds) {
			soundObjects.push_back(m_Sounds[selectedSoundIndex].second);
		}
		return soundObjects;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SoundContainer::SelectNextSounds() {
		int soundCount = GetSoundCount();
		if (soundCount == 0) {
			return false;
		}
		size_t previouslySelectedSound = (m_SelectedSounds.size() > 0) ? m_SelectedSounds[0] : 0;

		if (m_SelectedSounds.empty() || soundCount == 1) {
			m_SelectedSounds.clear();
			m_SelectedSounds.push_back(previouslySelectedSound);
		} else {
			m_SelectedSounds.clear();
			if (soundCount == 2) {
				m_SelectedSounds.push_back((previouslySelectedSound + 1) % soundCount);
			} else if (soundCount > 2) {
				size_t soundToSelect = floorf(static_cast<float>(soundCount) * PosRand());
				// Mix it up again if we got the same sound twice
				while (soundToSelect == previouslySelectedSound) {
					soundToSelect = floorf(static_cast<float>(soundCount) * PosRand());
				}
				m_SelectedSounds.push_back(soundToSelect);
			}
		}
		RTEAssert(m_SelectedSounds.size() > 0 && m_SelectedSounds[0] >= 0 && m_SelectedSounds[0] < soundCount, "Failed to select next sound, either none was selected or the selected sound was invalid.");
	
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FMOD_RESULT SoundContainer::UpdateSoundProperties() {
		FMOD_RESULT result = FMOD_OK;

		for (std::vector<std::pair<ContentFile, FMOD::Sound * >>::const_iterator itr = m_Sounds.begin(); itr != m_Sounds.end() && result == FMOD_OK; ++itr) {
			FMOD_MODE soundMode = (m_Loops == 0) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL;
			soundMode |= m_Immobile ? FMOD_3D_HEADRELATIVE : FMOD_3D_WORLDRELATIVE;

			result = (result == FMOD_OK) ? itr->second->setMode(soundMode) : result;
			result = (result == FMOD_OK) ? itr->second->setLoopCount(m_Loops) : result;
			result = (result == FMOD_OK) ? itr->second->set3DMinMaxDistance(m_AttenuationStartDistance, 100000) : result;
		}
		m_AllSoundPropertiesUpToDate = result == FMOD_OK;

		return result;
	}
}
