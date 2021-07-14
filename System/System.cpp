#include "System.h"
#include "unzip.h"

#ifdef __unix__
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace RTE {

	bool System::s_Quit = false;
	bool System::s_LogToCLI = false;
	std::string System::s_WorkingDirectory = ".";
	std::vector<size_t> System::s_WorkingTree;
	std::filesystem::file_time_type System::s_ProgramStartTime = std::filesystem::file_time_type::clock::now();
	bool System::s_CaseSensitive = true;
	const std::string System::s_ScreenshotDirectory = "_ScreenShots";
	const std::string System::s_ModDirectory = "_Mods";
	const std::string System::s_ModulePackageExtension = ".rte";
	const std::string System::s_ZippedModulePackageExtension = ".rte.zip";
	const std::unordered_set<std::string> System::s_SupportedExtensions = { ".ini", ".txt", ".lua", ".cfg", ".bmp", ".png", ".jpg", ".jpeg", ".wav", ".ogg", ".mp3", ".flac" };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void System::Initialize() {
		s_WorkingDirectory = std::filesystem::current_path().generic_string();
		if (s_WorkingDirectory.back() != '/') { s_WorkingDirectory.append("/"); }

		if (!std::filesystem::exists(s_WorkingDirectory + s_ScreenshotDirectory)) { MakeDirectory(s_WorkingDirectory + s_ScreenshotDirectory); }
		//if (!std::filesystem::exists(s_WorkingDirectory + s_ModDirectory)) { MakeDirectory(s_WorkingDirectory + s_ModDirectory); }

#ifdef __unix__
		EnableLoggingToCLI();
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool System::MakeDirectory(const std::string &pathToMake) {
		bool createResult = std::filesystem::create_directory(pathToMake);
		if (createResult) {
			std::filesystem::permissions(pathToMake, std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::group_exec | std::filesystem::perms::others_read | std::filesystem::perms::others_exec, std::filesystem::perm_options::add);
		}
		return createResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool System::PathExistsCaseSensitive(const std::string &pathToCheck) {
		if (s_CaseSensitive) {
			if (s_WorkingTree.empty()) {
				for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::recursive_directory_iterator(s_WorkingDirectory, std::filesystem::directory_options::follow_directory_symlink)) {
					s_WorkingTree.emplace_back(std::hash<std::string>()(directoryEntry.path().generic_string().substr(s_WorkingDirectory.length())));
				}
			}
			if (std::find(s_WorkingTree.begin(), s_WorkingTree.end(), std::hash<std::string>()(pathToCheck)) != s_WorkingTree.end()) {
				return true;
			} else if (std::filesystem::exists(pathToCheck) && std::filesystem::last_write_time(pathToCheck) > s_ProgramStartTime) {
				s_WorkingTree.emplace_back(std::hash<std::string>()(pathToCheck));
				return true;
			}
			return false;
		}
		return std::filesystem::exists(pathToCheck);
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
		size_t tabPos = 0;
		while ((tabPos = unicodedOutput.find("\t")) != std::string::npos) {
			unicodedOutput.replace(tabPos, 1, "    ");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string System::ExtractZippedDataModule(const std::string &zippedModulePath) {
		std::string zippedModuleName = std::filesystem::path(zippedModulePath).filename().generic_string();

		unzFile zippedModule = unzOpen(zippedModuleName.c_str());
		std::stringstream extractionProgressReport;
		bool abortExtract = false;

		if (zippedModule) {
			unz_global_info zippedModuleInfo;
			if (unzGetGlobalInfo(zippedModule, &zippedModuleInfo) != UNZ_OK) {
				extractionProgressReport << "\tSkipped: " + zippedModuleName + " - Could not read global file info!\n";
				abortExtract = true;
			}
			std::array<char, s_FileBufferSize> fileBuffer;

			// Go through and extract every file inside this zip, overwriting every colliding file that already exists in the install directory.
			for (int i = 0; i < zippedModuleInfo.number_entry && !abortExtract; ++i) {
				unz_file_info currentFileInfo;
				std::array<char, s_MaxFileName> outputFileInfoData;
				if (unzGetCurrentFileInfo(zippedModule, &currentFileInfo, outputFileInfoData.data(), s_MaxFileName, nullptr, 0, nullptr, 0) != UNZ_OK) {
					extractionProgressReport << "\tSkipped: " + std::string(outputFileInfoData.data()) + " - Could not read file info!\n";
					continue;
				}
				std::string outputFileName = outputFileInfoData.data();
#ifdef _WIN32
				// TODO: Windows 10 adds support for paths over 260 characters so investigate how to get Windows version and whether the setting is enabled at runtime.
				// Windows doesn't support paths over 260 characters long.
				if ((s_WorkingDirectory + outputFileName).length() >= MAX_PATH) {
					extractionProgressReport << "\tSkipped file: " + outputFileName + " - Full path to file exceeds 260 characters!\n";
					continue;
				}
#endif
				// Check if the directory we are trying to extract into exists, and if not, create it.
				std::string outputFileDirectory = outputFileName.substr(0, outputFileName.find_last_of("/\\") + 1);
				if (!std::filesystem::exists(outputFileDirectory)) {
					if (!MakeDirectory(s_WorkingDirectory + outputFileDirectory)) {
						extractionProgressReport << "\tFailed to create directory: " + outputFileName + " - Extraction aborted!\n";
						abortExtract = true;
						continue;
					} else {
						extractionProgressReport << "\tCreated directory: " + outputFileName + "\n";
					}
				}
				// If the output file is a directly, go the next entry listed in the zip file.
				if (std::filesystem::is_directory(outputFileName)) {
					unzCloseCurrentFile(zippedModule);
					if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
						extractionProgressReport << "\tCould not read next file inside zip - Extraction aborted!\n";
						abortExtract = true;
					}
					continue;
				}

				// Validate so only certain file types are extracted.
				std::string fileExtension = std::filesystem::path(outputFileName).extension().generic_string();
				std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

				if (s_SupportedExtensions.find(fileExtension) == s_SupportedExtensions.end()) {
					extractionProgressReport << "\tSkipped file: " + outputFileName + " - Bad extension!\n";
					unzCloseCurrentFile(zippedModule);

					if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
						extractionProgressReport << "\tCould not read next file inside zip - Extraction aborted!\n";
						abortExtract = true;
					}
					continue;
				}

				if (unzOpenCurrentFile(zippedModule) != UNZ_OK) {
					extractionProgressReport << "\tSkipped file: " + zippedModuleName + " - Could not open file!\n";
				} else {
					FILE *outputFile = fopen(outputFileName.c_str(), "wb");
					if (outputFile == nullptr) { extractionProgressReport << "\tSkipped file: " + outputFileName + " - Could not open/create destination file!\n"; }

					// Write the entire file out, reading in buffer size chunks and spitting them out to the output stream.
					bool abortWrite = false;
					int bytesRead = 0;
					int totalBytesRead = 0;
					do {
						bytesRead = unzReadCurrentFile(zippedModule, fileBuffer.data(), s_FileBufferSize);
						totalBytesRead += bytesRead;

						if (bytesRead < 0) {
							extractionProgressReport << "\tSkipped file: " + outputFileName + " - File is empty or corrupt!\n";
							abortWrite = true;
						// Sanity check how damn big this file we're writing is becoming. could prevent zip bomb exploits: http://en.wikipedia.org/wiki/Zip_bomb
						} else if (totalBytesRead >= s_MaxUnzippedFileSize) {
							extractionProgressReport << "\tSkipped file: " + outputFileName + " - File is too large, extract it manually!\n";
							abortWrite = true;
						}
						if (abortWrite) {
							break;
						}
						fwrite(fileBuffer.data(), bytesRead, 1, outputFile);
					// Keep going while bytes are still being read (0 means end of file).
					} while (bytesRead > 0 && outputFile);

					fclose(outputFile);
					unzCloseCurrentFile(zippedModule);

					extractionProgressReport << "\tExtracted file: " + outputFileName + "\n";
				}

				if ((i + 1) < zippedModuleInfo.number_entry && unzGoToNextFile(zippedModule) != UNZ_OK) {
					extractionProgressReport << "\tCould not read next file inside zip - Extraction aborted!\n";
					abortExtract = true;
				}
			}
			unzClose(zippedModule);
			extractionProgressReport << "Successfully extracted Data Module from: " + zippedModuleName + " - Deleting zip file!\n";
			std::remove((s_WorkingDirectory + zippedModuleName).c_str());
		} else {
			bool makeDirResult = false;
			if (!std::filesystem::exists(s_WorkingDirectory + "_FailedExtract")) { makeDirResult = MakeDirectory(s_WorkingDirectory + "_FailedExtract"); }
			if (makeDirResult) {
				extractionProgressReport << "Failed to extract Data module from: " + zippedModuleName + " - Moving zip file to failed extract directory!\n";
				std::filesystem::rename(s_WorkingDirectory + zippedModuleName, s_WorkingDirectory + "_FailedExtract/" + zippedModuleName);
			} else {
				extractionProgressReport << "Failed to extract Data module from: " + zippedModuleName + " - Failed to create directory to move zip file into, deleting zip file!\n";
				std::remove((s_WorkingDirectory + zippedModuleName).c_str());
			}
		}
		return extractionProgressReport.str();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int System::ASCIIFileContainsString(const std::string &filePath, const std::string &findString) {
		std::ifstream inputStream(filePath, std::ios::binary);
		if (!inputStream.is_open()) {
			return -1;
		} else {
			size_t fileSize = static_cast<size_t>(std::filesystem::file_size(filePath));
			std::vector<unsigned char> rawData(fileSize);
			inputStream.read(reinterpret_cast<char *>(&rawData.at(0)), fileSize);
			inputStream.close();

			return (std::search(rawData.begin(), rawData.end(), findString.begin(), findString.end()) != rawData.end()) ? 0 : 1;
		}
	}
}
