#ifndef _RTEMODMANAGERGUI_
#define _RTEMODMANAGERGUI_

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIControlManager;
	class GUILabel;
	class GUIButton;
	class GUIListBox;

	/// <summary>
	/// Integrated mod and script manager user interface composition and handling.
	/// </summary>
	class ModManagerGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ModManagerGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!</param>
		ModManagerGUI(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Handles the player interaction with the ModManagerGUI GUI elements.
		/// </summary>
		/// <returns>Whether the player requested to return to the main menu.</returns>
		bool HandleInputEvents();

		/// <summary>
		/// Draws the ModManagerGUI to the screen.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:

		/// <summary>
		/// Struct containing information about a valid mod DataModule.
		/// </summary>
		struct ModRecord {
			std::string ModulePath; //!< Mod DataModule path.
			std::string ModuleName; //!< Mod ModuleName.
			std::string Description; //!< Mod description.
			bool Disabled; //!< Whether the mod is disabled through the settings file or not.

			/// <summary>
			/// Makes GUI displayable string with mod info.
			/// </summary>
			/// <returns>String with mod info.</returns>
			std::string GetDisplayString() const { return (Disabled ? "- " : "+ ") + ModulePath + " - " + ModuleName; }

			/// <summary>
			/// Comparison operator for sorting the KnownMods list alphabetically by path with std::sort.
			/// </summary>
			/// <param name="rhs">ModRecord to compare with.</param>
			/// <returns>Bool with result of the alphabetical comparison.</returns>
			bool operator<(const ModRecord &rhs) const { return ModulePath < rhs.ModulePath; }
		};

		/// <summary>
		/// Struct containing information about a valid GlobalScript.
		/// </summary>
		struct ScriptRecord {
			std::string PresetName; //!< Script PresetName.
			std::string Description; //!< Script description.
			bool Enabled; //!< Whether the script is enabled through the settings file or not.

			/// <summary>
			/// Makes GUI displayable string with script info.
			/// </summary>
			/// <returns>String with script info.</returns>
			std::string GetDisplayString() const { return (!Enabled ? "- " : "+ ") + PresetName; }

			/// <summary>
			/// Comparison operator for sorting the KnownScripts list alphabetically by PresetName with std::sort.
			/// </summary>
			/// <param name="rhs">ScriptRecord to compare with.</param>
			/// <returns>Bool with result of the alphabetical comparison.</returns>
			bool operator<(const ScriptRecord &rhs) const { return PresetName < rhs.PresetName; }
		};

#pragma region Mod and Script Handling
		/// <summary>
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// </summary>
		/// <returns>Whether both lists were fetched, even if nothing valid was added to them.</returns>
		bool ListsFetched() const { return m_ModsListFetched && m_ScriptsListFetched; }

		/// <summary>
		/// Fills the KnownMods list with all valid mod DataModules, then fills the ModsListBox using it.
		/// </summary>
		void PopulateKnownModsList();

		/// <summary>
		/// Fills the KnownScripts list with all valid GlobalScripts, then fills the ScriptsListBox using it.
		/// </summary>
		void PopulateKnownScriptsList();

		/// <summary>
		/// Turns currently selected mod on and off and changes GUI elements accordingly.
		/// </summary>
		void ToggleMod();

		/// <summary>
		/// Turns currently selected script on and off and changes GUI elements accordingly.
		/// </summary>
		void ToggleScript();
#pragma endregion

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of the ModManagerGUI.

		std::vector<ModRecord> m_KnownMods; //!< Contains ModRecords for all valid mod DataModules.
		std::vector<ScriptRecord> m_KnownScripts; //!< Contains ScriptRecords for all valid GlobalScripts.

		bool m_ModsListFetched; //!< Whether the known mods list was fetched, even if no valid mod DataModules were added to it.
		bool m_ScriptsListFetched; //!< Whether the known scripts list was fetched, even if no valid GlobalScripts were added to it.

		/// <summary>
		/// GUI elements that compose the Mod Manager menu screen.
		/// </summary>
		GUIButton *m_BackToMainButton;
		GUIButton *m_ToggleModButton;
		GUIButton *m_ToggleScriptButton;
		GUIListBox *m_ModsListBox;
		GUIListBox *m_ScriptsListBox;
		GUILabel *m_ModOrScriptDescriptionLabel;

		// Disallow the use of some implicit methods.
		ModManagerGUI(const ModManagerGUI &reference) = delete;
		ModManagerGUI & operator=(const ModManagerGUI &rhs) = delete;
	};
}
#endif