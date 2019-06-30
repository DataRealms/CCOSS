//--------------------------------------------------------------------------------------------------------------------
// RTTI MAP CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _RTTIMAP_INL_
#define   _RTTIMAP_INL_

#ifdef    _DEBUG
#define   INLINE
#else
#define   INLINE    inline
#endif

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// CONSTRUCTORS / DESTRUCTOR
//--------------------------------------------------------------------------------------------------------------------

inline RTTIMap::RTTIMap()
{
}

inline RTTIMap::~RTTIMap()
{
}

//--------------------------------------------------------------------------------------------------------------------
// GET/SET
//--------------------------------------------------------------------------------------------------------------------

inline const RTTI *RTTIMap::GetRTTI(const StdString &strClass)
{
    iterStr2RTTI iter = m_map.find(strClass);
    if (iter == m_map.end()) return NULL;
    return (*iter).second;
}


inline void RTTIMap::AddRTTI(const StdString &strClass, const RTTI *pRTTI)
{
    if (!pRTTI) return;
    m_map[strClass] = pRTTI;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace

#endif // _RTTIMAP_INL_
