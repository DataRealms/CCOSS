//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIUtil.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIUtil class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrimString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes the preceeding and ending spaces from a c type string.

char *GUIUtil::TrimString(char *String)
{
    char *ptr = String;

    // Find the first non-space character
    while(*ptr) {
        if (*ptr != ' ')
            break;
        ptr++;
    }

    // Add a null terminator after the last character
    for(int i=strlen(ptr)-1; i>=0; i--) {
        if (ptr[i] != ' ') {
            ptr[i+1] = '\0';
            break;
        }
    }

    return ptr;
}

char* GUIUtil::SafeOverlappingStrCpy(char* dst, char* src)
{
	memmove(dst, src, strlen(src) + 1);
	return dst;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClipboardText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text from the clipboard.

bool GUIUtil::GetClipboardText(string *Text)
{
/* Platform dependent, moved to WinUtil
    HANDLE  CBDataHandle; // handle to the clipboard data
    LPSTR   CBDataPtr;    // pointer to data to send

    // Check the pointer
    assert(Text);
    
    // Does the clipboard contain text?
    if (IsClipboardFormatAvailable(CF_TEXT)) {
           
        // Open the clipboard
        if (OpenClipboard(m_hWnd)) {
            CBDataHandle = GetClipboardData(CF_TEXT);
                
            if (CBDataHandle) {
                CBDataPtr = (LPSTR)GlobalLock(CBDataHandle);
                int TextSize = strlen(CBDataPtr);

                // Insert the text
                Text->erase();
                Text->insert(0, CBDataPtr);
                CloseClipboard();

                GlobalUnlock(CBDataHandle);                

                return true;
            }

            CloseClipboard();
        }
    }
*/
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetClipboardText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text in the clipboard.

bool GUIUtil::SetClipboardText(string Text)
{
/* Platform dependent
    // Open the clipboard
    if (OpenClipboard(m_hWnd)) {

        // Allocate global memory for the text
        HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, Text.size()+1);
        if (hMemory == 0) {
            CloseClipboard();
            return false;
        }

        // Empty the clipboard
        EmptyClipboard(); 

        // Copy the text into memory
        char *CText = (char *)GlobalLock(hMemory);
        memcpy(CText, Text.c_str(), Text.size());
        CText[Text.size()] = '\0';
        GlobalUnlock(hMemory);    

        // Set the data
        SetClipboardData(CF_TEXT, hMemory);

        CloseClipboard();

        return true;
    }
*/
    return false;
}
