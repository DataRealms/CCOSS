#include "GlobalScript.h"

#include "LuaMan.h"
#include "MovableMan.h"
#include "PresetMan.h"

#include "ACraft.h"
#include "PieSlice.h"

namespace RTE {

	ConcreteClassInfo(GlobalScript, Entity, 10);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GlobalScript::Clear() {
		m_ScriptPath.clear();
		m_LuaClassName.clear();
		m_IsActive = false;
		m_LateUpdate = false;
		m_PieSlicesToAdd.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::Create(const GlobalScript &reference) {
		Entity::Create(reference);

		m_ScriptPath = reference.m_ScriptPath;
		m_LuaClassName = reference.m_LuaClassName;
		m_IsActive = reference.m_IsActive;
		m_LateUpdate = reference.m_LateUpdate;

		for (const std::unique_ptr<PieSlice> &referencePieSliceToAdd : reference.m_PieSlicesToAdd) {
			m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice *>(referencePieSliceToAdd->Clone())));
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::ReadProperty(const std::string_view &propName, Reader &reader) {
		if (propName == "ScriptPath") {
			m_ScriptPath = CorrectBackslashesInPath(reader.ReadPropValue());
		} else if (propName == "LuaClassName") {
			reader >> m_LuaClassName;
		} else if (propName == "LateUpdate") {
			reader >> m_LateUpdate;
		} else if (propName == "AddPieSlice") {
			m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice *>(g_PresetMan.ReadReflectedPreset(reader))));
		} else {
			return Entity::ReadProperty(propName, reader);
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::Save(Writer &writer) const {
		Entity::Save(writer);

		writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);
		writer.NewPropertyWithValue("LuaClassName", m_LuaClassName);
		writer.NewPropertyWithValue("LateUpdate", m_LateUpdate);

		for (const std::unique_ptr<PieSlice> &pieSliceToAdd : m_PieSlicesToAdd) {
			writer.NewPropertyWithValue("AddPieSlice", pieSliceToAdd.get());
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::ReloadScripts() {
		int error = 0;

		if (!m_ScriptPath.empty()) {
			if (!g_LuaMan.GetMasterScriptState().GlobalIsDefined(m_LuaClassName)) {
				g_LuaMan.GetMasterScriptState().SetTempEntity(this);
				error = g_LuaMan.GetMasterScriptState().RunScriptString(m_LuaClassName + " = ToGlobalScript(LuaMan.TempEntity);");
			}
			if (error == 0) { g_LuaMan.GetMasterScriptState().RunScriptFile(m_ScriptPath); }
		}

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::Start() {
		int error = ReloadScripts();
		if (error == 0) { error = g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".StartScript then " + m_LuaClassName + ":StartScript(); end"); }
		m_IsActive = error == 0;

		return error;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::Pause(bool pause) const {
		return g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".PauseScript then " + m_LuaClassName + ":PauseScript(" + (pause ? "true" : "false") + "); end");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GlobalScript::End() const {
		return g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".EndScript then " + m_LuaClassName + ":EndScript(); end");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GlobalScript::HandleCraftEnteringOrbit(const ACraft *orbitedCraft) const {
		if (orbitedCraft && g_MovableMan.IsActor(orbitedCraft)) {
			g_LuaMan.GetMasterScriptState().RunScriptFunctionString(m_LuaClassName + ".CraftEnteredOrbit", m_LuaClassName, { m_LuaClassName, m_LuaClassName + ".CraftEnteredOrbit" }, { orbitedCraft });
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GlobalScript::Update() {
		int error = g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".UpdateScript then " + m_LuaClassName + ":UpdateScript(); end");
		if (error) { SetActive(false); }
	}
}