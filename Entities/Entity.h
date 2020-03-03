#ifndef _RTEENTITY_
#define _RTEENTITY_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Entity.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Entity class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff

#include "Serializable.h"
#include "Reader.h"
#include "Writer.h"
#include "RTETools.h"
#include "Vector.h"

namespace RTE
{

// Preprocessor helper to define this:

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:   Allocate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Static method used in conjunction with ClassInfo to allocate a
//                  MOPixel. This function is passed intothe constructor of this
//                  MOPixel's static ClassInfo's constructor, so that it can
//                  instantiate MovableObject:s.
// Arguments:       None.
// Return value:    A pointer to the newly dynamically allocated MOPixel.
//                  Ownership is transferred as well.


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Clone
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses a passed-in instance, or creates a new one, and makes it identical
//                  to this.
// Arguments:       A pointer to an instance to make identical to this. If 0 is passed in,
//                  a new instance is made inside here, and owenership of it IS returned!
// Return value:    An Entity pointer to the newly cloned-to instance.
//                  Ownership IS transferred!

#define ENTITYALLOCATION(TYPE) \
\
    static void * operator new (size_t size) { return TYPE::m_sClass.GetPoolMemory(); } \
\
    static void operator delete (void *pInstance) { TYPE::m_sClass.ReturnPoolMemory(pInstance); } \
\
    static void * operator new (size_t size, void *p) throw() { return p; } \
\
    static void operator delete (void *, void *) throw() {  } \
\
    static void * Allocate() { return malloc(sizeof(TYPE)); } \
\
    static void Deallocate(void *pInstance) { free(pInstance); } \
\
    static Entity * NewInstance() { return new TYPE; } \
\
    virtual Entity * Clone(Entity *pCloneTo = 0) const { TYPE *pEnt = pCloneTo ? dynamic_cast<TYPE *>(pCloneTo) : new TYPE(); \
                                                         AAssert(pEnt, "Tried to clone to an incompatible instance!"); \
                                                         if (pCloneTo) { pEnt->Destroy(); } \
                                                         pEnt->Create(*this); \
                                                         return pEnt; }


//////////////////////////////////////////////////////////////////////////////////////////
// Static ClassInfo definitions everywhere

#define ABSTRACTCLASSINFO(TYPE, PARENT) \
    Entity::ClassInfo TYPE::m_sClass(#TYPE, &PARENT::m_sClass);

#define CONCRETECLASSINFO(TYPE, PARENT, BLOCKCOUNT) \
    Entity::ClassInfo TYPE::m_sClass(#TYPE, &PARENT::m_sClass, TYPE::Allocate, TYPE::Deallocate, TYPE::NewInstance, BLOCKCOUNT);

#define CONCRETESUBCLASSINFO(TYPE, SUPER, PARENT, BLOCKCOUNT) \
    Entity::ClassInfo SUPER::TYPE::m_sClass(#TYPE, &PARENT::m_sClass, SUPER::TYPE::Allocate, SUPER::TYPE::Deallocate, SUPER::TYPE::NewInstance, BLOCKCOUNT);

#define CLASSINFOGETTERS \
	const Entity::ClassInfo & GetClass() const { return m_sClass; } \
	const std::string & GetClassName() const { return m_sClass.GetName(); }


// Whether to draw the colors, or own material property, or to clear the
// corresponding non-key-color pixels of the Entity being drawn with
// key-color pixels on the target.
enum DrawMode
{
    g_DrawColor = 0,
    g_DrawMaterial,
    g_DrawAir,
    g_DrawKey,
    g_DrawWhite,
    g_DrawMOID,
    g_DrawNoMOID,
    g_DrawDebug,
    g_DrawLess,
    g_DrawTrans,
    g_DrawRedTrans,
    g_DrawScreen,
    g_DrawAlpha
};


//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  Entity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The base class that specifies certain common creation/destruction
//                  patterns and simple reflection support for virtually all RTE classes.
// Parent(s):       Serializable.
// Class history:   01/31/2002 Object created.
// Class history:   06/02/2008 Changed name to Entity so it's distinct from generic term.

class Entity:
    public Serializable
{
    friend class DataModule;


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    ClassInfo
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     The class that describes each subclass of Entity. There should be one
    //                  ClassInfo static instance for every Entity child.
    // Parent(s):       None.
    // Class history:   01/31/2002 ClassInfo created.

    class ClassInfo
    {
        friend class Entity;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     ClassInfo
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a ClassInfo Entity.
    // Arguments:       A friendly-formatted name of the Entity that is going to be
    //                  represented by this ClassInfo.
    //                  Pointer to the parent class' info. 0 if this describes a root class.
    //                  Function pointer to the raw allocation function of the derived's size. If
    //                  the represented Entity subclass isn't concrete, pass in 0.
    //                  Function pointer to the raw deallocation function of memory. If
    //                  the represented Entity subclass isn't concrete, pass in 0..
    //                  Function pointer to the new instance factory . If
    //                  the represented Entity subclass isn't concrete, pass in 0.
    //                  The number of new instances to fill the pre-allocated pool with when
    //                  it runs out.

        ClassInfo(const std::string &name, ClassInfo *pParentInfo = 0, void * (*fpAllocFunc)() = 0, void (*fpDeallocFunc)(void *) = 0, Entity * (*fpNewFunc)() = 0, int allocBlockCount = 10);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the name of this ClassInfo.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted name of this ClassInfo.

        const std::string & GetName() const { return m_Name; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetParent
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the ClassInfo which describes the parent of this.
    // Arguments:       None.
    // Return value:    A pointer to the parent ClassInfo. 0 if this is a root class.

        const ClassInfo * GetParent() const { return m_pParentInfo; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Static method:   GetClassNames
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the names of all ClassInfo:s in existence.
    // Arguments:       None.
    // Return value:    A list of the names.

        static std::list<std::string> GetClassNames();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Static method:   GetClass
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the ClassInfo of a particular RTE class corresponding to a
    //                  friendly-formatted string name. 
    // Arguments:       The friendly name of the desired ClassInfo.
    // Return value:    A pointer to the requested ClassInfo, or 0 if none that matched the
    //                  name was found. Ownership is NOT transferred!

        static const ClassInfo * GetClass(const std::string &name);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Static method:   FillAllPools
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Adds a certain number of newly allocated instances to all pools.
    // Arguments:       The number of instances to fill er up with. If 0 is specified, the set
    //                  refill amount will be used.
    // Return value:    None.

        static void FillAllPools(int fillAmount = 0);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          FillPool
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Adds a certain number of newly allocated instances to this' pool.
    // Arguments:       The number of instances to fill er up with.If 0 is specified, the set
    //                  refill amount will be used.
    // Return value:    None.

        void FillPool(int fillAmount = 0);

/*
    //////////////////////////////////////////////////////////////////////////////////////////
    // Static method:   ReadInstance
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Reads the class name from a Reader, and then does everything needed to
    //                  create an instance of that class and returns that instance.
    // Arguments:       The friendly name of the desired ClassInfo.
    // Return value:    A pointer to the dynamically allocated Entity. Ownership is transferred.
    //                  If the represented Entity subclass isn't concrete, 0 will be returned.

        static Entity * ReadInstance(Reader &reader);
*/

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          IsConcrete
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Returns whether the represented Entity subclass is concrete or not,
    //                  that is if it can create new instances through NewInstance().
    // Arguments:       None.
    // Return value:    Whether the represented Entity subclass is concrete or not.

        bool IsConcrete() const { return m_fpAllocate != 0 ? true : false; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  NewInstance
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Dynamically allocates an instance of the Entity subclass that this
    //                  ClassInfo represents. If the Entity isn't concrete, 0 will be returned.
    // Arguments:       None.
    // Return value:    A pointer to the dynamically allocated Entity. Ownership is transferred.
    //                  If the represented Entity subclass isn't concrete, 0 will be returned.

        virtual Entity * NewInstance() const { return IsConcrete() ? m_fpNewInstance() : 0; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetPoolMemory
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Grabs from the pre-allocated pool, an available chunk of memory the
    //                  exact size of the Entity this ClassInfo represents.
    //                  OWNERSHIP IS TRANSFERRED!
    // Arguments:       None.
    // Return value:    A pointer to the pre-allocated pool memory. OWNERSHIP IS TRANSFERRED!

        virtual void * GetPoolMemory();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  ReturnPoolMemory
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Returns a raw chunk of memory back to the pre-allocated available pool.
    // Arguments:       The raw chunk of memory that is being returned. Needs to be the same
    //                  size as the type this ClassInfo describes. OWNERSHIP IS TRANSFERRED!
    // Return value:    The count of outstanding memory chunks after this was returned.

        virtual int ReturnPoolMemory(void *pReturnedMemory);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Static method:   DumpPoolMemoryInfo
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Writes a bunch of useful debug info about the memory pools to a file.
    // Arguments:       The writer to write info to.
    // Return value:    None.

        static void DumpPoolMemoryInfo(Writer &fileWriter);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:


        // Forbidding copying
        ClassInfo(const ClassInfo &reference);
        ClassInfo & operator=(const ClassInfo &rhs);

        // Member variables
        const std::string m_Name;
        const ClassInfo *m_pParentInfo;
        // Raw memory allocation and deallocation funcs for the size of the type this ClassInfo describes
        void *(*m_fpAllocate)();
        void (*m_fpDeallocate)(void *);
        // Returns an actual new instance of the type that this describes
        Entity *(*m_fpNewInstance)();

        // Head of unordered linked list of ClassInfo:s in existence.
        static ClassInfo *m_sClassHead;
        // Next ClassInfo after this one on aforementioned unordered linked list.
        ClassInfo *m_NextClass;

        // Pool of pre-allocated objects of the type described by this ClassInfo
        std::vector<void *> m_AllocatedPool;
        // The number of instances to fill up the pool of this type with each time it runs dry
        int m_PoolAllocBlockCount;
        // The number of allocated instances passed out from the pool
        int m_InstancesInUse;
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Entity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Entity in system
//                  memory. Create() should be called before using the Entity.
// Arguments:       None.

    Entity() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Entity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Entity before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Entity() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Entity ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an Entity to be identical to another, by deep copy.
// Arguments:       A reference to the Entity to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Entity &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Serializable ready for use.
// Arguments:       A Reader that the Serializable will create itself from.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true) { return Serializable::Create(reader, checkType, doCreate); }


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
// Pure V. method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Entity to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       A Writer that the Entity will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SavePresetCopy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only saves out a Preset reference of this to the stream. Is only
//                  applicable to objects that are not original presets and haven't been
//                  altered since they were copied from their original.
// Arguments:       A Writer that the Entity will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int SavePresetCopy(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Entity stream insertion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A stream insertion operator for sending a Entity to an output
//                  stream.
// Arguments:       An ostream reference as the left hand side operand.
//                  A Entity reference as the right hand side operand.
// Return value:    An ostream reference for further use in an expression.

    friend std::ostream & operator<<(std::ostream &stream, const Entity &operand) { stream << operand.GetPresetName() << ", " << operand.GetClassName();
                                                                                    return stream; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Clone
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses a passed-in instance, or creates a new one, and makes it identical
//                  to this.
// Arguments:       A pointer to an instance to make identical to this. If 0 is passed in,
//                  a new instance is made inside here, and owenership of it IS returned!
// Return value:    An Entity pointer to the newly cloned-to instance.
//                  Ownership IS transferred!

    virtual Entity * Clone(Entity *pCloneTo = 0) const { DDTAbort("Attempt to clone an abstract or unclonable type!"); return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Entity, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); /*Serializable::Reset();*/ }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Entity.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReloadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the Preset scripts of this Entity, from the same script file
//                  path as was originally defined. This will also update the original
//                  Preset in the PresetMan with the updated scripts so future objects
//                  spawned will use the new scripts.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int ReloadScripts() { return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Entity.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  SetPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of this Entity's data Preset.
// Arguments:       A string reference with the instance name of this Entity.
// Return value:    None.

// TODO: Figure out how to handle if same name was set, still make it wasgivenname = true?
    virtual void SetPresetName(const std::string &newName) { /*if (m_PresetName != newName) { m_IsOriginalPreset = true; }*/ m_IsOriginalPreset = true; m_PresetName = newName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of this Entity's data Preset.
// Arguments:       None.
// Return value:    A string reference with the instance name of this Entity.

    const std::string & GetPresetName() const { return m_PresetName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleAndPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of this Entity's data Preset, preceded by the name of
//                  the Data Module it was defined in, separated with a '/'
// Arguments:       None.
// Return value:    A string with the module and instance name of this Entity.

    std::string GetModuleAndPresetName() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDescription
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the plain text description of this Entity's data Preset. Should
//                  only be a couple of sentences.
// Arguments:       A string reference with the preset description.
// Return value:    None.

    void SetDescription(const std::string &newDesc) { m_PresetDescription = newDesc; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDescription
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the plain text description of this Entity's data Preset.
// Arguments:       None.
// Return value:    A string reference with the plain text description name of this Preset.

    const std::string & GetDescription() const { return m_PresetDescription; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOriginalPreset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether this Entity was explicitly given a new instance name
//                  upon creation, or if it was just copied/inherited implicitly.
// Arguments:       None.
// Return value:    Whether this Entity was given a new Preset Name upon creation.

    bool IsOriginalPreset() const { return m_IsOriginalPreset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetOriginalPresetFlag
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets IsOriginalPreset flag to indicate that the object should be saved as CopyOf
// Arguments:       None.
// Return value:    None.

	void ResetOriginalPresetFlag() { m_IsOriginalPreset = false; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the module this Entity was supposed to be defined in.
// Arguments:       The ID of the module, or -1 if it hasn't been defined in any.
// Return value:    None.

    void SetModuleID(int whichModule) { m_DefinedInModule = whichModule; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetModuleID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows the ID of the DataModule this Entity has been defined in.
// Arguments:       None.
// Return value:    The ID of the module, or -1 if it hasn't been defined in any.

    int GetModuleID() const { return m_DefinedInModule; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  MigrateToModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this an original Preset in a different module than it was before.
//                  It severs ties deeply to the old module it was saved in.
// Arguments:       The ID of the new module.
// Return value:    Whether the migration was successful. If you tried to migrate to the
//                  same module it already was in, this would return false.

    virtual bool MigrateToModule(int whichModule);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddToGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds this Entity to a new grouping.
// Arguments:       A string which describes the group to add this to. Duplicates will be
//                  ignored.
// Return value:    None.

    void AddToGroup(std::string newGroup) { m_Groups.push_back(newGroup); m_Groups.sort(); m_Groups.unique(); m_LastGroupSearch.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGroupList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of groups this is member of.
// Arguments:       None.
// Return value:    A pointer to a list of strings which describes the groups this is added
//                  to. WOenrship is NOT transferred!

    const std::list<std::string> * GetGroupList() { return &m_Groups; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is part of a specific group or not.
// Arguments:       A string which describes the group to check for.
// Return value:    Whether this Entity is in the specified group or not.

    bool IsInGroup(const std::string &whichGroup);

//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Entity Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Entity from a Reader.
// Arguments:       A Reader reference as the left hand side operand.
//                  An Entity reference as the right hand side operand.
// Return value:    A Reader reference for further use in an expression.

    friend Reader & operator>>(Reader &reader, Entity &operand);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: Entity Pointer Reader extraction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A Reader extraction operator for filling an Entity from a Reader.
// Arguments:       A Reader reference as the left hand side operand.
//                  An Entity pointer as the right hand side operand.
// Return value:    A Reader reference for further use in an expression.

    friend Reader & operator>>(Reader &reader, Entity *operand);


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: GetRandomWeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns random weight used in PresetMan::GetRandomBuyableOfGroupFromTech
// Arguments:       None.
// Return value:    This item's random weight from 0 to 100

	int GetRandomWeight() const { return m_RandomWeight; }



//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Forbidding copying
    Entity(const Entity &reference) { }
    Entity & operator=(const Entity &rhs) { return *this; }

    // Member variables
    // Type description of this Entity
    static Entity::ClassInfo m_sClass;
    // The name of the Preset data this was cloned from, if any
    std::string m_PresetName;
    // Whether this is to be added to the PresetMan as an original preset instance.
    bool m_IsOriginalPreset;
    // The DataModule ID that this was successfully added to at some point. -1 if not added to anything yet.
    int m_DefinedInModule;
    // The description of the preset in user firendly plain text that will show up in menus etc
    std::string m_PresetDescription;
    // List of all tags associated with this. The groups are used to categorize and organize Entity:s
    std::list<std::string> m_Groups;
    // Last group search string, for more efficient response on multiple tries for the same group name
    std::string m_LastGroupSearch;
    // Last group search result, for more efficient response on multiple tries for the same group name
    bool m_LastGroupResult;
	// Random weight used when picking item using PresetMan::GetRandomBuyableOfGroupFromTech. 
	// From 0 to 100. 0 means item won't be ever picked.
	int m_RandomWeight;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Entity, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File
