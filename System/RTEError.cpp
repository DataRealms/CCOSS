#include "RTEError.h"
#include "Constants.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern bool RTEAbortFunc(const char *description, const char *file, int line) {
		// Save out the screen bitmap, after making a copy of it, faster sometimes
		if (screen) {
			if (!g_pScreendumpBuffer) { g_pScreendumpBuffer = create_bitmap(screen->w, screen->h); }
			blit(screen, g_pScreendumpBuffer, 0, 0, 0, 0, screen->w, screen->h);
			PALETTE palette;
			get_palette(palette);
			save_bmp("abortscreen.bmp", g_pScreendumpBuffer, palette);
		}
		// Ditch the video mode so the message box appears without problems
		if (screen != 0) { set_gfx_mode(GFX_TEXT, 0, 0, 0, 0); }
		// Set title of the messagebox
		set_window_title("RTE Aborted! (x_x)");

		char message[512];

#if defined DEBUG_BUILD || defined MIN_DEBUG_BUILD 	
		// Show message box with explanation
		sprintf_s(message, sizeof(message), "Abortion in file %s, line %i, because:\n\n%s\n\nThe last frame has been dumped to 'abortscreen.bmp'", file, line, description);
		allegro_message(message);
#else
		// Shortened and less confusing one. users have no use of knowing which source file and where.
		sprintf_s(message, sizeof(message), "%s\n\nThe last frame has been dumped to 'abortscreen.bmp'\n\nYou can copy this message with Ctrl+C", description);
		allegro_message(message);
#endif
		// True so that the debugbreak code is run and the debugger goes there.
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern bool RTEAbortFunc(const std::string description, const char *file, int line) {
		return RTEAbortFunc(description.c_str(), file, line);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEAssertFunc(bool expression, const char *description, const char *file, int line, bool &alwaysIgnore) {
		if (!expression) {
			// TODO: Make this display a box in the game asking whether to ignore or abort")
			// For now, always abort.
			char error[512];
			sprintf_s(error, sizeof(error), "Assertion failed: %s", description);
			RTEAbortFunc(error, __FILE__, __LINE__);

			// True so that the debugbreak code is run and the debugger goes there.
			return true;
		}
		// Assert didn't fail
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	extern bool RTEAssertFunc(bool expression, std::string description, const char *file, int line, bool &alwaysIgnore) {
		return RTEAssertFunc(expression, description.c_str(), file, line, alwaysIgnore);
	}
}