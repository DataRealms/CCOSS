//--------------------------------------------------------------------------------------------------------------------
// PROPERTY BASE CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "Property.h"
#ifdef    _DEBUG
#include  "Property.inl"
#endif

#include  "File.h"
#include  "LevelFiles.h"

namespace RTE
{

//----------------------------------------------- STATIC MEMBERS -----------------------------------------------------

IMPLEMENT_ROOT_RTTI(Property);

//--------------------------------------------------------------------------------------------------------------------
// Property
//--------------------------------------------------------------------------------------------------------------------

// copy

Property *Property::CreateCopy(Property *pDest)
{
    Property *pCopy = pDest;
    if (!pCopy) return NULL;

    pCopy->m_strName    = this->m_strName;
    pCopy->m_u32Offset  = this->m_u32Offset;
    pCopy->m_u32Size    = this->m_u32Size;
    pCopy->m_strHelp    = this->m_strHelp;

    pCopy->m_boExpose   = this->m_boExpose;
    pCopy->m_boStream   = this->m_boStream;
    pCopy->m_boReadOnly = this->m_boReadOnly;
    return pCopy;
}

// streaming

bool Property::Write(const Persistent *pObj, CLevelFiles &Level) const
{
    return Level.GetDataFile().PutString(GetRealValue(pObj));
}


bool Property::Read(const Persistent *pObj, const StdString &strVal, CLevelFiles &Level) const
{
    return SetValue(pObj, strVal);
}


bool Property::Link(const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace
