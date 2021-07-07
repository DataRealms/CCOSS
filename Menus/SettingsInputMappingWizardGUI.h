#ifndef _RTESETTINGSINPUTMAPPINGWIZARDGUI_
#define _RTESETTINGSINPUTMAPPINGWIZARDGUI_

#include "Timer.h"
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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputMappingWizardGUI. Ownership is NOT transferred!</param>
		explicit SettingsInputMappingWizardGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether this SettingsInputMappingWizardGUI is currently visible and enabled.
		/// </summary>
		/// <returns>Whether this SettingsInputMappingWizardGUI is currently visible and enabled.</returns>
		bool IsEnabled() const;

		/// <summary>
		/// Enables or disables the SettingsInputMappingWizardGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsInputMappingWizardGUI.</param>
		/// <param name="player">The player this SettingsInputMappingWizardGUI is mapping inputs for.</param>
		void SetEnabled(bool enable = true, int player = 0, InputScheme *playerScheme = nullptr);

		/// <summary>
		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// </summary>
		/// <returns>Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!</returns>
		GUICollectionBox * GetActiveDialogBox() const;

		/// <summary>
		/// Gets whether this SettingsInputMappingWizardGUI needs to capture input for manual configuration.
		/// </summary>
		/// <returns>Whether this SettingsInputMappingWizardGUI needs to capture input for manual configuration.</returns>
		bool IsConfiguringManually() const;
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
		/// Enumeration for the different types of gamepads that can be configured.
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

		static constexpr int m_KeyboardConfigSteps = 16; //!< The step count for keyboard only manual configuration.
		static constexpr int m_MouseAndKeyboardConfigSteps = 11; //!< The step count for mouse + keyboard manual configuration.
		static constexpr int m_DPadConfigSteps = 12; //!< The step count for DPad type gamepad manual configuration.
		static constexpr int m_DualAnalogConfigSteps = 20; //!< The step count for DualAnalog type gamepad manual configuration.

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingWizardGUI is configuring input mapping for.
		InputScheme *m_ConfiguringPlayerScheme; //!< The InputScheme of the configuring player.

		InputDevice m_ConfiguringDevice; //!< Which type of device we are currently configuring.
		bool m_ConfiguringDeviceIsGamepad; //!< Whether the device being configured is a gamepad of any type.
		GamepadType m_ConfiguringGamepadType; //!< Which type of gamepad we are currently configuring.
		int m_ConfiguringGamepadIndex; //!< The index number of the configuring gamepad. See UInputMan::GetJoystickIndex for info.

		bool m_ConfiguringManually; //!< Indicates that the SettingsInputMappingWizardGUI needs to capture input because the player is configuring manually.
		bool m_ConfigFinished; //!< Whether the last input was mapped and the manual configuration mode is ready to apply the new InputScheme.
		int m_ConfigStep; //!< The current step in the manual configuration sequence.
		bool m_ConfigStepChange; //!< Whether a configuration step was completed and the sequence needs updating to start handling the next step.

		InputScheme m_NewInputScheme; //!< The new InputScheme that was configured in manual configuration mode.
		bool m_NewInputSchemeApplied; //!< Whether the new InputScheme was applied as the configuring player's active InputScheme.

		Timer m_BlinkTimer; //!< Timer for blinking the "Apply Changes" button and animating the recommended input diagram when configuring gamepads.

		std::vector<BITMAP *> m_DPadDiagramBitmaps; //!< Vector containing all the D-Pad type gamepad recommended input diagram bitmaps.
		std::vector<BITMAP *> m_DualAnalogDSDiagramBitmaps; //!< Vector containing all the DualShock type gamepad recommended input diagram bitmaps.
		std::vector<BITMAP *> m_DualAnalogXBDiagramBitmaps; //!< Vector containing all the Xbox type gamepad recommended input diagram bitmaps.

		WizardManualConfigScreen m_WizardManualConfigScreen; //!< The manual input configuration menu screen.
		WizardPresetSelectScreen m_WizardPresetSelectScreen; //!< The preset selection menu screen.

		/// <summary>
		/// GUI elements that compose the input mapping wizard menu screen.
		/// </summary>
		GUICollectionBox *m_InputWizardScreenBox;
		GUILabel *m_InputWizardTitleLabel;

#pragma region Create Breakdown
		/// <summary>
		/// Creates all the elements that compose the manual input configuration box.
		/// </summary>
		void CreateManualConfigScreen();

		/// <summary>
		/// Creates all the elements that compose the gamepad input preset selection box.
		/// </summary>
		void CreatePresetSelectionScreen();
#pragma endregion

#pragma region Input Mapping Wizard Handling
		/// <summary>
		/// Makes the manual input configuration menu screen visible to be interacted with by the player.
		/// </summary>
		void ShowManualConfigScreen();

		/// <summary>
		/// Makes the gamepad input preset selection menu screen visible to be interacted with by the player.
		/// </summary>
		void ShowPresetSelectionScreen();

		/// <summary>
		/// Clears the InputScheme that was configured during manual configuration and resets the sequence to the first step.
		/// </summary>
		void ResetManualConfigSequence();

		/// <summary>
		/// Applies the manually configured InputScheme as the active InputScheme of the configuring player.
		/// </summary>
		void ApplyManuallyConfiguredScheme();

		/// <summary>
		/// Applies a gamepad InputScheme preset as the active InputScheme of the configuring player.
		/// </summary>
		void ApplyGamepadInputPreset(GamepadType gamepadType);
#pragma endregion

#pragma region Input Event Handling Breakdown
		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardManualConfigScreen GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleManualConfigScreenInputEvents(GUIEvent &guiEvent);

		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardPresetSelectScreen GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandlePresetSelectScreenInputEvents(GUIEvent &guiEvent);
#pragma endregion

#pragma region Input Configuration Sequence Handling Breakdown
		/// <summary>
		/// Handles step changes in the manual input configuration sequence.
		/// </summary>
		void HandleManualConfigStepChange();

		/// <summary>
		/// Handles capturing input and progressing the keyboard only configuration sequence.
		/// </summary>
		/// <returns>Whether input was captured and the sequence needs to progress.</returns>
		bool UpdateKeyboardConfigSequence();

		/// <summary>
		/// Handles capturing input and progressing the mouse and keyboard configuration sequence.
		/// </summary>
		/// <returns>Whether input was captured and the sequence needs to progress.</returns>
		bool UpdateMouseAndKeyboardConfigSequence();

		/// <summary>
		/// Handles capturing input and progressing the D-Pad type gamepad configuration sequence.
		/// </summary>
		/// <returns>Whether input was captured and the sequence needs to progress.</returns>
		bool UpdateGamepadDPadConfigSequence();

		/// <summary>
		/// Handles capturing input and progressing the dual-analog type gamepad (DualShock/Xbox) configuration sequence.
		/// </summary>
		/// <returns>Whether input was captured and the sequence needs to progress.</returns>
		bool UpdateGamepadAnalogConfigSequence();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this SettingsInputMappingWizardGUI, effectively resetting the members of this object.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SettingsInputMappingWizardGUI(const SettingsInputMappingWizardGUI &reference) = delete;
		SettingsInputMappingWizardGUI & operator=(const SettingsInputMappingWizardGUI &rhs) = delete;
	};
}
#endif