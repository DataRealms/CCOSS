#include "System.h"

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace RTE {

	bool System::s_LogToCLI = false;
	std::string System::s_WorkingDirectory = ".";
	const std::string System::s_ScreenshotDirectory = "_ScreenShots";
	const std::string System::s_ModDirectory = "_Mods";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::Initialize() {
		char currentWorkingDir[1024];
		getcwd(currentWorkingDir, sizeof(currentWorkingDir));
		s_WorkingDirectory = std::string(currentWorkingDir);
		while (std::find(s_WorkingDirectory.begin(), s_WorkingDirectory.end(), '\\') != s_WorkingDirectory.end()) {
			s_WorkingDirectory.replace(s_WorkingDirectory.find("\\"), 1, "/");
		}
		if (s_WorkingDirectory.back() != '/') { s_WorkingDirectory.append("/"); }

		if (!std::filesystem::exists(s_WorkingDirectory + s_ScreenshotDirectory)) { MakeDirectory(s_WorkingDirectory + s_ScreenshotDirectory); }
		//if (!std::filesystem::exists(s_WorkingDirectory + s_ModDirectory)) { MakeDirectory(s_WorkingDirectory + s_ModDirectory); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int System::MakeDirectory(const std::string &pathToMake) {
#ifdef _WIN32
		return _mkdir(pathToMake.c_str());	
#elif __unix__
		return mkdir(pathToMake.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::EnableLoggingToCLI() {
#ifdef _WIN32
		// Create a console instance for the current process
		if (AllocConsole()) {
			CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);
			consoleInfo.dwSize.X = 192;
			SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), consoleInfo.dwSize);

			static std::ofstream consoleOutStream("CONOUT$", std::ios::out);
			// Set std::cout stream buffer to consoleOut's buffer to redirect the output
			std::cout.rdbuf(consoleOutStream.rdbuf());
		} else {
			MessageBox(nullptr, "Failed to allocate a console instance for this process, game console output will not be printed to CLI!", "RTE Warning! (>_<)", MB_OK);
			return;
		}
#endif
		s_LogToCLI = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::PrintLoadingToCLI(const std::string &reportString, bool newItem) {
		if (newItem) { std::cout << std::endl; }
		// Overwrite current line
		std::cout << "\r";
		size_t startPos = 0;
		// Just make sure to really overwrite all old output, " - done! ✓" is shorter than "reading line 700"
		std::string unicodedOutput = reportString + "            ";
		
#ifdef __unix__
		// Colorize output with ANSI escape code
		std::string greenTick = "\033[1;32m✓\033[0;0m";
		std::string yellowDot = "\033[1;33m•\033[0;0m";
#elif _WIN32
		// Fancy colors don't work for the Windows console so just replace with blanks
		std::string greenTick = "";
		std::string yellowDot = "";
		// Also replace tab with 4 spaces because tab is super wide in the Windows console
		while (unicodedOutput.find("\t") != std::string::npos) {
			unicodedOutput.replace(unicodedOutput.find("\t"), 1, "    ");
		}	
#endif
		// Convert all ✓ characters to unicode, it's the 42th from last character in CC's custom font
		while ((startPos = unicodedOutput.find(-42, startPos)) != std::string::npos) {
			unicodedOutput.replace(startPos, 1, greenTick);
			// We don't have to check indices we just overwrote
			startPos += greenTick.length();
		}
		startPos = 0;

		// Convert all • characters to unicode
		while ((startPos = unicodedOutput.find(-43, startPos)) != std::string::npos) {
			unicodedOutput.replace(startPos, 1, yellowDot);
			startPos += yellowDot.length();
		}
		std::cout << unicodedOutput << std::flush;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::PrintToCLI(const std::string &stringToPrint) {
#ifdef __unix__
		std::string outputString = stringToPrint;
		// Color the words ERROR: and SYSTEM: red
		std::regex regexError("(ERROR|SYSTEM):");
		outputString = std::regex_replace(outputString, regexError, "\033[1;31m$&\033[0;0m");

		// Color .rte-paths green
		std::regex regexPath("\\w*\\.rte\\/(\\w| |\\.|\\/)*(\\/|\\.bmp|\\.png|\\.wav|\\.ogg|\\.flac||\\.lua|\\.ini)");
		outputString = std::regex_replace(outputString, regexPath, "\033[1;32m$&\033[0;0m");

		// Color names in quotes yellow, they have to start with an upper case letter to sort out apostrophes
		std::regex regexName("(\"[A-Z].*\"|\'[A-Z].*\')");
		outputString = std::regex_replace(outputString, regexName, "\033[1;33m$&\033[0;0m");

		std::cout << "\r" << outputString << std::endl;
#elif _WIN32
		// All the fancy formatting doesn't work with the Windows console so just print the string as it is
		std::cout << "\r" << stringToPrint << std::endl;
#endif
	}
}