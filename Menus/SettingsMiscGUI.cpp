#include "SettingsMiscGUI.h"
#include "SettingsMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsMiscGUI::SettingsMiscGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_MiscSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxMiscSettings"));

		m_ShowToolTipsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowToolTips"));
		m_ShowToolTipsCheckbox->SetCheck(g_SettingsMan.ShowToolTips());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMiscGUI::SetEnabled(bool enable) const {
		if (enable) {
			m_MiscSettingsBox->SetVisible(true);
			m_MiscSettingsBox->SetEnabled(true);
		} else {
			m_MiscSettingsBox->SetVisible(false);
			m_MiscSettingsBox->SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMiscGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			;
		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			;
		}
	}
}