#ifndef _RTELUAMAN_
#define _RTELUAMAN_



//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "Singleton.h"
#define g_LuaMan LuaMan::Instance()

#include "Serializable.h"
#include "Entity.h"

// Forward declarations
struct lua_State;

namespace RTE
{

#define MAX_OPEN_FILES 10

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           LuaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the master lua script state.
// Parent(s):       Singleton, Serializable?
// Class history:   3/13/2008 LuaMan created.

class LuaMan:
    public Singleton<LuaMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     LuaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a LuaMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    LuaMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~LuaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a LuaMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~LuaMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the LuaMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Initialize();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire LuaMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the LuaMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLastError
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last error message from executing scripts.
// Arguments:       None.
// Return value:    The error string with hopefully meaningful info about what went wrong.

    std::string GetLastError() const { return m_LastError; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearErrors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the error string to "". Will cause ErrorExists to return
//                  true again until RunScriptString is called.
// Arguments:       None.
// Return value:    None.

    void ClearErrors() { m_LastError.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ErrorExists
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether there are any errors reported waiting to be read.
// Arguments:       None.
// Return value:    Whether errors exist.

    bool ErrorExists() const { return !m_LastError.empty(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SavePointerAsGlobal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a pointer to an object and saves it in the Lua state as a global
//                  of a specified variable name.
// Arguments:       The pointer to the object to save. Ownership is NOT transferred!
//                  The name of the global var in the Lua state to save the pointer to.
// Return value:    Returns less than zero if any errors encountered when doing this.

    int SavePointerAsGlobal(void *pToSave, std::string globalName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GlobalIsDefined
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if there is anything defined on a specific global var in Lua.
// Arguments:       The name of the global var in the Lua state to check.
// Return value:    Whether that global var has been defined yet in the Lua state.

    bool GlobalIsDefined(std::string globalName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TableEntryIsDefined
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if there is anything defined in a specific index of a table.
// Arguments:       The name of the table to look inside.
//                  The name of the index to check inside that table.
// Return value:    Whether that table var has been defined yet in the Lua state.

    bool TableEntryIsDefined(std::string tableName, std::string indexName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ExpressionIsTrue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the result of an arbirary expression in lua as evaluating to
//                  true or false.
// Arguments:       The string with the expression to evaluate.
//                  Whether to report any errors to the console immediately.
// Return value:    Whether the expression was true.

    bool ExpressionIsTrue(std::string expression, bool consoleErrors);

    /// <summary>
    /// Runs the given Lua function with optional safety checks and arguments. The first argument to the function will always be the self object.
    /// If either argument list has entries, they will be passed into the function in order, with entity arguments first.
    /// </summary>
    /// <param name="functionName">The name that gives access to the function in the global Lua namespace.</param>
    /// <param name="selfObjectName">The name that gives access to the self object in the global Lua namespace.</param>
    /// <param name="variablesToSafetyCheck">Optional vector of strings that should be safety checked in order before running the Lua function. Defaults to empty.</param>
    /// <param name="functionEntityArguments">Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.</param>
    /// <param name="functionLiteralArguments">Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.</param>
    /// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
    int RunScriptedFunction(const std::string &functionName, const std::string &selfObjectName, std::vector<std::string> variablesToSafetyCheck = std::vector<std::string>(), std::vector<Entity *> functionEntityArguments = std::vector<Entity *>(), std::vector<std::string> functionLiteralArguments = std::vector<std::string>());

    /// <summary>
    /// Takes a string containing a script snippet and runs it on the master state.
    /// </summary>
    /// <param name="scriptString">The string with the script snippet.</param>
    /// <param name="consoleErrors">Whether to report any errors to the console immediately.</param>
    /// <returns>Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.</returns>
    int RunScriptString(const std::string &scriptString, bool consoleErrors = true);

    /// <summary>
    /// Opens and loads a file containing a script and runs it on the master state.
    /// </summary>
    /// <param name="filePath">The path to the file to load and run.</param>
    /// <param name="consoleErrors">Whether to report any errors to the console immediately.</param>
    /// <returns>Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.</returns>
    int RunScriptFile(const std::string &filePath, bool consoleErrors = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewPresetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an ID string unique to this runtime for use by original presets
//                  that have scripts associated with them.
// Arguments:       None.
// Return value:    Returns the unique ID as a string.

    std::string GetNewPresetID();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNewObjectID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns an ID string unique to this runtime for use by individual
//                  objects that are also tracked in the Lua state and have scripts
//                  associated with them.
// Arguments:       None.
// Return value:    Returns the unique ID as a string.

    std::string GetNewObjectID();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTempEntity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a temporary Entity that can be accessed in the Lua state.
// Arguments:       The temporary entity. Ownership is NOT transferred.
// Return value:    None.

    void SetTempEntity(Entity *pEntity) { m_pTempEntity = pEntity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTempEntity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a temporary Entity that can be accessed in the Lua state.
// Arguments:       None.
// Return value:    The temporary entity. Ownership is NOT transferred.

    Entity * GetTempEntity() const { return m_pTempEntity; }

    /// <summary>
    /// Sets a temporary vector of entities that can be accessed in the Lua state
    /// </summary>
    /// <param name="entityVector">The temporary vector of entities. Ownership is NOT transferred.</param>
    void SetTempEntityVector(std::vector<Entity *> entityVector) { m_TempEntityVector = entityVector; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this LuaMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.
 
	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileOpen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens file. You can open files only inside .rte folders of game directory.
//					you can open no more that MAX_OPEN_FILES file simultaneously.
// Arguments:       Path to file. All paths are made absoulte by adding current working directory
//					to specified path. File mode.
// Return value:    File number.

	int FileOpen(std::string filename, std::string mode);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileClose
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes a previously opened file.
// Arguments:       File number.
// Return value:    None.

	void FileClose(int file);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileCloseAll
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes all previously opened files.
// Arguments:       None.
// Return value:    None.

	void FileCloseAll();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileReadLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a line from file.
// Arguments:       File number.
// Return value:    Line from file, or empty string on error.

	std::string FileReadLine(int file);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileWriteLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Writes a text line to file.
// Arguments:       File number.
// Return value:    None.

	void FileWriteLine(int file, std::string line);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FileEOF
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if end of file was reached
// Arguments:       File number.
// Return value:    Whether or not EOF was reached.

	bool FileEOF(int file);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearUserModuleCache
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts()
// Arguments:       None.
// Return value:    None.

	void ClearUserModuleCache();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // The master parent script state
    lua_State *m_pMasterState;
    // Description of the last error that occurred in the script execution
    std::string m_LastError;
    // The next unique preset ID to hand out to the next Preset that wants to define some functions
    // This gets incremented each time a new one is requeste to give unique ID's to all original presets
    long m_NextPresetID;
    // The next unique object ID to hand out to the next scripted Entity instance that wants to run its preset's scripts
    // This gets incremented each time a new one is requested to give unique ID's to all scripted objects
    long m_NextObjectID;
    // Temporary holder for an Entity object that we want to pass into the Lua state without fuss. Lets you export objects to lua easily.
    Entity *m_pTempEntity;
    // Temporary holder for a vector of Entities that we want to pass into the Lua state without a fuss. Usually used to pass arguments to special Lua functions.
    std::vector<Entity *> m_TempEntityVector;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this LuaMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	LuaMan(const LuaMan &reference) = delete;
	LuaMan & operator=(const LuaMan &rhs) = delete;

	//Internal list of opened files used by File* functions 
	FILE * m_Files[MAX_OPEN_FILES];
};

} // namespace RTE

#endif // File