#ifndef _RTESETTINGSINPUTMAPPINGGUI_
#define _RTESETTINGSINPUTMAPPINGGUI_

#include "SettingsInputMappingWizardGUI.h"

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIEvent;

	/// <summary>
	/// Handling for player input mapping settings through the game settings user interface.
	/// </summary>
	class SettingsInputMappingGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsInputMappingGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputMappingGUI.</param>
		explicit SettingsInputMappingGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the input mapping settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the input mapping settings menu.</param>
		/// <param name="player">The player this SettingsInputMappingGUI is configuring input mapping for.</param>
		void SetEnabled(bool enable = true, int player = 0);

		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum PlayerInputMappings {
			InputMappingCount = 22
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingGUI is configuring input mapping for.

		std::unique_ptr<SettingsInputMappingWizardGUI> m_InputConfigWizardMenu; //!< The input mapping config wizard.

		/// <summary>
		/// GUI elements that compose the input mapping settings menu screen.
		/// </summary>
		GUICollectionBox *m_InputMappingSettingsBox;
		GUILabel *m_InputMappingSettingsLabel;
		GUIButton *m_CloseMappingBoxButton;
		GUIButton *m_RunConfigWizardButton;
		std::array<GUILabel *, PlayerInputMappings::InputMappingCount> m_InputMapLabel;
		std::array<GUIButton *, PlayerInputMappings::InputMappingCount> m_InputMapButton;

#pragma region Input Mapping Settings Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="player"></param>
		void UpdateMappingLabelsAndButtons(int player);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputMappingGUI(const SettingsInputMappingGUI &reference) = delete;
		SettingsInputMappingGUI & operator=(const SettingsInputMappingGUI &rhs) = delete;
	};
}
#endif