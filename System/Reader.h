#ifndef _RTEREADER_
#define _RTEREADER_

namespace RTE {

	using ProgressCallback = std::function<void(std::string, bool)>; //!< Convenient name definition for the progress report callback function.

	/// <summary>
	/// Reads RTE objects from std::istreams.
	/// </summary>
	class Reader {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Reader object in system memory. Create() should be called before using the object.
		/// </summary>
		Reader() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Reader object in system memory and make it ready for reading from the passed in file path.
		/// </summary>
		/// <param name="fileName">Path to the file to open for reading. If the file doesn't exist the stream will fail to open.</param>
		/// <param name="overwrites">Whether object definitions read here overwrite existing ones with the same names.</param>
		/// <param name="progressCallback">A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.</param>
		/// <param name="failOK">Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.</param>
		Reader(const std::string &fileName, bool overwrites = false, const ProgressCallback &progressCallback = nullptr, bool failOK = false) { Clear(); Create(fileName, overwrites, progressCallback, failOK); }

		/// <summary>
		/// Makes the Reader object ready for use.
		/// </summary>
		/// <param name="fileName">Path to the file to open for reading. If the file doesn't exist the stream will fail to open.</param>
		/// <param name="overwrites"> Whether object definitions read here overwrite existing ones with the same names.</param>
		/// <param name="progressCallback">A function pointer to a function that will be called and sent a string with information about the progress of this Reader's reading.</param>
		/// <param name="failOK">Whether it's ok for the file to not be there, ie we're only trying to open, and if it's not there, then fail silently.</param>
		/// <returns>An error return value signaling success or any particular failure.  Anything below 0 is an error signal.</returns>
		int Create(const std::string &fileName, bool overwrites = false, const ProgressCallback &progressCallback = nullptr, bool failOK = false);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the name of the DataModule this Reader is reading from.
		/// </summary>
		/// <returns>The name of the DataModule this reader is reading from.</returns>
		const std::string & GetReadModuleName() const { return m_DataModuleName; }

		/// <summary>
		/// Gets the ID of the DataModule this Reader is reading from. If the ID is invalid, attempts to get a valid ID using the DataModule name.
		/// </summary>
		/// <returns>The DataModule ID that this Reader is reading from.</returns>
		int GetReadModuleID() const;

		/// <summary>
		/// Gets a pointer to the istream of this reader.
		/// </summary>
		/// <returns>A pointer to the istream object for this reader.</returns>
		std::istream * GetStream() const { return m_Stream.get(); }

		/// <summary>
		/// Gets the path of the current file this reader is reading from.
		/// </summary>
		/// <returns>A string with the path, relative from the working directory.</returns>
		std::string GetCurrentFilePath() const { return m_FilePath; }

		/// <summary>
		/// Gets the line of the current file line this reader is reading from.
		/// </summary>
		/// <returns>A string with the line number that will be read from next.</returns>
		std::string GetCurrentFileLine() const { return std::to_string(m_CurrentLine); }

		/// <summary>
		/// Shows whether objects read from this will be overwriting any existing ones with the same names.
		/// </summary>
		/// <returns>Whether this overwrites or not.</returns>
		bool GetPresetOverwriting() const { return m_OverwriteExisting; }

		/// <summary>
		/// Sets whether objects read from this will be overwriting any existing ones with the same names.
		/// </summary>
		/// <param name="overwrites">Whether this should overwrite existing definitions or not.</param>
		void SetPresetOverwriting(bool overwrites = true) { m_OverwriteExisting = overwrites; }

		/// <summary>
		/// Returns true if reader was told to skip InlcudeFile statements
		/// </summary>
		/// <returns>Returns whether reader was told to skip included files.</returns>
		bool GetSkipIncludes() const { return m_SkipIncludes; };

		/// <summary>
		/// Set whether this reader should skip included files.
		/// </summary>
		/// <param name="skip>To make reader skip included files pass true, pass false otherwise.</param>
		void SetSkipIncludes(bool skip) { m_SkipIncludes = skip; };
#pragma endregion

#pragma region Reading Operations
		/// <summary>
		/// Reads a file and constructs a string from all its contents.
		/// </summary>
		/// <returns>A string containing the whole file contents.</returns>
		std::string WholeFileAsString() const;

		/// <summary>
		/// Reads the rest of the line from the context object Reader's stream current location.
		/// </summary>
		/// <returns>The std::string that will hold the line's contents.</returns>
		std::string ReadLine();

		/// <summary>
		/// Reads the next property name from the context object Reader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything between the last newline before text to the next "=" after that.
		/// </summary>
		/// <returns>The whitespace-trimmed std::string that will hold the next property's name.</returns>
		std::string ReadPropName();

		/// <summary>
		/// Reads the next property value from the context object Reader's stream after eating all whitespace including newlines up till the first newline char.
		/// Basically gets anything after the last "=" and up to the next newline after that.
		/// </summary>
		/// <returns>The whitespace-trimmed std::string that will hold the next property value.</returns>
		std::string ReadPropValue();

		/// <summary>
		/// Lines up the reader with the next property of the current object.
		/// </summary>
		/// <returns>Whether there are any more properties to be read by the current object.</returns>
		bool NextProperty();

		/// <summary>
		/// Takes out whitespace from the beginning and the end of a string.
		/// </summary>
		/// <param name="stringToTrim">String to remove whitespace from.</param>
		/// <returns>The string that was passed in, sans whitespace in the front and end.</returns>
		std::string TrimString(const std::string &stringToTrim) const;

		/// <summary>
		/// Discards all whitespace, newlines and comment lines (which start with '//') so that the next thing to be read will be actual data.
		/// </summary>
		/// <returns>Whether there is more data to read from the file streams after this eat.</returns>
		bool DiscardEmptySpace();
#pragma endregion

#pragma region Reader Status
		/// <summary>
		/// Shows whether this is still OK to read from. If file isn't present, etc, this will return false.
		/// </summary>
		/// <returns>Whether this Reader's stream is OK or not.</returns>
		bool ReaderOK() const { return m_Stream.get() && !m_Stream->fail() && m_Stream->is_open(); }

		/// <summary>
		/// Makes an error message box pop up for the user that tells them something went wrong with the reading, and where.
		/// </summary>
		/// <param name="errorDesc">The message describing what's wrong.</param>
		void ReportError(const std::string &errorDesc) const;
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// Stream extraction operator overloads for all the elemental types.
		/// </summary>
		/// <param name="var">A reference to the variable that will be filled by the extracted data.</param>
		/// <returns>A Reader reference for further use in an expression.</returns>
		Reader & operator>>(bool &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(char &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(unsigned char &var) { DiscardEmptySpace(); int temp; *m_Stream >> temp; var = temp; return *this; }
		Reader & operator>>(short &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(unsigned short &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(int &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(unsigned int &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(long &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(unsigned long &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(float &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(double &var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(char * var) { DiscardEmptySpace(); *m_Stream >> var; return *this; }
		Reader & operator>>(std::string &var) { var.assign(ReadLine()); return *this; }
#pragma endregion

	protected:

		/// <summary>
		/// A struct containing information from the currently used stream.
		/// </summary>
		struct StreamInfo {
			/// <summary>
			/// Constructor method used to instantiate a StreamInfo object in system memory.
			/// </summary>
			StreamInfo(std::ifstream *stream, const std::string &filePath, int currentLine, int prevIndent) : Stream(stream), FilePath(filePath), CurrentLine(currentLine), PreviousIndent(prevIndent) {}

			// NOTE: These members are owned by the reader that owns this struct, so are not deleted when this is destroyed.
			std::ifstream *Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
			std::string FilePath; //!< Currently used stream's filepath.
			int CurrentLine; //!< The line number the stream is on.
			int PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		};

		std::unique_ptr<std::ifstream> m_Stream; //!< Currently used stream, is not on the StreamStack until a new stream is opened.
		std::stack<StreamInfo> m_StreamStack; //!< Stack of open streams in this Reader, each one representing a file opened to read from within another.
		bool m_EndOfStreams; //!< All streams have been depleted.

		ProgressCallback m_ReportProgress; //!< Function pointer to report our reading progress to, by calling it and passing a descriptive string to it.

		std::string m_FilePath; //!< Currently used stream's filepath.
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path.

		std::string m_DataModuleName; //!< The current name of the data module being read from, including the .rte extension.
		int m_DataModuleID; //!< The current ID of the data module being read from.

		int m_PreviousIndent; //!< Count of tabs encountered on the last line DiscardEmptySpace() discarded.
		int m_IndentDifference; //!< Difference in indentation from the last line to the current line.
		std::string m_ReportTabs; //!< String containing the proper amount of tabs for the report.

		int m_CurrentLine; //!< The line number the stream is on.
		bool m_OverwriteExisting; //!< Whether object instances read from this should overwrite any already existing ones with the same names.
		bool m_SkipIncludes; //!< Indicates whether reader should skip included files.
		bool m_CanFail; //!< Whether it's ok for the Reader to fail reading a file and fail silently instead of aborting.

		/// <summary>
		/// When NextProperty() has returned false, indicating that there were no more properties to read on that object,
		/// this is incremented until it matches -m_IndentDifference, and then NextProperty will start returning true again.
		/// </summary>
		int m_ObjectEndings;

	private:

#pragma region Reading Operations
		/// <summary>
		/// When ReadPropName encounters the property name "IncludeFile", it will automatically call this function to get started reading on that file.
		/// This will create a new stream to the include file.
		/// </summary>
		/// <returns>Whether the include file was found and opened ok or not.</returns>
		bool StartIncludeFile();

		/// <summary>
		/// This should be called when end-of-file is detected in an included file stream.
		/// It will destroy the current stream pop the top stream off the stream stack to resume reading from it instead.
		/// </summary>
		/// <returns>Whether there were any stream on the stack to resume.</returns>
		bool EndIncludeFile();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this Reader, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Reader(const Reader &reference) = delete;
		Reader & operator=(const Reader &rhs) = delete;
	};
}
#endif
