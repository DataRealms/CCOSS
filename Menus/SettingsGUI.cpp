/*

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

clear {
		for (int button = 0; button < OPTIONSBUTTONCOUNT; ++button) {
			m_aOptionButton[button] = 0;
		}
		for (int label = 0; label < OPTIONSLABELCOUNT; ++label) {
			m_aOptionsLabel[label] = 0;
		}
		for (int checkbox = 0; checkbox < OPTIONSCHECKBOXCOUNT; ++checkbox) {
			m_aOptionsCheckbox[checkbox] = 0;
		}
		m_pResolutionCombo = 0;
		m_pSoundLabel = 0;
		m_pMusicLabel = 0;
		m_pSoundSlider = 0;
		m_pMusicSlider = 0;

		// Config screen
		m_ConfiguringPlayer = Players::PlayerOne;
		m_ConfiguringDevice = DEVICE_KEYB_ONLY;
		m_ConfiguringGamepad = DPAD;
		m_ConfigureStep = 0;
		for (int label = 0; label < CONFIGLABELCOUNT; ++label) {
			m_pConfigLabel[label] = 0;
		}

		m_aDPadBitmaps = 0;
		m_aDualAnalogBitmaps = 0;
		m_pRecommendationBox = 0;
		m_pRecommendationDiagram = 0;
		m_pConfigSkipButton = 0;
		m_pConfigBackButton = 0;
		m_pDPadTypeBox = 0;
		m_pDAnalogTypeBox = 0;
		m_pXBox360TypeBox = 0;
		m_pDPadTypeDiagram = 0;
		m_pDAnalogTypeDiagram = 0;
		m_pXBox360TypeDiagram = 0;
		m_pDPadTypeButton = 0;
		m_pDAnalogTypeButton = 0;
		m_pXBox360TypeButton = 0;

		m_ResolutionChangeDialog = nullptr;
		m_ButtonConfirmResolutionChange = nullptr;
		m_ButtonConfirmResolutionChangeFullscreen = nullptr;
		m_ButtonCancelResolutionChange = nullptr;
		m_ResolutionChangeToUpscaled = false;

		m_MaxResX = 0;
		m_MaxResY = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

create {
		m_aOptionButton[UPSCALEDFULLSCREEN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonUpscaledFullscreen"));
		m_aOptionButton[P1NEXT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1NextDevice"));
		m_aOptionButton[P2NEXT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2NextDevice"));
		m_aOptionButton[P3NEXT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3NextDevice"));
		m_aOptionButton[P4NEXT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4NextDevice"));
		m_aOptionButton[P1PREV] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1PrevDevice"));
		m_aOptionButton[P2PREV] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2PrevDevice"));
		m_aOptionButton[P3PREV] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3PrevDevice"));
		m_aOptionButton[P4PREV] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4PrevDevice"));
		m_aOptionButton[P1CONFIG] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1Config"));
		m_aOptionButton[P2CONFIG] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2Config"));
		m_aOptionButton[P3CONFIG] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3Config"));
		m_aOptionButton[P4CONFIG] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4Config"));
		m_aOptionButton[P1CLEAR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1Clear"));
		m_aOptionButton[P2CLEAR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2Clear"));
		m_aOptionButton[P3CLEAR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3Clear"));
		m_aOptionButton[P4CLEAR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4Clear"));

		m_aOptionsLabel[P1DEVICE] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP1Device"));
		m_aOptionsLabel[P2DEVICE] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP2Device"));
		m_aOptionsLabel[P3DEVICE] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP3Device"));
		m_aOptionsLabel[P4DEVICE] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP4Device"));

		m_aDeadZoneLabel[P1DEADZONESLIDER] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP1DeadZoneValue"));
		m_aDeadZoneLabel[P2DEADZONESLIDER] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP2DeadZoneValue"));
		m_aDeadZoneLabel[P3DEADZONESLIDER] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP3DeadZoneValue"));
		m_aDeadZoneLabel[P4DEADZONESLIDER] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelP4DeadZoneValue"));

		// Set slider values
		m_aDeadZoneSlider[P1DEADZONESLIDER] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderP1DeadZone"));
		m_aDeadZoneSlider[P2DEADZONESLIDER] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderP2DeadZone"));
		m_aDeadZoneSlider[P3DEADZONESLIDER] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderP3DeadZone"));
		m_aDeadZoneSlider[P4DEADZONESLIDER] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderP4DeadZone"));

		m_aDeadZoneSlider[P1DEADZONESLIDER]->SetValue(g_UInputMan.GetControlScheme(0)->GetJoystickDeadzone() * 250);
		m_aDeadZoneSlider[P2DEADZONESLIDER]->SetValue(g_UInputMan.GetControlScheme(1)->GetJoystickDeadzone() * 250);
		m_aDeadZoneSlider[P3DEADZONESLIDER]->SetValue(g_UInputMan.GetControlScheme(2)->GetJoystickDeadzone() * 250);
		m_aDeadZoneSlider[P4DEADZONESLIDER]->SetValue(g_UInputMan.GetControlScheme(3)->GetJoystickDeadzone() * 250);

		// Set value labels
		char s[256];

		std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[P1DEADZONESLIDER]->GetValue());
		m_aDeadZoneLabel[P1DEADZONESLIDER]->SetText(s);
		std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[P2DEADZONESLIDER]->GetValue());
		m_aDeadZoneLabel[P2DEADZONESLIDER]->SetText(s);
		std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[P3DEADZONESLIDER]->GetValue());
		m_aDeadZoneLabel[P3DEADZONESLIDER]->SetText(s);
		std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[P4DEADZONESLIDER]->GetValue());
		m_aDeadZoneLabel[P4DEADZONESLIDER]->SetText(s);

		// Set deadzone checkboxes
		m_aDeadZoneCheckbox[P1DEADZONESLIDER] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("CheckboxP1DeadZoneType"));
		m_aDeadZoneCheckbox[P2DEADZONESLIDER] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("CheckboxP2DeadZoneType"));
		m_aDeadZoneCheckbox[P3DEADZONESLIDER] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("CheckboxP3DeadZoneType"));
		m_aDeadZoneCheckbox[P4DEADZONESLIDER] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("CheckboxP4DeadZoneType"));



		int dztype = 0;
		char str[2];
		str[0] = -2;
		str[1] = 0;

		dztype = g_UInputMan.GetControlScheme(0)->GetJoystickDeadzoneType();
		if (dztype == DeadZoneType::CIRCLE) {
			m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetCheck(1);
			m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetText("O");
		} else if (dztype == DeadZoneType::SQUARE) {
			m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetCheck(0);
			m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetText(str);
		}

		dztype = g_UInputMan.GetControlScheme(1)->GetJoystickDeadzoneType();
		if (dztype == DeadZoneType::CIRCLE) {
			m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetCheck(1);
			m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetText("O");
		} else if (dztype == DeadZoneType::SQUARE) {
			m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetCheck(0);
			m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetText(str);
		}

		dztype = g_UInputMan.GetControlScheme(2)->GetJoystickDeadzoneType();
		if (dztype == DeadZoneType::CIRCLE) {
			m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetCheck(1);
			m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetText("O");
		} else if (dztype == DeadZoneType::SQUARE) {
			m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetCheck(0);
			m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetText(str);
		}

		dztype = g_UInputMan.GetControlScheme(3)->GetJoystickDeadzoneType();
		if (dztype == DeadZoneType::CIRCLE) {
			m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetCheck(1);
			m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetText("O");
		} else if (dztype == DeadZoneType::SQUARE) {
			m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetCheck(0);
			m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetText(str);
		}

		m_aOptionsCheckbox[FLASHONBRAINDAMAGE] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("FlashOnBrainDamageCheckbox"));
		m_aOptionsCheckbox[FLASHONBRAINDAMAGE]->SetCheck(g_SettingsMan.FlashOnBrainDamage());
		m_aOptionsCheckbox[BLIPONREVEALUNSEEN] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("BlipOnRevealUnseenCheckbox"));
		m_aOptionsCheckbox[BLIPONREVEALUNSEEN]->SetCheck(g_SettingsMan.BlipOnRevealUnseen());
		m_aOptionsCheckbox[SHOWFOREIGNITEMS] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("ShowForeignItemsCheckbox"));
		m_aOptionsCheckbox[SHOWFOREIGNITEMS]->SetCheck(g_SettingsMan.ShowForeignItems());
		m_aOptionsCheckbox[SHOWTOOLTIPS] = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("ShowToolTipsCheckbox"));
		m_aOptionsCheckbox[SHOWTOOLTIPS]->SetCheck(g_SettingsMan.ToolTips());

		m_pResolutionCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ComboResolution"));
		UpdateResolutionCombo();

		m_pSoundLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelSoundVolume"));
		m_pMusicLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelMusicVolume"));
		m_pSoundSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderSoundVolume"));
		m_pMusicSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SliderMusicVolume"));
		UpdateVolumeSliders();

		UpdateDeviceLabels();

		m_pBackToOptionsButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonBackToOptions"));
		m_pBackToOptionsButton->SetVisible(false);

		// Config screen controls
		m_pConfigLabel[CONFIGTITLE] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelConfigTitle"));
		m_pConfigLabel[CONFIGRECOMMENDATION] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelConfigRecKeyDesc"));
		m_pConfigLabel[CONFIGSTEPS] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelConfigStep"));
		m_pConfigLabel[CONFIGINSTRUCTION] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelConfigInstruction"));
		m_pConfigLabel[CONFIGINPUT] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelConfigInput"));
		ContentFile diagramFile("Base.rte/GUIs/Controllers/D-Pad.png");
		m_aDPadBitmaps = diagramFile.GetAsAnimation(DPADSTEPS, COLORCONV_8_TO_32);
		diagramFile.SetDataPath("Base.rte/GUIs/Controllers/DualAnalog.png");
		m_aDualAnalogBitmaps = diagramFile.GetAsAnimation(DANALOGSTEPS, COLORCONV_8_TO_32);
		m_pRecommendationBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigRec"));
		m_pRecommendationDiagram = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigRecDiagram"));
		m_pConfigSkipButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfigSkip"));
		m_pConfigBackButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfigBack"));



		// Resolution change dialog
		m_ResolutionChangeDialog = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ResolutionChangeDialog"));
		m_ResolutionChangeDialog->CenterInParent(true, true);
		m_ResolutionChangeDialog->SetVisible(false);
		m_ButtonConfirmResolutionChange = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfirmResolutionChange"));
		m_ButtonConfirmResolutionChange->SetVisible(false);
		m_ButtonConfirmResolutionChangeFullscreen = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfirmResolutionChangeFullscreen"));
		m_ButtonConfirmResolutionChangeFullscreen->SetVisible(false);
		m_ButtonCancelResolutionChange = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonCancelResolutionChange"));

		m_aOptionButton[FULLSCREENORWINDOWED] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonFullscreen"));
		if (g_FrameMan.GetResX() * g_FrameMan.ResolutionMultiplier() == m_MaxResX && g_FrameMan.GetResY() * g_FrameMan.ResolutionMultiplier() == m_MaxResY) {
			m_aOptionButton[FULLSCREENORWINDOWED]->SetText("Windowed");
		} else {
			m_aOptionButton[FULLSCREENORWINDOWED]->SetText("Fullscreen");
		}

		m_pDPadTypeBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigDPadType"));
		m_pDAnalogTypeBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigDAnalogType"));
		m_pXBox360TypeBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigXBox360Type"));
		m_pDPadTypeDiagram = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigDPadTypeDiagram"));
		m_pDAnalogTypeDiagram = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigDAnalogTypeDiagram"));
		m_pXBox360TypeDiagram = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BoxConfigXBox360TypeDiagram"));
		m_pDPadTypeButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfigDPadType"));
		m_pDAnalogTypeButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfigDAnalogType"));
		m_pXBox360TypeButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonConfigXBox360Type"));

		// Put the image in and resize appropriately
		m_pDPadTypeDiagram->Resize(m_aDPadBitmaps[0]->w, m_aDPadBitmaps[0]->h);
		m_pDPadTypeDiagram->CenterInParent(true, true);
		m_pDPadTypeDiagram->MoveRelative(0, -8);
		m_pDAnalogTypeDiagram->Resize(m_aDualAnalogBitmaps[0]->w, m_aDualAnalogBitmaps[0]->h);
		m_pDAnalogTypeDiagram->CenterInParent(true, true);
		m_pDAnalogTypeDiagram->MoveRelative(0, -10);
		m_pXBox360TypeDiagram->Resize(m_aDualAnalogBitmaps[0]->w, m_aDualAnalogBitmaps[0]->h);
		m_pXBox360TypeDiagram->CenterInParent(true, true);
		m_pXBox360TypeDiagram->MoveRelative(0, -10);



}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

destroy {
		// Delete only the array, not the bitmaps themselves, we don't own them
		delete[] m_aDPadBitmaps;
		delete[] m_aDualAnalogBitmaps;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

update {
		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent)) {
			// Commands
			if (anEvent.GetType() == GUIEvent::Command) {
				// Fullscreen/windowed toggle button pressed
				if (anEvent.GetControl() == m_aOptionButton[FULLSCREENORWINDOWED]) {
					g_GUISound.ButtonPressSound()->Play();

					if (!g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
						if (g_ActivityMan.GetActivity()) {
							m_ResolutionChangeToUpscaled = false;
							m_ResolutionChangeDialog->SetVisible(true);
							m_apScreenBox[OPTIONSSCREEN]->SetEnabled(false);
							m_MainMenuButtons[BACKTOMAIN]->SetEnabled(false);
							m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
						} else {
							HideAllScreens();
							m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
							g_FrameMan.SwitchToFullscreen(false);
						}
					} else if (g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
						if (g_ActivityMan.GetActivity()) {
							m_ResolutionChangeToUpscaled = false;
							m_ResolutionChangeDialog->SetVisible(true);
							m_apScreenBox[OPTIONSSCREEN]->SetEnabled(false);
							m_MainMenuButtons[BACKTOMAIN]->SetEnabled(false);
							m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
						} else {
							HideAllScreens();
							m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
							g_FrameMan.SwitchResolution(960, 540);
						}
					} else if (!g_FrameMan.IsFullscreen() && g_FrameMan.IsUpscaledFullscreen()) {
						g_FrameMan.SwitchResolutionMultiplier(1);
					}
					UpdateResolutionCombo();
				}

				// Upscaled fullscreen button pressed
				if (anEvent.GetControl() == m_aOptionButton[UPSCALEDFULLSCREEN]) {
					g_GUISound.ButtonPressSound()->Play();

					if (!g_FrameMan.IsUpscaledFullscreen()) {
						if (g_ActivityMan.GetActivity()) {
							m_ResolutionChangeToUpscaled = true;
							m_ResolutionChangeDialog->SetVisible(true);
							m_apScreenBox[OPTIONSSCREEN]->SetEnabled(false);
							m_MainMenuButtons[BACKTOMAIN]->SetEnabled(false);
							m_ButtonConfirmResolutionChangeFullscreen->SetVisible(true);
						} else {
							HideAllScreens();
							m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
							g_FrameMan.SwitchToFullscreen(true);
						}
					}
					UpdateResolutionCombo();
				}

				if (anEvent.GetControl() == m_ButtonConfirmResolutionChangeFullscreen) {
					g_GUISound.ButtonPressSound()->Play();
					HideAllScreens();
					m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
					m_MainMenuButtons[BACKTOMAIN]->SetEnabled(true);
					m_ResolutionChangeDialog->SetVisible(false);
					m_apScreenBox[OPTIONSSCREEN]->SetEnabled(true);
					m_ButtonConfirmResolutionChangeFullscreen->SetVisible(false);
					if (!m_ResolutionChangeToUpscaled && g_FrameMan.IsFullscreen() && !g_FrameMan.IsUpscaledFullscreen()) {
						g_FrameMan.SwitchResolution(960, 540, 1, true);
					} else {
						g_FrameMan.SwitchToFullscreen(m_ResolutionChangeToUpscaled ? true : false, true);
					}
					UpdateResolutionCombo();
				}

				if (anEvent.GetControl() == m_ButtonConfirmResolutionChange) {
					g_GUISound.ButtonPressSound()->Play();
					HideAllScreens();
					m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
					m_MainMenuButtons[BACKTOMAIN]->SetEnabled(true);
					m_ResolutionChangeDialog->SetVisible(false);
					m_apScreenBox[OPTIONSSCREEN]->SetEnabled(true);
					m_ButtonConfirmResolutionChange->SetVisible(false);
					g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1, true);
					UpdateResolutionCombo();
				}

				// Update the label to whatever we ended up with
				if (g_FrameMan.GetResX() * g_FrameMan.ResolutionMultiplier() == m_MaxResX && g_FrameMan.GetResY() * g_FrameMan.ResolutionMultiplier() == m_MaxResY) {
					m_aOptionButton[FULLSCREENORWINDOWED]->SetText("Windowed");
				} else {
					m_aOptionButton[FULLSCREENORWINDOWED]->SetText("Fullscreen");
				}

				if (anEvent.GetControl() == m_ButtonCancelResolutionChange) {
					g_GUISound.ButtonPressSound()->Play();
					m_ResolutionChangeDialog->SetVisible(false);
					m_apScreenBox[OPTIONSSCREEN]->SetEnabled(true);
					m_MainMenuButtons[BACKTOMAIN]->SetEnabled(true);
				}

				// Return to main menu button pressed
				if (anEvent.GetControl() == m_MainMenuButtons[BACKTOMAIN]) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);

					// If leaving the options screen, save the settings!
					if (m_MenuScreen == OPTIONSSCREEN) {
						g_SettingsMan.SetFlashOnBrainDamage(m_aOptionsCheckbox[FLASHONBRAINDAMAGE]->GetCheck());
						g_SettingsMan.SetBlipOnRevealUnseen(m_aOptionsCheckbox[BLIPONREVEALUNSEEN]->GetCheck());
						g_SettingsMan.SetShowForeignItems(m_aOptionsCheckbox[SHOWFOREIGNITEMS]->GetCheck());
						g_SettingsMan.SetShowToolTips(m_aOptionsCheckbox[SHOWTOOLTIPS]->GetCheck());

						g_SettingsMan.UpdateSettingsFile();
					}

					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;

					g_GUISound.BackButtonPressSound()->Play();
				}

				// Return to options menu button pressed
				if (anEvent.GetControl() == m_pBackToOptionsButton) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_pBackToOptionsButton->SetVisible(false);
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
						if (anEvent.GetControl() == m_aOptionButton[which]) {
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
						if (anEvent.GetControl() == m_aOptionButton[which]) {
							m_apScreenBox[OPTIONSSCREEN]->SetVisible(false);
							m_ConfiguringPlayer = which - P1CONFIG;
							m_ConfiguringDevice = g_UInputMan.GetControlScheme(m_ConfiguringPlayer)->GetDevice();
							m_ConfigureStep = 0;
							m_MenuScreen = CONFIGSCREEN;
							m_ScreenChange = true;

							g_GUISound.ButtonPressSound()->Play();
						}
					}

					// Handle all control reset buttons
					for (which = P1CLEAR; which <= P4CLEAR; ++which) {
						// Handle the appropriate player's clearing of mappings
						if (anEvent.GetControl() == m_aOptionButton[which]) {
							// Make user click twice to confirm
							if (m_aOptionButton[which]->GetText() == "Reset") {
								// Ask to confirm!
								m_aOptionButton[which]->SetText("CONFIRM?");
								// And clear all other buttons of it
								for (int otherButton = P1CLEAR; otherButton <= P4CLEAR; ++otherButton) {
									if (otherButton != which) { m_aOptionButton[otherButton]->SetText("Reset"); }
								}
								g_GUISound.ButtonPressSound()->Play();
							} else {
								// Set to a default control preset.
								Players inputPlayer = static_cast<Players>(which - P1CLEAR);
								InputPreset playerPreset = static_cast<InputPreset>(P1CLEAR - which - 1); // Player 1's default preset is at -1 and so on.
								g_UInputMan.GetControlScheme(inputPlayer)->SetPreset(playerPreset);

								// Set to a device that fits this preset.
								InputDevice deviceType[4] = { DEVICE_MOUSE_KEYB, DEVICE_KEYB_ONLY, DEVICE_GAMEPAD_1, DEVICE_GAMEPAD_2 };
								g_UInputMan.GetControlScheme(inputPlayer)->SetDevice(deviceType[inputPlayer]);

								UpdateDeviceLabels();

								// Set the dead zone slider value
								m_aDeadZoneSlider[which - P1CLEAR]->SetValue(g_UInputMan.GetControlScheme(which - P1CLEAR)->GetJoystickDeadzone() * 250);

								//                            m_aOptionsLabel[P1DEVICE + (which - P1CLEAR)]->SetText("NEEDS CONFIG!");
								//                            m_aOptionButton[P1CONFIG + (which - P1CLEAR)]->SetText("-> CONFIGURE <-");
								g_GUISound.ExitMenuSound()->Play();
							}
						}
					}
				}

				//////////////////////////////////
				// Control config buttons

				if (m_MenuScreen == CONFIGSCREEN) {
					// DPad Gamepad type selected
					if (anEvent.GetControl() == m_pDPadTypeButton) {
						m_ConfiguringGamepad = DPAD;
						m_ConfigureStep++;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}

					// DPad Gamepad type selected
					if (anEvent.GetControl() == m_pDAnalogTypeButton) {
						m_ConfiguringGamepad = DANALOG;
						m_ConfigureStep++;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}

					// XBox Gamepad type selected
					if (anEvent.GetControl() == m_pXBox360TypeButton) {
						// Not allowing config, this is a complete preset
						//					m_ConfiguringGamepad = XBOX360;
						//					m_ConfigureStep++;
						//					m_ScreenChange = true;
						//
						// Set up the preset that will work well for a 360 controller
						g_UInputMan.GetControlScheme(m_ConfiguringPlayer)->SetPreset(PRESET_XBOX360);

						// Go back to the options screen immediately since the preset is all taken care of
						m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
						m_MenuScreen = OPTIONSSCREEN;
						m_ScreenChange = true;

						g_GUISound.ExitMenuSound()->Play();
					}

					// Skip ahead one config step button pressed
					if (anEvent.GetControl() == m_pConfigSkipButton) {
						// TODO: error checking so that we don't put configurestep out of bounds!
						m_ConfigureStep++;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}

					// Go back one config step button pressed
					if (anEvent.GetControl() == m_pConfigBackButton) {
						if (m_ConfigureStep > 0) {
							m_ConfigureStep--;
							m_ScreenChange = true;
							g_GUISound.BackButtonPressSound()->Play();
						} else {
							g_GUISound.UserErrorSound()->Play();
						}
					}
				}


			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification) {


				// Resolution combobox closed, something new selected
				if (anEvent.GetControl() == m_pResolutionCombo) {
					// Closed it, IE selected somehting
					if (anEvent.GetMsg() == GUIComboBox::Closed) {
						// Get and read the new resolution data from the item's label
						GUIListPanel::Item *pResItem = m_pResolutionCombo->GetItem(m_pResolutionCombo->GetSelectedIndex());
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
								m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
								g_FrameMan.SwitchResolution(g_FrameMan.GetNewResX(), g_FrameMan.GetNewResY(), 1);
							}
						}
					}
				}

				// Sound Volume slider changed
				if (anEvent.GetControl() == m_pSoundSlider) {
					// See if we should play test sound after the volume has been set
					bool playTest = false;
					if (((double)m_pSoundSlider->GetValue() / 100) != g_AudioMan.GetSoundsVolume() && !g_GUISound.TestSound()->IsBeingPlayed()) { playTest = true; }

					g_AudioMan.SetSoundsVolume((double)m_pSoundSlider->GetValue() / 100);
					UpdateVolumeSliders();

					// Play test sound after new volume is set
					if (playTest) { g_GUISound.TestSound()->Play(); }
				}

				// Music Volume slider changed
				if (anEvent.GetControl() == m_pMusicSlider) {
					g_AudioMan.SetMusicVolume((double)m_pMusicSlider->GetValue() / 100);
					UpdateVolumeSliders();
				}

				// Dead zone sliders control
				for (int which = P1DEADZONESLIDER; which < DEADZONESLIDERCOUNT; ++which) {
					// Handle the appropriate player's clearing of mappings
					if (anEvent.GetControl() == m_aDeadZoneSlider[which]) {
						// Display value
						char s[256];
						std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[which]->GetValue());
						m_aDeadZoneLabel[which]->SetText(s);

						// Update control scheme
						g_UInputMan.GetControlScheme(which)->SetJoystickDeadzone((float)m_aDeadZoneSlider[which]->GetValue() / 200.0F);
					}

					if (anEvent.GetControl() == m_aDeadZoneCheckbox[which]) {
						if (m_aDeadZoneCheckbox[which]->GetCheck() == 1) {
							g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::CIRCLE);
							m_aDeadZoneCheckbox[which]->SetText("O");
						} else {
							g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::SQUARE);
							char str[2];
							str[0] = -2;
							str[1] = 0;
							m_aDeadZoneCheckbox[which]->SetText(str);
						}
					}
				}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateResolutionCombo() {
		// Refill possible resolutions
		m_pResolutionCombo->SetText("");
		m_pResolutionCombo->ClearList();

		if (m_pResolutionCombo->GetCount() <= 0) {
			// Get a list of modes from the fullscreen driver even though we're not using it. This is so we don't need to populate the list manually and has all the reasonable resolutions.
#ifdef _WIN32
			GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_DIRECTX_ACCEL);
#elif __unix__
			GFX_MODE_LIST *resList = get_gfx_mode_list(GFX_XWINDOWS_FULLSCREEN);
#endif
			int width = 0;
			int height = 0;
			std::string resString = "";
			// Index of found useful resolution (32bit)
			int foundIndex = 0;
			int currentResIndex = -1;

			// Process and annotate the list
			for (int i = 0; resList && i < resList->num_modes; ++i) {
				// Only list 32 bpp modes
				if (resList->mode[i].bpp == 32) {
					width = resList->mode[i].width;
					height = resList->mode[i].height;

					// Resolutions must be multiples of 4 or we'll get 'Overlays not supported' during GFX mode init
					if (g_FrameMan.IsValidResolution(width, height) && width % 4 == 0) {
						// Fix wacky resolutions that are taller than wide
						if (height > width) {
							height = resList->mode[i].width;
							width = resList->mode[i].height;
						}
						// Try to figure the max available resolution
						if (width > m_MaxResX) {
							m_MaxResX = width;
							m_MaxResY = height;
						}
						resString = std::to_string(width) + "x" + std::to_string(height);

						// Add useful notation to the standardized resolutions
						if (width == 800 && height == 600) { resString += " SVGA"; }
						if (width == 1024 && height == 600) { resString += " WSVGA"; }
						if (width == 1024 && height == 768) { resString += " XGA"; }
						if (width == 1280 && height == 720) { resString += " HD"; }
						if (width == 1280 && (height == 768 || height == 800)) { resString += " WXGA"; }
						if (width == 1280 && height == 1024) { resString += " SXGA"; }
						if (width == 1400 && height == 1050) { resString += " SXGA+"; }
						if (width == 1600 && height == 900) { resString += " HD+"; }
						if (width == 1600 && height == 1200) { resString += " UGA"; }
						if (width == 1680 && height == 1050) { resString += " WSXGA+"; }
						if (width == 1920 && height == 1080) { resString += " FHD"; }
						if (width == 1920 && height == 1200) { resString += " WUXGA"; }
						if (width == 2048 && height == 1080) { resString += " DCI 2K"; }
						if (width == 2560 && height == 1440) { resString += " QHD"; }
						if (width == 3200 && height == 1800) { resString += " QHD+"; }
						if (width == 3840 && height == 2160) { resString += " 4K UHD"; }
						if (width == 4096 && height == 2160) { resString += " DCI 4K"; }

						m_pResolutionCombo->AddItem(resString);

						// If this is what we're currently set to have at next start, select it afterward
						if ((g_FrameMan.GetNewResX() * g_FrameMan.ResolutionMultiplier()) == width && (g_FrameMan.GetNewResY() * g_FrameMan.ResolutionMultiplier()) == height) { currentResIndex = foundIndex; }
						// Only increment this when we find a usable 32bit resolution
						foundIndex++;
					}
				}
			}
			if (resList) { destroy_gfx_mode_list(resList); }

			// If none of the listed matched our resolution set for next start, add a 'custom' one to display as the current res
			if (currentResIndex < 0) {
				std::string isUpscaled = (g_FrameMan.ResolutionMultiplier() > 1) ? " Upscaled" : " Custom";
				resString = std::to_string(g_FrameMan.GetResX() / g_FrameMan.ResolutionMultiplier()) + "x" + std::to_string(g_FrameMan.GetResY() / g_FrameMan.ResolutionMultiplier()) + isUpscaled;
				m_pResolutionCombo->AddItem(resString);
				currentResIndex = m_pResolutionCombo->GetCount() - 1;
			}
			// Show the current resolution item to be the selected one
			m_pResolutionCombo->SetSelectedIndex(currentResIndex);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateVolumeSliders() {
		char labelText[512];
		int volume = static_cast<int>(std::round(g_AudioMan.GetSoundsVolume() * 100));
		std::snprintf(labelText, sizeof(labelText), "Sound Volume: %i", volume);
		m_pSoundLabel->SetText(labelText);
		m_pSoundSlider->SetValue(volume);

		volume = static_cast<int>(std::round(g_AudioMan.GetMusicVolume() * 100));
		std::snprintf(labelText, sizeof(labelText), "Music Volume: %i", volume);
		m_pMusicLabel->SetText(labelText);
		m_pMusicSlider->SetValue(volume);
	}

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
			m_aOptionsLabel[P1DEVICE + player]->SetText(label);

			// Reset Config and Clear button labels
			m_aOptionButton[P1CONFIG + player]->SetText("Configure");
			m_aOptionButton[P1CLEAR + player]->SetText("Reset");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::UpdateConfigScreen() {
		char str[256];

		if (m_ScreenChange) {
			// Hide most things first, enable as needed
			m_pConfigLabel[CONFIGINSTRUCTION]->SetVisible(false);
			m_pConfigLabel[CONFIGINPUT]->SetVisible(false);
			m_pRecommendationBox->SetVisible(false);
			m_pRecommendationDiagram->SetVisible(false);
			m_pConfigLabel[CONFIGSTEPS]->SetVisible(false);
			m_pConfigSkipButton->SetVisible(false);
			m_pConfigBackButton->SetVisible(false);
			m_pDPadTypeBox->SetVisible(false);
			m_pDAnalogTypeBox->SetVisible(false);
			m_pXBox360TypeBox->SetVisible(false);
		}

		// [CHRISK] Use GUI input class for better key detection
		g_UInputMan.SetInputClass(m_pGUIInput);

		// Keyboard screens
		if (m_ConfiguringDevice == DEVICE_KEYB_ONLY) {
			if (m_ScreenChange) {
				m_pConfigLabel[CONFIGINSTRUCTION]->SetVisible(true);
				m_pConfigLabel[CONFIGINPUT]->SetVisible(true);
				std::snprintf(str, sizeof(str), "Keyboard Configuration - Player %i", m_ConfiguringPlayer + 1);
				m_pConfigLabel[CONFIGTITLE]->SetText(str);
				m_pConfigLabel[CONFIGINSTRUCTION]->SetText("Press the key for");
				m_pConfigLabel[CONFIGSTEPS]->SetVisible(true);
				m_pRecommendationBox->SetVisible(true);
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetVisible(true);
				m_pConfigSkipButton->SetVisible(true);
				m_pConfigBackButton->SetVisible(true);
				m_ScreenChange = false;
			}

			// Step label update
			std::snprintf(str, sizeof(str), "Step %i / %i", m_ConfigureStep + 1, KEYBOARDSTEPS);
			m_pConfigLabel[CONFIGSTEPS]->SetText(str);

			// Move/Aim up
			if (m_ConfigureStep == 0) {
				// Hide the back button on this first step
				m_pConfigBackButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM UP");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Up Cursor]" : "[W]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM_UP)) {
					g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_UP);
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Move/Aim down
			else if (m_ConfigureStep == 1) {
				m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM DOWN");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Down Cursor]" : "[S]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM_DOWN)) {
					g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_DOWN);
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Move left
			else if (m_ConfigureStep == 2) {
				m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Left Cursor]" : "[A]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_LEFT)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Move right
			else if (m_ConfigureStep == 3) {
				m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Right Cursor]" : "[D]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_RIGHT)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Fire
			else if (m_ConfigureStep == 4) {
				m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 1]" : "[H]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_FIRE))

				{
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Sharp aim
			else if (m_ConfigureStep == 5) {
				m_pConfigLabel[CONFIGINPUT]->SetText("SHARP AIM");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 2]" : "[J]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Inventory
			else if (m_ConfigureStep == 6) {
				m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 3]" : "[K]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PIEMENU)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Jump
			else if (m_ConfigureStep == 7) {
				m_pConfigLabel[CONFIGINPUT]->SetText("JUMP");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num Enter]" : "[L]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_JUMP)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Crouch
			else if (m_ConfigureStep == 8) {
				m_pConfigLabel[CONFIGINPUT]->SetText("CROUCH");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num Del]" : "[.]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_CROUCH)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Prev actor
			else if (m_ConfigureStep == 9) {
				m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 4]" : "[Q]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PREV)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Next actor
			else if (m_ConfigureStep == 10) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 5]" : "[E]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_NEXT)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			}
			// Reload
			else if (m_ConfigureStep == 11) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 0]" : "[R]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			} else if (m_ConfigureStep == 12) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("PICK UP");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 9]" : "[F]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			} else if (m_ConfigureStep == 13) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("DROP");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 6]" : "[G]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			} else if (m_ConfigureStep == 14) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS WEAPON");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 7]" : "[X]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					m_ConfigureStep++;
					m_ScreenChange = true;
				}
			} else if (m_ConfigureStep == 15) {
				// Hide skip button on this last step
				m_pConfigSkipButton->SetVisible(false);

				m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
				m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 8]" : "[C]");
				if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
					m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;
				}
			}

			//
			//		// Start
			//		else if (m_ConfigureStep == 10)
			//		{
			//			m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
			//			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_START))
			//			{
			//				m_ConfigureStep++;
			//				m_ScreenChange = true;
			//			}
			//		}
			//		// Back
			//		else if (m_ConfigureStep == 11)
			//		{
			//			// Hide the skip button on this last step
			//			m_pConfigSkipButton->SetVisible(false);
			//			m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
			//			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_BACK))
			//			{
			//				// Done, go back to options screen
			//				m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
			//				m_MenuScreen = OPTIONSSCREEN;
			//				m_ScreenChange = true;
			//			}
			//		}
			//
		}
		// Mouse+keyb
		else if (m_ConfiguringDevice == DEVICE_MOUSE_KEYB) {
		if (m_ScreenChange) {
			m_pConfigLabel[CONFIGINSTRUCTION]->SetVisible(true);
			m_pConfigLabel[CONFIGINPUT]->SetVisible(true);
			std::snprintf(str, sizeof(str), "Mouse + Keyboard Configuration - Player %i", m_ConfiguringPlayer + 1);
			m_pConfigLabel[CONFIGTITLE]->SetText(str);
			m_pConfigLabel[CONFIGINSTRUCTION]->SetText("Press the key for");
			m_pConfigLabel[CONFIGSTEPS]->SetVisible(true);
			m_pRecommendationBox->SetVisible(true);
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetVisible(true);
			m_pConfigSkipButton->SetVisible(true);
			m_pConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", m_ConfigureStep + 1, MOUSESTEPS);
		m_pConfigLabel[CONFIGSTEPS]->SetText(str);

		// Move up
		if (m_ConfigureStep == 0) {
			// Hide the back button on this first step
			m_pConfigBackButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("MOVE UP or JUMP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[W]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_UP)) {
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_UP);
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_JUMP);
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Move down
		else if (m_ConfigureStep == 1) {
			m_pConfigLabel[CONFIGINPUT]->SetText("MOVE DOWN or CROUCH");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[S]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_DOWN)) {
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_DOWN);
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_CROUCH);
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Move left
		else if (m_ConfigureStep == 2) {
			m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[A]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_LEFT)) {
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_LEFT);
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Move right
		else if (m_ConfigureStep == 3) {
			m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[D]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_RIGHT)) {
				g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_RIGHT);
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Move right
		else if (m_ConfigureStep == 4) {
			m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[R]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		} else if (m_ConfigureStep == 5) {
			m_pConfigLabel[CONFIGINPUT]->SetText("PICK UP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[F]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		} else if (m_ConfigureStep == 6) {
			m_pConfigLabel[CONFIGINPUT]->SetText("DROP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[G]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_DROP)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		} else if (m_ConfigureStep == 7) {
			m_pConfigLabel[CONFIGINPUT]->SetText("PREV WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[X]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		} else if (m_ConfigureStep == 8) {
			m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[C]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Prev actor
		else if (m_ConfigureStep == 9) {
			m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[Q]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PREV)) {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Next actor
		else if (m_ConfigureStep == 10) {
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[E]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_NEXT)) {
				//                m_ConfigureStep++;
				//                m_ScreenChange = true;
								// Done, go back to options screen
				m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
				m_MenuScreen = OPTIONSSCREEN;
				m_ScreenChange = true;
			}
		}
		}
		// Gamepad screens
		else if (m_ConfiguringDevice >= DEVICE_GAMEPAD_1 && m_ConfiguringDevice <= DEVICE_GAMEPAD_4) {
		int whichJoy = m_ConfiguringDevice - DEVICE_GAMEPAD_1;
		AllegroBitmap *pDiagramBitmap = 0;

		// Choose which gamepad type - special first step
		if (m_ConfigureStep == 0) {
			// Set title
			std::snprintf(str, sizeof(str), "Choose Gamepad Type for Player %i:", m_ConfiguringPlayer + 1);
			m_pConfigLabel[CONFIGTITLE]->SetText(str);

			// Hide the back button on this first step
			m_pConfigBackButton->SetVisible(false);

			// Show the type option boxes
			m_pDPadTypeBox->SetVisible(true);
			m_pDAnalogTypeBox->SetVisible(true);
			m_pXBox360TypeBox->SetVisible(true);

			// Not passing in ownership of the BITMAP
			pDiagramBitmap = new AllegroBitmap(m_aDPadBitmaps[0]);
			// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
			m_pDPadTypeDiagram->SetDrawImage(pDiagramBitmap);
			pDiagramBitmap = 0;

			// Not passing in ownership of the BITMAP
			pDiagramBitmap = new AllegroBitmap(m_aDualAnalogBitmaps[0]);
			// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
			m_pDAnalogTypeDiagram->SetDrawImage(pDiagramBitmap);
			// Doing it again for the 360 one.. it's similar enough looking
			pDiagramBitmap = new AllegroBitmap(m_aDualAnalogBitmaps[0]);
			m_pXBox360TypeDiagram->SetDrawImage(pDiagramBitmap);
			pDiagramBitmap = 0;

			// The special selection buttons take care of advancing the step, so do nothing else
			m_ScreenChange = false;
		}
		// Configure selected gamepad type
		else {
			if (m_ScreenChange) {
				m_pConfigLabel[CONFIGINSTRUCTION]->SetVisible(true);
				m_pConfigLabel[CONFIGINPUT]->SetVisible(true);
				m_pConfigLabel[CONFIGSTEPS]->SetVisible(true);
				m_pRecommendationBox->SetVisible(true);
				m_pConfigSkipButton->SetVisible(true);
				m_pConfigBackButton->SetVisible(true);
				m_BlinkTimer.Reset();
			}

			// D-pad
			if (m_ConfiguringGamepad == DPAD) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "D-Pad Gamepad Configuration - Player %i", m_ConfiguringPlayer + 1);
					m_pConfigLabel[CONFIGTITLE]->SetText(str);
					m_pConfigLabel[CONFIGRECOMMENDATION]->SetVisible(false);
					m_pConfigLabel[CONFIGINSTRUCTION]->SetText("Press the button or move the stick for");
					m_pRecommendationDiagram->SetVisible(true);
					m_pRecommendationDiagram->Resize(m_aDPadBitmaps[0]->w, m_aDPadBitmaps[0]->h);
					m_pRecommendationDiagram->CenterInParent(true, true);
					m_pRecommendationDiagram->MoveRelative(0, 4);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", m_ConfigureStep + 1, DPADSTEPS);
				m_pConfigLabel[CONFIGSTEPS]->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(m_aDPadBitmaps[m_BlinkTimer.AlternateReal(500) ? 0 : m_ConfigureStep]);
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				m_pRecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				// Move/Aim up
				if (m_ConfigureStep == 1) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM UP");

					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM_UP)) {
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_UP);
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_L_UP);
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_UP);
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_UP);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move/Aim down
				else if (m_ConfigureStep == 2) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM DOWN");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM_DOWN)) {
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_DOWN);
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_L_DOWN);
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_DOWN);
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_DOWN);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move left
				else if (m_ConfigureStep == 3) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_LEFT);
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_LEFT);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move right
				else if (m_ConfigureStep == 4) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_RIGHT);
						//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_RIGHT);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Sharp aim
				else if (m_ConfigureStep == 5) {
					m_pConfigLabel[CONFIGINPUT]->SetText("SHARP AIM");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Fire
				else if (m_ConfigureStep == 6) {
					m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Jump
				else if (m_ConfigureStep == 7) {
					m_pConfigLabel[CONFIGINPUT]->SetText("JUMP");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_JUMP)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// TODO: CROUCH???
								// Inventory
				else if (m_ConfigureStep == 8) {
					m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Next actor
				else if (m_ConfigureStep == 9) {
					m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Prev actor
				else if (m_ConfigureStep == 10) {
					m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PREV)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Start
				else if (m_ConfigureStep == 11) {
					m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_START)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Back
				else if (m_ConfigureStep == 12) {
					// Hide the skip button on this last step
					m_pConfigSkipButton->SetVisible(false);
					m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_BACK)) {
						// Done, go back to options screen
						m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
						m_MenuScreen = OPTIONSSCREEN;
						m_ScreenChange = true;
					}
				}
			}
			// Dual analog OR XBox Controller
			else if (m_ConfiguringGamepad == DANALOG || m_ConfiguringGamepad == XBOX360) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "Dual Analog Gamepad Configuration - Player %i", m_ConfiguringPlayer + 1);
					m_pConfigLabel[CONFIGTITLE]->SetText(str);
					m_pConfigLabel[CONFIGRECOMMENDATION]->SetVisible(false);
					m_pConfigLabel[CONFIGINSTRUCTION]->SetText("Press the button or move the stick for");
					m_pRecommendationDiagram->SetVisible(true);
					m_pRecommendationDiagram->Resize(m_aDualAnalogBitmaps[0]->w, m_aDualAnalogBitmaps[0]->h);
					m_pRecommendationDiagram->CenterInParent(true, true);
					m_pRecommendationDiagram->MoveRelative(0, 8);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", m_ConfigureStep + 1, DANALOGSTEPS);
				m_pConfigLabel[CONFIGSTEPS]->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(m_aDualAnalogBitmaps[m_BlinkTimer.AlternateReal(500) ? 0 : m_ConfigureStep]);
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				m_pRecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				// Move up
				if (m_ConfigureStep == 1) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE UP or JUMP");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_UP)) {
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_AIM_UP);
						g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_JUMP);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move down
				else if (m_ConfigureStep == 2) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE DOWN or CROUCH");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_DOWN)) {
						g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_AIM_DOWN);
						g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_CROUCH);
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move left
				else if (m_ConfigureStep == 3) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Move right
				else if (m_ConfigureStep == 4) {
					m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Aim up
				else if (m_ConfigureStep == 5) {
					m_pConfigLabel[CONFIGINPUT]->SetText("AIM UP");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_UP)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Aim down
				else if (m_ConfigureStep == 6) {
					m_pConfigLabel[CONFIGINPUT]->SetText("AIM DOWN");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_DOWN)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Aim left
				else if (m_ConfigureStep == 7) {
					m_pConfigLabel[CONFIGINPUT]->SetText("AIM LEFT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_LEFT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Aim right
				else if (m_ConfigureStep == 8) {
					m_pConfigLabel[CONFIGINPUT]->SetText("AIM RIGHT");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_RIGHT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Fire
				else if (m_ConfigureStep == 9) {
					m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Inventory
				else if (m_ConfigureStep == 10) {
					m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Next actor
				else if (m_ConfigureStep == 11) {
					m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Prev actor
				else if (m_ConfigureStep == 12) {
					m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PREV)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}

				// Prev weapon
				else if (m_ConfigureStep == 13) {
					m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS WEAPON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_PREV)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Next weapon
				else if (m_ConfigureStep == 14) {
					m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_NEXT)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Pickup weapon
				else if (m_ConfigureStep == 15) {
					m_pConfigLabel[CONFIGINPUT]->SetText("PICKUP WEAPON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_PICKUP)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Reload weapon
				else if (m_ConfigureStep == 16) {
					m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD WEAPON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_RELOAD)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Start
				else if (m_ConfigureStep == 17) {
					m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_START)) {
						m_ConfigureStep++;
						m_ScreenChange = true;
					}
				}
				// Back
				else if (m_ConfigureStep == 18) {
					// Hide the skip button on this last step
					m_pConfigSkipButton->SetVisible(false);
					m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
					if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_BACK)) {
						// If Xbox controller; if the A button has not been mapped to Activate/fire, then map it automatically
	// These redundancies should apply to all custom analog setups, really
	//                        if (m_ConfiguringGamepad == XBOX360)
						{
							// No button assigned to fire, so give it 'A' on the controller (in addition to any axis inputs)
							if (g_UInputMan.GetButtonMapping(m_ConfiguringPlayer, INPUT_FIRE) == JOY_NONE) { g_UInputMan.SetButtonMapping(m_ConfiguringPlayer, INPUT_FIRE, JOY_1); }
							// No button assigned to pie menu, so give it 'B' on the controller (in addition to whatever axis it's assinged to)
							if (g_UInputMan.GetButtonMapping(m_ConfiguringPlayer, INPUT_PIEMENU) == JOY_NONE) { g_UInputMan.SetButtonMapping(m_ConfiguringPlayer, INPUT_PIEMENU, JOY_2); }
						}

						// Done, go back to options screen
						m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
						m_MenuScreen = OPTIONSSCREEN;
						m_ScreenChange = true;
					}
				}
			}
		}
		}

		g_UInputMan.SetInputClass(NULL);

		if (m_ScreenChange) { g_GUISound.ExitMenuSound()->Play(); }
	}



*/