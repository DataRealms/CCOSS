#ifndef _LINUXUTIL_
#define _LINUXUTIL_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIUtil.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIUtil class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal

namespace RTE {

	///<summary>
	///A utility class with misc static functions for different things, platform specific to
	///linux
	///</summary>
	class LinuxUtil {

		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:
		///<summary>
		///Constructor method used to instantiate a LinuxUtil object in system memory
		///</summary>
		LinuxUtil(){};

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetClipboardText
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the text from the clipboard.
		// Arguments:       Pointer to string receiving the text.
		// Returns:         Boolean. True if text was available in the clipboard.

    ///<summary>
    /// Gets the text from the Atom CLIPBOARD
    ///</summary>
    ///<param name="Text">Pointer to string receiving the text</param>
    ///<returns> Boolean. True if text was available in the clipboard</returns>
		static bool GetClipboardText(std::string *Text);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          SetClipboardText
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Sets the text in the clipboard.
		// Arguments:       String to put into the clipboard.
		// Returns:         Boolean. True if text was added to the clipboard.
    ///<summary>
    /// Sets the text in the Atom CLIPBOARD
    ///</summary>
    ///<param name="Text">String to put into the clipboard</param>
    ///<returns>Boolean. True if text was added to the clipboard</returns>
		static bool SetClipboardText(std::string Text);
	};

}; // namespace RTE

#endif //  _GUIUTIL_
