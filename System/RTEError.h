#ifndef _RTEERROR_
#define _RTEERROR_

#include "System.h"

#include "allegro.h"
#include "loadpng.h"

#ifdef _WIN32
#define DebuggerBreak __debugbreak()
#else
#define DebuggerBreak std::abort()
#endif

#ifndef RELEASE_BUILD
#define AbortAction DebuggerBreak
#else
#define AbortAction std::abort()
#endif

namespace RTE {

	/// <summary>
	/// Class for runtime error handling.
	/// </summary>
	class RTEError {

	public:

		static bool s_CurrentlyAborting; // Flag to prevent a potential recursive fault while attempting to save the game when aborting.

		/// <summary>
		/// Pops up a message box dialog in the OS. For debug purposes mostly.
		/// </summary>
		/// <param name="message">The string that the message box should display.</param>
		static void ShowMessageBox(const std::string &message);

		/// <summary>
		/// Abort on Error function. Will try to dump a screenshot, show an abort message, and then quit the program immediately.
		/// </summary>
		/// <param name="description">Message explaining the reason for aborting.</param>
		/// <param name="srcLocation">std::source_location corresponding to the location of the call site.</param>
		[[noreturn]] static void AbortFunc(const std::string &description, const std::source_location &srcLocation);

		/// <summary>
		/// An assert, which upon failure will abort.
		/// </summary>
		/// <param name="description">The description of the assertion.</param>
		/// <param name="srcLocation">std::source_location corresponding to the location of the call site.</param>
		[[noreturn]] static void AssertFunc(const std::string &description, const std::source_location &srcLocation);

	private:

		/// <summary>
		/// Pops up the abort message box dialog in the OS, notifying the user about a runtime error.
		/// </summary>
		/// <param name="message">The string that the message box should display.</param>
		/// <returns>Whether to restart the game by launching a new instance, or proceed to exit.</returns>
		static bool ShowAbortMessageBox(const std::string &message);

		/// <summary>
		/// Pops up the assert message box dialog in the OS, notifying the user about a runtime error.
		/// </summary>
		/// <param name="message">The string that the message box should display.</param>
		/// <returns>Whether to abort, or ignore the assert and continue execution.</returns>
		static bool ShowAssertMessageBox(const std::string &message);

		/// <summary>
		/// Saves the current frame to a file.
		/// </summary>
		/// <returns>Whether the file was saved successfully.<returns>
		static bool DumpAbortScreen();

		/// <summary>
		/// Attempts to save the current running Activity, so the player can hopefully resume where they were.
		/// </summary>
		/// <returns>Whether the Activity was saved successfully.</returns>
		static bool DumpAbortSave();
	};

#define RTEAbort(description) \
	if (!RTEError::s_CurrentlyAborting) {										\
		RTEError::AbortFunc(description, std::source_location::current());		\
	}

#define RTEAssert(expression, description) \
	if (!(expression)) {														\
		RTEError::AssertFunc(description, std::source_location::current());		\
	}
}
#endif