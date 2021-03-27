#ifndef _RTEMAINMENUGUI_
#define _RTEMAINMENUGUI_

#include "ActivityMan.h"
#include "Timer.h"
#include "Box.h"

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUICheckbox;
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

		class ModRecord {
		public:
			std::string ModulePath;
			std::string ModuleName;
			std::string Description;
			bool Disabled;

			bool operator<(const ModRecord &rhs) const { return ModulePath < rhs.ModulePath; }
		};

		class ScriptRecord {
		public:
			std::string PresetName;
			std::string Description;
			bool Enabled;

			bool operator<(const ScriptRecord &rhs) const { return PresetName < rhs.PresetName; }
		};

		enum MenuScreen {
			ROOT = 0,
			MAINSCREEN,
			PLAYERSSCREEN,
			SKIRMISHSCREEN,
			DIFFICULTYSCREEN,
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
		bool IsEnabled() { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }

		/// <summary>
		/// Reports whether the player has decided to start playing a Scenario this frame.
		/// </summary>
		/// <returns>Whether the Scenario mode should be started.</returns>
		bool ScenarioStarted() { return m_ScenarioStarted; }

		/// <summary>
		/// Reports whether the player has decided to start playing a Campaign this frame.
		/// </summary>
		/// <returns>Whether the Campaign mode should be started.</returns>
		bool CampaignStarted() { return m_CampaignStarted; }

		/// <summary>
		/// Reports whether the player has decided to restart an activity this frame. All parameters for the new game has been fed into ActivityMan already.
		/// </summary>
		/// <returns>Whether the activity should be restarted.</returns>
		bool ActivityRestarted() { return m_ActivityRestarted; }

		/// <summary>
		/// Reports whether the player has decided to resume the current activity.
		/// </summary>
		/// <returns>Whether the activity should be resumed.</returns>
		bool ActivityResumed() { return m_ActivityResumed; }

		/// <summary>
		/// Reports whether the player has decided to quit the program.
		/// </summary>
		/// <returns>Whether the program has been commanded to shit down by the user.</returns>
		bool QuitProgram() { return m_Quit; }

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
		void StartActorEditor();

		/// <summary>
		/// Loads "Editor Scene" and starts Gib Editor activity.
		/// </summary>
		void StartGibEditor();

		/// <summary>
		/// Loads "Editor Scene" and starts Scene Editor activity.
		/// </summary>
		void StartSceneEditor();

		/// <summary>
		/// Loads "Editor Scene" and starts Area Editor activity.
		/// </summary>
		void StartAreaEditor();

		/// <summary>
		/// Loads "Editor Scene" and starts Assembly Editor activity.
		/// </summary>
		void StartAssemblyEditor();
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

		/// <summary>
		/// Sets the ActivityMan up with the current data for a skirmish game.
		/// </summary>
		void SetupSkirmishActivity();

		/// <summary>
		/// Updates the contents of the scene selection box.
		/// </summary>
		void UpdateScenesBox();

		/// <summary>
		/// Updates the size and contents of the team assignment boxes, according to the number of players chosen.
		/// </summary>
		void UpdateTeamBoxes();

		/// <summary>
		/// Updates the contents of the screen resolution combo box.
		/// </summary>
		void UpdateResolutionCombo();

		/// <summary>
		/// Updates the position of the volume sliders, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateVolumeSliders();

		/// <summary>
		/// Updates the text on the configuration labels, based on actual UInputMan settings.
		/// </summary>
		void UpdateDeviceLabels();

		/// <summary>
		/// Updates the contents of the control configuration screen.
		/// </summary>
		void UpdateConfigScreen();


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

		enum SkirmishTeams {
			P1TEAM = 0,
			P2TEAM,
			P3TEAM,
			P4TEAM,
			SKIRMISHPLAYERCOUNT
		};

		enum OptionsButtons {
			FULLSCREENORWINDOWED = 0,
			P1NEXT,
			P2NEXT,
			P3NEXT,
			P4NEXT,
			P1PREV,
			P2PREV,
			P3PREV,
			P4PREV,
			P1CONFIG,
			P2CONFIG,
			P3CONFIG,
			P4CONFIG,
			P1CLEAR,
			P2CLEAR,
			P3CLEAR,
			P4CLEAR,
			UPSCALEDFULLSCREEN,
			OPTIONSBUTTONCOUNT
		};

		enum OptionsCheckboxes {
			FLASHONBRAINDAMAGE = 0,
			BLIPONREVEALUNSEEN,
			SHOWFOREIGNITEMS,
			SHOWTOOLTIPS,
			OPTIONSCHECKBOXCOUNT
		};


		enum OptionsLabels {
			P1DEVICE = 0,
			P2DEVICE,
			P3DEVICE,
			P4DEVICE,
			OPTIONSLABELCOUNT
		};

		enum OptionsFocus {
			MUSICVOLUME = 0,
			SOUNDVOLUME
		};

		enum GamepadType {
			DPAD = 0,
			DANALOG,
			XBOX360
		};

		enum ConfigSteps {
			KEYBOARDSTEPS = 16,
			MOUSESTEPS = 11,
			DPADSTEPS = 13,
			DANALOGSTEPS = 19,
			XBOX360STEPS = 19
		};

		enum ConfigLabels {
			CONFIGTITLE = 0,
			CONFIGRECOMMENDATION,
			CONFIGINSTRUCTION,
			CONFIGINPUT,
			CONFIGSTEPS,
			CONFIGLABELCOUNT
		};

		enum EditorButtons {
			SCENEEDITOR = 0,
			AREAEDITOR,
			ASSEMBLYEDITOR,
			GIBEDITOR,
			ACTOREDITOR,
			EDITORBUTTONCOUNT
		};

		enum BlinkMode {
			NOBLINK = 0,
			NOFUNDS,
			NOCRAFT,
			BLINKMODECOUNT
		};

		enum DeadZoneSliders {
			P1DEADZONESLIDER = 0,
			P2DEADZONESLIDER,
			P3DEADZONESLIDER,
			P4DEADZONESLIDER,
			DEADZONESLIDERCOUNT
		};

		Controller *m_pController; //!< Controller which conrols this menu. Not owned
		GUIScreen *m_pGUIScreen; //!< GUI Screen for use by the in-game GUI
		GUIInput *m_pGUIInput; //!< Input controller
		GUIControlManager *m_pGUIController; //!< The control manager which holds all the controls
		int m_MenuEnabled; //!< Visibility state of the menu
		int m_MenuScreen; //!< Screen selection state
		bool m_ScreenChange; //!< Change in menu screens detected
		int m_MainMenuFocus; //!< Focus state
		int m_FocusChange; //!< Focus change direction - 0 is none, negative is back, positive forward
		float m_MenuSpeed; //!< Speed at which the menus appear and disappear
		int m_ListItemIndex; //!< Which item in the currently focused list box we have selected
		Timer m_BlinkTimer; //!< Notification blink timer
		int m_BlinkMode; //!< What we're blinking

		GUICollectionBox *m_apScreenBox[SCREENCOUNT]; //!< Collection box of the buy GUIs
		GUIButton *m_MainMenuButtons[MAINMENUBUTTONCOUNT]; //!< The main menu buttons
		GUIComboBox *m_pSceneSelector; //!< Skirmish scene selection box

		//!< The skirmish setup screen team box panels
		GUICollectionBox *m_pTeamBox;
		GUICollectionBox *m_aSkirmishBox[SKIRMISHPLAYERCOUNT];

		GUIButton *m_aSkirmishButton[SKIRMISHPLAYERCOUNT]; //!< The skirmish setup screen buttons

		GUILabel *m_pCPUTeamLabel; //!< Label describing the CPU team
		GUIButton *m_aOptionButton[OPTIONSBUTTONCOUNT]; //!< The options buttons
		GUILabel *m_aOptionsLabel[OPTIONSLABELCOUNT]; //!< Labels of the options screen
		GUILabel *m_aDeadZoneLabel[DEADZONESLIDERCOUNT]; //!< Labels of the options screen
		GUISlider *m_aDeadZoneSlider[DEADZONESLIDERCOUNT]; //!< Slider for dead zone controls
		GUICheckbox *m_aDeadZoneCheckbox[DEADZONESLIDERCOUNT]; //!< Checkboxes for dead zone controls
		GUICheckbox *m_aOptionsCheckbox[OPTIONSCHECKBOXCOUNT]; //!< Checkboxes of the options screen

		GUIComboBox *m_pResolutionCombo; //!< Resolution combobox

		//!< Option sound sliders
		GUILabel *m_pSoundLabel;
		GUILabel *m_pMusicLabel;
		GUISlider *m_pSoundSlider;
		GUISlider *m_pMusicSlider;

		GUIButton *m_pBackToOptionsButton; //!< Back to options from the test and config screens
		GUIButton *m_pSkipButton; //!< Skip button for config screen
		GUIButton *m_pDefaultButton; //!< Defualt button for config screen

		int m_ConfiguringPlayer; //!< Which player's control scheme we are currently configuring
		int m_ConfiguringDevice; //!< Which type of device we are currently configuring
		int m_ConfiguringGamepad; //!< Which type of gamepad we are currently configuring
		int m_ConfigureStep; //!< Which step in current configure sequence

		GUILabel *m_pConfigLabel[CONFIGLABELCOUNT]; //!< Labels of the control config screen
		GUIButton *m_aEditorButton[EDITORBUTTONCOUNT]; // The editor buttons
		GUILabel *m_pMetaNoticeLabel; //!< Metagame notice label
		GUILabel *m_VersionLabel; //!< CCCP version number.

		//!< Controller diagram bitmaps
		BITMAP **m_aDPadBitmaps;
		BITMAP **m_aDualAnalogBitmaps;
		//!< Controller diagram panel
		GUICollectionBox *m_pRecommendationBox;
		GUICollectionBox *m_pRecommendationDiagram;

		GUIButton *m_pConfigSkipButton; //!< Skip forward one config step button
		GUIButton *m_pConfigBackButton; //!< Go back one config step button

		//!< Gamepad type selection UI elements
		GUICollectionBox *m_pDPadTypeBox;
		GUICollectionBox *m_pDAnalogTypeBox;
		GUICollectionBox *m_pXBox360TypeBox;
		GUICollectionBox *m_pDPadTypeDiagram;
		GUICollectionBox *m_pDAnalogTypeDiagram;
		GUICollectionBox *m_pXBox360TypeDiagram;
		GUIButton *m_pDPadTypeButton;
		GUIButton *m_pDAnalogTypeButton;
		GUIButton *m_pXBox360TypeButton;

		GUICollectionBox *m_pEditorPanel; //!< Panel behind editor menu to be resized depending on which editors are available
		GUICollectionBox *m_pScrollPanel; //!< Scrolling panel for the credits
		GUILabel *m_CreditsLabel; //!< The label containing all the credits text.
		Timer m_ScrollTimer; //!< Timer for credits scrolling pacing

		GUICollectionBox *m_ResolutionChangeDialog;
		GUIButton *m_ButtonConfirmResolutionChange;
		GUIButton *m_ButtonCancelResolutionChange;
		GUIButton *m_ButtonConfirmResolutionChangeFullscreen;
		bool m_ResolutionChangeToUpscaled;

		bool m_ScenarioStarted; //!< Whether Scenario mode was started
		bool m_CampaignStarted; //!< Whether Campaign mode was started
		bool m_ActivityRestarted; //!< Whether the game was restarted this frame or not
		bool m_ActivityResumed; //!< Whether the game was resumed this frame or not
		bool m_TutorialOffered; //!< Whether the player has been offered a tutoral yet this program run
		int m_StartPlayers; //!< How many players are chosen to be in the new game
		int m_StartTeams; //!< How many teams are chosen to be in the new game
		int m_StartFunds; //!< How much money both teams start with in the new game
		int m_aTeamAssignments[SKIRMISHPLAYERCOUNT]; //!< How many teams are chosen to be in the new game
		int m_CPUTeam; //!< Which team is CPU managed, if any (-1)
		int m_StartDifficulty; //!< Difficulty setting
		bool m_Quit; //!< Player selected to quit the program

		// Max available resolutions.
		int m_MaxResX;
		int m_MaxResY;

	private:

		/// <summary>
		/// Loads "Editor Scene" and starts the given editor activity
		/// </summary>
		/// <param name="editorActivity"></param>
		void StartEditorActivity(EditorActivity *editorActivity);

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