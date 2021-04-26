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
	/// The singleton manager responsible for handling all the out-of-game menu screens (main menu, scenario menu, etc.).
	/// </summary>
	class MenuMan : public Singleton<MenuMan> {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MenuMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		MenuMan() = default;

		/// <summary>
		/// Makes the MenuMan object ready for use.
		/// </summary>
		void Initialize(bool initLoadingScreen = true);

		/// <summary>
		/// Reinitializes all the Main Menu GUIs after a resolution change. Must be done otherwise the GUIs retain the original resolution settings and become all screwy.
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
		/// Updates the state of this MenuMan.
		/// </summary>
		bool Update();

		/// <summary>
		/// Draws the MenuMan to the screen.
		/// </summary>
		void Draw() const;
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

		ActiveMenu m_ActiveMenu; //!< The currently active menu screen that is being updated and drawn. See ActiveMenu enumeration.

		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface of this MenuMan.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface of this MenuMan.
		std::unique_ptr<Controller> m_MenuController; //!<

		std::unique_ptr<TitleScreen> m_TitleScreen; //!<
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!<
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!<

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