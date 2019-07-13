#ifndef _SCHEMEEDITORGUI_
#define _SCHEMEEDITORGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SchemeEditorGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     SchemeEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Sound.h"
#include "Timer.h"
#include "Vector.h"
#include "Controller.h"

#include <string>
#include <list>

struct BITMAP;


namespace RTE
{

class SceneObject;
class ObjectPickerGUI;
class PieMenuGUI;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SchemeEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A full menu system that represents the scene editing GUI for Cortex Command
// Parent(s):       None.
// Class history:   7/08/2007 SchemeEditorGUI Created.

class SchemeEditorGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:
    // Different modes of this editor
    enum EditorGUIMode
    {
        INACTIVE = 0,
        PICKINGOBJECT,
        ADDINGOBJECT,
        PLACINGOBJECT,
        MOVINGOBJECT,
        DELETINGOBJECT,
        DONEEDITING,
        EDITORGUIMODECOUNT
    };

    enum FeatureSets
    {
        ONLOADEDIT = 0,
    };

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SchemeEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SchemeEditorGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SchemeEditorGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SchemeEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SchemeEditorGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~SchemeEditorGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SchemeEditorGUI object ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
//                  Whether the editor should have all the features enabled, like load/save
//                  and undo capabilities.
//                  Which module space that this eidtor will be able to pick objects from.
//                  -1 means all modules.
//                  Which Tech module that will be presented as the native one to the player.
//                  The multiplier of all foreign techs' costs.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController, FeatureSets featureSet = ONLOADEDIT, int whichModuleSpace = -1, int nativeTechModule = 0, float foreignCostMult = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SchemeEditorGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SchemeEditorGUI object.
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

    void SetController(Controller *pController);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.
// Arguments:       The new screen position of this entire GUI.

    void SetPosOnScreen(int newPosX, int newPosY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCursorPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the absolute scene coordinates of the cursor of this Editor.
// Arguments:       The new cursor position in absolute scene units.
// Return value:    None.

    void SetCursorPos(const Vector &newCursorPos) { m_CursorPos = newCursorPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the new Object to be held at the cursor of this Editor. Ownership
//                  IS transferred!
// Arguments:       The new Object to be held by the cursor. Ownership IS transferred!
// Return value:    Whether the cursor holds a valid object after setting.

    bool SetCurrentObject(SceneObject *pNewObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.
// Arguments:       None.
// Return value:    The enum'd int of any slice activated. See the PieSliceIndex enum.

    int GetActivatedPieSlice();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently held Object in the cursor of this Editor. Ownership
//                  IS NOT transferred!
// Arguments:       None.
// Return value:    The currently held object, if any. OINT!

    const SceneObject * GetCurrentObject() const { return m_pCurrentObject; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current mode of this editor.
// Arguments:       The new mode to set to, see the EditorGUIMode enum.
// Return value:    None.

    void SetEditorGUIMode(EditorGUIMode newMode) { m_EditorGUIMode = newMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mode of this editor.
// Arguments:       None.
// Return value:    The current mode this is set to; see the EditorGUIMode enum.

    EditorGUIMode GetEditorGUIMode() const { return m_EditorGUIMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which DataModule space to be picking objects from. If -1, then
//                  let the player pick from all loaded modules.
// Arguments:       The ID of the module to let the player pick objects from. All official
//                  modules' objects will alwayws be presented, in addition to the one
//                  passed in here.
// Return value:    None.

    void SetModuleSpace(int moduleSpaceID = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EditMade
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether an edit on the scene was made in the last Update.
// Arguments:       None.
// Return value:    Whether any edit was made.

    bool EditMade() const { return m_EditMade; }


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
// Description:     Draws the editor
// Arguments:       The bitmap to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePieMenu
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the PieMenu config based ont eh current editor state.
// Arguments:       None.
// Return value:    None.

    void UpdatePieMenu();



    enum BlinkMode
    {
        NOBLINK = 0,
        OBJECTBLINKON,
        OBJECTBLINKOFF,
        BLINKMODECOUNT
    };

    // Controller which conrols this menu. Not owned
    Controller *m_pController;
    // Full featured or the in-game version, or the base building mode
    int m_FeatureSet;
    // Whether an edit was made to the Scene in the last Update
    bool m_EditMade;
    // The current mode of the whole GUI. See EditorGUIMode enum.
    EditorGUIMode m_EditorGUIMode;
    // The previous mode of the whole GUI, to go back to when the current mode is done in some cases
    EditorGUIMode m_PreviousMode;
    // Whether the editor mode has changed
    bool m_ModeChanged;
    // Notification blink timer
    Timer m_BlinkTimer;
    // What we're blinking
    int m_BlinkMode;
    // Measures the time to when to start repeating inputs when they're held down
    Timer m_RepeatStartTimer;
    // Measures the interval between input repeats
    Timer m_RepeatTimer;


    // The pie menu
    PieMenuGUI *m_pPieMenu;
    // The object picker
    ObjectPickerGUI *m_pPicker;
    // The ID of the DataModule that contains the native Tech of the Player using this menu
    int m_NativeTechModule;
    // Grid snapping enabled
    bool m_GridSnapping;
    // Current cursor position, in absolute scene coordinates
    Vector m_CursorPos;
    // The offset from the current object's position to the cursor, if any
    Vector m_CursorOffset;
    // Cursor position in free air, or over something
    bool m_CursorInAir;
    // SceneObject facing left or not when placing
    bool m_FacingLeft;
    // The team of the placed SceneObject:s
    int m_PlaceTeam;
    // Currently held object. This is what is attached to the cursor and will be placed when the fire button is pressed
    // OWNED by this.
    SceneObject *m_pCurrentObject;
    // Where in the scene's list order the next object should be placed. If -1, then place at the end of the list.
    int m_ObjectListOrder;
    // Whether to draw the currently held object
    bool m_DrawCurrentObject;
    // Currently placed scene object to make blink when drawing it. NOT OWNED.
    const SceneObject *m_pObjectToBlink;
    // Path found between brain pos and the sky to make sure fair brain placement
    std::list<Vector> m_BrainSkyPath;
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
    // Sound for placement of object
    Sound m_PlacementBlip;
    // Sound for placement of object
    Sound m_PlacementThud;
    // Sound for gravely placement of object sound
    Sound m_PlacementGravel;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SchemeEditorGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    SchemeEditorGUI(const SchemeEditorGUI &reference);
    SchemeEditorGUI & operator=(const SchemeEditorGUI &rhs);

};

} // namespace RTE

#endif  // File
