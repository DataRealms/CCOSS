#ifndef _RTESETTINGSMAN_
#define _RTESETTINGSMAN_

#include "Serializable.h"
#include "Singleton.h"

#define g_SettingsMan SettingsMan::Instance()

namespace RTE {

	/// <summary>
	/// The singleton manager over the application and misc settings.
	/// </summary>
	class SettingsMan : public Singleton<SettingsMan>, public Serializable {

	public:

		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMan object in system memory. Create() should be called before using the object.
		/// </summary>
		SettingsMan() { Clear(); }

		/// <summary>
		/// Makes the SettingsMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return Serializable::Create(); }

		/// <summary>
		/// Makes the SettingsMan object ready for use.
		/// </summary>
		/// <param name="reader">A Reader that the SettingsMan will create itself from.</param>
		/// <param name="checkType">Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.</param>
		/// <param name="doCreate">Whether to do any additional initialization of the object after reading in all the properties from the Reader.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Reader &reader, bool checkType = true, bool doCreate = true) override;
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SettingsMan object before deletion from system memory.
		/// </summary>
		~SettingsMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SettingsMan object.
		/// </summary>
		void Destroy() { Clear(); }

		/// <summary>
		/// Resets the entire SettingsMan, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Settings Manager Operations
		/// <summary>
		/// Gets whether Settings.ini needs to be overwritten with the complete list of settings or not. Will be true only if Settings.ini was created with default values on first load or after settings delete.
		/// </summary>
		/// <returns>Whether Settings.ini needs to be overwritten with the complete list of settings or not.</returns>
		bool SettingsNeedOverwrite() const { return m_SettingsNeedOverwrite; }

		/// <summary>
		/// Overwrites the settings file to save changes made from within the game.
		/// </summary>
		void UpdateSettingsFile();
#pragma endregion

#pragma region Engine Settings
		/// <summary>
		/// Returns the recommended MOID count. If this amount is exceeded then some units may be removed at the start of the activity.
		/// </summary>
		/// <returns>Recommended MOID count.</returns>
		unsigned int RecommendedMOIDCount() const { return m_RecommendedMOIDCount; }

		/// <summary>
		/// Gets whether we're using additional Draws during MO's PreTravel and PostTravel to update MO layer this frame with more precision, or just data from the last frame with less precision.
		/// </summary>
		/// <returns>Whether precise collisions are enabled.</returns>
		bool PreciseCollisions() const { return m_PreciseCollisions; }

		/// <summary>
		/// Sets whether to use additional Draws during MO's PreTravel and PostTravel to update MO layer this frame with more precision, or just data from the last frame with less precision.
		/// </summary>
		/// <param name="newValue">True for precise collisions.</param>
		void SetPreciseCollisions(bool newValue) { m_PreciseCollisions = newValue; }
#pragma endregion

#pragma region Display Settings
		/// <summary>
		/// Whether we're told to use windowed driver with borderless style driver via settings. Overrides any other windowed drivers. The driver that will be used is GFX_DIRECTX_WIN_BORDERLESS.
		/// </summary>
		/// <returns>True if forced to use software driver.</returns>
		bool ForceVirtualFullScreenGfxDriver() const { return m_ForceVirtualFullScreenGfxDriver; }

		/// <summary>
		/// Whether we're told to use non overlapped windowed driver. The driver that will be used is GFX_DIRECTX_OVL.
		/// </summary>
		/// <returns>True if forced to use software driver.</returns>
		bool ForceOverlayedWindowGfxDriver() const { return m_ForceOverlayedWindowGfxDriver; }

		/// <summary>
		/// Whether we're told to use non-overlay driver. Overrides overlayed driver setting. The driver that will be used is GFX_DIRECTX_WIN.
		/// </summary>
		/// <returns>True if forced to use software driver.</returns>
		bool ForceNonOverlayedWindowGfxDriver() const { return m_ForceNonOverlayedWindowGfxDriver; }
#pragma endregion

#pragma region Audio Settings
		/// <summary>
		/// The strength of the sound panning effect.
		/// </summary>
		/// <returns>0 - 1, where 0 is no panning and 1 is fully panned.</returns>
		float SoundPanningEffectStrength() const { return m_SoundPanningEffectStrength; }

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		float c_ListenerZOffset() const { return m_ListenerZOffset; }
		float c_MinimumDistanceForPanning() const { return m_MinimumDistanceForPanning; }
		//////////////////////////////////////////////////
#pragma endregion

#pragma region Gameplay Settings
		/// <summary>
		/// Returns true if endless metagame mode is enabled.
		/// </summary>
		/// <returns>Whether endless mode is enabled via settings.</returns>
		bool EndlessMode() const { return m_EndlessMode; }

		/// <summary>
		/// Whether we need to play blips when unseen layer is revealed.
		/// </summary>
		/// <returns>Whether we need to play blips when unseen layer is revealed.</returns>
		bool BlipOnRevealUnseen() const { return m_BlipOnRevealUnseen; }

		/// <summary>
		/// Sets whether we need to play blips when unseen layer is revealed.
		/// </summary>
		/// <param name="newValue">New value for Blip on reveal unseen option.</param>
		void SetBlipOnRevealUnseen(bool newValue) { m_BlipOnRevealUnseen = newValue; }

		/// <summary>
		/// Whether red and white flashes appear when brain is damaged.
		/// </summary>
		/// <returns>Whether red and white flashes appear when brain is damaged.</returns>
		bool FlashOnBrainDamage() const { return m_FlashOnBrainDamage; }

		/// <summary>
		/// Sets whether red and white flashes appear when brain is damaged.
		/// </summary>
		/// <param name="newValue">New value for Flash on brain damage setting.</param>
		void SetFlashOnBrainDamage(bool newValue) { m_FlashOnBrainDamage = newValue; }

		/// <summary>
		/// Whether we need to show items from other factions in buy menu GUI.
		/// </summary>
		/// <returns>True if we need to show foreign items.</returns>
		bool ShowForeignItems() const { return m_ShowForeignItems; }

		/// <summary>
		/// Set whether we need to show items from other factions in buy menu GUI.
		/// </summary>
		/// <param name="newValue">If we need to show foreign items.</param>
		void SetShowForeignItems(bool newValue) { m_ShowForeignItems = newValue; }

		/// <summary>
		/// Whether randomized hat attachables will be attached to all AHuman actors.
		/// </summary>
		/// <returns>Whether random hats are enabled or not.</returns>
		bool EnableHats() const { return m_EnableHats; }

		/// <summary>
		/// Gets whether the crab bomb effect is enabled or not. False means releasing whatever number of crabs will do nothing except release a whatever number of crabs.
		/// </summary>
		/// <returns>Whether the crab bomb effect is enabled or not.</returns>
		bool EnableCrabBombs() const { return m_EnableCrabBombs; }

		/// <summary>
		/// Gets the number of crabs needed to be released at once to trigger the crab bomb effect.
		/// </summary>
		/// <returns>The number of crabs needed to be released at once to trigger the crab bomb effect.</returns>
		unsigned short CrabBombThreshold() const { return m_CrabBombThreshold; }
#pragma endregion

#pragma region Default Activity Settings
		/// <summary>
		/// Gets whether the intro and main menu should be skipped on game start and launch directly into the set default activity instead.
		/// </summary>
		/// <returns>Whether the game is set to launch directly into the set default activity or not.</returns>
		bool LaunchIntoActivity() const { return m_LaunchIntoActivity; }
#pragma endregion

#pragma region Network Settings
		/// <summary>
		/// Gets the player name that is used in network multiplayer matches.
		/// </summary>
		/// <returns>String with the network player name.</returns>
		std::string GetPlayerNetworkName() const { return m_PlayerNetworkName; }

		/// <summary>
		/// Sets the player name that will be used in network multiplayer matches.
		/// </summary>
		/// <param name="newName">String with the new player name to use.</param>
		void SetPlayerNetworkName(std::string newName) { m_PlayerNetworkName = newName.empty() ? "Dummy" : newName; }

		/// <summary>
		/// Gets the LAN server address to connect to.
		/// </summary>
		/// <returns>The current LAN server address to connect to.</returns>
		std::string GetNetworkServerAddress() const { return m_NetworkServerAddress; }

		/// <summary>
		/// Sets the LAN server address to connect to.
		/// </summary>
		/// <param name="newName">New LAN server address to connect to.</param>
		void SetNetworkServerAddress(std::string newAddress) { m_NetworkServerAddress = newAddress.empty() ? "127.0.0.1:8000" : newAddress; }

		/// <summary>
		/// Gets the NAT punch-through server address.
		/// </summary>
		/// <returns>The current NAT punch-through server address to connect to.</returns>
		std::string & GetNATServiceAddress() { return m_NATServiceAddress; }

		/// <summary>
		/// Sets the NAT punch-through server address.
		/// </summary>
		/// <param name="newValue">New NAT punch-through server address to connect to.</param>
		void SetNATServiceAddress(std::string newAddress) { m_NATServiceAddress = newAddress.empty() ? "127.0.0.1:61111" : newAddress; }

		/// <summary>
		/// Gets the server name used when connecting via NAT punch-through service.
		/// </summary>
		/// <returns>Name of the NAT punch-through server.</returns>
		std::string & GetNATServerName() { return m_NATServerName; }

		/// <summary>
		/// Sets the server name to use when connecting via NAT punch-through service.
		/// </summary>
		/// <param name="newValue">New NAT punch-through server name.</param>
		void SetNATServerName(std::string newName) { m_NATServerName = newName.empty() ? "DefaultServerName" : newName; }

		/// <summary>
		/// Gets the server password to use when connecting via NAT punch-through service.
		/// </summary>
		/// <returns>The server password to use when connecting via NAT punch-through service.</returns>
		std::string & GetNATServerPassword() { return m_NATServerPassword; }

		/// <summary>
		/// Sets the server password to use when connecting via NAT punch-through service.
		/// </summary>
		/// <param name="newValue">New password to use when connecting via NAT punch-through service.</param>
		void SetNATServerPassword(std::string newValue) { m_NATServerPassword = newValue.empty() ? "DefaultServerPassword" : newValue; }

		/// <summary>
		/// Gets whether server is using higher compression methods.
		/// </summary>
		/// <returns>Whether server is using higher compression methods or not.</returns>
		bool GetServerUseHighCompression() const { return m_ServerUseHighCompression; }

		/// <summary>
		/// Gets whether server is using faster compression methods.
		/// </summary>
		/// <returns>Whether server is using faster compression methods or not.</returns>
		bool GetServerUseFastCompression() const { return m_ServerUseFastCompression; }

		/// <summary>
		/// Gets the compression level used by the server when in high compressing mode.
		/// </summary>
		/// <returns>The compression level currently used by the server.</returns>
		int GetServerHighCompressionLevel() const { return m_ServerHighCompressionLevel; }

		/// <summary>
		/// Gets the server acceleration factor, higher values consume more bandwidth but less CPU.
		/// </summary>
		/// <returns>The acceleration factor currently used by the server.</returns>
		int GetServerFastAccelerationFactor() const { return m_ServerFastAccelerationFactor; }

		/// <summary>
		/// Gets whether server is using interlacing to reduce bandwidth usage.
		/// </summary>
		/// <returns>Whether server uses interlacing or not.</returns>
		bool GetServerUseInterlacing() const { return m_ServerUseInterlacing; }

		/// <summary>
		/// Gets the server frame transmission rate.
		/// </summary>
		/// <returns>The server frame transmission rate.</returns>
		unsigned short GetServerEncodingFps() const { return m_ServerEncodingFps; }

		/// <summary>
		/// Gets the input send rate between the client and the server.
		/// </summary>
		/// <returns>The input send rate.</returns>
		unsigned short GetClientInputFps() const { return m_ClientInputFps; }

		/// <summary>
		/// Gets whether the server transmits frames as blocks instead of lines.
		/// </summary>
		/// <returns>Whether the server transmits frames as blocks instead of lines or not.</returns>
		bool GetServerTransmitAsBoxes() const { return m_ServerTransmitAsBoxes; }

		/// <summary>
		/// Gets the width of the transmitted block when transmitting frames as blocks.
		/// </summary>
		/// <returns>The width of the transmitted block.</returns>
		unsigned short GetServerBoxWidth() const { return m_ServerBoxWidth; }

		/// <summary>
		/// Gets the height of the transmitted block when transmitting frames as blocks.
		/// </summary>
		/// <returns>The height of the transmitted block.</returns>
		unsigned short GetServerBoxHeight() const { return m_ServerBoxHeight; }

		/// <summary>
		/// Gets whether a NAT service is used for punch-through.
		/// </summary>
		/// <returns>Whether a NAT service is used for punch-through or not.</returns>
		bool GetUseNATService() { return m_UseNATService; }

		/// <summary>
		/// Gets whether server puts threads to sleep if it didn't receive anything for 10 seconds to reduce CPU load.
		/// </summary>
		/// <returns>Whether threads will be put to sleep when server isn't receiving any data or not.</returns>
		bool GetServerSleepWhenIdle() { return m_ServerSleepWhenIdle; }

		/// <summary>
		/// Gets whether the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.
		/// </summary>
		/// <returns>Whether threads will be put to sleep if server completed frame faster than it normally should or not.</returns>
		bool GetServerSimSleepWhenIdle() { return m_ServerSimSleepWhenIdle; }
#pragma endregion

#pragma region Editor Settings
		/// <summary>
		/// Returns the list of visible assembly groups.
		/// </summary>
		/// <returns>List of visible assembly groups.</returns>
		std::list<std::string> GetVisibleAssemblyGroupsList() const { return m_VisibleAssemblyGroupsList; }

		/// <summary>
		/// Whether editors will allow to select Base.rte as a module to save in
		/// </summary>
		/// <returns>True of editors are allowed to select Base.rte as a module to save in.</returns>
		bool AllowSavingToBase() const { return m_AllowSavingToBase; }

		/// <summary>
		/// Whether we need to show MetaScenes in editors and scenario UI.
		/// </summary>
		/// <returns>True if we need to show MetaScenes.</returns>
		bool ShowMetascenes() const { return m_ShowMetaScenes; }
#pragma endregion

#pragma region Mod Manager
		/// <summary>
		/// Returns map of mods which were disabled.
		/// </summary>
		/// <returns>Map of mods which were disabled.</returns>
		std::map<std::string, bool> GetDisabledModsList() const { return m_DisabledMods; }

		/// <summary>
		/// Adds specified mod to internal list of disabled mods.
		/// </summary>
		/// <param name="modModule">Mod to enable.</param>
		void EnableMod(std::string modModule) { std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower); m_DisabledMods[modModule] = false; }

		/// <summary>
		/// Adds specified mod to internal list of installed workshop mods.
		/// </summary>
		/// <param name="modModule">Mod to disable.</param>
		void DisableMod(std::string modModule) { std::transform(modModule.begin(), modModule.end(), modModule.begin(), ::tolower); m_DisabledMods[modModule] = true; }

		/// <summary>
		/// Returns true if specified mod is not disabled in the settings.
		/// </summary>
		/// <param name="modModule">Mod to check.</param>
		/// <returns>Whether the mod is disabled via settings.</returns>
		bool IsModDisabled(std::string modModule);
#pragma endregion

#pragma region Global Script Manager
		/// <summary>
		/// Adds specified mod to internal list of disabled mods.
		/// </summary>
		/// <param name="scriptName">Mod to enable.</param>
		void EnableScript(std::string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = true; }

		/// <summary>
		/// Adds specified script to internal list of installed scripts.
		/// </summary>
		/// <param name="scriptName">Script to disable.</param>
		void DisableScript(std::string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = false; }

		/// <summary>
		/// Returns true if specified script is enabled in the settings.
		/// </summary>
		/// <param name="scriptName">Mod to check.</param>
		/// <returns>Whether the script is enabled via settings.</returns>
		bool IsScriptEnabled(std::string scriptName);
#pragma endregion

#pragma region Misc Settings
		/// <summary>
		/// Gets whether the game intro is set to be skipped on game startup or not.
		/// </summary>
		/// <returns>Whether intro is set to be skipped or not.</returns>
		bool SkipIntro() const { return m_SkipIntro; }

		/// <summary>
		/// Sets whether the game intro should be skipped on game startup or not.
		/// </summary>
		/// <param name="play">Whether to skip game intro or not.</param>
		void SetSkipIntro(bool play) { m_SkipIntro = play; }

		/// <summary>
		/// Gets whether tooltip display on certain UI elements is enabled or not.
		/// </summary>
		/// <returns>Whether tooltips are displayed or not.</returns>
		bool ToolTips() const { return m_ToolTips; }

		/// <summary>
		/// Sets whether to display tooltips on certain UI elements or not.
		/// </summary>
		/// <param name="showToolTips">Whether to display tooltips or not.</param>
		void SetShowToolTips(bool showToolTips) { m_ToolTips = showToolTips; }

		/// <summary>
		/// Gets whether debug print mode is enabled or not.
		/// </summary>
		/// <returns>Whether debug print mode is enabled or not.</returns>
		bool PrintDebugInfo() const { return m_PrintDebugInfo; }

		/// <summary>
		/// Sets print debug info mode.
		/// </summary>
		/// <param name="printDebugInfo">New debug print mode value.</param>
		void SetPrintDebugInfo(bool printDebugInfo) { m_PrintDebugInfo = printDebugInfo; }

		/// <summary>
		/// Gets whether the reader progress report is being displayed during module loading or not.
		/// </summary>
		/// <returns>Whether the reader progress report is being displayed during module loading or not.</returns>
		bool DisableLoadingScreen() { return m_DisableLoadingScreen; }

		/// <summary>
		/// Gets how accurately the reader progress report tells what line it's reading during module loading.
		/// </summary>
		/// <returns>How accurately the reader progress report tells what line it's reading during module loading.</returns>
		unsigned short LoadingScreenReportPrecision() const { return m_LoadingScreenReportPrecision; }

		/// <summary>
		/// Gets the multiplier value for the transition durations between different menus.
		/// </summary>
		/// <returns>The multiplier value for the transition durations between different menus. Lower values equal faster transitions.</returns>
		float GetMenuTransitionDurationMultiplier() const { return m_MenuTransitionDurationMultiplier; }

		/// <summary>
		/// Sets the multiplier value for the transition durations between different menus.
		/// </summary>
		/// <param name="newSpeed">New multiplier value for the transition durations between different menus. Lower values equal faster transitions.</param>
		void SetMenuTransitionDurationMultiplier(float newSpeed) { m_MenuTransitionDurationMultiplier = std::max(0.0F, newSpeed); }
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		const std::string & GetClassName() const override { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		bool m_SettingsNeedOverwrite; //!< Whether the settings file was generated with minimal defaults and needs to be overwritten to be fully populated.

		bool m_ForceVirtualFullScreenGfxDriver; //!< Whether we should try using fullscreen mode.
		bool m_ForceOverlayedWindowGfxDriver; //!< Whether we should try using overlayed window driver.
		bool m_ForceNonOverlayedWindowGfxDriver; //!< Whether we should try using non-overlayed window driver.

		float m_SoundPanningEffectStrength; //!< The strength of the sound panning effect, 0 (no panning) - 1 (full panning).

		//////////////////////////////////////////////////
		//TODO These need to be removed when our soundscape is sorted out. They're only here temporarily to allow for easier tweaking by pawnis.
		float m_ListenerZOffset;
		float m_MinimumDistanceForPanning;
		//////////////////////////////////////////////////

		bool m_ShowForeignItems; //!< Do not show foreign items in buy menu.
		bool m_FlashOnBrainDamage; //!< Whether red flashes on brain damage are on or off.
		bool m_BlipOnRevealUnseen; //!< Blip if unseen is revealed.	
		bool m_EndlessMode; //!< Endless MetaGame mode.
		bool m_EnableHats; //!< Whether randomized hat attachables will be attached to all AHuman actors.
		bool m_EnableCrabBombs; //!< Whether all actors (except Brains and Doors) should be annihilated if a number exceeding the crab bomb threshold is released at once.
		unsigned short m_CrabBombThreshold; //!< The number of crabs needed to be released at once to trigger the crab bomb effect.

		std::string m_PlayerNetworkName; //!< Player name used in network multiplayer matches.
		std::string m_NetworkServerAddress; //!< LAN server address to connect to.
		bool m_UseNATService; //!< Whether a NAT service is used for punch-through.
		std::string m_NATServiceAddress; //!< NAT punch-through server address.
		std::string m_NATServerName; //!< Server name to use when connecting via NAT punch-through service.
		std::string m_NATServerPassword; //!< Server password to use when connecting via NAT punch-through service.
		unsigned short m_ClientInputFps; //!< The rate (in FPS) the client input is sent to the server.
		bool m_ServerUseHighCompression; //!< Whether to use higher compression methods (default).
		bool m_ServerUseFastCompression; //!< Whether to use faster compression methods and conserve CPU.
		int m_ServerHighCompressionLevel; //!< Compression level. 10 is optimal, 12 is highest.
		bool m_ServerUseInterlacing; //!< Use interlacing to heavily reduce bandwidth usage at the cost of visual degradation (unusable at 30 fps, but may be suitable at 60 fps).
		unsigned short m_ServerEncodingFps; //!< Frame transmission rate. Higher value equals more CPU and bandwidth consumption.
		bool m_ServerSleepWhenIdle; //!< If true puts thread to sleep if it didn't receive anything for 10 seconds to avoid melting the CPU at 100% even if there are no connections.
		bool m_ServerSimSleepWhenIdle; //!< If true the server will try to put the thread to sleep to reduce CPU load if the sim frame took less time to complete than it should at 30 fps.

		/// <summary>
		/// Acceleration factor, higher values consume more bandwidth but less CPU.
		/// The larger the acceleration value, the faster the algorithm, but also lesser the compression. It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed. 
		/// An acceleration value of "1" is the same as regular LZ4_compress_default(). Values <= 0 will be replaced by ACCELERATION_DEFAULT(currently == 1, see lz4 documentation).
		/// </summary>
		int m_ServerFastAccelerationFactor;

		/// <summary>
		/// Transmit frames as blocks instead of lines. Provides better compression at the cost of higher CPU usage.
		/// Though the compression is quite high it is recommended that Width * Height are less than MTU size or about 1500 bytes or packets may be fragmented by network hardware or dropped completely.
		/// </summary>
		bool m_ServerTransmitAsBoxes;
		unsigned short m_ServerBoxWidth; //!< Width of the transmitted CPU block. Different values may improve bandwidth usage.
		unsigned short m_ServerBoxHeight; //!< Height of the transmitted CPU block. Different values may improve bandwidth usage.

		bool m_AllowSavingToBase; //!< Whether editors will allow to select Base.rte as a module to save in.
		bool m_ShowMetaScenes; //!< Show MetaScenes in editors and activities.

		unsigned int m_RecommendedMOIDCount; //!< Recommended max MOID's before removing actors from scenes.
		bool m_PreciseCollisions; //!<Whether to use additional Draws during MO's PreTravel and PostTravel to update MO layer this frame with more precision, or just uses data from the last frame with less precision.

		bool m_LaunchIntoActivity; //!< Whether to skip the intro and main menu and launch directly into the set default activity instead.

		bool m_SkipIntro; //!< Whether to play the intro of the game or skip directly to the main menu.
		bool m_ToolTips; //!< Whether ToolTips are enabled or not.
		bool m_DisableLoadingScreen; //!< Whether to display the reader progress report during module loading or not. Greatly increases loading speeds when disabled.
		unsigned short m_LoadingScreenReportPrecision; //!< How accurately the reader progress report tells what line it's reading during module loading. Lower values equal more precision at the cost of loading speed.
		float m_MenuTransitionDurationMultiplier; //!< Multiplier value for the transition durations between different menus. Lower values equal faster transitions.
		bool m_PrintDebugInfo; //!< Print some debug info in console.

		std::list<std::string> m_VisibleAssemblyGroupsList; //!< List of assemblies groups always shown in editors.
		std::map<std::string, bool> m_DisabledMods; //!< List of the module names we disabled.
		std::map<std::string, bool> m_EnabledScripts; //!< List of the script names we enabled.

	private:

		/// <summary>
		/// Writes the minimal default settings needed for the game to run to an output stream. These will be overwritten with the full list of available settings as soon as the game begins loading.
		/// </summary>
		/// <param name="writer">A Writer that the SettingsMan will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int WriteDefaultSettings(Writer &writer);

		/// <summary>
		/// Clears all the member variables of this SettingsMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SettingsMan(const SettingsMan &reference) {}
		SettingsMan & operator=(const SettingsMan &rhs) {}
	};
}
#endif