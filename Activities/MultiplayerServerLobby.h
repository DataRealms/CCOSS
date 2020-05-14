#ifndef _RTEMULTIPLAYERSERVERLOBBY_
#define _RTEMULTIPLAYERSERVERLOBBY_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MultiplayerServerLobby.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Project:         Retro Terrain Engine
// Author(s):       


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "ActivityMan.h"
#include "GameActivity.h"

#include "NetworkServer.h"

namespace RTE
{
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class Entity;
	class Scene;
	class Activity;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Class:           MultiplayerServerLobby
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Activity for editing scenes.
	// Parent(s):       EditorActivity.
	// Class history:   8/30/2007 MultiplayerServerLobby created, inheriting directly from Activity.
	//                  9/17/2007 Spliced out and made to derive from EditorActivty

	class MultiplayerServerLobby :
	public GameActivity
	{

		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:

		// Concrete allocation and cloning definitions
		EntityAllocation(MultiplayerServerLobby)

		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     MultiplayerServerLobby
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a MultiplayerServerLobby object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		MultiplayerServerLobby() { Clear(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~MultiplayerServerLobby
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a MultiplayerServerLobby object before deletion
		//                  from system memory.
		// Arguments:       None.

		virtual ~MultiplayerServerLobby() { Destroy(true); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the MultiplayerServerLobby object ready for use.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates a MultiplayerServerLobby to be identical to another, by deep copy.
		// Arguments:       A reference to the MultiplayerServerLobby to deep copy.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Create(const MultiplayerServerLobby &reference);


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
		// Description:     Resets the entire MultiplayerServerLobby, including its inherited members, to their
		//                  default settings or values.
		// Arguments:       None.
		// Return value:    None.

		virtual void Reset() { Clear(); Activity::Reset(); }


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Save
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Saves the complete state of this MultiplayerServerLobby to an output stream for
		//                  later recreation with Create(Reader &reader);
		// Arguments:       A Writer that the MultiplayerServerLobby will save itself with.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		virtual int Save(Writer &writer) const;


		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the MultiplayerServerLobby object.
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

		void UpdateInput();

		void UpdateActivityBox();

		//void UpdateScenesBox();

		void UpdatePlayersBox(bool newActivity);

		void UpdateGoldSlider(const GameActivity * pSelectedGA);

		void UpdateDifficultySlider();

		void UpdateSkillSlider();

		void GetAllScenesAndActivities();

		bool StartGame();

		int PlayerCount();


		//////////////////////////////////////////////////////////////////////////////////////////
		// Protected member variable and method declarations

	protected:

		// These add on the player and team max counts
		enum PlayerColumns
		{
			PLAYER_CPU = Activity::MAXPLAYERCOUNT,
			PLAYERCOLUMNCOUNT
		};

		enum TeamRows
		{
			TEAM_DISABLED = Activity::MAXTEAMCOUNT,
			TEAMROWCOUNT
		};


		// Member variables
		static Entity::ClassInfo m_sClass;

		// The editor GUI
		//MultiplayerServerLobbyGUI *m_pEditorGUI;

		// GUI Screen for use by the GUI dialog boxes. Owned
		GUIScreen *m_pGUIScreen;
		// Input controller for he dialog box gui.  Owned
		GUIInput *m_pGUIInput;
		// The control manager which holds all the gui elements for the dialog boxes.  Owned
		GUIControlManager *m_pGUIController;

		GUICollectionBox *m_pRootBox;
		GUICollectionBox *m_pPlayerSetupBox;

		// Activity selection screen controls
		GUIComboBox *m_pActivitySelect;
		GUIComboBox *m_pSceneSelect;
		GUILabel *m_pDifficultyLabel;
		GUISlider *m_pDifficultySlider;

		GUICollectionBox *m_aapPlayerBoxes[PLAYERCOLUMNCOUNT][TEAMROWCOUNT];
		GUICollectionBox *m_apTeamBoxes[TEAMROWCOUNT];
		GUILabel *m_apTeamNameLabels[TEAMROWCOUNT];
		GUILabel *m_pStartErrorLabel;
		GUILabel *m_pCPULockLabel;
		// Which team the CPU is locked to, if any
		int m_LockedCPUTeam;

		//Tech selection combos
		GUIComboBox *m_apTeamTechSelect[Activity::MAXTEAMCOUNT];

		// AI skill selection
		GUISlider *m_apTeamAISkillSlider[Activity::MAXTEAMCOUNT];
		GUILabel *m_apTeamAISkillLabel[Activity::MAXTEAMCOUNT];

		GUILabel *m_pGoldLabel;
		GUISlider *m_pGoldSlider;
		GUICheckbox *m_pFogOfWarCheckbox;
		GUICheckbox *m_pRequireClearPathToOrbitCheckbox;
		GUICheckbox *m_pDeployUnitsCheckbox;

		const Icon *m_apPlayerIcons[c_MaxClients];

		GUILabel * m_apPlayerNameLabel[c_MaxClients];

		BITMAP* m_pUIDrawBitmap;

		BITMAP * m_pCursor;

		BITMAP *m_pScenePreviewBitmap;
		BITMAP *m_pDefaultPreviewBitmap;

		GUIButton *m_pStartScenarioButton;

		// The scene preset currently selected, NOT OWNED
		const Scene *m_pSelectedScene;

		// The current set of Scenes being displayed - not owned, nor are the scenes
		std::list<Scene *> *m_pScenes;
		// The map of Activity:ies, and the Scene:s compatible with each, neither of which are owned here
		std::map<Activity *, std::list<Scene *> > m_Activities;

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
