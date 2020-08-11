#ifndef _RTEEDITORACTIVITY_
#define _RTEEDITORACTIVITY_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            EditorActivity.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the EditorActivity class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUISound.h"
#include "RTETools.h"
#include "ActivityMan.h"
#include "PostProcessMan.h"

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
class GUIComboBox;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           EditorActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activity for editing things; owns and manages all the dialog boxes
//                  etc for docuemnt model, ie new, load, save, etc.
// Parent(s):       Activity.
// Class history:   9/17/2007 EditorActivity created.

class EditorActivity:
    public Activity
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods
	ClassInfoGetters

    // Different modes of this editor
    enum EditorMode
    {
        NEWDIALOG = 0,
        LOADDIALOG,
        SAVEDIALOG,
        CHANGESDIALOG,
        OVERWRITEDIALOG,
        EDITINGOBJECT,
        TESTINGOBJECT,
        EDITORMODECOUNT
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     EditorActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a EditorActivity object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    EditorActivity() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~EditorActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a EditorActivity object before deletion
//                  from system memory.
// Arguments:       None.

	~EditorActivity() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the EditorActivity object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a EditorActivity to be identical to another, by deep copy.
// Arguments:       A reference to the EditorActivity to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const EditorActivity &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire EditorActivity, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); Activity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the EditorActivity object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

	void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current mode of this editor.
// Arguments:       The new mode to set to, see the EditorGUIMode enum.
// Return value:    None.

    void SetEditorMode(EditorMode newMode) { m_EditorMode = newMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mode of this editor.
// Arguments:       None.
// Return value:    The current mode this is set to; see the EditorGUIMode enum.

    EditorMode GetEditorMode() const { return m_EditorMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the game accroding to parameters previously set.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Start() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

	void SetPaused(bool pause = true) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

	void End() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
// Arguments:       A pointer to a screen-sized BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which screen's GUI to draw onto the bitmap.
// Return value:    None.

	void DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int which = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActivityMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.
// Arguments:       A pointer to a BITMAP to draw on. OWNERSHIP IS NOT TRANSFERRED!
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateNewDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateNewDialog() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateLoadDialog() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateSaveDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateSaveDialog() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChangesDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save Changes dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateChangesDialog() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateOverwriteDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Overwrite dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateOverwriteDialog() {}


    // Member variables
    static Entity::ClassInfo m_sClass;
    // The current mode of the whole editor. See EditorGUIMode enum.
    EditorMode m_EditorMode;
    // Indicates the previous mode before a confirm dialog box was shown
    EditorMode m_PreviousMode;
    // Whether the mode has been changed 
    bool m_ModeChange;
    // The DataModule space that we are editing within; can only place objects defined in the official modules plus this one
    int m_ModuleSpaceID;
    // Whether the edited object has been changed since last save
    bool m_NeedSave;
    // Whether this thing has ever been saved (or loaded). Will be true of new until it is saved
    bool m_HasEverBeenSaved;

    // GUI Screen for use by the GUI dialog boxes. Owned
    GUIScreen *m_pGUIScreen;
    // Input controller for he dialog box gui.  Owned
    GUIInput *m_pGUIInput;
    // The control manager which holds all the gui elements for the dialog boxes.  Owned
    GUIControlManager *m_pGUIController;

    // New Dialog box
    GUICollectionBox *m_pNewDialogBox;
    // The combobox which lists all the DataModules that the new Entity can belong to
    GUIComboBox *m_pNewModuleCombo;
    // The button for asking for new Entity
    GUIButton *m_pNewButton;
    // The button for canceling new Entity dialog
    GUIButton *m_pNewCancel;

    // Load Dialog box
    GUICollectionBox *m_pLoadDialogBox;
    // The combobox which lists all the Entities that can be loaded
    GUIComboBox *m_pLoadNameCombo;
    // The button for going to new dialog instead
    GUIButton *m_pLoadToNewButton;
    // The button for confirming Entity load
    GUIButton *m_pLoadButton;
    // The button for canceling load dialog
    GUIButton *m_pLoadCancel;

    // Save Dialog box
    GUICollectionBox *m_pSaveDialogBox;
    // Textbox for entering the name of the thing to save.
    GUITextBox *m_pSaveNameBox;
    // The label which shows which DataModule this Scene is set to be saved to
    GUILabel *m_pSaveModuleLabel;
    // The button for confirming save
    GUIButton *m_pSaveButton;
    // The button for canceling save dialog
    GUIButton *m_pSaveCancel;

    // Changes Dialog box
    GUICollectionBox *m_pChangesDialogBox;
    // The label for showing where it'll be saved
    GUILabel *m_pChangesNameLabel;
    // The button for confirming save changes
    GUIButton *m_pChangesYesButton;
    // The button for No resposnes to save changes
    GUIButton *m_pChangesNoButton;

    // Overwrite Dialog box
    GUICollectionBox *m_pOverwriteDialogBox;
    // The label for showing what is about to be overwritten
    GUILabel *m_pOverwriteNameLabel;
    // The button for confirming overwrite existing
    GUIButton *m_pOverwriteYesButton;
    // The button for No resposnes to overwrite
    GUIButton *m_pOverwriteNoButton;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Activity, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File