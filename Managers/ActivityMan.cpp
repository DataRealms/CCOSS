#include "ActivityMan.h"
#include "Activity.h"

#include "ConsoleMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"
#include "GAScripted.h"

extern bool g_ResumeActivity;
extern bool g_InActivity;

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::Clear() {
		m_DefaultActivityType = "GATutorial";
		m_DefaultActivityName = "Tutorial Mission";
		m_Activity = nullptr;
		m_StartActivity = nullptr;
		m_LastMusicPath = "";
		m_LastMusicPos = 0.0F;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartActivity(Activity *newActivity) {
		RTEAssert(newActivity, "Trying to replace an activity with a null one!");

		delete m_StartActivity;
		m_StartActivity = newActivity;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::StartActivity(Activity *activity) {
		RTEAssert(activity, "Trying to start a null activity!");

		int error = 0;

		// Stop all music played by the current activity. It will be re-started by the new Activity. 
		g_AudioMan.StopMusic();

		delete m_StartActivity;
		m_StartActivity = activity;

		delete m_Activity;
		m_Activity = dynamic_cast<Activity *>(m_StartActivity->Clone());

		m_Activity->SetupPlayers();
		error = m_Activity->Start();

		if (error >= 0)
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was successfully started");
		else {
			g_ConsoleMan.PrintString("ERROR: Activity \"" + m_Activity->GetPresetName() + "\" was NOT started due to errors!");
			m_Activity->SetActivityState(Activity::HasError);
			return error;
		}

		// Close the console in case it was open by the player or because of a previous Activity error.
		g_ConsoleMan.SetEnabled(false);

		g_ResumeActivity = true;
		g_InActivity = true;

		g_PostProcessMan.ClearScenePostEffects();
		g_FrameMan.ClearScreenText();

		// Reset the mouse input to the center
		g_UInputMan.SetMouseValueMagnitude(0.05F);

		m_LastMusicPath = "";
		m_LastMusicPos = 0;
		g_AudioMan.PauseAllMobileSounds(false);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::StartActivity(string className, string presetName) {
		const Entity *entity = g_PresetMan.GetEntityPreset(className, presetName);

		if (entity) {
			Activity *newActivity = dynamic_cast<Activity *>(entity->Clone());
			return StartActivity(newActivity);
		} else {
			g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + presetName + " to start! Has it been defined?");
			return -1;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::PauseActivity(bool pause) {
		if (!m_Activity || (pause && m_Activity->IsPaused()) || (!pause && !m_Activity->IsPaused())) {
			return;
		}

		if (m_Activity) {		
			if (pause) {
				m_LastMusicPath = g_AudioMan.GetMusicPath();
				m_LastMusicPos = g_AudioMan.GetMusicPosition();
			} else {
				if (!m_LastMusicPath.empty() && m_LastMusicPos > 0) {
					g_AudioMan.ClearMusicQueue();
					g_AudioMan.PlayMusic(m_LastMusicPath.c_str());
					g_AudioMan.SetMusicPosition(m_LastMusicPos);
					g_AudioMan.QueueSilence(30);
					g_AudioMan.QueueMusicStream("Base.rte/Music/Watts/Last Man.ogg");
					g_AudioMan.QueueSilence(30);
					g_AudioMan.QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");
				}
			}

			m_Activity->SetPaused(pause);
			g_InActivity = !pause;
			g_AudioMan.PauseAllMobileSounds(pause);
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was " + (pause ? "paused" : "resumed"));
		} else {
			g_ConsoleMan.PrintString("ERROR: No Activity to pause!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::RestartActivity() {
		if (m_StartActivity) {
			// Need to pass in a clone of the activity because the original will be deleted and re-set during StartActivity.
			return StartActivity(dynamic_cast<Activity *>(m_StartActivity->Clone()));
		}
		return StartActivity(m_DefaultActivityType, m_DefaultActivityName);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::EndActivity() {
		// TODO: Set the activity pointer to nullptr so it doesn't return junk after being destructed. Do it here, or wherever works without crashing.
		if (m_Activity) {
			m_Activity->End();
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was ended");
		} else {
			g_ConsoleMan.PrintString("ERROR: No Activity to end!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::LateUpdateGlobalScripts() {
		GAScripted *scriptedActivity = dynamic_cast<GAScripted *>(m_Activity);
		if (scriptedActivity) { scriptedActivity->UpdateGlobalScripts(true); }
	}
}