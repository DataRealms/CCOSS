#include "Reader.h"
#include "RTETools.h"
#include "PresetMan.h"
#include "SettingsMan.h"

namespace RTE {

	const std::string Reader::c_ClassName = "Reader";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Reader::Clear() {
		m_Stream = 0;
		m_FilePath.clear();
		m_CurrentLine = 1;
		m_StreamStack.clear();
		m_PreviousIndent = 0;
		m_IndentDifference = 0;
		m_ObjectEndings = 0;
		m_EndOfStreams = false;
		m_ReportProgress = 0;
		m_ReportTabs = "\t";
		m_FileName.clear();
		m_DataModuleName.clear();
		m_DataModuleID = -1;
		m_OverwriteExisting = false;
		m_SkipIncludes = false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Reader::Create(const char *fileName, bool overwrites, ProgressCallback fpProgressCallback, bool failOK) {
		m_FilePath = fileName;

		if (m_FilePath.empty()) {
			return -1;
		}
		// Extract just the filename
		int lastSlashPos = m_FilePath.find_last_of('/');
		if (lastSlashPos == std::string::npos) { lastSlashPos = m_FilePath.find_last_of('\\'); }
		m_FileName = m_FilePath.substr(lastSlashPos + 1);

		// Find the first slash so we can get the module name
		int firstSlashPos = m_FilePath.find_first_of('/');
		if (firstSlashPos == std::string::npos) { firstSlashPos = m_FilePath.find_first_of('\\'); }

		m_DataModuleName = m_FilePath.substr(0, firstSlashPos);
		m_DataModuleID = g_PresetMan.GetModuleID(m_DataModuleName);

		m_Stream = new std::ifstream(fileName);
		if (!failOK) { RTEAssert(m_Stream->good(), "Failed to open data file \'" + std::string(fileName) + "\'!"); }

		m_OverwriteExisting = overwrites;

		// Report that we're starting a new file
		m_ReportProgress = fpProgressCallback;
		if (m_ReportProgress && m_Stream->good()) {
			char report[512];
			std::snprintf(report, sizeof(report), "\t%s on line %i", m_FileName.c_str(), m_CurrentLine);
			m_ReportProgress(std::string(report), true);
		}
		return m_Stream->good() ? 0 : -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Reader::Destroy() {
		delete m_Stream;
		// Delete all the streams in the stream stack
		for (const StreamInfo &streamInfo : m_StreamStack) {
			delete streamInfo.Stream;
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Reader::GetReadModuleID() const {
		// If we have an invalid ID, try to get a valid one based on the name we do have
		return (m_DataModuleID < 0) ? g_PresetMan.GetModuleID(m_DataModuleName) : m_DataModuleID;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Reader::ReadLine(char *locString, int size) {
		DiscardEmptySpace();

		char temp;
		char peek = m_Stream->peek();
		int i = 0;

		for (i = 0; i < size - 1 && peek != '\n' && peek != '\r' && peek != '\t'; ++i) {
			temp = m_Stream->get();
			// Check for line comment "//"
			if (peek == '/' && m_Stream->peek() == '/') {
				m_Stream->putback(temp);
				break;
			}
			if (m_Stream->eof()) {
				EndIncludeFile();
				break;
			}
			if (!m_Stream->good()) { ReportError("Stream failed for some reason"); }

			locString[i] = temp;
			peek = m_Stream->peek();
		}
		locString[i] = '\0';
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Reader::ReadLine() {
		DiscardEmptySpace();

		std::string retString;
		char temp;
		char peek = m_Stream->peek();

		while (peek != '\n' && peek != '\r' && peek != '\t') {
			temp = m_Stream->get();

			// Check for line comment "//"
			if (peek == '/' && m_Stream->peek() == '/') {
				m_Stream->unget();
				break;
			}

			if (m_Stream->eof()) { break; }
			if (!m_Stream->good()) { ReportError("Stream failed for some reason"); }

			retString.append(1, temp);
			peek = m_Stream->peek();
		}
		return retString;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Reader::ReadTo(char terminator, bool discardTerminator) {
		std::string retString;
		char temp;
		char peek = m_Stream->peek();

		while (peek != terminator) {
			temp = m_Stream->get();

			if (m_Stream->eof()) { break; }
			if (!m_Stream->good()) { ReportError("Stream failed for some reason"); }

			retString.append(1, temp);
			peek = m_Stream->peek();
		}
		// Discard the terminator if instructed to
		if (discardTerminator && peek == terminator) { m_Stream->get(); }
		return retString;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Reader::NextProperty() {
		if (!DiscardEmptySpace() || m_EndOfStreams) {
			return false;
		}
		// If there are fewer tabs on the last line eaten this time,
		// that means there are no more properties to read on this object
		if (m_ObjectEndings < -m_IndentDifference) {
			m_ObjectEndings++;
			return false;
		}
		m_ObjectEndings = 0;
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string Reader::ReadPropName() {
		DiscardEmptySpace();

		std::string retString;
		char temp;
		char peek;

		while (true) {
			peek = m_Stream->peek();
			if (peek == '=') {
				m_Stream->ignore(1);
				break;
			}
			if (peek == '\n' || peek == '\r' || peek == '\t') {
				ReportError("Property name wasn't followed by a value");
			}
			temp = m_Stream->get();
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

	std::string Reader::ReadPropValue() {
		std::string fullLine = ReadLine();
		int begin = fullLine.find_first_of('=');
		std::string subStr = (begin == std::string::npos) ? fullLine : fullLine.substr(begin + 1);
		return TrimString(subStr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Reader::DiscardEmptySpace() {
		char peek;
		unsigned short indent = 0;
		bool discardedLine = false;
		char report[512];

		while (true) {
			peek = m_Stream->peek();

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
				if (peek == '\n') {
					m_CurrentLine++;
					// Only report every few lines
					if (m_ReportProgress && (m_CurrentLine % g_SettingsMan.LoadingScreenReportPrecision() == 0)) {
						std::snprintf(report, sizeof(report), "%s%s reading line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
						m_ReportProgress(std::string(report), false);
					}
				}
				indent = 0;
				discardedLine = true;
				m_Stream->ignore(1);

			// Comment line?
			} else if (m_Stream->peek() == '/') {
				char temp = m_Stream->get();
				char temp2;

				// Confirm that it's a comment line, if so discard it and continue
				if (m_Stream->peek() == '/') {
					while (m_Stream->peek() != '\n' && m_Stream->peek() != '\r' && !m_Stream->eof()) { m_Stream->ignore(1); }
				// Block comment
				} else if (m_Stream->peek() == '*') {
					// Find the matching "*/"
					while (!((temp2 = m_Stream->get()) == '*' && m_Stream->peek() == '/') && !m_Stream->eof()) {
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

	std::string Reader::TrimString(std::string &stringToTrim) {
		if (stringToTrim.empty()) {
			return "";
		}
		int start = stringToTrim.find_first_not_of(' ');
		int end = stringToTrim.find_last_not_of(' ');

		if (start > end) {
			return "";
		} else if (start == 0 && end == stringToTrim.size() - 1) {
			return stringToTrim;
		}
		return stringToTrim.substr(start, (end - start) + 1);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Reader::ReportError(std::string errorDesc) {
		char error[1024];
		std::snprintf(error, sizeof(error), "%s Error happened in %s at line %i!", errorDesc.c_str(), m_FilePath.c_str(), m_CurrentLine);
		RTEAbort(error);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Reader::StartIncludeFile() {
		// Report that we're including a file
		if (m_ReportProgress) {
			char report[512];
			std::snprintf(report, sizeof(report), "%s%s on line %i includes:", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
			m_ReportProgress(std::string(report), false);
		}
		// Push the current stream onto the StreamStack for future retrieval when the new include file has run out of data.
		m_StreamStack.push_back(StreamInfo(m_Stream, m_FilePath, m_CurrentLine, m_PreviousIndent));

		// Get the file path from the stream
		m_FilePath = ReadPropValue();
		m_Stream = new std::ifstream(m_FilePath.c_str());
		if (m_Stream->fail()) {
			// Backpedal and set up to read the next property in the old stream
			delete m_Stream;
			m_Stream = m_StreamStack.back().Stream;
			m_FilePath = m_StreamStack.back().FilePath;
			m_CurrentLine = m_StreamStack.back().CurrentLine;
			m_PreviousIndent = m_StreamStack.back().PreviousIndent;
			m_StreamStack.pop_back();

			ReportError("Failed to open included data file");

			DiscardEmptySpace();
			return false;
		}

		// Line counting starts with 1, not 0
		m_CurrentLine = 1;
		// This is set to 0, because locally in the included file, all properties start at that count
		m_PreviousIndent = 0;

		// Extract just the filename
		int firstSlashPos = m_FilePath.find_first_of('/');
		if (firstSlashPos == std::string::npos) { firstSlashPos = m_FilePath.find_first_of('\\'); }
		m_FileName = m_FilePath.substr(firstSlashPos + 1);

		// Report that we're starting a new file
		if (m_ReportProgress) {
			m_ReportTabs = "\t";
			for (unsigned int i = 0; i < m_StreamStack.size(); ++i) {
				m_ReportTabs.append("\t");
			}
			char report[512];
			std::snprintf(report, sizeof(report), "%s%s on line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
			m_ReportProgress(std::string(report), true);
		}
		// Discard any fluff in the beginning of the new file
		DiscardEmptySpace();
		// indicate success
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool Reader::EndIncludeFile() {
		// Do final report on the file we're closing
		if (m_ReportProgress) {
			char report[512];
			std::snprintf(report, sizeof(report), "%s%s - done! %c", m_ReportTabs.c_str(), m_FileName.c_str(), -42);
			m_ReportProgress(std::string(report), false);
		}
		if (m_StreamStack.empty()) {
			m_EndOfStreams = true;
			return false;
		}
		// Replace the current included stream with the parent one
		delete m_Stream;
		m_Stream = m_StreamStack.back().Stream;
		m_FilePath = m_StreamStack.back().FilePath;
		m_CurrentLine = m_StreamStack.back().CurrentLine;
		// Observe it's being added, not just replaced. This is to keep proper track when exiting out of a file
		m_PreviousIndent += m_StreamStack.back().PreviousIndent;
		m_StreamStack.pop_back();

		// Extract just the filename
		int firstSlashPos = m_FilePath.find_first_of('/');
		if (firstSlashPos == std::string::npos) { firstSlashPos = m_FilePath.find_first_of('\\'); }

		m_FileName = m_FilePath.substr(firstSlashPos + 1);

		// Report that we're going back a file
		if (m_ReportProgress) {
			m_ReportTabs = "\t";
			for (unsigned int i = 0; i < m_StreamStack.size(); ++i) {
				m_ReportTabs.append("\t");
			}
			char report[512];
			std::snprintf(report, sizeof(report), "%s%s on line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
			m_ReportProgress(std::string(report), true);
		}
		// Set up the resumed file for reading again
		DiscardEmptySpace();
		return true;
	}
}