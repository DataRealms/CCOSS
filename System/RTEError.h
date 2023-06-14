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
		/// <param name="abortMessage">Whether the message box should display the abort title and icon. If not, the warning title and icon will be displayed.</param>
		static void ShowMessageBox(const std::string &message, bool abortMessage = false);

		/// <summary>
		/// Abort on Error function. Will try to dump a screenshot, show an abort message, and then quit the program immediately.
		/// </summary>
		/// <param name="description">Message explaining the reason for aborting.</param>
		/// <param name="file">The source file from which abort was called.</param>
		/// <param name="line">The line abort was called from in the source file.</param>
		[[noreturn]] static void AbortFunc(const std::string &description, const std::string &file, int line);

		/// <summary>
		/// An assert, which upon failure will abort.
		/// </summary>
		/// <param name="description">The description of the assertion.</param>
		/// <param name="file">The source file in which the assertion is made.</param>
		/// <param name="line">The line where the assertion is made.</param>
		[[noreturn]] static void AssertFunc(const std::string &description, const char *file, int line);
	};

#define RTEAbort(description) \
	if (!RTEError::s_CurrentlyAborting) {						\
		RTEError::AbortFunc(description, __FILE__, __LINE__);	\
	}

#define RTEAssert(expression, description) \
	if (!(expression)) {										\
		RTEError::AssertFunc(description, __FILE__, __LINE__);	\
	}
}
#endif