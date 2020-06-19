#ifndef _RTEACTIVITYMAN_
#define _RTEACTIVITYMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActivityMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Singleton.h"
#define g_ActivityMan ActivityMan::Instance()


#include "Controller.h"

namespace RTE
{

#define LZCURSORFRAMECOUNT 4
#define OBJARROWFRAMECOUNT 4

class Actor;
#include "Activity.h"


	};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the Activity:s and rules of Cortex Command
// Parent(s):       Singleton.
// Class history:   8/13/2004 ActivityMan created.

class ActivityMan:
    public Singleton<ActivityMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ActivityMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ActivityMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ActivityMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~ActivityMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActivityMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
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
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ActivityMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ActivityMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ActivityMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActivityMan object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDefaultActivityType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the type name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       The default activity type name.
// Return value:    None.

    void SetDefaultActivityType(std::string defaultActivityType) { m_DefaultActivityType = defaultActivityType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDefaultActivityName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the instance name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       The default activity instance name.
// Return value:    None.

    void SetDefaultActivityName(std::string defaultActivityName) { m_DefaultActivityName = defaultActivityName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDefaultActivityType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the type name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       None.
// Return value:    The default activity type name.

    std::string GetDefaultActivityType() const { return m_DefaultActivityType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDefaultActivityName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       None.
// Return value:    The default activity instance name.

    std::string GetDefaultActivityName() const { return m_DefaultActivityName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new activity to copy for next restart. You have to use
//                  RestartActivity to get it going. Ownership IS transferred!
// Arguments:       The new activity to put into effect next time ResetActivity is called.
// Return value:    None.

    void SetStartActivity(Activity *pNewActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the activity set to use a copy of for next restart. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    The activity to put into effect next time ResetActivity is called. OINT!

    Activity * GetStartActivity() { return m_pStartActivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current activity in effect. Won't be what has been set by
//                  SetStartActivity unless RestartActivity has been called since.
// Arguments:       None.
// Return value:    The current Activity in effect. Will be 0 if no activity is going.

    Activity * GetActivity() { return m_pActivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the activity passed in. Ownership IS transferred!
// Arguments:       The new activity to start. Ownership IS passed in!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int StartActivity(Activity *pActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially gets and starts the activity described.
// Arguments:       The class and instance names of the activity to start. Has to have
//                  been read in already (duh).
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int StartActivity(std::string className, std::string instanceName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Completely restarts whatever activity was last started.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int RestartActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PauseActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

    void PauseActivity(bool pause = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

    void EndActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityRunning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently running or not (not editing,
//                  over or paused)
// Arguments:       None.
// Return value:    Whether the game is running or not.

    bool ActivityRunning() const { if (m_pActivity) { return m_pActivity->Running(); } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityPaused
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently paused or not.
// Arguments:       None.
// Return value:    Whether the game is paused or not.

    bool ActivityPaused() const { if (m_pActivity) { return m_pActivity->Paused(); } return true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this and the current Activity. Supposed to be
//                  done every frame before drawing.
// Arguments:       None.
// Return value:    None.

    void Update();

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LateUpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only updates Global Scripts of the current activity with LateUpdate flag enabled
// Arguments:       None.
// Return value:    None.

	void LateUpdateGlobalScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;
    // The type and name of the default activity to be loaded if nothing else is available.
    std::string m_DefaultActivityType;
    std::string m_DefaultActivityName;
    // The starting condition of the next activity to be (re)started. Owned by this
    Activity *m_pStartActivity;
    // The current Activity in action, OWNED by this!
    Activity *m_pActivity;
    // Path to the last music stream being played
    std::string m_LastMusicPath;
    // What the last position of the in-game music track was before pause, in seconds
    double m_LastMusicPos;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ActivityMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ActivityMan(const ActivityMan &reference);
    ActivityMan & operator=(const ActivityMan &rhs);

};

} // namespace RTE

#endif // File