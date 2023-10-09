#include "SettingsVideoGUI.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "ActivityMan.h"

#include "GUIInputWrapper.h"
#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUILabel.h"
#include "GUIButton.h"
#include "GUIRadioButton.h"
#include "GUICheckbox.h"
#include "GUIComboBox.h"
#include "GUITextBox.h"

#include "SDL_video.h"
#include "glm/ext.hpp"
#include "glm/gtc/epsilon.hpp"
#if __cpp_lib_format >= 201907L
#include <format>
#endif

namespace RTE {

	std::string SettingsVideoGUI::PresetResolutionRecord::GetDisplayString() const {
#if __cpp_lib_format >= 201907L
		return std::format("{}x{} ({:.1g}x Fullscreen scale)", Width, Height, Scale);
#else
		return std::to_string(Width) + "x" + std::to_string(Height); 
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsVideoGUI::SettingsVideoGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_NewResX = g_WindowMan.GetResX();
		m_NewResY = g_WindowMan.GetResY();
		m_NewResMultiplier = g_WindowMan.GetResMultiplier();
		m_NewFullscreen = g_WindowMan.IsFullscreen();

		m_VideoSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxVideoSettings"));

		m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Windowed] = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickWindowed"));
		m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Fullscreen] = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonQuickBorderless"));

		m_TwoPlayerSplitscreenHSplitRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioSplitscreenHoriz"));
		m_TwoPlayerSplitscreenVSplitRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioSplitscreenVert"));
		m_TwoPlayerSplitscreenVSplitRadioButton->SetCheck(g_FrameMan.GetTwoPlayerVSplit());

		m_EnableVSyncCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxEnableVSync"));
		m_EnableVSyncCheckbox->SetCheck(g_WindowMan.GetVSyncEnabled());

		m_FullscreenCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxFullscreen"));
		m_FullscreenCheckbox->SetCheck(m_NewFullscreen);

		m_UseMultiDisplaysCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxUseMultiDisplays"));
		m_UseMultiDisplaysCheckbox->SetCheck(g_WindowMan.GetUseMultiDisplays());
		m_UseMultiDisplaysCheckbox->SetVisible(m_UseMultiDisplaysCheckbox->GetVisible() && SDL_GetNumVideoDisplays() > 1);

		m_PresetResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioPresetResolution"));
		m_CustomResolutionRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioCustomResolution"));

		m_ResolutionChangeDialogBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ResolutionChangeDialog"));
		m_ResolutionChangeDialogBox->SetVisible(false);

		const GUICollectionBox *settingsRootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSettingsBase"));
		m_ResolutionChangeDialogBox->SetPositionAbs(settingsRootBox->GetXPos() + ((settingsRootBox->GetWidth() - m_ResolutionChangeDialogBox->GetWidth()) / 2), settingsRootBox->GetYPos() + ((settingsRootBox->GetHeight() - m_ResolutionChangeDialogBox->GetHeight()) / 2));

		m_ResolutionChangeConfirmButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChange"));
		m_ResolutionChangeCancelButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelResolutionChange"));

		CreatePresetResolutionBox();
		CreateCustomResolutionBox();

		double aspectRatio = g_WindowMan.GetResX() / static_cast<double>(g_WindowMan.GetResY());
		double presetRatio = g_WindowMan.GetMaxResX() / static_cast<double>(g_WindowMan.GetMaxResY());

		if (!g_WindowMan.IsFullscreen() && glm::epsilonNotEqual(aspectRatio, presetRatio, glm::epsilon<double>()) && m_CustomResolutionRadioButton->IsEnabled()) {
			m_CustomResolutionRadioButton->SetCheck(true);
			m_PresetResolutionBox->SetVisible(false);
			m_CustomResolutionBox->SetVisible(true);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::CreatePresetResolutionBox() {
		m_PresetResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionPresetResolution"));
		m_PresetResolutionComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboPresetResolution"));
		m_PresetResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyPresetResolution"));
		m_PresetResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPresetResolutonValidation"));
		m_PresetResolutionMessageLabel->SetVisible(false);

		PopulateResolutionsComboBox();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::CreateCustomResolutionBox() {
		m_CustomResolutionBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionCustomResolution"));
		m_CustomResolutionBox->SetVisible(false);

		m_CustomResolutionWidthTextBox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCustomWidth"));
		m_CustomResolutionWidthTextBox->SetNumericOnly(true);
		m_CustomResolutionWidthTextBox->SetMaxNumericValue(g_WindowMan.GetMaxResX());
		m_CustomResolutionWidthTextBox->SetMaxTextLength(4);
		m_CustomResolutionWidthTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResX())));

		m_CustomResolutionHeightTextBox = dynamic_cast<GUITextBox *>(m_GUIControlManager->GetControl("TextboxCustomHeight"));
		m_CustomResolutionHeightTextBox->SetNumericOnly(true);
		m_CustomResolutionHeightTextBox->SetMaxNumericValue(g_WindowMan.GetMaxResY());
		m_CustomResolutionHeightTextBox->SetMaxTextLength(4);
		m_CustomResolutionHeightTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResY())));

		m_CustomResolutionMultiplierComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboboxResolutionMultiplier"));
		PopulateResMultplierComboBox();
#if __cpp_lib_format >= 201907L
		m_CustomResolutionMultiplierComboBox->SetText(std::format("{:.3g}x", m_NewResMultiplier));
#else
		m_CustomResolutionMultiplierComboBox->SetText(std::to_string(m_NewResMultiplier));
#endif

		m_CustomResolutionApplyButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonApplyCustomResolution"));
		m_CustomResolutionMessageLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelCustomResolutionValidation"));
		m_CustomResolutionMessageLabel->SetVisible(false);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::SetEnabled(bool enable) const {
		m_VideoSettingsBox->SetVisible(enable);
		m_VideoSettingsBox->SetEnabled(enable);

		if (enable) {
			m_CustomResolutionWidthTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResX())));
			m_CustomResolutionHeightTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResY())));
#if __cpp_lib_format >= 201907L
			m_CustomResolutionMultiplierComboBox->SetText(std::format("{:.3g}x", m_NewResMultiplier));
#else
			m_CustomResolutionMultiplierComboBox->SetText(std::to_string(m_NewResMultiplier));
#endif
			std::string windowedText = g_WindowMan.IsFullscreen() ? "Windowed" : "Scale To Window";
			m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Windowed]->SetText(windowedText);
			std::string fullscreenText = g_WindowMan.IsFullscreen() ? "Scale To Fullscreen" : "Fullscreen";
			m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Fullscreen]->SetText(fullscreenText);
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
		if ((width >= c_MinResX && height >= c_MinResY) && (width <= g_WindowMan.GetMaxResX() && height <= g_WindowMan.GetMaxResY())) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::PopulateResolutionsComboBox() {
		m_PresetResolutions.clear();
		m_PresetResolutionComboBox->ClearList();

		std::set<PresetResolutionRecord> resRecords;
		g_WindowMan.MapDisplays();
		int resX = g_WindowMan.GetMaxResX();
		int resY = g_WindowMan.GetMaxResY();
		float scale = 1.0f;
		while ((resX >= c_MinResX) && (resY >= c_MinResY)) {
			if (IsSupportedResolution(resX, resY)) {
				resRecords.emplace(resX, resY, scale);
			}
			scale += 0.5f;
			resX = g_WindowMan.GetMaxResX() / scale;
			resY = g_WindowMan.GetMaxResY() / scale;
		}

		m_PresetResolutions.assign(resRecords.begin(), resRecords.end());

		float defaultScale = std::min<float>(std::round(g_WindowMan.GetMaxResX() / static_cast<float>(c_DefaultResX)), std::round(g_WindowMan.GetMaxResY() / static_cast<float>(c_DefaultResY)));

		for (int i = 0; i < m_PresetResolutions.size(); ++i) {
			const PresetResolutionRecord &resRecord = m_PresetResolutions[i];
			m_PresetResolutionComboBox->AddItem(resRecord.GetDisplayString());
			if (m_PresetResolutionComboBox->GetSelectedIndex() < 0 && (glm::epsilonEqual(resRecord.Scale, defaultScale, 0.5f))) {
				m_PresetResolutionComboBox->SetSelectedIndex(i);
			}
		}
	}

	void SettingsVideoGUI::PopulateResMultplierComboBox() {
		float maxResX = g_WindowMan.GetUseMultiDisplays() ? g_WindowMan.GetPrimaryWindowDisplayWidth() : g_WindowMan.GetMaxResX();
		float maxResY = g_WindowMan.GetUseMultiDisplays() ? g_WindowMan.GetPrimaryWindowDisplayHeight() : g_WindowMan.GetMaxResY();

		float maximumResMultiplier = std::floor((std::min<float>(maxResX / c_MinResX, maxResY / c_MinResY) * 2.f) + 0.5f) / 2.0f;
		m_CustomResolutionMultiplierComboBox->ClearList();

		for (float resMultiplier = 1.0f; resMultiplier <= maximumResMultiplier; resMultiplier += 0.5) {
#if __cpp_lib_format >= 201907L
			m_CustomResolutionMultiplierComboBox->AddItem(std::format("{:.3g}x", resMultiplier));
#else
			m_CustomResolutionMultiplierComboBox->AddItem(std::to_string(resMultiplier));
#endif
		}
		m_CustomResolutionMultiplierComboBox->SetSelectedIndex(0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::UpdateCustomResolutionLimits() {
		g_WindowMan.MapDisplays();

		int newMaxResX = g_WindowMan.GetMaxResX();
		int newMaxResY = g_WindowMan.GetMaxResY();

		m_CustomResolutionWidthTextBox->SetMaxNumericValue(newMaxResX);
		if (std::string widthText = m_CustomResolutionWidthTextBox->GetText(); widthText.empty() || (!widthText.empty() && std::stoi(widthText) > newMaxResX)) {
			m_CustomResolutionWidthTextBox->SetText(std::to_string(newMaxResX));
		}

		m_CustomResolutionHeightTextBox->SetMaxNumericValue(newMaxResY);
		if (std::string heightText = m_CustomResolutionHeightTextBox->GetText(); heightText.empty() || (!heightText.empty() && std::stoi(heightText) > newMaxResY)) {
			m_CustomResolutionHeightTextBox->SetText(std::to_string(newMaxResY));
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyNewResolution(bool displaysWereMapped) {
		bool needWarning = (g_WindowMan.GetResX() != m_NewResX) && (g_WindowMan.GetResY() != m_NewResY);
		if (needWarning && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			m_ResolutionChangeDialogBox->SetVisible(true);
			m_VideoSettingsBox->SetEnabled(false);
		} else {
			m_ResolutionChangeDialogBox->SetVisible(false);
			m_VideoSettingsBox->SetEnabled(true);
			g_WindowMan.ChangeResolution(m_NewResX, m_NewResY, m_NewResMultiplier, m_NewFullscreen, displaysWereMapped);
			m_FullscreenCheckbox->SetCheck(g_WindowMan.IsFullscreen());
			std::string windowedText = g_WindowMan.IsFullscreen() ? "Windowed" : "Scale To Window";
			m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Windowed]->SetText(windowedText);
			std::string fullscreenText = g_WindowMan.IsFullscreen() ? "Scale To Fullscreen" : "Fullscreen";
			m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Fullscreen]->SetText(fullscreenText);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyQuickChangeResolution(ResolutionQuickChangeType resolutionChangeType) {
		g_WindowMan.MapDisplays();

		switch (resolutionChangeType) {
			case ResolutionQuickChangeType::Windowed:
				if (g_WindowMan.IsFullscreen()) {
					m_NewResMultiplier = g_WindowMan.GetResMultiplier();
					m_NewResX = g_WindowMan.GetResX();
					m_NewResY = g_WindowMan.GetResY();
					m_NewFullscreen = false;
				} else {
					m_NewResMultiplier = std::min<float>(std::round(g_WindowMan.GetWindowResX() / static_cast<float>(c_DefaultResX)), std::round(g_WindowMan.GetWindowResY() / static_cast<float>(c_DefaultResY)));
					m_NewResX = g_WindowMan.GetWindowResX() / m_NewResMultiplier;
					m_NewResY = g_WindowMan.GetWindowResY() / m_NewResMultiplier;
					m_NewFullscreen = false;
				}
				break;
			case ResolutionQuickChangeType::Fullscreen:
				if (!g_WindowMan.IsFullscreen() || g_WindowMan.GetUseMultiDisplays()) {
					m_NewResMultiplier = g_WindowMan.GetResMultiplier();
					m_NewResX = g_WindowMan.GetResX();
					m_NewResY = g_WindowMan.GetResY();
					m_NewFullscreen = true;
				} else {
					m_NewResMultiplier = std::min<float>(std::round(g_WindowMan.GetMaxResX() / static_cast<float>(c_DefaultResX)), std::round(g_WindowMan.GetMaxResY() / static_cast<float>(c_DefaultResY)));
					m_NewResX = g_WindowMan.GetMaxResX() / m_NewResMultiplier;
					m_NewResY = g_WindowMan.GetMaxResY() / m_NewResMultiplier;
					m_NewFullscreen = true;
				}
				break;
			default:
				RTEAbort("Invalid resolution quick change type passed to SettingsVideoGUI::ApplyQuickChangeResolution!");
				break;
		}
		g_GUISound.ButtonPressSound()->Play();
		ApplyNewResolution(true);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyPresetResolution() {
		int presetResListEntryID = m_PresetResolutionComboBox->GetSelectedIndex();
		if (presetResListEntryID >= 0) {

			m_NewResX = m_PresetResolutions.at(presetResListEntryID).Width;
			m_NewResY = m_PresetResolutions.at(presetResListEntryID).Height;

			g_GUISound.ButtonPressSound()->Play();
			m_NewFullscreen = m_FullscreenCheckbox->GetCheck();
			if (m_NewFullscreen) {
				m_NewResMultiplier = m_PresetResolutions.at(presetResListEntryID).Scale;
			} else {
				m_NewResMultiplier = 1.0f;
			}
			ApplyNewResolution();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsVideoGUI::ApplyCustomResolution() {
		m_CustomResolutionMessageLabel->SetVisible(false);
		m_NewFullscreen = m_FullscreenCheckbox->GetCheck();

		float newResMultiplier(.0f);
		int exitCode = sscanf(m_CustomResolutionMultiplierComboBox->GetText().c_str(), "%f%*s", &newResMultiplier);

		if (exitCode == EOF || newResMultiplier == 0) {
			m_CustomResolutionMessageLabel->SetText("Resolution Multiplier is not a valid number!");
			m_CustomResolutionMessageLabel->SetVisible(true);
			return;
		}

		m_NewResMultiplier = newResMultiplier;

		m_NewResX = g_WindowMan.GetResX();
		m_NewResY = g_WindowMan.GetResY();

		if (std::string newResXString = m_CustomResolutionWidthTextBox->GetText(); !newResXString.empty()) {
			m_NewResX = std::stoi(newResXString);
		} else {
			m_CustomResolutionWidthTextBox->SetText(std::to_string(m_NewResX));
		}

		if (std::string newResYString = m_CustomResolutionHeightTextBox->GetText(); !newResYString.empty()) {
			m_NewResY = std::stoi(newResYString);
		} else {
			m_CustomResolutionHeightTextBox->SetText(std::to_string(m_NewResY));
		}


		bool invalidResolution = false;
		if (m_NewResX < c_MinResX || m_NewResY < c_MinResY) {
			m_CustomResolutionMessageLabel->SetText("Resolution width or height lower than the minimum (" + std::to_string(c_MinResX) + "x" + std::to_string(c_MinResY) + ") is not supported.");
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
				if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Windowed]) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::Windowed);
				} else if (guiEvent.GetControl() == m_ResolutionQuickToggleButtons[ResolutionQuickChangeType::Fullscreen]) {
					ApplyQuickChangeResolution(ResolutionQuickChangeType::Fullscreen);
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
				if (m_CustomResolutionWidthTextBox->GetText().empty()) {
					m_CustomResolutionWidthTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResX())));
				}
				if (m_CustomResolutionHeightTextBox->GetText().empty()) {
					m_CustomResolutionHeightTextBox->SetText(std::to_string(static_cast<int>(g_WindowMan.GetResY())));
				}
				m_VideoSettingsBox->SetFocus();
			}

			if (guiEvent.GetControl() == m_PresetResolutionComboBox) {
				if (guiEvent.GetMsg() == GUIComboBox::Dropped) {
					m_PresetResolutionBox->Resize(m_PresetResolutionBox->GetWidth(), 165);
				} else if (guiEvent.GetMsg() == GUIComboBox::Closed) {
					m_PresetResolutionBox->Resize(m_PresetResolutionBox->GetWidth(), 80);
				}
			}

			if (guiEvent.GetControl() == m_CustomResolutionMultiplierComboBox) {
				if (guiEvent.GetMsg() == GUIComboBox::Dropped) {
					m_CustomResolutionBox->Resize(m_CustomResolutionBox->GetWidth(), 165);
				} else if (guiEvent.GetMsg() == GUIComboBox::Closed) {
					m_CustomResolutionBox->Resize(m_CustomResolutionBox->GetWidth(), 80);
				}
			}

			if (guiEvent.GetMsg() == GUICheckbox::Changed) {
				if (guiEvent.GetControl() == m_EnableVSyncCheckbox) {
					g_WindowMan.SetVSyncEnabled(m_EnableVSyncCheckbox->GetCheck());
				} else if (guiEvent.GetControl() == m_UseMultiDisplaysCheckbox) {
					g_WindowMan.SetUseMultiDisplays(m_UseMultiDisplaysCheckbox->GetCheck());
					UpdateCustomResolutionLimits();
					PopulateResolutionsComboBox();
					PopulateResMultplierComboBox();
				}
			} else if (guiEvent.GetMsg() == GUIRadioButton::Pushed) {
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
					CreateCustomResolutionBox();
					m_CustomResolutionBox->SetVisible(true);
				}
			}
		}
	}
} // namespace RTE