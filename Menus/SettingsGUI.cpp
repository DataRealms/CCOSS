#include "SettingsGUI.h"

#include "UInputMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"
#include "GUIButton.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsGUI::SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller) {
		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		if (!m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSkin.ini")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSkin.ini"); }
		m_GUIControlManager->Load("Base.rte/GUIs/SettingsGUI.ini");

		m_Controller = controller;

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->SetPositionAbs((g_FrameMan.GetResX() - rootBox->GetWidth()) / 2, 0);

		//dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("OptionsScreen"))->SetVisible(false);
		//dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfigScreen"))->SetVisible(false);

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));

		m_VideoSettingsMenu = std::make_unique<SettingsVideoGUI>(m_GUIControlManager.get());
		m_AudioSettingsMenu = std::make_unique<SettingsAudioGUI>(m_GUIControlManager.get());
		m_InputSettingsMenu = std::make_unique<SettingsInputGUI>(m_GUIControlManager.get());
		m_GameplaySettingsMenu = std::make_unique<SettingsGameplayGUI>(m_GUIControlManager.get());

		m_ActiveSettingsMenu = ActiveSettingsMenu::GameplaySettingsActive;

		//m_ShowToolTipsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("ShowToolTipsCheckbox"));
		//m_ShowToolTipsCheckbox->SetCheck(g_SettingsMan.ToolTips());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					return true;
				}
			}

			switch (m_ActiveSettingsMenu) {
				case ActiveSettingsMenu::VideoSettingsActive:
					m_VideoSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::AudioSettingsActive:
					m_AudioSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::GameplaySettingsActive:
					m_GameplaySettingsMenu->HandleInputEvents(guiEvent);
					break;
				default:
					break;
			}
		}


		/*
			// Return to main menu button pressed
			if (guiEvent.GetControl() == m_MainMenuButtons.at(BACKTOMAIN)) {
				// Hide all screens, the appropriate screen will reappear on next update
				HideAllScreens();
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);

				// If leaving the options screen, save the settings!
				if (m_MenuScreen == OPTIONSSCREEN) {
					g_SettingsMan.SetShowToolTips(m_OptionsCheckbox.at(SHOWTOOLTIPS)->GetCheck());

					g_SettingsMan.UpdateSettingsFile();
				}

				m_MenuScreen = MAINSCREEN;
				m_ScreenChange = true;

				g_GUISound.BackButtonPressSound()->Play();
			}

			// Return to options menu button pressed
			if (guiEvent.GetControl() == m_BackToOptionsButton) {
				// Hide all screens, the appropriate screen will reappear on next update
				HideAllScreens();
				BackToOptionsButton->SetVisible(false);
				m_MenuScreen = OPTIONSSCREEN;
				m_ScreenChange = true;

				g_GUISound.BackButtonPressSound()->Play();
			}
		*/


		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::Draw() {
		m_GUIControlManager->Draw();
		m_GUIControlManager->DrawMouse();
	}
}