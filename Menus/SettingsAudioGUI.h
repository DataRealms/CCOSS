#ifndef _RTESETTINGSAUDIOGUI_
#define _RTESETTINGSAUDIOGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUILabel;
	class GUISlider;
	class GUICheckbox;
	class GUIEvent;

	/// <summary>
	/// Handling for audio settings through the game settings user interface.
	/// </summary>
	class SettingsAudioGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsAudioGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsAudioGUI.</param>
		explicit SettingsAudioGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Audio Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Audio Settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// User input handling for the Audio Settings menu.
		/// </summary>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_AudioSettingsBox; //!< The GUICollectionBox that contains all the Audio Settings menu GUI elements.

		GUILabel *m_MusicLabel; //!< GUILabel that shows the current music volume value.
		GUISlider *m_MusicVolumeSlider; //!< GUISlider that controls the music volume.
		GUICheckbox *m_MusicMuteCheckbox; //!< GUICheckbox that mutes and unmutes the music volume.

		GUILabel *m_SoundLabel; //!< GUILabel that shows the current sound effects volume value.
		GUISlider *m_SoundVolumeSlider; //!< GUISlider that controls the sound effects volume.
		GUICheckbox *m_SoundMuteCheckbox; //!< GUICheckbox that mutes and unmutes the sound effects volume.

#pragma region Audio Settings Handling
		/// <summary>
		/// Updates the position of the music volume slider, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateMusicVolumeSlider();

		/// <summary>
		/// Updates the position of the sound volume slider, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateSoundVolumeSlider();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsAudioGUI(const SettingsAudioGUI &reference) = delete;
		SettingsAudioGUI & operator=(const SettingsAudioGUI &rhs) = delete;
	};
}
#endif