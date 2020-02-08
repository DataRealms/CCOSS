#ifndef _RTETIMER_
#define _RTETIMER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Timer.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Timer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"
#include "TimerMan.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A precise timer for FPS synch etc.
// Parent(s):       None.
// Class history:   03/05/2001 Timer created.
//                  03/07/2001 Implemented method mapping and dynamic use of either
//                             QueryPerformanceCounter() or GetTickCount() depending on
//                             if QPC() is supported by the current CPU or not.
//                  04/04/2005 Reworked to use central TimerMan.
//                  04/12/2007 Keeps track o both real time and simulation time.

class Timer//:
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the timer so that the elapsed time is 0 ms.
// Arguments:       None.
// Return value:    None.

    void Reset() { m_StartRealTime = g_TimerMan.GetRealTickCount(); m_StartSimTime = g_TimerMan.GetSimTickCount(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the timer so that the elapsed time is 0 ms.
// Arguments:       None.
// Return value:    None.

    void Clear();


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Timer object.
// Arguments:       None.

    Timer();


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Timer object with a set
//                  sim time elapsed.
// Arguments:       A unsigned long defining the amount of time (in ms) that this Timer should
//                  start with elapsed.

    Timer(unsigned long elapsedSimTime);


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Timer object
//                  identical to an already existing one.
// Arguments:       A Timer object which is passed in by reference.

    Timer(const Timer &reference);// { Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Timer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Timer object before deletion.
// Arguments:       None.

    virtual ~Timer();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Timer object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Timer to be identical to another, by deep copy.
// Arguments:       A reference to the Timer to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Timer &reference);
*/
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
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Timer to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Timer will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Timer.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Serializable.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartRealTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start real time value of this Timer.
// Arguments:       A __int64 with the new real time value (ms since the OS was started).
// Return value:    None.

    void SetStartRealTimeMS(const int64_t newStartTime) { m_StartRealTime = newStartTime * m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRealTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the real time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       A positive double with the new real time limit relative to the start time.
// Return value:    None.

    void SetRealTimeLimitMS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit *  m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRealTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the real time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       A positive double with the new real time limit relative to the start time.
// Return value:    None.

    void SetRealTimeLimitS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit * (double)g_TimerMan.GetTicksPerSecond(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRealTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the real time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       None.
// Return value:    A positive double with the real time limit relative to the start time.

    double GetRealTimeLimitMS() const { return m_RealTimeLimit / m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the real time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       None.
// Return value:    A positive double with the real time limit relative to the start time.

    double GetRealTimeLimitS() const { return m_RealTimeLimit / (double)g_TimerMan.GetTicksPerSecond(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetElapsedRealTimeS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start real time value of this Timer.
// Arguments:       A __int64 with the new elapsed time value in seconds.
// Return value:    None.

    void SetElapsedRealTimeS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * (double)g_TimerMan.GetTicksPerSecond()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetElapsedRealTimeS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start real time value of this Timer, in seconds.
// Arguments:       A __int64 with the new elapsed time value.
// Return value:    None.

    void SetElapsedRealTimeMS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * m_TicksPerMS); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStartRealTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the start real time value of this Timer.
// Arguments:       None.
// Return value:    A __int64 value that represents the amount of real time in ms from when
//                  windows was started to when Reset() of this Timer was called.

    int64_t GetStartRealTimeMS() const { return m_StartRealTime; }


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPastRealMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the elapsed real time is past a certain amount of time
//                  relative to this' start.
// Arguments:       A long specifying the threshold amount of real time in ms.
// Return value:    A bool only yielding true if the elapsed real time is greater than the
//                  passed in value.

    bool IsPastRealMS(long limit) { return GetElapsedRealTimeMS() > limit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPastRealTimeLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the elapsed real time is past a certain amount of time
//                  after the start previously set by SetRealTimeLimit.
// Arguments:       An unsigned long specifying the threshold amount of real time in ms.
// Return value:    A bool only yielding true if the elapsed real time is greater than the
//                  set limit value. If no limit has been set, this returns false.

    bool IsPastRealTimeLimit() { return m_RealTimeLimit == 0 ? true : (m_RealTimeLimit > 0 && (g_TimerMan.GetRealTickCount() - m_StartRealTime) > m_RealTimeLimit); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillRealTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit previously set by SetRealTimeLimitMS.
// Arguments:       None.
// Return value:    A unsigned long with the real time left till the passed in value, or
//                  negative if this Timer is already past that point in time.

    double LeftTillRealTimeLimitMS() { return (m_RealTimeLimit / m_TicksPerMS) - GetElapsedRealTimeMS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillRealTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit previously set by SetRealTimeLimitS.
// Arguments:       None.
// Return value:    A unsigned long with the real time left till the passed in value, or
//                  negative if this Timer is already past that point in time.

    double LeftTillRealTimeLimitS() { return (m_RealTimeLimit * (double)g_TimerMan.GetTicksPerSecond()) - GetElapsedRealTimeS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RealTimeLimitProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much progress has been made toward the set time limit
//                  previously set by SetRealTimeLimitMS. 0 means no progress, 1.0 means
//                  the timer has reached, or is beyond the limit.
// Arguments:       None.
// Return value:    A normalzied scalar between 0.0 - 1.0 showing the progress toward the limit.

    double RealTimeLimitProgress() const { return m_RealTimeLimit == 0 ? 1.0 : (MIN(1.0, GetElapsedRealTimeMS() / (m_RealTimeLimit / m_TicksPerMS)));  }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillRealMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit.
// Arguments:       A unsigned long specifying till when there is time.
// Return value:    A unsigned long with the time left till the passed in value, or negative if
//                  this Timer is already past that point in time.

    unsigned long LeftTillRealMS(int64_t when) { return when - GetElapsedRealTimeMS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AlternateReal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true or false, depending on whether the elapsed time falls in
//                  one of two repeating intervals which divide it. This is useful for blink
//                  animations etc.
// Arguments:       An int with the alternating period in ms. The time specified here is how
//                  long it will take for the switch to alternate.
// Return value:    Whether the elapsed time is in the first state or not.

    bool AlternateReal(int period) const { return ((int)GetElapsedRealTimeMS() % (period * 2)) > period; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start time value of this Timer, in ticks
// Arguments:       A __int64 with the new time value (ms since windows was started).
// Return value:    None.

    void SetStartSimTimeMS(const int64_t newStartTime) { m_StartSimTime = newStartTime * m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSimTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sim time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       A positive double with the new sim time limit relative to the start time.
// Return value:    None.

    void SetSimTimeLimitMS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSimTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sim time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       A positive double with the new sim time limit relative to the start time.
// Return value:    None.

    void SetSimTimeLimitS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * (double)g_TimerMan.GetTicksPerSecond(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sim time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       None.
// Return value:    A positive double with the sim time limit relative to the start time.

    double GetSimTimeLimitMS() const { return m_SimTimeLimit / m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sim time limit value of this Timer, RELATVE to the start time.
//                  This is when the timer is supposed to show that it has 'expired' or
//                  reached whatever time limit it is supposed to keep track of.
// Arguments:       None.
// Return value:    A positive double with the sim time limit relative to the start time.

    double GetSimTimeLimitS() const { return m_SimTimeLimit / (double)g_TimerMan.GetTicksPerSecond(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetElapsedSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start time value of this Timer, in ms.
// Arguments:       A double with the new elapsed time value.
// Return value:    None.

    void SetElapsedSimTimeMS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * m_TicksPerMS); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetElapsedSimTimeS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the start time value of this Timer, in seconds.
// Arguments:       A __int64 with the new elapsed time value in seconds.
// Return value:    None.

    void SetElapsedSimTimeS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * (double)g_TimerMan.GetTicksPerSecond()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStartSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the start time value of this Timer.
// Arguments:       None.
// Return value:    A __int64 value that represents the amount of time in ticks from when
//                  windows was started to when Reset() of this Timer was called.

    int64_t GetStartSimTimeMS() const { return m_StartSimTime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetElapsedSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the elapsed time in ms since this Timer was Reset().
// Arguments:       None.
// Return value:    A unsigned long value that represents the elapsed time since Reset() in ms.

    double GetElapsedSimTimeMS() const { return (double)(g_TimerMan.GetSimTickCount() - m_StartSimTime) / m_TicksPerMS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetElapsedSimTimeS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the elapsed time in s since this Timer was Reset().
// Arguments:       None.
// Return value:    A unsigned long value that represents the elapsed time since Reset() in s.

    double GetElapsedSimTimeS() const { return (double)(g_TimerMan.GetSimTickCount() - m_StartSimTime) / (double)g_TimerMan.GetTicksPerSecond(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WaitTill
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Halts the program/thread till the elapsed time becomes greater than
//                  the passed in amount of milliseconds.
// Arguments:       A unsigned long specifying how how many ms the slapsed time has to pass before
//                  continuing program/thread execution.
// Return value:    None.

//    void WaitTill(unsigned long limit) { while (limit > g_TimerMan.GetSimTickCount() - m_StartSimTime); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPastSimTimeLimit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the elapsed sim time is past a certain amount of time
//                  after the start previously set by SetSimTimeLimit.
// Arguments:       An unsigned long specifying the threshold amount of real time in ms.
// Return value:    A bool only yielding true if the elapsed real time is greater than the
//                  set limit value. If no limit has been set, this returns false.

    bool IsPastSimTimeLimit() const { return m_SimTimeLimit == 0 ? true : (m_SimTimeLimit > 0 && (g_TimerMan.GetSimTickCount() - m_StartSimTime) > m_SimTimeLimit); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPastSimMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if the elapsed time is past a certain amount of time.
// Arguments:       A unsigned long specifying the threshold amount of time in ms.
// Return value:    A bool only yielding true if the elapsed time is greater than the
//                  passed in value.

    bool IsPastSimMS(double limit) const { return GetElapsedSimTimeMS() > limit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillSimTimeLimitMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit previously set by SetSimTimeLimitMS.
// Arguments:       None.
// Return value:    A unsigned long with the sim time left till the passed in value, or
//                  negative if this Timer is already past that point in time.

    double LeftTillSimTimeLimitMS() const { return (m_SimTimeLimit / m_TicksPerMS) - GetElapsedSimTimeMS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillSimTimeLimitS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit previously set by SetSimTimeLimitS.
// Arguments:       None.
// Return value:    A unsigned long with the sim time left till the passed in value, or
//                  negative if this Timer is already past that point in time.

    double LeftTillSimTimeLimitS() const { return (m_SimTimeLimit * (double)g_TimerMan.GetTicksPerSecond()) - GetElapsedSimTimeS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SimTimeLimitProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much progress has been made toward the set time limit
//                  previously set by SetSimTimeLimitMS. 0 means no progress, 1.0 means
//                  the timer has reached, or is beyond the limit.
// Arguments:       None.
// Return value:    A normalzied scalar between 0.0 - 1.0 showing the progress toward the limit.

    double SimTimeLimitProgress() const { return m_SimTimeLimit == 0 ? 1.0 : (MIN(1.0, GetElapsedSimTimeMS() / (m_SimTimeLimit / m_TicksPerMS)));  }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LeftTillSimMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much time in ms that there is left till this Timer reaches
//                  a certain time limit.
// Arguments:       A unsigned long specifying till when there is time.
// Return value:    A unsigned long with the time left till the passed in value, or negative if
//                  this Timer is already past that point in time.

    double LeftTillSimMS(double when) const { return when - GetElapsedSimTimeMS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AlternateSim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true or false, depending on whether the elapsed time falls in
//                  one of two repeating intervals which divide it. This is useful for blink
//                  animations etc.
// Arguments:       An int with the alternating period in ms. The time specified here is how
//                  long it will take for the switch to alternate.
// Return value:    Whether the elapsed time is in the first state or not.

    bool AlternateSim(int period) const { if (period == 0) return true; else return ((int)GetElapsedSimTimeMS() % (period * 2)) > period; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string ClassName;

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

} // namespace RTE

#endif // File