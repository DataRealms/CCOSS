#ifndef _RTEMENUMAN_
#define _RTEMENUMAN_

#include "Singleton.h"

#define g_MenuMan MenuMan::Instance()

namespace RTE {

	class Controller;
	class LoadingGUI;
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

#pragma region Destruction
		/// <summary>
		/// 
		/// </summary>
		~MenuMan() = default;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		void Update();

		/// <summary>
		/// 
		/// </summary>
		void Draw();
#pragma endregion

	private:

		int m_ActiveScreen; //!<

		std::unique_ptr<Controller> m_MenuController; //!<

		std::unique_ptr<LoadingGUI> m_LoadingScreen; //!<
		std::unique_ptr<TitleScreen> m_TitleScreen; //!<
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!<
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!<
	};
}
#endif