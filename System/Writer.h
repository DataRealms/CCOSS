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
		Writer(std::string filename, bool append = false) { Clear(); Create(filename.c_str(), append); }

		/// <summary>
		/// Makes the Writer object ready for use.
		/// </summary>
		/// <param name="filename">
		/// The filename of the file to open and write to.
		/// If the file path doesn't exist, the first directory name is used in an attempt to open a package and then read a file from within that.
		/// </param>
		/// <param name="append">Whether to append to the file if it exists, or to overwrite it.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const char *filename, bool append = false);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Writer object before deletion from system memory.
		/// </summary>
		virtual ~Writer() { Destroy(true); }

		/// <summary>
		/// Resets the entire Writer, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Writer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false) { delete m_pStream; Clear(); }
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
		virtual void ObjectStart(const std::string &className) {
			*m_pStream << className;
			++m_Indent;
		}

		/// <summary>
		/// Used to specify the end of an object that has just been written.
		/// </summary>
		virtual void ObjectEnd() {
			--m_Indent;
			// Make an extra line between big object definitions
			if (m_Indent == 0) { *m_pStream << "\n\n"; }
		}

		/// <summary>
		/// Creates a new line that is properly indented.
		/// </summary>
		virtual void NewLine() {
			*m_pStream << "\n";
			for (int i = 0; i < m_Indent; ++i) { *m_pStream << "\t"; }
		}

		/// <summary>
		/// Used to specify the name of a new property to be written.
		/// </summary>
		/// <param name="propName">The name of the property to be written.</param>
		virtual void NewProperty(std::string propName) {
			NewLine();
			*m_pStream << propName;
			*m_pStream << " = ";
		}

		/// <summary>
		/// Marks that there is a null reference to an object here.
		/// </summary>
		virtual void NoObject() {
			//NewLine();
			*m_pStream << "None";
		}
#pragma endregion

#pragma region Writer Status
		/// <summary>
		/// Shows whether the writer is ok to start accepting data streamed to it.
		/// </summary>
		/// <returns></returns>
		virtual bool WriterOK() { return m_pStream && !m_pStream->fail() && m_pStream->is_open(); }
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// Elemental types stream insertions. Stream insertion operator overloads for all the elemental types.
		/// </summary>
		/// <param name="var">A reference to the variable that will be written to the ostream.</param>
		/// <returns>A Writer reference for further use in an expression.</returns>
		virtual Writer & operator<<(const bool &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const char &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const unsigned char &var) { int temp = var; *m_pStream << temp; return *this; }
		virtual Writer & operator<<(const short &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const unsigned short &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const int &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const unsigned int &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const long &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const unsigned long &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const float &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const double &var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const char *var) { *m_pStream << var; return *this; }
		virtual Writer & operator<<(const std::string &var) { *m_pStream << var; return *this; }
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Writer.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this Writer.</returns>
		virtual const std::string & GetClassName() const { return m_ClassName; }
#pragma endregion

	protected:

		static const std::string m_ClassName; //!< A string with the friendly-formatted type name of this object.
		std::ofstream *m_pStream; //!< Stream used for writing to files.
		std::string m_FolderPath; //!< Only the path to the folder that we are writing a file in, excluding the filename
		std::string m_FilePath; //!< Currently used stream's filepath
		std::string m_FileName; //!< Only the name of the currently read file, excluding the path
		int m_Indent; //!< Indent counter.

	private:

		/// <summary>
		/// Clears all the member variables of this Writer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		Writer(const Writer &reference);
		Writer & operator=(const Writer &rhs);
	};
}
#endif