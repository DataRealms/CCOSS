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
#include "Sound.h"
#include "Timer.h"
#include "Controller.h"
#include "Loadout.h"

#include <string>
#include <list>

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
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire BuyMenuGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the BuyMenuGUI object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  LoadAllLoadoutsFromFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads or re-loads all the loadout presets from the appropriate files
//                  on disk. This will first clear out all current loadout presets!
// Arguments:       None.
// Return value:    Success or not.

    virtual bool LoadAllLoadoutsFromFile();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SaveAllLoadoutsToFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves all the loadouts to appropriate file on disk. Does NOT save
//                  any named presets which will be loaded from the standard preset
//                  loadouts first anyway.
// Arguments:       None.
// Return value:    Success or not.

    virtual bool SaveAllLoadoutsToFile();


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

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNativeTechModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule ID should be treated as the native tech of the
//                  user of this menu.
// Arguments:       The module ID to set as the native one. 0 means everything is native.
// Return value:    None.

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
// Virtual Method:  SaveCurrentLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current loadout into a Set.
// Arguments:       None.
// Return value:    None.

    virtual void SaveCurrentLoadout();


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalOrderMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return teh total mass of everything listed in the order box.
// Arguments:       None.
// Return value:    The total mass in kg.

	float GetTotalOrderMass();


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

	int GetTotalOrderPassengers();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu
// Arguments:       The bitmap to draw on.
// Return value:    None.

    virtual void Draw(BITMAP *drawBitmap) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EnforceMaxPassengersConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether passenger count constraints are enforced by this buy menu.
// Arguments:       None.
// Return value:    True if passenger constraints are enforced by this menu, false otherwise

	bool EnforceMaxPassengersConstraint() const { return m_EnforceMaxPassengersConstraint; };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SetEnforceMaxPassengersConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether passenger count constraints are enforced by this buy menu.
// Arguments:       True to enforce passenger constraints by this menu, false otherwise
// Return value:    None.

	void SetEnforceMaxPassengersConstraint(bool enforce) { m_EnforceMaxPassengersConstraint = enforce; };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EnforceMaxMassConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether mass constraints are enforced by this buy menu.
// Arguments:       True if mass constraints are enforced by this menu, false otherwise
// Return value:    None.

	bool EnforceMaxMassConstraint() const { return m_EnforceMaxMassConstraint; };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SetEnforceMaxMassConstraint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether mass constraints are enforced by this buy menu.
// Arguments:       True to enforce mass constraints by this menu, false otherwise
// Return value:    None.

	void SetEnforceMaxMassConstraint(bool enforce) { m_EnforceMaxMassConstraint = enforce; };


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  FocusChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all things that to happen when focus is moved from one area
//                  or control to the next within the menu, will happen.
// Arguments:       None.
// Return value:    None.

    virtual void FocusChange();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  CategoryChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all things that to happen when category is changed, happens.
// Arguments:       Wheter to change focus to the category tabs or not.
// Return value:    None.

    virtual void CategoryChange(bool focusOnCategoryTabs = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  DeployLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the loadout set into the cart, replacing whatever's there now.
// Arguments:       The index of the loadout to load.
// Return value:    Whether it was loaded successfully or not.

    virtual bool DeployLoadout(int index);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddObjectsToItemList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all objects of a specific type already defined in PresetMan
//                  to the current shop/item list. They will be grouped into the different
//                  data modules they were read from.
// Arguments:       Reference to the data module vector of entity lists to add the items to.
//                  The name of the class to add all objects of. "" or "All" looks for all.
//                  The name of the group to add all objects of. "" or "All" looks for all.
// Return value:    None.

    void AddObjectsToItemList(std::vector<std::list<Entity *> > &moduleList, std::string type = "", std::string group = "");


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
//                  all the items in teh order box.
// Arguments:       Craft to read MaxMass from. Label to update.
// Return value:    None.

	void UpdateTotalMassLabel(const ACraft * pCraft, GUILabel * pLabel);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateTotalPassengersLabel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the text of the specified label to reflect the total passenger count of
//                  all the items in teh order box.
// Arguments:       Craft to read MaxPassengers from. Label to update.
// Return value:    None.

	void UpdateTotalPassengersLabel(const ACraft * pCraft, GUILabel * pLabel);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  TryPurchase
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
        FOCUSCOUNT
    };

    enum MenuCategory
    {
        CRAFT = 0,
        BODIES,
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

    // Collection box of the buy GUIs
    GUICollectionBox *m_pParentBox;
    // Collection box of the buy popups that contain information about items
    GUICollectionBox *m_pPopupBox;
    // Label displaying the item popup description
    GUILabel *m_pPopupText;
    // Logo label that disappears when the sets category is selected
    GUICollectionBox *m_pLogo;
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
    // The save set button
    GUIButton *m_pSaveButton;
    // The clear set button
    GUIButton *m_pClearButton;
    // Sets of user-defined loadouts that can be selected quickly.
    std::vector<Loadout> m_Loadouts;
    // Purchase has been made
    bool m_PurchaseMade;
    // The cursor image shared by all buy menus
    static BITMAP *s_pCursor;
    // Screen position of the cursor
    Vector m_CursorPos;
    // Sound for enabling and disabling menu
    Sound m_EnterMenuSound;
    Sound m_ExitMenuSound;
    // Sound for changing focus
    Sound m_FocusChangeSound;
    // Sound for selecting items in list, etc.
    Sound m_SelectionChangeSound;
    // Sound for adding or deleting items in list.
    Sound m_ItemChangeSound;
    // Sound for making a purchase focus
    Sound m_PurchaseMadeSound;
    // Sound for erroneus input
    Sound m_UserErrorSound;

	// If true UI won't afford to order a craft with more passengers than allowed by craft
	bool m_EnforceMaxPassengersConstraint;
	// If true UI won't afford to order a craft with more mass than allowed by craft
	bool m_EnforceMaxMassConstraint;




//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this BuyMenuGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    BuyMenuGUI(const BuyMenuGUI &reference);
    BuyMenuGUI & operator=(const BuyMenuGUI &rhs);

};

} // namespace RTE

#endif  // File