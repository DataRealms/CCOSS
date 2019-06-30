//////////////////////////////////////////////////////////////////////////////////////////
// File:            Editor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Entry point file for the GUI Library editor
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jboett@internode.on.net



//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

/*#include <windows.h>
#include <windowsx.h>
#include <stdio.h>*/


#include "TimerMan.h"
#include "GUIEditorApp.h"
#include "GUIEditorLib.h"

#define RECT WINDOWS_RECT

#include "allegro.h"
#include "winalleg.h"
#undef RECT


extern "C" { FILE __iob_func[3] = { *stdin, *stdout, *stderr }; }

using namespace RTE;

namespace RTE {
	
	TimerMan	g_TimerMan;
};

GUIEditorApp g_GUIEditor;

extern HINSTANCE g_hInstance = 0;
extern HWND g_hWnd = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			main
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Entry point for the GUI Editor

int main(int argc, char *argv[])
{
	if(!g_GUIEditor.Initialize())
		return -1;
	
	// Get windows settings
	g_hWnd = win_get_window();
	g_hInstance = (HINSTANCE)GetWindowLong(g_hWnd, GWL_HINSTANCE);


	// Run editor loop
	while(1) {
		bool bContinue = g_GUIEditor.Update();
		if(!bContinue)
			break;
	}



	return 0;
}
END_OF_MAIN();