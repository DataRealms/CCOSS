#include "OsxUtil.h"

using namespace RTE;
using namespace std;

#define OSX_RESULT_SUCCESS 0
#define OSX_RESULT_FAILURE 1

PasteboardRef OsxUtil::m_pasteboard = NULL;

int InitPasteboard(PasteboardRef* ref)
{
	int rc = OSX_RESULT_SUCCESS;
	
	if (PasteboardCreate(kPasteboardClipboard, ref))
		rc = OSX_RESULT_FAILURE;
	
	return rc;
}

void DestroyPasteboard(PasteboardRef ref)
{
	CFRelease(ref);
}

int ReadFromPasteboard(PasteboardRef ref, char* pv)
{
	OSStatus error = noErr;
	
	PasteboardSynchronize(ref);
	
	ItemCount itemCount;
	error = PasteboardGetItemCount(ref, &itemCount);
	if (itemCount < 1)
		return OSX_RESULT_SUCCESS;
	
	int32_t rc = OSX_RESULT_FAILURE;
	PasteboardItemID itemID;
	
	if (!(error = PasteboardGetItemIdentifier(ref, 1, &itemID)))
	{
		CFDataRef dataOut;
		char* ptr = NULL;
		
		if (!(error = PasteboardCopyItemFlavorData(ref, itemID, CFSTR("public.utf8-plain-text"), &dataOut)))
		{			
			ptr = strdup((const char*)CFDataGetBytePtr(dataOut));
			if (ptr)
			{
				rc = OSX_RESULT_SUCCESS;
			}
			
			strncpy(pv, ptr, strlen(ptr));
		}
		
		free(ptr);
	}
	
	return rc;
}

int WriteToPasteboard(PasteboardRef ref, const char* pv)
{
	if (PasteboardClear(ref))
	{
		return OSX_RESULT_FAILURE;
	}
	
	// Make sure in sync
	PasteboardSynchronize(ref);
	
	int32_t len = strlen(pv);
	
	if (len == 0)
	{
		// no data just return success
		return OSX_RESULT_SUCCESS;
	}
	
	// allocate memory for text 
	char* destText = static_cast<char*>(malloc(len));
	
	strncpy(destText, pv, len);
	
	CFDataRef textData = NULL;
	// nothing fancy just set ID to 1
	PasteboardItemID itemID = (PasteboardItemID)1;
	
	// Create a CData object which we can pass to the pasteboard
	if ((textData = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<UInt8*>(destText), len)))
	{
		PasteboardPutItemFlavor(ref, itemID, CFSTR("public.utf8-plain-text"), textData, 0);
	}
	
	free(destText);
	
	return OSX_RESULT_SUCCESS;
}

bool OsxUtil::Create()
{
	if (m_pasteboard == NULL)
	{
		InitPasteboard(&m_pasteboard);
	}
	return true;
}

bool OsxUtil::Destroy()
{
	DestroyPasteboard(m_pasteboard);
	m_pasteboard = NULL;
	return true;
}

bool OsxUtil::GetClipboardText(string* text)
{	
	char buffer[256];

	memset(buffer, 0, 256);
	
	ReadFromPasteboard(m_pasteboard, buffer);
	
	if (strlen(buffer) > 0)
	{
		text->erase();
		text->insert(0, buffer);
	}
	
	return true;
}

bool OsxUtil::SetClipboardText(string text)
{
	WriteToPasteboard(m_pasteboard, text.c_str());
	
	return true;
}