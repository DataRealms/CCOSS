#ifndef _RTETIMERMAN_
#define _RTETIMERMAN_

#include "Singleton.h"

#include <atomic>

#define g_TimerMan TimerMan::Instance()

namespace RTE {

	/// <summary>
	/// The centralized singleton manager of all Timers and overall timekeeping in RTE.
	/// Uses QueryPerformanceCounter for sub-ms resolution timers and the model described in http://www.gaffer.org/game-physics/fix-your-timestep.
	/// </summary>
	class TimerMan : public Singleton<TimerMan> {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TimerMan object in system memory. Initialize() should be called before using this object.
		/// </summary>
		TimerMan() { Clear(); Initialize(); };

		/// <summary>
		/// Makes the TimerMan object ready for use.
		/// </summary>
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the TimerMan object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current time stamp in microseconds unrelated to TimerMan updates. Can be used to measure time intervals during a single frame update.
		/// </summary>
		/// <returns>Current time stamp in microseconds.</returns>
		long long GetAbsoluteTime() const;

		/// <summary>
		/// Sets the sim to be paused, ie no real time ticks will be transferred to the sim accumulator while this is set to true.
		/// This also clears the accumulator, to avoid the case where the sim may update while paused when behind schedule.
		/// </summary>
		/// <param name="pause">Whether the sim should be paused or not.</param>
		void PauseSim(bool pause = false) { m_SimPaused = pause; if(pause) m_SimAccumulator = 0.0F; }

		/// <summary>
		/// Tells whether there is enough sim time accumulated to do at least one physics update.
		/// </summary>
		/// <returns>Whether there is enough sim time to do a physics update.</returns>
		bool TimeForSimUpdate() const { return m_SimAccumulator >= m_DeltaTime; }

		/// <summary>
		/// Gets the simulation speed over real time.
		/// </summary>
		/// <returns>The value of the simulation speed over real time.</returns>
		float GetSimSpeed() const { return m_SimSpeed; }

		/// <summary>
		/// Gets a time scale factor which will be used to speed up or slow down the progress of the simulation time in relation to the real world time.
		/// </summary>
		/// <returns>A factor between the real world time, and the simulation time.</returns>
		float GetTimeScale() const { return m_TimeScale; }

		/// <summary>
		/// Sets a time scale factor which will be used to speed up or slow down the progress of the simulation time in relation to the real world time.
		/// </summary>
		/// <param name="timeScale">A factor between the real world time, and the simulation time. A value of 2.0 means simulation runs twice as fast as normal.</param>
		void SetTimeScale(float timeScale = 1.0F) { m_TimeScale = timeScale; }

		/// <summary>
		/// Gets the cap of the amount of seconds which can be transferred from the real time to the simulated time in one update.
		/// </summary>
		/// <returns>A float describing the current cap in seconds.</returns>
		float GetRealToSimCap() const;

		/// <summary>
		/// Gets the number of ticks per second (the resolution of the timer).
		/// </summary>
		/// <returns>The number of ticks per second.</returns>
		long long GetTicksPerSecond() const { return m_TicksPerSecond; }

		/// <summary>
		/// Gets a current global real time measured in ticks from the start of the simulation up to the last Update of this TimerMan. Use TickFrequency to determine how many ticks go in a second.
		/// </summary>
		/// <returns>The number of ticks passed since the simulation started.</returns>
		long long GetRealTickCount() const { return m_RealTimeTicks; }

		/// <summary>
		/// Gets a current global simulation time measured in ticks from the start of the simulation up to the last Update of this TimerMan. Use TickFrequency to determine how many ticks go in a second.
		/// </summary>
		/// <returns>The number of ticks passed since the simulation started.</returns>
		long long GetSimTickCount() const { return m_SimTimeTicks; }

		/// <summary>
		/// Gets a current global simulation time, measured in sim updates, from the start of the simulation up to the last Update of this TimerMan.
		/// </summary>
		/// <returns>The number of simulation updates that have occurred since the simulation started.</returns>
		long long GetSimUpdateCount() const { return m_SimUpdateCount; }

		/// <summary>
		/// Gets a current global simulation time measured in ms ticks from the start of the simulation up to the last UpdateSim of this TimerMan.
		/// </summary>
		/// <returns>The number of ms passed since the simulation started.</returns>
		long long GetSimTimeMS() const { return static_cast<long long>((static_cast<float>(m_SimTimeTicks) / static_cast<float>(m_TicksPerSecond)) * 0.001F); }

		/// <summary>
		/// Gets the current number of ticks that the simulation should be updating with.
		/// </summary>
		/// <returns>The current fixed delta time that the simulation should be updating with, in ticks.</returns>
		long long GetDeltaTimeTicks() const { return m_DeltaTime; }

		/// <summary>
		/// Sets the number of ticks that a simulation update delta time should take.
		/// </summary>
		/// <param name="newDelta">The new delta time in ticks.</param>
		void SetDeltaTimeTicks(int newDelta) { m_DeltaTime = newDelta; m_DeltaTimeS = static_cast<float>(m_DeltaTime) / static_cast<float>(m_TicksPerSecond); }

		/// <summary>
		/// Gets how long the last draw took, in seconds
		/// </summary>
		/// <returns>How long the last draw took, in seconds.</returns>
		float GetDrawDeltaTimeSecs() const { return m_DrawDeltaTimeS; }

		/// <summary>
		/// Gets how long the last draw took, in ms
		/// </summary>
		/// <returns>How long the last draw took, in ms.</returns>
		float GetDrawDeltaTimeMS() const { return m_DrawDeltaTimeS * 1000.0F; }

		/// <summary>
		/// Gets the current fixed delta time of the simulation updates, in ms.
		/// </summary>
		/// <returns>The current fixed delta time that the simulation should be updating with, in ms.</returns>
		float GetDeltaTimeMS() const { return m_DeltaTimeS * 1000.0F; }

		/// <summary>
		/// Gets the current fixed delta time of the simulation updates, in seconds.
		/// </summary>
		/// <returns>The current fixed delta time that the simulation should be updating with, in seconds.</returns>
		float GetDeltaTimeSecs() const { return m_DeltaTimeS; }

		/// <summary>
		/// Gets the current fixed delta time of AI updates, in seconds.
		/// </summary>
		/// <returns>The current fixed delta time of AI updates, in seconds.</returns>
		float GetAIDeltaTimeSecs() const;

		/// <summary>
		/// Gets the current fixed delta time of AI updates, in ms.
		/// </summary>
		/// <returns>The current fixed delta time of AI updates, in ms.</returns>
		float GetAIDeltaTimeMS() const { return GetAIDeltaTimeSecs() * 1000; };

		/// <summary>
		/// Sets the number of seconds that a simulation update delta time should take.
		/// </summary>
		/// <param name="newDelta">The new delta time in seconds.</param>
		void SetDeltaTimeSecs(float newDelta) { m_DeltaTimeS = newDelta; m_DeltaTime = static_cast<long long>(m_DeltaTimeS * static_cast<float>(m_TicksPerSecond)); }

		/// <summary>
		/// Returns the true update delta time, or how long has actually passed since the last update, in real time.
		/// </summary>
		/// <returns>The true update dt, in ms.</returns>
		float GetTrueUpdateDeltaTimeMS() const { return m_UpdateTrueDeltaTimeTicks / 1000.0F; };

		/// <summary>
		/// Returns a predicted proportion of how far the current simulation update is through completion, based on the last update time.
		/// Return 0.0F if a new update just started, and 1.0F if the time since this update started is equal to or past the last update time.
		/// </summary>
		/// <returns>A predicted proportion of how complete the current update cycle is.</returns>
		float GetPredictedProportionOfUpdateCompletion() const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Resets the measured real and simulation times to 0.
		/// </summary>
		void ResetTime();

		/// <summary>
		/// Updates the simulation time to represent the current amount of simulation time passed from the start of the simulation up to the last update.
		/// </summary>
		void UpdateSim();

		/// <summary>
		/// Marks that the last simulation update has completed.
		/// </summary>
		void MarkNewSimUpdateComplete();

		/// <summary>
		/// Updates the real time ticks based on the actual clock time and adds it to the accumulator which the simulation ticks will draw from in whole DeltaTime-sized chunks.
		/// </summary>
		void Update();
#pragma endregion

#pragma region Network Handling
		/// <summary>
		/// Gets the duration the thread should be put to sleep. This is used when ServerSimSleepWhenIdle is true to put the thread to sleep if the sim frame is finished faster than it usually should.
		/// </summary>
		/// <returns>The duration the thread should be put to sleep.</returns>
		long long GetTimeToSleep() const { return (m_DeltaTime - m_SimAccumulator) / 2; };
#pragma endregion

	protected:

		std::chrono::steady_clock::time_point m_StartTime; //!< The point in real time when the simulation (re)started.
		long long m_TicksPerSecond; //!< The frequency of ticks each second, ie the resolution of the timer.
		volatile long long m_RealTimeTicks; //!< The number of actual microseconds counted so far.
		long long m_SimTimeTicks; //!< The number of simulation time ticks counted so far.
		long long m_SimUpdateCount; //!< The number of whole simulation updates have been made since reset.
		std::atomic<long long> m_SimAccumulator; //!< Simulation time accumulator keeps track of how much actual time has passed and is chunked into whole DeltaTime:s upon UpdateSim.

		long long m_DeltaTime; //!< The fixed delta time chunk of the simulation update.
		float m_DeltaTimeS; //!< The simulation update step size, in seconds.
		std::deque<float> m_DeltaBuffer; //!< Buffer for measuring the most recent real time differences, used for averaging out the readings.

		float m_DrawDeltaTimeS; //!< How long the last draw took, in seconds.
		volatile long long m_UpdateTrueDeltaTimeTicks; //!< How long the last update took, in ticks.
		volatile long long m_LatestUpdateStartTime; //!< When our latest update started.
		volatile long long m_LatestUpdateEndTime; //!< When our latest update completed.

		float m_SimSpeed; //!< The simulation speed over real time.
		float m_TimeScale; //!< The relationship between the real world actual time and the simulation time. A value of 2.0 means simulation runs twice as fast as normal, as perceived by a player.

		std::atomic<bool> m_SimPaused; //!< Simulation paused; no real time ticks will go to the sim accumulator.

	private:

		/// <summary>
		/// Clears all the member variables of this TimerMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TimerMan(const TimerMan &reference) = delete;
		TimerMan & operator=(const TimerMan &rhs) = delete;
	};
}
#endif
