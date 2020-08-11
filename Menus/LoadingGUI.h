#ifndef _LOADINGUI_
#define _LOADINGGUI_

#include "Singleton.h"

struct BITMAP;

#define g_LoadingGUI LoadingGUI::Instance()

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIControlManager;
	class Writer;

	/// <summary>
	/// Represents the loading screen GUI when starting the game.
	/// </summary>
	class LoadingGUI : public Singleton<LoadingGUI> {

	public:

		/// <summary>
		/// Constructor method used to instantiate a LoadingGUI object in system memory.
		/// </summary>
		LoadingGUI() { Clear(); }

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
		void ExtractZippedModules();

	protected:

		static constexpr unsigned int s_MaxFileName = 256; //!< Maximum length of output file directory + name string.
		static constexpr unsigned int s_FileBufferSize = 8192; //!< Buffer to hold data read from the zip file.
		static constexpr unsigned int s_MaxUnzippedFileSize = 104857600; //!< Maximum size of single file being extracted from zip archive (100MiB).

		GUIControlManager *m_ControlManager; //!< Manager of the whole LoadingGUI.
		AllegroInput *m_GUIInput; //!< Input interface of this.
		AllegroScreen *m_GUIScreen; //!< Screen interface of this.
		Writer *m_LoadingLogWriter; //!< The Writer that generates the loading log.
		BITMAP *m_LoadingGUIBitmap; //!< BITMAP that the progress report will be drawn into.

		short m_PosX; //!< Position of the progress report box on X axis.
		short m_PosY; //!< Position of the progress report box on Y axis.

	private:

		/// <summary>
		/// Clears all the member variables of this LoadingGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingGUI(const LoadingGUI &reference) {}
		LoadingGUI &operator=(const LoadingGUI &rhs) {}
	};
}
#endif