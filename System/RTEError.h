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
	/// Pops up a message box dialog in the OS. For debug purposes mostly.
	/// </summary>
	/// <param name="message">The string that the message box should display.</param>
	/// <param name="abortMessage">Whether the message box should display the abort title and icon. If not, the warning title and icon will be displayed.</param>
	extern void ShowMessageBox(const std::string &message, bool abortMessage = false);

	/// <summary>
	/// Abort on Error function. Will try to dump a screenshot, show an abort message, and then quit the program immediately.
	/// </summary>
	/// <param name="description">Message explaining the reason for aborting.</param>
	/// <param name="file">The source file from which abort was called.</param>
	/// <param name="line">The line abort was called from in the source file.</param>
	[[noreturn]] extern void RTEAbortFunc(const std::string &description, const std::string &file, int line);

	#define RTEAbort(description) {							\
		RTEAbortFunc(description, __FILE__, __LINE__);		\
	}

	/// <summary>
	/// An assert, which upon failure will abort.
	/// </summary>
	/// <param name="description">The description of the assertion.</param>
	/// <param name="file">The source file in which the assertion is made.</param>
	/// <param name="line">The line where the assertion is made.</param>
	[[noreturn]] extern void RTEAssertFunc(const std::string &description, const char *file, int line);

	#define RTEAssert(expression, description) {			\
		if (!(expression)) {								\
			RTEAssertFunc(description, __FILE__, __LINE__);	\
		}													\
	}
}
#endif
