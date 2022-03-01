#include "TimerMan.h"
#include "PerformanceMan.h"
#include "AudioMan.h"

#ifdef _WIN32
#include <Windows.h>
#elif __unix__
#include <time.h>
#endif

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Clear() {
		m_StartTime = 0;
		m_TicksPerSecond = 1;
		m_RealTimeTicks = 0;
		m_RealToSimCap = 0;
		m_SimTimeTicks = 0;
		m_SimUpdateCount = 0;
		m_SimAccumulator = 0;
		m_DeltaTime = 0;
		m_DeltaTimeS = 0.016666666F;
		m_DeltaBuffer.clear();
		m_SimUpdatesSinceDrawn = -1;
		m_DrawnSimUpdate = false;
		m_SimSpeed = 1.0F;
		m_TimeScale = 1.0F;
		m_SimPaused = false;
		// This gets dynamically turned on for short periods when sim gets heavy (explosions) and slow-mo effect is appropriate
		m_OneSimUpdatePerFrame = false;
		m_SimSpeedLimited = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int TimerMan::Initialize() {
		// Get the frequency of ticks/s for this machine
#ifdef _WIN32
		LARGE_INTEGER tempLInt;
		QueryPerformanceFrequency(&tempLInt);
		m_TicksPerSecond = tempLInt.QuadPart;
#elif __unix__
		timespec my_TimeSpec;
		clock_getres(CLOCK_MONOTONIC, &my_TimeSpec);

		m_TicksPerSecond= ((1e9 / my_TimeSpec.tv_nsec) / 1000);
#endif

		// Reset the real time setting so that we can measure how much real time has passed till the next Update.
		ResetTime();

		// Calculate a reasonable delta time in ticks, based on the now known frequency
		SetDeltaTimeSecs(m_DeltaTimeS);

		// Set up a default cap if one hasn't been set yet
		if (m_RealToSimCap <= 0) { m_RealToSimCap = 0.0333333F * m_TicksPerSecond; }

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	long long TimerMan::GetAbsoluteTime() const {
#ifdef _WIN32
		LARGE_INTEGER tickReading;
		QueryPerformanceCounter(&tickReading);
		long long ticks = tickReading.QuadPart;
#elif __unix__
		timespec my_TimeSpec;
		clock_gettime(CLOCK_MONOTONIC, &my_TimeSpec);
		// Get the nanoseconds value for right now and convert it to microseconds, since we don't
		// honestly need anything more than that.
		long long ticks = static_cast<int64_t>((my_TimeSpec.tv_sec * 1000000) + (my_TimeSpec.tv_nsec / 1000));
#endif
	  return (ticks * 1000000) / m_TicksPerSecond;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::ResetTime() {
#ifdef _WIN32
		LARGE_INTEGER tempLInt;
		QueryPerformanceCounter(&tempLInt);
		m_StartTime = tempLInt.QuadPart;
#elif __unix__
		timespec my_TimeSpec;
		clock_gettime(CLOCK_MONOTONIC, &my_TimeSpec);
		// Get the nanoseconds value for right now and convert it to microseconds, since we don't
		// honestly need anything more than that.
		m_StartTime = static_cast<int64_t>((my_TimeSpec.tv_sec * 1000000) + (my_TimeSpec.tv_nsec / 1000));
#endif

		m_RealTimeTicks = 0;
		m_SimAccumulator = 0;
		m_SimTimeTicks = 0;
		m_SimUpdateCount = 0;
		m_SimUpdatesSinceDrawn = -1;
		m_DrawnSimUpdate = false;
		m_TimeScale = 1.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::UpdateSim() {
		if (TimeForSimUpdate()) {
			// Transfer ticks from the accumulator to the sim time ticks
			m_SimAccumulator -= m_DeltaTime;
			m_SimTimeTicks += m_DeltaTime;
			// Increment the sim update count
			++m_SimUpdateCount;
			++m_SimUpdatesSinceDrawn;

			// If after deducting the DeltaTime from the Accumulator, there is not enough time for another DeltaTime, then flag this as the last sim update before the frame is drawn.
			m_DrawnSimUpdate = !TimeForSimUpdate();
		} else {
			m_DrawnSimUpdate = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Update() {
		long long prevTime = m_RealTimeTicks;
#ifdef _WIN32
		LARGE_INTEGER tickReading;

		// Increase the real time ticks with the amount of actual time passed since the last Update
		QueryPerformanceCounter(&tickReading);

		m_RealTimeTicks = tickReading.QuadPart - m_StartTime;
#elif __unix__
		std::uint64_t curTime;
		timespec my_TimeSpec;
		clock_gettime(CLOCK_MONOTONIC, &my_TimeSpec);
		curTime= static_cast<long long>((my_TimeSpec.tv_sec * 1000000) + (my_TimeSpec.tv_nsec / 1000));
		m_RealTimeTicks = curTime - m_StartTime;
#endif
		// Figure the increase in real time
		unsigned long long timeIncrease = m_RealTimeTicks - prevTime;
		// Cap it if too long (as when the app went out of focus)
		if (timeIncrease > m_RealToSimCap) { timeIncrease = m_RealToSimCap; }

		RTEAssert(timeIncrease > 0, "It seems your CPU is giving bad timing data to the game, this is known to happen on some multi-core processors. This may be fixed by downloading the latest CPU drivers from AMD or Intel.");

		// If not paused, add the new time difference to the sim accumulator, scaling by the TimeScale
		if (!m_SimPaused) { m_SimAccumulator += timeIncrease * m_TimeScale; }

		RTEAssert(m_SimAccumulator >= 0, "Negative sim time accumulator?!");

		// Reset the counter since the last drawn update. Set it negative since we're counting full pure sim updates and this will be incremented to 0 on next SimUpdate
		if (m_DrawnSimUpdate) { m_SimUpdatesSinceDrawn = -1; }

		float globalPitch = 1.0F;

		// Override the accumulator and just put one delta time in there so sim updates only once per frame
		if (m_OneSimUpdatePerFrame) {
			// Only let it appear to go slower, not faster, if limited
			if (m_SimSpeedLimited && m_SimAccumulator > m_DeltaTime) { m_SimAccumulator = m_DeltaTime; }

			// Reset the counter of sim updates since the last drawn.. it will always be 0 since every update results in a drawn frame
			m_SimUpdatesSinceDrawn = -1;

			m_SimSpeed = GetDeltaTimeMS() / static_cast<float>(g_PerformanceMan.GetMSPFAverage());
			if (IsSimSpeedLimited() && m_SimSpeed > 1.0F) { m_SimSpeed = 1.0F; }

			// Soften the ratio of the pitch adjustment so it's not such an extreme effect on the audio
			// TODO: This coefficient should probably move to SettingsMan and be loaded from ini. That way this effect can be lessened or even turned off entirely by users. 0.35 is a good default value though.
			globalPitch = m_SimSpeed + (1.0F - m_SimSpeed) * 0.35F;
		} else {
			m_SimSpeed = 1.0F;
		}
		// TODO: Handle this from AudioMan::Update
		g_AudioMan.SetGlobalPitch(globalPitch);
	}
}
