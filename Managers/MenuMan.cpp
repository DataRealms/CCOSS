#include "MenuMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"

#include "Controller.h"
#include "TitleScreen.h"
#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "LoadingScreen.h"

#include "NetworkServer.h"

extern bool g_ResumeActivity;
extern bool g_ResetActivity;
extern bool g_InActivity;

extern volatile bool g_Quit;

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Initialize(bool initLoadingScreen) {
		m_GUIScreen = std::make_unique<AllegroScreen>(g_FrameMan.GetBackBuffer32());
		m_GUIInput = std::make_unique<AllegroInput>(-1);

		if (initLoadingScreen) {
			g_LoadingScreen.Create(m_GUIScreen.get(), m_GUIInput.get());
			g_PresetMan.LoadAllDataModules();
			g_LoadingScreen.Destroy();

			// Load the different input device icons. This can't be done during UInputMan::Create() because the icon presets don't exist so we need to do this after modules are loaded.
			g_UInputMan.LoadDeviceIcons();
		}

		m_MenuController = std::make_unique<Controller>(Controller::CIM_PLAYER, 0);
		m_MenuController->SetTeam(0);

		m_MainMenu = std::make_unique<MainMenuGUI>(m_GUIScreen.get(), m_GUIInput.get(), m_MenuController.get());

		m_ScenarioMenu = std::make_unique<ScenarioGUI>();

		//g_MetaMan.GetGUI()->Create(m_MenuController.get());

		m_TitleScreen = std::make_unique<TitleScreen>(m_MainMenu->GetGUIControlManager()->GetSkin()->GetFont("fatfont.png"));
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
		// Change the screen to the options menu otherwise we're at the main screen after reinitializing.
		m_MainMenu->SetActiveMenuScreen(MainMenuGUI::MenuScreen::SettingsScreen);

		g_FrameMan.DestroyTempBackBuffers();
		g_FrameMan.SetResolutionChanged(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMainMenu() {
		m_MainMenu->Update();

		if (!m_MainMenu->IsEnabled()) {
			m_MainMenu->SetEnabled(true);
			m_ScenarioMenu->SetEnabled();
		}

		if (m_TitleScreen->GetActiveMenu() == TitleScreen::ActiveMenu::MainMenuActive) {
			g_InActivity = false;
		}
		if (m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::ScenarioMenuActive && m_MainMenu->ScenarioStarted()) {
			m_ScenarioMenu->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToPlanet);
		} else if (m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::CampaignMenuActive && m_MainMenu->CampaignStarted()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCampaign);
		} else if (m_MainMenu->ActivityResumed()) {
			g_ResumeActivity = true;
		} else if (m_MainMenu->ActivityRestarted()) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeScrollOut);
			g_ResetActivity = true;
		//} else if (g_NetworkServer.IsServerModeEnabled()) {
		//	m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeScrollOut);
		//	EnterMultiplayerLobby();
		}
		return m_MainMenu->QuitProgram();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::UpdateScenarioMenu() {
		ScenarioGUI::ScenarioUpdateResult updateResult = m_ScenarioMenu->Update();

		//if (!m_ScenarioMenu->IsEnabled()) {
			//m_ScenarioMenu->SetEnabled(true);
			//m_MainMenu->SetEnabled(false);
		//}

		if (m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::MainMenuActive && updateResult == ScenarioGUI::ScenarioUpdateResult::BackToMain) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (updateResult == ScenarioGUI::ScenarioUpdateResult::ActivityResumed) {
			g_ResumeActivity = true;
		} else if (updateResult == ScenarioGUI::ScenarioUpdateResult::ActivityRestarted) {
			m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::FadeScrollOut);
			g_ResetActivity = true;
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

	void MenuMan::Update() {
		if (g_FrameMan.ResolutionChanged()) { Reinitialize(); }

		bool keyPressed = g_UInputMan.AnyStartPress();

		m_ActiveScreen = m_TitleScreen->Update(keyPressed);
		bool quitResult = false;

		m_MenuController->Update();
		m_GUIInput->Update();

		int mouseX = 0;
		int mouseY = 0;
		m_GUIInput->GetMousePosition(&mouseX, &mouseY);
		m_MousePos.SetXY(static_cast<float>(mouseX), static_cast<float>(mouseY));

		switch (m_ActiveScreen) {
			case TitleScreen::ActiveMenu::MainMenuActive:
				quitResult = UpdateMainMenu();
				break;
			case TitleScreen::ActiveMenu::ScenarioMenuActive:
				UpdateScenarioMenu();
				break;
			case TitleScreen::ActiveMenu::CampaignMenuActive:
				break;
			default:
				break;
		}

		g_Quit = g_Quit || quitResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Draw() {
		g_FrameMan.ClearBackBuffer32();

		m_TitleScreen->Draw();

		switch (m_ActiveScreen) {
			case TitleScreen::ActiveMenu::MainMenuActive:
				m_MainMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			case TitleScreen::ActiveMenu::ScenarioMenuActive:
				m_ScenarioMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			case TitleScreen::ActiveMenu::CampaignMenuActive:
				break;
			default:
				break;
		}

		int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();

		// Draw the active joystick's sprite next to the mouse.
		if (device >= InputDevice::DEVICE_GAMEPAD_1) {
			BITMAP *deviceIcon = g_UInputMan.GetDeviceIcon(device)->GetBitmaps32()[0];
			if (deviceIcon) { draw_sprite(g_FrameMan.GetBackBuffer32(), deviceIcon, m_MousePos.GetFloorIntX() + 16, m_MousePos.GetFloorIntY() - 4); }
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


	/*
			// Metagame menu update and drawing
			if (m_IntroSequenceState == CAMPAIGNPLAY) {
				g_MetaMan.GetGUI()->SetPlanetInfo(m_PlanetPos, planetRadius);
				g_MetaMan.Update();
				g_MetaMan.Draw(g_FrameMan.GetBackBuffer32());
			}
	*/

}