#include "GUI.h"
#include "GUIReader.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader::StreamInfo::StreamInfo(std::ifstream *stream, const std::string &filePath, int currentLine, int prevIndent) : Stream(stream), FilePath(filePath), CurrentLine(currentLine), PreviousIndent(prevIndent) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIReader::Clear() {
		m_Stream = nullptr;
		m_FilePath.clear();
		m_CurrentLine = 1;
		m_PreviousIndent = 0;
		m_IndentDifference = 0;
		m_ObjectEndings = 0;
		m_EndOfStreams = false;
		m_ReportTabs = "\t";
		m_FileName.clear();
		m_SkipIncludes = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader::GUIReader() {
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GUIReader::Create(const std::string &fileName) {
		m_FilePath = std::filesystem::path(fileName).generic_string();

		if (m_FilePath.empty()) {
			return -1;
		}
		// Extract the file name and module name from the path
		m_FileName = m_FilePath.substr(m_FilePath.find_last_of("/\\") + 1);

		m_Stream = std::make_unique<std::ifstream>(fileName);
		return m_Stream->good() ? 0 : -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::istream * GUIReader::GetStream() const {
		return m_Stream.get();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::GetCurrentFilePath() const {
		return m_FilePath;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::GetCurrentFileLine() const {
		return std::to_string(m_CurrentLine);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::GetSkipIncludes() const {
		return m_SkipIncludes;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIReader::SetSkipIncludes(bool skip) {
		m_SkipIncludes = skip;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::ReadLine() {
		DiscardEmptySpace();

		std::string retString;
		char temp;
		char peek = static_cast<char>(m_Stream->peek());

		while (peek != '\n' && peek != '\r' && peek != '\t') {
			temp = static_cast<char>(m_Stream->get());

			// Check for line comment "//"
			if (peek == '/' && m_Stream->peek() == '/') {
				m_Stream->unget();
				break;
			}

			if (m_Stream->eof()) { break; }
			if (!m_Stream->good()) { ReportError("Stream failed for some reason"); }

			retString.append(1, temp);
			peek = static_cast<char>(m_Stream->peek());
		}
		return TrimString(retString);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::ReadPropName() {
		DiscardEmptySpace();

		std::string retString;
		char temp;
		char peek;

		while (true) {
			peek = static_cast<char>(m_Stream->peek());
			if (peek == '=') {
				m_Stream->ignore(1);
				break;
			}
			if (peek == '\n' || peek == '\r' || peek == '\t') {
				ReportError("Property name wasn't followed by a value");
			}
			temp = static_cast<char>(m_Stream->get());
			if (m_Stream->eof()) {
				EndIncludeFile();
				break;
			}
			if (!m_Stream->good()) { ReportError("Stream failed for some reason"); }
			retString.append(1, temp);
		}
		// Trim the string of whitespace
		retString = TrimString(retString);

		// If the property name turns out to be the special IncludeFile,and we're not skipping include files then open that file and read the first property from it instead.
		if (retString == "IncludeFile") {
			if (m_SkipIncludes) {
				// Discard IncludeFile value
				std::string val = ReadPropValue();
				DiscardEmptySpace();
				retString = ReadPropName();
			} else {
				StartIncludeFile();
				// Return the first property name in the new file, this is to make the file inclusion seamless.
				// Alternatively, if StartIncludeFile failed, this will just grab the next prop name and ignore the failed IncludeFile property.
				retString = ReadPropName();
			}
		}
		return retString;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::ReadPropValue() {
		std::string fullLine = ReadLine();
		size_t valuePos = fullLine.find_first_of('=');
		std::string propValue = (valuePos == std::string::npos) ? fullLine : fullLine.substr(valuePos + 1);
		return TrimString(propValue);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::NextProperty() {
		if (!DiscardEmptySpace() || m_EndOfStreams) {
			return false;
		}
		// If there are fewer tabs on the last line eaten this time, that means there are no more properties to read on this object.
		if (m_ObjectEndings < -m_IndentDifference) {
			m_ObjectEndings++;
			return false;
		}
		m_ObjectEndings = 0;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string GUIReader::TrimString(const std::string &stringToTrim) const {
		if (stringToTrim.empty()) {
			return "";
		}
		size_t start = stringToTrim.find_first_not_of(' ');
		size_t end = stringToTrim.find_last_not_of(' ');

		return stringToTrim.substr(start, (end - start + 1));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::DiscardEmptySpace() {
		char peek;
		int indent = 0;
		bool discardedLine = false;

		while (true) {
			peek = static_cast<char>(m_Stream->peek());

			// If we have hit the end and don't have any files to resume, then quit and indicate that
			if (m_Stream->eof()) {
				return EndIncludeFile();
			}
			// Not end-of-file but still got junk back... something went to shit
			if (peek == -1) { ReportError("Something went wrong reading the line; make sure it is providing the expected type"); }

			// Discard spaces
			if (peek == ' ') {
				m_Stream->ignore(1);
			// Discard tabs, and count them
			} else if (peek == '\t') {
				indent++;
				m_Stream->ignore(1);
			// Discard newlines and reset the tab count for the new line, also count the lines
			} else if (peek == '\n' || peek == '\r') {
				// So we don't count lines twice when there are both newline and carriage return at the end of lines
				if (peek == '\n') { m_CurrentLine++; }
				indent = 0;
				discardedLine = true;
				m_Stream->ignore(1);

			// Comment line?
			} else if (m_Stream->peek() == '/') {
				char temp = static_cast<char>(m_Stream->get());
				char temp2;

				// Confirm that it's a comment line, if so discard it and continue
				if (m_Stream->peek() == '/') {
					while (m_Stream->peek() != '\n' && m_Stream->peek() != '\r' && !m_Stream->eof()) { m_Stream->ignore(1); }
				// Block comment
				} else if (m_Stream->peek() == '*') {
					// Find the matching "*/"
					while (!((temp2 = static_cast<char>(m_Stream->get())) == '*' && m_Stream->peek() == '/') && !m_Stream->eof()) {
						// Count the lines within the comment though
						if (temp2 == '\n') { ++m_CurrentLine; }
					}
					// Discard that final '/'
					if (!m_Stream->eof()) { m_Stream->ignore(1); }

				// Not a comment, so it's data, so quit.
				} else {
					m_Stream->putback(temp);
					break;
				}
			} else {
				break;
			}
		}

		// This precaution enables us to use DiscardEmptySpace repeatedly without messing up the indentation tracking logic
		if (discardedLine) {
			// Get indentation difference from the last line of the last call to DiscardEmptySpace(), and the last line of this call to DiscardEmptySpace().
			m_IndentDifference = indent - m_PreviousIndent;
			// Save the last tab count
			m_PreviousIndent = indent;
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::ReaderOK() const {
		return m_Stream.get() && !m_Stream->fail() && m_Stream->is_open();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUIReader::ReportError(const std::string &errorDesc) const {
		GUIAbort(errorDesc + "\nError happened in " + m_FilePath + " at line " + std::to_string(m_CurrentLine) + "!");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::StartIncludeFile() {
		// Get the file path from the current stream before pushing it into the StreamStack, otherwise we can't open a new stream after releasing it because we can't read.
		std::string includeFilePath = std::filesystem::path(ReadPropValue()).generic_string();

		// Push the current stream onto the StreamStack for future retrieval when the new include file has run out of data.
		m_StreamStack.emplace(StreamInfo(m_Stream.release(), m_FilePath, m_CurrentLine, m_PreviousIndent));

		m_FilePath = includeFilePath;
		m_Stream = std::make_unique<std::ifstream>(m_FilePath);

		if (m_Stream->fail() || !std::filesystem::exists(includeFilePath)) {
			// Backpedal and set up to read the next property in the old stream
			m_Stream.reset(m_StreamStack.top().Stream); // Destructs the current m_Stream and takes back ownership and management of the raw StreamInfo std::ifstream pointer.
			m_FilePath = m_StreamStack.top().FilePath;
			m_CurrentLine = m_StreamStack.top().CurrentLine;
			m_PreviousIndent = m_StreamStack.top().PreviousIndent;
			m_StreamStack.pop();
			DiscardEmptySpace();
			return false;
		}

		// Line counting starts with 1, not 0
		m_CurrentLine = 1;
		// This is set to 0, because locally in the included file, all properties start at that count
		m_PreviousIndent = 0;

		m_FileName = m_FilePath.substr(m_FilePath.find_first_of("/\\") + 1);

		DiscardEmptySpace();
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool GUIReader::EndIncludeFile() {
		if (m_StreamStack.empty()) {
			m_EndOfStreams = true;
			return false;
		}

		// Replace the current included stream with the parent one
		m_Stream.reset(m_StreamStack.top().Stream);
		m_FilePath = m_StreamStack.top().FilePath;
		m_CurrentLine = m_StreamStack.top().CurrentLine;

		// Observe it's being added, not just replaced. This is to keep proper track when exiting out of a file.
		m_PreviousIndent += m_StreamStack.top().PreviousIndent;

		m_StreamStack.pop();

		m_FileName = m_FilePath.substr(m_FilePath.find_first_of("/\\") + 1);

		DiscardEmptySpace();
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(bool &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(char &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(unsigned char &var) {
		DiscardEmptySpace();
		int temp;
		*m_Stream >> temp;
		var = static_cast<unsigned char>(temp);
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(short &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(unsigned short &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(int &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(unsigned int &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(long &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(unsigned long &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(float &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(double &var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(char * var) {
		DiscardEmptySpace();
		*m_Stream >> var;
		return *this;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIReader & GUIReader::operator>>(std::string &var) {
		var.assign(ReadLine());
		return *this;
	}
}
