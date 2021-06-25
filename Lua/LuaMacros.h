#ifndef _RTELUAMACROS_
#define _RTELUAMACROS_

#include "luabind.hpp"
#include "operator.hpp"
#include "copy_policy.hpp"
#include "adopt_policy.hpp"
#include "out_value_policy.hpp"
#include "iterator_policy.hpp"
#include "return_reference_to_policy.hpp"

using namespace luabind;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="TYPE"></param>
	#define LuaBindingRegisterFunctionForType(TYPENAME) \
		static luabind::scope Register##TYPENAME##LuaBindings()

	/// <summary>
	/// 
	/// </summary>
	#define AbstractTypeLuaClassDefinition(TYPE, PARENTTYPE) \
		luabind::class_<TYPE, PARENTTYPE>(#TYPE) \
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// 
	/// </summary>
	#define ConcreteTypeLuaClassDefinition(TYPE, PARENTTYPE) \
		luabind::class_<TYPE, PARENTTYPE>(#TYPE) \
			.def("Clone", &Clone##TYPE, adopt(result)) \
			.property("ClassName", &TYPE::GetClassName)

	/// <summary>
	/// 
	/// </summary>
	#define RegisterLuaBindingsOfAbstractType(OWNINGSCOPENAME, TYPE) \
		def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&To##TYPE), \
		def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&ToConst##TYPE), \
		OWNINGSCOPENAME##::Register##TYPE##LuaBindings()

	/// <summary>
	/// 
	/// </summary>
	#define RegisterLuaBindingsOfConcreteType(OWNINGSCOPENAME, TYPE) \
		def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&Create##TYPE, adopt(result)), \
		def((std::string("Create") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&Create##TYPE, adopt(result)), \
		def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, int))&Random##TYPE, adopt(result)), \
		def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string, std::string))&Random##TYPE, adopt(result)), \
		def((std::string("Random") + std::string(#TYPE)).c_str(), (TYPE *(*)(std::string))&Random##TYPE, adopt(result)), \
		def((std::string("To") + std::string(#TYPE)).c_str(), (TYPE *(*)(Entity *))&To##TYPE), \
		def((std::string("To") + std::string(#TYPE)).c_str(), (const TYPE *(*)(const Entity *))&ToConst##TYPE),	\
		def((std::string("Is") + std::string(#TYPE)).c_str(), (bool(*)(const Entity *))&Is##TYPE),	\
		OWNINGSCOPENAME##::Register##TYPE##LuaBindings()


	/// <summary>
	/// These are expanded by the preprocessor to all the different cloning function definitions.
	/// </summary>
	#define LuaEntityCreate(TYPE) \
		TYPE * Create##TYPE(std::string preseName, std::string moduleName) { \
			const Entity *entityPreset = g_PresetMan.GetEntityPreset(#TYPE, preseName, moduleName); \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("ERROR: There is no ") + std::string(#TYPE) + std::string(" of the Preset name \"") + preseName + std::string("\" defined in the \"") + moduleName + std::string("\" Data Module!")); \
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		TYPE * Create##TYPE(std::string preset) { \
			return Create##TYPE(preset, "All"); \
		} \
		TYPE * Random##TYPE(std::string groupName, int moduleSpaceID) { \
			const Entity *entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, moduleSpaceID); \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("ERROR: Could not find any ") + std::string(#TYPE) + std::string(" defined in a Group called \"") + groupName + std::string("\" in module ") + g_PresetMan.GetDataModuleName(moduleSpaceID) + "!"); \
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		TYPE * Random##TYPE(std::string groupName, std::string dataModuleName) { \
			int moduleSpaceID = g_PresetMan.GetModuleID(dataModuleName); \
			const Entity *entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, moduleSpaceID); \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(groupName, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
			if (!entityPreset) { entityPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
			if (!entityPreset) { \
				g_ConsoleMan.PrintString(std::string("ERROR: Could not find any ") + std::string(#TYPE) + std::string(" defined in a Group called \"") + groupName + std::string("\" in module ") + dataModuleName + "!"); \
				return nullptr; \
			} \
			return dynamic_cast<TYPE *>(entityPreset->Clone()); \
		} \
		TYPE * Random##TYPE(std::string groupName) { \
			return Random##TYPE(groupName, "All"); \
		}

	/// <summary>
	/// Preprocessor helper function so we don't need to maintain a dozen almost identical definitions.
	/// </summary>
	#define LuaEntityClone(TYPE) \
		TYPE * Clone##TYPE(const TYPE *thisEntity) { \
			if (thisEntity) { \
				return dynamic_cast<TYPE *>(thisEntity->Clone()); \
			} \
			g_ConsoleMan.PrintString(std::string("ERROR: Tried to clone a ") + std::string(#TYPE) + std::string(" reference that is nil!")); \
			return nullptr; \
		}

	/// <summary>
	/// Preprocessor helper function so we don't need to maintain a dozen almost identical definitions.
	/// </summary>
	#define LuaEntityCast(TYPE) \
		TYPE * To##TYPE(Entity *entity) { \
			TYPE *targetType = dynamic_cast<TYPE *>(entity); \
			if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference!")); } \
			return targetType; \
		} \
		const TYPE * ToConst##TYPE(const Entity *entity) { \
			const TYPE *targetType = dynamic_cast<const TYPE *>(entity); \
			if (!targetType) { g_ConsoleMan.PrintString(std::string("ERROR: Tried to convert a non-") + std::string(#TYPE) + std::string(" Entity reference to an ") + std::string(#TYPE) + std::string(" reference!")); } \
			return targetType; \
		} \
		bool Is##TYPE(Entity *entity) { \
			return dynamic_cast<TYPE *>(entity) ? true : false; \
		}

	/// <summary>
	/// Special handling for passing ownership through properties. If you try to pass null to this normally, luajit crashes.
	/// This handling avoids that, and is a bit safer since there's no actual ownership transfer from Lua to C++.
	/// </summary>
	#define LuaPropertyOwnershipSafetyFaker(OBJECTTYPE, PROPERTYTYPE, SETTERFUNCTION) \
		void OBJECTTYPE##SETTERFUNCTION(OBJECTTYPE *luaSelfObject, PROPERTYTYPE *objectToSet) { \
			luaSelfObject->SETTERFUNCTION(objectToSet ? dynamic_cast<PROPERTYTYPE *>(objectToSet->Clone()) : nullptr); \
		}

#endif