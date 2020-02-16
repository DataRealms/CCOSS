#include "Sound.h"
#include "DDTTools.h"
#include "AudioMan.h"

namespace RTE {

	CONCRETECLASSINFO(Sound, Entity, 0);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Sound::Clear() {
		m_Samples.clear();
		m_CurrentSample = 0;
		m_LastChannel = -1;
		m_Loops = 0;
		m_Priority = AudioMan::PRIORITY_LOW;
		m_AffectedByPitch = true;
		m_Hash = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Sound::Create(const Sound &reference) {
		Entity::Create(reference);

		for (std::vector<std::pair<ContentFile, AUDIO_STRUCT * >>::const_iterator itr = reference.m_Samples.begin(); itr != reference.m_Samples.end(); ++itr) {
			m_Samples.push_back(*itr);
		}
		m_CurrentSample = reference.m_CurrentSample;
		m_LastChannel = -1;
		m_Loops = reference.m_Loops;
		m_Priority = reference.m_Priority;
		m_AffectedByPitch = reference.m_AffectedByPitch;
		m_Hash = reference.m_Hash;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Sound::Create(std::string samplePath, bool pitched, int loops) {
		AddSample(samplePath);
		m_AffectedByPitch = pitched;
		m_Loops = loops;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Sound::ReadProperty(std::string propName, Reader &reader) {
		if (propName == "AddSample") {
			ContentFile newFile;
			reader >> newFile;
			m_Hash = newFile.GetHash();

			AUDIO_STRUCT *pNewSample = newFile.GetAsSample();
			if (!pNewSample) {

#ifdef __USE_SOUND_FMOD
				reader.ReportError(std::string("Failed to load the sample from the file, error: ") + FMOD_ErrorString(FSOUND_GetError()));
#elif __USE_SOUND_GORILLA
				reader.ReportError(std::string("Failed to load the sample from the file, error: "));
#endif
			}
			m_Samples.push_back(std::pair<ContentFile, AUDIO_STRUCT *>(newFile, pNewSample));
		} else if (propName == "LoopSetting")
			reader >> m_Loops;
		else if (propName == "Priority")
			reader >> m_Priority;
		else if (propName == "AffectedByPitch")
			reader >> m_AffectedByPitch;
		else
			// See if the base class(es) can find a match instead
			return Entity::ReadProperty(propName, reader);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Sound::Save(Writer &writer) const {
		Entity::Save(writer);

		for (std::vector<std::pair<ContentFile, AUDIO_STRUCT *>>::const_iterator itr = m_Samples.begin(); itr != m_Samples.end(); ++itr) {
			writer.NewProperty("AddSample");
			writer << (*itr).first;
		}
		writer.NewProperty("LoopSetting");
		writer << m_Loops;
		writer.NewProperty("Priority");
		writer << m_Priority;
		writer.NewProperty("AffectedByPitch");
		writer << m_AffectedByPitch;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Sound::Destroy(bool notInherited) {
		// Don't delete samples since they are owned in the CoententFile static maps
		if (!notInherited)
			Entity::Destroy();
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Sound::AddSample(std::string samplePath) {
		ContentFile newFile(samplePath.c_str());
		m_Hash = newFile.GetHash();

		AUDIO_STRUCT *pNewSample = newFile.GetAsSample();
		AAssert(pNewSample, "Failed to load the sample from the file");
		m_Samples.push_back(std::pair<ContentFile, AUDIO_STRUCT *>(newFile, pNewSample));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Sound::IsBeingPlayed() { return g_AudioMan.IsPlaying(this); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AUDIO_STRUCT * Sound::GetCurrentSample() {
		DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
		return m_Samples[m_CurrentSample].second;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Sound::UpdateAttenuation(float distance) { return g_AudioMan.SetSoundAttenuation(this, distance); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Sound::Play(float distance, int player) {
		if (!m_Samples.empty()) {
			return g_AudioMan.PlaySound(player, this, m_Priority, distance);
		} else return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Sound::Stop(int player) {
		if (!m_Samples.empty()) {
			return g_AudioMan.StopSound(player, this);
		} else return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AUDIO_STRUCT * Sound::StartNextSample() {

		// Only two samples; alternate
		if (m_Samples.size() == 2) {
			m_CurrentSample = m_CurrentSample == 0 ? 1 : 0;
		}
		// More than two, select randomly
		else if (m_Samples.size() > 2) {
			int lastSample = m_CurrentSample;
			m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
			// Mix it up again if we got the same sound twice
			if (m_CurrentSample == lastSample) {
				m_CurrentSample = floorf((float)m_Samples.size() * PosRand());
			}
		}
		DAssert(m_CurrentSample >= 0 && m_CurrentSample < m_Samples.size(), "Sample index is out of bounds!");
		return m_Samples[m_CurrentSample].second;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Sound::FadeOut(int fadeOutTime) {
		if (!m_Samples.empty()) {
			return g_AudioMan.FadeOutSound(this, fadeOutTime);
		}
	}
}
