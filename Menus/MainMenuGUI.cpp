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

		m_MainMenuScreens.fill(nullptr);
		m_MainMenuButtons.fill(nullptr);

		m_ModManagerMenu = nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Create(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller) {
		m_Controller = controller;

		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		if (!m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_GUIControlManager->Load("Base.rte/GUIs/MainMenuGUI.ini");

		m_RootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		m_RootBox->SetVisible(true);
		m_RootBox->SetPositionAbs((g_FrameMan.GetResX() - m_RootBox->GetWidth()) / 2, 0);
		m_RootBoxOriginalHeight = m_RootBox->GetHeight();

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
		m_MetaNoticeLabel->SetText("- A T T E N T I O N -\n\nPlease note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\nAs such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\nThat said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\nAlso, if you have not yet played Cortex Command, we recommend you first try the tutorial:");

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

		m_SettingsMenu = std::make_unique<SettingsGUI>(guiScreen, guiInput, controller);
		m_ModManagerMenu = std::make_unique<ModManagerGUI>(guiScreen, guiInput);

		m_ScreenChange = true;
		HideAllScreens();
	}

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

	void MainMenuGUI::StartEditorActivity(const std::string_view &editorToLaunch) const {
		EditorActivity *editorActivityToStart = nullptr;

		if (editorToLaunch == "ActorEditor") {
			editorActivityToStart = new ActorEditor;
		} else if (editorToLaunch == "GibEditor") {
			editorActivityToStart = new GibEditor;
		} else if (editorToLaunch == "SceneEditor") {
			editorActivityToStart = new SceneEditor;
		} else if (editorToLaunch == "AreaEditor") {
			editorActivityToStart = new AreaEditor;
		} else if (editorToLaunch == "AssemblyEditor") {
			editorActivityToStart = new AssemblyEditor;
		}
		if (editorActivityToStart) {
			g_SceneMan.SetSceneToLoad("Editor Scene");
			editorActivityToStart->Create();
			editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
			g_ActivityMan.SetStartActivity(editorActivityToStart);
		} else {
			RTEAbort("Failed to instantiate the " + std::string(editorToLaunch) + " Activity!");
		}
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

	void MainMenuGUI::HideAllScreens() {
		for (GUICollectionBox *menuScreen : m_MainMenuScreens) {
			if (menuScreen) { menuScreen->SetVisible(false); }
		}
		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowMainScreen() {
		m_ScreenChange = false;
		if (m_RootBox->GetHeight() != m_RootBoxOriginalHeight) { m_RootBox->Resize(m_RootBox->GetWidth(), m_RootBoxOriginalHeight); }
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
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowCampaignScreen() {
		m_ScreenChange = false;
		m_MainMenuScreens.at(MenuScreen::CampaignScreen)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::PlayTutorialButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::CampaignContinueButton)->SetVisible(true);

		m_MetaNoticeLabel->SetVisible(true);

		// Flag that this notice has now been shown once, so no need to keep showing it
		m_TutorialOffered = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowEditorsScreen() {
		m_ScreenChange = false;
		m_MainMenuScreens.at(MenuScreen::EditorScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::EditorScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionRel(4, 145);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowCreditsScreen() {
		m_ScreenChange = false;
		m_RootBox->Resize(m_RootBox->GetWidth(), g_FrameMan.GetResY() - m_RootBox->GetYPos() - 10);

		int scrollPanelHeight = m_RootBox->GetHeight() - m_CreditsScrollPanel->GetYPos() - 30;
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

	bool MainMenuGUI::RollCredits() {
		int scrollTime = m_CreditsLabel->GetHeight() * 50;
		int scrollDist = m_CreditsScrollPanel->GetHeight() + m_CreditsLabel->GetHeight();
		float scrollProgress = static_cast<float>(m_ScrollTimer.GetElapsedRealTimeMS()) / static_cast<float>(scrollTime);
		m_CreditsLabel->SetPositionRel(0, m_CreditsScrollPanel->GetHeight() - static_cast<int>(static_cast<float>(scrollDist) * scrollProgress));

		if (m_ScrollTimer.IsPastRealMS(scrollTime + 1000)) {
			m_ScreenChange = true;
			HideAllScreens();
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MainMenuGUI::HandleMainScreenButtonPresses(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::BackToMainButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			//m_ActiveMenuScreen = MenuScreen::MainScreen;
			return true;
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::CampaignButton)) {
			if (!m_TutorialOffered) {
				m_ActiveMenuScreen = MenuScreen::CampaignScreen;
			} else {
				m_CampaignStarted = true;
				m_ActiveMenuScreen = MenuScreen::MainScreen;
			}
			m_ScreenChange = true;
			HideAllScreens();
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::SkirmishButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			m_ScenarioStarted = true;
			m_CampaignStarted = false;

			if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

			m_ActiveMenuScreen = MenuScreen::MainScreen;
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::MultiplayerButton)) {
			m_ScreenChange = true;
			HideAllScreens();

			m_ScenarioStarted = true;
			m_CampaignStarted = false;

			if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

			g_GUISound.ButtonPressSound()->Play();

			m_ActiveMenuScreen = MenuScreen::MainScreen;

			m_ActivityRestarted = true;
			g_GUISound.ExitMenuSound()->Play();

			g_SceneMan.SetSceneToLoad("Editor Scene");
			MultiplayerGame *pMultiplayerGame = new MultiplayerGame;
			pMultiplayerGame->Create();
			g_ActivityMan.SetStartActivity(pMultiplayerGame);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::SettingsButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			m_ActiveMenuScreen = MenuScreen::SettingsScreen;
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::EditorsButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			m_ActiveMenuScreen = MenuScreen::EditorScreen;

			m_CampaignStarted = false;
			if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ModManagerButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			m_ActiveMenuScreen = MenuScreen::ModManagerScreen;
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::CreditsButton)) {
			m_ScreenChange = true;
			HideAllScreens();
			m_ActiveMenuScreen = MenuScreen::CreditsScreen;
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::QuitButton)) {
			QuitLogic();
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ResumeButton)) {
			m_ActivityResumed = true;
			g_GUISound.ExitMenuSound()->Play();
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MainMenuGUI::HandleInputEvents() {
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;

		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (HandleMainScreenButtonPresses(guiEvent.GetControl())) {
					return true;
				}

				if (m_ActiveMenuScreen == MenuScreen::EditorScreen && (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SceneEditorButton) || guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AreaEditorButton) ||
					guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AssemblyEditorButton) || guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::GitEditorButton) || guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ActorEditorButton))) {

					m_ScreenChange = true;
					HideAllScreens();
					m_ActivityRestarted = true;
					m_ActiveMenuScreen = MenuScreen::MainScreen;
					g_GUISound.ExitMenuSound()->Play();

					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::SceneEditorButton)) {
						StartEditorActivity("SceneEditor");
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AreaEditorButton)) {
						StartEditorActivity("EditorActivity");
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::AssemblyEditorButton)) {
						StartEditorActivity("AssemblyEditor");
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::GitEditorButton)) {
						StartEditorActivity("GibEditor");
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::ActorEditorButton)) {
						StartEditorActivity("ActorEditor");
					}
				} else if (m_ActiveMenuScreen == MenuScreen::CampaignScreen && (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::PlayTutorialButton) || guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::CampaignContinueButton))) {
					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::PlayTutorialButton)) {
						g_ActivityMan.SetStartActivity(dynamic_cast<Activity *>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
						if (GameActivity * gameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetStartActivity())) { gameActivity->SetStartingGold(10000); }
						g_SceneMan.SetSceneToLoad("Tutorial Bunker");
						m_ActivityRestarted = true;
						m_ActiveMenuScreen = MenuScreen::MainScreen;
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::CampaignContinueButton)) {
						m_CampaignStarted = true;
						m_ActiveMenuScreen = MenuScreen::MainScreen;
					}
					m_ScreenChange = true;
					HideAllScreens();
					g_GUISound.ButtonPressSound()->Play();
				} else if (m_ActiveMenuScreen == MenuScreen::QuitScreen && (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitConfirmButton) || guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitCancelButton))) {
					if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitConfirmButton)) {
						m_Quit = true;
					} else if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::QuitCancelButton)) {
						m_ActiveMenuScreen = MenuScreen::MainScreen;
					}
					m_ScreenChange = true;
					HideAllScreens();
					g_GUISound.ButtonPressSound()->Play();
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				// Button focus notification that we can play a sound to
				if (dynamic_cast<GUIButton *>(guiEvent.GetControl()) && guiEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Update() {
		//m_Controller->Update();

		m_Quit = false;

		if (!m_MenuEnabled || g_ConsoleMan.IsEnabled()) {
			return;
		}

		bool backToMainMenu = false;

		switch (m_ActiveMenuScreen) {
			case MenuScreen::MainScreen:
				if (m_ScreenChange) { ShowMainScreen(); }
				// Blink the resume button to show the game is still going
				if (m_MainMenuButtons.at(MenuButton::ResumeButton)->GetVisible()) {
					if (m_BlinkTimer.AlternateReal(500)) {
						m_MainMenuButtons.at(MenuButton::ResumeButton)->SetFocus();
					} else {
						m_GUIControlManager->GetManager()->SetFocus(nullptr);
					}
				}
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::SettingsScreen:
				if (m_ScreenChange) { m_SettingsMenu->SetEnabled(); }
				backToMainMenu = m_SettingsMenu->HandleInputEvents();
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
				if (m_ScreenChange) { m_ModManagerMenu->SetEnabled(); }
				backToMainMenu = m_ModManagerMenu->HandleInputEvents();
				break;
			case MenuScreen::EditorScreen:
				if (m_ScreenChange) { ShowEditorsScreen(); }
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::CreditsScreen:
				if (m_ScreenChange) { ShowCreditsScreen(); }
				backToMainMenu = RollCredits() ? true : HandleInputEvents();
				break;
			case MenuScreen::CampaignScreen:
				if (m_ScreenChange) { ShowCampaignScreen(); }
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::QuitScreen:
				if (m_ScreenChange) {
					m_ScreenChange = false;
					m_MainMenuScreens.at(MenuScreen::QuitScreen)->SetVisible(true);
				}
				backToMainMenu = HandleInputEvents();
				break;
			default:
				break;
		}

		// If esc pressed, show quit dialog if applicable
		if (backToMainMenu || g_UInputMan.KeyPressed(KEY_ESC)) {
			if (m_ActiveMenuScreen == MenuScreen::SettingsScreen || m_ActiveMenuScreen == MenuScreen::ModManagerScreen || m_ActiveMenuScreen == MenuScreen::EditorScreen || m_ActiveMenuScreen == MenuScreen::CreditsScreen) {
				HideAllScreens();
				m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
				m_ActiveMenuScreen = MenuScreen::MainScreen;
				m_ScreenChange = true;
				g_GUISound.BackButtonPressSound()->Play();
				m_ReturnToMainScreen = true;

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
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw(BITMAP *drawBitmap) const {
		switch (m_ActiveMenuScreen) {
			case MenuScreen::SettingsScreen:
				m_SettingsMenu->Draw();
				break;
			case MenuScreen::ModManagerScreen:
				m_ModManagerMenu->Draw();
				break;
			default:
				AllegroScreen drawScreen(drawBitmap);
				m_GUIControlManager->Draw(&drawScreen);
				break;
		}
		m_GUIControlManager->DrawMouse();
	}
}