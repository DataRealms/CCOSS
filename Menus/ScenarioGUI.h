#ifndef _RTESCENARIOGUI_
#define _RTESCENARIOGUI_

#include "ScenarioActivityConfigGUI.h"
#include "Timer.h"

namespace RTE {

	class Activity;
	class Scene;
	class AllegroScreen;
	class AllegroInput;
	class AllegroBitmap;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUIButton;
	class GUILabel;
	class GUISlider;

	/// <summary>
	/// Handling for the scenario menu screen composition and sub-menu interaction.
	/// </summary>
	class ScenarioGUI {

	public:

		/// <summary>
		/// Enumeration for the results of the ScenarioGUI input and event update.
		/// </summary>
		enum class ScenarioMenuUpdateResult {
			NoEvent,
			BackToMain,
			ActivityResumed,
			ActivityStarted
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate this ScenarioGUI object in system memory.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager.</param>
		ScenarioGUI(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Setters
		/// <summary>
		/// Enables the menu by making the activity box visible and fetching the available activities and scenes.
		/// </summary>
		void SetEnabled();

		/// <summary>
		/// Sets where the planet is on the screen and its other data so the menu can overlay properly on it.
		/// </summary>
		/// <param name="center">The absolute screen coordinates of the planet's center.</param>
		/// <param name="radius">The radius, in screen pixel units, of the planet.</param>
		void SetPlanetInfo(const Vector &center, float radius);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		ScenarioMenuUpdateResult Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the GUIButtons of the Scenario GUI. Used to access elements in the Buttons array.
		/// </summary>
		enum ScenarioButtons {
			BackToMainButton,
			StartHereButton,
			StartGameButton,
			ResumeButton,
			ButtonCount
		};

		/// <summary>
		/// GUI elements that compose the Activity info and selection box.
		/// </summary>
		struct ActivityInfoBox {
			GUICollectionBox *ActivityInfoBox;
			GUIComboBox *ActivitySelectComboBox;
			GUILabel *ActivityDescriptionLabel;
			GUILabel *DifficultyLabel;
			GUISlider *DifficultySlider;
		};

		/// <summary>
		/// GUI elements that compose the Scene info and preview box.
		/// </summary>
		struct SceneInfoBox {
			GUICollectionBox *SceneInfoBox;
			GUIButton *SceneCloseButton;
			GUILabel *SceneNameLabel;
			GUILabel *SceneInfoLabel;
			GUICollectionBox *ScenePreviewBox;
			std::unique_ptr<AllegroBitmap> ScenePreviewBitmap;
			std::unique_ptr<AllegroBitmap> DefaultPreviewBitmap;
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the ScenarioGUI.
		ScenarioMenuUpdateResult m_UpdateResult; //!< The result of the ScenarioGUI update. See ScenarioMenuUpdateResult enumeration.

		std::array<GUIButton *, ScenarioButtons::ButtonCount> m_ScenarioButtons; //!< The menu buttons we want to manipulate.

		Timer m_BlinkTimer; //!< Notification blink timer.

		GUICollectionBox *m_DraggedBox; //!< Currently dragged GUI box.
		Vector m_PrevMousePos; //!< Previous position of the mouse to calculate dragging.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		float m_PlanetRadius; //!< The screen radius of the planet.

		std::vector<Scene *> *m_ScenarioScenes; //!< Pointer to the current set of Scenes being displayed. Not owned, and neither are the scenes.

		Scene *m_HoveredScene; //!< The scene preset currently hovered. Not owned.
		Scene *m_SelectedScene; //!< The scene preset currently selected. Not owned.
		std::vector<Vector> m_LineToSitePoints; //!< Collection of points that form lines from a screen point to the selected site point.

		std::map<Activity *, std::vector<Scene *>> m_ScenarioActivities; //!< The map of Activities and the Scenes compatible with each, neither of which are owned here.
		const Activity *m_SelectedActivity; //!< The currently selected activity. Not owned.

		int m_LockedCPUTeam = Activity::Teams::NoTeam; //!< Which team the CPU is locked to, if any.

		ActivityInfoBox m_ActivityInfoBox; //!<
		SceneInfoBox m_SceneInfoBox; //!<

		std::unique_ptr<ScenarioActivityConfigGUI> m_ActivitySetupBox; //!<

		/// <summary>
		/// GUI elements that compose the scenario menu screen.
		/// </summary>
		GUICollectionBox *m_RootBox;
		GUILabel *m_SitePointLabel; //!< Hover name label over Scenes.

#pragma region Create Breakdown
		/// <summary>
		/// 
		/// </summary>
		void CreateActivitySelectionBox();

		/// <summary>
		/// 
		/// </summary>
		void CreateSceneInfoBox();
#pragma endregion

#pragma region CollectionBox Handling
		/// <summary>
		/// Shows the Scene info box.
		/// </summary>
		void ShowScenesBox();

		/// <summary>
		/// Hides all menu screens, so a single screen can be unhidden and shown alone.
		/// </summary>
		void HideAllScreens();

		/// <summary>
		/// Makes sure a specific box doesn't move off-screen.
		/// </summary>
		/// <param name="screenBox">The GUICollectionBox to adjust, if necessary.</param>
		void KeepBoxInScreenBounds(GUICollectionBox *screenBox) const;
#pragma endregion

#pragma region Scenes and Activities Handling
		/// <summary>
		/// Gathers all the available Scenes and Activity presets there are.
		/// </summary>
		void GetScenesAndActivities(bool selectTutorial);

		/// <summary>
		/// Sets the currently selected scene.
		/// </summary>
		/// <param name="newSelectedScene">The new selected scene or nullptr to deselect the current selection.</param>
		void SetSelectedScene(Scene *newSelectedScene);
#pragma endregion

#pragma region Planet Site Handling
		/// <summary>
		/// Calculates how to draw lines from the scene info box to the selected site point on the planet.
		/// </summary>
		void CalculateLinesToSitePoint();

		/// <summary>
		/// Draws the site points on top of the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void DrawSitePoints(BITMAP *drawBitmap) const;

		/// <summary>
		/// Draws fancy thick flickering lines to point out the selected scene point on the planet, from the scene info box.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		void DrawLineToSitePoint(BITMAP *drawBitmap) const;
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Handles the player interaction with the ScenarioGUI GUI elements.
		/// </summary>
		/// <returns>The result of the user input and event update. See ScenarioUpdateResult enumeration.</returns>
		ScenarioMenuUpdateResult HandleInputEvents();

		/// <summary>
		/// Updates the contents of the Activity selection box.
		/// </summary>
		void UpdateActivityBox();

		/// <summary>
		/// Displays the site's name label if the mouse is over a site point. Otherwise the label is hidden.
		/// </summary>
		void UpdateHoveredScene(int mouseX, int mouseY);
#pragma endregion

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI &reference) = delete;
		ScenarioGUI & operator=(const ScenarioGUI &rhs) = delete;
	};
}
#endif