#ifndef _RTEENTITYMAN_
#define _RTEENTITYMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            EntityMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the EntityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <vector>
#include <map>
#include <string>

#include "DDTTools.h"
#include "Singleton.h"
#define g_EntityMan EntityMan::Instance()

//#include "Serializable.h"
#include "Entity.h"
//#include "FrameMan.h"
//#include "SceneMan.h"
//#include "Vector.h"
//#include "MOPixel.h"
//#include "AHuman.h"
//#include "MovableEntity.h"
//#include "LimbPath.h"
//#include "AtomGroup.h"

namespace RTE
{


class DataModule;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           EntityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager for allocating and doling out instances of Entities.
// Parent(s):       Singleton, Serializable.
// Class history:   06/03/2008 EntityMan created.

class EntityMan:
    public Singleton<EntityMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     EntityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a EntityMan entity in system
//                  memory. Create() should be called before using the entity.
// Arguments:       None.

    EntityMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~EntityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a EntityMan entity before deletion
//                  from system memory.
// Arguments:       None.

    ~EntityMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the EntityMan entity ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this EntityMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the EntityMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire EntityMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the EntityMan entity.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this entity.

    virtual const std::string & GetClassName() const { return m_ClassName; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AssignConcreteTypeID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Assigns a new Concrete Entity Type ID to whomever called this.
// Arguments:       The name of the new type to assign the ID to.
// Return value:    The ID assigned to the new concrete Entity type.

    int AssignConcreteTypeID(std::string typeName);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;

    // Names of all assigned concrete types, their IDs corresponding to the position in the vector
//    static std::vector<std::string> m_ConcreteTypeNames;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this EntityMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    EntityMan(const EntityMan &reference);
    EntityMan & operator=(const EntityMan &rhs);

};

} // namespace RTE

#endif // File