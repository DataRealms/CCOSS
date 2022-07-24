#ifndef _SCENEEDITORGUI_
#define _SCENEEDITORGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneEditorGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     SceneEditorGUI class
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
#include "PieSlice.h"

struct BITMAP;


namespace RTE
{

class SceneObject;
class ObjectPickerGUI;
class PieMenuGUI;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SceneEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A full menu system that represents the scene editing GUI for Cortex Command
// Parent(s):       None.
// Class history:   7/08/2007 SceneEditorGUI Created.

class SceneEditorGUI {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    enum FeatureSets
    {
        ONLOADEDIT = 0,
        BLUEPRINTEDIT,
        AIPLANEDIT,
        INGAMEEDIT
    };

    // Different modes of this editor
    enum EditorGUIMode
    {
        INACTIVE = 0,
        PICKINGOBJECT,
        ADDINGOBJECT,
        INSTALLINGBRAIN,
        PLACINGOBJECT,
        MOVINGOBJECT,
        DELETINGOBJECT,
        PLACEINFRONT,
        PLACEBEHIND,
        DONEEDITING,
        EDITORGUIMODECOUNT
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SceneEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SceneEditorGUI object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SceneEditorGUI() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SceneEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SceneEditorGUI object before deletion
//                  from system memory.
// Arguments:       None.

    ~SceneEditorGUI() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneEditorGUI object ready for use.
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

    int Create(Controller *pController, FeatureSets featureSet = INGAMEEDIT, int whichModuleSpace = -1, int nativeTechModule = 0, float foreignCostMult = 1.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SceneEditorGUI, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneEditorGUI object.
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

    PieSlice::PieSliceIndex GetActivatedPieSlice() { return m_ActivatedPieSliceType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently held Object in the cursor of this Editor. Ownership
//                  IS NOT transferred!
// Arguments:       None.
// Return value:    The currently held object, if any. OWNERSHIP IS NOT TRANSFERRED!

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

    void SetForeignCostMultiplier(float newMultiplier);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EditMade
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether an edit on the scene was made in the last Update.
// Arguments:       None.
// Return value:    Whether any edit was made.

    bool EditMade() const { return m_EditMade; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TestBrainResidence
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether the resident brain is currently placed into a valid
//                  location in this scene, based on whether there is a clear path to the
//                  sky above it. This forces the editor into place brain mode with the
//                  current resident brain if the current placement is no bueno. It also
//                  removes the faulty brain from residence in the scene!
// Arguments:       Whether it's OK if we dont' have a brain right now - ie don't force
//                  into isntallation mode if no brain was found.
// Return value:    Whether a resident brain was found, AND found in a valid location!

    bool TestBrainResidence(bool noBrainIsOK = false);


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

	/// <summary>
	/// Updates the path to the current brain in the cursor or resident in the scene, if any. If there's none, the path is cleared.
	/// </summary>
	/// <returns>Whether a brain was found in the cursor or the scene.</returns>
	bool UpdateBrainPath();

	/// <summary>
	/// Updates the path from the designated position to orbit, and its cost.
	/// </summary>
	/// <param name="brainPos">The designated position of the brain.</param>
	void UpdateBrainSkyPathAndCost(Vector brainPos);


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
    // Measures the interval between graphically revealing objects
    Timer m_RevealTimer;
    // The index which keeps track of the point in the build queue that blueprint objects go from being ghosted to revealed
    int m_RevealIndex;
	// Whether we need a clear path to orbit to place brain
	bool m_RequireClearPathToOrbit;

    // The pie menu
    PieMenuGUI *m_pPieMenu;
    PieSlice::PieSliceIndex m_ActivatedPieSliceType; //!< The activated PieSliceType, reset every frame.
    // The object picker
    ObjectPickerGUI *m_pPicker;
    // The ID of the DataModule that contains the native Tech of the Player using this menu
    int m_NativeTechModule;
    // The multiplier of costs of any foreign tech items
    float m_ForeignCostMult;
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
    // The cost of the path from the current position of the brain to the sky
    float m_BrainSkyPathCost;
    // Valid brain path line dots
    static BITMAP *s_pValidPathDot;
    // Invalid brain path line dots
    static BITMAP *s_pInvalidPathDot;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneEditorGUI, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	SceneEditorGUI(const SceneEditorGUI &reference) = delete;
	SceneEditorGUI & operator=(const SceneEditorGUI &rhs) = delete;

};

} // namespace RTE

#endif  // File