#include "System.h"
#ifdef _WIN32 
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace RTE {

	System g_System;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string System::GetWorkingDirectory() {
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd))) {
			return std::string(cwd);
		}
		return std::string(".");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int System::MakeDirectory(const std::string& path) {
#ifdef _WIN32
		return _mkdir(path.c_str());
#elif __unix__
		return mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::PrintLoadingToCLI(std::string reportString, bool newItem) {
		if (newItem) { std::cout << std::endl; }
		// Overwrite current line
		std::cout << "\r";
		size_t startPos = 0;
		// Just make sure to really overwrite all old output
		// " - done! ✓" is shorter than "reading line 700"
		std::string unicoded = reportString + "          ";
		// Colorize output with ANSI escape code
		std::string greenTick = "\033[1;32m✓\033[0;0m";

		// Convert all ✓ characters to unicode
		// It's the 42th from last character in CC's custom font
		while ((startPos = unicoded.find(-42, startPos)) != std::string::npos) {
			unicoded.replace(startPos, 1, greenTick);
			// We don't have to check indices we just overwrote
			startPos += greenTick.length();
		}
		startPos = 0;
		std::string yellowDot = "\033[1;33m•\033[0;0m";
		// Convert all • characters to unicode
		while ((startPos = unicoded.find(-43, startPos)) != std::string::npos) {
			unicoded.replace(startPos, 1, yellowDot);
			startPos += yellowDot.length();
		}
		std::cout << unicoded << std::flush;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::PrintToCLI(std::string inputString) {
		// Color the words ERROR: and SYSTEM: red
		std::regex regexError("(ERROR|SYSTEM):");
		inputString = std::regex_replace(inputString, regexError, "\033[1;31m$&\033[0;0m");

		// Color .rte-paths green
		std::regex regexPath("\\w*\\.rte\\/(\\w| |\\.|\\/)*(\\/|\\.bmp|\\.png|\\.wav|\\.ogg|\\.flac||\\.lua|\\.ini)");
		inputString = std::regex_replace(inputString, regexPath, "\033[1;32m$&\033[0;0m");

		// Color names in quotes yellow
		// They have to start with an upper case letter to sort out apostrophes
		std::regex regexName("(\"[A-Z].*\"|\'[A-Z].*\')");
		inputString = std::regex_replace(inputString, regexName, "\033[1;33m$&\033[0;0m");

		std::cout << "\r" << inputString << std::endl;
	}
}
