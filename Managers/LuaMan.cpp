#include "LuaMan.h"

#include "LuabindObjectWrapper.h"
#include "LuaBindingRegisterDefinitions.h"
#include "ThreadMan.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"

namespace RTE {

	const std::unordered_set<std::string> LuaMan::c_FileAccessModes = { "r", "r+", "w", "w+", "a", "a+" };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Clear() {
		m_State = nullptr;
		m_TempEntity = nullptr;
		m_TempEntityVector.clear();
		m_LastError.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Initialize() {
		m_State = luaL_newstate();
		luabind::open(m_State);
		tracy::LuaRegister(m_State);

		// Disable gc. We do this manually, so we can thread it to occur parallel with non-lua updates
		lua_gc(m_State, LUA_GCSTOP, 0);

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
			if (g_SettingsMan.DisableLuaJIT() && strcmp(lib->name, LUA_JITLIBNAME) == 0) {
				continue;
			}
			lua_pushcfunction(m_State, lib->func);
			lua_pushstring(m_State, lib->name);
			lua_call(m_State, 1, 0);
		}

		// LuaJIT should start automatically after we load the library (if we loaded it) but we're making sure it did anyway.
		if (!g_SettingsMan.DisableLuaJIT() && !luaJIT_setmode(m_State, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) { RTEAbort("Failed to initialize LuaJIT!\nIf this error persists, please disable LuaJIT with \"Settings.ini\" property \"DisableLuaJIT\"."); }

		// From LuaBind documentation:
		// As mentioned in the Lua documentation, it is possible to pass an error handler function to lua_pcall(). LuaBind makes use of lua_pcall() internally when calling member functions and free functions.
		// It is possible to set the error handler function that LuaBind will use globally:
		//set_pcall_callback(&AddFileAndLineToError); // NOTE: this seems to do nothing because retrieving the error from the lua stack wasn't done correctly. The current error handling works just fine but might look into doing this properly sometime later.

		// Register all relevant bindings to the state. Note that the order of registration is important, as bindings can't derive from an unregistered type (inheritance and all that).
		luabind::module(m_State)[
			luabind::class_<LuaStateWrapper>("LuaManager")
				.property("TempEntity", &LuaStateWrapper::GetTempEntity)
				.property("TempEntities", &LuaStateWrapper::GetTempEntityVector, luabind::return_stl_iterator)
				.def("SelectRand", &LuaStateWrapper::SelectRand)
				.def("RangeRand", &LuaStateWrapper::RangeRand)
				.def("PosRand", &LuaStateWrapper::PosRand)
				.def("NormalRand", &LuaStateWrapper::NormalRand)
				.def("GetDirectoryList", &LuaStateWrapper::DirectoryList, luabind::return_stl_iterator)
				.def("GetFileList", &LuaStateWrapper::FileList, luabind::return_stl_iterator)
				.def("FileExists", &LuaStateWrapper::FileExists)
				.def("FileOpen", &LuaStateWrapper::FileOpen)
				.def("FileClose", &LuaStateWrapper::FileClose)
				.def("FileReadLine", &LuaStateWrapper::FileReadLine)
				.def("FileWriteLine", &LuaStateWrapper::FileWriteLine)
				.def("FileEOF", &LuaStateWrapper::FileEOF),

			luabind::def("DeleteEntity", &LuaAdaptersUtility::DeleteEntity, luabind::adopt(_1)), // NOT a member function, so adopting _1 instead of the _2 for the first param, since there's no "this" pointer!!
			luabind::def("Lerp", (float(*)(float, float, float, float, float)) &Lerp),
			luabind::def("Lerp", (Vector(*)(float, float, Vector, Vector, float)) &Lerp),
			luabind::def("Lerp", (Matrix(*)(float, float, Matrix, Matrix, float)) &Lerp),
			luabind::def("EaseIn", &EaseIn),
			luabind::def("EaseOut", &EaseOut),
			luabind::def("EaseInOut", &EaseInOut),
			luabind::def("Clamp", &Limit),
			luabind::def("NormalizeAngleBetween0And2PI", &NormalizeAngleBetween0And2PI),
			luabind::def("NormalizeAngleBetweenNegativePIAndPI", &NormalizeAngleBetweenNegativePIAndPI),
			luabind::def("AngleWithinRange", &AngleWithinRange),
			luabind::def("ClampAngle", &ClampAngle),
			luabind::def("GetPPM", &LuaAdaptersUtility::GetPPM),
			luabind::def("GetMPP", &LuaAdaptersUtility::GetMPP),
			luabind::def("GetPPL", &LuaAdaptersUtility::GetPPL),
			luabind::def("GetLPP", &LuaAdaptersUtility::GetLPP),
			luabind::def("GetPathFindingDefaultDigStrength", &LuaAdaptersUtility::GetPathFindingDefaultDigStrength),
			luabind::def("RoundFloatToPrecision", &RoundFloatToPrecision),
			luabind::def("RoundToNearestMultiple", &RoundToNearestMultiple),

			RegisterLuaBindingsOfType(SystemLuaBindings, Vector),
			RegisterLuaBindingsOfType(SystemLuaBindings, Box),
			RegisterLuaBindingsOfType(EntityLuaBindings, Entity),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, SoundContainer),
			RegisterLuaBindingsOfType(EntityLuaBindings, SoundSet),
			RegisterLuaBindingsOfType(EntityLuaBindings, LimbPath),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, SceneObject),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, MovableObject),
			RegisterLuaBindingsOfType(EntityLuaBindings, Material),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOPixel),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, TerrainObject),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, MOSprite),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOSParticle),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, MOSRotating),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Attachable),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, Emission),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, AEmitter),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, AEJetpack),
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
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, PieSlice),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, PieMenu),
			RegisterLuaBindingsOfType(EntityLuaBindings, Gib),
			RegisterLuaBindingsOfType(SystemLuaBindings, Controller),
			RegisterLuaBindingsOfType(SystemLuaBindings, Timer),
			RegisterLuaBindingsOfType(SystemLuaBindings, PathRequest),
			RegisterLuaBindingsOfConcreteType(EntityLuaBindings, Scene),
			RegisterLuaBindingsOfType(EntityLuaBindings, SceneArea),
			RegisterLuaBindingsOfType(EntityLuaBindings, SceneLayer),
			RegisterLuaBindingsOfType(EntityLuaBindings, SLBackground),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, Deployment),
			RegisterLuaBindingsOfType(SystemLuaBindings, DataModule),
			RegisterLuaBindingsOfType(ActivityLuaBindings, Activity),
			RegisterLuaBindingsOfAbstractType(ActivityLuaBindings, GameActivity),
			RegisterLuaBindingsOfAbstractType(EntityLuaBindings, GlobalScript),
			RegisterLuaBindingsOfType(EntityLuaBindings, MetaPlayer),
			RegisterLuaBindingsOfType(GUILuaBindings, GUIBanner),
			RegisterLuaBindingsOfType(GUILuaBindings, BuyMenuGUI),
			RegisterLuaBindingsOfType(GUILuaBindings, SceneEditorGUI),
			RegisterLuaBindingsOfType(ManagerLuaBindings, ActivityMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, AudioMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, CameraMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, ConsoleMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, FrameMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, MetaMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, MovableMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, PostProcessMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, PresetMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, PrimitiveMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, SceneMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, SettingsMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, TimerMan),
			RegisterLuaBindingsOfType(ManagerLuaBindings, UInputMan),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, GraphicalPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, LinePrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, ArcPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, SplinePrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, BoxPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, BoxFillPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, RoundedBoxPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, RoundedBoxFillPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, CirclePrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, CircleFillPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, EllipsePrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, EllipseFillPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, TrianglePrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, TriangleFillPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, TextPrimitive),
			RegisterLuaBindingsOfType(PrimitiveLuaBindings, BitmapPrimitive),
			RegisterLuaBindingsOfType(InputLuaBindings, InputDevice),
			RegisterLuaBindingsOfType(InputLuaBindings, InputElements),
			RegisterLuaBindingsOfType(InputLuaBindings, JoyButtons),
			RegisterLuaBindingsOfType(InputLuaBindings, JoyDirections),
			RegisterLuaBindingsOfType(InputLuaBindings, MouseButtons),
			RegisterLuaBindingsOfType(InputLuaBindings, SDL_Keycode),
			RegisterLuaBindingsOfType(InputLuaBindings, SDL_Scancode),
			RegisterLuaBindingsOfType(InputLuaBindings, SDL_GameControllerButton),
			RegisterLuaBindingsOfType(InputLuaBindings, SDL_GameControllerAxis),
			RegisterLuaBindingsOfType(MiscLuaBindings, AlarmEvent),
			RegisterLuaBindingsOfType(MiscLuaBindings, Directions),
			RegisterLuaBindingsOfType(MiscLuaBindings, DrawBlendMode)
		];

		// Assign the manager instances to globals in the lua master state
		luabind::globals(m_State)["TimerMan"] = &g_TimerMan;
		luabind::globals(m_State)["FrameMan"] = &g_FrameMan;
		luabind::globals(m_State)["PostProcessMan"] = &g_PostProcessMan;
		luabind::globals(m_State)["PrimitiveMan"] = &g_PrimitiveMan;
		luabind::globals(m_State)["PresetMan"] = &g_PresetMan;
		luabind::globals(m_State)["AudioMan"] = &g_AudioMan;
		luabind::globals(m_State)["UInputMan"] = &g_UInputMan;
		luabind::globals(m_State)["SceneMan"] = &g_SceneMan;
		luabind::globals(m_State)["ActivityMan"] = &g_ActivityMan;
		luabind::globals(m_State)["MetaMan"] = &g_MetaMan;
		luabind::globals(m_State)["MovableMan"] = &g_MovableMan;
		luabind::globals(m_State)["CameraMan"] = &g_CameraMan;
		luabind::globals(m_State)["ConsoleMan"] = &g_ConsoleMan;
		luabind::globals(m_State)["LuaMan"] = this;
		luabind::globals(m_State)["SettingsMan"] = &g_SettingsMan;

		const uint64_t seed = RandomNum<uint64_t>(0, std::numeric_limits<uint64_t>::max());
		m_RandomGenerator.Seed(seed);

		luaL_dostring(m_State,
			// Add cls() as a shortcut to ConsoleMan:Clear().
			"cls = function() ConsoleMan:Clear(); end"
			"\n"
			// Override "print" in the lua state to output to the console.
			"print = function(stringToPrint) ConsoleMan:PrintString(\"PRINT: \" .. tostring(stringToPrint)); end"
			"\n"
			// Override random functions to appear global instead of under LuaMan
			"SelectRand = function(lower, upper) return LuaMan:SelectRand(lower, upper); end;\n"
			"RangeRand = function(lower, upper) return LuaMan:RangeRand(lower, upper); end;\n"
			"PosRand = function() return LuaMan:PosRand(); end;\n"
			"NormalRand = function() return LuaMan:NormalRand(); end;\n"
			// Override "math.random" in the lua state to use RTETools MT19937 implementation. Preserve return types of original to not break all the things.
			"math.random = function(lower, upper) if lower ~= nil and upper ~= nil then return LuaMan:SelectRand(lower, upper); elseif lower ~= nil then return LuaMan:SelectRand(1, lower); else return LuaMan:PosRand(); end end"
			"\n"
			// Override "dofile" to be able to account for Data/ or Mods/ directory.
			"OriginalDoFile = dofile; dofile = function(filePath) filePath = PresetMan:GetFullModulePath(filePath); if filePath ~= '' then return OriginalDoFile(filePath); end end;"
			// Internal helper functions to add callbacks for async pathing requests
			"_AsyncPathCallbacks = {};"
			"_AddAsyncPathCallback = function(id, callback) _AsyncPathCallbacks[id] = callback; end\n"
			"_TriggerAsyncPathCallback = function(id, param) if _AsyncPathCallbacks[id] ~= nil then _AsyncPathCallbacks[id](param); _AsyncPathCallbacks[id] = nil; end end\n"
		);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::Destroy() {
		lua_close(m_State);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::SelectRand(int minInclusive, int maxInclusive) {
		return m_RandomGenerator.RandomNum<int>(minInclusive, maxInclusive);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::RangeRand(double minInclusive, double maxInclusive) {
		return m_RandomGenerator.RandomNum<double>(minInclusive, maxInclusive);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::NormalRand() {
		return m_RandomGenerator.RandomNormalNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double LuaStateWrapper::PosRand() {
		return m_RandomGenerator.RandomNum<double>();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Passthrough LuaMan Functions
	const std::vector<std::string>& LuaStateWrapper::DirectoryList(const std::string& relativeDirectory) { return g_LuaMan.DirectoryList(relativeDirectory); }
	const std::vector<std::string>& LuaStateWrapper::FileList(const std::string& relativeDirectory) { return g_LuaMan.FileList(relativeDirectory); }
	bool LuaStateWrapper::FileExists(const std::string &fileName) { return g_LuaMan.FileExists(fileName); }
	int LuaStateWrapper::FileOpen(const std::string& fileName, const std::string& accessMode) { return g_LuaMan.FileOpen(fileName, accessMode); }
	void LuaStateWrapper::FileClose(int fileIndex) { return g_LuaMan.FileClose(fileIndex); }
	void LuaStateWrapper::FileCloseAll() { return g_LuaMan.FileCloseAll(); }
	std::string LuaStateWrapper::FileReadLine(int fileIndex) { return g_LuaMan.FileReadLine(fileIndex); }
	void LuaStateWrapper::FileWriteLine(int fileIndex, const std::string& line) { return g_LuaMan.FileWriteLine(fileIndex, line); }
	bool LuaStateWrapper::FileEOF(int fileIndex) { return g_LuaMan.FileEOF(fileIndex); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Clear() {
		m_OpenedFiles.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Initialize() {
		m_MasterScriptState.Initialize();

		m_ScriptStates = std::vector<LuaStateWrapper>(std::thread::hardware_concurrency());
		for (LuaStateWrapper &luaState : m_ScriptStates) {
			luaState.Initialize();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaStateWrapper & LuaMan::GetMasterScriptState() {
        return m_MasterScriptState;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    LuaStatesArray & LuaMan::GetThreadedScriptStates() {
		return m_ScriptStates;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	thread_local LuaStateWrapper * s_luaStateOverride = nullptr;
    LuaStateWrapper * LuaMan::GetThreadLuaStateOverride() const {
		return s_luaStateOverride;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::SetThreadLuaStateOverride(LuaStateWrapper * luaState) {
		s_luaStateOverride = luaState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	thread_local LuaStateWrapper* s_currentLuaState = nullptr;
	LuaStateWrapper* LuaMan::GetThreadCurrentLuaState() const {
		return s_currentLuaState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaStateWrapper * LuaMan::GetAndLockFreeScriptState() {
		if (s_luaStateOverride) {
			// We're creating this object in a multithreaded environment, ensure that it's assigned to the same script state as us
			bool success = s_luaStateOverride->GetMutex().try_lock();
			RTEAssert(success, "Our lua state override for our thread already belongs to another thread!")
			return s_luaStateOverride;
		}

		// TODO
		// It would be nice to assign to least-saturated state, but that's a bit tricky with MO registering...
		/*auto itr = std::min_element(m_ScriptStates.begin(), m_ScriptStates.end(), 
			[](const LuaStateWrapper& lhs, const LuaStateWrapper& rhs) { return lhs.GetRegisteredMOs().size() < rhs.GetRegisteredMOs().size(); }
		);

		bool success = itr->GetMutex().try_lock();
		RTEAssert(success, "Script mutex was already locked while in a non-multithreaded environment!");

		return &(*itr);*/

		int ourState = m_LastAssignedLuaState;
		m_LastAssignedLuaState = (m_LastAssignedLuaState + 1) % m_ScriptStates.size();

		bool success = m_ScriptStates[ourState].GetMutex().try_lock();
		RTEAssert(success, "Script mutex was already locked while in a non-multithreaded environment!");

		return &m_ScriptStates[ourState];	
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::ClearUserModuleCache() {
		m_GarbageCollectionTask.wait();

		m_MasterScriptState.ClearLuaScriptCache();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearLuaScriptCache();
		}

		m_MasterScriptState.ClearUserModuleCache();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearUserModuleCache();
		}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::AddLuaScriptCallback(const std::function<void()> &callback) {
		std::scoped_lock lock(m_ScriptCallbacksMutex);
		m_ScriptCallbacks.emplace_back(callback);
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaMan::ExecuteLuaScriptCallbacks() {
		std::vector<std::function<void()>> callbacks;
		
		// Move our functions into the local buffer to clear the existing callbacks and to lock for as little time as possible
		{
			std::scoped_lock lock(m_ScriptCallbacksMutex);
			callbacks.swap(m_ScriptCallbacks);
		}
		
		for (const std::function<void()> &callback : callbacks) {
			callback();
		}
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::unordered_map<std::string, PerformanceMan::ScriptTiming> LuaMan::GetScriptTimings() const {
		std::unordered_map<std::string, PerformanceMan::ScriptTiming> timings = m_MasterScriptState.GetScriptTimings();
		for (const LuaStateWrapper &luaState : m_ScriptStates) {
			for (auto&& [functionName, timing] : luaState.GetScriptTimings()) {
				auto& existing = timings[functionName];
				existing.m_CallCount += timing.m_CallCount;
				existing.m_Time = std::max(existing.m_Time, timing.m_Time);
			}
		}
		return timings;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Destroy() {
		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			FileClose(i);
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearUserModuleCache() {
		luaL_dostring(m_State, "for m, n in pairs(package.loaded) do if type(n) == \"boolean\" then package.loaded[m] = nil; end; end;");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearLuaScriptCache() {
		m_ScriptCache.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Entity *LuaStateWrapper::GetTempEntity() const {
        return m_TempEntity;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::SetTempEntity(Entity *entity) {
		m_TempEntity = entity;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const std::vector<Entity *> & LuaStateWrapper::GetTempEntityVector() const {
        return m_TempEntityVector;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SetTempEntityVector(const std::vector<const Entity *> &entityVector) {
		m_TempEntityVector.reserve(entityVector.size());
		for (const Entity *entity : entityVector) {
			m_TempEntityVector.push_back(const_cast<Entity *>(entity));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SetLuaPath(const std::string &filePath) {
		const std::string moduleName = g_PresetMan.GetModuleNameFromPath(filePath);
		const std::string moduleFolder = g_PresetMan.IsModuleOfficial(moduleName) ? System::GetDataDirectory() : System::GetModDirectory();
		const std::string scriptPath = moduleFolder + moduleName + "/?.lua";

		lua_getglobal(m_State, "package");
		lua_getfield(m_State, -1, "path"); // get field "path" from table at top of stack (-1).
		std::string currentPath = lua_tostring(m_State, -1); // grab path string from top of stack.

		// check if scriptPath is already in there, if not add it.
		if (currentPath.find(scriptPath) == std::string::npos) {
			currentPath.append(";" + scriptPath);
		}

		lua_pop(m_State, 1); // get rid of the string on the stack we just pushed previously.
		lua_pushstring(m_State, currentPath.c_str()); // push the new one.
		lua_setfield(m_State, -2, "path"); // set the field "path" in table at -2 with value at top of stack.
		lua_pop(m_State, 1); // get rid of package table from top of stack.
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::unordered_map<std::string, PerformanceMan::ScriptTiming> & LuaStateWrapper::GetScriptTimings() const {
		return m_ScriptTimings;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFunctionString(const std::string &functionName, const std::string &selfObjectName, const std::vector<std::string_view> &variablesToSafetyCheck, const std::vector<const Entity *> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments) {
		std::stringstream scriptString;
		if (!variablesToSafetyCheck.empty()) {
			scriptString << "if ";
			for (const std::string_view &variableToSafetyCheck : variablesToSafetyCheck) {
				if (&variableToSafetyCheck != &variablesToSafetyCheck[0]) { scriptString << " and "; }
				scriptString << variableToSafetyCheck;
			}
			scriptString << " then ";
		}
		if (!functionEntityArguments.empty()) { scriptString << "local entityArguments = LuaMan.TempEntities; "; }

		// Lock here, even though we also lock in RunScriptString(), to ensure that the temp entity vector isn't stomped by separate threads.
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		scriptString << functionName + "(";
		if (!selfObjectName.empty()) { scriptString << selfObjectName; }
		bool isFirstFunctionArgument = selfObjectName.empty();
		if (!functionEntityArguments.empty()) {
			SetTempEntityVector(functionEntityArguments);
			for (const Entity *functionEntityArgument : functionEntityArguments) {
				if (!isFirstFunctionArgument) { scriptString << ", "; }
				scriptString << "(To" + functionEntityArgument->GetClassName() + " and To" + functionEntityArgument->GetClassName() + "(entityArguments()) or entityArguments())";
				isFirstFunctionArgument = false;
			}
		}
		if (!functionLiteralArguments.empty()) {
			for (const std::string_view &functionLiteralArgument : functionLiteralArguments) {
				if (!isFirstFunctionArgument) { scriptString << ", "; }
				scriptString << std::string(functionLiteralArgument);
				isFirstFunctionArgument = false;
			}
		}
		scriptString << ");";

		if (!variablesToSafetyCheck.empty()) { scriptString << " end;"; }

		int result = RunScriptString(scriptString.str());
		m_TempEntityVector.clear();
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptString(const std::string &scriptString, bool consoleErrors) {
		if (scriptString.empty()) {
			return -1;
		}
		int error = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		// Load the script string onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
		if (luaL_loadstring(m_State, scriptString.c_str()) || lua_pcall(m_State, 0, LUA_MULTRET, -2)) {
			// Retrieve the error message then pop it off the stack to clean it up
			m_LastError = lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}
		// Pop the file and line error handler off the stack to clean it up
		lua_pop(m_State, 1);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFunctionObject(const LuabindObjectWrapper *functionObject, const std::string &selfGlobalTableName, const std::string &selfGlobalTableKey, const std::vector<const Entity*> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments, const std::vector<LuabindObjectWrapper*> &functionObjectArguments) {
		int status = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		functionObject->GetLuabindObject()->push(m_State);

		int argumentCount = functionEntityArguments.size() + functionLiteralArguments.size() + functionObjectArguments.size();
		if (!selfGlobalTableName.empty() && TableEntryIsDefined(selfGlobalTableName, selfGlobalTableKey)) {
			lua_getglobal(m_State, selfGlobalTableName.c_str());
			lua_getfield(m_State, -1, selfGlobalTableKey.c_str());
			lua_remove(m_State, -2);
			argumentCount++;
		}

		for (const Entity *functionEntityArgument : functionEntityArguments) {
			std::unique_ptr<LuabindObjectWrapper> downCastEntityAsLuabindObjectWrapper(LuaAdaptersEntityCast::s_EntityToLuabindObjectCastFunctions.at(functionEntityArgument->GetClassName())(const_cast<Entity *>(functionEntityArgument), m_State));
			downCastEntityAsLuabindObjectWrapper->GetLuabindObject()->push(m_State);
		}

		for (const std::string_view &functionLiteralArgument : functionLiteralArguments) {
			char *stringToDoubleConversionFailed = nullptr;
			if (functionLiteralArgument == "nil") {
				lua_pushnil(m_State);
			} else if (functionLiteralArgument == "true" || functionLiteralArgument == "false") {
				lua_pushboolean(m_State, functionLiteralArgument == "true" ? 1 : 0);
			} else if (double argumentAsNumber = std::strtod(functionLiteralArgument.data(), &stringToDoubleConversionFailed); !*stringToDoubleConversionFailed) {
				lua_pushnumber(m_State, argumentAsNumber);
			} else {
				lua_pushlstring(m_State, functionLiteralArgument.data(), functionLiteralArgument.size());
			}
		}

		for (const LuabindObjectWrapper *functionObjectArgument : functionObjectArguments) {
			if (functionObjectArgument->GetLuabindObject()->interpreter() != m_State) {
				LuabindObjectWrapper copy = functionObjectArgument->GetCopyForState(*m_State);
				copy.GetLuabindObject()->push(m_State);
			} else {
				functionObjectArgument->GetLuabindObject()->push(m_State);
			}
		}

		const std::string& path = functionObject->GetFilePath();
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		{
			ZoneScoped;
			ZoneName(path.c_str(), path.length());

			if (lua_pcall(m_State, argumentCount, LUA_MULTRET, -argumentCount - 2) > 0) {
				m_LastError = lua_tostring(m_State, -1);
				lua_pop(m_State, 1);
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
				status = -1;
			}
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		// only track time in non-MT scripts, for now
		if (&g_LuaMan.GetMasterScriptState() == this) {
			m_ScriptTimings[path].m_Time += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
			m_ScriptTimings[path].m_CallCount++;
		}

		lua_pop(m_State, 1);

		return status;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFile(const std::string &filePath, bool consoleErrors) {
		const std::string fullScriptPath = g_PresetMan.GetFullModulePath(filePath);
		if (fullScriptPath.empty()) {
			m_LastError = "Can't run a script file with an empty filepath!";
			return -1;
		}

		if (!System::PathExistsCaseSensitive(fullScriptPath)) {
			m_LastError = "Script file: " + filePath + " doesn't exist!";
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return -1;
		}

		int error = 0;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		const int stackStart = lua_gettop(m_State);

		lua_pushcfunction(m_State, &AddFileAndLineToError);
		SetLuaPath(fullScriptPath);

		// Load the script file's contents onto the stack
		if (luaL_loadfile(m_State, fullScriptPath.c_str())) {
			m_LastError = lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}

		if (error == 0) {
			// create a new environment table
			lua_getglobal(m_State, filePath.c_str());
			if (lua_isnil(m_State, -1)) {
				lua_pop(m_State, 1);
				lua_newtable(m_State);
				lua_newtable(m_State);
				lua_getglobal(m_State, "_G");
				lua_setfield(m_State, -2, "__index");
				lua_setmetatable(m_State, -2);
				lua_setglobal(m_State, filePath.c_str());
				lua_getglobal(m_State, filePath.c_str());
			}

			lua_setfenv(m_State, -2);

			// execute script file with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
			if (lua_pcall(m_State, 0, LUA_MULTRET, -2)) {
				m_LastError = lua_tostring(m_State, -1);
				lua_pop(m_State, 1);
				if (consoleErrors) {
					g_ConsoleMan.PrintString("ERROR: " + m_LastError);
					ClearErrors();
				}
				error = -1;
			}
		}

		// Pop the line error handler off the stack to clean it up
		lua_pop(m_State, 1);

		RTEAssert(lua_gettop(m_State) == stackStart, "Malformed lua stack!");
		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaStateWrapper::RunScriptFileAndRetrieveFunctions(const std::string &filePath, const std::string &prefix, const std::vector<std::string> &functionNamesToLookFor, std::unordered_map<std::string, LuabindObjectWrapper *> &outFunctionNamesAndObjects, bool forceReload) {
		static bool disableCaching = false;
		forceReload = forceReload || disableCaching;

		// If it's already cached, we don't need to run it again
		// TODO - fix activity restarting needing to force reload
		auto cachedScript = m_ScriptCache.find(filePath);
		if (!forceReload && cachedScript != m_ScriptCache.end()) {
			for (auto& pair : cachedScript->second.functionNamesAndObjects) {
				luabind::object* functionObjectCopyForStoring = new luabind::object(*pair.second->GetLuabindObject());
				outFunctionNamesAndObjects.try_emplace(pair.first, new LuabindObjectWrapper(functionObjectCopyForStoring, filePath));
			}

			return 0;
		}

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		s_currentLuaState = this;

		if (int error = RunScriptFile(filePath); error < 0) {
			return error;
		}

		luabind::object prefixObject;
		if (prefix == "") {
			prefixObject = luabind::globals(m_State)[filePath.c_str()];
		} else {
			prefixObject = luabind::globals(m_State)[filePath.c_str()][prefix];
		}

		if (luabind::type(prefixObject) == LUA_TNIL) {
			return -1;
		}

		auto &newScript = m_ScriptCache[filePath];
		newScript.functionNamesAndObjects.clear();
		for (const std::string& functionName : functionNamesToLookFor) {
			luabind::object functionObject = prefixObject[functionName];
			if (luabind::type(functionObject) == LUA_TFUNCTION) {
				luabind::object* functionObjectCopyForStoring = new luabind::object(functionObject);
				newScript.functionNamesAndObjects.try_emplace(functionName, new LuabindObjectWrapper(functionObjectCopyForStoring, filePath));
			}
		}

		for (auto& pair : newScript.functionNamesAndObjects) {
			luabind::object* functionObjectCopyForStoring = new luabind::object(*pair.second->GetLuabindObject());
			outFunctionNamesAndObjects.try_emplace(pair.first, new LuabindObjectWrapper(functionObjectCopyForStoring, filePath));
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::Update() {
		for (MovableObject* mo : m_AddedRegisteredMOs) {
			m_RegisteredMOs.insert(mo);
		}
		m_AddedRegisteredMOs.clear();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::ClearScriptTimings() {
		m_ScriptTimings.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool LuaStateWrapper::ExpressionIsTrue(const std::string &expression, bool consoleErrors) {
        if (expression.empty()) {
			return false;
		}
		bool result = false;

		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Push the script string onto the stack so we can execute it, and then actually try to run it. Assign the result to a dedicated temp global variable.
		if (luaL_dostring(m_State, std::string("ExpressionResult = " + expression + ";").c_str())) {
			m_LastError = std::string("When evaluating Lua expression: ") + lua_tostring(m_State, -1);
			lua_pop(m_State, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return false;
		}
		// Put the result of the expression on the lua stack and check its value. Need to pop it off the stack afterwards so it leaves the stack unchanged.
		lua_getglobal(m_State, "ExpressionResult");
		result = lua_toboolean(m_State, -1);
		lua_pop(m_State, 1);

		return result;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaStateWrapper::SavePointerAsGlobal(void *objectToSave, const std::string &globalName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Push the pointer onto the Lua stack.
		lua_pushlightuserdata(m_State, objectToSave);
		// Pop and assign that pointer to a global var in the Lua state.
		lua_setglobal(m_State, globalName.c_str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaStateWrapper::GlobalIsDefined(const std::string &globalName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);

		// Get the var you want onto the stack so we can check it.
		lua_getglobal(m_State, globalName.c_str());
		// Now report if it is nil/null or not.
		bool isDefined = !lua_isnil(m_State, -1);
		// Pop the var so this operation is balanced and leaves the stack as it was.
		lua_pop(m_State, 1);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaStateWrapper::TableEntryIsDefined(const std::string &tableName, const std::string &indexName) {
		std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		
		// Push the table onto the stack, checking if it even exists.
		lua_getglobal(m_State, tableName.c_str());
		if (!lua_istable(m_State, -1)) {
			// Clean up and report that there was nothing properly defined here.
			lua_pop(m_State, 1);
			return false;
		}
		// Push the value at the requested index onto the stack so we can check if it's anything.
		lua_getfield(m_State, -1, indexName.c_str());
		// Now report if it is nil/null or not
		bool isDefined = !lua_isnil(m_State, -1);
		// Pop both the var and the table so this operation is balanced and leaves the stack as it was.
		lua_pop(m_State, 2);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool LuaStateWrapper::ErrorExists() const {
        return !m_LastError.empty();;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string LuaStateWrapper::GetLastError() const {
        return m_LastError;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void LuaStateWrapper::ClearErrors() {
		m_LastError.clear();
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaStateWrapper::DescribeLuaStack() {
		int indexOfTopOfStack = lua_gettop(m_State);
		if (indexOfTopOfStack == 0) {
			return "The Lua stack is empty.";
		}
		std::stringstream stackDescription;
		stackDescription << "The Lua stack contains " + std::to_string(indexOfTopOfStack) + " elements. From top to bottom, they are:\n";

		for (int i = indexOfTopOfStack; i > 0; --i) {
			switch (int type = lua_type(m_State, i)) {
				case LUA_TBOOLEAN:
					stackDescription << (lua_toboolean(m_State, i) ? "true" : "false");
					break;
				case LUA_TNUMBER:
					stackDescription << std::to_string(lua_tonumber(m_State, i));
					break;
				case LUA_TSTRING:
					stackDescription << lua_tostring(m_State, i);
					break;
				default:
					stackDescription << lua_typename(m_State, type);
					break;
			}
			if (i - 1 > 0) { stackDescription << "\n"; }
		}
		return stackDescription.str();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<std::string> & LuaMan::DirectoryList(const std::string &filePath) {
		thread_local std::vector<std::string> directoryPaths;
		directoryPaths.clear();

		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory() + filePath)) {
			if (directoryEntry.is_directory()) { directoryPaths.emplace_back(directoryEntry.path().filename().generic_string()); }
		}
		return directoryPaths;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<std::string> & LuaMan::FileList(const std::string &filePath) {
		thread_local std::vector<std::string> filePaths;
		filePaths.clear();

		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory() + filePath)) {
			if (directoryEntry.is_regular_file()) { filePaths.emplace_back(directoryEntry.path().filename().generic_string()); }
		}
		return filePaths;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::FileExists(const std::string &fileName) {
		std::string fullPath = System::GetWorkingDirectory() + g_PresetMan.GetFullModulePath(fileName);
		if ((fullPath.find("..") == std::string::npos) && (fullPath.find(System::GetModulePackageExtension()) != std::string::npos)) {
			return std::filesystem::exists(fullPath);
		}

		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::FileOpen(const std::string &fileName, const std::string &accessMode) {
		if (c_FileAccessModes.find(accessMode) == c_FileAccessModes.end()) {
			g_ConsoleMan.PrintString("ERROR: Cannot open file, invalid file access mode specified.");
			return -1;
		}

		int fileIndex = -1;
		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			if (!m_OpenedFiles[i]) {
				fileIndex = i;
				break;
			}
		}
		if (fileIndex == -1) {
			g_ConsoleMan.PrintString("ERROR: Cannot open file, maximum number of files already open.");
			return -1;
		}

		std::string fullPath = System::GetWorkingDirectory() + g_PresetMan.GetFullModulePath(fileName);
		if ((fullPath.find("..") == std::string::npos) && (fullPath.find(System::GetModulePackageExtension()) != std::string::npos)) {

#ifdef _WIN32
			FILE *file = fopen(fullPath.c_str(), accessMode.c_str());
#else
			FILE *file = [&fullPath, &accessMode]() -> FILE* {
				std::filesystem::path inspectedPath = System::GetWorkingDirectory();
				const std::filesystem::path relativeFilePath = std::filesystem::path(fullPath).lexically_relative(inspectedPath);

				for (std::filesystem::path::const_iterator relativeFilePathIterator = relativeFilePath.begin(); relativeFilePathIterator != relativeFilePath.end(); ++relativeFilePathIterator) {
					bool pathPartExists = false;

					// Check if a path part (directory or file) exists in the filesystem.
					for (const std::filesystem::path &filesystemEntryPath : std::filesystem::directory_iterator(inspectedPath)) {
						if (StringsEqualCaseInsensitive(filesystemEntryPath.filename().generic_string(), (*relativeFilePathIterator).generic_string())) {
							inspectedPath = filesystemEntryPath;
							pathPartExists = true;
							break;
						}
					}
					if (!pathPartExists) {
						// If this is the last part, then all directories in relativeFilePath exist, but the file doesn't.
						if (std::next(relativeFilePathIterator) == relativeFilePath.end()) {
							return fopen((inspectedPath / relativeFilePath.filename()).generic_string().c_str(), accessMode.c_str());
						}
						// Some directory in relativeFilePath doesn't exist, so the file can't be created.
						return nullptr;
					}
				}
				// If the file exists, open it.
				return fopen(inspectedPath.generic_string().c_str(), accessMode.c_str());
			}();
#endif
			if (file) {
				m_OpenedFiles[fileIndex] = file;
				return fileIndex;
			}
		}
		g_ConsoleMan.PrintString("ERROR: Failed to open file " + fileName);
		return -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileClose(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			fclose(m_OpenedFiles[fileIndex]);
			m_OpenedFiles[fileIndex] = nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileCloseAll() {
		for (int file = 0; file < c_MaxOpenFiles; ++file) {
			FileClose(file);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaMan::FileReadLine(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			char buf[4096];
			if (fgets(buf, sizeof(buf), m_OpenedFiles[fileIndex]) != nullptr) {
				return buf;
			}
#ifndef RELEASE_BUILD
			g_ConsoleMan.PrintString("ERROR: " + std::string(FileEOF(fileIndex) ? "Tried to read past EOF." : "Failed to read from file."));
#endif
		} else {
			g_ConsoleMan.PrintString("ERROR: Tried to read an invalid or closed file.");
		}
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileWriteLine(int fileIndex, const std::string &line) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			if (fputs(line.c_str(), m_OpenedFiles[fileIndex]) == EOF) {
				g_ConsoleMan.PrintString("ERROR: Failed to write to file. File might have been opened without writing permissions or is corrupt.");
			}
		} else {
			g_ConsoleMan.PrintString("ERROR: Tried to write to an invalid or closed file.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::FileEOF(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			return feof(m_OpenedFiles[fileIndex]);
		}
		g_ConsoleMan.PrintString("ERROR: Tried to check EOF for an invalid or closed file.");
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Update() {
		ZoneScoped;

		m_MasterScriptState.Update();
		for (LuaStateWrapper &luaState : m_ScriptStates) {
			luaState.Update();
		}

		// Make sure a GC run isn't happening while we try to apply deletions
		m_GarbageCollectionTask.wait();

		// Apply all deletions queued from lua
    	LuabindObjectWrapper::ApplyQueuedDeletions();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::StartAsyncGarbageCollection() {
		ZoneScoped;
		
		std::vector<LuaStateWrapper*> allStates;
		allStates.reserve(m_ScriptStates.size() + 1);

		allStates.push_back(&m_MasterScriptState);
		for (LuaStateWrapper& wrapper : m_ScriptStates) {
			allStates.push_back(&wrapper);
		}
		
		m_GarbageCollectionTask = BS::multi_future<void>();
		for (LuaStateWrapper* luaState : allStates) {
			m_GarbageCollectionTask.push_back(
				g_ThreadMan.GetPriorityThreadPool().submit([luaState]() {
					ZoneScopedN("Lua Garbage Collection");
					std::lock_guard<std::recursive_mutex> lock(luaState->GetMutex());
					lua_gc(luaState->GetLuaState(), LUA_GCSTEP, 100);
					lua_gc(luaState->GetLuaState(), LUA_GCSTOP, 0);
				})
			);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::ClearScriptTimings() {
		m_MasterScriptState.ClearScriptTimings();
		for (LuaStateWrapper& luaState : m_ScriptStates) {
			luaState.ClearScriptTimings();
		}
	}

}
