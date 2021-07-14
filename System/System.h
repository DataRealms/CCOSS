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

#pragma region Program Termination
		/// <summary>
		/// Gets whether the program was set to be terminated by the user.
		/// </summary>
		/// <returns>Whether the program was set to be terminated by the user.</returns>
		static bool IsSetToQuit() { return s_Quit; }

		/// <summary>
		/// Sets the program to be terminated.
		/// </summary>
		/// <param name="quitOrNot">Terminate or not.</param>
		static void SetQuit(bool quitOrNot = true) { s_Quit = quitOrNot; }

		/// <summary>
		/// Sets termination when the close button (X) is pressed on the program window.
		/// </summary>
		static void WindowCloseButtonHandler() { SetQuit(); }
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
		/// Gets the extension that determines a directory/file is an RTE module.
		/// </summary>
		/// <returns>String containing the RTE module extension.</returns>
		static const std::string & GetModulePackageExtension() { return s_ModulePackageExtension; }

		/// <summary>
		/// Gets the extension that determines a file is a zipped RTE module.
		/// </summary>
		/// <returns>String containing the zipped RTE module extension.</returns>
		static const std::string & GetZippedModulePackageExtension() { return s_ZippedModulePackageExtension; }

		/// <summary>
		/// Create a directory.
		/// </summary>
		/// <param name="path">Path to create.</param>
		/// <returns>Returns 0 if successful.</returns>
		static bool MakeDirectory(const std::string &pathToMake);
#pragma endregion

#pragma region Filesystem
		/// <summary>
		/// Gets whether case sensitivity is enforced when checking for file existence.
		/// </summary>
		/// <returns>Whether case sensitivity is enforced.</returns>
		static bool FilePathsCaseSensitive() { return s_CaseSensitive; }

		/// <summary>
		/// Sets whether case sensitivity should be enforced when checking for file existence.
		/// </summary>
		/// <param name="enable">Whether case sensitivity should be enforced or not.</param>
		static void EnableFilePathCaseSensitivity(bool enable) { s_CaseSensitive = enable; }

		/// <summary>
		/// Checks if a file exists. On case sensitive filesystems returns std::filesystem::exists, otherwise the working directory will be checked for a matching file.
		/// </summary>
		/// <param name="pathToCheck">The path to check.</param>
		/// <returns>Whether the file exists.</returns>
		static bool PathExistsCaseSensitive(const std::string &pathToCheck);
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

#pragma region Archived DataModule Handling
		/// <summary>
		/// Extracts all files from a zipped DataModule, overwriting any corresponding files already existing.
		/// </summary>
		/// <param name="zippedModulePath">Path to the module to extract.</param>
		/// <returns>A string containing the progress report of the extraction.</returns>
		static std::string ExtractZippedDataModule(const std::string &zippedModulePath);
#pragma endregion

#pragma region Misc
		/// <summary>
		/// Fires up the default browser for the current OS on a specific URL.
		/// </summary>
		/// <param name="goToURL">A string with the URL to send the browser to.</param>
		static void OpenBrowserToURL(const std::string_view &goToURL) { std::system(std::string("start ").append(goToURL).c_str()); }

		/// <summary>
		/// Searches through an ASCII file on disk for a specific string and tells whether it was found or not.
		/// </summary>
		/// <param name="">The path to the ASCII file to search.</param>
		/// <param name="">The exact string to look for. Case sensitive!</param>
		/// <returns>0 if the string was found in the file or 1 if not. -1 if the file was inaccessible.</returns>
		static int ASCIIFileContainsString(const std::string &filePath, const std::string &findString);
#pragma endregion

	private:

		static bool s_Quit; //!< Whether the user requested program termination through GUI or the window close button.
		static bool s_LogToCLI; //!< Bool to tell whether to print the loading log and anything specified with PrintToCLI to command-line or not.
		static std::string s_WorkingDirectory; //!< String containing the absolute path to current working directory.
		static std::vector<size_t> s_WorkingTree; //!< Vector of the hashes of all file paths in the working directory.
		static std::filesystem::file_time_type s_ProgramStartTime; //!< Low precision time point of program start for checking if a file was created after starting.

		static bool s_CaseSensitive; //!< Whether case sensitivity is enforced when checking for file existence.
		static const std::string s_ScreenshotDirectory; //!< String containing the folder name of the screenshots directory.
		static const std::string s_ModDirectory; //!< String containing the folder name of the mod directory.
		static const std::string s_ModulePackageExtension; //!< The extension that determines a directory/file is a RTE module.
		static const std::string s_ZippedModulePackageExtension; //!< The extension that determines a file is a zipped RTE module.

		static const std::unordered_set<std::string> s_SupportedExtensions; //!< Container for all the supported file extensions that are allowed to be extracted from zipped module files.
		static constexpr int s_MaxFileName = 512; //!< Maximum length of output file directory + name string.
		static constexpr int s_FileBufferSize = 8192; //!< Buffer to hold data read from the zip file.
		static constexpr int s_MaxUnzippedFileSize = 104857600; //!< Maximum size of single file being extracted from zip archive (100MiB).
	};
}
#endif
