#include "SettingsGameplayGUI.h"
#include "SettingsMan.h"
#include "MovableMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"
#include "GUITextBox.h"
#include "GUISlider.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsGameplayGUI::SettingsGameplayGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_GameplaySettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxGameplaySettings"));

		m_FlashOnBrainDamageCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxFlashOnBrainDamage"));
		m_FlashOnBrainDamageCheckbox->SetCheck(g_SettingsMan.FlashOnBrainDamage());

		m_BlipOnRevealUnseenCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxBlipOnRevealUnseen"));
		m_BlipOnRevealUnseenCheckbox->SetCheck(g_SettingsMan.BlipOnRevealUnseen());

		m_ShowForeignItemsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowForeignItems"));
		m_ShowForeignItemsCheckbox->SetCheck(g_SettingsMan.ShowForeignItems());

		m_EnableCrabBombsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxCrabBombs"));
		m_EnableCrabBombsCheckbox->SetCheck(g_SettingsMan.CrabBombsEnabled());

		m_EndlessMetaGameCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxEndlessMetaGame"));
		m_EndlessMetaGameCheckbox->SetCheck(g_SettingsMan.EndlessMetaGameMode());

		m_ShowEnemyHUDCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxEnemyHUD"));
		m_ShowEnemyHUDCheckbox->SetCheck(g_SettingsMan.ShowEnemyHUD());

		m_EnableSmartBuyMenuNavigationCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxSmartBuyMenuNavigation"));
		m_EnableSmartBuyMenuNavigationCheckbox->SetCheck(g_SettingsMan.SmartBuyMenuNavigationEnabled());

		m_MaxUnheldItemsTextbox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxMaxUnheldItems"));
		m_MaxUnheldItemsTextbox->SetText(std::to_string(g_MovableMan.GetMaxDroppedItems()));
		m_MaxUnheldItemsTextbox->SetNumericOnly(true);
		m_MaxUnheldItemsTextbox->SetMaxTextLength(2);

		m_CrabBombThresholdTextbox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCrabBombThreshold"));
		m_CrabBombThresholdTextbox->SetText(std::to_string(g_SettingsMan.GetCrabBombThreshold()));
		m_CrabBombThresholdTextbox->SetNumericOnly(true);
		m_CrabBombThresholdTextbox->SetMaxTextLength(3);

		m_UnheldItemsHUDDisplayRangeSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderUnheldItemsHUDRange"));
		int unheldItemsHUDDisplayRangeValue = static_cast<int>(g_SettingsMan.GetUnheldItemsHUDDisplayRange());
		if (unheldItemsHUDDisplayRangeValue <= -1) {
			m_UnheldItemsHUDDisplayRangeSlider->SetValue(m_UnheldItemsHUDDisplayRangeSlider->GetMinimum());
		} else if (unheldItemsHUDDisplayRangeValue == 0) {
			m_UnheldItemsHUDDisplayRangeSlider->SetValue(m_UnheldItemsHUDDisplayRangeSlider->GetMaximum());
		} else {
			m_UnheldItemsHUDDisplayRangeSlider->SetValue(static_cast<int>(g_SettingsMan.GetUnheldItemsHUDDisplayRange() / c_PPM));
		}
		m_UnheldItemsHUDDisplayRangeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelUnheldItemsHUDRangeValue"));
		UpdateUnheldItemsHUDDisplayRange();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGameplayGUI::SetEnabled(bool enable) {
		m_GameplaySettingsBox->SetVisible(enable);
		m_GameplaySettingsBox->SetEnabled(enable);

		if (enable) {
			UpdateMaxUnheldItemsTextbox();
			UpdateCrabBombThresholdTextbox();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGameplayGUI::UpdateMaxUnheldItemsTextbox() {
		if (m_MaxUnheldItemsTextbox->GetText().empty()) { m_MaxUnheldItemsTextbox->SetText(std::to_string(g_MovableMan.GetMaxDroppedItems())); }
		g_MovableMan.SetMaxDroppedItems(std::stoi(m_MaxUnheldItemsTextbox->GetText()));
		m_GameplaySettingsBox->SetFocus();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGameplayGUI::UpdateCrabBombThresholdTextbox() {
		if (m_CrabBombThresholdTextbox->GetText().empty()) { m_CrabBombThresholdTextbox->SetText(std::to_string(g_SettingsMan.GetCrabBombThreshold())); }
		if (std::stoi(m_CrabBombThresholdTextbox->GetText()) == 0) { m_CrabBombThresholdTextbox->SetText("1"); }
		g_SettingsMan.SetCrabBombThreshold(std::stoi(m_CrabBombThresholdTextbox->GetText()));
		m_GameplaySettingsBox->SetFocus();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGameplayGUI::UpdateUnheldItemsHUDDisplayRange() {
		int newValue = m_UnheldItemsHUDDisplayRangeSlider->GetValue();
		if (newValue < 3) {
			m_UnheldItemsHUDDisplayRangeLabel->SetText("Hidden");
			g_SettingsMan.SetUnheldItemsHUDDisplayRange(0);
		} else if (newValue > 50) {
			m_UnheldItemsHUDDisplayRangeLabel->SetText("Unlimited");
			g_SettingsMan.SetUnheldItemsHUDDisplayRange(-1.0F);
		} else {
			m_UnheldItemsHUDDisplayRangeLabel->SetText("Up to " + std::to_string(newValue) + " meters");
			g_SettingsMan.SetUnheldItemsHUDDisplayRange(static_cast<float>(newValue) * c_PPM);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGameplayGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_FlashOnBrainDamageCheckbox) {
				g_SettingsMan.SetFlashOnBrainDamage(m_FlashOnBrainDamageCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_BlipOnRevealUnseenCheckbox) {
				g_SettingsMan.SetBlipOnRevealUnseen(m_BlipOnRevealUnseenCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowForeignItemsCheckbox) {
				g_SettingsMan.SetShowForeignItems(m_ShowForeignItemsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_EnableCrabBombsCheckbox) {
				g_SettingsMan.SetCrabBombsEnabled(m_EnableCrabBombsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_EndlessMetaGameCheckbox) {
				g_SettingsMan.SetEndlessMetaGameMode(m_EndlessMetaGameCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowEnemyHUDCheckbox) {
				g_SettingsMan.SetShowEnemyHUD(m_ShowEnemyHUDCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_EnableSmartBuyMenuNavigationCheckbox) {
				g_SettingsMan.SetSmartBuyMenuNavigation(m_EnableSmartBuyMenuNavigationCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_MaxUnheldItemsTextbox && guiEvent.GetMsg() == GUITextBox::Enter) {
				UpdateMaxUnheldItemsTextbox();
			} else if (guiEvent.GetControl() == m_CrabBombThresholdTextbox && guiEvent.GetMsg() == GUITextBox::Enter) {
				UpdateCrabBombThresholdTextbox();
			} else if (guiEvent.GetControl() == m_UnheldItemsHUDDisplayRangeSlider) {
				UpdateUnheldItemsHUDDisplayRange();
			// Update both textboxes when clicking the main CollectionBox, otherwise clicking off focused textboxes does not remove their focus or update the setting values and they will still capture keyboard input.
			} else if (guiEvent.GetControl() == m_GameplaySettingsBox && guiEvent.GetMsg() == GUICollectionBox::Clicked && !m_GameplaySettingsBox->HasFocus()) {
				UpdateMaxUnheldItemsTextbox();
				UpdateCrabBombThresholdTextbox();
			}
		}
	}
}