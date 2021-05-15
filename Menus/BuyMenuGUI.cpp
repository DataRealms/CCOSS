//////////////////////////////////////////////////////////////////////////////////////////
// File:            BuyMenuGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the BuyMenuGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "BuyMenuGUI.h"

#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "MetaMan.h"
#include "SettingsMan.h"

#include "GUI/GUI.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroInput.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUITab.h"
#include "GUI/GUIListBox.h"
#include "GUI/GUITextBox.h"
#include "GUI/GUIButton.h"
#include "GUI/GUILabel.h"

#include "DataModule.h"
#include "Controller.h"
#include "SceneObject.h"
#include "MovableObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "ACraft.h"

using namespace RTE;

BITMAP *RTE::BuyMenuGUI::s_pCursor = 0;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this BuyMenuGUI, effectively
//                  resetting the members of this abstraction level only.

void BuyMenuGUI::Clear()
{
    m_pController = 0;
    m_pGUIScreen = 0;
    m_pGUIInput = 0;
    m_pGUIController = 0;
    m_MenuEnabled = DISABLED;
    m_MenuFocus = OK;
    m_FocusChange = false;
    m_MenuCategory = CRAFT;
    m_MenuSpeed = 8.0;
    m_ListItemIndex = 0;
    m_LastHoveredMouseIndex = 0;
    m_BlinkTimer.Reset();
    m_BlinkMode = NOBLINK;
    m_MenuTimer.Reset();
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();
    m_pParentBox = 0;
    m_pPopupBox = 0;
    m_pPopupText = 0;
    m_pLogo = 0;
    for (int i = 0; i < CATEGORYCOUNT; ++i)
    {
        m_pCategoryTabs[i] = 0;
        m_CategoryItemIndex[i] = 0;
    }
    m_MetaPlayer = Players::NoPlayer;
    m_NativeTechModule = 0;
    m_ForeignCostMult = 4.0;
    int moduleCount = g_PresetMan.GetTotalModuleCount();
    m_aExpandedModules = new bool[moduleCount];
    for (int i = 0; i < moduleCount; ++i)
        m_aExpandedModules[i] = i == 0 ? true : false;
    m_pShopList = 0;
    m_pCartList = 0;
    m_pCraftBox = 0;

	m_pCraftCollectionBox = 0;
	m_pCraftNameLabel = 0;
	m_pCraftPriceLabel = 0;
	m_pCraftPassengersCaptionLabel = 0;
	m_pCraftPassengersLabel = 0;
	m_pCraftMassCaptionLabel = 0;
	m_pCraftMassLabel = 0;

    m_pSelectedCraft = 0;
    m_pCostLabel = 0;
    m_pBuyButton = 0;
    m_pSaveButton = 0;
    m_pClearButton = 0;
    m_Loadouts.clear();
    m_PurchaseMade = false;
    m_CursorPos.Reset();

	m_EnforceMaxPassengersConstraint = true;
	m_EnforceMaxMassConstraint = true;

	m_OnlyShowOwnedItems = false;
	m_AllowedItems.clear();
	m_AlwaysAllowedItems.clear();
	m_OwnedItems.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BuyMenuGUI object ready for use.

int BuyMenuGUI::Create(Controller *pController)
{
    RTEAssert(pController, "No controller sent to BuyMenyGUI on creation!");
    m_pController = pController;

    if (!m_pGUIScreen)
        m_pGUIScreen = new AllegroScreen(g_FrameMan.GetNetworkBackBufferGUI8Current(pController->GetPlayer()));
    if (!m_pGUIInput)
        m_pGUIInput = new AllegroInput(pController->GetPlayer()); 
    if (!m_pGUIController)
        m_pGUIController = new GUIControlManager();
    if(!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins/Base"))
        RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Base");
    m_pGUIController->Load("Base.rte/GUIs/BuyMenuGUI.ini");
    m_pGUIController->EnableMouse(pController->IsMouseControlled());

    if (!s_pCursor)
    {
        ContentFile cursorFile("Base.rte/GUIs/Skins/Cursor.png");
        s_pCursor = cursorFile.GetAsBitmap();
    }

    // Stretch the invisible root box to fill the screen
	if (g_FrameMan.IsInMultiplayerMode())
	{
		dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetPlayerFrameBufferWidth(pController->GetPlayer()), g_FrameMan.GetPlayerFrameBufferHeight(pController->GetPlayer()));
	} 
	else
	{
		dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("base"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
	}

    // Make sure we have convenient points to teh containing GUI colleciton boxes that we will manipulate the positions of
    if (!m_pParentBox)
    {
        m_pParentBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BuyGUIBox"));

        // Set the background settings of the parent collection box
	    m_pParentBox->SetDrawBackground(true);
        m_pParentBox->SetDrawType(GUICollectionBox::Color);

        // Set the images for the logo and header decorations
        GUICollectionBox *pHeader = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CatalogHeader"));
        m_pLogo = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CatalogLogo"));
        ContentFile headerFile("Base.rte/GUIs/Skins/BuyMenu/BuyMenuHeader.png");
        ContentFile logoFile("Base.rte/GUIs/Skins/BuyMenu/BuyMenuLogo.png");
        pHeader->SetDrawImage(new AllegroBitmap(headerFile.GetAsBitmap()));
        m_pLogo->SetDrawImage(new AllegroBitmap(logoFile.GetAsBitmap()));
        pHeader->SetDrawType(GUICollectionBox::Image);
        m_pLogo->SetDrawType(GUICollectionBox::Image);
    }
    m_pParentBox->SetPositionAbs(-m_pParentBox->GetWidth(), 0);
    m_pParentBox->SetEnabled(false);
    m_pParentBox->SetVisible(false);

    if (!m_pPopupBox)
    {
        m_pPopupBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("BuyGUIPopup"));
        m_pPopupText = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("PopupText"));

        m_pPopupBox->SetDrawType(GUICollectionBox::Panel);
	    m_pPopupBox->SetDrawBackground(true);
        // Never enable the popup, because it steals focus and cuases other windows to think teh cursor left them
        m_pPopupBox->SetEnabled(false);
        m_pPopupBox->SetVisible(false);
        // Set the font
        m_pPopupText->SetFont(m_pGUIController->GetSkin()->GetFont("smallfont.png"));
    }

    m_pCategoryTabs[CRAFT] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("CraftTab"));
    m_pCategoryTabs[BODIES] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("BodiesTab"));
    m_pCategoryTabs[TOOLS] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("ToolsTab"));
    m_pCategoryTabs[GUNS] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("GunsTab"));
    m_pCategoryTabs[BOMBS] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("BombsTab"));
    m_pCategoryTabs[SHIELDS] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("ShieldsTab"));
    m_pCategoryTabs[SETS] = dynamic_cast<GUITab *>(m_pGUIController->GetControl("SetsTab"));

    m_pShopList = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("CatalogLB"));
    m_pCartList = dynamic_cast<GUIListBox *>(m_pGUIController->GetControl("OrderLB"));
    m_pCraftLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftLabel"));
    m_pCraftBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("CraftTB"));

	m_pCraftCollectionBox = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CraftCollection"));
	m_pCraftNameLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftNameLabel"));
	m_pCraftPriceLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftPriceLabel"));
	m_pCraftPassengersCaptionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftPassengersCaptionLabel"));
	m_pCraftPassengersLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftPassengersLabel"));
	m_pCraftMassCaptionLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftMassCaptionLabel"));
	m_pCraftMassLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("CraftMassLabel"));

    m_pCostLabel = dynamic_cast<GUILabel *>(m_pGUIController->GetControl("TotalLabel"));
    m_pBuyButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("BuyButton"));
    m_pSaveButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("SaveButton"));
    m_pClearButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ClearButton"));
    m_pSaveButton->SetVisible(false);
    m_pClearButton->SetVisible(false);

	// Stretch buy menu if in multiplayer mode
	if (g_FrameMan.IsInMultiplayerMode())
	{
		int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(pController->GetPlayer()) / 2;

		m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
		m_pShopList->SetSize(m_pShopList->GetWidth(), m_pShopList->GetHeight() + stretchAmount);
		m_pCartList->SetSize(m_pCartList->GetWidth(), m_pCartList->GetHeight() + stretchAmount);
		m_pCraftLabel->SetPositionAbs(m_pCraftLabel->GetXPos(), m_pCraftLabel->GetYPos() + stretchAmount);
		m_pCraftBox->SetPositionAbs(m_pCraftBox->GetXPos(), m_pCraftBox->GetYPos() + stretchAmount);

		m_pCraftCollectionBox->SetPositionAbs(m_pCraftCollectionBox->GetXPos(), m_pCraftCollectionBox->GetYPos() + stretchAmount);

		m_pCostLabel->SetPositionAbs(m_pCostLabel->GetXPos(), m_pCostLabel->GetYPos() + stretchAmount);
		m_pBuyButton->SetPositionAbs(m_pBuyButton->GetXPos(), m_pBuyButton->GetYPos() + stretchAmount);
	}
	else
	{
		// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
		int stretchAmount = g_FrameMan.GetResY() / 2;

		if (!g_FrameMan.GetHSplit())
		{
			m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
			m_pShopList->SetSize(m_pShopList->GetWidth(), m_pShopList->GetHeight() + stretchAmount);
			m_pCartList->SetSize(m_pCartList->GetWidth(), m_pCartList->GetHeight() + stretchAmount);
			m_pCraftLabel->SetPositionAbs(m_pCraftLabel->GetXPos(), m_pCraftLabel->GetYPos() + stretchAmount);
			m_pCraftBox->SetPositionAbs(m_pCraftBox->GetXPos(), m_pCraftBox->GetYPos() + stretchAmount);

			m_pCraftCollectionBox->SetPositionAbs(m_pCraftCollectionBox->GetXPos(), m_pCraftCollectionBox->GetYPos() + stretchAmount);

			m_pCostLabel->SetPositionAbs(m_pCostLabel->GetXPos(), m_pCostLabel->GetYPos() + stretchAmount);
			m_pBuyButton->SetPositionAbs(m_pBuyButton->GetXPos(), m_pBuyButton->GetYPos() + stretchAmount);
		}
	}

    m_pShopList->SetAlternateDrawMode(true);
    m_pCartList->SetAlternateDrawMode(true);
    m_pShopList->SetMultiSelect(false);
    m_pCartList->SetMultiSelect(false);
// Do this manually with the MoseMoved notifications
//    m_pShopList->SetHotTracking(true);
//    m_pCartList->SetHotTracking(true);
    m_pCraftBox->SetLocked(true);
    m_pShopList->EnableScrollbars(false, true);
    m_pCartList->EnableScrollbars(false, true);

    // Load the loadouts initially.. this might be done again later as well by Activity scripts after they set metaplayer etc
    LoadAllLoadoutsFromFile();

    // Set initial focus, category list, and label settings
    m_MenuFocus = OK;
    m_FocusChange = true;
    m_MenuCategory = SETS;
    CategoryChange();
    UpdateTotalCostLabel(m_pController->GetTeam());

	UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
	UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);

    // Reset repeat timers
    m_RepeatStartTimer.Reset();
    m_RepeatTimer.Reset();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the BuyMenuGUI object.

void BuyMenuGUI::Destroy()
{
    delete m_pGUIController;
    delete m_pGUIInput;
    delete m_pGUIScreen;

    delete [] m_aExpandedModules;

    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetHeaderImage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes the header image to the one specified in path

void BuyMenuGUI::SetHeaderImage(string path)
{
	GUICollectionBox *pHeader = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CatalogHeader"));
	ContentFile headerFile(path.c_str());
	pHeader->SetDrawImage(new AllegroBitmap(headerFile.GetAsBitmap()));
	pHeader->SetDrawType(GUICollectionBox::Image);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetLogoImage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes the logo image to the one specified in path

void BuyMenuGUI::SetLogoImage(string path)
{
	m_pLogo = dynamic_cast<GUICollectionBox *>(m_pGUIController->GetControl("CatalogLogo"));
	ContentFile logoFile(path.c_str());
	m_pLogo->SetDrawImage(new AllegroBitmap(logoFile.GetAsBitmap()));
	m_pLogo->SetDrawType(GUICollectionBox::Image);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearCartList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clear the cart out of items selected for purchase

void BuyMenuGUI::ClearCartList()
{
	m_pCartList->ClearList();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  LoadAllLoadoutsFromFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads or re-loads all the loadout presets from the appropriate files
//                  on disk. This will first clear out all current loadout presets!

bool BuyMenuGUI::LoadAllLoadoutsFromFile()
{
    // First clear out all loadouts
    m_Loadouts.clear();
    // Try to load the player's loadout settings from file, if there is one
    char loadoutPath[256];

    // A metagame player
    if (m_MetaPlayer != Players::NoPlayer)
    {
        // Start loading any additional stuff from the custom user file
        std::snprintf(loadoutPath, sizeof(loadoutPath), "Metagames.rte/%s - LoadoutsMP%d.ini", g_MetaMan.GetGameName().c_str(), m_MetaPlayer + 1);

        if (!exists(loadoutPath))
        {
            // If the file doesn't exist, then we're not loading it, are we?
            loadoutPath[0] = 0;
        }
    }
    // Not a metagame player, just a regular scenario player
    else
	{
        std::snprintf(loadoutPath, sizeof(loadoutPath), "Base.rte/LoadoutsP%d.ini", m_pController->GetPlayer() + 1);

	}

    // Open the file
    Reader loadoutFile(loadoutPath, false, 0, true);

    // Read any and all loadout presets from file
    while (loadoutFile.ReaderOK() && loadoutFile.NextProperty())
    {
        Loadout newLoad;
        loadoutFile >> newLoad;
        // If we successfully found everything this loadout requires, add it to the preset menu
// Why be picky?
        if (!newLoad.GetCargoList()->empty())//newLoad.IsComplete())
            m_Loadouts.push_back(newLoad);
    }

	if (m_NativeTechModule > 0)
	{
		// Then try to get the different standard Loadouts for this' player's native tech module if it's not -All-
		const Loadout *pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Default", m_NativeTechModule));
		// Add it to the Loadout list - it will be copied inside so no worry about passing in a preset instance
		if (pDefaultLoadoutPreset)
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		// Attempt to do it for all the other standard loadout types as well
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Light", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Heavy", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry CQB", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Grenadier", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Sniper", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Infantry Engineer", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Mecha", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
		if (pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Turret", m_NativeTechModule)))
			m_Loadouts.push_back(*pDefaultLoadoutPreset);
	}

    // If no file was found, try to load a Tech module-specified loadout defaults!
    if (m_Loadouts.empty())
    {
        // Try to get the default Loadout for this' player's native tech module
        const Loadout *pDefaultLoadoutPreset = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", "Default", m_NativeTechModule));
        // Add it to the Loadout list - it will be copied inside so no worry about passing in a preset instance
        if (pDefaultLoadoutPreset)
            m_Loadouts.push_back(*pDefaultLoadoutPreset);
    }
/* This is dangerous, crash prone and unneccessary
    // If there were no loadouts to load, or no file present, or default Presets defined, set up a single failsafe default one
    if (m_Loadouts.empty())
    {
        Loadout defaultLoadout;
        // Default craft
        defaultLoadout.SetDeliveryCraft(dynamic_cast<const ACraft *>(g_PresetMan.GetEntityPreset("ACRocket", "Rocket MK1")));
        // Default passenger
        defaultLoadout.AddToCargoList(dynamic_cast<const AHuman *>(g_PresetMan.GetEntityPreset("AHuman", "Robot 1")));
        // Default primary weapon
        defaultLoadout.AddToCargoList(dynamic_cast<const MOSprite *>(g_PresetMan.GetEntityPreset("HDFirearm", "SMG")));
        // Default tool
        defaultLoadout.AddToCargoList(dynamic_cast<const MOSprite *>(g_PresetMan.GetEntityPreset("HDFirearm", "Medium Digger")));
        // Add to list
        if (defaultLoadout.IsComplete())
            m_Loadouts.push_back(defaultLoadout);
    }
*/
    // Load the first loadout preset into the cart by default
    DeployLoadout(0);

    // Refresh all views so we see the new sets if we're in the preset category
    CategoryChange();

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SaveAllLoadoutsToFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves all the loadouts to appropriate file on disk. Does NOT save
//                  any named presets which will be loaded from the standard preset
//                  loadouts first anyway.

bool BuyMenuGUI::SaveAllLoadoutsToFile()
{
    // Nothing to save
    if (m_Loadouts.empty())
        return true;

    char loadoutPath[256];
    // A metagame player
    if (m_MetaPlayer != Players::NoPlayer)
    {
        // If a new metagame, then just save over the metagame autosave instead of to the new game save
        // Since the players of a new game are likely to have different techs and therefore different default loadouts
        // So we should start fresh with new loadouts loaded from tech defaults for each player
        if (g_MetaMan.GetGameName() == DEFAULTGAMENAME)
            std::snprintf(loadoutPath, sizeof(loadoutPath), "Metagames.rte/%s - LoadoutsMP%d.ini", AUTOSAVENAME, m_MetaPlayer + 1);
        else
            std::snprintf(loadoutPath, sizeof(loadoutPath), "Metagames.rte/%s - LoadoutsMP%d.ini", g_MetaMan.GetGameName().c_str(), m_MetaPlayer + 1);
    }
    else
        std::snprintf(loadoutPath, sizeof(loadoutPath), "Base.rte/LoadoutsP%d.ini", m_pController->GetPlayer() + 1);

    // Open the file
    Writer loadoutFile(loadoutPath, false);

	// Write out all the loadouts that are user made.
	for (const Loadout &loadoutEntry : m_Loadouts) {
		// Don't write out preset references, they'll be read first from PresetMan on load anyway
		if (loadoutEntry.GetPresetName() == "None") { loadoutFile.NewPropertyWithValue("AddLoadout", loadoutEntry); }
	}

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.

void BuyMenuGUI::SetEnabled(bool enable)
{
    if (enable && m_MenuEnabled != ENABLED && m_MenuEnabled != ENABLING)
    {
		if (g_FrameMan.IsInMultiplayerMode())
		{
			// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
			int stretchAmount = g_FrameMan.GetPlayerFrameBufferHeight(m_pController->GetPlayer()) - m_pParentBox->GetHeight();
			if (stretchAmount != 0)
			{
				m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
				m_pShopList->SetSize(m_pShopList->GetWidth(), m_pShopList->GetHeight() + stretchAmount);
				m_pCartList->SetSize(m_pCartList->GetWidth(), m_pCartList->GetHeight() + stretchAmount);
				m_pCraftLabel->SetPositionAbs(m_pCraftLabel->GetXPos(), m_pCraftLabel->GetYPos() + stretchAmount);
				m_pCraftBox->SetPositionAbs(m_pCraftBox->GetXPos(), m_pCraftBox->GetYPos() + stretchAmount);

				m_pCraftCollectionBox->SetPositionAbs(m_pCraftCollectionBox->GetXPos(), m_pCraftCollectionBox->GetYPos() + stretchAmount);

				m_pCostLabel->SetPositionAbs(m_pCostLabel->GetXPos(), m_pCostLabel->GetYPos() + stretchAmount);
				m_pBuyButton->SetPositionAbs(m_pBuyButton->GetXPos(), m_pBuyButton->GetYPos() + stretchAmount);
			}
		}
		else
		{
			// If we're not split screen horizontally, then stretch out the layout for all the relevant controls
			int stretchAmount = g_FrameMan.GetPlayerScreenHeight() - m_pParentBox->GetHeight();
			if (stretchAmount != 0)
			{
				m_pParentBox->SetSize(m_pParentBox->GetWidth(), m_pParentBox->GetHeight() + stretchAmount);
				m_pShopList->SetSize(m_pShopList->GetWidth(), m_pShopList->GetHeight() + stretchAmount);
				m_pCartList->SetSize(m_pCartList->GetWidth(), m_pCartList->GetHeight() + stretchAmount);
				m_pCraftLabel->SetPositionAbs(m_pCraftLabel->GetXPos(), m_pCraftLabel->GetYPos() + stretchAmount);
				m_pCraftBox->SetPositionAbs(m_pCraftBox->GetXPos(), m_pCraftBox->GetYPos() + stretchAmount);

				m_pCraftCollectionBox->SetPositionAbs(m_pCraftCollectionBox->GetXPos(), m_pCraftCollectionBox->GetYPos() + stretchAmount);

				m_pCostLabel->SetPositionAbs(m_pCostLabel->GetXPos(), m_pCostLabel->GetYPos() + stretchAmount);
				m_pBuyButton->SetPositionAbs(m_pBuyButton->GetXPos(), m_pBuyButton->GetYPos() + stretchAmount);
			}
		}

        m_MenuEnabled = ENABLING;
        // Reset repeat timers
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
        // Set the mouse cursor free
        g_UInputMan.TrapMousePos(false, m_pController->GetPlayer());
        // Move the mouse cursor to the middle of the player's screen
        int mouseOffX, mouseOffY;
        m_pGUIInput->GetMouseOffset(mouseOffX, mouseOffY);
        Vector mousePos(-mouseOffX + (g_FrameMan.GetPlayerFrameBufferWidth(m_pController->GetPlayer()) / 2), -mouseOffY + (g_FrameMan.GetPlayerFrameBufferHeight(m_pController->GetPlayer()) / 2));
        g_UInputMan.SetMousePos(mousePos, m_pController->GetPlayer());

        // Default focus to the menu button
        m_MenuFocus = OK;
        m_FocusChange = true;
        UpdateTotalCostLabel(m_pController->GetTeam());

		UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
		UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);

        g_GUISound.EnterMenuSound()->Play(m_pController->GetPlayer());
    }
    else if (!enable && m_MenuEnabled != DISABLED && m_MenuEnabled != DISABLING)
    {
        m_MenuEnabled = DISABLING;
        // Trap the mouse cursor again
        g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());
        // Only play switching away sound
//        if (!m_PurchaseMade)
            g_GUISound.ExitMenuSound()->Play(m_pController->GetPlayer());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void BuyMenuGUI::SetPosOnScreen(int newPosX, int newPosY)
{
    m_pGUIController->SetPosOnScreen(newPosX, newPosY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which MetaPlayer uses this menu, if any.

void BuyMenuGUI::SetMetaPlayer(int metaPlayer)
{
    if (metaPlayer >= Players::PlayerOne && metaPlayer < g_MetaMan.GetPlayerCount())
    {
        m_MetaPlayer = metaPlayer;
        SetNativeTechModule(g_MetaMan.GetPlayer(m_MetaPlayer)->GetNativeTechModule());
        SetForeignCostMultiplier(g_MetaMan.GetPlayer(m_MetaPlayer)->GetForeignCostMultiplier());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNativeTechModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule ID should be treated as the native tech of the
//                  user of this menu.

void BuyMenuGUI::SetNativeTechModule(int whichModule)
{
    if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount())
    {
        // Set the multipliers and refresh everything that needs refreshing to reflect the change
        m_NativeTechModule = whichModule;
        SetModuleExpanded(m_NativeTechModule);
        DeployLoadout(0);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleExpanded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether a data module shown in the item menu should be expanded
//                  or not.

void BuyMenuGUI::SetModuleExpanded(int whichModule, bool expanded)
{
    int moduleCount = g_PresetMan.GetTotalModuleCount();
    if (whichModule > 0 && whichModule < moduleCount)
    {
        m_aExpandedModules[whichModule] = expanded;
        // Refresh the item view with the newly expanded module items
        CategoryChange(false);
    }
    // If base module (0), or out of range module, then affect all
    else
    {
        for (int m = 0; m < moduleCount; ++m)
            m_aExpandedModules[m] = expanded;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOrderList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return the list of things currently in the purchase order list box.

bool BuyMenuGUI::GetOrderList(list<const SceneObject *> &listToFill)
{
    if (m_pCartList->GetItemList()->empty())
        return false;

    const SceneObject *pSObject = 0;
    for (vector<GUIListPanel::Item *>::iterator itr = m_pCartList->GetItemList()->begin(); itr != m_pCartList->GetItemList()->end(); ++itr)
    {
        if (pSObject = dynamic_cast<const SceneObject *>((*itr)->m_pEntity))
            listToFill.push_back(pSObject);
    }

    return true;
}

bool BuyMenuGUI::CommitPurchase(string presetName)
{
	if (m_OwnedItems.size() > 0)
	{
		if (m_OwnedItems.find(presetName) != m_OwnedItems.end() && m_OwnedItems[presetName] > 0)
		{
			m_OwnedItems[presetName] -= 1;
			return true;
		}
		else
			return false;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalOrderCost
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return teh total cost of everything listed in the order box.

float BuyMenuGUI::GetTotalOrderCost()
{
	float totalCost = 0;

	if (m_OwnedItems.size() > 0)
	{
		map<string, int> orderedItems;

		for (vector<GUIListPanel::Item *>::iterator itr = m_pCartList->GetItemList()->begin(); itr != m_pCartList->GetItemList()->end(); ++itr)
		{
			bool needsToBePaid = true;
			string presetName = (*itr)->m_pEntity->GetModuleAndPresetName();

			if (orderedItems.find(presetName) != orderedItems.end())
				orderedItems[presetName] = 1;
			else
				orderedItems[presetName] += 1;

			if (m_OwnedItems.find(presetName) != m_OwnedItems.end() && m_OwnedItems[presetName] >= orderedItems[presetName])
				needsToBePaid = false;

			if (needsToBePaid)
			{
				totalCost += dynamic_cast<const MOSprite *>((*itr)->m_pEntity)->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);
			}
		}

		if (m_pSelectedCraft)
		{
			bool needsToBePaid = true;
			string presetName = m_pSelectedCraft->GetModuleAndPresetName();

			if (orderedItems.find(presetName) != orderedItems.end())
				orderedItems[presetName] = 1;
			else
				orderedItems[presetName] += 1;

			if (m_OwnedItems.find(presetName) != m_OwnedItems.end() && m_OwnedItems[presetName] >= orderedItems[presetName])
				needsToBePaid = false;

			if (needsToBePaid)
			{
				totalCost += dynamic_cast<const MOSprite *>(m_pSelectedCraft)->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);
			}
		}
	}
	else 
	{
		for (vector<GUIListPanel::Item *>::iterator itr = m_pCartList->GetItemList()->begin(); itr != m_pCartList->GetItemList()->end(); ++itr)
			totalCost += dynamic_cast<const MOSprite *>((*itr)->m_pEntity)->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);

		// Add the delivery craft's cost
		if (m_pSelectedCraft)
		{
			totalCost += dynamic_cast<const MOSprite *>(m_pSelectedCraft)->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);
		}
	}

    return totalCost;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float BuyMenuGUI::GetTotalOrderMass() const {
	float totalMass = 0.0F;

	for (const GUIListPanel::Item *cartItem : *m_pCartList->GetItemList()) {
		const MovableObject *itemAsMO = dynamic_cast<const MovableObject*>(cartItem->m_pEntity);
		if (itemAsMO) {
			totalMass += itemAsMO->GetMass();
		} else {
			RTEAbort("Found a non-MO object in the cart and tried to add it's mass to order total!");
		}
	}

	return totalMass;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float BuyMenuGUI::GetCraftMass() {
	float totalMass = 0;

	// Add the delivery craft's mass
	if (m_pSelectedCraft)
		totalMass += dynamic_cast<const MOSprite *>(m_pSelectedCraft)->GetMass();

	return totalMass;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalOrderPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return the total number of passengers in the order box.
// Arguments:       None.
// Return value:    The total number of passengers.

int BuyMenuGUI::GetTotalOrderPassengers() const {
	int passengers = 0;
	for (vector<GUIListPanel::Item *>::iterator itr = m_pCartList->GetItemList()->begin(); itr != m_pCartList->GetItemList()->end(); ++itr)
	{
        const Actor* passenger = dynamic_cast<const Actor*>((*itr)->m_pEntity);
        if (passenger)
        {
            passengers += passenger->GetPassengerSlots();
        }
	}

	return passengers;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void BuyMenuGUI::Update()
{
    // Enable mouse input if the controller allows it
    m_pGUIController->EnableMouse(m_pController->IsMouseControlled());

    // Reset the purchasing indicator
    m_PurchaseMade = false;

    // Popup box is hidden by default
    m_pPopupBox->SetVisible(false);

    ////////////////////////////////////////////////////////////////////////
    // Animate the menu into and out of view if enabled or disabled

    if (m_MenuEnabled == ENABLING)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);

        Vector position, occlusion;

        float toGo = -std::floor((float)m_pParentBox->GetXPos());
        float goProgress = m_MenuSpeed * m_MenuTimer.GetElapsedRealTimeS();
        if (goProgress > 1.0)
            goProgress = 1.0;
        position.m_X = m_pParentBox->GetXPos() + std::ceil(toGo * goProgress);
        occlusion.m_X = m_pParentBox->GetWidth() + m_pParentBox->GetXPos();

        // If not split screened, then make the menu scroll in diagonally instead of straight from the side
        // Tie it to the (X) horizontal position
        // EDIT: nah, just make the menu larger, but do change the occlusion, looks better
        if (!g_FrameMan.GetHSplit())
        {
//            position.m_Y = -m_pParentBox->GetHeight() * fabs(position.m_X / m_pParentBox->GetWidth());
//            occlusion.m_Y = m_pParentBox->GetHeight() + m_pParentBox->GetYPos();
            occlusion.m_Y = m_pParentBox->GetHeight() / 2;
        }

        m_pParentBox->SetPositionAbs(position.m_X, position.m_Y);
        g_SceneMan.SetScreenOcclusion(occlusion, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

		if (m_pParentBox->GetXPos() >= 0)
		{
			m_MenuEnabled = ENABLED;
			CategoryChange();
		}
    }
    // Animate the menu out of view
    else if (m_MenuEnabled == DISABLING)
    {
        float toGo = -std::ceil(((float)m_pParentBox->GetWidth() + (float)m_pParentBox->GetXPos()));
        float goProgress = m_MenuSpeed * m_MenuTimer.GetElapsedRealTimeS();
        if (goProgress > 1.0)
            goProgress = 1.0;
        m_pParentBox->SetPositionAbs(m_pParentBox->GetXPos() + std::floor(toGo * goProgress), 0);
        g_SceneMan.SetScreenOcclusion(Vector(m_pParentBox->GetWidth() + m_pParentBox->GetXPos(), 0), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
        m_pPopupBox->SetVisible(false);

        if (m_pParentBox->GetXPos() <= -m_pParentBox->GetWidth())
        {
            m_pParentBox->SetEnabled(false);
            m_pParentBox->SetVisible(false);
            m_MenuEnabled = DISABLED;
        }
    }
    else if (m_MenuEnabled == ENABLED)
    {
        m_pParentBox->SetEnabled(true);
        m_pParentBox->SetVisible(true);

    }
    else if (m_MenuEnabled == DISABLED)
    {
        m_pParentBox->SetEnabled(false);
        m_pParentBox->SetVisible(false);
        m_pPopupBox->SetVisible(false);
    }

    // Reset the menu animation timer so we can measure how long it takes till next frame
    m_MenuTimer.Reset();

    // Quit now if we aren't enabled
    if (m_MenuEnabled != ENABLED &&  m_MenuEnabled != ENABLING)
        return;

    // Update the user controller
//    m_pController->Update();

    // Gotto update this all the time because other players may have bought stuff and changed the funds available to the team
    UpdateTotalCostLabel(m_pController->GetTeam());

	UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
	UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);

    /////////////////////////////////////////////////////
    // Mouse cursor logic

    int mouseX, mouseY;
    m_pGUIInput->GetMousePosition(&mouseX, &mouseY);
    m_CursorPos.SetXY(mouseX, mouseY);

    ////////////////////////////////////////////
    // Notification blinking logic

    if (m_BlinkMode == NOFUNDS)
    {
        m_pCostLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
    }
    else if (m_BlinkMode == NOCRAFT)
    {
        bool blink = m_BlinkTimer.AlternateSim(250);
        m_pCraftLabel->SetVisible(blink);
        m_pCraftBox->SetVisible(blink);
		m_pCraftCollectionBox->SetVisible(blink);
	}
	else if (m_BlinkMode == MAXMASS)
	{
		m_pCraftMassLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
		m_pCraftMassCaptionLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
	}
	else if (m_BlinkMode == MAXPASSENGERS)
	{
		m_pCraftPassengersLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
		m_pCraftPassengersCaptionLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
	}

    // Time out the blinker
    if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastSimMS(1500))
    {
        m_pCostLabel->SetVisible(true);
        m_pCraftLabel->SetVisible(true);
        m_pCraftBox->SetVisible(true);
		m_pCraftCollectionBox->SetVisible(true);
		m_pCraftMassCaptionLabel->SetVisible(true);
		m_pCraftMassLabel->SetVisible(true);
		m_pCraftPassengersCaptionLabel->SetVisible(true);
		m_pCraftPassengersLabel->SetVisible(true);
		m_BlinkMode = NOBLINK;
    }

    /////////////////////////////////////////////
    // Repeating input logic

    bool pressLeft = m_pController->IsState(PRESS_LEFT);
    bool pressRight = m_pController->IsState(PRESS_RIGHT);
    bool pressUp = m_pController->IsState(PRESS_UP) || m_pController->IsState(SCROLL_UP);
    bool pressDown = m_pController->IsState(PRESS_DOWN) || m_pController->IsState(SCROLL_DOWN);

    // If no direciton is held down, then cancel the repeating
    if (!(m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN)))
    {
        m_RepeatStartTimer.Reset();
        m_RepeatTimer.Reset();
    }

    // Check if any direction has been held for the starting amount of time to get into repeat mode
    if (m_RepeatStartTimer.IsPastRealMS(200))
    {
        // Check for the repeat interval
        if (m_RepeatTimer.IsPastRealMS(75))
        {
            if (m_pController->IsState(MOVE_RIGHT))
                pressRight = true;
            else if (m_pController->IsState(MOVE_LEFT))
                pressLeft = true;
            else if (m_pController->IsState(MOVE_UP))
                pressUp = true;
            else if (m_pController->IsState(MOVE_DOWN))
                pressDown = true;

            m_RepeatTimer.Reset();
        }
    }

    /////////////////////////////////////////////
    // Change focus as the user directs

    if (pressRight)
    {
        m_MenuFocus++;
        m_FocusChange = 1;

        // Went too far
        if (m_MenuFocus >= FOCUSCOUNT)
        {
            m_MenuFocus = FOCUSCOUNT - 1;
            m_FocusChange = 0;
            g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        }
        // Skip categories if we're going sideways from the sets buttons
        if (m_MenuFocus == CATEGORIES)
        {
            m_MenuFocus++;
            m_FocusChange++;
        }
        // Skip giving focus to the items list if it's empty
        if (m_MenuFocus == ITEMS && m_pShopList->GetItemList()->empty())
        {
            m_MenuFocus++;
            m_FocusChange++;
        }
        // Skip giving focus to the order list if it's empty
        if (m_MenuFocus == ORDER && m_pCartList->GetItemList()->empty())
        {
            m_MenuFocus++;
            m_FocusChange++;
        }
    }
    else if (pressLeft)
    {
        m_MenuFocus--;
        m_FocusChange = -1;

        // Went too far
        if (m_MenuFocus < 0)
        {
            m_MenuFocus = 0;
            g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        }
        // Skip giving focus to the order or item list if they're empty
        if (m_MenuFocus == ORDER && m_pCartList->GetItemList()->empty())
        {
            m_MenuFocus--;
            m_FocusChange--;
        }
        // Skip giving focus to the items list if it's empty
        if (m_MenuFocus == ITEMS && m_pShopList->GetItemList()->empty())
        {
            m_MenuFocus--;
            m_FocusChange--;
        }
    }
    // Play focus change sound, if applicable
    if (m_FocusChange && m_MenuEnabled != ENABLING)
        g_GUISound.FocusChangeSound()->Play(m_pController->GetPlayer());
/* Blah, should control whatever is currently focused
    // Mouse wheel only controls the categories, so switch to it and make the category go up or down
    if (m_pController->IsState(SCROLL_UP) || m_pController->IsState(SCROLL_DOWN))
    {
        m_FocusChange = CATEGORIES - m_MenuFocus;
        m_MenuFocus = CATEGORIES;
    }
*/

    /////////////////////////////////////////
    // SETS BUTTONS focus

    if (m_MenuFocus == SETBUTTONS)
    {
        if (m_FocusChange)
        {
            // Set the correct special Sets category so the sets buttons show up
            m_MenuCategory = SETS;
            CategoryChange();
            m_pSaveButton->SetFocus();
            m_FocusChange = 0;
        }

        if (m_pController->IsState(PRESS_FACEBUTTON))
        {
            if (m_pSaveButton->HasFocus())
                SaveCurrentLoadout();
			else if (m_pClearButton->HasFocus() && m_Loadouts.size() != 0)
            {
                m_Loadouts.pop_back();
                // Update the list of loadout presets so the removal shows up
                CategoryChange();
                // Set focus back on the save button (CatChange changed it)
                m_pClearButton->SetFocus();
            }
            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
        }

        // Switch back focus to the category list if the player presses up while on the save button
        if (pressUp)
        {
            if (m_pSaveButton->HasFocus())
            {
                m_MenuFocus = CATEGORIES;
                m_FocusChange = 1;
            }
            else if (m_pClearButton->HasFocus())
            {
                m_pSaveButton->SetFocus();
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
        }
        else if (pressDown)
        {
            if (m_pSaveButton->HasFocus())
            {
                m_pClearButton->SetFocus();
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
            else if (m_pClearButton->HasFocus())
                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        }
    }

    /////////////////////////////////////////
    // CATEGORIES focus

    else if (m_MenuFocus == CATEGORIES)
    {
        if (m_FocusChange)
        {
            m_pCategoryTabs[m_MenuCategory]->SetFocus();
            m_FocusChange = 0;
        }

        if (pressDown)
        {
            m_MenuCategory++;
            if (m_MenuCategory >= CATEGORYCOUNT)
            {
                m_MenuCategory = CATEGORYCOUNT - 1;
                // Go to the preset buttons if hit down on the last one
                m_MenuFocus = SETBUTTONS;
                m_FocusChange = -1;
//                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
            }
/*
            // Loop Around
            if (m_MenuCategory >= CATEGORYCOUNT)
                m_MenuCategory = 0;
*/
            else
            {
                CategoryChange();
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
        }
        else if (pressUp)
        {
            m_MenuCategory--;
            if (m_MenuCategory < 0)
            {
                m_MenuCategory = 0;
                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
            }
/*
            // Loop around
            if (m_MenuCategory < 0)
                m_MenuCategory = CATEGORYCOUNT - 1;
*/
            else
            {
                CategoryChange();
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
        }
    }

    /////////////////////////////////////////
    // ITEMS LIST focus

    else if (m_MenuFocus == ITEMS)
    {
        if (m_FocusChange)
        {
            m_pShopList->SetFocus();
            // Select the top one in the item list if none is already selected
            if (!m_pShopList->GetItemList()->empty() && m_pShopList->GetSelectedIndex() < 0)
                m_pShopList->SetSelectedIndex(m_ListItemIndex = 0);
            // Synch our index with the one already sleected in the list
            else
            {
                m_ListItemIndex = m_pShopList->GetSelectedIndex();
                m_pShopList->ScrollToSelected();
            }

            m_FocusChange = 0;
        }

        int listSize = m_pShopList->GetItemList()->size();
        if (pressDown)
        {
            m_ListItemIndex++;
            // Loop around
            if (m_ListItemIndex >= listSize)
                m_ListItemIndex = 0;

            // Update the selected shop item index
            m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex;
            m_pShopList->SetSelectedIndex(m_ListItemIndex);
            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }
        else if (pressUp)
        {
            m_ListItemIndex--;
            // Loop around
            if (m_ListItemIndex < 0)
                m_ListItemIndex = listSize - 1;

            // Update the selected shop item index
            m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex;
            m_pShopList->SetSelectedIndex(m_ListItemIndex);
            g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
        }

        // Get handle to the currently selected item, if any
        GUIListPanel::Item *pItem = m_pShopList->GetItem(m_ListItemIndex);
        std::string description = "";

        if (pItem && pItem->m_pEntity) {
			description = ((pItem->m_pEntity->GetDescription().empty()) ? "-No Information Found-": pItem->m_pEntity->GetDescription()) + "\n";
            const Entity *currentItem = pItem->m_pEntity;
            const ACraft *itemAsCraft = dynamic_cast<const ACraft *>(currentItem);
            if (itemAsCraft) {
                int craftMaxPassengers = itemAsCraft->GetMaxPassengers();
                float craftMaxMass = itemAsCraft->GetMaxInventoryMass();
                if (craftMaxMass == 0) {
                    description += "\nNO CARGO SPACE!";
                } else if (craftMaxMass > 0) {
                    description += "\nMax Mass: " + RoundFloatToPrecision(craftMaxMass, 1) + " kg";
                }
                if (craftMaxPassengers >= 0 && craftMaxMass != 0) { description += (craftMaxPassengers == 0) ? "\nNO PASSENGER SPACE!" : "\nMax Passengers: " + std::to_string(craftMaxPassengers); }
            } else {
				// Items in the BuyMenu always have any remainder rounded up in their masses.
                const Actor *itemAsActor = dynamic_cast<const Actor *>(currentItem);
                if (itemAsActor) {
                    description += "\nMass: " + RoundFloatToPrecision(itemAsActor->GetMass(), 1, 2) + " kg";
                    int passengerSlotsTaken = itemAsActor->GetPassengerSlots();
                    if (passengerSlotsTaken > 1) {
                        description += "\nPassenger Slots: " + std::to_string(passengerSlotsTaken);
                    }
                } else {
                    const MovableObject *itemAsMO = dynamic_cast<const MovableObject *>(currentItem);
                    if (itemAsMO) {
                        description += "\nMass: " + RoundFloatToPrecision(itemAsMO->GetMass(), 1, 2) + " kg";
                    }
                }
            }
        } else if (pItem && pItem->m_ExtraIndex >= 0) {
            const DataModule *pModule = g_PresetMan.GetDataModule(pItem->m_ExtraIndex);
            if (pModule && !pModule->GetDescription().empty()) {
                description = pModule->GetDescription();
            }
        }

        // Show popup info box next to selected item if it has a description or tooltip.
        if (!description.empty()) {
            // Show the popup box with the hovered item's description
            m_pPopupBox->SetVisible(true);
            // Need to add an offset to make it look better and not have the cursor obscure text
            m_pPopupBox->SetPositionAbs(m_pShopList->GetXPos() - 6 + m_pShopList->GetWidth(), m_pShopList->GetYPos() + m_pShopList->GetStackHeight(pItem) - m_pShopList->GetScrollVerticalValue());
            // Make sure the popup box doesn't drop out of sight
            if (m_pPopupBox->GetYPos() + m_pPopupBox->GetHeight() > m_pParentBox->GetHeight())
                m_pPopupBox->SetPositionAbs(m_pPopupBox->GetXPos(), m_pParentBox->GetHeight() - m_pPopupBox->GetHeight());
            m_pPopupText->SetHAlignment(GUIFont::Left);
            m_pPopupText->SetText(description);
            // Resize the box height to fit the text
            int newHeight = m_pPopupText->ResizeHeightToFit();
            m_pPopupBox->Resize(m_pPopupBox->GetWidth(), newHeight + 10);
        }

        // User selected to add an item to cart list!
        if (m_pController->IsState(PRESS_FACEBUTTON)) {
            // User pressed on a module group item; toggle its expansion!
			if (pItem && pItem->m_ExtraIndex >= 0) {
				// Make appropriate sound
				if (!m_aExpandedModules[pItem->m_ExtraIndex]) {
					g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
				} else {
					// Different, maybe?
					g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
				}
				// Toggle the expansion of the module group item's items below
				m_aExpandedModules[pItem->m_ExtraIndex] = !m_aExpandedModules[pItem->m_ExtraIndex];
				// Re-populate the item list with the new module expansion configuation
				CategoryChange(false);
			}
            // User pressed on a loadout set, so load it into the menu
            else if (pItem && m_MenuCategory == SETS) {
                // Beep if there's an error
                if (!DeployLoadout(m_ListItemIndex))
                    g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
            }
            // User mashed button on a regular shop item, add it to cargo, or select craft
            else if (pItem && pItem->m_pEntity)
            {
                // Select the craft 
                if (m_MenuCategory == CRAFT)
                {
                    if (m_pSelectedCraft = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
                    {
                        m_pCraftBox->SetText(pItem->m_Name);
                        m_pCraftBox->SetRightText(pItem->m_RightText);

						m_pCraftNameLabel->SetText(pItem->m_Name);
						m_pCraftPriceLabel->SetText(pItem->m_RightText);
						UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(pItem->m_pEntity), m_pCraftPassengersLabel);
						UpdateTotalMassLabel(dynamic_cast<const ACraft *>(pItem->m_pEntity), m_pCraftMassLabel);
					}
                }
                // Regular ship inventory
                else
                {
                    // Gotto make a copy of the bitmap to pass it to the next list
                    GUIBitmap *pItemBitmap = new AllegroBitmap(dynamic_cast<AllegroBitmap *>(pItem->m_pBitmap)->GetBitmap());
                    m_pCartList->AddItem(pItem->m_Name, pItem->m_RightText, pItemBitmap, pItem->m_pEntity);
                }
                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
            }

            UpdateTotalCostLabel(m_pController->GetTeam());

			UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
			UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);
        }
    }

    /////////////////////////////////////////
    // CART/ORDER LIST focus

    else if (m_MenuFocus == ORDER) {
        // Changed to the list, so select the top one in the item list
        if (m_FocusChange) {
            m_pCartList->SetFocus();
            if (!m_pCartList->GetItemList()->empty() && m_pCartList->GetSelectedIndex() < 0) {
                m_pCartList->SetSelectedIndex(m_ListItemIndex = 0);
                // Synch our index with the one already selected in the list
            } else {
                m_ListItemIndex = m_pCartList->GetSelectedIndex();
                m_pCartList->ScrollToSelected();
            }

            m_FocusChange = 0;
        }

        int listSize = m_pCartList->GetItemList()->size();
        if (pressDown) {
            m_ListItemIndex++;
            if (m_ListItemIndex >= listSize) {
                m_ListItemIndex = listSize - 1;
                // If at the end of the list and the player presses down, then switch focus to the BUY button
                m_FocusChange = 1;
                m_MenuFocus = OK;
            } else {
                // Only do list change logic if we actually did change
                m_pCartList->SetSelectedIndex(m_ListItemIndex);
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
        } else if (pressUp) {
            m_ListItemIndex--;
            if (m_ListItemIndex < 0) {
                m_ListItemIndex = 0;
                g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
            } else {
                // Only do list change logic if we actually did change
                m_pCartList->SetSelectedIndex(m_ListItemIndex);
                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
            }
        }

        // Get handle to the currently selected item, if any
        GUIListPanel::Item *pItem = m_pCartList->GetItem(m_ListItemIndex);
        std::string description = "";

        if (pItem && pItem->m_pEntity) {
			description = ((pItem->m_pEntity->GetDescription().empty()) ? "-No Information Found-" : pItem->m_pEntity->GetDescription()) + "\n";
            const Entity *currentItem = pItem->m_pEntity;
            const Actor *itemAsActor = dynamic_cast<const Actor *>(currentItem);
            if (itemAsActor) {
                description += "\nMass: " + RoundFloatToPrecision(itemAsActor->GetMass(), 1, 2) + " kg";

                int passengerSlotsTaken = itemAsActor->GetPassengerSlots();
                if (passengerSlotsTaken > 1) {
                    description += "\nPassenger Slots: " + std::to_string(passengerSlotsTaken);
                }
            } else {
                const MovableObject *itemAsMO = dynamic_cast<const MovableObject *>(currentItem);
                if (itemAsMO) {
                    description += "\nMass: " + RoundFloatToPrecision(itemAsMO->GetMass(), 1, 2) + " kg";
                }
            }
        }

        if (!description.empty()) {
            // Show the popup box with the hovered item's description
            m_pPopupBox->SetVisible(true);
            // Need to add an offset to make it look better and not have the cursor obscure text
            m_pPopupBox->SetPositionAbs(m_pCartList->GetXPos() - m_pPopupBox->GetWidth() + 4, m_pCartList->GetYPos() + m_pCartList->GetStackHeight(pItem) - m_pCartList->GetScrollVerticalValue());
            // Make sure the popup box doesn't drop out of sight
            if (m_pPopupBox->GetYPos() + m_pPopupBox->GetHeight() > m_pParentBox->GetHeight())
                m_pPopupBox->SetPositionAbs(m_pPopupBox->GetXPos(), m_pParentBox->GetHeight() - m_pPopupBox->GetHeight());
            m_pPopupText->SetHAlignment(GUIFont::Right);
            m_pPopupText->SetText(description);
            // Resize the box height to fit the text
            int newHeight = m_pPopupText->ResizeHeightToFit();
            m_pPopupBox->Resize(m_pPopupBox->GetWidth(), newHeight + 10);
        }

        // Fire button removes items from the order list
        if (m_pController->IsState(PRESS_FACEBUTTON))
        {
            m_pCartList->DeleteItem(m_ListItemIndex);
            // If we're not at the bottom, then select the item in the same place as the one just deleted
            if (m_pCartList->GetItemList()->size() > m_ListItemIndex)
                m_pCartList->SetSelectedIndex(m_ListItemIndex);
            // If we're not at the top, then move selection up one
            else if (m_ListItemIndex > 0)
                m_pCartList->SetSelectedIndex(--m_ListItemIndex);
            // Shift focus back to the item list
            else
                m_MenuFocus = ORDER;

            UpdateTotalCostLabel(m_pController->GetTeam());

			UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
			UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);

            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
        }
    }

    /////////////////////////////////////////
    // OK BUTTON focus

    else if (m_MenuFocus == OK)
    {
        if (m_FocusChange)
        {
            m_pBuyButton->SetFocus();
            m_FocusChange = 0;
        }

        if (m_pController->IsState(PRESS_FACEBUTTON))
        {
            // Attempt to do the purchase
            TryPurchase();
        }

        // Switch back focus to the order list if the player presses up
        if (pressUp)
        {
            m_MenuFocus = ORDER;
            m_FocusChange = -1;
        }
        else if (pressDown)
            g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
    }

    // If mouse clicked outside the buy menu, the user is considered havin g tried to buy
    if (m_pController->IsMouseControlled() && m_MenuEnabled == ENABLED && m_pController->IsState(PRESS_PRIMARY) && m_CursorPos.m_X > m_pParentBox->GetWidth())
        TryPurchase();

    // ESC, Right click, or pie menu press close the menu
    if (m_pController->IsState(PRESS_SECONDARY) || m_pController->IsState(PIE_MENU_ACTIVE) || g_UInputMan.AnyStartPress(false))
        SetEnabled(false);

    //////////////////////////////////////////
	// Update the ControlManager

	m_pGUIController->Update();


    ///////////////////////////////////////
    // Handle events

	GUIEvent anEvent;
	while(m_pGUIController->GetEvent(&anEvent))
    {
		if (anEvent.GetType() == GUIEvent::Command)
        {
            // SAVE button clicks
			if(anEvent.GetControl() == m_pSaveButton)
            {
                m_pSaveButton->SetFocus();
                SaveCurrentLoadout();
                m_MenuFocus = SETBUTTONS;
//                m_FocusChange = -1;
                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
			}

            // CLEAR button clicks
			if(anEvent.GetControl() == m_pClearButton)
            {
                m_pClearButton->SetFocus();
				if (!m_Loadouts.empty())
					m_Loadouts.pop_back();
                // Update the list of loadout presets so the removal shows up
                CategoryChange();
                // Save new loadout config to file
                SaveAllLoadoutsToFile();
                // Set focus back on the clear button (CatChange changed it)
                m_pClearButton->SetFocus();
                m_MenuFocus = SETBUTTONS;
//                m_FocusChange = -1;
                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
			}

            // BUY button clicks
			if(anEvent.GetControl() == m_pBuyButton)
            {
                m_pBuyButton->SetFocus();
                TryPurchase();
//                g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
			}
        }
        else if (anEvent.GetType() == GUIEvent::Notification)
        {
            //////////////////////////////////////////
			// Clicks on any of the category tabs

            for (int cat = 0; cat < CATEGORYCOUNT; ++cat)
            {
			    if(anEvent.GetControl() == m_pCategoryTabs[cat])
                {
                    // Mouse hovering over
                    if(anEvent.GetMsg() == GUITab::Hovered)
                    {
                        // Just give focus to the categories column
                        m_MenuFocus = CATEGORIES;
                        m_pCategoryTabs[m_MenuCategory]->SetFocus();
                    }
                    // Regular click
                    if(anEvent.GetMsg() == GUITab::Pushed)
                    {
                        m_MenuFocus = CATEGORIES;
                        m_MenuCategory = cat;
                        m_pCategoryTabs[m_MenuCategory]->SetFocus();
                        CategoryChange();
                        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                    }
                }
            }

            //////////////////////////////////////////
			// Events on the Shop List

			if(anEvent.GetControl() == m_pShopList)
            {
/*
                // Somehting was just selected, so update the selection index to the new selected index
                if(anEvent.GetMsg() == GUIListBox::Select)
                {
                    if (m_ListItemIndex != m_pShopList->GetSelectedIndex())
                        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                    m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex = m_pShopList->GetSelectedIndex();
                }
                // Mouse down, added something to cart!
                else*/ if(anEvent.GetMsg() == GUIListBox::MouseDown)
                {
                    m_pShopList->SetFocus();
                    m_MenuFocus = ITEMS;

                    GUIListPanel::Item *pItem = m_pShopList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y);

                    // If a module group list item, toggle its expansion and update the list
                    if (pItem && pItem->m_ExtraIndex >= 0)
                    {
                        // Make appropriate sound
                        if (!m_aExpandedModules[pItem->m_ExtraIndex])
                            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                        // Different, maybe?
                        else
                            g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                        // Toggle the expansion of the module group item's items below
                        m_aExpandedModules[pItem->m_ExtraIndex] = !m_aExpandedModules[pItem->m_ExtraIndex];
                        // Re-populate the item list with the new module expansion configuation
                        CategoryChange(false);
                    }
                    // Special case: user clicked on a loadout set, so load it into the menu
                    else if (pItem && m_MenuCategory == SETS)
                    {
                        // Beep if there's an error
                        if (!DeployLoadout(m_ListItemIndex))
                            g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
                    }
                    // Normal: only add an item if there's an entity attached to the list item
                    else if (pItem && pItem->m_pEntity)
                    {
                        m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex = m_pShopList->GetSelectedIndex();
                        m_pShopList->ScrollToSelected();

                        // Select the craft 
                        if (m_MenuCategory == CRAFT)
                        {
                            if (m_pSelectedCraft = dynamic_cast<const SceneObject *>(pItem->m_pEntity))
                            {
                                m_pCraftBox->SetText(pItem->m_Name);
                                m_pCraftBox->SetRightText(pItem->m_RightText);

								m_pCraftNameLabel->SetText(pItem->m_Name);
								m_pCraftPriceLabel->SetText(pItem->m_RightText);
								UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(pItem->m_pEntity), m_pCraftPassengersLabel);
								UpdateTotalMassLabel(dynamic_cast<const ACraft *>(pItem->m_pEntity), m_pCraftMassLabel);
							}
                        }
                        // Regular ship inventory
                        else
                        {
                            // Gotto make a copy of the bitmap to pass it to the next list
                            GUIBitmap *pItemBitmap = new AllegroBitmap(dynamic_cast<AllegroBitmap *>(pItem->m_pBitmap)->GetBitmap());

							if (m_OwnedItems.size() > 0 || m_OnlyShowOwnedItems)
							{
								if (GetOwnedItemsAmount(pItem->m_pEntity->GetModuleAndPresetName()) > 0)
								{
									m_pCartList->AddItem(pItem->m_Name, "1 pc", pItemBitmap, pItem->m_pEntity);
								}
								else
								{
									if (m_OnlyShowOwnedItems)
									{
										if (IsAlwaysAllowedItem(pItem->m_Name))
											m_pCartList->AddItem(pItem->m_Name, pItem->m_RightText, pItemBitmap, pItem->m_pEntity);
									}
									else 
									{
										m_pCartList->AddItem(pItem->m_Name, pItem->m_RightText, pItemBitmap, pItem->m_pEntity);
									}
								}
							}
							else
							{
								m_pCartList->AddItem(pItem->m_Name, pItem->m_RightText, pItemBitmap, pItem->m_pEntity);
							}
                        }
                        g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                    }
                    // Undo the click deselection if nothing was selected
//                    else
//                        m_pShopList->SetSelectedIndex(m_SelectedObjectIndex);

                    UpdateTotalCostLabel(m_pController->GetTeam());

					UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
					UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);
				}
                // Mouse moved over the panel, show the popup with item description
                else if(anEvent.GetMsg() == GUIListBox::MouseMove)
                {
                    // Mouse is moving within the list, so make it focus on the list
                    m_pShopList->SetFocus();
                    m_MenuFocus = ITEMS;

                    // See if it's hovering over any item
                    GUIListPanel::Item *pItem = m_pShopList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y);
                    if (pItem)
                    {
                        // Don't let mouse movement change the index if it's still hovering inside the same item.
                        // This is to avoid erratic selection curosr if using both mouse and keyboard to work the menu
                        if (m_LastHoveredMouseIndex != pItem->m_ID)
                        {
                            m_LastHoveredMouseIndex = pItem->m_ID;

                            // Play select sound if new index
                            if (m_ListItemIndex != pItem->m_ID)
                                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                            // Update the seleciton in both the GUI control and our menu
                            m_pShopList->SetSelectedIndex(m_CategoryItemIndex[m_MenuCategory] = m_ListItemIndex = pItem->m_ID);
                        }
                    }
                }
            }

            ///////////////////////////////////////////////
            // Events on the Cart List

            else if (anEvent.GetControl() == m_pCartList)
            {
/*
                // Somehting was just selected, so update the selection index to the new selected index
                if(anEvent.GetMsg() == GUIListBox::Select)
                {                   
                    if (m_ListItemIndex != m_pCartList->GetSelectedIndex())
                        g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                    m_ListItemIndex = m_pCartList->GetSelectedIndex();
                }
                // Somehting was clicked upon, therefore should be removed
                else*/ if(anEvent.GetMsg() == GUIListBox::MouseDown)
                {
                    m_pCartList->SetFocus();
                    m_MenuFocus = ORDER;

                    GUIListPanel::Item *pItem = m_pCartList->GetSelected();
                    if (pItem)
                    {
                        m_ListItemIndex = m_pCartList->GetSelectedIndex();
                        m_pCartList->ScrollToSelected();

                        m_pCartList->DeleteItem(m_ListItemIndex);
                        // If we're not at the bottom, then select the item in the same place as the one just deleted
                        if (m_pCartList->GetItemList()->size() > m_ListItemIndex)
                            m_pCartList->SetSelectedIndex(m_ListItemIndex);
                        // If we're not at the top, then move selection up one
                        else if (m_ListItemIndex > 0)
                            m_pCartList->SetSelectedIndex(--m_ListItemIndex);
                        // Shift focus back to the item list
                        else
                            m_MenuFocus = ORDER;

                        UpdateTotalCostLabel(m_pController->GetTeam());

						UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
						UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);

                        g_GUISound.ItemChangeSound()->Play(m_pController->GetPlayer());
                    }
                    // Undo the click deselection if nothing was selected
//                    else
//                        m_pCartList->SetSelectedIndex(m_SelectedObjectIndex);

                    UpdateTotalCostLabel(m_pController->GetTeam());

					UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
					UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);
				}
                // Mouse moved over the panel, show the popup with item description
                else if(anEvent.GetMsg() == GUIListBox::MouseMove)
                {                    
                    // Mouse is moving within the list, so make it focus on the list
                    m_pCartList->SetFocus();
                    m_MenuFocus = ORDER;

                    // See if it's hovering over any item
                    GUIListPanel::Item *pItem = m_pCartList->GetItem(m_CursorPos.m_X, m_CursorPos.m_Y);
                    if (pItem)
                    {
                        // Don't let mouse movement change the index if it's still hovering inside the same item.
                        // This is to avoid erratic selection curosr if using both mouse and keyboard to work the menu
                        if (m_LastHoveredMouseIndex != pItem->m_ID)
                        {
                            m_LastHoveredMouseIndex = pItem->m_ID;

                            // Play select sound if new index
                            if (m_ListItemIndex != pItem->m_ID)
                                g_GUISound.SelectionChangeSound()->Play(m_pController->GetPlayer());
                            // Update the seleciton in both the GUI control and our menu
                            m_pCartList->SetSelectedIndex(m_ListItemIndex = pItem->m_ID);
                        }
                    }
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void BuyMenuGUI::Draw(BITMAP *drawBitmap) const
{
    AllegroScreen drawScreen(drawBitmap);
    m_pGUIController->Draw(&drawScreen);

    // Draw the cursor on top of everything 
    if (IsEnabled() && m_pController->IsMouseControlled())
//        m_pGUIController->DrawMouse();
        draw_sprite(drawBitmap, s_pCursor, m_CursorPos.GetFloorIntX(), m_CursorPos.GetFloorIntY());
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FocusChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all things that to happen when focus is moved from one area
//                  or control to the next within the menu, will happen.

void BuyMenuGUI::FocusChange()
{
    // Set control focus accordingly
    if (m_MenuFocus == CATEGORIES)
    {
        if (m_pController->IsState(PRESS_DOWN))
        {
            m_MenuCategory++;
            if (m_MenuCategory >= CATEGORYCOUNT)
                m_MenuCategory = CATEGORYCOUNT - 1;
            // Only do category change logic if we actually did change
            else
                CategoryChange();
        }
        else if (m_pController->IsState(PRESS_UP))
        {
            m_MenuCategory--;
            if (m_MenuCategory < 0)
                m_MenuCategory = 0;
            // Only do category change logic if we actually did change
            else
                CategoryChange();
        }
    }
    else if (m_MenuFocus == ITEMS)
    {
        m_pShopList->SetFocus();
        m_pShopList->GetItemList()->size();
    }
    else if (m_MenuFocus == ORDER)
    {
        m_pCartList->SetFocus();
    }
    else if (m_MenuFocus == OK)
    {
        m_pBuyButton->SetFocus();
    }
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  CategoryChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all things that to happen when category is changed, happens.

void BuyMenuGUI::CategoryChange(bool focusOnCategoryTabs)
{
    // Re-set the GUI manager's focus on the tabs if we're supposed to
    // We don't want to do that if we're just refreshing the same category, like in the case of of expanding a module group item
    if (focusOnCategoryTabs)
    {
        m_pCategoryTabs[m_MenuCategory]->SetFocus();
        m_pCategoryTabs[m_MenuCategory]->SetCheck(true);
    }
    m_pShopList->ClearList();

    // Hide/show the logo and special sets category buttons, and add all current presets to the list, and we're done.
    if (m_MenuCategory == SETS)
    {
        m_pLogo->SetVisible(false);
        m_pSaveButton->SetVisible(true);
        m_pClearButton->SetVisible(true);
        // Add and done!
        AddPresetsToItemList();
        return;
    }
    // Hide the sets buttons otherwise
    else
    {
        m_pLogo->SetVisible(true);
        m_pSaveButton->SetVisible(false);
        m_pClearButton->SetVisible(false);
    }

    // The vector of lists which will be filled with catalog objects, grouped by which data module they were read from
    vector<list<Entity *> > catalogList;

    if (m_MenuCategory == CRAFT)
    {
        AddObjectsToItemList(catalogList, "ACRocket");
        AddObjectsToItemList(catalogList, "ACDropShip");
    }
    else if (m_MenuCategory == BODIES)
    {
        AddObjectsToItemList(catalogList, "AHuman");
        AddObjectsToItemList(catalogList, "ACrab");
    }
    else if (m_MenuCategory == TOOLS)
    {
        AddObjectsToItemList(catalogList, "HDFirearm", "Tools");
    }
    else if (m_MenuCategory == GUNS)
    {
        AddObjectsToItemList(catalogList, "HDFirearm", "Weapons");
    }
    else if (m_MenuCategory == BOMBS)
    {
        AddObjectsToItemList(catalogList, "ThrownDevice", "Bombs");
    }
    else if (m_MenuCategory == SHIELDS)
    {
        AddObjectsToItemList(catalogList, "HeldDevice", "Shields");
    }

    SceneObject *pSObject = 0;
    const DataModule *pModule = 0;
    GUIBitmap *pItemBitmap = 0;
    list<SceneObject *> tempList;
    for (int moduleID = 0; moduleID < catalogList.size(); ++moduleID)
    {
        // Don't add an empty module grouping
        if (!catalogList[moduleID].empty())
        {
            tempList.clear();

            // Move all valid/desired entities from the module list to the intermediate list
            for (list<Entity *>::iterator oItr = catalogList[moduleID].begin(); oItr != catalogList[moduleID].end(); ++oItr)
            {
                pSObject = dynamic_cast<SceneObject *>(*oItr);
                // Buyable and not brain?
                if (pSObject && pSObject->IsBuyable() && !pSObject->IsInGroup("Brains"))
                    tempList.push_back(pSObject);
            }

            // Don't add anyhting to the real buy item list if the current module didn't yield any valid items
            if (!tempList.empty())
            {
                // Add the DataModule separator in the shop list, with appropriate name and perhaps icon? Don't add for first base module
                if (moduleID != 0 && (pModule = g_PresetMan.GetDataModule(moduleID)))
                {
                    pItemBitmap = pModule->GetIcon() ? new AllegroBitmap(pModule->GetIcon()) : 0;
                    // Passing in ownership of the bitmap, making uppercase the name
                    string name = pModule->GetFriendlyName();
                    transform(name.begin(), name.end(), name.begin(), ::toupper);
                    m_pShopList->AddItem(name, m_aExpandedModules[moduleID] ? "-" : "+", pItemBitmap, 0, moduleID);
                }

                // If the module is expanded, add all the items within it below
                if (moduleID == 0 || m_aExpandedModules[moduleID])
                {
                    // Transfer from the temp intermediate list to the real gui list
                    for (list<SceneObject *>::iterator tItr = tempList.begin(); tItr != tempList.end(); ++tItr)
                    {
                        // Get a good icon and wrap it, while not passing ownership into the AllegroBitmap
                        pItemBitmap = new AllegroBitmap((*tItr)->GetGraphicalIcon());
                        // Passing in ownership of the bitmap, but not of the pSpriteObj
						if (m_OwnedItems.size() > 0 || m_OnlyShowOwnedItems)
						{ 
							if (GetOwnedItemsAmount((*tItr)->GetModuleAndPresetName()) > 0)
							{
								string amount = std::to_string(GetOwnedItemsAmount((*tItr)->GetModuleAndPresetName())) + " pcs";

								m_pShopList->AddItem((*tItr)->GetPresetName(), amount , pItemBitmap, *tItr);
							} 
							else 
							{ 
								if (!m_OnlyShowOwnedItems)
									m_pShopList->AddItem((*tItr)->GetPresetName(), (*tItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *tItr);
								else
								{
									if (m_AlwaysAllowedItems.find((*tItr)->GetModuleAndPresetName()) != m_AlwaysAllowedItems.end())
										m_pShopList->AddItem((*tItr)->GetPresetName(), (*tItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *tItr);
								}
							}
						}
						else
						{
							m_pShopList->AddItem((*tItr)->GetPresetName(), (*tItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *tItr);
						}
                    }
                }
            }
        }
    }

    // Set the last saved index for this category so the menu scrolls down to it
    m_pShopList->SetSelectedIndex(m_CategoryItemIndex[m_MenuCategory]);
    m_ListItemIndex = m_CategoryItemIndex[m_MenuCategory];
    if (focusOnCategoryTabs)
        m_pShopList->ScrollToSelected();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SaveCurrentLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current loadout into a Set.

void BuyMenuGUI::SaveCurrentLoadout()
{
    Loadout newSet;

    // Abort if there's no cargo to save into the preset
    if (!GetOrderList(*(newSet.GetCargoList())))
        return;

    // Add the ship
    newSet.SetDeliveryCraft(dynamic_cast<const ACraft *>(GetDeliveryCraftPreset()));

    // Add it to the loadouts
    m_Loadouts.push_back(newSet);

    // Save the new list of loadouts to file
    SaveAllLoadoutsToFile();

    // Update the list of loadout presets so the new one shows up
    CategoryChange();

    // Set focus back on the save button (CatChange changed it)
    m_pSaveButton->SetFocus();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  DeployLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the loadout set into the cart, replacing whatever's there now.

bool BuyMenuGUI::DeployLoadout(int index)
{
    if (index < 0 || index >= m_Loadouts.size())
        return false;

    // Clear the cart, we're going to refill it with the selected loadout
    m_pCartList->ClearList();

	// Check if the craft is available
	const ACraft * pCraft = m_Loadouts[index].GetDeliveryCraft();

	if (pCraft)
	{
		bool craftAvailable = true;

		if (IsAllowedItem(pCraft->GetModuleAndPresetName()))
			craftAvailable = true;

		if (IsProhibitedItem(pCraft->GetModuleAndPresetName()))
			craftAvailable = false;

		if (m_OnlyShowOwnedItems && craftAvailable)
		{
			if (GetOwnedItemsAmount(pCraft->GetModuleAndPresetName()) > 0)
				craftAvailable = true;
			else
				craftAvailable = false;
		}

		if (IsAlwaysAllowedItem(pCraft->GetModuleAndPresetName()))
			craftAvailable = true;

		if (!craftAvailable)
			return false;
	}

    // Get and add all the stuff in the selected loadout
    list<const SceneObject *> *pCargo = m_Loadouts[index].GetCargoList();
    AllegroBitmap *pItemBitmap = 0;
    for (list<const SceneObject *>::iterator cItr = pCargo->begin(); cItr != pCargo->end(); ++cItr)
    {
        // Get a good icon and wrap it, while not passing ownership into the AllegroBitmap
        pItemBitmap = new AllegroBitmap(const_cast<SceneObject *>(*cItr)->GetGraphicalIcon());
        // Take into account whether these are native or not, and multiply the cost accordingly
		bool canAdd = true;

		if (IsAllowedItem((*cItr)->GetModuleAndPresetName()))
			canAdd = true;

		if (IsProhibitedItem((*cItr)->GetModuleAndPresetName()))
			canAdd = false;

		if (m_OnlyShowOwnedItems && canAdd)
		{
			if (GetOwnedItemsAmount((*cItr)->GetModuleAndPresetName()) > 0)
			{
				canAdd = false;
				// Add manually with pcs counter
				m_pCartList->AddItem((*cItr)->GetPresetName(), "1 pc", pItemBitmap, *cItr);
			}
			else
				canAdd = false;
		}

		if (IsAlwaysAllowedItem((*cItr)->GetModuleAndPresetName()))
			canAdd = true;

		if (canAdd)
			m_pCartList->AddItem((*cItr)->GetPresetName(), (*cItr)->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult), pItemBitmap, *cItr);
    }
    // Now set the craft to what the loadout specifies, if anything
    if (m_Loadouts[index].GetDeliveryCraft())
    {
        m_pSelectedCraft = m_Loadouts[index].GetDeliveryCraft();
        // Take into account whether these are native or not, and multiply the cost accordingly
        m_pCraftBox->SetText(m_pSelectedCraft->GetPresetName());
        m_pCraftBox->SetRightText(m_pSelectedCraft->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult));

		m_pCraftNameLabel->SetText(m_pSelectedCraft->GetPresetName());
		m_pCraftPriceLabel->SetText(m_pSelectedCraft->GetGoldValueString(m_NativeTechModule, m_ForeignCostMult));
		UpdateTotalPassengersLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftPassengersLabel);
		UpdateTotalMassLabel(dynamic_cast<const ACraft *>(m_pSelectedCraft), m_pCraftMassLabel);
    }

    // Update labels with the new config's values
    UpdateTotalCostLabel();

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddObjectsToItemList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all objects of a specific type already defined in PresetMan
//                  to the current shop/item list. They will be grouped into the different
//                  data modules they were read from.

void BuyMenuGUI::AddObjectsToItemList(vector<list<Entity *> > &moduleList, string type, string group)
{

	if (g_SettingsMan.ShowForeignItems() || m_NativeTechModule <= 0)
	{
		// Make as many datamodule entries as necessary in the vector
		while (moduleList.size() < g_PresetMan.GetTotalModuleCount())
			moduleList.push_back(list<Entity *>());

		// Go through all the data modules, gathering the objects that match the criteria in each one
		for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID)
		{
			if (group.empty() || group == "All")
				g_PresetMan.GetAllOfType(moduleList[moduleID], type, moduleID);
			else
				g_PresetMan.GetAllOfGroup(moduleList[moduleID], group, type, moduleID);
		}
	} else {
		// Make as many datamodule entries as necessary in the vector
		while (moduleList.size() < g_PresetMan.GetTotalModuleCount())
			moduleList.push_back(list<Entity *>());

		// Go through all the data modules, gathering the objects that match the criteria in each one
		for (int moduleID = 0; moduleID < g_PresetMan.GetTotalModuleCount(); ++moduleID)
		{
			if (moduleID == 0 || moduleID == m_NativeTechModule)
			{
				if (group.empty() || group == "All")
					g_PresetMan.GetAllOfType(moduleList[moduleID], type, moduleID);
				else
					g_PresetMan.GetAllOfGroup(moduleList[moduleID], group, type, moduleID);
			}
		}
	}

	// Remove itwms which are not allowed to buy
	if (m_AllowedItems.size() > 0)
	{
		for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID)
		{
			list<Entity *> toRemove;

			for (list<Entity *>::iterator itr = moduleList[moduleID].begin(); itr != moduleList[moduleID].end(); ++itr)
			{
				bool allowed = false;

				if (m_AllowedItems.find((*itr)->GetModuleAndPresetName()) != m_AllowedItems.end())
					allowed = true;

				if (m_AlwaysAllowedItems.find((*itr)->GetModuleAndPresetName()) != m_AlwaysAllowedItems.end())
					allowed = true;

				if (!allowed)
					toRemove.push_back((*itr));
			}

			// Remove items from the list
			for (list<Entity *>::iterator itr = toRemove.begin(); itr != toRemove.end(); ++itr)
				moduleList[moduleID].remove((*itr));
		}
	}

	// Remove items which are prohibited
	if (m_ProhibitedItems.size() > 0)
	{
		for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID)
		{
			list<Entity *> toRemove;

			for (list<Entity *>::iterator itr = moduleList[moduleID].begin(); itr != moduleList[moduleID].end(); ++itr)
			{
				bool allowed = true;

				if (m_ProhibitedItems.find((*itr)->GetModuleAndPresetName()) != m_ProhibitedItems.end())
					allowed = false;

				if (m_AlwaysAllowedItems.find((*itr)->GetModuleAndPresetName()) != m_AlwaysAllowedItems.end())
					allowed = true;

				if (!allowed)
					toRemove.push_back((*itr));
			}

			// Remove items from the list
			for (list<Entity *>::iterator itr = toRemove.begin(); itr != toRemove.end(); ++itr)
				moduleList[moduleID].remove((*itr));
		}
	}

	// Remove items which are not in stock
	if (m_OnlyShowOwnedItems && m_OwnedItems.size() > 0)
	{
		for (int moduleID = 0; moduleID < moduleList.size(); ++moduleID)
		{
			list<Entity *> toRemove;

			for (list<Entity *>::iterator itr = moduleList[moduleID].begin(); itr != moduleList[moduleID].end(); ++itr)
			{
				bool allowed = false;

				for (map<string, int>::iterator itrA = m_OwnedItems.begin(); itrA != m_OwnedItems.end(); ++itrA)
				{
					if ((*itr)->GetModuleAndPresetName() == (*itrA).first && (*itrA).second > 0)
						allowed = true;
				}

				if (m_AlwaysAllowedItems.find((*itr)->GetModuleAndPresetName()) != m_AlwaysAllowedItems.end())
					allowed = true;

				if (!allowed)
					toRemove.push_back((*itr));
			}

			// Remove items from the list
			for (list<Entity *>::iterator itr = toRemove.begin(); itr != toRemove.end(); ++itr)
				moduleList[moduleID].remove((*itr));
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPresetsToItemList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all loadout presets' representations to the item GUI list.

void BuyMenuGUI::AddPresetsToItemList()
{
    GUIBitmap *pItemBitmap = 0;
    string loadoutLabel;
    float loadoutCost;
    const Actor *pPassenger = 0;
    char costString[256];

    // Go through all the presets, making intelligible list items from then for the GUI item list
    for (vector<Loadout>::iterator lItr = m_Loadouts.begin(); lItr != m_Loadouts.end(); ++lItr)
    {
        loadoutLabel.clear();
        loadoutCost = 0;
        pItemBitmap = 0;
        pPassenger = 0;

		// Add preset name at the begining to differentiate loadouts from user-defined presets
		if ((*lItr).GetPresetName() != "None")
			loadoutLabel = (*lItr).GetPresetName() + ":\n";

        // Go through the cargo setup of each loadout and encode a meaningful label for the list item
        for (list<const SceneObject *>::iterator cItr = (*lItr).GetCargoList()->begin(); cItr != (*lItr).GetCargoList()->end(); ++cItr)
        {
            // If not the first one, add a comma separator to the label
            if (cItr != (*lItr).GetCargoList()->begin())
                loadoutLabel += ", ";
            // Append the name of the current cargo thing to the label
            loadoutLabel += (*cItr)->GetPresetName();
            // Adjust price for foreignness of the items to this player
            loadoutCost += (*cItr)->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);
            if (!pPassenger)
                pPassenger = dynamic_cast<const Actor *>(*cItr);
        }

        // Add the ship's cost, if there is one defined
        if ((*lItr).GetDeliveryCraft())
        {
            loadoutLabel += " on " + (*lItr).GetDeliveryCraft()->GetPresetName();
            // Adjust price for foreignness of the ship to this player
            loadoutCost += (*lItr).GetDeliveryCraft()->GetGoldValue(m_NativeTechModule, m_ForeignCostMult);
        }

        // Make the cost label
        std::snprintf(costString, sizeof(costString), "%.0f", loadoutCost);
        // Get a good icon and wrap it, while not passing ownership into the AllegroBitmap
        // We're trying to pick the icon of the first passenger, or the first item if there's no passengers in the loadout
        pItemBitmap = new AllegroBitmap(pPassenger ? const_cast<Actor *>(pPassenger)->GetGraphicalIcon() : const_cast<SceneObject *>((*lItr).GetCargoList()->front())->GetGraphicalIcon());
        // Passing in ownership of the bitmap, but not of the pSpriteObj
        m_pShopList->AddItem(loadoutLabel, costString, pItemBitmap, 0);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuyMenuGUI::UpdateTotalCostLabel(int whichTeam) {
	std::string display = "Cost: " + RoundFloatToPrecision(GetTotalOrderCost(), 0, 2) + "/" + RoundFloatToPrecision(g_ActivityMan.GetActivity()->GetTeamFunds(whichTeam), 0);
	m_pCostLabel->SetText(display);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuyMenuGUI::UpdateTotalMassLabel(const ACraft* pCraft, GUILabel* pLabel) const {
	if (!pLabel) {
		return;
	}

	std::string display;
	if (pCraft && pCraft->GetMaxInventoryMass() != 0) {
		display = RoundFloatToPrecision(GetTotalOrderMass(), 1, 2);
		if (pCraft->GetMaxInventoryMass() > 0) {
			display += " / " + RoundFloatToPrecision(pCraft->GetMaxInventoryMass(), 1);
		}
	} else {
		display = "NO CARGO SPACE";
	}

	pLabel->SetText(display);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuyMenuGUI::UpdateTotalPassengersLabel(const ACraft* pCraft, GUILabel* pLabel) const {
	if (!pLabel) {
		return;
	}

	std::string display;
	if (pCraft && pCraft->GetMaxInventoryMass() != 0 && pCraft->GetMaxPassengers() != 0) {
		display = std::to_string(GetTotalOrderPassengers());
		if (pCraft->GetMaxPassengers() > 0) {
			display += " / " + std::to_string(pCraft->GetMaxPassengers());
		}
	} else {
		display = "NO SPACE";
	}

	pLabel->SetText(display);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BuyMenuGUI::TryPurchase()
{
    // Switch to the Craft category to give the user a hint
    if (!m_pSelectedCraft)
    {
        m_MenuCategory = CRAFT;
        CategoryChange();
        m_FocusChange = -2;
        m_MenuFocus = ITEMS;
        g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        // Set the notification blinker
        m_BlinkMode = NOCRAFT;
        m_BlinkTimer.Reset();
		return;
    }
    // Can't afford it :(
    else if (GetTotalOrderCost() > g_ActivityMan.GetActivity()->GetTeamFunds(m_pController->GetTeam()))
    {
        g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
        // Set the notification blinker
        m_BlinkMode = NOFUNDS;
        m_BlinkTimer.Reset();
		return;
	} else {
		const ACraft * pCraft = dynamic_cast<const ACraft *>(m_pSelectedCraft);
		if (pCraft) {
			// Enforce max mass
			if (m_EnforceMaxMassConstraint && pCraft->GetMaxInventoryMass() >= 0 && GetTotalOrderMass() > pCraft->GetMaxInventoryMass()) {
				g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
				// Set the notification blinker
				m_BlinkMode = MAXMASS;
				m_BlinkTimer.Reset();
				return;
			}

			// Enforce max passengers
			if (pCraft->GetMaxPassengers() >= 0 && GetTotalOrderPassengers() > pCraft->GetMaxPassengers() && m_EnforceMaxPassengersConstraint)
			{
				g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
				// Set the notification blinker
				m_BlinkMode = MAXPASSENGERS;
				m_BlinkTimer.Reset();
				return;
			}
		}
	}

	// Only allow purchase if there is a delivery craft and enough funds
	if (m_pSelectedCraft && std::floor(GetTotalOrderCost()) <= std::floor(g_ActivityMan.GetActivity()->GetTeamFunds(m_pController->GetTeam()))) {
		m_PurchaseMade = true;
		g_GUISound.PurchaseMadeSound()->Play(m_pController->GetPlayer());
	}
}

