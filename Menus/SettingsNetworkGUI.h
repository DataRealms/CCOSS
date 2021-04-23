#ifndef _RTESETTINGSNETWORKGUI_
#define _RTESETTINGSNETWORKGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIEvent;

	/// <summary>
	/// Handling for network settings through the game settings user interface.
	/// </summary>
	class SettingsNetworkGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMiscGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsMiscGUI.</param>
		explicit SettingsNetworkGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Network Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Network Settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// User input handling for the Network Settings menu.
		/// </summary>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_NetworkSettingsBox; //!< The GUICollectionBox that contains all the Network Settings menu GUI elements.

		// Disallow the use of some implicit methods.
		SettingsNetworkGUI(const SettingsNetworkGUI &reference) = delete;
		SettingsNetworkGUI & operator=(const SettingsNetworkGUI &rhs) = delete;
	};
}
#endif