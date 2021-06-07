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
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this ScenarioActivityConfigGUI.</param>
		explicit ScenarioActivityConfigGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets whether this Activity configuration menu is currently visible and enabled.
		/// </summary>
		/// <returns></returns>
		bool IsEnabled() const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="enable"></param>
		/// <param name="selectedActivity"></param>
		/// <param name="selectedScene"></param>
		void SetEnabled(bool enable, const Activity *selectedActivity = nullptr, Scene *selectedScene = nullptr);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the ScenarioActivityConfigGUI state.
		/// </summary>
		/// <param name="mouseX">Mouse X position.</param>
		/// <param name="mouseY">Mouse Y position.</param>
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

		const GameActivity *m_SelectedActivity; //!<
		Scene *m_SelectedScene; //!<
		int m_LockedCPUTeam = Activity::Teams::NoTeam; //!< Which team the CPU is locked to, if any.

		bool m_StartingGoldAdjustedManually; //!<

		Timer m_BlinkTimer; //!< Notification blink timer.

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
		/// 
		/// </summary>
		void PopulateTechComboBoxes();

		/// <summary>
		/// Shows the player configuration box.
		/// </summary>
		void ResetActivityConfigBox();

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateStartingGoldSliderAndLabel();

		/// <summary>
		/// Sets up and starts the currently selected Activity and settings.
		/// </summary>
		void StartGame();

		/// <summary>
		/// Handles player and team selection boxes in the player configuration box.
		/// </summary>
		/// <param name="clickedPlayer">The player box that was clicked.</param>
		/// <param name="clickedTeam">The team box that was clicked.</param>
		void ClickInPlayerSetup(int clickedPlayer, int clickedTeam);

		/// <summary>
		/// Handles the player interaction with the ScenarioActivityConfigGUI GUI elements.
		/// </summary>
		bool HandleInputEvents();
#pragma endregion

		// Disallow the use of some implicit methods.
		ScenarioActivityConfigGUI(const ScenarioActivityConfigGUI &reference) = delete;
		ScenarioActivityConfigGUI & operator=(const ScenarioActivityConfigGUI &rhs) = delete;
	};
}
#endif