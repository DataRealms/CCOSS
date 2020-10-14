//////////////////////////////////////////////////////////////////////////////////////////
// File:            ScenarioGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the ScenarioGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ScenarioGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "MetaMan.h"
#include "AudioMan.h"
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
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"

#include "Controller.h"
#include "Entity.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "GABrainMatch.h"
#include "GABaseDefense.h"
#include "GATutorial.h"
#include "GAScripted.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "DataModule.h"
#include "Scene.h"

extern volatile bool g_Quit;

using namespace RTE;

constexpr int CHAMFERSIZE = 40;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Clear() {
	m_ScenarioController = nullptr;
	m_ScenarioGUIScreen = nullptr;
	m_ScenarioGUIInput = nullptr;
	m_ScenarioGUIController = nullptr;
	m_BlinkTimer.Reset();

	m_PlanetCenter.Reset();
	m_PlanetRadius = 240.0F;

	for (GUICollectionBox *&screenIndex : m_ScenarioScreenBoxes) {
		screenIndex = nullptr;
	}

	for (GUIButton *&buttonIndex : m_ScenarioButtons) {
		buttonIndex = nullptr;
	}
	m_ScenarioScenePlanetLabel = nullptr;

	m_ActivitySelectComboBox = nullptr;
	m_ActivityLabel = nullptr;
	m_DifficultyLabel = nullptr;
	m_DifficultySlider = nullptr;

	m_SceneCloseButton = nullptr;
	m_SceneNameLabel = nullptr;
	m_SceneInfoLabel = nullptr;
	for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
		for (int teamIndex = Activity::TeamOne; teamIndex < TEAMROWCOUNT; ++teamIndex) {
			m_PlayerBoxes[playerIndex][teamIndex] = nullptr;
		}
	}

	for (int teamIndex = Activity::TeamOne; teamIndex < TEAMROWCOUNT; ++teamIndex) {
		m_TeamBoxes[teamIndex] = nullptr;
		m_TeamNameLabels[teamIndex] = nullptr;
	}

	for (int teamIndex = Activity::TeamOne; teamIndex < RTE::Activity::MaxTeamCount; ++teamIndex) {
		m_TeamTechSelect[teamIndex] = nullptr;
		m_TeamAISkillSlider[teamIndex] = nullptr;
		m_TeamAISkillLabel[teamIndex] = nullptr;
	}

	m_StartErrorLabel = nullptr;
	m_CPULockLabel = nullptr;
	m_LockedCPUTeam = Activity::NoTeam;

	m_GoldLabel = nullptr;
	m_GoldSlider = nullptr;
	m_FogOfWarCheckbox = nullptr;
	m_RequireClearPathToOrbitCheckbox = nullptr;
	m_DeployUnitsCheckbox = nullptr;

	m_QuitConfirmLabel = nullptr;
	m_QuitConfirmButton = nullptr;

	m_ScenarioScenes = nullptr;
	m_Activities.clear();
	m_ScenarioDraggedBox = nullptr;
	m_DragEngaged = false;
	m_ScenarioHoveredScene = nullptr;
	m_ScenarioSelectedScene = nullptr;
	m_PrevMousePos.Reset();
	m_LinePointsToSite.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ScenarioGUI::Create(Controller *pController) {
	RTEAssert(pController, "No controller sent to ScenarioGUI on creation!");
	m_ScenarioController = pController;

	if (!m_ScenarioGUIScreen) {
		m_ScenarioGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());
	}
	if (!m_ScenarioGUIInput) {
		m_ScenarioGUIInput = new AllegroInput(-1, true);
	}
	if (!m_ScenarioGUIController) {
		m_ScenarioGUIController = new GUIControlManager();
	}
	if (!m_ScenarioGUIController->Create(m_ScenarioGUIScreen, m_ScenarioGUIInput, "Base.rte/GUIs/Skins/MainMenu")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu");
	}
	m_ScenarioGUIController->Load("Base.rte/GUIs/ScenarioGUI.ini");

	// Make sure we have convenient points to the containing GUI collection boxes.
	m_ScenarioScreenBoxes[ROOTSCREEN] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("root"));
	m_ScenarioScreenBoxes[ACTIVITY] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ActivitySelectBox"));
	m_ScenarioScreenBoxes[SCENEINFO] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("SceneInfoBox"));
	m_ScenarioScreenBoxes[PLAYERSETUPSCREEN] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("PlayerSetupBox"));
	m_ScenarioScreenBoxes[QUITCONFIRM] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ConfirmDialog"));

	m_ScenarioScreenBoxes[ROOTSCREEN]->SetPositionAbs(0, 0);
	m_ScenarioScreenBoxes[ROOTSCREEN]->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

	m_ScenarioButtons[BACKTOMAINBUTTON] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("BackToMainButton"));
	m_ScenarioButtons[RESUME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ButtonResume"));
	m_ScenarioButtons[STARTHERE] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneSelectButton"));
	m_ScenarioButtons[STARTGAME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("StartButton"));

	m_ScenarioScenePlanetLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ScenePlanetLabel"));
	m_ScenarioScenePlanetLabel->SetVisible(false);

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
	m_TeamNameLabels[TEAM_DISABLED] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("TDLabel"));

	std::string controlString = "";
	for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
		controlString = "T" + std::to_string(teamIndex + 1) + "Icon";
		m_TeamBoxes[teamIndex] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl(controlString));

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

	// Make the lists be scrolled to the top when they are initially dropped.
	for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
		m_TeamTechSelect[teamIndex]->GetListPanel()->ScrollToTop();
	}

	m_GoldLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("GoldLabel"));
	m_GoldSlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("GoldSlider"));
	m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("FogOfWarCheckbox"));
	m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
	m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("DeployUnitsCheckbox"));

	m_QuitConfirmLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ConfirmLabel"));
	m_QuitConfirmButton = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ConfirmButton"));

	// Set up initial combobox locations and sizes.
	m_ScenarioButtons[BACKTOMAINBUTTON]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioButtons[BACKTOMAINBUTTON]->GetWidth() - 16, m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - m_ScenarioButtons[BACKTOMAINBUTTON]->GetHeight() - 22);
	m_ScenarioButtons[RESUME]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioButtons[RESUME]->GetWidth() - 16, m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - m_ScenarioButtons[RESUME]->GetHeight() - 47);
	m_ScenarioScreenBoxes[ACTIVITY]->SetPositionRel(16, 16);
	m_ScenarioScreenBoxes[SCENEINFO]->SetPositionRel(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - m_ScenarioScreenBoxes[SCENEINFO]->GetWidth() - 16, 16);
	m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->CenterInParent(true, true);
	m_ScenarioScreenBoxes[QUITCONFIRM]->CenterInParent(true, true);

	m_ScenePreviewBitmap = create_bitmap_ex(8, Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT);

	// Load default preview bitmap.
	ContentFile defaultPreview("Base.rte/GUIs/DefaultPreview.png");
	m_DefaultPreviewBitmap = defaultPreview.LoadAndReleaseBitmap();

	clear_to_color(m_ScenePreviewBitmap, g_MaskColor);

	GetAllScenesAndActivities(true);

	UpdateActivityBox();

	HideAllScreens();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Destroy() {
	delete m_ScenarioGUIController;
	delete m_ScenarioGUIInput;
	delete m_ScenarioGUIScreen;

	destroy_bitmap(m_ScenePreviewBitmap);
	destroy_bitmap(m_DefaultPreviewBitmap);

	Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControlManager *ScenarioGUI::GetGUIControlManager() {
	return m_ScenarioGUIController;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::SetEnabled() {
	HideAllScreens();
	m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
	// Reload all scenes and activities to reflect scene changes player might do in scene editor.
	GetAllScenesAndActivities(false);
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

ScenarioGUI::ScenarioUpdateResult ScenarioGUI::Update() {
	ScenarioUpdateResult result = ScenarioUpdateResult::NOEVENT;
	m_ScenarioController->Update();

	if (g_ConsoleMan.IsEnabled()) {
		return result;
	}

	int mouseX;
	int mouseY;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
	Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

	result = UpdateInput();

	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible()) {
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
		} else {
			if (m_ScenarioButtons[RESUME]->GetVisible()) {
				m_ScenarioButtons[RESUME]->SetVisible(false);
			}
		}

		// If the mouse is over the planet, check if it's over a site point, then display the site's label.
		if (m_ScenarioScenes && !m_ScenarioDraggedBox && (mousePos - m_PlanetCenter).GetMagnitude() < m_PlanetRadius && !(m_ScenarioScreenBoxes[ACTIVITY]->PointInside(mouseX, mouseY) || m_ScenarioScreenBoxes[SCENEINFO]->PointInside(mouseX, mouseY))) {
			bool foundAnyHover = false;
			Scene *candidateScene = nullptr;
			float shortestDist = 16.0F;
			Vector screenLocation(0, 0);
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
				UpdateSiteNameLabel(m_ScenarioHoveredScene->GetPresetName(), m_ScenarioHoveredScene->GetLocation() + m_ScenarioHoveredScene->GetLocationOffset());
				m_ScenarioScenePlanetLabel->SetVisible(true);
			} else if (!foundAnyHover) {
				m_ScenarioHoveredScene = nullptr;
				m_ScenarioScenePlanetLabel->SetVisible(false);
			}
		}
	}

	if (m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
		m_ScenarioButtons[STARTHERE]->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <");
	}
	
	if (m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetVisible()) {
		UpdatePlayersBox();
	}

	// Save mouse pos for next frame so we can do dragging.
	if (m_DragEngaged) {
		m_PrevMousePos = mousePos;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Draw(BITMAP *drawBitmap) const {
	// Transparency effect on the scene dots and lines.
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	// Screen blend the dots and lines, with some flickering in its intensity.
	int blendAmount = 65 + RandomNum(0, 110);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

	// Draw sites etc only when selecting them.
	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible() && m_ScenarioScenes) {
		// Draw the scene location dots.
		Vector screenLocation;
		for (const Scene *scenePointer : *m_ScenarioScenes) {
			int color;

			// Mark user-created scenes to let players easily distinguish them from built-in.
			if (scenePointer->GetModuleID() == g_PresetMan.GetModuleID("Scenes.rte")) {
				color = c_GUIColorGreen;
			} else {
				color = c_GUIColorYellow;
			}

			screenLocation = m_PlanetCenter + scenePointer->GetLocation() + scenePointer->GetLocationOffset();
			const int screenLocationX = screenLocation.GetFloorIntX();
			const int screenLocationY = screenLocation.GetFloorIntY();
			blendAmount = 60 + RandomNum(0, 50);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 4, color);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 2, color);
			blendAmount = 145 + RandomNum(0, 110);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocationX, screenLocationY, 1, color);
		}

		// Draw the lines etc pointing at the selected Scene from the Scene Info box.
		if (m_ScenarioSelectedScene && m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
			DrawWhiteScreenLineToSitePoint(drawBitmap, m_ScenarioSelectedScene->GetLocation() + m_ScenarioSelectedScene->GetLocationOffset());
		}
	}

	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	AllegroScreen drawScreen(drawBitmap);

	m_ScenarioGUIController->Draw(&drawScreen);

	// Draw scene preview after GUI.
	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible() && m_ScenarioScenes && m_ScenarioSelectedScene && m_ScenarioScreenBoxes[SCENEINFO]->GetVisible()) {
		BITMAP *preview = m_ScenarioSelectedScene->GetPreviewBitmap();
		if (!preview) {
			preview = m_DefaultPreviewBitmap;
		}

		blit(preview, m_ScenePreviewBitmap, 0, 0, 0, 0, m_ScenePreviewBitmap->w, m_ScenePreviewBitmap->h);
		draw_sprite(drawBitmap, m_ScenePreviewBitmap, m_ScenarioScreenBoxes[SCENEINFO]->GetXPos() + 10, m_ScenarioScreenBoxes[SCENEINFO]->GetYPos() + 33);
	} else if (m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetVisible()) {
		// Draw the Player-Team matrix lines and disabled overlay effects.
		const Activity *selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr;
		int lineY = 80;
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			// Disabled shaded boxes.
			if (selectedActivity && (!selectedActivity->TeamActive(teamIndex) || m_LockedCPUTeam == teamIndex)) {
				// TODO: understand why the blending isnt working as desired
				//Transparency effect on the overlay boxes.
				drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
				// Screen blend the dots and lines, with some flickering in its intensity.
				int blendAmountInner = 230;
				set_screen_blender(blendAmountInner, blendAmountInner, blendAmountInner, blendAmountInner);
				rectfill(drawBitmap, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetXPos() + 110, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetYPos() + lineY, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetXPos() + m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetWidth() - 12, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetYPos() + lineY + 25, c_GUIColorDarkBlue);
			}

			drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
			// Cell border separator lines.
			line(drawBitmap, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetXPos() + 110, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetYPos() + lineY, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetXPos() + m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetWidth() - 12, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->GetYPos() + lineY, c_GUIColorLightBlue);
			lineY += 25;
		}

		// Manually draw UI elements on top of colored rectangle
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

	int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();

	if (device >= DEVICE_GAMEPAD_1) {
		int mouseX;
		int mouseY;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);

		const Icon *pIcon = g_UInputMan.GetDeviceIcon(device);
		if (pIcon) {
			draw_sprite(drawBitmap, pIcon->GetBitmaps8()[0], mouseX + 16, mouseY - 4);
		}
	}

	// Show which joysticks are detected by the game.
	for (int joystickIndex = Players::PlayerOne; joystickIndex < Players::MaxPlayerCount; joystickIndex++) {
		if (g_UInputMan.JoystickActive(joystickIndex)) {
			int matchedDevice = DEVICE_GAMEPAD_1 + joystickIndex;

			if (matchedDevice != device) {
				const Icon *pIcon = g_UInputMan.GetDeviceIcon(matchedDevice);
				if (pIcon) {
					draw_sprite(drawBitmap, pIcon->GetBitmaps8()[0], g_FrameMan.GetResX() - 30 * g_UInputMan.GetJoystickCount() + 30 * joystickIndex, g_FrameMan.GetResY() - 25);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScenarioGUI::ScenarioUpdateResult ScenarioGUI::UpdateInput() {
	ScenarioUpdateResult result = NOEVENT;
	// TODO: if activity is running, allow esc to resume activity instead of quitting.
	if (g_UInputMan.KeyPressed(KEY_ESC)) {
		if (m_ScenarioScreenBoxes[QUITCONFIRM]->GetVisible()) {
			g_Quit = true;
		} else {
			HideAllScreens();
			m_QuitConfirmLabel->SetText("Are you sure you want to quit to OS?"); //\nAny unsaved progress\nwill be lost!");
			m_QuitConfirmButton->SetText("Quit");
			m_ScenarioScreenBoxes[QUITCONFIRM]->SetVisible(true);
		}
	}

	///////////////////////////////////////////////////////////
	// Mouse handling

	int mouseX;
	int mouseY;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
	Vector mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));

	if (m_ScenarioScreenBoxes[ACTIVITY]->GetVisible()) {
		if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER) && m_ScenarioHoveredScene) {
			m_ScenarioSelectedScene = m_ScenarioHoveredScene;
			ShowScenesBox();
			CalculateLinesToSitePoint();
			g_GUISound.ItemChangeSound()->Play();
		}

		bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
		// If not currently dragging a box, see if we should start.
		if (!m_ScenarioDraggedBox && menuButtonHeld && !m_DragEngaged && !m_ActivitySelectComboBox->IsDropped()) {
			GUICollectionBox *hoveredBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioScreenBoxes[ROOTSCREEN], 1));

			if (hoveredBox == m_ScenarioScreenBoxes[ACTIVITY] || hoveredBox == m_ScenarioScreenBoxes[SCENEINFO]) {
				m_ScenarioDraggedBox = hoveredBox;
				// Save the mouse pos at the start of the drag so we can measure if we should engage.
				m_PrevMousePos = mousePos;
			}
		} else if (!menuButtonHeld) {
			m_ScenarioDraggedBox = nullptr;
			m_DragEngaged = false;
		}
	}

	// Figure out dragging of the dialog boxes, if one is being dragged.
	// Only start drag if we're over a threshold, to prevent small unintentional nudges.
	if (m_ScenarioDraggedBox && !m_DragEngaged && (mousePos - m_PrevMousePos).GetLargest() > 4) {
		m_DragEngaged = true;
	}

	// Actually drag if we now are engaged.
	if (m_ScenarioDraggedBox && m_DragEngaged) {
		m_ScenarioDraggedBox->MoveRelative(mousePos.GetFloorIntX() - m_PrevMousePos.GetFloorIntX(), mousePos.GetFloorIntY() - m_PrevMousePos.GetFloorIntY());
		// Ensure the drag didn't shove it off-screen.
		KeepBoxOnScreen(m_ScenarioDraggedBox);
		if (m_ScenarioDraggedBox == m_ScenarioScreenBoxes[SCENEINFO]) {
			CalculateLinesToSitePoint();
		}
	}

	//////////////////////////////////////////
	// Update the ControlManager

	m_ScenarioGUIController->Update();

	///////////////////////////////////////
	// Handle events

	GUIEvent anEvent;
	while (m_ScenarioGUIController->GetEvent(&anEvent)) {
		const std::string eventControlName = anEvent.GetControl()->GetName();
		GUIControl *eventControl = anEvent.GetControl();

		if (anEvent.GetType() == GUIEvent::Command) {
			if (eventControlName == "BackToMainButton") {
				HideAllScreens();
				g_GUISound.BackButtonPressSound()->Play();
				result = ScenarioUpdateResult::BACKTOMAIN;
			} else if (eventControlName == "ConfirmButton") {
				// Quit program button pressed.
				HideAllScreens();
				g_Quit = true;
				g_GUISound.BackButtonPressSound()->Play();
			} else if (eventControlName == "ButtonResume") {
				g_GUISound.BackButtonPressSound()->Play();
				result = ScenarioUpdateResult::ACTIVITYRESUMED;
			} else if (eventControlName == "PlayerCancelButton") {
				// Cancel button on the player config box.
				HideAllScreens();
				m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
				ShowScenesBox();
				g_GUISound.BackButtonPressSound()->Play();
			} else if (eventControlName == "ConfirmCancelButton") {
				// Cancel button on the quit confirm dialog box.
				HideAllScreens();
				m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
				g_GUISound.BackButtonPressSound()->Play();
			} else if (eventControl == m_ScenarioButtons[STARTHERE]) {
				HideAllScreens();
				ShowPlayersBox();
				g_GUISound.ButtonPressSound()->Play();
			} else if (eventControl == m_ScenarioButtons[STARTGAME]) {
				if (StartGame()) {
					HideAllScreens();
					m_ScenarioScreenBoxes[ACTIVITY]->SetVisible(true);
					g_GUISound.ButtonPressSound()->Play();
					result = ScenarioUpdateResult::ACTIVITYRESTARTED;
				} else {
					g_GUISound.UserErrorSound()->Play();
				}
			} else if (eventControl == m_SceneCloseButton) {
				m_ScenarioSelectedScene = nullptr;
				HideScenesBox();
				m_LinePointsToSite.clear();
				g_GUISound.ButtonPressSound()->Play();
			}
		} else if (anEvent.GetType() == GUIEvent::Notification) {
			if (dynamic_cast<GUIButton *>(eventControl)) {
				if (anEvent.GetMsg() == GUIButton::Focused) {
					g_GUISound.SelectionChangeSound()->Play();
				}
				// Also stop dragging any panels if we're over any button.
				m_ScenarioDraggedBox = nullptr;
			} else if (eventControl == m_DifficultySlider) {
				UpdateActivityBox();
				// Also stop dragging any panels if we're over any button.
				m_ScenarioDraggedBox = nullptr;
			} else if (eventControl == m_ActivitySelectComboBox) {
				// Also stop dragging any panels if we're over the selection list.
				m_ScenarioDraggedBox = nullptr;

				// The activity selection changed.
				if (anEvent.GetMsg() == GUIComboBox::Closed) {
					UpdateActivityBox();

					// If there is only one Scene compatible with this newly selected Activity, then automatically select it.
					if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
						m_ScenarioSelectedScene = m_ScenarioScenes->front();
						ShowScenesBox();
						CalculateLinesToSitePoint();
					} else {
						// Deselect any previously selected scene. it may not be compatible with the new activity.
						m_ScenarioSelectedScene = nullptr;
						HideScenesBox();
						m_LinePointsToSite.clear();
					}

					g_GUISound.ItemChangeSound()->Play();
				}
			}
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::HideAllScreens() {
	for (int screenIndex = 1; screenIndex < SCREENCOUNT; screenIndex++) {
		m_ScenarioScreenBoxes[screenIndex]->SetVisible(false);
	}

	m_ScenarioScenePlanetLabel->SetVisible(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::KeepBoxOnScreen(GUICollectionBox *pBox, int margin) {
	// Make sure the box doesn't go entirely outside of the screen.
	if (pBox->GetXPos() < (margin - pBox->GetWidth())) {
		pBox->SetPositionAbs(margin - pBox->GetWidth(), pBox->GetYPos());
	}
	if (pBox->GetXPos() > m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - margin) {
		pBox->SetPositionAbs(m_ScenarioScreenBoxes[ROOTSCREEN]->GetWidth() - margin, pBox->GetYPos());
	}
	if (pBox->GetYPos() < (margin - pBox->GetHeight())) {
		pBox->SetPositionAbs(pBox->GetXPos(), margin - pBox->GetHeight());
	}
	if (pBox->GetYPos() > m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - margin) {
		pBox->SetPositionAbs(pBox->GetXPos(), m_ScenarioScreenBoxes[ROOTSCREEN]->GetHeight() - margin);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateActivityBox() {
	// Get the currently selected Activity.
	const Activity *selectedActivity = nullptr;
	if (m_ActivitySelectComboBox) {
		selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr;
	}

	if (selectedActivity) {
		// Pull out the list of Scenes that are compatible with this Activity.
		map<Activity *, list<Scene *> >::iterator asItr;
		if (m_Activities.end() != (asItr = m_Activities.find(const_cast<Activity *>(selectedActivity)))) {
			m_ScenarioScenes = &((*asItr).second);
		} else {
			m_ScenarioScenes = nullptr;
		}

		// Set the description.
		if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
		} else if (m_ScenarioScenes && m_ScenarioScenes->size() > 1) {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
		} else {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nNo sites appear to be compatible with this selected activity! Please try another.");
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

		// Resize the box to fit the desc.
		const int textHeight = m_ActivityLabel->ResizeHeightToFit();
		const int padding = 110;
		m_ScenarioScreenBoxes[ACTIVITY]->Resize(m_ScenarioScreenBoxes[ACTIVITY]->GetWidth(), textHeight + padding);

		const GameActivity *selectedGA = dynamic_cast<const GameActivity *>(selectedActivity);
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

			//Set default fog of war flag and enable or disable it if necessary.
			const int defFogOfWar = selectedGA->GetDefaultFogOfWar();
			if (defFogOfWar > -1) {
				m_FogOfWarCheckbox->SetCheck(defFogOfWar != 0);
			}
			m_FogOfWarCheckbox->SetEnabled(selectedGA->GetFogOfWarSwitchEnabled());

			//Set default clear path to orbit flag and enable or disable it if necessary.
			const int defReqClearPath = selectedGA->GetDefaultRequireClearPathToOrbit();
			if (defReqClearPath > -1) {
				m_RequireClearPathToOrbitCheckbox->SetCheck(defReqClearPath != 0);
			}
			m_RequireClearPathToOrbitCheckbox->SetEnabled(selectedGA->GetRequireClearPathToOrbitSwitchEnabled());

			//Set default deploy units flag and enable or disable it if necessary.
			const int defDeployUnits = selectedGA->GetDefaultDeployUnits();
			if (defDeployUnits > -1) {
				m_DeployUnitsCheckbox->SetCheck(defDeployUnits != 0);
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
		// Resize the box to fit the desc.
		const int textHeight = m_ActivityLabel->ResizeHeightToFit();
		const int padding = 125;
		m_ScenarioScreenBoxes[ACTIVITY]->Resize(m_ScenarioScreenBoxes[ACTIVITY]->GetWidth(), textHeight + padding);
	}
	// Make sure the box doesn't go entirely outside of the screen.
	KeepBoxOnScreen(m_ScenarioScreenBoxes[ACTIVITY]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::ShowScenesBox() {
	clear_to_color(m_ScenePreviewBitmap, g_MaskColor);
	if (m_ScenarioSelectedScene) {
		m_SceneNameLabel->SetText(m_ScenarioSelectedScene->GetPresetName());
		m_SceneInfoLabel->SetText(m_ScenarioSelectedScene->GetDescription());
	}
	const int textHeight = m_SceneInfoLabel->ResizeHeightToFit();
	const int padding = 140;
	m_ScenarioScreenBoxes[SCENEINFO]->Resize(m_ScenarioScreenBoxes[SCENEINFO]->GetWidth(), textHeight + padding);
	KeepBoxOnScreen(m_ScenarioScreenBoxes[SCENEINFO]);
	m_ScenarioScreenBoxes[SCENEINFO]->SetVisible(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::ShowPlayersBox() {
	m_ScenarioScreenBoxes[PLAYERSETUPSCREEN]->SetVisible(true);
	const Activity *selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr;

	if (selectedActivity && m_ScenarioSelectedScene) {
		int mouseX;
		int mouseY;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(selectedActivity);

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

		const Icon *iconPointer;

		// Human players start on the disabled team row.
		for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; ++playerIndex) {
			m_PlayerBoxes[playerIndex][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
			iconPointer = g_UInputMan.GetSchemeIcon(playerIndex);
			if (iconPointer) {
				m_PlayerBoxes[playerIndex][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
			}
		}

		// CPU player either has a locked team or starts on the disabled team row.
		int InitialCPUTeam = (m_LockedCPUTeam != Activity::NoTeam) ? m_LockedCPUTeam : TEAM_DISABLED;
		m_PlayerBoxes[PLAYER_CPU][InitialCPUTeam]->SetDrawType(GUICollectionBox::Image);
		iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
		if (iconPointer) {
			m_PlayerBoxes[PLAYER_CPU][InitialCPUTeam]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
		}

		iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
		m_TeamNameLabels[TEAM_DISABLED]->SetText("Not Playing:");
		if (iconPointer) {
			m_TeamBoxes[TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
			m_TeamBoxes[TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(iconPointer->GetBitmaps32()[0]));
		}

		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			// Reset controls to default values.
			m_TeamTechSelect[teamIndex]->SetSelectedIndex(0);
			m_TeamAISkillSlider[teamIndex]->SetValue(Activity::DefaultSkill);

			m_TeamBoxes[teamIndex]->SetDrawType(GUICollectionBox::Image);
			if (selectedActivity->TeamActive(teamIndex)) {
				iconPointer = selectedActivity->GetTeamIcon(teamIndex);
				// Revert to default if needed.
				if (!iconPointer) {
					const std::string teamString = "Team " + std::to_string(teamIndex + 1) + " Default";
					iconPointer = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", teamString));
				}

				m_TeamNameLabels[teamIndex]->SetText(selectedActivity->GetTeamName(teamIndex) + ":");

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
	const Activity *selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr;

	if (selectedActivity && m_ScenarioSelectedScene) {
		int mouseX;
		int mouseY;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		const GameActivity *gameActivity = dynamic_cast<const GameActivity *>(selectedActivity);

		const GUICollectionBox *hoveredCell = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioScreenBoxes[PLAYERSETUPSCREEN], 1));
		if (hoveredCell) {
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
			if ((selectedActivity->TeamActive(hoveredTeam) || hoveredTeam == TEAM_DISABLED) && m_LockedCPUTeam != hoveredTeam
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
		int teamsWithHumans = 0;
		int humansInTeams = 0;
		for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
			if (selectedActivity->TeamActive(teamIndex)) {
				for (int playerIndex = Players::PlayerOne; playerIndex < PLAYERCOLUMNCOUNT; ++playerIndex) {
					if (m_PlayerBoxes[playerIndex][teamIndex]->GetDrawType() == GUICollectionBox::Image) {
						++teamsWithPlayers;
						if (playerIndex != PLAYER_CPU) {
							++teamsWithHumans;
							++humansInTeams;
						}
						break;
					}
				}
			}
		}

		if (gameActivity) {
			int maxPlayers = gameActivity->GetMaxPlayerSupport();
			int minTeamsRequired = gameActivity->GetMinTeamsRequired();
			if (humansInTeams > maxPlayers) {
				m_ScenarioButtons[STARTGAME]->SetVisible(false);
				const std::string msgString = "Too many players assigned! Max for this activity is " + std::to_string(maxPlayers);
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (minTeamsRequired > teamsWithPlayers) {
				m_ScenarioButtons[STARTGAME]->SetVisible(false);
				const std::string msgString = "Assign players to at\nleast " + std::to_string(minTeamsRequired) + " of the teams!";
				m_StartErrorLabel->SetText(msgString);
				m_StartErrorLabel->SetVisible(true);
			} else if (teamsWithHumans == 0) {
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
	const Activity *activityPreset = nullptr;
	if (m_ActivitySelectComboBox) {
		activityPreset = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : nullptr;
	}

	if (!activityPreset || !m_ScenarioSelectedScene) {
		return false;
	}

	Activity *activityInstance = dynamic_cast<Activity *>(activityPreset->Clone());
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

	const GUIListPanel::Item *techItem;
	for (int teamIndex = Activity::TeamOne; teamIndex < Activity::MaxTeamCount; ++teamIndex) {
		techItem = m_TeamTechSelect[teamIndex]->GetSelectedItem();
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

	if (g_MetaMan.GameInProgress()) {
		g_MetaMan.EndGame();
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::GetAllScenesAndActivities(bool selectTutorial) {
	// Redo the list of Activities.
	m_Activities.clear();
	m_ScenarioScenes = nullptr;

	// Get the list of all read in Scene presets.
	list<Entity *> presetList;
	g_PresetMan.GetAllOfType(presetList, "Scene");
	list<Scene *> filteredScenes;

	// Go through the list and cast all the pointers to scenes so we have a handy list.
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
	Vector planetCenter = m_PlanetCenter.IsZero() ? Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2) : m_PlanetCenter;

	//Move out-of-screen scenes closer to the middle of the planet if we have planet info.
	for (Scene *filteredScene : filteredScenes) {
		float y = planetCenter.GetY() + filteredScene->GetLocation().GetY();

		// Do not touch scenes outside the planet, they might be hidden intentionally.
		if (abs(filteredScene->GetLocation().GetY()) < m_PlanetRadius + 100 && abs(filteredScene->GetLocation().GetX()) < m_PlanetRadius + 100) {
			if (y < 10) {
				filteredScene->SetLocationOffset(Vector(0, -y + 14));
			}

			if (y > g_FrameMan.GetResY() - 10) {
				filteredScene->SetLocationOffset(Vector(0, -(y - g_FrameMan.GetResY() + 14)));
			}
		}
	}

	// Add offsets to reveal overlapping scenes if any.
	for (Scene *filteredScene1 : filteredScenes) {
		bool isOverlapped = false;

		do {
			isOverlapped = false;

			// Find overlapping scene dot.
			for (const Scene *filteredScene2 : filteredScenes) {
				if (filteredScene1 != filteredScene2) {
					Vector pos1 = filteredScene1->GetLocation() + filteredScene1->GetLocationOffset();
					Vector pos2 = filteredScene2->GetLocation() + filteredScene2->GetLocationOffset();

					if ((pos1 - pos2).GetMagnitude() < 8) {
						isOverlapped = true;
						break;
					}
				}
			}

			// Move the dot closer to the planet center.
			Vector offsetIncrement;
			if (filteredScene1->GetLocation().GetY() > 0) {
				offsetIncrement = Vector(0, -8);
			} else {
				offsetIncrement = Vector(0, 8);
			}

			if (isOverlapped) {
				if (abs(filteredScene1->GetLocation().GetY()) > m_PlanetRadius) {
					offsetIncrement.m_Y = -offsetIncrement.m_Y * 2;
				}

				filteredScene1->SetLocationOffset(filteredScene1->GetLocationOffset() + offsetIncrement);

			}
		} while (isOverlapped);
	}

	// Get the list of all read-in Activity presets.
	presetList.clear();
	g_PresetMan.GetAllOfType(presetList, "Activity");
	Activity *presetActivity = nullptr;

	int selectedActivityIndex = m_ActivitySelectComboBox->GetSelectedIndex();

	// Go through the list and cast all the pointers to Activities so we have a handy list.
	// Associate all Scenes compatible with each Activity.
	// Populate the activities selection dropdown while we're at it.
	m_ActivitySelectComboBox->ClearList();
	int index = 0;
	int tutorialIndex = -1;
	for (Entity *presetEntity : presetList) {
		presetActivity = dynamic_cast<Activity *>(presetEntity);
		// Only add non-editor and non-special activities.
		if (presetActivity/* && pActivity->GetClassName() != "GATutorial" */ && presetActivity->GetClassName().find("Editor") == string::npos) {
			// Prepare a new entry in the list of Activities that we have.
			pair<Activity *, list<Scene *> > newPair(presetActivity, list<Scene *>());
			for (Scene *filteredScene : filteredScenes) {
				// Check if the Scene has the required Areas and such needed for this Activity.
				if (presetActivity->SceneIsCompatible(filteredScene)) {
					newPair.second.push_back(filteredScene);
				}
			}

			m_Activities.insert(newPair);
			// Add to the activity selection combobox, and attach the activity copy, not passing in ownership.
			m_ActivitySelectComboBox->AddItem(presetActivity->GetPresetName(), "", 0, presetActivity);

			// Save the tutorial mission so we can select it by default.
			if (presetActivity->GetClassName() == "GATutorial") {
				tutorialIndex = index;
			}
			index++;
		}
	}

	// Select the Tutorial Activity and Scene by default to start.
	if (selectTutorial && tutorialIndex >= 0) {
		m_ActivitySelectComboBox->SetSelectedIndex(tutorialIndex);
	} else {
		m_ActivitySelectComboBox->SetSelectedIndex(selectedActivityIndex);
	}

	UpdateActivityBox();
	if (m_ScenarioScenes) {
		m_ScenarioSelectedScene = m_ScenarioScenes->front();
		ShowScenesBox();
		CalculateLinesToSitePoint();
	} else {
		m_ScenarioSelectedScene = nullptr;
		HideScenesBox();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateSiteNameLabel(const string &text, const Vector &location) {
	m_ScenarioScenePlanetLabel->SetText(text);
	Vector AbsolutePosition = m_PlanetCenter + location - Vector(m_ScenarioScenePlanetLabel->GetWidth() / 2, 0.0F) - Vector(0.0F, m_ScenarioScenePlanetLabel->GetHeight() * 1.5F);
	float padding = 6.0F;
	AbsolutePosition.m_X = std::clamp(AbsolutePosition.m_X, padding, g_FrameMan.GetResX() - m_ScenarioScenePlanetLabel->GetWidth() - padding);
	AbsolutePosition.m_Y = std::clamp(AbsolutePosition.m_Y,padding, g_FrameMan.GetResY() - m_ScenarioScenePlanetLabel->GetHeight() - padding);
	m_ScenarioScenePlanetLabel->SetPositionAbs(AbsolutePosition.GetFloorIntX(), AbsolutePosition.GetFloorIntY());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const {
	int blendAmount = 210 + RandomNum(-15, 15);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY(), end.GetFloorIntX(), end.GetFloorIntY(), color);
	/* Looks like ass.
		// Draw the thickener lines thicker in the appropriate directions
		if (fabs(end.GetFloorIntX() - start.GetFloorIntX()) > fabs(end.GetFloorIntY() - start.GetFloorIntY()))
		{
			line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY() + 1, end.GetFloorIntX(), end.GetFloorIntY() + 1, color);
			line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY() - 1, end.GetFloorIntX(), end.GetFloorIntY() - 1, color);
		}
		else
		{
			line(drawBitmap, start.GetFloorIntX() + 1, start.GetFloorIntY(), end.GetFloorIntX() + 1, end.GetFloorIntY(), color);
			line(drawBitmap, start.GetFloorIntX() - 1, start.GetFloorIntY(), end.GetFloorIntX() - 1, end.GetFloorIntY(), color);
		}
	*/
	blendAmount = 45 + RandomNum(-25, 25);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	line(drawBitmap, start.GetFloorIntX() + 1, start.GetFloorIntY(), end.GetFloorIntX() + 1, end.GetFloorIntY(), color);
	line(drawBitmap, start.GetFloorIntX() - 1, start.GetFloorIntY(), end.GetFloorIntX() - 1, end.GetFloorIntY(), color);
	line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY() + 1, end.GetFloorIntX(), end.GetFloorIntY() + 1, color);
	line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY() - 1, end.GetFloorIntX(), end.GetFloorIntY() - 1, color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawWhiteScreenLineToSitePoint(BITMAP *drawBitmap, const Vector &planetPoint) const {
	const int color = c_GUIColorWhite;
	const Vector sitePos = m_PlanetCenter + planetPoint;
	const int circleRadius = 8;

	for (int index = 0; index < m_LinePointsToSite.size() - 1; index++) {
		DrawGlowLine(drawBitmap, m_LinePointsToSite[index], m_LinePointsToSite[index + 1], color);
	}

	// Draw a circle around the site target.
	int blendAmount = 225 + RandomNum(-20, 20);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

	circle(drawBitmap, sitePos.GetFloorIntX(), sitePos.GetFloorIntY(), circleRadius, color);
	circle(drawBitmap, sitePos.GetFloorIntX(), sitePos.GetFloorIntY(), circleRadius - 1, color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::CalculateLinesToSitePoint() {
	const Vector sceneInfoBoxPos(m_ScenarioScreenBoxes[SCENEINFO]->GetXPos() + (m_ScenarioScreenBoxes[SCENEINFO]->GetWidth() / 2), m_ScenarioScreenBoxes[SCENEINFO]->GetYPos() + (m_ScenarioScreenBoxes[SCENEINFO]->GetHeight() / 2));
	const Vector planetPoint = m_ScenarioSelectedScene->GetLocation() + m_ScenarioSelectedScene->GetLocationOffset();
	const int channelHeight = (m_ScenarioScreenBoxes[SCENEINFO]->GetHeight() / 2) + CHAMFERSIZE + 6;
	const Vector sitePos = m_PlanetCenter + planetPoint;
	const float yDirMult = sitePos.m_Y < sceneInfoBoxPos.m_Y ? -1.0F : 1.0F;
	const int circleRadius = 8;

	m_LinePointsToSite.clear();

	if (std::fabs(sitePos.GetFloorIntX() - sceneInfoBoxPos.GetFloorIntX()) < circleRadius) {
		// No bends, meaning the mid of the meter goes straight up/down into the site circle.
		m_LinePointsToSite.emplace_back(sceneInfoBoxPos + Vector(sitePos.m_X - sceneInfoBoxPos.m_X, 0));
		m_LinePointsToSite.emplace_back(sitePos + Vector(0, (circleRadius + 1) * -yDirMult));
	} else if (std::fabs(sitePos.GetFloorIntY() - sceneInfoBoxPos.GetFloorIntY()) < (channelHeight - circleRadius)) {
		// Extra lines depending on whether there needs to be two bends due to the site being in the 'channel', ie next to the floating player bar.
		const Vector firstBend(sceneInfoBoxPos.m_X, sceneInfoBoxPos.m_Y + channelHeight * yDirMult);
		const Vector secondBend(sitePos.m_X, firstBend.m_Y);
		Vector chamferPoint1;
		Vector chamferPoint2;
		const float xDirMult = sitePos.m_X < sceneInfoBoxPos.m_X ? -1.0F : 1.0F;
		int chamferSize = CHAMFERSIZE;
		// Cap the chamfer size on the second bend appropriately.
		chamferSize = std::min(static_cast<int>((firstBend - secondBend).GetMagnitude() - 15), chamferSize);
		chamferSize = std::min(static_cast<int>((secondBend - sitePos).GetMagnitude() - circleRadius * 3), chamferSize);
		// Snap the chamfer to not exist below a minimum size.
		chamferSize = (chamferSize < 15) ? 0 : chamferSize;
		// No inverted chamfer.
		chamferSize = std::max(0, chamferSize);
		chamferPoint1.SetXY(secondBend.m_X + chamferSize * -xDirMult, secondBend.m_Y);
		chamferPoint2.SetXY(secondBend.m_X, secondBend.m_Y + chamferSize * -yDirMult);
		// Line to the first bend.
		m_LinePointsToSite.emplace_back(sceneInfoBoxPos);
		m_LinePointsToSite.emplace_back(firstBend);
		// Line to the second bend, including the chamfer.
		m_LinePointsToSite.emplace_back(chamferPoint1);
		if (chamferSize > 0) {
			m_LinePointsToSite.emplace_back(chamferPoint2);
		}
		// Line to the site.
		m_LinePointsToSite.emplace_back(sitePos + Vector(0, (circleRadius + 1) * yDirMult));
	} else {
		// Just one bend.
		const Vector firstBend(sceneInfoBoxPos.m_X, sitePos.m_Y);
		Vector chamferPoint1;
		Vector chamferPoint2;
		const float xDirMult = sitePos.m_X < sceneInfoBoxPos.m_X ? -1.0F : 1.0F;
		int chamferSize = CHAMFERSIZE;
		// Cap the chamfer size on the first bend appropriately.
		chamferSize = std::min(static_cast<int>((sceneInfoBoxPos - firstBend).GetMagnitude() - 15), chamferSize);
		chamferSize = std::min(static_cast<int>((firstBend - sitePos).GetMagnitude() - circleRadius * 3), chamferSize);
		// Snap the chamfer to not exist below a minimum size.
		chamferSize = (chamferSize < 15) ? 0 : chamferSize;
		// No inverted chamfer.
		chamferSize = std::max(0, chamferSize);
		chamferPoint1.SetXY(sceneInfoBoxPos.m_X, firstBend.m_Y + chamferSize * -yDirMult);
		chamferPoint2.SetXY(firstBend.m_X + chamferSize * xDirMult, sitePos.m_Y);
		// Line to the first bend, including the chamfer.
		m_LinePointsToSite.emplace_back(sceneInfoBoxPos);
		m_LinePointsToSite.emplace_back(chamferPoint1);
		if (chamferSize > 0) {
			m_LinePointsToSite.emplace_back(chamferPoint2);
		}
		// Line to the site.
		m_LinePointsToSite.emplace_back(sitePos + Vector((circleRadius + 1) * -xDirMult, 0));
	}
}

void ScenarioGUI::HideScenesBox() const {
	m_ScenarioScreenBoxes[SCENEINFO]->SetVisible(false);
}
