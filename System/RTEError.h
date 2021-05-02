#ifndef _RTEERROR_
#define _RTEERROR_

#include "System.h"

#include "allegro.h"
#include "loadpng.h"

#ifdef _WIN32
#define DebuggerBreak __debugbreak();
#else
#define DebuggerBreak std::exit(EXIT_FAILURE);
#endif

#ifndef RELEASE_BUILD
#define AbortAction DebuggerBreak
#else
#define AbortAction std::exit(EXIT_FAILURE);
#endif

namespace RTE {

	/// <summary>
	/// Pops up a message box dialog in the OS. For debug purposes mostly.
	/// </summary>
	/// <param name="message">The string that the message box should display.</param>
	extern void ShowMessageBox(const std::string &message);

	/// <summary>
	/// Abort on Error function. Will try to dump a screenshot, show an abort message, and then quit the program immediately.
	/// </summary>
	/// <param name="description">Message explaining the reason for aborting.</param>
	/// <param name="file">The source file from which abort was called.</param>
	/// <param name="line">The line abort was called from in the source file.</param>
	/// <returns>Always returns true to trigger AbortAction.</returns>
	extern bool RTEAbortFunc(const std::string &description, const std::string &file, int line);

	#define RTEAbort(description) {											\
		if (RTEAbortFunc(description, __FILE__, __LINE__)) { AbortAction }	\
	}

	/// <summary>
	/// A souped-up, customized assert function that brings up a nice dialog box on assert failure.
	/// The user can choose to break or ignore the particular assertion failure once, or to always ignore.
	/// </summary>
	/// <param name="expression">The expression that will be asserted to be true.</param>
	/// <param name="description">The description of the assertion.</param>
	/// <param name="file">The source file in which the assertion is made.</param>
	/// <param name="line">The line where the assertion is made.</param>
	/// <param name="alwaysIgnore">A reference to a bool that is used in an "Always ignore" functionality.</param>
	/// <returns>Whether the assertion failed AND the user chose to break in the dialog box.</returns>
	extern bool RTEAssertFunc(bool expression, const char *description, const char *file, int line, bool &alwaysIgnore);

	// TODO: Investigate why removing the 'const char *' method and using just this one impacts performance so much (roughly 10 fps drop in debug).
	inline extern bool RTEAssertFunc(bool expression, const std::string &description, const char *file, int line, bool &alwaysIgnore) {
		return RTEAssertFunc(expression, description.c_str(), file, line, alwaysIgnore);
	}

	#define RTEAssert(expression, description) {															\
		static bool alwaysIgnore = false;																	\
		if (!alwaysIgnore) {																				\
			if (RTEAssertFunc(expression, description, __FILE__, __LINE__, alwaysIgnore)) { AbortAction }	\
		}																									\
	}
}
#endif
