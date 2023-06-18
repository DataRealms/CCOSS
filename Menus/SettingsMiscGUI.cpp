#include "SettingsMiscGUI.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "PerformanceMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUICheckbox.h"
#include "GUILabel.h"
#include "GUISlider.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsMiscGUI::SettingsMiscGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_MiscSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxMiscSettings"));

		m_SkipIntroCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxSkipIntro"));
		m_SkipIntroCheckbox->SetCheck(g_SettingsMan.SkipIntro());

		m_ShowToolTipsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowToolTips"));
		m_ShowToolTipsCheckbox->SetCheck(g_SettingsMan.ShowToolTips());

		m_ShowLoadingScreenProgressReportCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowLoadingScreenProgressReport"));
		m_ShowLoadingScreenProgressReportCheckbox->SetCheck(!g_SettingsMan.GetLoadingScreenProgressReportDisabled());

		m_ShowAdvancedPerfStatsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxShowAdvancedPerfStats"));
		m_ShowAdvancedPerfStatsCheckbox->SetCheck(g_PerformanceMan.AdvancedPerformanceStatsEnabled());

		m_MeasureLoadTimeCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxMeasureLoadingTime"));
		m_MeasureLoadTimeCheckbox->SetCheck(g_SettingsMan.IsMeasuringModuleLoadTime());

		m_UseMonospaceConsoleFontCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxUseMonospaceConsoleFont"));
		m_UseMonospaceConsoleFontCheckbox->SetCheck(g_ConsoleMan.GetConsoleUseMonospaceFont());

		m_DisableFactionBuyMenuThemesCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxDisableFactionBuyMenuThemes"));
		m_DisableFactionBuyMenuThemesCheckbox->SetCheck(g_SettingsMan.FactionBuyMenuThemesDisabled());

		m_DisableFactionBuyMenuThemeCursorsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxDisableFactionBuyMenuThemeCursors"));
		m_DisableFactionBuyMenuThemeCursorsCheckbox->SetCheck(g_SettingsMan.FactionBuyMenuThemeCursorsDisabled());

		m_SceneBackgroundAutoScaleLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelSceneBackgroundAutoScaleSetting"));
		UpdateSceneBackgroundAutoScaleLabel();

		m_SceneBackgroundAutoScaleSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderSceneBackgroundAutoScale"));
		m_SceneBackgroundAutoScaleSlider->SetValue(g_SettingsMan.GetSceneBackgroundAutoScaleMode());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMiscGUI::SetEnabled(bool enable) const {
		m_MiscSettingsBox->SetVisible(enable);
		m_MiscSettingsBox->SetEnabled(enable);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMiscGUI::UpdateSceneBackgroundAutoScaleLabel() {
		switch (g_SettingsMan.GetSceneBackgroundAutoScaleMode()) {
			case 1:
				m_SceneBackgroundAutoScaleLabel->SetText("Stretch to fit screen");
				break;
			case 2:
				m_SceneBackgroundAutoScaleLabel->SetText("Always upscaled");
				break;
			default:
				m_SceneBackgroundAutoScaleLabel->SetText("Disabled");
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMiscGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_SkipIntroCheckbox) {
				g_SettingsMan.SetSkipIntro(m_SkipIntroCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowToolTipsCheckbox) {
				g_SettingsMan.SetShowToolTips(m_ShowToolTipsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowLoadingScreenProgressReportCheckbox) {
				g_SettingsMan.SetLoadingScreenProgressReportDisabled(!m_ShowLoadingScreenProgressReportCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_ShowAdvancedPerfStatsCheckbox) {
				g_PerformanceMan.ShowAdvancedPerformanceStats(m_ShowAdvancedPerfStatsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_MeasureLoadTimeCheckbox) {
				g_SettingsMan.MeasureModuleLoadTime(m_MeasureLoadTimeCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_UseMonospaceConsoleFontCheckbox) {
				g_ConsoleMan.SetConsoleUseMonospaceFont(m_UseMonospaceConsoleFontCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_DisableFactionBuyMenuThemesCheckbox) {
				g_SettingsMan.SetFactionBuyMenuThemesDisabled(m_DisableFactionBuyMenuThemesCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_DisableFactionBuyMenuThemeCursorsCheckbox) {
				g_SettingsMan.SetFactionBuyMenuThemeCursorsDisabled(m_DisableFactionBuyMenuThemeCursorsCheckbox->GetCheck());
			} else if (guiEvent.GetControl() == m_SceneBackgroundAutoScaleSlider) {
				g_SettingsMan.SetSceneBackgroundAutoScaleMode(m_SceneBackgroundAutoScaleSlider->GetValue());
				UpdateSceneBackgroundAutoScaleLabel();
			}
		}
	}
}