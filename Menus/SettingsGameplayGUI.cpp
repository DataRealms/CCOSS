#include "SettingsGameplayGUI.h"
#include "SettingsMan.h"
#include "MovableMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"
#include "GUITextBox.h"

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

		m_MaxUnheldItemsTextbox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxMaxUnheldItems"));
		m_MaxUnheldItemsTextbox->SetText(std::to_string(g_MovableMan.GetMaxDroppedItems()));
		m_MaxUnheldItemsTextbox->SetNumericOnly(true);
		m_MaxUnheldItemsTextbox->SetMaxTextLength(2);

		m_CrabBombThresholdTextbox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCrabBombThreshold"));
		m_CrabBombThresholdTextbox->SetText(std::to_string(g_SettingsMan.GetCrabBombThreshold()));
		m_CrabBombThresholdTextbox->SetNumericOnly(true);
		m_CrabBombThresholdTextbox->SetMaxTextLength(3);
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
			} else if (guiEvent.GetControl() == m_MaxUnheldItemsTextbox && guiEvent.GetMsg() == GUITextBox::Enter) {
				UpdateMaxUnheldItemsTextbox();
			} else if (guiEvent.GetControl() == m_CrabBombThresholdTextbox && guiEvent.GetMsg() == GUITextBox::Enter) {
				UpdateCrabBombThresholdTextbox();
			// Update both textboxes when clicking the main CollectionBox, otherwise clicking off focused textboxes does not remove their focus or update the setting values and they will still capture keyboard input.
			} else if (guiEvent.GetControl() == m_GameplaySettingsBox && guiEvent.GetMsg() == GUICollectionBox::Clicked && !m_GameplaySettingsBox->HasFocus()) {
				UpdateMaxUnheldItemsTextbox();
				UpdateCrabBombThresholdTextbox();
			}
		}
	}
}