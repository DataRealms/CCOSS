//--------------------------------------------------------------------------------------------------------------------
// PERSISTENT OBJECT CLASS
// written by Frederic My
// fmy@fairyengine.com
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- INCLUDES -----------------------------------------------------------

#include  "Persistent.h"

#include  "LevelFiles.h"
#include  "Logger.h"
#include  "Properties.h"
#include  "PropListLoad.h"
#include  "RTTIMap.h"

namespace RTE
{

//----------------------------------------------- STATIC MEMBERS -----------------------------------------------------

IMPLEMENT_RTTI_PROP (Persistent, RefCounted);
IMPLEMENT_PROPERTIES(Persistent, ExtraProp);

//--------------------------------------------------------------------------------------------------------------------
// PROPERTIES
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::DefineProperties() // static
{
    REGISTER_PROP(String, Persistent, m_strName, "Name", Property::EXPOSE|Property::STREAM, "");
    return true;
}

//----------------------------------------------- IsPropOfThis -------------------------------------------------------
// check the given property belongs to the RTTI tree of "this"
// in : prop
// out: found in RTTI tree ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::IsPropOfThis(const Property *pProp) const
{
    const RTTI *pRTTI = GetRTTI();
    while(pRTTI)
    {
        if (IsPropOfRTTI(pProp, pRTTI))
            return true;
        pRTTI = pRTTI->GetBaseRTTI();
    }
    return false;
}

//----------------------------------------------- IsPropOfRTTI -------------------------------------------------------
// check the given property belongs to the given RTTI
// in : prop, RTTI
// out: found in RTTI ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::IsPropOfRTTI(const Property *pProp, const RTTI *pRTTI) const
{
    if (!pRTTI)
        return false;
    ExtraData *pData  = pRTTI->GetExtraData();
    ExtraProp *pExtra = DYNAMIC_CAST(ExtraProp, pData);
    if (!pExtra)
        return false;
    PropList * pProps = pExtra->GetPropList();
    if (!pProps)
        return false;
  
    Property *pPropThis = pProps->GetFirstProp();
    while(pPropThis)
    {
        if (pPropThis == pProp)
            return true;
        pPropThis = pProps->GetNextProp();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------------------------
// EDITING
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- IsPropExposed ------------------------------------------------------
// determine if a property is exposed
// in : property
// out: true==exposed
// rem: virtual function, aimed at being overriden
// the default behaviour is simply to ask the property
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::IsPropExposed(Property *pProp)
{
    return pProp->IsExposed();
}

//----------------------------------------------- IsPropReadOnly -----------------------------------------------------
// determine if a property is read only
// in : property
// out: true==read only
// rem: virtual function, aimed at being overriden
// the default behaviour is simply to ask the property
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::IsPropReadOnly(Property *pProp)
{
    return pProp->IsReadOnly();
}

//--------------------------------------------------------------------------------------------------------------------
// STREAMING : WRITE
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- WriteString2Bin ----------------------------------------------------
// write StdString to binary format file: length + chrs
// in : file, StdString
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::WriteString2Bin(IFile &File, const StdString &strTxt)
{
    long u32Len = strTxt.size();
    if (File.Write(&u32Len, 4) != 4) return false;
    if (!u32Len) return true;
    if (File.Write(strTxt.c_str(), u32Len) != u32Len) return false;
    return true;
}

//----------------------------------------------- Write --------------------------------------------------------------
// write THIS object to current level
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::Write(CLevelFiles &Level)
{
    if (Level.ObjIsSaved(this))
        return true;

    if (!WritePropDesc(Level))
        return false;
    if (!WriteData    (Level))
        return false;

    Level.AddSavedObj(this);

    // write pointed to objects
    bool boRes = true;

    while(true)
    {
        if (Level.ObjToSaveCount() == 0)
            break; // finished
        Persistent *pPointed = Level.PopObjToSave();
        if (!pPointed)
            continue;

        boRes &= pPointed->Write(Level);
    }

    return boRes;
}

//----------------------------------------------- WritePropDesc ------------------------------------------------------
// write description of properties
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::WritePropDesc(CLevelFiles &Level)
{
    const RTTI *pRTTI = GetWriteRTTI(); // avoid renderer specific classes
    if (!pRTTI)
        return false;
    return WriteBaseDesc(Level, pRTTI);
}

//----------------------------------------------- WriteBaseDesc ------------------------------------------------------
// recursive function writing the hierarchy of props desc.
// in : level, current RTTI
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::WriteBaseDesc(CLevelFiles &Level, const RTTI *pRTTI)
{
    if (!pRTTI)
        return true; // we've reached the end
    if (Level.DescIsSaved(pRTTI->GetClassName())) 
        return true; // already saved

    const RTTI *pBase = pRTTI->GetBaseRTTI();
    if (pBase) WriteBaseDesc(Level, pBase); // recurse

    File &File = Level.GetDescFile();

    // binary format
    if (Level.BinaryMode())
    {
        WriteString2Bin(File, pRTTI->GetClassName());
        if (pBase)
            WriteString2Bin(File, pBase->GetClassName());
        else
            WriteString2Bin(File, "");

        ExtraData *pData  = pRTTI->GetExtraData();
        ExtraProp *pExtra = DYNAMIC_CAST(ExtraProp, pData);
        if (pExtra)
        {
            PropList *pPropList = pExtra->GetPropList();
            pPropList->WriteBin(File); // "this" class prop desc
        }

        long u32EndMarker = 0;
        File.Write(&u32EndMarker, 4);
    }
    // text format
    else
    {
        File.PutString("<class name='");
        File.PutString(pRTTI->GetClassName() + "'"); // class name
      
        File.PutString(" base='");
        if (pBase)
            File.PutString(pBase->GetClassName()); // base class
        File.PutString("'>\n");

        ExtraData *pData  = pRTTI->GetExtraData();
        ExtraProp *pExtra = DYNAMIC_CAST(ExtraProp, pData);
        if (pExtra)
        {
            PropList *pPropList = pExtra->GetPropList();
            pPropList->Write(File); // "this" class prop desc
        }
      
        if (!File.PutString("</class>\n\n"))
            return false;
    }

    Level.AddSavedDesc(pRTTI->GetClassName());
    return true;
}

//----------------------------------------------- WriteData ----------------------------------------------------------
// write properties'data
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::WriteData(CLevelFiles &Level)
{
    const RTTI *pRTTI = GetWriteRTTI(); // avoid renderer specific classes
    if (!pRTTI)
        return false;

    File &File = Level.GetDataFile();
    bool  boRes = true;

    // binary format
    if (Level.BinaryMode())
    {
        boRes &= WriteString2Bin(File, pRTTI->GetClassName());
        const Persistent *pThis = this;
        boRes &= (File.Write(&pThis, 4) == 4);

        boRes &= WriteBaseData(Level, pRTTI);
    }
    // text mode
    else
    {
        File.PutString("<data class='");
        File.PutString(pRTTI->GetClassName() + "' id='"); // class name

        const Persistent *pThis = this;
        File.PutString(PropPointer::Ptr2String(&pThis) + "'>\n"); // object address (=ID) for linking
      
        bool boRes = WriteBaseData(Level, pRTTI);
      
        File.PutString("</data>\n\n");
    }

    return boRes;
}

//----------------------------------------------- WriteBaseData ------------------------------------------------------
// recursive function writing the hierarchy of props data
// in : level, current RTTI
// out: OK ?
// rem: traversal MUST be the same as in WriteBaseDesc
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::WriteBaseData(CLevelFiles &Level, const RTTI *pRTTI)
{
    if (!pRTTI)
        return true;
  
    const RTTI *pBase = pRTTI->GetBaseRTTI();
    if (pBase)
        WriteBaseData(Level, pBase); // recurse
  
    ExtraData *pData  = pRTTI->GetExtraData();
    ExtraProp *pExtra = DYNAMIC_CAST(ExtraProp, pData);
    if (pExtra)
    {
        PropList *pPropList = pExtra->GetPropList();
        Property *pProp     = pPropList->GetFirstProp();

        File &File = Level.GetDataFile();
            
        while(pProp)
        {
            if (pProp->IsStreamed())
            {
                // binary format
                if (Level.BinaryMode())
                {
                    pProp->WriteBin(this, Level);
                }
                // text format
                else
                {
                    File.PutString("  ");
                    pProp->Write(this, Level);      
                    File.PutString("\n");
                }
            }
            pProp = pPropList->GetNextProp();
        }
    }
  
    return true;
}

//--------------------------------------------------------------------------------------------------------------------
// STREAMING : READ
//--------------------------------------------------------------------------------------------------------------------

//----------------------------------------------- LookForTag ---------------------------------------------------------
// look for a given tag in a text file - static
// in : file, line to return, line buffer size, tag, stop marker
// out: found ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::LookForTag(IFile &File, char *pszLine, const long u32Size, const char *pszTag, const char *pszStop)
{
    while(File.GetString(pszLine, u32Size))
    {
        if (strstr(pszLine, pszTag))
            return true; // tag  found
        if (strstr(pszLine, pszStop))
            return false; // stop found (ex: </xml>)
    }

    return false; // EOF, tag not found
}

//----------------------------------------------- LookForTags --------------------------------------------------------
// look for several tags at a time in a text file - static
// in : file, line to return, line buffer size, tags array, nb tags in array
// out: tag number (-1 = none found)
//--------------------------------------------------------------------------------------------------------------------

long Persistent::LookForTags(IFile &File, char *pszLine, const long u32Size, StdString *pstrTags, const long u32NbTags)
{
    while(File.GetString(pszLine, u32Size))
    {
        for(long u32Tag = 0; u32Tag < u32NbTags; u32Tag + +)
        {
            if (strstr(pszLine, pstrTags[u32Tag]))
                return u32Tag;
        }
    }

    return long(-1);
}

//----------------------------------------------- GetTagParam --------------------------------------------------------
// find the value of a given parameter in a line of text - static
// in : line buffer, wanted parameter
// out: parameter value
//--------------------------------------------------------------------------------------------------------------------

StdString Persistent::GetTagParam(const char *pszLine, const char *pszParam)
{
    StdString strVal;
    StdString strParam(pszParam);
    strParam += "='";

    char *pszPos = strstr(pszLine, strParam.c_str());
    if (pszPos)
    {
        pszPos += strParam.GetLength();
        char *pszEnd = strchr(pszPos, '\'');
        if (pszEnd)
        {
            *pszEnd = 0;
            strVal  = pszPos;
            *pszEnd = '\'';
        }
    }

    return strVal;
}

//----------------------------------------------- ReadStringBin ------------------------------------------------------
// read StdString from binary format file: length + chrs
// in : file, &StdString
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadStringBin(IFile &File, StdString *pstrTxt)
{
    if (!pstrTxt)
        return false;
    *pstrTxt   = "";
    long u32Len = 0;
    if (File.Read(&u32Len, 4) != 4)
        return false;
    if (!u32Len)
        return true; // empty StdString

    char szTxt[1024];
    if (File.Read(szTxt, u32Len) != u32Len)
        return false;
    szTxt[u32Len] = 0;
    *pstrTxt = szTxt;
    return true;
}

//----------------------------------------------- ReadPropFile -------------------------------------------------------
// read properties descriptions file
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadPropFile(CLevelFiles &Level)
{
    StdString  strTags[2] = { "<class ", "</xml>" }; // optim: most used comes 1st
    long         u32Tag;
    const long   u32Size    = 1024;
    char szLine[u32Size];

    CLogger *pLogger = Level.GetLogger();
    File &  DescFile = Level.GetDescFile();

    if (LookForTag(DescFile, szLine, u32Size, "<xml>"))
    {
        while((u32Tag = LookForTags(DescFile, szLine, u32Size, strTags, 2)) != long(-1))
        {
            if (u32Tag == 1)
                return true; // </xml>
            if (u32Tag == 0)
            {
                // <class
                StdString strClass = GetTagParam(szLine, "name");
                StdString strBase  = GetTagParam(szLine, "base");
                if (pLogger) _DLOG2_((*pLogger), LPCTSTR("class: " + strClass + "  base: " + strBase), "props file");

                // look for base class
                PropListLoad *pListBase = NULL;
                if (!strBase.empty())
                {
                    pListBase = Level.GetPropDesc(strBase);
                    if (!pListBase)
                    {
                        if (pLogger)
                            _DWAR_((*pLogger), LPCTSTR("base class [" + strBase + "] not found"));
                        return false;
                    }
                }

                // look for class prop desc
                if (Level.GetPropDesc(strClass))
                {
                    if (pLogger)
                        _DWAR_((*pLogger), LPCTSTR("class [" + strClass + "] already exists"));
                    return false;
                }

                // create new prop desc & add to map
                PropListLoad *pListLoad = new PropListLoad(strClass, pListBase);
                if (!pListLoad)
                {
                    if (pLogger)
                        _DWAR_((*pLogger), "'new' failed");
                    return false;
                }
                Level.AddPropDesc(strClass, pListLoad);

                // load properties
                if (!ReadProperties(Level, pListLoad))
                {
                    if (pLogger)
                        _DWAR_((*pLogger), "'ReadProperties' failed");
                    return false;
                }
                if (!MatchProperties(Level, pListLoad))
                {
                    if (pLogger)
                        _DWAR_((*pLogger), "'MatchProperties' failed : class [" + strClass + "]");
                    return false;
                }
            }
        }
    }

    return false;
}

//----------------------------------------------- ReadPropFileBin ----------------------------------------------------
// read properties descriptions file
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadPropFileBin(CLevelFiles &Level)
{
    CLogger *pLogger = Level.GetLogger();
    File &  DescFile = Level.GetDescFile();
    StdString strClass, strBase;

    while(!DescFile.Eof())
    {
        if (!ReadStringBin(DescFile, &strClass))
            return true; // EOF
        if (!ReadStringBin(DescFile, &strBase))
            return false;
        if (pLogger)
            _DLOG2_((*pLogger), LPCTSTR("class: " + strClass + "  base: " + strBase), "props file");

        // look for base class
        PropListLoad *pListBase = NULL;
        if (!strBase.empty())
        {
            pListBase = Level.GetPropDesc(strBase);
            if (!pListBase)
            {
                if (pLogger)
                    _DWAR_((*pLogger), LPCTSTR("base class [" + strBase + "] not found"));
                return false;
            }
        }

        // look for class prop desc
        if (Level.GetPropDesc(strClass))
        {
            if (pLogger)
                _DWAR_((*pLogger), LPCTSTR("class [" + strClass + "] already exists"));
            return false;
        }

        // create new prop desc & add to map
        PropListLoad *pListLoad = new PropListLoad(strClass, pListBase);
        if (!pListLoad)
        {
            if (pLogger)
                _DWAR_((*pLogger), "'new' failed");
            return false;
        }
        Level.AddPropDesc(strClass, pListLoad);

        // load properties
        if (!ReadPropertiesBin(Level, pListLoad))
        {
            if (pLogger)
                _DWAR_((*pLogger), "'ReadPropertiesBin' failed");
            return false;
        }
        if (!MatchProperties(Level, pListLoad))
        {
            if (pLogger)
                _DWAR_((*pLogger), "'MatchProperties' failed : class [" + strClass + "]");
            return false;
        }
    }

    return true;
}

//----------------------------------------------- ReadProperties -----------------------------------------------------
// read properties of a class
// in : level, prop desc
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadProperties(CLevelFiles &Level, PropListLoad *pListLoad)
{
    StdString  strTags[2] = { "<prop ", "</class>" };
    long         u32Tag;
    const long   u32Size    = 1024;
    char szLine[u32Size];

    CLogger *pLogger = Level.GetLogger();
    File &  DescFile = Level.GetDescFile();

    while((u32Tag = LookForTags(DescFile, szLine, u32Size, strTags, 2)) != long(-1))
    {
        if (u32Tag == 1)
            return true; // </class>
        if (u32Tag == 0)
        {
            // <prop 
            StdString strVar  = GetTagParam(szLine, "name");
            StdString strType = GetTagParam(szLine, "type");
            if (pLogger)
                _DLOG2_((*pLogger), LPCTSTR("prop: " + strVar + "  type: " + strType), "props file");

            // create property
            if (!pListLoad->RegisterProp(strType, strVar))
            {
                if (pLogger)
                    _DWAR_((*pLogger), LPCTSTR("unknown property type [" + strType + "]"));
                return false;
            }
        }
    }

    return false;
}

//----------------------------------------------- ReadPropertiesBin --------------------------------------------------
// read properties of a class
// in : level, prop desc
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadPropertiesBin(CLevelFiles &Level, PropListLoad *pListLoad)
{
    CLogger *pLogger = Level.GetLogger();
    File &  DescFile = Level.GetDescFile();
    StdString strVar;

    while(!DescFile.Eof())
    {
        if (!ReadStringBin(DescFile, &strVar))
            return false;
        if (strVar.empty())
            return true; // end of list

        long u32Type;
        if (DescFile.Read(&u32Type, 4) != 4)
            return false;
        if (pLogger)
            _DLOG2_((*pLogger), LPCTSTR("prop: " + strVar + "  type: " + Properties::GetTypeName(u32Type)), "props file");

        // create property
        Properties::PROPERTY_TYPE  Type = (Properties::PROPERTY_TYPE)u32Type;
        if (!pListLoad->RegisterProp(Type, strVar, long(-1), Property::STREAM, ""))
        {
            if (pLogger)
                _DWAR_((*pLogger), LPCTSTR("unknown property type [" + Properties::GetTypeName(u32Type) + "]"));
            return false;
        }
    }

    return false;
}

//----------------------------------------------- MatchProperties ----------------------------------------------------
// make the link between file & exe properties
// in : level, file prop desc
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::MatchProperties(CLevelFiles &Level, PropListLoad *pListLoad)
{
    if (!pListLoad || !Level.GetRTTIMap())
        return false;

	// get RTTI & props infos
    const RTTI *pRTTI = Level.GetRTTIMap()->GetRTTI(pListLoad->GetName());
    if (!pRTTI)
        return false;

    RecursiveMatch(pListLoad, pListLoad, pRTTI);
    return true;
}

//----------------------------------------------- RecursiveMatch -----------------------------------------------------
// make the link between file & exe properties
// in : file prop desc, base prop desc, exe RTTI
// out:
//--------------------------------------------------------------------------------------------------------------------

void Persistent::RecursiveMatch(PropListLoad *pDescFile, PropListLoad *pDescBase, const RTTI *pExeRTTI)
{
	// we need to begin to parse the props with the base-most class :
	if (pDescBase->GetBase())
        RecursiveMatch(pDescFile, pDescBase->GetBase(), pExeRTTI);

    Property *pPropLoad  = pDescBase->GetFirstProp();
    long        u32NbProp  = 0;
    long        u32NbMatch = 0;

	while(pPropLoad)
	{
		const RTTI *pRTTI = pExeRTTI;
		
		while(pRTTI)
		{
			if (MatchProperty(pDescFile, pRTTI, pPropLoad))
			{
				u32NbMatch + +;
				break;
			}
			pRTTI = pRTTI->GetBaseRTTI();
		}
		
		if (!pRTTI)
		{
            // insert the prop as 'unmatched'
			InsertProperty(pDescFile, pPropLoad, NULL);
		}
		
		u32NbProp + +;
		pPropLoad = pDescBase->GetNextProp();
	}
}

//----------------------------------------------- MatchProperty ------------------------------------------------------
// look for given property in given RTTI
// in : file prop desc, RTTI, property
// out: found?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::MatchProperty(PropListLoad *pDescFile, const RTTI *pRTTI, Property *pPropLoad)
{
    if (!pRTTI)
        return false;
    ExtraData *pData    = pRTTI->GetExtraData();
    ExtraProp *pExtra   = DYNAMIC_CAST(ExtraProp, pData);
    if (!pExtra)
        return false;
    PropList * pDescExe = pExtra  ->GetPropList();
    Property * pPropExe = pDescExe->GetFirstProp();
  
    while(pPropExe)
    {
        if (pPropExe->IsStreamed())
        {
            if ((pPropExe->GetName() == pPropLoad->GetName()) && (pPropExe->GetEnumType() == pPropLoad->GetEnumType()))
            {
                // insert the prop as 'matched'
                return InsertProperty(pDescFile, pPropLoad, pPropExe);
            }
        }
        pPropExe = pDescExe->GetNextProp();
    }
  
    return false;
}

//----------------------------------------------- InsertProperty -----------------------------------------------------
// insert property in flat match
// in : file prop desc, file prop, exe prop
// out: OK?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::InsertProperty(PropListLoad *pDescFile, Property *pPropLoad, Property *pPropExe)
{
    Property *  pProp = NULL;
    if (pPropExe)
        pProp = pPropExe ->CreateCopy();
    else
        pProp = pPropLoad->CreateCopy();

    if (!pProp)
        return false;

    pDescFile->GetFlatMatch().AddProp(pProp);
    return true;
}

//----------------------------------------------- ReadDataFile -------------------------------------------------------
// read data file
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadDataFile(CLevelFiles &Level)
{
    StdString  strTags[4] = { "<data ", "</xml>", "<engine_root ", "<editor_root " };
    long         u32Tag;
    const long   u32Size    = 1024;
    char szLine[u32Size];

    CLogger *pLogger = Level.GetLogger();
    File &  DataFile = Level.GetDataFile();

    if (LookForTag(DataFile, szLine, u32Size, "<xml>"))
    {
        while((u32Tag = LookForTags(DataFile, szLine, u32Size, strTags, 4)) != long(-1))
        {
            if (u32Tag == 1)
                return true; // </xml>

            if (u32Tag == 0)
            {
                // <data
                StdString strClass = GetTagParam(szLine, "class");
                StdString strID    = GetTagParam(szLine, "id");
                if (pLogger)
                    _DLOG2_((*pLogger), LPCTSTR("class: " + strClass + "  id: " + strID), "data file");

                // look for class prop desc
                PropListLoad *pPropDesc = Level.GetPropDesc(strClass);
                if (!pPropDesc)
                {
                    if (pLogger)
                        _DWAR_((*pLogger), LPCTSTR("class [" + strClass + "] not found"));
                    return false;
                }

                // create object
                Persistent *pObj = Level.CreateObject(strClass, strID);
                if (!pObj)
                {
                    if (pLogger)
                        _DWAR_((*pLogger), LPCTSTR("error creating object of class [" + strClass + "]"));
                    return false;
                }

                if (!ReadValues(Level, pPropDesc, pObj))
                {
                    if (pLogger)
                        _DWAR_((*pLogger), LPCTSTR("error reading object of class [" + strClass + "]"));
                    return false;
                }

                // </data>
                if (!LookForTag(DataFile, szLine, u32Size, "</data>"))
                    return false;
            }
            else if (u32Tag == 2)
            {
                // <engine_root 
                StdString strObj = GetTagParam(szLine, "obj");
                StdString strID  = GetTagParam(szLine, "addr");
                if (pLogger)
                    _DLOG2_((*pLogger), LPCTSTR("engine root: " + strObj + "  id: " + strID), "data file");

                Level.SetEngineRootID(strID);
            }
            else if (u32Tag == 3)
            {
                // <editor_root 
                StdString strObj = GetTagParam(szLine, "obj");
                StdString strID  = GetTagParam(szLine, "addr");
                if (pLogger) _DLOG2_((*pLogger), LPCTSTR("editor root: " + strObj + "  id: " + strID), "data file");

                Level.SetEditorRootID(strID);
            }
        }
    }

    return false;
}

//----------------------------------------------- ReadDataFileBin ----------------------------------------------------
// read data file
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadDataFileBin(CLevelFiles &Level)
{
    CLogger *pLogger = Level.GetLogger();
    File &  DataFile = Level.GetDataFile();

    Persistent *pEngineRootAddr = NULL;
    Persistent *pEditorRootAddr = NULL;
    if (DataFile.Read(&pEngineRootAddr, 4) != 4)
        return false;
    if (DataFile.Read(&pEditorRootAddr, 4) != 4)
        return false;

    Level.SetEngineRootAddr(pEngineRootAddr);
    Level.SetEditorRootAddr(pEditorRootAddr);

    if (pLogger)
    {
        StdString strEngineRoot, strEditorRoot;
        strEngineRoot.Format("0x%x", (long)pEngineRootAddr);
        strEditorRoot.Format("0x%x", (long)pEditorRootAddr);
        _DLOG2_((*pLogger), LPCTSTR("engine root: " + strEngineRoot), "data file");
        _DLOG2_((*pLogger), LPCTSTR("editor root: " + strEditorRoot), "data file");
    }

    // objects
    StdString strClass, strID;

    while(!DataFile.Eof())
    {
        if (!ReadStringBin(DataFile, &strClass))
            return true; // EOF
        long u32ID = 0;
        if (DataFile.Read(&u32ID, 4) != 4)
            return false;
        strID.Format("0x%x", u32ID);
        if (pLogger)
            _DLOG2_((*pLogger), LPCTSTR("class: " + strClass + "  id: " + strID), "data file");

        // look for class prop desc
        PropListLoad *pPropDesc = Level.GetPropDesc(strClass);
        if (!pPropDesc)
        {
            if (pLogger)
                _DWAR_((*pLogger), LPCTSTR("class [" + strClass + "] not found"));
            return false;
        }

        // create object
        Persistent *pObj = Level.CreateObject(strClass, (Persistent *)u32ID);
        if (!pObj)
        {
            if (pLogger)
                _DWAR_((*pLogger), LPCTSTR("error creating object of class [" + strClass + "]"));
            return false;
        }

        if (!ReadValuesBin(Level, pPropDesc, pObj))
        {
            if (pLogger)
                _DWAR_((*pLogger), LPCTSTR("error reading object of class [" + strClass + "]"));
            return false;
        }
    }

    return true;
}

//----------------------------------------------- ReadValue ----------------------------------------------------------
// read one line and format value
// in : level, value pointer
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadValue(CLevelFiles &Level, StdString *pstrVal)
{
    if (!pstrVal)
        return false;

    const long    u32Size = 1024;
    char  szLine[u32Size];
	char *pcStart;

    if (!Level.GetDataFile().GetString(szLine, u32Size))
        return false; // EOF

    long u32Len = strlen(szLine);
    if (szLine[u32Len-1] == 10)
        szLine[u32Len-1] = 0; // remove carriage return

    pcStart = szLine;
    while(*pcStart == ' ')
        pcStart + +; // remove spaces at the beginning

    *pstrVal = pcStart;
    return true;
}

//----------------------------------------------- ReadValues ---------------------------------------------------------
// read data values
// in : level, prop desc, object to fill
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadValues(CLevelFiles &Level, PropListLoad *pPropDesc, Persistent *pObj)
{
    if (!pPropDesc || !pObj) return false;
    StdString strVal;
	Property *pProp = pPropDesc->GetFlatMatch().GetFirstProp();
	
	// parse properties
    while(pProp)
    {
        if (!ReadValue(Level, &strVal))
            return false;

        // -1 means 'no match'
        if (pProp->GetOffset() != long(-1))
        {
            pProp->Read(pObj, strVal, Level);
        }
        else
        {
            pObj->ReadUnmatched(Level, pProp, strVal);
        }

        pProp = pPropDesc->GetFlatMatch().GetNextProp();
    }

    return true;
}

//----------------------------------------------- ReadValuesBin ------------------------------------------------------
// read data values
// in : level, prop desc, object to fill
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadValuesBin(CLevelFiles &Level, PropListLoad *pPropDesc, Persistent *pObj)
{
    if (!pPropDesc || !pObj) return false;
	Property *pProp = pPropDesc->GetFlatMatch().GetFirstProp();

	// parse properties
    while(pProp)
    {
        if (!pProp->ReadBin(pObj, Level))
            return false;

        // -1 means 'no match'
        if (pProp->GetOffset() == long(-1))
        {
            pObj->ReadUnmatchedBin(Level, pProp);
        }

        pProp = pPropDesc->GetFlatMatch().GetNextProp();
    }

    return true;
}

//----------------------------------------------- ReadUnmatched ------------------------------------------------------
// overridable function for unmatched props (ex: old format compatibility)
// in : level, property, value
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadUnmatched(CLevelFiles &Level, Property *pProp, const StdString &strVal)
{
    CLogger *pLogger = Level.GetLogger();
    if (pLogger)
        _DWAR_((*pLogger), "unmatched property [" + pProp->GetName() + "]");
    return true;
}

//----------------------------------------------- ReadUnmatchedBin ---------------------------------------------------
// overridable function for unmatched props (ex: old format compatibility)
// in : level, property
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::ReadUnmatchedBin(CLevelFiles &Level, Property *pProp)
{
    CLogger *pLogger = Level.GetLogger();
    if (pLogger)
        _DWAR_((*pLogger), "unmatched property [" + pProp->GetName() + "]");
    return true;
}

//----------------------------------------------- LinkObjects --------------------------------------------------------
// link loaded objects
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::LinkObjects(CLevelFiles &Level)
{
    return Level.LinkObjects();
}

//----------------------------------------------- PostRead -----------------------------------------------------------
// overridable function for special actions when loading is completed
// in :
// out: OK ?
// rem: called by PostProcess below
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::PostRead()
{
    return true;
}

//----------------------------------------------- PostProcess --------------------------------------------------------
// post-process loaded objects
// in : level
// out: OK ?
//--------------------------------------------------------------------------------------------------------------------

bool Persistent::PostProcess(CLevelFiles &Level)
{
    return Level.PostProcess();
}

//--------------------------------------------------------------------------------------------------------------------

} // namespace
