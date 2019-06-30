//--------------------------------------------------------------------------------------------------------------------
// LIST OF PROPERTIES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _PROPLIST_H_
#define   _PROPLIST_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "Properties.h"
#include  "StdString.h"
#include  "StlIncludes.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

namespace RTE
{

class IFile;
}

//--------------------------------------------------------------------------------------------------------------------
// PropList
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class PropList
{
// typedefs

    public:

    typedef std::list< Property * >               listProp;
    typedef listProp::iterator                    iterProp;

// public methods

    public:

// constructors & destructor

                              PropList           (void);
    virtual                  ~PropList           (void);

// access

    Property *                GetFirstProp        (void);
    Property *                GetNextProp         (void);

// management

    void                      AddProp             (Property *pProp);
    Property *                RegisterProp        (const Properties::PROPERTY_TYPE Type, const StdString &strName, const long u32Off, 
                                                   const long u32Flags, const StdString &strHelp);
    Property *                RegisterProp        (const StdString &strType, const StdString &strName);

// streaming

    virtual bool              Write               (IFile &File);
    virtual bool              WriteBin            (IFile &File);

// protected data

    protected:

    listProp                  m_Properties;
    iterProp                  m_CurrentProp;
};
}

//----------------------------------------------- INLINES ------------------------------------------------------------

#ifndef   _DEBUG
#include  "PropList.inl"
#endif

//--------------------------------------------------------------------------------------------------------------------

#endif // _PROPLIST_H_
