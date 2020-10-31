#ifndef _SCENARIOGUI_
#define _SCENARIOGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ScenarioGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     ScenarioGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ActivityMan.h"
#include "Timer.h"

struct BITMAP;


namespace RTE
{

class GUIScreen;
class GUIInput;
class GUIControlManager;
class GUICollectionBox;
class GUIComboBox;
class GUICheckbox;
class GUITab;
class GUIListBox;
class GUITextBox;
class GUIButton;
class GUILabel;
class GUISlider;
class Entity;
class Scene;
class Activity;

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ScenarioGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A menu for setting up and launching scenario games.
// Parent(s):       None.
// Class history:   11/02/2010 ScenarioGUI Created.

class ScenarioGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ScenarioGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ScenarioGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ScenarioGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ScenarioGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ScenarioGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~ScenarioGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ScenarioGUI object ready for use.
// Arguments:       A pointer to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ScenarioGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ScenarioGUI object.
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
// Method:          SetPlanetInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where the planet is on the scren and its other data so the menu
//                  can overlay properly on it.
// Arguments:       The absolute screen coordinates of the planet's center.
//                  The radius, in screen pixel units, of the planet.
// Return value:    None.

    void SetPlanetInfo(const Vector &center, float radius) { m_PlanetCenter = center; m_PlanetRadius = radius; }


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
// Method:          BackToMain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to go back to the main menu.
// Arguments:       None.
// Return value:    Whether we should go back to main menu.

    bool BackToMain() { return m_BackToMain; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QuitProgram
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the player has decided to quit the program.
// Arguments:       None.
// Return value:    Whether the program has been commanded to shit down by the user.

    bool QuitProgram() { return m_Quit; }


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawScreenLineToSitePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering lines to point out scene points on the
//                  planet, FROM an arbitrary screen point.
// Arguments:       The bitmap to draw to.
//                  The point on the screen to point from, in screen coordinates.
//                  The point on the planet to point at, in planet coordinates.
//                  The color of the line.
//                  How many of the segments from the start (the start of the line) to draw.
//                  How many of the segments from the end (site circle) to draw. -1 is all.
//                  The height of the 'channel' above and below that the lines will go around
//                  the player bar.
//                  What size factor from 'normal' should the circle's diameter be drawn.
// Return value:    Whether all segments of the line were drawn with the segment params.

	bool DrawScreenLineToSitePoint(BITMAP *drawBitmap,
		const Vector &screenPoint,
		const Vector &planetPoint,
		int color,
		int onlyFirstSegments = -1,
		int onlyLastSegments = -1,
		int channelHeight = 80,
		float circleSize = 1.0,
		bool squareSite = false) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the user input processing.
// Arguments:       None.
// Return value:    None.

    void UpdateInput();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideAllScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides all menu screens, so one can easily be unhidden and shown only.
// Arguments:       None.
// Return value:    None.

    void HideAllScreens();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeepBoxOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure a specific box doesn't end up moved completely off-screen.
// Arguments:       The GUICollectionBox to adjust, if necessary.
//                  The amount of margin to allow the box to stay within.
// Return value:    None.

    void KeepBoxOnScreen(GUICollectionBox *pBox, int margin = 10);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSkirmishActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ActivityMan up with the current data for a skirmish game.
// Arguments:       None.
// Return value:    None.

    void SetupSkirmishActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateActivityBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the Activity selection box.
// Arguments:       None.
// Return value:    None.

    void UpdateActivityBox();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateScenesBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the Scene selection box.
// Arguments:       None.
// Return value:    None.

    void UpdateScenesBox();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayersBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the player config box.
// Arguments:       Whether we shuold refresh completely becaue there's a new Activity
//                  selected.
// Return value:    None.

    void UpdatePlayersBox(bool newActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Counts how many players are currently assigned to play this Activity.
// Arguments:       None.
// Return value:    The number of players already assigned to play the selected Activity.

    int PlayerCount();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up and starts the currently selected Activity and settings.
// Arguments:       None.
// Return value:    Whether the game was set up and started successfully.

    bool StartGame();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllScenesAndActivities
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers all the available Scene:s and Activity presets there are.
// Arguments:       None.
// Return value:    None.

    void GetAllScenesAndActivities();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSiteHoverLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the floating label over a planet site.
// Arguments:       Label is visible.
//                  Text to show above the location.
//                  The location in planetary coords.
//                  How high above the location to show the text, adjustment from a good default.
// Return value:    None.

    void UpdateSiteNameLabel(bool visible, string text = "", const Vector &location = Vector(), float height = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGlowLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering line to point out scene points on the
//                  planet.
// Arguments:       The bitmap to draw to.
//                  The start and end Vector:s for the line, in absolute screen coordinates.
//                  The color to draw the line in. Use makecol(r, g, b) to create the color
// Return value:    None.

    void DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const;



    enum MenuEnabled
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    enum MenuScreen
    {
        ROOT = 0,
        SCENESELECT,
        PLAYERSETUP,
        CONFIRMQUIT,
        SCREENCOUNT
    };

    enum ScenarioButtons
    {
        BACKTOMAIN = 0,
        STARTHERE,
        STARTGAME,
		RESUME,
        SCENARIOBUTTONCOUNT
    };

    enum BlinkMode
    {
        NOBLINK = 0,
        NOFUNDS,
        NOCRAFT,
        BLINKMODECOUNT
    };

    // These add on the player and team max counts
    enum PlayerColumns
    {
        PLAYER_CPU = Players::MaxPlayerCount,
        PLAYERCOLUMNCOUNT
    };

    enum TeamRows
    {
        TEAM_DISABLED = Activity::MaxTeamCount,
        TEAMROWCOUNT
    };

    // Controller which controls this menu. Not owned
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
    // Focus state on selecting scenes
    int m_SceneFocus;
    // Focus change direction - 0 is none, negative is back, positive forward
    int m_FocusChange;
    // Speed at which the menus appear and disappear
    float m_MenuSpeed;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;

    // The absolute screen position of the planet center
    Vector m_PlanetCenter;
    // The screen radius of the planet
    float m_PlanetRadius;

    // The different dialog/floating boxes
    GUICollectionBox *m_pRootBox;
    GUICollectionBox *m_pActivityBox;
    GUICollectionBox *m_pSceneInfoBox;
    GUICollectionBox *m_pPlayerSetupBox;
    GUICollectionBox *m_pQuitConfirmBox;

    // The menu buttons we want to manipulate
    GUIButton *m_aScenarioButton[SCENARIOBUTTONCOUNT];
    // Hover name label over Scene:s
    GUILabel *m_pScenePlanetLabel;

    // Activity selection screen controls
    GUIComboBox *m_pActivitySelect;
    GUILabel *m_pActivityLabel;
    GUILabel *m_pDifficultyLabel;
    GUISlider *m_pDifficultySlider;

    // Scene Info controls
    GUIButton *m_pSceneCloseButton;
    GUILabel *m_pSceneNameLabel;
    GUILabel *m_pSceneInfoLabel;

    // Player setup controls
    // Boxes in the matrix which detect and display where a control setting is and should be
//    bool m_aaControls[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
    GUICollectionBox *m_aapPlayerBoxes[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
    GUICollectionBox *m_apTeamBoxes[TEAMROWCOUNT];
    GUILabel *m_apTeamNameLabels[TEAMROWCOUNT];
    GUILabel *m_pStartErrorLabel;
    GUILabel *m_pCPULockLabel;
    // Which team the CPU is locked to, if any
    int m_LockedCPUTeam;

    //Tech selection combos
	GUIComboBox *m_apTeamTechSelect[Activity::MaxTeamCount];
	
	// AI skill selection
	GUISlider *m_apTeamAISkillSlider[Activity::MaxTeamCount];
	GUILabel *m_apTeamAISkillLabel[Activity::MaxTeamCount];

    GUILabel *m_pGoldLabel;
    GUISlider *m_pGoldSlider;
	GUICheckbox *m_pFogOfWarCheckbox;
	GUICheckbox *m_pRequireClearPathToOrbitCheckbox;
	GUICheckbox *m_pDeployUnitsCheckbox;

    // The confirmation box and its controls
    GUILabel *m_pQuitConfirmLabel;
    GUIButton *m_pQuitConfirmButton;

	BITMAP *m_pScenePreviewBitmap;
	BITMAP *m_pDefaultPreviewBitmap;

    // The current set of Scenes being displayed - not owned, nor are the scenes
    std::list<Scene *> *m_pScenes;
    // The map of Activity:ies, and the Scene:s compatible with each, neither of which are owned here
    std::map<Activity *, std::list<Scene *> > m_Activities;
    // Currently dragged GUI box
    GUICollectionBox *m_pDraggedBox;
    // New potential drag is starting
    bool m_EngageDrag;
    // The scene preset currently hovered, NOT OWNED
    Scene *m_pHoveredScene;
    // The scene preset currently selected, NOT OWNED
    Scene *m_pSelectedScene;
    // Previous pos of mouse to calculate dragging
    Vector m_PrevMousePos;

    // Whether the game was restarted this frame or not
    bool m_ActivityRestarted;
    // Whether the game was resumed this frame or not
    bool m_ActivityResumed;
    // How many players are chosen to be in the new game
    int m_StartPlayers;
    // How many teams are chosen to be in the new game
    int m_StartTeams;
    // How much money both teams start with in the new game
    int m_StartFunds;
    // Difficulty setting
    int m_StartDifficulty;
    // Whether user has chosen to go back to the main menu
    bool m_BackToMain;
    // Player selected to quit the program
    bool m_Quit;
	// Select tutorial activity when switched to scenario GUI
	bool m_SelectTutorial;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ScenarioGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	ScenarioGUI(const ScenarioGUI &reference) = delete;
	ScenarioGUI & operator=(const ScenarioGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File
