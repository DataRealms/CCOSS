//////////////////////////////////////////////////////////////////////////////////////////
// File:            MainMenuGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the MainMenuGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MainMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "AudioMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"
#include "GlobalScript.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIComboBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"
#include "GUI/GUICheckbox.h"

#include "Controller.h"
#include "Entity.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "DataModule.h"
#include "GABrainMatch.h"
#include "GABaseDefense.h"
#include "GATutorial.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"
#include "EditorActivity.h"
#include "MultiplayerGame.h"

extern int g_IntroState;

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MainMenuGUI, effectively
//                  resetting the members of this abstraction level only.

void MainMenuGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_MenuEnabled = ENABLED;
    m_MenuScreen = MAINSCREEN;
    m_ScreenChange = false;
    m_MainMenuFocus = CAMPAIGN;
    m_FocusChange = 0;
    m_MenuSpeed = 0.3;
    m_ListItemIndex = 0;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    for (int screen = 0; screen < SCREENCOUNT; ++screen)
        m_apScreenBox[screen] = 0;
    for (int button = 0; button < MAINMENUBUTTONCOUNT; ++button)
        m_MainMenuButtons[button] = 0;
    m_pTeamBox = 0;
    m_pSceneSelector = 0;
    for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box)
        m_aSkirmishBox[box] = 0;
    for (int button = 0; button < SKIRMISHPLAYERCOUNT; ++button)
        m_aSkirmishButton[button] = 0;
    m_pCPUTeamLabel = 0;
    for (int button = 0; button < OPTIONSBUTTONCOUNT; ++button)
        m_aOptionButton[button] = 0;
    for (int label = 0; label < OPTIONSLABELCOUNT; ++label)
        m_aOptionsLabel[label] = 0;
    for (int checkbox = 0; checkbox < OPTIONSCHECKBOXCOUNT; ++checkbox)
        m_aOptionsCheckbox[checkbox] = 0;
    m_pResolutionCombo = 0;
    m_pSoundLabel = 0;
    m_pMusicLabel = 0;
    m_pSoundSlider = 0;
    m_pMusicSlider = 0;
    m_pEditorPanel = 0;
    m_pScrollPanel = 0;
    m_ScrollTimer.Reset();
    m_ScenarioStarted = false;
    m_CampaignStarted = false;
    m_ActivityRestarted = false;
    m_ActivityResumed = false;
    m_TutorialOffered = false;
    m_StartPlayers = 1;
    m_StartTeams = 2;
    m_StartFunds = 1600;
    for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player)
        m_aTeamAssignments[player] = Activity::TeamOne;
    m_CPUTeam = -1;
    m_StartDifficulty = Activity::MediumDifficulty;
    m_Quit = false;

    // Config screen
    m_ConfiguringPlayer = Players::PlayerOne;
    m_ConfiguringDevice = DEVICE_KEYB_ONLY;
    m_ConfiguringGamepad = DPAD;
    m_ConfigureStep = 0;
    for (int label = 0; label < CONFIGLABELCOUNT; ++label)
        m_pConfigLabel[label] = 0;

    // Editor screen
    for (int button = 0; button < EDITORBUTTONCOUNT; ++button)
        m_aEditorButton[button] = 0;

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

    m_pModManagerBackButton = 0;

	m_ResolutionChangeDialog = nullptr;
	m_ButtonConfirmResolutionChange = nullptr;
	m_ButtonConfirmResolutionChangeFullscreen = nullptr;
	m_ButtonCancelResolutionChange = nullptr;
	m_ResolutionChangeToUpscaled = false;

	m_MaxResX = 0;
	m_MaxResY = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MainMenuGUI object ready for use.

int MainMenuGUI::Create(Controller *pController)
{
    RTEAssert(pController, "No controller sent to MainMenuGUI on creation!");
    m_pController = pController;

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(-1, true); 
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    if(!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/MainMenu"))
        RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu");
    m_pGUIController->Load("Base.rte/GUIs/MainMenuGUI.ini");

    // Make sure we have convenient points to the containing GUI colleciton boxes that we will manipulate the positions of
	m_apScreenBox[ROOT] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
    m_apScreenBox[MAINSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MainScreen"));
    m_apScreenBox[PLAYERSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PlayersScreen"));
    m_apScreenBox[SKIRMISHSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SkirmishScreen"));
    m_apScreenBox[DIFFICULTYSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("DifficultyScreen"));
    m_apScreenBox[OPTIONSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("OptionsScreen"));
    m_apScreenBox[CONFIGSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConfigScreen"));
    m_apScreenBox[EDITORSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorScreen"));
    m_apScreenBox[METASCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("MetaScreen"));
    m_apScreenBox[CREDITSSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsScreen"));
    m_apScreenBox[QUITSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("QuitConfirmBox"));
    m_apScreenBox[MODMANAGERSCREEN] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ModManagerScreen"));

	m_apScreenBox[ROOT]->SetPositionAbs((g_FrameMan.GetResX() - m_apScreenBox[ROOT]->GetWidth()) / 2, 0);// (g_FrameMan.GetResY() - m_apScreenBox[ROOT]->GetHeight()) / 2);
// NO, this screws up the menu positioning!
//    m_apScreenBox[ROOT]->Resize(m_apScreenBox[ROOT]->GetWidth(), g_FrameMan.GetResY());

    // Set up screens' initial positions and visibility
    m_apScreenBox[QUITSCREEN]->CenterInParent(true, true);
    // Hide all screens, the appropriate screen will reappear on next update
    HideAllScreens();

    // Panel behind editor menu to be resized depending on which editors are available
    m_pEditorPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("EditorPanel"));
    // Credits scrolling panel
    m_pScrollPanel = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CreditsPanel"));

    m_MainMenuButtons[CAMPAIGN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCampaign"));
    m_MainMenuButtons[SKIRMISH] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToSkirmish"));
	m_MainMenuButtons[MULTIPLAYER] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToMultiplayer"));
	m_MainMenuButtons[OPTIONS] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToOptions"));
    m_MainMenuButtons[MODMANAGER] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToModManager"));
    m_MainMenuButtons[EDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToEditor"));
    m_MainMenuButtons[CREDITS] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonMainToCreds"));
    m_MainMenuButtons[QUIT] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonQuit"));
    m_MainMenuButtons[RESUME] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonResume"));
    m_MainMenuButtons[PLAYTUTORIAL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonTutorial"));
    m_MainMenuButtons[METACONTINUE] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonContinue"));
    m_MainMenuButtons[BACKTOMAIN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonBackToMain"));
    m_MainMenuButtons[QUITCONFIRM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitConfirmButton"));
    m_MainMenuButtons[QUITCANCEL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("QuitCancelButton"));
    m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
    m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(false);
    m_MainMenuButtons[METACONTINUE]->SetVisible(false);

    m_pSceneSelector = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ComboScene"));
    m_pTeamBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelTeams"));
    m_aSkirmishBox[P1TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP1Team"));
    m_aSkirmishBox[P2TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP2Team"));
    m_aSkirmishBox[P3TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP3Team"));
    m_aSkirmishBox[P4TEAM] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PanelP4Team"));
    m_aSkirmishButton[P1TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP1Team"));
    m_aSkirmishButton[P2TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP2Team"));
    m_aSkirmishButton[P3TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP3Team"));
    m_aSkirmishButton[P4TEAM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonP4Team"));
    m_pCPUTeamLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelCPUTeam"));
	
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
	if (dztype == DeadZoneType::CIRCLE)
	{
		m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetCheck(1);
		m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetText("O");
	} else if(dztype == DeadZoneType::SQUARE) {
		m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetCheck(0);
		m_aDeadZoneCheckbox[P1DEADZONESLIDER]->SetText(str);
	}

	dztype = g_UInputMan.GetControlScheme(1)->GetJoystickDeadzoneType();
	if (dztype == DeadZoneType::CIRCLE)
	{
		m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetCheck(1);
		m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetText("O");
	}
	else if (dztype == DeadZoneType::SQUARE) {
		m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetCheck(0);
		m_aDeadZoneCheckbox[P2DEADZONESLIDER]->SetText(str);
	}

	dztype = g_UInputMan.GetControlScheme(2)->GetJoystickDeadzoneType();
	if (dztype == DeadZoneType::CIRCLE)
	{
		m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetCheck(1);
		m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetText("O");
	}
	else if (dztype == DeadZoneType::SQUARE) {
		m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetCheck(0);
		m_aDeadZoneCheckbox[P3DEADZONESLIDER]->SetText(str);
	}

	dztype = g_UInputMan.GetControlScheme(3)->GetJoystickDeadzoneType();
	if (dztype == DeadZoneType::CIRCLE)
	{
		m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetCheck(1);
		m_aDeadZoneCheckbox[P4DEADZONESLIDER]->SetText("O");
	}
	else if (dztype == DeadZoneType::SQUARE) {
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

    m_aEditorButton[SCENEEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonSceneEditor"));
    m_aEditorButton[AREAEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAreaEditor"));
    m_aEditorButton[ASSEMBLYEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonAssemblyEditor"));
    m_aEditorButton[GIBEDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonGibPlacement"));
    m_aEditorButton[ACTOREDITOR] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonActorEditor"));

    m_pMetaNoticeLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("MetaLabel"));

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

	// Mod manager controls
	m_pModManagerBackButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonExitModManager"));
	m_pModManagerToggleModButton =  dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonToggleMod"));
	m_pModManagerToggleScriptButton =  dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonToggleScript"));
	m_pModManagerModsListBox = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("ModsLB"));
	m_pModManagerScriptsListBox = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("ScriptsLB"));
	m_pModManagerDescriptionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelDescription"));

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

	m_VersionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("VersionLabel"));
	m_VersionLabel->SetText(c_GameVersion);
	m_VersionLabel->SetPositionAbs(10, g_FrameMan.GetResY() - m_VersionLabel->GetTextHeight() - 10);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Load mod data and fill the lists
    for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i)  
    {
		// Discard official modules
		if (i >= g_PresetMan.GetOfficialModuleCount() && i < g_PresetMan.GetTotalModuleCount() - 2)
		{
			const DataModule *pModule = g_PresetMan.GetDataModule(i);
			if (pModule)
			{
				ModRecord r;
				r.ModulePath = pModule->GetFileName();
				r.Description = pModule->GetDescription();
				r.ModuleName = pModule->GetFriendlyName();
				r.Disabled = g_SettingsMan.IsModDisabled(r.ModulePath);

				m_KnownMods.push_back(r);
			}
		}
    }

	// Now add missing data from disabled mods settings
	map<string, bool> disabledMods = g_SettingsMan.GetDisabledModsList();
	for(map<std::string, bool>::iterator itr = disabledMods.begin(); itr != disabledMods.end(); ++itr)
	{
		string modPath = itr->first;
		std::transform(modPath.begin(), modPath.end(), modPath.begin(), ::tolower);

		bool found = false;

		// Check if this mod is already in list
		for (vector<ModRecord>::iterator mItr = m_KnownMods.begin(); mItr != m_KnownMods.end(); ++mItr)
			if (modPath == (*mItr).ModulePath)
				found = true;

		if (!found)
		{
			ModRecord r;
			r.ModulePath = modPath;
			r.Description = "n/a, module not loaded";
			r.ModuleName = "n/a, module not loaded";
			r.Disabled = itr->second;

			m_KnownMods.push_back(r);
		}
	}

	// Sort the list
	std::sort(m_KnownMods.begin(), m_KnownMods.end());

	// Fill the GUI listbox with loaded mod data
	for (int i = 0; i < m_KnownMods.size(); i++)
	{
		ModRecord r = m_KnownMods.at(i);
		m_pModManagerModsListBox->AddItem(MakeModString(r), "", 0, 0, i);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////
	// Load script data and fill the lists
	std::list<Entity *> globalScripts;
	g_PresetMan.GetAllOfType(globalScripts, "GlobalScript");

	for (std::list<Entity *>::iterator sItr = globalScripts.begin(); sItr != globalScripts.end(); ++sItr )
	{
		GlobalScript * script = dynamic_cast<GlobalScript *>(*sItr);
		if (script)
		{
			ScriptRecord r;
			r.PresetName = script->GetModuleAndPresetName();
			r.Description = script->GetDescription();
			r.Enabled = g_SettingsMan.IsScriptEnabled(r.PresetName);
			m_KnownScripts.push_back(r);
		}
	}

	// Sort the list
	std::sort(m_KnownScripts.begin(), m_KnownScripts.end());

	// Fill the GUI listbox with loaded mod data
	for (int i = 0; i < m_KnownScripts.size(); i++)
	{
		ScriptRecord r = m_KnownScripts.at(i);
		m_pModManagerScriptsListBox->AddItem(MakeScriptString(r), "", 0, 0, i);
	}

    // Read all the credits from the file and set the credits label
	m_CreditsLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CreditsLabel"));
    Reader creditsReader("Credits.txt");
    std::string creditsText = creditsReader.WholeFileAsString();

// TODO: Get Unicode going!
    // Hack here to change the special characters over 128 in the ansi ascii table to match our font files
    for (string::iterator sItr = creditsText.begin(); sItr != creditsText.end(); ++sItr)
    {
        if (*sItr == -60)//'Ä')
            (*sItr) = (char)142;
        if (*sItr == -42)//'Ö')
            (*sItr) = (char)153;
        if (*sItr == -87)//'©')
            (*sItr) = (char)221;
    }
	m_CreditsLabel->SetText(creditsText);
	m_CreditsLabel->ResizeHeightToFit();

    // Set initial focus, category list, and label settings
    m_ScreenChange = true;
    m_FocusChange = 1;
//    CategoryChange();

	m_PioneerPromoVisible = false;

	// Load the different input device icons. This can't be done during UInputMan::Create() because the icon presets don't exist so we need to do this after modules are loaded.
	g_UInputMan.LoadDeviceIcons();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MainMenuGUI object.

void MainMenuGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    // Delete only the array, not the bitmaps themselves, we don't own them
    delete [] m_aDPadBitmaps;
    delete [] m_aDualAnalogBitmaps;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGUIControlManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the GUIControlManager owned and used by this.

GUIControlManager * MainMenuGUI::GetGUIControlManager()
{
    return m_pGUIController;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void MainMenuGUI::SetEnabled(bool enable)
{
    if (enable && m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING)
    {
        m_MenuEnabled = ENABLING;
        g_GUISound.EnterMenuSound()->Play();
    }
    else if (!enable && m_MenuEnabled != DISABLED && m_MenuEnabled != DISABLING)
    {
        m_MenuEnabled = DISABLING;
        g_GUISound.ExitMenuSound()->Play();
    }

    m_ScreenChange = true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void MainMenuGUI::Update()
{
    // Update the input controller
    m_pController->Update();

    // Reset the specific triggers
    m_ScenarioStarted = false;
    m_CampaignStarted = false;
    m_ActivityRestarted = false;
    m_ActivityResumed = false;
    m_StartDifficulty = 0;
    m_Quit = false;

    // Don't update the main menu if the console is open
    if (g_ConsoleMan.IsEnabled())
        return;

    // If esc pressed, show quit dialog if applicable
	if (g_UInputMan.KeyPressed(KEY_ESC)) {
		if (m_MenuScreen == OPTIONSSCREEN || m_MenuScreen == MODMANAGERSCREEN || m_MenuScreen == EDITORSCREEN || m_MenuScreen == CREDITSSCREEN) {
			HideAllScreens();
			m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
			m_MenuScreen = MAINSCREEN;
			m_ScreenChange = true;
			g_GUISound.BackButtonPressSound()->Play();

			if (m_MenuScreen == OPTIONSSCREEN) {
				g_SettingsMan.SetFlashOnBrainDamage(m_aOptionsCheckbox[FLASHONBRAINDAMAGE]->GetCheck());
				g_SettingsMan.SetBlipOnRevealUnseen(m_aOptionsCheckbox[BLIPONREVEALUNSEEN]->GetCheck());
				g_SettingsMan.SetShowForeignItems(m_aOptionsCheckbox[SHOWFOREIGNITEMS]->GetCheck());
				g_SettingsMan.SetShowToolTips(m_aOptionsCheckbox[SHOWTOOLTIPS]->GetCheck());
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
    if (m_MenuEnabled != ENABLED &&  m_MenuEnabled != ENABLING)
        return;

	//////////////////////////////////////
    // MAINSCREEN MENU SCREEN

    if (m_MenuScreen == MAINSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[MAINSCREEN]->SetVisible(true);

            if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing))
            {
                m_apScreenBox[MAINSCREEN]->Resize(128, 220);
                m_MainMenuButtons[RESUME]->SetVisible(true);
            }
            else
            {
                m_apScreenBox[MAINSCREEN]->Resize(128, 196);
                m_MainMenuButtons[RESUME]->SetVisible(false);
            }
            // Restore the label on the campaign button
            m_MainMenuButtons[CAMPAIGN]->SetText("Metagame (WIP)");

            m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
            m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(false);
            m_MainMenuButtons[METACONTINUE]->SetVisible(false);
            m_ScreenChange = false;
        }
    
        // Blink the resume button to show the game is still going
        if (m_MainMenuButtons[RESUME]->GetVisible())
        {
            if (m_BlinkTimer.AlternateReal(500))
                m_MainMenuButtons[RESUME]->SetFocus();
            else
                m_pGUIController->GetManager()->SetFocus(0);
        }
		
		// Detect whether Planetoid Pioneers promo was clicked and open 
		if (m_PioneerPromoVisible)
		{
			bool buttonHeld = g_UInputMan.MouseButtonPressed(MOUSE_LEFT, -1);
			if (buttonHeld)
			{
				// Get mouse position
				int mouseX, mouseY;
				m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
				Vector mouse(mouseX, mouseY);

				if (m_PioneerPromoBox.IsWithinBox(mouse))
				{
					System::OpenBrowserToURL("http://store.steampowered.com/app/300260/");
				}
			}
		}
    }

    //////////////////////////////////////
    // PLAYERS MENU SCREEN

    else if (m_MenuScreen == PLAYERSSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[PLAYERSSCREEN]->SetVisible(true);
            m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
            m_ScreenChange = false;
        }

//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////
    // SKIRMISH SETUP MENU SCREEN

    else if (m_MenuScreen == SKIRMISHSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[SKIRMISHSCREEN]->SetVisible(true);
            // Set up the list of scenes to choose from
            UpdateScenesBox();
//            m_pGUIController->GetControl("ButtonStartSkirmish")->SetVisible(true);
            UpdateTeamBoxes();
            // Move main menu button over so the start button fits
			m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(200, 280);
            m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
            m_ScreenChange = false;
        }

//        for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box)
//            m_aSkirmishBox[box] = 0;

//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
    }

	//////////////////////////////////////
    // DIFFICULTY MENU SCREEN

    else if (m_MenuScreen == DIFFICULTYSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[DIFFICULTYSCREEN]->SetVisible(true);
            m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
            m_ScreenChange = false;
        }

//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////
    // OPTIONS MENU SCREEN

    else if (m_MenuScreen == OPTIONSSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[OPTIONSSCREEN]->SetVisible(true);
            m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
			m_apScreenBox[OPTIONSSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
			m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(180, 220);
            m_pBackToOptionsButton->SetVisible(false);
            UpdateDeviceLabels();
            m_ScreenChange = false;
        }
    }

    //////////////////////////////////////
    // CONFIGURATION SCREEN

    else if (m_MenuScreen == CONFIGSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[CONFIGSCREEN]->SetVisible(true);
            m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);
            m_pBackToOptionsButton->SetVisible(true);
            // Let this pass through, UpdateConfigScreen uses it
            //m_ScreenChange = false;
        }

        // Continuously update the contents through all the config steps
        UpdateConfigScreen();
    }

    //////////////////////////////////////
    // MOD MANAGER SCREEN

    else if (m_MenuScreen == MODMANAGERSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[MODMANAGERSCREEN]->SetVisible(true);
        }
    }

    //////////////////////////////////////
    // EDITOR MENU SCREEN

	else if (m_MenuScreen == EDITORSCREEN) {
		if (m_ScreenChange) {
			m_apScreenBox[EDITORSCREEN]->SetVisible(true);
			m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
			m_apScreenBox[EDITORSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
			m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(4, 145);
			m_ScreenChange = false;
		}
	}

    //////////////////////////////////////
    // CREDITS MENU SCREEN

	else if (m_MenuScreen == CREDITSSCREEN) {
		if (m_ScreenChange) {
			m_apScreenBox[CREDITSSCREEN]->SetVisible(true);
			m_MainMenuButtons[BACKTOMAIN]->SetVisible(true);
			m_apScreenBox[CREDITSSCREEN]->GUIPanel::AddChild(m_MainMenuButtons[BACKTOMAIN]);
			m_MainMenuButtons[BACKTOMAIN]->SetPositionRel(240, 298);
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
        if (m_ScrollTimer.IsPastRealMS(scrollTime))
        {
			m_CreditsLabel->SetPositionRel(0, m_pScrollPanel->GetHeight());
            m_ScrollTimer.Reset();
        }

//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////
    // METAGAME NOTICE SCREEN

    else if (m_MenuScreen == METASCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[METASCREEN]->SetVisible(true);
            m_MainMenuButtons[PLAYTUTORIAL]->SetVisible(true);
            m_MainMenuButtons[METACONTINUE]->SetVisible(true);
            m_pMetaNoticeLabel->SetText("- A T T E N T I O N -\n\nPlease note that the Campaign is in an INCOMPLETE, fully playable, yet still imperfect state!\nAs such, it is lacking some polish, audio, and game balancing, and we will be upgrading it significantly in future.\nThat said, you can absolutely enjoy fighting the A.I. and/or up to three friends in co-op, 2 vs 2, etc.\n\nAlso, if you have not yet played Cortex Command, we recommend you first try the tutorial:");
            m_pMetaNoticeLabel->SetVisible(true);
            // Flag that this notice has now been shown once, so no need to keep showing it
            m_TutorialOffered = true;
            m_ScreenChange = false;
        }

//        m_MainMenuButtons[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////
    // QUIT CONFIRM SCREEN

    else if (m_MenuScreen == QUITSCREEN)
    {
        if (m_ScreenChange)
        {
            m_apScreenBox[QUITSCREEN]->SetVisible(true);
            m_ScreenChange = false;
        }

//        m_MainMenuButtons[QUITCONFIRM]->SetFocus();
    }

    //////////////////////////////////////////
    // Update the ControlManager

    m_pGUIController->Update();

    ///////////////////////////////////////
    // Handle events

	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
        // Commands
		if (anEvent.GetType() == GUIEvent::Command)
        {
			// Campaign button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[CAMPAIGN])
            {
/*
                // Disable the campaign button for now
                if (m_MainMenuButtons[CAMPAIGN]->GetText() == "Campaign")
                {
                    m_MainMenuButtons[CAMPAIGN]->SetText("COMING SOON!");
                    g_GUISound.ExitMenuSound()->Play();
                }
                else
                {
                    m_MainMenuButtons[CAMPAIGN]->SetText("Campaign");
                    g_GUISound.ButtonPressSound()->Play();
                }
*/
                // Show the metagame notice screen if it hasn't already been shown yet
                if (!m_TutorialOffered)
                    m_MenuScreen = METASCREEN;
                // Start the campaign right away!
                else
                {
                    m_CampaignStarted = true;
                    m_MenuScreen = MAINSCREEN;
                }

                HideAllScreens();
                m_ScreenChange = true;
                g_GUISound.ButtonPressSound()->Play();
            }

			// Skirmish button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[SKIRMISH])
            {
                m_ScenarioStarted = true;
                m_CampaignStarted = false;

                if (g_MetaMan.GameInProgress())
					g_MetaMan.EndGame();

                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
//                m_MenuScreen = PLAYERSSCREEN;
                m_MenuScreen = MAINSCREEN;
                m_ScreenChange = true;
                g_GUISound.ButtonPressSound()->Play();
//                g_GUISound.ExitMenuSound()->Play();
            }

			if (anEvent.GetControl() == m_MainMenuButtons[MULTIPLAYER])
			{
				m_ScenarioStarted = true;
				m_CampaignStarted = false;

				if (g_MetaMan.GameInProgress())
					g_MetaMan.EndGame();

				// Hide all screens, the appropriate screen will reappear on next update
				//HideAllScreens();
				//                m_MenuScreen = PLAYERSSCREEN;
				//m_MenuScreen = MAINSCREEN;
				//m_ScreenChange = true;
				g_GUISound.ButtonPressSound()->Play();
				//                g_GUISound.ExitMenuSound()->Play();

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
			if (anEvent.GetControl() == m_MainMenuButtons[OPTIONS])
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = OPTIONSSCREEN;
                m_ScreenChange = true;

                g_GUISound.ButtonPressSound()->Play();
            }

			// Editor button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[EDITOR])
            {
                m_CampaignStarted = false;

                if (g_MetaMan.GameInProgress())
					g_MetaMan.EndGame();

                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = EDITORSCREEN;
                m_ScreenChange = true;

                g_GUISound.ButtonPressSound()->Play();
//                g_GUISound.UserErrorSound()->Play();
            }

			// Editor button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[MODMANAGER])
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = MODMANAGERSCREEN;
                m_ScreenChange = true;

                g_GUISound.ButtonPressSound()->Play();
            }

			// Credits button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[CREDITS])
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = CREDITSSCREEN;
                m_ScreenChange = true;

                g_GUISound.ButtonPressSound()->Play();
            }

			// Quit button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[QUIT])
            {
                QuitLogic();
                g_GUISound.ButtonPressSound()->Play();
            }

			// Resume button pressed
			if (anEvent.GetControl() == m_MainMenuButtons[RESUME])
            {
                m_ActivityResumed = true;

                g_GUISound.ExitMenuSound()->Play();
            }

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
						g_FrameMan.SwitchResolution(960,540);
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
			if (anEvent.GetControl() == m_MainMenuButtons[BACKTOMAIN])
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MainMenuButtons[BACKTOMAIN]->SetVisible(false);

                // If leaving the options screen, save the settings!
                if (m_MenuScreen == OPTIONSSCREEN)
				{
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
			if (anEvent.GetControl() == m_pBackToOptionsButton)
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_pBackToOptionsButton->SetVisible(false);
                m_MenuScreen = OPTIONSSCREEN;
                m_ScreenChange = true;

                g_GUISound.BackButtonPressSound()->Play();
            }

            /////////////////////////////////////////////
            // PLAYER SCREEN BUTTONS
			// Player count setting button pressed

			if (m_MenuScreen == PLAYERSSCREEN && 
                (anEvent.GetControl()->GetName() == "ButtonOnePlayer" ||
                 anEvent.GetControl()->GetName() == "ButtonTwoPlayers" ||
                 anEvent.GetControl()->GetName() == "ButtonThreePlayers" ||
                 anEvent.GetControl()->GetName() == "ButtonFourPlayers"))
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = SKIRMISHSCREEN;
                m_ScreenChange = true;

                // Set desired player count
                if (anEvent.GetControl()->GetName() == "ButtonOnePlayer")
                    m_StartPlayers = 1;
                else if (anEvent.GetControl()->GetName() == "ButtonTwoPlayers")
                    m_StartPlayers = 2;
                else if (anEvent.GetControl()->GetName() == "ButtonThreePlayers")
                    m_StartPlayers = 3;
                else if (anEvent.GetControl()->GetName() == "ButtonFourPlayers")
                    m_StartPlayers = 4;
                else
                    m_StartPlayers = 0;

                g_GUISound.ButtonPressSound()->Play();
            }

            /////////////////////////////////////////////
            // SKIRMISH SETUP SCREEN BUTTONS

			if (m_MenuScreen == SKIRMISHSCREEN)
            {
                for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player)
                {
                    // Player team toggle button
			        if (anEvent.GetControl() == m_aSkirmishButton[player])
                    {
                        // Toggle
                        if (m_aTeamAssignments[player] == Activity::TeamOne)
                            m_aTeamAssignments[player] = Activity::TeamTwo;
                        else
                            m_aTeamAssignments[player] = Activity::TeamOne;

                        UpdateTeamBoxes();

                        g_GUISound.ButtonPressSound()->Play();
                    }
                }

			    // Start Skirmish menu button pressed
			    if (anEvent.GetControl()->GetName() == "ButtonStartSkirmish")
                {
                    // Hide all screens, the appropriate screen will reappear on next update
                    HideAllScreens();
                    
                    // No CPU team, so just start game
                    if (m_CPUTeam < 0)
                    {
                        m_MenuScreen = MAINSCREEN;
                        m_ScreenChange = true;
                        m_ActivityRestarted = true;
                        SetupSkirmishActivity();
                        g_GUISound.ExitMenuSound()->Play();
                    }
                    // CPU team present, so ask for the difficulty level of it before starting
                    else
                    {
                        m_MenuScreen = DIFFICULTYSCREEN;
                        m_ScreenChange = true;
                        g_GUISound.ButtonPressSound()->Play();
                    }
                }
            }

            /////////////////////////////////////////////
            // DIFFICULTY SETUP SCREEN BUTTONS

			if (m_MenuScreen == DIFFICULTYSCREEN && 
                (anEvent.GetControl()->GetName() == "ButtonStartEasy" ||
                anEvent.GetControl()->GetName() == "ButtonStartMedium" ||
                anEvent.GetControl()->GetName() == "ButtonStartHard" ||
                anEvent.GetControl()->GetName() == "ButtonStartDeath"))
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = MAINSCREEN;
                m_ScreenChange = true;

                m_ActivityRestarted = true;

                // Set appropriate difficulty level
                if (anEvent.GetControl()->GetName() == "ButtonStartEasy")
                    m_StartDifficulty = Activity::EasyDifficulty;
                else if (anEvent.GetControl()->GetName() == "ButtonStartMedium")
                    m_StartDifficulty = Activity::MediumDifficulty;
                else if (anEvent.GetControl()->GetName() == "ButtonStartHard")
                    m_StartDifficulty = Activity::HardDifficulty;
                else if (anEvent.GetControl()->GetName() == "ButtonStartDeath")
                    m_StartDifficulty = Activity::MaxDifficulty;
                else
                    m_StartDifficulty = Activity::MediumDifficulty;

                SetupSkirmishActivity();

//                g_GUISound.BackButtonPressSound()->Play();
                g_GUISound.ExitMenuSound()->Play();
            }

            /////////////////////////////////////////////
            // CONTROL CONFIG UI

            // Control options
            if (m_MenuScreen == OPTIONSSCREEN)
            {
                int which = 0;
                int player = 0;

                // Handle all device select button pushes
                for (which = P1NEXT; which <= P4PREV; ++which)
                {
                    // Calculate the owner of the currently checked button, and if it's next/prev button
                    player = (which - P1NEXT) % Players::MaxPlayerCount;
                    bool nextButton = which < P1PREV;

                    // Handle the appropriate player's device setting
                    if (anEvent.GetControl() == m_aOptionButton[which])
                    {
                        // What's the current device
                        int currentDevice = g_UInputMan.GetControlScheme(player)->GetDevice();
                        // Next button pressed, so increment
                        if (nextButton)
                        {
                            // Loop around to first if we've gone around
                            if (++currentDevice >= DEVICE_COUNT)
                                currentDevice = 0;
                        }
                        // Prev button pressed, so decrement
                        else
                        {
                            // Loop around to last if we've gone around
                            if (--currentDevice < 0)
                                currentDevice = DEVICE_COUNT - 1;
                        }
                        // Set the device and update labels
                        g_UInputMan.GetControlScheme(player)->SetDevice(static_cast<InputDevice>(currentDevice));
                        UpdateDeviceLabels();

                        g_GUISound.ButtonPressSound()->Play();
                    }
                }

                // Handle all control config buttons
                for (which = P1CONFIG; which <= P4CONFIG; ++which)
                {
                    // Handle the appropriate player's device setting
                    if (anEvent.GetControl() == m_aOptionButton[which])
                    {
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
                for (which = P1CLEAR; which <= P4CLEAR; ++which)
                {
                    // Handle the appropriate player's clearing of mappings
                    if (anEvent.GetControl() == m_aOptionButton[which])
                    {
                        // Make user click twice to confirm
                        if (m_aOptionButton[which]->GetText() == "Reset")
                        {
                            // Ask to confirm!
                            m_aOptionButton[which]->SetText("CONFIRM?");
                            // And clear all other buttons of it
                            for (int otherButton = P1CLEAR; otherButton <= P4CLEAR; ++otherButton)
                                if (otherButton != which)
                                    m_aOptionButton[otherButton]->SetText("Reset");
                            g_GUISound.ButtonPressSound()->Play();
                        }
                        else
                        {
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

            if (m_MenuScreen == CONFIGSCREEN)
            {
			    // DPad Gamepad type selected
			    if (anEvent.GetControl() == m_pDPadTypeButton)
                {
                    m_ConfiguringGamepad = DPAD;
                    m_ConfigureStep++;
                    m_ScreenChange = true;

                    g_GUISound.ButtonPressSound()->Play();
                }

			    // DPad Gamepad type selected
			    if (anEvent.GetControl() == m_pDAnalogTypeButton)
                {
                    m_ConfiguringGamepad = DANALOG;
                    m_ConfigureStep++;
                    m_ScreenChange = true;

                    g_GUISound.ButtonPressSound()->Play();
                }

			    // XBox Gamepad type selected
			    if (anEvent.GetControl() == m_pXBox360TypeButton)
                {
/* Not allowing config, this is a complete preset
                    m_ConfiguringGamepad = XBOX360;
                    m_ConfigureStep++;
                    m_ScreenChange = true;
*/
                    // Set up the preset that will work well for a 360 controller
                    g_UInputMan.GetControlScheme(m_ConfiguringPlayer)->SetPreset(PRESET_XBOX360);

                    // Go back to the options screen immediately since the preset is all taken care of
                    m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
                    m_MenuScreen = OPTIONSSCREEN;
                    m_ScreenChange = true;

                    g_GUISound.ExitMenuSound()->Play();
                }

			    // Skip ahead one config step button pressed
			    if (anEvent.GetControl() == m_pConfigSkipButton)
                {
// TODO: error checking so that we don't put configurestep out of bounds!
                    m_ConfigureStep++;
                    m_ScreenChange = true;

                    g_GUISound.ButtonPressSound()->Play();
                }

			    // Go back one config step button pressed
			    if (anEvent.GetControl() == m_pConfigBackButton)
                {
                    if (m_ConfigureStep > 0)
                    {
                        m_ConfigureStep--;
                        m_ScreenChange = true;
                        g_GUISound.BackButtonPressSound()->Play();
                    }
                    else
                        g_GUISound.UserErrorSound()->Play();
                }
            }

            /////////////////////////////////////////////
            // EDITOR SCREEN BUTTONS

			if (m_MenuScreen == EDITORSCREEN &&
                (anEvent.GetControl() == m_aEditorButton[SCENEEDITOR] ||
                 anEvent.GetControl() == m_aEditorButton[AREAEDITOR] ||
                 anEvent.GetControl() == m_aEditorButton[ASSEMBLYEDITOR] ||
                 anEvent.GetControl() == m_aEditorButton[GIBEDITOR] ||
                 anEvent.GetControl() == m_aEditorButton[ACTOREDITOR]))
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = MAINSCREEN;
                m_ScreenChange = true;

                m_ActivityRestarted = true;
// TEMP ugly hack to get the damn main menu to hurry up when starting to edit
//                g_IntroState = 20;
//                set_palette(;

                // Create and start the appropriate editor Activity
				if (anEvent.GetControl() == m_aEditorButton[SCENEEDITOR]) {
					StartSceneEditor();
				} else if (anEvent.GetControl() == m_aEditorButton[AREAEDITOR]) {
					StartAreaEditor();
				} else if (anEvent.GetControl() == m_aEditorButton[ASSEMBLYEDITOR]) {
					StartAssemblyEditor();
				} else if (anEvent.GetControl() == m_aEditorButton[GIBEDITOR]) {
					StartGibEditor();
				} else if (anEvent.GetControl() == m_aEditorButton[ACTOREDITOR]) {
					StartActorEditor();
				}

//                g_GUISound.BackButtonPressSound()->Play();
                g_GUISound.ExitMenuSound()->Play();
            }

            /////////////////////////////////////////////
            // META NOTICE SCREEN BUTTONS

			if (m_MenuScreen == METASCREEN)
            {
                // Play tutorial button pressed
                if (anEvent.GetControl() == m_MainMenuButtons[PLAYTUTORIAL])
                {
                    // Hide all screens, the appropriate screen will reappear on next update
                    HideAllScreens();
                    m_MenuScreen = MAINSCREEN;
                    m_ScreenChange = true;

                    // Set up and start the tutorial!
                    g_ActivityMan.SetStartActivity(dynamic_cast<Activity *>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
					GameActivity * pGameActivity = dynamic_cast<GameActivity *>(g_ActivityMan.GetStartActivity());
					if (pGameActivity)
						pGameActivity->SetStartingGold(10000);
                    g_SceneMan.SetSceneToLoad("Tutorial Bunker");
                    m_ActivityRestarted = true;

                    g_GUISound.ButtonPressSound()->Play();
                }
                // Go to registration dialog button
                else if (anEvent.GetControl() == m_MainMenuButtons[METACONTINUE])
                {
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

			if (m_MenuScreen == MODMANAGERSCREEN)
            {
				// Return to main menu button pressed
				if (anEvent.GetControl() == m_pModManagerBackButton)
				{
					// Save settings
					g_SettingsMan.UpdateSettingsFile();

					// Hide all screens, the appropriate screen will reappear on next update
					HideAllScreens();
					m_MenuScreen = MAINSCREEN;
					m_ScreenChange = true;
					g_GUISound.BackButtonPressSound()->Play();
				}

				// Disable/Enable mod pressed
				if (anEvent.GetControl() == m_pModManagerToggleModButton)
				{
					ToggleMod();
				}

				// Disable/Enable script pressed
				if (anEvent.GetControl() == m_pModManagerToggleScriptButton)
				{
					ToggleScript();
				}
			}

            /////////////////////////////////////////////
            // QUIT SCREEN BUTTONS

			if (m_MenuScreen == QUITSCREEN)
            {
                // Confirm quitting of game
                if (anEvent.GetControl() == m_MainMenuButtons[QUITCONFIRM])
                {
                    m_Quit = true;

                    // Hide all screens, the appropriate screen will reappear on next update
                    HideAllScreens();
                    m_ScreenChange = true;

                    g_GUISound.ButtonPressSound()->Play();
                }
                // Cancel quitting
                else if (anEvent.GetControl() == m_MainMenuButtons[QUITCANCEL])
                {
                    // Hide all screens, the appropriate screen will reappear on next update
                    HideAllScreens();
                    m_MenuScreen = MAINSCREEN;
                    m_ScreenChange = true;

                    g_GUISound.ButtonPressSound()->Play();
                }
            }
        }

		// Notifications
		else if (anEvent.GetType() == GUIEvent::Notification)
        {
            // Button focus notification that we can play a sound to
            if (dynamic_cast<GUIButton *>(anEvent.GetControl()))
            {
                if (anEvent.GetMsg() == GUIButton::Focused)
                    g_GUISound.SelectionChangeSound()->Play();
            }

			// Mod list pressed
			if (anEvent.GetControl() == m_pModManagerModsListBox)
			{
				if (anEvent.GetMsg() == GUIListBox::Select)
				{
					int index = m_pModManagerModsListBox->GetSelectedIndex();
					if (index > -1)
					{
						GUIListPanel::Item *selectedItem = m_pModManagerModsListBox->GetSelected();
						ModRecord r = m_KnownMods.at(selectedItem->m_ExtraIndex);
						m_pModManagerDescriptionLabel->SetText(r.Description);

						if (r.Disabled)
							m_pModManagerToggleModButton->SetText("Enable");
						else
							m_pModManagerToggleModButton->SetText("Disable");
					}
				}

				// Not reliable at all!!
				/*if (anEvent.GetMsg() == GUIListBox::DoubleClick)
				{
					ToggleMod();
				}*/
			}

			// Script list pressed
			if (anEvent.GetControl() == m_pModManagerScriptsListBox)
			{
				if (anEvent.GetMsg() == GUIListBox::Select)
				{
					int index = m_pModManagerScriptsListBox->GetSelectedIndex();
					if (index > -1)
					{
						GUIListPanel::Item *selectedItem = m_pModManagerScriptsListBox->GetSelected();
						ScriptRecord r = m_KnownScripts.at(selectedItem->m_ExtraIndex);
						m_pModManagerDescriptionLabel->SetText(r.Description);

						if (r.Enabled)
							m_pModManagerToggleScriptButton->SetText("Disable");
						else
							m_pModManagerToggleScriptButton->SetText("Enable");
					}
				}

				/*if (anEvent.GetMsg() == GUIListBox::DoubleClick)
				{
					ToggleScript();
				}*/
			}

            // Resolution combobox closed, something new selected
    		if (anEvent.GetControl() == m_pResolutionCombo)
            {
                // Closed it, IE selected somehting
                if(anEvent.GetMsg() == GUIComboBox::Closed)
                {
                    // Get and read the new resolution data from the item's label
                    GUIListPanel::Item *pResItem = m_pResolutionCombo->GetItem(m_pResolutionCombo->GetSelectedIndex());
                    if (pResItem && !pResItem->m_Name.empty())
                    {
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
			if(anEvent.GetControl() == m_pSoundSlider)
            {
                // See if we should play test sound after the volume has been set
                bool playTest = false;
                if (((double)m_pSoundSlider->GetValue() / 100) != g_AudioMan.GetSoundsVolume() && !g_GUISound.TestSound()->IsBeingPlayed())
                    playTest = true;

                g_AudioMan.SetSoundsVolume((double)m_pSoundSlider->GetValue() / 100);
                UpdateVolumeSliders();

                // Play test sound after new volume is set
                if (playTest)
                    g_GUISound.TestSound()->Play();
			}

			// Music Volume slider changed
			if(anEvent.GetControl() == m_pMusicSlider)
            {
                g_AudioMan.SetMusicVolume((double)m_pMusicSlider->GetValue() / 100);
                UpdateVolumeSliders();
			}

			// Dead zone sliders control
			for (int which = P1DEADZONESLIDER; which < DEADZONESLIDERCOUNT; ++which)
			{
				// Handle the appropriate player's clearing of mappings
				if (anEvent.GetControl() == m_aDeadZoneSlider[which])
				{
					// Display value
					char s[256];
					std::snprintf(s, sizeof(s), "%d", m_aDeadZoneSlider[which]->GetValue());
					m_aDeadZoneLabel[which]->SetText(s);

					// Update control scheme
					g_UInputMan.GetControlScheme(which)->SetJoystickDeadzone((float)m_aDeadZoneSlider[which]->GetValue() / 200.0F);
				}

				if (anEvent.GetControl() == m_aDeadZoneCheckbox[which])
				{
					if (m_aDeadZoneCheckbox[which]->GetCheck() == 1)
					{
						g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::CIRCLE);
						m_aDeadZoneCheckbox[which]->SetText("O");
					}
					else 
					{
						g_UInputMan.GetControlScheme(which)->SetJoystickDeadzoneType(DeadZoneType::SQUARE);
						char str[2];
						str[0] = -2;
						str[1] = 0;
						m_aDeadZoneCheckbox[which]->SetText(str);
					}
				}
			}
/*
			// Scrollbar changed
			if(anEvent.GetControl()->GetName() == "scroll1")
            {
				int Value = 0;
				GUIScrollbar *S = (GUIScrollbar *)anEvent.GetControl();
				Value = S->GetValue();
				GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
				char buf[64];
				L->SetText(itoa(Value, buf, 10));
			}

			// Double click on the listbox
			if(anEvent.GetControl()->GetName() == "list1")
            {
				if(anEvent.GetMsg() == GUIListBox::DoubleClicked)
                {
					GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
					GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
					if(LB->GetSelected())
						L->SetText(LB->GetSelected()->m_Name);
				}

				if(anEvent.GetMsg() == GUIListBox::KeyDown)
                {
					// Delete
					if(anEvent.GetData() == GUIInput::Key_Delete)
                    {
						GUIListBox *LB = (GUIListBox *)anEvent.GetControl();
						LB->DeleteItem(LB->GetSelectedIndex());
					}
				}
			}

			if(anEvent.GetControl()->GetName() == "text1")
            {
				if(anEvent.GetMsg() == GUITextBox::Enter)
                {
					GUILabel *L = (GUILabel *)m_pGUIController->GetControl("label1");
					GUITextBox *T = (GUITextBox *)anEvent.GetControl();

					L->SetText(T->GetText());
				}
			}
*/
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void MainMenuGUI::Draw(BITMAP *drawBitmap) const
{
    AllegroScreen drawScreen(drawBitmap);
    m_pGUIController->Draw(&drawScreen);
    m_pGUIController->DrawMouse();

	// Show who controls the cursor
	int device = g_UInputMan.GetLastDeviceWhichControlledGUICursor();

	if (device >= DEVICE_GAMEPAD_1)
	{
		int mouseX, mouseY;
		m_pGUIInput->GetMousePosition(&mouseX, &mouseY);

		const Icon * pIcon = g_UInputMan.GetDeviceIcon(device);
		if (pIcon)
			draw_sprite(drawBitmap, pIcon->GetBitmaps8()[0], mouseX + 16, mouseY - 4);

/*#ifdef DEBUG_BUILD
		if (g_UInputMan.JoystickActive(0))
		{
			Vector aim = g_UInputMan.AnalogAimValues(0);
			float axis00 = g_UInputMan.AnalogAxisValue(0, 0, 0);
			float axis01 = g_UInputMan.AnalogAxisValue(0, 0, 1);

			float axis10 = g_UInputMan.AnalogAxisValue(0, 1, 0);
			float axis11 = g_UInputMan.AnalogAxisValue(0, 1, 1);
			char s[256];
			std::snprintf(s, sizeof(s), "Aim %.1f %.1f - Stick 0 %.1f %.1f - Stick 1 %.1f %.1f", aim.GetX(), aim.GetY(), axis00, axis01, axis10, axis11);

			GUILabel * debugLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LabelDebug"));
			if (debugLabel)
				debugLabel->SetText(s);
		}
#endif*/

	}

	// Show which joysticks are detected by the game
	for (int joystick = Players::PlayerOne; joystick < Players::MaxPlayerCount; joystick++)
	{
		if (g_UInputMan.JoystickActive(joystick))
		{
			int matchedDevice = DEVICE_GAMEPAD_1 + joystick;

			if (matchedDevice != device)
			{
				const Icon * pIcon = g_UInputMan.GetDeviceIcon(matchedDevice);
				if (pIcon)
					draw_sprite(drawBitmap, pIcon->GetBitmaps8()[0], g_FrameMan.GetResX() - 30 * g_UInputMan.GetJoystickCount() + 30 * joystick, g_FrameMan.GetResY() - 25);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideAllScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides all menu screens, so one can easily be unhidden and shown only.

void MainMenuGUI::HideAllScreens()
{
    for (int iscreen = MAINSCREEN; iscreen < SCREENCOUNT; ++iscreen)
    {
        if (m_apScreenBox[iscreen])
            m_apScreenBox[iscreen]->SetVisible(false);
    }
    m_ScreenChange = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          QuitLogic
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles quitting of the game.

void MainMenuGUI::QuitLogic()
{
    // If quit confirm dialog not already showing, or an activity is running, show it
    if (m_MenuScreen != QUITSCREEN && g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing))
    {
        HideAllScreens();
        m_MenuScreen = QUITSCREEN;
        m_ScreenChange = true;
    }
    // No activity, so just start quitting
    else
    {
        m_Quit = true;

        // Hide all screens, the appropriate screen will reappear on next update
        HideAllScreens();
        m_ScreenChange = true;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSkirmishActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ActivityMan up with the current data for a skirmish game.

void MainMenuGUI::SetupSkirmishActivity()
{
    // If activity restarted, stuff the ActivityMan with the selected data
    if (m_ActivityRestarted)
    {
// TODO: ******* add the game mode drop down and base the game mode selection off that instead

        // No CPU team, so Brain match
        if (m_CPUTeam == Activity::NoTeam)
        {
            g_SceneMan.SetSceneToLoad(m_pSceneSelector->GetItem(m_pSceneSelector->GetSelectedIndex())->m_Name);
// TODO: Let player choose the GABrainMatch activity instance!
            GABrainMatch *pNewGame = new GABrainMatch;

            for (int player = Players::PlayerOne; player < m_StartPlayers; ++player)
                pNewGame->SetTeamOfPlayer(player, m_aTeamAssignments[player]);

            pNewGame->SetCPUTeam(m_CPUTeam);
            pNewGame->Create();
            g_ActivityMan.SetStartActivity(pNewGame);
        }
        // CPU present, so base defense
        else
        {
            g_SceneMan.SetSceneToLoad(m_pSceneSelector->GetItem(m_pSceneSelector->GetSelectedIndex())->m_Name);
// TODO: Let player choose the GABaseDefense activity instance!
            GABaseDefense *pNewGame = dynamic_cast<GABaseDefense *>(g_PresetMan.GetEntityPreset("GABaseDefense", "Skirmish Defense")->Clone());
            RTEAssert(pNewGame, "Couldn't find the \"Skirmish Defense\" GABaseDefense Activity! Has it been defined?");

            for (int player = Players::PlayerOne; player < m_StartPlayers; ++player)
                pNewGame->SetTeamOfPlayer(player, m_aTeamAssignments[player]);

            pNewGame->SetCPUTeam(m_CPUTeam);
            pNewGame->SetDifficulty(m_StartDifficulty);

            pNewGame->Create();
            g_ActivityMan.SetStartActivity(pNewGame);
        }

// TODO: Reenable and make GUI control for this!
/*
        g_ActivityMan.GetActivity()->SetStartingFunds(m_StartFunds);
        for (int team = 0; team < m_StartTeams; ++team)
        {
            g_ActivityMan.GetActivity()->SetTeamFunds(m_StartFunds, team);
        }
*/
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateScenesBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the scene selection box.

void MainMenuGUI::UpdateScenesBox()
{
    // Clear out the control
    m_pSceneSelector->ClearList();

    // Get the list of all read in scenes
    list<Entity *> sceneList;
    g_PresetMan.GetAllOfType(sceneList, "Scene");

    // Go through the list and add their names to the combo box
    for (list<Entity *>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr)
    {
        m_pSceneSelector->AddItem((*itr)->GetPresetName());
    }

    // Select the first one
    m_pSceneSelector->SetSelectedIndex(0);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTeamBoxes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the size and contents of the team assignment boxes, according
//                  to the number of players chosen.

void MainMenuGUI::UpdateTeamBoxes()
{
    char str[128];

    // Hide all team assignment panels initially, and center their contents
    for (int box = 0; box < SKIRMISHPLAYERCOUNT; ++box)
    {
        m_aSkirmishBox[box]->SetVisible(false);
        m_aSkirmishButton[box]->CenterInParent(true, true);
    }

    // Total area size
    int areaWidth = m_pTeamBox->GetWidth();
    int areaHeight = m_pTeamBox->GetHeight();

    // Set up the team assignment boxes and contained labels and buttons
    // Single team to set up for single player
    if (m_StartPlayers == 1)
    {
        // Show and resize
        m_aSkirmishBox[P1TEAM]->Resize(areaWidth, areaHeight);
        m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
        m_aSkirmishBox[P1TEAM]->SetVisible(true);
    }
    // Two player split one above the other
    else if (m_StartPlayers == 2)
    {
        int boxHeight = (areaHeight - 4) / 2;

        // Player 1
        m_aSkirmishBox[P1TEAM]->Resize(areaWidth, boxHeight);
        m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
        m_aSkirmishBox[P1TEAM]->SetVisible(true);

        // Player 2
        m_aSkirmishBox[P2TEAM]->Resize(areaWidth, boxHeight);
        m_aSkirmishBox[P2TEAM]->SetPositionRel(0, boxHeight + 4);
        m_aSkirmishButton[P2TEAM]->CenterInParent(true, true);
        m_aSkirmishBox[P2TEAM]->SetVisible(true);
    }
    // Four-way split, either three or four players
    else if (m_StartPlayers >= 3)
    {
        int boxWidth = (areaWidth - 4) / 2;
        int boxHeight = (areaHeight - 4) / 2;

        // Player 1
        m_aSkirmishBox[P1TEAM]->Resize(boxWidth, boxHeight);
        m_aSkirmishButton[P1TEAM]->CenterInParent(true, true);
        m_aSkirmishBox[P1TEAM]->SetVisible(true);

        // Player 2
        m_aSkirmishBox[P2TEAM]->Resize(boxWidth, boxHeight);
        m_aSkirmishBox[P2TEAM]->SetPositionRel(boxWidth + 4, 0);
        m_aSkirmishButton[P2TEAM]->CenterInParent(true, true);
        m_aSkirmishBox[P2TEAM]->SetVisible(true);

        // Player 3
        m_aSkirmishBox[P3TEAM]->SetVisible(true);

        // Player 4
        if (m_StartPlayers == 4)
            m_aSkirmishBox[P4TEAM]->SetVisible(true);
    }

    // Update button labels
    for (int player = Players::PlayerOne; player < SKIRMISHPLAYERCOUNT; ++player)
    {
        if (m_aTeamAssignments[player] == Activity::TeamOne)
        {
            m_aSkirmishBox[player]->SetDrawColor(makecol(70, 27, 12));
            std::snprintf(str, sizeof(str), "Player %i: %c", player + 1, -62);
        }
        else
        {
            m_aSkirmishBox[player]->SetDrawColor(makecol(47, 55, 40));
            std::snprintf(str, sizeof(str), "Player %i: %c", player + 1, -59);
        }
        m_aSkirmishButton[player]->SetText(str);
    }

    ////////////////////////////////
    // Update CPU team label

    // Count how many players on each team
    int team0Count = 0;
    int team1Count = 0;
    for (int player = Players::PlayerOne; player < m_StartPlayers; ++player)
    {
        if (m_aTeamAssignments[player] == 0)
            team0Count++;
        else if (m_aTeamAssignments[player] == 1)
            team1Count++;
    }

    // See if either team is empty of human players - that becomes the CPU team
    if (team0Count == 0 || team1Count == 0)
    {
        std::snprintf(str, sizeof(str), "CPU Team: %c", team0Count == 0 ? -62 : -59);
        m_CPUTeam = team0Count == 0 ? 0 : 1;
    }
    else
    {
        std::snprintf(str, sizeof(str), "No CPU Team (both have players)");
        m_CPUTeam = -1;
    }

    // Finally set the label
    m_pCPUTeamLabel->SetText(str);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateResolutionCombo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the screen resolution combo box

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
					if ((g_FrameMan.GetNewResX() * g_FrameMan.ResolutionMultiplier()) == width && (g_FrameMan.GetNewResY() * g_FrameMan.ResolutionMultiplier()) == height) {
						currentResIndex = foundIndex;
					}
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateVolumeSliders
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the position of the volume sliders, based on what the AudioMan
//                  is currently set to.

void MainMenuGUI::UpdateVolumeSliders()
{
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateDeviceLabels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text on the config buttons, based on what they are mapped as.

void MainMenuGUI::UpdateDeviceLabels()
{
    int device = 0;
    string label;

    // Cycle through all players
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        device = g_UInputMan.GetControlScheme(player)->GetDevice();

        if (device == DEVICE_KEYB_ONLY)
            label = "Classic Keyb";
        else if (device == DEVICE_MOUSE_KEYB)
            label = "Keyb + Mouse";
        else if (device == DEVICE_GAMEPAD_1)
            label = "Gamepad 1";
        else if (device == DEVICE_GAMEPAD_2)
            label = "Gamepad 2";
        else if (device == DEVICE_GAMEPAD_3)
            label = "Gamepad 3";
        else if (device == DEVICE_GAMEPAD_4)
            label = "Gamepad 4";

        // Set the label
        m_aOptionsLabel[P1DEVICE + player]->SetText(label);

        // Reset Config and Clear button labels
        m_aOptionButton[P1CONFIG + player]->SetText("Configure");
        m_aOptionButton[P1CLEAR + player]->SetText("Reset");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateConfigScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the control configuarion screen.

void MainMenuGUI::UpdateConfigScreen()
{
    char str[256];

    if (m_ScreenChange)
    {
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
    if (m_ConfiguringDevice == DEVICE_KEYB_ONLY)
    {
        if (m_ScreenChange)
        {
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
        if (m_ConfigureStep == 0)
        {
            // Hide the back button on this first step
            m_pConfigBackButton->SetVisible(false);

            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM UP");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Up Cursor]" : "[W]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM_UP))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_UP);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move/Aim down
        else if (m_ConfigureStep == 1)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM DOWN");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Down Cursor]" : "[S]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM_DOWN))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_DOWN);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move left
        else if (m_ConfigureStep == 2)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Left Cursor]" : "[A]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_LEFT))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move right
        else if (m_ConfigureStep == 3)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Right Cursor]" : "[D]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_RIGHT))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Fire
        else if (m_ConfigureStep == 4)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 1]" : "[H]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_FIRE))

            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Sharp aim
        else if (m_ConfigureStep == 5)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("SHARP AIM");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 2]" : "[J]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_AIM))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Inventory
        else if (m_ConfigureStep == 6)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 3]" : "[K]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PIEMENU))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Jump
        else if (m_ConfigureStep == 7)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("JUMP");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num Enter]" : "[L]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_JUMP))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Crouch
        else if (m_ConfigureStep == 8)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("CROUCH");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num Del]" : "[.]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_CROUCH))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Prev actor
        else if (m_ConfigureStep == 9)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 4]" : "[Q]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PREV))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Next actor
        else if (m_ConfigureStep == 10)
        {
            // Hide skip button on this last step
            m_pConfigSkipButton->SetVisible(false);

            m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 5]" : "[E]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_NEXT))
            {
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
        }
		// Reload
		else if (m_ConfigureStep == 11)
		{
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 0]" : "[R]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_RELOAD))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 12)
		{
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("PICK UP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 9]" : "[F]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_PICKUP))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 13)
		{
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("DROP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 6]" : "[G]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_DROP))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 14)
		{
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 7]" : "[X]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 15)
		{
			// Hide skip button on this last step
			m_pConfigSkipButton->SetVisible(false);

			m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText(m_ConfiguringPlayer % 2 ? "[Num 8]" : "[C]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT))
			{
				m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
				m_MenuScreen = OPTIONSSCREEN;
				m_ScreenChange = true;
			}
		}

/*
        // Start
        else if (m_ConfigureStep == 10)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_START))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Back
        else if (m_ConfigureStep == 11)
        {
            // Hide the skip button on this last step
            m_pConfigSkipButton->SetVisible(false);
            m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_BACK))
            {
                // Done, go back to options screen
                m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
                m_MenuScreen = OPTIONSSCREEN;
                m_ScreenChange = true;
            }
        }
*/      
    }
    // Mouse+keyb
    else if (m_ConfiguringDevice == DEVICE_MOUSE_KEYB)
    {
        if (m_ScreenChange)
        {
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
        if (m_ConfigureStep == 0)
        {
            // Hide the back button on this first step
            m_pConfigBackButton->SetVisible(false);

            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE UP or JUMP");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[W]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_UP))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_UP);
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_JUMP);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move down
        else if (m_ConfigureStep == 1)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE DOWN or CROUCH");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[S]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_DOWN))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_DOWN);
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_CROUCH);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move left
        else if (m_ConfigureStep == 2)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[A]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_LEFT))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_LEFT);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move right
        else if (m_ConfigureStep == 3)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[D]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_L_RIGHT))
            {
                g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_R_RIGHT);
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Move right
        else if (m_ConfigureStep == 4)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[R]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_RELOAD))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
		else if (m_ConfigureStep == 5)
		{
			m_pConfigLabel[CONFIGINPUT]->SetText("PICK UP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[F]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_PICKUP))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 6)
		{
			m_pConfigLabel[CONFIGINPUT]->SetText("DROP");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[G]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_DROP))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 7)
		{
			m_pConfigLabel[CONFIGINPUT]->SetText("PREV WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[X]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_PREV))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		else if (m_ConfigureStep == 8)
		{
			m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
			m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[C]");
			if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_WEAPON_CHANGE_NEXT))
			{
				m_ConfigureStep++;
				m_ScreenChange = true;
			}
		}
		// Prev actor
        else if (m_ConfigureStep == 9)
        {
            m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[Q]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_PREV))
            {
                m_ConfigureStep++;
                m_ScreenChange = true;
            }
        }
        // Next actor
        else if (m_ConfigureStep == 10)
        {
            // Hide skip button on this last step
            m_pConfigSkipButton->SetVisible(false);

            m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
            m_pConfigLabel[CONFIGRECOMMENDATION]->SetText("[E]");
            if (g_UInputMan.CaptureKeyMapping(m_ConfiguringPlayer, INPUT_NEXT))
            {
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
    else if (m_ConfiguringDevice >= DEVICE_GAMEPAD_1 && m_ConfiguringDevice <= DEVICE_GAMEPAD_4)
    {
        int whichJoy = m_ConfiguringDevice - DEVICE_GAMEPAD_1;
        AllegroBitmap *pDiagramBitmap = 0;

        // Choose which gamepad type - special first step
        if (m_ConfigureStep == 0)
        {
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
        else
        {
            if (m_ScreenChange)
            {
                m_pConfigLabel[CONFIGINSTRUCTION]->SetVisible(true);
                m_pConfigLabel[CONFIGINPUT]->SetVisible(true);
                m_pConfigLabel[CONFIGSTEPS]->SetVisible(true);
                m_pRecommendationBox->SetVisible(true);
                m_pConfigSkipButton->SetVisible(true);
                m_pConfigBackButton->SetVisible(true);
                m_BlinkTimer.Reset();
            }

            // D-pad
            if (m_ConfiguringGamepad == DPAD)
            {
                if (m_ScreenChange)
                {
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
                if (m_ConfigureStep == 1)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM UP");

                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM_UP))
                    {
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_UP);
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_L_UP);
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_UP);
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_UP);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move/Aim down
                else if (m_ConfigureStep == 2)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE or AIM DOWN");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM_DOWN))
                    {
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_DOWN);
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_L_DOWN);
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_DOWN);
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_DOWN);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move left
                else if (m_ConfigureStep == 3)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_LEFT))
                    {
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_LEFT);
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_LEFT);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move right
                else if (m_ConfigureStep == 4)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_RIGHT))
                    {
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_R_RIGHT);
//                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_RIGHT);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Sharp aim
                else if (m_ConfigureStep == 5)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("SHARP AIM");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_AIM))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Fire
                else if (m_ConfigureStep == 6)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_FIRE))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Jump
                else if (m_ConfigureStep == 7)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("JUMP");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_JUMP))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
// TODO: CROUCH???
                // Inventory
                else if (m_ConfigureStep == 8)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PIEMENU))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Next actor
                else if (m_ConfigureStep == 9)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_NEXT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Prev actor
                else if (m_ConfigureStep == 10)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PREV))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Start
                else if (m_ConfigureStep == 11)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_START))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Back
                else if (m_ConfigureStep == 12)
                {
                    // Hide the skip button on this last step
                    m_pConfigSkipButton->SetVisible(false);
                    m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_BACK))
                    {
                        // Done, go back to options screen
                        m_apScreenBox[CONFIGSCREEN]->SetVisible(false);
                        m_MenuScreen = OPTIONSSCREEN;
                        m_ScreenChange = true;
                    }
                }
            }
            // Dual analog OR XBox Controller
            else if (m_ConfiguringGamepad == DANALOG || m_ConfiguringGamepad == XBOX360)
            {
                if (m_ScreenChange)
                {
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
                if (m_ConfigureStep == 1)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE UP or JUMP");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_UP))
                    {
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_AIM_UP);
                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_JUMP);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move down
                else if (m_ConfigureStep == 2)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE DOWN or CROUCH");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_DOWN))
                    {
                        g_UInputMan.ClearMapping(m_ConfiguringPlayer, INPUT_AIM_DOWN);
                        g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_CROUCH);
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move left
                else if (m_ConfigureStep == 3)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE LEFT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_LEFT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Move right
                else if (m_ConfigureStep == 4)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("MOVE RIGHT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_L_RIGHT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Aim up
                else if (m_ConfigureStep == 5)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("AIM UP");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_UP))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Aim down
                else if (m_ConfigureStep == 6)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("AIM DOWN");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_DOWN))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Aim left
                else if (m_ConfigureStep == 7)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("AIM LEFT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_LEFT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Aim right
                else if (m_ConfigureStep == 8)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("AIM RIGHT");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_R_RIGHT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Fire
                else if (m_ConfigureStep == 9)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("FIRE / ACTIVATE");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_FIRE))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Inventory
                else if (m_ConfigureStep == 10)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("COMMAND MENU");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PIEMENU))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Next actor
                else if (m_ConfigureStep == 11)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("NEXT BODY");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_NEXT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Prev actor
                else if (m_ConfigureStep == 12)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS BODY");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_PREV))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }

                // Prev weapon
                else if (m_ConfigureStep == 13)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("PREVIOUS WEAPON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_PREV))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Next weapon
                else if (m_ConfigureStep == 14)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("NEXT WEAPON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_CHANGE_NEXT))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Pickup weapon
                else if (m_ConfigureStep == 15)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("PICKUP WEAPON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_PICKUP))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Reload weapon
                else if (m_ConfigureStep == 16)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("RELOAD WEAPON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_WEAPON_RELOAD))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Start
                else if (m_ConfigureStep == 17)
                {
                    m_pConfigLabel[CONFIGINPUT]->SetText("START BUTTON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_START))
                    {
                        m_ConfigureStep++;
                        m_ScreenChange = true;
                    }
                }
                // Back
                else if (m_ConfigureStep == 18)
                {
                    // Hide the skip button on this last step
                    m_pConfigSkipButton->SetVisible(false);
                    m_pConfigLabel[CONFIGINPUT]->SetText("BACK BUTTON");
                    if (g_UInputMan.CaptureJoystickMapping(m_ConfiguringPlayer, whichJoy, INPUT_BACK))
                    {
                        // If Xbox controller; if the A button has not been mapped to Activate/fire, then map it automatically
// These redundancies should apply to all custom analog setups, really
//                        if (m_ConfiguringGamepad == XBOX360)
                        {
                            // No button assigned to fire, so give it 'A' on the controller (in addition to any axis inputs)
                            if (g_UInputMan.GetButtonMapping(m_ConfiguringPlayer, INPUT_FIRE) == JOY_NONE)
                                g_UInputMan.SetButtonMapping(m_ConfiguringPlayer, INPUT_FIRE, JOY_1);
                            // No button assigned to pie menu, so give it 'B' on the controller (in addition to whatever axis it's assinged to)
                            if (g_UInputMan.GetButtonMapping(m_ConfiguringPlayer, INPUT_PIEMENU) == JOY_NONE)
                                g_UInputMan.SetButtonMapping(m_ConfiguringPlayer, INPUT_PIEMENU, JOY_2);
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
	
    if (m_ScreenChange)
        g_GUISound.ExitMenuSound()->Play();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeModString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes UI displayable string with mod info

std::string MainMenuGUI::MakeModString(ModRecord r)
{
	string s;

	if (r.Disabled)
		s = "- ";
	else
		s = "+ ";
	s = s + r.ModulePath + " - " + r.ModuleName;

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeScriptString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes UI-displayable string with script info

std::string MainMenuGUI::MakeScriptString(ScriptRecord r)
{
	string s;

	if (!r.Enabled)
		s = "- ";
	else 
		s = "+ ";
	s = s + r.PresetName;

	return s;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleMod
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns currently selected mod on and aff and changes UI elements accordingly.

void MainMenuGUI::ToggleMod()
{
	int index = m_pModManagerModsListBox->GetSelectedIndex();
	if (index > -1)
	{
		GUIListPanel::Item *selectedItem = m_pModManagerModsListBox->GetSelected();
		ModRecord r = m_KnownMods.at(selectedItem->m_ExtraIndex);

		r.Disabled = !r.Disabled;

		if (r.Disabled)
		{
			m_pModManagerToggleModButton->SetText("Enable");
			g_SettingsMan.DisableMod(r.ModulePath);
		}
		else
		{
			m_pModManagerToggleModButton->SetText("Disable");
			g_SettingsMan.EnableMod(r.ModulePath);
		}
		selectedItem->m_Name = MakeModString(r);
		m_KnownMods[selectedItem->m_ExtraIndex] = r;
		m_pModManagerModsListBox->SetSelectedIndex(index);
		m_pModManagerModsListBox->Invalidate();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToggleScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns currently selected script on and aff and changes UI elements accordingly.

void MainMenuGUI::ToggleScript()
{
	int index = m_pModManagerScriptsListBox->GetSelectedIndex();
	if (index > -1)
	{
		GUIListPanel::Item *selectedItem = m_pModManagerScriptsListBox->GetSelected();
		ScriptRecord r = m_KnownScripts.at(selectedItem->m_ExtraIndex);

		r.Enabled = !r.Enabled;

		if (r.Enabled)
		{
			m_pModManagerToggleScriptButton->SetText("Disable");
			g_SettingsMan.EnableScript(r.PresetName);
		}
		else
		{
			m_pModManagerToggleScriptButton->SetText("Enable");
			g_SettingsMan.DisableScript(r.PresetName);
		}
		selectedItem->m_Name = MakeScriptString(r);
		m_KnownScripts[selectedItem->m_ExtraIndex] = r;
		m_pModManagerScriptsListBox->SetSelectedIndex(index);
		m_pModManagerScriptsListBox->Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartActorEditor() { StartEditorActivity(new ActorEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartGibEditor() { StartEditorActivity(new GibEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartSceneEditor() { StartEditorActivity(new SceneEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartAreaEditor() { StartEditorActivity(new AreaEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartAssemblyEditor() { StartEditorActivity(new AssemblyEditor); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainMenuGUI::StartEditorActivity(EditorActivity *editorActivityToStart) {
    g_SceneMan.SetSceneToLoad("Editor Scene");
    editorActivityToStart->Create();
    editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
    g_ActivityMan.SetStartActivity(editorActivityToStart);
}
