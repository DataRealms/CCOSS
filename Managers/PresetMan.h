#ifndef _RTEPRESETMAN_
#define _RTEPRESETMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            PresetMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the PresetMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Singleton.h"

#define g_PresetMan PresetMan::Instance()

namespace RTE
{

class Actor;
class DataModule;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           PresetMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of all presets of Entity:s in the RTE.
//                  The presets serve as a respository of Entity instances with specific
//                  and unique and initial runtime data.
// Parent(s):       Singleton, Serializable.
// Class history:   12/25/2001 PresetMan created.
// Class history:   05/30/2008 Changed name to PresetMan.

class PresetMan : public Singleton<PresetMan> {
	friend class LuaMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     PresetMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a PresetMan entity in system
//                  memory. Create() should be called before using the entity.
// Arguments:       None.

    PresetMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~PresetMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a PresetMan entity before deletion
//                  from system memory.
// Arguments:       None.

    ~PresetMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the PresetMan entity ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Initialize() { return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire PresetMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the PresetMan entity.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadDataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an entire data module and adds it to this. NOTE that official
//                  modules can't be loaded after any non-official ones!
// Arguments:       The module name to read, eg "Base.rte"
//                  Whether this module is 'official' or user-made. If official, it has to
//                  not have any name conflicts with any other offical module.
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this DataModule's creation.
// Return value:    Whether the DataModule was read and added correctly.

    bool LoadDataModule(std::string moduleName, bool official, ProgressCallback fpProgressCallback = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadDataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an entire data module and adds it to this. NOTE that official
//                  modules can't be loaded after any non-official ones!
// Arguments:       The module name to read, eg "Base.rte"
// Return value:    Whether the DataModule was read and added correctly.

    bool LoadDataModule(std::string moduleName) { return LoadDataModule(moduleName, false, 0); }

	/// <summary>
	/// Loads all the official data modules individually with LoadDataModule, then proceeds to look for any non-official modules and loads them as well.
	/// </summary>
	/// <returns></returns>
	bool LoadAllDataModules();

	/// <summary>
	/// Sets the single module to be loaded after the official modules. This will be the ONLY non-official module to be loaded.
	/// </summary>
	/// <param name="moduleName">Name of the module to load.</param>
	void SetSingleModuleToLoad(std::string moduleName) { m_SingleModuleToLoad = moduleName; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific loaded DataModule
// Arguments:       The ID of the module to get.
// Return value:    The requested DataModule. Ownership is NOT transferred!

    const DataModule * GetDataModule(int whichModule = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDataModuleName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a name specific loaded DataModule
// Arguments:       The ID of the module to get.
// Return value:    The requested DataModule name.

	const std::string GetDataModuleName(int whichModule = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of a loaded DataModule.
// Arguments:       The name of the DataModule to get the ID from, including the ".rte"
// Return value:    The requested ID. If no module of the name was found, -1 will be returned.

    int GetModuleID(std::string moduleName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleIDFromPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ID of a loaded DataModule, from a full data file path.
// Arguments:       The full path to a data file inside the data module id you want to get.
// Return value:    The requested ID. If no module of the name was found, -1 will be returned.

    int GetModuleIDFromPath(std::string dataPath);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalModuleCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of modules loaded so far, official or not.
// Arguments:       None.
// Return value:    The number of modules loaded so far, both official and non.

    int GetTotalModuleCount() { return m_pDataModules.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOfficialModuleCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of OFFICIAL modules loaded so far.
// Arguments:       None.
// Return value:    The number of official modules loaded so far.

    int GetOfficialModuleCount() { return m_OfficialModuleCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Entity instance's pointer and name associations to the
//                  internal list of already read in Entity:s. Ownership is NOT transferred!
//                  If there already is an instance defined, nothing happens. If there
//                  is not, a clone is made of the passed-in Entity and added to the library.
// Arguments:       A pointer to the Entity derived instance to add. It should be created
//                  from a Reader. Ownership is NOT transferred!
//                  Which module to add the entity to.
//                  Whether to overwrite if an instance of the EXACT same TYPE and name
//                  was found. If one of the same name but not the exact type, false
//                  is returned regardless and nothing will have been added.
//                  The file this instance was read from, or where it should be written.
//                  If "Same" is passed as the file path read from, an overwritten instance
//                  will keep the old one's file location entry.
// Return value:    Whether or not a copy of the passed-in instance was successfully inserted
//                  into the module. False will be returned if there already was an instance
//                  of that class and instance name inserted previously, unless overwritten.

    bool AddEntityPreset(Entity *pEntToAdd, int whichModule = 0, bool overwriteSame = false, std::string readFromFile = "Same");


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a previously read in (defined) Entity, by type and instance name.
// Arguments:       The type name of the derived Entity. Ownership is NOT transferred!
//                  The instance name of the derived Entity instance.
//                  Which module to try to get the entity from. If it's not found there,
//                  the official modules will be searched also. -1 means search ALL modules!
// Return value:    A pointer to the requested Entity instance. 0 if no Entity with that
//                  derived type or instance name was found. Ownership is NOT transferred!

    const Entity * GetEntityPreset(std::string type, std::string preset, int whichModule = -1);
    // Helper for passing in string module name instead of ID
    const Entity * GetEntityPreset(std::string type, std::string preset, std::string module) { return GetEntityPreset(type, preset, GetModuleID(module)); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads an instance of an Entity that will be used as preset
//                  to later on be used to generate more instances of the same state.
//                  Will check if there was already one of the same class and instance
//                  name read in previously, and will return that one if found, or
//                  add the newly read in one to this PresetMan's list if not found to
//                  exist there previously. Ownership is NOT transferred!
// Arguments:       The Reader which is about to read in an instance reference. It'll make
//                  this look in the same module as it's reading from.
// Return value:    A const pointer to the Entity instance read in. 0 if there was an
//                  error, or the instance name was 'None'. Ownership is NOT transferred!

    const Entity * GetEntityPreset(Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReadReflectedPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a preset of an Entity and tries to add it to the list of
//                  read-in instances. Regardless of whether there is a name collision,
//                  the read-in preset will be returned, ownership TRANSFERRED!
// Arguments:       The Reader which is about to read in a preset.
// Return value:    A pointer to the Entity preset read in. 0 if there was an
//                  error, or the instance name was 'None'. Ownership IS transferred!

    Entity * ReadReflectedPreset(Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys, by type.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The type name of the Entitys you want.
//                  Whether to only get those of one specific DataModule (0-n), or all (-1).
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfType(std::list<Entity *> &entityList, std::string type, int whichModule = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfTypeInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  of a specific type, and only exist in a specific module space.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The type name of the Entitys you want.
//                  Which module to get the instances for, in addition to all groups in
//                  official modules loaded earlier than the one specified here. -1 means
//                  get ALL groups ever reg'd.
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfTypeInModuleSpace(std::list<Entity *> &entityList, std::string type, int whichModuleSpace);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The group to look for. "All" will look in all.
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
//                  Whether to only get those of one specific DataModule (0-n), or all (-1).
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfGroup(std::list<Entity *> &entityList, std::string group, std::string type = "All", int whichModule = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is randomly
//                  selected from a specific group.
// Arguments:       The group to randomly select an Entity from. "All" will look in all.
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
//                  Whether to only get those of one specific DataModule (0-n), or all (-1).
// Return value:    The Entity preset that was randomly selected. Ownership is NOT transferred!

    Entity * GetRandomOfGroup(std::string group, std::string type = "All", int whichModule = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroupFromTech
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is randomly
//                  selected from a specific group only if it belongs to some tech.
// Arguments:       The group to randomly select an Entity from. "All" will look in all.
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
//                  Whether to only get those of one specific DataModule (0-n), or all (-1)
//					or all modules uncluding non-tech ones.
// Return value:    The Entity preset that was randomly selected. Ownership is NOT transferred!

    Entity * GetRandomBuyableOfGroupFromTech(std::string group, std::string type = "All", int whichModule = -1);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroupInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group, and only exist in a specific module
//                  space.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The group to look for. "All" will look in all.
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
//                  Which module to get the instances for, in addition to all groups in
//                  official modules loaded earlier than the one specified here. -1 means
//                  get ALL groups ever reg'd.
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfGroupInModuleSpace(std::list<Entity *> &entityList, std::string group, std::string type, int whichModuleSpace);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRandomOfGroupInModuleSpace
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a previously read in (defined) Entity which is associated with
//                  a specific group, randomly selected and only exist in a specific module
//                  space.
// Arguments:       Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The group to randomly select from. "All" will look in all.
//                  The name of the least common denominator type of the Entity:s you want.
//                  "All" will look at all types.
//                  Which module to get the instances for, in addition to all groups in
//                  official modules loaded earlier than the one specified here. -1 means
//                  get ALL groups ever reg'd.
// Return value:    The randomly select preset, if any was found with thse search params.
//                  Ownership is NOT transferred!

    Entity * GetRandomOfGroupInModuleSpace(std::string group, std::string type, int whichModuleSpace);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityDataLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the data file path of a previously read in (defined) Entity.
// Arguments:       The type name of the derived Entity. Ownership is NOT transferred!
//                  The preset name of the derived Entity preset.
//                  Which module to try to get the entity from. If it's not found there,
//                  the official modules will be searched also.
// Return value:    The file path of the data file that the specified Entity was read from.
//                  If no Entity of that description was found, "" is returned.

    std::string GetEntityDataLocation(std::string type, std::string preset, int whichModule);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadAllScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads all scripted Entity Presets witht he latest version of their
//                  respective script files.
// Arguments:       None.
// Return value:    None.

    void ReloadAllScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Material mapping local to a DataModule. This is used for when
//                  multiple DataModule:s are loading conflicting Material:s, and need to
//                  resolve the conflicts by mapping their materials to ID's different than
//                  those specified in the data files.
// Arguments:       The material ID to map from.
//                  The material ID to map to.
//                  The ID of the  DataModule we are making the mapping for. This should be
//                  a non-official module as mapping shouldn't be needed in the official
//                  modules.
// Return value:    Whether this created a new mapping which didn't override a previous
//                  material mapping.

    bool AddMaterialMapping(int fromID, int toID, int whichModule);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers the existence of an Entity group, and in which module.
// Arguments:       The group to register.
//                  The ID of the module in which at least one entity of this group can be
//                  found.
//                  global register.
// Return value:    None.

    void RegisterGroup(std::string newGroup, int whichModule);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroups
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills out a list with all groups registered in a specific module.
// Arguments:       The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
//                  Which module to get the groups for. -1 means get ALL groups ever reg'd.
//                  Pass a type name here and only groups with entitys of that type will be
//                  be included. "All" means don't consider what types are in the groups.
// Return value:    Whether any groups were found and thus added to the list.

    bool GetGroups(std::list<std::string> &groupList, int whichModule = -1, std::string withType = "All") const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleSpaceGroups
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills out a list with all groups registered in all official modules,
//                  PLUS a specific non-official module as well.
// Arguments:       The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
//                  Which module to get the groups for, in addition to all groups in
//                  official modules loaded earlier than the one specified here. -1 means
//                  get ALL groups ever reg'd.
//                  Pass a type name here and only groups with entitys of that type will be
//                  be included. "All" means don't consider what types are in the groups.
// Return value:    Whether any groups were found and thus added to the list.

    bool GetModuleSpaceGroups(std::list<std::string> &groupList, int whichModule, std::string withType = "All") const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns actor defined in the specified loadout.
// Arguments:       Loadout preset name, module name, whether or not spawn delivery craft defined for that loadout
// Return value:    Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.

	Actor * GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadout
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns actor defined in the specified loadout.
// Arguments:       Loadout preset name, module id, whether or not spawn delivery craft defined for that loadout
// Return value:    Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.

	Actor * GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Owned and loaded DataModule:s
    std::vector<DataModule *> m_pDataModules;

    // Names of all DataModule:s mapped to indices into the m_pDataModules vector.
    // The names are all lowercase name so we can more easily find them in case-agnostic fashion
    std::map<std::string, size_t> m_DataModuleIDs;

    // How many modules are 'official' and shipped with the game, and guaranteed to not have name conflicts among them
    // All official modules are in the beginning of the m_TypeMap, so this count shows how many into that vector they represent
    int m_OfficialModuleCount;

	std::string m_SingleModuleToLoad; //!< Name of the single module to load after the official modules.

    // List of all Entity groups ever registered, all uniques
    // This is just a handy total of all the groups registered in all the individual DataModule:s
    std::list<std::string> m_TotalGroupRegister;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

	/// <summary>
	/// Iterates through the working directory to find any files matching the zipped module package extension (.rte.zip) and proceeds to extract them.
	/// </summary>
	void FindAndExtractZippedModules() const;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this PresetMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


	// Disallow the use of some implicit methods.
	PresetMan(const PresetMan &reference) = delete;
	PresetMan & operator=(const PresetMan &rhs) = delete;

};

} // namespace RTE

#endif // File