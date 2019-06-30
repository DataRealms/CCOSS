#ifndef _RTECONTENTFILE_
#define _RTECONTENTFILE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ContentFile.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ContentFile class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <string>
#include <map>

struct DATAFILE;
struct BITMAP;

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ContentFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A representation of a content file that is stored either directly on
//                  on disk or packaged in another file.
// Parent(s):       Serializable.
// Class history:   04/06/2002 ContentFile created.

class ContentFile
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ContentFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ContentFile object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ContentFile() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ContentFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ContentFile object in system
//                  memory, and also do a Create() in the same line. Create() should
//                  therefore not be called after using this constructor.
// Arguments:       A string defining the path to where the content file itself is located,
//                  either within the package file, or directly on the disk.
//                  A directory

    ContentFile(const char *filePath) { Clear(); Create(filePath); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ContentFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ContentFile object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ContentFile() { Destroy(true); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ContentFile object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ContentFile object ready for use.
// Arguments:       A string defining the path to where the content file itself is located,
//                  either within the package file, or directly on the disk.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const char *filePath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ContentFile to be identical to another, by deep copy.
// Arguments:       A reference to the ContentFile to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const ContentFile &reference);



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ContentFile, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }




//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ContentFile object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   FreeAllLoaded
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Frees all loaded data used by all ContentFile instances. This should
//                  ONLY be done when quitting the app, or after everything else is
//                  completly destroyed
// Arguments:       None.
// Return value:    None.

    static void FreeAllLoaded();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ContentFile
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a ContentFile object
//                  identical to an already existing one.
// Arguments:       A ContentFile object which is passed in by reference.

    ContentFile(const ContentFile &reference);
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        ContentFile assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one ContentFile equal to another.
// Arguments:       A ContentFile reference.
// Return value:    A reference to the changed ContentFile.

    virtual ContentFile & operator=(const ContentFile &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: ContentFile equality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An equality operator for testing if any two ContentFile:es are equal.
// Arguments:       A ContentFile reference as the left hand side operand.
//                  A ContentFile reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are equal or not.

    friend bool operator==(const ContentFile &lhs, const ContentFile &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: ContentFile inequality
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An inequality operator for testing if any two ContentFile:es are unequal.
// Arguments:       A ContentFile reference as the left hand side operand.
//                  A ContentFile reference as the right hand side operand.
// Return value:    A boolean indicating whether the two operands are unequal or not.

    friend bool operator!=(const ContentFile &lhs, const ContentFile &rhs);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDataPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the file path of the content file represented by this ContentFile
//                  object.
// Arguments:       A string with the datafile object name path, like: "datafile.dat#mydataobject"
// Return value:    None.

    virtual void SetDataPath(std::string newDataPath);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the file path of the content file represented by this ContentFile
//                  object.
// Arguments:       None.
// Return value:    A string with the datafile object name path, like "datafile.dat#mydataobject"

    virtual const std::string & GetDataPath() const { return m_DataPath; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of the Data Module this file is inside.
// Arguments:       None.
// Return value:    The ID of the Data Module containing this' file.

    int GetDataModuleID();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the file size of the content file represented by this ContentFile
//                  object, in bytes. This should be called AFTER using any of the GetAs
//                  methods.
// Arguments:       None.
// Return value:    A long describing the file size of the content file.

    virtual unsigned long GetDataSize();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Allegro DATAFILE type of the DATAFILE represented by this
//                  ContentFile.
// Arguments:       None.
// Return value:    An DATAFILE type as described in the Allegro docs.

    virtual int GetDataType();
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAsRawBinary
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as a
//                  binary chunk of data. Note that ownership of the DATA IS NOT TRANSFERRED!
//                  Also, this should only be done once upon startup, since getting the
//                  data again is slow. Get the size of the data by 
// Arguments:       None.
// Return value:    The pointer to the beginning of the raw data loaded from the allegro
//                  .dat datafile. file.
//                  Owenship is NOT transferred! If 0, the file could not be found/loaded.

    virtual char * GetAsRawBinary();
*/


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAsBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and gets the data represtented by this ConentFile object as an
//                  Allegro BITMAP. Note that ownership of the BITMAP IS NOT TRANSFERRED!
//                  Also, this should only be done once upon startup, since getting the
//                  BITMAP again is slow. 
// Arguments:       The Allegro color converison mode to use when loading this bitmap.
//                  Note it will only apply the first time you get a bitmap since it is
//                  only loaded from disk the first time. See allegro docs for the modes.
// Return value:    The pointer to the beginning of the data object loaded from the allegro
//                  .dat datafile. file.
//                  Owenship is NOT transferred! If 0, the file could not be found/loaded.

    virtual BITMAP * GetAsBitmap(int conversionMode = 0);



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadAndReleaseBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads and transfers the data represtented by this ConentFile object as
//                  an Allegro BITMAP. Note that ownership of the BITMAP IS TRANSFERRED!
//                  Also, this is relatively slow since it reads the data from disk each time.
// Arguments:       The Allegro color converison mode to use when loading this bitmap.
//                  Note it will only apply the first time you get a bitmap since it is
//                  only loaded from disk the first time. See allegro docs for the modes.
// Return value:    The pointer to the BITMAP loaded from disk Owenship IS transferred!
//                  If 0, the file could not be found/loaded.

    virtual BITMAP * LoadAndReleaseBitmap(int conversionMode = 0);


/* This is foolish
//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   ClearAllLoadedData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the previously loaded data shared among all ContentFile
//                  instances. NEVER USE THIS UNLESS ALL INSTANCES OF CONTENTFILE ARE GONE!
// Arguments:       None.
// Return value:    None.

    static void ClearAllLoadedData();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static const std::string m_ClassName;

    // Static map containing all the already loaded binary data. First in pair is the data,
    // second is size in bytes.
//    static std::map<std::string, std::pair<char *, long> > m_sLoadedBinary;

    // Static map containing all the already loaded BITMAP:s and their paths
    static std::map<std::string, BITMAP *> m_sLoadedBitmaps;


    // Path to this ContentFile's Datafile Object's path. "datafile.dat#objectname"
    // In the case of an animation, this filename/name will be appended with 000, 001, 002 etc
    std::string m_DataPath;

    // Data Module ID of where this was loaded from
    int m_DataModuleID;

    // Non-ownership pointer to the loaded data for convenience. Do not release/delete this.
    void *m_pLoadedData;
    // Size of loaded data
    long m_LoadedDataSize;

    // This is only if the data is loaded from a datafile;
    // needs to be saved so that it can be unloaded as some point
    DATAFILE *m_pDataFile;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ContentFile, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File