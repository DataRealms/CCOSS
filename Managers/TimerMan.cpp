#include "TimerMan.h"
#include "Constants.h"
#include "PerformanceMan.h"
#include "SettingsMan.h"

#ifdef _WIN32
#include <Windows.h>
#elif _LINUX_OR_MACOSX_
#include <time.h>
#endif

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Clear() {
		m_StartTime = std::chrono::steady_clock::now();
		m_TicksPerSecond = 1000000;
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
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Initialize() {
		ResetTime();
		if (m_DeltaTimeS <= 0) { SetDeltaTimeSecs(c_DefaultDeltaTimeS); }
		if (m_RealToSimCap <= 0) { SetRealToSimCap(c_DefaultRealToSimCap); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	long long TimerMan::GetAbsoluteTime() const {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float TimerMan::GetAIDeltaTimeSecs() const {
		return m_DeltaTimeS * static_cast<float>(g_SettingsMan.GetAIUpdateInterval());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::ResetTime() {
		m_StartTime = std::chrono::steady_clock::now();

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
		m_RealTimeTicks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartTime).count();
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

			m_SimSpeed = GetDeltaTimeMS() / g_PerformanceMan.GetMSPFAverage();
			if (IsSimSpeedLimited() && m_SimSpeed > 1.0F) { m_SimSpeed = 1.0F; }
		} else {
			m_SimSpeed = 1.0F;
		}
	}
}
