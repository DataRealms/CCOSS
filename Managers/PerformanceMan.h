#ifndef _RTEPERFORMANCEMAN_
#define _RTEPERFORMANCEMAN_

#include "Singleton.h"

#define g_PerformanceMan PerformanceMan::Instance()

namespace RTE {

	class AllegroBitmap;
	class Timer;

	/// <summary>
	/// Singleton manager responsible for all performance stats counting and drawing.
	/// </summary>
	class PerformanceMan : public Singleton<PerformanceMan> {
		friend class SettingsMan;

	public:

		/// <summary>
		/// Enumeration of all available performance counters.
		/// </summary>
		enum PerformanceCounters {
			SimTotal = 0,
			ActorsAIUpdate,
			ActorsTravel,
			ActorsUpdate,
			ParticlesTravel,
			ParticlesUpdate,
			ActivityUpdate,
			PerfCounterCount
		};

#pragma region Creation
		/// <summary>
		///  Constructor method used to instantiate a PerformanceMan object in system memory. Create() should be called before using the object.
		/// </summary>
		PerformanceMan() { Clear(); }

		/// <summary>
		/// Makes the PerformanceMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire PerformanceMan to the default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this PerformanceMan. Supposed to be done every sim update.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the performance stats to the screen.
		/// </summary>
		/// <param name="whichScreen">Which player screen to draw on.</param>
		/// <param name="whichScreenGUIBitmap">The GUI bitmap to draw to.</param>
		void Draw(AllegroBitmap &bitmapToDrawTo);

		/// <summary>
		/// Draws the current ping value to the screen.
		/// </summary>
		void DrawCurrentPing() const;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Tells whether to display the performance stats on-screen or not.
		/// </summary>
		/// <returns>Whether to show the performance stats or not.</returns>
		bool IsShowingPerformanceStats() const { return m_ShowPerfStats; }

		/// <summary>
		/// Sets whether to display the performance stats on-screen or not.
		/// </summary>
		/// <param name="showStats">Whether to show the performance stats or not.</param>
		void ShowPerformanceStats(bool showStats = true) { m_ShowPerfStats = showStats; }

		/// <summary>
		/// Tells whether to display the performance graphs on-screen or not.
		/// </summary>
		/// <returns>Whether to show the performance graphs or not.</returns>
		bool AdvancedPerformanceStatsEnabled() const { return m_AdvancedPerfStats; }

		/// <summary>
		/// Sets whether to display the performance graphs on-screen or not.
		/// </summary>
		/// <param name="showGraphs">Whether to show the performance graphs or not.</param>
		void ShowAdvancedPerformanceStats(bool showGraphs = true) { m_AdvancedPerfStats = showGraphs; }

		/// <summary>
		/// Gets the average of the MSPF reading buffer, calculated each frame.
		/// </summary>
		/// <returns>The average value of the MSPF reading buffer.</returns>
		int GetMSPFAverage() const { return m_MSPFAverage; }
#pragma endregion

#pragma region Performance Counter Handling
		/// <summary>
		/// Resets the frame timer to restart counting.
		/// </summary>
		void ResetFrameTimer() const;

		/// <summary>
		/// Moves sample counter to next sample and clears it's values.
		/// </summary>
		void NewPerformanceSample();

		/// <summary>
		/// Saves current absolute time in microseconds as a start of performance measurement.
		/// </summary>
		/// <param name="counter">Counter to start measurement for.</param>
		void StartPerformanceMeasurement(PerformanceCounters counter);

		/// <summary>
		/// Saves current absolute time in microseconds as an end of performance measurement. The difference is added to the value of current performance sample.
		/// </summary>
		/// <param name="counter">Counter to stop and updated measurement for.</param>
		void StopPerformanceMeasurement(PerformanceCounters counter);

		/// <summary>
		/// Sets the current ping value to display.
		/// </summary>
		/// <param name="ping">Ping value to display.</param>
		void SetCurrentPing(int ping) { m_CurrentPing = ping; }
#pragma endregion

	protected:

		static constexpr int c_MSPFAverageSampleSize = 10; //!< How many samples to use to calculate average MSPF value.
		static constexpr int c_MaxSamples = 120; //!< How many performance samples to store, directly affects graph size.
		static constexpr int c_Average = 10; //!< How many samples to use to calculate average value displayed on screen.

		const int c_StatsOffsetX = 17; //!< Offset of the stat text from the left edge of the screen.
		const int c_StatsHeight = 14; //!< Height of each stat text line.
		const int c_GraphsOffsetX = 14; //!< Offset of the graph from the left edge of the screen.
		const int c_GraphsStartOffsetY = 134; //!< Position the first graph block will be drawn from the top edge of the screen.
		const int c_GraphHeight = 20; //!< Height of the performance graph.
		const int c_GraphBlockHeight = 34; //!< Height of the whole graph block (text height and graph height combined).

		bool m_ShowPerfStats; //!< Whether to show performance stats on screen or not.
		bool m_AdvancedPerfStats; //!< Whether to show performance graphs on screen or not.

		std::unique_ptr<Timer> m_FrameTimer; //!< Timer for measuring milliseconds per frame for performance stats readings.
		int m_Sample; //!< Sample counter.

		std::deque<int> m_MSPFs; //!< History log of readings, for averaging the results.
		int m_MSPFAverage; //!< The average of the MSPF reading buffer above, calculated each frame.
		int m_CurrentPing; //!< Current ping value to display on screen.

		std::array<std::array<int, c_MaxSamples>, PerformanceCounters::PerfCounterCount>  m_PerfPercentages; //!< Array to store percentages from SimTotal.
		std::array<std::array<uint64_t, c_MaxSamples>, PerformanceCounters::PerfCounterCount> m_PerfData; //!< Array to store performance measurements in microseconds.
		std::array<uint64_t, PerformanceCounters::PerfCounterCount> m_PerfMeasureStart; //!< Current measurement start time in microseconds.
		std::array<uint64_t, PerformanceCounters::PerfCounterCount> m_PerfMeasureStop; //!< Current measurement stop time in microseconds.
		std::array<std::string, PerformanceCounters::PerfCounterCount> m_PerfCounterNames; //!< Performance counter names displayed on screen.

	private:

#pragma region Performance Counter Handling
		/// <summary>
		/// Adds provided value to current sample of specified performance counter.
		/// </summary>
		/// <param name="counter">Counter to update.</param>
		/// <param name="value">Value to add to this counter.</param>
		void AddPerformanceSample(PerformanceCounters counter, uint64_t value) { m_PerfData.at(counter).at(m_Sample) += value; }

		/// <summary>
		/// Calculates current sample's percentages from SIM_TOTAL for all performance counters and stores them to m_PerfPercenrages.
		/// </summary>
		void CalculateSamplePercentages();

		/// <summary>
		/// Returns an average value of c_Average last samples for specified performance counter.
		/// </summary>
		/// <param name="counter">Counter to get average value from.</param>
		/// <returns>An average value for specified counter.</returns>
		uint64_t GetPerformanceCounterAverage(PerformanceCounters counter) const;
#pragma endregion

		/// <summary>
		/// Draws the performance graphs to the screen. This will be called by Draw() if advanced performance stats are enabled.
		/// </summary>
		void DrawPeformanceGraphs(AllegroBitmap &bitmapToDrawTo);

		/// <summary>
		/// Clears all the member variables of this PerformanceMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PerformanceMan(const PerformanceMan &reference) = delete;
		PerformanceMan & operator=(const PerformanceMan &rhs) = delete;
	};
}
#endif