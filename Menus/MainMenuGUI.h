#ifndef _RTEMAINMENUGUI_
#define _RTEMAINMENUGUI_

#include "Controller.h"

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
		/// 
		/// </summary>
		enum MenuScreen {
			Root,
			MainScreen,
			CampaignScreen,
			SettingsScreen,
			ModManagerScreen,
			EditorScreen,
			CreditsScreen,
			QuitScreen,
			ScreenCount
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
		GUIControlManager * GetGUIControlManager();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		MenuScreen GetActiveMenuScreen() const { return m_ActiveMenuScreen; }

		/// <summary>
		/// Sets the main menu GUI to display a screen.
		/// </summary>
		/// <param name="screenToShow">Which screen to show. See MenuScreen enumeration.</param>
		void SetActiveMenuScreen(MenuScreen screenToShow) { m_ActiveMenuScreen = screenToShow; }

		/// <summary>
		/// Reports whether the player has decided to start playing a Scenario this frame.
		/// </summary>
		/// <returns>Whether the Scenario mode should be started.</returns>
		bool ScenarioStarted() const { return m_ScenarioStarted; }

		/// <summary>
		/// Reports whether the player has decided to start playing a Campaign this frame.
		/// </summary>
		/// <returns>Whether the Campaign mode should be started.</returns>
		bool CampaignStarted() const { return m_CampaignStarted; }

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

		/// <summary>
		/// Reports whether the player has decided to quit the program.
		/// </summary>
		/// <returns>Whether the program has been commanded to shit down by the user.</returns>
		bool QuitProgram() const { return m_Quit; }
#pragma endregion

#pragma region Editor Activity Handling
		/// <summary>
		/// Loads "Editor Scene" and starts Actor Editor activity.
		/// </summary>
		void StartActorEditor() const;

		/// <summary>
		/// Loads "Editor Scene" and starts Gib Editor activity.
		/// </summary>
		void StartGibEditor() const;

		/// <summary>
		/// Loads "Editor Scene" and starts Scene Editor activity.
		/// </summary>
		void StartSceneEditor() const;

		/// <summary>
		/// Loads "Editor Scene" and starts Area Editor activity.
		/// </summary>
		void StartAreaEditor() const;

		/// <summary>
		/// Loads "Editor Scene" and starts Assembly Editor activity.
		/// </summary>
		void StartAssemblyEditor() const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;
#pragma endregion

	private:

		enum MenuButton {
			CampaignButton,
			SkirmishButton,
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

		Controller *m_Controller; //!< Controller which controls this menu. Not owned.
		GUIScreen *m_GUIScreen; //!< GUI Screen for use by the in-game GUI.
		GUIInput *m_GUIInput; //!< Input controller.
		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls.


		bool m_MenuEnabled; //!< Visibility state of the menu.
		MenuScreen m_ActiveMenuScreen; //!< Screen selection state.
		bool m_ScreenChange; //!< Change in menu screens detected.

		bool m_ScenarioStarted; //!< Whether Scenario mode was started.
		bool m_CampaignStarted; //!< Whether Campaign mode was started.
		bool m_ActivityRestarted; //!< Whether the game was restarted this frame or not.
		bool m_ActivityResumed; //!< Whether the game was resumed this frame or not.

		std::array<GUICollectionBox *, MenuScreen::ScreenCount> m_MainMenuScreens; //!< Collection box of the buy GUIs.
		std::array<GUIButton *, MenuButton::ButtonCount> m_MainMenuButtons; //!< The main menu buttons.

		int m_RootCollectionBoxOriginalHeight;

		GUICollectionBox *m_EditorMenuPanel; //!< Panel behind editor menu to be resized depending on which editors are available.

		GUICollectionBox *m_CreditsScrollPanel; //!< Scrolling panel for the credits.
		GUILabel *m_CreditsLabel; //!< The label containing all the credits text.
		Timer m_ScrollTimer; //!< Timer for credits scrolling pacing.

		Timer m_BlinkTimer; //!< Notification blink timer.

		GUILabel *m_MetaNoticeLabel; //!< MetaGame notice label.
		bool m_TutorialOffered; //!< Whether the player has been offered a tutorial yet this program run.

		GUILabel *m_VersionLabel; //!< CCCP version number.
		bool m_Quit; //!< Player selected to quit the program.

#pragma region
		/// <summary>
		/// Handles quitting of the game.
		/// </summary>
		void QuitLogic();

		/// <summary>
		/// Loads "Editor Scene" and starts the given editor activity
		/// </summary>
		/// <param name="editorActivity"></param>
		void StartEditorActivity(EditorActivity *editorActivity) const;
#pragma endregion

#pragma region Menu Screen Handling
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
		void ShowCampaignScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowEditorsScreen();

		/// <summary>
		/// 
		/// </summary>
		void ShowCreditsScreen();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// 
		/// </summary>
		void RollCredits();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="guiEventControl"></param>
		/// <returns></returns>
		bool HandleMainScreenButtonPresses(const GUIControl *guiEventControl);

		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();
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