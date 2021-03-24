/*          ______   ______   ______  ______  ______  __  __       ______   ______   __    __   __    __   ______   __   __   _____
           /\  ___\ /\  __ \ /\  == \/\__  _\/\  ___\/\_\_\_\     /\  ___\ /\  __ \ /\ "-./  \ /\ "-./  \ /\  __ \ /\ "-.\ \ /\  __-.
           \ \ \____\ \ \/\ \\ \  __<\/_/\ \/\ \  __\\/_/\_\/_    \ \ \____\ \ \/\ \\ \ \-./\ \\ \ \-./\ \\ \  __ \\ \ \-.  \\ \ \/\ \
            \ \_____\\ \_____\\ \_\ \_\ \ \_\ \ \_____\/\_\/\_\    \ \_____\\ \_____\\ \_\ \ \_\\ \_\ \ \_\\ \_\ \_\\ \_\\"\_\\ \____-
  	         \/_____/ \/_____/ \/_/ /_/  \/_/  \/_____/\/_/\/_/     \/_____/ \/_____/ \/_/  \/_/ \/_/  \/_/ \/_/\/_/ \/_/ \/_/ \/____/
   ______   ______   __    __   __    __   __  __   __   __   __   ______  __  __       ______  ______   ______      __   ______   ______   ______
  /\  ___\ /\  __ \ /\ "-./  \ /\ "-./  \ /\ \/\ \ /\ "-.\ \ /\ \ /\__  _\/\ \_\ \     /\  == \/\  == \ /\  __ \    /\ \ /\  ___\ /\  ___\ /\__  _\
  \ \ \____\ \ \/\ \\ \ \-./\ \\ \ \-./\ \\ \ \_\ \\ \ \-.  \\ \ \\/_/\ \/\ \____ \    \ \  _-/\ \  __< \ \ \/\ \  _\_\ \\ \  __\ \ \ \____\/_/\ \/
   \ \_____\\ \_____\\ \_\ \ \_\\ \_\ \ \_\\ \_____\\ \_\\"\_\\ \_\  \ \_\ \/\_____\    \ \_\   \ \_\ \_\\ \_____\/\_____\\ \_____\\ \_____\  \ \_\
    \/_____/ \/_____/ \/_/  \/_/ \/_/  \/_/ \/_____/ \/_/ \/_/ \/_/   \/_/  \/_____/     \/_/    \/_/ /_/ \/_____/\/_____/ \/_____/ \/_____/   \/_/

/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\/////\\\\\*/

/// <summary>
/// Main driver implementation of the Retro Terrain Engine.
/// Data Realms, LLC - http://www.datarealms.com
/// Cortex Command Center - https://discord.gg/SdNnKJN
/// Cortex Command Community Project - https://github.com/cortex-command-community
/// </summary>

#include "MetaMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PerformanceMan.h"
#include "PrimitiveMan.h"
#include "UInputMan.h"
#include "MenuMan.h"

#include "LoadingScreen.h"
#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "TitleScreen.h"

#include "DataModule.h"

#include "MultiplayerServerLobby.h"
#include "NetworkServer.h"

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Global variables.
/// </summary>


volatile bool g_Quit = false;
bool g_ResetRTE = false; //!< Signals to reset the entire RTE next iteration.
bool g_LaunchIntoEditor = false; //!< Flag for launching directly into editor activity.
std::string g_EditorToLaunch = ""; //!< String with editor activity name to launch.
bool g_InActivity = false;
bool g_ResetActivity = false;
bool g_ResumeActivity = false;
int g_StationOffsetX;
int g_StationOffsetY;

bool g_HadResolutionChange = false; //!< Need this so we can restart PlayIntroTitle without an endless loop or leaks. Will be set true by ReinitMainMenu and set back to false at the end of the switch.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This handles when the quit or exit button is pressed on the window.
/// </summary>
void QuitHandler(void) { g_Quit = true; }
END_OF_FUNCTION(QuitHandler)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Reset the current activity.
/// </summary>
/// <returns></returns>
bool ResetActivity() {
	g_ConsoleMan.PrintString("SYSTEM: Activity was reset!");
    g_ResetActivity = false;

    // Clear and reset out things
    g_FrameMan.ClearBackBuffer8();
    g_FrameMan.FlipFrameBuffers();
    g_AudioMan.StopAll();

    // Quit if we should
	if (g_Quit) {
		return false;
	}

	// TODO: Deal with GUI resetting here!$@#")
    // Clear out all MO's
    g_MovableMan.PurgeAllMOs();
    // Have to reset TimerMan before creating anything else because all timers are reset against it
    g_TimerMan.ResetTime();

    g_FrameMan.LoadPalette("Base.rte/palette.bmp");
    g_FrameMan.FlipFrameBuffers();

    // Reset TimerMan again after loading so there's no residual delay
    g_TimerMan.ResetTime();
    // Unpause
    g_TimerMan.PauseSim(false);

    int error = g_ActivityMan.RestartActivity();
	if (error >= 0) {
		g_InActivity = true;
	} else {
        // Something went wrong when restarting, so drop out to scenario menu and open the console to show the error messages
		g_InActivity = false;
		g_ActivityMan.PauseActivity();
		g_ConsoleMan.SetEnabled(true);
		//g_IntroState = MAINTOSCENARIO;
		return false;
	}
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Start the simulation back up after being paused.
/// </summary>
void ResumeActivity() {
	if (g_ActivityMan.GetActivity()->GetActivityState() != Activity::NotStarted) {
		g_Quit = false;
		g_InActivity = true;
		g_ResumeActivity = false;

		g_FrameMan.ClearBackBuffer8();
		g_FrameMan.FlipFrameBuffers();
		g_FrameMan.LoadPalette("Base.rte/palette.bmp");

		g_PerformanceMan.ResetFrameTimer();
		g_TimerMan.PauseSim(false);
		g_ActivityMan.PauseActivity(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Launch multiplayer lobby activity.
/// </summary>
void EnterMultiplayerLobby() {
	g_SceneMan.SetSceneToLoad("Multiplayer Scene");
	MultiplayerServerLobby *pMultiplayerServerLobby = new MultiplayerServerLobby;
	pMultiplayerServerLobby->Create();

	pMultiplayerServerLobby->ClearPlayers(true);
	pMultiplayerServerLobby->AddPlayer(0, true, 0, 0);
	pMultiplayerServerLobby->AddPlayer(1, true, 0, 1);
	pMultiplayerServerLobby->AddPlayer(2, true, 0, 2);
	pMultiplayerServerLobby->AddPlayer(3, true, 0, 3);

	//g_FrameMan.ResetSplitScreens(true, true);
	g_ActivityMan.SetStartActivity(pMultiplayerServerLobby);
	g_ResetActivity = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Launch editor activity specified in command-line argument.
/// </summary>
/// <returns>Whether a valid editor name was passed in and set to be launched.</returns>
bool EnterEditorActivity(const std::string &editorToEnter) {
	/*
	if (editorToEnter == "ActorEditor") {
		g_pMainMenuGUI->StartActorEditor();
	} else if (editorToEnter == "GibEditor") {
		g_pMainMenuGUI->StartGibEditor();
	} else if (editorToEnter == "SceneEditor") {
		g_pMainMenuGUI->StartSceneEditor();
	} else if (editorToEnter == "AreaEditor") {
		g_pMainMenuGUI->StartAreaEditor();
	} else if (editorToEnter == "AssemblyEditor") {
		g_pMainMenuGUI->StartAssemblyEditor();
	} else {
		g_ConsoleMan.PrintString("ERROR: Invalid editor name passed into \"-editor\" argument!");
		g_ConsoleMan.SetEnabled(true);
		g_LaunchIntoEditor = false;
		return false;
	}
	*/
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Load and display the into, title and menu sequence.
/// </summary>
/// <returns></returns>
bool PlayIntroTitle() {
	// Untrap the mouse and keyboard
	g_UInputMan.DisableKeys(false);
	g_UInputMan.TrapMousePos(false);

	// Don't stop the music if reiniting after a resolution change
	if (!g_FrameMan.ResolutionChanged()) { g_AudioMan.StopAll(); }

	//g_FrameMan.ClearBackBuffer32();
	//g_FrameMan.FlipFrameBuffers();

	while (!g_Quit && /*g_IntroState != END &&*/ !g_ResumeActivity) {
		g_MenuMan.Update();

		if (g_NetworkServer.IsServerModeEnabled()) { g_NetworkServer.Update(); }

		g_MenuMan.Draw();

		////////////////////////////////
		// Additional user input and skipping handling
		/*
		if (g_IntroState >= FADEIN && g_IntroState <= SHOWSLIDE8 && keyPressed) {
			g_IntroState = MENUAPPEAR;
			sectionSwitch = true;

			scrollOffset.m_Y = preMenuYOffset;
			orbitRotation = c_HalfPI - c_EighthPI;

			orbitRotation = -c_PI * 1.20;
		}
		*/
	}

	if (g_FrameMan.ResolutionChanged()) { PlayIntroTitle(); }

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Orders to reset the entire Retro Terrain Engine system next iteration.
/// </summary>
void ResetRTE() { g_ResetRTE = true; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Indicates whether the system is about to be reset before the next loop starts.
/// </summary>
/// <returns>Whether the RTE is about to reset next iteration of the loop or not.</returns>
bool IsResettingRTE() { return g_ResetRTE; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Game simulation loop.
/// </summary>
bool RunGameLoop() {
	if (g_Quit) {
		return true;
	}
	g_PerformanceMan.ResetFrameTimer();
	g_TimerMan.PauseSim(false);

	if (g_ResetActivity) { ResetActivity(); }

	while (!g_Quit) {
		// Need to clear this out; sometimes background layers don't cover the whole back
		g_FrameMan.ClearBackBuffer8();

		// Update the real time measurement and increment
		g_TimerMan.Update();

		bool serverUpdated = false;

		// Simulation update, as many times as the fixed update step allows in the span since last frame draw
		while (g_TimerMan.TimeForSimUpdate()) {
			serverUpdated = false;
			g_PerformanceMan.NewPerformanceSample();

			// Advance the simulation time by the fixed amount
			g_TimerMan.UpdateSim();

			g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::SimTotal);

			g_UInputMan.Update();

			// It is vital that server is updated after input manager but before activity because input manager will clear received pressed and released events on next update.
			if (g_NetworkServer.IsServerModeEnabled()) {
				g_NetworkServer.Update(true);
				serverUpdated = true;
			}
			g_FrameMan.Update();
			g_LuaMan.Update();
			g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActivityUpdate);
			g_ActivityMan.Update();
			g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActivityUpdate);
			g_MovableMan.Update();
            g_AudioMan.Update();

			g_ActivityMan.LateUpdateGlobalScripts();

			g_ConsoleMan.Update();
			g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::SimTotal);

			if (!g_InActivity) {
				g_TimerMan.PauseSim(true);
				// If we're not in a metagame, then show main menu
				if (g_MetaMan.GameInProgress()) {
					//g_IntroState = CAMPAIGNFADEIN;
				} else {
					const Activity *activity = g_ActivityMan.GetActivity();
					// If we edited something then return to main menu instead of scenario menu player will probably switch to area/scene editor.
					if (activity && activity->GetPresetName() == "None") {
						//g_IntroState = MENUAPPEAR;
					} else {
						//g_IntroState = MAINTOSCENARIO;
					}
				}
				PlayIntroTitle();
			}
			// Resetting the simulation
			if (g_ResetActivity) {
				// Reset and quit if user quit during reset loading
				if (!ResetActivity()) { break; }
			}
			// Resuming the simulation
			if (g_ResumeActivity) { ResumeActivity(); }
		}

		if (g_NetworkServer.IsServerModeEnabled()) {
			// Pause sim while we're waiting for scene transmission or scene will start changing before clients receive them and those changes will be lost.
			if (!g_NetworkServer.ReadyForSimulation()) {
				g_TimerMan.PauseSim(true);
			} else {
				if (g_InActivity) { g_TimerMan.PauseSim(false); }
			}
			if (!serverUpdated) {
				g_NetworkServer.Update();
				serverUpdated = true;
			}
			if (g_NetworkServer.GetServerSimSleepWhenIdle()) {
				long long ticksToSleep = g_TimerMan.GetTimeToSleep();
				if (ticksToSleep > 0) {
					double secsToSleep = static_cast<double>(ticksToSleep) / static_cast<double>(g_TimerMan.GetTicksPerSecond());
					long long milisToSleep = static_cast<long long>(secsToSleep) * 1000;
					std::this_thread::sleep_for(std::chrono::milliseconds(milisToSleep));
				}
			}
		}
		g_FrameMan.Draw();
		g_FrameMan.FlipFrameBuffers();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Command-line argument handling.
/// </summary>
/// <param name="argCount">Argument count.</param>
/// <param name="argValue">Argument values.</param>
void HandleMainArgs(int argCount, char **argValue) {
	// Discard the first argument because it's always the executable path/name
	argCount--;
	argValue++;
	if (argCount == 0) {
		return;
	}
	bool launchModeSet = false;
	bool singleModuleSet = false;

	for (int i = 0; i < argCount;) {
		std::string currentArg = argValue[i];
		bool lastArg = i + 1 == argCount;

		// Print loading screen console to cout
		if (currentArg == "-cout") { System::EnableLoggingToCLI(); }

		// Load a single module right after the official modules
		if (!lastArg && !singleModuleSet && currentArg == "-module") {
			std::string moduleToLoad = argValue[++i];
			if (moduleToLoad.find(System::GetModulePackageExtension()) == moduleToLoad.length() - System::GetModulePackageExtension().length()) {
				g_PresetMan.SetSingleModuleToLoad(moduleToLoad);
				singleModuleSet = true;
			}
		}
		if (!launchModeSet) {
			// Launch game in server mode
			if (currentArg == "-server") {
				g_NetworkServer.EnableServerMode();
				g_NetworkServer.SetServerPort(!lastArg ? argValue[++i] : "8000");
				launchModeSet = true;
			// Launch game directly into editor activity
			} else if (!lastArg && currentArg == "-editor") {
				g_EditorToLaunch = argValue[++i];
				g_LaunchIntoEditor = true;
				launchModeSet = true;
			}
		}
		++i;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Implementation of the main function.
/// </summary>
int main(int argc, char **argv) {

	///////////////////////////////////////////////////////////////////
    // Initialize Allegro

    set_config_file("Base.rte/AllegroConfig.txt");
    allegro_init();
	loadpng_init();

    // Enable the exit button on the window
    LOCK_FUNCTION(QuitHandler);
    set_close_button_callback(QuitHandler);

    // Seed the random number generator
    SeedRNG();

	System::Initialize();

    ///////////////////////////////////////////////////////////////////
    // Create the essential managers

	Reader settingsReader("Base.rte/Settings.ini", false, nullptr, true);
    g_SettingsMan.Initialize(settingsReader);

	g_LuaMan.Initialize();
	g_NetworkServer.Initialize();
	g_NetworkClient.Initialize();
    g_TimerMan.Initialize();
	g_PerformanceMan.Initialize();
    g_FrameMan.Initialize();
    g_PostProcessMan.Initialize();
    if (g_AudioMan.Initialize() >= 0) {
        g_GUISound.Initialize();
    }
    g_UInputMan.Initialize();
    g_ConsoleMan.Initialize();
    g_ActivityMan.Initialize();
    g_MovableMan.Initialize();
    g_MetaMan.Initialize();
	g_MenuMan.Initialize();

	HandleMainArgs(argc, argv);

    ///////////////////////////////////////////////////////////////////
    // Main game driver

	if (g_NetworkServer.IsServerModeEnabled()) {
		g_NetworkServer.Start();
		g_UInputMan.SetMultiplayerMode(true);
		g_FrameMan.SetMultiplayerMode(true);
		g_AudioMan.SetMultiplayerMode(true);
		g_AudioMan.SetSoundsVolume(0);
		g_AudioMan.SetMusicVolume(0);
	}

	g_FrameMan.PrintForcedGfxDriverMessage();

	if (g_ConsoleMan.LoadWarningsExist()) {
		g_ConsoleMan.PrintString("WARNING: References to files that could not be located or failed to load detected during module loading!\nSee \"LogLoadingWarning.txt\" for a list of bad references.");
		g_ConsoleMan.SaveLoadWarningLog("LogLoadingWarning.txt");
		// Open the console so the user is aware there are loading warnings.
		g_ConsoleMan.SetEnabled(true);
	} else {
		// Delete an existing log if there are no warnings so there's less junk in the root folder.
		if (std::filesystem::exists(System::GetWorkingDirectory() + "LogLoadingWarning.txt")) { std::remove("LogLoadingWarning.txt"); }
	}

    if (!g_NetworkServer.IsServerModeEnabled()) {
		if (g_LaunchIntoEditor) {
			// Force mouse + keyboard with default mapping so we won't need to change manually if player 1 is set to keyboard only or gamepad.
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetDevice(InputDevice::DEVICE_MOUSE_KEYB);
			g_UInputMan.GetControlScheme(Players::PlayerOne)->SetPreset(InputPreset::PRESET_WASDKEYS);
			// Start the specified editor activity.
			if (!EnterEditorActivity(g_EditorToLaunch)) {
				//g_IntroState = g_SettingsMan.SkipIntro() ? MENUAPPEAR : START;
				PlayIntroTitle();
			}
		} else if (!g_SettingsMan.LaunchIntoActivity()) {
			//g_IntroState = g_SettingsMan.SkipIntro() ? MENUAPPEAR : START;
			PlayIntroTitle();
		}
	} else {
		// NETWORK Create multiplayer lobby activity to start as default if server is running
		EnterMultiplayerLobby();
	}

    // If we fail to start/reset the activity, then revert to the intro/menu
    if (!ResetActivity()) { PlayIntroTitle(); }
	
    RunGameLoop();

    ///////////////////////////////////////////////////////////////////
    // Clean up

	g_NetworkClient.Destroy();
	g_NetworkServer.Destroy();

    g_MetaMan.Destroy();
    g_MovableMan.Destroy();
    g_SceneMan.Destroy();
    g_ActivityMan.Destroy();
	g_GUISound.Destroy();
    g_AudioMan.Destroy();
    g_PresetMan.Destroy();
    g_UInputMan.Destroy();
    g_FrameMan.Destroy();
    g_TimerMan.Destroy();
    g_LuaMan.Destroy();
    ContentFile::FreeAllLoaded();
    g_ConsoleMan.Destroy();

#ifdef DEBUG_BUILD
    // Dump out the info about how well memory cleanup went
    Entity::ClassInfo::DumpPoolMemoryInfo(Writer("MemCleanupInfo.txt"));
#endif
	
    return 0;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { return main(__argc, __argv); }
#endif
