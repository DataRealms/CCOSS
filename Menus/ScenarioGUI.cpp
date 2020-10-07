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
	m_ScenarioController = 0;
	m_ScenarioGUIScreen = 0;
	m_ScenarioGUIInput = 0;
	m_ScenarioGUIController = 0;
	m_MenuEnabled = ENABLED;
	m_MenuScreen = SCENESELECT;
	m_ScreenChange = false;
	m_SceneFocus = 0;
	m_FocusChange = 0;
	m_MenuSpeed = 0.3F;
	m_BlinkTimer.Reset();
	m_BlinkMode = NOBLINK;

	m_PlanetCenter.Reset();
	m_PlanetRadius = 240.0F;

	m_ScenarioRootBox = 0;
	m_ScenarioActivityBox = 0;
	m_ScenarioSceneInfoBox = 0;
	m_ScenarioPlayerSetupBox = 0;
	m_ScenarioQuitConfirmBox = 0;

	m_SelectTutorial = true;

	for (int button = 0; button < SCENARIOBUTTONCOUNT; ++button) {
		m_ScenarioButtons[button] = 0;
	}
	m_ScenarioScenePlanetLabel = 0;

	m_ActivitySelectComboBox = 0;
	m_ActivityLabel = 0;
	m_DifficultyLabel = 0;
	m_DifficultySlider = 0;

	m_SceneCloseButton = 0;
	m_SceneNameLabel = 0;
	m_SceneInfoLabel = 0;
	for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player) {
		for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
			//m_aaControls = team == TEAM_DISABLED;
			m_PlayerBoxes[player][team] = 0;
		}
	}

	for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
		m_TeamBoxes[team] = 0;
		m_TeamNameLabels[team] = 0;
	}

	for (int team = Activity::TeamOne; team < RTE::Activity::MaxTeamCount; ++team) {
		m_TeamTechSelect[team] = 0;
		m_TeamAISkillSlider[team] = 0;
		m_TeamAISkillLabel[team] = 0;
	}

	m_StartErrorLabel = 0;
	m_CPULockLabel = 0;
	m_LockedCPUTeam = Activity::NoTeam;

	m_GoldLabel = 0;
	m_GoldSlider = 0;
	m_FogOfWarCheckbox = 0;
	m_RequireClearPathToOrbitCheckbox = 0;
	m_DeployUnitsCheckbox = 0;

	m_QuitConfirmLabel = 0;
	m_QuitConfirmButton = 0;

	m_ScenarioScenes = 0;
	m_Activities.clear();
	m_ScenarioDraggedBox = 0;
	m_EngageDrag = false;
	m_ScenarioHoveredScene = 0;
	m_ScenarioSelectedScene = 0;
	m_PrevMousePos.Reset();

	m_ActivityRestarted = false;
	m_ActivityResumed = false;
	m_StartPlayers = 1;
	m_StartTeams = 2;
	m_StartFunds = 1600;
	m_StartDifficulty = Activity::MediumDifficulty;
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

	// Make sure we have convenient points to the containing GUI collection boxes that we will manipulate the positions of
	m_ScenarioRootBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("root"));
	m_ScenarioActivityBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ActivitySelectBox"));
	m_ScenarioSceneInfoBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("SceneInfoBox"));
	m_ScenarioPlayerSetupBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("PlayerSetupBox"));
	m_ScenarioQuitConfirmBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("ConfirmDialog"));

	// Make the root box fill the screen
	m_ScenarioRootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

	m_ScenarioButtons[BACKTOMAIN] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("BackToMainButton"));
	m_ScenarioButtons[RESUME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ButtonResume"));
	m_ScenarioButtons[STARTHERE] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneSelectButton"));
	m_ScenarioButtons[STARTGAME] = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("StartButton"));

	m_ScenarioScenePlanetLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ScenePlanetLabel"));
	m_ScenarioScenePlanetLabel->SetVisible(false);

	// Activity Selection Box
	m_ActivitySelectComboBox = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("ActivitySelectCombo"));
	m_ActivityLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ActivityDescLabel"));
	m_DifficultyLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("DifficultyLabel"));
	m_DifficultySlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("DifficultySlider"));
	m_ActivitySelectComboBox->SetDropHeight(64);
	//    m_ActivitySelectComboBox->GetListPanel()->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));
	m_ActivityLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));

	// Scene Info Box
	m_SceneCloseButton = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("SceneCloseButton"));
	m_SceneNameLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneNameLabel"));
	m_SceneInfoLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("SceneInfoLabel"));
	m_SceneInfoLabel->SetFont(m_ScenarioGUIController->GetSkin()->GetFont("smallfont.png"));

	// Player team assignment box
	char str[128];
	for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player) {
		for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
			// +1 because the controls are indexed starting at 1, not 0
			std::snprintf(str, sizeof(str), "P%dT%dBox", player + 1, team + 1);
			m_PlayerBoxes[player][team] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl(str));
		}
	}

	m_TeamBoxes[TEAM_DISABLED] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("TDIcon"));
	m_TeamBoxes[Activity::TeamOne] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("T1Icon"));
	m_TeamBoxes[Activity::TeamTwo] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("T2Icon"));
	m_TeamBoxes[Activity::TeamThree] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("T3Icon"));
	m_TeamBoxes[Activity::TeamFour] = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControl("T4Icon"));
	m_TeamNameLabels[TEAM_DISABLED] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("TDLabel"));
	m_TeamNameLabels[Activity::TeamOne] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T1Label"));
	m_TeamNameLabels[Activity::TeamTwo] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T2Label"));
	m_TeamNameLabels[Activity::TeamThree] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T3Label"));
	m_TeamNameLabels[Activity::TeamFour] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T4Label"));
	m_TeamTechSelect[Activity::TeamOne] = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("T1TechCombo"));
	m_TeamTechSelect[Activity::TeamTwo] = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("T2TechCombo"));
	m_TeamTechSelect[Activity::TeamThree] = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("T3TechCombo"));
	m_TeamTechSelect[Activity::TeamFour] = dynamic_cast<GUIComboBox *>(m_ScenarioGUIController->GetControl("T4TechCombo"));
	m_TeamAISkillSlider[Activity::TeamOne] = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("T1AISkillSlider"));
	m_TeamAISkillSlider[Activity::TeamTwo] = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("T2AISkillSlider"));
	m_TeamAISkillSlider[Activity::TeamThree] = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("T3AISkillSlider"));
	m_TeamAISkillSlider[Activity::TeamFour] = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("T4AISkillSlider"));
	m_TeamAISkillLabel[Activity::TeamOne] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T1AISkillLabel"));
	m_TeamAISkillLabel[Activity::TeamTwo] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T2AISkillLabel"));
	m_TeamAISkillLabel[Activity::TeamThree] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T3AISkillLabel"));
	m_TeamAISkillLabel[Activity::TeamFour] = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("T4AISkillLabel"));

	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++) {
		// Hide tech combobxes by default
		m_TeamTechSelect[team]->SetEnabled(false);
		m_TeamTechSelect[team]->SetVisible(false);
		// Add a "Random" tech choice to all the combo boxes first so that's at least in there always
		m_TeamTechSelect[team]->GetListPanel()->AddItem("-All-", "", 0, 0, -2);
		m_TeamTechSelect[team]->GetListPanel()->AddItem("-Random-", "", 0, 0, -1);
		m_TeamTechSelect[team]->SetSelectedIndex(0);
		// Hide AIs skill combobxes by default
		m_TeamAISkillSlider[team]->SetEnabled(false);
		m_TeamAISkillSlider[team]->SetVisible(false);
		m_TeamAISkillSlider[team]->SetValue(Activity::DefaultSkill);

		m_TeamAISkillLabel[team]->SetEnabled(false);
		m_TeamAISkillLabel[team]->SetVisible(false);
		m_TeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_TeamAISkillSlider[team]->GetValue()));
	}
	m_StartErrorLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("StartErrorLabel"));
	m_CPULockLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("CPULockLabel"));

	// Populate the tech comboboxes with the available tech modules
	const DataModule *dataModule = nullptr;
	for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i) {
		dataModule = g_PresetMan.GetDataModule(i);
		if (dataModule) {
			std::string techName = dataModule->GetFriendlyName();
			std::string techString = " Tech";
			std::string::size_type techPos = techName.find(techString);
			if (techPos != string::npos) {
				techName.replace(techPos, techString.length(), "");
				m_TeamTechSelect[Activity::TeamOne]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_TeamTechSelect[Activity::TeamTwo]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_TeamTechSelect[Activity::TeamThree]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_TeamTechSelect[Activity::TeamFour]->GetListPanel()->AddItem(techName, "", 0, 0, i);
			}
		}
	}

	// Make the lists be scrolled to the top when they are initially dropped
	m_TeamTechSelect[Activity::TeamOne]->GetListPanel()->ScrollToTop();
	m_TeamTechSelect[Activity::TeamTwo]->GetListPanel()->ScrollToTop();
	m_TeamTechSelect[Activity::TeamThree]->GetListPanel()->ScrollToTop();
	m_TeamTechSelect[Activity::TeamFour]->GetListPanel()->ScrollToTop();

	m_GoldLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("GoldLabel"));
	m_GoldSlider = dynamic_cast<GUISlider *>(m_ScenarioGUIController->GetControl("GoldSlider"));
	m_FogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("FogOfWarCheckbox"));
	m_RequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
	m_DeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_ScenarioGUIController->GetControl("DeployUnitsCheckbox"));

	m_QuitConfirmLabel = dynamic_cast<GUILabel *>(m_ScenarioGUIController->GetControl("ConfirmLabel"));
	m_QuitConfirmButton = dynamic_cast<GUIButton *>(m_ScenarioGUIController->GetControl("ConfirmButton"));

	// Set up initial combo box locations and sizes
	m_ScenarioButtons[BACKTOMAIN]->SetPositionRel(m_ScenarioRootBox->GetWidth() - m_ScenarioButtons[BACKTOMAIN]->GetWidth() - 16, m_ScenarioRootBox->GetHeight() - m_ScenarioButtons[BACKTOMAIN]->GetHeight() - 22);
	m_ScenarioButtons[RESUME]->SetPositionRel(m_ScenarioRootBox->GetWidth() - m_ScenarioButtons[RESUME]->GetWidth() - 16, m_ScenarioRootBox->GetHeight() - m_ScenarioButtons[RESUME]->GetHeight() - 47);
	m_ScenarioActivityBox->SetPositionRel(16, 16);
	m_ScenarioSceneInfoBox->SetPositionRel(m_ScenarioRootBox->GetWidth() - m_ScenarioSceneInfoBox->GetWidth() - 16, 16);
	m_ScenarioPlayerSetupBox->CenterInParent(true, true);
	m_ScenarioQuitConfirmBox->CenterInParent(true, true);

	m_ScenePreviewBitmap = create_bitmap_ex(8, Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT);

	// Load default preview bitmap
	ContentFile defaultPreview("Base.rte/GUIs/DefaultPreview.png");
	m_DefaultPreviewBitmap = defaultPreview.LoadAndReleaseBitmap();

	clear_to_color(m_ScenePreviewBitmap, g_MaskColor);

	// Set initial focus, category list, and label settings
	m_ScreenChange = true;
	m_FocusChange = 1;
	//CategoryChange();

	// Fill our Scene and Activity lists
	GetAllScenesAndActivities();

	// Update the first box visible
	UpdateActivityBox();

	// Hide all screens, the appropriate screen will reappear on next update
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

void ScenarioGUI::SetEnabled(bool enable) {
	if (enable && m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
		m_MenuEnabled = ENABLING;
		g_GUISound.EnterMenuSound()->Play();
	} else if (!enable && m_MenuEnabled != DISABLED && m_MenuEnabled != DISABLING) {
		m_MenuEnabled = DISABLING;
		g_GUISound.ExitMenuSound()->Play();
	} else if (enable && m_MenuEnabled == ENABLED) {
		HideAllScreens();
		m_MenuScreen = SCENESELECT;
		// Reload all scenes and actvities to reflect scene changes player might do in scene editor.
		GetAllScenesAndActivities();
	}

	m_ScreenChange = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScenarioGUI::ScenarioUpdateResult ScenarioGUI::Update() {
	ScenarioUpdateResult result = ScenarioUpdateResult::NOEVENT;
	m_ScenarioController->Update();

	// Reset the specific triggers
	m_ActivityRestarted = false;
	m_ActivityResumed = false;
	m_StartDifficulty = 0;

	if (g_ConsoleMan.IsEnabled()) {
		return result;
	}

	if (m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
		return result;
	}

	////////////////////////////////////////////
	// Do all input handling!

	int mouseX;
	int mouseY;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
	Vector mousePos(static_cast<float>(mouseX),static_cast<float>(mouseY));

	ScenarioUpdateInputResult updateInputResult = UpdateInput();
	if (updateInputResult == ScenarioUpdateInputResult::BACKTOMAIN) {
		result = ScenarioUpdateResult::BACKTOMAIN;
	}

	/*
		////////////////////////////////////////////
		// Notification blinking logic

		if (m_BlinkMode == NOFUNDS)
		{
			m_pCostLabel->SetVisible((m_BlinkTimer.GetElapsedRealTimeMS() % 500) > 250);
		}
		else if (m_BlinkMode == NOCRAFT)
		{
			bool blink = (m_BlinkTimer.GetElapsedRealTimeMS() % 500) > 250;
			m_pCraftLabel->SetVisible(blink);
			m_pCraftBox->SetVisible(blink);
		}

		// Time out the blinker
		if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastRealMS(1500))
		{
			m_pCostLabel->SetVisible(true);
			m_pCraftLabel->SetVisible(true);
			m_pCraftBox->SetVisible(true);
			m_BlinkMode = NOBLINK;
		}
	*/

	//////////////////////////////////////
	// SCENE SELECTION SCREEN

	if (m_MenuScreen == SCENESELECT) {
		if (m_ScreenChange) {
			m_ScenarioActivityBox->SetVisible(true);
			// Scene box only appears when a scene is actively selected
			m_ScenarioSceneInfoBox->SetVisible(false);

			m_ScreenChange = false;
		}

		if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
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

		// Get mouse position so we can see which scene it hovers over
		int mouseX;
		int mouseY;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		Vector mousePos(static_cast<float>(mouseX),static_cast<float>(mouseY));
		Vector screenLocation;

		bool mouseIsInBox = false;

		// Detect if mouse is inside UI boxes
		GUIRect *rect;
		rect = m_ScenarioActivityBox->GetRect();
		if (mouseX > rect->left && mouseX < rect->right && mouseY > rect->top && mouseY < rect->bottom) {
			mouseIsInBox = true;
		}

		rect = m_ScenarioSceneInfoBox->GetRect();
		if (mouseX > rect->left && mouseX < rect->right && mouseY > rect->top && mouseY < rect->bottom) {
			mouseIsInBox = true;
		}

		// Validate mouse position as being over the planet area for hover operations!
		if (m_ScenarioScenes && !m_ScenarioDraggedBox && (mousePos - m_PlanetCenter).GetMagnitude() < m_PlanetRadius && !mouseIsInBox) {
			// If unlocked, detect any Scene close to the mouse and highlight it
			bool foundAnyHover = false;
			//bool foundNewHover = false;
			list<Scene *>::iterator sItr;
			list<Scene *>::iterator newCandidateItr = m_ScenarioScenes->end();
			float distance = 0;
			float shortestDist = 1000000.0F;
			for (sItr = m_ScenarioScenes->begin(); sItr != m_ScenarioScenes->end(); ++sItr) {
				screenLocation = m_PlanetCenter + (*sItr)->GetLocation() + (*sItr)->GetLocationOffset();
				distance = (screenLocation - mousePos).GetMagnitude();

				// The first new scene the mouse's position is close to when unlocked, make selected
				if (distance < 16 && distance < shortestDist) {
					// This is now the shortest
					shortestDist = distance;
					foundAnyHover = true;
					// See if the scene hovered is different from the previously hovered one, and if so, set it to the new candidate to switch hovering to
// Actually, don't because it will cause alternating each frame if two hover zones overlap!
//                    if (*sItr != m_ScenarioHoveredScene)
					newCandidateItr = sItr;
				}
			}

			// Set new hovered scene to be the one now closest to the cursor, if there is any and if it is different the a currently hovered one
			if (newCandidateItr != m_ScenarioScenes->end() && (*newCandidateItr) != m_ScenarioHoveredScene) {
				m_ScenarioHoveredScene = (*newCandidateItr);
				//foundNewHover = true;
				g_GUISound.SelectionChangeSound()->Play();
				UpdateScenesBox();
			}

			if (!foundAnyHover) {
				m_ScenarioHoveredScene = 0;
			}

			// Set up the hover label to appear over any hovered scene location
			if (m_ScenarioHoveredScene) {
				UpdateSiteNameLabel(true, m_ScenarioHoveredScene->GetPresetName(), m_ScenarioHoveredScene->GetLocation() + m_ScenarioHoveredScene->GetLocationOffset());
			} else {
				UpdateSiteNameLabel(false);
			}

			// If clicked, whatever is hovered becomes selected
			if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER)) {
				if (m_ScenarioHoveredScene) {
					m_ScenarioSelectedScene = m_ScenarioHoveredScene;
					g_GUISound.ItemChangeSound()->Play();
					UpdateScenesBox();
				}
				/* Can't do this, doesn't take into account clicks on floating UI boxes
								// Not hovering over anything on click, so deselect whatever was selected
								else if (m_ScenarioSelectedScene)
								{
									m_ScenarioSelectedScene = 0;
									g_GUISound.FocusChangeSound()->Play();
								}
				*/
			}
		}

		// Update the Activity setup and Scene info boxes
// No need to do this all the time, just when controls change
//        UpdateActivityBox();
		//if (!m_pSchemeSelect->IsDropped())
		UpdateScenesBox();
	}

	//////////////////////////////////////
	// PLAYER TEAM ASSIGNMENT SCREEN

	else if (m_MenuScreen == PLAYERSETUP) {
		if (m_ScreenChange) {
			m_ScenarioPlayerSetupBox->SetVisible(true);
			//            m_ScenarioButtons[BACKTOMAIN]->SetVisible(true);
			m_ScreenChange = false;
		}

		// Update the player selection box
		UpdatePlayersBox(false);

		//        m_ScenarioButtons[BACKTOMAIN]->SetFocus();
	}

	//////////////////////////////////////
	// QUIT CONFIRM DIALOG

	else if (m_MenuScreen == CONFIRMQUIT) {
		if (m_ScreenChange) {
			m_ScenarioQuitConfirmBox->SetVisible(true);
			m_ScreenChange = false;
		}

		//        m_ScenarioButtons[BACKTOMAIN]->SetFocus();
	}

	// Save mouse pos for next frame so we can do dragging
	if (m_EngageDrag) {
		m_PrevMousePos = mousePos;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::Draw(BITMAP *drawBitmap) const {
	// Transparency effect on the scene dots and lines
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
	// Screen blend the dots and lines, with some flicekring in its intensity
	int blendAmount = 120 + RandomNum(-55, 55);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

	// Draw sites etc only when selecting them
	if (m_MenuScreen == SCENESELECT && m_ScenarioScenes) {
		// Draw the scene location dots
		Vector screenLocation;
		for (const Scene * scene : *m_ScenarioScenes) {
			int color;

			// Mark user-created scenes to let players easily distinguish them from built-in
			if (scene->GetModuleID() == g_PresetMan.GetModuleID("Scenes.rte")) {
				color = c_GUIColorGreen;
			} else {
				color = c_GUIColorYellow;
			}

			screenLocation = m_PlanetCenter + scene->GetLocation() + scene->GetLocationOffset();
			blendAmount = 85 + RandomNum(-25, 25);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocation.GetFloorIntX(), screenLocation.GetFloorIntY(), 4, color);
			circlefill(drawBitmap, screenLocation.GetFloorIntX(), screenLocation.GetFloorIntY(), 2, color);
			blendAmount = 200 + RandomNum(-55, 55);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			circlefill(drawBitmap, screenLocation.GetFloorIntX(), screenLocation.GetFloorIntY(), 1, color);
		}

		// Draw the lines etc pointing at the selected Scene from the Scene Info box
		if (m_ScenarioSelectedScene && m_ScenarioSceneInfoBox->GetVisible()) {
			Vector sceneInfoBoxPos(m_ScenarioSceneInfoBox->GetXPos() + (m_ScenarioSceneInfoBox->GetWidth() / 2), m_ScenarioSceneInfoBox->GetYPos() + (m_ScenarioSceneInfoBox->GetHeight() / 2));
			DrawScreenLineToSitePoint(drawBitmap, sceneInfoBoxPos, m_ScenarioSelectedScene->GetLocation() + m_ScenarioSelectedScene->GetLocationOffset(), c_GUIColorWhite, -1, -1, (m_ScenarioSceneInfoBox->GetHeight() / 2) + CHAMFERSIZE + 6, 1.0);
		}
	}

	// Back to solid drawing
	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	AllegroScreen drawScreen(drawBitmap);

	m_ScenarioGUIController->Draw(&drawScreen);

	// Draw scene preview after GUI
	if (m_MenuScreen == SCENESELECT && m_ScenarioScenes && m_ScenarioSelectedScene && m_ScenarioSceneInfoBox->GetVisible()) {
		BITMAP *preview = m_ScenarioSelectedScene->GetPreviewBitmap();
		int xOffset = 0;
		int yOffset = 0;
		preview = preview ? preview : m_DefaultPreviewBitmap;
		blit(preview, m_ScenePreviewBitmap, xOffset, yOffset, 0, 0, m_ScenePreviewBitmap->w, m_ScenePreviewBitmap->h);
		draw_sprite(drawBitmap, m_ScenePreviewBitmap, m_ScenarioSceneInfoBox->GetXPos() + 10, m_ScenarioSceneInfoBox->GetYPos() + 33);
	}

	// Draw the Player-Team matrix lines and disabled overlay effects
	if (m_MenuScreen == PLAYERSETUP) {
		const Activity *pActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : 0;
		int lineY = 80;
		for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team) {
			// Disabled shaded boxes
			if (pActivity && (!pActivity->TeamActive(team) || m_LockedCPUTeam == team)) {
				// TODO: understand why the blending isnt working as desired
				//Transparency effect on the overlay boxes
				drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
				// Screen blend the dots and lines, with some flicekring in its intensity
				int blendAmountInner = 230;
				set_screen_blender(blendAmountInner, blendAmountInner, blendAmountInner, blendAmountInner);
				rectfill(drawBitmap, m_ScenarioPlayerSetupBox->GetXPos() + 110, m_ScenarioPlayerSetupBox->GetYPos() + lineY, m_ScenarioPlayerSetupBox->GetXPos() + m_ScenarioPlayerSetupBox->GetWidth() - 12, m_ScenarioPlayerSetupBox->GetYPos() + lineY + 25, c_GUIColorDarkBlue);
			}
			// Back to solid drawing
			drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
			// Cell border separator lines
			line(drawBitmap, m_ScenarioPlayerSetupBox->GetXPos() + 110, m_ScenarioPlayerSetupBox->GetYPos() + lineY, m_ScenarioPlayerSetupBox->GetXPos() + m_ScenarioPlayerSetupBox->GetWidth() - 12, m_ScenarioPlayerSetupBox->GetYPos() + lineY, c_GUIColorLightBlue);
			lineY += 25;
		}

		// Manually draw UI elements on top of colored rectangle
		for (int team = Activity::MaxTeamCount - 1; team >= Activity::TeamOne; team--) {
			if (m_TeamTechSelect[team]->GetVisible()) {
				m_TeamTechSelect[team]->Draw(&drawScreen);
				if (m_TeamTechSelect[team]->IsDropped()) {
					m_TeamTechSelect[team]->GetListPanel()->Draw(&drawScreen);
				}
			}

			if (m_TeamAISkillSlider[team]->GetVisible()) {
				m_TeamAISkillSlider[team]->Draw(&drawScreen);
				m_TeamAISkillLabel[team]->Draw(&drawScreen);
			}
		}
	}

	m_ScenarioGUIController->DrawMouse();

	// Show who controls the cursor
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

	// Show which joysticks are detected by the game
	for (int joystick = Players::PlayerOne; joystick < Players::MaxPlayerCount; joystick++) {
		if (g_UInputMan.JoystickActive(joystick)) {
			int matchedDevice = DEVICE_GAMEPAD_1 + joystick;

			if (matchedDevice != device) {
				const Icon *pIcon = g_UInputMan.GetDeviceIcon(matchedDevice);
				if (pIcon) {
					draw_sprite(drawBitmap, pIcon->GetBitmaps8()[0], g_FrameMan.GetResX() - 30 * g_UInputMan.GetJoystickCount() + 30 * joystick, g_FrameMan.GetResY() - 25);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScenarioGUI::ScenarioUpdateInputResult ScenarioGUI::UpdateInput() {
	ScenarioUpdateInputResult result = NOEVENT;
	// TODO: if activity is running, allow esc to resume activity instead of quitting.
	// If esc pressed, show quit dialog if applicable
	if (g_UInputMan.KeyPressed(KEY_ESC)) {
		// Just quit if the dialog is already up
		if (m_ScenarioQuitConfirmBox->GetVisible()) {
			g_Quit = true;
		} else {
			HideAllScreens();
			m_QuitConfirmLabel->SetText("Sure you want to quit to OS?"); //\nAny unsaved progress\nwill be lost!");
			m_QuitConfirmButton->SetText("Quit");
			m_ScenarioQuitConfirmBox->SetVisible(true);
			m_MenuScreen = CONFIRMQUIT;
			m_ScreenChange = true;
		}
	}

	///////////////////////////////////////////////////////////
	// Mouse handling

	// Get mouse position
	int mouseX;
	int mouseY;
	m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
	Vector mousePos(static_cast<float>(mouseX),static_cast<float>(mouseY));

	// If not currently dragging a box, see if we should start
	bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
	if (m_MenuScreen == SCENESELECT && !m_ScenarioDraggedBox && menuButtonHeld && !m_EngageDrag && !m_ActivitySelectComboBox->IsDropped()) {
		GUICollectionBox *pBox = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioRootBox, 1));

		if (pBox == m_ScenarioActivityBox || pBox == m_ScenarioSceneInfoBox) {
			m_ScenarioDraggedBox = pBox;
		}

		// Save the mouse pos at the start of the drag so we can measure if we should engage
		if (m_ScenarioDraggedBox) {
			m_PrevMousePos = mousePos;
		}
	} else if (!menuButtonHeld) {
		m_ScenarioDraggedBox = 0;
		m_EngageDrag = false;
	}

	// Figure out dragging of the dialog boxes, if one is being dragged.
	// Only start drag if we're over a threshold, to prevent small unintentional nudges.
	if (m_ScenarioDraggedBox && !m_EngageDrag && (mousePos - m_PrevMousePos).GetLargest() > 4) {
		m_EngageDrag = true;
	}

	// Actually drag if we now are engaged
	if (m_ScenarioDraggedBox && m_EngageDrag) {
		m_ScenarioDraggedBox->MoveRelative(mousePos.GetFloorIntX() - m_PrevMousePos.GetFloorIntX(), mousePos.GetFloorIntY() - m_PrevMousePos.GetFloorIntY());
		// Ensure the drag didn't shove it off-screen
		KeepBoxOnScreen(m_ScenarioDraggedBox);
	}

	//////////////////////////////////////////
	// Update the ControlManager

	m_ScenarioGUIController->Update();

	///////////////////////////////////////
	// Handle events

	GUIEvent anEvent;
	while (m_ScenarioGUIController->GetEvent(&anEvent)) {
		std::string eventControlName = anEvent.GetControl()->GetName();
		GUIControl *eventControl = anEvent.GetControl();
		// Commands
		if (anEvent.GetType() == GUIEvent::Command) {
			// Return to main menu button pressed
			if (eventControlName == "BackToMainButton") {
				// Hide all screens, the appropriate screen will reappear on next update
				HideAllScreens();

				m_MenuScreen = SCENESELECT;
				m_ScreenChange = true;

				g_GUISound.BackButtonPressSound()->Play();

				result = ScenarioUpdateInputResult::BACKTOMAIN;
			}

			// Quit program button pressed
			else if (eventControlName == "ConfirmButton") {
				// Hide all screens, the appropriate screen will reappear on next update
				HideAllScreens();
				m_ScreenChange = true;
				g_Quit = true;

				g_GUISound.BackButtonPressSound()->Play();
			}

			else if (eventControlName == "ButtonResume") {
				m_ActivityResumed = true;
				g_GUISound.BackButtonPressSound()->Play();
			}

			// Most big dialog cancel buttons lead back to the game menu too
			else if (eventControlName == "PlayerCancelButton" || eventControlName == "ConfirmCancelButton") {
				// Hide all previously shown screens
				HideAllScreens();
				m_MenuScreen = SCENESELECT;
				m_ScreenChange = true;
				g_GUISound.BackButtonPressSound()->Play();
			}

			// Start Scenario Here menu button pressed
			else if (eventControl == m_ScenarioButtons[STARTHERE]) {
				// Set up the player setup box based on updated Activity selection
				UpdatePlayersBox(true);
				// Hide all screens, the appropriate screen will reappear on next update
				HideAllScreens();
				m_MenuScreen = PLAYERSETUP;
				m_ScreenChange = true;

				g_GUISound.ButtonPressSound()->Play();
			}

			// Start game button pressed
			else if (eventControl == m_ScenarioButtons[STARTGAME]) {
				// Try to start the game
				if (StartGame()) {
					// Hide all previously shown screens, show
					HideAllScreens();
					//                    m_MenuScreen = SCENESELECT;
					//                    m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				} else {
					g_GUISound.UserErrorSound()->Play();
				}
			}

			else if (eventControl == m_SceneCloseButton) {
				m_ScenarioSelectedScene = 0;
				g_GUISound.ButtonPressSound()->Play();
			}
		}

		else if (anEvent.GetType() == GUIEvent::Notification) {
			// Button focus notification that we can play a sound to
			if (dynamic_cast<GUIButton *>(eventControl)) {
				if (anEvent.GetMsg() == GUIButton::Focused) {
					g_GUISound.SelectionChangeSound()->Play();
				}
				// Also stop dragging any panels if we're over any button
				m_ScenarioDraggedBox = 0;
				m_EngageDrag = true;
			}

			else if (eventControl == m_DifficultySlider) {
				// Update the difficulty label etc
				UpdateActivityBox();

				// Also stop dragging any panels if we're over any button
				m_ScenarioDraggedBox = 0;
				m_EngageDrag = true;
			}

			else if (eventControl == m_ActivitySelectComboBox) {
				// Also stop dragging any panels if we're over the selection list
				m_ScenarioDraggedBox = 0;
				m_EngageDrag = true;

				// The activity selection changed
				if (anEvent.GetMsg() == GUIComboBox::Closed) {
					// Update the difficulty label etc
					UpdateActivityBox();

					// If there is only one Scene compatible with this newly selected Activity, then automatically select it
					if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
						m_ScenarioSelectedScene = m_ScenarioScenes->front();
					}
					// Deselect any previously selected scene.. it may not be compatible with the new activity
					else {
						m_ScenarioSelectedScene = 0;
					}

					// Update the scene info box
					UpdateScenesBox();
					g_GUISound.ItemChangeSound()->Play();
				}
			}
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::HideAllScreens() {
	m_ScenarioActivityBox->SetVisible(false);
	m_ScenarioSceneInfoBox->SetVisible(false);
	m_ScenarioPlayerSetupBox->SetVisible(false);
	m_ScenarioQuitConfirmBox->SetVisible(false);

	m_ScenarioScenePlanetLabel->SetVisible(false);

	m_ScreenChange = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::KeepBoxOnScreen(GUICollectionBox *pBox, int margin) {
	//TODO: Consider the GUIControl anchor point?
	// Make sure the box doesn't go entirely outside of the screen
	if (pBox->GetXPos() < (margin - pBox->GetWidth())) {
		pBox->SetPositionAbs(margin - pBox->GetWidth(), pBox->GetYPos());
	}
	if (pBox->GetXPos() > m_ScenarioRootBox->GetWidth() - margin) {
		pBox->SetPositionAbs(m_ScenarioRootBox->GetWidth() - margin, pBox->GetYPos());
	}
	if (pBox->GetYPos() < (margin - pBox->GetHeight())) {
		pBox->SetPositionAbs(pBox->GetXPos(), margin - pBox->GetHeight());
	}
	if (pBox->GetYPos() > m_ScenarioRootBox->GetHeight() - margin) {
		pBox->SetPositionAbs(pBox->GetXPos(), m_ScenarioRootBox->GetHeight() - margin);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateActivityBox() {
	// Get the currently selected Activity
	const Activity *selectedActivity = nullptr;
	if (m_ActivitySelectComboBox) {
		selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : 0;
	}

	if (selectedActivity) {
		// Pull out the list of Scenes that are compatible with this Activity
		map<Activity *, list<Scene *> >::iterator asItr;
		if (m_Activities.end() != (asItr = m_Activities.find(const_cast<Activity *>(selectedActivity)))) {
			m_ScenarioScenes = &((*asItr).second);
		} else {
			m_ScenarioScenes = 0;
		}

		// Set the description
		if (m_ScenarioScenes && m_ScenarioScenes->size() == 1) {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
		} else if (m_ScenarioScenes && m_ScenarioScenes->size() > 1) {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
		} else {
			m_ActivityLabel->SetText(selectedActivity->GetDescription() + "\n\nNO sites appear to be compatible with this selected activity! Please try another.");
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

		// Resize the box to fit the desc
		int newHeight = m_ActivityLabel->ResizeHeightToFit();
		m_ScenarioActivityBox->Resize(m_ScenarioActivityBox->GetWidth(), newHeight + 110);
		UpdateScenesBox();

		const GameActivity *selectedGA = dynamic_cast<const GameActivity *>(selectedActivity);
		if (selectedGA) {
			// Set gold slider value if activity sepcifies default gold amounts for difficulties
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

			//Set default fog of war flag and enable or disable it if necessary
			if (selectedGA->GetDefaultFogOfWar() > -1) {
				if (selectedGA->GetDefaultFogOfWar() == 0) {
					m_FogOfWarCheckbox->SetCheck(0);
				} else {
					m_FogOfWarCheckbox->SetCheck(1);
				}
			}
			m_FogOfWarCheckbox->SetEnabled(selectedGA->GetFogOfWarSwitchEnabled());

			//Set default clear path to orbit flag and enable or disable it if necessary
			if (selectedGA->GetDefaultRequireClearPathToOrbit() > -1) {
				if (selectedGA->GetDefaultRequireClearPathToOrbit() == 0) {
					m_RequireClearPathToOrbitCheckbox->SetCheck(0);
				} else {
					m_RequireClearPathToOrbitCheckbox->SetCheck(1);
				}
			}
			m_RequireClearPathToOrbitCheckbox->SetEnabled(selectedGA->GetRequireClearPathToOrbitSwitchEnabled());

			//Set default deploy units flag and enable or disable it if necessary
			if (selectedGA->GetDefaultDeployUnits() > -1) {
				if (selectedGA->GetDefaultDeployUnits() == 0) {
					m_DeployUnitsCheckbox->SetCheck(0);
				} else {
					m_DeployUnitsCheckbox->SetCheck(1);
				}
			}
			m_DeployUnitsCheckbox->SetEnabled(selectedGA->GetDeployUnitsSwitchEnabled());
		}
	} else {
		m_ScenarioScenes = 0;
		m_ActivityLabel->SetText("No Activity selected.");
		m_DifficultyLabel->SetVisible(false);
		if (m_DifficultySlider) {
			m_DifficultySlider->SetVisible(false);
		}
		// Resize the box to fit the desc
		int newHeight = m_ActivityLabel->ResizeHeightToFit();
		m_ScenarioActivityBox->Resize(m_ScenarioActivityBox->GetWidth(), newHeight + 125);
		UpdateScenesBox();
	}
	// Make sure the box doesn't go entirely outside of the screen
	KeepBoxOnScreen(m_ScenarioActivityBox);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateScenesBox() {
	// Always show the info box if something is selected
	if (m_ScenarioSelectedScene) {
		// Fill combo box
		const Activity *selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : 0;

		if (selectedActivity) {
			// Clear preview bitmap
			clear_to_color(m_ScenePreviewBitmap, g_MaskColor);
		}

		// Set the currently selected scene's texts
		m_ScenarioSceneInfoBox->SetVisible(true);
		m_SceneNameLabel->SetText(m_ScenarioSelectedScene->GetPresetName());
		m_SceneInfoLabel->SetText(m_ScenarioSelectedScene->GetDescription());
		// Adjust the height of the text box and container so it fits the text to display
		int newHeight = m_SceneInfoLabel->ResizeHeightToFit();
		m_ScenarioSceneInfoBox->Resize(m_ScenarioSceneInfoBox->GetWidth(), newHeight + 140);
		// Blink the start game button
		m_ScenarioButtons[STARTHERE]->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <");
	} else {
		m_ScenarioSceneInfoBox->SetVisible(false);
		m_SceneNameLabel->SetText("");
		m_SceneInfoLabel->SetText("");
	}

	// Make sure the box doesn't go entirely outside of the screen
	KeepBoxOnScreen(m_ScenarioSceneInfoBox);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdatePlayersBox(bool newActivity) {
	// Get the currently selected Activity
	const Activity *selectedActivity = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : 0;
	const Icon *pIcon = 0;
	bool teamHasPlayers = false;
	bool teamHasHumans = false;
	int teamsWithPlayers = 0;
	int teamsWithHumans = 0;

	if (selectedActivity && m_ScenarioSelectedScene) {
		// Get mouse position and figure out if any cell is being hovered over
		int mouseX;
		int mouseY;
		m_ScenarioGUIInput->GetMousePosition(&mouseX, &mouseY);
		Vector mousePos(static_cast<float>(mouseX),static_cast<float>(mouseY));
		bool menuButtonReleased = g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER);
		const GUICollectionBox *pHoveredCell = dynamic_cast<GUICollectionBox *>(m_ScenarioGUIController->GetControlUnderPoint(mouseX, mouseY, m_ScenarioPlayerSetupBox, 1));

		// Is this a game activity?
		const GameActivity *pGameActivity = dynamic_cast<const GameActivity *>(selectedActivity);

		if (newActivity && pGameActivity) {
			// The pre-set team that should absolutely be CPU played
			m_LockedCPUTeam = pGameActivity->GetCPUTeam();
			// Align the locked CPU team text label with the appropriate row
			if (m_LockedCPUTeam != Activity::NoTeam) {
				m_CPULockLabel->SetPositionAbs(m_CPULockLabel->GetXPos(), m_TeamNameLabels[m_LockedCPUTeam]->GetYPos());
			}
		}

		// Set up the matrix of player control boxes
		for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player) {
			for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
				if (newActivity) {
					// Everyone starts on the Disabled row, except perhaps the CPU which may be on its locked team
					if (team == TEAM_DISABLED) {
						m_PlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
						pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : g_UInputMan.GetSchemeIcon(player);
						if (pIcon) {
							m_PlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
						}
					}
					// De-highlight all other cells initially
					else {
						m_PlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
						m_PlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
					}

					// The CPU gets placed on its locked team
					if (m_LockedCPUTeam != Activity::NoTeam && player == PLAYER_CPU) {
						if (team == m_LockedCPUTeam) {
							m_PlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
							pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
							if (pIcon) {
								m_PlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
							}
						} else {
							m_PlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
							m_PlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
						}
					}
				}

				// Make the hovered cell light up and able to be selected
				if (m_PlayerBoxes[player][team] == pHoveredCell
					// if an active team row, but including the 'not playing' row
					&& (selectedActivity->TeamActive(team) || team == TEAM_DISABLED)
					// That isn't on a team row locked to the CPU
					&& m_LockedCPUTeam != team
					// And not the CPU player if he is locked to a CPU team
					&& (m_LockedCPUTeam == Activity::NoTeam || player != PLAYER_CPU))
					// And a cell not already selected
					//&& m_PlayerBoxes[player][team]->GetDrawType() != GUICollectionBox::Image)
					// And players aren't maxed out for this Activity, or we are removing a player from team assignment
//                    && (player == PLAYER_CPU || team == TEAM_DISABLED || (pGameActivity && PlayerCount() < pGameActivity->GetMaxPlayerSupport())))
				{
					// Is this being pushed and selected?
					if (menuButtonReleased) {
						// Need to clear all other rows of this column
// TODO:  -- unless the CPU column?
						for (int t2 = Activity::TeamOne; t2 < TEAMROWCOUNT; ++t2) {
							// This clicked cell should get the icon of this column
							if (t2 == team) {
								if (player != PLAYER_CPU) {
									m_PlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
									pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : g_UInputMan.GetSchemeIcon(player);
									if (pIcon) {
										m_PlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
									}
								} else {
									//Select or unselect CPU cells
									if (m_PlayerBoxes[player][t2]->GetDrawType() == GUICollectionBox::Image) {
										m_PlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
										m_PlayerBoxes[player][t2]->SetDrawColor(c_GUIColorBlue);
									} else {
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
										if (pIcon) {
											m_PlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
											m_PlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
										}
									}
								}
							}
							// Now unselected columns
							else {
								if (player != PLAYER_CPU) {
									m_PlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
									m_PlayerBoxes[player][t2]->SetDrawColor(c_GUIColorBlue);
								}
							}
						}
						// If CPU changed to an actual team assignment, clear all human players off his new team
						if (player == PLAYER_CPU && team != TEAM_DISABLED) {
							for (int p2 = Players::PlayerOne; p2 < Players::MaxPlayerCount; ++p2) {
								// Deselect the player's team assignment if he's on the same team as the CPU
								if (m_PlayerBoxes[p2][team]->GetDrawType() == GUICollectionBox::Image) {
									m_PlayerBoxes[p2][team]->SetDrawType(GUICollectionBox::Color);
									m_PlayerBoxes[p2][team]->SetDrawColor(c_GUIColorBlue);
									// Move him to disabled
									m_PlayerBoxes[p2][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
									pIcon = g_UInputMan.GetSchemeIcon(p2);
									if (pIcon) {
										m_PlayerBoxes[p2][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
									}
								}
							}
						}
						// If Player clicked CPU disabled button, clear CPU row
						if (player == PLAYER_CPU && team == TEAM_DISABLED) {
							for (int t2 = Activity::TeamOne; t2 <= Activity::TeamFour; ++t2) {
								if (m_PlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image) {
									m_PlayerBoxes[PLAYER_CPU][t2]->SetDrawType(GUICollectionBox::Color);
									m_PlayerBoxes[PLAYER_CPU][t2]->SetDrawColor(c_GUIColorBlue);
								}
							}
						}
						// If a human player changed to a CPU team, remove the CPU guy.
						// Deselect the CPU's team assignment if he's on the same team as the newly assigned human player.
						else if (player != PLAYER_CPU && team != TEAM_DISABLED && m_PlayerBoxes[PLAYER_CPU][team]->GetDrawType() == GUICollectionBox::Image) {
							m_PlayerBoxes[PLAYER_CPU][team]->SetDrawType(GUICollectionBox::Color);
							m_PlayerBoxes[PLAYER_CPU][team]->SetDrawColor(c_GUIColorBlue);
							// Move him to disabled
							//m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
							//pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
							//if (pIcon)
							//    m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
						}
						g_GUISound.FocusChangeSound()->Play();

						//Check if we need to clear or set CPU disabled team icon
						bool noCPUs = true;
						for (int t2 = Activity::TeamOne; t2 <= Activity::TeamFour; ++t2) {
							if (m_PlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image) {
								noCPUs = false;
							}
						}
						//Select or unselect CPU disabled icon
						if (noCPUs) {
							pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
							if (pIcon) {
								m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
								m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
							}
						} else {
							m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Color);
							m_PlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawColor(c_GUIColorBlue);
						}

					}
					// Just highlight the cell
					else if (m_PlayerBoxes[player][team]->GetDrawColor() != c_GUIColorLightBlue) {
						m_PlayerBoxes[player][team]->SetDrawColor(c_GUIColorLightBlue);
						g_GUISound.SelectionChangeSound()->Play();
					}
				}
				// Un-highlight all other cells
				else if (pHoveredCell && m_PlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Color) {
					m_PlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
				}
			}
		}

		// Team info columns
		for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
			// Update the team names and such
			if (newActivity) {
				m_TeamBoxes[team]->SetDrawType(GUICollectionBox::Image);
				/* pointless; the CPU player icon suffices, and doesn't block the real team banner
								// CPU Team
								if (pGameActivity && pGameActivity->GetCPUTeam() == team)
								{
									pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "CPU Team"));
									m_TeamNameLabels[team]->SetText(pActivity->GetTeamName(team) + ":");
								}
								// The not playing row
								else */if (team == TEAM_DISABLED) {
									pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
									m_TeamNameLabels[team]->SetText("Not Playing:");
								}
								// Active player team
								else if (selectedActivity->TeamActive(team)) {
									// Set the team flag icons on the floating player bars
									pIcon = selectedActivity->GetTeamIcon(team);
									// Revert to default if needed
									if (!pIcon) {
										char str[128];
										std::snprintf(str, sizeof(str), "Team %d Default", team + 1);
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", str));
									}
									m_TeamNameLabels[team]->SetText(selectedActivity->GetTeamName(team) + ":");
								}
								// Disabled/unplayable teams
								else {
									pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
									m_TeamNameLabels[team]->SetText("Unavailable");
								}

								// Finally set whatever Icon we came up with
								if (pIcon) {
									m_TeamBoxes[team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
								}
			}

			// Check if the team has any players assigned at all
			if (selectedActivity->TeamActive(team)) {
				teamHasPlayers = false;
				for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player) {
					// CPU is sometimes disabled, but still counts as a team
					if (team != TEAM_DISABLED && m_PlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image) {
						teamHasPlayers = true;
						if (player != PLAYER_CPU) {
							teamHasHumans = true;
						}
					}
				}
				if (teamHasPlayers) {
					++teamsWithPlayers;
				}
				if (teamHasHumans) {
					++teamsWithHumans;
				}

				m_TeamTechSelect[team]->SetEnabled(true);
				m_TeamTechSelect[team]->SetVisible(true);

				m_TeamAISkillSlider[team]->SetEnabled(true);
				m_TeamAISkillSlider[team]->SetVisible(true);
				m_TeamAISkillLabel[team]->SetVisible(true);
			} else {
				if (team >= Activity::TeamOne && team < Activity::MaxTeamCount) {
					m_TeamTechSelect[team]->SetEnabled(false);
					m_TeamTechSelect[team]->SetVisible(false);

					m_TeamAISkillSlider[team]->SetEnabled(false);
					m_TeamAISkillSlider[team]->SetVisible(false);
					m_TeamAISkillLabel[team]->SetVisible(false);
				}
			}
		}

		// If we are over capacity with players, disable the start button and show why
		if (selectedActivity->GetMaxPlayerSupport() < PlayerCount()) {
			m_ScenarioButtons[STARTGAME]->SetVisible(false);
			m_StartErrorLabel->SetVisible(true);
			char str[256];
			std::snprintf(str, sizeof(str), "Too many players assigned! Max for this activity is %d", pGameActivity->GetMaxPlayerSupport());
			m_StartErrorLabel->SetText(str);
		}
		// If we are under the required number of teams with players assigned, disable the start button and show why
		else if (selectedActivity->GetMinTeamsRequired() > teamsWithPlayers) {
			m_ScenarioButtons[STARTGAME]->SetVisible(false);
			m_StartErrorLabel->SetVisible(true);
			char str[256];
			std::snprintf(str, sizeof(str), "Assign players to at\nleast %d of the teams!", pGameActivity->GetMinTeamsRequired());
			m_StartErrorLabel->SetText(str);
		}
		// Assign at least one human player
		else if (teamsWithHumans == 0) {
			m_ScenarioButtons[STARTGAME]->SetVisible(false);
			m_StartErrorLabel->SetVisible(true);
			m_StartErrorLabel->SetText("Assign human players\nto at least one team!");
		}
		// Everything checks out; let the player start if they want to
		else {
			m_ScenarioButtons[STARTGAME]->SetVisible(true);
			m_StartErrorLabel->SetVisible(false);
		}

		// How much starting gold does the slider yield
		char str[256];
		//int startGold = (float)m_GoldSlider->GetMinimum() + ((m_GoldSlider->GetMaximum() - m_GoldSlider->GetMinimum()) * (float)m_GoldSlider->GetValue() / 100.0);
		int startGold = m_GoldSlider->GetValue();
		startGold = startGold - startGold % 500;
		if (m_GoldSlider->GetValue() == m_GoldSlider->GetMaximum()) {
			std::snprintf(str, sizeof(str), "Starting Gold: %c Infinite", -58);
		} else {
			std::snprintf(str, sizeof(str), "Starting Gold: %c %d oz", -58, startGold);
		}
		m_GoldLabel->SetText(str);


		// Set skill labels
		for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++) {
			m_TeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_TeamAISkillSlider[team]->GetValue()));
		}
	}

	// Reset all buttons and positions of things if a new activity has been selected
	if (newActivity) {
		m_ScenarioButtons[STARTGAME]->SetVisible(false);
		m_StartErrorLabel->SetVisible(true);
		m_CPULockLabel->SetVisible(m_LockedCPUTeam != Activity::NoTeam);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ScenarioGUI::PlayerCount() const {
	int count = 0;
	// Go through all the on-team non-CPU cells and see how many players are already assigned.
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team) {
			if (m_PlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image) {
				++count;
			}
		}
	}
	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ScenarioGUI::StartGame() {
	// Get the currently selected Activity
	const Activity *activityPreset = nullptr;
	if (m_ActivitySelectComboBox) {
		activityPreset = m_ActivitySelectComboBox->GetSelectedItem() ? dynamic_cast<const Activity *>(m_ActivitySelectComboBox->GetSelectedItem()->m_pEntity) : 0;
	}

	if (!activityPreset || !m_ScenarioSelectedScene) {
		return false;
	}

	// Create the actual instance of hte Activity we want to start
	Activity *activityInstance = dynamic_cast<Activity *>(activityPreset->Clone());
	GameActivity *gameActivity = dynamic_cast<GameActivity *>(activityInstance);

	// Set up the basic settings
	if (gameActivity) {
		gameActivity->SetDifficulty(m_DifficultySlider->GetValue());

		gameActivity->SetFogOfWarEnabled(m_FogOfWarCheckbox->GetCheck());
		gameActivity->SetRequireClearPathToOrbit(m_RequireClearPathToOrbitCheckbox->GetCheck());


		// If gold slider is at it max value then the amount is 'infinite' and we must set some ridiculously high value.
		if (m_GoldSlider->GetValue() == m_GoldSlider->GetMaximum()) {
			gameActivity->SetStartingGold(1000000);
		} else {
			int startGold = m_GoldSlider->GetValue();
			startGold = startGold - startGold % 500;
			gameActivity->SetStartingGold(startGold);
		}
	}

	g_SceneMan.SetSceneToLoad(m_ScenarioSelectedScene, true, m_DeployUnitsCheckbox->GetCheck());

	// Set up the player and team assignments
	activityInstance->ClearPlayers(false);
	for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player) {
		for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team) {
			if (team != TEAM_DISABLED && m_PlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image) {
				// Add the human players, not including CPU players
				if (player != PLAYER_CPU) {
					activityInstance->AddPlayer(player, true, team, 0);
				}
				// CPU team, so mark it as such.. there are no actual CPU players
				else if (gameActivity) {
					gameActivity->SetCPUTeam(team);
				}
			}
		}
	}

	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team) {
		// Set up techs
		const GUIListPanel::Item *pTechItem = m_TeamTechSelect[team]->GetSelectedItem();
		if (pTechItem) {
			// If the "random" selection, choose one from the list of loaded techs
			if (m_TeamTechSelect[team]->GetSelectedIndex() == 1)//pTechItem->m_ExtraIndex < 0)
			{
				int selection = RandomNum<int>(1, m_TeamTechSelect[team]->GetListPanel()->GetItemList()->size() - 1);
				m_TeamTechSelect[team]->SetSelectedIndex(selection);
				pTechItem = m_TeamTechSelect[team]->GetSelectedItem();

				// Switch back to -Random-
				m_TeamTechSelect[team]->SetSelectedIndex(1);
			}

			// Now set the selected tech's module index as what the metaplayer is going to use
			if (pTechItem) {
				if (pTechItem->m_ExtraIndex == -2) {
					gameActivity->SetTeamTech(team, "-All-");
				} else {
					gameActivity->SetTeamTech(team, g_PresetMan.GetDataModuleName(pTechItem->m_ExtraIndex));
				}
			}
		}

		// Set up AI skill levels
		if (m_TeamAISkillSlider[team]->IsEnabled()) {
			gameActivity->SetTeamAISkill(team, m_TeamAISkillSlider[team]->GetValue());
		} else {
			gameActivity->SetTeamAISkill(team, Activity::DefaultSkill);
		}
	}

	//Force close all previously opened files
	g_LuaMan.FileCloseAll();

	// Put the new and newly set up Activity as the one to start
	g_ActivityMan.SetStartActivity(activityInstance);

	// Kill any Campaign games currently running
	if (g_MetaMan.GameInProgress()) {
		g_MetaMan.EndGame();
	}

	// Signal the start of this Activity we just set up
	return m_ActivityRestarted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::GetAllScenesAndActivities() {
	// Redo the list of Activities
	m_Activities.clear();
	m_ScenarioScenes = 0;

	// Get the list of all read in Scene presets
	list<Entity *> presetList;
	g_PresetMan.GetAllOfType(presetList, "Scene");
	list<Scene *> filteredScenes;

	// Go through the list and cast all the pointers to scenes so we have a handy list
	for (Entity * presetEntity : presetList) {
		Scene *presetScene = dynamic_cast<Scene *>(presetEntity);
		// Only add non-editor and non-special scenes, or ones that don't have locations defined, or have Test in their names, or are metascenes
		if (presetScene && !presetScene->GetLocation().IsZero() && !presetScene->IsMetagameInternal() && (presetScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes())) {
			filteredScenes.push_back(presetScene);
		}
	}

	for (Scene * filteredScene : filteredScenes) {
		filteredScene->SetLocationOffset(Vector(0, 0));
	}

	// We need to calculate planet center manually because m_PlanetCenter reflects coords of moving planet
	// which is outside the screen when this is called first time
	Vector planetCenter = Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
	// Correct planet pos a bit when its location is known.
	if (!m_PlanetCenter.IsZero()) {
		planetCenter = m_PlanetCenter;
	}

	//Move out-of-screen scenes closer to the middle of the planet if we have planet info
	for (Scene *filteredScene : filteredScenes) {
		float y = planetCenter.GetY() + filteredScene->GetLocation().GetY();

		// Do not touch scenes outside the planet, they might be hidden intentionally
		if (abs(filteredScene->GetLocation().GetY()) < m_PlanetRadius + 100 && abs(filteredScene->GetLocation().GetX()) < m_PlanetRadius + 100) {
			if (y < 10) {
				filteredScene->SetLocationOffset(Vector(0, -y + 14));
			}

			if (y > g_FrameMan.GetResY() - 10) {
				filteredScene->SetLocationOffset(Vector(0, -(y - g_FrameMan.GetResY() + 14)));
			}
		}
	}

	// Add offsets to reveal overlapping scenes if any
	for (Scene *filteredScene1 : filteredScenes) {
		bool isOverlapped = false;

		do {
			isOverlapped = false;

			// Find overlapping scene dot
			for (const Scene * filteredScene2 : filteredScenes) {
				if (filteredScene1 != filteredScene2) {
					Vector pos1 = filteredScene1->GetLocation() + filteredScene1->GetLocationOffset();
					Vector pos2 = filteredScene2->GetLocation() + filteredScene2->GetLocationOffset();

					if ((pos1 - pos2).GetMagnitude() < 8) {
						isOverlapped = true;
						break;
					}
				}
			}

			// Move the dot closer to the planet center
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

	// Get the list of all read-in Activity presets
	presetList.clear();
	g_PresetMan.GetAllOfType(presetList, "Activity");
	Activity *presetActivity = nullptr;

	int selectedActivityIndex = m_ActivitySelectComboBox->GetSelectedIndex();

	// Go through the list and cast all the pointers to Activities so we have a handy list
	// Associate all Scenes compatible with each Activity
	// Populate the activities selection dropdown while we're at it
	m_ActivitySelectComboBox->ClearList();
	int index = 0;
	int tutorialIndex = -1;
	for (Entity *presetEntity : presetList) {
		presetActivity = dynamic_cast<Activity *>(presetEntity);
		// Only add non-editor and non-special activities
		if (presetActivity/* && pActivity->GetClassName() != "GATutorial" */ && presetActivity->GetClassName().find("Editor") == string::npos) {
			// Prepare a new entry in the list of Activity:ies that we have
			pair<Activity *, list<Scene *> > newPair(presetActivity, list<Scene *>());
			for (Scene *filteredScene : filteredScenes) {
				// Check if the Scene has the required Areas and such needed for this Activity.
				if (presetActivity->SceneIsCompatible(filteredScene)) {
					newPair.second.push_back(filteredScene);
				}
			}

			m_Activities.insert(newPair);
			// Add to the activity selection combo, and attach the activity copy, not passing in ownership
			m_ActivitySelectComboBox->AddItem(presetActivity->GetPresetName(), "", 0, presetActivity);

			// Save the tutorial mission so we can select it by default
			if (presetActivity->GetClassName() == "GATutorial") {
				tutorialIndex = index;
			}
			index++;
		}
	}

	// Select the Tutorial Activity and Scene by default to start
	if (tutorialIndex >= 0) {
		if (m_SelectTutorial) {
			m_ActivitySelectComboBox->SetSelectedIndex(tutorialIndex);
		} else {
			m_ActivitySelectComboBox->SetSelectedIndex(selectedActivityIndex);
		}
		UpdateActivityBox();
		m_ScenarioSelectedScene = m_ScenarioScenes ? m_ScenarioScenes->front() : 0;
		UpdateScenesBox();
		// Switch to tutorial just once
		m_SelectTutorial = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::UpdateSiteNameLabel(bool visible, const string &text, const Vector &location, float height) {
	// Set up the hover label to appear over any hovered scene location
	m_ScenarioScenePlanetLabel->SetVisible(visible);
	if (visible) {
		m_ScenarioScenePlanetLabel->SetText(text);
		m_ScenarioScenePlanetLabel->SetPositionAbs(m_PlanetCenter.m_X + location.m_X - (m_ScenarioScenePlanetLabel->GetWidth() / 2),
			m_PlanetCenter.m_Y + location.m_Y - (m_ScenarioScenePlanetLabel->GetHeight() * 1.5 * height));

		// Clamp it to within the screen.. only Y applies to the label though
		int pad = 6;
		/*
				if (m_ScenarioScenePlanetLabel->GetXPos() < pad)
					m_ScenarioScenePlanetLabel->SetPositionAbs(pad, m_ScenarioScenePlanetLabel->GetYPos());
				else if (m_ScenarioScenePlanetLabel->GetXPos() + m_ScenarioScenePlanetLabel->GetWidth() + pad >= g_FrameMan.GetResX())
					m_ScenarioScenePlanetLabel->SetPositionAbs(g_FrameMan.GetResX() - m_ScenarioScenePlanetLabel->GetWidth() - pad, m_ScenarioScenePlanetLabel->GetYPos());
		*/
		if (m_ScenarioScenePlanetLabel->GetYPos() < pad) {
			m_ScenarioScenePlanetLabel->SetPositionAbs(m_ScenarioScenePlanetLabel->GetXPos(), pad);
		}
		else if (m_ScenarioScenePlanetLabel->GetYPos() + m_ScenarioScenePlanetLabel->GetHeight() + pad >= g_FrameMan.GetResY()) {
			m_ScenarioScenePlanetLabel->SetPositionAbs(m_ScenarioScenePlanetLabel->GetXPos(), g_FrameMan.GetResY() - m_ScenarioScenePlanetLabel->GetHeight() - pad);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ScenarioGUI::DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const {
	int blendAmount = 210 + RandomNum(-15, 15);
	set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
	line(drawBitmap, start.GetFloorIntX(), start.GetFloorIntY(), end.GetFloorIntX(), end.GetFloorIntY(), color);
	/* Looks like ass
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

bool ScenarioGUI::DrawScreenLineToSitePoint(BITMAP *drawBitmap,
	const Vector &screenPoint,
	const Vector &planetPoint,
	int color,
	int onlyFirstSegments,
	int onlyLastSegments,
	int channelHeight,
	float circleSize,
	bool squareSite) const {
	// No part of the line is visible with these params, so just quit
	if (onlyFirstSegments == 0 || onlyLastSegments == 0) {
		return false;
	}
	// Detect disabling of the segment controls
	if (onlyFirstSegments < 0) {
		onlyFirstSegments = 100;
	}
	if (onlyLastSegments < 0) {
		onlyLastSegments = 100;
	}

	int totalSegments = 0;
	int drawnFirstSegments = 0;
	int lastSegmentsToDraw = 0;
	int circleRadius = squareSite ? static_cast<int>(6.0F * circleSize) : static_cast<int>(8.0F * circleSize);
	int chamferSize = CHAMFERSIZE;
	Vector chamferPoint1;
	Vector chamferPoint2;
	Vector sitePos = m_PlanetCenter + planetPoint;
	bool siteIsAbove = sitePos.GetFloorIntY() < screenPoint.GetFloorIntY();
	float yDirMult = siteIsAbove ? -1.0 : 1.0;
	bool twoBends = std::fabs(sitePos.GetFloorIntY() - screenPoint.GetFloorIntY()) < (channelHeight - circleRadius);
	bool noBends = std::fabs(sitePos.GetFloorIntX() - screenPoint.GetFloorIntX()) < circleRadius;// && ((m_apPlayerBox[player]->GetWidth() * meterAmount * 0.5) >= fabs(sitePos.GetFloorIntX() - screenPoint.GetFloorIntX()));
	Vector firstBend(screenPoint.m_X, twoBends ? (screenPoint.m_Y + channelHeight * yDirMult) : sitePos.m_Y);
	Vector secondBend(sitePos.m_X, firstBend.m_Y);
	bool siteIsLeft = sitePos.m_X < screenPoint.m_X;
	float xDirMult = siteIsLeft ? -1.0 : 1.0;

	// No bends, meaning the mid of the meter goes straight up/down into the site circle
	if (noBends) {
		// How many possible segments there are total for this type of line: to site + circle
		totalSegments = lastSegmentsToDraw = 1 + 1;
		// Draw the line to the site
		if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, screenPoint + Vector(sitePos.m_X - screenPoint.m_X, 0), sitePos + Vector(0, (circleRadius + 1) * -yDirMult), color);
		}
	}
	// Extra lines depending on whether there needs to be two bends due to the site being in the 'channel', ie next to the floating player bar
	else if (twoBends) {
		// Cap the chamfer size on the second bend appropriately
		chamferSize = std::min(static_cast<int>((firstBend - secondBend).GetMagnitude() - 15), chamferSize);
		chamferSize = std::min(static_cast<int>((secondBend - sitePos).GetMagnitude() - circleRadius * 3), chamferSize);
		// Snap the chamfer to not exist below a minimum size
		chamferSize = (chamferSize < 15) ? 0 : chamferSize;
		// No inverted chamfer
		chamferSize = std::max(0, chamferSize);
		chamferPoint1.SetXY(secondBend.m_X + chamferSize * -xDirMult, secondBend.m_Y);
		chamferPoint2.SetXY(secondBend.m_X, secondBend.m_Y + chamferSize * -yDirMult);
		// How many of the last segments to draw: to first bend + to second bend chamfer + chamfer + to site + circle
		totalSegments = lastSegmentsToDraw = 1 + 1 + (int)(chamferSize > 0) + 1 + 1;
		// Line to the first bend
		drawnFirstSegments++;
		lastSegmentsToDraw--;
		if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, screenPoint, firstBend, color);
		}
		// Line to the second bend, including the chamfer
		drawnFirstSegments++;
		lastSegmentsToDraw--;
		if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, firstBend, chamferPoint1, color);
		}
		if (chamferSize > 0) {
			drawnFirstSegments++;
			lastSegmentsToDraw--;
			if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
				DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
			}
		}
		// Line to the site
		drawnFirstSegments++;
		lastSegmentsToDraw--;
		if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector(0, (circleRadius + 1) * yDirMult), color);
		}
	}
	// Just one bend
	else {
		// Cap the chamfer size on the first bend appropriately
		chamferSize = std::min(static_cast<int>((screenPoint - firstBend).GetMagnitude() - 15), chamferSize);
		chamferSize = std::min(static_cast<int>((firstBend - sitePos).GetMagnitude() - circleRadius * 3), chamferSize);
		// Snap the chamfer to not exist below a minimum size
		chamferSize = (chamferSize < 15) ? 0 : chamferSize;
		// No inverted chamfer
		chamferSize = std::max(0, chamferSize);
		chamferPoint1.SetXY(screenPoint.m_X, firstBend.m_Y + chamferSize * -yDirMult);
		chamferPoint2.SetXY(firstBend.m_X + chamferSize * xDirMult, sitePos.m_Y);
		// How many of the last segments to draw: to first bend chamfer + chamfer + to site + circle
		totalSegments = lastSegmentsToDraw = 1 + (int)(chamferSize > 0) + 1 + 1;
		// Draw line to the first bend, including the chamfer
		drawnFirstSegments++;
		lastSegmentsToDraw--;
		if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, screenPoint, chamferPoint1, color);
		}
		if (chamferSize > 0) {
			drawnFirstSegments++;
			lastSegmentsToDraw--;
			if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
				DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
			}
		}
		// Draw line to the site
		drawnFirstSegments++;
		lastSegmentsToDraw--;
		if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
			DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector((circleRadius + 1) * -xDirMult, 0), color);
		}
	}

	// Draw a circle around the site target
	drawnFirstSegments++;
	lastSegmentsToDraw--;
	if (!(drawnFirstSegments >= onlyFirstSegments || lastSegmentsToDraw > onlyLastSegments)) {
		int blendAmount = 225 + RandomNum(-20, 20);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

		// If specified, draw a squareSite instead (with chamfered corners)
		if (squareSite) {
			hline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1, sitePos.GetFloorIntY() - circleRadius - 1, sitePos.GetFloorIntX() + circleRadius, color);
			hline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1, sitePos.GetFloorIntY() - circleRadius - 1 - 1, sitePos.GetFloorIntX() + circleRadius, color);
			hline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1, sitePos.GetFloorIntY() + circleRadius, sitePos.GetFloorIntX() + circleRadius, color);
			hline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1, sitePos.GetFloorIntY() + circleRadius + 1, sitePos.GetFloorIntX() + circleRadius, color);
			vline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1, sitePos.GetFloorIntY() - circleRadius - 1, sitePos.GetFloorIntY() + circleRadius, color);
			vline(drawBitmap, sitePos.GetFloorIntX() - circleRadius - 1 - 1, sitePos.GetFloorIntY() - circleRadius - 1, sitePos.GetFloorIntY() + circleRadius, color);
			vline(drawBitmap, sitePos.GetFloorIntX() + circleRadius, sitePos.GetFloorIntY() + circleRadius, sitePos.GetFloorIntY() - circleRadius - 1, color);
			vline(drawBitmap, sitePos.GetFloorIntX() + circleRadius + 1, sitePos.GetFloorIntY() + circleRadius, sitePos.GetFloorIntY() - circleRadius - 1, color);
		} else {
			circle(drawBitmap, sitePos.GetFloorIntX(), sitePos.GetFloorIntY(), circleRadius, color);
			circle(drawBitmap, sitePos.GetFloorIntX(), sitePos.GetFloorIntY(), circleRadius - 1, color);
		}
	}

	return totalSegments <= onlyFirstSegments && totalSegments <= onlyLastSegments;
}
