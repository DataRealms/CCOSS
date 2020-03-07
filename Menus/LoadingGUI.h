#ifndef _LOADINGUI_
#define _LOADINGGUI_

namespace RTE {

	class LoadingGUI {

	public:

		/// <summary>
		/// Creates the loading screen and the log writer, then calls loading all the modules.
		/// </summary>
		void InitLoadingScreen();

		/// <summary>
		/// Updates the loading progress report.
		/// </summary>
		/// <param name="reportString">The string to print in the report and log.</param>
		/// <param name="newItem">Whether to start a new line in the log writer and to scroll the bitmap.</param>
		static void LoadingSplashProgressReport(std::string reportString, bool newItem = false);

		/// <summary>
		/// Finding and loading all DataModules
		/// </summary>
		bool LoadDataModules();

		/// <summary>
		/// Unzip all *.rte.zip files found in the install directory, overwriting all files already existing.
		/// This will cause extracted and available data modules to be updated to whatever is within their corresponding zip files.
		/// The point of this is that it facilitates downloaded mods being loaded without having to be manually unzipped first by the user.
		/// </summary>
		void ExtractArchivedModules();

		/// <summary>
		/// Sets whether to print the progress report log to command-line.
		/// </summary>
		/// <param name="enable">True to print progress report log to command-line.</param>
		void SetLogToCLI(bool enable) { m_LogToCLI = true; }

	protected:

		static int m_LoadingGUIPosX; //! Position of the progress report box on X axis.
		static int m_LoadingGUIPosY; //! Position of the progress report box on Y axis.

		static bool m_LogToCLI; //! Bool to tell whether to print progress report to CLI or not.

		static const unsigned int s_MaxFileName = 256; //! Maximum length of output file directory + name string.
		static const unsigned int s_FileBufferSize = 8192; //! Buffer to hold data read from the zip file.
		static const unsigned int s_MaxUnzippedFileSize = 104857600; //! Maximum size of single file being extracted from zip archive (100MiB).
	};
	extern LoadingGUI g_LoadingGUI;
}
#endif