#include "RTEError.h"

#include "ActivityMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShowMessageBox(const std::string &message) { allegro_message(message.c_str()); }

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
				BITMAP *abortScreenBuffer = create_bitmap(screen->w, screen->h);
				blit(screen, abortScreenBuffer, 0, 0, 0, 0, screen->w, screen->h);
				save_bmp("AbortScreen.bmp", abortScreenBuffer, nullptr);
				destroy_bitmap(abortScreenBuffer);
			}

			// Ditch the video mode so the message box appears without problems.
			if (screen != nullptr) { set_gfx_mode(GFX_TEXT, 0, 0, 0, 0); }
			set_window_title("RTE Aborted! (x_x)");

			// This typically gets passed __FILE__ which contains the full path to the file from whatever machine this was compiled on, so in that case get only the file name.
			std::filesystem::path filePath = file;
			std::string fileName = (filePath.has_root_name() || filePath.has_root_directory() ? filePath.filename().generic_string() : file);

			std::string abortMessage = "Runtime Error in file '" + fileName + "', line " + std::to_string(line) + ", because:\n\n" + description + "\n\nThe game has attempted to save to 'AbortSave'.\nThe last frame has been dumped to 'AbortScreen.bmp'.";
			ShowMessageBox(abortMessage);
		}

		currentAborting = false;
		AbortAction;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEAssertFunc(const std::string &description, const char *file, int line, bool &alwaysIgnore) {
		// TODO: Make this display a box in the game asking whether to ignore or abort. For now, always abort.
		RTEAbortFunc(description, file, line);
	}
}