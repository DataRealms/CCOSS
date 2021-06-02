#ifndef _RTEMAINMENUGUI_
#define _RTEMAINMENUGUI_

#include "Controller.h"
#include "SettingsGUI.h"
#include "ModManagerGUI.h"

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIButton;
	class GUILabel;
	class GUIControl;

	/// <summary>
	/// Handling for the main menu screen composition and sub-menu interaction.
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
			EnterCreditsScreen,
			BackToMainFromCredits,
			ActivityStarted,
			ActivityResumed,
			Quit
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MainMenuGUI object in system memory and makes it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this MainMenuGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this MainMenuGUI's GUIControlManager.</param>
		MainMenuGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) { Clear(); Create(guiScreen, guiInput); }

		/// <summary>
		/// Makes the MainMenuGUI object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this MainMenuGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this MainMenuGUI's GUIControlManager.</param>
		void Create(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the MainMenuGUI state.
		/// </summary>
		MainMenuUpdateResult Update();

		/// <summary>
		/// Draws the MainMenuGUI to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different sub-menu screens of the main menu.
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
		/// Enumeration for all the different buttons of the main menu and sub-menus.
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

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the MainMenuGUI.
		GUICollectionBox *m_ActiveDialogBox; // The currently active GUICollectionBox in any of the main or sub-menu screens that acts as a dialog box and requires drawing an overlay.

		MenuScreen m_ActiveMenuScreen; //!< The currently active menu screen that is being updated and drawn to the screen. See MenuScreen enumeration.
		MainMenuUpdateResult m_UpdateResult; //!< The result of the MainMenuGUI update. See MainMenuUpdateResult enumeration.
		bool m_MenuScreenChange; //!< Whether the active menu screen was changed and a different one needs to be shown.
		bool m_CampaignNoticeShown; //!< Whether the campaign notice and tutorial offer have been shown to the player.

		int m_RootBoxOriginalHeight; //!< The initial height of the top-level CollectionBox. Used to resize the box back to original height when returning to the main screen from the credits screen.

		Timer m_ResumeButtonBlinkTimer; //!< Activity resume button blink timer.
		Timer m_CreditsScrollTimer; //!< Credits scrolling timer.

		std::unique_ptr<SettingsGUI> m_SettingsMenu; //!< The settings menu screen.
		std::unique_ptr<ModManagerGUI> m_ModManagerMenu; //!< The mod manager menu screen.

		/// <summary>
		/// GUI elements that compose the main menu screen.
		/// </summary>
		GUICollectionBox *m_RootBox;
		GUILabel *m_VersionLabel;
		GUILabel *m_CreditsTextLabel;
		GUICollectionBox *m_CreditsScrollPanel;
		std::array<GUICollectionBox *, MenuScreen::ScreenCount> m_MainMenuScreens;
		std::array<GUIButton *, MenuButton::ButtonCount> m_MainMenuButtons;

#pragma region Create Breakdown
		/// <summary>
		/// Creates all the elements that compose the campaign notice menu screen.
		/// </summary>
		void CreateCampaignNoticeScreen();

		/// <summary>
		/// Creates all the elements that compose the editor selection menu screen.
		/// </summary>
		void CreateEditorsScreen();

		/// <summary>
		/// Creates all the elements that compose the credits menu screen.
		/// </summary>
		void CreateCreditsScreen();
#pragma endregion

#pragma region Menu Screen Handling
		/// <summary>
		/// Hides all main menu screens.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// Sets the MainMenuGUI to display a menu screen.
		/// </summary>
		/// <param name="screenToShow">Which menu screen to display. See MenuScreen enumeration.</param>
		/// <param name="playButtonPressSound">Whether to play a sound if the menu screen change is triggered by a button press.</param>
		void SetActiveMenuScreen(MenuScreen screenToShow, bool playButtonPressSound = true);

		/// <summary>
		/// Makes the main menu screen visible to be interacted with by the player.
		/// </summary>
		void ShowMainScreen();

		/// <summary>
		/// Makes the campaign notice menu screen visible to be interacted with by the player.
		/// </summary>
		void ShowCampaignNoticeScreen();

		/// <summary>
		/// Makes the editor selection menu screen visible to be interacted with by the player.
		/// </summary>
		void ShowEditorsScreen();

		/// <summary>
		/// Makes the credits menu screen visible to be interacted with by the player and resets the scrolling timer for the credits.
		/// </summary>
		void ShowCreditsScreen();

		/// <summary>
		/// Makes the quit confirmation menu screen visible to be interacted with by the user if a game is in progress, or immediately sets the UpdateResult to Quit if not.
		/// </summary>
		void ShowQuitScreenOrQuit();

		/// <summary>
		/// Makes the resume game button visible to be interacted with by the player if a game is in progress and animates it (blinking).
		/// </summary>
		void ShowAndBlinkResumeButton();

		/// <summary>
		/// Progresses the credits scrolling.
		/// </summary>
		/// <returns>Whether the credits finished scrolling.</returns>
		bool RollCredits();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Handles the player interaction with the MainMenuGUI GUI elements.
		/// </summary>
		/// <returns>Whether the player requested to return to the main menu from one of the sub-menus.</returns>
		bool HandleInputEvents();

		/// <summary>
		/// Handles the player interaction with the main screen GUI elements.
		/// </summary>
		/// <param name="guiEventControl">Pointer to the GUI element that the player interacted with.</param>
		void HandleMainScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// Handles the player interaction with the campaign notice screen GUI elements.
		/// </summary>
		/// <param name="guiEventControl">Pointer to the GUI element that the player interacted with.</param>
		void HandleCampaignNoticeScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// Handles the player interaction with the editor selection screen GUI elements.
		/// </summary>
		/// <param name="guiEventControl">Pointer to the GUI element that the player interacted with.</param>
		void HandleEditorsScreenInputEvents(const GUIControl *guiEventControl);

		/// <summary>
		/// Handles the player interaction with the quit screen GUI elements.
		/// </summary>
		/// <param name="guiEventControl">Pointer to the GUI element that the player interacted with.</param>
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