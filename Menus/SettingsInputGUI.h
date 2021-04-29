#ifndef _RTESETTINGSINPUTGUI_
#define _RTESETTINGSINPUTGUI_

#include "Constants.h"

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
		struct InputConfigScreen {

			/// <summary>
			/// 
			/// </summary>
			enum InputMapLabels {
				InputMapLabelCount
			};

			/// <summary>
			/// 
			/// </summary>
			enum InputMapButtons {
				InputMapButtonCount
			};

			bool Enabled = false; //!<
			bool InputConfigWizardActive = false; //!<

			std::array<GUILabel *, InputMapLabels::InputMapLabelCount> InputMapLabel; //!<
			std::array<GUIButton *, InputMapButtons::InputMapButtonCount> InputMapButton; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsInputGUI.</param>
			void Create(GUIControlManager *parentControlManager);

			/// <summary>
			/// 
			/// </summary>
			/// <param name="guiEvent"></param>
			/// <param name="player"></param>
			void HandleInputEvents(GUIEvent &guiEvent, int player);
		};

		/// <summary>
		/// 
		/// </summary>
		struct InputConfigWizard {

			/// <summary>
			/// 
			/// </summary>
			enum ConfigWizardLabels {
				ConfigTitle,
				ConfigRecommendation,
				ConfigInstruction,
				ConfigInput,
				ConfigSteps,
				ConfigLabelCount
			};

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
			enum class GamepadType { DPad, DualAnalog, Xbox360 };

			bool Enabled = false; //!<

			GUIButton *BackToOptionsButton; //!< Back to options from the test and config screens.

			Players ConfiguringPlayer; //!< Which player's control scheme we are currently configuring.
			InputDevice ConfiguringDevice; //!< Which type of device we are currently configuring.
			GamepadType ConfiguringGamepad; //!< Which type of gamepad we are currently configuring.
			int ConfigureStep; //!< Which step in current configure sequence.

			std::array<GUILabel *, ConfigWizardLabels::ConfigLabelCount> ConfigLabel; //!< Labels of the control config screen.

			std::array<BITMAP *, ConfigWizardSteps::DPadConfigSteps> DPadBitmaps;
			std::array<BITMAP *, ConfigWizardSteps::DualAnalogConfigSteps> DualAnalogBitmaps;

			GUICollectionBox *RecommendationBox; //!<
			GUICollectionBox *RecommendationDiagram; //!<

			GUIButton *ConfigSkipButton; //!< Skip forward one config step button.
			GUIButton *ConfigBackButton; //!< Go back one config step button.

			GUICollectionBox *DPadTypeBox; //!<
			GUICollectionBox *DPadTypeDiagram; //!<
			GUIButton *DPadTypeButton; //!<

			GUICollectionBox *DAnalogTypeBox; //!<
			GUICollectionBox *DAnalogTypeDiagram; //!<
			GUIButton *DAnalogTypeButton; //!<

			GUICollectionBox *XBox360TypeBox; //!<
			GUICollectionBox *XBox360TypeDiagram; //!<
			GUIButton *XBox360TypeButton; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsInputGUI.</param>
			void Create(GUIControlManager *parentControlManager);

			/// <summary>
			/// 
			/// </summary>
			/// <param name="guiEvent"></param>
			/// <param name="player"></param>
			void HandleInputEvents(GUIEvent &guiEvent, int player);

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
		};

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

		InputConfigScreen m_InputConfigMenu;
		InputConfigWizard m_InputConfigWizardMenu; //!<

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