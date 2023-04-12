#ifndef _BUYMENUGUI_
#define _BUYMENUGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            BuyMenuGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     BuyMenuGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Timer.h"
#include "Controller.h"
#include "Loadout.h"

struct BITMAP;


namespace RTE
{

class GUIScreen;
class GUIInput;
class GUIControlManager;
class GUICollectionBox;
class GUITab;
class GUIListBox;
class GUITextBox;
class GUIButton;
class GUIBitmap;
class GUILabel;
class SceneObject;
class MovableObject;
class ACraft;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           BuyMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A full menu system that represents a purchasing GUI for Cortex Command
// Parent(s):       None.
// Class history:   8/22/2006 BuyMenuGUI Created.

class BuyMenuGUI {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     BuyMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a BuyMenuGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    BuyMenuGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~BuyMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a BuyMenuGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~BuyMenuGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BuyMenuGUI object ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController);


//////////////////////////////////////////////////////////////////////////////////////////
// method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire BuyMenuGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the BuyMenuGUI object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LoadAllLoadoutsFromFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads or re-loads all the loadout presets from the appropriate files
//                  on disk. This will first clear out all current loadout presets!
// Arguments:       None.
// Return value:    Success or not.

    bool LoadAllLoadoutsFromFile();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SaveAllLoadoutsToFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves all the loadouts to appropriate file on disk. Does NOT save
//                  any named presets which will be loaded from the standard preset
//                  loadouts first anyway.
// Arguments:       None.
// Return value:    Success or not.

    bool SaveAllLoadoutsToFile();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the controller used by this. The ownership of the controller is
//                  NOT transferred!
// Arguments:       The new controller for this menu. Ownership is NOT transferred
// Return value:    None.

    void SetController(Controller *pController) { m_pController = pController; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.
// Arguments:       Whether to enable or disable the menu.
// Return value:    None.

    void SetEnabled(bool enable = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is enabled or not.
// Arguments:       None.
// Return value:    None.

    bool IsEnabled() const { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is at all visible or not.
// Arguments:       None.
// Return value:    None.

    bool IsVisible() const { return m_MenuEnabled == ENABLED || m_MenuEnabled == ENABLING || m_MenuEnabled == DISABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.
// Arguments:       The new screen position of this entire GUI.

    void SetPosOnScreen(int newPosX, int newPosY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which MetaPlayer uses this menu, if any.
// Arguments:       The index of the MetaPlayer that uses this menu.
// Return value:    None.

    void SetMetaPlayer(int metaPlayer);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which MetaPlayer uses this menu, if any.
// Arguments:       None.
// Return value:    Metaplayer who owns this buy menu

	int GetMetaPlayer() const { return m_MetaPlayer; }


    /// <summary>
    /// Sets which DataModule ID should be treated as the native tech of the user of this menu.
	/// This will also apply the DataModule's faction BuyMenu theme, if applicable.
    /// </summary>
	/// <param name="whichModule">The module ID to set as the native one. 0 means everything is native.</param>
    void SetNativeTechModule(int whichModule);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetForeignCostMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the multiplier of the cost of any foreign Tech items.
// Arguments:       The scalar multiplier of the costs of foreign Tech items.
// Return value:    None.

    void SetForeignCostMultiplier(float newMultiplier) { m_ForeignCostMult = newMultiplier; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleExpanded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether a data module shown in the item menu should be expanded
//                  or not.
// Arguments:       The module ID to set as expanded.
//                  Whether should be expanded or not.
// Return value:    None.

    void SetModuleExpanded(int whichModule, bool expanded = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PurchaseMade
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether a purchase was made during the last Update.
// Arguments:       None.
// Return value:    Wheter the BUY button was pressed or not during the last update.

    bool PurchaseMade() const { return m_PurchaseMade; }


	/// <summary>
	/// Gets the width of the current delivery craft.
	/// </summary>
	/// <returns>The width of the delivery craft, in pixels.</returns>
	int GetDeliveryWidth() const { return m_DeliveryWidth; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOrderList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return the list of things currently in the purchase order list box.
// Arguments:       A reference to a an empty list to fill with the Object:s ordered.
//                  Ownership of the Object:s is NOT TRANSFERRED!
// Return value:    Whetehr any items were put in the list at all. false if there are no
//                  items in the order listbox.

    bool GetOrderList(std::list<const SceneObject *> &listToFill);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadoutPresets
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return the list of loadouts currently saved as presets.
// Arguments:       None.
// Return value:    A reference to the list of loadout presets.

    std::vector<Loadout> & GetLoadoutPresets() { return m_Loadouts; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SaveCurrentLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current loadout into a Set.
// Arguments:       None.
// Return value:    None.

    void SaveCurrentLoadout();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeliveryCraftPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return the intended delivery vehicle instance from the order.
// Arguments:       None.
// Return value:    The poiner to the specified delivery craft instance. Note that this is
//                  just PresetMan's const pointer, so ownership is NOT transferred!

    const SceneObject * GetDeliveryCraftPreset() { return m_pSelectedCraft; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalOrderCost
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return teh total cost of everything listed in the order box.
// Arguments:       None.
// Return value:    The total cost in ounces of gold.

    float GetTotalOrderCost();


	/// <summary>
	/// Return the total mass of all items listed in the order box.
	/// </summary>
	/// <returns>The total mass (in kg) of the BuyMenu's cart.</returns>
	float GetTotalOrderMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCraftMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return mass of craft used in the order box.
// Arguments:       None.
// Return value:    The total mass in kg.

	float GetCraftMass();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalOrderPassengers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return teh total number of passengers in the order box.
// Arguments:       None.
// Return value:    The total number of passengers.

	int GetTotalOrderPassengers() const;

    /// <summary>
    /// Enable or disable the equipment selection mode for this BuyMenuGUI.
    /// </summary>
	/// <param name="enabled">Whether or not equipment selection mode should be enabled.</param>
    void EnableEquipmentSelection(bool enabled);

    /// <summary>
    /// Updates the nesting level for every item in the cart.
    /// </summary>
    void UpdateItemNestingLevels();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu
// Arguments:       The bitmap to draw on.
// Return value:    None.

    void Draw(BITMAP *drawBitmap) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:			EnforceMaxPassengersConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether passenger count constraints are enforced by this buy menu.
// Arguments:       None.
// Return value:    True if passenger constraints are enforced by this menu, false otherwise

	bool EnforceMaxPassengersConstraint() const { return m_EnforceMaxPassengersConstraint; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetEnforceMaxPassengersConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether passenger count constraints are enforced by this buy menu.
// Arguments:       True to enforce passenger constraints by this menu, false otherwise
// Return value:    None.

	void SetEnforceMaxPassengersConstraint(bool enforce) { m_EnforceMaxPassengersConstraint = enforce; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			EnforceMaxMassConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether mass constraints are enforced by this buy menu.
// Arguments:       True if mass constraints are enforced by this menu, false otherwise
// Return value:    None.

	bool EnforceMaxMassConstraint() const { return m_EnforceMaxMassConstraint; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetEnforceMaxMassConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether mass constraints are enforced by this buy menu.
// Arguments:       True to enforce mass constraints by this menu, false otherwise
// Return value:    None.

	void SetEnforceMaxMassConstraint(bool enforce) { m_EnforceMaxMassConstraint = enforce; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			AddAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an item to the list of allowed items.
//					If the list is not empty then everything not in the list is removed from the buy menu
//					Items will be removed from the buy menu when it's called, category changed or after a ForceRefresh().
// Arguments:       Full preset name to add.
// Return value:    None.

	void AddAllowedItem(std::string presetName) { m_AllowedItems[presetName] = true; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			RemoveAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an item from the list of allowed items.
// Arguments:       Full preset name to remove.
// Return value:    None.

	void RemoveAllowedItem(std::string presetName) { m_AllowedItems.erase(presetName); };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearAllowedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of allowed items
// Arguments:       None.
// Return value:    None.

	void ClearAllowedItems() { m_AllowedItems.clear(); };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the item is in allowed list
// Arguments:       Full preset name.
// Return value:    None.

	bool IsAllowedItem(std::string presetName) { return m_AllowedItems.find(presetName) != m_AllowedItems.end(); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			AddAlwaysAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an item to the list of always allowed items. This list overrides all previous constraints.
// Arguments:       Full preset name to add.
// Return value:    None.

	void AddAlwaysAllowedItem(std::string presetName) { m_AlwaysAllowedItems[presetName] = true; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			RemoveAlwaysAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an item from the list of always allowed items.
// Arguments:       Full preset name to remove.
// Return value:    None.

	void RemoveAlwaysAllowedItem(std::string presetName) { m_AlwaysAllowedItems.erase(presetName); };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearAlwaysAllowedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of allowed items
// Arguments:       None.
// Return value:    None.

	void ClearAlwaysAllowedItems() { m_AlwaysAllowedItems.clear(); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsAlwaysAllowedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the item is in always allowed list
// Arguments:       Full preset name.
// Return value:    None.

	bool IsAlwaysAllowedItem(std::string presetName) { return m_AlwaysAllowedItems.find(presetName) != m_AlwaysAllowedItems.end(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			AddProhibitedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an item prohibited to buy from the buy menu.
//					The item will be removed from the buy menu when it's called, category changed or after a ForceRefresh().
// Arguments:       Full preset name to add.
// Return value:    None.

	void AddProhibitedItem(std::string presetName) { m_ProhibitedItems[presetName] = true; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			RemoveProhibitedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes item from the list of prohibited items
// Arguments:       Full preset name to remove.
// Return value:    None.

	void RemoveProhibitedItem(std::string presetName) { m_ProhibitedItems.erase(presetName); };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearProhibitedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of prohibited items
// Arguments:       None.
// Return value:    None.

	void ClearProhibitedItems() { m_ProhibitedItems.clear(); };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsProhibitedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the item is in prohibited list
// Arguments:       Full preset name.
// Return value:    None.

	bool IsProhibitedItem(std::string presetName) { return m_ProhibitedItems.find(presetName) != m_ProhibitedItems.end(); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceRefresh
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces a refresh update of the list of buy menu items
// Arguments:       None.
// Return value:    None.

	void ForceRefresh() { CategoryChange(); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearCartList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clear the cart out of items selected for purchase
// Arguments:       None.
// Return value:    None.

	void ClearCartList();

    /// <summary>
    /// Adds an item to the cart.
    /// </summary>
    /// <param name="name">The name shown for the item.</param>
    /// <param name="rightText">The text that is shown right-aligned on the item (typically the cost of the item).</param>
    /// <param name="pBitmap">The sprite image rendered for the item. This takes ownership!</param>
    /// <param name="pEntity">The entity that this item refers to and will create when bought.</param>
    /// <param name="extraIndex">Extra index for special indexing or reference that the item is associated with. Menu-specific.</param>
    void AddCartItem(const std::string &name, const std::string &rightText = "", GUIBitmap *pBitmap = nullptr, const Entity *pEntity = 0, const int extraIndex = -1);

    /// <summary>
    /// Duplicates an item in the cart.
    /// </summary>
    /// <param name="itemIndex">The index of the item to duplicate.</param>
    void DuplicateCartItem(const int itemIndex);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			LoadDefaultLoadoutToCart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the default loadout to the cart
// Arguments:       None.
// Return value:    None.

	void LoadDefaultLoadoutToCart() { DeployLoadout(0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetOnlyShowOwnedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If set to true only owned items will be shown in buy menu. Overriden by AlwaysAllowed list.
// Arguments:       Value.
// Return value:    None.

	void SetOnlyShowOwnedItems(bool value) { m_OnlyShowOwnedItems = value; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetOnlyShowOwnedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether only owned items will be shown in buy menu. Overriden by AlwaysAllowed list.
// Arguments:       None.
// Return value:    Whether only owned items will be shown in buy menu.

	bool GetOnlyShowOwnedItems() const { return m_OnlyShowOwnedItems; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetOwnedItemsAmount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of specified items to be owned in this buy menu
// Arguments:       Full preset name of item to own. Amount of owned items.
// Return value:    None.

	void SetOwnedItemsAmount(std::string presetName, int amount) { m_OwnedItems[presetName] = amount; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetOwnedItemsAmount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the amount of specified items owned in this buy menu
// Arguments:       Full preset name of item.
// Return value:    Amount of owned items.

	int GetOwnedItemsAmount(std::string presetName) { if (m_OwnedItems.find(presetName) != m_OwnedItems.end()) return m_OwnedItems[presetName]; else return 0; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			CommitPurchase
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deducts 1 piece of owned item and return true if purchase can be made or false if the item is out of stock.
// Arguments:       Full preset name of item.
// Return value:    Whether the purchase can be conducted or the item is out of stock.

	bool CommitPurchase(std::string presetName);

#pragma region Faction Theme Handling
	/// <summary>
	/// Changes the banner image to the one specified. If none is specified, resets it to the default banner image.
	/// </summary>
	/// <param name="imagePath">Path to image to set as banner.</param>
	void SetBannerImage(const std::string &imagePath);

	/// <summary>
	/// Changes the logo image to the one specified. If none is specified, resets it to the default logo image.
	/// </summary>
	/// <param name="imagePath">Path to image to set as logo.</param>
	void SetLogoImage(const std::string &imagePath);
#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  CategoryChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all things that to happen when category is changed, happens.
// Arguments:       Wheter to change focus to the category tabs or not.
// Return value:    None.

    void CategoryChange(bool focusOnCategoryTabs = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  DeployLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the loadout set into the cart, replacing whatever's there now.
// Arguments:       The index of the loadout to load.
// Return value:    Whether it was loaded successfully or not.

    bool DeployLoadout(int index);


	/// <summary>
	/// Adds all objects of a specific type already defined in PresetMan to the current shop/item list. They will be grouped into the different data modules they were read from.
	/// </summary>
	/// <param name="moduleList">Reference to the data module vector of entity lists to add the items to.</param>
	/// <param name="type">The name of the class to add all objects of. "" or "All" looks for all.</param>
	/// <param name="groups">The name of the groups to add all objects of. An empty vector or "All" looks for all.</param>
	/// <param name="excludeGroups">Whether the specified groups should be excluded, meaning all objects NOT associated with the groups will be added.</param>
	void AddObjectsToItemList(std::vector<std::list<Entity *>> &moduleList, const std::string &type = "", const std::vector<std::string> &groups = {}, bool excludeGroups = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPresetsToItemList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all loadout presets' representations to the item GUI list.
// Arguments:       None.
// Return value:    None.

    void AddPresetsToItemList();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTotalCostLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text of the total cost label to reflect the total cost of
//                  all the items in teh order box.
// Arguments:       The team to display the total funds of.
// Return value:    None.

	void UpdateTotalCostLabel(int whichTeam = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTotalMassLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text of the specified label to reflect the total mass of
//                  all the items in the order box.
// Arguments:       Craft to read MaxMass from. Label to update.
// Return value:    None.

	void UpdateTotalMassLabel(const ACraft * pCraft, GUILabel * pLabel) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTotalPassengersLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text of the specified label to reflect the total passenger count of
//                  all the items in teh order box.
// Arguments:       Craft to read MaxPassengers from. Label to update.
// Return value:    None.

	void UpdateTotalPassengersLabel(const ACraft * pCraft, GUILabel * pLabel) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  TryPurchase
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Attempts to make a purchase with everything already set up.
// Arguments:       None.
// Return value:    None.

    void TryPurchase();


    enum MenuEnabled
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    enum MenuFocus
    {
        SETBUTTONS = 0,
        CATEGORIES,
        ITEMS,
        ORDER,
        OK,
		CLEARORDER,
        FOCUSCOUNT
    };

    enum MenuCategory
    {
        CRAFT = 0,
        BODIES,
		MECHA,
        TOOLS,
        GUNS,
        BOMBS,
        SHIELDS,
        SETS,
        CATEGORYCOUNT
    };

    enum BlinkMode
    {
        NOBLINK = 0,
        NOFUNDS,
        NOCRAFT,
		MAXPASSENGERS,
		MAXMASS,
        BLINKMODECOUNT
    };

    // Controls this Menu. Not owned
    Controller *m_pController;
    // GUI Screen for use by the in-game GUI
    GUIScreen *m_pGUIScreen;
    // Input controller
    GUIInput *m_pGUIInput;
    // The control manager which holds all the controls
    GUIControlManager *m_pGUIController;
    // Visibility state of each menu
    int m_MenuEnabled;
    // Focus state
    int m_MenuFocus;
    // Focus change direction - 0 is non,e negative is back, positive forward
    int m_FocusChange;
    // Category selection state
    int m_MenuCategory;
    // Speed at which the menus appear and disappear
    float m_MenuSpeed;
    // Which item in the currently focused list box we have selected
    int m_ListItemIndex;
    // Which item we're dragging
    int m_DraggedItemIndex;
    // Whether we're currently dragging
    bool m_IsDragging;
    // Which object was last hovered over by the mouse, to avoid repeatedly selecting hte same item over and over when mose only moves a pixel
    int m_LastHoveredMouseIndex;
    // Which item in each of the categories was last selected, so the scrolling doesn't have to be redone each time user flips back and forth
    int m_CategoryItemIndex[CATEGORYCOUNT];
    // Which metaplayer, if any, is using this menu
    int m_MetaPlayer;
    // The ID of the DataModule that contains the native Tech of the Player using this menu
    int m_NativeTechModule;
    // The multiplier of costs of any foreign tech items
    float m_ForeignCostMult;
    // Arry of bools showing which modules that have been expanded in the item list
    bool *m_aExpandedModules;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;
    // Measures real time to determine how fast the menu should animate when opening/closing to appear real time to the player
    Timer m_MenuTimer;
    // Measures the time to when to start repeating inputs when they're held down
    Timer m_RepeatStartTimer;
    // Measures the interval between input repeats
    Timer m_RepeatTimer;

    bool m_SelectingEquipment; //!< Whether or not the menu is in equipment mode.
    MenuCategory m_LastVisitedEquipmentTab; //!< The last tab visited while in equipment mode.
    MenuCategory m_LastVisitedMainTab; //!< The last tab visited while not in equipment mode.
    int m_LastEquipmentScrollPosition; //!< The last scroll position while in equipment mode.
    int m_LastMainScrollPosition; //!< The last scroll position while not in equipment mode.
    MenuCategory m_FirstMainTab; //!< The first enabled tab when not in equipment mode.
    MenuCategory m_LastMainTab; //!< The last enabled tab when not in equipment mode.
    MenuCategory m_FirstEquipmentTab; //!< The first enabled tab when in equipment mode.
    MenuCategory m_LastEquipmentTab; //!< The last enabled tab when in equipment mode.

    // Collection box of the buy GUIs
    GUICollectionBox *m_pParentBox;
    // Collection box of the buy popups that contain information about items
    GUICollectionBox *m_pPopupBox;
    // Label displaying the item popup description
    GUILabel *m_pPopupText;
    // Top banner
    GUICollectionBox *m_Banner;
    // Logo label that disappears when the sets category is selected
    GUICollectionBox *m_Logo;
    // All the radio buttons for the different shop categories
    GUITab *m_pCategoryTabs[CATEGORYCOUNT];
    // The Listbox which lists all the shop's items in the currently selected category
    GUIListBox *m_pShopList;
    // The Listbox which lists all the items currently in the shopping cart or order
    GUIListBox *m_pCartList;
    // The single-line textbox which shows the selected delivery craft
    GUITextBox *m_pCraftBox;

	// Panel with craft parameters
	GUICollectionBox *m_pCraftCollectionBox;

	// Selected craft name
	GUILabel *m_pCraftNameLabel;
	// Selected craft price
	GUILabel *m_pCraftPriceLabel;
	// Selected craft passenger caption
	GUILabel *m_pCraftPassengersCaptionLabel;
	// Selected craft passenger count
	GUILabel *m_pCraftPassengersLabel;
	// Selected craft total mass caption
	GUILabel *m_pCraftMassCaptionLabel;
	// Selected craft total mass
	GUILabel *m_pCraftMassLabel;

    // Label displaying "Delivered On:"
    GUILabel *m_pCraftLabel;
    // The selected craft instance for delivery
    const SceneObject *m_pSelectedCraft;
    // Label displaying the total cost
    GUILabel *m_pCostLabel;
    // The purchasing button
    GUIButton *m_pBuyButton;

	GUIButton *m_ClearOrderButton; //!< Button for clearing the cart.
    // The save set button
    GUIButton *m_pSaveButton;
    // The clear set button
    GUIButton *m_pClearButton;
    // Sets of user-defined loadouts that can be selected quickly.
    std::vector<Loadout> m_Loadouts;
    // Purchase has been made
    bool m_PurchaseMade;
	int m_DeliveryWidth; //!< The width of the currently selected delivery craft, which will determine the width of the LZ marker.
    // The cursor image shared by all buy menus
    static BITMAP *s_pCursor;
    // Screen position of the cursor
    Vector m_CursorPos;

	// If true UI won't afford to order a craft with more passengers than allowed by craft
	bool m_EnforceMaxPassengersConstraint;
	// If true UI won't afford to order a craft with more mass than allowed by craft
	bool m_EnforceMaxMassConstraint;

	// Only show items that owned
	bool m_OnlyShowOwnedItems;
	// If not empty then only shows items present in this list
	std::map<std::string, bool> m_AllowedItems;
	// If not empty then items from this list are always shown int he buy menu no matter what other constraints there are
	std::map<std::string, bool> m_AlwaysAllowedItems;
	// If not empty then removes items from ths list the buy menu
	std::map<std::string, bool> m_ProhibitedItems;
	// A map of owned items, for which the gold will not be deducted when bought
	std::map<std::string, int> m_OwnedItems;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

	static const std::string c_DefaultBannerImagePath; //!< Path to the default banner image.
	static const std::string c_DefaultLogoImagePath; //!< Path to the default logo image.

    /// <summary>
    /// Refresh tab disabled states, so tabs get properly enabled/disabled based on whether or not equipment selection mode is enabled.
    /// </summary>
    void RefreshTabDisabledStates();

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this BuyMenuGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	BuyMenuGUI(const BuyMenuGUI &reference) = delete;
	BuyMenuGUI & operator=(const BuyMenuGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File
