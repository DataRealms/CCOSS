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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsAudioGUI. Ownership is NOT transferred!</param>
		explicit SettingsAudioGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the SettingsAudioGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsAudioGUI.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// Handles the player interaction with the AudioVideoGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		/// <summary>
		/// GUI elements that compose the audio settings menu screen.
		/// </summary>
		GUICollectionBox *m_AudioSettingsBox;
		GUILabel *m_MasterVolumeLabel;
		GUISlider *m_MasterVolumeSlider;
		GUICheckbox *m_MasterMuteCheckbox;
		GUILabel *m_MusicVolumeLabel;
		GUISlider *m_MusicVolumeSlider;
		GUICheckbox *m_MusicMuteCheckbox;
		GUILabel *m_SoundVolumeLabel;
		GUISlider *m_SoundVolumeSlider;
		GUICheckbox *m_SoundMuteCheckbox;

#pragma region Audio Settings Handling
		/// <summary>
		/// Updates the position of the master volume slider and volume value label, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateMasterVolumeControls();

		/// <summary>
		/// Updates the position of the music volume slider and volume value label, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateMusicVolumeControls();

		/// <summary>
		/// Updates the position of the sound volume slider and volume value label, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateSoundVolumeControls();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsAudioGUI(const SettingsAudioGUI &reference) = delete;
		SettingsAudioGUI & operator=(const SettingsAudioGUI &rhs) = delete;
	};
}
#endif