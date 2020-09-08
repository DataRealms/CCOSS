#ifndef _LINUXUTIL_
#define _LINUXUTIL_

namespace RTE {

	///<summary>
	///A utility class with misc static functions for different things, platform specific to
	///linux
	///</summary>
	class LinuxUtil {

	public:
		///<summary>
		///Constructor method used to instantiate a LinuxUtil object in system memory
		///</summary>
		LinuxUtil() {};

		///<summary>
		/// Gets the text from the Atom CLIPBOARD
		///</summary>
		///<param name="Text">Pointer to string receiving the text</param>
		///<returns> Boolean. True if text was available in the clipboard</returns>
		static bool GetClipboardText(std::string *Text);

		///<summary>
		/// Sets the text in the Atom CLIPBOARD
		///</summary>
		///<param name="Text">String to put into the clipboard</param>
		///<returns>Boolean. True if text was added to the clipboard</returns>
		static bool SetClipboardText(std::string Text);
	};

}; // namespace RTE

#endif //  _LINUXUTIL_
