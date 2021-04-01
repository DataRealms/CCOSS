#ifndef _RTEMETAMAN_
#define _RTEMETAMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MetaMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MetaMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "Singleton.h"
#define g_MetaMan MetaMan::Instance()

//#include "Serializable.h"
#include "SceneObject.h"
#include "Controller.h"
//#include "FrameMan.h"
//#include "SceneMan.h"
#include "ActivityMan.h"
#include "MetaPlayer.h"
#include "GAScripted.h"
#include "Icon.h"
#include "GUIBanner.h"

namespace RTE
{

#define DEFAULTGAMENAME "NewGame"
#define AUTOSAVENAME "AutoSave"
#define METASAVEPATH "Metagames.rte/"
#define METASAVEMODULENAME "Metagames.rte"

class MetagameGUI;
class MetaSave;
class Scene;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MetaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of the Metagame of Cortex Command, ie the
//                  games played out in the campaign screen.
// Parent(s):       Singleton, serializable
// Class history:   10/10/2009 MetaMan created.

class MetaMan:
    public Singleton<MetaMan>,
    public Serializable
{
    friend class LuaMan;


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

friend class MetagameGUI;
friend class MetaSave;

	SerializableClassNameGetter
	SerializableOverrideMethods

    enum MetagameState
    {
        NOGAME = -1,
        GAMEINTRO = 0,
        NEWROUND,
        REVEALSCENES,
        COUNTINCOME,
        PLAYER1TURN,
        PLAYER2TURN,
        PLAYER3TURN,
        PLAYER4TURN,
        BUILDBASES,
        RUNACTIVITIES,
        ENDROUND,
        GAMEOVER
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MetaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MetaMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MetaMan() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MetaMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MetaMan object before deletion
//                  from system memory.
// Arguments:       None.

    ~MetaMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetaMan object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Initialize();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  NewGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wipes any current and sets up a new game based on a size parameter.
// Arguments:       The size of the new Metagame, which will affect how
//                  many Scenes/Sites will ultimately be used.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int NewGame(int gameSize = 3);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  EndGame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wipes any current metagame and sets things back to as if program start.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int EndGame();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Load
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Load a Metagame from disk out of the special Metagame.rte data module
// Arguments:       The MetaSave object to load from - Ownership Is Not Transferred!
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int Load(const MetaSave *pSave);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SaveSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the bitmap data of all Scenes of this Metagame that are currently
//                  loaded.
// Arguments:       The filepath base to the where to save the Bitmap data. This means
//                  everything up to and including the unique name of the game.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    int SaveSceneData(std::string pathBase);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the bitmap data of all Scenes of this Metagame that have once
//                  been saved to files.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    int LoadSceneData();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSceneData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the bitmap data of all Scenes of this Metagame that have once
//                  been saved to files.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    int ClearSceneData();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MetaMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MetaMan object.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGameName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of the currently played Metagame. It's what's used when
//                  saving to disk.
// Arguments:       The Metagame's name.
// Return value:    None.

    void SetGameName(std::string newName) { m_GameName = newName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGameName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the currently played Metagame. It's what's used when
//                  saving to disk.
// Arguments:       None.
// Return value:    The name of the current metagame.

    std::string GetGameName() const { return m_GameName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the GUI controller of this Metagame.
// Arguments:       None.
// Return value:    The GUI controller of the metagame.

    MetagameGUI *GetGUI() { return m_pMetaGUI; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerTurn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows which player's turn is now or coming up.
// Arguments:       None.
// Return value:    The player who is currently doing his turn, or coming up next in an
//                  intermediate phase.

    int GetPlayerTurn() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayerCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets current number of MetaPlayers
// Arguments:       None
// Return value:    The number of meta players in the current game.

    int GetPlayerCount() const { return m_Players.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the designated team of a specific player
// Arguments:       Which player.
// Return value:    The team of that player.

    int GetTeamOfPlayer(int metaPlayer) const { return metaPlayer >= Players::PlayerOne && metaPlayer < m_Players.size() ? m_Players[metaPlayer].GetTeam() : Activity::NoTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the specified MetaPlayer
// Arguments:       Which player.
// Return value:    The requested MetaPlayer

    MetaPlayer * GetPlayer(int metaPlayer) { return (metaPlayer >= Players::PlayerOne && metaPlayer < m_Players.size()) ? &(m_Players[metaPlayer]) : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMetaPlayerOfInGamePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the MetaPlayer playing a specific in-game player, if any.
// Arguments:       Which in-game player to translate into a metaplayer.
// Return value:    The requested MetaPlayer, if any is playing that in-game player. If not
//                  0 is returned.

    MetaPlayer * GetMetaPlayerOfInGamePlayer(int inGamePlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the flag Icon of a specific team
// Arguments:       The team to get the Team icon of.
// Return value:    A reference to the Icon.

    Icon & GetTeamIcon(int team) { return m_TeamIcons[team]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNextSceneOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next Scene in play that is owned by a specific player.
// Arguments:       The player to get the next owned Scene of.
//                  The Scene to start searching from in the current roster of Scenes, OWNERSHIP IS NOT TRANSFERRED!
// Return value:    A pointer to the next Scene found in the sequence. OWNERSHIP IS NOT TRANSFERRED!

    const Scene * GetNextSceneOfPlayer(int metaPlayer, const Scene *pScene = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalBrainCountOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of brains that a player has, including ones of
//                  his that are resident down on sites.
// Arguments:       The metagame player to get the total brain count from.
//                  Whether to only count the brains in the pools, or to also include all
//                  resident brains as well.
// Return value:    The total number of brains that belong to the metagame player.

    int GetTotalBrainCountOfPlayer(int metaPlayer, bool countPoolsOnly = false) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total gold funds of all the players of a specific team combined.
// Arguments:       The metagame team to get the total gold funds of.
// Return value:    The total amount of ounces of gold this team has.

    int GetGoldCountOfTeam(int team) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of bases that any specific team owns.
// Arguments:       The team to get the scene/site ownership count of.
// Return value:    The count of scenes owned by this team.

    int GetSceneCountOfTeam(int team) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalBrainCountOfTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total number of brains that a team has, including ones that
//                  are resident down on sites.
// Arguments:       The metagame team to get the total brain count from.
//                  Whether to only count the brains in the pools, or to also include all
//                  resident brains as well.
// Return value:    The total number of brains that belong to the metagame team.

    int GetTotalBrainCountOfTeam(int team, bool countPoolsOnly = false) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnlyTeamWithAnyBrainPoolLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which team, if any, is the only one left with brains in its
//                  pool.
// Arguments:       None.
// Return value:    Which team, if any, is the sole remaining with any brains left in its
//                  players' brain pools.

    int OnlyTeamWithAnyBrainPoolLeft();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NoBrainsLeftInAnyPool
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether there are no brains left in any active player's pool
//                  at all. This does NOT count deployed brain in bases.
// Arguments:       None.
// Return value:    Whether there are no brains left in any player's brain pool.

    bool NoBrainsLeftInAnyPool();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamIsLeading
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which single team has the most owned bases, and if there's a
//                  tie between two teams, total owned gold funds is used as a tiebreaker.
// Arguments:       None.
// Return value:    Which team is currently in the lead.

    int WhichTeamIsLeading();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneIncomeOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total income from all scenes owned by a specific metaPlayer.
// Arguments:       The metagame player to get the total scene income from.
// Return value:    The amount of income, in oz, the player made this round from its scenes.

    float GetSceneIncomeOfPlayer(int metaPlayer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBudgetedRatioOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ratio of funds already allocated to budgets of this player.
// Arguments:       The metagame player to get the budget ratio of.
//                  A scene to exclude from the tally, if any.
//                  Whether to count the money allocated for offensive action.
//                  Whether to count the money allocated for defensive actions.
// Return value:    The amount, in ratio, that this player already has allocated.

    float GetBudgetedRatioOfPlayer(int metaPlayer, const Scene *pException = 0, bool includeOffensive = true, bool includeDefensive = true) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRemainingFundsOfPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the count of funds still unbudgeted and available of a player.
// Arguments:       The metagame player to get the unallocated funds of.
//                  A scene to exclude from the tally, if any.
//                  Whether to count the money allocated for offensive action as remaining.
//                  Whether to count the money allocated for defensive action as remaining.
// Return value:    The amount, in oz, that this player unallocated and unused this turn.

    float GetRemainingFundsOfPlayer(int metaPlayer, const Scene *pException = 0, bool deductOffensive = false, bool deductDefensive = false) const  { return m_Players[metaPlayer].GetFunds() - m_Players[metaPlayer].GetFunds() * GetBudgetedRatioOfPlayer(metaPlayer, pException, !deductOffensive, !deductDefensive); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GameInProgress
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether a game is currently in progress
// Arguments:       None.
// Return value:    Whether a game is going or not.

    bool GameInProgress() { return m_GameState >= GAMEINTRO && m_GameState <= ENDROUND; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsSuspended
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether game is suspended or not.
// Arguments:       None.
// Return value:    Whether suspended or not.

    bool IsSuspended() { return m_Suspended; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSuspend
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Suspends or unsuspends the game so exclusive GUIs and menus can be
//                  shown.
// Arguments:       Whether to suspend or not.
// Return value:    None.

    void SetSuspend(bool suspend);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsActivePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks wheter a certain player index is valid for the current game
// Arguments:       None.
// Return value:    Whether the player index passed in is active for the current game.

    bool IsActivePlayer(int metaPlayer) { return metaPlayer >= Players::PlayerOne && metaPlayer < m_Players.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsActiveTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks wheter a certain team index is valid for the current game
// Arguments:       None.
// Return value:    Whether the team index passed in is active for the current game.

    bool IsActiveTeam(int team) { return team >= Activity::TeamOne && team < m_TeamCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamOwnsAllSites
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether one team has ownership of all revealed sites.
// Arguments:       None.
// Return value:    Which team has all sites, if any. If not NoTeam is returned.

    int WhichTeamOwnsAllSites();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsGameOver
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for game over condition
// Arguments:       None.
// Return value:    Whether the game over conditions have been met

    bool IsGameOver();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GameIsSaved
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether the game has been saved and no data loss will result
//                  if program is quit right now.
// Arguments:       None.
// Return value:    Whether the game is saved.

    bool GameIsSaved() { return m_GameSaved || m_GameState <= NOGAME || m_GameState >= GAMEOVER; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TotalScenePresets
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Yields a set of ALL eligible Scene presets for a new game.
// Arguments:       The list to fill with all the eligible presets. If no list is passed
//                  it will be ignored. Presets returned in list are NOT OWNED there.
// Return value:    The count of total number preset scenes eligible for gameplay.

    int TotalScenePresets(std::list<Scene *> *pScenes = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SelectScenePresets
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Yields a set of randomly selected Scene presets for a new game.
// Arguments:       The size of the set.
//                  The list to fill with the selected presets, depending on currently
//                  set player numbers and loaded eligible scenes. If no list is passed
//                  it will be ignored. Presets returned in list are NOT OWNED there.
// Return value:    The count of selected preset scenes.

    int SelectScenePresets(int gameSize, std::list<Scene *> *pSelected = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearActivities
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out all the lined-up activities for the current round.
// Arguments:       None.
// Return value:    None.

    void ClearActivities();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AIPlayerTurn
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does all the things an AI metaPlayer needs to do during his turn.
// Arguments:       Which AI metaPlayer we're going to process.
// Return value:    None.

    void AIPlayerTurn(int metaPlayer);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this and the current Metagame. Supposed to be
//                  done every frame before drawing.
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MetaMan's current graphical representation to a BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector());


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // GUI controller, owned
    MetagameGUI *m_pMetaGUI;

    // Current Metagame state
    int m_GameState;
    // Whether the state just changed
    bool m_StateChanged;
    // Whether the game is currently suspended (e.g. in the menu)
    bool m_Suspended;
    // Whether game has been saved since the last suspension of it
    bool m_GameSaved;

    // The save name of the currently played metagame
    std::string m_GameName;
    // The players of the metagame
    std::vector<MetaPlayer> m_Players;
    // The number of Team:s in play this game
    int m_TeamCount;
    // The flag icons of all teams
    Icon m_TeamIcons[Activity::MaxTeamCount];
    // The current round the game is on, starting with count on 0
    int m_CurrentRound;
    // All Scenes of the current game, OWNED by this. Stored sequentially in order of revealing
    std::vector<Scene *> m_Scenes;
    // How many of the scenes have been revealed so far - the whole number. It's a float to increase it slower than once a round
    float m_RevealedScenes;
    // How many scenes to reveal each round.. can be a fractional that adds up over several days
    float m_RevealRate;
    // Any extra reveals to make next reveal phase
    float m_RevealExtra;

    // The Activities generated by the current round's offensive maneuvers
    std::vector<GAScripted *> m_RoundOffensives;
    // The current offensive action that we're about to play next
    int m_CurrentOffensive;
	// Game difficulty
	int m_Difficulty;
	// Teams AI Skill
	int m_TeamAISkill[Activity::MaxTeamCount];

    // Timer for measuring how long each phase has gone for
    Timer m_PhaseTimer;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

	static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MetaMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	MetaMan(const MetaMan &reference) = delete;
	MetaMan & operator=(const MetaMan &rhs) = delete;

};
} // namespace RTE

#endif // File