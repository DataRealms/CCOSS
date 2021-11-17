#include "SettingsMan.h"
#include "ConsoleMan.h"
#include "MovableMan.h"
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
		m_SettingsPath = "Base.rte/Settings.ini";
		m_SettingsNeedOverwrite = false;

		m_FlashOnBrainDamage = true;
		m_BlipOnRevealUnseen = true;
		m_UnheldItemsHUDDisplayRange = 25;
		m_EndlessMetaGameMode = false;
		m_EnableCrabBombs = false;
		m_CrabBombThreshold = 42;
		m_ShowEnemyHUD = true;
		m_EnableSmartBuyMenuNavigation = true;

		m_NetworkServerAddress = "127.0.0.1:8000";
		m_PlayerNetworkName = "Dummy";
		m_NATServiceAddress = "127.0.0.1:61111";
		m_NATServerName = "DefaultServerName";
		m_NATServerPassword = "DefaultServerPassword";

		m_AllowSavingToBase = false;
		m_ShowForeignItems = true;
		m_ShowMetaScenes = false;

		m_RecommendedMOIDCount = 240;
		m_SimplifiedCollisionDetection = false;

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

		Reader settingsReader(m_SettingsPath, false, nullptr, true);

		if (!settingsReader.ReaderOK()) {
			Writer settingsWriter(m_SettingsPath);
			RTEAssert(settingsWriter.WriterOK(), "After failing to open the " + m_SettingsPath + ", could not then even create a new one to save settings to!\nAre you trying to run the game from a read-only disk?\nYou need to install the game to a writable area before running it!");

			// Settings file doesn't need to be populated with anything right now besides this manager's ClassName for serialization. It will be overwritten with the full list of settings with default values from all the managers before modules start loading.
			settingsWriter.ObjectStart(GetClassName());
			settingsWriter.EndWrite();

			m_SettingsNeedOverwrite = true;

			Reader newSettingsReader(m_SettingsPath);
			return Serializable::Create(newSettingsReader);
		}
		return Serializable::Create(settingsReader);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMan::UpdateSettingsFile() const {
		Writer settingsWriter(m_SettingsPath);
		g_SettingsMan.Save(settingsWriter);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "PaletteFile") {
			reader >> g_FrameMan.m_PaletteFile;
		} else if (propName == "ResolutionX") {
			reader >> g_FrameMan.m_ResX;
		} else if (propName == "ResolutionY") {
			reader >> g_FrameMan.m_ResY;
		} else if (propName == "ResolutionMultiplier") {
			reader >> g_FrameMan.m_ResMultiplier;
		} else if (propName == "DisableMultiScreenResolutionValidation") {
			reader >> g_FrameMan.m_DisableMultiScreenResolutionValidation;
		} else if (propName == "ForceVirtualFullScreenGfxDriver") {
			reader >> g_FrameMan.m_ForceVirtualFullScreenGfxDriver;
		} else if (propName == "ForceDedicatedFullScreenGfxDriver") {
			reader >> g_FrameMan.m_ForceDedicatedFullScreenGfxDriver;
		} else if (propName == "TwoPlayerSplitscreenVertSplit") {
			reader >> g_FrameMan.m_TwoPlayerVSplit;
		} else if (propName == "MasterVolume") {
			g_AudioMan.SetMasterVolume(std::stof(reader.ReadPropValue()) / 100.0F);
		} else if (propName == "MuteMaster") {
			reader >> g_AudioMan.m_MuteMaster;
		} else if (propName == "MusicVolume") {
			g_AudioMan.SetMusicVolume(std::stof(reader.ReadPropValue()) / 100.0F);
		} else if (propName == "MuteMusic") {
			reader >> g_AudioMan.m_MuteMusic;
		} else if (propName == "SoundVolume") {
			g_AudioMan.SetSoundsVolume(std::stof(reader.ReadPropValue()) / 100.0F);
		} else if (propName == "MuteSounds") {
			reader >> g_AudioMan.m_MuteSounds;
		} else if (propName == "SoundPanningEffectStrength") {
			reader >> g_AudioMan.m_SoundPanningEffectStrength;

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		} else if (propName == "ListenerZOffset") {
			reader >> g_AudioMan.m_ListenerZOffset;
		} else if (propName == "MinimumDistanceForPanning") {
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
		} else if (propName == "SloMoThreshold") {
			reader >> g_MovableMan.m_SloMoThreshold;
		} else if (propName == "SloMoDurationMS") {
			reader >> g_MovableMan.m_SloMoDuration;
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
		} else if (propName == "SimplifiedCollisionDetection") {
			reader >> m_SimplifiedCollisionDetection;
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
					g_UInputMan.m_ControlScheme.at(player).Reset();
					reader >> g_UInputMan.m_ControlScheme.at(player);
					break;
				}
			}
		} else {
			return Serializable::ReadProperty(propName, reader);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::Save(Writer &writer) const {
		Serializable::Save(writer);

		writer.NewDivider(false);
		writer.NewLineString("// Display Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("PaletteFile", g_FrameMan.m_PaletteFile);
		writer.NewPropertyWithValue("ResolutionX", g_FrameMan.m_ResX);
		writer.NewPropertyWithValue("ResolutionY", g_FrameMan.m_ResY);
		writer.NewPropertyWithValue("ResolutionMultiplier", g_FrameMan.m_ResMultiplier);
		writer.NewPropertyWithValue("DisableMultiScreenResolutionValidation", g_FrameMan.m_DisableMultiScreenResolutionValidation);
		writer.NewPropertyWithValue("ForceVirtualFullScreenGfxDriver", g_FrameMan.m_ForceVirtualFullScreenGfxDriver);
		writer.NewPropertyWithValue("ForceDedicatedFullScreenGfxDriver", g_FrameMan.m_ForceDedicatedFullScreenGfxDriver);
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
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
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
		writer.NewPropertyWithValue("SloMoThreshold", g_MovableMan.m_SloMoThreshold);
		writer.NewPropertyWithValue("SloMoDurationMS", g_MovableMan.m_SloMoDuration);
		writer.NewPropertyWithValue("EndlessMetaGameMode", m_EndlessMetaGameMode);
		writer.NewPropertyWithValue("EnableCrabBombs", m_EnableCrabBombs);
		writer.NewPropertyWithValue("CrabBombThreshold", m_CrabBombThreshold);
		writer.NewPropertyWithValue("ShowEnemyHUD", m_ShowEnemyHUD);
		writer.NewPropertyWithValue("SmartBuyMenuNavigation", m_EnableSmartBuyMenuNavigation);

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
		writer.NewPropertyWithValue("DisableLuaJIT", g_LuaMan.m_DisableLuaJIT);
		writer.NewPropertyWithValue("RecommendedMOIDCount", m_RecommendedMOIDCount);
		writer.NewPropertyWithValue("SimplifiedCollisionDetection", m_SimplifiedCollisionDetection);
		writer.NewPropertyWithValue("EnableParticleSettling", g_MovableMan.m_SettlingEnabled);
		writer.NewPropertyWithValue("EnableMOSubtraction", g_MovableMan.m_MOSubtractionEnabled);
		writer.NewPropertyWithValue("DeltaTime", g_TimerMan.GetDeltaTimeSecs());
		writer.NewPropertyWithValue("RealToSimCap", g_TimerMan.GetRealToSimCap());

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
			writer.NewPropertyWithValue("Player" + playerNum + "Scheme", g_UInputMan.m_ControlScheme.at(player));
		}

		writer.ObjectEnd();

		return 0;
	}
}
