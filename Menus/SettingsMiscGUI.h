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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsMiscGUI.</param>
		explicit SettingsMiscGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Misc Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Misc Settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// User input handling for the Audio Settings menu.
		/// </summary>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_MiscSettingsBox; //!< The GUICollectionBox that contains all the Misc Settings menu GUI elements.

		GUICheckbox *m_ShowToolTipsCheckbox; //!<

		// Disallow the use of some implicit methods.
		SettingsMiscGUI(const SettingsMiscGUI &reference) = delete;
		SettingsMiscGUI & operator=(const SettingsMiscGUI &rhs) = delete;
	};
}
#endif