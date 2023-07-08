#include "RTEError.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"

#include "SDL_messagebox.h"

namespace RTE {

	bool RTEError::s_CurrentlyAborting = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::ShowMessageBox(const std::string &message) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "RTE Warning! (>_<)", message.c_str(), nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::ShowAbortMessageBox(const std::string &message) {
		enum AbortMessageButton { ButtonInvalid, ButtonExit, ButtonRestart };

		std::vector<SDL_MessageBoxButtonData> abortMessageBoxButtons = {
			SDL_MessageBoxButtonData(SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, AbortMessageButton::ButtonExit, "OK")
		};

		// Don't even show the restart button in debug builds.
#ifdef RELEASE_BUILD
		// Getting a junk path from argv[0] is, or should be, impossible but check anyway.
		if (std::filesystem::exists(System::GetThisExePathAndName())) {
			abortMessageBoxButtons.emplace_back(0, AbortMessageButton::ButtonRestart, "Restart Game");
		}
#endif

		SDL_MessageBoxData abortMessageBox = {
			SDL_MESSAGEBOX_ERROR,
			g_WindowMan.GetWindow(),
			"RTE Aborted! (x_x)",
			message.c_str(),
			static_cast<int>(abortMessageBoxButtons.size()),
			abortMessageBoxButtons.data(),
			nullptr
		};

		int pressedButton = AbortMessageButton::ButtonInvalid;
		SDL_ShowMessageBox(&abortMessageBox, &pressedButton);

		return pressedButton == AbortMessageButton::ButtonRestart;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::ShowAssertMessageBox(const std::string &message) {
		enum AssertMessageButton { ButtonInvalid, ButtonAbort, ButtonIgnore };

		std::vector<SDL_MessageBoxButtonData> abortMessageBoxButtons = {
			SDL_MessageBoxButtonData(SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, AssertMessageButton::ButtonAbort, "Abort"),
			SDL_MessageBoxButtonData(0, AssertMessageButton::ButtonIgnore, "Ignore")
		};

		SDL_MessageBoxData assertMessageBox = {
			SDL_MESSAGEBOX_ERROR,
			g_WindowMan.GetWindow(),
			"RTE Assert! (x_x)",
			message.c_str(),
			static_cast<int>(abortMessageBoxButtons.size()),
			abortMessageBoxButtons.data(),
			nullptr
		};

		int pressedButton = AssertMessageButton::ButtonInvalid;
		SDL_ShowMessageBox(&assertMessageBox, &pressedButton);

		return pressedButton == AssertMessageButton::ButtonAbort;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::AbortFunc(const std::string &description, const std::source_location &srcLocation) {
		s_CurrentlyAborting = true;

		if (!System::IsInExternalModuleValidationMode()) {
			// This typically contains the absolute path to the file on whatever machine this was compiled on, so in that case get only the file name.
			std::filesystem::path filePath = srcLocation.file_name();
			std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : srcLocation.file_name();

			std::string lineNum = std::to_string(srcLocation.line());
			std::string funcName = srcLocation.function_name();

			std::string abortMessage = "Runtime Error in file '" + fileName + "', line " + lineNum + ",\nin function '" + funcName + "'\nbecause:\n\n" + description + "\n";

			if (DumpAbortSave()) {
				abortMessage += "\nThe game has saved to 'AbortSave'.";
			}
			if (DumpAbortScreen()) {
				abortMessage += "\nThe last frame has been dumped to 'AbortScreen.png'.";
			}

			g_ConsoleMan.PrintString(abortMessage);
			if (g_ConsoleMan.SaveAllText("AbortLog.txt")) {
				abortMessage += "\nThe console has been dumped to 'AbortLog.txt'.";
			}
			System::PrintToCLI(abortMessage);

			// Ditch the video mode so the message box appears without problems.
			if (g_WindowMan.GetWindow()) {
				SDL_SetWindowFullscreen(g_WindowMan.GetWindow(), 0);
			}

			if (ShowAbortMessageBox(abortMessage)) {
				// Enable restarting in release builds only.
				// Once this exits the debugger is detached and there doesn't seem to be a way to programatically re-attach it to the new instance.
				// This will prevent your day from being ruined when your breakpoints don't trigger during a meltdown because you launched a new instance and didn't realize you're not attached to it.
#ifdef RELEASE_BUILD
#ifdef _WIN32
				std::system(std::string(R"(start "" ")" + System::GetThisExePathAndName() + "\"").c_str());
#else
				std::system(std::string("\"" + System::GetThisExePathAndName() + "\"").c_str());
#endif
#endif
			}
		}
		s_CurrentlyAborting = false;
		AbortAction;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::AssertFunc(const std::string &description, const std::source_location &srcLocation) {
		// This typically contains the absolute path to the file on whatever machine this was compiled on, so in that case get only the file name.
		std::filesystem::path filePath = srcLocation.file_name();
		std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : srcLocation.file_name();

		std::string lineNum = std::to_string(srcLocation.line());
		std::string funcName = srcLocation.function_name();

		std::string assertMessage =
			"Assertion in file '" + fileName + "', line " + lineNum + ",\nin function '" + funcName + "'\nbecause:\n\n" + description + "\n\n"
			"You may choose to ignore this and crash immediately\nor at some unexpected point later on.\n\nProceed at your own risk!";

		if (ShowAssertMessageBox(assertMessage)) {
			AbortFunc(description, srcLocation);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::DumpAbortScreen() {
		int success = -1;
		if (BITMAP *backbuffer = g_FrameMan.GetBackBuffer32(); backbuffer) {
			// Have to convert the 32bpp backbuffer to 24bpp otherwise the saved file is blank for reasons that don't matter.
			BITMAP *abortScreenBuffer = create_bitmap_ex(24, backbuffer->w, backbuffer->h);
			blit(backbuffer, abortScreenBuffer, 0, 0, 0, 0, backbuffer->w, backbuffer->h);
			success = save_png("AbortScreen.png", abortScreenBuffer, nullptr);
			destroy_bitmap(abortScreenBuffer);
		}
		return success == 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::DumpAbortSave() {
		bool success = false;
		if (g_ActivityMan.GetActivityAllowsSaving()) {
			success = g_ActivityMan.SaveCurrentGame("AbortSave");
		}
		return success;
	}
}