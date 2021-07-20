#ifndef _RTELUABINDDEFINITIONS_
#define _RTELUABINDDEFINITIONS_

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include "lua.hpp"
#include "luabind.hpp"
#include "operator.hpp"
#include "copy_policy.hpp"
#include "adopt_policy.hpp"
#include "out_value_policy.hpp"
#include "iterator_policy.hpp"
#include "return_reference_to_policy.hpp"

namespace luabind {
	/// <summary>
	/// Function that extracts the raw pointer from the smart pointer. This is needed when Lua calls member functions on held types, the 'this' pointer must be a raw pointer, it is also needed to allow the smart_pointer to raw_pointer conversion from Lua to C++.
	/// </summary>
	/// <param name="ptr">The smart pointer to get raw pointer for.</param>
	/// <returns>Raw pointer of the passed in smart pointer.</returns>
	template<class Type> Type * get_pointer(boost::shared_ptr<Type> &ptr) {
		return ptr.get();
	}

	/// <summary>
	/// Can't have global enums in the master state so we use this dummy struct as a class and register the enums under it.
	/// </summary>
	struct enum_wrapper {};
}

namespace RTE {
	/// <summary>
	/// Derived structs for each of the input enums because we can't register enum_wrapper multiple times under a different name.
	/// We're doing this so we can access each enum separately by name rather than having all of them accessed from a shared name.
	/// If this proves to be a hassle then we can easily revert to the shared name access by registering everything under enum_wrapper.
	/// </summary>
	struct input_device : public luabind::enum_wrapper {};
	struct input_elements : public luabind::enum_wrapper {};
	struct mouse_buttons : public luabind::enum_wrapper {};
	struct joy_buttons : public luabind::enum_wrapper {};
	struct joy_directions : public luabind::enum_wrapper {};

	/// <summary>
	/// Special callback function for adding file name and line number to error messages when calling functions incorrectly.
	/// </summary>
	/// <param name="luaState">The Lua master state.</param>
	/// <returns>An error signal, 1, so Lua correctly reports that there's been an error.</returns>
	static int AddFileAndLineToError(lua_State *luaState) {
		lua_Debug luaDebug;
		if (lua_getstack(luaState, 2, &luaDebug) > 0) {
			lua_getinfo(luaState, "Sln", &luaDebug);
			std::string errorString = lua_tostring(luaState, -1);

			if (errorString.find(".lua") != std::string::npos) {
				lua_pushstring(luaState, errorString.c_str());
			} else {
				std::stringstream messageStream;
				messageStream << ((luaDebug.name == nullptr || strstr(luaDebug.name, ".rte") == nullptr) ? luaDebug.short_src : luaDebug.name);
				messageStream << ":" << luaDebug.currentline << ": " << errorString;
				lua_pushstring(luaState, messageStream.str().c_str());
			}
		}
		return 1;
	}
}
#endif