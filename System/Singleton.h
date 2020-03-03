#ifndef _RTESINGLETON_
#define _RTESINGLETON_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Singleton.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Singleton base class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com



//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTEError.h"

namespace RTE
{



//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Singleton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Anything derived from this class will adhere to the Singleton pattern.
//                  To convert any class into a Singleton, do the three following steps:
//                  1. Publicly derive your class MyClass from Singleton<MyClass>.
//                  2. Make sure to instantiate MyClass once before using it.
//                  3. Call MyClass::Instance() to use the MyClass object from anywhere.
// Parent(s):       None.
// Class history:   12/24/2001 Singleton created.

template <typename Type>
class Singleton
{



//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:



//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Singleton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Singleton object.
// Arguments:       None.

    Singleton() { DAssert(!ms_Instance, "Trying to create a second instance of a Singleton");
                  // Take nonexistent object sitting at address 0x1 in memory,
                  // cast to both Singleton, and whatever is deriving, and with the
                  // difference calculate the singleton's instance pointer.
    			  uintptr_t offset = (uintptr_t)(Type *)1 - (uintptr_t)(Singleton<Type> *)(Type *)1;
                  ms_Instance = (Type *)((uintptr_t)this + offset); }



//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Singleton
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Singleton object before deletion.
// Arguments:       None.

    ~Singleton() { AAssert(ms_Instance, "Trying to destruct nonexistent Singleton instance");
                   ms_Instance = 0; }



//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   Preset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns this Singleton.
// Arguments:       None.
// Return value:    A reference to the sole instance of this Singleton.

    static Type & Instance() { DAssert(ms_Instance, "Trying to use Singleton before instantiation");
                               return *ms_Instance; }



//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

private:

    // Member variables
    static Type *ms_Instance;

};

template <typename Type> Type * Singleton<Type>::ms_Instance = 0;

} // namespace RTE

#endif // File
