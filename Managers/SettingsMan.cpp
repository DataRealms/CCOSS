#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "CameraMan.h"
#include "MovableMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "PostProcessMan.h"
#include "AudioMan.h"
#include "PerformanceMan.h"
#include "UInputMan.h"
#include "NetworkClient.h"
#include "NetworkServer.h"

namespace RTE {

	const std::string SettingsMan::c_ClassName = "SettingsMan";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMan::Clear() {
		m_SettingsPath = System::GetUserdataDirectory() + "Settings.ini";
		m_SettingsNeedOverwrite = false;

		m_FlashOnBrainDamage = true;
		m_BlipOnRevealUnseen = false;
		m_UnheldItemsHUDDisplayRange = 25 * c_PPM;
		m_AlwaysDisplayUnheldItemsInStrategicMode = true;
		m_SubPieMenuHoverOpenDelay = 1000;
		m_EndlessMetaGameMode = false;
		m_EnableCrabBombs = false;
		m_CrabBombThreshold = 42;
		m_ShowEnemyHUD = true;
		m_EnableSmartBuyMenuNavigation = true;
		m_AutomaticGoldDeposit = true;

		m_NetworkServerAddress = "127.0.0.1:8000";
		m_PlayerNetworkName = "Dummy";
		m_NATServiceAddress = "127.0.0.1:61111";
		m_NATServerName = "DefaultServerName";
		m_NATServerPassword = "DefaultServerPassword";
		m_UseExperimentalMultiplayerSpeedBoosts = true;

		m_AllowSavingToBase = false;
		m_ShowForeignItems = true;
		m_ShowMetaScenes = false;

		m_RecommendedMOIDCount = 240;
		m_SceneBackgroundAutoScaleMode = 1;
		m_DisableFactionBuyMenuThemes = false;
		m_DisableFactionBuyMenuThemeCursors = false;
		m_PathFinderGridNodeSize = c_PPM;
		m_AIUpdateInterval = 2;

		m_SkipIntro = false;
		m_ShowToolTips = true;
		m_DisableLoadingScreenProgressReport = true;
		m_LoadingScreenProgressReportPrecision = 100;
		m_MenuTransitionDurationMultiplier = 1.0F;

		m_DrawAtomGroupVisualizations = false;
		m_DrawHandAndFootGroupVisualizations = false;
		m_DrawLimbPathVisualizations = false;
		m_PrintDebugInfo = false;
		m_MeasureModuleLoadTime = false;

		m_DisabledMods.clear();
		m_EnabledGlobalScripts.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::Initialize() {
		if (const char *settingsTempPath = std::getenv("CCCP_SETTINGSPATH")) { m_SettingsPath = std::string(settingsTempPath); }

		Reader settingsReader(m_SettingsPath, false, nullptr, true, true);

		if (!settingsReader.ReaderOK()) {
			Writer settingsWriter(m_SettingsPath);
			RTEAssert(settingsWriter.WriterOK(), "After failing to open the " + m_SettingsPath + ", could not then even create a new one to save settings to!\nAre you trying to run the game from a read-only disk?\nYou need to install the game to a writable area before running it!");

			// Settings file doesn't need to be populated with anything right now besides this manager's ClassName for serialization. It will be overwritten with the full list of settings with default values from all the managers before modules start loading.
			settingsWriter.ObjectStart(GetClassName());
			settingsWriter.EndWrite();

			m_SettingsNeedOverwrite = true;

			Reader newSettingsReader(m_SettingsPath, false, nullptr, false, true);
			return Serializable::Create(newSettingsReader);
		}

		int failureCode = Serializable::Create(settingsReader);

		if (GetAnyExperimentalSettingsEnabled()) {
			// Show a message box to annoy people as much as possible while they're using experimental settings, so they can't leave it on accidentally
			RTEError::ShowMessageBox("Experimental settings are enabled!\nThis may break mods, crash the game, corrupt saves or worse.\nUse at your own risk.");
		}

		return failureCode;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMan::UpdateSettingsFile() const {
		Writer settingsWriter(m_SettingsPath);
		g_SettingsMan.Save(settingsWriter);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::ReadProperty(const std::string_view &propName, Reader &reader) {
		StartPropertyList(return Serializable::ReadProperty(propName, reader));
		
		MatchProperty("PaletteFile", { reader >> g_FrameMan.m_PaletteFile; });
		MatchProperty("ResolutionX", { reader >> g_WindowMan.m_ResX; });
		MatchProperty("ResolutionY", { reader >> g_WindowMan.m_ResY; });
		MatchProperty("ResolutionMultiplier", { reader >> g_WindowMan.m_ResMultiplier; });
		MatchProperty("EnableVSync", { reader >> g_WindowMan.m_EnableVSync; });
		MatchProperty("Fullscreen", { reader >> g_WindowMan.m_Fullscreen; });
		MatchProperty("UseMultiDisplays", { reader >> g_WindowMan.m_UseMultiDisplays; });
		MatchProperty("TwoPlayerSplitscreenVertSplit", { reader >> g_FrameMan.m_TwoPlayerVSplit; });
		MatchProperty("MasterVolume", { g_AudioMan.SetMasterVolume(std::stof(reader.ReadPropValue()) / 100.0F); });
		MatchProperty("MuteMaster", { reader >> g_AudioMan.m_MuteMaster; });
		MatchProperty("MusicVolume", { g_AudioMan.SetMusicVolume(std::stof(reader.ReadPropValue()) / 100.0F); });
		MatchProperty("MuteMusic", { reader >> g_AudioMan.m_MuteMusic; });
		MatchProperty("SoundVolume", { g_AudioMan.SetSoundsVolume(std::stof(reader.ReadPropValue()) / 100.0F); });
		MatchProperty("MuteSounds", { reader >> g_AudioMan.m_MuteSounds; });
		MatchProperty("SoundPanningEffectStrength", {
			reader >> g_AudioMan.m_SoundPanningEffectStrength;

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		});
		MatchProperty("ListenerZOffset", { reader >> g_AudioMan.m_ListenerZOffset; });
		MatchProperty("MinimumDistanceForPanning", {
			reader >> g_AudioMan.m_MinimumDistanceForPanning;
		//////////////////////////////////////////////////

		} else if (propName == "ShowForeignItems") {
			reader >> m_ShowForeignItems;
		} else if (propName == "FlashOnBrainDamage") {
			reader >> m_FlashOnBrainDamage;
		} else if (propName == "BlipOnRevealUnseen") {
			reader >> m_BlipOnRevealUnseen;
		} else if (propName == "MaxUnheldItems") {
			reader >> g_MovableMan.m_MaxDroppedItems;
		} else if (propName == "UnheldItemsHUDDisplayRange") {
			SetUnheldItemsHUDDisplayRange(std::stof(reader.ReadPropValue()));
		} else if (propName == "AlwaysDisplayUnheldItemsInStrategicMode") {
			reader >> m_AlwaysDisplayUnheldItemsInStrategicMode;
    } else if (propName == "SubPieMenuHoverOpenDelay") {
			reader >> m_SubPieMenuHoverOpenDelay;
		} else if (propName == "EndlessMode") {
			reader >> m_EndlessMetaGameMode;
		} else if (propName == "EnableCrabBombs") {
			reader >> m_EnableCrabBombs;
		} else if (propName == "CrabBombThreshold") {
			reader >> m_CrabBombThreshold;
		} else if (propName == "ShowEnemyHUD") {
			reader >> m_ShowEnemyHUD;
		} else if (propName == "SmartBuyMenuNavigation") {
			reader >> m_EnableSmartBuyMenuNavigation;
		} else if (propName == "ScreenShakeStrength") {
			reader >> m_ScreenShakeStrength;
		} else if (propName == "ScreenShakeDecay") {
			reader >> m_ScreenShakeDecay;
		} else if (propName == "MaxScreenShakeTime") {
			reader >> m_MaxScreenShakeTime;
		} else if (propName == "DefaultShakePerUnitOfGibEnergy") {
			reader >> m_DefaultShakePerUnitOfGibEnergy;
		} else if (propName == "DefaultShakePerUnitOfRecoilEnergy") {
			reader >> m_DefaultShakePerUnitOfRecoilEnergy;
		} else if (propName == "DefaultShakeFromRecoilMaximum") {
			reader >> m_DefaultShakeFromRecoilMaximum;
		} else if (propName == "LaunchIntoActivity") {
			reader >> g_ActivityMan.m_LaunchIntoActivity;
		} else if (propName == "DefaultActivityType") {
			reader >> g_ActivityMan.m_DefaultActivityType;
		} else if (propName == "DefaultActivityName") {
			reader >> g_ActivityMan.m_DefaultActivityName;
		} else if (propName == "DefaultSceneName") {
			reader >> g_SceneMan.m_DefaultSceneName;
		} else if (propName == "DisableLuaJIT") {
			reader >> g_LuaMan.m_DisableLuaJIT;
		} else if (propName == "RecommendedMOIDCount") {
			reader >> m_RecommendedMOIDCount;
		} else if (propName == "SceneBackgroundAutoScaleMode") {
			SetSceneBackgroundAutoScaleMode(std::stoi(reader.ReadPropValue()));
		} else if (propName == "DisableFactionBuyMenuThemes") {
			reader >> m_DisableFactionBuyMenuThemes;
		} else if (propName == "PathFinderGridNodeSize") {
			reader >> m_PathFinderGridNodeSize;
		} else if (propName == "AIUpdateInterval") {
			reader >> m_AIUpdateInterval;
		} else if (propName == "EnableParticleSettling") {
			reader >> g_MovableMan.m_SettlingEnabled;
		} else if (propName == "EnableMOSubtraction") {
			reader >> g_MovableMan.m_MOSubtractionEnabled;
		} else if (propName == "DeltaTime") {
			g_TimerMan.SetDeltaTimeSecs(std::stof(reader.ReadPropValue()));
		} else if (propName == "RealToSimCap") {
			g_TimerMan.SetRealToSimCap(std::stof(reader.ReadPropValue()));
		} else if (propName == "AllowSavingToBase") {
			reader >> m_AllowSavingToBase;
		} else if (propName == "ShowMetaScenes") {
			reader >> m_ShowMetaScenes;
		} else if (propName == "SkipIntro") {
			reader >> m_SkipIntro;
		} else if (propName == "ShowToolTips") {
			reader >> m_ShowToolTips;
		} else if (propName == "CaseSensitiveFilePaths") {
			System::EnableFilePathCaseSensitivity(std::stoi(reader.ReadPropValue()));
		} else if (propName == "DisableLoadingScreenProgressReport") {
			reader >> m_DisableLoadingScreenProgressReport;
		} else if (propName == "LoadingScreenProgressReportPrecision") {
			reader >> m_LoadingScreenProgressReportPrecision;
		} else if (propName == "ConsoleScreenRatio") {
			g_ConsoleMan.SetConsoleScreenSize(std::stof(reader.ReadPropValue()));
		} else if (propName == "ConsoleUseMonospaceFont") {
			reader >> g_ConsoleMan.m_ConsoleUseMonospaceFont;
		} else if (propName == "AdvancedPerformanceStats") {
			reader >> g_PerformanceMan.m_AdvancedPerfStats;
		} else if (propName == "MenuTransitionDurationMultiplier") {
			SetMenuTransitionDurationMultiplier(std::stof(reader.ReadPropValue()));
		} else if (propName == "DrawAtomGroupVisualizations") {
			reader >> m_DrawAtomGroupVisualizations;
		} else if (propName == "DrawHandAndFootGroupVisualizations") {
			reader >> m_DrawHandAndFootGroupVisualizations;
		} else if (propName == "DrawLimbPathVisualizations") {
			reader >> m_DrawLimbPathVisualizations;
		} else if (propName == "DrawRaycastVisualizations") {
			reader >> g_SceneMan.m_DrawRayCastVisualizations;
		} else if (propName == "DrawPixelCheckVisualizations") {
			reader >> g_SceneMan.m_DrawPixelCheckVisualizations;
		} else if (propName == "PrintDebugInfo") {
			reader >> m_PrintDebugInfo;
		} else if (propName == "MeasureModuleLoadTime") {
			reader >> m_MeasureModuleLoadTime;
		} else if (propName == "PlayerNetworkName") {
			reader >> m_PlayerNetworkName;
		} else if (propName == "NetworkServerName") {
			reader >> m_NetworkServerAddress;
		} else if (propName == "UseNATService") {
			reader >> g_NetworkServer.m_UseNATService;
		} else if (propName == "NATServiceAddress") {
			reader >> m_NATServiceAddress;
		} else if (propName == "NATServerName") {
			reader >> m_NATServerName;
		} else if (propName == "NATServerPassword") {
			reader >> m_NATServerPassword;
		} else if (propName == "UseExperimentalMultiplayerSpeedBoosts") {
			reader >> m_UseExperimentalMultiplayerSpeedBoosts;
		} else if (propName == "ClientInputFps") {
			reader >> g_NetworkClient.m_ClientInputFps;
		} else if (propName == "ServerTransmitAsBoxes") {
			reader >> g_NetworkServer.m_TransmitAsBoxes;
		} else if (propName == "ServerBoxWidth") {
			reader >> g_NetworkServer.m_BoxWidth;
		} else if (propName == "ServerBoxHeight") {
			reader >> g_NetworkServer.m_BoxHeight;
		} else if (propName == "ServerUseHighCompression") {
			reader >> g_NetworkServer.m_UseHighCompression;
		} else if (propName == "ServerUseFastCompression") {
			reader >> g_NetworkServer.m_UseFastCompression;
		} else if (propName == "ServerUseDeltaCompression") {
			reader >> g_NetworkServer.m_UseDeltaCompression;
		} else if (propName == "ServerHighCompressionLevel") {
			reader >> g_NetworkServer.m_HighCompressionLevel;
		} else if (propName == "ServerFastAccelerationFactor") {
			reader >> g_NetworkServer.m_FastAccelerationFactor;
		} else if (propName == "ServerUseInterlacing") {
			reader >> g_NetworkServer.m_UseInterlacing;
		} else if (propName == "ServerEncodingFps") {
			reader >> g_NetworkServer.m_EncodingFps;
		} else if (propName == "ServerSleepWhenIdle") {
			reader >> g_NetworkServer.m_SleepWhenIdle;
		} else if (propName == "ServerSimSleepWhenIdle") {
			reader >> g_NetworkServer.m_SimSleepWhenIdle;
		} else if (propName == "VisibleAssemblyGroup") {
			m_VisibleAssemblyGroupsList.push_back(reader.ReadPropValue());
		} else if (propName == "DisableMod") {
			m_DisabledMods.try_emplace(reader.ReadPropValue(), true);
		} else if (propName == "EnableGlobalScript") {
			m_EnabledGlobalScripts.try_emplace(reader.ReadPropValue(), true);
		} else if (propName == "MouseSensitivity") {
			reader >> g_UInputMan.m_MouseSensitivity;
		} else if (propName == "Player1Scheme" || propName == "Player2Scheme" || propName == "Player3Scheme" || propName == "Player4Scheme") {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "Player" + playerNum + "Scheme") {
					g_UInputMan.m_ControlScheme[player].Reset();
					reader >> g_UInputMan.m_ControlScheme[player];
					break;
				}
			}
		});
		
		EndPropertyList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewDivider(false);
		writer.NewLineString("// Display Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("PaletteFile", g_FrameMan.m_PaletteFile);
		writer.NewPropertyWithValue("ResolutionX", g_WindowMan.m_ResX);
		writer.NewPropertyWithValue("ResolutionY", g_WindowMan.m_ResY);
		writer.NewPropertyWithValue("ResolutionMultiplier", g_WindowMan.m_ResMultiplier);
		writer.NewPropertyWithValue("Fullscreen", g_WindowMan.m_Fullscreen);
		writer.NewPropertyWithValue("EnableVSync", g_WindowMan.m_EnableVSync);
		writer.NewPropertyWithValue("UseMultiDisplays", g_WindowMan.m_UseMultiDisplays);
		writer.NewPropertyWithValue("TwoPlayerSplitscreenVertSplit", g_FrameMan.m_TwoPlayerVSplit);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Audio Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("MasterVolume", g_AudioMan.m_MasterVolume * 100);
		writer.NewPropertyWithValue("MuteMaster", g_AudioMan.m_MuteMaster);
		writer.NewPropertyWithValue("MusicVolume", g_AudioMan.m_MusicVolume * 100);
		writer.NewPropertyWithValue("MuteMusic", g_AudioMan.m_MuteMusic);
		writer.NewPropertyWithValue("SoundVolume", g_AudioMan.m_SoundsVolume * 100);
		writer.NewPropertyWithValue("MuteSounds", g_AudioMan.m_MuteSounds);
		writer.NewPropertyWithValue("SoundPanningEffectStrength", g_AudioMan.m_SoundPanningEffectStrength);

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking.
		writer.NewPropertyWithValue("ListenerZOffset", g_AudioMan.m_ListenerZOffset);
		writer.NewPropertyWithValue("MinimumDistanceForPanning", g_AudioMan.m_MinimumDistanceForPanning);
		//////////////////////////////////////////////////

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Gameplay Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("ShowForeignItems", m_ShowForeignItems);
		writer.NewPropertyWithValue("FlashOnBrainDamage", m_FlashOnBrainDamage);
		writer.NewPropertyWithValue("BlipOnRevealUnseen", m_BlipOnRevealUnseen);
		writer.NewPropertyWithValue("MaxUnheldItems", g_MovableMan.m_MaxDroppedItems);
		writer.NewPropertyWithValue("UnheldItemsHUDDisplayRange", m_UnheldItemsHUDDisplayRange);
		writer.NewPropertyWithValue("AlwaysDisplayUnheldItemsInStrategicMode", m_AlwaysDisplayUnheldItemsInStrategicMode);
		writer.NewPropertyWithValue("SubPieMenuHoverOpenDelay", m_SubPieMenuHoverOpenDelay);
		writer.NewPropertyWithValue("EndlessMetaGameMode", m_EndlessMetaGameMode);
		writer.NewPropertyWithValue("EnableCrabBombs", m_EnableCrabBombs);
		writer.NewPropertyWithValue("CrabBombThreshold", m_CrabBombThreshold);
		writer.NewPropertyWithValue("ShowEnemyHUD", m_ShowEnemyHUD);
		writer.NewPropertyWithValue("SmartBuyMenuNavigation", m_EnableSmartBuyMenuNavigation);
		writer.NewPropertyWithValue("ScrapCompactingHeight", g_SceneMan.m_ScrapCompactingHeight);
		writer.NewPropertyWithValue("AutomaticGoldDeposit", m_AutomaticGoldDeposit);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Screen Shake Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("ScreenShakeStrength", g_CameraMan.m_ScreenShakeStrength);
		writer.NewPropertyWithValue("ScreenShakeDecay", g_CameraMan.m_ScreenShakeDecay);
		writer.NewPropertyWithValue("MaxScreenShakeTime", g_CameraMan.m_MaxScreenShakeTime);
		writer.NewPropertyWithValue("DefaultShakePerUnitOfGibEnergy", g_CameraMan.m_DefaultShakePerUnitOfGibEnergy);
		writer.NewPropertyWithValue("DefaultShakePerUnitOfRecoilEnergy", g_CameraMan.m_DefaultShakePerUnitOfRecoilEnergy);
		writer.NewPropertyWithValue("DefaultShakeFromRecoilMaximum", g_CameraMan.m_DefaultShakeFromRecoilMaximum);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Default Activity Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("LaunchIntoActivity", g_ActivityMan.m_LaunchIntoActivity);
		writer.NewPropertyWithValue("DefaultActivityType", g_ActivityMan.m_DefaultActivityType);
		writer.NewPropertyWithValue("DefaultActivityName", g_ActivityMan.m_DefaultActivityName);
		writer.NewPropertyWithValue("DefaultSceneName", g_SceneMan.m_DefaultSceneName);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Engine Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("DisableLuaJIT", m_DisableLuaJIT);
		writer.NewPropertyWithValue("RecommendedMOIDCount", m_RecommendedMOIDCount);
		writer.NewPropertyWithValue("SceneBackgroundAutoScaleMode", m_SceneBackgroundAutoScaleMode);
		writer.NewPropertyWithValue("DisableFactionBuyMenuThemes", m_DisableFactionBuyMenuThemes);
		writer.NewPropertyWithValue("DisableFactionBuyMenuThemeCursors", m_DisableFactionBuyMenuThemeCursors);
		writer.NewPropertyWithValue("PathFinderGridNodeSize", m_PathFinderGridNodeSize);
		writer.NewPropertyWithValue("AIUpdateInterval", m_AIUpdateInterval);
		writer.NewPropertyWithValue("EnableParticleSettling", g_MovableMan.m_SettlingEnabled);
		writer.NewPropertyWithValue("EnableMOSubtraction", g_MovableMan.m_MOSubtractionEnabled);
		writer.NewPropertyWithValue("DeltaTime", g_TimerMan.GetDeltaTimeSecs());
		
		// No experimental settings right now :)
		//writer.NewLine(false, 2);
		//writer.NewDivider(false);
		//writer.NewLineString("// Engine Settings - EXPERIMENTAL", false);
		//writer.NewLineString("// These settings are experimental! They may break mods, crash the game, corrupt saves or worse. Use at your own risk.", false);
		//writer.NewLine(false);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Editor Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("AllowSavingToBase", m_AllowSavingToBase);
		writer.NewPropertyWithValue("ShowMetaScenes", m_ShowMetaScenes);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Misc Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("SkipIntro", m_SkipIntro);
		writer.NewPropertyWithValue("ShowToolTips", m_ShowToolTips);
		writer.NewPropertyWithValue("CaseSensitiveFilePaths", System::FilePathsCaseSensitive());
		writer.NewPropertyWithValue("DisableLoadingScreenProgressReport", m_DisableLoadingScreenProgressReport);
		writer.NewPropertyWithValue("LoadingScreenProgressReportPrecision", m_LoadingScreenProgressReportPrecision);
		writer.NewPropertyWithValue("ConsoleScreenRatio", g_ConsoleMan.m_ConsoleScreenRatio);
		writer.NewPropertyWithValue("ConsoleUseMonospaceFont", g_ConsoleMan.m_ConsoleUseMonospaceFont);
		writer.NewPropertyWithValue("AdvancedPerformanceStats", g_PerformanceMan.m_AdvancedPerfStats);
		writer.NewPropertyWithValue("MenuTransitionDurationMultiplier", m_MenuTransitionDurationMultiplier);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Modder Debug Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("DrawAtomGroupVisualizations", m_DrawAtomGroupVisualizations);
		writer.NewPropertyWithValue("DrawHandAndFootGroupVisualizations", m_DrawHandAndFootGroupVisualizations);
		writer.NewPropertyWithValue("DrawLimbPathVisualizations", m_DrawLimbPathVisualizations);
		writer.NewPropertyWithValue("DrawRaycastVisualizations", g_SceneMan.m_DrawRayCastVisualizations);
		writer.NewPropertyWithValue("DrawPixelCheckVisualizations", g_SceneMan.m_DrawPixelCheckVisualizations);
		writer.NewPropertyWithValue("PrintDebugInfo", m_PrintDebugInfo);
		writer.NewPropertyWithValue("MeasureModuleLoadTime", m_MeasureModuleLoadTime);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Network Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("PlayerNetworkName", m_PlayerNetworkName);
		writer.NewPropertyWithValue("NetworkServerName", m_NetworkServerAddress);
		writer.NewPropertyWithValue("UseNATService", g_NetworkServer.m_UseNATService);
		writer.NewPropertyWithValue("NATServiceAddress", m_NATServiceAddress);
		writer.NewPropertyWithValue("NATServerName", m_NATServerName);
		writer.NewPropertyWithValue("NATServerPassword", m_NATServerPassword);
		writer.NewPropertyWithValue("UseExperimentalMultiplayerSpeedBoosts", m_UseExperimentalMultiplayerSpeedBoosts);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Advanced Network Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("ClientInputFps", g_NetworkClient.m_ClientInputFps);
		writer.NewPropertyWithValue("ServerTransmitAsBoxes", g_NetworkServer.m_TransmitAsBoxes);
		writer.NewPropertyWithValue("ServerBoxWidth", g_NetworkServer.m_BoxWidth);
		writer.NewPropertyWithValue("ServerBoxHeight", g_NetworkServer.m_BoxHeight);
		writer.NewPropertyWithValue("ServerUseHighCompression", g_NetworkServer.m_UseHighCompression);
		writer.NewPropertyWithValue("ServerUseFastCompression", g_NetworkServer.m_UseFastCompression);
		writer.NewPropertyWithValue("ServerUseDeltaCompression", g_NetworkServer.m_UseDeltaCompression);
		writer.NewPropertyWithValue("ServerHighCompressionLevel", g_NetworkServer.m_HighCompressionLevel);
		writer.NewPropertyWithValue("ServerFastAccelerationFactor", g_NetworkServer.m_FastAccelerationFactor);
		writer.NewPropertyWithValue("ServerUseInterlacing", g_NetworkServer.m_UseInterlacing);
		writer.NewPropertyWithValue("ServerEncodingFps", g_NetworkServer.m_EncodingFps);
		writer.NewPropertyWithValue("ServerSleepWhenIdle", g_NetworkServer.m_SleepWhenIdle);
		writer.NewPropertyWithValue("ServerSimSleepWhenIdle", g_NetworkServer.m_SimSleepWhenIdle);

		if (!m_VisibleAssemblyGroupsList.empty()) {
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Enabled Bunker Assembly Groups", false);
			writer.NewLine(false);
			for (const std::string &visibleAssembly : m_VisibleAssemblyGroupsList) {
				writer.NewPropertyWithValue("VisibleAssemblyGroup", visibleAssembly);
			}
		}

		if (!m_DisabledMods.empty()) {
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Disabled Mods", false);
			writer.NewLine(false);
			for (const auto &[modPath, modDisabled] : m_DisabledMods) {
				if (modDisabled) { writer.NewPropertyWithValue("DisableMod", modPath); }
			}
		}

		if (!m_EnabledGlobalScripts.empty()) {
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Enabled Global Scripts", false);
			writer.NewLine(false);
			for (const auto &[scriptPresetName, scriptEnabled] : m_EnabledGlobalScripts) {
				if (scriptEnabled) { writer.NewPropertyWithValue("EnableGlobalScript", scriptPresetName); }
			}
		}

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Input Mapping", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("MouseSensitivity", g_UInputMan.m_MouseSensitivity);

		writer.NewLine(false);
		writer.NewLineString("// Input Devices:  0 = Keyboard Only, 1 = Mouse + Keyboard, 2 = Gamepad One, 3 = Gamepad Two, , 4 = Gamepad Three, 5 = Gamepad Four");
		writer.NewLineString("// Scheme Presets: 0 = No Preset, 1 = Arrow Keys, 2 = WASD Keys, 3 = Mouse + WASD Keys, 4 = Generic DPad, 5 = Generic Dual Analog, 6 = SNES, 7 = DualShock 4, 8 = XBox 360");

		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			std::string playerNum = std::to_string(player + 1);
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Player " + playerNum, false);
			writer.NewLine(false);
			writer.NewPropertyWithValue("Player" + playerNum + "Scheme", g_UInputMan.m_ControlScheme[player]);
		}

		writer.ObjectEnd();

		return 0;
	}
}
