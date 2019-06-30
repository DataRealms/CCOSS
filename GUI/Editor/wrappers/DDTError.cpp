//////////////////////////////////////////////////////////////////////////////////////////
// File:            DDTError.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for general error handling functions.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DDTError.h"
#include "allegro.h"


namespace RTE
{

static BITMAP *g_pScreendumpBuffer = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DDTAbortFunc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Abort on Error function. Will try to dump a screenshot, show an
//                  abortion message, and then quit the program immediately.
// Arguments:       The description of the abortion.
//                  The source file in which the abortion is made.
//                  The line where the abortion is made.
// Return value:    None.

extern bool DDTAbortFunc(const char *description,
                         const char *file,
                         int line)
{
    // Save out the screen bitmap, after making a copy of it, faster sometimes
    if (screen) {
        if (!g_pScreendumpBuffer)
            g_pScreendumpBuffer = create_bitmap(screen->w, screen->h);
        blit(screen, g_pScreendumpBuffer, 0, 0, 0, 0, screen->w, screen->h);
        PALETTE palette;
        get_palette(palette);
        save_bmp("abortscreen.bmp", g_pScreendumpBuffer, palette);
    }

    // Ditch the video mode so the messagebox appears without problems
    if (screen != 0)
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

    // Set title of the messagebox
    set_window_title("RTE Aborted! (x_x)");

    // Show messagebox with explanation
//    allegro_message("Abortion in file %s, line %i, because:\n\n%s\n\nThe last frame has been dumped to 'abortscreen.bmp'", file, line, description);
    // Shortened and less confusing one.. users have no use of knowing which source file and where
#ifdef WIN32
    allegro_message("%s\n\nThe last frame has been dumped to 'abortscreen.bmp'\n\nYou can copy this message with Ctrl+C", description);
#else // WIN32
    allegro_message("%s\n\nThe last frame has been dumped to 'abortscreen.bmp'", description);
#endif // WIN32
    // True so that the asm_ { int 3 } code is run and the debugger goes there.
//    exit(-1);
    return true;
}


extern bool DDTAbortFunc(const std::string description,
                         const char *file,
                         int line)
{
    return DDTAbortFunc(description.c_str(), file, line);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DDTAssert
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A souped-up, customized assert function that brings up a nice dialog
//                  box on assert failure, where the user can choose to: break, ignore
//                  the particular assertion failure once, or to always ignore.

bool DDTAssert(bool expression,
               const char *description,
               const char *file,
               int line,
               bool &alwaysIgnore)
{
//    ASSERT(expression);

    if (!expression) {
/*
        int response;
        char string[512];
        sprintf(string, "What: %s\nWhere: %s - Line %i", description, file, line);
        response = MessageBox(0,
                              string,
                              "Assertion Failure!",
                              MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
        if (response == IDABORT)
            return true;
        if (response == IDRETRY)
            return false;
        if (response == IDIGNORE) {
            alwaysIgnore = true;
            return false;
        }
*/
// TODO: Make this display a box in the game asking whether to ignore or abort")
        // For now, always abort.
        char error[512];
        sprintf(error, "Assertion failed: %s", description);
        DDTAbortFunc(error, __FILE__, __LINE__);

        // True so that the asm_ { int 3 } code is run and the debugger goes there.
        return true;
    }

    // Assert didn't fail
    return false;
}


extern bool DDTAssert(bool expression,
                      std::string description,
                      const char *file,
                      int line,
                      bool &alwaysIgnore)
{
    return DDTAssert(expression, description.c_str(), file, line, alwaysIgnore);
}

} // namespace RTE
