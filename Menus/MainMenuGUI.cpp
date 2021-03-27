#include "MainMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

#include "GATutorial.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"
#include "EditorActivity.h"
#include "MultiplayerGame.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Clear() {
		m_pController = nullptr;
		m_pGUIScreen = nullptr;
		m_pGUIInput = nullptr;
		m_pGUIController = nullptr;
		m_MenuEnabled = ENABLED;
		m_MenuScreen = MAINSCREEN;
		m_ScreenChange = false;
		m_BlinkTimer.Reset();
		for (int screen = 0; screen < SCREENCOUNT; ++screen) {
			m_apScreenBox.at(screen) = nullptr;
		}
		for (int button = 0; button < MAINMENUBUTTONCOUNT; ++button) {
			m_MainMenuButtons.at(button) = nullptr;
		}

		m_pEditorPanel = nullptr;
		m_pScrollPanel = nullptr;
		m_ScrollTimer.Reset();
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;
		m_TutorialOffered = false;

		m_Quit = false;

		// Editor screen
		for (int button = 0; button < EDITORBUTTONCOUNT; ++button) {
			m_aEditorButton.at(button) = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int MainMenuGUI::Create(Controller *pController) {
		RTEAssert(pController, "No controller sent to MainMenuGUI on creation!");
		m_pController = pController;

		if (!m_pGUIScreen) { m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32()); }
		if (!m_pGUIInput) { m_pGUIInput = new AllegroInput(-1, true); }
		if (!m_pGUIController) { m_pGUIController = new GUIControlManager(); }
		if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/MainMenu")) { RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu"); }
		m_pGUIController->Load("Base.rte/GUIs/MainMenuGUI.ini");

		// Make sure we have convenient points to the containing GUI collection boxes that we will manipulate the positions of
		m_apScreenBox.at(ROOT) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
		m_apScreenBox.at(MAINSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MainScreen"));
		m_apScreenBox.at(OPTIONSSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("OptionsScreen"));
		m_apScreenBox.at(CONFIGSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConfigScreen"));
		m_apScreenBox.at(EDITORSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorScreen"));
		m_apScreenBox.at(METASCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MetaScreen"));
		m_apScreenBox.at(CREDITSSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsScreen"));
		m_apScreenBox.at(QUITSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("QuitConfirmBox"));
		m_apScreenBox.at(MODMANAGERSCREEN) = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ModManagerScreen"));

		m_apScreenBox.at(ROOT)->SetPositionAbs((g_FrameMan.GetResX() - m_apScreenBox.at(ROOT)->GetWidth()) / 2, 0);

		// Set up screens' initial positions and visibility
		m_apScreenBox.at(QUITSCREEN)->CenterInParent(true, true);
		// Hide all screens, the appropriate screen will reappear on next update
		HideAllScreens();

		// Panel behind editor menu to be resized depending on which editors are available
		m_pEditorPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorPanel"));
		// Credits scrolling panel
		m_pScrollPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsPanel"));

		m_MainMenuButtons.at(CAMPAIGN) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCampaign"));
		m_MainMenuButtons.at(SKIRMISH) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToSkirmish"));
		m_MainMenuButtons.at(MULTIPLAYER) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToMultiplayer"));
		m_MainMenuButtons.at(OPTIONS) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToOptions"));
		m_MainMenuButtons.at(MODMANAGER) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToModManager"));
		m_MainMenuButtons.at(EDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToEditor"));
		m_MainMenuButtons.at(CREDITS) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCreds"));
		m_MainMenuButtons.at(QUIT) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonQuit"));
		m_MainMenuButtons.at(RESUME) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonResume"));
		m_MainMenuButtons.at(PLAYTUTORIAL) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonTutorial"));
		m_MainMenuButtons.at(METACONTINUE) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonContinue"));
		m_MainMenuButtons.at(BACKTOMAIN) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonBackToMain"));
		m_MainMenuButtons.at(QUITCONFIRM) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitConfirmButton"));
		m_MainMenuButtons.at(QUITCANCEL) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitCancelButton"));
		m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
		m_MainMenuButtons.at(PLAYTUTORIAL)->SetVisible(false);
		m_MainMenuButtons.at(METACONTINUE)->SetVisible(false);

		m_aEditorButton.at(SCENEEDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonSceneEditor"));
		m_aEditorButton.at(AREAEDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAreaEditor"));
		m_aEditorButton.at(ASSEMBLYEDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAssemblyEditor"));
		m_aEditorButton.at(GIBEDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonGibPlacement"));
		m_aEditorButton.at(ACTOREDITOR) = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonActorEditor"));

		m_pMetaNoticeLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("MetaLabel"));

		m_VersionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("VersionLabel"));
		m_VersionLabel->SetText(c_GameVersion);
		m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 10);

		// Read all the credits from the file and set the credits label
		m_CreditsLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CreditsLabel"));
		Reader creditsReader("Credits.txt");
		std::string creditsText = creditsReader.WholeFileAsString();

		// TODO: Get Unicode going!
			// Hack here to change the special characters over 128 in the ANSI ASCII table to match our font files
		for (string::iterator sItr = creditsText.begin(); sItr != creditsText.end(); ++sItr) {
			if (*sItr == -60) { (*sItr) = (char)142; } //'Ä'
			if (*sItr == -42) { (*sItr) = (char)153; } //'Ö'
			if (*sItr == -87) { (*sItr) = (char)221; } //'©'
		}
		m_CreditsLabel->SetText(creditsText);
		m_CreditsLabel->ResizeHeightToFit();

		// Set initial focus, category list, and label settings
		m_ScreenChange = true;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Destroy() {
		delete m_pGUIController;
		delete m_pGUIInput;
		delete m_pGUIScreen;

		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIControlManager * MainMenuGUI::GetGUIControlManager() {
		return m_pGUIController;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::SetEnabled(bool enable) {
		if (enable && m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
			m_MenuEnabled = ENABLING;
			g_GUISound.EnterMenuSound()->Play();
		} else if (!enable && m_MenuEnabled != DISABLED && m_MenuEnabled != DISABLING) {
			m_MenuEnabled = DISABLING;
			g_GUISound.ExitMenuSound()->Play();
		}

		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Update() {
		// Update the input controller
		m_pController->Update();

		// Reset the specific triggers
		m_ScenarioStarted = false;
		m_CampaignStarted = false;
		m_ActivityRestarted = false;
		m_ActivityResumed = false;

		m_Quit = false;

		// Don't update the main menu if the console is open
		if (g_ConsoleMan.IsEnabled())
			return;

		// If esc pressed, show quit dialog if applicable
		if (g_UInputMan.KeyPressed(KEY_ESC)) {
			if (m_MenuScreen == OPTIONSSCREEN || m_MenuScreen == MODMANAGERSCREEN || m_MenuScreen == EDITORSCREEN || m_MenuScreen == CREDITSSCREEN) {
				HideAllScreens();
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
				m_MenuScreen = MAINSCREEN;
				m_ScreenChange = true;
				g_GUISound.BackButtonPressSound()->Play();

				if (m_MenuScreen == OPTIONSSCREEN) {
					//g_SettingsMan.SetFlashOnBrainDamage(m_aOptionsCheckbox.at(FLASHONBRAINDAMAGE)->GetCheck());
					//g_SettingsMan.SetBlipOnRevealUnseen(m_aOptionsCheckbox.at(BLIPONREVEALUNSEEN)->GetCheck());
					//g_SettingsMan.SetShowForeignItems(m_aOptionsCheckbox.at(SHOWFOREIGNITEMS)->GetCheck());
					//g_SettingsMan.SetShowToolTips(m_aOptionsCheckbox.at(SHOWTOOLTIPS)->GetCheck());
					g_SettingsMan.UpdateSettingsFile();
				} else if (m_MenuScreen == MODMANAGERSCREEN) {
					g_SettingsMan.UpdateSettingsFile();
				}
			} else {
				QuitLogic();
			}
		}

		////////////////////////////////////////////////////////////////////////
		// Animate the menu into and out of view if enabled or disabled

		// Quit now if we aren't enabled
		if (m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING) {
			return;
		}

		//////////////////////////////////////
		// MAINSCREEN MENU SCREEN

		if (m_MenuScreen == MAINSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(MAINSCREEN)->SetVisible(true);

				if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
					m_apScreenBox.at(MAINSCREEN)->Resize(128, 220);
					m_MainMenuButtons.at(RESUME)->SetVisible(true);
				} else {
					m_apScreenBox.at(MAINSCREEN)->Resize(128, 196);
					m_MainMenuButtons.at(RESUME)->SetVisible(false);
				}
				// Restore the label on the campaign button
				m_MainMenuButtons.at(CAMPAIGN)->SetText("MetaGame (WIP)");

				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
				m_MainMenuButtons.at(PLAYTUTORIAL)->SetVisible(false);
				m_MainMenuButtons.at(METACONTINUE)->SetVisible(false);
				m_ScreenChange = false;
			}

			// Blink the resume button to show the game is still going
			if (m_MainMenuButtons.at(RESUME)->GetVisible()) {
				if (m_BlinkTimer.AlternateReal(500)) {
					m_MainMenuButtons.at(RESUME)->SetFocus();
				} else {
					m_pGUIController->GetManager()->SetFocus(nullptr);
				}
			}
		}

		//////////////////////////////////////
		// OPTIONS MENU SCREEN

		else if (m_MenuScreen == OPTIONSSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(OPTIONSSCREEN)->SetVisible(true);
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(true);
				m_apScreenBox.at(OPTIONSSCREEN)->GUIPanel::AddChild(m_MainMenuButtons.at(BACKTOMAIN));
				m_MainMenuButtons.at(BACKTOMAIN)->SetPositionRel(180, 220);
				//m_pBackToOptionsButton->SetVisible(false);
				//UpdateDeviceLabels();
				m_ScreenChange = false;
			}
		}

		//////////////////////////////////////
		// CONFIGURATION SCREEN

		else if (m_MenuScreen == CONFIGSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(CONFIGSCREEN)->SetVisible(true);
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(false);
				//m_pBackToOptionsButton->SetVisible(true);
				// Let this pass through, UpdateConfigScreen uses it
				//m_ScreenChange = false;
			}

			// Continuously update the contents through all the config steps
			//UpdateConfigScreen();
		}

		//////////////////////////////////////
		// MOD MANAGER SCREEN

		else if (m_MenuScreen == MODMANAGERSCREEN) {
			if (m_ScreenChange) { m_apScreenBox.at(MODMANAGERSCREEN)->SetVisible(true); }
		}

		//////////////////////////////////////
		// EDITOR MENU SCREEN

		else if (m_MenuScreen == EDITORSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(EDITORSCREEN)->SetVisible(true);
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(true);
				m_apScreenBox.at(EDITORSCREEN)->GUIPanel::AddChild(m_MainMenuButtons.at(BACKTOMAIN));
				m_MainMenuButtons.at(BACKTOMAIN)->SetPositionRel(4, 145);
				m_ScreenChange = false;
			}
		}

		//////////////////////////////////////
		// CREDITS MENU SCREEN

		else if (m_MenuScreen == CREDITSSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(CREDITSSCREEN)->SetVisible(true);
				m_MainMenuButtons.at(BACKTOMAIN)->SetVisible(true);
				m_apScreenBox.at(CREDITSSCREEN)->GUIPanel::AddChild(m_MainMenuButtons.at(BACKTOMAIN));
				m_MainMenuButtons.at(BACKTOMAIN)->SetPositionRel(240, 298);
				m_pScrollPanel->SetPositionRel(0, 0);
				m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight());
				m_ScrollTimer.Reset();
				m_ScreenChange = false;
			}

			long scrollTime = 90000;
			float scrollProgress = (float)m_ScrollTimer.GetElapsedRealTimeMS() / (float)scrollTime;
			int scrollDist = m_pScrollPanel->GetHeight() + m_CreditsLabel->GetHeight();
			m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight() - static_cast<int>(static_cast<float>(scrollDist) * scrollProgress));
			// If we've scrolled through the whole thing, reset to the bottom and restart scroll
			if (m_ScrollTimer.IsPastRealMS(scrollTime)) {
				m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight());
				m_ScrollTimer.Reset();
			}

			//        m_MainMenuButtons.at(BACKTOMAIN)->SetFocus();
		}

		//////////////////////////////////////
		// METAGAME NOTICE SCREEN

		else if (m_MenuScreen == METASCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(METASCREEN)->SetVisible(true);
				m_MainMenuButtons.at(PLAYTUTORIAL)->SetVisible(true);
				m_MainMenuButtons.at(METACONTINUE)->SetVisible(true);
				m_pMetaNoticeLabel->SetText("- A T T E N T I O N -\n\nPlease note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\nAs such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\nThat said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\nAlso, if you have not yet played Cortex Command, we recommend you first try the tutorial:");
				m_pMetaNoticeLabel->SetVisible(true);
				// Flag that this notice has now been shown once, so no need to keep showing it
				m_TutorialOffered = true;
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons.at(BACKTOMAIN)->SetFocus();
		}

		//////////////////////////////////////
		// QUIT CONFIRM SCREEN

		else if (m_MenuScreen == QUITSCREEN) {
			if (m_ScreenChange) {
				m_apScreenBox.at(QUITSCREEN)->SetVisible(true);
				m_ScreenChange = false;
			}

			//        m_MainMenuButtons.at(QUITCONFIRM)->SetFocus();
		}

		//////////////////////////////////////////
		// Update the ControlManager

		m_pGUIController->Update();

		///////////////////////////////////////
		// Handle events

		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent)) {
			// Commands
			if (anEvent.GetType() == GUIEvent::Command) {
				// Campaign button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(CAMPAIGN)) {
					// Show the MetaGame notice screen if it hasn't already been shown yet
					if (!m_TutorialOffered)
						m_MenuScreen = METASCREEN;
					// Start the campaign right away!
					else {
						m_CampaignStarted = true;
						m_MenuScreen = MAINSCREEN;
					}
					HideAllScreens();
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				}

				// Skirmish button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(SKIRMISH)) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;
					g_GUISound.ButtonPressSound()->Play();
				}

				if (anEvent.GetControl() == m_MainMenuButtons.at(MULTIPLAYER)) {
					m_ScenarioStarted = true;
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					g_GUISound.ButtonPressSound()->Play();

					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;
					m_ActivityRestarted = true;
					g_GUISound.ExitMenuSound()->Play();

					g_SceneMan.SetSceneToLoad("Editor Scene");
					MultiplayerGame *pMultiplayerGame = new MultiplayerGame;
					pMultiplayerGame->Create();
					g_ActivityMan.SetStartActivity(pMultiplayerGame);
				}

				// Options button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(OPTIONS)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = OPTIONSSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Editor button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(EDITOR)) {
					m_CampaignStarted = false;

					if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = EDITORSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
					//                g_GUISound.UserErrorSound()->Play();
				}

				// Editor button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(MODMANAGER)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MODMANAGERSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Credits button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(CREDITS)) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = CREDITSSCREEN;
					m_ScreenChange = true;

					g_GUISound.ButtonPressSound()->Play();
				}

				// Quit button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(QUIT)) {
					QuitLogic();
					g_GUISound.ButtonPressSound()->Play();
				}

				// Resume button pressed
				if (anEvent.GetControl() == m_MainMenuButtons.at(RESUME)) {
					m_ActivityResumed = true;

					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// EDITOR SCREEN BUTTONS

				if (m_MenuScreen == EDITORSCREEN &&
					(anEvent.GetControl() == m_aEditorButton.at(SCENEEDITOR) ||
						anEvent.GetControl() == m_aEditorButton.at(AREAEDITOR) ||
						anEvent.GetControl() == m_aEditorButton.at(ASSEMBLYEDITOR) ||
						anEvent.GetControl() == m_aEditorButton.at(GIBEDITOR) ||
						anEvent.GetControl() == m_aEditorButton.at(ACTOREDITOR))) {
					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;

					m_ActivityRestarted = true;

					// Create and start the appropriate editor Activity
					if (anEvent.GetControl() == m_aEditorButton.at(SCENEEDITOR)) {
						StartSceneEditor();
					} else if (anEvent.GetControl() == m_aEditorButton.at(AREAEDITOR)) {
						StartAreaEditor();
					} else if (anEvent.GetControl() == m_aEditorButton.at(ASSEMBLYEDITOR)) {
						StartAssemblyEditor();
					} else if (anEvent.GetControl() == m_aEditorButton.at(GIBEDITOR)) {
						StartGibEditor();
					} else if (anEvent.GetControl() == m_aEditorButton.at(ACTOREDITOR)) {
						StartActorEditor();
					}

					//                g_GUISound.BackButtonPressSound()->Play();
					g_GUISound.ExitMenuSound()->Play();
				}

				/////////////////////////////////////////////
				// META NOTICE SCREEN BUTTONS

				if (m_MenuScreen == METASCREEN) {
					// Play tutorial button pressed
					if (anEvent.GetControl() == m_MainMenuButtons.at(PLAYTUTORIAL)) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;

						// Set up and start the tutorial!
						g_ActivityMan.SetStartActivity(dynamic_cast<Activity *>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
						GameActivity * pGameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetStartActivity());
						if (pGameActivity) { pGameActivity->SetStartingGold(10000); }
						g_SceneMan.SetSceneToLoad("Tutorial Bunker");
						m_ActivityRestarted = true;

						g_GUISound.ButtonPressSound()->Play();
					}
					// Go to registration dialog button
					else if (anEvent.GetControl() == m_MainMenuButtons.at(METACONTINUE)) {
						m_CampaignStarted = true;

						// Hide all screens, and stay in main menu for if/when player comes back to the main menu
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;
						g_GUISound.ButtonPressSound()->Play();
					}
				}

				/////////////////////////////////////////////
				// MOD MANAGER SCREEN BUTTONS

				if (m_MenuScreen == MODMANAGERSCREEN) {
					;
				}

				/////////////////////////////////////////////
				// QUIT SCREEN BUTTONS

				if (m_MenuScreen == QUITSCREEN) {
					// Confirm quitting of game
					if (anEvent.GetControl() == m_MainMenuButtons.at(QUITCONFIRM)) {
						m_Quit = true;

						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
					// Cancel quitting
					else if (anEvent.GetControl() == m_MainMenuButtons.at(QUITCANCEL)) {
						// Hide all screens, the appropriate screen will reappear on next update
						HideAllScreens();
						m_MenuScreen = MAINSCREEN;
						m_ScreenChange = true;

						g_GUISound.ButtonPressSound()->Play();
					}
				}
			}

			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification) {
				// Button focus notification that we can play a sound to
				if (dynamic_cast<GUIButton *>(anEvent.GetControl())) {
					if (anEvent.GetMsg() == GUIButton::Focused) { g_GUISound.SelectionChangeSound()->Play(); }
				}

				// Mod list pressed
				//if (anEvent.GetControl() == m_pModManagerModsListBox) { ; }

				// Script list pressed
				//if (anEvent.GetControl() == m_pModManagerScriptsListBox) { ; }

				// Resolution combobox closed, something new selected
				//if (anEvent.GetControl() == m_pResolutionCombo) { ; }

				// Sound Volume slider changed
				//if (anEvent.GetControl() == m_pSoundSlider) { ; }

				// Music Volume slider changed
				//if (anEvent.GetControl() == m_pMusicSlider) { ; }

				// Dead zone sliders control
				//for (int which = P1DEADZONESLIDER; which < DEADZONESLIDERCOUNT; ++which) { ; }
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::Draw(BITMAP *drawBitmap) const {
		AllegroScreen drawScreen(drawBitmap);
		m_pGUIController->Draw(&drawScreen);
		m_pGUIController->DrawMouse();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::HideAllScreens() {
		for (int iscreen = MAINSCREEN; iscreen < SCREENCOUNT; ++iscreen) {
			if (m_apScreenBox.at(iscreen)) { m_apScreenBox.at(iscreen)->SetVisible(false); }
		}
		m_ScreenChange = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::QuitLogic() {
		// If quit confirm dialog not already showing, or an activity is running, show it
		if (m_MenuScreen != QUITSCREEN && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing)) {
			HideAllScreens();
			m_MenuScreen = QUITSCREEN;
			m_ScreenChange = true;
		}
		// No activity, so just start quitting
		else {
			m_Quit = true;

			// Hide all screens, the appropriate screen will reappear on next update
			HideAllScreens();
			m_ScreenChange = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartActorEditor() const { StartEditorActivity(new ActorEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartGibEditor() const { StartEditorActivity(new GibEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartSceneEditor() const { StartEditorActivity(new SceneEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAreaEditor() const { StartEditorActivity(new AreaEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartAssemblyEditor() const { StartEditorActivity(new AssemblyEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void MainMenuGUI::StartEditorActivity(EditorActivity *editorActivityToStart) const {
		g_SceneMan.SetSceneToLoad("Editor Scene");
		editorActivityToStart->Create();
		editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
		g_ActivityMan.SetStartActivity(editorActivityToStart);
	}
}