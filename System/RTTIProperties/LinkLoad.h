//--------------------------------------------------------------------------------------------------------------------
// LINK CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _LINKLOAD_H_
#define   _LINKLOAD_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "RefCounted.h" // LinkLoad is smart-pointable
#include  "SmartPointer.h"
#include  "StdString.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{
    class Persistent;
    class Property;
}

//--------------------------------------------------------------------------------------------------------------------
// LinkLoad
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class LinkLoad : public RefCounted
{
// public methods
    public:

// constructors & destructor
                              LinkLoad           (const Persistent *pObj, const Property *pProp, const StdString &strID, const Persistent *pID) {}
    virtual                  ~LinkLoad           (void) {}

// get/set
    const Persistent *        GetObj              (void) const { return m_pObj; }
    const Property *          GetProp             (void) const { return m_pProp; }
    const StdString &         GetID               (void) const { return m_strID; }
    const Persistent *        GetParam            (void) const { return m_pID; }

// protected data
    protected:

    const Persistent *        m_pObj; // object containing a pointer
    const Property *          m_pProp; // pointer or sp property
    StdString                 m_strID; // ID of pointed to object (text)
    const Persistent *        m_pID; // ID of pointed to object (bianry)
};

// smart pointer
    SMART_POINTER(LinkLoad);
}

//--------------------------------------------------------------------------------------------------------------------

#endif // _LINKLOAD_H_
