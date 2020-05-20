#include "Writer.h"

namespace RTE {

	const std::string Writer::c_ClassName = "Writer";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Writer::Clear() {
		m_Stream = 0;
		m_FileName.clear();
		m_FolderPath.clear();
		m_Indent = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int Writer::Create(const char *filename, bool append) {
		m_Stream = new std::ofstream(filename, append ? (std::ios_base::out | std::ios_base::app | std::ios_base::ate) : (std::ios_base::out | std::ios_base::trunc));

		if (!m_Stream->good()) {
			return -1;
		}
		// Save the file path
		m_FilePath = filename;

		// Extract just the filename and the path by first finding the last slash int he total path
		size_t slashPos = m_FilePath.find_last_of('/');
		if (slashPos == std::string::npos) { slashPos = m_FilePath.find_last_of('\\'); }

		// Extract filename
		m_FileName = m_FilePath.substr(slashPos + 1);
		// Extract folder path
		m_FolderPath = m_FilePath.substr(0, slashPos + 1);

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Writer::NewLine(bool toIndent, unsigned short lineCount) {
		for (unsigned short lines = 0; lines < lineCount; lines++) {
			*m_Stream << "\n";
			if (toIndent) {
				for (short indent = 0; indent < m_Indent; ++indent) {
					*m_Stream << "\t";
				}
			}
		}
	}
}