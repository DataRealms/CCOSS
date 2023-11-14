// Make sure that this wrapper file is always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire cause luabind is a nightmare!

#include "LuabindObjectWrapper.h"
#include "luabind/object.hpp"

#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

// With multithreaded Lua, objects can be destructed from multiple threads at once
// This is okay, but LuaBind wants to do some management on the lua state when one of it's objects is deleted
// This means that potentially an object being deleted by one lua state actually exists in another lua state
// And upon deletion, it's unsafe for LuaBind to poke at the state until we're out the multithreaded context
// As such, we don't actually delete the object until we're in a safe environment outside the multithreaded parts
// Note - this is required even though we force objects in multithreaded environments to be within our Lua state
// This is because we may assign an object to another state in a singlethreaded context, before the GC runs in the multithreaded context
static std::vector<luabind::adl::object *> s_QueuedDeletions;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuabindObjectWrapper::ApplyQueuedDeletions() {
	for (luabind::adl::object *obj : s_QueuedDeletions) {
		delete obj;
	}

	s_QueuedDeletions.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LuabindObjectWrapper::~LuabindObjectWrapper() {
	if (m_OwnsObject) {
		static std::mutex mut;
		std::lock_guard<std::mutex> guard(mut);
		s_QueuedDeletions.push_back(m_LuabindObject);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

luabind::adl::object GetCopyForStateInternal(const luabind::adl::object& obj, lua_State& targetState) {
	if (obj.is_valid()) {
		int type = luabind::type(obj);
		if (type == LUA_TNUMBER) {
			return luabind::adl::object(&targetState, luabind::object_cast<double>(obj));
		}
		else if (type == LUA_TBOOLEAN) {
			return luabind::adl::object(&targetState, luabind::object_cast<bool>(obj));
		}
		else if (type == LUA_TSTRING) {
			return luabind::adl::object(&targetState, luabind::object_cast<std::string>(obj));
		}
		else if (type == LUA_TTABLE) {
			luabind::object table = luabind::newtable(&targetState);
			for (luabind::iterator itr(obj), itrEnd; itr != itrEnd; ++itr) {
				table[GetCopyForStateInternal(itr.key(), targetState)] = GetCopyForStateInternal(*itr, targetState);
			}
			return table;
		}
		else if (type == LUA_TUSERDATA) {
#define PER_LUA_BINDING(Type)																		\
			if (boost::optional<Type*> boundObject = luabind::object_cast_nothrow<Type*>(obj)) {	\
				return luabind::adl::object(&targetState, boundObject.get());						\
			}

			LIST_OF_LUABOUND_OBJECTS
#undef PER_LUA_BINDING
		}

	}

	// Dear god, I hope this is safe and equivalent to nil, because I can't find another way of doing it.
	return luabind::adl::object();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LuabindObjectWrapper LuabindObjectWrapper::GetCopyForState(lua_State& targetState) const {
	luabind::adl::object* copy = new luabind::adl::object(GetCopyForStateInternal(*m_LuabindObject, targetState));
	return LuabindObjectWrapper(copy, m_FilePath, true);
}

}