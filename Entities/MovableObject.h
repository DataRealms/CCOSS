#ifndef _RTEMOVABLEOBJECT_
#define _RTEMOVABLEOBJECT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MovableObject.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MovableObject class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff

#include "SceneObject.h"
#include "Vector.h"
#include "Matrix.h"
#include "Timer.h"
#include "Material.h"
#include "MovableMan.h"
#include "FrameMan.h"

struct BITMAP;

namespace RTE
{


#pragma region Global Macro Definitions
    #define ScriptFunctionNames(...) \
        virtual std::vector<std::string> GetSupportedScriptFunctionNames() const { return {__VA_ARGS__}; }

    #define AddScriptFunctionNames(PARENT, ...) \
        std::vector<std::string> GetSupportedScriptFunctionNames() const override { \
            std::vector<std::string> functionNames = PARENT::GetSupportedScriptFunctionNames(); \
            functionNames.insert(functionNames.end(), {__VA_ARGS__}); \
            return functionNames; \
        }
#pragma endregion

struct HitData;

class MOSRotating;

//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  MovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A movable object with mass.
// Parent(s):       SceneObject.
// Class history:   03/18/2001 MovableObject created.

class MovableObject:
    public SceneObject
{

friend class Atom;
friend struct EntityLuaBindings;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	ScriptFunctionNames("Create", "Destroy", "Update", "OnScriptDisable", "OnScriptEnable", "OnPieMenu", "OnCollideWithTerrain", "OnCollideWithMO")
	SerializableOverrideMethods
	ClassInfoGetters

enum MOType
{
    TypeGeneric = 0,
    TypeActor,
    TypeHeldDevice,
    TypeThrownDevice
};



//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MovableObject object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MovableObject() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MovableObject object before deletion
//                  from system memory.
// Arguments:       None.

	~MovableObject() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableObject object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableObject object ready for use.
// Arguments:       A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  The rotation angle in r.
//                  The angular velocity in r/s.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
//                  Whether or not this MO will collide with other MO's while travelling.
//                  Whether or not this MO be collided with bt other MO's during their travel.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(float mass, const Vector &position = Vector(0, 0), const Vector &velocity = Vector(0, 0), float rotAngle = 0, float angleVel = 0, unsigned long lifetime = 0, bool hitMOs = true, bool getHitByMOs = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MovableObject to be identical to another, by deep copy.
// Arguments:       A reference to the MovableObject to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const MovableObject &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MovableObject, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); SceneObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MovableObject object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;

    /// <summary>
    /// Loads the script at the given script path onto the object, checking for appropriately named functions within it.
    /// </summary>
    /// <param name="scriptPath">The path to the script to load.</param>
    /// <param name="loadAsEnabledScript">Whether or not the script should load as enabled. Defaults to true.</param>
    /// <returns>0 on success. -1 if scriptPath is empty. -2 if the script is already loaded. -3 if setup to load the script or modify the global lua state fails. -4 if the script fails to load.</returns>
    virtual int LoadScript(const std::string &scriptPath, bool loadAsEnabledScript = true);

    /// <summary>
    /// Reloads the all of the scripts on this object. This will also reload scripts for the original preset in PresetMan so future objects spawned will use the new scripts.
    /// </summary>
    /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	int ReloadScripts();

    /// <summary>
    /// Checks if this MO has any scripts on it.
    /// </summary>
    /// <returns>Whether or not this MO has any scripts on it.</returns>
	bool HasAnyScripts() const { return !m_AllLoadedScripts.empty(); }

    /// <summary>
    /// Checks if the script at the given path is one of the scripts on this MO.
    /// </summary>
    /// <param name="scriptPath">The path to the script to check.</param>
    /// <returns>Whether or not the script is on this MO.</returns>
	bool HasScript(const std::string &scriptPath) const { return m_AllLoadedScripts.find(scriptPath) != m_AllLoadedScripts.end(); }

    /// <summary>
    /// Adds the script at the given path as one of the scripts on this MO.
    /// </summary>
    /// <param name="scriptPath">The path to the script to add.</param>
    /// <returns>Whether or not the script was successfully added.</returns>
	bool AddScript(const std::string &scriptPath);

    /// <summary>
    /// Checks if the script at the given path is one of the enabled scripts on this MO.
    /// </summary>
    /// <param name="scriptPath">The path to the script to check.</param>
    /// <returns>Whether or not the script is enabled on this MO.</returns>
    bool ScriptEnabled(const std::string &scriptPath) const { std::map<std::string, bool>::const_iterator scriptPathIterator = m_AllLoadedScripts.find(scriptPath); return scriptPathIterator != m_AllLoadedScripts.end() && scriptPathIterator->second == true; }

    /// <summary>
    /// Enable the script at the given path on this MO.
    /// </summary>
    /// <param name="scriptPath">The path to the script to enable.</param>
    /// <returns>Whether or not the script was successfully enabled.</returns>
	bool EnableScript(const std::string &scriptPath);

    /// <summary>
    /// Disables the script at the given path for this MO.
    /// </summary>
    /// <param name="scriptPath">The path to the script to disable.</param>
    /// <returns>Whether or not the script was successfully disabled..</returns>
	bool DisableScript(const std::string &scriptPath);

    /// <summary>
    /// Enables or disables all scripts on this MovableObject.
    /// </summary>
    /// <param name="enableScripts">Whether to enable (true) or disable (false) all scripts on this MovableObject.</param>
    void EnableOrDisableAllScripts(bool enableScripts);

    /// <summary>
    /// Runs the given function for the given script, with the given arguments. The first argument to the function will always be 'self'.
    /// If either argument list is not empty, its entries will be passed into the Lua function in order, with entity arguments first.
    /// </summary>
    /// <param name="scriptPath">The path to the script to run.</param>
    /// <param name="functionName">The name of the function to run.</param>
    /// <param name="functionEntityArguments">Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.</param>
    /// <param name="functionLiteralArguments">Optional vector of strings, that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.</param>
    /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
    int RunScriptedFunction(const std::string &scriptPath, const std::string &functionName, const std::vector<Entity *> &functionEntityArguments = std::vector<Entity *>(), const std::vector<std::string> &functionLiteralArguments = std::vector<std::string>()) const;

    /// <summary>
    /// Runs the given function in all scripts that have it, with the given arguments, with the ability to not run on disabled scripts and to cease running if there's an error.
    /// The first argument to the function will always be 'self'. If either argument list is not empty, its entries will be passed into the Lua function in order, with entity arguments first.
    /// </summary>
    /// <param name="functionName">The name of the function to run.</param>
    /// <param name="runOnDisabledScripts">Whether to run the function on disabled scripts. Defaults to false.</param>
    /// <param name="stopOnError">Whether to stop if there's an error running any script, or simply print it to the console and continue. Defaults to false.</param>
    /// <param name="functionEntityArguments">Optional vector of entity pointers that should be passed into the Lua function. Their internal Lua states will not be accessible. Defaults to empty.</param>
    /// <param name="functionLiteralArguments">Optional vector of strings, that should be passed into the Lua function. Entries must be surrounded with escaped quotes (i.e.`\"`) they'll be passed in as-is, allowing them to act as booleans, etc.. Defaults to empty.</param>
    /// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
    int RunScriptedFunctionInAppropriateScripts(const std::string &functionName, bool runOnDisabledScripts = false, bool stopOnError = false, const std::vector<Entity *> &functionEntityArguments = std::vector<Entity *>(), const std::vector<std::string> &functionLiteralArguments = std::vector<std::string>());

    /// <summary>
    /// Gets whether or not the object has a script name, and there were no errors when initializing its Lua scripts. If there were, the object would need to be reloaded.
    /// </summary>
    /// <returns>Whether or not the object's scripts have been successfully initialized.</returns>
    bool ObjectScriptsInitialized() const { return !m_ScriptObjectName.empty() && m_ScriptObjectName != "ERROR"; }

    /// <summary>
    /// Override SetPresetName so it also resets script preset name and then reloads scripts to safely allow for multiple scripts.
    /// </summary>
    /// <param name="newName">A string reference with the instance name of this Entity.</param>
    /// <param name="calledFromLua">Whether this method was called from Lua, in which case this change is cosmetic only and shouldn't affect scripts.</param>
    void SetPresetName(const std::string &newName, bool calledFromLua = false) override { Entity::SetPresetName(newName); if (!calledFromLua) { m_ScriptPresetName.clear(); ReloadScripts(); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MO type code of this MO. Either Actor, Item, or Generic.
// Arguments:       None.
// Return value:    An int describing the MO Type code of this MovableObject.

	int GetMOType() const { return m_MOType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this MovableObject.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const { return m_Mass; }


	/// <summary>
	/// Gets the previous position vector of this MovableObject, prior to this frame.
	/// </summary>
	/// <returns>A Vector describing the previous position vector.</returns>
	const Vector & GetPrevPos() const { return m_PrevPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the velocity vector of this MovableObject.
// Arguments:       None.
// Return value:    A Vector describing the current velocity vector.

    const Vector & GetVel() const { return m_Vel; }


	/// <summary>
	/// Gets the previous velocity vector of this MovableObject, prior to this frame.
	/// </summary>
	/// <returns>A Vector describing the previous velocity vector.</returns>
	const Vector & GetPrevVel() const { return m_PrevVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAngularVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current angular velocity of this MovableObject. Positive is
//                  a counter-clockwise rotation.
// Arguments:       None.
// Return value:    The angular velocity in radians per second.

    virtual float GetAngularVel() const { return 0.0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the largest radius of this in pixels.
// Arguments:       None.
// Return value:    The radius from its center to the edge of its graphical representation.

    virtual float GetRadius() const { return 1.0f; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDiameter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the largest diameter of this in pixels.
// Arguments:       None.
// Return value:    The largest diameter across its graphical representation.

    virtual float GetDiameter() const { return 2.0F; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScale
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current scale of this MOSRotating. This is mostly for fun.
// Arguments:       None.
// Return value:    The normalized scale.

    float GetScale() const { return m_Scale; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGlobalAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets How this is affected by global effects, from +1.0 to -1.0.
//                  Something with a negative value will 'float' upward.
// Arguments:       None.
// Return value:    The global acceleration scalar.

    float GetGlobalAccScalar() const { return m_GlobalAccScalar; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGlobalAccScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets How this is affected by global effects, from +1.0 to -1.0.
//                  Something with a negative value will 'float' upward.
// Arguments:       The global acceleration scalar.
// Return value:    None.

	void SetGlobalAccScalar(float newValue)  { m_GlobalAccScalar = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAirResistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     How much this is affected by air resistance when traveling over a
//                  second, 0 to 1.0, with 0 as default
// Arguments:       None.
// Return value:    The air resistance coefficient.

    float GetAirResistance() const { return m_AirResistance; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAirResistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much this is affected by air resistance when traveling over a
//                  second, 0 to 1.0, with 0 as default
// Arguments:       The air resistance coefficient.
// Return value:    None.

	void SetAirResistance(float newValue)  { m_AirResistance = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAirThreshold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     At which threshold of velocity, in m/s, the effect of AirResistance
//                  kicks in.
// Arguments:       None.
// Return value:    The air threshold speed.

    float GetAirThreshold() const { return m_AirThreshold; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAirThreshold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets at which threshold of velocity, in m/s, the effect of AirResistance
//                  kicks in.
// Arguments:       The air threshold speed.
// Return value:    None.

	void SetAirThreshold(float newValue) { m_AirThreshold = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAge
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets real time age of this MovableObject.
// Arguments:       None.
// Return value:    A unsigned long describing the current age in ms.

    unsigned long GetAge() const { return m_AgeTimer.GetElapsedSimTimeMS(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLifetime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time this MovableObject will exist from creation.
// Arguments:       None.
// Return value:    A unsigned long describing the current lifetime in ms. 0 means unlimited.

    unsigned long GetLifetime() const { return m_Lifetime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MOID of this MovableObject for this frame.
// Arguments:       None.
// Return value:    An int specifying the MOID that this MovableObject is
//                  assigned for the current frame only.

    MOID GetID() const { return m_MOID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRootID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MOID of the MovableObject which is the root MO of this MO for
//                  this frame. If same as what GetID returns, then this is owned by
//                  MovableMan.
// Arguments:       None.
// Return value:    An int specifying the MOID of the MO that this MovableObject
//                  is owned by for the current frame only.

    MOID GetRootID() const { return m_RootMOID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDFootprint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many total (subsequent) MOID's this MO and all its children
//                  are taking up this frame. ie if this MO has no children, this will
//                  likely be 1. Note this is only valid for this frame!
// Arguments:       None.
// Return value:    The number of MOID indices this MO and all its children are taking up.

    int GetMOIDFootprint() const { return m_MOIDFootprint; }

    /// <summary>
    /// Returns whether or not this object has ever been added to MovableMan. Does not account for removal from MovableMan.
    /// </summary>
    /// <returns></returns>
    bool HasEverBeenAddedToMovableMan() const { return m_HasEverBeenAddedToMovableMan; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSharpness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the sharpness factor of this MO.
// Arguments:       None.
// Return value:    The sharpness factor of this MO. 1.0 means normal sharpness, no alter-
//                  ation to any of the impulses.

	float GetSharpness() const { return m_Sharpness; }


	/// <summary>
	/// Placeholder method to allow for ease of use with Attachables. Returns nullptr for classes that aren't derived from Attachable.
	/// </summary>
	/// <returns>Nothing.</returns>
	virtual MOSRotating * GetParent() { return nullptr; }

    /// <summary>
    /// Placeholder method to allow for ease of use with Attachables. Returns nullptr for classes that aren't derived from Attachable.
    /// </summary>
    /// <returns>Nothing.</returns>
	virtual const MOSRotating * GetParent() const { return nullptr; }

	/// <summary>
	/// Returns a pointer to this MO, this is to enable Attachables to get their root nodes.
	/// </summary>
	/// <returns>A pointer to this MovableObject.</returns>
    virtual MovableObject * GetRootParent() { return this; }

	/// <summary>
	/// Returns a pointer to this MO, this is to enable Attachables to get their root nodes.
	/// </summary>
	/// <returns>A pointer to this MovableObject.</returns>
    virtual const MovableObject * GetRootParent() const { return this; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the altitide of this' pos (or appropriate low point) over the
//                  terrain, in pixels.
// Arguments:       The max altitude you care to check for. 0 Means check the whole scene's height.
//                  The accuracy within which measurement is acceptable. Higher number
//                  here means less calculation.
// Return value:    The rough altitude over the terrain, in pixels.

    virtual float GetAltitude(int max = 0, int accuracy = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAboveHUDPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of the top of this' HUD stack.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' HUD stack top point.

    virtual Vector GetAboveHUDPos() const { return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IntersectionWarning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this may have started to intersect the terrain since the
//                  last frame, e g due to flipping.
// Arguments:       None.
// Return value:    Whether this may have started to intersect the terrain since last frame.

    bool IntersectionWarning() const { return m_CheckTerrIntersection; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during its travel.
// Arguments:       None.
// Return value:    Whether this hits other MO's during its travel, or not.

    bool HitsMOs() const { return m_HitsMOs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetsHitByMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to be able to get hit by other
//                  MovableObject:s during their travel.
// Arguments:       None.
// Return value:    Whether this can get hit by MO's, or not.

    bool GetsHitByMOs() const { return m_GetsHitByMOs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIgnoresTeamHits
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this will collide with any other MO of the same team.
// Arguments:       Whether this can hit or get hit by other MOs of the same team.
// Return value:    None.

    void SetIgnoresTeamHits(bool ignoreTeam = true) { m_IgnoresTeamHits = ignoreTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IgnoresTeamHits
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this will collide with any other MO of the same team.
// Arguments:       None.
// Return value:    Whether this can hit or get hit by other MOs of the same team.

    bool IgnoresTeamHits() const { return m_IgnoresTeamHits; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IgnoresWhichTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells which team this would be ignoring hits with, if we're ignoring
//                  hits at all.
// Arguments:       None.
// Return value:    Which team this ignores hits with, if any.

    int IgnoresWhichTeam() const { return m_IgnoresTeamHits ? m_Team : Activity::NoTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIgnoresAtomGroupHits
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this will collide with any other MO that uses an
//                  AtomGroup as a physical representation. This also overrides the
//                  IgnoresAGHitsWhenSlowerThan property.
// Arguments:       Whether this can hit or get hit by other MOs which use AGs.
// Return value:    None.

    void SetIgnoresAtomGroupHits(bool ignoreAG = true) { m_IgnoresAtomGroupHits = ignoreAG; if (ignoreAG) m_IgnoresAGHitsWhenSlowerThan = -1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IgnoresAtomGroupHits
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this will collide with any MO that uses an AtomGroup
//                  as physical representation. (as opposed to single-atom ones)
// Arguments:       None.
// Return value:    Whether this can hit or get hit by other MOs that use AGs.

    bool IgnoresAtomGroupHits() const { return m_IgnoresAtomGroupHits; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IgnoreTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this will collide with any Terrain
// Arguments:       None.
// Return value:    Whether this can hit terrain.

	bool IgnoreTerrain() const { return m_IgnoreTerrain; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetIgnoreTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this will collide with any Terrain
// Arguments:       Whether this can hit terrain.
// Return value:    None.

	void SetIgnoreTerrain(bool ignores) { m_IgnoreTerrain = ignores; }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main material of this MovableObject.
// Arguments:       None.
// Return value:    The the material of this MovableObject.

    virtual Material const * GetMaterial() const = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.
// Arguments:       None.
// Return value:    The the priority  of this MovableObject. Higher number, the higher
//                  priority.

    virtual int GetDrawPriority() const = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenEffect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the screen effect this has loaded, which can be applied to post
//                  rendering. Ownership is NOT transferred!
// Arguments:       None.
// Return value:    The 32bpp screen effect BITMAP. Ownership is NOT transferred!

    BITMAP * GetScreenEffect() const { return m_pScreenEffect; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenEffectHash
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the hash of the path of this object's screen effect file. Used to 
//					transfer glow effects over network. The hash itself is calculated during
//					load.
// Arguments:       None.
// Return value:    This effect's unique hash.

	size_t GetScreenEffectHash() const { return m_ScreenEffectHash; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the mass of this MovableObject.
// Arguments:       A float specifying the new mass value in Kilograms (kg).
// Return value:    None.

    virtual void SetMass(const float newMass) { m_Mass = newMass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPrevPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the position at the start of the sim update.
// Arguments:       A Vector specifying the new 'prev' pos.
// Return value:    None.

    void SetPrevPos(const Vector &newPrevPos) {m_PrevPos = newPrevPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the velocity vector of this MovableObject.
// Arguments:       A Vector specifying the new velocity vector.
// Return value:    None.

    void SetVel(const Vector &newVel) {m_Vel = newVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current absolute angle of rotation of this MovableObject.
// Arguments:       The new absolute angle in radians.
// Return value:    None.

	void SetRotAngle(float newAngle) override {}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetEffectRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current absolute angle of rotation of this MovableObject's effect.
// Arguments:       The new absolute angle in radians.
// Return value:    None.

	void SetEffectRotAngle(float newAngle) { m_EffectRotAngle = newAngle; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEffectRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current absolute angle of rotation of this MovableObject's effect.
// Arguments:       None.
// Return value:    The absolute angle in radians.

	float GetEffectRotAngle() const { return m_EffectRotAngle; }

	/// <summary>
	/// Gets the starting strength of this MovableObject's effect.
	/// </summary>
	/// <returns>The starting strength of the effect, 0-255.</returns>
	int GetEffectStartStrength() const { return m_EffectStartStrength; }

	/// <summary>
	/// Gets the stopping strength of this MovableObject's effect.
	/// </summary>
	/// <returns>The stopping strength of the effect, 0-255.</returns>
	int GetEffectStopStrength() const { return m_EffectStopStrength; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetAngularVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current angular velocity of this MovableObject. Positive is
//                  a counter clockwise rotation.
// Arguments:       The new angular velocity in radians per second.
// Return value:    None.

    virtual void SetAngularVel(float newRotVel) {}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScale
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current scale of this MOSRotating. This is mostly for fun.
// Arguments:       The new normalized scale.
// Return value:    None.

    void SetScale(float newScale) { m_Scale = newScale; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLifetime
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of time this MovableObject will exist.
// Arguments:       A unsigned long specifying amount of time in ms. 0 means unlimited life.
// Return value:    None.

    void SetLifetime(const int newLifetime = 0) { m_Lifetime = newLifetime; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAge
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this' age timer to a specific value, in ms.
// Arguments:       The new age of this, in MS.
// Return value:    None.

    void SetAge(double newAge = 0) { m_AgeTimer.SetElapsedSimTimeMS(newAge); }


    /// <summary>
    /// Sets the MOID of this MovableObject to be g_NoMOID (255) for this frame.
    /// </summary>
    virtual void SetAsNoID() { m_MOID = g_NoMOID; }

    /// <summary>
    /// Sets this object as having been added to MovableMan. Should only really be done in MovableMan::AddObject.
    /// </summary>
	void SetAsAddedToMovableMan() { m_HasEverBeenAddedToMovableMan = true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSharpness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the sharpness factor of this MO.
// Arguments:       The sharpness factor of this MO. 1.0 means normal sharpness, no alter-
//                  ation to any of the impulses.
// Return value:    None.

	void SetSharpness(const float sharpness) { m_Sharpness = sharpness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.
// Arguments:       Whether to hit other MO's during travel, or not.
// Return value:    None.

    void SetToHitMOs(bool hitMOs = true) { m_HitsMOs = hitMOs; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToGetHitByMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to be able to be collided with by other
//                  MovableObjects during their travel.
// Arguments:       Whether this should get hit by other MO's during travel, or not.
// Return value:    None.

    void SetToGetHitByMOs(bool getHitByMOs = true) { m_GetsHitByMOs = getHitByMOs; }


    /// <summary>
    /// Gets the MO this MO is set not to hit even when MO hitting is enabled on this MO.
    /// </summary>
    /// <returns>The MO this MO is set not to hit.</returns>
    const MovableObject * GetWhichMOToNotHit() const { return m_pMOToNotHit; }

    /// <summary>
    /// Sets this MO to not hit a specific other MO and all its children even when MO hitting is enabled on this MO.
    /// </summary>
    /// <param name="moToNotHit">A pointer to the MO to not be hitting. Null pointer means don't ignore anyhting. Ownership is NOT transferred!</param>
    /// <param name="forHowLong">How long, in seconds, to ignore the specified MO. A negative number means forever.</param>
    virtual void SetWhichMOToNotHit(MovableObject *moToNotHit = nullptr, float forHowLong = -1) { m_pMOToNotHit = moToNotHit; m_MOIgnoreTimer.Reset(); m_MOIgnoreTimer.SetSimTimeLimitS(forHowLong); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetWrapDoubleDrawing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Enables or disables double drawing of this across wrapping seams.
// Arguments:       Wheter to enable or not.
// Return value:    None.

    void SetWrapDoubleDrawing(bool wrapDraw = true) { m_WrapDoubleDraw = wrapDraw; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToSettle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Marks this MovableObject for settling onto the terrain at the end of
//                  the MovableMan update.
// Arguments:       Whether to mark this MO for settling or not.
// Return value:    None.

    void SetToSettle(bool toSettle = true) { m_ToSettle = toSettle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToDelete
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Marks this MovableObject for deletion at the end of the MovableMan
//                  update.
// Arguments:       Whether to mark this MO for deletion or not.
// Return value:    None.

    void SetToDelete(bool toDelete = true) { m_ToDelete = toDelete; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsSetToDelete
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if this MovableObject is marked for deletion at the end of the
//                  update.
// Arguments:       None.
// Return value:    Whether this is marked for deletion or not.

    bool IsSetToDelete() const { return m_ToDelete; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMissionCritical
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is mission critical and should therefore NEVER be
//                  settled or otherwise destroyed during teh course of a mission.
// Arguments:       None.
// Return value:    Whetehr this should be immune to settling and destruction.

    bool IsMissionCritical() const { return m_MissionCritical; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMissionCritical
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this is mission critical and should therefore NEVER be
//                  settled or otherwise destroyed during teh course of a mission.
// Arguments:       Whether this should be immune to settling and destruction.
// Return value:    None.

	void SetMissionCritical(bool missionCritical) { m_MissionCritical = missionCritical; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CanBeSquished
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this can be squished by getting pushed into the ground.
// Arguments:       None.
// Return value:    Whetehr this should be immune to squishing or not.

    bool CanBeSquished() const { return m_CanBeSquished; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHUDVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this Actor's HUD is drawn or not.
// Arguments:       None.
// Return value:    Whether this' HUD gets drawn or not.

    void SetHUDVisible(bool visible) { m_HUDVisible = visible; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHUDVisible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this Actor's HUD is drawn or not.
// Arguments:       None.
// Return value:    Whether this' HUD gets drawn or not.

    bool GetHUDVisible() const { return m_HUDVisible; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsTooFast
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is moving or rotating stupidly fast in a way
//                  that will screw up the simulation.
// Arguments:       None.
// Return value:    Whetehr this is eitehr moving or rotating too fast.

    virtual bool IsTooFast() const { return m_Vel.GetLargest() > 500; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  FixTooFast
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Slows the speed of anyhting that is deemed to be too fast to within
//                  acceptable rates.
// Arguments:       None.
// Return value:    None.

    virtual void FixTooFast() { if (IsTooFast()) { m_Vel.SetMagnitude(450); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsGeneric
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is an Generic or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Type Generic or not.

	bool IsGeneric() const { return m_MOType == TypeGeneric; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is an Actor or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Type Actor or not.

	bool IsActor() const { return m_MOType == TypeActor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is a Device or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Type Device (Held or Thrown) or not.

	bool IsDevice() const { return m_MOType == TypeHeldDevice || m_MOType == TypeThrownDevice; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsHeldDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is a HeldDevice or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Type HeldDevice or not.

// LEGACY CRAP
	bool IsHeldDevice() const { return m_MOType == TypeHeldDevice || m_MOType == TypeThrownDevice; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsThrownDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is a ThrownDevice or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Type ThrownDevice or not.

	bool IsThrownDevice() const { return m_MOType == TypeThrownDevice; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is made of Gold or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Gold or not.

    bool IsGold() const { return m_MOType == TypeGeneric && GetMaterial()->GetIndex() == c_GoldMaterialID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDrawnAfterParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MovableObject is to be drawn after
//                  (in front of) or before (behind) the parent.
// Arguments:       None.
// Return value:    Whether it's to be drawn after parent or not.

    virtual bool IsDrawnAfterParent() const { return true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically named object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.
// Arguments:       The Preset name of the object to look for.
// Return value:    Whetehr the object was found carried by this.

    virtual bool HasObject(std::string objectName) const { return m_PresetName == objectName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasObjectInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is or carries a specifically grouped object in its
//                  inventory. Also looks through the inventories of potential passengers,
//                  as applicable.
// Arguments:       The name of the group to look for.
// Return value:    Whetehr the object in the group was found carried by this.

    virtual bool HasObjectInGroup(std::string groupName) const { return const_cast<MovableObject *>(this)->IsInGroup(groupName); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds force to this MovableObject for the next time Update() is called.
// Arguments:       An Vector with the external force vector that will be added to this
//                  MovableObject and affect its path next Update(). In N or kg * m/s^2.
//                  A Vector with the offset, in METERS, of where the force is being
//                  applied relative to the center of this MovableObject.
// Return value:    None.

    void AddForce(const Vector &force, const Vector &offset = Vector())
        { m_Forces.push_back(std::make_pair(force, offset)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAbsForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds force to this MovableObject for the next time Update() is called.
// Arguments:       An Vector with the external force vector that will be added to this
//                  MovableObject and affect its path next Update(). In N or kg * m/s^2.
//                  A Vector with the absolute world coordinates, in PIXELS, of where the
//                  force is being applied to the center of this MovableObject.
// Return value:    None.

    void AddAbsForce(const Vector &force, const Vector &absPos)
        { m_Forces.push_back(std::make_pair(force, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddImpulseForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds impulse force (or instant momentum) to this MovableObject for
//                  the next time Update() is called.
// Arguments:       An Vector with the impulse force vector that will directly be added
//                  to this MovableObject's momentum  next Update(). In kg * m/s.
//                  A Vector with the offset, in METERS, of where the impulse is being
//                  applied relative to the center of this MovableObject.
// Return value:    None.

	void AddImpulseForce(const Vector &impulse, const Vector &offset = Vector()) {

#ifndef RELEASE_BUILD
		RTEAssert(impulse.GetLargest() < 500000, "HUEG IMPULSE FORCE");
		RTEAssert(offset.GetLargest() < 5000, "HUEG IMPULSE FORCE OFFSET");
#endif

        m_ImpulseForces.push_back({impulse, offset});
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddAbsImpulseForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds impulse force (or instant momentum) to this MovableObject for
//                  the next time Update() is called.
// Arguments:       An Vector with the impulse force vector that will directly be added
//                  to this MovableObject's momentum  next Update(). In kg * m/s.
//                  A Vector with the absolute world coordinates, in PIXELS, of where the
//                  force is being applied to the center of this MovableObject.
// Return value:    None.

	void AddAbsImpulseForce(const Vector &impulse, const Vector &absPos) {

#ifndef RELEASE_BUILD
		RTEAssert(impulse.GetLargest() < 500000, "HUEG IMPULSE FORCE");
#endif

		m_ImpulseForces.push_back(std::make_pair(impulse, g_SceneMan.ShortestDistance(m_Pos, absPos) * c_MPP));
	}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the forces this MO has accumulated during this frame.
// Arguments:       None.
// Return value:    None.

	void ClearForces() { m_Forces.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearImpulseForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the impulses this MO has accumulated during this frame.
// Arguments:       None.
// Return value:    None.

	void ClearImpulseForces() { m_ImpulseForces.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPinStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the impulse force threshold which has to be exceeded to
//                  'shake loose' this from a 'pinned' state. Pinned MOs don't get moved
//                  by travel algos. If 0, this isn't pinned.
// Arguments:       None.
// Return value:    The impulse threshold in kg * (m/s). 0 means no pinning

    float GetPinStrength() const { return m_PinStrength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPinStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a impulse force threshold which has to be exceeded to
//                  'shake loose' this from a 'pinned' state. Pinned MOs don't get moved
//                  by travel algos. If 0, this isn't pinned.
// Arguments:       The impulse threshold in kg * (m/s). 0 means no pinning
// Return value:    None.

    void SetPinStrength(float pinStrength) { m_PinStrength = pinStrength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ResetAllTimers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
//                  prevent backed up emissions to come out all at once while this has been
//                  held dormant in an inventory.
// Arguments:       None.
// Return value:    None.

	virtual void ResetAllTimers() {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    virtual void RestDetection();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  NotResting
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO reset its tiemr that keeps track of how long it's been
//                  at rest, effectively delaying it.
// Arguments:       None.
// Return value:    None.

	void NotResting() { m_RestTimer.Reset(); m_ToSettle = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsAtRest
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates wheter the MovableObject has been at rest (no velocity) for more
//                  than one (1) second.
// Arguments:       None.
// Return value:    Wheter the MovableObject has been at rest for more than one full second.

	bool IsAtRest();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsUpdated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates wheter this MovableObject has been updated yet during this
//                  frame.
// Arguments:       None.
// Return value:    Wheter or not the MovableObject has been updated yet during this frame.

    bool IsUpdated() const { return m_IsUpdated; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NewFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tell this MovableObject that a new frame has started.
// Arguments:       None.
// Return value:    None.

    void NewFrame() { m_IsUpdated = false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToSettle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is marked for settling at the end of the
//                  MovableMan update.
// Arguments:       None.
// Return value:    Whether this MO is marked for settling ontot the terrain or not.

    bool ToSettle() const { return !m_MissionCritical && m_ToSettle; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ToDelete
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is marked for deletion at the end of the
//                  MovableMan update.
// Arguments:       None.
// Return value:    Whether this MO is marked for deletion or not.

    bool ToDelete() const { return m_ToDelete; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DidWrap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO moved across the scene wrap seam during the
//                  last update.
// Arguments:       None.
// Return value:    Whether this MO wrapped or not.

    bool DidWrap() { return m_DidWrap; }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure v. method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.
// Arguments:       Reference to the HitData struct which describes the collision. This
//                  will be modified to represent the results of the collision.
// Return value:    Whether the collision has been deemed valid. If false, then disregard
//                  any impulses in the Hitdata.

    virtual bool CollideAtPoint(HitData &hitData) = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this hit.

	bool OnMOHit(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  OnMOHit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits another MO.
//                  This is called by the owned Atom/AtomGroup of this MovableObject during
//                  travel.
// Arguments:       The other MO hit. Ownership is not transferred.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this hit.

    virtual bool OnMOHit(MovableObject *pOtherMO);


//////////////////////////////////////////////////////////////////////////////////////////
// Pure v. method:  OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this bounce.

    virtual bool OnBounce(HitData &hd) = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Pure v. method:  OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this sinkage.

    virtual bool OnSink(HitData &hd) = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MoveOutOfTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether any of the Atom:s in this MovableObject are on top of
//                  terrain pixels, and if so, attempt to move this out so none of this'
//                  Atoms are on top of the terrain any more.
// Arguments:       Only consider materials stronger than this in the terrain for
//                  intersections.
// Return value:    Whether any intersection was successfully resolved. Will return true
//                  even if there wasn't any intersections to begin with.

    virtual bool MoveOutOfTerrain(unsigned char strongerThan = g_MaterialAir) { return true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RotateOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Rotates a vector offset from this MORotating's position according to
//                  the rotate angle and flipping.
// Arguments:       A const reference the offset Vector to rotate.
// Return value:    A new vector that is the result of the rotation.

    virtual Vector RotateOffset(const Vector &offset) const { return offset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyForces
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the global and accumulated forces. Then it clears
//                  out the force list.Note that this does NOT apply the accumulated
//                  impulses (impulse forces)!
// Arguments:       None.
// Return value:    None.

    virtual void ApplyForces();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyImpulses
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gathers and applies the accumulated impulse forces. Then it clears
//                  out the impulse list.Note that this does NOT apply the accumulated
//                  regular forces (non-impulse forces)!
// Arguments:       None.
// Return value:    None.

    virtual void ApplyImpulses();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetForcesCount()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of Forces vectors to apply.
// Arguments:       None.
// Return value:    Number of entries in Forces list.

	int GetForcesCount() { return m_Forces.size(); };

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetForceVector()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns force vector in newtons of the specified Force record.
// Arguments:       Force record index to get data from.
// Return value:    Force vector in newtons of the specified Force record.

	Vector GetForceVector(int n) { if (n > 0 && n < m_Forces.size()) return m_Forces[n].first; else return Vector(0, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetForceOffset()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns offset vector in METERS (not pixels) of the specified Force record.
// Arguments:       Force record index to get data from.
// Return value:    Offset vector in meters of the specified Force record.

	Vector GetForceOffset(int n) { if (n > 0 && n < m_Forces.size()) return m_Forces[n].second; else return Vector(0, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetForceVector()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets force vector in newtons of the specified Force record.
// Arguments:       Force record index to get data from. New Vector force value in newtons.
// Return value:    None.

	void SetForceVector(int n, Vector v) { if (n > 0 && n < m_Forces.size()) m_Forces[n].first = v; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetForceOffset()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets offset vector in METERS (not pixels) of the specified Force record.
// Arguments:       Force record index to get data from. New Vector offset value in meters.
// Return value:    None.

	void SetForceOffset(int n, Vector v) { if (n > 0 && n < m_Forces.size()) m_Forces[n].second = v; }

    /// <summary>
    /// Gets the pairs of impulse forces and their offsets that have to be applied.
    /// </summary>
    /// <returns>A constant reference to the deque of impulses for this MovableObject.</returns>
    const std::deque <std::pair<Vector, Vector>> &GetImpulses() { return m_ImpulseForces; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetImpulsesCount()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the number of ImpulseForces vectors to apply.
// Arguments:       None.
// Return value:    Number of entries in ImpulseForces list.

	int GetImpulsesCount() { return m_ImpulseForces.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetImpulseVector()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns Impulse vector in newtons of the specified Impulse record.
// Arguments:       Impulse record index to get data from.
// Return value:    Impulse vector in newtons of the specified Impulse record.

	Vector GetImpulseVector(int n) { if (n > 0 && n < m_ImpulseForces.size()) return m_ImpulseForces[n].first; else return Vector(0, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetImpulseOffset()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns offset vector in METERS (not pixels) of the specified Impulse record.
// Arguments:       Impulse record index to get data from.
// Return value:    Offset vector in meters of the specified Impulse record.

	Vector GetImpulseOffset(int n) { if (n > 0 && n < m_ImpulseForces.size()) return m_ImpulseForces[n].second; else return Vector(0, 0); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetImpulseVector()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns offset vector in METERS (not pixels) of the specified Impulse record.
// Arguments:       Impulse record index to get data from.
// Return value:    Offset vector in meters of the specified Impulse record.

	void SetImpulseVector(int n, Vector v) { if (n > 0 && n < m_ImpulseForces.size()) m_ImpulseForces[n].first = v; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetImpulseOffset()
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets offset vector in METERS (not pixels) of the specified Impulse record.
// Arguments:       Impulse record index to get data from. New Vector offset value in meters.
// Return value:    None.

	void SetImpulseOffset(int n, Vector v) { if (n > 0 && n < m_ImpulseForces.size()) m_ImpulseForces[n].second = v; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done before Travel(). Always call before
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

    virtual void PreTravel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MovableObject, using its physical representation.
// Arguments:       None.
// Return value:    None.

    virtual void Travel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PostTravel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does stuff that needs to be done after Travel(). Always call after
//                  calling Travel.
// Arguments:       None.
// Return value:    None.

    virtual void PostTravel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame. This also
//                  applies and clear the accumulated impulse forces (impulses), and the
//                  transferred forces of MOs attached to this.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateScript
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject's Lua scripts. Supposed to be done every
//                  frame after the rest of the hardcoded C++ update is done.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int UpdateScripts();

    /// <summary>
    /// Executes the Lua-defined OnPieMenu event handler for this MO.
    /// </summary>
    /// <param name="pieMenuActor">The actor which triggered the pie menu event.</param>
    /// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
	virtual int OnPieMenu(Actor *pieMenuActor);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this' and its its childrens' MOID's and foorprint. Should
//                  be done every frame.
// Arguments:       None.
// Return value:    None.

	void UpdateMOID(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawMOIDIfOverlapping
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the MOID representation of this to the SceneMan's MOID layer if
//                  this is found to potentially overlap another MovableObject.
// Arguments:       The MovableObject to check this for overlap against.
// Return value:    Whether it was drawn or not.

    virtual bool DrawMOIDIfOverlapping(MovableObject *pOverlapMO) { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this' current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the draw bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) { return; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRestThreshold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns current rest threshold for this MO
// Arguments:       None
// Return value:    Rest threshold of this MO

	int GetRestThreshold() const { return m_RestThreshold; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetRestThreshold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets current rest threshold for this MO
// Arguments:       New rest threshold value
// Return value:    None

	void SetRestThreshold(int newRestThreshold)  { m_RestThreshold = newRestThreshold; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:  GetNextID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the next unique id for MO's and increments unique ID counter
// Arguments:       None.
// Return value:    Returns the next unique id.

	static unsigned long int GetNextUniqueID() { return ++m_UniqueIDCounter; }

//////////////////////////////////////////////////////////////////////////////////////////
// Static method:  GetUniqueID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns this MO's unique persistent ID
// Arguments:       None.
// Return value:    Returns this MO's unique persistent ID

	unsigned long int const GetUniqueID() const { return m_UniqueID; }

    /// <summary>
    /// Gets the preset name and unique ID of this MO, often useful for error messages.
    /// </summary>
    /// <returns>A string containing the unique ID and preset name of this MO.</returns>
    std::string GetPresetNameAndUniqueID() const { return m_PresetName + ", UID: " + std::to_string(m_UniqueID); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DamageOnCollision
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If not zero applyies specified ammount of damage points to actors on 
//					collision even without penetration.
// Arguments:       None
// Return value:    Amount of damage to apply.

	float DamageOnCollision() const { return m_DamageOnCollision; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDamageOnCollision
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If not zero applyies specified ammount of damage points to actors on 
//					collision even without penetration.
// Arguments:       Amount of damage to apply.
// Return value:    None.

	void SetDamageOnCollision(float value) { m_DamageOnCollision = value; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DamageOnPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If not zero applies specified ammount of damage points to actors on 
//					collision if penetration occured.
// Arguments:       None
// Return value:    Amount of damage to apply.

	float DamageOnPenetration() const { return m_DamageOnPenetration; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetDamageOnPenetration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If not zero applies specified ammount of damage points to actors on 
//					collision if penetration occured.
// Arguments:       Amount of damage to apply.
// Return value:    None.

	void SetDamageOnPenetration(float value) { m_DamageOnPenetration = value; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  WoundDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns damage multiplier transferred to wound inflicted by this object on penetration
// Arguments:       None
// Return value:    Damage multiplier to apply to wound.

	float WoundDamageMultiplier() const { return m_WoundDamageMultiplier; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetWoundDamageMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets damage multiplier transferred to wound inflicted by this object on penetration
// Arguments:       New damage multiplier to apply to wound.
// Return value:    None.

	void SetWoundDamageMultiplier(float value) { m_WoundDamageMultiplier = value; }

    /// <summary>
    /// Gets whether or not this MovableObject should apply wound damage when it collides with another MovableObject.
    /// </summary>
    /// <returns>Whether or not this MovableObject should apply wound damage when it collides with another MovableObject.</returns>
    bool GetApplyWoundDamageOnCollision() const { return m_ApplyWoundDamageOnCollision; }

    /// <summary>
    /// Sets whether or not this MovableObject should apply wound damage when it collides with another MovableObject.
    /// </summary>
    /// <param name="applyWoundDamageOnCollision">Whether or not this MovableObject should apply wound damage on collision.</param>
    void SetApplyWoundDamageOnCollision(bool applyWoundDamageOnCollision) { m_ApplyWoundDamageOnCollision = applyWoundDamageOnCollision; }

    /// <summary>
    /// Gets whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.
    /// </summary>
    /// <returns>Whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.</returns>
    bool GetApplyWoundBurstDamageOnCollision() const { return m_ApplyWoundBurstDamageOnCollision; }

    /// <summary>
    /// Sets whether or not this MovableObject should apply burst wound damage when it collides with another MovableObject.
    /// </summary>
    /// <param name="applyWoundDamageOnCollision">Whether or not this MovableObject should apply burst wound damage on collision.</param>
    void SetApplyWoundBurstDamageOnCollision(bool applyWoundBurstDamageOnCollision) { m_ApplyWoundBurstDamageOnCollision = applyWoundBurstDamageOnCollision; }



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the ID of the MO hit at the previously taken Travel
//                  This will only potentially return non-g_NoMOID if this object's Atom is set to
//                  hit MO's and the MO hit isn't marked to be ignored.
// Arguments:       None.
// Return value:    The ID of the non-ignored MO, if any, that this object's Atom or AtomGroup is now
//                  intersecting because of the last Travel taken.

	MOID HitWhatMOID() const { if (m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber()) return m_MOIDHit; else return g_NoMOID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitWhatMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ID of the MO hit at the previously taken Travel
//                  This will only potentially return non-g_NoMOID if this object's Atom is set to
//                  hit MO's and the MO hit isn't marked to be ignored.
// Arguments:       The ID of the non-ignored MO, if any, that this object's Atom or AtomGroup is now
//                  intersecting because of the last Travel taken.
// Return value:    None.

	void SetHitWhatMOID(MOID id) { m_MOIDHit = id;  m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the unique ID of the particle hit at the previously taken Travel
// Arguments:       None.
// Return value:    Unique ID of the particle hit at the previously taken Travel

	long int HitWhatParticleUniqueID() const { if (m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber()) return m_ParticleUniqueIDHit; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the unique ID of the particle hit at the previously taken Travel
// Arguments:       Unique ID of the particle hit at the previously taken Travel.
// Return value:    None.

	void SetHitWhatParticleUniqueID(long int id) { m_ParticleUniqueIDHit = id; m_LastCollisionSimFrameNumber = g_MovableMan.GetSimUpdateFrameNumber(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HitWhatTerrMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the terrain material the previously taken Tarvel
//                  hit, if any.
// Arguments:       None.
// Return value:    The ID of the material, if any, that this MO hit during the last Travel.

	unsigned char HitWhatTerrMaterial() const { if (m_LastCollisionSimFrameNumber == g_MovableMan.GetSimUpdateFrameNumber()) return m_TerrainMatHit; else return g_MaterialAir; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHitWhatTerrMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the terrain material the previously taken Tarvel hit, if any.
// Arguments:       The ID of the material, if any, that this MO hit during the last Travel.
// Return value:    None.

	void SetHitWhatTerrMaterial(unsigned char matID);

	bool ProvidesPieMenuContext() const { return m_ProvidesPieMenuContext; }

	void SetProvidesPieMenuContext(bool value) { m_ProvidesPieMenuContext = value; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:
    /// <summary>
    /// Does necessary work to setup a script object name for this object, allowing it to be accessed in Lua, then runs all of the MO's scripts' Create functions in Lua.
    /// </summary>
    /// <returns>0 on success, -2 if it fails to setup the script object in Lua, and -3 if it fails to run any Create function.</returns>
    int InitializeObjectScripts();

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.
// Arguments:       The MOID index to register itself and its children in.
//                  The MOID of the root MO of this MO, ie the highest parent of this MO.
//                  0 means that this MO is the root, ie it is owned by MovableMan.
//                  Whether this MO should make a new MOID to use for itself, or to use
//                  the same as the last one in the index (presumably its parent),
// Return value:    None.

	virtual void UpdateChildMOIDs(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true) {}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RegMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself in the MOID register and get ID:s for
//                  itself and its children for this frame.
//                  BITMAP of choice.
// Arguments:       The MOID index to register itself and its children in.
//                  The MOID of the root MO of this MO, ie the highest parent of this MO.
//                  0 means that this MO is the root, ie it is owned by MovableMan.
//                  Whether this MO should make a new MOID to use for itself, or to use
//                  the same as the last one in the index (presumably its parent),
// Return value:    None.

	void RegMOID(std::vector<MovableObject *> &MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true);

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a MovableObject object
//                  identical to an already existing one.
// Arguments:       A MovableObject object which is passed in by reference.


    // Member variables
    static Entity::ClassInfo m_sClass;
	// Global counter with unique ID's
	static unsigned long int m_UniqueIDCounter;
    // The type of MO this is, either Actor, Item, or Particle
    int m_MOType;
    float m_Mass; // In metric kilograms (kg).
    Vector m_Vel; // In meters per second (m/s).
    Vector m_PrevPos; // Previous frame's position.
    Vector m_PrevVel; // Previous frame's velocity.
    float m_Scale; // The scale that this MovableObject's representation will be drawn in. 1.0 being 1:1;
    // How this is affected by global effects, from +1.0 to -1.0. Something with a negative value will 'float' upward
    float m_GlobalAccScalar;
    // How much this is affected by air resistance when traveling over a second, 0 to 1.0, with 0 as default
    float m_AirResistance;
    // At which threshold of velocity, in m/s, the effect of AirResistance kicks in
    float m_AirThreshold;
    // The impulse force in kg * (m/s) needed to unpin this. Pinned MO's don't travel at all.
    float m_PinStrength;
    // The threshold in ms as to how long this MO should wait after being at rest
    // to get flagged to be copied to the terrain.
    int m_RestThreshold;
    // The forces acting on this MovableObject, the first vector being the force in
    // In kg * m/s^2 (Newtons), and the second one being the offset the force is being
    // applied from the m_Pos, IN METERS (not pixels!).
    std::deque<std::pair<Vector, Vector> > m_Forces;
    std::deque<std::pair<Vector, Vector> > m_ImpulseForces; // First in kg * m/s, second vector in meters.
    Timer m_AgeTimer;
    Timer m_RestTimer;

    unsigned long m_Lifetime;
    // The sharpness factor that gets added to single pixel hit impulses in
    // applicable situations.
    float m_Sharpness;
    // This is to be set each frame that this may be intersecting the terrain, like when it has been flipped
    bool m_CheckTerrIntersection;
    // Whether or not this MovableObject will test for collisions against other MOs.
    bool m_HitsMOs;
    // Another MovableObject that this should not be hitting even if it is set to hit MOs.
    MovableObject *m_pMOToNotHit;
    // For how long to not hit specific MO above
    Timer m_MOIgnoreTimer;
    // Whether or not this MovableObject can get hit by other MOs.
    bool m_GetsHitByMOs;
    // Whether this ignores collisions with other MOs of the same Team as this.
    bool m_IgnoresTeamHits;
    // This currently ignores hits with other AtomGroup MOs.
    bool m_IgnoresAtomGroupHits;
    // This will flip the IgnoreAtomGroupHits on or off depending on whether this MO is travelling slower than the threshold here, in m/s
    // This is disabled if set to negative value, and 0 means AG hits are never ignored
    float m_IgnoresAGHitsWhenSlowerThan;
    // This is mission critical, which means it should NEVER be settled or destroyed by gibbing
    bool m_MissionCritical;
    // Whether this can be destroyed by being squished into the terrain
    bool m_CanBeSquished;
    // Whether or not this MovableObject has been updated yet this frame.
    bool m_IsUpdated;
    // Whether wrap drawing double across wrapping seams is enabled or not
    bool m_WrapDoubleDraw;
    // Whether the position of this object wrapped around the world this frame, or not.
    // This is just run-time data, don't need to be saved.
    bool m_DidWrap;
    // This is only valid the same frame it was assigned!
    MOID m_MOID;
    // This is only valid the same frame it was assigned!
    // MOID of the root MO, same as this' m_MOID if this is owned by MovableMan.
    MOID m_RootMOID;
    // How many total (subsequent) MOID's this MO and all its children are taking up this frame.
    // ie if this MO has no children, this will likely be 1.
    int m_MOIDFootprint;
    // Whether or not this object has been added to MovableMan. Does not take into account the object being removed from MovableMan, though in practice it usually will.
    bool m_HasEverBeenAddedToMovableMan;
    // A set of ID:s of MO:s that already have collided with this MO during this frame.
    std::set<MOID> m_AlreadyHitBy;
    // A counter to count the oscillations in translational velocity, in order to detect settling.
    int m_VelOscillations;
    // Mark to have the MovableMan copy this the terrain layers at the end
    // of update.
    bool m_ToSettle;
    // Mark to delete at the end of MovableMan update
    bool m_ToDelete;
    // To draw this guy's HUD or not
    bool m_HUDVisible;

    std::map<std::string, bool> m_AllLoadedScripts; //!< A map of script paths to the enabled state of the given script.
    std::unordered_map<std::string, std::vector<std::string>> m_FunctionsAndScripts; //!< A map of function names to vectors of scripts paths. Used to maintain script execution order and avoid extraneous Lua calls.

    // The ID name unique to this' preset and its defined scripted functions in the lua state.
    std::string m_ScriptPresetName;
    // The ID name unique to this' object instance representation in the Lua state.
    std::string m_ScriptObjectName;

    // Special post processing flash effect file and Bitmap. Shuold be loaded from a 32bpp bitmap
    ContentFile m_ScreenEffectFile;
    // Not owned by this, owned by the contentfiles
    BITMAP *m_pScreenEffect;

	size_t m_ScreenEffectHash;

    // Effect's showing is delayed by this amount into this' lifetime, in ms
    int m_EffectStartTime;
    // Effect's ending this far into the lifetime of this, in ms. 0 means as long as this is alive
    int m_EffectStopTime;
    // The intensity of the effect blend at the start time, 0-255
    int m_EffectStartStrength;
    // The intensity of the effect blend at the stop time, 0-255
    int m_EffectStopStrength;
    // The effect can't be obscured
    bool m_EffectAlwaysShows;
	// The effect's rotation angle
	float m_EffectRotAngle;
	// Whether effect's rot angle should be inherited from parent
	bool m_InheritEffectRotAngle;
	// Whether effect's rot angle should be randomized on creation
	bool m_RandomizeEffectRotAngle;
	// Whether effects rot angle should be randomized every frame
	bool m_RandomizeEffectRotAngleEveryFrame;

	// This object's unique persistent ID
	long int m_UniqueID;
	// In which radis should we look to remove orphaned terrain on terrain penetration, 
	// must not be greater than SceneMan::ORPHANSIZE, or will be truncated
	int m_RemoveOrphanTerrainRadius;
	// What is the max orphan area to trigger terrain removal
	int m_RemoveOrphanTerrainMaxArea;
	// Rate at which terrain removal checks will be triggered
	float m_RemoveOrphanTerrainRate;
	// Whether this should cause health-damage on collision
	float m_DamageOnCollision;
	// Whether this should cause health-damage on penetration
	float m_DamageOnPenetration;
	// Damage multiplier transferred to wound inflicted by this object on penetration
	float m_WoundDamageMultiplier;
    bool m_ApplyWoundDamageOnCollision; //!< Whether or not this should apply wound damage on collision, respecting WoundDamageMultiplier and without creating a wound.
    bool m_ApplyWoundBurstDamageOnCollision; //!< Whether or not this should apply wound burst damage on collision, respecting WoundDamageMultiplier and without creating a wound.
	//Whether this MO should ignore terrain when traveling
	bool m_IgnoreTerrain;
	// MOID hit during last Travel
	MOID m_MOIDHit;
	// Terrain material hit during last travel
	unsigned char m_TerrainMatHit;
	// Unique ID of particle hit this MO
	long int m_ParticleUniqueIDHit;
	// Number of sim update frame when last collision was detcted
	unsigned int m_LastCollisionSimFrameNumber;
	// If true, the object will receive OnPieMenu event whenever someone activated a pie menu
	bool m_ProvidesPieMenuContext;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MovableObject, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

	// Disallow the use of some implicit methods.
	MovableObject(const MovableObject &reference) = delete;
	MovableObject& operator=(const MovableObject& ref) = delete;
};

} // namespace RTE

#endif // File