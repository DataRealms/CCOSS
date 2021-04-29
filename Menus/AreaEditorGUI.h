#ifndef _AREAEDITORGUI_
#define _AREAEDITORGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            AreaEditorGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     AreaEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Scene.h"
#include "Timer.h"
#include "Vector.h"
#include "Box.h"
#include "Controller.h"
#include "PieSlice.h"

struct BITMAP;


namespace RTE
{

class AreaPickerGUI;
class PieMenuGUI;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           AreaEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A full menu system that represents the scene editing GUI for Cortex Command
// Parent(s):       None.
// Class history:   7/08/2007 AreaEditorGUI Created.

class AreaEditorGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    // Different modes of this editor
    enum EditorGUIMode
    {
        INACTIVE = 0,
        PICKINGAREA,
        PREADDMOVEBOX,
        ADDINGBOX,
        MOVINGBOX,
        DELETINGBOX,
        DONEEDITING,
        EDITORGUIMODECOUNT
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AreaEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AreaEditorGUI Area in system
//                  memory. Create() should be called before using the Area.
// Arguments:       None.

    AreaEditorGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AreaEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AreaEditorGUI Area before deletion
//                  from system memory.
// Arguments:       None.

    ~AreaEditorGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AreaEditorGUI Area ready for use.
// Arguments:       A poitner to a Controller which will control this Menu. Ownership is
//                  NOT TRANSFERRED!
//                  Whether the editor should have all the features enabled, like load/save
//                  and undo capabilities.
//                  Which module space that this eidtor will be able to pick Areas from.
//                  -1 means all modules.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(Controller *pController, bool fullFeatured = false, int whichModuleSpace = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire AreaEditorGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AreaEditorGUI Area.
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
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.
// Arguments:       None.
// Return value:    The enum'd int of any slice activated. See the PieSliceIndex enum.

    PieSlice::PieSliceIndex GetActivatedPieSlice();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the currently selected Area of this Editor. Ownership IS NOT
//                  transferred!
// Arguments:       The new area for this to work with, if any. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    None.

    void SetCurrentArea(Scene::Area *pArea);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently held Area in the cursor of this Editor. Ownership
//                  IS NOT transferred!
// Arguments:       None.
// Return value:    The currently held Area, if any. OWNERSHIP IS NOT TRANSFERRED!

    Scene::Area * GetCurrentArea() { return m_pCurrentArea; }


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
// Method:          EditMade
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether an edit on the scene was made in the last Update.
// Arguments:       None.
// Return value:    Whether any edit was made.

    bool EditMade() const { return m_EditMade; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePickerList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the list that the GUI's Area picker has, from the current
//                  scene state.
// Arguments:       The name of the Area to leave selected after the list is updated.
// Return value:    None.

	void UpdatePickerList(std::string selectAreaName = "");


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


    enum BlinkMode
    {
        NOBLINK = 0,
        OBJECTBLINKON,
        OBJECTBLINKOFF,
        BLINKMODECOUNT
    };

    // Controller which conrols this menu. Not owned
    Controller *m_pController;
    // Full featured or the in-game version
    bool m_FullFeatured;
    // Whether an editor was made to the Scene in the last Update
    bool m_EditMade;
    // The current mode of the whole GUI. See EditorGUIMode enum.
    EditorGUIMode m_EditorGUIMode;
    // The previous mode of the whole GUI, to go back to when the current mode is done in some cases
    EditorGUIMode m_PreviousMode;
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
    // The Area picker
    AreaPickerGUI *m_pPicker;
    // Grid snapping enabled
    bool m_GridSnapping;
    // Current cursor position, in absolute scene coordinates
    Vector m_CursorPos;
    // The offset from the currently dragged Box's corner position to the cursor, if any
    Vector m_CursorOffset;
    // Cursor position in free air, or over something
    bool m_CursorInAir;
    // Currently selected Area. NOT OWNED BY THIS
    Scene::Area *m_pCurrentArea;
    // Whether to draw the currently held Area
    bool m_DrawCurrentArea;
    // The Box currently being added/moved
    Box m_EditedBox;
    // Currently placed scene Area to make blink when drawing it. NOT OWNED.
    const Box *m_pBoxToBlink;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AreaEditorGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	AreaEditorGUI(const AreaEditorGUI &reference) = delete;
	AreaEditorGUI & operator=(const AreaEditorGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File