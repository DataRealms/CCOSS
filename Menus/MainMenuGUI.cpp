#include "MainMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "AudioMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"
#include "GlobalScript.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIComboBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"
#include "GUI/GUICheckbox.h"

#include "Controller.h"
#include "Entity.h"
#include "MOSprite.h"
#include "DataModule.h"
#include "GABrainMatch.h"
#include "GABaseDefense.h"
#include "GATutorial.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"
#include "EditorActivity.h"
#include "MultiplayerGame.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Clear() {
		m_pController = 0;
		m_pGUIScreen = 0;
		m_pGUIInput = 0;
		m_pGUIController = 0;
		m_MenuEnabled = ENABLED;
		m_MenuScreen = MAINSCREEN;
		m_ScreenChange = false;
		m_MainMenuFocus = CAMPAIGN;
		m_FocusChange = 0;
		m_MenuSpeed = 0.3;
		m_ListItemIndex = 0;
		m_BlinkTimer.Reset();
		m_BlinkMode = NOBLINK;
		for (int screen = 0; screen < SCREENCOUNT; ++screen) {
			m_apScreenBox[screen] = 0;
		}
		for (int button = 0; button < MAINMENUBUTTONCOUNT; ++button) {
			m_MainMenuButtons[button] = 0;
		}
		m_pTeamBox = 0;
		m_pSceneSelector = 0;
		for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box) {
			m_aSkirmishBox[box] = 0;
		}
		for (int button = 0; button < SKIRMISHPLAYERCOUNT; ++button) {
			m_aSkirmishButton[button] = 0;
		}
		m_pCPUTeamLabel = 0;
		m_pEditorPanel = 0;
		m_pScrollPanel = 0;
		m_ScrollTimer.Reset();
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;
		m_TutorialOffered = false;
		m_StartPlayers = 1;
		m_StartTeams = 2;
		m_StartFunds = 1600;
		for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player) {
			m_aTeamAssignments[player] = Activity::TeamOne;
		}
		m_CPUTeam = -1;
		m_StartDifficulty = Activity::MediumDifficulty;
		m_Quit = false;

		// Editor screen
		for (int button = 0; button < EDITORBUTTONCOUNT; ++button) {
			m_aEditorButton[button] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MainMenuGUI::Create(Controller *pController) {
		RTEAssert(pController, "No controller sent to MainMenuGUI on creation!");
		m_pController = pController;

		if (!m_pGUIScreen) { m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32()); }
		if (!m_pGUIInput) { m_pGUIInput = new AllegroInput(-1, true); }
		if (!m_pGUIController) { m_pGUIController = new GUIControlManager(); }
		if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_pGUIController->Load("Base.rte/GUIs/MainMenuGUI.ini");

		// Make sure we have convenient points to the containing GUI collection boxes that we will manipulate the positions of
		m_apScreenBox[ROOT] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
		m_apScreenBox[MAINSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MainScreen"));
		m_apScreenBox[PLAYERSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PlayersScreen"));
		m_apScreenBox[SKIRMISHSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SkirmishScreen"));
		m_apScreenBox[DIFFICULTYSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("DifficultyScreen"));
		m_apScreenBox[OPTIONSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("OptionsScreen"));
		m_apScreenBox[CONFIGSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConfigScreen"));
		m_apScreenBox[EDITORSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorScreen"));
		m_apScreenBox[METASCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MetaScreen"));
		m_apScreenBox[CREDITSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsScreen"));
		m_apScreenBox[QUITSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("QuitConfirmBox"));
		m_apScreenBox[MODMANAGERSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ModManagerScreen"));

		m_apScreenBox[ROOT]->SetPositionAbs((g_FrameMan.GetResX() - m_apScreenBox[ROOT]->GetWidth()) / 2, 0);// (g_FrameMan.GetResY() - m_apScreenBox[ROOT]->GetHeight()) / 2);
	// NO, this screws up the menu positioning!
	//    m_apScreenBox[ROOT]->Resize(mapScreenBox[ROOT]->GetWidth(), g_FrameMan.GetResY());

		// Set up screens' initial positions and visibility
		m_apScreenBox[QUITSCREEN]->CenterInParent(true, true);
		// Hide all screens, the appropriate screen will reappear on next update
		HideAllScreens();

		// Panel behind editor menu to be resized depending on which editors are available
		m_pEditorPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorPanel"));
		// Credits scrolling panel
		m_pScrollPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsPanel"));

		m_MainMenuButtons[CAMPAIGN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCampaign"));
		m_MainMenuButtons[SKIRMISH] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToSkirmish"));
		m_MainMenuButtons[MULTIPLAYER] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToMultiplayer"));
		m_MainMenuButtons[OPTIONS] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToOptions"));
		m_MainMenuButtons[MODMANAGER] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToModManager"));
		m_MainMenuButtons[EDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToEditor"));
		m_MainMenuButtons[CREDITS] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCreds"));
		m_MainMenuButtons[QUIT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonQuit"));
		m_MainMenuButtons[RESUME] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonResume"));
		m_MainMenuButtons[PLAYTUTORIAL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonTutorial"));
		m_MainMenuButtons[METACONTINUE] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonContinue"));
		m_MainMenuButtons[BACKTOMAIN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonBackToMain"));
		m_MainMenuButtons[QUITCONFIRM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitConfirmButton"));
		m_MainMenuButtons[QUITCANCEL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitCancelButton"));
		m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
		m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(false);
		m_MainMenuButtons[METACONTINUE]->SetVisible(false);

		m_pSceneSelector = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ComboScene"));
		m_pTeamBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelTeams"));
		m_aSkirmishBox[P1TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP1Team"));
		m_aSkirmishBox[P2TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP2Team"));
		m_aSkirmishBox[P3TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP3Team"));
		m_aSkirmishBox[P4TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP4Team"));
		m_aSkirmishButton[P1TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1Team"));
		m_aSkirmishButton[P2TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2Team"));
		m_aSkirmishButton[P3TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3Team"));
		m_aSkirmishButton[P4TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4Team"));
		m_pCPUTeamLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelCPUTeam"));

		m_aEditorButton[SCENEEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonSceneEditor"));
		m_aEditorButton[AREAEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAreaEditor"));
		m_aEditorButton[ASSEMBLYEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAssemblyEditor"));
		m_aEditorButton[GIBEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonGibPlacement"));
		m_aEditorButton[ACTOREDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonActorEditor"));

		m_pMetaNoticeLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("MetaLabel"));

		m_VersionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("VersionLabel"));
		m_VersionLabel->SetText(c_GameVersion);
		m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 10);

		// Read all the credits from the file and set the credits label
		m_CreditsLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CreditsLabel"));
		Reader creditsReader("Credits.txt");
		std::string creditsText = creditsReader.WholeFileAsString();

		// TODO: Get Unicode going!
			// Hack here to change the special characters over 128 in the ansi ascii table to match our font files
		for (string::iterator sItr = creditsText.begin(); sItr != creditsText.end(); ++sItr) {
			if (*sItr == -60) { (*sItr) = (char)142; } //'Ä'
			if (*sItr == -42) { (*sItr) = (char)153; } //'Ö'
			if (*sItr == -87) { (*sItr) = (char)221; } //'©'
		}
		m_CreditsLabel->SetText(creditsText);
		m_CreditsLabel->ResizeHeightToFit();

		// Set initial focus, category list, and label settings
		m_ScreenChange = true;
		m_FocusChange = 1;
		//    CategoryChange();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Destroy() {
		delete m_pGUIController;
		delete m_pGUIInput;
		delete m_pGUIScreen;

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIControlManager * MainMenuGUI::GetGUIControlManager() {
		return m_pGUIController;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::SetEnabled(bool enable) {
		if (enable && m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
			m_MenuEnabled = ENABLING;
			g_GUISound.EnterMenuSound()->Play();
		} else if (!enable && m_MenuEnabled != DISABLED && m_MenuEnabled != DISABLING) {
			m_MenuEnabled = DISABLING;
			g_GUISound.ExitMenuSound()->Play();
		}

		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Update() {
		// Update the input controller
		m_pController->Update();

		// Reset the specific triggers
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;
		m_StartDifficulty = 0;
		m_Quit = false;

		// Don't update the main menu if the console is open
		if (g_ConsoleMan.IsEnabled())
			return;

		// If esc pressed, show quit dialog if applicable
		if (g_UInputMan.KeyPressed(KEY_ESC)) {
			if (m_MenuScreen == OPTIONSSCREEN || m_MenuScreen == MODMANAGERSCREEN || m_MenuScreen == EDITORSCREEN || m_MenuScreen == CREDITSSCREEN) {
				HideAllScreens();
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
				m_MenuScreen = MAINSCREEN;
				m_ScreenChange = true;
				g_GUISound.BackButtonPressSound()->Play();

				if (m_MenuScreen == OPTIONSSCREEN) {
					//g_SettingsMan.SetFlashOnBrainDamage(m_aOptionsCheckbox[FLASHONBRAINDAMAGE]->GetCheck());
					//g_SettingsMan.SetBlipOnRevealUnseen(m_aOptionsCheckbox[BLIPONREVEALUNSEEN]->GetCheck());
					//g_SettingsMan.SetShowForeignItems(m_aOptionsCheckbox[SHOWFOREIGNITEMS]->GetCheck());
					//g_SettingsMan.SetShowToolTips(m_aOptionsCheckbox[SHOWTOOLTIPS]->GetCheck());
					g_SettingsMan.UpdateSettingsFile();
				} else if (m_MenuScreen == MODMANAGERSCREEN) {
					g_SettingsMan.UpdateSettingsFile();
				}
			} else {
				QuitLogic();
			}
		}

		////////////////////////////////////////////////////////////////////////
		// Animate the menu into and out of view if enabled or disabled

		// Quit now if we aren't enabled
		if (m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
			return;
		}

		//////////////////////////////////////
		// MAINSCREEN MENU SCREEN

		if (m_MenuScreen == MAINSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[MAINSCREEN]->SetVisible(true);

				if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
					m_apScreenBox[MAINSCREEN]->Resize(128, 220);
					m_MainMenuButtons[RESUME]->SetVisible(true);
				} else {
					m_apScreenBox[MAINSCREEN]->Resize(128, 196);
					m_MainMenuButtons[RESUME]->SetVisible(false);
				}
				// Restore the label on the campaign button
				m_MainMenuButtons[CAMPAIGN]->SetText("Metagame (WIP)");

				m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
				m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(false);
				m_MainMenuButtons[METACONTINUE]->SetVisible(false);
				m_ScreenChange = false;
			}

			// Blink the resume button to show the game is still going
			if (m_MainMenuButtons[RESUME]->GetVisible()) {
				if (m_BlinkTimer.AlternateReal(500)) {
					m_MainMenuButtons[RESUME]->SetFocus();
				} else {
					m_pGUIController->GetManager()->SetFocus(0);
				}
			}
		}

		//////////////////////////////////////
		// PLAYERS MENU SCREEN

		else if (m_MenuScreen == PLAYERSSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[PLAYERSSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
		}

		//////////////////////////////////////
		// SKIRMISH SETUP MENU SCREEN

		else if (m_MenuScreen == SKIRMISHSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[SKIRMISHSCREEN]->SetVisible(true);
				// Set up the list of scenes to choose from
				UpdateScenesBox();
				//            m_pGUIController->GetControl("ButtonStartSkirmish")->SetVisible(true);
				UpdateTeamBoxes();
				// Move main menu button over so the start button fits
				m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(200, 280);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_ScreenChange = false;
			}

			//        for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box)
			//            m_aSkirmishBox[box] = 0;

			//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
		}

		//////////////////////////////////////
		// DIFFICULTY MENU SCREEN

		else if (m_MenuScreen == DIFFICULTYSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[DIFFICULTYSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
		}

		//////////////////////////////////////
		// OPTIONS MENU SCREEN

		else if (m_MenuScreen == OPTIONSSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[OPTIONSSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_apScreenBox[OPTIONSSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
				m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(180, 220);
				//m_pBackToOptionsButton->SetVisible(false);
				//UpdateDeviceLabels();
				m_ScreenChange = false;
			}
		}

		//////////////////////////////////////
		// CONFIGURATION SCREEN

		else if (m_MenuScreen == CONFIGSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[CONFIGSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
				//m_pBackToOptionsButton->SetVisible(true);
				// Let this pass through, UpdateConfigScreen uses it
				//m_ScreenChange = false;
			}

			// Continuously update the contents through all the config steps
			//UpdateConfigScreen();
		}

		//////////////////////////////////////
		// MOD MANAGER SCREEN

		else if (m_MenuScreen == MODMANAGERSCREEN) {
			if (m_ScreenChange) { m_apScreenBox[MODMANAGERSCREEN]->SetVisible(true); }
		}

		//////////////////////////////////////
		// EDITOR MENU SCREEN

		else if (m_MenuScreen == EDITORSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[EDITORSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_apScreenBox[EDITORSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
				m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(4, 145);
				m_ScreenChange = false;
			}
		}

		//////////////////////////////////////
		// CREDITS MENU SCREEN

		else if (m_MenuScreen == CREDITSSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[CREDITSSCREEN]->SetVisible(true);
				m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
				m_apScreenBox[CREDITSSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
				m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(240, 298);
				m_pScrollPanel->SetPositionRel(0, 0);
				m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight());
				m_ScrollTimer.Reset();
				m_ScreenChange = false;
			}

			long scrollTime = 90000;
			float scrollProgress = (float)m_ScrollTimer.GetElapsedRealTimeMS() / (float)scrollTime;
			int scrollDist = m_pScrollPanel->GetHeight() + m_CreditsLabel->GetHeight();
			m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight() - static_cast<int>(static_cast<float>(scrollDist) * scrollProgress));
			// If we've scrolled through the whole thing, reset to the bottom and restart scroll
			if (m_ScrollTimer.IsPastRealMS(scrollTime)) {
				m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight());
				m_ScrollTimer.Reset();
			}

			//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
		}

		//////////////////////////////////////
		// METAGAME NOTICE SCREEN

		else if (m_MenuScreen == METASCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[METASCREEN]->SetVisible(true);
				m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(true);
				m_MainMenuButtons[METACONTINUE]->SetVisible(true);
				m_pMetaNoticeLabel->SetText("- A T T E N T I O N -\n\nPlease note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\nAs such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\nThat said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\nAlso, if you have not yet played Cortex Command, we recommend you first try the tutorial:");
				m_pMetaNoticeLabel->SetVisible(true);
				// Flag that this notice has now been shown once, so no need to keep showing it
				m_TutorialOffered = true;
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
		}

		//////////////////////////////////////
		// QUIT CONFIRM SCREEN

		else if (m_MenuScreen == QUITSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox[QUITSCREEN]->SetVisible(true);
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons[QUITCONFIRM]->SetFocus();
		}

		//////////////////////////////////////////
		// Update the ControlManager

		m_pGUIController->Update();

		///////////////////////////////////////
		// Handle events

		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent)) {
			// Commands
			if (anEvent.GetType() == GUIEvent::Command) {
				// Campaign button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[CAMPAIGN]) {
					/*
									// Disable the campaign button for now
									if (m_MainMenuButtons[CAMPAIGN]->GetText() == "Campaign")
									{
										m_MainMenuButtons[CAMPAIGN]->SetText("COMING SOON!");
										g_GUISound.ExitMenuSound()->Play();
									}
									else
									{
										m_MainMenuButtons[CAMPAIGN]->SetText("Campaign");
										g_GUISound.ButtonPressSound()->Play();
									}
					*/
					// Show the metagame notice screen if it hasn't already been shown yet
					if (!m_TutorialOffered)
						m_MenuScreen = METASCREEN;
					// Start the campaign right away!
					else {
						m_CampaignStarted = true;
						m_MenuScreen = MAINSCREEN;
					}

					HideAllScreens();
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				}

				// Skirmish button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[SKIRMISH]) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					//                m_MenuScreen = PLAYERSSCREEN;
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
					//                g_GUISound.ExitMenuSound()->Play();
				}

				if (anEvent.GetControl() == m_MainMenuButtons[MULTIPLAYER]) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					//HideAllScreens();
					//                m_MenuScreen = PLAYERSSCREEN;
					//m_MenuScreen = MAINSCREEN;
					//m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
					//                g_GUISound.ExitMenuSound()->Play();

					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;
					m_ActivityRestarted = true;
					g_GUISound.ExitMenuSound()->Play();

					g_SceneMan.SetSceneToLoad("Editor Scene");
					MultiplayerGame *pMultiplayerGame = new MultiplayerGame;
					pMultiplayerGame->Create();
					g_ActivityMan.SetStartActivity(pMultiplayerGame);
				}

				// Options button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[OPTIONS]) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Editor button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[EDITOR]) {
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = EDITORSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
					//                g_GUISound.UserErrorSound()->Play();
				}

				// Editor button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[MODMANAGER]) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MODMANAGERSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Credits button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[CREDITS]) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = CREDITSSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Quit button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[QUIT]) {
					QuitLogic();
					g_GUISound.ButtonPressSound()->Play();
				}

				// Resume button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[RESUME]) {
					m_ActivityResumed = true;

					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// PLAYER SCREEN BUTTONS
				// Player count setting button pressed

				if (m_MenuScreen == PLAYERSSCREEN &&
					(anEvent.GetControl()->GetName() == "ButtonOnePlayer" ||
						anEvent.GetControl()->GetName() == "ButtonTwoPlayers" ||
						anEvent.GetControl()->GetName() == "ButtonThreePlayers" ||
						anEvent.GetControl()->GetName() == "ButtonFourPlayers")) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = SKIRMISHSCREEN;
					m_ScreenChange = true;

					// Set desired player count
					if (anEvent.GetControl()->GetName() == "ButtonOnePlayer") {
						m_StartPlayers = 1;
					} else if (anEvent.GetControl()->GetName() == "ButtonTwoPlayers") {
						m_StartPlayers = 2;
					} else if (anEvent.GetControl()->GetName() == "ButtonThreePlayers") {
						m_StartPlayers = 3;
					} else if (anEvent.GetControl()->GetName() == "ButtonFourPlayers") {
						m_StartPlayers = 4;
					} else {
						m_StartPlayers = 0;
					}

					g_GUISound.ButtonPressSound()->Play();
				}

				/////////////////////////////////////////////
				// SKIRMISH SETUP SCREEN BUTTONS

				if (m_MenuScreen == SKIRMISHSCREEN) {
					for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player) {
						// Player team toggle button
						if (anEvent.GetControl() == m_aSkirmishButton[player]) {
							// Toggle
							if (m_aTeamAssignments[player] == Activity::TeamOne) {
								m_aTeamAssignments[player] = Activity::TeamTwo;
							} else {
								m_aTeamAssignments[player] = Activity::TeamOne;
							}
							UpdateTeamBoxes();

							g_GUISound.ButtonPressSound()->Play();
						}
					}

					// Start Skirmish menu button pressed
					if (anEvent.GetControl()->GetName() == "ButtonStartSkirmish") {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();

						// No CPU team, so just start game
						if (m_CPUTeam < 0) {
							m_MenuScreen = MAINSCREEN;
							m_ScreenChange = true;
							m_ActivityRestarted = true;
							SetupSkirmishActivity();
							g_GUISound.ExitMenuSound()->Play();
						}
						// CPU team present, so ask for the difficulty level of it before starting
						else {
							m_MenuScreen = DIFFICULTYSCREEN;
							m_ScreenChange = true;
							g_GUISound.ButtonPressSound()->Play();
						}
					}
				}

				/////////////////////////////////////////////
				// DIFFICULTY SETUP SCREEN BUTTONS

				if (m_MenuScreen == DIFFICULTYSCREEN &&
					(anEvent.GetControl()->GetName() == "ButtonStartEasy" ||
						anEvent.GetControl()->GetName() == "ButtonStartMedium" ||
						anEvent.GetControl()->GetName() == "ButtonStartHard" ||
						anEvent.GetControl()->GetName() == "ButtonStartDeath")) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;

					m_ActivityRestarted = true;

					// Set appropriate difficulty level
					if (anEvent.GetControl()->GetName() == "ButtonStartEasy") {
						m_StartDifficulty = Activity::EasyDifficulty;
					} else if (anEvent.GetControl()->GetName() == "ButtonStartMedium") {
						m_StartDifficulty = Activity::MediumDifficulty;
					} else if (anEvent.GetControl()->GetName() == "ButtonStartHard") {
						m_StartDifficulty = Activity::HardDifficulty;
					} else if (anEvent.GetControl()->GetName() == "ButtonStartDeath") {
						m_StartDifficulty = Activity::MaxDifficulty;
					} else {
						m_StartDifficulty = Activity::MediumDifficulty;
					}

					SetupSkirmishActivity();

					//                g_GUISound.BackButtonPressSound()->Play();
					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// EDITOR SCREEN BUTTONS

				if (m_MenuScreen == EDITORSCREEN &&
					(anEvent.GetControl() == m_aEditorButton[SCENEEDITOR] ||
						anEvent.GetControl() == m_aEditorButton[AREAEDITOR] ||
						anEvent.GetControl() == m_aEditorButton[ASSEMBLYEDITOR] ||
						anEvent.GetControl() == m_aEditorButton[GIBEDITOR] ||
						anEvent.GetControl() == m_aEditorButton[ACTOREDITOR])) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;

					m_ActivityRestarted = true;

					// Create and start the appropriate editor Activity
					if (anEvent.GetControl() == m_aEditorButton[SCENEEDITOR]) {
						StartSceneEditor();
					} else if (anEvent.GetControl() == m_aEditorButton[AREAEDITOR]) {
						StartAreaEditor();
					} else if (anEvent.GetControl() == m_aEditorButton[ASSEMBLYEDITOR]) {
						StartAssemblyEditor();
					} else if (anEvent.GetControl() == m_aEditorButton[GIBEDITOR]) {
						StartGibEditor();
					} else if (anEvent.GetControl() == m_aEditorButton[ACTOREDITOR]) {
						StartActorEditor();
					}

					//                g_GUISound.BackButtonPressSound()->Play();
					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// META NOTICE SCREEN BUTTONS

				if (m_MenuScreen == METASCREEN) {
					// Play tutorial button pressed
					if (anEvent.GetControl() == m_MainMenuButtons[PLAYTUTORIAL]) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;

						// Set up and start the tutorial!
						g_ActivityMan.SetStartActivity(dynamic_cast<Activity *>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
						GameActivity * pGameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetStartActivity());
						if (pGameActivity) { pGameActivity->SetStartingGold(10000); }
						g_SceneMan.SetSceneToLoad("Tutorial Bunker");
						m_ActivityRestarted = true;

						g_GUISound.ButtonPressSound()->Play();
					}
					// Go to registration dialog button
					else if (anEvent.GetControl() == m_MainMenuButtons[METACONTINUE]) {
						m_CampaignStarted = true;

						// Hide all screens, and stay in main menu for if/when player comes back to the main menu
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;
						g_GUISound.ButtonPressSound()->Play();
					}
				}

				/////////////////////////////////////////////
				// MOD MANAGER SCREEN BUTTONS

				if (m_MenuScreen == MODMANAGERSCREEN) {
					;
				}

				/////////////////////////////////////////////
				// QUIT SCREEN BUTTONS

				if (m_MenuScreen == QUITSCREEN) {
					// Confirm quitting of game
					if (anEvent.GetControl() == m_MainMenuButtons[QUITCONFIRM]) {
						m_Quit = true;

						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
					// Cancel quitting
					else if (anEvent.GetControl() == m_MainMenuButtons[QUITCANCEL]) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
				}
			}

			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification) {
				// Button focus notification that we can play a sound to
				if (dynamic_cast<GUIButton *>(anEvent.GetControl())) {
					if (anEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
				}

				// Mod list pressed
				//if (anEvent.GetControl() == m_pModManagerModsListBox) { ; }

				// Script list pressed
				//if (anEvent.GetControl() == m_pModManagerScriptsListBox) { ; }

				// Resolution combobox closed, something new selected
				//if (anEvent.GetControl() == m_pResolutionCombo) { ; }

				// Sound Volume slider changed
				//if (anEvent.GetControl() == m_pSoundSlider) { ; }

				// Music Volume slider changed
				//if (anEvent.GetControl() == m_pMusicSlider) { ; }

				// Dead zone sliders control
				//for (int which = P1DEADZONESLIDER; which < DEADZONESLIDERCOUNT; ++which) { ; }
				/*
							// Scrollbar changed
							if(anEvent.GetControl()->GetName() == "scroll1")
							{
								int Value = 0;
								GUIScrollbar *S = (GUIScrollbar *)anEvent.GetControl();
								Value = S->GetValue();
								GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
								char buf[64];
								L->SetText(itoa(Value, buf, 10));
							}

							// Double click on the listbox
							if(anEvent.GetControl()->GetName() == "list1")
							{
								if(anEvent.GetMsg() == GUIListBox::DoubleClicked)
								{
									GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
									GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
									if(LB->GetSelected())
										L->SetText(LB->GetSelected()->m_Name);
								}

								if(anEvent.GetMsg() == GUIListBox::KeyDown)
								{
									// Delete
									if(anEvent.GetData() == GUIInput::Key_Delete)
									{
										GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
										LB->DeleteItem(LB->GetSelectedIndex());
									}
								}
							}

							if(anEvent.GetControl()->GetName() == "text1")
							{
								if(anEvent.GetMsg() == GUITextBox::Enter)
								{
									GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
									GUITextBox *T = (GUITextBox *)anEvent.GetControl();

									L->SetText(T->GetText());
								}
							}
				*/
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw(BITMAP *drawBitmap) const {
		AllegroScreen drawScreen(drawBitmap);
		m_pGUIController->Draw(&drawScreen);
		m_pGUIController->DrawMouse();

		/*#ifdef DEBUG_BUILD
				if (g_UInputMan.JoystickActive(0))
				{
					Vector aim = g_UInputMan.AnalogAimValues(0);
					float axis00 = g_UInputMan.AnalogAxisValue(0, 0, 0);
					float axis01 = g_UInputMan.AnalogAxisValue(0, 0, 1);

					float axis10 = g_UInputMan.AnalogAxisValue(0, 1, 0);
					float axis11 = g_UInputMan.AnalogAxisValue(0, 1, 1);
					char s[256];
					std::snprintf(s, sizeof(s), "Aim %.1f %.1f - Stick 0 %.1f %.1f - Stick 1 %.1f %.1f", aim.GetX(), aim.GetY(), axis00, axis01, axis10, axis11);

					GUILabel * debugLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelDebug"));
					if (debugLabel)
						debugLabel->SetText(s);
				}
		#endif*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HideAllScreens() {
		for (int iscreen = MAINSCREEN; iscreen < SCREENCOUNT; ++iscreen) {
			if (m_apScreenBox[iscreen]) { m_apScreenBox[iscreen]->SetVisible(false); }
		}
		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::QuitLogic() {
		// If quit confirm dialog not already showing, or an activity is running, show it
		if (m_MenuScreen != QUITSCREEN && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			HideAllScreens();
			m_MenuScreen = QUITSCREEN;
			m_ScreenChange = true;
		}
		// No activity, so just start quitting
		else {
			m_Quit = true;

			// Hide all screens, the appropriate screen will reappear on next update
			HideAllScreens();
			m_ScreenChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::SetupSkirmishActivity() {
		// If activity restarted, stuff the ActivityMan with the selected data
		if (m_ActivityRestarted) {
			// TODO: ******* add the game mode drop down and base the game mode selection off that instead

					// No CPU team, so Brain match
			if (m_CPUTeam == Activity::NoTeam) {
				g_SceneMan.SetSceneToLoad(m_pSceneSelector->GetItem(m_pSceneSelector->GetSelectedIndex())->m_Name);
				// TODO: Let player choose the GABrainMatch activity instance!
				GABrainMatch *pNewGame = new GABrainMatch;

				for (int player = Players::PlayerOne; player < m_StartPlayers; ++player) {
					pNewGame->SetTeamOfPlayer(player, m_aTeamAssignments[player]);
				}
				pNewGame->SetCPUTeam(m_CPUTeam);
				pNewGame->Create();
				g_ActivityMan.SetStartActivity(pNewGame);
			}
			// CPU present, so base defense
			else {
				g_SceneMan.SetSceneToLoad(m_pSceneSelector->GetItem(m_pSceneSelector->GetSelectedIndex())->m_Name);
				// TODO: Let player choose the GABaseDefense activity instance!
				GABaseDefense *pNewGame = dynamic_cast<GABaseDefense *>(g_PresetMan.GetEntityPreset("GABaseDefense", "Skirmish Defense")->Clone());
				RTEAssert(pNewGame, "Couldn't find the \"Skirmish Defense\" GABaseDefense Activity! Has it been defined?");

				for (int player = Players::PlayerOne; player < m_StartPlayers; ++player) {
					pNewGame->SetTeamOfPlayer(player, m_aTeamAssignments[player]);
				}
				pNewGame->SetCPUTeam(m_CPUTeam);
				pNewGame->SetDifficulty(m_StartDifficulty);

				pNewGame->Create();
				g_ActivityMan.SetStartActivity(pNewGame);
			}

			// TODO: Reenable and make GUI control for this!
			/*
					g_ActivityMan.GetActivity()->SetStartingFunds(m_StartFunds);
					for (int team = 0; team < m_StartTeams; ++team)
					{
						g_ActivityMan.GetActivity()->SetTeamFunds(m_StartFunds, team);
					}
			*/
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateScenesBox() {
		// Clear out the control
		m_pSceneSelector->ClearList();

		// Get the list of all read in scenes
		list<Entity *> sceneList;
		g_PresetMan.GetAllOfType(sceneList, "Scene");

		// Go through the list and add their names to the combo box
		for (list<Entity *>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr) {
			m_pSceneSelector->AddItem((*itr)->GetPresetName());
		}

		// Select the first one
		m_pSceneSelector->SetSelectedIndex(0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateTeamBoxes() {
		char str[128];

		// Hide all team assignment panels initially, and center their contents
		for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box) {
			m_aSkirmishBox[box]->SetVisible(false);
			m_aSkirmishButton[box]->CenterInParent(true, true);
		}

		// Total area size
		int areaWidth = m_pTeamBox->GetWidth();
		int areaHeight = m_pTeamBox->GetHeight();

		// Set up the team assignment boxes and contained labels and buttons
		// Single team to set up for single player
		if (m_StartPlayers == 1) {
			// Show and resize
			m_aSkirmishBox[P1TEAM]->Resize(areaWidth, areaHeight);
			m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
			m_aSkirmishBox[P1TEAM]->SetVisible(true);
		}
		// Two player split one above the other
		else if (m_StartPlayers == 2) {
			int boxHeight = (areaHeight - 4) / 2;

			// Player 1
			m_aSkirmishBox[P1TEAM]->Resize(areaWidth, boxHeight);
			m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
			m_aSkirmishBox[P1TEAM]->SetVisible(true);

			// Player 2
			m_aSkirmishBox[P2TEAM]->Resize(areaWidth, boxHeight);
			m_aSkirmishBox[P2TEAM]->SetPositionRel(0, boxHeight + 4);
			m_aSkirmishButton[P2TEAM]->CenterInParent(true, true);
			m_aSkirmishBox[P2TEAM]->SetVisible(true);
		}
		// Four-way split, either three or four players
		else if (m_StartPlayers >= 3) {
			int boxWidth = (areaWidth - 4) / 2;
			int boxHeight = (areaHeight - 4) / 2;

			// Player 1
			m_aSkirmishBox[P1TEAM]->Resize(boxWidth, boxHeight);
			m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
			m_aSkirmishBox[P1TEAM]->SetVisible(true);

			// Player 2
			m_aSkirmishBox[P2TEAM]->Resize(boxWidth, boxHeight);
			m_aSkirmishBox[P2TEAM]->SetPositionRel(boxWidth + 4, 0);
			m_aSkirmishButton[P2TEAM]->CenterInParent(true, true);
			m_aSkirmishBox[P2TEAM]->SetVisible(true);

			// Player 3
			m_aSkirmishBox[P3TEAM]->SetVisible(true);

			// Player 4
			if (m_StartPlayers == 4) { m_aSkirmishBox[P4TEAM]->SetVisible(true); }
		}

		// Update button labels
		for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player) {
			if (m_aTeamAssignments[player] == Activity::TeamOne) {
				m_aSkirmishBox[player]->SetDrawColor(makecol(70, 27, 12));
				std::snprintf(str, sizeof(str), "Player %i: %c", player + 1, -62);
			} else {
				m_aSkirmishBox[player]->SetDrawColor(makecol(47, 55, 40));
				std::snprintf(str, sizeof(str), "Player %i: %c", player + 1, -59);
			}
			m_aSkirmishButton[player]->SetText(str);
		}

		////////////////////////////////
		// Update CPU team label

		// Count how many players on each team
		int team0Count = 0;
		int team1Count = 0;
		for (int player = Players::PlayerOne; player < m_StartPlayers; ++player) {
			if (m_aTeamAssignments[player] == 0) {
				team0Count++;
			} else if (m_aTeamAssignments[player] == 1) {
				team1Count++;
			}
		}

		// See if either team is empty of human players - that becomes the CPU team
		if (team0Count == 0 || team1Count == 0) {
			std::snprintf(str, sizeof(str), "CPU Team: %c", team0Count == 0 ? -62 : -59);
			m_CPUTeam = team0Count == 0 ? 0 : 1;
		} else {
			std::snprintf(str, sizeof(str), "No CPU Team (both have players)");
			m_CPUTeam = -1;
		}

		// Finally set the label
		m_pCPUTeamLabel->SetText(str);
	}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartActorEditor() { StartEditorActivity(new ActorEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartGibEditor() { StartEditorActivity(new GibEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartSceneEditor() { StartEditorActivity(new SceneEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAreaEditor() { StartEditorActivity(new AreaEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAssemblyEditor() { StartEditorActivity(new AssemblyEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartEditorActivity(EditorActivity *editorActivityToStart) {
		g_SceneMan.SetSceneToLoad("Editor Scene");
		editorActivityToStart->Create();
		editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
		g_ActivityMan.SetStartActivity(editorActivityToStart);
	}
}