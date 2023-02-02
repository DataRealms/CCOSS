#include "RTEError.h"

// For saving our game on failure
#include "ActivityMan.h"

#include "SDL2/SDL_messagebox.h"
#include "FrameMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ShowMessageBox(const std::string &message) { SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RTE Aborted *.*", message.c_str(), NULL); }

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

			// Show message box with explanation
			abortMessage = "Runtime Error in file " + file + ", line " + std::to_string(line) + ", because:\n\n" + description + "\n\nThe game has attempted to save to 'AbortSave'.'";

			ShowMessageBox(abortMessage);
		}

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
