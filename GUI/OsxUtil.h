#ifndef _OSX_UTIL_H_
#define _OSX_UTIL_H_

// OSX
#include <Carbon/Carbon.h>
// std
#include <string>

namespace RTE
{

class OsxUtil
{
public:
	
	OsxUtil() {};
	
	static bool Create();
	static bool Destroy();
	
	static bool GetClipboardText(std::string* text);
	
	static bool SetClipboardText(std::string text);
	
private:
	
	static PasteboardRef m_pasteboard;
};
	
} // namespace RTE

#endif // _OSX_UTIL_H_
