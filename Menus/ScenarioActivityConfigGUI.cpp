#include "ScenarioActivityConfigGUI.h"

#include "PresetMan.h"
#include "UInputMan.h"

#include "DataModule.h"
#include "GameActivity.h"
#include "Scene.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIComboBox.h"
#include "GUICheckbox.h"
#include "GUIButton.h"
#include "GUILabel.h"
#include "GUISlider.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioActivityConfigGUI::ScenarioActivityConfigGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_ActivityConfigBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxActivityConfig"));
		m_ActivityConfigBox->CenterInParent(true, true);
		m_ActivityConfigBox->SetVisible(false);

		m_CancelConfigButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelConfig"));

		m_ActivityDifficultyLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelActivityDifficultyValue"));
		m_ActivityDifficultySlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderActivityDifficulty"));
		m_StartingGoldLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelStartingGoldValue"));
		m_StartingGoldSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderStartingGold"));

		m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxRequireClearPathToOrbit"));
		m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxFogOfWar"));
		m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxDeployUnits"));

		m_PlayersAndTeamsConfigBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPlayersAndTeamsConfig"));
		m_TeamIconBoxes.at(TeamRows::DisabledTeam) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxDisabledTeamIcon"));
		m_TeamNameLabels.at(TeamRows::DisabledTeam) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelDisabledTeam"));

		GUILabel *teamTechLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeamTech"));
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			std::string teamNumber = std::to_string(team + 1);
			m_TeamIconBoxes.at(team) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxTeam" + teamNumber + "Icon"));
			m_TeamNameLabels.at(team) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeam" + teamNumber));

			m_TeamTechComboBoxes.at(team) = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboBoxTeam" + teamNumber + "Tech"));
			m_TeamTechComboBoxes.at(team)->Move(teamTechLabel->GetXPos(), teamTechLabel->GetYPos() + teamTechLabel->GetHeight() + 5 + (25 * (team + 1)));
			m_TeamTechComboBoxes.at(team)->SetVisible(false);

			m_TeamAISkillSliders.at(team) = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderTeam" + teamNumber + "AISkill"));
			m_TeamAISkillSliders.at(team)->SetValue(Activity::AISkillSetting::DefaultSkill);
			m_TeamAISkillLabels.at(team) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeam" + teamNumber + "AISkill"));
			m_TeamAISkillLabels.at(team)->SetText(Activity::GetAISkillString(m_TeamAISkillSliders.at(team)->GetValue()));
		}
		for (int player = Players::PlayerOne; player < PlayerColumns::PlayerColumnCount; ++player) {
			for (int team = Activity::Teams::TeamOne; team < TeamRows::TeamRowCount; ++team) {
				m_PlayerBoxes.at(player).at(team) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("P" + std::to_string(player + 1) + "T" + std::to_string(team + 1) + "Box"));
			}
		}
		m_CPULockLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelCPUTeamLock"));
		m_StartErrorLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelStartError"));
		m_StartGameButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonStartGame"));

		m_TechListFetched = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::PopulateTechComboBoxes() {
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			m_TeamTechComboBoxes.at(team)->GetListPanel()->AddItem("-All-", "", nullptr, nullptr, -2);
			m_TeamTechComboBoxes.at(team)->GetListPanel()->AddItem("-Random-", "", nullptr, nullptr, -1);
		}
		for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
			if (const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID)) {
				if (dataModule->IsFaction()) {
					for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
						m_TeamTechComboBoxes.at(team)->GetListPanel()->AddItem(dataModule->GetFriendlyName(), "", nullptr, nullptr, moduleID);
						m_TeamTechComboBoxes.at(team)->GetListPanel()->ScrollToTop();
						m_TeamTechComboBoxes.at(team)->SetSelectedIndex(0);
					}
				}
			}
		}
		m_TechListFetched = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioActivityConfigGUI::IsEnabled() const {
		return m_ActivityConfigBox->GetEnabled() && m_ActivityConfigBox->GetVisible();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::SetEnabled(bool enable, const Activity *selectedActivity, Scene *selectedScene) {
		m_ActivityConfigBox->SetEnabled(enable);
		m_ActivityConfigBox->SetVisible(enable);

		if (enable) {
			m_SelectedActivity = dynamic_cast<const GameActivity *>(selectedActivity);
			m_SelectedScene = selectedScene;
			RTEAssert(m_SelectedActivity && m_SelectedScene, "Trying to start a scenario game without an Activity or a Scene!");
		} else {
			m_SelectedActivity = nullptr;
			m_SelectedScene = nullptr;
		}
		// The tech select ComboBoxes aren't children of the config box (dirty hack to allow the drop-down list to extend beyond the parent box bounds without clipping) so we need to set their visibility separately.
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			m_TeamTechComboBoxes.at(team)->SetVisible(enable);
		}
		if (enable && m_SelectedActivity && m_SelectedScene) {
			if (!m_TechListFetched) { PopulateTechComboBoxes(); }
			ResetActivityConfigBox();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::ResetActivityConfigBox() {
		m_ActivityDifficultyLabel->SetText(" " + Activity::GetDifficultyString(m_ActivityDifficultySlider->GetValue()));

		m_StartingGoldAdjustedManually = false;
		UpdateStartingGoldSliderAndLabel();
		m_StartingGoldSlider->SetEnabled(m_SelectedActivity->GetGoldSwitchEnabled());

		m_FogOfWarCheckbox->SetCheck(m_SelectedActivity->GetDefaultFogOfWar() > 0);
		m_FogOfWarCheckbox->SetEnabled(m_SelectedActivity->GetFogOfWarSwitchEnabled());

		m_RequireClearPathToOrbitCheckbox->SetCheck(m_SelectedActivity->GetDefaultRequireClearPathToOrbit() > 0);
		m_RequireClearPathToOrbitCheckbox->SetEnabled(m_SelectedActivity->GetRequireClearPathToOrbitSwitchEnabled());

		m_DeployUnitsCheckbox->SetCheck(m_SelectedActivity->GetDefaultDeployUnits() > 0);
		m_DeployUnitsCheckbox->SetEnabled(m_SelectedActivity->GetDeployUnitsSwitchEnabled());

		m_StartGameButton->SetVisible(false);
		m_StartErrorLabel->SetVisible(true);

		for (int player = Players::PlayerOne; player < PlayerColumns::PlayerColumnCount; ++player) {
			for (int team = Activity::Teams::TeamOne; team < TeamRows::TeamRowCount; ++team) {
				m_PlayerBoxes.at(player).at(team)->SetDrawType(GUICollectionBox::Color);
				m_PlayerBoxes.at(player).at(team)->SetDrawColor(c_GUIColorBlue);
			}
			if (player < Players::MaxPlayerCount) {
				if (const Icon *playerDeviceIcon = g_UInputMan.GetSchemeIcon(player)) { m_PlayerBoxes.at(player).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerDeviceIcon->GetBitmaps32()[0])); }
				m_PlayerBoxes.at(player).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
			} else {
				int cpuInitialTeam = TeamRows::DisabledTeam;
				m_LockedCPUTeam = m_SelectedActivity->GetCPUTeam();
				if (m_LockedCPUTeam != Activity::Teams::NoTeam) {
					cpuInitialTeam = m_LockedCPUTeam;
					m_CPULockLabel->SetPositionAbs(m_CPULockLabel->GetXPos(), m_TeamNameLabels.at(m_LockedCPUTeam)->GetYPos());
					m_CPULockLabel->SetVisible(true);
				} else {
					m_CPULockLabel->SetVisible(false);
				}
				m_PlayerBoxes.at(player).at(cpuInitialTeam)->SetDrawType(GUICollectionBox::Image);
				if (const Icon *cpuIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"))) { m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(cpuInitialTeam)->SetDrawImage(new AllegroBitmap(cpuIcon->GetBitmaps32()[0])); }
			}
		}

		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			const Icon *teamIcon = nullptr;
			if (m_SelectedActivity->TeamActive(team)) {
				teamIcon = m_SelectedActivity->GetTeamIcon(team);
				if (!teamIcon) { teamIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Team " + std::to_string(team + 1) + " Default")); }
				m_TeamNameLabels.at(team)->SetText(m_SelectedActivity->GetTeamName(team));
			} else {
				teamIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
				m_TeamNameLabels.at(team)->SetText("Unavailable");
			}
			if (teamIcon) { m_TeamIconBoxes.at(team)->SetDrawImage(new AllegroBitmap(teamIcon->GetBitmaps32()[0])); }

			m_TeamTechComboBoxes.at(team)->SetVisible(m_SelectedActivity->TeamActive(team));
			m_TeamAISkillSliders.at(team)->SetVisible(m_SelectedActivity->TeamActive(team));
			m_TeamAISkillLabels.at(team)->SetVisible(m_SelectedActivity->TeamActive(team));
		}
		if (const Icon *disabledTeamIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"))) { m_TeamIconBoxes.at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(disabledTeamIcon->GetBitmaps32()[0])); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::StartGame() {
		GameActivity *gameActivity = dynamic_cast<GameActivity *>(m_SelectedActivity->Clone());

		gameActivity->SetDifficulty(m_ActivityDifficultySlider->GetValue());
		gameActivity->SetStartingGold((m_StartingGoldSlider->GetValue() == m_StartingGoldSlider->GetMaximum()) ? 1000000000 : m_StartingGoldSlider->GetValue() - (m_StartingGoldSlider->GetValue() % 500));

		gameActivity->SetRequireClearPathToOrbit(m_RequireClearPathToOrbitCheckbox->GetCheck());
		gameActivity->SetFogOfWarEnabled(m_FogOfWarCheckbox->GetCheck());
		g_SceneMan.SetSceneToLoad(m_SelectedScene, true, m_DeployUnitsCheckbox->GetCheck());

		gameActivity->ClearPlayers(false);
		for (int player = Players::PlayerOne; player < PlayerColumns::PlayerColumnCount; ++player) {
			for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
				if (m_PlayerBoxes.at(player).at(team)->GetDrawType() == GUICollectionBox::Image) {
					if (player == PlayerColumns::PlayerCPU) {
						gameActivity->SetCPUTeam(team);
					} else {
						gameActivity->AddPlayer(player, true, team, 0);
						break;
					}
				}
			}
		}

		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			if (const GUIListPanel::Item *techItem = m_TeamTechComboBoxes.at(team)->GetSelectedItem()) {
				if (techItem->m_ExtraIndex == -2) {
					gameActivity->SetTeamTech(team, "-All-");
				} else if (techItem->m_ExtraIndex == -1) {
					gameActivity->SetTeamTech(team, g_PresetMan.GetDataModuleName(m_TeamTechComboBoxes.at(team)->GetItem(RandomNum<int>(2, m_TeamTechComboBoxes.at(team)->GetListPanel()->GetItemList()->size() - 1))->m_ExtraIndex));
				} else {
					gameActivity->SetTeamTech(team, g_PresetMan.GetDataModuleName(techItem->m_ExtraIndex));
				}
			}
			gameActivity->SetTeamAISkill(team, (m_TeamAISkillSliders.at(team)->IsEnabled()) ? m_TeamAISkillSliders.at(team)->GetValue() : Activity::AISkillSetting::DefaultSkill);
		}
		g_LuaMan.FileCloseAll();
		g_ActivityMan.SetStartActivity(gameActivity);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioActivityConfigGUI::Update(int mouseX, int mouseY) {
		UpdatePlayerTeamSetupCell(mouseX, mouseY);

		int teamsWithPlayers = 0;
		bool teamWithHumans = false;
		int humansInTeams = 0;
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			bool foundPlayer = false;
			if (m_SelectedActivity->TeamActive(team)) {
				for (int player = Players::PlayerOne; player < PlayerColumns::PlayerColumnCount; ++player) {
					if (m_PlayerBoxes.at(player).at(team)->GetDrawType() == GUICollectionBox::Image) {
						foundPlayer = true;
						if (player != PlayerColumns::PlayerCPU) {
							humansInTeams++;
							teamWithHumans = true;
						}
					}
				}
				if (foundPlayer) { teamsWithPlayers++; }
			}
		}

		int maxHumanPlayers = m_SelectedActivity->GetMaxPlayerSupport();
		int minTeamsRequired = m_SelectedActivity->GetMinTeamsRequired();

		std::string errorMessage = "";
		if (humansInTeams > maxHumanPlayers) {
			errorMessage = "Too many players assigned! Max for this activity is " + std::to_string(maxHumanPlayers);
		} else if (minTeamsRequired > teamsWithPlayers) {
			errorMessage = "Assign players to at least " + std::to_string(minTeamsRequired) + " of the teams!";
		} else if (teamWithHumans == 0) {
			errorMessage = "Assign human players to at least one team!";
		}
		m_StartErrorLabel->SetText(errorMessage);
		m_StartErrorLabel->SetVisible(!errorMessage.empty());
		m_StartGameButton->SetVisible(errorMessage.empty());

		if (m_StartGameButton->GetVisible()) { m_GUIControlManager->GetManager()->SetFocus(m_StartGameButtonBlinkTimer.AlternateReal(500) ? m_StartGameButton : nullptr); }

		return HandleInputEvents();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::UpdateStartingGoldSliderAndLabel() {
		if (!m_StartingGoldAdjustedManually) {
			if (m_ActivityDifficultySlider->GetValue() <= Activity::DifficultySetting::CakeDifficulty && m_SelectedActivity->GetDefaultGoldCake() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldCake());
			} else if (m_ActivityDifficultySlider->GetValue() <= Activity::DifficultySetting::EasyDifficulty && m_SelectedActivity->GetDefaultGoldEasy() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldEasy());
			} else if (m_ActivityDifficultySlider->GetValue() <= Activity::DifficultySetting::MediumDifficulty && m_SelectedActivity->GetDefaultGoldMedium() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldMedium());
			} else if (m_ActivityDifficultySlider->GetValue() <= Activity::DifficultySetting::HardDifficulty && m_SelectedActivity->GetDefaultGoldHard() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldHard());
			} else if (m_ActivityDifficultySlider->GetValue() <= Activity::DifficultySetting::NutsDifficulty && m_SelectedActivity->GetDefaultGoldNuts() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldNuts());
			} else if (m_SelectedActivity->GetDefaultGoldNuts() > -1) {
				m_StartingGoldSlider->SetValue(m_SelectedActivity->GetDefaultGoldNuts());
			}
		}
		std::string goldString(16, '\0');
		if (m_StartingGoldSlider->GetValue() == m_StartingGoldSlider->GetMaximum()) {
			std::snprintf(goldString.data(), goldString.size(), " %c Infinite", -58);
		} else {
			int startGold = m_StartingGoldSlider->GetValue();
			startGold = startGold - (startGold % 500);
			std::snprintf(goldString.data(), goldString.size(), " %c %d oz", -58, startGold);
		}
		m_StartingGoldLabel->SetText(goldString);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::UpdatePlayerTeamSetupCell(int mouseX, int mouseY) {
		if (const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControlUnderPoint(mouseX, mouseY, m_PlayersAndTeamsConfigBox, 1))) {
			int hoveredPlayer = PlayerColumns::PlayerColumnCount;
			int hoveredTeam = TeamRows::TeamRowCount;
			for (int player = Players::PlayerOne; player < PlayerColumns::PlayerColumnCount; ++player) {
				for (int team = Activity::Teams::TeamOne; team < TeamRows::TeamRowCount; ++team) {
					if (m_PlayerBoxes.at(player).at(team) == hoveredCell) {
						hoveredPlayer = player;
						hoveredTeam = team;
					} else if (m_PlayerBoxes.at(player).at(team)->GetDrawType() == GUICollectionBox::Color) {
						m_PlayerBoxes.at(player).at(team)->SetDrawColor(c_GUIColorBlue);
					}
				}
			}
			if ((m_SelectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TeamRows::DisabledTeam) && hoveredTeam != m_LockedCPUTeam && (m_LockedCPUTeam == Activity::Teams::NoTeam || hoveredPlayer != PlayerColumns::PlayerCPU) && m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() != GUICollectionBox::Image) {
				if (g_UInputMan.MenuButtonReleased(UInputMan::MENU_PRIMARY)) {
					HandleClickOnPlayerTeamSetupCell(hoveredPlayer, hoveredTeam);
				} else if (m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() == GUICollectionBox::Color && m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawColor() != c_GUIColorLightBlue) {
					m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->SetDrawColor(c_GUIColorLightBlue);
					g_GUISound.SelectionChangeSound()->Play();
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::HandleClickOnPlayerTeamSetupCell(int clickedPlayer, int clickedTeam) {
		m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Image);
		const Icon *playerIcon = (clickedPlayer != PlayerColumns::PlayerCPU) ? g_UInputMan.GetSchemeIcon(clickedPlayer) : dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (playerIcon) { m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }

		if (clickedPlayer == PlayerColumns::PlayerCPU) {
			m_PlayerBoxes.at(clickedPlayer).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Color);
			m_PlayerBoxes.at(clickedPlayer).at(TeamRows::DisabledTeam)->SetDrawColor(c_GUIColorBlue);
		} else {
			for (int nonHoveredTeam = Activity::Teams::TeamOne; nonHoveredTeam < TeamRows::TeamRowCount; ++nonHoveredTeam) {
				if (nonHoveredTeam != clickedTeam) {
					m_PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawType(GUICollectionBox::Color);
					m_PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawColor(c_GUIColorBlue);
				}
			}
		}

		// Remove human players from their team row if a CPU player is added to it, and vice - versa for CPU players.
		if (clickedPlayer == PlayerColumns::PlayerCPU && clickedTeam != TeamRows::DisabledTeam) {
			for (int humanPlayer = Players::PlayerOne; humanPlayer < Players::MaxPlayerCount; ++humanPlayer) {
				if (m_PlayerBoxes.at(humanPlayer).at(clickedTeam)->GetDrawType() == GUICollectionBox::Image) {
					m_PlayerBoxes.at(humanPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Color);
					m_PlayerBoxes.at(humanPlayer).at(clickedTeam)->SetDrawColor(c_GUIColorBlue);
					m_PlayerBoxes.at(humanPlayer).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
					playerIcon = g_UInputMan.GetSchemeIcon(humanPlayer);
					if (playerIcon) { m_PlayerBoxes.at(humanPlayer).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }
				}
			}
		} else if (clickedPlayer != PlayerColumns::PlayerCPU && clickedTeam != TeamRows::DisabledTeam && m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->GetDrawType() == GUICollectionBox::Image) {
			m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->SetDrawType(GUICollectionBox::Color);
			m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->SetDrawColor(c_GUIColorBlue);

			int cpuTeamCount = 0;
			for (int teamToCount = Activity::Teams::TeamOne; teamToCount < TeamRows::DisabledTeam; ++teamToCount) {
				if (m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(teamToCount)->GetDrawType() == GUICollectionBox::Image) { cpuTeamCount++; }
			}
			if (cpuTeamCount == 0) {
				m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
				playerIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
				if (playerIcon) { m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }
			}
		}
		g_GUISound.FocusChangeSound()->Play();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioActivityConfigGUI::HandleInputEvents() {
		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_CancelConfigButton) {
					g_GUISound.BackButtonPressSound()->Play();
					SetEnabled(false);
				} else if (guiEvent.GetControl() == m_StartGameButton) {
					g_GUISound.ButtonPressSound()->Play();
					StartGame();
					SetEnabled(false);
					return true;
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetControl() == m_ActivityDifficultySlider) {
					m_ActivityDifficultyLabel->SetText(" " + Activity::GetDifficultyString(m_ActivityDifficultySlider->GetValue()));
					if (!m_StartingGoldAdjustedManually) { UpdateStartingGoldSliderAndLabel(); }
				} else if (guiEvent.GetControl() == m_StartingGoldSlider) {
					if (guiEvent.GetMsg() == GUISlider::Clicked) { m_StartingGoldAdjustedManually = true; }
					UpdateStartingGoldSliderAndLabel();
				} else if (guiEvent.GetMsg() == GUISlider::Changed) {
					for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; team++) {
						if (guiEvent.GetControl() == m_TeamAISkillSliders.at(team)) { m_TeamAISkillLabels.at(team)->SetText(Activity::GetAISkillString(m_TeamAISkillSliders.at(team)->GetValue())); }
					}
				}
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::Draw() {
		m_GUIControlManager->Draw();

		int rectPosY = 50;
		// Apply a colored overlay on top of team rows that are locked or disabled by the Activity and are not modifiable.
		for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
			if (!m_SelectedActivity->TeamActive(team) || m_LockedCPUTeam == team) {
				drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
				int blendAmount = 230;
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				rectfill(g_FrameMan.GetBackBuffer32(), m_PlayersAndTeamsConfigBox->GetXPos() + 2, m_PlayersAndTeamsConfigBox->GetYPos() + rectPosY, m_PlayersAndTeamsConfigBox->GetXPos() + m_PlayersAndTeamsConfigBox->GetWidth() - 3, m_PlayersAndTeamsConfigBox->GetYPos() + rectPosY + 25, c_GUIColorDarkBlue);
				drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
			}
			rectPosY += 25;
		}

		int linePosY = 25;
		for (int i = 0; i < 6; ++i) {
			hline(g_FrameMan.GetBackBuffer32(), m_PlayersAndTeamsConfigBox->GetXPos() + 2, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY, m_PlayersAndTeamsConfigBox->GetXPos() + m_PlayersAndTeamsConfigBox->GetWidth() - 2, c_GUIColorLightBlue);
			linePosY += 25;
		}

		// The colored overlay is semi-transparent and is drawn on top of GUI elements, so we need to redraw the relevant elements again so they aren't affected.
		for (int team = Activity::Teams::MaxTeamCount - 1; team >= Activity::Teams::TeamOne; --team) {
			if (m_TeamTechComboBoxes.at(team)->GetVisible()) {
				m_TeamTechComboBoxes.at(team)->Draw(m_GUIControlManager->GetScreen());
				if (m_TeamTechComboBoxes.at(team)->IsDropped()) { m_TeamTechComboBoxes.at(team)->GetListPanel()->Draw(m_GUIControlManager->GetScreen()); }
			}
			if (m_TeamAISkillSliders.at(team)->GetVisible()) {
				m_TeamAISkillSliders.at(team)->Draw(m_GUIControlManager->GetScreen());
				m_TeamAISkillLabels.at(team)->Draw(m_GUIControlManager->GetScreen());
			}
		}
	}
}