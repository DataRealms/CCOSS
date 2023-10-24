#include "RTEError.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"

#include "SDL_messagebox.h"

#ifdef _WIN32
#include "Windows.h"
#include "DbgHelp.h"
#include "RTEStackTrace.h"
#endif

namespace RTE {

	bool RTEError::s_CurrentlyAborting = false;
	bool RTEError::s_IgnoreAllAsserts = false;
	std::string RTEError::s_LastIgnoredAssertDescription = "";
	std::source_location RTEError::s_LastIgnoredAssertLocation = {};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	/// <summary>
	/// Custom exception handler for Windows SEH.
	/// Unfortunately this also intercepts any C++ exceptions and turns them into SE bullshit, meaning we can't get and rethrow the current C++ exception to get what() from it.
	/// Even if we "translate" SE exceptions to C++ exceptions it's still ass and doesn't really work, so this is what it is and it is good enough.
	/// </summary>
	/// <param name="exceptPtr">Struct containing information about the exception. This will be provided by the OS exception handler.</param>
	static LONG WINAPI RTEWindowsExceptionHandler([[maybe_unused]] EXCEPTION_POINTERS *exceptPtr) {
		// This sorta half-assedly works in x86 because exception handling is slightly different, but since the main target is x64 we can just not care about it.
		// Something something ESP. ESP is a guitar brand.
#ifndef TARGET_MACHINE_X86

		// Returns the last Win32 error in string format. Returns an empty string if there is no error.
		static auto getLastWinErrorAsString = []() -> std::string {
			DWORD errorMessageID = GetLastError();
			if (errorMessageID == 0) {
				return "";
			}
			LPSTR messageBuffer = nullptr;
			DWORD messageFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

			// This bullshit makes the error string and returns the size because we can't know it in advance. Don't think we actually care about the size when we construct string from a buffer but whatever.
			size_t messageSize = FormatMessage(messageFlags, nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);
			std::string message(messageBuffer, messageSize);
			LocalFree(messageBuffer);
			return message;
		};

		// Returns a string with the type of the exception from the passed in code.
		static auto getExceptionDescriptionFromCode = [](const DWORD &exceptCode) -> std::string {
			switch (exceptCode) {
				case EXCEPTION_ACCESS_VIOLATION:			return "EXCEPTION_ACCESS_VIOLATION";
				case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:		return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
				case EXCEPTION_BREAKPOINT:					return "EXCEPTION_BREAKPOINT";
				case EXCEPTION_DATATYPE_MISALIGNMENT:		return "EXCEPTION_DATATYPE_MISALIGNMENT";
				case EXCEPTION_FLT_DENORMAL_OPERAND:		return "EXCEPTION_FLT_DENORMAL_OPERAND";
				case EXCEPTION_FLT_DIVIDE_BY_ZERO:			return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
				case EXCEPTION_FLT_INEXACT_RESULT:			return "EXCEPTION_FLT_INEXACT_RESULT";
				case EXCEPTION_FLT_INVALID_OPERATION:		return "EXCEPTION_FLT_INVALID_OPERATION";
				case EXCEPTION_FLT_OVERFLOW:				return "EXCEPTION_FLT_OVERFLOW";
				case EXCEPTION_FLT_STACK_CHECK:				return "EXCEPTION_FLT_STACK_CHECK";
				case EXCEPTION_FLT_UNDERFLOW:				return "EXCEPTION_FLT_UNDERFLOW";
				case EXCEPTION_ILLEGAL_INSTRUCTION:			return "EXCEPTION_ILLEGAL_INSTRUCTION";
				case EXCEPTION_IN_PAGE_ERROR:				return "EXCEPTION_IN_PAGE_ERROR";
				case EXCEPTION_INT_DIVIDE_BY_ZERO:			return "EXCEPTION_INT_DIVIDE_BY_ZERO";
				case EXCEPTION_INT_OVERFLOW:				return "EXCEPTION_INT_OVERFLOW";
				case EXCEPTION_INVALID_DISPOSITION:			return "EXCEPTION_INVALID_DISPOSITION";
				case EXCEPTION_NONCONTINUABLE_EXCEPTION:	return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
				case EXCEPTION_PRIV_INSTRUCTION:			return "EXCEPTION_PRIV_INSTRUCTION";
				case EXCEPTION_SINGLE_STEP:					return "EXCEPTION_SINGLE_STEP";
				case EXCEPTION_STACK_OVERFLOW:				return "EXCEPTION_STACK_OVERFLOW";
				default:
					return "UNKNOWN EXCEPTION";
			}
		};

		// Attempts to get a symbol name from the exception address.
		static auto getSymbolNameFromAddress = [](HANDLE &procHandle, const size_t &exceptAddr) {
			if (SymInitialize(procHandle, nullptr, TRUE)) {
				SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

				char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)];
				PSYMBOL_INFO symbolInfo = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);

				symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
				symbolInfo->MaxNameLen = MAX_SYM_NAME;

				if (SymFromAddr(procHandle, exceptAddr, nullptr, symbolInfo)) {
					std::string symbolName = symbolInfo->Name;
					return "The symbol name at this address is" + (symbolName.empty() ? " empty for reasons unknown to man." : ": \"" + symbolName + "\"");
				} else {
					return "Unable to get symbol name at address because:\n\n" + getLastWinErrorAsString();
				}
			}
			std::string error = getLastWinErrorAsString();
			return "Unable to get symbol name at address.\nSymbol Handler failed to initialize " + (error.empty() ? "for reasons unknown to man." : "because\n\n" + error);
		};

		HANDLE processHandle = GetCurrentProcess();

		std::stringstream exceptionDescription;
		DWORD exceptionCode = exceptPtr->ExceptionRecord->ExceptionCode;
		size_t exceptionAddress = reinterpret_cast<size_t>(exceptPtr->ExceptionRecord->ExceptionAddress);

		if (exceptionCode == EXCEPTION_BREAKPOINT) {
			// Advance to the next instruction otherwise this handler will be called for all eternity.
			exceptPtr->ContextRecord->Rip++;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		std::string symbolNameAtAddress = getSymbolNameFromAddress(processHandle, exceptionAddress);
		RTEError::FormatFunctionSignature(symbolNameAtAddress);

		exceptionDescription << getExceptionDescriptionFromCode(exceptionCode) << " at address 0x" << std::uppercase << std::hex << exceptionAddress << ".\n\n" << symbolNameAtAddress << std::endl;

		RTEStackTrace stackTrace;

		RTEError::UnhandledExceptionFunc(exceptionDescription.str(), stackTrace.GetCallStackAsString(processHandle, exceptPtr->ContextRecord));
		return EXCEPTION_EXECUTE_HANDLER;
#endif
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::SetExceptionHandlers() {
		// Basic handling for C++ exceptions. Doesn't give us much meaningful information.
		[[maybe_unused]] static const std::terminate_handler terminateHandler = []() {
			std::exception_ptr currentException = std::current_exception();

			if (currentException) {
				try {
					std::rethrow_exception(currentException);
				} catch (const std::bad_exception &exception) {
					RTEError::UnhandledExceptionFunc("Unable to get exception description because: " + std::string(exception.what()) + ".\n");
				} catch (const std::exception &exception) {
					RTEError::UnhandledExceptionFunc(std::string(exception.what()) + ".\n");
				}
			} else {
				RTEError::UnhandledExceptionFunc("Terminate was called without an exception.\nMay god have mercy on us all.");
			}
		};

#ifdef _WIN32
#ifndef TARGET_MACHINE_X86
		SetUnhandledExceptionFilter(RTEWindowsExceptionHandler);
#else
		// This only works for C++ exceptions and doesn't catch and access violations and such, or provide much meaningful info.
		std::set_terminate(terminateHandler);
#endif
#else
		// TODO: Deal with segfaults and such on other systems. Probably need to use Unix signal junk to get any meaningful information. Good luck and godspeed to whoever deals with this.
		std::set_terminate(terminateHandler);
#endif
	}

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
		enum AssertMessageButton { ButtonInvalid, ButtonAbort, ButtonIgnore, ButtonIgnoreAll };

		std::vector<SDL_MessageBoxButtonData> assertMessageBoxButtons = {
			SDL_MessageBoxButtonData(SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, AssertMessageButton::ButtonAbort, "Abort"),
			SDL_MessageBoxButtonData(0, AssertMessageButton::ButtonIgnore, "Ignore"),
			SDL_MessageBoxButtonData(0, AssertMessageButton::ButtonIgnoreAll, "Ignore All")
		};

		SDL_MessageBoxData assertMessageBox = {
			SDL_MESSAGEBOX_ERROR,
			g_WindowMan.GetWindow(),
			"RTE Assert! (x_x)",
			message.c_str(),
			static_cast<int>(assertMessageBoxButtons.size()),
			assertMessageBoxButtons.data(),
			nullptr
		};

		int pressedButton = AssertMessageButton::ButtonInvalid;
		SDL_ShowMessageBox(&assertMessageBox, &pressedButton);

		if (pressedButton == AssertMessageButton::ButtonIgnoreAll) {
			s_IgnoreAllAsserts = true;
		}

		return pressedButton == AssertMessageButton::ButtonAbort;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::UnhandledExceptionFunc(const std::string &description, const std::string &callstack) {
		s_CurrentlyAborting = true;

		std::string exceptionMessage = "Runtime Error due to unhandled exception!\n\n" + description;

		if (!s_LastIgnoredAssertDescription.empty()) {
			exceptionMessage += "\nThe last ignored Assertion was: " + s_LastIgnoredAssertDescription;
		}

		if (s_LastIgnoredAssertLocation.line() > 0) {
			// This typically contains the absolute path to the file on whatever machine this was compiled on, so in that case get only the file name.
			std::filesystem::path filePath = s_LastIgnoredAssertLocation.file_name();
			std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : s_LastIgnoredAssertLocation.file_name();
			std::string srcLocation = "file '" + fileName + "', line " + std::to_string(s_LastIgnoredAssertLocation.line()) + ",\nin function '" + s_LastIgnoredAssertLocation.function_name() + "'";

			if (!s_LastIgnoredAssertDescription.empty()) {
				exceptionMessage += "\nIn " + srcLocation + ".\n";
			} else {
				exceptionMessage += "\nThe last ignored Assertion was in " + srcLocation + ".\n";
			}
		}

		if (DumpAbortSave()) {
			exceptionMessage += "\nThe game has saved to 'AbortSave'.";
		}
		if (DumpAbortScreen()) {
			exceptionMessage += "\nThe last frame has been dumped to 'AbortScreen.png'.";
		}

		g_ConsoleMan.PrintString(exceptionMessage);

		std::string consoleSaveMsg;
		if (!callstack.empty()) {
			g_ConsoleMan.PrintString(callstack);
			consoleSaveMsg = "\nThe console and callstack have been dumped to 'AbortLog.txt'.";
		} else {
			consoleSaveMsg = "\nThe console has been dumped to 'AbortLog.txt'.";
		}
		if (g_ConsoleMan.SaveAllText("AbortLog.txt")) {
			exceptionMessage += consoleSaveMsg;
		}
		System::PrintToCLI(exceptionMessage);

		// Ditch the video mode so the message box appears without problems.
		if (g_WindowMan.GetWindow()) {
			SDL_SetWindowFullscreen(g_WindowMan.GetWindow(), 0);
		}

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RTE CATASTROPHIC ERROR!!! (X_X)", exceptionMessage.c_str(), nullptr);
		AbortAction;
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
			FormatFunctionSignature(funcName);

			std::string abortMessage = "Runtime Error in file '" + fileName + "', line " + lineNum + ",\nin function '" + funcName + "'\nbecause:\n\n" + description + "\n";

			if (DumpAbortSave()) {
				abortMessage += "\nThe game has saved to 'AbortSave'.";
			}
			if (DumpAbortScreen()) {
				abortMessage += "\nThe last frame has been dumped to 'AbortScreen.png'.";
			}

			g_ConsoleMan.PrintString(abortMessage);

			std::string callstack = "";

#ifdef _WIN32
			RTEStackTrace stackTrace;
			callstack += ("\n\n" + stackTrace.GetCallStackAsString());
#endif

			std::string consoleSaveMsg;
			if (!callstack.empty()) {
				g_ConsoleMan.PrintString(callstack);
				consoleSaveMsg = "\nThe console and callstack have been dumped to 'AbortLog.txt'.";
			} else {
				consoleSaveMsg = "\nThe console has been dumped to 'AbortLog.txt'.";
			}

			if (g_ConsoleMan.SaveAllText("AbortLog.txt")) {
				abortMessage += consoleSaveMsg;
			}
			System::PrintToCLI(abortMessage);

			// Ditch the video mode so the message box appears without problems.
			if (g_WindowMan.GetWindow()) {
				SDL_SetWindowFullscreen(g_WindowMan.GetWindow(), 0);
			}

			if (ShowAbortMessageBox(abortMessage)) {
				// Enable restarting in release builds only.
				// Once this exits the debugger is detached and while there does seem to be a way to programatically re-attach it to the new instance (at least in Windows), it is so incredibly ass and I cannot even begin to can.
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
		if (System::IsInExternalModuleValidationMode()) {
			AbortFunc(description, srcLocation);
		}

		// This typically contains the absolute path to the file on whatever machine this was compiled on, so in that case get only the file name.
		std::filesystem::path filePath = srcLocation.file_name();
		std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : srcLocation.file_name();

		std::string lineNum = std::to_string(srcLocation.line());
		std::string funcName = srcLocation.function_name();

		g_ConsoleMan.PrintString("ERROR: Assertion in file '" + fileName + "', line " + lineNum + ", in function '" + funcName + "' because: " + description);

		bool storeAssertInfo = false;

		if (!s_IgnoreAllAsserts) {
			std::string assertMessage =
				"Assertion in file '" + fileName + "', line " + lineNum + ",\nin function '" + funcName + "'\nbecause:\n\n" + description + "\n\n"
				"You may choose to ignore this and crash immediately\nor at some unexpected point later on.\n\nProceed at your own risk!";

			if (ShowAssertMessageBox(assertMessage)) {
				AbortFunc(description, srcLocation);
			} else {
				storeAssertInfo = true;
			}
		} else {
			storeAssertInfo = true;
		}

		if (storeAssertInfo) {
			s_LastIgnoredAssertDescription = description;
			s_LastIgnoredAssertLocation = srcLocation;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::DumpAbortScreen() {
		int success = -1;
		if (glReadPixels != nullptr) {
			int w ,h;
			SDL_GL_GetDrawableSize(g_WindowMan.GetWindow(), &w, &h);
			if (!(w>0 && h>0)) {
				return false;
			}
			BITMAP* readBuffer = create_bitmap_ex(24, w, h);
			// Read screen from the front buffer since that is the only framebuffer guaranteed to exist at this point.
			// Read twice because front buffer content is technically undefined, but most drivers still eventually give up the contents correctly.
			glReadBuffer(GL_FRONT);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, readBuffer->line[0]);
			glFinish();
			glReadBuffer(GL_BACK);
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, readBuffer->line[0]);
			glFinish();

			BITMAP* flipBuffer = create_bitmap_ex(24, w, h);
			draw_sprite_v_flip(flipBuffer, readBuffer, 0, 0);

			success = save_png("AbortScreen.png", flipBuffer, nullptr);
		}
		return success == 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEError::DumpAbortSave() {
		bool success = false;
		if (g_ActivityMan.GetActivity() && g_ActivityMan.GetActivity()->CanBeUserSaved()) {
			success = g_ActivityMan.SaveCurrentGame("AbortSave");
		}
		return success;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::FormatFunctionSignature(std::string &symbolName) {
		// TODO: Expand this with more dumb signatures, or make something that makes more sense.
		static const std::array<std::pair<std::regex, std::string>, 3> stlSigs {{
			{std::regex("( >)"), ">"},
			{std::regex("(std::basic_string<char,std::char_traits<char>,std::allocator<char>>)"), "std::string"},
			{std::regex("(class ?std::basic_string<char,struct ?std::char_traits<char>,class ?std::allocator<char>>)"), "std::string"}
		}};
		for (const auto &[fullSig, simpleSig] : stlSigs) {
			symbolName = std::regex_replace(symbolName, fullSig, simpleSig);
		}
		for (size_t pos = 0;;) {
			pos += 100;
			if (pos < symbolName.size()) {
				if (size_t lastCommaPos = symbolName.find_last_of(',', pos); lastCommaPos != std::string::npos) {
					symbolName.insert(lastCommaPos + 1, "\n");
				}
			} else {
				break;
			}
		}
	}
}