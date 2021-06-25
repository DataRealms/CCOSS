#ifndef _RTELUABINDACTIVITIES_
#define _RTELUABINDACTIVITIES_

#include "LuaMacros.h"

#include "Entity.h"
#include "Activity.h"
#include "GameActivity.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	struct ActivityLuaBindings {

		LuaBindingRegisterFunctionForType(Activity) {
			return luabind::class_<Activity, Entity>("Activity")
				.enum_("Players")[
					value("PLAYER_NONE", Players::NoPlayer),
					value("PLAYER_1", Players::PlayerOne),
					value("PLAYER_2", Players::PlayerTwo),
					value("PLAYER_3", Players::PlayerThree),
					value("PLAYER_4", Players::PlayerFour),
					value("MAXPLAYERCOUNT", Players::MaxPlayerCount)
				]
				.enum_("ActivityState")[
					value("NOACTIVITY", Activity::ActivityState::NoActivity),
					value("NOTSTARTED", Activity::ActivityState::NotStarted),
					value("STARTING", Activity::ActivityState::Starting),
					value("EDITING", Activity::ActivityState::Editing),
					value("PREGAME", Activity::ActivityState::PreGame),
					value("RUNNING", Activity::ActivityState::Running),
					value("INERROR", Activity::ActivityState::HasError),
					value("OVER", Activity::ActivityState::Over)
				]
				.enum_("Team")[
					value("NOTEAM", Activity::Teams::NoTeam),
					value("TEAM_1", Activity::Teams::TeamOne),
					value("TEAM_2", Activity::Teams::TeamTwo),
					value("TEAM_3", Activity::Teams::TeamThree),
					value("TEAM_4", Activity::Teams::TeamFour),
					value("MAXTEAMCOUNT", Activity::Teams::MaxTeamCount)
				]
				.enum_("ViewState")[
					value("NORMAL", Activity::ViewState::Normal),
					value("OBSERVE", Activity::ViewState::Observe),
					value("DEATHWATCH", Activity::ViewState::DeathWatch),
					value("ACTORSELECT", Activity::ViewState::ActorSelect),
					value("AISENTRYPOINT", Activity::ViewState::AISentryPoint),
					value("AIPATROLPOINTS", Activity::ViewState::AIPatrolPoints),
					value("AIGOLDDIGPOINT", Activity::ViewState::AIGoldDigPoint),
					value("AIGOTOPOINT", Activity::ViewState::AIGoToPoint),
					value("LZSELECT", Activity::ViewState::LandingZoneSelect)
				]
				.enum_("DifficultySetting")[
					value("MINDIFFICULTY", Activity::DifficultySetting::MinDifficulty),
					value("CAKEDIFFICULTY", Activity::DifficultySetting::CakeDifficulty),
					value("EASYDIFFICULTY", Activity::DifficultySetting::EasyDifficulty),
					value("MEDIUMDIFFICULTY", Activity::DifficultySetting::MediumDifficulty),
					value("HARDDIFFICULTY", Activity::DifficultySetting::HardDifficulty),
					value("NUTSDIFFICULTY", Activity::DifficultySetting::NutsDifficulty),
					value("MAXDIFFICULTY", Activity::DifficultySetting::MaxDifficulty)
				]
				.enum_("AISkillSetting")[
					value("MINSKILL", Activity::AISkillSetting::MinSkill),
					value("INFERIORSKILL", Activity::AISkillSetting::InferiorSkill),
					value("DEFAULTSKILL", Activity::AISkillSetting::DefaultSkill),
					value("AVERAGESKILL", Activity::AISkillSetting::AverageSkill),
					value("GOODSKILL", Activity::AISkillSetting::GoodSkill),
					value("UNFAIRSKILL", Activity::AISkillSetting::UnfairSkill)
				]
				.def(luabind::constructor<>())
				.property("ClassName", &Activity::GetClassName)
				.property("Description", &Activity::GetDescription)
				.property("InCampaignStage", &Activity::GetInCampaignStage, &Activity::SetInCampaignStage)
				.property("ActivityState", &Activity::GetActivityState, &Activity::SetActivityState)
				.property("SceneName", &Activity::GetSceneName, &Activity::SetSceneName)
				.property("PlayerCount", &Activity::GetPlayerCount)
				.def("DeactivatePlayer", &Activity::DeactivatePlayer)
				.def("PlayerActive", &Activity::PlayerActive)
				.def("PlayerHuman", &Activity::PlayerHuman)
				.property("HumanCount", &Activity::GetHumanCount)
				.property("TeamCount", &Activity::GetTeamCount)
				.def("TeamActive", &Activity::TeamActive)
				.def("GetTeamOfPlayer", &Activity::GetTeamOfPlayer)
				.def("SetTeamOfPlayer", &Activity::SetTeamOfPlayer)
				.def("PlayersInTeamCount", &Activity::PlayersInTeamCount)
				.def("ScreenOfPlayer", &Activity::ScreenOfPlayer)
				.def("GetViewState", &Activity::GetViewState)
				.def("SetViewState", &Activity::SetViewState)
				.def("GetPlayerBrain", &Activity::GetPlayerBrain)
				.def("SetPlayerBrain", &Activity::SetPlayerBrain)
				.def("PlayerHadBrain", &Activity::PlayerHadBrain)
				.def("SetBrainEvacuated", &Activity::SetBrainEvacuated)
				.def("BrainWasEvacuated", &Activity::BrainWasEvacuated)
				.def("IsAssignedBrain", &Activity::IsAssignedBrain)
				.def("IsBrainOfWhichPlayer", &Activity::IsBrainOfWhichPlayer)
				.def("IsOtherPlayerBrain", &Activity::IsOtherPlayerBrain)
				.def("HumanBrainCount", &Activity::HumanBrainCount)
				.def("AIBrainCount", &Activity::AIBrainCount)
				.def("GetControlledActor", &Activity::GetControlledActor)
				.def("SetTeamFunds", &Activity::SetTeamFunds)
				.def("GetTeamFunds", &Activity::GetTeamFunds)
				.def("SetTeamAISkill", &Activity::SetTeamAISkill)
				.def("GetTeamAISkill", &Activity::GetTeamAISkill)
				.def("ChangeTeamFunds", &Activity::ChangeTeamFunds)
				.def("TeamFundsChanged", &Activity::TeamFundsChanged)
				.def("ReportDeath", &Activity::ReportDeath)
				.def("GetTeamDeathCount", &Activity::GetTeamDeathCount)
				.def("Running", &Activity::IsRunning)
				.def("Paused", &Activity::IsPaused)
				.def("ActivityOver", &Activity::IsOver)
				.def("EnteredOrbit", &Activity::EnteredOrbit)
				.def("SwitchToActor", &Activity::SwitchToActor)
				.def("SwitchToNextActor", &Activity::SwitchToNextActor)
				.def("SwitchToPrevActor", &Activity::SwitchToPrevActor)
				.property("Difficulty", &Activity::GetDifficulty, &Activity::SetDifficulty)
				.def("IsHumanTeam", &Activity::IsHumanTeam)
				.def("ResetMessageTimer", &Activity::ResetMessageTimer);
		}

		LuaBindingRegisterFunctionForType(GameActivity) {
			return luabind::class_<GameActivity, Activity>("GameActivity")
				.enum_("ObjectiveArrowDir")[
					value("ARROWDOWN", GameActivity::ObjectiveArrowDir::ARROWDOWN),
					value("ARROWLEFT", GameActivity::ObjectiveArrowDir::ARROWLEFT),
					value("ARROWRIGHT", GameActivity::ObjectiveArrowDir::ARROWRIGHT),
					value("ARROWUP", GameActivity::ObjectiveArrowDir::ARROWUP)
				]
				.def(luabind::constructor<>())
				.def("SetObservationTarget", &GameActivity::SetObservationTarget)
				.def("SetDeathViewTarget", &GameActivity::SetDeathViewTarget)
				.def("SetLandingZone", &GameActivity::SetLandingZone)
				.def("GetLandingZone", &GameActivity::GetLandingZone)
				.def("SetActorSelectCursor", &GameActivity::SetActorSelectCursor)
				.def("GetBuyGUI", &GameActivity::GetBuyGUI)
				.def("GetEditorGUI", &GameActivity::GetEditorGUI)
				.property("WinnerTeam", &GameActivity::GetWinnerTeam, &GameActivity::SetWinnerTeam)
				.property("CPUTeam", &GameActivity::GetCPUTeam, &GameActivity::SetCPUTeam)
				//.def_readwrite("ActorCursor", &GameActivity::m_ActorCursor)
				.def_readwrite("CursorTimer", &GameActivity::m_CursorTimer)
				.def_readwrite("GameTimer", &GameActivity::m_GameTimer)
				.def_readwrite("GameOverTimer", &GameActivity::m_GameOverTimer)
				.def_readwrite("GameOverPeriod", &GameActivity::m_GameOverPeriod)
				.def("OtherTeam", &GameActivity::OtherTeam)
				.def("OneOrNoneTeamsLeft", &GameActivity::OneOrNoneTeamsLeft)
				.def("WhichTeamLeft", &GameActivity::WhichTeamLeft)
				.def("NoTeamLeft", &GameActivity::NoTeamLeft)
				// Backwards compat
				.def("OnlyOneTeamLeft", &GameActivity::OneOrNoneTeamsLeft)
				.def("GetBanner", &GameActivity::GetBanner)
				.def("SetLZArea", &GameActivity::SetLZArea)
				.def("GetLZArea", &GameActivity::GetLZArea)
				.def("SetBrainLZWidth", &GameActivity::SetBrainLZWidth)
				.def("GetBrainLZWidth", &GameActivity::GetBrainLZWidth)
				.def("GetActiveCPUTeamCount", &GameActivity::GetActiveCPUTeamCount)
				.def("GetActiveHumanTeamCount", &GameActivity::GetActiveHumanTeamCount)
				.def("AddObjectivePoint", &GameActivity::AddObjectivePoint)
				.def("YSortObjectivePoints", &GameActivity::YSortObjectivePoints)
				.def("ClearObjectivePoints", &GameActivity::ClearObjectivePoints)
				.def("AddOverridePurchase", &GameActivity::AddOverridePurchase)
				.def("SetOverridePurchaseList", (int (GameActivity::*)(const Loadout *, int))&GameActivity::SetOverridePurchaseList)
				.def("SetOverridePurchaseList", (int (GameActivity::*)(string, int))&GameActivity::SetOverridePurchaseList)
				.def("ClearOverridePurchase", &GameActivity::ClearOverridePurchase)
				.def("CreateDelivery", (bool (GameActivity::*)(int))&GameActivity::CreateDelivery)
				.def("CreateDelivery", (bool (GameActivity::*)(int, int))&GameActivity::CreateDelivery)
				.def("CreateDelivery", (bool (GameActivity::*)(int, int, Vector&))&GameActivity::CreateDelivery)
				.def("CreateDelivery", (bool (GameActivity::*)(int, int, Actor*))&GameActivity::CreateDelivery)
				.def("GetDeliveryCount", &GameActivity::GetDeliveryCount)
				.property("DeliveryDelay", &GameActivity::GetDeliveryDelay, &GameActivity::SetDeliveryDelay)
				.def("GetTeamTech", &GameActivity::GetTeamTech)
				.def("SetTeamTech", &GameActivity::SetTeamTech)
				.def("GetCrabToHumanSpawnRatio", &GameActivity::GetCrabToHumanSpawnRatio)
				.property("BuyMenuEnabled", &GameActivity::GetBuyMenuEnabled, &GameActivity::SetBuyMenuEnabled)
				.property("CraftsOrbitAtTheEdge", &GameActivity::GetCraftOrbitAtTheEdge, &GameActivity::SetCraftOrbitAtTheEdge)
				.def("TeamIsCPU", &GameActivity::TeamIsCPU)
				.def("GetStartingGold", &GameActivity::GetStartingGold)
				.def("GetFogOfWarEnabled", &GameActivity::GetFogOfWarEnabled)
				.def("UpdateEditing", &GameActivity::UpdateEditing)
				.def("DisableAIs", &GameActivity::DisableAIs)
				.def("InitAIs", &GameActivity::InitAIs)
				.def("AddPieMenuSlice", &GameActivity::AddPieMenuSlice)
				.def("AlterPieMenuSlice", &GameActivity::AlterPieMenuSlice)
				.def("RemovePieMenuSlice", &GameActivity::RemovePieMenuSlice)
				.def_readwrite("PieMenuSlices", &GameActivity::m_CurrentPieMenuSlices, return_stl_iterator);
		}
	};
}
#endif