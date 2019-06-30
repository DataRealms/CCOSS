#ifndef _RTETERRAINDEBRIS_
#define _RTETERRAINDEBRIS_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            TerrainDebris.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the TerrainDebris class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "ContentFile.h"
#include "Vector.h"
#include "Material.h"
#include <vector>
#include <map>

//struct BITMAP;

namespace RTE
{   

class SLTerrain;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           TerrainDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Specifies a certain type of debris scattered randomly throughout a
//                  Terrain, before TerrainObjects are placed.
// Parent(s):       Entity.
// Class history:   08/28/2006 TerrainDebris created.

class TerrainDebris:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(TerrainDebris)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     TerrainDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a TerrainDebris object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    TerrainDebris() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     TerrainDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a TerrainDebris object
//                  identical to an already existing one.
// Arguments:       A TerrainDebris object which is passed in by reference.

    TerrainDebris(const TerrainDebris &reference) { Clear(); Create(reference); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~TerrainDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a TerrainDebris object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~TerrainDebris() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainDebris object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TerrainDebris to be identical to another, by deep copy.
// Arguments:       A reference to the TerrainDebris to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const TerrainDebris &reference);


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
// Description:     Resets the entire TerrainDebris, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainDebris to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the TerrainDebris will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TerrainDebris object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: TerrainDebris addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for TerrainDebris:s which adds together the
//                  Atom:s of two Groups and merges them into one.
// Arguments:       An TerrainDebris reference as the left hand side operand..
//                  An TerrainDebris reference as the right hand side operand.
// Return value:    The resulting TerrainDebris.

    friend TerrainDebris operator+(const TerrainDebris &lhs, const TerrainDebris &rhs);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: TerrainDebris pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for TerrainDebris:s pointer which adds together
//                  the Atom:s of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!
// Arguments:       An TerrainDebris pointer as the left hand side operand.
//                  An TerrainDebris pointer as the right hand side operand.
// Return value:    The resulting dynamically allocated TerrainDebris. OWNERSHIP IS TRANSFERRED!

    friend TerrainDebris * operator+(const TerrainDebris *lhs, const TerrainDebris *rhs);
*/

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
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmaps
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets all the bitmaps of this collection of debris.
// Arguments:       None.
// Return value:    An array of pointers to BITMAPs. Ownership is NOT transferred.

    BITMAP ** GetBitmaps() { return m_aBitmaps; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmapCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current number of BITMAPs that describe all the differnt
//                  pieces of debris.
// Arguments:       None.
// Return value:    An int with the count.

    int GetBitmapCount() const { return m_BitmapCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies the debris to an SLTerrain as its read parameters specify.
// Arguments:       Pointer to the terrain to place the debris on. Ownership is NOT xferred!
// Return value:    None.

    void ApplyDebris(SLTerrain *pTerrain);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TerrainDebris, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    static Entity::ClassInfo m_sClass;

    // Shows where the bitmaps are
    ContentFile m_DebrisFile;
    // All the different bitmaps for each chunk of debris. Not owned.
    BITMAP **m_aBitmaps;
    // How many bitmaps we have loaded
    int m_BitmapCount;
    // The material of all this debris
    Material m_Material;
    // The target material in which this debris should only exist in
    Material m_TargetMaterial;
    // Whether to only place if the target material is exposed on the surface of the terrain.
    // If false, checking will continue to penetrate down into non-air materials to try to find the target material
    bool m_OnlyOnSurface;
    // Whether to only place a piece of this if we find a spot for it to fit completely buried int eh terrain
    bool m_OnlyBuried;
    // Minimum depth into the terrain contour. This can be negative for debris placed above ground
    int m_MinDepth;
    // Max depth into the terrain contour. This can be negative for debris placed above ground
    int m_MaxDepth;
    // Approximate Density count per meter
    float m_Density;

};

} // namespace RTE

#endif // File
