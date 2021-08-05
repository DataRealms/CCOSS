//////////////////////////////////////////////////////////////////////////////////////////
// File:            MultiplayerServerLobby.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Project:         Retro Terrain Engine
// Author(s):       


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MultiplayerServerLobby.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"
#include "MetaMan.h"
#include "AudioMan.h"

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
#include "PieMenuGUI.h"

#include "NetworkServer.h"

namespace RTE {

	ConcreteClassInfo(MultiplayerServerLobby, Activity, 0)

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this MultiplayerServerLobby, effectively
	//                  resetting the members of this abstraction level only.

	void MultiplayerServerLobby::Clear()
	{
		//m_pEditorGUI = 0;
		m_pGUIController = 0;
		m_pGUIInput = 0;
		m_pGUIScreen = 0;

		m_pCursor = 0;

		m_pRootBox = 0;
		m_pPlayerSetupBox = 0;

		m_pActivitySelect = 0;
		m_pDifficultyLabel = 0;
		m_pDifficultySlider = 0;

		for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
		{
			for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
			{
				//            m_aaControls = team == TEAM_DISABLED;
				m_aapPlayerBoxes[player][team] = 0;
			}
		}

		for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
		{
			m_apTeamBoxes[team] = 0;
			m_apTeamNameLabels[team] = 0;
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
		{
			m_apTeamTechSelect[team] = 0;
			m_apTeamAISkillSlider[team] = 0;
			m_apTeamAISkillLabel[team] = 0;
		}

		m_pStartErrorLabel = 0;
		m_pCPULockLabel = 0;
		m_LockedCPUTeam = Teams::NoTeam;

		m_pGoldLabel = 0;
		m_pGoldSlider = 0;
		m_pFogOfWarCheckbox = 0;
		m_pRequireClearPathToOrbitCheckbox = 0;
		m_pDeployUnitsCheckbox = 0;

		m_pScenes = 0;
		m_Activities.clear();
		m_pSelectedScene = 0;

		m_pScenePreviewBitmap = 0;
		m_pDefaultPreviewBitmap = 0;

		m_pUIDrawBitmap = 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the MultiplayerServerLobby object ready for use.

	int MultiplayerServerLobby::Create()
	{
		if (Activity::Create() < 0)
			return -1;

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a MultiplayerServerLobby to be identical to another, by deep copy.

	int MultiplayerServerLobby::Create(const MultiplayerServerLobby &reference)
	{
		if (Activity::Create(reference) < 0)
			return -1;

		if (m_Description.empty())
			m_Description = "Edit this Scene, including placement of all terrain objects and movable objects, AI blueprints, etc.";

		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  ReadProperty
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Reads a property value from a reader stream. If the name isn't
	//                  recognized by this class, then ReadProperty of the parent class
	//                  is called. If the property isn't recognized by any of the base classes,
	//                  false is returned, and the reader's position is untouched.

	int MultiplayerServerLobby::ReadProperty(const std::string_view &propName, Reader &reader)
	{
		return Activity::ReadProperty(propName, reader);
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Save
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Saves the complete state of this MultiplayerServerLobby with a Writer for
	//                  later recreation with Create(Reader &reader);

	int MultiplayerServerLobby::Save(Writer &writer) const
	{
		Activity::Save(writer);
		return 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Destroy
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destroys and resets (through Clear()) the MultiplayerServerLobby object.

	void MultiplayerServerLobby::Destroy(bool notInherited)
	{
		delete m_pGUIController;
		delete m_pGUIInput;
		delete m_pGUIScreen;

		if (m_pScenePreviewBitmap)
			destroy_bitmap(m_pScenePreviewBitmap);
		if (m_pDefaultPreviewBitmap)
			destroy_bitmap(m_pDefaultPreviewBitmap);
		if (m_pUIDrawBitmap)
			destroy_bitmap(m_pUIDrawBitmap);

		if (!notInherited)
			Activity::Destroy();
		Clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Start
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Officially starts this. Creates all the data etc necessary to start
	//                  the activity.

	int MultiplayerServerLobby::Start()
	{
		int error = GameActivity::Start();

		g_AudioMan.ClearMusicQueue();
		g_AudioMan.StopMusic();

		SetPresetName("Multiplayer Lobby");

		//////////////////////////////////////////////
		// Allocate and (re)create the Editor GUI

		/*if (m_pEditorGUI)
			m_pEditorGUI->Destroy();
		else
			m_pEditorGUI = new MultiplayerServerLobbyGUI;
		m_pEditorGUI->Create(&(m_PlayerController[0]));*/

		//////////////////////////////////////////////////////////////
		// Hooking up directly to the controls defined in the GUI ini
		if (!m_pGUIScreen)
			m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
		if (!m_pGUIInput)
			m_pGUIInput = new AllegroInput(-1, true);
		if (!m_pGUIController)
			m_pGUIController = new GUIControlManager();
		if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins", "DefaultSkin.ini")) {
			RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/DefaultSkin.ini");
		}

		m_pGUIController->Load("Base.rte/GUIs/MultiplayerServerLobbyGUI.ini");
		m_pGUIController->EnableMouse(true);

		// Resize the invisible root container so it matches the screen rez
		m_pRootBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
		m_pPlayerSetupBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PlayerSetupBox"));


		// Activity Selection Box
		m_pActivitySelect = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ActivitySelectCombo"));
		m_pSceneSelect = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("SceneSelectCombo"));
		m_pDifficultyLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("DifficultyLabel"));
		m_pDifficultySlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("DifficultySlider"));
		//m_pActivitySelect->SetDropHeight(64);
		//    m_pActivitySelect->GetListPanel()->SetFont(m_pGUIController->GetSkin()->GetFont("FontSmall.png"));
		//m_pActivityLabel->SetFont(m_pGUIController->GetSkin()->GetFont("FontSmall.png"));

		// Player team assignment box
		char str[128];
		for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
		{
			for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
			{
				// +1 because the controls are indexed starting at 1, not 0
				std::snprintf(str, sizeof(str), "P%dT%dBox", player + 1, team + 1);
				m_aapPlayerBoxes[player][team] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl(str));
			}
		}
		m_apTeamBoxes[TEAM_DISABLED] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("TDIcon"));
		m_apTeamBoxes[Teams::TeamOne] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T1Icon"));
		m_apTeamBoxes[Teams::TeamTwo] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T2Icon"));
		m_apTeamBoxes[Teams::TeamThree] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T3Icon"));
		m_apTeamBoxes[Teams::TeamFour] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T4Icon"));
		m_apTeamNameLabels[TEAM_DISABLED] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("TDLabel"));
		m_apTeamNameLabels[Teams::TeamOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T1Label"));
		m_apTeamNameLabels[Teams::TeamTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T2Label"));
		m_apTeamNameLabels[Teams::TeamThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T3Label"));
		m_apTeamNameLabels[Teams::TeamFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T4Label"));
		m_apTeamTechSelect[Teams::TeamOne] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T1TechCombo"));
		m_apTeamTechSelect[Teams::TeamTwo] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T2TechCombo"));
		m_apTeamTechSelect[Teams::TeamThree] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T3TechCombo"));
		m_apTeamTechSelect[Teams::TeamFour] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T4TechCombo"));
		m_apTeamAISkillSlider[Teams::TeamOne] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T1AISkillSlider"));
		m_apTeamAISkillSlider[Teams::TeamTwo] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T2AISkillSlider"));
		m_apTeamAISkillSlider[Teams::TeamThree] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T3AISkillSlider"));
		m_apTeamAISkillSlider[Teams::TeamFour] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T4AISkillSlider"));
		m_apTeamAISkillLabel[Teams::TeamOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T1AISkillLabel"));
		m_apTeamAISkillLabel[Teams::TeamTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T2AISkillLabel"));
		m_apTeamAISkillLabel[Teams::TeamThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T3AISkillLabel"));
		m_apTeamAISkillLabel[Teams::TeamFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T4AISkillLabel"));


		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++)
		{
			// Hide tech combobxes by default
			m_apTeamTechSelect[team]->SetEnabled(false);
			m_apTeamTechSelect[team]->SetVisible(false);
			// Add a "Random" tech choice to all the combo boxes first so that's at least in there always
			m_apTeamTechSelect[team]->GetListPanel()->AddItem("-All-", "", 0, 0, -2);
			m_apTeamTechSelect[team]->GetListPanel()->AddItem("-Random-", "", 0, 0, -1);
			m_apTeamTechSelect[team]->SetSelectedIndex(0);
			// Hide AIs skill combobxes by default
			m_apTeamAISkillSlider[team]->SetEnabled(false);
			m_apTeamAISkillSlider[team]->SetVisible(false);
			m_apTeamAISkillSlider[team]->SetValue(AISkillSetting::DefaultSkill);

			m_apTeamAISkillLabel[team]->SetEnabled(false);
			m_apTeamAISkillLabel[team]->SetVisible(false);
			m_apTeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_apTeamAISkillSlider[team]->GetValue()));
		}
		m_pStartErrorLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("StartErrorLabel"));
		m_pCPULockLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CPULockLabel"));

		// Populate the tech comboboxes with the available tech modules
		for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
			if (const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID)) {
				if (dataModule->IsFaction()) {
					for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
						m_apTeamTechSelect[team]->GetListPanel()->AddItem(dataModule->GetFriendlyName(), "", nullptr, nullptr, moduleID);
						m_apTeamTechSelect[team]->GetListPanel()->ScrollToTop();
					}
				}
			}
		}

		m_pGoldLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("GoldLabel"));
		m_pGoldSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("GoldSlider"));
		m_pFogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("FogOfWarCheckbox"));
		m_pRequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
		m_pDeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("DeployUnitsCheckbox"));

		m_pStartScenarioButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("StartButton"));

		// TODO: Use old dimensions because don't feel like redesigning this whole GUI. Deal with this eventually.
		m_pScenePreviewBitmap = create_bitmap_ex(8, 140, 55);
		//m_pScenePreviewBitmap = create_bitmap_ex(8, c_ScenePreviewWidth, c_ScenePreviewHeight);

		ContentFile defaultPreview("Base.rte/GUIs/DefaultPreview000.png");
		m_pDefaultPreviewBitmap = defaultPreview.GetAsBitmap(COLORCONV_NONE, false);

		clear_to_color(m_pScenePreviewBitmap, g_MaskColor);

		m_apPlayerIcons[0] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 1"));
		m_apPlayerIcons[1] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 2"));
		m_apPlayerIcons[2] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 3"));
		m_apPlayerIcons[3] = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad 4"));

		m_apPlayerNameLabel[0] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("Player1NameLabel"));
		m_apPlayerNameLabel[1] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("Player2NameLabel"));
		m_apPlayerNameLabel[2] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("Player3NameLabel"));
		m_apPlayerNameLabel[3] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("Player4NameLabel"));

		if (!m_pCursor)
		{
			ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
			m_pCursor = cursorFile.GetAsBitmap();
		}

		m_pUIDrawBitmap = create_bitmap_ex(8, m_pRootBox->GetWidth(), m_pRootBox->GetHeight());

		GetAllScenesAndActivities();

		UpdateActivityBox();

		// This allow to reduce looby bandwidth by 50% while still somewhat descent looking
		g_NetworkServer.SetInterlacingMode(true);

		return error;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          UpdateActivityBox
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the contents of the Activity selection box.

	void MultiplayerServerLobby::UpdateActivityBox()
	{
		// Get the currently selected Activity
		const Activity *pSelected = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;

		if (pSelected)
		{
			m_pSceneSelect->ClearList();

			// Pull out the list of Scenes that are compatible with this Activity
			map<Activity *, list<Scene *> >::iterator asItr;
			if (m_Activities.end() != (asItr = m_Activities.find(const_cast<Activity *>(pSelected))))
			{
				m_pScenes = &((*asItr).second);

				// Fill scenes combo with compatible scenes
				for (list<Scene *>::iterator pItr = m_pScenes->begin(); pItr != m_pScenes->end(); ++pItr)
				{
					Scene *pScene = (*pItr);
					m_pSceneSelect->AddItem(pScene->GetPresetName(), "", 0, pScene);
				}

				if (m_pSceneSelect->GetCount() > 0)
					m_pSceneSelect->SetSelectedIndex(0);
				m_pSelectedScene = m_pSceneSelect->GetSelectedItem() ? dynamic_cast<const Scene *>(m_pSceneSelect->GetSelectedItem()->m_pEntity) : 0;
			}
			else
			{
				m_pSelectedScene = 0;
				m_pScenes = 0;
			}

			// Resize the box to fit the desc
			//m_pActivityBox->Resize(m_pActivityBox->GetWidth(), newHeight + 110);
			//UpdateScenesBox();

			const GameActivity * pSelectedGA = dynamic_cast<const GameActivity *>(pSelected);
			if (pSelectedGA)
			{
				UpdateGoldSlider(pSelectedGA);
				UpdateDifficultySlider();

				//Set default fog of war flag and enable or disable it if necessary
				if (pSelectedGA->GetDefaultFogOfWar() > -1)
				{
					if (pSelectedGA->GetDefaultFogOfWar() == 0)
						m_pFogOfWarCheckbox->SetCheck(0);
					else
						m_pFogOfWarCheckbox->SetCheck(1);
				}
				m_pFogOfWarCheckbox->SetEnabled(pSelectedGA->GetFogOfWarSwitchEnabled());

				//Set default clear path to orbit flag and enable or disable it if necessary
				if (pSelectedGA->GetDefaultRequireClearPathToOrbit() > -1)
				{
					if (pSelectedGA->GetDefaultRequireClearPathToOrbit() == 0)
						m_pRequireClearPathToOrbitCheckbox->SetCheck(0);
					else
						m_pRequireClearPathToOrbitCheckbox->SetCheck(1);
				}
				m_pRequireClearPathToOrbitCheckbox->SetEnabled(pSelectedGA->GetRequireClearPathToOrbitSwitchEnabled());

				//Set default deploy units flag and enable or disable it if necessary
				if (pSelectedGA->GetDefaultDeployUnits() > -1)
				{
					if (pSelectedGA->GetDefaultDeployUnits() == 0)
						m_pDeployUnitsCheckbox->SetCheck(0);
					else
						m_pDeployUnitsCheckbox->SetCheck(1);
				}
				m_pDeployUnitsCheckbox->SetEnabled(pSelectedGA->GetDeployUnitsSwitchEnabled());
			}
			UpdatePlayersBox(true);
		}
	}

	void MultiplayerServerLobby::UpdateGoldSlider(const GameActivity * pSelectedGA)
	{
		if (!pSelectedGA)
			return;

		// Set gold slider value if activity sepcifies default gold amounts for difficulties
		if (m_pDifficultySlider->GetValue() < DifficultySetting::CakeDifficulty)
		{
			if (pSelectedGA->GetDefaultGoldCake() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldCake());
		}
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::EasyDifficulty)
		{
			if (pSelectedGA->GetDefaultGoldEasy() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldEasy());
		}
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::MediumDifficulty)
		{
			if (pSelectedGA->GetDefaultGoldMedium() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldMedium());
		}
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::HardDifficulty)
		{
			if (pSelectedGA->GetDefaultGoldHard() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldHard());
		}
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::NutsDifficulty)
		{
			if (pSelectedGA->GetDefaultGoldNuts() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldNuts());
		}
		else
		{
			if (pSelectedGA->GetDefaultGoldNuts() > -1)
				m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldNuts());
		}
		m_pGoldSlider->SetEnabled(pSelectedGA->GetGoldSwitchEnabled());
	}

	void MultiplayerServerLobby::UpdateDifficultySlider()
	{
		// Set the description
		if (m_pDifficultySlider->GetValue() < DifficultySetting::CakeDifficulty)
			m_pDifficultyLabel->SetText("Difficulty: Cake");
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::EasyDifficulty)
			m_pDifficultyLabel->SetText("Difficulty: Easy");
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::MediumDifficulty)
			m_pDifficultyLabel->SetText("Difficulty: Medium");
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::HardDifficulty)
			m_pDifficultyLabel->SetText("Difficulty: Hard");
		else if (m_pDifficultySlider->GetValue() < DifficultySetting::NutsDifficulty)
			m_pDifficultyLabel->SetText("Difficulty: Nuts");
		else
			m_pDifficultyLabel->SetText("Difficulty: Nuts!");
	}


	void MultiplayerServerLobby::UpdateSkillSlider()
	{

	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          UpdatePlayersBox
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the contents of the player config box.

	void MultiplayerServerLobby::UpdatePlayersBox(bool newActivity)
	{
		// Get the currently selected Activity
		const Activity *pActivity = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
		const Icon *pIcon = 0;
		bool teamHasPlayers = false;
		bool teamHasHumans = false;
		int teamsWithPlayers = 0;
		int teamsWithHumans = 0;

		if (pActivity && m_pSelectedScene)
		{
			// Get mouse position and figure out if any cell is being hovered over
			int mouseX, mouseY;
			m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
			Vector mousePos(mouseX, mouseY);
			bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
			bool menuButtonReleased = g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER);
			GUICollectionBox *pHoveredCell = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControlUnderPoint(mouseX, mouseY, m_pPlayerSetupBox, 1));

			// Is this a game activity?
			const GameActivity *pGameActivity = dynamic_cast<const GameActivity *>(pActivity);

			if (newActivity && pGameActivity)
			{
				// The pre-set team that should absolutely be CPU played
				m_LockedCPUTeam = pGameActivity->GetCPUTeam();
				// Align the locked CPU team text label with the appropriate row
				if (m_LockedCPUTeam != Teams::NoTeam)
					m_pCPULockLabel->SetPositionAbs(m_pCPULockLabel->GetXPos(), m_apTeamNameLabels[m_LockedCPUTeam]->GetYPos());
			}

			// Set up the matrix of player control boxes
			for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
			{
				for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
				{
					if (newActivity)
					{
						// Everyone starts on the Disabled row, except perhaps the CPU which may be on its locked team
						if (team == TEAM_DISABLED)
						{
							m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
							pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : m_apPlayerIcons[player];// g_UInputMan.GetSchemeIcon(player);
							if (pIcon)
								m_aapPlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
						}
						// De-highlight all other cells initially
						else
						{
							m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
							m_aapPlayerBoxes[player][team]->SetDrawColor(c_PlayerSlotColorDefault);
						}

						// The CPU gets placed on its locked team
						if (m_LockedCPUTeam != Teams::NoTeam && player == PLAYER_CPU)
						{
							if (team == m_LockedCPUTeam)
							{
								m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
								pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
								if (pIcon)
									m_aapPlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
							}
							else
							{
								m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
								m_aapPlayerBoxes[player][team]->SetDrawColor(c_PlayerSlotColorDefault);
							}
						}
					}

					// Make the hovered cell light up and able to be selected
					if (m_aapPlayerBoxes[player][team] == pHoveredCell
						// if an active team row, but including the 'not playing' row
						&& (pActivity->TeamActive(team) || team == TEAM_DISABLED)
						// That isn't on a team row locked to the CPU
						&& m_LockedCPUTeam != team
						// And not the CPU player if he is locked to a CPU team
						&& (m_LockedCPUTeam == Teams::NoTeam || player != PLAYER_CPU))
						// And a cell not already selected
						//&& m_aapPlayerBoxes[player][team]->GetDrawType() != GUICollectionBox::Image)
						// And players aren't maxed out for this Activity, or we are removing a player from team assignment
	//                    && (player == PLAYER_CPU || team == TEAM_DISABLED || (pGameActivity && PlayerCount() < pGameActivity->GetMaxPlayerSupport())))
					{
						// Is this being pushed and selected?
						if (menuButtonReleased)
						{
							// Need to clear all other rows of this column
	// TODO:  -- unless the CPU column?
							for (int t2 = Teams::TeamOne; t2 < TEAMROWCOUNT; ++t2)
							{
								// This clicked cell should get the icon of this column
								if (t2 == team)
								{
									if (player != PLAYER_CPU)
									{
										m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
										pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : m_apPlayerIcons[player]; //g_UInputMan.GetSchemeIcon(player);
										if (pIcon)
											m_aapPlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
									}
									else {
										//Select or unselect CPU cells
										if (m_aapPlayerBoxes[player][t2]->GetDrawType() == GUICollectionBox::Image)
										{
											m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
											m_aapPlayerBoxes[player][t2]->SetDrawColor(c_PlayerSlotColorDefault);
										}
										else {
											pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
											if (pIcon)
											{
												m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
												m_aapPlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
											}
										}
									}
								}
								// Now unselected columns
								else
								{
									if (player != PLAYER_CPU)
									{
										m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
										m_aapPlayerBoxes[player][t2]->SetDrawColor(c_PlayerSlotColorDefault);
									}
								}
							}
							// If CPU changed to an actual team assignment, clear all human players off his new team
							if (player == PLAYER_CPU && team != TEAM_DISABLED)
							{
								for (int p2 = Players::PlayerOne; p2 < Players::MaxPlayerCount; ++p2)
								{
									// Deselect the player's team assignment if he's on the same team as the CPU
									if (m_aapPlayerBoxes[p2][team]->GetDrawType() == GUICollectionBox::Image)
									{
										m_aapPlayerBoxes[p2][team]->SetDrawType(GUICollectionBox::Color);
										m_aapPlayerBoxes[p2][team]->SetDrawColor(c_PlayerSlotColorDefault);
										// Move him to disabled
										m_aapPlayerBoxes[p2][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
										pIcon = g_UInputMan.GetSchemeIcon(p2);
										if (pIcon)
											m_aapPlayerBoxes[p2][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
									}
								}
							}
							// If Player clicked CPU disabled button, clear CPU row
							if (player == PLAYER_CPU && team == TEAM_DISABLED)
							{
								for (int t2 = Teams::TeamOne; t2 <= Teams::TeamFour; ++t2)
								{
									if (m_aapPlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image)
									{
										m_aapPlayerBoxes[PLAYER_CPU][t2]->SetDrawType(GUICollectionBox::Color);
										m_aapPlayerBoxes[PLAYER_CPU][t2]->SetDrawColor(c_PlayerSlotColorDefault);
									}
								}
							}
							// If a human player changed to a CPU team, remove the CPU guy
							else if (player != PLAYER_CPU && team != TEAM_DISABLED)
							{
								// Deselect the CPU's team assignment if he's on the same team as the newly assigned human player
								if (m_aapPlayerBoxes[PLAYER_CPU][team]->GetDrawType() == GUICollectionBox::Image)
								{
									m_aapPlayerBoxes[PLAYER_CPU][team]->SetDrawType(GUICollectionBox::Color);
									m_aapPlayerBoxes[PLAYER_CPU][team]->SetDrawColor(c_PlayerSlotColorDefault);
									// Move him to disabled
									//m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
									//pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
									//if (pIcon)
									//    m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
								}
							}
							//g_GUISound.FocusChangeSound()->Play();

							//Check if we need to clear or set CPU disabled team icon
							bool noCPUs = true;
							for (int t2 = Teams::TeamOne; t2 <= Teams::TeamFour; ++t2)
							{
								if (m_aapPlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image)
									noCPUs = false;
							}
							//Select or unselect CPU disabled icon
							if (noCPUs)
							{
								pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
								if (pIcon)
								{
									m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
									m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
								}
							}
							else {
								m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Color);
								m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawColor(c_PlayerSlotColorDefault);
							}

						}
						// Just highlight the cell
						else if (m_aapPlayerBoxes[player][team]->GetDrawColor() != c_PlayerSlotColorHovered)
						{
							m_aapPlayerBoxes[player][team]->SetDrawColor(c_PlayerSlotColorHovered);
							//g_GUISound.SelectionChangeSound()->Play();
						}
					}
					// Un-highlight all other cells
					else if (pHoveredCell && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Color)
						m_aapPlayerBoxes[player][team]->SetDrawColor(c_PlayerSlotColorDefault);
				}
			}

			// Team info columns
			for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
			{
				// Update the team names and such
				if (newActivity)
				{
					m_apTeamBoxes[team]->SetDrawType(GUICollectionBox::Image);
					/* pointless; the CPU player icon suffices, and doesn't block the real team banner
									// CPU Team
									if (pGameActivity && pGameActivity->GetCPUTeam() == team)
									{
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "CPU Team"));
										m_apTeamNameLabels[team]->SetText(pActivity->GetTeamName(team) + ":");
									}
									// The not playing row
									else */if (team == TEAM_DISABLED)
									{
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
										m_apTeamNameLabels[team]->SetText("Not Playing:");
									}
					// Active player team
									else if (pActivity->TeamActive(team))
									{
										// Set the team flag icons on the floating player bars
										pIcon = pActivity->GetTeamIcon(team);
										// Revert to default if needed
										if (!pIcon)
										{
											char str[128];
											std::snprintf(str, sizeof(str), "Team %d Default", team + 1);
											pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", str));
										}
										m_apTeamNameLabels[team]->SetText(pActivity->GetTeamName(team) + ":");
									}
					// Disabled/unplayable teams
									else
									{
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
										m_apTeamNameLabels[team]->SetText("Unavailable");
									}

					// Finally set whatever Icon we came up with
					if (pIcon)
						m_apTeamBoxes[team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps8()[0]));
				}

				// Check if the team has any players assigned at all
				if (pActivity->TeamActive(team))
				{
					teamHasPlayers = false;
					for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
					{
						// CPU is sometimes disabled, but still counts as a team
						if (team != TEAM_DISABLED && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
						{
							teamHasPlayers = true;
							if (player != PLAYER_CPU)
								teamHasHumans = true;
						}
					}
					if (teamHasPlayers)
						++teamsWithPlayers;
					if (teamHasHumans)
						++teamsWithHumans;

					m_apTeamTechSelect[team]->SetEnabled(true);
					m_apTeamTechSelect[team]->SetVisible(true);

					m_apTeamAISkillSlider[team]->SetEnabled(true);
					m_apTeamAISkillSlider[team]->SetVisible(true);
					m_apTeamAISkillLabel[team]->SetVisible(true);
				}
				else
				{
					if (team >= Teams::TeamOne && team < Teams::MaxTeamCount)
					{
						m_apTeamTechSelect[team]->SetEnabled(false);
						m_apTeamTechSelect[team]->SetVisible(false);

						m_apTeamAISkillSlider[team]->SetEnabled(false);
						m_apTeamAISkillSlider[team]->SetVisible(false);
						m_apTeamAISkillLabel[team]->SetVisible(false);
					}
				}
			}

			// If we are over capacity with players, disable the start button and show why
			if (pActivity->GetMaxPlayerSupport() < PlayerCount())
			{
				m_pStartScenarioButton->SetVisible(false);
				m_pStartErrorLabel->SetVisible(true);
				char str[256];
				std::snprintf(str, sizeof(str), "Too many players assigned! Max for this activity is %d", pGameActivity->GetMaxPlayerSupport());
				m_pStartErrorLabel->SetText(str);
			}
			// If we are under the required number of teams with players assigned, disable the start button and show why
			else if (pActivity->GetMinTeamsRequired() > teamsWithPlayers)
			{
				m_pStartScenarioButton->SetVisible(false);
				m_pStartErrorLabel->SetVisible(true);
				char str[256];
				std::snprintf(str, sizeof(str), "Assign players to at\nleast %d of the teams!", pGameActivity->GetMinTeamsRequired());
				m_pStartErrorLabel->SetText(str);
			}
			// Assign at least one human player
			else if (teamsWithHumans == 0)
			{
				m_pStartScenarioButton->SetVisible(false);
				m_pStartErrorLabel->SetVisible(true);
				m_pStartErrorLabel->SetText("Assign human players\nto at least one team!");
			}
			// Everything checks out; let the player start if they want to
			else
			{
				m_pStartScenarioButton->SetVisible(true);
				m_pStartErrorLabel->SetVisible(false);
			}

			// How much starting gold does the slider yield
			char str[256];
			//int startGold = (float)m_pGoldSlider->GetMinimum() + ((m_pGoldSlider->GetMaximum() - m_pGoldSlider->GetMinimum()) * (float)m_pGoldSlider->GetValue() / 100.0);
			int startGold = m_pGoldSlider->GetValue();
			startGold = startGold - startGold % 500;
			if (m_pGoldSlider->GetValue() == m_pGoldSlider->GetMaximum())
				std::snprintf(str, sizeof(str), "Starting Gold: %c Infinite", -58);
			else
				std::snprintf(str, sizeof(str), "Starting Gold: %c %d oz", -58, startGold);
			m_pGoldLabel->SetText(str);


			// Set skill labels
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; team++)
			{
				m_apTeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_apTeamAISkillSlider[team]->GetValue()));
			}
		}

		// Reset all buttons and positions of things if a new activity has been selected
		if (newActivity)
		{
			m_pStartScenarioButton->SetVisible(false);
			m_pStartErrorLabel->SetVisible(true);
			m_pCPULockLabel->SetVisible(m_LockedCPUTeam != Teams::NoTeam);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          PlayerCount
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Counts how many players are currently assigned to play this Activity.

	int MultiplayerServerLobby::PlayerCount()
	{
		int count = 0;
		// Go through all the on-team non-CPU cells and see how many players are already assigned.
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
		{
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
			{
				if (m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
					++count;
			}
		}
		return count;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          StartGame
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets up and starts the currently selected Activity and settings.
	// Arguments:       None.
	// Return value:    None.

	bool MultiplayerServerLobby::StartGame()
	{
		// Get the currently selected Activity
		const Activity *pActivityPreset = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;

		if (!pActivityPreset || !m_pSelectedScene)
			return false;

		// Create the actual instance of hte Activity we want to start
		Activity *pActivity = dynamic_cast<Activity *>(pActivityPreset->Clone());
		GameActivity *pGameActivity = dynamic_cast<GameActivity *>(pActivity);

		// Set up the basic settings
		if (pGameActivity)
		{
			pGameActivity->SetDifficulty(m_pDifficultySlider->GetValue());

			pGameActivity->SetFogOfWarEnabled(m_pFogOfWarCheckbox->GetCheck());
			pGameActivity->SetRequireClearPathToOrbit(m_pRequireClearPathToOrbitCheckbox->GetCheck());

			for (int i = 0; i < c_MaxClients; i++)
				pGameActivity->SetNetworkPlayerName(i, g_NetworkServer.GetPlayerName(i));

			// If gold slider is at it max value then the amount is 'infinite' and we must set some rediculously high value
			if (m_pGoldSlider->GetValue() == m_pGoldSlider->GetMaximum())
				pGameActivity->SetStartingGold(1000000);
			else
			{
				int startGold = m_pGoldSlider->GetValue();
				startGold = startGold - startGold % 500;
				pGameActivity->SetStartingGold(startGold);
			}
		}

		g_SceneMan.SetSceneToLoad(m_pSelectedScene, true, m_pDeployUnitsCheckbox->GetCheck());

		// Set up the player and team assignments
		pActivity->ClearPlayers(false);
		for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
		{
			for (int team = Teams::TeamOne; team < TEAMROWCOUNT; ++team)
			{
				if (team != TEAM_DISABLED && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
				{
					// Add the human players, not including CPU players
					if (player != PLAYER_CPU)
						pActivity->AddPlayer(player, true, team, 0);
					// CPU team, so mark it as such.. there are no actual CPU players
					else if (pGameActivity)
					{
						pGameActivity->SetCPUTeam(team);
					}
				}
			}
		}

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
		{
			// Set up techs
			GUIListPanel::Item *pTechItem = m_apTeamTechSelect[team]->GetSelectedItem();
			if (pTechItem)
			{
				// If the "random" selection, choose one from the list of loaded techs
				if (m_apTeamTechSelect[team]->GetSelectedIndex() == 1)//pTechItem->m_ExtraIndex < 0)
				{
					int selection = RandomNum<int>(1, m_apTeamTechSelect[team]->GetListPanel()->GetItemList()->size() - 1);
					m_apTeamTechSelect[team]->SetSelectedIndex(selection);
					pTechItem = m_apTeamTechSelect[team]->GetSelectedItem();

					// Switch back to -Random-
					m_apTeamTechSelect[team]->SetSelectedIndex(1);
				}

				// Now set the selected tech's module index as what the metaplayer is going to use
				if (pTechItem)
					if (pTechItem->m_ExtraIndex == -2)
						pGameActivity->SetTeamTech(team, "-All-");
					else
						pGameActivity->SetTeamTech(team, g_PresetMan.GetDataModuleName(pTechItem->m_ExtraIndex));
			}

			// Set up AI skill levels
			if (m_apTeamAISkillSlider[team]->IsEnabled())
				pGameActivity->SetTeamAISkill(team, m_apTeamAISkillSlider[team]->GetValue());
			else
				pGameActivity->SetTeamAISkill(team, AISkillSetting::DefaultSkill);
		}

		//Force close all previously opened files
		g_LuaMan.FileCloseAll();

		// Put the new and newly set up Activity as the one to start
		g_ActivityMan.EndActivity();

		g_AudioMan.ClearMusicQueue();
		g_AudioMan.StopMusic();

		g_ActivityMan.SetRestartActivity();
		g_ActivityMan.SetStartActivity(pActivity);

		// Kill any Campaign games currently running
		if (g_MetaMan.GameInProgress())
			g_MetaMan.EndGame();

		// Signal the start of this Activity we just set up
		return /*m_ActivityRestarted = */true;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetAllScenesAndActivities
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gathers all the available Scene:s and Activity presets there are.

	void MultiplayerServerLobby::GetAllScenesAndActivities()
	{
		// Redo the list of Activities
		m_Activities.clear();

		// Get the list of all read in Scene presets
		list<Entity *> presetList;
		g_PresetMan.GetAllOfType(presetList, "Scene");
		list<Scene *> filteredScenes;
		Scene *pScene = 0;

		// Go through the list and cast all the pointers to scenes so we have a handy list
		for (list<Entity *>::iterator pItr = presetList.begin(); pItr != presetList.end(); ++pItr)
		{
			pScene = dynamic_cast<Scene *>(*pItr);
			// Only add non-editor and non-special scenes, or ones that don't have locations defined, or have Test in their names, or are metascenes
			if (pScene && !pScene->GetLocation().IsZero() && !pScene->IsMetagameInternal() && (pScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes()))
				filteredScenes.push_back(pScene);
		}

		// Get the list of all read-in Activity presets
		presetList.clear();
		g_PresetMan.GetAllOfType(presetList, "Activity");
		Activity *pActivity = 0;

		int selectedActivityIndex = m_pActivitySelect->GetSelectedIndex();

		// Go through the list and cast all the pointers to Activities so we have a handy list
		// Associate all Scenes compatible with each Activity
		// Populate the activities selection dropdown while we're at it
		m_pActivitySelect->ClearList();
		int index = 0;
		int skirmishIndex = -1;
		for (list<Entity *>::iterator pItr = presetList.begin(); pItr != presetList.end(); ++pItr)
		{
			bool isMetaActivity = false;

			pActivity = dynamic_cast<Activity *>(*pItr);
			// Only add non-editor and non-special activities
			if (pActivity/* && pActivity->GetClassName() != "GATutorial" */&& pActivity->GetClassName().find("Editor") == string::npos)
			{
				// Prepare a new entry in the list of Activity:ies that we have
				pair<Activity *, list<Scene *> > newPair(pActivity, list<Scene *>());
				for (list<Scene *>::iterator sItr = filteredScenes.begin(); sItr != filteredScenes.end(); ++sItr)
				{
					// Check if the Scene has the required Area:s and such needed for this Activity
					if (pActivity->SceneIsCompatible(*sItr))
						newPair.second.push_back(*sItr);
				}

				m_Activities.insert(newPair);
				// Add to the activity selection combo, and attach the activity copy, not passing in ownership
				m_pActivitySelect->AddItem(pActivity->GetPresetName(), "", 0, pActivity);

				// Save the tutorial mission so we can select it by default
#ifdef DEBUG_BUILD
				if (pActivity->GetPresetName() == "Network Test Activity")
					skirmishIndex = index;
#else
				if (pActivity->GetPresetName() == "Skirmish Defense")
					skirmishIndex = index;
#endif
				index++;
			}
		}

		// Select the Tutorial Activity and Scene by default to start
		if (skirmishIndex >= 0)
			m_pActivitySelect->SetSelectedIndex(skirmishIndex);
		else 
			m_pActivitySelect->SetSelectedIndex(0);

		UpdateActivityBox();
		//UpdateScenesBox();
		m_pSelectedScene = m_pScenes ? m_pScenes->front() : 0;
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Pause
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Pauses and unpauses the game.

	void MultiplayerServerLobby::SetPaused(bool pause)
	{
		// Override the pause
		m_Paused = false;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          End
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Forces the current game's end.

	void MultiplayerServerLobby::End()
	{
		Activity::End();

		m_ActivityState = ActivityState::Over;
		g_FrameMan.SetDrawNetworkBackBuffer(false);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Update
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the state of this MultiplayerServerLobby. Supposed to be done every frame
	//                  before drawing.

	void MultiplayerServerLobby::Update()
	{
		Activity::Update();

		for (int i = 0; i < c_MaxClients; i++)
		{
			if (g_NetworkServer.IsPlayerConnected(i))
			{
				if (m_apPlayerNameLabel[i]->GetText() != g_NetworkServer.GetPlayerName(i) && g_NetworkServer.GetPlayerName(i) != "")
					m_apPlayerNameLabel[i]->SetText(g_NetworkServer.GetPlayerName(i));
			}
			else 
			{
				if (m_apPlayerNameLabel[i]->GetText() != "- NO PLAYER -")
					m_apPlayerNameLabel[i]->SetText("- NO PLAYER -");
			}
		}

		{
			/*
			std::string result = "";
			int events[3];
			int states[3];
			int x, y;


			m_pGUIInput->GetMouseButtons(events, states);
			m_pGUIInput->GetMousePosition(&x, &y);

			char buf[256];
			std::snprintf(buf, sizeof(buf), "MB-%d%d%d MS-%d%d%d   %d - %d", states[0], states[1], states[2], events[0], events[1], events[2], x, y);

			result = result + buf;
			g_FrameMan.SetScreenText(result, 0, 0, -1, false);
			*/
		}

		UpdateInput();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          UpdateInput
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the user input processing.
	// Arguments:       None.
	// Return value:    None.

	void MultiplayerServerLobby::UpdateInput()
	{
		// Move the dialog to the center of the player 0 screen so it could operate in absoute mouse coordintaes of the player 0 screen
		// During draw we move the dialog to 0,0 before drawing to draw it properly into the intermediate buffer co we can draw it centered on other player's screens.
		BITMAP* drawBitmap = m_pUIDrawBitmap;
		BITMAP* finalDestBitmap = g_FrameMan.GetNetworkBackBufferIntermediate8Current(0);

		int offsetX = finalDestBitmap->w / 2 - m_pRootBox->GetWidth() / 2;
		int offsetY = finalDestBitmap->h / 2 - m_pRootBox->GetHeight() / 2;

		// Move to the center of the player 0 screen
		m_pRootBox->SetPositionAbs(offsetX, offsetY);


		m_pGUIController->Update();

		// Move to next state as current have been processed in GUIController:Update
		/*int states[3];
		int events[3];

		m_pGUIInput->GetMouseButtons(events, states);

		events[0] = states[0] == 1 ? 3 : 0;
		events[1] = states[1] == 1 ? 3 : 0;
		events[2] = states[2] == 1 ? 3 : 0;

		m_pGUIInput->SetNetworkMouseButton(0, events[0], events[1], events[2], states[0], states[1], states[2]);*/

		///////////////////////////////////////
		// Handle events

		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent))
		{
			// Commands
			if (anEvent.GetType() == GUIEvent::Command)
			{
				// Start game button pressed
				if (anEvent.GetControl() == m_pStartScenarioButton)
				{
					// Try to start the game
					if (StartGame())
					{
						// Hide all previously shown screens, show
						//HideAllScreens();
						//                    m_MenuScreen = SCENESELECT;
						//                    m_ScreenChange = true;
						//g_GUISound.ButtonPressSound()->Play();
					}
					else
						g_GUISound.UserErrorSound()->Play();
				}
			}

			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification)
			{
				// Difficulty slider changed
				if (anEvent.GetControl() == m_pDifficultySlider)
				{
					// Update the difficulty label etc
					//UpdateActivityBox();
					UpdateDifficultySlider();

					const Activity *pSelected = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
					const GameActivity * pSelectedGA = dynamic_cast<const GameActivity *>(pSelected);

					if (pSelectedGA)
						UpdateGoldSlider(pSelectedGA);
				}

				// Activity selection box affected
				if (anEvent.GetControl() == m_pActivitySelect)
				{
					// The activity selection changed
					if (anEvent.GetMsg() == GUIComboBox::Closed)
					{
						// Update the difficulty label etc
						UpdateActivityBox();

						// If there is only one Scene compatible with this newly selected Activity, then automatically select it
						if (m_pScenes && m_pScenes->size() > 0)
							m_pSelectedScene = m_pScenes->front();
						// Deselect any previously selected scene.. it may not be compatible with the new activity
						else
							m_pSelectedScene = 0;

						// Update the scene info box
						//UpdateScenesBox();
						//g_GUISound.ItemChangeSound()->Play();
					}
				}

				// Scene selection affected
				if (anEvent.GetControl() == m_pSceneSelect)
				{
					// Scene selection changed
					if (anEvent.GetMsg() == GUIComboBox::Closed)
					{
						if (m_pSceneSelect->GetSelectedItem())
							m_pSelectedScene = m_pSceneSelect->GetSelectedItem() ? dynamic_cast<const Scene *>(m_pSceneSelect->GetSelectedItem()->m_pEntity) : 0;
					}
				}
			}

			UpdatePlayersBox(false);
		}
	}



	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          DrawGUI
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

	void MultiplayerServerLobby::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which)
	{
		if (!m_pGUIController) return;

		BITMAP * drawBitmap = m_pUIDrawBitmap;
		BITMAP * finalDestBitmap = 0;

		clear_to_color(drawBitmap, g_MaskColor);

		// Store offset set for player 0
		int baseOffsetX = m_pRootBox->GetXPos();
		int baseOffsetY = m_pRootBox->GetYPos();

		// Move to default position to draw properly onto intermediate buffer
		m_pRootBox->SetPositionAbs(0, 0);

		// We need to manually draw UI's to intermediate buffer first, then to the player's backbuffer to make it centered on each player's screen.
		for (int i = 0; i < 4; i++)
		{
			if (i < c_MaxClients)
				finalDestBitmap = g_FrameMan.GetNetworkBackBufferIntermediate8Current(i);
			else
				finalDestBitmap = pTargetBitmap;

			AllegroScreen drawScreen(drawBitmap);
			m_pGUIController->Draw(&drawScreen);
			//m_pGUIController->DrawMouse();

			//Draw player icons
			for (int j = 0; j < c_MaxClients; j++)
				draw_sprite(drawBitmap, m_apPlayerIcons[j]->GetBitmaps8()[0], m_apPlayerNameLabel[j]->GetXPos() - 32, m_apPlayerNameLabel[j]->GetYPos() - 5);

			// Draw scene preview after GUI
			if (m_pSelectedScene)
			{
				BITMAP * preview = m_pSelectedScene->GetPreviewBitmap();
				if (preview)
				{
					int xOffset = 0;
					int yOffset = 0;
					// TODO: Scale down the previews to old dimensions so they fit. Deal with this when redesigning GUI one day.
					stretch_blit(preview, m_pScenePreviewBitmap, xOffset, yOffset, c_ScenePreviewWidth, c_ScenePreviewHeight, 0, 0, 140, 55 );
					//blit(preview, m_pScenePreviewBitmap, xOffset, yOffset, 0, 0, m_pScenePreviewBitmap->w, m_pScenePreviewBitmap->h);
				}
				else 
				{
					int xOffset = 0;
					int yOffset = 0;
					// TODO: Scale down the previews to old dimensions so they fit. Deal with this when redesigning GUI one day.
					stretch_blit(m_pDefaultPreviewBitmap, m_pScenePreviewBitmap, xOffset, yOffset, c_ScenePreviewWidth, c_ScenePreviewHeight, 0, 0, 140, 55);
					//blit(m_pDefaultPreviewBitmap, m_pScenePreviewBitmap, xOffset, yOffset, 0, 0, m_pScenePreviewBitmap->w, m_pScenePreviewBitmap->h);
				}
				draw_sprite(drawBitmap, m_pScenePreviewBitmap,  419, 57);
			}

			// Draw the Player-Team matrix lines and disabled overlay effects
			const Activity *pActivity = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
			int lineY = 80;
			for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team)
			{
				// Disabled shaded boxes
				if (pActivity && (!pActivity->TeamActive(team) || m_LockedCPUTeam == team))
				{
					// TODO: understand why the blending isnt working as desired
									// Transparency effect on the overlay boxes
					//drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
					// Screen blend the dots and lines, with some flicekring in its intensity
					//int blendAmount = 230;
					//set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
					rectfill(drawBitmap, m_pPlayerSetupBox->GetXPos() + 110, m_pPlayerSetupBox->GetYPos() + lineY + 56, m_pPlayerSetupBox->GetXPos() + m_pPlayerSetupBox->GetWidth() - 12, m_pPlayerSetupBox->GetYPos() + lineY + 25 + 56, c_PlayerSlotColorDisabled);
				}
				// Back to solid drawing
				//drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
				// Cell border separator lines
				line(drawBitmap, m_pPlayerSetupBox->GetXPos() + 110, m_pPlayerSetupBox->GetYPos() + lineY + 56, m_pPlayerSetupBox->GetXPos() + m_pPlayerSetupBox->GetWidth() - 12, m_pPlayerSetupBox->GetYPos() + lineY + 56, c_PlayerSlotColorHovered);
				lineY += 25;
			}

			// Manually draw UI elements on top of colored rectangle
			for (int team = Teams::MaxTeamCount - 1; team >= Teams::TeamOne; team--)
			{
				if (m_apTeamTechSelect[team]->GetVisible())
				{
					m_apTeamTechSelect[team]->Draw(&drawScreen);
					if (m_apTeamTechSelect[team]->IsDropped())
						m_apTeamTechSelect[team]->GetListPanel()->Draw(&drawScreen);
				}

				if (m_apTeamAISkillSlider[team]->GetVisible())
				{
					m_apTeamAISkillSlider[team]->Draw(&drawScreen);
					m_apTeamAISkillLabel[team]->Draw(&drawScreen);
				}
			}

			int mouseX, mouseY;
			m_pGUIInput->GetMousePosition(&mouseX, &mouseY);

			int offsetX = finalDestBitmap->w / 2 - m_pRootBox->GetWidth() / 2;
			int offsetY = finalDestBitmap->h / 2 - m_pRootBox->GetHeight() / 2;

			masked_blit(drawBitmap, finalDestBitmap, 0, 0, offsetX, offsetY, drawBitmap->w, drawBitmap->h);

			if (i == 0)
			{
				// Don't apply offsets for player 0 as he always sees mouse in abs coords
				draw_sprite(finalDestBitmap, m_pCursor, mouseX, mouseY);
				draw_sprite(finalDestBitmap, m_apPlayerIcons[0]->GetBitmaps8()[0], mouseX + 7, mouseY + 7);
			}
			/*else
			{
				draw_sprite(finalDestBitmap, m_pCursor, mouseX - offsetX, mouseY - offsetY);
				draw_sprite(finalDestBitmap, m_apPlayerIcons[0]->GetBitmaps8()[0], mouseX + 7 + (baseOffsetX - offsetX), mouseY + 7 + (baseOffsetY - offsetY));
			}*/
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Draw
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws this MultiplayerServerLobby's current graphical representation to a
	//                  BITMAP of choice. This includes all game-related graphics.

	void MultiplayerServerLobby::Draw(BITMAP* pTargetBitmap, const Vector &targetPos)
	{
		Activity::Draw(pTargetBitmap, targetPos);
	}

} // namespace RTE