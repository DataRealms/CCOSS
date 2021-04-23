#include "SettingsGUI.h"

#include "UInputMan.h"
#include "SettingsMan.h"
#include "FrameMan.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"
#include "GUIButton.h"
#include "GUITab.h"
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
		rootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		m_SettingsTabberBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSettingsBase"));

		m_SettingsMenuTabs.at(ActiveSettingsMenu::VideoSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabVideoSettings"));
		m_SettingsMenuTabs.at(ActiveSettingsMenu::AudioSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabAudioSettings"));
		m_SettingsMenuTabs.at(ActiveSettingsMenu::InputSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabInputSettings"));
		m_SettingsMenuTabs.at(ActiveSettingsMenu::GameplaySettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabGameplaySettings"));
		m_SettingsMenuTabs.at(ActiveSettingsMenu::NetworkSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabNetworkSettings"));
		m_SettingsMenuTabs.at(ActiveSettingsMenu::MiscSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabMiscSettings"));

		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("OptionsScreen"))->SetVisible(false);
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfigScreen"))->SetVisible(false);

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));

		m_VideoSettingsMenu = std::make_unique<SettingsVideoGUI>(m_GUIControlManager.get());
		m_AudioSettingsMenu = std::make_unique<SettingsAudioGUI>(m_GUIControlManager.get());
		m_InputSettingsMenu = std::make_unique<SettingsInputGUI>(m_GUIControlManager.get());
		m_GameplaySettingsMenu = std::make_unique<SettingsGameplayGUI>(m_GUIControlManager.get());
		m_NetworkSettingsMenu = std::make_unique<SettingsNetworkGUI>(m_GUIControlManager.get());
		m_MiscSettingsMenu = std::make_unique<SettingsMiscGUI>(m_GUIControlManager.get());

		m_SettingsMenuTabs.at(ActiveSettingsMenu::VideoSettingsMenu)->SetCheck(true);
		SetActiveSettingsMenu(ActiveSettingsMenu::VideoSettingsMenu);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::SetActiveSettingsMenu(ActiveSettingsMenu activeMenu) {
		m_VideoSettingsMenu->SetEnabled(false);
		m_AudioSettingsMenu->SetEnabled(false);
		m_InputSettingsMenu->SetEnabled(false);
		m_GameplaySettingsMenu->SetEnabled(false);
		m_NetworkSettingsMenu->SetEnabled(false);
		m_MiscSettingsMenu->SetEnabled(false);

		switch (activeMenu) {
			case ActiveSettingsMenu::VideoSettingsMenu:
				m_VideoSettingsMenu->SetEnabled(true);
				break;
			case ActiveSettingsMenu::AudioSettingsMenu:
				m_AudioSettingsMenu->SetEnabled(true);
				break;
			case ActiveSettingsMenu::InputSettingsMenu:
				m_InputSettingsMenu->SetEnabled(true);
				break;
			case ActiveSettingsMenu::GameplaySettingsMenu:
				m_GameplaySettingsMenu->SetEnabled(true);
				break;
			case ActiveSettingsMenu::NetworkSettingsMenu:
				m_NetworkSettingsMenu->SetEnabled(true);
				break;
			case ActiveSettingsMenu::MiscSettingsMenu:
				m_MiscSettingsMenu->SetEnabled(true);
				break;
			default:
				RTEAbort("Invalid settings menu passed to SettingsGUI::SetActiveSettingsMenu!");
		}
		m_ActiveSettingsMenu = activeMenu;
		m_SettingsTabberBox->SetFocus();
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
			} else if (guiEvent.GetType() == GUIEvent::Notification && guiEvent.GetMsg() == GUITab::Pushed) {
				if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::VideoSettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::VideoSettingsMenu);
				} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::AudioSettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::AudioSettingsMenu);
				} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::InputSettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::InputSettingsMenu);
				} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::GameplaySettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::GameplaySettingsMenu);
				} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::NetworkSettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::NetworkSettingsMenu);
				} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(ActiveSettingsMenu::MiscSettingsMenu)) {
					SetActiveSettingsMenu(ActiveSettingsMenu::MiscSettingsMenu);
				}
			}

			switch (m_ActiveSettingsMenu) {
				case ActiveSettingsMenu::VideoSettingsMenu:
					m_VideoSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::AudioSettingsMenu:
					m_AudioSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::InputSettingsMenu:
					m_InputSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::GameplaySettingsMenu:
					m_GameplaySettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::NetworkSettingsMenu:
					m_NetworkSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case ActiveSettingsMenu::MiscSettingsMenu:
					m_MiscSettingsMenu->HandleInputEvents(guiEvent);
					break;
				default:
					break;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::Draw() {
		m_GUIControlManager->Draw();
		m_GUIControlManager->DrawMouse();
	}
}