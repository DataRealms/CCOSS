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
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIComboBox.h"
#include "GUI/GUICheckbox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"

#include "Entity.h"
#include "DataModule.h"
#include "Scene.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScenarioGUI::ScenarioGUI(Controller *pController) :
	m_ScenarioController(pController) {
	RTEAssert(pController, "No controller sent to ScenarioGUI on creation!");
	m_ScenarioGUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
	m_ScenarioGUIInput = std::make_unique <AllegroInput>(-1, true);
	if (!m_ScenarioGUIController->Create(m_ScenarioGUIScreen.get(), m_ScenarioGUIInput.get(), "Base.rte/GUIs/Skins/MainMenu")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu");
	}
	m_ScenarioGUIController->Load("Base.rte/GUIs/ScenarioGUI.ini");

	// Make sure we have convenient points to the containing GUI collection boxes.
	m_ScenarioScreenBoxes[ROOTSCREEN] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("root"));
	m_ScenarioScreenBoxes[ACTIVITY] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ActivitySelectBox"));
	m_ScenarioScreenBoxes[SCENEINFO] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("SceneInfoBox"));
	m_ScenarioScreenBoxes[PLAYERSETUP] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("PlayerSetupBox"));
	
	m_ScenarioScreenBoxes[ROOTSCREEN]->SetPositionAbs(0, 0);
	m_ScenarioScreenBoxes[ROOTSCREEN]->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

	m_ScenarioButtons[BACKTOMAINBUTTON] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("BackToMainButton"));
	m_ScenarioButtons[RESUME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ButtonResume"));
	m_ScenarioButtons[STARTHERE] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneSelectButton"));
	m_ScenarioButtons[STARTGAME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("StartButton"));

	m_SitePointLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ScenePlanetLabel"));
	m_SitePointLabel->SetVisible(false);

	// Activity Selection Box.
	m_ActivitySelectComboBox = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("ActivitySelectCombo"));
	m_ActivityLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ActivityDescLabel"));
	m_DifficultyLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("DifficultyLabel"));
	m_DifficultySlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("DifficultySlider"));
	m_ActivitySelectComboBox->SetDropHeight(64);
	m_ActivityLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));

	// Scene Info Box.
	m_SceneCloseButton = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneCloseButton"));
	m_SceneNameLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneNameLabel"));
	m_SceneInfoLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneInfoLabel"));
	m_SceneInfoLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));

	// Player team assignment box.
	for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
		for (int teamIndex = Activity::TeamOne; teamIndex < TEAMROWCOUNT; ++teamIndex) {
			const std::string controlString = "P" + std::to_string(playerIndex + 1) + "T" + std::to_string(teamIndex + 1) + "Box";
			m_PlayerBoxes[playerIndex][teamIndex] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl(controlString));
		}
	}

	m_TeamBoxes[TEAM_DISABLED] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("TDIcon"));
	m_TeamBoxes[TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
	m_TeamNameLabels[TEAM_DISABLED] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("TDLabel"));

	std::string controlString = "";
	for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
		controlString = "T" + std::to_string(teamIndex + 1) + "Icon";
		m_TeamBoxes[teamIndex] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl(controlString));
		m_TeamBoxes[teamIndex]->SetDrawType(GUICollectionBox::Image);

		controlString = "T" + std::to_string(teamIndex + 1) + "Label";
		m_TeamNameLabels[teamIndex] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl(controlString));

		controlString = "T" + std::to_string(teamIndex + 1) + "TechCombo";
		m_TeamTechSelect[teamIndex] = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl(controlString));
		m_TeamTechSelect[teamIndex]->SetEnabled(false);
		m_TeamTechSelect[teamIndex]->SetVisible(false);
		m_TeamTechSelect[teamIndex]->GetListPanel()->AddItem("-All-", "", 0, 0, -2);
		m_TeamTechSelect[teamIndex]->GetListPanel()->AddItem("-Random-", "", 0, 0, -1);
		m_TeamTechSelect[teamIndex]->SetSelectedIndex(0);

		controlString = "T" + std::to_string(teamIndex + 1) + "AISkillSlider";
		m_TeamAISkillSlider[teamIndex] = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl(controlString));
		m_TeamAISkillSlider[teamIndex]->SetEnabled(false);
		m_TeamAISkillSlider[teamIndex]->SetVisible(false);
		m_TeamAISkillSlider[teamIndex]->SetValue(Activity::DefaultSkill);

		controlString = "T" + std::to_string(teamIndex + 1) + "AISkillLabel";
		m_TeamAISkillLabel[teamIndex] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl(controlString));
		m_TeamAISkillLabel[teamIndex]->SetEnabled(false);
		m_TeamAISkillLabel[teamIndex]->SetVisible(false);
		m_TeamAISkillLabel[teamIndex]->SetText(Activity::GetAISkillString(m_TeamAISkillSlider[teamIndex]->GetValue()));
	}

	m_StartErrorLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("StartErrorLabel"));
	m_CPULockLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("CPULockLabel"));

	// Populate the tech comboboxes with the available tech modules.
	const DataModule *dataModule = nullptr;
	const std::string techString = " Tech";
	std::string techName = "";
	for (int moduleIndex = 0; moduleIndex < g_PresetMan.GetTotalModuleCount(); ++moduleIndex) {
		dataModule = g_PresetMan.GetDataModule(moduleIndex);
		if (dataModule) {
			techName = dataModule->GetFriendlyName();
			const std::string::size_type techPos = techName.find(techString);
			if (techPos != string::npos) {
				techName.replace(techPos, techString.length(), "");
				for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
					m_TeamTechSelect[teamIndex]->GetListPanel()->AddItem(techName, "", 0, 0, moduleIndex);
				}
			}
		}
	}

	m_GoldLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("GoldLabel"));
	m_GoldSlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("GoldSlider"));
	m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("FogOfWarCheckbox"));
	m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
	m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("DeployUnitsCheckbox"));

	// Set up initial combobox locations and sizes.
	m_ScenarioButtons[BACKTOMAINBUTTON]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioButtons[BACKTOMAINBUTTON]->GetWidth() - 16, m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - m_ScenarioButtons[BACKTOMAINBUTTON]->GetHeight() - 22);
	m_ScenarioButtons[RESUME]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioButtons[RESUME]->GetWidth() - 16, m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - m_ScenarioButtons[RESUME]->GetHeight() - 47);
	m_ScenarioScreenBoxes[ACTIVITY]->SetPositionRel(16, 16);
	m_ScenarioScreenBoxes[SCENEINFO]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioScreenBoxes[SCENEINFO]->GetWidth() - 16, 16);
	m_ScenarioScreenBoxes[PLAYERSETUP]->CenterInParent(true, true);

	m_ScenePreviewBitmap->Create(Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT, 8);

	// Load default preview bitmap.
	m_DefaultPreviewBitmap->Create(Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT, 8);
	ContentFile defaultPreviewContent("Base.rte/GUIs/DefaultPreview.png");
	BITMAP *defaultPreview = defaultPreviewContent.LoadAndReleaseBitmap();
	blit(defaultPreview, m_DefaultPreviewBitmap->GetBitmap(), 0, 0, 0, 0, m_DefaultPreviewBitmap->GetWidth(), m_DefaultPreviewBitmap->GetHeight());
	destroy_bitmap(defaultPreview);
	clear_to_color(m_ScenePreviewBitmap->GetBitmap(), g_MaskColor);

	GetScenesAndActivities(true);

	UpdateActivityBox();

	HideAllScreens();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::SetEnabled() {
	HideAllScreens();
	m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
	// Reload all scenes and activities to reflect scene changes player might do in scene editor.
	GetScenesAndActivities(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::SetPlanetInfo(const Vector &center, float radius) {
	const bool centerChanged = (center != m_PlanetCenter);
	m_PlanetCenter = center;
	m_PlanetRadius = radius;
	if (centerChanged && m_ScenarioSelectedScene) {
		CalculateLinesToSitePoint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Update() {
	m_ScenarioController->Update();

	if (g_ConsoleMan.IsEnabled()) {
		return;
	}

	int mouseX = 0;
	int mouseY = 0;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);

	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible()) {
		// Handle the resume button.
		const Activity *currentActivity = g_ActivityMan.GetActivity();
		if (currentActivity && (currentActivity->GetActivityState() == Activity::Running || currentActivity->GetActivityState() == Activity::Editing)) {
			if (!m_ScenarioButtons[RESUME]->GetVisible()) {
				m_ScenarioButtons[RESUME]->SetVisible(true);
			}

			if (m_BlinkTimer.AlternateReal(500)) {
				m_ScenarioButtons[RESUME]->SetFocus();
			} else {
				m_ScenarioGUIController->GetManager()->SetFocus(0);
			}
		} else if (m_ScenarioButtons[RESUME]->GetVisible()) {
			m_ScenarioButtons[RESUME]->SetVisible(false);
		}

		// If the mouse is over a site point, then display the site's label.
		bool foundAnyHover = false;
		if (m_ScenarioScenes && !m_ScenarioDraggedBox && !m_ScenarioScreenBoxes[ACTIVITY]->PointInside(mouseX, mouseY) && !m_ScenarioScreenBoxes[SCENEINFO]->PointInside(mouseX, mouseY)) {
			Scene *candidateScene = nullptr;
			float shortestDist = 16.0F;
			Vector screenLocation(0, 0);
			const Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
			for (Scene *scenarioScene : *m_ScenarioScenes) {
				screenLocation = m_PlanetCenter + scenarioScene->GetLocation() + scenarioScene->GetLocationOffset();
				const float distance = (screenLocation - mousePos).GetMagnitude();

				if (distance < shortestDist) {
					shortestDist = distance;
					foundAnyHover = true;
					candidateScene = scenarioScene;
				}
			}

			// Set new hovered scene to be the one closest to the cursor, if there is any and if it is different than the currently hovered one.
			if (candidateScene != nullptr && candidateScene != m_ScenarioHoveredScene) {
				m_ScenarioHoveredScene = candidateScene;
				g_GUISound.SelectionChangeSound()->Play();
				SetSiteNameLabel(m_ScenarioHoveredScene->GetPresetName(), m_ScenarioHoveredScene->GetLocation() + m_ScenarioHoveredScene->GetLocationOffset());
				m_SitePointLabel->SetVisible(true);
			}
		}

		if (!foundAnyHover) {
			m_ScenarioHoveredScene = nullptr;
			m_SitePointLabel->SetVisible(false);
		}
	}

	if (m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
		m_ScenarioButtons[STARTHERE]->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <");
	} else if (m_ScenarioScreenBoxes[PLAYERSETUP]->GetVisible()) {
		UpdatePlayersBox();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawSitePoints(BITMAP *drawBitmap) const {
	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible() && m_ScenarioScenes) {
		drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

		for (const Scene *scenePointer : *m_ScenarioScenes) {
			int color = 0;

			// Mark user-created scenes to let players distinguish them from built-in.
			if (scenePointer->GetModuleID() == g_PresetMan.GetModuleID("Scenes.rte")) {
				color = c_GUIColorRed;
			} else {
				color = c_GUIColorYellow;
			}

			const Vector screenLocation(m_PlanetCenter + scenePointer->GetLocation() + scenePointer->GetLocationOffset());
			const int screenLocationX = screenLocation.GetFloorIntX();
			const int screenLocationY = screenLocation.GetFloorIntY();
			int blendAmount = 60 + RandomNum(0, 50);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 4, color);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 2, color);
			blendAmount = 145 + RandomNum(0, 110);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 1, color);
		}

		if (m_ScenarioSelectedScene && m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
			DrawLineToSitePoint(drawBitmap);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Draw(BITMAP *drawBitmap) const {
	DrawSitePoints(drawBitmap);

	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	AllegroScreen drawScreen(drawBitmap);

	m_ScenarioGUIController->Draw(&drawScreen);

	// Draw scene preview after GUI.
	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible() && m_ScenarioScenes && m_ScenarioSelectedScene && m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
		draw_sprite(drawBitmap, m_ScenePreviewBitmap->GetBitmap(), m_ScenarioScreenBoxes[SCENEINFO]->GetXPos() + 10, m_ScenarioScreenBoxes[SCENEINFO]->GetYPos() + 33);
	} else if (m_ScenarioScreenBoxes[PLAYERSETUP]->GetVisible()) {
		// Draw the Player-Team matrix lines and disabled overlay effects.
		int lineY = 80;
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			if (m_ScenarioSelectedActivity && (!m_ScenarioSelectedActivity->TeamActive(teamIndex) || m_LockedCPUTeam == teamIndex)) {
				// Apply a colored overlay on top of team rows that are not human-playable.
				drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
				const int blendAmount = 230;
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				rectfill(drawBitmap, m_ScenarioScreenBoxes[PLAYERSETUP]->GetXPos() + 110, m_ScenarioScreenBoxes[PLAYERSETUP]->GetYPos() + lineY, m_ScenarioScreenBoxes[PLAYERSETUP]->GetXPos() + m_ScenarioScreenBoxes[PLAYERSETUP]->GetWidth() - 12, m_ScenarioScreenBoxes[PLAYERSETUP]->GetYPos() + lineY + 25, c_GUIColorDarkBlue);
			}

			drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
			// Cell border separator lines.
			line(drawBitmap, m_ScenarioScreenBoxes[PLAYERSETUP]->GetXPos() + 110, m_ScenarioScreenBoxes[PLAYERSETUP]->GetYPos() + lineY, m_ScenarioScreenBoxes[PLAYERSETUP]->GetXPos() + m_ScenarioScreenBoxes[PLAYERSETUP]->GetWidth() - 12, m_ScenarioScreenBoxes[PLAYERSETUP]->GetYPos() + lineY, c_GUIColorLightBlue);
			lineY += 25;
		}

		// Manually draw UI elements on top of colored rectangle.
		for (int teamIndex = Activity::MaxTeamCount - 1; teamIndex >= Activity::TeamOne; teamIndex--) {
			if (m_TeamTechSelect[teamIndex]->GetVisible()) {
				m_TeamTechSelect[teamIndex]->Draw(&drawScreen);
				if (m_TeamTechSelect[teamIndex]->IsDropped()) {
					m_TeamTechSelect[teamIndex]->GetListPanel()->Draw(&drawScreen);
				}
			}

			if (m_TeamAISkillSlider[teamIndex]->GetVisible()) {
				m_TeamAISkillSlider[teamIndex]->Draw(&drawScreen);
				m_TeamAISkillLabel[teamIndex]->Draw(&drawScreen);
			}
		}
	}

	m_ScenarioGUIController->DrawMouse();

	const int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();
	const Icon *deviceIcon = nullptr;

	// Draw the active joystick's sprite next to the mouse.
	if (device >= DEVICE_GAMEPAD_1) {
		int mouseX = 0;
		int mouseY = 0;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		deviceIcon = g_UInputMan.GetDeviceIcon(device);
		if (deviceIcon) {
			draw_sprite(drawBitmap, deviceIcon->GetBitmaps8()[0], mouseX + 16, mouseY - 4);
		}
	}

	// Show which joysticks are detected by the game.
	for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; playerIndex++) {
		if (g_UInputMan.JoystickActive(playerIndex)) {
			const int matchedDevice = DEVICE_GAMEPAD_1 + playerIndex;
			if (matchedDevice != device) {
				deviceIcon = g_UInputMan.GetDeviceIcon(matchedDevice);
				if (deviceIcon) {
					draw_sprite(drawBitmap, deviceIcon->GetBitmaps8()[0], g_FrameMan.GetResX() - 30 * g_UInputMan.GetJoystickCount() + 30 * playerIndex, g_FrameMan.GetResY() - 25);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScenarioGUI::ScenarioUpdateResult ScenarioGUI::UpdateInput() {
	// TODO: if activity is running, allow esc to resume activity instead of quitting.
	if (g_UInputMan.KeyPressed(KEY_ESC)) {
		g_GUISound.BackButtonPressSound()->Play();
		if (m_ScenarioScreenBoxes[PLAYERSETUP]->GetVisible()) {
			HideAllScreens();
			m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
			ShowScenesBox();
		} else {
			return ScenarioUpdateResult::BACKTOMAIN;
		}
	}

	int mouseX = 0;
	int mouseY = 0;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
	const Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

	// Handle GUI panel dragging.
	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible()) {
		if (g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER) && m_ScenarioDraggedBox) {
			m_ScenarioDraggedBox->MoveRelative(mousePos.GetFloorIntX() - m_PrevMousePos.GetFloorIntX(), mousePos.GetFloorIntY() - m_PrevMousePos.GetFloorIntY());
			m_PrevMousePos = mousePos;

			KeepBoxOnScreen(m_ScenarioDraggedBox);
			if (m_ScenarioDraggedBox == m_ScenarioScreenBoxes[SCENEINFO]) {
				CalculateLinesToSitePoint();
			}
		} else {
			m_ScenarioDraggedBox = nullptr;
		}

		if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER)) {
			if (m_ScenarioHoveredScene) {
				SetSelectedScene(m_ScenarioHoveredScene);
				g_GUISound.ItemChangeSound()->Play();
			}

			GUICollectionBox *hoveredBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioScreenBoxes[ROOTSCREEN], 1));
			const GUIControl *hoveredControl = m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioScreenBoxes[ROOTSCREEN], -1);
			const bool nonDragControl = (dynamic_cast<const GUIButton *>(hoveredControl) || dynamic_cast<const GUISlider *>(hoveredControl) || dynamic_cast<const GUIComboBox *>(hoveredControl));
			if (hoveredBox && !nonDragControl && !m_ScenarioDraggedBox && !m_ActivitySelectComboBox->IsDropped()) {
				m_ScenarioDraggedBox = hoveredBox;
				m_PrevMousePos = mousePos;
			}
		}
	}

	m_ScenarioGUIController->Update();

	///////////////////////////////////////
	// Handle events

	GUIEvent anEvent;
	while (m_ScenarioGUIController->GetEvent(&anEvent)) {
		const std::string eventControlName = anEvent.GetControl()->GetName();
		const GUIControl *eventControl = anEvent.GetControl();

		if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible()) {
			if (anEvent.GetType() == GUIEvent::Command) {
				if (eventControlName == "BackToMainButton") {
					HideAllScreens();
					g_GUISound.BackButtonPressSound()->Play();
					return ScenarioUpdateResult::BACKTOMAIN;
				} else if (eventControlName == "ButtonResume") {
					g_GUISound.BackButtonPressSound()->Play();
					return ScenarioUpdateResult::ACTIVITYRESUMED;
				}

				if (m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
					if (eventControl == m_ScenarioButtons[STARTHERE]) {
						HideAllScreens();
						ShowPlayersBox();
						g_GUISound.ButtonPressSound()->Play();
					} else if (eventControl == m_SceneCloseButton) {
						UnselectScene();
						g_GUISound.ButtonPressSound()->Play();
					}
				}

			} else if (anEvent.GetType() == GUIEvent::Notification) {
				if (eventControl == m_ActivitySelectComboBox && anEvent.GetMsg() == GUIComboBox::Closed) {
					const Activity *activity = dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity);
					if (activity != m_ScenarioSelectedActivity) {
						m_ScenarioSelectedActivity = activity;
						// The activity selection has changed.
						UpdateActivityBox();
						if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
							SetSelectedScene(m_ScenarioScenes->front());
						} else {
							// Deselect any previously selected scene. it may not be compatible with the new activity.
							UnselectScene();
						}

						g_GUISound.ItemChangeSound()->Play();
					} 
				} else if (eventControl == m_DifficultySlider) {
					UpdateActivityBox();
				}
			}

		} else if (m_ScenarioScreenBoxes[PLAYERSETUP]->GetVisible()) {
			if (anEvent.GetType() == GUIEvent::Command) {
				if (eventControlName == "PlayerCancelButton") {
					HideAllScreens();
					m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
					ShowScenesBox();
					g_GUISound.BackButtonPressSound()->Play();
				} else if (eventControl == m_ScenarioButtons[STARTGAME]) {
					if (StartGame()) {
						HideAllScreens();
						m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
						g_GUISound.ButtonPressSound()->Play();
						return ScenarioUpdateResult::ACTIVITYRESTARTED;
					} else {
						g_GUISound.UserErrorSound()->Play();
					}
				}
			}
		}

		if (dynamic_cast<const GUIButton *>(eventControl) && anEvent.GetType() == GUIEvent::Notification && anEvent.GetMsg() == GUIButton::Focused) {
			g_GUISound.SelectionChangeSound()->Play();
		}
	}

	return ScenarioUpdateResult::NOEVENT;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::HideAllScreens() {
	for (int screenIndex = 1; screenIndex < SCREENCOUNT; screenIndex++) {
		m_ScenarioScreenBoxes[screenIndex]->SetVisible(false);
	}

	m_SitePointLabel->SetVisible(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::KeepBoxOnScreen(GUICollectionBox *screenBox) const {
	if (screenBox->GetXPos() < 0) {
		screenBox->SetPositionAbs(0, screenBox->GetYPos());
	} else if (screenBox->GetXPos() > m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - screenBox->GetWidth()) {
		screenBox->SetPositionAbs(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - screenBox->GetWidth(), screenBox->GetYPos());
	}

	if (screenBox->GetYPos() < 0) {
		screenBox->SetPositionAbs(screenBox->GetXPos(), 0);
	} else if (screenBox->GetYPos() > m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - screenBox->GetHeight()) {
		screenBox->SetPositionAbs(screenBox->GetXPos(), m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - screenBox->GetHeight());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateActivityBox() {
	if (m_ScenarioSelectedActivity) {
		// Pull out the list of Scenes that are compatible with this Activity.
		m_ScenarioScenes = nullptr;
		for (std::pair<Activity * const, list<Scene *>> &activityScenePair : m_Activities) {
			if (activityScenePair.first == m_ScenarioSelectedActivity) {
				m_ScenarioScenes = &(activityScenePair.second);
				break;
			}
		}

		// Set the description.
		if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
			m_ActivityLabel->SetText(m_ScenarioSelectedActivity->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
		} else if (m_ScenarioScenes && m_ScenarioScenes->size() > 1) {
			m_ActivityLabel->SetText(m_ScenarioSelectedActivity->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
		} else {
			m_ActivityLabel->SetText(m_ScenarioSelectedActivity->GetDescription() + "\n\nNo sites appear to be compatible with this selected activity! Please try another.");
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

		const int textHeight = m_ActivityLabel->ResizeHeightToFit();
		const int padding = 110;
		m_ScenarioScreenBoxes[ACTIVITY]->Resize(m_ScenarioScreenBoxes[ACTIVITY]->GetWidth(), textHeight + padding);

		const GameActivity *selectedGA = dynamic_cast<const GameActivity *>(m_ScenarioSelectedActivity);
		if (selectedGA) {
			// Set gold slider value if activity specifies default gold amounts for difficulties.
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

			// Set default checkbox states and enable or disable them.
			const int defaultFogOfWar = selectedGA->GetDefaultFogOfWar();
			if (defaultFogOfWar > -1) {
				m_FogOfWarCheckbox->SetCheck(defaultFogOfWar != 0);
			}
			m_FogOfWarCheckbox->SetEnabled(selectedGA->GetFogOfWarSwitchEnabled());

			const int defaultReqClearPath = selectedGA->GetDefaultRequireClearPathToOrbit();
			if (defaultReqClearPath > -1) {
				m_RequireClearPathToOrbitCheckbox->SetCheck(defaultReqClearPath != 0);
			}
			m_RequireClearPathToOrbitCheckbox->SetEnabled(selectedGA->GetRequireClearPathToOrbitSwitchEnabled());

			const int defaultDeployUnits = selectedGA->GetDefaultDeployUnits();
			if (defaultDeployUnits > -1) {
				m_DeployUnitsCheckbox->SetCheck(defaultDeployUnits != 0);
			}
			m_DeployUnitsCheckbox->SetEnabled(selectedGA->GetDeployUnitsSwitchEnabled());
		}
	} else {
		m_ScenarioScenes = nullptr;
		m_ActivityLabel->SetText("No Activity selected.");
		m_DifficultyLabel->SetVisible(false);
		if (m_DifficultySlider) {
			m_DifficultySlider->SetVisible(false);
		}

		const int textHeight = m_ActivityLabel->ResizeHeightToFit();
		const int padding = 125;
		m_ScenarioScreenBoxes[ACTIVITY]->Resize(m_ScenarioScreenBoxes[ACTIVITY]->GetWidth(), textHeight + padding);
	}

	KeepBoxOnScreen(m_ScenarioScreenBoxes[ACTIVITY]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::ShowScenesBox() {
	clear_to_color(m_ScenePreviewBitmap->GetBitmap(), g_MaskColor);
	if (m_ScenarioSelectedScene) {
		m_SceneNameLabel->SetText(m_ScenarioSelectedScene->GetPresetName());
		m_SceneInfoLabel->SetText(m_ScenarioSelectedScene->GetDescription());

		BITMAP *preview = m_ScenarioSelectedScene->GetPreviewBitmap();
		if (!preview) {
			preview = m_DefaultPreviewBitmap->GetBitmap();
		}
		blit(preview, m_ScenePreviewBitmap->GetBitmap(), 0, 0, 0, 0, m_ScenePreviewBitmap->GetBitmap()->w, m_ScenePreviewBitmap->GetBitmap()->h);
	}

	const int textHeight = m_SceneInfoLabel->ResizeHeightToFit();
	const int padding = 140;
	m_ScenarioScreenBoxes[SCENEINFO]->Resize(m_ScenarioScreenBoxes[SCENEINFO]->GetWidth(), textHeight + padding);
	KeepBoxOnScreen(m_ScenarioScreenBoxes[SCENEINFO]);
	m_ScenarioScreenBoxes[SCENEINFO]->SetVisible(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::ShowPlayersBox() {
	m_ScenarioScreenBoxes[PLAYERSETUP]->SetVisible(true);

	if (m_ScenarioSelectedActivity && m_ScenarioSelectedScene) {
		int mouseX = 0;
		int mouseY = 0;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_ScenarioSelectedActivity);

		if (gameActivity) {
			m_LockedCPUTeam = gameActivity->GetCPUTeam();
			if (m_LockedCPUTeam != Activity::NoTeam) {
				m_CPULockLabel->SetPositionAbs(m_CPULockLabel->GetXPos(), m_TeamNameLabels[m_LockedCPUTeam]->GetYPos());
			}
		}

		m_ScenarioButtons[STARTGAME]->SetVisible(false);
		m_StartErrorLabel->SetVisible(true);
		m_CPULockLabel->SetVisible(m_LockedCPUTeam != Activity::NoTeam);

		// Set up initial color for all cells.
		for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
			for (int teamIndex = Activity::TeamOne; teamIndex < TEAMROWCOUNT; ++teamIndex) {
				m_PlayerBoxes[playerIndex][teamIndex]->SetDrawType(GUICollectionBox::Color);
				m_PlayerBoxes[playerIndex][teamIndex]->SetDrawColor(c_GUIColorBlue);
			}
		}

		const Icon *iconPointer = nullptr;

		// Human players start on the disabled team row.
		for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
			m_PlayerBoxes[playerIndex][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
			iconPointer = g_UInputMan.GetSchemeIcon(playerIndex);
			if (iconPointer) {
				m_PlayerBoxes[playerIndex][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
			}
		}

		// CPU player either has a locked team or starts on the disabled team row.
		const int InitialCPUTeam = (m_LockedCPUTeam != Activity::NoTeam) ? m_LockedCPUTeam : TEAM_DISABLED;
		m_PlayerBoxes[PLAYER_CPU][InitialCPUTeam]->SetDrawType(GUICollectionBox::Image);
		iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (iconPointer) {
			m_PlayerBoxes[PLAYER_CPU][InitialCPUTeam]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
		}

		iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
		m_TeamNameLabels[TEAM_DISABLED]->SetText("Not Playing:");
		if (iconPointer) {
			m_TeamBoxes[TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
		}

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			// Reset GUI controls to default values.
			m_TeamTechSelect[teamIndex]->SetSelectedIndex(0);
			m_TeamAISkillSlider[teamIndex]->SetValue(Activity::DefaultSkill);

			if (m_ScenarioSelectedActivity->TeamActive(teamIndex)) {
				iconPointer = m_ScenarioSelectedActivity->GetTeamIcon(teamIndex);
				if (!iconPointer) {
					const std::string teamString = "Team " + std::to_string(teamIndex + 1) + " Default";
					iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", teamString));
				}

				m_TeamNameLabels[teamIndex]->SetText(m_ScenarioSelectedActivity->GetTeamName(teamIndex) + ":");
				m_TeamTechSelect[teamIndex]->SetEnabled(true);
				m_TeamTechSelect[teamIndex]->SetVisible(true);
				m_TeamAISkillSlider[teamIndex]->SetEnabled(true);
				m_TeamAISkillSlider[teamIndex]->SetVisible(true);
				m_TeamAISkillLabel[teamIndex]->SetVisible(true);
			} else {
				iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
				m_TeamNameLabels[teamIndex]->SetText("Unavailable");
				m_TeamTechSelect[teamIndex]->SetEnabled(false);
				m_TeamTechSelect[teamIndex]->SetVisible(false);
				m_TeamAISkillSlider[teamIndex]->SetEnabled(false);
				m_TeamAISkillSlider[teamIndex]->SetVisible(false);
				m_TeamAISkillLabel[teamIndex]->SetVisible(false);
			}

			if (iconPointer) {
				m_TeamBoxes[teamIndex]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdatePlayersBox() {
	if (m_ScenarioSelectedActivity && m_ScenarioSelectedScene) {
		int mouseX = 0;
		int mouseY = 0;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(m_ScenarioSelectedActivity);

		const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioScreenBoxes[PLAYERSETUP], 1));
		if (hoveredCell) {
			// Find which cell is being hovered over.
			int hoveredPlayer = PLAYERCOLUMNCOUNT;
			int hoveredTeam = TEAMROWCOUNT;
			for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
				for (int teamIndex = Activity::TeamOne; teamIndex < TEAMROWCOUNT; ++teamIndex) {
					if (m_PlayerBoxes[playerIndex][teamIndex] == hoveredCell) {
						hoveredPlayer = playerIndex;
						hoveredTeam = teamIndex;
					} else if (m_PlayerBoxes[playerIndex][teamIndex]->GetDrawType() == GUICollectionBox::Color) {
						// Un-highlight all other cells.
						m_PlayerBoxes[playerIndex][teamIndex]->SetDrawColor(c_GUIColorBlue);
					}
				}
			}

			// Make the hovered cell light up and able to be selected if:
			// It's under an active team row or the disabled team row.
			// It's not a team row locked to the CPU.
			// It's not the CPU player if he is locked to a CPU team.
			// It doesn't already contain an image.
			if ((m_ScenarioSelectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TEAM_DISABLED) && m_LockedCPUTeam != hoveredTeam
				&& (m_LockedCPUTeam == Activity::NoTeam || hoveredPlayer != PLAYER_CPU) && m_PlayerBoxes[hoveredPlayer][hoveredTeam]->GetDrawType() != GUICollectionBox::Image) {
				if (g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER)) {
					// Move the player's icon to the correct row.
					m_PlayerBoxes[hoveredPlayer][hoveredTeam]->SetDrawType(GUICollectionBox::Image);
					const Icon *playerIcon = (hoveredPlayer != PLAYER_CPU) ? g_UInputMan.GetSchemeIcon(hoveredPlayer) : dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
					if (playerIcon) {
						m_PlayerBoxes[hoveredPlayer][hoveredTeam]->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0]));
					}

					// Clear all unhovered rows of this column.
					for (int nonHoveredTeam = Activity::TeamOne; nonHoveredTeam < TEAMROWCOUNT; ++nonHoveredTeam) {
						if (nonHoveredTeam != hoveredTeam) {
							m_PlayerBoxes[hoveredPlayer][nonHoveredTeam]->SetDrawType(GUICollectionBox::Color);
							m_PlayerBoxes[hoveredPlayer][nonHoveredTeam]->SetDrawColor(c_GUIColorBlue);
						}
					}

					// If CPU changed team, remove human players from the team.
					if (hoveredPlayer == PLAYER_CPU && hoveredTeam != TEAM_DISABLED) {
						for (int humanPlayer = Players::PlayerOne; humanPlayer < Players::MaxPlayerCount; ++humanPlayer) {
							if (m_PlayerBoxes[humanPlayer][hoveredTeam]->GetDrawType() == GUICollectionBox::Image) {
								m_PlayerBoxes[humanPlayer][hoveredTeam]->SetDrawType(GUICollectionBox::Color);
								m_PlayerBoxes[humanPlayer][hoveredTeam]->SetDrawColor(c_GUIColorBlue);
								m_PlayerBoxes[humanPlayer][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
								playerIcon = g_UInputMan.GetSchemeIcon(humanPlayer);
								if (playerIcon) {
									m_PlayerBoxes[humanPlayer][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0]));
								}
							}
						}
					}

					// If a human player changed team, remove the CPU from the team.
					else if (hoveredPlayer != PLAYER_CPU && hoveredTeam != TEAM_DISABLED && m_PlayerBoxes[PLAYER_CPU][hoveredTeam]->GetDrawType() == GUICollectionBox::Image) {
						m_PlayerBoxes[PLAYER_CPU][hoveredTeam]->SetDrawType(GUICollectionBox::Color);
						m_PlayerBoxes[PLAYER_CPU][hoveredTeam]->SetDrawColor(c_GUIColorBlue);
						m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
						playerIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
						if (playerIcon) {
							m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(playerIcon->GetBitmaps32()[0]));
						}
					}

					g_GUISound.FocusChangeSound()->Play();
				} else if (m_PlayerBoxes[hoveredPlayer][hoveredTeam]->GetDrawType() == GUICollectionBox::Color &&
					m_PlayerBoxes[hoveredPlayer][hoveredTeam]->GetDrawColor() != c_GUIColorLightBlue) {
					// Just highlight the cell.
					m_PlayerBoxes[hoveredPlayer][hoveredTeam]->SetDrawColor(c_GUIColorLightBlue);
					g_GUISound.SelectionChangeSound()->Play();
				}
			}
		}

		// Count players in the teams.
		int teamsWithPlayers = 0;
		bool teamWithHumans = false;
		int humansInTeams = 0;
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			bool foundPlayer = false;
			if (m_ScenarioSelectedActivity->TeamActive(teamIndex)) {
				for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
					if (m_PlayerBoxes[playerIndex][teamIndex]->GetDrawType() == GUICollectionBox::Image) {
						foundPlayer = true;
						if (playerIndex != PLAYER_CPU) {
							++humansInTeams;
							teamWithHumans = true;
						}
					}
				}

				if (foundPlayer) {
					teamsWithPlayers++;
				}
			}
		}

		if (gameActivity) {
			const int maxHumanPlayers = gameActivity->GetMaxPlayerSupport();
			const int minTeamsRequired = gameActivity->GetMinTeamsRequired();
			if (humansInTeams > maxHumanPlayers) {
				m_ScenarioButtons[STARTGAME]->SetVisible(false);
				const std::string msgString = "Too many players assigned! Max for this activity is " + std::to_string(maxHumanPlayers);
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (minTeamsRequired > teamsWithPlayers) {
				m_ScenarioButtons[STARTGAME]->SetVisible(false);
				const std::string msgString = "Assign players to at\nleast " + std::to_string(minTeamsRequired) + " of the teams!";
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (teamWithHumans == 0) {
				m_ScenarioButtons[STARTGAME]->SetVisible(false);
				m_StartErrorLabel->SetText("Assign human players\nto at least one team!");
				m_StartErrorLabel->SetVisible(true);
			} else {
				m_ScenarioButtons[STARTGAME]->SetVisible(true);
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
			m_TeamAISkillLabel[teamIndex]->SetText(Activity::GetAISkillString(m_TeamAISkillSlider[teamIndex]->GetValue()));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScenarioGUI::StartGame() {
	if (!m_ScenarioSelectedActivity || !m_ScenarioSelectedScene) {
		return false;
	}

	Activity *activityInstance = dynamic_cast<Activity *>(m_ScenarioSelectedActivity->Clone());
	GameActivity *gameActivity = dynamic_cast<GameActivity *>(activityInstance);

	// Set up the basic settings.
	if (gameActivity) {
		gameActivity->SetDifficulty(m_DifficultySlider->GetValue());
		gameActivity->SetFogOfWarEnabled(m_FogOfWarCheckbox->GetCheck());
		gameActivity->SetRequireClearPathToOrbit(m_RequireClearPathToOrbitCheckbox->GetCheck());

		// If gold slider is at its max value then the amount is 'infinite' and we must set some ridiculously high value.
		if (m_GoldSlider->GetValue() == m_GoldSlider->GetMaximum()) {
			gameActivity->SetStartingGold(1000000);
		} else {
			int startGold = m_GoldSlider->GetValue();
			startGold = startGold - startGold % 500;
			gameActivity->SetStartingGold(startGold);
		}
	}

	g_SceneMan.SetSceneToLoad(m_ScenarioSelectedScene, true, m_DeployUnitsCheckbox->GetCheck());

	// Set up the player and team assignments.
	activityInstance->ClearPlayers(false);
	for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			if (m_PlayerBoxes[playerIndex][teamIndex]->GetDrawType() == GUICollectionBox::Image) {
				activityInstance->AddPlayer(playerIndex, true, teamIndex, 0);
				break;
			}
		}
	}

	if (gameActivity) {
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			if (m_PlayerBoxes[PLAYER_CPU][teamIndex]->GetDrawType() == GUICollectionBox::Image) {
				gameActivity->SetCPUTeam(teamIndex);
				break;
			}
		}
	}

	for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
		// Handle player techs.
		const GUIListPanel::Item *techItem = m_TeamTechSelect[teamIndex]->GetSelectedItem();
		if (techItem) {
			if (techItem->m_ExtraIndex == -2) {
				// Selected "All".
				gameActivity->SetTeamTech(teamIndex, "-All-");
			} else if (techItem->m_ExtraIndex == -1) {
				// Selected "Random".
				const int selection = RandomNum<int>(2, m_TeamTechSelect[teamIndex]->GetListPanel()->GetItemList()->size() - 1);
				m_TeamTechSelect[teamIndex]->SetSelectedIndex(selection);
				gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(m_TeamTechSelect[teamIndex]->GetSelectedItem()->m_ExtraIndex));
			} else {
				gameActivity->SetTeamTech(teamIndex, g_PresetMan.GetDataModuleName(techItem->m_ExtraIndex));
			}
		}

		if (m_TeamAISkillSlider[teamIndex]->IsEnabled()) {
			gameActivity->SetTeamAISkill(teamIndex, m_TeamAISkillSlider[teamIndex]->GetValue());
		} else {
			gameActivity->SetTeamAISkill(teamIndex, Activity::DefaultSkill);
		}
	}

	g_LuaMan.FileCloseAll();

	g_ActivityMan.SetStartActivity(activityInstance);

	//TODO: Why is there metagame handling here?
	if (g_MetaMan.GameInProgress()) {
		g_MetaMan.EndGame();
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::GetScenesAndActivities(bool selectTutorial) {
	// Redo the list of Activities.
	m_Activities.clear();
	m_ScenarioScenes = nullptr;

	// Get the list of all read in Scene presets.
	list<Entity *> presetList;
	g_PresetMan.GetAllOfType(presetList, "Scene");
	list<Scene *> filteredScenes;

	for (Entity *presetEntity : presetList) {
		Scene *presetScene = dynamic_cast<Scene *>(presetEntity);
		// Only add non-editor and non-special scenes, or ones that don't have locations defined, or are metascenes.
		if (presetScene && !presetScene->GetLocation().IsZero() && !presetScene->IsMetagameInternal() && (presetScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes())) {
			filteredScenes.push_back(presetScene);
		}
	}

	for (Scene *filteredScene : filteredScenes) {
		filteredScene->SetLocationOffset(Vector(0, 0));
	}

	// We need to calculate planet center manually because m_PlanetCenter reflects coords of moving planet which is outside the screen when this is called first time.
	const Vector planetCenter = m_PlanetCenter.IsZero() ? Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2) : m_PlanetCenter;

	//If a scene is on the planet but outside the screen then move it into the screen.
	for (Scene *filteredScene : filteredScenes) {
		const float sceneYPos = (planetCenter + filteredScene->GetLocation() + filteredScene->GetLocationOffset()).GetY();
		if (std::abs(filteredScene->GetLocation().GetY()) < m_PlanetRadius + 100 && std::abs(filteredScene->GetLocation().GetX()) < m_PlanetRadius + 100) {
			if (sceneYPos < 10) {
				filteredScene->SetLocationOffset(filteredScene->GetLocationOffset() + Vector(0, 10 - sceneYPos));
			}

			if (sceneYPos > g_FrameMan.GetResY() - 10) {
				filteredScene->SetLocationOffset(filteredScene->GetLocationOffset() + Vector(0, g_FrameMan.GetResY() - 10 - sceneYPos));
			}
		}
	}

	// If site points are overlapping then move one of them towards the planet center.
	const float requiredDistance = 8.0F;
	bool foundOverlap = true;
	while (foundOverlap) {
		foundOverlap = false;
		for (Scene *filteredScene1 : filteredScenes) {
			for (const Scene *filteredScene2 : filteredScenes) {
				if (filteredScene1 != filteredScene2) {
					const Vector pos1 = filteredScene1->GetLocation() + filteredScene1->GetLocationOffset();
					const Vector pos2 = filteredScene2->GetLocation() + filteredScene2->GetLocationOffset();
					const Vector overlap = pos1 - pos2;
					const float overlapMagnitude = overlap.GetMagnitude();

					if (overlapMagnitude < requiredDistance) {
						foundOverlap = true;
						const float overlapY = overlap.GetY();
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

	// Get the list of all read-in Activity presets.
	presetList.clear();
	g_PresetMan.GetAllOfType(presetList, "Activity");
	Activity *presetActivity = nullptr;

	const int previousSelectedActivityIndex = m_ActivitySelectComboBox->GetSelectedIndex();

	// Associate all Scenes compatible with each Activity.
	// Populate the activities selection dropdown.
	m_ActivitySelectComboBox->ClearList();
	int index = 0;
	int tutorialIndex = -1;
	for (Entity *presetEntity : presetList) {
		presetActivity = dynamic_cast<Activity *>(presetEntity);
		if (presetActivity && presetActivity->GetClassName().find("Editor") == string::npos) {
			pair<Activity *, list<Scene *> > newPair(presetActivity, list<Scene *>());
			for (Scene *filteredScene : filteredScenes) {
				if (presetActivity->SceneIsCompatible(filteredScene)) {
					newPair.second.push_back(filteredScene);
				}
			}

			m_Activities.insert(newPair);
			// Add to the activity selection combobox, and attach the activity pointer, not passing in ownership.
			m_ActivitySelectComboBox->AddItem(presetActivity->GetPresetName(), "", 0, presetActivity);

			// Save the tutorial mission so we can select it by default.
			if (selectTutorial && presetActivity->GetClassName() == "GATutorial") {
				tutorialIndex = index;
			}
			index++;
		}
	}

	if (selectTutorial && tutorialIndex >= 0) {
		m_ActivitySelectComboBox->SetSelectedIndex(tutorialIndex);
	} else {
		m_ActivitySelectComboBox->SetSelectedIndex(previousSelectedActivityIndex);
	}
	m_ScenarioSelectedActivity = dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity);

	UpdateActivityBox();
	if (m_ScenarioScenes && (m_ScenarioScenes->size() == 1 || selectTutorial)) {
		SetSelectedScene(m_ScenarioScenes->front());
	} else {
		UnselectScene();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::SetSiteNameLabel(const string &text, const Vector &location) {
	m_SitePointLabel->SetText(text);
	Vector AbsolutePosition = m_PlanetCenter + location - Vector(m_SitePointLabel->GetWidth() / 2, 0.0F) - Vector(0.0F, m_SitePointLabel->GetHeight() * 1.5F);
	const float padding = 6.0F;
	AbsolutePosition.m_X = std::clamp(AbsolutePosition.m_X, padding, g_FrameMan.GetResX() - m_SitePointLabel->GetWidth() - padding);
	AbsolutePosition.m_Y = std::clamp(AbsolutePosition.m_Y,padding, g_FrameMan.GetResY() - m_SitePointLabel->GetHeight() - padding);
	m_SitePointLabel->SetPositionAbs(AbsolutePosition.GetFloorIntX(), AbsolutePosition.GetFloorIntY());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const {
	int blendAmount = 195 + RandomNum(0, 30);
	const int startX = start.GetFloorIntX();
	const int startY = start.GetFloorIntY();
	const int endX = end.GetFloorIntX();
	const int endY = end.GetFloorIntY();
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	line(drawBitmap, startX, startY, endX, endY, color);

	blendAmount = 20 + RandomNum(0, 50);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	line(drawBitmap, startX + 1, startY, endX + 1, endY, color);
	line(drawBitmap, startX - 1, startY, endX - 1, endY, color);
	line(drawBitmap, startX, startY + 1, endX, endY + 1, color);
	line(drawBitmap, startX, startY - 1, endX, endY - 1, color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawLineToSitePoint(BITMAP *drawBitmap) const {
	const int color = c_GUIColorWhite;
	
	for (int index = 1; index < m_LinePointsToSite.size(); index++) {
		DrawGlowLine(drawBitmap, m_LinePointsToSite[index - 1], m_LinePointsToSite[index], color);
	}

	// Draw a circle around the selected site point.
	const Vector sitePos = m_PlanetCenter + m_ScenarioSelectedScene->GetLocation() + m_ScenarioSelectedScene->GetLocationOffset();
	const int sitePosX = sitePos.GetFloorIntX();
	const int sitePosY = sitePos.GetFloorIntY();
	const int circleRadius = 8;
	const int blendAmount = 225 + RandomNum(-20, 20);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	circle(drawBitmap, sitePosX, sitePosY, circleRadius, color);
	circle(drawBitmap, sitePosX, sitePosY, circleRadius - 1, color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::CalculateLinesToSitePoint() {
	m_LinePointsToSite.clear();

	const Vector sitePos = m_PlanetCenter + m_ScenarioSelectedScene->GetLocation() + m_ScenarioSelectedScene->GetLocationOffset(); // Target site point.
	if (m_ScenarioScreenBoxes[SCENEINFO]->PointInside(sitePos.GetFloorIntX(), sitePos.GetFloorIntY())) {
		return;
	}

	const int halfBoxHeight = m_ScenarioScreenBoxes[SCENEINFO]->GetHeight() / 2;
	const Vector sceneBoxCenter(m_ScenarioScreenBoxes[SCENEINFO]->GetXPos() + (m_ScenarioScreenBoxes[SCENEINFO]->GetWidth() / 2), m_ScenarioScreenBoxes[SCENEINFO]->GetYPos() + halfBoxHeight);
	const float yDirMult = sitePos.m_Y < sceneBoxCenter.m_Y ? -1.0F : 1.0F;
	const Vector sceneBoxEdge = sceneBoxCenter + Vector(0, halfBoxHeight) * yDirMult; // Point on the scene box where the line starts.
	const int circleRadius = 8; // Radius of the circle drawn around the site point.
	const int minStraightLength = 15; // Minimum length of straight line at the box edge and site point edge.
	const int minSiteDistance = circleRadius + minStraightLength; // Minimum distance from a chamfer point to the site point.

	if (std::fabs(sceneBoxCenter.GetFloorIntX() - sitePos.GetFloorIntX()) < minSiteDistance) {
		// No bends, meaning the line goes straight up/down to the site circle.
		m_LinePointsToSite.emplace_back(sceneBoxEdge + Vector(sitePos.m_X - sceneBoxEdge.m_X, 0));
		m_LinePointsToSite.emplace_back(sitePos + Vector(0, (circleRadius + 1) * (-yDirMult)));
		return;
	}

	m_LinePointsToSite.emplace_back(sceneBoxEdge); // Point at the scene info box edge.
	
	const int minChamferSize = 15; // Minimum x and y lengths of the chamfer.
	const int maxChamferSize = 40; // Maximum x and y lengths of the chamfer.
	int chamferSize = maxChamferSize;
	const float xDirMult = sitePos.m_X < sceneBoxEdge.m_X ? -1.0F : 1.0F;

	if (std::fabs(sitePos.GetFloorIntY() - sceneBoxCenter.GetFloorIntY()) > halfBoxHeight + minStraightLength) {
		// One bend.
		const Vector bendPoint(sceneBoxEdge.m_X, sitePos.m_Y); // At this point the line bends. If the bend is chamfered then the two chamfer points will be equally distanced from this point.

		chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(sceneBoxEdge.GetFloorIntY() - sitePos.GetFloorIntY()) - minStraightLength);
		chamferSize = std::min(chamferSize, maxChamferSize);
		if (chamferSize < minChamferSize) {
			chamferSize = 0;
		}

		m_LinePointsToSite.emplace_back(Vector(bendPoint.m_X, bendPoint.m_Y + chamferSize * -yDirMult));
		if (chamferSize > 0) {
			m_LinePointsToSite.emplace_back(Vector(bendPoint.m_X + chamferSize * xDirMult, bendPoint.m_Y));
		}
		m_LinePointsToSite.emplace_back(sitePos + Vector((circleRadius + 1) * -xDirMult, 0));
	} else {
		// Two bends.
		// extraLength ensures that there will be straight lines coming out of the site and the box, and that they are nearly as short as possible.
		const int extraLength = std::clamp(minSiteDistance + (sitePos.GetFloorIntY() - sceneBoxEdge.GetFloorIntY()) * static_cast<int>(yDirMult), 0, minSiteDistance);
		
		const Vector firstBend(sceneBoxEdge.m_X, sceneBoxEdge.m_Y + (extraLength + minStraightLength) * yDirMult);
		const Vector secondBend(sitePos.m_X, firstBend.m_Y);

		chamferSize = std::min(std::abs(sceneBoxEdge.GetFloorIntX() - sitePos.GetFloorIntX()) - minSiteDistance, std::abs(secondBend.GetFloorIntY() - sitePos.GetFloorIntY()) - minSiteDistance);
		chamferSize = std::min(chamferSize, maxChamferSize);
		if (chamferSize < minChamferSize) {
			chamferSize = 0;
		}

		m_LinePointsToSite.emplace_back(firstBend);
		m_LinePointsToSite.emplace_back(Vector(secondBend.m_X + chamferSize * -xDirMult, secondBend.m_Y));
		if (chamferSize > 0) {
			m_LinePointsToSite.emplace_back(Vector(secondBend.m_X, secondBend.m_Y + chamferSize * -yDirMult));
		}
		m_LinePointsToSite.emplace_back(sitePos + Vector(0, (circleRadius + 1) * yDirMult));
	}
}

void ScenarioGUI::HideScenesBox() const {
	m_ScenarioScreenBoxes[SCENEINFO]->SetVisible(false);
}

void ScenarioGUI::SetSelectedScene(Scene *newSelectedScene) {
	m_ScenarioSelectedScene = newSelectedScene;
	ShowScenesBox();
	CalculateLinesToSitePoint();
}

void ScenarioGUI::UnselectScene() {
	m_ScenarioSelectedScene = nullptr;
	HideScenesBox();
	m_LinePointsToSite.clear();
}