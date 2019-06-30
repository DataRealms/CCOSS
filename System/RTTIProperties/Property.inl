//--------------------------------------------------------------------------------------------------------------------
// PROPERTY BASE CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPERTY_INL_
#define   _PROPERTY_INL_

#ifdef    _DEBUG
#define   INLINE
#else
#define   INLINE    inline
#endif

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// Property
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE Property::Property(const StdString &strName, const long u32Offset, const long u32Size) :
                            m_strName(strName), m_u32Offset(u32Offset), m_u32Size(u32Size), m_u32CtrlType(NONE)
{}

INLINE Property::~Property()
{}

// get/set params

INLINE const StdString Property::GetType() const
{
    return "";
}

INLINE const StdString &Property::GetName() const
{
    return m_strName;
}

INLINE long Property::GetOffset() const
{
    return m_u32Offset;
}

INLINE long Property::GetSize() const
{
    return m_u32Size;
}

INLINE const StdString &Property::GetHelp() const
{
    return m_strHelp;
}

INLINE void Property::SetHelp(const StdString &strHelp)
{
    m_strHelp = strHelp;
}

// value

INLINE const StdString Property::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE bool Property::SetValue(const Persistent *Persistent, const StdString &strVal) const
{
    return false;
}

INLINE const StdString Property::GetRealValue(const Persistent *pObj) const
{
    return "";
}

// flags

INLINE bool Property::IsExposed() const
{
    return m_boExpose;
}

INLINE bool Property::IsStreamed() const
{
    return m_boStream;
}

INLINE bool Property::IsReadOnly() const
{
    return m_boReadOnly;
}

INLINE void Property::SetFlags(const long u32Flags)
{
    m_boExpose   = (u32Flags & EXPOSE)   != 0;
    m_boStream   = (u32Flags & STREAM)   != 0;
    m_boReadOnly = (u32Flags & READONLY) != 0;
}

// control for editing

INLINE long Property::GetCtrlType() const
{
    return m_u32CtrlType;
}

INLINE void Property::SetCtrlType(long u32Type)
{
    m_u32CtrlType = u32Type;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace

#endif // _PROPERTY_INL_
