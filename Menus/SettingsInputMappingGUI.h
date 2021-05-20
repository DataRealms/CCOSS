#ifndef _RTESETTINGSINPUTMAPPINGGUI_
#define _RTESETTINGSINPUTMAPPINGGUI_

#include "SettingsInputMappingWizardGUI.h"

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIScrollbar;
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

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether this input mapping settings menu is currently visible and enabled.
		/// </summary>
		bool IsEnabled() const;

		/// <summary>
		/// Enables or disables the input mapping settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the input mapping settings menu.</param>
		/// <param name="player">The player this SettingsInputMappingGUI is configuring input mapping for.</param>
		void SetEnabled(bool enable = true, int player = 0);

		/// <summary>
		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// </summary>
		/// <returns>Pointer to the GUICollectionBox that is the currently active dialog box, if any.</returns>
		GUICollectionBox * GetActiveDialogBox() const;

		/// <summary>
		/// Gets the SettingsInputMappingWizardGUI of this SettingsInputMappingGUI.
		/// </summary>
		/// <returns>Pointer to the SettingsInputMappingWizardGUI of this SettingsInputMappingGUI.</returns>
		SettingsInputMappingWizardGUI * GetInputConfigWizardMenu() { return m_InputConfigWizardMenu.get(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Handles the player interaction with the SettingsInputMappingGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingGUI is configuring input mapping for.
		InputScheme *m_ConfiguringPlayerInputScheme; //!< The InputScheme of the configuring player.

		int m_LastInputMapScrollingBoxScrollbarValue; //!< The previous value of the input mappings scrolling box scrollbar. Used to calculate the scroll position.

		std::unique_ptr<SettingsInputMappingWizardGUI> m_InputConfigWizardMenu; //!< The input mapping config wizard.

		/// <summary>
		/// GUI elements that compose the input mapping settings menu screen.
		/// </summary>
		GUICollectionBox *m_InputMappingSettingsBox;
		GUILabel *m_InputMappingSettingsLabel;
		GUIButton *m_CloseMappingBoxButton;
		GUIButton *m_RunConfigWizardButton;
		GUICollectionBox *m_InputMapScrollingBox;
		GUIScrollbar *m_InputMapScrollingBoxScrollbar;
		std::array<GUILabel *, InputElements::INPUT_COUNT> m_InputMapLabel;
		std::array<GUIButton *, InputElements::INPUT_COUNT> m_InputMapButton;

#pragma region Input Mapping Settings Handling
		/// <summary>
		/// Updates the mapping button key labels with the configuring player's InputScheme mappings.
		/// </summary>
		void UpdateMappingButtonLabels();

		/// <summary>
		/// Updates the input mapping scrolling box scroll position.
		/// </summary>
		void UpdateScrollingInputBoxScrollPosition();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputMappingGUI(const SettingsInputMappingGUI &reference) = delete;
		SettingsInputMappingGUI & operator=(const SettingsInputMappingGUI &rhs) = delete;
	};
}
#endif