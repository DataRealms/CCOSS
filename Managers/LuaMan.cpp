#include "LuaMan.h"

#include "LuaBindingRegisterDefinitions.h"
#include "LuaAdapters.h"
#include "LuaAdaptersEntities.h"

#include "Reader.h"
#include "Writer.h"

#include "ActivityMan.h"
#include "GAScripted.h"
#include "Scene.h"
#include "SceneMan.h"

namespace RTE {

	const std::unordered_set<std::string> LuaMan::c_FileAccessModes = { "r", "r+", "w", "w+", "a", "a+" };
	constexpr static char* sc_scriptSavesPath = "Saves.rte/";

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
		//set_pcall_callback(&AddFileAndLineToError); // NOTE: this seems to do nothing because retrieving the error from the lua stack wasn't done correctly. The current error handling works just fine but might look into doing this properly sometime later.

		// Register all relevant bindings to the master state. Note that the order of registration is important, as bindings can't derive from an unregistered type (inheritance and all that).
		luabind::module(m_MasterState)[
			luabind::class_<LuaMan>("LuaManager")
				.property("TempEntity", &LuaMan::GetTempEntity)
				.def_readonly("TempEntities", &LuaMan::m_TempEntityVector, luabind::return_stl_iterator)
				.def("FileOpen", &LuaMan::FileOpen)
				.def("FileClose", &LuaMan::FileClose)
				.def("FileReadLine", &LuaMan::FileReadLine)
				.def("FileWriteLine", &LuaMan::FileWriteLine)
				.def("FileEOF", &LuaMan::FileEOF)
				.def("SaveScene", &LuaMan::SaveScriptedScene)
				.def("LoadScene", &LuaMan::LoadScriptedScene),

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
			luabind::def("NormalizeAngleBetween0And2PI", &NormalizeAngleBetween0And2PI),
			luabind::def("NormalizeAngleBetweenNegativePIAndPI", &NormalizeAngleBetweenNegativePIAndPI),
			luabind::def("AngleWithinRange", &AngleWithinRange),
			luabind::def("ClampAngle", &ClampAngle),
			luabind::def("GetPPM", &GetPPM),
			luabind::def("GetMPP", &GetMPP),
			luabind::def("GetPPL", &GetPPL),
			luabind::def("GetLPP", &GetLPP),
			luabind::def("RoundFloatToPrecision", &RoundFloatToPrecision),

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
			RegisterLuaBindingsOfType(SystemLuaBindings, Controller),
			RegisterLuaBindingsOfType(SystemLuaBindings, Timer),
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
			RegisterLuaBindingsOfType(MiscLuaBindings, AlarmEvent),
			RegisterLuaBindingsOfType(MiscLuaBindings, InputDevice),
			RegisterLuaBindingsOfType(MiscLuaBindings, InputElements),
			RegisterLuaBindingsOfType(MiscLuaBindings, JoyButtons),
			RegisterLuaBindingsOfType(MiscLuaBindings, JoyDirections),
			RegisterLuaBindingsOfType(MiscLuaBindings, MouseButtons),
			RegisterLuaBindingsOfType(MiscLuaBindings, Directions)
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

	void LuaMan::SetTempEntityVector(const std::vector<const Entity *> &entityVector) {
		m_TempEntityVector.reserve(entityVector.size());
		for (const Entity *entity : entityVector) {
			m_TempEntityVector.emplace_back(const_cast<Entity *>(entity));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int LuaMan::RunScriptedFunction(const std::string &functionName, const std::string &selfObjectName, const std::vector<std::string_view> &variablesToSafetyCheck, const std::vector<const Entity *> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments) {
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

	int LuaMan::RunScriptString(const std::string &scriptString, bool consoleErrors) {
		if (scriptString.empty()) {
			return -1;
		}
		int error = 0;

		lua_pushcfunction(m_MasterState, &AddFileAndLineToError);
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
		// Load the script file's contents onto the stack and then execute it with pcall. Pcall will call the file and line error handler if there's an error by pointing 2 up the stack to it.
		if (luaL_loadfile(m_MasterState, filePath.c_str()) || lua_pcall(m_MasterState, 0, LUA_MULTRET, -2)) {
			m_LastError = lua_tostring(m_MasterState, -1);
			lua_pop(m_MasterState, 1);
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

	bool LuaMan::ExpressionIsTrue(const std::string &expression, bool consoleErrors) {
		if (expression.empty()) {
			return false;
		}
		bool result = false;

		// Push the script string onto the stack so we can execute it, and then actually try to run it. Assign the result to a dedicated temp global variable.
		if (luaL_dostring(m_MasterState, std::string("ExpressionResult = " + expression + ";").c_str())) {
			m_LastError = std::string("When evaluating Lua expression: ") + lua_tostring(m_MasterState, -1);
			lua_pop(m_MasterState, 1);
			if (consoleErrors) {
				g_ConsoleMan.PrintString("ERROR: " + m_LastError);
				ClearErrors();
			}
			return false;
		}
		// Put the result of the expression on the lua stack and check its value. Need to pop it off the stack afterwards so it leaves the stack unchanged.
		lua_getglobal(m_MasterState, "ExpressionResult");
		result = lua_toboolean(m_MasterState, -1);
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

		std::string fullPath = System::GetWorkingDirectory() + fileName;
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

	bool LuaMan::SaveScriptedScene(const std::string &fileName) {
		Scene* scene = dynamic_cast<Scene*>(g_SceneMan.GetScene());
		GAScripted* activity = dynamic_cast<GAScripted*>(g_ActivityMan.GetActivity());

		if (!scene || !activity) {
			return false;
		}

		// Save the scene bitmaps
		if (scene->SaveData(sc_scriptSavesPath + fileName) < 0) {
			return false;
		}

		// We need a copy of our scene, because we have to do some fixup to remove PLACEONLOAD items and only keep the current g_movableMan state
		Scene* sceneAltered = dynamic_cast<Scene*>(g_SceneMan.GetScene()->Clone());

		// Delete any existing objects from our scene - we don't want replace broken doors or repair any stuff when we load
		sceneAltered->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, true);

		// Pull all stuff from movableMan into the scene for saving, so existing actors/doors are transfered
		sceneAltered->RetrieveSceneObjects(false); // Don't transfer ownership

		// We also need to stop being a copy of the scene we got cloned from - otherwise we'll still pick up the PlacedObjectSets from our parent when loading
		// So become our own original preset
		sceneAltered->SetPresetName(sceneAltered->GetPresetName() + " - " + fileName);
		sceneAltered->SetScriptSave(true);

		// We don't need to block the main thread for too long, just need to pause update for a little bit, until the writer has all the data
		g_TimerMan.PauseSim(true);

		auto saveWriterData = [sceneAltered, activity](const std::string& filename) {
			Writer writer(filename.c_str());
			writer.NewPropertyWithValue("Scene", sceneAltered);
			writer.NewPropertyWithValue("Activity", activity);

			// We have all the data in the writer
			// Now we can continue and the flushing to disk (upon scope exit) can be done concurrently with the simulation
			g_TimerMan.PauseSim(false);

			// We didn't transfer ownership, so we must be very careful that sceneAltered's deletion doesn't touch the stuff we got from MovableMan
			sceneAltered->ClearPlacedObjectSet(Scene::PlacedObjectSets::PLACEONLOAD, false);

			delete sceneAltered;
		};

		std::thread saveThread(saveWriterData, sc_scriptSavesPath + fileName + ".ini");
		saveThread.detach();

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool LuaMan::LoadScriptedScene(const std::string &fileName) {
		Scene* scene = new Scene();
		GAScripted* activity = new GAScripted();

 		Reader reader((sc_scriptSavesPath + fileName + ".ini").c_str(), true, nullptr, true);
		if (!reader.ReaderOK()) {
			return false;
		}

		while (reader.NextProperty()) {
			std::string propName = reader.ReadPropName();
        	if (propName == "Scene") {
				reader >> scene;
			} else if (propName == "Activity") {
				reader >> activity;
			}
    	}

		// Copy our scene/activity over, and then free the memory
		g_SceneMan.SetSceneToLoad(scene, true, true); // SetSceneToLoad() doesn't Clone(), but when the activity starts, it will eventually call LoadScene(), which does a Clone() of scene internally
		g_ActivityMan.StartActivity(dynamic_cast<GAScripted*>(activity->Clone())); // But for activity we need to do the Clone() here
		// Messy, right? :/

		delete scene;
		delete activity;

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LuaMan::Update() const {
		lua_gc(m_MasterState, LUA_GCSTEP, 1);
	}
}
