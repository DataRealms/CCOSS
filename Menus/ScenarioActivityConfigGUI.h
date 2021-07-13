#ifndef _RTESCENARIOACTIVITYCONFIGGUI_
#define _RTESCENARIOACTIVITYCONFIGGUI_

#include "Activity.h"

namespace RTE {

	class GameActivity;
	class AllegroBitmap;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class GUIEvent;

	/// <summary>
	/// Handling for the scenario Activity configuration screen composition and interaction.
	/// </summary>
	class ScenarioActivityConfigGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ScenarioActivityConfigGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this ScenarioActivityConfigGUI. Ownership is NOT transferred!</param>
		explicit ScenarioActivityConfigGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether this ScenarioActivityConfigGUI is currently visible and enabled.
		/// </summary>
		/// <returns>Whether this ScenarioActivityConfigGUI is currently visible and enabled.</returns>
		bool IsEnabled() const;

		/// <summary>
		/// Enables or disables the ScenarioActivityConfigGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the ScenarioActivityConfigGUI.</param>
		/// <param name="selectedActivity">Pointer to the Activity this ScenarioActivityConfigGUI will be configuring for.</param>
		/// <param name="selectedScene">Pointer to the Scene the passed in Activity will be using.</param>
		void SetEnabled(bool enable, const Activity *selectedActivity = nullptr, Scene *selectedScene = nullptr);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the ScenarioActivityConfigGUI state.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
		/// <returns>Whether the player started a new game through the ScenarioActivityConfigGUI.</returns>
		bool Update(int mouseX, int mouseY);

		/// <summary>
		/// Draws the ScenarioActivityConfigGUI to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for all the player columns in the player setup box. "Extends" the Players enumeration by adding an entry for the CPU player.
		/// </summary>
		enum PlayerColumns { PlayerCPU = Players::MaxPlayerCount, PlayerColumnCount };

		/// <summary>
		/// Enumeration for all the team rows in the player setup box. "Extends" the Teams enumeration by adding an entry for unused (disabled) Team.
		/// </summary>
		enum TeamRows { DisabledTeam = Activity::Teams::MaxTeamCount, TeamRowCount };

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		const GameActivity *m_SelectedActivity; //!< The Activity this ScenarioActivityConfigGUI is configuring.
		Scene *m_SelectedScene; //!< The Scene the selected Activity will be using.
		int m_LockedCPUTeam = Activity::Teams::NoTeam; //!< Which team the CPU is locked to, if any.

		bool m_StartingGoldAdjustedManually; //!< Whether the player adjusted the starting gold, meaning it should stop automatically adjusting to the difficulty setting default starting gold where applicable.

		Timer m_StartGameButtonBlinkTimer; //!< Timer for blinking the start game button.

		bool m_TechListFetched; //!< Whether the tech list was fetched and each team's ComboBox was populated with it, even if no valid tech modules were added.

		/// <summary>
		/// GUI elements that compose the Activity setup box.
		/// </summary>
		GUICollectionBox *m_ActivityConfigBox;
		GUILabel *m_StartErrorLabel;
		GUIButton *m_StartGameButton;
		GUIButton *m_CancelConfigButton;
		GUILabel *m_ActivityDifficultyLabel;
		GUISlider *m_ActivityDifficultySlider;
		GUILabel *m_StartingGoldLabel;
		GUISlider *m_StartingGoldSlider;
		GUICheckbox *m_RequireClearPathToOrbitCheckbox;
		GUICheckbox *m_FogOfWarCheckbox;
		GUICheckbox *m_DeployUnitsCheckbox;
		GUILabel *m_CPULockLabel;
		GUICollectionBox *m_PlayersAndTeamsConfigBox;
		std::array<GUICollectionBox *, TeamRows::TeamRowCount> m_TeamIconBoxes;
		std::array<GUILabel *, TeamRows::TeamRowCount> m_TeamNameLabels;
		std::array<std::array<GUICollectionBox *, TeamRows::TeamRowCount>, PlayerColumns::PlayerColumnCount> m_PlayerBoxes;
		std::array<GUIComboBox *, Activity::Teams::MaxTeamCount> m_TeamTechComboBoxes;
		std::array<GUILabel *, Activity::Teams::MaxTeamCount> m_TeamAISkillLabels;
		std::array<GUISlider *, Activity::Teams::MaxTeamCount> m_TeamAISkillSliders;

#pragma region Activity Configuration Screen Handling
		/// <summary>
		/// Fills each team's Tech ComboBox with all valid Tech DataModules.
		/// </summary>
		void PopulateTechComboBoxes();

		/// <summary>
		/// Resets the configuration screen to the selected Activity's default settings and enables/disables attribute settings accordingly, making the configuration screen ready for interaction.
		/// </summary>
		void ResetActivityConfigBox();

		/// <summary>
		/// Sets up and starts the currently selected Activity with the configured settings.
		/// </summary>
		void StartGame();

		/// <summary>
		/// Updates the starting gold slider to the Activity difficulty setting (when applicable) and updates the value in the label according to the value in the slider.
		/// </summary>
		/// <returns></returns>
		void UpdateStartingGoldSliderAndLabel();

		/// <summary>
		/// Updates the currently hovered cell in the players and teams config box to apply the hovered visual and removes the hovered visual from any other cells. Also handles clicking on cells.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
		void UpdatePlayerTeamSetupCell(int mouseX, int mouseY);

		/// <summary>
		/// Handles the player interaction with a cell in the players and teams config box.
		/// </summary>
		/// <param name="clickedPlayer">The player box that was clicked.</param>
		/// <param name="clickedTeam">The team box that was clicked.</param>
		void HandleClickOnPlayerTeamSetupCell(int clickedPlayer, int clickedTeam);

		/// <summary>
		/// Handles the player interaction with the ScenarioActivityConfigGUI GUI elements.
		/// </summary>
		/// <returns>Whether the player started a new game through the ScenarioActivityConfigGUI.</returns>
		bool HandleInputEvents();
#pragma endregion

		// Disallow the use of some implicit methods.
		ScenarioActivityConfigGUI(const ScenarioActivityConfigGUI &reference) = delete;
		ScenarioActivityConfigGUI & operator=(const ScenarioActivityConfigGUI &rhs) = delete;
	};
}
#endif