#ifndef _RTEMODMANAGERGUI_
#define _RTEMODMANAGERGUI_

namespace RTE {

	class GUILabel;
	class GUIButton;
	class GUIListBox;
	class GUICollectionBox;
	class GUIControlManager;
	class AllegroScreen;
	class AllegroInput;

	/// <summary>
	/// Integrated mod and script manager user interface.
	/// </summary>
	class ModManagerGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ModManagerGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this ModManagerGUI's GUIControlManager.</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this ModManagerGUI's GUIControlManager.</param>
		ModManagerGUI(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// User input handling for the Mod Manager menu screen.
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// Draws the Mod Manager menu screen to the GUIScreen bitmap of it's GUIControlManager.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		struct ModRecord {
			std::string ModulePath; //!<
			std::string ModuleName; //!<
			std::string Description; //!<
			bool Disabled; //!<

			/// <summary>
			/// Makes UI displayable string with mod info.
			/// </summary>
			/// <returns>String with mod info.</returns>
			std::string MakeModString() const { return (Disabled ? "- " : "+ ") + ModulePath + " - " + ModuleName; }

			/// <summary>
			/// Comparison operator for sorting the KnownMods list alphabetically by path with std::sort.
			/// </summary>
			/// <param name="rhs">ModRecord to compare with.</param>
			/// <returns>Bool with result of the alphabetical comparison.</returns>
			bool operator<(const ModRecord &rhs) const { return ModulePath < rhs.ModulePath; }
		};

		/// <summary>
		/// 
		/// </summary>
		struct ScriptRecord {
			std::string PresetName; //!<
			std::string Description; //!<
			bool Enabled; //!<

			/// <summary>
			/// Makes UI displayable string with script info.
			/// </summary>
			/// <returns>String with script info.</returns>
			std::string MakeScriptString() const { return (!Enabled ? "- " : "+ ") + PresetName; }

			/// <summary>
			/// Comparison operator for sorting the KnownScripts list alphabetically by PresetName with std::sort.
			/// </summary>
			/// <param name="rhs">ScriptRecord to compare with.</param>
			/// <returns>Bool with result of the alphabetical comparison.</returns>
			bool operator<(const ScriptRecord &rhs) const { return PresetName < rhs.PresetName; }
		};

#pragma region Mod and Script Handling
		/// <summary>
		/// Turns currently selected mod on and off and changes UI elements accordingly.
		/// </summary>
		void ToggleMod();

		/// <summary>
		/// Turns currently selected script on and off and changes UI elements accordingly.
		/// </summary>
		void ToggleScript();

		/// <summary>
		/// 
		/// </summary>
		void PopulateKnownModsList();

		/// <summary>
		/// 
		/// </summary>
		void PopulateKnownScriptsList();
#pragma endregion

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu screen.

		std::vector<ModRecord> m_KnownMods; //!<
		std::vector<ScriptRecord> m_KnownScripts; //!<

		/// <summary>
		/// GUI elements that compose the Mod Manager menu screen.
		/// </summary>
		GUICollectionBox *m_RootBox;
		GUICollectionBox *m_ModManagerScreen;
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