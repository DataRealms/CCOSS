//////////////////////////////////////////////////////////////////////////////////////////
// File:            GlobalScript.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the GlobalScript class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GlobalScript.h"
#include "PresetMan.h"
#include "LuaMan.h"
#include "MovableMan.h"

namespace RTE {

ConcreteClassInfo(GlobalScript, Entity, 10);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GlobalScript, effectively
//                  resetting the members of this abstraction level only.

void GlobalScript::Clear()
{
    m_ScriptPath.clear();
    m_LuaClassName.clear();
	m_IsActive = false;
	m_LateUpdate = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an GlobalScript object to be identical to another, by deep copy.

int GlobalScript::Create(const GlobalScript &reference)
{
    Entity::Create(reference);

    m_ScriptPath = reference.m_ScriptPath;
    m_LuaClassName = reference.m_LuaClassName;
	m_IsActive = reference.m_IsActive;
	m_LateUpdate = reference.m_LateUpdate;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.

int GlobalScript::ReadProperty(const std::string_view &propName, Reader &reader)
{
	if (propName == "ScriptPath") {
		m_ScriptPath = CorrectBackslashesInPath(reader.ReadPropValue());
	} else if (propName == "LuaClassName")
        reader >> m_LuaClassName;
	else if (propName == "LateUpdate")
		reader >> m_LateUpdate;
	else if (propName == "AddPieSlice")
	{
		PieSlice newSlice;
		reader >> newSlice;
		PieMenuGUI::StoreCustomLuaPieSlice(newSlice);
	}
	else
        return Entity::ReadProperty(propName, reader);

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GlobalScript to an output stream for
//                  later recreation with Create(istream &stream);

int GlobalScript::Save(Writer &writer) const
{
    Entity::Save(writer);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReloadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the preset scripts of this object, from the same script file
//                  path as was originally defined. This will also update the original
//                  preset in the PresetMan with the updated scripts so future objects
//                  spawned will use the new scripts.

int GlobalScript::ReloadScripts()
{
    int error = 0;

    // Read in the Lua script function definitions for this preset
    if (!m_ScriptPath.empty())
    {
        // If it hasn't been yet, run the file that specifies the Lua functions for this' operating logic (including the scene test function)
        if (!g_LuaMan.GlobalIsDefined(m_LuaClassName))
        {
            // Temporarily store this Activity so the Lua state can access it
            g_LuaMan.SetTempEntity(this);
            // Define the var that will hold the script file definitions
            if ((error = g_LuaMan.RunScriptString(m_LuaClassName + " = ToGlobalScript(LuaMan.TempEntity);")) < 0)
                return error;
        }

        // Load and run the file, defining all the scripted functions of this Activity
        if ((error = g_LuaMan.RunScriptFile(m_ScriptPath)) < 0)
            return error;
    }

    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GlobalScript::EnteredOrbit(Actor *orbitedActor) {
    if (orbitedActor && g_MovableMan.IsActor(orbitedActor)) {
        g_LuaMan.RunScriptedFunction(m_LuaClassName + ".CraftEnteredOrbit", m_LuaClassName, {m_LuaClassName, m_LuaClassName + ".CraftEnteredOrbit"}, {orbitedActor});
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GlobalScript::OnPieMenu(Actor *pieMenuActor) {
	if (pieMenuActor && g_MovableMan.IsActor(pieMenuActor)) {
        g_LuaMan.RunScriptedFunction(m_LuaClassName + ".OnPieMenu", m_LuaClassName, {m_LuaClassName, m_LuaClassName + ".OnPieMenu"}, {pieMenuActor});
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.

int GlobalScript::Start()
{
	int error = 0;
	m_IsActive = false;

    g_LuaMan.SetTempEntity(this);
    // Create the Lua variable which will hold the class representation that we'll add some definitions to
    if ((error = g_LuaMan.RunScriptString(m_LuaClassName + " = ToGlobalScript(LuaMan.TempEntity);")) < 0)
        return error;

    // Run the file that specifies the Lua functions for this' operating logic
    if ((error = g_LuaMan.RunScriptFile(m_ScriptPath)) < 0)
        return error;

	// Activate the script
	m_IsActive = true;

    // Call the defined function, but only after first checking if it exists
    if ((error = g_LuaMan.RunScriptString("if " + m_LuaClassName + ".StartScript then " + m_LuaClassName + ":StartScript(); end")) < 0)
	{
		// Deactivate if it failed to start
		m_IsActive = false;
        return error;
	}

	return error;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.

void GlobalScript::Pause(bool pause)
{
    // Call the defined function, but only after first checking if it exists
    g_LuaMan.RunScriptString("if " + m_LuaClassName + ".PauseScript then " + m_LuaClassName + ":PauseScript(" + (pause ? "true" : "false") + "); end");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.

void GlobalScript::End()
{
    // Call the defined function, but only after first checking if it exists
    g_LuaMan.RunScriptString("if " + m_LuaClassName + ".EndScript then " + m_LuaClassName + ":EndScript(); end");
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this GAScripted. Supposed to be done every frame
//                  before drawing.

void GlobalScript::Update()
{
    // Call the defined function, but only after first checking if it exists
    int error = g_LuaMan.RunScriptString("if " + m_LuaClassName + ".UpdateScript then " + m_LuaClassName + ":UpdateScript(); end");
	// Kill script on any error to avoid spamming the console with error messages
	if (error)
		Deactivate();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GlobalScript object.

void GlobalScript::Destroy(bool notInherited)
{
    if (!notInherited)
        Entity::Destroy();
    Clear();
}

} // namespace RTE