#include "MenuMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"
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

#include "NetworkServer.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Initialize(bool firstTimeInit) {
		m_ActiveMenu = ActiveMenu::MenusDisabled;

		m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
		m_GUIInput = std::make_unique<AllegroInput>(-1, true);

		if (firstTimeInit) {
			g_LoadingScreen.Create(m_GUIScreen.get(), m_GUIInput.get());
			g_PresetMan.LoadAllDataModules();
			g_LoadingScreen.Destroy();

			// Load the different input device icons. This can't be done during UInputMan::Create() because the icon presets don't exist so we need to do this after modules are loaded.
			g_UInputMan.LoadDeviceIcons();

			if (g_NetworkServer.IsServerModeEnabled()) { g_SettingsMan.SetSkipIntro(true); }
		}
		m_TitleScreen = std::make_unique<TitleScreen>(m_GUIScreen.get());
		m_MainMenu = std::make_unique<MainMenuGUI>(m_GUIScreen.get(), m_GUIInput.get());
		m_ScenarioMenu = std::make_unique<ScenarioGUI>(m_GUIScreen.get(), m_GUIInput.get());

		// TODO: MetaGameGUI doesn't seem to actually do anything with the Controller but removing conflicts with the second Create() method so that needs to be sorted out sometime in the year 3000.
		m_MenuController = std::make_unique<Controller>(Controller::CIM_PLAYER);
		g_MetaMan.GetGUI()->Create(m_MenuController.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Reinitialize() {
		g_ConsoleMan.Destroy();
		g_MetaMan.GetGUI()->Destroy();

		m_ScenarioMenu.reset();
		m_MainMenu.reset();
		m_TitleScreen.reset();
		m_MenuController.reset();

		Initialize(false);
		g_ConsoleMan.Initialize();

		g_FrameMan.DestroyTempBackBuffers();
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
			case TitleScreen::TitleTransition::CampaignMenu:
				newActiveMenu = ActiveMenu::CampaignMenuActive;
				break;
			default:
				break;
		}

		if (newActiveMenu != m_ActiveMenu) {
			m_ActiveMenu = newActiveMenu;
			switch (m_ActiveMenu) {
				case ActiveMenu::ScenarioMenuActive:
					m_ScenarioMenu->SetEnabled();
					m_ScenarioMenu->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
					break;
				case ActiveMenu::CampaignMenuActive:
					g_MetaMan.GetGUI()->SetEnabled();
					g_MetaMan.GetGUI()->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
					break;
				default:
					break;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMainMenu() const {
		MainMenuGUI::MainMenuUpdateResult updateResult = m_MainMenu->Update();

		if (updateResult == MainMenuGUI::MainMenuUpdateResult::ScenarioStarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToScenario);
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::CampaignStarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCampaign);
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::EnterCreditsScreen) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCredits);
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::BackToMainFromCredits) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::CreditsToMainMenu);
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::ActivityResumed) {
			g_ActivityMan.SetResumeActivity();
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::ActivityStarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::ScrollingFadeOut);
			g_ActivityMan.SetRestartActivity();
		}
		return updateResult == MainMenuGUI::MainMenuUpdateResult::Quit;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::UpdateScenarioMenu() const {
		ScenarioGUI::ScenarioMenuUpdateResult updateResult = m_ScenarioMenu->Update();

		if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::BackToMain) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityResumed) {
			g_ActivityMan.SetResumeActivity();
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityRestarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeOut);
			g_ActivityMan.SetRestartActivity();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateCampaignMenu() const {
		g_MetaMan.GetGUI()->SetStationInfo(m_TitleScreen->GetStationPos());
		g_MetaMan.Update();

		if (g_MetaMan.GetGUI()->BackToMain()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (g_MetaMan.GetGUI()->ActivityRestarted()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeOut);
			g_ActivityMan.SetRestartActivity();
		} else if (g_MetaMan.GetGUI()->ActivityResumed()) {
			g_ActivityMan.SetResumeActivity();
		}

		return g_MetaMan.GetGUI()->QuitProgram();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::Update() {
		if (g_FrameMan.ResolutionChanged()) { Reinitialize(); }

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
			case ActiveMenu::CampaignMenuActive:
				quitResult = UpdateCampaignMenu();
				break;
			default:
				break;
		}
		g_ConsoleMan.Update();

		System::SetQuit(quitResult);

		if (m_TitleScreen->GetTitleTransitionState() == TitleScreen::TitleTransition::TransitionEnd) {
			m_TitleScreen->SetTitlePendingTransition();
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Draw() const {
		g_FrameMan.ClearBackBuffer32();

		if (g_FrameMan.ResolutionChanged()) {
			return;
		}

		m_TitleScreen->Draw();

		switch (m_ActiveMenu) {
			case ActiveMenu::MainMenuActive:
				m_MainMenu->Draw();
				break;
			case ActiveMenu::ScenarioMenuActive:
				m_ScenarioMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			case ActiveMenu::CampaignMenuActive:
				g_MetaMan.Draw(g_FrameMan.GetBackBuffer32());
				break;
			default:
				break;
		}

		int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();

		// Draw the active joystick's sprite next to the mouse.
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			int mouseX = 0;
			int mouseY = 0;
			m_GUIInput->GetMousePosition(&mouseX, &mouseY);
			BITMAP *deviceIcon = g_UInputMan.GetDeviceIcon(device)->GetBitmaps32()[0];
			if (deviceIcon) { draw_sprite(g_FrameMan.GetBackBuffer32(), deviceIcon, mouseX + 16, mouseY - 4); }
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

		g_ConsoleMan.Draw(g_FrameMan.GetBackBuffer32());
	}
}