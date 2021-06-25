#include "LuaMan.h"


#include "lua.hpp"


/// <summary>
/// Special callback function for adding file name and line number to error messages when calling functions incorrectly.
/// </summary>
/// <param name="pState">The Lua master state.</param>
/// <returns>An error signal, 1, so Lua correctly reports that there's been an error.</returns>
int AddFileAndLineToError(lua_State* pState) {
    lua_Debug luaDebug;
    if (lua_getstack(pState, 2, &luaDebug) > 0) {
        lua_getinfo(pState, "Sln", &luaDebug);
        std::string errorString = lua_tostring(pState, -1);

        if (errorString.find(".lua") != std::string::npos) {
            lua_pushstring(pState, errorString.c_str());
        } else {
            std::stringstream messageStream;
            messageStream << ((luaDebug.name == nullptr || strstr(luaDebug.name, ".rte") == nullptr) ? luaDebug.short_src : luaDebug.name);
            messageStream << ":" << luaDebug.currentline << ": " << errorString;
            lua_pushstring(pState, messageStream.str().c_str());
        }
    }
   return 1;
}






namespace RTE {






//////////////////////////////////////////////////////////////////////////////////////////
// Explicit deletion of any Entity instance that Lua owns.. it will probably be handled
// by the GC, but this makes it instantaneous

void DeleteEntity(Entity *pEntity)
{
    delete pEntity;
    pEntity = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Other misc adapters to eliminate/emulate default parameters etc


double NormalRand() { return RandomNormalNum<double>(); }
double PosRand() { return RandomNum<double>(); }



/*
//////////////////////////////////////////////////////////////////////////////////////////
// Wrapper for the GAScripted so we can derive new classes from it purely in lua:
//
// "It is also possible to derive Lua classes from C++ classes, and override virtual functions with Lua functions.
// To do this we have to create a wrapper class for our C++ base class.
// This is the class that will hold the Lua object when we instantiate a Lua class"

struct GAScriptedWrapper:
    GAScripted,
    wrap_base
{
    GAScriptedWrapper(): GAScripted() { ; }

    // Passing in the path of the script file that defines, in Lua, the GAScripted-derived class
//    virtual int Create(const GAScripted &reference) { return call<int>("Create", reference); }
    virtual Entity * Clone(Entity *pCloneTo = 0) const { return call<Entity *>("Clone", pCloneTo); }
    virtual int Start() { call<int>("Start"); }
    virtual void Pause(bool pause) { call<void>("Pause", pause); }
    virtual void End() { call<void>("End"); }
    virtual void Update() { call<void>("Update"); }

//    static int static_Create(GAScripted *pSelf, const GAScripted &reference) { return pSelf->GAScripted::Create(reference); }
    static Entity * static_Clone(GAScripted *pSelf, Entity *pCloneTo = 0) { return pSelf->GAScripted::Clone(pCloneTo); }
    static int static_Start(GAScripted *pSelf) { return pSelf->GAScripted::Start(); }
    static void static_Pause(GAScripted *pSelf, bool pause) { return pSelf->GAScripted::Pause(pause); }
    static void static_End(GAScripted *pSelf) { return pSelf->GAScripted::End(); }
    static void static_Update(GAScripted *pSelf) { return pSelf->GAScripted::Update(); }
};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Clear()
{
    m_pMasterState = 0;
    m_LastError.clear();
// TODO: is this the best way to give ID's.. won't ever be reset?
    m_NextPresetID = 0;
    m_NextObjectID = 0;
    m_pTempEntity = 0;
    m_TempEntityVector.clear();
    m_TempEntityVector.shrink_to_fit();

	//Clear files list
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		m_Files[i] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::Initialize() {
    m_pMasterState = luaL_newstate();
    // Attach the master state to LuaBind
    luabind::open(m_pMasterState);

	const luaL_Reg libsToLoad[] = {
		{ LUA_COLIBNAME, luaopen_base },
		{ LUA_LOADLIBNAME, luaopen_package },
		{ LUA_TABLIBNAME, luaopen_table },
		{ LUA_STRLIBNAME, luaopen_string },
		{ LUA_MATHLIBNAME, luaopen_math },
		{ LUA_DBLIBNAME, luaopen_debug },
		{ LUA_JITLIBNAME, luaopen_jit },
		{ NULL, NULL } // End of array
	};

	for (const luaL_Reg *lib = libsToLoad; lib->func; lib++) {
		lua_pushcfunction(m_pMasterState, lib->func);
		lua_pushstring(m_pMasterState, lib->name);
		lua_call(m_pMasterState, 1, 0);
	}

	// LuaJIT should start automatically after we load the library but we're making sure it did anyway.
	if (!luaJIT_setmode(m_pMasterState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) { RTEAbort("Failed to initialize LuaJIT!"); }

    // From LuaBind documentation:
    // As mentioned in the Lua documentation, it is possible to pass an error handler function to lua_pcall().
    // Luabind makes use of lua_pcall() internally when calling member functions and free functions.
    // It is possible to set the error handler function that Luabind will use globally:
    //set_pcall_callback(&AddFileAndLineToError); //NOTE: This seems to do nothing

    // Register all relevant bindings to the master state. Note that the order of registration is important, as bindings can't derive from an unregistered type (inheritance and all that).
	luabind::module(m_pMasterState) [
		SystemLuaBindings::RegisterVectorLuaBindings(),
		SystemLuaBindings::RegisterBoxLuaBindings(),
		EntityLuaBindings::RegisterSceneAreaLuaBindings(),
		EntityLuaBindings::RegisterEntityLuaBindings(),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, SoundContainer),
		EntityLuaBindings::RegisterSoundSetLuaBindings(),
		EntityLuaBindings::RegisterLimbPathLuaBindings(),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, SceneObject),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, MovableObject),
		EntityLuaBindings::RegisterMaterialLuaBindings(),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOPixel),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, TerrainObject),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, MOSprite),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOSParticle),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOSRotating),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Attachable),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, Emission),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, AEmitter),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, PEmitter),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Actor),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, ADoor),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Arm),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Leg),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, AHuman),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, ACrab),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Turret),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, ACraft),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, ACDropShip),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, ACRocket),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, HeldDevice),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Magazine),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Round),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, HDFirearm),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, ThrownDevice),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, TDExplosive),
		SystemLuaBindings::RegisterControllerLuaBindings(),
		SystemLuaBindings::RegisterTimerLuaBindings(),
		RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Scene),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, Deployment),
		SystemLuaBindings::RegisterDataModuleLuaBindings(),
		ActivityLuaBindings::RegisterActivityLuaBindings(),
		RegisterLuaBindingsOfAbstractType(ActivityLuaBindings, GameActivity),
		SystemLuaBindings::RegisterPieSliceLuaBindings(),
		RegisterLuaBindingsOfAbstractType(EntityLuaBindings, GlobalScript),
		EntityLuaBindings::RegisterMetaPlayerLuaBindings(),
		GUILuaBindings::RegisterGUIBannerLuaBindings(),
		GUILuaBindings::RegisterBuyMenuGUILuaBindings(),
		GUILuaBindings::RegisterSceneEditorGUILuaBindings(),
		ManagerLuaBindings::RegisterActivityManLuaBindings(),
		ManagerLuaBindings::RegisterAudioManLuaBindings(),
		ManagerLuaBindings::RegisterConsoleManLuaBindings(),
		ManagerLuaBindings::RegisterFrameManLuaBindings(),
		ManagerLuaBindings::RegisterMetaManLuaBindings(),
		ManagerLuaBindings::RegisterMovableManLuaBindings(),
		ManagerLuaBindings::RegisterPostProcessManLuaBindings(),
		ManagerLuaBindings::RegisterPresetManLuaBindings(),
		ManagerLuaBindings::RegisterPrimitiveManLuaBindings(),
		ManagerLuaBindings::RegisterSceneManLuaBindings(),
		ManagerLuaBindings::RegisterSettingsManLuaBindings(),
		ManagerLuaBindings::RegisterTimerManLuaBindings(),
		ManagerLuaBindings::RegisterUInputManLuaBindings(),

		luabind::class_<AlarmEvent>("AlarmEvent")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const Vector &, int, float>())
			.def_readwrite("ScenePos", &AlarmEvent::m_ScenePos)
			.def_readwrite("Team", &AlarmEvent::m_Team)
			.def_readwrite("Range", &AlarmEvent::m_Range),

		luabind::class_<LuaMan>("LuaManager")
			.property("TempEntity", &LuaMan::GetTempEntity)
			.def_readonly("TempEntities", &LuaMan::m_TempEntityVector, return_stl_iterator)
			.def("FileOpen", &LuaMan::FileOpen)
			.def("FileClose", &LuaMan::FileClose)
			.def("FileReadLine", &LuaMan::FileReadLine)
			.def("FileWriteLine", &LuaMan::FileWriteLine)
			.def("FileEOF", &LuaMan::FileEOF),

        // NOT a member function, so adopting _1 instead of the _2 for the first param, since there's no "this" pointer!!
        def("DeleteEntity", &DeleteEntity, adopt(_1)),
		def("RangeRand", (double(*)(double, double))& RandomNum),
		def("PosRand", &PosRand),
        def("NormalRand", &NormalRand),
        def("SelectRand", (int(*)(int, int)) &RandomNum),
        def("LERP", &LERP),
        def("EaseIn", &EaseIn),
        def("EaseOut", &EaseOut),
        def("EaseInOut", &EaseInOut),
        def("Clamp", &Limit),
		def("GetPPM", &GetPPM),
		def("GetMPP", &GetMPP),
		def("GetPPL", &GetPPL),
		def("GetLPP", &GetLPP),
		def("RoundFloatToPrecision", &RoundFloatToPrecision),

		class_<enum_wrapper::input_device>("InputDevice")
			.enum_("InputDevice")[
				value("DEVICE_KEYB_ONLY", InputDevice::DEVICE_KEYB_ONLY),
				value("DEVICE_MOUSE_KEYB", InputDevice::DEVICE_MOUSE_KEYB),
				value("DEVICE_GAMEPAD_1", InputDevice::DEVICE_GAMEPAD_1),
				value("DEVICE_GAMEPAD_2", InputDevice::DEVICE_GAMEPAD_2),
				value("DEVICE_GAMEPAD_3", InputDevice::DEVICE_GAMEPAD_3),
				value("DEVICE_GAMEPAD_4", InputDevice::DEVICE_GAMEPAD_4),
				value("DEVICE_COUNT", InputDevice::DEVICE_COUNT)
			],

		class_<enum_wrapper::input_elements>("InputElements")
			.enum_("InputElements")[
				value("INPUT_L_UP", InputElements::INPUT_L_UP),
				value("INPUT_L_DOWN", InputElements::INPUT_L_DOWN),
				value("INPUT_L_LEFT", InputElements::INPUT_L_LEFT),
				value("INPUT_L_RIGHT", InputElements::INPUT_L_RIGHT),
				value("INPUT_R_UP", InputElements::INPUT_R_UP),
				value("INPUT_R_DOWN", InputElements::INPUT_R_DOWN),
				value("INPUT_R_LEFT", InputElements::INPUT_R_LEFT),
				value("INPUT_R_RIGHT", InputElements::INPUT_R_RIGHT),
				value("INPUT_FIRE", InputElements::INPUT_FIRE),
				value("INPUT_AIM", InputElements::INPUT_AIM),
				value("INPUT_AIM_UP", InputElements::INPUT_AIM_UP),
				value("INPUT_AIM_DOWN", InputElements::INPUT_AIM_DOWN),
				value("INPUT_AIM_LEFT", InputElements::INPUT_AIM_LEFT),
				value("INPUT_AIM_RIGHT", InputElements::INPUT_AIM_RIGHT),
				value("INPUT_PIEMENU", InputElements::INPUT_PIEMENU),
				value("INPUT_JUMP", InputElements::INPUT_JUMP),
				value("INPUT_CROUCH", InputElements::INPUT_CROUCH),
				value("INPUT_NEXT", InputElements::INPUT_NEXT),
				value("INPUT_PREV", InputElements::INPUT_PREV),
				value("INPUT_START", InputElements::INPUT_START),
				value("INPUT_BACK", InputElements::INPUT_BACK),
				value("INPUT_COUNT", InputElements::INPUT_COUNT)
			],

		class_<enum_wrapper::mouse_buttons>("MouseButtons")
			.enum_("MouseButtons")[
				value("MOUSE_NONE", MouseButtons::MOUSE_NONE),
				value("MOUSE_LEFT", MouseButtons::MOUSE_LEFT),
				value("MOUSE_RIGHT", MouseButtons::MOUSE_RIGHT),
				value("MOUSE_MIDDLE", MouseButtons::MOUSE_MIDDLE),
				value("MAX_MOUSE_BUTTONS", MouseButtons::MAX_MOUSE_BUTTONS)
			],

		class_<enum_wrapper::joy_buttons>("JoyButtons")
			.enum_("JoyButtons")[
				value("JOY_NONE", JoyButtons::JOY_NONE),
				value("JOY_1", JoyButtons::JOY_1),
				value("JOY_2", JoyButtons::JOY_2),
				value("JOY_3", JoyButtons::JOY_3),
				value("JOY_4", JoyButtons::JOY_4),
				value("JOY_5", JoyButtons::JOY_5),
				value("JOY_6", JoyButtons::JOY_6),
				value("JOY_7", JoyButtons::JOY_7),
				value("JOY_8", JoyButtons::JOY_8),
				value("JOY_9", JoyButtons::JOY_9),
				value("JOY_10", JoyButtons::JOY_10),
				value("JOY_11", JoyButtons::JOY_11),
				value("JOY_12", JoyButtons::JOY_12),
				value("MAX_JOY_BUTTONS", JoyButtons::MAX_JOY_BUTTONS)
			],

		class_<enum_wrapper::joy_directions>("JoyDirections")
			.enum_("JoyDirections")[
				value("JOYDIR_ONE", JoyDirections::JOYDIR_ONE),
				value("JOYDIR_TWO", JoyDirections::JOYDIR_TWO)
			]
    ];

    // Assign the manager instances to globals in the lua master state
    globals(m_pMasterState)["TimerMan"] = &g_TimerMan;
    globals(m_pMasterState)["FrameMan"] = &g_FrameMan;
	globals(m_pMasterState)["PostProcessMan"] = &g_PostProcessMan;
	globals(m_pMasterState)["PrimitiveMan"] = &g_PrimitiveMan;
    globals(m_pMasterState)["PresetMan"] = &g_PresetMan;
    globals(m_pMasterState)["AudioMan"] = &g_AudioMan;
    globals(m_pMasterState)["UInputMan"] = &g_UInputMan;
    globals(m_pMasterState)["SceneMan"] = &g_SceneMan;
    globals(m_pMasterState)["ActivityMan"] = &g_ActivityMan;
    globals(m_pMasterState)["MetaMan"] = &g_MetaMan;
    globals(m_pMasterState)["MovableMan"] = &g_MovableMan;
    globals(m_pMasterState)["ConsoleMan"] = &g_ConsoleMan;
    globals(m_pMasterState)["LuaMan"] = &g_LuaMan;
    globals(m_pMasterState)["SettingsMan"] = &g_SettingsMan;

    luaL_dostring(m_pMasterState,
        // Override print() in the lua state to output to the console
        "print = function(toPrint) ConsoleMan:PrintString(\"PRINT: \" .. tostring(toPrint)); end;\n"
        // Add cls() as a shorcut to ConsoleMan:Clear()
        "cls = function() ConsoleMan:Clear(); end;"
        // Add package path to the defaults
        "package.path = package.path .. \";Base.rte/?.lua\";\n"
    );

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::ClearUserModuleCache()
{
	luaL_dostring(m_pMasterState, "for m, n in pairs(package.loaded) do if type(n) == \"boolean\" then package.loaded[m] = nil end end");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Destroy()
{
    lua_close(m_pMasterState);

	//Close all opened files
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		FileClose(i);

    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::SavePointerAsGlobal(void *pToSave, string globalName)
{
    // Push the pointer onto the Lua stack
    lua_pushlightuserdata(m_pMasterState, pToSave);
    // Pop and assign that pointer to a global var in the Lua state
    lua_setglobal(m_pMasterState, globalName.c_str());

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::GlobalIsDefined(string globalName)
{
    // Get the var you want onto the stack so we can check it
    lua_getglobal(m_pMasterState, globalName.c_str());
    // Now report if it is nil/null or not
    bool isDefined = !lua_isnil(m_pMasterState, -1);
    // Pop the var so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 1);

    return isDefined;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::TableEntryIsDefined(string tableName, string indexName)
{
    // Push the table onto the stack, checking if it even exists
    lua_getglobal(m_pMasterState, tableName.c_str());
    if (!lua_istable(m_pMasterState, -1))
    {
//        error(m_pMasterState, tableName + " is not a table when checking for the " + indexName + " within it.");
        // Clean up and report that there was nothing properly defined here
        lua_pop(m_pMasterState, 1);
        return false;
    }

    // Push the value at the requested index onto the stack so we can check if it's anything
    lua_getfield(m_pMasterState, -1, indexName.c_str());
    // Now report if it is nil/null or not
    bool isDefined = !lua_isnil(m_pMasterState, -1);
    // Pop both the var and the table so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 2);

    return isDefined;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::ExpressionIsTrue(string expression, bool consoleErrors)
{
    if (expression.empty())
        return false;

    bool result = false;

    try
    {
        // Push the script string onto the stack so we can execute it, and then actually try to run it
        // Assign the result to a dedicated temp global variable
        if (luaL_dostring(m_pMasterState, (string("ExpressionResult = ") + expression + string(";")).c_str()))
        {
            // Retrieve and pop the error message off the stack
            m_LastError = string("When evaluating Lua expression: ") + lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors)
            {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            return false;
        }
    }
    catch(const std::exception &e)
    {
        m_LastError = string("When evaluating Lua expression: ") + e.what();
        if (consoleErrors)
        {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        return false;
    }

    // Get the result var onto the stack so we can check it
    lua_getglobal(m_pMasterState, "ExpressionResult");
    // Now report if it is nil/null or not
    result = lua_toboolean(m_pMasterState, -1);
    // Pop the result var so this operation is balanced and leaves the stack as it was
    lua_pop(m_pMasterState, 1);

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptedFunction(const std::string &functionName, const std::string &selfObjectName, std::vector<std::string> variablesToSafetyCheck, std::vector<Entity *> functionEntityArguments, std::vector<std::string> functionLiteralArguments) {
    std::string scriptString = "";
    if (!variablesToSafetyCheck.empty()) {
        scriptString += "if ";
        for (const std::string &variableToSafetyCheck : variablesToSafetyCheck) {
            if (&variableToSafetyCheck != &variablesToSafetyCheck[0]) {
                scriptString += " and ";
            }
            scriptString += variableToSafetyCheck;
        }
        scriptString += " then ";
    }
    if (!functionEntityArguments.empty()) {
        scriptString += "local entityArguments = LuaMan.TempEntities; ";
    }
    scriptString += functionName + "(" + selfObjectName;
    if (!functionEntityArguments.empty()) {
        g_LuaMan.SetTempEntityVector(functionEntityArguments);
        for (const Entity *functionEntityArgument : functionEntityArguments) {
            scriptString += ", (To" + functionEntityArgument->GetClassName() + " and To" + functionEntityArgument->GetClassName() + "(entityArguments()) or entityArguments())";
        }
    }
    if (!functionLiteralArguments.empty()) {
        for (const std::string functionLiteralArgument : functionLiteralArguments) {
            scriptString += ", " + functionLiteralArgument;
        }
    }
    scriptString += ");";

    if (!variablesToSafetyCheck.empty()) { scriptString += " end;"; }
    
    return RunScriptString(scriptString);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptString(const std::string &scriptString, bool consoleErrors) {
    if (scriptString.empty()) {
        return -1;
    }
    int error = 0;

    lua_pushcfunction(m_pMasterState, &AddFileAndLineToError);
    try {
        // Load the script string onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
        if (luaL_loadstring(m_pMasterState, scriptString.c_str()) || lua_pcall(m_pMasterState, 0, LUA_MULTRET, -2)) {
            // Retrieve the error message then pop it off the stack to clean it up
			m_LastError = lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors) {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            error = -1;
        }
    } catch(const std::exception &e) {
        m_LastError = e.what();
        if (consoleErrors) {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        error = -1;
    }

    // Pop the file and line error handler off the stack to clean it up
    lua_pop(m_pMasterState, 1);
        
    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::RunScriptFile(const std::string &filePath, bool consoleErrors) {
    if (filePath.empty()) {
        m_LastError = "Can't run a script file with an empty filepath!";
        return -1;
    }

	if (!System::PathExistsCaseSensitive(filePath)){
		m_LastError = "Script file: " + filePath + " doesn't exist!";
		if (consoleErrors) {
			g_ConsoleMan.PrintString("ERROR: " + m_LastError);
			ClearErrors();
		}
		return -1;
	}

    int error = 0;

    lua_pushcfunction(m_pMasterState, &AddFileAndLineToError);
    try {
        // Load the script file's contents onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
        if (luaL_loadfile(m_pMasterState, filePath.c_str()) || lua_pcall(m_pMasterState, 0, LUA_MULTRET, -2)) {
            // Retrieve the error message then pop it off the stack
            m_LastError = lua_tostring(m_pMasterState, -1);
            lua_pop(m_pMasterState, 1);
            if (consoleErrors) {
                g_ConsoleMan.PrintString("ERROR: " + m_LastError);
                ClearErrors();
            }
            error = -1;
        }
    } catch(const std::exception &e) {
        m_LastError = e.what();
        if (consoleErrors) {
            g_ConsoleMan.PrintString("ERROR: " + m_LastError);
            ClearErrors();
        }
        error = -1;
    }

    // Pop the file and line error handler off the stack to clean it up
    lua_pop(m_pMasterState, 1);

    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string LuaMan::GetNewPresetID()
{
    // Generate the new ID
    char newID[64];
    std::snprintf(newID, sizeof(newID), "Pre%05i", m_NextPresetID);
    // Increment the ID so it will be diff for the next one (improve this primitive approach??)
    m_NextPresetID++;

    return string(newID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

string LuaMan::GetNewObjectID()
{
    // Generate the new ID
    char newID[64];
    std::snprintf(newID, sizeof(newID), "Obj%05i", m_NextObjectID);
    // Increment the ID so it will be diff for the next one (improve this primitive approach??)
    m_NextObjectID++;

    return string(newID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::Update()
{
	lua_gc(m_pMasterState, LUA_GCSTEP, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LuaMan::FileOpen(std::string filename, std::string mode)
{
	int fl = -1;

	// Find some suitable file
	for (int i = 0; i < MAX_OPEN_FILES; ++i)
		if (m_Files[i] == 0)
		{
			fl = i;
			break;
		}

	if (fl == -1)
	{
		g_ConsoleMan.PrintString("Error: Can't open file, no more slots.");
		return -1;
	}

	//Check for path back-traversing and .rte extension. Everything is allowed to read or write only inside rte's
    string dotString = "..";
	string rteString = ".rte";

	string fullPath = System::GetWorkingDirectory() + filename;

	// Do not open paths with '..'
	if (fullPath.find(dotString) != string::npos)
		return -1;

	// Do not open paths that aren't written correctly
	if (!System::PathExistsCaseSensitive(std::filesystem::path(fullPath).lexically_normal().generic_string()))
		return -1;

	// Allow to edit files only inside .rte folders
	if (fullPath.find(rteString) == string::npos)
		return -1;

	// Open file and save handle
	FILE * f = fopen(fullPath.c_str(), mode.c_str());
	if (f)
	{
		m_Files[fl] = f;
		return fl;
	}

#ifdef _DEBUG
	g_ConsoleMan.PrintString("Error: Can't open file. " + fullPath);
#endif
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileClose(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
	{
		fclose(m_Files[file]);
		m_Files[file] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileCloseAll()
{
	for (int file = 0 ; file < MAX_OPEN_FILES ; ++file)
		FileClose(file);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LuaMan::FileReadLine(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
	{
		char buf[4096];
		fgets(buf, 4095, m_Files[file]);
		return (std::string(buf));
	}
	g_ConsoleMan.PrintString("Error: Tried to read a closed file, or read past EOF.");
	return "";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LuaMan::FileWriteLine(int file, std::string line)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
		fputs(line.c_str(), m_Files[file]);
	else
		g_ConsoleMan.PrintString("Error: Tried to write to a closed file.");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LuaMan::FileEOF(int file)
{
	if (file > -1 && file < MAX_OPEN_FILES && m_Files[file])
		if (!feof(m_Files[file]))
			return false;
	return true;
}
}
