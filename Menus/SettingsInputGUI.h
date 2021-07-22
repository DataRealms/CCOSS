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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputGUI. Ownership is NOT transferred!</param>
		explicit SettingsInputGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Enables or disables the SettingsInputGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsInputGUI.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// </summary>
		/// <returns>Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!</returns>
		GUICollectionBox * GetActiveDialogBox() const { return m_InputMappingConfigMenu->GetActiveDialogBox(); }

		/// <summary>
		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		/// </summary>
		void CloseActiveDialogBox() const { m_InputMappingConfigMenu->CloseActiveDialogBox(); }
#pragma endregion

#pragma region Input Config Wizard Handling
		/// <summary>
		/// Gets whether the player is currently manually configuring an InputMapping through the input mapping menu screen.
		/// </summary>
		/// <returns>Whether the player is currently manually configuring an InputMapping through the input mapping menu screen.</returns>
		bool InputMappingConfigIsConfiguringManually() const { return m_InputMappingConfigMenu->IsConfiguringManually(); }

		/// <summary>
		/// Handles input capture logic of the input mapping menu screen manual configuration sequence.
		/// </summary>
		void HandleMappingConfigManualConfiguration() const { m_InputMappingConfigMenu->HandleManualConfigSequence(); }

		/// <summary>
		/// Gets whether the player is currently manually configuring the InputScheme through the input mapping wizard.
		/// </summary>
		/// <returns>Whether the player is currently manually configuring the InputScheme through the input mapping wizard.</returns>
		bool InputConfigWizardIsConfiguringManually() const { return m_InputMappingConfigMenu->GetInputConfigWizardMenu()->IsConfiguringManually(); }

		/// <summary>
		/// Handles input capture logic of the input mapping wizard manual configuration sequence.
		/// </summary>
		void HandleConfigWizardManualConfiguration() const { m_InputMappingConfigMenu->GetInputConfigWizardMenu()->HandleManualConfigSequence(); }
#pragma endregion

#pragma region Concrete Methods
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
			GUILabel *SensitivityLabel;
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
		void ShowOrHidePlayerInputDeviceSensitivityControls(int player);

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