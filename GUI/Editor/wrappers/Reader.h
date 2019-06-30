#ifndef _RTEREADER_
#define _RTEREADER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Reader.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Reader class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <ISTREAM>
#include <FSTREAM>
#include <string>
#include <list>
#include "Writer.h"

namespace RTE
{

// Not necessary anymore since enforce *.rte in dirnames if not packaged.
//const char g_ReadPackageExtension[8] = ".rte";



//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Reader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads RTE objects from std::istreams.
// Parent(s):       None.
// Class history:   01/20/2002 Reader created.

class Reader
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Reader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Reader object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Reader() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Reader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Reader object in system
//                  memory. Create() should be called before using the object.
//                  Whether object defintions read here overwrite existing ones with the
//                  same names.
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this Reader's reading.
//                  Whether it's ok for the file to not be there, ie we're only trying to
//                  open, and if it's not there, then fail silently.
// Arguments:       Path to the file to open for reading.

    Reader(const char *filename, bool overwrites = false, void (*fpProgressCallback)(std::string, bool) = 0, bool failOK = false) { Clear(); Create(filename, overwrites, fpProgressCallback, failOK); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Reader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Reader object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Reader() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Reader object ready for use.
// Arguments:       The filename of the file to open and read from. If the file isn't found
//                  directly on disk, the first directory in the path will be used to try
//                  open a package of that name. If that doesn't work, and error code will
//                  be returned.
//                  Whether object defintions read here overwrite existing ones with the
//                  same names.
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this Reader's reading.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const char *filename, bool overwrites = false, void (*fpProgressCallback)(std::string, bool) = 0, bool failOK = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Reader, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Reader object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Operators:       Elemental types stream extractions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stream extraction operator overloads for all the elemental types
// Arguments:       A reference to the variable that will be filled by the extracted data.
// Return value:    A Reader reference for further use in an expression.

    virtual Reader & operator>>(bool &var)              { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(char &var)              { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(unsigned char &var)     { Eat(); int temp; *m_pStream >> temp; var = temp; return *this; }
    virtual Reader & operator>>(short &var)             { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(unsigned short &var)    { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(int &var)               { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(unsigned int &var)      { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(long &var)              { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(unsigned long &var)     { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(float &var)             { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(double &var)            { Eat(); *m_pStream >> var; return *this; }
    virtual Reader & operator>>(char * var)             { Eat(); *m_pStream >> var; return *this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Operators:       std::string extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stream extraction operator overloads for std::string.
// Arguments:       A reference to the variable that will be filled by the extracted data.
// Return value:    A Reader reference for further use in an expression.

    virtual Reader & operator>>(std::string &var);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Reader.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Reader.

    virtual const std::string & GetClassName() const { return ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetReadModuleName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of Data Module this reader is reading from.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Reader.

    virtual const std::string & GetReadModuleName() const { return m_DataModuleName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetReadModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of Data Module this reader is reading from.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Reader.

    virtual int GetReadModuleID() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStream
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a pointer to the istream of this reader.
// Arguments:       None.
// Return value:    A pointer to the istream object fo this reader.

    std::istream * GetStream() { return m_pStream; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOK
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is still OK to read from. If file isn't present,
//                  etc, this will return false.
// Arguments:       None.
// Return value:    Whether this Reader's stream is OK or not.

    bool IsOK() { return m_pStream && m_pStream->good(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentFilePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the path of the current file this reader is reading from.
// Arguments:       None.
// Return value:    A string with the path, relative from the working directory.

    std::string GetCurrentFilePath() const { return m_FilePath; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCurrentFileLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the line of the current file line this reader is reading from.
// Arguments:       None.
// Return value:    The line number that will be read from next.

    int GetCurrentFileLine() const { return m_CurrentLine; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPresetOverwriting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether objects read from this will be overwritng any existing
//                  ones with the same names.
// Arguments:       None.
// Return value:    Whether this overwrites or not.

    bool GetPresetOverwriting() { return m_OverwriteExisting; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPresetOverwriting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether objects read from this will be overwritng any existing
//                  ones with the same names.
// Arguments:       Whether this should overwrite existing definitions or not.
// Return value:    None.

    void SetPresetOverwriting(bool overwrites = true) { m_OverwriteExisting = overwrites; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the mass of this Reader.
// Arguments:       A float specifying the new mass value in Kilograms (kg).
// Return value:    None.

    void SetMass(const float newMass) { m_Mass = newMass; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Eat
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Eats all whitespace and newlines and comment lines (which start with 
//                  '//') so that the next thing to be read will be actual data.
// Arguments:       None.
// Return value:    Whether there is more data to read from the filestreams after this eat.

    bool Eat();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the rest of the line from the context object Reader's stream
//                  current location.
// Arguments:       The c-string that will be filled out with the line.
//                  An int specifying the max size of the c-string.
// Return value:    None.

    void ReadLine(char *locString, int size);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the rest of the line from the context object Reader's stream
//                  current location.
// Arguments:       None.
// Return value:    The std::string that will hold the line's contents.

    std::string ReadLine();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadTo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads from teh current position up to a specific character or eof.
// Arguments:       Which character to stop reading at.
//                  Whether to also eat the terminator when it is encountered, or to leave
//                  it in the stream.
// Return value:    The std::string that will hold what has been read up till, but not
//                  including the terminator char.

    std::string ReadTo(char terminator, bool eatTerminator = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NextProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Lines up the reader with the next property of the current object.
// Arguments:       None.
// Return value:    Whether there are any more properties to be read by the current object.

    bool NextProperty();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadPropName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the next property name from the context object Reader's stream
//                  after eating all whitespace including newlines up till the first
//                  newline char. Basically gets anything between the last newline before
//                  text to the next "=" after that.
// Arguments:       None.
// Return value:    The whitespace-trimmed std::string that will hold the next property's
//                  name.

    std::string ReadPropName();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadPropValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the next property value from the context object Reader's stream
//                  after eating all whitespace including newlines up till the first
//                  newline char. Basically gets anything after the last "=" and up to
//                  the next newline after that.
// Arguments:       None.
// Return value:    The whitespace-trimmed std::string that will hold the next property
//                  value.

    std::string ReadPropValue();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TrimString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes out whitespace from the beginning and the end of a string.
// Arguments:       None.
// Return value:    The string that was passed in, sans whitespace in the front and end.

    std::string TrimString(std::string &stringToTrim);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReportError
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes an error message box pop up for the user that tells them
//                  something went wrong with the reading, and where.
// Arguments:       The message describing what's wrong.
// Return value:    None.

    void ReportError(std::string errorDesc);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


    struct StreamInfo
    {
        StreamInfo(std::ifstream *pStream, std::string filePath, int currentLine, int prevIndent):
            m_pStream(pStream), m_FilePath(filePath), m_CurrentLine(currentLine), m_PreviousIndent(prevIndent) { ; }

        // Owned by the reader, so not deleted by this
        std::ifstream *m_pStream;
        std::string m_FilePath;
        int m_CurrentLine;
        int m_PreviousIndent;
    };

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartIncludeFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     When ReadPropName encounters the property name "IncludeFile", it will
//                  automatically call this function to get started reading on that file.
//                  This will create a new stream to the include file.
// Arguments:       None.
// Return value:    Whether the include file was found and opened ok or not.

    bool StartIncludeFile();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndIncludeFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This should be called when eof is detected in an included file stream.
//                  It will destroy the current stream pop the top stream off the stream
//                  stack to resume reading from it instead.
// Arguments:       None.
// Return value:    Wheteher there were any stream on the stack to resume.

    bool EndIncludeFile();


    // Member variables
    static const std::string ClassName;
    // Currently used stream, is not on the StreamStack until a new stream is opned
    std::ifstream *m_pStream;
    // Currently used stream's filepath
    std::string m_FilePath;
    // The line number the stream is on
    int m_CurrentLine;
    // Stack of stream and filepath pairs, each one representing a file opened to read from within another
    std::list<StreamInfo> m_StreamStack;
//    zip::izipfile *m_Package;
    // Count of tabs encountered on the last line Eat() ate.
    int m_PreviousIndent;
    // Difference in indentation from the last line to the current line
    int m_IndentDifference;
    // When NextProperty() has returned false, indicating that there were no more properties to read on that object,
    // this is incremented until it matches -m_IndentDifference, and then NextProperty will start returning true again
    int m_ObjectEndings;
/* Couldn't figure out how to determine a new object started as opposed to a one-line property
    // Shows wheter we just started reading a new property/object, and should have a positive indentdiff on the first line
    // If it's 0 or negative, it means the new object to be read didn't ahve any properties defined at all
    bool m_NewObject;
*/
    // All streams have been depleted
    bool m_EndOfStreams;

    // Funciton pointer to report our reading progress to, by calling it and passing a descriptive string to it.
    void (*m_fpReportProgress)(std::string, bool);
    // String containing the proper amount of tabs for the report
    std::string m_ReportTabs;
    // Only the name of the currently read file, excluding the path
    std::string m_FileName;
    // The current name of the data module being read from, including the .rte extension
    std::string m_DataModuleName;
    // The current ID of the data module being read from
    int m_DataModuleID;
    // Whether object instances read from this should overwrite any already existing ones with the same names
    bool m_OverwriteExisting;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Reader, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Reader(const Reader &reference);
    Reader & operator=(const Reader &rhs);

};

} // namespace RTE

#endif // File