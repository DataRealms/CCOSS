//--------------------------------------------------------------------------------------------------------------------
// LIST OF PROPERTIES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "PropList.h"
#ifdef    _DEBUG
#include  "PropList.inl"
#endif

#include  "File.h"
#include  "Persistent.h"
#include  "Properties.h"
#include  "Vector.h"
//#include  "VectorMatrix.h"

namespace RTE
{

//--------------------------------------------------------------------------------------------------------------------
// CONSTRUCTORS / DESTRUCTOR
//--------------------------------------------------------------------------------------------------------------------

PropList::~PropList()
{
    for(iterProp iter = m_Properties.begin(); iter != m_Properties.end(); + + iter)
    {
        // could use for_each
        Property *pProp = *iter;
        delete pProp;
    }
}

//--------------------------------------------------------------------------------------------------------------------
// MANAGEMENT
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- RegisterProp -------------------------------------------------------
// create new property according to Type
// in : prop type, name, variable's offset, flags, help
// out: new property
//--------------------------------------------------------------------------------------------------------------------

Property *PropList::RegisterProp(const Properties::PROPERTY_TYPE Type, const StdString &strName, const long u32Off, 
                                   const long u32Flags, const StdString &strHelp)
{
    Property *pProp;

    switch(Type)
    {
        case Properties::String   : pProp = new PropString  (strName, u32Off, sizeof(StdString)); break;
        case Properties::Float    : pProp = new PropFloat   (strName, u32Off, sizeof(float));     break;
        case Properties::Bool     : pProp = new PropBool    (strName, u32Off, sizeof(bool));      break;
        case Properties::Long     : pProp = new PropInt     (strName, u32Off, sizeof(long));      break;
        case Properties::Pointer  : pProp = new PropPointer (strName, u32Off, sizeof(void *));    break;
        case Properties::SP       : pProp = new PropSP      (strName, u32Off, sizeof(void *));    break;
        case Properties::Function : pProp = new PropFunction(strName, u32Off, sizeof(void *));    break;
        case Properties::Vect2D   : pProp = new PropVect2D  (strName, u32Off, sizeof(CVect2D));   break;
        case Properties::Vect3D   : pProp = new PropVect3D  (strName, u32Off, sizeof(CVect3D));   break;
        case Properties::Vect4D   : pProp = new PropVect4D  (strName, u32Off, sizeof(CVect4D));   break;
        default                   : pProp = NULL;
    }

    if (!pProp)
        return NULL;

    pProp->SetFlags(u32Flags);
    pProp->SetHelp (strHelp);

    AddProp(pProp);
    return pProp;
}

//----------------------------------------------- RegisterProp -------------------------------------------------------
// create new property according to strType
// in : type, name
// out: new property
// rem: this function is called when loading descriptions from a file, before mathing them with those in the exe;
// that's why offset is set to (-1)
//--------------------------------------------------------------------------------------------------------------------

Property *PropList::RegisterProp(const StdString &strType, const StdString &strName)
{
    Properties::PROPERTY_TYPE Type = Properties::Unknown;
    if (strType == "String")    Type = Properties::String;
    if (strType == "Float" )    Type = Properties::Float;
    if (strType == "Bool"  )    Type = Properties::Bool;
    if (strType == "Long"   )    Type = Properties::Long;
    if (strType == "Vect2D")    Type = Properties::Vect2D;
    if (strType == "Vect3D")    Type = Properties::Vect3D;
    if (strType == "Vect4D")    Type = Properties::Vect4D;
    if (strType == "Pointer"   )    Type = Properties::Pointer;
    if (strType == "SP"    )    Type = Properties::SP;
    if (strType == "Function"   )    Type = Properties::Function;

    if (Type == Properties::Unknown) return NULL;
    return RegisterProp(Type, strName, long(-1), Property::STREAM, "");
}

//--------------------------------------------------------------------------------------------------------------------
// STREAMING
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- Write --------------------------------------------------------------
// write properties in text file
// in : file
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool PropList::Write(IFile &File)
{
    Property *pProp = GetFirstProp();

    while(pProp)
    {
    if (pProp->IsStreamed())
    {
        File.PutString("  <prop name='");
        File.PutString(pProp->GetName() + "'");
        File.PutString(" type='");
        File.PutString(pProp->GetType() + "'/>\n");
    }
    pProp = GetNextProp();
    }

    return true;
}

//----------------------------------------------- WriteBin -----------------------------------------------------------
// write properties in binary file
// in : file
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool PropList::WriteBin(IFile &File)
{
    Property *pProp = GetFirstProp();

    while(pProp)
    {
    if (pProp->IsStreamed())
    {
        Persistent::WriteString2Bin(File, pProp->GetName());
        long u32Type = pProp->GetEnumType();
        File.Write(&u32Type, 4);
    }
    pProp = GetNextProp();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace
