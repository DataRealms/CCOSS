#include "SettingsGUI.h"
#include "FrameMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUITab.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsGUI::SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/SettingsGUI.ini");

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

		m_SettingsTabberBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSettingsBase"));
		m_SettingsTabberBox->SetPositionAbs((rootBox->GetWidth() - m_SettingsTabberBox->GetWidth()) / 2, 140);
		if (rootBox->GetHeight() < 540) { m_SettingsTabberBox->CenterInParent(true, true); }

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));
		m_BackToMainButton->SetPositionAbs((rootBox->GetWidth() - m_BackToMainButton->GetWidth()) / 2, m_SettingsTabberBox->GetYPos() + m_SettingsTabberBox->GetHeight() + 10);

		m_SettingsMenuTabs.at(SettingsMenuScreen::VideoSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabVideoSettings"));
		m_SettingsMenuTabs.at(SettingsMenuScreen::AudioSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabAudioSettings"));
		m_SettingsMenuTabs.at(SettingsMenuScreen::InputSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabInputSettings"));
		m_SettingsMenuTabs.at(SettingsMenuScreen::GameplaySettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabGameplaySettings"));
		m_SettingsMenuTabs.at(SettingsMenuScreen::MiscSettingsMenu) = dynamic_cast<GUITab *>(m_GUIControlManager->GetControl("TabMiscSettings"));

		m_VideoSettingsMenu = std::make_unique<SettingsVideoGUI>(m_GUIControlManager.get());
		m_AudioSettingsMenu = std::make_unique<SettingsAudioGUI>(m_GUIControlManager.get());
		m_InputSettingsMenu = std::make_unique<SettingsInputGUI>(m_GUIControlManager.get());
		m_GameplaySettingsMenu = std::make_unique<SettingsGameplayGUI>(m_GUIControlManager.get());
		m_MiscSettingsMenu = std::make_unique<SettingsMiscGUI>(m_GUIControlManager.get());

		SetActiveSettingsMenuScreen(SettingsMenuScreen::VideoSettingsMenu, false);
		m_SettingsMenuTabs.at(m_ActiveSettingsMenuScreen)->SetCheck(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUICollectionBox * SettingsGUI::GetActiveDialogBox() const {
		GUICollectionBox *activeDialogBox = nullptr;
		switch (m_ActiveSettingsMenuScreen) {
			case SettingsMenuScreen::VideoSettingsMenu:
				activeDialogBox = m_VideoSettingsMenu->GetActiveDialogBox();
				break;
			case SettingsMenuScreen::InputSettingsMenu:
				activeDialogBox = m_InputSettingsMenu->GetActiveDialogBox();
				break;
			default:
				break;
		}
		DisableSettingsMenuNavigation(activeDialogBox);

		return activeDialogBox;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::CloseActiveDialogBox() const {
		switch (m_ActiveSettingsMenuScreen) {
			case SettingsMenuScreen::VideoSettingsMenu:
				m_VideoSettingsMenu->CloseActiveDialogBox();
				g_GUISound.BackButtonPressSound()->Play();
				break;
			case SettingsMenuScreen::InputSettingsMenu:
				m_InputSettingsMenu->CloseActiveDialogBox();
				g_GUISound.BackButtonPressSound()->Play();
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::DisableSettingsMenuNavigation(bool disable) const {
		m_BackToMainButton->SetEnabled(!disable);
		for (GUITab *settingsTabberTab : m_SettingsMenuTabs) {
			settingsTabberTab->SetEnabled(!disable);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::SetActiveSettingsMenuScreen(SettingsMenuScreen activeMenu, bool playButtonPressSound) {
		m_VideoSettingsMenu->SetEnabled(false);
		m_AudioSettingsMenu->SetEnabled(false);
		m_InputSettingsMenu->SetEnabled(false);
		m_GameplaySettingsMenu->SetEnabled(false);
		m_MiscSettingsMenu->SetEnabled(false);

		switch (activeMenu) {
			case SettingsMenuScreen::VideoSettingsMenu:
				m_VideoSettingsMenu->SetEnabled(true);
				break;
			case SettingsMenuScreen::AudioSettingsMenu:
				m_AudioSettingsMenu->SetEnabled(true);
				break;
			case SettingsMenuScreen::InputSettingsMenu:
				m_InputSettingsMenu->SetEnabled(true);
				break;
			case SettingsMenuScreen::GameplaySettingsMenu:
				m_GameplaySettingsMenu->SetEnabled(true);
				break;
			case SettingsMenuScreen::MiscSettingsMenu:
				m_MiscSettingsMenu->SetEnabled(true);
				break;
			default:
				RTEAbort("Invalid settings menu passed to SettingsGUI::SetActiveSettingsMenuScreen!");
				break;
		}
		m_ActiveSettingsMenuScreen = activeMenu;
		// Remove focus so the tab hovered graphic is removed after being pressed, otherwise it remains stuck on the active tab.
		m_GUIControlManager->GetManager()->SetFocus(nullptr);

		if (playButtonPressSound) { g_GUISound.BackButtonPressSound()->Play(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					RefreshActiveSettingsMenuScreen();
					return true;
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if ((guiEvent.GetMsg() == GUIButton::Focused) && dynamic_cast<GUIButton *>(guiEvent.GetControl()) || (guiEvent.GetMsg() == GUITab::Hovered && dynamic_cast<GUITab *>(guiEvent.GetControl()))) { g_GUISound.SelectionChangeSound()->Play(); }

				if (guiEvent.GetMsg() == GUITab::UnPushed) {
					if (guiEvent.GetControl() == m_SettingsMenuTabs.at(SettingsMenuScreen::VideoSettingsMenu)) {
						SetActiveSettingsMenuScreen(SettingsMenuScreen::VideoSettingsMenu);
					} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(SettingsMenuScreen::AudioSettingsMenu)) {
						SetActiveSettingsMenuScreen(SettingsMenuScreen::AudioSettingsMenu);
					} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(SettingsMenuScreen::InputSettingsMenu)) {
						SetActiveSettingsMenuScreen(SettingsMenuScreen::InputSettingsMenu);
					} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(SettingsMenuScreen::GameplaySettingsMenu)) {
						SetActiveSettingsMenuScreen(SettingsMenuScreen::GameplaySettingsMenu);
					} else if (guiEvent.GetControl() == m_SettingsMenuTabs.at(SettingsMenuScreen::MiscSettingsMenu)) {
						SetActiveSettingsMenuScreen(SettingsMenuScreen::MiscSettingsMenu);
					}
				}
			}
			switch (m_ActiveSettingsMenuScreen) {
				case SettingsMenuScreen::VideoSettingsMenu:
					m_VideoSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case SettingsMenuScreen::AudioSettingsMenu:
					m_AudioSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case SettingsMenuScreen::InputSettingsMenu:
					m_InputSettingsMenu->HandleInputEvents(guiEvent);
					break;
				case SettingsMenuScreen::GameplaySettingsMenu:
					m_GameplaySettingsMenu->HandleInputEvents(guiEvent);
					break;
				case SettingsMenuScreen::MiscSettingsMenu:
					m_MiscSettingsMenu->HandleInputEvents(guiEvent);
					break;
				default:
					RTEAbort("Trying to handle input events for an invalid settings menu in SettingsGUI::HandleInputEvents!");
					break;
			}
		}
		// Manual input config sequence has to be updated outside the GUI event handling loop otherwise input capture doesn't work (loop only runs if event queue isn't empty).
		if (m_ActiveSettingsMenuScreen == SettingsMenuScreen::InputSettingsMenu) {
			if (m_InputSettingsMenu->InputMappingConfigIsConfiguringManually()) {
				m_InputSettingsMenu->HandleMappingConfigManualConfiguration();
			} else if (m_InputSettingsMenu->InputConfigWizardIsConfiguringManually()) {
				m_InputSettingsMenu->HandleConfigWizardManualConfiguration();
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::Draw() const {
		m_GUIControlManager->Draw();
		m_GUIControlManager->DrawMouse();
	}
}