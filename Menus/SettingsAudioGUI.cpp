#include "SettingsAudioGUI.h"
#include "AudioMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUISlider.h"
#include "GUICheckbox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsAudioGUI::SettingsAudioGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_AudioSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxAudioSettings"));

		m_MasterVolumeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelMasterVolume"));
		m_MasterVolumeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderMasterVolume"));
		m_MasterMuteCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMuteMaster"));
		m_MasterMuteCheckbox->SetCheck(g_AudioMan.GetMasterMuted());

		m_MusicVolumeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelMusicVolume"));
		m_MusicVolumeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderMusicVolume"));
		m_MusicMuteCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMuteMusic"));
		m_MusicMuteCheckbox->SetCheck(g_AudioMan.GetMusicMuted());

		m_SoundVolumeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSoundVolume"));
		m_SoundVolumeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderSoundVolume"));
		m_SoundMuteCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMuteSound"));
		m_SoundMuteCheckbox->SetCheck(g_AudioMan.GetSoundsMuted());

		UpdateMasterVolumeControls();
		UpdateMusicVolumeControls();
		UpdateSoundVolumeControls();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::SetEnabled(bool enable) const {
		m_AudioSettingsBox->SetVisible(enable);
		m_AudioSettingsBox->SetEnabled(enable);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::UpdateMasterVolumeControls() {
		int masterVolume = static_cast<int>(std::round(g_AudioMan.GetMasterVolume() * 100));
		m_MasterVolumeLabel->SetText("Volume: " + std::to_string(masterVolume));
		m_MasterVolumeSlider->SetValue(masterVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::UpdateMusicVolumeControls() {
		int musicVolume = static_cast<int>(std::round(g_AudioMan.GetMusicVolume() * 100));
		m_MusicVolumeLabel->SetText("Volume: " + std::to_string(musicVolume));
		m_MusicVolumeSlider->SetValue(musicVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::UpdateSoundVolumeControls() {
		int soundVolume = static_cast<int>(std::round(g_AudioMan.GetSoundsVolume() * 100));
		m_SoundVolumeLabel->SetText("Volume: " + std::to_string(soundVolume));
		m_SoundVolumeSlider->SetValue(soundVolume);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsAudioGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_MasterVolumeSlider) {
				float newMasterVolume = static_cast<float>(m_MasterVolumeSlider->GetValue()) / 100.0F;
				if (newMasterVolume != g_AudioMan.GetSoundsVolume() && !g_GUISound.TestSound()->IsBeingPlayed()) { g_GUISound.TestSound()->Play(); }
				g_AudioMan.SetMasterVolume(newMasterVolume);
				UpdateMasterVolumeControls();
			} else if (guiEvent.GetControl() == m_MusicVolumeSlider) {
				g_AudioMan.SetMusicVolume(static_cast<float>(m_MusicVolumeSlider->GetValue()) / 100.0F);
				UpdateMusicVolumeControls();
			} else if (guiEvent.GetControl() == m_SoundVolumeSlider) {
				float newSoundVolume = static_cast<float>(m_SoundVolumeSlider->GetValue()) / 100.0F;
				if (newSoundVolume != g_AudioMan.GetSoundsVolume() && !g_GUISound.TestSound()->IsBeingPlayed()) { g_GUISound.TestSound()->Play(); }
				g_AudioMan.SetSoundsVolume(newSoundVolume);
				UpdateSoundVolumeControls();
			} else if (guiEvent.GetControl() == m_MasterMuteCheckbox) {
				g_AudioMan.SetMasterMuted(m_MasterMuteCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_MusicMuteCheckbox) {
				g_AudioMan.SetMusicMuted(m_MusicMuteCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_SoundMuteCheckbox) {
				g_AudioMan.SetSoundsMuted(m_SoundMuteCheckbox->GetCheck());
			}
		}
	}
}