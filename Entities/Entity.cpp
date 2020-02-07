//////////////////////////////////////////////////////////////////////////////////////////
// File:            Entity.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the Entity class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "PresetMan.h"
#include "ConsoleMan.h"
#include "DataModule.h"

namespace RTE {

Entity::ClassInfo * Entity::ClassInfo::m_sClassHead = 0;

Entity::ClassInfo Entity::m_sClass("Entity");


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ClassInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ClassInfo Entity.

Entity::ClassInfo::ClassInfo(const std::string &name, ClassInfo *pParentInfo, void * (*fpAllocFunc)(), void (*fpDeallocFunc)(void *), Entity * (*fpNewFunc)(), int allocBlockCount):
    m_Name(name),
    m_pParentInfo(pParentInfo),
    m_fpAllocate(fpAllocFunc),
    m_fpDeallocate(fpDeallocFunc),
    m_fpNewInstance(fpNewFunc),
    m_NextClass(m_sClassHead)
{
    m_sClassHead = this;

    m_AllocatedPool.clear();
    m_PoolAllocBlockCount = allocBlockCount > 0 ? allocBlockCount : 10;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   GetClassNames
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the names of all ClassInfo:s in existence.

std::list<std::string> Entity::ClassInfo::GetClassNames()
{
    list<string> retList;
    for (const ClassInfo *itr = m_sClassHead; itr != 0; itr = itr->m_NextClass)
        retList.push_back(itr->GetName());
    return retList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo of a particular RTE class corresponding to a
//                  friendly-formatted string name.

const Entity::ClassInfo * Entity::ClassInfo::GetClass(const std::string &name)
{
    if (name == "" || name == "None")
        return 0;
    for (const ClassInfo *itr = m_sClassHead; itr != 0; itr = itr->m_NextClass)
        if (itr->GetName() == name)
            return itr;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   FillAllPools
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain number of newly allocated instances to all pools.

void Entity::ClassInfo::FillAllPools(int fillAmount)
{
    for (ClassInfo *itr = m_sClassHead; itr != 0; itr = itr->m_NextClass)
    {
        if (itr->IsConcrete())
            itr->FillPool(fillAmount);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FillPool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a certain number of newly allocated instances to this' pool.

void Entity::ClassInfo::FillPool(int fillAmount)
{
    // Default to the set block allocation size if fillAmount is 0
    if (fillAmount <= 0)
        fillAmount = m_PoolAllocBlockCount;

    // If concrete class, fill up the pool with pre-allocated memory blocks the size of the type
    if (m_fpAllocate && fillAmount > 0)
    {
        // As many as we're asked to make
        for (int i = 0; i < fillAmount; ++i)
            m_AllocatedPool.push_back(m_fpAllocate());
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Grabs from the pre-allocated pool, an available chunk of memory the
//                  exact size of the Entity this ClassInfo represents.

void * Entity::ClassInfo::GetPoolMemory()
{
    DAssert(IsConcrete(), "Trying to get pool memory of an abstract Entity class!");

    // If the pool is empty, then fill it up again with as many instances as we are set to
    if (m_AllocatedPool.empty())
        FillPool(m_PoolAllocBlockCount > 0 ? m_PoolAllocBlockCount : 10);

    // Get the instance in the top of the pool and pop it off
    void *pFoundMemory = m_AllocatedPool.back();
    m_AllocatedPool.pop_back();

    DAssert(pFoundMemory, "Could not find an available instance in the pool, even after increasing its size!");

    // Keep track of the number of instaces passed out
    m_InstancesInUse++;

    return pFoundMemory;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReturnPoolMemory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a raw chunk of memory back to the pre-allocated available pool.

int Entity::ClassInfo::ReturnPoolMemory(void *pReturnedMemory)
{
    if (!pReturnedMemory)
        return false;

    m_AllocatedPool.push_back(pReturnedMemory);

    // Keep track of the number of instaces passed in
    m_InstancesInUse--;

    return m_InstancesInUse;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   DumpPoolMemoryInfo
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes a bunch of useful debug info about the memory pools to a file.

void Entity::ClassInfo::DumpPoolMemoryInfo(Writer &fileWriter)
{
    for (ClassInfo *itr = m_sClassHead; itr != 0; itr = itr->m_NextClass)
    {
        if (itr->IsConcrete())
        {
            fileWriter << itr->GetName() << ": " << itr->m_InstancesInUse << "\n";
        }
    }
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   ReadInstance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads the class name from a Reader, and then does everything needed to
//                  create an instance of that class and returns that instance.

Entity * ReadInstance(Reader &reader)
{
    string readClassName;
    const Entity::ClassInfo *pClass = 0;
    Entity *pEntity = 0;
    reader >> readClassName;
    pClass = Entity::ClassInfo::GetClass(readClassName);
    if (pClass && pClass->IsConcrete())
        pEntity = pClass->NewInstance();
    else {
        DDTAbort("Reader stream suggests allocating an unallocatable type");
    }

    return pObject;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Entity, effectively
//                  resetting the members of this abstraction level only.

void Entity::Clear()
{
    m_PresetName = "None";
    m_IsOriginalPreset = false;
    m_DefinedInModule = -1;
    m_PresetDescription.clear();
    m_Groups.clear();
    m_LastGroupSearch.clear();
    m_LastGroupResult = false;

	m_RandomWeight = 1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Entity ready for use.

int Entity::Create()
{
    // Special "All" group that includes.. all
    m_Groups.push_back("All");

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an Entity to be identical to another, by deep copy.

int Entity::Create(const Entity &reference)
{
    m_PresetName = reference.m_PresetName;
    // Note how m_IsOriginalPreset is NOT assigned, automatically indicating that the copy is not an original Preset!
    m_DefinedInModule = reference.m_DefinedInModule;
    m_PresetDescription = reference.m_PresetDescription;

    for (list<string>::const_iterator itr = reference.m_Groups.begin(); itr != reference.m_Groups.end(); ++itr)
        m_Groups.push_back(*itr);

	m_RandomWeight = reference.m_RandomWeight;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.

int Entity::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "CopyOf")
    {
        string refName = reader.ReadPropValue();
        const Entity *pPreset = g_PresetMan.GetEntityPreset(GetClassName(), refName, reader.GetReadModuleID());
#ifndef WIN32
	if ( !pPreset )
	{
		extern char *fcase( const char *path );
		char *fixed = fcase( refName.c_str() );
		if ( fixed )
		{
			pPreset = g_PresetMan.GetEntityPreset(GetClassName(), fixed, reader.GetReadModuleID());
		}
	}
#endif
        if (pPreset)
            pPreset->Clone(this);
        // Couldn't find the preset to copy from? Then just read it in as an original
        else
        {
/* Just read as an original preset instead of failing
            char error[256];
            sprintf_s(error, "Referring to an instance ('%s') to copy from that hasn't been defined!", refName.c_str());
            reader.ReportError(error);
            return -1;
*/
            // Do report this error to the console though!
            g_ConsoleMan.PrintString("ERROR: Couldn't find the preset '" + refName + "' accessed in "  + reader.GetCurrentFilePath() + " at line " + reader.GetCurrentFileLineString());

            // Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
            int slashPos = refName.find_first_of('/');
            if (slashPos != string::npos)
                m_PresetName = refName.substr(slashPos + 1);
            else
                m_PresetName = refName;

            // Mark this so that the derived class knows it shuold be added to the PresetMan when it's done reading all properties.
            m_IsOriginalPreset = true;
            // Indicate where this was read from
            m_DefinedInModule = reader.GetReadModuleID();
        }
    }
    else if (propName == "PresetName" || propName == "InstanceName")
    {
        reader >> m_PresetName;

        // Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
        int slashPos = m_PresetName.find_first_of('/');
        if (slashPos != string::npos)
            m_PresetName = m_PresetName.substr(slashPos + 1);

        // Mark this so that the derived class knows it shuold be added to the PresetMan when it's done reading all properties.
        m_IsOriginalPreset = true;
        // Indicate where this was read from
        m_DefinedInModule = reader.GetReadModuleID();
    }
    else if (propName == "Description")
        reader >> m_PresetDescription;
	else if (propName == "RandomWeight")
	{
		reader >> m_RandomWeight;
		if (m_RandomWeight < 0)
			m_RandomWeight = 0;
		if (m_RandomWeight > 100)
			m_RandomWeight = 100;
	}
    else if (propName == "AddToGroup")
    {
        string newGroup;
        reader >> newGroup;
        AddToGroup(newGroup);
        // Do this in AddToGroup instead?
        g_PresetMan.RegisterGroup(newGroup, reader.GetReadModuleID());
    }
    else
    {
        // Search for a property name match failed!
    // TODO: write this out to some logfile
        return Serializable::ReadProperty(propName, reader);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Entity to an output stream for
//                  later recreation with Create(istream &stream);

int Entity::Save(Writer &writer) const
{
    Serializable::Save(writer);

    // Is an original preset definition
    if (m_IsOriginalPreset)
    {
        writer.NewProperty("PresetName");
        writer << m_PresetName;
    }
    // Only write out a copy reference if there is one
    else if (!m_PresetName.empty() && m_PresetName != "None")
    {
        writer.NewProperty("CopyOf");
        writer << GetModuleAndPresetName();
    }

    if (!m_PresetDescription.empty())
    {
        writer.NewProperty("Description");
        writer << m_PresetDescription;
    }

// TODO: Make proper save system that knows not to save redundant data!
/*
    for (list<string>::const_iterator itr = m_Groups.begin(); itr != m_Groups.end(); ++itr)
    {
        writer.NewProperty("AddToGroup");
        writer << *itr;
    }
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SavePresetCopy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only saves out a Preset reference of this to the stream. Is only
//                  applicable to objects that are not original presets and haven't been
//                  altered since they were copied from their original.

int Entity::SavePresetCopy(Writer &writer) const
{
    // Can only save out copies with this
    if (m_IsOriginalPreset)
    {
        DDTAbort("Tried to save out a pure Preset Copy Reference from an original Preset!");
        return -1;
    }

    writer.ObjectStart(GetClassName());
    writer.NewProperty("CopyOf");
    writer << GetModuleAndPresetName();
    writer.ObjectEnd();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Entity.

void Entity::Destroy(bool notInherited)
{
//    if (!notInherited)
//        Serializable::Destroy();

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleAndPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of this Entity's data Preset, preceded by the name of
//                  the Data Module it was defined in, separated with a '/'

string Entity::GetModuleAndPresetName() const
{
    if (m_DefinedInModule < 0)
        return GetPresetName();

    const DataModule *pModule = g_PresetMan.GetDataModule(m_DefinedInModule);
    if (!pModule)
        return GetPresetName();

    return pModule->GetFileName() + "/" + GetPresetName();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  MigrateToModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this an original Preset in a different module than it was before.
//                  It severs ties deeply to the old module it was saved in.

bool Entity::MigrateToModule(int whichModule)
{
    if (m_DefinedInModule == whichModule)
        return false;

    // This now a unique snowflake
    m_IsOriginalPreset = true;
    m_DefinedInModule = whichModule;
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is part of a specific group or not.

bool Entity::IsInGroup(const string &whichGroup)
{
    // Do quick check against last search try to see if we can answer without searching again
    if (!whichGroup.empty() && m_LastGroupSearch == whichGroup)
        return m_LastGroupResult;

    // Searched for Any or All yeilds ALL
    if (whichGroup == "Any" || whichGroup == "All")
        return true;

    // Searched for None returns false
    if (whichGroup == "None")
        return false;

    for (list<string>::const_iterator itr = m_Groups.begin(); itr != m_Groups.end(); ++itr)
    {
        if (whichGroup == *itr)
        {
            // Save the search result for quicker response next time
            m_LastGroupSearch = whichGroup;
            return m_LastGroupResult = true;
        }
    }

    // Save the search result for quicker response next time
    m_LastGroupSearch = whichGroup;
    return m_LastGroupResult = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Entity Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Entity from a Reader.

Reader & operator>>(Reader &reader, Entity &operand)
{
    // Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
    string objectFilePath = reader.GetCurrentFilePath();

    // Read the Entity from the file
    operand.Create(reader);

    // Try to add it to the PresetMan
    g_PresetMan.AddEntityPreset(&operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);

    return reader;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Entity Pointer Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Entity from a Reader.

Reader & operator>>(Reader &reader, Entity *operand)
{
    if (operand)
    {
        // Get this before reading Entity, since if it's the last one in its datafile, the stream will show the parent file instead
        string objectFilePath = reader.GetCurrentFilePath();

        // Read the Entity from the file
        operand->Create(reader);

        // Try to add it to the PresetMan
        g_PresetMan.AddEntityPreset(operand, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);
    }
    else
        reader.ReportError("Tried to read an .ini file into a null Entity pointer!");

    return reader;
}

} // namespace RTE
