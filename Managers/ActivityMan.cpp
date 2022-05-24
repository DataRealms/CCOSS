#include "ActivityMan.h"
#include "Activity.h"

#include "ConsoleMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"
#include "MetaMan.h"

#include "GAScripted.h"

#include "EditorActivity.h"
#include "SceneEditor.h"
#include "AreaEditor.h"
#include "GibEditor.h"
#include "ActorEditor.h"
#include "AssemblyEditor.h"

#include "NetworkServer.h"
#include "MultiplayerServerLobby.h"
#include "MultiplayerGame.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::Clear() {
		m_DefaultActivityType = "GATutorial";
		m_DefaultActivityName = "Tutorial Mission";
		m_Activity = nullptr;
		m_StartActivity = nullptr;
		m_InActivity = false;
		m_ActivityNeedsRestart = false;
		m_ActivityNeedsResume = false;
		m_LastMusicPath.clear();
		m_LastMusicPos = 0.0F;
		m_LaunchIntoActivity = false;
		m_LaunchIntoEditor = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::Initialize() {
		if (g_NetworkServer.IsServerModeEnabled()) {
			return SetStartMultiplayerServerOverview();
		} else if (IsSetToLaunchIntoEditor()) {
			// Evaluate LaunchIntoEditor before LaunchIntoActivity so it takes priority when both are set, otherwise it is ignored and editor is never launched.
			return SetStartEditorActivitySetToLaunchInto();
		} else if (IsSetToLaunchIntoActivity()) {
			m_ActivityNeedsRestart = true;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartActivity(Activity *newActivity) {
		RTEAssert(newActivity, "Trying to replace an activity with a null one!");
		m_StartActivity.reset(newActivity);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartTutorialActivity() {
		SetStartActivity(dynamic_cast<Activity *>(g_PresetMan.GetEntityPreset("GATutorial", "Tutorial Mission")->Clone()));
		if (GameActivity * gameActivity = dynamic_cast<GameActivity *>(GetStartActivity())) { gameActivity->SetStartingGold(10000); }
		g_SceneMan.SetSceneToLoad("Tutorial Bunker");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::SetStartEditorActivity(const std::string_view &editorToLaunch) {
		std::unique_ptr<EditorActivity> editorActivityToStart = nullptr;

		if (editorToLaunch == "ActorEditor") {
			editorActivityToStart = std::make_unique<ActorEditor>();
		} else if (editorToLaunch == "GibEditor") {
			editorActivityToStart = std::make_unique<GibEditor>();
		} else if (editorToLaunch == "SceneEditor") {
			editorActivityToStart = std::make_unique<SceneEditor>();
		} else if (editorToLaunch == "AreaEditor") {
			editorActivityToStart = std::make_unique<AreaEditor>();
		} else if (editorToLaunch == "AssemblyEditor") {
			editorActivityToStart = std::make_unique<AssemblyEditor>();
		}
		if (editorActivityToStart) {
			if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }
			g_SceneMan.SetSceneToLoad("Editor Scene");
			editorActivityToStart->Create();
			editorActivityToStart->SetEditorMode(EditorActivity::LOADDIALOG);
			SetStartActivity(editorActivityToStart.release());
			m_ActivityNeedsRestart = true;
		} else {
			RTEAbort("Failed to instantiate the " + std::string(editorToLaunch) + " Activity!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::SetStartEditorActivitySetToLaunchInto() {
		std::array<std::string_view, 5> validEditorNames = { "ActorEditor", "GibEditor", "SceneEditor", "AreaEditor", "AssemblyEditor" };

		if (std::find(validEditorNames.begin(), validEditorNames.end(), m_EditorToLaunch) != validEditorNames.end()) {
			// Force mouse + keyboard with default mapping so we won't need to change manually if player 1 is set to keyboard only or gamepad.
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetPreset(InputScheme::InputPreset::PresetMouseWASDKeys);
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

	bool ActivityMan::SetStartMultiplayerActivity() {
		if (std::unique_ptr<MultiplayerGame> multiplayerGame = std::make_unique<MultiplayerGame>()) {
			if (g_MetaMan.GameInProgress()) { g_MetaMan.EndGame(); }
			g_SceneMan.SetSceneToLoad("Multiplayer Scene");
			multiplayerGame->Create();
			SetStartActivity(multiplayerGame.release());
			m_ActivityNeedsRestart = true;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::SetStartMultiplayerServerOverview() {
		g_NetworkServer.Start();

		if (std::unique_ptr<MultiplayerServerLobby> multiplayerServerLobby = std::make_unique<MultiplayerServerLobby>()) {
			g_UInputMan.SetMultiplayerMode(true);
			g_FrameMan.SetMultiplayerMode(true);
			g_AudioMan.SetMultiplayerMode(true);
			g_AudioMan.SetMasterMuted();
			g_SceneMan.SetSceneToLoad("Multiplayer Scene");

			multiplayerServerLobby->Create();
			multiplayerServerLobby->ClearPlayers(true);
			for (int playerAndTeamNum = Players::PlayerOne; playerAndTeamNum < Players::MaxPlayerCount; ++playerAndTeamNum) {
				multiplayerServerLobby->AddPlayer(playerAndTeamNum, true, playerAndTeamNum, 0);
			}
			SetStartActivity(multiplayerServerLobby.release());
			m_ActivityNeedsRestart = true;
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::StartActivity(Activity *activity) {
		RTEAssert(activity, "Trying to start a null activity!");

		// Stop all music played by the current activity. It will be re-started by the new Activity. 
		g_AudioMan.StopMusic();

		m_StartActivity.reset(activity);
		m_Activity.reset(dynamic_cast<Activity *>(m_StartActivity->Clone()));

		m_Activity->SetupPlayers();
		int error = m_Activity->Start();

		if (error >= 0)
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was successfully started");
		else {
			g_ConsoleMan.PrintString("ERROR: Activity \"" + m_Activity->GetPresetName() + "\" was NOT started due to errors!");
			m_Activity->SetActivityState(Activity::HasError);
			return error;
		}

		// Close the console in case it was open by the player or because of a previous Activity error.
		g_ConsoleMan.SetEnabled(false);

		m_ActivityNeedsResume = true;
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

	int ActivityMan::StartActivity(const std::string &className, const std::string &presetName) {
		if (const Entity *entity = g_PresetMan.GetEntityPreset(className, presetName)) {
			Activity *newActivity = dynamic_cast<Activity *>(entity->Clone());
			if (GameActivity *newActivityAsGameActivity = dynamic_cast<GameActivity *>(newActivity)) {
				newActivityAsGameActivity->SetStartingGold(newActivityAsGameActivity->GetDefaultGoldMedium());
				if (newActivityAsGameActivity->GetStartingGold() <= 0) {
					newActivityAsGameActivity->SetStartingGold(static_cast<int>(newActivityAsGameActivity->GetTeamFunds(0)));
				} else {
					newActivityAsGameActivity->SetTeamFunds(static_cast<float>(newActivityAsGameActivity->GetStartingGold()), 0);
				}
			}
			return StartActivity(newActivity);
		} else {
			g_ConsoleMan.PrintString("ERROR: Couldn't find the " + className + " named " + presetName + " to start! Has it been defined?");
			return -1;
		}
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

	void ActivityMan::ResumeActivity() {
		if (GetActivity()->GetActivityState() != Activity::NotStarted) {
			m_InActivity = true;
			m_ActivityNeedsResume = false;

			g_FrameMan.ClearBackBuffer8();
			g_FrameMan.FlipFrameBuffers();

			PauseActivity(false);
			g_TimerMan.PauseSim(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::RestartActivity() {
		m_ActivityNeedsRestart = false;
		g_ConsoleMan.PrintString("SYSTEM: Activity was reset!");

		g_FrameMan.ClearBackBuffer8();
		g_FrameMan.FlipFrameBuffers();

		g_AudioMan.StopAll();
		g_MovableMan.PurgeAllMOs();
		// Have to reset TimerMan before creating anything else because all timers are reset against it.
		g_TimerMan.ResetTime();

		// TODO: Deal with GUI resetting here!$@#") // Figure out what the hell this is about.

		// Need to pass in a clone of the activity because the original will be deleted and re-set during StartActivity.
		int activityStarted = m_StartActivity ? StartActivity(dynamic_cast<Activity *>(m_StartActivity->Clone())) : StartActivity(m_DefaultActivityType, m_DefaultActivityName);
		g_TimerMan.PauseSim(false);
		if (activityStarted >= 0) {
			m_InActivity = true;
			return true;
		} else {
			m_InActivity = false;
			PauseActivity();
			g_ConsoleMan.SetEnabled(true);
			return false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::EndActivity() const {
		// TODO: Set the activity pointer to nullptr so it doesn't return junk after being destructed. Do it here, or wherever works without crashing.
		if (m_Activity) {
			m_Activity->End();
			g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was ended");
		} else {
			g_ConsoleMan.PrintString("ERROR: No Activity to end!");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::LateUpdateGlobalScripts() const {
		if (GAScripted *scriptedActivity = dynamic_cast<GAScripted *>(m_Activity.get())) { scriptedActivity->UpdateGlobalScripts(true); }
	}
}