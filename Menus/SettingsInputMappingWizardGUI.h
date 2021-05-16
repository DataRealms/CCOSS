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
		/// Enables or disables the input mapping wizard menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the input mapping wizard menu.</param>
		/// <param name="player">The player this SettingsInputMappingWizardGUI is mapping inputs for.</param>
		void SetEnabled(bool enable = true, int player = 0);

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
		enum class GamepadType { DPad, DualAnalog, Xbox360 };

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< Which player's control scheme we are currently configuring.
		InputDevice m_ConfiguringDevice; //!< Which type of device we are currently configuring.
		GamepadType m_ConfiguringGamepad; //!< Which type of gamepad we are currently configuring.
		int m_ConfigureStep; //!< Which step in current configure sequence.

		std::array<BITMAP *, ConfigWizardSteps::DPadConfigSteps> m_DPadBitmaps;
		std::array<BITMAP *, ConfigWizardSteps::DualAnalogConfigSteps> m_DualAnalogBitmaps;

		/// <summary>
		/// GUI elements that compose the input mapping wizard menu screen.
		/// </summary>
		GUICollectionBox *m_InputMappingWizardBox;
		GUIButton *m_BackToOptionsButton;
		GUICollectionBox *m_RecommendationBox;
		GUICollectionBox *m_RecommendationDiagram;
		GUIButton *m_ConfigSkipButton;
		GUIButton *m_ConfigBackButton;
		GUICollectionBox *m_DPadTypeBox;
		GUICollectionBox *m_DPadTypeDiagram;
		GUIButton *m_DPadTypeButton;
		GUICollectionBox *m_DAnalogTypeBox;
		GUICollectionBox *m_DAnalogTypeDiagram;
		GUIButton *m_DAnalogTypeButton;
		GUICollectionBox *m_XBox360TypeBox;
		GUICollectionBox *m_XBox360TypeDiagram;
		GUIButton *m_XBox360TypeButton;
		std::array<GUILabel *, ConfigWizardLabels::ConfigLabelCount> m_ConfigLabel;

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