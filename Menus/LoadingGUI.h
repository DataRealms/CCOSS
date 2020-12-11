#ifndef _LOADINGGUI_
#define _LOADINGGUI_

#include "Singleton.h"

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

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a LoadingGUI object in system memory.
		/// </summary>
		LoadingGUI() { Clear(); }

		/// <summary>
		/// Creates the loading screen GUI and the log writer, then proceeds loading all the DataModules.
		/// </summary>
		void InitLoadingScreen();

		/// <summary>
		/// Creates the GUI listbox that the progress report will be drawn to, if not disabled through the settings file to speed up loading times.
		/// As it turned out, a massive amount of time is spent updating the GUI control and flipping the frame buffers.
		/// </summary>
		void CreateProgressReportListbox();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the LoadingGUI object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the loading progress report.
		/// </summary>
		/// <param name="reportString">The string to print in the report and log.</param>
		/// <param name="newItem">Whether to start a new line in the log writer and to scroll the bitmap.</param>
		static void LoadingSplashProgressReport(const std::string &reportString, bool newItem = false);

		/// <summary>
		/// Finding and loading all DataModules.
		/// </summary>
		bool LoadDataModules();

		/// <summary>
		/// Unzip all *.rte.zip files found in the install directory, overwriting all files already existing.
		/// This will cause extracted and available data modules to be updated to whatever is within their corresponding zip files.
		/// The point of this is that it facilitates downloaded mods being loaded without having to be manually unzipped first by the user.
		/// </summary>
		// TODO: Break this down between PresetMan and System.
		void ExtractZippedModules() const;
#pragma endregion

	protected:

		static constexpr int s_MaxFileName = 512; //!< Maximum length of output file directory + name string.
		static constexpr int s_FileBufferSize = 8192; //!< Buffer to hold data read from the zip file.
		static constexpr int s_MaxUnzippedFileSize = 104857600; //!< Maximum size of single file being extracted from zip archive (100MiB).

		/// <summary>
		/// Container for all the supported file extensions that are allowed to be extracted from zipped module files.
		/// </summary>
		const std::unordered_set<std::string> s_SupportedExtensions = { ".ini", ".txt", ".lua", ".cfg", ".bmp", ".png", ".jpg", ".jpeg", ".wav", ".ogg", ".mp3", ".flac" };

		/// <summary>
		/// Custom deleters for std::unique_ptr members. Must be defined to avoid including the class headers and just rely on forward declaration.
		/// </summary>
		struct GUIControlManagerDeleter { void operator()(GUIControlManager *ptr) const; };
		struct AllegroInputDeleter { void operator()(AllegroInput *ptr) const; };
		struct AllegroScreenDeleter { void operator()(AllegroScreen *ptr) const; };
		struct WriterDeleter { void operator()(Writer *ptr) const; };

		std::unique_ptr<GUIControlManager, GUIControlManagerDeleter> m_ControlManager; //!< Manager of the whole LoadingGUI.
		std::unique_ptr<AllegroInput, AllegroInputDeleter> m_GUIInput; //!< Input interface of this.
		std::unique_ptr<AllegroScreen, AllegroScreenDeleter> m_GUIScreen; //!< Screen interface of this.
		std::unique_ptr<Writer, WriterDeleter> m_LoadingLogWriter; //!< The Writer that generates the loading log.
		BITMAP *m_ProgressListboxBitmap; //!< BITMAP that the progress report will be drawn into.

		int m_PosX; //!< Position of the progress report box on X axis.
		int m_PosY; //!< Position of the progress report box on Y axis.

	private:

		/// <summary>
		/// Clears all the member variables of this LoadingGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingGUI(const LoadingGUI &reference) = delete;
		LoadingGUI &operator=(const LoadingGUI &rhs) = delete;
	};
}
#endif