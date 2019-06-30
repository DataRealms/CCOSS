//--------------------------------------------------------------------------------------------------------------------
// PERSISTENT OBJECT CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PERSISTENT_H_
#define   _PERSISTENT_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "ExtraProp.h"
#include  "RefCounted.h"
#include  "SmartPointer.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{
    class CLevelFiles;
    class PropListLoad;
}

//--------------------------------------------------------------------------------------------------------------------
// Persistent
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class Persistent : public RefCounted
{
    DECLARE_RTTI;
    DECLARE_PROPERTIES(Persistent, ExtraProp);

// public methods
    public:

// constructors & destructor
                              Persistent         (void) {}
                              Persistent         (const Persistent &Persistent): RefCounted(Persistent) {}
    virtual                  ~Persistent         (void) {}

// get/set
    virtual StdString         GetDefaultName        (void) const { return ""; }
    virtual StdString         GetEditName           (void) const { return ""; }


    virtual const StdString   &GetName              (void) const { return m_strName; }
    virtual void              SetName               (const StdString &strName) { m_strName = strName; }
    virtual const RTTI *      GetWriteRTTI          (void) const { return GetRTTI(); }

// streaming
    virtual bool              Write               (CLevelFiles &Level);

    virtual bool              ReadPropFile        (CLevelFiles &Level);
    virtual bool              ReadPropFileBin     (CLevelFiles &Level);
    virtual bool              ReadDataFile        (CLevelFiles &Level);
    virtual bool              ReadDataFileBin     (CLevelFiles &Level);

    virtual bool              LinkObjects         (CLevelFiles &Level);
    virtual bool              PostProcess         (CLevelFiles &Level);
    virtual bool              ReadValue           (CLevelFiles &Level, StdString *pstrVal);
    virtual bool              PostRead            (void);

// editing
    bool                      IsPropOfThis        (const Property *pProp) const;
    bool                      IsPropOfRTTI        (const Property *pProp, const RTTI *pRTTI) const;
    virtual bool              ModifyProp          (Property *pProp, const StdString &strNew) { return false; }

    virtual bool              IsPropExposed       (Property *pProp);
    virtual bool              IsPropReadOnly      (Property *pProp);

// helpers
    static bool               LookForTag          (IFile &File, char *pszLine, const long u32Size, const char *pszTag,  const char *pszStop="</xml>");
    static long               LookForTags         (IFile &File, char *pszLine, const long u32Size, StdString *pstrTags, const long u32NbTags);
    static StdString          GetTagParam         (const char *pszLine, const char *pszParam);

    static  bool              WriteString2Bin     (IFile &File, const StdString &strTxt);
    static  bool              ReadStringBin       (IFile &File, StdString *pstrTxt);

// protected methods
    protected:

// streaming
    virtual bool              WritePropDesc       (CLevelFiles &Level);
    virtual bool              WriteBaseDesc       (CLevelFiles &Level, const RTTI *pRTTI);
    virtual bool              WriteData           (CLevelFiles &Level);
    virtual bool              WriteBaseData       (CLevelFiles &Level, const RTTI *pRTTI);

    virtual bool              ReadProperties      (CLevelFiles &Level, PropListLoad *pListLoad);
    virtual bool              ReadPropertiesBin   (CLevelFiles &Level, PropListLoad *pListLoad);
    virtual bool              MatchProperties     (CLevelFiles &Level, PropListLoad *pListLoad);
    virtual void              RecursiveMatch      (PropListLoad *pListFile, PropListLoad *pListBase, const RTTI *pExeRTTI);
    virtual bool              MatchProperty       (PropListLoad *pDescFile, const RTTI *pRTTI,  Property *pPropLoad);
    virtual bool              InsertProperty      (PropListLoad *pDescFile, Property *pPropLoad, Property *pPropExe);

    virtual bool              ReadValues          (CLevelFiles &Level, PropListLoad *pPropDesc, Persistent *pObj);
    virtual bool              ReadValuesBin       (CLevelFiles &Level, PropListLoad *pPropDesc, Persistent *pObj);
    virtual bool              ReadUnmatched       (CLevelFiles &Level, Property *pProp, const StdString &strVal);
    virtual bool              ReadUnmatchedBin    (CLevelFiles &Level, Property *pProp);

// operators
    Persistent &             operator =          (const Persistent &Persistent) { return *this; }

// protected data
    protected:

    StdString                m_strName;
};

// smart pointer
    SMART_POINTER(Persistent);
}

//--------------------------------------------------------------------------------------------------------------------

#endif // _PERSISTENT_H_
