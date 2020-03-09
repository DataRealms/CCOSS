#include "System.h"
#include <direct.h>

#define getcwd _getcwd

namespace RTE {

	System g_System;

	bool System::m_LogToCLI = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string System::GetWorkingDirectory() {
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd))) {
			return std::string(cwd);
		}
		return std::string(".");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int System::MakeDirectory(const std::string& path) { return _mkdir(path.c_str()); }

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

	void System::PrintConsoleToCLI(std::string consoleString) {
		// Color the words ERROR: and SYSTEM: red
		std::regex regexError("(ERROR|SYSTEM):");
		consoleString = std::regex_replace(consoleString, regexError, "\033[1;31m$&\033[0;0m");

		// Color .rte-paths green
		std::regex regexPath("\\w*\\.rte\\/(\\w| |\\.|\\/)*(\\/|\\.bmp|\\.wav|\\.lua|\\.ini)");
		consoleString = std::regex_replace(consoleString, regexPath, "\033[1;32m$&\033[0;0m");

		// Color names in quotes yellow
		// They have to start with an upper case letter to sort out apostrophes
		std::regex regexName("(\"[A-Z].*\"|\'[A-Z].*\')");
		consoleString = std::regex_replace(consoleString, regexName, "\033[1;33m$&\033[0;0m");

		std::cout << "\r" << consoleString << std::endl;
	}
}
