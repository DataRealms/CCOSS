#ifndef _RTETIMERMAN_
#define _RTETIMERMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            TimerMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the TimerMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files


#include <string>
#include <deque>



#if defined(_MSC_VER)
#define RECT WINDOWS_RECT // grr typedef repeat fix
#include "allegro.h"
#include "winalleg.h"
#undef RECT			
#undef GetClassName // grr MACRO NAMESPACE clash
#undef PlaySound	// and again == windows is a parasite
#endif // defined(WIN32)


namespace RTE
{

class QPCTimer;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           TimerMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The centralized singleton manager of all Timer:s and overall
//                  timekeeping in RTE.
// Parent(s):       Singleton
// Class history:   07/07/2004  TimerMan created.
//                  04/10/2007  Changed to use platform specific, sub-ms resolution timers
//                              instead of Allegro's crappy ms-only timer. Also implemented
//                              the model described here: http://www.gaffer.org/game-physics/fix-your-timestep

class TimerMan
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     TimerMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a TimerMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    TimerMan() { Clear(); Create(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~TimerMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a TimerMan object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~TimerMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TimerMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire TimerMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TimerMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the measured real and sim times to 0.
// Arguments:       None.
// Return value:    None.

    void ResetTime();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTicksPerSecond
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of ticks per second, or the resolution.
// Arguments:       None.
// Return value:    The number of ticks per second.

    int64_t GetTicksPerSecond() const { return m_TicksPerSecond; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRealTickCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a current global real time measured in ticks from the start of the
//                  simulation up to the last Update of this TimerMan. Use TickFrequency
//                  to determine how many ticks go in a second.
// Arguments:       None.
// Return value:    The number of ticks passed since the simulation started.

    int64_t GetRealTickCount() const { return m_RealTimeTicks; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTickCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a current global simulation time measured in ticks from the start
//                  of the simulation up to the last Update of this TimerMan. Use TickFrequency
//                  to determine how many ticks go in a second.
// Arguments:       None.
// Return value:    The number of ticks passed since the simulation started.

    int64_t GetSimTickCount() const { return m_SimTimeTicks; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a current global simulation time measured in ms ticks from the start
//                  of the simulation up to the last UpdateSim of this TimerMan.
// Arguments:       None.
// Return value:    The number of ms passed since the simulation started.

    int64_t GetSimTimeMS() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTimeScale
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a time scale factor which will be used to speed up or slow down
//                  the progress of the simulation time in relation to the real world
//                  time.
// Arguments:       None.
// Return value:    A factor between the real world time, and the simulation time. A value
//                  of 2.0 means simulation runs twice as fast as normal.

    float GetTimeScale() const { return m_TimeScale; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTimeScale
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a time scale factor which will be used to speed up or slow down
//                  the progress of the simulation time in relation to the real world
//                  time.
// Arguments:       A factor between the real world time, and the simulation time. A value
//                  of 2.0 means simulation runs twice as fast as normal.
// Return value:    None.

    void SetTimeScale(float timeScale = 1.0) { m_TimeScale = timeScale; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableAveraging
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables the averaging of time measurements done each Update().
//                  These help smooth out and prevent choppy animation.
// Arguments:       Whether ot not to enable the averaging.
// Return value:    None.

    void EnableAveraging(bool enable = true) { m_AveragingEnabled = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRealToSimCap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the cap of the amount of seconds which can be transferred from
//                  the real time to the simulated time in one update.
// Arguments:       A float specifying the new cap in seconds.
// Return value:    None.

    void SetRealToSimCap(float newCap) { m_RealToSimCap = newCap * m_TicksPerSecond; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRealToSimCap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the cap of the amount of seconds which can be transferred from
//                  the real time to the simulated time in one update.
// Arguments:       None.
// Return value:    A float describing the current cap in seconds.

    float GetRealToSimCap() const { return (float)m_RealToSimCap / (float)m_TicksPerSecond; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDeltaTimeTicks
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the number of ticks that a simulation update delta time should 
//                  take.
// Arguments:       The new delta time in ticks.
// Return value:    None.

    void SetDeltaTimeTicks(int newDelta) { m_DeltaTime = newDelta; m_DeltaTimeS = m_DeltaTime / m_TicksPerSecond; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeltaTimeTicks
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current number of ticks that the of the simulation updates in.
// Arguments:       None.
// Return value:    The current fixed delta time that the simulation should be updating with,
//                  in ticks.

    int GetDeltaTimeTicks() const { return m_DeltaTime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDeltaTimeSecs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the number of secibds that a simulation update delta time should 
//                  take.
// Arguments:       The new delta time in seconds.
// Return value:    None.

    void SetDeltaTimeSecs(float newDelta) { m_DeltaTimeS = newDelta; m_DeltaTime = m_DeltaTimeS * m_TicksPerSecond; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeltaTimeSecs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current fixed delta time of the simulation updates, in seconds.
// Arguments:       None.
// Return value:    The current fixed delta time that the simulation should be updating with,
//                  in seconds.

    float GetDeltaTimeSecs() const { return m_DeltaTimeS; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeltaTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current fixed delta time of the simulation updates, in ms.
// Arguments:       None.
// Return value:    The current fixed delta time that the simulation should be updating with,
//                  in ms.

    float GetDeltaTimeMS() const { return m_DeltaTimeS * 1000; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PauseSim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sim to be paused, ie no real time ticks will be transferred
//                  to the sim accumulator while this is set to true;
// Arguments:       Whether the sim should be paused or not.
// Return value:    None.

    void PauseSim(bool pause = false) { m_SimPaused = pause; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOneSimUpdatePerFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether to force this to artifically make time for only one single
//                  sim update for the graphics frame. Useful for debugging or profiling.
// Arguments:       Whether the sim should be set to only update once per graphics frame or not.
// Return value:    None.

    void SetOneSimUpdatePerFrame(bool oneUpdate = true) { m_OneSimUpdatePerFrame = oneUpdate; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOneSimUpdatePerFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether to force this to artifically make time for only one single
//                  sim update for the graphics frame. Useful for debugging or profiling.
// Arguments:       None.
// Return value:    Whether the sim should be set to only update once per graphics frame or not.

    bool IsOneSimUpdatePerFrame() const { return m_OneSimUpdatePerFrame; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSimSpeedLimited
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether to limit the sim speed to not exceed real time, ie 1.0x.
// Arguments:       Whether the sim speed should be limited to not exceed 1.0.
// Return value:    None.

    void SetSimSpeedLimited(bool simLimited = true) { m_SimSpeedLimited = simLimited; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsSimSpeedLimited
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the sim speed is limited to not exceed 1.0x.
// Arguments:       None.
// Return value:    Whether the sim is limited to not exceed 1.0x of real time.

    bool IsSimSpeedLimited() const { return m_OneSimUpdatePerFrame && m_SimSpeedLimited; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TimeForSimUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether there is enough sim time accumulated to do at least one
//                  physics update.
// Arguments:       None.
// Return value:    Whetehr there is enough sim time to do a physics update.

    float TimeForSimUpdate() { return m_SimAccumulator >= m_DeltaTime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawnSimUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the current simulation update will be drawn in a frame.
//                  Use this to check if it is necessary to draw purely graphical things
//                  during the sim update.
// Arguments:       None.
// Return value:    Whether this is the last sim update before a frame with its results
//                  will appear.

    bool DrawnSimUpdate() const { return m_DrawnSimUpdate; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the real time ticks based on the actual time clock time, and
//                  adding it to the accumulator which the sim ticks will draw from in
//                  whole DeltaTime-sized chunks.
// Arguments:       None.
// Return value:    None.

    void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the simulation time to represent the current amount of sim
//                  time passed from the start of the simulation up to the this last
//                  update.
// Arguments:       None.
// Return value:    None.

    void UpdateSim();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

    // The point in real time when the simulation (re)started
    int64_t m_StartTime;
    // The frequency of ticks each second, ie the resolution of the timer
    int64_t m_TicksPerSecond;
    // The number of actual time ticks counted so far
    int64_t m_RealTimeTicks;
    // The cap of number of ticks that the real time can add to the accumulator each update
    int64_t m_RealToSimCap;
    // Simulation time accumulator keeps track of how much actual time has passed and is chunked into whole DeltaTime:s upon UpdateSim
    int64_t m_SimAccumulator;
    // The fixed delta time chunk of the simulation update
    int64_t m_DeltaTime;
    // The simulation update step size, in seconds
    float m_DeltaTimeS;
    // The number of simulation time ticks counted so far
    int64_t m_SimTimeTicks;
    // The number of whole simulation updates have been made since reset
    int64_t m_SimUpdateCount;
    // Tells whether the current simulation update will be drawn in a frame.
    bool m_DrawnSimUpdate;
    // Time scale. The relationship between the real world actual time, and the simulation time.
    // A value of 2.0 means simulation runs twice as fast as normal, as percieved by a player.
    float m_TimeScale;
    // Whether calculated delta time averaging is enabled.
    bool m_AveragingEnabled;
    // The buffer of measured the most recent real time differences, used for averaging out the readings
    std::deque<float> m_DeltaBuffer;
    // Simulation paused; no real time ticks will go to the sim accumulator
    bool m_SimPaused;
    // Whether to force this to artifically make time for only one single sim update for the graphics frame. Useful for debugging or profiling.
    bool m_OneSimUpdatePerFrame;
    // Whether the simulation is limted to going at 1.0x and not faster
    bool m_SimSpeedLimited;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TimerMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // Disallow the use of some implicit methods.
    TimerMan(const TimerMan &reference);
    TimerMan & operator=(const TimerMan &rhs);

};

} // namespace RTE

#endif // File