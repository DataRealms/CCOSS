#ifndef _RTESETTINGSINPUTGUI_
#define _RTESETTINGSINPUTGUI_

#include "SettingsInputMappingGUI.h"

struct BITMAP;

namespace RTE {

	class Controller;
	class GUIControlManager;
	class GUICollectionBox;
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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputGUI.</param>
		explicit SettingsInputGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the input settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the input settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// Handles the player interaction with the SettingsInputGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// Struct containing GUI elements that compose the input settings box of a player.
		/// </summary>
		struct PlayerInputSettingsBox {
			GUILabel *SelectedDeviceLabel;
			GUIButton *NextDeviceButton;
			GUIButton *PrevDeviceButton;
			GUIButton *ConfigureControlsButton;
			GUIButton *ResetControlsButton;
			GUILabel *SensitivtyLabel;
			GUISlider *SensitivitySlider;
			GUICollectionBox *DeadZoneControlsBox;
			GUIRadioButton *CircleDeadZoneRadioButton;
			GUIRadioButton *SquareDeadZoneRadioButton;
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		std::unique_ptr<SettingsInputMappingGUI> m_InputMappingConfigMenu; //!< The input mapping configuration sub-menu.

		/// <summary>
		/// GUI elements that compose the input settings menu screen.
		/// </summary>
		GUICollectionBox *m_InputSettingsBox;
		std::array<PlayerInputSettingsBox, Players::MaxPlayerCount> m_PlayerInputSettingsBoxes;

#pragma region Input Settings Handling
		/// <summary>
		/// Resets the player input settings to the defaults.
		/// </summary>
		/// <param name="player">The player to reset input settings for.</param>
		void ResetPlayerInputSettings(int player);

		/// <summary>
		/// Changes the player's input device in the InputScheme and proceeds to update the device labels accordingly.
		/// </summary>
		/// <param name="player">The player to change input device for.</param>
		/// <param name="nextDevice">Whether to change to the next or previous input device.</param>
		void SetPlayerNextOrPrevInputDevice(int player, bool nextDevice);

		/// <summary>
		/// Updates the currently selected input device label of a player according to the InputScheme.
		/// </summary>
		/// <param name="player">The player to update selected input device label for.</param>
		void UpdatePlayerSelectedDeviceLabel(int player);

		/// <summary>
		/// Enables the input sensitivity controls for a player if applicable to the selected input device.
		/// </summary>
		/// <param name="player">The player to enable input sensitivity controls for.</param>
		void ShowPlayerInputDeviceSensitivityControls(int player);

		/// <summary>
		/// Updates the input sensitivity controls of a player according to the InputScheme.
		/// </summary>
		/// <param name="player">The player to update input sensitivity control values for.</param>
		void UpdatePlayerInputSensitivityControlValues(int player);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputGUI(const SettingsInputGUI &reference) = delete;
		SettingsInputGUI & operator=(const SettingsInputGUI &rhs) = delete;
	};
}
#endif