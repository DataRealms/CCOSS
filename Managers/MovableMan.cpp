//////////////////////////////////////////////////////////////////////////////////////////
// File:            MovableMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the MovableMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MovableMan.h"

#include "PrimitiveMan.h"
#include "PostProcessMan.h"
#include "PerformanceMan.h"
#include "ThreadMan.h"
#include "PresetMan.h"
#include "AEmitter.h"
#include "AHuman.h"
#include "MOPixel.h"
#include "HeldDevice.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "AtomGroup.h"
#include "Actor.h"
#include "HeldDevice.h"
#include "ADoor.h"
#include "Atom.h"
#include "Scene.h"
#include "FrameMan.h"
#include "SceneMan.h"
#include "SettingsMan.h"
#include "LuaMan.h"
#include "ThreadMan.h"

#include "tracy/Tracy.hpp"

#include <execution>

namespace RTE {

AlarmEvent::AlarmEvent(const Vector &pos, int team, float range)
{
    m_ScenePos = pos;
    m_Team = (Activity::Teams)team;
    m_Range = range * g_FrameMan.GetPlayerScreenWidth() * 0.51F;
}

const std::string MovableMan::c_ClassName = "MovableMan";


// Comparison functor for sorting movable objects by their X position using STL's sort
struct MOXPosComparison {
    bool operator()(MovableObject *pRhs, MovableObject *pLhs) { return pRhs->GetPos().m_X < pLhs->GetPos().m_X; }
};


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MovableMan, effectively
//                  resetting the members of this abstraction level only.

void MovableMan::Clear()
{
    m_Actors.clear();
    m_ContiguousActorIDs.clear();
    m_Items.clear();
    m_Particles.clear();
    m_AddedActors.clear();
    m_AddedItems.clear();
    m_AddedParticles.clear();
    m_ValidActors.clear();
    m_ValidItems.clear();
    m_ValidParticles.clear();
    m_ActorRoster[Activity::TeamOne].clear();
    m_ActorRoster[Activity::TeamTwo].clear();
    m_ActorRoster[Activity::TeamThree].clear();
    m_ActorRoster[Activity::TeamFour].clear();
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;
    m_AddedAlarmEvents.clear();
    m_AlarmEvents.clear();
    m_MOIDIndex.clear();
    m_SplashRatio = 0.75;
	m_MaxDroppedItems = 100;
    m_SettlingEnabled = true;
    m_MOSubtractionEnabled = true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MovableMan object ready for use.

int MovableMan::Initialize()
{
    // TODO: Increase this number, or maybe only for certain classes?
    Entity::ClassInfo::FillAllPools();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int MovableMan::ReadProperty(const std::string_view &propName, Reader &reader)
{
    StartPropertyList(return Serializable::ReadProperty(propName, reader));
    
    MatchProperty("AddEffect", { g_PresetMan.GetEntityPreset(reader); });
    MatchProperty("AddAmmo", { g_PresetMan.GetEntityPreset(reader); });
    MatchProperty("AddDevice", { g_PresetMan.GetEntityPreset(reader); });
    MatchProperty("AddActor", { g_PresetMan.GetEntityPreset(reader); });
    MatchProperty("SplashRatio", { reader >> m_SplashRatio; });
    
    EndPropertyList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MovableMan with a Writer for
//                  later recreation with Create(Reader &reader);

int MovableMan::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer << m_Actors.size();
    for (std::deque<Actor *>::const_iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        writer << **itr;

    writer << m_Particles.size();
    for (std::deque<MovableObject *>::const_iterator itr2 = m_Particles.begin(); itr2 != m_Particles.end(); ++itr2)
        writer << **itr2;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MovableMan object.

void MovableMan::Destroy()
{
    for (std::deque<Actor *>::iterator it1 = m_Actors.begin(); it1 != m_Actors.end(); ++it1)
        delete (*it1);
    for (std::deque<MovableObject *>::iterator it2 = m_Items.begin(); it2 != m_Items.end(); ++it2)
        delete (*it2);
    for (std::deque<MovableObject *>::iterator it3 = m_Particles.begin(); it3 != m_Particles.end(); ++it3)
        delete (*it3);

    Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOFromID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a MO from its MOID. Note that MOID's are only valid during the
//                  same frame as they were assigned to the MOs!

MovableObject * MovableMan::GetMOFromID(MOID whichID) {
	if (whichID != g_NoMOID && whichID != 0 && whichID < m_MOIDIndex.size()) {
		return m_MOIDIndex[whichID];
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MOID MovableMan::GetMOIDPixel(int pixelX, int pixelY, const std::vector<int> &moidList) {
    // Note - We loop through the MOs in reverse to make sure that the topmost (last drawn) MO that overlaps the specified coordinates is the one returned.
    for (auto itr = moidList.rbegin(), itrEnd = moidList.rend(); itr < itrEnd; ++itr) {
        MOID moid = *itr;
        const MovableObject *mo = GetMOFromID(moid);

        RTEAssert(mo, "Null MO found in MOID list!");
        if (mo == nullptr) {
            continue;
        }

		if (mo->GetScale() == 0.0f) {
			return g_NoMOID;
		} else if (mo->HitTestAtPixel(pixelX, pixelY)) {
            return moid;
        }
    }

    return g_NoMOID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an object in a global Map collection so it could be found later with FindObjectByUniqueId
// Arguments:       MO to register.
// Return value:    None.

void MovableMan::RegisterObject(MovableObject * mo) 
{ 
	if (!mo) {
        return;
    }

    std::lock_guard<std::mutex> guard(m_ObjectRegisteredMutex);
    m_KnownObjects[mo->GetUniqueID()] = mo;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UnregisterObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an object from the global lookup collection
// Arguments:       MO to remove.
// Return value:    None.

void MovableMan::UnregisterObject(MovableObject * mo) 
{
	if (!mo) {
        return;
    }

    std::lock_guard<std::mutex> guard(m_ObjectRegisteredMutex);
    m_KnownObjects.erase(mo->GetUniqueID());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<MovableObject *> * MovableMan::GetMOsInBox(const Box &box, int ignoreTeam, bool getsHitByMOsOnly) const {
    std::vector<MovableObject *> *vectorForLua = new std::vector<MovableObject *>();
    *vectorForLua = std::move(g_SceneMan.GetMOIDGrid().GetMOsInBox(box, ignoreTeam, getsHitByMOsOnly));
    return vectorForLua;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<MovableObject *> * MovableMan::GetMOsInRadius(const Vector &centre, float radius, int ignoreTeam, bool getsHitByMOsOnly) const {
    std::vector<MovableObject *> *vectorForLua = new std::vector<MovableObject *>();
    *vectorForLua = std::move(g_SceneMan.GetMOIDGrid().GetMOsInRadius(centre, radius, ignoreTeam, getsHitByMOsOnly));
    return vectorForLua;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PurgeAllMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all MovableObject:s out of this. Effectively empties the world
//                  of anything moving, without resetting all of this' settings.

void MovableMan::PurgeAllMOs()
{
    for (std::deque<Actor*>::iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr) {
        (*itr)->DestroyScriptState();
    }
    for (std::deque<MovableObject*>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr) {
        (*itr)->DestroyScriptState();
    }
    for (std::deque<MovableObject*>::iterator itr = m_Particles.begin(); itr != m_Particles.end(); ++itr) {
        (*itr)->DestroyScriptState();
    }

    for (std::deque<Actor*>::iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr) {
        delete (*itr);
    }
    for (std::deque<MovableObject*>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr) {
        delete (*itr);
    }
    for (std::deque<MovableObject*>::iterator itr = m_Particles.begin(); itr != m_Particles.end(); ++itr) {
        delete (*itr);
    }

    m_Actors.clear();
    m_Items.clear();
    m_Particles.clear();
    m_AddedActors.clear();
    m_AddedItems.clear();
    m_AddedParticles.clear();
    m_ValidActors.clear();
    m_ValidItems.clear();
    m_ValidParticles.clear();
    m_ActorRoster[Activity::TeamOne].clear();
    m_ActorRoster[Activity::TeamTwo].clear();
    m_ActorRoster[Activity::TeamThree].clear();
    m_ActorRoster[Activity::TeamFour].clear();
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;
    m_AddedAlarmEvents.clear();
    m_AlarmEvents.clear();
    m_MOIDIndex.clear();
	m_KnownObjects.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextActorInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the first Actor in the internal Actor list that is
//                  of a specifc group, alternatively the first one AFTER a specific actor!

Actor * MovableMan::GetNextActorInGroup(std::string group, Actor *pAfterThis)
{
    if (group.empty())
        return 0;

    // Begin at the beginning
    std::deque<Actor *>::const_iterator aIt = m_Actors.begin();

    // Search for the actor to start search from, if specified
    if (pAfterThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_Actors.end() && !((*aIt)->IsInGroup(group) && *aIt == pAfterThis); ++aIt)
            ;

        // If we couldn't find the one to search for,
        // then just start at the beginning again and get the first actor at the next step
        if (aIt == m_Actors.end())
            aIt = m_Actors.begin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
            ++aIt;
    }

    // Now search for the first actor of the team from the search point (beginning or otherwise)
    for (; aIt != m_Actors.end() && !(*aIt)->IsInGroup(group); ++aIt)
        ;

    // If nothing found between a specified actor and the end,
    // then restart and see if there's anything between beginning and that specified actor
    if (pAfterThis && aIt == m_Actors.end())
    {
        for (aIt = m_Actors.begin(); aIt != m_Actors.end() && !(*aIt)->IsInGroup(group); ++aIt)
            ;

        // Still nothing?? Should at least get the specified actor and return it! - EDIT No becuase it just may not be there!
//        RTEAssert(aIt != m_Actors.end(), "Search for something after specified actor, and didn't even find the specified actor!?");
    }

    // Still nothing, so return nothing
    if (aIt == m_Actors.end())
        return 0;

    if ((*aIt)->IsInGroup(group))
        return *aIt;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevActorInGroup
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the last Actor in the internal Actor list that is
//                  of a specifc group, alternatively the last one BEFORE a specific actor!

Actor * MovableMan::GetPrevActorInGroup(std::string group, Actor *pBeforeThis)
{
    if (group.empty())
        return 0;

    // Begin at the reverse beginning
    std::deque<Actor *>::reverse_iterator aIt = m_Actors.rbegin();

    // Search for the actor to start search from, if specified
    if (pBeforeThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_Actors.rend() && !((*aIt)->IsInGroup(group) && *aIt == pBeforeThis); ++aIt)
            ;

        // If we couldn't find the one to search for,
        // then just start at the beginning again and get the first actor at the next step
        if (aIt == m_Actors.rend())
            aIt = m_Actors.rbegin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
            ++aIt;
    }

    // Now search for the first actor of the team from the search point (beginning or otherwise)
    for (; aIt != m_Actors.rend() && !(*aIt)->IsInGroup(group); ++aIt)
        ;

    // If nothing found between a specified actor and the end,
    // then restart and see if there's anything between beginning and that specified actor
    if (pBeforeThis && aIt == m_Actors.rend())
    {
        for (aIt = m_Actors.rbegin(); aIt != m_Actors.rend() && !(*aIt)->IsInGroup(group); ++aIt)
            ;

        // Still nothing?? Should at least get the specified actor and return it! - EDIT No becuase it just may not be there!
//        RTEAssert(aIt != m_Actors.rend(), "Search for something after specified actor, and didn't even find the specified actor!?");
    }

    // Still nothing, so return nothing
    if (aIt == m_Actors.rend())
        return 0;

    if ((*aIt)->IsInGroup(group))
        return *aIt;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the first Actor in the internal Actor list that is
//                  of a specifc team, alternatively the first one AFTER a specific actor!

Actor * MovableMan::GetNextTeamActor(int team, Actor *pAfterThis)
{
    if (team < Activity::TeamOne || team >= Activity::MaxTeamCount || m_ActorRoster[team].empty())
        return 0;
/*
    // Begin at the beginning
    std::deque<Actor *>::const_iterator aIt = m_Actors.begin();

    // Search for the actor to start search from, if specified
    if (pAfterThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_Actors.end() && !((*aIt)->GetTeam() == team && *aIt == pAfterThis); ++aIt)
            ;

        // If we couldn't find the one to search for,
        // then just start at the beginning again and get the first actor at the next step
        if (aIt == m_Actors.end())
            aIt = m_Actors.begin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
            ++aIt;
    }

    // Now search for the first actor of the team from the search point (beginning or otherwise)
    for (; aIt != m_Actors.end() && (*aIt)->GetTeam() != team; ++aIt)
        ;

    // If nothing found between a specified actor and the end,
    // then restart and see if there's anything between beginning and that specified actor
    if (pAfterThis && aIt == m_Actors.end())
    {
        for (aIt = m_Actors.begin(); aIt != m_Actors.end() && (*aIt)->GetTeam() != team; ++aIt)
            ;

        // Still nothing?? Should at least get the specified actor and return it! - EDIT No becuase it just may not be there!
//        RTEAssert(aIt != m_Actors.end(), "Search for something after specified actor, and didn't even find the specified actor!?");
    }

    // Still nothing, so return nothing
    if (aIt == m_Actors.end())
        return 0;

    if ((*aIt)->GetTeam() == team)
        return *aIt;

    return 0;
*/
    // First sort the roster
    m_ActorRoster[team].sort(MOXPosComparison());

    // Begin at the beginning
    std::list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin();

    // Search for the actor to start search from, if specified
    if (pAfterThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_ActorRoster[team].end() && *aIt != pAfterThis; ++aIt)
            ;

        // If we couldn't find the one to search for, then just return the first one
        if (aIt == m_ActorRoster[team].end())
            aIt = m_ActorRoster[team].begin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
        {
            ++aIt;
            // If that was the last one, then return the first in the list
            if (aIt == m_ActorRoster[team].end())
                aIt = m_ActorRoster[team].begin();
        }
    }

    RTEAssert((*aIt)->GetTeam() == team, "Actor of wrong team found in the wrong roster!");
    return *aIt;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPrevTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the last Actor in the internal Actor list that is
//                  of a specifc team, alternatively the last one BEFORE a specific actor!

Actor * MovableMan::GetPrevTeamActor(int team, Actor *pBeforeThis)
{
    if (team < Activity::TeamOne || team >= Activity::MaxTeamCount || m_Actors.empty() ||  m_ActorRoster[team].empty())
        return 0;
/* Obsolete, now uses team rosters which are sorted
    // Begin at the reverse beginning
    std::deque<Actor *>::const_reverse_iterator aIt = m_Actors.rbegin();

    // Search for the actor to start search from, if specified
    if (pBeforeThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_Actors.rend() && !((*aIt)->GetTeam() == team && *aIt == pBeforeThis); ++aIt)
            ;

        // If we couldn't find the one to search for,
        // then just start at the beginning again and get the first actor at the next step
        if (aIt == m_Actors.rend())
            aIt = m_Actors.rbegin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
            ++aIt;
    }

    // Now search for the first actor of the team from the search point (beginning or otherwise)
    for (; aIt != m_Actors.rend() && (*aIt)->GetTeam() != team; ++aIt)
        ;

    // If nothing found between a specified actor and the end,
    // then restart and see if there's anything between beginning and that specified actor
    if (pBeforeThis && aIt == m_Actors.rend())
    {
        for (aIt = m_Actors.rbegin(); aIt != m_Actors.rend() && (*aIt)->GetTeam() != team; ++aIt)
            ;

        // Still nothing?? Should at least get the specified actor and return it! - EDIT No becuase it just may not be there!
//        RTEAssert(aIt != m_Actors.rend(), "Search for something after specified actor, and didn't even find the specified actor!?");
    }

    // Still nothing, so return nothing
    if (aIt == m_Actors.rend())
        return 0;

    if ((*aIt)->GetTeam() == team)
        return *aIt;

    return 0;
*/
    // First sort the roster
    m_ActorRoster[team].sort(MOXPosComparison());

    // Begin at the reverse beginning of roster
    std::list<Actor *>::reverse_iterator aIt = m_ActorRoster[team].rbegin();

    // Search for the actor to start search from, if specified
    if (pBeforeThis)
    {
        // Make the iterator point to the specified starting point actor
        for (; aIt != m_ActorRoster[team].rend() && *aIt != pBeforeThis; ++aIt)
            ;

        // If we couldn't find the one to search for, then just return the one at the end
        if (aIt == m_ActorRoster[team].rend())
            aIt = m_ActorRoster[team].rbegin();
        // Go one more step so we're not pointing at the one we're not supposed to get
        else
        {
            ++aIt;
            // If that was the first one, then return the last in the list
            if (aIt == m_ActorRoster[team].rend())
                aIt = m_ActorRoster[team].rbegin();
        }
    }

    RTEAssert((*aIt)->GetTeam() == team, "Actor of wrong team found in the wrong roster!");
    return *aIt;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestTeamActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is of a
//                  specifc team and closest to a specific scene point.

Actor * MovableMan::GetClosestTeamActor(int team, int player, const Vector &scenePoint, int maxRadius, Vector &getDistance, bool onlyPlayerControllableActors, const Actor *excludeThis)
{
    if (team < Activity::NoTeam || team >= Activity::MaxTeamCount || m_Actors.empty() || m_ActorRoster[team].empty())
        return 0;

    Activity *pActivity = g_ActivityMan.GetActivity();

    float sqrShortestDistance = static_cast<float>(maxRadius * maxRadius);
    Actor *pClosestActor = 0;

    // If we're looking for a noteam actor, then go through the entire actor list instead
    if (team == Activity::NoTeam)
    {
        for (std::deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
        {
            if ((*aIt) == excludeThis || (*aIt)->GetTeam() != Activity::NoTeam || (onlyPlayerControllableActors && !(*aIt)->IsPlayerControllable())) {
                continue;
            }

            // Check if even within search radius
            float sqrDistance = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint, g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY()).GetSqrMagnitude();
            if (sqrDistance < sqrShortestDistance)
            {
                sqrShortestDistance = sqrDistance;
                pClosestActor = *aIt;
            }
        }
    }
    // A specific team, so use the rosters instead
    else
    {
        for (std::list<Actor *>::iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
        {
			if ((*aIt) == excludeThis || (onlyPlayerControllableActors && !(*aIt)->IsPlayerControllable()) || (player != NoPlayer && ((*aIt)->GetController()->IsPlayerControlled(player) || (pActivity && pActivity->IsOtherPlayerBrain(*aIt, player))))) {
				continue;
			}

            Vector distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint, g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY());

            // Check if even within search radius
            float sqrDistance = distanceVec.GetSqrMagnitude();
            if (sqrDistance < sqrShortestDistance)
            {
                sqrShortestDistance = sqrDistance;
                pClosestActor = *aIt;
				getDistance.SetXY(distanceVec.GetX(), distanceVec.GetY());
            }
        }
    }

    return pClosestActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestEnemyActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is is not of
//                  the specified team and closest to a specific scene point.

Actor * MovableMan::GetClosestEnemyActor(int team, const Vector &scenePoint, int maxRadius, Vector &getDistance)
{
    if (team < Activity::NoTeam || team >= Activity::MaxTeamCount || m_Actors.empty())
        return 0;
    
    Activity *pActivity = g_ActivityMan.GetActivity();
    
    float sqrShortestDistance = static_cast<float>(maxRadius * maxRadius);
    Actor *pClosestActor = 0;
    
    for (std::deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        if ((*aIt)->GetTeam() == team)
            continue;

        Vector distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint, g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY());
        
        // Check if even within search radius
        float sqrDistance = distanceVec.GetSqrMagnitude();
        if (sqrDistance < sqrShortestDistance)
        {
            sqrShortestDistance = sqrDistance;
            pClosestActor = *aIt;
            getDistance.SetXY(distanceVec.GetX(), distanceVec.GetY());
        }
    }
    
    return pClosestActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is closest
//                  to a specific scene point.

Actor * MovableMan::GetClosestActor(const Vector &scenePoint, int maxRadius, Vector &getDistance, const Actor *pExcludeThis)
{
    if (m_Actors.empty())
        return 0;

    Activity *pActivity = g_ActivityMan.GetActivity();

    float sqrShortestDistance = static_cast<float>(maxRadius * maxRadius);
    Actor *pClosestActor = 0;

    for (std::deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        if ((*aIt) == pExcludeThis)
            continue;

        Vector distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint, g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY());

        // Check if even within search radius
        float sqrDistance = distanceVec.GetSqrMagnitude();
        if (sqrDistance < sqrShortestDistance)
        {
            sqrShortestDistance = sqrDistance;
            pClosestActor = *aIt;
			getDistance.SetXY(distanceVec.GetX(), distanceVec.GetY());
        }
    }

    return pClosestActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor of a specific team that is closest to
//                  a scene point.

Actor * MovableMan::GetClosestBrainActor(int team, const Vector &scenePoint) const
{
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount || m_ActorRoster[team].empty())
        return 0;

    float sqrShortestDistance = std::numeric_limits<float>::infinity();
    sqrShortestDistance *= sqrShortestDistance;

    Actor *pClosestBrain = 0;

    for (std::list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
    {
        if (!(*aIt)->HasObjectInGroup("Brains"))
            continue;

        // Check if closer than best so far
        float sqrDistance = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint, g_SceneMan.SceneWrapsX() || g_SceneMan.SceneWrapsY()).GetSqrMagnitude();
        if (sqrDistance < sqrShortestDistance)
        {
            sqrShortestDistance = sqrDistance;
            pClosestBrain = *aIt;
        }
    }

    return pClosestBrain;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestOtherBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor NOT of a specific team that is closest
//                  to a scene point.

Actor * MovableMan::GetClosestOtherBrainActor(int notOfTeam, const Vector &scenePoint) const
{
    if (notOfTeam < Activity::TeamOne || notOfTeam >= Activity::MaxTeamCount || m_Actors.empty())
        return 0;

    float sqrShortestDistance = std::numeric_limits<float>::infinity();
    sqrShortestDistance *= sqrShortestDistance;

    Actor *pClosestBrain = 0;
    Actor *pContenderBrain = 0;

    for (int t = Activity::TeamOne; t < g_ActivityMan.GetActivity()->GetTeamCount(); ++t)
    {
        if (t != notOfTeam)
        {
            pContenderBrain = GetClosestBrainActor(t, scenePoint);
            float sqrDistance = (pContenderBrain->GetPos() - scenePoint).GetSqrMagnitude();
            if (sqrDistance < sqrShortestDistance)
            {
                sqrShortestDistance = sqrDistance;
                pClosestBrain = pContenderBrain;
            }
        }
    }
    return pClosestBrain;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUnassignedBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor of a specific team.
// Arguments:       Which team to try to get the brain for. 0 means first team, 1 means 2nd.

Actor * MovableMan::GetUnassignedBrain(int team) const
{
    if (/*m_Actors.empty() || */m_ActorRoster[team].empty())
        return 0;

    for (std::list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
    {
        if ((*aIt)->HasObjectInGroup("Brains") && !g_ActivityMan.GetActivity()->IsAssignedBrain(*aIt))
            return *aIt;
    }

    // Also need to look through all the actors added this frame, one might be a brain.
    int actorTeam = Activity::NoTeam;
    for (std::deque<Actor *>::const_iterator aaIt = m_AddedActors.begin(); aaIt != m_AddedActors.end(); ++aaIt)
    {
        int actorTeam = (*aaIt)->GetTeam();
        // Accept no-team brains too - ACTUALLY, DON'T
        if ((actorTeam == team/* || actorTeam == Activity::NoTeam*/) && (*aaIt)->HasObjectInGroup("Brains") && !g_ActivityMan.GetActivity()->IsAssignedBrain(*aaIt))
            return *aaIt;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MovableMan::AddMO(MovableObject *movableObjectToAdd) {
    if (!movableObjectToAdd) {
        return false;
    }

    if (Actor *actorToAdd = dynamic_cast<Actor *>(movableObjectToAdd)) {
        AddActor(actorToAdd);
        return true;
    } else if (HeldDevice *heldDeviceToAdd = dynamic_cast<HeldDevice *>(movableObjectToAdd)) {
        AddItem(heldDeviceToAdd);
        return true;
    }
    AddParticle(movableObjectToAdd);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::AddActor(Actor *actorToAdd) {
	if (actorToAdd) {
		actorToAdd->SetAsAddedToMovableMan();
		actorToAdd->CorrectAttachableAndWoundPositionsAndRotations();

		if (actorToAdd->IsTooFast()) {
			actorToAdd->SetToDelete(true);
		} else {
			if (!dynamic_cast<ADoor *>(actorToAdd)) { actorToAdd->MoveOutOfTerrain(g_MaterialGrass); }
			if (actorToAdd->IsStatus(Actor::INACTIVE)) { actorToAdd->SetStatus(Actor::STABLE); }
			actorToAdd->NotResting();
			actorToAdd->NewFrame();
			actorToAdd->SetAge(g_TimerMan.GetDeltaTimeMS() * -1.0f);
        }

        {
            std::lock_guard<std::mutex> lock(m_AddedActorsMutex);
            m_AddedActors.push_back(actorToAdd);
            m_ValidActors.insert(actorToAdd);

            // This will call SetTeam and subsequently force the team as active.
		    AddActorToTeamRoster(actorToAdd);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::AddItem(HeldDevice *itemToAdd) {
    if (itemToAdd) {
		g_ActivityMan.GetActivity()->ForceSetTeamAsActive(itemToAdd->GetTeam());
		itemToAdd->SetAsAddedToMovableMan();
		itemToAdd->CorrectAttachableAndWoundPositionsAndRotations();

		if (itemToAdd->IsTooFast()) {
			itemToAdd->SetToDelete(true);
		}  else {
            if (!itemToAdd->IsSetToDelete()) { itemToAdd->MoveOutOfTerrain(g_MaterialGrass); }
			itemToAdd->NotResting();
			itemToAdd->NewFrame();
			itemToAdd->SetAge(g_TimerMan.GetDeltaTimeMS() * -1.0f);
        }

        std::lock_guard<std::mutex> lock(m_AddedItemsMutex);
        m_AddedItems.push_back(itemToAdd);
        m_ValidItems.insert(itemToAdd);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::AddParticle(MovableObject *particleToAdd){
    if (particleToAdd) {
		g_ActivityMan.GetActivity()->ForceSetTeamAsActive(particleToAdd->GetTeam());
        particleToAdd->SetAsAddedToMovableMan();
		if (MOSRotating *particleToAddAsMOSRotating = dynamic_cast<MOSRotating *>(particleToAdd)) { particleToAddAsMOSRotating->CorrectAttachableAndWoundPositionsAndRotations(); }

		if (particleToAdd->IsTooFast()) {
			particleToAdd->SetToDelete(true);
		} else {
			//TODO consider moving particles out of grass. It's old code that was removed because it's slow to do this for every particle.
            particleToAdd->NotResting();
            particleToAdd->NewFrame();
            particleToAdd->SetAge(g_TimerMan.GetDeltaTimeMS() * -1.0f);
        }
		if (particleToAdd->IsDevice()) {
            std::lock_guard<std::mutex> lock(m_AddedItemsMutex);
			m_AddedItems.push_back(particleToAdd);
            m_ValidItems.insert(particleToAdd);
		} else {
            std::lock_guard<std::mutex> lock(m_AddedParticlesMutex);
			m_AddedParticles.push_back(particleToAdd);
            m_ValidParticles.insert(particleToAdd);
		}
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an Actor from the internal list of MO:s. After the Actor is
//                  removed, ownership is effectively released and transferred to whatever
//                  client called this method.

Actor * MovableMan::RemoveActor(MovableObject *pActorToRem)
{
    Actor *removed = nullptr;

    if (pActorToRem)
    {
        for (std::deque<Actor *>::iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        {
            if (*itr == pActorToRem)
            {
                std::lock_guard<std::mutex> lock(m_ActorsMutex);
                removed = *itr;
                m_ValidActors.erase(*itr);
                m_Actors.erase(itr);
                break;
            }
        }
        // Try the newly added actors if we couldn't find it in the regular deque
        if (!removed)
        {
            for (std::deque<Actor *>::iterator itr = m_AddedActors.begin(); itr != m_AddedActors.end(); ++itr)
            {
                if (*itr == pActorToRem)
                {
                    std::lock_guard<std::mutex> lock(m_AddedActorsMutex);
                    removed = *itr;
                    m_ValidActors.erase(*itr);
                    m_AddedActors.erase(itr);
                    break;
                }
            }
        }
		RemoveActorFromTeamRoster(dynamic_cast<Actor *>(pActorToRem));
		pActorToRem->SetAsAddedToMovableMan(false);
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a pickup-able MovableObject item from the internal list of
//                  MO:s. After the item is removed, ownership is effectively released and
//                  transferred to whatever client called this method.

MovableObject * MovableMan::RemoveItem(MovableObject *pItemToRem)
{
    MovableObject *removed = nullptr;

    if (pItemToRem)
    {
        for (std::deque<MovableObject *>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr)
        {
            if (*itr == pItemToRem)
            {
                std::lock_guard<std::mutex> lock(m_ItemsMutex);
                removed = *itr;
                m_ValidItems.erase(*itr);
                m_Items.erase(itr);
                break;
            }
        }
        // Try the newly added items if we couldn't find it in the regular deque
        if (!removed)
        {
            for (std::deque<MovableObject *>::iterator itr = m_AddedItems.begin(); itr != m_AddedItems.end(); ++itr)
            {
                if (*itr == pItemToRem)
                {
                    std::lock_guard<std::mutex> lock(m_AddedItemsMutex);
                    removed = *itr;
                    m_ValidItems.erase(*itr);
                    m_AddedItems.erase(itr);
                    break;
                }
            }
        }
		pItemToRem->SetAsAddedToMovableMan(false);
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject from the internal list of MO:s. After the
//                  MO is removed, ownership is effectively released and transferred to
//                  whatever client called this method.

MovableObject * MovableMan::RemoveParticle(MovableObject *pMOToRem)
{
    MovableObject *removed = nullptr;

    if (pMOToRem)
    {
        for (std::deque<MovableObject *>::iterator itr = m_Particles.begin(); itr != m_Particles.end(); ++itr)
        {
            if (*itr == pMOToRem)
            {
                std::lock_guard<std::mutex> lock(m_ParticlesMutex);
                removed = *itr;
                m_ValidParticles.erase(*itr);
                m_Particles.erase(itr);
                break;
            }
        }
        // Try the newly added particles if we couldn't find it in the regular deque
        if (!removed)
        {
            for (std::deque<MovableObject *>::iterator itr = m_AddedParticles.begin(); itr != m_AddedParticles.end(); ++itr)
            {
                if (*itr == pMOToRem)
                {
                    std::lock_guard<std::mutex> lock(m_AddedParticlesMutex);
                    removed = *itr;
                    m_ValidParticles.erase(*itr);
                    m_AddedParticles.erase(itr);
                    break;
                }
            }
        }
		pMOToRem->SetAsAddedToMovableMan(false);
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddActorToTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds actor to internal team roster
// Arguments:       Pointer to actor
// Return value:    None.

void MovableMan::AddActorToTeamRoster(Actor * pActorToAdd)
{
	if (!pActorToAdd) {
		return;
    }

	// Add to the team roster and then sort it too
	int team = pActorToAdd->GetTeam();
	// Also re-set the TEam so that the Team Icons get set up properly
	pActorToAdd->SetTeam(team);
	// Only add to a roster if it's on a team AND is controllable (eg doors are not)
	if (team >= Activity::TeamOne && team < Activity::MaxTeamCount && pActorToAdd->IsControllable())
	{
        std::lock_guard<std::mutex> lock(m_ActorRosterMutex);
		m_ActorRoster[pActorToAdd->GetTeam()].push_back(pActorToAdd);
		m_ActorRoster[pActorToAdd->GetTeam()].sort(MOXPosComparison());
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveActorToTeamRoster
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes actor from internal team roster
// Arguments:       Pointer to actor
// Return value:    None.

void MovableMan::RemoveActorFromTeamRoster(Actor * pActorToRem)
{
	if (!pActorToRem) {
		return;
    }

	int team = pActorToRem->GetTeam();

	// Remove from roster as well
	if (team >= Activity::TeamOne && team < Activity::MaxTeamCount) {
        std::lock_guard<std::mutex> lock(m_ActorRosterMutex);
		m_ActorRoster[team].remove(pActorToRem);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeActorTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes actor team and updates team rosters.

void MovableMan::ChangeActorTeam(Actor * pActor, int team)
{
	if (!pActor) {
		return;
    }

	if (pActor->IsPlayerControlled()) { g_ActivityMan.GetActivity()->LoseControlOfActor(pActor->GetController()->GetPlayer()); }

	RemoveActorFromTeamRoster(pActor);
	pActor->SetTeam(team);
	AddActorToTeamRoster(pActor);

	// Because doors affect the team-based pathfinders, we need to tell them there's been a change.
	// This is hackily done by erasing the door material, updating the pathfinders, then redrawing it and updating them again so they properly account for the door's new team.
	if (ADoor *actorAsADoor = dynamic_cast<ADoor *>(pActor); actorAsADoor && actorAsADoor->GetDoorMaterialDrawn()) {
		actorAsADoor->TempEraseOrRedrawDoorMaterial(true);
		g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(actorAsADoor->GetBoundingBox());
		g_SceneMan.GetScene()->UpdatePathFinding();
		actorAsADoor->TempEraseOrRedrawDoorMaterial(false);
		g_SceneMan.GetTerrain()->AddUpdatedMaterialArea(actorAsADoor->GetBoundingBox());
		g_SceneMan.GetScene()->UpdatePathFinding();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidateMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through and checks that all MOID's have valid MO pointers
//                  associated with them. This shuold only be used for testing, as it will
//                  crash the app if validation fails.

bool MovableMan::ValidateMOIDs() {
#ifdef DEBUG_BUILD
    for (const MovableObject *mo : m_MOIDIndex) {
        RTEAssert(mo, "Null MO found!");
    }
#endif
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject pointer points to an
//                  MO that's currently active in the simulation, and kept by this MovableMan.

bool MovableMan::ValidMO(const MovableObject *pMOToCheck) {
    bool exists = m_ValidActors.find(pMOToCheck) != m_ValidActors.end() ||
                  m_ValidItems.find(pMOToCheck) != m_ValidItems.end() ||
                  m_ValidParticles.find(pMOToCheck) != m_ValidParticles.end();

    return pMOToCheck && exists;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Actor kept
//                  by this MovableMan or not.                  

bool MovableMan::IsActor(const MovableObject *pMOToCheck)
{
    return pMOToCheck && m_ValidActors.find(pMOToCheck) != m_ValidActors.end();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                  

bool MovableMan::IsDevice(const MovableObject *pMOToCheck)
{
    return pMOToCheck && m_ValidItems.find(pMOToCheck) != m_ValidItems.end();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                      

bool MovableMan::IsParticle(const MovableObject *pMOToCheck)
{
    return pMOToCheck && m_ValidParticles.find(pMOToCheck) != m_ValidParticles.end();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOfActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MOID is that of an MO which either is
//                  or is parented to an active Actor by this MovableMan, or not.

bool MovableMan::IsOfActor(MOID checkMOID)
{
    if (checkMOID == g_NoMOID)
        return false;

    bool found = false;
    MovableObject *pMO = GetMOFromID(checkMOID);

    if (pMO)
    {
        MOID rootMOID = pMO->GetRootID();
        if (checkMOID != g_NoMOID)
        {
            for (std::deque<Actor *>::iterator itr = m_Actors.begin(); !found && itr != m_Actors.end(); ++itr)
            {
                if ((*itr)->GetID() == checkMOID || (*itr)->GetID() == rootMOID)
                {
                    found = true;
                    break;
                }
            }
            // Check actors just added this frame
            if (!found)
            {
                for (std::deque<Actor *>::iterator itr = m_AddedActors.begin(); !found && itr != m_AddedActors.end(); ++itr)
                {
                    if ((*itr)->GetID() == checkMOID || (*itr)->GetID() == rootMOID)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    return found;
}

//////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::GetContiguousActorID(const Actor *actor) const {
    auto itr = m_ContiguousActorIDs.find(actor);
    if (itr == m_ContiguousActorIDs.end()) {
        return -1;
    }

    return itr->second;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRootMOID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Produces the root MOID of the MOID of a potential child MO to another MO.

MOID MovableMan::GetRootMOID(MOID checkMOID)
{
    MovableObject *pMO = GetMOFromID(checkMOID);
    if (pMO)
        return pMO->GetRootID();

    return g_NoMOID;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject from the any and all internal lists of MO:s.
//                  After the MO is removed, ownership is effectively released and
//                  transferred to whatever client called this method.

bool MovableMan::RemoveMO(MovableObject *pMOToRem)
{
    if (pMOToRem)
    {
        if (RemoveActor(pMOToRem))
            return true;
        if (RemoveItem(pMOToRem))
            return true;
        if (RemoveParticle(pMOToRem))
            return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::KillAllTeamActors(int teamToKill) const {
    int killCount = 0;

    for (std::deque<Actor *> actorList : { m_Actors, m_AddedActors }) {
        for (Actor *actor : actorList) {
            if (actor->GetTeam() == teamToKill) {
                const AHuman *actorAsHuman = dynamic_cast<AHuman *>(actor);
                if (actorAsHuman && actorAsHuman->GetHead()) {
                    actorAsHuman->GetHead()->GibThis();
                } else {
                    actor->GibThis();
                }
                killCount++;
            }
        }
    }

    return killCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::KillAllEnemyActors(int teamNotToKill) const {
    int killCount = 0;

    for (std::deque<Actor *> actorList : { m_Actors, m_AddedActors }) {
        for (Actor *actor : actorList) {
            if (actor->GetTeam() != teamNotToKill) {
                const AHuman *actorAsHuman = dynamic_cast<AHuman *>(actor);
                if (actorAsHuman && actorAsHuman->GetHead()) {
                    actorAsHuman->GetHead()->GibThis();
                } else {
                    actor->GibThis();
                }
                killCount++;
            }
        }
    }

    return killCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::GetAllActors(bool transferOwnership, std::list<SceneObject *> &actorList, int onlyTeam, bool noBrains)
{
    int addedCount = 0;

    // Add all regular Actors
    for (std::deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        Actor *actor = *aIt;
        // Only grab ones of a specific team; delete all others
        if ((onlyTeam == Activity::NoTeam || actor->GetTeam() == onlyTeam) && (!noBrains || !actor->HasObjectInGroup("Brains")))
        {
            actorList.push_back(actor);
            addedCount++;
        }
        else if (transferOwnership)
        {
            delete actor;
        }
    }

    // Add all Actors added this frame
    for (std::deque<Actor *>::iterator aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
    {
        Actor *actor = *aIt;
        // Only grab ones of a specific team; delete all others
        if ((onlyTeam == Activity::NoTeam || actor->GetTeam() == onlyTeam) && (!noBrains || !actor->HasObjectInGroup("Brains")))
        {
            actorList.push_back(actor);
            addedCount++;
        }
        else if (transferOwnership)
        {
            delete actor;
        }
    }

    if (transferOwnership)
    {
        // Clear the internal Actor lists; we transferred the ownership of them
        m_Actors.clear();
        m_AddedActors.clear();
        m_ValidActors.clear();

        // Also clear the actor rosters
        for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        {
            m_ActorRoster[team].clear();
        }
    }

    return addedCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::GetAllItems(bool transferOwnership, std::list<SceneObject *> &itemList)
{
    int addedCount = 0;

    // Add all regular Items
    for (std::deque<MovableObject *>::iterator iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
    {
        itemList.push_back((*iIt));
        addedCount++;
    }

    // Add all Items added this frame
    for (std::deque<MovableObject *>::iterator iIt = m_AddedItems.begin(); iIt != m_AddedItems.end(); ++iIt)
    {
        itemList.push_back((*iIt));
        addedCount++;
    }

    if (transferOwnership)
    {
        // Clear the internal Item list; we transferred the ownership of them
        m_Items.clear();
        m_AddedItems.clear();
        m_ValidItems.clear();
    }

    return addedCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::GetAllParticles(bool transferOwnership, std::list<SceneObject *> &particleList)
{
    int addedCount = 0;

    // Add all regular particles
    for (std::deque<MovableObject *>::iterator iIt = m_Particles.begin(); iIt != m_Particles.end(); ++iIt)
    {
        particleList.push_back((*iIt));
        addedCount++;
    }

    // Add all particles added this frame
    for (std::deque<MovableObject *>::iterator iIt = m_AddedParticles.begin(); iIt != m_AddedParticles.end(); ++iIt)
    {
        particleList.push_back((*iIt));
        addedCount++;
    }

    if (transferOwnership)
    {
        // Clear the internal Particle list; we transferred the ownership of them
        m_Particles.clear();
        m_AddedParticles.clear();
        m_ValidParticles.clear();
    }

    return addedCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MovableMan::GetTeamMOIDCount(int team) const
{
	if (team > Activity::NoTeam && team < Activity::MaxTeamCount)
		return m_TeamMOIDCount[team];
	else
		return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::OpenAllDoors(bool open, int team) const {
	for (std::deque<Actor *> actorDeque : { m_Actors, m_AddedActors }) {
		for (Actor *actor : actorDeque) {
			if (ADoor *actorAsADoor = dynamic_cast<ADoor *>(actor); actorAsADoor && (team == Activity::NoTeam || actorAsADoor->GetTeam() == team)) {
				if (actorAsADoor->GetDoorState() != (open ? ADoor::DoorState::OPEN : ADoor::DoorState::CLOSED)) {
					actorAsADoor->Update();
					actorAsADoor->SetClosedByDefault(!open);
				}
				actorAsADoor->ResetSensorTimer();
				if (open) {
					actorAsADoor->OpenDoor();
				} else {
					actorAsADoor->CloseDoor();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Completely tear out and delete this.
// It shouldn't belong to MovableMan, instead it probably ought to be on the pathfinder. On that note, pathfinders shouldn't be part of the scene!
// AIMan? PathingMan? Something like that. Ideally, we completely tear out this hack, and allow for doors in a completely different way.
void MovableMan::OverrideMaterialDoors(bool eraseDoorMaterial, int team) const {
	for (std::deque<Actor *> actorDeque : { m_Actors, m_AddedActors }) {
		for (Actor *actor : actorDeque) {
			if (ADoor *actorAsDoor = dynamic_cast<ADoor *>(actor); actorAsDoor && (team == Activity::NoTeam || actorAsDoor->GetTeam() == team)) {
				actorAsDoor->TempEraseOrRedrawDoorMaterial(eraseDoorMaterial);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::RegisterAlarmEvent(const AlarmEvent &newEvent)
{
    std::lock_guard<std::mutex> lock(m_AddedAlarmEventsMutex);
    m_AddedAlarmEvents.push_back(newEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void callLuaFunctionOnMORecursive(MovableObject* mo, const std::string& functionName, const std::vector<const Entity*>& functionEntityArguments, const std::vector<std::string_view>& functionLiteralArguments, const std::vector<LuabindObjectWrapper*>& functionObjectArguments) {
    if (MOSRotating* mosr = dynamic_cast<MOSRotating*>(mo)) {
        for (auto attachablrItr = mosr->GetAttachableList().begin(); attachablrItr != mosr->GetAttachableList().end(); ) {
            Attachable* attachable = *attachablrItr;
            ++attachablrItr;

            attachable->RunScriptedFunctionInAppropriateScripts(functionName, false, false, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
            callLuaFunctionOnMORecursive(attachable, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
        }

        for (auto woundItr = mosr->GetWoundList().begin(); woundItr != mosr->GetWoundList().end(); ) {
            AEmitter* wound = *woundItr;
            ++woundItr;

            wound->RunScriptedFunctionInAppropriateScripts(functionName, false, false, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
            callLuaFunctionOnMORecursive(wound, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
        }
    }

    mo->RunScriptedFunctionInAppropriateScripts(functionName, false, false, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::RunLuaFunctionOnAllMOs(const std::string &functionName, bool includeAdded, const std::vector<const Entity*> &functionEntityArguments, const std::vector<std::string_view> &functionLiteralArguments, const std::vector<LuabindObjectWrapper*> &functionObjectArguments) {
    if (includeAdded) {
        for (Actor* actor : m_AddedActors) {
            callLuaFunctionOnMORecursive(actor, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
        }

        for (MovableObject *item : m_AddedItems) {
            callLuaFunctionOnMORecursive(item, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
        }

        for (MovableObject* particle : m_AddedParticles) {
            callLuaFunctionOnMORecursive(particle, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
        }
    }
    
    for (Actor *actor : m_Actors) {
        callLuaFunctionOnMORecursive(actor, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
    }

    for (MovableObject *item : m_Items) {
        callLuaFunctionOnMORecursive(item, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
    }

    for (MovableObject* particle : m_Particles) {
        callLuaFunctionOnMORecursive(particle, functionName, functionEntityArguments, functionLiteralArguments, functionObjectArguments);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void reloadLuaFunctionsOnMORecursive(MovableObject* mo) {
    if (MOSRotating* mosr = dynamic_cast<MOSRotating*>(mo)) {
        for (auto attachablrItr = mosr->GetAttachableList().begin(); attachablrItr != mosr->GetAttachableList().end(); ) {
            Attachable* attachable = *attachablrItr;
            ++attachablrItr;

            attachable->ReloadScripts();
            reloadLuaFunctionsOnMORecursive(attachable);
        }

        for (auto woundItr = mosr->GetWoundList().begin(); woundItr != mosr->GetWoundList().end(); ) {
            AEmitter* wound = *woundItr;
            ++woundItr;

            wound->ReloadScripts();
            reloadLuaFunctionsOnMORecursive(wound);
        }
    }

    mo->ReloadScripts();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::ReloadLuaScripts() {
    for (Actor* actor : m_AddedActors) {
        reloadLuaFunctionsOnMORecursive(actor);
    }

    for (MovableObject* item : m_AddedItems) {
        reloadLuaFunctionsOnMORecursive(item);
    }

    for (MovableObject* particle : m_AddedParticles) {
        reloadLuaFunctionsOnMORecursive(particle);
    }

    for (Actor* actor : m_Actors) {
        reloadLuaFunctionsOnMORecursive(actor);
    }

    for (MovableObject* item : m_Items) {
        reloadLuaFunctionsOnMORecursive(item);
    }

    for (MovableObject* particle : m_Particles) {
        reloadLuaFunctionsOnMORecursive(particle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this MovableMan. Supposed to be done every frame.

void MovableMan::Update()
{
    ZoneScoped;

    // Don't update if paused
    if (g_ActivityMan.GetActivity() && g_ActivityMan.ActivityPaused()) {
        return;
    }

	m_SimUpdateFrameNumber++;

    g_PostProcessMan.ClearScenePostEffects();

    // Reset the draw HUD roster line settings
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;

    // Move all last frame's alarm events into the proper buffer, and clear out the new one to fill up with this frame's
    m_AlarmEvents.clear();
    for (std::vector<AlarmEvent>::iterator aeItr = m_AddedAlarmEvents.begin(); aeItr != m_AddedAlarmEvents.end(); ++aeItr) {
        m_AlarmEvents.push_back(*aeItr);
    }
    m_AddedAlarmEvents.clear();

    // Travel MOs
    Travel();

    // Prior to controller/AI update, execute lua callbacks
    g_LuaMan.ExecuteLuaScriptCallbacks();

    // Updates everything needed prior to AI/user input being processed
    // Fugly hack to keep backwards compat with scripts that rely on weird frame-delay-ordering behaviours
    // TODO, cleanup the pre-controller update and post-controller updates to have some consistent logic of what goes where
	PreControllerUpdate();

    // Updates AI/user input
	UpdateControllers();

    // Will use some common iterators
    std::deque<Actor *>::iterator aIt;
    std::deque<Actor *>::iterator amidIt;
    std::deque<MovableObject *>::iterator iIt;
    std::deque<MovableObject *>::iterator imidIt;
    std::deque<MovableObject *>::iterator parIt;
    std::deque<MovableObject *>::iterator midIt;

    // Update all multithreaded scripts for all objects
    g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ScriptsUpdate);
    {
        ZoneScopedN("Multithreaded Scripts Update");

        const std::string threadedUpdate = "ThreadedUpdate"; // avoid string reconstruction

        LuaStatesArray& luaStates = g_LuaMan.GetThreadedScriptStates();
        g_ThreadMan.GetPriorityThreadPool().parallelize_loop(luaStates.size(),
            [&](int start, int end) {
                RTEAssert(start + 1 == end, "Threaded script state being updated across multiple threads!");
                LuaStateWrapper& luaState = luaStates[start];
                g_LuaMan.SetThreadLuaStateOverride(&luaState);

                for (MovableObject *mo : luaState.GetRegisteredMOs()) {
                    mo->RunScriptedFunctionInAppropriateScripts(threadedUpdate, false, false, {}, {}, {});
                }

                g_LuaMan.SetThreadLuaStateOverride(nullptr);
            }).wait();
    }

    {
        ZoneScopedN("Multithreaded Scripts SyncedUpdate");

        const std::string syncedUpdate = "SyncedUpdate"; // avoid string reconstruction

        for (LuaStateWrapper& luaState : g_LuaMan.GetThreadedScriptStates()) {
            g_LuaMan.SetThreadLuaStateOverride(&luaState);

            for (MovableObject* mo : luaState.GetRegisteredMOs()) {
                if (mo->HasRequestedSyncedUpdate()) {
                    mo->RunScriptedFunctionInAppropriateScripts(syncedUpdate, false, false, {}, {}, {});
                    mo->ResetRequestedSyncedUpdateFlag();
                }
            }

            g_LuaMan.SetThreadLuaStateOverride(nullptr);
        }
    }
    g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ScriptsUpdate);

    {
        {
            ZoneScopedN("Actors Update");

		    g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsUpdate);
            for (Actor* actor : m_Actors) {
                actor->Update();

                g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ScriptsUpdate);
                actor->UpdateScripts();
                g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ScriptsUpdate);

                actor->ApplyImpulses();
            }
            g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsUpdate);
        }

        {
            ZoneScopedN("Items Update");

            int count = 0;
            int itemLimit = m_Items.size() - m_MaxDroppedItems;
            for (iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt, ++count) {
                (*iIt)->Update();

                g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ScriptsUpdate);
                (*iIt)->UpdateScripts();
                g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ScriptsUpdate);

                (*iIt)->ApplyImpulses();
                if (count <= itemLimit) {
                    (*iIt)->SetToSettle(true);
                }
            }
        }

        {
            ZoneScopedN("Particles Update");

            g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
            for (MovableObject* particle : m_Particles) {
                particle->Update();

                g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ScriptsUpdate);
                particle->UpdateScripts();
                g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ScriptsUpdate);

                particle->ApplyImpulses();
                particle->RestDetection();
                // Copy particles that are at rest to the terrain and mark them for deletion.
                if (particle->IsAtRest()) {
                    particle->SetToSettle(true);
                }
            }
            g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
        }

        {
            ZoneScopedN("Post Update");

            for (Actor* actor : m_Actors) {
                actor->PostUpdate();
            }
            for (MovableObject* item : m_Items) {
                item->PostUpdate();
            }
            for (MovableObject* particle : m_Particles) {
                particle->PostUpdate();
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // TRANSFER ALL MOs ADDED THIS FRAME
    // All Actors, Items, and Particles added this frame now are officially added

    {
        ZoneScopedN("MO Transfer and Deletion");

        {
            // Actors
            for (aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
            {
                // Delete instead if it's marked for it
                if (!(*aIt)->IsSetToDelete())
                    m_Actors.push_back(*aIt);
                else
			    {
                    m_ValidActors.erase(*aIt);

				    // Also remove actor from the roster
                    if ((*aIt)->GetTeam() >= 0) {
                        //m_ActorRoster[(*aIt)->GetTeam()].remove(*aIt);
                        RemoveActorFromTeamRoster(*aIt);
                    }

                    (*aIt)->DestroyScriptState();
                    delete (*aIt);
			    }
            }
            m_AddedActors.clear();

            // Items
            for (iIt = m_AddedItems.begin(); iIt != m_AddedItems.end(); ++iIt)
            {
                // Delete instead if it's marked for it
                if (!(*iIt)->IsSetToDelete()) {
                    m_Items.push_back(*iIt);
                } else {
                    m_ValidItems.erase(*iIt);
                    (*iIt)->DestroyScriptState();
                    delete (*iIt);
                }
            }
            m_AddedItems.clear();

            // Particles
            for (parIt = m_AddedParticles.begin(); parIt != m_AddedParticles.end(); ++parIt)
            {
                // Delete instead if it's marked for it
                if (!(*parIt)->IsSetToDelete()) {
                    m_Particles.push_back(*parIt);
                } else {
                    m_ValidParticles.erase(*parIt);
                    (*parIt)->DestroyScriptState();
                    delete (*parIt);
                }
            }
            m_AddedParticles.clear();
        }

        ////////////////////////////////////////////////////////////////////////////
        // Copy (Settle) Pass

        {
            // DEATH //////////////////////////////////////////////////////////
            // Transfer dead actors from Actor list to particle list
            aIt = partition(m_Actors.begin(), m_Actors.end(), std::not_fn(std::mem_fn(&Actor::IsDead)));
            amidIt = aIt;

            // Move dead Actor to particles list
            if (amidIt != m_Actors.end()/* && m_Actors.size() > 1*/)
            {
                while (aIt != m_Actors.end())
                {
                    // Report the death of the actor to the game
                    g_ActivityMan.GetActivity()->ReportDeath((*aIt)->GetTeam());

                    // Add to the particles list
                    m_Particles.push_back(*aIt);
                    m_ValidParticles.insert(*aIt);
                    // Remove from the team roster

                    if ((*aIt)->GetTeam() >= 0) {
                        //m_ActorRoster[(*aIt)->GetTeam()].remove(*aIt);
                        RemoveActorFromTeamRoster(*aIt);
                    }
                
                    m_ValidActors.erase(*aIt);
                    aIt++;
                }
                // Try to set the existing iterator to a safer value, erase can crash in debug mode otherwise?
                aIt = m_Actors.begin();
                m_Actors.erase(amidIt, m_Actors.end());
            }

            // ITEM SETTLE //////////////////////////////////////////////////////////
            // Transfer excess items to particle list - use stable partition, item orde is important
            iIt = stable_partition(m_Items.begin(), m_Items.end(), std::not_fn(std::mem_fn(&MovableObject::ToSettle)));
            imidIt = iIt;

            // Move force-settled items to particles list
            if (imidIt != m_Items.end()/* && m_Items.size() > 1*/)
            {
                while (iIt != m_Items.end())
                {
                    (*iIt)->SetToSettle(false);
				    // Disable TDExplosive's immunity to settling
                    if ((*iIt)->GetRestThreshold() < 0) {
                        (*iIt)->SetRestThreshold(500);
                    }
                    m_ValidItems.erase(*iIt);
                    m_Particles.push_back(*iIt);
                    iIt++;
                }
                m_Items.erase(imidIt, m_Items.end());
            }

            // DELETE //////////////////////////////////////////////////////////
            // Only delete after all travels & updates are done
            // Actors
            aIt = partition(m_Actors.begin(), m_Actors.end(), std::not_fn(std::mem_fn(&MovableObject::ToDelete)));
            amidIt = aIt;

            while (aIt != m_Actors.end())
            {
			    // Set brain to 0 to avoid crashes due to brain deletion
			    Activity * pActivity = g_ActivityMan.GetActivity();
			    if (pActivity)
			    {
				    if (pActivity->IsAssignedBrain(*aIt))
					    pActivity->SetPlayerBrain(0, pActivity->IsBrainOfWhichPlayer(*aIt));

				    pActivity->ReportDeath((*aIt)->GetTeam());
			    }

                // Remove from team rosters
			    if ((*aIt)->GetTeam() >= Activity::TeamOne && (*aIt)->GetTeam() < Activity::MaxTeamCount)
                    //m_ActorRoster[(*aIt)->GetTeam()].remove(*aIt);
				    RemoveActorFromTeamRoster(*aIt);

                // Delete
                m_ValidActors.erase(*aIt);
                (*aIt)->DestroyScriptState();
                delete (*aIt);
                aIt++;
            }
            // Try to set the existing iterator to a safer value, erase can crash in debug mode otherwise?
            aIt = m_Actors.begin();
            m_Actors.erase(amidIt, m_Actors.end());

            // Items
            iIt = stable_partition(m_Items.begin(), m_Items.end(), std::not_fn(std::mem_fn(&MovableObject::ToDelete)));
            imidIt = iIt;

            while (iIt != m_Items.end()) {
                m_ValidItems.erase(*iIt);
                (*iIt)->DestroyScriptState();
                delete (*iIt);
                iIt++;
            }
            m_Items.erase(imidIt, m_Items.end());

            // Particles
            parIt = partition(m_Particles.begin(), m_Particles.end(), std::not_fn(std::mem_fn(&MovableObject::ToDelete)));
            midIt = parIt;

            while (parIt != m_Particles.end()) {
                m_ValidParticles.erase(*parIt);
                (*parIt)->DestroyScriptState();
                delete (*parIt);
                parIt++;
            }
            m_Particles.erase(midIt, m_Particles.end());
        }

        // SETTLE PARTICLES //////////////////////////////////////////////////
        // Only settle after all updates and deletions are done
	    if (m_SettlingEnabled) {
		    parIt = partition(m_Particles.begin(), m_Particles.end(), std::not_fn(std::mem_fn(&MovableObject::ToSettle)));
		    midIt = parIt;

		    while (parIt != m_Particles.end()) {
			    Vector parPos((*parIt)->GetPos());
			    Material const * terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(parPos.GetFloorIntX(), parPos.GetFloorIntY()));
			    int piling = (*parIt)->GetMaterial()->GetPiling();
			    if (piling > 0) {
				    for (int s = 0; s < piling && (terrMat->GetIndex() == (*parIt)->GetMaterial()->GetIndex() || terrMat->GetIndex() == (*parIt)->GetMaterial()->GetSettleMaterial()); ++s) {
					    if ((piling - s) % 2 == 0) {
						    parPos.m_Y -= 1.0F;
					    } else {
						    parPos.m_X += (RandomNum() >= 0.5F ? 1.0F : -1.0F);
					    }
					    terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(parPos.GetFloorIntX(), parPos.GetFloorIntY()));
				    }
				    (*parIt)->SetPos(parPos.GetFloored());
			    }
			    if ((*parIt)->GetDrawPriority() >= terrMat->GetPriority()) { (*parIt)->DrawToTerrain(g_SceneMan.GetTerrain()); }
                m_ValidParticles.erase(*parIt);
                (*parIt)->DestroyScriptState();
			    delete (*parIt);
                parIt++;
		    }
		    m_Particles.erase(midIt, m_Particles.end());
	    }
    }

    // We've finished stuff that can interact with lua script, so it's the ideal time to start a gc run
    g_LuaMan.StartAsyncGarbageCollection();

    ////////////////////////////////////////////////////////////////////////
    // Draw the MO matter and IDs to their layers for next frame
    m_DrawMOIDsTask = g_ThreadMan.GetPriorityThreadPool().submit([this]() {
        UpdateDrawMOIDs();
    });

    // Sort team rosters if necessary
    {
        if (m_SortTeamRoster[Activity::TeamOne])
            m_ActorRoster[Activity::TeamOne].sort(MOXPosComparison());
        if (m_SortTeamRoster[Activity::TeamTwo])
            m_ActorRoster[Activity::TeamTwo].sort(MOXPosComparison());
        if (m_SortTeamRoster[Activity::TeamThree])
            m_ActorRoster[Activity::TeamThree].sort(MOXPosComparison());
        if (m_SortTeamRoster[Activity::TeamFour])
            m_ActorRoster[Activity::TeamFour].sort(MOXPosComparison());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::Travel()
{
    ZoneScoped;

    if (m_DrawMOIDsTask.valid()) {
        m_DrawMOIDsTask.wait();
    }

    // Travel Actors
    {
        ZoneScopedN("Actors Travel");

        g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsTravel);
        for (auto aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
        {
            (*aIt)->NewFrame();
            (*aIt)->ApplyForces();
            (*aIt)->PreTravel();
            (*aIt)->Travel();
            (*aIt)->PostTravel();
        }
        g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsTravel);
    }

    // Travel items
    {
        ZoneScopedN("Items Travel");

        for (auto iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
        {
            (*iIt)->NewFrame();
            (*iIt)->ApplyForces();
            (*iIt)->PreTravel();
            (*iIt)->Travel();
            (*iIt)->PostTravel();
        }
    }

    // Travel particles
    {
        ZoneScopedN("Particles Travel");

        g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ParticlesTravel);
        for (auto parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
        {
            (*parIt)->ApplyForces();
            (*parIt)->PreTravel();
            (*parIt)->Travel();
            (*parIt)->PostTravel();
            (*parIt)->NewFrame();
        }
        g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ParticlesTravel);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::UpdateControllers()
{
    ZoneScoped;

    g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsAI);
    {
        for (Actor* actor : m_Actors) {
            actor->GetController()->Update();
        }

        LuaStatesArray& luaStates = g_LuaMan.GetThreadedScriptStates();
        g_ThreadMan.GetPriorityThreadPool().parallelize_loop(luaStates.size(),
            [&](int start, int end) {
                RTEAssert(start + 1 == end, "Threaded script state being updated across multiple threads!");
                LuaStateWrapper& luaState = luaStates[start];
                g_LuaMan.SetThreadLuaStateOverride(&luaState);
                for (Actor *actor : m_Actors) {
                    if (actor->GetLuaState() == &luaState && actor->GetController()->ShouldUpdateAIThisFrame()) {
                        actor->RunScriptedFunctionInAppropriateScripts("ThreadedUpdateAI", false, true, {}, {}, {});
                    }
                }
                g_LuaMan.SetThreadLuaStateOverride(nullptr);
            }).wait();

        for (Actor* actor : m_Actors) {
            if (actor->GetController()->ShouldUpdateAIThisFrame()) {
                actor->RunScriptedFunctionInAppropriateScripts("UpdateAI", false, true, {}, {}, {});
            }
        }
    }
    g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsAI);
}

//////////////////////////////////////////////////////////////////////////////////////////

void MovableMan::PreControllerUpdate()
{
    ZoneScoped;

    g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsUpdate);
    for (Actor *actor : m_Actors) {
        actor->PreControllerUpdate();
    }
    g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsUpdate);

    for (MovableObject* item : m_Items) {
        item->PreControllerUpdate();
    }

    g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
    for (MovableObject* particle : m_Particles) {
        particle->PreControllerUpdate();
    }
    g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawMatter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's all MO's current material representations to a
//                  BITMAP of choice.

void MovableMan::DrawMatter(BITMAP *pTargetBitmap, Vector &targetPos)
{
    // Draw objects to accumulation bitmap
    for (std::deque<Actor *>::iterator aIt = --m_Actors.end(); aIt != --m_Actors.begin(); --aIt)
        (*aIt)->Draw(pTargetBitmap, targetPos, g_DrawMaterial);

    for (std::deque<MovableObject *>::iterator parIt = --m_Particles.end(); parIt != --m_Particles.begin(); --parIt)
        (*parIt)->Draw(pTargetBitmap, targetPos, g_DrawMaterial);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          VerifyMOIDIndex
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Verifieis whether all elements of MOID index has correct ID. Should be used in Debug mode only.
// Arguments:       None.
// Return value:    None.

void MovableMan::VerifyMOIDIndex()
{
	int count = 0;
	for (std::vector<MovableObject *>::iterator aIt = m_MOIDIndex.begin(); aIt != m_MOIDIndex.end(); ++aIt)
	{
		if (*aIt)
		{
			RTEAssert((*aIt)->GetID() == g_NoMOID || (*aIt)->GetID() == count, "MOIDIndex broken!");
			RTEAssert((*aIt)->GetRootID() == g_NoMOID || ((*aIt)->GetRootID() >= 0 && (*aIt)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
		}
		count++;
		if (count == g_NoMOID) count++;
	}


	for (std::deque<MovableObject *>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr)
	{
		RTEAssert((*itr)->GetID() == g_NoMOID || (*itr)->GetID() < GetMOIDCount(), "MOIDIndex broken!");
		RTEAssert((*itr)->GetRootID() == g_NoMOID || ((*itr)->GetRootID() >= 0 && (*itr)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
	}
	// Try the items just added this frame
	for (std::deque<MovableObject *>::iterator itr = m_AddedItems.begin(); itr != m_AddedItems.end(); ++itr)
	{
		RTEAssert((*itr)->GetID() == g_NoMOID || (*itr)->GetID() < GetMOIDCount(), "MOIDIndex broken!");
		RTEAssert((*itr)->GetRootID() == g_NoMOID || ((*itr)->GetRootID() >= 0 && (*itr)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateDrawMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the MOIDs of all current MOs and draws their ID's to the MOID grid

void MovableMan::UpdateDrawMOIDs()
{
    ZoneScoped;

    ///////////////////////////////////////////////////
    // Clear the MOID layer before starting to delete stuff which may be in the MOIDIndex
    g_SceneMan.ClearAllMOIDDrawings();

    // Clear the index each frame and do it over because MO's get added and deleted between each frame.
    m_MOIDIndex.clear();
    m_ContiguousActorIDs.clear();

    // Add a null and start counter at 1 because MOID == 0 means no MO.
    // - Update: This isnt' true anymore, but still keep 0 free just to be safe
    m_MOIDIndex.push_back(0);

    MOID currentMOID = 1;

    int actorID = 0;
    for (Actor *actor : m_Actors) {
        m_ContiguousActorIDs[actor] = actorID++;
		if (!actor->IsSetToDelete()) {
            actor->UpdateMOID(m_MOIDIndex);
            actor->Draw(nullptr, Vector(), g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        } else {
            actor->SetAsNoID();
        }
    }

    for (MovableObject *item : m_Items) {
        if (!item->IsSetToDelete()) {
            item->UpdateMOID(m_MOIDIndex);
            item->Draw(nullptr, Vector(), g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        } else {
            item->SetAsNoID();
        }
    }

    for (MovableObject *particle : m_Particles) {
        if (!particle->IsSetToDelete()) {
            particle->UpdateMOID(m_MOIDIndex);
            particle->Draw(nullptr, Vector(), g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        } else {
            particle->SetAsNoID();
        }
    }

    // COUNT MOID USAGE PER TEAM  //////////////////////////////////////////////////
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; team++) {
        m_TeamMOIDCount[team] = 0;
    }

    for (auto itr = m_MOIDIndex.begin(); itr != m_MOIDIndex.end(); ++itr) {
        if (*itr) {
            int team = (*itr)->GetTeam();
            if (team > Activity::NoTeam && team < Activity::MaxTeamCount) {
                m_TeamMOIDCount[team]++;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's current graphical representation to a
//                  BITMAP of choice.

void MovableMan::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    ZoneScoped;

    // Draw objects to accumulation bitmap, in reverse order so actors appear on top.

    {
        ZoneScopedN("Particles Draw");

        for (std::deque<MovableObject*>::iterator parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt) {
            (*parIt)->Draw(pTargetBitmap, targetPos);
        }
    }

    {
        ZoneScopedN("Items Draw");

        for (std::deque<MovableObject*>::reverse_iterator itmIt = m_Items.rbegin(); itmIt != m_Items.rend(); ++itmIt) {
            (*itmIt)->Draw(pTargetBitmap, targetPos);
        }
    }

    {
        ZoneScopedN("Actors Draw");

        for (std::deque<Actor*>::reverse_iterator aIt = m_Actors.rbegin(); aIt != m_Actors.rend(); ++aIt) {
            (*aIt)->Draw(pTargetBitmap, targetPos);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's current graphical representation to a
//                  BITMAP of choice.

void MovableMan::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int which, bool playerControlled)
{
    ZoneScoped;

    // Draw HUD elements
	for (std::deque<MovableObject *>::reverse_iterator itmIt = m_Items.rbegin(); itmIt != m_Items.rend(); ++itmIt)
        (*itmIt)->DrawHUD(pTargetBitmap, targetPos, which);

    for (std::deque<Actor *>::reverse_iterator aIt = m_Actors.rbegin(); aIt != m_Actors.rend(); ++aIt)
        (*aIt)->DrawHUD(pTargetBitmap, targetPos, which);
}

} // namespace RTE
