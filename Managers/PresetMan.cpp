//////////////////////////////////////////////////////////////////////////////////////////
// File:            PresetMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the PresetMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com

// Suppress compiler warning about unrecognized escape sequence on line 183
#pragma warning( disable : 4129 )


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "PresetMan.h"
#include "DataModule.h"
#include "SceneObject.h"
#include "Loadout.h"
#include "ACraft.h"
//#include "AHuman.h"
//#include "MOPixel.h"
//#include "SLTerrain.h"
//#include "AtomGroup.h"
//#include "Atom.h"

#include "ConsoleMan.h"
#include "LoadingScreen.h"
#include "SettingsMan.h"

namespace RTE {

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this PresetMan, effectively
//                  resetting the members of this abstraction level only.

void PresetMan::Clear()
{
    m_pDataModules.clear();
    m_DataModuleIDs.clear();
    m_OfficialModuleCount = 0;
    m_TotalGroupRegister.clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this PresetMan with a Writer for
//                  later recreation with Create(Reader &reader);

int PresetMan::Save(Writer &writer) const
{
    writer << m_Actors.size();
    for (list<Actor *>::const_iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        writer << **itr;

    writer << m_Particles.size();
    for (list<MovableEntity *>::const_iterator itr2 = m_Particles.begin(); itr2 != m_Particles.end(); ++itr2)
        writer << **itr2;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the PresetMan entity.

void PresetMan::Destroy()
{
    for (vector<DataModule *>::iterator dmItr = m_pDataModules.begin(); dmItr != m_pDataModules.end(); ++dmItr)
    {
        delete (*dmItr);
    }

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadDataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an entire data module and adds it to this. NOTE that official
//                  modules can't be loaded after any non-official ones!

bool PresetMan::LoadDataModule(string moduleName, bool official, ProgressCallback fpProgressCallback)
{
    if (moduleName.empty())
        return false;

    vector<DataModule *>::iterator itr;
    // Make a lowercase-version of the module name so it makes it easier to compare to and find case-agnostically
    string lowercaseName = moduleName;
    std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

    // Make sure we don't add the same module twice
    for (itr = m_pDataModules.begin(); itr != m_pDataModules.end(); ++itr)
    {
        if ((*itr)->GetFileName() == moduleName)
            return false;
    }

    // Only instantiate it here, because it needs to be in the lists of this before being created
    DataModule *pModule = new DataModule();

    // Official modules are stacked in the beginning of the vector
    if (official)
    {
        // Halt if an official module is being loaded after any non-official ones!
// We need to disable this because Metagames.rte gets loaded after non-official modules
//        RTEAssert(m_pDataModules.size() == m_OfficialModuleCount, "Trying to load an official module after a non-official one has been loaded!");

        // Find where the offical modules end in the vector
        itr = m_pDataModules.begin();
        int i = 0;
        for (; i < m_OfficialModuleCount; ++i)
            itr++;

        // Insert into after the last official one
        m_pDataModules.insert(itr, pModule);

        // Add the name to ID mapping
        // Adding the lowercase name version so we can more easily find with case-agnostic search
        m_DataModuleIDs.insert(pair<string, int>(lowercaseName, i));

        // Adjust offical tally
        m_OfficialModuleCount++;
    }
    // Non-official modules are just added the end
    else
    {
        m_pDataModules.push_back(pModule);

        // Add the name to ID mapping - note that official modules can't be loaded after any non-official ones!
        // Adding the lowercase name version so we can more easily find with case-agnostic search
		m_DataModuleIDs.insert(pair<string, size_t>(lowercaseName, m_pDataModules.size() - 1));
    }

    // Now actually create it
    if (pModule->Create(moduleName, fpProgressCallback) < 0)
    {
        RTEAbort("Failed to find the " + moduleName + " Data Module!");
        return false;
    }

    pModule = 0;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PresetMan::LoadAllDataModules() {
	auto moduleLoadTimerStart = std::chrono::high_resolution_clock::now();

	// Destroy any possible loaded modules
	Destroy();

	FindAndExtractZippedModules();

	// Load all the official modules first!
	std::array<std::string, 10> officialModules = { "Base.rte", "Coalition.rte", "Imperatus.rte", "Techion.rte", "Dummy.rte", "Ronin.rte", "Browncoats.rte", "Uzira.rte", "MuIlaak.rte", "Missions.rte" };
	for (const std::string &officialModule : officialModules) {
		if (!LoadDataModule(officialModule, true, &LoadingScreen::LoadingSplashProgressReport)) {
			return false;
		}
	}

	// If a single module is specified, skip loading all other unofficial modules and load specified module only.
	if (!m_SingleModuleToLoad.empty() && std::find(officialModules.begin(), officialModules.end(), m_SingleModuleToLoad) == officialModules.end()) {
		if (!LoadDataModule(m_SingleModuleToLoad, false, &LoadingScreen::LoadingSplashProgressReport)) {
			g_ConsoleMan.PrintString("ERROR: Failed to load DataModule \"" + m_SingleModuleToLoad + "\"! Only official modules were loaded!");
			return false;
		}
	} else {
		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory())) {
			std::string directoryEntryPath = directoryEntry.path().generic_string();
			if (std::filesystem::is_directory(directoryEntryPath) && std::regex_match(directoryEntryPath, std::regex(".*\.rte"))) {
				std::string moduleName = directoryEntryPath.substr(directoryEntryPath.find_last_of('/') + 1, std::string::npos);
				if (!g_SettingsMan.IsModDisabled(moduleName) && (std::find(officialModules.begin(), officialModules.end(), moduleName) == officialModules.end() && moduleName != "Metagames.rte" && moduleName != "Scenes.rte")) {
					int moduleID = GetModuleID(moduleName);
					// NOTE: LoadDataModule can return false (especially since it may try to load already loaded modules, which is okay) and shouldn't cause stop, so we can ignore its return value here.
					if (moduleID < 0 || moduleID >= GetOfficialModuleCount()) { LoadDataModule(moduleName, false, &LoadingScreen::LoadingSplashProgressReport); }
				}
			}
		}
		// Load scenes and MetaGames AFTER all other techs etc are loaded; might be referring to stuff in user mods.
		if (!LoadDataModule("Scenes.rte", false, &LoadingScreen::LoadingSplashProgressReport)) {
			return false;
		}
		if (!LoadDataModule("Metagames.rte", false, &LoadingScreen::LoadingSplashProgressReport)) {
			return false;
		}
	}

	if (g_SettingsMan.IsMeasuringModuleLoadTime()) {
		std::chrono::milliseconds moduleLoadElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - moduleLoadTimerStart);
		g_ConsoleMan.PrintString("Module load duration is: " + std::to_string(moduleLoadElapsedTime.count()) + "ms");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific loaded DataModule

const DataModule * PresetMan::GetDataModule(int whichModule)
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");
    return m_pDataModules[whichModule];
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModuleName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a name specific loaded DataModule

const std::string PresetMan::GetDataModuleName(int whichModule)
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");
    return m_pDataModules[whichModule]->GetFileName();
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of a loaded DataModule.

int PresetMan::GetModuleID(string moduleName)
{
    // Lower-case search name so we can match up against the already-lowercase names in m_DataModuleIDs
    std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);

    // First pass
    map<string, size_t>::iterator itr = m_DataModuleIDs.find(moduleName);
    if (itr != m_DataModuleIDs.end())
        return (*itr).second;

    // Try with or without the .rte on the end before giving up
    int dotPos = moduleName.find_last_of('.');
    // Wasnt, so try adding it
    if (dotPos == string::npos)
        moduleName = moduleName + System::GetModulePackageExtension();
    // There was ".rte", so try to shave it off the name
    else
        moduleName = moduleName.substr(0, dotPos);

    // Try to find the module again!
    itr = m_DataModuleIDs.find(moduleName);
    if (itr != m_DataModuleIDs.end())
        return (*itr).second;

/* No need to do this second pass now; we immediately do the case-agnostic search
	// Add .rte and try to find the module in case-agnostic fashion
	moduleName += ".rte";
	std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);

    itr = m_DataModuleIDs.find(moduleName);
    if (itr != m_DataModuleIDs.end())
        return (*itr).second;

    // Try with or without the .rte on the end before giving up
    dotPos = moduleName.find_last_of('.');
    // Wasnt, so try adding it
    if (dotPos == string::npos)
        moduleName = moduleName + System::GetModulePackageExtension();
    // There was ".rte", so try to shave it off the name
    else
        moduleName = moduleName.substr(0, dotPos);

    // Try to find the module again!
    itr = m_DataModuleIDs.find(moduleName);
    if (itr != m_DataModuleIDs.end())
        return (*itr).second;
*/
    return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleIDFromPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of a loaded DataModule, from a full data file path.

int PresetMan::GetModuleIDFromPath(std::string dataPath)
{
    if (dataPath.empty())
        return -1;

    int slashPos = dataPath.find_first_of('/');
    if (slashPos == string::npos)
        slashPos = dataPath.find_first_of('\\');

    return GetModuleID(dataPath.substr(0, slashPos));
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Entity instance's pointer and name associations to the
//                  internal list of already read in Entity:s. Ownership is NOT transferred!
//                  If there already is an instance defined, nothing happens. If there
//                  is not, a clone is made of the passed-in Entity and added to the library.

bool PresetMan::AddEntityPreset(Entity *pEntToAdd, int whichModule, bool overwriteSame, string readFromFile)
{
    RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    return m_pDataModules[whichModule]->AddEntityPreset(pEntToAdd, overwriteSame, readFromFile);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a previously read in (defined) Entity, by type and instance name.

const Entity * PresetMan::GetEntityPreset(string type, string preset, int whichModule)
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    const Entity *pRetEntity = 0;

    // Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
    int slashPos = preset.find_first_of('/');
    if (slashPos != string::npos)
    {
        // Get the module ID and cut off the module specifier in the string
        whichModule = GetModuleID(preset.substr(0, slashPos));
        preset = preset.substr(slashPos + 1);
    }

    // All modules
    if (whichModule < 0)
    {
        // Search all modules
        for (int i = 0; i < m_pDataModules.size() && !pRetEntity; ++i)
            pRetEntity = m_pDataModules[i]->GetEntityPreset(type, preset);
    }
    // Specific module
    else
    {
        // Try to get it from the asked for module
        pRetEntity = m_pDataModules[whichModule]->GetEntityPreset(type, preset);

        // If couldn't find it in there, then try all the official modules!
        if (!pRetEntity)
        {
            RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
            for (int i = 0; i < m_OfficialModuleCount && !pRetEntity; ++i)
            {
                pRetEntity = m_pDataModules[i]->GetEntityPreset(type, preset);
            }
        }
    }

    return pRetEntity;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an instance of an Entity that will be used as preset
//                  to later on be used to generate more instances of the same state.
//                  Will check if there was already one of the same class and instance
//                  name read in previously, and will return  that one if found, or
//                  add the newly read in one to this PresetMan's list if not found to
//                  exist there previously. Ownership is NOT transferred!

const Entity * PresetMan::GetEntityPreset(Reader &reader)
{
    // The reader is aware of which DataModule it is reading within
    int whichModule = reader.GetReadModuleID();
    RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Reader has an out of bounds module number!");

    string ClassName;
    const Entity::ClassInfo *pClass = 0;
    Entity *pNewInstance = 0;
    const Entity *pReturnPreset = 0;
    // Load class name and then preset instance
    reader >> ClassName;
    pClass = Entity::ClassInfo::GetClass(ClassName);

	if (pClass && pClass->IsConcrete())
	{
		// Instantiate
		pNewInstance = pClass->NewInstance();

		// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
		string entityFilePath = reader.GetCurrentFilePath();

		// Try to read in the preset instance's data from the reader
		if (pNewInstance && pNewInstance->Create(reader, false) < 0)
		{
			// Abort loading if we can't create entity and it's not in Scenes.rte
			if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems())
				RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
		}
		else if (pNewInstance)
		{
			// Try to add the instance to the collection
			m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);

			// Regardless of whether there was a collision or not, use whatever now exists in the instance map of that class and name
			pReturnPreset = m_pDataModules[whichModule]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
			// If the instance wasn't found in the specific DataModule, try to find it in all the official ones instead
			if (!pReturnPreset)
			{
				RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < m_OfficialModuleCount && !pReturnPreset; ++i)
					pReturnPreset = m_pDataModules[i]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
			}
		}
        // Get rid of the read-in instance as its copy is now either added to the map, or discarded as there already was somehting in there of the same name.
        delete pNewInstance; pNewInstance = 0;
    }
    else
        pReturnPreset = 0;

    return pReturnPreset;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadReflectedPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an preset of an Entity and tries to add it to the list of
//                  read-in presets. Regardless of whether there is a name collision,
//                  the read-in preset will be returned, ownership TRANSFERRED!

Entity * PresetMan::ReadReflectedPreset(Reader &reader)
{
    // The reader is aware of which DataModule it's reading within
    int whichModule = reader.GetReadModuleID();
    RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Reader has an out of bounds module number!");

    string ClassName;
    const Entity::ClassInfo *pClass = 0;
    Entity *pNewInstance = 0;
    // Load class name and then preset instance
    reader >> ClassName;
    pClass = Entity::ClassInfo::GetClass(ClassName);

    if (pClass && pClass->IsConcrete())
    {
        // Instantiate
        pNewInstance = pClass->NewInstance();

        // Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
        string entityFilePath = reader.GetCurrentFilePath();

        // Try to read in the preset instance's data from the reader
        if (pNewInstance && pNewInstance->Create(reader, false) < 0)
		{
			if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems())
	            RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
		}
		else
		{
			// Try to add the instance to the collection.
			// Note that we'll return this instance regardless of whether the adding was succesful or not
			m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);
		    return pNewInstance;
		}
    }

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys, by type.

bool PresetMan::GetAllOfType(list<Entity *> &entityList, string type, int whichModule)
{
    if (type.empty())
        return false;

    bool foundAny = false;

    // All modules
    if (whichModule < 0)
    {
        // Send the list to each module
        for (int i = 0; i < m_pDataModules.size(); ++i)
            foundAny = m_pDataModules[i]->GetAllOfType(entityList, type) || foundAny; 
    }
    // Specific module
    else
    {
        RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
        foundAny = m_pDataModules[whichModule]->GetAllOfType(entityList, type);
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfTypeInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  of a specific type, and only exist in a specific module space.

bool PresetMan::GetAllOfTypeInModuleSpace(std::list<Entity *> &entityList, std::string type, int whichModuleSpace)
{
    if (type.empty())
        return false;

    bool foundAny = false;

    // All modules
    if (whichModuleSpace < 0)
        foundAny = GetAllOfType(entityList, type, whichModuleSpace);
    // Specific module space
    else
    {
        // Get all entitys of the specific type in the official modules loaded before the specified one
        for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
            foundAny = GetAllOfType(entityList, type, module) || foundAny;

        // Now get the groups of the specified module (official or not)
        foundAny = GetAllOfType(entityList, type, whichModuleSpace) || foundAny;
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group.

bool PresetMan::GetAllOfGroup(list<Entity *> &entityList, string group, string type, int whichModule)
{
    RTEAssert(!group.empty(), "Looking for empty group!");

    bool foundAny = false;

    // All modules
    if (whichModule < 0)
    {
        // Get from all modules
        for (int i = 0; i < m_pDataModules.size(); ++i)
            // Send the list to each module, let them add
            foundAny = m_pDataModules[i]->GetAllOfGroup(entityList, group, type) || foundAny;
    }
    // Specific one
    else
    {
        RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
        foundAny = m_pDataModules[whichModule]->GetAllOfGroup(entityList, group, type);
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is randomly
//                  selected from a specific group.

Entity * PresetMan::GetRandomOfGroup(string group, string type, int whichModule)
{
    RTEAssert(!group.empty(), "Looking for empty group!");

    bool foundAny = false;
    // The total list we'll select a random one from
    list<Entity *> entityList;

    // All modules
    if (whichModule < 0)
    {
        // Get from all modules
        for (int i = 0; i < m_pDataModules.size(); ++i)
            // Send the list to each module, let them add
            foundAny = m_pDataModules[i]->GetAllOfGroup(entityList, group, type) || foundAny;
    }
    // Specific one
    else
    {
        RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
        foundAny = m_pDataModules[whichModule]->GetAllOfGroup(entityList, group, type);
    }

    // Didn't find any of that group in those module(s)
    if (!foundAny)
        return 0;

    // Pick one and return it
    int current = 0;
    int selection = RandomNum<int>(0, entityList.size() - 1);
    for (list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
    {
        if (current == selection)
            return (*itr);
        current++;
    }

    RTEAssert(0, "Tried selecting randomly but didn't?");
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroupFromTech
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is randomly
//                  selected from a specific group.

Entity * PresetMan::GetRandomBuyableOfGroupFromTech(string group, string type, int whichModule)
{
    RTEAssert(!group.empty(), "Looking for empty group!");

    bool foundAny = false;
    // The total list we'll select a random one from
    list<Entity *> entityList;
    list<Entity *> tempList;


    string techString = " Tech";
    string techName;
    string::size_type techPos = string::npos;

    // All modules
    if (whichModule < 0)
    {
        // Get from all modules
        for (int i = 0; i < m_pDataModules.size(); ++i)
		{
			// Select from tech-only modules
			techName = m_pDataModules[i]->GetFriendlyName();
			if (techName.find(techString) != string::npos)
				// Send the list to each module, let them add
				foundAny = m_pDataModules[i]->GetAllOfGroup(tempList, group, type) || foundAny;
		}
    }
    // Specific one
    else
    {
        RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
        foundAny = m_pDataModules[whichModule]->GetAllOfGroup(tempList, group, type);
    }

	//Filter found entities, we need only buyables
	if (foundAny)
	{
		//Do not filter anything if we're looking for brains
		if (group == "Brains")
		{
			foundAny = false;
			for (list<Entity *>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr)
			{
				entityList.push_back(*oItr);
				foundAny = true;
			}
		}
		else
		{
			foundAny = false;
			for (list<Entity *>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr)
			{
				SceneObject * pSObject = dynamic_cast<SceneObject *>(*oItr);
				// Buyable and not brain?
				if (pSObject && pSObject->IsBuyable() && !pSObject->IsInGroup("Brains"))
				{
					entityList.push_back(*oItr);
					foundAny = true;
				}
			}
		}
	}

	// Didn't find any of that group in those module(s)
    if (!foundAny)
        return 0;

    // Pick one and return it
    int current = 0;
    int selection = RandomNum<int>(0, entityList.size() - 1);

	int totalWeight = 0;
	for (list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
		totalWeight += (*itr)->GetRandomWeight();

	// Use random weights if looking in specific modules
	if (whichModule >= 0)
	{
		if (totalWeight == 0)
			return 0;

		selection = RandomNum(0, totalWeight - 1);

		for (list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
		{
			bool found = false;
			int bucketCounter = 0;

			if ((*itr)->GetRandomWeight() > 0)
			{
				while (bucketCounter < (*itr)->GetRandomWeight())
				{
					if (current == selection)
					{
						found = true;
						break;
					}

					current++;
					bucketCounter++;
				}
			}

			if (found)
				return (*itr);
		}
	}
	else 
	{
		for (list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
		{
			if (current == selection)
				return (*itr);

			current++;
		}
	}

    RTEAssert(0, "Tried selecting randomly but didn't?");
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroupInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group, and only exist in a specific module
//                  space.

bool PresetMan::GetAllOfGroupInModuleSpace(list<Entity *> &entityList, string group, string type, int whichModuleSpace)
{
    RTEAssert(!group.empty(), "Looking for empty group!");

    bool foundAny = false;

    // All modules
    if (whichModuleSpace < 0)
        foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
    // Specific module space
    else
    {
        // Get all entitys of the specific group the official modules loaded before the specified one
        for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
            foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;

        // Now get the groups of the specified module (official or not)
        foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroupInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is associated with
//                  a specific group, randomly selected and only exist in a specific module
//                  space.

Entity * PresetMan::GetRandomOfGroupInModuleSpace(string group, string type, int whichModuleSpace)
{
    RTEAssert(!group.empty(), "Looking for empty group!");

    bool foundAny = false;
    // The total list we'll select a random one from
    list<Entity *> entityList;

    // All modules
    if (whichModuleSpace < 0)
        foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
    // Specific module space
    else
    {
        // Get all entitys of the specific group the official modules loaded before the specified one
        for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
            foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;

        // Now get the groups of the specified module (official or not)
        foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
    }

    // Didn't find any of that group in those module(s)
    if (!foundAny)
        return 0;

    // Pick one and return it
    int current = 0;
    int selection = RandomNum<int>(0, entityList.size() - 1);
    for (list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
    {
        if (current == selection)
            return (*itr);
        current++;
    }

    RTEAssert(0, "Tried selecting randomly but didn't?");
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityDataLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:    Gets the data file path of a previously read in (defined) Entity.

string PresetMan::GetEntityDataLocation(string type, string preset, int whichModule)
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    string pRetPath = "";

    // All modules
    if (whichModule < 0)
    {
        // Search all modules
        for (int i = 0; i < m_pDataModules.size() && pRetPath.empty(); ++i)
            pRetPath = m_pDataModules[i]->GetEntityDataLocation(type, preset);
    }
    // Specific module
    else
    {
        // Try to get it from the asked for module
        pRetPath = m_pDataModules[whichModule]->GetEntityDataLocation(type, preset);

        // If couldn't find it in there, then try all the official modules!
        if (pRetPath.empty())
        {
            RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
            for (int i = 0; i < m_OfficialModuleCount && pRetPath.empty(); ++i)
            {
                pRetPath = m_pDataModules[i]->GetEntityDataLocation(type, preset);
            }
        }
    }

    return pRetPath;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadAllScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads all scripted Entity Presets witht he latest version of their
//                  respective script files.

void PresetMan::ReloadAllScripts()
{
	g_LuaMan.ClearUserModuleCache();

    // Go through all modules and reset all scripts in all their Presets
    for (int i = 0; i < m_pDataModules.size(); ++i)
        m_pDataModules[i]->ReloadAllScripts();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Material mapping local to a DataModule. This is used for when
//                  multiple DataModule:s are loading conflicting Material:s, and need to
//                  resolve the conflicts by mapping their materials to ID's different than
//                  those specified in the data files.

bool PresetMan::AddMaterialMapping(int fromID, int toID, int whichModule)
{
    RTEAssert(whichModule >= m_OfficialModuleCount && whichModule < m_pDataModules.size(), "Tried to make a material mapping in an offical or out-of-bounds DataModule!");

    return m_pDataModules[whichModule]->AddMaterialMapping(fromID, toID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers the existence of an Entity group, and in which module.

void PresetMan::RegisterGroup(std::string newGroup, int whichModule)
{
    RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    // Register in the handy total list
    m_TotalGroupRegister.push_back(newGroup);
    m_TotalGroupRegister.sort();
    m_TotalGroupRegister.unique();

    // Register in the specified module too
    m_pDataModules[whichModule]->RegisterGroup(newGroup);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroups
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of all groups registered in a specific module.

bool PresetMan::GetGroups(list<string> &groupList, int whichModule, string withType) const
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    bool foundAny = false;

    // Asked for ALL groups ever registered
    if (whichModule < 0)
    {
        // Get all applicable groups
        if (withType == "All" || withType.empty())
        {
            for (list<string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr)
                groupList.push_back(*gItr);

            foundAny = !m_TotalGroupRegister.empty();
        }
        // Filter out groups without any entitys of the specified type
        else
        {
            for (int module = 0; module < (int)m_pDataModules.size(); ++module)
                foundAny = m_pDataModules[module]->GetGroupsWithType(groupList, withType) || foundAny;
        }
    }
    // Asked for specific DataModule's groups
    else if (!m_pDataModules[whichModule]->GetGroupRegister()->empty())
    {
        // Get ALL groups of that module
        if (withType == "All" || withType.empty())
        {
            const list<string> *pGroupList = m_pDataModules[whichModule]->GetGroupRegister();
            for (list<string>::const_iterator gItr = pGroupList->begin(); gItr != pGroupList->end(); ++gItr)
                groupList.push_back(*gItr);

            foundAny = !pGroupList->empty();
        }
        // Get only modules that contain an entity of valid type 
        else
            foundAny = m_pDataModules[whichModule]->GetGroupsWithType(groupList, withType) || foundAny;
    }

    return foundAny;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleSpaceGroups
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills out a list with all groups registered in all official, PLUS a
//                  a specific non-official module as well.

bool PresetMan::GetModuleSpaceGroups(list<string> &groupList, int whichModule, string withType) const
{
    RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

    bool foundAny = false;

    // If all, then just copy the total register
    if (whichModule < 0)
    {
        // Just get all groups ever registered
        if (withType == "All" || withType.empty())
        {
            for (list<string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr)
                groupList.push_back(*gItr);

            foundAny = !m_TotalGroupRegister.empty();
        }
        // Get type filtered groups from ALL data modules
        else
        {
            for (int module = 0; module < (int)m_pDataModules.size(); ++module)
                foundAny = GetGroups(groupList, module, withType) || foundAny;            
        }
    }
    // Getting modulespace of specific module
    else
    {
        // Get all groups of the official modules that are loaded before the specified one
        for (int module = 0; module < m_OfficialModuleCount && module < whichModule; ++module)
            foundAny = GetGroups(groupList, module, withType) || foundAny;

        // Now get the groups of the specified module (official or not)
        foundAny = GetGroups(groupList, whichModule, withType) || foundAny;

        // Make sure there are no dupe groups in the list
        groupList.sort();
        groupList.unique();
    }

    return foundAny;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns actor defined in the specified loadout.

Actor * PresetMan::GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip)
{
	return GetLoadout(loadoutName, GetModuleID(module), spawnDropShip);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns actor defined in the specified loadout.

Actor * PresetMan::GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip)
{
	if (spawnDropShip)
	{
		// Find the Loadout that this Deployment is referring to
		const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
		if (pLoadout)
		{
			const ACraft * pCraftPreset = pLoadout->GetDeliveryCraft();
			if (pCraftPreset)
			{
				ACraft * pCraft = dynamic_cast<ACraft *>(pCraftPreset->Clone());
				if (pCraft)
				{
					float tally = 0;
					// Create and pass along the first Actor and his inventory defined in the Loadout
					Actor * pActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
					// Set the position and team etc for the Actor we are prepping to spawn
					if (pActor)
						pCraft->AddInventoryItem(pActor);
				}
				return pCraft;
			}
		}
	}
	else
	{
		// Find the Loadout that this Deployment is referring to
		const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
		if (pLoadout)
		{
			float tally = 0;
			// Create and pass along the first Actor and his inventory defined in the Loadout
			Actor * pReturnActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
			// Set the position and team etc for the Actor we are prepping to spawn
			return pReturnActor;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PresetMan::FindAndExtractZippedModules() const {
	for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory())) {
		std::string zippedModulePath = std::filesystem::path(directoryEntry).generic_string();
		if (zippedModulePath.find(System::GetZippedModulePackageExtension()) == zippedModulePath.length() - System::GetZippedModulePackageExtension().length()) {
			LoadingScreen::LoadingSplashProgressReport("Extracting Data Module from: " + directoryEntry.path().filename().generic_string(), true);
			LoadingScreen::LoadingSplashProgressReport(System::ExtractZippedDataModule(zippedModulePath), true);
		}
	}
}

} // namespace RTE
