//--------------------------------------------------------------------------------------------------------------------
// MAP OF LOADED PROPERTIES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPDESCMAP_H_
#define   _PROPDESCMAP_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "StdString.h"
#include  "StlIncludes.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{
    class PropListLoad;
}

//--------------------------------------------------------------------------------------------------------------------
// PropDescMap
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropDescMap
{
// typedefs
    public:

    typedef std::map<StdString, PropListLoad *>   mapStr2ListLoad;
    typedef mapStr2ListLoad::iterator             iterStr2ListLoad;

// public methods
    public:

// constructors & destructor
                              PropDescMap        (void);
    virtual                  ~PropDescMap        (void);

// operations
    void                      AddPropDesc         (const StdString &strClass, PropListLoad *pListLoad);
    PropListLoad *            GetPropDesc         (const StdString &strClass);

// protected methods
    protected:

    static void               DeleteListLoad      (std::pair<StdString, PropListLoad *> Iter);

// protected data
    protected:

    mapStr2ListLoad           m_map;
};
}

//--------------------------------------------------------------------------------------------------------------------

#endif // _PROPDESCMAP_H_
