#ifndef _RTEICON_
#define _RTEICON_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Icon.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Icon class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "ContentFile.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Icon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Represents an Icon in the interface that can be loaded and stored from
//                  different data modules etc.
// Parent(s):       Entity.
// Class history:   05/27/2010 Icon created.

class Icon:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(Icon)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Icon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Icon object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Icon() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Icon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Icon object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Icon() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Icon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Icon object
//                  identical to an already existing one.
// Arguments:       A Icon object which is passed in by reference.

    Icon(const Icon &reference) { if (this != &reference) { Clear(); Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Icon assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Icon equal to another.
// Arguments:       A Icon reference.
// Return value:    A reference to the changed Atom.

    Icon & operator=(const Icon &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Icon object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Icon to be identical to another, by deep copy.
// Arguments:       A reference to the Icon to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Icon &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
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

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Icon, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Icon to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       A Writer that the Icon will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Icon object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Icon.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Icon.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFrameCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of frames in this Icon's animation.
// Arguments:       None.
// Return value:    The number of frames in the animation

    int GetFrameCount() const { return m_FrameCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmaps8
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the array of 8-bit bitmaps of this Icon, as many as GetFrameCount
//                  says. Neither the array nor the BITMAPs are transferred ownership!
// Arguments:       None.
// Return value:    The BITMAPs in 8bpp of this Icon - OINT of neither the array or BITMAPs.

    BITMAP ** GetBitmaps8() const { return m_apBitmaps8; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmaps32
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the array of 32-bit bitmaps of this Icon, as many as GetFrameCount
//                  says. Neither the array nor the BITMAPs are transferred ownership!
// Arguments:       None.
// Return value:    The BITMAPs in 32bpp of this Icon - OINT of neither the array or BITMAPs.

    BITMAP ** GetBitmaps32() const { return m_apBitmaps32; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSettleIconID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If this material transforms into something else when settling into the
//                  terrain, this will return that different material index. If not, it will
//                  just return the regular index of this material.
// Arguments:       None.
// Return value:    The the settling material index of this MOSPixel, or the regular index.

    virtual unsigned char GetSettleIconID() const { return settleIcon != 0 ? settleIcon : id; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UsesOwnColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether or not to use the Icon's own color when a pixel
//                  of this Icon is knocked loose from the terrain.
// Arguments:       None.
// Return value:    A bool that if false, means tthe terrain pixel's color should be applied
//                  instead.

    bool UsesOwnColor() const { return m_UseOwnColor; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static Entity::ClassInfo m_sClass;
    // The file pointing to the terrain texture of this Icon
    ContentFile m_BitmapFile;
    // Number of frames in this Icon's animation
    int m_FrameCount;
    // The bitmaps of this Icon, in both bit depth flavors. The dynamic arrays ARE owned here, but NOT the BITMAPs!
    BITMAP **m_apBitmaps8;
    BITMAP **m_apBitmaps32;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Icon, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


};

} // namespace RTE

#endif // File