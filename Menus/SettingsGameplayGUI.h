#ifndef _RTESETTINGSGAMEPLAYGUI_
#define _RTESETTINGSGAMEPLAYGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUICheckbox;
	class GUITextBox;
	class GUISlider;
	class GUILabel;
	class GUIEvent;

	/// <summary>
	/// Handling for gameplay settings through the game settings user interface.
	/// </summary>
	class SettingsGameplayGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsGameplayGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsGameplayGUI. Ownership is NOT transferred!</param>
		explicit SettingsGameplayGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the SettingsGameplayGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsGameplayGUI.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Handles the player interaction with the SettingsInputGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		/// <summary>
		/// GUI elements that compose the gameplay settings menu screen.
		/// </summary>
		GUICollectionBox *m_GameplaySettingsBox;
		GUICheckbox *m_FlashOnBrainDamageCheckbox;
		GUICheckbox *m_BlipOnRevealUnseenCheckbox;
		GUICheckbox *m_ShowForeignItemsCheckbox;
		GUICheckbox *m_EnableCrabBombsCheckbox;
		GUICheckbox *m_EndlessMetaGameCheckbox;
		GUICheckbox *m_ShowEnemyHUDCheckbox;
		GUICheckbox *m_EnableSmartBuyMenuNavigationCheckbox;
		GUITextBox *m_MaxUnheldItemsTextbox;
		GUITextBox *m_CrabBombThresholdTextbox;
		GUISlider *m_UnheldItemsHUDDisplayRangeSlider;
		GUILabel *m_UnheldItemsHUDDisplayRangeLabel;

#pragma region Gameplay Settings Handling
		/// <summary>
		/// Updates the MaxUnheldItems textbox to override any invalid input, applies the setting value and removes its focus.
		/// </summary>
		void UpdateMaxUnheldItemsTextbox();

		/// <summary>
		/// Updates the CrabBombThreshold textbox to override any invalid input, applies the setting value and removes its focus.
		/// </summary>
		void UpdateCrabBombThresholdTextbox();

		/// <summary>
		/// Updates the UnheldItemsHUDDisplayRange setting and label according to the slider value.
		/// </summary>
		void UpdateUnheldItemsHUDDisplayRange();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsGameplayGUI(const SettingsGameplayGUI &reference) = delete;
		SettingsGameplayGUI & operator=(const SettingsGameplayGUI &rhs) = delete;
	};
}
#endif