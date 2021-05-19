#ifndef _RTESETTINGSINPUTMAPPINGWIZARDGUI_
#define _RTESETTINGSINPUTMAPPINGWIZARDGUI_

#include "InputScheme.h"

struct BITMAP;

namespace RTE {

	class InputScheme;
	class GUIControlManager;
	class GUICollectionBox;
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

#pragma region Getters and Setters
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
		/// Gets whether the input mapping wizard needs to capture input for manual configuration.
		/// </summary>
		/// <returns>Whether the input mapping wizard needs to capture input for manual configuration.</returns>
		bool IsConfiguringManually() const { return m_ConfiguringManually; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		/// <returns>Whether this SettingsInputMappingGUI changed the input scheme of the configuring player.</returns>
		bool HandleInputEvents(GUIEvent &guiEvent);

		/// <summary>
		/// Handles updating and progressing the manual input configuration sequence.
		/// </summary>
		void HandleManualConfigSequence();
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum ConfigWizardSteps {
			NoConfigSteps = 0,
			KeyboardConfigSteps = 16,
			MouseAndKeyboardConfigSteps = 11,
			DPadConfigSteps = 13,
			DualAnalogConfigSteps = 19,
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
			GUILabel *ConfigDeviceTypeLabel;
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

		bool m_ConfiguringManually; //!<
		bool m_ConfigFinished; //!<
		int m_ConfigStep; //!< Which step in current configure sequence.
		bool m_ConfigStepChange; //!<

		InputScheme m_NewInputScheme; //!<
		bool m_NewInputSchemeApplied; //!< Whether the new InputScheme was applied as the configuring player's active InputScheme.

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
		void ApplyManuallyConfiguredScheme();

		/// <summary>
		/// 
		/// </summary>
		void ApplyGamepadInputPreset(GamepadType gamepadType);


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

#pragma region Input Configuration Sequence Handling Breakdown
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateKeyboardConfigSequence();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateMouseAndKeyboardConfigSequence();

		/// <summary>
		/// 
		/// </summary>
		bool UpdateGamepadDPadConfigSequence();

		/// <summary>
		/// 
		/// </summary>
		bool UpdateGamepadAnalogConfigSequence();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateGamepadConfigSequence();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this SettingsInputMappingWizardGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SettingsInputMappingWizardGUI(const SettingsInputMappingWizardGUI &reference) = delete;
		SettingsInputMappingWizardGUI & operator=(const SettingsInputMappingWizardGUI &rhs) = delete;
	};
}
#endif