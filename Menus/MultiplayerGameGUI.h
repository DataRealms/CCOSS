#ifndef _MULTIPLAYERGAMEGUI_
#define _MULTIPLAYERGAMEGUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MultiplayerGameGUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Project:         GUI Library
// Author(s):       


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

//#include "FrameMan.h"
#include "Timer.h"
#include "Vector.h"
#include "Controller.h"

struct BITMAP;


namespace RTE
{
	class SceneObject;
	class ObjectPickerGUI;
	class PieMenuGUI;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Class:           MultiplayerGameGUI
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     
	// Parent(s):       None.
	// Class history:   

	class MultiplayerGameGUI {

		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:
		// Different modes of this editor
		enum GUIMode
		{
			INACTIVE = 0,
			ACTIVE = 1
		};


		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     MultiplayerGameGUI
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a MultiplayerGameGUI object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		MultiplayerGameGUI() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~MultiplayerGameGUI
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a MultiplayerGameGUI object before deletion
		//                  from system memory.
		// Arguments:       None.

		~MultiplayerGameGUI() { Destroy(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the MultiplayerGameGUI object ready for use.
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

		int Create(Controller *pController);


		//////////////////////////////////////////////////////////////////////////////////////////
		//  Method:  Reset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Resets the entire MultiplayerGameGUI, including its inherited members, to
		//                  their default settings or values.
		// Arguments:       None.
		// Return value:    None.

		 void Reset() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the MultiplayerGameGUI object.
		// Arguments:       None.
		// Return value:    None.

		void Destroy();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Update
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the state of this Menu each frame
		// Arguments:       None.
		// Return value:    None.

		 void Update();

		//////////////////////////////////////////////////////////////////////////////////////////
		//  Method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws the editor
		// Arguments:       The bitmap to draw on.
		//                  The absolute position of the target bitmap's upper left corner in the scene.
		// Return value:    None.

		 void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) const;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Protected member variable and method declarations

	protected:

		// Controller which conrols this menu. Not owned
		Controller *m_pController;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Private member variable and method declarations

	private:

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the member variables of this MultiplayerGameGUI, effectively
		//                  resetting the members of this abstraction level only.
		// Arguments:       None.
		// Return value:    None.

		void Clear();


		// Disallow the use of some implicit methods.
		MultiplayerGameGUI(const MultiplayerGameGUI &reference);
		MultiplayerGameGUI & operator=(const MultiplayerGameGUI &rhs);

	};

} // namespace RTE

#endif  // File
