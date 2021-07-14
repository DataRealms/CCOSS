#include "SettingsVideoGUI.h"
#include "FrameMan.h"
#include "ActivityMan.h"

#include "AllegroInput.h"
#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIRadioButton.h"
#include "GUICheckbox.h"
#include "GUIComboBox.h"
#include "GUITextBox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsVideoGUI::SettingsVideoGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_NewGraphicsDriver = g_FrameMan.GetGraphicsDriver();
		m_NewResX = g_FrameMan.GetResX();
		m_NewResY = g_FrameMan.GetResY();
		m_NewResUpscaled = g_FrameMan.GetResMultiplier() > 1;

		m_VideoSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxVideoSettings"));

		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Windowed) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickWindowed"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Borderless) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickBorderless"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledBorderless) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledBorderless"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::Dedicated) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickDedicated"));
		m_ResolutionQuickToggleButtons.at(ResolutionQuickChangeType::UpscaledDedicated) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickUpscaledDedicated"));

		m_TwoPlayerSplitscreenHSplitRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioSplitscreenHoriz"));
		m_TwoPlayerSplitscreenVSplitRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioSplitscreenVert"));
		m_TwoPlayerSplitscreenVSplitRadioButton->SetCheck(g_FrameMan.GetTwoPlayerVSplit());

		m_PresetResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioPresetResolution"));
		m_CustomResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioCustomResolution"));

		m_ResolutionChangeDialogBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ResolutionChangeDialog"));
		m_ResolutionChangeDialogBox->SetVisible(false);

		GUICollectionBox *settingsRootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSettingsBase"));
		m_ResolutionChangeDialogBox->SetPositionAbs(settingsRootBox->GetXPos() + ((settingsRootBox->GetWidth() - m_ResolutionChangeDialogBox->GetWidth()) / 2), settingsRootBox->GetYPos() + ((settingsRootBox->GetHeight() - m_ResolutionChangeDialogBox->GetHeight()) / 2));

		m_ResolutionChangeConfirmButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChange"));
		m_ResolutionChangeCancelButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelResolutionChange"));

		CreatePresetResolutionBox();
		CreateCustomResolutionBox();

		if (m_PresetResolutionComboBox->GetSelectedIndex() < 0 || g_FrameMan.IsUsingDedicatedGraphicsDriver()) {
			m_CustomResolutionRadioButton->SetCheck(true);
			m_PresetResolutionBox->SetVisible(false);
			m_CustomResolutionBox->SetVisible(true);
			if (g_FrameMan.IsUsingDedicatedGraphicsDriver()) {
				m_CustomResolutionDedicatedRadioButton->SetCheck(true);
				m_CustomResolutionMessageLabel->SetText("WARNING: ATTEMPTING TO SET A RESOLUTION NOT SUPPORTED BY YOUR GRAPHICS CARD OR MONITOR WITH THE DEDICATED DRIVER MAY LEAD TO THE GAME OR SYSTEM HARD-LOCKING!");
				m_CustomResolutionMessageLabel->SetVisible(true);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::CreatePresetResolutionBox() {
		m_PresetResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionPresetResolution"));
		m_PresetResolutionComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboPresetResolution"));
		m_PresetResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyPresetResolution"));
		m_PresetResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPresetResolutonValidation"));

		if (g_FrameMan.GetMaxResX() == 1366 && g_FrameMan.GetMaxResY() == 768) {
			m_PresetResolutionMessageLabel->SetText("1366x768 is not supported by the borderless driver for reasons unknown to man.\nPlease use the custom resolution controls with the dedicated driver to set it.");
		} else {
			m_PresetResolutionMessageLabel->SetVisible(false);
		}
		PopulateResolutionsComboBox();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::CreateCustomResolutionBox() {
		m_CustomResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionCustomResolution"));
		m_CustomResolutionBox->SetVisible(false);

		m_CustomResolutionWidthTextBox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCustomWidth"));
		m_CustomResolutionWidthTextBox->SetNumericOnly(true);
		m_CustomResolutionWidthTextBox->SetMaxNumericValue(g_FrameMan.GetMaxResX());
		m_CustomResolutionWidthTextBox->SetMaxTextLength(4);
		m_CustomResolutionWidthTextBox->SetText(std::to_string(m_NewResX * g_FrameMan.GetResMultiplier()));

		m_CustomResolutionHeightTextBox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCustomHeight"));
		m_CustomResolutionHeightTextBox->SetNumericOnly(true);
		m_CustomResolutionHeightTextBox->SetMaxNumericValue(g_FrameMan.GetMaxResY());
		m_CustomResolutionHeightTextBox->SetMaxTextLength(4);
		m_CustomResolutionHeightTextBox->SetText(std::to_string(m_NewResY * g_FrameMan.GetResMultiplier()));

		m_CustomResolutionUpscaledCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxCustomUpscaled"));
		m_CustomResolutionUpscaledCheckbox->SetCheck(m_NewResUpscaled);

		m_CustomResolutionBorderlessRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioBorderlessDriver"));
		m_CustomResolutionDedicatedRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioDedicatedDriver"));
		m_CustomResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyCustomResolution"));
		m_CustomResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelCustomResolutionValidation"));
		m_CustomResolutionMessageLabel->SetVisible(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::SetEnabled(bool enable) const {
		m_VideoSettingsBox->SetVisible(enable);
		m_VideoSettingsBox->SetEnabled(enable);

		if (enable) {
			if (m_CustomResolutionWidthTextBox->GetText().empty()) { m_CustomResolutionWidthTextBox->SetText(std::to_string(g_FrameMan.GetResX())); }
			if (m_CustomResolutionHeightTextBox->GetText().empty()) { m_CustomResolutionHeightTextBox->SetText(std::to_string(g_FrameMan.GetResY())); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUICollectionBox * SettingsVideoGUI::GetActiveDialogBox() const {
		return (m_ResolutionChangeDialogBox->GetEnabled() && m_ResolutionChangeDialogBox->GetVisible()) ? m_ResolutionChangeDialogBox : nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::CloseActiveDialogBox() const {
		if (m_ResolutionChangeDialogBox->GetEnabled() && m_ResolutionChangeDialogBox->GetVisible()) {
			m_ResolutionChangeDialogBox->SetVisible(false);
			m_VideoSettingsBox->SetEnabled(true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsVideoGUI::IsSupportedResolution(int width, int height) const {
		if ((width >= 640 && height >= 450) && (width <= g_FrameMan.GetMaxResX() && height <= g_FrameMan.GetMaxResY())) {
			// Disallow wacky resolutions that are taller than wide and some other dumb ones.
			if ((height > width) || (width == 1152 && height == 864) || (width == 1176 && height == 664)) {
				return false;
			}
			// Disallow resolution width that isn't in multiples of 4 otherwise Allegro fails to initialize graphics, but only in windowed/borderless mode.
			if (!g_FrameMan.IsUsingDedicatedGraphicsDriver() && (width % 4 != 0)) {
				return false;
			}
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::PopulateResolutionsComboBox() {
		m_PresetResolutions.clear();

		// Get a list of modes from the fullscreen driver even though we're not necessarily using it. This is so we don't need to populate the list manually, and have all the reasonable resolutions.
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
			if (resList->mode[i].bpp == 32) {
				int width = resList->mode[i].width;
				int height = resList->mode[i].height;
				if (IsSupportedResolution(width, height)) { resRecords.emplace(width, height, false); }
			}
		}
		// Manually add qHD (960x540) to the list because it's rarely present in drivers.
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
			m_PresetResolutionComboBox->AddItem(resRecord.GetDisplayString());
			if (m_PresetResolutionComboBox->GetSelectedIndex() < 0 && (resRecord.Width == g_FrameMan.GetResX() * g_FrameMan.GetResMultiplier()) && (resRecord.Height == g_FrameMan.GetResY() * g_FrameMan.GetResMultiplier()) && (resRecord.Upscaled == g_FrameMan.GetResMultiplier() > 1)) {
				m_PresetResolutionComboBox->SetSelectedIndex(i);
			}
		}
		destroy_gfx_mode_list(resList);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyNewResolution() {
		if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			m_ResolutionChangeDialogBox->SetVisible(true);
			m_VideoSettingsBox->SetEnabled(false);
		} else {
			m_ResolutionChangeDialogBox->SetVisible(false);
			m_VideoSettingsBox->SetEnabled(true);
			g_FrameMan.ChangeResolution(m_NewResX, m_NewResY, m_NewResUpscaled, m_NewGraphicsDriver);
			dynamic_cast<AllegroInput *>(m_GUIControlManager->GetManager()->GetInputController())->AdjustMouseMovementSpeedToGraphicsDriver(m_NewGraphicsDriver);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyQuickChangeResolution(ResolutionQuickChangeType resolutionChangeType) {
		switch (resolutionChangeType) {
			case ResolutionQuickChangeType::Windowed:
				m_NewGraphicsDriver = GFX_AUTODETECT_WINDOWED;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			case ResolutionQuickChangeType::Borderless:
			case ResolutionQuickChangeType::Dedicated:
				m_NewGraphicsDriver = (resolutionChangeType == ResolutionQuickChangeType::Borderless) ? GFX_DIRECTX_WIN_BORDERLESS : GFX_DIRECTX_ACCEL;
				m_NewResUpscaled = false;
				m_NewResX = g_FrameMan.GetMaxResX();
				m_NewResY = g_FrameMan.GetMaxResY();
				break;
			case ResolutionQuickChangeType::UpscaledBorderless:
			case ResolutionQuickChangeType::UpscaledDedicated:
				m_NewGraphicsDriver = (resolutionChangeType == ResolutionQuickChangeType::UpscaledBorderless) ? GFX_DIRECTX_WIN_BORDERLESS : GFX_DIRECTX_ACCEL;
				m_NewResUpscaled = true;
				m_NewResX = g_FrameMan.GetMaxResX() / 2;
				m_NewResY = g_FrameMan.GetMaxResY() / 2;
				break;
			default:
				RTEAbort("Invalid resolution quick change type passed to SettingsVideoGUI::ApplyQuickChangeResolution!")
				break;
		}
		g_GUISound.ButtonPressSound()->Play();
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
			m_NewGraphicsDriver = GFX_AUTODETECT_WINDOWED;

#ifdef __unix__
			m_NewGraphicsDriver = ((m_NewResX * newResMultiplier == g_FrameMan.GetMaxResX()) && (m_NewResY * newResMultiplier == g_FrameMan.GetMaxResY())) ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;
#endif

			g_GUISound.ButtonPressSound()->Play();
			ApplyNewResolution();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyCustomResolution() {
		m_CustomResolutionMessageLabel->SetVisible(false);

		m_NewResUpscaled = m_CustomResolutionUpscaledCheckbox->GetCheck();
		int newMultiplier = m_NewResUpscaled ? 2 : 1;
		m_NewResX = std::stoi(m_CustomResolutionWidthTextBox->GetText()) / newMultiplier;
		m_NewResY = std::stoi(m_CustomResolutionHeightTextBox->GetText()) / newMultiplier;
		m_NewGraphicsDriver = m_CustomResolutionBorderlessRadioButton->GetCheck() ? GFX_AUTODETECT_WINDOWED : GFX_DIRECTX_ACCEL;

		bool invalidResolution = false;

		if (m_NewGraphicsDriver == GFX_AUTODETECT_WINDOWED && m_NewResX % 4 != 0) {
			m_CustomResolutionMessageLabel->SetText("Resolution width not divisible by 4 is not supported by the borderless driver.\nPlease use the dedicated driver instead.");
			invalidResolution = true;
		} else if (m_NewResY > m_NewResX) {
			m_CustomResolutionMessageLabel->SetText("Resolution that is taller than wide is not supported.");
			invalidResolution = true;
		} else if (m_NewResX * newMultiplier < 640 || m_NewResY * newMultiplier < 384) {
			m_CustomResolutionMessageLabel->SetText("Resolution width or height lower than the minimum (640x384) is not supported.");
			invalidResolution = true;
		}
		g_GUISound.ButtonPressSound()->Play();

		if (invalidResolution) {
			m_CustomResolutionMessageLabel->SetVisible(true);
			return;
		} else {
			ApplyNewResolution();
		}
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
				} else if (guiEvent.GetControl() == m_ResolutionChangeConfirmButton) {
					g_GUISound.ButtonPressSound()->Play();
					// Must end any running activity otherwise have to deal with recreating all the GUI elements in GameActivity because it crashes when opening the BuyMenu. Easier to just end it.
					g_ActivityMan.EndActivity();
					ApplyNewResolution();
				} else if (guiEvent.GetControl() == m_ResolutionChangeCancelButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_ResolutionChangeDialogBox->SetVisible(false);
					m_VideoSettingsBox->SetEnabled(true);
				}
			}
		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			// Clicking off focused textboxes does not remove their focus and they will still capture keyboard input, so remove focus when clicking CollectionBoxes.
			if (guiEvent.GetMsg() == GUICollectionBox::Clicked && !m_VideoSettingsBox->HasFocus() && (guiEvent.GetControl() == m_VideoSettingsBox || guiEvent.GetControl() == m_CustomResolutionBox)) {
				if (m_CustomResolutionWidthTextBox->GetText().empty()) { m_CustomResolutionWidthTextBox->SetText(std::to_string(g_FrameMan.GetResX())); }
				if (m_CustomResolutionHeightTextBox->GetText().empty()) { m_CustomResolutionHeightTextBox->SetText(std::to_string(g_FrameMan.GetResY())); }
				m_VideoSettingsBox->SetFocus();
			}

			if (guiEvent.GetMsg() == GUIRadioButton::Pushed) {
				if (guiEvent.GetControl() == m_TwoPlayerSplitscreenHSplitRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					g_FrameMan.SetTwoPlayerVSplit(false);
				} else if (guiEvent.GetControl() == m_TwoPlayerSplitscreenVSplitRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					g_FrameMan.SetTwoPlayerVSplit(true);
				} else if (guiEvent.GetControl() == m_PresetResolutionRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_PresetResolutionBox->SetVisible(true);
					m_CustomResolutionBox->SetVisible(false);
				} else if (guiEvent.GetControl() == m_CustomResolutionRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_PresetResolutionBox->SetVisible(false);
					m_CustomResolutionBox->SetVisible(true);
				} else if (guiEvent.GetControl() == m_CustomResolutionBorderlessRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_CustomResolutionMessageLabel->SetVisible(false);
				} else if (guiEvent.GetControl() == m_CustomResolutionDedicatedRadioButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_CustomResolutionMessageLabel->SetText("WARNING: ATTEMPTING TO SET A RESOLUTION NOT SUPPORTED BY YOUR GRAPHICS CARD OR MONITOR WITH THE DEDICATED DRIVER MAY LEAD TO THE GAME OR SYSTEM HARD-LOCKING!");
					m_CustomResolutionMessageLabel->SetVisible(true);
				}
			}
		}
	}
}