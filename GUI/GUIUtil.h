#ifndef _GUIUTIL_
#define _GUIUTIL_

namespace RTE {

	/// <summary>
	/// A utility class with misc static functions for different things.
	/// </summary>
	class GUIUtil {

	public:

		/// <summary>
		/// Removes the preceding and ending spaces from a c type string.
		/// </summary>
		/// <param name="String">String to trim.</param>
		/// <returns>Trimmed string.</returns>
		static char * TrimString(char *String);

		/// <summary>
		/// Gets the text from the clipboard.
		/// </summary>
		/// <param name="text">Pointer to string receiving the text.</param>
		/// <returns>True if text was available in the clipboard.</returns>
		static bool GetClipboardText(std::string *text);

		/// <summary>
		/// Sets the text in the clipboard.
		/// </summary>
		/// <param name="text">String to put into the clipboard.</param>
		/// <returns>True if text was added to the clipboard.</returns>
		static bool SetClipboardText(std::string text);
	};
}
#endif