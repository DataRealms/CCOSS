#ifndef _RTEACTIVITY_
#define _RTEACTIVITY_

#include "Icon.h"
#include "Controller.h"

namespace RTE {

	class Scene;
	class Actor;

	/// <summary>
	/// Base class for all Activities, including game modes and editors.
	/// </summary>
	class Activity : public Entity {

	public:

		SerializableOverrideMethods;
		ClassInfoGetters;

		/// <summary>
		/// Enumeration for the different states the Activity can be in.
		/// </summary>
		enum ActivityState {
			NoActivity = -1,
			NotStarted = 0,
			Starting,
			Editing,
			PreGame,
			Running,
			HasError,
			Over
		};

		/// <summary>
		/// Enumeration for the different teams in an activity.
		/// </summary>
		enum Teams {
			NoTeam = -1,
			TeamOne = 0,
			TeamTwo,
			TeamThree,
			TeamFour,
			MaxTeamCount
		};

		/// <summary>
		/// Enumeration for the different observations states (camera viewpoint).
		/// </summary>
		enum ViewState {
			Normal = 0,
			Observe,
			DeathWatch,
			ActorSelect,
			AISentryPoint,
			AIPatrolPoints,
			AIGoldDigPoint,
			AIGoToPoint,
			LandingZoneSelect,
			UnitSelectCircle,
		};

		/// <summary>
		/// Enumeration for the different difficulty settings.
		/// </summary>
		enum DifficultySetting {
			MinDifficulty = 0,
			CakeDifficulty = 20,
			EasyDifficulty = 40,
			MediumDifficulty = 60,
			HardDifficulty = 80,
			NutsDifficulty = 95,
			MaxDifficulty = 100,
		};

		/// <summary>
		/// Enumeration for the different AI skill settings.
		/// </summary>
		enum AISkillSetting {
			MinSkill = 1,
			InferiorSkill = 35,
			DefaultSkill = 50,
			AverageSkill = 70,
			GoodSkill = 99,
			UnfairSkill = 100
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an Activity object in system memory. Create() should be called before using the object.
		/// </summary>
		Activity() { Clear(); }

		/// <summary>
		/// Makes the Activity object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates an Activity to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Activity to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Activity &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an Activity object before deletion from system memory.
		/// </summary>
		~Activity() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Activity object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current Activity state code. See the ActivityState enumeration for values.
		/// </summary>
		/// <returns>The current state of this Activity. See ActivityState enumeration.</returns>
		ActivityState GetActivityState() const { return m_ActivityState; }

		/// <summary>
		/// Overrides the current Activity state. Should not be used much, use dedicated state setting functions instead.
		/// </summary>
		/// <param name="newState">The new state to set.</param>
		void SetActivityState(ActivityState newState) { m_ActivityState = newState; }

		/// <summary>
		/// Indicates whether the Activity is currently running or not (not editing, over or paused)
		/// </summary>
		/// <returns>Whether the Activity is running or not.</returns>
		bool IsRunning() const { return (m_ActivityState == ActivityState::Running || m_ActivityState == ActivityState::Editing) && !m_Paused; }

		/// <summary>
		/// Indicates whether the Activity is currently paused or not.
		/// </summary>
		/// <returns>Whether the Activity is paused or not.</returns>
		bool IsPaused() const { return m_Paused; }

		/// <summary>
		/// Pauses and unpauses the Activity.
		/// </summary>
		/// <param name="pause">Whether to pause the Activity or not.</param>
		virtual void SetPaused(bool pause = true) { m_Paused = pause; }

		/// <summary>
		/// Indicates whether the Activity is over or not.
		/// </summary>
		/// <returns>Whether the Activity is over or not.</returns>
		bool IsOver() const { return m_ActivityState == ActivityState::Over; }

		/// <summary>
		/// Gets the user-friendly description of this Activity.
		/// </summary>
		/// <returns>A string with the user-friendly description of this Activity.</returns>
		std::string GetDescription() const { return m_Description; }

		/// <summary>
		/// Gets the max number of players supported by this Activity.
		/// </summary>
		/// <returns>The max number of players supported by this Activity.</returns>
		int GetMaxPlayerSupport() const { return m_MaxPlayerSupport; }

		/// <summary>
		/// Gets the minimum number of teams with players that this Activity requires.
		/// </summary>
		/// <returns>The minimum number of Teams this Activity requires to run.</returns>
		int GetMinTeamsRequired() const { return m_MinTeamsRequired; }

		/// <summary>
		/// Tells if a particular Scene supports this specific Activity on it. Usually that means certain Areas need to be defined in the Scene.
		/// </summary>
		/// <param name="scene">The Scene to check if it supports this Activity. Ownership IS NOT TRANSFERRED!</param>
		/// <param name="teams">How many teams we're checking for. Some scenes may support and Activity but only for a limited number of teams. If -1, not applicable.</param>
		/// <returns>Whether the Scene has the right stuff.</returns>
		virtual bool SceneIsCompatible(Scene *scene, int teams = -1) { return scene && teams <= m_MinTeamsRequired; }

		/// <summary>
		/// Shows in which stage of the Campaign this appears.
		/// </summary>
		/// <returns>The stage number in the campaign. -1 means it's not in the campaign.</returns>
		int GetInCampaignStage() const { return m_InCampaignStage; }

		/// <summary>
		/// Sets in which stage of the Campaign this appears.
		/// </summary>
		/// <param name="newStage">The new stage to set. -1 means it doesn't appear in the campaign.</param>
		void SetInCampaignStage(int newStage) { m_InCampaignStage = newStage; }

		/// <summary>
		/// Gets the name of the current scene.
		/// </summary>
		/// <returns>A string with the instance name of the scene.</returns>
		std::string GetSceneName() const { return m_SceneName; }

		/// <summary>
		/// Sets the name of the scene this is associated with.
		/// </summary>
		/// <param name="sceneName">The new name of the scene to load next game.</param>
		void SetSceneName(const std::string sceneName) { m_SceneName = sceneName; }
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Officially starts this Activity. Creates all the data necessary to start the Activity.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Start();

		/// <summary>
		/// Forces this Activity to end.
		/// </summary>
		virtual void End();

		/// <summary>
		/// Updates the state of this Activity. Supposed to be done every frame before drawing.
		/// </summary>
		virtual void Update();

		/// <summary>
		/// Draws the currently active GUI of a screen to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a screen-sized BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		/// <param name="whichScreen">Which screen's GUI to draw onto the bitmap.</param>
		virtual void DrawGUI(BITMAP *targetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0) {}

		/// <summary>
		/// Draws this Activity's current graphical representation to a BITMAP of choice. This includes all game-related graphics.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		virtual void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector()) {}
#pragma endregion

#pragma region Player Handling
		/// <summary>
		/// Pre-calculates the player-to-screen index map, counts the number of active players etc.
		/// </summary>
		virtual void SetupPlayers();

		/// <summary>
		/// Indicates whether a specific player is active in the current game.
		/// </summary>
		/// <param name="player">Which player index to check.</param>
		/// <returns>Whether the player is active in the current Activity.</returns>
		bool PlayerActive(int player) const { return m_IsActive[player]; }

		/// <summary>
		/// Turns off a player if they were active. Should only be done if brain etc are already taken care of and disposed of properly.
		/// Will also deactivate the team this player is on, if there's no other active players still on it.
		/// </summary>
		/// <param name="playerToDeactivate">Which player index to deactivate.</param>
		/// <returns>Whether the player was active before trying to deactivate.</returns>
		bool DeactivatePlayer(int playerToDeactivate);

		/// <summary>
		/// Sets up a specific player for this Activity, AI or Human.
		/// </summary>
		/// <param name="playerToAdd">Which player slot to set up - PlayerOne to PlayerFour.</param>
		/// <param name="isHuman">Whether this player is Human.</param>
		/// <param name="team">Which Team this player belongs to.</param>
		/// <param name="funds">How many funds this player contributes to its Team's total funds.</param>
		/// <param name="teamIcon">The team flag icon of this player - OWNERSHIP IS NOT TRANSFERRED.</param>
		/// <returns>The new total number of active players in the current game.</returns>
		int AddPlayer(int playerToAdd, bool isHuman, int team, float funds, const Icon *teamIcon = 0);

		/// <summary>
		/// Sets all players as not active in the current Activity.
		/// </summary>
		/// <param name="resetFunds">Whether to reset the team funds as well.</param>
		void ClearPlayers(bool resetFunds = true);

		/// <summary>
		/// Gets the total number of active players in the current Activity, AI or Human.
		/// </summary>
		/// <returns>The total number of players in the current Activity.</returns>
		int GetPlayerCount() const { return m_PlayerCount; }

		/// <summary>
		/// Gets the total number of human players in the current Activity.
		/// </summary>
		/// <returns>The total number of players in the current Activity.</returns>
		int GetHumanCount() const;

		/// <summary>
		/// Indicates whether a specific player is human in the current game, ie not an AI player and has a screen etc.
		/// </summary>
		/// <param name="player">Which player index to check.</param>
		/// <returns>Whether the player is active as a Human in the current Activity.</returns>
		bool PlayerHuman(int player) const { return m_IsHuman[player]; }

		/// <summary>
		/// Gets the current team a specific player belongs to.
		/// </summary>
		/// <param name="player">The player to get the team info on.</param>
		/// <returns>The team number of the specified player.</returns>
		int GetTeamOfPlayer(int player) const { return m_Team[player]; }

		/// <summary>
		/// Sets the current team a specific player belongs to.
		/// </summary>
		/// <param name="player">The player to set the team for.</param>
		/// <param name="team">The team number to set the player to.</param>
		void SetTeamOfPlayer(int player, int team);

		/// <summary>
		/// Converts a player index into a screen index, and only if that player is human.
		/// </summary>
		/// <param name="player">Which player index to convert.</param>
		/// <returns>The screen index, or -1 if non-human player or no players.</returns>
		int ScreenOfPlayer(int player) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_PlayerScreen[player] : -1; }

		/// <summary>
		/// Converts a screen index into a player index, if that screen exists.
		/// </summary>
		/// <param name="screen">Which screen index to convert.</param>
		/// <returns>The player index, or -1 if that screen is not in use.</returns>
		int PlayerOfScreen(int screen) const;

		/// <summary>
		/// Gets the current viewing state for a specific player. See the ViewState enumeration for values.
		/// </summary>
		/// <param name="whichPlayer">Which player to get the view state for.</param>
		/// <returns>The current viewing state of the player.</returns>
		ViewState GetViewState(int whichPlayer = 0) const { return m_ViewState[whichPlayer]; }

		/// <summary>
		/// Sets the current viewing state for a specific player. See the ViewState enumeration for values.
		/// </summary>
		/// <param name="whichViewState">The state to set to.</param>
		/// <param name="whichPlayer">Which player to set the view state for.</param>
		void SetViewState(ViewState whichViewState, int whichPlayer = 0) { m_ViewState[whichPlayer] = whichViewState; }

		/// <summary>
		/// Resets the message timer for one player.
		/// </summary>
		/// <param name="player">The player to reset the message timer for.</param>
		void ResetMessageTimer(int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) { m_MessageTimer[player].Reset(); } }
#pragma endregion

#pragma region Team Handling
		/// <summary>
		/// Gets the total number of teams in the current Activity.
		/// </summary>
		/// <returns>The total number of teams in the current Activity.</returns>
		int GetTeamCount() const { return m_TeamCount; }

		/// <summary>
		/// Gets the name of a specific team.
		/// </summary>
		/// <param name="whichTeam">Which team to get the name of. 0 = first team.</param>
		/// <returns>The current name of that team.</returns>
		std::string GetTeamName(int whichTeam = 0) const;

		/// <summary>
		/// Sets the name of a specific team.
		/// </summary>
		/// <param name="whichTeam">Which team to set the name of. 0 = first team.</param>
		/// <param name="newName">The name to set it to.</param>
		void SetTeamName(int whichTeam, const std::string &newName) { if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) { m_TeamNames[whichTeam] = newName; } }

		/// <summary>
		/// Gets the Icon of a specific team.
		/// </summary>
		/// <param name="whichTeam">Which team to get the Icon of. 0 = first team.</param>
		/// <returns>The current Icon of that team.</returns>
		const Icon * GetTeamIcon(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? &m_TeamIcons[whichTeam] : nullptr; }

		/// <summary>
		/// Sets the Icon of a specific team.
		/// </summary>
		/// <param name="whichTeam">Which team to set the Icon of. 0 = first team.</param>
		/// <param name="newIcon">The Icon to set it to.</param>
		void SetTeamIcon(int whichTeam, const Icon &newIcon) { if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) { m_TeamIcons[whichTeam] = newIcon; } }

		/// <summary>
		/// Indicates whether a specific team is active in the current game.
		/// </summary>
		/// <param name="team">Which team index to check.</param>
		/// <returns>Whether the team is active in the current Activity.</returns>
		bool TeamActive(int team) const { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamActive[team] : false; }

		/// <summary>
		/// Indicates whether a team is player controlled or not.
		/// </summary>
		/// <param name="team">The team number to check.</param>
		/// <returns>Whether  team is player controlled or not.</returns>
		bool IsHumanTeam(int team) const;

		/// <summary>
		/// Gets the current number of players in a specific team.
		/// </summary>
		/// <param name="team">Which team to get the player count for.</param>
		/// <returns>The player count in the specified team.</returns>
		int PlayersInTeamCount(int team) const;

		/// <summary>
		/// Gets the number of deaths on a specific team so far on the current Activity.
		/// </summary>
		/// <param name="whichTeam">Which team to get the death tally of. 0 = first team.</param>
		/// <returns>The current death count.</returns>
		int GetTeamDeathCount(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamDeaths[whichTeam] : 0; }

		/// <summary>
		/// Increments the tally of a death of an actor on a specific team.
		/// </summary>
		/// <param name="whichTeam">Which team to increase the death count of. 0 = first team.</param>
		/// <param name="howMany">The new death count.</param>
		/// <returns>The updated death count of the team.</returns>
		int ReportDeath(int whichTeam = 0, int howMany = 1) { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamDeaths[whichTeam] += howMany : 0; }
#pragma endregion

#pragma region Funds Handling
		/// <summary>
		/// Gets the amount of funds a specific team currently has in the Activity.
		/// </summary>
		/// <param name="whichTeam">Which team to get the fund count from. 0 = first team.</param>
		/// <returns>A float with the funds tally for the requested team.</returns>
		float GetTeamFunds(int whichTeam = 0) const { return (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) ? m_TeamFunds[whichTeam] : 0; }

		/// <summary>
		/// Sets the amount of funds a specific team currently has in the Activity.
		/// </summary>
		/// <param name="newFunds">Which team to set the fund count for. 0 = first team.</param>
		/// <param name="which">A float with the funds tally for the requested team.</param>
		void SetTeamFunds(float newFunds, int whichTeam = 0) { if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) { m_TeamFunds[whichTeam] = newFunds; } }

		/// <summary>
		/// Changes a team's funds level by a certain amount.
		/// </summary>
		/// <param name="howMuch">The amount with which to change the funds balance.</param>
		/// <param name="whichTeam">Which team to alter the funds of. 0 = first team.</param>
		void ChangeTeamFunds(float howMuch, int whichTeam = 0);

		/// <summary>
		/// Checks whether the team funds changed since last time this was called. This also resets the state, so calling this again on the same team will yield false unless it's been changed again.
		/// </summary>
		/// <param name="whichTeam">Which team's funds to check.</param>
		/// <returns>Whether funds amount changed for this team since last time this was called.</returns>
		bool TeamFundsChanged(int whichTeam = 0);

		/// <summary>
		/// Gets the amount of funds a specific player originally added to his team's collective stash.
		/// </summary>
		/// <param name="player">Which player to check for.</param>
		/// <returns>A float with the funds originally deposited by this player.</returns>
		float GetPlayerFundsContribution(int player) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_FundsContribution[player] : 0; }

		/// <summary>
		/// Sets a new amount of starting funds for a player, after he has already been added.
		/// </summary>
		/// <param name="player">Which player slot to update - has to already be active.</param>
		/// <param name="newFunds">Updated value of how many funds this player contributes to its Team's total funds.</param>
		/// <returns>Whether the update was successful.</returns>
		bool UpdatePlayerFundsContribution(int player, float newFunds);

		/// <summary>
		/// Gets the share of funds a specific PLAYER currently has in the game, calculated from his original contribution to his team's collective funds.
		/// </summary>
		/// <param name="player">Which player to get the fund count from.</param>
		/// <returns>A float with the funds tally for the requested player.</returns>
		float GetPlayerFundsShare(int player = 0) const;
#pragma endregion

#pragma region Brain Handling
		/// <summary>
		/// Shows how many human controlled brains are left in this Activity.
		/// </summary>
		/// <returns>How many human controlled brains are left in this Activity.</returns>
		int HumanBrainCount() const { return GetBrainCount(true); }

		/// <summary>
		/// Shows how many AI controlled brains are left in this Activity.
		/// </summary>
		/// <returns>how many AI controlled brains are left in this Activity.</returns>
		int AIBrainCount() const { return GetBrainCount(false); }

		/// <summary>
		/// Gets the current Brain actor for a specific player. 
		/// </summary>
		/// <param name="player">Which player to get the brain actor for.</param>
		/// <returns>A pointer to the Brain Actor. Ownership is NOT transferred!</returns>
		Actor * GetPlayerBrain(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_Brain[player] : nullptr; }

		/// <summary>
		/// Sets the current Brain actor for a specific player.
		/// </summary>
		/// <param name="newBrain">A pointer to the new brain Actor. Ownership is NOT transferred!</param>
		/// <param name="player"> Which team to set the brain actor for.</param>
		void SetPlayerBrain(Actor *newBrain, int player = 0);

		/// <summary>
		/// Shows whether a specific player ever had a Brain yet.
		/// </summary>
		/// <param name="player">Which player to check whether they ever had a Brain.</param>
		/// <returns>Whether this player ever had a Brain.</returns>
		bool PlayerHadBrain(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_HadBrain[player] : false; }

		/// <summary>
		/// Sets to indicate that the player had a Brain at some point. This is to simulate that in automated battle cases.
		/// </summary>
		/// <param name="player">Which player to set whether he had a Brain or not.</param>
		/// <param name="hadBrain">Whether he should be flagged as having had a Brain.</param>
		void SetPlayerHadBrain(int player, bool hadBrain = true) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) m_HadBrain[player] = hadBrain; }

		/// <summary>
		/// Shows whether a specific player's Brain was evacuated into orbit so far.
		/// </summary>
		/// <param name="player">Which player to check whether their Brain was evacuated.</param>
		/// <returns>Whether this player had a Brain that was evacuated.</returns>
		bool BrainWasEvacuated(int player = 0) const { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_BrainEvacuated[player] : false; }

		/// <summary>
		/// Sets whether a player's Brain was evacuated during the Activity.
		/// </summary>
		/// <param name="player">Which player to check whether their Brain was evacuated.</param>
		/// <param name="evacuated">Whether it was evacuated yet.</param>
		void SetBrainEvacuated(int player = 0, bool evacuated = true) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) { m_BrainEvacuated[player] = evacuated; } }

		/// <summary>
		/// Shows whether ANY player evacuated their Brain.
		/// </summary>
		/// <returns>Whether any player evacuated their Brain yet.</returns>
		bool AnyBrainWasEvacuated() const;

		/// <summary>
		/// Shows whether the passed in actor is the Brain of any player.
		/// </summary>
		/// <param name="actor">Which Actor to check for player braininess.</param>
		/// <returns>Whether any player's Brain or not.</returns>
		bool IsAssignedBrain(Actor *actor) const;

		/// <summary>
		/// Shows which player has a specific actor as a Brain, if any.
		/// </summary>
		/// <param name="actor">Which Actor to check for player braininess.</param>
		/// <returns>Which player has this assigned as a Brain, if any.</returns>
		int IsBrainOfWhichPlayer(Actor *actor) const;

		/// <summary>
		/// Shows whether the passed in actor is the Brain of any other player.
		/// </summary>
		/// <param name="actor">Which Actor to check for other player braininess.</param>
		/// <param name="player">From which player's perspective to check.</param>
		/// <returns>Whether other player's Brain or not.</returns>
		bool IsOtherPlayerBrain(Actor *actor, int player) const;
#pragma endregion

#pragma region Difficulty Handling
		/// <summary>
		/// Returns string representation of a given difficulty value.
		/// </summary>
		/// <param name="difficulty">Difficulty setting</param>
		/// <returns>Corresponding difficulty string.</returns>
		static std::string GetDifficultyString(int difficulty);

		/// <summary>
		/// Gets the current difficulty setting.
		/// </summary>
		/// <returns>The current setting.</returns>
		int GetDifficulty() const { return m_Difficulty; }

		/// <summary>
		/// Sets the current difficulty setting.
		/// </summary>
		/// <param name="newDifficulty">The new difficulty setting.</param>
		void SetDifficulty(int newDifficulty) { m_Difficulty = Limit(newDifficulty, DifficultySetting::MaxDifficulty, DifficultySetting::MinDifficulty); }
#pragma endregion

#pragma region AI Handling
		/// <summary>
		/// Returns string representation of a given AI skill value.
		/// </summary>
		/// <param name="skill">AI skill setting.</param>
		/// <returns>Corresponding AI skill string.</returns>
		static std::string GetAISkillString(int skill);

		/// <summary>
		/// Returns skill level for specified team. If team is less than 0 or greater than 3 an average of all teams is returned.
		/// </summary>
		/// <param name="team">Team to get skill level for.</param>
		/// <returns>Team skill level.</returns>
		int GetTeamAISkill(int team) const;

		/// <summary>
		/// Sets AI skill level for specified team.
		/// </summary>
		/// <param name="team">The team to set for.</param>
		/// <param name="skill">AI skill level, 1-100.</param>
		void SetTeamAISkill(int team, int skill) { if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) { m_TeamAISkillLevels[team] = Limit(skill, AISkillSetting::UnfairSkill, AISkillSetting::MinSkill); } }
#pragma endregion

#pragma region Actor Handling
		/// <summary>
		/// Gets the currently controlled actor of a specific player.
		/// </summary>
		/// <param name="player">Which player to get the controlled actor of.</param>
		/// <returns>A pointer to the controlled Actor. Ownership is NOT transferred! 0 If no actor is currently controlled by this player.</returns>
		Actor * GetControlledActor(int player = 0) { return (player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_ControlledActor[player] : 0; }

		/// <summary>
		/// Makes the player's ControlledActor the leader of any squad it is a member of.
		/// </summary>
		/// <param name="player">Player to reassign for.</param>
		/// <param name="team">Team of the player.</param>
		void ReassignSquadLeader(const int player, const int team);

		/// <summary>
		/// Forces the ActivityMan to focus player control to a specific Actor for a specific team. OWNERSHIP IS NOT TRANSFERRED!
		/// </summary>
		/// <param name="actor">Which Actor to switch focus to. The team of this Actor will be set once it is passed in. The actor should have been added to MovableMan already.</param>
		/// <param name="player">Player to force for.</param>
		/// <param name="team">Which team to switch to next actor on.</param>
		/// <returns>Whether the focus switch was successful or not.</returns>
		virtual bool SwitchToActor(Actor *actor, int player = 0, int team = 0);

		/// <summary>
		/// Forces the Activity to focus player control to the previous Actor of a specific team, other than the current one focused on.
		/// </summary>
		/// <param name="player">Player to force for.</param>
		/// <param name="team">Which team to switch to next Actor on.</param>
		/// <param name="actorToSkip">An Actor pointer to skip in the sequence.</param>
		virtual void SwitchToPrevActor(int player, int team, Actor *actorToSkip = 0) { SwitchToPrevOrNextActor(false, player, team, actorToSkip); }

		/// <summary>
		/// Forces the Activity to focus player control to the next Actor of a specific team, other than the current one focused on.
		/// </summary>
		/// <param name="player">Player to force for.</param>
		/// <param name="team">Which team to switch to next Actor on.</param>
		/// <param name="actorToSkip">An Actor pointer to skip in the sequence.</param>
		virtual void SwitchToNextActor(int player, int team, Actor *actorToSkip = 0) { SwitchToPrevOrNextActor(true, player, team, actorToSkip); }

		/// <summary>
		/// Indicates an Actor as having left the game scene and entered orbit.  OWNERSHIP IS NOT transferred, as the Actor's inventory is just 'unloaded'.
		/// </summary>
		/// <param name="orbitedCraft">The actor instance that entered orbit. Ownership IS NOT TRANSFERRED!</param>
		virtual void EnteredOrbit(Actor *orbitedCraft);

		/// <summary>
		/// Gets whether craft must be considered orbited if they reach the map border on non-wrapped maps.
		/// </summary>
		/// <returns>Whether craft are considered orbited when at the border of a non-wrapping map.</returns>
		bool GetCraftOrbitAtTheEdge() const { return m_CraftOrbitAtTheEdge; }

		/// <summary>
		/// Sets whether craft must be considered orbited if they reach the map border on non-wrapped maps.
		/// </summary>
		/// <param name="value">Whether to consider orbited or not.</param>
		void SetCraftOrbitAtTheEdge(bool value) { m_CraftOrbitAtTheEdge = value; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ActivityState m_ActivityState; //!< Current state of this Activity.	
		bool m_Paused; //!< Whether this Activity is paused or not.

		std::string m_Description; //!< User-friendly description of what this Activity is all about.
		std::string m_SceneName; //!< The name of the Scene in which this Activity takes place.

		int m_MaxPlayerSupport; //!< How many separate players this Activity can support at the same time.
		int m_MinTeamsRequired; //!< How many separate teams this Activity can support at the same time.

		int m_Difficulty; //!< Current difficulty setting of this Activity.
		bool m_CraftOrbitAtTheEdge; //!< If true then on non-wrapping maps craft beyond the edge of the map are considered orbited.
		int m_InCampaignStage; //!< Which stage of the campaign this Activity appears in, if any (-1 means it's not in the campaign).

		int m_PlayerCount; //!< The number of total players in the current Activity, AI and Human.
		bool m_IsActive[Players::MaxPlayerCount]; //!< Whether a specific player is at all active and playing this Activity.
		bool m_IsHuman[Players::MaxPlayerCount]; //!< Whether a specific player is Human or not, and needs a screen etc.

		int m_PlayerScreen[Players::MaxPlayerCount]; //!< The screen index of each player - only applicable to human players. -1 if AI or other.
		ViewState m_ViewState[Players::MaxPlayerCount]; //!< What to be viewing for each player.

		std::string m_TeamNames[Teams::MaxTeamCount]; //!< Names for each team.
		Icon m_TeamIcons[Teams::MaxTeamCount]; //!< Icons for each team.

		int m_TeamCount; //!< The number of teams in the current Activity.
		bool m_TeamActive[Teams::MaxTeamCount]; //!< Whether a specific team is active or not in this Activity.
		int m_Team[Players::MaxPlayerCount]; //!< The designated team of each player.	
		int m_TeamDeaths[Teams::MaxTeamCount]; //!< The count of how many actors have died on this team.
		int m_TeamAISkillLevels[Teams::MaxTeamCount]; //!< AI skill levels for teams.

		float m_TeamFunds[Teams::MaxTeamCount]; //!< Gold counter for each team.
		float m_TeamFundsShare[Players::MaxPlayerCount]; //!< The ratio of how much this player contributed to his team's funds at the start of the Activity.
		bool m_FundsChanged[Teams::MaxTeamCount]; //!< Whether the team funds have changed during the current frame.
		float m_FundsContribution[Players::MaxPlayerCount]; //!< How much this player contributed to his team's funds at the start of the Activity.

		Actor *m_Brain[Players::MaxPlayerCount]; //!< The Brain of each player. Not owned!
		bool m_HadBrain[Players::MaxPlayerCount]; //!< Whether each player has yet had a Brain. If not, then their Activity doesn't end if no brain is found.
		bool m_BrainEvacuated[Players::MaxPlayerCount]; //!< Whether a player has evacuated his Brain into orbit.

		Actor *m_ControlledActor[Players::MaxPlayerCount]; //!< Currently controlled actor, not owned.
		Controller m_PlayerController[Players::MaxPlayerCount]; //!< The Controllers of all the players for the GUIs.

		Timer m_MessageTimer[Players::MaxPlayerCount]; //!< Message timer for each player.

	private:

		/// <summary>
		/// Shared method to get the amount of human or AI controlled brains that are left in this Activity.
		/// </summary>
		/// <param name="getForHuman">Whether to get brain count for Human or for AI. True for Human.</param>
		/// <returns>How many human or AI controlled brains are left in this Activity.</returns>
		int GetBrainCount(bool getForHuman) const;

		/// <summary>
		/// Shared method to force the Activity to focus player control to the previous or next Actor of a specific team, other than the current one focused on.
		/// </summary>
		/// <param name="nextActor">Whether to switch to the previous or the next Actor. True for next Actor.</param>
		/// <param name="player">Player to force for.</param>
		/// <param name="team">Which team to switch to next Actor on.</param>
		/// <param name="skip">An Actor pointer to skip in the sequence.</param>
		void SwitchToPrevOrNextActor(bool nextActor, int player, int team, const Actor *skip = 0);

		/// <summary>
		/// Clears all the member variables of this Activity, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Activity(const Activity &reference) = delete;
		Activity & operator=(const Activity &rhs) = delete;
	};
}
#endif