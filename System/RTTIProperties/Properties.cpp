//--------------------------------------------------------------------------------------------------------------------
// PROPERTIES CLASSES
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "Properties.h"
#ifdef    _DEBUG
#include  "Properties.inl"
#endif

#include  "LevelFiles.h"
#include  "LinkLoad.h"
#include  "Persistent.h"

namespace RTE
{

//----------------------------------------------- STATIC MEMBERS -----------------------------------------------------

IMPLEMENT_RTTI(PropString, Property)
IMPLEMENT_RTTI(PropFloat, Property)
IMPLEMENT_RTTI(PropBool, Property)
IMPLEMENT_RTTI(PropInt, Property)
IMPLEMENT_RTTI(PropVect2D, Property)
IMPLEMENT_RTTI(PropVect3D, Property)
IMPLEMENT_RTTI(PropVect4D, Property)
IMPLEMENT_RTTI(PropPointer, Property)
IMPLEMENT_RTTI(PropSP, PropPointer)
IMPLEMENT_RTTI(PropFunction, Property)

//--------------------------------------------------------------------------------------------------------------------
// Properties
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- GetTypeName --------------------------------------------------------
// return type name - static
// in : type
// out: name
//--------------------------------------------------------------------------------------------------------------------

StdString Properties::GetTypeName(long u32Type)
{
    StdString strTmp("Unknown");
    if (u32Type == String) strTmp = "String";
    if (u32Type == Float ) strTmp = "Float";
    if (u32Type == Bool  ) strTmp = "Bool";
    if (u32Type == Long   ) strTmp = "Long";
    if (u32Type == Vect2D) strTmp = "Vect2D";
    if (u32Type == Vect3D) strTmp = "Vect3D";
    if (u32Type == Vect4D) strTmp = "Vect4D";
    if (u32Type == Pointer   ) strTmp = "Pointer";
    if (u32Type == SP    ) strTmp = "SP";
    if (u32Type == Function   ) strTmp = "Function";
    return strTmp;
}

//--------------------------------------------------------------------------------------------------------------------
// PropString
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(String)

// value
bool PropString::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;

    // convert "\n" to chrs 13 + 10
    StdString strTmp(strVal);
    char szCR[3] = { 13, 10, 0 };
    strTmp.Replace("\\n", szCR);

    *(StdString *)((BYTE *)pObj + m_u32Offset) = strTmp;
    return true;
}


const StdString PropString::GetRealValue(const Persistent *pObj) const
{
    if (!pObj)
        return "";

    // convert chrs 13 + 10 to "\n"
    StdString strVal(*(StdString *)((BYTE *)pObj + m_u32Offset));
    char szCR[3] = { 13, 10, 0 };
    strVal.Replace(szCR, "\\n");

    return strVal;
}

// streaming
bool PropString::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    return Persistent::WriteString2Bin(Level.GetDataFile(), GetRealValue(pObj));
}


bool PropString::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    StdString strVal;
    if (!Persistent::ReadStringBin(Level.GetDataFile(), &strVal))
        return false;

    if (GetOffset() != long(-1))
        return SetValue(pObj, strVal);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropFloat
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Float)

// value
bool PropFloat::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;

    float fVal = 0.f;
    if (sscanf(strVal, "%f", &fVal) != 1)
        return false;

    *(float *)((BYTE *)pObj + m_u32Offset) = fVal;
    return true;
}


bool PropFloat::String2Float(const StdString &strVal, float *pfVal) const
{
    if (!pfVal)
        return false;

    float f0;
    if (sscanf(strVal, "%f", &f0) != 1)
        return false;

    *pfVal = f0;
    return true;
}

// streaming
bool PropFloat::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    float fVal = Get(pObj);
    return(Level.GetDataFile().Write(&fVal, sizeof(float)) == sizeof(float));
}


bool PropFloat::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    float fVal;
    if (Level.GetDataFile().Read(&fVal, 4) != 4)
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, fVal);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropBool
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Bool)

// value
bool PropBool::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;

    if (stricmp(strVal.c_str(), "true") == 0)
    {
        *(bool *)((BYTE *)pObj + m_u32Offset) = true;
        return true;
    }

    if (stricmp(strVal.c_str(), "false") == 0)
    {
        *(bool *)((BYTE *)pObj + m_u32Offset) = false;
        return true;
    }

    return false;
}

// streaming
bool PropBool::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    bool boVal = Get(pObj);
    return(Level.GetDataFile().Write(&boVal, sizeof(bool)) == sizeof(bool));
}


bool PropBool::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    bool boVal;
    if (Level.GetDataFile().Read(&boVal, sizeof(bool)) != sizeof(bool))
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, boVal);
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropInt
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Long)

// value
bool PropInt::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;

    long u32Val = 0;
    if (sscanf(strVal, "%u", &u32Val) != 1)
        return false;

    *(long *)((BYTE *)pObj + m_u32Offset) = u32Val;
    return true;
}


bool PropInt::String2U32(const StdString &strVal, long *pu32Val) const
{
    if (!pu32Val)
        return false;
    long u32Val;
    if (sscanf(strVal, "%u", &u32Val) != 1)
        return false;

    *pu32Val = u32Val;
    return true;
}

// streaming
bool PropInt::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    long u32Val = Get(pObj);
    return(Level.GetDataFile().Write(&u32Val, sizeof(long)) == sizeof(long));
}


bool PropInt::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    long u32Val;
    if (Level.GetDataFile().Read(&u32Val, 4) != 4)
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, u32Val);
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVector
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Vector)

// value
const StdString PropVector::GetRealValue(const Persistent *pObj) const
{
    if (!pObj)
        return "";
    float *pAddr = (float *)((BYTE *)pObj + m_u32Offset);

    return PropFloat::Float2String(pAddr) + "; " + PropFloat::Float2String(pAddr + 1);
}


bool PropVector::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;
    float f0, f1;
    if (sscanf(strVal, "%f; %f", &f0, &f1) != 2)
        return false;

    float * pf = (float *)((BYTE *)pObj + m_u32Offset);
    *pf + + = f0;
    *pf + + = f1;
    return true;
}


bool PropVector::String2Floats(const StdString &strVal, float *pfVect) const
{
    if (!pfVect)
        return false;
    float f0, f1;
    if (sscanf(strVal, "%f; %f", &f0, &f1) != 2)
        return false;

    float * pf = pfVect;
    *pf + + = f0;
    *pf + + = f1;
    return true;
}

// streaming

bool PropVector::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    Vector v2Val = Get(pObj);
    return(Level.GetDataFile().Write(&v2Val.V, sizeof(Vector)) == sizeof(Vector));
}


bool PropVector::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    Vector v2Val;
    if (Level.GetDataFile().Read(&v2Val.V, sizeof(Vector)) != sizeof(CVector))
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, v2Val);
    return true;
}
/*
//--------------------------------------------------------------------------------------------------------------------
// PropVect2D
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Vect2D)

// value
const StdString PropVect2D::GetRealValue(const Persistent *pObj) const
{
    if (!pObj)
        return "";
    float *pAddr = (float *)((BYTE *)pObj + m_u32Offset);

    return PropFloat::Float2String(pAddr) + "; " + PropFloat::Float2String(pAddr + 1);
}


bool PropVect2D::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;
    float f0, f1;
    if (sscanf(strVal, "%f; %f", &f0, &f1) != 2)
        return false;

    float * pf = (float *)((BYTE *)pObj + m_u32Offset);
    *pf + + = f0;
    *pf + + = f1;
    return true;
}


bool PropVect2D::String2Floats(const StdString &strVal, float *pfVect) const
{
    if (!pfVect)
        return false;
    float f0, f1;
    if (sscanf(strVal, "%f; %f", &f0, &f1) != 2)
        return false;

    float * pf = pfVect;
    *pf + + = f0;
    *pf + + = f1;
    return true;
}

// streaming

bool PropVect2D::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect2D v2Val = Get(pObj);
    return(Level.GetDataFile().Write(&v2Val.V, sizeof(CVect2D)) == sizeof(CVect2D));
}


bool PropVect2D::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect2D v2Val;
    if (Level.GetDataFile().Read(&v2Val.V, sizeof(CVect2D)) != sizeof(CVect2D))
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, v2Val);
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect3D
//--------------------------------------------------------------------------------------------------------------------

// copy

PROPERTY_CREATECOPY(Vect3D)

// value

const StdString PropVect3D::GetRealValue(const Persistent *pObj) const
{
    if (!pObj)
        return "";
    float *pAddr = (float *)((BYTE *)pObj + m_u32Offset);

    return PropFloat::Float2String(pAddr) + "; " + PropFloat::Float2String(pAddr + 1) + "; " + PropFloat::Float2String(pAddr + 2);
}


bool PropVect3D::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;
    float f0, f1, f2;
    if (sscanf(strVal, "%f; %f; %f", &f0, &f1, &f2) != 3)
        return false;

    float * pf = (float *)((BYTE *)pObj + m_u32Offset);
    *pf + + = f0;
    *pf + + = f1;
    *pf + + = f2;
    return true;
}


bool PropVect3D::String2Floats(const StdString &strVal, float *pfVect) const
{
    if (!pfVect)
        return false;
    float f0, f1, f2;
    if (sscanf(strVal, "%f; %f; %f", &f0, &f1, &f2) != 3)
        return false;

    float * pf = pfVect;
    *pf + + = f0;
    *pf + + = f1;
    *pf + + = f2;
    return true;
}

// streaming

bool PropVect3D::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect3D v3Val = Get(pObj);
    return(Level.GetDataFile().Write(&v3Val.V, sizeof(CVect3D)) == sizeof(CVect3D));
}


bool PropVect3D::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect3D v3Val;
    if (Level.GetDataFile().Read(&v3Val.V, sizeof(CVect3D)) != sizeof(CVect3D))
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, v3Val);
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropVect4D
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Vect4D)

// value
const StdString PropVect4D::GetRealValue(const Persistent *pObj) const
{
    if (!pObj)
        return "";
    float *pAddr = (float *)((BYTE *)pObj + m_u32Offset);

    return PropFloat::Float2String(pAddr)   + "; " + PropFloat::Float2String(pAddr + 1) + "; " + PropFloat::Float2String(pAddr + 2) + "; " + PropFloat::Float2String(pAddr + 3);
}


bool PropVect4D::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;
    float f0, f1, f2, f3;
    if (sscanf(strVal, "%f; %f; %f; %f", &f0, &f1, &f2, &f3) != 4)
        return false;

    float * pf = (float *)((BYTE *)pObj + m_u32Offset);
    *pf + + = f0;
    *pf + + = f1;
    *pf + + = f2;
    *pf + + = f3;
    return true;
}


bool PropVect4D::String2Floats(const StdString &strVal, float *pfVect) const
{
    if (!pfVect)
        return false;
    float f0, f1, f2, f3;
    if (sscanf(strVal, "%f; %f; %f; %f", &f0, &f1, &f2, &f3) != 4)
        return false;

    float * pf = pfVect;
    *pf + + = f0;
    *pf + + = f1;
    *pf + + = f2;
    *pf + + = f3;
    return true;
}

// streaming
bool PropVect4D::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect4D v4Val = Get(pObj);
    return(Level.GetDataFile().Write(&v4Val.V, sizeof(CVect4D)) == sizeof(CVect4D));
}


bool PropVect4D::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    CVect4D v4Val;
    if (Level.GetDataFile().Read(&v4Val.V, sizeof(CVect4D)) != sizeof(CVect4D))
        return false;

    if (GetOffset() != long(-1))
        return Set(pObj, v4Val);
    return true;
}
*/
//--------------------------------------------------------------------------------------------------------------------
// PropPointer
//--------------------------------------------------------------------------------------------------------------------

// copy
PROPERTY_CREATECOPY(Pointer)

// value
const StdString PropPointer::Ptr2String(const void *pAddr)
{
    StdString strVal;
    strVal.Format("%x", *((long *)pAddr));
    strVal.ToUpper();
    return "0x" + strVal;
}


bool PropPointer::String2Ptr(const StdString &strVal, void** ppVoid) const
{
    if (!ppVoid)
        return false;
    long u32Val = 0;
    if (sscanf(strVal, "%x", &u32Val) != 1)
        return false;

    *ppVoid = (void *)u32Val;
    return true;
}


bool PropPointer::SetValue(const Persistent *pObj, const StdString &strVal) const
{
    if (!pObj)
        return false;
    long u32Val = 0;
    if (sscanf(strVal, "%x", &u32Val) != 1)
        return false;

    *(long *)((BYTE *)pObj + m_u32Offset) = u32Val;
    return true;
}

// streaming

bool PropPointer::Write(const Persistent *pObj, CLevelFiles &Level) const
{
    if (!pObj)
        return false;
    if (!Property::Write(pObj, Level))
        return false;

    Persistent *pPointed = (Persistent *)*(long *)((BYTE *)pObj + m_u32Offset);
    if (!pPointed)
        return true;

    Level.AddObjToSave(pPointed);
    return true;
}


bool PropPointer::WriteBin(const Persistent *pObj, CLevelFiles &Level) const
{
    if (!pObj)
        return false;
    Persistent *pPointed = (Persistent *)*(long *)((BYTE *)pObj + m_u32Offset);

    if (Level.GetDataFile().Write(&pPointed, 4) != 4)
        return false;
    if (!pPointed)
        return true;

    Level.AddObjToSave(pPointed);
    return true;
}


bool PropPointer::Read(const Persistent *pObj, const StdString &strVal, CLevelFiles &Level) const
{
    if (!pObj)
        return false;
    SetValue(pObj, "0x0"); // default = NULL

    if (strVal != "0x0")
    {
        LinkLoad *pLink = new LinkLoad(pObj, const_cast<PropPointer*>(this), strVal, NULL);
        if (!pLink)
            return false;
        Level.AddLink(pLink);
    }
    return true;
}


bool PropPointer::ReadBin(const Persistent *pObj, CLevelFiles &Level) const
{
    if (!pObj)
        return false;
    Set(pObj, NULL);

    Persistent *pPtr;
    if (Level.GetDataFile().Read(&pPtr, 4) != 4)
        return false;

    if ((GetOffset() != long(-1)) && pPtr)
    {
        LinkLoad *pLink = new LinkLoad(pObj, const_cast<PropPointer*>(this), "", pPtr);
        if (!pLink)
            return false;
        Level.AddLink(pLink);
    }
    return true;
}


bool PropPointer::Link(const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const
{
    if (!pObj)
        return false;
    if (pPointed)
        return Set(pObj, pPointed);
    return SetValue(pObj, strVal);
}

//--------------------------------------------------------------------------------------------------------------------
// PropSP
//--------------------------------------------------------------------------------------------------------------------

// copy
Property *PropSP::CreateCopy(Property *pDest)
{
    PropSP *pCopy = DYNAMIC_CAST(PropSP, pDest);
    if (!pCopy)
    {
        pCopy = new PropSP("", 0, 0);
        if (!pCopy)
            return NULL;
    }

    return PropPointer::CreateCopy(pCopy);
}

// streaming
bool PropSP::Link(const Persistent *pObj, const StdString &strVal, Persistent *pPointed) const
{
    if (!pObj || !pPointed)
        return false;
    if (!Set(pObj, pPointed))
        return false;

    // ref counter ++
    pPointed->AddRef();
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// PropFunction
//--------------------------------------------------------------------------------------------------------------------

// copy
Property *PropFunction::CreateCopy(Property *pDest)
{
    PropFunction *pCopy = DYNAMIC_CAST(PropFunction, pDest);
    if (!pCopy)
    {
        pCopy = new PropFunction("", 0, 0);
        if (!pCopy)
            return NULL;
    }

    pCopy->m_pfnGet   = this->m_pfnGet;
    pCopy->m_pfnSet   = this->m_pfnSet;
    pCopy->m_pfnWrite = this->m_pfnWrite;
    pCopy->m_pfnRead  = this->m_pfnRead;
    pCopy->m_pfnLink  = this->m_pfnLink;

    return Property::CreateCopy(pCopy);
}

// get/set params
void PropFunction::SetFunction(pfnGet pGet, pfnSet pSet, pfnWrite pWrite, pfnRead pRead, pfnLink pLink)
{
    m_pfnGet   = pGet;
    m_pfnSet   = pSet;
    m_pfnWrite = pWrite;
    m_pfnRead  = pRead;
    m_pfnLink  = pLink;
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace
