#ifndef _RTEMOVABLEMAN_
#define _RTEMOVABLEMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MovableMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MovableMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"
#include "Singleton.h"
#include "Activity.h"

#define g_MovableMan MovableMan::Instance()

namespace RTE
{

class MovableObject;
class Actor;
class HeldDevice;
class MOPixel;
class MOSprite;
class AHuman;
class SceneLayer;
class SceneObject;
class Box;


//////////////////////////////////////////////////////////////////////////////////////////
// Struct:          AlarmEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A struct to keep all data about a an alarming event for the AI Actors.
// Parent(s):       None.
// Class history:   10/3/2008  AlarmEvent created.

struct AlarmEvent {
	AlarmEvent() { m_ScenePos.Reset(); m_Team = Activity::NoTeam; m_Range = 1.0F; }
	// TODO: Stop relying on screen width for this shit!
	AlarmEvent(const Vector &pos, int team = Activity::NoTeam, float range = 1.0F);

    // Absolute position in the scene where this occurred
    Vector m_ScenePos;
    // The team of whatever object that caused this event
    Activity::Teams m_Team;
    // The range multiplier, that this alarming event can be heard
    float m_Range;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MovableMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of all movable objects in the RTE.
// Parent(s):       Singleton, Serializable.
// Class history:   12/25/2001 MovableMan created.

class MovableMan : public Singleton<MovableMan>, public Serializable {
	friend class SettingsMan;
    friend struct ManagerLuaBindings;


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableClassNameGetter;
	SerializableOverrideMethods;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MovableMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MovableMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MovableMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MovableMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MovableMan object before deletion
//                  from system memory.
// Arguments:       None.

	~MovableMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Initialize();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MovableMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MovableMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOFromID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a MO from its MOID. Note that MOID's are only valid during the
//                  same frame as they were assigned to the MOs!
// Arguments:       The MOID to get the matching MO from.
// Return value:    A pointer to the requested MovableObject instance. 0 if no MO with that
//                  MOID was found. 0 if 0 was passed in as MOID (no MOID). Ownership is
//                  *NOT* transferred!!

    MovableObject * GetMOFromID(MOID whichID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of MOID's currently in use this frame.
// Arguments:       None.
// Return value:    The count of MOIDs in use this frame.

    int GetMOIDCount() { return m_MOIDIndex.size(); }

    /// <summary>
    /// Tests whether the given MovableObject is currently at the specified pixel coordinates.
    /// </summary>
    /// <param name="mo">The MovableObject to test.</param>
    /// <param name="pixelX">The X coordinate of the Scene pixel to test.</param>
    /// <param name="pixelY">The Y coordinate of the Scene pixel to test.</param>
    /// <returns>Whether the given MovableObject is currently at the specified pixel coordinates.</returns>
    bool HitTestMOAtPixel(const MovableObject &mo, int pixelX, int pixelY) const;

    /// <summary>
    /// Gets a MOID from pixel coordinates in the Scene.
    /// </summary>
    /// <param name="pixelX">The X coordinate of the Scene pixel to get the MOID of.</param>
    /// <param name="pixelY">The Y coordinate of the Scene pixel to get the MOID of.</param>
    /// <param name="moidList">The collection of MOIDs to check the against the specified coordinates.</param>
    /// <returns>The topmost MOID currently at the specified pixel coordinates.</returns>
    MOID GetMOIDPixel(int pixelX, int pixelY, const std::vector<int> &moidList);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamMOIDCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns MO count for specified team
// Arguments:       Team to count MO's
// Return value:    MO's count owned by this team

	int GetTeamMOIDCount(int team) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PurgeAllMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all MovableObject:s out of this. Effectively empties the world
//                  of anything moving, without resetting all of this' settings.
// Arguments:       None.
// Return value:    None.

    void PurgeAllMOs();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextActorInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the first Actor in the internal Actor list that is
//                  of a specifc group, alternatively the first one AFTER a specific actor!
// Arguments:       Which group to try to get an Actor for.
//                  A pointer to an Actor to use as starting point in the forward search.
//                  Ownership NOT xferred!
// Return value:    An Actor pointer to the requested team's first Actor encountered
//                  in the list. 0 if there are no Actors of that team.

    Actor * GetNextActorInGroup(std::string group, Actor *pAfterThis = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevActorInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the last Actor in the internal Actor list that is
//                  of a specifc group, alternatively the last one BEFORE a specific actor!
// Arguments:       Which group to try to get an Actor for.
//                  A pointer to an Actor to use as starting point in the backward search.
//                  Ownership NOT xferred!
// Return value:    An Actor pointer to the requested team's last Actor encountered
//                  in the list. 0 if there are no Actors of that team.

    Actor * GetPrevActorInGroup(std::string group, Actor *pBeforeThis = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of all actors on one team, ordered by their X positions.
// Arguments:       Which team to try to get the roster for.
// Return value:    A pointer to the list of all the actors on the specified team, sorted
//                  ascending by their X posistions. Ownership of the list or contained
//                  actors is NOT transferred!

    std::list<Actor *> * GetTeamRoster(int team = 0) { return &(m_ActorRoster[team]); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the first Actor in the internal Actor list that is
//                  of a specifc team, alternatively the first one AFTER a specific actor!
// Arguments:       Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
//                  A pointer to an Actor to use as starting point in the forward search.
//                  Ownership NOT xferred!
// Return value:    An Actor pointer to the requested team's first Actor encountered
//                  in the list. 0 if there are no Actors of that team.

    Actor * GetNextTeamActor(int team = 0, Actor *pAfterThis = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the last Actor in the internal Actor list that is
//                  of a specifc team, alternatively the last one BEFORE a specific actor!
// Arguments:       Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
//                  A pointer to an Actor to use as starting point in the backward search.
//                  Ownership NOT xferred!
// Return value:    An Actor pointer to the requested team's last Actor encountered
//                  in the list. 0 if there are no Actors of that team.

    Actor * GetPrevTeamActor(int team = 0, Actor *pBeforeThis = 0);

	/// <summary>
	/// Get a pointer to an Actor in the internal Actor list that is of a specifc team and closest to a specific scene point.
	/// </summary>
	/// <param name="team">Which team to try to get an Actor for. 0 means first team, 1 means 2nd.</param>
	/// <param name="player">The player to get the Actor for. This affects which brain can be marked.</param>
	/// <param name="scenePoint">The Scene point to search for the closest to.</param>
	/// <param name="maxRadius">The maximum radius around that scene point to search.</param>
	/// <param name="getDistance">A Vector to be filled out with the distance of the returned closest to the search point. Will be unaltered if no object was found within radius.</param>
	/// <param name="excludeThis">An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!</param>
	/// <returns>An Actor pointer to the requested team's Actor closest to the Scene point, but not outside the max radius. If no Actor other than the excluded one was found within the radius of the point, nullptr is returned.</returns>
	Actor * GetClosestTeamActor(int team, int player, const Vector &scenePoint, int maxRadius, Vector &getDistance, const Actor *excludeThis = nullptr) { return GetClosestTeamActor(team, player, scenePoint, maxRadius, getDistance, false, excludeThis); }

	/// <summary>
	/// Get a pointer to an Actor in the internal Actor list that is of a specifc team and closest to a specific scene point.
	/// </summary>
	/// <param name="team">Which team to try to get an Actor for. 0 means first team, 1 means 2nd.</param>
	/// <param name="player">The player to get the Actor for. This affects which brain can be marked.</param>
	/// <param name="scenePoint">The Scene point to search for the closest to.</param>
	/// <param name="maxRadius">The maximum radius around that scene point to search.</param>
	/// <param name="getDistance">A Vector to be filled out with the distance of the returned closest to the search point. Will be unaltered if no object was found within radius.</param>
	/// <param name="onlyPlayerControllableActors">Whether to only get Actors that are flagged as player controllable.</param>
	/// <param name="excludeThis">An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!</param>
	/// <returns>An Actor pointer to the requested team's Actor closest to the Scene point, but not outside the max radius. If no Actor other than the excluded one was found within the radius of the point, nullptr is returned.</returns>
	Actor * GetClosestTeamActor(int team, int player, const Vector &scenePoint, int maxRadius, Vector &getDistance, bool onlyPlayerControllableActors, const Actor *excludeThis = nullptr);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestEnemyActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is is not of
//                  the specified team and closest to a specific scene point.
// Arguments:       Which team to try to get an enemy Actor for. NoTeam means all teams.
//                  The Scene point to search for the closest to.
//                  The maximum radius around that scene point to search.
//                  A Vector to be filled out with the distance of the returned closest to
//                  the search point. Will be unaltered if no object was found within radius.
// Return value:    An Actor pointer to the enemy closest to the Scene
//                  point, but not outside the max radius. If no Actor
//                  was found within the radius of the point, 0 is returned.

    Actor * GetClosestEnemyActor(int team, const Vector &scenePoint, int maxRadius, Vector &getDistance);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFirstTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to first best Actor in the internal Actor list that is
//                  of a specifc team.
// Arguments:       Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
//                  The player to get the Actor for. This affects which brain can be marked.
// Return value:    An Actor pointer to the first one of the requested team. If no Actor
//                  is in that team, 0 is returned.

	Actor * GetFirstTeamActor(int team, int player) { Vector temp; return GetClosestTeamActor(team, player, Vector(), 10000000, temp); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is closest
//                  to a specific scene point.
// Arguments:       Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
//                  The Scene point to search for the closest to.
//                  The maximum radius around that scene point to search.
//                  A Vector to be filled out with the distance of the returned closest to
//                  the search point. Will be unaltered if no object was found within radius.
//                  An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    An Actor pointer to the requested Actor closest to the Scene
//                  point, but not outside the max radius. If no Actor other than the
//                  excluded one was found within the radius of the point, 0 is returned.

    Actor * GetClosestActor(const Vector &scenePoint, int maxRadius, Vector &getDistance, const Actor *pExcludeThis = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor of a specific team that is closest to
//                  a scene point. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which team to try to get the brain for. 0 means first team, 1 means 2nd.
//                  The point in the scene where to look for the closest opposite team brain.
// Return value:    An Actor pointer to the requested team's brain closest to the point.
//                  0 if there are no brains of that team. OWNERSHIP IS NOT TRANSFERRED!

    Actor * GetClosestBrainActor(int team, const Vector &scenePoint) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFirstBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor of a specific team that is closest to
//                  a scene point. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which team to try to get the brain for. 0 means first team, 1 means 2nd.
//                  The point in the scene where to look for the closest opposite team brain.
// Return value:    An Actor pointer to the requested team's brain closest to the point.
//                  0 if there are no brains of that team. OWNERSHIP IS NOT TRANSFERRED!

    Actor * GetFirstBrainActor(int team) const { return GetClosestBrainActor(team, Vector()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestOtherBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor NOT of a specific team that is closest
//                  to a scene point. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which team to NOT get the brain for. 0 means first team, 1 means 2nd.
//                  The point where to look for the closest brain not of this team.
// Return value:    An Actor pointer to the requested brain closest to the point.
//                  0 if there are no brains not on that team. OWNERSHIP IS NOT TRANSFERRED!

    Actor * GetClosestOtherBrainActor(int notOfTeam, const Vector &scenePoint) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFirstOtherBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor NOT of a specific team. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which team to NOT get the brain for. 0 means first team, 1 means 2nd.
// Return value:    An Actor pointer to the requested brain of that team.
//                  0 if there are no brains not on that team. OWNERSHIP IS NOT TRANSFERRED!

    Actor * GetFirstOtherBrainActor(int notOfTeam) const { return GetClosestOtherBrainActor(notOfTeam, Vector()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUnassignedBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the first brain actor of a specific team which hasn't
//                  been assigned to a player yet.
// Arguments:       Which team to try to get the brain for. 0 means first team, 1 means 2nd.
// Return value:    An Actor pointer to the requested team's first brain encountered
//                  in the list that hasn't been assigned to a player. 0 if there are no
//                  unassigned brains of that team.

    Actor * GetUnassignedBrain(int team = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActorCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of actors currently held.
// Arguments:       None.
// Return value:    The number of actors.

    long GetActorCount() const { return m_Actors.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParticleCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of particles (MOPixel:s) currently held.
// Arguments:       None.
// Return value:    The number of particles.

    long GetParticleCount() const { return m_Particles.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSplashRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the global setting for how much splash MOPixels should be created
//                  an MO penetrates the terrain deeply.
// Arguments:       None.
// Return value:    A float with the global splash amount setting, form 1.0 to 0.0.

    float GetSplashRatio() const { return m_SplashRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaxDroppedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the max number of dropped items that will be reached before the
//                  first dropped with be copied to the terrain.
// Arguments:       An int spefifying the limit.
// Return value:    None.

    void SetMaxDroppedItems(int newLimit) { m_MaxDroppedItems = newLimit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaxDroppedItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the max number of dropped items that will be reached before the
//                  first dropped with be copied to the terrain.
// Arguments:       None.
// Return value:    An int spefifying the limit.

    int GetMaxDroppedItems() const { return m_MaxDroppedItems; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SortTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this to draw HUD lines for a specific team's roster this frame.
// Arguments:       Which team to have lines drawn of.
// Return value:    None.

    void SortTeamRoster(int team) { m_SortTeamRoster[team] = true; }

    /// <summary>
    /// Adds a MovableObject to this, after it is determined what it is and the best way to add it is. E.g. if it's an Actor, it will be added as such. Ownership IS transferred!
    /// </summary>
    /// <param name="movableObjectToAdd">A pointer to the MovableObject to add. Ownership IS transferred!</param>
    /// <returns>Whether the MovableObject was successfully added or not. Note that Ownership IS transferred either way, but the MovableObject will be deleted if this is not successful.</returns>
    bool AddMO(MovableObject *movableObjectToAdd);

    /// <summary>
    /// Adds an Actor to the internal list of Actors. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
    /// </summary>
    /// <param name="actorToAdd">A pointer to the Actor to add. Ownership IS transferred!</param>
    void AddActor(Actor *actorToAdd);

    /// <summary>
    /// Adds a pickup-able item to the internal list of items. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
    /// </summary>
	/// <param name="itemToAdd">A pointer to the item to add. Ownership IS transferred!</param>
    void AddItem(HeldDevice *itemToAdd);

    /// <summary>
    /// Adds a MovableObject to the internal list of particles. Destruction and deletion will be taken care of automatically. Ownership IS transferred!
    /// </summary>
    /// <param name="particleToAdd">A pointer to the MovableObject to add. Ownership is transferred!</param>
    void AddParticle(MovableObject *particleToAdd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an Actor from the internal list of MO:s. After the Actor is
//                  removed, ownership is effectively released and transferred to whatever
//                  client called this method.
// Arguments:       A pointer to the MovableObject to remove.
// Return value:    Whether the object was found in the particle list, and consequently
//                  removed. If the particle entry wasn't found, false is returned.

    bool RemoveActor(MovableObject *pActorToRem);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a pickup-able MovableObject item from the internal list of
//                  MO:s. After the item is removed, ownership is effectively released and
//                  transferred to whatever client called this method.
// Arguments:       A pointer to the MovableObject to remove.
// Return value:    Whether the object was found in the particle list, and consequently
//                  removed. If the particle entry wasn't found, false is returned.

    bool RemoveItem(MovableObject *pItemToRem);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject from the internal list of MO:s. After the
//                  MO is removed, ownership is effectively released and transferred to
//                  whatever client called this method.
// Arguments:       A pointer to the MovableObject to remove.
// Return value:    Whether the object was found in the particle list, and consequently
//                  removed. If the particle entry wasn't found, false is returned.

    bool RemoveParticle(MovableObject *pMOToRem);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeActorTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes actor team and updates team rosters.
// Arguments:       Pointer to actor, new team value
// Return value:    None.

	void ChangeActorTeam(Actor * pActor, int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddActorToTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds actor to internal team roster
// Arguments:       Pointer to actor
// Return value:    None.

	void AddActorToTeamRoster(Actor * pActorToAdd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveActorToTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes actor from internal team roster
// Arguments:       Pointer to actor
// Return value:    None.

	void RemoveActorFromTeamRoster(Actor * pActorToRem);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidateMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through and checks that all MOID's have valid MO pointers
//                  associated with them. This shuold only be used for testing, as it will
//                  crash the app if validation fails.
// Arguments:       None.
// Return value:    All MOIDs valid.

    bool ValidateMOIDs();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject pointer points to an
//                  MO that's currently active in the simulation, and kept by this
//                  MovableMan. Internal optimization is made so that the same MO can
//                  efficiently be checked many times during the same frame.
// Arguments:       A pointer to the MovableObject to check for being actively kept by
//                  this MovableMan.
// Return value:    Whether the MO instance was found in the active list or not.

    bool ValidMO(const MovableObject *pMOToCheck);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Actor kept
//                  by this MovableMan or not.                  
// Arguments:       A pointer to the MovableObject to check for Actorness.
// Return value:    Whether the object was found in the Actor list or not.

    bool IsActor(const MovableObject *pMOToCheck);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                  
// Arguments:       A pointer to the MovableObject to check for Itemness.
// Return value:    Whether the object was found in the Item list or not.

    bool IsDevice(const MovableObject *pMOToCheck);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                  
// Arguments:       A pointer to the MovableObject to check for Itemness.
// Return value:    Whether the object was found in the Particle list or not.

    bool IsParticle(const MovableObject *pMOToCheck);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOfActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MOID is that of an MO which either is
//                  or is parented to an active Actor by this MovableMan, or not.
// Arguments:       An MOID to check for Actorness.
// Return value:    Whether the object was found or owned by an MO in the Actor list or not.

    bool IsOfActor(MOID checkMOID);

    /// <summary>
    /// Gives a unique, contiguous id per-actor. This is regenerated every frame.
    /// </summary>
    /// <param name="actor">The actor to get a contiguous id for.</param>
    /// <returns>A contiguous id for the actor. Returns -1 if the actor doesn't exist in MovableMan.</returns>
    /// <remarks>This function is used for AI throttling.</remarks>
    int GetContiguousActorID(const Actor *actor) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRootMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Produces the root MOID of the MOID of a potential child MO to another MO.
// Arguments:       An MOID to get the root MOID of.
// Return value:    The MOID of the root MO of the MO the passed-in MOID represents. This
//                  will be the same as the MOID passed in if the MO is a root itself. It will
//                  be equal to g_NoMOID if the MOID isn't allocated to an MO.

    MOID GetRootMOID(MOID checkMOID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject from the any and all internal lists of MO:s.
//                  After the MO is removed, ownership is effectively released and
//                  transferred to whatever client called this method.
// Arguments:       A pointer to the MovableObject to remove.
// Return value:    Whether the object was found in MovableMan's custody, and consequently
//                  removed. If the MO entry wasn't found, false is returned.

    bool RemoveMO(MovableObject *pMOToRem);

	/// <summary>
	/// Kills and destroys all Actors of a specific Team.
	/// </summary>
	/// <param name="teamToKill">The team to annihilate. If NoTeam is passed in, then NO Actors die.</param>
	/// <returns>How many Actors were killed.</returns>
    int KillAllTeamActors(int teamToKill) const;

	/// <summary>
	/// Kills and destroys all enemy Actors of a specific Team.
	/// </summary>
	/// <param name="teamNotToKill">The team to NOT annihilate. If NoTeam is passed in, then ALL Actors die.</param>
	/// <returns>How many Actors were killed.</returns>
	int KillAllEnemyActors(int teamNotToKill = Activity::NoTeam) const;

    /// <summary>
    /// Adds all Actors in MovableMan to the given list.
    /// </summary>
    /// <param name="transferOwnership">Whether or not ownership of the Actors should be transferred from MovableMan to the list.</param>
    /// <param name="actorList">The list to be filled with Actors.</param>
    /// <param name="onlyTeam">The team to get Actors of. If NoTeam, then all teams will be used.</param>
    /// <param name="noBrains">Whether or not to get brain Actors.</param>
	/// <returns>The number of Actors added to the list.</returns>
    int GetAllActors(bool transferOwnership, std::list<SceneObject *> &actorList, int onlyTeam = -1, bool noBrains = false);

    /// </summary>
    /// <param name="transferOwnership">Whether or not ownershp of the items shoudl be transferred from MovableMan to the list.</param>
    /// <param name="itemList">The list to be filled with items.</param>
	/// <returns>The number of items added to the list.</returns>
    int GetAllItems(bool transferOwnership, std::list<SceneObject *> &itemList);

    /// <summary>
    /// Adds all particles in MovableMan to the given list.
    /// </summary>
    /// <param name="transferOwnership">Whether or not ownership of the particles should be transferred from MovableMan to the list.</param>
    /// <param name="particleList">The list to be filled with particles.</param>
    /// <returns>The number of particles added to the list.</returns>
    int GetAllParticles(bool transferOwnership, std::list<SceneObject *> &particleList);

	/// <summary>
	/// Opens all doors and keeps them open until this is called again with false.
	/// </summary>
	/// <param name="open">Whether to open all doors (true), or close all doors (false).</param>
	/// <param name="team">Which team to open doors for. NoTeam means all teams.</param>
    void OpenAllDoors(bool open = true, int team = Activity::NoTeam) const;

	/// <summary>
	/// Temporarily erases or redraws any material door representations of a specific team.
	/// Used to make pathfinding work better, allowing Actors to navigate through firendly bases despite the door material layer.
	/// </summary>
	/// <param name="eraseDoorMaterial">Whether to erase door material, thereby overriding it, or redraw it and undo the override.</param>
    /// <param name="team">Which team to do this for, NoTeam means all teams.</param>
    void OverrideMaterialDoors(bool eraseDoorMaterial, int team = Activity::NoTeam) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterAlarmEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an AlarmEvent to notify things around that somehting alarming
//                  like a gunshot or explosion just happened.
// Arguments:       The AlarmEvent to register.
// Return value:    None.

    void RegisterAlarmEvent(const AlarmEvent &newEvent);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAlarmEvents
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of AlarmEvent:s from last frame's update.
// Arguments:       None.
// Return value:    The const list of AlarmEvent:s.

    const std::vector<AlarmEvent> & GetAlarmEvents() const { return m_AlarmEvents; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsParticleSettlingEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whetehr particles are set to get copied to the terrain upon
//                  settling
// Arguments:       None.
// Return value:    Whether enabled or not.

    bool IsParticleSettlingEnabled() { return m_SettlingEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EnableParticleSettling
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether particles will get copied into the terrain upon them
//                  settling down.
// Arguments:       Whether to enable or not.
// Return value:    None.

    void EnableParticleSettling(bool enable = true) { m_SettlingEnabled = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMOSubtractionEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether MO's sihouettes can get subtracted from the terrain at all.
// Arguments:       None.
// Return value:    Whether enabled or not.

    bool IsMOSubtractionEnabled() { return m_MOSubtractionEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RedrawOverlappingMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces all objects potnetially overlapping a specific MO to re-draw
//                  this MOID representations onto the MOID bitmap.
// Arguments:       A pointer to the MO to check for overlaps against. Ownerhip is NOT
//                  transferred.
// Return value:    None.

    void RedrawOverlappingMOIDs(MovableObject *pOverlapsThis);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this MovableMan. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawMatter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's all MO's current material representations to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    void DrawMatter(BITMAP *pTargetBitmap, Vector& targetPos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateDrawMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the MOIDs of all current MOs and draws their ID's to a BITMAP
//                  of choice. If there are more than 255 MO's to draw, some will not be.
// Arguments:       A pointer to a BITMAP to draw on.
// Return value:    None.

    void UpdateDrawMOIDs(BITMAP *pTargetBitmap);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the HUDs of all MovableObject:s of this MovableMan to a BITMAP
//                  of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which player's screen is being drawn. Tis affects which actor's HUDs
//                  get drawn.
// Return value:    None.

    void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int which = 0, bool playerControlled = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          VerifyMOIDIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Verifieis whether all elements of MOID index has correct ID. Should be used in Debug mode only.
// Arguments:       None.
// Return value:    None.

	void VerifyMOIDIndex(); 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an object in a global Map collection so it could be found later with FindObjectByUniqueId
// Arguments:       MO to register.
// Return value:    None.

	void RegisterObject(MovableObject * mo);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UnregisterObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an object from the global lookup collection
// Arguments:       MO to remove.
// Return value:    None.

	void UnregisterObject(MovableObject * mo);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FindObjectByUniqueId
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Uses a global lookup map to find an object by it's unique id.
// Arguments:       Unique Id to look for.
// Return value:    Object found or 0 if not found any.

	MovableObject * FindObjectByUniqueID(long int id) { if (m_KnownObjects.count(id) > 0) return m_KnownObjects[id]; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKnownObjectsCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the size of the object registry collection
// Arguments:       None.
// Return value:    Size of the objects registry.

	unsigned int GetKnownObjectsCount() { return m_KnownObjects.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSimUpdateFrameNumber
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the current sim update frame number
// Arguments:       None.
// Return value:    Current sim update frame number.

	unsigned int GetSimUpdateFrameNumber() const { return m_SimUpdateFrameNumber; }

	/// <summary>
	/// Gets pointers to the MOs that are within the given Box, and whose team is not ignored.
	/// </summary>
	/// <param name="box">The Box to get MOs within.</param>
	/// <param name="ignoreTeam">The team to ignore.</param>
	/// <param name="getsHitByMOsOnly">Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.</param>
	/// <returns>Pointers to the MOs that are within the given Box, and whose team is not ignored.</returns>
	const std::vector<MovableObject *> *GetMOsInBox(const Box &box, int ignoreTeam, bool getsHitByMOsOnly) const;

    /// <summary>
    /// Gets pointers to the MOs that are within the given Box, and whose team is not ignored.
    /// </summary>
    /// <param name="box">The Box to get MOs within.</param>
    /// <param name="ignoreTeam">The team to ignore.</param>
    /// <returns>Pointers to the MOs that are within the given Box, and whose team is not ignored.</returns>
	const std::vector<MovableObject *> *GetMOsInBox(const Box &box, int ignoreTeam) const { return GetMOsInBox(box, ignoreTeam, false); }

	/// <summary>
	/// Gets pointers to the MOs that are within the given Box.
	/// </summary>
	/// <param name="box">The Box to get MOs within.</param>
	/// <returns>Pointers to the MOs that are within the given Box.</returns>
    const std::vector<MovableObject *> * GetMOsInBox(const Box &box) const { return GetMOsInBox(box, Activity::NoTeam); }

	/// <summary>
	/// Gets pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
	/// </summary>
	/// <param name="centre">The position to check for MOs in.</param>
	/// <param name="radius">The radius to check for MOs within.</param>
	/// <param name="ignoreTeam">The team to ignore.</param>
	/// <param name="getsHitByMOsOnly">Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.</param>
	/// <returns>Pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.</returns>
	const std::vector<MovableObject *> *GetMOsInRadius(const Vector &centre, float radius, int ignoreTeam, bool getsHitByMOsOnly) const;

	/// <summary>
	/// Gets pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.
	/// </summary>
	/// <param name="centre">The position to check for MOs in.</param>
	/// <param name="radius">The radius to check for MOs within.</param>
	/// <param name="ignoreTeam">The team to ignore.</param>
	/// <returns>Pointers to the MOs that are within the specified radius of the given centre position, and whose team is not ignored.</returns>
	const std::vector<MovableObject *> *GetMOsInRadius(const Vector &centre, float radius, int ignoreTeam) const { return GetMOsInRadius(centre, radius, ignoreTeam, false); }

	/// <summary>
	/// Gets pointers to the MOs that are within the specified radius of the given centre position.
	/// </summary>
	/// <param name="centre">The position to check for MOs in.</param>
	/// <param name="radius">The radius to check for MOs within.</param>
	/// <returns>Pointers to the MOs that are within the specified radius of the given centre position.</returns>
    const std::vector<MovableObject *> * GetMOsInRadius(const Vector &centre, float radius) const { return GetMOsInRadius(centre, radius, Activity::NoTeam); }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // All actors in the scene
    std::deque<Actor *> m_Actors;
    // A map to give a unique contiguous identifier per-actor. This is re-created per frame.
    std::unordered_map<const Actor *, int> m_ContiguousActorIDs;
    // List of items that are pickup-able by actors
    std::deque<MovableObject *> m_Items;
    // List of free, dead particles flying around
    std::deque<MovableObject *> m_Particles;
    // These are the actors/items/particles which were added during a frame.
    // They are moved to the containers above at the end of the frame.
    std::deque<Actor *> m_AddedActors;
    std::deque<MovableObject *> m_AddedItems;
    std::deque<MovableObject *> m_AddedParticles;

    // Mutexes to ensure MOs aren't being removed from separate threads at the same time
    std::mutex m_ActorsMutex;
    std::mutex m_ItemsMutex;
    std::mutex m_ParticlesMutex;

    // Mutexes to ensure MOs aren't being added from separate threads at the same time
    std::mutex m_AddedActorsMutex;
    std::mutex m_AddedItemsMutex;
    std::mutex m_AddedParticlesMutex;

    // Mutex to ensure objects aren't registered/deregistered from separate threads at the same time
    std::mutex m_ObjectRegisteredMutex;

    // Mutex to ensure actors don't change team roster from seperate threads at the same time
    std::mutex m_ActorRosterMutex;

    // Roster of each team's actors, sorted by their X positions in the scene. Actors not owned here
    std::list<Actor *> m_ActorRoster[Activity::MaxTeamCount];
    // Whether to draw HUD lines between the actors of a specific team
    bool m_SortTeamRoster[Activity::MaxTeamCount];
	// Every team's MO footprint
	int m_TeamMOIDCount[Activity::MaxTeamCount];

    // The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
    // This is the last frame's events, is the one for Actors to poll for events, should be cleaned out and refilled each frame.
    std::vector<AlarmEvent> m_AlarmEvents;
    // The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
    // This is the current frame's events, will be filled up during MovableMan Updates, should be transferred to Last Frame at end of update.
    std::vector<AlarmEvent> m_AddedAlarmEvents;

    // Mutexes to ensure alarm events aren't being added from separate threads at the same time
    std::mutex m_AddedAlarmEventsMutex;

    // The list created each frame to register all the current MO's
    std::vector<MovableObject *> m_MOIDIndex;

    // The ration of terrain pixels to be converted into MOPixel:s upon
    // deep impact of MO.
    float m_SplashRatio;
    // The maximum number of loose items allowed.
    int m_MaxDroppedItems;

    // Whether settling of particles is enabled or not
    bool m_SettlingEnabled;
    // Whtehr MO's vcanng et subtracted form the terrain at all
    bool m_MOSubtractionEnabled;

	unsigned int m_SimUpdateFrameNumber;

	// Global map which stores all objects so they could be foud by their unique ID
	std::map<long int, MovableObject *> m_KnownObjects;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

	static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MovableMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    /// <summary>
    /// Travels all of our MOs, updating their location/velocity/physical characteristics.
    /// </summary>
    void Travel();

    /// <summary>
    /// Updates the controllers of all the actors we own.
    /// This is needed for a tricky reason - we want the controller from the activity to override the normal controller state
    /// So we need to update the controller state prior to activity, so the changes from activity are layered on top.
    /// </summary>
    void UpdateControllers();

    /// <summary>
    /// Updates all things that need to be done before we update the controllers.
    /// This is needed because of a very awkward and ugly old code path where controllers were updated in the middle of update, and various mods relied of this behaviour for actions that were therefore delayed by a frame
    /// Ideally we wouldn't need this, but this is all very fragile code and I'd prefer to avoid breaking things.
    /// </summary>
    void PreControllerUpdate();

    // Disallow the use of some implicit methods.
	MovableMan(const MovableMan &reference) = delete;
	MovableMan & operator=(const MovableMan &rhs) = delete;

};

} // namespace RTE

#endif // File