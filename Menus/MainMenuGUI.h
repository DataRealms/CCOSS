#ifndef _RTEMAINMENUGUI_
#define _RTEMAINMENUGUI_

#include "Controller.h"
#include "SettingsGUI.h"
#include "ModManagerGUI.h"

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIControl;
	class GUIButton;
	class GUILabel;
	class EditorActivity;

	/// <summary>
	/// 
	/// </summary>
	class MainMenuGUI {

	public:

		/// <summary>
		/// Enumeration for the results of the MainMenuGUI input and event update.
		/// </summary>
		enum class MainMenuUpdateResult {
			NoEvent,
			CampaignStarted,
			ScenarioStarted,
			MultiplayerStarted,
			EnterCreditsScreen,
			BackToMainFromScenario,
			BackToMenuFromCampaign,
			BackToMainFromCredits,
			Quit
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MainMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		MainMenuGUI(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller) { Clear(); Create(guiScreen, guiInput, controller); }

		/// <summary>
		/// Makes the MainMenuGUI object ready for use.
		/// </summary>
		/// <param name="controller">A pointer to a Controller which will control this Menu. Ownership is NOT TRANSFERRED!</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		void Create(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Reports whether the menu is enabled or not.
		/// </summary>
		/// <returns></returns>
		bool IsEnabled() const { return m_MenuEnabled; }

		/// <summary>
		/// Enables or disables the menu. This will animate it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Gets the GUIControlManager owned and used by this.
		/// </summary>
		/// <returns>The GUIControlManager. Ownership is not transferred!</returns>
		GUIControlManager * GetGUIControlManager() const { return m_GUIControlManager.get(); }

		/// <summary>
		/// Reports whether the player has decided to restart an activity this frame. All parameters for the new game has been fed into ActivityMan already.
		/// </summary>
		/// <returns>Whether the activity should be restarted.</returns>
		bool ActivityRestarted() const { return m_ActivityRestarted; }

		/// <summary>
		/// Reports whether the player has decided to resume the current activity.
		/// </summary>
		/// <returns>Whether the activity should be resumed.</returns>
		bool ActivityResumed() const { return m_ActivityResumed; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		MainMenuUpdateResult Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum MenuScreen {
			MainScreen,
			CampaignNoticeScreen,
			SettingsScreen,
			ModManagerScreen,
			EditorScreen,
			CreditsScreen,
			QuitScreen,
			ScreenCount
		};

		/// <summary>
		/// 
		/// </summary>
		enum MenuButton {
			CampaignButton,
			ScenarioButton,
			MultiplayerButton,
			SettingsButton,
			ModManagerButton,
			EditorsButton,
			CreditsButton,
			QuitButton,
			ResumeButton,
			BackToMainButton,
			PlayTutorialButton,
			CampaignContinueButton,
			QuitConfirmButton,
			QuitCancelButton,
			SceneEditorButton,
			AreaEditorButton,
			AssemblyEditorButton,
			GitEditorButton,
			ActorEditorButton,
			ButtonCount
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls.

		bool m_MenuEnabled; //!< Visibility state of the menu.
		MenuScreen m_ActiveMenuScreen; //!< Screen selection state.
		bool m_ScreenChange; //!< Change in menu screens detected.

		bool m_ActivityRestarted; //!< Whether the game was restarted this frame or not.
		bool m_ActivityResumed; //!< Whether the game was resumed this frame or not.

		MainMenuUpdateResult m_UpdateResult; //!<

		GUICollectionBox *m_RootBox; //!<
		int m_RootBoxOriginalHeight; //!<

		GUICollectionBox *m_CreditsScrollPanel; //!< Scrolling panel for the credits.
		GUILabel *m_CreditsLabel; //!< The label containing all the credits text.
		Timer m_ScrollTimer; //!< Timer for credits scrolling pacing.

		Timer m_BlinkTimer; //!< Notification blink timer.

		GUILabel *m_MetaNoticeLabel; //!< MetaGame notice label.
		bool m_TutorialOffered; //!< Whether the player has been offered a tutorial yet this program run.

		GUILabel *m_VersionLabel; //!< CCCP version number.

		std::array<GUICollectionBox *, MenuScreen::ScreenCount> m_MainMenuScreens; //!< Collection box of the buy GUIs.
		std::array<GUIButton *, MenuButton::ButtonCount> m_MainMenuButtons; //!< The main menu buttons.

		std::unique_ptr<SettingsGUI> m_SettingsMenu; //!<
		std::unique_ptr<ModManagerGUI> m_ModManagerMenu; //!<

#pragma region Create Breakdown
		/// <summary>
		/// 
		/// </summary>
		void CreateCampaignNoticeScreen();

		/// <summary>
		/// 
		/// </summary>
		void CreateEditorsScreen();

		/// <summary>
		/// 
		/// </summary>
		void CreateCreditsScreen();
#pragma endregion

#pragma region
		/// <summary>
		/// Handles quitting of the game.
		/// </summary>
		void QuitLogic();
#pragma endregion

#pragma region Menu Screen Handling
		/// <summary>
		/// Sets the main menu GUI to display a screen.
		/// </summary>
		/// <param name="screenToShow">Which screen to show. See MenuScreen enumeration.</param>
		void SetActiveMenuScreen(MenuScreen screenToShow, bool playButtonPressSound = true);

		/// <summary>
		/// Hides all menu screens, so one can easily be unhidden and shown only.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// 
		/// </summary>
		void ShowMainScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowCampaignNoticeScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowEditorsScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowCreditsScreen();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool RollCredits();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiEventControl"></param>
		void HandleMainScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiEventControl"></param>
		void HandleCampaignNoticeScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiEventControl"></param>
		void HandleEditorsScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiEventControl"></param>
		void HandleQuitScreenInputEvents(const GUIControl *guiEventControl);
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this MainMenuGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		MainMenuGUI(const MainMenuGUI &reference) = delete;
		MainMenuGUI & operator=(const MainMenuGUI &rhs) = delete;
	};
}
#endif