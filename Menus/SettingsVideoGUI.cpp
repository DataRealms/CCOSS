#include "SettingsVideoGUI.h"
#include "FrameMan.h"
#include "ActivityMan.h"

#include "GUI.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIRadioButton.h"
#include "GUICheckbox.h"
#include "GUIComboBox.h"
#include "GUICollectionBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsVideoGUI::SettingsVideoGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_VideoSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxVideoSettings"));

		m_WindowedButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickWindowed"));
		m_BorderlessButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickBorderless"));
		m_UpscaledBorderlessButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledBorderless"));
		m_DedicatedButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickDedicated"));
		m_UpscaledDedicatedButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledDedicated"));

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

		m_NewGraphicsDriver = g_FrameMan.GetGraphicsDriver();
		m_NewResUpscaled = g_FrameMan.GetResMultiplier() > 1 ? true : false;
		m_NewResX = g_FrameMan.GetResX();
		m_NewResY = g_FrameMan.GetResY();
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
			// Disallow wacky resolutions that are taller than wide
			if (height > width) {
				return false;
			}
			// Disallow resolution width that isn't in multiples of 4 otherwise Allegro fails to initialize graphics, but only in windowed/borderless mode
			int currentGfxDriver = g_FrameMan.GetGraphicsDriver();
			if ((currentGfxDriver != GFX_AUTODETECT_FULLSCREEN && currentGfxDriver != GFX_DIRECTX_ACCEL) && width % 4 != 0) {
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

		// Index of found useful resolution (32bit)
		int foundIndex = 0;
		int currentResIndex = -1;

		std::set<PresetResolutionRecord> resRecords;

		for (int i = 0; resList && i < resList->num_modes; ++i) {
			// Only list 32bpp modes
			if (resList->mode[i].bpp == 32) {
				int width = resList->mode[i].width;
				int height = resList->mode[i].height;

				if (IsSupportedResolution(width, height)) {
					resRecords.emplace(width, height, false);

					// If this is what we're currently set to have at next start, select it afterward
					//if ((g_FrameMan.GetNewResX() * g_FrameMan.GetResMultiplier()) == width && (g_FrameMan.GetNewResY() * g_FrameMan.GetResMultiplier()) == height) { currentResIndex = foundIndex; }
					// Only increment this when we find a usable 32bit resolution
					//foundIndex++;
				}
			}
		}
		// Manually add qHD (960x540) to the list because it's rarely present in drivers
		resRecords.emplace(960, 540, false);

		std::set<PresetResolutionRecord> upscaledResRecords;
		for (const PresetResolutionRecord &resRecord : resRecords) {
			PresetResolutionRecord upscaledResRecord(resRecord.Width * 2, resRecord.Height * 2, true);
			if (upscaledResRecord.Width <= g_FrameMan.GetMaxResX() && upscaledResRecord.Height <= g_FrameMan.GetMaxResY()) { upscaledResRecords.emplace(upscaledResRecord); }
		}
		resRecords.merge(upscaledResRecords);

		m_PresetResolutions.assign(resRecords.begin(), resRecords.end());

		for (const PresetResolutionRecord &resRecord : m_PresetResolutions) {
			m_PresetResolutionComboBox->AddItem(resRecord.MakeResolutionString());
		}

		if (resList) { destroy_gfx_mode_list(resList); }


		// If none of the listed matched our resolution set for next start, add a 'custom' one to display as the current res
		/*
		if (currentResIndex < 0) {
			std::string isUpscaled = (g_FrameMan.GetResMultiplier() > 1) ? " Upscaled" : " Custom";
			std::string resString = std::to_string(g_FrameMan.GetResX() / g_FrameMan.GetResMultiplier()) + "x" + std::to_string(g_FrameMan.GetResY() / g_FrameMan.GetResMultiplier()) + isUpscaled;
			m_PresetResolutionComboBox->AddItem(resString);
			currentResIndex = m_PresetResolutionComboBox->GetCount() - 1;
		}
		// Show the current resolution item to be the selected one
		m_PresetResolutionComboBox->SetSelectedIndex(currentResIndex);
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::SetNewResolutionProperties(ResolutionChangeType resolutionChangeType, int newResX, int newResY, bool newResUpscaled, int newGfxDriver) {
		int newResMultiplier = newResUpscaled ? 2 : 1;
		bool linuxCompatible = false;

#ifdef __unix__
		linuxCompatible = true;
#endif

		switch (resolutionChangeType) {
			case ResolutionChangeType::Windowed:
				m_NewGraphicsDriver = GFX_AUTODETECT_WINDOWED;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			case ResolutionChangeType::Borderless:
				m_NewGraphicsDriver = !linuxCompatible ? GFX_DIRECTX_WIN_BORDERLESS : GFX_AUTODETECT_FULLSCREEN;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX();
				m_NewResY = g_FrameMan.GetMaxResY();
				break;
			case ResolutionChangeType::UpscaledBorderless:
				m_NewGraphicsDriver = !linuxCompatible ? GFX_DIRECTX_WIN_BORDERLESS : GFX_AUTODETECT_FULLSCREEN;
				m_NewResUpscaled = true;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			case ResolutionChangeType::Dedicated:
				m_NewGraphicsDriver = !linuxCompatible ? GFX_DIRECTX_ACCEL : GFX_AUTODETECT_FULLSCREEN;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX();
				m_NewResY = g_FrameMan.GetMaxResY();
				break;
			case ResolutionChangeType::UpscaledDedicated:
				m_NewGraphicsDriver = !linuxCompatible ? GFX_DIRECTX_ACCEL : GFX_AUTODETECT_FULLSCREEN;
				m_NewResUpscaled = true;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			default:
				m_NewGraphicsDriver = newGfxDriver;
				m_NewResUpscaled = newResUpscaled;
				m_NewResX = newResX / newResMultiplier;
				m_NewResY = newResY / newResMultiplier;
				break;
		}
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

	void SettingsVideoGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_WindowedButton) {
				SetNewResolutionProperties(ResolutionChangeType::Windowed);
				ApplyNewResolution();
			} else if (guiEvent.GetControl() == m_BorderlessButton) {
				SetNewResolutionProperties(ResolutionChangeType::Borderless);
				ApplyNewResolution();
			} else if (guiEvent.GetControl() == m_UpscaledBorderlessButton) {
				SetNewResolutionProperties(ResolutionChangeType::UpscaledBorderless);
				ApplyNewResolution();
			} else if (guiEvent.GetControl() == m_DedicatedButton) {
				SetNewResolutionProperties(ResolutionChangeType::Dedicated);
				ApplyNewResolution();
			} else if (guiEvent.GetControl() == m_UpscaledDedicatedButton) {
				SetNewResolutionProperties(ResolutionChangeType::UpscaledDedicated);
				ApplyNewResolution();
			} else if (guiEvent.GetControl() == m_PresetResolutionApplyButton || guiEvent.GetControl() == m_CustomResolutionApplyButton) {
				ApplyNewResolution();

			} else if (guiEvent.GetControl() == m_ConfirmResolutionChangeButton) {
				g_GUISound.ButtonPressSound()->Play();
				// Must end any running activity otherwise have to deal with recreating all the GUI elements in GameActivity because it crashes when opening the BuyMenu. Easier to just end it.
				g_ActivityMan.EndActivity();
				g_FrameMan.ChangeResolution(m_NewResX, m_NewResY, m_NewResUpscaled, m_NewGraphicsDriver);
			} else if (guiEvent.GetControl() == m_CancelResolutionChangeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ResolutionChangeDialogBox->SetVisible(false);
			}

		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			if (guiEvent.GetControl() == m_PresetResolutionComboBox && guiEvent.GetMsg() == GUIComboBox::Closed) {
				int resListEntryID = m_PresetResolutionComboBox->GetSelectedIndex();
				if (resListEntryID >= 0) { SetNewResolutionPropertiesFromPreset(m_PresetResolutions.at(resListEntryID)); }
			} else if (guiEvent.GetControl() == m_PresetResolutionRadioButton && guiEvent.GetMsg() == GUIRadioButton::Pushed) {
				g_GUISound.ButtonPressSound()->Play();
				m_PresetResolutionBox->SetVisible(true);
				m_CustomResolutionBox->SetVisible(false);
				int resListEntryID = m_PresetResolutionComboBox->GetSelectedIndex();
				if (resListEntryID >= 0) { SetNewResolutionPropertiesFromPreset(m_PresetResolutions.at(resListEntryID)); }
			} else if (guiEvent.GetControl() == m_CustomResolutionRadioButton && guiEvent.GetMsg() == GUIRadioButton::Pushed) {
				g_GUISound.ButtonPressSound()->Play();
				m_PresetResolutionBox->SetVisible(false);
				m_CustomResolutionBox->SetVisible(true);
			}
		}
	}
}