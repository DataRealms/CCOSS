#ifndef _RTESTACKTRACE_
#define _RTESTACKTRACE_

#include "StackWalker/StackWalker.h"

namespace RTE {

	/// <summary>
	/// Class for dumping stack traces on Windows.
	/// Wraps the StackWalker class which uses WinAPI Debug Help Library routines.
	/// </summary>
	class RTEStackTrace : public StackWalker {

	public:

		/// <summary>
		/// Constructor method used to instantiate an RTEStackTrace object in system memory and make it ready for use.
		/// </summary>
		/// <param name="options"></param>
		RTEStackTrace() : StackWalker() {}

		/// <summary>
		/// Destructor method used to clean up a RTEStackTrace object before deletion from system memory.
		/// </summary>
		~RTEStackTrace() override = default;

		/// <summary>
		/// Gets the current call stack as a string.
		/// </summary>
		/// <param name="handle">Handle to the current process. If none provided will get the current thread handle.</param>
		/// <param name="context">Register data. If none provided will get it from the caller.</param>
		/// <returns>A string with the call stack.</returns>
		std::string GetCallStackAsString(const HANDLE &handle = nullptr, const CONTEXT *context = nullptr);

	protected:

		/// <summary>
		/// Redirects the output string to the member string stream.
		/// </summary>
		/// <param name="text">The output string. Provided by the base class method.</param>
		void OnOutput(LPCSTR text) override;

	private:

		std::stringstream m_CallstackStream; //!< Call stack output stream.

		// Disallow the use of some implicit methods.
		RTEStackTrace(const RTEStackTrace &reference) = delete;
		RTEStackTrace & operator=(const RTEStackTrace &rhs) = delete;
	};
}
#endif