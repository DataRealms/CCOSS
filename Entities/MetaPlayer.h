#ifndef _RTEMETAPLAYER_
#define _RTEMETAPLAYER_

#include "Entity.h"

namespace RTE {

	/// <summary>
	/// Holds data for a Metagame player aka "Tech" or "House".
	/// </summary>
	class MetaPlayer : public Entity {
		friend class MetagameGUI;

	public:

		EntityAllocation(MetaPlayer)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MetaPlayer object in system memory. Create() should be called before using the object.
		/// </summary>
		MetaPlayer() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a MetaPlayer object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A MetaPlayer object which is passed in by reference.</param>
		MetaPlayer(const MetaPlayer &reference) { if (this != &reference) { Clear(); Create(reference); } }

		/// <summary>
		/// Makes the MetaPlayer object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return Entity::Create(); }

		/// <summary>
		/// Creates a MetaPlayer to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the MetaPlayer to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const MetaPlayer &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a MetaPlayer object before deletion from system memory.
		/// </summary>
		~MetaPlayer() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the MetaPlayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire MetaPlayer, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the name of the MetaPlayer.
		/// </summary>
		/// <returns>The name of the player.</returns>
		std::string GetName() const { return m_Name; }

		/// <summary>
		/// Sets the name of the MetaPlayer.
		/// </summary>
		/// <param name="newName">The new name to set.</param>
		void SetName(std::string newName) { m_Name = newName; }

		/// <summary>
		/// Gets the Team of this MetaPlayer.
		/// </summary>
		/// <returns>The Team of this player.</returns>
		int GetTeam() const { return m_Team; }

		/// <summary>
		/// Sets the Team of the MetaPlayer.
		/// </summary>
		/// <param name="newTeam">The new team to set.</param>
		void SetTeam(int newTeam) { m_Team = newTeam; }

		/// <summary>
		/// Indicates whether this MetaPlayer is human controlled or not (AI).
		/// </summary>
		/// <returns>Whether this player is human controlled.</returns>
		bool IsHuman() const { return m_Human; }

		/// <summary>
		/// Sets whether this MetaPlayer is human controlled or not (AI).
		/// </summary>
		/// <param name="human">Whether this MetaPlayer is human controlled or not.</param>
		void SetHuman(bool human) { m_Human = human; }

		/// <summary>
		/// Shows which in-game player controls this MetaPlayer is mapped to.
		/// </summary>
		/// <returns>The in-game player number this is mapped to.</returns>
		int GetInGamePlayer() const { return m_InGamePlayer; }

		/// <summary>
		/// Gets ID of the DataModule that this MetaPlayer is native to.
		/// </summary>
		/// <returns>The ID of the DataModule this is native to.</returns>
		int GetNativeTechModule() const { return m_NativeTechModule; }

		/// <summary>
		/// Gets the normalized aggressiveness scalar of this player if an AI.
		/// </summary>
		/// <returns>The current aggressiveness scalar, 0 min to 1.0 max.</returns>
		float GetAggressiveness() const { return m_Aggressiveness; }

		/// <summary>
		/// Sets the normalized aggressiveness scalar of this player if an AI.
		/// </summary>
		/// <param name="aggressiveness">The new aggressiveness scalar, 0 min to 1.0 max.</param>
		void SetAggressiveness(float aggressiveness) { m_Aggressiveness = aggressiveness; }

		/// <summary>
		/// Indicates which round this MetaPlayer made it to. If negative, he is still in the game.
		/// </summary>
		/// <returns>Which round the MetaPlayer made it to.</returns>
		int GetGameOverRound() const { return m_GameOverRound; }

		/// <summary>
		/// Sets which round this MetaPlayer lost out on. If set to negative, it means he's still in the game.
		/// </summary>
		/// <param name="gameOverRound">The round the MetaPlayer lost out on.</param>
		void SetGameOverRound(int gameOverRound) { m_GameOverRound = gameOverRound; }

		/// <summary>
		/// Tells whether this MetaPlayer is out of the game on or before a particular round of the current metagame.
		/// </summary>
		/// <param name="whichRound">Which round to check against.</param>
		/// <returns>Whether the MetaPlayer was flagged as being out of the game on that or any earlier round.</returns>
		bool IsGameOverByRound(int whichRound) const { return m_GameOverRound >= 0 && m_GameOverRound <= whichRound; }

		/// <summary>
		/// Gets the name of the scene this MetaPlayer is targeting for offensive.
		/// </summary>
		/// <returns>The name of the Scene this MetaPlayer is targeting.</returns>
		std::string GetOffensiveTargetName() const { return m_OffensiveTarget; }

		/// <summary>
		/// Sets the name of the scene this MetaPlayer is targeting for offensive.
		/// </summary>
		/// <param name="targetName">The name of the Scene this MetaPlayer is targeting.</param>
		void SetOffensiveTargetName(std::string targetName) { m_OffensiveTarget = targetName; }
#pragma endregion

#pragma region Funds and Costs
		/// <summary>
		/// Gets the amount of funds this MetaPlayer currently has in the game.
		/// </summary>
		/// <returns>A float with the funds tally for the requested MetaPlayer.</returns>
		float GetFunds() const { return m_Funds; }

		/// <summary>
		/// Sets the amount of funds this MetaPlayer currently has in the game.
		/// </summary>
		/// <param name="newFunds">The new funds value for this MetaPlayer.</param>
		void SetFunds(float newFunds) { m_Funds = newFunds; }

		/// <summary>
		/// Changes this MetaPlayer's funds level by a certain amount.
		/// </summary>
		/// <param name="howMuch">The amount with which to change the funds balance.</param>
		/// <returns>The new amount of funds of this MetaPlayer.</returns>
		float ChangeFunds(float howMuch) { return m_Funds += howMuch; }

		/// <summary>
		/// Decreases this player's funds level by a certain absolute amount, and returns that difference as a positive value.
		/// If the amount isn't available to spend in the funds, the rest of the funds are spent and only that amount is returned.
		/// </summary>
		/// <param name="howMuch">The amount with which to change the funds balance. This should be a positive value to decrease the funds amount.</param>
		/// <returns>The amount of funds that were spent.</returns>
		float SpendFunds(float howMuch) { howMuch = std::min(m_Funds, howMuch); m_Funds -= howMuch; return howMuch; }

		/// <summary>
		/// Gets the offensive budget of this MetaPlayer for this round, in oz.
		/// </summary>
		/// <returns>The offensive budget, in oz.</returns>
		float GetOffensiveBudget() const { return m_OffensiveBudget; }

		/// <summary>
		/// Sets the offensive budget of this MetaPlayer for this round, in oz.
		/// </summary>
		/// <param name="newBudget">The new offensive budget, in oz.</param>
		void SetOffensiveBudget(float newBudget) { m_OffensiveBudget = newBudget; }

		/// <summary>
		/// Gets the multiplier of costs of any Tech items foreign to this MetaPlayer.
		/// </summary>
		/// <returns>The scalar multiplier of all costs of foreign tech items.</returns>
		float GetForeignCostMultiplier() const { return m_ForeignCostMult; }

		/// <summary>
		/// Gets the multiplier of costs of any Tech items native to this MetaPlayer.
		/// </summary>
		/// <returns>The scalar multiplier of all costs of native tech items.</returns>
		float GetNativeCostMultiplier() const { return m_NativeCostMult; }

		/// <summary>
		/// Sets the multiplier of costs of any Tech items native to this MetaPlayer.
		/// </summary>
		/// <param name="newNativeCostMult">The scalar multiplier of all costs of native tech items.</param>
		void SetNativeCostMultiplier(float newNativeCostMult) { m_NativeCostMult = newNativeCostMult; }
#pragma endregion

#pragma region Brain Pool
		/// <summary>
		/// Gets the number of brains in this MetaPlayer's brain pool.
		/// </summary>
		/// <returns>The number of brains that are available for deployment.</returns>
		int GetBrainPoolCount() const { return m_BrainPool; }

		/// <summary>
		/// Sets the number of brains in this MetaPlayer's brain pool.
		/// </summary>
		/// <param name="brainCount">The number of brains that should be available for deployment.</param>
		void SetBrainPoolCount(int brainCount) { m_BrainPool = brainCount; }

		/// <summary>
		/// Alters the number of brains in this MetaPlayer's brain pool.
		/// </summary>
		/// <param name="change">The number of brains to add or remove from the pool.</param>
		/// <returns>The resulting count after the alteration.</returns>
		int ChangeBrainPoolCount(int change) { return m_BrainPool += change; }

		/// <summary>
		/// Gets the number of brains in this MetaPlayer that are out on the move between the pool and sites.
		/// </summary>
		/// <returns>The number of brains that are out in transit.</returns>
		int GetBrainsInTransit() const { return m_BrainsInTransit; }

		/// <summary>
		/// Sets the number of brains in this MetaPlayer that are out on the move between the pool and sites.
		/// </summary>
		/// <param name="transitCount">The number of brains that are out in transit.</param>
		void SetBrainsInTransit(int transitCount) { m_BrainsInTransit = transitCount; }

		/// <summary>
		/// Alters the number of brains of this MetaPlayer which are traveling.
		/// </summary>
		/// <param name="change">The number of brains to add or remove from transit.</param>
		/// <returns>The resulting count after the alteration.</returns>
		int ChangeBrainsInTransit(int change) { return m_BrainsInTransit += change; }
#pragma endregion

#pragma region Operator Overloads
		/// <summary>
		/// An assignment operator for setting one MetaPlayer equal to another.
		/// </summary>
		/// <param name="rhs">A MetaPlayer reference.</param>
		/// <returns>A reference to the changed MetaPlayer.</returns>
		MetaPlayer & operator=(const MetaPlayer &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::string m_Name; //!< The name of the player.

		int m_Team; //!< The team index of this player.
		bool m_Human; //!< Whether the player is human controlled.
		int m_InGamePlayer; //!< The in-game player controls that this MetaPlayer is mapped to.
		float m_Aggressiveness; //!< Normalized aggressiveness temperament of this if it's an AI player. 0 is min, 1.0 max.

		int m_GameOverRound; //!< Which round this player ended up getting to. If negative, he's still in the game.

		int m_NativeTechModule; //!< The tech module that will have native pricing for this player.
		float m_NativeCostMult; //!< The cost multiplier of native groups available to this player.
		float m_ForeignCostMult; //!< The cost multiplier of foreign groups available to this player.

		int m_BrainPool; //!< The current number of unused brains left available to this player in the pool.
		int m_BrainsInTransit; //!< The current number of brains that are traveling to or from a site; this is only for UI purposes to show the player what's happening, it's just a runtime var - don't save it.

		float m_Funds; //!< The current funds of this player, in oz.
		float m_PhaseStartFunds; //!< The previous round's funds of this player, in oz. No need to save this, a runtime var.
		float m_OffensiveBudget; //!< The money allocated by this player this round to attack or explore a specific site

		std::string m_OffensiveTarget; //!< Name of the Scene this player is targeting for its offensive this round.

	private:

		/// <summary>
		/// Clears all the member variables of this MetaPlayer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif