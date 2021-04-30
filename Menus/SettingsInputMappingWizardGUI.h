#ifndef _RTESETTINGSINPUTMAPPINGWIZARDGUI_
#define _RTESETTINGSINPUTMAPPINGWIZARDGUI_

#include "Constants.h"

struct BITMAP;

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIEvent;

	/// <summary>
	/// 
	/// </summary>
	class SettingsInputMappingWizardGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsInputMappingWizardGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsInputMappingWizardGUI.</param>
		explicit SettingsInputMappingWizardGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Input Mapping Wizard menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Input Mapping Wizard menu.</param>
		/// <param name="player"></param>
		void SetEnabled(bool enable = true, int player = 0);

		/// <summary>
		/// User input handling for the Input Settings menu.
		/// </summary>
		/// <param name="guiEvent"></param>
		/// <param name="player"></param>
		void HandleInputEvents(GUIEvent &guiEvent, int player);
#pragma endregion

	private:

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

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_InputMappingWizardBox; //!<

		GUIButton *m_BackToOptionsButton; //!< Back to options from the test and config screens.

		Players m_ConfiguringPlayer; //!< Which player's control scheme we are currently configuring.
		InputDevice m_ConfiguringDevice; //!< Which type of device we are currently configuring.
		GamepadType m_ConfiguringGamepad; //!< Which type of gamepad we are currently configuring.
		int m_ConfigureStep; //!< Which step in current configure sequence.

		std::array<GUILabel *, ConfigWizardLabels::ConfigLabelCount> m_ConfigLabel; //!< Labels of the control config screen.

		std::array<BITMAP *, ConfigWizardSteps::DPadConfigSteps> m_DPadBitmaps;
		std::array<BITMAP *, ConfigWizardSteps::DualAnalogConfigSteps> m_DualAnalogBitmaps;

		GUICollectionBox *m_RecommendationBox; //!<
		GUICollectionBox *m_RecommendationDiagram; //!<

		GUIButton *m_ConfigSkipButton; //!< Skip forward one config step button.
		GUIButton *m_ConfigBackButton; //!< Go back one config step button.

		GUICollectionBox *m_DPadTypeBox; //!<
		GUICollectionBox *m_DPadTypeDiagram; //!<
		GUIButton *m_DPadTypeButton; //!<

		GUICollectionBox *m_DAnalogTypeBox; //!<
		GUICollectionBox *m_DAnalogTypeDiagram; //!<
		GUIButton *m_DAnalogTypeButton; //!<

		GUICollectionBox *m_XBox360TypeBox; //!<
		GUICollectionBox *m_XBox360TypeDiagram; //!<
		GUIButton *m_XBox360TypeButton; //!<

#pragma region Input Mapping Wizard Handling
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

		// Disallow the use of some implicit methods.
		SettingsInputMappingWizardGUI(const SettingsInputMappingWizardGUI &reference) = delete;
		SettingsInputMappingWizardGUI & operator=(const SettingsInputMappingWizardGUI &rhs) = delete;
	};
}
#endif