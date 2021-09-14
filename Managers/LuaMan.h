#ifndef _RTELUAMAN_
#define _RTELUAMAN_

#include "Singleton.h"
#include "Serializable.h"
#include "Entity.h"

#define g_LuaMan LuaMan::Instance()

struct lua_State;

namespace RTE {

	/// <summary>
	/// The singleton manager of the master Lua script state.
	/// </summary>
	class LuaMan : public Singleton<LuaMan> {
		friend class SettingsMan;

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

		/// <summary>
		/// Clears internal Lua package tables from all user-defined modules. Those must be reloaded with ReloadAllScripts().
		/// </summary>
		void ClearUserModuleCache();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Returns an ID string unique to this runtime for use by original presets that have scripts associated with them.
		/// </summary>
		/// <returns>Returns the unique ID as a string.</returns>
		std::string GetNewPresetID();

		/// <summary>
		/// Returns an ID string unique to this runtime for use by individual objects that are also tracked in the Lua state and have scripts associated with them.
		/// </summary>
		/// <returns>Returns the unique ID as a string.</returns>
		std::string GetNewObjectID();

		/// <summary>
		/// Gets a temporary Entity that can be accessed in the Lua state.
		/// </summary>
		/// <returns>The temporary entity. Ownership is NOT transferred!</returns>
		Entity * GetTempEntity() const { return m_TempEntity; }

		/// <summary>
		/// Sets a temporary Entity that can be accessed in the Lua state.
		/// </summary>
		/// <param name="entity">The temporary entity. Ownership is NOT transferred!</param>
		void SetTempEntity(Entity *entity) { m_TempEntity = entity; }

		/// <summary>
		/// Sets a temporary vector of entities that can be accessed in the Lua state.
		/// </summary>
		/// <param name="entityVector">The temporary vector of entities. Ownership is NOT transferred!</param>
		void SetTempEntityVector(std::vector<Entity *> entityVector) { m_TempEntityVector = entityVector; }
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
		/// <param name="functionLiteralArguments">Optional vector of strings that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
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
#pragma endregion

#pragma region Error Handling
		/// <summary>
		/// Tells whether there are any errors reported waiting to be read.
		/// </summary>
		/// <returns>Whether errors exist.</returns>
		bool ErrorExists() const { return !m_LastError.empty(); }

		/// <summary>
		/// Returns the last error message from executing scripts.
		/// </summary>
		/// <returns>The error string with hopefully meaningful info about what went wrong.</returns>
		std::string GetLastError() const { return m_LastError; }

		/// <summary>
		/// Clears the last error message, so the Lua state will not be considered to have any errors until the next time there's a script error.
		/// </summary>
		void ClearErrors() { m_LastError.clear(); }
#pragma endregion

#pragma region File I/O Handling
		/// <summary>
		/// Opens a file. You can open files only inside .rte folders in the working directly. You can't open more that c_MaxOpenFiles file simultaneously.
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
		void Update() const;
#pragma endregion

	private:

		static constexpr int c_MaxOpenFiles = 10; //!< The maximum number of files that can be opened with FileOpen at runtime.

		lua_State *m_MasterState; //!< The master parent script state.

		bool m_DisableLuaJIT; //!< Whether to disable LuaJIT or not. Disabling will skip loading the JIT library entirely as just setting 'jit.off()' seems to have no visible effect.

		std::string m_LastError; //!< Description of the last error that occurred in the script execution.

		long m_NextPresetID; //!< The next unique preset ID to hand out to the next Preset that wants to define some functions. This gets incremented each time a new one is requested to give unique ID's to all original presets.
		long m_NextObjectID; //!< The next unique object ID to hand out to the next scripted Entity instance that wants to run its preset's scripts. This gets incremented each time a new one is requested to give unique ID's to all scripted objects.
		Entity *m_TempEntity; //!< Temporary holder for an Entity object that we want to pass into the Lua state without fuss. Lets you export objects to lua easily.
		std::vector<Entity *> m_TempEntityVector; //!< Temporary holder for a vector of Entities that we want to pass into the Lua state without a fuss. Usually used to pass arguments to special Lua functions.

		std::array<FILE *, c_MaxOpenFiles> m_OpenedFiles; //!< Internal list of opened files used by File functions.

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