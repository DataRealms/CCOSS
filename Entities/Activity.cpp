#include "Activity.h"

#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "FrameMan.h"
#include "MetaMan.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

namespace RTE {

	AbstractClassInfo(Activity, Entity);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Activity::Clear() {
		m_ActivityState = ActivityState::NotStarted;
		m_Paused = false;
		m_Description.clear();
		m_SceneName.clear();
		m_MaxPlayerSupport = Players::MaxPlayerCount;
		m_MinTeamsRequired = 2;
		m_Difficulty = 50;
		m_CraftOrbitAtTheEdge = false;
		m_InCampaignStage = -1;
		m_PlayerCount = 1;
		m_TeamCount = 1;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			// Player 1 is active by default, for the editor etc
			m_IsActive[player] = player == Players::PlayerOne;
			m_IsHuman[player] = player == Players::PlayerOne;
			m_PlayerScreen[player] = (player == Players::PlayerOne) ? Players::PlayerOne : Players::NoPlayer;
			m_ViewState[player] = ViewState::Normal;
			m_Team[player] = Teams::TeamOne;
			m_TeamFundsShare[player] = 1.0F;
			m_FundsContribution[player] = 0;
			m_Brain[player] = 0;
			m_HadBrain[player] = false;
			m_BrainEvacuated[player] = false;
			m_ControlledActor[player] = 0;
			m_PlayerController[player].Reset();
			m_MessageTimer[player].Reset();
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			m_TeamNames[team] = "Team " + std::to_string(team + 1);
			m_TeamIcons[team].Reset();
			// Team 1 is active by default, for the editors etc
			m_TeamActive[team] = team == Teams::TeamOne;
			m_TeamDeaths[team] = 0;
			m_TeamAISkillLevels[team] = AISkillSetting::DefaultSkill;
			m_TeamFunds[team] = 2000;
			m_FundsChanged[team] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Create() {
		if (Entity::Create() < 0) {
			return -1;
		}
		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			std::string teamNum = std::to_string(team + 1);
			m_TeamIcons[team] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team " + teamNum + " Default"));

		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Create(const Activity &reference) {
		Entity::Create(reference);

		m_ActivityState = reference.m_ActivityState;
		m_Paused = reference.m_Paused;
		m_Description = reference.m_Description;
		m_MaxPlayerSupport = reference.m_MaxPlayerSupport;
		m_MinTeamsRequired = reference.m_MinTeamsRequired;
		m_Difficulty = reference.m_Difficulty;
		m_CraftOrbitAtTheEdge = reference.m_CraftOrbitAtTheEdge;
		m_InCampaignStage = reference.m_InCampaignStage;
		m_PlayerCount = reference.m_PlayerCount;
		m_TeamCount = reference.m_TeamCount;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_Team[player] = reference.m_Team[player];
			m_FundsContribution[player] = reference.m_FundsContribution[player];
			m_TeamFundsShare[player] = reference.m_TeamFundsShare[player];
			m_IsActive[player] = reference.m_IsActive[player];
			m_IsHuman[player] = reference.m_IsHuman[player];
			m_PlayerScreen[player] = reference.m_PlayerScreen[player];
			m_Brain[player] = reference.m_Brain[player];
			m_HadBrain[player] = reference.m_HadBrain[player];
			m_BrainEvacuated[player] = reference.m_BrainEvacuated[player];
			m_ViewState[player] = reference.m_ViewState[player];
			m_ControlledActor[player] = reference.m_ControlledActor[player];
			m_PlayerController[player] = reference.m_PlayerController[player];
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			m_TeamActive[team] = reference.m_TeamActive[team];
			m_TeamFunds[team] = reference.m_TeamFunds[team];
			m_FundsChanged[team] = reference.m_FundsChanged[team];
			m_TeamNames[team] = reference.m_TeamNames[team];
			if (reference.m_TeamIcons[team].GetFrameCount() > 0) { m_TeamIcons[team] = reference.m_TeamIcons[team]; }
			m_TeamDeaths[team] = reference.m_TeamDeaths[team];
			m_TeamAISkillLevels[team] = reference.m_TeamAISkillLevels[team];
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "Description") {
			reader >> m_Description;
		} else if (propName == "SceneName") {
			reader >> m_SceneName;
		} else if (propName == "MaxPlayerSupport") {
			reader >> m_MaxPlayerSupport;
		} else if (propName == "MinTeamsRequired") {
			reader >> m_MinTeamsRequired;
		} else if (propName == "Difficulty") {
			reader >> m_Difficulty;
		} else if (propName == "CraftOrbitAtTheEdge") {
			reader >> m_CraftOrbitAtTheEdge;
		} else if (propName == "InCampaignStage") {
			reader >> m_InCampaignStage;
		} else if (propName == "TeamOfPlayer1" || propName == "TeamOfPlayer2" || propName == "TeamOfPlayer3" || propName == "TeamOfPlayer4") {
			for (int playerTeam = Teams::TeamOne; playerTeam < Teams::MaxTeamCount; playerTeam++) {
				std::string playerTeamNum = std::to_string(playerTeam + 1);
				if (propName == "TeamOfPlayer" + playerTeamNum) {
					int team;
					reader >> team;
					if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) {
						m_Team[playerTeam] = team;
						m_IsActive[playerTeam] = true;
						if (!m_TeamActive[team]) { m_TeamCount++; }
						m_TeamActive[team] = true;
					} else {
						m_IsActive[playerTeam] = false;
					}
					break;
				}
			}
		} else if (propName == "Player1IsHuman" || propName == "Player2IsHuman" || propName == "Player3IsHuman" || propName == "Player4IsHuman") {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "Player" + playerNum + "IsHuman") {
					reader >> m_IsHuman[player];
					break;
				}
			}
		} else if (propName == "Team1Name" || propName == "Team2Name" || propName == "Team3Name" || propName == "Team4Name") {
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Name") {
					reader >> m_TeamNames[team];
					if (!m_TeamActive[team]) { m_TeamCount++; }
					m_TeamActive[team] = true;
					break;
				}
			}
		} else if (propName == "Team1Icon" || propName == "Team2Icon" || propName == "Team3Icon" || propName == "Team4Icon") {
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Icon") {
					reader >> m_TeamIcons[team];
					break;
				}
			}
		} else if (propName == "Team1Funds" || propName == "Team2Funds" || propName == "Team3Funds" || propName == "Team4Funds") {
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Funds") {
					reader >> m_TeamFunds[team];
					break;
				}
			}
		} else if (propName == "TeamFundsShareOfPlayer1" || propName == "TeamFundsShareOfPlayer2" || propName == "TeamFundsShareOfPlayer3" || propName == "TeamFundsShareOfPlayer4") {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "TeamFundsShareOfPlayer" + playerNum) {
					reader >> m_TeamFundsShare[player];
					break;
				}
			}
		} else if (propName == "FundsContributionOfPlayer1" || propName == "FundsContributionOfPlayer2" || propName == "FundsContributionOfPlayer3" || propName == "FundsContributionOfPlayer4") {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "FundsContributionOfPlayer" + playerNum) {
					reader >> m_FundsContribution[player];
					break;
				}
			}
		} else {
			return Entity::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewProperty("Description");
		writer << m_Description;
		writer.NewProperty("SceneName");
		writer << m_SceneName;
		writer.NewProperty("MaxPlayerSupport");
		writer << m_MaxPlayerSupport;
		writer.NewProperty("MinTeamsRequired");
		writer << m_MinTeamsRequired;
		writer.NewProperty("Difficulty");
		writer << m_Difficulty;
		writer.NewProperty("CraftOrbitAtTheEdge");
		writer << m_CraftOrbitAtTheEdge;
		writer.NewProperty("InCampaignStage");
		writer << m_InCampaignStage;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player + 1);
			if (m_IsActive[player]) {
				writer.NewProperty("TeamOfPlayer" + playerNum);
				writer << m_Team[player];
				writer.NewProperty("FundsContributionOfPlayer" + playerNum);
				writer << m_FundsContribution[player];
				writer.NewProperty("TeamFundsShareOfPlayer" + playerNum);
				writer << m_TeamFundsShare[player];
				writer.NewProperty("Player" + playerNum + "IsHuman");
				writer << m_IsHuman[player];
			} else {
				writer.NewProperty("TeamOfPlayer" + playerNum);
				writer << Teams::NoTeam;
			}
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
			std::string teamNum = std::to_string(team + 1);
			if (m_TeamActive[team]) {
				writer.NewProperty("Team" + teamNum + "Funds");
				writer << m_TeamFunds[team];
				writer.NewProperty("Team" + teamNum + "Name");
				writer << m_TeamNames[team];
				writer.NewProperty("Team" + teamNum + "Icon");
				m_TeamIcons[team].SavePresetCopy(writer);
			}
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Start() {
		m_ActivityState = ActivityState::Running;
		m_Paused = false;

		// Reset the mouse moving so that it won't trap the mouse if the window isn't in focus (common after loading)
		if (!g_FrameMan.IsInMultiplayerMode()) {
			g_UInputMan.DisableMouseMoving(true);
			g_UInputMan.DisableMouseMoving(false);
			g_UInputMan.DisableKeys(false);
		}

		int error = g_SceneMan.LoadScene();
		if (error < 0) {
			return error;
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			if (!m_TeamActive[team]) {
				continue;
			}
			m_FundsChanged[team] = false;
			m_TeamDeaths[team] = 0;
		}

		// Intentionally doing all players, all need controllers
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ViewState[player] = ViewState::Normal;
			g_FrameMan.ClearScreenText(ScreenOfPlayer(player));
			g_SceneMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(player));

			m_PlayerController[player].Reset();
			m_PlayerController[player].Create(Controller::CIM_PLAYER, player);
			m_PlayerController[player].SetTeam(m_Team[player]);

			m_MessageTimer[player].Reset();
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::End() {
		// Actor control is automatically disabled when players are set to observation mode, so no need to do anything directly.
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_ViewState[player] = ViewState::Observe;
		}
		m_ActivityState = ActivityState::Over;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetupPlayers() {
		m_TeamCount = 0;
		m_PlayerCount = 0;

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			m_TeamActive[team] = false;
		}

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player]) {
				m_PlayerCount++;
				if (!m_TeamActive[m_Team[player]]) { m_TeamCount++; }
				m_TeamActive[m_Team[player]] = true;
			}

			// Calculate which screen each human player is using, based on how many non-human players there are before him
			int screenIndex = -1;
			if (m_IsActive[player] && m_IsHuman[player]) {
				for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount && playerToCheck <= player; ++playerToCheck) {
					if (m_IsActive[playerToCheck] && m_IsHuman[playerToCheck]) { screenIndex++; }
				}
			}
			m_PlayerScreen[player] = screenIndex;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::DeactivatePlayer(int playerToDeactivate) {
		if (playerToDeactivate < Players::PlayerOne || playerToDeactivate >= Players::MaxPlayerCount || !m_IsActive[playerToDeactivate] || !m_TeamActive[m_Team[playerToDeactivate]]) {
			return false;
		}

		// We need to check if this player is the last one on their team and, if so, deactivate the team
		bool lastOnTeam = true;

		for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; ++playerToCheck) {
			if (m_IsActive[playerToCheck] && playerToCheck != playerToDeactivate && m_Team[playerToCheck] == m_Team[playerToDeactivate]) {
				lastOnTeam = false;
				break;
			}
		}
		if (lastOnTeam) {
			m_TeamActive[m_Team[playerToDeactivate]] = false;
			m_TeamCount--;
		}
		m_IsActive[playerToDeactivate] = false;
		m_PlayerCount--;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::AddPlayer(int playerToAdd, bool isHuman, int team, float funds, const Icon *teamIcon) {
		if (playerToAdd < Players::PlayerOne || playerToAdd >= Players::MaxPlayerCount || team < Teams::TeamOne || team >= Teams::MaxTeamCount) {
			return m_PlayerCount;
		}

		if (!m_TeamActive[team]) {
			m_TeamFundsShare[playerToAdd] = 1.0F;
			m_TeamCount++;
		} else {
			float totalFunds = m_TeamFunds[team] + funds;
			float newRatio = 1.0F + (funds / totalFunds);

			for (int teamPlayer = Players::PlayerOne; teamPlayer < Players::MaxPlayerCount; ++teamPlayer) {
				if (m_IsActive[teamPlayer] && m_Team[teamPlayer] == team) { m_TeamFundsShare[teamPlayer] /= newRatio; }
			}
			m_TeamFundsShare[playerToAdd] = funds / totalFunds;
		}

		m_TeamActive[team] = true;
		if (teamIcon) { SetTeamIcon(team, *teamIcon); }

		if (!m_IsActive[playerToAdd]) { m_PlayerCount++; }
		m_IsActive[playerToAdd] = true;

		m_IsHuman[playerToAdd] = isHuman;
		m_Team[playerToAdd] = team;
		m_TeamFunds[team] += funds;
		m_FundsContribution[playerToAdd] = funds;

		return m_PlayerCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ClearPlayers(bool resetFunds) {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			m_IsActive[player] = false;
			m_IsHuman[player] = false;

			if (resetFunds) {
				m_FundsContribution[player] = 0;
				m_TeamFundsShare[player] = 1.0F;
			}
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			m_TeamActive[team] = false;
			if (resetFunds) { m_TeamFunds[team] = 0; }
		}

		m_PlayerCount = m_TeamCount = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::GetHumanCount() const {
		int humans = 0;
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_IsHuman[player]) { humans++; }
		}
		return humans;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetTeamOfPlayer(int player, int team) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return;
		}

		m_Team[player] = team;
		m_TeamActive[team] = true;
		m_IsActive[player] = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::PlayerOfScreen(int screen) const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_PlayerScreen[player] == screen) {
				return player;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetTeamName(int whichTeam) const {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			return m_TeamActive[whichTeam] ? m_TeamNames[whichTeam] : "Inactive Team";
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsHumanTeam(int whichTeam) const {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
				if (m_IsActive[player] && m_Team[player] == whichTeam && m_IsHuman[player]) {
					return true;
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::PlayersInTeamCount(int team) const {
		int count = 0;
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_Team[player] == team) { count++; }
		}
		return count;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ChangeTeamFunds(float howMuch, int whichTeam) {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			m_TeamFunds[whichTeam] += howMuch;
			m_FundsChanged[whichTeam] = true;
			if (IsHumanTeam(whichTeam)) {
				for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
					if (m_Team[player] == whichTeam) { g_GUISound.FundsChangedSound()->Play(player); }
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::TeamFundsChanged(int whichTeam) {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			bool changed = m_FundsChanged[whichTeam];
			m_FundsChanged[whichTeam] = false;
			return changed;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::UpdatePlayerFundsContribution(int player, float newFunds) {
		if (player < Players::PlayerOne || player >= Players::MaxPlayerCount || !m_IsActive[player] || !m_TeamActive[m_Team[player]]) {
			return false;
		}

		// If this guy is responsible for all funds, then just update his contribution and be done
		if (m_TeamFundsShare[player] == 1.0F) {
			m_FundsContribution[player] = newFunds;
			m_TeamFunds[m_Team[player]] = newFunds;
		} else {
			m_FundsContribution[player] = newFunds;

			// Tally up all the funds of all players on this guy's team
			for (int playerOnTeam = Players::PlayerOne; playerOnTeam < Players::MaxPlayerCount; ++playerOnTeam) {
				if (m_IsActive[playerOnTeam] && m_Team[playerOnTeam] == m_Team[player]) { m_TeamFunds[m_Team[player]] += m_FundsContribution[playerOnTeam]; }
			}
			// Now that we have the updated total, update the shares of all team players
			for (int playerOnTeam = Players::PlayerOne; playerOnTeam < Players::MaxPlayerCount; ++playerOnTeam) {
				if (m_IsActive[playerOnTeam] && m_Team[playerOnTeam] == m_Team[player]) { m_TeamFundsShare[playerOnTeam] = m_FundsContribution[playerOnTeam] / m_TeamFunds[m_Team[player]]; }
			}
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Activity::GetPlayerFundsShare(int player) const {
		if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
			return (m_FundsContribution[player] > 0.0F) ? (m_TeamFunds[m_Team[player]] * m_TeamFundsShare[player]) : 0.0F;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetPlayerBrain(Actor *newBrain, int player) {
		if ((player >= Players::PlayerOne || player < Players::MaxPlayerCount) && newBrain) {
			newBrain->SetTeam(m_Team[player]);
			m_HadBrain[player] = true;
		}
		m_Brain[player] = newBrain;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::AnyBrainWasEvacuated() const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_BrainEvacuated[player]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsAssignedBrain(Actor *actor) const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (actor == m_Brain[player]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::IsBrainOfWhichPlayer(Actor *actor) const {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (actor == m_Brain[player]) {
				return player;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsOtherPlayerBrain(Actor *actor, int player) const {
		for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; ++playerToCheck) {
			if (m_IsActive[playerToCheck] && playerToCheck != player && actor == m_Brain[playerToCheck]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetDifficultyString(int difficulty) {
		if (difficulty < DifficultySetting::CakeDifficulty) {
			return "Cake";
		} else if (difficulty <= DifficultySetting::EasyDifficulty) {
			return "Easy";
		} else if (difficulty <= DifficultySetting::MediumDifficulty) {
			return "Medium";
		} else if (difficulty <= DifficultySetting::HardDifficulty) {
			return "Hard";
		} else if (difficulty <= DifficultySetting::NutsDifficulty) {
			return "Nuts";
		} else {
			return "Nuts!";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetAISkillString(int skill) {
		if (skill < AISkillSetting::InferiorSkill) {
			return "Inferior";
		} else if (skill < AISkillSetting::AverageSkill) {
			return "Average";
		} else if (skill < AISkillSetting::GoodSkill) {
			return "Good";
		} else {
			return "Unfair";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::GetTeamAISkill(int team) const {
		if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) {
			return m_TeamAISkillLevels[team];
		} else {
			int avgskill = 0;
			int count = 0;

			for (int playerTeam = Teams::TeamOne; playerTeam < Teams::MaxTeamCount; playerTeam++) {
				if (TeamActive(playerTeam)) {
					avgskill += GetTeamAISkill(playerTeam);
					count++;
				}
			}
			return (count > 0) ? avgskill / count : AISkillSetting::DefaultSkill;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ReassignSquadLeader(const int player, const int team) {
		if (m_ControlledActor[player]->GetAIMode() == Actor::AIMODE_SQUAD) {
			MOID leaderID = m_ControlledActor[player]->GetAIMOWaypointID();

			if (leaderID != g_NoMOID) {
				Actor *actor = g_MovableMan.GetNextTeamActor(team, m_ControlledActor[player]);

				do {
					// Set the controlled actor as new leader if actor follow the old leader, and not player controlled and not brain
					if (actor && (actor->GetAIMode() == Actor::AIMODE_SQUAD) && (actor->GetAIMOWaypointID() == leaderID) && !actor->GetController()->IsPlayerControlled() && !actor->IsInGroup("Brains")) {
						actor->ClearAIWaypoints();
						actor->AddAIMOWaypoint(m_ControlledActor[player]);
						// Make sure actor has m_ControlledActor registered as an AIMOWaypoint
						actor->UpdateMovePath();
					} else if (actor && actor->GetID() == leaderID) {
						// Set the old leader to follow the controlled actor and inherit his AI mode
						m_ControlledActor[player]->ClearAIWaypoints();
						m_ControlledActor[player]->SetAIMode(static_cast<Actor::AIMode>(actor->GetAIMode()));

						if (m_ControlledActor[player]->GetAIMode() == Actor::AIMODE_GOTO) {
							// Copy the old leaders move orders
							if (actor->GetAIMOWaypointID() != g_NoMOID) {
								const MovableObject *targetMO = g_MovableMan.GetMOFromID(actor->GetAIMOWaypointID());
								if (targetMO) { m_ControlledActor[player]->AddAIMOWaypoint(targetMO); }
							} else if ((actor->GetLastAIWaypoint() - actor->GetPos()).GetLargest() > 1) {
								m_ControlledActor[player]->AddAISceneWaypoint(actor->GetLastAIWaypoint());
							}
						}
						actor->ClearAIWaypoints();
						actor->SetAIMode(Actor::AIMODE_SQUAD);
						actor->AddAIMOWaypoint(m_ControlledActor[player]);
						// Make sure actor has m_ControlledActor registered as an AIMOWaypoint
						actor->UpdateMovePath();
					}
					actor = g_MovableMan.GetNextTeamActor(team, actor);
				} while (actor && actor != m_ControlledActor[player]);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::SwitchToActor(Actor *actor, int player, int team) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount || !m_IsHuman[player]) {
			return false;
		}
		if (!actor || !g_MovableMan.IsActor(actor)) {
			return false;
		}
		if ((actor != m_Brain[player] && actor->IsPlayerControlled()) || IsOtherPlayerBrain(actor, player)) {
			g_GUISound.UserErrorSound()->Play(player);
			return false;
		}

		Actor *preSwitchActor = (m_ControlledActor[player] && g_MovableMan.IsActor(m_ControlledActor[player])) ? m_ControlledActor[player] : 0;
		if (preSwitchActor) {
			preSwitchActor->SetControllerMode(Controller::CIM_AI);
			preSwitchActor->GetController()->SetDisabled(false);
		}

		m_ControlledActor[player] = actor;
		m_ControlledActor[player]->SetTeam(team);
		m_ControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);
		m_ControlledActor[player]->GetController()->SetDisabled(false);

		SoundContainer *actorSwitchSoundToPlay = (m_ControlledActor[player] == m_Brain[player]) ? g_GUISound.BrainSwitchSound() : g_GUISound.ActorSwitchSound();
		actorSwitchSoundToPlay->Play(player);

		if (preSwitchActor && g_SceneMan.ShortestDistance(preSwitchActor->GetPos(), m_ControlledActor[player]->GetPos(), g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY()).GetMagnitude() > g_FrameMan.GetResX() / 2) {
			g_GUISound.CameraTravelSound()->Play(player);
		}

		ReassignSquadLeader(player, team);
		m_ViewState[player] = Normal;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::EnteredOrbit(Actor *orbitedCraft) {
		if (!orbitedCraft) {
			return;
		}

		char messageString[64];
		float foreignCostMult = 0.9F;
		float nativeCostMult = 0.9F;
		int orbitedCraftTeam = orbitedCraft->GetTeam(); //TODO this should be explicitly casted. Preferred solution would be to use int consistently for teams.
		bool brainOnBoard = orbitedCraft->HasObjectInGroup("Brains");
		
		if (g_MetaMan.GameInProgress()) {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				if (GetTeamOfPlayer(static_cast<Players>(player)) == orbitedCraftTeam) {
					const MetaPlayer *metaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
					if (metaPlayer) {
						foreignCostMult = metaPlayer->GetForeignCostMultiplier();
						nativeCostMult = metaPlayer->GetNativeCostMultiplier();
					}
				}
			}
		}

		float totalValue = orbitedCraft->GetTotalValue(0, foreignCostMult, nativeCostMult);
		std::snprintf(messageString, sizeof(messageString), "Returned Craft + Cargo added %.0f oz to Funds!", totalValue);

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player]) {
				if (brainOnBoard && orbitedCraft == GetPlayerBrain(static_cast<Players>(player))) {
					m_BrainEvacuated[player] = true;
					std::snprintf(messageString, sizeof(messageString), "YOUR BRAIN HAS BEEN EVACUATED BACK INTO ORBIT!");
				}

				if (m_Team[player] == orbitedCraftTeam) {
					g_FrameMan.ClearScreenText(ScreenOfPlayer(ScreenOfPlayer(static_cast<Players>(player))));
					g_FrameMan.SetScreenText(messageString, ScreenOfPlayer(static_cast<Players>(player)), 0, 3500);
					m_MessageTimer[player].Reset();
				}
			}
		}

		m_TeamFunds[orbitedCraftTeam] += totalValue;
		orbitedCraft->SetGoldCarried(0);
		orbitedCraft->SetHealth(orbitedCraft->GetMaxHealth());

		// The craft entering orbit will count as a death for the team because it's being deleted, so we need to decrement the team's death count to keep it correct.
		m_TeamDeaths[orbitedCraftTeam]--;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::GetBrainCount(bool getForHuman) const {
		int brainCount = 0;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (getForHuman) {
				if (m_IsActive[player] && m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
					brainCount++;
				}
			} else {
				if (m_IsActive[player] && !m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
					brainCount++;
				}
			}
		}
		return brainCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SwitchToPrevOrNextActor(bool nextActor, int player, int team, const Actor *actorToSkip) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount || !m_IsHuman[player]) {
			return;
		}

		Actor *preSwitchActor = m_ControlledActor[player];
		Actor *actorToSwitchTo = nextActor ? g_MovableMan.GetNextTeamActor(team, preSwitchActor) : g_MovableMan.GetPrevTeamActor(team, preSwitchActor);
		const Actor *firstAttemptedSwitchActor = nullptr;

		bool actorSwitchSucceedOrTriedAllActors = false;
		while (!actorSwitchSucceedOrTriedAllActors) {
			if (actorToSwitchTo == preSwitchActor) {
				g_GUISound.UserErrorSound()->Play(player);
				actorSwitchSucceedOrTriedAllActors = true;
			} else if (actorToSwitchTo == firstAttemptedSwitchActor) {
				if (m_Brain[player]) {
					SwitchToActor(m_Brain[player]);
				} else {
					m_ControlledActor[player] = nullptr;
				}
				actorSwitchSucceedOrTriedAllActors = true;
			} else {
				actorSwitchSucceedOrTriedAllActors = (actorToSwitchTo != actorToSkip && SwitchToActor(actorToSwitchTo, player, team));
			}
			firstAttemptedSwitchActor = firstAttemptedSwitchActor ? firstAttemptedSwitchActor : actorToSwitchTo;
			if (!actorSwitchSucceedOrTriedAllActors) { actorToSwitchTo = nextActor ? g_MovableMan.GetNextTeamActor(team, actorToSwitchTo) : g_MovableMan.GetPrevTeamActor(team, actorToSwitchTo); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::Update() {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_MessageTimer[player].IsPastSimMS(5000)) { g_FrameMan.ClearScreenText(ScreenOfPlayer(player)); }
			if (m_IsActive[player]) { m_PlayerController[player].Update(); }
		}
	}
}
