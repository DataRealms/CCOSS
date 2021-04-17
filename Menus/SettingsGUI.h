#ifndef _RTESETTINGSGUI_
#define _RTESETTINGSGUI_

#include "Constants.h"

struct BITMAP;

namespace RTE {

	class Controller;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class GUIComboBox;
	class GUICheckbox;
	class GUICollectionBox;
	class GUIControlManager;
	class AllegroScreen;
	class AllegroInput;

	/// <summary>
	/// 
	/// </summary>
	class SettingsGUI {

	public:

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIInput interface that will be used by this SettingsGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIScreen interface that will be used by this SettingsGUI's GUIControlManager.</param>
		/// <param name="controller"></param>
		SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// 
		/// </summary>
		void SetEnabled();
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// 
		/// </summary>
		void Draw();
#pragma endregion

	private:

#pragma region Video Settings Menu
		/// <summary>
		/// 
		/// </summary>
		struct VideoSettingsMenu {

			/// <summary>
			/// 
			/// </summary>
			struct ResolutionRecord {
				int Width; //!<
				int Height; //!<
				bool Upscaled; //!<

				/// <summary>
				/// Makes UI displayable string with resolution info.
				/// </summary>
				/// <returns>String with resolution info.</returns>
				std::string MakeResolutionString();

				/// <summary>
				/// 
				/// </summary>
				/// <param name="rhs"></param>
				/// <returns></returns>
				bool operator<(const ResolutionRecord &rhs) const { return Width < rhs.Width; }
			};

			std::vector<ResolutionRecord> ValidResolutions; //!<
			GUIComboBox *ResolutionComboBox; //!<
			GUIButton *FullscreenOrWindowedButton; //!<
			GUIButton *UpscaledFullscreenButton; //!<

			GUICollectionBox *ResolutionChangeDialogBox; //!<
			GUIButton *ConfirmResolutionChangeButton; //!<
			GUIButton *ConfirmResolutionChangeFullscreenButton; //!<
			GUIButton *CancelResolutionChangeButton; //!<

			bool m_ResolutionChangeToUpscaled; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager"></param>
			void Create(GUIControlManager *parentControlManager);

			/// <summary>
			/// Updates the contents of the screen resolution combo box.
			/// </summary>
			void PopulateResolutionsComboBox();
		};
#pragma endregion

#pragma region Audio Settings Menu
		/// <summary>
		/// 
		/// </summary>
		struct AudioSettingsMenu {
			GUILabel *MusicLabel; //!<
			GUISlider *MusicSlider; //!<
			GUILabel *SoundLabel; //!<
			GUISlider *SoundSlider; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager"></param>
			void Create(GUIControlManager *parentControlManager);

			/// <summary>
			/// Updates the position of the volume sliders, based on what the AudioMan is currently set to.
			/// </summary>
			//void UpdateVolumeSliders();
		};
#pragma endregion

#pragma region Input Settings Menu
		/// <summary>
		/// 
		/// </summary>
		struct InputSettingsMenu {

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

			/// <summary>
			/// 
			/// </summary>
			struct ControlConfigWizard {

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

			ControlConfigWizard ControlConfigWizardMenu; //!<

			std::array<PlayerInputSettingsBox, Players::MaxPlayerCount> PlayerInputSettingsBoxes; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager"></param>
			void Create(GUIControlManager *parentControlManager);

			/// <summary>
			/// Updates the text on the configuration labels, based on actual UInputMan settings.
			/// </summary>
			//void UpdateDeviceLabels();
		};
#pragma endregion

#pragma region Gameplay Settings Menu
		/// <summary>
		/// 
		/// </summary>
		struct GameplaySettingsMenu {
			GUICheckbox *FlashOnBrainDamageCheckbox; //!<
			GUICheckbox *BlipOnRevealUnseenCheckbox; //!<
			GUICheckbox *ShowForeignItemsCheckbox; //!<
			GUICheckbox *ShowToolTipsCheckbox; //!<

			/// <summary>
			/// 
			/// </summary>
			/// <param name="parentControlManager"></param>
			void Create(GUIControlManager *parentControlManager);
		};
#pragma endregion

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!<
		Controller *m_Controller; //!<

		VideoSettingsMenu m_VideoSettingsMenu; //!<
		AudioSettingsMenu m_AudioSettingsMenu; //!<
		InputSettingsMenu m_InputSettingsMenu; //!<
		GameplaySettingsMenu m_GameplaySettingsMenu; //!<

		// Disallow the use of some implicit methods.
		SettingsGUI(const SettingsGUI &reference) = delete;
		SettingsGUI & operator=(const SettingsGUI &rhs) = delete;
	};
}
#endif