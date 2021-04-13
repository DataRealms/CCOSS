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

	void MenuMan::Initialize(bool initLoadingScreen) {
		m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
		m_GUIInput = std::make_unique<AllegroInput>(-1);
		m_MenuController = std::make_unique<Controller>(Controller::CIM_PLAYER);

		if (initLoadingScreen) {
			g_LoadingScreen.Create(m_GUIScreen.get(), m_GUIInput.get());
			g_PresetMan.LoadAllDataModules();
			g_LoadingScreen.Destroy();

			// Load the different input device icons. This can't be done during UInputMan::Create() because the icon presets don't exist so we need to do this after modules are loaded.
			g_UInputMan.LoadDeviceIcons();

			if (g_NetworkServer.IsServerModeEnabled()) { g_SettingsMan.SetSkipIntro(true); }
		}
		m_MainMenu = std::make_unique<MainMenuGUI>(m_GUIScreen.get(), m_GUIInput.get(), m_MenuController.get());
		m_TitleScreen = std::make_unique<TitleScreen>(m_MainMenu->GetGUIControlManager()->GetSkin()->GetFont("fatfont.png"));

		m_ScenarioMenu = std::make_unique<ScenarioGUI>(m_GUIScreen.get(), m_GUIInput.get());
		g_MetaMan.GetGUI()->Create(m_MenuController.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Reinitialize() {
		m_MenuController.reset();
		m_MainMenu.reset();
		m_ScenarioMenu.reset();
		m_TitleScreen.reset();

		g_ConsoleMan.Destroy();
		g_ConsoleMan.Initialize();

		Initialize(false);

		g_FrameMan.DestroyTempBackBuffers();
		g_FrameMan.SetResolutionChanged(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMainMenu() {
		if (!m_MainMenu->IsEnabled()) { m_MainMenu->SetEnabled(true); }

		MainMenuGUI::MainMenuUpdateResult updateResult = m_MainMenu->Update();

		//if (m_TitleScreen->GetActiveMenu() == TitleScreen::ActiveMenu::MainMenuActive) { g_ActivityMan.SetInActivity(false); }

		if (updateResult == MainMenuGUI::MainMenuUpdateResult::ScenarioStarted && m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::ScenarioMenuActive) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToScenario);
			m_ScenarioMenu->SetEnabled();
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::CampaignStarted && m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::CampaignMenuActive) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCampaign);
			g_MetaMan.GetGUI()->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
		} else if (m_MainMenu->ActivityResumed()) {
			g_ActivityMan.SetResumeActivity();
		} else if (m_MainMenu->ActivityRestarted()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::ScrollFadeOut);
			g_ActivityMan.SetRestartActivity();
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::EnterCreditsScreen) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCredits);
		} else if (updateResult == MainMenuGUI::MainMenuUpdateResult::BackToMainFromCredits) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::CreditsToMainMenu);
		}
		return updateResult == MainMenuGUI::MainMenuUpdateResult::Quit;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::UpdateScenarioMenu() {
		m_ScenarioMenu->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());

		ScenarioGUI::ScenarioMenuUpdateResult updateResult = m_ScenarioMenu->Update();

		if (m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::MainMenuActive && updateResult == ScenarioGUI::ScenarioMenuUpdateResult::BackToMain) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityResumed) {
			g_ActivityMan.SetResumeActivity();
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityRestarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeOut);
			g_ActivityMan.SetRestartActivity();
		}

		/*
		// In server mode once we exited to main or scenario menu we need to start Lobby activity
		if (g_NetworkServer.IsServerModeEnabled()) {
			EnterMultiplayerLobby();
			g_IntroState = FADEOUT;
			m_SectionSwitch = true;
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateCampaignMenu() {
		g_MetaMan.GetGUI()->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
		g_MetaMan.GetGUI()->SetStationPos(m_TitleScreen->GetStationPos());
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

		m_ActiveScreen = m_TitleScreen->Update();
		bool quitResult = false;

		switch (m_ActiveScreen) {
			case TitleScreen::ActiveMenu::MainMenuActive:
				quitResult = UpdateMainMenu();
				break;
			case TitleScreen::ActiveMenu::ScenarioMenuActive:
				UpdateScenarioMenu();
				break;
			case TitleScreen::ActiveMenu::CampaignMenuActive:
				quitResult = UpdateCampaignMenu();
				break;
			default:
				break;
		}
		g_ConsoleMan.Update();

		System::SetQuit(quitResult);

		return m_TitleScreen->GetTitleTransitionState() == TitleScreen::TitleTransition::End;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Draw() {
		g_FrameMan.ClearBackBuffer32();

		m_TitleScreen->Draw();

		switch (m_ActiveScreen) {
			case TitleScreen::ActiveMenu::MainMenuActive:
				m_MainMenu->Draw();
				break;
			case TitleScreen::ActiveMenu::ScenarioMenuActive:
				m_ScenarioMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			case TitleScreen::ActiveMenu::CampaignMenuActive:
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

		//vsync();
		g_FrameMan.FlipFrameBuffers();
	}
}