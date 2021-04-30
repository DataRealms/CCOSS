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
	/// 
	/// </summary>
	class SettingsInputMappingGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsInputMappingGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsInputMappingGUI.</param>
		explicit SettingsInputMappingGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Input Mapping Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Input Mapping Settings menu.</param>
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
		enum PlayerInputMappings {
			InputMappingCount = 18
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_InputMappingSettingsBox; //!<

		GUICollectionBox *m_MappingConfigBox;
		GUILabel *m_MappingConfigLabel;
		GUIButton *m_CloseMappingBoxButton;
		GUIButton *m_RunConfigWizardButton;

		std::array<GUILabel *, PlayerInputMappings::InputMappingCount> m_InputMapLabel; //!<
		std::array<GUIButton *, PlayerInputMappings::InputMappingCount> m_InputMapButton; //!<

		bool m_InputConfigWizardActive = false; //!<

		std::unique_ptr<SettingsInputMappingWizardGUI> m_InputConfigWizardMenu; //!<

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