//////////////////////////////////////////////////////////////////////////////////////////
// File:            SettingsMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SettingsMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SettingsMan.h"
#include "ContentFile.h"
#include "PresetMan.h"
#include "UInputMan.h"
#include "ActivityMan.h"
#include "ConsoleMan.h"
#include "PerformanceMan.h"
#include "PostProcessMan.h"

#include "GUI/GUI.h"
#include "GUI/GUIControlManager.h"

#include "RTEManagers.h"

extern bool g_ResetActivity;
extern bool g_ResetActivity;

namespace RTE {

const std::string SettingsMan::m_ClassName = "Settings";

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SettingsMan, effectively
//                  resetting the members of this abstraction level only.

void SettingsMan::Clear()
{
    m_PlayIntro = true;
    m_ToolTips = true;
	m_FlashOnBrainDamage = true;
	m_EnableHats = false;
	m_BlipOnRevealUnseen = true;
	m_ShowMetascenes = false;
	m_ShowForeignItems = true;
	m_EndlessMode = false;
	m_PrintDebugInfo = false;
	m_PreciseCollisions = true;
	m_ForceSafeGfxDriver = false;
	m_ForceSoftwareGfxDriver = false;
	m_ForceSafeGfxDriver = false;
	m_ForceVirtualFullScreenGfxDriver = false;
	m_ForceOverlayedWindowGfxDriver = false;
	m_ForceNonOverlayedWindowGfxDriver = false;
	m_AllowSavingToBase = false;
	m_RecommendedMOIDCount = 240;
    m_SoundPanningEffectStrength = 0.5;
	m_NetworkServerName = "";
	m_PlayerNetworkName = "";

	m_ServerSleepWhenIdle = false;
	m_ServerSimSleepWhenIdle = false;

	m_ServerUseHighCompression = true;
	m_ServerUseFastCompression = false;
	m_ServerHighCompressionLevel = 10;
	m_ServerFastAccelerationFactor = 1;
	m_ServerUseInterlacing = false;
	m_ServerEncodingFps = 30;
	m_ClientInputFps = 30;

	m_ServerTransmitAsBoxes = true;
	m_ServerBoxWidth = 32;
	m_ServerBoxHeight = 44;

	m_UseNATService = false;
	m_DisableLoadingScreen = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SettingsMan object ready for use.

int SettingsMan::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Serializable ready for use.

int SettingsMan::Create(Reader &reader, bool checkType, bool doCreate)
{
    if (reader.IsOK())
        return Serializable::Create(reader, checkType);

    // Couldn't find the settings file, so create a new one with all the good defaults!
    Writer writer("Base.rte/Settings.ini");
    RTEAssert(writer.WriterOK(), "After failing to open the Base.rte/Settings.ini, could not then even create a new one to save settings to! Are you trying to run the game from a read-only disk? You need to install the game to a writable area before running it!");

    // Save out the defaults
    SaveDefaults(writer);
    // Close the newly created default file, we're opening it again to read from
    writer.Destroy();

	Reader settings("Base.rte/Settings.ini");
	
    // Now read from that same file
    return Serializable::Create(settings, true, doCreate);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int SettingsMan::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ResolutionX")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "ResolutionY")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PaletteFile")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "Fullscreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "NxWindowed")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "NxFullscreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PixelsPerMeter")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PlayIntro")
        reader >> m_PlayIntro;
    else if (propName == "ToolTips")
        reader >> m_ToolTips;
	else if (propName == "AdvancedPerformanceStats") {
		bool enableAdvanced;
		reader >> enableAdvanced;
		g_PerformanceMan.ShowAdvancedPerformanceStats(enableAdvanced);	
	} else if (propName == "FlashOnBrainDamage")
        reader >> m_FlashOnBrainDamage;
    else if (propName == "EnableHats")
        reader >> m_EnableHats;
    else if (propName == "BlipOnRevealUnseen")
        reader >> m_BlipOnRevealUnseen;
    else if (propName == "ShowForeignItems")
        reader >> m_ShowForeignItems;
    else if (propName == "ForceSoftwareGfxDriver")
        reader >> m_ForceSoftwareGfxDriver;
    else if (propName == "ForceSafeGfxDriver")
        reader >> m_ForceSafeGfxDriver;
    else if (propName == "ForceVirtualFullScreenGfxDriver")
        reader >> m_ForceVirtualFullScreenGfxDriver;
    else if (propName == "ForceOverlayedWindowGfxDriver")
        reader >> m_ForceOverlayedWindowGfxDriver;
    else if (propName == "ForceNonOverlayedWindowGfxDriver")
        reader >> m_ForceNonOverlayedWindowGfxDriver;
	else if (propName == "AllowSavingToBase")
		reader >> m_AllowSavingToBase;
	else if (propName == "ConsoleScreenRatio")
    {
        float ratio;
        reader >> ratio;
        g_ConsoleMan.SetConsoleScreenSize(ratio);
    }
    else if (propName == "DeltaTime")
    {
        float dt;
        reader >> dt; 
        g_TimerMan.SetDeltaTimeSecs(dt);
    }
	else if (propName == "PreciseCollisions")
		reader >> m_PreciseCollisions;
	else if (propName == "RealToSimCap")
    {
        float cap;
        reader >> cap;
        g_TimerMan.SetRealToSimCap(cap);
    }
    else if (propName == "HSplitScreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "VSplitScreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "DefaultActivityType")
    {
        string type;
        reader >> type;
        g_ActivityMan.SetDefaultActivityType(type);
    }
    else if (propName == "DefaultActivityName")
    {
        string name;
        reader >> name;
        g_ActivityMan.SetDefaultActivityName(name);
    }
    else if (propName == "DefaultSceneName")
    {
        string name;
        reader >> name;
        g_SceneMan.SetDefaultSceneName(name);
    }
    else if (propName == "MaxUnheldItems")
        g_MovableMan.ReadProperty(propName, reader);
    else if (propName == "ShowMetascenes")
        reader >> m_ShowMetascenes;
    else if (propName == "SloMoThreshold")
        g_MovableMan.ReadProperty(propName, reader);
    else if (propName == "SloMoDurationMS")
        g_MovableMan.ReadProperty(propName, reader);
    else if (propName == "EnableParticleSettling")
        g_MovableMan.ReadProperty(propName, reader);
    else if (propName == "EnableMOSubtraction")
        g_MovableMan.ReadProperty(propName, reader);
    else if (propName == "EndlessMode")
        reader >> m_EndlessMode;
    else if (propName == "PrintDebugInfo")
        reader >> m_PrintDebugInfo;
	else if (propName == "RecommendedMOIDCount")
		reader >> m_RecommendedMOIDCount;
    else if (propName == "SoundPanningEffectStrength")
        reader >> m_SoundPanningEffectStrength;
	else if (propName == "PlayerNetworkName")
		reader >> m_PlayerNetworkName;
	else if (propName == "NetworkServerName")
		reader >> m_NetworkServerName;
	else if (propName == "ServerUseHighCompression")
		reader >> m_ServerUseHighCompression;
	else if (propName == "ServerUseFastCompression")
		reader >> m_ServerUseFastCompression;
	else if (propName == "ServerHighCompressionLevel")
		reader >> m_ServerHighCompressionLevel;
	else if (propName == "ServerFastAccelerationFactor")
		reader >> m_ServerFastAccelerationFactor;
	else if (propName == "ServerUseInterlacing")
		reader >> m_ServerUseInterlacing;
	else if (propName == "ServerEncodingFps")
		reader >> m_ServerEncodingFps;
	else if (propName == "ServerTransmitAsBoxes")
		reader >> m_ServerTransmitAsBoxes;
	else if (propName == "ServerBoxWidth")
		reader >> m_ServerBoxWidth;
	else if (propName == "ServerBoxHeight")
		reader >> m_ServerBoxHeight;
	else if (propName == "ClientInputFps")
		reader >> m_ClientInputFps;
	else if (propName == "UseNATService")
		reader >> m_UseNATService;
	else if (propName == "NATServiceAddress")
		reader >> m_NATServiceAddress;
	else if (propName == "NATServerName")
		reader >> m_NATServerName;
	else if (propName == "NATServerPassword")
		reader >> m_NATServerPassword;
	else if (propName == "ServerSleepWhenIdle")
		reader >> m_ServerSleepWhenIdle;
	else if (propName == "ServerSimSleepWhenIdle")
		reader >> m_ServerSimSleepWhenIdle;
	else if (propName == "DisableLoadingScreen")
		reader >> m_DisableLoadingScreen;
	else if (propName == "SoundVolume")
    {
        int volume = 0;
        reader >> volume;
        g_AudioMan.SetSoundsVolume((double)volume / (double)100);
    }
    else if (propName == "MusicVolume")
    {
        int volume = 0;
        reader >> volume;
        g_AudioMan.SetMusicVolume((double)volume / (double)100);
    }
    else if (propName == "P1Scheme" || propName == "P2Scheme" || propName == "P3Scheme" || propName == "P4Scheme" || propName == "MouseSensitivity")
        g_UInputMan.ReadProperty(propName, reader);
    else if (propName == "DisableMod")
    {
        string mod;
        reader >> mod;
		std::transform(mod.begin(), mod.end(), mod.begin(), ::tolower);
        DisableMod(mod);
    }
    else if (propName == "EnableScript")
    {
        string mod;
        reader >> mod;
		std::transform(mod.begin(), mod.end(), mod.begin(), ::tolower);
        EnableScript(mod);
    }
    else if (propName == "VisibleAssemblyGroup")
    {
        string assemblyGroup;
        reader >> assemblyGroup;
		m_VisibleAssemblyGroupsList.push_back(assemblyGroup);
    }
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SettingsMan with a Writer for
//                  later recreation with Create(Reader &reader);

int SettingsMan::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("ResolutionX");
    writer << g_FrameMan.GetNewResX();
    writer.NewProperty("ResolutionY");
    writer << g_FrameMan.GetNewResY();
    writer.NewProperty("PaletteFile");
    writer << g_FrameMan.GetPaletteFile();
    writer.NewProperty("Fullscreen");
    writer << g_FrameMan.IsFullscreen();
    writer.NewProperty("NxWindowed");
    writer << g_FrameMan.NxWindowed();
    writer.NewProperty("NxFullscreen");
    writer << g_FrameMan.GetNewNxFullscreen();
    writer.NewProperty("PixelsPerMeter");
    writer << g_FrameMan.GetPPM();
    writer.NewProperty("PlayIntro");
    writer << m_PlayIntro;
    writer.NewProperty("ToolTips");
    writer << m_ToolTips;
	writer.NewProperty("AdvancedPerformanceStats");
	writer << g_PerformanceMan.AdvancedPerformanceStatsEnabled();
    writer.NewProperty("FlashOnBrainDamage");
    writer << m_FlashOnBrainDamage;
    writer.NewProperty("EnableHats");
    writer << m_EnableHats;
    writer.NewProperty("BlipOnRevealUnseen");
    writer << m_BlipOnRevealUnseen;
    writer.NewProperty("ShowMetascenes");
    writer << m_ShowMetascenes;
	writer.NewProperty("AllowSavingToBase");
	writer << m_AllowSavingToBase;
	writer.NewProperty("ShowForeignItems");
    writer << m_ShowForeignItems;
	writer.NewProperty("ConsoleScreenRatio");
    writer << g_ConsoleMan.GetConsoleScreenSize();
    writer.NewProperty("DeltaTime");
    writer << g_TimerMan.GetDeltaTimeSecs();
    writer.NewProperty("RealToSimCap");
    writer << g_TimerMan.GetRealToSimCap();
	writer.NewProperty("PreciseCollisions");
	writer << m_PreciseCollisions;
    writer.NewProperty("HSplitScreen");
    writer << g_FrameMan.GetHSplit();
    writer.NewProperty("VSplitScreen");
    writer << g_FrameMan.GetVSplit();
    writer.NewProperty("DefaultActivityType");
    writer << g_ActivityMan.GetDefaultActivityType();
    writer.NewProperty("DefaultActivityName");
    writer << g_ActivityMan.GetDefaultActivityName();
    writer.NewProperty("DefaultSceneName");
    writer << g_SceneMan.GetDefaultSceneName();
    writer.NewProperty("MaxUnheldItems");
    writer << g_MovableMan.GetMaxDroppedItems();
    writer.NewProperty("SloMoThreshold");
    writer << g_MovableMan.GetSloMoThreshold();
    writer.NewProperty("SloMoDurationMS");
    writer << g_MovableMan.GetSloMoDuration();
    writer.NewProperty("EnableParticleSettling");
    writer << g_MovableMan.IsParticleSettlingEnabled();
    writer.NewProperty("EnableMOSubtraction");
    writer << g_MovableMan.IsMOSubtractionEnabled();
    writer.NewProperty("ForceSoftwareGfxDriver");
    writer << m_ForceSoftwareGfxDriver;
    writer.NewProperty("ForceSafeGfxDriver");
    writer << m_ForceSafeGfxDriver;
	writer.NewProperty("ForceVirtualFullScreenGfxDriver");
	writer << m_ForceVirtualFullScreenGfxDriver;
	writer.NewProperty("ForceOverlayedWindowGfxDriver");
	writer << m_ForceOverlayedWindowGfxDriver;
	writer.NewProperty("ForceNonOverlayedWindowGfxDriver");
	writer << m_ForceNonOverlayedWindowGfxDriver;
	writer.NewProperty("EndlessMode");
    writer << m_EndlessMode;
	writer.NewProperty("PrintDebugInfo");
    writer << m_PrintDebugInfo;
	writer.NewProperty("RecommendedMOIDCount");
	writer << m_RecommendedMOIDCount;
    writer.NewProperty("SoundPanningEffectStrength");
    writer << m_SoundPanningEffectStrength;
	writer.NewProperty("PlayerNetworkName");
	if (m_PlayerNetworkName == "")
		writer << "Dummy";
	else
		writer << m_PlayerNetworkName;
	writer.NewProperty("NetworkServerName");
	if (m_NetworkServerName == "")
		writer << "127.0.0.1:8000";
	else
		writer << m_NetworkServerName;
	writer.NewProperty("ServerUseHighCompression");
	writer << m_ServerUseHighCompression;
	writer.NewProperty("ServerUseFastCompression");
	writer << m_ServerUseFastCompression;
	writer.NewProperty("ServerHighCompressionLevel");
	writer << m_ServerHighCompressionLevel;
	writer.NewProperty("ServerFastAccelerationFactor");
	writer << m_ServerFastAccelerationFactor;
	writer.NewProperty("ServerUseInterlacing");
	writer << m_ServerUseInterlacing;
	writer.NewProperty("ServerEncodingFps");
	writer << m_ServerEncodingFps;
	writer.NewProperty("ServerTransmitAsBoxes");
	writer << m_ServerTransmitAsBoxes;
	writer.NewProperty("ServerBoxWidth");
	writer << m_ServerBoxWidth;
	writer.NewProperty("ServerBoxHeight");
	writer << m_ServerBoxHeight;
	writer.NewProperty("ClientInputFps");
	writer << m_ClientInputFps;
	writer.NewProperty("UseNATService");
	writer << m_UseNATService;

	writer.NewProperty("NATServiceAddress");
	if (m_NATServiceAddress == "")
		writer << "127.0.0.1:61111";
	else
		writer << m_NATServiceAddress;

	writer.NewProperty("NATServerName");
	if (m_NATServerName == "")
		writer << "DefaultServerName";
	else
		writer << m_NATServerName;

	writer.NewProperty("NATServerPassword");
	if (m_NATServerPassword == "")
		writer << "DefaultServerPassword";
	else
		writer << m_NATServerPassword;

	writer.NewProperty("ServerSleepWhenIdle");
	writer << m_ServerSleepWhenIdle;
	writer.NewProperty("ServerSimSleepWhenIdle");
	writer << m_ServerSimSleepWhenIdle;
	
	writer.NewProperty("DisableLoadingScreen");
	writer << m_DisableLoadingScreen;

	writer.NewProperty("SoundVolume");
    writer << g_AudioMan.GetSoundsVolume() * 100;
    writer.NewProperty("MusicVolume");
    writer << g_AudioMan.GetMusicVolume() * 100;

    // Write visible assembly groups
	for (list<string>::const_iterator itr = m_VisibleAssemblyGroupsList.begin(); itr != m_VisibleAssemblyGroupsList.end(); itr++)
	{
        writer.NewProperty("VisibleAssemblyGroup");
        writer << (*itr);
	}

	// Write disabled mods
	for(map<std::string, bool>::const_iterator itr = m_DisabledMods.begin(); itr != m_DisabledMods.end(); ++itr)
	{
		if (itr->second)
		{
	        writer.NewProperty("DisableMod");
			writer << itr->first;
		}
	}

	// Write enabled scripts
	for(map<std::string, bool>::const_iterator itr = m_EnabledScripts.begin(); itr != m_EnabledScripts.end(); ++itr)
	{
		if (itr->second)
		{
	        writer.NewProperty("EnableScript");
			writer << itr->first;
		}
	}

    // Dump out all the input schemes
    writer << g_UInputMan;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveDefaults
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the default state of this SettingsMan to an output stream for
//                  later recreation with Create(Reader &reader).

int SettingsMan::SaveDefaults(Writer &writer) const
{
    if (!writer.WriterOK())
        return -1;

    writer.ObjectStart(GetClassName());

    // 1080p with double pixels
    writer.NewProperty("ResolutionX");
    writer << 960;
    writer.NewProperty("ResolutionY");
    writer << 540;
    writer.NewProperty("PaletteFile");
    ContentFile paletteFile("Base.rte/palette.bmp");
    writer << paletteFile;
    writer.NewProperty("Fullscreen");
    writer << 0;
    writer.NewProperty("NxWindowed");
    writer << 1;
    writer.NewProperty("NxFullscreen");
    writer << 2;
    writer.NewProperty("PixelsPerMeter");
    writer << 20;
    writer.NewProperty("PlayIntro");
    writer << 1;
    writer.NewProperty("ToolTips");
    writer << 1;
	writer.NewProperty("AdvancedPerformanceStats");
	writer << 1;
    writer.NewProperty("FlashOnBrainDamage");
    writer << 1;
    writer.NewProperty("ShowForeignItems");
    writer << 1;
    writer.NewProperty("EnableHats");
    writer << 0;
    writer.NewProperty("ShowMetascenes");
    writer << 0;
    writer.NewProperty("BlipOnRevealUnseen");
    writer << 1;
    writer.NewProperty("ConsoleScreenRatio");
    writer << 0.4;
    writer.NewProperty("DeltaTime");
    writer << 0.0166667;
	writer.NewProperty("RealToSimCap");
    writer << 0.075;
	writer.NewProperty("PreciseCollisions");
	writer << 1;
	writer.NewProperty("HSplitScreen");
    writer << 0;
    writer.NewProperty("VSplitScreen");
    writer << 0;
    writer.NewProperty("DefaultActivityType");
    writer << "GATutorial";
    writer.NewProperty("DefaultActivityName");
    writer << "Tutorial Mission";
    writer.NewProperty("DefaultSceneName");
    writer << "Tutorial Bunker";
    writer.NewProperty("MaxUnheldItems");
    writer << 25;
    writer.NewProperty("SloMoThreshold");
    writer << 0;
    writer.NewProperty("SloMoDurationMS");
    writer << 1000;
    writer.NewProperty("EnableParticleSettling");
    writer << 1;
    writer.NewProperty("EnableMOSubtraction");
    writer << 1;
    writer.NewProperty("SoundVolume");
    writer << 41;
    writer.NewProperty("MusicVolume");
    writer << 100;

	// Default visible assembly groups
    writer.NewProperty("VisibleAssemblyGroup");
	writer << "Assemblies - Passages";
    writer.NewProperty("VisibleAssemblyGroup");
	writer << "Assemblies - Rooms";
    writer.NewProperty("VisibleAssemblyGroup");
	writer << "Assemblies - Prefabs";
    writer.NewProperty("VisibleAssemblyGroup");
	writer << "Assemblies - Generic small";
    writer.NewProperty("VisibleAssemblyGroup");
	writer << "Assemblies - Generic huge";

    // Dump out all the input schemes
//    writer << g_UInputMan;

    writer.ObjectEnd();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SettingsMan object.

void SettingsMan::Destroy()
{


    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsModEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified mod is not disabled in the settings.

bool SettingsMan::IsModDisabled(string modModule)
{  
	std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower); 
	if (m_DisabledMods.find(modModule) != m_DisabledMods.end())
		return m_DisabledMods[modModule];
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsScriptEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified mod is not disabled in the settings.

bool SettingsMan::IsScriptEnabled(string scriptName)
{
	std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower);
	if (m_EnabledScripts.find(scriptName) != m_EnabledScripts.end())
		return m_EnabledScripts[scriptName];
	return false;
}

} // namespace RTE