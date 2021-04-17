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

#include "GUI.h"
#include "AllegroInput.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"

#include "MainMenuGUI.h"
#include "ScenarioGUI.h"
#include "TitleScreen.h"

#include "MenuMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "PerformanceMan.h"
#include "MetaMan.h"
#include "NetworkServer.h"

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

using namespace RTE;

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Game menus loop.
	/// </summary>
	/// <returns></returns>
	void RunMenuLoop() {
		g_UInputMan.DisableKeys(false);
		g_UInputMan.TrapMousePos(false);

		// Don't stop the music if reinitializing after a resolution change
		if (!g_FrameMan.ResolutionChanged()) { g_AudioMan.StopAll(); }

		while (!System::IsSetToQuit() && !g_ActivityMan.ActivitySetToResume()) {
			g_UInputMan.Update();
			g_TimerMan.Update();
			g_TimerMan.UpdateSim();
			g_AudioMan.Update();

			if (g_NetworkServer.IsServerModeEnabled()) { g_NetworkServer.Update(); }

			if (g_MenuMan.Update()) {
				break;
			}
			g_MenuMan.Draw();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// Game simulation loop.
	/// </summary>
	void RunGameLoop() {
		if (System::IsSetToQuit()) {
			return;
		}
		g_PerformanceMan.ResetFrameTimer();
		g_TimerMan.PauseSim(false);

		if (g_ActivityMan.ActivitySetToRestart() && !g_ActivityMan.RestartActivity()) { g_MenuMan.GetTitleScreen()->SetTitleTransitionState(TitleScreen::TitleTransition::MainMenuToScenario); }

		while (!System::IsSetToQuit()) {
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

				if (!g_ActivityMan.IsInActivity()) {
					g_TimerMan.PauseSim(true);
					if (g_MetaMan.GameInProgress()) {
						g_MenuMan.GetTitleScreen()->SetTitleTransitionState(TitleScreen::TitleTransition::CampaignFadeIn);
					} else {
						const Activity *activity = g_ActivityMan.GetActivity();
						// If we edited something then return to main menu instead of scenario menu player will probably switch to area/scene editor.
						if (activity && activity->GetPresetName() == "None") {
							g_MenuMan.GetTitleScreen()->SetTitleTransitionState(TitleScreen::TitleTransition::PlanetToMainMenu);
						} else {
							g_MenuMan.GetTitleScreen()->SetTitleTransitionState(TitleScreen::TitleTransition::ScenarioFadeIn);
						}
					}
					RunMenuLoop();
				}
				if (g_ActivityMan.ActivitySetToRestart() && !g_ActivityMan.RestartActivity()) {
					break;
				}
				if (g_ActivityMan.ActivitySetToResume()) {
					g_ActivityMan.ResumeActivity();
					g_PerformanceMan.ResetFrameTimer();
				}
			}

			if (g_NetworkServer.IsServerModeEnabled()) {
				// Pause sim while we're waiting for scene transmission or scene will start changing before clients receive them and those changes will be lost.
				if (!g_NetworkServer.ReadyForSimulation()) {
					g_TimerMan.PauseSim(true);
				} else {
					if (g_ActivityMan.IsInActivity()) { g_TimerMan.PauseSim(false); }
				}
				if (!serverUpdated) { g_NetworkServer.Update(); }

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
					g_ActivityMan.SetEditorToLaunch(argValue[++i]);
					launchModeSet = true;
				}
			}
			++i;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Implementation of the main function.
/// </summary>
int main(int argc, char **argv) {
    set_config_file("Base.rte/AllegroConfig.txt");
    allegro_init();
	loadpng_init();

    // Enable the exit button on the window
    LOCK_FUNCTION(System::WindowCloseButtonHandler);
    set_close_button_callback(System::WindowCloseButtonHandler);

	System::Initialize();
    SeedRNG();

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
    g_MovableMan.Initialize();
    g_MetaMan.Initialize();
	g_MenuMan.Initialize();

	HandleMainArgs(argc, argv);

    ///////////////////////////////////////////////////////////////////
    // Main game driver

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

    if (!g_ActivityMan.Initialize()) { RunMenuLoop(); }
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