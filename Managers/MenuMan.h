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
		bool LaunchIntoEditor() const { return m_LaunchIntoEditor; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="editorName"></param>
		void SetEditorToLaunch(const std::string_view &editorName) { if (!editorName.empty()) { m_EditorToLaunch = editorName; m_LaunchIntoEditor = true; } }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Launch editor activity specified in command-line argument.
		/// </summary>
		/// <param name="editorToEnter"></param>
		/// <returns>Whether a valid editor name was passed in and set to be launched.</returns>
		bool EnterEditorActivity();

		/// <summary>
		/// Launch multiplayer lobby activity.
		/// </summary>
		void EnterMultiplayerLobby();

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

		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface of this MenuMan.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface of this MenuMan.
		std::unique_ptr<Controller> m_MenuController; //!<

		std::unique_ptr<TitleScreen> m_TitleScreen; //!<
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!<
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!<

		int m_ActiveScreen; //!<

		bool m_LaunchIntoEditor; //!<
		std::string_view m_EditorToLaunch; //!<
	};
}
#endif