#include "TimerMan.h"

#include "Constants.h"
#include "PerformanceMan.h"
#include "SettingsMan.h"
#include "ThreadMan.h"
#include "ConsoleMan.h"

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
		m_SimTimeTicks = 0;
		m_SimUpdateCount = 0;
		m_SimAccumulator = 0;
		m_DeltaTime = 0;
		m_DeltaTimeS = 0.0F;
		m_DeltaBuffer.clear();
		m_SimSpeed = 1.0F;
		m_TimeScale = 1.0F;
		m_SimPaused = false;
		m_LatestUpdateStartTime = 0;
		m_LatestUpdateEndTime = 0;
		m_UpdateTrueDeltaTimeTicks = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void TimerMan::Initialize() {
		// Get the frequency of ticks/s for this machine
		m_TicksPerSecond = 1000000;

		ResetTime();
		if (m_DeltaTimeS <= 0) { SetDeltaTimeSecs(c_DefaultDeltaTimeS); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	long long TimerMan::GetAbsoluteTime() const {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float TimerMan::GetRealToSimCap() const {
		return c_RealToSimCap;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float TimerMan::GetAIDeltaTimeSecs() const {
		return m_DeltaTimeS * static_cast<float>(g_SettingsMan.GetAIUpdateInterval());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float TimerMan::GetPredictedProportionOfUpdateCompletion() const {
		// We might be rendering the last sim frame even while a new one is waiting for us
		// If that's the case, return 1.0F, so we don't "jump" back and start rendering the beginning of the last frame again
		if (g_ThreadMan.NewSimFrameIsReady()) {
			return 1.0F;
		}

		// Our full update time is the max of either how long our update actually took, or how often we update 
		double fullUpdateTime = std::max(static_cast<double>(m_UpdateTrueDeltaTimeTicks), m_DeltaTime / static_cast<double>(m_TimeScale));
        double timeSinceLastUpdate = m_RealTimeTicks - m_LatestUpdateEndTime;
		return std::min(timeSinceLastUpdate / fullUpdateTime, 1.0);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void TimerMan::ResetTime()
    {
        m_StartTime = std::chrono::steady_clock::now();

		m_RealTimeTicks = 0;
		m_SimAccumulator = 0;
		m_SimTimeTicks = 0;
		m_SimUpdateCount = 0;
		m_TimeScale = 1.0F;
		m_LatestUpdateStartTime = 0;
		m_LatestUpdateEndTime = 0;
		m_UpdateTrueDeltaTimeTicks = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::UpdateSim() {
		m_LatestUpdateStartTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartTime).count();
		m_SimSpeed = std::min(GetDeltaTimeMS() / g_PerformanceMan.GetMSPUAverage(), m_TimeScale);

		if (TimeForSimUpdate()) {
			// Transfer ticks from the accumulator to the sim time ticks.
			m_SimAccumulator -= m_DeltaTime;
			m_SimTimeTicks += m_DeltaTime;

			++m_SimUpdateCount;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::MarkNewSimUpdateComplete() {
		m_LatestUpdateEndTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartTime).count();
		m_UpdateTrueDeltaTimeTicks = m_LatestUpdateEndTime - m_LatestUpdateStartTime;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TimerMan::Update() {
		long long prevTime = m_RealTimeTicks;
		m_RealTimeTicks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartTime).count();

		// Cap timeIncrease if too long (as when the app went out of focus), to c_RealToSimCap.
		long long timeIncrease = std::min(m_RealTimeTicks - prevTime, static_cast<long long>(c_RealToSimCap * m_TicksPerSecond));

		RTEAssert(timeIncrease > 0, "It seems your CPU is giving bad timing data to the game, this is known to happen on some multi-core processors. This may be fixed by downloading the latest CPU drivers from AMD or Intel.");

		m_DrawDeltaTimeS = static_cast<float>(timeIncrease) / 1000.0F;

		// If not paused, add the new time difference to the sim accumulator
		if (!m_SimPaused) {
			m_SimAccumulator += static_cast<long long>(static_cast<float>(timeIncrease) * m_TimeScale);
		}

		float maxPossibleSimSpeed = GetDeltaTimeMS() / std::max(g_PerformanceMan.GetMSPUAverage(), std::numeric_limits<float>::epsilon());

		// Make sure we don't get runaway behind schedule
		m_SimAccumulator = std::min(m_SimAccumulator.load(), m_DeltaTime + static_cast<long long>(m_DeltaTime * maxPossibleSimSpeed));

		RTEAssert(m_SimAccumulator >= 0, "Negative sim time accumulator?!");

		m_SimSpeed = std::min(maxPossibleSimSpeed, GetTimeScale());
	}
}
