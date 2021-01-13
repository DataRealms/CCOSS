#include "PerformanceMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "AudioMan.h"
#include "Timer.h"

#include "GUI.h"
#include "AllegroBitmap.h"

namespace RTE {

	const std::string PerformanceMan::c_ClassName = "PerformanceMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Clear() {
		m_ShowPerfStats = false;
		m_AdvancedPerfStats = true;
		m_CurrentPing = 0;
		m_FrameTimer = 0;
		m_MSPFs.clear();
		m_MSPFAverage = 0;
		m_SimSpeed = 1.0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PerformanceMan::Create() {
		m_FrameTimer = new Timer();
		m_Sample = 0;

		for (unsigned short counter = 0; counter < PERF_COUNT; ++counter) {
			std::fill_n(m_PerfData[counter], c_MaxSamples, 0);
			std::fill_n(m_PerfPercentages[counter], c_MaxSamples, 0);
			m_PerfMeasureStart[counter] = 0;
			m_PerfMeasureStop[counter] = 0;
		}

		// Set up performance counter's names
		m_PerfCounterNames[PERF_SIM_TOTAL] = "Total";
		m_PerfCounterNames[PERF_ACTORS_PASS1] = "Act Travel";
		m_PerfCounterNames[PERF_PARTICLES_PASS1] = "Prt Travel";
		m_PerfCounterNames[PERF_ACTORS_PASS2] = "Act Update";
		m_PerfCounterNames[PERF_PARTICLES_PASS2] = "Prt Update";
		m_PerfCounterNames[PERF_ACTORS_AI] = "Act AI";
		m_PerfCounterNames[PERF_ACTIVITY] = "Activity";

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Destroy() {
		delete m_FrameTimer;
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Update() {
		// TODO: Figure out how doing this here and then in Draw() evens out the fps values and see if we can get same results without this.
		// Time and store the milliseconds per frame reading to the buffer, and trim the buffer as needed
		m_MSPFs.push_back(m_FrameTimer->GetElapsedRealTimeMS());
		while (m_MSPFs.size() > c_MSPFAverageSampleSize) {
			m_MSPFs.pop_front();
		}

		// Calculate the average milliseconds per frame over the last sampleSize frames
		m_MSPFAverage = 0;
		for (const unsigned int &mspf : m_MSPFs) {
			m_MSPFAverage += mspf;
		}
		m_MSPFAverage /= m_MSPFs.size();

		float pitch = 1.0F;

		// Update the SimSpeed; if set to do one sim update per frame, adjust global sound pitch to match the ratio of sim time over real time.
		// TODO: This belongs in TimerMan so figure out where exactly to shove it in it.
		if (g_TimerMan.IsOneSimUpdatePerFrame()) {
			m_SimSpeed = g_TimerMan.GetDeltaTimeMS() / static_cast<float>(m_MSPFAverage);

			// TODO: This should be built into the SimSpeed setter (which again, should be in TimerMan) so you can't screw it up.
			if (g_TimerMan.IsSimSpeedLimited() && m_SimSpeed > 1.0F) { m_SimSpeed = 1.0F; }

			// Soften the ratio of the pitch adjustment so it's not such an extreme effect on the audio
			// TODO: This coefficient should probably move to SettingsMan and be loaded from ini. That way this effect can be lessened or even turned off entirely by users. 0.35 is a good default value though.
			pitch = m_SimSpeed + (1.0F - m_SimSpeed) * 0.35F;
		} else {
			m_SimSpeed = 1.0F;
		}
		g_AudioMan.SetGlobalPitch(pitch);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::ResetFrameTimer() { m_FrameTimer->Reset(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StartPerformanceMeasurement(PerformanceCounters counter) { m_PerfMeasureStart[counter] = g_TimerMan.GetAbsoluteTime(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::StopPerformanceMeasurement(PerformanceCounters counter) {
		m_PerfMeasureStop[counter] = g_TimerMan.GetAbsoluteTime();
		AddPerformanceSample(counter, m_PerfMeasureStop[counter] - m_PerfMeasureStart[counter]);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::NewPerformanceSample() {
		m_Sample++;
		if (m_Sample >= c_MaxSamples) { m_Sample = 0; }

		for (unsigned short counter = 0; counter < PERF_COUNT; ++counter) {
			m_PerfData[counter][m_Sample] = 0;
			m_PerfPercentages[counter][m_Sample] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::CalculateSamplePercentages() {
		for (unsigned short counter = 0; counter < PERF_COUNT; ++counter) {
			unsigned short samplePercentage = static_cast<unsigned short>(static_cast<float>(m_PerfData[counter][m_Sample]) / static_cast<float>(m_PerfData[counter][PERF_SIM_TOTAL]) * 100);
			m_PerfPercentages[counter][m_Sample] = samplePercentage;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned long long PerformanceMan::GetPerformanceCounterAverage(PerformanceCounters counter) const {
		unsigned long long totalPerformanceMeasurement = 0;
		unsigned short sample = m_Sample;
		for (unsigned short i = 0; i < c_Average; ++i) {
			totalPerformanceMeasurement += m_PerfData[counter][sample];
			if (sample == 0) { sample = c_MaxSamples; }
			sample--;
		}
		return totalPerformanceMeasurement / c_Average;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::Draw(AllegroBitmap bitmapToDrawTo) {
		if (m_ShowPerfStats) {
			// Time and store the milliseconds per frame reading to the buffer, and trim the buffer as needed
			m_MSPFs.push_back(m_FrameTimer->GetElapsedRealTimeMS());
			m_FrameTimer->Reset();
			while (m_MSPFs.size() > c_MSPFAverageSampleSize) {
				m_MSPFs.pop_front();
			}
			// Calculate the average milliseconds per frame over the last sampleSize frames
			for(const unsigned int &mspf : m_MSPFs){
				m_MSPFAverage += mspf;
			}
			m_MSPFAverage /= m_MSPFs.size();

			char str[512];

			// Calculate the fps from the average
			float fps = 1.0F / (static_cast<float>(m_MSPFAverage) / 1000.0F);
			std::snprintf(str, sizeof(str), "FPS: %.0f", fps);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight, str, GUIFont::Left);

			// Display the average
			std::snprintf(str, sizeof(str), "MSPF: %zi", m_MSPFAverage);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 10, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Time Scale: x%.2f ([1]-, [2]+)", g_TimerMan.IsOneSimUpdatePerFrame() ? m_SimSpeed : g_TimerMan.GetTimeScale());
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 20, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Real to Sim Cap: %.2f ms ([3]-, [4]+)", g_TimerMan.GetRealToSimCap() * 1000.0F);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 30, str, GUIFont::Left);

			float deltaTime = g_TimerMan.GetDeltaTimeMS();
			std::snprintf(str, sizeof(str), "DeltaTime: %.2f ms ([5]-, [6]+)", deltaTime);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, c_StatsHeight + 40, str, GUIFont::Left);

			std::snprintf(str, sizeof(str), "Particles: %i", g_MovableMan.GetParticleCount());
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

	void PerformanceMan::DrawPeformanceGraphs(AllegroBitmap bitmapToDrawTo) {
		CalculateSamplePercentages();

		char str[512];

		for (unsigned short pc = 0; pc < PERF_COUNT; ++pc) {
			unsigned short blockStart = c_GraphsStartOffsetY + pc * c_GraphBlockHeight;

			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX, blockStart, m_PerfCounterNames[pc], GUIFont::Left);

			// Print percentage from PerformanceCounters::PERF_SIM_TOTAL
			unsigned short perc = static_cast<unsigned short>((static_cast<float>(GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc))) / static_cast<float>(GetPerformanceCounterAverage(PERF_SIM_TOTAL)) * 100));
			std::snprintf(str, sizeof(str), "%%: %u", perc);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 60, blockStart, str, GUIFont::Left);

			// Print average processing time in ms
			std::snprintf(str, sizeof(str), "T: %lli", GetPerformanceCounterAverage(static_cast<PerformanceCounters>(pc)) / 1000);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 96, blockStart, str, GUIFont::Left);

			unsigned short graphStart = blockStart + c_GraphsOffsetX;

			// Draw graph backgrounds
			bitmapToDrawTo.DrawRectangle(c_StatsOffsetX, graphStart, c_MaxSamples, c_GraphHeight, 240, true);
			bitmapToDrawTo.DrawLine(c_StatsOffsetX, graphStart + c_GraphHeight / 2, c_StatsOffsetX - 1 + c_MaxSamples, graphStart + c_GraphHeight / 2, 96);

			// Draw sample dots
			unsigned short peak = 0;
			unsigned short sample = m_Sample;
			for (unsigned short i = 0; i < c_MaxSamples; i++) {
				// Show microseconds in graphs, assume that 33333 microseconds (one frame of 30 fps) is the highest value on the graph
				unsigned short value = Limit(static_cast<unsigned short>(static_cast<float>(m_PerfData[pc][sample]) / (1000000 / 30) * 100), 100, 0);
				unsigned short dotHeight = static_cast<unsigned short>(static_cast<float>(c_GraphHeight) / 100.0 * static_cast<float>(value));

				bitmapToDrawTo.SetPixel(c_StatsOffsetX - 1 + c_MaxSamples - i, graphStart + c_GraphHeight - dotHeight, 13);
				peak = Limit(peak, m_PerfData[pc][sample], 0);

				if (sample == 0) { sample = c_MaxSamples; }
				sample--;
			}

			// Print peak values
			std::snprintf(str, sizeof(str), "Peak: %i", peak / 1000);
			g_FrameMan.GetLargeFont()->DrawAligned(&bitmapToDrawTo, c_StatsOffsetX + 130, blockStart, str, GUIFont::Left);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PerformanceMan::DrawCurrentPing() {
		AllegroBitmap allegroBitmap(g_FrameMan.GetBackBuffer8());
		char buf[32];
		std::snprintf(buf, sizeof(buf), "PING: %u", m_CurrentPing);
		g_FrameMan.GetLargeFont()->DrawAligned(&allegroBitmap, g_FrameMan.GetBackBuffer8()->w - 25, g_FrameMan.GetBackBuffer8()->h - 14, buf, GUIFont::Right);
	}
}