#include "SettingsInputGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUISlider.h"
#include "GUIRadioButton.h"
#include "GUILabel.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputGUI::SettingsInputGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_InputSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxInputSettings"));

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player + 1);

			m_PlayerInputSettingsBoxes.at(player).SelectedDeviceLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "SelectedDevice"));

			m_PlayerInputSettingsBoxes.at(player).NextDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "NextDevice"));
			m_PlayerInputSettingsBoxes.at(player).PrevDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "PrevDevice"));
			m_PlayerInputSettingsBoxes.at(player).ConfigureControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Config"));
			m_PlayerInputSettingsBoxes.at(player).ResetControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Clear"));

			m_PlayerInputSettingsBoxes.at(player).SensitivtyLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "Sensitivity"));
			m_PlayerInputSettingsBoxes.at(player).SensitivitySlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderP" + playerNum + "Sensitivity"));

			m_PlayerInputSettingsBoxes.at(player).DeadZoneControlsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxP" + playerNum + "DeadzoneControls"));
			m_PlayerInputSettingsBoxes.at(player).CircleDeadZoneRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioP" + playerNum + "DeadzoneCircle"));
			m_PlayerInputSettingsBoxes.at(player).SquareDeadZoneRadioButton = dynamic_cast<GUIRadioButton *>(m_GUIControlManager->GetControl("RadioP" + playerNum + "DeadzoneSquare"));
		}
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			UpdatePlayerSelectedDeviceLabel(player);
			ShowPlayerSensitivityControls(player);
		}
		m_InputConfigMenu.Create(parentControlManager);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::InputConfigScreen::Create(GUIControlManager *parentControlManager) {
		MappingConfigBox = dynamic_cast<GUICollectionBox *>(parentControlManager->GetControl("CollectionBoxPlayerInputMapping"));
		MappingConfigBox->SetVisible(false);

		MappingConfigLabel = dynamic_cast<GUILabel *>(parentControlManager->GetControl("LabelPlayerInputMappingTitle"));
		CloseMappingBoxButton = dynamic_cast<GUIButton *>(parentControlManager->GetControl("ButtonCloseMappingBox"));
		RunConfigWizardButton = dynamic_cast<GUIButton *>(parentControlManager->GetControl("ButtonRunConfigWizard"));

		for (int i = 0; i < 18; ++i) {
			InputMapLabel.at(i) = dynamic_cast<GUILabel *>(parentControlManager->GetControl("LabelMapping" + std::to_string(i + 1)));
			InputMapButton.at(i) = dynamic_cast<GUIButton *>(parentControlManager->GetControl("ButtonMapping" + std::to_string(i + 1)));
		}
	}

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::SetEnabled(bool enable) const {
		if (enable) {
			m_InputSettingsBox->SetVisible(true);
			m_InputSettingsBox->SetEnabled(true);
		} else {
			m_InputSettingsBox->SetVisible(false);
			m_InputSettingsBox->SetEnabled(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::InputConfigScreen::SetEnabled(bool enable, int player) {
		if (enable) {
			MappingConfigBox->SetVisible(true);
			MappingConfigBox->SetEnabled(true);
			MappingConfigLabel->SetText("P L A Y E R   " + std::to_string(player + 1) + "   I N P U T   M A P P I N G");
			UpdateMappingLabelsAndButtons(player);
		} else {
			MappingConfigBox->SetVisible(false);
			MappingConfigBox->SetEnabled(false);
		}
		Enabled = enable;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::InputConfigScreen::UpdateMappingLabelsAndButtons(int player) {
		//const InputMapping *inputMappings = g_UInputMan.GetControlScheme(player)->GetInputMappings();

		for (int i = 0; i < PlayerInputMappings::InputMappingCount; ++i) {
			InputMapLabel.at(i)->SetText("- Map Name");
			InputMapButton.at(i)->SetText("[Button]");
		}

		switch (g_UInputMan.GetControlScheme(player)->GetDevice()) {
			case InputDevice::DEVICE_KEYB_ONLY:
				InputMapLabel.at(0)->SetText("- Move/Aim Up");
				InputMapLabel.at(1)->SetText("- Move/Aim Down");
				InputMapLabel.at(2)->SetText("- Move Left");
				InputMapLabel.at(3)->SetText("- Move Right");
				InputMapLabel.at(4)->SetText("- Fire/Activate");
				InputMapLabel.at(5)->SetText("- Sharp Aim");
				InputMapLabel.at(6)->SetText("- Command Menu");
				InputMapLabel.at(7)->SetText("- Jump");
				InputMapLabel.at(8)->SetText("- Crouch");
				InputMapLabel.at(9)->SetText("- Prev. Body");
				InputMapLabel.at(10)->SetText("- Next Body");
				InputMapLabel.at(11)->SetText("- Reload");
				InputMapLabel.at(12)->SetText("- Pick Up");
				InputMapLabel.at(13)->SetText("- Drop Device");
				InputMapLabel.at(14)->SetText("- Prev. Device");
				InputMapLabel.at(15)->SetText("- Next. Device");
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				InputMapLabel.at(0)->SetText("- Move Up/Jump");
				InputMapLabel.at(1)->SetText("- Move Down/Crouch");
				InputMapLabel.at(2)->SetText("- Move Left");
				InputMapLabel.at(3)->SetText("- Move Right");
				InputMapLabel.at(4)->SetText("- Reload");
				InputMapLabel.at(5)->SetText("- Pick Up");
				InputMapLabel.at(6)->SetText("- Drop Device");
				InputMapLabel.at(7)->SetText("- Prev. Device");
				InputMapLabel.at(8)->SetText("- Next. Device");
				InputMapLabel.at(9)->SetText("- Prev. Body");
				InputMapLabel.at(10)->SetText("- Next Body");
				break;
			default:
				/*
				// D-Pad
				InputMapLabel.at(0)->SetText("- Move/Aim Up");
				InputMapLabel.at(1)->SetText("- Move/Aim Down");
				InputMapLabel.at(2)->SetText("- Move Left");
				InputMapLabel.at(3)->SetText("- Move Right");
				InputMapLabel.at(4)->SetText("- Sharp Aim");
				InputMapLabel.at(5)->SetText("- Fire/Activate");
				InputMapLabel.at(6)->SetText("- Jump");
				InputMapLabel.at(7)->SetText("- Command Menu");
				InputMapLabel.at(8)->SetText("- Next Body");
				InputMapLabel.at(9)->SetText("- Prev. Body");
				InputMapLabel.at(10)->SetText("- Start");
				InputMapLabel.at(11)->SetText("- Back");
				*/
				/*
				// Dual Analog
				InputMapLabel.at(0)->SetText("- Move Up/Jump");
				InputMapLabel.at(1)->SetText("- Move Down/Crouch");
				InputMapLabel.at(2)->SetText("- Move Left");
				InputMapLabel.at(3)->SetText("- Move Right");
				InputMapLabel.at(4)->SetText("- Aim Up");
				InputMapLabel.at(5)->SetText("- Aim Down");
				InputMapLabel.at(6)->SetText("- Aim Left");
				InputMapLabel.at(7)->SetText("- Aim Right");
				InputMapLabel.at(8)->SetText("- Fire/Activate");
				InputMapLabel.at(9)->SetText("- Command Menu");
				InputMapLabel.at(10)->SetText("- Next Body");
				InputMapLabel.at(11)->SetText("- Prev. Body");
				InputMapLabel.at(12)->SetText("- Prev. Device");
				InputMapLabel.at(13)->SetText("- Next. Device");
				InputMapLabel.at(14)->SetText("- Pick Up");
				InputMapLabel.at(15)->SetText("- Reload");
				InputMapLabel.at(16)->SetText("- Start");
				InputMapLabel.at(17)->SetText("- Back");
				*/
				break;
		}
		for (int i = 0; i < PlayerInputMappings::InputMappingCount; ++i) {
			if (InputMapLabel.at(i)->GetText() == "- Map Name") {
				InputMapLabel.at(i)->SetVisible(false);
				InputMapButton.at(i)->SetVisible(false);
			} else {
				InputMapLabel.at(i)->SetVisible(true);
				InputMapButton.at(i)->SetVisible(true);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::SetPlayerNextOrPrevInputDevice(int player, bool nextDevice) {
		int currentDevice = static_cast<int>(g_UInputMan.GetControlScheme(player)->GetDevice());

		if (nextDevice) {
			currentDevice++;
			if (currentDevice >= InputDevice::DEVICE_COUNT) { currentDevice = InputDevice::DEVICE_KEYB_ONLY; }
		} else {
			currentDevice--;
			if (currentDevice < InputDevice::DEVICE_KEYB_ONLY) { currentDevice = InputDevice::DEVICE_GAMEPAD_4; }
		}
		g_UInputMan.GetControlScheme(player)->SetDevice(static_cast<InputDevice>(currentDevice));
		UpdatePlayerSelectedDeviceLabel(player);
		ShowPlayerSensitivityControls(player);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::UpdatePlayerSelectedDeviceLabel(int player) {
		std::string deviceLabel;

		switch (g_UInputMan.GetControlScheme(player)->GetDevice()) {
			case InputDevice::DEVICE_KEYB_ONLY:
				deviceLabel = "Keyboard Only";
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				deviceLabel = "Mouse + Keyboard";
				break;
			case InputDevice::DEVICE_GAMEPAD_1:
				deviceLabel = "Gamepad 1";
				break;
			case InputDevice::DEVICE_GAMEPAD_2:
				deviceLabel = "Gamepad 2";
				break;
			case InputDevice::DEVICE_GAMEPAD_3:
				deviceLabel = "Gamepad 3";
				break;
			case InputDevice::DEVICE_GAMEPAD_4:
				deviceLabel = "Gamepad 4";
				break;
			default:
				break;
		}
		m_PlayerInputSettingsBoxes.at(player).SelectedDeviceLabel->SetText(deviceLabel);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::ShowPlayerSensitivityControls(int player) {
		m_PlayerInputSettingsBoxes.at(player).SensitivtyLabel->SetVisible(false);
		m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetVisible(false);
		m_PlayerInputSettingsBoxes.at(player).DeadZoneControlsBox->SetVisible(false);

		switch (g_UInputMan.GetControlScheme(player)->GetDevice()) {
			case InputDevice::DEVICE_KEYB_ONLY:
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				// Mouse sensitivity doesn't seem to really work so keep the controls disabled for now, also it's shared between all mouse+keyboard using players.
				/*
				m_PlayerInputSettingsBoxes.at(player).SensitivtyLabel->SetVisible(true);
				m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetVisible(true);
				m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetMaximum(100);
				*/
				break;
			default:
				m_PlayerInputSettingsBoxes.at(player).SensitivtyLabel->SetVisible(true);
				m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetVisible(true);
				m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetMaximum(50);
				m_PlayerInputSettingsBoxes.at(player).DeadZoneControlsBox->SetVisible(true);
				break;
		}
		UpdatePlayerSensitivityValues(player);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::UpdatePlayerSensitivityValues(int player) {
		switch (g_UInputMan.GetControlScheme(player)->GetDevice()) {
			case InputDevice::DEVICE_KEYB_ONLY:
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				// Mouse sensitivity is shared between all players
				for (int otherPlayer = Players::PlayerOne; otherPlayer < Players::MaxPlayerCount; ++otherPlayer) {
					if (g_UInputMan.GetControlScheme(otherPlayer)->GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
						m_PlayerInputSettingsBoxes.at(otherPlayer).SensitivitySlider->SetValue(static_cast<int>(g_UInputMan.GetMouseSensitivity() * 100));
						m_PlayerInputSettingsBoxes.at(otherPlayer).SensitivtyLabel->SetText("Mouse Sensitivity: " + std::to_string(m_PlayerInputSettingsBoxes.at(otherPlayer).SensitivitySlider->GetValue()));
					}
				}
				break;
			default:
				m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->SetValue(static_cast<int>(g_UInputMan.GetControlScheme(player)->GetJoystickDeadzone() * 250));
				m_PlayerInputSettingsBoxes.at(player).SensitivtyLabel->SetText("Stick Deadzone: " + std::to_string(m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->GetValue()));

				if (g_UInputMan.GetControlScheme(player)->GetJoystickDeadzoneType() == DeadZoneType::CIRCLE) {
					m_PlayerInputSettingsBoxes.at(player).CircleDeadZoneRadioButton->SetCheck(true);
				} else {
					m_PlayerInputSettingsBoxes.at(player).SquareDeadZoneRadioButton->SetCheck(true);
				}
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::ResetPlayerControlMappings(int player) {
		if (m_PlayerInputSettingsBoxes.at(player).ResetControlsButton->GetText() == "Reset") {
			// Reset any other pending mapping reset confirmations
			for (int otherPlayer = Players::PlayerOne; otherPlayer < Players::MaxPlayerCount; ++otherPlayer) {
				if (otherPlayer != player) { m_PlayerInputSettingsBoxes.at(otherPlayer).ResetControlsButton->SetText("Reset"); }
			}
			m_PlayerInputSettingsBoxes.at(player).ResetControlsButton->SetText("CONFIRM?");
		} else {
			InputScheme *playerControlScheme = g_UInputMan.GetControlScheme(player);
			playerControlScheme->SetDevice(static_cast<InputDevice>(player));
			playerControlScheme->SetPreset(static_cast<InputPreset>(-(player + 1))); // Player 1's default preset is at -1 and so on.

			if (playerControlScheme->GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				g_UInputMan.SetMouseSensitivity(0.6F);
			} else if (playerControlScheme->GetDevice() != InputDevice::DEVICE_MOUSE_KEYB && playerControlScheme->GetDevice() != InputDevice::DEVICE_KEYB_ONLY) {
				playerControlScheme->SetJoystickDeadzone(0);
				playerControlScheme->SetJoystickDeadzoneType(DeadZoneType::CIRCLE);
			}
			UpdatePlayerSelectedDeviceLabel(player);
			ShowPlayerSensitivityControls(player);

			m_PlayerInputSettingsBoxes.at(player).ResetControlsButton->SetText("Reset");
			g_GUISound.ExitMenuSound()->Play();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::HandleInputEvents(GUIEvent &guiEvent) {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			if (m_InputConfigMenu.Enabled) {
				m_InputConfigMenu.HandleInputEvents(guiEvent, player);
				return;
			}
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).NextDeviceButton) {
					g_GUISound.ButtonPressSound()->Play();
					SetPlayerNextOrPrevInputDevice(player, true);
				} else if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).PrevDeviceButton) {
					g_GUISound.ButtonPressSound()->Play();
					SetPlayerNextOrPrevInputDevice(player, false);
				} else if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).ConfigureControlsButton) {
					g_GUISound.ButtonPressSound()->Play();
					m_InputConfigMenu.SetEnabled(true, player);
				} else if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).ResetControlsButton) {
					g_GUISound.ButtonPressSound()->Play();
					ResetPlayerControlMappings(player);
				}
			} else if (guiEvent.GetType() == GUIEvent::Notification) {
				if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).SensitivitySlider) {
					if (g_UInputMan.GetControlScheme(player)->GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
						g_UInputMan.SetMouseSensitivity(static_cast<float>(m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->GetValue()) / 100.0F);
					} else {
						g_UInputMan.GetControlScheme(player)->SetJoystickDeadzone(static_cast<float>(m_PlayerInputSettingsBoxes.at(player).SensitivitySlider->GetValue()) / 250.0F);
					}
					UpdatePlayerSensitivityValues(player);
				} else if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).CircleDeadZoneRadioButton && guiEvent.GetMsg() == GUIRadioButton::Pushed) {
					g_UInputMan.GetControlScheme(player)->SetJoystickDeadzoneType(DeadZoneType::CIRCLE);
					UpdatePlayerSensitivityValues(player);
				} else if (guiEvent.GetControl() == m_PlayerInputSettingsBoxes.at(player).SquareDeadZoneRadioButton && guiEvent.GetMsg() == GUIRadioButton::Pushed) {
					g_UInputMan.GetControlScheme(player)->SetJoystickDeadzoneType(DeadZoneType::SQUARE);
					UpdatePlayerSensitivityValues(player);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::InputConfigScreen::HandleInputEvents(GUIEvent &guiEvent, int player) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			if (guiEvent.GetControl() == CloseMappingBoxButton) {
				g_GUISound.ButtonPressSound()->Play();
				SetEnabled(false);
			} else if (guiEvent.GetControl() == RunConfigWizardButton) {
				;
			}
			for (int mapButton = 0; mapButton < PlayerInputMappings::InputMappingCount; ++mapButton) {
				if (guiEvent.GetControl() == InputMapButton.at(mapButton)) {
					;
				}
			}
		}
	}


}