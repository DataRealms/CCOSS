#ifndef _RTESCENARIOACTIVITYCONFIGGUI_
#define _RTESCENARIOACTIVITYCONFIGGUI_

#include "Activity.h"

namespace RTE {

	class AllegroBitmap;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUIButton;
	class GUILabel;
	class GUISlider;

	/// <summary>
	/// 
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

#pragma region Setters

#pragma endregion

#pragma region Concrete Methods

#pragma endregion

	private:

		/// <summary>
		/// Enumeration for all the player columns in the player setup screen. "Extends" the Players enumeration by adding an entry for the CPU player.
		/// </summary>
		enum PlayerColumns {
			PlayerCPU = Players::MaxPlayerCount,
			PlayerColumnCount
		};

		/// <summary>
		/// Enumeration for all the team rows in the player setup screen. "Extends" the Teams enumeration by adding an entry for unused (disabled) Team.
		/// </summary>
		enum TeamRows {
			DisabledTeam = Activity::Teams::MaxTeamCount,
			TeamRowCount
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		int m_LockedCPUTeam = Activity::Teams::NoTeam; //!< Which team the CPU is locked to, if any.

		/// <summary>
		/// GUI elements that compose the Activity setup box.
		/// </summary>
		GUICollectionBox *m_ActivitySetupBox;
		GUILabel *m_ActivityDifficultyLabel;
		GUISlider *m_ActivityDifficultySlider;
		std::array<std::array<GUICollectionBox *, TeamRows::TeamRowCount>, PlayerColumns::PlayerColumnCount> m_PlayerBoxes;
		std::array<GUICollectionBox *, TeamRows::TeamRowCount> m_TeamBoxes;
		std::array<GUILabel *, TeamRows::TeamRowCount> m_TeamNameLabels;
		GUILabel *m_StartErrorLabel;
		GUILabel *m_CPULockLabel;
		GUILabel *m_GoldLabel;
		GUISlider *m_GoldSlider;
		GUICheckbox *m_FogOfWarCheckbox;
		GUICheckbox *m_RequireClearPathToOrbitCheckbox;
		GUICheckbox *m_DeployUnitsCheckbox;
		std::array<GUIComboBox *, Activity::Teams::MaxTeamCount> m_TeamTechSelect;
		std::array<GUISlider *, Activity::Teams::MaxTeamCount> m_TeamAISkillSlider;
		std::array<GUILabel *, Activity::Teams::MaxTeamCount> m_TeamAISkillLabel;

#pragma region Activity Config Screen Handling
		/// <summary>
		/// Shows the player configuration box.
		/// </summary>
		//void ShowPlayersBox();

		/// <summary>
		/// Updates the contents of the player configuration box.
		/// </summary>
		//void UpdatePlayersBox();

		/// <summary>
		/// Handles player and team selection boxes in the player configuration box.
		/// </summary>
		/// <param name="clickedPlayer">The player box that was clicked.</param>
		/// <param name="clickedTeam">The team box that was clicked.</param>
		//void ClickInPlayerSetup(int clickedPlayer, int clickedTeam);

		/// <summary>
		/// Sets up and starts the currently selected Activity and settings.
		/// </summary>
		/// <returns>Whether the game was set up and started successfully.</returns>
		//bool StartGame();
#pragma endregion

		// Disallow the use of some implicit methods.
		ScenarioActivityConfigGUI(const ScenarioActivityConfigGUI &reference) = delete;
		ScenarioActivityConfigGUI & operator=(const ScenarioActivityConfigGUI &rhs) = delete;
	};
}
#endif