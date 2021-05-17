#ifndef _RTESETTINGSINPUTMAPPINGWIZARDGUI_
#define _RTESETTINGSINPUTMAPPINGWIZARDGUI_

#include "Constants.h"

struct BITMAP;

namespace RTE {

	class InputScheme;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIEvent;

	/// <summary>
	/// Handling for the user input mapping wizard through the game settings user interface.
	/// </summary>
	class SettingsInputMappingWizardGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsInputMappingWizardGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputMappingWizardGUI.</param>
		explicit SettingsInputMappingWizardGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Gets whether this input mapping wizard menu is currently visible and enabled.
		/// </summary>
		bool IsEnabled() const;

		/// <summary>
		/// Enables or disables the input mapping wizard menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the input mapping wizard menu.</param>
		/// <param name="player">The player this SettingsInputMappingWizardGUI is mapping inputs for.</param>
		void SetEnabled(bool enable = true, int player = 0, InputScheme *playerScheme = nullptr);

		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum ConfigWizardSteps {
			KeyboardConfigSteps = 16,
			MouseAndKeyboardConfigSteps = 11,
			DPadConfigSteps = 13,
			DualAnalogConfigSteps = 19,
			Xbox360ConfigSteps = 19
		};

		/// <summary>
		/// 
		/// </summary>
		enum GamepadType { DPad, AnalogDualShock, AnalogXbox };

		/// <summary>
		/// Struct containing GUI elements that compose the input mapping wizard manual configuration menu screen.
		/// </summary>
		struct WizardManualConfigScreen {
			GUICollectionBox *ManualConfigBox;
			GUILabel *ConfigStepDescriptionLabel;
			GUILabel *ConfigStepRecommendedKeyLabel;
			GUICollectionBox *GamepadConfigRecommendedBox;
			GUILabel *GamepadConfigStepRecommendedInputLabel;
			GUICollectionBox *GamepadConfigRecommendedDiagramBox;
			GUILabel *ConfigStepLabel;
			GUIButton *PrevConfigStepButton;
			GUIButton *NextConfigStepButton;
			GUIButton *ResetConfigButton;
			GUIButton *DiscardOrApplyConfigButton;
		};

		/// <summary>
		/// Struct containing GUI elements that compose the input mapping wizard preset selection menu screen.
		/// </summary>
		struct WizardPresetSelectScreen {
			GUICollectionBox *PresetSelectBox;
			GUIButton *CloseWizardButton;
			GUIButton *PresetSelectSNESButton;
			GUIButton *PresetSelectDS4Button;
			GUIButton *PresetSelectXB360Button;
			GUIButton *StartConfigDPadTypeButton;
			GUIButton *StartConfigAnalogDSTypeButton;
			GUIButton *StartConfigAnalogXBTypeButton;
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingWizardGUI is configuring input mapping for.
		InputScheme *m_ConfiguringPlayerScheme; //!< The InputScheme of the configuring player.

		InputDevice m_ConfiguringDevice; //!< Which type of device we are currently configuring.
		bool m_ConfiguringDeviceIsGamepad; //!< Whether the device being configured is a gamepad of any type.
		GamepadType m_ConfiguringGamepadType; //!< Which type of gamepad we are currently configuring.
		int m_ConfigureStep; //!< Which step in current configure sequence.

		std::vector<BITMAP *> m_DPadDiagramBitmaps; //!<
		std::vector<BITMAP *> m_DualAnalogDSDiagramBitmaps; //!<
		std::vector<BITMAP *> m_DualAnalogXBDiagramBitmaps; //!<

		WizardManualConfigScreen m_WizardManualConfigScreen; //!<
		WizardPresetSelectScreen m_WizardPresetSelectScreen; //!<

		/// <summary>
		/// GUI elements that compose the input mapping wizard menu screen.
		/// </summary>
		GUICollectionBox *m_InputWizardScreenBox;
		GUILabel *m_InputWizardTitleLabel;

#pragma region Create Breakdown
		/// <summary>
		/// 
		/// </summary>
		void CreateManualConfigScreen();

		/// <summary>
		/// 
		/// </summary>
		void CreatePresetSelectionScreen();
#pragma endregion

#pragma region Input Mapping Wizard Handling
		/// <summary>
		/// 
		/// </summary>
		void ShowManualConfigScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowPresetSelectionScreen();
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateKeyboardConfigWizard();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateMouseAndKeyboardConfigWizard();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateGamepadConfigWizard();
#pragma endregion

#pragma region Input Event Handling Breakdown
		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardManualConfigScreen GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleManualConfigScreenInputEvenets(GUIEvent &guiEvent);

		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardPresetSelectScreen GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandlePresetSelectScreenInputEvents(GUIEvent &guiEvent);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputMappingWizardGUI(const SettingsInputMappingWizardGUI &reference) = delete;
		SettingsInputMappingWizardGUI & operator=(const SettingsInputMappingWizardGUI &rhs) = delete;
	};
}
#endif