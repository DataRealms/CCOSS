//--------------------------------------------------------------------------------------------------------------------
// MAP OF LOADED PROPERTIES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "PropDescMap.h"
#ifdef    _DEBUG
#include  "PropDescMap.inl"
#endif

#include  "PropListLoad.h"

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// CONSTRUCTORS / DESTRUCTOR
//--------------------------------------------------------------------------------------------------------------------

PropDescMap::PropDescMap()
{}

PropDescMap::~PropDescMap()
{
    std::for_each(m_map.begin(), m_map.end(), DeleteListLoad);
}


void PropDescMap::DeleteListLoad(std::pair<StdString, PropListLoad *> Iter)
{
	PropListLoad *pLoad = Iter.second;
	delete pLoad;
}

//--------------------------------------------------------------------------------------------------------------------
// OPERATIONS
//--------------------------------------------------------------------------------------------------------------------

void PropDescMap::AddPropDesc(const StdString &strClass, PropListLoad *pListLoad)
{
    m_map[strClass] = pListLoad;
}

PropListLoad *PropDescMap::GetPropDesc(const StdString &strClass)
{
    iterStr2ListLoad iter = m_map.find(strClass);
    if (iter == m_map.end()) return NULL;
    return (*iter).second;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace
