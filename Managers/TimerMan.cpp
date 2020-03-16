//////////////////////////////////////////////////////////////////////////////////////////
// File:            TimerMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the TimerMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files


#include "TimerMan.h"

/* Obsolete Allegro timer
// Needs to be declared volatile so that the optimizer doens't mess with it
volatile unsigned long g_MSSinceStart = 0;

// Millisecond-counting interrupt timer
void TimerMSTick()
{
   g_MSSinceStart++;
}
END_OF_FUNCTION(TimerMSTick);
*/

namespace RTE {

#define DELTABUFFERSIZE 30

const std::string TimerMan::m_ClassName = "TimerMan";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TimerMan, effectively
//                  resetting the members of this abstraction level only.

void TimerMan::Clear()
{
    m_StartTime = 0;
    m_TicksPerSecond = 1;
    m_RealTimeTicks = 0;
    m_RealToSimCap = 0;
    m_SimAccumulator = 0;
    m_DeltaTime = 0;
    m_DeltaTimeS = 0.016666666f;
    m_SimTimeTicks = 0;
    m_SimUpdateCount = 0;
    m_SimUpdatesSinceDrawn = -1;
    m_DrawnSimUpdate = false;
    m_TimeScale = 1.0;
    m_AveragingEnabled = false;
    m_DeltaBuffer.clear();
    m_SimPaused = false;
    // This gets dynamically turned on for short periods when sim gets heavy (explosions) and slomo effect is appropriate
    m_OneSimUpdatePerFrame = false;
    m_SimSpeedLimited = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TimerMan object ready for use.

int TimerMan::Create()
{
/*
    // Allegro interrupt init
    install_timer();

    // Lock the timer and functions involved in the interrupt. Allegro crap.
    LOCK_VARIABLE(g_MSSinceStart);
    LOCK_FUNCTION(TimerMSTick);
    // Install the Allegro timer interrupt to increment the g_MSSinceStart every millisecond
    install_int(TimerMSTick, 1);
*/

    // Get the frequency of ticks/s for this machine
    LARGE_INTEGER tempLInt;
    QueryPerformanceFrequency(&tempLInt);
    m_TicksPerSecond = tempLInt.QuadPart;

    // Reset the real time setting so that we can measure how much real time
    // has passed till the next Update.
    ResetTime();

    // Calculate a reasonable delta time in ticks, based on the now known frequency
    SetDeltaTimeSecs(m_DeltaTimeS);

    // Set up a default cap if one hasn't been set yet
    if (m_RealToSimCap <= 0)
        m_RealToSimCap = 0.0333333f * m_TicksPerSecond;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TimerMan object.

void TimerMan::Destroy()
{


    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the measured real and sim times to 0.
// Arguments:       None.
// Return value:    None.

void TimerMan::ResetTime()
{
    // Set the new starting point
    LARGE_INTEGER tempLInt;
    QueryPerformanceCounter(&tempLInt);
    m_StartTime = tempLInt.QuadPart;

    // Reset the counters
    m_RealTimeTicks = 0;
    m_SimAccumulator = 0;
    m_SimTimeTicks = 0;
    m_SimUpdateCount = 0;
    m_SimUpdatesSinceDrawn = -1;
    m_DrawnSimUpdate = false;
    m_TimeScale = 1.0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimTimeMS
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a current global simulation time measured in ms ticks from the start
//                  of the simulation up to the last UpdateSim of this TimerMan.

int64_t TimerMan::GetSimTimeMS() const
{
    return (m_SimTimeTicks / m_TicksPerSecond) * 0.001f;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAbsoulteTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns current time stamp in microseconds unrelated to TimerMan updates.
//					Can be used to measure time intervals during single frame update.
int64_t TimerMan::GetAbsoulteTime()
{
	int64_t ticks;
	LARGE_INTEGER tickReading;

	QueryPerformanceCounter(&tickReading);

	ticks = tickReading.QuadPart;
	
	ticks *= 1000000;
	ticks /= m_TicksPerSecond;

	return ticks;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the simulation time to represent the current amount of sim
//                  time passed from the start of the simulation up to the this last
//                  update. This is accomplished by measuring the real time passed since
//                  the last update, and scaling it by the current time scale.

void TimerMan::Update()
{
    int64_t prevTime = m_RealTimeTicks;

    LARGE_INTEGER tickReading;

    // Increase the real time ticks with the amount of actual time passed since the last Update
    QueryPerformanceCounter(&tickReading);

    m_RealTimeTicks = tickReading.QuadPart - m_StartTime;

    // Figure the increase in real time 
    uint64_t timeIncrease = m_RealTimeTicks - prevTime;
    // Cap it if too long (as when the app went out of focus)
    if (timeIncrease > m_RealToSimCap)
        timeIncrease = m_RealToSimCap;

    // Make sure it's above 0
    if (timeIncrease < 0)
    {
        RTEAbort("It seems your CPU is giving bad timing data to the game. This is known to happen on some multi-core/cpu processors. This may be fixed by downloading the latest CPU drivers from AMD or Intel. A lower resolution timer is going to be used instead until then, please restart the game.");
    }

    // If not paused, add the new time difference to the sim accumulator, scaling by the TimeScale
    if (!m_SimPaused)
        m_SimAccumulator += timeIncrease * m_TimeScale;

    RTEAssert(m_SimAccumulator >= 0, "Negative sim time accumulator?!");

    // Reset the counter since the last drawn update. Set it negative since we're counting full pure sim updates and this will be incremented to 0 on next SimUpdate
    if (m_DrawnSimUpdate)
        m_SimUpdatesSinceDrawn = -1;

    // Override the accumulator and just put one delta time in there so sim updates only once per frame
    if (m_OneSimUpdatePerFrame)
    {
        // Only let it appear to go slower, not faster, if limited
        if (m_SimSpeedLimited && m_SimAccumulator > m_DeltaTime)
            m_SimAccumulator = m_DeltaTime;
        // Reset the counter of sim updates since the last drawn.. it will always be 0 since every update results in a drawn frame
        m_SimUpdatesSinceDrawn = -1;
    }
/*
#ifdef DEBUG_BUILD
    // Override the accumulator and just put one delta time in there so sim updates only once per frame
    m_SimAccumulator = m_DeltaTime;
#endif // _DEBUG
*/
/*
// TODO: Maybe apply delta time smoothing to the FrameMan's deltaTime instead?
    if (m_AveragingEnabled)
    {
        // Add this reading to the averaging buffer
        m_DeltaBuffer.push_back(elapsedRT);

        // Keep the averaging buffer at a certain size
        while (m_DeltaBuffer.size() >= DELTABUFFERSIZE)
            m_DeltaBuffer.pop_front();

        // Calculate the accumulated delta time readings
        long accumulatedRT = 0;
        for (deque<long>::iterator itr = m_DeltaBuffer.begin(); itr != m_DeltaBuffer.end(); ++itr)
            accumulatedRT += *itr;

        // Now get the averaged real time delta reading
        elapsedRT = accumulatedRT / m_DeltaBuffer.size();
    }

    if (elapsedRT > g_FrameMan.GetFrameDelay())
        elapsedRT = g_FrameMan.GetFrameDelay();
    m_SimTimeTicks += elapsedRT;
#else // _DEBUG
    m_SimTimeTicks += g_FrameMan.GetFrameDelay() * m_TimeScale;
#endif // _DEBUG

    // Reset the real time setting so that we can measure how much real time
    // has passed till the next Update.
    m_LastRealTime = g_MSSinceStart;

    // Increment the Simulation Tick counter.
    ++m_SimTickCount;
*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateSim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the simulation time to represent the current amount of sim
//                  time passed from the start of the simulation up to the this last
//                  update.

void TimerMan::UpdateSim()
{
    if (TimeForSimUpdate())
    {
        // Transfer ticks from the accumulator to the sim time ticks
        m_SimAccumulator -= m_DeltaTime;
        m_SimTimeTicks += m_DeltaTime;
        // Increment the sim update count
        ++m_SimUpdateCount;
        ++m_SimUpdatesSinceDrawn;

        // If after deducting the DeltaTime from the Accumulator, there is not enough time for another DeltaTime,
        // then flag this as the last sim update before the frame is drawn
        m_DrawnSimUpdate = !TimeForSimUpdate();
    }
    else
        m_DrawnSimUpdate = true;
}


} // namespace RTE
