//////////////////////////////////////////////////////////////////////////////////////////
// File:            MetagameGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the MetagameGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MetagameGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "AudioMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MetaMan.h"
#include "MetaSave.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIComboBox.h"
#include "GUI/GUICheckbox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"
#include "GUI/GUISlider.h"

#include "Controller.h"
#include "Entity.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "GABrainMatch.h"
#include "GABaseDefense.h"
#include "GATutorial.h"
#include "GAScripted.h"
#include "BaseEditor.h"
#include "Scene.h"
#include "SLTerrain.h"
#include "DataModule.h"
#include "Loadout.h"

using namespace RTE;

#define CHAMFERSIZE 40

#define SCANCOST 250
#define TRADESTARRENT 200
#define STARTGOLDMIN 2000
#define STARTGOLDMAX 8000
#define BRAINPOOLMAX 20
#define BRAINGOLDVALUE 4000//1250
#define BATTLEPAD 10
#define BRAINOVERLAP 2


const string MetagameGUI::c_ClassName = "MetagameGUI";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a neat animation over a site to show it changing team ownership.

void MetagameGUI::SiteTarget::Draw(BITMAP *drawBitmap) const
{
    if (!drawBitmap)
        return;

    // Draw the appropriate growing geometric figure around the location, growing
    if (m_Style == SiteTarget::CROSSHAIRSSHRINK)
    {
        float radius = LERP(0.0, 1.0, 200, 10, m_AnimProgress);
        float lineLen = LERP(0.0, 1.0, 60, 10, m_AnimProgress);
        float rotation = 0;//LERP(0.0, 1.0, -c_EighthPI, 0.0, m_AnimProgress);
        Vector inner;
        Vector outer;

        // Draw all the crosshair lines, being rotated
        for (int i = 0; i < 4; ++i)
        {
            inner.SetXY(radius, 0);
            outer.SetXY(radius + lineLen, 0);
            inner.RadRotate(rotation + (c_HalfPI * i));
            outer.RadRotate(rotation + (c_HalfPI * i));
            DrawGlowLine(drawBitmap, m_CenterPos + inner, m_CenterPos + outer, m_Color);
        }
    }
    else if (m_Style == SiteTarget::CROSSHAIRSGROW)
    {
        float radius = LERP(0.0, 1.0, 10, 200, m_AnimProgress);
        float lineLen = LERP(0.0, 1.0, 10, 60, m_AnimProgress);
        float rotation = 0;//LERP(0.0, 1.0, -c_EighthPI, 0.0, m_AnimProgress);
        Vector inner;
        Vector outer;

        // Draw all the crosshair lines, being rotated
        for (int i = 0; i < 4; ++i)
        {
            inner.SetXY(radius, 0);
            outer.SetXY(radius + lineLen, 0);
            inner.RadRotate(rotation + (c_HalfPI * i));
            outer.RadRotate(rotation + (c_HalfPI * i));
            DrawGlowLine(drawBitmap, m_CenterPos + inner, m_CenterPos + outer, m_Color);
        }
    }
    else if (m_Style == SiteTarget::CIRCLESHRINK)
    {
        float radius = LERP(0.0, 1.0, 24, 6, m_AnimProgress);
        int blendAmount = LERP(0.0, 1.0, 0, 255, m_AnimProgress);// + 15 * NormalRand();
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
        circle(drawBitmap, m_CenterPos.m_X, m_CenterPos.m_Y, radius, m_Color);
    }
    else if (m_Style == SiteTarget::CIRCLEGROW)
    {
        float radius = LERP(0.0, 1.0, 6, 24, m_AnimProgress);
        int blendAmount = LERP(0.0, 1.0, 255, 0, m_AnimProgress);// + 15 * NormalRand();
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
        circle(drawBitmap, m_CenterPos.m_X, m_CenterPos.m_Y, radius, m_Color);
    }
    else if (m_Style == SiteTarget::SQUARESHRINK)
    {
        float radius = LERP(0.0, 1.0, 24, 6, m_AnimProgress);
        int blendAmount = LERP(0.0, 1.0, 0, 255, m_AnimProgress);// + 15 * NormalRand();
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
        rect(drawBitmap, m_CenterPos.m_X - radius, m_CenterPos.m_Y - radius, m_CenterPos.m_X + radius, m_CenterPos.m_Y + radius, m_Color);
    }
    // Default
    else// if (m_Style == SiteTarget::SQUAREGROW)
    {
        float radius = LERP(0.0, 1.0, 6, 24, m_AnimProgress);
        int blendAmount = LERP(0.0, 1.0, 255, 0, m_AnimProgress);// + 15 * NormalRand();
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
        rect(drawBitmap, m_CenterPos.m_X - radius, m_CenterPos.m_Y - radius, m_CenterPos.m_X + radius, m_CenterPos.m_Y + radius, m_Color);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MetagameGUI, effectively
//                  resetting the members of this abstraction level only.

void MetagameGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_MenuEnabled = ENABLED;
    m_MenuScreen = NEWDIALOG;
    m_ScreenChange = true;
    m_SceneFocus = 0;
    m_FocusChange = 0;
    m_MenuSpeed = 0.3;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;

    m_pBannerRedTop = 0;
    m_pBannerRedBottom = 0;
    m_pBannerYellowTop = 0;
    m_pBannerYellowBottom = 0;

    m_AnimTimer1.Reset();
    m_AnimTimer2.Reset();
    m_AnimTimer3.Reset();

    m_AnimMode = 0;
    m_AnimModeChange = false;
    m_AnimModeDuration = 2000;
    m_AnimMetaPlayer = Players::NoPlayer;
    m_AnimDefenseTeam = Activity::NoTeam;
    m_AnimActivityChange = false;
    Scene *m_pAnimScene = 0;
    m_AnimRatio = 0;
    m_AnimProgress = 0;
    m_AnimTotalFunds = 0;
    m_AnimFundsMax = 0;
    m_AnimFundsMin = 0;
    m_AnimBuildCount = 0;
    m_AnimIncomeLine = 0;
    m_AnimIncomeLineChange = false;
    m_AnimActionLine = 0;
    m_AnimActionLineChange = false;
    m_AnimSegment = 0;
    m_AnimCountStart = 0;
    m_AnimCountCurrent = 0;
    m_AnimCountEnd = 0;
    m_LineConnected = false;

    m_IncomeSiteLines.clear();
    m_ActivePlayerIncomeLines = -1;
    m_ActionMeterDrawOverride = false;
    m_NewSiteIndicators.clear();
    m_SiteSwitchIndicators.clear();

    m_PlanetCenter.Reset();
    m_PlanetRadius = 240.0f;

    m_pGameMessageLabel = 0;

    m_pToolTipBox = 0;
    m_pToolTipText = 0;
    m_ToolTipTimer.Reset();
    m_pHoveredControl = 0;

    for (int iscreen = 0; iscreen < SCREENCOUNT; ++iscreen)
        m_apScreenBox[iscreen] = 0;
    for (int button = 0; button < METABUTTONCOUNT; ++button)
        m_apMetaButton[button] = 0;

    for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
    {
        m_apPlayerBox[metaPlayer] = 0;
        m_apPlayerTeamBox[metaPlayer] = 0;
        m_apPlayerTeamActionBox[metaPlayer] = 0;
        m_apPlayerBarLabel[metaPlayer] = 0;
        m_apPlayerBrainTravelLabel[metaPlayer] = 0;
        m_aBattleFunds[metaPlayer] = 0;
        m_aBattleAttacker[metaPlayer] = false;
        m_aAnimDestroyed[metaPlayer] = false;
        m_aBrainIconPos[metaPlayer].Reset();
        m_aQuadTakenBy[metaPlayer] = Players::NoPlayer;
        m_apBrainPoolLabel[metaPlayer] = 0;
        m_apFundsChangeLabel[metaPlayer] = 0;
        m_apBrainChangeLabel[metaPlayer] = 0;
        m_apFundsChangeTimer[metaPlayer].Reset();
        m_apBrainsChangeTimer[metaPlayer].Reset();

        m_apPlayerControlButton[metaPlayer] = 0;
        m_apPlayerTeamSelect[metaPlayer] = 0;
        m_apPlayerTechSelect[metaPlayer] = 0;
        m_apPlayerHandicap[metaPlayer] = 0;
        m_apPlayerNameBox[metaPlayer] = 0;
		m_apPlayerAISkillSlider[metaPlayer] = 0;
		m_apPlayerAISkillLabel[metaPlayer] = 0;

        m_aStationIncomeLineIndices[metaPlayer] = -1;
        m_aBrainSaleIncomeLineIndices[metaPlayer] = -1;
        m_ActionSiteLines[metaPlayer].clear();
    }

    m_PrevMousePos.Reset();

    m_pConfirmationBox = 0;
    m_pConfirmationLabel = 0;
    m_pConfirmationButton = 0;

    m_pPhaseBox = 0;
    m_pPhaseLabel = 0;
    m_PreTurn = false;
    m_BattleToResume = false;
    m_PostBattleReview = false;
    m_BattleCausedOwnershipChange = false;
    m_PreBattleTeamOwnership = Activity::NoTeam;

    m_pScenePlanetLabel = 0;
    m_pSceneInfoPopup = 0;
    m_pSceneCloseButton = 0;
    m_pSceneNameLabel = 0;
    m_pSceneOwnerTeam = 0;
    m_pSceneResidentsLabel = 0;
    m_pSceneInfoLabel = 0;
    m_pSceneBudgetLabel = 0;
    m_pSceneBudgetSlider = 0;
    m_pSceneBudgetBar = 0;
    m_pAutoDesignCheckbox = 0;
    m_pScanInfoLabel = 0;

    m_pDraggedBox = 0;
    m_EngageDrag = false;
    m_pHoveredScene = 0;
    m_pSelectedScene = 0;
    m_pPlayingScene = 0;

    m_pSizeLabel = 0;
    m_pSizeSlider = 0;
    m_pDifficultyLabel = 0;
    m_pDifficultySlider = 0;
    m_pGoldLabel = 0;
    m_pGoldSlider = 0;
    m_pLengthLabel = 0;
    m_pLengthSlider = 0;
    m_pErrorLabel = 0;

    m_NewSaveBox = 0;
    m_pSavesToOverwriteCombo = 0;
    m_pSavesToLoadCombo = 0;
    m_pSaveInfoLabel = 0;
    m_pLoadInfoLabel = 0;
    m_pSelectedGameToLoad = 0;

    m_ContinuePhase = false;
    m_ActivityRestarted = false;
    m_ActivityResumed = false;
    m_StartFunds = 1600;
    m_CPUPlayer = -1;
    m_StartDifficulty = Activity::MediumDifficulty;
    m_BackToMain = false;
    m_Quit = false;

	m_StationPosOnOrbit.Reset();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:		SetToStartNewGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets internal state of GUI to show 'Start new campaign' screen
// Arguments:       None.
// Return value:    None.
void MetagameGUI::SetToStartNewGame()
{
	g_MetaMan.SetSuspend(true);
	HideAllScreens();
	UpdatePlayerSetup();
	//UpdatePlayerBars();
	SwitchToScreen(MetagameGUI::NEWDIALOG);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetagameGUI object ready for use.

int MetagameGUI::Create()
{
    for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
    {
        m_ActionSiteLines[metaPlayer].clear();
        // The relevant onces will be enabled again in update
        m_apPlayerBox[metaPlayer]->SetVisible(false);
        // Clear out the team flag Icons; they will be re-set on next update
        m_apPlayerTeamBox[metaPlayer]->SetDrawImage(0);
        m_apPlayerTeamActionBox[metaPlayer]->SetDrawImage(0);
        m_apBrainPoolLabel[metaPlayer]->SetVisible(false);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetagameGUI object ready for use.

int MetagameGUI::Create(Controller *pController)
{
    RTEAssert(pController, "No controller sent to MetagameGUI on creation!");
    m_pController = pController;

    char str[256];

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(-1, true);
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
	if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
	}
    m_pGUIController->Load("Base.rte/GUIs/MetagameGUI.ini");

    // Make sure we have convenient points to the containing GUI colleciton boxes that we will manipulate the positions of
    GUICollectionBox *pRootBox = m_apScreenBox[ROOTBOX] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
    // Make the root box fill the screen
//    pRootBox->SetPositionAbs((g_FrameMan.GetResX() - pRootBox->GetWidth()) / 2, 0);// (g_FrameMan.GetResY() - pRootBox->GetHeight()) / 2);
    pRootBox->SetDrawBackground(false);
    pRootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    m_pBannerRedTop = new GUIBanner();
    m_pBannerRedBottom = new GUIBanner();
    m_pBannerYellowTop = new GUIBanner();
    m_pBannerYellowBottom = new GUIBanner();
    m_pBannerRedTop->Create("Base.rte/GUIs/Fonts/BannerFontRedReg.png", "Base.rte/GUIs/Fonts/BannerFontRedBlur.png", 32);
    m_pBannerRedBottom->Create("Base.rte/GUIs/Fonts/BannerFontRedReg.png", "Base.rte/GUIs/Fonts/BannerFontRedBlur.png", 32);
    m_pBannerYellowTop->Create("Base.rte/GUIs/Fonts/BannerFontYellowReg.png", "Base.rte/GUIs/Fonts/BannerFontYellowBlur.png", 32);
    m_pBannerYellowBottom->Create("Base.rte/GUIs/Fonts/BannerFontYellowReg.png", "Base.rte/GUIs/Fonts/BannerFontYellowBlur.png", 32);

    // General game message label
    m_pGameMessageLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("GameMessageLabel"));
    m_pGameMessageLabel->CenterInParent(true, false);
    m_pGameMessageLabel->SetPositionAbs(m_pGameMessageLabel->GetXPos(), 0);
    m_pGameMessageLabel->SetVisible(false);

    // ToolTip box
    m_pToolTipBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ToolTipBox"));
    m_pToolTipText = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ToolTipLabel"));
    m_pToolTipBox->SetDrawType(GUICollectionBox::Panel);
    m_pToolTipBox->SetDrawBackground(true);
    // Never enable the popup, because it steals focus and cuases other windows to think teh cursor left them
    m_pToolTipBox->SetEnabled(false);
    m_pToolTipBox->SetVisible(false);
    // Set the font
    m_pToolTipText->SetFont(m_pGUIController->GetSkin()->GetFont("FontSmall.png"));

    // Make sure we have convenient points to the containing GUI colleciton boxes that we will manipulate the positions of
    m_apScreenBox[NEWDIALOG] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("NewGameDialog"));
    m_apScreenBox[LOADDIALOG] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("LoadDialog"));
    m_apScreenBox[SAVEDIALOG] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SaveDialog"));
    m_apScreenBox[MENUDIALOG] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("GameMenu"));
    m_apScreenBox[STATSDIALOG] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("GameStatsBox"));
    m_apScreenBox[SCENEINFOBOX] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SceneInfoBox"));

    // Center most of the main boxes in the root box
    m_apScreenBox[NEWDIALOG]->CenterInParent(true, true);
    m_apScreenBox[LOADDIALOG]->CenterInParent(true, true);
    m_apScreenBox[SAVEDIALOG]->CenterInParent(true, true);
    m_apScreenBox[MENUDIALOG]->CenterInParent(true, true);
    m_apScreenBox[STATSDIALOG]->CenterInParent(true, true);

    // Buttons which will be altered
    m_apMetaButton[CONFIRM] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConfirmButton"));
    m_apMetaButton[P1CONTROL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P1ControlButton"));
    m_apMetaButton[P2CONTROL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P2ControlButton"));
    m_apMetaButton[P3CONTROL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P3ControlButton"));
    m_apMetaButton[P4CONTROL] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P4ControlButton"));
    m_apMetaButton[STARTNEW] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("StartButton"));
    m_apMetaButton[LOADNOW] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("LoadButton"));
    m_apMetaButton[SAVENOW] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SaveButton"));
    m_apMetaButton[CONTINUE] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ContinueButton"));
    m_apMetaButton[SCENEACTION] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SceneActionButton"));
    m_apMetaButton[DESIGNBASE] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("DesignBaseButton"));
    m_apMetaButton[SCANNOW] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ScanNowButton"));
    m_apMetaButton[SCANLATER] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ScanLaterButton"));

    // Confirmation box
    m_pConfirmationBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConfirmDialog"));
    m_pConfirmationLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ConfirmLabel"));
    m_pConfirmationButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConfirmButton"));
    m_pConfirmationBox->CenterInParent(true, true);
    m_pConfirmationBox->SetVisible(false);

    // Floating player bars
    m_apPlayerBox[Players::PlayerOne] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P1Bar"));
    m_apPlayerBox[Players::PlayerTwo] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P2Bar"));
    m_apPlayerBox[Players::PlayerThree] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P3Bar"));
    m_apPlayerBox[Players::PlayerFour] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P4Bar"));
    m_apPlayerTeamBox[Players::PlayerOne] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P1Team"));
    m_apPlayerTeamBox[Players::PlayerTwo] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P2Team"));
    m_apPlayerTeamBox[Players::PlayerThree] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P3Team"));
    m_apPlayerTeamBox[Players::PlayerFour] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P4Team"));
    m_apPlayerBarLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1FundsLabel"));
    m_apPlayerBarLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2FundsLabel"));
    m_apPlayerBarLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3FundsLabel"));
    m_apPlayerBarLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4FundsLabel"));
    m_apBrainPoolLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1BrainLabel"));
    m_apBrainPoolLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2BrainLabel"));
    m_apBrainPoolLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3BrainLabel"));
    m_apBrainPoolLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4BrainLabel"));
    m_apFundsChangeLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1FundsChangeLabel"));
    m_apFundsChangeLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2FundsChangeLabel"));
    m_apFundsChangeLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3FundsChangeLabel"));
    m_apFundsChangeLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4FundsChangeLabel"));
    m_apBrainChangeLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1BrainChangeLabel"));
    m_apBrainChangeLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2BrainChangeLabel"));
    m_apBrainChangeLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3BrainChangeLabel"));
    m_apBrainChangeLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4BrainChangeLabel"));

    // Battle site display
    m_apPlayerTeamActionBox[Players::PlayerOne] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P1TeamAction"));
    m_apPlayerTeamActionBox[Players::PlayerTwo] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P2TeamAction"));
    m_apPlayerTeamActionBox[Players::PlayerThree] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P3TeamAction"));
    m_apPlayerTeamActionBox[Players::PlayerFour] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("P4TeamAction"));
    m_apPlayerBrainTravelLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1BrainTravelLabel"));
    m_apPlayerBrainTravelLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2BrainTravelLabel"));
    m_apPlayerBrainTravelLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3BrainTravelLabel"));
    m_apPlayerBrainTravelLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4BrainTravelLabel"));

    for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
    {
        m_apBrainPoolLabel[metaPlayer]->SetVisible(false);
        m_apBrainPoolLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
        // Tuck these away so they don't interfere with dragging etc
        m_apFundsChangeLabel[metaPlayer]->SetVisible(false);
        m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
        m_apBrainChangeLabel[metaPlayer]->SetVisible(false);
        m_apBrainChangeLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);

        // Hide the battle display initially
        m_apPlayerTeamActionBox[metaPlayer]->SetVisible(false);
        m_apPlayerTeamActionBox[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
        m_apPlayerBrainTravelLabel[metaPlayer]->SetVisible(false);
        m_apPlayerBrainTravelLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
    }

    // Phase info box
    m_pPhaseBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PhaseBox"));
    m_pPhaseLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("PhaseLabel"));
    // Just get the menu button temporarily so we can set the custom menu icon
    if (GUIButton *pMenuButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("OpenMenuButton")))
    {
        std::snprintf(str, sizeof(str), "%c", -22);
        pMenuButton->SetText(string(str));
    }

    // Planet mouseover scene label
    m_pScenePlanetLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ScenePlanetLabel"));
    m_pScenePlanetLabel->SetVisible(false);

    // Scene Info Box
    m_pSceneInfoPopup = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SceneInfoBox"));
    m_pSceneCloseButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SceneCloseButton"));
    m_pSceneNameLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneNameLabel"));
    m_pSceneOwnerTeam = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SceneOwnerTeam"));
    m_pSceneResidentsLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneResidentsLabel"));
    m_pSceneInfoLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneInfoLabel"));
    m_pSceneInfoLabel->SetFont(m_pGUIController->GetSkin()->GetFont("FontSmall.png"));
    m_pSceneInfoPopup->SetVisible(false);
    m_pSceneBudgetLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneBudgetLabel"));
    m_pSceneBudgetSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SceneBudgetSlider"));
    m_pSceneBudgetBar = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SceneBudgetBar"));
    m_pSceneBudgetBar->SetDrawColor(makecol(55, 5, 10));
    m_pAutoDesignCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("AutoDesignCheckbox"));
    m_pScanInfoLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ScanInfoLabel"));

    // Set initial focus, category list, and label settings
    m_ScreenChange = true;
    m_FocusChange = 1;
//    CategoryChange();

    // New Game Dialog
    m_pSizeLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SizeLabel"));
    m_pSizeSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("SizeSlider"));
    m_pDifficultyLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("DifficultyLabel"));
    m_pDifficultySlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("DifficultySlider"));
    m_pGoldLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("GoldLabel"));
    m_pGoldSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("GoldSlider"));
    m_pLengthLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LengthLabel"));
    m_pLengthSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("LengthSlider"));
    m_pErrorLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("StartErrorLabel"));
    m_apPlayerControlButton[Players::PlayerOne] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P1ControlButton"));
    m_apPlayerControlButton[Players::PlayerTwo] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P2ControlButton"));
    m_apPlayerControlButton[Players::PlayerThree] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P3ControlButton"));
    m_apPlayerControlButton[Players::PlayerFour] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("P4ControlButton"));
    m_apPlayerTeamSelect[Players::PlayerOne] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P1TeamCombo"));
    m_apPlayerTeamSelect[Players::PlayerTwo] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P2TeamCombo"));
    m_apPlayerTeamSelect[Players::PlayerThree] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P3TeamCombo"));
    m_apPlayerTeamSelect[Players::PlayerFour] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P4TeamCombo"));
    m_apPlayerTechSelect[Players::PlayerOne] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P1TechCombo"));
    m_apPlayerTechSelect[Players::PlayerTwo] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P2TechCombo"));
    m_apPlayerTechSelect[Players::PlayerThree] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P3TechCombo"));
    m_apPlayerTechSelect[Players::PlayerFour] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P4TechCombo"));
    m_apPlayerHandicap[Players::PlayerOne] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P1HCCombo"));
    m_apPlayerHandicap[Players::PlayerTwo] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P2HCCombo"));
    m_apPlayerHandicap[Players::PlayerThree] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P3HCCombo"));
    m_apPlayerHandicap[Players::PlayerFour] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("P4HCCombo"));
    m_apPlayerNameBox[Players::PlayerOne] = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("P1NameText"));
    m_apPlayerNameBox[Players::PlayerTwo] = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("P2NameText"));
    m_apPlayerNameBox[Players::PlayerThree] = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("P3NameText"));
    m_apPlayerNameBox[Players::PlayerFour] = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("P4NameText"));
    m_apPlayerAISkillSlider[Players::PlayerOne] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("P1AISkillSlider"));
    m_apPlayerAISkillSlider[Players::PlayerTwo] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("P2AISkillSlider"));
    m_apPlayerAISkillSlider[Players::PlayerThree] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("P3AISkillSlider"));
    m_apPlayerAISkillSlider[Players::PlayerFour] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("P4AISkillSlider"));
	m_apPlayerAISkillLabel[Players::PlayerOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P1AISkillLabel"));
	m_apPlayerAISkillLabel[Players::PlayerTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P2AISkillLabel"));
	m_apPlayerAISkillLabel[Players::PlayerThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P3AISkillLabel"));
	m_apPlayerAISkillLabel[Players::PlayerFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("P4AISkillLabel"));

	m_apPlayerControlButton[Players::PlayerOne]->SetText("Human");
//    m_apPlayerControlButton[Players::PlayerTwo]->SetText("Human");
    m_apPlayerControlButton[Players::PlayerTwo]->SetText("A.I.");
    m_apPlayerNameBox[Players::PlayerOne]->SetText("Player 1");
    m_apPlayerNameBox[Players::PlayerTwo]->SetText("Player 2");
    m_apPlayerNameBox[Players::PlayerThree]->SetText("Player 3");
    m_apPlayerNameBox[Players::PlayerFour]->SetText("Player 4");

    // Add the handicap options to the dropdowns
    // Prepare the brain icon
    std::snprintf(str, sizeof(str), "%c", -48);
    for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
    {
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " +5", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " +3", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " +1", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " +-", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " -1", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " -3", "", 0, 0, -1);
        m_apPlayerHandicap[metaPlayer]->GetListPanel()->AddItem(string(str) + " -5", "", 0, 0, -1);

        // Preselect the 0 handicap
        m_apPlayerHandicap[metaPlayer]->SetSelectedIndex(3);
        // Make the lists be scrolled to the top when they are initially dropped
//        m_apPlayerHandicap[metaPlayer]->GetListPanel()->ScrollToTop();
    }
    
    // Save and Load Dialogs
    m_NewSaveBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NewSaveText"));
    m_pSavesToOverwriteCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("OverwriteList"));
    m_pSavesToLoadCombo = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("LoadList"));
    m_pSaveInfoLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SavedGameStats"));
    m_pLoadInfoLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("LoadStats"));

	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		m_apPlayerTeamSelect[player]->ClearList();
		m_apPlayerTeamSelect[player]->SetDropHeight(70);
		m_apPlayerTechSelect[player]->SetDropHeight(70);
		m_apPlayerHandicap[player]->SetDropHeight(70);
		m_apPlayerTeamSelect[player]->GetListPanel()->SetAlternateDrawMode(true);
		m_apPlayerTeamSelect[player]->SetDropDownStyle(GUIComboBox::DropDownList);
	}

    // Special height for the last one so it doesn't fly out of the dialog box
    m_apPlayerTeamSelect[Players::PlayerFour]->SetDropHeight(40);
    m_apPlayerTechSelect[Players::PlayerFour]->SetDropHeight(40);
    m_apPlayerHandicap[Players::PlayerFour]->SetDropHeight(40);

    // Put the new game dialog in order
    UpdatePlayerSetup();
    UpdateGameSizeLabels();

	UpdateAISkillSliders(Players::PlayerOne);
	UpdateAISkillSliders(Players::PlayerTwo);
	UpdateAISkillSliders(Players::PlayerThree);
	UpdateAISkillSliders(Players::PlayerFour);

	MoveLocationsIntoTheScreen();

    // Hide all screens, the appropriate screen will reappear on next update
    HideAllScreens();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MoveLocationsIntoTheScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Moves any locations closer to the center of the planet if they were left out 
void MetagameGUI::MoveLocationsIntoTheScreen()
{
	//Clear offsets
    for (vector<Scene *>::iterator pItr = g_MetaMan.m_Scenes.begin(); pItr != g_MetaMan.m_Scenes.end(); ++pItr)
		(*pItr)->SetLocationOffset(Vector(0,0));

	// We need to calculate planet center manually because m_PlanetCenter reflects coords of moving planet
	// which is outside the screen when this is called first time
	Vector planetCenter = Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
	// Correct planet pos a bit when it's location is known
	if (!m_PlanetCenter.IsZero())
		planetCenter = m_PlanetCenter;

	//Move out-of-screen scenes closer to the middle of the planet if we have planet info
	for (vector<Scene *>::iterator pItr = g_MetaMan.m_Scenes.begin(); pItr != g_MetaMan.m_Scenes.end(); ++pItr)
	{
		float y = planetCenter.GetY() + (*pItr)->GetLocation().GetY();

		// Do not touch scenes outside the planet, they might be hidden intentionally
		if (abs((*pItr)->GetLocation().GetY()) < m_PlanetRadius + 100 && abs((*pItr)->GetLocation().GetX()) < m_PlanetRadius + 100)
		{
			if (y < 10)
				(*pItr)->SetLocationOffset(Vector(0, -y + 14));

			if (y > g_FrameMan.GetResY() - 10)
				(*pItr)->SetLocationOffset(Vector(0, -(y - g_FrameMan.GetResY() + 14)));
		}
	}

	// Add offsets to reveal overlapping scenes if any
    for (vector<Scene *>::iterator pItr = g_MetaMan.m_Scenes.begin(); pItr != g_MetaMan.m_Scenes.end(); ++pItr)
    {
		bool isOverlapped = false;

		do 
		{
			isOverlapped = false;

			// Find overlapping scene dot
			for (vector<Scene *>::iterator pItr2 = g_MetaMan.m_Scenes.begin(); pItr2 != g_MetaMan.m_Scenes.end(); ++pItr2)
			{
				if ((*pItr) != (*pItr2))
				{
					Vector pos1 = (*pItr)->GetLocation() + (*pItr)->GetLocationOffset();
					Vector pos2 = (*pItr2)->GetLocation() + (*pItr2)->GetLocationOffset();

					if ((pos1 - pos2).GetMagnitude() < 8)
					{
						isOverlapped = true;
						break;
					}
				}
			}

			// Move the dot closer to the planet center
			Vector offsetIncrement;
			if ((*pItr)->GetLocation().GetY() > 0)
				offsetIncrement = Vector(0, -8);
			else
				offsetIncrement = Vector(0, 8);

			if (isOverlapped)
				(*pItr)->SetLocationOffset((*pItr)->GetLocationOffset() + offsetIncrement);
		}
		while (isOverlapped);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.

int MetagameGUI::ReadProperty(const std::string_view &propName, Reader &reader)
{
    Vector tempPos;

    if (propName == "P1BoxPos")
    {
        reader >> tempPos;
        m_apPlayerBox[Players::PlayerOne]->SetPositionAbs(tempPos.GetFloorIntX(), tempPos.GetFloorIntY());
    }
    else if (propName == "P2BoxPos")
    {
        reader >> tempPos;
        m_apPlayerBox[Players::PlayerTwo]->SetPositionAbs(tempPos.GetFloorIntX(), tempPos.GetFloorIntY());
    }
    else if (propName == "P3BoxPos")
    {
        reader >> tempPos;
        m_apPlayerBox[Players::PlayerThree]->SetPositionAbs(tempPos.GetFloorIntX(), tempPos.GetFloorIntY());
    }
    else if (propName == "P4BoxPos")
    {
        reader >> tempPos;
        m_apPlayerBox[Players::PlayerFour]->SetPositionAbs(tempPos.GetFloorIntX(), tempPos.GetFloorIntY());
    }
    else if (propName == "PhaseBoxPos")
    {
        reader >> tempPos;
        m_pPhaseBox->SetPositionAbs(tempPos.GetFloorIntX(), tempPos.GetFloorIntY());
    }
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MetagameGUI to an output stream for
//                  later recreation with Create(Reader &reader);

int MetagameGUI::Save(Writer &writer) const {
	Serializable::Save(writer);

	writer.NewPropertyWithValue("P1BoxPos", Vector(m_apPlayerBox[Players::PlayerOne]->GetXPos(), m_apPlayerBox[Players::PlayerOne]->GetYPos()));
	writer.NewPropertyWithValue("P2BoxPos", Vector(m_apPlayerBox[Players::PlayerTwo]->GetXPos(), m_apPlayerBox[Players::PlayerTwo]->GetYPos()));
	writer.NewPropertyWithValue("P3BoxPos", Vector(m_apPlayerBox[Players::PlayerThree]->GetXPos(), m_apPlayerBox[Players::PlayerThree]->GetYPos()));
	writer.NewPropertyWithValue("P4BoxPos", Vector(m_apPlayerBox[Players::PlayerFour]->GetXPos(), m_apPlayerBox[Players::PlayerFour]->GetYPos()));

	writer.NewPropertyWithValue("PhaseBoxPos", Vector(m_pPhaseBox->GetXPos(), m_pPhaseBox->GetYPos()));

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MetagameGUI object.

void MetagameGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    delete m_pBannerRedTop;
    delete m_pBannerRedBottom;
    delete m_pBannerYellowTop;
    delete m_pBannerYellowBottom;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGUIControlManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the GUIControlManager owned and used by this.

GUIControlManager * MetagameGUI::GetGUIControlManager()
{
    return m_pGUIController;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void MetagameGUI::SetEnabled(bool enable)
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

	// Populate the tech comboboxes with the available tech modules
	for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
		m_apPlayerTechSelect[team]->GetListPanel()->AddItem("-Random-", "", nullptr, nullptr, -1);
		m_apPlayerTechSelect[team]->SetSelectedIndex(0);
	}
	std::string techString = " Tech";
	for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID) {
		if (const DataModule *dataModule = g_PresetMan.GetDataModule(moduleID)) {
			std::string techName = dataModule->GetFriendlyName();
			size_t techPos = techName.find(techString);
			if (techPos != string::npos) {
				techName.replace(techPos, techString.length(), "");
				for (int team = Activity::Teams::TeamOne; team < Activity::Teams::MaxTeamCount; ++team) {
					m_apPlayerTechSelect[team]->GetListPanel()->AddItem(techName, "", nullptr, nullptr, moduleID);
					m_apPlayerTechSelect[team]->GetListPanel()->ScrollToTop();
				}
			}
		}
	}

	list<Entity *> flagList;
	g_PresetMan.GetAllOfGroup(flagList, "Flags", "Icon");
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		for (list<Entity *>::iterator itr = flagList.begin(); itr != flagList.end(); ++itr) {
			if (const Icon *pIcon = dynamic_cast<Icon *>(*itr)) { m_apPlayerTeamSelect[player]->AddItem("", "", new AllegroBitmap(pIcon->GetBitmaps32()[0]), pIcon); }
		}
		m_apPlayerTeamSelect[player]->SetSelectedIndex(player);
	}

    m_ScreenChange = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tries to select a specifically named scene on the metagame field.

void MetagameGUI::SelectScene(Scene *pScene)
{
    m_pSelectedScene = pScene;

    // Update the budget slider
    if (m_pSelectedScene)
    {
        // If during a player's round phase, show and set the budget slider to reflect the newly selected scene's setting
        if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
        {
            int metaPlayer = g_MetaMan.m_GameState - MetaMan::PLAYER1TURN;

            // If owned by this player's team, make the budget slider represent the currently set setting of this Scene
            if (m_pSelectedScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer))
            {
                m_pSceneBudgetSlider->SetValue(std::floor((m_pSelectedScene->GetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer()) / g_MetaMan.m_Players[metaPlayer].GetFunds()) * 100));
            }
            // Owned by enemy player, so show the attack budget set up for this scene
            else if (g_MetaMan.IsActiveTeam(m_pSelectedScene->GetTeamOwnership()))
            {
                if (m_pSelectedScene->GetPresetName() == g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName())
                    m_pSceneBudgetSlider->SetValue(std::floor((g_MetaMan.m_Players[metaPlayer].GetOffensiveBudget() / g_MetaMan.m_Players[metaPlayer].GetFunds()) * 100));
                // Not the current target, so set slider to 0. It will set the new budget as 
                else
                    m_pSceneBudgetSlider->SetValue(0);
            }
            // Unowned site, so set up expedition budget (same so far)
            else
            {
                if (m_pSelectedScene->GetPresetName() == g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName())
                    m_pSceneBudgetSlider->SetValue(std::floor((g_MetaMan.m_Players[metaPlayer].GetOffensiveBudget() / g_MetaMan.m_Players[metaPlayer].GetFunds()) * 100));
                // Not the current target, so set slider to 0. It will set the new budget as 
                else
                    m_pSceneBudgetSlider->SetValue(0);
            }
        }

        UpdateScenesBox(true);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tries to select a specifically named scene on the metagame field.

bool MetagameGUI::SelectScene(std::string sceneName)
{
    for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        // Only allow selection if the Scene is revealed yet!
        if ((*sItr)->GetPresetName() == sceneName && (*sItr)->IsRevealed())
        {
            SelectScene(*sItr);
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SwitchToScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Switches to showing a specific menu screen/mode.

void MetagameGUI::SwitchToScreen(int newScreen)
{
    RTEAssert(newScreen >= ROOTBOX && newScreen < SCREENCOUNT, "Tried to switch to an out of bounds screen!");

    // Hide all previously shown screens
    HideAllScreens();

    m_MenuScreen = newScreen;
    m_ScreenChange = true;

    // Show the selected screen!
    m_apScreenBox[m_MenuScreen]->SetVisible(true);

    // Suspend game depending on which screen it is
    g_MetaMan.SetSuspend(m_MenuScreen != ROOTBOX && m_MenuScreen != STATSDIALOG && m_MenuScreen != SCENEINFOBOX);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes a round number into a nice friendly text string. "ONE" for 1 etc

string MetagameGUI::GetRoundName(int roundNumber)
{
    if (roundNumber < 12)
    {
        if (roundNumber == 0)
            return "ONE";
        else if (roundNumber == 1)
            return "TWO";
        else if (roundNumber == 2)
            return "THREE";
        else if (roundNumber == 3)
            return "FOUR";
        else if (roundNumber == 4)
            return "FIVE";
        else if (roundNumber == 5)
            return "SIX";
        else if (roundNumber == 6)
            return "SEVEN";
        else if (roundNumber == 7)
            return "EIGHT";
        else if (roundNumber == 8)
            return "NINE";
        else if (roundNumber == 9)
            return "TEN";
        else if (roundNumber == 10)
            return "ELEVEN";
        else if (roundNumber == 11)
            return "TWELVE";
    }
    char numStr[8];
    std::snprintf(numStr, sizeof(numStr), "%d", roundNumber + 1);
    return string(numStr);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartNewGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attempts to start a new Metagame using the settings set in the
//                  New Game dialog box.

bool MetagameGUI::StartNewGame()
{
    // Prepare the UI for the game intro/start
    UpdatePlayerSetup();
    HideAllScreens();
    m_IncomeSiteLines.clear();
    m_NewSiteIndicators.clear();
    m_SiteSwitchIndicators.clear();

    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        g_MetaMan.m_TeamIcons[team].Reset();

    // Create the MetaPlayer:s based on the settings in the dialog box
    char str[256];
    g_MetaMan.m_Players.clear();
    g_MetaMan.m_TeamCount = 0;
	g_MetaMan.m_Difficulty = m_pDifficultySlider->GetValue();
	m_StartDifficulty = m_pDifficultySlider->GetValue();
    const Icon *pTeamIcon = 0;

	//Clear metaman's AI skill to defaults
	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++)
		g_MetaMan.m_TeamAISkill[team] = Activity::DefaultSkill;

    // Starting gold amount is common to all
    int startGold = STARTGOLDMIN + ((STARTGOLDMAX - STARTGOLDMIN) * (float)m_pGoldSlider->GetValue() / 100.0);

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        m_ActionSiteLines[player].clear();
        // clear the flag icons on the floating player bars; they will be set on next update in UpdatePlayerBars()
        m_apPlayerTeamBox[player]->SetDrawType(GUICollectionBox::Image);
        m_apPlayerTeamBox[player]->SetDrawImage(0);
        // Same for the players' team flags that appear around the battle sites
        m_apPlayerTeamActionBox[player]->SetDrawType(GUICollectionBox::Image);
        m_apPlayerTeamActionBox[player]->SetDrawImage(0);

        // Found an active player
        if (m_apPlayerControlButton[player]->GetText() != "None")
        {
            // Disallow empty player name strings
            if (m_apPlayerNameBox[player]->GetText() == "")
            {
                std::snprintf(str, sizeof(str), "Player %d", player);
                m_apPlayerNameBox[player]->SetText(str);
            }

            // Set up the new player and add it
            MetaPlayer newPlayer;
            newPlayer.SetName(m_apPlayerNameBox[player]->GetText());
            // Set the in-game control mapping of this metagame player
            newPlayer.m_InGamePlayer = player;
            // Whether this is a human or AI player
            newPlayer.SetHuman(m_apPlayerControlButton[player]->GetText() == "Human");

			// Set native cost multypliiers according to difficulty
			if (!newPlayer.IsHuman())
			{
				if (g_MetaMan.m_Difficulty < Activity::CakeDifficulty)
					newPlayer.SetNativeCostMultiplier(1.2);
				else if (g_MetaMan.m_Difficulty < Activity::EasyDifficulty)
					newPlayer.SetNativeCostMultiplier(1.1);
				else if (g_MetaMan.m_Difficulty < Activity::MediumDifficulty)
					newPlayer.SetNativeCostMultiplier(1.0);
				else if (g_MetaMan.m_Difficulty < Activity::HardDifficulty)
					newPlayer.SetNativeCostMultiplier(0.80);
				else if (g_MetaMan.m_Difficulty < Activity::NutsDifficulty)
					newPlayer.SetNativeCostMultiplier(0.60);
				else
					newPlayer.SetNativeCostMultiplier(0.40);
			}

// TODO: Add the control scheme icons to the newgame dialog for clarity

            // Get the chosen team icon
            if (m_apPlayerTeamSelect[player]->GetSelectedItem())
                pTeamIcon = dynamic_cast<const Icon *>(m_apPlayerTeamSelect[player]->GetSelectedItem()->m_pEntity);
            // Just get the first one if nothing is selected
            else if (m_apPlayerTeamSelect[player]->GetCount() > 0)
                pTeamIcon = dynamic_cast<const Icon *>(m_apPlayerTeamSelect[player]->GetItem(0)->m_pEntity);

            // See if the player is designated to a new team or one that has already been created
            bool newTeam = true;
            for (int team = Activity::TeamOne; team < g_MetaMan.m_TeamCount; ++team)
            {
                // Join existing team!
                if (pTeamIcon->GetPresetName() == g_MetaMan.m_TeamIcons[team].GetPresetName())
                {
                    newPlayer.SetTeam(team);
                    newTeam = false;
                    break;
                }
            }

            // If we didn't find that the team we were designated already exists, then create it
            if (newTeam)
            {
                // Set the team of the new player
                newPlayer.SetTeam(g_MetaMan.m_TeamCount);
				// Set AI Skill level
				g_MetaMan.m_TeamAISkill[g_MetaMan.m_TeamCount] = m_apPlayerAISkillSlider[player]->GetValue();
                // Set the new team icon
                g_MetaMan.m_TeamIcons[g_MetaMan.m_TeamCount] = *pTeamIcon;
                // Increase the team count
                g_MetaMan.m_TeamCount++;
            }

            // Get the tech selection and apply it to the metaplayer
            GUIListPanel::Item *pTechItem = m_apPlayerTechSelect[player]->GetSelectedItem();
            if (pTechItem)
            {
                // If the "random" selection, choose one from the list of loaded techs
                if (m_apPlayerTechSelect[player]->GetSelectedIndex() <= 0)//pTechItem->m_ExtraIndex < 0)
                {
                    int selection = RandomNum<int>(1, m_apPlayerTechSelect[player]->GetListPanel()->GetItemList()->size() - 1);
					
					// Don't let the game to chose the same faction twice
					bool ok = false;
					while (!ok)
					{
						ok = true;
						selection = RandomNum<int>(1, m_apPlayerTechSelect[player]->GetListPanel()->GetItemList()->size() - 1);

						for (int p = 0; p < player; p++)
							if (selection == m_apPlayerTechSelect[p]->GetSelectedIndex())
								ok = false;
					}

                    m_apPlayerTechSelect[player]->SetSelectedIndex(selection);
                    pTechItem = m_apPlayerTechSelect[player]->GetSelectedItem();
                }

                // Now set the selected tech's module index as what the metaplayer is going to use
                if (pTechItem)
                    newPlayer.m_NativeTechModule = pTechItem->m_ExtraIndex;
            }
            
            // Set the starting brains for this player
            // Start with the baseline setting
            newPlayer.m_BrainPool = m_pLengthSlider->GetValue();
            // Baseline can never be 0
            newPlayer.m_BrainPool = MAX(newPlayer.m_BrainPool, 1);
            // Apply the handicap!
            if (m_apPlayerHandicap[player]->GetSelectedIndex() == 0)
                newPlayer.m_BrainPool += 5;
            else if (m_apPlayerHandicap[player]->GetSelectedIndex() == 1)
                newPlayer.m_BrainPool += 3;
            else if (m_apPlayerHandicap[player]->GetSelectedIndex() == 2)
                newPlayer.m_BrainPool += 1;
            else if (m_apPlayerHandicap[player]->GetSelectedIndex() == 4)
                newPlayer.m_BrainPool -= 1;
            else if (m_apPlayerHandicap[player]->GetSelectedIndex() == 5)
                newPlayer.m_BrainPool -= 3;
            else if (m_apPlayerHandicap[player]->GetSelectedIndex() == 6)
                newPlayer.m_BrainPool -= 5;
            // Give at least ONE brain!
            newPlayer.m_BrainPool = MAX(newPlayer.m_BrainPool, 1);

            // Starting gold amount; common to all
            newPlayer.m_Funds = startGold;

            g_MetaMan.m_Players.push_back(newPlayer);

            m_apPlayerTeamSelect[player]->SetVisible(false);
            m_apPlayerTechSelect[player]->SetVisible(false);
            m_apPlayerNameBox[player]->SetVisible(false);
			m_apPlayerAISkillSlider[player]->SetVisible(false);
			m_apPlayerAISkillLabel[player]->SetVisible(false);
            m_aBattleFunds[player] = 0;
            m_aBattleAttacker[player] = false;
            m_aAnimDestroyed[player] = false;
            m_aBrainIconPos[player].Reset();
            continue;
        }
    }

    // Arrange all the floating UI elements neatly, depending on number of playing players
    m_apPlayerBox[Players::PlayerOne]->SetPositionRel(20, 30);
    m_apPlayerBox[Players::PlayerTwo]->SetPositionRel(20, m_apScreenBox[ROOTBOX]->GetHeight() - m_apPlayerBox[Players::PlayerTwo]->GetHeight() - 30);
    m_apPlayerBox[Players::PlayerThree]->SetPositionRel(m_apScreenBox[ROOTBOX]->GetWidth() - m_apPlayerBox[Players::PlayerThree]->GetWidth() - 20, m_apScreenBox[ROOTBOX]->GetHeight() - m_apPlayerBox[Players::PlayerThree]->GetHeight() - 30);
    m_apPlayerBox[Players::PlayerFour]->SetPositionRel(m_apScreenBox[ROOTBOX]->GetWidth() - m_apPlayerBox[Players::PlayerFour]->GetWidth() - 20, 30);
    UpdatePlayerBars();

    // Place Scene Info popup in convenient position
    if (g_MetaMan.m_Players.size() <= 3)
        m_pSceneInfoPopup->SetPositionRel(m_apScreenBox[ROOTBOX]->GetWidth() -  m_pSceneInfoPopup->GetWidth() - 16, 16);
    else
        m_pSceneInfoPopup->SetPositionRel(m_apScreenBox[ROOTBOX]->GetWidth() -  m_pSceneInfoPopup->GetWidth() - 16, 110);

    // If two or fewer players, place phase box in lower right corner of screen, otherwise center bottom
    if (g_MetaMan.m_Players.size() <= 2)
        m_pPhaseBox->SetPositionRel(m_apScreenBox[ROOTBOX]->GetWidth() - m_pPhaseBox->GetWidth() - 10, m_apScreenBox[ROOTBOX]->GetHeight() - m_pPhaseBox->GetHeight() - 40);
    else
        m_pPhaseBox->SetPositionRel((m_apScreenBox[ROOTBOX]->GetWidth() / 2) - (m_pPhaseBox->GetWidth() / 2), m_apScreenBox[ROOTBOX]->GetHeight() - m_pPhaseBox->GetHeight() - 10);
    

    // Start game of specified size!
    g_MetaMan.NewGame(m_pSizeSlider->GetValue());

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attempts to load a Metagame from disk using the settings set in the
//                  Load Game dialog box.

bool MetagameGUI::LoadGame()
{
    // Get the MetaSave to load from the previously temporarily saved combobox selection
    if (m_pSelectedGameToLoad)
    {
        const MetaSave *saveToLoad = dynamic_cast<const MetaSave *>(m_pSelectedGameToLoad);
        if (saveToLoad)
        {
            if (g_MetaMan.Load(saveToLoad) < 0)
            {
                g_ConsoleMan.PrintString("ERROR: Failed to load Metagame '" + saveToLoad->GetPresetName() + "' from " + saveToLoad->GetSavePath());
                return false;
            }

            // Reconstruct income site lines
            UpdateIncomeCounting(true);

            // Reconstruct the player action lines - they are important!
            for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
                UpdatePlayerActionLines(metaPlayer);

            // Re-init some other GUI elements
            UpdatePlayerBars();
            UpdateScenesBox(true);

            // Make sure GUI boxes are on the screen; save game might have been made on wonky resolution
            for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
                KeepBoxOnScreen(m_apPlayerBox[metaPlayer], 30);
            KeepBoxOnScreen(m_pPhaseBox, 30);
            KeepBoxOnScreen(m_pSceneInfoPopup, 30);

            // Reset some special state vars
            m_PostBattleReview = false;
            m_BattleCausedOwnershipChange = false;

			//Move locations back to screen if they are not visible
			MoveLocationsIntoTheScreen();

            g_ConsoleMan.PrintString("Successfully loaded Metagame '" + saveToLoad->GetPresetName() + "' from " + saveToLoad->GetSavePath());
            m_ContinuePhase = false;
            return true;
        }
    }

    g_ConsoleMan.PrintString("ERROR: Failed to load the Metagame");
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attempts to save a Metagame to disk using the settings set in the
//                  Save Game dialog box.

bool MetagameGUI::SaveGame(string saveName, string savePath, bool resaveSceneData)
{
    // If specified, first load all bitmap data of all Scenes in the current Metagame that have once saved em, so we can re-save them to the new files
    if (resaveSceneData)
        g_MetaMan.LoadSceneData();

    // Set the GameName of the MetaGame to be the save name
    g_MetaMan.m_GameName = saveName;

    // Save any loaded scene data FIRST, so that all the paths of ContentFiles get updated to the actual save location first,
    // which may have been changed due to the saveName being different than before.   
    g_MetaMan.SaveSceneData(METASAVEPATH + saveName);

    // Whichever new or existing, create a writer with the path
    Writer metaWriter(savePath.c_str());
    // Now that all the updated data files have been written to disk and their paths updated, send the MetaMan state for actual writing to an ini
    if (g_MetaMan.Save(metaWriter) < 0)
        return false;

    // Clear out the scene data again so we're not keeping it in memory unnecessarily
    if (resaveSceneData)
        g_MetaMan.ClearSceneData();

    // After successful save, update the corresponding preset to reflect the newly saved game
    // Create a new MetaSave preset that will hold the runtime info of this new save (so it shows up as something we can overwrite later this same runtime)
    MetaSave newSave;
    // This will automatically set all internal members to represent what MetaMan's current state is
    newSave.Create(savePath);
    newSave.SetPresetName(saveName);

    // Now add or update the actual Preset
    g_PresetMan.AddEntityPreset(&newSave, g_PresetMan.GetModuleID(METASAVEMODULENAME), true, string(METASAVEPATH) + string("Index.ini"));

    // Now write out the index file of all MetaSaves so the new save is found on next runtime
    Writer indexWriter((string(METASAVEPATH) + string("Index.ini")).c_str());
    indexWriter.ObjectStart("DataModule");
    indexWriter.NewPropertyWithValue("ModuleName", "Metagame Saves");
    // Get the current list of all MetaSave Preset:s, including the new one we just saved
    list<Entity *> saveList;
    g_PresetMan.GetAllOfType(saveList, "MetaSave");
    // Go through the list and add their names to the combo box
	for (const Entity *saveListEntry : saveList) {
        indexWriter.NewPropertyWithValue("AddMetaSave", saveListEntry);
    }
    indexWriter.ObjectEnd();

    // Report to console
    g_ConsoleMan.PrintString("Successfully saved Metagame  '" + saveName + "' to " + savePath);

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveGameFromDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attempts to save a Metagame to disk using the settings set in the
//                  Save Game dialog box.

bool MetagameGUI::SaveGameFromDialog()
{
    string saveName;
    string savePath;

    // Determine whether we have a new save file or one chosen from the list to overwrite
    // User wrote in a new save file/folder to use, so go ahead and create it
    if (!m_NewSaveBox->GetText().empty())
    {
        saveName = m_NewSaveBox->GetText();
        savePath = METASAVEPATH + m_NewSaveBox->GetText() + ".ini";
    }
    // A game was selected to be overwritten, so extract its path and save to it
    else if (m_pSavesToOverwriteCombo->GetSelectedItem())
    {
        saveName = m_pSavesToOverwriteCombo->GetSelectedItem()->m_Name;
        const MetaSave *pSave = dynamic_cast<const MetaSave *>(m_pSavesToOverwriteCombo->GetSelectedItem()->m_pEntity);
        if (pSave)
            savePath = pSave->GetSavePath();
        // Fall back on just doing a new save with that name
        else
            savePath = METASAVEPATH + m_NewSaveBox->GetText() + ".ini";
    }
    // Coulnd't find a valid name selected by the player?? (GUI problem)
    else
    {
        g_ConsoleMan.PrintString("ERROR: Could not save Metagame, because no valid save name was specified?");
        return false;
    }

    // First load all bitmap data of all Scenes in the current Metagame that have once saved em, so we can re-save them to the new files
    return SaveGame(saveName, savePath, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void MetagameGUI::Update()
{
    // Update the input controller
    m_pController->Update();

    // ToolTip box is hidden by default
    m_pToolTipBox->SetVisible(false);

    // Handle recovering from a completed activity
    if (m_ActivityRestarted || m_ActivityResumed)
        CompletedActivity();

    // Reset the specific triggers
    m_ContinuePhase = false;
    m_ActivityRestarted = false;
    m_ActivityResumed = false;
    m_StartDifficulty = 0;
    m_BackToMain = false;
    m_Quit = false;

    // Don't update the menu if the console is open
    if (g_ConsoleMan.IsEnabled())
        return;

    // Quit now if we aren't enabled
    if (m_MenuEnabled != ENABLED &&  m_MenuEnabled != ENABLING)
        return;

    ////////////////////////////////////////////
    // Do all input handling!

    int mouseX, mouseY;
    m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
    Vector mousePos(mouseX, mouseY);

    UpdateInput();

/*
    ////////////////////////////////////////////
    // Notification blinking logic

    if (m_BlinkMode == NOFUNDS)
    {
        m_pCostLabel->SetVisible((m_BlinkTimer.GetElapsedRealTimeMS() % 500) > 250);
    }
    else if (m_BlinkMode == NOCRAFT)
    {
        bool blink = (m_BlinkTimer.GetElapsedRealTimeMS() % 500) > 250;
        m_pCraftLabel->SetVisible(blink);
        m_pCraftBox->SetVisible(blink);
    }

    // Time out the blinker
    if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastRealMS(1500))
    {
        m_pCostLabel->SetVisible(true);
        m_pCraftLabel->SetVisible(true);
        m_pCraftBox->SetVisible(true);
        m_BlinkMode = NOBLINK;
    }
*/
    //////////////////////////////////////////////////////////////
    // Update screens

    if (m_MenuScreen == NEWDIALOG)
    {
        if (m_ScreenChange)
        {
            m_ScreenChange = false;
        }
    }
    else if (m_MenuScreen == LOADDIALOG)
    {
        if (m_ScreenChange)
        {
            // Clear out the loadable save list control so we can repopulate it
            m_pSavesToLoadCombo->ClearList();
            // Get the list of all read in MetaSave:s
            list<Entity *> saveList;
            g_PresetMan.GetAllOfType(saveList, "MetaSave");
            // Go through the list and add their names to the combo box
            int i = 0;
            int autoSaveIndex = -1;
            MetaSave *pAutoSave = 0;
            for (list<Entity *>::iterator itr = saveList.begin(); itr != saveList.end(); ++itr)
            {
                m_pSavesToLoadCombo->AddItem((*itr)->GetPresetName(), "", 0, *itr);
                // Take note of the autosave, if it is in here
                if ((*itr)->GetPresetName() == AUTOSAVENAME)
                {
                    autoSaveIndex = i;
                    pAutoSave = dynamic_cast<MetaSave *>(*itr);
                }
                ++i;
            }
            // Select the autosave index if we found it
            if (autoSaveIndex >= 0 && pAutoSave)
            {
                m_pSavesToLoadCombo->SetSelectedIndex(autoSaveIndex);
                // Update the game stats info box with the info of the autosave
                char info[512];
				std::string difficultyString;

				if (pAutoSave->GetDifficulty() < Activity::CakeDifficulty)
					difficultyString = "Difficulty: Cake";
				else if (pAutoSave->GetDifficulty() < Activity::EasyDifficulty)
					difficultyString = "Difficulty: Easy";
				else if (pAutoSave->GetDifficulty() < Activity::MediumDifficulty)
					difficultyString = "Difficulty: Medium";
				else if (pAutoSave->GetDifficulty() < Activity::HardDifficulty)
					difficultyString = "Difficulty: Hard";
				else if (pAutoSave->GetDifficulty() < Activity::NutsDifficulty)
					difficultyString = "Difficulty: Nuts";
				else
					difficultyString = "Difficulty: Nuts!";

				std::snprintf(info, sizeof(info), "Game Size: %d sites\nTotal Players: %d\nDay: %d\n%s", pAutoSave->GetSiteCount(), pAutoSave->GetPlayerCount(), pAutoSave->GetRoundCount() + 1, difficultyString.c_str());
                m_pLoadInfoLabel->SetText(info);
                // Show the Load button since we have one locked in
                m_apMetaButton[LOADNOW]->SetVisible(true);
            }
            // Couldn't find the autosave, so leave the combobox empty and let the player select something
            else
            {
                // Clear out the load game info box
                m_pLoadInfoLabel->SetText("");
                // Hide the load button until we have a game to load selected
                m_apMetaButton[LOADNOW]->SetVisible(false);
            }
            m_ScreenChange = false;
        }
    }
    else if (m_MenuScreen == SAVEDIALOG)
    {
        if (m_ScreenChange)
        {
            // Clear out the new save text box
            m_NewSaveBox->SetText("");
            // Clear out the overwrite save list control so we can repopulate it
            m_pSavesToOverwriteCombo->ClearList();
            // Get the list of all read in MetaSave:s
            list<Entity *> saveList;
            g_PresetMan.GetAllOfType(saveList, "MetaSave");
            // Go through the list and add their names to the combo box
            for (list<Entity *>::iterator itr = saveList.begin(); itr != saveList.end(); ++itr)
                m_pSavesToOverwriteCombo->AddItem((*itr)->GetPresetName(), "", 0, *itr);
            // Select the first one - don't, let the player select one if they want to overwrite
//            m_pSavesToOverwriteCombo->SetSelectedIndex(0);
            // Clear out the save game info box
            m_pSaveInfoLabel->SetText("");
            // Hide the save button until we either have a new name or a game to overwrite selected
            m_apMetaButton[SAVENOW]->SetVisible(false);
            m_apMetaButton[SAVENOW]->SetText("Save");
            m_ScreenChange = false;
        }

        // Something written in the new save box, so disable overwriting
        if (!m_NewSaveBox->GetText().empty())
        {
            m_pGUIController->GetControl("SaveOrLabel")->SetVisible(false);
            m_pGUIController->GetControl("SaveAsLabel")->SetVisible(false);
            m_pSavesToOverwriteCombo->SetVisible(false);
            m_pSaveInfoLabel->SetVisible(false);
            // Show the save button since we can now save
            m_apMetaButton[SAVENOW]->SetVisible(true);
        }
        // Overwriting is an option
        else
        {
            m_pGUIController->GetControl("SaveOrLabel")->SetVisible(true);
            m_pGUIController->GetControl("SaveAsLabel")->SetVisible(true);
            m_pSavesToOverwriteCombo->SetVisible(true);
            // Hide the save button if nothing is selected in the overwrite combo box
            m_apMetaButton[SAVENOW]->SetVisible(m_pSavesToOverwriteCombo->GetSelectedItem() && m_pSavesToOverwriteCombo->GetSelectedItem()->m_pEntity);

            // If the button is visible and we're overwriting something, make the player press the Save button twice to confirm overwriting a game
            if (m_apMetaButton[SAVENOW]->GetVisible() && m_apMetaButton[SAVENOW]->GetText() != "Save")
            {
                m_apMetaButton[SAVENOW]->SetText(m_BlinkTimer.AlternateReal(333) ? "CONFIRM?" : "");
                m_pGUIController->GetControl("SaveAsLabel")->SetVisible(m_BlinkTimer.AlternateReal(333));
            }
        }
    }


    /////////////////////////////////////////////////////////////
    // Update based on the MetaGame's state

    // Always show the phase box if we're in-game
    if (g_MetaMan.m_GameState != MetaMan::NOGAME && !g_MetaMan.IsSuspended())
    {
        m_pPhaseBox->SetVisible(true);
        // Never let a game go if we have 0 players
//        RTEAssert(g_MetaMan.m_Players.size() > 0, "Game in progress without any players!");
    }

    // Deselect scenes and player bars on state change
    if (g_MetaMan.m_StateChanged)
    {
        m_pSelectedScene = 0;
        m_ActivePlayerIncomeLines = -1;
        // Go through all lines and make them fully visible
        for (vector<SiteLine>::iterator slItr = m_IncomeSiteLines.begin(); slItr != m_IncomeSiteLines.end(); ++slItr)
            (*slItr).m_OnlyFirstSegments = (*slItr).m_OnlyLastSegments = -1;
        for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
        {
            for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[metaPlayer].begin(); slItr != m_ActionSiteLines[metaPlayer].end(); ++slItr)
                (*slItr).m_OnlyFirstSegments = (*slItr).m_OnlyLastSegments = -1;
        }
    }

    // Game is suspended
    if (g_MetaMan.IsSuspended())
    {
        // Do nothing, game is suspended while in the game menu
        m_AnimTimer1.Reset();
        m_AnimTimer2.Reset();
        m_AnimTimer3.Reset();

        // Show the current menu if we're not showing some confirmation box and we're not already showing the screen we're supposed to be
        if (!m_pConfirmationBox->GetVisible() && !m_apScreenBox[m_MenuScreen]->GetVisible())
            SwitchToScreen(m_MenuScreen);
    }
    // No game yet, show the new game menu
    else if (g_MetaMan.m_GameState == MetaMan::NOGAME)
    {
        m_pPhaseLabel->SetText("Game Not Started");
        if (m_ScreenChange)
        {
            SwitchToScreen(NEWDIALOG);
            UpdatePlayerSetup();
            m_ScreenChange = false;
        }
    }
    else if (g_MetaMan.m_GameState == MetaMan::GAMEINTRO)
    {
        m_pPhaseLabel->SetText("Game Intro");
        m_apMetaButton[CONTINUE]->SetText("Skip Intro");
    }
    else if (g_MetaMan.m_GameState == MetaMan::NEWROUND)
    {
        if (g_MetaMan.m_StateChanged)
        {
            m_pBannerYellowTop->ShowText("DAY", GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 2500, 200);
            m_pBannerYellowBottom->ShowText(GetRoundName(g_MetaMan.m_CurrentRound), GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6,  2500, 300);
        }
        m_pPhaseLabel->SetText("New Day");
        // Blink the start button to draw attention to it
        m_apMetaButton[CONTINUE]->SetText(m_BlinkTimer.AlternateReal(333) ? "> Start <" : "Start");
    }
    else if (g_MetaMan.m_GameState == MetaMan::REVEALSCENES)
    {
        if (g_MetaMan.m_StateChanged)
        {
            m_pBannerYellowTop->HideText(2500, 200);
            m_pBannerYellowBottom->HideText(2500, 300);
//            m_pBannerRedTop->ShowText("Sites Found", GUIBanner::FLYBYLEFTWARD, 1000, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.1, 3500, 200);
        }

        m_pPhaseLabel->SetText("New Sites Found");
        m_apMetaButton[CONTINUE]->SetText("Skip");
        UpdateSiteRevealing();
    }
    else if (g_MetaMan.m_GameState == MetaMan::COUNTINCOME)
    {
        if (g_MetaMan.m_StateChanged)
        {
//            m_pBannerRedBottom->ShowText("Incomes", GUIBanner::FLYBYLEFTWARD, 1000, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.2, 3500, 200);
        }
        m_pPhaseLabel->SetText("Counting Incomes");
        m_apMetaButton[CONTINUE]->SetText("Skip");
        UpdateIncomeCounting();
    }
    else if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
    {
        // Who's turn is it, anyway?
        int metaPlayer = g_MetaMan.m_GameState - MetaMan::PLAYER1TURN;

        // Set up the pre-player-turn intermediate status IF this is a human player still in the game
        if (g_MetaMan.m_Players[metaPlayer].IsHuman() && !g_MetaMan.m_Players[metaPlayer].IsGameOverByRound(g_MetaMan.m_CurrentRound))
        {
            if (g_MetaMan.m_StateChanged)
            {
                // If this human player has no brains left anywhere, there's really nothing he can do, and it's game over for him
                // Communicate it to the loser
                if (g_MetaMan.GetTotalBrainCountOfPlayer(metaPlayer) <= 0)
                {
                    m_apMetaButton[CONTINUE]->SetText("Continue");
                    m_pPhaseLabel->SetText(g_MetaMan.m_Players[metaPlayer].GetName() + "'s Turn");
                    m_pBannerRedTop->ShowText("Game Over", GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 3500, 0);
                    m_pBannerRedBottom->ShowText("for " + g_MetaMan.m_Players[metaPlayer].GetName() + "!", GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6, 3500, 0);

                    // Show a lil descriptive message as to why the game ended
                    m_pGameMessageLabel->SetVisible(true);
                    m_pGameMessageLabel->SetPositionAbs(m_pGameMessageLabel->GetXPos(), (m_apScreenBox[ROOTBOX]->GetHeight() / 2) + 110 - 16);
                    m_pGameMessageLabel->SetText(g_MetaMan.m_Players[metaPlayer].GetName() + "'s brains are all gone, so he/she can do nothing more. Good effort, though!");

                    // Just skip this guy's turn completely
                    m_PreTurn = false;
                }
                // Normal player turn start
                else
                {
                    m_apMetaButton[CONTINUE]->SetText("Start Turn");
                    m_pPhaseLabel->SetText(g_MetaMan.m_Players[metaPlayer].GetName() + "'s Turn");
                    m_pBannerRedTop->ShowText(g_MetaMan.m_Players[metaPlayer].GetName() + "'s", GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 3500, 0);
                    m_pBannerRedBottom->ShowText("Turn", GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6, 3500, 0);
                    m_pGameMessageLabel->SetVisible(false);
                    m_PreTurn = true;
                }
            }

            // Do some more init and then cleanup later
            UpdateHumanPlayerTurn(metaPlayer);
        }
        // Non-human player
        else
        {
            m_pGameMessageLabel->SetVisible(false);
        }

        // Before or after the turn prep start
        if (m_PreTurn)
        {
            // Blink the start button to draw attention to it
            m_apMetaButton[CONTINUE]->SetText(m_BlinkTimer.AlternateReal(333) ? "> Start Turn <" : "Start Turn");
        }
        else
        {
            // Dead player's turn, just waiting for the continue button to be pressed
            if (g_MetaMan.GetTotalBrainCountOfPlayer(metaPlayer) <= 0)
            {
                m_apMetaButton[CONTINUE]->SetText(m_BlinkTimer.AlternateReal(333) ? "> Continue <" : "Continue");
                UpdatePlayerActionLines(metaPlayer);                
            }
            // Normal turn
            else
            {
                m_apMetaButton[CONTINUE]->SetText("End Turn");
                m_pBannerRedTop->HideText(3500, 0);
                m_pBannerRedBottom->HideText(3500, 0);
                UpdatePlayerActionLines(metaPlayer);
            }
        }
    }
    else if (g_MetaMan.m_GameState == MetaMan::BUILDBASES)
    {
        // Hide any banners that might be up
        if (g_MetaMan.m_StateChanged)
        {
            m_pBannerYellowTop->HideText(2500, 200);
            m_pBannerYellowBottom->HideText(2500, 300);
            m_pBannerRedTop->HideText(3500, 0);
            m_pBannerRedBottom->HideText(3500, 0);
        }
        m_apMetaButton[CONTINUE]->SetText("Skip");
        UpdateBaseBuilding();
    }
    else if (g_MetaMan.m_GameState == MetaMan::RUNACTIVITIES)
    {
        // Hide any banners that might be up
        if (g_MetaMan.m_StateChanged)
        {
            m_pBannerYellowTop->HideText(2500, 200);
            m_pBannerYellowBottom->HideText(2500, 300);
            m_pBannerRedTop->HideText(3500, 0);
            m_pBannerRedBottom->HideText(3500, 0);
        }
        m_apMetaButton[CONTINUE]->SetText("Start!");
        UpdateOffensives();
    }
    else if (g_MetaMan.m_GameState == MetaMan::ENDROUND)
    {
        m_pPhaseLabel->SetText("End of Day");
        m_apMetaButton[CONTINUE]->SetText("Continue");
    }
    else if (g_MetaMan.m_GameState == MetaMan::GAMEOVER)
    {
        if (g_MetaMan.m_StateChanged)
        {
            m_pPhaseLabel->SetText("Game Over");
            m_apMetaButton[CONTINUE]->SetText("Good Game!");
            // Which team has the most bases?
            int winnerTeam = g_MetaMan.WhichTeamIsLeading();

            // Noone left??
            if (winnerTeam == Activity::NoTeam)
            {
                m_pBannerRedTop->ShowText("EVERYONE", GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 3500, 0);
                m_pBannerYellowBottom->ShowText("-DIED-", GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6, 3500, 0);                
            }
            else
            {

                // Find out who was on this winning team so we can name them by name
                string winnerNames = "";
                bool plural = false;
                for (vector<MetaPlayer>::iterator pItr = g_MetaMan.m_Players.begin(); pItr != g_MetaMan.m_Players.end(); ++pItr)
                {
                    // WINRAR
                    if ((*pItr).GetTeam() == winnerTeam)
                    {
                        // There's now more than one name in there
                        if (!winnerNames.empty())
                            plural = true;

                        winnerNames = winnerNames + (winnerNames.empty() ? "" : " and ") + (*pItr).GetName();
                    }
                }
                m_pBannerRedTop->ShowText(winnerNames, GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 3500, 0);
                m_pBannerYellowBottom->ShowText(plural ? "WIN!" : "WINS!", GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6, 3500, 0);
//                char winStr[256];
//                std::snprintf(winStr, sizeof(winStr), "Team %d", winner + 1);
//                m_pBannerRedTop->ShowText(winStr, GUIBanner::FLYBYLEFTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.4, 3500, 0);
//                m_pBannerYellowBottom->ShowText("WINS!", GUIBanner::FLYBYRIGHTWARD, -1, Vector(g_FrameMan.GetResX(), g_FrameMan.GetResY()), 0.6, 3500, 0);
            }
        }

        // Show that the game is over because noone has any brains left to do anyhting with
        for (int metaPlayer = 0; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
            m_apBrainPoolLabel[metaPlayer]->SetVisible(m_AnimTimer2.AlternateReal(333));

        // Show a lil descriptive message as to why the game ended
        m_pGameMessageLabel->SetVisible(true);
        m_pGameMessageLabel->SetPositionAbs(m_pGameMessageLabel->GetXPos(), (m_apScreenBox[ROOTBOX]->GetHeight() / 2) + 110 - 16);
        if (g_MetaMan.NoBrainsLeftInAnyPool())
            m_pGameMessageLabel->SetText("All players' brains have been deployed, and so the team with the most\nowned sites (and if tied, the most gold) won the mining contract for this planet!");
        else
            m_pGameMessageLabel->SetText("Only the team which is already in the lead (in owned sites - or gold, if tied)\nhas any brains left to deploy, so they won the mining contract for this planet!");
    }

    // Update site change animations independent of the phase/mode.. they can happen here and there
    UpdateSiteChangeAnim();

    // SCENE SELECTION LOGIC
    if (!g_MetaMan.IsSuspended() && !m_PreTurn)
    {
        if (m_ScreenChange)
        {
//            m_apScreenBox[SCENESCREEN]->SetVisible(true);
//            m_apMetaButton[BACKTOMAIN]->SetVisible(true);
            m_ScreenChange = false;
        }

        Vector screenLocation;

        // Make any player box which is floated over with cursor the one to show its sitelines
        if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
        {
            GUICollectionBox *pBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControlUnderPoint(mouseX, mouseY, m_apScreenBox[ROOTBOX], 1));
            bool found = false;
            for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
            {
                // Find hovered box
                if (pBox == m_apPlayerBox[metaPlayer])
                {
                    m_ActivePlayerIncomeLines = metaPlayer;
                    found = true;
                }
            }
            // Not hovering over any player box, so don't show any lines
            if (!found)
                m_ActivePlayerIncomeLines = Players::NoPlayer;
        }

        // Validate mouse position as being over the planet area for hover operations!
        if (!m_pDraggedBox && (mousePos - m_PlanetCenter).GetMagnitude() < m_PlanetRadius)
        {
            // If unlocked, detect any Scene close to the mouse and highlight it
            bool foundAnyHover = false;
            bool foundNewHover = false;
            vector<Scene *>::iterator sItr;
            vector<Scene *>::iterator newCandidateItr = g_MetaMan.m_Scenes.end();
            float distance = 0;
            float shortestDist = 1000000.0;
            for (sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
            {
                // Only mess with Scenes we can see
                if (!(*sItr)->IsRevealed())
                    continue;

                screenLocation = m_PlanetCenter + (*sItr)->GetLocation() + (*sItr)->GetLocationOffset();
                distance = (screenLocation - mousePos).GetMagnitude();

                // The first new scene the mouse's position is close to when unlocked, make selected
                if (distance < 16 && distance < shortestDist)
                {
                    // This is now the shortest
                    shortestDist = distance;
                    foundAnyHover = true;
                    // See if the scene hovered is different from the previously hovered one, and if so, set it to the new candidate to switch hovering to
// Actually, don't because it will cause alternating each frame if two hover zones overlap!
//                    if (*sItr != m_pHoveredScene)
                        newCandidateItr = sItr;
                }
            }
            
            // Set new hovered scene to be the one now closest to the cursor, if there is any and if it is different the a currently hovered one
            if (newCandidateItr != g_MetaMan.m_Scenes.end() && (*newCandidateItr) != m_pHoveredScene)
            {
                m_pHoveredScene = (*newCandidateItr);
                foundNewHover = true;
                g_GUISound.SelectionChangeSound()->Play();
            }

            // If we didn't find anything to hover over, then remove the hover status
            if (!foundAnyHover)
                m_pHoveredScene = 0;

            // Set up the hover label to appear over any hovered scene location
            if (m_pHoveredScene)
                UpdateSiteNameLabel(true, m_pHoveredScene->GetPresetName(), m_pHoveredScene->GetLocation() + m_pHoveredScene->GetLocationOffset());
            else if (g_MetaMan.m_GameState != MetaMan::COUNTINCOME && g_MetaMan.m_GameState != MetaMan::BUILDBASES && g_MetaMan.m_GameState != MetaMan::RUNACTIVITIES)
                UpdateSiteNameLabel(false);

            // If clicked, whatever is hovered becomes selected
            if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER))
            {
                if (m_pHoveredScene)
                {
                    SelectScene(m_pHoveredScene);
                    g_GUISound.ItemChangeSound()->Play();
                }
/* Can't do this, doesn't take into account clicks on floating UI boxes
                // Not hovering over anything on click, so deselect whatever was selected
                else if (m_pSelectedScene)
                {
                    m_pSelectedScene = 0;
                    g_GUISound.FocusChangeSound()->Play();
                }
*/
            }
        }
    }

    // Update the scene info/action box
    UpdateScenesBox();

    // Update the site lines, if neccessary
    for (int metaPlayer = 0; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
    {
        // The tradestar is a moving target
		if (m_aStationIncomeLineIndices[metaPlayer] >= 0 && !m_IncomeSiteLines.empty())
			m_IncomeSiteLines[m_aStationIncomeLineIndices[metaPlayer]].m_PlanetPoint = m_StationPosOnOrbit;

        // The brain pool counters might also be if player moves the player bar
        if (m_aBrainSaleIncomeLineIndices[metaPlayer] >= 0 && !m_IncomeSiteLines.empty())
        {
            m_IncomeSiteLines[m_aBrainSaleIncomeLineIndices[metaPlayer]].m_CircleSize = 1.0;
            m_IncomeSiteLines[m_aBrainSaleIncomeLineIndices[metaPlayer]].m_PlanetPoint.SetXY(m_apBrainPoolLabel[metaPlayer]->GetXPos() + (m_apBrainPoolLabel[metaPlayer]->GetHAlignment() == GUIFont::Left ? 5 : 16), m_apBrainPoolLabel[metaPlayer]->GetYPos() + 9);
            // Compensate for the fact that we're tracking a screen UI element and not a planetary site
            m_IncomeSiteLines[m_aBrainSaleIncomeLineIndices[metaPlayer]].m_PlanetPoint -= m_PlanetCenter;
        }
    }

    // Update the GUI Banners
    m_pBannerRedTop->Update();
    m_pBannerRedBottom->Update();
    m_pBannerYellowTop->Update();
    m_pBannerYellowBottom->Update();

    // Update the floating Player bars
    UpdatePlayerBars();

    // Save mouse pos for next frame so we can do dragging
    if (m_EngageDrag)
        m_PrevMousePos = mousePos;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void MetagameGUI::Draw(BITMAP *drawBitmap)
{
    // Don't draw site lines and dots if we're in the menus
    if (!g_MetaMan.IsSuspended())
    {
        // Transparency effect on the scene dots and lines
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        // Screen blend the dots and lines, with some flickering in its intensity
		int blendAmount = 130 + RandomNum(-45, 45);
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

        // Draw the scene location dots
        Vector screenLocation;
        BITMAP *pIcon = 0;
        for (vector<Scene *>::const_iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            // Only draw Scenes that are revealed yet
            if (!(*sItr)->IsRevealed())
                continue;

            screenLocation = m_PlanetCenter + (*sItr)->GetLocation() + (*sItr)->GetLocationOffset();
            // If currently being shown as being fought over; make more dramatic
            bool battleSite = m_PostBattleReview && (*sItr) == m_pAnimScene;

            // Find out what team we should show here.. it is not always the current team ownership; but might be a previous one temporarily displayed for dramatic effect
            int team = battleSite ? m_PreBattleTeamOwnership : (*sItr)->GetTeamOwnership();

            // Make sure team is within bounds to show an icon
            pIcon = g_MetaMan.IsActiveTeam(team) ? g_MetaMan.GetTeamIcon(team).GetBitmaps32()[0] : 0;
            if (pIcon)
                masked_blit(pIcon, drawBitmap, 0, 0, screenLocation.m_X - (pIcon->w / 2), screenLocation.m_Y - (pIcon->h / 2), pIcon->w, pIcon->h);
            // Ownership not known, so place nondescript dot instead
            else
            {
                // Make it flicker more if it's currently being fought over
				blendAmount = 95 + (battleSite ? RandomNum(-25, 25) : RandomNum(-15, 15));
                set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
                circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 4, c_GUIColorYellow);
                circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 2, c_GUIColorYellow);
				blendAmount = 210 + RandomNum(-45, 45);
                set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
                circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 1, c_GUIColorYellow);
            }
        }

        // Draw the lines etc pointing at the selected Scene from the Scene Info box
        if (m_pSelectedScene && m_pSceneInfoPopup->GetVisible() && !m_PreTurn)
        {
            Vector sceneInfoBoxPos(m_pSceneInfoPopup->GetXPos() + (m_pSceneInfoPopup->GetWidth() / 2), m_pSceneInfoPopup->GetYPos() + (m_pSceneInfoPopup->GetHeight() / 2));
            DrawScreenLineToSitePoint(drawBitmap, sceneInfoBoxPos, m_pSelectedScene->GetLocation() + m_pSelectedScene->GetLocationOffset(), c_GUIColorWhite, -1, -1, (m_pSceneInfoPopup->GetHeight() / 2) + CHAMFERSIZE + 6, 1.0, g_MetaMan.IsActiveTeam(m_pSelectedScene->GetTeamOwnership()));
        }

        // Draw all the player income site lines we are supposed to
    //    for (vector<SiteLine>::const_iterator slItr = m_IncomeSiteLines.begin(); slItr != m_IncomeSiteLines.end(); ++slItr)
        for (int slI = 0; slI < m_IncomeSiteLines.size(); ++slI)
        {
            // Don't draw these during state change frames, to avoid blinking things inappropriately
            if (m_ContinuePhase || g_MetaMan.m_StateChanged)
                continue;
            // Only draw the lines of the active player, when we're not in RUNACTIVITIES state
            if (g_MetaMan.m_GameState != MetaMan::RUNACTIVITIES && m_IncomeSiteLines[slI].m_Player != m_ActivePlayerIncomeLines)
                continue;
            // If this is the line currently being animated, report back if the line connected with the current parameters
            if (slI == m_AnimIncomeLine)
                m_LineConnected = DrawPlayerLineToSitePoint(drawBitmap, m_IncomeSiteLines[slI]) || m_LineConnected;
            // Just draw the regular unanimated line
            else
                DrawPlayerLineToSitePoint(drawBitmap, m_IncomeSiteLines[slI]);
        }

        // Action lines
        // If during a player's round phase, and not showing any income lines
        if (!m_PreTurn && !g_MetaMan.m_StateChanged && g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::BUILDBASES && m_ActivePlayerIncomeLines == Players::NoPlayer)
        {
            int metaPlayer = 0;
            // Show the lines of the relevant player during turns
            if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
                metaPlayer = g_MetaMan.m_GameState - MetaMan::PLAYER1TURN;
            // If we're showing base building or attacks, then go with the currently animated player
            else if (g_MetaMan.m_GameState == MetaMan::BUILDBASES && m_AnimMetaPlayer < g_MetaMan.m_Players.size())
                metaPlayer = m_AnimMetaPlayer;

            for (int slI = 0; slI < m_ActionSiteLines[metaPlayer].size(); ++slI)
            {
                // Only draw the lines of the active player
//                if (m_ActionSiteLines[metaPlayer][slI].m_Player != m_ActivePlayerIncomeLines)
//                    continue;
                // If this is the line currently being animated, report back if the line connected with the current parameters
                if (slI == m_AnimActionLine)
                    m_LineConnected = DrawPlayerLineToSitePoint(drawBitmap, m_ActionSiteLines[metaPlayer][slI], m_ActionMeterDrawOverride) || m_LineConnected;
                // Just draw the regular unanimated line
                else
                    DrawPlayerLineToSitePoint(drawBitmap, m_ActionSiteLines[metaPlayer][slI], m_ActionMeterDrawOverride);
            }
        }

        // Draw the new site location crosshairs when revealing them
        if (g_MetaMan.m_GameState == MetaMan::REVEALSCENES && !g_MetaMan.m_StateChanged)
        {
            for (vector<SiteTarget>::iterator stItr = m_NewSiteIndicators.begin(); stItr != m_NewSiteIndicators.end(); ++stItr)
                (*stItr).Draw(drawBitmap);
        }

        // Draw any site ownership change animations
//        if (g_MetaMan.m_GameState == MetaMan::REVEALSCENES && !g_MetaMan.m_StateChanged)
        {
            for (vector<SiteTarget>::iterator stItr = m_SiteSwitchIndicators.begin(); stItr != m_SiteSwitchIndicators.end(); ++stItr)
                (*stItr).Draw(drawBitmap);
        }

        // Draw the attack activity crosshairs over the site being violated
        if (g_MetaMan.m_GameState == MetaMan::RUNACTIVITIES && !g_MetaMan.m_StateChanged)
        {
            // Draw the target crosshairs on the attacked site
            m_SiteAttackTarget.Draw(drawBitmap);

            // Draw the attack lines that are currently being revealed by possibly multiple attackers
            for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
            {
                for (int slI = 0; slI < m_ActionSiteLines[metaPlayer].size(); ++slI)
                {
                    DrawPlayerLineToSitePoint(drawBitmap, m_ActionSiteLines[metaPlayer][slI], m_ActionMeterDrawOverride);
                }
            }
        }
    }

    // Back to solid drawing
    drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

    // Draw the GUI elements first
    AllegroScreen drawScreen(drawBitmap);
    m_pGUIController->Draw(&drawScreen);
    m_pGUIController->DrawMouse();

    // Draw the banners on top of all GUI
    if (!g_MetaMan.IsSuspended())
    {
        m_pBannerRedTop->Draw(drawBitmap);
        m_pBannerRedBottom->Draw(drawBitmap);
        m_pBannerYellowTop->Draw(drawBitmap);
        m_pBannerYellowBottom->Draw(drawBitmap);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the user input processing.
// Arguments:       None.
// Return value:    None.

void MetagameGUI::UpdateInput()
{
    // If esc pressed, show campaign dialog if applicable
	if (g_UInputMan.KeyPressed(KEY_ESC)) {
		if (m_MenuScreen == MENUDIALOG) {
			g_MetaMan.SetSuspend(false);
			SwitchToScreen(ROOTBOX);
		} else {
			HideAllScreens();
			g_MetaMan.SetSuspend(true);
			SwitchToScreen(MENUDIALOG);
		}
		return;
	}

    ///////////////////////////////////////////////////////////
    // Mouse handling

    // Get mouse position
    int mouseX, mouseY;
    m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
    Vector mousePos(mouseX, mouseY);
    
    // If not currently dragging a box, see if we should start
    bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
    if (!m_pDraggedBox && menuButtonHeld && !m_EngageDrag)
    {
        GUICollectionBox *pBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControlUnderPoint(mouseX, mouseY, m_apScreenBox[ROOTBOX], 1));

        // Player bars
        for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
        {
            if (pBox == m_apPlayerBox[metaPlayer])
                m_pDraggedBox = pBox;
        }

        // Scene info box?
        if (pBox == m_pSceneInfoPopup)
            m_pDraggedBox = pBox;

        // Phase box?
        if (pBox == m_pPhaseBox)
            m_pDraggedBox = pBox;

        // Save the mouse pos at the start of the drag so we can measure if we should engage
        if (m_pDraggedBox)
            m_PrevMousePos = mousePos;
    }
    // Release if button isn't being held
    else if (!menuButtonHeld)
    {
        m_pDraggedBox = 0;
        m_EngageDrag = false;
    }

    // Figure out dragging of the player bars, if one is being dragged
    if (m_pDraggedBox && !m_EngageDrag)
    {
        // Only start drag if we're over a threshold, to prevent small unintentional nudges
        if ((mousePos - m_PrevMousePos).GetLargest() > 4)
            m_EngageDrag = true;
    }

    // Actually drag if we now are engaged
    if (m_pDraggedBox && m_EngageDrag)
    {
        m_pDraggedBox->MoveRelative(mousePos.m_X - m_PrevMousePos.m_X, mousePos.m_Y - m_PrevMousePos.m_Y);
        // Ensure the drag didn't shove it off-screen
        KeepBoxOnScreen(m_pDraggedBox);
    }

    //////////////////////////////////////////
    // Update the ControlManager

    m_pGUIController->Update();

    //////////////////////////////////////////
    // Update the ToolTip popup

    // Show the ToolTip popup, if we are hovering over anything that has one to show
    string toolTip = "";
    GUIControl *pCurrentHover = m_pGUIController->GetControlUnderPoint(mouseX, mouseY);
    if (g_SettingsMan.ShowToolTips() && pCurrentHover && !(toolTip = pCurrentHover->GetToolTip()).empty())
    {
        // Restart timer if there's a new thing we're hovering over
        if (pCurrentHover != m_pHoveredControl)
            m_ToolTipTimer.Reset();

        // If we've been hovering over the same thing for enough time, then show the tooltip
        if (m_ToolTipTimer.IsPastRealMS(500))
        {
            // Show the popup box with the hovered item's description
            m_pToolTipBox->SetVisible(true);
            // Need to add an offset to make it look better and not have the cursor obscure text
            m_pToolTipBox->SetPositionAbs(mouseX + 6, mouseY + 6);
            m_pToolTipText->SetHAlignment(GUIFont::Left);
            m_pToolTipText->SetText(toolTip);
            // Resize the box height to fit the text
            int newHeight = m_pToolTipText->ResizeHeightToFit();
            m_pToolTipBox->Resize(m_pToolTipBox->GetWidth(), newHeight + 10);
            // Make sure the popup box doesn't drop out of sight
            KeepBoxOnScreen(m_pToolTipBox, -1);
        }

        // Save the control we're currently hovering over so we can compare next frame
        m_pHoveredControl = pCurrentHover;
    }
    else
    {
        m_pToolTipBox->SetVisible(false);
        m_pToolTipText->SetText("");
        m_ToolTipTimer.Reset();
        m_pHoveredControl = 0;
    }


    ///////////////////////////////////////
    // Handle events

	// Phase advance button pressed while in an unpaused game; set the signal flag at the end of input handling
    bool phaseButtonPressed = !g_MetaMan.IsSuspended() && g_UInputMan.AnyStartPress();

	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
		const std::string eventControlName = anEvent.GetControl()->GetName();

        // Commands
		if (anEvent.GetType() == GUIEvent::Command)
        {
			// Open game menu button pressed
			// Most big dialog cancel buttons lead back to the game menu too
			if (eventControlName == "OpenMenuButton" ||
                eventControlName == "SaveCancelButton" ||
                eventControlName == "LoadCancelButton" ||
				(eventControlName == "NewCancelButton" && g_MetaMan.GameInProgress()) ||
                eventControlName == "ConfirmCancelButton")
            {
                g_MetaMan.SetSuspend(true);
                SwitchToScreen(MENUDIALOG);
                g_GUISound.BackButtonPressSound()->Play();
            }

			// Return to main menu button pressed
			else if (eventControlName == "MainMenuButton" || eventControlName == "NewCancelButton")
            {
				//Return Metagame dialog to new game state
				// weegee SwitchToScreen(NEWDIALOG);
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                // Signal that we want to go back to main menu
                m_BackToMain = true;
                g_GUISound.BackButtonPressSound()->Play();
				
            }

			// Open save menu button pressed
			else if (eventControlName == "MenuSaveButton")
            {
                g_MetaMan.SetSuspend(true);
                SwitchToScreen(SAVEDIALOG);
                g_GUISound.ButtonPressSound()->Play();
            }

			// Open load menu button pressed
			else if (eventControlName == "MenuLoadButton" ||
                eventControlName == "NewLoadButton")
            {
                g_MetaMan.SetSuspend(true);
                SwitchToScreen(LOADDIALOG);
                g_GUISound.ButtonPressSound()->Play();
            }

			// New Game menu button pressed
			else if (eventControlName == "MenuNewButton")
            {
                g_MetaMan.SetSuspend(true);
                SwitchToScreen(NEWDIALOG);
                UpdatePlayerSetup();
                g_GUISound.ButtonPressSound()->Play();
            }

			// Quit Program button pressed
			else if (eventControlName == "MenuQuitButton")
            {
                HideAllScreens();
                g_MetaMan.SetSuspend(true);
                m_pConfirmationLabel->SetText("Sure you want to quit to OS?\nAny unsaved progress\nwill be lost!");
                m_pConfirmationButton->SetText("Quit");
                m_pConfirmationBox->SetVisible(true);
                g_GUISound.ButtonPressSound()->Play();
            }

			// Resume Game menu button pressed
			else if (eventControlName == "MenuResumeButton")
            {
                g_MetaMan.SetSuspend(false);
                // If game over, then go to new game dialog on resume
                if (g_MetaMan.m_GameState == MetaMan::GAMEOVER)
                {
                    SwitchToScreen(NEWDIALOG);
                    UpdatePlayerSetup();
                }
                else
                    SwitchToScreen(ROOTBOX);
                g_GUISound.ButtonPressSound()->Play();
            }

			// Confirm button pressed
			if (anEvent.GetControl() == m_apMetaButton[CONFIRM])
            {
                // Confirm Quit Program button
				if (m_pConfirmationButton->GetText() == "Quit")
					System::SetQuit();

                // Do the appropriate thing depending on which screen we're confirming
                if (m_MenuScreen == NEWDIALOG)
                {
//                    if (m_apMetaButton[CONFIRM]->GetText() == "Load")
//                        ;
//                    else if (m_apMetaButton[CONFIRM]->GetText() == "Start New")
                        StartNewGame();
                }
                else if (m_MenuScreen == LOADDIALOG)
                {
                    LoadGame();
                    // Switch directly into the game after a load is done
                    g_MetaMan.SetSuspend(false);
                    SwitchToScreen(ROOTBOX);
                }
                else if (m_MenuScreen == SAVEDIALOG)
                {
                    SaveGameFromDialog();
                    // Go back to the metagame menu after a save is done
                    SwitchToScreen(MENUDIALOG);
                }

                g_MetaMan.SetSuspend(false);
                SwitchToScreen(ROOTBOX);

                g_GUISound.ButtonPressSound()->Play();
            }

            // NEW GAME SETUP DIALOG
            // Player Control Setup toggle buttons
            for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            {
                if (anEvent.GetControl() == m_apPlayerControlButton[player])
                {
                    // Cycle through the different modes
                    if (m_apPlayerControlButton[player]->GetText() == "None")
                        m_apPlayerControlButton[player]->SetText("A.I.");
                    else if (m_apPlayerControlButton[player]->GetText() == "A.I.")
                        m_apPlayerControlButton[player]->SetText("Human");
                    else
                        m_apPlayerControlButton[player]->SetText("None");
                    // Show changes in affected player' gizmos
                    UpdatePlayerSetup();
                    
                    g_GUISound.ButtonPressSound()->Play();
                } 
            }

			// Start New Game menu button pressed
			if (eventControlName == "StartButton")
            {
                // Current game needs saved or there will be data loss, so show confirmation box
                if (!g_MetaMan.GameIsSaved())
                {
                    HideAllScreens();
                    m_pConfirmationLabel->SetText("There is a game going!\nAny unsaved progress\nin it will be lost!");
                    m_pConfirmationButton->SetText("Start New");
                    m_pConfirmationBox->SetVisible(true);
                }
                else
                    StartNewGame();

                g_GUISound.ButtonPressSound()->Play();
            }

			// Save game button pressed
			if (eventControlName == "SaveButton")
            {
                // Overwrite confirmation click has been done already
                if (!m_NewSaveBox->GetText().empty() || m_apMetaButton[SAVENOW]->GetText() != "Save")
                {
                    // Make sure there is somehting to save
                    if (!g_MetaMan.GameIsSaved())
                    {
// TODO: WARN the player that they will have to re-play an offensive battle from start if they save a metagame
/*                        if ()
                        {
                            HideAllScreens();
                            m_pConfirmationLabel->SetText("There is an on-planet battle\ncurrently underway!\nThe progress in it will NOT be saved here;\nyou will have to re-play it\nif you load this game later!");
                            m_pConfirmationButton->SetText("Save Anyway");
                            m_pConfirmationBox->SetVisible(true);
                        }
                        else
                        {
*/
                            SaveGameFromDialog();
                            // Switch back to the menu after a save is done
                            SwitchToScreen(MENUDIALOG);
//                        }
                    }
                    g_GUISound.ButtonPressSound()->Play();
                }
                else
                {
                    m_apMetaButton[SAVENOW]->SetText("CONFIRM?");
                    g_GUISound.ItemChangeSound()->Play();
                }
            }

			// Load game button pressed
			if (eventControlName == "LoadButton" && m_pSavesToLoadCombo->GetSelectedItem())
            {
                // Save this Entity selection because the ComboBox gets cleared out when the conf dlg appears, so we can't get to the selection later when we acutally decide to load the damn thing
                m_pSelectedGameToLoad = m_pSavesToLoadCombo->GetSelectedItem()->m_pEntity;

                // Current game needs saved or there will be data loss, so show conf box
                if (!g_MetaMan.GameIsSaved())
                {
                    HideAllScreens();
                    m_pConfirmationLabel->SetText("There is a game going!\nAny unsaved progress\nin it will be lost!");
                    m_pConfirmationButton->SetText("Load");
                    m_pConfirmationBox->SetVisible(true);
                }
                else
                {
                    LoadGame();
                    // Switch directly into the game after a load is done
                    g_MetaMan.SetSuspend(false);
                    SwitchToScreen(ROOTBOX);
                }

                g_GUISound.ButtonPressSound()->Play();
            }

			// Graphical 'Continue Phase' button pressed; set the signal flag and handle at the end of UpdateInput
			if (anEvent.GetControl() == m_apMetaButton[CONTINUE])
                phaseButtonPressed = true;

			// Scene Action button pressed; this only means to scan at this point
			if (anEvent.GetControl() == m_apMetaButton[SCENEACTION] && m_pSelectedScene)
            {
                // Set up site scan of it (for a price)
                int metaPlayer = g_MetaMan.GetPlayerTurn();
                int team = g_MetaMan.m_Players[metaPlayer].GetTeam();
                // Check if we have enough money for this!
                if (g_MetaMan.GetRemainingFundsOfPlayer(metaPlayer, 0, false, false) < SCANCOST)
                {
                    m_apMetaButton[SCENEACTION]->SetText("NOT ENOUGH FUNDS!");
                    g_GUISound.UserErrorSound()->Play();
                }
                // Ask whether the player wants to scan now or later, with two new buttons
                else
                {
                    m_apMetaButton[SCENEACTION]->SetVisible(false);
                    m_apMetaButton[DESIGNBASE]->SetVisible(false);
                    m_apMetaButton[SCANNOW]->SetVisible(true);
                    m_apMetaButton[SCANLATER]->SetVisible(true);
                    m_pScanInfoLabel->SetVisible(true);
                    m_pScanInfoLabel->SetText("or");

                    g_GUISound.ButtonPressSound()->Play();
                }
            }

			// manual Base Design button pressed; start an activity to let player design the blueprints to be built there
			if (anEvent.GetControl() == m_apMetaButton[DESIGNBASE] && m_pSelectedScene)
            {
                // If owned by this player, then set up base building controls
                if (m_pSelectedScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(g_MetaMan.GetPlayerTurn()))
                {
                    // Set up and start the editor activity for making changes to the base blueprint for this round
                    BaseEditor *pNewEditor = new BaseEditor;
                    pNewEditor->Create();
                    char str[64];
                    std::snprintf(str, sizeof(str), "R%dEdit", g_MetaMan.m_CurrentRound + 1);
                    pNewEditor->SetPresetName(g_MetaMan.GetGameName() + str);

                    // Gotto deact all players since by default there is one in slot 1
                    pNewEditor->ClearPlayers();
                    // Editing player
                    int metaPlayer = g_MetaMan.GetPlayerTurn();
                    int team = g_MetaMan.m_Players[metaPlayer].GetTeam();
// TODO: add all other players of same team, for coop editing?? Whata bout resident brains of lazy teammates who hacen't placed yet
// NO, this is silly. Just make the objects placed by this player marked as placed by him
                    pNewEditor->AddPlayer(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), true, team, 0, &(g_MetaMan.GetTeamIcon(team)));

                    // Send in the Scene to load in later when the screen has faded out etc
                    m_pPlayingScene = m_pSelectedScene;
                    g_SceneMan.SetSceneToLoad(m_pPlayingScene);
                    g_ActivityMan.SetStartActivity(pNewEditor);
                    m_ActivityRestarted = true;

                    g_GUISound.ButtonPressSound()->Play();
                }
            }

			// Scan Now button pressed; time to start the scanning process immediately
			if (anEvent.GetControl() == m_apMetaButton[SCANNOW] && m_pSelectedScene)
            {
                int metaPlayer = g_MetaMan.GetPlayerTurn();
                int team = g_MetaMan.m_Players[metaPlayer].GetTeam();
                // Set up and start the scripted activity for scanning the site for this' team
                GAScripted *pScanActivity = new GAScripted;
                pScanActivity->Create("Base.rte/Activities/SiteScan.lua", "SiteScan");
                char str[64];
                std::snprintf(str, sizeof(str), "R%dScan", g_MetaMan.m_CurrentRound + 1);
                pScanActivity->SetPresetName(g_MetaMan.GetGameName() + str);

                // Gotto deact all players since by default there is one in slot 1
                pScanActivity->ClearPlayers();
                // Add the player who ordered this snooping around
                pScanActivity->AddPlayer(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), true, team, SCANCOST, &(g_MetaMan.GetTeamIcon(team)));
                // Add the players who own this place, if any - their actors and brains are in the scene and should be shown
                if (g_MetaMan.IsActiveTeam(m_pSelectedScene->GetTeamOwnership()))
                {
                    // Go through all players and add the ones of the defending team, based on who has resident brains here
                    for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
                    {
                        // Got to remember to translate from metagame player index into the in-game player index and to flag them as not a human so they dont' get their own screens
//                            if (g_MetaMan.m_Players[mp].GetTeam() == m_pSelectedScene->GetTeamOwnership())
                        if (m_pSelectedScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                            pScanActivity->AddPlayer(g_MetaMan.m_Players[mp].GetInGamePlayer(), false, g_MetaMan.m_Players[mp].GetTeam(), 0, &(g_MetaMan.GetTeamIcon(g_MetaMan.m_Players[mp].GetTeam())));
                    }
                }
                // Send in the Scene to load in later when the screen has faded out etc
                m_pPlayingScene = m_pSelectedScene;
                g_SceneMan.SetSceneToLoad(m_pPlayingScene);
                g_ActivityMan.SetStartActivity(pScanActivity);
                m_ActivityRestarted = true;

                g_GUISound.ButtonPressSound()->Play();
            }

			// Scan Later button pressed; mark the Scene for scanning by this metaplayer's team
			if (anEvent.GetControl() == m_apMetaButton[SCANLATER] && m_pSelectedScene)
            {
                int metaPlayer = g_MetaMan.GetPlayerTurn();
                int team = g_MetaMan.m_Players[metaPlayer].GetTeam();
                // Double-check to make sure we do have the funds to do this AND that the scan hasn't already been paid for
                if (g_MetaMan.m_Players[metaPlayer].m_Funds < SCANCOST && !m_pSelectedScene->IsScanScheduled(team))
                    g_GUISound.UserErrorSound()->Play();
                else
                {
                    // Show the cost change the funds meter
                    FundsChangeIndication(metaPlayer, -SCANCOST, Vector(m_apPlayerBarLabel[metaPlayer]->GetXPos() + m_apPlayerBarLabel[metaPlayer]->GetWidth(), m_apPlayerBarLabel[metaPlayer]->GetYPos()), 2000);
                    // Actually change the player's funds
                    g_MetaMan.m_Players[metaPlayer].m_Funds -= SCANCOST;

                    // Mark the selected Scene to be scanned next time it is loaded by a player of this' team
                    m_pSelectedScene->SetScheduledScan(team, true);

                    // Update the scenes box to show that we have scheduled this scan
                    UpdateScenesBox(true);

                    // Update the budget slider to reflect the scan cost being deducted from the funds
                    if (g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName() == m_pSelectedScene->GetPresetName())
                        m_pSceneBudgetSlider->SetValue(std::floor((g_MetaMan.m_Players[metaPlayer].GetOffensiveBudget() / g_MetaMan.m_Players[metaPlayer].GetFunds()) * 100));

                    // Play an appropriate sound to indicate that the scan is bought and scheduled
                    g_GUISound.ItemChangeSound()->Play();
                }
            }

			// Scene info box close button pressed
			if (anEvent.GetControl() == m_pSceneCloseButton)
            {
                m_pSelectedScene = 0;
                g_GUISound.ButtonPressSound()->Play();
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
                // Also stop dragging any panels if we're over any button
                m_pDraggedBox = 0;
                m_EngageDrag = true;
            }

			// Site budget slider changed
			if(anEvent.GetControl() == m_pSceneBudgetSlider)
            {
                // Update the appropriate budget var
                if (m_pSelectedScene && !g_MetaMan.IsSuspended())
                {
                    // If during a player's round phase, figure out which player
                    if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
                    {
                        int metaPlayer = g_MetaMan.m_GameState - MetaMan::PLAYER1TURN;

                        // Update the scenes box
                        UpdateScenesBox(true);

                        // If owned by this player, then set update base building budget for this Scene
                        float budget = ((float)m_pSceneBudgetSlider->GetValue() / 100.0f) * g_MetaMan.m_Players[metaPlayer].GetFunds();
                        if (m_pSelectedScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer))
                        {
                            m_pSelectedScene->SetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), budget);
                        }
                        // Site owned by enemy player, update the attack budget
                        else if (g_MetaMan.IsActiveTeam(m_pSelectedScene->GetTeamOwnership()))
                        {
                            g_MetaMan.m_Players[metaPlayer].SetOffensiveBudget(budget);
                            g_MetaMan.m_Players[metaPlayer].SetOffensiveTargetName(m_pSelectedScene->GetPresetName());
                        }
                        // Unowned site, update the expedition budget
                        else
                        {
                            g_MetaMan.m_Players[metaPlayer].SetOffensiveBudget(budget);
                            g_MetaMan.m_Players[metaPlayer].SetOffensiveTargetName(m_pSelectedScene->GetPresetName());
                        }
                    }
                }

                // Also stop dragging any panels if we're over any button
                m_pDraggedBox = 0;
                m_EngageDrag = true;
            }

            // Checkbox for changing the auto design setting of a scene changed; update the actual scene to reflect this
			if (anEvent.GetControl() == m_pAutoDesignCheckbox && m_pSelectedScene)
            {
                if (anEvent.GetMsg() == GUICheckbox::Changed)
                    m_pSelectedScene->SetAutoDesigned(m_pAutoDesignCheckbox->GetCheck());
            }

			// Game Size slider changed
			if(anEvent.GetControl() == m_pSizeSlider || anEvent.GetControl() == m_pGoldSlider || anEvent.GetControl() == m_pLengthSlider || anEvent.GetControl() == m_pDifficultySlider)
                UpdateGameSizeLabels();

			if(anEvent.GetControl() == m_apPlayerAISkillSlider[Players::PlayerOne])
                UpdateAISkillSliders(Players::PlayerOne);
			if(anEvent.GetControl() == m_apPlayerAISkillSlider[Players::PlayerTwo])
                UpdateAISkillSliders(Players::PlayerTwo);
			if(anEvent.GetControl() == m_apPlayerAISkillSlider[Players::PlayerThree])
                UpdateAISkillSliders(Players::PlayerThree);
			if(anEvent.GetControl() == m_apPlayerAISkillSlider[Players::PlayerFour])
                UpdateAISkillSliders(Players::PlayerFour);

/* Dun work so well, only if hovering naked box
            // If hovering over any player bar, show the site lines going out from it
            for (int metaPlayer = Players::PlayerOne; metaPlayer < Players::MaxPlayerCount; ++metaPlayer)
            {
                if (anEvent.GetControl() == m_apPlayerBox[metaPlayer])
                    m_ActivePlayerIncomeLines = metaPlayer;
            }
*/
            // Player flag selection changed
            for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            {
                if (anEvent.GetControl() == m_apPlayerTeamSelect[player] && anEvent.GetMsg() == GUIComboBox::Closed)
                {
/* Not relevant anymore, more than one Player can be of the same Team
                    // If some flag has been selected, make sure all other lists don't show the currently selected icon
                    if (m_apPlayerTeamSelect[player]->GetSelectedItem() && m_apPlayerTeamSelect[player]->GetSelectedItem()->m_pEntity)
                    {
                        for (int otherPlayer = Players::PlayerOne; otherPlayer < Players::MaxPlayerCount; ++otherPlayer)
                        {
                            if (player != otherPlayer && m_apPlayerTeamSelect[otherPlayer]->GetSelectedItem() && m_apPlayerTeamSelect[otherPlayer]->GetSelectedItem()->m_pEntity)
                            {
                                // They're the same, so swap them so they'll all stay unique
                                if (m_apPlayerTeamSelect[player]->GetSelectedItem()->m_pEntity->GetPresetName() == m_apPlayerTeamSelect[otherPlayer]->GetSelectedItem()->m_pEntity->GetPresetName())
                                {
                                    // Gotto use the old selection previous to the list opening
                                    int swapIndex = m_apPlayerTeamSelect[player]->GetOldSelectionIndex();
                                    m_apPlayerTeamSelect[player]->SetSelectedIndex(m_apPlayerTeamSelect[otherPlayer]->GetSelectedIndex());
                                    m_apPlayerTeamSelect[otherPlayer]->SetSelectedIndex(swapIndex);
                                }
                            }
                        }
                    }
*/
					// Update sliders
					UpdateAISkillSliders(player);

                    UpdatePlayerSetup();
//                    g_GUISound.ButtonPressSound()->Play();
                }
            }

            // Load game combo box selection changed
            if (anEvent.GetControl() == m_pSavesToLoadCombo && anEvent.GetMsg() == GUIComboBox::Closed)
            {
                // Get a handle to the actual game save Entity so we can extract its info
                const MetaSave *pGame = 0;
                GUIListPanel::Item *pItem = m_pSavesToLoadCombo->GetSelectedItem();
                if (pItem)
                    pGame = dynamic_cast<const MetaSave *>(pItem->m_pEntity);
                if (pGame)
                {
                    // Update the game stats info box with the info of the selected game
                    char info[512];
					std::string difficultyString;

					if (pGame->GetDifficulty() < Activity::CakeDifficulty)
						difficultyString = "Difficulty: Cake";
					else if (pGame->GetDifficulty() < Activity::EasyDifficulty)
						difficultyString = "Difficulty: Easy";
					else if (pGame->GetDifficulty() < Activity::MediumDifficulty)
						difficultyString = "Difficulty: Medium";
					else if (pGame->GetDifficulty() < Activity::HardDifficulty)
						difficultyString = "Difficulty: Hard";
					else if (pGame->GetDifficulty() < Activity::NutsDifficulty)
						difficultyString = "Difficulty: Nuts";
					else
						difficultyString = "Difficulty: Nuts!";

                    std::snprintf(info, sizeof(info), "Game Size: %d sites\nTotal Players: %d\nDay: %d\n%s", pGame->GetSiteCount(), pGame->GetPlayerCount(), pGame->GetRoundCount() + 1, difficultyString.c_str());
                    m_pLoadInfoLabel->SetText(info);
                    // Show the Load button since we have one locked in
                    m_apMetaButton[LOADNOW]->SetVisible(true);
                }
                // Hide the Load button if we don't have a valid game selected
                else
                    m_apMetaButton[LOADNOW]->SetVisible(false);
                // Play a ding
                g_GUISound.ItemChangeSound()->Play();
            }

            // Save overwrite combo box selection changed
            if (anEvent.GetControl() == m_pSavesToOverwriteCombo && anEvent.GetMsg() == GUIComboBox::Closed)
            {
                // Get a handle to the actual game save Entity so we can extract its info
                const MetaSave *pGame = 0;
                GUIListPanel::Item *pItem = m_pSavesToOverwriteCombo->GetSelectedItem();
                if (pItem)
                    pGame = dynamic_cast<const MetaSave *>(pItem->m_pEntity);
                if (pGame)
                {
                    // Update the game stats info box with the info of the selected game
                    char info[512];
					std::string difficultyString;

					if (pGame->GetDifficulty() < Activity::CakeDifficulty)
						difficultyString = "Difficulty: Cake";
					else if (pGame->GetDifficulty() < Activity::EasyDifficulty)
						difficultyString = "Difficulty: Easy";
					else if (pGame->GetDifficulty() < Activity::MediumDifficulty)
						difficultyString = "Difficulty: Medium";
					else if (pGame->GetDifficulty() < Activity::HardDifficulty)
						difficultyString = "Difficulty: Hard";
					else if (pGame->GetDifficulty() < Activity::NutsDifficulty)
						difficultyString = "Difficulty: Nuts";
					else
						difficultyString = "Difficulty: Nuts!";

                    std::snprintf(info, sizeof(info), "Game Size: %d sites\nTotal Players: %d\nDay: %d\n%s", pGame->GetSiteCount(), pGame->GetPlayerCount(), pGame->GetRoundCount() + 1, difficultyString.c_str());
                    m_pSaveInfoLabel->SetText(info);
                    m_pSaveInfoLabel->SetVisible(true);
                    // Show the Save button since we have one to overwrite locked in
                    m_apMetaButton[SAVENOW]->SetVisible(true);
                    m_apMetaButton[SAVENOW]->SetText("Save");
                }
                // Hide the Save button if we don't have a valid game selected, AND no new save name to use
                else if (m_NewSaveBox->GetText().empty())
                {
                    m_pSaveInfoLabel->SetVisible(false);
                    m_apMetaButton[SAVENOW]->SetVisible(false);
                }

                // Play a ding
                g_GUISound.ItemChangeSound()->Play();
            }
        }
    }

	// Phase advance button pressed; set the signal flag at the end of input handling
	if (phaseButtonPressed)
    {
        // If we're in a pre-player-turn state, then continue out of it first
        if (m_PreTurn && g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
            m_PreTurn = false;
        // If we/re going through offensive ativities, then that's what the continue button starts
        else if (m_PreTurn && g_MetaMan.m_GameState == MetaMan::RUNACTIVITIES)
            m_PreTurn = false;
        else if (g_MetaMan.m_GameState == MetaMan::GAMEOVER)
            SwitchToScreen(MENUDIALOG);
        else
            m_ContinuePhase = true;
        g_GUISound.ButtonPressSound()->Play();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideAllScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides all menu screens, so one can easily be unhidden and shown only.

void MetagameGUI::HideAllScreens()
{
    for (int iscreen = 0; iscreen < SCREENCOUNT; ++iscreen)
    {
        if (m_apScreenBox[iscreen] && iscreen != ROOTBOX)
            m_apScreenBox[iscreen]->SetVisible(false);
    }

    // Also hide the player bars
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        m_apPlayerBox[player]->SetVisible(false);
        m_apBrainPoolLabel[player]->SetVisible(false);
        m_apFundsChangeLabel[player]->SetVisible(false);
        m_apBrainChangeLabel[player]->SetVisible(false);
        m_apPlayerTeamActionBox[player]->SetVisible(false);
        m_apPlayerBrainTravelLabel[player]->SetVisible(false);
    }

    // Message label too
    m_pGameMessageLabel->SetVisible(false);

    // And the phase, scene info, and confirmation boxes
    m_pPhaseBox->SetVisible(false);
    m_pSceneInfoPopup->SetVisible(false);
    m_pConfirmationBox->SetVisible(false);
    UpdateSiteNameLabel(false);

    // And the big text banners
    m_pBannerRedTop->HideText();
    m_pBannerRedBottom->HideText();
    m_pBannerYellowTop->HideText();
    m_pBannerYellowBottom->HideText();

    m_ScreenChange = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeepBoxOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure a specific box doesn't end up moved completely off-screen.

void MetagameGUI::KeepBoxOnScreen(GUICollectionBox *pBox, int margin)
{
    if (margin < 0)
    {
        // Use the dimensions of the box itself to prevent it from at all going outside the screen
        if (pBox->GetXPos() < 0)
            pBox->SetPositionAbs(0, pBox->GetYPos());
        if (pBox->GetXPos() > m_apScreenBox[ROOTBOX]->GetWidth() - pBox->GetWidth())
            pBox->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth() - pBox->GetWidth(), pBox->GetYPos());
        if (pBox->GetYPos() < 0)
            pBox->SetPositionAbs(pBox->GetXPos(), 0);
        if (pBox->GetYPos() > m_apScreenBox[ROOTBOX]->GetHeight() - pBox->GetHeight())
            pBox->SetPositionAbs(pBox->GetXPos(), m_apScreenBox[ROOTBOX]->GetHeight() - pBox->GetHeight());
    }
    else
    {
        // Make sure the box doesn't go entirely outside of the screen
        if (pBox->GetXPos() < (-pBox->GetWidth() + margin))
            pBox->SetPositionAbs(-pBox->GetWidth() + margin, pBox->GetYPos());
        if (pBox->GetXPos() > m_apScreenBox[ROOTBOX]->GetWidth() - margin)
            pBox->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth() - margin, pBox->GetYPos());
        if (pBox->GetYPos() < (-pBox->GetHeight() + margin))
            pBox->SetPositionAbs(pBox->GetXPos(), -pBox->GetHeight() + margin);
        if (pBox->GetYPos() > m_apScreenBox[ROOTBOX]->GetHeight() - margin)
            pBox->SetPositionAbs(pBox->GetXPos(), m_apScreenBox[ROOTBOX]->GetHeight() - margin);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CompletedActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Handles what happens after an Activity within the Metagame was
//                  run and completed fully.

void MetagameGUI::CompletedActivity()
{
    Activity *pDoneActivity = dynamic_cast<Activity *>(g_ActivityMan.GetActivity());
    GAScripted *pDoneScriptedActivity = dynamic_cast<GAScripted *>(g_ActivityMan.GetActivity());
    Scene *pAlteredScene = g_SceneMan.GetScene();
    bool autoResolved = false;
    int winningTeam = Activity::NoTeam;

    // Retain any changes done to the Scene just played to the one that is kept by the MetaMan session
    if (pDoneActivity && pAlteredScene && m_pPlayingScene)
    {
        // There was an error with the activity and it bailed.. try to recover
        if (pDoneActivity->GetActivityState() == Activity::HasError)
        {
            m_AnimTimer2.Reset();
            m_apMetaButton[CONTINUE]->SetText("Start!");
            m_PreTurn = true;
        }
        // If a MetaFight activity isn't over yet, that means we're just pausing and will resume
        else if (pDoneScriptedActivity && pDoneScriptedActivity->GetLuaClassName() == "MetaFight" && !pDoneActivity->IsOver())
        {
            m_AnimTimer2.Reset();
            m_apMetaButton[CONTINUE]->SetText("Resume");
            m_BattleToResume = true;
            m_PreTurn = true;
        }
        // All types of activites should be saving their scenes to disk - objects may have placed/stamped onto their layers
        else
        {
            // Coming out of a base design session
            if (pDoneActivity->GetClassName() == "BaseEditor")
            {
                // ?
            }
            // Site scan session
            else if (pDoneScriptedActivity && pDoneScriptedActivity->GetLuaClassName() == "SiteScan")
            {
                // ?
            }
            // Offensive action session needs some things done after a battle is concluded
            else if (pDoneScriptedActivity && pDoneScriptedActivity->GetLuaClassName() == "MetaFight")
            {
                if (pDoneActivity->IsOver())
                {
                    // If this ended for whatever reason without a winning team, then resolve the rest of the fight automatically
                    if (pDoneScriptedActivity->GetWinnerTeam() == Activity::NoTeam)//pDoneActivity->HumanBrainCount() == 0)
                    {
                        // Resolve the rest of the fight between the AI guys and display the outcome
                        m_BattleCausedOwnershipChange = AutoResolveOffensive(pDoneScriptedActivity, pAlteredScene, true);

                        autoResolved = true;
                        winningTeam = pDoneScriptedActivity->GetWinnerTeam();
                    }
                    // If it ended and is truly over, see if it resulted in a team ownership change
                    else
                        m_BattleCausedOwnershipChange = m_pPlayingScene->GetTeamOwnership() != pAlteredScene->GetTeamOwnership();

                    // Copy the completed activity back on the one in the queue that it started as, so we can grab parameters for the post battle review
                    g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->Destroy();
                    g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->Create(*pDoneScriptedActivity);
                    // Scrub the module ID so the migration goes well.. this is a bit hacky, but ok in this special case
                    g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->SetModuleID(-1);
                    // Remind the Offensive that it is a unique snowflake and should save itself as such
                    g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->MigrateToModule(g_PresetMan.GetModuleID(METASAVEMODULENAME));

                    // We are now reviewing the battle after it has concluded; UpdateOffensives will show all that stuff
                    m_PreTurn = true;
                    m_PostBattleReview = true;
                    // Pause for a little to allow players to reorient themselves visually on the new screen before animaitons start happening
                    ChangeAnimMode(SHOWPOSTBATTLEPAUSE);
                }
            }

            // Transfer back the brain to the Scene as a resident AND collect the remaining funds left over
            // UNLESS we're editing the base, which handles the brains specially by not actually placing them into the scene, but always keeping them as residents in the scene
            // Also BaseEditor doesn't use funds at all, so there's nothing to collect
            if (pDoneActivity->GetClassName() != "BaseEditor")
            {
                // Don't mess with this if activity was automatically resolved - then the resident brains and funds adjustments are already taken care of
                if (!autoResolved)
                    pAlteredScene->RetrieveResidentBrains(*pDoneActivity);
/* Later in FinalizeOffensive
                // Deduct the original funds contribution of each player - less any unused funds of the team, taking original player contribution ratios into account
                for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
                {
// ANIMATE THIS NICELY INSTEAD
                    // Deduct the player's original contribution to team funds
                    g_MetaMan.m_Players[metaPlayer].m_Funds -= pDoneActivity->GetPlayerFundsContribution(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer());
                    // Add back whatever his share of whatever his team has left in the fight at its end
                    g_MetaMan.m_Players[metaPlayer].m_Funds += pDoneActivity->GetPlayerFundsShare(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer());
                    // IF This guy was ATTACKING this turn, adjust his attack budget to match what just happened in the battle
                    // so in case he is defending in a upcoming battle this turn, his avaialbe funds will be accurately calculated
                    if (g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName() == m_pAnimScene->GetPresetName())
                    {
                        g_MetaMan.m_Players[metaPlayer].SetOffensiveBudget(g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsShare(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer()));
//                        g_MetaMan.m_Players[metaPlayer].SetOffensiveTargetName("");
                    }
                    // Update the ratios of the meter now that the funds have changed
                    UpdatePlayerLineRatios(m_ActionSiteLines[metaPlayer], metaPlayer, false, g_MetaMan.m_Players[metaPlayer].m_Funds);
                }
*/
            }
            // Suck up all the remaining Actors and Items left in the world and put them into the list to place next load
            // However, don't suck up actors of any non-winning team, and don't save the brains if we autoresolved, because that took care of placing the resident brains already
            pAlteredScene->RetrieveActorsAndDevices(winningTeam, autoResolved);
            // Save out the altered scene before clearing out its data from memory
            pAlteredScene->SaveData(METASAVEPATH + string(AUTOSAVENAME) + " - " + pAlteredScene->GetPresetName());
            // Clear the bitmap data etc of the altered scene, we don't need to copy that over
            pAlteredScene->ClearData();
            // Deep copy over all the edits made to the newly played Scene
            m_pPlayingScene->Destroy();
            m_pPlayingScene->Create(*pAlteredScene);
            // Null the current scene, which is pointed to by pAlteredScene.
            g_SceneMan.ClearCurrentScene();
            // Scrub the module ID so the migration goes well.. this is a bit hacky, but ok in this special case
            m_pPlayingScene->SetModuleID(-1);
            m_pPlayingScene->GetTerrain()->SetModuleID(-1);
            // Remind the Scene that it is a unique snowflake and should save itself as such
            m_pPlayingScene->MigrateToModule(g_PresetMan.GetModuleID(METASAVEMODULENAME));
            // We're not playing this anymore
            m_pPlayingScene = 0;
            // Auto save the entire MetaMan state too
// Do this later in FinalizeOffensive, since the funds change until then
//            SaveGame(AUTOSAVENAME, METASAVEPATH + string(AUTOSAVENAME) + ".ini", false);

            // Update the Scene info box since the scene might have changed
            UpdateScenesBox(true);

            // Clear out the Lua state completely so it's not running some BS in the background
            g_LuaMan.Destroy();
            g_LuaMan.Initialize();
            g_PresetMan.ReloadAllScripts();

            // Play some nice ambient music
            g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1, 0.4);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AutoResolveOffensive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Automatically resolves an offensive fight without actually launching
//                  and going through an Activity. Will randomly determine who won and
//                  what the consequences are.

bool MetagameGUI::AutoResolveOffensive(GAScripted *pOffensive, Scene *pScene, bool brainCheck)
{
    bool changedOwnership = false;
    const Loadout *pLoadout = 0;
    Actor *pBrain = 0;
    float cost = 0;
/*
    // Find the scene being attacked in this offensive Activity
    Scene *pScene = 0;
    for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
    {
        if ((*sItr)->IsRevealed() && (*sItr)->GetPresetName() == pOffensive->GetSceneName())
            pScene = (*sItr);
    }
*/
    RTEAssert(pScene, "Couldn't find the Site that has been selected as auto resolution of an attack!");

    // Check all players for active brains, and deactivate them if they don't have them
    if (brainCheck)
    {
        for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
        {
            // Deactivate any players which had brains but don't anymore - ie they died and are not in the game anymore
            if (pOffensive->PlayerActive(player) && pOffensive->PlayerHadBrain(player) && !pOffensive->GetPlayerBrain(player))
                pOffensive->DeactivatePlayer(player);
        }
    }
    // If we're not making this check, we should assume all active players had a brain at some point
    // This is effectively a simluation for pure offensive activities where no human player was ever involved
    else
    {
        for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
        {
            // Simulate that all active players had a brain at some point
            if (pOffensive->PlayerActive(player))
                pOffensive->SetPlayerHadBrain(player, true);
        }        
    }

    // SINGLE player going to an unowned site.. it will be always be taken over, at some base cost of a brain landing
    if (pOffensive->GetPlayerCount() == 1)
    {
        // Find out which single player is visiting this place, and grant him victory
        int activePlayer = Players::NoPlayer;
        for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
        {
            if (pOffensive->PlayerActive(player))
            {
                activePlayer = player;
                break;
            }
        }
        // Add a resident brain to this scene for the AI player, based on the metaplayer's Loadout corresponding to its tech
        MetaPlayer *pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(activePlayer);
        if (pMetaPlayer)
        {
            // NOTE: Brain pool resource counter gets adjusted down in FinalizeOffense

            // Find the player's tech's brain, and instantiate it
            if (pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Brain", pMetaPlayer->GetNativeTechModule())))
                pBrain = pLoadout->CreateFirstActor(pMetaPlayer->GetNativeTechModule(), pMetaPlayer->GetForeignCostMultiplier(), pMetaPlayer->GetNativeCostMultiplier(), cost);
            // Pass the instance and ownership thereof to the scene as a resident
            if (pBrain)
            {
                // Set a pos outside the Scene so it'll be gracefully placed later when Scene's Terrain is actually loaded
                pBrain->SetPos(Vector(-1, -1));
				pBrain->SetTeam(pMetaPlayer->GetTeam());
                pScene->SetResidentBrain(activePlayer, pBrain);
                // Set his team to own this place now
                pScene->SetTeamOwnership(pOffensive->GetTeamOfPlayer(activePlayer));
                // Also declare winner of the activity
                pOffensive->SetWinnerTeam(pOffensive->GetTeamOfPlayer(activePlayer));
                // Just mess with the funds; the metaplayers' funds will be affected afterward, according to their shares etc
//                pOffensive->SetTeamFunds(pOffensive->GetTeamFunds(pOffensive->GetTeamOfPlayer(activePlayer)) * RandomNum(), pOffensive->GetTeamOfPlayer(activePlayer));
                // For now, just deduct the price of the brain
                pOffensive->ChangeTeamFunds(-cost, pOffensive->GetTeamOfPlayer(activePlayer));
                // Signal that ownership of the site has changed
                changedOwnership = true;
            }
        }
    }
    // MULTIPLE players are battling it out.. randomize the outcome
    else
    {
        // First see if NO TEAM will get this - could be all brains die
        if (RandomNum() < 0.05F)
        {
            // See if we should signal change of ownership
            if (pScene->GetTeamOwnership() != Activity::NoTeam)
                changedOwnership = true;

            // Eliminate all ownership of this place
			pScene->RemoveAllPlacedActors(Activity::NoTeam);
            pScene->SetTeamOwnership(Activity::NoTeam);

            // Each player's brain gets wiped out
            for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            {
                if (pOffensive->PlayerActive(player))
                    pScene->SetResidentBrain(player, 0);
            }
        }
        // Okay, someone got this place - now figure out who
        else
        {
            MetaPlayer * aMetaPlayers[Players::MaxPlayerCount];
            // The normalized scalar chances of each team to win
            float aTeamChance[Activity::MaxTeamCount];
			for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
				aTeamChance[team] = 0;

            // The total number of 'chance' points that all teams have in aggregate
            float totalPoints = 0;

            // Add the points representing the defense investment for the defenders
            for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
            {
                // Invested defenses counts for half the points.. some may have been destroyed, defense is harder etc
                if (pOffensive->TeamActive(team) && pScene->GetTeamOwnership() == team)
                    aTeamChance[team] += pScene->GetTotalInvestment() / 2;
            }

            // Now add the funds contributed by each player to each team's chance pool
            for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            {
                if (pOffensive->PlayerActive(player))
                {
                    aMetaPlayers[player] = g_MetaMan.GetMetaPlayerOfInGamePlayer(player);
                    // Add the funds contributed to this activity's attack or defense
                    if (aMetaPlayers[player])
                        aTeamChance[aMetaPlayers[player]->GetTeam()] += pOffensive->GetPlayerFundsContribution(player);
                }
            }

            // Now tally up the total chance points
            for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
            {
                if (pOffensive->TeamActive(team))
                    totalPoints += aTeamChance[team];
            }
            // Normalize all the teams' chances
            for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
            {
                if (pOffensive->TeamActive(team))
                    aTeamChance[team] = aTeamChance[team] / totalPoints;
            }
            // The deciding normalized scalar number
            float decision = RandomNum();
            // Keeps track of the thresholds
            float teamChanceTally = 0;
            int winnerTeam = Activity::NoTeam;
            // See who actually won, based on the respective chances
            for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
            {
                if (pOffensive->TeamActive(team))
                {
                    // Move the threshold forward
                    teamChanceTally += aTeamChance[team];
                    // See if this team was chosen as winner
                    if (decision < teamChanceTally)
                    {
                        // Declare winner
                        winnerTeam = team;
                        // See if we should signal change of ownership
                        if (pScene->GetTeamOwnership() != winnerTeam)
                            changedOwnership = true;
                        // We have picked our winner and are done
						// NOTE: Actual scene owners will be set below, along with removal of actors
						// and door replacements.
                        break;
                    }
                }
            }

            // Now change brain residencies if the ownership actually changed
            if (changedOwnership)
            {
                for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
                {
                    pLoadout = 0;
                    pBrain = 0;
                    if (pOffensive->PlayerActive(player) && aMetaPlayers[player])
                    {
                        // Winning player, so place a brain of his as a resident
                        if (aMetaPlayers[player]->GetTeam() == winnerTeam)
                        {
                            // NOTE: Brain pool resource counter gets adjusted down in FinalizeOffense

                            // Find the player's tech's brain, and instantiate it
                            if (pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Brain", aMetaPlayers[player]->GetNativeTechModule())))
                                pBrain = pLoadout->CreateFirstActor(aMetaPlayers[player]->GetNativeTechModule(), aMetaPlayers[player]->GetForeignCostMultiplier(), aMetaPlayers[player]->GetNativeCostMultiplier(), cost);
                            // Pass the instance and ownership thereof to the scene as a resident
                            if (pBrain)
                            {
                                // Set a pos outside the Scene so it'll be gracefully placed later when Scene's Terrain is actually loaded
                                pBrain->SetPos(Vector(-1, -1));
                                pScene->SetResidentBrain(player, pBrain);
                                // Deduct the cost of the brain from the player's funds
// This is covered by the global cost adjustments below
//                                aMetaPlayers[player]->ChangeFunds(-cost);
                            }
                            // Change the ownership of all placed doors to the winning team
                            pScene->SetOwnerOfAllDoors(winnerTeam, player);
                        }
                        // Losing player, clear out his resident brain, if any
                        else
                            pScene->SetResidentBrain(player, 0);
                    }
                    // Nonexistent player, so clear out the resident brain just to be safe
                    else
                        pScene->SetResidentBrain(player, 0);
                }
                // Remove all remaining actors placed in blueprints etc by teams other than the winning one
                pScene->RemoveAllPlacedActors(winnerTeam);

                // Actually change ownership and declare winner team
                pScene->SetTeamOwnership(winnerTeam);
                pOffensive->SetWinnerTeam(winnerTeam);
            }

            // Figure out who of the losers actually evacuated instead of dying
            for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
            {
                // LOSER.. but evacuated successfully?
                if (pOffensive->PlayerActive(player) && pOffensive->PlayerHadBrain(player) && aMetaPlayers[player] && aMetaPlayers[player]->GetTeam() != winnerTeam)
                    pOffensive->SetBrainEvacuated(player, RandomNum() < 0.25F);
            }
        }

        // Now make the party cost for all the teams
        for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        {
            // Only mess with active teams
            if (pOffensive->TeamActive(team))
            {
                // Just mess with the funds; the metaplayers' funds will be affected afterward, according to their shares
                // Never let team funds dip below 0
                pOffensive->SetTeamFunds(MAX(0, pOffensive->GetTeamFunds(team) * RandomNum()), team);
            }
        }
    }

    return changedOwnership;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSiteRevealing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New Site Revealing animation

void MetagameGUI::UpdateSiteRevealing()
{
    // First set up all the targets we'll be animating into view
    if (g_MetaMan.m_StateChanged)
    {
        // Save the number of sites we have reveelaed up til now so we can compare
        m_AnimCountStart = m_AnimCountCurrent = (int)std::floor(g_MetaMan.m_RevealedScenes);
        // Progress the number of site we have revealed with the set rate + the extra
        g_MetaMan.m_RevealedScenes += g_MetaMan.m_RevealRate + g_MetaMan.m_RevealExtra;
        // Reset the extra to 0 now after we've applied it
        g_MetaMan.m_RevealExtra = 0;
        // Don't reveal more than there are scenes!
        if ((int)std::floor(g_MetaMan.m_RevealedScenes) >= g_MetaMan.m_Scenes.size())
            g_MetaMan.m_RevealedScenes = g_MetaMan.m_Scenes.size();
        // Figure out how many new sites we gots this round
        int delta = (int)std::floor(g_MetaMan.m_RevealedScenes) - m_AnimCountStart;
        // No new sites this round, so just continue onto next phase!
        if (delta < 1)
        {
            m_ContinuePhase = true;
            return;
        }
        // Where we need to go with the animation
        m_AnimCountEnd = m_AnimCountStart + delta;
        RTEAssert(m_AnimCountEnd <= g_MetaMan.m_Scenes.size(), "Trying to reveal more scenes than there are!");

        // Clear and add target crosshairs pointing out all the new scenes
        m_NewSiteIndicators.clear();
        m_AnimTimer1.Reset();
        // We want to start right away
        m_AnimTimer1.SetElapsedRealTimeMS(1000);
        m_AnimTimer2.Reset();
    }

    // Show a new set of crosshairs
    if (m_AnimTimer1.GetElapsedRealTimeMS() > 600 && m_AnimCountCurrent < m_AnimCountEnd)
    {
        // Make the scene draw and selectable in the metagame view
        g_MetaMan.m_Scenes[m_AnimCountCurrent]->SetRevealed(true);
        // Create and set up its crosshairs
// TODO: TEMP change this back
        m_NewSiteIndicators.push_back(SiteTarget(m_PlanetCenter + g_MetaMan.m_Scenes[m_AnimCountCurrent]->GetLocation() + g_MetaMan.m_Scenes[m_AnimCountCurrent]->GetLocationOffset(), 0, SiteTarget::CROSSHAIRSSHRINK, c_GUIColorYellow, m_AnimTimer2.GetElapsedRealTimeMS()));
        m_SiteSwitchIndicators.push_back(SiteTarget(m_PlanetCenter + g_MetaMan.m_Scenes[m_AnimCountCurrent]->GetLocation() + g_MetaMan.m_Scenes[m_AnimCountCurrent]->GetLocationOffset(), 0, SiteTarget::CIRCLEGROW, c_GUIColorYellow, m_AnimTimer2.GetElapsedRealTimeMS()));
        // Increment the current scene index
        m_AnimCountCurrent++;
        // Reset the timer so we will get the next reveal in the set time
        m_AnimTimer1.Reset();
    }

    // Animate the existing crosshairs
    double shrinkTime = 0;
    double shrinkInterval = 600;
    for (int i = 0 ; i < (m_AnimCountCurrent - m_AnimCountStart); ++i)
    {
        // How long have we been animating this one?
        shrinkTime = m_AnimTimer2.GetElapsedRealTimeMS() - m_NewSiteIndicators[i].m_StartTime;
        // If it's still in the shrink interval, keep doing it
        if (shrinkTime < shrinkInterval)
            m_NewSiteIndicators[i].m_AnimProgress = EaseOut(0.0, 1.0, shrinkTime / shrinkInterval);
        // If it's after, then just keep pulsating the target
        else
            m_NewSiteIndicators[i].m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)shrinkTime % (int)shrinkInterval) / shrinkInterval);
    }

    // Set the phase label grammatically correct
    m_pPhaseLabel->SetText((m_AnimCountEnd - m_AnimCountStart) > 1 ? "Gold Sites Found!" : "Gold Site Found!");

    // If we are done showing things, then make the continue button blink
    if (m_NewSiteIndicators.size() == m_AnimCountEnd - m_AnimCountStart)
    {
        m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Continue <" : "Continue");
    }

    // Clean up if we're going onto next phase
    if (m_ContinuePhase)
    {
        m_NewSiteIndicators.clear();

        // Reveal all sites that are supposed to ahve been revelaed if player skipped the animation early
        for (m_AnimCountCurrent = m_AnimCountStart; m_AnimCountCurrent < m_AnimCountEnd; ++m_AnimCountCurrent)
            if (m_AnimCountCurrent < g_MetaMan.m_Scenes.size())
                g_MetaMan.m_Scenes[m_AnimCountCurrent]->SetRevealed(true);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSiteChangeAnim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates team ownership change animations, if any.

void MetagameGUI::UpdateSiteChangeAnim()
{
    // Continue animating the existing markers
    double animInterval = 600;
    double animTime = 0;
    for (vector<SiteTarget>::iterator itr = m_SiteSwitchIndicators.begin(); itr != m_SiteSwitchIndicators.end(); ++itr)
    {
        // How long have we been animating this one?
        animTime = (*itr).m_AnimTimer.GetElapsedRealTimeMS();
        // If it's still in the shrink interval, keep doing it
        if (animTime < animInterval)
            (*itr).m_AnimProgress = EaseOut(0.0, 1.0, animTime / animInterval);
//        // If it's after, then just keep pulsating the target
//        else
//            (*itr).m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)animTime % (int)animInterval) / animInterval);
    }

    // Remove the markers that are done animating
    for (vector<SiteTarget>::iterator itr = m_SiteSwitchIndicators.begin(); itr != m_SiteSwitchIndicators.end();)
    {
        if ((*itr).m_AnimTimer.GetElapsedRealTimeMS() > animInterval)//(*itr).m_AnimProgress >= 1.0)
        {
            m_SiteSwitchIndicators.erase(itr);
            // Start over so we don't have trouble with invalidated iterators
            itr = m_SiteSwitchIndicators.begin();
        }
        else
            itr++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateIncomeCounting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Count Income animation

void MetagameGUI::UpdateIncomeCounting(bool initOverride)
{
    // First set up all the site lines we should be animating into view
    if (g_MetaMan.m_StateChanged || initOverride)
    {
        // Make note of all funds from previous round
        for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
            (*mpItr).m_PhaseStartFunds = (*mpItr).m_Funds;

        // If this is the very first round and no Scenes are owned yet, then we should just skip this
        // Don't return out of this yet thuogh because there's some finalization going on in the end of this func
        if (g_MetaMan.m_CurrentRound == 0)
            m_ContinuePhase = true;

        // Init all the SiteLine:s and make them hidden; we will reveal each one sequentially with an animation
        m_IncomeSiteLines.clear();
        for (int metaPlayer = 0; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
            m_aStationIncomeLineIndices[metaPlayer] = -1;
        m_AnimMetaPlayer = m_ActivePlayerIncomeLines = Players::PlayerOne;
        m_pAnimScene = 0;
        float totalRent, totalIncome, totalEndFunds;
        int channelHeight;
        // Loop through the players
        do
        {
            // If this player is DONE FOR, skip income counting completely
            if (g_MetaMan.GetTotalBrainCountOfPlayer(m_AnimMetaPlayer) <= 0 ||
                g_MetaMan.m_Players[m_AnimMetaPlayer].IsGameOverByRound(g_MetaMan.m_CurrentRound))
            {
                m_aBrainSaleIncomeLineIndices[m_AnimMetaPlayer] = -1;
                UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer, false);
                continue;
            }

            // Only charge rent if there's any brains left in pool
// Make this rent variable somehow??
            totalRent = g_MetaMan.m_Players[m_AnimMetaPlayer].GetBrainPoolCount() > 0 ? TRADESTARRENT : 0;
            totalIncome = g_MetaMan.GetSceneIncomeOfPlayer(m_AnimMetaPlayer);
            totalEndFunds = g_MetaMan.m_Players[m_AnimMetaPlayer].m_PhaseStartFunds + totalIncome - totalRent;
            channelHeight = 60;

            // TRADESTAR BANK ACCOUNT AND RENT
            // Add line to show existing funds stored in space station, and deduct rent from
            m_IncomeSiteLines.push_back(SiteLine(m_AnimMetaPlayer, 0, 1.0, m_StationPosOnOrbit, "TradeStar Midas", 0, c_GUIColorYellow, -1, 0, channelHeight, 2.0f));
            m_IncomeSiteLines.back().m_FundsAmount = g_MetaMan.m_Players[m_AnimMetaPlayer].m_PhaseStartFunds;
            m_IncomeSiteLines.back().m_FundsTarget = m_IncomeSiteLines.back().m_FundsAmount - totalRent;
            // Save the index so we can update the line later
            m_aStationIncomeLineIndices[m_AnimMetaPlayer] = m_IncomeSiteLines.size() - 1;

            // SCENE INCOME
            // Loop through the scenes owned by that player, setting up the site line for each
            while (m_pAnimScene = g_MetaMan.GetNextSceneOfPlayer(m_AnimMetaPlayer, m_pAnimScene))
            {
                m_IncomeSiteLines.push_back(SiteLine(m_AnimMetaPlayer, 1.0, 0, m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), m_pAnimScene->GetPresetName(), m_pAnimScene, c_GUIColorYellow, -1, 0, channelHeight, 1.0f, g_MetaMan.IsActiveTeam(m_pAnimScene->GetTeamOwnership())));
                // Star them at 0, make them go to the round income for this base
                m_IncomeSiteLines.back().m_FundsAmount = 0;
                m_IncomeSiteLines.back().m_FundsTarget = m_pAnimScene->GetRoundIncome();
                channelHeight += 10;
            }

            // BRAIN LIQUIDATION if funds will end up under 0
            if (totalEndFunds <= 0 || g_MetaMan.GetTotalBrainCountOfPlayer(m_AnimMetaPlayer) <= 0)
            {
                m_IncomeSiteLines.push_back(SiteLine(m_AnimMetaPlayer, 1.0, 0, Vector(m_apBrainPoolLabel[m_AnimMetaPlayer]->GetXPos() + (m_apBrainPoolLabel[m_AnimMetaPlayer]->GetHAlignment() == GUIFont::Left ? 5 : 16), m_apBrainPoolLabel[m_AnimMetaPlayer]->GetYPos() + 9) - m_PlanetCenter, "Brain Liquidation", 0, c_GUIColorYellow, -1, 0, channelHeight, 2.0f, false));
                // Start at 0 and grow to how much the brain is worth
                m_IncomeSiteLines.back().m_FundsAmount = 0;
                m_IncomeSiteLines.back().m_FundsTarget = BRAINGOLDVALUE;
                m_aBrainSaleIncomeLineIndices[m_AnimMetaPlayer] = m_IncomeSiteLines.size() - 1;
            }
            else
                m_aBrainSaleIncomeLineIndices[m_AnimMetaPlayer] = -1;

            // This will set up all meter ratios properly, based on the actual funds numbers
// TODO, really?: "actually not a good idea here, the values above are good init"
            UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer, false);
        }
        while (++m_AnimMetaPlayer < g_MetaMan.m_Players.size());

        // Start animating these lines appearing, one after another
        m_AnimIncomeLine = 0;
        m_AnimIncomeLineChange = true;
        m_AnimTimer1.Reset();
        ChangeAnimMode(PAUSEANIM);
    }

    // Animate sitelines into view, one by one
    if (!initOverride && !m_IncomeSiteLines.empty() && m_AnimIncomeLine < m_IncomeSiteLines.size())
    {
        // Did the players change? If so, pause
//        if (m_AnimMetaPlayer != m_IncomeSiteLines[m_AnimIncomeLine].m_Player && !m_AnimTimer1.IsPastRealMS(1000))
//            ChangeAnimMode(PAUSEANIM);
        // If a new line, choose which animation is appropriate
        if (m_AnimIncomeLineChange)
        {
            // Which player is this line of?
            m_AnimMetaPlayer = m_ActivePlayerIncomeLines = m_IncomeSiteLines[m_AnimIncomeLine].m_Player;
            // Station line, blink its meter and grow outward to the station
            if (m_AnimIncomeLine == m_aStationIncomeLineIndices[m_AnimMetaPlayer])
                ChangeAnimMode(BLINKMETER);
            // Regular site line, start with shrinking circle around the site and draw the line backward toward the bar
            else
                ChangeAnimMode(SHRINKCIRCLE);

            m_AnimIncomeLineChange = false;
        }

        if (m_AnimMode == BLINKCIRCLE)
        {
/*
            // Show the meter
            m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = -1;
            m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = 1;
            // Start blinking
            if (!m_AnimTimer1.IsPastRealMS(600))
            {
                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(m_AnimTimer1.AlternateReal(150));
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = m_AnimTimer1.AlternateReal(150) ? 0 : 1;
            }
            else
            {
                // Leave the label showing
                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(true);
                // Start connecting the meter to the circle!
                ChangeAnimMode(LINECONNECTFW);
            }
*/
        }
        else if (m_AnimMode == SHRINKCIRCLE)
        {
            if (NewAnimMode())
            {
                m_AnimTimer1.Reset();
                // Start with the circle showing
                m_AnimSegment = 1;
                m_AnimFundsMax = 0;
                m_LineConnected = false;
            }
            // Show the circle
            m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = m_AnimSegment;
            // Show the site name over the site loc
            UpdateSiteNameLabel(true, m_IncomeSiteLines[m_AnimIncomeLine].m_SiteName, m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint);
            // Shrink by certain rate
            if (!m_AnimTimer1.IsPastRealMS(350))
            {
                // If line to space station, special case animation
                if (m_AnimIncomeLine == m_aStationIncomeLineIndices[m_AnimMetaPlayer])
                    m_IncomeSiteLines[m_AnimIncomeLine].m_CircleSize = EaseOut(20.0, 2.0, EaseOut(0, 1.0, m_AnimTimer1.GetElapsedRealTimeMS() / 350));
                else
                    m_IncomeSiteLines[m_AnimIncomeLine].m_CircleSize = EaseOut(7.0, 1.0, EaseOut(0, 1.0, m_AnimTimer1.GetElapsedRealTimeMS() / 350));

                m_AnimTimer2.Reset();
            }
            // Finished shrinking circle to the target size, now pause and blink it for a lil bit
            else if (!m_AnimTimer2.IsPastRealMS(600))
            {
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = m_AnimTimer2.AlternateReal(150) ? 0 : 1;
                // Also finish the circle in the destination size
                m_IncomeSiteLines[m_AnimIncomeLine].m_CircleSize = m_AnimIncomeLine == m_aStationIncomeLineIndices[m_AnimMetaPlayer] ? 2.0 : 1.0;
            }
            else
            {
                // Start connecting the circle to the player bar!
                ChangeAnimMode(LINECONNECTBW);
                m_LineConnected = false;
            }
        }
        else if (m_AnimMode == LINECONNECTFW)
        {
            if (NewAnimMode())
            {
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = 0;
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = -1;
                m_LineConnected = false;
                // Hide the site name over the site loc at first
                UpdateSiteNameLabel(false);
            }

            // If line not yet connected, keep revealing segments
            if (!m_LineConnected)
            {
                if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
                {
                    m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = ++m_AnimSegment;
                    m_AnimTimer1.Reset();
                }
            }
/*
            // Oh! Line now completed, pause for a while before continuing to next line
            else if (!m_AnimTimer1.IsPastRealMS(1500))
            {
                // Show the site name over the site loc while we're waiting
                UpdateSiteNameLabel(true, m_IncomeSiteLines[m_AnimIncomeLine].m_SiteName, m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint, 1.3);
            }
*/
            // Done waiting, continue to next line
            else
            {
                // If this is also the last line of a player, then do the retract animation
//                if ((m_AnimIncomeLine + 1) >= m_IncomeSiteLines.size() || m_IncomeSiteLines[m_AnimIncomeLine + 1].m_Player != m_AnimMetaPlayer)
//                    ChangeAnimMode(RETRACTLINES);
                // Just another line on the same player, so start animating it immediately
//                else
                {
                    m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = -1;
/*
                    // Hide the site label again
                    UpdateSiteNameLabel(false);
                    m_AnimIncomeLine++;
                    m_AnimIncomeLineChange = true;
*/
                    // Going to show rent being deducted from the tradestar
                    ChangeAnimMode(SHRINKMETER);
                }
            }
        }
        else if (m_AnimMode == LINECONNECTBW)
        {
            if (NewAnimMode())
            {
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = -1;
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = 0;
                m_LineConnected = false;
            }
            // Show the site name over the site loc
            UpdateSiteNameLabel(true, m_IncomeSiteLines[m_AnimIncomeLine].m_SiteName, m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint);
            // Need to set up the ratio animation before we draw the line so it appears as the meter is 0
            if (m_AnimFundsMax == 0)
            {
                m_AnimFundsMax = m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount;
                m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount = 0;
            }

            // If line not yet connected, keep revealing segments
            if (!m_LineConnected)
            {
                if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
                {
                    m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = ++m_AnimSegment;
                    m_AnimTimer1.Reset();
                }
            }
            // Oh! Line now completed, start growing the meter
            else
                ChangeAnimMode(GROWMETER);
        }
        else if (m_AnimMode == BLINKMETER)
        {
            if (NewAnimMode())
            {
                m_AnimTimer1.Reset();
                // Show the meter
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = -1;
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = 1;
            }
            // Start blinking
            if (!m_AnimTimer1.IsPastRealMS(1500))
            {
                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(m_AnimTimer1.AlternateReal(150));
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyFirstSegments = m_AnimTimer1.AlternateReal(150) ? 1 : 0;
            }
            else
            {
                // Leave the label showing
                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(true);
                // Start connecting the meter to the circle!
                ChangeAnimMode(LINECONNECTFW);
                m_AnimSegment = 1;
            }
        }
        // Grow each meter from 0 to its target ratio
        else if (m_AnimMode == GROWMETER)
        {
            if (NewAnimMode())
            {
                // Longer if we're liquidating brain.. it's important
                m_AnimModeDuration = m_AnimIncomeLine == m_aBrainSaleIncomeLineIndices[m_AnimMetaPlayer] ? 4000 : 2000;
                m_AnimTimer1.Reset();
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = -1;
                // Show the change 
                FundsChangeIndication(m_AnimMetaPlayer, m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget - m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount, Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth(), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos()), m_AnimModeDuration);

                // Show the brain being sucked away if this is a brain liquidation income event
                if (m_AnimIncomeLine == m_aBrainSaleIncomeLineIndices[m_AnimMetaPlayer])
                {
                    // This is the text label showing the brain going away
                    BrainsChangeIndication(m_AnimMetaPlayer, -1, Vector(m_apBrainPoolLabel[m_AnimMetaPlayer]->GetXPos(), m_apBrainPoolLabel[m_AnimMetaPlayer]->GetYPos()), m_apBrainPoolLabel[m_AnimMetaPlayer]->GetHAlignment(), m_AnimModeDuration);
                    // This is the display adjustment to the actual counter; the final actual change at the end of the animation
                    g_MetaMan.m_Players[m_AnimMetaPlayer].ChangeBrainsInTransit(1);
                }
            }
            // Show the site name over the site loc
            UpdateSiteNameLabel(true, m_IncomeSiteLines[m_AnimIncomeLine].m_SiteName, m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint);
            // Grow for a certain amount of time
            if (!m_AnimTimer1.IsPastRealMS(m_AnimModeDuration))
            {
                // Make this animation correlate in duration with the funds change label that rises
                m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount = EaseOut(0, m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget, m_AnimTimer1.GetElapsedRealTimeMS() / m_AnimModeDuration);
                g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds = GetPlayerLineFunds(m_IncomeSiteLines, m_AnimMetaPlayer);
                UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer);
                m_AnimTimer2.Reset();
            }
            // Finished growing the meter to the target size
            else
            {
                UpdateSiteNameLabel(false);
                m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount = m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget;
                g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds = GetPlayerLineFunds(m_IncomeSiteLines, m_AnimMetaPlayer);
                UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer);

                // Check if there's more lines to draw, and if so, if the next one is of a different player
                // OR if there's no lines left at all, just retract the last player's lines we just finished
                // Then pause to retract all the lines of the just finished player
                if ((m_AnimIncomeLine + 1) >= m_IncomeSiteLines.size() || m_IncomeSiteLines[m_AnimIncomeLine + 1].m_Player != m_AnimMetaPlayer)
                {
                    // Wait for a little bit when we've displayed all sites of a player
                    if (m_AnimTimer2.IsPastRealMS(500))
                        ChangeAnimMode(RETRACTLINES);
                }
                // Just another line on the same player, so start animating it immediately
                else
                {
                    m_AnimIncomeLine++;
                    m_AnimIncomeLineChange = true;
                    ChangeAnimMode(SHRINKCIRCLE);
                }
            }
        }
        // Shrink the station line to show it costing rent
        else if (m_AnimMode == SHRINKMETER)
        {
            if (NewAnimMode())
            {
                m_AnimModeDuration = 2000;
                m_AnimTimer1.Reset();
                m_IncomeSiteLines[m_AnimIncomeLine].m_OnlyLastSegments = -1;
                // Show the change, if any
                if (fabs(m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget - m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount) > 0)
                {
                    // Show why we are paying money
                    PlayerTextIndication(m_AnimMetaPlayer, "TradeStar brain storage rent", Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + (m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth() / 2), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos() + (m_apPlayerBarLabel[m_AnimMetaPlayer]->GetHeight() / 2)), m_AnimModeDuration);
                    FundsChangeIndication(m_AnimMetaPlayer, m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget - m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount, Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth(), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos()), m_AnimModeDuration);
                }
                // Indicate why we're not paying anything
                else
                    PlayerTextIndication(m_AnimMetaPlayer, "No brains; no rent!", Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + (m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth() / 2), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos() + (m_apPlayerBarLabel[m_AnimMetaPlayer]->GetHeight() / 2)), m_AnimModeDuration);

/* This is done above on init now
                // Only charge rent if we've still got brains at the tradestar
                if (g_MetaMan.m_Players[m_AnimMetaPlayer].GetBrainPoolCount() > 0)
                {
                    // Establish the after-rent target
                    m_AnimFundsMin = m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount - TRADESTARRENT;
                    // Show the change in rent
                    FundsChangeIndication(m_AnimMetaPlayer, -TRADESTARRENT, Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth(), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos()), 2000);
                }
                // No change in funds
                else
                    m_AnimFundsMin = m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount;
*/
            }
            // Show the site name over the site loc
//            UpdateSiteNameLabel(true, m_IncomeSiteLines[m_AnimIncomeLine].m_SiteName, m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint, 1.2);
            // This'll always be the tradestar, so hardcode some descriptive text
            UpdateSiteNameLabel(true, "TradeStar Midas", m_IncomeSiteLines[m_AnimIncomeLine].m_PlanetPoint, 1.25);
            // Shrink for a certain amount of time
            if (!m_AnimTimer1.IsPastRealMS(m_AnimModeDuration))
            {
                // Make this animation correlate in duration with the funds change label that rises
                m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount = EaseOut(g_MetaMan.m_Players[m_AnimMetaPlayer].m_PhaseStartFunds, m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget, m_AnimTimer1.GetElapsedRealTimeMS() / m_AnimModeDuration);
                UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer);
                g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds = GetPlayerLineFunds(m_IncomeSiteLines, m_AnimMetaPlayer);
                m_AnimTimer2.Reset();
            }
            // Finished shrinking the meter to the target size
            else
            {
                UpdateSiteNameLabel(false);
                m_IncomeSiteLines[m_AnimIncomeLine].m_FundsAmount = m_IncomeSiteLines[m_AnimIncomeLine].m_FundsTarget;
                g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds = GetPlayerLineFunds(m_IncomeSiteLines, m_AnimMetaPlayer);
                UpdatePlayerLineRatios(m_IncomeSiteLines, m_AnimMetaPlayer);

                // Check if there's more lines to draw, and if so, if the next one is of a different player
                // OR if there's no lines left at all, just retract the last player's lines we just finished
                // Then pause to retract all the lines of the just finished player
                if ((m_AnimIncomeLine + 1) >= m_IncomeSiteLines.size() || m_IncomeSiteLines[m_AnimIncomeLine + 1].m_Player != m_AnimMetaPlayer)
                {
                    // Wait for a little bit when we've displayed all sites of a player
                    if (m_AnimTimer2.IsPastRealMS(500))
                        ChangeAnimMode(RETRACTLINES);
                }
                // Just another line on the same player, so start animating it immediately
                else
                {
                    m_AnimIncomeLine++;
                    m_AnimIncomeLineChange = true;
                    ChangeAnimMode(SHRINKCIRCLE);
                }
            }
        }
        // Retract all the lines for a specific player from the sites to the bar
        else if (m_AnimMode == RETRACTLINES)
        {
            if (NewAnimMode())
            {
                m_AnimTimer1.Reset();
                // A few extra will give some pause
                m_AnimSegment = 7;
                // Stop showing the site label
                UpdateSiteNameLabel(false);
            }

            if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
            {
                m_AnimSegment--;
                m_AnimTimer1.Reset();
                // Go through all lines and animate away the ones belonging to this player
                for (vector<SiteLine>::iterator slItr = m_IncomeSiteLines.begin(); slItr != m_IncomeSiteLines.end(); ++slItr)
                {
                    if ((*slItr).m_Player == m_AnimMetaPlayer)
                    {
                        (*slItr).m_OnlyFirstSegments = m_AnimSegment;
                        (*slItr).m_OnlyLastSegments = -1;
                    }
                }
            }
            if (m_AnimSegment == 0)
            {
                // Have another player's lines to animate, so start doing that
                m_AnimIncomeLine++;
                m_AnimIncomeLineChange = true;
                ChangeAnimMode(SHRINKCIRCLE);
            }
        }
    }

    // If no more lines, DONE, continue phase to next
    if (m_AnimIncomeLine >= m_IncomeSiteLines.size() && !initOverride)
        m_ContinuePhase = true;

    // Phase ending, make sure everything is set up to continue
    if (m_ContinuePhase || initOverride)
    {
        // Set all lines' fund amounts to their targets and reset their segment animations so we can see them
        int lineIndex = 0;
        for (vector<SiteLine>::iterator slItr = m_IncomeSiteLines.begin(); slItr != m_IncomeSiteLines.end(); ++slItr)
        {
            (*slItr).m_FundsAmount = (*slItr).m_FundsTarget;
            (*slItr).m_OnlyFirstSegments = -1;
            (*slItr).m_OnlyLastSegments = -1;

            // Also, if there are any brain liquidation lines, then adjust the brain counter for that poor player
            if ((*slItr).m_Player >= Players::PlayerOne && (*slItr).m_Player < Players::MaxPlayerCount &&
                m_aBrainSaleIncomeLineIndices[(*slItr).m_Player] == lineIndex)
            {
                // Remove the display adjustment and APPLY the actual change to brains when one is liquidated
                g_MetaMan.m_Players[(*slItr).m_Player].SetBrainsInTransit(0);
                g_MetaMan.m_Players[(*slItr).m_Player].ChangeBrainPoolCount(-1);
            }
            // Keep this synched with the iterator
            lineIndex++;
        }
        // Hide all lines for rendering.. appropriate lines will show up when user mouseovers bars in turn phases
        m_ActivePlayerIncomeLines = -1;

        // Make sure all fund labels and line ratios are good
        for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
        {
            UpdatePlayerLineRatios(m_IncomeSiteLines, metaPlayer, false);
            m_apPlayerBarLabel[metaPlayer]->SetVisible(true);
            // Set all funds to the final values, if not a gameover guy
            if (g_MetaMan.GetTotalBrainCountOfPlayer(metaPlayer) > 0)
                g_MetaMan.m_Players[metaPlayer].m_Funds = GetPlayerLineFunds(m_IncomeSiteLines, metaPlayer, false);

			if (g_SettingsMan.EndlessMetaGameMode())
			{
				g_MetaMan.m_Players[metaPlayer].ChangeBrainPoolCount(20 - g_MetaMan.m_Players[metaPlayer].GetBrainPoolCount());
				g_MetaMan.m_Players[metaPlayer].ChangeFunds(10000 - g_MetaMan.m_Players[metaPlayer].GetFunds());
			}
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateHumanPlayerTurn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates a human player's turn

void MetagameGUI::UpdateHumanPlayerTurn(int metaPlayer)
{
    // In-game player - IMPORTANT to pass this to the Scenes, and not the metaplayer
    int player = g_MetaMan.m_Players[metaPlayer].GetInGamePlayer();

    // First do setup
    if (g_MetaMan.m_StateChanged)
    {
        // Reset the target so we don't put the player into autopilot (and also he might not have brains to deploy!)
        g_MetaMan.m_Players[metaPlayer].SetOffensiveBudget(0);
        g_MetaMan.m_Players[metaPlayer].SetOffensiveTargetName("");
        // Re-set up all build budgets in oz to match what the player spent on this site last round, proportionally
        for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            // Only mess with Scenes we can see and that are owned by this player, and he spent something on last turn
            if ((*sItr)->IsRevealed() && (*sItr)->GetTeamOwnership() == g_MetaMan.m_Players[metaPlayer].GetTeam() && (*sItr)->GetBuildBudgetRatio(player) > 0)
                (*sItr)->SetBuildBudget(player, g_MetaMan.m_Players[metaPlayer].GetFunds() * (*sItr)->GetBuildBudgetRatio(player));
        }
    }

    // Phase ENDING, make sure everything is set up to continue
    if (m_ContinuePhase)
    {
        // Save all the base building budget ratios sowe can re-set the gold values next turn, for player convenience
        for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            // Only mess with Scenes we can see and that are owned by this player, and he spent something on last turn
            if ((*sItr)->IsRevealed() && (*sItr)->GetTeamOwnership() == g_MetaMan.m_Players[metaPlayer].GetTeam() && g_MetaMan.m_Players[metaPlayer].GetFunds() > 0)
                (*sItr)->SetBuildBudgetRatio(player, (*sItr)->GetBuildBudget(player) / g_MetaMan.m_Players[metaPlayer].GetFunds());
            else
                (*sItr)->SetBuildBudgetRatio(player, 0);
        }

        // Hide the game message label
        m_pGameMessageLabel->SetVisible(false);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateBaseBuilding
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Base Building animation

void MetagameGUI::UpdateBaseBuilding()
{
    m_pPhaseLabel->SetText("Building Bases");

    // First do setup
    if (g_MetaMan.m_StateChanged)
    {
        // Make sure all fund labels and line ratios are good
        for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
        {
            // Save the fund levels FROM THE START so we can calculate the after state if players skip the animation
            g_MetaMan.m_Players[metaPlayer].m_PhaseStartFunds = g_MetaMan.m_Players[metaPlayer].m_Funds;

            // Update the player action lines for all players one last time; this will catch the AI ones as well and reflect their actions
            UpdatePlayerActionLines(metaPlayer);

            // Hide all lines so we only see their meters
            for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[metaPlayer].begin(); slItr != m_ActionSiteLines[metaPlayer].end(); ++slItr)
            {
                (*slItr).m_OnlyFirstSegments = 1;
                (*slItr).m_OnlyLastSegments = -1;
            }
            UpdatePlayerLineRatios(m_ActionSiteLines[metaPlayer], metaPlayer, false, g_MetaMan.m_Players[metaPlayer].m_Funds);

            // Also, for human players, auto design their blueprints if they have chosen to do so
            if (g_MetaMan.m_Players[metaPlayer].IsHuman())
            {
                // Go through all scenes and check if they're owned by this, and if so, whether their defenses should be automatically designed by canned AI plan
                for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
                {
                    // Move building pieces from the Scene's AI plan queue to the actual blueprints, but only approximately as much as can afford, so the entire AI pre-built base plan isn't revealed
                    if ((*sItr)->GetTeamOwnership() == g_MetaMan.m_Players[metaPlayer].GetTeam() && (*sItr)->GetAutoDesigned())
                        (*sItr)->ApplyAIPlan(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer());
                }
            }
        }

        // Start animating these defensive lines appearing, one after another
        m_AnimMetaPlayer = Players::PlayerOne;
        m_AnimActionLine = 0;
        m_AnimActionLineChange = true;
        m_ActionMeterDrawOverride = false;
        m_AnimTimer1.Reset();
        ChangeAnimMode(PAUSEANIM);
    }

    // If a new line, set it up
    if (m_AnimActionLineChange)
    {
        // New Player also? If so, set up his lines
        if (m_AnimActionLine == 0)
        {
            // Hide all but the meters of all lines
            for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[m_AnimMetaPlayer].begin(); slItr != m_ActionSiteLines[m_AnimMetaPlayer].end(); ++slItr)
            {
                (*slItr).m_OnlyFirstSegments = 1;
                (*slItr).m_OnlyLastSegments = -1;
            }
        }

        // Did the players change? If so, pause
//        if (m_AnimMetaPlayer != m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_Player && !m_AnimTimer1.IsPastRealMS(1000))
//            ChangeAnimMode(PAUSEANIM);

        // Find the next green defense line of this player
        while (m_AnimActionLine < m_ActionSiteLines[m_AnimMetaPlayer].size() && m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_Color != c_GUIColorGreen)
            m_AnimActionLine++;

        // Regular site line, start with bar bracket and line appearing toward the site
// Blinking is too tedious to watch
//        ChangeAnimMode(BLINKMETER);
        ChangeAnimMode(LINECONNECTFW);

        m_AnimActionLineChange = false;
    }

    // Animate defense spending sitelines into view, and then count them away one by one
//    for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[m_AnimMetaPlayer].begin(); slItr != m_ActionSiteLines[m_AnimMetaPlayer].end(); ++slItr)
    if (!m_ActionSiteLines[m_AnimMetaPlayer].empty() && m_AnimActionLine < m_ActionSiteLines[m_AnimMetaPlayer].size())
    {
        if (m_AnimMode == BLINKMETER)
        {
            if (NewAnimMode())
            {
                m_AnimTimer1.Reset();
                // Show the meter
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments = -1;
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = 1;
            }
            // Start blinking
            if (!m_AnimTimer1.IsPastRealMS(1500))
            {
//                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(m_AnimTimer1.AlternateReal(150));
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = m_AnimTimer1.AlternateReal(150) ? 1 : 0;
            }
            else
            {
                // Leave the label showing
                m_apPlayerBarLabel[m_AnimMetaPlayer]->SetVisible(true);
                // Start connecting the meter to the circle!
                ChangeAnimMode(LINECONNECTFW);
                m_AnimSegment = 1;
            }
        }
        else if (m_AnimMode == LINECONNECTFW)
        {
            if (NewAnimMode())
            {
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = 1;
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments = -1;
                m_LineConnected = false;
                // Hide the site name over the site loc at first
                UpdateSiteNameLabel(false);
                m_AnimSegment = 1;
                m_AnimTimer1.Reset();
            }

            // If line not yet connected, keep revealing segments
            if (!m_LineConnected)
            {
                if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
                {
                    m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments++;
                    m_AnimTimer1.Reset();
                }
            }
            // Oh! Line now completed, pause for a while before continuing to shrinking the meter away
            else if (!m_AnimTimer1.IsPastRealMS(500))
            {
                // Show the site name over the site loc while we're waiting
                UpdateSiteNameLabel(true, m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_SiteName, m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_PlanetPoint);
            }
            // Done waiting, proceed to shrink the meter
            else
            {
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = -1;
                // Start shrinking the meter, showing that the money is being spent on the base
                ChangeAnimMode(SHRINKMETER);
            }
        }
        // Shrink each meter from its current value to 0
        else if (m_AnimMode == SHRINKMETER)
        {
            if (NewAnimMode())
            {
                m_AnimTimer1.Reset();
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments = -1;
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = -1;
                // Save the total funds so we can make the proportional animation right
                m_AnimTotalFunds = g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds;
                // Get a handy pointer to the scene we're talking about
                m_pAnimScene = m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_pScene;
                RTEAssert(m_pAnimScene, "Couldn't find the scene that we're building the base on!");
                // Using the line target as the going-from point
                m_AnimFundsMax = m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_FundsAmount;
                // The calculated budget use is the target value we're shrinking to
                m_AnimFundsMin = m_AnimFundsMax - m_pAnimScene->CalcBuildBudgetUse(g_MetaMan.m_Players[m_AnimMetaPlayer].GetInGamePlayer(), &m_AnimBuildCount);
                // Show the negative change of funds, if any
                if ((m_AnimFundsMin - m_AnimFundsMax) < 0)
                    FundsChangeIndication(m_AnimMetaPlayer, m_AnimFundsMin - m_AnimFundsMax, Vector(m_apPlayerBarLabel[m_AnimMetaPlayer]->GetXPos() + m_apPlayerBarLabel[m_AnimMetaPlayer]->GetWidth(), m_apPlayerBarLabel[m_AnimMetaPlayer]->GetYPos()), 2000);
            }
            // Show the site name over the site loc
            UpdateSiteNameLabel(true, m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_SiteName, m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_PlanetPoint);
            // Shrink by certain rate to the target value
            if (m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_FundsAmount > m_AnimFundsMin)
            {
                // Make this animation match in duration with the funds change label that is falling
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_FundsAmount = EaseOut(m_AnimFundsMax, m_AnimFundsMin, m_AnimTimer1.GetElapsedRealTimeMS() / 2000);//(m_AnimFundsMax * 2));
                // Adjust the funds to show how much we subtracted this frame
                g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds = m_AnimTotalFunds - (m_AnimFundsMax - m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_FundsAmount);
                UpdatePlayerLineRatios(m_ActionSiteLines[m_AnimMetaPlayer], m_AnimMetaPlayer, false, g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds);
                m_AnimTimer2.Reset();
            }
            // Finished shrinking the meter to the target size, now start disconnecting it
            else
            {
                UpdateSiteNameLabel(false);
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_FundsAmount = m_AnimFundsMin;
                UpdatePlayerLineRatios(m_ActionSiteLines[m_AnimMetaPlayer], m_AnimMetaPlayer, false, g_MetaMan.m_Players[m_AnimMetaPlayer].m_Funds);

                ChangeAnimMode(LINEDISCONNECTFW);
            }
        }
        else if (m_AnimMode == LINEDISCONNECTFW)
        {
            if (NewAnimMode())
            {
                // Describe what happened over the site while the lines are disconnecting
                if (m_AnimBuildCount == 0)
                    PlayerTextIndication(m_AnimMetaPlayer, m_AnimFundsMax < 100 ? "Can't afford any!" : "Nothing to build!", m_PlanetCenter + m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_PlanetPoint, 1500);
                else
                {
                    char str[64];
                    std::snprintf(str, sizeof(str), m_AnimBuildCount == 1 ? "Built %d item" : "Built %d items", m_AnimBuildCount);
                    PlayerTextIndication(m_AnimMetaPlayer, str, m_PlanetCenter + m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_PlanetPoint, 2500);
                }
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyFirstSegments = -1;
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments = 6;
                m_LineConnected = false;
            }

            // If line not yet gone, keep removing segments
            if (m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments > 0)
            {
                if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
                {
                    m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments--;
                    m_AnimTimer1.Reset();
                }
            }
            // Oh! site line completely gone, remove it and continue to next line
            else
            {
// No need to do this; the budgets get set to 0 so they will diasappear next update of the bars
// DON'T DO THIS, we need it later to make sure everyhting got built!
//                RemoveSiteLine(m_ActionSiteLines[m_AnimMetaPlayer], m_AnimActionLine);
                // Dont' need to increment since the vector just shrunk due to removal
                m_ActionSiteLines[m_AnimMetaPlayer][m_AnimActionLine].m_OnlyLastSegments = 0;
                m_AnimActionLine++;
                m_AnimActionLineChange = true;
                ChangeAnimMode(BLINKMETER);
            }
        }
    }

    // If no more lines, DONE, continue to next player, and if no more players, continue to next phase of the round
    if (m_AnimActionLine >= m_ActionSiteLines[m_AnimMetaPlayer].size())
    {
        m_AnimActionLineChange = true;
        m_AnimActionLine = 0;

        if (++m_AnimMetaPlayer >= g_MetaMan.m_Players.size())
            m_ContinuePhase = true;
    }

    // Phase ENDING, make sure everything is set up to continue
    if (m_ContinuePhase)
    {
        m_AnimMetaPlayer = g_MetaMan.m_Players.size() - 1;

        // Make sure all fund labels and line ratios are good
        Scene *pScene = 0;
        for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
        {
            // Reset the funds to the full value before we started messing with animating them
            g_MetaMan.m_Players[metaPlayer].m_Funds = g_MetaMan.m_Players[metaPlayer].m_PhaseStartFunds;

            // Go through the sitelines and make sure all the things that need to be done are done before moving onto next phase
            for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[metaPlayer].begin(); slItr != m_ActionSiteLines[metaPlayer].end(); ++slItr)
            {
                // APPLY all the defense budget allocations to actually building, but not before subtracting their values from the pre funds
                if ((*slItr).m_Color == c_GUIColorGreen)
                {
                    if ((*slItr).m_pScene)
                    {
                        // Get a non-const pointer to the scene
                        pScene = const_cast<Scene *>((*slItr).m_pScene);
                        // Acutally do the spending on these places that have this player's brain and has funds budgeted for them
                        // THIS IS WHAT GETS STUFF BUILT
                        // NOTE THE CAREFUL MAPPING BETWEEN METAPLAYERS AND IN-GAME PLAYERS
                        int player = g_MetaMan.m_Players[metaPlayer].GetInGamePlayer();
                        if (pScene->GetResidentBrain(player) && pScene->GetBuildBudget(player) > 0)
                            g_MetaMan.m_Players[metaPlayer].m_Funds -= pScene->ApplyBuildBudget(player);
                    }
                }
                // Reset all non-defensive lines' segment animations so we can see them
                else
                {
                    (*slItr).m_OnlyFirstSegments = -1;
                    (*slItr).m_OnlyLastSegments = -1;
                }
            }

            // Make all scene build budgets set to 0; they will be re-set to their previous turns' budget ratios on the next player turns
            for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
            {
                // Only mess with Scenes we can see and that are owned by this player's team
                if ((*sItr)->IsRevealed() && (*sItr)->GetTeamOwnership() == g_MetaMan.m_Players[metaPlayer].GetTeam())
                    (*sItr)->SetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), 0);
            }
            // Refresh the aciton lines one last time before moving on, which will CLEAN OUT all defensive action lines!
            UpdatePlayerActionLines(metaPlayer);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupOffensives
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up the Activities that represent all the offensive actions of
//                  the teams this round.

void MetagameGUI::SetupOffensives()
{
    // Clear out the old ones, if any
    g_MetaMan.ClearActivities();

    // Go through all players' offensive actions and create the unique activities
    string targetName;
    float offensiveBudget;
    bool playerDone = false;
    int team = Activity::NoTeam;
    int offensiveCount = 0;
    for (int metaPlayer = Players::PlayerOne; metaPlayer < g_MetaMan.m_Players.size(); ++metaPlayer)
    {
        playerDone = false;
        team = g_MetaMan.m_Players[metaPlayer].GetTeam();
        // If we have a selected offensive target, find its scene in the metagame
        targetName = g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName();
        offensiveBudget = g_MetaMan.m_Players[metaPlayer].GetOffensiveBudget();
        if (!targetName.empty() && offensiveBudget > 0)
        {
            for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); !playerDone && sItr != g_MetaMan.m_Scenes.end(); ++sItr)
            {
                if ((*sItr)->IsRevealed() && (*sItr)->GetPresetName() == targetName)
                {
                    // Now check that we haven't already created an offensive action at this site
                    for (vector<GAScripted *>::iterator aItr = g_MetaMan.m_RoundOffensives.begin(); aItr != g_MetaMan.m_RoundOffensives.end(); ++aItr)
                    {
                        if ((*aItr)->GetSceneName() == targetName)
                        {
                            // Ok, activity at this site already exists, so add this player to the attacking forces
                            (*aItr)->AddPlayer(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), g_MetaMan.m_Players[metaPlayer].IsHuman(), team, offensiveBudget, &(g_MetaMan.GetTeamIcon(team)));
                            // Move onto next player
                            playerDone = true;
                        }
                    }

                    // Create new offensive Activity
                    if (!playerDone)
                    {
                        // Set up the MetaFight activity
                        GAScripted *pOffensive = new GAScripted;
                        pOffensive->Create("Base.rte/Activities/MetaFight.lua", "MetaFight");
                        char str[64];
                        std::snprintf(str, sizeof(str), "R%dA%d", g_MetaMan.m_CurrentRound + 1, offensiveCount);
                        pOffensive->SetPresetName(g_MetaMan.GetGameName() + str);
                        // Associate the name of the scene with where this thing is supposed to take place
                        pOffensive->SetSceneName(targetName);
                        // Gotto deact all players since by default there is one in slot 1
                        pOffensive->ClearPlayers();
						// Set difficulty
						pOffensive->SetDifficulty(g_MetaMan.m_Difficulty);
						// Set AI skill levels
						for (int t = Activity::TeamOne; t < Activity::MaxTeamCount; t++)
							pOffensive->SetTeamAISkill(t, g_MetaMan.m_TeamAISkill[t]);

                        // Attacker
                        pOffensive->AddPlayer(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer(), g_MetaMan.m_Players[metaPlayer].IsHuman(), team, offensiveBudget, &(g_MetaMan.GetTeamIcon(team)));

                        // Unless exploring an unclaimed spot, there's going to be defenders
                        if ((*sItr)->GetTeamOwnership() != Activity::NoTeam)
                        {
                            // Go through all players and add the ones of the defending team, based on who has resident brains here
                            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
                            {
                                // Got to remember to translate from metagame player index into the in-game player index
// TODO: Remove this requirement to have a brain resident to play? error-prone and not so fun for co-op player on sme team if they can't all play
//                                if (g_MetaMan.m_Players[mp].GetTeam() == (*sItr)->GetTeamOwnership())
                                if ((*sItr)->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                                {
                                    pOffensive->AddPlayer(g_MetaMan.m_Players[mp].GetInGamePlayer(), g_MetaMan.m_Players[mp].IsHuman(), g_MetaMan.m_Players[mp].GetTeam(), g_MetaMan.GetRemainingFundsOfPlayer(mp, *sItr, false, false), &(g_MetaMan.GetTeamIcon(g_MetaMan.m_Players[mp].GetTeam())));
                                }
                            }
                        }

                        // Add the new Activity to the queue of offensive events that should happen this round!
                        g_MetaMan.m_RoundOffensives.push_back(pOffensive);
                        // Done with this guy's offensive action
                        playerDone = true;
                        // Count this new offensive Activity
                        ++offensiveCount;
                    }
                }
            }
        }
    }
/*
    // Check all created Offensive Activites that none only contain AI MetaPlayer:s, because they have to be handled differently and not actually played
    for (vector<GAScripted *>::iterator aItr = g_MetaMan.m_RoundOffensives.begin(); aItr != g_MetaMan.m_RoundOffensives.end(); ++aItr)
    {
        // No hooomins??
        if ((*aItr)->GetHumanCount() < 1)
        {
            
        }
    }
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateOffensives
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the offensive actions animation

void MetagameGUI::UpdateOffensives()
{
    char str[256];

    ///////////////////////////////////
    // First do setup of all the activities we're going to play in order
    if (g_MetaMan.m_StateChanged)
    {
        // Generate all offensive activites, based on the players' offensive moves
        // But ONLY do this if it doesn't appear we are continuing a loaded game that has already completed battles!!
        if (g_MetaMan.m_CurrentOffensive == 0)
            SetupOffensives();

        // Hide all income lines so they don't flash once on phase start
        for (vector<SiteLine>::iterator slItr = m_IncomeSiteLines.begin(); slItr != m_IncomeSiteLines.end(); ++slItr)
            (*slItr).m_OnlyFirstSegments = (*slItr).m_OnlyLastSegments = 0;

        // Set the battle info display to the start of its animation.
        UpdatePreBattleAttackers(0);
        UpdatePreBattleDefenders(0);

        // Start processing the first activity
        m_AnimActivityChange = true;
        // Turn off the post battle review flag
        m_PostBattleReview = false;
// Don't clear this! We may have loaded a game with a battle completed so we don't want to start on 0 again
//        g_MetaMan.m_CurrentOffensive = 0;
    }

    // No Offensives this round? then skip all this business
    if (g_MetaMan.m_RoundOffensives.empty() || g_MetaMan.m_CurrentOffensive < 0 || g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size())
    {
        m_ContinuePhase = true;
// TODO: Make sure that quitting out here is ok
        return;
    }

    // If we're still reviewing a previous battle and the player decided to skip to the next battle right away then do so and move directly to the next offensive
    if (m_PostBattleReview && !m_PreTurn)
    {
        // We're done with this activity
        m_AnimActivityChange = true;
        // Turn off the post battle review flag too
        m_PostBattleReview = false;
        // This finishes off the current battle we're reviewing, and moves onto the next
        if (!FinalizeOffensive())
            return;
    }

    ////////////////////////////////////////////////////
    // New Offensive, so set it up
    if (m_AnimActivityChange)
    {   
        // Show which mission we're on of all the offensive activities in queue
        std::snprintf(str, sizeof(str), "Battle %d of %d", (g_MetaMan.m_CurrentOffensive + 1), (int)(g_MetaMan.m_RoundOffensives.size()));
        m_pPhaseLabel->SetText(str);

        // Find the scene being attacked in this offensive Activity
        for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            if ((*sItr)->IsRevealed() && (*sItr)->GetPresetName() == g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetSceneName())
                m_pAnimScene = (*sItr);
        }
        RTEAssert(m_pAnimScene, "Couldn't find the Site that has been selected as attacked!");

        // It's owned by a team, so set up and show its defenders
        if (m_pAnimScene->GetTeamOwnership() != Activity::NoTeam)
            m_AnimDefenseTeam = m_pAnimScene->GetTeamOwnership();

        // Set up all the offensive and defensive lines for each player involved in this site's battle
        for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
        {
            // If this player is involved in this fight, show his lines etc
            if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer()))
            {
                // Save the fund levels FROM THE START of each battle so we can calculate the after state if players skip the animation
                g_MetaMan.m_Players[mp].m_PhaseStartFunds = g_MetaMan.m_Players[mp].m_Funds;

                // Rebuild all the action site lines for the player
                float meterStart = UpdatePlayerActionLines(mp);
                // Reset battle funds and other animation indicators
                m_aBattleFunds[mp] = 0;
                m_aBattleAttacker[mp] = false;
                m_aAnimDestroyed[mp] = false;
                m_aBrainIconPos[mp].Reset();

                // What is this player attacking?
                string targetName = g_MetaMan.m_Players[mp].GetOffensiveTargetName();
                float offensiveBudget = g_MetaMan.m_Players[mp].GetOffensiveBudget();
                // If attacking anything, see if it's the current site that's getting hit
                if (!targetName.empty() && offensiveBudget > 0)
                {
                    // Set the displayed battle funds for this player if it's attacking this site
                    if (m_pAnimScene->GetPresetName() == targetName)
                    {
                        m_aBattleFunds[mp] = offensiveBudget;
                        m_aBattleAttacker[mp] = true;
                    }
                }

                // If this player owns and has a resident brain at this site, add his defending action line of unallocated funds,
                // and also update the offensive activity itself to match teh defensive funds to what he has left after multiple completed offensives this turn
                if (m_pAnimScene->GetTeamOwnership() == g_MetaMan.m_Players[mp].GetTeam() && m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // Find out how much gold this player has left after all allocations and previous battles this turn
                    float remainingFunds = g_MetaMan.GetRemainingFundsOfPlayer(mp);
                    // UPDATE the contributed funds of the defending player to whatever gold he has left after all (if any) previous battles this turn!
                    bool updated = g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->UpdatePlayerFundsContribution(g_MetaMan.m_Players[mp].GetInGamePlayer(), remainingFunds);
                    // Add the unused build budget FOR THIS SCENE to the battle funds display - doesn't do anyhting now because build budgets are set to 0 in UpdateBaseBuilding
                    m_aBattleFunds[mp] += m_pAnimScene->GetBuildBudget(g_MetaMan.m_Players[mp].GetInGamePlayer());
                    // Add the unallocated funds meter, even if there's 0 left.. it shows why there's 0, and also the meter can grow after battle if the defending player digs up gold
                    if (remainingFunds >= 0)
                    {
                        // Tell the player why he has no defensive money... uh too early
//                        if (remainingFunds <= 0)
//                            PlayerTextIndication(mp, "No unallocated funds!", Vector(m_apPlayerBox[mp]->GetXPos(), m_apPlayerBox[mp]->GetYPos()), 1500);

                        m_ActionSiteLines[mp].push_back(SiteLine(mp, meterStart, remainingFunds / g_MetaMan.m_Players[mp].m_Funds, m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), m_pAnimScene->GetPresetName(), m_pAnimScene, c_GUIColorYellow, -1, -1, 60, 1.0f, g_MetaMan.IsActiveTeam(m_pAnimScene->GetTeamOwnership())));
                        // This will actually affect the line meter
                        m_ActionSiteLines[mp].back().m_FundsAmount = remainingFunds;
                        // Add to the battle funds display too
                        m_aBattleFunds[mp] += remainingFunds;
                        // Move the meter start position forward as necessary
                        meterStart += m_ActionSiteLines[mp].back().m_MeterAmount;
                    }
                }

                // Hide all action lines except for their bars
                for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                {
                    (*slItr).m_OnlyFirstSegments = 1;
                    (*slItr).m_OnlyLastSegments = -1;
                }

                // Update the ratios now that we've messed with them
                UpdatePlayerLineRatios(m_ActionSiteLines[mp], mp, false, g_MetaMan.m_Players[mp].m_Funds);
            }
            // If player not involved in the fight then hide all lines of this player
            {
                for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                {
                    (*slItr).m_OnlyFirstSegments = 0;
                    (*slItr).m_OnlyLastSegments = -1;
                }
            }
        }

        // Start out the brain travel animaitons hidden and at 0
        ResetBattleInfo();
        UpdatePreBattleAttackers(0);
        UpdatePreBattleDefenders(0);
        // Make the meters draw all the time
// This might not be such a great thing after all to show all the bars.. not needed anymore since we're not connecting to defensive budgets in offensive phases anymore
//        m_ActionMeterDrawOverride = true;

        // Show that we are waiting for the players to press the Continue button so we can start the Activity we're setting up here
        m_PreTurn = true;
        m_BattleToResume = false;
        // New battle, reset the post battle flags
        m_PostBattleReview = false;
        m_BattleCausedOwnershipChange = false;
        m_PreBattleTeamOwnership = Activity::NoTeam;

        ChangeAnimMode(TARGETZEROING);
        m_AnimActivityChange = false;
    }

    /////////////////////////////////////////////
    // Make a menacing target crosshairs zero in on the site we're playing this activity on
    if (m_AnimMode == TARGETZEROING)
    {
        if (NewAnimMode())
        {
            m_SiteAttackTarget.m_CenterPos = m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset();
            m_SiteAttackTarget.m_AnimProgress = 0;
            m_SiteAttackTarget.m_Color = c_GUIColorRed;
            m_AnimModeDuration = 600;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }
        m_SiteAttackTarget.m_AnimProgress = EaseOut(0.0, 1.0, m_AnimTimer1.GetElapsedRealTimeMS() / m_AnimModeDuration);

        // BAM - show the name of the site and start showing who's attacking it
        if (m_SiteAttackTarget.m_AnimProgress == 1.0)
        {
//        // After a while, go on to show the attacker action site lines
//        if (m_AnimTimer1.IsPastRealMS(1200))
//        {
            ChangeAnimMode(LINECONNECTFW);
        }
    }

    /////////////////////////////////////////////
    // Show who's attacking this target; connect the attack budget lines (while still zeroed) and animate the brains going there from their respective pools
    if (m_AnimMode == LINECONNECTFW)
    {
        if (NewAnimMode())
        {
            // Make sure all offensive action lines are set up for this phase
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {
                // Find all players that are active during this battle
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // If this player is attacking, indicate that we've got a brain in transit.. this just changes the display, not the actual brain pool count yet
                    if (m_pAnimScene->GetPresetName() == g_MetaMan.m_Players[mp].GetOffensiveTargetName())
                        g_MetaMan.m_Players[mp].ChangeBrainsInTransit(1);

                    // Find their offensive funds site lines
                    for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                    {
                        // Offensive lines are red; set them up to have their meters visible
                        if ((*slItr).m_Color == c_GUIColorRed)
                        {
                           (*slItr).m_OnlyFirstSegments = 1;
                           (*slItr).m_OnlyLastSegments = -1;
                        }
                    }
                }
            }

            m_AnimSegment = 0;
            m_AnimModeDuration = 2000;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);

        // Keep revealing segments simultaneously from all attackers until they are all revealed
        if (m_AnimSegment < 15)
        {
            if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
            {
                for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
                {
                    // Only care if this player is attacking this site
                    if (m_pAnimScene->GetPresetName() == g_MetaMan.m_Players[mp].GetOffensiveTargetName() && g_MetaMan.m_Players[mp].GetOffensiveBudget() > 0)
//                    if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    {
                        // Find their offensive funds site lines
                        for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                        {
                            // Offensive lines are red; grow all of em
                            if ((*slItr).m_Color == c_GUIColorRed)
                                (*slItr).m_OnlyFirstSegments++;
                        }
                    }
                }

                m_AnimSegment++;
                m_AnimTimer1.Reset();
            }
        }

        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // Animate the brain label travel animations
        UpdatePreBattleAttackers(EaseInOut(0, 1.0, MIN(1.0, m_AnimTimer2.GetElapsedRealTimeMS() / m_AnimModeDuration)));
        UpdatePreBattleDefenders(0);

        // Just wait until the players hit the continue button..
        if (m_BattleToResume)
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Resume <" : "Resume");
        else
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Start! <" : "Start!");

        // Move onto growing the attack budget meters
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
            ChangeAnimMode(SHOWDEFENDERS);
    }

    /////////////////////////////////////////////
    // Show the defending brains for this battle
    if (m_AnimMode == SHOWDEFENDERS)
    {
        if (NewAnimMode())
        {
            // Make sure all defensive and unallocated budget action lines are set up for this phase
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {

            }

            m_AnimModeDuration = 500;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);

        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // Keep the brain label travel animations in one spot and their labels updated
        UpdatePreBattleAttackers(1.0);
        UpdatePreBattleDefenders(EaseOut(0, 1.0, MIN(1.0, m_AnimTimer2.GetElapsedRealTimeMS() / m_AnimModeDuration)));

        // Just wait until the players hit the continue button..
        if (m_BattleToResume)
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Resume <" : "Resume");
        else
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Start! <" : "Start!");

        // Move onto growing the attack budget meters
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
            ChangeAnimMode(LINECONNECTBW);
    }

    /////////////////////////////////////////////
    // Show the defending budget lines of the players who are defending this site
    if (m_AnimMode == LINECONNECTBW)
    {
        if (NewAnimMode())
        {
            // Make sure all defensive and unallocated budget action lines are set up for this phase
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {
                // Only care of this player is involved in this particular battle
                // Only care about defending players of this site
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer())&&
                    m_pAnimScene->GetTeamOwnership() == g_MetaMan.m_Players[mp].GetTeam() &&
                    m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // Find their defensive funds site lines
                    for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                    {
                        // Offensive lines are green; unallocated funds are white - attach all of them
                        if ((*slItr).m_Color != c_GUIColorRed)
                        {
                            (*slItr).m_OnlyFirstSegments = -1;
                            (*slItr).m_OnlyLastSegments = 0;
                        }
                    }
                }
            }

            m_AnimSegment = 0;
            m_AnimModeDuration = 1000;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);

        // Keep revealing segments simultaneously from all attackers until they are all revealed
        if (m_AnimSegment < 15)
        {
            if (m_AnimTimer1.GetElapsedRealTimeMS() > 150)
            {
                for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
                {
                    // Only care of this player is involved in this particular battle
                    if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    {
                        // Find their defensive-related funds site lines
                        for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                        {
                            // Offensive lines are red; grow the defensive ones relevant to this scene
                            if ((*slItr).m_Color != c_GUIColorRed && m_pAnimScene->GetPresetName() == (*slItr).m_SiteName &&
                                (m_pAnimScene->GetTeamOwnership() == g_MetaMan.m_Players[mp].GetTeam() && m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer())))
                                (*slItr).m_OnlyLastSegments++;
                        }
                    }
                }
                m_AnimSegment++;
                m_AnimTimer1.Reset();
            }
        }

        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // Keep the brain label travel animations in one spot and their labels updated
        UpdatePreBattleAttackers(1.0);
        UpdatePreBattleDefenders(1.0);

        // Just wait until the players hit the continue button..
        if (m_BattleToResume)
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Resume <" : "Resume");
        else
            m_apMetaButton[CONTINUE]->SetText(m_AnimTimer2.AlternateReal(333) ? "> Start! <" : "Start!");
    }

    //////////////////////////////////
    // POST BATTLE REVIEW

    /////////////////////////////////////////////
    // A Short pause for after players come out of a run simulation battle,
    // so they can orient themselves visually before battle review starts happening
    if (m_AnimMode == SHOWPOSTBATTLEPAUSE)
    {
        if (NewAnimMode())
        {
            // The length of the pause
            m_AnimModeDuration = 2000;

            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);
        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        UpdatePostBattleRetreaters(0);
        UpdatePostBattleResidents(0);

        // Give the players the option to skip the post battle review and go to next battle, or phase if there aren't any more battles
        m_apMetaButton[CONTINUE]->SetText("Next");

        // Move onto having the victorious brains go back inside the site and take it over
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
            ChangeAnimMode(SHOWPOSTBATTLEBALANCE);
    }

    /////////////////////////////////////////////
    // Show how much is being allocated to attacking this place
    if (m_AnimMode == SHOWPOSTBATTLEBALANCE)
    {
        if (NewAnimMode())
        {
            m_AnimModeDuration = 2000;

            // Make sure all offensive action-related lines are set up for this battle review animation
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {
                // Only the players of this battle
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // Find their site lines that are connected to the site
                    for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                    {
                        // Only lines that are connected are relevant
                        if ((*slItr).m_OnlyFirstSegments > 1 || (*slItr).m_OnlyLastSegments > 1)
                        {
                            // Re-set the funds amount we're going from - actually, not necessary
                            (*slItr).m_FundsAmount = g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsContribution(g_MetaMan.m_Players[mp].GetInGamePlayer());
                            // Set the target we're going toward
                            (*slItr).m_FundsTarget = g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsShare(g_MetaMan.m_Players[mp].GetInGamePlayer());
                            // Start the battle money display off at the right point too
                            m_aBattleFunds[mp] = (*slItr).m_FundsAmount;
                            // Display the change amount over/under the player bar
                            FundsChangeIndication(mp, (*slItr).m_FundsTarget - (*slItr).m_FundsAmount, Vector(m_apPlayerBarLabel[mp]->GetXPos() + m_apPlayerBarLabel[mp]->GetWidth(), m_apPlayerBarLabel[mp]->GetYPos()), m_AnimModeDuration);

                            // Update the ratios.. shouldn't do anyhitng, really
                            UpdatePlayerLineRatios(m_ActionSiteLines[mp], mp, false, g_MetaMan.m_Players[mp].m_Funds);
                        }
                    }
                }
            }

            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Find the players who are involved in this battle
        for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
        {
            // Only the players of this battle
            if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
            {
                // The brains who DID NOT MAKE IT - Show them blowing up at some random interval into the animation
                if (!m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()) &&
                    !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // If not yet blown up, then see if we should yet
                    if (!m_aAnimDestroyed[mp] && m_AnimTimer2.GetElapsedRealTimeMS() > (m_AnimModeDuration * 0.5F) && RandomNum() < 0.05F)
                    {
                        // Add circle explosion effect to where the brain icon used to be
                        m_SiteSwitchIndicators.push_back(SiteTarget(m_aBrainIconPos[mp], 0, SiteTarget::CIRCLEGROW, c_GUIColorRed));
                        // Switch the brain icon drawing off
                        m_aAnimDestroyed[mp] = true;
                    }
                }

                // Find their site lines that are connected to the site
                for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                {
                    // Only lines that are connected are relevant
                    if ((*slItr).m_OnlyFirstSegments > 1 || (*slItr).m_OnlyLastSegments > 1)
                    {
                        float startFunds = g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsContribution(g_MetaMan.m_Players[mp].GetInGamePlayer());

                        // Grow/shrink all the meters, showing how the funds changed during battle
                        if (m_AnimTimer1.GetElapsedRealTimeMS() < m_AnimModeDuration)
                        {
                            // Make this animation correlate in duration with the funds change label that rises
                            (*slItr).m_FundsAmount = EaseOut(startFunds, (*slItr).m_FundsTarget, m_AnimTimer1.GetElapsedRealTimeMS() / m_AnimModeDuration);
                            // Update the battle funds display; it will be shown in the brian action label
                            m_aBattleFunds[mp] = (*slItr).m_FundsAmount;
                            // Adjust the funds to show how much we subtracted this frame - this will be reset in the end anyway, it's just for show during this battle review animation
                            g_MetaMan.m_Players[mp].m_Funds = g_MetaMan.m_Players[mp].m_PhaseStartFunds - (startFunds - (*slItr).m_FundsAmount);
                            UpdatePlayerLineRatios(m_ActionSiteLines[mp], mp, false, g_MetaMan.m_Players[mp].m_Funds);
                        }
                        // Finished growing the meter to the target size
                        else
                        {
                            m_aBattleFunds[mp] = (*slItr).m_FundsAmount = (*slItr).m_FundsTarget;
                            g_MetaMan.m_Players[mp].m_Funds = g_MetaMan.m_Players[mp].m_PhaseStartFunds - (startFunds - (*slItr).m_FundsAmount);
                            UpdatePlayerLineRatios(m_ActionSiteLines[mp], mp, false, g_MetaMan.m_Players[mp].m_Funds);
                        }
                    }
                }
            }
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);
        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // Animate the brain label travel animations
        UpdatePostBattleRetreaters(0);
        UpdatePostBattleResidents(0);

        UpdatePlayerBars();
        // Give the players the option to skip the post battle review and go to next battle, or phase if there aren't any more battles
        m_apMetaButton[CONTINUE]->SetText("Next");

        // Move onto showing what happened to each brain
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
        {
            // Blow up any remaining brains who are doomed
            // Find the players who are involved in this battle
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {
                // Only the players of this battle who didn't evacuate
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()) &&
                    !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                {
                    // The brains who DID NOT MAKE IT - Show them blowing up
                    if (!m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    {
                        // If not yet blown up, then we should be
                        if (!m_aAnimDestroyed[mp])
                        {
                            // Add circle explosion effect to where the brain icon used to be
                            m_SiteSwitchIndicators.push_back(SiteTarget(m_aBrainIconPos[mp], 0, SiteTarget::CIRCLEGROW, c_GUIColorRed));
                            // Switch the brain icon drawing off
                            m_aAnimDestroyed[mp] = true;
                        }
                    }
                }
            }
            ChangeAnimMode(SHOWPOSTBATTLEBRAINS);
        }
    }

    /////////////////////////////////////////////
    // Show what happened to the brains after a battle
    if (m_AnimMode == SHOWPOSTBATTLEBRAINS)
    {
        if (NewAnimMode())
        {
            // Make sure all defensive and unallocated budget action lines are set up for this phase
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {

            }

            // The duration of this depends on whethere there are any evacuees to travel back
            m_AnimModeDuration = g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->AnyBrainWasEvacuated() ? 2000 : 500;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);
        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // The retreating brain label travel animations get updated to go back to their pools
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->AnyBrainWasEvacuated())
            UpdatePostBattleRetreaters(EaseInOut(0, 1.0, MIN(1.0, m_AnimTimer2.GetElapsedRealTimeMS() / m_AnimModeDuration)));
        else
            UpdatePostBattleRetreaters(1.0);
        UpdatePostBattleResidents(0);

        // Give the players the option to skip the post battle review and go to next battle, or phase if there aren't any more battles
        m_apMetaButton[CONTINUE]->SetText("Next");

        // Move onto having the victorious brains go back inside the site and take it over
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
        {
            // Change the display to show the evacuees transferring back to their brain pools
            if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->AnyBrainWasEvacuated())
            {
                // Find the players who are evacuated anything this battle
                for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
                {
                    // Only the players of this battle who evac'd their brain
                    if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    {
                        // Both an Attacker aborting an attack, and a Defender abandoning his site has the same effect here on the brian pool display
                        g_MetaMan.m_Players[mp].ChangeBrainsInTransit(-1);
                    }
                }
            }

            ChangeAnimMode(SHOWNEWRESIDENTS);
        }
    }

    /////////////////////////////////////////////
    // Show the victorious brains going back into the site and taking it over
    if (m_AnimMode == SHOWNEWRESIDENTS)
    {
        if (NewAnimMode())
        {
            // Make sure all defensive and unallocated budget action lines are set up for this phase
            for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
            {

            }

            m_AnimModeDuration = m_BattleCausedOwnershipChange ? 500 : 750;
            m_AnimTimer1.Reset();
            m_AnimTimer2.Reset();
        }

        // Keep the name on the attack site, keeping the name high so the contestants' info can fit in the quadrants around the site
        UpdateSiteNameLabel(true, m_pAnimScene->GetPresetName(), m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 1.8);
        // Animate the crosshairs subtly
        m_SiteAttackTarget.m_AnimProgress = 0.975 + 0.025 * cos(c_TwoPI * (float)((int)m_AnimTimer2.GetElapsedRealTimeMS() % 666) / 666.0f);

        // The brains going back into their lair are animated
        UpdatePostBattleRetreaters(1.0);
        // Tweak the animaiton slightly based on whether the site switched hands or not
        if (m_BattleCausedOwnershipChange)
            UpdatePostBattleResidents(EaseIn(0, 1.0, MIN(1.0, m_AnimTimer2.GetElapsedRealTimeMS() / m_AnimModeDuration)));
        else
            UpdatePostBattleResidents(EaseOut(0, 1.0, MIN(1.0, m_AnimTimer2.GetElapsedRealTimeMS() / m_AnimModeDuration)));

        // Find the players who are involved in this battle
        for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
        {
            // Only the players of this battle
            if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
            {
                // The guys who died - remove their icons completely
                if (!m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    m_apPlayerBrainTravelLabel[mp]->SetVisible(false);
            }
        }

        // Give the players the option to skip the post battle review and go to next battle, or phase if there aren't any more battles
        m_apMetaButton[CONTINUE]->SetText("Next");

        // Move onto the next offensive in line, if any!
        if (m_AnimTimer2.GetElapsedRealTimeMS() > m_AnimModeDuration)
        {
            // Quit if there's no more offensives after the one we just finalized
            if (!FinalizeOffensive())
                return;
        }
    }


    /////////////////////////////////////////////
    // If the player have chosen to start the attack activity by hitting the continue button, then do eeeet
    if (!m_PreTurn)
    {
        // Make sure this is set back
        m_apMetaButton[CONTINUE]->SetText("Start!");
        UpdateSiteNameLabel(false);

        // Max out the offensive lines in case the player started the battle before the lines were fully animated as connected
        for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
        {
            // Only care of this player is involved in this particular battle
            if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive(g_MetaMan.m_Players[mp].GetInGamePlayer()))
            {
                for (vector<SiteLine>::iterator slItr = m_ActionSiteLines[mp].begin(); slItr != m_ActionSiteLines[mp].end(); ++slItr)
                {
                    // Only show lines that are going to the site we're battling on
                    if ((*slItr).m_SiteName == m_pAnimScene->GetPresetName())
                    {
                        // Find their offensive funds site lines and set the offensive budget back to where it is supposed to end up
                        if ((*slItr).m_Color == c_GUIColorRed)
                            (*slItr).m_OnlyFirstSegments = 15;
                        // Defensive site
                        else if (m_pAnimScene->GetTeamOwnership() == g_MetaMan.m_Players[mp].GetTeam() && m_pAnimScene->GetResidentBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                            (*slItr).m_OnlyLastSegments = 15;
                    }
                    // Hide all other lines
                    else
                    {
                        (*slItr).m_OnlyFirstSegments = 0;
                        (*slItr).m_OnlyLastSegments = 0;
                    }
                }
            }
        }

        // Resuming ongoing battle
        if (m_BattleToResume)
        {
            m_BattleToResume = false;
            m_ActivityResumed = true;
            g_GUISound.ExitMenuSound()->Play();
        }
        // Starting the next battle
        else
        {
            // Clear the battle info of the last one
// No need here; it is done in FinalizeOffensive, and it actually screw up the positions of the losing brain labels, which should stay put until after rewview is over
//            ResetBattleInfo();

            if (g_MetaMan.m_CurrentOffensive < g_MetaMan.m_RoundOffensives.size())
            {
                // Temporarly save the previous ownership state of the scene, so it can be held and displayed until the dramatic change is shown in battle review
                m_PreBattleTeamOwnership = m_pAnimScene->GetTeamOwnership();

                // No human players in this one, or only one team going here? Then skip the whole sim and just show the result of the site
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetHumanCount() < 1 || g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetTeamCount() == 1)
                {
                    // AUTOMATIC BATTLE RESOLUTION
                    // If the automatic resolution caused a site change, show it clearly with animated crosshairs
                    m_BattleCausedOwnershipChange = AutoResolveOffensive(g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive], const_cast<Scene *>(m_pAnimScene));

                    // Move onto showing what happened in the battle before we move on to next battle
                    m_PreTurn = true;
                    m_PostBattleReview = true;
                    ChangeAnimMode(SHOWPOSTBATTLEBALANCE);
                }
                // START SIM with activity that has at least one human player involved and more than one team total
                else
                {
                    m_pPlayingScene = const_cast<Scene *>(m_pAnimScene);
                    g_SceneMan.SetSceneToLoad(m_pPlayingScene);
                    g_ActivityMan.SetStartActivity(dynamic_cast<Activity *>(g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->Clone()));
                    m_ActivityRestarted = true;
                }
            }
            // Whoops, we have no further activities left to start, so just continue to next round phase (shouldn't happen; this is handled in CompletedActivity)
            else
            {
                // Turn off drawing the aciton meters draw all the time
                m_ActionMeterDrawOverride = false;
                m_ContinuePhase = true;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FinalizeOffensive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Finishes one battle in the UpdateOffensives and moves onto the next.

bool MetagameGUI::FinalizeOffensive()
{
    // No Offensives this round? then skip all this business
    if (g_MetaMan.m_RoundOffensives.empty() || g_MetaMan.m_CurrentOffensive < 0 || g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size() || !m_pAnimScene)
    {
        m_ContinuePhase = true;
        return false;
    }

    // Deduct the original funds contribution of each player - less any unused funds of the team, taking original player contribution ratios into account
    for (int mp = Players::PlayerOne; mp < g_MetaMan.m_Players.size(); ++mp)
    {
        // Only the players who were battling this offensive
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain(g_MetaMan.m_Players[mp].GetInGamePlayer()))
        {
            // Re-set the funds level to where it was at the start of this offensive (NOT at the start of the phase, actually)
            g_MetaMan.m_Players[mp].m_Funds = g_MetaMan.m_Players[mp].m_PhaseStartFunds;
            // Deduct the original contribution to team funds
            g_MetaMan.m_Players[mp].m_Funds -= g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsContribution(g_MetaMan.m_Players[mp].GetInGamePlayer());
            // Add back whatever his share of whatever his team has left in the fight at its end
            g_MetaMan.m_Players[mp].m_Funds += g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsShare(g_MetaMan.m_Players[mp].GetInGamePlayer());
            // IF This guy was ATTACKING this turn, adjust his attack budget to match what just happened in the battle
            // so in case he is defending in a upcoming battle this turn, his avaialbe funds will be accurately calculated
            if (g_MetaMan.m_Players[mp].GetOffensiveTargetName() == m_pAnimScene->GetPresetName())
            {
                g_MetaMan.m_Players[mp].SetOffensiveBudget(g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->GetPlayerFundsShare(g_MetaMan.m_Players[mp].GetInGamePlayer()));
//                g_MetaMan.m_Players[mp].SetOffensiveTargetName("");

                // The only condition where this attacking player does NOT get a brain deducted from pool, is when he aborts the attack with an evacuation and the brain goes back to pool!
                if (!g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    g_MetaMan.m_Players[mp].ChangeBrainPoolCount(-1);
            }
            // Defending player
            else
            {
                // If this player evacuated his brain, WHILE DEFENDING, then add it back to his pool
                if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated(g_MetaMan.m_Players[mp].GetInGamePlayer()))
                    g_MetaMan.m_Players[mp].ChangeBrainPoolCount(1);
            }
            // Update the ratios of the meter now that the funds have changed
            UpdatePlayerLineRatios(m_ActionSiteLines[mp], mp, false, g_MetaMan.m_Players[mp].m_Funds);
            // Clear out the brain transit display either way; nothing should be out flying now anyway
            g_MetaMan.m_Players[mp].SetBrainsInTransit(0);
        }
    }

    // If the battle caused ownership change, then show it with a cool indication
    if (m_BattleCausedOwnershipChange)
        m_SiteSwitchIndicators.push_back(SiteTarget(m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset(), 0, m_pAnimScene->GetTeamOwnership() != Activity::NoTeam ? SiteTarget::SQUAREGROW : SiteTarget::CIRCLEGROW, c_GUIColorRed, m_AnimTimer2.GetElapsedRealTimeMS()));

    // Clear the battle info of the last one
    ResetBattleInfo();
    // Clear the site name label
    UpdateSiteNameLabel(false);
    // Done reviewing the battle
    m_PostBattleReview = false;
    // We're done with this activity
    m_AnimActivityChange = true;

    // Update the count to point to the next (now current) offensive
    g_MetaMan.m_CurrentOffensive++;

    // AUTO-SAVE THE GAME AFTER EACH PLAYED BATTLE
    SaveGame(AUTOSAVENAME, METASAVEPATH + string(AUTOSAVENAME) + ".ini", false);

    // If we're out of offensives, then move onto the next phase
    if (g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size())
    {
        m_ContinuePhase = true;
        // Quit out of here, there's no more battles to fight!
        return false;
    }
    // Indicate that there's at least one more battle lined up now
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetBattleInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides and resets all battle info labels and panels

void MetagameGUI::ResetBattleInfo()
{
    int mp = 0;
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Init and hide everything initially
        if (!m_apPlayerTeamActionBox[mp]->GetDrawImage())
        {
            // Set the flag icons on the floating player bars
            m_apPlayerTeamActionBox[mp]->SetDrawType(GUICollectionBox::Image);
            m_apPlayerTeamActionBox[mp]->SetDrawImage(new AllegroBitmap(g_MetaMan.m_TeamIcons[(*mpItr).GetTeam()].GetBitmaps32()[0]));
        }
        // Hide everything initially
        m_apPlayerTeamActionBox[mp]->SetVisible(false);
        m_apPlayerTeamActionBox[mp]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
        m_apPlayerBrainTravelLabel[mp]->SetVisible(false);
        m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);

        ++mp;
    }

    // Done reviewing the battle too
    m_PostBattleReview = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateBattleQuads
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates which player get placed in which quad around a fought-over
//                  site.

void MetagameGUI::UpdateBattleQuads(Vector targetPos)
{
    // Start with a clean slate
    for (int q = Players::PlayerOne; q < Players::MaxPlayerCount; ++q)
        m_aQuadTakenBy[q] = Players::NoPlayer;

    // Go through all players, assigning the quads depending on how the player bars are positioned in relation to each other
    int mp = 0;
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Select which quadrant makes most sense for this player, based on his floating bar's position relative to the site
        int initialQuad = 0;
        if (m_apPlayerBox[mp]->GetXPos() < targetPos.m_X)
            initialQuad = m_apPlayerBox[mp]->GetYPos() < targetPos.m_Y ? 0 : 1;
        else
            initialQuad = m_apPlayerBox[mp]->GetYPos() < targetPos.m_Y ? 3 : 2;

        // If the initial selection is taken, just find the next available one
        int quadIndex = initialQuad;
        while (m_aQuadTakenBy[quadIndex] != Players::NoPlayer)
        {
            quadIndex++;
            // Loop around
            if (quadIndex >= 4)
                quadIndex = 0;
            // If we've looped completely around, somehting's wrong! There should always be at least one quad available
            if (quadIndex == initialQuad)
                break;
        }
        // Mark this spot as taken by this player
        m_aQuadTakenBy[quadIndex] = mp;
        // Advance the metaplayer index to sync with the iterator
        mp++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePreBattleAttackers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the animation and display of the info for the current
//                  offensive battle being next in line for this round.

void MetagameGUI::UpdatePreBattleAttackers(float progress)
{
    // Sanity check that we have any offensive battle activity and scene to display around
    if (g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size() ||
        !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive] ||
        !m_pAnimScene)
        return;

    int mp = 0;
    char str[256];
    int quadIndex = Players::NoPlayer;
    // The screen coordinate position of the current battle site
    Vector siteScreenPos = m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset();
    // Where the brain label starts from
    Vector brainLabelStartPos;
    // Where it ends up, aligned properly next to the site, in the proper quadrant
    Vector brainLabelSitePos;
    // The distance the brain label should travel between the two
    Vector brainLabelTravel;
    // The actual total distance it will travel, when going along each cardinal axis at a time
    float brainTravelLength;
    // The animation progress point at which travel switches from one axis to the other
    float travelSwitchRatio;
    // The actual current position of the brain label at the current specified animaiton progress point
    Vector brainTravelPos;

    // Update the quad assignments for all players
    UpdateBattleQuads(siteScreenPos);
    
    // Go through all attacking players for this activity
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Player active and ATTACKING in current battle, so display his team flag and place it according to the animation progress
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive((*mpItr).GetInGamePlayer()) &&
            !m_pAnimScene->GetResidentBrain((*mpItr).GetInGamePlayer()))
        {
            // Show the active players' team flag icons around the site if the brains have arrived
            m_apPlayerTeamActionBox[mp]->SetVisible(progress >= 1.0);
            // Show the traveling brain if we're not at 0
            m_apPlayerBrainTravelLabel[mp]->SetVisible(progress > 0);

            // Figure out which quad this player is assigned to
            for (int q = Players::PlayerOne; q < Players::MaxPlayerCount; ++q)
            {
                if (m_aQuadTakenBy[q] == mp)
                {
                    quadIndex = q;
                    break;
                }
            }

            // Write the brain label, with info if applicable for the current progress of animation
            if (progress < 1.0)
                std::snprintf(str, sizeof(str), "%c", -48);
            // When at site destination, take into account the side the brain icon needs to be on
            else
            {
                if (quadIndex <= 1)
                    std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], m_aBattleAttacker[mp] ? -46 : -47, -48);
                else
                    std::snprintf(str, sizeof(str), "%c%c %c %.0f oz", -48, m_aBattleAttacker[mp] ? -46 : -47, -58, m_aBattleFunds[mp]);
            }
            m_apPlayerBrainTravelLabel[mp]->SetText(str);
            m_apPlayerBrainTravelLabel[mp]->SetToolTip("The specific brain that is being sent in to attack this place, and the funds he has been budgeted to do so with.");

            // Figure out start and ending positions for the brain label's travels and the team flag badge
            // Upper left quadrant 
            if (quadIndex == 0)
            {
                // Team flag position
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() - (m_apPlayerBrainTravelLabel[mp]->GetWidth() / 2) - 30, m_apBrainPoolLabel[mp]->GetYPos() - 5);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), -m_apPlayerBrainTravelLabel[mp]->GetHeight());
            }
            // Lower left quadrant
            else if (quadIndex == 1)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() - (m_apPlayerBrainTravelLabel[mp]->GetWidth() / 2) - 30, m_apBrainPoolLabel[mp]->GetYPos() - 5);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), 0);
            }
            // Lower right quadrant
            else if (quadIndex == 2)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() + 8, m_apBrainPoolLabel[mp]->GetYPos() - 5);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, 0);
            }
            // Upper right quadrant
            else if (quadIndex == 3)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() + 8, m_apBrainPoolLabel[mp]->GetYPos() - 5);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, -m_apPlayerBrainTravelLabel[mp]->GetHeight());
            }

            // Now place the travelling brain label at the appropriate spot based on the animation progress
            brainLabelTravel = brainLabelSitePos - brainLabelStartPos;
            // The raw distance the label needs to travel when going down one axis and then the other
            brainTravelLength = fabs(brainLabelTravel.m_X) + fabs(brainLabelTravel.m_Y);
            // Figure out where along the progression that brain travel switches from one axis to the other
            travelSwitchRatio = fabs(brainLabelTravel.m_Y) / brainTravelLength;

            // If animation progress is less than where the switch happens, handle that
            if (progress <= travelSwitchRatio)
            {
                // No progress in X
                brainTravelPos.m_X = brainLabelStartPos.m_X;
                // Some progress in Y
                brainTravelPos.m_Y = brainLabelStartPos.m_Y + (brainLabelTravel.m_Y * (progress / travelSwitchRatio));
            }
            // Progress is past the switch
            else if (progress < 1.0)
            {
                // Some progress in X
                brainTravelPos.m_X = brainLabelStartPos.m_X + (brainLabelTravel.m_X * ((progress - travelSwitchRatio) / (1.0 - travelSwitchRatio)));
                // FULL progress in Y
                brainTravelPos.m_Y = brainLabelSitePos.m_Y;
            }
            // Reached destination
            else
                brainTravelPos = brainLabelSitePos;

            // Now actually move the label
            m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(brainTravelPos.m_X, brainTravelPos.m_Y);
        }
        // Advance the metaplayer index to sync with the iterator
        mp++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePreBattleDefenders
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the animation and display of the info for the current
//                  offensive battle's defenders being next in line for this round.

void MetagameGUI::UpdatePreBattleDefenders(float progress)
{
    // Sanity check that we have any offensive battle activity and scene to display around
    if (g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size() ||
        !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive] ||
        !m_pAnimScene)
        return;

    int mp = 0;
    char str[256];
    int quadIndex = Players::NoPlayer;
    // The screen coordinate position of the current battle site
    Vector siteScreenPos = m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset();
    // Where the brain label starts from
    Vector brainLabelStartPos;
    // Where it ends up, aligned properly next to the site, in the proper quadrant
    Vector brainLabelSitePos;
    // The distance the brain label should travel between the two
    Vector brainLabelTravel;
    // The actual total distance it will travel, when going along each cardinal axis at a time
    //float brainTravelLength;
    // The animation progress point at which travel switches from one axis to the other
    //float travelSwitchRatio;
    // The actual current position of the brain label at the current specified animaiton progress point
    Vector brainTravelPos;

    // Update the quad assignments for all players
    UpdateBattleQuads(siteScreenPos);
    
    // Go through all players for this activity
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Player active and DEFENDING in current battle, so display his team flag and place it according to the animation progress
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerActive((*mpItr).GetInGamePlayer()) &&
            m_pAnimScene->GetResidentBrain((*mpItr).GetInGamePlayer()))
        {
            // Show the active players' team flag icons around the site if the brains have arrived
            m_apPlayerTeamActionBox[mp]->SetVisible(progress >= 1.0);
            // Show the traveling brain if we're not at 0
            m_apPlayerBrainTravelLabel[mp]->SetVisible(progress > 0);

            // Figure out which quad this player is assigned to
            for (int q = Players::PlayerOne; q < Players::MaxPlayerCount; ++q)
            {
                if (m_aQuadTakenBy[q] == mp)
                {
                    quadIndex = q;
                    break;
                }
            }

            // Write the brain label, with info if applicable for the current progress of animation
            if (progress < 1.0)
                std::snprintf(str, sizeof(str), "%c", -48);
            // When at site destination, take into account the side the brain icon needs to be on
            else
            {
                if (quadIndex <= 1)
                    std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], m_aBattleAttacker[mp] ? -46 : -47, -48);
                else
                    std::snprintf(str, sizeof(str), "%c%c %c %.0f oz", -48, m_aBattleAttacker[mp] ? -46 : -47, -58, m_aBattleFunds[mp]);
            }
            m_apPlayerBrainTravelLabel[mp]->SetText(str);
            m_apPlayerBrainTravelLabel[mp]->SetToolTip("The resident brain that is defending this site from attack, and the unallocated funds of its player that he gets to use (beyond the defense investments already made here).");

            // Figure out start and ending positions for the brain label's travels and the team flag badge
            // Upper left quadrant 
            if (quadIndex == 0)
            {
                // Team flag position
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(siteScreenPos.m_X - m_apPlayerBrainTravelLabel[mp]->GetWidth() + 7, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), -m_apPlayerBrainTravelLabel[mp]->GetHeight());
            }
            // Lower left quadrant
            else if (quadIndex == 1)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(siteScreenPos.m_X - m_apPlayerBrainTravelLabel[mp]->GetWidth() + 7, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), 0);
            }
            // Lower right quadrant
            else if (quadIndex == 2)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(siteScreenPos.m_X - 5, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, 0);
            }
            // Upper right quadrant
            else if (quadIndex == 3)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label start position, taking into account its text alignment
                brainLabelStartPos.SetXY(siteScreenPos.m_X - 5, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, -m_apPlayerBrainTravelLabel[mp]->GetHeight());
            }

            // Now place the travelling brain label at the appropriate spot based on the animation progress
            brainLabelTravel = brainLabelSitePos - brainLabelStartPos;

            // How far have we come
            if (progress <= 0)
                brainTravelPos = brainLabelStartPos;
            // Somewhere in between
            else if (progress < 1.0)
                brainTravelPos = brainLabelStartPos + brainLabelTravel * progress;
            // Reached destination
            else
                brainTravelPos = brainLabelSitePos;

            // Now actually move the label
            m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(brainTravelPos.m_X, brainTravelPos.m_Y);
        }
        // Advance the metaplayer index to sync with the iterator
        mp++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePostBattleRetreaters
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the animation and display of the info for the current
//                  offensive battle's retreating brains going back to their pools

void MetagameGUI::UpdatePostBattleRetreaters(float progress)
{
    // Sanity check that we have any offensive battle activity and scene to display around
    if (g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size() ||
        !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive] ||
        !m_pAnimScene)
        return;

    int mp = 0;
    char str[256];
    int quadIndex = Players::NoPlayer;
    // The screen coordinate position of the current battle site
    Vector siteScreenPos = m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset();
    // Where it ends up, aligned properly next to the site, in the proper quadrant
    Vector brainLabelSitePos;
    // Where the brain label ends at, back at the pool
    Vector brainLabelEndPos;
    // The distance the brain label should travel between the two
    Vector brainLabelTravel;
    // The actual total distance it will travel, when going along each cardinal axis at a time
    float brainTravelLength;
    // The animation progress point at which travel switches from one axis to the other
    float travelSwitchRatio;
    // The actual current position of the brain label at the current specified animaiton progress point
    Vector brainTravelPos;

    // Update the quad assignments for all players
    UpdateBattleQuads(siteScreenPos);
    
    // Go through all players for this activity
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Player active and EVACUATION/RETREATING in current battle, so display his team flag and place it according to the animation progress
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain((*mpItr).GetInGamePlayer()) &&
            g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated((*mpItr).GetInGamePlayer()))
        {
            // Show the active players' team flag icons around the site if the brains have arrived
            m_apPlayerTeamActionBox[mp]->SetVisible(progress <= 0);
            // Show the traveling brain if we're not at destination yet
            m_apPlayerBrainTravelLabel[mp]->SetVisible(progress < 1.0);

            // Figure out which quad this player is assigned to
            for (int q = Players::PlayerOne; q < Players::MaxPlayerCount; ++q)
            {
                if (m_aQuadTakenBy[q] == mp)
                {
                    quadIndex = q;
                    break;
                }
            }

            // Write the brain label, with info if applicable for the current progress of animation
            if (progress > 0)
                std::snprintf(str, sizeof(str), "%c", -48);
            // When at site destination, take into account the side the brain icon needs to be on
            else
            {
                if (quadIndex <= 1)
                    std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], m_aBattleAttacker[mp] ? -46 : -47, -48);
                else
                    std::snprintf(str, sizeof(str), "%c%c %c %.0f oz", -48, m_aBattleAttacker[mp] ? -46 : -47, -58, m_aBattleFunds[mp]);
            }
            m_apPlayerBrainTravelLabel[mp]->SetText(str);
            m_apPlayerBrainTravelLabel[mp]->SetToolTip("The specific brain that is being sent in to attack this place, and the funds he has been budgeted to do so with.");

            // Figure out start and ending positions for the brain label's travels and the team flag badge
            // Upper left quadrant 
            if (quadIndex == 0)
            {
                // Team flag position
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                // Brain label start position, taking into account its text alignment
                brainLabelEndPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() - (m_apPlayerBrainTravelLabel[mp]->GetWidth() / 2) - 30, m_apBrainPoolLabel[mp]->GetYPos() - 5);
            }
            // Lower left quadrant
            else if (quadIndex == 1)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), 0);
                // Brain label start position, taking into account its text alignment
                brainLabelEndPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() - (m_apPlayerBrainTravelLabel[mp]->GetWidth() / 2) - 30, m_apBrainPoolLabel[mp]->GetYPos() - 5);
            }
            // Lower right quadrant
            else if (quadIndex == 2)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, 0);
                // Brain label start position, taking into account its text alignment
                brainLabelEndPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() + 8, m_apBrainPoolLabel[mp]->GetYPos() - 5);
            }
            // Upper right quadrant
            else if (quadIndex == 3)
            {
                m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                // Brain label text alignment, depending on the quadrant
                m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                // Brain label end/site position, taking into account its text alignment
                brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                // Brain label start position, taking into account its text alignment
                brainLabelEndPos.SetXY(m_apBrainPoolLabel[mp]->GetXPos() + 8, m_apBrainPoolLabel[mp]->GetYPos() - 5);
            }

            // Now place the travelling brain label at the appropriate spot based on the animation progress
            brainLabelTravel = brainLabelEndPos - brainLabelSitePos;
            // The raw distance the label needs to travel when going down one axis and then the other
            brainTravelLength = fabs(brainLabelTravel.m_X) + fabs(brainLabelTravel.m_Y);
            // Figure out where along the progression that brain travel switches from one axis to the other
            travelSwitchRatio = fabs(brainLabelTravel.m_X) / brainTravelLength;

            // If animation progress is less than where the switch happens, handle that
            if (progress <= travelSwitchRatio)
            {

                // Some progress in X
                brainTravelPos.m_X = brainLabelSitePos.m_X + (brainLabelTravel.m_X * (progress / travelSwitchRatio));
                // No progress in Y
                brainTravelPos.m_Y = brainLabelSitePos.m_Y;
            }
            // Progress is past the switch
            else if (progress < 1.0)
            {
                // FULL progress in X
                brainTravelPos.m_X = brainLabelEndPos.m_X;
                // Some progress in Y
                brainTravelPos.m_Y = brainLabelSitePos.m_Y + (brainLabelTravel.m_Y * ((progress - travelSwitchRatio) / (1.0 - travelSwitchRatio)));
            }
            // Reached destination
            else
                brainTravelPos = brainLabelEndPos;

            // Now actually move the label
            m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(brainTravelPos.m_X, brainTravelPos.m_Y);
        }
        // Advance the metaplayer index to sync with the iterator
        mp++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePostBattleResidents
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the animation and display of the info for the current done
//                  offensive battle's winning brains going back into the site.

void MetagameGUI::UpdatePostBattleResidents(float progress)
{
    // Sanity check that we have any offensive battle activity and scene to display around
    if (g_MetaMan.m_CurrentOffensive >= g_MetaMan.m_RoundOffensives.size() ||
        !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive] ||
        !m_pAnimScene)
        return;

    int mp = 0;
    char str[256];
    int quadIndex = Players::NoPlayer;
    // The screen coordinate position of the current battle site
    Vector siteScreenPos = m_PlanetCenter + m_pAnimScene->GetLocation() + m_pAnimScene->GetLocationOffset();
    // Where the brain label ends at
    Vector brainLabelEndPos;
    // Where it ends up, aligned properly next to the site, in the proper quadrant
    Vector brainLabelSitePos;
    // The distance the brain label should travel between the two
    Vector brainLabelTravel;
    // The actual total distance it will travel, when going along each cardinal axis at a time
    //float brainTravelLength;
    // The animation progress point at which travel switches from one axis to the other
    //float travelSwitchRatio;
    // The actual current position of the brain label at the current specified animaiton progress point
    Vector brainTravelPos;

    // Update the quad assignments for all players
    UpdateBattleQuads(siteScreenPos);
    
    // Go through all players for this activity
    for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
    {
        // Player active in the past battle, so display his team flag and place it according to the animation progress
        // Also player who didn't evacuate - they are handled by UpdatePostBattleRetreaters
        if (g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->PlayerHadBrain((*mpItr).GetInGamePlayer()) &&
            !g_MetaMan.m_RoundOffensives[g_MetaMan.m_CurrentOffensive]->BrainWasEvacuated((*mpItr).GetInGamePlayer()))
        {
            // Figure out which quad this player is assigned to
            for (int q = Players::PlayerOne; q < Players::MaxPlayerCount; ++q)
            {
                if (m_aQuadTakenBy[q] == mp)
                {
                    quadIndex = q;
                    break;
                }
            }

            // The brains who DID NOT MAKE IT - DEAD and did not evac
            if (!m_pAnimScene->GetResidentBrain((*mpItr).GetInGamePlayer()))
            {
                // Hide the losers after the residents start moving in
                m_apPlayerTeamActionBox[mp]->SetVisible(progress <= 0);
                m_apPlayerBrainTravelLabel[mp]->SetVisible(progress <= 0);

                if (progress <= 0)
                {
                    // Death mask
                    if (progress > 0)
                        std::snprintf(str, sizeof(str), "%c", -26);
                    // Brain with line blinking over it and the funds still showing
                    else
                    {
                        if (quadIndex <= 1)
                        {
                            if (m_aAnimDestroyed[mp])
                                std::snprintf(str, sizeof(str), "%c %.0f oz     ", -58, m_aBattleFunds[mp]);
                            else
                                std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], m_aBattleAttacker[mp] ? -46 : -47, -26);
//                            std::snprintf(str, sizeof(str), "%c %.0f oz %c", -58, m_aBattleFunds[mp], m_AnimTimer2.AlternateReal(200) ? -39 : -26);
                        }
                        else
                        {
                            if (m_aAnimDestroyed[mp])
                                std::snprintf(str, sizeof(str), "     %c %.0f oz", -58, m_aBattleFunds[mp]);
                            else
                                std::snprintf(str, sizeof(str), "%c%c %c %.0f oz",  m_aAnimDestroyed[mp] ? ' ' : -26, m_aBattleAttacker[mp] ? -46 : -47, -58, m_aBattleFunds[mp]);
//                            std::snprintf(str, sizeof(str), "%c %c %.0f oz", m_AnimTimer2.AlternateReal(200) ? -39 : -26, -58, m_aBattleFunds[mp]);
                        }
                    }
                    m_apPlayerBrainTravelLabel[mp]->SetText(str);
                    m_apPlayerBrainTravelLabel[mp]->SetToolTip("The specific brain that is being sent in to attack this place, and the funds he has been budgeted to do so with.");
                }

                // Figure out the position of the dead brain label
                // Upper left quadrant 
                if (quadIndex == 0)
                {
                    // Team flag position
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                    // Brain label start position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                    // Where the brain icon is now exactly, so we can accurately pin the blow up animation on it
                    m_aBrainIconPos[mp] = brainLabelSitePos + Vector(m_apPlayerBrainTravelLabel[mp]->GetWidth() - 6, m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2);
                }
                // Lower left quadrant
                else if (quadIndex == 1)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                    // Brain label start position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), 0);
                    // Where the brain icon is now exactly, so we can accurately pin the blow up animation on it
                    m_aBrainIconPos[mp] = brainLabelSitePos + Vector(m_apPlayerBrainTravelLabel[mp]->GetWidth() - 6, m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2);
                }
                // Lower right quadrant
                else if (quadIndex == 2)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                    // Brain label start position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, 0);
                    // Where the brain icon is now exactly, so we can accurately pin the blow up animation on it
                    m_aBrainIconPos[mp] = brainLabelSitePos + Vector(6, m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2);
                }
                // Upper right quadrant
                else if (quadIndex == 3)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                    // Brain label start position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                    // Where the brain icon is now exactly, so we can accurately pin the blow up animation on it
                    m_aBrainIconPos[mp] = brainLabelSitePos + Vector(6, m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2);
                }

                // Now actually place the label
                m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(brainLabelSitePos.m_X, brainLabelSitePos.m_Y);
            }
            // The SURVIVORS who aren't evacuating either - ie they won and are staying
            else
            {
                // Show the active players' team flag icons around the site if the brains have arrived
                m_apPlayerTeamActionBox[mp]->SetVisible(progress <= 0);
                // Show the traveling brain if we're not at 1.0
                m_apPlayerBrainTravelLabel[mp]->SetVisible(progress < 1.0);

                // Write the brain label, with info if applicable for the current progress of animation
                if (progress > 0)
                    std::snprintf(str, sizeof(str), "%c", -48);
                // When at site start position, take into account the side the brain icon needs to be on
                else
                {
                    if (quadIndex <= 1)
                        std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], m_aBattleAttacker[mp] ? -46 : -47, -48);
//                        std::snprintf(str, sizeof(str), "%c %.0f oz %c%c", -58, m_aBattleFunds[mp], -47, -48);
                    else
                        std::snprintf(str, sizeof(str), "%c%c %c %.0f oz", -48, m_aBattleAttacker[mp] ? -46 : -47, -58, m_aBattleFunds[mp]);
//                        std::snprintf(str, sizeof(str), "%c%c %c %.0f oz", -48, -47, -58, m_aBattleFunds[mp]);
                }
                m_apPlayerBrainTravelLabel[mp]->SetText(str);
                m_apPlayerBrainTravelLabel[mp]->SetToolTip("The new resident brain that has won this site and is settling in here now.");

                // Figure out start and ending positions for the brain label's travels and the team flag badge
                // Upper left quadrant 
                if (quadIndex == 0)
                {
                    // Team flag position
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                    // Brain label end/site position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                    // Brain label start position, taking into account its text alignment
                    brainLabelEndPos.SetXY(siteScreenPos.m_X - m_apPlayerBrainTravelLabel[mp]->GetWidth() + 7, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                }
                // Lower left quadrant
                else if (quadIndex == 1)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X - m_apPlayerTeamActionBox[mp]->GetWidth() - BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Right);
                    // Brain label end/site position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(-BATTLEPAD - (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) - BRAINOVERLAP - m_apPlayerBrainTravelLabel[mp]->GetWidth(), 0);
                    // Brain label start position, taking into account its text alignment
                    brainLabelEndPos.SetXY(siteScreenPos.m_X - m_apPlayerBrainTravelLabel[mp]->GetWidth() + 7, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                }
                // Lower right quadrant
                else if (quadIndex == 2)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y + BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                    // Brain label end/site position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, 0);
                    // Brain label start position, taking into account its text alignment
                    brainLabelEndPos.SetXY(siteScreenPos.m_X - 5, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                }
                // Upper right quadrant
                else if (quadIndex == 3)
                {
                    m_apPlayerTeamActionBox[mp]->SetPositionAbs(siteScreenPos.m_X + BATTLEPAD, siteScreenPos.m_Y - m_apPlayerTeamActionBox[mp]->GetHeight() - BATTLEPAD);
                    // Brain label text alignment, depending on the quadrant
                    m_apPlayerBrainTravelLabel[mp]->SetHAlignment(GUIFont::Left);
                    // Brain label end/site position, taking into account its text alignment
                    brainLabelSitePos = siteScreenPos + Vector(BATTLEPAD + (m_apPlayerTeamActionBox[mp]->GetWidth() / 2) + BRAINOVERLAP, -m_apPlayerBrainTravelLabel[mp]->GetHeight());
                    // Brain label start position, taking into account its text alignment
                    brainLabelEndPos.SetXY(siteScreenPos.m_X - 5, siteScreenPos.m_Y - (m_apPlayerBrainTravelLabel[mp]->GetHeight() / 2) + 2);
                }

                // Now place the travelling brain label at the appropriate spot based on the animation progress
                brainLabelTravel = brainLabelEndPos - brainLabelSitePos;

                // How far have we come
                if (progress <= 0)
                    brainTravelPos = brainLabelSitePos;
                // Somewhere in between
                else if (progress < 1.0)
                    brainTravelPos = brainLabelSitePos + brainLabelTravel * progress;
                // Reached destination
                else
                    brainTravelPos = brainLabelEndPos;

                // Now actually move the label
                m_apPlayerBrainTravelLabel[mp]->SetPositionAbs(brainTravelPos.m_X, brainTravelPos.m_Y);
            }
        }
        // Advance the metaplayer index to sync with the iterator
        mp++;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerActionLines
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the action lines as per what the player has chosen to do
//                  during the current turn so far.

float MetagameGUI::UpdatePlayerActionLines(int metaPlayer)//, bool addUnallocated)
{
    // Make sure we're in a player turn phase
    if (metaPlayer < Players::PlayerOne || metaPlayer >= g_MetaMan.m_Players.size())
        return 0;

    // First clean out all Action sitelines for this player
    m_ActionSiteLines[metaPlayer].clear();

    // Get the total funds we have to work with
    float totalFunds = g_MetaMan.m_Players[metaPlayer].GetFunds();

    // Loop through the scenes owned by that player, setting up the building budget site line for each
    const Scene *pScene = 0;
    int channelHeight = 60;
    float meterStart = 0;
    while (pScene = g_MetaMan.GetNextSceneOfPlayer(metaPlayer, pScene))
    {
        // Add line for scenes which are owned and whose build budgets have been set to something
        if (pScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer) && std::floor(pScene->GetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer())) > 0)
        {
            m_ActionSiteLines[metaPlayer].push_back(SiteLine(metaPlayer, meterStart, pScene->GetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer()) / totalFunds, pScene->GetLocation() + pScene->GetLocationOffset(), pScene->GetPresetName(), pScene, c_GUIColorGreen, -1, -1, channelHeight, 1.0f, g_MetaMan.IsActiveTeam(pScene->GetTeamOwnership())));
            m_ActionSiteLines[metaPlayer].back().m_FundsAmount = pScene->GetBuildBudget(g_MetaMan.m_Players[metaPlayer].GetInGamePlayer());
            meterStart += m_ActionSiteLines[metaPlayer].back().m_MeterAmount;
            channelHeight += 10;
        }
    }

    // If we have a selected offensive target, find it and create its line as well
    string targetName = g_MetaMan.m_Players[metaPlayer].GetOffensiveTargetName();
    float offensiveBudget = g_MetaMan.m_Players[metaPlayer].GetOffensiveBudget();
    if (!targetName.empty() && offensiveBudget > 0)
    {
        for (vector<Scene *>::iterator sItr = g_MetaMan.m_Scenes.begin(); sItr != g_MetaMan.m_Scenes.end(); ++sItr)
        {
            if ((*sItr)->IsRevealed() && (*sItr)->GetPresetName() == targetName)
            {
                m_ActionSiteLines[metaPlayer].push_back(SiteLine(metaPlayer, meterStart, offensiveBudget / totalFunds, (*sItr)->GetLocation() + (*sItr)->GetLocationOffset(), (*sItr)->GetPresetName(), (*sItr), c_GUIColorRed, -1, -1, channelHeight, 1.0f, g_MetaMan.IsActiveTeam((*sItr)->GetTeamOwnership())));
                m_ActionSiteLines[metaPlayer].back().m_FundsAmount = offensiveBudget;
                meterStart += m_ActionSiteLines[metaPlayer].back().m_MeterAmount;
            }
        }
    }

    return meterStart;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateScenesBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the scene selection box.

void MetagameGUI::UpdateScenesBox(bool sceneChanged)
{
    // Always show the info box if something is selected
    if (m_pSelectedScene && !g_MetaMan.IsSuspended())
    {
        char str[256];
        // Set the close button to have that fancy X
        std::snprintf(str, sizeof(str), "%c", -36);
        m_pSceneCloseButton->SetText(string(str));

        // Set the currently selected scene's texts
        m_pSceneInfoPopup->SetVisible(true);
        m_pSceneNameLabel->SetText(m_pSelectedScene->GetPresetName());

        // Show which team owns this place, and how many resident brains there are here of that team
        if (m_pSelectedScene->GetTeamOwnership() > Activity::NoTeam && m_pSelectedScene->GetTeamOwnership() < Activity::MaxTeamCount)
        {
            // Set the team flag icon
            m_pSceneOwnerTeam->SetVisible(true);
            m_pSceneOwnerTeam->SetDrawType(GUICollectionBox::Image);
            m_pSceneOwnerTeam->SetDrawImage(new AllegroBitmap(g_MetaMan.m_TeamIcons[m_pSelectedScene->GetTeamOwnership()].GetBitmaps32()[0]));
            // Show how many resident brains there are hanging out here
            std::snprintf(str, sizeof(str), "%c", -26);
            string brainString = "";
            int brainCount = m_pSelectedScene->GetResidentBrainCount();
            for (int i = 0; i < brainCount; ++i)
                brainString += str;
            m_pSceneResidentsLabel->SetText(brainString);
            m_pSceneResidentsLabel->SetVisible(true);
        }
        // Noone owns this place, so hide this ownership stuff
        else
        {
            m_pSceneOwnerTeam->SetVisible(false);
            m_pSceneResidentsLabel->SetVisible(false);
            m_pSceneResidentsLabel->SetText("");
        }

        // Write the description, and add the total defense investment in this place so far as a lil stat
        std::snprintf(str, sizeof(str), "Total base investments here: %doz", (int)floorf(m_pSelectedScene->GetTotalInvestment()));
        m_pSceneInfoLabel->SetText(m_pSelectedScene->GetDescription() + "\n" + string(str));
        // Adjust the height of the text box and container so it fits the text to display
        int newHeight = m_pSceneInfoLabel->ResizeHeightToFit();
        m_pSceneInfoPopup->Resize(m_pSceneInfoPopup->GetWidth(), newHeight + 96);

        // If during a player's round phase, show the budget slider and edit button
        if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
        {
            int metaPlayer = g_MetaMan.m_GameState - MetaMan::PLAYER1TURN;
            int team = g_MetaMan.m_Players[metaPlayer].GetTeam();

            // Resize the collection box to fit the extra controls
            m_pSceneInfoPopup->Resize(m_pSceneInfoPopup->GetWidth(), newHeight + 96);
            m_pSceneBudgetLabel->SetVisible(true);
            m_pSceneBudgetSlider->SetVisible(true);
            m_pSceneBudgetBar->SetVisible(true);

            // Set up the slider limit bar
            bool sceneOwnedByPlayer = m_pSelectedScene->GetTeamOwnership() == g_MetaMan.GetTeamOfPlayer(metaPlayer);
            int blockedWidth = std::floor((m_pSceneBudgetSlider->GetWidth() - 4) * g_MetaMan.GetBudgetedRatioOfPlayer(metaPlayer, m_pSelectedScene, sceneOwnedByPlayer));

            if (blockedWidth > 0)
            {
                m_pSceneBudgetBar->SetVisible(true);
                m_pSceneBudgetBar->SetPositionAbs(m_pSceneBudgetSlider->GetXPos() - 2 + m_pSceneBudgetSlider->GetWidth() - blockedWidth, m_pSceneBudgetSlider->GetYPos());
                m_pSceneBudgetBar->SetSize(blockedWidth, m_pSceneBudgetSlider->GetHeight());
            }
            else
                m_pSceneBudgetBar->SetVisible(false);

            // Make sure the slider can't go over the max allowed, as blocked by already budgeted funds
            int maxVal = 100 - g_MetaMan.GetBudgetedRatioOfPlayer(metaPlayer, m_pSelectedScene, sceneOwnedByPlayer) * 100;
            if (m_pSceneBudgetSlider->GetValue() > maxVal)
            {
// TODO: Play some sound, blink the bar etc too
                m_pSceneBudgetSlider->SetValue(maxVal);
            }

            // If owned by this player, then set up base building controls
            if (sceneOwnedByPlayer)
            {
                // Set the budget label as per the slider
                int budget = floorf(((float)m_pSceneBudgetSlider->GetValue() / 100.0f) * g_MetaMan.m_Players[metaPlayer].GetFunds());
                std::snprintf(str, sizeof(str), "Build Budget: %d oz", budget);
                m_pSceneBudgetLabel->SetText(str);
                m_apMetaButton[SCANNOW]->SetVisible(false);
                m_apMetaButton[SCANLATER]->SetVisible(false);
                m_pScanInfoLabel->SetVisible(false);
                m_apMetaButton[SCENEACTION]->SetVisible(false);
                m_apMetaButton[DESIGNBASE]->SetVisible(true);
                m_apMetaButton[DESIGNBASE]->SetText("Design Base");
                m_pSceneBudgetLabel->SetToolTip("Sets how much of your total funds will be budgeted toward building base defense blueprints on this site.");
                m_pSceneBudgetSlider->SetToolTip("Sets how much of your total funds will be budgeted toward building base defense blueprints on this site.");
                m_pAutoDesignCheckbox->SetVisible(true);
                m_pAutoDesignCheckbox->SetCheck(m_pSelectedScene->GetAutoDesigned());
            }
            // Not owned by this player's team, so set up the scanning controls
            else
            {
                // Only update this if the scene has changed in any way - otherwise UI changes done elsewhere might be overridden
                if (sceneChanged)
                {
                    // Set the budget label as per the slider
                    int budget = std::floor(((float)m_pSceneBudgetSlider->GetValue() / 100.0f) * g_MetaMan.m_Players[metaPlayer].GetFunds());
                    // Set the appropriate action message, depending on whether this is enemy owned, or merely unexplored
                    if (g_MetaMan.IsActiveTeam(m_pSelectedScene->GetTeamOwnership()))
                    {
                        std::snprintf(str, sizeof(str), "Attack Budget: %d oz", budget);
                        m_pSceneBudgetLabel->SetToolTip("Sets how much of your total funds will be budgeted toward exploring this site. Any gold that isn't used in the attack will return to your account afterward, but will also be tied up and can't be used for defense if someone else attacks any of your bases during the same turn. You can only attack one site per turn!");
                        m_pSceneBudgetSlider->SetToolTip("Sets how much of your total funds will be budgeted toward exploring this site. Any gold that isn't used in the attack will return to your account afterward, but will also be tied up and can't be used for defense if someone else attacks any of your bases during the same turn. You can only attack one site per turn!");
                    }
                    else
                    {
                        std::snprintf(str, sizeof(str), "Expedition Budget: %d oz", budget);
                        m_pSceneBudgetLabel->SetToolTip("Sets how much of your total funds will be budgeted toward attacking this site. Any gold that isn't used in the attack will return to your account afterward, but will also be tied up and can't be used for defense if someone else attacks any of your bases during the same turn. You can only explore one site per turn!");
                        m_pSceneBudgetSlider->SetToolTip("Sets how much of your total funds will be budgeted toward attacking this site. Any gold that isn't used in the attack will return to your account afterward, but will also be tied up and can't be used for defense if someone else attacks any of your bases during the same turn. You can only explore one site per turn!");
                    }
                    m_pSceneBudgetLabel->SetText(str);

                    // A Scan is already scheduled for this scene
                    if (m_pSelectedScene->IsScanScheduled(team))
                    {
                        // Hide buttons and show feedback message that scan has already been scheduled
                        m_apMetaButton[SCENEACTION]->SetVisible(false);
                        m_apMetaButton[DESIGNBASE]->SetVisible(false);
                        m_apMetaButton[SCANNOW]->SetVisible(false);
                        m_apMetaButton[SCANLATER]->SetVisible(false);
                        m_pScanInfoLabel->SetVisible(true);
                        m_pScanInfoLabel->SetText("- Orbital scan scheduled -");
                    }
                    // Check to make sure we're even able to afford a new scan at all
                    else if (g_MetaMan.GetRemainingFundsOfPlayer(metaPlayer, 0, false, false) < SCANCOST)
                    {
                        // Hide buttons and show feedback message that scan has already been scheduled
                        m_apMetaButton[SCENEACTION]->SetVisible(false);
                        m_apMetaButton[DESIGNBASE]->SetVisible(false);
                        m_apMetaButton[SCANNOW]->SetVisible(false);
                        m_apMetaButton[SCANLATER]->SetVisible(false);
                        m_pScanInfoLabel->SetVisible(true);
                        std::snprintf(str, sizeof(str), "%d", SCANCOST);
                        m_pScanInfoLabel->SetText("Need " + string(str) + " oz left to Scan!");
                    }
                    // Site can be scheduled to be scanned
                    else
                    {
                        // Show the scan button and hide the label
                        m_apMetaButton[SCANNOW]->SetVisible(false);
                        m_apMetaButton[SCANLATER]->SetVisible(false);
                        m_pScanInfoLabel->SetVisible(false);
                        m_apMetaButton[DESIGNBASE]->SetVisible(false);
                        m_apMetaButton[SCENEACTION]->SetVisible(true);
                        std::snprintf(str, sizeof(str), "%d", SCANCOST);
                        m_apMetaButton[SCENEACTION]->SetText("Scan Site (" + string(str) + " oz)");
                        m_apMetaButton[SCENEACTION]->SetToolTip("Performs an orbital scan of this site, which will show everything that is on the surface, but will not be able to penetrate far into the ground.");
                    }
                    // Hide the auto check box
                    m_pAutoDesignCheckbox->SetVisible(false);
                }

                // If the player doesn't have any brains left to deploy, then disable these attack controls instead
                if (g_MetaMan.m_Players[metaPlayer].GetBrainPoolCount() <= 0)
                {
                    m_pSceneBudgetLabel->SetText("- NO BRAINS TO DEPLOY -");
                    m_pSceneBudgetLabel->SetToolTip("Since you do not have any more brains in your brain pool available for deployment, you can't attack this or any other site. Defend the sites you do own and hope you'll keep enough of them to win the game!");
                    m_pSceneBudgetBar->SetToolTip("Since you do not have any more brains in your brain pool available for deployment, you can't attack this or any other site. Defend the sites you do own and hope you'll keep enough of them to win the game!");
                    // Make the blockage bar be all over the place
                    m_pSceneBudgetBar->SetVisible(true);
                    m_pSceneBudgetBar->SetPositionAbs(m_pSceneBudgetSlider->GetXPos() - 2, m_pSceneBudgetSlider->GetYPos());
                    m_pSceneBudgetBar->SetSize(std::floor((m_pSceneBudgetSlider->GetWidth() + 4)), m_pSceneBudgetSlider->GetHeight());
                    m_pSceneBudgetSlider->SetVisible(false);
                }
            }
        }
        // Just showing scene info
        else
        {
            // Resize the collection box to not show the player-relevant controls
            m_pSceneInfoPopup->Resize(m_pSceneInfoPopup->GetWidth(), newHeight + 34);
            m_pSceneBudgetLabel->SetVisible(false);
            m_pSceneBudgetSlider->SetVisible(false);
            m_pSceneBudgetBar->SetVisible(false);
            m_apMetaButton[SCENEACTION]->SetVisible(false);
            m_apMetaButton[DESIGNBASE]->SetVisible(false);
            m_apMetaButton[SCANNOW]->SetVisible(false);
            m_apMetaButton[SCANLATER]->SetVisible(false);
            m_pAutoDesignCheckbox->SetVisible(false);
            m_pScanInfoLabel->SetVisible(false);
        }

        // Make sure the box doesn't go entirely outside of the screen
        KeepBoxOnScreen(m_pSceneInfoPopup);
    }
    else
    {
        m_pSceneInfoPopup->SetVisible(false);
        m_pSceneNameLabel->SetText("");
        m_pSceneInfoLabel->SetText("");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateAISkillSliders
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates AI skill sliders and labels for all players.

void MetagameGUI::UpdateAISkillSliders(int player)
{
	if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
	{
		m_apPlayerAISkillLabel[player]->SetText(Activity::GetAISkillString(m_apPlayerAISkillSlider[player]->GetValue()));

		for (int otherPlayer = Players::PlayerOne; otherPlayer < Players::MaxPlayerCount; otherPlayer++)
		{
			if (otherPlayer != player && m_apPlayerTeamSelect[player]->GetSelectedIndex() == m_apPlayerTeamSelect[otherPlayer]->GetSelectedIndex())
			{
				m_apPlayerAISkillSlider[otherPlayer]->SetValue(m_apPlayerAISkillSlider[player]->GetValue());
				m_apPlayerAISkillLabel[otherPlayer]->SetText(Activity::GetAISkillString(m_apPlayerAISkillSlider[otherPlayer]->GetValue()));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateGameSizeLabels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the game size label of the new game dialog

void MetagameGUI::UpdateGameSizeLabels()
{
    // How many players do we have set to go
    int playerCount = 0;
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
        if (m_apPlayerControlButton[player]->GetText() != "None")
            ++playerCount;

	// How many scenes the game should end up with, according to the specified game size.
	// Note that it will never be all or none of all the available scenes!
// TODO: Hook these constants up to settings!!
	// How many scenes are there total
	const int totalCount = g_MetaMan.TotalScenePresets();
	const int minCount = std::clamp((playerCount * 3 / 2), 3, totalCount);
	m_pSizeSlider->SetMinimum(minCount);
	m_pSizeSlider->SetMaximum(std::max(totalCount * 7 / 10, minCount));
	m_pSizeSlider->SetValueResolution(1);

    char str[256];
    std::snprintf(str, sizeof(str), "Game Size: %d/%d sites", m_pSizeSlider->GetValue(), totalCount);
    m_pSizeLabel->SetText(str);

    // How much starting gold does the slider yield
    int startGold = STARTGOLDMIN + ((STARTGOLDMAX - STARTGOLDMIN) * (float)m_pGoldSlider->GetValue() / 100.0);
    std::snprintf(str, sizeof(str), "Starting Gold: %c %d oz", -58, startGold);
    m_pGoldLabel->SetText(str);

    // Set the length label also according to the game length slider
    int brainCount = m_pLengthSlider->GetValue();
    brainCount = MAX(brainCount, 1);
    std::snprintf(str, sizeof(str), "Game Length: %c%c%d starting brains", -48, -36, brainCount);
    m_pLengthLabel->SetText(str);

    if (m_pDifficultySlider->GetValue() < Activity::CakeDifficulty)
        m_pDifficultyLabel->SetText("Difficulty: Cake");
    else if (m_pDifficultySlider->GetValue() < Activity::EasyDifficulty)
        m_pDifficultyLabel->SetText("Difficulty: Easy");
    else if (m_pDifficultySlider->GetValue() < Activity::MediumDifficulty)
        m_pDifficultyLabel->SetText("Difficulty: Medium");
    else if (m_pDifficultySlider->GetValue() < Activity::HardDifficulty)
        m_pDifficultyLabel->SetText("Difficulty: Hard");
    else if (m_pDifficultySlider->GetValue() < Activity::NutsDifficulty)
        m_pDifficultyLabel->SetText("Difficulty: Nuts");
    else
        m_pDifficultyLabel->SetText("Difficulty: Nuts!");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerSetup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the player setup controls of the new game dialog

void MetagameGUI::UpdatePlayerSetup()
{
    int humanPlayers = 0;
    int totalPlayers = 0;
    const Icon *pTeamIcon = 0;
    list<const Icon *> teamList;
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        if (m_apPlayerControlButton[player]->GetText() == "None")
        {
            m_apPlayerTeamSelect[player]->SetVisible(false);
            m_apPlayerTechSelect[player]->SetVisible(false);
            m_apPlayerHandicap[player]->SetVisible(false);
            m_apPlayerNameBox[player]->SetVisible(false);
			m_apPlayerAISkillSlider[player]->SetVisible(false);
			m_apPlayerAISkillLabel[player]->SetVisible(false);
            continue;
        }
        else
        {
            totalPlayers++;
            if (m_apPlayerControlButton[player]->GetText() == "Human")
                humanPlayers++;
            m_apPlayerTeamSelect[player]->SetVisible(true);
            m_apPlayerTechSelect[player]->SetVisible(true);
            m_apPlayerHandicap[player]->SetVisible(true);
            m_apPlayerNameBox[player]->SetVisible(true);
			m_apPlayerAISkillSlider[player]->SetVisible(true);
			m_apPlayerAISkillLabel[player]->SetVisible(true);
        }

        // Count teams
        if (m_apPlayerTeamSelect[player]->GetVisible())
        {
            // Get the chosen team icon
            if (m_apPlayerTeamSelect[player]->GetSelectedItem())
                pTeamIcon = dynamic_cast<const Icon *>(m_apPlayerTeamSelect[player]->GetSelectedItem()->m_pEntity);
            // Just get the first one if nothing is selected
            else if (m_apPlayerTeamSelect[player]->GetCount() > 0)
                pTeamIcon = dynamic_cast<const Icon *>(m_apPlayerTeamSelect[player]->GetItem(0)->m_pEntity);

            if (pTeamIcon)
            {
                // See if the player is designated to a new team or one that has already been found
                bool newTeam = true;
                for (list<const Icon *>::iterator itr = teamList.begin(); itr != teamList.end(); ++itr)
                {
                    // Found existing team!
                    if (pTeamIcon->GetPresetName() == (*itr)->GetPresetName())
                    {
                        newTeam = false;
                        break;
                    }
                }

                // If we didn't find that the team we were designated already exists, then count it
                if (newTeam)
                    teamList.push_back(pTeamIcon);
            }
        }
    }

    // Hide/show the start game button depending on whether there's a valid player config right now
    m_apMetaButton[STARTNEW]->SetVisible(humanPlayers >= 1 && totalPlayers >= 2 && teamList.size() >= 2);

    // Show a helpful error message if the requirements for starting a game aren't met
    if (humanPlayers < 1)
        m_pErrorLabel->SetText("Need 1 Human");
    else if (totalPlayers < 2)
        m_pErrorLabel->SetText("Need 2 Players");
    else if (teamList.size() < 2)
        m_pErrorLabel->SetText("Need 2 Teams");
    else
        m_pErrorLabel->SetText("");

    // Update the game size label since the number of players may have changed
    UpdateGameSizeLabels();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerBars
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the floating player bars with current funds, flag, etc.

void MetagameGUI::UpdatePlayerBars()
{
    if (g_MetaMan.m_GameState >= MetaMan::NOGAME && g_MetaMan.m_GameState <= MetaMan::ENDROUND && !g_MetaMan.IsSuspended())
    {
        int metaPlayer = 0;
        char str[256];
        for (vector<MetaPlayer>::iterator mpItr = g_MetaMan.m_Players.begin(); mpItr != g_MetaMan.m_Players.end(); ++mpItr)
        {
//            m_apPlayerBox[metaPlayer];
//            m_apPlayerTeamBox[metaPlayer];

            // Show only the active players this game
            m_apPlayerBox[metaPlayer]->SetVisible(true);

            // Make sure their team flag icons are set up correctly
            if (!m_apPlayerTeamBox[metaPlayer]->GetDrawImage())
            {
                // Set the flag icons on the floating player bars
                m_apPlayerTeamBox[metaPlayer]->SetDrawType(GUICollectionBox::Image);
                m_apPlayerTeamBox[metaPlayer]->SetDrawImage(new AllegroBitmap(g_MetaMan.m_TeamIcons[(*mpItr).GetTeam()].GetBitmaps32()[0]));
            }

            // Show funds of player if income lines are showing, or we are counting income/expenses somehow
            if ((!m_PreTurn && metaPlayer == (g_MetaMan.m_GameState - MetaMan::PLAYER1TURN) && m_pSelectedScene) ||
                metaPlayer == m_ActivePlayerIncomeLines || g_MetaMan.m_GameState == MetaMan::COUNTINCOME || g_MetaMan.m_GameState == MetaMan::BUILDBASES || g_MetaMan.m_GameState == MetaMan::RUNACTIVITIES || g_MetaMan.m_GameState == MetaMan::ENDROUND)
            {
                std::snprintf(str, sizeof(str), "%c %.0f oz", -58, (*mpItr).m_Funds);
//                std::snprintf(str, sizeof(str), "%cx%d %c %.0f oz", -48, (*mpItr).GetBrainPoolCount(), -58, (*mpItr).m_Funds);
                m_apPlayerBarLabel[metaPlayer]->SetText(str);
                m_apPlayerBarLabel[metaPlayer]->SetHAlignment(GUIFont::Right);
                m_apPlayerBarLabel[metaPlayer]->SetToolTip("This player's total funds");
            }
            // Show player name instead
            else
            {
                // If the player is out of the game, show a skull before the name
                if (g_MetaMan.GetTotalBrainCountOfPlayer(metaPlayer) <= 0)
                    std::snprintf(str, sizeof(str), "%c ", -39);
                else
                    str[0] = 0;

                m_apPlayerBarLabel[metaPlayer]->SetText(string(str) + (*mpItr).GetName());
                m_apPlayerBarLabel[metaPlayer]->SetHAlignment(GUIFont::Left);
                m_apPlayerBarLabel[metaPlayer]->SetToolTip("");
            }

            // Update the brain pool count display
            m_apBrainPoolLabel[metaPlayer]->SetVisible(true);
            // Put the brain label on the right or left of the player box based on which half of the screen it's on
            if (m_apPlayerBox[metaPlayer]->GetXPos() > ((m_apScreenBox[ROOTBOX]->GetWidth() / 2) - (m_apPlayerBox[metaPlayer]->GetWidth() / 2)))
            {
                m_apBrainPoolLabel[metaPlayer]->SetPositionAbs(m_apPlayerBox[metaPlayer]->GetXPos() - m_apBrainPoolLabel[metaPlayer]->GetWidth() - 5, m_apPlayerBox[metaPlayer]->GetYPos());
                m_apBrainPoolLabel[metaPlayer]->SetHAlignment(GUIFont::Right);
            }
            else
            {
                m_apBrainPoolLabel[metaPlayer]->SetPositionAbs(m_apPlayerBox[metaPlayer]->GetXPos() + m_apPlayerBox[metaPlayer]->GetWidth() + 5, m_apPlayerBox[metaPlayer]->GetYPos());
                m_apBrainPoolLabel[metaPlayer]->SetHAlignment(GUIFont::Left);
            }
            // [Brain Icon] [X] Number
            // The number to display is adjusted with whether any brains are out and about in the gui animations
            int brainDisplayCount = (*mpItr).GetBrainPoolCount() - (*mpItr).GetBrainsInTransit();
            std::snprintf(str, sizeof(str), "%c%c%d", brainDisplayCount > 0 ? -48 : -25, -36, brainDisplayCount);
            m_apBrainPoolLabel[metaPlayer]->SetText(str);

            // Animate any funds change indicator labels, make them float upward
            if (!m_apFundsChangeTimer[metaPlayer].IsPastRealTimeLimit())
            {
                // Animate downward if value is negative, upward if positive
                int animDir = m_apFundsChangeLabel[metaPlayer]->GetText()[2] == '-' ? 1 : -1;
                int heightChange = EaseOut(0, 25, m_apFundsChangeTimer[metaPlayer].RealTimeLimitProgress());
                // Use the height of the label to keep track of the animation progress over several frames
                m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(m_apFundsChangeLabel[metaPlayer]->GetXPos(), m_apFundsChangeLabel[metaPlayer]->GetYPos() + animDir * (heightChange - m_apFundsChangeLabel[metaPlayer]->GetHeight() + 16));
                m_apFundsChangeLabel[metaPlayer]->Resize(m_apFundsChangeLabel[metaPlayer]->GetWidth(), 16 + heightChange);
                m_apFundsChangeLabel[metaPlayer]->SetVisible(true);
            }
            // Done animating and showing the change label, make it invisible and disappear
            else
            {
                m_apFundsChangeLabel[metaPlayer]->SetVisible(false);
                m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
            }

            // Animate any brain change indicator labels, make them float upward
            if (!m_apBrainsChangeTimer[metaPlayer].IsPastRealTimeLimit())
            {
                // Animate downward if value is negative, upward if positive
                int animDir = m_apBrainChangeLabel[metaPlayer]->GetText()[1] == '-' ? 1 : -1;
                int heightChange = EaseOut(0, 25, m_apBrainsChangeTimer[metaPlayer].RealTimeLimitProgress());
                // Use the height of the label to keep track of the animation progress over several frames
                m_apBrainChangeLabel[metaPlayer]->SetPositionAbs(m_apBrainChangeLabel[metaPlayer]->GetXPos(), m_apBrainChangeLabel[metaPlayer]->GetYPos() + animDir * (heightChange - m_apBrainChangeLabel[metaPlayer]->GetHeight() + 16));
                m_apBrainChangeLabel[metaPlayer]->Resize(m_apBrainChangeLabel[metaPlayer]->GetWidth(), 16 + heightChange);
                m_apBrainChangeLabel[metaPlayer]->SetVisible(true);
            }
            // Done animating and showing the change label, make it invisible and disappear
            else
            {
                m_apBrainChangeLabel[metaPlayer]->SetVisible(false);
                m_apBrainChangeLabel[metaPlayer]->SetPositionAbs(m_apScreenBox[ROOTBOX]->GetWidth(), 0);
            }

/* This is now obsolete; we show the name inside the bar
            // Update the player name labels above each floating bar, IF we're not drawing lines
            if (metaPlayer != m_ActivePlayerIncomeLines)
            {
                m_apFundsChangeLabel[metaPlayer]->SetText((*mpItr).GetName());
                m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(m_apPlayerBox[metaPlayer]->GetXPos() + 5, m_apPlayerBox[metaPlayer]->GetYPos() - 15);
                m_apFundsChangeLabel[metaPlayer]->SetVisible(true);
            }
            // Don't show the name label if we're drawing lines out of this player bar
            else
            {

                if (g_MetaMan.m_GameState >= MetaMan::PLAYER1TURN && g_MetaMan.m_GameState <= MetaMan::PLAYER4TURN)
                {
                    if ()
                    m_apFundsChangeLabel[metaPlayer]->SetVisible(false);
                }
                else
            }
*/
            metaPlayer++;
        }

        // Always hide any remaining player labels
        while (metaPlayer < Players::MaxPlayerCount)
        {
            m_apFundsChangeLabel[metaPlayer]->SetVisible(false);
            m_apBrainChangeLabel[metaPlayer]->SetVisible(false);
            metaPlayer++;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSiteHoverLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the floating label over a planet site.

void MetagameGUI::UpdateSiteNameLabel(bool visible, string text, const Vector &location, float height)
{
    // Set up the hover label to appear over any hovered scene location
    m_pScenePlanetLabel->SetVisible(visible);
    if (visible)
    {
        m_pScenePlanetLabel->SetText(text);
        m_pScenePlanetLabel->SetPositionAbs(m_PlanetCenter.m_X + location.m_X - (m_pScenePlanetLabel->GetWidth() / 2),
                                            m_PlanetCenter.m_Y + location.m_Y - (m_pScenePlanetLabel->GetHeight() * 1.5 * height));

        // Clamp it to within the screen.. only Y applies to the label though
        int pad = 6;
/*
        if (m_pScenePlanetLabel->GetXPos() < pad)
            m_pScenePlanetLabel->SetPositionAbs(pad, m_pScenePlanetLabel->GetYPos());
        else if (m_pScenePlanetLabel->GetXPos() + m_pScenePlanetLabel->GetWidth() + pad >= g_FrameMan.GetResX())
            m_pScenePlanetLabel->SetPositionAbs(g_FrameMan.GetResX() - m_pScenePlanetLabel->GetWidth() - pad, m_pScenePlanetLabel->GetYPos());
*/
        if (m_pScenePlanetLabel->GetYPos() < pad)
            m_pScenePlanetLabel->SetPositionAbs(m_pScenePlanetLabel->GetXPos(), pad);
        else if (m_pScenePlanetLabel->GetYPos() + m_pScenePlanetLabel->GetHeight() + pad >= g_FrameMan.GetResY())
            m_pScenePlanetLabel->SetPositionAbs(m_pScenePlanetLabel->GetXPos(), g_FrameMan.GetResY() - m_pScenePlanetLabel->GetHeight() - pad);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerTextIndication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts an animation of a label showing a text string over a player bar

void MetagameGUI::PlayerTextIndication(int metaPlayer, string text, const Vector &screenPos, double animLengthMS)
{
    m_apFundsChangeLabel[metaPlayer]->SetText(text);
    m_apFundsChangeLabel[metaPlayer]->SetHAlignment(GUIFont::Centre);
    m_apFundsChangeLabel[metaPlayer]->SetVAlignment(GUIFont::Middle);
    m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(screenPos.m_X - (m_apFundsChangeLabel[metaPlayer]->GetWidth() / 2), screenPos.m_Y - (m_apFundsChangeLabel[metaPlayer]->GetHeight() / 2));
    // The height is how the things get animated
    m_apFundsChangeLabel[metaPlayer]->Resize(m_apFundsChangeLabel[metaPlayer]->GetWidth(), 16);
    m_apFundsChangeLabel[metaPlayer]->SetVisible(true);

    // Start off the timer for the animation, which is updated in UpdatePlayerBars()
    m_apFundsChangeTimer[metaPlayer].Reset();
    m_apFundsChangeTimer[metaPlayer].SetRealTimeLimitMS(animLengthMS);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FundsChangeIndication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts an animation of a label showing funds changing for a player

void MetagameGUI::FundsChangeIndication(int metaPlayer, float change, const Vector &screenPos, double animLengthMS)
{
    char str[256];
    std::snprintf(str, sizeof(str), change >= 1.0 ? "%c +%.0f oz" : (change <= -1.0 ? "%c %.0f oz" : "%c %.0f oz"), -58, change);
    m_apFundsChangeLabel[metaPlayer]->SetText(str);
    m_apFundsChangeLabel[metaPlayer]->SetHAlignment(GUIFont::Right);
    m_apFundsChangeLabel[metaPlayer]->SetVAlignment(GUIFont::Top);
    m_apFundsChangeLabel[metaPlayer]->SetPositionAbs(screenPos.m_X - m_apFundsChangeLabel[metaPlayer]->GetWidth(), screenPos.m_Y);
    // The height is how the things get animated
    m_apFundsChangeLabel[metaPlayer]->Resize(m_apFundsChangeLabel[metaPlayer]->GetWidth(), 16);
    m_apFundsChangeLabel[metaPlayer]->SetVisible(true);

    // Start off the timer for the animation, which is updated in UpdatePlayerBars()
    m_apFundsChangeTimer[metaPlayer].Reset();
    m_apFundsChangeTimer[metaPlayer].SetRealTimeLimitMS(animLengthMS);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BrainsChangeIndication
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts an animation of a label showing brains changing for a metaPlayer

void MetagameGUI::BrainsChangeIndication(int metaPlayer, int change, const Vector &screenPos, int fontAlignment, double animLengthMS)
{
    char str[256];
    // [Brain Icon] [X] Number
    // The number to display is adjusted with whether any brains are out and about in the gui animations
    std::snprintf(str, sizeof(str), change >= 0 ? "%c+%d" : "%c%d", -48, change);
    m_apBrainChangeLabel[metaPlayer]->SetText(str);

    m_apBrainChangeLabel[metaPlayer]->SetHAlignment(fontAlignment);
    m_apBrainChangeLabel[metaPlayer]->SetVAlignment(GUIFont::Top);
    m_apBrainChangeLabel[metaPlayer]->SetPositionAbs(screenPos.m_X, screenPos.m_Y);
    // The height is how the things get animated
    m_apBrainChangeLabel[metaPlayer]->Resize(m_apBrainPoolLabel[metaPlayer]->GetWidth(), 16);
    m_apBrainChangeLabel[metaPlayer]->SetVisible(true);

    // Start off the timer for the animation, which is updated in UpdatePlayerBars()
    m_apBrainsChangeTimer[metaPlayer].Reset();
    m_apBrainsChangeTimer[metaPlayer].SetRealTimeLimitMS(animLengthMS);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveSiteLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specific index siteline out of a vector.

bool MetagameGUI::RemoveSiteLine(vector<SiteLine> &lineList, int removeIndex)
{
    if (lineList.empty())
        return false;

    int index = 0;
    bool removed = false;
    for (vector<SiteLine>::iterator slItr = lineList.begin(); slItr != lineList.end(); ++slItr, ++index)
    {        
        if (index == removeIndex)
        {
            lineList.erase(slItr);
            removed = true;
        }
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerLineFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total funds of all visible lines of a specific player.

float MetagameGUI::GetPlayerLineFunds(vector<SiteLine> &lineList, int metaPlayer, bool onlyVisible)
{
    if (metaPlayer < Players::PlayerOne || metaPlayer >= g_MetaMan.m_Players.size())
        return 0;

    // Figure out the total visible meter funds of this player
    float totalFunds = 0;
    for (vector<SiteLine>::iterator slItr = lineList.begin(); slItr != lineList.end(); ++slItr)
        if ((*slItr).m_Player == metaPlayer && (!onlyVisible || IsSiteLineVisible(*slItr)))
            totalFunds += (*slItr).m_FundsAmount;

    return totalFunds;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerLineRatios
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the site line meter ratios of a player based on their fund
//                  amounts and visibilty.

void MetagameGUI::UpdatePlayerLineRatios(vector<SiteLine> &lineList, int metaPlayer, bool onlyVisible, float total)
{
    if (metaPlayer < Players::PlayerOne || metaPlayer >= g_MetaMan.m_Players.size())
        return;

    // Figure out the total visible meter funds of this player, unless a total already specifically specified
    float totalFunds = total > 0 ? total : GetPlayerLineFunds(lineList, metaPlayer, onlyVisible);

    // Now go through the visible ones and set their meter ratios appropriately
    float meterStart = 0;
    for (vector<SiteLine>::iterator slItr = lineList.begin(); slItr != lineList.end(); ++slItr)
    {
        if ((*slItr).m_Player == metaPlayer && (!onlyVisible || IsSiteLineVisible(*slItr)))
        {
            (*slItr).m_StartMeterAt = meterStart;
            (*slItr).m_MeterAmount = (*slItr).m_FundsAmount / totalFunds;
            meterStart += (*slItr).m_MeterAmount;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGlowLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering line to point out scene points on the
//                  planet.

void MetagameGUI::DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color)
{
	int blendAmount = 210 + RandomNum(-15, 15);
    set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
    line(drawBitmap, start.m_X, start.m_Y, end.m_X, end.m_Y, color);
/* Looks like ass
    // Draw the thickener lines thicker in the appropriate directions
    if (fabs(end.m_X - start.m_X) > fabs(end.m_Y - start.m_Y))
    {
        line(drawBitmap, start.m_X, start.m_Y + 1, end.m_X, end.m_Y + 1, color);
        line(drawBitmap, start.m_X, start.m_Y - 1, end.m_X, end.m_Y - 1, color);
    }
    else
    {
        line(drawBitmap, start.m_X + 1, start.m_Y, end.m_X + 1, end.m_Y, color);
        line(drawBitmap, start.m_X - 1, start.m_Y, end.m_X - 1, end.m_Y, color);
    }
*/
	blendAmount = 45 + RandomNum(-25, 25);
    set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
    line(drawBitmap, start.m_X + 1, start.m_Y, end.m_X + 1, end.m_Y, color);
    line(drawBitmap, start.m_X - 1, start.m_Y, end.m_X - 1, end.m_Y, color);
    line(drawBitmap, start.m_X, start.m_Y + 1, end.m_X, end.m_Y + 1, color);
    line(drawBitmap, start.m_X, start.m_Y - 1, end.m_X, end.m_Y - 1, color);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawScreenLineToSitePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering lines to point out scene points on the
//                  planet, FROM an arbitrary screen point.

bool MetagameGUI::DrawScreenLineToSitePoint(BITMAP *drawBitmap,
                                            const Vector &screenPoint,
                                            const Vector &planetPoint,
                                            int color,
                                            int onlyFirstSegments,
                                            int onlyLastSegments,
                                            int channelHeight,
                                            float circleSize,
                                            bool squareSite) const
{
    // No part of the line is visible with these params, so just quit
    if (onlyFirstSegments == 0 || onlyLastSegments == 0)
        return false;
    // Detect disabling of the segment controls
    if (onlyFirstSegments < 0)
        onlyFirstSegments = 100;
    if (onlyLastSegments < 0)
        onlyLastSegments = 100;

    int totalSegments = 0;
    int drawnFirstSegments = 0;
    int lastSegmentsToDraw = 0;
    int circleRadius = squareSite ? std::floor(6 * circleSize) : std::floor(8 * circleSize);
    int chamferSize = CHAMFERSIZE;
    Vector chamferPoint1;
    Vector chamferPoint2;
    Vector sitePos = m_PlanetCenter + planetPoint;
    bool siteIsAbove = sitePos.m_Y < screenPoint.m_Y;
    float yDirMult = siteIsAbove ? -1.0 : 1.0;
    bool twoBends = fabs(sitePos.m_Y - screenPoint.m_Y) < (channelHeight - circleRadius);
    bool noBends = (fabs(sitePos.m_X - screenPoint.m_X) < circleRadius);// && ((m_apPlayerBox[player]->GetWidth() * meterAmount * 0.5) >= fabs(sitePos.m_X - screenPoint.m_X));
    Vector firstBend(screenPoint.m_X, twoBends ? (screenPoint.m_Y + channelHeight * yDirMult) : sitePos.m_Y);
    Vector secondBend(sitePos.m_X, firstBend.m_Y);
    bool siteIsLeft = sitePos.m_X < screenPoint.m_X;
    float xDirMult = siteIsLeft ? -1.0 : 1.0;

    // No bends, meaning the mid of the meter goes straight up/down into the site circle
    if (noBends)
    {
        // How many possible segments there are total for this type of line: to site + circle
        totalSegments = lastSegmentsToDraw = 1 + 1;
        // Draw the line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, screenPoint + Vector(sitePos.m_X - screenPoint.m_X, 0), sitePos + Vector(0, (circleRadius + 1) * -yDirMult), color);
    }
    // Extra lines depending on whether there needs to be two bends due to the site being in the 'channel', ie next to the floating player bar
    else if (twoBends)
    {
        // Cap the chamfer size on the second bend appropriately
        chamferSize = MIN((firstBend - secondBend).GetMagnitude() - 15, chamferSize);
        chamferSize = MIN((secondBend - sitePos).GetMagnitude() - circleRadius * 3, chamferSize);
        // Snap the chamfer to not exist below a minimum size
        chamferSize = (chamferSize < 15) ? 0 : chamferSize;
        // No inverted chamfer
        chamferSize = MAX(0, chamferSize);
        chamferPoint1.SetXY(secondBend.m_X + chamferSize * -xDirMult, secondBend.m_Y);
        chamferPoint2.SetXY(secondBend.m_X, secondBend.m_Y + chamferSize * -yDirMult);
        // How many of the last segments to draw: to first bend + to second bend chamfer + chamfer + to site + circle
        totalSegments = lastSegmentsToDraw = 1 + 1 + (int)(chamferSize > 0) + 1 + 1;
        // Line to the first bend
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, screenPoint, firstBend, color);
        // Line to the second bend, incl the chamfer
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, firstBend, chamferPoint1, color);
        if (chamferSize > 0 && !(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
        // Line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector(0, (circleRadius + 1) * yDirMult), color);
    }
    // Just one bend
    else
    {
        // Cap the chamfer size on the first bend appropriately
        chamferSize = MIN((screenPoint - firstBend).GetMagnitude() - 15, chamferSize);
        chamferSize = MIN((firstBend - sitePos).GetMagnitude() - circleRadius * 3, chamferSize);
        // Snap the chamfer to not exist below a minimum size
        chamferSize = (chamferSize < 15) ? 0 : chamferSize;
        // No inverted chamfer
        chamferSize = MAX(0, chamferSize);
        chamferPoint1.SetXY(screenPoint.m_X, firstBend.m_Y + chamferSize * -yDirMult);
        chamferPoint2.SetXY(firstBend.m_X + chamferSize * xDirMult, sitePos.m_Y);
        // How many of the last segments to draw: to first bend chamfer + chamfer + to site + circle
        totalSegments = lastSegmentsToDraw = 1 + (int)(chamferSize > 0) + 1 + 1;
        // Draw line to the first bend, incl the chamfer
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, screenPoint, chamferPoint1, color);
        if (chamferSize > 0 && !(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
        // Draw line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector((circleRadius + 1) * -xDirMult, 0), color);
    }

    // Draw a circle around the site target
    if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
    {
		int blendAmount = 225 + RandomNum(-20, 20);
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

        // If specified, draw a squareSite instead (with chamfered corners)
        if (squareSite)
        {
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1 - 1, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y + circleRadius, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y + circleRadius + 1, sitePos.m_X + circleRadius, color);
            vline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_Y + circleRadius, color);
            vline(drawBitmap, sitePos.m_X - circleRadius - 1 - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_Y + circleRadius, color);
            vline(drawBitmap, sitePos.m_X + circleRadius, sitePos.m_Y + circleRadius, sitePos.m_Y - circleRadius - 1, color);
            vline(drawBitmap, sitePos.m_X + circleRadius + 1, sitePos.m_Y + circleRadius, sitePos.m_Y - circleRadius - 1, color);
        }
        else
        {
            circle(drawBitmap, sitePos.m_X, sitePos.m_Y, circleRadius, color);
            circle(drawBitmap, sitePos.m_X, sitePos.m_Y, circleRadius - 1, color);            
        }
    }

    return totalSegments <= onlyFirstSegments && totalSegments <= onlyLastSegments;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawPlayerLineToSitePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering lines to point out scene points on the
//                  planet, FROM a floating player bar, showing a certain ratio.

bool MetagameGUI::DrawPlayerLineToSitePoint(BITMAP *drawBitmap,
                                          int metaPlayer,
                                          float startMeterAt,
                                          float meterAmount,
                                          const Vector &planetPoint,
                                          int color,
                                          int onlyFirstSegments,
                                          int onlyLastSegments,
                                          int channelHeight,
                                          float circleSize,
                                          bool squareSite,
                                          bool drawMeterOverride) const
{
    RTEAssert(metaPlayer >= Players::PlayerOne && metaPlayer < Players::MaxPlayerCount, "Player out of bounds");
    // No part of the line is visible with these params, so just quit
    if ((onlyFirstSegments == 0 || onlyLastSegments == 0) && !drawMeterOverride)
        return false;
    startMeterAt = MAX(0, startMeterAt);
    startMeterAt = MIN(1.0, startMeterAt);
    if ((startMeterAt + meterAmount) > 1.0)
        meterAmount = 1.0 - startMeterAt;
    // Detect disabling of the segment controls
    if (onlyFirstSegments < 0)
        onlyFirstSegments = 100;
    if (onlyLastSegments < 0)
        onlyLastSegments = 100;

    int totalSegments = 0;
    int drawnFirstSegments = 0;
    int lastSegmentsToDraw = 0;
    int meterHeight = 5;
    int circleRadius = squareSite ? std::floor(6 * circleSize) : std::floor(8 * circleSize);
    int chamferSize = CHAMFERSIZE;
    Vector chamferPoint1;
    Vector chamferPoint2;
    int boxMidY = m_apPlayerBox[metaPlayer]->GetYPos() + (m_apPlayerBox[metaPlayer]->GetHeight() / 2);
    Vector sitePos = m_PlanetCenter + planetPoint;
    bool siteIsAbove = sitePos.m_Y < boxMidY;
    float yDirMult = siteIsAbove ? -1.0 : 1.0;
    bool twoBends = fabs(sitePos.m_Y - boxMidY) < (channelHeight - circleRadius);
    Vector startMeter(m_apPlayerBox[metaPlayer]->GetXPos() + (m_apPlayerBox[metaPlayer]->GetWidth() - 1) * startMeterAt + 1, m_apPlayerBox[metaPlayer]->GetYPos() + (siteIsAbove ? 0 : m_apPlayerBox[metaPlayer]->GetHeight()));
    Vector endMeter(startMeter.m_X + MAX(0, (m_apPlayerBox[metaPlayer]->GetWidth() - 1) * meterAmount - 2), startMeter.m_Y);
    Vector midMeter(startMeter.m_X + MAX(0, (m_apPlayerBox[metaPlayer]->GetWidth() - 1) * meterAmount * 0.5 - 1), startMeter.m_Y + meterHeight * yDirMult);
    bool noBends = (fabs(sitePos.m_X - midMeter.m_X) < circleRadius) && ((m_apPlayerBox[metaPlayer]->GetWidth() * meterAmount * 0.5) >= fabs(sitePos.m_X - midMeter.m_X));
    Vector firstBend(midMeter.m_X, twoBends ? (boxMidY + channelHeight * yDirMult) : sitePos.m_Y);
    Vector secondBend(sitePos.m_X, firstBend.m_Y);
    bool siteIsLeft = sitePos.m_X < midMeter.m_X;
    float xDirMult = siteIsLeft ? -1.0 : 1.0;

    // No bends, meaning the mid of the meter goes straight up/down into the site circle
    if (noBends)
    {
        // How many possible segments there are total for this type of line: meter + to site + circle
        totalSegments = lastSegmentsToDraw = 1 + 1 + 1;
        // Draw the meter
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments) || drawMeterOverride)
        {
            DrawGlowLine(drawBitmap, startMeter, startMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, endMeter, endMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, startMeter + Vector(0, meterHeight * yDirMult), endMeter + Vector(0, meterHeight * yDirMult), color);
        }
        // Draw the line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, midMeter + Vector(sitePos.m_X - midMeter.m_X, 0), sitePos + Vector(0, (circleRadius + 1) * -yDirMult), color);
    }
    // Extra lines depending on whether there needs to be two bends due to the site being in the 'channel', ie next to teh floating metaPlayer bar
    else if (twoBends)
    {
        // Cap the chamfer size on the second bend appropriately
        chamferSize = MIN((firstBend - secondBend).GetMagnitude() - meterHeight * 3, chamferSize);
        chamferSize = MIN((secondBend - sitePos).GetMagnitude() - circleRadius * 3, chamferSize);
        // Snap the chamfer to not exist below a minimum size
        chamferSize = (chamferSize < (meterHeight * 3)) ? 0 : chamferSize;
        // No inverted chamfer
        chamferSize = MAX(0, chamferSize);
        chamferPoint1.SetXY(secondBend.m_X + chamferSize * -xDirMult, secondBend.m_Y);
        chamferPoint2.SetXY(secondBend.m_X, secondBend.m_Y + chamferSize * -yDirMult);
        // How many of the last segments to draw: meter + to first bend + to second bend chamfer + chamfer + to site + circle
        totalSegments = lastSegmentsToDraw = 1 + 1 + 1 + (int)(chamferSize > 0) + 1 + 1;
        // Draw the meter
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments) || drawMeterOverride)
        {
            DrawGlowLine(drawBitmap, startMeter, startMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, endMeter, endMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, startMeter + Vector(0, meterHeight * yDirMult), endMeter + Vector(0, meterHeight * yDirMult), color);
        }
        // Line to the first bend
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, midMeter, firstBend, color);
        // Line to the second bend, incl the chamfer
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, firstBend, chamferPoint1, color);
        if (chamferSize > 0 && !(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
        // Line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector(0, (circleRadius + 1) * yDirMult), color);
    }
    // Just one bend
    else
    {
        // Cap the chamfer size on the first bend appropriately
        chamferSize = MIN((midMeter - firstBend).GetMagnitude() - meterHeight * 3, chamferSize);
        chamferSize = MIN((firstBend - sitePos).GetMagnitude() - circleRadius * 3, chamferSize);
        // Snap the chamfer to not exist below a minimum size
        chamferSize = (chamferSize < (meterHeight * 3)) ? 0 : chamferSize;
        // No inverted chamfer
        chamferSize = MAX(0, chamferSize);
        chamferPoint1.SetXY(midMeter.m_X, firstBend.m_Y + chamferSize * -yDirMult);
        chamferPoint2.SetXY(firstBend.m_X + chamferSize * xDirMult, sitePos.m_Y);
        // How many of the last segments to draw: meter + to first bend chamfer + chamfer + to site + circle
        totalSegments = lastSegmentsToDraw = 1 + 1 + (int)(chamferSize > 0) + 1 + 1;
        // Draw the meter
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments) || drawMeterOverride)
        {
            DrawGlowLine(drawBitmap, startMeter, startMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, endMeter, endMeter + Vector(0, meterHeight * yDirMult), color);
            DrawGlowLine(drawBitmap, startMeter + Vector(0, meterHeight * yDirMult), endMeter + Vector(0, meterHeight * yDirMult), color);
        }
        // Draw line to the first bend, incl the chamfer
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, midMeter, chamferPoint1, color);
        if (chamferSize > 0 && !(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint1, chamferPoint2, color);
        // Draw line to the site
        if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
            DrawGlowLine(drawBitmap, chamferPoint2, sitePos + Vector((circleRadius + 1) * -xDirMult, 0), color);
    }

    // Draw a circle around the site target
    if (!(drawnFirstSegments++ >= onlyFirstSegments || lastSegmentsToDraw-- > onlyLastSegments))
    {
        int blendAmount = 225 + RandomNum(-20, 20);
        set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

        // If specified, draw a squareSite instead (with chamfered corners)
        if (squareSite)
        {
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1 - 1, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y + circleRadius, sitePos.m_X + circleRadius, color);
            hline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y + circleRadius + 1, sitePos.m_X + circleRadius, color);
            vline(drawBitmap, sitePos.m_X - circleRadius - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_Y + circleRadius, color);
            vline(drawBitmap, sitePos.m_X - circleRadius - 1 - 1, sitePos.m_Y - circleRadius - 1, sitePos.m_Y + circleRadius, color);
            vline(drawBitmap, sitePos.m_X + circleRadius, sitePos.m_Y + circleRadius, sitePos.m_Y - circleRadius - 1, color);
            vline(drawBitmap, sitePos.m_X + circleRadius + 1, sitePos.m_Y + circleRadius, sitePos.m_Y - circleRadius - 1, color);
        }
        else
        {
            circle(drawBitmap, sitePos.m_X, sitePos.m_Y, circleRadius, color);
            circle(drawBitmap, sitePos.m_X, sitePos.m_Y, circleRadius - 1, color);            
        }
    }

    return totalSegments <= onlyFirstSegments && totalSegments <= onlyLastSegments;
}
