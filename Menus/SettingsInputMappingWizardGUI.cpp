#include "SettingsInputMappingWizardGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::Clear() {
		m_ConfiguringPlayer = Players::NoPlayer;
		m_ConfiguringPlayerScheme = nullptr;

		m_ConfiguringDevice = InputDevice::DEVICE_KEYB_ONLY;
		m_ConfiguringDeviceIsGamepad = false;
		m_ConfiguringGamepadType = GamepadType::DPad;
		m_ConfiguringGamepadIndex = -1;

		m_ConfiguringManually = false;
		m_ConfigFinished = false;
		m_ConfigStep = 0;
		m_ConfigStepChange = false;

		m_NewInputScheme.Reset();
		m_NewInputSchemeApplied = false;

		m_GamepadRecommendedDiagramBlinkTimer.Reset();

		m_DPadDiagramBitmaps.clear();
		m_DualAnalogDSDiagramBitmaps.clear();
		m_DualAnalogXBDiagramBitmaps.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputMappingWizardGUI::SettingsInputMappingWizardGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		Clear();

		m_InputWizardScreenBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxInputMappingWizard"));
		m_InputWizardTitleLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelPlayerInputMappingWizardTitle"));

		int dpadDiagramBitampCount = 13;
		m_DPadDiagramBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Controllers/D-Pad.png").GetAsAnimation(dpadDiagramBitampCount, COLORCONV_8_TO_32), dpadDiagramBitampCount);

		int analogDiagramBitmapCount = 21;
		m_DualAnalogDSDiagramBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Controllers/DualAnalogDS.png").GetAsAnimation(analogDiagramBitmapCount, COLORCONV_8_TO_32), analogDiagramBitmapCount);
		m_DualAnalogXBDiagramBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Controllers/DualAnalogXB.png").GetAsAnimation(analogDiagramBitmapCount, COLORCONV_8_TO_32), analogDiagramBitmapCount);

		CreateManualConfigScreen();
		CreatePresetSelectionScreen();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::CreateManualConfigScreen() {
		m_WizardManualConfigScreen.ManualConfigBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxWizardManualConfig"));

		m_WizardManualConfigScreen.ConfigDeviceTypeLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigDeviceType"));
		m_WizardManualConfigScreen.ConfigStepDescriptionLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigInputDescription"));
		m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelRecommendedKeyInput"));
		m_WizardManualConfigScreen.GamepadConfigRecommendedBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxGamepadRecommendedInput"));
		m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelRecommendedGamepadInput"));
		m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxRecommendedGamepadInputDiagram"));

		m_WizardManualConfigScreen.ConfigStepLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigStep"));
		m_WizardManualConfigScreen.PrevConfigStepButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigPrevStep"));
		m_WizardManualConfigScreen.NextConfigStepButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigNextStep"));
		m_WizardManualConfigScreen.ResetConfigButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigRestart"));
		m_WizardManualConfigScreen.DiscardOrApplyConfigButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonDiscardOrApply"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::CreatePresetSelectionScreen() {
		m_WizardPresetSelectScreen.PresetSelectBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxWizardPresets"));
		m_WizardPresetSelectScreen.CloseWizardButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCloseWizardBox"));

		m_WizardPresetSelectScreen.PresetSelectSNESButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonPresetDPadSNES"));
		m_WizardPresetSelectScreen.PresetSelectDS4Button = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonPresetAnalogDS4"));
		m_WizardPresetSelectScreen.PresetSelectXB360Button = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonPresetAnalogXB360"));
		m_WizardPresetSelectScreen.StartConfigDPadTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDPadType"));
		m_WizardPresetSelectScreen.StartConfigAnalogDSTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigAnalogTypeDS"));
		m_WizardPresetSelectScreen.StartConfigAnalogXBTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigAnalogTypeXB"));

		// Ownership of the AllegroBitmaps is passed to the GUIControlManager.
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxConfigDPadTypeDiagram"))->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps.at(0)));
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPresetDPadSNESDiagram"))->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps.at(0)));
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxConfigAnalogTypeDSDiagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps.at(0)));
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPresetAnalogDS4Diagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps.at(0)));
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxConfigAnalogTypeXBDiagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps.at(0)));
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxPresetAnalogXB360Diagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps.at(0)));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsInputMappingWizardGUI::IsEnabled() const {
		return m_InputWizardScreenBox->GetVisible() && m_InputWizardScreenBox->GetEnabled();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::SetEnabled(bool enable, int player, InputScheme *playerScheme) {
		if (enable) {
			m_InputWizardTitleLabel->SetText("P L A Y E R   " + std::to_string(player + 1) + "   I N P U T   C O N F I G U R A T I O N");
			m_InputWizardScreenBox->SetVisible(true);
			m_InputWizardScreenBox->SetEnabled(true);

			m_ConfiguringPlayer = static_cast<Players>(player);
			m_ConfiguringPlayerScheme = playerScheme;
			m_ConfiguringDevice = m_ConfiguringPlayerScheme->GetDevice();

			if (m_ConfiguringDevice == InputDevice::DEVICE_KEYB_ONLY || m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
				m_ConfiguringDeviceIsGamepad = false;
				m_ConfiguringGamepadIndex = -1;
				ShowManualConfigScreen();
			} else {
				m_ConfiguringDeviceIsGamepad = true;
				m_ConfiguringGamepadIndex = g_UInputMan.GetJoystickIndex(m_ConfiguringDevice);
				ShowPresetSelectionScreen();
			}
		} else {
			m_InputWizardScreenBox->SetVisible(false);
			m_InputWizardScreenBox->SetEnabled(false);
			m_ConfiguringPlayer = Players::NoPlayer;
			m_ConfiguringPlayerScheme = nullptr;
			m_ConfigFinished = false;
			m_NewInputSchemeApplied = false;

			g_UInputMan.SetInputClass(nullptr);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUICollectionBox * SettingsInputMappingWizardGUI::GetActiveDialogBox() const {
		return (m_InputWizardScreenBox->GetEnabled() && m_InputWizardScreenBox->GetVisible()) ? m_InputWizardScreenBox : nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::ShowManualConfigScreen() {
		m_WizardPresetSelectScreen.PresetSelectBox->SetVisible(false);
		m_WizardPresetSelectScreen.PresetSelectBox->SetEnabled(false);

		m_WizardManualConfigScreen.ManualConfigBox->SetVisible(true);
		m_WizardManualConfigScreen.ManualConfigBox->SetEnabled(true);
		m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetVisible(!m_ConfiguringDeviceIsGamepad);
		m_WizardManualConfigScreen.GamepadConfigRecommendedBox->SetVisible(m_ConfiguringDeviceIsGamepad);

		std::string inputDeviceName;
		if (m_ConfiguringDeviceIsGamepad) {
			switch (m_ConfiguringGamepadType) {
				case SettingsInputMappingWizardGUI::DPad:
					inputDeviceName = "Gamepad (Classic D-Pad Style)";
					m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DPadDiagramBitmaps.at(0)->w, m_DPadDiagramBitmaps.at(0)->h);
					break;
				case SettingsInputMappingWizardGUI::AnalogDualShock:
					inputDeviceName = "Dual Analog Gamepad (DualShock Style)";
					m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DualAnalogDSDiagramBitmaps.at(0)->w, m_DualAnalogDSDiagramBitmaps.at(0)->h);
					break;
				case SettingsInputMappingWizardGUI::AnalogXbox:
					inputDeviceName = "Dual Analog Gamepad (Xbox Style)";
					m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DualAnalogXBDiagramBitmaps.at(0)->w, m_DualAnalogXBDiagramBitmaps.at(0)->h);
					break;
				default:
					break;
			}
			m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->CenterInParent(true, true);
		} else {
			if (m_ConfiguringDevice == InputDevice::DEVICE_KEYB_ONLY) {
				inputDeviceName = "Keyboard Only";
			} else if (m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
				inputDeviceName = "Mouse + Keyboard";
			}
		}
		m_WizardManualConfigScreen.ConfigDeviceTypeLabel->SetText(inputDeviceName);

		m_ConfiguringManually = true;
		m_ConfigFinished = false;

		m_ConfigStep = 0;
		m_ConfigStepChange = true;

		m_NewInputScheme.Reset();
		m_NewInputScheme.SetDevice(m_ConfiguringDevice);

		// Use GUIInput class for better key detection
		g_UInputMan.SetInputClass(m_GUIControlManager->GetManager()->GetInputController());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::ShowPresetSelectionScreen() {
		m_WizardManualConfigScreen.ManualConfigBox->SetVisible(false);
		m_WizardManualConfigScreen.ManualConfigBox->SetEnabled(false);

		m_WizardPresetSelectScreen.PresetSelectBox->SetVisible(true);
		m_WizardPresetSelectScreen.PresetSelectBox->SetEnabled(true);

		m_ConfiguringManually = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::ApplyGamepadInputPreset(GamepadType gamepadType) {
		switch (gamepadType) {
			case GamepadType::DPad:
				// TODO: Doesn't have an actual preset at the moment so don't apply any.
				//m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadSNES);
				break;
			case GamepadType::AnalogDualShock:
				// TODO: Doesn't have an actual preset at the moment so don't apply any.
				//m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadDS4);
				break;
			case GamepadType::AnalogXbox:
				m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadXbox360);
				SetEnabled(false);
				break;
			default:
				RTEAbort("Invalid GamepadType passed to SettingsInputMappingWizardGUI::ApplyGamepadInputPreset!");
				break;
		}
		g_GUISound.ButtonPressSound()->Play();
		m_NewInputSchemeApplied = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::ApplyManuallyConfiguredScheme() {
		m_ConfiguringPlayerScheme->SetDevice(m_NewInputScheme.GetDevice());
		m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::NoPreset);

		if (m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
			m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_FIRE).SetMouseButton(MouseButtons::MOUSE_LEFT);
			m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_PIEMENU).SetMouseButton(MouseButtons::MOUSE_RIGHT);
		}
		std::swap(*m_ConfiguringPlayerScheme->GetInputMappings(), *m_NewInputScheme.GetInputMappings());

		m_NewInputSchemeApplied = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsInputMappingWizardGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (m_WizardManualConfigScreen.ManualConfigBox->GetVisible()) {
			HandleManualConfigScreenInputEvents(guiEvent);
		} else if (m_WizardPresetSelectScreen.PresetSelectBox->GetVisible()) {
			HandlePresetSelectScreenInputEvents(guiEvent);
		}
		if (m_NewInputSchemeApplied) {
			SetEnabled(false);
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::HandleManualConfigScreenInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_WizardManualConfigScreen.PrevConfigStepButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfigStep--;
				m_ConfigStepChange = true;
			} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.NextConfigStepButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfigStep++;
				m_ConfigStepChange = true;
			} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.ResetConfigButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfigStep = 0;
				m_ConfigFinished = false;
				m_ConfigStepChange = true;
			} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.DiscardOrApplyConfigButton) {
				g_GUISound.ButtonPressSound()->Play();
				if (m_ConfigFinished) {
					ApplyManuallyConfiguredScheme();
				} else {
					if (m_ConfiguringDeviceIsGamepad) {
						ShowPresetSelectionScreen();
					} else {
						SetEnabled(false);
					}
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::HandlePresetSelectScreenInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == m_WizardPresetSelectScreen.CloseWizardButton) {
				g_GUISound.ButtonPressSound()->Play();
				SetEnabled(false);
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigDPadTypeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfiguringGamepadType = GamepadType::DPad;
				ShowManualConfigScreen();
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigAnalogDSTypeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfiguringGamepadType = GamepadType::AnalogDualShock;
				ShowManualConfigScreen();
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigAnalogXBTypeButton) {
				g_GUISound.ButtonPressSound()->Play();
				m_ConfiguringGamepadType = GamepadType::AnalogXbox;
				ShowManualConfigScreen();
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectSNESButton) {
				ApplyGamepadInputPreset(GamepadType::DPad);
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectDS4Button) {
				ApplyGamepadInputPreset(GamepadType::AnalogDualShock);
			} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectXB360Button) {
				ApplyGamepadInputPreset(GamepadType::AnalogXbox);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::HandleManualConfigSequence() {
		if (m_ConfigStepChange) {
			ConfigWizardSteps configuringDeviceSteps = ConfigWizardSteps::NoConfigSteps;

			switch (m_ConfiguringDevice) {
				case InputDevice::DEVICE_KEYB_ONLY:
					configuringDeviceSteps = ConfigWizardSteps::KeyboardConfigSteps;
					break;
				case InputDevice::DEVICE_MOUSE_KEYB:
					configuringDeviceSteps = ConfigWizardSteps::MouseAndKeyboardConfigSteps;
					break;
				default:
					configuringDeviceSteps = (m_ConfiguringGamepadType == GamepadType::DPad) ? ConfigWizardSteps::DPadConfigSteps : ConfigWizardSteps::DualAnalogConfigSteps;
					break;
			}
			if (!m_ConfigFinished) {
				std::string configStepLabel(16, '\0');
				std::snprintf(configStepLabel.data(), configStepLabel.size(), "Step %i of %i", m_ConfigStep + 1, configuringDeviceSteps);
				m_WizardManualConfigScreen.ConfigStepLabel->SetText(configStepLabel);
				m_WizardManualConfigScreen.DiscardOrApplyConfigButton->SetText("Discard Changes");
			} else {
				m_WizardManualConfigScreen.ConfigStepLabel->SetText("Config Complete");
				m_WizardManualConfigScreen.DiscardOrApplyConfigButton->SetText("Apply Changes");
				if (m_ConfigStep < configuringDeviceSteps - 1) { m_ConfigFinished = false; }
			}
			if (m_ConfigStep == 0) {
				m_WizardManualConfigScreen.PrevConfigStepButton->SetVisible(false);
				m_WizardManualConfigScreen.NextConfigStepButton->SetVisible(true);
			} else if (m_ConfigStep >= configuringDeviceSteps - 1) {
				m_WizardManualConfigScreen.PrevConfigStepButton->SetVisible(true);
				m_WizardManualConfigScreen.NextConfigStepButton->SetVisible(false);
			} else {
				m_WizardManualConfigScreen.PrevConfigStepButton->SetVisible(true);
				m_WizardManualConfigScreen.NextConfigStepButton->SetVisible(true);
			}
		}
		switch (m_ConfiguringDevice) {
			case InputDevice::DEVICE_KEYB_ONLY:
				UpdateKeyboardConfigSequence();
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				UpdateMouseAndKeyboardConfigSequence();
				break;
			default:
				int diagramBitmapIndex = 0;
				if (!m_ConfigFinished) { diagramBitmapIndex = m_GamepadRecommendedDiagramBlinkTimer.AlternateReal(500) ? m_ConfigStep + 1 : 0; }
				if (m_ConfiguringGamepadType == GamepadType::DPad) {
					m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps.at(diagramBitmapIndex)));
					UpdateGamepadDPadConfigSequence();
				} else {
					if (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) {
						m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps.at(diagramBitmapIndex)));
					} else {
						m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps.at(diagramBitmapIndex)));
					}
					UpdateGamepadAnalogConfigSequence();
				}
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::UpdateKeyboardConfigSequence() {
		bool inputCaptured = false;
		switch (m_ConfigStep) {
			case 0:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE or AIM UP");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[W] or [Up Arrow]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM_UP)) {
					m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_UP);
					inputCaptured = true;
				}
				break;
			case 1:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE or AIM DOWN");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[S] or [Down Arrow]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM_DOWN)) {
					m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_DOWN);
					inputCaptured = true;
				}
				break;
			case 2:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE LEFT");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[A] or [Left Arrow]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_LEFT)) { inputCaptured = true; }
				break;
			case 3:
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE RIGHT");
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[D] or [Right Arrow]");
				m_ConfigStepChange = false;
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_RIGHT)) { inputCaptured = true; }
				break;
			case 4:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("FIRE or ACTIVATE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[H] or [Num 1]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_FIRE)) { inputCaptured = true; }
				break;
			case 5:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("SHARP AIM");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[J] or [Num 2]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM)) { inputCaptured = true; }
				break;
			case 6:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PIE MENU");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[K] or [Num 3]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PIEMENU)) { inputCaptured = true; }
				break;
			case 7:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("JUMP");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[L] or [Num Enter]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_JUMP)) { inputCaptured = true; }
				break;
			case 8:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("CROUCH");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[C] or [Num 0]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_CROUCH)) { inputCaptured = true; }
				break;
			case 9:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS BODY");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[U] or [Num 4]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PREV)) { inputCaptured = true; }
				break;
			case 10:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT BODY");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Y] or [Num 5]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_NEXT)) { inputCaptured = true; }
				break;
			case 11:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("RELOAD WEAPON");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[R] or [Num Del.]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_RELOAD)) { inputCaptured = true; }
				break;
			case 12:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PICK UP DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[F] or [Num 6]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_PICKUP)) { inputCaptured = true; }
				break;
			case 13:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("DROP DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[B] or [Num 9]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_DROP)) { inputCaptured = true; }
				break;
			case 14:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Q] or [Num 7]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_PREV)) { inputCaptured = true; }
				break;
			case 15:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[E] or [Num 8]");
					m_ConfigStepChange = false;
				}
				if (!m_ConfigFinished && m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_NEXT)) { inputCaptured = true; }
				if (inputCaptured) { m_ConfigFinished = true; }
				break;
			default:
				break;
		}
		if (inputCaptured) {
			if (!m_ConfigFinished) { m_ConfigStep++; }
			g_GUISound.ExitMenuSound()->Play();
			m_ConfigStepChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::UpdateMouseAndKeyboardConfigSequence() {
		bool inputCaptured = false;
		switch (m_ConfigStep) {
			case 0:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE UP or JUMP");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[W]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_UP)) {
					m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_JUMP);
					inputCaptured = true;
				}
				break;
			case 1:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE DOWN or CROUCH");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[S]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_DOWN)) {
					m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_CROUCH);
					inputCaptured = true;
				}
				break;
			case 2:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE LEFT");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[A]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_LEFT)) { inputCaptured = true; }
				break;
			case 3:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE RIGHT");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[D]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_RIGHT)) { inputCaptured = true; }
				break;
			case 4:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS BODY");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Q]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PREV)) { inputCaptured = true; }
				break;
			case 5:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT BODY");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[E]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_NEXT)) { inputCaptured = true; }
				break;
			case 6:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("RELOAD WEAPON");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[R]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_RELOAD)) { inputCaptured = true; }
				break;
			case 7:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PICK UP DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[F]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_PICKUP)) { inputCaptured = true; }
				break;
			case 8:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("DROP DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[G]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_DROP)) { inputCaptured = true; }
				break;
			case 9:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[1]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_PREV)) { inputCaptured = true; }
				break;
			case 10:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT DEVICE");
					m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[2]");
					m_ConfigStepChange = false;
				}
				if (!m_ConfigFinished && m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_NEXT)) { inputCaptured = true; }
				if (inputCaptured) { m_ConfigFinished = true; }
				break;
			default:
				break;
		}
		if (inputCaptured) {
			if (!m_ConfigFinished) { m_ConfigStep++; }
			g_GUISound.ExitMenuSound()->Play();
			m_ConfigStepChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::UpdateGamepadDPadConfigSequence() {
		bool inputCaptured = false;
		switch (m_ConfigStep) {
			case 0:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE or AIM UP");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Up]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM_UP)) {
					m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_UP);
					inputCaptured = true;
				}
				break;
			case 1:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE or AIM DOWN");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Down]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM_DOWN)) {
					m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_DOWN);
					inputCaptured = true;
				}
				break;
			case 2:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE LEFT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Left]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_LEFT)) { inputCaptured = true; }
				break;
			case 3:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE RIGHT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Right]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_RIGHT)) { inputCaptured = true; }
				break;
			case 4:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("FIRE / ACTIVATE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[B Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_FIRE)) { inputCaptured = true; }
				break;
			case 5:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("SHARP AIM");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Y Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM)) { inputCaptured = true; }
				break;
			case 6:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("COMMAND MENU");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[X Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PIEMENU)) { inputCaptured = true; }
				break;
			case 7:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("JUMP");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[A Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_JUMP)) { inputCaptured = true; }
				break;
			case 8:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT BODY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Bumper Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_NEXT)) { inputCaptured = true; }
				break;
			case 9:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS BODY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Bumper Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PREV)) { inputCaptured = true; }
				break;
			case 10:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("START / PAUSE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Start Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_START)) { inputCaptured = true; }
				break;
			case 11:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("RESET ACTIVITY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Select Button]");
					m_ConfigStepChange = false;
				}
				if (!m_ConfigFinished && m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_BACK)) { inputCaptured = true; }
				if (inputCaptured) { m_ConfigFinished = true; }
				break;
			default:
				break;
		}
		if (inputCaptured) {
			if (!m_ConfigFinished) { m_ConfigStep++; }
			g_GUISound.ExitMenuSound()->Play();
			m_ConfigStepChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::UpdateGamepadAnalogConfigSequence() {
		bool inputCaptured = false;
		switch (m_ConfigStep) {
			case 0:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE UP or JUMP");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Up]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_UP)) {
					m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_JUMP);
					inputCaptured = true;
				}
				break;
			case 1:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE DOWN or CROUCH");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Down]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_DOWN)) {
					m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_CROUCH);
					inputCaptured = true;
				}
				break;
			case 2:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE LEFT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Left]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_LEFT)) { inputCaptured = true; }
				break;
			case 3:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("MOVE RIGHT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Right]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_RIGHT)) { inputCaptured = true; }
				break;
			case 4:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("AIM UP");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Up]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_UP)) { inputCaptured = true; }
				break;
			case 5:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("AIM DOWN");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Down]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_DOWN)) { inputCaptured = true; }
				break;
			case 6:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("AIM LEFT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Left]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_LEFT)) { inputCaptured = true; }
				break;
			case 7:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("AIM RIGHT");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Right]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_RIGHT)) { inputCaptured = true; }
				break;
			case 8:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("FIRE / ACTIVATE");
					std::string buttonName = (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[X Button]" : "[A Button]";
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(buttonName + " or [R. Trigger]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_FIRE)) { inputCaptured = true; }
				break;
			case 9:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("SHARP AIM");
					std::string buttonName = (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Square Button]" : "[X Button]";
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(buttonName + " or [L. Trigger]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM)) { inputCaptured = true; }
				break;
			case 10:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("COMMAND MENU");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(m_ConfiguringGamepadType == GamepadType::AnalogDualShock ? "[Circle Button]" : "[B Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PIEMENU)) { inputCaptured = true; }
				break;
			case 11:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT BODY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Bumper Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_NEXT)) { inputCaptured = true; }
				break;
			case 12:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS BODY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Bumper Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PREV)) { inputCaptured = true; }
				break;
			case 13:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("RELOAD WEAPON");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(m_ConfiguringGamepadType == GamepadType::AnalogDualShock ? "[Triangle Button]" : "[Y Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_RELOAD)) { inputCaptured = true; }
				break;
			case 14:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PICK UP DEVICE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Up]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_PICKUP)) { inputCaptured = true; }
				break;
			case 15:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("DROP DEVICE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Down]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_DROP)) { inputCaptured = true; }
				break;
			case 16:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("NEXT DEVICE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Right]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_CHANGE_NEXT)) { inputCaptured = true; }
				break;
			case 17:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("PREVIOUS DEVICE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Left]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_CHANGE_PREV)) { inputCaptured = true; }
				break;
			case 18:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("START / PAUSE");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(m_ConfiguringGamepadType == GamepadType::AnalogDualShock ? "[Options Button]" : "[Start Button]");
					m_ConfigStepChange = false;
				}
				if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_START)) { inputCaptured = true; }
				break;
			case 19:
				if (m_ConfigStepChange) {
					m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("RESET ACTIVITY");
					m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(m_ConfiguringGamepadType == GamepadType::AnalogDualShock ? "[Share Button]" : "[Back Button]");
					m_ConfigStepChange = false;
				}
				if (!m_ConfigFinished && m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_BACK)) { inputCaptured = true; }
				if (inputCaptured) { m_ConfigFinished = true; }
				break;
			default:
				break;
		}
		if (inputCaptured) {
			if (!m_ConfigFinished) { m_ConfigStep++; }
			g_GUISound.ExitMenuSound()->Play();
			m_ConfigStepChange = true;
		}
	}
}