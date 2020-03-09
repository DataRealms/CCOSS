#ifndef _RTESYSTEM_
#define _RTESYSTEM_

namespace RTE {

	/// <summary>
	/// Class for the system functionality.
	/// </summary>
	class System {

	public:

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

		static bool m_LogToCLI; //!< Bool to tell whether to print to command-line or not.
	};

	extern System g_System;
}
#endif