#ifndef _RTETIMER_
#define _RTETIMER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Timer.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wrapper timer class for GUI Editor
// Project:         Retro Terrain Engine
// Author(s):       Jason Boettcher


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <string>
#include "TimerMan.h"
#include "DDTError.h"



namespace RTE
{

	extern TimerMan	g_TimerMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A precise timer for FPS synch etc.
// Parent(s):       None.
// Class history:   29/03/2009 Timer Wrapper created.
class Timer {
public:
	Timer()		{ Reset(); }

	// Reset the timer
	void		Reset(void)	{ m_StartRealTime = g_TimerMan.GetRealTickCount(); }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetElapsedRealTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the elapsed real time in ms since this Timer was Reset().
// Arguments:       None.
// Return value:    A unsigned long value that represents the elapsed real time since Reset() in ms.

    double GetElapsedRealTimeMS() const { return (double)(g_TimerMan.GetRealTickCount() - m_StartRealTime) / m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetElapsedRealTimeS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the elapsed real time in seconds since this Timer was Reset().
// Arguments:       None.
// Return value:    A double value that represents the elapsed real time since Reset() in s.

    double GetElapsedRealTimeS() const { return (double)(g_TimerMan.GetRealTickCount() - m_StartRealTime) / (double)g_TimerMan.GetTicksPerSecond(); }


	double	GetElapsedSimTimeMS() const { return GetElapsedRealTimeMS(); }


protected:



private:

	// Absolute tick count when this was started in real time
    int64_t m_StartRealTime;
    // Absolute tick count when this was started in sim time
    int64_t m_StartSimTime;
    // Tick count, relative to the start time, when this should indicate end or expired in real time
    int64_t m_RealTimeLimit;
    // Tick count, relative to the start time, when this should indicate end or expired in sim time
    int64_t m_SimTimeLimit;
    // Ticks per MS
    double m_TicksPerMS;

};


}



#endif  _RTETIMER_