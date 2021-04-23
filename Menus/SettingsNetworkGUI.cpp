#include "SettingsNetworkGUI.h"

#include "GUI.h"
#include "GUICollectionBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsNetworkGUI::SettingsNetworkGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_NetworkSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxNetworkSettings"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsNetworkGUI::SetEnabled(bool enable) const {
		if (enable) {
			m_NetworkSettingsBox->SetVisible(true);
			m_NetworkSettingsBox->SetEnabled(true);
		} else {
			m_NetworkSettingsBox->SetVisible(false);
			m_NetworkSettingsBox->SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsNetworkGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			;
		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			;
		}
	}
}