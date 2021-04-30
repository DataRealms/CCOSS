#include "SettingsInputMappingWizardGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SettingsInputMappingWizardGUI::SettingsInputMappingWizardGUI(GUIControlManager *parentControlManager) : m_GUIControlManager(parentControlManager) {
		m_BackToOptionsButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToOptions"));
		m_BackToOptionsButton->SetVisible(false);

		m_ConfigLabel.at(ConfigWizardLabels::ConfigTitle) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigTitle"));
		m_ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigRecKeyDesc"));
		m_ConfigLabel.at(ConfigWizardLabels::ConfigSteps) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigStep"));
		m_ConfigLabel.at(ConfigWizardLabels::ConfigInstruction) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigWizardLabels::ConfigInstruction"));
		m_ConfigLabel.at(ConfigWizardLabels::ConfigInput) = dynamic_cast<GUILabel *>(m_GUIControlManager->GetControl("LabelConfigInput"));

		ContentFile diagramFile("Base.rte/GUIs/Controllers/D-Pad.png");
		BITMAP **tempDPadBitmaps = diagramFile.GetAsAnimation(ConfigWizardSteps::DPadConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDPadBitmaps); ++i) {
			m_DPadBitmaps.at(i) = tempDPadBitmaps[i];
		}
		delete[] tempDPadBitmaps;

		diagramFile.SetDataPath("Base.rte/GUIs/Controllers/DualAnalog.png");
		BITMAP **tempDualAnalogBitmaps = diagramFile.GetAsAnimation(ConfigWizardSteps::DualAnalogConfigSteps, COLORCONV_8_TO_32);
		for (int i = 0; i < sizeof(tempDualAnalogBitmaps); ++i) {
			m_DualAnalogBitmaps.at(i) = tempDualAnalogBitmaps[i];
		}
		delete[] tempDualAnalogBitmaps;

		m_RecommendationBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRec"));
		m_RecommendationDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigRecDiagram"));
		m_ConfigSkipButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigSkip"));
		m_ConfigBackButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigBack"));

		m_DPadTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadType"));
		m_DPadTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDPadType"));
		m_DPadTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDPadTypeDiagram"));
		m_DPadTypeDiagram->Resize(m_DPadBitmaps.at(0)->w, m_DPadBitmaps.at(0)->h);
		m_DPadTypeDiagram->CenterInParent(true, true);
		m_DPadTypeDiagram->MoveRelative(0, -8);

		m_DAnalogTypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogType"));
		m_DAnalogTypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigDAnalogType"));
		m_DAnalogTypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigDAnalogTypeDiagram"));
		m_DAnalogTypeDiagram->Resize(m_DualAnalogBitmaps.at(0)->w, m_DualAnalogBitmaps.at(0)->h);
		m_DAnalogTypeDiagram->CenterInParent(true, true);
		m_DAnalogTypeDiagram->MoveRelative(0, -10);

		m_XBox360TypeBox = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360Type"));
		m_XBox360TypeButton = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonConfigXBox360Type"));
		m_XBox360TypeDiagram = dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("BoxConfigXBox360TypeDiagram"));
		m_XBox360TypeDiagram->Resize(m_DualAnalogBitmaps.at(0)->w, m_DualAnalogBitmaps.at(0)->h);
		m_XBox360TypeDiagram->CenterInParent(true, true);
		m_XBox360TypeDiagram->MoveRelative(0, -10);

		m_ConfiguringPlayer = Players::PlayerOne;
		m_ConfiguringDevice = InputDevice::DEVICE_KEYB_ONLY;
		m_ConfiguringGamepad = GamepadType::DPad;
		m_ConfigureStep = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsInputMappingWizardGUI::HandleInputEvents(GUIEvent &guiEvent, int player) {
		if (guiEvent.GetType() == GUIEvent::Command) {
			/*
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
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	void SettingsInputMappingWizardGUI::UpdateConfigScreen() {
		char str[256];

		if (m_ScreenChange) {
			// Hide most things first, enable as needed
			ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetVisible(false);
			ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetVisible(false);
			RecommendationBox->SetVisible(false);
			RecommendationDiagram->SetVisible(false);
			ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetVisible(false);
			ConfigSkipButton->SetVisible(false);
			ConfigBackButton->SetVisible(false);
			DPadTypeBox->SetVisible(false);
			DAnalogTypeBox->SetVisible(false);
			XBox360TypeBox->SetVisible(false);
		}

		// Use GUIInput class for better key detection
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
	bool SettingsInputMappingWizardGUI::UpdateKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ConfigWizardSteps::KeyboardConfigSteps);
		ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Up Cursor]" : "[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Down Cursor]" : "[S)");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Left Cursor]" : "[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Right Cursor]" : "[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 1]" : "[H]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_FIRE))

				{
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 2]" : "[J]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_AIM)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 3]" : "[K]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PIEMENU)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("JUMP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Enter]" : "[L]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("CROUCH");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num Del]" : "[.]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 4]" : "[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 5]" : "[E]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 11:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 0]" : "[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 12:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 9]" : "[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 13:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 6]" : "[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 14:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 7]" : "[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 15:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText(ConfiguringPlayer % 2 ? "[Num 8]" : "[C]");
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
	bool SettingsInputMappingWizardGUI::UpdateMouseAndKeyboardConfigWizard() {
		if (m_ScreenChange) {
			ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
			ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetVisible(true);
			std::snprintf(str, sizeof(str), "Mouse + Keyboard Configuration - Player %i", ConfiguringPlayer + 1);
			ConfigLabel.at(ConfigWizardLabels::ConfigTitle)->SetText(str);
			ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetText("Press the key for");
			ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetVisible(true);
			RecommendationBox->SetVisible(true);
			ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetVisible(true);
			ConfigSkipButton->SetVisible(true);
			ConfigBackButton->SetVisible(true);
			m_ScreenChange = false;
		}

		// Step label update
		std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ConfigWizardSteps::MouseAndKeyboardConfigSteps);
		ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetText(str);

		switch (ConfigureStep) {
			case 0:
				// Hide the back button on this first step
				ConfigBackButton->SetVisible(false);

				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[W]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_UP)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_UP);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_JUMP);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 1:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[S]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_DOWN)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_DOWN);
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_CROUCH);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 2:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[A]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_LEFT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_LEFT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 3:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[D]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_L_RIGHT)) {
					g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_R_RIGHT);
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 4:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("RELOAD");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[R]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_RELOAD)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 5:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PICK UP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[F]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_PICKUP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 6:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("DROP");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[G]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_DROP)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 7:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREV WEAPON");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[X]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 8:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[C]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 9:
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[Q]");
				if (g_UInputMan.CaptureKeyMapping(ConfiguringPlayer, INPUT_PREV)) {
					ConfigureStep++;
					m_ScreenChange = true;
				}
				break;
			case 10:
				// Hide skip button on this last step
				ConfigSkipButton->SetVisible(false);

				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
				ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetText("[E]");
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
	bool SettingsInputMappingWizardGUI::UpdateGamepadConfigWizard() {
		int whichJoy = ConfiguringDevice - InputDevice::DEVICE_GAMEPAD_1;
		AllegroBitmap *pDiagramBitmap = 0;

		// Choose which gamepad type - special first step

		if (ConfigureStep == 0) {
			// Set title
			std::snprintf(str, sizeof(str), "Choose Gamepad Type for Player %i:", ConfiguringPlayer + 1);
			ConfigLabel.at(ConfigWizardLabels::ConfigTitle)->SetText(str);

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
				ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetVisible(true);
				ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetVisible(true);
				ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetVisible(true);
				RecommendationBox->SetVisible(true);
				ConfigSkipButton->SetVisible(true);
				ConfigBackButton->SetVisible(true);
				m_BlinkTimer.Reset();
			}

			// D-pad
			if (ConfiguringGamepad == DPAD) {
				if (m_ScreenChange) {
					std::snprintf(str, sizeof(str), "D-Pad Gamepad Configuration - Player %i", ConfiguringPlayer + 1);
					ConfigLabel.at(ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DPadBitmaps.at(0)->w, DPadBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 4);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ConfigWizardSteps::DPadConfigSteps);
				ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DPadBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM UP");

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
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE or AIM DOWN");
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
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_LEFT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_R_RIGHT);
							//                        g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("SHARP AIM");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_AIM)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
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
					ConfigLabel.at(ConfigWizardLabels::ConfigTitle)->SetText(str);
					ConfigLabel.at(ConfigWizardLabels::ConfigRecommendation)->SetVisible(false);
					ConfigLabel.at(ConfigWizardLabels::ConfigInstruction)->SetText("Press the button or move the stick for");
					RecommendationDiagram->SetVisible(true);
					RecommendationDiagram->Resize(DualAnalogBitmaps.at(0)->w, DualAnalogBitmaps.at(0)->h);
					RecommendationDiagram->CenterInParent(true, true);
					RecommendationDiagram->MoveRelative(0, 8);
					m_ScreenChange = false;
				}

				// Step label update
				std::snprintf(str, sizeof(str), "Step %i / %i", ConfigureStep + 1, ConfigWizardSteps::DualAnalogConfigSteps);
				ConfigLabel.at(ConfigWizardLabels::ConfigSteps)->SetText(str);

				// Diagram update
				// Not passing in ownership of the BITMAP
				pDiagramBitmap = new AllegroBitmap(DualAnalogBitmaps.at(m_BlinkTimer.AlternateReal(500) ? 0 : ConfigureStep));
				// Passing in ownership of the AllegroBitmap, but again, not the BITMAP contained within
				RecommendationDiagram->SetDrawImage(pDiagramBitmap);
				pDiagramBitmap = 0;

				switch (ConfigureStep) {
					case 1:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE UP or JUMP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_UP)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_UP);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_JUMP);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 2:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE DOWN or CROUCH");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_DOWN)) {
							g_UInputMan.ClearMapping(ConfiguringPlayer, INPUT_AIM_DOWN);
							g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_CROUCH);
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 3:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 4:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("MOVE RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_L_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 5:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("AIM UP");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_UP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 6:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("AIM DOWN");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_DOWN)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 7:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("AIM LEFT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_LEFT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 8:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("AIM RIGHT");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_R_RIGHT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 9:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("FIRE / ACTIVATE");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_FIRE)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 10:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("COMMAND MENU");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PIEMENU)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 11:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 12:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS BODY");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 13:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PREVIOUS WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_PREV)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 14:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("NEXT WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_NEXT)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 15:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("PICKUP WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_PICKUP)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 16:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("RELOAD WEAPON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_WEAPON_RELOAD)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 17:
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("START BUTTON");
						if (g_UInputMan.CaptureJoystickMapping(ConfiguringPlayer, whichJoy, INPUT_START)) {
							ConfigureStep++;
							m_ScreenChange = true;
						}
						break;
					case 18:
						// Hide the skip button on this last step
						ConfigSkipButton->SetVisible(false);
						ConfigLabel.at(ConfigWizardLabels::ConfigInput)->SetText("BACK BUTTON");
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