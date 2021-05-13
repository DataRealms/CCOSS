#include "SettingsInputMappingGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputMappingGUI::SettingsInputMappingGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_InputMappingSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPlayerInputMapping"));
		m_InputMappingSettingsBox->SetVisible(false);

		m_InputMappingSettingsLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPlayerInputMappingTitle"));
		m_CloseMappingBoxButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCloseMappingBox"));
		m_RunConfigWizardButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonRunConfigWizard"));

		for (int i = 0; i < 18; ++i) {
			m_InputMapLabel.at(i) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelInputMapping" + std::to_string(i + 1)));
			m_InputMapButton.at(i) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonInputMapping" + std::to_string(i + 1)));
		}
		m_InputConfigWizardMenu = std::make_unique<SettingsInputMappingWizardGUI>(parentControlManager);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::SetEnabled(bool enable, int player) {
		if (enable) {
			m_InputMappingSettingsBox->SetVisible(true);
			m_InputMappingSettingsBox->SetEnabled(true);
			m_InputMappingSettingsLabel->SetText("P L A Y E R   " + std::to_string(player + 1) + "   I N P U T   M A P P I N G");
			m_ConfiguringPlayer = static_cast<Players>(player);
			//UpdateMappingLabelsAndButtons(player);
		} else {
			m_InputMappingSettingsBox->SetVisible(false);
			m_InputMappingSettingsBox->SetEnabled(false);
			m_ConfiguringPlayer = Players::NoPlayer;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::UpdateMappingLabelsAndButtons(int player) {
		//const InputMapping *inputMappings = g_UInputMan.GetControlScheme(player)->GetInputMappings();

		for (int i = 0; i < PlayerInputMappings::InputMappingCount; ++i) {
			m_InputMapLabel.at(i)->SetText("- Map Name");
			m_InputMapButton.at(i)->SetText("[Button]");
		}

		switch (g_UInputMan.GetControlScheme(player)->GetDevice()) {
			case InputDevice::DEVICE_KEYB_ONLY:
				m_InputMapLabel.at(0)->SetText("- Move/Aim Up");
				m_InputMapLabel.at(1)->SetText("- Move/Aim Down");
				m_InputMapLabel.at(2)->SetText("- Move Left");
				m_InputMapLabel.at(3)->SetText("- Move Right");
				m_InputMapLabel.at(4)->SetText("- Fire/Activate");
				m_InputMapLabel.at(5)->SetText("- Sharp Aim");
				m_InputMapLabel.at(6)->SetText("- Command Menu");
				m_InputMapLabel.at(7)->SetText("- Jump");
				m_InputMapLabel.at(8)->SetText("- Crouch");
				m_InputMapLabel.at(9)->SetText("- Prev. Body");
				m_InputMapLabel.at(10)->SetText("- Next Body");
				m_InputMapLabel.at(11)->SetText("- Reload");
				m_InputMapLabel.at(12)->SetText("- Pick Up");
				m_InputMapLabel.at(13)->SetText("- Drop Device");
				m_InputMapLabel.at(14)->SetText("- Prev. Device");
				m_InputMapLabel.at(15)->SetText("- Next. Device");
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				m_InputMapLabel.at(0)->SetText("- Move Up/Jump");
				m_InputMapLabel.at(1)->SetText("- Move Down/Crouch");
				m_InputMapLabel.at(2)->SetText("- Move Left");
				m_InputMapLabel.at(3)->SetText("- Move Right");
				m_InputMapLabel.at(4)->SetText("- Reload");
				m_InputMapLabel.at(5)->SetText("- Pick Up");
				m_InputMapLabel.at(6)->SetText("- Drop Device");
				m_InputMapLabel.at(7)->SetText("- Prev. Device");
				m_InputMapLabel.at(8)->SetText("- Next. Device");
				m_InputMapLabel.at(9)->SetText("- Prev. Body");
				m_InputMapLabel.at(10)->SetText("- Next Body");
				break;
			default:
				/*
				// D-Pad
				m_InputMapLabel.at(0)->SetText("- Move/Aim Up");
				m_InputMapLabel.at(1)->SetText("- Move/Aim Down");
				m_InputMapLabel.at(2)->SetText("- Move Left");
				m_InputMapLabel.at(3)->SetText("- Move Right");
				m_InputMapLabel.at(4)->SetText("- Sharp Aim");
				m_InputMapLabel.at(5)->SetText("- Fire/Activate");
				m_InputMapLabel.at(6)->SetText("- Jump");
				m_InputMapLabel.at(7)->SetText("- Command Menu");
				m_InputMapLabel.at(8)->SetText("- Next Body");
				m_InputMapLabel.at(9)->SetText("- Prev. Body");
				m_InputMapLabel.at(10)->SetText("- Start");
				m_InputMapLabel.at(11)->SetText("- Back");
				*/
				/*
				// Dual Analog
				m_InputMapLabel.at(0)->SetText("- Move Up/Jump");
				m_InputMapLabel.at(1)->SetText("- Move Down/Crouch");
				m_InputMapLabel.at(2)->SetText("- Move Left");
				m_InputMapLabel.at(3)->SetText("- Move Right");
				m_InputMapLabel.at(4)->SetText("- Aim Up");
				m_InputMapLabel.at(5)->SetText("- Aim Down");
				m_InputMapLabel.at(6)->SetText("- Aim Left");
				m_InputMapLabel.at(7)->SetText("- Aim Right");
				m_InputMapLabel.at(8)->SetText("- Fire/Activate");
				m_InputMapLabel.at(9)->SetText("- Command Menu");
				m_InputMapLabel.at(10)->SetText("- Next Body");
				m_InputMapLabel.at(11)->SetText("- Prev. Body");
				m_InputMapLabel.at(12)->SetText("- Prev. Device");
				m_InputMapLabel.at(13)->SetText("- Next. Device");
				m_InputMapLabel.at(14)->SetText("- Pick Up");
				m_InputMapLabel.at(15)->SetText("- Reload");
				m_InputMapLabel.at(16)->SetText("- Start");
				m_InputMapLabel.at(17)->SetText("- Back");
				*/
				break;
		}
		for (int i = 0; i < PlayerInputMappings::InputMappingCount; ++i) {
			if (m_InputMapLabel.at(i)->GetText() == "- Map Name") {
				m_InputMapLabel.at(i)->SetVisible(false);
				m_InputMapButton.at(i)->SetVisible(false);
			} else {
				m_InputMapLabel.at(i)->SetVisible(true);
				m_InputMapButton.at(i)->SetVisible(true);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_CloseMappingBoxButton) {
				g_GUISound.ButtonPressSound()->Play();
				SetEnabled(false);
			} else if (guiEvent.GetControl() == m_RunConfigWizardButton) {
				m_InputConfigWizardMenu->SetEnabled(true, m_ConfiguringPlayer);
			}
			for (int mapButton = 0; mapButton < PlayerInputMappings::InputMappingCount; ++mapButton) {
				if (guiEvent.GetControl() == m_InputMapButton.at(mapButton)) {
					;
				}
			}
		}
		m_InputConfigWizardMenu->HandleInputEvents(guiEvent);
	}
}