#ifndef _RTESYSTEM_
#define _RTESYSTEM_

namespace RTE {

	/// <summary>
	/// Class for the system functionality.
	/// </summary>
	class System {

	public:

		/// <summary>
		/// Returns current working directory.
		/// </summary>
		/// <returns>Absolute path to current working directory.</returns>
		std::string GetWorkingDirectory();

		/// <summary>
		/// Create a directory.
		/// </summary>
		/// <param name="path">Path to create.</param>
		/// <returns>Returns 0 if successful, POSIX compliant error code if error.</returns>
		int MakeDirectory(const std::string& path);

		/// <summary>
		/// Prints the loading progress report to command line.
		/// </summary>
		void LogToCLI(std::string reportString, bool newItem = false);
	};

	extern System g_System;
}
#endif