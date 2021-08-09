#ifndef _RTEGAMEACTIVITY_
#define _RTEGAMEACTIVITY_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GameActivity.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ActivityMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUISound.h"
#include "RTETools.h"
#include "ActivityMan.h"
#include "Scene.h"
#include "Actor.h"

namespace RTE
{

#define OBJARROWFRAMECOUNT 4
#define LZCURSORFRAMECOUNT 4

class Actor;
class ACraft;
class PieMenuGUI;
class InventoryMenuGUI;
class BuyMenuGUI;
class SceneEditorGUI;
class GUIBanner;
class Loadout;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           GameActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Base class for all GameActivity:s, including game modes and editors.
// Parent(s):       Activity.
// Class history:   8/7/2007 GameActivity created.

class GameActivity:
    public Activity
{

    friend struct ActivityLuaBindings;

    // Keeps track of everything about a delivery in transit after purchase has been made with the menu
    struct Delivery
    {
        // OWNED by this until the delivery is made!
        ACraft *pCraft;
        // Which player ordered this delivery
        int orderedByPlayer;
        // Where to land
        Vector landingZone;
        // How long left until entry, in ms
        long delay;
        // Times how long we've been in transit
        Timer timer;
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods
	ClassInfoGetters

    enum ObjectiveArrowDir
    {
        ARROWDOWN = 0,
        ARROWLEFT,
        ARROWRIGHT,
        ARROWUP
    };

    enum BannerColor
    {
        RED = 0,
        YELLOW
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GameActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GameActivity object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    GameActivity() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~GameActivity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a GameActivity object before deletion
//                  from system memory.
// Arguments:       None.

	~GameActivity() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the GameActivity object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a GameActivity to be identical to another, by deep copy.
// Arguments:       A reference to the GameActivity to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const GameActivity &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire GameActivity, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); Activity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the GameActivity object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

	void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCPUTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current CPU-assisted team, if any (NoTeam) - LEGACY function
// Arguments:       None.
// Return value:    The current setting. NoTeam is no team is assisted.

    int GetCPUTeam() const { return m_CPUTeam; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCPUTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current CPU-assisted team, if any (NoTeam) - LEGACY function
// Arguments:       The new setting. NoTeam is no team is assisted.
// Return value:    None.

    void SetCPUTeam(int team = Activity::NoTeam);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetObservationTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the observation sceneman scroll targets, for when the game is
//                  over or a player is in observation mode
// Arguments:       The new absolute position to observe.
//                  Which player to set it for.
// Return value:    None.

    void SetObservationTarget(const Vector &newTarget, int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) m_ObservationTarget[player] = newTarget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDeathViewTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the player death sceneman scroll targets, for when a player-
//                  controlled actor dies and the view should go to his last position
// Arguments:       The new absolute position to set as death view.
//                  Which player to set it for.
// Return value:    None.

    void SetDeathViewTarget(const Vector &newTarget, int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) m_DeathViewTarget[player] = newTarget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLandingZone
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the he last selected landing zone.
// Arguments:       The new absolute position to set as the last selected landing zone.
//                  Which player to set it for.
// Return value:    None.

    void SetLandingZone(const Vector &newZone, int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) m_LandingZone[player] = newZone; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLandingZone
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the he last selected landing zone.
// Arguments:       Which player to get it for.
// Return value:    The new absolute position to set as the last selected landing zone.

    Vector GetLandingZone(int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) return m_LandingZone[player]; else return Vector(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetActorSelectCursor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the actor selection cursor position.
// Arguments:       The new absolute position to put the cursor at.
//                  Which player to set it for.
// Return value:    None.

    void SetActorSelectCursor(const Vector &newPos, int player = 0) { if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) m_ActorCursor[player] = newPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBuyGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the an in-game GUI Object for a specific player.
// Arguments:       Which player to get the GUI for.
// Return value:    A pointer to a BuyMenuGUI. Ownership is NOT transferred!

    BuyMenuGUI * GetBuyGUI(unsigned int which = 0) const { return m_pBuyGUI[which]; }

    /// <summary>
    /// Checks if the in-game GUI Object is visible for a specific player.
    /// </summary>
    /// <param name="which">Which player to check the GUI for. -1 will check all players.</param>
    /// <returns>Whether or not the BuyMenuGUI is visible for input player(s).</returns>
    bool IsBuyGUIVisible(int which = 0) const;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEditorGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the an in-game editor GUI Object for a specific player.
// Arguments:       Which player to get the GUI for.
// Return value:    A pointer to a SceneEditorGUI. Ownership is NOT transferred!

    SceneEditorGUI * GetEditorGUI(unsigned int which = 0) const { return m_pEditorGUI[which]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the this to focus player control to a specific Actor for a
//                  specific team. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which Actor to switch focus to. The team of this Actor will be set
//                  once it is passed in. Ownership IS NOT TRANSFERRED! The Actor should
//                  be added to MovableMan already.
// Return value:    Whether the focus switch was successful or not.

	bool SwitchToActor(Actor *pActor, int player = 0, int team = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToNextActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the this to focus player control to the next Actor of a
//                  specific team, other than the current one focused on.
// Arguments:       Which team to switch to next actor on.
//                  An actor pointer to skip in the sequence.
// Return value:    None.

	void SwitchToNextActor(int player, int team, Actor *pSkip = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SwitchToPrevActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces this to focus player control to the previous Actor of a
//                  specific team, other than the current one focused on.
// Arguments:       Which team to switch to next actor on.
//                  An actor pointer to skip in the sequence.
// Return value:    None.

	void SwitchToPrevActor(int player, int team, Actor *pSkip = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetWinnerTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team is the winner, when the game is over.
// Arguments:       The team number of the winning team. 0 is team #1. Negative number
//                  means the game isn't over yet.
// Return value:    None.

    void SetWinnerTeam(int winnerTeam) { m_WinnerTeam = winnerTeam; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWinnerTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which team is the winner, when the game is over.
// Arguments:       None.
// Return value:    The team number of the winning team. 0 is team #1. Negative number
//                  means the game isn't over yet.

    int GetWinnerTeam() const { return m_WinnerTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBanner
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets access to the huge banner of any player that can display
//                  messages which can not be missed or ignored.
// Arguments:       Which color banner to get - see the GameActivity::BannerColor enum.
//                  Which player's banner to get.
// Return value:    A pointer to the GUIBanner object that we can 

    GUIBanner * GetBanner(int whichColor = YELLOW, int player = Players::PlayerOne) { return whichColor == YELLOW ? m_pBannerYellow[player] : m_pBannerRed[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLZArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Area within which a team can land things.
// Arguments:       The number of the team we're setting for.
//                  The Area we're setting to limit their landings within.
// Return value:    None.

    void SetLZArea(int team, const Scene::Area &newArea) { m_LandingZoneArea[team].Reset(); m_LandingZoneArea[team].Create(newArea); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLZArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Area within which a team can land things. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       The number of the team we're setting for.
// Return value:    The Area we're using to limit their landings within. OWNERSHIP IS NOT TRANSFERRED!

    const Scene::Area & GetLZArea(int team) const { return m_LandingZoneArea[team]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBrainLZWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the width of the landing zone box that follows around a player's
//                  brain.
// Arguments:       The number of the in-game player we're setting for.
//                  The width of the box, in pixels. 0 means disabled.
// Return value:    None.

    void SetBrainLZWidth(int player, int width) { m_BrainLZWidth[player] = width; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBrainLZWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the landing zone box that follows around a player's
//                  brain.
// Arguments:       The number of the player we're getting for.
// Return value:    The width in pixels of the landing zone.

    int GetBrainLZWidth(int player) const { return m_BrainLZWidth[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddObjectivePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Created an objective point for one of the teams to show until cleared.
// Arguments:       The team number of the team to give objective. 0 is team #1.
//                  The very short description of what the objective is (three short words max)
//                  The absolute scene coordiante position of the objective.
//                  The desired direction of the arrow when the point is on screen.
// Return value:    None.

    void AddObjectivePoint(std::string description, Vector objPos, int whichTeam = Teams::TeamOne, ObjectiveArrowDir arrowDir = ARROWDOWN);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          YSortObjectivePoints
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sorts all objective points according to their positions on the Y axis.
// Arguments:       None.
// Return value:    None.

    void YSortObjectivePoints();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearObjectivePoints
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all objective points previously added, for both teams.
// Arguments:       None.
// Return value:    None.

    void ClearObjectivePoints() { m_Objectives.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddOverridePurchase
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds somehting to the purchase list that will override what is set
//                  in the buy guy next time CreateDelivery is called.
// Arguments:       The SceneObject preset to add to the override purchase list. OWNERSHIP IS NOT TRANSFERRED!
//                  Which player's list to add an override purchase item to.
// Return value:    The new total value of what's in the override purchase list.

    int AddOverridePurchase(const SceneObject *pPurchase, int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOverridePurchaseList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     First clears and then adds all the stuff in a Loadout to the override
//                  purchase list.
// Arguments:       The Loadout preset to set the override purchase list to reflect. OWNERSHIP IS NOT TRANSFERRED!
//                  The player we're talking about.
// Return value:    The new total value of what's in the override purchase list.

    int SetOverridePurchaseList(const Loadout *pLoadout, int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOverridePurchaseList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     First clears and then adds all the stuff in a Loadout to the override
//                  purchase list.
// Arguments:       The name of the Loadout preset to set the override purchase list to
//                  represent.
// Return value:    The new total value of what's in the override purchase list.

    int SetOverridePurchaseList(string loadoutName, int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearOverridePurchase
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all items from a specific player's override purchase list.
// Arguments:       Which player's override purchase list to clear.
// Return value:    None.

    void ClearOverridePurchase(int player) { m_PurchaseOverride[player].clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateDelivery
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current order out of a player's buy GUI, creates a Delivery
//                  based off it, and stuffs it into that player's delivery queue.
// Arguments:       Which player to create the delivery for. Cargo AI mode and waypoint.
// Return value:    Success or not.

    bool CreateDelivery(int player, int mode, Vector &waypoint) { return CreateDelivery(player, mode, waypoint, NULL); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateDelivery
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current order out of a player's buy GUI, creates a Delivery
//                  based off it, and stuffs it into that player's delivery queue.
// Arguments:       Which player to create the delivery for. Cargo AI mode and TargetMO.
// Return value:    Success or not.

    bool CreateDelivery(int player, int mode, Actor *pTargetMO) { Vector point( -1, -1 ); return CreateDelivery(player, mode, point, pTargetMO); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateDelivery
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current order out of a player's buy GUI, creates a Delivery
//                  based off it, and stuffs it into that player's delivery queue.
// Arguments:       Which player to create the delivery for and Cargo AI mode.
// Return value:    Success or not.

    bool CreateDelivery(int player, int mode) { Vector point( -1, -1 ); return CreateDelivery(player, mode, point, NULL); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateDelivery
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes the current order out of a player's buy GUI, creates a Delivery
//                  based off it, and stuffs it into that player's delivery queue.
// Arguments:       Which player to create the delivery for. 
// Return value:    Success or not.

    bool CreateDelivery(int player) { Vector point( -1, -1 ); return CreateDelivery(player, Actor::AIMODE_SENTRY, point, NULL); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeliveryCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how many deliveries this team has pending.
// Arguments:       Which team to check the delivery count for.
// Return value:    The number of deliveries this team has coming.

    int GetDeliveryCount(int team) { return m_Deliveries[team].size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetupPlayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Precalculates the player-to-screen index map, counts the number of
//                  active players etc.
// Arguments:       None.
// Return value:    None.

	void SetupPlayers() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Start
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Officially starts the game accroding to parameters previously set.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Start() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Pause
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Pauses and unpauses the game.
// Arguments:       Whether to pause the game or not.
// Return value:    None.

	void SetPaused(bool pause = true) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  End
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Forces the current game's end.
// Arguments:       None.
// Return value:    None.

	void End() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  UpdateEditing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     This is a special update step for when any player is still editing the
//                  scene.
// Arguments:       None.
// Return value:    None.

	void UpdateEditing();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawGUI
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
// Arguments:       A pointer to a screen-sized BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the scene.
//                  Which screen's GUI to draw onto the bitmap.
// Return value:    None.

	void DrawGUI(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int which = 0) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ActivityMan's current graphical representation to a
//                  BITMAP of choice. This includes all game-related graphics.
// Arguments:       A pointer to a BITMAP to draw on. OWNERSHIP IS NOT TRANSFERRED!
//                  The absolute position of the target bitmap's upper left corner in the scene.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) override;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamTech
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the name of the tech module selected for this team during scenario setup
// Arguments:       Team to return tech module for
// Return value:    Tech module name, for example Dummy.rte, or empty string if there is no team
	std::string GetTeamTech(int team) { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamTech[team] : ""; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamTech
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets tech module name for specified team. Module must set must be loaded.
// Arguments:       Team to set module, module name, for example Dummy.rte
// Return value:    None
	void SetTeamTech(int team, std::string tech);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TeamIsCPU
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a specific team is assigned a CPU player in the current game.
// Arguments:       Which team index to check.
// Return value:    Whether the team is assigned a CPU player in the current activity.
    bool TeamIsCPU(int team) const { return (team >= Teams::TeamOne && team < Teams::MaxTeamCount) ? m_TeamIsCPU[team] : false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActiveCPUTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns active CPU team count.
// Arguments:       None.
// Return value:    Returns active CPU team count.
	int GetActiveCPUTeamCount() const;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActiveHumanTeamCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns active human team count.
// Arguments:       None.
// Return value:    Returns active human team count.
	int GetActiveHumanTeamCount() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetStartingGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes how much starting gold was selected in scenario setup dialog. 20000 - infinite amount.
// Arguments:       Starting gold amount
// Return value:    None.
	void SetStartingGold(int amount) { m_StartingGold = amount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStartingGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns how much starting gold was selected in scenario setup dialog. 20000 - infinite amount.
// Arguments:       None.
// Return value:    How much starting gold must be given to human players.
	int GetStartingGold() { return m_StartingGold; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFogOfWarEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes whether fog of war must be enabled for this activity or not. 
//					Never hides or reveals anything, just changes internal flag.
// Arguments:       New fog of war state. true = enabled.
// Return value:	None.    
	void SetFogOfWarEnabled(bool enable) { m_FogOfWarEnabled = enable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFogOfWareEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether fog of war must be enabled for this activity or not.
//					Call it to determine whether you should call MakeAllUnseen or not at the start of activity.
// Arguments:       None.
// Return value:    Whether Fog of war flag was checked during scenario setup dialog.
	bool GetFogOfWarEnabled() { return m_FogOfWarEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRequireClearPathToOrbit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether player activity requires a cleat path to orbit to place brain
//					
// Arguments:       None.
// Return value:	Whether we need a clear path to orbit to place brains.    
	bool GetRequireClearPathToOrbit() const { return m_RequireClearPathToOrbit; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRequireClearPathToOrbit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether player activity requires a cleat path to orbit to place brain
//					
// Arguments:       Whether we need a clear path to orbit to place brains.
// Return value:	None.
	void SetRequireClearPathToOrbit(bool newvalue) { m_RequireClearPathToOrbit = newvalue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultFogOfWar() const { return m_DefaultFogOfWar; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultRequireClearPathToOrbit() const { return m_DefaultRequireClearPathToOrbit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultDeployUnits() const { return m_DefaultDeployUnits; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultGoldCake() const { return m_DefaultGoldCake; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultGoldEasy() const { return m_DefaultGoldEasy; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultGoldMedium() const { return m_DefaultGoldMedium; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultGoldHard() const { return m_DefaultGoldHard; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	int GetDefaultGoldNuts() const { return m_DefaultGoldNuts; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	bool GetFogOfWarSwitchEnabled() const { return m_FogOfWarSwitchEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	
	bool GetDeployUnitsSwitchEnabled() const { return m_DeployUnitsSwitchEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	

	bool GetGoldSwitchEnabled() const { return m_GoldSwitchEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:	

	bool GetRequireClearPathToOrbitSwitchEnabled() const { return m_RequireClearPathToOrbitSwitchEnabled; }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetCrabToHumanSpawnRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns CrabToHumanSpawnRatio for specified module
// Arguments:       None.
// Return value:    Crab-To-Human spawn ratio value set for specified module, 0.25 is default.

	float GetCrabToHumanSpawnRatio(int moduleid); 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeliveryDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns current delivery delay
// Arguments:       None.
// Return value:    Returns current delivery delay

	long GetDeliveryDelay() const {  return m_DeliveryDelay; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDeliveryDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets delivery delay
// Arguments:       New delivery delay value in ms
// Return value:    None

	void SetDeliveryDelay(long newDeliveryDelay) { m_DeliveryDelay =  newDeliveryDelay > 1 ? newDeliveryDelay : 1; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBuyMenuEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether buy menu is enabled in this activity.
// Arguments:       True if buy menu enabled false otherwise
// Return value:    None.

	bool GetBuyMenuEnabled() const { return m_BuyMenuEnabled; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBuyMenuEnabled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether buy menu is enabled in this activity
// Arguments:       True to enable buy menu, false otherwise
// Return value:    None.

	void SetBuyMenuEnabled(bool newValue) { m_BuyMenuEnabled = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNetworkPlayerName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns network player name
// Arguments:       Player 
// Return value:    Network player name

	std::string & GetNetworkPlayerName(int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNetworkPlayerName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets network player name
// Arguments:       Player number, player name
// Return value:    None

	void SetNetworkPlayerName(int player, std::string name);

    virtual void OnPieMenu(Actor *actor) { /* Does nothing, kept here for program control flow. Method is not pure virtual to avoid a bunch of junk implementations in non-scritped activities. */};

	void AddPieMenuSlice(std::string description, std::string functionName, PieSlice::SliceDirection direction, bool isEnabled)
	{ 
		if (m_CurrentPieMenuPlayer >= Players::PlayerOne && m_CurrentPieMenuPlayer < Players::MaxPlayerCount)
			m_pPieMenu[m_CurrentPieMenuPlayer]->AddPieSliceLua(description, functionName, direction, isEnabled);
	};

	void AlterPieMenuSlice(std::string description, std::string functionName, PieSlice::SliceDirection direction, bool isEnabled)
	{
		if (m_CurrentPieMenuPlayer >= Players::PlayerOne && m_CurrentPieMenuPlayer < Players::MaxPlayerCount)
			m_pPieMenu[m_CurrentPieMenuPlayer]->AlterPieSliceLua(description, functionName, direction, isEnabled);
	};

    PieSlice RemovePieMenuSlice(std::string description, std::string functionName)
	{
		if (m_CurrentPieMenuPlayer >= Players::PlayerOne && m_CurrentPieMenuPlayer < Players::MaxPlayerCount)
			return m_pPieMenu[m_CurrentPieMenuPlayer]->RemovePieSliceLua(description, functionName);
		return PieSlice("", PieSlice::PieSliceIndex::PSI_NONE, PieSlice::SliceDirection::NONE, false);
	};

	std::vector<PieSlice *> GetCurrentPieMenuSlices(int player) const
	{ 
		//if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
			return m_pPieMenu[player]->GetCurrentSlices();
		//return 0;
	}

	/*std::vector<PieSlice> * GetAvailablePieMenuSlices(int player) const 
	{ 
		if (player >= Players::PlayerOne && player < Players::MaxPlayerCount)
			return &m_pPieMenu[player]->GetAvailableSlices();
		return 0;
	}*/

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          CreateDelivery
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Takes the current order out of a player's buy GUI, creates a Delivery
    //                  based off it, and stuffs it into that player's delivery queue.
    // Arguments:       Which player to create the delivery for. Cargo AI mode waypoint or TargetMO.
    // Return value:    Success or not.

        bool CreateDelivery(int player, int mode, Vector &waypoint, Actor *pTargetMO);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Struct:          ObjectivePoint
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     A struct to keep all data about a mission objective.
    // Parent(s):       None.
    // Class history:   11/17/2008  ObjectivePoint

    struct ObjectivePoint
    {
        ObjectivePoint() { m_Description.clear(); m_ScenePos.Reset(); m_Team = Teams::NoTeam; m_ArrowDir = ARROWDOWN; }
        ObjectivePoint(const std::string &desc, const Vector &pos, int team = -1, ObjectiveArrowDir arrowDir = ARROWDOWN) { m_Description = desc; m_ScenePos = pos; m_Team = (Teams)team; m_ArrowDir = arrowDir; }


        //////////////////////////////////////////////////////////////////////////////////////////
        // Method:          Draw
        //////////////////////////////////////////////////////////////////////////////////////////
        // Description:     Simply draws this' arrow relative to a point on a bitmap.
        // Arguments:       A pointer to the BITMAP to draw on.
        //                  The arrow bitmap to draw, assuming it points downward.
        //                  The absolute position on the bitmap to draw the point of the arrow at.
        //                  Which orientation to draw the arrow in, relative to the point.
        // Return value:    None.

        void Draw(BITMAP *pTargetBitmap, BITMAP *pArrowBitmap, const Vector &arrowPoint, ObjectiveArrowDir arrowDir = ARROWDOWN);


        // The description of this objective point
        std::string m_Description;
        // Absolute position in the scene where this is pointed
        Vector m_ScenePos;
        // The team this objective is relevant to
        Teams m_Team;
        // The positioning of the arrow that points at this objective
        ObjectiveArrowDir m_ArrowDir;
    };


    // Comparison functor for sorting objective points by their y pos using STL's sort
    struct ObjPointYPosComparison {
        bool operator()(ObjectivePoint &rhs, ObjectivePoint &lhs) { return rhs.m_ScenePos.m_Y < lhs.m_ScenePos.m_Y; }
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OtherTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the next other team number from the one passed in, if any. If there
//                  are more than two teams in this game, then the next one in the series
//                  will be returned here.
// Arguments:       The team not to get.
// Return value:    The other team's number.

    int OtherTeam(int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OneOrNoneTeamsLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether there is less than two teams left in this game with
//                  a brain in its ranks.
// Arguments:       None.
// Return value:    Whether less than two teams have brains in them left.

    bool OneOrNoneTeamsLeft();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WhichTeamLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which single team is left, if any.
// Arguments:       None.
// Return value:    Which team stands alone with any brains in its ranks, if any. NoTeam
//                  is returned if there's either more than one team, OR there are no
//                  teams at all left with brains in em.

    int WhichTeamLeft();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NoTeamLeft
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether there are NO teams left with any brains at all!
// Arguments:       None.
// Return value:    Whether any team has a brain in it at all.

    bool NoTeamLeft();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitAIs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through all Actor:s currently in the MovableMan and sets each
//                  one not controlled by a player to be AI controlled and AIMode setting
//                  based on team and CPU team.
// Arguments:       None.
// Return value:    None.

    virtual void InitAIs();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DisableAIs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Goes through all Actor:s currently in the MovableMan and disables or
//                  enables each one with a Controller set to AI input.
// Arguments:       Whether to disable or enable them;
//                  Which team to do this to. If all, then pass Teams::NoTeam
// Return value:    None.

    void DisableAIs(bool disable = true, int whichTeam = Teams::NoTeam);




    // Member variables
    static Entity::ClassInfo m_sClass;

    // Which team is CPU-managed, if any (-1) - LEGACY, now controlled by Activity::m_IsHuman
    int m_CPUTeam;
    // Team is active or not this game
    bool m_TeamIsCPU[Teams::MaxTeamCount];

    // The observation sceneman scroll targets, for when the game is over or a player is in observation mode
    Vector m_ObservationTarget[Players::MaxPlayerCount];
    // The player death sceneman scroll targets, for when a player-controlled actor dies and the view should go to his last position
    Vector m_DeathViewTarget[Players::MaxPlayerCount];
    // Timers for measuring death cam delays.
    Timer m_DeathTimer[Players::MaxPlayerCount];
    // Times the delay between regular actor swtich, and going into manual siwtch mode
    Timer m_ActorSelectTimer[Players::MaxPlayerCount];
    // The cursor for selecting new Actors
    Vector m_ActorCursor[Players::MaxPlayerCount];
    // Highlighted actor while cursor switching; will be switched to if switch button is released now
    Actor *m_pLastMarkedActor[Players::MaxPlayerCount];
    // The last selected landing zone
    Vector m_LandingZone[Players::MaxPlayerCount];
    // Whether the last craft was set to return or not after delivering
    bool m_AIReturnCraft[Players::MaxPlayerCount];
    // The pie menus for each player
    PieMenuGUI *m_pPieMenu[Players::MaxPlayerCount];
    // The inventory menu gui for each player
    InventoryMenuGUI *m_InventoryMenuGUI[Players::MaxPlayerCount];
    // The in-game buy GUIs for each player
    BuyMenuGUI *m_pBuyGUI[Players::MaxPlayerCount];
    // The in-game scene editor GUI for each player
    SceneEditorGUI *m_pEditorGUI[Players::MaxPlayerCount];
    // The in-game important message banners for each player
    GUIBanner *m_pBannerRed[Players::MaxPlayerCount];
    GUIBanner *m_pBannerYellow[Players::MaxPlayerCount];
    // How many times a banner has been repeated.. so we dont' annoy by repeating forever
    int m_BannerRepeats[Players::MaxPlayerCount];
    // Whether each player has marked himself as ready to start. Can still edit while this is set, but when all are set, the game starts
    bool m_ReadyToStart[Players::MaxPlayerCount];
    // An override purchase list that can be set by a script and will be used instead of what's in the buy menu. Object held in here are NOT OWNED
    // Once a delivery is made with anything in here, this list is automatically cleared out, and the next delivery will be what's set in the buy menu.
    std::list<const SceneObject *> m_PurchaseOverride[Players::MaxPlayerCount];

    // The delivery queue which contains all the info about all the made orders currently in transit to delivery
    std::deque<Delivery> m_Deliveries[Teams::MaxTeamCount];
    // The box within where landing zones can be put
    Scene::Area m_LandingZoneArea[Teams::MaxTeamCount];
    // How wide around the brain the automatic LZ is following
    int m_BrainLZWidth[Players::MaxPlayerCount];
    // The objective points for each team
    std::list<ObjectivePoint> m_Objectives;

    // Tech of player
    std::string m_TeamTech[Teams::MaxTeamCount];

	// Initial gold amount selected by player in scenario setup dialog
	int m_StartingGold;
	// Whether fog of war was enabled or not in scenario setup dialog
	bool m_FogOfWarEnabled;
	// Whether we need a clear path to orbit to place brain
	bool m_RequireClearPathToOrbit;

	// Default fog of war switch state for this activity, default -1 (unspecified)
	int m_DefaultFogOfWar;
	// Default clear path to orbit switch value, default -1 (unspecified)
	int m_DefaultRequireClearPathToOrbit;
	// Default deploy units swutch value, default -1 (unspecified)
	int m_DefaultDeployUnits;
	// Default gold amount for different difficulties, defalt -1 (unspecified)
	int m_DefaultGoldCake;
	int m_DefaultGoldEasy;
	int m_DefaultGoldMedium;
	int m_DefaultGoldHard;
	int m_DefaultGoldNuts;
	// Whether those switches are enabled or disabled in scenario setup dialog, true by default
	bool m_FogOfWarSwitchEnabled;
	bool m_DeployUnitsSwitchEnabled;
	bool m_GoldSwitchEnabled;
	bool m_RequireClearPathToOrbitSwitchEnabled;
	bool m_BuyMenuEnabled;

    // The cursor animations for the LZ indicators
    std::vector<BITMAP *> m_aLZCursor[4];
    // The cursor animations for the objective indications
    std::vector<BITMAP *> m_aObjCursor[4];

    // Time it takes for a delivery to be made, in ms
    long m_DeliveryDelay;
    // Cursor animation timer
    Timer m_CursorTimer;
    // Total gameplay timer, not including editing phases
    Timer m_GameTimer;
    // Game end timer
    Timer m_GameOverTimer;
    // Time between game over and reset
    long m_GameOverPeriod;
    // The winning team number, when the game is over
    int m_WinnerTeam;

	std::vector<PieSlice *> m_CurrentPieMenuSlices;

	int m_CurrentPieMenuPlayer;

	std::string m_NetworkPlayerNames[Players::MaxPlayerCount];

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
