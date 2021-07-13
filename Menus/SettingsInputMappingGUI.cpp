#include "SettingsInputMappingGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUIScrollbar.h"
#include "GUILabel.h"

namespace RTE {

	std::array<InputElements, 7> SettingsInputMappingGUI::m_InputElementsUsedByMouse = { InputElements::INPUT_FIRE, InputElements::INPUT_PIEMENU, InputElements::INPUT_AIM, InputElements::INPUT_AIM_UP, InputElements::INPUT_AIM_DOWN, InputElements::INPUT_AIM_LEFT, InputElements::INPUT_AIM_RIGHT };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputMappingGUI::SettingsInputMappingGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_InputMappingSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPlayerInputMapping"));
		m_InputMappingSettingsBox->SetVisible(false);

		m_InputMappingSettingsLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPlayerInputMappingTitle"));
		m_CloseMappingBoxButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCloseMappingBox"));
		m_RunConfigWizardButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonRunConfigWizard"));

		m_InputMapScrollingBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxScrollingMappingBox"));
		m_InputMapScrollingBoxScrollbar = dynamic_cast<GUIScrollbar *>(m_GUIControlManager->GetControl("ScrollbarScrollingMappingBox"));
		m_InputMapScrollingBoxScrollbar->SetMaximum(m_InputMapScrollingBox->GetHeight());
		m_InputMapScrollingBoxScrollbar->SetPageSize(m_InputMapScrollingBoxScrollbar->GetMaximum() / 2);
		m_LastInputMapScrollingBoxScrollbarValue = m_InputMapScrollingBoxScrollbar->GetValue();

		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			m_InputMapLabel.at(i) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelInputName" + std::to_string(i + 1)));
			m_InputMapLabel.at(i)->SetText(c_InputElementNames.at(i));
			m_InputMapButton.at(i) = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonInputKey" + std::to_string(i + 1)));
		}
		m_InputMappingCaptureBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxInputCapture"));
		m_InputMappingCaptureBox->SetVisible(false);

		GUICollectionBox *settingsRootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxSettingsBase"));
		m_InputMappingCaptureBox->SetPositionAbs(settingsRootBox->GetXPos() + ((settingsRootBox->GetWidth() - m_InputMappingCaptureBox->GetWidth()) / 2), settingsRootBox->GetYPos() + ((settingsRootBox->GetHeight() - m_InputMappingCaptureBox->GetHeight()) / 2));

		m_InputElementCapturingInputNameLabel = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonLabelInputMappingName"));

		m_InputConfigWizardMenu = std::make_unique<SettingsInputMappingWizardGUI>(parentControlManager);

		m_ConfiguringPlayer = Players::NoPlayer;
		m_ConfiguringPlayerInputScheme = nullptr;
		m_ConfiguringManually = false;
		m_InputElementCapturingInput = InputElements::INPUT_COUNT;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsInputMappingGUI::IsEnabled() const {
		return m_InputMappingSettingsBox->GetVisible() && m_InputMappingSettingsBox->GetEnabled();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::SetEnabled(bool enable, int player) {
		m_InputMappingSettingsBox->SetVisible(enable);
		m_InputMappingSettingsBox->SetEnabled(enable);

		if (enable) {
			m_InputMappingSettingsLabel->SetText("P L A Y E R   " + std::to_string(player + 1) + "   I N P U T   M A P P I N G");
			m_ConfiguringPlayer = static_cast<Players>(player);
			m_ConfiguringPlayerInputScheme = g_UInputMan.GetControlScheme(player);

			m_InputMapScrollingBoxScrollbar->SetValue(0);
			UpdateScrollingInputBoxScrollPosition();
			UpdateMappingButtonLabels();
		} else {
			m_ConfiguringPlayer = Players::NoPlayer;
			m_ConfiguringPlayerInputScheme = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUICollectionBox * SettingsInputMappingGUI::GetActiveDialogBox() const {
		if (m_InputConfigWizardMenu->IsEnabled()) {
			return m_InputConfigWizardMenu->GetActiveDialogBox();
		} else if (m_InputMappingCaptureBox->GetEnabled() && m_InputMappingCaptureBox->GetVisible()) {
			return m_InputMappingCaptureBox;
		}
		return (m_InputMappingSettingsBox->GetEnabled() && m_InputMappingSettingsBox->GetVisible()) ? m_InputMappingSettingsBox : nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::CloseActiveDialogBox() {
		if (m_InputConfigWizardMenu->IsEnabled()) {
			m_InputConfigWizardMenu->SetEnabled(false);
		} else if (m_InputMappingCaptureBox->GetEnabled() && m_InputMappingCaptureBox->GetVisible()) {
			HideInputMappingCaptureBox();
		} else if (m_InputMappingSettingsBox->GetEnabled() && m_InputMappingSettingsBox->GetVisible()) {
			SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsInputMappingGUI::IsConfiguringManually() const {
		return m_ConfiguringManually && m_InputMappingCaptureBox->GetVisible() && m_InputMappingCaptureBox->GetEnabled();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::ShowInputMappingCaptureBox(InputElements inputElement) {
		m_InputMappingSettingsBox->SetEnabled(false);
		m_InputMappingCaptureBox->SetVisible(true);
		m_InputMappingCaptureBox->SetEnabled(true);
		m_InputElementCapturingInput = inputElement;
		m_InputElementCapturingInputNameLabel->SetText(m_InputMapLabel.at(inputElement)->GetText());
		m_ConfiguringManually = true;

		g_UInputMan.SetGUIInputInstanceToCaptureKeyStateFrom(m_GUIControlManager->GetManager()->GetInputController());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::HideInputMappingCaptureBox() {
		m_InputMappingSettingsBox->SetEnabled(true);
		m_InputMappingCaptureBox->SetVisible(false);
		m_InputMappingCaptureBox->SetEnabled(false);
		m_ConfiguringManually = false;

		g_UInputMan.SetGUIInputInstanceToCaptureKeyStateFrom(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::UpdateMappingButtonLabels() {
		const std::array<InputMapping, InputElements::INPUT_COUNT> *inputMappings = m_ConfiguringPlayerInputScheme->GetInputMappings();
		for (int i = 0; i < InputElements::INPUT_COUNT; ++i) {
			std::string inputDescription = inputMappings->at(i).GetPresetDescription();
			if (inputDescription.empty()) { inputDescription = m_ConfiguringPlayerInputScheme->GetMappingName(i); }
			m_InputMapButton.at(i)->SetText(!inputDescription.empty() ? "[" + inputDescription + "]" : "[Undefined]");
		}
		// Adjust the scrolling box scroll range to hide mappings that are only relevant to gamepads.
		m_InputMapScrollingBoxScrollbar->SetMaximum(m_InputMapScrollingBox->GetHeight() - ((m_ConfiguringPlayerInputScheme->GetDevice() < InputDevice::DEVICE_GAMEPAD_1) ? 141 : -8));
		m_InputMapScrollingBoxScrollbar->SetPageSize(m_InputMapScrollingBoxScrollbar->GetMaximum() / 2);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::UpdateScrollingInputBoxScrollPosition() {
		int scrollbarValue = m_InputMapScrollingBoxScrollbar->GetValue();
		m_InputMapScrollingBox->SetPositionRel(m_InputMapScrollingBox->GetRelXPos(), m_InputMapScrollingBox->GetRelYPos() + (m_LastInputMapScrollingBoxScrollbarValue - scrollbarValue));
		m_LastInputMapScrollingBoxScrollbarValue = scrollbarValue;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (m_InputConfigWizardMenu->IsEnabled()) {
			if (m_InputConfigWizardMenu->HandleInputEvents(guiEvent)) { UpdateMappingButtonLabels(); }
			return;
		}
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_CloseMappingBoxButton) {
				g_GUISound.ButtonPressSound()->Play();
				SetEnabled(false);
			} else if (guiEvent.GetControl() == m_RunConfigWizardButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_InputConfigWizardMenu->SetEnabled(true, m_ConfiguringPlayer, m_ConfiguringPlayerInputScheme);
			} else if (guiEvent.GetMsg() == GUIButton::Pushed) {
				for (int mapButton = 0; mapButton < InputElements::INPUT_COUNT; ++mapButton) {
					if (guiEvent.GetControl() == m_InputMapButton.at(mapButton)) {
						// Don't want to deal with special handling for mouse so don't allow remapping any mouse controls when using mouse+keyboard.
						// TODO: Add handling for input mapping with mouse directions and buttons (extra buttons too if applicable).
						if (m_ConfiguringPlayerInputScheme->GetDevice() == InputDevice::DEVICE_MOUSE_KEYB && std::find(m_InputElementsUsedByMouse.begin(), m_InputElementsUsedByMouse.end(), mapButton) != m_InputElementsUsedByMouse.end()) {
							g_GUISound.UserErrorSound()->Play();
						} else {
							g_GUISound.ButtonPressSound()->Play();
							ShowInputMappingCaptureBox(static_cast<InputElements>(mapButton));
						}
						// Remove focus so the mapping button doesn't remain stuck with the hovered state graphic after being pressed.
						m_InputMappingSettingsBox->SetFocus();
						break;
					}
				}
			}
		} else if (guiEvent.GetType() == GUIEvent::Notification && guiEvent.GetMsg() == GUIScrollbar::ChangeValue && guiEvent.GetControl() == m_InputMapScrollingBoxScrollbar) {
			UpdateScrollingInputBoxScrollPosition();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingGUI::HandleManualConfigSequence() {
		bool inputCaptured = false;
		if (g_UInputMan.KeyReleased(KEY_DEL)) {
			m_ConfiguringPlayerInputScheme->ClearMapping(m_InputElementCapturingInput);
			inputCaptured = true;
		}
		if (!inputCaptured) {
			InputDevice inputDevice = m_ConfiguringPlayerInputScheme->GetDevice();
			if (inputDevice == InputDevice::DEVICE_KEYB_ONLY || inputDevice == InputDevice::DEVICE_MOUSE_KEYB) {
				inputCaptured = m_ConfiguringPlayerInputScheme->CaptureKeyMapping(m_InputElementCapturingInput);
			} else {
				inputCaptured = m_ConfiguringPlayerInputScheme->CaptureJoystickMapping(g_UInputMan.GetJoystickIndex(inputDevice), m_InputElementCapturingInput);
			}
		}
		if (inputCaptured) {
			g_GUISound.ExitMenuSound()->Play();
			m_ConfiguringPlayerInputScheme->SetPreset(InputScheme::InputPreset::NoPreset);
			UpdateMappingButtonLabels();
			HideInputMappingCaptureBox();
		}
	}
}