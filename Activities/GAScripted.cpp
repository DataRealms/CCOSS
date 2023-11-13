//////////////////////////////////////////////////////////////////////////////////////////
// File:            GAScripted.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GAScripted class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GAScripted.h"

#include "SceneMan.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "AudioMan.h"
#include "SettingsMan.h"
#include "AHuman.h"
#include "ACrab.h"
#include "ACraft.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Scene.h"
#include "Actor.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

#include "BuyMenuGUI.h"
#include "SceneEditorGUI.h"

namespace RTE {

ConcreteClassInfo(GAScripted, GameActivity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GAScripted, effectively
//                  resetting the members of this abstraction level only.

void GAScripted::Clear() {
    m_ScriptPath.clear();
    m_LuaClassName.clear();
    m_RequiredAreas.clear();
	m_PieSlicesToAdd.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GAScripted object ready for use.

int GAScripted::Create() {
    if (GameActivity::Create() < 0) {
        return -1;
    }

    if (m_Description.empty()) {
        m_Description = "A scripted activity without a description yet. Tell the creator to write one.";
    }

    // Gotto have a script file path and lua class names defined to Create
    if (m_ScriptPath.empty() || m_LuaClassName.empty()) {
        return -1;
    }

    // Scan the script file for any mentions/uses of Areas.
    CollectRequiredAreas();


    // If the GAScripted has a OnSave() function, we assume it can be saved by default
    ReloadScripts();
    m_AllowsUserSaving = HasSaveFunction();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GAScripted to be identical to another, by deep copy.

int GAScripted::Create(const GAScripted &reference) {
    if (GameActivity::Create(reference) < 0) {
        return -1;
    }

    m_ScriptPath = reference.m_ScriptPath;
    m_LuaClassName = reference.m_LuaClassName;
	for (const std::string &referenceRequiredArea : reference.m_RequiredAreas) {
		m_RequiredAreas.emplace(referenceRequiredArea);
	}
	for (const std::unique_ptr<PieSlice> &referencePieSliceToAdd : reference.m_PieSlicesToAdd) {
		m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice *>(referencePieSliceToAdd->Clone())));
	}

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int GAScripted::ReadProperty(const std::string_view &propName, Reader &reader) {
	StartPropertyList(return GameActivity::ReadProperty(propName, reader));
    
    MatchProperty("ScriptPath", {
		m_ScriptPath = g_PresetMan.GetFullModulePath(reader.ReadPropValue());
    });
    MatchProperty("LuaClassName", {
		reader >> m_LuaClassName;
    });
    MatchProperty("AddPieSlice", {
		m_PieSlicesToAdd.emplace_back(std::unique_ptr<PieSlice>(dynamic_cast<PieSlice *>(g_PresetMan.ReadReflectedPreset(reader)))); 
    });

    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GAScripted with a Writer for
//                  later recreation with Create(Reader &reader);

int GAScripted::Save(Writer &writer) const {
    // Hmm. We should probably be calling this prior to the writer Save, instead of const-casting.
    const_cast<GAScripted*>(this)->RunLuaFunction("OnSave");

    GameActivity::Save(writer);

	writer.NewPropertyWithValue("ScriptPath", m_ScriptPath);
	writer.NewPropertyWithValue("LuaClassName", m_LuaClassName);

	for (const std::unique_ptr<PieSlice> &pieSliceToAdd : m_PieSlicesToAdd) {
		writer.NewPropertyWithValue("AddPieSlice", pieSliceToAdd.get());
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GAScripted object.

void GAScripted::Destroy(bool notInherited) {
	// Delete global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		delete (*sItr);
    }

	m_GlobalScriptsList.clear();

    if (!notInherited) {
        GameActivity::Destroy();
    }

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReloadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the preset scripts of this object, from the same script file
//                  path as was originally defined. This will also update the original
//                  preset in the PresetMan with the updated scripts so future objects
//                  spawned will use the new scripts.

int GAScripted::ReloadScripts() {
    if (m_ScriptPath.empty()) {
        return 0;
    }

    int error = 0;
    CollectRequiredAreas();

    // If it hasn't been yet, run the file that specifies the Lua functions for this' operating logic (including the scene test function)
    if (!g_LuaMan.GetMasterScriptState().GlobalIsDefined(m_LuaClassName)) {
        // Temporarily store this Activity so the Lua state can access it
        g_LuaMan.GetMasterScriptState().SetTempEntity(this);

        // Define the var that will hold the script file definitions
        if ((error = g_LuaMan.GetMasterScriptState().RunScriptString(m_LuaClassName + " = ToGameActivity(LuaMan.TempEntity);")) < 0) {
            return error;
        }
    }

    std::unordered_map<std::string, LuabindObjectWrapper*> scriptFileFunctions;
    if ((error = g_LuaMan.GetMasterScriptState().RunScriptFileAndRetrieveFunctions(m_ScriptPath, m_LuaClassName, GetSupportedScriptFunctionNames(), scriptFileFunctions, true)) < 0) {
        return error;
    }

    m_ScriptFunctions.clear();
    for (const auto& [functionName, functionObject] : scriptFileFunctions) {
        m_ScriptFunctions[functionName] = std::unique_ptr<LuabindObjectWrapper>(functionObject);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GAScripted::HasSaveFunction() const {
    return m_ScriptFunctions.find("OnSave") != m_ScriptFunctions.end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SceneIsCompatible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if a particular Scene supports this specific Activity on it.
//                  Usually that means certain Area:s need to be defined in the Scene.

bool GAScripted::SceneIsCompatible(Scene *pScene, int teams) {
    if (!GameActivity::SceneIsCompatible(pScene, teams)) {
        return false;
    }

    // Check if all Areas required by this are defined in the Scene
    for (std::set<std::string>::iterator itr = m_RequiredAreas.begin(); itr != m_RequiredAreas.end(); ++itr) {
        // If Area is missing, this Scene is not up to par
        if (!pScene->HasArea(*itr)) {
            return false;
        }
    }

    // Temporarily store the scene so the Lua state can access it and check for the necessary Areas etc.
    g_LuaMan.GetMasterScriptState().SetTempEntity(pScene);
    // Cast the test scene it to a Scene object in Lua
    if (g_LuaMan.GetMasterScriptState().RunScriptString("TestScene = ToScene(LuaMan.TempEntity);") < 0) {
        return false;
    }

    // If it hasn't been yet, run the file that specifies the Lua functions for this' operating logic (including the scene test function)
    if (!g_LuaMan.GetMasterScriptState().GlobalIsDefined(m_LuaClassName)) {
        // Temporarily store this Activity so the Lua state can access it
        g_LuaMan.GetMasterScriptState().SetTempEntity(this);
        // Define the var that will hold the script file definitions..
        // it's OK if the script fails, then the scene is still deemed compatible
        if (g_LuaMan.GetMasterScriptState().RunScriptString(m_LuaClassName + " = ToGameActivity(LuaMan.TempEntity);") < 0) {
            return true;
        }
        // Load and run the file, defining all the scripted functions of this Activity
        if (g_LuaMan.GetMasterScriptState().RunScriptFile(m_ScriptPath) < 0) {
            return true;
        }
    }

    // Call the defined function, but only after first checking if it exists
    g_LuaMan.GetMasterScriptState().RunScriptString("if " + m_LuaClassName + ".SceneTest then " + m_LuaClassName + ":SceneTest(); end");

    // If the test left the Scene pointer still set, it means it passed the test
    return g_LuaMan.GetMasterScriptState().GlobalIsDefined("TestScene");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GAScripted::HandleCraftEnteringOrbit(ACraft *orbitedCraft) {
    GameActivity::HandleCraftEnteringOrbit(orbitedCraft);

    if (orbitedCraft && g_MovableMan.IsActor(orbitedCraft)) {
        g_LuaMan.GetMasterScriptState().RunScriptFunctionString(m_LuaClassName + ".CraftEnteredOrbit", m_LuaClassName, {m_LuaClassName, m_LuaClassName + ".CraftEnteredOrbit"}, {orbitedCraft});
        for (const GlobalScript *globalScript : m_GlobalScriptsList) {
            if (globalScript->IsActive()) { globalScript->HandleCraftEnteringOrbit(orbitedCraft); }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GAScripted::Start() {
	ActivityState initialActivityState = m_ActivityState;

    int error = GameActivity::Start();
    if (error < 0) {
        return error;
    }

    // Temporarily store this Activity so the Lua state can access it
    g_LuaMan.GetMasterScriptState().SetTempEntity(this);

    // Define the var that will hold the script file definitions
    if ((error = g_LuaMan.GetMasterScriptState().RunScriptString(m_LuaClassName + " = ToGameActivity(LuaMan.TempEntity);")) < 0) {
        return error;
    }

    // Run the file that specifies the Lua functions for this' operating logic
    if ((error = ReloadScripts()) < 0) {
        return error;
    }

    // Call the create function
    if ((error = RunLuaFunction("StartActivity", {}, { initialActivityState == ActivityState::NotStarted ? "true" : "false" }, {})) < 0) {
        return error;
    }

	// Clear active global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		delete (*sItr);
    }

	m_GlobalScriptsList.clear();

	// Get all global scripts and add to execution list
	std::list<Entity *> globalScripts;
	g_PresetMan.GetAllOfType(globalScripts, "GlobalScript");

	for (std::list<Entity *>::iterator sItr = globalScripts.begin(); sItr != globalScripts.end(); ++sItr) {
		GlobalScript * script = dynamic_cast<GlobalScript *>(*sItr);
        if (script && g_SettingsMan.IsGlobalScriptEnabled(script->GetModuleAndPresetName())) {
            m_GlobalScriptsList.push_back(dynamic_cast<GlobalScript*>(script->Clone()));
        }
	}

	// Start all global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		if (g_SettingsMan.PrintDebugInfo()) {
			g_ConsoleMan.PrintString("DEBUG: Start Global Script: " + (*sItr)->GetPresetName());
        }

		(*sItr)->Start();
	}

    return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GAScripted::SetPaused(bool pause) {
    GameActivity::SetPaused(pause);

    RunLuaFunction("PauseActivity", {}, { pause ? "true" : "false" }, {});

	// Pause all global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		if ((*sItr)->IsActive()) {
			(*sItr)->Pause(pause);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GAScripted::End() {
    GameActivity::End();

    RunLuaFunction("EndActivity");

	// End all global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		if ((*sItr)->IsActive()) {
			if (g_SettingsMan.PrintDebugInfo()) {
				g_ConsoleMan.PrintString("DEBUG: End Global Script: " + (*sItr)->GetPresetName());
            }
			(*sItr)->End();
		}
    }

	// Delete all global scripts, in case destructor is not called when activity restarts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		delete (*sItr);
    }

	m_GlobalScriptsList.clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.

void GAScripted::UpdateEditing() {
    GameActivity::UpdateEditing();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GAScripted. Supposed to be done every frame
//                  before drawing.

void GAScripted::Update() {
    GameActivity::Update();

    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
        if (!(m_IsActive[player] && m_IsHuman[player])) {
            continue;
        }

        // The current player's team
        int team = m_Team[player];
        if (team == Teams::NoTeam) {
            continue;
        }
    }

    // If the game didn't end, keep updating activity
    if (m_ActivityState != ActivityState::Over) {
		AddPieSlicesToActiveActorPieMenus();

        RunLuaFunction("UpdateActivity");

        UpdateGlobalScripts(false);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates globals scripts loaded with this activity.

void GAScripted::UpdateGlobalScripts(bool lateUpdate) {
	// Update all global scripts
	for (std::vector<GlobalScript *>::iterator sItr = m_GlobalScriptsList.begin(); sItr < m_GlobalScriptsList.end(); ++sItr) {
		if ((*sItr)->IsActive() && (*sItr)->ShouldLateUpdate() == lateUpdate) {
			(*sItr)->Update();
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

void GAScripted::DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos, int which) {
    GameActivity::DrawGUI(pTargetBitmap, targetPos, which);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this GAScripted's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.

void GAScripted::Draw(BITMAP *pTargetBitmap, const Vector &targetPos) {
    GameActivity::Draw(pTargetBitmap, targetPos);
}

int GAScripted::RunLuaFunction(const std::string& functionName, const std::vector<const Entity*>& functionEntityArguments, const std::vector<std::string_view>& functionLiteralArguments, const std::vector<LuabindObjectWrapper*>& functionObjectArguments) {
    // Call the defined function, but only after first checking if it exists
    auto funcItr = m_ScriptFunctions.find(functionName);
    if (funcItr == m_ScriptFunctions.end()) {
        return 0;
    }

    return g_LuaMan.GetMasterScriptState().RunScriptFunctionObject(funcItr->second.get(), "_G", m_LuaClassName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollectRequiredAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through the script file and checks for any mentions and uses of
//                  Area:s that are required for this Activity to run in a Scene.

void GAScripted::CollectRequiredAreas() {
    // Open the script file so we can check it out
    std::ifstream scriptFile = std::ifstream(g_PresetMan.GetFullModulePath(m_ScriptPath.c_str()));
    if (!scriptFile.good()) {
        return;
    }

    // Harvest the required Area:s from the file
    m_RequiredAreas.clear();

    bool blockCommented = false;

    while (!scriptFile.eof()) {
        // Go through the script file, line by line
        char rawLine[512];
        scriptFile.getline(rawLine, 512);
        std::string line = rawLine;
        std::string::size_type pos = 0;
        std::string::size_type endPos = 0;
        std::string::size_type commentPos = std::string::npos;

        // Check for block comments
        if (!blockCommented && (commentPos = line.find("--[[", 0)) != std::string::npos) {
            blockCommented = true;
        }

        // Find the end of the block comment
        if (blockCommented) {
            if ((commentPos = line.find("]]", commentPos == std::string::npos ? 0 : commentPos)) != std::string::npos) {
                blockCommented = false;
                pos = commentPos;
            }
        }

        // Process the line as usual
        if (!blockCommented) {
            // See if this line is commented out anywhere
            commentPos = line.find("--", 0);
            do {
                // Find the beginning of a mentioned Area name
                pos = line.find(":GetArea(\"", pos);
                if (pos != std::string::npos && pos < commentPos) {
                    // Move position forward to the actual Area name
                    pos += 10;
                    // Find the end of the Area name
                    endPos = line.find_first_of('"', pos);
                    // Copy it out and put into the list
                    if (endPos != std::string::npos) {
                        m_RequiredAreas.insert(line.substr(pos, endPos - pos));
                    }
                }
            } while(pos != std::string::npos && pos < commentPos);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GAScripted::AddPieSlicesToActiveActorPieMenus() {
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		if (m_IsActive[player] && m_IsHuman[player] && m_ControlledActor[player] && m_ViewState[player] != ViewState::DeathWatch && m_ViewState[player] != ViewState::ActorSelect && m_ViewState[player] != ViewState::AIGoToPoint && m_ViewState[player] != ViewState::UnitSelectCircle) {
			PieMenu *controlledActorPieMenu = m_ControlledActor[player]->GetPieMenu();
			if (controlledActorPieMenu && m_ControlledActor[player]->GetController()->IsState(PIE_MENU_ACTIVE) && controlledActorPieMenu->IsEnabling()) {
				for (const std::unique_ptr<PieSlice> &pieSlice : m_PieSlicesToAdd) {
					controlledActorPieMenu->AddPieSliceIfPresetNameIsUnique(pieSlice.get(), this, true);
				}
				for (const GlobalScript *globalScript : m_GlobalScriptsList) {
					if (globalScript->IsActive()) {
						for (const std::unique_ptr<PieSlice> &pieSlice : globalScript->GetPieSlicesToAdd()) {
							controlledActorPieMenu->AddPieSliceIfPresetNameIsUnique(pieSlice.get(), globalScript, true);
						}
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace RTE