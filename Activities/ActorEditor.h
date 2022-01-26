#ifndef _RTEACTOREDITOR_
#define _RTEACTOREDITOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActorEditor.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActorEditor class.
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

class Actor;
class ObjectPickerGUI;
class PieMenuGUI;
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
// Class:           ActorEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activity for testing and quickly iterating on Actor data definitions.
// Parent(s):       EditorActivity.
// Class history:   10/08/2007 ActorEditor Created.

class ActorEditor : public EditorActivity {

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(ActorEditor);
SerializableOverrideMethods;
ClassInfoGetters;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ActorEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ActorEditor object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ActorEditor() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ActorEditor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ActorEditor object before deletion
//                  from system memory.
// Arguments:       None.

	~ActorEditor() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActorEditor object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ActorEditor to be identical to another, by deep copy.
// Arguments:       A reference to the ActorEditor to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const ActorEditor &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ActorEditor, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); EditorActivity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActorEditor object.
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
// Method:          LoadActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the Actor itself and sets up the pie menu to match its setup.
// Arguments:       An Entity Preset of the Actor to load into the editor. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    Whether the Actor was loaded successfully from the PresetMan.

    bool LoadActor(const Entity *pActorToLoad);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadActorData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the ini with the currently edited Actor's definitions.
// Arguments:       None.
// Return value:    Whether the data file was successfully read.

    bool ReloadActorData();


    // Member variables
    static Entity::ClassInfo m_sClass;

    // The loaded MOSR of which we are editing. Owned by this.
    Actor *m_pEditedActor;
    // The file to reload to get any alterations to the current actor.
    std::string m_ActorDataFilePath;
    // The picker for selecting which object to load
    ObjectPickerGUI *m_pPicker;


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