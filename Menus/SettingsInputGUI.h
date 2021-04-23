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
	class GUICheckbox;
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
		/// <param name="controller"></param>
		explicit SettingsInputGUI(GUIControlManager *parentControlManager, Controller *controller);
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
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

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
			/// Updates the contents of the control configuration screen.
			/// </summary>
			void UpdateConfigScreen();

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
			GUILabel *InputDeviceLabel; //!< Label for the input device name that is currently being used.
			GUIButton *NextInputDeviceButton; //!< Button for changing to the next input device type.
			GUIButton *PrevInputDeviceButton; //!< Button for changing to the previous input device type.
			GUIButton *ConfigureControlsButton; //!< Button for starting the controls configuration wizard.
			GUIButton *ClearControlsButton; //!< Button to clear all mapped controls.
			GUILabel *DeadZoneLabel; //!< Label for the controller deadzone value.
			GUISlider *DeadZoneSlider; //!< Slider for setting the controller deadzone value.
			GUICheckbox *DeadZoneTypeCheckbox; //!< Checkbox for setting the controller deadzone type.
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.
		Controller *m_Controller;

		GUICollectionBox *m_InputSettingsBox; //!<

		InputConfigWizard m_InputConfigWizardMenu; //!<

		std::array<PlayerInputSettingsBox, Players::MaxPlayerCount> m_PlayerInputSettingsBoxes; //!<

		/// <summary>
		/// 
		/// </summary>
		void CreateInputConfigWizard(); //!<

		/// <summary>
		/// Updates the text on the configuration labels, based on actual UInputMan settings.
		/// </summary>
		//void UpdateDeviceLabels();

		// Disallow the use of some implicit methods.
		SettingsInputGUI(const SettingsInputGUI &reference) = delete;
		SettingsInputGUI & operator=(const SettingsInputGUI &rhs) = delete;
	};
}
#endif