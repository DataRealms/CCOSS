#ifndef _RTESAVELOADMENUGUI_
#define _RTESAVELOADMENUGUI_

#include <filesystem>

namespace RTE {

	class PauseMenuGUI;

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUILabel;
	class GUIButton;
	class GUIListBox;
	class GUITextBox;
	class GUIComboBox;
	class GUICollectionBox;

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
		/// <param name="pauseMenu">Pointer to the pause menu, if we're being called from the pause menu. Ownership is NOT transferred!</param>
		/// <returns>Whether the player requested to return to the main menu.</returns>
		bool HandleInputEvents(PauseMenuGUI *pauseMenu = nullptr);

		/// <summary>
		/// Causes a refresh of the save files.
		/// </summary>
		void Refresh();

		/// <summary>
		/// Draws the SaveLoadMenuGUI to the screen.
		/// </summary>
		void Draw() const;
#pragma endregion

	private:
		enum class ConfirmDialogMode {
			None,
			ConfirmOverwrite,
			ConfirmDelete
		};

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
		GUICollectionBox *m_SaveGameMenuBox;
		GUIButton *m_BackToMainButton;
		GUITextBox *m_SaveGameName;
		GUIButton *m_LoadButton;
		GUIButton *m_CreateButton;
		GUIButton *m_OverwriteButton;
		GUIButton *m_DeleteButton;
		GUIListBox *m_SaveGamesListBox;
		GUILabel *m_ActivityCannotBeSavedLabel;
		GUIComboBox *m_OrderByComboBox;

		// The confirmation box and its controls
		ConfirmDialogMode m_ConfirmDialogMode;
		GUICollectionBox *m_ConfirmationBox;
		GUILabel *m_ConfirmationLabel;
		GUIButton* m_ConfirmationButton;
		GUIButton *m_CancelButton;

#pragma region Savegame Handling
		/// <summary>
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// </summary>
		/// <returns>Whether save games were fetched, even if nothing valid was added to them.</returns>
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
		/// <returns>Whether a same was succesfully loaded.</returns>
		bool LoadSave();

		/// <summary>
		/// Creates a new savefile (or overwrites the existing one) with the name from the textbox.
		/// </summary>
		void CreateSave();

		/// <summary>
		/// Deletes the savefile with the name from the textbox.
		/// </summary>
		void DeleteSave();
#pragma endregion

		/// <summary>
		/// Updates buttons and sets whether or not they should be enabled.
		/// </summary>
		void UpdateButtonEnabledStates();

		/// <summary>
		/// Shows confirmation box for overwrite or delete.
		/// </summary>
		void SwitchToConfirmDialogMode(ConfirmDialogMode mode);

		// Disallow the use of some implicit methods.
		SaveLoadMenuGUI(const SaveLoadMenuGUI &reference) = delete;
		SaveLoadMenuGUI & operator=(const SaveLoadMenuGUI &rhs) = delete;
	};
}
#endif