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
		m_InputMapScrollingBoxScrollbar->SetMaximum(m_InputMapScrollingBox->GetHeight());
		m_InputMapScrollingBoxScrollbar->SetPageSize(m_InputMapScrollingBoxScrollbar->GetMaximum() / 2 - 5);
		m_LastInputMapScrollingBoxScrollbarValue = m_InputMapScrollingBoxScrollbar->GetValue();

		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			m_InputMapLabel.at(i) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelInputName" + std::to_string(i + 1)));
			m_InputMapLabel.at(i)->SetText(c_InputElementNames.at(i));
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
		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			std::string inputDescription = g_UInputMan.GetControlScheme(m_ConfiguringPlayer)->GetInputMappings()[i].GetPresetDescription();
			m_InputMapButton.at(i)->SetText(!inputDescription.empty() ? "[" + inputDescription + "]" : "[Undefined]");
		}
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