#ifndef _RTEMENUMAN_
#define _RTEMENUMAN_

#include "Singleton.h"

#define g_MenuMan MenuMan::Instance()

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class Controller;
	class TitleScreen;
	class MainMenuGUI;
	class ScenarioGUI;

	/// <summary>
	/// 
	/// </summary>
	class MenuMan : public Singleton<MenuMan> {

	public:

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		MenuMan() = default;

		/// <summary>
		/// 
		/// </summary>
		void Initialize(bool initLoadingScreen = true);

		/// <summary>
		/// Destroy the Main Menu and initialize it again after a resolution change. Must be done otherwise the GUIs retain the original resolution settings and become all screwy.
		/// </summary>
		void Reinitialize();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		TitleScreen * GetTitleScreen() const { return m_TitleScreen.get(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		bool Update();

		/// <summary>
		/// 
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different menu screens that are active based on transition states.
		/// </summary>
		enum ActiveMenu {
			MenusDisabled,
			MainMenuActive,
			ScenarioMenuActive,
			CampaignMenuActive
		};

		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface of this MenuMan.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface of this MenuMan.
		std::unique_ptr<Controller> m_MenuController; //!<

		std::unique_ptr<TitleScreen> m_TitleScreen; //!<
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!<
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!<

		ActiveMenu m_ActiveMenu; //!<

#pragma region Updates
		/// <summary>
		/// 
		/// </summary>
		void SetActiveMenu();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateMainMenu() const;

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateScenarioMenu() const;

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateCampaignMenu() const;
#pragma endregion
	};
}
#endif