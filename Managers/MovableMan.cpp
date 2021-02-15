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
#include "PostProcessMan.h"
#include "PerformanceMan.h"
#include "PresetMan.h"
#include "AHuman.h"
#include "MOPixel.h"
#include "HeldDevice.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "AtomGroup.h"
#include "Actor.h"
#include "ADoor.h"
#include "Atom.h"

namespace RTE {

const string MovableMan::c_ClassName = "MovableMan";


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
    m_Items.clear();
    m_Particles.clear();
    m_AddedActors.clear();
    m_AddedItems.clear();
    m_AddedParticles.clear();
    m_ActorRoster[Activity::TeamOne].clear();
    m_ActorRoster[Activity::TeamTwo].clear();
    m_ActorRoster[Activity::TeamThree].clear();
    m_ActorRoster[Activity::TeamFour].clear();
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;
    m_ValiditySearchResults.clear();
    m_AddedAlarmEvents.clear();
    m_AlarmEvents.clear();
    m_MOIDIndex.clear();
    m_SplashRatio = 0.75;
    m_MaxDroppedItems = 25;
    m_SloMoTimer.Reset();
    m_SloMoThreshold = 100;
    m_SloMoDuration = 1000;
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

    // Set the time limit to 0 so it will report as being past it from the get go
    m_SloMoTimer.SetSimTimeLimitMS(0);
    m_SloMoTimer.SetRealTimeLimitMS(0);

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
    if (propName == "AddEffect")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddAmmo")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddDevice")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddActor")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "SplashRatio")
        reader >> m_SplashRatio;
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
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
    for (deque<Actor *>::const_iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        writer << **itr;

    writer << m_Particles.size();
    for (deque<MovableObject *>::const_iterator itr2 = m_Particles.begin(); itr2 != m_Particles.end(); ++itr2)
        writer << **itr2;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MovableMan object.

void MovableMan::Destroy()
{
    for (deque<Actor *>::iterator it1 = m_Actors.begin(); it1 != m_Actors.end(); ++it1)
        delete (*it1);
    for (deque<MovableObject *>::iterator it2 = m_Items.begin(); it2 != m_Items.end(); ++it2)
        delete (*it2);
    for (deque<MovableObject *>::iterator it3 = m_Particles.begin(); it3 != m_Particles.end(); ++it3)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an object in a global Map collection so it could be found later with FindObjectByUniqueId
// Arguments:       MO to register.
// Return value:    None.

void MovableMan::RegisterObject(MovableObject * mo) 
{ 
	if (mo) 
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
	if (mo)
	{
		m_KnownObjects.erase(mo->GetUniqueID());
		//g_ConsoleMan.PrintString(std::to_string(mo->GetUniqueID()));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PurgeAllMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all MovableObject:s out of this. Effectively empties the world
//                  of anything moving, without resetting all of this' settings.

void MovableMan::PurgeAllMOs()
{
    for (deque<Actor *>::iterator it1 = m_Actors.begin(); it1 != m_Actors.end(); ++it1)
        delete (*it1);
    for (deque<MovableObject *>::iterator it2 = m_Items.begin(); it2 != m_Items.end(); ++it2)
        delete (*it2);
    for (deque<MovableObject *>::iterator it3 = m_Particles.begin(); it3 != m_Particles.end(); ++it3)
        delete (*it3);

    m_Actors.clear();
    m_Items.clear();
    m_Particles.clear();
    m_AddedActors.clear();
    m_AddedItems.clear();
    m_AddedParticles.clear();
    m_ActorRoster[Activity::TeamOne].clear();
    m_ActorRoster[Activity::TeamTwo].clear();
    m_ActorRoster[Activity::TeamThree].clear();
    m_ActorRoster[Activity::TeamFour].clear();
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;
    m_ValiditySearchResults.clear();
    m_AddedAlarmEvents.clear();
    m_AlarmEvents.clear();
    m_MOIDIndex.clear();

    // Set the time limit to 0 so it will report as being past it from the start of simulation
    m_SloMoTimer.SetRealTimeLimitMS(0);
    m_SloMoTimer.SetSimTimeLimitMS(0);

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
    deque<Actor *>::const_iterator aIt = m_Actors.begin();

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
    deque<Actor *>::reverse_iterator aIt = m_Actors.rbegin();

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
    deque<Actor *>::const_iterator aIt = m_Actors.begin();

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
    list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin();

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
    deque<Actor *>::const_reverse_iterator aIt = m_Actors.rbegin();

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
    list<Actor *>::reverse_iterator aIt = m_ActorRoster[team].rbegin();

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

Actor * MovableMan::GetClosestTeamActor(int team, int player, const Vector &scenePoint, int maxRadius, float &getDistance, const Actor *pExcludeThis)
{
    if (team < Activity::NoTeam || team >= Activity::MaxTeamCount || m_Actors.empty() ||  m_ActorRoster[team].empty())
        return 0;

    Activity *pActivity = g_ActivityMan.GetActivity();

    Vector distanceVec;
    float distance;
    float shortestDistance = maxRadius;
    Actor *pClosestActor = 0;

    // If we're looking for a noteam actor, then go through the entire actor list instead
    if (team == Activity::NoTeam)
    {
        for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
        {
            if ((*aIt) == pExcludeThis || (*aIt)->GetTeam() != Activity::NoTeam)
                continue;

            distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint);
            distance = distanceVec.GetMagnitude();

            // Check if even within search radius
            if (distance < shortestDistance)
            {
                shortestDistance = distance;
                pClosestActor = *aIt;
            }
        }
    }
    // A specific team, so use the rosters instead
    else
    {
        for (list<Actor *>::iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
        {
            if ((*aIt) == pExcludeThis || (*aIt)->GetController()->IsPlayerControlled(player) || (pActivity && pActivity->IsOtherPlayerBrain(*aIt, player)))
                continue;

            distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint);
            distance = distanceVec.GetMagnitude();

            // Check if even within search radius
            if (distance < shortestDistance)
            {
                shortestDistance = distance;
                pClosestActor = *aIt;
            }
        }
    }

    getDistance = shortestDistance;
    return pClosestActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestEnemyActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to an Actor in the internal Actor list that is is not of
//                  the specified team and closest to a specific scene point.

Actor * MovableMan::GetClosestEnemyActor(int team, const Vector &scenePoint, int maxRadius, Vector &getDistance)
{
    if (team < Activity::NoTeam || team >= Activity::MaxTeamCount || m_Actors.empty() ||  m_ActorRoster[team].empty())
        return 0;
    
    Activity *pActivity = g_ActivityMan.GetActivity();
    
    Vector distanceVec;
    float distance;
    float shortestDistance = maxRadius;
    Actor *pClosestActor = 0;
    
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        if ((*aIt)->GetTeam() == team)
            continue;

        distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint);
        distance = distanceVec.GetMagnitude();
        
        // Check if even within search radius
        if (distance < shortestDistance)
        {
            shortestDistance = distance;
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

Actor * MovableMan::GetClosestActor(Vector &scenePoint, int maxRadius, float &getDistance, const Actor *pExcludeThis)
{
    if (m_Actors.empty())
        return 0;

    Activity *pActivity = g_ActivityMan.GetActivity();

    Vector distanceVec;
    float distance;
    float shortestDistance = maxRadius;
    Actor *pClosestActor = 0;

    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        if ((*aIt) == pExcludeThis)
            continue;

        distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint);
        distance = distanceVec.GetMagnitude();

        // Check if even within search radius
        if (distance < shortestDistance)
        {
            shortestDistance = distance;
            pClosestActor = *aIt;
        }
    }

    getDistance = shortestDistance;
    return pClosestActor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetClosestBrainActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get a pointer to the brain actor of a specific team that is closest to
//                  a scene point.

Actor * MovableMan::GetClosestBrainActor(int team, const Vector &scenePoint) const
{
    if (team < Activity::TeamOne || team >= Activity::MaxTeamCount || m_Actors.empty() ||  m_ActorRoster[team].empty())
        return 0;

    Vector distanceVec;
    float distance;
    float shortestDistance = g_SceneMan.GetSceneDim().GetLargest();
    Actor *pClosestBrain = 0;

    for (list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
    {
        if (!(*aIt)->HasObjectInGroup("Brains"))
            continue;

        distanceVec = g_SceneMan.ShortestDistance((*aIt)->GetPos(), scenePoint);
        distance = distanceVec.GetMagnitude();

        // Check if closer than best so far
        if (distance < shortestDistance)
        {
            shortestDistance = distance;
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

    float testDistance = g_SceneMan.GetSceneDim().GetLargest();
    float shortestDistance = g_SceneMan.GetSceneDim().GetLargest();
    Actor *pClosestBrain = 0;
    Actor *pContenderBrain = 0;

    for (int t = Activity::TeamOne; t < g_ActivityMan.GetActivity()->GetTeamCount(); ++t)
    {
        if (t != notOfTeam)
        {
            pContenderBrain = GetClosestBrainActor(t, scenePoint);
            testDistance = (pContenderBrain->GetPos() - scenePoint).GetMagnitude();
            if (testDistance < shortestDistance)
            {
                pClosestBrain = pContenderBrain;
                shortestDistance = testDistance;
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

    for (list<Actor *>::const_iterator aIt = m_ActorRoster[team].begin(); aIt != m_ActorRoster[team].end(); ++aIt)
    {
        if ((*aIt)->HasObjectInGroup("Brains") && !g_ActivityMan.GetActivity()->IsAssignedBrain(*aIt))
            return *aIt;
    }

    // Also need to look through all the actors added this frame, one might be a brain.
    int actorTeam = Activity::NoTeam;
    for (deque<Actor *>::const_iterator aaIt = m_AddedActors.begin(); aaIt != m_AddedActors.end(); ++aaIt)
    {
        int actorTeam = (*aaIt)->GetTeam();
        // Accept no-team brains too - ACTUALLY, DON'T
        if ((actorTeam == team/* || actorTeam == Activity::NoTeam*/) && (*aaIt)->HasObjectInGroup("Brains") && !g_ActivityMan.GetActivity()->IsAssignedBrain(*aaIt))
            return *aaIt;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to this, after it is determined what it is and the
//                  best way to add it is. E.g. if it's an Actor, it will be added as such.
//                  Ownership IS transferred!

bool MovableMan::AddMO(MovableObject *pMOToAdd) {
    if (!pMOToAdd) {
        return false;
    }

    pMOToAdd->SetAsAddedToMovableMan();

    // Find out what kind it is and apply accordingly
    if (Actor *pActor = dynamic_cast<Actor *>(pMOToAdd)) {
        AddActor(pActor);
        return true;
    } else if (HeldDevice *pHeldDevice = dynamic_cast<HeldDevice *>(pMOToAdd)) {
        AddItem(pHeldDevice);
        return true;
    } else {
        AddParticle(pMOToAdd);
        return true;
    }
/*
// TODO: sort out helddevices too?
    else if (MovableObject *pMO = dynamic_cast<MovableObject *>(pMOToAdd))
    {
        AddParticle(pMO);
        return true;
    }
    // Weird type, get rid of since we have ownership
    else
    {
        delete pMOToAdd;
        pMOToAdd = 0;
    }
*/
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds an Actor to the internal list of MO:s. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here!

void MovableMan::AddActor(Actor *pActorToAdd)
{
    if (pActorToAdd)
    {
//        pActorToAdd->SetPrevPos(pActorToAdd->GetPos());
//        pActorToAdd->Update();
//        pActorToAdd->PostTravel();
        pActorToAdd->SetAsAddedToMovableMan();

        // Filter out stupid fast objects
        if (pActorToAdd->IsTooFast())
            pActorToAdd->SetToDelete(true);
        else
        {
            // Should not be done to doors, which will get jolted around when they shouldn't!
            if (!dynamic_cast<ADoor *>(pActorToAdd))
                pActorToAdd->MoveOutOfTerrain(g_MaterialGrass);
            if (pActorToAdd->IsStatus(Actor::INACTIVE))
                pActorToAdd->SetStatus(Actor::STABLE);
            pActorToAdd->NotResting();
            pActorToAdd->NewFrame();
            pActorToAdd->SetAge(0);
        }
        m_AddedActors.push_back(pActorToAdd);

		AddActorToTeamRoster(pActorToAdd);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a pickup-able item to the internal list of items. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here! i.e. Ownership IS transferred!

void MovableMan::AddItem(MovableObject *pItemToAdd)
{
    if (pItemToAdd)
    {
//        pItemToAdd->SetPrevPos(pItemToAdd->GetPos());
//        pItemToAdd->Update();
//        pItemToAdd->PostTravel();
        pItemToAdd->SetAsAddedToMovableMan();

        // Filter out stupid fast objects
        if (pItemToAdd->IsTooFast())
            pItemToAdd->SetToDelete(true);
        else
        {
            // Move out so not embedded in terrain - and if we can't, DON'T delete it now, but mark for deletion at the end of frame!$@
            // Deleting it now causes crashes as it may have had its MOID registered and things collidng withthat MOID area would be led to deleted memory
// Don't delete; buried stuff is cool, also causes problems becuase it thinks hollowed out material is not air and therefore  makes objects inside bunkers disappear!
//        if (!pItemToAdd->MoveOutOfTerrain(g_MaterialGrass))
//            pItemToAdd->SetToDelete();
            pItemToAdd->MoveOutOfTerrain(g_MaterialGrass);

            pItemToAdd->NotResting();
            pItemToAdd->NewFrame();
            pItemToAdd->SetAge(0);
        }
        m_AddedItems.push_back(pItemToAdd);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a MovableObject to the internal list of MO:s. Destruction and 
//                  deletion will be taken care of automatically. Do NOT delete the passed
//                  MO after adding it here! i.e. Ownership IS transferred!

void MovableMan::AddParticle(MovableObject *pMOToAdd)
{
    if (pMOToAdd)
    {
//        pMOToAdd->SetPrevPos(pMOToAdd->GetPos());
//        pMOToAdd->Update();
//        pMOToAdd->Travel();
//        pMOToAdd->PostTravel();
        pMOToAdd->SetAsAddedToMovableMan();

        // Filter out stupid fast objects
        if (pMOToAdd->IsTooFast())
            pMOToAdd->SetToDelete(true);
        else
        {
            // Move out so not embedded in terrain
// This is a bit slow to be doing on every particle added!
//            pMOToAdd->MoveOutOfTerrain(g_MaterialGrass);

            pMOToAdd->NotResting();
            pMOToAdd->NewFrame();
            pMOToAdd->SetAge(0);
        }
        if (pMOToAdd->IsDevice())
            m_AddedItems.push_back(pMOToAdd);
        else
            m_AddedParticles.push_back(pMOToAdd);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes an Actor from the internal list of MO:s. After the Actor is
//                  removed, ownership is effectively released and transferred to whatever
//                  client called this method.

bool MovableMan::RemoveActor(MovableObject *pActorToRem)
{
    bool removed = false;

    if (pActorToRem)
    {
        for (deque<Actor *>::iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        {
            if (*itr == pActorToRem)
            {
                m_Actors.erase(itr);
                removed = true;
                break;
            }
        }
        // Try the newly added actors if we couldn't find it in the regular deque
        if (!removed)
        {
            for (deque<Actor *>::iterator itr = m_AddedActors.begin(); itr != m_AddedActors.end(); ++itr)
            {
                if (*itr == pActorToRem)
                {
                    m_AddedActors.erase(itr);
                    removed = true;
                    break;
                }
            }
        }
		RemoveActorFromTeamRoster(dynamic_cast<Actor *>(pActorToRem));
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a pickup-able MovableObject item from the internal list of
//                  MO:s. After the item is removed, ownership is effectively released and
//                  transferred to whatever client called this method.

bool MovableMan::RemoveItem(MovableObject *pItemToRem)
{
    bool removed = false;

    if (pItemToRem)
    {
        for (deque<MovableObject *>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr)
        {
            if (*itr == pItemToRem)
            {
                m_Items.erase(itr);
                removed = true;
                break;
            }
        }
        // Try the newly added items if we couldn't find it in the regular deque
        if (!removed)
        {
            for (deque<MovableObject *>::iterator itr = m_AddedItems.begin(); itr != m_AddedItems.end(); ++itr)
            {
                if (*itr == pItemToRem)
                {
                    m_AddedItems.erase(itr);
                    removed = true;
                    break;
                }
            }
        }
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
	if (!pActorToAdd)
		return;

	// Add to the team roster and then sort it too
	int team = pActorToAdd->GetTeam();
	// Also re-set the TEam so that the Team Icons get set up properly
	pActorToAdd->SetTeam(team);
	// Only add to a roster if it's on a team AND is controllable (eg doors are not)
	if (team >= Activity::TeamOne && team < Activity::MaxTeamCount && pActorToAdd->IsControllable())
	{
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
	if (!pActorToRem)
		return;

	int team = pActorToRem->GetTeam();

	// Remove from roster as well
	if (team >= Activity::TeamOne && team < Activity::MaxTeamCount)
		m_ActorRoster[team].remove(pActorToRem);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeActorTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes actor team and updates team rosters.

void MovableMan::ChangeActorTeam(Actor * pActor, int team)
{
	if (!pActor)
		return;

	RemoveActorFromTeamRoster(pActor);
	pActor->SetTeam(team);
	AddActorToTeamRoster(pActor);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a MovableObject from the internal list of MO:s. After the
//                  MO is removed, ownership is effectively released and transferred to
//                  whatever client called this method.

bool MovableMan::RemoveParticle(MovableObject *pMOToRem)
{
    bool removed = false;

    if (pMOToRem)
    {
        for (deque<MovableObject *>::iterator itr = m_Particles.begin(); itr != m_Particles.end(); ++itr)
        {
            if (*itr == pMOToRem)
            {
                m_Particles.erase(itr);
                removed = true;
                break;
            }
        }
        // Try the newly added particles if we couldn't find it in the regular deque
        if (!removed)
        {
            for (deque<MovableObject *>::iterator itr = m_AddedParticles.begin(); itr != m_AddedParticles.end(); ++itr)
            {
                if (*itr == pMOToRem)
                {
                    m_AddedParticles.erase(itr);
                    removed = true;
                    break;
                }
            }
        }
    }
    return removed;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidateMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through and checks that all MOID's have valid MO pointers
//                  associated with them. This shuold only be used for testing, as it will
//                  crash the app if validation fails.

bool MovableMan::ValidateMOIDs()
{
#ifdef DEBUG_BUILD
    float test;
    for (vector<MovableObject *>::iterator itr = m_MOIDIndex.begin(); itr != m_MOIDIndex.end(); ++itr)
    {
        if (*itr)
            test = (*itr)->GetGoldValue();
    }
#endif
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ValidMO
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject pointer points to an
//                  MO that's currently active in the simulation, and kept by this
//                  MovableMan. Internal optimization is made so that the same MO can
//                  efficiently be checked many times during the same frame.

bool MovableMan::ValidMO(const MovableObject *pMOToCheck)
{
    bool found = false;

    if (pMOToCheck)
    {

        // See if this MO has been found earlier this frame
        for (deque<pair <const MovableObject *, bool> >::iterator itr = m_ValiditySearchResults.begin(); !found && itr != m_ValiditySearchResults.end(); ++itr)
        {
            // If the MO is found to have been searched for earlier this frame, then just return the search results
            if (itr->first == pMOToCheck)
                return itr->second;
        }

        // Check actors
        found = found ? true : IsActor(pMOToCheck);
        // Check Items
        found = found ? true : IsDevice(pMOToCheck);
        // Check particles
        found = found ? true : IsParticle(pMOToCheck);

        // Save search result for future requests this frame
        m_ValiditySearchResults.push_back(pair<const MovableObject *, bool>(pMOToCheck, found));
    }

    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Actor kept
//                  by this MovableMan or not.                  

bool MovableMan::IsActor(const MovableObject *pMOToCheck)
{
    bool found = false;

    if (pMOToCheck)
    {
        for (deque<Actor *>::iterator itr = m_Actors.begin(); !found && itr != m_Actors.end(); ++itr)
        {
            if (*itr == pMOToCheck)
            {
                found = true;
                break;
            }
        }
        // Try the actors just added this frame
        if (!found)
        {
            for (deque<Actor *>::iterator itr = m_AddedActors.begin(); !found && itr != m_AddedActors.end(); ++itr)
            {
                if (*itr == pMOToCheck)
                {
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsDevice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                  

bool MovableMan::IsDevice(const MovableObject *pMOToCheck)
{
    bool found = false;

    if (pMOToCheck)
    {
        for (deque<MovableObject *>::iterator itr = m_Items.begin(); !found && itr != m_Items.end(); ++itr)
        {
            if (*itr == pMOToCheck)
            {
                found = true;
                break;
            }
        }
        // Try the items just added this frame
        if (!found)
        {
            for (deque<MovableObject *>::iterator itr = m_AddedItems.begin(); !found && itr != m_AddedItems.end(); ++itr)
            {
                if (*itr == pMOToCheck)
                {
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsParticle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the passed in MovableObject is an active Item kept
//                  by this MovableMan or not.                      

bool MovableMan::IsParticle(const MovableObject *pMOToCheck)
{
    bool found = false;

    if (pMOToCheck)
    {
        for (deque<MovableObject *>::iterator itr = m_Particles.begin(); !found && itr != m_Particles.end(); ++itr)
        {
            if (*itr == pMOToCheck)
            {
                found = true;
                break;
            }
        }
        // Try the items just added this frame
        if (!found)
        {
            for (deque<MovableObject *>::iterator itr = m_AddedParticles.begin(); !found && itr != m_AddedParticles.end(); ++itr)
            {
                if (*itr == pMOToCheck)
                {
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
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
            for (deque<Actor *>::iterator itr = m_Actors.begin(); !found && itr != m_Actors.end(); ++itr)
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
                for (deque<Actor *>::iterator itr = m_AddedActors.begin(); !found && itr != m_AddedActors.end(); ++itr)
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
        if (RemoveItem(pMOToRem))
            return true;
        if (RemoveParticle(pMOToRem))
            return true;
        if (RemoveActor(pMOToRem))
            return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          KillAllActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Kills and destroys all actors of a specific team.

int MovableMan::KillAllActors(int exceptTeam)
{
    int killCount = 0;
    AHuman *pHuman = 0;

    // Kill all regular Actors
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        if ((*aIt)->GetTeam() != exceptTeam)
        {
            // Blow up the head of humanoids, for effect
            if (pHuman = dynamic_cast<AHuman *>(*aIt))
                pHuman->GetHead()->GibThis();
            else
                (*aIt)->GibThis();
            killCount++;
        }
    }

    // Kill all Actors added this frame
    for (deque<Actor *>::iterator aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
    {
        if ((*aIt)->GetTeam() != exceptTeam)
        {
            // Blow up the head of humanoids, for effect
            if (pHuman = dynamic_cast<AHuman *>(*aIt))
                pHuman->GetHead()->GibThis();
            else
                (*aIt)->GibThis();
            killCount++;
        }
    }

    return killCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EjectAllActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list ALL Actors in the world and removes them from the
//                  MovableMan. Ownership IS transferred!

int MovableMan::EjectAllActors(list<SceneObject *> &actorList, int onlyTeam, bool noBrains)
{
    int addedCount = 0;

    // Add all regular Actors
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        // Only grab ones of a specific team; delete all others
        if ((onlyTeam == Activity::NoTeam || (*aIt)->GetTeam() == onlyTeam) && (!noBrains || !(*aIt)->HasObjectInGroup("Brains")))
        {
            actorList.push_back((*aIt));
            addedCount++;
        }
        else
            delete *aIt;
    }
    // Clear the internal Actor list; we transferred the ownership of them
    m_Actors.clear();

    // Add all Actors added this frame
    for (deque<Actor *>::iterator aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
    {
        // Only grab ones of a specific team; delete all others
        if ((onlyTeam == Activity::NoTeam || (*aIt)->GetTeam() == onlyTeam) && (!noBrains || !(*aIt)->HasObjectInGroup("Brains")))
        {
            actorList.push_back((*aIt));
            addedCount++;
        }
        else
            delete *aIt;
    }
    // Clear the internal Actor list; we transferred the ownership of them
    m_AddedActors.clear();

    // Also clear the actor rosters
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        m_ActorRoster[team].clear();

    return addedCount;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EjectAllItems
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds to a list ALL Items in the world and removes them from the
//                  MovableMan. Ownership IS transferred!

int MovableMan::EjectAllItems(list<SceneObject *> &itemList)
{
    int addedCount = 0;

    // Add all regular Items
    for (deque<MovableObject *>::iterator iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
    {
        itemList.push_back((*iIt));
        addedCount++;
    }
    // Clear the internal Actor list; we transferred the ownership of them
    m_Items.clear();

    // Add all Items added this frame
    for (deque<MovableObject *>::iterator iIt = m_AddedItems.begin(); iIt != m_AddedItems.end(); ++iIt)
    {
        itemList.push_back((*iIt));
        addedCount++;
    }
    // Clear the internal Item list; we transferred the ownership of them
    m_AddedItems.clear();

    return addedCount;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamMOIDCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns MO count for specified team

int MovableMan::GetTeamMOIDCount(int team) const
{
	if (team > Activity::NoTeam && team < Activity::MaxTeamCount)
		return m_TeamMOIDCount[team];
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenAllDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens all doors and keeps them open until this is called again with false.

void MovableMan::OpenAllDoors(bool open, int team)
{
    ADoor *pDoor = 0;
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        pDoor = dynamic_cast<ADoor *>(*aIt);
        if (pDoor && (team == Activity::NoTeam || pDoor->GetTeam() == team))
        {
            // Update first so the door attachable piece is in the right position and doesn't take out a werid chunk of the terrain
            pDoor->Update();
            if (open)
                pDoor->OpenDoor();
            else
                pDoor->CloseDoor();
            pDoor->SetClosedByDefault(!open);
        }
    }
    // Also check all doors added this frame
    for (deque<Actor *>::iterator aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
    {
        pDoor = dynamic_cast<ADoor *>(*aIt);
        if (pDoor && (team == Activity::NoTeam || pDoor->GetTeam() == team))
        {
            // Update first so the door attachable piece is in the right position and doesn't take out a werid chunk of the terrain
            pDoor->Update();
            if (open)
                pDoor->OpenDoor();
            else
                pDoor->CloseDoor();
            pDoor->SetClosedByDefault(!open);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverrideMaterialDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Temporarily erases any material door representations of a specific team.
//                  Used for making pathfinding work better, allowing teammember to navigate
//                  through friendly bases.

void MovableMan::OverrideMaterialDoors(bool enable, int team)
{
    ADoor *pDoor = 0;
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        pDoor = dynamic_cast<ADoor *>(*aIt);
        if (pDoor && (team == Activity::NoTeam || pDoor->GetTeam() == team))
        {
            // Update first so the door attachable piece is in the right position and doesn't take out a werid chunk of the terrain
            pDoor->Update();
            pDoor->MaterialDrawOverride(enable);
        }
    }
    // Also check all doors added this frame
    for (deque<Actor *>::iterator aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
    {
        pDoor = dynamic_cast<ADoor *>(*aIt);
        if (pDoor && (team == Activity::NoTeam || pDoor->GetTeam() == team))
        {
            // Update first so the door attachable piece is in the right position and doesn't take out a werid chunk of the terrain
            pDoor->Update();
            pDoor->MaterialDrawOverride(enable);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RedrawOverlappingMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces all objects potnetially overlapping a specific MO to re-draw
//                  this MOID representations onto the MOID bitmap.

void MovableMan::RedrawOverlappingMOIDs(MovableObject *pOverlapsThis)
{
    for (deque<Actor *>::iterator aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
    {
        (*aIt)->DrawMOIDIfOverlapping(pOverlapsThis);
    }

    for (deque<MovableObject *>::iterator iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
    {
        (*iIt)->DrawMOIDIfOverlapping(pOverlapsThis);
    }

    for (deque<MovableObject *>::iterator parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
    {
        (*parIt)->DrawMOIDIfOverlapping(pOverlapsThis);
    }
}


void MovableMan::OnPieMenu(Actor * pActor)
{
	deque<Actor *>::iterator aIt;
	deque<MovableObject *>::iterator iIt;
	deque<MovableObject *>::iterator parIt;

	for (aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
	{
		if ((*aIt)->ProvidesPieMenuContext())
			(*aIt)->OnPieMenu(pActor);
	}

	for (iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
	{
		if ((*iIt)->ProvidesPieMenuContext())
			(*iIt)->OnPieMenu(pActor);
	}

	for (parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
	{
		if ((*parIt)->ProvidesPieMenuContext())
			(*parIt)->OnPieMenu(pActor);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this MovableMan. Supposed to be done every frame.

void MovableMan::Update()
{
    // Don't update if paused
    if (g_ActivityMan.GetActivity() && g_ActivityMan.ActivityPaused())
        return;

	m_SimUpdateFrameNumber++;

    // Clear the MO color layer only if this is a drawn update
    if (g_TimerMan.DrawnSimUpdate())
        g_SceneMan.ClearMOColorLayer();

    // If this is the first sim update since a drawn one, then clear the post effects
    if (g_TimerMan.SimUpdatesSinceDrawn() == 0)
		g_PostProcessMan.ClearScenePostEffects();

    // Reset the draw HUD roster line settings
    m_SortTeamRoster[Activity::TeamOne] = false;
    m_SortTeamRoster[Activity::TeamTwo] = false;
    m_SortTeamRoster[Activity::TeamThree] = false;
    m_SortTeamRoster[Activity::TeamFour] = false;
    // Clear out MO finding optimization buffer - will be added to each frame as thigns are searched for as curently exisitng in the manager
    m_ValiditySearchResults.clear();

    // Move all last frame's alarm events into the proper buffer, and clear out the new one to fill up with this frame's
    m_AlarmEvents.clear();
    for (list<AlarmEvent>::iterator aeItr = m_AddedAlarmEvents.begin(); aeItr != m_AddedAlarmEvents.end(); ++aeItr)
        m_AlarmEvents.push_back(*aeItr);
    m_AddedAlarmEvents.clear();

    // Pre-lock Scene for all the accesses to its bitmaps
    g_SceneMan.LockScene();

    // Will use some common iterators
    deque<Actor *>::iterator aIt;
    deque<Actor *>::iterator amidIt;
    deque<MovableObject *>::iterator iIt;
    deque<MovableObject *>::iterator imidIt;
    deque<MovableObject *>::iterator parIt;
    deque<MovableObject *>::iterator midIt;

    ////////////////////////////////////////////////////////////////////////////
    // First Pass

    {
        // Travel Actors
		g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsTravel);
        {
            for (aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
            {
                if (!((*aIt)->IsUpdated()))
                {
                    (*aIt)->ApplyForces();
                    (*aIt)->PreTravel();
        /*
                    if (aIt == m_Actors.begin())
                    {
                        PALETTE palette;
                        get_palette(palette);
                        save_bmp("poop.bmp", g_SceneMan.GetMOIDBitmap(), palette);
                    }
        */
                    (*aIt)->Travel();
                    (*aIt)->PostTravel();
                }
                (*aIt)->NewFrame();
            }
        }
		g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsTravel);

        // Travel items
        {
            for (iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt)
            {
                if (!((*iIt)->IsUpdated()))
                {
                    (*iIt)->ApplyForces();
                    (*iIt)->PreTravel();
                    (*iIt)->Travel();
                    (*iIt)->PostTravel();
                }
                (*iIt)->NewFrame();
            }
        }

        // Travel particles
		g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ParticlesTravel);
        {
            for (parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
            {
                if (!((*parIt)->IsUpdated()))
                {
                    (*parIt)->ApplyForces();
                    (*parIt)->PreTravel();
                    (*parIt)->Travel();
                    (*parIt)->PostTravel();
                }
                (*parIt)->NewFrame();
            }
        }
		g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ParticlesTravel);

        g_SceneMan.UnlockScene();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Second Pass

    {
        g_SceneMan.LockScene();

        // Actors
		g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ActorsUpdate);
        {
            for (aIt = m_Actors.begin(); aIt != m_Actors.end(); ++aIt)
            {
				(*aIt)->Update();
                (*aIt)->UpdateScripts();
                (*aIt)->ApplyImpulses();
            }
        }
		g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ActorsUpdate);

        // Items
        {
            int count = 0;
            int itemLimit = m_Items.size() - m_MaxDroppedItems;
            for (iIt = m_Items.begin(); iIt != m_Items.end(); ++iIt, ++count)
            {
                (*iIt)->Update();
                (*iIt)->UpdateScripts();
                (*iIt)->ApplyImpulses();
                if (count <= itemLimit)
                {
                    (*iIt)->SetToSettle(true);
                }
            }
        }

        // Particles
		g_PerformanceMan.StartPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
        {
            for (parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
            {
                (*parIt)->Update();
                (*parIt)->UpdateScripts();
                (*parIt)->ApplyImpulses();
                (*parIt)->RestDetection();
                // Copy particles that are at rest to the terrain and mark them for deletion.
                if ((*parIt)->IsAtRest())
                {
                    // Mark for settling after update loop.
                    (*parIt)->SetToSettle(true);
                }
            }
        }
		g_PerformanceMan.StopPerformanceMeasurement(PerformanceMan::ParticlesUpdate);
    }

    ///////////////////////////////////////////////////
    // Clear the MOID layer before starting to delete stuff which may be in the MOIDIndex

    g_SceneMan.ClearAllMOIDDrawings();
//    g_SceneMan.MOIDClearCheck();

    ///////////////////////////////////////////////////
    // Determine whether we should go into a brief period of slo-mo for when the sim gets hit heavily all of a sudden

    if (m_AddedActors.size() + m_AddedActors.size() + m_AddedParticles.size() > m_SloMoThreshold)
    {
        m_SloMoTimer.SetSimTimeLimitMS(m_SloMoDuration);
        m_SloMoTimer.Reset();
    }
    g_TimerMan.SetOneSimUpdatePerFrame(!m_SloMoTimer.IsPastSimTimeLimit());

    //////////////////////////////////////////////////////////////////////
    // TRANSFER ALL MOs ADDED THIS FRAME
    // All Actors, Items, and Particles added this frame now are officially added

    {
        // Actors
        for (aIt = m_AddedActors.begin(); aIt != m_AddedActors.end(); ++aIt)
        {
            // Delete instead if it's marked for it
            if (!(*aIt)->IsSetToDelete())
                m_Actors.push_back(*aIt);
            else
			{
				// Also remove actor from the roster
				if ((*aIt)->GetTeam() >= 0)
					//m_ActorRoster[(*aIt)->GetTeam()].remove(*aIt);
					RemoveActorFromTeamRoster(*aIt);
                delete (*aIt);
			}
        }
        m_AddedActors.clear();

        // Items
        for (iIt = m_AddedItems.begin(); iIt != m_AddedItems.end(); ++iIt)
        {
            // Delete instead if it's marked for it
            if (!(*iIt)->IsSetToDelete())
                m_Items.push_back(*iIt);
            else
                delete (*iIt);
        }
        m_AddedItems.clear();

        // Particles
        for (parIt = m_AddedParticles.begin(); parIt != m_AddedParticles.end(); ++parIt)
        {
            // Delete instead if it's marked for it
            if (!(*parIt)->IsSetToDelete())
                m_Particles.push_back(*parIt);
            else
                delete (*parIt);
        }
        m_AddedParticles.clear();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Copy (Settle) Pass

    {
        g_SceneMan.UnlockScene();
        acquire_bitmap(g_SceneMan.GetTerrain()->GetMaterialBitmap());

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
                // Remove from the team roster

                if ((*aIt)->GetTeam() >= 0)
                    //m_ActorRoster[(*aIt)->GetTeam()].remove(*aIt);
					RemoveActorFromTeamRoster(*aIt);
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
				if ((*iIt)->GetRestThreshold()< 0)
					(*iIt)->SetRestThreshold(500);
                m_Particles.push_back(*(iIt++));
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
			// Set brain to 0 to avoid crasehs due to brain deletion
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
            delete *aIt;
            aIt++;
        }
        // Try to set the existing iterator to a safer value, erase can crash in debug mode otherwise?
        aIt = m_Actors.begin();
        m_Actors.erase(amidIt, m_Actors.end());

        // Items
        iIt = stable_partition(m_Items.begin(), m_Items.end(), std::not_fn(std::mem_fn(&MovableObject::ToDelete)));
        imidIt = iIt;

        while (iIt != m_Items.end())
            delete *(iIt++);
        m_Items.erase(imidIt, m_Items.end());

        // Particles
        parIt = partition(m_Particles.begin(), m_Particles.end(), std::not_fn(std::mem_fn(&MovableObject::ToDelete)));
        midIt = parIt;

        while (parIt != m_Particles.end())
            delete *(parIt++);
        m_Particles.erase(midIt, m_Particles.end());
    }

    // SETTLE PARTICLES //////////////////////////////////////////////////
    // Only settle after all updates and deletions are done
    if (m_SettlingEnabled)
    {
        parIt = partition(m_Particles.begin(), m_Particles.end(), std::not_fn(std::mem_fn(&MovableObject::ToSettle)));
        midIt = parIt;

        while (parIt != m_Particles.end())
        {
            Vector parPos((*parIt)->GetPos().GetFloored());
            Material const * terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(parPos.m_X, parPos.m_Y));
            if ((*parIt)->GetDrawPriority() >= terrMat->GetPriority())
            {
                // Gold particle special case to avoid compacting of gold
                if ((*parIt)->GetMaterial()->GetIndex() == c_GoldMaterialID)
                {
                    for (int s = 0; terrMat->GetIndex() == c_GoldMaterialID; ++s)
                    {
                        if (s % 2 == 0)
                            parPos.m_Y -= 1.0;
                        else
                            parPos.m_X += (RandomNum() >= 0.5F ? 1.0F : -1.0F);
                        terrMat = g_SceneMan.GetMaterialFromID(g_SceneMan.GetTerrain()->GetMaterialPixel(parPos.m_X, parPos.m_Y));
                    }
                    (*parIt)->SetPos(parPos);
                }

//                (*parIt)->Draw(g_SceneMan.GetTerrain()->GetFGColorBitmap(), Vector(), g_DrawColor, true);
//                (*parIt)->Draw(g_SceneMan.GetTerrain()->GetMaterialBitmap(), Vector(), g_DrawMaterial, true);
                g_SceneMan.GetTerrain()->ApplyMovableObject(*parIt);
            }
            delete *(parIt++);
        }
        m_Particles.erase(midIt, m_Particles.end());
    }

    release_bitmap(g_SceneMan.GetTerrain()->GetMaterialBitmap());

    ////////////////////////////////////////////////////////////////////////
    // Draw the MO matter and IDs to their layers for next frame

// Not anymore, we're using ClearAllMOIDDrawings instead.. much more efficient
//    g_SceneMan.ClearMOIDLayer();
    UpdateDrawMOIDs(g_SceneMan.GetMOIDBitmap());

	// COUNT MOID USAGE PER TEAM  //////////////////////////////////////////////////
	{
		int team = Activity::NoTeam;

		for (team = Activity::TeamOne; team < Activity::MaxTeamCount; team++)
			m_TeamMOIDCount[team] = 0;
		
		for (vector<MovableObject *>::iterator itr = m_MOIDIndex.begin(); itr != m_MOIDIndex.end(); ++itr)
		{
			if (*itr)
			{
				team = (*itr)->GetTeam();

				if (team > Activity::NoTeam && team < Activity::MaxTeamCount)
					m_TeamMOIDCount[team]++;
			}
		}
	}


    ////////////////////////////////////////////////////////////////////
    // Draw the MO colors ONLY if this is a drawn update!

    if (g_TimerMan.DrawnSimUpdate())
        Draw(g_SceneMan.GetMOColorBitmap());

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
// Method:          DrawMatter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's all MO's current material representations to a
//                  BITMAP of choice.

void MovableMan::DrawMatter(BITMAP *pTargetBitmap, Vector &targetPos)
{
    // Draw objects to accumulation bitmap
    for (deque<Actor *>::iterator aIt = --m_Actors.end(); aIt != --m_Actors.begin(); --aIt)
        (*aIt)->Draw(pTargetBitmap, targetPos, g_DrawMaterial);

    for (deque<MovableObject *>::iterator parIt = --m_Particles.end(); parIt != --m_Particles.begin(); --parIt)
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
	for (vector<MovableObject *>::iterator aIt = m_MOIDIndex.begin(); aIt != m_MOIDIndex.end(); ++aIt)
	{
		if (*aIt)
		{
			RTEAssert((*aIt)->GetID() == g_NoMOID || (*aIt)->GetID() == count, "MOIDIndex broken!");
			RTEAssert((*aIt)->GetRootID() == g_NoMOID || ((*aIt)->GetRootID() >= 0 && (*aIt)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
		}
		count++;
		if (count == g_NoMOID) count++;
	}


	for (deque<MovableObject *>::iterator itr = m_Items.begin(); itr != m_Items.end(); ++itr)
	{
		RTEAssert((*itr)->GetID() == g_NoMOID || (*itr)->GetID() < GetMOIDCount(), "MOIDIndex broken!");
		RTEAssert((*itr)->GetRootID() == g_NoMOID || ((*itr)->GetRootID() >= 0 && (*itr)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
	}
	// Try the items just added this frame
	for (deque<MovableObject *>::iterator itr = m_AddedItems.begin(); itr != m_AddedItems.end(); ++itr)
	{
		RTEAssert((*itr)->GetID() == g_NoMOID || (*itr)->GetID() < GetMOIDCount(), "MOIDIndex broken!");
		RTEAssert((*itr)->GetRootID() == g_NoMOID || ((*itr)->GetRootID() >= 0 && (*itr)->GetRootID() < g_MovableMan.GetMOIDCount()), "MOIDIndex broken!");
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdateDrawMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the MOIDs of all current MOs and draws their ID's to a BITMAP
//                  of choice.

void MovableMan::UpdateDrawMOIDs(BITMAP *pTargetBitmap)
{
    int aCount = m_Actors.size();
    int iCount = m_Items.size();
    int parCount = m_Particles.size();

    // Clear the index each frame and do it over because MO's get added and
    // deleted between each frame.
    m_MOIDIndex.clear();
    // Add a null and start counter at 1 because MOID == 0 means no MO.
    // - Update: This isnt' true anymore, but still keep 0 free just to be safe
    m_MOIDIndex.push_back(0);

    MOID currentMOID = 1;
    int i = 0;

    for (i = 0; i < aCount; ++i) {
		if (m_Actors[i]->GetsHitByMOs() && !m_Actors[i]->IsSetToDelete())
        {
			Vector notUsed;
            m_Actors[i]->UpdateMOID(m_MOIDIndex);
            m_Actors[i]->Draw(pTargetBitmap, notUsed, g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        }
        else
            m_Actors[i]->SetAsNoID();
    }
    for (i = 0; i < iCount; ++i)
    {
        if (m_Items[i]->GetsHitByMOs() && !m_Items[i]->IsSetToDelete())
        {
            m_Items[i]->UpdateMOID(m_MOIDIndex);
            m_Items[i]->Draw(pTargetBitmap, Vector(), g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        }
        else
            m_Items[i]->SetAsNoID();
    }
    for (i = 0; i < parCount; ++i)
    {
        if (m_Particles[i]->GetsHitByMOs() && !m_Particles[i]->IsSetToDelete())
        {
            m_Particles[i]->UpdateMOID(m_MOIDIndex);
            m_Particles[i]->Draw(pTargetBitmap, Vector(), g_DrawMOID, true);
            currentMOID = m_MOIDIndex.size();
        }
        else
            m_Particles[i]->SetAsNoID();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's current graphical representation to a
//                  BITMAP of choice.

void MovableMan::Draw(BITMAP *pTargetBitmap, const Vector &targetPos)
{
    // Draw objects to accumulation bitmap, in reverse order so actors appear on top.
    for (deque<MovableObject *>::iterator parIt = m_Particles.begin(); parIt != m_Particles.end(); ++parIt)
        (*parIt)->Draw(pTargetBitmap, targetPos);

	for (deque<MovableObject *>::reverse_iterator itmIt = m_Items.rbegin(); itmIt != m_Items.rend(); ++itmIt)
        (*itmIt)->Draw(pTargetBitmap, targetPos);

    for (deque<Actor *>::reverse_iterator aIt = m_Actors.rbegin(); aIt != m_Actors.rend(); ++aIt)
        (*aIt)->Draw(pTargetBitmap, targetPos);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MovableMan's current graphical representation to a
//                  BITMAP of choice.

void MovableMan::DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos, int which, bool playerControlled)
{
    // Draw HUD elements
	for (deque<MovableObject *>::reverse_iterator itmIt = m_Items.rbegin(); itmIt != m_Items.rend(); ++itmIt)
        (*itmIt)->DrawHUD(pTargetBitmap, targetPos, which);

    for (deque<Actor *>::reverse_iterator aIt = m_Actors.rbegin(); aIt != m_Actors.rend(); ++aIt)
        (*aIt)->DrawHUD(pTargetBitmap, targetPos, which);
}

} // namespace RTE
