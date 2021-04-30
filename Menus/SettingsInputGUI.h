#ifndef _RTESETTINGSINPUTGUI_
#define _RTESETTINGSINPUTGUI_

#include "SettingsInputMappingGUI.h"

struct BITMAP;

namespace RTE {

	class Controller;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUISlider;
	class GUIRadioButton;
	class GUIEvent;

	/// <summary>
	/// Handling for player input settings through the game settings user interface.
	/// </summary>
	class SettingsInputGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsInputGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsInputGUI.</param>
		explicit SettingsInputGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Input Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Input Settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// User input handling for the Input Settings menu.
		/// </summary>
		/// <param name="guiEvent"></param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		struct PlayerInputSettingsBox {
			GUILabel *SelectedDeviceLabel; //!< Label for the input device name that is currently being used.
			GUIButton *NextDeviceButton; //!< Button for changing to the next input device type.
			GUIButton *PrevDeviceButton; //!< Button for changing to the previous input device type.
			GUIButton *ConfigureControlsButton; //!< Button for starting the controls configuration wizard.
			GUIButton *ResetControlsButton; //!< Button to clear all mapped controls.
			GUILabel *SensitivtyLabel; //!< Label for the mouse sensitivity or controller deadzone value.
			GUISlider *SensitivitySlider; //!< Slider for setting the controller deadzone value.
			GUICollectionBox *DeadZoneControlsBox; //!<
			GUIRadioButton *CircleDeadZoneRadioButton; //!< Checkbox for setting the controller deadzone type.
			GUIRadioButton *SquareDeadZoneRadioButton; //!<
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_InputSettingsBox; //!<

		std::unique_ptr<SettingsInputMappingGUI> m_InputConfigMenu;

		std::array<PlayerInputSettingsBox, Players::MaxPlayerCount> m_PlayerInputSettingsBoxes; //!<

#pragma region Input Settings Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		/// <param name="nextDevice"></param>
		void SetPlayerNextOrPrevInputDevice(int player, bool nextDevice);

		/// <summary>
		/// Updates the text on the configuration labels, based on actual UInputMan settings.
		/// </summary>
		/// <param name="player"></param>
		void UpdatePlayerSelectedDeviceLabel(int player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void ShowPlayerSensitivityControls(int player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void UpdatePlayerSensitivityValues(int player);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void ResetPlayerControlMappings(int player);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputGUI(const SettingsInputGUI &reference) = delete;
		SettingsInputGUI & operator=(const SettingsInputGUI &rhs) = delete;
	};
}
#endif