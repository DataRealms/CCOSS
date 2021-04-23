#include "SettingsInputGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUISlider.h"
#include "GUICheckbox.h"
#include "GUILabel.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputGUI::SettingsInputGUI(GUIControlManager *parentControlManager, Controller *controller) : m_GUIControlManager(parentControlManager), m_Controller(controller) {
		m_InputSettingsBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("CollectionBoxInputSettings"));

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			m_PlayerInputSettingsBoxes.at(player) = PlayerInputSettingsBox();

			std::string playerNum = std::to_string(player + 1);

			/*
			m_PlayerInputSettingsBoxes.at(player).InputDeviceLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "Device"));
			m_PlayerInputSettingsBoxes.at(player).NextInputDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "NextDevice"));
			m_PlayerInputSettingsBoxes.at(player).PrevInputDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "PrevDevice"));
			m_PlayerInputSettingsBoxes.at(player).ConfigureControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Config"));
			m_PlayerInputSettingsBoxes.at(player).ClearControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Clear"));

			m_PlayerInputSettingsBoxes.at(player).DeadZoneSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderP" + playerNum + "DeadZone"));
			m_PlayerInputSettingsBoxes.at(player).DeadZoneSlider->SetValue(static_cast<int>(g_UInputMan.GetControlScheme(0)->GetJoystickDeadzone()) * 250);

			m_PlayerInputSettingsBoxes.at(player).DeadZoneLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "DeadZoneValue"));
			m_PlayerInputSettingsBoxes.at(player).DeadZoneLabel->SetText(std::to_string(m_PlayerInputSettingsBoxes.at(player).DeadZoneSlider->GetValue()));

			m_PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxP" + playerNum + "DeadZoneType"));

			if (g_UInputMan.GetControlScheme(0)->GetJoystickDeadzoneType() == DeadZoneType::CIRCLE) {
				m_PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetCheck(true);
				m_PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetText("O");
			} else {
				m_PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetCheck(false);
				m_PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetText(std::string({ -2, 0 }));
			}
			*/
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputGUI::CreateInputConfigWizard() {
		m_InputConfigWizardMenu = InputConfigWizard();

		m_InputConfigWizardMenu.BackToOptionsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToOptions"));
		m_InputConfigWizardMenu.BackToOptionsButton->SetVisible(false);

		m_InputConfigWizardMenu.ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigTitle"));
		m_InputConfigWizardMenu.ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigRecKeyDesc"));
		m_InputConfigWizardMenu.ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigStep"));
		m_InputConfigWizardMenu.ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelInputConfigWizard::ConfigWizardLabels::ConfigInstruction"));
		m_InputConfigWizardMenu.ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigInput"));

		ContentFile diagramFile("Base.rte/GUIs/Controllers/D-Pad.png");
		BITMAP **tempDPadBitmaps = diagramFile.GetAsAnimation(InputConfigWizard::ConfigWizardSteps::DPadConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDPadBitmaps); ++i) {
			m_InputConfigWizardMenu.DPadBitmaps.at(i) = tempDPadBitmaps[i];
		}
		delete[] tempDPadBitmaps;

		diagramFile.SetDataPath("Base.rte/GUIs/Controllers/DualAnalog.png");
		BITMAP **tempDualAnalogBitmaps = diagramFile.GetAsAnimation(InputConfigWizard::ConfigWizardSteps::DualAnalogConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDualAnalogBitmaps); ++i) {
			m_InputConfigWizardMenu.DualAnalogBitmaps.at(i) = tempDualAnalogBitmaps[i];
		}
		delete[] tempDualAnalogBitmaps;

		m_InputConfigWizardMenu.RecommendationBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRec"));
		m_InputConfigWizardMenu.RecommendationDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRecDiagram"));
		m_InputConfigWizardMenu.ConfigSkipButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigSkip"));
		m_InputConfigWizardMenu.ConfigBackButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigBack"));

		m_InputConfigWizardMenu.DPadTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadType"));
		m_InputConfigWizardMenu.DPadTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDPadType"));
		m_InputConfigWizardMenu.DPadTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadTypeDiagram"));
		m_InputConfigWizardMenu.DPadTypeDiagram->Resize(m_InputConfigWizardMenu.DPadBitmaps.at(0)->w, m_InputConfigWizardMenu.DPadBitmaps.at(0)->h);
		m_InputConfigWizardMenu.DPadTypeDiagram->CenterInParent(true, true);
		m_InputConfigWizardMenu.DPadTypeDiagram->MoveRelative(0, -8);

		m_InputConfigWizardMenu.DAnalogTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogType"));
		m_InputConfigWizardMenu.DAnalogTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDAnalogType"));
		m_InputConfigWizardMenu.DAnalogTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogTypeDiagram"));
		m_InputConfigWizardMenu.DAnalogTypeDiagram->Resize(m_InputConfigWizardMenu.DualAnalogBitmaps.at(0)->w, m_InputConfigWizardMenu.DualAnalogBitmaps.at(0)->h);
		m_InputConfigWizardMenu.DAnalogTypeDiagram->CenterInParent(true, true);
		m_InputConfigWizardMenu.DAnalogTypeDiagram->MoveRelative(0, -10);

		m_InputConfigWizardMenu.XBox360TypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360Type"));
		m_InputConfigWizardMenu.XBox360TypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigXBox360Type"));
		m_InputConfigWizardMenu.XBox360TypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360TypeDiagram"));
		m_InputConfigWizardMenu.XBox360TypeDiagram->Resize(m_InputConfigWizardMenu.DualAnalogBitmaps.at(0)->w, m_InputConfigWizardMenu.DualAnalogBitmaps.at(0)->h);
		m_InputConfigWizardMenu.XBox360TypeDiagram->CenterInParent(true, true);
		m_InputConfigWizardMenu.XBox360TypeDiagram->MoveRelative(0, -10);

		m_InputConfigWizardMenu.ConfiguringPlayer = Players::PlayerOne;
		m_InputConfigWizardMenu.ConfiguringDevice = InputDevice::DEVICE_KEYB_ONLY;
		m_InputConfigWizardMenu.ConfiguringGamepad = InputConfigWizard::GamepadType::DPad;
		m_InputConfigWizardMenu.ConfigureStep = 0;
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

	void SettingsInputGUI::HandleInputEvents(GUIEvent &guiEvent) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			/*
			// Control options
			if (m_MenuScreen == OPTIONSSCREEN) {
				int which = 0;
				int player = 0;

				// Handle all device select button pushes
				for (which = P1NEXT; which <= P4PREV; ++which) {
					// Calculate the owner of the currently checked button, and if it's next/prev button
					player = (which - P1NEXT) % Players::MaxPlayerCount;
					bool nextButton = which < P1PREV;

					// Handle the appropriate player's device setting
					if (guiEvent.GetControl() == m_OptionButton.at(which)) {
						// What's the current device
						int currentDevice = g_UInputMan.GetControlScheme(player)->GetDevice();
						// Next button pressed, so increment
						if (nextButton) {
							// Loop around to first if we've gone around
							if (++currentDevice >= DEVICE_COUNT) { currentDevice = 0; }
						}
						// Prev button pressed, so decrement
						else {
							// Loop around to last if we've gone around
							if (--currentDevice < 0) { currentDevice = DEVICE_COUNT - 1; }
						}
						// Set the device and update labels
						g_UInputMan.GetControlScheme(player)->SetDevice(static_cast<InputDevice>(currentDevice));
						UpdateDeviceLabels();

						g_GUISound.ButtonPressSound()->Play();
					}
				}

				// Handle all control config buttons
				for (which = P1CONFIG; which <= P4CONFIG; ++which) {
					// Handle the appropriate player's device setting
					if (guiEvent.GetControl() == m_OptionButton.at(which)) {
						m_apScreenBox.at(OPTIONSSCREEN)->SetVisible(false);
						ConfiguringPlayer = which - P1CONFIG;
						m_ConfiguringDevice = g_UInputMan.GetControlScheme(ConfiguringPlayer)->GetDevice();
						ConfigureStep = 0;
						m_MenuScreen = CONFIGSCREEN;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
				}

				// Handle all control reset buttons
				for (which = P1CLEAR; which <= P4CLEAR; ++which) {
					// Handle the appropriate player's clearing of mappings
					if (guiEvent.GetControl() == m_OptionButton.at(which)) {
						// Make user click twice to confirm
						if (m_OptionButton.at(which)->GetText() == "Reset") {
							// Ask to confirm!
							m_OptionButton.at(which)->SetText("CONFIRM?");
							// And clear all other buttons of it
							for (int otherButton = P1CLEAR; otherButton <= P4CLEAR; ++otherButton) {
								if (otherButton != which) { m_OptionButton.at(otherButton)->SetText("Reset"); }
							}
							g_GUISound.ButtonPressSound()->Play();
						} else {
							// Set to a default control preset.
							Players inputPlayer = static_cast<Players>(which - P1CLEAR);
							InputPreset playerPreset = static_cast<InputPreset>(P1CLEAR - which - 1); // Player 1's default preset is at -1 and so on.
							g_UInputMan.GetControlScheme(inputPlayer)->SetPreset(playerPreset);

							// Set to a device that fits this preset.
							InputDevice deviceType.at(4) = { DEVICE_MOUSE_KEYB, DEVICE_KEYB_ONLY, DEVICE_GAMEPAD_1, DEVICE_GAMEPAD_2 };
							g_UInputMan.GetControlScheme(inputPlayer)->SetDevice(deviceType.at(inputPlayer));

							UpdateDeviceLabels();

							// Set the dead zone slider value
							m_DeadZoneSlider.at(which - P1CLEAR)->SetValue(g_UInputMan.GetControlScheme(which - P1CLEAR)->GetJoystickDeadzone() * 250);

							//                            m_OptionsLabel.at(P1DEVICE + (which - P1CLEAR))->SetText("NEEDS CONFIG!");
							//                            m_OptionButton.at(P1CONFIG + (which - P1CLEAR))->SetText("-> CONFIGURE <-");
							g_GUISound.ExitMenuSound()->Play();
						}
					}
				}
			}

			//////////////////////////////////
			// Control config buttons

			if (m_MenuScreen == CONFIGSCREEN) {
				// DPad Gamepad type selected
				if (guiEvent.GetControl() == m_DPadTypeButton) {
					ConfiguringGamepad = DPAD;
					ConfigureStep++;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// DPad Gamepad type selected
				if (guiEvent.GetControl() == m_DAnalogTypeButton) {
					ConfiguringGamepad = DANALOG;
					ConfigureStep++;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// XBox Gamepad type selected
				if (guiEvent.GetControl() == m_XBox360TypeButton) {
					// Not allowing config, this is a complete preset
					//					ConfiguringGamepad = XBOX360;
					//					ConfigureStep++;
					//					m_ScreenChange = true;
					//
					// Set up the preset that will work well for a 360 controller
					g_UInputMan.GetControlScheme(ConfiguringPlayer)->SetPreset(PRESET_XBOX360);

					// Go back to the options screen immediately since the preset is all taken care of
					m_apScreenBox.at(CONFIGSCREEN)->SetVisible(false);
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;

					g_GUISound.ExitMenuSound()->Play();
				}

				// Skip ahead one config step button pressed
				if (guiEvent.GetControl() == m_ConfigSkipButton) {
					// TODO: error checking so that we don't put configurestep out of bounds!
					ConfigureStep++;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Go back one config step button pressed
				if (guiEvent.GetControl() == m_ConfigBackButton) {
					if (ConfigureStep > 0) {
						ConfigureStep--;
						m_ScreenChange = true;
						g_GUISound.BackButtonPressSound()->Play();
					} else {
						g_GUISound.UserErrorSound()->Play();
					}
				}
			}
			*/
		} else if (guiEvent.GetType() == GUIEvent::Notification) {
			/*
			// Dead zone sliders control
			for (int which = P1DEADZONESLIDER; which < DEADZONESLIDERCOUNT; ++which) {
				// Handle the appropriate player's clearing of mappings
				if (guiEvent.GetControl() == m_DeadZoneSlider.at(which)) {
					// Display value
					char s.at(256);
					std::snprintf(s, sizeof(s), "%d", m_DeadZoneSlider.at(which)->GetValue());
					m_DeadZoneLabel.at(which)->SetText(s);

					// Update control scheme
					g_UInputMan.GetControlScheme(which)->SetJoystickDeadzone((float)m_DeadZoneSlider.at(which)->GetValue() / 200.0F);
				}

				if (guiEvent.GetControl() == m_DeadZoneCheckbox.at(which)) {
					if (m_DeadZoneCheckbox.at(which)->GetCheck() == 1) {
						g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::CIRCLE);
						m_DeadZoneCheckbox.at(which)->SetText("O");
					} else {
						g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::SQUARE);
						char str[2];
						str[0] = -2;
						str[1] = 0;
						m_DeadZoneCheckbox.at(which)->SetText(str);
					}
				}
			}
			*/
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	void SettingsInputGUI::UpdateDeviceLabels() {
		int device = 0;
		string label;

		// Cycle through all players
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
			device = g_UInputMan.GetControlScheme(player)->GetDevice();

			if (device == DEVICE_KEYB_ONLY) {
				label = "Classic Keyb";
			} else if (device == DEVICE_MOUSE_KEYB) {
				label = "Keyb + Mouse";
			} else if (device == DEVICE_GAMEPAD_1) {
				label = "Gamepad 1";
			} else if (device == DEVICE_GAMEPAD_2) {
				label = "Gamepad 2";
			} else if (device == DEVICE_GAMEPAD_3) {
				label = "Gamepad 3";
			} else if (device == DEVICE_GAMEPAD_4) {
				label = "Gamepad 4";
			}
			// Set the label
			m_OptionsLabel.at(P1DEVICE + player)->SetText(label);

			// Reset Config and Clear button labels
			m_OptionButton.at(P1CONFIG + player)->SetText("Configure");
			m_OptionButton.at(P1CLEAR + player)->SetText("Reset");
		}
	}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	void SettingsInputGUI::InputSettingsMenu::InputConfigWizard::UpdateConfigScreen() {
		char str[256];

		if (m_ScreenChange) {
			// Hide most things first, enable as needed
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(false);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(false);
			RecommendationBox->SetVisible(false);
			RecommendationDiagram->SetVisible(false);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(false);
			ConfigSkipButton->SetVisible(false);
			ConfigBackButton->SetVisible(false);
			DPadTypeBox->SetVisible(false);
			DAnalogTypeBox->SetVisible(false);
			XBox360TypeBox->SetVisible(false);
		}

		// .at(CHRISK) Use GUI input class for better key detection
		g_UInputMan.SetInputClass(GUIInput);

		switch (ConfiguringDevice) {
			case InputDevice::DEVICE_KEYB_ONLY:
				UpdateKeyboardConfigWizard();
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				UpdateMouseAndKeyboardConfigWizard();
				break;
			case InputDevice::DEVICE_GAMEPAD_1:
			case InputDevice::DEVICE_GAMEPAD_2:
			case InputDevice::DEVICE_GAMEPAD_3:
			case InputDevice::DEVICE_GAMEPAD_4:
				UpdateGamepadConfigWizard();
			default:
				break;
		}

		g_UInputMan.SetInputClass(NULL);
		if (m_ScreenChange) { g_GUISound.ExitMenuSound()->Play(); }
	}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	bool SettingsInputGUI::InputConfigWizard::UpdateKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, InputConfigWizard::ConfigWizardSteps::KeyboardConfigSteps);
		ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Up Cursor]" : "[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Down Cursor]" : "[S)");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Left Cursor]" : "[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Right Cursor]" : "[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 1]" : "[H]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_FIRE))

				{
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 2]" : "[J]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 3]" : "[K]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PIEMENU)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("JUMP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Enter]" : "[L]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("CROUCH");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Del]" : "[.]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 4]" : "[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 5]" : "[E]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 11:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 0]" : "[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 12:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 9]" : "[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 13:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 6]" : "[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 14:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 7]" : "[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 15:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 8]" : "[C]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
					m_apScreenBox.at(CONFIGSCREEN)->SetVisible(false);
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;
					return true;
				}
				break;
			default:
				break;
		}
		return false;
	}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	bool SettingsInputGUI::InputConfigWizard::UpdateMouseAndKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Mouse + Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, InputConfigWizard::ConfigWizardSteps::MouseAndKeyboardConfigSteps);
		ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_UP);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[S]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_DOWN);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_LEFT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_RIGHT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREV WEAPON");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[C]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[E]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_NEXT)) {
					//                ConfigureStep++;
					//                m_ScreenChange = true;
									// Done, go back to options screen
					m_apScreenBox.at(CONFIGSCREEN)->SetVisible(false);
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;
					return true;
				}
				break;
			default:
				break;
		}
		return false;
	}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	bool SettingsInputGUI::InputConfigWizard::UpdateGamepadConfigWizard() {
		int whichJoy = ConfiguringDevice - InputDevice::DEVICE_GAMEPAD_1;
		AllegroBitmap *pDiagramBitmap = 0;

		// Choose which gamepad type - special first step

		if (ConfigureStep == 0) {
			// Set title
			std::snprintf(str, sizeof(str), "Choose Gamepad Type for Player %i:", ConfiguringPlayer + 1);
			ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);

			// Hide the back button on this first step
			ConfigBackButton->SetVisible(false);

			// Show the type option boxes
			DPadTypeBox->SetVisible(true);
			DAnalogTypeBox->SetVisible(true);
			XBox360TypeBox->SetVisible(true);

			// Not passing in ownership of the BITMAP
			pDiagramBitmap = new AllegroBitmap(DPadBitmaps.at(0));
			// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
			DPadTypeDiagram->SetDrawImage(pDiagramBitmap);
			pDiagramBitmap = 0;

			// Not passing in ownership of the BITMAP
			pDiagramBitmap = new AllegroBitmap(DualAnalogBitmaps.at(0));
			// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
			DAnalogTypeDiagram->SetDrawImage(pDiagramBitmap);
			// Doing it again for the 360 one.. it's similar enough looking
			pDiagramBitmap = new AllegroBitmap(DualAnalogBitmaps.at(0));
			XBox360TypeDiagram->SetDrawImage(pDiagramBitmap);
			pDiagramBitmap = 0;

			// The special selection buttons take care of advancing the step, so do nothing else
			m_ScreenChange = false;
		}
		// Configure selected gamepad type
		else {
			if (m_ScreenChange) {
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
				RecommendationBox->SetVisible(true);
				ConfigSkipButton->SetVisible(true);
				ConfigBackButton->SetVisible(true);
				m_BlinkTimer.Reset();
			}

			// D-pad
			if (ConfiguringGamepad == DPAD) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "D-Pad Gamepad Configuration - Player %i", ConfiguringPlayer + 1);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DPadBitmaps.at(0)->w, DPadBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 4);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, InputConfigWizard::ConfigWizardSteps::DPadConfigSteps);
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DPadBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");

						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_AIM_UP)) {
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_UP);
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_L_UP);
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_UP);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_UP);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 2:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_AIM_DOWN)) {
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_DOWN);
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_L_DOWN);
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_DOWN);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_DOWN);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 3:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_LEFT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_RIGHT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_AIM)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_BACK)) {
							// Done, go back to options screen
							m_apScreenBox.at(CONFIGSCREEN)->SetVisible(false);
							m_MenuScreen = OPTIONSSCREEN;
							m_ScreenChange = true;
							return true;
						}
						break;
					default:
						break;
				}
			}
			// Dual analog OR XBox Controller
			else if (ConfiguringGamepad == DANALOG || ConfiguringGamepad == XBOX360) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "Dual Analog Gamepad Configuration - Player %i", ConfiguringPlayer + 1);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DualAnalogBitmaps.at(0)->w, DualAnalogBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 8);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, InputConfigWizard::ConfigWizardSteps::DualAnalogConfigSteps);
				ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DualAnalogBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_UP)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_UP);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 2:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_DOWN)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_DOWN);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_CROUCH);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 3:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM UP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_UP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM DOWN");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_DOWN)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 13:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 14:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 15:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICKUP WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_PICKUP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 16:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_RELOAD)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 17:
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 18:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(InputConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_BACK)) {
							// If Xbox controller; if the A button has not been mapped to Activate/fire, then map it automatically
							// These redundancies should apply to all custom analog setups, really
							//if (ConfiguringGamepad == XBOX360)
							{
								// No button assigned to fire, so give it 'A' on the controller (in addition to any axis inputs)
								if (g_UInputMan.GetButtonMapping(ConfiguringPlayer, INPUT_FIRE) == JOY_NONE) { g_UInputMan.SetButtonMapping(ConfiguringPlayer, INPUT_FIRE, JOY_1); }
								// No button assigned to pie menu, so give it 'B' on the controller (in addition to whatever axis it's assinged to)
								if (g_UInputMan.GetButtonMapping(ConfiguringPlayer, INPUT_PIEMENU) == JOY_NONE) { g_UInputMan.SetButtonMapping(ConfiguringPlayer, INPUT_PIEMENU, JOY_2); }
							}

							// Done, go back to options screen
							m_apScreenBox.at(CONFIGSCREEN)->SetVisible(false);
							m_MenuScreen = OPTIONSSCREEN;
							m_ScreenChange = true;
							return true;
						}
						break;
					default:
						break;
				}
			}
		}
		return false;
	}
*/
}