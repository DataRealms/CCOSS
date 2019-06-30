//--------------------------------------------------------------------------------------------------------------------
// PROPERTIES CLASSES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPERTIES_INL_
#define   _PROPERTIES_INL_

#ifdef    _DEBUG
#define   INLINE
#else
#define   INLINE    inline
#endif

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// PropString
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropString::PropString(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropString::GetType() const
{
    return "String";
}

INLINE long PropString::GetEnumType() const
{
    return Properties::String;
}


INLINE StdString PropString::Get(const Persistent *pObj) const
{
    return pObj ? *(StdString *)((BYTE *)pObj + m_u32Offset) : "";
}

//--------------------------------------------------------------------------------------------------------------------
// PropFloat
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropFloat::PropFloat(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropFloat::GetType() const
{
    return "Float";
}

INLINE long PropFloat::GetEnumType() const
{
    return Properties::Float;
}

// value

INLINE const StdString PropFloat::Float2String(const void *pAddr)
{
    StdString strVal;
    strVal.Format("%g", *((float *)pAddr));
    return strVal;
}

INLINE const StdString PropFloat::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE const StdString PropFloat::GetRealValue(const Persistent *pObj) const
{
    return pObj ? Float2String((BYTE *)pObj + m_u32Offset) : "";
}


INLINE float PropFloat::Get(const Persistent *pObj) const
{
    return *(float *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropFloat::Set(const Persistent *pObj, const float fVal) const
{
    if (!pObj) return false;
    *(float *)((BYTE *)pObj + m_u32Offset) = fVal;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropBool
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropBool::PropBool(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropBool::GetType() const
{
    return "Bool";
}

INLINE long PropBool::GetEnumType() const
{
    return Properties::Bool;
}

// value

INLINE const StdString PropBool::Bool2String(const void *pAddr)
{
    return *((bool *)pAddr) ? "true" : "false";
}

INLINE const StdString PropBool::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE const StdString PropBool::GetRealValue(const Persistent *pObj) const
{
    return pObj ? Bool2String((BYTE *)pObj + m_u32Offset) : "";
}


INLINE bool PropBool::Get(const Persistent *pObj) const
{
    return *(bool *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropBool::Set(const Persistent *pObj, const bool boVal) const
{
    if (!pObj) return false;
    *(bool *)((BYTE *)pObj + m_u32Offset) = boVal;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropInt
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropInt::PropInt(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropInt::GetType() const
{
    return "Long";
}

INLINE long PropInt::GetEnumType() const
{
    return Properties::Long;
}

// value

INLINE const StdString PropInt::U32ToString(const void *pAddr)
{
    StdString strVal;
    strVal.Format("%u", *((long *)pAddr));
    return strVal;
}

INLINE const StdString PropInt::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE const StdString PropInt::GetRealValue(const Persistent *pObj) const
{
    return pObj ? U32ToString((BYTE *)pObj + m_u32Offset) : "";
}


INLINE long PropInt::Get(const Persistent *pObj) const
{
    return *(long *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropInt::Set(const Persistent *pObj, const long u32Val) const
{
    if (!pObj) return false;
    *(long *)((BYTE *)pObj + m_u32Offset) = u32Val;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVector
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropVector::PropVector(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropVector::GetType() const
{
    return "Vector";
}

INLINE long PropVector::GetEnumType() const
{
    return Properties::Vector;
}

// value

INLINE const StdString PropVector::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}


INLINE Vector PropVector::Get(const Persistent *pObj) const
{
    return *(Vector *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropVector::Set(const Persistent *pObj, const Vector &v2Val) const
{
    if (!pObj)
        return false;

    *(Vector *)((BYTE *)pObj + m_u32Offset) = v2Val;
    return true;
}

/*
//--------------------------------------------------------------------------------------------------------------------
// PropVect2D
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropVect2D::PropVect2D(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropVect2D::GetType() const
{
    return "Vect2D";
}

INLINE long PropVect2D::GetEnumType() const
{
    return Properties::Vect2D;
}

// value

INLINE const StdString PropVect2D::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}


INLINE CVect2D PropVect2D::Get(const Persistent *pObj) const
{
    return *(CVect2D *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropVect2D::Set(const Persistent *pObj, const CVect2D &v2Val) const
{
    if (!pObj) return false;
    *(CVect2D *)((BYTE *)pObj + m_u32Offset) = v2Val;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect3D
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropVect3D::PropVect3D(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropVect3D::GetType() const
{
    return "Vect3D";
}

INLINE long PropVect3D::GetEnumType() const
{
    return Properties::Vect3D;
}

// value

INLINE const StdString PropVect3D::GetValue(const Persistent *pObj) const
{
// use this line to verify that "this" is a property of pObj, and include Persistent.h
// if (!pObj || !pObj->IsPropOfThis(this)) return "";
    return GetRealValue(pObj);
}


INLINE CVect3D PropVect3D::Get(const Persistent *pObj) const
{
    return *(CVect3D *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropVect3D::Set(const Persistent *pObj, const CVect3D &v3Val) const
{
    if (!pObj) return false;
    *(CVect3D *)((BYTE *)pObj + m_u32Offset) = v3Val;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect4D
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropVect4D::PropVect4D(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropVect4D::GetType() const
{
    return "Vect4D";
}

INLINE long PropVect4D::GetEnumType() const
{
    return Properties::Vect4D;
}

// value

INLINE const StdString PropVect4D::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}


INLINE CVect4D PropVect4D::Get(const Persistent *pObj) const
{
    return *(CVect4D *)((BYTE *)pObj + m_u32Offset);
}

INLINE bool PropVect4D::Set(const Persistent *pObj, const CVect4D &v4Val) const
{
    if (!pObj) return false;
    *(CVect4D *)((BYTE *)pObj + m_u32Offset) = v4Val;
    return true;
}
*/
//--------------------------------------------------------------------------------------------------------------------
// PropPointer
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropPointer::PropPointer(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropPointer::GetType() const
{
    return "Pointer";
}

INLINE long PropPointer::GetEnumType() const
{
    return Properties::Pointer;
}

// value

INLINE const StdString PropPointer::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE const StdString PropPointer::GetRealValue(const Persistent *pObj) const
{
    return pObj ? Ptr2String((BYTE *)pObj + m_u32Offset) : "";
}


INLINE void *PropPointer::Get(const Persistent *pObj) const
{
    long u32Val = *(long *)((BYTE *)pObj + m_u32Offset);
    return((void *)u32Val);
}

INLINE bool PropPointer::Set(const Persistent *pObj, const void *pPtr) const
{
    if (!pObj) return false;
    if (m_u32Offset == long(-1)) return false;
    *(long *)((BYTE *)pObj + m_u32Offset) = (long)pPtr;
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropSP
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropSP::PropSP(const StdString &strName, const long u32Offset, const long u32Size) : PropPointer(strName, u32Offset, u32Size)
{}

// get/set params

INLINE const StdString PropSP::GetType() const
{
    return "SP";
}

INLINE long PropSP::GetEnumType() const
{
    return Properties::SP;
}

//--------------------------------------------------------------------------------------------------------------------
// PropFunction
//--------------------------------------------------------------------------------------------------------------------

// constructors & destructor

INLINE PropFunction::PropFunction(const StdString &strName, const long u32Offset, const long u32Size) : Property(strName, u32Offset, u32Size)
{
    m_pfnGet   = NULL;
    m_pfnSet   = NULL;
    m_pfnWrite = NULL;
    m_pfnRead  = NULL;
    m_pfnLink  = NULL;
}

// get/set params

INLINE const StdString PropFunction::GetType() const
{
    return "Function";
}

INLINE long PropFunction::GetEnumType() const
{
    return Properties::Function;
}

// value

INLINE const StdString PropFunction::GetValue(const Persistent *pObj) const
{
    return GetRealValue(pObj);
}

INLINE bool PropFunction::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    return m_pfnSet ? (*m_pfnSet)(this, pObj, strVal) : false;
}

INLINE const StdString PropFunction::GetRealValue(const Persistent *pObj) const
{
    return m_pfnGet ? (*m_pfnGet)(this, pObj) : "";
}

// streaming

INLINE bool PropFunction::Write(const Persistent *pObj, CLevelFiles &Level) const
{
    return m_pfnWrite ? (*m_pfnWrite)(this, pObj, Level) : false;
}

INLINE bool PropFunction::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    return m_pfnWrite ? (*m_pfnWrite)(this, pObj, Level) : false;
}

INLINE bool PropFunction::Read(const Persistent *pObj, const StdString &strVal, CLevelFiles &Level) const
{
    return m_pfnRead ? (*m_pfnRead)(this, pObj, strVal, Level) : false;
}

INLINE bool PropFunction::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    return m_pfnRead ? (*m_pfnRead)(this, pObj, "", Level) : false;
}

INLINE bool PropFunction::Link(const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const
{
    return m_pfnLink ? (*m_pfnLink)(this, pObj, strVal, pPointed) : false;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace

#endif // _PROPERTIES_INL_
