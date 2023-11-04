#ifndef _RTELUABINDOBJECTWRAPPER_
#define _RTELUABINDOBJECTWRAPPER_

namespace luabind::adl {
	class object;
}

namespace RTE {

#pragma region Global Macro Definitions
#define ScriptFunctionNames(...) \
        virtual std::vector<std::string> GetSupportedScriptFunctionNames() const { return {__VA_ARGS__}; }

#define AddScriptFunctionNames(PARENT, ...) \
        std::vector<std::string> GetSupportedScriptFunctionNames() const override { \
            std::vector<std::string> functionNames = PARENT::GetSupportedScriptFunctionNames(); \
            functionNames.insert(functionNames.end(), {__VA_ARGS__}); \
            return functionNames; \
        }
#pragma endregion

	/// <summary>
	/// A wrapper for luabind objects, to avoid include problems with luabind.
	/// </summary>
	class LuabindObjectWrapper {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used for LuabindObjectWrapper.
		/// </summary>
		LuabindObjectWrapper() = default;

		/// <summary>
		/// Constructor method used to instantiate a LuabindObjectWrapper object in system memory.
		/// </summary>
		explicit LuabindObjectWrapper(luabind::adl::object* luabindObject, const std::string_view& filePath, bool ownsObject = true) : m_LuabindObject(luabindObject), m_FilePath(filePath), m_OwnsObject(ownsObject) {}
#pragma endregion

#pragma region Destruction
		static void ApplyQueuedDeletions();

		/// <summary>
		/// Destructor method used to clean up a LuabindObjectWrapper object before deletion from system memory.
		/// </summary>
		~LuabindObjectWrapper();
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the LuabindObjectWrapper's luabind object. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The LuabindObjectWrapper's luabind object.</returns>
		luabind::adl::object * GetLuabindObject() const { return m_LuabindObject; }

		/// <summary>
		/// Gets the LuabindObjectWrapper's file path.
		/// </summary>
		/// <returns>The LuabindObjectWrapper's file path.</returns>
		const std::string & GetFilePath() const { return m_FilePath; }
#pragma endregion

	private:

		bool m_OwnsObject; //!< Whether or not we own the luabind object this is wrapping.
		luabind::adl::object *m_LuabindObject; //!< The luabind object this is wrapping.
		std::string m_FilePath; //!< The filepath the wrapped luabind object represents, if it's a function.

		// Disallow the use of some implicit methods.
		LuabindObjectWrapper(const LuabindObjectWrapper &reference) = delete;
		LuabindObjectWrapper &operator=(const LuabindObjectWrapper &rhs) = delete;
	};
}
#endif