#ifndef _RTEMETAPLAYER_
#define _RTEMETAPLAYER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MetaPlayer.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MetaPlayer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <string>
#include <vector>
#include "Entity.h"
#include "ContentFile.h"
#include "Icon.h"


namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Holds data for a Metagame player aka "Tech" or "House"
// Parent(s):       Entity
// Class history:   11/14/2009 MetaPlayer created.

class MetaPlayer:
    public Entity
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

// Give direct access to the GUI
friend class MetagameGUI;
// Concrete allocation and cloning definitions
ENTITYALLOCATION(MetaPlayer)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MetaPlayer object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MetaPlayer() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MetaPlayer object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~MetaPlayer() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MetaPlayer object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MetaPlayer to be identical to another, by deep copy.
// Arguments:       A reference to the MetaPlayer to deep copy.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const MetaPlayer &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MetaPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a MetaPlayer object
//                  identical to an already existing one.
// Arguments:       A MetaPlayer object which is passed in by reference.

    MetaPlayer(const MetaPlayer &reference) { if (this != &reference) { Clear(); Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        MetaPlayer assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one MetaPlayer equal to another.
// Arguments:       A MetaPlayer reference.
// Return value:    A reference to the changed MetaPlayer.

    MetaPlayer & operator=(const MetaPlayer &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }


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
// Description:     Resets the entire MetaPlayer, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this MetaPlayer to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the MetaPlayer will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MetaPlayer object.
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
// Method:          SetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of the metagame player
// Arguments:       The new name to set.
// Return value:    None.

    void SetName(std::string newName) { m_Name = newName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the player.
// Arguments:       None.
// Return value:    The name of the player.

    std::string GetName() const { return m_Name; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the Team of the metagame player
// Arguments:       The new team to set.
// Return value:    None.

    void SetTeam(int newTeam) { m_Team = newTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Team of this player.
// Arguments:       None.
// Return value:    The Team of this player.

    int GetTeam() const { return m_Team; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAggressiveness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the normalized aggressiveness scalar of this player if an AI.
// Arguments:       The new aggressiveness scalar, 0 min to 1.0 max
// Return value:    None.

    void SetAggressiveness(float aggressiveness) { m_Aggressiveness = aggressiveness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAggressiveness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the normalized aggressiveness scalar of this player if an AI.
// Arguments:       None.
// Return value:    The current aggressiveness scalar, 0 min to 1.0 max

    float GetAggressiveness() const { return m_Aggressiveness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNativeTechModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets ID of the DataModule that this player is native to.
// Arguments:       None.
// Return value:    The ID of the DataModule this is native to.

    int GetNativeTechModule() const { return m_NativeTechModule; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetForeignCostMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the multiplier of costs of any Tech items foreign to this.
// Arguments:       None.
// Return value:    The scalar multiplier of all costs of foreign tech items.

    float GetForeignCostMultiplier() const { return m_ForeignCostMult; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetNativeCostMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the multiplier of costs of any Tech items native to this.
// Arguments:       None.
// Return value:    The scalar multiplier of all costs of native tech items.

    float GetNativeCostMultiplier() const { return m_NativeCostMult; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNativeCostMultiplier
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the multiplier of costs of any Tech items native to this.
// Arguments:       The scalar multiplier of all costs of native tech items.
// Return value:    None.

    void SetNativeCostMultiplier(float newNativeCostMult) { m_NativeCostMult = newNativeCostMult; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBrainsInTransit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of brains in this player that are out on the move
//                  between the pool and sites.
// Arguments:       None.
// Return value:    The number of brains that are out in transit.

    int GetBrainsInTransit() const { return m_BrainsInTransit; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBrainsInTransit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the number of brains in this player that are out on the move
//                  between the pool and sites.
// Arguments:       None.
// Return value:    The number of brains that are out in transit.

    void SetBrainsInTransit(int transitCount) { m_BrainsInTransit = transitCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeBrainsInTransit
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Alters the number of brains of this player which are travelling.
// Arguments:       The number of brains to add or remove from transit.
// Return value:    The resulting count after the alteration.

    int ChangeBrainsInTransit(int change) { return m_BrainsInTransit += change; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBrainPoolCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of brains in this player's brain pool.
// Arguments:       None.
// Return value:    The number of brains that are available for deployment.

    int GetBrainPoolCount() const { return m_BrainPool; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBrainPoolCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the number of brains in this player's brain pool.
// Arguments:       The number of brains that should be available for deployment.
// Return value:    None.

    void SetBrainPoolCount(int brainCount) { m_BrainPool = brainCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeBrainPoolCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Alters the number of brains in this player's brain pool.
// Arguments:       The number of brains to add or remove from the pool.
// Return value:    The resulting count after the alteration.

    int ChangeBrainPoolCount(int change) { return m_BrainPool += change; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount of funds this player currently has in the game.
// Arguments:       The new funds value for this player.
// Return value:    None.

    void SetFunds(float newFunds) { m_Funds = newFunds; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount of funds this player currently has in the game.
// Arguments:       None.
// Return value:    A float with the funds tally for the requested player.

    float GetFunds() const { return m_Funds; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffensiveBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offensive budget of this Player for this round, in oz.
// Arguments:       The new offensive budget, in oz.
// Return value:    None.

    void SetOffensiveBudget(float newBudget) { m_OffensiveBudget = newBudget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffensiveBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offensive budget of this Player for this round, in oz.
// Arguments:       None.
// Return value:    The offensive budget, in oz.

    float GetOffensiveBudget() const { return m_OffensiveBudget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffensiveTargetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of the scene this player is targeting for offensive.
// Arguments:       The name of the Scene this is targeting.
// Return value:    None.

    void SetOffensiveTargetName(std::string targetName) { m_OffensiveTarget = targetName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffensiveTargetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the scene this player is targeting for offensive.
// Arguments:       None.
// Return value:    The name of the Scene this is targeting.

    std::string GetOffensiveTargetName() const { return m_OffensiveTarget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes this player's funds level by a certain amount.
// Arguments:       The amount with which to change the funds balance.
// Return value:    The new amount of funds of this player.

    float ChangeFunds(float howMuch) { return m_Funds += howMuch; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SpendFunds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Decreases this player's funds level by a certain absolute amount, and
//                  returns that difference as a positive value. If the amount isn't
//                  available to spend in the funds, the rest of the funds are spent and
//                  only that amount is returned.
// Arguments:       The amount with which to change the funds balance. This should be a
//                  positive value to decrease the funds amount.
// Return value:    The amount of funds that were spent.

    float SpendFunds(float howMuch) { howMuch = MIN(m_Funds, howMuch); m_Funds -= howMuch; return howMuch; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whehter this is human-controlled or not (AI)
// Arguments:       Whether human controlled.
// Return value:    None.

    void SetHuman(bool human) { m_Human = human; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsHuman
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether human-controlled or not.
// Arguments:       None.
// Return value:    Whether human controlled

    bool IsHuman() const { return m_Human; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGameOverRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which round this player lost out on. If set to negative, it means
//                  he's still in the game.
// Arguments:       The round he lost out on.
// Return value:    None.

    void SetGameOverRound(int gameOverRound) { m_GameOverRound = gameOverRound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGameOverRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates which round this player made it to. If negative, he is still
//                  in the game.
// Arguments:       None.
// Return value:    Which round he made it to

    int GetGameOverRound() const { return m_GameOverRound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsGameOverByRound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this player is out of the game on or before a particular
//                  round of the current metagame.
// Arguments:       Which round to check against.
// Return value:    Whether the player was flagged as being out of the game on that or any
//                  earlier round.

    bool IsGameOverByRound(int whichRound) const { return m_GameOverRound >= 0 && m_GameOverRound <= whichRound; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetInGamePlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows which in-game player controls this metagame player is mapped to.
// Arguments:       None.
// Return value:    The in-game player number this is mapped to.

    int GetInGamePlayer() const { return m_InGamePlayer; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


    static Entity::ClassInfo m_sClass;
    // The name of the player
    std::string m_Name;
    // The team index of this player
    int m_Team;
    // Human-controlled player?
    bool m_Human;
    // Which round this player ended up getting to. If negative, he's still in the game
    int m_GameOverRound;
    // The in-game player controls that this metaplayer is mapped to
    int m_InGamePlayer;
    // Normalized aggressiveness temperament of this if it's an AI player. 0 is min, 1.0 max
    float m_Aggressiveness;
    // The tech module that will have native pricing for this player
    int m_NativeTechModule;
    // The cost multiplier of native groups available to this player
    float m_NativeCostMult;
    // The cost multiplier of foreign groups available to this player
    float m_ForeignCostMult;
    // The current number of unused brains left available to this player in the pool
    int m_BrainPool;
    // The current number of brains that are travelling to or from a site;
    // this is only for UI purposes to show the player what's happening, it's just a runtime var - don't save it
    int m_BrainsInTransit;
    // The current funds of this player, in oz
    float m_Funds;
    // The previous round's funds of this player, in oz.. no need to save this, a runtime var
    float m_PhaseStartFunds;
    // The money allocated by this player this round to attack or explore a specific site
    float m_OffensiveBudget;
    // Name of the Scene this player is targeting for its offensive this round.
    std::string m_OffensiveTarget;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MetaPlayer, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

void Clear();

};

} // namespace RTE

#endif // File