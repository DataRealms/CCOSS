//////////////////////////////////////////////////////////////////////////////////////////
// File:            DataModule.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the DataModule class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "DataModule.h"
#include "RTEManagers.h"
#include "Entity.h"
#include <map>

#include "allegro.h"

using namespace std;
//using namespace zip;

namespace RTE
{

const string DataModule::m_ClassName = "DataModule";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this DataModule, effectively
//                  resetting the members of this abstraction level only.

void DataModule::Clear()
{
    m_FileName.clear();
    m_FriendlyName.clear();
    m_Author.clear();
    m_Description.clear();
    m_Version = 1;
    m_ModuleID = -1;
    m_IconFile.Reset();
    m_pIcon = 0;
    m_PresetList.clear();
	m_EntityList.clear();
    m_TypeMap.clear();
    for (int i = 0; i < NUM_PALETTE_ENTRIES; ++i)
        m_MaterialMappings[i] = 0;
	m_ScanFolderContents = false;
	m_IgnoreMissingItems = false;
	m_CrabToHumanSpawnRatio = 0;
	m_ScriptPath.clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the DataModule entity ready for use.

int DataModule::Create()
{
    // Read all the properties
    if (Serializable::Create() < 0)
        return -1;

    // Load the icon
    if (!m_pIcon)
    {
        m_pIcon = m_IconFile.get
    }

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the DataModule entity ready for use.

int DataModule::Create(string moduleName, void (*fpProgressCallback)(std::string, bool))
{
    m_FileName = moduleName;
    m_ModuleID = g_PresetMan.GetModuleID(moduleName);
	m_CrabToHumanSpawnRatio = 0;

    // Report that we're starting to read a new DataModule
    if (fpProgressCallback)
    {
        char report[512];
        sprintf(report, "%s %c loading:", m_FileName.c_str(), -43);
        fpProgressCallback(string(report), true);
    }

    Reader reader;
    string indexPath(m_FileName + "/Index.ini");
	string mergedIndexPath(m_FileName + "/MergedIndex.ini");

	// Look for merged version of the index file, a single file where all the INI data was merged to load faster
	if (exists(mergedIndexPath.c_str()))
		indexPath = mergedIndexPath;

    if (exists(indexPath.c_str()) && reader.Create(indexPath.c_str(), true, fpProgressCallback) >= 0)
    {
        int result = Serializable::Create(reader);

        // Report loading result
        if (fpProgressCallback)
        {
// TODO, more info here?
/*
            char report[512];
            sprintf(report, "%s loading:", m_FileName.c_str());
            fpProgressCallback(string(report), true);
*/
            fpProgressCallback(string(" "), true);
        }

		// Scan folder contents and load everything *.ini from there
		if 	(m_ScanFolderContents)
		{
			al_ffblk fileInfo;
			std::string searchPath = m_FileName + "/" + "*.ini";

			for (int result = al_findfirst(searchPath.c_str(), &fileInfo, FA_ALL); result == 0; result = al_findnext(&fileInfo))
			{
				Reader iniReader;
				// Make sure we're not adding Index.ini again
				if (strlen(fileInfo.name) > 0 && string(fileInfo.name) != "Index.ini")
				{
					string iniPath(m_FileName + "/" + fileInfo.name);
					if (exists(iniPath.c_str()) && iniReader.Create(iniPath.c_str(), false, fpProgressCallback) >= 0)
					{
						int result = Serializable::Create(iniReader, false, true);

						// Report loading result
						if (fpProgressCallback)
						{
							fpProgressCallback(string(" "), true);
						}
					}
				}
			}
			// Close the file search to avoid memory leaks
			al_findclose(&fileInfo);
		}

        return result;
    }

    return -1;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadModuleProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Read module specific properties from index.ini without processing 
//					IncludeFiles and loading the whole module

int DataModule::ReadModuleProperties(string moduleName, void (*fpProgressCallback)(std::string, bool))
{
    m_FileName = moduleName;
    m_ModuleID = g_PresetMan.GetModuleID(moduleName);
	m_CrabToHumanSpawnRatio = 0;

    // Report that we're starting to read a new DataModule
    if (fpProgressCallback)
    {
        char report[512];
        sprintf(report, "%s %c reading properties:", m_FileName.c_str(), -43);
        fpProgressCallback(string(report), true);
    }

    Reader reader;
    string indexPath(m_FileName + "/Index.ini");

    if (exists(indexPath.c_str()) && reader.Create(indexPath.c_str(), true, fpProgressCallback) >= 0)
    {
		reader.SetSkipIncludes(true);
        int result = Serializable::Create(reader);
        return result;
    }

    return -1;
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a DataModule to be identical to another, by deep copy.

int DataModule::Create(const DataModule &reference)
{
    DDTAbort("Can't clone Data Modules!");

//    Serializable::Create(reference);

    m_FileName = reference.m_FileName;
    m_FriendlyName = reference.m_FriendlyName;
    m_Author = reference.m_Author;
    m_Description = reference.m_Description;
    m_Version = reference.m_Version;
    m_ModuleID = reference.m_ModuleID;
    m_IconFile = reference.m_IconFile;
    m_pIcon = reference.m_pIcon;
	m_ScanFolderContents = reference.m_ScanFolderContents;
	m_IgnoreMissingItems = reference.m_IgnoreMissingItems;
	m_CrabToHumanSpawnRatio = reference.m_CrabToHumanSpawnRatio;
	m_ScriptPath = reference.m_ScriptPath;

/* Redundant, won't ever clone ada datamodule
    for (map<string, map<string, Entity *> >::iterator it0a = reference.m_TypeMap.begin(); it0a != reference.m_TypeMap.end(); ++it0a)
    {
        m_TypeMap.insert(pair<string, map<string, Entity *> >(

        for (map<string, Entity *>::iterator it0b = (*it0a).second.begin(); it0b != (*it0a).second.end(); ++it0b)
        {
            delete (*it0b).second;
        }
        (*it0a).first.clear();
    }
*/
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the preset scripts of this object, from a specified path.

int DataModule::LoadScripts()
{
	if (m_ScriptPath != "")
	{
		return g_LuaMan.RunScriptFile(m_ScriptPath);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int DataModule::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "ModuleName")
        reader >> m_FriendlyName;
    else if (propName == "Author")
        reader >> m_Author;
    else if (propName == "Description")
        reader >> m_Description;
    else if (propName == "Version")
        reader >> m_Version;
    else if (propName == "ScanFolderContents")
        reader >> m_ScanFolderContents;
    else if (propName == "IgnoreMissingItems")
        reader >> m_IgnoreMissingItems;
    else if (propName == "CrabToHumanSpawnRatio")
        reader >> m_CrabToHumanSpawnRatio;
	else if (propName == "ScriptPath")
	{
		reader >> m_ScriptPath;
		LoadScripts();
	}
    else if (propName == "LoadFirst")
        // Do nothing; this is a special param looked for by Main.cpp
        reader.ReadPropValue();
    else if (propName == "Require")
	{
		// Check for required dependenceis if we're not load properties
		std::string requiredModule;
		reader >> requiredModule;

		if (!reader.GetSkipIncludes() && g_PresetMan.GetModuleID(requiredModule) == -1)
			reader.ReportError("\"" + m_FileName + "\" requires \"" + requiredModule + "\" in order to load!\n");
	}
    else if (propName == "IconFile")
    {
        // Load the bitmap as well
        reader >> m_IconFile;
        m_pIcon = m_IconFile.GetAsBitmap();
    }
    else if (propName == "AddMaterial")
        return g_SceneMan.ReadProperty(propName, reader);
    else
    {
        // Try to read in the preset and add it to the PresetMan in one go, and report if fail
        if (!g_PresetMan.GetEntityPreset(reader))
            reader.ReportError("Could not understand Preset type!");
    }
/* This seems inefficient and redundant
    else if (propName == "AddEffect" || propName == "AddSound" || propName == "AddIcon" || propName == "AddAmmo" || propName == "AddDevice" ||
             propName == "AddActor" || propName == "AddTerrainDebris" || propName == "AddTerrainObject" || propName == "AddEntity" ||
             propName == "AddBackgroundLayer" || propName == "AddTerrain" || propName == "AddScene" || propName == "AddActivity")
        g_PresetMan.GetEntityPreset(reader);
    else
        // See if the base class(es) can find a match instead
        return Serializable::ReadProperty(propName, reader);
*/
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this DataModule with a Writer for
//                  later recreation with Create(Reader &reader);

int DataModule::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("ModuleName");
    writer << m_FriendlyName;
    writer.NewProperty("Author");
    writer << m_Author;
    writer.NewProperty("Description");
    writer << m_Description;
    writer.NewProperty("Version");
    writer << m_Version;
    writer.NewProperty("IconFile");
    writer << m_IconFile;

// TODO: Write out all the different entity instances, each having their own relative location within the data module stored
// Will need the writer to be able to open different files and append to them as needed, probably done in NewEntity()
    // writer.NewEntity()

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the DataModule entity.

void DataModule::Destroy(bool notInherited)
{
    for (list<PresetEntry>::iterator itr = m_PresetList.begin(); itr != m_PresetList.end(); ++itr)
        delete (*itr).m_pEntityPreset;

//    if (!notInherited)
//        Serializable::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadAllScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads all scripted Entity Presets witht he latest version of their
//                  respective script files.

void DataModule::ReloadAllScripts()
{
    for (list<PresetEntry>::iterator itr = m_PresetList.begin(); itr != m_PresetList.end(); ++itr)
        (*itr).m_pEntityPreset->ReloadScripts();

	LoadScripts();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Entity instance's pointer and name associations to the
//                  internal list of already read in Entity:s. Ownership is NOT transferred!
//                  If there already is an instance defined, nothing happens. If there
//                  is not, a clone is made of the passed-in Entity and added to the library.

bool DataModule::AddEntityPreset(Entity *pEntToAdd, bool overwriteSame, string readFromFile)
{
    // Check if an unnamed instance is being added, and fail
    if (pEntToAdd->GetPresetName() == "None" || pEntToAdd->GetPresetName().empty())
        return false;

    // Check if the entity was even given a new name. If not, it's certainly in the containers already
//    // If we're overwriting though, don't do this - actually can't figure out why not!?
    if (!pEntToAdd->IsOriginalPreset())// && !overwriteSame)
        return false;

    bool added = false;

    // First check if there is an entity preset of the EXACT same type and name, and if so, if we should overwrite it
    Entity *pExistingEntity = GetEntityIfExactType(pEntToAdd->GetClassName(), pEntToAdd->GetPresetName());

    if (pExistingEntity)
    {
        // If we're commanded to overwrite any collisions, then do so by cloning over the existing instance in the list
        // This way we're not invalidating any instance references that would have been taken out and held by clients
        if (overwriteSame)
        {
            // Let the entity know which module it belongs to
            pEntToAdd->SetModuleID(m_ModuleID);
            // Clone the passed-in entity *TO* the instance already in the Data Module
            pEntToAdd->Clone(pExistingEntity);
            // Make sure the existing one is still marked as the Original Preset
            pExistingEntity->m_IsOriginalPreset = true;
            // Alter the instance entry to reflect the data file location of the new definition
            if (readFromFile != "Same")
            {
                list<PresetEntry>::iterator itr = m_PresetList.begin();
                for (; itr != m_PresetList.end(); ++itr)
                {
                    // When we find the correct entry, alter its data file location
                    if ((*itr).m_pEntityPreset == pExistingEntity)
                    {
                        (*itr).m_FileReadFrom = readFromFile;
                        break;
                    }
                }
                AAssert(itr != m_PresetList.end(), "Tried to alter allegedly existing Entity Preset Entry: " + pEntToAdd->GetPresetName() + ", but couldn't find it in the list!");
            }
            // Report success
            return true;
        }
        // No, hit a collision and aren't allowed to overwrite it; entity wasn't added
        else
            return false;
    }
    // Ok, no exact collision, so now check all the parent classes for collisions. If there are none, add the entity to them all.
    else
    {
/* Don't do this; multiple instances of the same name are allowed on the same type typelist, as long as there isn't more than one whose EXACT type is that of the typelist's typename
        // Walk up the class hierarchy till we reach the top, checking each level for whether there is an entity of the same or more general type, and same name
        for (const Entity::ClassInfo *pClass = &(pEntToAdd->GetClass()); pClass != 0; pClass = pClass->GetParent())
        {
            // If any collisions, then report that we couldn't add the entity and quit
            if (GetEntityIfExactType(pClass->GetName(), pEntToAdd->GetPresetName()))
                return false;
        }

        // Ok, doesn't seem to be any collisions, so just add the new entity
*/
        // Let the entity know which module it belongs to
        pEntToAdd->SetModuleID(m_ModuleID);
        // Make the copy which will become the preset instance
        Entity *pEntClone = pEntToAdd->Clone();
        // Mark the one we are about to add to the list as the Original now - this is now the acutal Original Preset instance
        pEntClone->m_IsOriginalPreset = true;

        // Insert clone into the list of entries where it will now be owned
        // New file location specified
		if (readFromFile != "Same")
		{
			m_PresetList.push_back(PresetEntry(pEntClone, readFromFile));
			m_EntityList.push_back(pEntClone);
		}
        // If same file specified for new instance, use the one in the last entry
		else if (!m_PresetList.empty())
		{
			m_PresetList.push_back(PresetEntry(pEntClone, m_PresetList.back().m_FileReadFrom));
			m_EntityList.push_back(pEntClone);
		}
        else
            DDTAbort("Tried to add first entity isntance to data module " + m_FileName + " without specifying a data file!");

        // Add it to the type map and quit
        added = AddToTypeMap(pEntClone);
        AAssert(added, "Unexpected problem while adding Entity instance \"" + pEntToAdd->GetPresetName() + "\" to the type map of data module: " + m_FileName);
    }

    return added;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a previously read in (defined) Entity, by exact type and instance
//                  name. Ownership is NOT transferred!

const Entity * DataModule::GetEntityPreset(string exactType, string instance)
{
    if (instance == "None" || instance.empty())
        return 0;

    map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(exactType);
    if (clsItr == m_TypeMap.end())
        return 0;

    // Find an instance of that EXACT type and name
    for (list<pair<string, Entity *> >::iterator instItr = (*clsItr).second.begin(); instItr != (*clsItr).second.end(); ++instItr)
    {
        // Check that the type is really the exact same as asked for; there may be others which are actually more derived types
        if ((*instItr).first == instance && (*instItr).second->GetClassName() == exactType)
            return (*instItr).second;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityDataLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the data file path of a previously read in (defined) Entity.

string DataModule::GetEntityDataLocation(std::string exactType, std::string instance)
{
    if (instance == "None" || instance.empty())
        return "";

    map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(exactType);
    if (clsItr == m_TypeMap.end())
        return "";

    Entity *pFoundEnt = 0;

    // Find an instance of that EXACT type and name
    for (list<pair<string, Entity *> >::iterator instItr = (*clsItr).second.begin(); instItr != (*clsItr).second.end(); ++instItr)
    {
        // Check that the type is really the exact same as asked for; there may be others which are actually more derived types
        if ((*instItr).first == instance && (*instItr).second->GetClassName() == exactType)
            pFoundEnt = (*instItr).second;
    }

    if (!pFoundEnt)
        return "";

    // Now find that entity in the isntanceList
    for (list<PresetEntry>::iterator itr = m_PresetList.begin(); itr != m_PresetList.end(); ++itr)
    {
        // When we find the correct entry, alter its data file location
        if ((*itr).m_pEntityPreset == pFoundEnt)
            return (*itr).m_FileReadFrom;
    }

    DDTAbort("Tried to find allegedly existing Entity Preset Entry: " + pFoundEnt->GetPresetName() + ", but couldn't!");
    return "";
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys, by inexact type.

bool DataModule::GetAllOfType(list<Entity *> &entityList, string type)
{
    if (type.empty())
        return false;

    // Look for the class entry
    map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(type);
    if (clsItr == m_TypeMap.end())
        return false;

    AAssert(!clsItr->second.empty(), "DataModule has class entry without instances in its map!?");

    // Found something, so let's add it to the list!
    for (list<pair<string, Entity *> >::iterator instItr = clsItr->second.begin(); instItr != clsItr->second.end(); ++instItr)
    {
        // Get the entitys, without transferring ownership
        entityList.push_back(instItr->second);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group.

bool DataModule::GetAllOfGroup(list<Entity *> &entityList, string group, string type)
{
    if (group.empty())
        return false;

    bool foundAny = false;

    // Look in all classes, so only look in the Entity level, which includes all!
    if (type.empty() || type == "All")
    {
        map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find("Entity");
        if (clsItr == m_TypeMap.end())
            return false;

        // Go through only the Entity typelist since it should contain all entitys in this DataModule
        for (list<pair<string, Entity *> >::iterator instItr = clsItr->second.begin(); instItr != clsItr->second.end(); ++instItr)
        {
            // Get the grouped entitys, without transferring ownership
            if (instItr->second->IsInGroup(group))
            {
                entityList.push_back(instItr->second);
                foundAny = true;
            }
        }
    }
    // Look only in one specific class (which will get all derived classes' entitys too!)
    else
    {
        // Look for the class entry
        map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(type);
        if (clsItr == m_TypeMap.end())
            return 0;

        AAssert(!clsItr->second.empty(), "DataModule has class entry without instances in its map!?");

        for (list<pair<string, Entity *> >::iterator instItr = clsItr->second.begin(); instItr != clsItr->second.end(); ++instItr)
        {
            // Get the grouped entitys, without transferring ownership
            if (instItr->second->IsInGroup(group))
            {
                entityList.push_back(instItr->second);
                foundAny = true;
            }
        }
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroupsWithType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills out a list with all groups registered with this that contain any
//                  entitys of a specific type and it derivatives.

bool DataModule::GetGroupsWithType(list<string> &groupList, string withType)
{
    bool foundAny = false;

    if (withType == "All" || withType.empty())
    {
        // Just give all the groups
        for (list<string>::const_iterator gItr = m_GroupRegister.begin(); gItr != m_GroupRegister.end(); ++gItr)
            groupList.push_back(*gItr);
    }
    // Go through all entitys, adding the groups the ones of the right type belong to
    else
    {
        // Find the type entry in the type map
        map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(withType);
        if (clsItr == m_TypeMap.end())
            return false;

        const list<string> *pGroupList = 0;
        // Go through all the entitys of that type, adding the groups they belong to
        for (list<pair<string, Entity *> >::iterator instItr = clsItr->second.begin(); instItr != clsItr->second.end(); ++instItr)
        {
            pGroupList = instItr->second->GetGroupList();
            // Get the grouped entitys, without transferring ownership
            for (list<string>::const_iterator gItr = pGroupList->begin(); gItr != pGroupList->end(); ++gItr)
            {
                groupList.push_back(*gItr);
                foundAny = true;
            }
        }

        // Make sure there are no dupe groups in the list
        groupList.sort();
        groupList.unique();
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Material mapping local to a DataModule. This is used for when
//                  multiple DataModule:s are loading conflicting Material:s, and need to
//                  resolve the conflicts by mapping their materials to ID's different than
//                  those specified in the data files.

bool DataModule::AddMaterialMapping(int fromID, int toID)
{
    AAssert(fromID > 0 && fromID < NUM_PALETTE_ENTRIES && toID > 0 && toID < NUM_PALETTE_ENTRIES, "Tried to make an out-of-bounds Material mapping");

    bool clear = m_MaterialMappings[fromID] == 0;
    m_MaterialMappings[fromID] = toID;

    return clear;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityIfExactType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if the type map has an instance added of a specific name and
//                  exact type. Does not check if any parent types with that name has been
//                  added. If found, that instance is returned, otherwise 0.

Entity * DataModule::GetEntityIfExactType(const std::string &exactType, const std::string &instanceName)
{
    // Check if null name
    if (exactType.empty() || instanceName == "None" || instanceName.empty())
        return 0;

    map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(exactType);
    // We didn't find any instances of this class, so report false
    if (clsItr == m_TypeMap.end())
        return 0;
    // We found that instances of the same type exist
    else
    {
        // See if there already is an instance of that class and name
        // Find an instance of that EXACT type and name
        for (list<pair<string, Entity *> >::iterator instItr = (*clsItr).second.begin(); instItr != (*clsItr).second.end(); ++instItr)
        {
            // Check that the type is really the exact same as asked for; there may be others which are actually more derived types
            if ((*instItr).first == instanceName && (*instItr).second->GetClassName() == exactType)
                return (*instItr).second;
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddToTypeMap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a newly added preset instance to the type map, where it will
//                  end up in every typelist of every class it derived from as well.
//                  I.e the "Entity" map will contain every single instance added to this.
//                  This will NOT check if duplicates are added to any type typelist, so
//                  please use GetEntityIfExactType to check this beforehand.

bool DataModule::AddToTypeMap(Entity *pEntToAdd)
{
    // Check if an unnamed instance is being added, and fail
    if (!pEntToAdd || pEntToAdd->GetPresetName() == "None" || pEntToAdd->GetPresetName().empty())
        return false;

    // Walk up the class hierarchy till we reach the top, adding an entry of the passed in entity into each typelist as we go along
    for (const Entity::ClassInfo *pClass = &(pEntToAdd->GetClass()); pClass != 0; pClass = pClass->GetParent())
    {
        // Look for the current class level in the map
        map<string, list<pair<string, Entity *> > >::iterator clsItr = m_TypeMap.find(pClass->GetName());

        // We didn't find any instances of this class already added, so add a new class category and add our new entry
        if (clsItr == m_TypeMap.end())
        {
            clsItr = (m_TypeMap.insert(pair<string, list<pair<string, Entity *> > >(pClass->GetName(), list<pair<string, Entity *> >()))).first;
            // Add into the new typelist, but isn't owned there
            (*clsItr).second.push_back(pair<string, Entity *>(pEntToAdd->GetPresetName(), pEntToAdd));
        }
        // We found that instances of the same class have been added before
        else
        {
            // Insert into the map, but isn't owned there - NOTE that we're not checking for collisions, they are assumed to have been checked already!
            (*clsItr).second.push_back(pair<string, Entity *>(pEntToAdd->GetPresetName(), pEntToAdd));
        }
    }

    // Signal that we successfully added the instance
    return true;
}

} // namespace RTE