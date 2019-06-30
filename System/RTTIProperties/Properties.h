//--------------------------------------------------------------------------------------------------------------------
// PROPERTIES CLASSES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPERTIES_H_
#define   _PROPERTIES_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "Property.h"
#include  "Vector.h"
//#include  "VectorMatrix.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// PropString
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class Properties
{
// public enums
    public:

    typedef enum
    {
        String        = 1 << 0, 
        Float         = 1 << 1, 
        Bool          = 1 << 2, 
        Long          = 1 << 3, 
        Pointer       = 1 << 4, 
        SP            = 1 << 5, 
        Function           = 1 << 6, 
        Vector        = 1 << 7, 
//        Vect2D        = 1 << 7, 
//        Vect3D        = 1 << 8, 
//        Vect4D        = 1 << 9, 

        Unknown       = 0xffffffff
    }
    PROPERTY_TYPE;


    public:

    static StdString GetTypeName (long u32Type);
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropString
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropString : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropString         (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString    GetType             (void) const;
    virtual long             GetEnumType         (void) const;

// value
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString       GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    StdString                    Get                 (const Persistent *pObj) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropFloat
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropFloat : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropFloat          (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *        CreateCopy         (Property *pDest=NULL);

// get/set params
    virtual const StdString      GetType            (void) const;
    virtual long              GetEnumType         (void) const;

// value
    static  const StdString      Float2String        (const void *pAddr);
    virtual bool              String2Float        (const StdString &strVal, float *pfVal) const;

    virtual const StdString      GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString      GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    float                     Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const float fVal) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropBool
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropBool : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropBool           (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *        CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString      GetType             (void) const;
    virtual long              GetEnumType         (void) const;

// value
    static  const StdString      Bool2String         (const void *pAddr);

    virtual const StdString      GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString      GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    bool                      Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const bool boVal) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropInt
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropInt : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropInt            (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *        CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString      GetType             (void) const;
    virtual long              GetEnumType         (void) const;

// value
    static  const StdString      U32ToString         (const void *pAddr);
    virtual bool              String2U32          (const StdString &strVal, long *pu32Val) const;

    virtual const StdString      GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString      GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    long                      Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const long u32Val) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropVector
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropVector : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropVector         (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *        CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString      GetType             (void) const;
    virtual long              GetEnumType         (void) const;

// value
    virtual bool              String2Floats       (const StdString &strVal, float *pfVect) const;

    virtual const StdString      GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString      GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    Vector                    Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const Vector &v2Val) const;
};
}

/*
//--------------------------------------------------------------------------------------------------------------------
// PropVect2D
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropVect2D : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropVect2D         (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

// value
    virtual bool              String2Floats       (const StdString &strVal, float *pfVect) const;

    virtual const StdString  GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString  GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    CVect2D                   Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const CVect2D &v2Val) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect3D
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropVect3D : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropVect3D         (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

// value
    virtual bool              String2Floats       (const StdString &strVal, float *pfVect) const;

    virtual const StdString  GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString  GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    CVect3D                   Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const CVect3D &v3Val) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect4D
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropVect4D : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropVect4D         (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

// value
    virtual bool              String2Floats       (const StdString &strVal, float *pfVect) const;

    virtual const StdString  GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString  GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

// direct access
    CVect4D                   Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const CVect4D &v4Val) const;
};
}
*/
//--------------------------------------------------------------------------------------------------------------------
// PropPointer
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropPointer : public Property
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropPointer            (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

// value
    static  const StdString  Ptr2String          (const void *pAddr);
    virtual bool              String2Ptr          (const StdString &strVal, void** ppVoid) const;

    virtual const StdString  GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString  GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              Write               (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              Read                (const Persistent *pObj, const StdString &strVal, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              Link                (const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const;

// direct access
    void *                    Get                 (const Persistent *pObj) const;
    bool                      Set                 (const Persistent *pObj, const void *pPtr) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropSP
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropSP : public PropPointer
{
    DECLARE_RTTI;

// public methods
    public:

// constructors & destructor
                              PropSP             (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

// streaming
    virtual bool              Link                (const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const;
};
}

//--------------------------------------------------------------------------------------------------------------------
// PropFunction
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropFunction : public Property
{
    DECLARE_RTTI;

// typedefs
    public:

    typedef const StdString  (*pfnGet)           (const PropFunction *pProp, const Persistent *pObj);
    typedef bool              (*pfnSet)           (const PropFunction *pProp, const Persistent *pObj, const StdString &strVal);
    typedef bool              (*pfnWrite)         (const PropFunction *pProp, const Persistent *pObj, CLevelFiles &Level);
    typedef bool              (*pfnRead)          (const PropFunction *pProp, const Persistent *pObj, const StdString &strVal, CLevelFiles &Level);
    typedef bool              (*pfnLink)          (const PropFunction *pProp, const Persistent *pObj, const StdString &strVal, Persistent *pPointed);

// public methods
    public:

// constructors & destructor
                              PropFunction            (const StdString &strName, const long u32Offset, const long u32Size);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params
    virtual const StdString  GetType             (void) const;
    virtual long               GetEnumType         (void) const;

    void                      SetFunction              (pfnGet pGet, pfnSet pSet, pfnWrite pWrite, pfnRead pRead, pfnLink pLink);

// value
    virtual const StdString  GetValue            (const Persistent *pObj) const;
    virtual bool              SetValue            (const Persistent *pObj, const StdString &strVal) const;
    virtual const StdString  GetRealValue        (const Persistent *pObj) const;

// streaming
    virtual bool              Write               (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const;

    virtual bool              Read                (const Persistent *pObj, const StdString &strVal, CLevelFiles &Level) const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const;

    virtual bool              Link                (const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const;

// protected data
    protected:

    pfnGet                    m_pfnGet;
    pfnSet                    m_pfnSet;
    pfnWrite                  m_pfnWrite;
    pfnRead                   m_pfnRead;
    pfnLink                   m_pfnLink;
};
}

//----------------------------------------------- INLINES ------------------------------------------------------------

#ifndef   _DEBUG
#include  "Properties.inl"
#endif

//--------------------------------------------------------------------------------------------------------------------

#endif // _PROPERTIES_H_
