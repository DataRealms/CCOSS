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

	const string ActivityMan::c_ClassName = "ActivityMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::Clear() {
		m_DefaultActivityType = "GATutorial";
		m_DefaultActivityName = "Tutorial Mission";
		m_Activity = 0;
		m_StartActivity = 0;
		m_LastMusicPath = "";
		m_LastMusicPos = 0;
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

		// Stop all music, will be started by the Activity right below
		g_AudioMan.StopMusic();

		// Replace the start activity
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
			m_Activity->SetActivityState(Activity::INERROR);
			return error;
		}

		// Make sure the main menu and console exits and we're in the game when the activity starts
		g_ConsoleMan.SetEnabled(false);
		g_ResumeActivity = true;
		g_InActivity = true;

		g_PostProcessMan.ClearScenePostEffects();
		g_FrameMan.ClearScreenText();

		// Reset the mouse input to the center
		g_UInputMan.SetMouseValueMagnitude(0.05F);

		// Reset the last music position
		m_LastMusicPath = "";
		m_LastMusicPos = 0;

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::StartActivity(string className, string instanceName) {
		const Entity *entity = g_PresetMan.GetEntityPreset(className, instanceName);

		if (entity) {
			Activity *newActivity = dynamic_cast<Activity *>(entity->Clone());
			if (newActivity) {
				return StartActivity(newActivity);
			}
		} else {
			g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + instanceName + " to start! Has it been defined?");
			return -1;
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::PauseActivity(bool pause) {
		if (!m_Activity || (pause && m_Activity->Paused()) || (!pause && !m_Activity->Paused())) {
			return;
		}

		if (m_Activity) {		
			if (pause) {
				// Save the current in-game music position on pause
				m_LastMusicPath = g_AudioMan.GetMusicPath();
				m_LastMusicPos = g_AudioMan.GetMusicPosition();
			} else {
				// Re-start it again where it was on unpause but only if we have a position to actually resume
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

			m_Activity->Pause(pause);
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was " + (pause ? "paused" : "resumed"));
		} else {
			g_ConsoleMan.PrintString("ERROR: No Activity to pause!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::RestartActivity() {
		// If we have a start activity set, then clone it and pass in. (have to clone, or will delete self in StartActivity)
		if (m_StartActivity) {
			return StartActivity(dynamic_cast<Activity *>(m_StartActivity->Clone()));
		} else {
			return StartActivity(m_DefaultActivityType, m_DefaultActivityName);
		}
		// Report that we had to start the default because there wasn't a specified start activity
		return -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::EndActivity() {
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