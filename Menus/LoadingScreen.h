#ifndef _RTELOADINGSCREEN_
#define _RTELOADINGSCREEN_

#include "Singleton.h"

#define g_LoadingScreen LoadingScreen::Instance()

namespace RTE {

	class AllegroScreen;
	class AllegroInput;
	class GUIControlManager;
	class Writer;

	/// <summary>
	/// Handling for the loading screen composition and loading progress box when starting the game.
	/// </summary>
	class LoadingScreen : public Singleton<LoadingScreen> {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a LoadingScreen object in system memory.
		/// </summary>
		LoadingScreen() { Clear(); }

		/// <summary>
		/// Makes the LoadingScreen object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="progressReportDisabled">Whether the loading screen progress report is disabled meaning GUI elements and adjustments relevant to it can be skipped.</param>
		void Create(AllegroScreen *guiScreen, AllegroInput *guiInput, bool progressReportDisabled);

		/// <summary>
		/// Creates the GUIListBox that the progress report will be drawn to, if not disabled through the settings file to speed up loading times.
		/// As it turned out, a massive amount of time is spent updating the GUI control and flipping the frame buffers.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this LoadingScreen. Ownership is NOT transferred!</param>
		void CreateProgressReportListbox(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the LoadingScreen object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the loading progress report and draws it to the screen if not disabled through the settings file.
		/// </summary>
		/// <param name="reportString">The string to print in the report and log.</param>
		/// <param name="newItem">Whether to start a new line in the log writer and to scroll the bitmap.</param>
		static void LoadingSplashProgressReport(const std::string &reportString, bool newItem = false);
#pragma endregion

	private:

		std::unique_ptr<Writer> m_LoadingLogWriter; //!< The Writer that generates the loading log.

		BITMAP *m_ProgressListboxBitmap; //!< BITMAP that the progress report will be drawn into.
		int m_ProgressListboxPosX; //!< Position of the progress report box on X axis.
		int m_ProgressListboxPosY; //!< Position of the progress report box on Y axis.

		/// <summary>
		/// Clears all the member variables of this LoadingScreen, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingScreen(const LoadingScreen &reference) = delete;
		LoadingScreen &operator=(const LoadingScreen &rhs) = delete;
	};
}
#endif