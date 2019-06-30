//--------------------------------------------------------------------------------------------------------------------
// RTTI MAP CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _RTTIMAP_H_
#define   _RTTIMAP_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "StdString.h"
#include  "StlIncludes.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{

class RTTI;
}

//--------------------------------------------------------------------------------------------------------------------
// RTTIMap
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class RTTIMap
{
// typedefs

    public:

    typedef std::map< StdString, const RTTI * >   mapStr2RTTI;
    typedef mapStr2RTTI::iterator                 iterStr2RTTI;

// public methods

    public:
                              RTTIMap            (void);
                             ~RTTIMap            (void);

// get/set

    const RTTI *              GetRTTI             (const StdString &strClass);
    void                      AddRTTI             (const StdString &strClass, const RTTI *pRTTI);

// protected data

    protected:

    mapStr2RTTI               m_map;
};

}

//----------------------------------------------- INLINES ------------------------------------------------------------

#include  "RTTIMap.inl"

//--------------------------------------------------------------------------------------------------------------------

#endif // _RTTIMAP_H_
