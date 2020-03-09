#ifndef _LOADINGUI_
#define _LOADINGGUI_

struct BITMAP;

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIControlManager;
	class Writer;

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

	protected:

		static GUIControlManager *m_LoadingGUI; //! Manager of the whole LoadingGUI.

		static AllegroInput *m_GUIInput; //! Input interface of this.
		static AllegroScreen *m_GUIScreen; //! Screen interface of this.
		static BITMAP *m_LoadingGUIBitmap; //! BITMAP that the progress report will be drawn into.
		static int m_LoadingGUIPosX; //! Position of the progress report box on X axis.
		static int m_LoadingGUIPosY; //! Position of the progress report box on Y axis.

		static Writer *m_LoadingLogWriter; //! The Writer that generates the loading log.

		static const unsigned int s_MaxFileName = 256; //! Maximum length of output file directory + name string.
		static const unsigned int s_FileBufferSize = 8192; //! Buffer to hold data read from the zip file.
		static const unsigned int s_MaxUnzippedFileSize = 104857600; //! Maximum size of single file being extracted from zip archive (100MiB).
	};
	extern LoadingGUI g_LoadingGUI;
}
#endif