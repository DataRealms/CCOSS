#include "MainMenuGUI.h"

#include "FrameMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Clear() {
		m_GUIControlManager = nullptr;
		m_ActiveDialogBox = nullptr;

		m_ActiveMenuScreen = MenuScreen::ScreenCount;
		m_UpdateResult = MainMenuUpdateResult::NoEvent;
		m_MenuScreenChange = false;
		m_CampaignNoticeShown = false;

		m_RootBoxOriginalHeight = 0;

		m_ResumeButtonBlinkTimer.Reset();
		m_CreditsScrollTimer.Reset();

		m_SettingsMenu = nullptr;
		m_ModManagerMenu = nullptr;

		m_RootBox = nullptr;
		m_VersionLabel = nullptr;
		m_CreditsTextLabel = nullptr;
		m_CreditsScrollPanel = nullptr;
		m_MainMenuScreens.fill(nullptr);
		m_MainMenuButtons.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Create(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/MainMenuGUI.ini");

		m_RootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		m_RootBox->SetVisible(true);
		m_RootBox->SetPositionAbs((g_FrameMan.GetResX() - m_RootBox->GetWidth()) / 2, 0);
		m_RootBoxOriginalHeight = m_RootBox->GetHeight();

		m_VersionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("VersionLabel"));
		m_VersionLabel->SetText(c_GameVersion);
		m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 5);

		m_MainMenuScreens.at(MenuScreen::MainScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("MainScreen"));
		m_MainMenuScreens.at(MenuScreen::QuitScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("QuitConfirmBox"));
		m_MainMenuScreens.at(MenuScreen::QuitScreen)->CenterInParent(true, true);

		m_MainMenuButtons.at(MenuButton::CampaignButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToCampaign"));
		m_MainMenuButtons.at(MenuButton::ScenarioButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToSkirmish"));
		m_MainMenuButtons.at(MenuButton::MultiplayerButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToMultiplayer"));
		m_MainMenuButtons.at(MenuButton::SettingsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToOptions"));
		m_MainMenuButtons.at(MenuButton::ModManagerButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToModManager"));
		m_MainMenuButtons.at(MenuButton::EditorsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToEditor"));
		m_MainMenuButtons.at(MenuButton::CreditsButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonMainToCreds"));
		m_MainMenuButtons.at(MenuButton::QuitButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuit"));
		m_MainMenuButtons.at(MenuButton::ResumeButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonResume"));
		m_MainMenuButtons.at(MenuButton::BackToMainButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMain"));
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
		m_MainMenuButtons.at(MenuButton::QuitConfirmButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("QuitConfirmButton"));
		m_MainMenuButtons.at(MenuButton::QuitCancelButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("QuitCancelButton"));

		CreateCampaignNoticeScreen();
		CreateEditorsScreen();
		CreateCreditsScreen();

		m_SettingsMenu = std::make_unique<SettingsGUI>(guiScreen, guiInput);
		m_ModManagerMenu = std::make_unique<ModManagerGUI>(guiScreen, guiInput);

		// Set the active screen to the settings screen otherwise we're at the main screen after reinitializing.
		SetActiveMenuScreen(g_FrameMan.ResolutionChanged() ? MenuScreen::SettingsScreen : MenuScreen::MainScreen, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateCampaignNoticeScreen() {
		m_MainMenuScreens.at(MenuScreen::CampaignNoticeScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("MetaScreen"));

		m_MainMenuButtons.at(MenuButton::PlayTutorialButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonTutorial"));
		m_MainMenuButtons.at(MenuButton::CampaignContinueButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonContinue"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateEditorsScreen() {
		m_MainMenuScreens.at(MenuScreen::EditorScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("EditorScreen"));

		m_MainMenuButtons.at(MenuButton::SceneEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonSceneEditor"));
		m_MainMenuButtons.at(MenuButton::AreaEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonAreaEditor"));
		m_MainMenuButtons.at(MenuButton::AssemblyEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonAssemblyEditor"));
		m_MainMenuButtons.at(MenuButton::GitEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonGibPlacement"));
		m_MainMenuButtons.at(MenuButton::ActorEditorButton) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonActorEditor"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateCreditsScreen() {
		m_MainMenuScreens.at(MenuScreen::CreditsScreen) = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CreditsScreen"));
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->Resize(m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GetWidth(), g_FrameMan.GetResY());

		m_CreditsScrollPanel = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CreditsPanel"));
		m_CreditsTextLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("CreditsLabel"));

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
		m_CreditsTextLabel->SetText(creditsText);
		m_CreditsTextLabel->ResizeHeightToFit();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HideAllScreens() {
		for (GUICollectionBox *menuScreen : m_MainMenuScreens) {
			if (menuScreen) { menuScreen->SetVisible(false); }
		}
		m_MenuScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::SetActiveMenuScreen(MenuScreen screenToShow, bool playButtonPressSound) {
		if (screenToShow != m_ActiveMenuScreen) {
			HideAllScreens();
			m_ActiveMenuScreen = screenToShow;
			m_MenuScreenChange = true;
		}
		if (playButtonPressSound) { g_GUISound.ButtonPressSound()->Play(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowMainScreen() {
		if (m_RootBox->GetHeight() != m_RootBoxOriginalHeight) { m_RootBox->Resize(m_RootBox->GetWidth(), m_RootBoxOriginalHeight); }
		m_VersionLabel->SetVisible(true);

		m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(128, 196);
		m_MainMenuScreens.at(MenuScreen::MainScreen)->SetVisible(true);

		m_MainMenuButtons.at(MenuButton::CampaignButton)->SetText("MetaGame (WIP)");
		m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(false);

		m_MenuScreenChange = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowCampaignNoticeScreen() {
		m_MainMenuScreens.at(MenuScreen::CampaignNoticeScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::CampaignNoticeScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuButtons.at(MenuButton::BackToMainButton)->GetWidth()) / 2, m_MainMenuButtons.at(MenuButton::CampaignContinueButton)->GetYPos() + 25);

		GUILabel *metaNoticeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("MetaLabel"));

		const char *metaNotice = {
			"- A T T E N T I O N -\n\n"
			"Please note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\n"
			"As such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\n"
			"That said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\n"
			"Also, if you have not yet played Cortex Command, we recommend you first try the tutorial:"
		};
		metaNoticeLabel->SetText(metaNotice);
		metaNoticeLabel->SetVisible(true);

		// Flag that this notice has now been shown once, so no need to keep showing it
		m_CampaignNoticeShown = true;

		m_MenuScreenChange = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowEditorsScreen() {
		m_MainMenuScreens.at(MenuScreen::EditorScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::EditorScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionRel(4, 145);

		m_MenuScreenChange = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowCreditsScreen() {
		m_RootBox->Resize(m_RootBox->GetWidth(), g_FrameMan.GetResY() - m_RootBox->GetYPos() - 10);
		m_VersionLabel->SetVisible(false);

		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		int scrollPanelHeight = m_RootBox->GetHeight() - m_CreditsScrollPanel->GetYPos() - 30;
		if (m_CreditsScrollPanel->GetHeight() != scrollPanelHeight) { m_CreditsScrollPanel->Resize(m_CreditsScrollPanel->GetWidth(), scrollPanelHeight); }
		m_CreditsScrollPanel->SetPositionRel(0, 0);
		m_CreditsTextLabel->SetPositionRel(0, scrollPanelHeight);
		m_CreditsScrollTimer.Reset();

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuButtons.at(MenuButton::BackToMainButton)->GetWidth()) / 2, m_CreditsScrollPanel->GetYPos() + scrollPanelHeight + 10);

		m_MenuScreenChange = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowQuitScreenOrQuit() {
		if (m_ActiveMenuScreen != MenuScreen::QuitScreen && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			SetActiveMenuScreen(MenuScreen::QuitScreen);
			m_MainMenuScreens.at(MenuScreen::QuitScreen)->SetVisible(true);
			m_MenuScreenChange = false;
		} else {
			m_UpdateResult = MainMenuUpdateResult::Quit;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowAndBlinkResumeButton() {
		if (!m_MainMenuButtons.at(MenuButton::ResumeButton)->GetVisible()) {
			if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
				m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(128, 220);
				m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(true);
			}
		} else {
			if (m_ResumeButtonBlinkTimer.AlternateReal(500)) {
				m_MainMenuButtons.at(MenuButton::ResumeButton)->SetFocus();
			} else {
				m_GUIControlManager->GetManager()->SetFocus(nullptr);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MainMenuGUI::RollCredits() {
		int scrollDuration = m_CreditsTextLabel->GetHeight() * 50;
		float scrollDist = static_cast<float>(m_CreditsScrollPanel->GetHeight() + m_CreditsTextLabel->GetHeight());
		float scrollProgress = static_cast<float>(m_CreditsScrollTimer.GetElapsedRealTimeMS()) / static_cast<float>(scrollDuration);
		m_CreditsTextLabel->SetPositionRel(0, m_CreditsScrollPanel->GetHeight() - static_cast<int>(scrollDist * scrollProgress));

		if (m_CreditsScrollTimer.IsPastRealMS(scrollDuration + 1000)) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	MainMenuGUI::MainMenuUpdateResult MainMenuGUI::Update() {
		m_UpdateResult = MainMenuUpdateResult::NoEvent;

		if (g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) {
			return m_UpdateResult;
		}

		bool backToMainMenu = false;

		switch (m_ActiveMenuScreen) {
			case MenuScreen::MainScreen:
				if (m_MenuScreenChange) { ShowMainScreen(); }
				ShowAndBlinkResumeButton();
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::CampaignNoticeScreen:
				if (m_MenuScreenChange) { ShowCampaignNoticeScreen(); }
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::SettingsScreen:
				backToMainMenu = m_SettingsMenu->HandleInputEvents();
				m_ActiveDialogBox = m_SettingsMenu->GetActiveDialogBox();
				break;
			case MenuScreen::ModManagerScreen:
				backToMainMenu = m_ModManagerMenu->HandleInputEvents();
				break;
			case MenuScreen::EditorScreen:
				if (m_MenuScreenChange) { ShowEditorsScreen(); }
				backToMainMenu = HandleInputEvents();
				break;
			case MenuScreen::CreditsScreen:
				if (m_MenuScreenChange) { ShowCreditsScreen(); }
				backToMainMenu = RollCredits() ? true : HandleInputEvents();
				break;
			case MenuScreen::QuitScreen:
				backToMainMenu = HandleInputEvents();
				m_ActiveDialogBox = m_MainMenuScreens.at(MenuScreen::QuitScreen);
				break;
			default:
				break;
		}

		// If esc pressed, show quit dialog if applicable
		if (backToMainMenu || g_UInputMan.KeyPressed(KEY_ESC)) {
			if (m_ActiveMenuScreen != MenuScreen::MainScreen) {
				if (m_ActiveMenuScreen == MenuScreen::SettingsScreen || m_ActiveMenuScreen == MenuScreen::ModManagerScreen) {
					g_SettingsMan.UpdateSettingsFile();
				} else if (m_ActiveMenuScreen == MenuScreen::CreditsScreen) {
					m_UpdateResult = MainMenuUpdateResult::BackToMainFromCredits;
				}
				SetActiveMenuScreen(MenuScreen::MainScreen, false);
				g_GUISound.BackButtonPressSound()->Play();
			} else {
				ShowQuitScreenOrQuit();
			}
		}
		return m_UpdateResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MainMenuGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::BackToMainButton)) {
					return true;
				}
				switch (m_ActiveMenuScreen) {
					case MenuScreen::MainScreen:
						HandleMainScreenInputEvents(guiEvent.GetControl());
						break;
					case MenuScreen::CampaignNoticeScreen:
						HandleCampaignNoticeScreenInputEvents(guiEvent.GetControl());
						break;
					case MenuScreen::EditorScreen:
						HandleEditorsScreenInputEvents(guiEvent.GetControl());
						break;
					case MenuScreen::QuitScreen:
						HandleQuitScreenInputEvents(guiEvent.GetControl());
						break;
					default:
						break;
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification && (dynamic_cast<GUIButton *>(guiEvent.GetControl()) && guiEvent.GetMsg() == GUIButton::Focused)) {
				g_GUISound.SelectionChangeSound()->Play();
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleMainScreenInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::CampaignButton)) {
			if (!m_CampaignNoticeShown) {
				SetActiveMenuScreen(MenuScreen::CampaignNoticeScreen);
			} else {
				m_UpdateResult = MainMenuUpdateResult::CampaignStarted;
				SetActiveMenuScreen(MenuScreen::MainScreen);
			}
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ScenarioButton)) {
			m_UpdateResult = MainMenuUpdateResult::ScenarioStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::MultiplayerButton)) {
			m_UpdateResult = MainMenuUpdateResult::ActivityStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen, false);
			g_GUISound.BackButtonPressSound()->Play();
			g_ActivityMan.SetStartMultiplayerActivity();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::SettingsButton)) {
			SetActiveMenuScreen(MenuScreen::SettingsScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::EditorsButton)) {
			SetActiveMenuScreen(MenuScreen::EditorScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ModManagerButton)) {
			SetActiveMenuScreen(MenuScreen::ModManagerScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::CreditsButton)) {
			SetActiveMenuScreen(MenuScreen::CreditsScreen);
			m_UpdateResult = MainMenuUpdateResult::EnterCreditsScreen;
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::QuitButton)) {
			ShowQuitScreenOrQuit();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ResumeButton)) {
			m_UpdateResult = MainMenuUpdateResult::ActivityResumed;
			g_GUISound.BackButtonPressSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleCampaignNoticeScreenInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::PlayTutorialButton)) {
			m_UpdateResult = MainMenuUpdateResult::ActivityStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::CampaignContinueButton)) {
			m_UpdateResult = MainMenuUpdateResult::CampaignStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleEditorsScreenInputEvents(const GUIControl *guiEventControl) {
		std::string editorToStart;
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::SceneEditorButton)) {
			editorToStart = "SceneEditor";
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::AreaEditorButton)) {
			editorToStart = "AreaEditor";
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::AssemblyEditorButton)) {
			editorToStart = "AssemblyEditor";
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::GitEditorButton)) {
			editorToStart = "GibEditor";
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ActorEditorButton)) {
			editorToStart = "ActorEditor";
		}
		if (!editorToStart.empty()) {
			m_UpdateResult = MainMenuUpdateResult::ActivityStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen, false);
			g_GUISound.ExitMenuSound()->Play();
			g_ActivityMan.SetStartEditorActivity(editorToStart);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleQuitScreenInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::QuitConfirmButton)) {
			m_UpdateResult = MainMenuUpdateResult::Quit;
			g_GUISound.ButtonPressSound()->Play();
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::QuitCancelButton)) {
			SetActiveMenuScreen(MenuScreen::MainScreen);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw() {
		switch (m_ActiveMenuScreen) {
			case MenuScreen::SettingsScreen:
				m_SettingsMenu->Draw();
				break;
			case MenuScreen::ModManagerScreen:
				m_ModManagerMenu->Draw();
				break;
			default:
				m_GUIControlManager->Draw();
				break;
		}
		if (m_ActiveDialogBox) {
			set_trans_blender(128, 128, 128, 128);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), g_FrameMan.GetOverlayBitmap32(), 0, 0);
			// Whatever this box may be at this point it's already been drawn by the owning GUIControlManager, but we need to draw it again on top of the overlay so it's not affected by it.
			m_ActiveDialogBox->Draw(m_GUIControlManager->GetScreen());
			m_ActiveDialogBox = nullptr;
		}
		m_GUIControlManager->DrawMouse();
	}
}