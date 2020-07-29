#include "Activity.h"

#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "FrameMan.h"
#include "MetaMan.h"

#include "GUI/GUI.h"
#include "GUI/GUIFont.h"
#include "GUI/AllegroBitmap.h"

namespace RTE {

	AbstractClassInfo(Activity, Entity)

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

		for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			// Player 1 is active by default, for the editor etc
			m_IsActive[player] = player == Players::PlayerOne;
			m_IsHuman[player] = player == Players::PlayerOne;
			m_PlayerScreen[player] = player == Players::PlayerOne ? 0 : -1;
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

		for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
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

		m_TeamIcons[Team::TEAM_1] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1 Default"));
		m_TeamIcons[Team::TEAM_2] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2 Default"));
		m_TeamIcons[Team::TEAM_3] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3 Default"));
		m_TeamIcons[Team::TEAM_4] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4 Default"));

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

		for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
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

		for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
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

	int Activity::ReadProperty(std::string propName, Reader &reader) {
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
			for (unsigned short playerTeam = Teams::TeamOne; playerTeam < Teams::MaxTeamCount; playerTeam++) {
				std::string playerTeamNum = std::to_string(playerTeam + 1);
				if (propName == "TeamOfPlayer" + playerTeamNum) {
					unsigned short team;
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
			for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "Player" + playerNum + "IsHuman") {
					reader >> m_IsHuman[player];
					break;
				}
			}
		} else if (propName == "Team1Name" || propName == "Team2Name" || propName == "Team3Name" || propName == "Team4Name") {
			for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Name") {
					reader >> m_TeamNames[team];
					if (!m_TeamActive[team]) { m_TeamCount++; }
					m_TeamActive[team] = true;
					break;
				}
			}
		} else if (propName == "Team1Icon" || propName == "Team2Icon" || propName == "Team3Icon" || propName == "Team4Icon") {
			for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Icon") {
					reader >> m_TeamIcons[team];
					break;
				}
			}
		} else if (propName == "Team1Funds" || propName == "Team2Funds" || propName == "Team3Funds" || propName == "Team4Funds") {
			for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
				std::string teamNum = std::to_string(team + 1);
				if (propName == "Team" + teamNum + "Funds") {
					reader >> m_TeamFunds[team];
					break;
				}
			}
		} else if (propName == "TeamFundsShareOfPlayer1" || propName == "TeamFundsShareOfPlayer2" || propName == "TeamFundsShareOfPlayer3" || propName == "TeamFundsShareOfPlayer4") {
			for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "TeamFundsShareOfPlayer" + playerNum) {
					reader >> m_TeamFundsShare[player];
					break;
				}
			}
		} else if (propName == "FundsContributionOfPlayer1" || propName == "FundsContributionOfPlayer2" || propName == "FundsContributionOfPlayer3" || propName == "FundsContributionOfPlayer4") {
			for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
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

		for (unsigned short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player);
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

		for (unsigned short team = Teams::TeamOne; team < Teams::MaxTeamCount; team++) {
			std::string teamNum = std::to_string(team);
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

			m_PlayerController[player].Destroy();
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

	bool Activity::DeactivatePlayer(short playerToDeactivate) {
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

	short Activity::AddPlayer(short playerToAdd, bool isHuman, short team, int funds, const Icon *teamIcon) {
		if (playerToAdd < Players::PlayerOne || playerToAdd >= Players::MaxPlayerCount || team < Teams::TeamOne || team >= Teams::MaxTeamCount) {
			return m_PlayerCount;
		}

		if (!m_TeamActive[team]) {
			m_TeamFundsShare[playerToAdd] = 1.0F;
			m_TeamCount++;
		} else {
			// Team already existed, so adjust the contribution shares of all team members accordingly
			float totalFunds = m_TeamFunds[team] + static_cast<float>(funds);
			float newRatio = 1.0F + (static_cast<float>(funds) / totalFunds);
			// Go through all team members of this team, not including the one we're just adding (he's not activated yet)
			for (int p = Players::PlayerOne; p < Players::MaxPlayerCount; ++p) {
				if (m_IsActive[p] && m_Team[p] == team) { m_TeamFundsShare[p] /= newRatio; }
			}

			// The new added player's share
			m_TeamFundsShare[playerToAdd] = static_cast<float>(funds) / totalFunds;
		}
		m_TeamActive[team] = true;
		if (teamIcon) { SetTeamIcon(team, *teamIcon); }

		// Activate player after we did the above team fund share computation
		if (!m_IsActive[playerToAdd]) { m_PlayerCount++; }
		m_IsActive[playerToAdd] = true;

		m_IsHuman[playerToAdd] = isHuman;
		m_Team[playerToAdd] = team;
		m_TeamFunds[team] += static_cast<float>(funds);
		m_FundsContribution[playerToAdd] = static_cast<float>(funds);

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
		unsigned short humans = 0;
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_IsHuman[player]) { humans++; }
		}
		return humans;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetTeamOfPlayer(short player, short team) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return;
		}

		m_Team[player] = team;
		m_TeamActive[team] = true;
		m_IsActive[player] = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	short Activity::PlayerOfScreen(short screen) const {
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_PlayerScreen[player] == screen) {
				return player;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetTeamName(unsigned short whichTeam) const {
			return (whichTeam >= TEAM_1 && whichTeam < MAXTEAMCOUNT && m_TeamActive[whichTeam]) ? m_TeamNames[whichTeam] : "Inactive Team";
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
		} else {
			return "";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsPlayerTeam(short whichTeam) {
		if (whichTeam < Team::TEAM_1 || whichTeam >= Team::MAXTEAMCOUNT) {
			return false;
		}
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_Team[player] == whichTeam && m_IsHuman[player]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::PlayersInTeamCount(short team) const {
		int count = 0;
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_Team[player] == team) { count++; }
		}
		return count;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ChangeTeamFunds(float howMuch, unsigned short whichTeam) {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			m_TeamFunds[whichTeam] += howMuch;
			m_FundsChanged[whichTeam] = true;
			if (IsPlayerTeam(whichTeam)) { g_GUISound.FundsChangedSound()->Play(); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::TeamFundsChanged(unsigned short whichTeam) {
		if (whichTeam >= Teams::TeamOne && whichTeam < Teams::MaxTeamCount) {
			bool changed = m_FundsChanged[whichTeam];
			m_FundsChanged[whichTeam] = false;
			return changed;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::UpdatePlayerFundsContribution(short player, int newFunds) {
		// Player and his team have to already be active and kicking
		if (player < Players::PlayerOne || player >= Players::MaxPlayerCount || !m_IsActive[player] || !m_TeamActive[m_Team[player]]) {
			return false;
		}

		// If this guy is responsible for all funds, then just update his contribution and be done
		if (m_TeamFundsShare[player] == 1.0F) {
			m_FundsContribution[player] = newFunds;
			m_TeamFunds[m_Team[player]] = newFunds;
		} else {
			// Ok more complex situation of multiple players on this guy's team, so adjust all shares accordingly
			// Update the contribution of the player
			m_FundsContribution[player] = newFunds;

			// Tally up all the funds of all players on this guy's team
			for (int p = Players::PlayerOne; p < Players::MaxPlayerCount; ++p) {
				if (m_IsActive[p] && m_Team[p] == m_Team[player]) { m_TeamFunds[m_Team[player]] += m_FundsContribution[p]; }
			}
			// Now that we have the updated total, update the shares of all team players
			for (int p = Players::PlayerOne; p < Players::MaxPlayerCount; ++p) {
				if (m_IsActive[p] && m_Team[p] == m_Team[player]) { m_TeamFundsShare[p] = m_FundsContribution[p] / m_TeamFunds[m_Team[player]]; }
			}
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Activity::GetPlayerFundsShare(short player) const {
		if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
			return (m_FundsContribution[player] > 0) ? (m_TeamFunds[m_Team[player]] * m_TeamFundsShare[player]) : 0;
		} else {
			return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::HumanBrainCount() const {
		int brainCount = 0;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
				brainCount++;
			}
		}
		return brainCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::AIBrainCount() const {
		int brainCount = 0;

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player] && !m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
				brainCount++;
			}
		}
		return brainCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetPlayerBrain(Actor *newBrain, short player) {
		if (player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return;
		}

		if (newBrain) {
			newBrain->SetTeam(m_Team[player]);
			// Note that we have now had a brain
			m_HadBrain[player] = true;
		}

		m_Brain[player] = newBrain;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::AnyBrainWasEvacuated() const {
		for (int i = Players::PlayerOne; i < Players::MaxPlayerCount; ++i) {
			if (m_BrainEvacuated[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsAssignedBrain(Actor *actor) const {
		for (int i = Players::PlayerOne; i < Players::MaxPlayerCount; ++i) {
			if (actor == m_Brain[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::IsBrainOfWhichPlayer(Actor *actor) const {
		for (int i = Players::PlayerOne; i < Players::MaxPlayerCount; ++i) {
			if (actor == m_Brain[i]) {
				return i;
			}
		}
		return Players::NoPlayer;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsOtherPlayerBrain(Actor *actor, short player) const {
		for (int i = Players::PlayerOne; i < Players::MaxPlayerCount; ++i) {
			if (m_IsActive[i] && i != player && actor == m_Brain[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetDifficultyString(int difficulty) {
		if (difficulty < DifficultySetting::CakeDifficulty) {
			return "Cake";
		} else if (difficulty < DifficultySetting::EasyDifficulty) {
			return "Easy";
		} else if (difficulty < DifficultySetting::MediumDifficulty) {
			return "Medium";
		} else if (difficulty < DifficultySetting::HardDifficulty) {
			return "Hard";
		} else if (difficulty < DifficultySetting::NutsDifficulty) {
			return "Nuts";
		} else {
			return "Nuts!";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetDifficulty(int difficulty) {
		// Translate specific enums to be within their ranges, and not just at the limits
		if (difficulty == DifficultySetting::CakeDifficulty) {
			m_Difficulty = 10;
		} else if (difficulty == DifficultySetting::EasyDifficulty) {
			m_Difficulty = 30;
		} else if (difficulty == DifficultySetting::MediumDifficulty) {
			m_Difficulty = 50;
		} else if (difficulty == DifficultySetting::HardDifficulty) {
			m_Difficulty = 70;
		} else if (difficulty == DifficultySetting::NutsDifficulty) {
			m_Difficulty = 95;
		} else {
			m_Difficulty = difficulty;
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

			for (short playerTeam = Teams::TeamOne; playerTeam < Teams::MaxTeamCount; playerTeam++) {
				if (TeamActive(playerTeam)) {
					avgskill += GetTeamAISkill(playerTeam);
					count++;
				}
			}
			return (count > 0) ? avgskill / count : AISkillSetting::DefaultSkill;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetTeamAISkill(int team, int skill) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount) {
			return;
		}

		Limit(skill, AISkillSetting::UnfairSkill, 1);

		if (team >= Teams::TeamOne && team < Teams::MaxTeamCount) { m_TeamAISkillLevels[team] = skill; }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ReassignSquadLeader(const short player, const short team) {
		if (m_ControlledActor[player]->GetAIMode() == Actor::AIMODE_SQUAD) {
			// Get the id of the Squad Leader
			MOID leaderID = m_ControlledActor[player]->GetAIMOWaypointID();

			if (leaderID != g_NoMOID) {
				Actor *actor = g_MovableMan.GetNextTeamActor(team, m_ControlledActor[player]);

				do {
					// Set the controlled actor as new leader if actor follow the old leader, and not player controlled and not brain
					if (actor && (actor->GetAIMode() == Actor::AIMODE_SQUAD) && (actor->GetAIMOWaypointID() == leaderID) &&
						!actor->GetController()->IsPlayerControlled() && !actor->IsInGroup("Brains")) {
						actor->ClearAIWaypoints();
						actor->AddAIMOWaypoint(m_ControlledActor[player]);
						// Make sure actor has m_ControlledActor registered as an AIMOWaypoint
						actor->UpdateMovePath();
					} else if (actor && actor->GetID() == leaderID) {
						// Set the old leader to follow the controlled actor
						m_ControlledActor[player]->ClearAIWaypoints();
						// Inherit the old leader's AI mode
						m_ControlledActor[player]->SetAIMode((Actor::AIMode)actor->GetAIMode());

						if (m_ControlledActor[player]->GetAIMode() == Actor::AIMODE_GOTO) {
							// Copy the old leaders move orders
							if (actor->GetAIMOWaypointID() != g_NoMOID) {
								MovableObject * targetMO = g_MovableMan.GetMOFromID(actor->GetAIMOWaypointID());
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

	bool Activity::SwitchToActor(Actor *actor, short player, short team) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return false;
		}
		if (!actor || !g_MovableMan.IsActor(actor)) {
			return false;
		}

		if (!m_IsHuman[player]) {
			return false;
		}

		// If the desired actor is not the brain and is controlled by someone else OR if it's actually a brain of another player, we can't switch to it
		if (actor != m_Brain[player] && actor->IsPlayerControlled() || IsOtherPlayerBrain(m_ControlledActor[player], player)) {
			g_GUISound.UserErrorSound()->Play();
			return false;
		}

		const Actor *prevActor = 0;
		// Disable human player control of the actor we're switching from
		if (m_ControlledActor[player] && g_MovableMan.IsActor(m_ControlledActor[player])) {
			// Set AI controller active to the guy we're switching from
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_AI);
			m_ControlledActor[player]->GetController()->SetDisabled(false);
			prevActor = m_ControlledActor[player];
		}
		// Switch
		m_ControlledActor[player] = actor;

		// Now set the controller and team of the actor we switched to.
		if (m_ControlledActor[player]) {
			m_ControlledActor[player]->SetTeam(team);
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);
			m_ControlledActor[player]->GetController()->SetDisabled(false);

			// Play actor switching sound effects
			(m_ControlledActor[player] == m_Brain[player]) ? g_GUISound.BrainSwitchSound()->Play() : g_GUISound.ActorSwitchSound()->Play();
			// Only play air swoosh if actors are out of sight of each other
			if (prevActor && Vector(prevActor->GetPos() - m_ControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2) { g_GUISound.CameraTravelSound()->Play(); }

			ReassignSquadLeader(player, team);
		}
		// Follow the new guy normally
		m_ViewState[player] = Normal;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SwitchToNextActor(short player, short team, Actor *skip) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return;
		}

		Actor *prevActor = 0;
		// Disable human player control of the actor we're switching from
		if (m_ControlledActor[player] && g_MovableMan.IsActor(m_ControlledActor[player])) {
			// Set AI controller active to the guy we're switching from
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_AI);
			m_ControlledActor[player]->GetController()->SetDisabled(false);
			prevActor = m_ControlledActor[player];
		} else {
			m_ControlledActor[player] = 0;
		}

		// Get next Actor after the current one.
		m_ControlledActor[player] = g_MovableMan.GetNextTeamActor(team, m_ControlledActor[player]);

		Actor *start = m_ControlledActor[player];

		// If it's the skip guy, then skip him OR if it's a non-brain guy controlled by another player on the same team, then skip him too
		while ((skip && skip == m_ControlledActor[player]) || (m_ControlledActor[player] && m_ControlledActor[player] != m_Brain[player] && m_ControlledActor[player]->IsPlayerControlled()) || (IsOtherPlayerBrain(m_ControlledActor[player], player))) {
			// Check previous in line
			m_ControlledActor[player] = g_MovableMan.GetNextTeamActor(team, m_ControlledActor[player]);

			// Looped around the whole actor chain, and couldn't find an available actor, so switch back to the original
			if (m_ControlledActor[player] == prevActor) {
				g_GUISound.UserErrorSound()->Play();
				m_ControlledActor[player] = prevActor;
				break;
			}
			// If we're gone around without even finding the one we started with (he might be dead),
			// then try to switch to the brain, and if not that, then just the start
			if (m_ControlledActor[player] == start) {
				if (m_Brain[player]) {
					m_ControlledActor[player] = m_Brain[player];
				} else if (g_MovableMan.IsActor(start)) {
					m_ControlledActor[player] = start;
				} else {
					m_ControlledActor[player] = 0;
				}
				break;
			}
		}

		// Now set the controller of the actor we switched to, and play switching sound effects
		if (m_ControlledActor[player]) {
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);

			// Play actor switching sound effects
			(m_ControlledActor[player] == m_Brain[player]) ? g_GUISound.BrainSwitchSound()->Play() : g_GUISound.ActorSwitchSound()->Play();
			// Only play air swoosh if actors are out of sight of each other
			if (prevActor && Vector(prevActor->GetPos() - m_ControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2) { g_GUISound.CameraTravelSound()->Play(); }

			// Follow the new guy normally
			m_ViewState[player] = Normal;

			ReassignSquadLeader(player, team);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SwitchToPrevActor(short player, short team, Actor *skip) {
		if (team < Teams::TeamOne || team >= Teams::MaxTeamCount || player < Players::PlayerOne || player >= Players::MaxPlayerCount) {
			return;
		}
		Actor *prevActor = 0;
		// Disable human player control of the actor we're switching from
		if (m_ControlledActor[player] && g_MovableMan.IsActor(m_ControlledActor[player])) {
			// Set AI controller active to the guy we're switching from
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_AI);
			m_ControlledActor[player]->GetController()->SetDisabled(false);
			prevActor = m_ControlledActor[player];
		} else {
			m_ControlledActor[player] = 0;
		}

		// Get previous Actor before the current one.
		m_ControlledActor[player] = g_MovableMan.GetPrevTeamActor(team, m_ControlledActor[player]);

		// If it's the skip guy, then skip him OR if it's a non-brain guy controlled by another player on the same team, then skip him too OR if it's a brain of another player
		Actor *start = m_ControlledActor[player];
		while ((skip && skip == m_ControlledActor[player]) || (m_ControlledActor[player] && m_ControlledActor[player] != m_Brain[player] && m_ControlledActor[player]->IsPlayerControlled()) || (IsOtherPlayerBrain(m_ControlledActor[player], player))) {
			// Check previous in line
			m_ControlledActor[player] = g_MovableMan.GetPrevTeamActor(team, m_ControlledActor[player]);

			// Looped around the whole actor chain, and couldn't find an available actor, so switch back to the original
			if (m_ControlledActor[player] == prevActor) {
				g_GUISound.UserErrorSound()->Play();
				m_ControlledActor[player] = prevActor;
				break;
			}
			// If we're gone around without even finding the one we started with (he might be dead), then try to switch to the brain, and if not that, then 0
			if (m_ControlledActor[player] == start) {
				if (m_Brain[player]) {
					m_ControlledActor[player] = m_Brain[player];
				} else if (g_MovableMan.IsActor(start)) {
					m_ControlledActor[player] = start;
				} else {
					m_ControlledActor[player] = 0;
				}
				break;
			}
		}

		// Now set the controller of the actor we switched to, and play switching sound effects
		if (m_ControlledActor[player]) {
			m_ControlledActor[player]->SetControllerMode(Controller::CIM_PLAYER, player);

			// Play actor switching sound effects
			(m_ControlledActor[player] == m_Brain[player]) ? g_GUISound.BrainSwitchSound()->Play() : g_GUISound.ActorSwitchSound()->Play();
			// Only play air swoosh if actors are out of sight of each other
			if (prevActor && Vector(prevActor->GetPos() - m_ControlledActor[player]->GetPos()).GetMagnitude() > g_FrameMan.GetResX() / 2) { g_GUISound.CameraTravelSound()->Play(); }

			ReassignSquadLeader(player, team);
		}

		// Follow the new guy normally
		m_ViewState[player] = Normal;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::EnteredOrbit(Actor *orbitedCraft) {
		if (!orbitedCraft) {
			return;
		}
		int team = orbitedCraft->GetTeam();

		// Find out cost multipliers for metagame, or we end up AI's earning money when their craft return if their nativeCostMultiplier is too low
		float foreignCostMult = 1.0F;
		float nativeCostMult = 1.0F;
		if (g_MetaMan.GameInProgress()) {
			for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				if (GetTeamOfPlayer(player) == orbitedCraft->GetTeam()) {
					const MetaPlayer *metaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
					if (metaPlayer) {
						foreignCostMult = metaPlayer->GetForeignCostMultiplier();
						nativeCostMult = metaPlayer->GetNativeCostMultiplier();
					}
				}
			}
		}

		// Did a brain just evacuate the Scene??
		bool brainOnBoard = orbitedCraft->HasObjectInGroup("Brains");
		// Total value of ship and cargo and crew
		float totalValue = orbitedCraft->GetTotalValue(0, foreignCostMult, nativeCostMult);
		// TODO ARGH WHAT IF TWO PLAYERS ON SAME TEAM ARE OF DIFFERENT TECHSS??!?!?!?$?!?!$?!$?
		// A: Just let the base cost be the liquidation value.. they could cheat otherwise, one buying, one selling
		char str[64];
		sprintf_s(str, sizeof(str), "Returned Craft + Cargo added %.0f oz to Funds!", totalValue);
		for (short player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_IsActive[player]) {
				// Figure out whose brain just left the building
				if (brainOnBoard && orbitedCraft == GetPlayerBrain(player)) {
					m_BrainEvacuated[player] = true;
					sprintf_s(str, sizeof(str), "YOUR BRAIN HAS BEEN EVACUATED BACK INTO ORBIT!");
				}

				if (m_Team[player] == team) {
					g_FrameMan.ClearScreenText(ScreenOfPlayer(ScreenOfPlayer(player)));
					g_FrameMan.SetScreenText(str, ScreenOfPlayer(player), 0, 3500);
					m_MessageTimer[player].Reset();
				}
			}
		}

		m_TeamFunds[team] += totalValue;
		orbitedCraft->SetGoldCarried(0);
		orbitedCraft->SetHealth(orbitedCraft->GetMaxHealth());

		// The craft entering orbit will count as a death for the team because it's being deleted, so we need to decrement the team's death count to keep it correct.
		m_TeamDeaths[team]--;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::Update() {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_MessageTimer[player].IsPastSimMS(5000)) { g_FrameMan.ClearScreenText(ScreenOfPlayer(player)); }
			if (m_IsActive[player]) { m_PlayerController[player].Update(); }
		}
	}
}