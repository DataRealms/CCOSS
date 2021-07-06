#include "MenuMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"
#include "MetaMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"
#include "AllegroInput.h"

#include "Controller.h"
#include "TitleScreen.h"
#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "MetagameGUI.h"
#include "LoadingScreen.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Initialize(bool firstTimeInit) {
		m_ActiveMenu = ActiveMenu::MenusDisabled;

		m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
		m_GUIInput = std::make_unique<AllegroInput>(-1, g_UInputMan.GetJoystickCount() > 0);

		if (firstTimeInit) { g_LoadingScreen.Create(m_GUIScreen.get(), m_GUIInput.get(), g_SettingsMan.GetLoadingScreenProgressReportDisabled()); }

		m_TitleScreen = std::make_unique<TitleScreen>(m_GUIScreen.get());
		m_MainMenu = std::make_unique<MainMenuGUI>(m_GUIScreen.get(), m_GUIInput.get());
		m_ScenarioMenu = std::make_unique<ScenarioGUI>(m_GUIScreen.get(), m_GUIInput.get());

		// TODO: MetaGameGUI doesn't seem to actually do anything with the Controller but removing conflicts with the second Create() method so that needs to be sorted out sometime in the year 3000.
		m_MenuController = std::make_unique<Controller>(Controller::CIM_PLAYER);
		g_MetaMan.GetGUI()->Create(m_MenuController.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Reinitialize() {
		g_MetaMan.GetGUI()->Destroy();

		m_ScenarioMenu.reset();
		m_MainMenu.reset();
		m_TitleScreen.reset();
		m_MenuController.reset();

		Initialize(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::SetActiveMenu() {
		ActiveMenu newActiveMenu = ActiveMenu::MenusDisabled;

		switch (m_TitleScreen->GetTitleTransitionState()) {
			case TitleScreen::TitleTransition::MainMenu:
			case TitleScreen::TitleTransition::MainMenuToCredits:
				newActiveMenu = ActiveMenu::MainMenuActive;
				break;
			case TitleScreen::TitleTransition::ScenarioMenu:
				newActiveMenu = ActiveMenu::ScenarioMenuActive;
				break;
			case TitleScreen::TitleTransition::MetaGameMenu:
				newActiveMenu = ActiveMenu::MetaGameMenuActive;
				break;
			default:
				break;
		}

		if (newActiveMenu != m_ActiveMenu) {
			m_ActiveMenu = newActiveMenu;
			switch (m_ActiveMenu) {
				case ActiveMenu::ScenarioMenuActive:
					m_ScenarioMenu->SetEnabled(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
					break;
				case ActiveMenu::MetaGameMenuActive:
					g_MetaMan.GetGUI()->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
					g_MetaMan.GetGUI()->SetEnabled();
					break;
				default:
					break;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::Update() {
		m_TitleScreen->Update();
		SetActiveMenu();

		bool quitResult = false;

		switch (m_ActiveMenu) {
			case ActiveMenu::MainMenuActive:
				quitResult = UpdateMainMenu();
				break;
			case ActiveMenu::ScenarioMenuActive:
				UpdateScenarioMenu();
				break;
			case ActiveMenu::MetaGameMenuActive:
				quitResult = UpdateMetaGameMenu();
				break;
			default:
				break;
		}
		if (quitResult) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::ScrollingFadeOutQuit);
		} else if (m_TitleScreen->GetTitleTransitionState() != TitleScreen::TitleTransition::ScrollingFadeOutQuit && g_UInputMan.DetectJoystickHotPlug()) {
			m_GUIInput->SetKeyJoyMouseCursor(g_UInputMan.GetJoystickCount() > 0);
		}
		if (m_TitleScreen->GetTitleTransitionState() == TitleScreen::TitleTransition::TransitionEndQuit) {
			System::SetQuit();
		} else if (m_TitleScreen->GetTitleTransitionState() == TitleScreen::TitleTransition::TransitionEnd) {
			m_TitleScreen->SetTitlePendingTransition();
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMainMenu() const {
		switch (m_MainMenu->Update()) {
			case MainMenuGUI::MainMenuUpdateResult::MetaGameStarted:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToMetaGame);
				break;
			case MainMenuGUI::MainMenuUpdateResult::ScenarioStarted:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToScenario);
				break;
			case MainMenuGUI::MainMenuUpdateResult::EnterCreditsScreen:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCredits);
				break;
			case MainMenuGUI::MainMenuUpdateResult::BackToMainFromCredits:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::CreditsToMainMenu);
				break;
			case MainMenuGUI::MainMenuUpdateResult::ActivityStarted:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::ScrollingFadeOut);
				g_ActivityMan.SetRestartActivity();
				break;
			case MainMenuGUI::MainMenuUpdateResult::ActivityResumed:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::TransitionEnd);
				g_ActivityMan.SetResumeActivity();
				break;
			case MainMenuGUI::MainMenuUpdateResult::Quit:
				return true;
			default:
				break;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::UpdateScenarioMenu() const {
		switch (m_ScenarioMenu->Update()) {
			case ScenarioGUI::ScenarioMenuUpdateResult::BackToMain:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
				break;
			case ScenarioGUI::ScenarioMenuUpdateResult::ActivityResumed:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::TransitionEnd);
				g_ActivityMan.SetResumeActivity();
				break;
			case ScenarioGUI::ScenarioMenuUpdateResult::ActivityStarted:
				m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeOut);
				if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }
				g_ActivityMan.SetRestartActivity();
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMetaGameMenu() const {
		g_MetaMan.GetGUI()->SetStationOrbitPos(m_TitleScreen->GetStationPos());
		g_MetaMan.Update();

		if (g_MetaMan.GetGUI()->BackToMain()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (g_MetaMan.GetGUI()->ActivityRestarted()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeOut);
			g_ActivityMan.SetRestartActivity();
		} else if (g_MetaMan.GetGUI()->ActivityResumed()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::TransitionEnd);
			g_ActivityMan.SetResumeActivity();
		}

		return g_MetaMan.GetGUI()->QuitProgram();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Draw() const {
		g_FrameMan.ClearBackBuffer32();

		// Early return when changing resolution so screen remains black while everything is being recreated instead of being stuck showing a badly aligned title screen.
		if (g_FrameMan.ResolutionChanged()) {
			return;
		}

		m_TitleScreen->Draw();

		switch (m_ActiveMenu) {
			case ActiveMenu::MainMenuActive:
				m_MainMenu->Draw();
				break;
			case ActiveMenu::ScenarioMenuActive:
				m_ScenarioMenu->Draw();
				break;
			case ActiveMenu::MetaGameMenuActive:
				g_MetaMan.Draw(g_FrameMan.GetBackBuffer32());
				break;
			default:
				break;
		}
		if (m_ActiveMenu != ActiveMenu::MenusDisabled && g_UInputMan.GetJoystickCount() > 0) {
			int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();

			// Draw the active joystick's sprite next to the mouse.
			if (device >= InputDevice::DEVICE_GAMEPAD_1) {
				int mouseX = 0;
				int mouseY = 0;
				m_GUIInput->GetMousePosition(&mouseX, &mouseY);
				BITMAP *deviceIcon = g_UInputMan.GetDeviceIcon(device)->GetBitmaps32()[0];
				if (deviceIcon) { draw_sprite(g_FrameMan.GetBackBuffer32(), deviceIcon, mouseX + (deviceIcon->w / 2), mouseY - (deviceIcon->h / 5)); }
			}
			// Show which joysticks are detected by the game.
			for (int playerIndex = Players::PlayerOne; playerIndex < Players::MaxPlayerCount; playerIndex++) {
				if (g_UInputMan.JoystickActive(playerIndex)) {
					int matchedDevice = InputDevice::DEVICE_GAMEPAD_1 + playerIndex;
					if (matchedDevice != device) {
						BITMAP *deviceIcon = g_UInputMan.GetDeviceIcon(matchedDevice)->GetBitmaps32()[0];
						if (deviceIcon) { draw_sprite(g_FrameMan.GetBackBuffer32(), deviceIcon, g_FrameMan.GetResX() - 30 * g_UInputMan.GetJoystickCount() + 30 * playerIndex, g_FrameMan.GetResY() - 25); }
					}
				}
			}
		}
	}
}