#ifndef _RTESCENARIOGUI_
#define _RTESCENARIOGUI_

#include "ScenarioActivityConfigGUI.h"
#include "MOSParticle.h"

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
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!</param>
		ScenarioGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) { Clear(); Create(guiScreen, guiInput); }

		/// <summary>
		/// Makes the ScenarioGUI object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!</param>
		void Create(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Setters
		/// <summary>
		/// Enables the GUI elements for the menu, sets the planet coordinates on the screen so the menu can properly overlay it, and fetches the Scenes and Activities lists.
		/// </summary>
		/// <param name="center">The absolute screen coordinates of the planet's center.</param>
		/// <param name="radius">The radius, in screen pixel units, of the planet.</param>
		void SetEnabled(const Vector &center, float radius);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the ScenarioGUI state.
		/// </summary>
		/// <returns>The result of the ScenarioGUI input and event update. See ScenarioMenuUpdateResult enumeration.</returns>
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
		const Activity *m_SelectedActivity; //!< The currently selected Activity. Not owned.

		std::vector<Scene *> *m_ActivityScenes; //!< Pointer to the current set of Scenes being displayed. Not owned, and neither are the Scenes.
		Scene *m_SelectedScene; //!< The scene preset currently selected. Not owned.
		Scene *m_HoveredScene; //!< The scene preset currently hovered. Not owned.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		float m_PlanetRadius; //!< The screen radius of the planet.
		std::vector<Vector> m_LineToSitePoints; //!< Collection of points that form lines from a screen point to the selected site point.

		GUICollectionBox *m_DraggedBox; //!< Currently dragged GUI box.
		Vector m_PrevMousePos; //!< Previous position of the mouse to calculate dragging.

		Timer m_BlinkTimer; //!< Timer for blinking the resume and config start buttons.
		Timer m_ScenePreviewAnimTimer; //!< Timer for animating the Scene preview image.

		MOSParticle m_DefaultScenePreview; //!< MOSParticle that acts as the default Scene preview image, to avoid having to animate manually.
		bool m_DrawDefaultScenePreview; //!< Whether the default Scene preview should be drawn or not.

		std::unique_ptr<ScenarioActivityConfigGUI> m_ActivityConfigBox; //!< The Activity configuration box.

		/// <summary>
		/// GUI elements that compose the scenario menu screen.
		/// </summary>
		GUICollectionBox *m_RootBox;
		GUICollectionBox *m_ActivityConfigBoxRootBox;
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
		/// Sets the CollectionBox that is currently being dragged, if applicable.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
		void SetDraggedBox(int mouseX, int mouseY);

		/// <summary>
		/// Sets the selected Activity, refreshes the compatible Scenes on the planet and updates the Activity info box appropriately.
		/// </summary>
		/// <param name="newSelectedActivity">The new selected Activity.</param>
		void SetSelectedActivity(const Activity *newSelectedActivity);

		/// <summary>
		/// Sets the currently selected Scene and updates the Scene info box appropriately.
		/// </summary>
		/// <param name="newSelectedScene">The new selected Scene.</param>
		void SetSelectedScene(Scene *newSelectedScene);

		/// <summary>
		/// Moves the CollectionBox that is selected as being dragged, if any.
		/// </summary>
		/// <param name="mouseX">Mouse X position to calculate box position.</param>
		/// <param name="mouseY">Mouse Y position to calculate box position.</param>
		void DragBox(int mouseX, int mouseY);

		/// <summary>
		/// Fetches all the available Scenes and Activity presets from PresetMan.
		/// </summary>
		void FetchActivitiesAndScenesLists();

		/// <summary>
		/// Adjusts the positions of the site points on the planet if they don't fit the screen or overlap.
		/// </summary>
		/// <param name="sceneList">Vector of Scenes to adjust positions for.</param>
		void AdjustSitePointOffsetsOnPlanet(const std::vector<Scene *> &sceneList) const;

		/// <summary>
		/// Calculates how to draw lines from the Scene info box to the selected site point on the planet.
		/// </summary>
		void CalculateLinesToSitePoint();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Displays the site name label if the mouse is over a site point.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
		void UpdateHoveredSitePointLabel(int mouseX, int mouseY);

		/// <summary>
		/// Handles the player interaction with the ScenarioGUI GUI elements.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
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
		/// Clears all the member variables of this ScenarioGUI, effectively resetting the members of this object.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI &reference) = delete;
		ScenarioGUI & operator=(const ScenarioGUI &rhs) = delete;
	};
}
#endif