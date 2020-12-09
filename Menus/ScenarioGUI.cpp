//////////////////////////////////////////////////////////////////////////////////////////
// File:            ScenarioGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the ScenarioGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ScenarioGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "MetaMan.h"
#include "AudioMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

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
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "DataModule.h"
#include "Scene.h"

extern int g_IntroState;
extern volatile bool g_Quit;
extern int g_StationOffsetX;
extern int g_StationOffsetY;

using namespace RTE;

#define CHAMFERSIZE 40

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ScenarioGUI, effectively
//                  resetting the members of this abstraction level only.

void ScenarioGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_MenuEnabled = ENABLED;
    m_MenuScreen = SCENESELECT;
    m_ScreenChange = false;
    m_SceneFocus = 0;
    m_FocusChange = 0;
    m_MenuSpeed = 0.3;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;

    m_PlanetCenter.Reset();
    m_PlanetRadius = 240.0f;

    m_pRootBox = 0;
    m_pActivityBox = 0;
    m_pSceneInfoBox = 0;
    m_pPlayerSetupBox = 0;
    m_pQuitConfirmBox = 0;

	m_SelectTutorial = true;

    for (int button = 0; button < SCENARIOBUTTONCOUNT; ++button)
        m_aScenarioButton[button] = 0;
    m_pScenePlanetLabel = 0;

    m_pActivitySelect = 0;
    m_pActivityLabel = 0;
    m_pDifficultyLabel = 0;
    m_pDifficultySlider = 0;

    m_pSceneCloseButton = 0;
    m_pSceneNameLabel = 0;
    m_pSceneInfoLabel = 0;
    for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
    {
        for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
        {
//            m_aaControls = team == TEAM_DISABLED;
            m_aapPlayerBoxes[player][team] = 0;
        }
    }

    for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
    {
        m_apTeamBoxes[team] = 0;
        m_apTeamNameLabels[team] = 0;
    }

	for (int team = Activity::TeamOne; team < RTE::Activity::MaxTeamCount; ++team)
	{
        m_apTeamTechSelect[team] = 0;
		m_apTeamAISkillSlider[team] = 0;
		m_apTeamAISkillLabel[team] = 0;
	}

    m_pStartErrorLabel = 0;
    m_pCPULockLabel = 0;
    m_LockedCPUTeam = Activity::NoTeam;

    m_pGoldLabel = 0;
    m_pGoldSlider = 0;
	m_pFogOfWarCheckbox = 0;
	m_pRequireClearPathToOrbitCheckbox = 0;
	m_pDeployUnitsCheckbox = 0;

    m_pQuitConfirmLabel = 0;
    m_pQuitConfirmButton = 0;

    m_pScenes = 0;
    m_Activities.clear();
    m_pDraggedBox = 0;
    m_EngageDrag = false;
    m_pHoveredScene = 0;
    m_pSelectedScene = 0;
    m_PrevMousePos.Reset();

    m_ActivityRestarted = false;
    m_ActivityResumed = false;
    m_StartPlayers = 1;
    m_StartTeams = 2;
    m_StartFunds = 1600;
    m_StartDifficulty = Activity::MediumDifficulty;
    m_BackToMain = false;
    m_Quit = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ScenarioGUI object ready for use.

int ScenarioGUI::Create(Controller *pController)
{
    RTEAssert(pController, "No controller sent to ScenarioGUI on creation!");
    m_pController = pController;

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer32());
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(-1, true); 
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    if(!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/MainMenu"))
        RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu");
    m_pGUIController->Load("Base.rte/GUIs/ScenarioGUI.ini");

    // Make sure we have convenient points to the containing GUI collection boxes that we will manipulate the positions of
    m_pRootBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("root"));
    m_pActivityBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ActivitySelectBox"));
    m_pSceneInfoBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("SceneInfoBox"));
    m_pPlayerSetupBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("PlayerSetupBox"));
    m_pQuitConfirmBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("ConfirmDialog"));

    // Make the root box fill the screen
//    m_pRootBox->SetPositionAbs((g_FrameMan.GetResX() - m_pRootBox->GetWidth()) / 2, 0);// (g_FrameMan.GetResY() - m_pRootBox->GetHeight()) / 2);
    m_pRootBox->Resize(g_FrameMan.GetResX(), g_FrameMan.GetResY());

    m_aScenarioButton[BACKTOMAIN] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("BackToMainButton"));
    m_aScenarioButton[RESUME] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ButtonResume"));
    m_aScenarioButton[STARTHERE] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SceneSelectButton"));
    m_aScenarioButton[STARTGAME] = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("StartButton"));

    m_pScenePlanetLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ScenePlanetLabel"));
    m_pScenePlanetLabel->SetVisible(false);

    // Activity Selection Box
    m_pActivitySelect = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("ActivitySelectCombo"));
    m_pActivityLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ActivityDescLabel"));
    m_pDifficultyLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("DifficultyLabel"));
    m_pDifficultySlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("DifficultySlider"));
    m_pActivitySelect->SetDropHeight(64);
//    m_pActivitySelect->GetListPanel()->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));
    m_pActivityLabel->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));

    // Scene Info Box
    m_pSceneCloseButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SceneCloseButton"));
    m_pSceneNameLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneNameLabel"));
    m_pSceneInfoLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("SceneInfoLabel"));
    m_pSceneInfoLabel->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));

    // Player team assignment box
    char str[128];
    for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
    {
        for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
        {
            // +1 because the controls are indexed starting at 1, not 0
            std::snprintf(str, sizeof(str), "P%dT%dBox", player + 1, team + 1);
            m_aapPlayerBoxes[player][team] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl(str));
        }
    }
    m_apTeamBoxes[TEAM_DISABLED] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("TDIcon"));
    m_apTeamBoxes[Activity::TeamOne] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T1Icon"));
    m_apTeamBoxes[Activity::TeamTwo] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T2Icon"));
    m_apTeamBoxes[Activity::TeamThree] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T3Icon"));
    m_apTeamBoxes[Activity::TeamFour] = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("T4Icon"));
    m_apTeamNameLabels[TEAM_DISABLED] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("TDLabel"));
    m_apTeamNameLabels[Activity::TeamOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T1Label"));
    m_apTeamNameLabels[Activity::TeamTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T2Label"));
    m_apTeamNameLabels[Activity::TeamThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T3Label"));
    m_apTeamNameLabels[Activity::TeamFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T4Label"));
    m_apTeamTechSelect[Activity::TeamOne] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T1TechCombo"));
    m_apTeamTechSelect[Activity::TeamTwo] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T2TechCombo"));
    m_apTeamTechSelect[Activity::TeamThree] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T3TechCombo"));
    m_apTeamTechSelect[Activity::TeamFour] = dynamic_cast<GUIComboBox *>(m_pGUIController->GetControl("T4TechCombo"));
	m_apTeamAISkillSlider[Activity::TeamOne] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T1AISkillSlider"));
	m_apTeamAISkillSlider[Activity::TeamTwo] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T2AISkillSlider"));
	m_apTeamAISkillSlider[Activity::TeamThree] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T3AISkillSlider"));
	m_apTeamAISkillSlider[Activity::TeamFour] = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("T4AISkillSlider"));
	m_apTeamAISkillLabel[Activity::TeamOne] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T1AISkillLabel"));
	m_apTeamAISkillLabel[Activity::TeamTwo] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T2AISkillLabel"));
	m_apTeamAISkillLabel[Activity::TeamThree] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T3AISkillLabel"));
	m_apTeamAISkillLabel[Activity::TeamFour] = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("T4AISkillLabel"));

	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++)
	{
		// Hide tech combobxes by default
	    m_apTeamTechSelect[team]->SetEnabled(false);
		m_apTeamTechSelect[team]->SetVisible(false);
	    // Add a "Random" tech choice to all the combo boxes first so that's at least in there always
	    m_apTeamTechSelect[team]->GetListPanel()->AddItem("-All-", "", 0, 0, -2);
	    m_apTeamTechSelect[team]->GetListPanel()->AddItem("-Random-", "", 0, 0, -1);
	    m_apTeamTechSelect[team]->SetSelectedIndex(0);
		// Hide AIs skill combobxes by default
	    m_apTeamAISkillSlider[team]->SetEnabled(false);
		m_apTeamAISkillSlider[team]->SetVisible(false);
		m_apTeamAISkillSlider[team]->SetValue(Activity::DefaultSkill);

	    m_apTeamAISkillLabel[team]->SetEnabled(false);
		m_apTeamAISkillLabel[team]->SetVisible(false);
		m_apTeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_apTeamAISkillSlider[team]->GetValue()));
	}
	m_pStartErrorLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("StartErrorLabel"));
    m_pCPULockLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CPULockLabel"));

    // Populate the tech comboboxes with the available tech modules
    const DataModule *pModule = 0;
    string techName;
    string techString = " Tech";
    string::size_type techPos = string::npos;
    for (int i = 0; i < g_PresetMan.GetTotalModuleCount(); ++i)  
    {
        pModule = g_PresetMan.GetDataModule(i);
		if (pModule)
		{
			techName = pModule->GetFriendlyName();
			if ((techPos = techName.find(techString)) != string::npos)
			{
				techName.replace(techPos, techString.length(), "");
				m_apTeamTechSelect[Activity::TeamOne]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_apTeamTechSelect[Activity::TeamTwo]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_apTeamTechSelect[Activity::TeamThree]->GetListPanel()->AddItem(techName, "", 0, 0, i);
				m_apTeamTechSelect[Activity::TeamFour]->GetListPanel()->AddItem(techName, "", 0, 0, i);
			}
		}
    }
    // Make the lists be scrolled to the top when they are initially dropped
    m_apTeamTechSelect[Activity::TeamOne]->GetListPanel()->ScrollToTop();
    m_apTeamTechSelect[Activity::TeamTwo]->GetListPanel()->ScrollToTop();
    m_apTeamTechSelect[Activity::TeamThree]->GetListPanel()->ScrollToTop();
    m_apTeamTechSelect[Activity::TeamFour]->GetListPanel()->ScrollToTop();

    m_pGoldLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("GoldLabel"));
    m_pGoldSlider = dynamic_cast<GUISlider *>(m_pGUIController->GetControl("GoldSlider"));
    m_pFogOfWarCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("FogOfWarCheckbox"));
    m_pRequireClearPathToOrbitCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("RequireClearPathToOrbitCheckbox"));
	m_pDeployUnitsCheckbox = dynamic_cast<GUICheckbox *>(m_pGUIController->GetControl("DeployUnitsCheckbox"));

    m_pQuitConfirmLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("ConfirmLabel"));
    m_pQuitConfirmButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConfirmButton"));

    // Set up initial combo box locations and sizes
    m_aScenarioButton[BACKTOMAIN]->SetPositionRel(m_pRootBox->GetWidth() - m_aScenarioButton[BACKTOMAIN]->GetWidth() - 16, m_pRootBox->GetHeight() - m_aScenarioButton[BACKTOMAIN]->GetHeight() - 22);
    m_aScenarioButton[RESUME]->SetPositionRel(m_pRootBox->GetWidth() - m_aScenarioButton[RESUME]->GetWidth() - 16, m_pRootBox->GetHeight() - m_aScenarioButton[RESUME]->GetHeight() - 47);
    m_pActivityBox->SetPositionRel(16, 16);
    m_pSceneInfoBox->SetPositionRel(m_pRootBox->GetWidth() -  m_pSceneInfoBox->GetWidth() - 16, 16);
    m_pPlayerSetupBox->CenterInParent(true, true);
    m_pQuitConfirmBox->CenterInParent(true, true);

	m_pScenePreviewBitmap = create_bitmap_ex(8, Scene::PREVIEW_WIDTH, Scene::PREVIEW_HEIGHT);

	// Load default preview bitmap
	ContentFile defaultPreview("Base.rte/GUIs/DefaultPreview.png");
	m_pDefaultPreviewBitmap = defaultPreview.GetAsBitmap(COLORCONV_NONE, false);

	clear_to_color(m_pScenePreviewBitmap, g_MaskColor);

    // Set initial focus, category list, and label settings
    m_ScreenChange = true;
    m_FocusChange = 1;
//    CategoryChange();

    // Fill our Scene and Activity lists
    GetAllScenesAndActivities();

    // Update the first box visible
    UpdateActivityBox();

    // Hide all screens, the appropriate screen will reappear on next update
    HideAllScreens();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ScenarioGUI object.

void ScenarioGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

	destroy_bitmap(m_pScenePreviewBitmap);
	destroy_bitmap(m_pDefaultPreviewBitmap);

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGUIControlManager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the GUIControlManager owned and used by this.

GUIControlManager * ScenarioGUI::GetGUIControlManager()
{
    return m_pGUIController;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void ScenarioGUI::SetEnabled(bool enable)
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
    else if (enable && m_MenuEnabled == ENABLED)
    {
        HideAllScreens();
        m_MenuScreen = SCENESELECT;
		// Reload all scenes and actvities to reflect scene changes player might do in scene editor.
		GetAllScenesAndActivities();
    }

    m_ScreenChange = true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void ScenarioGUI::Update()
{
    // Update the input controller
    m_pController->Update();

    // Reset the specific triggers
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
    //////////////////////////////////////
    // SCENE SELECTION SCREEN

    if (m_MenuScreen == SCENESELECT)
    {
        if (m_ScreenChange)
        {
            m_pActivityBox->SetVisible(true);
            // Scene box only appears when a scene is actively selected
            m_pSceneInfoBox->SetVisible(false);

            m_ScreenChange = false;
        }

        if (g_ActivityMan.GetActivity() && (g_ActivityMan.GetActivity()->GetActivityState() == Activity::Running || g_ActivityMan.GetActivity()->GetActivityState() == Activity::Editing))
		{
			if (!m_aScenarioButton[RESUME]->GetVisible())
				m_aScenarioButton[RESUME]->SetVisible(true);

			if (m_BlinkTimer.AlternateReal(500))
				m_aScenarioButton[RESUME]->SetFocus();
			else
				m_pGUIController->GetManager()->SetFocus(0);
		}
		else
			if (m_aScenarioButton[RESUME]->GetVisible())
	            m_aScenarioButton[RESUME]->SetVisible(false);

        // Get mouse position so we can see which scene it hovers over
        int mouseX, mouseY;
        m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
        Vector mousePos(mouseX, mouseY);
        Vector screenLocation;

		bool mouseIsInBox = false;

		// Detect if mouse is inside UI boxes
		GUIRect * r;
		r = m_pActivityBox->GetRect();
		if (mouseX > r->left && mouseX < r->right &&
			mouseY > r->top && mouseY < r->bottom)
			mouseIsInBox = true;

		r = m_pSceneInfoBox->GetRect();
		if (mouseX > r->left && mouseX < r->right &&
			mouseY > r->top && mouseY < r->bottom)
			mouseIsInBox = true;

        // Validate mouse position as being over the planet area for hover operations!
        if (m_pScenes && !m_pDraggedBox && (mousePos - m_PlanetCenter).GetMagnitude() < m_PlanetRadius && !mouseIsInBox)
        {
            // If unlocked, detect any Scene close to the mouse and highlight it
            bool foundAnyHover = false;
            bool foundNewHover = false;
            list<Scene *>::iterator sItr;
            list<Scene *>::iterator newCandidateItr = m_pScenes->end();
            float distance = 0;
            float shortestDist = 1000000.0;
            for (sItr = m_pScenes->begin(); sItr != m_pScenes->end(); ++sItr)
            {
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
            if (newCandidateItr != m_pScenes->end() && (*newCandidateItr) != m_pHoveredScene)
            {
                m_pHoveredScene = (*newCandidateItr);
                foundNewHover = true;
                g_GUISound.SelectionChangeSound()->Play();
				UpdateScenesBox();
            }

            // If we didn't find anything to hover over, then remove the hover status
            if (!foundAnyHover)
                m_pHoveredScene = 0;

            // Set up the hover label to appear over any hovered scene location
            if (m_pHoveredScene)
                UpdateSiteNameLabel(true, m_pHoveredScene->GetPresetName(), m_pHoveredScene->GetLocation()  + m_pHoveredScene->GetLocationOffset());
            else
                UpdateSiteNameLabel(false);

            // If clicked, whatever is hovered becomes selected
            if (g_UInputMan.MenuButtonPressed(UInputMan::MENU_EITHER))
            {
                if (m_pHoveredScene)
                {
                    m_pSelectedScene = m_pHoveredScene;
                    g_GUISound.ItemChangeSound()->Play();
					UpdateScenesBox();
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

        // Update the Activity setup and Scene info boxes
// No need to do this all the time, just when controls change
//        UpdateActivityBox();
		//if (!m_pSchemeSelect->IsDropped())
		UpdateScenesBox();
    }

    //////////////////////////////////////
    // PLAYER TEAM ASSIGNMENT SCREEN

    else if (m_MenuScreen == PLAYERSETUP)
    {
        if (m_ScreenChange)
        {
            m_pPlayerSetupBox->SetVisible(true);
//            m_aScenarioButton[BACKTOMAIN]->SetVisible(true);
            m_ScreenChange = false;
        }

        // Update the player selection box
        UpdatePlayersBox(false);

//        m_aScenarioButton[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////
    // QUIT CONFIRM DIALOG

    else if (m_MenuScreen == CONFIRMQUIT)
    {
        if (m_ScreenChange)
        {
            m_pQuitConfirmBox->SetVisible(true);
            m_ScreenChange = false;
        }

//        m_aScenarioButton[BACKTOMAIN]->SetFocus();
    }

    //////////////////////////////////////////
    // Update the ControlManager
// Don't do this twice!! it's already done in UpdateInput
//    m_pGUIController->Update();

    // Save mouse pos for next frame so we can do dragging
    if (m_EngageDrag)
        m_PrevMousePos = mousePos;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void ScenarioGUI::Draw(BITMAP *drawBitmap) const
{
    // Transparency effect on the scene dots and lines
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    // Screen blend the dots and lines, with some flicekring in its intensity
	int blendAmount = 120 + RandomNum(-55, 55);
    set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);

    // Draw sites etc only when selecting them
    if (m_MenuScreen == SCENESELECT && m_pScenes)
    {
        // Draw the scene location dots
        Vector screenLocation;
        for (list<Scene *>::const_iterator sItr = m_pScenes->begin(); sItr != m_pScenes->end(); ++sItr)
        {
			int color;

			// Mark user-created scenes to let players easily distinguish them from built-in
			if ((*sItr)->GetModuleID() == g_PresetMan.GetModuleID("Scenes.rte"))
				color = c_GUIColorGreen;
			else
				color = c_GUIColorYellow;

            screenLocation = m_PlanetCenter + (*sItr)->GetLocation() + (*sItr)->GetLocationOffset();
			blendAmount = 85 + RandomNum(-25, 25);
            set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
            circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 4, color);
            circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 2, color);
			blendAmount = 200 + RandomNum(-55, 55);
            set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
            circlefill(drawBitmap, screenLocation.m_X, screenLocation.m_Y, 1, color);
        }

        // Draw the lines etc pointing at the selected Scene from the Scene Info box
        if (m_pSelectedScene && m_pSceneInfoBox->GetVisible())
        {
            Vector sceneInfoBoxPos(m_pSceneInfoBox->GetXPos() + (m_pSceneInfoBox->GetWidth() / 2), m_pSceneInfoBox->GetYPos() + (m_pSceneInfoBox->GetHeight() / 2));
            DrawScreenLineToSitePoint(drawBitmap, sceneInfoBoxPos, m_pSelectedScene->GetLocation() +  m_pSelectedScene->GetLocationOffset(), c_GUIColorWhite, -1, -1, (m_pSceneInfoBox->GetHeight() / 2) + CHAMFERSIZE + 6, 1.0);
	    }
    }

    // Back to solid drawing
    drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

    AllegroScreen drawScreen(drawBitmap);

	m_pGUIController->Draw(&drawScreen);

    // Draw scene preview after GUI
    if (m_MenuScreen == SCENESELECT && m_pScenes)
    {
		if (m_pSelectedScene && m_pSceneInfoBox->GetVisible())
        {
			BITMAP * preview = m_pSelectedScene->GetPreviewBitmap();
			if (preview)
			{
				int xOffset = 0;
				int yOffset = 0;
				blit(preview, m_pScenePreviewBitmap, xOffset, yOffset, 0, 0, m_pScenePreviewBitmap->w, m_pScenePreviewBitmap->h);
			} else {
				int xOffset = 0;
				int yOffset = 0;
				blit(m_pDefaultPreviewBitmap, m_pScenePreviewBitmap, xOffset, yOffset, 0, 0, m_pScenePreviewBitmap->w, m_pScenePreviewBitmap->h);
			}
			draw_sprite(drawBitmap, m_pScenePreviewBitmap, m_pSceneInfoBox->GetXPos() + 10, m_pSceneInfoBox->GetYPos() + 33);
	    }
    }

    // Draw the Player-Team matrix lines and disabled overlay effects
    if (m_MenuScreen == PLAYERSETUP)
    {
        const Activity *pActivity = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
        int lineY = 80;
        for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        {
            // Disabled shaded boxes
            if (pActivity && (!pActivity->TeamActive(team) || m_LockedCPUTeam == team))
            {
// TODO: understand why the blending isnt working as desired
                // Transparency effect on the overlay boxes
                drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
                // Screen blend the dots and lines, with some flicekring in its intensity
                int blendAmount = 230;
                set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
                rectfill(drawBitmap, m_pPlayerSetupBox->GetXPos() + 110, m_pPlayerSetupBox->GetYPos() + lineY, m_pPlayerSetupBox->GetXPos() + m_pPlayerSetupBox->GetWidth() - 12, m_pPlayerSetupBox->GetYPos() + lineY + 25, c_GUIColorDarkBlue);
            }
            // Back to solid drawing
            drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
            // Cell border separator lines
            line(drawBitmap, m_pPlayerSetupBox->GetXPos() + 110, m_pPlayerSetupBox->GetYPos() + lineY, m_pPlayerSetupBox->GetXPos() + m_pPlayerSetupBox->GetWidth() - 12, m_pPlayerSetupBox->GetYPos() + lineY, c_GUIColorLightBlue);
            lineY += 25;
        }

		// Manually draw UI elements on top of colored rectangle
		for (int team = Activity::MaxTeamCount - 1; team >= Activity::TeamOne; team--)
		{
			if (m_apTeamTechSelect[team]->GetVisible())
			{
				m_apTeamTechSelect[team]->Draw(&drawScreen);
				if (m_apTeamTechSelect[team]->IsDropped())
					m_apTeamTechSelect[team]->GetListPanel()->Draw(&drawScreen);
			}

			if (m_apTeamAISkillSlider[team]->GetVisible())
			{
				m_apTeamAISkillSlider[team]->Draw(&drawScreen);
				m_apTeamAISkillLabel[team]->Draw(&drawScreen);
			}
		}
    }

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
// Method:          UpdateInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the user input processing.
// Arguments:       None.
// Return value:    None.

void ScenarioGUI::UpdateInput()
{
	// TODO: if activity is running, allow esc to resume activity instead of quitting.
    // If esc pressed, show quit dialog if applicable
    if (g_UInputMan.KeyPressed(KEY_ESC))
    {
        // Just quit if the dialog is already up
        if (m_pQuitConfirmBox->GetVisible())
            g_Quit = true;
        else
        {
            HideAllScreens();
            m_pQuitConfirmLabel->SetText("Sure you want to quit to OS?");//\nAny unsaved progress\nwill be lost!");
            m_pQuitConfirmButton->SetText("Quit");
            m_pQuitConfirmBox->SetVisible(true);
            m_MenuScreen = CONFIRMQUIT;
            m_ScreenChange = true;
        }
    }

    ///////////////////////////////////////////////////////////
    // Mouse handling

    // Get mouse position
    int mouseX, mouseY;
    m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
    Vector mousePos(mouseX, mouseY);
    
    // If not currently dragging a box, see if we should start
    bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
	if (m_MenuScreen == SCENESELECT && !m_pDraggedBox && menuButtonHeld && !m_EngageDrag && !m_pActivitySelect->IsDropped())
    {
        GUICollectionBox *pBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControlUnderPoint(mouseX, mouseY, m_pRootBox, 1));

        // Activity box?
        if (pBox == m_pActivityBox)
            m_pDraggedBox = pBox;

        // Scene info box?
        if (pBox == m_pSceneInfoBox)
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

    // Figure out dragging of the dialog boxes, if one is being dragged
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
// TODO ARG FIX THIS SHOULD NOT BE TWO UPDATES OF GUICONTROLLER IN THIS GUI
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
			// Return to main menu button pressed
			if (anEvent.GetControl()->GetName() == "BackToMainButton")
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();

                // Signal that we want to go back to main menu
                m_BackToMain = true;
                m_MenuScreen = SCENESELECT;
                m_ScreenChange = true;

                g_GUISound.BackButtonPressSound()->Play();
            }

			// Quit program button pressed
			if (anEvent.GetControl()->GetName() == "ConfirmButton")
            {
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_ScreenChange = true;
                g_Quit = true;

                g_GUISound.BackButtonPressSound()->Play();
            }

			if (anEvent.GetControl()->GetName() == "ButtonResume")
            {
                m_ActivityResumed = true;
                g_GUISound.BackButtonPressSound()->Play();
            }

			// Most big dialog cancel buttons lead back to the game menu too
			if (anEvent.GetControl()->GetName() == "PlayerCancelButton" ||
                anEvent.GetControl()->GetName() == "ConfirmCancelButton")
            {
                // Hide all previously shown screens
                HideAllScreens();
                m_MenuScreen = SCENESELECT;
                m_ScreenChange = true;
                g_GUISound.BackButtonPressSound()->Play();
            }

		    // Start Scenario Here menu button pressed
			if (anEvent.GetControl() == m_aScenarioButton[STARTHERE])
            {
                // Set up the player setup box based on updated Activity selection
                UpdatePlayersBox(true);
                // Hide all screens, the appropriate screen will reappear on next update
                HideAllScreens();
                m_MenuScreen = PLAYERSETUP;
                m_ScreenChange = true;

                g_GUISound.ButtonPressSound()->Play();
            }

			// Start game button pressed
			if (anEvent.GetControl() == m_aScenarioButton[STARTGAME])
            {
                // Try to start the game
                if (StartGame())
                {
                    // Hide all previously shown screens, show
                    HideAllScreens();
//                    m_MenuScreen = SCENESELECT;
//                    m_ScreenChange = true;
                    g_GUISound.ButtonPressSound()->Play();
                }
                else
                    g_GUISound.UserErrorSound()->Play();
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

			// Difficulty slider changed
			if (anEvent.GetControl() == m_pDifficultySlider)
            {
                // Update the difficulty label etc
                UpdateActivityBox();

                // Also stop dragging any panels if we're over any button
                m_pDraggedBox = 0;
                m_EngageDrag = true;
            }

			// Activity selection box affected
            if (anEvent.GetControl() == m_pActivitySelect)
            {
                // Also stop dragging any panels if we're over the selection list
                m_pDraggedBox = 0;
                m_EngageDrag = true;

                // The activity selection changed
                if (anEvent.GetMsg() == GUIComboBox::Closed)
                {
                    // Update the difficulty label etc
                    UpdateActivityBox();

                    // If there is only one Scene compatible with this newly selected Activity, then automatically select it
                    if (m_pScenes && m_pScenes->size() == 1)
                        m_pSelectedScene = m_pScenes->front();
                    // Deselect any previously selected scene.. it may not be compatible with the new activity
                    else
                        m_pSelectedScene = 0;

                    // Update the scene info box
                    UpdateScenesBox();
                    g_GUISound.ItemChangeSound()->Play();
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HideAllScreens
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides all menu screens, so one can easily be unhidden and shown only.

void ScenarioGUI::HideAllScreens()
{
    m_pActivityBox->SetVisible(false);
    m_pSceneInfoBox->SetVisible(false);
    m_pPlayerSetupBox->SetVisible(false);
    m_pQuitConfirmBox->SetVisible(false);

    m_pScenePlanetLabel->SetVisible(false);

    m_ScreenChange = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KeepBoxOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure a specific box doesn't end up moved completely off-screen.

void ScenarioGUI::KeepBoxOnScreen(GUICollectionBox *pBox, int margin)
{
    // Make sure the box doesn't go entirely outside of the screen
    if (pBox->GetXPos() < (-pBox->GetWidth() + margin))
        pBox->SetPositionAbs(-pBox->GetWidth() + margin, pBox->GetYPos());
    if (pBox->GetXPos() > m_pRootBox->GetWidth() - margin)
        pBox->SetPositionAbs(m_pRootBox->GetWidth() - margin, pBox->GetYPos());
    if (pBox->GetYPos() < (-pBox->GetHeight() + margin))
        pBox->SetPositionAbs(pBox->GetXPos(), -pBox->GetHeight() + margin);
    if (pBox->GetYPos() > m_pRootBox->GetHeight() - margin)
        pBox->SetPositionAbs(pBox->GetXPos(), m_pRootBox->GetHeight() - margin);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetupSkirmishActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ActivityMan up with the current data for a skirmish game.
// Arguments:       None.
// Return value:    None.

void ScenarioGUI::SetupSkirmishActivity()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateActivityBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the Activity selection box.

void ScenarioGUI::UpdateActivityBox()
{
    // Get the currently selected Activity
    const Activity *pSelected = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;

    if (pSelected)
    {
        // Pull out the list of Scenes that are compatible with this Activity
        map<Activity *, list<Scene *> >::iterator asItr;
        if (m_Activities.end() != (asItr = m_Activities.find(const_cast<Activity *>(pSelected))))
            m_pScenes = &((*asItr).second);
        else
            m_pScenes = 0;

        // Set the description
        if (m_pScenes && m_pScenes->size() == 1)
            m_pActivityLabel->SetText(pSelected->GetDescription() + "\n\nThe only site where this activity can be played has been selected for you.");
        else if (m_pScenes && m_pScenes->size() > 1)
            m_pActivityLabel->SetText(pSelected->GetDescription() + "\n\nSites where this activity can be played appear on the planet. Select one to begin!");
        else
            m_pActivityLabel->SetText(pSelected->GetDescription() + "\n\nNO sites appear to be compatible with this selected activity! Please try another.");
        m_pDifficultyLabel->SetVisible(true);
        m_pDifficultySlider->SetVisible(true);
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

        // Resize the box to fit the desc
        int newHeight = m_pActivityLabel->ResizeHeightToFit();
        m_pActivityBox->Resize(m_pActivityBox->GetWidth(), newHeight + 110);
		UpdateScenesBox();

		const GameActivity * pSelectedGA = dynamic_cast<const GameActivity *>(pSelected);
		if (pSelectedGA)
		{
			// Set gold slider value if activity sepcifies default gold amounts for difficulties
			if (m_pDifficultySlider->GetValue() < Activity::CakeDifficulty)
			{
				if (pSelectedGA->GetDefaultGoldCake() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldCake());
			}
			else if (m_pDifficultySlider->GetValue() < Activity::EasyDifficulty)
			{
				if (pSelectedGA->GetDefaultGoldEasy() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldEasy());
			}
			else if (m_pDifficultySlider->GetValue() < Activity::MediumDifficulty)
			{
				if (pSelectedGA->GetDefaultGoldMedium() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldMedium());
			}
			else if (m_pDifficultySlider->GetValue() < Activity::HardDifficulty)
			{
				if (pSelectedGA->GetDefaultGoldHard() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldHard());
			}
			else if (m_pDifficultySlider->GetValue() < Activity::NutsDifficulty)
			{
				if (pSelectedGA->GetDefaultGoldNuts() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldNuts());
			}
			else
			{
				if (pSelectedGA->GetDefaultGoldNuts() > -1)
					m_pGoldSlider->SetValue(pSelectedGA->GetDefaultGoldNuts());
			}
			m_pGoldSlider->SetEnabled(pSelectedGA->GetGoldSwitchEnabled());

			//Set default fog of war flag and enable or disable it if necessary
			if (pSelectedGA->GetDefaultFogOfWar() > -1)
			{
				if (pSelectedGA->GetDefaultFogOfWar() == 0)
					m_pFogOfWarCheckbox->SetCheck(0);
				else
					m_pFogOfWarCheckbox->SetCheck(1);
			}
			m_pFogOfWarCheckbox->SetEnabled(pSelectedGA->GetFogOfWarSwitchEnabled());

			//Set default clear path to orbit flag and enable or disable it if necessary
			if (pSelectedGA->GetDefaultRequireClearPathToOrbit() > -1)
			{
				if (pSelectedGA->GetDefaultRequireClearPathToOrbit() == 0)
					m_pRequireClearPathToOrbitCheckbox->SetCheck(0);
				else
					m_pRequireClearPathToOrbitCheckbox->SetCheck(1);
			}
			m_pRequireClearPathToOrbitCheckbox->SetEnabled(pSelectedGA->GetRequireClearPathToOrbitSwitchEnabled());

			//Set default deploy units flag and enable or disable it if necessary
			if (pSelectedGA->GetDefaultDeployUnits() > -1)
			{
				if (pSelectedGA->GetDefaultDeployUnits() == 0)
					m_pDeployUnitsCheckbox->SetCheck(0);
				else
					m_pDeployUnitsCheckbox->SetCheck(1);
			}
			m_pDeployUnitsCheckbox->SetEnabled(pSelectedGA->GetDeployUnitsSwitchEnabled());
		}
    }
    else
    {
		m_pScenes = 0;
        m_pActivityLabel->SetText("No Activity selected.");
        m_pDifficultyLabel->SetVisible(false);
        m_pDifficultySlider->SetVisible(false);
        // Resize the box to fit the desc
        int newHeight = m_pActivityLabel->ResizeHeightToFit();
        m_pActivityBox->Resize(m_pActivityBox->GetWidth(), newHeight + 125);
		UpdateScenesBox();
    }
    // Make sure the box doesn't go entirely outside of the screen
    KeepBoxOnScreen(m_pActivityBox);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateScenesBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the scene selection box.

void ScenarioGUI::UpdateScenesBox()
{
    // Always show the info box if something is selected
    if (m_pSelectedScene)
    {
		// Fill combo box
        const Activity *pActivity = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
		
		if (pActivity)
		{
			// Clear preview bitmap
			clear_to_color(m_pScenePreviewBitmap, g_MaskColor);
		}

        // Set the currently selected scene's texts
        m_pSceneInfoBox->SetVisible(true);
        m_pSceneNameLabel->SetText(m_pSelectedScene->GetPresetName());
        m_pSceneInfoLabel->SetText(m_pSelectedScene->GetDescription());
        // Adjust the height of the text box and container so it fits the text to display
        int newHeight = m_pSceneInfoLabel->ResizeHeightToFit();
        m_pSceneInfoBox->Resize(m_pSceneInfoBox->GetWidth(), newHeight + 140);
        // Blink the start game button
        m_aScenarioButton[STARTHERE]->SetText(m_BlinkTimer.AlternateReal(333) ? "Start Here" : "> Start Here <");
    }
    else
    {
        m_pSceneInfoBox->SetVisible(false);
        m_pSceneNameLabel->SetText("");
        m_pSceneInfoLabel->SetText("");
    }

    // Make sure the box doesn't go entirely outside of the screen
    KeepBoxOnScreen(m_pSceneInfoBox);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayersBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the contents of the player config box.

void ScenarioGUI::UpdatePlayersBox(bool newActivity)
{
    // Get the currently selected Activity
    const Activity *pActivity = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;
    const Icon *pIcon = 0;
    bool teamHasPlayers = false;
    bool teamHasHumans = false;
    int teamsWithPlayers = 0;
    int teamsWithHumans = 0;

    if (pActivity && m_pSelectedScene)
    {
        // Get mouse position and figure out if any cell is being hovered over
        int mouseX, mouseY;
        m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
        Vector mousePos(mouseX, mouseY);
        bool menuButtonHeld = g_UInputMan.MenuButtonHeld(UInputMan::MENU_EITHER);
		bool menuButtonReleased = g_UInputMan.MenuButtonReleased(UInputMan::MENU_EITHER);
        GUICollectionBox *pHoveredCell = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControlUnderPoint(mouseX, mouseY, m_pPlayerSetupBox, 1));

        // Is this a game activity?
        const GameActivity *pGameActivity = dynamic_cast<const GameActivity *>(pActivity);

        if (newActivity && pGameActivity)
        {
            // The pre-set team that should absolutely be CPU played
            m_LockedCPUTeam = pGameActivity->GetCPUTeam();
            // Align the locked CPU team text label with the appropriate row
            if (m_LockedCPUTeam != Activity::NoTeam)
                m_pCPULockLabel->SetPositionAbs(m_pCPULockLabel->GetXPos(), m_apTeamNameLabels[m_LockedCPUTeam]->GetYPos());
        }

        // Set up the matrix of player control boxes
        for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
        {
            for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
            {
                if (newActivity)
                {
                    // Everyone starts on the Disabled row, except perhaps the CPU which may be on its locked team
                    if (team == TEAM_DISABLED)
                    {
                        m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
                        pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : g_UInputMan.GetSchemeIcon(player);
                        if (pIcon)
                            m_aapPlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
                    }
                    // De-highlight all other cells initially
                    else
                    {
                        m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
                        m_aapPlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
                    }

                    // The CPU gets placed on its locked team
                    if (m_LockedCPUTeam != Activity::NoTeam && player == PLAYER_CPU)
                    {
                        if (team == m_LockedCPUTeam)
                        {
                            m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Image);
                            pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
                            if (pIcon)
                                m_aapPlayerBoxes[player][team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
                        }
                        else
                        {
                            m_aapPlayerBoxes[player][team]->SetDrawType(GUICollectionBox::Color);
                            m_aapPlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
                        }
                    }
                }
  
                // Make the hovered cell light up and able to be selected
                if (m_aapPlayerBoxes[player][team] == pHoveredCell
                    // if an active team row, but including the 'not playing' row
                    && (pActivity->TeamActive(team) || team == TEAM_DISABLED)
                    // That isn't on a team row locked to the CPU
                    && m_LockedCPUTeam != team
                    // And not the CPU player if he is locked to a CPU team
                    && (m_LockedCPUTeam == Activity::NoTeam || player != PLAYER_CPU))
                    // And a cell not already selected
                    //&& m_aapPlayerBoxes[player][team]->GetDrawType() != GUICollectionBox::Image)
                    // And players aren't maxed out for this Activity, or we are removing a player from team assignment
//                    && (player == PLAYER_CPU || team == TEAM_DISABLED || (pGameActivity && PlayerCount() < pGameActivity->GetMaxPlayerSupport())))
                {
                    // Is this being pushed and selected?
                    if (menuButtonReleased)
                    {
                        // Need to clear all other rows of this column
// TODO:  -- unless the CPU column?
                        for (int t2 = Activity::TeamOne; t2 < TEAMROWCOUNT; ++t2)
                        {
                            // This clicked cell should get the icon of this column
                            if (t2 == team)
                            {
								if (player != PLAYER_CPU)
								{
									m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
									pIcon = player == PLAYER_CPU ? dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU")) : g_UInputMan.GetSchemeIcon(player);
									if (pIcon)
										m_aapPlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
								} else {
									//Select or unselect CPU cells
									if (m_aapPlayerBoxes[player][t2]->GetDrawType() == GUICollectionBox::Image)
									{
										m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
										m_aapPlayerBoxes[player][t2]->SetDrawColor(c_GUIColorBlue);
									} else {
										pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
										if (pIcon)
										{
											m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Image);
											m_aapPlayerBoxes[player][t2]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
										}
									}
								}
                            }
                            // Now unselected columns
                            else
                            {
								if (player != PLAYER_CPU)
								{
									m_aapPlayerBoxes[player][t2]->SetDrawType(GUICollectionBox::Color);
									m_aapPlayerBoxes[player][t2]->SetDrawColor(c_GUIColorBlue);
								}
                            }
                        }
                        // If CPU changed to an actual team assignment, clear all human players off his new team
                        if (player == PLAYER_CPU && team != TEAM_DISABLED)
                        {
                            for (int p2 = Players::PlayerOne; p2 < Players::MaxPlayerCount; ++p2)
                            {
                                // Deselect the player's team assignment if he's on the same team as the CPU
                                if (m_aapPlayerBoxes[p2][team]->GetDrawType() == GUICollectionBox::Image)
                                {
                                    m_aapPlayerBoxes[p2][team]->SetDrawType(GUICollectionBox::Color);
                                    m_aapPlayerBoxes[p2][team]->SetDrawColor(c_GUIColorBlue);
                                    // Move him to disabled
                                    m_aapPlayerBoxes[p2][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
                                    pIcon = g_UInputMan.GetSchemeIcon(p2);
                                    if (pIcon)
                                        m_aapPlayerBoxes[p2][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
                                }
                            }
                        }
                        // If Player clicked CPU disabled button, clear CPU row
                        if (player == PLAYER_CPU && team == TEAM_DISABLED)
                        {
							for (int t2 = Activity::TeamOne; t2 <= Activity::TeamFour; ++t2)
							{
								if (m_aapPlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image)
								{
									m_aapPlayerBoxes[PLAYER_CPU][t2]->SetDrawType(GUICollectionBox::Color);
									m_aapPlayerBoxes[PLAYER_CPU][t2]->SetDrawColor(c_GUIColorBlue);
								}
							}
						}
                        // If a human player changed to a CPU team, remove the CPU guy
                        else if (player != PLAYER_CPU && team != TEAM_DISABLED)
                        {
                            // Deselect the CPU's team assignment if he's on the same team as the newly assigned human player
                            if (m_aapPlayerBoxes[PLAYER_CPU][team]->GetDrawType() == GUICollectionBox::Image)
                            {
                                m_aapPlayerBoxes[PLAYER_CPU][team]->SetDrawType(GUICollectionBox::Color);
                                m_aapPlayerBoxes[PLAYER_CPU][team]->SetDrawColor(c_GUIColorBlue);
                                // Move him to disabled
                                //m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
                                //pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
                                //if (pIcon)
                                //    m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
                            }
                        }
                        g_GUISound.FocusChangeSound()->Play();

						//Check if we need to clear or set CPU disabled team icon
						bool noCPUs = true;
						for (int t2 = Activity::TeamOne; t2 <= Activity::TeamFour; ++t2)
						{
							if (m_aapPlayerBoxes[PLAYER_CPU][t2]->GetDrawType() == GUICollectionBox::Image)
								noCPUs = false;
						}
						//Select or unselect CPU disabled icon
						if (noCPUs)
						{
							pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Device CPU"));
							if (pIcon)
							{
								m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Image);
								m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
							}
						} else {
							m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawType(GUICollectionBox::Color);
							m_aapPlayerBoxes[PLAYER_CPU][TEAM_DISABLED]->SetDrawColor(c_GUIColorBlue);
						}

                    }
                    // Just highlight the cell
                    else if (m_aapPlayerBoxes[player][team]->GetDrawColor() != c_GUIColorLightBlue)
                    {
                        m_aapPlayerBoxes[player][team]->SetDrawColor(c_GUIColorLightBlue);
                        g_GUISound.SelectionChangeSound()->Play();
                    }
                }
                // Un-highlight all other cells
                else if (pHoveredCell && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Color)
                    m_aapPlayerBoxes[player][team]->SetDrawColor(c_GUIColorBlue);
            }
        }

        // Team info columns
        for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
        {
            // Update the team names and such
            if (newActivity)
            {
                m_apTeamBoxes[team]->SetDrawType(GUICollectionBox::Image);
/* pointless; the CPU player icon suffices, and doesn't block the real team banner
                // CPU Team
                if (pGameActivity && pGameActivity->GetCPUTeam() == team)
                {
                    pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "CPU Team"));
                    m_apTeamNameLabels[team]->SetText(pActivity->GetTeamName(team) + ":");
                }
                // The not playing row
                else */if (team == TEAM_DISABLED)
                {
                    pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Disabled Team"));
                    m_apTeamNameLabels[team]->SetText("Not Playing:");
                }
                // Active player team
                else if (pActivity->TeamActive(team))
                {
                    // Set the team flag icons on the floating player bars
                    pIcon = pActivity->GetTeamIcon(team);
                    // Revert to default if needed
                    if (!pIcon)
                    {
                        char str[128];
                        std::snprintf(str, sizeof(str), "Team %d Default", team + 1);
                        pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", str));
                    }
                    m_apTeamNameLabels[team]->SetText(pActivity->GetTeamName(team) + ":");
                }
                // Disabled/unplayable teams
                else
                {
                    pIcon = dynamic_cast<const Icon *>(g_PresetMan.GetEntityPreset("Icon", "Locked Team"));
                    m_apTeamNameLabels[team]->SetText("Unavailable");
                }

                // Finally set whatever Icon we came up with
                if (pIcon)
                    m_apTeamBoxes[team]->SetDrawImage(new AllegroBitmap(pIcon->GetBitmaps32()[0]));
            }

            // Check if the team has any players assigned at all
            if (pActivity->TeamActive(team))
            {
                teamHasPlayers = false;
                for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
                {
                    // CPU is sometimes disabled, but still counts as a team
                    if (team != TEAM_DISABLED && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
					{
                        teamHasPlayers = true;
						if (player != PLAYER_CPU)
							teamHasHumans = true;
					}
                }
                if (teamHasPlayers)
                    ++teamsWithPlayers;
                if (teamHasHumans)
                    ++teamsWithHumans;

				m_apTeamTechSelect[team]->SetEnabled(true);
				m_apTeamTechSelect[team]->SetVisible(true);
				
				m_apTeamAISkillSlider[team]->SetEnabled(true);
				m_apTeamAISkillSlider[team]->SetVisible(true);
				m_apTeamAISkillLabel[team]->SetVisible(true);
            }
			else
			{
				if (team >= Activity::TeamOne && team < Activity::MaxTeamCount)
				{
					m_apTeamTechSelect[team]->SetEnabled(false);
					m_apTeamTechSelect[team]->SetVisible(false);
					
					m_apTeamAISkillSlider[team]->SetEnabled(false);
					m_apTeamAISkillSlider[team]->SetVisible(false);
					m_apTeamAISkillLabel[team]->SetVisible(false);
				}
			}
        }

        // If we are over capacity with players, disable the start button and show why
        if (pActivity->GetMaxPlayerSupport() < PlayerCount())
        {
            m_aScenarioButton[STARTGAME]->SetVisible(false);
            m_pStartErrorLabel->SetVisible(true);
            char str[256];
            std::snprintf(str, sizeof(str), "Too many players assigned! Max for this activity is %d", pGameActivity->GetMaxPlayerSupport());
            m_pStartErrorLabel->SetText(str);
        }
        // If we are under the required number of teams with players assigned, disable the start button and show why
        else if (pActivity->GetMinTeamsRequired() > teamsWithPlayers)
        {
            m_aScenarioButton[STARTGAME]->SetVisible(false);
            m_pStartErrorLabel->SetVisible(true);
            char str[256];
            std::snprintf(str, sizeof(str), "Assign players to at\nleast %d of the teams!", pGameActivity->GetMinTeamsRequired());
            m_pStartErrorLabel->SetText(str);
        }
		// Assign at least one human player
        else if (teamsWithHumans == 0)
        {
            m_aScenarioButton[STARTGAME]->SetVisible(false);
            m_pStartErrorLabel->SetVisible(true);
            m_pStartErrorLabel->SetText("Assign human players\nto at least one team!");
        }
        // Everything checks out; let the player start if they want to
        else
        {
            m_aScenarioButton[STARTGAME]->SetVisible(true);
            m_pStartErrorLabel->SetVisible(false);
        }

		// How much starting gold does the slider yield
        char str[256];
		//int startGold = (float)m_pGoldSlider->GetMinimum() + ((m_pGoldSlider->GetMaximum() - m_pGoldSlider->GetMinimum()) * (float)m_pGoldSlider->GetValue() / 100.0);
		int startGold = m_pGoldSlider->GetValue();
		startGold = startGold - startGold % 500;
		if (m_pGoldSlider->GetValue() == m_pGoldSlider->GetMaximum())
			std::snprintf(str, sizeof(str), "Starting Gold: %c Infinite", -58);
		else
			std::snprintf(str, sizeof(str), "Starting Gold: %c %d oz", -58, startGold);
		m_pGoldLabel->SetText(str);


		// Set skill labels
		for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++)
		{
			m_apTeamAISkillLabel[team]->SetText(Activity::GetAISkillString(m_apTeamAISkillSlider[team]->GetValue()));
		}
    }

    // Reset all buttons and positions of things if a new activity has been selected
    if (newActivity)
    {
        m_aScenarioButton[STARTGAME]->SetVisible(false);
        m_pStartErrorLabel->SetVisible(true);
        m_pCPULockLabel->SetVisible(m_LockedCPUTeam != Activity::NoTeam);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Counts how many players are currently assigned to play this Activity.

int ScenarioGUI::PlayerCount()
{
    int count = 0;
    // Go through all the on-team non-CPU cells and see how many players are already assigned.
    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
    {
        for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        {
            if (m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
                ++count;
        }
    }
    return count;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up and starts the currently selected Activity and settings.
// Arguments:       None.
// Return value:    None.

bool ScenarioGUI::StartGame()
{
    // Get the currently selected Activity
    const Activity *pActivityPreset = m_pActivitySelect->GetSelectedItem() ? dynamic_cast<const Activity *>(m_pActivitySelect->GetSelectedItem()->m_pEntity) : 0;

    if (!pActivityPreset || !m_pSelectedScene)
        return false;

    // Create the actual instance of hte Activity we want to start
    Activity *pActivity = dynamic_cast<Activity *>(pActivityPreset->Clone());
    GameActivity *pGameActivity = dynamic_cast<GameActivity *>(pActivity);

    // Set up the basic settings
    if (pGameActivity)
    {
        pGameActivity->SetDifficulty(m_pDifficultySlider->GetValue());

		pGameActivity->SetFogOfWarEnabled(m_pFogOfWarCheckbox->GetCheck());
		pGameActivity->SetRequireClearPathToOrbit(m_pRequireClearPathToOrbitCheckbox->GetCheck());


		// If gold slider is at it max value then the amount is 'infinite' and we must set some rediculously high value
		if (m_pGoldSlider->GetValue() == m_pGoldSlider->GetMaximum())
			pGameActivity->SetStartingGold(1000000);
		else
		{
			int startGold = m_pGoldSlider->GetValue();
			startGold = startGold - startGold % 500;
			pGameActivity->SetStartingGold(startGold);
		}
    }

	g_SceneMan.SetSceneToLoad(m_pSelectedScene, true, m_pDeployUnitsCheckbox->GetCheck());

    // Set up the player and team assignments
    pActivity->ClearPlayers(false);
    for (int player = Players::PlayerOne; player < PLAYERCOLUMNCOUNT; ++player)
    {
        for (int team = Activity::TeamOne; team < TEAMROWCOUNT; ++team)
        {
            if (team != TEAM_DISABLED && m_aapPlayerBoxes[player][team]->GetDrawType() == GUICollectionBox::Image)
            {
                // Add the human players, not including CPU players
                if (player != PLAYER_CPU)
                    pActivity->AddPlayer(player, true, team, 0);
                // CPU team, so mark it as such.. there are no actual CPU players
                else if (pGameActivity)
				{
                    pGameActivity->SetCPUTeam(team);
				}
            }
        }
    }

	for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
		// Set up techs
		GUIListPanel::Item *pTechItem = m_apTeamTechSelect[team]->GetSelectedItem();
		if (pTechItem)
		{
			// If the "random" selection, choose one from the list of loaded techs
			if (m_apTeamTechSelect[team]->GetSelectedIndex() == 1)//pTechItem->m_ExtraIndex < 0)
			{
				int selection = RandomNum<int>(1, m_apTeamTechSelect[team]->GetListPanel()->GetItemList()->size() - 1);
				m_apTeamTechSelect[team]->SetSelectedIndex(selection);
				pTechItem = m_apTeamTechSelect[team]->GetSelectedItem();

				// Switch back to -Random-
				m_apTeamTechSelect[team]->SetSelectedIndex(1);
			}

			// Now set the selected tech's module index as what the metaplayer is going to use
			if (pTechItem)
				if (pTechItem->m_ExtraIndex == -2)
					pGameActivity->SetTeamTech(team, "-All-");
				else
					pGameActivity->SetTeamTech(team, g_PresetMan.GetDataModuleName(pTechItem->m_ExtraIndex));
		}
		
		// Set up AI skill levels
		if (m_apTeamAISkillSlider[team]->IsEnabled())
			pGameActivity->SetTeamAISkill(team, m_apTeamAISkillSlider[team]->GetValue());
		else
			pGameActivity->SetTeamAISkill(team, Activity::DefaultSkill);
	}

	//Force close all previously opened files
	g_LuaMan.FileCloseAll();

    // Put the new and newly set up Activity as the one to start
    g_ActivityMan.SetStartActivity(pActivity);

    // Kill any Campaign games currently running
    if (g_MetaMan.GameInProgress())
	    g_MetaMan.EndGame();

    // Signal the start of this Activity we just set up
    return m_ActivityRestarted = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllScenesAndActivities
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers all the available Scene:s and Activity presets there are.

void ScenarioGUI::GetAllScenesAndActivities()
{
    // Redo the list of Activities
    m_Activities.clear();
	m_pScenes = 0;

    // Get the list of all read in Scene presets
    list<Entity *> presetList;
    g_PresetMan.GetAllOfType(presetList, "Scene");
    list<Scene *> filteredScenes;
    Scene *pScene = 0;

    // Go through the list and cast all the pointers to scenes so we have a handy list
    for (list<Entity *>::iterator pItr = presetList.begin(); pItr != presetList.end(); ++pItr)
    {
        pScene = dynamic_cast<Scene *>(*pItr);
        // Only add non-editor and non-special scenes, or ones that don't have locations defined, or have Test in their names, or are metascenes
		if (pScene && !pScene->GetLocation().IsZero() && !pScene->IsMetagameInternal() && (pScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes()))
			filteredScenes.push_back(pScene);
    }

	//Clear offsets
    for (list<Scene *>::iterator pItr = filteredScenes.begin(); pItr != filteredScenes.end(); ++pItr)
		(*pItr)->SetLocationOffset(Vector(0,0));

	// We need to calculate planet center manually because m_PlanetCenter reflects coords of moving planet
	// which is outside the screen when this is called first time
	Vector planetCenter = Vector(g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2);
	// Correct planet pos a bit when it's location is known
	if (!m_PlanetCenter.IsZero())
		planetCenter = m_PlanetCenter;

	//Move out-of-screen scenes closer to the middle of the planet if we have planet info
	for (list<Scene *>::iterator pItr = filteredScenes.begin(); pItr != filteredScenes.end(); ++pItr)
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
    for (list<Scene *>::iterator pItr = filteredScenes.begin(); pItr != filteredScenes.end(); ++pItr)
    {
		bool isOverlapped = false;

		do 
		{
			isOverlapped = false;

			// Find overlapping scene dot
			for (list<Scene *>::iterator pItr2 = filteredScenes.begin(); pItr2 != filteredScenes.end(); ++pItr2)
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
			{
				if (abs((*pItr)->GetLocation().GetY()) > m_PlanetRadius)
					offsetIncrement.m_Y = -offsetIncrement.m_Y * 2;

				(*pItr)->SetLocationOffset((*pItr)->GetLocationOffset() + offsetIncrement);

			}
		}
		while (isOverlapped);
	}

    // Get the list of all read-in Activity presets
    presetList.clear();
    g_PresetMan.GetAllOfType(presetList, "Activity");
    Activity *pActivity = 0;

	int selectedActivityIndex = m_pActivitySelect->GetSelectedIndex();

    // Go through the list and cast all the pointers to Activities so we have a handy list
    // Associate all Scenes compatible with each Activity
    // Populate the activities selection dropdown while we're at it
    m_pActivitySelect->ClearList();
    int index = 0;
    int tutorialIndex = -1;
    for (list<Entity *>::iterator pItr = presetList.begin(); pItr != presetList.end(); ++pItr)
    {
		bool isMetaActivity = false;

		pActivity = dynamic_cast<Activity *>(*pItr);
		// Only add non-editor and non-special activities
        if (pActivity/* && pActivity->GetClassName() != "GATutorial" */&& pActivity->GetClassName().find("Editor") == string::npos)
        {
            // Prepare a new entry in the list of Activity:ies that we have
            pair<Activity *, list<Scene *> > newPair(pActivity, list<Scene *>());
            for (list<Scene *>::iterator sItr = filteredScenes.begin(); sItr != filteredScenes.end(); ++sItr)
            {
                // Check if the Scene has the required Area:s and such needed for this Activity
                if (pActivity->SceneIsCompatible(*sItr))
                    newPair.second.push_back(*sItr);
            }

            m_Activities.insert(newPair);
            // Add to the activity selection combo, and attach the activity copy, not passing in ownership
            m_pActivitySelect->AddItem(pActivity->GetPresetName(), "", 0, pActivity);

            // Save the tutorial mission so we can select it by default
            if (pActivity->GetClassName() == "GATutorial")
                tutorialIndex = index;
            index++;
        }
    }

    // Select the Tutorial Activity and Scene by default to start
    if (tutorialIndex >= 0)
    {
		if (m_SelectTutorial)
			m_pActivitySelect->SetSelectedIndex(tutorialIndex);
		else
			m_pActivitySelect->SetSelectedIndex(selectedActivityIndex);
        UpdateActivityBox();
        m_pSelectedScene = m_pScenes ? m_pScenes->front() : 0;
        UpdateScenesBox();
		// Switch to tutorial just once
		m_SelectTutorial = false;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSiteHoverLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the floating label over a planet site.

void ScenarioGUI::UpdateSiteNameLabel(bool visible, string text, const Vector &location, float height)
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
// Method:          DrawGlowLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a fancy thick flickering line to point out scene points on the
//                  planet.

void ScenarioGUI::DrawGlowLine(BITMAP *drawBitmap, const Vector &start, const Vector &end, int color) const
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

bool ScenarioGUI::DrawScreenLineToSitePoint(BITMAP *drawBitmap,
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

