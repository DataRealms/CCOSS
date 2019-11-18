#ifndef _PIEMENUGUI_
#define _PIEMENUGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            PieMenuGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     PieMenuGUI class
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
#include "Icon.h"

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
class Actor;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           PieMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A flexible GUI for doing any command or toggle or action, anywhere
// Parent(s):       None.
// Class history:   8/21/2007 PieMenuGUI Created.

class PieMenuGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    enum PieSliceIndex
    {
        PSI_NONE = 0,
        // Inventory management
        PSI_PICKUP,
        PSI_DROP,
        PSI_NEXTITEM,
        PSI_PREVITEM,
        PSI_RELOAD,
        // Menu and GUI activations
        PSI_BUYMENU,
        PSI_STATS,
        PSI_MINIMAP,
        PSI_FORMSQUAD,
        PSI_CEASEFIRE,
        // AI Modes
        PSI_SENTRY,
        PSI_PATROL,
        PSI_BRAINHUNT,
        PSI_GOLDDIG,
        PSI_GOTO,
        PSI_RETURN,
        PSI_STAY,
        PSI_DELIVER,
        PSI_SCUTTLE,
        // Editor stuff
        PSI_DONE,
        PSI_LOAD,
        PSI_SAVE,
        PSI_NEW,
        PSI_PICK,
        PSI_MOVE,
        PSI_REMOVE,
        PSI_INFRONT,
        PSI_BEHIND,
        PSI_ZOOMIN,
        PSI_ZOOMOUT,
        PSI_TEAM1,
        PSI_TEAM2,
        PSI_TEAM3,
        PSI_TEAM4,
        // Custom
        PSI_SCRIPTED,
        // How many pie slice types there are
        PSI_COUNT
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    Slice
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     An option on the pie menu that does something
    // Parent(s):       Serializable.
    // Class history:   8/21/2007 Slice created.

    class Slice:
        public Serializable
    {


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

    enum SliceDirection
    {
        NONE = 0,
        UP,
        RIGHT,
        DOWN,
        LEFT
    };


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Slice
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Slice object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Slice() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Slice
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Slice object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Slice(std::string desc, PieSliceIndex sliceType, int direction, bool enabled = true)
        {
            Clear();
            m_Description = desc;
            m_SliceType = sliceType;
            m_Direction = direction;
            m_Enabled = enabled;
        }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a Slice to be identical to another, by deep copy.
    // Arguments:       A reference to the Slice to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(const Slice &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Slice object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling success or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Serializable ready for use.
    // Arguments:       A Reader that the Serializable will create itself from.
    //                  Whether there is a class name in the stream to check against to make
    //                  sure the correct type is being read from the stream.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true)
        {
            // If we're being read from a file, we are ALWAYS scripted!
            m_SliceType = PSI_SCRIPTED;
            return Serializable::Create(reader, checkType, doCreate);
        }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  ReadProperty
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Reads a property value from a Reader stream. If the name isn't
    //                  recognized by this class, then ReadProperty of the parent class
    //                  is called. If the property isn't recognized by any of the base classes,
    //                  false is returned, and the Reader's position is untouched.
    // Arguments:       The name of the property to be read.
    //                  A Reader lined up to the value of the property to be read.
    // Return value:    An error return value signaling whether the property was successfully
    //                  read or not. 0 means it was read successfully, and any nonzero indicates
    //                  that a property of that name could not be found in this or base classes.

        virtual int ReadProperty(std::string propName, Reader &reader);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        virtual void Reset() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Save
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Saves the complete state of this Slice to an output stream for
    //                  later recreation with Create(Reader &reader);
    // Arguments:       A Writer that the Slice will save itself with.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Save(Writer &writer) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetClassName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the class name of this Entity.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted type name of this object.

        virtual const std::string & GetClassName() const { return m_sClassName; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetScriptPath
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the file path of the scripted file this should run when activated
    //                  IF a scripted pie menu.
    // Arguments:       None.
    // Return value:    The file path to the script file this should load when activated.

        std::string GetScriptPath() const { return m_ScriptPath; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetFunctionName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the name of the Lua function to run when this is activated as a
    //                  scripted pie menu option.
    // Arguments:       None.
    // Return value:    The Lua function name this should execute when activated.

        std::string GetFunctionName() const { return m_FunctionName; }


		std::string GetDescription() const { return m_Description; }

		PieSliceIndex GetType() const { return m_SliceType; }

		SliceDirection GetDirection() const { return (SliceDirection)m_Direction; }


        // Member variables
        static const std::string m_sClassName;
        // Description of what this slice option does
        std::string m_Description;
        // The Slice type, also serves as icon index
        PieMenuGUI::PieSliceIndex m_SliceType;
        // The icon of this pie slice
        Icon m_Icon;
        // Enabled, or disabled (greyed out)?
        bool m_Enabled;
        // Highlighted on the circle by the cursor
        bool m_Highlighted;
        // The desired direction/location of this on the pie menu
        int m_Direction;
        // The grouping of this with other slices, assigned by the pie
        int m_GroupID;
        // The start angle of this' area on the pie menu, counted in radians from straight out right and going counter clockwise
        float m_AreaStart;
        // The arc length of the slice area, so that the icon should be drawn at the areastart + halfway of this
        float m_AreaArc;
        // Mid angle, basically m_AreaStart + (m_AreaArc / 2)
        float m_MidAngle;
        // Path to the script file this should run if this is a scripted selection
        std::string m_ScriptPath;
        // Name of the function in the script that this should run if a scripted pie option
        std::string m_FunctionName;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Slice, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };

    // Nested class:    PieSubMenu
    // Description: A submenu for the pie menu with some contents, opened by an appropriate pie slice

    class PieSubMenu:
        public Serializable
    {


    public:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     PieSubMenu
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a PieSubMenu object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        PieSubMenu() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a PieSubMenu to be identical to another, by deep copy.
    // Arguments:       A reference to the PieSubMenu to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(const PieSubMenu &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the PieSubMenu object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling success or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Serializable ready for use.
    // Arguments:       A Reader that the Serializable will create itself from.
    //                  Whether there is a class name in the stream to check against to make
    //                  sure the correct type is being read from the stream.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true)
        {
            return Serializable::Create(reader, checkType, doCreate);
        }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  ReadProperty
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Reads a property value from a Reader stream. If the name isn't
    //                  recognized by this class, then ReadProperty of the parent class
    //                  is called. If the property isn't recognized by any of the base classes,
    //                  false is returned, and the Reader's position is untouched.
    // Arguments:       The name of the property to be read.
    //                  A Reader lined up to the value of the property to be read.
    // Return value:    An error return value signaling whether the property was successfully
    //                  read or not. 0 means it was read successfully, and any nonzero indicates
    //                  that a property of that name could not be found in this or base classes.

        virtual int ReadProperty(std::string propName, Reader &reader);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        virtual void Reset() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Save
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Saves the complete state of this PieSubMenu to an output stream for
    //                  later recreation with Create(Reader &reader);
    // Arguments:       A Writer that the Slice will save itself with.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Save(Writer &writer) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetClassName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the class name of this Entity.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted type name of this object.

        virtual const std::string & GetClassName() const { return m_sClassName; }

 
    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetMenuName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the identifying MenuName of this SubMenu.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted MenuName of this SubMenu.

		std::string GetMenuName() const { return m_MenuName; }

        
    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetModuleID
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets the module this Entity was supposed to be defined in.
    // Arguments:       The ID of the module, or -1 if it hasn't been defined in any.
    // Return value:    None.

        void SetModuleID(int whichModule) { m_DefinedInModule = whichModule; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetModuleID
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows the ID of the DataModule this Entity has been defined in.
    // Arguments:       None.
    // Return value:    The ID of the module, or -1 if it hasn't been defined in any.

        int GetModuleID() const { return m_DefinedInModule; }


        // Member variables
        static const std::string m_sClassName;
        // Name of menu, used to internally identify it
        std::string m_MenuName;
        // The DataModule ID that this was defined in, not added or handled like Entity so this should never be -1
        int m_DefinedInModule;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Slice, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     PieMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a PieMenuGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    PieMenuGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~PieMenuGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a PieMenuGUI object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~PieMenuGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the PieMenuGUI object ready for use.
// Arguments:       A pointer to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
//                  The actor that this menu is currently associated with. Ownership is NOT
//                  TRANSFERRED! This is optional.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController, Actor *pFocusActor = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire PieMenuGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the PieMenuGUI object.
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
// Method:          SetActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the currently attached-to Actor. It will populate this menu with
//                  some of its options. The ownership of the Actor is NOT transferred!
// Arguments:       The new actor associated for this menu. Ownership is NOT transferred
// Return value:    None.

    void SetActor(Actor *pActor) { m_pActor = pActor; m_pLastKnownActor = pActor;}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the menu. This will animate it in and out of view.
// Arguments:       Whether to enable or disable the menu.
// Return value:    None.

    void SetEnabled(bool enable = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DisableAnim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Just plays the disabling animation, regardless of whether the menu
//                  was enabled or not. This is for an effect when actors are switched.
// Arguments:       None.
// Return value:    None.

    void DisableAnim() { m_InnerRadius = m_EnabledRadius; m_Wobbling = false; m_EnablingTimer.Reset(); m_PieEnabled = DISABLING; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WobbleAnim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Plays an animation of the pie menu circle expanding and contracting
//                  continuously. The menu is effectively disabled while doing this.
//                  It will continue until the next call to SetEnabled.
// Arguments:       None.
// Return value:    None.

    void WobbleAnim() { m_Wobbling = true; m_Freeze = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FreezeAtRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the background circle freeze at a certain radius until SetEnabled
//                  is called. The menu is effectively disabled while doing this.
// Arguments:       None.
// Return value:    None.

    void FreezeAtRadius(int radius) { m_Freeze = true; m_Wobbling = false; m_InnerRadius = radius; m_RedrawBG = true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is enabled or not.
// Arguments:       None.
// Return value:    None.

    bool IsEnabled() const { return (m_PieEnabled == ENABLED || m_PieEnabled == ENABLING) && !m_Wobbling; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether the menu is at all visible or not.
// Arguments:       None.
// Return value:    None.

    bool IsVisible() const { return m_PieEnabled != DISABLED; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute center position of this.
// Arguments:       None.
// Return value:    A Vector describing the current absolute position in pixels.

    const Vector & GetPos() const { return m_CenterPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the absolute center position of this in the scene.
// Arguments:       A Vector describing the current absolute position in pixels, in the scene.
// Return value:    None.

    void SetPos(const Vector &newPos) { m_CenterPos = newPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets and removes all slices from the menu, so that new configuration
//                  of them can be added.
// Arguments:       None.
// Return value:    None.

    void ResetSlices();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Slice to the menu. It will be placed according to what's already
//                  in there, and what placement apriority parameters it has.
// Arguments:       The new slice to add.
//                  Whether the new slice can be placed on the closest free cardinal if the
//                  one specified in it isn't free. If false, it will be placed in a corner
//                  spot as close to its desired direction as possible.
// Return value:    Whetehr the slice was added successfully. If tehre wasn't enough room
//                  or there was duplicate slice, then this will return false.

    bool AddSlice(Slice &newSlice, bool takeAnyFreeCardinal = false);


	bool AddSliceLua(string description, string functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled);

	Slice RemoveSliceLua(string description, string functionName);

	void AlterSliceLua(string description, string functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled);

	static void AddAvailableSlice(Slice newSlice) { m_AllAvailableSlices[newSlice.m_Description + "::" + newSlice.m_FunctionName] = newSlice; }

	//std::vector<Slice> GetAvailableSlices() const { return m_AllAvailableSlices; };

	std::vector<Slice *> GetCurrentSlices() const { return m_AllSlices; };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSliceToGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Slice to the menu, as part of a group. It will be assigned a
//                  group ID, which will also be returned. If a new group is wanted, pass
//                  -1 as groupID.
// Arguments:       The new slice to add. Ownership IS NOT transferred!
//                  The groupID to add the Slice to. If a new group is wanted, pass -1.
// Return value:    The ID of the group the slice was added to, or -1 if adding failed.

    int AddSliceToGroup(Slice &newSlice, int groupID = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RealignSlices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure all currently added slices' areas are set up correctly
//                  position and sizewise on the pie menu circle.
// Arguments:       None.
// Return value:    None.

    void RealignSlices();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSliceOnAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific slice based on an angle on the pie menu.
// Arguments:       An angle on the circle, in radins, CCW from straight out right.
// Return value:    The Slice which exists on that angle, if any. 0 if not. Ownership
//                  is NOT transferred!

    const Slice * GetSliceOnAngle(float angle);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SliceActivated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether and which Slice has been activated by the player. This
//                  may happen even though the player isn't done with the pie menu.
// Arguments:       None.
// Return value:    The Slice which has been picked by the player, if any. 0 if not. Ownership
//                  is NOT transferred!

    const Slice * SliceActivated() { return m_pActivatedSlice; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPieCommand
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reports whether and which Slice command has been activated by the
//                  player during last update. This may happen even though the player
//                  isn't done with the pie menu.
// Arguments:       None.
// Return value:    The Slice type which has been picked by the player, if any. 0 if not.
//                  See the PieSliceIndex enum.

    int GetPieCommand() { return m_PieCommand; }


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
// Arguments:       A pointer to a BITMAP to draw on. OINT.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) const;




//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a slice to be selected.
// Arguments:       The slice to be selected. Has to be a slice currently in this - OINT!
//                  Whether to also move the cursor to the center of the newly selected slice.
// Return value:    Whether this resulted in a different actual slice being selected (not 0).

    bool SelectSlice(Slice *pSelected, bool moveCursor = false);


    enum PieEnabled
    {
        ENABLING = 0,
        ENABLED,
        DISABLING,
        DISABLED
    };

    enum BlinkMode
    {
        NOBLINK = 0,
        NOFUNDS,
        NOCRAFT,
        BLINKMODECOUNT
    };

    enum PieIconSelection
    {
        PIS_NORMAL = 0,
        PIS_SELECTED,
        PIS_DISABLED,
        PIS_COUNT
    };

    // The cursor
    static BITMAP *s_pCursor;
//    static BITMAP *s_pCursorGlow;

    // Controller which controls this menu. Not owned
    Controller *m_pController;
    // The actor that this menu is attached to and getting some options from
    Actor *m_pActor;
    // This actor pointer is not cleared evety time, I'm not touching the original to not ruin something
    Actor *m_pLastKnownActor;
    // The center position of this in the scene
    Vector m_CenterPos;
    // Visibility state of the object picker
    int m_PieEnabled;
    // Currently hovered over slice
    const Slice *m_pHoveredSlice;
    // Currently activated slice, when a user chooses one. Is 0 always otherwise, even if a slice is hovered over.
    const Slice *m_pActivatedSlice;
    // The index of the activated slice since last update
    PieSliceIndex m_PieCommand;
    // Indicates which command was already activated with primary button, so it shouldn't be activated again when pie menu is disabled.
    PieSliceIndex m_AlreadyActivated;
    // Timer to measure how long to hold a hovered over slice
    Timer m_HoverTimer;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;

    // The cardinal axis slices, owned here
    Slice m_UpSlice;
    Slice m_LeftSlice;
    Slice m_DownSlice;
    Slice m_RightSlice;

    // The slices between the cardinal axes, owned here
    std::list<Slice> m_UpRightSlices;
    std::list<Slice> m_UpLeftSlices;
    std::list<Slice> m_DownLeftSlices;
    std::list<Slice> m_DownRightSlices;
    std::list<string> m_hithere;
    std::vector<string> m_hothere;

	// All slices ever added to this pie-menu, serves as directory of slices available to add
	static std::map<string, Slice> m_AllAvailableSlices;

    // All the slices, in order and aligned, not owned here, just pointing to the ones above
    std::vector<Slice *> m_AllSlices;
    // How many groups there currently are
    int m_SliceGroupCount;

    // Time in ms for how long it takes to enable/disable
    static const int s_EnablingDelay;
    // Timer for the appear and disappear animations
    Timer m_EnablingTimer;
    // Special mode where the menu circle expands and contracts continuously, while being effectively disabled
    bool m_Wobbling;
    // Special mode where the menu circle is frozen at the current m_InnerRadius.
    bool m_Freeze;
    // The current radius of the innermost circle of the pie menu, in pixels
    int m_InnerRadius;
    // When fully enabled, the inner radius is this, in pixels
    int m_EnabledRadius;
    // The thickness of the pie menu circle, in pixels
    int m_Thickness;
    // The intermediary bitmap used to first draw the menu background, which will be blitted to the final draw target surface
    BITMAP *m_pBGBitmap;
    // Whether we need to redraw the BG bitmap
    bool m_RedrawBG;
    // Position of the cursor on the circle, in radians, counterclockwise from straight out to rhe right
    float m_CursorAng;
    // Whether there is enough analog input magnitude to allow a direction to be ascertained
    bool m_EnoughInput;
    // Times how long after digital input the enoughinput signal is given
    Timer m_DInputHoldTimer;
    
    // Sound for enabling and disabling menu
    Sound m_EnterMenuSound;
    // For when no slice was picked
    Sound m_ExitMenuSound;
    // Sound for when hover arrow appears or changes slice
    Sound m_HoverChangeSound;
    // Sound for when hover arrow appears or changes to a disabled slice
    Sound m_HoverDisabledSound;
    // Sound for when hover arrow disappears forn not enough input
    Sound m_HoverGoneSound;
    // Sound for making picking a valid slice
    Sound m_SlicePickedSound;
    // Sound for erroneus input
    Sound m_DisabledPickedSound;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this PieMenuGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    PieMenuGUI(const PieMenuGUI &reference);
    PieMenuGUI & operator=(const PieMenuGUI &rhs);

};

} // namespace RTE

#endif  // File