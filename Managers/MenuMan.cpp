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
#include "MultiplayerServerLobby.h"

extern bool g_ResumeActivity;
extern volatile bool g_Quit;

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

		m_LaunchIntoEditor = false;
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

	TitleScreen::TitleTransition MenuMan::GetTitleTransitionState() const {
		return m_TitleScreen->GetTitleTransitionState();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::SetTitleTransitionStateTarget(TitleScreen::TitleTransition targetState) const {
		m_TitleScreen->SetTitleTransitionStateTarget(targetState);
		//m_TitleScreen->SetTitleTransitionState(targetState);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::SetTitlePendingTransition() const {
		m_TitleScreen->SetTitlePendingTransition();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::EnterEditorActivity() {
		bool validEditorName = false;
		std::array<std::string_view, 5> validEditorNames = { "ActorEditor", "GibEditor", "SceneEditor", "AreaEditor", "AssemblyEditor" };
		if (std::find(validEditorNames.begin(), validEditorNames.end(), m_EditorToLaunch) != validEditorNames.end()) { validEditorName = true; }

		if (validEditorName) {
			// Force mouse + keyboard with default mapping so we won't need to change manually if player 1 is set to keyboard only or gamepad.
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetPreset(InputPreset::PRESET_WASDKEYS);
			m_MainMenu->StartEditorActivity(m_EditorToLaunch);
			return true;
		} else {
			g_ConsoleMan.PrintString("ERROR: Invalid editor name passed into \"-editor\" argument!");
			g_ConsoleMan.SetEnabled(true);
			m_LaunchIntoEditor = false;
			return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::EnterMultiplayerLobby() {
		g_SceneMan.SetSceneToLoad("Multiplayer Scene");
		MultiplayerServerLobby *multiplayerServerLobby = new MultiplayerServerLobby;
		multiplayerServerLobby->Create();

		multiplayerServerLobby->ClearPlayers(true);
		multiplayerServerLobby->AddPlayer(0, true, 0, 0);
		multiplayerServerLobby->AddPlayer(1, true, 0, 1);
		multiplayerServerLobby->AddPlayer(2, true, 0, 2);
		multiplayerServerLobby->AddPlayer(3, true, 0, 3);

		//g_FrameMan.ResetSplitScreens(true, true);
		g_ActivityMan.SetStartActivity(multiplayerServerLobby);
		g_ActivityMan.SetResetActivity(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool MenuMan::UpdateMainMenu() {
		if (!m_MainMenu->IsEnabled()) { m_MainMenu->SetEnabled(true); }

		m_MainMenu->Update();

		if (m_TitleScreen->GetActiveMenu() == TitleScreen::ActiveMenu::MainMenuActive) {
			g_ActivityMan.SetInActivity(false);
		}

		if (m_MainMenu->ScenarioStarted() && m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::ScenarioMenuActive) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::MainMenuToScenario);
			m_ScenarioMenu->SetEnabled();
		} else if (m_MainMenu->CampaignStarted() && m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::CampaignMenuActive) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::MainMenuToCampaign);
			g_MetaMan.GetGUI()->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());
		} else if (m_MainMenu->ActivityResumed()) {
			g_ResumeActivity = true;
		} else if (m_MainMenu->ActivityRestarted()) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::ScrollFadeOut);
			g_ActivityMan.SetResetActivity(true);
		//} else if (m_MainMenu->GetActiveMenuScreen() == MainMenuGUI::MenuScreen::CreditsScreen) {
		//	m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCredits);
		//} else if (m_MainMenu->ReturnToMainScreen()) {
		//	m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::CreditsToMainMenu);
		} else if (g_NetworkServer.IsServerModeEnabled()) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::ScrollFadeOut);
			EnterMultiplayerLobby();
		}
		return m_MainMenu->QuitProgram();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::UpdateScenarioMenu() {
		m_ScenarioMenu->SetPlanetInfo(m_TitleScreen->GetPlanetPos(), m_TitleScreen->GetPlanetRadius());

		ScenarioGUI::ScenarioMenuUpdateResult updateResult = m_ScenarioMenu->Update();

		if (m_TitleScreen->GetActiveMenu() != TitleScreen::ActiveMenu::MainMenuActive && updateResult == ScenarioGUI::ScenarioMenuUpdateResult::BackToMain) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::PlanetToMainMenu);
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityResumed) {
			g_ResumeActivity = true;
		} else if (updateResult == ScenarioGUI::ScenarioMenuUpdateResult::ActivityRestarted) {
			m_TitleScreen->SetTitleTransitionStateTarget(TitleScreen::TitleTransition::ScrollFadeOut);
			g_ActivityMan.SetResetActivity(true);
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

		m_ActiveScreen = m_TitleScreen->Update();
		bool quitResult = false;

		//m_MenuController->Update();
		//m_GUIInput->Update();

		switch (m_ActiveScreen) {
			case TitleScreen::ActiveMenu::MainMenuActive:
				quitResult = UpdateMainMenu();
				break;
			case TitleScreen::ActiveMenu::ScenarioMenuActive:
				UpdateScenarioMenu();
				break;
			case TitleScreen::ActiveMenu::CampaignMenuActive:
				g_MetaMan.Update();
				break;
			default:
				break;
		}
		g_ConsoleMan.Update();

		g_Quit = g_Quit || quitResult;

		return m_TitleScreen->GetTitleTransitionState() == TitleScreen::TitleTransition::End;
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
				g_MetaMan.Draw(g_FrameMan.GetBackBuffer32());
				break;
			default:
				break;
		}
		g_ConsoleMan.Draw(g_FrameMan.GetBackBuffer32());

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
		//vsync();
		g_FrameMan.FlipFrameBuffers();
	}
}