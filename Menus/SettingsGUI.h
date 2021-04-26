#ifndef _RTESETTINGSGUI_
#define _RTESETTINGSGUI_

#include "SettingsVideoGUI.h"
#include "SettingsAudioGUI.h"
#include "SettingsInputGUI.h"
#include "SettingsGameplayGUI.h"
#include "SettingsNetworkGUI.h"
#include "SettingsMiscGUI.h"

namespace RTE {

	class Controller;
	class GUIButton;
	class GUITab;
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
		/// Constructor method used to instantiate a SettingsGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIInput interface that will be used by this SettingsGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIScreen interface that will be used by this SettingsGUI's GUIControlManager.</param>
		SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// 
		/// </summary>
		void Draw() const;
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum ActiveSettingsMenu {
			VideoSettingsMenu,
			AudioSettingsMenu,
			InputSettingsMenu,
			GameplaySettingsMenu,
			NetworkSettingsMenu,
			MiscSettingsMenu,
			SettingsMenuCount
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!<

		GUICollectionBox *m_SettingsTabberBox;
		std::array<GUITab *, ActiveSettingsMenu::SettingsMenuCount> m_SettingsMenuTabs;

		ActiveSettingsMenu m_ActiveSettingsMenu;

		GUIButton *m_BackToMainButton;

		std::unique_ptr<SettingsVideoGUI> m_VideoSettingsMenu; //!<
		std::unique_ptr<SettingsAudioGUI> m_AudioSettingsMenu; //!<
		std::unique_ptr<SettingsInputGUI> m_InputSettingsMenu; //!<
		std::unique_ptr<SettingsGameplayGUI> m_GameplaySettingsMenu; //!<
		std::unique_ptr<SettingsNetworkGUI> m_NetworkSettingsMenu; //!<
		std::unique_ptr<SettingsMiscGUI> m_MiscSettingsMenu; //!<

		/// <summary>
		/// 
		/// </summary>
		/// <param name="activeMenu"></param>
		void SetActiveSettingsMenu(ActiveSettingsMenu activeMenu);

		// Disallow the use of some implicit methods.
		SettingsGUI(const SettingsGUI &reference) = delete;
		SettingsGUI & operator=(const SettingsGUI &rhs) = delete;
	};
}
#endif