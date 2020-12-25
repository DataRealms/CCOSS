#ifndef _RTEWRITER_
#define _RTEWRITER_

namespace RTE {

	static constexpr char g_WritePackageExtension[8] = ".rte";

	/// <summary>
	/// Writes RTE objects to std::ostreams.
	/// </summary>
	class Writer {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Writer object in system memory. Create() should be called before using the object.
		/// </summary>
		Writer() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a Writer object in system memory. Create() should be called before using the object.
		/// </summary>
		/// <param name="filename">Path to the file to open for writing.</param>
		/// <param name="append">Whether to append to the file if it exists, or to overwrite it.</param>
		Writer(const char *filename, bool append = false) { Clear(); Create(filename, append); }

		/// <summary>
		/// Makes the Writer object ready for use.
		/// </summary>
		/// <param name="filename">
		/// The filename of the file to open and write to.
		/// If the file path doesn't exist, the first directory name is used in an attempt to open a package and then read a file from within that.
		/// </param>
		/// <param name="append">Whether to append to the file if it exists, or to overwrite it.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const char *filename, bool append = false);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Writer object before deletion from system memory.
		/// </summary>
		~Writer() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Writer object.
		/// </summary>
		void Destroy() { delete m_Stream; Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the path to the file being written.
		/// </summary>
		/// <returns>The full path to the file being written.</returns>
		std::string GetFilePath() const { return m_FilePath; }

		/// <summary>
		/// Gets the name (without path) of the file being written.
		/// </summary>
		/// <returns>The name of file being written.</returns>
		std::string GetFileName() const { return m_FileName; }

		/// <summary>
		/// Gets the folder path (without filename) to where the file is being written.
		/// </summary>
		/// <returns>The name of folder being written in.</returns>
		std::string GetFolderPath() const { return m_FolderPath; }
#pragma endregion

#pragma region Writing Operations
		/// <summary>
		/// Used to specify the start of an object to be written.
		/// </summary>
		/// <param name="className">The class name of the object about to be written.</param>
		void ObjectStart(const std::string &className) { *m_Stream << className; ++m_Indent; }

		/// <summary>
		/// Used to specify the end of an object that has just been written.
		/// </summary>
		void ObjectEnd() { --m_Indent; if (m_Indent == 0) { NewLine(false, 2); } }

		/// <summary>
		/// Creates a new line that can be properly indented.
		/// </summary>
		/// <param name="toIndent">Whether to indent the new line or not.</param>
		/// <param name="lineCount">How many new lines to create.</param>
		void NewLine(bool toIndent = true, unsigned short lineCount = 1);

		/// <summary>
		/// Creates a new line and writes the specified string to it.
		/// </summary>
		/// <param name="textString">The text string to write to the new line.</param>
		/// <param name="toIndent">Whether to indent the new line or not.</param>
		void NewLineString(std::string textString, bool toIndent = true) { NewLine(toIndent); *m_Stream << textString; }

		/// <summary>
		/// Creates a new line and fills it with slashes to create a divider line for INI.
		/// </summary>
		/// <param name="toIndent">Whether to indent the new line or not.</param>
		/// <param name="dividerLength">The length of the divider (number of slashes).</param>
		void NewDivider(bool toIndent = true, unsigned short dividerLength = 72) {
			NewLine(toIndent);
			for (unsigned short slash = 0; slash < dividerLength; slash++) {
				*m_Stream << "/";
			}
		}

		/// <summary>
		/// Used to specify the name of a new property to be written.
		/// </summary>
		/// <param name="propName">The name of the property to be written.</param>
		void NewProperty(std::string propName) { NewLine(); *m_Stream << propName; *m_Stream << " = "; }

		/// <summary>
		/// Marks that there is a null reference to an object here.
		/// </summary>
		void NoObject() { *m_Stream << "None"; }
#pragma endregion

#pragma region Writer Status
		/// <summary>
		/// Shows whether the writer is ok to start accepting data streamed to it.
		/// </summary>
		/// <returns></returns>
		bool WriterOK() const { return m_Stream && !m_Stream->fail() && m_Stream->is_open(); }
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// Elemental types stream insertions. Stream insertion operator overloads for all the elemental types.
		/// </summary>
		/// <param name="var">A reference to the variable that will be written to the ostream.</param>
		/// <returns>A Writer reference for further use in an expression.</returns>
		Writer & operator<<(const bool &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const char &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const unsigned char &var) { int temp = var; *m_Stream << temp; return *this; }
		Writer & operator<<(const short &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const unsigned short &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const int &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const unsigned int &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const long &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const unsigned long &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const float &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const double &var) { *m_Stream << var; return *this; }
		Writer & operator<<(const char *var) { *m_Stream << var; return *this; }
		Writer & operator<<(const std::string &var) { *m_Stream << var; return *this; }
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Writer.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this Writer.</returns>
		const std::string & GetClassName() const { return c_ClassName; }
#pragma endregion

	protected:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		std::ofstream *m_Stream; //!< Stream used for writing to files.
		std::string m_FolderPath; //!< Only the path to the folder that we are writing a file in, excluding the filename
		std::string m_FilePath; //!< Currently used stream's filepath
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path
		short m_Indent; //!< Indent counter.

	private:

		/// <summary>
		/// Clears all the member variables of this Writer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Writer(const Writer &reference) = delete;
		Writer & operator=(const Writer &rhs) = delete;
	};
}
#endif