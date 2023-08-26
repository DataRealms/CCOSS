#include "ActivityMan.h"
#include "Activity.h"

#include "CameraMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "AudioMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"
#include "MetaMan.h"

#include "GAScripted.h"
#include "SLTerrain.h"

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
		m_ActiveSavingThreadCount = 0;
		m_IsLoading = false;
		m_InActivity = false;
		m_ActivityNeedsRestart = false;
		m_ActivityNeedsResume = false;
		m_ResumingActivityFromPauseMenu = false;
		m_SkipPauseMenuWhenPausingActivity = false;
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

	bool ActivityMan::SaveCurrentGame(const std::string &fileName) {
		if (IsSaving() || m_IsLoading) {
			RTEError::ShowMessageBox("Cannot Save Game\nA game is currently being saved/loaded, try again shortly.");
			return false;
		}

		Scene *scene = g_SceneMan.GetScene();
		GAScripted *activity = dynamic_cast<GAScripted *>(GetActivity());

		if (!scene || !activity || (activity && activity->GetActivityState() == Activity::ActivityState::Over)) {
			g_ConsoleMan.PrintString("ERROR: Cannot save when there's no game running, or the game is finished!");
			return false;
		}

		// TODO, save to a zip instead of a directory
		std::filesystem::create_directory(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + fileName);

		if (scene->SaveData(c_UserScriptedSavesModuleName + "/" + fileName + "/Save") < 0) {
			// This print is actually pointless because game will abort if it fails to save layer bitmaps. It stays here for now because in reality the game doesn't properly abort if the layer bitmaps fail to save. It is what it is.
			g_ConsoleMan.PrintString("ERROR: Failed to save scene bitmaps while saving!");
			return false;
		}

		IncrementSavingThreadCount();

		// We need a copy of our scene, because we have to do some fixup to remove PLACEONLOAD items and only keep the current MovableMan state.
		std::unique_ptr<Scene> modifiableScene(dynamic_cast<Scene*>(scene->Clone()));

		// Delete any existing objects from our scene - we don't want to replace broken doors or repair any stuff when we load.
		modifiableScene->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, true);

		// Become our own original preset, instead of being a copy of the Scene we got cloned from, so we don't still pick up the PlacedObjectSets from our parent when loading.
		modifiableScene->SetPresetName(fileName);
		modifiableScene->MigrateToModule(g_PresetMan.GetModuleID(c_UserScriptedSavesModuleName));
		modifiableScene->SetSavedGameInternal(true);

		// Make sure the terrain is also treated as an original preset, otherwise it will screw up if we save then load then save again, since it'll try to be a CopyOf of itself.
		modifiableScene->GetTerrain()->SetPresetName(fileName);
		modifiableScene->GetTerrain()->MigrateToModule(g_PresetMan.GetModuleID(c_UserScriptedSavesModuleName));

		// Block the main thread for a bit to let the Writer access the relevant data.
		std::unique_ptr<Writer> writer(std::make_unique<Writer>(g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + fileName + "/Save.ini"));
		writer->NewPropertyWithValue("Activity", activity);

		// Pull all stuff from MovableMan into the Scene for saving, so existing Actors/ADoors are saved, without transferring ownership, so the game can continue.
		// This is done after the activity is saved, in case the activity wants to add anything to the scene while saving.
		modifiableScene->RetrieveSceneObjects(false);
		for (SceneObject *objectToSave : *modifiableScene->GetPlacedObjects(Scene::PlacedObjectSets::PLACEONLOAD)) {
			if (MovableObject *objectToSaveAsMovableObject = dynamic_cast<MovableObject *>(objectToSave)) {
				objectToSaveAsMovableObject->OnGameSave();
			}
		}

		writer->NewPropertyWithValue("OriginalScenePresetName", scene->GetPresetName());
		writer->NewPropertyWithValue("PlaceObjectsIfSceneIsRestarted", g_SceneMan.GetPlaceObjectsOnLoad());
		writer->NewPropertyWithValue("PlaceUnitsIfSceneIsRestarted", g_SceneMan.GetPlaceUnitsOnLoad());
		writer->NewPropertyWithValue("Scene", modifiableScene.get());

		auto saveWriterData = [this](std::unique_ptr<Writer> writerToSave) {
			// Explicitly flush to disk. This'll happen anyways at the end of this scope, but otherwise this lambda looks rather empty :)
			writerToSave->EndWrite();
			DecrementSavingThreadCount();
		};

		// Make a thread to flush the data to the disk, and detach it so it can run concurrently with the game simulation.
		std::thread saveThread(saveWriterData, std::move(writer));
		saveThread.detach();

		// We didn't transfer ownership, so we must be very careful that sceneAltered's deletion doesn't touch the stuff we got from MovableMan.
		modifiableScene->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, false);

		g_ConsoleMan.PrintString("SYSTEM: Game saved to \"" + fileName + "\"!");
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::LoadAndLaunchGame(const std::string &fileName) {
		if (IsSaving() || m_IsLoading) {
			RTEError::ShowMessageBox("Cannot Load Game\nA game is currently being saved/loaded, try again shortly.");
			return false;
		}

		std::string saveFilePath = g_PresetMan.GetFullModulePath(c_UserScriptedSavesModuleName) + "/" + fileName + "/Save.ini";

		if (!std::filesystem::exists(saveFilePath)) {
			RTEError::ShowMessageBox("Game loading failed! Make sure you have a saved game called \"" + fileName + "\"");
			return false;
		}

		Reader reader(saveFilePath, true, nullptr, false);
		m_IsLoading = true;

		std::unique_ptr<Scene> scene(std::make_unique<Scene>());
		std::unique_ptr<GAScripted> activity(std::make_unique<GAScripted>());

		std::string originalScenePresetName = fileName;
		bool placeObjectsIfSceneIsRestarted = true;
		bool placeUnitsIfSceneIsRestarted = true;
		while (reader.NextProperty()) {
			std::string propName = reader.ReadPropName();
			if (propName == "Activity") {
				reader >> activity.get();
			} else if (propName == "OriginalScenePresetName") {
				reader >> originalScenePresetName;
			} else if (propName == "PlaceObjectsIfSceneIsRestarted") {
				reader >> placeObjectsIfSceneIsRestarted;
			} else if (propName == "PlaceUnitsIfSceneIsRestarted") {
				reader >> placeUnitsIfSceneIsRestarted;
			} else if (propName == "Scene") {
				reader >> scene.get();
			}
		}

		// SetSceneToLoad() doesn't Clone(), but when the Activity starts, it will eventually call LoadScene(), which does a Clone() of scene internally.
		g_SceneMan.SetSceneToLoad(scene.get(), true, true);
		// Saved Scenes get their presetname set to their filename to ensure they're separate from the preset Scene they're based off of.
		// However, saving a game you've already saved will end up with its OriginalScenePresetName set to the filename, which will screw up restarting the Activity, so we set its PresetName here.
		scene->SetPresetName(originalScenePresetName);
		// For starting Activity, we need to directly clone the Activity we want to start.
		StartActivity(dynamic_cast<GAScripted*>(activity->Clone()));
		// When this method exits, our Scene object will be destroyed, which will cause problems if you try to restart it. To avoid this, set the Scene to load to the preset object with the same name.
		g_SceneMan.SetSceneToLoad(originalScenePresetName, placeObjectsIfSceneIsRestarted, placeUnitsIfSceneIsRestarted);

		g_ConsoleMan.PrintString("SYSTEM: Game \"" + fileName + "\" loaded!");

		m_IsLoading = false;
		return true;
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
		g_AudioMan.PauseIngameSounds(false);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ActivityMan::StartActivity(const std::string &className, const std::string &presetName) {
		if (const Entity *entity = g_PresetMan.GetEntityPreset(className, presetName)) {
			Activity *newActivity = dynamic_cast<Activity *>(entity->Clone());
			if (GameActivity *newActivityAsGameActivity = dynamic_cast<GameActivity *>(newActivity)) {
				newActivityAsGameActivity->SetStartingGold(newActivityAsGameActivity->GetDefaultGoldMediumDifficulty());
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

	void ActivityMan::PauseActivity(bool pause, bool skipPauseMenu) {
		if (!m_Activity) {
			g_ConsoleMan.PrintString("ERROR: No Activity to pause!");
			return;
		}

		if (pause == m_Activity->IsPaused()) {
			return;
		}

		if (pause) {
			m_LastMusicPath = g_AudioMan.GetMusicPath();
			m_LastMusicPos = g_AudioMan.GetMusicPosition();
		} else {
			if (!m_ResumingActivityFromPauseMenu && (!m_LastMusicPath.empty() && m_LastMusicPos > 0)) {
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
		m_ResumingActivityFromPauseMenu = false;
		m_SkipPauseMenuWhenPausingActivity = skipPauseMenu;
		g_AudioMan.PauseIngameSounds(pause);
		g_ConsoleMan.PrintString("SYSTEM: Activity \"" + m_Activity->GetPresetName() + "\" was " + (pause ? "paused" : "resumed"));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ActivityMan::ResumeActivity() {
		if (GetActivity()->GetActivityState() != Activity::NotStarted) {
			m_InActivity = true;
			m_ActivityNeedsResume = false;

			PauseActivity(false);
			g_TimerMan.PauseSim(false);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ActivityMan::RestartActivity() {
		m_ActivityNeedsRestart = false;
		g_ConsoleMan.PrintString("SYSTEM: Activity was reset!");

		g_AudioMan.StopAll();
		g_MovableMan.PurgeAllMOs();
		// Have to reset TimerMan before creating anything else because all timers are reset against it.
		g_TimerMan.ResetTime();

		// TODO: Deal with GUI resetting here!$@#") // Figure out what the hell this is about.

		int activityStarted;
		if (m_StartActivity) {
			// Need to pass in a clone of the activity because the original will be deleted and re-set during StartActivity.
			Activity *startActivityToUse = dynamic_cast<Activity *>(m_StartActivity->Clone());
			startActivityToUse->SetActivityState(Activity::ActivityState::NotStarted);
			activityStarted = StartActivity(startActivityToUse);
		} else {
			activityStarted = StartActivity(m_DefaultActivityType, m_DefaultActivityName);
		}
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
