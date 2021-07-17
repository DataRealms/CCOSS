#include "LuaMan.h"

#include "LuaBindingsManagers.h"
#include "LuaBindingsActivities.h"
#include "LuaBindingsGUI.h"
#include "LuaBindingsMisc.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Clear() {
		m_MasterState = nullptr;
		m_DisableLuaJIT = false;
		m_LastError.clear();
		m_NextPresetID = 0;
		m_NextObjectID = 0;
		m_TempEntity = nullptr;
		m_TempEntityVector.clear();

		m_OpenedFiles.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Initialize() {
		m_MasterState = luaL_newstate();
		luabind::open(m_MasterState);

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
			if (m_DisableLuaJIT && lib->name == LUA_JITLIBNAME) {
				continue;
			}
			lua_pushcfunction(m_MasterState, lib->func);
			lua_pushstring(m_MasterState, lib->name);
			lua_call(m_MasterState, 1, 0);
		}

		// LuaJIT should start automatically after we load the library (if we loaded it) but we're making sure it did anyway.
		if (!m_DisableLuaJIT && !luaJIT_setmode(m_MasterState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON)) { RTEAbort("Failed to initialize LuaJIT!\nIf this error persists, please disable LuaJIT with \"Settings.ini\" property \"DisableLuaJIT\"."); }

		// From LuaBind documentation:
		// As mentioned in the Lua documentation, it is possible to pass an error handler function to lua_pcall(). LuaBind makes use of lua_pcall() internally when calling member functions and free functions.
		// It is possible to set the error handler function that LuaBind will use globally:
		//set_pcall_callback(&AddFileAndLineToError); //NOTE: This seems to do nothing

		// Register all relevant bindings to the master state. Note that the order of registration is important, as bindings can't derive from an unregistered type (inheritance and all that).
		luabind::module(m_MasterState)[
			luabind::class_<LuaMan>("LuaManager")
				.property("TempEntity", &LuaMan::GetTempEntity)
				.def_readonly("TempEntities", &LuaMan::m_TempEntityVector, luabind::return_stl_iterator)
				.def("FileOpen", &LuaMan::FileOpen)
				.def("FileClose", &LuaMan::FileClose)
				.def("FileReadLine", &LuaMan::FileReadLine)
				.def("FileWriteLine", &LuaMan::FileWriteLine)
				.def("FileEOF", &LuaMan::FileEOF),

			SystemLuaBindings::RegisterVectorLuaBindings(),
			SystemLuaBindings::RegisterBoxLuaBindings(),
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
			EntityLuaBindings::RegisterSceneAreaLuaBindings(),
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
			MiscLuaBindings::RegisterAlarmEventLuaBindings(),
			MiscLuaBindings::RegisterInputDeviceLuaBindings(),
			MiscLuaBindings::RegisterInputElementsLuaBindings(),
			MiscLuaBindings::RegisterJoyButtonsLuaBindings(),
			MiscLuaBindings::RegisterJoyDirectionsLuaBindings(),
			MiscLuaBindings::RegisterMouseButtonsLuaBindings(),

			luabind::def("DeleteEntity", &DeleteEntity, luabind::adopt(_1)), // NOT a member function, so adopting _1 instead of the _2 for the first param, since there's no "this" pointer!!
			luabind::def("RangeRand", (double(*)(double, double)) &RandomNum),
			luabind::def("PosRand", &PosRand),
			luabind::def("NormalRand", &NormalRand),
			luabind::def("SelectRand", (int(*)(int, int)) &RandomNum),
			luabind::def("LERP", &LERP),
			luabind::def("EaseIn", &EaseIn),
			luabind::def("EaseOut", &EaseOut),
			luabind::def("EaseInOut", &EaseInOut),
			luabind::def("Clamp", &Limit),
			luabind::def("GetPPM", &GetPPM),
			luabind::def("GetMPP", &GetMPP),
			luabind::def("GetPPL", &GetPPL),
			luabind::def("GetLPP", &GetLPP),
			luabind::def("RoundFloatToPrecision", &RoundFloatToPrecision)
		];

		// Assign the manager instances to globals in the lua master state
		luabind::globals(m_MasterState)["TimerMan"] = &g_TimerMan;
		luabind::globals(m_MasterState)["FrameMan"] = &g_FrameMan;
		luabind::globals(m_MasterState)["PostProcessMan"] = &g_PostProcessMan;
		luabind::globals(m_MasterState)["PrimitiveMan"] = &g_PrimitiveMan;
		luabind::globals(m_MasterState)["PresetMan"] = &g_PresetMan;
		luabind::globals(m_MasterState)["AudioMan"] = &g_AudioMan;
		luabind::globals(m_MasterState)["UInputMan"] = &g_UInputMan;
		luabind::globals(m_MasterState)["SceneMan"] = &g_SceneMan;
		luabind::globals(m_MasterState)["ActivityMan"] = &g_ActivityMan;
		luabind::globals(m_MasterState)["MetaMan"] = &g_MetaMan;
		luabind::globals(m_MasterState)["MovableMan"] = &g_MovableMan;
		luabind::globals(m_MasterState)["ConsoleMan"] = &g_ConsoleMan;
		luabind::globals(m_MasterState)["LuaMan"] = &g_LuaMan;
		luabind::globals(m_MasterState)["SettingsMan"] = &g_SettingsMan;

		luaL_dostring(m_MasterState,
			// Override print() in the lua state to output to the console.
			"print = function(toPrint) ConsoleMan:PrintString(\"PRINT: \" .. tostring(toPrint)); end;\n"
			// Add cls() as a shortcut to ConsoleMan:Clear().
			"cls = function() ConsoleMan:Clear(); end;"
			// Add package path to the defaults.
			"package.path = package.path .. \";Base.rte/?.lua\";\n"
		);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Destroy() {
		lua_close(m_MasterState);

		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			FileClose(i);
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::ClearUserModuleCache() {
		luaL_dostring(m_MasterState, "for m, n in pairs(package.loaded) do if type(n) == \"boolean\" then package.loaded[m] = nil; end; end;");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaMan::GetNewPresetID() {
		char newID[16];
		std::snprintf(newID, sizeof(newID), "Pre%05li", m_NextPresetID);

		m_NextPresetID++;
		return std::string(newID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string LuaMan::GetNewObjectID() {
		char newID[16];
		std::snprintf(newID, sizeof(newID), "Obj%05li", m_NextObjectID);

		m_NextObjectID++;
		return std::string(newID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::RunScriptedFunction(const std::string &functionName, const std::string &selfObjectName, std::vector<std::string> variablesToSafetyCheck, std::vector<Entity *> functionEntityArguments, std::vector<std::string> functionLiteralArguments) {
		std::stringstream scriptString;
		if (!variablesToSafetyCheck.empty()) {
			scriptString << "if ";
			for (const std::string &variableToSafetyCheck : variablesToSafetyCheck) {
				if (&variableToSafetyCheck != &variablesToSafetyCheck[0]) { scriptString << " and "; }
				scriptString << variableToSafetyCheck;
			}
			scriptString << " then ";
		}
		if (!functionEntityArguments.empty()) { scriptString << "local entityArguments = LuaMan.TempEntities; "; }

		scriptString << functionName + "(" + selfObjectName;
		if (!functionEntityArguments.empty()) {
			SetTempEntityVector(functionEntityArguments);
			for (const Entity *functionEntityArgument : functionEntityArguments) {
				scriptString << ", (To" + functionEntityArgument->GetClassName() + " and To" + functionEntityArgument->GetClassName() + "(entityArguments()) or entityArguments())";
			}
		}
		if (!functionLiteralArguments.empty()) {
			for (const std::string &functionLiteralArgument : functionLiteralArguments) {
				scriptString << ", " + functionLiteralArgument;
			}
		}
		scriptString << ");";

		if (!variablesToSafetyCheck.empty()) { scriptString << " end;"; }

		return RunScriptString(scriptString.str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::RunScriptString(const std::string &scriptString, bool consoleErrors) {
		if (scriptString.empty()) {
			return -1;
		}
		int error = 0;

		lua_pushcfunction(m_MasterState, &AddFileAndLineToError);
		try {
			// Load the script string onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
			if (luaL_loadstring(m_MasterState, scriptString.c_str()) || lua_pcall(m_MasterState, 0, LUA_MULTRET, -2)) {
				// Retrieve the error message then pop it off the stack to clean it up
				m_LastError = lua_tostring(m_MasterState, -1);
				lua_pop(m_MasterState, 1);
				if (consoleErrors) {
					g_ConsoleMan.PrintString("ERROR: " + m_LastError);
					ClearErrors();
				}
				error = -1;
			}
		} catch (const std::exception &e) {
			m_LastError = e.what();
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}

		// Pop the file and line error handler off the stack to clean it up
		lua_pop(m_MasterState, 1);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::RunScriptFile(const std::string &filePath, bool consoleErrors) {
		if (filePath.empty()) {
			m_LastError = "Can't run a script file with an empty filepath!";
			return -1;
		}

		if (!System::PathExistsCaseSensitive(filePath)) {
			m_LastError = "Script file: " + filePath + " doesn't exist!";
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return -1;
		}

		int error = 0;

		lua_pushcfunction(m_MasterState, &AddFileAndLineToError);
		try {
			// Load the script file's contents onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
			if (luaL_loadfile(m_MasterState, filePath.c_str()) || lua_pcall(m_MasterState, 0, LUA_MULTRET, -2)) {
				// Retrieve the error message then pop it off the stack
				m_LastError = lua_tostring(m_MasterState, -1);
				lua_pop(m_MasterState, 1);
				if (consoleErrors) {
					g_ConsoleMan.PrintString("ERROR: " + m_LastError);
					ClearErrors();
				}
				error = -1;
			}
		} catch (const std::exception &e) {
			m_LastError = e.what();
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			error = -1;
		}

		// Pop the file and line error handler off the stack to clean it up
		lua_pop(m_MasterState, 1);

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::ExpressionIsTrue(std::string expression, bool consoleErrors) {
		if (expression.empty()) {
			return false;
		}
		bool result = false;

		try {
			// Push the script string onto the stack so we can execute it, and then actually try to run it
			// Assign the result to a dedicated temp global variable
			if (luaL_dostring(m_MasterState, (string("ExpressionResult = ") + expression + string(";")).c_str())) {
				// Retrieve and pop the error message off the stack
				m_LastError = string("When evaluating Lua expression: ") + lua_tostring(m_MasterState, -1);
				lua_pop(m_MasterState, 1);
				if (consoleErrors) {
					g_ConsoleMan.PrintString("ERROR: " + m_LastError);
					ClearErrors();
				}
				return false;
			}
		} catch (const std::exception &e) {
			m_LastError = string("When evaluating Lua expression: ") + e.what();
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return false;
		}
		// Get the result var onto the stack so we can check it
		lua_getglobal(m_MasterState, "ExpressionResult");
		// Now report if it is nil/null or not
		result = lua_toboolean(m_MasterState, -1);
		// Pop the result var so this operation is balanced and leaves the stack as it was
		lua_pop(m_MasterState, 1);

		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::SavePointerAsGlobal(void *objectToSave, const std::string &globalName) {
		// Push the pointer onto the Lua stack.
		lua_pushlightuserdata(m_MasterState, objectToSave);
		// Pop and assign that pointer to a global var in the Lua state.
		lua_setglobal(m_MasterState, globalName.c_str());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::GlobalIsDefined(const std::string &globalName) {
		// Get the var you want onto the stack so we can check it.
		lua_getglobal(m_MasterState, globalName.c_str());
		// Now report if it is nil/null or not.
		bool isDefined = !lua_isnil(m_MasterState, -1);
		// Pop the var so this operation is balanced and leaves the stack as it was.
		lua_pop(m_MasterState, 1);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::TableEntryIsDefined(const std::string &tableName, const std::string &indexName) {
		// Push the table onto the stack, checking if it even exists.
		lua_getglobal(m_MasterState, tableName.c_str());
		if (!lua_istable(m_MasterState, -1)) {
			// Clean up and report that there was nothing properly defined here.
			lua_pop(m_MasterState, 1);
			return false;
		}
		// Push the value at the requested index onto the stack so we can check if it's anything.
		lua_getfield(m_MasterState, -1, indexName.c_str());
		// Now report if it is nil/null or not
		bool isDefined = !lua_isnil(m_MasterState, -1);
		// Pop both the var and the table so this operation is balanced and leaves the stack as it was.
		lua_pop(m_MasterState, 2);

		return isDefined;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::FileOpen(const std::string &fileName, const std::string &accessMode) {
		int fileIndex = -1;
		for (int i = 0; i < c_MaxOpenFiles; ++i) {
			if (!m_OpenedFiles.at(i)) {
				fileIndex = i;
				break;
			}
		}
		if (fileIndex == -1) {
			g_ConsoleMan.PrintString("Error: Can't open file, maximum number of files already open.");
			return -1;
		}

		std::string fullPath = System::GetWorkingDirectory() + fileName;

		// Do not open paths with '..'
		if (fullPath.find("..") != std::string::npos) {
			return -1;
		}
		// Do not open paths that aren't written correctly
		if (!System::PathExistsCaseSensitive(std::filesystem::path(fileName).lexically_normal().generic_string())) {
			return -1;
		}
		// Allow to edit files only inside .rte folders
		if (fullPath.find(System::GetModulePackageExtension()) == std::string::npos) {
			return -1;
		}

		if (FILE * file = fopen(fullPath.c_str(), accessMode.c_str())) {
			m_OpenedFiles.at(fileIndex) = file;
			return fileIndex;
		}
		g_ConsoleMan.PrintString("Error: Failed to open file " + fullPath);
		return -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileClose(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			fclose(m_OpenedFiles.at(fileIndex));
			m_OpenedFiles.at(fileIndex) = nullptr;
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
			fgets(buf, 4095, m_OpenedFiles.at(fileIndex));
			return (std::string(buf));
		}
		g_ConsoleMan.PrintString("Error: Tried to read a closed file, or read past EOF.");
		return "";
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::FileWriteLine(int fileIndex, const std::string &line) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex)) {
			fputs(line.c_str(), m_OpenedFiles.at(fileIndex));
		} else {
			g_ConsoleMan.PrintString("Error: Tried to write to a closed file.");
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::FileEOF(int fileIndex) {
		if (fileIndex > -1 && fileIndex < c_MaxOpenFiles && m_OpenedFiles.at(fileIndex) && !feof(m_OpenedFiles.at(fileIndex))) {
			return false;
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Update() const {
		lua_gc(m_MasterState, LUA_GCSTEP, 1);
	}
}