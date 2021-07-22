#include "GUIUtil.h"

#ifdef _WIN32
#include "Windows.h"
#endif

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
#ifdef _WIN32
		HANDLE clipboardDataHandle;
		LPSTR clipboardData;
		assert(text);

		if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(nullptr)) {
			clipboardDataHandle = GetClipboardData(CF_TEXT);
			if (clipboardDataHandle) {
				clipboardData = static_cast<LPSTR>(GlobalLock(clipboardDataHandle));
				text->erase();
				text->insert(0, clipboardData);
				CloseClipboard();

				GlobalUnlock(clipboardDataHandle);
				return true;
			}
			CloseClipboard();
		}
#elif __unix__
		// TODO: Implement.
#endif
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIUtil::SetClipboardText(std::string text) {
#ifdef _WIN32
		if (OpenClipboard(nullptr)) {
			// Allocate global memory for the text
			HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
			if (hMemory == nullptr) {
				CloseClipboard();
				return false;
			}
			EmptyClipboard();

			// Copy the text into memory
			char *clipboardText = static_cast<char *>(GlobalLock(hMemory));
			memcpy(clipboardText, text.c_str(), text.size());
			clipboardText[text.size()] = '\0';
			GlobalUnlock(hMemory);

			SetClipboardData(CF_TEXT, hMemory);
			CloseClipboard();
			return true;
		}
#elif __unix__
		// TODO: Implement.
#endif
		return false;
	}
}