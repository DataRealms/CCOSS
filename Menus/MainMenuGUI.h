#ifndef _MAINMENUGUI_
#define _MAINMENUGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MainMenuGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     MainMenuGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ActivityMan.h"
#include "Timer.h"
#include "Box.h"

struct BITMAP;


namespace RTE
{

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


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MainMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A full menu system that represents a purchasing GUI for Cortex Command
// Parent(s):       None.
// Class history:   8/22/2006 MainMenuGUI Created.

class MainMenuGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	class ModRecord
	{
		public: 
			std::string ModulePath;
			std::string ModuleName;
			std::string Description;
			bool Disabled;

			bool operator<(const ModRecord &rhs) const { return ModulePath < rhs.ModulePath; }
	};

	class ScriptRecord
	{
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

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MainMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MainMenuGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MainMenuGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MainMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MainMenuGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~MainMenuGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MainMenuGUI object ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController);


//////////////////////////////////////////////////////////////////////////////////////////
// method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MainMenuGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MainMenuGUI object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGUIControlManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the GUIControlManager owned and used by this.
// Arguments:       None.
// Return value:    The GUIControlManager. Ownership is not transferred!

    GUIControlManager * GetGUIControlManager();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.
// Arguments:       Whether to enable or disable the menu.
// Return value:    None.

    void SetEnabled(bool enable = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is enabled or not.
// Arguments:       None.
// Return value:    None.

    bool IsEnabled() { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ScenarioStarted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to start playing a Scenario
//                  this frame.
// Arguments:       None.
// Return value:    Whether the Scenario mode should be started.

    bool ScenarioStarted() { return m_ScenarioStarted; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CampaignStarted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to start playing a Campaign
//                  this frame.
// Arguments:       None.
// Return value:    Whether the Campaign mode should be started.

    bool CampaignStarted() { return m_CampaignStarted; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityRestarted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to restart an activity this frame.
//                  All parameters for the new game has been fed into ActivityMan already.
// Arguments:       None.
// Return value:    Whether the activity should be restarted.

    bool ActivityRestarted() { return m_ActivityRestarted; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityResumed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to resume the current activity.
// Arguments:       None.
// Return value:    Whether the activity should be resumed.

    bool ActivityResumed() { return m_ActivityResumed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QuitProgram
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to quit the program.
// Arguments:       None.
// Return value:    Whether the program has been commanded to shit down by the user.

    bool QuitProgram() { return m_Quit; }

// Temp, should be doen throught the GUI
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default starting funds for all games.
// Arguments:       The starting funds.
// Return value:    None.

    void SetStartFunds(int startFunds) { m_StartFunds = startFunds; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   AllowPioneerPromo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if we're allowed to show Planetoid Pioneer promo
// Arguments:       None.
// Return value:    true if we're in the main menu i.e. allowed to show PP promo

	bool AllowPioneerPromo() { return m_MenuScreen == MAINSCREEN; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   EnablePioneerPromoButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the menu that Planetoid Pioneers promo is active and can be clicked
// Arguments:       None.
// Return value:    None.

	void EnablePioneerPromoButton() { m_PioneerPromoVisible = true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   EnablePioneerPromoButton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the menu that Planetoid Pioneers promo is not visible active and can't be clicked
// Arguments:       None.
// Return value:    None.

	void DisablePioneerPromoButton() { m_PioneerPromoVisible = false; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   SetPioneerPromoBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells the menu where Planetoid Pioneers promo pic is located so it could be clicked
// Arguments:       Location of promo pic clockable area.
// Return value:    None.

	void SetPioneerPromoBox(Box newBox) { m_PioneerPromoBox = newBox; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu
// Arguments:       The bitmap to draw on.
// Return value:    None.

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

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideAllScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides all menu screens, so one can easily be unhidden and shown only.
// Arguments:       None.
// Return value:    None.

    void HideAllScreens();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QuitLogic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles quitting of the game.
// Arguments:       None.
// Return value:    None.

    void QuitLogic();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSkirmishActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ActivityMan up with the current data for a skirmish game.
// Arguments:       None.
// Return value:    None.

    void SetupSkirmishActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateScenesBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the scene selection box.
// Arguments:       None.
// Return value:    None.

    void UpdateScenesBox();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTeamBoxes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the size and contents of the team assignment boxes, according
//                  to the number of players chosen.
// Arguments:       None.
// Return value:    None.

    void UpdateTeamBoxes();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateResolutionCombo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the screen resolution combo box
// Arguments:       None.
// Return value:    None.

    void UpdateResolutionCombo();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateVolumeSliders
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the position of the volume sliders, based on what the AudioMan
//                  is currently set to.
// Arguments:       None.
// Return value:    None.

    void UpdateVolumeSliders();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateDeviceLabels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text on the config labels, based on actual UInputMan settings.
// Arguments:       None.
// Return value:    None.

    void UpdateDeviceLabels();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateConfigScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the control configuarion screen.
// Arguments:       None.
// Return value:    None.

    void UpdateConfigScreen();



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeModString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes UI displayable string with mod info
// Arguments:       None.
// Return value:    String with mod info.

	std::string MakeModString(ModRecord r);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeScriptString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes UI displayable string with script info
// Arguments:       None.
// Return value:    String with script info.

	std::string MakeScriptString(ScriptRecord r);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleMod
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns currently selected mod on and aff and changes UI elements accordingly.
// Arguments:       None.
// Return value:    None.

	void ToggleMod();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns currently selected script on and aff and changes UI elements accordingly.
// Arguments:       None.
// Return value:    None.

	void ToggleScript();


    enum MenuEnabled
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    enum MainButtons
    {
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

    enum SkirmishTeams
    {
        P1TEAM = 0,
        P2TEAM,
        P3TEAM,
        P4TEAM,
        SKIRMISHPLAYERCOUNT
    };

    enum OptionsButtons
    {
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

    enum OptionsCheckboxes
    {
		FLASHONBRAINDAMAGE = 0,
		BLIPONREVEALUNSEEN,
		SHOWFOREIGNITEMS,
		SHOWTOOLTIPS,
		OPTIONSCHECKBOXCOUNT
    };


    enum OptionsLabels
    {
        P1DEVICE = 0,
        P2DEVICE,
        P3DEVICE,
        P4DEVICE,
        OPTIONSLABELCOUNT
    };

    enum OptionsFocus
    {
        MUSICVOLUME = 0,
        SOUNDVOLUME
    };

    enum GamepadType
    {
        DPAD = 0,
        DANALOG,
        XBOX360
    };

    enum ConfigSteps
    {
        KEYBOARDSTEPS = 16,
        MOUSESTEPS = 11,
        DPADSTEPS = 13,
        DANALOGSTEPS = 19,
        XBOX360STEPS = 19
    };

    enum ConfigLabels
    {
        CONFIGTITLE = 0,
        CONFIGRECOMMENDATION,
        CONFIGINSTRUCTION,
        CONFIGINPUT,
        CONFIGSTEPS,
        CONFIGLABELCOUNT
    };

    enum EditorButtons
    {
        SCENEEDITOR = 0,
        AREAEDITOR,
        ASSEMBLYEDITOR,
        GIBEDITOR,
        ACTOREDITOR,
        EDITORBUTTONCOUNT
    };

    enum BlinkMode
    {
        NOBLINK = 0,
        NOFUNDS,
        NOCRAFT,
        BLINKMODECOUNT
    };

	enum DeadZoneSliders
	{
		P1DEADZONESLIDER = 0,
		P2DEADZONESLIDER,
		P3DEADZONESLIDER,
		P4DEADZONESLIDER,
		DEADZONESLIDERCOUNT
	};

    // Controller which conrols this menu. Not owned
    Controller *m_pController;
    // GUI Screen for use by the in-game GUI
    GUIScreen *m_pGUIScreen;
    // Input controller
    GUIInput *m_pGUIInput;
    // The control manager which holds all the controls
    GUIControlManager *m_pGUIController;
    // Visibility state of the menu
    int m_MenuEnabled;
    // Screen selection state
    int m_MenuScreen;
    // Change in menu screens detected
    bool m_ScreenChange;
    // Focus state
    int m_MainMenuFocus;
    // Focus change direction - 0 is none, negative is back, positive forward
    int m_FocusChange;
    // Speed at which the menus appear and disappear
    float m_MenuSpeed;
    // Which item in the currently focused list box we have selected
    int m_ListItemIndex;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;
	// Whether Pioneer Promo click-area is active
	bool m_PioneerPromoVisible;
	// Location of pioneer promo image
	Box m_PioneerPromoBox;

    // Collection box of the buy GUIs
    GUICollectionBox *m_apScreenBox[SCREENCOUNT];
    // The main menu buttons
    GUIButton *m_MainMenuButtons[MAINMENUBUTTONCOUNT];
    // Skirmish scene selction box
    GUIComboBox *m_pSceneSelector;
    // The skirmish setup screen team box panels
    GUICollectionBox *m_pTeamBox;
    GUICollectionBox *m_aSkirmishBox[SKIRMISHPLAYERCOUNT];
    // The skirmish setup screen buttons
    GUIButton *m_aSkirmishButton[SKIRMISHPLAYERCOUNT];
    // Label describing the CPU team
    GUILabel *m_pCPUTeamLabel;
    // The options buttons
    GUIButton *m_aOptionButton[OPTIONSBUTTONCOUNT];
    // Labels of the options screen
    GUILabel *m_aOptionsLabel[OPTIONSLABELCOUNT];
	// Labels of the options screen
	GUILabel *m_aDeadZoneLabel[DEADZONESLIDERCOUNT];
	// Slider for dead zone controls
	GUISlider *m_aDeadZoneSlider[DEADZONESLIDERCOUNT];
	// Checkboxes for dead zone controls
	GUICheckbox *m_aDeadZoneCheckbox[DEADZONESLIDERCOUNT];
    // Checkboxes of the options screen
    GUICheckbox *m_aOptionsCheckbox[OPTIONSCHECKBOXCOUNT];
    // Resolution combobox
    GUIComboBox *m_pResolutionCombo;
    // Option sound sliders
    GUILabel *m_pSoundLabel;
    GUILabel *m_pMusicLabel;
    GUISlider *m_pSoundSlider;
    GUISlider *m_pMusicSlider;
    // Back to options from the test and config screens
    GUIButton *m_pBackToOptionsButton;
    // Skip button for config screen
    GUIButton *m_pSkipButton;
    // Defualt button for config screen
    GUIButton *m_pDefaultButton;
    // Which player's control scheme we are currently configuring
    int m_ConfiguringPlayer;
    // Which type of device we are currently configuring
    int m_ConfiguringDevice;
    // Which type of gamepad we are currently configuring
    int m_ConfiguringGamepad;
    // Which step in current configure sequence
    int m_ConfigureStep;
    // Labels of the control config screen
    GUILabel *m_pConfigLabel[CONFIGLABELCOUNT];
    // The editor buttons
    GUIButton *m_aEditorButton[EDITORBUTTONCOUNT];
    // Metagame notice label
    GUILabel *m_pMetaNoticeLabel;
	GUILabel *m_VersionLabel; //!< CCCP version number.

    // Controller diagram bitmaps
    BITMAP **m_aDPadBitmaps;
    BITMAP **m_aDualAnalogBitmaps;
    // Controller diagram panel
    GUICollectionBox *m_pRecommendationBox;
    GUICollectionBox *m_pRecommendationDiagram;
    // Skip forward one config step button
    GUIButton *m_pConfigSkipButton;
    // Go back one config step button
    GUIButton *m_pConfigBackButton;

    // Gamepad type selection UI elements
    GUICollectionBox *m_pDPadTypeBox;
    GUICollectionBox *m_pDAnalogTypeBox;
    GUICollectionBox *m_pXBox360TypeBox;
    GUICollectionBox *m_pDPadTypeDiagram;
    GUICollectionBox *m_pDAnalogTypeDiagram;
    GUICollectionBox *m_pXBox360TypeDiagram;
    GUIButton *m_pDPadTypeButton;
    GUIButton *m_pDAnalogTypeButton;
    GUIButton *m_pXBox360TypeButton;

    // Panel behind editor menu to be resized depending on which editors are available
    GUICollectionBox *m_pEditorPanel;
    // Scrolling panel for the credits
    GUICollectionBox *m_pScrollPanel;
	GUILabel *m_CreditsLabel; //!< The label containing all the credits text.
    // Timer for credits scrolling pacing
    Timer m_ScrollTimer;

    GUIButton *m_pModManagerBackButton;
	GUIButton *m_pModManagerToggleModButton;
	GUIButton *m_pModManagerToggleScriptButton;
	GUIListBox *m_pModManagerModsListBox;
	GUIListBox *m_pModManagerScriptsListBox;
	GUILabel *m_pModManagerDescriptionLabel;

	GUICollectionBox *m_ResolutionChangeDialog;
	GUIButton *m_ButtonConfirmResolutionChange;
	GUIButton *m_ButtonCancelResolutionChange;
	GUIButton *m_ButtonConfirmResolutionChangeFullscreen;
	bool m_ResolutionChangeToUpscaled;

	std::vector<ModRecord> m_KnownMods;
	std::vector<ScriptRecord> m_KnownScripts;

    // Whether Scenario mode was started
    bool m_ScenarioStarted;
    // Whether Campaign mode was started
    bool m_CampaignStarted;
    // Whether the game was restarted this frame or not
    bool m_ActivityRestarted;
    // Whether the game was resumed this frame or not
    bool m_ActivityResumed;
    // Whether the player has been offered a tutoral yet this program run
    bool m_TutorialOffered;
    // How many players are chosen to be in the new game
    int m_StartPlayers;
    // How many teams are chosen to be in the new game
    int m_StartTeams;
    // How much money both teams start with in the new game
    int m_StartFunds;
    // How many teams are chosen to be in the new game
    int m_aTeamAssignments[SKIRMISHPLAYERCOUNT];
    // Which team is CPU managed, if any (-1)
    int m_CPUTeam;
    // Difficulty setting
    int m_StartDifficulty;
    // Player selected to quit the program
    bool m_Quit;

	// Max available resolutions.
	int m_MaxResX;
	int m_MaxResY;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MainMenuGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    /// <summary>
    /// Loads "Editor Scene" and starts the given editor activity
    /// </summary>
    /// <param name="editorActivity"></param>
    void StartEditorActivity(EditorActivity *editorActivity);


    // Disallow the use of some implicit methods.
	MainMenuGUI(const MainMenuGUI &reference) = delete;
	MainMenuGUI & operator=(const MainMenuGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File