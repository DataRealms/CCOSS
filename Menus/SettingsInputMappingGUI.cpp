#include "SettingsInputMappingGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUIScrollbar.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputMappingGUI::SettingsInputMappingGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_InputMappingSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPlayerInputMapping"));
		m_InputMappingSettingsBox->SetVisible(false);

		m_InputMappingSettingsLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPlayerInputMappingTitle"));
		m_CloseMappingBoxButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCloseMappingBox"));
		m_RunConfigWizardButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonRunConfigWizard"));

		m_InputMapScrollingBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxScrollingMappingBox"));
		m_InputMapScrollingBoxScrollbar = dynamic_cast<GUIScrollbar *>(m_GUIControlManager->GetControl("ScrollbarScrollingMappingBox"));
		m_InputMapScrollingBoxScrollbar->SetMaximum(m_InputMapScrollingBox->GetHeight() / 2 + 5);
		m_LastInputMapScrollingBoxScrollbarValue = m_InputMapScrollingBoxScrollbar->GetValue();

		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			m_InputMapLabel.at(i) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelInputName" + std::to_string(i + 1)));
			m_InputMapButton.at(i) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonInputKey" + std::to_string(i + 1)));
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
			m_InputMapScrollingBoxScrollbar->SetValue(0);
			UpdateScrollingInputBoxScrollPosition();
			UpdateMappingLabelsAndButtons();
		} else {
			m_InputMappingSettingsBox->SetVisible(false);
			m_InputMappingSettingsBox->SetEnabled(false);
			m_ConfiguringPlayer = Players::NoPlayer;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::UpdateMappingLabelsAndButtons() {
		const InputMapping *inputMappings = g_UInputMan.GetControlScheme(m_ConfiguringPlayer)->GetInputMappings();

		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			m_InputMapLabel.at(i)->SetText("InputName");
			m_InputMapButton.at(i)->SetText("[InputKey]");
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
		/*
		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			if (m_InputMapLabel.at(i)->GetText() == "InputName") {
				m_InputMapLabel.at(i)->SetVisible(false);
				m_InputMapButton.at(i)->SetVisible(false);
			} else {
				m_InputMapLabel.at(i)->SetVisible(true);
				m_InputMapButton.at(i)->SetVisible(true);
			}
		}
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::UpdateScrollingInputBoxScrollPosition() {
		int scrollbarValue = m_InputMapScrollingBoxScrollbar->GetValue();
		m_InputMapScrollingBox->SetPositionRel(m_InputMapScrollingBox->GetRelXPos(), m_InputMapScrollingBox->GetRelYPos() + (m_LastInputMapScrollingBoxScrollbarValue - scrollbarValue));
		m_LastInputMapScrollingBoxScrollbarValue = scrollbarValue;
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
			for (int mapButton = 0; mapButton < InputElements::INPUT_COUNT; ++mapButton) {
				if (guiEvent.GetControl() == m_InputMapButton.at(mapButton)) {
					m_InputMapButton.at(mapButton)->SetText("Press Any Key");
				}
			}
		} else if (guiEvent.GetType() == GUIEvent::Notification && guiEvent.GetControl() == m_InputMapScrollingBoxScrollbar && guiEvent.GetMsg() == GUIScrollbar::ChangeValue) {
			UpdateScrollingInputBoxScrollPosition();
		}
		m_InputConfigWizardMenu->HandleInputEvents(guiEvent);
	}
}