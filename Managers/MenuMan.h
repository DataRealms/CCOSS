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

#pragma region Updates
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool UpdateMainMenu();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateScenarioMenu();
#pragma endregion

		/// <summary>
		/// Custom deleters for std::unique_ptr members. Must be defined to avoid including the class headers and just rely on forward declaration.
		/// </summary>
		struct AllegroInputDeleter { void operator()(AllegroInput *ptr) const; };
		struct AllegroScreenDeleter { void operator()(AllegroScreen *ptr) const; };

		std::unique_ptr<AllegroInput, AllegroInputDeleter> m_GUIInput; //!< Input interface of this.
		std::unique_ptr<AllegroScreen, AllegroScreenDeleter> m_GUIScreen; //!< Screen interface of this.

		std::unique_ptr<Controller> m_MenuController; //!<

		std::unique_ptr<TitleScreen> m_TitleScreen; //!<
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!<
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!<

		int m_ActiveScreen; //!<

	};
}
#endif