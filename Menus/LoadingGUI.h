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
		/// Creates the GUI ListBox that the progress report will be drawn to, if not disabled through the settings file to speed up loading times.
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
#pragma endregion

	protected:

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