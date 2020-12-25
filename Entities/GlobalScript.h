#ifndef _RTEGLOBALSCRIPT_
#define _RTEGLOBALSCRIPT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GlobalScript.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the GlobalScript class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff

#include "Entity.h"
#include "Actor.h"

namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  GlobalScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The base class shared by Both TerrainObject:s and MovableObject:s, ie
//                  anything that can be places in a scene.
// Parent(s):       Entity.
// Class history:   8/6/2007 GlobalScript created.

class GlobalScript:
    public Entity
{

friend class LuaMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

EntityAllocation(GlobalScript)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GlobalScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GlobalScript object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GlobalScript() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GlobalScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GlobalScript object before deletion
//                  from system memory.
// Arguments:       None.

	~GlobalScript() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GlobalScript object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override { return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an GlobalScript to be identical to another, by deep copy.
// Arguments:       A reference to the GlobalScript to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const GlobalScript &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GlobalScript, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GlobalScript object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReloadScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the preset scripts of this object, from the same script file
//                  path as was originally defined. This will also update the original
//                  preset in the PresetMan with the updated scripts so future objects
//                  spawned will use the new scripts.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int ReloadScripts() override;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  IsActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the script is active or not. Active script can be deactivated
//					automatically if ti fails to execute it's Update function without errors to avoid
//					performance drain due to console prints caused by error messages.
// Arguments:       None.
// Return value:    True if script is active

	bool IsActive() const { return m_IsActive; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates or deactivates the script.
// Arguments:       Whether this script myst be active or not.
// Return value:    None.

	void SetActive( bool active) { m_IsActive = active; }

	//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Deactivate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Deactivates the script. Can be called during script's Update execution to 
//					stop it's processing when it's not needed anymore.
// Arguments:       None.
// Return value:    None.

	void Deactivate() { m_IsActive = false; } 

// Method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Starts the script.
// Arguments:       None.
// Return value:    Error code on error.

	int Start();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the script.
// Arguments:       Whether to pause the script or not.
// Return value:    None.

	void Pause(bool pause = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Ends the script.
// Arguments:       None.
// Return value:    None.

	void End();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this GlobalScript. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update();

    /// <summary>
    /// Indicates an Actor as having left the game scene and entered orbit.  OWNERSHIP IS NOT transferred, as the Actor's inventory is just 'unloaded'.
    /// </summary>
    /// <param name="orbitedActor">The actor instance that entered orbit. Ownership IS NOT TRANSFERRED!</param>
	void EnteredOrbit(Actor *orbitedCraft);

    /// <summary>
    /// Executes the Lua-defined OnPieMenu event handler for this global script.
    /// </summary>
    /// <param name="pieMenuActor">The actor which triggered the pie menu event.</param>
    /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	void OnPieMenu(Actor *pieMenuActor);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ShouldLateUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns true if this script should be updated after MovableMan instead of default update 
//					during ActivityMan
// Arguments:       None.
// Return value:    True if script must be updated after MovableMan

	bool ShouldLateUpdate() const { return m_LateUpdate; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Forbidding copying
	GlobalScript(const GlobalScript &reference) = delete;
    GlobalScript & operator=(const GlobalScript &rhs) { return *this; }

    // Member variables
    static Entity::ClassInfo m_sClass;

    // The name of the class (table) defining the logic of this in Lua, as specified in the script file
    std::string m_LuaClassName;
	// The path to the lua script file that defines this' behaviors in update
    std::string m_ScriptPath;
	// Whether this script allowed to run
	bool m_IsActive;

	// Whether the script should Update before MovableMan or after
	bool m_LateUpdate;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this GlobalScript, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File