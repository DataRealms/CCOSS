#ifndef _RTEWRITER_
#define _RTEWRITER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Writer.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Writer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <ostream>
#include <fstream>
//#include <zipstream.h>
#include <string>

namespace RTE
{

const char g_WritePackageExtension[8] = ".rte";


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Writer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes RTE objects to std::ostreams.
// Parent(s):       None.
// Class history:   02/14/2002 Writer created.

class Writer
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Writer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Writer object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Writer() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Writer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Writer object in system
//                  memory. Create() should be called before using the object.
// Arguments:       Path to the file to open for writing.
//                  Whether to append to the file if it exists, or to overwrite it.

    Writer(const char *filename, bool append = false) { Clear(); Create(filename, append); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Writer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Writer object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Writer() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Writer object ready for use.
// Arguments:       The filename of the file to open and write to. If the file path doesn't
//                  exist, the first directory name is used in an attempt to open a package
//                  and then read a file from within that.
//                  Whether to append to the file if it exists, or to overwrite it.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const char *filename, bool append = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Writer, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Writer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  WriterOK
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the writer is ok to start accepting data streamed to it.
// Arguments:       None.
// Return value:    Whetehr writer is ready for data.

    virtual bool WriterOK();


//////////////////////////////////////////////////////////////////////////////////////////
// Operators:       Elemental types stream insertions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stream insertion operator overloads for all the elemental types
// Arguments:       A reference to the variable that will be written to the ostream.
// Return value:    A Writer reference for further use in an expression.

    virtual Writer & operator<<(const bool &var)            { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const char &var)            { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const unsigned char &var)   { int temp = var; *m_pStream << temp; return *this; }
    virtual Writer & operator<<(const short &var)           { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const unsigned short &var)  { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const int &var)             { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const unsigned int &var)    { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const long &var)            { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const unsigned long &var)   { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const float &var)           { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const double &var)          { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const char *var)            { *m_pStream << var; return *this; }
    virtual Writer & operator<<(const std::string &var)     { *m_pStream << var; return *this; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the mass of this Writer.
// Arguments:       A float specifying the new mass value in Kilograms (kg).
// Return value:    None.

    void SetMass(const float newMass) { m_Mass = newMass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this Writer.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    float GetMass() const { return m_Mass; }
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this Writer. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update() = 0;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ObjectStart
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to specify the start of an object to be written.
// Arguments:       The classname of the object about to be written.
// Return value:    None.

    virtual void ObjectStart(const std::string &className)
    {
        *m_pStream << className;
        ++m_Indent;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ObjectEnd
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to specify the end of an object that has just been written.
// Arguments:       None.
// Return value:    None.

    virtual void ObjectEnd()
    {
        --m_Indent;

        // Make an extra line between big object definitions
        if (m_Indent == 0)
            *m_pStream << "\n\n";
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NewProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to specify the name of a new property to be written.
// Arguments:       The name of the property to be written.
// Return value:    None.

    virtual void NewProperty(std::string propName)
    {
        NewLine();
        *m_pStream << propName;
        *m_pStream << " = ";
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NoObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Marks that there is a null reference to an object here.
// Arguments:       None.
// Return value:    None.

    virtual void NoObject()
    {
//        NewLine();
        *m_pStream << "None";
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NewLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a new line that is properly indented.
// Arguments:       None.
// Return value:    None.

    virtual void NewLine()
    {
        *m_pStream << "\n";
        for (int i = 0; i < m_Indent; ++i)
            *m_pStream << "\t";
    }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Manipulator:     newl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a new line that is properly indented.
// Arguments:       An ostream reference.
// Return value:    An ostream reference.

    static std::ostream & newl(std::ostream &stream)
    {
        stream << "\n";
        for (int i = 0; i < m_Indent; ++i)
            stream << "\t";
        return stream;
    }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string ClassName;
    std::ofstream *m_pStream;
//    zip::ozipfile *m_Package;
    int m_Indent;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Writer, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Writer(const Writer &reference);
    Writer & operator=(const Writer &rhs);

};

} // namespace RTE

#endif // File