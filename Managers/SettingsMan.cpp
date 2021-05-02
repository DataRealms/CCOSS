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
		m_SettingsNeedOverwrite = false;

		m_FlashOnBrainDamage = true;
		m_BlipOnRevealUnseen = true;
		m_EndlessMode = false;
		m_EnableHats = false;
		m_EnableCrabBombs = false;
		m_CrabBombThreshold = 42;

		m_NetworkServerAddress = "127.0.0.1:8000";
		m_PlayerNetworkName = "Dummy";
		m_NATServiceAddress = "127.0.0.1:61111";
		m_NATServerName = "DefaultServerName";
		m_NATServerPassword = "DefaultServerPassword";

		m_AllowSavingToBase = false;
		m_ShowForeignItems = true;
		m_ShowMetaScenes = false;

		m_RecommendedMOIDCount = 240;

		m_LaunchIntoActivity = false;

		m_SkipIntro = false;
		m_ToolTips = true;
		m_DisableLoadingScreen = true;
		m_LoadingScreenReportPrecision = 100;
		m_MenuTransitionDurationMultiplier = 1.0F;

		m_DrawAtomGroupVisualizations = false;
		m_DrawHandAndFootGroupVisualizations = false;
		m_DrawLimbPathVisualizations = false;
		m_PrintDebugInfo = false;
		m_MeasureModuleLoadTime = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::Initialize(Reader &reader, bool checkType, bool doCreate) {
		if (reader.ReaderOK()) {
			return Serializable::Create(reader, checkType);
		}

		// Couldn't find the settings file, so create a new one with all the good defaults!
		Writer settingsWriter("Base.rte/Settings.ini");
		RTEAssert(settingsWriter.WriterOK(), "After failing to open the Base.rte/Settings.ini, could not then even create a new one to save settings to!\nAre you trying to run the game from a read-only disk?\nYou need to install the game to a writable area before running it!");

		WriteDefaultSettings(settingsWriter);
		settingsWriter.EndWrite();

		Reader settingsReader("Base.rte/Settings.ini");
		return Serializable::Create(settingsReader, true, doCreate);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SettingsMan::UpdateSettingsFile() const {
		Writer settingsWriter("Base.rte/Settings.ini");
		g_SettingsMan.Save(settingsWriter);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "PaletteFile") {
			reader >> g_FrameMan.m_PaletteFile;
		} else if (propName == "ResolutionX") {
			reader >> g_FrameMan.m_ResX;
			g_FrameMan.m_NewResX = g_FrameMan.m_ResX;
		} else if (propName == "ResolutionY") {
			reader >> g_FrameMan.m_ResY;
			g_FrameMan.m_NewResY = g_FrameMan.m_ResY;
		} else if (propName == "ResolutionMultiplier") {
			reader >> g_FrameMan.m_ResMultiplier;
		} else if (propName == "DisableMultiScreenResolutionValidation") {
			reader >> g_FrameMan.m_DisableMultiScreenResolutionValidation;
		} else if (propName == "HSplitScreen") {
			reader >> g_FrameMan.m_HSplitOverride;
		} else if (propName == "VSplitScreen") {
			reader >> g_FrameMan.m_VSplitOverride;
		} else if (propName == "ForceVirtualFullScreenGfxDriver") {
			reader >> g_FrameMan.m_ForceVirtualFullScreenGfxDriver;
		} else if (propName == "ForceDedicatedFullScreenGfxDriver") {
			reader >> g_FrameMan.m_ForceDedicatedFullScreenGfxDriver;
		} else if (propName == "SoundVolume") {
			g_AudioMan.SetSoundsVolume(std::stof(reader.ReadPropValue()) / 100.0F);
		} else if (propName == "MusicVolume") {
			g_AudioMan.SetMusicVolume(std::stof(reader.ReadPropValue()) / 100.0F);
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
		} else if (propName == "SloMoThreshold") {
			reader >> g_MovableMan.m_SloMoThreshold;
		} else if (propName == "SloMoDurationMS") {
			reader >> g_MovableMan.m_SloMoDuration;
		} else if (propName == "EndlessMode") {
			reader >> m_EndlessMode;
		} else if (propName == "EnableHats") {
			reader >> m_EnableHats;
		} else if (propName == "EnableCrabBombs") {
			reader >> m_EnableCrabBombs;
		} else if (propName == "CrabBombThreshold") {
			reader >> m_CrabBombThreshold;
		} else if (propName == "LaunchIntoActivity") {
			reader >> m_LaunchIntoActivity;
		} else if (propName == "DefaultActivityType") {
			reader >> g_ActivityMan.m_DefaultActivityType;
		} else if (propName == "DefaultActivityName") {
			reader >> g_ActivityMan.m_DefaultActivityName;
		} else if (propName == "DefaultSceneName") {
			reader >> g_SceneMan.m_DefaultSceneName;
		} else if (propName == "RecommendedMOIDCount") {
			reader >> m_RecommendedMOIDCount;
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
		} else if (propName == "ToolTips") {
			reader >> m_ToolTips;
		} else if (propName == "CaseSensitiveFilePaths") {
			System::EnableFilePathCaseSensitivity(std::stoi(reader.ReadPropValue()));
		} else if (propName == "DisableLoadingScreen") {
			reader >> m_DisableLoadingScreen;
		} else if (propName == "LoadingScreenReportPrecision") {
			reader >> m_LoadingScreenReportPrecision;
		} else if (propName == "ConsoleScreenRatio") {
			g_ConsoleMan.SetConsoleScreenSize(std::stof(reader.ReadPropValue()));
		} else if (propName == "AdvancedPerformanceStats") {
			reader >> g_PerformanceMan.m_AdvancedPerfStats;
		} else if (propName == "MenuTransitionDuration") {
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
			DisableMod(reader.ReadPropValue());
		} else if (propName == "EnableScript") {
			EnableScript(reader.ReadPropValue());
		} else if (propName == "MouseSensitivity") {
			reader >> g_UInputMan.m_MouseSensitivity;
		} else if (propName == "Player1Scheme" || propName == "Player2Scheme" || propName == "Player3Scheme" || propName == "Player4Scheme") {
			for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
				std::string playerNum = std::to_string(player + 1);
				if (propName == "Player" + playerNum + "Scheme") {
					reader >> g_UInputMan.m_ControlScheme[player];
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
		writer.NewPropertyWithValue("ResolutionX", g_FrameMan.m_NewResX);
		writer.NewPropertyWithValue("ResolutionY", g_FrameMan.m_NewResY);
		writer.NewPropertyWithValue("ResolutionMultiplier", g_FrameMan.m_ResMultiplier);
		writer.NewPropertyWithValue("DisableMultiScreenResolutionValidation", g_FrameMan.m_DisableMultiScreenResolutionValidation);
		writer.NewPropertyWithValue("HSplitScreen", g_FrameMan.m_HSplitOverride);
		writer.NewPropertyWithValue("VSplitScreen", g_FrameMan.m_VSplitOverride);
		writer.NewPropertyWithValue("ForceVirtualFullScreenGfxDriver", g_FrameMan.m_ForceVirtualFullScreenGfxDriver);
		writer.NewPropertyWithValue("ForceDedicatedFullScreenGfxDriver", g_FrameMan.m_ForceDedicatedFullScreenGfxDriver);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Audio Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("SoundVolume", g_AudioMan.m_SoundsVolume * 100);
		writer.NewPropertyWithValue("MusicVolume", g_AudioMan.m_MusicVolume * 100);
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
		writer.NewPropertyWithValue("SloMoThreshold", g_MovableMan.m_SloMoThreshold);
		writer.NewPropertyWithValue("SloMoDurationMS", g_MovableMan.m_SloMoDuration);
		writer.NewPropertyWithValue("EndlessMode", m_EndlessMode);
		writer.NewPropertyWithValue("EnableHats", m_EnableHats);
		writer.NewPropertyWithValue("EnableCrabBombs", m_EnableCrabBombs);
		writer.NewPropertyWithValue("CrabBombThreshold", m_CrabBombThreshold);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Default Activity Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("LaunchIntoActivity", m_LaunchIntoActivity);
		writer.NewPropertyWithValue("DefaultActivityType", g_ActivityMan.m_DefaultActivityType);
		writer.NewPropertyWithValue("DefaultActivityName", g_ActivityMan.m_DefaultActivityName);
		writer.NewPropertyWithValue("DefaultSceneName", g_SceneMan.m_DefaultSceneName);

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Engine Settings", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("RecommendedMOIDCount", m_RecommendedMOIDCount);
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
		writer.NewPropertyWithValue("ToolTips", m_ToolTips);
		writer.NewPropertyWithValue("CaseSensitiveFilePaths", System::FilePathsCaseSensitive());
		writer.NewPropertyWithValue("DisableLoadingScreen", m_DisableLoadingScreen);
		writer.NewPropertyWithValue("LoadingScreenReportPrecision", m_LoadingScreenReportPrecision);
		writer.NewPropertyWithValue("ConsoleScreenRatio", g_ConsoleMan.m_ConsoleScreenRatio);
		writer.NewPropertyWithValue("AdvancedPerformanceStats", g_PerformanceMan.m_AdvancedPerfStats);
		writer.NewPropertyWithValue("MenuTransitionDuration", m_MenuTransitionDurationMultiplier);

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
			for (const std::pair<std::string, bool> &disabledMod : m_DisabledMods) {
				if (disabledMod.second) { writer.NewPropertyWithValue("DisableMod", disabledMod.first); }
			}
		}

		if (!m_EnabledScripts.empty()) {
			writer.NewLine(false, 2);
			writer.NewDivider(false);
			writer.NewLineString("// Enabled Global Scripts", false);
			writer.NewLine(false);
			for (const std::pair<std::string, bool> &enabledScript : m_EnabledScripts) {
				if (enabledScript.second) { writer.NewPropertyWithValue("EnableScript", enabledScript.first); }
			}
		}

		writer.NewLine(false, 2);
		writer.NewDivider(false);
		writer.NewLineString("// Input Mapping", false);
		writer.NewLine(false);
		writer.NewPropertyWithValue("MouseSensitivity", g_UInputMan.m_MouseSensitivity);

		writer.NewLine(false);
		writer.NewLineString("// Input Devices:  0 = Keyboard Only, 1 = Mouse + Keyboard, 2 = Joystick One, 3 = Joystick Two, , 4 = Joystick Three, 5 = Joystick Four");
		writer.NewLineString("// Scheme Presets: 0 = No Preset, 1 = WASD, 2 = Cursor Keys, 3 = XBox 360 Controller");

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SettingsMan::WriteDefaultSettings(Writer &writer) {
		if (!writer.WriterOK()) {
			return -1;
		}
		writer.ObjectStart(GetClassName());

		writer.NewPropertyWithValue("PaletteFile", ContentFile("Base.rte/palette.bmp"));
		writer.NewPropertyWithValue("ResolutionX", 960);
		writer.NewPropertyWithValue("ResolutionY", 540);
		writer.NewPropertyWithValue("ResolutionMultiplier", true);
		writer.NewPropertyWithValue("DisableMultiScreenResolutionValidation", false);
		writer.NewPropertyWithValue("SoundVolume", 40);
		writer.NewPropertyWithValue("MusicVolume", 60);

		writer.ObjectEnd();

		m_SettingsNeedOverwrite = true;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsMan::IsModDisabled(std::string modModule) {
		std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower);
		if (m_DisabledMods.find(modModule) != m_DisabledMods.end()) {
			return m_DisabledMods[modModule];
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool SettingsMan::IsScriptEnabled(std::string scriptName) {
		std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower);
		if (m_EnabledScripts.find(scriptName) != m_EnabledScripts.end()) {
			return m_EnabledScripts[scriptName];
		}
		return false;
	}
}
