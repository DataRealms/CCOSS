//--------------------------------------------------------------------------------------------------------------------
// REFERENCE COUNTING CLASS
// written by Frederic My
// fmy@fairyengine.com
// most of the time this class is going to be used with the SmartPointer template
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _REFCOUNTED_H_
#define   _REFCOUNTED_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "RTTI.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// RefCounted
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class RefCounted
{
    DECLARE_ROOT_RTTI;

// public methods
    public:

// counting
    void            AddRef              (void) { m_u32RefCount + +; }
    void            Release             (void) { m_u32RefCount--; if (!m_u32RefCount) delete this; }
    long            GetRefCount         (void) const { return m_u32RefCount; }

// protected methods
    protected:

// constructors & destructor
                    RefCounted           (void) : m_u32RefCount(0) {}
                    RefCounted           (const RefCounted &RefCount) : m_u32RefCount(0) {}
    virtual        ~RefCounted           (void) {}

// operators
    RefCounted &     operator =          (const RefCounted &RefCount) { return *this; }

// private data
        private:

        long m_u32RefCount;
};
}

#endif // _REFCOUNTED_H_
