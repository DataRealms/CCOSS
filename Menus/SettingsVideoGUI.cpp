#include "SettingsVideoGUI.h"
#include "FrameMan.h"
#include "ActivityMan.h"

#include "GUI.h"
#include "GUILabel.h"
#include "GUIButton.h"
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
		m_DedicatedFullscreenButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickDedicatedFullscreen"));

		m_PresetResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionPresetResolution"));
		m_PresetResolutionComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboPresetResolution"));
		m_PresetResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyPresetResolution"));
		m_PresetResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("CollectionPresetResolution"));

		m_CustomResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionCustomResolution"));
		m_CustomResolutionBox->SetVisible(false);

		m_ResolutionChangeDialogBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ResolutionChangeDialog"));
		m_ResolutionChangeDialogBox->CenterInParent(true, true);
		m_ResolutionChangeDialogBox->SetVisible(false);

		m_ConfirmResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChange"));
		m_ConfirmResolutionChangeButton->SetVisible(false);
		m_ConfirmResolutionChangeFullscreenButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChangeFullscreen"));
		m_ConfirmResolutionChangeFullscreenButton->SetVisible(false);
		m_CancelResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelResolutionChange"));

		m_PresetResolutions.clear();
		PopulateResolutionsComboBox();

		SetEnabled(false);
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

	void SettingsVideoGUI::PopulateResolutionsComboBox() {
	#ifdef _WIN32
		// Get a list of modes from the fullscreen driver even though we're not using it. This is so we don't need to populate the list manually and has all the reasonable resolutions (along with some stupid ones but whatever)
		GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
#elif __unix__
		GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_XWINDOWS_FULLSCREEN);
#endif
		// Index of found useful resolution (32bit)
		int foundIndex = 0;
		int currentResIndex = -1;

		for (int i = 0; resList && i < resList->num_modes; ++i) {
			// Only list 32bpp modes
			if (resList->mode[i].bpp == 32) {
				int width = resList->mode[i].width;
				int height = resList->mode[i].height;

				if (g_FrameMan.IsSupportedResolution(width, height)) {
					m_PresetResolutions.emplace(width, height, false);

					// If this is what we're currently set to have at next start, select it afterward
					if ((g_FrameMan.GetNewResX() * g_FrameMan.ResolutionMultiplier()) == width && (g_FrameMan.GetNewResY() * g_FrameMan.ResolutionMultiplier()) == height) { currentResIndex = foundIndex; }
					// Only increment this when we find a usable 32bit resolution
					foundIndex++;
				}
			}
		}
		// Manually add qHD (960x540) to the list because it's rarely present in drivers
		m_PresetResolutions.emplace(960, 540, false);

		std::set<PresetResolutionRecord> upscaledResRecords;
		for (const PresetResolutionRecord &resRecord : m_PresetResolutions) {
			PresetResolutionRecord upscaledResRecord(resRecord.Width * 2, resRecord.Height * 2, true);
			if (upscaledResRecord.Width <= g_FrameMan.GetMaxResX() && upscaledResRecord.Height <= g_FrameMan.GetMaxResY()) { upscaledResRecords.emplace(upscaledResRecord); }
		}
		m_PresetResolutions.merge(upscaledResRecords);

		for (const PresetResolutionRecord &resRecord : m_PresetResolutions) {
			m_PresetResolutionComboBox->AddItem(resRecord.MakeResolutionString());
		}

		if (resList) { destroy_gfx_mode_list(resList); }


		// If none of the listed matched our resolution set for next start, add a 'custom' one to display as the current res
		if (currentResIndex < 0) {
			std::string isUpscaled = (g_FrameMan.ResolutionMultiplier() > 1) ? " Upscaled" : " Custom";
			std::string resString = std::to_string(g_FrameMan.GetResX() / g_FrameMan.ResolutionMultiplier()) + "x" + std::to_string(g_FrameMan.GetResY() / g_FrameMan.ResolutionMultiplier()) + isUpscaled;
			m_PresetResolutionComboBox->AddItem(resString);
			currentResIndex = m_PresetResolutionComboBox->GetCount() - 1;
		}
		// Show the current resolution item to be the selected one
		m_PresetResolutionComboBox->SetSelectedIndex(currentResIndex);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_WindowedButton) {
				if (g_ActivityMan.GetActivity()) {
					m_ResolutionChangeToUpscaled = false;
					m_ResolutionChangeDialogBox->SetVisible(true);
					m_ConfirmResolutionChangeFullscreenButton->SetVisible(true);
				} else {
					g_FrameMan.SwitchResolution(960, 540);
				}
				g_GUISound.ButtonPressSound()->Play();
			} else if (guiEvent.GetControl() == m_BorderlessButton) {
				if (!g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
					if (g_ActivityMan.GetActivity()) {
						m_ResolutionChangeToUpscaled = false;
						m_ResolutionChangeDialogBox->SetVisible(true);
						m_ConfirmResolutionChangeFullscreenButton->SetVisible(true);
					} else {
						g_FrameMan.SwitchToFullscreen(false);
					}
				}
				g_GUISound.ButtonPressSound()->Play();
			} else if (guiEvent.GetControl() == m_UpscaledBorderlessButton) {
				if (!g_FrameMan.IsUpscaledFullscreen()) {
					if (g_ActivityMan.GetActivity()) {
						m_ResolutionChangeToUpscaled = true;
						m_ResolutionChangeDialogBox->SetVisible(true);
						m_ConfirmResolutionChangeFullscreenButton->SetVisible(true);
					} else {
						g_FrameMan.SwitchToFullscreen(true);
					}
				}
				g_GUISound.ButtonPressSound()->Play();
			} else if (guiEvent.GetControl() == m_DedicatedFullscreenButton) {
				;

			} else if (guiEvent.GetControl() == m_ConfirmResolutionChangeFullscreenButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ResolutionChangeDialogBox->SetVisible(false);
				m_ConfirmResolutionChangeFullscreenButton->SetVisible(false);
				if (!m_ResolutionChangeToUpscaled && g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
					g_FrameMan.SwitchResolution(960, 540, 1, true);
				} else {
					g_FrameMan.SwitchToFullscreen(m_ResolutionChangeToUpscaled ? true : false, true);
				}

			} else if (guiEvent.GetControl() == m_ConfirmResolutionChangeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ResolutionChangeDialogBox->SetVisible(false);
				m_ConfirmResolutionChangeButton->SetVisible(false);
				g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1, true);

			} else if (guiEvent.GetControl() == m_CancelResolutionChangeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ResolutionChangeDialogBox->SetVisible(false);
			}


		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			// Resolution combobox closed, something new selected
			if (guiEvent.GetControl() == m_PresetResolutionComboBox) {
				// Closed it, IE selected something
				if (guiEvent.GetMsg() == GUIComboBox::Closed) {
					// Get and read the new resolution data from the item's label
					GUIListPanel::Item *pResItem = m_PresetResolutionComboBox->GetItem(m_PresetResolutionComboBox->GetSelectedIndex());
					if (pResItem && !pResItem->m_Name.empty()) {
						int newResX;
						int newResY;
						sscanf(pResItem->m_Name.c_str(), "%4dx%4d", &newResX, &newResY);
						// Sanity check the values and then set them as the new resolution to be switched to next time FrameMan is created
						if (g_FrameMan.IsSupportedResolution(newResX, newResY)) {
							g_FrameMan.SetNewResX(newResX);
							g_FrameMan.SetNewResY(newResY);
						}
					}
					if (g_FrameMan.IsNewResSet()) {
						if (g_ActivityMan.GetActivity()) {
							m_ResolutionChangeDialogBox->SetVisible(true);
							m_ConfirmResolutionChangeButton->SetVisible(true);
						} else {
							g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1);
						}
					}
				}
			}
		}
	}
}