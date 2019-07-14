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
#include "LicenseMan.h"
#include "ConsoleMan.h"

#include "GUI/GUI.h"
#include "GUI/GUIControlManager.h"

#include "RTEManagers.h"

using namespace std;

extern bool g_ResetActivity;
extern bool g_ResetActivity;

namespace RTE
{

const string SettingsMan::m_ClassName = "Settings";
#define LICENSEFILEPATH "Base.rte/titlepalette.bmp"


#ifndef __OPEN_SOURCE_EDITION

/////////////////////////////
// TURN OPTIMIZATIONS OFF
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", off)

#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SettingsMan, effectively
//                  resetting the members of this abstraction level only.

void SettingsMan::Clear()
{
#ifndef __OPEN_SOURCE_EDITION
    CRYPT_START
#endif

    m_PlayIntro = true;
    m_ToolTips = true;
	m_FlashOnBrainDamage = true;
	m_EnableHats = false;
	m_BlipOnRevealUnseen = true;
	m_ShowMetascenes = false;
	m_ShowForeignItems = true;
	m_SteamDisabled = false;
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

	m_AudioChannels = 32;

    // Hardcode all the license pixel coordiantes
    m_LicensePixels.clear();
    m_LicensePixels.push_back(Vector(1, 0));
    m_LicensePixels.push_back(Vector(10, 0));
    m_LicensePixels.push_back(Vector(5, 1));
    // -
    m_LicensePixels.push_back(Vector(8, 5));
    m_LicensePixels.push_back(Vector(4, 6));
    m_LicensePixels.push_back(Vector(6, 9));
    m_LicensePixels.push_back(Vector(2, 11));
    // -
    m_LicensePixels.push_back(Vector(14, 9));
    m_LicensePixels.push_back(Vector(0, 14));
    m_LicensePixels.push_back(Vector(5, 15));
    m_LicensePixels.push_back(Vector(1, 4));
    // -
    m_LicensePixels.push_back(Vector(1, 14));
    m_LicensePixels.push_back(Vector(12, 14));
    m_LicensePixels.push_back(Vector(15, 9));
    m_LicensePixels.push_back(Vector(7, 12));

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SettingsMan object ready for use.

int SettingsMan::Create()
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    if (Serializable::Create() < 0)
        return -1;

// This is now done in main() before this is created
    // Attempt to read in and load the license key from the secret place. This is OK if it fails, just means the copy isn't licensed yet
//    if (ReadLicenseKey())
//    {
// This is done later when the email is also read from the setting ini
        // If the key was read and entered successfully, try to validate it
//        LicenseMan::ServerResult result = g_LicenseMan.Validate();
        // If this fails to contact the server, that's OK too.. we need to allow registered players to play even when they are not connected to the internet
//        if (result == LicenseMan::INVALIDKEY)
//            ;
//    }
    return 0;
    
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
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
    AAssert(writer.WriterOK(), "After failing to open the Base.rte/Settings.ini, could not then even create a new one to save settings to! Are you trying to run the game from a read-only disk? You need to install the game to a writable area before running it!");

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
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

    if (propName == "ResolutionX")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "ResolutionY")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "TrueColorMode")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PaletteFile")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "Fullscreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "NxWindowed")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "NxFullscreen")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PostProcessing")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PostPixelGlow")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PixelsPerMeter")
        g_FrameMan.ReadProperty(propName, reader);
    else if (propName == "PlayIntro")
        reader >> m_PlayIntro;
    else if (propName == "ToolTips")
        reader >> m_ToolTips;
    else if (propName == "FlashOnBrainDamage")
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
    else if (propName == "DisableSteam")
        reader >> m_SteamDisabled;
    else if (propName == "EndlessMode")
        reader >> m_EndlessMode;
    else if (propName == "PrintDebugInfo")
        reader >> m_PrintDebugInfo;
	else if (propName == "RecommendedMOIDCount")
		reader >> m_RecommendedMOIDCount;
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
	else if (propName == "AudioChannels")
		reader >> m_AudioChannels;
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
    else if (propName == "LastEmail")
    {
        string email;
        reader >> email;
        g_LicenseMan.SetLicenseEmail(email);
        // See if we already have a key, then try to validate
		std::string licenseKey = g_LicenseMan.GetLicenseKey();
        if (g_LicenseMan.CheckKeyFormatting(licenseKey))
        {
            LicenseMan::ServerResult result = g_LicenseMan.Validate();
        }
    }
    else if (propName == "LastKey")
    {
        string key;
        reader >> key;
        g_LicenseMan.SetLastLicenseKey(key);
    }
    else if (propName == "SteamWorkshopMod")
    {
        string mod;
        reader >> mod;
        AddWorkshopModToList(mod);
    }
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

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
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
    writer.NewProperty("TrueColorMode");
    writer << (g_FrameMan.GetBPP() == 32);
    writer.NewProperty("PaletteFile");
    writer << g_FrameMan.GetPaletteFile();
    writer.NewProperty("Fullscreen");
    writer << g_FrameMan.IsFullscreen();
    writer.NewProperty("NxWindowed");
    writer << g_FrameMan.NxWindowed();
    writer.NewProperty("NxFullscreen");
    writer << g_FrameMan.GetNewNxFullscreen();
    writer.NewProperty("PostProcessing");
    writer << g_FrameMan.IsPostProcessing();
    writer.NewProperty("PostPixelGlow");
    writer << g_FrameMan.IsPixelGlowEnabled();
    writer.NewProperty("PixelsPerMeter");
    writer << g_FrameMan.GetPPM();
    writer.NewProperty("PlayIntro");
    writer << m_PlayIntro;
    writer.NewProperty("ToolTips");
    writer << m_ToolTips;
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
	writer.NewProperty("DisableSteam");
    writer << m_SteamDisabled;
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

	writer.NewProperty("AudioChannels");
	writer << m_AudioChannels;
	writer.NewProperty("SoundVolume");
    writer << g_AudioMan.GetSoundsVolume() * 100;
    writer.NewProperty("MusicVolume");
    writer << g_AudioMan.GetMusicVolume() * 100;

    // Write workshop installed mod list
	for (list<string>::const_iterator itr = m_SubscribedModList.begin(); itr != m_SubscribedModList.end(); itr++)
	{
        writer.NewProperty("SteamWorkshopMod");
        writer << (*itr);
	}

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

	std::string lastLicenseKey = g_LicenseMan.GetLastLicenseKey();
    if (g_LicenseMan.CheckKeyFormatting(lastLicenseKey))
    {
        writer.NewProperty("LastEmail");
        writer << g_LicenseMan.GetLastLicenseEmail();
        writer.NewProperty("LastKey");
        writer << g_LicenseMan.GetLastLicenseKey();
    }

    // Dump out all the input schemes
    writer << g_UInputMan;


    // Write the license file - DON'T.. only write it just when we register successfully. Don't risk messing up the file afterward
//    WriteLicenseKey();

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
    writer.NewProperty("TrueColorMode");
    writer << 1;
    writer.NewProperty("PaletteFile");
    ContentFile paletteFile("Base.rte/palette.bmp");
    writer << paletteFile;
    writer.NewProperty("Fullscreen");
    writer << 0;
    writer.NewProperty("NxWindowed");
    writer << 1;
    writer.NewProperty("NxFullscreen");
    writer << 2;
    writer.NewProperty("PostProcessing");
    writer << 1;
    writer.NewProperty("PostPixelGlow");
    writer << 1;
    writer.NewProperty("PixelsPerMeter");
    writer << 20;
    writer.NewProperty("PlayIntro");
    writer << 1;
    writer.NewProperty("ToolTips");
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
// Method:          ReadLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the license key in from the secret/obfuscated data location in
//                  the current installation.

bool SettingsMan::ReadLicenseKey() const
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

#ifdef _DEBUG
    return true;
#endif // _DEBUG

    // Load the image with the data encoded
    PALETTE licensePalette;
    BITMAP *pLicBitmap = load_bitmap(LICENSEFILEPATH, licensePalette);
    if (!pLicBitmap || pLicBitmap->w < 16 || pLicBitmap->h < 16)
        return false;

    // Extract the string from a bunch of predetermined pixels of the bitmap XXX-XXXX-XXXX-XXXX
    string keyString;
    // Go through all the hardcoded coordinates and extract from each pixel
    for (list<Vector>::const_iterator itr = m_LicensePixels.begin(); itr != m_LicensePixels.end(); ++itr)
    {
        keyString += (char)(getpixel(pLicBitmap, (*itr).GetFloorIntX(), (*itr).GetFloorIntY()));
    }

    // Put the dashes back where they are supposed to be or decryption will screw up
    keyString.insert(3, "-");
    keyString.insert(8, "-");
    keyString.insert(13, "-");

    // Decrypt the key using the hostname. If the current machine's hostname is different, then we have been moved!
//    keyString = XORStrings(keyString, g_LicenseMan.GetMachineHostname(), '-');

    // Attempt to set the license key. If it's rejected, report that
    bool success = g_LicenseMan.SetLicenseKey(keyString);

    // Clean up the bitmap
    destroy_bitmap(pLicBitmap);
    pLicBitmap = 0;

    return success;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ModsInstalledLastTime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns list of mods which were installed last time.

std::list<string> SettingsMan::ModsInstalledLastTime() const
{
	return m_SubscribedModList;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ClearModList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears internal list of installed workshop mods

void SettingsMan::ClearWorkshopModList()
{
	m_SubscribedModList.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			AddModToList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified mod to internal list of installed workshop mods.

void SettingsMan::AddWorkshopModToList(string modModule)
{
	m_SubscribedModList.push_back(modModule);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WriteLicenseKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes the current license key of the LicenseMan to the secret/obfuscated
//                  data location in the current installation. This will write an invalid
//                  key string if there isn't a valid key, so it will be identified as such
//                  (being invalid, that is) upon reading next time.

bool SettingsMan::WriteLicenseKey() const
{
#ifndef __OPEN_SOURCE_EDITION
	CRYPT_START
#endif

#ifdef _DEBUG
    return true;
#endif // _DEBUG

    // Only write out if the current license is valid
// NO! because if the key was released or doesn't exist, it should be written out as "" and not stay in data
//    if (!g_LicenseMan.HasValidatedLicense())
//        return false;

    // Get the current license string from the licenseMan
    string keyString = g_LicenseMan.GetLicenseKey();
    // Check that it has a proper length
// Again, no, we want to write out "" if that's what we've got
//    if (!(keyString.size() == 15 || keyString.size() == 18))
//        return false;

    // Encrypt the license key with the hostname as encryption key
//    keyString = XORStrings(keyString, g_LicenseMan.GetMachineHostname(), '-');

    // Create an image to encode the key into
    BITMAP *pLicBitmap = create_bitmap_ex(8, 16, 16);

    // Write a bunch of noise into the bitmap so the key pixels won't stand out
    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            putpixel(pLicBitmap, x, y, (int)RangeRand(0, 255));

    // Encode the string from a bunch of predetermined pixels of the bitmap
    // Go through all the hardcoded coordinates and encode into each pixel for each string letter
    int stringIndex = 0;
    for (list<Vector>::const_iterator itr = m_LicensePixels.begin(); itr != m_LicensePixels.end(); ++itr)
    {
        // If no key, write invalid characters so the key will be invalid upon reading and we will know that it isn't an actual key
        if (keyString.empty())
        {
            // Write the first char as invalid for sure, and the rest as random to not make it obvious which pixels in teh bitmap are used
            putpixel(pLicBitmap, (*itr).GetFloorIntX(), (*itr).GetFloorIntY(), stringIndex == 0 ? ' ' : RangeRand(0, 255));
        }
        // Write the actual key properly if there is one
        else
        {
            // Skip the dash locations if the key seems to have the dashes
            if (keyString.size() == 18 && (stringIndex == 3 || stringIndex == 8 || stringIndex == 13))
                stringIndex++;

            putpixel(pLicBitmap, (*itr).GetFloorIntX(), (*itr).GetFloorIntY(), (char)keyString[stringIndex]);
        }

        stringIndex++;
    }

    // Write the bitmap to the secret file, overwriting it
    // Always use the black palette, more incognito!
    bool success = save_bmp(LICENSEFILEPATH, pLicBitmap, black_palette) == 0;

    // Clean up the bitmap
    destroy_bitmap(pLicBitmap);
    pLicBitmap = 0;

    return success;

#ifndef __OPEN_SOURCE_EDITION
	CRYPT_END
#endif
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

#ifndef __OPEN_SOURCE_EDITION

/////////////////////////////
// TURN OPTIMIZATIONS ON
// This is so the EXECryptor markers don't get mangled by the optimizer

#pragma optimize("", on)

#endif

} // namespace RTE
