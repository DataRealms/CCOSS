#include "RTEError.h"

// For saving our game on failure
#include "ActivityMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShowMessageBox(const std::string &message) { allegro_message(message.c_str()); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEAbortFunc(const std::string &description, const std::string &file, int line) {
		// We attempt to save the game when aborting, but this could potentially lead to a recursive fault if the saving itself is aborting
		// So ensure we're not recursing back in here
		static bool currentAborting = false;
		if (currentAborting) {
			// Crap, we're going in a loop. Just ignore all future aborts and let the saving hopefully complete, until the stack unrolls
			return;
		}

		currentAborting = true;

		// Save out the screen bitmap, after making a copy of it, faster sometimes
		if (screen) {
			BITMAP *abortScreenBuffer = create_bitmap(screen->w, screen->h);
			blit(screen, abortScreenBuffer, 0, 0, 0, 0, screen->w, screen->h);
			PALETTE palette;
			get_palette(palette);
			save_bmp("AbortScreen.bmp", abortScreenBuffer, palette);
			destroy_bitmap(abortScreenBuffer);
		}

		// Ditch the video mode so the message box appears without problems
		if (screen != nullptr) { set_gfx_mode(GFX_TEXT, 0, 0, 0, 0); }
		set_window_title("RTE Aborted! (x_x)");

		std::string abortMessage;

		// Show message box with explanation
		abortMessage = "Runtime Error in file " + file + ", line " + std::to_string(line) + ", because:\n\n" + description + "\n\nThe game has attempted to save to 'AbortSave'.'";
		ShowMessageBox(abortMessage);

		// Attempt to save the game itself, so the player can hopefully resume where they were
		g_ActivityMan.SaveCurrentGame("AbortSave");

		currentAborting = false;
		AbortAction;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void RTEAssertFunc(const char *description, const char *file, int line, bool &alwaysIgnore) {
		// TODO: Make this display a box in the game asking whether to ignore or abort. For now, always abort.
		RTEAbortFunc(description, __FILE__, __LINE__);
	}
}