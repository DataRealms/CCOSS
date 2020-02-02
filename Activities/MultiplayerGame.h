#ifndef _RTEMULTIPLAYERGAME_
#define _RTEMULTIPLAYERGAME_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MultiplayerGame.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Project:         Retro Terrain Engine
// Author(s):       


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DDTTools.h"
#include "ActivityMan.h"

namespace RTE
{

	class MultiplayerGameGUI;
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
	// Class:           MultiplayerGame
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Activity for editing scenes.
	// Parent(s):       EditorActivity.
	// Class history:   8/30/2007 MultiplayerGame created, inheriting directly from Activity.
	//                  9/17/2007 Spliced out and made to derive from EditorActivty

	class MultiplayerGame :
	public Activity
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:

		// Concrete allocation and cloning definitions
		ENTITYALLOCATION(MultiplayerGame)


		enum MultiplayerGameMode
		{
			SETUP,
			CONNECTION,
			GAMEPLAY
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     MultiplayerGame
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a MultiplayerGame object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		MultiplayerGame() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~MultiplayerGame
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a MultiplayerGame object before deletion
		//                  from system memory.
		// Arguments:       None.

		virtual ~MultiplayerGame() { Destroy(true); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the MultiplayerGame object ready for use.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates a MultiplayerGame to be identical to another, by deep copy.
		// Arguments:       A reference to the MultiplayerGame to deep copy.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create(const MultiplayerGame &reference);


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
		// Description:     Resets the entire MultiplayerGame, including its inherited members, to their
		//                  default settings or values.
		// Arguments:       None.
		// Return value:    None.

		virtual void Reset() { Clear(); Activity::Reset(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Save
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Saves the complete state of this MultiplayerGame to an output stream for
		//                  later recreation with Create(Reader &reader);
		// Arguments:       A Writer that the MultiplayerGame will save itself with.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Save(Writer &writer) const;


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the MultiplayerGame object.
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
		// Member variables
		static Entity::ClassInfo m_sClass;

		// The editor GUI
		//MultiplayerGameGUI *m_pEditorGUI;

		// 
		GUITextBox *m_pServerNameTextBox;

		GUITextBox *m_pPlayerNameTextBox;

		GUIButton *m_pConnectButton;



		GUITextBox *m_pNATServiceServerNameTextBox;

		GUITextBox *m_pNATServerNameTextBox;

		GUITextBox *m_pNATServerPasswordTextBox;

		GUIButton *m_pConnectNATButton;



		GUILabel *m_pStatusLabel;

		// GUI Screen for use by the GUI dialog boxes. Owned
		GUIScreen *m_pGUIScreen;
		// Input controller for he dialog box gui.  Owned
		GUIInput *m_pGUIInput;
		// The control manager which holds all the gui elements for the dialog boxes.  Owned
		GUIControlManager *m_pGUIController;
		// Current state of the activity
		MultiplayerGameMode m_Mode;

		// Timer to watchdog connection states and abort connection if nothing happened
		Timer m_ConnectionWaitTimer;

		// Last music file being player, used to recover playback state after pause
		std::string m_LastMusic;

		// Position of music being played, used to recover playback state after pause
		double m_LastMusicPos;

		// Sound for enabling and disabling menu
		Sound m_EnterMenuSound;
		Sound m_ExitMenuSound;
		// Sound for changing focus
		Sound m_FocusChangeSound;
		// Sound for selecting items in list, etc.
		Sound m_SelectionChangeSound;
		// Sound for button press
		Sound m_ButtonPressSound;
		// Sound for button press of going back button
		Sound m_BackButtonPressSound;
		// Sound for adding or deleting items in list.
		Sound m_ItemChangeSound;
		// Sound for testing sound volume
		Sound m_TestSound;
		// Sound for erroneus input
		Sound m_UserErrorSound;


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
