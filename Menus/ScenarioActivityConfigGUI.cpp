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

		m_PlayersAndTeamsConfigBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPlayersAndTeamsConfig"));

		m_StartErrorLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelStartError"));
		m_StartGameButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonStartGame"));
		m_CancelConfigButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelConfig"));

		m_ActivityDifficultyLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelActivityDifficulty"));
		m_ActivityDifficultySlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderActivityDifficulty"));
		m_StartingGoldLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelStartingGold"));
		m_StartingGoldSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderStartingGold"));

		m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxRequireClearPathToOrbit"));
		m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxFogOfWar"));
		m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxDeployUnits"));

		m_TeamNameLabels.at(TeamRows::DisabledTeam) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelDisabledTeam"));
		m_TeamIconBoxes.at(TeamRows::DisabledTeam) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxDisabledTeamIcon"));

		m_CPULockLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelCPUTeamLock"));

		for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
			for (int teamIndex = Activity::Teams::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
				m_PlayerBoxes.at(playerIndex).at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("P" + std::to_string(playerIndex + 1) + "T" + std::to_string(teamIndex + 1) + "Box"));
			}
		}
		GUILabel *teamTechLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeamTech"));
		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			std::string teamNumber = std::to_string(teamIndex + 1);

			m_TeamIconBoxes.at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxTeam" + teamNumber + "Icon"));
			m_TeamNameLabels.at(teamIndex) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeam" + teamNumber));


			m_TeamTechSelect.at(teamIndex) = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboBoxTeam" + teamNumber + "Tech"));
			m_TeamTechSelect.at(teamIndex)->Move(teamTechLabel->GetXPos(), teamTechLabel->GetYPos() + teamTechLabel->GetHeight() + 5 + (25 * (teamIndex + 1)));
			m_TeamTechSelect.at(teamIndex)->SetVisible(false);

			m_TeamAISkillSlider.at(teamIndex) = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderTeam" + teamNumber + "AISkill"));
			m_TeamAISkillSlider.at(teamIndex)->SetValue(Activity::AISkillSetting::DefaultSkill);

			m_TeamAISkillLabel.at(teamIndex) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelTeam" + teamNumber + "AISkill"));
			m_TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_TeamAISkillSlider.at(teamIndex)->GetValue()));
		}
		PopulateTechComboBoxes();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::PopulateTechComboBoxes() {
		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-All-", "", nullptr, nullptr, -2);
			m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-Random-", "", nullptr, nullptr, -1);
			m_TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);
		}
		std::string techString = " Tech";
		for (int moduleIndex = 0; moduleIndex < g_PresetMan.GetTotalModuleCount(); ++moduleIndex) {
			const DataModule *dataModule = g_PresetMan.GetDataModule(moduleIndex);
			if (dataModule) {
				std::string techName = dataModule->GetFriendlyName();
				size_t techPos = techName.find(techString);
				if (techPos != string::npos) {
					techName.replace(techPos, techString.length(), "");
					for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
						m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem(techName, "", nullptr, nullptr, moduleIndex);
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioActivityConfigGUI::IsEnabled() const {
		return m_ActivityConfigBox->GetEnabled() && m_ActivityConfigBox->GetVisible();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::SetEnabled(bool enable, const Activity *selectedActivity, Scene *selectedScene) {
		// The tech select ComboBoxes aren't children of the config box (dirty hack to allow the drop-down list to extend beyond the parent box bounds without clipping) so we need to set their visibility separately.
		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			m_TeamTechSelect.at(teamIndex)->SetEnabled(enable);
			m_TeamTechSelect.at(teamIndex)->SetVisible(enable);
		}
		if (enable) {
			m_ActivityConfigBox->SetEnabled(true);
			m_ActivityConfigBox->SetVisible(true);
			m_SelectedActivity = selectedActivity;
			m_SelectedScene = selectedScene;

			ShowPlayersBox();
		} else {
			m_ActivityConfigBox->SetEnabled(false);
			m_ActivityConfigBox->SetVisible(false);
			m_SelectedActivity = nullptr;
			m_SelectedScene = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::ShowPlayersBox() {
		if (m_SelectedActivity && m_SelectedScene) {
			if (const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity)) {
				m_LockedCPUTeam = gameActivity->GetCPUTeam();
				if (m_LockedCPUTeam != Activity::Teams::NoTeam) { m_CPULockLabel->SetPositionAbs(m_CPULockLabel->GetXPos(), m_TeamNameLabels.at(m_LockedCPUTeam)->GetYPos()); }
			}
			m_CPULockLabel->SetVisible(m_LockedCPUTeam != Activity::Teams::NoTeam);

			m_StartGameButton->SetVisible(false);
			m_StartErrorLabel->SetVisible(true);

			// Set up initial color for all cells.
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::Teams::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
					m_PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawType(GUICollectionBox::Color);
					m_PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawColor(c_GUIColorBlue);
				}
			}

			const Icon *iconPointer = nullptr;

			// Human players start on the disabled team row.
			for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
				m_PlayerBoxes.at(playerIndex).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
				iconPointer = g_UInputMan.GetSchemeIcon(playerIndex);
				if (iconPointer) { m_PlayerBoxes.at(playerIndex).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }
			}

			// CPU player either has a locked team or starts on the disabled team row.
			int InitialCPUTeam = (m_LockedCPUTeam != Activity::Teams::NoTeam) ? m_LockedCPUTeam : TeamRows::DisabledTeam;
			m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawType(GUICollectionBox::Image);
			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
			if (iconPointer) { m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
			m_TeamNameLabels.at(TeamRows::DisabledTeam)->SetText("Not Playing:");
			if (iconPointer) { m_TeamIconBoxes.at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
				// Reset GUI controls to default values.
				m_TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);
				m_TeamAISkillSlider.at(teamIndex)->SetValue(Activity::AISkillSetting::DefaultSkill);

				if (m_SelectedActivity->TeamActive(teamIndex)) {
					iconPointer = m_SelectedActivity->GetTeamIcon(teamIndex);
					if (!iconPointer) {
						std::string teamString = "Team " + std::to_string(teamIndex + 1) + " Default";
						iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", teamString));
					}
					m_TeamNameLabels.at(teamIndex)->SetText(m_SelectedActivity->GetTeamName(teamIndex) + ":");
					m_TeamTechSelect.at(teamIndex)->SetEnabled(true);
					m_TeamTechSelect.at(teamIndex)->SetVisible(true);
					m_TeamAISkillSlider.at(teamIndex)->SetEnabled(true);
					m_TeamAISkillSlider.at(teamIndex)->SetVisible(true);
					m_TeamAISkillLabel.at(teamIndex)->SetVisible(true);
				} else {
					iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
					m_TeamNameLabels.at(teamIndex)->SetText("Unavailable");
					m_TeamTechSelect.at(teamIndex)->SetEnabled(false);
					m_TeamTechSelect.at(teamIndex)->SetVisible(false);
					m_TeamAISkillSlider.at(teamIndex)->SetEnabled(false);
					m_TeamAISkillSlider.at(teamIndex)->SetVisible(false);
					m_TeamAISkillLabel.at(teamIndex)->SetVisible(false);
				}
				if (iconPointer) { m_TeamIconBoxes.at(teamIndex)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioActivityConfigGUI::StartGame() {
		Activity *activityInstance = dynamic_cast<Activity *>(m_SelectedActivity->Clone());
		GameActivity *gameActivity = dynamic_cast<GameActivity *>(activityInstance);

		if (gameActivity) {
			gameActivity->SetDifficulty(m_ActivityDifficultySlider->GetValue());
			gameActivity->SetFogOfWarEnabled(m_FogOfWarCheckbox->GetCheck());
			gameActivity->SetRequireClearPathToOrbit(m_RequireClearPathToOrbitCheckbox->GetCheck());
			gameActivity->SetStartingGold((m_StartingGoldSlider->GetValue() == m_StartingGoldSlider->GetMaximum()) ? 9999999 : m_StartingGoldSlider->GetValue() - (m_StartingGoldSlider->GetValue() % 500));
		}
		g_SceneMan.SetSceneToLoad(m_SelectedScene, true, m_DeployUnitsCheckbox->GetCheck());

		activityInstance->ClearPlayers(false);
		for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
			for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
				if (m_PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					activityInstance->AddPlayer(playerIndex, true, teamIndex, 0);
					break;
				}
			}
		}

		if (gameActivity) {
			for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
				if (m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					gameActivity->SetCPUTeam(teamIndex);
					break;
				}
			}
		}

		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			if (const GUIListPanel::Item *techItem = m_TeamTechSelect.at(teamIndex)->GetSelectedItem()) {
				// ExtraIndex -2 is "All", -1 "Random"
				if (techItem->m_ExtraIndex == -2) {
					gameActivity->SetTeamTech(teamIndex, "-All-");
				} else if (techItem->m_ExtraIndex == -1) {
					int selection = RandomNum<int>(2, m_TeamTechSelect.at(teamIndex)->GetListPanel()->GetItemList()->size() - 1);
					m_TeamTechSelect.at(teamIndex)->SetSelectedIndex(selection);
					gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(m_TeamTechSelect.at(teamIndex)->GetSelectedItem()->m_ExtraIndex));
				} else {
					gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(techItem->m_ExtraIndex));
				}
			}
			gameActivity->SetTeamAISkill(teamIndex, (m_TeamAISkillSlider.at(teamIndex)->IsEnabled()) ? m_TeamAISkillSlider.at(teamIndex)->GetValue() : Activity::AISkillSetting::DefaultSkill);
		}

		g_LuaMan.FileCloseAll();
		g_ActivityMan.SetStartActivity(activityInstance);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::ClickInPlayerSetup(int clickedPlayer, int clickedTeam) {
		// Move the player's icon to the correct row.
		m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Image);
		const Icon *playerIcon = (clickedPlayer != PlayerColumns::PlayerCPU) ? g_UInputMan.GetSchemeIcon(clickedPlayer) : dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (playerIcon) { m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }

		for (int nonHoveredTeam = Activity::Teams::TeamOne; nonHoveredTeam < TeamRows::TeamRowCount; ++nonHoveredTeam) {
			if (nonHoveredTeam != clickedTeam) {
				m_PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawType(GUICollectionBox::Color);
				m_PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawColor(c_GUIColorBlue);
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
			m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
			playerIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
			if (playerIcon) { m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }
		}
		g_GUISound.FocusChangeSound()->Play();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	m_ActivityDifficultyLabel->SetVisible(true);
	m_ActivityDifficultySlider->SetVisible(true);
	if (m_ActivityDifficultySlider->GetValue() < Activity::DifficultySetting::CakeDifficulty) {
		m_ActivityDifficultyLabel->SetText("Difficulty: Cake");
	} else if (m_ActivityDifficultySlider->GetValue() < Activity::DifficultySetting::EasyDifficulty) {
		m_ActivityDifficultyLabel->SetText("Difficulty: Easy");
	} else if (m_ActivityDifficultySlider->GetValue() < Activity::DifficultySetting::MediumDifficulty) {
		m_ActivityDifficultyLabel->SetText("Difficulty: Medium");
	} else if (m_ActivityDifficultySlider->GetValue() < Activity::DifficultySetting::HardDifficulty) {
		m_ActivityDifficultyLabel->SetText("Difficulty: Hard");
	} else if (m_ActivityDifficultySlider->GetValue() < Activity::DifficultySetting::NutsDifficulty) {
		m_ActivityDifficultyLabel->SetText("Difficulty: Nuts");
	} else {
		m_ActivityDifficultyLabel->SetText("Difficulty: Nuts!");
	}
	m_ActivityInfoBox->Resize(m_ActivityInfoBox->GetWidth(), m_ActivityDescriptionLabel->ResizeHeightToFit() + 110);

	const GameActivity *selectedGA = dynamic_cast<const GameActivity *>(m_SelectedActivity);
	if (selectedGA) {
		if (m_DifficultySlider->GetValue() < Activity::DifficultySetting::CakeDifficulty && selectedGA->GetDefaultGoldCake() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldCake());
		} else if (m_DifficultySlider->GetValue() < Activity::DifficultySetting::EasyDifficulty && selectedGA->GetDefaultGoldEasy() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldEasy());
		} else if (m_DifficultySlider->GetValue() < Activity::DifficultySetting::MediumDifficulty && selectedGA->GetDefaultGoldMedium() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldMedium());
		} else if (m_DifficultySlider->GetValue() < Activity::DifficultySetting::HardDifficulty && selectedGA->GetDefaultGoldHard() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldHard());
		} else if (m_DifficultySlider->GetValue() < Activity::DifficultySetting::NutsDifficulty && selectedGA->GetDefaultGoldNuts() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldNuts());
		} else if (selectedGA->GetDefaultGoldNuts() > -1) {
			m_GoldSlider->SetValue(selectedGA->GetDefaultGoldNuts());
		}
		m_GoldSlider->SetEnabled(selectedGA->GetGoldSwitchEnabled());

		int defaultFogOfWar = selectedGA->GetDefaultFogOfWar();
		if (defaultFogOfWar > -1) { m_FogOfWarCheckbox->SetCheck(defaultFogOfWar != 0); }
		m_FogOfWarCheckbox->SetEnabled(selectedGA->GetFogOfWarSwitchEnabled());

		int defaultReqClearPath = selectedGA->GetDefaultRequireClearPathToOrbit();
		if (defaultReqClearPath > -1) { m_RequireClearPathToOrbitCheckbox->SetCheck(defaultReqClearPath != 0); }
		m_RequireClearPathToOrbitCheckbox->SetEnabled(selectedGA->GetRequireClearPathToOrbitSwitchEnabled());

		int defaultDeployUnits = selectedGA->GetDefaultDeployUnits();
		if (defaultDeployUnits > -1) { m_DeployUnitsCheckbox->SetCheck(defaultDeployUnits != 0); }
		m_DeployUnitsCheckbox->SetEnabled(selectedGA->GetDeployUnitsSwitchEnabled());
	}
	*/

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::Update(int mouseX, int mouseY) {
		//RTEAssert(m_SelectedActivity && m_SelectedScene, "Trying to start a scenario game without an activity or a scene.");

		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity);
		if (const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControlUnderPoint(mouseX, mouseY, m_PlayersAndTeamsConfigBox, 1))) {
			// Find which cell is being hovered over.
			int hoveredPlayer = PlayerColumns::PlayerColumnCount;
			int hoveredTeam = TeamRows::TeamRowCount;
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::Teams::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
					if (m_PlayerBoxes.at(playerIndex).at(teamIndex) == hoveredCell) {
						hoveredPlayer = playerIndex;
						hoveredTeam = teamIndex;
					} else if (m_PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Color) {
						// Un-highlight all other cells.
						m_PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawColor(c_GUIColorBlue);
					}
				}
			}

			// Make the hovered cell light up and able to be selected if:
			// It's under an active team row or the disabled team row.
			// It's not a team row locked to the CPU.
			// It's not the CPU player if he is locked to a CPU team.
			// It doesn't already contain an image.
			if ((m_SelectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TeamRows::DisabledTeam) && m_LockedCPUTeam != hoveredTeam && (m_LockedCPUTeam == Activity::Teams::NoTeam || hoveredPlayer != PlayerColumns::PlayerCPU) && m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() != GUICollectionBox::Image) {
				if (g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER)) {
					ClickInPlayerSetup(hoveredPlayer, hoveredTeam);
				} else if (m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() == GUICollectionBox::Color && m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawColor() != c_GUIColorLightBlue) {
					// Just highlight the cell.
					m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->SetDrawColor(c_GUIColorLightBlue);
					g_GUISound.SelectionChangeSound()->Play();
				}
			}
		}

		int teamsWithPlayers = 0;
		bool teamWithHumans = false;
		int humansInTeams = 0;
		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			bool foundPlayer = false;
			if (m_SelectedActivity->TeamActive(teamIndex)) {
				for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
					if (m_PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
						foundPlayer = true;
						if (playerIndex != PlayerColumns::PlayerCPU) {
							++humansInTeams;
							teamWithHumans = true;
						}
					}
				}
				if (foundPlayer) { teamsWithPlayers++; }
			}
		}

		if (gameActivity) {
			int maxHumanPlayers = gameActivity->GetMaxPlayerSupport();
			int minTeamsRequired = gameActivity->GetMinTeamsRequired();
			if (humansInTeams > maxHumanPlayers) {
				m_StartGameButton->SetVisible(false);
				m_StartErrorLabel->SetText("Too many players assigned! Max for this activity is " + std::to_string(maxHumanPlayers));
				m_StartErrorLabel->SetVisible(true);
			} else if (minTeamsRequired > teamsWithPlayers) {
				m_StartGameButton->SetVisible(false);
				m_StartErrorLabel->SetText("Assign players to at least " + std::to_string(minTeamsRequired) + " of the teams!");
				m_StartErrorLabel->SetVisible(true);
			} else if (teamWithHumans == 0) {
				m_StartGameButton->SetVisible(false);
				m_StartErrorLabel->SetText("Assign human players to at least one team!");
				m_StartErrorLabel->SetVisible(true);
			} else {
				m_StartGameButton->SetVisible(true);
				m_StartErrorLabel->SetVisible(false);
			}
		}
		std::string goldString(32, '\0');
		if (m_StartingGoldSlider->GetValue() == m_StartingGoldSlider->GetMaximum()) {
			std::snprintf(goldString.data(), goldString.size(), "Starting Gold: %c Infinite", -58);
		} else {
			int startGold = m_StartingGoldSlider->GetValue();
			startGold = startGold - (startGold % 500);
			std::snprintf(goldString.data(), goldString.size(), "Starting Gold: %c %d oz", -58, startGold);
		}
		m_StartingGoldLabel->SetText(goldString);

		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; teamIndex++) {
			m_TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_TeamAISkillSlider.at(teamIndex)->GetValue()));
		}


		HandleInputEvents();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_CancelConfigButton) {
					g_GUISound.BackButtonPressSound()->Play();
					SetEnabled(false);
				} else if (guiEvent.GetControl() == m_StartGameButton) {
					if (StartGame()) {
						g_GUISound.ButtonPressSound()->Play();
						//return ScenarioMenuUpdateResult::ActivityStarted;
					} else {
						g_GUISound.UserErrorSound()->Play();
					}
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {


			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioActivityConfigGUI::Draw() {
		m_GUIControlManager->Draw();

		// Draw the Player-Team matrix lines and disabled overlay effects.
		int linePosY = 50;
		for (int teamIndex = Activity::Teams::TeamOne; teamIndex < Activity::Teams::MaxTeamCount; ++teamIndex) {
			if (m_SelectedActivity && (!m_SelectedActivity->TeamActive(teamIndex) || m_LockedCPUTeam == teamIndex)) {
				// Apply a colored overlay on top of team rows that are not human-playable.
				drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
				int blendAmount = 230;
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				//rectfill(g_FrameMan.GetBackBuffer32(), m_PlayersAndTeamsConfigBox->GetXPos() + 100, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY, m_PlayersAndTeamsConfigBox->GetXPos() + m_PlayersAndTeamsConfigBox->GetWidth() - 3, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY + 25, c_GUIColorDarkBlue);
				rectfill(g_FrameMan.GetBackBuffer32(), m_PlayersAndTeamsConfigBox->GetXPos() + 2, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY, m_PlayersAndTeamsConfigBox->GetXPos() + m_PlayersAndTeamsConfigBox->GetWidth() - 3, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY + 25, c_GUIColorDarkBlue);
				drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
			}
			linePosY += 25;
		}
		linePosY = 25;
		for (int i = 0; i < 6; ++i) {
			hline(g_FrameMan.GetBackBuffer32(), m_PlayersAndTeamsConfigBox->GetXPos() + 2, m_PlayersAndTeamsConfigBox->GetYPos() + linePosY, m_PlayersAndTeamsConfigBox->GetXPos() + m_PlayersAndTeamsConfigBox->GetWidth() - 2, c_GUIColorLightBlue);
			linePosY += 25;
		}
		// Manually draw UI elements on top of colored rectangle.
		for (int teamIndex = Activity::Teams::MaxTeamCount - 1; teamIndex >= Activity::Teams::TeamOne; --teamIndex) {
			if (m_TeamTechSelect.at(teamIndex)->GetVisible()) {
				m_TeamTechSelect.at(teamIndex)->Draw(m_GUIControlManager->GetScreen());
				if (m_TeamTechSelect.at(teamIndex)->IsDropped()) { m_TeamTechSelect.at(teamIndex)->GetListPanel()->Draw(m_GUIControlManager->GetScreen()); }
			}
			if (m_TeamAISkillSlider.at(teamIndex)->GetVisible()) {
				m_TeamAISkillSlider.at(teamIndex)->Draw(m_GUIControlManager->GetScreen());
				m_TeamAISkillLabel.at(teamIndex)->Draw(m_GUIControlManager->GetScreen());
			}
		}
	}
}