//--------------------------------------------------------------------------------------------------------------------
// RTTI CLASS & MACROS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _RTTI_H_
#define   _RTTI_H_


//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "StdString.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{
    class ExtraData;
}

//--------------------------------------------------------------------------------------------------------------------
// RTTI
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class RTTI
{
// public methods

    public:
                              RTTI                (const StdString &strClassName, const RTTI *pBase, ExtraData *pExtra = NULL): m_strClassName(strClassName), m_pBaseRTTI(pBase), m_pExtraData(pExtra) {}
    virtual                  ~RTTI                (void)                      {}

    const StdString &            GetClassName        (void) const                { return m_strClassName; }
    const RTTI *               GetBaseRTTI         (void) const            { return m_pBaseRTTI;  }
          ExtraData *          GetExtraData        (void) const             { return m_pExtraData; }

// protected data

    protected:

    const StdString          m_strClassName; // name of the class containing this (static) object
    const RTTI *               m_pBaseRTTI; // pointer to the (static) object of the container's base class
          ExtraData *         m_pExtraData; // here we can store some additional infos (eg. properties)
};
}

//----------------------------------------------- MACROS -------------------------------------------------------------

// in non-root class .h
// put this macro in the declaration of a DERIVED class - it defines a static RTTI object for this class
// (be careful that after this macro, things are private)

#define   DECLARE_RTTI                                                                               \
    public:                                                                                            \
    static  const RTE::RTTI           m_RTTI;                                                      \
    virtual const RTE::RTTI *          GetRTTI   (void) const                  { return &m_RTTI; }  \
        private:

//in root class .h
// put this macro in the declaration of a BASE class
// (be careful that after this macro, things are private)

#define   DECLARE_ROOT_RTTI                                                                                                 \
    DECLARE_RTTI;                                                                                                           \
    public:                                                                                                                   \
    bool                      IsExactClass        (const RTE::RTTI *pRTTI)  { return(GetRTTI() == pRTTI); }             \
    bool                      IsKindOf            (const RTE::RTTI *pRTTI)  { const RTE::RTTI *pBase = GetRTTI();     \
                                                                                  while(pBase)                              \
                                                                                  {                                       \
                                                                                    if (pBase == pRTTI) return true;         \
                                                                                    pBase = pBase->GetBaseRTTI();           \
                                                                                  }                                       \
                                                                                  return false;                             \
                                                                              }                                           \
    void *                     DynamicCast         (const RTE::RTTI *pRTTI)    { return(IsKindOf(pRTTI) ? this : NULL); }  \
        private:

// in non-root class .cpp
// put one of these macros in the implementation file of a DERIVED class

#define IMPLEMENT_RTTI(ClassName, BaseClassName)                       \
    const RTE::RTTI ClassName::m_RTTI(#ClassName, &BaseClassName::m_RTTI);

#define IMPLEMENT_RTTI_PROP(ClassName, BaseClassName)                  \
    const RTE::RTTI ClassName::m_RTTI(#ClassName, &BaseClassName::m_RTTI, &m_Prop);

// in root class .cpp
// put one of these macros in the implementation file of a BASE class - they create a static RTTI member

#define IMPLEMENT_ROOT_RTTI(ClassName)                                \
    const RTE::RTTI ClassName::m_RTTI(#ClassName, NULL);

#define IMPLEMENT_ROOT_RTTI_PROP(ClassName)                           \
    const RTE::RTTI ClassName::m_RTTI(#ClassName, NULL, &m_Prop);

// runtime type macros

#define IS_EXACT_CLASS(ClassName, pObj)                                \
  (pObj ? pObj->IsExactClass(&ClassName::m_RTTI) : false)

#define IS_KIND_OF(ClassName, pObj)                                    \
  (pObj ? pObj->IsKindOf(&ClassName::m_RTTI) : false)

#define STATIC_CAST(ClassName, pObj)                                   \
  ((ClassName *)pObj)

#define DYNAMIC_CAST(ClassName, pObj)                                  \
  (pObj ? STATIC_CAST(ClassName, pObj->DynamicCast(&ClassName::m_RTTI)) : NULL)

//--------------------------------------------------------------------------------------------------------------------

#endif // _RTTI_H_
