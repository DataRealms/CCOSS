#ifndef _OBJECTPICKERGUI_
#define _OBJECTPICKERGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ObjectPickerGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     ObjectPickerGUI class
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

#include <string>
#include <list>

struct BITMAP;


namespace RTE
{

class GUIScreen;
class GUIInput;
class GUIControlManager;
class GUICollectionBox;
class GUIListBox;
class GUITextBox;
class GUIButton;
class GUILabel;
class SceneObject;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ObjectPickerGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A GUI for picking isntance objects in Cortex Command
// Parent(s):       None.
// Class history:   7/16/2007 ObjectPickerGUI Created.

class ObjectPickerGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ObjectPickerGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ObjectPickerGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ObjectPickerGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ObjectPickerGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ObjectPickerGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~ObjectPickerGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ObjectPickerGUI object ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
//                  Which DataModule space to be picking from. -1 means pick from ALL
//                  objects loaded in all DataModule:s.
//                  Which lowest common denominator type to be showing.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController, int whichModuleSpace = -1, std::string onlyOfType = "All");


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ObjectPickerGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ObjectPickerGUI object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


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

    bool IsEnabled() const { return m_PickerEnabled == ENABLED || m_PickerEnabled == ENABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is at all visible or not.
// Arguments:       None.
// Return value:    None.

    bool IsVisible() const { return m_PickerEnabled != DISABLED; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.
// Arguments:       The new screen position of this entire GUI.

    void SetPosOnScreen(int newPosX, int newPosY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.
// Arguments:       The ID of the module to let the player pick objects from. All official
//                  modules' objects will always be presented, in addition to the one
//                  passed in here.
// Return value:    None.

    void SetModuleSpace(int moduleSpaceID = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowOnlyType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.
// Arguments:       The ID of the module to let the player pick objects from. All official
//                  modules' objects will always be presented, in addition to the one
//                  passed in here.
// Return value:    None.

    void ShowOnlyType(std::string showType = "All") { m_ShowType = showType; UpdateGroupsList(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShowSpecificGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes a group of a specific name show up in the picker, IF that group
//                  is currently shown!
// Arguments:       The name of the group to show in the picker.
// Return value:    Whether the group was found and switched to successfully.

    bool ShowSpecificGroup(std::string groupName);


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
// Method:          ObjectPicked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether and which Object has been picked by the player. This
//                  may happen even though the player isn't done with the picker. (ie
//                  a different object is picked each time the user selects something else
//                  in the objects list).
// Arguments:       None.
// Return value:    Whether an object has been picked bt the player. 0 if not. Ownership
//                  is NOT transferred!

    const SceneObject * ObjectPicked() { return m_pPickedObject; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DonePicking
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the user has finished using the picker, and the final
//                  picked Object is returned.
// Arguments:       None.
// Return value:    Whether an object has been positively and finally picked bt the player.
//                  0 if not. Ownership is NOT transferred!

    const SceneObject * DonePicking() { return !IsEnabled() && m_pPickedObject ? m_pPickedObject : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next object in the objects list, even if the picker is disabled.
// Arguments:       None.
// Return value:    The next object in the picker list, looping around if necessary.
//                  0 if no object can be selected.

    const SceneObject * GetNextObject();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the prev object in the objects list, even if the picker is disabled.
// Arguments:       None.
// Return value:    The prev object in the picker list, looping around if necessary.
//                  0 if no object can be selected.

    const SceneObject * GetPrevObject();


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
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateGroupsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all groups with a specific type already defined in PresetMan
//                  to the current Objects list
// Arguments:       None.
// Return value:    None.

    void UpdateGroupsList();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateObjectsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds all objects of the currently selected group to the Objects list.
// Arguments:       Whether to reset the selection to the top of the list when we're done
//                  updating this.
// Return value:    None.

    void UpdateObjectsList(bool selectTop = true);


    enum PickerEnabled
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    enum PickerFocus
    {
        GROUPS = 0,
        OBJECTS,
        FOCUSCOUNT
    };

    enum BlinkMode
    {
        NOBLINK = 0,
        NOFUNDS,
        NOCRAFT,
        BLINKMODECOUNT
    };

    // Controller which conrols this menu. Not owned
    Controller *m_pController;
    // GUI Screen for use by the in-game GUI
    GUIScreen *m_pGUIScreen;
    // Input controller
    GUIInput *m_pGUIInput;
    // The control manager which holds all the controls
    GUIControlManager *m_pGUIController;
    // Visibility state of the object picker
    int m_PickerEnabled;
    // Focus state
    int m_PickerFocus;
    // Focus change direction - 0 is non,e negative is back, positive forward
    int m_FocusChange;
    // Speed at which the menus appear and disappear
    float m_MenuSpeed;
    // The DataModule ID of the non-official module that this picker should be restricted to, in addition to all the official modules as well
    // If -1, the picker will be able to pick from ALL loaded datamodules.
    int m_ModuleSpaceID;
    // Only show objects of this type. "" or "All" will show objects of all types
    std::string m_ShowType;
    // Which Group in the groups list box we have selected
    int m_SelectedGroupIndex;
    // Which object in the Objects list box we have selected
    int m_SelectedObjectIndex;
    // Which object was last hovered over by the mouse in the objects list
    int m_LastHoveredMouseIndex;
    // The ID of the DataModule that contains the native Tech of the Player using this menu
    int m_NativeTechModule;
    // The multiplier of costs of any foreign tech items
    float m_ForeignCostMult;
    // The modules that have been expanded in the item list
    bool *m_aExpandedModules;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;
    // Measures the time to when to start repeating inputs when they're held down
    Timer m_RepeatStartTimer;
    // Measures the interval between input repeats
    Timer m_RepeatTimer;

    // Collection box of the puicker GUI
    GUICollectionBox *m_pParentBox;
    // Collection box of the buy popups that contain information about items
    GUICollectionBox *m_pPopupBox;
    // Label displaying the item popup description
    GUILabel *m_pPopupText;
    // The Listbox which lists all the groups
    GUIListBox *m_pGroupsList;
    // The Listbox which lists all the objects in the currently selected group
    GUIListBox *m_pObjectsList;
    // Currently picked object. This is 0 until the user actually picks somehting, not just has the cursor over it
    // Not owned by this.
    const SceneObject *m_pPickedObject;
    // The cursor image shared by all pickers
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
    Sound m_ObjectPickedSound;
    // Sound for erroneus input
    Sound m_UserErrorSound;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ObjectPickerGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ObjectPickerGUI(const ObjectPickerGUI &reference);
    ObjectPickerGUI & operator=(const ObjectPickerGUI &rhs);

};

} // namespace RTE

#endif  // File