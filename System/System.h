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
		/// Store the current working directory and create any missing subdirectories.
		/// </summary>
		static void Initialize();
#pragma endregion

#pragma region Directories
		/// <summary>
		/// Gets the current working directory.
		/// </summary>
		/// <returns>Absolute path to current working directory.</returns>
		static const std::string & GetWorkingDirectory() { return s_WorkingDirectory; }

		/// <summary>
		/// Gets the screenshot directory name.
		/// </summary>
		/// <returns>Folder name of the screenshots directory.</returns>
		static const std::string & GetScreenshotDirectory() { return s_ScreenshotDirectory; }

		/// <summary>
		/// Gets the mod directory name.
		/// </summary>
		/// <returns>Folder name of the mod directory.</returns>
		static const std::string & GetModDirectory() { return s_ModDirectory; }

		/// <summary>
		/// Gets the extension that determines a directory/file is a RTE module. 
		/// </summary>
		/// <returns>String containing the RTE module extension.</returns>
		static const std::string & GetModulePackageExtension() { return s_ModulePackageExtension; }

		/// <summary>
		/// Create a directory.
		/// </summary>
		/// <param name="path">Path to create.</param>
		/// <returns>Returns 0 if successful.</returns>
		static bool MakeDirectory(const std::string &pathToMake);
#pragma endregion

#pragma region Command-Line Interface
		/// <summary>
		/// Tells whether printing loading progress report and console to command-line is enabled or not.
		/// </summary>
		/// <returns>Whether printing to command-line is enabled or not.</returns>
		static bool IsLoggingToCLI() { return s_LogToCLI; }

		/// <summary>
		/// Enables printing the loading progress report and console to command-line. For Windows, also allocates a console instance and redirects cout to it.
		/// </summary>
		static void EnableLoggingToCLI();

		/// <summary>
		/// Prints the loading progress report to command-line.
		/// </summary>
		static void PrintLoadingToCLI(const std::string &reportString, bool newItem = false);

		/// <summary>
		/// Prints console output to command-line.
		/// </summary>
		/// <param name="inputString"></param>
		static void PrintToCLI(const std::string &stringToPrint);
#pragma endregion

	private:

		static bool s_LogToCLI; //!< Bool to tell whether to print the loading log and anything specified with PrintToCLI to command-line or not.
		static std::string s_WorkingDirectory; //!< String containing the absolute path to current working directory.
		static const std::string s_ScreenshotDirectory; //!< String containing the folder name of the screenshots directory.
		static const std::string s_ModDirectory; //!< String containing the folder name of the mod directory.
		static const std::string s_ModulePackageExtension; //!< The extension that determines a directory/file is a RTE module.
	};
}
#endif