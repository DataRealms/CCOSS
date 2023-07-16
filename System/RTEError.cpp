#include "RTEError.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "ActivityMan.h"

#include "SDL_messagebox.h"

namespace RTE {

	bool RTEError::s_CurrentlyAborting = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::ShowMessageBox(const std::string &message, bool abortMessage) {
		const char *messageBoxTitle = "RTE Warning! (>_<)";
		int messageBoxFlags = SDL_MESSAGEBOX_WARNING;

		if (abortMessage) {
			messageBoxTitle = "RTE Aborted! (x_x)";
			messageBoxFlags = SDL_MESSAGEBOX_ERROR;
		}

		SDL_ShowSimpleMessageBox(messageBoxFlags, messageBoxTitle, message.c_str(), nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::AbortFunc(const std::string &description, const std::string &file, int line) {
		s_CurrentlyAborting = true;

		if (!System::IsInExternalModuleValidationMode()) {
			// Attempt to save the game itself, so the player can hopefully resume where they were.
			bool abortSaveMade = false;
			if (g_ActivityMan.GetActivityAllowsSaving()) {
				abortSaveMade = g_ActivityMan.SaveCurrentGame("AbortSave");
			}

			if (BITMAP *backbuffer = g_FrameMan.GetBackBuffer32(); backbuffer) {
				// Have to convert the 32bpp backbuffer to 24bpp otherwise the saved file is blank for reasons that don't matter.
				BITMAP *abortScreenBuffer = create_bitmap_ex(24, backbuffer->w, backbuffer->h);
				blit(backbuffer, abortScreenBuffer, 0, 0, 0, 0, backbuffer->w, backbuffer->h);
				save_png("AbortScreen.png", abortScreenBuffer, nullptr);
				destroy_bitmap(abortScreenBuffer);
			}

			// Ditch the video mode so the message box appears without problems.
			if (g_WindowMan.GetWindow()) {
				SDL_SetWindowFullscreen(g_WindowMan.GetWindow(), 0);
			}

			// This typically gets passed __FILE__ which contains the full path to the file from whatever machine this was compiled on, so in that case get only the file name.
			std::filesystem::path filePath = file;
			std::string fileName = (filePath.has_root_name() || filePath.has_root_directory()) ? filePath.filename().generic_string() : file;

			std::string abortMessage = "Runtime Error in file '" + fileName + "', line " + std::to_string(line) + ", because:\n\n" + description + "\n\n";
			if (abortSaveMade) {
				abortMessage += "The game has saved to 'AbortSave'.\n";
			}
			abortMessage += "The console has been dumped to 'AbortLog.txt'.\nThe last frame has been dumped to 'AbortScreen.bmp'.";

			g_ConsoleMan.PrintString(abortMessage);
			g_ConsoleMan.SaveAllText("AbortLog.txt");
			System::PrintToCLI(abortMessage);

			ShowMessageBox(abortMessage, true);
		}
		s_CurrentlyAborting = false;
		AbortAction;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEError::AssertFunc(const std::string &description, const char *file, int line) {
		// TODO: Make this display a box in the game asking whether to ignore or abort. For now, always abort.
		AbortFunc(description, file, line);
	}
}
