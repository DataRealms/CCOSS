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
		/// <param name="firstTimeInit">Whether this is initializing for the first time, meaning the game is booting up, so the loading screen needs to be shown and all module loading should happen.</param>
		void Initialize(bool firstTimeInit = true);

		/// <summary>
		/// Reinitializes all the Main Menu GUIs after a resolution change. Must be done otherwise the GUIs retain the original resolution settings and become all screwy.
		/// </summary>
		void Reinitialize();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the TitleScreen of this MenuMan.
		/// </summary>
		/// <returns>Pointer to the TitleScreen object of this MenuMan. Ownership is NOT transferred!</returns>
		TitleScreen * GetTitleScreen() const { return m_TitleScreen.get(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the MenuMan state.
		/// </summary>
		/// <returns>Whether the MenuMan update has reached a state where the menu loop should be exited so the simulation loop can proceed.</returns>
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
			MetaGameMenuActive
		};

		ActiveMenu m_ActiveMenu; //!< The currently active menu screen that is being updated and drawn. See ActiveMenu enumeration.

		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface of this MenuMan.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface of this MenuMan.
		std::unique_ptr<Controller> m_MenuController; //!< A Controller to handle player input in menu screens that require it.

		std::unique_ptr<TitleScreen> m_TitleScreen; //!< The title screen.
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!< The main menu screen.
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!< The scenario menu screen.

#pragma region Updates
		/// <summary>
		/// Sets the active menu screen to be enabled, updated and drawn to the screen, besides the title screen which is always active.
		/// </summary>
		void SetActiveMenu();

		/// <summary>
		/// Updates the main menu screen and handles the update results.
		/// </summary>
		/// <returns>Whether the program was set to be terminated by the user through the main menu screen.</returns>
		bool UpdateMainMenu() const;

		/// <summary>
		/// Updates the scenario menu screen and handles the update results.
		/// </summary>
		void UpdateScenarioMenu() const;

		/// <summary>
		/// Updates the MetaGame menu screen and handles the update results.
		/// </summary>
		/// <returns>Whether the program was set to be terminated by the user through the MetaGame menu screen.</returns>
		bool UpdateMetaGameMenu() const;
#pragma endregion

		// Disallow the use of some implicit methods.
		MenuMan(const MenuMan &reference) = delete;
		MenuMan & operator=(const MenuMan &rhs) = delete;
	};
}
#endif