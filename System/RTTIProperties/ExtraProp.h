//--------------------------------------------------------------------------------------------------------------------
// PROPERTIES AS RTTI EXTRA DATA
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

#ifndef   _EXTRAPROP_H_
#define   _EXTRAPROP_H_

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "ExtraData.h"
#include  "PropList.h"

//----------------------------------------------- CLASSES ------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// ExtraProp
//--------------------------------------------------------------------------------------------------------------------

namespace RTE
{

class ExtraProp : public ExtraData
{
    DECLARE_RTTI;

// typedefs
    public:

    typedef bool            (*pfnInitList)        (void);

// public methods
    public:

                              ExtraProp          (pfnInitList pfnInit) { if (pfnInit) (*pfnInit)(); }
    virtual                  ~ExtraProp          (void) {}

    PropList *               GetPropList         (void) { return &m_PropList; }

// protected data
    protected:

    PropList                 m_PropList;
};
}

//----------------------------------------------- MACROS -------------------------------------------------------------

// put this macro in the .h of the class that defines its properties
#define   DECLARE_PROPERTIES(ClassName, ExtraClass)                                              \
    public:                                                                                     \
    static ExtraClass  m_Prop;                                                                  \
    static PropList *GetPropList (void) { return m_Prop.GetPropList(); }                        \
    private:                                                                                    \
    static bool DefineProperties (void);

// put this macro in the .cpp of the class that defines its properties
#define  IMPLEMENT_PROPERTIES(ClassName, ExtraClass)  \
    ExtraClass ClassName::m_Prop(ClassName::DefineProperties);

// use this macro in DefineProperties
#define  REGISTER_PROP(Type, Class, Var, Name, Flags, Help)  \
    GetPropList()->RegisterProp(Properties::Type, Name, offsetof(Class, Var), Flags, Help);

//--------------------------------------------------------------------------------------------------------------------

#endif // _EXTRAPROP_H_
