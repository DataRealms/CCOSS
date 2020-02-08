#ifndef _RTESERIALIZEABLE_
#define _RTESERIALIZEABLE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Serializable.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Serializable class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff

#include "Reader.h"
#include "Writer.h"
#include "DDTTools.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  Serializable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The base class that specifies common creation/destruction patterns
//                  associated with reading and writing member data from disk. Is only
//                  intended to be inherited from in one level, and
// Parent(s):       None.
// Class history:   01/31/2002 Serializable created.

class Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Serializable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Serializable object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Serializable() { /*Clear();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Serializable ready for use, usually after all necessary
//                  properties have been set with Create(Reader).
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create() { return 0; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an Serializable to be identical to another, by deep copy.
// Arguments:       A reference to the Serializable to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Serializable &reference);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Serializable ready for use.
// Arguments:       A Reader that the Serializable will create itself from.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
//                  Whether to do any additional initialization of the object after reading
//                  in all the properties from the Reader. This is done by calling Create().
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true)
    {
        if (checkType && reader.ReadPropValue() != GetClassName())
        {
            reader.ReportError("Wrong type in Reader when passed to Serializable::Create()");
            return -1;
        }

        // This is the engine for processing all properties of this Serializable upon read creation
        while (reader.NextProperty())
        {
            string propName = reader.ReadPropName();
			// We need to check if propName != "" because ReadPropName may return "" when it reads 
			// an InlcudeFile without any properties (say, they are all commented out) and it's the last 
			// line in file. Also ReadModuleProperty may return "" when it skips IncludeFile till the 
			// end of file
            if (propName != "" && ReadProperty(propName, reader) < 0)
            {
// TODO Log here!
            }
        }

        // Now do all the additional initalizing needed
        if (doCreate)
            return Create();
        else
            return 0;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    // Eat the value of the property which failed to read
    virtual int ReadProperty(std::string propName, Reader &reader)
    {
        reader.ReadPropValue();
        reader.ReportError("Could not match property");
        return -1;
    }

// Considered making this protected because we need to call Writer::ObjectEnd() after saving, so the << does that and we force users to use it only
// But.... nah. just remember to use <<, or  remember to call ObjectEnd
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Serializable to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       A Writer that the Serializable will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const { writer.ObjectStart(GetClassName()); return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Serializable, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Serializable Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Serializable from a Reader.
// Arguments:       A Reader reference as the left hand side operand.
//                  An Serializable reference as the right hand side operand.
// Return value:    A Reader reference for further use in an expression.

    friend Reader & operator>>(Reader &reader, Serializable &operand)
    {
        operand.Create(reader);
        return reader;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Serializable Pointer Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Serializable from a Reader.
// Arguments:       A Reader reference as the left hand side operand.
//                  An Serializable pointer as the right hand side operand.
// Return value:    A Reader reference for further use in an expression.

    friend Reader & operator>>(Reader &reader, Serializable *operand)
    {
        if (operand)
            operand->Create(reader);
        return reader;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Serializable Writer insertion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Writer insertion operator for sending a Serializable to a Writer.
// Arguments:       A Writer reference as the left hand side operand.
//                  A Serializable reference as the right hand side operand.
// Return value:    A Writer reference for further use in an expression.

    friend Writer & operator<<(Writer &writer, const Serializable &operand)
    {
        operand.Save(writer);
        writer.ObjectEnd();
        return writer;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Serializable Pointer Writer insertion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Writer insertion operator for sending a Serializable to a Writer.
// Arguments:       A Writer reference as the left hand side operand.
//                  A Serializable pointer as the right hand side operand.
// Return value:    A Writer reference for further use in an expression.

    friend Writer & operator<<(Writer &writer, const Serializable *operand)
    {
        if (operand)
        {
            operand->Save(writer);
            writer.ObjectEnd();
        }
        else
            writer.NoObject();
        return writer;
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Serializable.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Serializable.

    virtual const std::string & GetClassName() const = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

//    static const std::string ClassName;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Object, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear() { ; }

};

} // namespace RTE

#endif // File