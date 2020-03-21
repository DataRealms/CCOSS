#ifndef _RTESETTINGSMAN_
#define _RTESETTINGSMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SettingsMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the SettingsMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "Singleton.h"
#define g_SettingsMan SettingsMan::Instance()
#include "Serializable.h"

namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SettingsMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager over the application and misc settings.
// Parent(s):       Singleton, Serializable.
// Class history:   8/4/2006 SettingsMan created.

class SettingsMan:
    public Singleton<SettingsMan>,
    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SettingsMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SettingsMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SettingsMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SettingsMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SettingsMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~SettingsMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SettingsMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Serializable ready for use.
// Arguments:       A Reader that the Serializable will create itself from.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SettingsMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SettingsMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the SettingsMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveDefaults
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the default state of this SettingsMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the SettingsMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int SaveDefaults(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SettingsMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PlayIntro
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Is app set to play the game intro on startup?
// Arguments:       None.
// Return value:    Whether should play intro or not.

    bool PlayIntro() const { return m_PlayIntro; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetPlayIntro
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Is app set to play the game intro on startup?
// Arguments:       Whether should play intro or not.
// Return value:    None.

	void SetPlayIntro(bool play) { m_PlayIntro = play; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ToolTips
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Is app set to display tools tips on certain UI elements?
// Arguments:       None.
// Return value:    Whether should show tool tips or not.

    bool ToolTips() const { return m_ToolTips; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetShowToolTips
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether app set to display tools tips on certain UI elements.
// Arguments:       Whether should show tool tips or not.
// Return value:    None.

    void SetShowToolTips(bool showToolTips) { m_ToolTips = showToolTips; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FlashOnBrainDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether red and white flashes appear when brain is damaged.
// Arguments:       None.
// Return value:    Whether red and white flashes appear when brain is damaged.

    bool FlashOnBrainDamage() const { return m_FlashOnBrainDamage; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FlashOnBrainDamage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether red and white flashes appear when brain is damaged.
// Arguments:       New value for Flash on brain damage setting.
// Return value:    None.

    void SetFlashOnBrainDamage(bool newValue) { m_FlashOnBrainDamage = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EnableHats
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether random hats are enabled or not.
// Arguments:       None.
// Return value:    Whether random hats are enabled or not.

    bool EnableHats() const { return m_EnableHats; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  BlipOnRevealUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we need to play blips when unseen layer is revealed.
// Arguments:       None.
// Return value:    Whether we need to play blips when unseen layer is revealed.

    bool BlipOnRevealUnseen() const { return m_BlipOnRevealUnseen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetBlipOnRevealUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether we need to play blips when unseen layer is revealed.
// Arguments:       New value for Blip on reveal unseen option
// Return value:    None.

	void SetBlipOnRevealUnseen(bool newValue) { m_BlipOnRevealUnseen = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ShowMetaScenes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we need to show metascenes in editors and scenario UI
// Arguments:       None.
// Return value:    True if we need to show metascenes

	bool ShowMetascenes() const { return m_ShowMetascenes; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ShowForeignItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we need to show items from other factions in buy GUI
// Arguments:       None.
// Return value:    True if we need to show foreign items

	bool ShowForeignItems() const { return m_ShowForeignItems; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetShowForeignItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set whether we need to show items from other factions in buy GUI
// Arguments:       If we need to show foreign items.
// Return value:    None.

	void SetShowForeignItems(bool newValue) { m_ShowForeignItems = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  PreciseCollisions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether CC uses additional Draws during MO's PreTravel and PostTravel to 
//					update MO layer this frame with more precision(true), or it just uses data from the last frame with less precision(false)
// Arguments:       None.
// Return value:    True if collisions are precise

	bool PreciseCollisions() const { return m_PreciseCollisions; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  PreciseCollisions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets, whether CC uses additional Draws during MO's PreTravel and PostTravel to 
//					update MO layer this frame with more precision(true), or it just uses data from the last frame with less precision(false)
// Arguments:       None.
// Return value:    None

	void SetPreciseCollisions(bool newValue) { m_PreciseCollisions = newValue; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetVisibleAssemblyGroupsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the list of visible assembly groups.
// Arguments:       None.
// Return value:    List of visible assembly groups.

	std::list<string> GetVisibleAssemblyGroupsList() const { return m_VisibleAssemblyGroupsList; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetDisabledModsList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns map of mods which were disabled.
// Arguments:       None.
// Return value:    Map of mods wich were disabled.

	std::map<std::string, bool> GetDisabledModsList() const {return m_DisabledMods; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			EnableMod
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified mod to internal list of disabled mods.
// Arguments:       Mod to enable.
// Return value:    None.

	void EnableMod(string modModule) { std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower); m_DisabledMods[modModule] = false; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			DisableMod
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified mod to internal list of installed workshop mods.
// Arguments:       Mod to disable.
// Return value:    None.

	void DisableMod(string modModule) { std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower); m_DisabledMods[modModule] = true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsModEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified mod is not disabled in the settings.
// Arguments:       Mod to check.
// Return value:    Whether the mod is disabled via settings.

	bool IsModDisabled(string modModule);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			EnableScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified mod to internal list of disabled mods.
// Arguments:       Mod to enable.
// Return value:    None.

	void EnableScript(string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = true; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			DisableScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds specified mod to internal list of installed workshop mods.
// Arguments:       Mod to disable.
// Return value:    None.

	void DisableScript(string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = false; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsScriptEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if specified script is enabled in the settings.
// Arguments:       Mod to check.
// Return value:    Whether the script is enabled via settings.

	bool IsScriptEnabled(string scriptName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			EndlessMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if endless metagame mode is enabled.
// Arguments:       None.
// Return value:    Whether endless mode is enabled via settings.

	bool EndlessMode() const { return m_EndlessMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			PrintDebugInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if debug inf mode is set.
// Arguments:       None.
// Return value:    Whether we need to print some debug info.

	bool PrintDebugInfo() const { return m_PrintDebugInfo; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			RecommendedMOIDCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the recommended MOID count. If this amount is exceeded then 
//					some unit may be removed at the start of the activity
// Arguments:       None.
// Return value:    Recommended MOID count.

	int RecommendedMOIDCount() const { return m_RecommendedMOIDCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			SetPrintDebugInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets print debug info mode.
// Arguments:       New debug print mode value.
// Return value:    None.

	void SetPrintDebugInfo(bool printDebugInfo) { m_PrintDebugInfo = printDebugInfo; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceSoftwareGfxDriver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we're told to use software driver via settings
//					Windows: GFX_DIRECTX_SOFT
// Arguments:       None.
// Return value:    true if forced to use software driver.

	bool ForceSoftwareGfxDriver() const { return m_ForceSoftwareGfxDriver; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceSafeGfxDriver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we're told to use safe driver via settings. Overrides software driver.
//					Windows: GFX_DIRECTX_SAFE
// Arguments:       None.
// Return value:    true if forced to use software driver.

	bool ForceSafeGfxDriver() const { return m_ForceSafeGfxDriver; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceVirtualFullScreenGfxDriver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we're told to use windowed driver with borderless style driver via settings.
//					Overrides any other windowed drivers
//					Windows: GFX_DIRECTX_WIN_BORDERLESS
// Arguments:       None.
// Return value:    true if forced to use software driver.

	bool ForceVirtualFullScreenGfxDriver() const { return m_ForceVirtualFullScreenGfxDriver; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceOverlayedWindowGfxDriver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we're told to use non overlapped windowed driver.
//					Windows: GFX_DIRECT_OVL
// Arguments:       None.
// Return value:    true if forced to use software driver.

	bool ForceOverlayedWindowGfxDriver() const { return m_ForceOverlayedWindowGfxDriver; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			ForceNonOverlayedWindowGfxDriver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether we're told to use non-overlay driver. Overrides overlayed driver setting.
//					Windows
// Arguments:       None.
// Return value:    true if forced to use software driver.

	bool ForceNonOverlayedWindowGfxDriver() const { return m_ForceNonOverlayedWindowGfxDriver; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			AllowSavingToBase
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether editors will allow to select Base.rte as a module to save in
// Arguments:       None.
// Return value:    true of editors are allowed to select Base.rte as a module to save in

	bool AllowSavingToBase() const { return m_AllowSavingToBase; }


	std::string GetPlayerNetworkName() const { return m_PlayerNetworkName;  }

	void SetPlayerNetworkName(std::string newName) { m_PlayerNetworkName = newName; }

	std::string GetNetworkServerName() const { return m_NetworkServerName; }

	void SetNetworkServerName(std::string newName) { m_NetworkServerName = newName; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  

	bool GetServerUseHighCompression() const { return m_ServerUseHighCompression; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  

	bool GetServerUseFastCompression() const { return m_ServerUseFastCompression; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  

	int GetServerHighCompressionLevel() const { return m_ServerHighCompressionLevel; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			GetServerFastAccelerationFactor
	//////////////////////////////////////////////////////////////////////////////////////////
	//  The larger the acceleration value, the faster the algorithm, but also the lesser the compression.
	//	It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed.
	//	An acceleration value of "1" is the same as regular LZ4_compress_default()
	//	Values <= 0 will be replaced by ACCELERATION_DEFAULT(currently == 1, see lz4.c).
	int GetServerFastAccelerationFactor() const { return m_ServerFastAccelerationFactor; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	bool GetServerUseInterlacing() const { return m_ServerUseInterlacing; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	int GetServerEncodingFps() const { return m_ServerEncodingFps; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	int GetClientInputFps() const { return m_ClientInputFps; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	bool GetServerTransmitAsBoxes() const { return m_ServerTransmitAsBoxes; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	int GetServerBoxWidth() const { return m_ServerBoxWidth; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:			_
	//////////////////////////////////////////////////////////////////////////////////////////
	//  
	int GetServerBoxHeight() const { return m_ServerBoxHeight; }

	bool GetUseNATService() { return m_UseNATService; }

	std::string & GetNATServiceAddress() { return m_NATServiceAddress; }

	void SetNATServiceAddress(std::string newValue) { m_NATServiceAddress = newValue; }

	std::string & GetNATServerName() { return m_NATServerName; }

	void SetNATServerName(std::string newValue) { m_NATServerName = newValue; }

	std::string & GetNATServerPassword() { return m_NATServerPassword; }

	void SetNATServerPassword(std::string newValue) { m_NATServerPassword = newValue; }

	bool GetServerSleepWhenIdle() { return m_ServerSleepWhenIdle; }

	bool GetServerSimSleepWhenIdle() { return m_ServerSimSleepWhenIdle; }
	
	bool DisableLoadingScreen() { return m_DisableLoadingScreen; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

    // Whether to play the intro of the game
    bool m_PlayIntro;
    // Whether ToolTips are enabled or not
    bool m_ToolTips;
	// Whether red flashes on brain damage are on or off
	bool m_FlashOnBrainDamage;
	// Hats enabled
	bool m_EnableHats;
	// Blip if unseen is revealed
	bool m_BlipOnRevealUnseen;
	// Show metascenes in editors and activities
	bool m_ShowMetascenes;
	// Do not show foreign items in buy menu
	bool m_ShowForeignItems;
	// Endless metagame mode
	bool m_EndlessMode;
	// Print some debug info in console
	bool m_PrintDebugInfo;
	//Whether CC uses additional Draws during MO's PreTravel and PostTravel to 
	//update MO layer this frame with more precision(true), or it just uses data from the last frame with less precision(false)
	bool m_PreciseCollisions;
	// Whether we should try using software-mode drivers
	bool m_ForceSoftwareGfxDriver;
	// Whether we should try using safe-mode drivers
	bool m_ForceSafeGfxDriver;
	// Whether we should try using fullscreen mode
	bool m_ForceVirtualFullScreenGfxDriver;
	// Whether we should try using overlayed window driver
	bool m_ForceOverlayedWindowGfxDriver;
	// Whether we should try using non-overlayd window driver
	bool m_ForceNonOverlayedWindowGfxDriver;
	// Whether editors will allow to select Base.rte as a module to save in
	bool m_AllowSavingToBase;
	// Recommended max MOID's before removing actors from scenes
	int m_RecommendedMOIDCount;

	std::string m_PlayerNetworkName;

	std::string m_NetworkServerName;

	bool m_ServerUseHighCompression;

	bool m_ServerUseFastCompression;

	int m_ServerHighCompressionLevel;

	int m_ServerFastAccelerationFactor;

	bool m_ServerUseInterlacing;

	int m_ServerEncodingFps;

	int m_ClientInputFps;

	bool m_ServerTransmitAsBoxes;

	int m_ServerBoxWidth;

	int m_ServerBoxHeight;

	bool m_UseNATService;

	std::string m_NATServiceAddress;

	std::string m_NATServerName;

	std::string m_NATServerPassword;

	bool m_ServerSleepWhenIdle;

	bool m_ServerSimSleepWhenIdle;

	bool m_DisableLoadingScreen;

    // List of assemblies groups always shown in editors
    std::list<std::string> m_VisibleAssemblyGroupsList;
    // List of the module names we disabled
    std::map<std::string, bool> m_DisabledMods;
    // List of the script names we enabled
    std::map<std::string, bool> m_EnabledScripts;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SettingsMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    SettingsMan(const SettingsMan &reference);
    SettingsMan & operator=(const SettingsMan &rhs);

};

} // namespace RTE

#endif // File