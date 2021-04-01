#ifndef _RTECONSOLEMAN_
#define _RTECONSOLEMAN_

#include "Singleton.h"

#define g_ConsoleMan ConsoleMan::Instance()

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUITextBox;
	class GUILabel;

	/// <summary>
	/// The singleton manager of the lua console.
	/// </summary>
	class ConsoleMan : public Singleton<ConsoleMan> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ConsoleMan object in system memory. Create() should be called before using the object.
		/// </summary>
		ConsoleMan() { Clear(); }

		/// <summary>
		/// Makes the ConsoleMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a ConsoleMan object before deletion from system memory.
		/// </summary>
		~ConsoleMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ConsoleMan object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Reports whether the console is enabled or not.
		/// </summary>
		/// <returns>Whether the console is enabled or not.</returns>
		bool IsEnabled() const { return m_ConsoleState == ConsoleState::Enabled || m_ConsoleState == ConsoleState::Enabling; }

		/// <summary>
		/// Enables or disables the console.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the console.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Reports whether the console is in read-only mode or not.
		/// </summary>
		/// <returns>Whether the console is in read-only mode or not.</returns>
		bool IsReadOnly() const { return m_ReadOnly; }

		/// <summary>
		/// Gets how much of the screen that the console is covering when opened.
		/// </summary>
		/// <returns>The ratio of the screen that is covered.</returns>
		float GetConsoleScreenSize() const { return m_ConsoleScreenRatio; }

		/// <summary>
		/// Sets how much of the screen that the console should cover when opened.
		/// </summary>
		/// <param name="screenRatio">The ratio of the screen to cover. 0 - 1.0.</param>
		void SetConsoleScreenSize(float screenRatio = 0.3F);
#pragma endregion

#pragma region Logging
		/// <summary>
		/// Gets whether the loading warning log has any warnings logged or not.
		/// </summary>
		/// <returns>Whether the log has logged warnings.</returns>
		bool LoadWarningsExist() const { return !m_LoadWarningLog.empty(); }

		/// <summary>
		/// Adds a new entry to the loading warning log.
		/// </summary>
		/// <param name="pathToLog">The path that produced the warning.</param>
		/// <param name="readerPosition">The file and line currently being loaded.</param>
		/// <param name="altFileExtension">The alternative file extension to the path that produced the warning (e.g. if file is ".bmp", alternative extension is ".png").</param>
		void AddLoadWarningLogEntry(const std::string &pathToLog, const std::string &readerPosition = "", const std::string &altFileExtension = "" );

		/// <summary>
		/// Writes the entire loading warning log to a file.
		/// </summary>
		/// <param name="filePath">The filename of the file to write to.</param>
		void SaveLoadWarningLog(const std::string &filePath) const;

		/// <summary>
		/// Writes all the input strings to a log in the order they were entered.
		/// </summary>
		/// <param name="filePath">The filename of the file to write to.</param>
		void SaveInputLog(const std::string &filePath);

		/// <summary>
		/// Writes the entire console buffer to a file.
		/// </summary>
		/// <param name="filePath">The filename of the file to write to.</param>
		void SaveAllText(const std::string &filePath) const;

		/// <summary>
		/// Clears all previous input.
		/// </summary>
		void ClearLog();
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Prints a string into the console.
		/// </summary>
		/// <param name="stringToPrint">The string to print.</param>
		void PrintString(const std::string &stringToPrint) const;

		/// <summary>
		/// Opens the console and prints the shortcut help text.
		/// </summary>
		void ShowShortcuts();

		/// <summary>
		/// Updates the state of this ConsoleMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws this ConsoleMan's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		void Draw(BITMAP *targetBitmap) const;
#pragma endregion

	protected:

		/// <summary>
		/// Enumeration for console states when enabling/disabling the console. NOTE: This can't be lower down because m_ConsoleState relies on this definition.
		/// </summary>
		enum ConsoleState { Enabling = 0, Enabled, Disabling, Disabled };

		ConsoleState m_ConsoleState; //!< Current state of the console.
		bool m_ReadOnly; //!< Read-only mode where console text input is disabled and controller input should be preserved.
		float m_ConsoleScreenRatio; //!< The ratio of the screen that the console should take up, from 0.1 to 1.0 (whole screen).

		GUIScreen *m_GUIScreen; //!< GUI Screen for use by the in-game GUI.
		GUIInput *m_GUIInput; //!< GUI Input controller.
		GUIControlManager *m_GUIControlManager; //!< Manager of the console GUI elements.
		GUICollectionBox *m_ParentBox; //!< Collection box of the console GUI.
		GUILabel *m_ConsoleText; //!< The label which presents the console output.
		GUITextBox *m_InputTextBox; //!< The TextBox which the user types in the edited line.

		std::deque<std::string> m_InputLog; //!< Log of previously entered input strings.
		std::deque<std::string>::iterator m_InputLogPosition; //!< Iterator to the current position in the log.
		std::unordered_set<std::string> m_LoadWarningLog; //!< Log for non-fatal errors produced during loading (e.g. used .bmp file extension to load a .png file). 

		std::string m_LastInputString; //!< Place to save the last worked on input string before deactivating the console.
		short m_LastLogMove; //!< The last direction the log marker was moved. Needed so that changing directions won't need double tapping.

		std::string m_ConsoleTextBackup; //!< A copy of the whole console text at the time of destruction. Used to restore console text when ConsoleMan is re-created after a resolution change.

	private:

		/// <summary>
		/// Sets the console to read-only mode and enables it.
		/// </summary>
		void SetReadOnly();

#pragma region Update Breakdown
		/// <summary>
		/// Console open/close animation handling and GUI element enabling/disabling. This is called from Update().
		/// </summary>
		void ConsoleOpenClose();

		/// <summary>
		/// Executes the string currently in the console textbox or multiple strings if a newline character is found.
		/// The input string is saved to the input log if it's different from the previous string. This is called from Update().
		/// </summary>
		/// <param name="feedEmptyString">Whether to just pass in an empty string to make a new line.</param>
		void FeedString(bool feedEmptyString = false);

		/// <summary>
		/// Loads a previously entered console string from the input log when pressing up or down. This is called from Update().
		/// </summary>
		/// <param name="nextEntry">Whether to load the next entry in the log (true) or the previous (false).</param>
		void LoadLoggedInput(bool nextEntry);

		/// <summary>
		/// Removes any grave accents (`) that are pasted or typed into the textbox by opening/closing it. This is called from Update().
		/// </summary>
		void RemoveGraveAccents() const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ConsoleMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ConsoleMan(const ConsoleMan &reference) = delete;
		ConsoleMan & operator=(const ConsoleMan &rhs) = delete;
	};
}
#endif