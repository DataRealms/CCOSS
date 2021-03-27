#include "MainMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

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
		m_Controller = nullptr;
		m_GUIScreen = nullptr;
		m_GUIInput = nullptr;
		m_GUIControlManager = nullptr;
		m_MenuEnabled = false;
		m_ActiveMenuScreen = MenuScreen::MainScreen;
		m_ScreenChange = false;
		m_BlinkTimer.Reset();

		m_EditorMenuPanel = nullptr;
		m_CreditsScrollPanel = nullptr;
		m_ScrollTimer.Reset();
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;
		m_TutorialOffered = false;

		m_Quit = false;

		std::fill(m_MainMenuScreens.begin(), m_MainMenuScreens.end(), nullptr);
		std::fill(m_MainMenuButtons.begin(), m_MainMenuButtons.end(), nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Create(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller) {
		m_Controller = controller;

		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		if (!m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_GUIControlManager->Load("Base.rte/GUIs/MainMenuGUI.ini");

		m_MainMenuScreens.at(MenuScreen::Root) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		m_MainMenuScreens.at(MenuScreen::Root)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::Root)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuScreens.at(MenuScreen::Root)->GetWidth()) / 2, 0);
		m_RootCollectionBoxOriginalHeight = m_MainMenuScreens.at(MenuScreen::Root)->GetHeight();

		m_MainMenuScreens.at(MenuScreen::MainScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("MainScreen"));
		m_MainMenuScreens.at(MenuScreen::SettingsScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("OptionsScreen"));
		//m_MainMenuScreens.at(CONFIGSCREEN) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfigScreen"));
		m_MainMenuScreens.at(MenuScreen::EditorScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("EditorScreen"));
		m_MainMenuScreens.at(MenuScreen::CampaignScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("MetaScreen"));
		m_MainMenuScreens.at(MenuScreen::CreditsScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CreditsScreen"));
		//m_MainMenuScreens.at(MenuScreen::ModManagerScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ModManagerScreen"));

		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->Resize(m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GetWidth(), g_FrameMan.GetResY());

		m_MainMenuScreens.at(MenuScreen::QuitScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("QuitConfirmBox"));
		m_MainMenuScreens.at(MenuScreen::QuitScreen)->CenterInParent(true, true);

		// Panel behind editor menu to be resized depending on which editors are available
		m_EditorMenuPanel = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("EditorPanel"));

		m_MainMenuButtons.at(MenuButton::CampaignButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToCampaign"));
		m_MainMenuButtons.at(MenuButton::SkirmishButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToSkirmish"));
		m_MainMenuButtons.at(MenuButton::MultiplayerButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToMultiplayer"));
		m_MainMenuButtons.at(MenuButton::SettingsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToOptions"));
		m_MainMenuButtons.at(MenuButton::ModManagerButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToModManager"));
		m_MainMenuButtons.at(MenuButton::EditorsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToEditor"));
		m_MainMenuButtons.at(MenuButton::CreditsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToCreds"));
		m_MainMenuButtons.at(MenuButton::QuitButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuit"));
		m_MainMenuButtons.at(MenuButton::ResumeButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonResume"));
		m_MainMenuButtons.at(MenuButton::PlayTutorialButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonTutorial"));
		m_MainMenuButtons.at(MenuButton::CampaignContinueButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonContinue"));
		m_MainMenuButtons.at(MenuButton::BackToMainButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMain"));
		m_MainMenuButtons.at(MenuButton::QuitConfirmButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("QuitConfirmButton"));
		m_MainMenuButtons.at(MenuButton::QuitCancelButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("QuitCancelButton"));
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
		m_MainMenuButtons.at(MenuButton::PlayTutorialButton)->SetVisible(false);
		m_MainMenuButtons.at(MenuButton::CampaignContinueButton)->SetVisible(false);

		m_MainMenuButtons.at(MenuButton::SceneEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonSceneEditor"));
		m_MainMenuButtons.at(MenuButton::AreaEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonAreaEditor"));
		m_MainMenuButtons.at(MenuButton::AssemblyEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonAssemblyEditor"));
		m_MainMenuButtons.at(MenuButton::GitEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonGibPlacement"));
		m_MainMenuButtons.at(MenuButton::ActorEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonActorEditor"));

		m_MetaNoticeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("MetaLabel"));

		m_VersionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("VersionLabel"));
		m_VersionLabel->SetText(c_GameVersion);
		m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 10);

		m_CreditsScrollPanel = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CreditsPanel"));
		m_CreditsLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("CreditsLabel"));
		std::string creditsText = Reader("Credits.txt").WholeFileAsString();

		// TODO: Get Unicode going!
		// Hack here to change the special characters over 128 in the ANSI ASCII table to match our font files
		for (char &stringChar : creditsText) {
			if (stringChar == -60) {
				stringChar = static_cast<unsigned char>(142); //'Ä'
			} else if (stringChar == -42) {
				stringChar = static_cast<unsigned char>(153); //'Ö'
			} else if (stringChar == -87) {
				stringChar = static_cast<unsigned char>(221); //'©'
			}
		}
		m_CreditsLabel->SetText(creditsText);
		m_CreditsLabel->ResizeHeightToFit();

		// Set initial focus, category list, and label settings
		m_ScreenChange = true;

		HideAllScreens();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void MainMenuGUI::Destroy() {
		Clear();
	}
	*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::SetEnabled(bool enable) {
		if (enable && m_MenuEnabled != true) {
			m_MenuEnabled = true;
			g_GUISound.EnterMenuSound()->Play();
		} else if (!enable && m_MenuEnabled != false) {
			m_MenuEnabled = false;
			g_GUISound.ExitMenuSound()->Play();
		}
		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIControlManager * MainMenuGUI::GetGUIControlManager() {
		return m_GUIControlManager.get();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartActorEditor() const { StartEditorActivity(new ActorEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartGibEditor() const { StartEditorActivity(new GibEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartSceneEditor() const { StartEditorActivity(new SceneEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAreaEditor() const { StartEditorActivity(new AreaEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAssemblyEditor() const { StartEditorActivity(new AssemblyEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartEditorActivity(EditorActivity *editorActivityToStart) const {
		g_SceneMan.SetSceneToLoad("Editor Scene");
		editorActivityToStart->Create();
		editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
		g_ActivityMan.SetStartActivity(editorActivityToStart);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HideAllScreens() {
		for (int screen = MenuScreen::MainScreen; screen < MenuScreen::ScreenCount; ++screen) {
			if (m_MainMenuScreens.at(screen)) { m_MainMenuScreens.at(screen)->SetVisible(false); }
		}
		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::QuitLogic() {
		// If quit confirm dialog not already showing, or an activity is running, show it
		if (m_ActiveMenuScreen != MenuScreen::QuitScreen && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			HideAllScreens();
			m_ActiveMenuScreen = MenuScreen::QuitScreen;
			m_ScreenChange = true;
		} else {
			m_Quit = true;

			// Hide all screens, the appropriate screen will reappear on next update
			HideAllScreens();
			m_ScreenChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowCreditsScreen() {
		m_ScreenChange = false;
		m_MainMenuScreens.at(MenuScreen::Root)->Resize(m_MainMenuScreens.at(MenuScreen::Root)->GetWidth(), g_FrameMan.GetResY() - m_MainMenuScreens.at(MenuScreen::Root)->GetYPos() - 10);

		int scrollPanelHeight = m_MainMenuScreens.at(MenuScreen::Root)->GetHeight() - m_CreditsScrollPanel->GetYPos() - 30;
		if (m_CreditsScrollPanel->GetHeight() != scrollPanelHeight) { m_CreditsScrollPanel->Resize(m_CreditsScrollPanel->GetWidth(), scrollPanelHeight); }
		m_CreditsScrollPanel->SetPositionRel(0, 0);
		m_CreditsLabel->SetPositionRel(0, scrollPanelHeight);

		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuButtons.at(MenuButton::BackToMainButton)->GetWidth()) / 2, m_CreditsScrollPanel->GetYPos() + scrollPanelHeight + 10);

		m_ScrollTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::RollCredits() {
		int scrollTime = m_CreditsLabel->GetHeight() * 50;
		int scrollDist = m_CreditsScrollPanel->GetHeight() + m_CreditsLabel->GetHeight();
		float scrollProgress = static_cast<float>(m_ScrollTimer.GetElapsedRealTimeMS()) / static_cast<float>(scrollTime);
		m_CreditsLabel->SetPositionRel(0, m_CreditsScrollPanel->GetHeight() - static_cast<int>(static_cast<float>(scrollDist) * scrollProgress));
		// If we've scrolled through the whole thing, reset to the bottom and restart scroll
		if (m_ScrollTimer.IsPastRealMS(scrollTime)) {
			m_CreditsLabel->SetPositionRel(0, m_CreditsScrollPanel->GetHeight());
			m_ScrollTimer.Reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleInputEvents() {
		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			// Commands
			if (guiEvent.GetType() == GUIEvent::Command) {
				// Campaign button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::CampaignButton)) {
					// Show the MetaGame notice screen if it hasn't already been shown yet
					if (!m_TutorialOffered)
						m_ActiveMenuScreen = MenuScreen::CampaignScreen;
					// Start the campaign right away!
					else {
						m_CampaignStarted = true;
						m_ActiveMenuScreen = MenuScreen::MainScreen;
					}
					HideAllScreens();
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				}

				// Skirmish button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SkirmishButton)) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::MainScreen;
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				}

				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::MultiplayerButton)) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					g_GUISound.ButtonPressSound()->Play();

					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::MainScreen;
					m_ScreenChange = true;
					m_ActivityRestarted = true;
					g_GUISound.ExitMenuSound()->Play();

					g_SceneMan.SetSceneToLoad("Editor Scene");
					MultiplayerGame *pMultiplayerGame = new MultiplayerGame;
					pMultiplayerGame->Create();
					g_ActivityMan.SetStartActivity(pMultiplayerGame);
				}

				// Options button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SettingsButton)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::SettingsScreen;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Editor button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::EditorsButton)) {
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::EditorScreen;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
					//                g_GUISound.UserErrorSound()->Play();
				}

				// Editor button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ModManagerButton)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::ModManagerScreen;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Credits button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::CreditsButton)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::CreditsScreen;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Quit button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitButton)) {
					QuitLogic();
					g_GUISound.ButtonPressSound()->Play();
				}

				// Resume button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ResumeButton)) {
					m_ActivityResumed = true;

					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// EDITOR SCREEN BUTTONS

				if (m_ActiveMenuScreen == MenuScreen::EditorScreen &&
					(guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SceneEditorButton) ||
						guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AreaEditorButton) ||
						guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AssemblyEditorButton) ||
						guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::GitEditorButton) ||
						guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ActorEditorButton))) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_ActiveMenuScreen = MenuScreen::MainScreen;
					m_ScreenChange = true;

					m_ActivityRestarted = true;

					// Create and start the appropriate editor Activity
					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SceneEditorButton)) {
						StartSceneEditor();
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AreaEditorButton)) {
						StartAreaEditor();
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AssemblyEditorButton)) {
						StartAssemblyEditor();
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::GitEditorButton)) {
						StartGibEditor();
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ActorEditorButton)) {
						StartActorEditor();
					}

					//                g_GUISound.BackButtonPressSound()->Play();
					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// META NOTICE SCREEN BUTTONS

				if (m_ActiveMenuScreen == MenuScreen::CampaignScreen) {
					// Play tutorial button pressed
					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::PlayTutorialButton)) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_ActiveMenuScreen = MenuScreen::MainScreen;
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
					else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::CampaignContinueButton)) {
						m_CampaignStarted = true;

						// Hide all screens, and stay in main menu for if/when player comes back to the main menu
						HideAllScreens();
						m_ActiveMenuScreen = MenuScreen::MainScreen;
						m_ScreenChange = true;
						g_GUISound.ButtonPressSound()->Play();
					}
				}

				/////////////////////////////////////////////
				// MOD MANAGER SCREEN BUTTONS

				if (m_ActiveMenuScreen == MenuScreen::ModManagerScreen) {
					;
				}

				/////////////////////////////////////////////
				// QUIT SCREEN BUTTONS

				if (m_ActiveMenuScreen == MenuScreen::QuitScreen) {
					// Confirm quitting of game
					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitConfirmButton)) {
						m_Quit = true;

						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitCancelButton)) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_ActiveMenuScreen = MenuScreen::MainScreen;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
				}
			}

			// Notifications
			else if (guiEvent.GetType() == GUIEvent::Notification) {
				// Button focus notification that we can play a sound to
				if (dynamic_cast<GUIButton *>(guiEvent.GetControl())) {
					if (guiEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Update() {
		m_Controller->Update();

		// Reset the specific triggers
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;

		m_Quit = false;

		// Don't update the main menu if the console is open
		if (g_ConsoleMan.IsEnabled()) {
			return;
		}

		// If esc pressed, show quit dialog if applicable
		if (g_UInputMan.KeyPressed(KEY_ESC)) {
			if (m_ActiveMenuScreen == MenuScreen::SettingsScreen || m_ActiveMenuScreen == MenuScreen::ModManagerScreen || m_ActiveMenuScreen == MenuScreen::EditorScreen || m_ActiveMenuScreen == MenuScreen::CreditsScreen) {
				HideAllScreens();
				m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
				m_ActiveMenuScreen = MenuScreen::MainScreen;
				m_ScreenChange = true;
				g_GUISound.BackButtonPressSound()->Play();

				if (m_ActiveMenuScreen == MenuScreen::SettingsScreen) {
					//g_SettingsMan.SetFlashOnBrainDamage(m_aOptionsCheckbox.at(FLASHONBRAINDAMAGE)->GetCheck());
					//g_SettingsMan.SetBlipOnRevealUnseen(m_aOptionsCheckbox.at(BLIPONREVEALUNSEEN)->GetCheck());
					//g_SettingsMan.SetShowForeignItems(m_aOptionsCheckbox.at(SHOWFOREIGNITEMS)->GetCheck());
					//g_SettingsMan.SetShowToolTips(m_aOptionsCheckbox.at(SHOWTOOLTIPS)->GetCheck());
					g_SettingsMan.UpdateSettingsFile();
				} else if (m_ActiveMenuScreen == MenuScreen::ModManagerScreen) {
					g_SettingsMan.UpdateSettingsFile();
				}
			} else {
				QuitLogic();
			}
		}

		// Quit now if we aren't enabled
		if (!m_MenuEnabled) {
			return;
		}

		switch (m_ActiveMenuScreen) {
			case MenuScreen::MainScreen:
				if (m_ScreenChange) {
					if (m_MainMenuScreens.at(MenuScreen::Root)->GetHeight() != m_RootCollectionBoxOriginalHeight) {
						m_MainMenuScreens.at(MenuScreen::Root)->Resize(m_MainMenuScreens.at(MenuScreen::Root)->GetWidth(), m_RootCollectionBoxOriginalHeight);
					}
					m_MainMenuScreens.at(MenuScreen::MainScreen)->SetVisible(true);

					if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
						m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(128, 220);
						m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(true);
					} else {
						m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(128, 196);
						m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(false);
					}
					// Restore the label on the campaign button
					m_MainMenuButtons.at(MenuButton::CampaignButton)->SetText("MetaGame (WIP)");

					m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
					m_MainMenuButtons.at(MenuButton::PlayTutorialButton)->SetVisible(false);
					m_MainMenuButtons.at(MenuButton::CampaignContinueButton)->SetVisible(false);
					m_ScreenChange = false;
				}

				// Blink the resume button to show the game is still going
				if (m_MainMenuButtons.at(MenuButton::ResumeButton)->GetVisible()) {
					if (m_BlinkTimer.AlternateReal(500)) {
						m_MainMenuButtons.at(MenuButton::ResumeButton)->SetFocus();
					} else {
						m_GUIControlManager->GetManager()->SetFocus(nullptr);
					}
				}
				break;
			case MenuScreen::SettingsScreen:
				/*
				if (m_ScreenChange) {
					m_MainMenuScreens.at(MenuScreen::SettingsScreen)->SetVisible(true);
					m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
					m_MainMenuScreens.at(MenuScreen::SettingsScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));
					m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionRel(180, 220);
					//m_pBackToOptionsButton->SetVisible(false);
					//UpdateDeviceLabels();
					m_ScreenChange = false;
				}
				*/
				break;
			case MenuScreen::ModManagerScreen:
				/*
				if (m_ScreenChange) { m_MainMenuScreens.at(MenuScreen::ModManagerScreen)->SetVisible(true); }
				*/
				break;
			case MenuScreen::EditorScreen:
				if (m_ScreenChange) {
					m_MainMenuScreens.at(MenuScreen::EditorScreen)->SetVisible(true);
					m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
					m_MainMenuScreens.at(MenuScreen::EditorScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));
					m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionRel(4, 145);
					m_ScreenChange = false;
				}
				break;
			case MenuScreen::CreditsScreen:
				if (m_ScreenChange) { ShowCreditsScreen(); }
				RollCredits();
				break;
			case MenuScreen::CampaignScreen:
				if (m_ScreenChange) {
					m_MainMenuScreens.at(MenuScreen::CampaignScreen)->SetVisible(true);
					m_MainMenuButtons.at(MenuButton::PlayTutorialButton)->SetVisible(true);
					m_MainMenuButtons.at(MenuButton::CampaignContinueButton)->SetVisible(true);
					m_MetaNoticeLabel->SetText("- A T T E N T I O N -\n\nPlease note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\nAs such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\nThat said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\nAlso, if you have not yet played Cortex Command, we recommend you first try the tutorial:");
					m_MetaNoticeLabel->SetVisible(true);
					// Flag that this notice has now been shown once, so no need to keep showing it
					m_TutorialOffered = true;
					m_ScreenChange = false;
				}
				break;
			case MenuScreen::QuitScreen:
				if (m_ScreenChange) {
					m_MainMenuScreens.at(MenuScreen::QuitScreen)->SetVisible(true);
					m_ScreenChange = false;
				}
				break;
			default:
				break;
		}

		//////////////////////////////////////
		// CONFIGURATION SCREEN
		/*
		else if (m_ActiveMenuScreen == CONFIGSCREEN) {
			if (m_ScreenChange) {
				m_MainMenuScreens.at(CONFIGSCREEN)->SetVisible(true);
				m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
				//m_pBackToOptionsButton->SetVisible(true);
				// Let this pass through, UpdateConfigScreen uses it
				//m_ScreenChange = false;
			}

			// Continuously update the contents through all the config steps
			//UpdateConfigScreen();
		}
		*/

		m_GUIControlManager->Update();
		HandleInputEvents();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw(BITMAP *drawBitmap) const {
		AllegroScreen drawScreen(drawBitmap);
		m_GUIControlManager->Draw(&drawScreen);
		m_GUIControlManager->DrawMouse();
	}
}