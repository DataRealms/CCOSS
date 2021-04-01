#ifndef _ALLEGROINPUT_
#define _ALLEGROINPUT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            AllegroInput.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     AllegroInput Class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

#include "GUIInput.h"


enum aMButtons {
    AMBLEFT = 1, // 0b1
    AMBRIGHT = 2, // 0b10
    AMBMIDDLE = 4 // 0b100
};

namespace RTE
{

class Timer;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           AllegroInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An input interface using the CDX Library for input
// Parent(s):       GUIInput.
// Class history:   12/30/2003 AllegroInput Created.
//                  11/09/2005 Ported to use Allegro and renamed to AllegroInput.

class AllegroInput :
    public GUIInput
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AllegroInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AllegroInput object in system
//                  memory.
// Arguments:       Whether the keyboard and joysticks also can control the mouse crsr.

    AllegroInput(int whichPlayer, bool keyJoyMouseCursor = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~AllegroInput
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a AllegroInput object.
// Arguments:       None.

    ~AllegroInput() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:         Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the Input.
// Arguments:       None.

    void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConvertKeyEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts from allegro's key push to that used by this GUI lib, with
//                  timings for repats taken into consideration.
// Arguments:       The allegro scancode, the corrsponding guilib scancode, and the elapsed
//                  time in S since last update.

//    int ConvertKeyEvent(bool keyDown, int oldKeyEvent) { return keyDown ? (oldKeyEvent == Pushed ? Repeat : Pushed) : ((oldKeyEvent == Pushed || oldKeyEvent == Repeat) ? Released : None); }
    void ConvertKeyEvent(int allegroKey, int guilibKey, float elapsedS);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:         CreateTimer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates the timer.
// Arguments:       None.

    bool CreateTimer();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:         GetCurrentTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current time since the timer was created.
// Arguments:       None.

    float GetTime();

    // Members
    float       m_KeyDelay;
    float       m_KeyRepeat;

    int         m_ModStates[6];
    float       m_KeyboardTimes[256];

    // Timers
    Timer       *m_pTimer;
    Timer       *m_pCursorAccelTimer;
};


}; // namespace RTE


#endif // File
 