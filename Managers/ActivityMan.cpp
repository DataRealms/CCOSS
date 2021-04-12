#include "ActivityMan.h"
#include "Activity.h"

#include "ConsoleMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"

#include "GAScripted.h"

#include "EditorActivity.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"

#include "MultiplayerServerLobby.h"

extern bool g_ResumeActivity;

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::Clear() {
		m_DefaultActivityType = "GATutorial";
		m_DefaultActivityName = "Tutorial Mission";
		m_Activity = nullptr;
		m_StartActivity = nullptr;
		m_InActivity = false;
		m_LastMusicPath.clear();
		m_LastMusicPos = 0.0F;
		m_LaunchIntoActivity = false;
		m_LaunchIntoEditor = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartActivity(Activity *newActivity) {
		RTEAssert(newActivity, "Trying to replace an activity with a null one!");

		delete m_StartActivity;
		m_StartActivity = newActivity;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartEditorActivity(const std::string_view &editorToLaunch) const {
		EditorActivity *editorActivityToStart = nullptr;

		if (editorToLaunch == "ActorEditor") {
			editorActivityToStart = new ActorEditor;
		} else if (editorToLaunch == "GibEditor") {
			editorActivityToStart = new GibEditor;
		} else if (editorToLaunch == "SceneEditor") {
			editorActivityToStart = new SceneEditor;
		} else if (editorToLaunch == "AreaEditor") {
			editorActivityToStart = new AreaEditor;
		} else if (editorToLaunch == "AssemblyEditor") {
			editorActivityToStart = new AssemblyEditor;
		}
		if (editorActivityToStart) {
			g_SceneMan.SetSceneToLoad("Editor Scene");
			editorActivityToStart->Create();
			editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
			g_ActivityMan.SetStartActivity(editorActivityToStart);
		} else {
			RTEAbort("Failed to instantiate the " + std::string(editorToLaunch) + " Activity!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::SetStartEditorActivitySetToLaunchInto() {
		bool validEditorName = false;
		std::array<std::string_view, 5> validEditorNames = { "ActorEditor", "GibEditor", "SceneEditor", "AreaEditor", "AssemblyEditor" };
		if (std::find(validEditorNames.begin(), validEditorNames.end(), m_EditorToLaunch) != validEditorNames.end()) { validEditorName = true; }

		if (validEditorName) {
			// Force mouse + keyboard with default mapping so we won't need to change manually if player 1 is set to keyboard only or gamepad.
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetPreset(InputPreset::PRESET_WASDKEYS);
			SetStartEditorActivity(m_EditorToLaunch);
			return true;
		} else {
			g_ConsoleMan.PrintString("ERROR: Invalid editor name passed into \"-editor\" argument!");
			g_ConsoleMan.SetEnabled(true);
			m_LaunchIntoEditor = false;
			return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartMultiplayerServerOverview() const {
		g_SceneMan.SetSceneToLoad("Multiplayer Scene");
		MultiplayerServerLobby *multiplayerServerLobby = new MultiplayerServerLobby;
		multiplayerServerLobby->Create();
		multiplayerServerLobby->ClearPlayers(true);
		for (int playerAndTeamNum = Players::PlayerOne; playerAndTeamNum < Players::MaxPlayerCount; ++playerAndTeamNum) {
			multiplayerServerLobby->AddPlayer(playerAndTeamNum, true, playerAndTeamNum, 0);
		}
		g_ActivityMan.SetStartActivity(multiplayerServerLobby);
		g_ActivityMan.SetResetActivity(true);
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
		m_InActivity = true;

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
			m_InActivity = !pause;
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