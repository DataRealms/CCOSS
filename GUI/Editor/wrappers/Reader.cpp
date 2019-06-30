//////////////////////////////////////////////////////////////////////////////////////////
// File:            Reader.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Reader class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Reader.h"
#include <cctype>
#include <fstream>

using namespace std;
//using namespace zip;

namespace RTE
{

const string Reader::ClassName = "Reader";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Reader, effectively
//                  resetting the members of this abstraction level only.

void Reader::Clear()
{
    m_pStream = 0;
    m_FilePath.clear();
    m_CurrentLine = 1;
    m_StreamStack.clear();
    m_PreviousIndent = 0;
    m_IndentDifference = 0;
    m_ObjectEndings = 0;
    m_EndOfStreams = false;
    m_fpReportProgress = 0;
    m_ReportTabs = "\t";
    m_FileName.clear();
    m_DataModuleName.clear();
    m_DataModuleID = -1;
    m_OverwriteExisting = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Reader object ready for use.

int Reader::Create(const char *filename, bool overwrites, void (*fpProgressCallback)(std::string, bool), bool failOK)
{
    m_FilePath.assign(filename);

    // Extract just the filename
    int slashPos = m_FilePath.find_first_of('/');
    if (slashPos == string::npos)
        slashPos = m_FilePath.find_first_of('\\');
    m_FileName = m_FilePath.substr(slashPos + 1);

    m_DataModuleName = m_FilePath.substr(0, slashPos);
    m_DataModuleID = 0;//g_PresetMan.GetModuleID(m_DataModuleName);
// This is OK, may be able to do it later when needed
//    Assert(m_DataModuleID > 0, "Couldn't establish which DataModule we're reading from when creating Reader!");

    m_pStream = new ifstream(filename);
    if (!m_pStream && !failOK)
		return -1;
        

    m_OverwriteExisting = overwrites;

    // Report that we're starting a new file
    m_fpReportProgress = fpProgressCallback;
    if (m_fpReportProgress && m_pStream->good())
    {
        char report[512];
        sprintf(report, "\t%s on line %i", m_FileName.c_str(), m_CurrentLine);
        m_fpReportProgress(string(report), true);
    }


// Only streams not currently used are pushed onto the streamstack
//    m_StreamStack.push_back(pair<istream *, string>(new ifstream(filename), string(filename)));
/*
    if (!*m_pStream) {
        delete m_pStream;

        // Get the packagename from the path, since the conventional path didn't work.
        char packageName[128];
        for (int i = 0; i < strlen(filename) && filename[i] != '/'; ++i)
            packageName[i] = filename[i];
        packageName[i] = '\0';
//        strcpy(&packageName[i], g_ReadPackageExtension);

        m_Package = new izipfile(packageName);
        if (!m_Package->isOk())
            return -1;
        m_pStream = new izipstream(*m_Package, &filename[i + 1]);
        if (!*m_pStream)
            return -1;
    }
*/
    return m_pStream->good() ? 0 : -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Reader object.

void Reader::Destroy(bool notInherited)
{
    delete m_pStream;

    // Delete all the streams in the stream stack
    for (list<StreamInfo>::iterator itr = m_StreamStack.begin(); itr != m_StreamStack.end(); ++itr)
        delete (*itr).m_pStream;

//    if (!notInherited)
//        Serializable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Operators:       std::string extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stream extraction operator overloads for std::string.

Reader & Reader::operator>>(std::string &var)
{
    var.assign(ReadLine());
    return *this;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetReadModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of Data Module this reader is reading from.

int Reader::GetReadModuleID() const
{
    // If we have an invalid ID, try to get a valid one based on the name we do have
    return 0; //m_DataModuleID < 0 ? g_PresetMan.GetModuleID(m_DataModuleName) : m_DataModuleID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Eat
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Eats all whitespace and newlines and comment lines (which start with 
//                  '//') so that the next thing to be read will be actual data.

bool Reader::Eat()
{
    char peek;
    int indent = 0;
    bool ateLine = false;
    char report[512];

    while (1)
    {
        peek = m_pStream->peek();

        // If we have hit the end and don't have any files to resume, then quit and indicate that
        if (m_pStream->eof())
        {
            return EndIncludeFile();
        }

        // Not eof but still got junk back... something went to shit
        if (peek == -1)
            ReportError("Something went wrong reading the line; make sure it is providing the expected type");

        // Eat spaces
        if (peek == ' ')
        {
            m_pStream->ignore(1);
        }
        // Eat tabs, and count them
        else if (peek == '\t')
        {
            indent++;
            m_pStream->ignore(1);
        }
        // Eat newlines and reset the tab count for the new line, also count the lines
        else if (peek == '\n' || peek == '\r')
        {
            // So we don't count lines twice when there are both newline and carriage return at the end of lines
            if (peek == '\n')
            {
                m_CurrentLine++;

                // Only report every few lines
                if (m_fpReportProgress && (m_CurrentLine % 100 == 0))
                {
                    char report[512];
                    sprintf(report, "%s%s reading line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
                    m_fpReportProgress(string(report), false);
                }
            }

            indent = 0;
            ateLine = true;
            m_pStream->ignore(1);
        }
        // Comment line?
        else if (m_pStream->peek() == '/')
        {
            char temp = m_pStream->get();
            // Confirm that it's a comment line, if so eat it and continue
            if (m_pStream->peek() == '/')
            {
                while (m_pStream->peek() != '\n' && m_pStream->peek() != '\r' && !m_pStream->eof())
                    m_pStream->ignore(1);
            }
            // Block comment
            else if (m_pStream->peek() == '*')
            {
                // Find the matching "*/"
                while (!(m_pStream->get() == '*' && m_pStream->peek() == '/') && !m_pStream->eof())
                    ;
                // Eat that final '/'
                if (!m_pStream->eof())
                    m_pStream->ignore(1);
            }
            // Not a comment, so it's data, so quit.
            else
            {
                m_pStream->putback(temp);
                break;
            }
        }
        else
            break;
    }

    // Only do this if we actually ate an endline
    // This precaution enables us to use Eat repeatedly without messing up the indentation tracking logic
    if (ateLine)
    {
        // Get indentation difference from the last line of the last call to Eat(),
        // and the last line of this call to Eat().
        m_IndentDifference = indent - m_PreviousIndent;

        // Save the last tab count
        m_PreviousIndent = indent;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the rest of the line from the context object Reader's stream
//                  current location.

void Reader::ReadLine(char *locString, int size)
{
    // Make sure we're about to get real data.
    Eat();

    char temp, peek = m_pStream->peek();
//    m_pStream->getline(locString, size, '\n');
	int i = 0;
    for (i = 0; i < size - 1 && peek != '\n' && peek != '\r' && peek != '\t'; ++i)
    {
        temp = m_pStream->get();

        // Check for line comment "//"
        if (peek == '/')
        {
            if (m_pStream->peek() == '/')
            {
                m_pStream->putback(temp);
                break;
            }
        }

        if (m_pStream->eof())
        {
            EndIncludeFile();
            break;
        }
        if (!m_pStream->good())
            ReportError("Stream failed for some reason");

        locString[i] = temp;

        peek = m_pStream->peek();
    }
    locString[i] = '\0';
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the rest of the line from the context object Reader's stream
//                  current location.

string Reader::ReadLine()
{
    // Make sure we're about to get real data.
    Eat();

    string retString;

    char temp, peek = m_pStream->peek();
    while (peek != '\n' && peek != '\r' && peek != '\t')
    {
        temp = m_pStream->get();

        // Check for line comment "//"
        if (peek == '/')
        {
            if (m_pStream->peek() == '/')
            {
//                m_pStream->putback(temp);
                m_pStream->unget();
                break;
            }
        }

        if (m_pStream->eof())
        {
// Let Eat respond to eof instead
//            EndIncludeFile();
            break;
        }
        if (!m_pStream->good())
            ReportError("Stream failed for some reason");

        retString.append(1, temp);

        peek = m_pStream->peek();
    }

    return retString;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads from the current position up to a specific character or eof.

string Reader::ReadTo(char terminator, bool eatTerminator)
{
    string retString;

    char temp, peek = m_pStream->peek();
    while (peek != terminator)
    {
        temp = m_pStream->get();

        if (m_pStream->eof())
        {
// Let Eat respond to eof instead
//            EndIncludeFile();
            break;
        }
        if (!m_pStream->good())
            ReportError("Stream failed for some reason");

        retString.append(1, temp);

        peek = m_pStream->peek();
    }

    // Eat the terminator if instructed to
    if (eatTerminator && peek == terminator)
        m_pStream->get();

    return retString;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NextProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Lines up the reader with the next property of the current object.

bool Reader::NextProperty()
{
    if (!Eat())
        return false;

    if (m_EndOfStreams)
        return false;

    // If there are fewer tabs on the last line eaten this time,
    // that means there are no more properties to read on this object
    if (m_ObjectEndings < -m_IndentDifference)
    {
        m_ObjectEndings++;
        return false;
    }
    
    m_ObjectEndings = 0;
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadPropName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the next property name from the context object Reader's stream
//                  after eating all whitespace including newlines up till the first
//                  newline char. Basically gets anything between the last newline before
//                  text to the next "=" after that.

string Reader::ReadPropName()
{
    // Make sure we're about to get real data.
    Eat();

    string retString;
    char temp, peek;

    while (1)
    {
        peek = m_pStream->peek();

        if (peek == '=')
        {
            m_pStream->ignore(1);
            break;
        }

        if (peek == '\n' || peek == '\r' || peek == '\t')
        {
// TODO add file name and line number here!
            ReportError("Property name wasn't followed by a value");
// TODO handle this gracefully by ignoring the property and reading the next somehow instead
        }

        temp = m_pStream->get();
        if (m_pStream->eof())
        {
            EndIncludeFile();
            break;
        }

        if (!m_pStream->good())
            ReportError("Stream failed for some reason");

        retString.append(1, temp);
    }

    // Trim the string of whitespace
    retString = TrimString(retString);

    // If the property name turns out to be the special IncludeFile,
    // then open that file and read the first property from it instead.
    if (retString == "IncludeFile")
    {
        StartIncludeFile();
        // Return the first property name in the new file, this is to make the file inclusion seamless
        // Alternatively, if StartIncludeFile failed, this will just grab the next prop name and ignore the failed IncludeFile property
        retString = ReadPropName();
    }

    return retString;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadPropValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the next property value from the context object Reader's stream
//                  after eating all whitespace including newlines up till the first
//                  newline char. Basically gets anything after the last "=" and up to
//                  the next newline after that.

string Reader::ReadPropValue()
{
    string fullLine = ReadLine();
    int begin = fullLine.find_first_of('=');
	string subStr = fullLine.substr(begin + 1);
    return TrimString(subStr);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrimString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes out whitespace from the beginning and the end of a string.

string Reader::TrimString(string &stringToTrim)
{
    if (stringToTrim.empty())
        return "";

    int start = stringToTrim.find_first_not_of(' ');
    int end = stringToTrim.find_last_not_of(' ');

    if (start > end)
        return "";
    else if (start == 0 && end == stringToTrim.size() - 1)
        return stringToTrim;

    return stringToTrim.substr(start, (end - start) + 1);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReportError
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes an error message box pop up for the user that tells them
//                  something went wrong with the reading, and where.

void Reader::ReportError(std::string errorDesc)
{
    char error[1024];
    sprintf(error, "%s in %s at line %i!", errorDesc.c_str(), m_FilePath.c_str(), m_CurrentLine);
    //DDTAbort(error);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartIncludeFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     When ReadPropName encounters the property name "IncludeFile", it will
//                  automatically call this function to get started reading on that file.
//                  This will create a new stream to the include file.

bool Reader::StartIncludeFile()
{
    // Report that we're including a file
    if (m_fpReportProgress)
    {
        char report[512];
        sprintf(report, "%s%s on line %i includes:", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
        m_fpReportProgress(string(report), false);
    }

    // Push the current stream onto the streamstack for future retrieval when the new include file has run out of data.
    m_StreamStack.push_back(StreamInfo(m_pStream, m_FilePath, m_CurrentLine, m_PreviousIndent));

    // Get the file path from the stream
    m_FilePath = ReadPropValue();
    m_pStream = new ifstream(m_FilePath.c_str());
    if (m_pStream->fail())
    {
        // Backpedal and set up to read the next property in the old stream
        delete m_pStream;
        m_pStream = m_StreamStack.back().m_pStream;
        m_FilePath = m_StreamStack.back().m_FilePath;
        m_CurrentLine = m_StreamStack.back().m_CurrentLine;
        m_PreviousIndent = m_StreamStack.back().m_PreviousIndent;
        m_StreamStack.pop_back();

// TODO make this graceful log error instead
        ReportError("Failed to open included data file");

        Eat();
        return false;
    }

    // Line counting starts with 1, not 0
    m_CurrentLine = 1;
    // This is set to 0, because locally in the included file, all properties start at that count
    m_PreviousIndent = 0;

    // Extract just the filename
    int slashPos = m_FilePath.find_first_of('/');
    if (slashPos == string::npos)
        slashPos = m_FilePath.find_first_of('\\');
    m_FileName = m_FilePath.substr(slashPos + 1);

    // Report that we're starting a new file
    if (m_fpReportProgress)
    {
        m_ReportTabs = "\t";
        for (int i = 0; i < m_StreamStack.size(); ++i)
            m_ReportTabs.append("\t");

        char report[512];
        sprintf(report, "%s%s on line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
        m_fpReportProgress(string(report), true);
    }

    // Eat away any fluff in the beginning of the new file
    Eat();

    // indicate sucess
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndIncludeFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This should be called when eof is detected in an included file stream.
//                  It will destroy the current stream pop the top stream off the stream
//                  stack to resume reading from it instead.

bool Reader::EndIncludeFile()
{
    // Do final report on the file we're closing
    if (m_fpReportProgress)
    {
        char report[512];
        sprintf(report, "%s%s - done! %c", m_ReportTabs.c_str(), m_FileName.c_str(), -42);
        m_fpReportProgress(string(report), false);
    }

    if (m_StreamStack.empty())
    {
        m_EndOfStreams = true;
        return false;
    }

    // Replace the current included stream with the parent one
    delete m_pStream;
    m_pStream = m_StreamStack.back().m_pStream;
    m_FilePath = m_StreamStack.back().m_FilePath;
    m_CurrentLine = m_StreamStack.back().m_CurrentLine;
    // Observe it's being added, not just replaced. This is to keep proper track when exiting out of a file
    m_PreviousIndent += m_StreamStack.back().m_PreviousIndent;
    m_StreamStack.pop_back();

    // Extract just the filename
    int slashPos = m_FilePath.find_first_of('/');
    if (slashPos == string::npos)
        slashPos = m_FilePath.find_first_of('\\');
    m_FileName = m_FilePath.substr(slashPos + 1);

    // Report that we're going back a file
    if (m_fpReportProgress)
    {
        m_ReportTabs = "\t";
        for (int i = 0; i < m_StreamStack.size(); ++i)
            m_ReportTabs.append("\t");

        char report[512];
        sprintf(report, "%s%s on line %i", m_ReportTabs.c_str(), m_FileName.c_str(), m_CurrentLine);
        m_fpReportProgress(string(report), true);
    }

    // Set up the resumed file for reading again
    Eat();

    return true;
}

} // namespace RTE