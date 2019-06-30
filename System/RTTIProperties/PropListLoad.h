//--------------------------------------------------------------------------------------------------------------------
// LIST OF LOADED PROPERTIES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPLISTLOAD_H_
#define   _PROPLISTLOAD_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "PropList.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// PropListLoad
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropListLoad : public PropList
{
// public methods

    public:

// constructors & destructor

                              PropListLoad       (const StdString &strName, PropListLoad *pBase);
    virtual                  ~PropListLoad       (void);

// get

    PropListLoad *           GetBase             (void) const { return m_pBase; }
    const StdString &        GetName             (void) const { return m_strName; }
    PropList &               GetFlatMatch        (void) { return m_FlatMatch; }

// protected data

    protected:

    PropListLoad *           m_pBase; // PropList base
    StdString                m_strName; // class name

    PropList                 m_FlatMatch; // match with exe layout, containing all inherited properties
};
}

//--------------------------------------------------------------------------------------------------------------------

#endif // _PROPLISTLOAD_H_
