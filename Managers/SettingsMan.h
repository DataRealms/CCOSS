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

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMan object in system memory. Create() should be called before using the object.
		/// </summary>
		SettingsMan() { Clear(); }

		/// <summary>
		/// Makes the SettingsMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create() { return (Serializable::Create() < 0) ? -1 : 0; }

		/// <summary>
		/// Makes the Serializable ready for use.
		/// </summary>
		/// <param name="reader">A Reader that the Serializable will create itself from.</param>
		/// <param name="checkType">Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.</param>
		/// <param name="doCreate">Whether to do any additional initialization of the object after reading in all the properties from the Reader.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true);
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
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
		/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
		/// </summary>
		/// <param name="propName">The name of the property to be read.</param>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>
		/// An error return value signaling whether the property was successfully read or not.
		/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
		/// </returns>
		virtual int ReadProperty(std::string propName, Reader &reader);

		/// <summary>
		/// Saves the complete state of this SettingsMan to an output stream for later recreation with Create(Reader &reader);
		/// </summary>
		/// <param name="writer">A Writer that the SettingsMan will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool SettingsNeedOverwrite() const { return m_SettingsNeedOverwrite; }
#pragma endregion

#pragma region Engine Settings
		/// <summary>
		/// Returns the recommended MOID count. If this amount is exceeded then some unit may be removed at the start of the activity.
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
		/// Whether we're told to use windowed driver with borderless style driver via settings. Overrides any other windowed drivers. Windows: GFX_DIRECTX_WIN_BORDERLESS.
		/// </summary>
		/// <returns>True if forced to use software driver.</returns>
		bool ForceVirtualFullScreenGfxDriver() const { return m_ForceVirtualFullScreenGfxDriver; }

		/// <summary>
		/// Whether we're told to use non overlapped windowed driver. Windows: GFX_DIRECT_OVL.
		/// </summary>
		/// <returns>True if forced to use software driver.</returns>
		bool ForceOverlayedWindowGfxDriver() const { return m_ForceOverlayedWindowGfxDriver; }

		/// <summary>
		/// Whether we're told to use non-overlay driver. Overrides overlayed driver setting.
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
		/// Whether random hats on actors are enabled or not.
		/// </summary>
		/// <returns>Whether random hats are enabled or not.</returns>
		bool EnableHats() const { return m_EnableHats; }
#pragma endregion

#pragma region Network Settings
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::string GetPlayerNetworkName() const { return m_PlayerNetworkName; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newName"></param>
		void SetPlayerNetworkName(std::string newName) { m_PlayerNetworkName = (newName == "") ? "Dummy" : newName; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::string GetNetworkServerName() const { return m_NetworkServerName; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newName"></param>
		void SetNetworkServerName(std::string newName) { m_NetworkServerName = (newName == "") ? "127.0.0.1:8000" : newName; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::string & GetNATServiceAddress() { return m_NATServiceAddress; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newValue"></param>
		void SetNATServiceAddress(std::string newValue) { m_NATServiceAddress = (newValue == "") ? "127.0.0.1:61111" : newValue; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::string & GetNATServerName() { return m_NATServerName; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newValue"></param>
		void SetNATServerName(std::string newValue) { m_NATServerName = (newValue == "") ? "DefaultServerName" : newValue; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		std::string & GetNATServerPassword() { return m_NATServerPassword; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newValue"></param>
		void SetNATServerPassword(std::string newValue) { m_NATServerPassword = (newValue == "") ? "DefaultServerPassword" : newValue; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetServerUseHighCompression() const { return m_ServerUseHighCompression; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetServerUseFastCompression() const { return m_ServerUseFastCompression; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		int GetServerHighCompressionLevel() const { return m_ServerHighCompressionLevel; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		/// <remarks>
		/// The larger the acceleration value, the faster the algorithm, but also lesser the compression. It's a trade-off. It can be fine tuned, with each successive value providing roughly +~3% to speed. 
		/// An acceleration value of "1" is the same as regular LZ4_compress_default(). Values <= 0 will be replaced by ACCELERATION_DEFAULT(currently == 1, see lz4.c).
		/// </remarks>
		int GetServerFastAccelerationFactor() const { return m_ServerFastAccelerationFactor; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetServerUseInterlacing() const { return m_ServerUseInterlacing; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned short GetServerEncodingFps() const { return m_ServerEncodingFps; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned short GetClientInputFps() const { return m_ClientInputFps; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetServerTransmitAsBoxes() const { return m_ServerTransmitAsBoxes; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned short GetServerBoxWidth() const { return m_ServerBoxWidth; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned short GetServerBoxHeight() const { return m_ServerBoxHeight; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetUseNATService() { return m_UseNATService; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool GetServerSleepWhenIdle() { return m_ServerSleepWhenIdle; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
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
		void EnableScript(string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = true; }

		/// <summary>
		/// Adds specified script to internal list of installed scripts.
		/// </summary>
		/// <param name="scriptName">Script to disable.</param>
		void DisableScript(string scriptName) { std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower); m_EnabledScripts[scriptName] = false; }

		/// <summary>
		/// Returns true if specified script is enabled in the settings.
		/// </summary>
		/// <param name="scriptName">Mod to check.</param>
		/// <returns>Whether the script is enabled via settings.</returns>
		bool IsScriptEnabled(string scriptName);
#pragma endregion

#pragma region Misc Settings
		/// <summary>
		/// Is app set to play the game intro on startup?
		/// </summary>
		/// <returns>Whether should play intro or not.</returns>
		bool PlayIntro() const { return m_PlayIntro; }

		/// <summary>
		/// Is app set to play the game intro on startup?
		/// </summary>
		/// <param name="play">Whether should play intro or not.</param>
		void SetPlayIntro(bool play) { m_PlayIntro = play; }

		/// <summary>
		/// Is app set to display tools tips on certain UI elements?
		/// </summary>
		/// <returns>Whether should show tool tips or not.</returns>
		bool ToolTips() const { return m_ToolTips; }

		/// <summary>
		/// Sets whether app set to display tools tips on certain UI elements.
		/// </summary>
		/// <param name="showToolTips">Whether should show tool tips or not.</param>
		void SetShowToolTips(bool showToolTips) { m_ToolTips = showToolTips; }

		/// <summary>
		/// Returns true if debug inf mode is set.
		/// </summary>
		/// <returns>Whether we need to print some debug info.</returns>
		bool PrintDebugInfo() const { return m_PrintDebugInfo; }

		/// <summary>
		/// Sets print debug info mode.
		/// </summary>
		/// <param name="printDebugInfo">New debug print mode value.</param>
		void SetPrintDebugInfo(bool printDebugInfo) { m_PrintDebugInfo = printDebugInfo; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool DisableLoadingScreen() { return m_DisableLoadingScreen; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		unsigned short LoadingScreenReportPrecision() const { return m_LoadingScreenReportPrecision; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		float GetMenuTransitionSpeed() const { return m_MenuTransitionSpeed; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newSpeed"></param>
		void SetMenuTransitionSpeed(float newSpeed) { m_MenuTransitionSpeed = (newSpeed >= 0.0F) ? newSpeed : 0.0F; }
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return m_ClassName; }
#pragma endregion

	protected:

		static const std::string m_ClassName; //!< A string with the friendly-formatted type name of this.

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
		bool m_EndlessMode; //!< Endless metagame mode.
		bool m_EnableHats; //!< Hats enabled.

		std::string m_PlayerNetworkName; //!<
		std::string m_NetworkServerName; //!<
		bool m_UseNATService; //!<
		std::string m_NATServiceAddress; //!<
		std::string m_NATServerName; //!<
		std::string m_NATServerPassword; //!<
		unsigned short m_ClientInputFps; //!<
		bool m_ServerTransmitAsBoxes; //!<
		unsigned short m_ServerBoxWidth; //!<
		unsigned short m_ServerBoxHeight; //!<
		bool m_ServerUseHighCompression; //!<
		bool m_ServerUseFastCompression; //!<
		int m_ServerHighCompressionLevel; //!<
		int m_ServerFastAccelerationFactor; //!<
		bool m_ServerUseInterlacing; //!<
		unsigned short m_ServerEncodingFps; //!<
		bool m_ServerSleepWhenIdle; //!<
		bool m_ServerSimSleepWhenIdle; //!<

		bool m_AllowSavingToBase; //!< Whether editors will allow to select Base.rte as a module to save in.
		bool m_ShowMetaScenes; //!< Show MetaScenes in editors and activities.

		unsigned int m_RecommendedMOIDCount; //!< Recommended max MOID's before removing actors from scenes.
		bool m_PreciseCollisions; //!<Whether to use additional Draws during MO's PreTravel and PostTravel to update MO layer this frame with more precision, or just uses data from the last frame with less precision.

		bool m_PlayIntro; //!< Whether to play the intro of the game.	
		bool m_ToolTips; //!< Whether ToolTips are enabled or not.	
		bool m_DisableLoadingScreen; //!<
		unsigned short m_LoadingScreenReportPrecision; //!<
		float m_MenuTransitionSpeed; //!<
		bool m_PrintDebugInfo; //!< Print some debug info in console.

		std::list<std::string> m_VisibleAssemblyGroupsList; //!< List of assemblies groups always shown in editors.
		std::map<std::string, bool> m_DisabledMods; //!< List of the module names we disabled.
		std::map<std::string, bool> m_EnabledScripts; //!< List of the script names we enabled.

	private:

		/// <summary>
		/// Writes the minimal default settings needed for the game to run to an output stream. These will be overwritten with the full list of available settings as soon as the game beings loading.
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