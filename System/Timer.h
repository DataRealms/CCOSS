#ifndef _RTETIMER_
#define _RTETIMER_

#include "TimerMan.h"

namespace RTE {

	/// <summary>
	/// A precise timer for FPS sync etc.
	/// </summary>
	class Timer {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Timer object.
		/// </summary>
		Timer() { Clear(); Create(); }

		/// <summary>
		/// Constructor method used to instantiate a Timer object with a set sim time elapsed.
		/// </summary>
		/// <param name="elapsedSimTime">A unsigned long defining the amount of time (in ms) that this Timer should start with elapsed.</param>
		Timer(unsigned long elapsedSimTime) { Clear(); Create(elapsedSimTime); }

		/// <summary>
		/// Copy constructor method used to instantiate a Timer object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A Timer object which is passed in by reference.</param>
		Timer(const Timer &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Makes the Timer object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create();

		/// <summary>
		/// Makes the Timer object ready for use.
		/// </summary>
		/// <param name="elapsedSimTime">A unsigned long defining the amount of time (in ms) that this Timer should start with elapsed.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(unsigned long elapsedSimTime);

		/// <summary>
		/// Creates a Timer to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Timer to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Timer &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the timer so that the elapsed time is 0 ms.
		/// </summary>
		// TODO: Figure out why calling Clear() here breaks time.
		void Reset() { m_StartRealTime = g_TimerMan.GetRealTickCount(); m_StartSimTime = g_TimerMan.GetSimTickCount(); }
#pragma endregion

#pragma region Real Time
		/// <summary>
		/// Gets the start real time value of this Timer.
		/// </summary>
		/// <returns>An int64 value that represents the amount of real time in ms from when windows was started to when Reset() of this Timer was called.</returns>
		int64_t GetStartRealTimeMS() const { return m_StartRealTime; }

		/// <summary>
		/// Sets the start real time value of this Timer.
		/// </summary>
		/// <param name="newStartTime">An int64 with the new real time value (ms since the OS was started).</param>
		void SetStartRealTimeMS(const int64_t newStartTime) { m_StartRealTime = newStartTime * m_TicksPerMS; }

		/// <summary>
		/// Gets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <returns>A positive double with the real time limit relative to the start time.</returns>
		double GetRealTimeLimitMS() const { return m_RealTimeLimit / m_TicksPerMS; }

		/// <summary>
		/// Sets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <param name="newTimeLimit">A positive double with the new real time limit relative to the start time.</param>
		void SetRealTimeLimitMS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit * m_TicksPerMS; }

		/// <summary>
		/// Gets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <returns>A positive double with the real time limit relative to the start time.</returns>
		double GetRealTimeLimitS() const { return m_RealTimeLimit / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Sets the real time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <param name="newTimeLimit">A positive double with the new real time limit relative to the start time.</param>
		void SetRealTimeLimitS(double newTimeLimit) { m_RealTimeLimit = newTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Gets the elapsed real time in ms since this Timer was Reset().
		/// </summary>
		/// <returns>A unsigned long value that represents the elapsed real time since Reset() in ms.</returns>
		double GetElapsedRealTimeMS() const { return static_cast<double>(g_TimerMan.GetRealTickCount() - m_StartRealTime) / m_TicksPerMS; }

		/// <summary>
		/// Sets the start real time value of this Timer, in seconds.
		/// </summary>
		/// <param name="newElapsedRealTime">An int64 with the new elapsed time value.</param>
		void SetElapsedRealTimeMS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * m_TicksPerMS); }

		/// <summary>
		/// Gets the elapsed real time in seconds since this Timer was Reset().
		/// </summary>
		/// <returns>A double value that represents the elapsed real time since Reset() in s.</returns>
		double GetElapsedRealTimeS() const { return static_cast<double>(g_TimerMan.GetRealTickCount() - m_StartRealTime) / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Sets the start real time value of this Timer.
		/// </summary>
		/// <param name="newElapsedRealTime">An int64 with the new elapsed time value in seconds.</param>
		void SetElapsedRealTimeS(const double newElapsedRealTime) { m_StartRealTime = g_TimerMan.GetRealTickCount() - (newElapsedRealTime * static_cast<double>(g_TimerMan.GetTicksPerSecond())); }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit.
		/// </summary>
		/// <param name="when">A unsigned long specifying till when there is time.</param>
		/// <returns>A unsigned long with the time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		unsigned long LeftTillRealMS(int64_t when) { return when - GetElapsedRealTimeMS(); }

		/// <summary>
		/// Returns true if the elapsed real time is past a certain amount of time relative to this' start.
		/// </summary>
		/// <param name="limit">A long specifying the threshold amount of real time in ms.</param>
		/// <returns>A bool only yielding true if the elapsed real time is greater than the passed in value.</returns>
		bool IsPastRealMS(long limit) { return GetElapsedRealTimeMS() > limit; }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetRealTimeLimitMS.
		/// </summary>
		/// <returns>A unsigned long with the real time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		double LeftTillRealTimeLimitMS() { return (m_RealTimeLimit / m_TicksPerMS) - GetElapsedRealTimeMS(); }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetRealTimeLimitS.
		/// </summary>
		/// <returns>A unsigned long with the real time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		double LeftTillRealTimeLimitS() { return (m_RealTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond())) - GetElapsedRealTimeS(); }

		/// <summary>
		/// Returns true if the elapsed real time is past a certain amount of time after the start previously set by SetRealTimeLimit.
		/// </summary>
		/// <returns>A bool only yielding true if the elapsed real time is greater than the set limit value. If no limit has been set, this returns false.</returns>
		bool IsPastRealTimeLimit() { return (m_RealTimeLimit == 0) ? true : (m_RealTimeLimit > 0 && (g_TimerMan.GetRealTickCount() - m_StartRealTime) > m_RealTimeLimit); }

		/// <summary>
		/// Returns how much progress has been made toward the set time limit previously set by SetRealTimeLimitMS.
		/// 0 means no progress, 1.0 means the timer has reached, or is beyond the limit.
		/// </summary>
		/// <returns>A normalized scalar between 0.0 - 1.0 showing the progress toward the limit.</returns>
		double RealTimeLimitProgress() const { return (m_RealTimeLimit == 0) ? 1.0 : (std::min(1.0, GetElapsedRealTimeMS() / (m_RealTimeLimit / m_TicksPerMS))); }

		/// <summary>
		/// Returns true or false, depending on whether the elapsed time falls in one of two repeating intervals which divide it.
		/// This is useful for blink animations etc.
		/// </summary>
		/// <param name="period">An int with the alternating period in ms. The time specified here is how long it will take for the switch to alternate.</param>
		/// <returns>Whether the elapsed time is in the first state or not.</returns>
		bool AlternateReal(int period) const { return (static_cast<int>(GetElapsedRealTimeMS()) % (period * 2)) > period; }
#pragma endregion

#pragma region Simulation Time
		/// <summary>
		/// Gets the start time value of this Timer.
		/// </summary>
		/// <returns>An int64 value that represents the amount of time in ticks from when windows was started to when Reset() of this Timer was called.</returns>
		int64_t GetStartSimTimeMS() const { return m_StartSimTime; }

		/// <summary>
		/// Sets the start time value of this Timer, in ticks
		/// </summary>
		/// <param name="newStartTime">An int64 with the new time value (ms since windows was started).</param>
		void SetStartSimTimeMS(const int64_t newStartTime) { m_StartSimTime = newStartTime * m_TicksPerMS; }

		/// <summary>
		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <returns>A positive double with the sim time limit relative to the start time.</returns>
		double GetSimTimeLimitMS() const { return m_SimTimeLimit / m_TicksPerMS; }

		/// <summary>
		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <param name="newTimeLimit">A positive double with the new sim time limit relative to the start time.</param>
		void SetSimTimeLimitMS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * m_TicksPerMS; }

		/// <summary>
		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <returns>A positive double with the sim time limit relative to the start time.</returns>
		double GetSimTimeLimitS() const { return m_SimTimeLimit / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Sets the sim time limit value of this Timer, RELATVE to the start time.
		/// This is when the timer is supposed to show that it has 'expired' or reached whatever time limit it is supposed to keep track of.
		/// </summary>
		/// <param name="newTimeLimit">A positive double with the new sim time limit relative to the start time.</param>
		void SetSimTimeLimitS(double newTimeLimit) { m_SimTimeLimit = newTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Gets the elapsed time in ms since this Timer was Reset().
		/// </summary>
		/// <returns>A unsigned long value that represents the elapsed time since Reset() in ms.</returns>
		double GetElapsedSimTimeMS() const { return static_cast<double>(g_TimerMan.GetSimTickCount() - m_StartSimTime) / m_TicksPerMS; }

		/// <summary>
		/// Sets the start time value of this Timer, in ms.
		/// </summary>
		/// <param name="newElapsedSimTime">A double with the new elapsed time value.</param>
		void SetElapsedSimTimeMS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * m_TicksPerMS); }

		/// <summary>
		/// Gets the elapsed time in s since this Timer was Reset().
		/// </summary>
		/// <returns>A unsigned long value that represents the elapsed time since Reset() in s.</returns>
		double GetElapsedSimTimeS() const { return static_cast<double>(g_TimerMan.GetSimTickCount() - m_StartSimTime) / static_cast<double>(g_TimerMan.GetTicksPerSecond()); }

		/// <summary>
		/// Sets the start time value of this Timer, in seconds.
		/// </summary>
		/// <param name="newElapsedSimTime">An int64 with the new elapsed time value in seconds.</param>
		void SetElapsedSimTimeS(const double newElapsedSimTime) { m_StartSimTime = g_TimerMan.GetSimTickCount() - (newElapsedSimTime * static_cast<double>(g_TimerMan.GetTicksPerSecond())); }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit.a certain time limit.
		/// </summary>
		/// <param name="when">A unsigned long specifying till when there is time.</param>
		/// <returns>A unsigned long with the time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		double LeftTillSimMS(double when) const { return when - GetElapsedSimTimeMS(); }

		/// <summary>
		/// Returns true if the elapsed time is past a certain amount of time.
		/// </summary>
		/// <param name="limit">A unsigned long specifying the threshold amount of time in ms.</param>
		/// <returns>A bool only yielding true if the elapsed time is greater than the passed in value.</returns>
		bool IsPastSimMS(double limit) const { return GetElapsedSimTimeMS() > limit; }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetSimTimeLimitMS.
		/// </summary>
		/// <returns>A unsigned long with the sim time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		double LeftTillSimTimeLimitMS() const { return (m_SimTimeLimit / m_TicksPerMS) - GetElapsedSimTimeMS(); }

		/// <summary>
		/// Returns how much time in ms that there is left till this Timer reaches a certain time limit previously set by SetSimTimeLimitS.
		/// </summary>
		/// <returns>A unsigned long with the sim time left till the passed in value, or negative if this Timer is already past that point in time.</returns>
		double LeftTillSimTimeLimitS() const { return (m_SimTimeLimit * static_cast<double>(g_TimerMan.GetTicksPerSecond())) - GetElapsedSimTimeS(); }

		/// <summary>
		/// Returns true if the elapsed sim time is past a certain amount of time after the start previously set by SetSimTimeLimit.
		/// </summary>
		/// <returns>A bool only yielding true if the elapsed real time is greater than the set limit value. If no limit has been set, this returns false.</returns>
		bool IsPastSimTimeLimit() const { return (m_SimTimeLimit == 0) ? true : (m_SimTimeLimit > 0 && (g_TimerMan.GetSimTickCount() - m_StartSimTime) > m_SimTimeLimit); }

		/// <summary>
		/// Returns how much progress has been made toward the set time limit previously set by SetSimTimeLimitMS.
		/// 0 means no progress, 1.0 means the timer has reached, or is beyond the limit.
		/// </summary>
		/// <returns>A normalized scalar between 0.0 - 1.0 showing the progress toward the limit.</returns>
		double SimTimeLimitProgress() const { return (m_SimTimeLimit == 0) ? 1.0 : (std::min(1.0, GetElapsedSimTimeMS() / (m_SimTimeLimit / m_TicksPerMS))); }

		/// <summary>
		/// Returns true or false, depending on whether the elapsed time falls in one of two repeating intervals which divide it.
		/// This is useful for blink animations etc.
		/// </summary>
		/// <param name="period">An int with the alternating period in ms. The time specified here is how long it will take for the switch to alternate.</param>
		/// <returns>Whether the elapsed time is in the first state or not.</returns>
		bool AlternateSim(int period) const { return (period == 0) ? true : (static_cast<int>(GetElapsedSimTimeMS()) % (period * 2)) > period; }
#pragma endregion

	protected:


		double m_TicksPerMS; //!< Ticks per MS.

		int64_t m_StartRealTime; //!< Absolute tick count when this was started in real time.
		int64_t m_RealTimeLimit; //!< Tick count, relative to the start time, when this should indicate end or expired in real time.

		int64_t m_StartSimTime; //!< Absolute tick count when this was started in simulation time.
		int64_t m_SimTimeLimit; //!< Tick count, relative to the start time, when this should indicate end or expired in simulation time.

	private:

		/// <summary>
		/// Clears all the member variables of this Timer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif