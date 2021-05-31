#include "ScenarioActivityConfigGUI.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUIComboBox.h"
#include "GUICheckbox.h"
#include "GUIButton.h"
#include "GUILabel.h"
#include "GUISlider.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioActivityConfigGUI::ScenarioActivityConfigGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_ActivitySetupBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("PlayerSetupBox"));
		m_ActivitySetupBox->CenterInParent(true, true);
		m_ActivitySetupBox->SetVisible(false);

		/*
		m_ActivitySetupBox.TeamBoxes.at(TeamRows::DisabledTeam) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("TDIcon"));
		m_ActivitySetupBox.TeamBoxes.at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
		m_ActivitySetupBox.TeamNameLabels.at(TeamRows::DisabledTeam) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("TDLabel"));

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			std::string teamNumber = std::to_string(teamIndex + 1);

			m_ActivitySetupBox.TeamBoxes.at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("T" + teamNumber + "Icon"));
			m_ActivitySetupBox.TeamBoxes.at(teamIndex)->SetDrawType(GUICollectionBox::Image);

			m_ActivitySetupBox.TeamNameLabels.at(teamIndex) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("T" + teamNumber + "Label"));

			m_ActivitySetupBox.TeamTechSelect.at(teamIndex) = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("T" + teamNumber + "TechCombo"));
			m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-All-", "", nullptr, nullptr, -2);
			m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-Random-", "", nullptr, nullptr, -1);
			m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);

			m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex) = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("T" + teamNumber + "AISkillSlider"));
			m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetValue(Activity::DefaultSkill);

			m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("T" + teamNumber + "AISkillLabel"));
			m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->GetValue()));
		}

		for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
			for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
				m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("P" + std::to_string(playerIndex + 1) + "T" + std::to_string(teamIndex + 1) + "Box"));
			}
		}

		m_ActivitySetupBox.GoldLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("GoldLabel"));
		m_ActivitySetupBox.GoldSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("GoldSlider"));
		m_ActivitySetupBox.FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("FogOfWarCheckbox"));
		m_ActivitySetupBox.RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("RequireClearPathToOrbitCheckbox"));
		m_ActivitySetupBox.DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("DeployUnitsCheckbox"));

		m_ActivitySetupBox.StartErrorLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("StartErrorLabel"));
		m_ActivitySetupBox.CPULockLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("CPULockLabel"));

		// Populate the tech ComboBoxes with the available tech modules.
		std::string techString = " Tech";
		for (int moduleIndex = 0; moduleIndex < g_PresetMan.GetTotalModuleCount(); ++moduleIndex) {
			const DataModule *dataModule = g_PresetMan.GetDataModule(moduleIndex);
			if (dataModule) {
				std::string techName = dataModule->GetFriendlyName();
				size_t techPos = techName.find(techString);
				if (techPos != string::npos) {
					techName.replace(techPos, techString.length(), "");
					for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
						m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem(techName, "", nullptr, nullptr, moduleIndex);
					}
				}
			}
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void ScenarioGUI::ShowPlayersBox() {
		m_ActivitySetupBox.ActivitySetupBox->SetVisible(true);

		if (m_SelectedActivity && m_SelectedScene) {
			if (const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity)) {
				m_LockedCPUTeam = gameActivity->GetCPUTeam();
				if (m_LockedCPUTeam != Activity::NoTeam) { m_ActivitySetupBox.CPULockLabel->SetPositionAbs(m_ActivitySetupBox.CPULockLabel->GetXPos(), m_ActivitySetupBox.TeamNameLabels.at(m_LockedCPUTeam)->GetYPos()); }
			}

			m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
			m_ActivitySetupBox.StartErrorLabel->SetVisible(true);
			m_ActivitySetupBox.CPULockLabel->SetVisible(m_LockedCPUTeam != Activity::NoTeam);

			// Set up initial color for all cells.
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
					m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawType(GUICollectionBox::Color);
					m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawColor(c_GUIColorBlue);
				}
			}

			const Icon *iconPointer = nullptr;

			// Human players start on the disabled team row.
			for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
				m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
				iconPointer = g_UInputMan.GetSchemeIcon(playerIndex);
				if (iconPointer) { m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }
			}

			// CPU player either has a locked team or starts on the disabled team row.
			int InitialCPUTeam = (m_LockedCPUTeam != Activity::NoTeam) ? m_LockedCPUTeam : TeamRows::DisabledTeam;
			m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawType(GUICollectionBox::Image);
			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
			if (iconPointer) { m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
			m_ActivitySetupBox.TeamNameLabels.at(TeamRows::DisabledTeam)->SetText("Not Playing:");
			if (iconPointer) { m_ActivitySetupBox.TeamBoxes.at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				// Reset GUI controls to default values.
				m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);
				m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetValue(Activity::DefaultSkill);

				if (m_SelectedActivity->TeamActive(teamIndex)) {
					iconPointer = m_SelectedActivity->GetTeamIcon(teamIndex);
					if (!iconPointer) {
						std::string teamString = "Team " + std::to_string(teamIndex + 1) + " Default";
						iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", teamString));
					}
					m_ActivitySetupBox.TeamNameLabels.at(teamIndex)->SetText(m_SelectedActivity->GetTeamName(teamIndex) + ":");
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetEnabled(true);
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetVisible(true);
					m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetEnabled(true);
					m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetVisible(true);
					m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex)->SetVisible(true);
				} else {
					iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
					m_ActivitySetupBox.TeamNameLabels.at(teamIndex)->SetText("Unavailable");
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetEnabled(false);
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetVisible(false);
					m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetEnabled(false);
					m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->SetVisible(false);
					m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex)->SetVisible(false);
				}
				if (iconPointer) { m_ActivitySetupBox.TeamBoxes.at(teamIndex)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }
			}
		}
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	bool ScenarioGUI::StartGame() {
		Activity *activityInstance = dynamic_cast<Activity *>(m_SelectedActivity->Clone());
		GameActivity *gameActivity = dynamic_cast<GameActivity *>(activityInstance);

		if (gameActivity) {
			gameActivity->SetDifficulty(m_ActivityInfoBox.DifficultySlider->GetValue());
			gameActivity->SetFogOfWarEnabled(m_ActivitySetupBox.FogOfWarCheckbox->GetCheck());
			gameActivity->SetRequireClearPathToOrbit(m_ActivitySetupBox.RequireClearPathToOrbitCheckbox->GetCheck());
			gameActivity->SetStartingGold((m_ActivitySetupBox.GoldSlider->GetValue() == m_ActivitySetupBox.GoldSlider->GetMaximum()) ? 9999999 : m_ActivitySetupBox.GoldSlider->GetValue() - (m_ActivitySetupBox.GoldSlider->GetValue() % 500));
		}
		g_SceneMan.SetSceneToLoad(m_SelectedScene, true, m_ActivitySetupBox.DeployUnitsCheckbox->GetCheck());

		activityInstance->ClearPlayers(false);
		for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				if (m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					activityInstance->AddPlayer(playerIndex, true, teamIndex, 0);
					break;
				}
			}
		}

		if (gameActivity) {
			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				if (m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					gameActivity->SetCPUTeam(teamIndex);
					break;
				}
			}
		}

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			if (const GUIListPanel::Item *techItem = m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetSelectedItem()) {
				// ExtraIndex -2 is "All", -1 "Random"
				if (techItem->m_ExtraIndex == -2) {
					gameActivity->SetTeamTech(teamIndex, "-All-");
				} else if (techItem->m_ExtraIndex == -1) {
					int selection = RandomNum<int>(2, m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetListPanel()->GetItemList()->size() - 1);
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->SetSelectedIndex(selection);
					gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetSelectedItem()->m_ExtraIndex));
				} else {
					gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(techItem->m_ExtraIndex));
				}
			}
			gameActivity->SetTeamAISkill(teamIndex, (m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->IsEnabled()) ? m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->GetValue() : Activity::DefaultSkill);
		}

		g_LuaMan.FileCloseAll();
		g_ActivityMan.SetStartActivity(activityInstance);

		return true;
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void ScenarioGUI::UpdatePlayersBox() {
		RTEAssert(m_SelectedActivity && m_SelectedScene, "Trying to start a scenario game without an activity or a scene.");

		int mouseX;
		int mouseY;
		m_GUIControlManager->GetManager()->GetInputController()->GetMousePosition(&mouseX, &mouseY);

		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity);
		if (const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControlUnderPoint(mouseX, mouseY, m_ActivitySetupBox.ActivitySetupBox, 1))) {
			// Find which cell is being hovered over.
			int hoveredPlayer = PlayerColumns::PlayerColumnCount;
			int hoveredTeam = TeamRows::TeamRowCount;
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
					if (m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex) == hoveredCell) {
						hoveredPlayer = playerIndex;
						hoveredTeam = teamIndex;
					} else if (m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Color) {
						// Un-highlight all other cells.
						m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->SetDrawColor(c_GUIColorBlue);
					}
				}
			}

			// Make the hovered cell light up and able to be selected if:
			// It's under an active team row or the disabled team row.
			// It's not a team row locked to the CPU.
			// It's not the CPU player if he is locked to a CPU team.
			// It doesn't already contain an image.
			if ((m_SelectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TeamRows::DisabledTeam) && m_LockedCPUTeam != hoveredTeam && (m_LockedCPUTeam == Activity::NoTeam || hoveredPlayer != PlayerColumns::PlayerCPU) && m_ActivitySetupBox.PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() != GUICollectionBox::Image) {
				if (g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER)) {
					ClickInPlayerSetup(hoveredPlayer, hoveredTeam);
				} else if (m_ActivitySetupBox.PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() == GUICollectionBox::Color && m_ActivitySetupBox.PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawColor() != c_GUIColorLightBlue) {
					// Just highlight the cell.
					m_ActivitySetupBox.PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->SetDrawColor(c_GUIColorLightBlue);
					g_GUISound.SelectionChangeSound()->Play();
				}
			}
		}

		int teamsWithPlayers = 0;
		bool teamWithHumans = false;
		int humansInTeams = 0;
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			bool foundPlayer = false;
			if (m_SelectedActivity->TeamActive(teamIndex)) {
				for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
					if (m_ActivitySetupBox.PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
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
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				std::string msgString = "Too many players assigned! Max for this activity is " + std::to_string(maxHumanPlayers);
				m_ActivitySetupBox.StartErrorLabel->SetText(msgString);
				m_ActivitySetupBox.StartErrorLabel->SetVisible(true);
			} else if (minTeamsRequired > teamsWithPlayers) {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				std::string msgString = "Assign players to at\nleast " + std::to_string(minTeamsRequired) + " of the teams!";
				m_ActivitySetupBox.StartErrorLabel->SetText(msgString);
				m_ActivitySetupBox.StartErrorLabel->SetVisible(true);
			} else if (teamWithHumans == 0) {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				m_ActivitySetupBox.StartErrorLabel->SetText("Assign human players\nto at least one team!");
				m_ActivitySetupBox.StartErrorLabel->SetVisible(true);
			} else {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(true);
				m_ActivitySetupBox.StartErrorLabel->SetVisible(false);
			}
		}

		char goldString[256];
		if (m_ActivitySetupBox.GoldSlider->GetValue() == m_ActivitySetupBox.GoldSlider->GetMaximum()) {
			std::snprintf(goldString, sizeof(goldString), "Starting Gold: %c Infinite", -58);
		} else {
			int startGold = m_ActivitySetupBox.GoldSlider->GetValue();
			startGold = startGold - (startGold % 500);
			std::snprintf(goldString, sizeof(goldString), "Starting Gold: %c %d oz", -58, startGold);
		}
		m_ActivitySetupBox.GoldLabel->SetText(goldString);

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; teamIndex++) {
			m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->GetValue()));
		}
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void ScenarioGUI::ClickInPlayerSetup(int clickedPlayer, int clickedTeam) {
		// Move the player's icon to the correct row.
		m_ActivitySetupBox.PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Image);
		const Icon *playerIcon = (clickedPlayer != PlayerColumns::PlayerCPU) ? g_UInputMan.GetSchemeIcon(clickedPlayer) : dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (playerIcon) { m_ActivitySetupBox.PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }

		for (int nonHoveredTeam = Activity::TeamOne; nonHoveredTeam < TeamRows::TeamRowCount; ++nonHoveredTeam) {
			if (nonHoveredTeam != clickedTeam) {
				m_ActivitySetupBox.PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawType(GUICollectionBox::Color);
				m_ActivitySetupBox.PlayerBoxes.at(clickedPlayer).at(nonHoveredTeam)->SetDrawColor(c_GUIColorBlue);
			}
		}

		// Remove human players from their team row if a CPU player is added to it, and vice - versa for CPU players.
		if (clickedPlayer == PlayerColumns::PlayerCPU && clickedTeam != TeamRows::DisabledTeam) {
			for (int humanPlayer = Players::PlayerOne; humanPlayer < Players::MaxPlayerCount; ++humanPlayer) {
				if (m_ActivitySetupBox.PlayerBoxes.at(humanPlayer).at(clickedTeam)->GetDrawType() == GUICollectionBox::Image) {
					m_ActivitySetupBox.PlayerBoxes.at(humanPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Color);
					m_ActivitySetupBox.PlayerBoxes.at(humanPlayer).at(clickedTeam)->SetDrawColor(c_GUIColorBlue);
					m_ActivitySetupBox.PlayerBoxes.at(humanPlayer).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
					playerIcon = g_UInputMan.GetSchemeIcon(humanPlayer);
					if (playerIcon) { m_ActivitySetupBox.PlayerBoxes.at(humanPlayer).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }
				}
			}
		} else if (clickedPlayer != PlayerColumns::PlayerCPU && clickedTeam != TeamRows::DisabledTeam && m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->GetDrawType() == GUICollectionBox::Image) {
			m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->SetDrawType(GUICollectionBox::Color);
			m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(clickedTeam)->SetDrawColor(c_GUIColorBlue);
			m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
			playerIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
			if (playerIcon) { m_ActivitySetupBox.PlayerBoxes.at(PlayerColumns::PlayerCPU).at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }
		}
		g_GUISound.FocusChangeSound()->Play();
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	handleinputevents {
		} else if (m_ActivitySetupBox.ActivitySetupBox->GetVisible() && guiEvent.GetType() == GUIEvent::Command) {
			if (eventControlName == "PlayerCancelButton") {
				HideAllScreens();
				m_ActivityInfoBox.ActivityInfoBox->SetVisible(true);
				ShowScenesBox();
				g_GUISound.BackButtonPressSound()->Play();
			} else if (eventControl == m_ScenarioButtons.at(ScenarioButtons::StartGameButton)) {
				if (StartGame()) {
					HideAllScreens();
					m_ActivityInfoBox.ActivityInfoBox->SetVisible(true);
					g_GUISound.ButtonPressSound()->Play();
					return ScenarioMenuUpdateResult::ActivityStarted;
				} else {
					g_GUISound.UserErrorSound()->Play();
				}
			}
		}
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	draw {
		if (m_ActivitySetupBox.ActivitySetupBox->GetVisible()) {
			GUICollectionBox *playerSetupBox = m_ActivitySetupBox.ActivitySetupBox;

			// Draw the Player-Team matrix lines and disabled overlay effects.
			int linePosY = 80;
			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				if (m_SelectedActivity && (!m_SelectedActivity->TeamActive(teamIndex) || m_LockedCPUTeam == teamIndex)) {
					// Apply a colored overlay on top of team rows that are not human-playable.
					drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
					int blendAmount = 230;
					set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
					rectfill(drawBitmap, playerSetupBox->GetXPos() + 110, playerSetupBox->GetYPos() + linePosY, playerSetupBox->GetXPos() + playerSetupBox->GetWidth() - 12, playerSetupBox->GetYPos() + linePosY + 25, c_GUIColorDarkBlue);
					drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
				}
				// Cell border separator lines.
				hline(drawBitmap, playerSetupBox->GetXPos() + 110, playerSetupBox->GetYPos() + linePosY, playerSetupBox->GetXPos() + playerSetupBox->GetWidth() - 12, c_GUIColorLightBlue);
				linePosY += 25;
			}

			AllegroScreen drawScreen(drawBitmap);

			// Manually draw UI elements on top of colored rectangle.
			for (int teamIndex = Activity::MaxTeamCount - 1; teamIndex >= Activity::TeamOne; --teamIndex) {
				if (m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetVisible()) {
					m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->Draw(&drawScreen);
					if (m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->IsDropped()) { m_ActivitySetupBox.TeamTechSelect.at(teamIndex)->GetListPanel()->Draw(&drawScreen); }
				}
				if (m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->GetVisible()) {
					m_ActivitySetupBox.TeamAISkillSlider.at(teamIndex)->Draw(&drawScreen);
					m_ActivitySetupBox.TeamAISkillLabel.at(teamIndex)->Draw(&drawScreen);
				}
			}
		}
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



}