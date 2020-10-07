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

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class Scene;
	class Activity;

	/// <summary>
	/// A menu for setting up and launching scenario games.
	/// </summary>
	class ScenarioGUI {

	public:

		enum ScenarioUpdateResult {
			NOEVENT = 0,
			BACKTOMAIN,
			ACTIVITYRESUMED,
			ACTIVITYRESTARTED
		};

		/// <summary>
		/// Constructor method used to instantiate a ScenarioGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		ScenarioGUI() { Clear(); }

		/// <summary>
		/// Destructor method used to clean up a ScenarioGUI object before deletion from system memory.
		/// </summary>
		~ScenarioGUI() { Destroy(); }

		/// <summary>
		/// Makes the ScenarioGUI object ready for use.
		/// </summary>
		/// <param name="pController">A pointer to a Controller which will control this Menu. Ownership is NOT TRANSFERRED!</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *pController);

		/// <summary>
		/// Resets the entire ScenarioGUI, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ScenarioGUI object.
		/// </summary>
		void Destroy();

		/// <summary>
		/// Gets the GUIControlManager owned and used by this.
		/// </summary>
		/// <returns>The GUIControlManager. Ownership is not transferred!</returns>
		GUIControlManager *GetGUIControlManager();

		/// <summary>
		/// Enables or disables the menu. This will animate it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Reports whether the menu is enabled or not.
		/// </summary>
		/// <returns>Whether the menu is enabled or not.</returns>
		bool IsEnabled() { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }

		/// <summary>
		/// Sets where the planet is on the screen and its other data so the menu can overlay properly on it.
		/// </summary>
		/// <param name="center">The absolute screen coordinates of the planet's center.</param>
		/// <param name="radius">The radius, in screen pixel units, of the planet.</param>
		void SetPlanetInfo(const Vector &center, float radius) { m_PlanetCenter = center; m_PlanetRadius = radius; }

		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		ScenarioUpdateResult Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;

		/// <summary>
		/// Draws fancy thick flickering lines to point out scene points on the planet, from an arbitrary screen point.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		/// <param name="screenPoint">The point on the screen to point from, in screen coordinates.</param>
		/// <param name="planetPoint">The point on the planet to point at, in planet coordinates.</param>
		/// <param name="color">The color of the line.</param>
		/// <param name="onlyFirstSegments">How many of the segments from the start (the start of the line) to draw.</param>
		/// <param name="onlyLastSegments">How many of the segments from the end (site circle) to draw. -1 is all.</param>
		/// <param name="channelHeight">The height of the 'channel' above and below that the lines will go around the player bar.</param>
		/// <param name="circleSize">What size factor from 'normal' should the circle's diameter be drawn.</param>
		/// <param name="squareSite">Whether to draw the planet site as a square (true) or a circle (false).</param>
		/// <returns>Whether all segments of the line were drawn with the segment params.</returns>
		bool DrawScreenLineToSitePoint(BITMAP *drawBitmap, const Vector &screenPoint, const Vector &planetPoint, int color, int onlyFirstSegments = -1, int onlyLastSegments = -1,
			int channelHeight = 80, float circleSize = 1.0F, bool squareSite = false) const;

	protected:

		/// <summary>
		/// Updates the user input processing.
		/// </summary>
		ScenarioUpdateResult UpdateInput();

		/// <summary>
		/// Hides all menu screens, so a single screen can be unhidden and shown alone.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// Makes sure a specific box doesn't end up moved completely off-screen.
		/// </summary>
		/// <param name="pBox">The GUICollectionBox to adjust, if necessary.</param>
		/// <param name="margin">The amount of margin to allow the box to stay within.</param>
		void KeepBoxOnScreen(GUICollectionBox *pBox, int margin = 10);

		/// <summary>
		/// Updates the contents of the Activity selection box.
		/// </summary>
		void UpdateActivityBox();

		/// <summary>
		/// Updates the contents of the Scene selection box.
		/// </summary>
		void UpdateScenesBox();

		/// <summary>
		/// Updates the contents of the player config box.
		/// </summary>
		/// <param name="newActivity">Whether we shuold refresh completely because there's a new Activity selected.</param>
		void UpdatePlayersBox(bool newActivity);

		/// <summary>
		/// Counts how many players are currently assigned to play this Activity.
		/// </summary>
		/// <returns>The number of players already assigned to play the selected Activity.</returns>
		int PlayerCount() const;

		/// <summary>
		/// Sets up and starts the currently selected Activity and settings.
		/// </summary>
		/// <returns>Whether the game was set up and started successfully.</returns>
		bool StartGame();

		/// <summary>
		/// Gathers all the available Scene:s and Activity presets there are.
		/// </summary>
		void GetAllScenesAndActivities();

		/// <summary>
		/// Updates the floating label over a planet site.
		/// </summary>
		/// <param name="visible">Label is visible.</param>
		/// <param name="text">Text to show above the location.</param>
		/// <param name="location">The location in planetary coords.</param>
		/// <param name="height">How high above the location to show the text.</param>
		void UpdateSiteNameLabel(bool visible, const string &text = "", const Vector &location = Vector(), float height = 1.0F);

		/// <summary>
		/// Draws a fancy thick flickering line to point out scene points on the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		/// <param name="start">The start Vector for the line, in absolute screen coordinates.</param>
		/// <param name="end">The end Vector for the line, in absolute screen coordinates.</param>
		/// <param name="color">The color to draw the line in. Use makecol(r, g, b) to create the color.</param>
		void DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const;

		enum MenuEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		enum MenuScreen {
			ROOT = 0,
			SCENESELECT,
			PLAYERSETUP,
			CONFIRMQUIT,
			SCREENCOUNT
		};

		enum ScenarioButtons {
			BACKTOMAINBUTTON = 0,
			STARTHERE,
			STARTGAME,
			RESUME,
			SCENARIOBUTTONCOUNT
		};

		enum BlinkMode {
			NOBLINK = 0,
			NOFUNDS,
			NOCRAFT,
			BLINKMODECOUNT
		};

		// These add on the player and team max counts
		enum PlayerColumns {
			PLAYER_CPU = Players::MaxPlayerCount,
			PLAYERCOLUMNCOUNT
		};

		enum TeamRows {
			TEAM_DISABLED = Activity::MaxTeamCount,
			TEAMROWCOUNT
		};

		Controller *m_ScenarioController; //!< Controller which controls this menu. Not owned.
		
		GUIScreen *m_ScenarioGUIScreen; //!< GUI Screen for use by the in-game GUI.
		
		GUIInput *m_ScenarioGUIInput; //!< Input controller.
		
		GUIControlManager *m_ScenarioGUIController; //!< The control manager which holds all the controls.
		
		int m_MenuEnabled; //!< Visibility state of the menu.
		
		int m_MenuScreen; //!< Screen selection state.
		
		bool m_ScreenChange; //!< Change in menu screens detected.
		
		int m_SceneFocus; //!< Focus state on selecting scenes.
		
		int m_FocusChange; //!< Focus change direction - 0 is none, negative is back, positive forward.
		
		float m_MenuSpeed; //!< Speed at which the menus appear and disappear.
		
		Timer m_BlinkTimer; //!< Notification blink timer.
		
		int m_BlinkMode; //!< What we're blinking.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		
		float m_PlanetRadius; //!< The screen radius of the planet.

		// The different dialog/floating boxes
		GUICollectionBox *m_ScenarioRootBox;
		GUICollectionBox *m_ScenarioActivityBox;
		GUICollectionBox *m_ScenarioSceneInfoBox;
		GUICollectionBox *m_ScenarioPlayerSetupBox;
		GUICollectionBox *m_ScenarioQuitConfirmBox;

		GUIButton *m_ScenarioButtons[SCENARIOBUTTONCOUNT]; //!< The menu buttons we want to manipulate.
		
		GUILabel *m_ScenarioScenePlanetLabel; //!< Hover name label over Scenes.

		// Activity selection screen controls
		GUIComboBox *m_ActivitySelectComboBox;
		GUILabel *m_ActivityLabel;
		GUILabel *m_DifficultyLabel;
		GUISlider *m_DifficultySlider;

		// Scene Info controls
		GUIButton *m_SceneCloseButton;
		GUILabel *m_SceneNameLabel;
		GUILabel *m_SceneInfoLabel;

		// Player setup controls
		// Boxes in the matrix which detect and display where a control setting is and should be
	    //bool m_aaControls[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
		GUICollectionBox *m_PlayerBoxes[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
		GUICollectionBox *m_TeamBoxes[TEAMROWCOUNT];
		GUILabel *m_TeamNameLabels[TEAMROWCOUNT];
		GUILabel *m_StartErrorLabel;
		GUILabel *m_CPULockLabel;
		
		int m_LockedCPUTeam; //!< Which team the CPU is locked to, if any.

		GUIComboBox *m_TeamTechSelect[Activity::MaxTeamCount]; //!< Tech selection combos.

		// AI skill selection
		GUISlider *m_TeamAISkillSlider[Activity::MaxTeamCount];
		GUILabel *m_TeamAISkillLabel[Activity::MaxTeamCount];

		GUILabel *m_GoldLabel;
		GUISlider *m_GoldSlider;
		GUICheckbox *m_FogOfWarCheckbox;
		GUICheckbox *m_RequireClearPathToOrbitCheckbox;
		GUICheckbox *m_DeployUnitsCheckbox;

		// The confirmation box and its controls
		GUILabel *m_QuitConfirmLabel;
		GUIButton *m_QuitConfirmButton;

		BITMAP *m_ScenePreviewBitmap;
		BITMAP *m_DefaultPreviewBitmap;

		std::list<Scene *> *m_ScenarioScenes; //!< Pointer to the current set of Scenes being displayed - not owned, and neither are the scenes.
		
		std::map<Activity *, std::list<Scene *> > m_Activities; //!< The map of Activities, and the Scenes compatible with each, neither of which are owned here.
		
		GUICollectionBox *m_ScenarioDraggedBox; //!< Currently dragged GUI box.
		
		bool m_EngageDrag; //!< New potential drag is starting.
		
		Scene *m_ScenarioHoveredScene; //!< The scene preset currently hovered, NOT OWNED.
		
		Scene *m_ScenarioSelectedScene; //!< The scene preset currently selected, NOT OWNED.
		
		Vector m_PrevMousePos; //!< Previous pos of mouse to calculate dragging.

		bool m_SelectTutorial; //!< Select tutorial activity when switched to scenario GUI.

	private:

		/// <summary>
		/// Clears all the member variables of this ScenarioGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI &reference) {}
		ScenarioGUI &operator=(const ScenarioGUI &rhs) {}

	};
}
#endif
