#include "MenuMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "PresetMan.h"
#include "ConsoleMan.h"

#include "Controller.h"
#include "TitleScreen.h"
#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "LoadingGUI.h"

extern bool g_ResumeActivity;
extern bool g_ResetActivity;

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Initialize(bool initLoadingScreen) {
		m_MenuController = std::make_unique<Controller>(Controller::CIM_PLAYER, 0);
		m_MenuController->SetTeam(0);

		m_MainMenu = std::make_unique<MainMenuGUI>();
		m_MainMenu->Create(m_MenuController.get());

		m_ScenarioMenu = std::make_unique<ScenarioGUI>();
		m_ScenarioMenu->Create(m_MenuController.get());

		m_TitleScreen = std::make_unique<TitleScreen>(m_MainMenu->GetGUIControlManager()->GetSkin()->GetFont("fatfont.png"));

		if (initLoadingScreen) {
			m_LoadingScreen = std::make_unique<LoadingGUI>();
			g_PresetMan.LoadAllDataModules();
			m_LoadingScreen->Destroy();
			m_LoadingScreen.reset();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Reinitialize() {
		m_MainMenu.reset();
		m_ScenarioMenu.reset();

		//g_pMainMenuGUI->Destroy();
		//g_pMainMenuController->Reset();
		//g_pScenarioGUI->Destroy();
		//g_MetaMan.GetGUI()->Destroy();

		g_ConsoleMan.Destroy();
		g_ConsoleMan.Initialize();

		//g_FrameMan.LoadPalette("Base.rte/palette.bmp");

		m_MainMenu->Create(m_MenuController.get());
		m_ScenarioMenu->Create(m_MenuController.get());

		//g_MetaMan.GetGUI()->Create(m_MenuController.get());

		g_FrameMan.DestroyTempBackBuffers();
		//g_HadResolutionChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Update() {
		bool keyPressed = g_UInputMan.AnyStartPress();

		// Reset the key press states
		g_UInputMan.Update();
		g_TimerMan.Update();
		g_TimerMan.UpdateSim();
		g_ConsoleMan.Update();
		g_AudioMan.Update();

		m_ActiveScreen = m_TitleScreen->Update(keyPressed);

		switch (m_ActiveScreen) {
			case TitleScreen::TitleTransition::MainMenu:
				m_MainMenu->SetEnabled(true);
				m_MainMenu->Update();

				if (m_MainMenu->ScenarioStarted()) {
					m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToScenario);
				} else if (m_MainMenu->CampaignStarted()) {
					m_TitleScreen->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToCampaign);
				} else 	if (m_MainMenu->ActivityResumed()) {
					g_ResumeActivity = true;
				}
				break;
			case TitleScreen::TitleTransition::ScenarioMenu:

				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MenuMan::Draw() {
		g_FrameMan.ClearBackBuffer32();

		m_TitleScreen->Draw();

		switch (m_ActiveScreen) {
			case TitleScreen::TitleTransition::MainMenu:
				m_MainMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			case TitleScreen::TitleTransition::ScenarioMenu:
				m_ScenarioMenu->Draw(g_FrameMan.GetBackBuffer32());
				break;
			default:
				break;
		}

		g_ConsoleMan.Draw(g_FrameMan.GetBackBuffer32());

		//vsync();
		g_FrameMan.FlipFrameBuffers();
	}
}