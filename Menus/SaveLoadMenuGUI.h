#ifndef _RTESAVELOADMENUGUI_
#define _RTESAVELOADMENUGUI_

#include <filesystem>

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUILabel;
	class GUIButton;
	class GUIListBox;
	class GUITextBox;
	class GUIComboBox;

	/// <summary>
	/// Integrated savegame user interface composition and handling.
	/// </summary>
	class SaveLoadMenuGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SaveLoadMenuGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this SaveLoadMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this SaveLoadMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="createForPauseMenu">Whether this SettingsGUI is part of SaveLoadMenuGUI and should have a slightly different layout.</param>
		SaveLoadMenuGUI(AllegroScreen *guiScreen, GUIInputWrapper *guiInput, bool createForPauseMenu = false);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Handles the player interaction with the SaveLoadMenuGUI GUI elements.
		/// </summary>
		/// <returns>Whether the player requested to return to the main menu.</returns>
		bool HandleInputEvents();

		/// <summary>
		/// Draws the SaveLoadMenuGUI to the screen.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:
		/// <summary>
		/// Struct containing information about a valid Savegame.
		/// </summary>
		struct SaveRecord {
			std::filesystem::path SavePath; //!< Savegame filepath.
			std::filesystem::file_time_type SaveDate; //!< Last modified date.
			std::string Activity; //!< The activity name.
			std::string Scene; //!< The scene name.
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of the SaveLoadMenuGUI.

		std::vector<SaveRecord> m_SaveGames; //!< Contains all SaveGames.

		bool m_SaveGamesFetched; //!< Whether the savegames list has been fetched.

		/// <summary>
		/// GUI elements that compose the Mod Manager menu screen.
		/// </summary>
		GUIButton *m_BackToMainButton;
		GUITextBox *m_SaveGameName;
		GUIButton *m_LoadButton;
		GUIButton *m_CreateButton;
		GUIListBox *m_SaveGamesListBox;
		GUILabel* m_DescriptionLabel;
		GUIComboBox *m_OrderByComboBox;

#pragma region Mod and Script Handling
		/// <summary>
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// </summary>
		/// <returns>Whether both lists were fetched, even if nothing valid was added to them.</returns>
		bool ListsFetched() const { return m_SaveGamesFetched; }

		/// <summary>
		/// Fills the SaveGames list with all valid savegames.
		/// </summary>
		void PopulateSaveGamesList();

		/// <summary>
		/// Updates the SaveGamesListBox GUI.
		/// </summary>
		void UpdateSaveGamesGUIList();

		/// <summary>
		/// Loads the currently selected savefile.
		/// </summary>
		void LoadSave();

		/// <summary>
		/// Creates a new savefile (or overwrites the existing one) with the name from the textbox.
		/// </summary>
		void CreateSave();
#pragma endregion

		// Disallow the use of some implicit methods.
		SaveLoadMenuGUI(const SaveLoadMenuGUI &reference) = delete;
		SaveLoadMenuGUI & operator=(const SaveLoadMenuGUI &rhs) = delete;
	};
}
#endif