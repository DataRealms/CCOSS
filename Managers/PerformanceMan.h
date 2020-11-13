#ifndef _RTEPERFORMANCEMAN_
#define _RTEPERFORMANCEMAN_

#include "Singleton.h"
#include "Constants.h"

#define g_PerformanceMan PerformanceMan::Instance()

namespace RTE {

	class AllegroBitmap;
	class Timer;

	/// <summary>
	/// Singleton manager responsible for all performance stats counting and drawing.
	/// </summary>
	class PerformanceMan : public Singleton<PerformanceMan> {

	public:

		/// <summary>
		/// Enumeration of all available performance counters.
		/// </summary>
		enum PerformanceCounters {
			PERF_SIM_TOTAL = 0,
			PERF_ACTORS_AI,
			PERF_ACTORS_PASS2,
			PERF_ACTORS_PASS1,
			PERF_PARTICLES_PASS2,
			PERF_PARTICLES_PASS1,
			PERF_ACTIVITY,
			PERF_COUNT
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
		int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PerformanceMan object before deletion from system memory.
		/// </summary>
		~PerformanceMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PerformanceMan object.
		/// </summary>
		void Destroy();

		/// <summary>
		/// Resets the entire PerformanceMan to the default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this PerformanceMan. Supposed to be done every frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the performance stats to the screen.
		/// </summary>
		/// <param name="whichScreen">Which player screen to draw on.</param>
		/// <param name="whichScreenGUIBitmap">The GUI bitmap to draw to.</param>
		void Draw(AllegroBitmap bitmapToDrawTo);

		/// <summary>
		/// Draws the current ping value to the screen.
		/// </summary>
		void DrawCurrentPing();
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
#pragma endregion

#pragma region Performance Counter Handling
		/// <summary>
		/// Resets the frame timer to restart counting.
		/// </summary>
		void ResetFrameTimer();

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
		void SetCurrentPing(unsigned short ping) { m_CurrentPing = ping; }
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Entity.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.
		static constexpr unsigned short c_MSPFAverageSampleSize = 10; //!< How many samples to use to calculate average MSPF value.
		static constexpr unsigned short c_MaxSamples = 120; //!< How many performance samples to store, directly affects graph size.
		static constexpr unsigned short c_Average = 10; //!< How many samples to use to calculate average value displayed on screen.

		const unsigned short c_StatsOffsetX = 17; //!< Offset of the stat text from the left edge of the screen.
		const unsigned short c_StatsHeight = 14; //!< Height of each stat text line.
		const unsigned short c_GraphsOffsetX = 14; //!< Offset of the graph from the left edge of the screen.
		const unsigned short c_GraphsStartOffsetY = 134; //!< Position the first graph block will be drawn from the top edge of the screen.
		const unsigned short c_GraphHeight = 20; //!< Height of the performance graph.
		const unsigned short c_GraphBlockHeight = 34; //!< Height of the whole graph block (text height and graph height combined).

		bool m_ShowPerfStats; //!< Whether to show performance stats on screen or not.
		bool m_AdvancedPerfStats; //!< Whether to show performance graphs on screen or not.

		Timer *m_FrameTimer; //!< Timer for measuring milliseconds per frame for performance stats readings.
		float m_SimSpeed; //!< The simulation speed over real time.
		unsigned short m_Sample; //!< Sample counter.

		std::deque<unsigned int> m_MSPFs; //!< History log of readings, for averaging the results.
		size_t m_MSPFAverage; //!< The average of the MSPF reading buffer above, calculated each frame.
		unsigned short m_CurrentPing; //!< Current ping value to display on screen.

		std::string m_PerfCounterNames[PERF_COUNT]; //!< Performance counter names displayed on screen.
		unsigned short m_PerfPercentages[PERF_COUNT][c_MaxSamples]; //!< Array to store percentages from PERF_SIM_TOTAL.
		unsigned long long m_PerfData[PERF_COUNT][c_MaxSamples]; //!< Array to store performance measurements in microseconds.	
		unsigned long long m_PerfMeasureStart[PERF_COUNT]; //!< Current measurement start time in microseconds.
		unsigned long long m_PerfMeasureStop[PERF_COUNT]; //!< Current measurement stop time in microseconds.

	private:

#pragma region Performance Counter Handling
		/// <summary>
		/// Adds provided value to current sample of specified performance counter.
		/// </summary>
		/// <param name="counter">Counter to update.</param>
		/// <param name="value">Value to add to this counter.</param>
		void AddPerformanceSample(PerformanceCounters counter, int64_t value) { m_PerfData[counter][m_Sample] += value; }

		/// <summary>
		/// Calculates current sample's percentages from SIM_TOTAL for all performance counters and stores them to m_PerfPercenrages.
		/// </summary>
		void CalculateSamplePercentages();

		/// <summary>
		/// Returns an average value of c_Average last samples for specified performance counter.
		/// </summary>
		/// <param name="counter">Counter to get average value from.</param>
		/// <returns>An average value for specified counter.</returns>
		unsigned long long GetPerformanceCounterAverage(PerformanceCounters counter) const;
#pragma endregion

		/// <summary>
		/// Draws the performance graphs to the screen. This will be called by Draw() if advanced performance stats are enabled.
		/// </summary>
		void DrawPeformanceGraphs(AllegroBitmap bitmapToDrawTo);

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