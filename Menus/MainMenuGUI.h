#ifndef _RTEMAINMENUGUI_
#define _RTEMAINMENUGUI_

#include "Controller.h"

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;

	class GUIButton;
	class GUILabel;
	class GUISlider;
	class Entity;
	class EditorActivity;

	/// <summary>
	/// 
	/// </summary>
	class MainMenuGUI {

	public:

		enum MenuScreen {
			ROOT = 0,
			MAINSCREEN,
			OPTIONSSCREEN,
			CONFIGSCREEN,
			EDITORSCREEN,
			CREDITSSCREEN,
			METASCREEN,
			QUITSCREEN,
			MODMANAGERSCREEN,
			SCREENCOUNT
		};

		/// <summary>
		/// Constructor method used to instantiate a MainMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		MainMenuGUI() { Clear(); }

		/// <summary>
		/// Destructor method used to clean up a MainMenuGUI object before deletion from system memory.
		/// </summary>
		~MainMenuGUI() { Destroy(); }

		/// <summary>
		/// Makes the MainMenuGUI object ready for use.
		/// </summary>
		/// <param name="pController">A pointer to a Controller which will control this Menu. Ownership is NOT TRANSFERRED!</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *pController);

		/// <summary>
		/// Destroys and resets (through Clear()) the MainMenuGUI object.
		/// </summary>
		void Destroy();

		/// <summary>
		/// Gets the GUIControlManager owned and used by this.
		/// </summary>
		/// <returns>The GUIControlManager. Ownership is not transferred!</returns>
		GUIControlManager * GetGUIControlManager();

		/// <summary>
		/// Enables or disables the menu. This will animate it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Reports whether the menu is enabled or not.
		/// </summary>
		/// <returns></returns>
		bool IsEnabled() const { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }

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

		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;

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

		/// <summary>
		/// Sets the main menu GUI to display a screen.
		/// </summary>
		/// <param name="screenToShow">Which screen to show. See MenuScreen enumeration.</param>
		void SetMenuScreen(MenuScreen screenToShow) { m_MenuScreen = screenToShow; }

	protected:

		/// <summary>
		/// Hides all menu screens, so one can easily be unhidden and shown only.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// Handles quitting of the game.
		/// </summary>
		void QuitLogic();

		enum MenuEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		enum MainButtons {
			CAMPAIGN = 0,
			SKIRMISH,
			MULTIPLAYER,
			OPTIONS,
			MODMANAGER,
			EDITOR,
			CREDITS,
			QUIT,
			RESUME,
			BACKTOMAIN,
			PLAYTUTORIAL,
			METACONTINUE,
			QUITCONFIRM,
			QUITCANCEL,
			MAINMENUBUTTONCOUNT
		};

		enum EditorButtons {
			SCENEEDITOR = 0,
			AREAEDITOR,
			ASSEMBLYEDITOR,
			GIBEDITOR,
			ACTOREDITOR,
			EDITORBUTTONCOUNT
		};

		Controller *m_pController; //!< Controller which controls this menu. Not owned
		GUIScreen *m_pGUIScreen; //!< GUI Screen for use by the in-game GUI
		GUIInput *m_pGUIInput; //!< Input controller
		GUIControlManager *m_pGUIController; //!< The control manager which holds all the controls
		int m_MenuEnabled; //!< Visibility state of the menu
		int m_MenuScreen; //!< Screen selection state
		bool m_ScreenChange; //!< Change in menu screens detected
		Timer m_BlinkTimer; //!< Notification blink timer

		std::array< GUICollectionBox *, SCREENCOUNT> m_apScreenBox; //!< Collection box of the buy GUIs
		std::array<GUIButton *, MAINMENUBUTTONCOUNT> m_MainMenuButtons; //!< The main menu buttons
		GUIComboBox *m_pSceneSelector; //!< Skirmish scene selection box

		std::array<GUIButton *, EDITORBUTTONCOUNT> m_aEditorButton; // The editor buttons
		GUILabel *m_pMetaNoticeLabel; //!< MetaGame notice label
		GUILabel *m_VersionLabel; //!< CCCP version number.

		GUICollectionBox *m_pEditorPanel; //!< Panel behind editor menu to be resized depending on which editors are available
		GUICollectionBox *m_pScrollPanel; //!< Scrolling panel for the credits
		GUILabel *m_CreditsLabel; //!< The label containing all the credits text.
		Timer m_ScrollTimer; //!< Timer for credits scrolling pacing

		bool m_ScenarioStarted; //!< Whether Scenario mode was started
		bool m_CampaignStarted; //!< Whether Campaign mode was started
		bool m_ActivityRestarted; //!< Whether the game was restarted this frame or not
		bool m_ActivityResumed; //!< Whether the game was resumed this frame or not
		bool m_TutorialOffered; //!< Whether the player has been offered a tutorial yet this program run

		bool m_Quit; //!< Player selected to quit the program

	private:

		/// <summary>
		/// Loads "Editor Scene" and starts the given editor activity
		/// </summary>
		/// <param name="editorActivity"></param>
		void StartEditorActivity(EditorActivity *editorActivity) const;

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