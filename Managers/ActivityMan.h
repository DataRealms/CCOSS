#ifndef _RTEACTIVITYMAN_
#define _RTEACTIVITYMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ActivityMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <string>

#include "DDTTools.h"
#include "Singleton.h"
#define g_ActivityMan ActivityMan::Instance()

#include <deque>

#include "Sound.h"

#include "SceneObject.h"
#include "Controller.h"
#include "Icon.h"

namespace RTE
{

#define FRIENDLYCURSOR 0
#define ENEMYCURSOR 1
#define LZCURSORFRAMECOUNT 4
#define OBJARROWFRAMECOUNT 4

class Scene;
class Actor;
class ACraft;
class BuyMenuGUI;
class SceneEditorGUI;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Activity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Base class for all Activity:s, including game modes and editors.
// Parent(s):       Entity.
// Class history:   8/5/2007 Activity created.

class Activity:
    public Entity
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

    enum ActivityState
    {
        NOACTIVITY = -1,
        NOTSTARTED = 0,
        STARTING,
        EDITING,
        PREGAME,
        RUNNING,
        DEMOEND,
        INERROR,
        OVER
    };

    enum Player
    {
        NOPLAYER = -1,
        PLAYER_1 = 0,
        PLAYER_2,
        PLAYER_3,
        PLAYER_4,
        MAXPLAYERCOUNT
    };

    enum Team
    {
        NOTEAM = -1,
        TEAM_1 = 0,
        TEAM_2,
        TEAM_3,
        TEAM_4,
        MAXTEAMCOUNT
    };

    enum ViewState
    {
        NORMAL = 0,
        OBSERVE,
        DEATHWATCH,
        ACTORSELECT,
        AISENTRYPOINT,
        AIPATROLPOINTS,
        AIGOLDDIGPOINT,
        AIGOTOPOINT,
        LZSELECT,
		UNITSELECTCIRCLE,
    };

    enum DifficultySetting
    {
        MINDIFFICULTY = 0,
        CAKEDIFFICULTY = 15,
        EASYDIFFICULTY = 40,
        MEDIUMDIFFICULTY = 60,
        HARDDIFFICULTY = 85,
        NUTSDIFFICULTY = 98,
        MAXDIFFICULTY = 100,
        TESTDIFFICULTY = -1
    };

	enum AISkillSetting
	{
		INFERIORSKILL = 35,
		AVERAGESKILL = 70,
		GOODSKILL = 99,
		UNFAIRSKILL = 100,
		DEFAULTSKILL = 50
	};

/* abstract class
    // Concrete allocation and cloning definitions
    ENTITYALLOCATION(Activity)
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Activity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Activity object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Activity() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Activity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Activity object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~Activity() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Activity object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Activity to be identical to another, by deep copy.
// Arguments:       A reference to the Activity to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const Activity &reference);


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
// Description:     Resets the entire Activity, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Activity to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the Activity will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Activity object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDescription
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the user-friendly description of this.
// Arguments:       None.
// Return value:    A string with the user-friendly description of this.

    std::string GetDescription() const { return m_Description; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetInCampaignStage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets in which stage of the Campaign this appears.
// Arguments:       The new stage to set. < 0 means it doesn't appear in the campaign.
// Return value:    None.

    void SetInCampaignStage(int newStage) { m_InCampaignStage = newStage; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetInCampaignStage
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows in which stage of the Campaign this appears.
// Arguments:       None.
// Return value:    An int with the stage number. < 0 means it's not in the campaign

    int GetInCampaignStage() const { return m_InCampaignStage; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetActivityState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Overrides the current game state. Should not be used much, use dedicted
//                  state setting funcitons instead.
// Arguments:       The new state to set.
// Return value:    None.

    void SetActivityState(int newState) { m_ActivityState = newState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivityState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current activity state code. See the ActivityState enumeration for
//                  the legend.
// Arguments:       None.
// Return value:    An int with the game state code.

    int GetActivityState() const { return m_ActivityState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the current scene.
// Arguments:       None.
// Return value:    A string with the instance name of the scene.

    std::string GetSceneName() const { return m_SceneName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSceneName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of the scene this is associated with.
// Arguments:       The new name of the scene to load next game.
// Return value:    None.

    void SetSceneName(const std::string sceneName) { m_SceneName = sceneName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearPlayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all players as not active in the current game.
// Arguments:       Whether to reset the team funds as well.
// Return value:    None.

    void ClearPlayers(bool resetFunds = true);

/* This needs to reset m_TeamFundsShare and m_FundsContribution
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetAllFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets all teams' funds to be 0.
// Arguments:       None.
// Return value:    None.

    void ResetAllFunds() { for (int team = TEAM_1; team < MAXTEAMCOUNT; ++team) { m_TeamFunds[team] = 0; } }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets up a specific player for this activity, AI or Human
// Arguments:       Which player slot to set up - PLAYER_1 to PLAYER_4
//                  Whether this player is Human.
//                  Which Team this player belongs to.
//                  How many funds this player contributes to its Team's total funds
//                  The team flag icon of this player - OWNERSHIP IS NOT TRANSFERRED.
// Return value:    The new total number of active players in the current game.

    int AddPlayer(int player, bool isHuman, int team, int funds, const Icon *pTeamIcon = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlayerFundsContribution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new amount of starting funds for a player, after he has already
//                  been added.
// Arguments:       Which player slot to update - has to already be active.
//                  Updated value of how many funds this player contributes to its Team's
//                  total funds.
// Return value:    Whether the update was successful.

    bool UpdatePlayerFundsContribution(int player, int newFunds);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of active players in the current game, AI or Human
// Arguments:       None.
// Return value:    The total number of players in the current game.

    int GetPlayerCount() const { return m_PlayerCount; }

// Not relevant anymore; this gets calculated automatically
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPlayerCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the total number of players the next game will have.
// Arguments:       The total number of players for the next game.
// Return value:    None.

    void SetPlayerCount(int playerCount) { /*m_PlayerCount = playerCount*/; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DeactivatePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Turns off a player if they were active. Should only be done if brain
//                  etc are already taken care of and disposed of properly. Will also
//                  deactivate the team this player is on, if there's no other active
//                  players still on it.
// Arguments:       Which player index to deactivate.
// Return value:    Whether the player was active before trying to deactivate.

    bool DeactivatePlayer(int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a specific player is active in the current game.
// Arguments:       Which player index to check.
// Return value:    Whether the player is active in the current activity.

    bool PlayerActive(int player) const { return m_IsActive[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a specific player is human in the current game, ie
//                  not an AI player and has a screen etc.
// Arguments:       Which player index to check.
// Return value:    Whether the player is active as a Human in the current activity.

    bool PlayerHuman(int player) const { return m_IsHuman[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHumanCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of human players in the current game
// Arguments:       None.
// Return value:    The total number of players in the current game.

    int GetHumanCount() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaxPlayerSupport
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the max number of players supported by this Activity.
// Arguments:       None.
// Return value:    The max number of players supported by this activity.

    int GetMaxPlayerSupport() const { return m_MaxPlayerSupport; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMinTeamsRequired
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the minimum number of teams with players that this Activity
//                  requires.
// Arguments:       None.
// Return value:    The min number of Teams this requires to run.

    int GetMinTeamsRequired() const { return m_MinTeamsRequired; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of teams in the current game
// Arguments:       None.
// Return value:    The total number of teams in the current game.

    int GetTeamCount() const { return m_TeamCount; }

// Not relevant anymore; this gets calculated automatically
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the total number of teams in the current game
// Arguments:       The total number of teams the next game will have game.
// Return value:    None.

    void SetTeamCount(int teamCount) { /*m_TeamCount = teamCount*/; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TeamActive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a specific team is active in the current game.
// Arguments:       Which team index to check.
// Return value:    Whether the team is active in the current activity.

    bool TeamActive(int team) const { return (team >= TEAM_1 && team < MAXTEAMCOUNT) ? m_TeamActive[team] : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current team a specific player belongs to.
// Arguments:       The player to get the team info on.
// Return value:    An int with the team number.

    int GetTeamOfPlayer(int player) const { return m_Team[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current team a specific player belongs to.
// Arguments:       The player to set the team info on, and the team.
// Return value:    None.

    void SetTeamOfPlayer(int player, int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayersInTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current number of players in a specific team.
// Arguments:       Which team to get the player count for.
// Return value:    An int with the player count.

    int PlayersInTeamCount(int team) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ScreenOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts a player index into a screen index, and only if that player
//                  is human.
// Arguments:       Which player index to convert.
// Return value:    An int with the screen index, or -1 if nonhuman player or no players.

    int ScreenOfPlayer(int player) const { return (player >= PLAYER_1 && player < MAXPLAYERCOUNT) ? m_PlayerScreen[player] : -1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerOfScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts a screen index into a player index, if that screen exists.
// Arguments:       Which screen index to convert.
// Return value:    An int with the player index, or -1 if that screen is not in use;

    int PlayerOfScreen(int screen) const { for (int p = PLAYER_1; p < MAXPLAYERCOUNT; ++p) { if (m_PlayerScreen[p] == screen) return p; } return NOPLAYER; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetViewState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current viewing state for a specific player. See the
//                  ViewState enumeration for the legend.
// Arguments:       Which player to get the view state for.
// Return value:    None.

    int GetViewState(int whichPlayer = 0) const { return m_ViewState[whichPlayer]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetViewState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current viewing state for a specific player. See the
//                  ViewState enumeration for the legend.
// Arguments:       The state you want to set to.
//                  Which player to get the view state for.
// Return value:    None.

    void SetViewState(int whichViewState, int whichPlayer = 0) { m_ViewState[whichPlayer] = whichViewState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPlayerBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current Brain actor for a specific player.
// Arguments:       A pointer to the new brain Actor. Ownership is NOT transferred!
//                  Which team to set the brain actor for.
// Return value:    None.

    void SetPlayerBrain(Actor *pNewBrain, int player = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current Brain actor for a specific player.
// Arguments:       Which player to get the brain actor for.
// Return value:    A pointer to the Brain Actor. Ownership is NOT transferred!

	Actor * GetPlayerBrain(int player = 0) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_pBrain[player]; else return 0;  }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPlayerHadBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets to indicate that the player had a brian at some point. This is
//                  to simulate that in automated battle cases.
// Arguments:       Which player to set whether he had a brain or not.
//                  Whether he should be flagged as having had a brain.
// Return value:    None.

    void SetPlayerHadBrain(int player, bool hadBrain = true) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) m_HadBrain[player] = hadBrain; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlayerHadBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific player ever had a brain yet.
// Arguments:       Which player to check whether they ever had a brain.
// Return value:    Whether this player ever had a brain.

	bool PlayerHadBrain(int player = 0) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_HadBrain[player]; else return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBrainEvacuated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether a player's brain was evacuated during the Activity.
// Arguments:       Which player to check whether their brain was evac'd.
//                  Whether it was evacuated yet.
// Return value:    None.

	void SetBrainEvacuated(int player = 0, bool evacuated = true) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) m_BrainEvacuated[player] = evacuated; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          BrainWasEvacuated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a specific player's brain was evacuated into orbit so far.
// Arguments:       Which player to check whether their brain was evac'd.
// Return value:    Whether this player had a brain that was evacuated.

	bool BrainWasEvacuated(int player = 0) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_BrainEvacuated[player]; else return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnyEvacuees
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether ANY player evacuated their brain.
// Arguments:       None.
// Return value:    Whether any player evac'd their brain yet.

    bool AnyEvacuees() { for (int i = 0; i < MAXPLAYERCOUNT; ++i) { if (m_BrainEvacuated[i]) return true; } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAssignedBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the passed in actor is the brain of any player.
// Arguments:       Which Actor to check for player brainedness.
// Return value:    Whetehr any player's brain or not.

    bool IsAssignedBrain(Actor *pActor) { for (int i = 0; i < MAXPLAYERCOUNT; ++i) { if (pActor == m_pBrain[i]) return true; } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBrainOfWhichPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows which player has a specific actor as a brain, if any.
// Arguments:       Which Actor to check for player brainedness.
// Return value:    Which player has this assigned as a brain, if any.

    int IsBrainOfWhichPlayer(Actor *pActor) { for (int i = 0; i < MAXPLAYERCOUNT; ++i) { if (pActor == m_pBrain[i]) return i; } return NOPLAYER; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsOtherPlayerBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the passed in actor is the brain of any other player.
// Arguments:       Which Actor to check for other player brainedness.
//                  From which player's perspective to make the query.
// Return value:    Whetehr other player's brain or not.

    bool IsOtherPlayerBrain(Actor *pActor, int player) { for (int i = 0; i < MAXPLAYERCOUNT; ++i) { if (m_IsActive[i] &&  i != player && pActor == m_pBrain[i]) return true; } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HumanBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many human-controlled brains are left in this Activity.
// Arguments:       None.
// Return value:    How many brains are left as human-controlled in this Activity.

    int HumanBrainCount();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AIBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many A.I.-controlled brains are left in this Activity.
// Arguments:       None.
// Return value:    How many brains are left on as A.I.-controlled in this Activity.

    int AIBrainCount();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlledActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently controlled actor of a specific player.
// Arguments:       Which player to get the controlled actor of.
// Return value:    A pointer to the controlled Actor. Ownership is NOT transferred! 0 If
//                  no actor is currently controlled by this player.

	Actor * GetControlledActor(int player = 0) { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_pControlledActor[player]; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of funds a specific team currently has in the game.
// Arguments:       Which team to set the fund count for. 0 = first team.
// Return value:    A float with the funds tally for the requested team.

    void SetTeamFunds(float newFunds, unsigned int which = 0) { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) m_TeamFunds[which] = newFunds; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of funds a specific team currently has in the game.
// Arguments:       Which team to get the fund count from. 0 = first team.
// Return value:    A float with the funds tally for the requested team.

	float GetTeamFunds(unsigned int which = 0) const { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) return m_TeamFunds[which]; else return 0; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeTeamFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes a team's funds level by a certain amount.
// Arguments:       The amount with which to change the funds balance.
//                  Which team to alter the funds of. 0 = first team.
// Return value:    None.

    void ChangeTeamFunds(float howMuch, unsigned int which = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TeamFundsChanged
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whetehr the team funds changed since last time this was called.
//                  This also resets the state, so calling this again on the same team will
//                  ield false unless it's been changed again.
// Arguments:       Which team's funds to check.
// Return value:    Whether funds amount changed for this team since last time this was
//                  called.

    bool TeamFundsChanged(unsigned int which = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerFundsContribution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of funds a specific player originally added to his
//                  team's collective stash.
// Arguments:       Which player to inquire about.
// Return value:    A float with the funds originally deposited by this player.

	float GetPlayerFundsContribution(int player) const { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_FundsContribution[player]; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerFundsShare
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the share of funds a specific PLAYER currently has in the game,
//                  calculated from his original contribution to his team's collective funds.
// Arguments:       Which player to get the fund count from.
// Return value:    A float with the funds tally for the requested player.

	float GetPlayerFundsShare(int player = 0) const { if (player >= Activity::PLAYER_1 && player < Activity::MAXPLAYERCOUNT) return m_FundsContribution[player] > 0 ? (m_TeamFunds[m_Team[player]] * m_TeamFundsShare[player]) : 0; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of a specific team.
// Arguments:       Which team to set the name of. 0 = first team.
//                  The name to set it to.
// Return value:    None.

	void SetTeamName(unsigned int which, const std::string &newName) { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) m_TeamNames[which] = newName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of a specific team.
// Arguments:       Which team to get the name of. 0 = first team.
// Return value:    The current name of that team.

	std::string GetTeamName(unsigned int which = 0) const { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) return (which >= TEAM_1 && which < MAXTEAMCOUNT && m_TeamActive[which]) ? m_TeamNames[which] : "Inactive Team"; else return ""; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Icon of a specific team.
// Arguments:       Which team to set the Icon of. 0 = first team.
//                  The Icon to set it to.
// Return value:    None.

	void SetTeamIcon(unsigned int which, const Icon &newIcon) { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) m_TeamIcons[which] = newIcon; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Icon of a specific team.
// Arguments:       Which team to get the Icon of. 0 = first team.
// Return value:    The current Icon of that team.

	const Icon * GetTeamIcon(unsigned int which = 0) const { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) return (which >= TEAM_1 && which < MAXTEAMCOUNT) ? &(m_TeamIcons[which]) : 0; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ReportDeath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Increments the tally of a death of an actor on a specific team.
// Arguments:       Which team to increase the death count of. 0 = first team.
// Return value:    The new death count.

	int ReportDeath(unsigned int which = 0, int howMany = 1) { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) return m_TeamDeaths[which] += howMany; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamDeathCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of deaths on a specific team so far on the current game.
// Arguments:       Which team to get the death tally of. 0 = first team.
// Return value:    The current death count.

	int GetTeamDeathCount(unsigned int which = 0) { if (which >= Activity::TEAM_1 && which < Activity::MAXTEAMCOUNT) return m_TeamDeaths[which]; else return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetDemoTimeLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many seconds of demo time is left, if indeed in demo mode.
// Arguments:       None.
// Return value:    An int wwith how many demo seconds are left, or < 0 if not applicable.

    virtual long GetDemoTimeLeft() { return -1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Running
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently running or not (not editing,
//                  over or paused)
// Arguments:       None.
// Return value:    Whether the game is running or not.

    bool Running() const { return (m_ActivityState == RUNNING || m_ActivityState == EDITING) && !m_Paused; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Paused
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently paused or not.
// Arguments:       None.
// Return value:    Whether the game is paused or not.

    bool Paused() const { return m_Paused; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityOver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is over or not.
// Arguments:       None.
// Return value:    Whether the game is over or not.

    bool ActivityOver() const { return m_ActivityState == OVER; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SceneIsCompatible
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells if a particular Scene supports this specific Activity on it.
//                  Usually that means certain Area:s need to be defined in the Scene.
// Arguments:       The Scene to check if it supports this Activiy. Ownership IS NOT TRANSFERRED!
//                  How many teams we're checking for. Some scenes may support and activity
//                  but only for a limited number of teams. If -1, not applicable.
// Return value:    Whether the Scene has the right stuff.

    virtual bool SceneIsCompatible(Scene *pScene, int teams = -1) { return pScene && teams <= m_MinTeamsRequired; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  EnteredOrbit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates an Actor as having left the game scene and entered orbit.
//                  OWNERSHIP IS NOT transferred, as the Actor's inventory is just 'unloaded'.
// Arguments:       The actor instance. Ownership IS NOT TRANSFERRED!
// Return value:    None.

    virtual void EnteredOrbit(Actor *pActor);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReassignSquadLeader
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the player's ControlledActor the leader of any squad it is a
//                  momber of.
// Arguments:       Player and team numbers.
// Return value:    None.

    virtual void ReassignSquadLeader(const int player, const int team);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the ActivityMan to focus player control to a specific Actor for a
//                  specific team. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which Actor to switch focus to. The team of this Actor will be set
//                  once it is passed in. Ownership IS NOT TRANSFERRED! The Actor should
//                  be added to MovableMan already.
// Return value:    Whether the focus switch was successful or not.

    virtual bool SwitchToActor(Actor *pActor, int player = 0, int team = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToNextActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the ActivityMan to focus player control to the next Actor of a
//                  specific team, other than the current one focused on.
// Arguments:       Which team to switch to next actor on.
//                  An actor pointer to skip in the sequence.
// Return value:    None.

    virtual void SwitchToNextActor(int player, int team, Actor *pSkip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToPrevActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the ActivityMan to focus player control to the previous Actor of a
//                  specific team, other than the current one focused on.
// Arguments:       Which team to switch to next actor on.
//                  An actor pointer to skip in the sequence.
// Return value:    None.

    virtual void SwitchToPrevActor(int player, int team, Actor *pSkip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetMessageTimer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the message timer for one player
// Arguments:       The player to rese tthe message timer for.
// Return value:    None.

    void ResetMessageTimer(int player = 0) { if (player >= 0 && player < MAXPLAYERCOUNT) m_MsgTimer[player].Reset(); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CurrentActorControlled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the currently controlled actor to temporarily
// Arguments:       Which team to switch to next actor on.
//                  An actor pointer to skip in the sequence.
// Return value:    None.

    virtual void SwitchToPrevActor(int player, int team, Actor *pSkip = 0);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDifficulty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current difficulty setting
// Arguments:       The new setting.
// Return value:    None.

    void SetDifficulty(int difficulty);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDifficulty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current difficulty setting
// Arguments:       None.
// Return value:    The current setting.

    int GetDifficulty() const { return m_Difficulty; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDifficultyString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns string representation of a given difficulty value
// Arguments:       Difficulty setting
// Return value:    Corresponding difficulty string

	static std::string GetDifficultyString(int difficulty);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamAISkill
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets AI skill level for specified team
// Arguments:		AI skill level, 1-100
// Return value:    None

	void SetTeamAISkill(int team, int skill);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamAISkill
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Returns skil level for specified team. If team is < 0 or > 3 an average of all teams is returned.
// Arguments:       Team to get skill level for
// Return value:    Team skill level

    int GetTeamAISkill(int team) const ;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAISkillString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns string representation of a given AI skill value
// Arguments:       AI skill setting
// Return value:    Corresponding AI skill string

	static std::string GetAISkillString(int skill);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsPlayerTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a team is player controlled or not.
// Arguments:       The team nuber to check.
// Return value:    Whether  team is player controlled or not.

    bool IsPlayerTeam(int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetupPlayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Precalculates the player-to-screen index map, counts the number of
//                  active players etc.
// Arguments:       None.
// Return value:    None.

    virtual void SetupPlayers();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts this. Creates all the data etc necessary to start
//                  the activity.
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
// Description:     Forces this activity's game end.
// Arguments:       None.
// Return value:    None.

    virtual void End();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
// Arguments:       A pointer to a screen-sized BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which screen's GUI to draw onto the bitmap.
// Return value:    None.

    virtual void DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int which = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActivityMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCraftsOrbitAtTheEdge
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether crafts must be considered orbited if they reach the map border on non-wrapped maps
// Arguments:       Flag value
// Return value:    None

	void SetCraftsOrbitAtTheEdge(bool value) { m_CraftsOrbitAtTheEdge = value; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCraftsOrbitAtTheEdge
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether crafts must be considered orbited if they reach the map border on non-wrapped maps
// Arguments:       None
// Return value:    Flag value

	bool GetCraftsOrbitAtTheEdge() const { return m_CraftsOrbitAtTheEdge; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


    static Entity::ClassInfo m_sClass;
    // User-friendly description of what this activity is all about
    string m_Description;
    // Which stage of the campaign that this appears in, if any (-1 means it's not in the campaign)
    int m_InCampaignStage;
    // How many separate players this activity can support at the same time.
    int m_MaxPlayerSupport;
    // How many separate teams this activity can support at the same time.
    int m_MinTeamsRequired;
    // Current state of the game (STARTING, RUNNING, OVER, etc).
    int m_ActivityState;
    // Whether game is paused or not.
    bool m_Paused;
    // The name of the scene in which this activity takes place
    std::string m_SceneName;
    // The number of teams in the current game.
    int m_TeamCount;
    // The number of total players in the current game, AI and Human.
    int m_PlayerCount;
    // The designated team of each player
    int m_Team[MAXPLAYERCOUNT];
    // How much this player contributed to his team's funds at the start of the activity
    float m_FundsContribution[MAXPLAYERCOUNT];
    // The ratio of how much this player contributed to his team's funds at the start of the activity
    float m_TeamFundsShare[MAXPLAYERCOUNT];
    // Whether a specific player is at all active and playing this activity
    bool m_IsActive[MAXPLAYERCOUNT];
    // Whether a specific player is Human or not, and needs a screen etc
    bool m_IsHuman[MAXPLAYERCOUNT];
    // The screen index of each player - only applicable to human players; -1 if AI or other
    int m_PlayerScreen[MAXPLAYERCOUNT];
    // Current difficulty setting, see the enum
    int m_Difficulty;

	bool m_IsMultiplayerReady;

    // The brain of each player. Not owned!
    Actor *m_pBrain[MAXPLAYERCOUNT];
    // Whether each player has yet had a brain. If not, then their game doesn't end if no brain is found
    bool m_HadBrain[MAXPLAYERCOUNT];
    // Whether a player has evacuated his brain into orbit
    bool m_BrainEvacuated[MAXPLAYERCOUNT];
    // What to be viewing for each player
    int m_ViewState[MAXPLAYERCOUNT];
    // Currently controlled actor, not owned
    Actor *m_pControlledActor[MAXPLAYERCOUNT];
    // The Controllers of all the players for the GUIs.
    Controller m_PlayerController[MAXPLAYERCOUNT];
    // Message timer
    Timer m_MsgTimer[MAXPLAYERCOUNT];

    // Team is active or not this game
    bool m_TeamActive[MAXTEAMCOUNT];
    // Gold counters for all teams.
    float m_TeamFunds[MAXTEAMCOUNT];
    // Whether the team funds have changed during the current frame
    bool m_FundsChanged[MAXTEAMCOUNT];
    // Names for each team
    std::string m_TeamNames[MAXTEAMCOUNT];
    // Icons for each team
    Icon m_TeamIcons[MAXTEAMCOUNT];
    // The count of how many actors have died on this team
    int m_TeamDeaths[MAXTEAMCOUNT];
    // AI skill levels for teams
    int m_TeamAISkillLevels[MAXTEAMCOUNT];

	// If true then on non-wrapped maps crafts beyound the edge of the map are considered orbited
	bool m_CraftsOrbitAtTheEdge;

    // Sound for when the funds of a team changes. Some kind of subtle gold/money sound
    Sound m_FundsChangedSound;
    // Sound for switching between regular (non-brain) actors
    Sound m_ActorSwitchSound;
    // Sound for switching to the brain shortcut
    Sound m_BrainSwitchSound;
    // Air woosh sound for traveling between actors
    Sound m_CameraTravelSound;
    // Sound for confirming a choice
    Sound m_ConfirmSound;
    // Sound for selecting items in list, etc.
    Sound m_SelectionChangeSound;
    // Sound for erroneus input
    Sound m_UserErrorSound;


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


    // Disallow the use of some implicit methods.
    Activity(const Activity &reference);
    Activity & operator=(const Activity &rhs);

};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the Activity:s and rules of Cortex Command
// Parent(s):       Singleton.
// Class history:   8/13/2004 ActivityMan created.

class ActivityMan:
    public Singleton<ActivityMan>//,
//    public Serializable
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ActivityMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ActivityMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ActivityMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ActivityMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~ActivityMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ActivityMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
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
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ActivityMan to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ActivityMan will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ActivityMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the ActivityMan object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_ClassName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDefaultActivityType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the type name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       The default activity type name.
// Return value:    None.

    void SetDefaultActivityType(std::string defaultActivityType) { m_DefaultActivityType = defaultActivityType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDefaultActivityName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the instance name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       The default activity instance name.
// Return value:    None.

    void SetDefaultActivityName(std::string defaultActivityName) { m_DefaultActivityName = defaultActivityName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDefaultActivityType
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the type name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       None.
// Return value:    The default activity type name.

    std::string GetDefaultActivityType() const { return m_DefaultActivityType; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDefaultActivityName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the default activity to be loaded if nothing
//                  else is available.
// Arguments:       None.
// Return value:    The default activity instance name.

    std::string GetDefaultActivityName() const { return m_DefaultActivityName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a new activity to copy for next restart. You have to use
//                  RestartActivity to get it going. Ownership IS transferred!
// Arguments:       The new activity to put into effect next time ResetActivity is called.
// Return value:    None.

    void SetStartActivity(Activity *pNewActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the activity set to use a copy of for next restart. Ownership is
//                  NOT transferred!
// Arguments:       None.
// Return value:    The activity to put into effect next time ResetActivity is called. OINT!

    Activity * GetStartActivity() { return m_pStartActivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current activity in effect. Won't be what has been set by
//                  SetStartActivity unless RestartActivity has been called since.
// Arguments:       None.
// Return value:    The current Activity in effect. Will be 0 if no activity is going.

    Activity * GetActivity() { return m_pActivity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the activity passed in. Ownership IS transferred!
// Arguments:       The new activity to start. Ownership IS passed in!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int StartActivity(Activity *pActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially gets and starts the activity described.
// Arguments:       The class and instance names of the activity to start. Has to have
//                  been read in already (duh).
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int StartActivity(std::string className, std::string instanceName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestartActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Completely restarts whatever activity was last started.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int RestartActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PauseActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

    void PauseActivity(bool pause = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EndActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

    void EndActivity();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityRunning
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently running or not (not editing,
//                  over or paused)
// Arguments:       None.
// Return value:    Whether the game is running or not.

    bool ActivityRunning() const { if (m_pActivity) { return m_pActivity->Running(); } return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ActivityPaused
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the game is currently paused or not.
// Arguments:       None.
// Return value:    Whether the game is paused or not.

    bool ActivityPaused() const { if (m_pActivity) { return m_pActivity->Paused(); } return true; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this and the current Activity. Supposed to be
//                  done every frame before drawing.
// Arguments:       None.
// Return value:    None.

    void Update();

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LateUpdateGlobalScripts
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only updates Global Scripts of the current activity with LateUpdate flag enabled
// Arguments:       None.
// Return value:    None.

	void LateUpdateGlobalScripts();


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static const std::string m_ClassName;
    // The type and name of the default activity to be loaded if nothing else is available.
    std::string m_DefaultActivityType;
    std::string m_DefaultActivityName;
    // The starting condition of the next activity to be (re)started. Owned by this
    Activity *m_pStartActivity;
    // The current Activity in action, OWNED by this!
    Activity *m_pActivity;
    // Path to the last music stream being played
    std::string m_LastMusicPath;
    // What the last position of the in-game music track was before pause, in seconds
    double m_LastMusicPos;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ActivityMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ActivityMan(const ActivityMan &reference);
    ActivityMan & operator=(const ActivityMan &rhs);

};

} // namespace RTE

#endif // File