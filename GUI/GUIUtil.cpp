#include "GUIUtil.h"

#include "SDL_clipboard.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	char * GUIUtil::TrimString(char *String) {
		char *ptr = String;
		// Find the first non-space character
		while (*ptr) {
			if (*ptr != ' ') {
				break;
			}
			ptr++;
		}
		// Add a null terminator after the last character
		for (int i = strlen(ptr) - 1; i >= 0; i--) {
			if (ptr[i] != ' ') {
				ptr[i + 1] = '\0';
				break;
			}
		}
		return ptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIUtil::GetClipboardText(std::string *text) {
		if (SDL_HasClipboardText()) {
			*text = SDL_GetClipboardText();
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIUtil::SetClipboardText(const std::string &text) {
		int result = SDL_SetClipboardText(text.c_str());
		return result == 0;
	}
}
