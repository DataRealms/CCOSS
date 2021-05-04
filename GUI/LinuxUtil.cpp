#include "LinuxUtil.h"

#include "ConsoleMan.h"

#include <allegro.h>
#include <xalleg.h>
#include <limits>

/*Code borrowed from SDL2*/
#ifdef X_HAVE_UTF8_STRING
#define TEXT_FORMAT XInternAtom(display, "UTF8_STRING", False)
#else
#define TEXT_FORMAT XInternAtom(display, "XA_STRING", False)
#endif

namespace RTE {
	Window LinuxUtil::s_ClipboardWindow;

	LinuxUtil::~LinuxUtil(){
		if(s_ClipboardWindow){
			XDestroyWindow(_xwin.display, s_ClipboardWindow);
		}
	}

	Window LinuxUtil::GetWindow(){
		if(!s_ClipboardWindow){
			XSetWindowAttributes xattr;
			s_ClipboardWindow = XCreateWindow(
			    _xwin.display,
			    RootWindow(_xwin.display, DefaultScreen(_xwin.display)), -10,
			    -10, 1, 1, 0, CopyFromParent, InputOnly, CopyFromParent, 0,
			    &xattr);
		}

		return s_ClipboardWindow;
	}

	bool LinuxUtil::GetClipboardText(std::string *Text) {
		Display *display = _xwin.display;
		Atom format;
		Window window;
		Window owner;
		Atom selection;
		Atom seln_type;
		int seln_format;
		unsigned long nbytes;
		unsigned long overflow;
		unsigned char *src;
		char *text{nullptr};
		Atom XA_CLIPBOARD = XInternAtom(display, "CLIPBOARD", 0);
		if (XA_CLIPBOARD == None) {
			g_ConsoleMan.PrintString("ERROR: Couldn't access X clipboard");
			return false;
		}

		/* Get the window that holds the selection */
		window = GetWindow();
		format = TEXT_FORMAT;
		owner = XGetSelectionOwner(display, XA_CLIPBOARD);
		if (owner == None) {
			/* Fall back to ancient X10 cut-buffers which do not support UTF8
			 * strings*/
			owner = DefaultRootWindow(display);
			selection = XInternAtom(display, "XA_CUT_BUFFER0", False);
			format = XInternAtom(display, "XA_STRING", False);
		} else if (owner == window) {
			owner = DefaultRootWindow(display);
			selection = XInternAtom(display, "CC_CUTBUFFER", False);
		} else {
			/* Request that the selection owner copy the data to our window */
			owner = window;
			selection = XInternAtom(display, "CC_SELECTION", False);
			XConvertSelection(display, XA_CLIPBOARD, format, selection, owner,
			                  CurrentTime);
		}

		if (XGetWindowProperty(display, owner, selection, 0, std::numeric_limits<int>::max() / 4, False,
		                       format, &seln_type, &seln_format, &nbytes,
		                       &overflow, &src) == Success) {
			if (seln_type == format) {
				text = (char *)std::malloc(nbytes + 1);
				if (text) {
					std::memcpy(text, src, nbytes);
					text[nbytes] = '\0';
					Text = new std::string(text);
				}
			}
			XFree(src);
		}

		if (!text || Text->compare("")) {
			return false;
		}

		return true;
	}

	bool LinuxUtil::SetClipboardText(std::string Text) {
		Display *display = _xwin.display;
		Atom format;
		Window window;
		Atom XA_CLIPBOARD = XInternAtom(display, "CLIPBOARD", False);
		Atom XA_PRIMARY = XInternAtom(display, "PRIMARY", False);
		Atom CC_CUTBUFFER = XInternAtom(display, "CC_CUTBUFFER", False);

		/* Get the SDL window that will own the selection */
		window = GetWindow();
		if (window == None) {
			g_ConsoleMan.PrintString("ERROR: Couldn't find a window to own the selection");
			return false;
		}

		/* Save the selection on the root window */
		format = TEXT_FORMAT;
		XChangeProperty(display, DefaultRootWindow(display), CC_CUTBUFFER,
		                format, 8, PropModeReplace,
		                reinterpret_cast<const unsigned char *>(Text.c_str()),
		                Text.length());

		if (XA_CLIPBOARD != None &&
		    XGetSelectionOwner(display, XA_CLIPBOARD) != window) {
			XSetSelectionOwner(display, XA_CLIPBOARD, window, CurrentTime);
		}

		if (XGetSelectionOwner(display, XA_PRIMARY) != window) {
			XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
		}
		return true;
	}
} // namespace RTE
