#include "SettingsAudioGUI.h"
#include "AudioMan.h"

#include "GUI.h"
#include "GUILabel.h"
#include "GUISlider.h"
#include "GUICheckbox.h"
#include "GUICollectionBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsAudioGUI::SettingsAudioGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_AudioSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxAudioSettings"));

		m_MusicLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelMusicVolume"));
		m_MusicVolumeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderMusicVolume"));
		//m_MusicMuteCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMuteMusic"));

		m_SoundLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSoundVolume"));
		m_SoundVolumeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderSoundVolume"));
		//m_MusicMuteCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMuteSound"));

		UpdateMusicVolumeSlider();
		UpdateSoundVolumeSlider();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::SetEnabled(bool enable) const {
		if (enable) {
			m_AudioSettingsBox->SetVisible(true);
			m_AudioSettingsBox->SetEnabled(true);
		} else {
			m_AudioSettingsBox->SetVisible(false);
			m_AudioSettingsBox->SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::UpdateMusicVolumeSlider() {
		int musicVolume = static_cast<int>(std::round(g_AudioMan.GetMusicVolume() * 100));
		m_MusicLabel->SetText("- Music Volume: " + std::to_string(musicVolume));
		m_MusicVolumeSlider->SetValue(musicVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::UpdateSoundVolumeSlider() {
		int soundVolume = static_cast<int>(std::round(g_AudioMan.GetSoundsVolume() * 100));
		m_SoundLabel->SetText("- Sound Volume: " + std::to_string(soundVolume));
		m_SoundVolumeSlider->SetValue(soundVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void SettingsAudioGUI::MuteMusic() {

	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void SettingsAudioGUI::MuteSounds() {

	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_MusicVolumeSlider) {
				g_AudioMan.SetMusicVolume(static_cast<float>(m_MusicVolumeSlider->GetValue()) / 100);
				UpdateMusicVolumeSlider();
			} else if (guiEvent.GetControl() == m_SoundVolumeSlider) {
				float newSoundVolume = (static_cast<float>(m_SoundVolumeSlider->GetValue()) / 100);
				if (newSoundVolume != g_AudioMan.GetSoundsVolume() && !g_GUISound.TestSound()->IsBeingPlayed()) { g_GUISound.TestSound()->Play(); }
				g_AudioMan.SetSoundsVolume(newSoundVolume);
				UpdateSoundVolumeSlider();
			}
		}
	}
}