#include "GUI.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace RTE {
	bool GUIUtil::GetClipboardText(std::string *Text) { return false; }

	bool GUIUtil::SetClipboardText(std::string Text) { return false; }
} // namespace RTE
