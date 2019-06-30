//--------------------------------------------------------------------------------------------------------------------
// PROPERTY BASE CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPERTY_H_
#define   _PROPERTY_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "RTTI.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{
    class CLevelFiles;
    class Persistent;
}

//--------------------------------------------------------------------------------------------------------------------
// Property
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class Property
{
    DECLARE_ROOT_RTTI;



    public:

    enum
    {
        EXPOSE        = 1 << 0, 
        STREAM        = 1 << 1, 
        READONLY      = 1 << 2, 
    }
    FLAGS;

    enum
    {
        NONE          = 0, 
        BUTTON, 
        COLOR, 
    }
    CONTROL;

// public methods

    public:

// constructors & destructor

                              Property           (const StdString &strName, const long u32Offset, const long u32Size);
    virtual                  ~Property           (void);
    virtual Property *       CreateCopy          (Property *pDest=NULL);

// get/set params

    virtual const StdString  GetType             (void) const = 0;
    virtual long               GetEnumType         (void) const = 0;
    virtual const StdString &GetName             (void) const;
    virtual long               GetOffset           (void) const;
    virtual long               GetSize             (void) const;

    virtual const StdString &GetHelp             (void) const;
    virtual void              SetHelp             (const StdString &strHelp);

// value

    virtual const StdString  GetValue            (const Persistent * pObj) const;
    virtual bool              SetValue            (const Persistent * pObj, const StdString &strVal) const = 0;
    virtual const StdString  GetRealValue        (const Persistent * pObj) const = 0;

// flags

    bool                      IsExposed           (void) const;
    bool                      IsStreamed          (void) const;
    bool                      IsReadOnly          (void) const;
    void                      SetFlags            (const long u32Flags);

// control for editing

    long                       GetCtrlType         (void) const;
    void                      SetCtrlType         (const long u32Type);

// streaming

    virtual bool              Write               (const Persistent *pObj, CLevelFiles &Level) const;
    virtual bool              WriteBin            (const Persistent *pObj, CLevelFiles &Level) const = 0;
    virtual bool              Read                (const Persistent *pObj, const StdString &strVal, CLevelFiles &Level)    const;
    virtual bool              ReadBin             (const Persistent *pObj, CLevelFiles &Level) const = 0;
    virtual bool              Link                (const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const;

// protected data

    protected:

    StdString                m_strName;
    long                       m_u32Offset; // var offset in class
    long                       m_u32Size; // var size
    StdString                m_strHelp;

    struct
    {
        bool                  m_boExpose    : 1; // prop appears in grid
        bool                  m_boStream    : 1; // prop is loaded/saved
        bool                  m_boReadOnly  : 1; // prop can't be edited
    };

    long                       m_u32CtrlType;
};
}

//----------------------------------------------- MACROS -------------------------------------------------------------

#define PROPERTY_CREATECOPY(Name)                            \
    Property *CProp##Name::CreateCopy(Property *pDest)       \
    {                                                        \
    CProp##Name *pCopy = DYNAMIC_CAST(CProp##Name, pDest);    \
    if (!pCopy)                                               \
    {                                                      \
        pCopy = new CProp##Name("", 0, 0);                       \
        if (!pCopy) return NULL;                                \
    }                                                      \
                                                             \
    return Property::CreateCopy(pCopy);                     \
    }

//----------------------------------------------- INLINES ------------------------------------------------------------

#ifndef   _DEBUG
#include  "Property.inl"
#endif

//--------------------------------------------------------------------------------------------------------------------

#endif // _PROPERTY_H_
