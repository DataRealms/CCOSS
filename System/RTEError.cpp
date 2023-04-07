#include "RTEError.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"

#include "SDL_messagebox.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShowMessageBox(const std::string &message, bool abortMessage) {
		const char *messageBoxTitle = "RTE Warning! (>_<)";
		int messageBoxFlags = SDL_MESSAGEBOX_WARNING;

		if (abortMessage) {
			messageBoxTitle = "RTE Aborted! (x_x)";
			messageBoxFlags = SDL_MESSAGEBOX_ERROR;
		}

		SDL_ShowSimpleMessageBox(messageBoxFlags, messageBoxTitle, message.c_str(), nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEAbortFunc(const std::string &description, const std::string &file, int line) {
		// We attempt to save the game when aborting, but this could potentially lead to a recursive fault if the saving itself is aborting.
		static bool currentAborting = false;
		if (currentAborting) {
			// We're going in a loop! Just ignore all future aborts and let the saving hopefully complete, until the stack unrolls.
			return;
		}

		currentAborting = true;

		if (!System::IsInExternalModuleValidationMode()) {
			// Attempt to save the game itself, so the player can hopefully resume where they were.
			g_ActivityMan.SaveCurrentGame("AbortSave");

			// Save out the screen bitmap, after making a copy of it, faster sometimes.
			if (screen) {
				int backbufferWidth = g_FrameMan.GetBackBuffer32()->w;
				int backbufferHeight = g_FrameMan.GetBackBuffer32()->h;
				BITMAP *abortScreenBuffer = create_bitmap(backbufferWidth, backbufferHeight);
				blit(g_FrameMan.GetBackBuffer32(), abortScreenBuffer, 0, 0, 0, 0, backbufferWidth, backbufferHeight);
				save_bmp("AbortScreen.bmp", abortScreenBuffer, nullptr);
				destroy_bitmap(abortScreenBuffer);
			}

			// Ditch the video mode so the message box appears without problems.
			if (g_WindowMan.GetWindow()) {
				SDL_SetWindowFullscreen(g_WindowMan.GetWindow(), 0);
			}

			// This typically gets passed __FILE__ which contains the full path to the file from whatever machine this was compiled on, so in that case get only the file name.
			std::filesystem::path filePath = file;
			std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : file;

			std::string abortMessage = "Runtime Error in file '" + fileName + "', line " + std::to_string(line) + ", because:\n\n" + description + "\n\nThe game has attempted to save to 'AbortSave'.\nThe console has been dumped to 'AbortLog.txt'.\nThe last frame has been dumped to 'AbortScreen.bmp'.";

			g_ConsoleMan.PrintString(abortMessage);
			g_ConsoleMan.SaveAllText("AbortLog.txt");
			System::PrintToCLI(abortMessage);

			ShowMessageBox(abortMessage, true);
		}

		currentAborting = false;
		AbortAction;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEAssertFunc(const std::string &description, const char *file, int line) {
		// TODO: Make this display a box in the game asking whether to ignore or abort. For now, always abort.
		RTEAbortFunc(description, file, line);
	}
}
