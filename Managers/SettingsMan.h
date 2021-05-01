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

		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsMan object in system memory. Create() should be called before using the object.
		/// </summary>
		SettingsMan() { Clear(); }

		/// <summary>
		/// Makes the SettingsMan object ready for use.
		/// </summary>
		/// <param name="reader">A Reader that the SettingsMan will create itself from.</param>
		/// <param name="checkType">Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.</param>
		/// <param name="doCreate">Whether to do any additional initialization of the object after reading in all the properties from the Reader.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize(Reader &reader, bool checkType = true, bool doCreate = true);
#pragma endregion

#pragma region Destruction
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
		void UpdateSettingsFile() const;
#pragma endregion

#pragma region Engine Settings
		/// <summary>
		/// Returns the recommended MOID count. If this amount is exceeded then some units may be removed at the start of the activity.
		/// </summary>
		/// <returns>Recommended MOID count.</returns>
		int RecommendedMOIDCount() const { return m_RecommendedMOIDCount; }
#pragma endregion

#pragma region Gameplay Settings
		/// <summary>
		/// Returns true if endless MetaGame mode is enabled.
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
		int CrabBombThreshold() const { return m_CrabBombThreshold; }
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
		void SetPlayerNetworkName(const std::string &newName) { m_PlayerNetworkName = newName.empty() ? "Dummy" : newName; }

		/// <summary>
		/// Gets the LAN server address to connect to.
		/// </summary>
		/// <returns>The current LAN server address to connect to.</returns>
		std::string GetNetworkServerAddress() const { return m_NetworkServerAddress; }

		/// <summary>
		/// Sets the LAN server address to connect to.
		/// </summary>
		/// <param name="newName">New LAN server address to connect to.</param>
		void SetNetworkServerAddress(const std::string &newAddress) { m_NetworkServerAddress = newAddress.empty() ? "127.0.0.1:8000" : newAddress; }

		/// <summary>
		/// Gets the NAT punch-through server address.
		/// </summary>
		/// <returns>The current NAT punch-through server address to connect to.</returns>
		std::string & GetNATServiceAddress() { return m_NATServiceAddress; }

		/// <summary>
		/// Sets the NAT punch-through server address.
		/// </summary>
		/// <param name="newValue">New NAT punch-through server address to connect to.</param>
		void SetNATServiceAddress(const std::string &newAddress) { m_NATServiceAddress = newAddress.empty() ? "127.0.0.1:61111" : newAddress; }

		/// <summary>
		/// Gets the server name used when connecting via NAT punch-through service.
		/// </summary>
		/// <returns>Name of the NAT punch-through server.</returns>
		std::string & GetNATServerName() { return m_NATServerName; }

		/// <summary>
		/// Sets the server name to use when connecting via NAT punch-through service.
		/// </summary>
		/// <param name="newValue">New NAT punch-through server name.</param>
		void SetNATServerName(const std::string &newName) { m_NATServerName = newName.empty() ? "DefaultServerName" : newName; }

		/// <summary>
		/// Gets the server password to use when connecting via NAT punch-through service.
		/// </summary>
		/// <returns>The server password to use when connecting via NAT punch-through service.</returns>
		std::string & GetNATServerPassword() { return m_NATServerPassword; }

		/// <summary>
		/// Sets the server password to use when connecting via NAT punch-through service.
		/// </summary>
		/// <param name="newValue">New password to use when connecting via NAT punch-through service.</param>
		void SetNATServerPassword(const std::string &newValue) { m_NATServerPassword = newValue.empty() ? "DefaultServerPassword" : newValue; }
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
		/// Gets whether to draw AtomGroup visualizations or not.
		/// </summary>
		/// <returns>Whether to draw AtomGroup visualizations or not.</returns>
		bool DrawAtomGroupVisualizations() const { return m_DrawAtomGroupVisualizations; }

		/// <summary>
		/// Sets whether to draw AtomGroup visualizations or not.
		/// </summary>
		/// <param name="drawAtomGroupVisualizations">Whether to draw AtomGroup visualizations or not.</param>
		void SetDrawAtomGroupVisualizations(bool drawAtomGroupVisualizations) { m_DrawAtomGroupVisualizations = drawAtomGroupVisualizations; }

		/// <summary>
		/// Gets whether to draw HandGroup and FootGroup visualizations or not.
		/// </summary>
		/// <returns>Whether to draw HandGroup and FootGroup visualizations or not.</returns>
		bool DrawHandAndFootGroupVisualizations() const { return m_DrawHandAndFootGroupVisualizations; }

		/// <summary>
		/// Sets whether to draw HandGroup and FootGroup visualizations or not.
		/// </summary>
		/// <param name="drawHandAndFootGroupVisualizations">Whether to draw HandGroup and FootGroup visualizations or not.</param>
		void SetDrawHandAndFootGroupVisualizations(bool drawHandAndFootGroupVisualizations) { m_DrawHandAndFootGroupVisualizations = drawHandAndFootGroupVisualizations; }

		/// <summary>
		/// Gets whether to draw LimbPath visualizations or not.
		/// </summary>
		/// <returns>Whether to draw LimbPath visualizations or not.</returns>
		bool DrawLimbPathVisualizations() const { return m_DrawLimbPathVisualizations; }

		/// <summary>
		/// Sets whether to draw LimbPath visualizations or not.
		/// </summary>
		/// <param name="drawAtomGroupVisualizations">Whether to draw AtomGroup visualizations or not.</param>
		void SetDrawLimbPathVisualizations(bool drawLimbPathVisualizations) { m_DrawLimbPathVisualizations = drawLimbPathVisualizations; }

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
		bool DisableLoadingScreen() const { return m_DisableLoadingScreen; }

		/// <summary>
		/// Gets how accurately the reader progress report tells what line it's reading during module loading.
		/// </summary>
		/// <returns>How accurately the reader progress report tells what line it's reading during module loading.</returns>
		int LoadingScreenReportPrecision() const { return m_LoadingScreenReportPrecision; }

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

		/// <summary>
		/// Gets whether the duration of module loading (extraction included) should be measured or not. For benchmarking purposes.
		/// </summary>
		/// <returns>Whether duration should be measured or not.</returns>
		bool MeasureModuleLoadTime() const { return m_MeasureModuleLoadTime; }
#pragma endregion

	protected:

		bool m_SettingsNeedOverwrite; //!< Whether the settings file was generated with minimal defaults and needs to be overwritten to be fully populated.

		bool m_ShowForeignItems; //!< Do not show foreign items in buy menu.
		bool m_FlashOnBrainDamage; //!< Whether red flashes on brain damage are on or off.
		bool m_BlipOnRevealUnseen; //!< Blip if unseen is revealed.
		bool m_EndlessMode; //!< Endless MetaGame mode.
		bool m_EnableHats; //!< Whether randomized hat attachables will be attached to all AHuman actors.
		bool m_EnableCrabBombs; //!< Whether all actors (except Brains and Doors) should be annihilated if a number exceeding the crab bomb threshold is released at once.
		int m_CrabBombThreshold; //!< The number of crabs needed to be released at once to trigger the crab bomb effect.

		std::string m_PlayerNetworkName; //!< Player name used in network multiplayer matches.
		std::string m_NetworkServerAddress; //!< LAN server address to connect to.
		std::string m_NATServiceAddress; //!< NAT punch-through server address.
		std::string m_NATServerName; //!< Server name to use when connecting via NAT punch-through service.
		std::string m_NATServerPassword; //!< Server password to use when connecting via NAT punch-through service.

		bool m_AllowSavingToBase; //!< Whether editors will allow to select Base.rte as a module to save in.
		bool m_ShowMetaScenes; //!< Show MetaScenes in editors and activities.

		int m_RecommendedMOIDCount; //!< Recommended max MOID's before removing actors from scenes.

		bool m_LaunchIntoActivity; //!< Whether to skip the intro and main menu and launch directly into the set default activity instead.

		bool m_SkipIntro; //!< Whether to play the intro of the game or skip directly to the main menu.
		bool m_ToolTips; //!< Whether ToolTips are enabled or not.
		bool m_DisableLoadingScreen; //!< Whether to display the reader progress report during module loading or not. Greatly increases loading speeds when disabled.
		int m_LoadingScreenReportPrecision; //!< How accurately the reader progress report tells what line it's reading during module loading. Lower values equal more precision at the cost of loading speed.
		float m_MenuTransitionDurationMultiplier; //!< Multiplier value for the transition durations between different menus. Lower values equal faster transitions.
		
		bool m_DrawAtomGroupVisualizations; //!< Whether to draw MOSRotating AtomGroups to the Scene MO color Bitmap.
		bool m_DrawHandAndFootGroupVisualizations; //!< Whether to draw Actor HandGroups and FootGroups to the Scene MO color Bitmap.
		bool m_DrawLimbPathVisualizations; //!< Whether to draw Actor LimbPaths to the Scene MO color Bitmap.
		bool m_PrintDebugInfo; //!< Print some debug info in console.
		bool m_MeasureModuleLoadTime; //!< Whether to measure the duration of data module loading (extraction included). For benchmarking purposes.

		std::list<std::string> m_VisibleAssemblyGroupsList; //!< List of assemblies groups always shown in editors.
		std::map<std::string, bool> m_DisabledMods; //!< List of the module names we disabled.
		std::map<std::string, bool> m_EnabledScripts; //!< List of the script names we enabled.

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

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
		SettingsMan(const SettingsMan &reference) = delete;
		SettingsMan & operator=(const SettingsMan &rhs) = delete;
	};
}
#endif
