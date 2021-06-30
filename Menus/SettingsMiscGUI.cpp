#include "SettingsMiscGUI.h"
#include "SettingsMan.h"
#include "PerformanceMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsMiscGUI::SettingsMiscGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_MiscSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxMiscSettings"));

		m_SkipIntroCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxSkipIntro"));
		m_SkipIntroCheckbox->SetCheck(g_SettingsMan.SkipIntro());

		m_ShowToolTipsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowToolTips"));
		m_ShowToolTipsCheckbox->SetCheck(g_SettingsMan.ShowToolTips());

		m_DisableLoadingScreenReportCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxDisableLoadingScreenReport"));
		m_DisableLoadingScreenReportCheckbox->SetCheck(!g_SettingsMan.GetLoadingScreenProgressReportDisabled());

		m_ShowAdvancedPerfStatsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowAdvancedPerfStats"));
		m_ShowAdvancedPerfStatsCheckbox->SetCheck(g_PerformanceMan.AdvancedPerformanceStatsEnabled());

		m_MeasureLoadTimeCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMeasureLoadingTime"));
		m_MeasureLoadTimeCheckbox->SetCheck(g_SettingsMan.IsMeasuringModuleLoadTime());
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
		if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_SkipIntroCheckbox) {
				g_SettingsMan.SetSkipIntro(m_SkipIntroCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowToolTipsCheckbox) {
				g_SettingsMan.SetShowToolTips(m_ShowToolTipsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_DisableLoadingScreenReportCheckbox) {
				g_SettingsMan.SetLoadingScreenProgressReportDisabled(!m_DisableLoadingScreenReportCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowAdvancedPerfStatsCheckbox) {
				g_PerformanceMan.ShowAdvancedPerformanceStats(m_ShowAdvancedPerfStatsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_MeasureLoadTimeCheckbox) {
				g_SettingsMan.MeasureModuleLoadTime(m_MeasureLoadTimeCheckbox->GetCheck());
			}
		}
	}
}