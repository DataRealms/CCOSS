#include "RTEError.h"

#include "SDL2/SDL_messagebox.h"
#include "FrameMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShowMessageBox(const std::string &message) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RTE Aborted *.*", message.c_str(), NULL); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEAbortFunc(const std::string &description, const std::string &file, int line) {
		// Save out the screen bitmap, after making a copy of it, faster sometimes
		if (screen) {
			BITMAP *abortScreenBuffer = create_bitmap(g_FrameMan.GetBackBuffer32()->w, g_FrameMan.GetBackBuffer32()->h);
			blit(g_FrameMan.GetBackBuffer32(), abortScreenBuffer, 0, 0, 0, 0, g_FrameMan.GetBackBuffer32()->w, g_FrameMan.GetBackBuffer32()->h);
			PALETTE palette;
			get_palette(palette);
			save_bmp("AbortScreen.bmp", abortScreenBuffer, palette);
			destroy_bitmap(abortScreenBuffer);
		}
		// Ditch the video mode so the message box appears without problems
		if (g_FrameMan.GetWindow()) {
			SDL_SetWindowFullscreen(g_FrameMan.GetWindow(), 0);
			SDL_SetWindowTitle(g_FrameMan.GetWindow(), "RTE Aborted! (x_x)");
		}

		std::string abortMessage;

#ifndef RELEASE_BUILD
		// Show message box with explanation
		abortMessage = "Runtime Error in file " + file + ", line " + std::to_string(line) + ", because:\n\n" + description + "\n\nThe last frame has been dumped to 'AbortScreen.bmp'";
#else
		// Shortened and less confusing one. users have no use of knowing which source file and where.
		abortMessage = description + "\n\nThe last frame has been dumped to 'AbortScreen.bmp'";
#endif
		ShowMessageBox(abortMessage);

		// True so that the debugbreak code is run and the debugger goes there.
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool RTEAssertFunc(bool expression, const char *description, const char *file, int line, bool &alwaysIgnore) {
		if (!expression) {
			// TODO: Make this display a box in the game asking whether to ignore or abort. For now, always abort.
			RTEAbortFunc(description, __FILE__, __LINE__);

			// True so that the debugbreak code is run and the debugger goes there.
			return true;
		}
		return false;
	}
}
