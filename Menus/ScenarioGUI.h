#ifndef _SCENARIOGUI_
#define _SCENARIOGUI_

#include "ActivityMan.h"
#include "Timer.h"

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
	class AllegroBitmap;

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
		explicit ScenarioGUI(Controller *pController);

		/// <summary>
		/// Destructor method used to clean up a ScenarioGUI object before deletion from system memory.
		/// </summary>
		~ScenarioGUI() = default;

		/// <summary>
		/// Enables the menu. This will animate it in and out of view.
		/// </summary>
		void SetEnabled();

		/// <summary>
		/// Sets where the planet is on the screen and its other data so the menu can overlay properly on it.
		/// </summary>
		/// <param name="center">The absolute screen coordinates of the planet's center.</param>
		/// <param name="radius">The radius, in screen pixel units, of the planet.</param>
		void SetPlanetInfo(const Vector &center, float radius);

		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Updates the user input processing.
		/// </summary>
		ScenarioUpdateResult UpdateInput();

		/// <summary>
		/// Draws the site points on top of the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void DrawSitePoints(BITMAP *drawBitmap) const;

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;

	private:

		/// <summary>
		/// Hides all menu screens, so a single screen can be unhidden and shown alone.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// Makes sure a specific box doesn't move off-screen.
		/// </summary>
		/// <param name="screenBox">The GUICollectionBox to adjust, if necessary.</param>
		void KeepBoxOnScreen(GUICollectionBox &screenBox) const;

		/// <summary>
		/// Updates the contents of the Activity selection box.
		/// </summary>
		void UpdateActivityBox();

		/// <summary>
		/// Shows the Scene info box.
		/// </summary>
		void ShowScenesBox();

		/// <summary>
		/// Hides the Scene info box.
		/// </summary>
		void HideScenesBox() const;

		/// <summary>
		/// Sets the currently selected scene.
		/// </summary>
		/// <param name="newSelectedScene">The new selected scene.</param>
		void SetSelectedScene(Scene *newSelectedScene);

		/// <summary>
		/// Unsets the currently selected scene.
		/// </summary>
		void UnselectScene();

		/// <summary>
		/// Shows the player config box.
		/// </summary>
		void ShowPlayersBox();

		/// <summary>
		/// Updates the contents of the player config box.
		/// </summary>
		void UpdatePlayersBox();

		/// <summary>
		/// Sets up and starts the currently selected Activity and settings.
		/// </summary>
		/// <returns>Whether the game was set up and started successfully.</returns>
		bool StartGame();

		/// <summary>
		/// Gathers all the available Scenes and Activity presets there are.
		/// </summary>
		void GetAllScenesAndActivities(bool selectTutorial);

		/// <summary>
		/// Updates the floating label over a planet site.
		/// </summary>
		/// <param name="text">Text to show above the location.</param>
		/// <param name="location">The location in planetary coords (relative to the planet center).</param>
		void UpdateSiteNameLabel(const string &text = "", const Vector &location = Vector());

		/// <summary>
		/// Draws fancy thick flickering lines to point out the selected scene point on the planet, from the scene info box.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		void DrawWhiteScreenLineToSitePoint(BITMAP *drawBitmap) const;

		/// <summary>
		/// Draws a fancy thick flickering line to point out scene points on the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		/// <param name="start">The start Vector for the line, in absolute screen coordinates.</param>
		/// <param name="end">The end Vector for the line, in absolute screen coordinates.</param>
		/// <param name="color">The color to draw the line in. Use makecol(r, g, b) to create the color.</param>
		void DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const;

		/// <summary>
		/// Calculates how to draw lines from the scene info box to the selected site point on the planet.
		/// </summary>
		void CalculateLinesToSitePoint();

		enum ScenarioButtons {
			BACKTOMAINBUTTON = 0,
			STARTHERE,
			STARTGAME,
			RESUME,
			SCENARIOBUTTONCOUNT
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

		enum ScreenBox {
			ROOTSCREEN = 0,
			ACTIVITY,
			SCENEINFO,
			PLAYERSETUP,
			SCREENCOUNT
		};

		Controller *m_ScenarioController; //!< Controller which controls this menu. Not owned.
		std::unique_ptr<GUIScreen> m_ScenarioGUIScreen; //!< GUI Screen for use by the in-game GUI.
		std::unique_ptr<GUIInput> m_ScenarioGUIInput; //!< Input controller.
		std::unique_ptr<GUIControlManager> m_ScenarioGUIController; //!< The control manager which holds all the controls.

		Timer m_BlinkTimer; //!< Notification blink timer.

		GUICollectionBox *m_ScenarioScreenBoxes[SCREENCOUNT]; //!< The different dialog/floating boxes.

		GUIButton *m_ScenarioButtons[SCENARIOBUTTONCOUNT]; //!< The menu buttons we want to manipulate.

		GUICollectionBox *m_ScenarioDraggedBox; //!< Currently dragged GUI box.
		Vector m_PrevMousePos; //!< Previous pos of mouse to calculate dragging.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		float m_PlanetRadius = 240.0F; //!< The screen radius of the planet.

		std::list<Scene *> *m_ScenarioScenes; //!< Pointer to the current set of Scenes being displayed - not owned, and neither are the scenes.
		Scene *m_ScenarioHoveredScene; //!< The scene preset currently hovered, NOT OWNED.
		GUILabel *m_ScenarioScenePlanetLabel; //!< Hover name label over Scenes.
		Scene *m_ScenarioSelectedScene; //!< The scene preset currently selected, NOT OWNED.

		std::vector<Vector> m_LinePointsToSite; //!< Collection of points that form lines from a screen point to the selected site point.

		// Activity selection.
		GUIComboBox *m_ActivitySelectComboBox;
		GUILabel *m_ActivityLabel;
		GUILabel *m_DifficultyLabel;
		GUISlider *m_DifficultySlider;

		std::map<Activity *, std::list<Scene *> > m_Activities; //!< The map of Activities, and the Scenes compatible with each, neither of which are owned here.
		const Activity *m_ScenarioSelectedActivity; //!< The currently selected activity, not owned.

		// Scene Info.
		GUIButton *m_SceneCloseButton;
		GUILabel *m_SceneNameLabel;
		GUILabel *m_SceneInfoLabel;

		std::unique_ptr<AllegroBitmap> m_ScenePreviewBitmap;
		std::unique_ptr<AllegroBitmap> m_DefaultPreviewBitmap;

		// Player setup.
		GUICollectionBox *m_PlayerBoxes[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
		GUICollectionBox *m_TeamBoxes[TEAMROWCOUNT];
		GUILabel *m_TeamNameLabels[TEAMROWCOUNT];
		GUILabel *m_StartErrorLabel;
		GUILabel *m_CPULockLabel;
		GUILabel *m_GoldLabel;
		GUISlider *m_GoldSlider;
		GUICheckbox *m_FogOfWarCheckbox;
		GUICheckbox *m_RequireClearPathToOrbitCheckbox;
		GUICheckbox *m_DeployUnitsCheckbox;
		GUIComboBox *m_TeamTechSelect[Activity::MaxTeamCount]; //!< Tech selection combobox array.
		GUISlider *m_TeamAISkillSlider[Activity::MaxTeamCount]; //!< AI skill slider array.
		GUILabel *m_TeamAISkillLabel[Activity::MaxTeamCount]; //!< AI skill label array.

		int m_LockedCPUTeam = Activity::NoTeam; //!< Which team the CPU is locked to, if any.

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI &reference) = delete;
		ScenarioGUI & operator=(const ScenarioGUI &rhs) = delete;
	};
}
#endif
