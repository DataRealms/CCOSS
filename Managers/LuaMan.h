#ifndef _RTELUAMAN_
#define _RTELUAMAN_

#include "Singleton.h"
#include "Entity.h"
#include "RTETools.h"

#define g_LuaMan LuaMan::Instance()

struct lua_State;

namespace RTE {

	class LuabindObjectWrapper;

	class LuaStateWrapper {
	public:
#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a LuaStateWrapper object in system memory. Initialize() should be called before using the object.
		/// </summary>
		LuaStateWrapper() { Clear(); }

		/// <summary>
		/// Makes the LuaStateWrapper object ready for use.
		/// </summary>
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a LuaStateWrapper object before deletion from system memory.
		/// </summary>
		~LuaStateWrapper() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the LuaStateWrapper object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets a temporary Entity that can be accessed in the Lua state.
		/// </summary>
		/// <returns>The temporary entity. Ownership is NOT transferred!</returns>
		Entity * GetTempEntity() const;

		/// <summary>
		/// Sets a temporary Entity that can be accessed in the Lua state.
		/// </summary>
		/// <param name="entity">The temporary entity. Ownership is NOT transferred!</param>
		void SetTempEntity(Entity *entity);

		/// <summary>
		/// Gets the temporary vector of Entities that can be accessed in the Lua state.
		/// </summary>
		/// <returns>The temporary vector of entities. Ownership is NOT transferred!</returns>
		const std::vector<Entity *> & GetTempEntityVector() const;

		/// <summary>
		/// Sets a temporary vector of Entities that can be accessed in the Lua state. These Entities are const_cast so they're non-const, for ease-of-use in Lua.
		/// </summary>
		/// <param name="entityVector">The temporary vector of entities. Ownership is NOT transferred!</param>
		void SetTempEntityVector(const std::vector<const Entity *> &entityVector);

		/// <summary>
		/// Sets the proper package.path for the script to run.
		/// </summary>
		/// <param name="filePath">The path to the file to load and run.</param>
		void SetLuaPath(const std::string &filePath);

		/// <summary>
		/// Gets this LuaStateWrapper's internal lua state.
		/// </summary>
		/// <returns>This LuaStateWrapper's internal lua state.</returns>
		lua_State* GetLuaState() { return m_State; };
#pragma endregion

#pragma region Script Execution Handling
		/// <summary>
		/// Runs the given Lua function with optional safety checks and arguments. The first argument to the function will always be the self object.
		/// If either argument list has entries, they will be passed into the function in order, with entity arguments first.
		/// </summary>
		/// <param name="functionName">The name that gives access to the function in the global Lua namespace.</param>
		/// <param name="selfObjectName">The name that gives access to the self object in the global Lua namespace.</param>
		/// <param name="variablesToSafetyCheck">Optional vector of strings that should be safety checked in order before running the Lua function. Defaults to empty.</param>
		/// <param name="functionEntityArguments">Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.</param>
		/// <param name="functionLiteralArguments">Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc. Defaults to empty.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int RunScriptFunctionString(const std::string &functionName, const std::string &selfObjectName, const std::vector<std::string_view> &variablesToSafetyCheck = std::vector<std::string_view>(), const std::vector<const Entity *> &functionEntityArguments = std::vector<const Entity *>(), const std::vector<std::string_view> &functionLiteralArguments = std::vector<std::string_view>());

		/// <summary>
		/// Takes a string containing a script snippet and runs it on the state.
		/// </summary>
		/// <param name="scriptString">The string with the script snippet.</param>
		/// <param name="consoleErrors">Whether to report any errors to the console immediately.</param>
		/// <returns>Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.</returns>
		int RunScriptString(const std::string &scriptString, bool consoleErrors = true);

		/// <summary>
		/// Runs the given Lua function object. The first argument to the function will always be the self object.
		/// If either argument list has entries, they will be passed into the function in order, with entity arguments first.
		/// </summary>
		/// <param name="functionObjectWrapper">The LuabindObjectWrapper containing the Lua function to be run.</param>
		/// <param name="selfGlobalTableName">The name of the global Lua table that gives access to the self object.</param>
		/// <param name="selfGlobalTableKey">The key for this object in the respective global Lua table.</param>
		/// <param name="functionEntityArguments">Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.</param>
		/// <param name="functionLiteralArguments">Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int RunScriptFunctionObject(const LuabindObjectWrapper *functionObjectWrapper, const std::string &selfGlobalTableName, const std::string &selfGlobalTableKey, const std::vector<const Entity *> &functionEntityArguments = std::vector<const Entity *>(), const std::vector<std::string_view> &functionLiteralArguments = std::vector<std::string_view>());

		/// <summary>
		/// Opens and loads a file containing a script and runs it on the state.
		/// </summary>
		/// <param name="filePath">The path to the file to load and run.</param>
		/// <param name="consoleErrors">Whether to report any errors to the console immediately.</param>
		/// <returns>Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.</returns>
		int RunScriptFile(const std::string &filePath, bool consoleErrors = true);

		/// <summary>
		/// Opens and loads a file containing a script and runs it on the state, then retrieves all of the specified functions that exist into the output map.
		/// </summary>
		/// <param name="filePath">The path to the file to load and run.</param>
		/// <param name="functionNamesToLookFor">The vector of strings defining the function names to be retrieved.</param>
		/// <param name="outFunctionNamesAndObjects">The map of function names to LuabindObjectWrappers to be retrieved from the script that was run.</param>
		/// <returns>Returns less than zero if any errors encountered when running this script. To get the actual error string, call GetLastError.</returns>
		int RunScriptFileAndRetrieveFunctions(const std::string &filePath, const std::vector<std::string> &functionNamesToLookFor, std::unordered_map<std::string, LuabindObjectWrapper *> &outFunctionNamesAndObjects);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates this Lua state.
		/// </summary>
		void Update();
#pragma endregion

#pragma region MultiThreading
		/// <summary>
		/// Gets the mutex to lock this lua state.
		/// </summary>
		std::recursive_mutex& GetMutex() { return m_Mutex; };
#pragma endregion

#pragma region
		/// <summary>
		/// Gets whether the given Lua expression evaluates to true or false.
		/// </summary>
		/// <param name="expression">The string with the expression to evaluate.</param>
		/// <param name="consoleErrors">Whether to report any errors to the console immediately.</param>
		/// <returns>Whether the expression was true.</returns>
		bool ExpressionIsTrue(const std::string &expression, bool consoleErrors);

		/// <summary>
		/// Takes a pointer to an object and saves it in the Lua state as a global of a specified variable name.
		/// </summary>
		/// <param name="objectToSave">The pointer to the object to save. Ownership is NOT transferred!</param>
		/// <param name="globalName">The name of the global var in the Lua state to save the pointer to.</param>
		void SavePointerAsGlobal(void *objectToSave, const std::string &globalName);

		/// <summary>
		/// Checks if there is anything defined on a specific global var in Lua.
		/// </summary>
		/// <param name="globalName">The name of the global var in the Lua state to check.</param>
		/// <returns>Whether that global var has been defined yet in the Lua state.</returns>
		bool GlobalIsDefined(const std::string &globalName);

		/// <summary>
		/// Checks if there is anything defined in a specific index of a table.
		/// </summary>
		/// <param name="tableName">The name of the table to look inside.</param>
		/// <param name="indexName">The name of the index to check inside that table.</param>
		/// <returns>Whether that table var has been defined yet in the Lua state.</returns>
		bool TableEntryIsDefined(const std::string &tableName, const std::string &indexName);

		/// <summary>
		/// Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts().
		/// </summary>
		void ClearUserModuleCache();
#pragma endregion

#pragma region Error Handling
		/// <summary>
		/// Tells whether there are any errors reported waiting to be read.
		/// </summary>
		/// <returns>Whether errors exist.</returns>
		bool ErrorExists() const;

		/// <summary>
		/// Returns the last error message from executing scripts.
		/// </summary>
		/// <returns>The error string with hopefully meaningful info about what went wrong.</returns>
		std::string GetLastError() const;

		/// <summary>
		/// Clears the last error message, so the Lua state will not be considered to have any errors until the next time there's a script error.
		/// </summary>
		void ClearErrors();
#pragma endregion

	private:
		/// <summary>
		/// Gets a random integer between minInclusive and maxInclusive.
		/// </summary>
		/// <returns>A random integer between minInclusive and maxInclusive.</returns>
		int SelectRand(int minInclusive, int maxInclusive);

		/// <summary>
		/// Gets a random real between minInclusive and maxInclusive.
		/// </summary>
		/// <returns>A random real between minInclusive and maxInclusive.</returns>
		double RangeRand(double minInclusive, double maxInclusive);

		/// <summary>
		/// Gets a random number between -1 and 1.
		/// </summary>
		/// <returns>A random number between -1 and 1.</returns>
		double NormalRand();

		/// <summary>
		/// Gets a random number between 0 and 1.
		/// </summary>
		/// <returns>A random number between 0 and 1.</returns>
		double PosRand();

		/// <summary>
		/// Generates a string that describes the current state of the Lua stack, for debugging purposes.
		/// </summary>
		/// <returns>A string that describes the current state of the Lua stack.</returns>
		std::string DescribeLuaStack();

		/// <summary>
		/// Clears all the member variables of this LuaStateWrapper, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		lua_State *m_State;
		Entity *m_TempEntity; //!< Temporary holder for an Entity object that we want to pass into the Lua state without fuss. Lets you export objects to lua easily.
		std::vector<Entity *> m_TempEntityVector; //!< Temporary holder for a vector of Entities that we want to pass into the Lua state without a fuss. Usually used to pass arguments to special Lua functions.
		std::string m_LastError; //!< Description of the last error that occurred in the script execution.

		// This mutex is more for safety, and with new script/AI architecture we shouldn't ever be locking on a mutex. As such we use this primarily to fire asserts.
		std::recursive_mutex m_Mutex; //!< Mutex to ensure multiple threads aren't running something in this lua state simultaneously.

		// For determinism, every Lua state has it's own random number generator.
		RandomGenerator m_RandomGenerator; //!< The random number generator used for this lua state.
	};

	static constexpr int c_NumThreadedLuaStates = 8;
	typedef std::array<LuaStateWrapper, c_NumThreadedLuaStates> LuaStatesArray;

	/// <summary>
	/// The singleton manager of the master Lua script state.
	/// </summary>
	class LuaMan : public Singleton<LuaMan> {
		friend class SettingsMan;
		friend class LuaStateWrapper;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a LuaMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		LuaMan() { Clear(); }

		/// <summary>
		/// Makes the LuaMan object ready for use.
		/// </summary>
		void Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a LuaMan object before deletion from system memory.
		/// </summary>
		~LuaMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the LuaMan object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Lua State Handling
		/// <summary>
		/// Returns our master script state (where activies, global scripts etc run).
		/// </summary>
		/// <returns>The master script state.</returns>
		LuaStateWrapper & GetMasterScriptState();

		/// <summary>
		/// Returns our threaded script states which movable objects use.
		/// </summary>
		/// <returns>A list of threaded script states.</returns>
		LuaStatesArray & GetThreadedScriptStates();
		
		/// <summary>
		/// Gets the current thread lua state override that new objects created will be assigned to.
		/// </summary>
		/// <returns>The current lua state to force objects to be assigned to.</returns>
		LuaStateWrapper * GetThreadLuaStateOverride() const;

		/// <summary>
		/// Forces all new MOs created in this thread to be assigned to a particular lua state.
		/// This is to ensure that objects created in threaded Lua environments can be safely used.
		/// </summary>
		/// <param name="luaState">The lua state to force objects to be assigned to.</returns>
		void SetThreadLuaStateOverride(LuaStateWrapper* luaState);

		/// <summary>
		/// Gets the current thread lua state that is running.
		/// </summary>
		/// <returns>The current lua state that is running.</returns>
		LuaStateWrapper* GetThreadCurrentLuaState() const;

		/// <summary>
		/// Returns a free threaded script states to assign a movableobject to.
		/// This will be locked to our thread and safe to use - ensure that it'll be unlocked after use!
		/// </summary>
		/// <returns>A script state.</returns>
		LuaStateWrapper * GetAndLockFreeScriptState();

		/// <summary>
		/// Returns whether a script is safe to run in a multithreaded manner.
		/// </summary>
		/// <returns>Whether the script is thread-safe.</returns>
		bool IsScriptThreadSafe(const std::string &scriptPath);

		/// <summary>
		/// Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts().
		/// </summary>
		void ClearUserModuleCache();

		/// <summary>
		/// Adds a function to be called prior to executing lua scripts. This is used to callback into lua from other threads safely.
		/// </summary>
		/// <param name="callback">The callback function that will be executed.</returns>
		void AddLuaScriptCallback(const std::function<void()> &callback);

		/// <summary>
		/// Executes and clears all pending script callbacks.
		/// </summary>
		void ExecuteLuaScriptCallbacks();
#pragma endregion

#pragma region File I/O Handling
		/// <summary>
		/// Returns a vector of all the directories in relativeDirectory, which is relative to the working directory.
		/// Note that a call to this method overwrites any previously returned vector from DirectoryList() or FileList().
		/// </summary>
		/// <param name="relativeDirectory">Directory path relative to the working directory.</param>
		/// <returns>A vector of the directories in relativeDirectory.</returns>
		const std::vector<std::string> & DirectoryList(const std::string &relativeDirectory);

		/// <summary>
		/// Returns a vector of all the files in relativeDirectory, which is relative to the working directory.
		/// Note that a call to this method overwrites any previously returned vector from DirectoryList() or FileList().
		/// </summary>
		/// <param name="relativeDirectory">Directory path relative to the working directory.</param>
		/// <returns>A vector of the files in relativeDirectory.</returns>
		const std::vector<std::string> & FileList(const std::string &relativeDirectory);

		/// <summary>
		/// Opens a file or creates one if it does not exist, depending on access mode. You can open files only inside .rte folders in the working directly. You can't open more that c_MaxOpenFiles file simultaneously.
		/// On Linux will attempt to open a file case insensitively.
		/// </summary>
		/// <param name="filename">Path to the file. All paths are made absolute by adding current working directory to the specified path.</param>
		/// <param name="mode">File access mode. See 'fopen' for list of modes.</param>
		/// <returns>File index in the opened files array.</returns>
		int FileOpen(const std::string &fileName, const std::string &accessMode);

		/// <summary>
		/// Closes a previously opened file.
		/// </summary>
		/// <param name="fileIndex">File index in the opened files array.</param>
		void FileClose(int fileIndex);

		/// <summary>
		/// Closes all previously opened files.
		/// </summary>
		void FileCloseAll();

		/// <summary>
		/// Reads a line from a file.
		/// </summary>
		/// <param name="fileIndex">File index in the opened files array.</param>
		/// <returns>Line from file, or empty string on error.</returns>
		std::string FileReadLine(int fileIndex);

		/// <summary>
		/// Writes a text line to a file.
		/// </summary>
		/// <param name="fileIndex">File index in the opened files array.</param>
		/// <param name="line">String to write.</param>
		void FileWriteLine(int fileIndex, const std::string &line);

		/// <summary>
		/// Returns true if end of file was reached.
		/// </summary>
		/// <param name="fileIndex">File index in the opened files array.</param>
		/// <returns>Whether or not EOF was reached.</returns>
		bool FileEOF(int fileIndex);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this LuaMan.
		/// </summary>
		void Update();
#pragma endregion

	private:

		static constexpr int c_MaxOpenFiles = 10; //!< The maximum number of files that can be opened with FileOpen at runtime.
		static const std::unordered_set<std::string> c_FileAccessModes; //!< Valid file access modes when opening files with FileOpen.

		std::array<FILE *, c_MaxOpenFiles> m_OpenedFiles; //!< Internal list of opened files used by File functions.

		LuaStateWrapper m_MasterScriptState;
		LuaStatesArray m_ScriptStates;

		std::unordered_map<std::string, bool> m_ScriptThreadSafetyMap;

		std::vector<std::function<void()>> m_ScriptCallbacks; //!< A list of callback functions we'll trigger before processing lua scripts. This allows other threads (i.e pathing requests) to safely trigger callbacks in lua
		std::mutex m_ScriptCallbacksMutex; //!< Mutex to ensure multiple threads aren't modifying the script callback vector at the same time.

		int m_LastAssignedLuaState = 0;

		/// <summary>
		/// Clears all the member variables of this LuaMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		LuaMan(const LuaMan &reference) = delete;
		LuaMan & operator=(const LuaMan &rhs) = delete;
	};
}
#endif