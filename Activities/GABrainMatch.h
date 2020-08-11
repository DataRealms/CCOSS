#ifndef _RTEGABRAINMATCH_
#define _RTEGABRAINMATCH_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GABrainMatch.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GameActivity.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GABrainMatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Base class for all GABrainMatch:s, including game modes and editors.
// Parent(s):       GameActivity.
// Class history:   8/11/2007 GABrainMatch created.

class GABrainMatch:
    public GameActivity
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(GABrainMatch)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GABrainMatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GABrainMatch object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GABrainMatch() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GABrainMatch
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GABrainMatch object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~GABrainMatch() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GABrainMatch object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GABrainMatch to be identical to another, by deep copy.
// Arguments:       A reference to the GABrainMatch to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const GABrainMatch &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GABrainMatch, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Activity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GABrainMatch object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the game accroding to parameters previously set.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Start();


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
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    // Brain unit of the CPU team; not owned!
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