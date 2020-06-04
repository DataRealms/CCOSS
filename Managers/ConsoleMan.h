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
		virtual int Create();
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
		bool IsEnabled() const { return m_ConsoleState == ENABLED || m_ConsoleState == ENABLING; }

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
		/// Writes all the input strings to a log in the order they were entered.
		/// </summary>
		/// <param name="filePath">The filename of the file to write to.</param>
		void SaveInputLog(std::string filePath);

		/// <summary>
		/// Writes the entire console buffer to a file.
		/// </summary>
		/// <param name="filePath">The filename of the file to write to.</param>
		void SaveAllText(std::string filePath);

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
		void PrintString(std::string stringToPrint);

		/// <summary>
		/// Updates the state of this ConsoleMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws this ConsoleMan's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		void Draw(BITMAP *targetBitmap);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this object.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// <summary>
		/// //!< Enumeration for console states when enabling/disabling the console. NOTE: This can't be lower down because m_ConsoleState relies on this definition.
		/// </summary>
		enum ConsoleState { ENABLING = 0, ENABLED, DISABLING, DISABLED };

		ConsoleState m_ConsoleState; //!< Current state of the console.
		bool m_ReadOnly; //!< Read-only mode where console text input is disabled and controller input should be preserved.
		float m_ConsoleScreenRatio; //!< The ratio of the screen that the console should take up, from 0.1 to 1.0 (whole screen).

		GUIScreen *m_GUIScreen; //!< GUI Screen for use by the in-game GUI.
		GUIInput *m_GUIInput; //!< GUI Input controller.
		GUIControlManager *m_GUIController; //!< Manager of the console GUI elements.
		GUICollectionBox *m_ParentBox; //!< Collection box of the console GUI.
		GUILabel *m_ConsoleText; //!< The label which presents the console output.
		GUITextBox *m_InputTextBox; //!< The Textbox which the user types in the edited line.

		std::deque<std::string> m_InputLog; //!< Log of previously entered input strings.
		std::deque<std::string>::iterator m_InputLogPosition; //!< Iterator to the current position in the log.

		std::string m_LastInputString; //!< Place to save the last worked on input string before deactivating the console.
		short m_LastLogMove; //!< The lat direction the log marker was moved. Needed so that changing directions won't need double tapping.

	private:

		/// <summary>
		/// Sets the console to read-only mode and enables it.
		/// </summary>
		void SetReadOnly();

#pragma region Update Breakdown
		/// <summary>
		/// Executes the input string when Enter is pressed and clears the textbox. This is called from Update().
		/// </summary>
		void FeedString();

		/// <summary>
		/// Executes multiple input strings that were pasted into the console. This is called from Update().
		/// </summary>
		void FeedMultipleStrings();

		/// <summary>
		/// Console open/close animation handling and GUI element enabling/disabling. This is called from Update().
		/// </summary>
		void ConsoleOpenClose();

		/// <summary>
		/// Loads a previously entered console string from the input log when pressing up or down. This is called from Update().
		/// </summary>
		/// <param name="nextEntry">Whether to load the next entry in the log (true) or the previous (false).</param>
		void LoadLoggedInput(bool nextEntry);
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ConsoleMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ConsoleMan(const ConsoleMan &reference) {}
		ConsoleMan & operator=(const ConsoleMan &rhs) {}
	};
}
#endif