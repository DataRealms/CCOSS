//////////////////////////////////////////////////////////////////////////////////////////
// File:            WinUtil.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     WinUtil class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <Windows.h>
#include "WinUtil.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClipboardText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the text from the clipboard.

bool WinUtil::GetClipboardText(std::string *Text)
{
    HANDLE  CBDataHandle; // handle to the clipboard data
    LPSTR   CBDataPtr;    // pointer to data to send

    // Check the pointer
    assert(Text);
    
    // Does the clipboard contain text?
    if (IsClipboardFormatAvailable(CF_TEXT))
    {
        // Open the clipboard
        if (OpenClipboard(0))
        {
            CBDataHandle = GetClipboardData(CF_TEXT);
                
            if (CBDataHandle)
            {
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
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetClipboardText
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the text in the clipboard.

bool WinUtil::SetClipboardText(std::string Text)
{
    // Open the clipboard
    if (OpenClipboard(0))
    {
        // Allocate global memory for the text
        HGLOBAL hMemory = GlobalAlloc(GMEM_MOVEABLE, Text.size()+1);
        if (hMemory == 0)
        {
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
    return false;
}