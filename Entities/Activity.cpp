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
		m_Description.clear();
		m_InCampaignStage = -1;
		m_MaxPlayerSupport = MAXPLAYERCOUNT;
		m_MinTeamsRequired = 2;
		m_ActivityState = NOTSTARTED;
		m_Paused = false;
		m_TeamCount = 1;
		m_PlayerCount = 1;
		m_Difficulty = 50;
		m_CraftOrbitAtTheEdge = false;

		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			m_Team[player] = Activity::TEAM_1;
			m_FundsContribution[player] = 0;
			m_TeamFundsShare[player] = 1.0;
			// Player 1 is active by default, for the editor etc
			m_IsActive[player] = player == PLAYER_1;
			m_IsHuman[player] = player == PLAYER_1;
			m_PlayerScreen[player] = player == PLAYER_1 ? 0 : -1;
			m_Brain[player] = 0;
			m_HadBrain[player] = false;
			m_BrainEvacuated[player] = false;
			m_ViewState[player] = NORMAL;
			m_ControlledActor[player] = 0;
			m_PlayerController[player].Reset();
			m_MsgTimer[player].Reset();
		}

		for (int team = 0; team < MAXTEAMCOUNT; ++team) {
			// Team 1 is active by default, for the editors etc
			m_TeamActive[team] = team == TEAM_1;
			m_TeamFunds[team] = 2000;
			m_FundsChanged[team] = false;
			m_TeamIcons[team].Reset();
			m_TeamDeaths[team] = 0;
			m_TeamAISkillLevels[team] = DEFAULTSKILL;
		}

		m_TeamNames[TEAM_1] = "Team 1";
		m_TeamNames[TEAM_2] = "Team 2";
		m_TeamNames[TEAM_3] = "Team 3";
		m_TeamNames[TEAM_4] = "Team 4";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Create() {
		if (Entity::Create() < 0) {
			return -1;
		}

		m_TeamIcons[Activity::TEAM_1] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 1 Default"));
		m_TeamIcons[Activity::TEAM_2] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 2 Default"));
		m_TeamIcons[Activity::TEAM_3] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 3 Default"));
		m_TeamIcons[Activity::TEAM_4] = *dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team 4 Default"));

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Create(const Activity &reference) {
		Entity::Create(reference);

		m_Description = reference.m_Description;
		m_InCampaignStage = reference.m_InCampaignStage;
		m_MaxPlayerSupport = reference.m_MaxPlayerSupport;
		m_MinTeamsRequired = reference.m_MinTeamsRequired;
		m_ActivityState = reference.m_ActivityState;
		m_Paused = reference.m_Paused;
		m_TeamCount = reference.m_TeamCount;
		m_PlayerCount = reference.m_PlayerCount;
		m_Difficulty = reference.m_Difficulty;
		m_CraftOrbitAtTheEdge = reference.m_CraftOrbitAtTheEdge;

		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
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

		for (int team = 0; team < MAXTEAMCOUNT; ++team) {
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
		} else if (propName == "InCampaignStage") {
			reader >> m_InCampaignStage;
		} else if (propName == "MaxPlayerSupport") {
			reader >> m_MaxPlayerSupport;
		} else if (propName == "MinTeamsRequired") {
			reader >> m_MinTeamsRequired;

		// Gets tallied as team assignments are read in below, so doesn't really matter here, but should still be handled
		} else if (propName == "TeamCount") {
			reader.ReadPropValue(); //reader >> m_TeamCount;

		} else if (propName == "PlayerCount") {
			reader >> m_PlayerCount;
		} else if (propName == "Difficulty") {
			reader >> m_Difficulty;
		} else if (propName == "CraftOrbitAtTheEdge") {
			reader >> m_CraftOrbitAtTheEdge;
		} else if (propName == "TeamOfPlayer1") {
			short team;
			reader >> team;
			if (team >= TEAM_1 && team < MAXTEAMCOUNT) {
				m_Team[PLAYER_1] = team;
				m_IsActive[PLAYER_1] = true;
				if (!m_TeamActive[team]) { m_TeamCount++; }
				m_TeamActive[team] = true;
			} else {
				// Non-active player if team is out of range
				m_IsActive[PLAYER_1] = false;
			}
		} else if (propName == "TeamOfPlayer2") {
			short team;
			reader >> team;
			if (team >= TEAM_1 && team < MAXTEAMCOUNT) {
				m_Team[PLAYER_2] = team;
				m_IsActive[PLAYER_2] = true;
				if (!m_TeamActive[team]) { m_TeamCount++; }
				m_TeamActive[team] = true;
			} else {
				m_IsActive[PLAYER_2] = false;
			}
		} else if (propName == "TeamOfPlayer3") {
			short team;
			reader >> team;
			if (team >= TEAM_1 && team < MAXTEAMCOUNT) {
				m_Team[PLAYER_3] = team;
				m_IsActive[PLAYER_3] = true;
				if (!m_TeamActive[team]) { m_TeamCount++; }
				m_TeamActive[team] = true;
			} else {
				m_IsActive[PLAYER_3] = false;
			}
		} else if (propName == "TeamOfPlayer4") {
			short team;
			reader >> team;
			if (team >= TEAM_1 && team < MAXTEAMCOUNT) {
				m_Team[PLAYER_4] = team;
				m_IsActive[PLAYER_4] = true;
				if (!m_TeamActive[team]) { m_TeamCount++; }				
				m_TeamActive[team] = true;
			} else {
				m_IsActive[PLAYER_4] = false;
			}
		} else if (propName == "FundsContributionOfPlayer1") {
			reader >> m_FundsContribution[PLAYER_1];
		} else if (propName == "FundsContributionOfPlayer2") {
			reader >> m_FundsContribution[PLAYER_2];
		} else if (propName == "FundsContributionOfPlayer3") {
			reader >> m_FundsContribution[PLAYER_3];
		} else if (propName == "FundsContributionOfPlayer4") {
			reader >> m_FundsContribution[PLAYER_4];
		} else if (propName == "TeamFundsShareOfPlayer1") {
			reader >> m_TeamFundsShare[PLAYER_1];
		} else if (propName == "TeamFundsShareOfPlayer2") {
			reader >> m_TeamFundsShare[PLAYER_2];
		} else if (propName == "TeamFundsShareOfPlayer3") {
			reader >> m_TeamFundsShare[PLAYER_3];
		} else if (propName == "TeamFundsShareOfPlayer4") {
			reader >> m_TeamFundsShare[PLAYER_4];
		} else if (propName == "Player1IsHuman") {
			reader >> m_IsHuman[PLAYER_1];
		} else if (propName == "Player2IsHuman") {
			reader >> m_IsHuman[PLAYER_2];
		} else if (propName == "Player3IsHuman") {
			reader >> m_IsHuman[PLAYER_3];
		} else if (propName == "Player4IsHuman") {
			reader >> m_IsHuman[PLAYER_4];
		} else if (propName == "Team1Funds" || propName == "FundsOfTeam1") {
			reader >> m_TeamFunds[TEAM_1];
		} else if (propName == "Team2Funds" || propName == "FundsOfTeam2") {
			reader >> m_TeamFunds[TEAM_2];
		} else if (propName == "Team3Funds" || propName == "FundsOfTeam3") {
			reader >> m_TeamFunds[TEAM_3];
		} else if (propName == "Team4Funds" || propName == "FundsOfTeam4") {
			reader >> m_TeamFunds[TEAM_4];
		} else if (propName == "Team1Name") {
			reader >> m_TeamNames[TEAM_1];
			if (!m_TeamActive[TEAM_1]) { m_TeamCount++; }
			m_TeamActive[TEAM_1] = true;
		} else if (propName == "Team2Name") {
			reader >> m_TeamNames[TEAM_2];
			if (!m_TeamActive[TEAM_2]) { m_TeamCount++; }				
			m_TeamActive[TEAM_2] = true;
		} else if (propName == "Team3Name") {
			reader >> m_TeamNames[TEAM_3];
			if (!m_TeamActive[TEAM_3]) { m_TeamCount++; }	
			m_TeamActive[TEAM_3] = true;
		} else if (propName == "Team4Name") {
			reader >> m_TeamNames[TEAM_4];
			if (!m_TeamActive[TEAM_4]) { m_TeamCount++; }
			m_TeamActive[TEAM_4] = true;
		} else if (propName == "Team1Icon") {
			reader >> m_TeamIcons[TEAM_1];
		} else if (propName == "Team2Icon") {
			reader >> m_TeamIcons[TEAM_2];
		} else if (propName == "Team3Icon") {
			reader >> m_TeamIcons[TEAM_3];
		} else if (propName == "Team4Icon") {
			reader >> m_TeamIcons[TEAM_4];
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
		writer.NewProperty("InCampaignStage");
		writer << m_InCampaignStage;
		writer.NewProperty("MaxPlayerSupport");
		writer << m_MaxPlayerSupport;
		writer.NewProperty("MinTeamsRequired");
		writer << m_MinTeamsRequired;
		writer.NewProperty("TeamCount");
		writer << m_TeamCount;
		writer.NewProperty("PlayerCount");
		writer << m_PlayerCount;
		writer.NewProperty("Difficulty");
		writer << m_Difficulty;
		writer.NewProperty("CraftOrbitAtTheEdge");
		writer << m_CraftOrbitAtTheEdge;

		if (m_IsActive[PLAYER_1]) {
			writer.NewProperty("TeamOfPlayer1");
			writer << m_Team[PLAYER_1];
			writer.NewProperty("FundsContributionOfPlayer1");
			writer << m_FundsContribution[PLAYER_1];
			writer.NewProperty("TeamFundsShareOfPlayer1");
			writer << m_TeamFundsShare[PLAYER_1];
			writer.NewProperty("Player1IsHuman");
			writer << m_IsHuman[PLAYER_1];
		} else {
			writer.NewProperty("TeamOfPlayer1");
			writer << NOTEAM;
		}

		if (m_IsActive[PLAYER_2]) {
			writer.NewProperty("TeamOfPlayer2");
			writer << m_Team[PLAYER_2];
			writer.NewProperty("FundsContributionOfPlayer2");
			writer << m_FundsContribution[PLAYER_2];
			writer.NewProperty("TeamFundsShareOfPlayer2");
			writer << m_TeamFundsShare[PLAYER_2];
			writer.NewProperty("Player2IsHuman");
			writer << m_IsHuman[PLAYER_2];
		} else {
			writer.NewProperty("TeamOfPlayer2");
			writer << NOTEAM;
		}

		if (m_IsActive[PLAYER_3]) {
			writer.NewProperty("TeamOfPlayer3");
			writer << m_Team[PLAYER_3];
			writer.NewProperty("FundsContributionOfPlayer3");
			writer << m_FundsContribution[PLAYER_3];
			writer.NewProperty("TeamFundsShareOfPlayer3");
			writer << m_TeamFundsShare[PLAYER_3];
			writer.NewProperty("Player3IsHuman");
			writer << m_IsHuman[PLAYER_3];
		} else {
			writer.NewProperty("TeamOfPlayer3");
			writer << NOTEAM;
		}

		if (m_IsActive[PLAYER_4]) {
			writer.NewProperty("TeamOfPlayer4");
			writer << m_Team[PLAYER_4];
			writer.NewProperty("FundsContributionOfPlayer4");
			writer << m_FundsContribution[PLAYER_4];
			writer.NewProperty("TeamFundsShareOfPlayer4");
			writer << m_TeamFundsShare[PLAYER_4];
			writer.NewProperty("Player4IsHuman");
			writer << m_IsHuman[PLAYER_4];
		} else {
			writer.NewProperty("TeamOfPlayer4");
			writer << NOTEAM;
		}

		if (m_TeamActive[Activity::TEAM_1]) {
			writer.NewProperty("Team1Funds");
			writer << m_TeamFunds[TEAM_1];
			writer.NewProperty("Team1Name");
			writer << m_TeamNames[TEAM_1];
			writer.NewProperty("Team1Icon");
			m_TeamIcons[TEAM_1].SavePresetCopy(writer);
		}

		if (m_TeamActive[Activity::TEAM_2]) {
			writer.NewProperty("Team2Funds");
			writer << m_TeamFunds[TEAM_2];
			writer.NewProperty("Team2Name");
			writer << m_TeamNames[TEAM_2];
			writer.NewProperty("Team2Icon");
			m_TeamIcons[TEAM_2].SavePresetCopy(writer);
		}

		if (m_TeamActive[Activity::TEAM_3]) {
			writer.NewProperty("Team3Funds");
			writer << m_TeamFunds[TEAM_3];
			writer.NewProperty("Team3Name");
			writer << m_TeamNames[TEAM_3];
			writer.NewProperty("Team3Icon");
			m_TeamIcons[TEAM_3].SavePresetCopy(writer);
		}

		if (m_TeamActive[Activity::TEAM_4]) {
			writer.NewProperty("Team4Funds");
			writer << m_TeamFunds[TEAM_4];
			writer.NewProperty("Team4Name");
			writer << m_TeamNames[TEAM_4];
			writer.NewProperty("Team4Icon");
			m_TeamIcons[TEAM_4].SavePresetCopy(writer);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::Start() {
		m_ActivityState = RUNNING;
		m_Paused = false;

		// Reset the mouse moving so that it won't trap the mouse if the window isn't in focus (common after loading)
		if (!g_FrameMan.IsInMultiplayerMode()) {
			g_UInputMan.DisableMouseMoving(true);
			g_UInputMan.DisableMouseMoving(false);

			// Enable keys again
			g_UInputMan.DisableKeys(false);
		}

		int error = g_SceneMan.LoadScene();
		if (error < 0) {
			return error;
		}

		for (int team = 0; team < MAXTEAMCOUNT; ++team) {
			if (!m_TeamActive[team]) {
				continue;
			}
			m_FundsChanged[team] = false;
			m_TeamDeaths[team] = 0;
		}

		// Intentionally doing all players, all need controllers
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			m_ViewState[player] = NORMAL;
			g_FrameMan.ClearScreenText(ScreenOfPlayer(player));
			g_SceneMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(player));

			m_PlayerController[player].Destroy();
			m_PlayerController[player].Create(Controller::CIM_PLAYER, player);
			m_PlayerController[player].SetTeam(m_Team[player]);

			m_MsgTimer[player].Reset();
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::End() {
		// Disable control of actors. will be handed over to the observation targets instead
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			m_ViewState[player] = OBSERVE;
		}
		m_ActivityState = OVER;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetupPlayers() {
		// Clear out the settings, we'll set them up again
		m_TeamCount = 0;
		m_PlayerCount = 0;

		// Don't clear a CPU team's active status though
		for (int team = Activity::TEAM_1; team < MAXTEAMCOUNT; ++team) {
			m_TeamActive[team] = false;
		}

		for (int player = Activity::PLAYER_1; player < MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player]) {
				m_PlayerCount++;
				if (!m_TeamActive[m_Team[player]]) { m_TeamCount++; }			
				m_TeamActive[m_Team[player]] = true;
			}

			// Calculate which screen each human player is using, based on how many non-human players there are before him
			int screenIndex = -1;
			if (m_IsActive[player] && m_IsHuman[player]) {
				for (int p = Activity::PLAYER_1; p < MAXPLAYERCOUNT && p <= player; ++p) {
					if (m_IsActive[p] && m_IsHuman[p]) { screenIndex++; }				
				}
			}
			m_PlayerScreen[player] = screenIndex;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::DeactivatePlayer(int player) {
		if (player < PLAYER_1 || player >= MAXPLAYERCOUNT) {
			return false;
		}
		if (!m_IsActive[player] || !m_TeamActive[m_Team[player]]) {
			return false;
		}

		// See if this was the last player on its team
		bool lastOnTeam = true;

		for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p) {
			if (m_IsActive[p] && p != player && m_Team[p] == m_Team[player]) { lastOnTeam = false; }		
		}

		// If we were the last, then deactivate the team as well
		if (lastOnTeam) {
			m_TeamActive[m_Team[player]] = false;
			m_TeamCount--;
		}

		// Deactivate the player
		m_IsActive[player] = false;
		m_PlayerCount--;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::AddPlayer(int player, bool isHuman, int team, int funds, const Icon *teamIcon) {
		if (player < PLAYER_1 || player >= MAXPLAYERCOUNT || team < TEAM_1 || team >= MAXTEAMCOUNT) {
			return m_PlayerCount;
		}

		// Activate team
		if (!m_TeamActive[team]) {
			// This guy is responsible for all this team's funds so far
			m_TeamFundsShare[player] = 1.0F;
			m_TeamCount++;
		} else {
			// Team already existed, so adjust the contribution shares of all team members accordingly
			float totalFunds = m_TeamFunds[team] + funds;
			float newRatio = 1.0 + (funds / totalFunds);
			// Go through all team members of this team, not including the one we're just adding (he's not activated yet)
			for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p) {
				if (m_IsActive[p] && m_Team[p] == team) { m_TeamFundsShare[p] /= newRatio; }					
			}

			// The new added player's share
			m_TeamFundsShare[player] = funds / totalFunds;
		}
		m_TeamActive[team] = true;
		if (teamIcon) { SetTeamIcon(team, *teamIcon); }
			
		// Activate player after we did the above team fund share computation
		if (!m_IsActive[player]) { m_PlayerCount++; }		
		m_IsActive[player] = true;

		m_IsHuman[player] = isHuman;
		m_Team[player] = team;
		m_TeamFunds[team] += funds;
		m_FundsContribution[player] = funds;

		return m_PlayerCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ClearPlayers(bool resetFunds) {
		for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player) {
			m_IsActive[player] = false;
			m_IsHuman[player] = false;

			if (resetFunds) {
				m_FundsContribution[player] = 0;
				m_TeamFundsShare[player] = 1.0F;
			}
		}

		for (int team = TEAM_1; team < MAXTEAMCOUNT; ++team) {
			m_TeamActive[team] = false;
			if (resetFunds) { m_TeamFunds[team] = 0; }			
		}

		m_PlayerCount = m_TeamCount = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::GetHumanCount() const {
		unsigned short humans = 0;
		for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player] && m_IsHuman[player]) { humans++; }			
		}
		return humans;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetTeamOfPlayer(int player, int team) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT) {
			return;
		}

		m_Team[player] = team;
		m_TeamActive[team] = true;
		m_IsActive[player] = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::PlayerOfScreen(int screen) const {
		for (int player = PLAYER_1; player < MAXPLAYERCOUNT; ++player) {
			if (m_PlayerScreen[player] == screen) {
				return player;
			}
		}
		return NOPLAYER;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetTeamName(unsigned int which) const {
		if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) {
			return (which >= TEAM_1 && which < MAXTEAMCOUNT && m_TeamActive[which]) ? m_TeamNames[which] : "Inactive Team";
		} else {
			return "";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Icon * Activity::GetTeamIcon(unsigned int which) const {
		if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) {
			return (which >= TEAM_1 && which < MAXTEAMCOUNT) ? &(m_TeamIcons[which]) : 0;
		} else {
			return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsPlayerTeam(int team) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT) {
			return false;
		}
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player] && m_Team[player] == team && m_IsHuman[player]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::PlayersInTeamCount(int team) const {
		int count = 0;
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player] && m_Team[player] == team) { count++; }				
		}
		return count;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ChangeTeamFunds(float howMuch, unsigned int which) {
		if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) {
			m_TeamFunds[which] += howMuch;
			m_FundsChanged[which] = true;
			if (IsPlayerTeam(which)) { g_GUISound.FundsChangedSound()->Play(); }				
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::TeamFundsChanged(unsigned int which) {
		if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) {
			bool changed = m_FundsChanged[which];
			m_FundsChanged[which] = false;
			return changed;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::UpdatePlayerFundsContribution(int player, int newFunds) {
		// Player and his team have to already be active and kicking
		if (player < PLAYER_1 || player >= MAXPLAYERCOUNT || !m_IsActive[player] || !m_TeamActive[m_Team[player]]) {
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
			for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p) {
				if (m_IsActive[p] && m_Team[p] == m_Team[player]) { m_TeamFunds[m_Team[player]] += m_FundsContribution[p]; }			
			}
			// Now that we have the updated total, update the shares of all team players
			for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p) {
				if (m_IsActive[p] && m_Team[p] == m_Team[player]) { m_TeamFundsShare[p] = m_FundsContribution[p] / m_TeamFunds[m_Team[player]]; }				
			}
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float Activity::GetPlayerFundsShare(int player) const {
		if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) {
			return (m_FundsContribution[player] > 0) ? (m_TeamFunds[m_Team[player]] * m_TeamFundsShare[player]) : 0;
		} else {
			return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::HumanBrainCount() {
		int brainCount = 0;

		for (int player = Activity::PLAYER_1; player < Activity::MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player] && m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
				brainCount++;
			}
		}
		return brainCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::AIBrainCount() {
		int brainCount = 0;

		for (int player = Activity::PLAYER_1; player < Activity::MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player] && !m_IsHuman[player] && m_HadBrain[player] && (g_MovableMan.IsActor(m_Brain[player]) || (m_Brain[player] && m_Brain[player]->HasObjectInGroup("Brains")))) {
				brainCount++;
			}
		}
		return brainCount;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetPlayerBrain(Actor *pNewBrain, int player) {
		if (player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT) {
			return;
		}

		if (pNewBrain) {
			pNewBrain->SetTeam(m_Team[player]);
			// Note that we have now had a brain
			m_HadBrain[player] = true;
		}

		m_Brain[player] = pNewBrain;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::AnyEvacuees() const {
		for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
			if (m_BrainEvacuated[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsAssignedBrain(Actor *actor) const {
		for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
			if (actor == m_Brain[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::IsBrainOfWhichPlayer(Actor *actor) const {
		for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
			if (actor == m_Brain[i]) {
				return i;
			}
		}
		return NOPLAYER;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Activity::IsOtherPlayerBrain(Actor *actor, int player) const {
		for (int i = 0; i < MAXPLAYERCOUNT; ++i) {
			if (m_IsActive[i] && i != player && actor == m_Brain[i]) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetDifficultyString(int difficulty) {
		if (difficulty < CAKEDIFFICULTY) {
			return "Cake";
		} else if (difficulty < EASYDIFFICULTY) {
			return "Easy";
		} else if (difficulty < MEDIUMDIFFICULTY) {
			return "Medium";
		} else if (difficulty < HARDDIFFICULTY) {
			return "Hard";
		} else if (difficulty < NUTSDIFFICULTY) {
			return "Nuts";
		} else {
			return "Nuts!";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetDifficulty(int difficulty) {
		// Translate specific enums to be within their ranges, and not just at the limits
		if (difficulty == CAKEDIFFICULTY) {
			m_Difficulty = 10;
		} else if (difficulty == EASYDIFFICULTY) {
			m_Difficulty = 30;
		} else if (difficulty == MEDIUMDIFFICULTY) {
			m_Difficulty = 50;
		} else if (difficulty == HARDDIFFICULTY) {
			m_Difficulty = 70;
		} else if (difficulty == NUTSDIFFICULTY) {
			m_Difficulty = 95;
		} else {
			m_Difficulty = difficulty;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Activity::GetAISkillString(int skill) {
		if (skill < INFERIORSKILL) {
			return "Inferior";
		} else if (skill < AVERAGESKILL) {
			return "Average";
		} else if (skill < GOODSKILL) {
			return "Good";
		} else {
			return "Unfair";
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Activity::GetTeamAISkill(int team) const {
		if (team >= TEAM_1 && team < MAXTEAMCOUNT) {
			return m_TeamAISkillLevels[team];
		} else {
			int avgskill = 0;
			int count = 0;

			for (int playerTeam = TEAM_1; playerTeam < MAXTEAMCOUNT; playerTeam++) {
				if (TeamActive(playerTeam)) {
					avgskill += GetTeamAISkill(playerTeam);
					count++;
				}
			}
			return (count > 0) ? avgskill / count : DEFAULTSKILL;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SetTeamAISkill(int team, int skill) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT) {
			return;
		}

		Limit(skill, UNFAIRSKILL, 1);

		if (team >= TEAM_1 && team < MAXTEAMCOUNT) { m_TeamAISkillLevels[team] = skill; }		
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::ReassignSquadLeader(const int player, const int team) {
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

	bool Activity::SwitchToActor(Actor *actor, int player, int team) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT) {
			return false;
		}
		if (!actor || !g_MovableMan.IsActor(actor)) {
			return false;
		}
		// Computer players don't focus on any Actor
		if (!m_IsHuman[player]) {
			return false;
		}

		// If the desired actor is not the brain and is controlled by someone else, 
		// OR if it's actually a brain of another player, we can't switch to it
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
		m_ViewState[player] = NORMAL;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SwitchToNextActor(int player, int team, Actor *skip) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT) {
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
			m_ViewState[player] = NORMAL;

			ReassignSquadLeader(player, team);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::SwitchToPrevActor(int player, int team, Actor *skip) {
		if (team < Activity::TEAM_1 || team >= Activity::MAXTEAMCOUNT || player < Activity::PLAYER_1 || player >= Activity::MAXPLAYERCOUNT) {
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
		m_ViewState[player] = NORMAL;
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
			for (int player = 0; player < MAXPLAYERCOUNT; player++) {
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
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			if (m_IsActive[player]) {
				// Figure out whose brain just left the building
				if (brainOnBoard && orbitedCraft == GetPlayerBrain(player)) {
					m_BrainEvacuated[player] = true;
					sprintf_s(str, sizeof(str), "YOUR BRAIN HAS BEEN EVACUATED BACK INTO ORBIT!");
				}

				if (m_Team[player] == team) {
					g_FrameMan.ClearScreenText(ScreenOfPlayer(ScreenOfPlayer(player)));
					g_FrameMan.SetScreenText(str, ScreenOfPlayer(player), 0, 3500);
					m_MsgTimer[player].Reset();
				}
			}
		}

		m_TeamFunds[team] += totalValue;
		orbitedCraft->SetGoldCarried(0);
		orbitedCraft->SetHealth(orbitedCraft->GetMaxHealth());

		// Counter-adjust the death toll because the craft leaving (being deleted) will increment
		m_TeamDeaths[team]--;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Activity::Update() {
		for (int player = 0; player < MAXPLAYERCOUNT; ++player) {
			if (m_MsgTimer[player].IsPastSimMS(5000)) { g_FrameMan.ClearScreenText(ScreenOfPlayer(player)); }
			if (m_IsActive[player]) { m_PlayerController[player].Update(); }
		}
	}
}