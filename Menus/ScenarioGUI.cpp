#include "ScenarioGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "MetaMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIComboBox.h"
#include "GUI/GUICheckbox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"

#include "DataModule.h"
#include "Entity.h"
#include "Scene.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioGUI::ScenarioGUI() {
		m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
		m_GUIInput = std::make_unique<AllegroInput>(-1, true);
		m_ScenarioGUIController = std::make_unique<GUIControlManager>();

		if (!m_ScenarioGUIController->Create(m_GUIScreen.get(), m_GUIInput.get(), "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_ScenarioGUIController->Load("Base.rte/GUIs/ScenarioGUI.ini");

		m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("root"));
		m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ActivitySelectBox"));
		m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->SetPositionRel(16, 16);
		m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("SceneInfoBox"));
		m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->SetPositionRel(m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetWidth() - m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetWidth() - 16, 16);
		m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("PlayerSetupBox"));
		m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->CenterInParent(true, true);

		m_ScenarioButtons.at(ScenarioButtons::BackToMainButton) = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("BackToMainButton"));
		m_ScenarioButtons.at(ScenarioButtons::BackToMainButton)->SetPositionRel(m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetWidth() - m_ScenarioButtons.at(ScenarioButtons::BackToMainButton)->GetWidth() - 16, m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetHeight() - m_ScenarioButtons.at(ScenarioButtons::BackToMainButton)->GetHeight() - 22);
		m_ScenarioButtons.at(ScenarioButtons::ResumeButton) = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ButtonResume"));
		m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->SetPositionRel(m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetWidth() - m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->GetWidth() - 16, m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetHeight() - m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->GetHeight() - 47);
		m_ScenarioButtons.at(ScenarioButtons::StartHereButton) = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneSelectButton"));
		m_ScenarioButtons.at(ScenarioButtons::StartGameButton) = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("StartButton"));

		m_ActivitySelectComboBox = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("ActivitySelectCombo"));
		m_ActivityLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ActivityDescLabel"));
		m_ActivityLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));
		m_DifficultyLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("DifficultyLabel"));
		m_DifficultySlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("DifficultySlider"));

		m_SceneCloseButton = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneCloseButton"));
		m_SceneNameLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneNameLabel"));
		m_SceneInfoLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneInfoLabel"));
		m_SceneInfoLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));
		m_ScenePreviewBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ScenePreviewBox"));
		m_ScenePreviewBox->SetPositionRel(10, 33);
		m_ScenePreviewBox->SetDrawType(GUICollectionBox::Image);
		m_DefaultPreviewBitmap = std::make_unique<AllegroBitmap>();
		m_DefaultPreviewBitmap->Create("Base.rte/GUIs/DefaultPreview.png");
		m_ScenePreviewBitmap = std::make_unique<AllegroBitmap>();
		m_ScenePreviewBitmap->Create(Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT, 32);
		m_SitePointLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ScenePlanetLabel"));

		for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
			for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
				m_PlayerBoxes.at(playerIndex).at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("P" + std::to_string(playerIndex + 1) + "T" + std::to_string(teamIndex + 1) + "Box"));
			}
		}

		m_TeamBoxes.at(TeamRows::DisabledTeam) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("TDIcon"));
		m_TeamBoxes.at(TeamRows::DisabledTeam)->SetDrawType(GUICollectionBox::Image);
		m_TeamNameLabels.at(TeamRows::DisabledTeam) = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("TDLabel"));

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			std::string teamNumber = std::to_string(teamIndex + 1);

			m_TeamBoxes.at(teamIndex) = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("T" + teamNumber + "Icon"));
			m_TeamBoxes.at(teamIndex)->SetDrawType(GUICollectionBox::Image);

			m_TeamNameLabels.at(teamIndex) = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T" + teamNumber + "Label"));

			m_TeamTechSelect.at(teamIndex) = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("T" + teamNumber + "TechCombo"));
			m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-All-", "", nullptr, nullptr, -2);
			m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem("-Random-", "", nullptr, nullptr, -1);
			m_TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);

			m_TeamAISkillSlider.at(teamIndex) = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("T" + teamNumber + "AISkillSlider"));
			m_TeamAISkillSlider.at(teamIndex)->SetValue(Activity::DefaultSkill);

			m_TeamAISkillLabel.at(teamIndex) = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T" + teamNumber + "AISkillLabel"));
			m_TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_TeamAISkillSlider.at(teamIndex)->GetValue()));
		}

		m_StartErrorLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("StartErrorLabel"));
		m_CPULockLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("CPULockLabel"));

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
						m_TeamTechSelect.at(teamIndex)->GetListPanel()->AddItem(techName, "", nullptr, nullptr, moduleIndex);
					}
				}
			}
		}

		m_GoldLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("GoldLabel"));
		m_GoldSlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("GoldSlider"));
		m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("FogOfWarCheckbox"));
		m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
		m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("DeployUnitsCheckbox"));

		GetScenesAndActivities(true);
		UpdateActivityBox();
		HideAllScreens();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetEnabled() {
		HideAllScreens();
		m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->SetVisible(true);
		// Reload all scenes and activities to reflect scene changes player might do in scene editor.
		GetScenesAndActivities(false);

		const Activity *currentActivity = g_ActivityMan.GetActivity();
		if (currentActivity && (currentActivity->GetActivityState() == Activity::Running || currentActivity->GetActivityState() == Activity::Editing)) {
			if (!m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->GetVisible()) { m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->SetVisible(true); }
		} else if (m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->GetVisible()) {
			m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->SetVisible(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetPlanetInfo(const Vector &center, float radius) {
		bool centerChanged = (center != m_PlanetCenter);
		m_PlanetCenter = center;
		m_PlanetRadius = radius;
		if (centerChanged && m_SelectedScene) { CalculateLinesToSitePoint(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::ShowScenesBox() {
		clear_to_color(m_ScenePreviewBitmap->GetBitmap(), ColorKeys::g_MaskColor);
		if (m_SelectedScene) {
			m_SceneNameLabel->SetText(m_SelectedScene->GetPresetName());
			m_SceneInfoLabel->SetText(m_SelectedScene->GetDescription());

			BITMAP *preview = m_SelectedScene->GetPreviewBitmap();
			if (!preview) { preview = m_DefaultPreviewBitmap->GetBitmap(); }
			draw_sprite(m_ScenePreviewBitmap->GetBitmap(), preview, 0, 0);
			m_ScenePreviewBox->SetDrawImage(new AllegroBitmap(m_ScenePreviewBitmap->GetBitmap()));
		}

		int textHeight = m_SceneInfoLabel->ResizeHeightToFit();
		int padding = 140;
		m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->Resize(m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetWidth(), textHeight + padding);
		KeepBoxInScreenBounds(m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox));
		m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->SetVisible(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::ShowPlayersBox() {
		m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->SetVisible(true);

		if (m_SelectedActivity && m_SelectedScene) {
			int mouseX = 0;
			int mouseY = 0;
			m_GUIInput->GetMousePosition(&mouseX, &mouseY);

			const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity);
			if (gameActivity) {
				m_LockedCPUTeam = gameActivity->GetCPUTeam();
				if (m_LockedCPUTeam != Activity::NoTeam) { m_CPULockLabel->SetPositionAbs(m_CPULockLabel->GetXPos(), m_TeamNameLabels.at(m_LockedCPUTeam)->GetYPos()); }
			}

			m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
			m_StartErrorLabel->SetVisible(true);
			m_CPULockLabel->SetVisible(m_LockedCPUTeam != Activity::NoTeam);

			// Set up initial color for all cells.
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
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
			int InitialCPUTeam = (m_LockedCPUTeam != Activity::NoTeam) ? m_LockedCPUTeam : TeamRows::DisabledTeam;
			m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawType(GUICollectionBox::Image);
			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
			if (iconPointer) { m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(InitialCPUTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
			m_TeamNameLabels.at(TeamRows::DisabledTeam)->SetText("Not Playing:");
			if (iconPointer) { m_TeamBoxes.at(TeamRows::DisabledTeam)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }

			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				// Reset GUI controls to default values.
				m_TeamTechSelect.at(teamIndex)->SetSelectedIndex(0);
				m_TeamAISkillSlider.at(teamIndex)->SetValue(Activity::DefaultSkill);

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
				if (iconPointer) { m_TeamBoxes.at(teamIndex)->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0])); }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::HideAllScreens() {
		for (int collectionBox = 1; collectionBox < ScenarioCollections::CollectionBoxCount; ++collectionBox) {
			m_ScenarioCollectionBoxes.at(collectionBox)->SetVisible(false);
		}
		m_SitePointLabel->SetVisible(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::KeepBoxInScreenBounds(GUICollectionBox *collectionBox) const {
		int clampedPosX = std::clamp(collectionBox->GetXPos(), 0, m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetWidth() - collectionBox->GetWidth());
		int clampedPosY = std::clamp(collectionBox->GetYPos(), 0, m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox)->GetHeight() - collectionBox->GetHeight());
		collectionBox->SetPositionAbs(clampedPosX, clampedPosY);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::GetScenesAndActivities(bool selectTutorial) {
		m_ScenarioActivities.clear();
		m_ScenarioScenes = nullptr;

		std::list<Entity *> presetList;
		std::list<Scene *> filteredScenes;

		g_PresetMan.GetAllOfType(presetList, "Scene");

		for (Entity *presetEntity : presetList) {
			Scene *presetScene = dynamic_cast<Scene *>(presetEntity);
			if (presetScene && !presetScene->GetLocation().IsZero() && !presetScene->IsMetagameInternal() && (presetScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes())) { filteredScenes.push_back(presetScene); }
		}

		// We need to calculate planet center manually because m_PlanetCenter reflects coordinates of moving planet which is outside the screen when this is called first time.
		Vector planetCenter = m_PlanetCenter.IsZero() ? Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2) : m_PlanetCenter;

		// If a scene is on the planet but outside the screen then move it into the screen.
		for (Scene *filteredScene : filteredScenes) {
			float sceneYPos = (planetCenter + filteredScene->GetLocation() + filteredScene->GetLocationOffset()).GetY();
			if (std::abs(filteredScene->GetLocation().GetY()) < m_PlanetRadius + 100 && std::abs(filteredScene->GetLocation().GetX()) < m_PlanetRadius + 100) {
				if (sceneYPos < 10) { filteredScene->SetLocationOffset(filteredScene->GetLocationOffset() + Vector(0, 10 - sceneYPos)); }
				if (sceneYPos > g_FrameMan.GetResY() - 10) { filteredScene->SetLocationOffset(filteredScene->GetLocationOffset() + Vector(0, g_FrameMan.GetResY() - 10 - sceneYPos)); }
			}
		}

		// If site points are overlapping then move one of them towards the planet center.
		float requiredDistance = 8.0F;
		bool foundOverlap = true;
		while (foundOverlap) {
			foundOverlap = false;
			for (Scene *filteredScene1 : filteredScenes) {
				for (const Scene *filteredScene2 : filteredScenes) {
					if (filteredScene1 != filteredScene2) {
						Vector pos1 = filteredScene1->GetLocation() + filteredScene1->GetLocationOffset();
						Vector pos2 = filteredScene2->GetLocation() + filteredScene2->GetLocationOffset();
						Vector overlap = pos1 - pos2;
						float overlapMagnitude = overlap.GetMagnitude();

						if (overlapMagnitude < requiredDistance) {
							foundOverlap = true;
							float overlapY = overlap.GetY();
							float yDirMult = 0;

							if (overlapY > 0 && pos1.GetY() > 0) {
								yDirMult = -1.0F;
							} else if (overlapY < 0 && pos1.GetY() < 0) {
								yDirMult = 1.0F;
							}

							if (yDirMult != 0) {
								filteredScene1->SetLocationOffset(filteredScene1->GetLocationOffset() + Vector(0, -overlapY + (requiredDistance * yDirMult)));
							} else if (overlapMagnitude == 0.0F) {
								filteredScene1->SetLocationOffset(filteredScene1->GetLocationOffset() + Vector(0, (pos1.GetY() > 0) ? -requiredDistance : requiredDistance));
							} else {
								filteredScene1->SetLocationOffset(filteredScene1->GetLocationOffset() + Vector(0, overlapY));
							}
						}
					}
				}
			}
		}

		presetList.clear();
		g_PresetMan.GetAllOfType(presetList, "Activity");
		Activity *presetActivity = nullptr;

		int previousSelectedActivityIndex = m_ActivitySelectComboBox->GetSelectedIndex();

		// Associate all Scenes compatible with each Activity and populate the activities selection list.
		m_ActivitySelectComboBox->ClearList();
		int index = 0;
		int tutorialIndex = -1;
		for (Entity *presetEntity : presetList) {
			presetActivity = dynamic_cast<Activity *>(presetEntity);
			if (presetActivity && presetActivity->GetClassName().find("Editor") == std::string::npos) {
				std::pair<Activity *, std::vector<Scene *>> newPair(presetActivity, std::vector<Scene *>());
				for (Scene *filteredScene : filteredScenes) {
					if (presetActivity->SceneIsCompatible(filteredScene)) { newPair.second.push_back(filteredScene); }
				}
				m_ScenarioActivities.insert(newPair);
				// Add to the activity selection ComboBox, and attach the activity pointer, not passing in ownership.
				m_ActivitySelectComboBox->AddItem(presetActivity->GetPresetName(), "", nullptr, presetActivity);

				// Save the tutorial mission so we can select it by default.
				if (selectTutorial && presetActivity->GetClassName() == "GATutorial") { tutorialIndex = index; }
				index++;
			}
		}
		m_ActivitySelectComboBox->SetSelectedIndex((selectTutorial && tutorialIndex >= 0) ? tutorialIndex : previousSelectedActivityIndex);

		m_SelectedActivity = dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity);

		UpdateActivityBox();
		SetSelectedScene((m_ScenarioScenes && (m_ScenarioScenes->size() == 1 || selectTutorial)) ? m_ScenarioScenes->front() : nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::SetSelectedScene(Scene *newSelectedScene) {
		m_SelectedScene = newSelectedScene;
		if (m_SelectedScene) {
			ShowScenesBox();
			CalculateLinesToSitePoint();
		} else {
			m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->SetVisible(false);
			m_LineToSitePoints.clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ScenarioGUI::StartGame() {
		Activity *activityInstance = dynamic_cast<Activity *>(m_SelectedActivity->Clone());
		GameActivity *gameActivity = dynamic_cast<GameActivity *>(activityInstance);

		if (gameActivity) {
			gameActivity->SetDifficulty(m_DifficultySlider->GetValue());
			gameActivity->SetFogOfWarEnabled(m_FogOfWarCheckbox->GetCheck());
			gameActivity->SetRequireClearPathToOrbit(m_RequireClearPathToOrbitCheckbox->GetCheck());
			gameActivity->SetStartingGold((m_GoldSlider->GetValue() == m_GoldSlider->GetMaximum()) ? 9999999 : m_GoldSlider->GetValue() - (m_GoldSlider->GetValue() % 500));
		}
		g_SceneMan.SetSceneToLoad(m_SelectedScene, true, m_DeployUnitsCheckbox->GetCheck());

		activityInstance->ClearPlayers(false);
		for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				if (m_PlayerBoxes.at(playerIndex).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					activityInstance->AddPlayer(playerIndex, true, teamIndex, 0);
					break;
				}
			}
		}

		if (gameActivity) {
			for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
				if (m_PlayerBoxes.at(PlayerColumns::PlayerCPU).at(teamIndex)->GetDrawType() == GUICollectionBox::Image) {
					gameActivity->SetCPUTeam(teamIndex);
					break;
				}
			}
		}

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			const GUIListPanel::Item *techItem = m_TeamTechSelect.at(teamIndex)->GetSelectedItem();
			if (techItem) {
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
			gameActivity->SetTeamAISkill(teamIndex, (m_TeamAISkillSlider.at(teamIndex)->IsEnabled()) ? m_TeamAISkillSlider.at(teamIndex)->GetValue() : Activity::DefaultSkill);
		}

		g_LuaMan.FileCloseAll();
		g_ActivityMan.SetStartActivity(activityInstance);

		if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::CalculateLinesToSitePoint() {
		m_LineToSitePoints.clear();

		Vector sitePos = m_PlanetCenter + m_SelectedScene->GetLocation() + m_SelectedScene->GetLocationOffset();
		if (m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->PointInside(sitePos.GetFloorIntX(), sitePos.GetFloorIntY())) {
			return;
		}

		int halfBoxHeight = m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetHeight() / 2;
		Vector sceneBoxCenter(static_cast<float>(m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetXPos() + (m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetWidth() / 2)), static_cast<float>(m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetYPos() + halfBoxHeight));
		float yDirMult = sitePos.GetY() < sceneBoxCenter.GetY() ? -1.0F : 1.0F;
		Vector sceneBoxEdge = sceneBoxCenter + Vector(0, static_cast<float>(halfBoxHeight)) * yDirMult; // Point on the scene box where the line starts.
		int circleRadius = 8; // Radius of the circle drawn around the site point.
		int minStraightLength = 15; // Minimum length of straight line at the box edge and site point edge.
		int minSiteDistance = circleRadius + minStraightLength; // Minimum distance from a chamfer point to the site point.

		if (std::abs(sceneBoxCenter.GetFloorIntX() - sitePos.GetFloorIntX()) < minSiteDistance) {
			// No bends, meaning the line goes straight up/down to the site circle.
			m_LineToSitePoints.emplace_back(sceneBoxEdge + Vector(sitePos.GetX() - sceneBoxEdge.GetX(), 0));
			m_LineToSitePoints.emplace_back(sitePos + Vector(0, (static_cast<float>(circleRadius + 1)) * -yDirMult));
			return;
		}

		m_LineToSitePoints.emplace_back(sceneBoxEdge); // Point at the scene info box edge.

		int minChamferSize = 15; // Minimum x and y lengths of the chamfer.
		int maxChamferSize = 40; // Maximum x and y lengths of the chamfer.
		float xDirMult = sitePos.GetX() < sceneBoxEdge.GetX() ? -1.0F : 1.0F;

		int chamferSize;
		if (std::abs(sitePos.GetFloorIntY() - sceneBoxCenter.GetFloorIntY()) > halfBoxHeight + minStraightLength) {
			// One bend.
			Vector bendPoint(sceneBoxEdge.GetX(), sitePos.GetY()); // At this point the line bends. If the bend is chamfered then the two chamfer points will be equally distanced from this point.

			chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(sceneBoxEdge.GetFloorIntY() - sitePos.GetFloorIntY()) - minStraightLength);
			chamferSize = std::min(chamferSize, maxChamferSize);
			if (chamferSize < minChamferSize) { chamferSize = 0; }

			m_LineToSitePoints.emplace_back(Vector(bendPoint.GetX(), bendPoint.GetY() + static_cast<float>(chamferSize) * -yDirMult));
			if (chamferSize > 0) { m_LineToSitePoints.emplace_back(Vector(bendPoint.GetX() + static_cast<float>(chamferSize) * xDirMult, bendPoint.GetY())); }
			m_LineToSitePoints.emplace_back(sitePos + Vector((static_cast<float>(circleRadius + 1)) * -xDirMult, 0));
		} else {
			// Two bends.
			// extraLength ensures that there will be straight lines coming out of the site and the box, and that they are nearly as short as possible.
			int extraLength = std::clamp(minSiteDistance + (sitePos.GetFloorIntY() - sceneBoxEdge.GetFloorIntY()) * static_cast<int>(yDirMult), 0, minSiteDistance);

			Vector firstBend(sceneBoxEdge.GetX(), sceneBoxEdge.GetY() + (static_cast<float>(extraLength + minStraightLength)) * yDirMult);
			Vector secondBend(sitePos.GetX(), firstBend.GetY());

			chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(secondBend.GetFloorIntY() - sitePos.GetFloorIntY()) - minSiteDistance);
			chamferSize = std::min(chamferSize, maxChamferSize);
			if (chamferSize < minChamferSize) { chamferSize = 0; }

			m_LineToSitePoints.emplace_back(firstBend);
			m_LineToSitePoints.emplace_back(Vector(secondBend.GetX() + static_cast<float>(chamferSize) * -xDirMult, secondBend.GetY()));
			if (chamferSize > 0) { m_LineToSitePoints.emplace_back(Vector(secondBend.GetX(), secondBend.GetY() + static_cast<float>(chamferSize) * -yDirMult)); }
			m_LineToSitePoints.emplace_back(sitePos + Vector(0, (static_cast<float>(circleRadius + 1)) * yDirMult));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::UpdateActivityBox() {
		if (m_SelectedActivity) {
			// Pull out the list of Scenes that are compatible with this Activity.
			m_ScenarioScenes = nullptr;
			for (std::pair<Activity * const, std::vector<Scene *>> &activityScenePair : m_ScenarioActivities) {
				if (activityScenePair.first == m_SelectedActivity) {
					m_ScenarioScenes = &(activityScenePair.second);
					break;
				}
			}

			if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
				m_ActivityLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
			} else if (m_ScenarioScenes && m_ScenarioScenes->size() > 1) {
				m_ActivityLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
			} else {
				m_ActivityLabel->SetText(m_SelectedActivity->GetDescription() + "\n\nNo sites appear to be compatible with this selected activity! Please try another.");
			}

			m_DifficultyLabel->SetVisible(true);
			m_DifficultySlider->SetVisible(true);
			if (m_DifficultySlider->GetValue() < Activity::CakeDifficulty) {
				m_DifficultyLabel->SetText("Difficulty: Cake");
			} else if (m_DifficultySlider->GetValue() < Activity::EasyDifficulty) {
				m_DifficultyLabel->SetText("Difficulty: Easy");
			} else if (m_DifficultySlider->GetValue() < Activity::MediumDifficulty) {
				m_DifficultyLabel->SetText("Difficulty: Medium");
			} else if (m_DifficultySlider->GetValue() < Activity::HardDifficulty) {
				m_DifficultyLabel->SetText("Difficulty: Hard");
			} else if (m_DifficultySlider->GetValue() < Activity::NutsDifficulty) {
				m_DifficultyLabel->SetText("Difficulty: Nuts");
			} else {
				m_DifficultyLabel->SetText("Difficulty: Nuts!");
			}
			m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->Resize(m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetWidth(), m_ActivityLabel->ResizeHeightToFit() + 110);

			const GameActivity *selectedGA = dynamic_cast<const GameActivity *>(m_SelectedActivity);
			if (selectedGA) {
				if (m_DifficultySlider->GetValue() < Activity::CakeDifficulty && selectedGA->GetDefaultGoldCake() > -1) {
					m_GoldSlider->SetValue(selectedGA->GetDefaultGoldCake());
				} else if (m_DifficultySlider->GetValue() < Activity::EasyDifficulty && selectedGA->GetDefaultGoldEasy() > -1) {
					m_GoldSlider->SetValue(selectedGA->GetDefaultGoldEasy());
				} else if (m_DifficultySlider->GetValue() < Activity::MediumDifficulty && selectedGA->GetDefaultGoldMedium() > -1) {
					m_GoldSlider->SetValue(selectedGA->GetDefaultGoldMedium());
				} else if (m_DifficultySlider->GetValue() < Activity::HardDifficulty && selectedGA->GetDefaultGoldHard() > -1) {
					m_GoldSlider->SetValue(selectedGA->GetDefaultGoldHard());
				} else if (m_DifficultySlider->GetValue() < Activity::NutsDifficulty && selectedGA->GetDefaultGoldNuts() > -1) {
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
		} else {
			m_ScenarioScenes = nullptr;
			m_ActivityLabel->SetText("No Activity selected.");
			m_DifficultyLabel->SetVisible(false);
			if (m_DifficultySlider) { m_DifficultySlider->SetVisible(false); }

			m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->Resize(m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetWidth(), m_ActivityLabel->ResizeHeightToFit() + 125);
		}

		KeepBoxInScreenBounds(m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::UpdatePlayersBox() {
		RTEAssert(m_SelectedActivity && m_SelectedScene, "Trying to start a scenario game without an activity or a scene.");

		int mouseX = 0;
		int mouseY = 0;
		m_GUIInput->GetMousePosition(&mouseX, &mouseY);
		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_SelectedActivity);

		const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox), 1));
		if (hoveredCell) {
			// Find which cell is being hovered over.
			int hoveredPlayer = PlayerColumns::PlayerColumnCount;
			int hoveredTeam = TeamRows::TeamRowCount;
			for (int playerIndex = Players::PlayerOne; playerIndex < PlayerColumns::PlayerColumnCount; ++playerIndex) {
				for (int teamIndex = Activity::TeamOne; teamIndex < TeamRows::TeamRowCount; ++teamIndex) {
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
			if ((m_SelectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TeamRows::DisabledTeam) && m_LockedCPUTeam != hoveredTeam && (m_LockedCPUTeam == Activity::NoTeam || hoveredPlayer != PlayerColumns::PlayerCPU) && m_PlayerBoxes.at(hoveredPlayer).at(hoveredTeam)->GetDrawType() != GUICollectionBox::Image) {
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
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
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
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				std::string msgString = "Too many players assigned! Max for this activity is " + std::to_string(maxHumanPlayers);
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (minTeamsRequired > teamsWithPlayers) {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				std::string msgString = "Assign players to at\nleast " + std::to_string(minTeamsRequired) + " of the teams!";
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (teamWithHumans == 0) {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(false);
				m_StartErrorLabel->SetText("Assign human players\nto at least one team!");
				m_StartErrorLabel->SetVisible(true);
			} else {
				m_ScenarioButtons.at(ScenarioButtons::StartGameButton)->SetVisible(true);
				m_StartErrorLabel->SetVisible(false);
			}
		}

		char goldString[256];
		if (m_GoldSlider->GetValue() == m_GoldSlider->GetMaximum()) {
			std::snprintf(goldString, sizeof(goldString), "Starting Gold: %c Infinite", -58);
		} else {
			int startGold = m_GoldSlider->GetValue();
			startGold = startGold - (startGold % 500);
			std::snprintf(goldString, sizeof(goldString), "Starting Gold: %c %d oz", -58, startGold);
		}
		m_GoldLabel->SetText(goldString);

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; teamIndex++) {
			m_TeamAISkillLabel.at(teamIndex)->SetText(Activity::GetAISkillString(m_TeamAISkillSlider.at(teamIndex)->GetValue()));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::ClickInPlayerSetup(int clickedPlayer, int clickedTeam) {
		// Move the player's icon to the correct row.
		m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawType(GUICollectionBox::Image);
		const Icon *playerIcon = (clickedPlayer != PlayerColumns::PlayerCPU) ? g_UInputMan.GetSchemeIcon(clickedPlayer) : dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (playerIcon) { m_PlayerBoxes.at(clickedPlayer).at(clickedTeam)->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0])); }

		for (int nonHoveredTeam = Activity::TeamOne; nonHoveredTeam < TeamRows::TeamRowCount; ++nonHoveredTeam) {
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

	void ScenarioGUI::UpdateHoveredScene(int mouseX, int mouseY) {
		bool foundAnyHover = false;
		if (m_ScenarioScenes && !m_DraggedBox && !m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->PointInside(mouseX, mouseY) && !m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->PointInside(mouseX, mouseY)) {
			Scene *candidateScene = nullptr;
			float shortestDist = 10.0F;
			Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
			for (Scene *scenarioScene : *m_ScenarioScenes) {
				float distance = (m_PlanetCenter + scenarioScene->GetLocation() + scenarioScene->GetLocationOffset() - mousePos).GetMagnitude();
				if (distance < shortestDist) {
					shortestDist = distance;
					candidateScene = scenarioScene;
					foundAnyHover = true;
				}
			}

			// Set new hovered scene to be the one closest to the cursor, if there is any and if it is different than the currently hovered one.
			if (candidateScene && candidateScene != m_HoveredScene) {
				m_HoveredScene = candidateScene;
				g_GUISound.SelectionChangeSound()->Play();

				m_SitePointLabel->SetText(m_HoveredScene->GetPresetName());

				Vector labelAbsPos = m_PlanetCenter + Vector(m_HoveredScene->GetLocation() + m_HoveredScene->GetLocationOffset()) - Vector(m_SitePointLabel->GetWidth() / 2, 0.0F) - Vector(0.0F, m_SitePointLabel->GetHeight() * 1.5F);
				float padding = 6.0F;
				labelAbsPos.m_X = std::clamp(labelAbsPos.m_X, padding, g_FrameMan.GetResX() - m_SitePointLabel->GetWidth() - padding);
				labelAbsPos.m_Y = std::clamp(labelAbsPos.m_Y, padding, g_FrameMan.GetResY() - m_SitePointLabel->GetHeight() - padding);
				m_SitePointLabel->SetPositionAbs(labelAbsPos.GetFloorIntX(), labelAbsPos.GetFloorIntY());

				m_SitePointLabel->SetVisible(true);
			}
		}
		if (!foundAnyHover) {
			m_HoveredScene = nullptr;
			m_SitePointLabel->SetVisible(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioGUI::ScenarioUpdateResult ScenarioGUI::UpdateInput() {
		// TODO: if activity is running, allow esc to resume activity instead of quitting.
		if (g_UInputMan.KeyPressed(KEY_ESC)) {
			g_GUISound.BackButtonPressSound()->Play();
			if (m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->GetVisible()) {
				HideAllScreens();
				m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->SetVisible(true);
				ShowScenesBox();
			} else {
				return ScenarioUpdateResult::BackToMain;
			}
		}

		int mouseX = 0;
		int mouseY = 0;
		m_GUIInput->GetMousePosition(&mouseX, &mouseY);
		Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

		// Handle mouse hover and drag.
		if (m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetVisible()) {
			UpdateHoveredScene(mouseX, mouseY);

			if (g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER) && m_DraggedBox) {
				m_DraggedBox->MoveRelative(mousePos.GetFloorIntX() - m_PrevMousePos.GetFloorIntX(), mousePos.GetFloorIntY() - m_PrevMousePos.GetFloorIntY());
				m_PrevMousePos = mousePos;

				KeepBoxInScreenBounds(m_DraggedBox);
				if (m_DraggedBox == m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)) { CalculateLinesToSitePoint(); }
			} else {
				m_DraggedBox = nullptr;
			}

			if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER)) {
				if (m_HoveredScene) {
					SetSelectedScene(m_HoveredScene);
					g_GUISound.ItemChangeSound()->Play();
				}

				GUICollectionBox *hoveredBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioCollectionBoxes.at(ScenarioCollections::RootBox), 1));
				const GUIControl *hoveredControl = m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, hoveredBox, 1);
				bool nonDragControl = (dynamic_cast<const GUIButton *>(hoveredControl) || dynamic_cast<const GUISlider *>(hoveredControl) || dynamic_cast<const GUIComboBox *>(hoveredControl));
				if (hoveredBox && !nonDragControl && !m_DraggedBox && !m_ActivitySelectComboBox->IsDropped()) {
					m_DraggedBox = hoveredBox;
					m_PrevMousePos = mousePos;
				}
			}
		}

		m_ScenarioGUIController->Update();

		///////////////////////////////////////
		// Handle events

		GUIEvent guiEvent;
		while (m_ScenarioGUIController->GetEvent(&guiEvent)) {
			const GUIControl *eventControl = guiEvent.GetControl();
			std::string eventControlName = guiEvent.GetControl()->GetName();

			if (m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetVisible()) {
				if (guiEvent.GetType() == GUIEvent::Command) {
					if (eventControlName == "BackToMainButton") {
						HideAllScreens();
						g_GUISound.BackButtonPressSound()->Play();
						return ScenarioUpdateResult::BackToMain;
					} else if (eventControlName == "ButtonResume") {
						g_GUISound.BackButtonPressSound()->Play();
						return ScenarioUpdateResult::ActivityResumed;
					}

					if (m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetVisible()) {
						if (eventControl == m_ScenarioButtons.at(ScenarioButtons::StartHereButton)) {
							HideAllScreens();
							ShowPlayersBox();
							g_GUISound.ButtonPressSound()->Play();
						} else if (eventControl == m_SceneCloseButton) {
							SetSelectedScene(nullptr);
							g_GUISound.ButtonPressSound()->Play();
						}
					}
				} else if (guiEvent.GetType() == GUIEvent::Notification) {
					if (eventControl == m_ActivitySelectComboBox && guiEvent.GetMsg() == GUIComboBox::Closed) {
						const Activity *activity = dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity);
						if (activity != m_SelectedActivity) {
							m_SelectedActivity = activity;
							UpdateActivityBox();

							// Deselect any previously selected scene. it may not be compatible with the new activity.
							SetSelectedScene((m_ScenarioScenes && m_ScenarioScenes->size() == 1) ? m_ScenarioScenes->front() : nullptr);

							g_GUISound.ItemChangeSound()->Play();
						}
					} else if (eventControl == m_DifficultySlider) {
						UpdateActivityBox();
					}
				}
			} else if (m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->GetVisible() && guiEvent.GetType() == GUIEvent::Command) {
				if (eventControlName == "PlayerCancelButton") {
					HideAllScreens();
					m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->SetVisible(true);
					ShowScenesBox();
					g_GUISound.BackButtonPressSound()->Play();
				} else if (eventControl == m_ScenarioButtons.at(ScenarioButtons::StartGameButton)) {
					if (StartGame()) {
						HideAllScreens();
						m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->SetVisible(true);
						g_GUISound.ButtonPressSound()->Play();
						return ScenarioUpdateResult::ActivityRestarted;
					} else {
						g_GUISound.UserErrorSound()->Play();
					}
				}
			}
			if (dynamic_cast<const GUIButton *>(eventControl) && guiEvent.GetType() == GUIEvent::Notification && guiEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
		}
		return ScenarioUpdateResult::NoEvent;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	ScenarioGUI::ScenarioUpdateResult ScenarioGUI::Update() {
		if (g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) {
			return ScenarioUpdateResult::NoEvent;
		}
		ScenarioUpdateResult inputUpdateResult = UpdateInput();

		if (m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetVisible()) {
			if (m_ScenarioButtons.at(ScenarioButtons::ResumeButton)->GetVisible()) { m_ScenarioGUIController->GetManager()->SetFocus((m_BlinkTimer.AlternateReal(500)) ? m_ScenarioButtons.at(ScenarioButtons::ResumeButton) : nullptr); }
			if (m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetVisible()) { m_ScenarioButtons.at(ScenarioButtons::StartHereButton)->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <"); }
		} else if (m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->GetVisible()) {
			UpdatePlayersBox();
		}
		return inputUpdateResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::DrawSitePoints(BITMAP *drawBitmap) const {
		if (m_ScenarioScenes && m_ScenarioCollectionBoxes.at(ScenarioCollections::ActivitySelectBox)->GetVisible()) {
			for (const Scene *scenePointer : *m_ScenarioScenes) {
				int drawColor = (scenePointer->GetModuleID() == g_PresetMan.GetModuleID("Scenes.rte")) ? c_GUIColorRed : c_GUIColorYellow;

				Vector sitePos(m_PlanetCenter + scenePointer->GetLocation() + scenePointer->GetLocationOffset());
				int sitePosX = sitePos.GetFloorIntX();
				int sitePosY = sitePos.GetFloorIntY();

				int blendAmount = 70 + RandomNum(0, 40);
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				circlefill(drawBitmap, sitePosX, sitePosY, 4, drawColor);
				circlefill(drawBitmap, sitePosX, sitePosY, 2, drawColor);

				blendAmount = 145 + RandomNum(0, 110);
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				circlefill(drawBitmap, sitePosX, sitePosY, 1, drawColor);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::DrawLineToSitePoint(BITMAP *drawBitmap) const {
		int drawColor = c_GUIColorWhite;
		int blendAmount = 0;

		for (int index = 1; index < m_LineToSitePoints.size(); index++) {
			int lineStartX = m_LineToSitePoints.at(index - 1).GetFloorIntX();
			int lineStartY = m_LineToSitePoints.at(index - 1).GetFloorIntY();
			int lineEndX = m_LineToSitePoints.at(index).GetFloorIntX();
			int lineEndY = m_LineToSitePoints.at(index).GetFloorIntY();

			blendAmount = 195 + RandomNum(0, 30);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			line(drawBitmap, lineStartX, lineStartY, lineEndX, lineEndY, drawColor);

			blendAmount = 30 + RandomNum(0, 50);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			line(drawBitmap, lineStartX + 1, lineStartY, lineEndX + 1, lineEndY, drawColor);
			line(drawBitmap, lineStartX - 1, lineStartY, lineEndX - 1, lineEndY, drawColor);
			line(drawBitmap, lineStartX, lineStartY + 1, lineEndX, lineEndY + 1, drawColor);
			line(drawBitmap, lineStartX, lineStartY - 1, lineEndX, lineEndY - 1, drawColor);
		}

		// Draw a circle around the selected site point.
		Vector sitePos = m_PlanetCenter + m_SelectedScene->GetLocation() + m_SelectedScene->GetLocationOffset();
		int sitePosX = sitePos.GetFloorIntX();
		int sitePosY = sitePos.GetFloorIntY();
		int circleRadius = 8;

		blendAmount = 195 + RandomNum(0, 30);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
		circle(drawBitmap, sitePosX, sitePosY, circleRadius, drawColor);

		blendAmount = 120 + RandomNum(0, 50);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
		circle(drawBitmap, sitePosX, sitePosY, circleRadius - 1, drawColor);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ScenarioGUI::Draw(BITMAP *drawBitmap) const {
		drawing_mode(DRAW_MODE_TRANS, nullptr, 0, 0);
		DrawSitePoints(drawBitmap);
		if (m_SelectedScene && m_ScenarioCollectionBoxes.at(ScenarioCollections::SceneInfoBox)->GetVisible()) { DrawLineToSitePoint(drawBitmap); }
		drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);

		AllegroScreen drawScreen(drawBitmap);
		m_ScenarioGUIController->Draw(&drawScreen);

		if (m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox)->GetVisible()) {
			GUICollectionBox *playerSetupBox = m_ScenarioCollectionBoxes.at(ScenarioCollections::PlayerSetupBox);

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

			// Manually draw UI elements on top of colored rectangle.
			for (int teamIndex = Activity::MaxTeamCount - 1; teamIndex >= Activity::TeamOne; --teamIndex) {
				if (m_TeamTechSelect.at(teamIndex)->GetVisible()) {
					m_TeamTechSelect.at(teamIndex)->Draw(&drawScreen);
					if (m_TeamTechSelect.at(teamIndex)->IsDropped()) { m_TeamTechSelect.at(teamIndex)->GetListPanel()->Draw(&drawScreen); }
				}
				if (m_TeamAISkillSlider.at(teamIndex)->GetVisible()) {
					m_TeamAISkillSlider.at(teamIndex)->Draw(&drawScreen);
					m_TeamAISkillLabel.at(teamIndex)->Draw(&drawScreen);
				}
			}
		}
		m_ScenarioGUIController->DrawMouse();
	}
}