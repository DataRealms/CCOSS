#ifndef _RTEMETASAVE_
#define _RTEMETASAVE_

#include "Entity.h"

namespace RTE {

	/// <summary>
	/// A light-weight Entity for storing only the necessary info about how to load an entire MetaMan state from disk.
	/// </summary>
	class MetaSave : public Entity {

	public:

		EntityAllocation(MetaSave);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MetaSave object in system memory. Create() should be called before using the object.
		/// </summary>
		MetaSave() { Clear(); }

		/// <summary>
		/// Makes the MetaSave object ready for use from the currently loaded MetaMan state.
		/// </summary>
		/// <param name="savePath">The path of the file to where the MetaMan state should be saved.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(std::string savePath);

		/// <summary>
		/// Creates a MetaSave to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the MetaSave to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const MetaSave &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a MetaSave object before deletion from system memory.
		/// </summary>
		~MetaSave() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the MetaSave object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { Entity::Destroy(); } Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the full path to the ini file that stores the state of the MetaMan this is associated with.
		/// </summary>
		/// <returns>The path to the ini with the MetaMan state info.</returns>
		std::string GetSavePath() const { return m_SavePath; }

		/// <summary>
		/// Gets the total number of players this game has (including AIs).
		/// </summary>
		/// <returns>The player count.</returns>
		int GetPlayerCount() const { return m_PlayerCount; }

		/// <summary>
		/// Gets the difficulty for this game.
		/// </summary>
		/// <returns>Difficulty setting.</returns>
		int GetDifficulty() const { return m_Difficulty; }

		/// <summary>
		/// Gets the round number that this game is on.
		/// </summary>
		/// <returns>The round count.</returns>
		int GetRoundCount() const { return m_RoundCount; }

		/// <summary>
		/// Gets the total number of Scenes this game has.
		/// </summary>
		/// <returns>The number of Scenes.</returns>
		int GetSiteCount() const { return m_SiteCount; }
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
	
		std::string m_SavePath; //!< The full path to the ini file which stores the stat of MetaMan this is associated with.
		int m_PlayerCount; //!< The number of players in this saved game.
		int m_Difficulty; //!< Game difficulty.
		int m_RoundCount; //!< The round this game is on.
		int m_SiteCount; //!< The site count of this game.

	private:

		/// <summary>
		/// Clears all the member variables of this MetaSave, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		MetaSave(const MetaSave &reference) = delete;
		MetaSave & operator=(const MetaSave &rhs) = delete;
	};
}
#endif