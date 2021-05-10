#include "SettingsVideoGUI.h"
#include "FrameMan.h"
#include "ActivityMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIRadioButton.h"
#include "GUICheckbox.h"
#include "GUIComboBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsVideoGUI::SettingsVideoGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_NewGraphicsDriver = g_FrameMan.GetGraphicsDriver();
		m_NewResX = g_FrameMan.GetResX();
		m_NewResY = g_FrameMan.GetResY();
		m_NewResUpscaled = (g_FrameMan.GetResMultiplier() > 1) ? true : false;

		m_VideoSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxVideoSettings"));

		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Windowed) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickWindowed"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Borderless) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickBorderless"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledBorderless) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledBorderless"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Dedicated) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickDedicated"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledDedicated) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledDedicated"));

		m_PresetResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioPresetResolution"));
		m_CustomResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioCustomResolution"));

		m_PresetResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionPresetResolution"));
		m_PresetResolutionComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboPresetResolution"));
		m_PresetResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyPresetResolution"));
		m_PresetResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPresetResolutonValidation"));
		m_PresetResolutionMessageLabel->SetVisible(false);

		m_CustomResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionCustomResolution"));
		m_CustomResolutionBox->SetVisible(false);

		m_ResolutionChangeDialogBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ResolutionChangeDialog"));
		m_ResolutionChangeDialogBox->CenterInParent(true, true);
		m_ResolutionChangeDialogBox->SetVisible(false);

		m_ConfirmResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChange"));
		m_CancelResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelResolutionChange"));

		PopulateResolutionsComboBox();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::SetEnabled(bool enable) const {
		if (enable) {
			m_VideoSettingsBox->SetVisible(true);
			m_VideoSettingsBox->SetEnabled(true);
		} else {
			m_VideoSettingsBox->SetVisible(false);
			m_VideoSettingsBox->SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsVideoGUI::IsSupportedResolution(int width, int height) const {
		if ((width >= 640 && height >= 450) && (width <= g_FrameMan.GetMaxResX() && height <= g_FrameMan.GetMaxResY())) {
			// Disallow wacky resolutions that are taller than wide and some other dumb ones
			if ((height > width) || (width == 1152 && height == 864) || (width == 1176 && height == 664)) {
				return false;
			}
			// Disallow resolution width that isn't in multiples of 4 otherwise Allegro fails to initialize graphics, but only in windowed/borderless mode
			if ((g_FrameMan.GetGraphicsDriver() != GFX_AUTODETECT_FULLSCREEN && g_FrameMan.GetGraphicsDriver() != GFX_DIRECTX_ACCEL) && width % 4 != 0) {
				return false;
			}
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::PopulateResolutionsComboBox() {
		m_PresetResolutions.clear();

		// Get a list of modes from the fullscreen driver even though we're not using it. This is so we don't need to populate the list manually and has all the reasonable resolutions (along with some stupid ones but whatever)
#ifdef _WIN32
		GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
#elif __unix__
		GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_XWINDOWS_FULLSCREEN);
#endif

		if (!resList) {
			m_PresetResolutionComboBox->SetVisible(false);
			m_PresetResolutionApplyButton->SetVisible(false);

			m_PresetResolutionMessageLabel->SetText("Failed to get the preset resolution list from the graphics driver!\nPlease use the custom resolution controls instead.");
			m_PresetResolutionMessageLabel->SetVisible(true);
			m_PresetResolutionMessageLabel->CenterInParent(true, true);
			return;
		}

		std::set<PresetResolutionRecord> resRecords;
		for (int i = 0; i < resList->num_modes; ++i) {
			// Only list 32bpp modes
			if (resList->mode[i].bpp == 32) {
				int width = resList->mode[i].width;
				int height = resList->mode[i].height;
				if (IsSupportedResolution(width, height)) { resRecords.emplace(width, height, false); }
			}
		}
		// Manually add qHD (960x540) to the list because it's rarely present in drivers
		resRecords.emplace(960, 540, false);

		std::set<PresetResolutionRecord> upscaledResRecords;
		for (const PresetResolutionRecord &resRecord : resRecords) {
			PresetResolutionRecord upscaledResRecord(resRecord.Width * 2, resRecord.Height * 2, true);
			if (IsSupportedResolution(upscaledResRecord.Width, upscaledResRecord.Height)) { upscaledResRecords.emplace(upscaledResRecord); }
		}
		resRecords.merge(upscaledResRecords);
		m_PresetResolutions.assign(resRecords.begin(), resRecords.end());

		for (int i = 0; i < m_PresetResolutions.size(); ++i) {
			const PresetResolutionRecord &resRecord = m_PresetResolutions.at(i);
			m_PresetResolutionComboBox->AddItem(resRecord.MakeResolutionString());
			if ((resRecord.Width == g_FrameMan.GetResX() * g_FrameMan.GetResMultiplier()) && (resRecord.Height == g_FrameMan.GetResY() * g_FrameMan.GetResMultiplier()) && (resRecord.Upscaled == g_FrameMan.GetResMultiplier() > 1)) { m_PresetResolutionComboBox->SetSelectedIndex(i); }
		}
		destroy_gfx_mode_list(resList);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyNewResolution() {
		g_GUISound.ButtonPressSound()->Play();

		if (g_ActivityMan.GetActivity()) {
			m_ResolutionChangeDialogBox->SetVisible(true);
		} else {
			g_FrameMan.ChangeResolution(m_NewResX, m_NewResY, m_NewResUpscaled, m_NewGraphicsDriver);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyQuickChangeResolution(ResolutionQuickChangeType resolutionChangeType) {
#ifndef __unix__
		bool linuxCompatible = false;
#else
		bool linuxCompatible = true;
#endif

		switch (resolutionChangeType) {
			case ResolutionQuickChangeType::Windowed:
				m_NewGraphicsDriver = GFX_AUTODETECT_WINDOWED;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			case ResolutionQuickChangeType::Borderless:
			case ResolutionQuickChangeType::Dedicated:
				if (!linuxCompatible) {
					m_NewGraphicsDriver = (resolutionChangeType == ResolutionQuickChangeType::Borderless) ? GFX_DIRECTX_WIN_BORDERLESS : GFX_DIRECTX_ACCEL;
				} else {
					m_NewGraphicsDriver = GFX_AUTODETECT_FULLSCREEN;
				}
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX();
				m_NewResY = g_FrameMan.GetMaxResY();
				break;
			case ResolutionQuickChangeType::UpscaledBorderless:
			case ResolutionQuickChangeType::UpscaledDedicated:
				if (!linuxCompatible) {
					m_NewGraphicsDriver = (resolutionChangeType == ResolutionQuickChangeType::UpscaledBorderless) ? GFX_DIRECTX_WIN_BORDERLESS : GFX_DIRECTX_ACCEL;
				} else {
					m_NewGraphicsDriver = GFX_AUTODETECT_FULLSCREEN;
				}
				m_NewResUpscaled = true;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			default:
				RTEAbort("Invalid resolution quick change type passed to SettingsVideoGUI::ApplyQuickChangeResolution!")
				break;
		}
		ApplyNewResolution();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyPresetResolution() {
		int presetResListEntryID = m_PresetResolutionComboBox->GetSelectedIndex();
		if (presetResListEntryID >= 0) {
			m_NewResUpscaled = m_PresetResolutions.at(presetResListEntryID).Upscaled;

			int newResMultiplier = m_NewResUpscaled ? 2 : 1;
			m_NewResX = m_PresetResolutions.at(presetResListEntryID).Width / newResMultiplier;
			m_NewResY = m_PresetResolutions.at(presetResListEntryID).Height / newResMultiplier;

#ifndef __unix__
			int newGfxDriver = GFX_AUTODETECT_WINDOWED;
#else
			int newGfxDriver = ((m_NewResX * newResMultiplier == g_FrameMan.GetMaxResX()) && (m_NewResY * newResMultiplier == g_FrameMan.GetMaxResY())) ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;
#endif
			m_NewGraphicsDriver = newGfxDriver;

			ApplyNewResolution();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyCustomResolution() {

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetMsg() == GUIButton::Pushed) {
				if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Windowed)) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::Windowed);
				} else if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Borderless)) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::Borderless);
				} else if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledBorderless)) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::UpscaledBorderless);
				} else if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Dedicated)) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::Dedicated);
				} else if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledDedicated)) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::UpscaledDedicated);
				} else if (guiEvent.GetControl() == m_PresetResolutionApplyButton) {
					ApplyPresetResolution();
				} else if (guiEvent.GetControl() == m_CustomResolutionApplyButton) {
					ApplyCustomResolution();
				} else if (guiEvent.GetControl() == m_ConfirmResolutionChangeButton) {
					g_GUISound.ButtonPressSound()->Play();
					// Must end any running activity otherwise have to deal with recreating all the GUI elements in GameActivity because it crashes when opening the BuyMenu. Easier to just end it.
					g_ActivityMan.EndActivity();
					g_FrameMan.ChangeResolution(m_NewResX, m_NewResY, m_NewResUpscaled, m_NewGraphicsDriver);
				} else if (guiEvent.GetControl() == m_CancelResolutionChangeButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_ResolutionChangeDialogBox->SetVisible(false);
				}
			}
		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetMsg() == GUIRadioButton::Pushed) {
				if (guiEvent.GetControl() == m_PresetResolutionRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_PresetResolutionBox->SetVisible(true);
					m_CustomResolutionBox->SetVisible(false);
				} else if (guiEvent.GetControl() == m_CustomResolutionRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_PresetResolutionBox->SetVisible(false);
					m_CustomResolutionBox->SetVisible(true);
				}
			}
		}
	}
}