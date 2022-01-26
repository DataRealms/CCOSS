#ifndef _RTEGATUTORIAL_
#define _RTEGATUTORIAL_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GATutorial.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GameActivity.h"
#include "Box.h"

namespace RTE
{

class Actor;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GATutorial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tutorial mission with lots of special triggering logic.
// Parent(s):       GameActivity.
// Class history:   10/13/2007 GATutorial created.

class GATutorial : public GameActivity {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(GATutorial);
SerializableOverrideMethods;
ClassInfoGetters;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GATutorial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GATutorial object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GATutorial() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GATutorial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GATutorial object before deletion
//                  from system memory.
// Arguments:       None.

	~GATutorial() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GATutorial object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GATutorial to be identical to another, by deep copy.
// Arguments:       A reference to the GATutorial to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const GATutorial &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GATutorial, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); Activity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GATutorial object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

	void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SceneIsCompatible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if a particular Scene supports this specific Activity on it.
//                  Usually that means certain Area:s need to be defined in the Scene.
// Arguments:       The Scene to check if it supports this Activiy. Ownership IS NOT TRANSFERRED!
//                  How many teams we're checking for. Some scenes may support and activity
//                  but only for a limited number of teams. If -1, not applicable.
// Return value:    Whether the Scene has the right stuff.

	bool SceneIsCompatible(Scene *pScene, int teams = -1) override;


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
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitAIs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through all Actor:s currently in the MovableMan and sets each
//                  one not controlled by a player to be AI controlled and AIMode setting
//                  based on team and CPU team.
// Arguments:       None.
// Return value:    None.

	void InitAIs() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetupAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up or resets the Tutorial Areas to show the current control
//                  mappings etc.
// Arguments:       None.
// Return value:    None.

	void SetupAreas();


    enum TutorialArea
    {
        BRAINCHAMBER = 0,
        BODYSTORAGE,
        SHAFT,
        OBSTACLECOURSE,
        FIRINGRANGE,
        ROOFTOP,
		ROOFEAST,
        AREACOUNT
    };

    enum TutorialRoom
    {
        ROOM0 = 0,
        ROOM1,
        ROOM2,
        ROOM3,
        ROOMCOUNT
    };

    enum LitState
    {
        UNLIT = 0,
        LIT,
        LITSTATECOUNT
    };

    enum ScreenState
    {
        SCREENOFF = 0,
        STATICLITTLE,
        STATICLARGE,
        SHOWINGSTEP,
        SREENSTATECOUNT
    };

    enum FightStage
    {
        NOFIGHT = 0,
        DEFENDING,
        ATTACK,
        FIGHTSTAGECOUNT
    };

    struct TutStep
    {
        // Text of this step
        std::string m_Text;
        // Duration of the whole step
        int m_Duration;
        // BITMAPs not owned here
        std::vector<BITMAP *> m_pScreens;
        // The duration of one frame
        int m_FrameDuration;

        TutStep(std::string text, int stepDuration, string screensPath = "", int frameCount = 1, int frameDuration = 250);
    };

    // Member variables
    static Entity::ClassInfo m_sClass;

    // The player which is actually playing the tut
    int m_TutorialPlayer;
    // The areas that trigger specific sets of steps to be shown
    Box m_TriggerBoxes[AREACOUNT];
    // Positions of the screens for each area
    Vector m_ScreenPositions[AREACOUNT];
    // The current state of the all the different areas' screens
    ScreenState m_ScreenStates[AREACOUNT];
    // Offsets of the center of the text line from the screen position
    Vector m_TextOffsets[AREACOUNT];
    // Screen bitmaps common to all areas.. off, static etc
    BITMAP *m_apCommonScreens[STATICLARGE + 1];
    // The steps themselves; cycles through for each area
    std::vector<TutStep> m_TutAreaSteps[AREACOUNT];
    // Positions of the numbered room signs
    Vector m_RoomSignPositions[ROOMCOUNT];
    // Room sign bitmaps, unlit and lit
    BITMAP *m_aapRoomSigns[ROOMCOUNT][LITSTATECOUNT];
    // The timer which keeps track of how long each area has been showing
    Timer m_AreaTimer;
    // The timer which keeps track of how long each step should be shown
    Timer m_StepTimer;
    // Which are the player-controlled actor is within
    TutorialArea m_CurrentArea;
    // Which are the player-controlled actor was in last
    TutorialArea m_PrevArea;
    // If teh screen has just changed and needs to be redrawn
    bool m_ScreenChange;
    // Which tutorial step of the current area currently being played back
    int m_CurrentStep;
    // Which frame of the current step's animation are we on?
    int m_CurrentFrame;
    // Current room
    TutorialRoom m_CurrentRoom;
    // Trigger box for the subsequent fight
    Box m_FightTriggers[FIGHTSTAGECOUNT];
    // The current fight stage
    FightStage m_CurrentFightStage;
    // The CPU opponent brain; not owned!
    Actor *m_pCPUBrain;


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