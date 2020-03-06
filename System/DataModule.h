#ifndef _RTEDATAMODULE_
#define _RTEDATAMODULE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            DataModule.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the DataModule class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"
#include "ContentFile.h"
#include "Constants.h"

struct DATAFILE;
struct BITMAP;

namespace RTE
{

class Entity;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           DataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A representation of a Data Module containing zero or many Material,
//                  Effect, Ammo, Device, Actor, or Scene definitions.
// Parent(s):       Serializable.
// Class history:   08/02/2006 DataModule created.

class DataModule:
    public Serializable
{
	friend class LuaMan;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Struct:          PresetEntry
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Holds and owns the actual object instance pointer, and the location of
    //                  of the data file it was read from, as well as where in that file.
    // Parent(s):       None.
    // Class history:   10/08/2007 PresetEntry created.

    struct PresetEntry
    {
        // Owned by this
        Entity *m_pEntityPreset;
        // Where the instance was read from
        std::string m_FileReadFrom;

        PresetEntry(Entity *pPreset, std::string file) { m_pEntityPreset = pPreset; m_FileReadFrom = file; }
    };

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     DataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a DataModule object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    DataModule() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     DataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a DataModule object in system
//                  memory, and also do a Create() in the same line. Create() should
//                  therefore not be called after using this constructor.
// Arguments:       A string defining the path to where the content file itself is located,
//                  either within the package file, or directly on the disk.
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this DataModule's creation.

    DataModule(std::string moduleName, ProgressCallback fpProgressCallback = 0) { Clear(); Create(moduleName, fpProgressCallback); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~DataModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a DataModule object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~DataModule() { Destroy(true); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the DataModule object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the DataModule object ready for use. This looks for an "index.ini"
//                  within the specified .rte directory and loads all the defined objects
//                  in that index file. This needs to be called after PresetMan is created.
// Arguments:       A string defining the name of this Data Module, e.g. "MyModule.rte".
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this DataModule's creation.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(std::string moduleName, ProgressCallback fpProgressCallback = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadModuleProperties
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Read module specific properties from index.ini without processing 
//					IncludeFiles and loading the whole module
// Arguments:       A string defining the name of this Data Module, e.g. "MyModule.rte".
//                  A function pointer to a function that will be called and sent a string
//                  with information about the progress of this DataModule's creation.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int ReadModuleProperties(std::string moduleName, ProgressCallback fpProgressCallback = 0);



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire DataModule, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this DataModule to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the DataModule will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the DataModule object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReloadAllScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads all scripted Entity Presets witht he latest version of their
//                  respective script files.
// Arguments:       None.
// Return value:    None.

    void ReloadAllScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetFileName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the file name of this Data Module, e.g. "MyMod.rte".
// Arguments:       None.
// Return value:    A string with the data module file name.

    virtual const std::string & GetFileName() const { return m_FileName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetFriendlyName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the friendly name of this Data Module, e.g. "My Great Mod".
// Arguments:       None.
// Return value:    A string with the data module's friendly name.

    virtual const std::string & GetFriendlyName() const { return m_FriendlyName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAuthor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the author name of this Data Module, e.g. "Data Realms, LLC".
// Arguments:       None.
// Return value:    A string with the author's name.

    virtual const std::string & GetAuthor() const { return m_Author; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDescription
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the description of this Data Module's contents.
// Arguments:       None.
// Return value:    A string with the description.

    virtual const std::string & GetDescription() const { return m_Description; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetVersionNumber
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the version number of this Data Module.
// Arguments:       None.
// Return value:    An int with the version number, starting at 1.

    virtual int GetVersionNumber() const { return m_Version; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the BITMAP that visually represents this Data Module, for use in
//                  menues. 
// Arguments:       None.
// Return value:    BITMAP pointer that might have the icon. 0 is very possible.

    BITMAP * GetIcon() const { return m_pIcon; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Entity instance's pointer and name associations to the
//                  internal list of already read in Entity:s. Ownership is NOT transferred!
//                  If there already is an instance defined, nothing happens. If there
//                  is not, a clone is made of the passed-in Entity and added to the library.
// Arguments:       A pointer to the Entity derived instance to add. It should be created
//                  from a Reader. Ownership is NOT transferred!
//                  Whether to overwrite if an instance of the EXACT same TYPE and name
//                  was found. If one of the same name but not the exact type, false
//                  is returned regardless and nothing will have been added.
//                  The file the instance was read from, or where it should be written.
//                  If "Same" is passed as the file path read from, an overwritten isntance
//                  will keep the old one's file location entry.
// Return value:    Whether or not a copy of the passed-in instance was successfully inserted
//                  into the module. False will be returned if there already was an instance
//                  of that class and instance name inserted previously, unless overwritten.

    bool AddEntityPreset(Entity *pEntToAdd, bool overwriteSame = false, std::string readFromFile = "Same");


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a previously read in (defined) Entity, by exact type and instance
//                  name. Ownership is NOT transferred!
// Arguments:       The exact type name of the derived Entity instance to get.
//                  The instance name of the derived Entity instance.
// Return value:    A pointer to the requested Entity instance. 0 if no Entity with that
//                  derived type or instance name was found. Ownership is NOT transferred!

    const Entity * GetEntityPreset(std::string exactType, std::string instance);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityDataLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the data file path of a previously read in (defined) Entity.
// Arguments:       The type name of the derived Entity. Ownership is NOT transferred!
//                  The instance name of the derived Entity instance.
// Return value:    The file path of the data file that the specified Entity was read from.
//                  If no Entity of that description was found, "" is returned.

    std::string GetEntityDataLocation(std::string exactType, std::string instance);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys, by inexact type.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfType(std::list<Entity *> &objectList, std::string type);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllOfGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list all previously read in (defined) Entitys which are
//                  associated with a specific group.
// Arguments:       Reference to a list which will get all matching Entity:s added to it.
//                  Ownership of the list or the Entitys placed in it are NOT transferred!
//                  The group to look for.
//                  The name of the least common denominator type of the Entitys you want.
//                  "All" will look at all types.
// Return value:    Whether any Entity:s were found and added to the list.

    bool GetAllOfGroup(std::list<Entity *> &objectList, std::string group, std::string type);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers the existence of an Entity group in this module.
// Arguments:       The group to register.
// Return value:    None.

    void RegisterGroup(std::string newGroup) { m_GroupRegister.push_back(newGroup); m_GroupRegister.sort(); m_GroupRegister.unique(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroupRegister
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of all registered Entity groups of this.
// Arguments:       None.
// Return value:    The list of all groups. Ownership is not transferred.

    const std::list<std::string> * GetGroupRegister() const { return &m_GroupRegister; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroupsWithType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Fills out a list with all groups registered with this that contain any
//                  objects of a specific type and it derivatives.
// Arguments:       The list that all found groups will be ADDED to. OINT.
//                  The name of the type to only get groups of.
// Return value:    Whether any groups with the specified type was found.

    bool GetGroupsWithType(std::list<std::string> &groupList, std::string withType);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a Material mapping local to a DataModule. This is used for when
//                  multiple DataModule:s are loading conflicting Material:s, and need to
//                  resolve the conflicts by mapping their materials to ID's different than
//                  those specified in the data files.
// Arguments:       The material ID to map from.
//                  The material ID to map to.
// Return value:    Whether this created a new mapping which didn't override a previous
//                  material mapping.

    bool AddMaterialMapping(int fromID, int toID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialMapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a Material mapping local to this DataModule. This is used for when
//                  multiple DataModule:s are loading conflicting Material:s, and need to
//                  resolve the conflicts by mapping their materials to ID's different than
//                  those specified in the data files.
// Arguments:       The material ID to get the mapping for.
// Return value:    The material ID that the passed in ID is mapped to, if any. 0 if no
//                  mapping present.

    int GetMaterialMapping(int materialID) const { return m_MaterialMappings[materialID]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAllMaterialMappings
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the entire Material mapping array local to this DataModule.
// Arguments:       None.
// Return value:    A pointer to the entire local mapping array, 256 unsigned chars.
//                  Ownership is NOT transferred!

    const unsigned char * GetAllMaterialMappings() const { return m_MaterialMappings; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetIgnoreMissingItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if laoder should ignore missing items in this module.
// Arguments:       None.
// Return value:    true if laoder should ignore missing items in this module.
	const bool GetIgnoreMissingItems() const { return m_IgnoreMissingItems; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCrabToHumanSpawnRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns crab-to-human spawn ration for this tech.
// Arguments:       None.
// Return value:    Crab-to-human spawn ration value.
	const float GetCrabToHumanSpawnRatio() const { return m_CrabToHumanSpawnRatio; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the preset scripts of this object, from a specified path.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEntityIfExactType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if the type map has an instance added of a specific name and
//                  exact type. Does not check if any parent types with that name has been
//                  added. If found, that instance is returned, otherwise 0.
// Arguments:       The exact type name to look for.
//                  The exact instance name to look for.
// Return value:    The found Entity Preset of the exact type and name, if found.

    Entity * GetEntityIfExactType(const std::string &exactType, const std::string &instanceName);

	Entity * GetEntityIfExactType(const std::string &exactType, size_t instanceHash);




//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  AddToTypeMap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a newly added preset instance to the type map, where it will
//                  end up in every typelist of every class it derived from as well.
//                  I.e the "Entity" map will contain every single instance added to this.
//                  This will NOT check if duplicates are added to any type typelist, so
//                  please use GetEntityIfExactType to check this beforehand. Dupes are
//                  allowed if there are no more than one of the exact class and name.
// Arguments:       The new object instance to add. OINT!
// Return value:    Whether the Entity was added successfully or not.

    bool AddToTypeMap(Entity *pEntToAdd);


    static const std::string m_ClassName;

    // File/folder name of the data module, eg "MyMod.rte"
    std::string m_FileName;
    // Friendly name of the data module, eg "My Weapons Mod"
    std::string m_FriendlyName;
    // Name of the author of this module
    std::string m_Author;
    // Brief description of what this module is and contains
    std::string m_Description;
    // Version number, starting with 1
    int m_Version;
    // ID number assigned to this upon loading, for internal use only, don't reflect in ini's
    int m_ModuleID;
    // File to the icon/symbol bitmap
    ContentFile m_IconFile;
    // Bitmap with the icon loaded form above file
    BITMAP *m_pIcon;
    // Ordered list of all owned Entity instances, ordered by the sequence of their reading - really now with overwriting?.
    // This is used to be able to write back all of them in proper order into their respective files in the DataModule when writing this
    // The Entity instances ARE owned by this list.
    std::list<PresetEntry> m_PresetList;

	// A list of loaded entities solely for the purpose of enumaretion presets from Lua
	std::list<const Entity *> m_EntityList;

    // map of <class names and <map of instance template names and actual Entity instances> > that were read for this DataModule
    // An Entity instance of a derived typ will be placed in EACH of EVERY of its parent class' maps here.
    // There can be multiple entries of the same instance name in any of the type submaps, but only ONE whose exact class is that of the typelist!
    // The Entity instaces are NOT owned by this map.
    std::map<std::string, std::list<std::pair<std::string, Entity *> > > m_TypeMap;

    // List of all Entity groups ever registered in this, all uniques
    std::list<std::string> m_GroupRegister;
    // Material mappings local to this DataModule
    unsigned char m_MaterialMappings[c_PaletteEntriesNumber];
	// Indicates whether module loader should scan for any .ini's inside module folder
	// instead of using IncludeFile
	bool m_ScanFolderContents;
	// Indicates whether module loader should ignore missing items in this module
	bool m_IgnoreMissingItems;
	// Crab-to-human Spawn ratio to replace value from Constants.lua
	float m_CrabToHumanSpawnRatio;
	// Path to script to execute when this module is loaded
	std::string m_ScriptPath;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this DataModule, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the DataModule object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/
// Prevent from cloning Data Modules
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a DataModule to be identical to another, by deep copy.
// Arguments:       A reference to the DataModule to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const DataModule &reference);


};

} // namespace RTE

#endif // File