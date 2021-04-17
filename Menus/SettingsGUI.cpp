#include "SettingsGUI.h"

#include "FrameMan.h"
#include "UInputMan.h"
#include "ActivityMan.h"
#include "SettingsMan.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUIComboBox.h"
#include "GUIListBox.h"
#include "GUICheckbox.h"
#include "GUIButton.h"
#include "GUISlider.h"
#include "GUILabel.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"
#include "AllegroInput.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsGUI::SettingsGUI(AllegroScreen *guiScreen, AllegroInput *guiInput, Controller *controller) {
		if (!m_GUIControlManager) { m_GUIControlManager = std::make_unique<GUIControlManager>(); }
		if (!m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSkin.ini")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSkin.ini"); }
		m_GUIControlManager->Load("Base.rte/GUIs/SettingsGUI.ini");

		m_Controller = controller;

		GUICollectionBox *rootBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"));
		rootBox->SetPositionAbs((g_FrameMan.GetResX() - rootBox->GetWidth()) / 2, 0);

		//dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("OptionsScreen"))->SetVisible(false);
		//dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ConfigScreen"))->SetVisible(false);

		m_BackToMainButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMainMenu"));

		m_AudioSettingsMenu = std::make_unique<SettingsAudioGUI>(m_GUIControlManager.get());

		CreateVideoSettingsMenu();
		CreateInputSettingsMenu();
		CreateGameplaySettingsMenu();

		m_ActiveSettingsMenu = ActiveSettingsMenu::AudioSettingsActive;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::HandleInputEvents() {
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_BackToMainButton) {
					return true;
				}
			}

			switch (m_ActiveSettingsMenu) {
				case ActiveSettingsMenu::AudioSettingsActive:
					m_AudioSettingsMenu->HandleInputEvents(guiEvent);
					break;
				default:
					break;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::HandleVideoSettingsMenuInputEvents(const GUIControl *guiEventControl) {
		if (guiEventControl == m_VideoSettingsMenu.FullscreenOrWindowedButton) {
			g_GUISound.ButtonPressSound()->Play();

			if (!g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
				/*
				if (g_ActivityMan.GetActivity()) {
					m_VideoSettingsMenu.ResolutionChangeToUpscaled = false;
					m_VideoSettingsMenu.ResolutionChangeDialogBox->SetVisible(true);
					m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(false);
					m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(false);
					m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
				} else {
					HideAllScreens();
					m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
					g_FrameMan.SwitchToFullscreen(false);
				}
				*/
				g_FrameMan.SwitchToFullscreen(false);
			} else if (g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
				/*
				if (g_ActivityMan.GetActivity()) {
					m_ResolutionChangeToUpscaled = false;
					m_ResolutionChangeDialog->SetVisible(true);
					m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(false);
					m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(false);
					m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
				} else {
					HideAllScreens();
					m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
					g_FrameMan.SwitchResolution(960, 540);
				}
				*/
				g_FrameMan.SwitchResolution(960, 540);
			} else if (!g_FrameMan.IsFullscreen() && g_FrameMan.IsUpscaledFullscreen()) {
				g_FrameMan.SwitchResolutionMultiplier(1);
			}
			//UpdateResolutionCombo();
		} else if (guiEventControl == m_VideoSettingsMenu.UpscaledFullscreenButton) {
			g_GUISound.ButtonPressSound()->Play();
			/*
			if (!g_FrameMan.IsUpscaledFullscreen()) {
				if (g_ActivityMan.GetActivity()) {
					m_ResolutionChangeToUpscaled = true;
					m_ResolutionChangeDialog->SetVisible(true);
					m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(false);
					m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(false);
					m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
				} else {
					HideAllScreens();
					m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
					g_FrameMan.SwitchToFullscreen(true);
				}
			}
			UpdateResolutionCombo();
			*/
		} else if (guiEventControl == m_VideoSettingsMenu.ConfirmResolutionChangeFullscreenButton) {
			g_GUISound.ButtonPressSound()->Play();
			/*
			HideAllScreens();
			m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
			m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(true);
			m_ResolutionChangeDialog->SetVisible(false);
			m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(true);
			m_ButtonConfirmResolutionChangeFullscreen->SetVisible(false);
			if (!m_ResolutionChangeToUpscaled && g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
				g_FrameMan.SwitchResolution(960, 540, 1, true);
			} else {
				g_FrameMan.SwitchToFullscreen(m_ResolutionChangeToUpscaled ? true : false, true);
			}
			UpdateResolutionCombo();
			*/
		} else if (guiEventControl == m_VideoSettingsMenu.ConfirmResolutionChangeButton) {
			g_GUISound.ButtonPressSound()->Play();
			/*
			HideAllScreens();
			m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
			m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(true);
			m_ResolutionChangeDialog->SetVisible(false);
			m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(true);
			m_ButtonConfirmResolutionChange->SetVisible(false);
			g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1, true);
			UpdateResolutionCombo();
			*/
		} else if (guiEventControl == m_VideoSettingsMenu.CancelResolutionChangeButton) {
			g_GUISound.ButtonPressSound()->Play();
			/*
			m_ResolutionChangeDialog->SetVisible(false);
			m_apScreenBox.at(OPTIONSSCREEN)->SetEnabled(true);
			m_MainMenuButtons.at(BACKTOMAIN)->SetEnabled(true);
			*/
		}
	}




	/*
				// Return to main menu button pressed
				if (guiEvent.GetControl() == m_MainMenuButtons.at(BACKTOMAIN)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);

					// If leaving the options screen, save the settings!
					if (m_MenuScreen == OPTIONSSCREEN) {
						g_SettingsMan.SetFlashOnBrainDamage(m_OptionsCheckbox.at(FLASHONBRAINDAMAGE)->GetCheck());
						g_SettingsMan.SetBlipOnRevealUnseen(m_OptionsCheckbox.at(BLIPONREVEALUNSEEN)->GetCheck());
						g_SettingsMan.SetShowForeignItems(m_OptionsCheckbox.at(SHOWFOREIGNITEMS)->GetCheck());
						g_SettingsMan.SetShowToolTips(m_OptionsCheckbox.at(SHOWTOOLTIPS)->GetCheck());

						g_SettingsMan.UpdateSettingsFile();
					}

					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;

					g_GUISound.BackButtonPressSound()->Play();
				}

				// Return to options menu button pressed
				if (guiEvent.GetControl() == m_BackToOptionsButton) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					BackToOptionsButton->SetVisible(false);
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;

					g_GUISound.BackButtonPressSound()->Play();
				}


				/////////////////////////////////////////////
				// CONTROL CONFIG UI

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


				// Notifications
				else if (guiEvent.GetType() == GUIEvent::Notification) {


					// Resolution combobox closed, something new selected
					if (guiEvent.GetControl() == m_ResolutionCombo) {
						// Closed it, IE selected somehting
						if (guiEvent.GetMsg() == GUIComboBox::Closed) {
							// Get and read the new resolution data from the item's label
							GUIListPanel::Item *pResItem = m_ResolutionCombo->GetItem(m_ResolutionCombo->GetSelectedIndex());
							if (pResItem && !pResItem->m_Name.empty()) {
								int newResX;
								int newResY;
								sscanf(pResItem->m_Name.c_str(), "%4dx%4d", &newResX, &newResY);
								// Sanity check the values and then set them as the new resolution to be switched to next time FrameMan is created
								if (g_FrameMan.IsValidResolution(newResX, newResY)) {
									g_FrameMan.SetNewResX(newResX);
									g_FrameMan.SetNewResY(newResY);
								}
							}

							if (g_FrameMan.IsNewResSet()) {
								if (g_ActivityMan.GetActivity()) {
									m_ResolutionChangeDialog->SetVisible(true);
									m_ButtonConfirmResolutionChange->SetVisible(true);
								} else {
									HideAllScreens();
									m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
									g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1);
								}
							}
						}
					}

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
				}
			}
		}
	*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::Draw() {
		m_GUIControlManager->Draw();
		m_GUIControlManager->DrawMouse();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::CreateVideoSettingsMenu() {
		m_VideoSettingsMenu = VideoSettingsMenu();

		m_VideoSettingsMenu.ValidResolutions.clear();

		m_VideoSettingsMenu.ResolutionComboBox = dynamic_cast<GUIComboBox *>(m_GUIControlManager->GetControl("ComboResolution"));

		m_VideoSettingsMenu.FullscreenOrWindowedButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonFullscreen"));
		m_VideoSettingsMenu.UpscaledFullscreenButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonUpscaledFullscreen"));

		m_VideoSettingsMenu.ResolutionChangeDialogBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("ResolutionChangeDialog"));
		m_VideoSettingsMenu.ResolutionChangeDialogBox->CenterInParent(true, true);
		m_VideoSettingsMenu.ResolutionChangeDialogBox->SetVisible(false);

		m_VideoSettingsMenu.ConfirmResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChange"));
		m_VideoSettingsMenu.ConfirmResolutionChangeButton->SetVisible(false);
		m_VideoSettingsMenu.ConfirmResolutionChangeFullscreenButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfirmResolutionChangeFullscreen"));
		m_VideoSettingsMenu.ConfirmResolutionChangeFullscreenButton->SetVisible(false);
		m_VideoSettingsMenu.CancelResolutionChangeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonCancelResolutionChange"));

		/*
		if (g_FrameMan.GetResX() * g_FrameMan.ResolutionMultiplier() == g_FrameMan.GetMaxResX() && g_FrameMan.GetResY() * g_FrameMan.ResolutionMultiplier() == g_FrameMan.GetMaxResY()) {
			m_VideoSettingsMenu.FullscreenOrWindowedButton->SetText("Windowed");
		} else {
			m_VideoSettingsMenu.FullscreenOrWindowedButton->SetText("Fullscreen");
		}
		*/

		//UpdateResolutionCombo();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::CreateInputSettingsMenu() {
		m_InputSettingsMenu = InputSettingsMenu();

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player + 1);

			/*
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player) = PlayerInputSettingsBox();

			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).InputDeviceLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "Device"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).NextInputDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "NextDevice"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).PrevInputDeviceButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "PrevDevice"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).ConfigureControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Config"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).ClearControlsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonP" + playerNum + "Clear"));

			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneSlider = dynamic_cast<GUISlider *>(m_GUIControlManager->GetControl("SliderP" + playerNum + "DeadZone"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneSlider->SetValue(static_cast<int>(g_UInputMan.GetControlScheme(0)->GetJoystickDeadzone()) * 250);

			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneLabel = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelP" + playerNum + "DeadZoneValue"));
			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneLabel->SetText(std::to_string(m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneSlider->GetValue()));

			m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("CheckboxP" + playerNum + "DeadZoneType"));

			if (g_UInputMan.GetControlScheme(0)->GetJoystickDeadzoneType() == DeadZoneType::CIRCLE) {
				m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetCheck(true);
				m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetText("O");
			} else {
				m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetCheck(false);
				m_InputSettingsMenu.PlayerInputSettingsBoxes.at(player).DeadZoneTypeCheckbox->SetText(std::string({ -2, 0 }));
			}
			*/
		}

		//UpdateDeviceLabels();

		/*
		ControlConfigWizardMenu = ControlConfigWizard();

		ControlConfigWizardMenu.BackToOptionsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToOptions"));
		ControlConfigWizardMenu.BackToOptionsButton->SetVisible(false);

		// Config screen controls
		ControlConfigWizardMenu.ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigTitle"));
		ControlConfigWizardMenu.ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigRecKeyDesc"));
		ControlConfigWizardMenu.ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigStep"));
		ControlConfigWizardMenu.ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelControlConfigWizard::ConfigWizardLabels::ConfigInstruction"));
		ControlConfigWizardMenu.ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigInput"));

		ContentFile diagramFile("Base.rte/GUIs/Controllers/D-Pad.png");
		BITMAP **tempDPadBitmaps = diagramFile.GetAsAnimation(ControlConfigWizard::ConfigWizardSteps::DPadConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDPadBitmaps); ++i) {
			ControlConfigWizardMenu.DPadBitmaps.at(i) = tempDPadBitmaps[i];
		}
		delete[] tempDPadBitmaps;

		diagramFile.SetDataPath("Base.rte/GUIs/Controllers/DualAnalog.png");
		BITMAP **tempDualAnalogBitmaps = diagramFile.GetAsAnimation(ControlConfigWizard::ConfigWizardSteps::DualAnalogConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDualAnalogBitmaps); ++i) {
			ControlConfigWizardMenu.DualAnalogBitmaps.at(i) = tempDualAnalogBitmaps[i];
		}
		delete[] tempDualAnalogBitmaps;

		ControlConfigWizardMenu.RecommendationBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRec"));
		ControlConfigWizardMenu.RecommendationDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRecDiagram"));
		ControlConfigWizardMenu.ConfigSkipButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigSkip"));
		ControlConfigWizardMenu.ConfigBackButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigBack"));

		ControlConfigWizardMenu.DPadTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadType"));
		ControlConfigWizardMenu.DPadTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDPadType"));
		ControlConfigWizardMenu.DPadTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadTypeDiagram"));
		ControlConfigWizardMenu.DPadTypeDiagram->Resize(ControlConfigWizardMenu.DPadBitmaps.at(0)->w, ControlConfigWizardMenu.DPadBitmaps.at(0)->h);
		ControlConfigWizardMenu.DPadTypeDiagram->CenterInParent(true, true);
		ControlConfigWizardMenu.DPadTypeDiagram->MoveRelative(0, -8);

		ControlConfigWizardMenu.DAnalogTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogType"));
		ControlConfigWizardMenu.DAnalogTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDAnalogType"));
		ControlConfigWizardMenu.DAnalogTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogTypeDiagram"));
		ControlConfigWizardMenu.DAnalogTypeDiagram->Resize(ControlConfigWizardMenu.DualAnalogBitmaps.at(0)->w, ControlConfigWizardMenu.DualAnalogBitmaps.at(0)->h);
		ControlConfigWizardMenu.DAnalogTypeDiagram->CenterInParent(true, true);
		ControlConfigWizardMenu.DAnalogTypeDiagram->MoveRelative(0, -10);

		ControlConfigWizardMenu.XBox360TypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360Type"));
		ControlConfigWizardMenu.XBox360TypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigXBox360Type"));
		ControlConfigWizardMenu.XBox360TypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360TypeDiagram"));
		ControlConfigWizardMenu.XBox360TypeDiagram->Resize(ControlConfigWizardMenu.DualAnalogBitmaps.at(0)->w, ControlConfigWizardMenu.DualAnalogBitmaps.at(0)->h);
		ControlConfigWizardMenu.XBox360TypeDiagram->CenterInParent(true, true);
		ControlConfigWizardMenu.XBox360TypeDiagram->MoveRelative(0, -10);

		ControlConfigWizardMenu.ConfiguringPlayer = Players::PlayerOne;
		ControlConfigWizardMenu.ConfiguringDevice = InputDevice::DEVICE_KEYB_ONLY;
		ControlConfigWizardMenu.ConfiguringGamepad = ControlConfigWizard::GamepadType::DPad;
		ControlConfigWizardMenu.ConfigureStep = 0;
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsGUI::CreateGameplaySettingsMenu() {
		m_GameplaySettingsMenu = GameplaySettingsMenu();

		m_GameplaySettingsMenu.FlashOnBrainDamageCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("FlashOnBrainDamageCheckbox"));
		m_GameplaySettingsMenu.FlashOnBrainDamageCheckbox->SetCheck(g_SettingsMan.FlashOnBrainDamage());

		m_GameplaySettingsMenu.BlipOnRevealUnseenCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("BlipOnRevealUnseenCheckbox"));
		m_GameplaySettingsMenu.BlipOnRevealUnseenCheckbox->SetCheck(g_SettingsMan.BlipOnRevealUnseen());

		m_GameplaySettingsMenu.ShowForeignItemsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("ShowForeignItemsCheckbox"));
		m_GameplaySettingsMenu.ShowForeignItemsCheckbox->SetCheck(g_SettingsMan.ShowForeignItems());

		m_GameplaySettingsMenu.ShowToolTipsCheckbox = dynamic_cast<GUICheckbox *>(m_GUIControlManager->GetControl("ShowToolTipsCheckbox"));
		m_GameplaySettingsMenu.ShowToolTipsCheckbox->SetCheck(g_SettingsMan.ToolTips());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string SettingsGUI::VideoSettingsMenu::ResolutionRecord::MakeResolutionString() {
		if (Upscaled) {
			return std::to_string(Width) + "x" + std::to_string(Height) + "Upscaled (" + std::to_string(Width / 2) + "x" + std::to_string(Height / 2) + ")";
		} else {
			return std::to_string(Width) + "x" + std::to_string(Height);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	void SettingsGUI::VideoSettingsMenu::PopulateResolutionsComboBox() {
			// Get a list of modes from the fullscreen driver even though we're not using it. This is so we don't need to populate the list manually and has all the reasonable resolutions.
#ifdef _WIN32
			GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
#elif __unix__
			GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_XWINDOWS_FULLSCREEN);
#endif
			// Index of found useful resolution (32bit)
			int foundIndex = 0;
			int currentResIndex = -1;

			for (int i = 0; resList && i < resList->num_modes; ++i) {
				ResolutionRecord resRecord;
				// Only list 32bpp modes
				if (resList->mode[i].bpp == 32) {
					int width = resList->mode[i].width;
					int height = resList->mode[i].height;

					// Resolution width must be in multiples of 4 otherwise Allegro fails to initialize graphics
					if (g_FrameMan.IsValidResolution(width, height) && width % 4 == 0) {
						// Fix wacky resolutions that are taller than wide
						if (height > width) {
							std::swap(width, height);
							//height = resList->mode[i].width;
							//width = resList->mode[i].height;
						}
						resRecord.Width = width;
						resRecord.Height = height;

						ValidResolutions.emplace_back(resRecord);

						// If this is what we're currently set to have at next start, select it afterward
						if ((g_FrameMan.GetNewResX() * g_FrameMan.ResolutionMultiplier()) == width && (g_FrameMan.GetNewResY() * g_FrameMan.ResolutionMultiplier()) == height) { currentResIndex = foundIndex; }
						// Only increment this when we find a usable 32bit resolution
						foundIndex++;
					}
				}
			}
			// Manually add qHD (960x540) to the list because it's rarely present in drivers
			ValidResolutions.emplace_back(ResolutionRecord({ 960, 540, false }));

			std::sort(ValidResolutions.begin(), ValidResolutions.end());

			for (ResolutionRecord &resRecord : ValidResolutions) {
				ResolutionComboBox->AddItem(resRecord.MakeResolutionString());
			}

			if (resList) { destroy_gfx_mode_list(resList); }


			// If none of the listed matched our resolution set for next start, add a 'custom' one to display as the current res
			if (currentResIndex < 0) {
				std::string isUpscaled = (g_FrameMan.ResolutionMultiplier() > 1) ? " Upscaled" : " Custom";
				resString = std::to_string(g_FrameMan.GetResX() / g_FrameMan.ResolutionMultiplier()) + "x" + std::to_string(g_FrameMan.GetResY() / g_FrameMan.ResolutionMultiplier()) + isUpscaled;
				ResolutionComboBox->AddItem(resString);
				currentResIndex = ResolutionComboBox->GetCount() - 1;
			}
			// Show the current resolution item to be the selected one
			ResolutionComboBox->SetSelectedIndex(currentResIndex);
	}

	*/

/*

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateDeviceLabels() {
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
	void SettingsGUI::InputSettingsMenu::ControlConfigWizard::UpdateConfigScreen() {
		char str[256];

		if (m_ScreenChange) {
			// Hide most things first, enable as needed
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(false);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(false);
			RecommendationBox->SetVisible(false);
			RecommendationDiagram->SetVisible(false);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(false);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::InputSettingsMenu::ControlConfigWizard::UpdateKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ControlConfigWizard::ConfigWizardSteps::KeyboardConfigSteps);
		ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Up Cursor]" : "[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Down Cursor]" : "[S)");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Left Cursor]" : "[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Right Cursor]" : "[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 1]" : "[H]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_FIRE))

				{
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 2]" : "[J]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 3]" : "[K]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PIEMENU)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("JUMP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Enter]" : "[L]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("CROUCH");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Del]" : "[.]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 4]" : "[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 5]" : "[E]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 11:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 0]" : "[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 12:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 9]" : "[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 13:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 6]" : "[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 14:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 7]" : "[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 15:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 8]" : "[C]");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::InputSettingsMenu::ControlConfigWizard::UpdateMouseAndKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Mouse + Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ControlConfigWizard::ConfigWizardSteps::MouseAndKeyboardConfigSteps);
		ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_UP);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[S]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_DOWN);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_LEFT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_RIGHT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREV WEAPON");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[C]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetText("[E]");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsGUI::InputSettingsMenu::ControlConfigWizard::UpdateGamepadConfigWizard() {
		int whichJoy = ConfiguringDevice - InputDevice::DEVICE_GAMEPAD_1;
		AllegroBitmap *pDiagramBitmap = 0;

		// Choose which gamepad type - special first step

		if (ConfigureStep == 0) {
			// Set title
			std::snprintf(str, sizeof(str), "Choose Gamepad Type for Player %i:", ConfiguringPlayer + 1);
			ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);

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
			pDiagramBitmap = new AllegroBitmap(m_aDualAnalogBitmaps.at(0));
			// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
			DAnalogTypeDiagram->SetDrawImage(pDiagramBitmap);
			// Doing it again for the 360 one.. it's similar enough looking
			pDiagramBitmap = new AllegroBitmap(m_aDualAnalogBitmaps.at(0));
			XBox360TypeDiagram->SetDrawImage(pDiagramBitmap);
			pDiagramBitmap = 0;

			// The special selection buttons take care of advancing the step, so do nothing else
			m_ScreenChange = false;
		}
		// Configure selected gamepad type
		else {
			if (m_ScreenChange) {
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetVisible(true);
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetVisible(true);
				RecommendationBox->SetVisible(true);
				ConfigSkipButton->SetVisible(true);
				ConfigBackButton->SetVisible(true);
				m_BlinkTimer.Reset();
			}

			// D-pad
			if (ConfiguringGamepad == DPAD) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "D-Pad Gamepad Configuration - Player %i", ConfiguringPlayer + 1);
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DPadBitmaps.at(0)->w, DPadBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 4);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ControlConfigWizard::ConfigWizardSteps::DPadConfigSteps);
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DPadBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");

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
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
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
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_LEFT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_RIGHT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_AIM)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
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
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DualAnalogBitmaps.at(0)->w, DualAnalogBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 8);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ControlConfigWizard::ConfigWizardSteps::DualAnalogConfigSteps);
				ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DualAnalogBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_UP)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_UP);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 2:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_DOWN)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_DOWN);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_CROUCH);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 3:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM UP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_UP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM DOWN");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_DOWN)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("AIM RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 13:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 14:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 15:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("PICKUP WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_PICKUP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 16:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("RELOAD WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_RELOAD)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 17:
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 18:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(ControlConfigWizard::ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
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