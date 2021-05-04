#include "LinuxUtil.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace RTE {
	bool LinuxUtil::GetClipboardText(std::string *Text) { return false; }

	bool LinuxUtil::SetClipboardText(std::string Text) { return false; }
} // namespace RTE
