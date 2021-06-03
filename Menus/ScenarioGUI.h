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
	class GUIButton;
	class GUILabel;

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
		ScenarioGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) { Clear(); Create(guiScreen, guiInput); }

		/// <summary>
		/// Makes the ScenarioGUI object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager.</param>
		void Create(AllegroScreen *guiScreen, AllegroInput *guiInput);
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
		/// Updates the ScenarioGUI state.
		/// </summary>
		/// <returns></returns>
		ScenarioMenuUpdateResult Update();

		/// <summary>
		/// Draws the ScenarioGUI to the screen.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the ScenarioGUI.
		ScenarioMenuUpdateResult m_UpdateResult; //!< The result of the ScenarioGUI update. See ScenarioMenuUpdateResult enumeration.

		std::map<Activity *, std::vector<Scene *>> m_ScenarioActivities; //!< The map of Activities and the Scenes compatible with each, neither of which are owned here.
		const Activity *m_SelectedActivity; //!< The currently selected activity. Not owned.

		std::vector<Scene *> *m_ActivityScenes; //!< Pointer to the current set of Scenes being displayed. Not owned, and neither are the scenes.
		Scene *m_SelectedScene; //!< The scene preset currently selected. Not owned.
		Scene *m_HoveredScene; //!< The scene preset currently hovered. Not owned.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		float m_PlanetRadius; //!< The screen radius of the planet.
		std::vector<Vector> m_LineToSitePoints; //!< Collection of points that form lines from a screen point to the selected site point.

		GUICollectionBox *m_DraggedBox; //!< Currently dragged GUI box.
		Vector m_PrevMousePos; //!< Previous position of the mouse to calculate dragging.

		Timer m_BlinkTimer; //!< Notification blink timer.

		std::unique_ptr<ScenarioActivityConfigGUI> m_ActivityConfigBox; //!<

		/// <summary>
		/// GUI elements that compose the scenario menu screen.
		/// </summary>
		GUICollectionBox *m_RootBox;
		GUIButton *m_BackToMainButton;
		GUIButton *m_ResumeButton;
		GUICollectionBox *m_ActivityInfoBox;
		GUIComboBox *m_ActivitySelectComboBox;
		GUILabel *m_ActivityDescriptionLabel;
		GUICollectionBox *m_SceneInfoBox;
		GUIButton *m_SceneBoxCloseButton;
		GUILabel *m_SceneNameLabel;
		GUILabel *m_SceneDescriptionLabel;
		GUICollectionBox *m_ScenePreviewImageBox;
		std::unique_ptr<AllegroBitmap> m_DefaultPreviewBitmap;
		std::unique_ptr<AllegroBitmap> m_ScenePreviewBitmap;
		GUIButton *m_StartActivityConfigButton;
		GUILabel *m_SitePointNameLabel;

#pragma region Create Breakdown
		/// <summary>
		/// Creates all the elements that compose the Activity info box.
		/// </summary>
		void CreateActivityInfoBox();

		/// <summary>
		/// Creates all the elements that compose the Scene info box.
		/// </summary>
		void CreateSceneInfoBox();
#pragma endregion

#pragma region Scenario Menu Handling
		/// <summary>
		/// 
		/// </summary>
		/// <param name="mouseX"></param>
		/// <param name="mouseY"></param>
		void SetDraggedBox(int mouseX, int mouseY);

		/// <summary>
		/// Updates the contents of the Activity selection box.
		/// </summary>
		/// <param name="newSelectedActivity"></param>
		void SetSelectedActivity(const Activity *newSelectedActivity);

		/// <summary>
		/// Sets the currently selected scene.
		/// </summary>
		/// <param name="newSelectedScene">The new selected scene or nullptr to deselect the current selection.</param>
		void SetSelectedScene(Scene *newSelectedScene);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="mouseX"></param>
		/// <param name="mouseY"></param>
		void DragBox(int mouseX, int mouseY);

		/// <summary>
		/// Gathers all the available Scenes and Activity presets there are.
		/// </summary>
		void PopulateActivitiesAndScenesLists();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sceneList"></param>
		void AdjustSitePointOffsetsOnPlanet(const std::list<Scene *> &sceneList) const;

		/// <summary>
		/// Calculates how to draw lines from the Scene info box to the selected site point on the planet.
		/// </summary>
		void CalculateLinesToSitePoint();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Displays the site's name label if the mouse is over a site point. Otherwise the label is hidden.
		/// </summary>
		/// <param name="mouseX"></param>
		/// <param name="mouseY"></param>
		void UpdateHoveredScene(int mouseX, int mouseY);

		/// <summary>
		/// Handles the player interaction with the ScenarioGUI GUI elements.
		/// </summary>
		/// <param name="mouseX"></param>
		/// <param name="mouseY"></param>
		void HandleInputEvents(int mouseX, int mouseY);
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Draws the site points on top of the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw on.</param>
		void DrawSitePoints(BITMAP *drawBitmap) const;

		/// <summary>
		/// Draws fancy thick flickering lines from the Scene info box to the selected scene point on the planet.
		/// </summary>
		/// <param name="drawBitmap">The bitmap to draw to.</param>
		void DrawLinesToSitePoint(BITMAP *drawBitmap) const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ScenarioGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI &reference) = delete;
		ScenarioGUI & operator=(const ScenarioGUI &rhs) = delete;
	};
}
#endif