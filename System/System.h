#ifndef _RTESYSTEM_
#define _RTESYSTEM_

namespace RTE {

	/// <summary>
	/// Class for the system functionality.
	/// </summary>
	class System {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a System object in system memory. Should never be called directly, since g_System is an extern linked global that should be used by everything trying to use System.
		/// </summary>
		System() { m_LogToCLI = true; }
#pragma endregion

#pragma region Directories
		/// <summary>
		/// Returns current working directory.
		/// </summary>
		/// <returns>Absolute path to current working directory.</returns>
		std::string GetWorkingDirectory();

		/// <summary>
		/// Create a directory.
		/// </summary>
		/// <param name="path">Path to create.</param>
		/// <returns>Returns 0 if successful.</returns>
		int MakeDirectory(const std::string& path);
#pragma endregion

#pragma region Command-Line Interface
		/// <summary>
		/// Tells whether printing loading progress report and console to command-line is enabled or not.
		/// </summary>
		/// <returns>Whether printing to command-line is enabled or not.</returns>
		bool GetLogToCLI() const { return m_LogToCLI; }

		/// <summary>
		/// Sets whether to print the loading progress report and console to command-line or not.
		/// </summary>
		/// <param name="enable">True to enable printing to command-line.</param>
		void SetLogToCLI(bool enable) { m_LogToCLI = enable; }

		/// <summary>
		/// Prints the loading progress report to command-line.
		/// </summary>
		void PrintLoadingToCLI(std::string reportString, bool newItem = false);

		/// <summary>
		/// Prints console output to command-line.
		/// </summary>
		/// <param name="consoleString"></param>
		void PrintConsoleToCLI(std::string consoleString);
#pragma endregion

	protected:

		bool m_LogToCLI; //!< Bool to tell whether to print to command-line or not.
	};

	extern System g_System;
}
#endif