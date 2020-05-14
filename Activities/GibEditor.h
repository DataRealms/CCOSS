#ifndef _RTEGIBEDITOR_
#define _RTEGIBEDITOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GibEditor.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the GibEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "EditorActivity.h"

namespace RTE
{

class MOSRotating;
class GibEditorGUI;
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
// Class:           GibEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activity for editing gib placement within MOSRotating:s.
// Parent(s):       EditorActivity.
// Class history:   9/17/2007 GibEditor Created.

class GibEditor:
    public EditorActivity
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EnitityAllocation(GibEditor)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GibEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GibEditor object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GibEditor() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GibEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GibEditor object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~GibEditor() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GibEditor object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GibEditor to be identical to another, by deep copy.
// Arguments:       A reference to the GibEditor to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const GibEditor &reference);


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
// Description:     Resets the entire GibEditor, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); EditorActivity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GibEditor to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the GibEditor will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GibEditor object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current mode of this editor.
// Arguments:       The new mode to set to, see the EditorGUIMode enum.
// Return value:    None.

    void SetEditorMode(EditorActivity::EditorMode newMode) { m_EditorMode = newMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEditorMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current mode of this editor.
// Arguments:       None.
// Return value:    The current mode this is set to; see the EditorGUIMode enum.

    EditorActivity::EditorMode GetEditorMode() const { return m_EditorMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the game accroding to parameters previously set.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Start();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

    virtual void Pause(bool pause = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

    virtual void End();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
// Arguments:       A pointer to a screen-sized BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which screen's GUI to draw onto the bitmap.
// Return value:    None.

    virtual void DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int which = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActivityMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.
// Arguments:       A pointer to a BITMAP to draw on. OINT.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the current object to an appropriate ini file, and asks user if
//                  they want to overwrite first if object of this name exists.
// Arguments:       The name of the new object to be saved.
//                  Whetehr to force any existing Object of that name to be overwritten if
//                  it already exists.
// Return value:    Whether actually managed to save. Will return false both if an object
//                  of this name already exists (and not overwriting), or if other error.

    bool SaveObject(std::string saveAsName, bool forceOverwrite = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StuffEditedGibs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces all the gibs owned by the passed in MOSR with the ones being
//                  edited that represent the new gibbing setup.
// Arguments:       The MOSRotating to replace the gibs of. Ownership is NOT trnasferred!
// Return value:    None.

    void StuffEditedGibs(MOSRotating *pEditedObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateNewDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the New dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateNewDialog();


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateLoadDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Load dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateLoadDialog();


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateSaveDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateSaveDialog();


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateChangesDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Save Changes dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateChangesDialog();


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  UpdateOverwriteDialog
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Overwrite dialog box, populates its lists etc.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateOverwriteDialog();


    // Member variables
    static Entity::ClassInfo m_sClass;

    // The loaded MOSR of which we are editing the gibs. Owned by this.
    MOSRotating *m_pEditedObject;
    // The temporary copy of the edited object which will be gibbed in the test mode. Owned by this.
    MOSRotating *m_pTestingObject;
    // How many times tests ahve been run; then the terrain is cleared
    int m_TestCounter;
    // The MOSR which the user has picked to load. Not owned
    const MOSRotating *m_pObjectToLoad;
    // The editor GUI
    GibEditorGUI *m_pEditorGUI;


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