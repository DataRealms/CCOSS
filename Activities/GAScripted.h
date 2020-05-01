#ifndef _RTEGASCRIPTED_
#define _RTEGASCRIPTED_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GAScripted.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GameActivity.h"
#include "GlobalScript.h"
#include "Box.h"

namespace RTE
{

class Actor;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GAScripted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Scripted activity
// Parent(s):       GameActivity.
// Class history:   07/03/2008 GAScripted created.

class GAScripted:
    public GameActivity
{

    friend class LuaMan;
    friend class ActivityMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(GAScripted)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GAScripted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GAScripted object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GAScripted() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GAScripted
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GAScripted object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~GAScripted() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GAScripted object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GAScripted object ready for use.
// Arguments:       The filepath to the script that defines this' Lua-defined derivation
//                  of this class.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(std::string scriptPath, std::string scriptClassName) { m_ScriptPath = scriptPath; m_LuaClassName = scriptClassName; return Create(); };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GAScripted to be identical to another, by deep copy.
// Arguments:       A reference to the GAScripted to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const GAScripted &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GAScripted, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Activity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this GAScripted to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the GAScripted will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GAScripted object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


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

    virtual int ReloadScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetLuaClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of the Lua-derived class defined in this' script.
// Arguments:       None.
// Return value:    A string with the friendly-formatted Lua type name of this object.

    virtual const std::string & GetLuaClassName() const { return m_LuaClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SceneIsCompatible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if a particular Scene supports this specific Activity on it.
//                  Usually that means certain Area:s need to be defined in the Scene.
// Arguments:       The Scene to check if it supports this Activiy. Ownership IS NOT TRANSFERRED!
//                  How many teams we're checking for. Some scenes may support and activity
//                  but only for a limited number of teams. If -1, not applicable.
// Return value:    Whether the Scene has the right stuff.

    virtual bool SceneIsCompatible(Scene *pScene, int teams = -1);


    /// <summary>
    /// Indicates an Actor as having left the game scene and entered orbit.  OWNERSHIP IS NOT transferred, as the Actor's inventory is just 'unloaded'.
    /// </summary>
    /// <param name="orbitedCraft">The actor instance that entered orbit. Ownership IS NOT TRANSFERRED!</param>
    virtual void EnteredOrbit(Actor *orbitedCraft);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the game accroding to parameters previously set.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Start();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

    virtual void Pause(bool pause = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

    virtual void End();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.
// Arguments:       None.
// Return value:    None.

    virtual void UpdateEditing();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates globals scripts loaded with this activity.
// Arguments:       Whether it's an early update, during Activity update, or late update, after MovableMan
// Return value:    None.

	virtual void UpdateGlobalScripts(bool lateUpdate);


    /// <summary>
    /// Calls this to be processed by derived classes to enable pie-menu dynamic change.
    /// </summary>
    /// <param name="pieMenuActor">The actor which triggered the pie menu event.</param>
	virtual void OnPieMenu(Actor *pieMenuActor);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
// Arguments:       A pointer to a screen-sized BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which screen's GUI to draw onto the bitmap.
// Return value:    None.

    virtual void DrawGUI(BITMAP *pTargetBitmap, const Vector& targetPos = Vector(), int which = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActivityMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector& targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollectRequiredAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through the script file and checks for any mentions and uses of
//                  Area:s that are required for this Activity to run in a Scene.
// Arguments:       None.
// Return value:    None.

    virtual void CollectRequiredAreas();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitAIs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through all Actor:s currently in the MovableMan and sets each
//                  one not controlled by a player to be AI controlled and AIMode setting
//                  based on team and CPU team.
// Arguments:       None.
// Return value:    None.

    virtual void InitAIs();


    // Member variables
    static Entity::ClassInfo m_sClass;

    // The path to the lua script file that defines this' behaviors with overrides of its virtual functions
    std::string m_ScriptPath;
    // The name of the class (table) defining the logic of this in Lua, as specified in the script file
    std::string m_LuaClassName;
    // The list of Area:s required in a Scene to play this Activity on it
    std::set<std::string> m_RequiredAreas;
    // The list of global scripts allowed to run during this activity
    std::vector<GlobalScript *> m_GlobalScriptsList;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Activity, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File