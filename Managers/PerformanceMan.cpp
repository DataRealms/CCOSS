#include "PerformanceMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "AudioMan.h"
#include "Timer.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Clear() {
		m_ShowPerfStats = false;
		m_AdvancedPerfStats = true;
		m_CurrentPing = 0;
		m_SimUpdateTimer = nullptr;
		m_MSPSUs.clear();
		m_MSPSUAverage = 0;
		m_MSPFs.clear();
		m_MSPFAverage = 0;
		m_MSPUs.clear();
		m_MSPUAverage = 0;
		m_MSPDs.clear();
		m_MSPDAverage = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PerformanceMan::Initialize() {
		m_SimUpdateTimer = std::make_unique<Timer>();
		m_Sample = 0;

		for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
			m_PerfData[counter].fill(0);
			m_PerfPercentages[counter].fill(0);
		}
		m_PerfMeasureStart.fill(0);
		m_PerfMeasureStop.fill(0);

		// Set up performance counter's names
		m_PerfCounterNames[PerformanceCounters::SimTotal] = "Total";
		m_PerfCounterNames[PerformanceCounters::ActorsTravel] = "Act Travel";
		m_PerfCounterNames[PerformanceCounters::ParticlesTravel] = "Prt Travel";
		m_PerfCounterNames[PerformanceCounters::ActorsUpdate] = "Act Update";
		m_PerfCounterNames[PerformanceCounters::ParticlesUpdate] = "Prt Update";
		m_PerfCounterNames[PerformanceCounters::ActorsAIUpdate] = "Act AI";
		m_PerfCounterNames[PerformanceCounters::ActivityUpdate] = "Activity";

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::UpdateMSPF(long long measuredUpdateTime, long long measuredDrawTime) {
		CalculateTimeAverage(m_MSPUs, m_MSPUAverage, static_cast<float>(measuredUpdateTime / 1000));
		CalculateTimeAverage(m_MSPDs, m_MSPDAverage, static_cast<float>(measuredDrawTime / 1000));
		CalculateTimeAverage(m_MSPFs, m_MSPFAverage, static_cast<float>((measuredUpdateTime + measuredDrawTime) / 1000));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StartPerformanceMeasurement(PerformanceCounters counter) {
		m_PerfMeasureStart[counter] = g_TimerMan.GetAbsoluteTime();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StopPerformanceMeasurement(PerformanceCounters counter) {
		m_PerfMeasureStop[counter] = g_TimerMan.GetAbsoluteTime();
		AddPerformanceSample(counter, m_PerfMeasureStop[counter] - m_PerfMeasureStart[counter]);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::NewPerformanceSample() {
		m_Sample++;
		if (m_Sample >= c_MaxSamples) { m_Sample = 0; }

		for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
			m_PerfData[counter].at(m_Sample) = 0;
			m_PerfPercentages[counter].at(m_Sample) = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::CalculateSamplePercentages() {
		for (int counter = 0; counter < PerformanceCounters::PerfCounterCount; ++counter) {
			int samplePercentage = static_cast<int>(static_cast<float>(m_PerfData[counter].at(m_Sample)) / static_cast<float>(m_PerfData[counter][PerformanceCounters::SimTotal]) * 100);
			m_PerfPercentages[counter].at(m_Sample) = samplePercentage;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	uint64_t PerformanceMan::GetPerformanceCounterAverage(PerformanceCounters counter) const {
		uint64_t totalPerformanceMeasurement = 0;
		int sample = m_Sample;
		for (int i = 0; i < c_Average; ++i) {
			totalPerformanceMeasurement += m_PerfData[counter].at(sample);
			if (sample == 0) { sample = c_MaxSamples; }
			sample--;
		}
		return totalPerformanceMeasurement / c_Average;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::CalculateTimeAverage(std::deque<float> &timeMeasurements, float &avgResult, float newTimeMeasurement) const {
		timeMeasurements.emplace_back(newTimeMeasurement);
		while (timeMeasurements.size() > c_MSPAverageSampleSize) {
			timeMeasurements.pop_front();
		}
		avgResult = 0;
		for (const float &timeMeasurement : timeMeasurements) {
			avgResult += timeMeasurement;
		}
		avgResult /= static_cast<float>(timeMeasurements.size());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Draw(AllegroBitmap &bitmapToDrawTo) {
		if (m_ShowPerfStats) {
			char str[128];

			// Calculate the fps from the average
			float fps = 1.0F / (m_MSPFAverage / 1000.0F);
			float ups = 1.0F / (m_MSPSUAverage / 1000.0F);
			std::snprintf(str, sizeof(str), "FPS: %.0f | UPS: %.0f", fps, ups);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight, str, GUIFont::Left);

			// Display the average
			std::snprintf(str, sizeof(str), "Frame: %.1fms | Update: %.1fms | Draw: %.1fms", m_MSPFAverage, m_MSPUAverage, m_MSPDAverage);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 10, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Time Scale: x%.2f ([1]-, [2]+, [Ctrl+1]Reset)", g_TimerMan.IsOneSimUpdatePerFrame() ? g_TimerMan.GetSimSpeed() : g_TimerMan.GetTimeScale());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 20, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Real to Sim Cap: %.2f ms ([3]-, [4]+, [Ctrl+3]Reset)", g_TimerMan.GetRealToSimCap() * 1000.0F);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 30, str, GUIFont::Left);

			float deltaTime = g_TimerMan.GetDeltaTimeMS();
			std::snprintf(str, sizeof(str), "DeltaTime: %.2f ms ([5]-, [6]+, [Ctrl+5]Reset)", deltaTime);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 40, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Particles: %li", g_MovableMan.GetParticleCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 50, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Objects: %i", g_MovableMan.GetKnownObjectsCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 60, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "MOIDs: %i", g_MovableMan.GetMOIDCount());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 70, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Sim Updates Since Last Drawn: %i", g_TimerMan.SimUpdatesSinceDrawn());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 80, str, GUIFont::Left);

			if (g_TimerMan.IsOneSimUpdatePerFrame()) { g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 90, "ONE Sim Update Per Frame!", GUIFont::Left); }

			int totalPlayingChannelCount;
			int realPlayingChannelCount;
			if (g_AudioMan.GetPlayingChannelCount(&totalPlayingChannelCount, &realPlayingChannelCount)) {
				std::snprintf(str, sizeof(str), "Sound Channels: %d / %d Real | %d / %d Virtual", realPlayingChannelCount, g_AudioMan.GetTotalRealChannelCount(), totalPlayingChannelCount - realPlayingChannelCount, g_AudioMan.GetTotalVirtualChannelCount());
			}
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 100, str, GUIFont::Left);

			// If in split screen mode don't draw graphs because they don't fit anyway.
			if (m_AdvancedPerfStats && g_FrameMan.GetScreenCount() == 1) { DrawPeformanceGraphs(bitmapToDrawTo); }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::DrawPeformanceGraphs(AllegroBitmap &bitmapToDrawTo) {
		CalculateSamplePercentages();

		char str[128];

		for (int pc = 0; pc < PerformanceCounters::PerfCounterCount; ++pc) {
			int blockStart = c_GraphsStartOffsetY + pc * c_GraphBlockHeight;

			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, blockStart, m_PerfCounterNames[pc], GUIFont::Left);

			// Print percentage from PerformanceCounters::SimTotal
			int perc = static_cast<int>((static_cast<float>(GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc))) / static_cast<float>(GetPerformanceCounterAverage(PerformanceCounters::SimTotal)) * 100));
			std::snprintf(str, sizeof(str), "%%: %u", perc);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 60, blockStart, str, GUIFont::Left);

			// Print average processing time in ms
			std::snprintf(str, sizeof(str), "T: %llu", GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 96, blockStart, str, GUIFont::Left);

			int graphStart = blockStart + c_GraphsOffsetX;

			// Draw graph backgrounds
			bitmapToDrawTo.DrawRectangle(c_StatsOffsetX, graphStart, c_MaxSamples, c_GraphHeight, 240, true);
			bitmapToDrawTo.DrawLine(c_StatsOffsetX, graphStart + c_GraphHeight / 2, c_StatsOffsetX - 1 + c_MaxSamples, graphStart + c_GraphHeight / 2, 96);

			// Draw sample dots
			int peak = 0;
			int sample = m_Sample;
			for (int i = 0; i < c_MaxSamples; ++i) {
				// Show microseconds in graphs, assume that 33333 microseconds (one frame of 30 fps) is the highest value on the graph
				int value = std::clamp(static_cast<int>(static_cast<float>(m_PerfData[pc].at(sample)) / (1000000.0F / 30.0F) * 100.0F), 0, 100);
				int dotHeight = static_cast<int>(static_cast<float>(c_GraphHeight) / 100.0F * static_cast<float>(value));

				bitmapToDrawTo.SetPixel(c_StatsOffsetX - 1 + c_MaxSamples - i, graphStart + c_GraphHeight - dotHeight, 13);
				peak = std::clamp(peak, 0, static_cast<int>(m_PerfData[pc].at(sample)));

				if (sample == 0) { sample = c_MaxSamples; }
				sample--;
			}

			// Print peak values
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 130, blockStart, "Peak: " + std::to_string(peak / 1000), GUIFont::Left);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::DrawCurrentPing() const {
		AllegroBitmap allegroBitmap(g_FrameMan.GetBackBuffer8());
		g_FrameMan.GetLargeFont()->DrawAligned(&allegroBitmap, g_FrameMan.GetBackBuffer8()->w - 25, g_FrameMan.GetBackBuffer8()->h - 14, "PING: " + std::to_string(m_CurrentPing), GUIFont::Right);
	}
}