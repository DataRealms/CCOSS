#ifndef _GIBEDITORGUI_
#define _GIBEDITORGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GibEditorGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GibEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Timer.h"
#include "Vector.h"
#include "Controller.h"

struct BITMAP;


namespace RTE
{

class MovableObject;
class MOSRotating;
class ObjectPickerGUI;
class PieMenuGUI;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GibEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A placement part of the gib editor which manages the pie menu and picker.
// Parent(s):       None.
// Class history:   9/16/2007 GibEditorGUI Created.

class GibEditorGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    // Different modes of this editor
    enum EditorGUIMode
    {
        INACTIVE = 0,
        PICKOBJECTTOLOAD,
        PICKINGGIB,
        ADDINGGIB,
        PLACINGGIB,
        MOVINGGIB,
        DELETINGGIB,
        PLACEINFRONT,
        PLACEBEHIND,
        DONEEDITING,
        EDITORGUIMODECOUNT
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GibEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GibEditorGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GibEditorGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GibEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GibEditorGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~GibEditorGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GibEditorGUI object ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
//                  Which module space that this eidtor will be able to pick objects from.
//                  -1 means all modules.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController, int whichModuleSpace = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GibEditorGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GibEditorGUI object.
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
// Method:          SetCurrentGib
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the new Object to be held at the gib cursor of this Editor. Ownership
//                  IS transferred!
// Arguments:       The new Object to be held by the cursor. Ownership IS transferred!
// Return value:    None.

    void SetCurrentGib(MovableObject *pNewGibObject) { m_pCurrentGib = pNewGibObject; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.
// Arguments:       None.
// Return value:    The enum'd int of any slice activated. See the PieSliceIndex enum.

    int GetActivatedPieSlice();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlacedGibs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of already placed gibs of the currently edited object.
//                  Ownership of neither list not objects IS NOT transferred!
// Arguments:       None.
// Return value:    The current list of placed gibs. OWNERSHIP IS NOT TRANSFERRED!

    std::list<MovableObject *> * GetPlacedGibs() { return &m_PlacedGibs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentGib
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently held Object in the cursor of this Editor. Ownership
//                  IS NOT transferred!
// Arguments:       None.
// Return value:    The currently held object, if any. OWNERSHIP IS NOT TRANSFERRED!

    const MovableObject * GetCurrentGib() { return m_pCurrentGib; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetObjectToLoad
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a refernece instance of an object if it has been picked to be
//                  loaded into the editor. Ownership is NOT transferred.
// Arguments:       None.
// Return value:    Reference instance of the picked object to be loaded. 0 if nothing. OWNERSHIP IS NOT TRANSFERRED!
//                  was picked since last update.

    const MOSRotating * GetObjectToLoad() { return m_pObjectToLoad; }


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

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the editor
// Arguments:       The bitmap to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) const;


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to be placed in the editor. Ownership IS transferred!
// Arguments:       The MovableOjbect instace to add, OIT!
//                  Where in the list the object should be inserted. -1 means at the end
//                  of the list.
// Return value:    None.

    void AddPlacedObject(MovableObject *pObjectToAdd, int listOrder = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemovePlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject placed in this editor.
// Arguments:       The list order number of the object to remove. If -1, the last one is removed.
// Return value:    None.

    void RemovePlacedObject(int whichToRemove = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed object that graphically overlaps an absolute
//                  point in the scene. Note that the placed object don't really exist in
//                  the scene, but in the editor. Their own Pos's are used.
// Arguments:       The point in absolute scene coordinates that will be used to pick the
//                  last placed MovableObject which overlaps it.
//                  An int which will be filled out with the order place of any found object
//                  in the list. if nothing is found, it will get a value of -1.
// Return value:    The last hit MovableObject, if any. Ownership is NOT transferred!

    const MovableObject * PickPlacedObject(Vector &scenePoint, int *pListOrderPlace = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updated the objects in the placed scene objects list of this. This is
//                  mostly for the editor to represent the items correctly.
// Arguments:       None.
// Return value:    None.

    void UpdatePlacedObjects();


    enum BlinkMode
    {
        NOBLINK = 0,
        OBJECTBLINKON,
        OBJECTBLINKOFF,
        BLINKMODECOUNT
    };

    // Controller which conrols this menu. Not owned
    Controller *m_pController;
    // Whether an editor was made to the Scene in the last Update
    bool m_EditMade;
    // The current mode of the whole GUI. See EditorGUIMode enum.
    EditorGUIMode m_EditorGUIMode;
    // The previous mode of the whole GUI, to go back to when the current mode is done in some cases
    EditorGUIMode m_PreviousMode;
    // The ref instance picked to be loaded into the editor from the picker. 0 if none has been yet. Not owned.
    const MOSRotating *m_pObjectToLoad;
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
    // Grid snapping enabled
    bool m_GridSnapping;
    // The zooming bitmaps owned by this; source gets the area from the screen, destination is the zoomed in view
    BITMAP *m_pZoomSource;
    // The zoom factor of the magnifying window. 0 means no zoom window
    int m_ZoomFactor;
    // Current cursor position, in absolute scene coordinates
    Vector m_CursorPos;
    // The offset from the current object's position to the cursor, if any
    Vector m_CursorOffset;
    // Cursor position in free air, or over something
    bool m_CursorInAir;
    // Gib facing left or not when placing
    bool m_FacingLeft;
    // List of all the gibs currently placed on the edited object.
    // They are proxies of the actual gib list of the edited object, and are OWNED by editor.
    std::list<MovableObject *> m_PlacedGibs;
    // Currently held object to place as a gib. This is what is attached to the cursor and will be placed when the fire button is pressed
    // OWNED by this.
    MovableObject *m_pCurrentGib;
    // Where in the scene's list order the next object should be placed. If -1, then place at the end of the list.
    int m_GibListOrder;
    // Whether to draw the currently held object
    bool m_DrawCurrentGib;
    // Currently placed scene object to make blink when drawing it. NOT OWNED.
    const MovableObject *m_pObjectToBlink;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GibEditorGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	GibEditorGUI(const GibEditorGUI &reference) = delete;
	GibEditorGUI & operator=(const GibEditorGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File