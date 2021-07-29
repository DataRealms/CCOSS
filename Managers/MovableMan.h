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
#include "Entity.h"
#include "SceneMan.h"
#include "LuaMan.h"
#include "Singleton.h"

#define g_MovableMan MovableMan::Instance()

namespace RTE
{

class MovableObject;
class Actor;
class MOPixel;
class AHuman;
class SceneLayer;


//////////////////////////////////////////////////////////////////////////////////////////
// Struct:          AlarmEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A struct to keep all data about a an alarming event for the AI Actors.
// Parent(s):       None.
// Class history:   10/3/2008  AlarmEvent created.

struct AlarmEvent
{
    AlarmEvent() { m_ScenePos.Reset(); m_Team = Activity::NoTeam; m_Range = 1; }
    AlarmEvent(const Vector &pos, int team = Activity::NoTeam, float range = 1) { m_ScenePos = pos; m_Team = (Activity::Teams)team; m_Range = range; }
    
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

	SerializableClassNameGetter
	SerializableOverrideMethods


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is of a
//                  specifc team and closest to a specific scene point.
// Arguments:       Which team to try to get an Actor for. 0 means first team, 1 means 2nd.
//                  The player to get the Actor for. This affects which brain can be marked.
//                  The Scene point to search for the closest to.
//                  The maximum radius around that scene point to search.
//                  A float to be filled out with the distance of the returned closest to
//                  the search point. Will be unaltered if no object was found within radius.
//                  An Actor to exclude from the search. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    An Actor pointer to the requested team's Actor closest to the Scene
//                  point, but not outside the max radius. If no Actor other than the
//                  excluded one was found within the radius of the point, 0 is returned.

    Actor * GetClosestTeamActor(int team, int player, const Vector &scenePoint, int maxRadius, float &getDistance, const Actor *pExcludeThis = 0);


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

    Actor * GetFirstTeamActor(int team, int player) { float temp; return GetClosestTeamActor(team, player, Vector(), 10000000, temp); }


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to this, after it is determined what it is and the
//                  best way to add it is. E.g. if it's an Actor, it will be added as such.
//                  Ownership IS transferred! TODO: ITEMS ARE NOT SORTED OUT YET
// Arguments:       A pointer to the MovableObject to add. Ownership IS transferred!
// Return value:    Whether the MovableObject was successfully added or not. Either way,
//                  ownership was transferred. If no success, the object was deleted.


    bool AddMO(MovableObject *pMOToAdd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Actor to the internal list of MO:s. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here! i.e. Ownership IS transferred!
// Arguments:       A pointer to the Actor to add.
// Return value:    None.

    void AddActor(Actor *pActorToAdd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a pickup-able item to the internal list of items. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here! i.e. Ownership IS transferred!
// Arguments:       A pointer to the item MovableObject to add. Ownership is transferred.
// Return value:    None.

    void AddItem(MovableObject *pItemToAdd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to the internal list of MO:s. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here! i.e. Ownership IS transferred!
// Arguments:       A pointer to the MovableObject to add. Ownership is transferred.
// Return value:    None.

    void AddParticle(MovableObject *pMOToAdd);


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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KillAllActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Kills and destroys all actors of a specific team.
// Arguments:       The team to NOT annihilate, if NoTeam, then ALL actors die.
// Return value:    How many Actors were killed.

    int KillAllActors(int exceptTeam = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EjectAllActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list ALL Actors in the world and removes them from the
//                  MovableMan. Ownership IS transferred!
// Arguments:       The list of Actors to put the evacuated Actor instances in.
//                  The team to only eject Actors of. If NoTeam, then all will be grabbed.
//                  Whether to not grab any brains at all.
// Return value:    How many Actors was transferred to the list.

    int EjectAllActors(std::list<SceneObject *> &actorList, int onlyTeam = -1, bool noBrains = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EjectAllItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list ALL Items in the world and removes them from the
//                  MovableMan. Ownership IS transferred!
// Arguments:       The list of MovableObject:s to put the evacuated MovableObject instances
//                  in.
// Return value:    How many Items were transferred to the list.

    int EjectAllItems(std::list<SceneObject *> &itemList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenAllDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens all doors and keeps them open until this is called again with false.
// Arguments:       Whether to open all doors (true), or undo this action (false)
//                  Which team to do this for. NoTeam means all teams.
// Return value:    None.

    void OpenAllDoors(bool open = true, int team = Activity::NoTeam);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverrideMaterialDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Temporarily erases any material door representations of a specific team.
//                  Used for making pathfinding work better, allowing teammember to navigate
//                  through friendly bases.
// Arguments:       Whether to enable the override (true), or undo this action (false)
//                  Which team to do this for. NoTeam means all teams.
// Return value:    None.

    void OverrideMaterialDoors(bool enable, int team = Activity::NoTeam);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterAlarmEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an AlarmEvent to notify things around that somehting alarming
//                  like a gunshot or explosion just happened.
// Arguments:       The AlarmEvent to register.
// Return value:    None.

    void RegisterAlarmEvent(const AlarmEvent &newEvent) { m_AddedAlarmEvents.push_back(newEvent); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAlarmEvents
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of AlarmEvent:s from last frame's update.
// Arguments:       None.
// Return value:    The const list of AlarmEvent:s.

    const std::list<AlarmEvent> & GetAlarmEvents() const { return m_AlarmEvents; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSloMoThreshold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of new MO:s that need to be added in a single update
//                  to trigger the slo motion effect.
// Arguments:       None.
// Return value:    The number of MO's needed to be added to the MovableMan in a single
//                  update to trigger the slo-mo effect

    int GetSloMoThreshold() { return m_SloMoThreshold; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSloMoDuration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of time, in ms, that the slow-motion effect is active
//                  once it's triggered.
// Arguments:       None.
// Return value:    The amount of time the slow-motion effect runs for, in ms sim time.

    int GetSloMoDuration() { return m_SloMoDuration; }


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

	void OnPieMenu(Actor *pActor);


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // All actors in the scene
    std::deque<Actor *> m_Actors;
    // List of items that are pickup-able by actors
    std::deque<MovableObject *> m_Items;
    // List of free, dead particles flying around
    std::deque<MovableObject *> m_Particles;
    // These are the actors/items/particles which were added during a frame.
    // They are moved to the containers above at the end of the frame.
    std::deque<Actor *> m_AddedActors;
    std::deque<MovableObject *> m_AddedItems;
    std::deque<MovableObject *> m_AddedParticles;

    // Roster of each team's actors, sorted by their X positions in the scene. Actors not owned here
    std::list<Actor *> m_ActorRoster[Activity::MaxTeamCount];
    // Whether to draw HUD lines between the actors of a specific team
    bool m_SortTeamRoster[Activity::MaxTeamCount];
	// Every team's MO footprint
	int m_TeamMOIDCount[Activity::MaxTeamCount];

    // Optimization implementation
    // MO's that have already been asked whether they exist in the manager this frame, and the search result.
    // Gets cleaned out each frame. Does NOT own any instances.
    std::deque<std::pair<const MovableObject *, bool> > m_ValiditySearchResults;

    // The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
    // This is the last frame's events, is the one for Actors to poll for events, should be cleaned out and refilled each frame.
    std::list<AlarmEvent> m_AlarmEvents;
    // The alarm events on the scene where something alarming happened, for use with AI firings awareness os they react to shots fired etc.
    // This is the current frame's events, will be filled up during MovableMan Updates, should be transferred to Last Frame at end of update.
    std::list<AlarmEvent> m_AddedAlarmEvents;

    // The list created each frame to register all the current MO's
    std::vector<MovableObject *> m_MOIDIndex;
    // The ration of terrain pixels to be converted into MOPixel:s upon
    // deep impact of MO.
    float m_SplashRatio;
    // The maximum number of loose items allowed.
    int m_MaxDroppedItems;

    // Timer for measuring periods of slo-mo effects
    Timer m_SloMoTimer;
    // The threshold for how many new MOs in one frame will trigger the slo-mo effect
    int m_SloMoThreshold;
    // The amount of time, in ms, that the slo-mo effect should be in effect when it is triggered
    int m_SloMoDuration;

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


    // Disallow the use of some implicit methods.
	MovableMan(const MovableMan &reference) = delete;
	MovableMan & operator=(const MovableMan &rhs) = delete;

};

} // namespace RTE

#endif // File