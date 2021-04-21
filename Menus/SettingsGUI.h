#ifndef _RTESETTINGSGUI_
#define _RTESETTINGSGUI_

#include "SettingsVideoGUI.h"
#include "SettingsAudioGUI.h"
#include "SettingsInputGUI.h"
#include "SettingsGameplayGUI.h"

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
		/// 
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIInput interface that will be used by this SettingsGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIScreen interface that will be used by this SettingsGUI's GUIControlManager.</param>
		/// <param name="controller"></param>
		SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// 
		/// </summary>
		void Draw();
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

		//GUICheckbox *m_ShowToolTipsCheckbox; //!<

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!<
		Controller *m_Controller; //!<

		GUICollectionBox *m_SettingsTabberBox;
		std::array<GUITab *, ActiveSettingsMenu::SettingsMenuCount> m_SettingsMenuTabs;

		ActiveSettingsMenu m_ActiveSettingsMenu;

		GUIButton *m_BackToMainButton;

		std::unique_ptr<SettingsVideoGUI> m_VideoSettingsMenu; //!<
		std::unique_ptr<SettingsAudioGUI> m_AudioSettingsMenu; //!<
		std::unique_ptr<SettingsInputGUI> m_InputSettingsMenu; //!<
		std::unique_ptr<SettingsGameplayGUI> m_GameplaySettingsMenu; //!<

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