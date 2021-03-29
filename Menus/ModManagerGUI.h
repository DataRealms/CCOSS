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
	/// 
	/// </summary>
	class ModManagerGUI {

	public:

#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		ModManagerGUI(AllegroScreen *guiScreen, AllegroInput *guiInput);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// 
		/// </summary>
		void SetEnabled() const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		bool HandleInputEvents();

		/// <summary>
		/// 
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
			bool Disabled;

			/// <summary>
			/// 
			/// </summary>
			/// <param name="rhs"></param>
			/// <returns></returns>
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
			/// 
			/// </summary>
			/// <param name="rhs"></param>
			/// <returns></returns>
			bool operator<(const ScriptRecord &rhs) const { return PresetName < rhs.PresetName; }
		};

#pragma region Mod and Script Handling
		/// <summary>
		/// Makes UI displayable string with mod info.
		/// </summary>
		/// <param name="modRecord"></param>
		/// <returns>String with mod info.</returns>
		std::string MakeModString(const ModRecord &modRecord) const { return (modRecord.Disabled ? "- " : "+ ") + modRecord.ModulePath + " - " + modRecord.ModuleName; }

		/// <summary>
		/// Makes UI displayable string with script info.
		/// </summary>
		/// <param name="scriptRecord"></param>
		/// <returns>String with script info.</returns>
		std::string MakeScriptString(const ScriptRecord &scriptRecord) const { return (!scriptRecord.Enabled ? "- " : "+ ") + scriptRecord.PresetName; }

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
		void FillKnownModsList();

		/// <summary>
		/// 
		/// </summary>
		void FillKnownScriptsList();
#pragma endregion

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!<

		std::vector<ModRecord> m_KnownMods; //!<
		std::vector<ScriptRecord> m_KnownScripts; //!<

		GUICollectionBox *m_RootBox;
		GUICollectionBox *m_ModManagerScreen;
		GUIButton *m_BackToMainButton;
		GUIButton *m_ToggleModButton;
		GUIButton *m_ToggleScriptButton;
		GUIListBox *m_ModsListBox;
		GUIListBox *m_ScriptsListBox;
		GUILabel *m_ModOrScriptDescriptionLabel;
	};
}
#endif