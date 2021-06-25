#ifndef _RTELUAMACROS_
#define _RTELUAMACROS_

//namespace RTE {


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

	//////////////////////////////////////////////////////////////////////////////////////////
	// Preset clone adapters that will return the exact pre-cast types so we don't have to do:
	// myNewActor = ToActor(PresetMan:GetPreset("AHuman", "Soldier Light", "All")):Clone()
	// but can instead do:
	// myNewActor = CreateActor("Soldier Light", "All");
	// or even:
	// myNewActor = CreateActor("Soldier Light");
	// or for a randomly selected Preset within a group:
	// myNewActor = RandomActor("Light Troops");

#define LUAENTITYCREATE(TYPE) \
    TYPE * Create##TYPE(std::string preset, std::string module) { \
        const Entity *pPreset = g_PresetMan.GetEntityPreset(#TYPE, preset, module); \
        if (!pPreset) { \
            g_ConsoleMan.PrintString(string("ERROR: There is no ") + string(#TYPE) + string(" of the Preset name \"") + preset + string("\" defined in the \"") + module + string("\" Data Module!")); \
            return 0; \
        } \
        return dynamic_cast<TYPE *>(pPreset->Clone()); \
    } \
	\
    TYPE * Create##TYPE(std::string preset) { \
		return Create##TYPE(preset, "All"); \
	} \
	\
    TYPE * Random##TYPE(std::string group, int moduleSpaceID) { \
        const Entity *pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(group, #TYPE, moduleSpaceID); \
        if (!pPreset) { pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(group, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
        if (!pPreset) { pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
        if (!pPreset) { \
            g_ConsoleMan.PrintString(string("ERROR: Could not find any ") + string(#TYPE) + string(" defined in a Group called \"") + group + string("\" in module ") + g_PresetMan.GetDataModuleName(moduleSpaceID) + string("!")); \
            return 0; \
        } \
        return dynamic_cast<TYPE *>(pPreset->Clone()); \
    } \
	\
    TYPE * Random##TYPE(std::string group, std::string module) { \
        int moduleSpaceID = g_PresetMan.GetModuleID(module); \
        const Entity *pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(group, #TYPE, moduleSpaceID); \
        if (!pPreset) { pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech(group, #TYPE, g_PresetMan.GetModuleID("Base.rte")); } \
        if (!pPreset) { pPreset = g_PresetMan.GetRandomBuyableOfGroupFromTech("Any", #TYPE, moduleSpaceID); } \
        if (!pPreset) { \
            g_ConsoleMan.PrintString(string("ERROR: Could not find any ") + string(#TYPE) + string(" defined in a Group called \"") + group + string("\" in module ") + module + string("!")); \
            return 0; \
        } \
        return dynamic_cast<TYPE *>(pPreset->Clone()); \
    } \
	\
    TYPE * Random##TYPE(std::string group) { \
		return Random##TYPE(group, "All"); \
	}





// Preprocessor helper function so we don't need to maintain a dozen almost identical definitions
#define LUAENTITYCLONE(TYPE)																													\
	TYPE * Clone##TYPE(const TYPE *thisEntity) {																								\
		if (thisEntity) {																														\
			return dynamic_cast<TYPE *>(thisEntity->Clone());																					\
		} else {																																\
			g_ConsoleMan.PrintString(std::string("ERROR: Tried to clone a ") + std::string(#TYPE) + std::string(" reference that is nil!"));	\
		}																																		\
		return 0;																																\
	}


// Preprocessor helper function so we don't need to maintain a dozen almost identical definitions
#define LUAENTITYCAST(TYPE) \
    TYPE * To##TYPE(Entity *pEntity) \
    { \
        TYPE *pTarget = dynamic_cast<TYPE *>(pEntity); \
        if (!pTarget) \
            g_ConsoleMan.PrintString(string("ERROR: Tried to convert a non-") + string(#TYPE) + string(" Entity reference to an ") + string(#TYPE) + string(" reference!")); \
        return pTarget; \
    } \
    const TYPE * ToConst##TYPE(const Entity *pEntity) \
    { \
        const TYPE *pTarget = dynamic_cast<const TYPE *>(pEntity); \
        if (!pTarget) \
            g_ConsoleMan.PrintString(string("ERROR: Tried to convert a non-") + string(#TYPE) + string(" Entity reference to an ") + string(#TYPE) + string(" reference!")); \
        return pTarget; \
    } \
    bool Is##TYPE(Entity *pEntity) { return dynamic_cast<TYPE *>(pEntity) ? true : false; }




/// <summary>
/// Special handling for passing ownership through properties. If you try to pass null to this normally, luajit crashes.
/// This handling avoids that, and is a bit safer since there's no actual ownership transfer from Lua to C++.
/// </summary>
#define PROPERTYOWNERSHIPSAFETYFAKER(OBJECTTYPE, PROPERTYTYPE, SETTERFUNCTION) \
    void OBJECTTYPE##SETTERFUNCTION(OBJECTTYPE *luaSelfObject, PROPERTYTYPE *objectToSet) { \
        if (objectToSet) { \
            luaSelfObject->SETTERFUNCTION(dynamic_cast<PROPERTYTYPE *>(objectToSet->Clone())); \
        } else { \
            luaSelfObject->SETTERFUNCTION(nullptr); \
        } \
    } \

//}
#endif