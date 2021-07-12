#ifndef _RTESETTINGSMISCGUI_
#define _RTESETTINGSMISCGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUICheckbox;
	class GUIEvent;

	/// <summary>
	/// Handling for misc settings through the game settings user interface.
	/// </summary>
	class SettingsMiscGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMiscGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsMiscGUI. Ownership is NOT transferred!</param>
		explicit SettingsMiscGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the SettingsMiscGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsMiscGUI.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// Handles the player interaction with the SettingsMiscGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		/// <summary>
		/// GUI elements that compose the misc settings menu screen.
		/// </summary>
		GUICollectionBox *m_MiscSettingsBox;
		GUICheckbox *m_SkipIntroCheckbox;
		GUICheckbox *m_ShowToolTipsCheckbox;
		GUICheckbox *m_ShowLoadingScreenProgressReportCheckbox;
		GUICheckbox *m_ShowAdvancedPerfStatsCheckbox;
		GUICheckbox *m_MeasureLoadTimeCheckbox;
		GUICheckbox *m_UseMonospaceConsoleFontCheckbox;

		// Disallow the use of some implicit methods.
		SettingsMiscGUI(const SettingsMiscGUI &reference) = delete;
		SettingsMiscGUI & operator=(const SettingsMiscGUI &rhs) = delete;
	};
}
#endif