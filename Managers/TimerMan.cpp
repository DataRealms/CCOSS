#include "TimerMan.h"
#include "Constants.h"
#include "PerformanceMan.h"

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
		m_RealToSimCap = 0.0F;
		m_SimTimeTicks = 0;
		m_SimUpdateCount = 0;
		m_SimAccumulator = 0;
		m_DeltaTime = 0;
		m_DeltaTimeS = 0.0F;
		m_DeltaBuffer.clear();
		m_SimUpdatesSinceDrawn = -1;
		m_DrawnSimUpdate = false;
		m_SimSpeed = 1.0F;
		m_TimeScale = 1.0F;
		m_SimPaused = false;
		// This gets dynamically turned on for short periods when sim gets heavy (explosions) and slow-mo effect is appropriate.
		m_OneSimUpdatePerFrame = false;
		m_SimSpeedLimited = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TimerMan::TimerMan() {
		Clear();

#ifdef _WIN32
		LARGE_INTEGER tempLInt;
		QueryPerformanceFrequency(&tempLInt);
		m_TicksPerSecond = tempLInt.QuadPart;
#elif __unix__
		timespec my_TimeSpec;
		clock_getres(CLOCK_MONOTONIC, &my_TimeSpec);
		m_TicksPerSecond = ((1e9 / my_TimeSpec.tv_nsec) / 1000);
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Initialize() {
		ResetTime();
		if (m_DeltaTimeS <= 0) { SetDeltaTimeSecs(c_DefaultDeltaTimeS); }
		if (m_RealToSimCap <= 0) { SetRealToSimCap(c_DefaultRealToSimCap); }
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
		// Get the nanoseconds value for right now and convert it to microseconds, since we don't honestly need anything more than that.
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
		// Get the nanoseconds value for right now and convert it to microseconds, since we don't honestly need anything more than that.
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
			// Transfer ticks from the accumulator to the sim time ticks.
			m_SimAccumulator -= m_DeltaTime;
			m_SimTimeTicks += m_DeltaTime;

			++m_SimUpdateCount;
			++m_SimUpdatesSinceDrawn;

			// If after deducting the DeltaTime from the accumulator, there is not enough time for another DeltaTime, then flag this as the last sim update before the frame is drawn.
			m_DrawnSimUpdate = !TimeForSimUpdate();
		} else {
			m_DrawnSimUpdate = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Update() {
		long long prevTime = m_RealTimeTicks;
		// Increase the real time ticks with the amount of actual time passed since the last Update.
#ifdef _WIN32
		LARGE_INTEGER tickReading;
		QueryPerformanceCounter(&tickReading);
		m_RealTimeTicks = tickReading.QuadPart - m_StartTime;
#elif __unix__
		std::uint64_t curTime;
		timespec my_TimeSpec;
		clock_gettime(CLOCK_MONOTONIC, &my_TimeSpec);
		curTime = static_cast<long long>((my_TimeSpec.tv_sec * 1000000) + (my_TimeSpec.tv_nsec / 1000));
		m_RealTimeTicks = curTime - m_StartTime;
#endif
		// Figure the increase in real time.
		unsigned long long timeIncrease = m_RealTimeTicks - prevTime;
		// Cap it if too long (as when the app went out of focus).
		if (timeIncrease > m_RealToSimCap) { timeIncrease = m_RealToSimCap; }

		RTEAssert(timeIncrease > 0, "It seems your CPU is giving bad timing data to the game, this is known to happen on some multi-core processors. This may be fixed by downloading the latest CPU drivers from AMD or Intel.");

		// If not paused, add the new time difference to the sim accumulator, scaling by the TimeScale.
		if (!m_SimPaused) { m_SimAccumulator += static_cast<long long>(static_cast<float>(timeIncrease) * m_TimeScale); }

		RTEAssert(m_SimAccumulator >= 0, "Negative sim time accumulator?!");

		// Reset the counter since the last drawn update. Set it negative since we're counting full pure sim updates and this will be incremented to 0 on next SimUpdate.
		if (m_DrawnSimUpdate) { m_SimUpdatesSinceDrawn = -1; }

		// Override the accumulator and just put one delta time in there so sim updates only once per frame.
		if (m_OneSimUpdatePerFrame) {
			// Only let it appear to go slower, not faster, if limited.
			if (m_SimSpeedLimited && m_SimAccumulator > m_DeltaTime) { m_SimAccumulator = m_DeltaTime; }

			// Reset the counter of sim updates since the last drawn. it will always be 0 since every update results in a drawn frame.
			m_SimUpdatesSinceDrawn = -1;

			m_SimSpeed = GetDeltaTimeMS() / static_cast<float>(g_PerformanceMan.GetMSPFAverage());
			if (IsSimSpeedLimited() && m_SimSpeed > 1.0F) { m_SimSpeed = 1.0F; }
		} else {
			m_SimSpeed = 1.0F;
		}
	}
}
