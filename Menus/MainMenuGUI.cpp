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
		m_MainMenuScreenGUIControlManager = nullptr;
		m_SubMenuScreenGUIControlManager = nullptr;
		m_ActiveGUIControlManager = nullptr;
		m_ActiveDialogBox = nullptr;

		m_ActiveMenuScreen = MenuScreen::ScreenCount;
		m_UpdateResult = MainMenuUpdateResult::NoEvent;
		m_MenuScreenChange = false;
		m_MetaGameNoticeShown = false;

		m_ResumeButtonBlinkTimer.Reset();
		m_CreditsScrollTimer.Reset();

		m_SettingsMenu = nullptr;
		m_ModManagerMenu = nullptr;

		m_VersionLabel = nullptr;
		m_CreditsTextLabel = nullptr;
		m_CreditsScrollPanel = nullptr;
		m_MainMenuScreens.fill(nullptr);
		m_MainMenuButtons.fill(nullptr);

		m_MainScreenButtonHoveredText.fill(std::string());
		m_MainScreenButtonUnhoveredText.fill(std::string());
		m_MainScreenHoveredButton = nullptr;
		m_MainScreenPrevHoveredButtonIndex = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Create(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		m_MainMenuScreenGUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_MainMenuScreenGUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuScreenSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuScreenSkin.ini");
		m_MainMenuScreenGUIControlManager->Load("Base.rte/GUIs/MainMenuGUI.ini");

		m_SubMenuScreenGUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_SubMenuScreenGUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_SubMenuScreenGUIControlManager->Load("Base.rte/GUIs/MainMenuSubMenuGUI.ini");

		GUICollectionBox *mainScreenRootBox = dynamic_cast<GUICollectionBox *>(m_MainMenuScreenGUIControlManager->GetControl("root"));
		mainScreenRootBox->Resize(g_FrameMan.GetResX(), mainScreenRootBox->GetHeight());

		GUICollectionBox *subMenuScreenRootBox = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("root"));
		subMenuScreenRootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		m_MainMenuButtons.at(MenuButton::BackToMainButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonBackToMain"));
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->CenterInParent(true, false);

		CreateMainScreen();
		CreateMetaGameNoticeScreen();
		CreateEditorsScreen();
		CreateCreditsScreen();
		CreateQuitScreen();

		m_SettingsMenu = std::make_unique<SettingsGUI>(guiScreen, guiInput);
		m_ModManagerMenu = std::make_unique<ModManagerGUI>(guiScreen, guiInput);

		// Set the active screen to the settings screen otherwise we're at the main screen after reinitializing.
		SetActiveMenuScreen(g_FrameMan.ResolutionChanged() ? MenuScreen::SettingsScreen : MenuScreen::MainScreen, false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateMainScreen() {
		m_MainMenuScreens.at(MenuScreen::MainScreen) = dynamic_cast<GUICollectionBox *>(m_MainMenuScreenGUIControlManager->GetControl("MainScreen"));
		m_MainMenuScreens.at(MenuScreen::MainScreen)->CenterInParent(true, false);

		m_MainMenuButtons.at(MenuButton::MetaGameButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToMetaGame"));
		m_MainMenuButtons.at(MenuButton::ScenarioButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToSkirmish"));
		m_MainMenuButtons.at(MenuButton::MultiplayerButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToMultiplayer"));
		m_MainMenuButtons.at(MenuButton::SettingsButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToOptions"));
		m_MainMenuButtons.at(MenuButton::ModManagerButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToModManager"));
		m_MainMenuButtons.at(MenuButton::EditorsButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToEditor"));
		m_MainMenuButtons.at(MenuButton::CreditsButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonMainToCreds"));
		m_MainMenuButtons.at(MenuButton::QuitButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonQuit"));
		m_MainMenuButtons.at(MenuButton::ResumeButton) = dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControl("ButtonResume"));

		for (int mainScreenButton = 0; mainScreenButton < 9; ++mainScreenButton) {
			m_MainMenuButtons.at(mainScreenButton)->CenterInParent(true, false);
			std::string buttonText = m_MainMenuButtons.at(mainScreenButton)->GetText();
			std::transform(buttonText.begin(), buttonText.end(), buttonText.begin(), ::toupper);
			m_MainScreenButtonHoveredText.at(mainScreenButton) = buttonText;
			std::transform(buttonText.begin(), buttonText.end(), buttonText.begin(), ::tolower);
			m_MainScreenButtonUnhoveredText.at(mainScreenButton) = buttonText;

			m_MainMenuButtons.at(mainScreenButton)->SetText(m_MainScreenButtonUnhoveredText.at(mainScreenButton));
		}

		m_VersionLabel = dynamic_cast<GUILabel *>(m_MainMenuScreenGUIControlManager->GetControl("VersionLabel"));
		m_VersionLabel->SetText("Community Project\n" + std::string(c_GameVersion));
		m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 5);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateMetaGameNoticeScreen() {
		m_MainMenuScreens.at(MenuScreen::MetaGameNoticeScreen) = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("MetaScreen"));
		m_MainMenuScreens.at(MenuScreen::MetaGameNoticeScreen)->CenterInParent(true, false);

		m_MainMenuButtons.at(MenuButton::PlayTutorialButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonTutorial"));
		m_MainMenuButtons.at(MenuButton::MetaGameContinueButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonContinue"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateEditorsScreen() {
		m_MainMenuScreens.at(MenuScreen::EditorScreen) = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("EditorScreen"));
		m_MainMenuScreens.at(MenuScreen::EditorScreen)->CenterInParent(true, false);

		m_MainMenuButtons.at(MenuButton::SceneEditorButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonSceneEditor"));
		m_MainMenuButtons.at(MenuButton::AreaEditorButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonAreaEditor"));
		m_MainMenuButtons.at(MenuButton::AssemblyEditorButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonAssemblyEditor"));
		m_MainMenuButtons.at(MenuButton::GibEditorButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonGibPlacement"));
		m_MainMenuButtons.at(MenuButton::ActorEditorButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("ButtonActorEditor"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::CreateCreditsScreen() {
		m_MainMenuScreens.at(MenuScreen::CreditsScreen) = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("CreditsScreen"));
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->Resize(m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GetWidth(), g_FrameMan.GetResY());
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->CenterInParent(true, false);

		m_CreditsScrollPanel = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("CreditsPanel"));
		m_CreditsScrollPanel->Resize(m_CreditsScrollPanel->GetWidth(), g_FrameMan.GetResY() - m_CreditsScrollPanel->GetYPos() - 50);

		m_CreditsTextLabel = dynamic_cast<GUILabel *>(m_SubMenuScreenGUIControlManager->GetControl("CreditsLabel"));

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

	void MainMenuGUI::CreateQuitScreen() {
		m_MainMenuScreens.at(MenuScreen::QuitScreen) = dynamic_cast<GUICollectionBox *>(m_SubMenuScreenGUIControlManager->GetControl("QuitConfirmBox"));
		m_MainMenuScreens.at(MenuScreen::QuitScreen)->CenterInParent(true, false);

		m_MainMenuButtons.at(MenuButton::QuitConfirmButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("QuitConfirmButton"));
		m_MainMenuButtons.at(MenuButton::QuitCancelButton) = dynamic_cast<GUIButton *>(m_SubMenuScreenGUIControlManager->GetControl("QuitCancelButton"));
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
			m_ActiveGUIControlManager = (m_ActiveMenuScreen == MenuScreen::MainScreen) ? m_MainMenuScreenGUIControlManager.get() : m_SubMenuScreenGUIControlManager.get();
			m_MenuScreenChange = true;

			if (playButtonPressSound) { g_GUISound.ButtonPressSound()->Play(); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowMainScreen() {
		m_VersionLabel->SetVisible(true);

		m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(300, 196);
		m_MainMenuScreens.at(MenuScreen::MainScreen)->SetVisible(true);

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(false);
		m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(false);

		m_MenuScreenChange = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::ShowMetaGameNoticeScreen() {
		m_MainMenuScreens.at(MenuScreen::MetaGameNoticeScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::MetaGameNoticeScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuButtons.at(MenuButton::BackToMainButton)->GetWidth()) / 2, m_MainMenuButtons.at(MenuButton::MetaGameContinueButton)->GetYPos() + 25);

		GUILabel *metaNoticeLabel = dynamic_cast<GUILabel *>(m_SubMenuScreenGUIControlManager->GetControl("MetaLabel"));

		std::string metaNotice = {
			"- A T T E N T I O N -\n\n"
			"Please note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\n"
			"As such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\n"
			"That said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\n"
			"Also, if you have not yet played Cortex Command, we recommend you first try the tutorial:"
		};
		metaNoticeLabel->SetText(metaNotice);
		metaNoticeLabel->SetVisible(true);

		// Flag that this notice has now been shown once, so no need to keep showing it
		m_MetaGameNoticeShown = true;

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
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->SetVisible(true);
		m_MainMenuScreens.at(MenuScreen::CreditsScreen)->GUIPanel::AddChild(m_MainMenuButtons.at(MenuButton::BackToMainButton));

		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetVisible(true);
		m_MainMenuButtons.at(MenuButton::BackToMainButton)->SetPositionAbs((g_FrameMan.GetResX() - m_MainMenuButtons.at(MenuButton::BackToMainButton)->GetWidth()) / 2, g_FrameMan.GetResY() - 35);

		m_VersionLabel->SetVisible(false);

		m_CreditsTextLabel->SetPositionRel(0, g_FrameMan.GetResY() - m_CreditsScrollPanel->GetYPos() - 50);
		m_CreditsScrollTimer.Reset();

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
			m_ResumeButtonBlinkTimer.Reset();
			if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
				m_MainMenuScreens.at(MenuScreen::MainScreen)->Resize(300, 220);
				m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(true);
			}
		} else {
			if (m_MainScreenHoveredButton && m_MainScreenHoveredButton == m_MainMenuButtons.at(MenuButton::ResumeButton)) {
				m_MainMenuButtons.at(MenuButton::ResumeButton)->SetText(m_ResumeButtonBlinkTimer.AlternateReal(500) ? m_MainScreenButtonHoveredText.at(MenuButton::ResumeButton) : "]" + m_MainScreenButtonHoveredText.at(MenuButton::ResumeButton) + "[");
			} else {
				m_MainMenuButtons.at(MenuButton::ResumeButton)->SetText(m_ResumeButtonBlinkTimer.AlternateReal(500) ? m_MainScreenButtonUnhoveredText.at(MenuButton::ResumeButton) : ">" + m_MainScreenButtonUnhoveredText.at(MenuButton::ResumeButton) + "<");
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
				backToMainMenu = HandleInputEvents();
				ShowAndBlinkResumeButton();
				break;
			case MenuScreen::MetaGameNoticeScreen:
				if (m_MenuScreenChange) { ShowMetaGameNoticeScreen(); }
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
				m_ActiveDialogBox = m_MainMenuScreens.at(MenuScreen::QuitScreen)->GetVisible() ? m_MainMenuScreens.at(MenuScreen::QuitScreen) : nullptr;
				break;
			default:
				break;
		}
		HandleBackNavigation(backToMainMenu);

		if (m_UpdateResult == MainMenuUpdateResult::ActivityStarted || m_UpdateResult == MainMenuUpdateResult::ActivityResumed) { m_MainMenuButtons.at(MenuButton::ResumeButton)->SetVisible(false); }
		return m_UpdateResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleBackNavigation(bool backButtonPressed) {
		if ((!m_ActiveDialogBox || m_ActiveDialogBox == m_MainMenuScreens.at(MenuScreen::QuitScreen)) && (backButtonPressed || g_UInputMan.KeyPressed(KEY_ESC))) {
			if (m_ActiveMenuScreen != MenuScreen::MainScreen) {
				if (m_ActiveMenuScreen == MenuScreen::SettingsScreen || m_ActiveMenuScreen == MenuScreen::ModManagerScreen) {
					if (m_ActiveMenuScreen == MenuScreen::SettingsScreen) { m_SettingsMenu->RefreshActiveSettingsMenuScreen(); }
					g_SettingsMan.UpdateSettingsFile();
				} else if (m_ActiveMenuScreen == MenuScreen::CreditsScreen) {
					m_UpdateResult = MainMenuUpdateResult::BackToMainFromCredits;
				}
				m_ActiveDialogBox = nullptr;
				SetActiveMenuScreen(MenuScreen::MainScreen, false);
				g_GUISound.BackButtonPressSound()->Play();
			} else {
				ShowQuitScreenOrQuit();
			}
		} else if (m_ActiveMenuScreen == MenuScreen::SettingsScreen && m_ActiveDialogBox && g_UInputMan.KeyPressed(KEY_ESC)) {
			m_SettingsMenu->CloseActiveDialogBox();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MainMenuGUI::HandleInputEvents() {
		if (m_ActiveMenuScreen == MenuScreen::MainScreen) {
			int mouseX = 0;
			int mouseY = 0;
			m_ActiveGUIControlManager->GetManager()->GetInputController()->GetMousePosition(&mouseX, &mouseY);
			UpdateMainScreenHoveredButton(dynamic_cast<GUIButton *>(m_MainMenuScreenGUIControlManager->GetControlUnderPoint(mouseX, mouseY, m_MainMenuScreens.at(MenuScreen::MainScreen), 1)));
		}
		m_ActiveGUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_ActiveGUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_MainMenuButtons.at(MenuButton::BackToMainButton)) {
					return true;
				}
				switch (m_ActiveMenuScreen) {
					case MenuScreen::MainScreen:
						HandleMainScreenInputEvents(guiEvent.GetControl());
						break;
					case MenuScreen::MetaGameNoticeScreen:
						HandleMetaGameNoticeScreenInputEvents(guiEvent.GetControl());
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
			} else if (guiEvent.GetType() == GUIEvent::Notification && (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton *>(guiEvent.GetControl()))) { g_GUISound.SelectionChangeSound()->Play(); }
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HandleMainScreenInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::MetaGameButton)) {
			if (!m_MetaGameNoticeShown) {
				SetActiveMenuScreen(MenuScreen::MetaGameNoticeScreen);
			} else {
				m_UpdateResult = MainMenuUpdateResult::MetaGameStarted;
				SetActiveMenuScreen(MenuScreen::MainScreen);
			}
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::ScenarioButton)) {
			m_UpdateResult = MainMenuUpdateResult::ScenarioStarted;
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::MultiplayerButton)) {
			m_UpdateResult = MainMenuUpdateResult::ActivityStarted;
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

	void MainMenuGUI::HandleMetaGameNoticeScreenInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_MainMenuButtons.at(MenuButton::PlayTutorialButton)) {
			m_UpdateResult = MainMenuUpdateResult::ActivityStarted;
			SetActiveMenuScreen(MenuScreen::MainScreen);
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::MetaGameContinueButton)) {
			m_UpdateResult = MainMenuUpdateResult::MetaGameStarted;
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
		} else if (guiEventControl == m_MainMenuButtons.at(MenuButton::GibEditorButton)) {
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

	void MainMenuGUI::UpdateMainScreenHoveredButton(const GUIButton *hoveredButton) {
		int hoveredButtonIndex = -1;
		if (hoveredButton) {
			hoveredButtonIndex = std::distance(m_MainMenuButtons.begin(), std::find(m_MainMenuButtons.begin(), m_MainMenuButtons.begin() + 8, hoveredButton));
			if (hoveredButton != m_MainScreenHoveredButton) { m_MainMenuButtons.at(hoveredButtonIndex)->SetText(m_MainScreenButtonHoveredText.at(hoveredButtonIndex)); }
			m_MainScreenHoveredButton = m_MainMenuButtons.at(hoveredButtonIndex);
		}
		if (!hoveredButton || hoveredButtonIndex != m_MainScreenPrevHoveredButtonIndex) { m_MainMenuButtons.at(m_MainScreenPrevHoveredButtonIndex)->SetText(m_MainScreenButtonUnhoveredText.at(m_MainScreenPrevHoveredButtonIndex)); }

		if (hoveredButtonIndex >= 0) {
			m_MainScreenPrevHoveredButtonIndex = hoveredButtonIndex;
		} else {
			m_MainScreenHoveredButton = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw() {
		// Early return to avoid single frame flicker when title screen goes into transition from the meta notice screen to meta config screen.
		if (m_UpdateResult == MainMenuUpdateResult::MetaGameStarted) {
			return;
		}
		switch (m_ActiveMenuScreen) {
			case MenuScreen::SettingsScreen:
				m_SettingsMenu->Draw();
				break;
			case MenuScreen::ModManagerScreen:
				m_ModManagerMenu->Draw();
				break;
			default:
				m_ActiveGUIControlManager->Draw();
				break;
		}
		if (m_ActiveDialogBox) {
			set_trans_blender(128, 128, 128, 128);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), g_FrameMan.GetOverlayBitmap32(), 0, 0);
			// Whatever this box may be at this point it's already been drawn by the owning GUIControlManager, but we need to draw it again on top of the overlay so it's not affected by it.
			m_ActiveDialogBox->Draw(m_ActiveGUIControlManager->GetScreen());
		}
		m_ActiveGUIControlManager->DrawMouse();
	}
}