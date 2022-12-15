#ifndef _RTESPATIALPARTITIONGRID_
#define _RTESPATIALPARTITIONGRID_

// For Teams :(
#include "Activity.h"

#include "Constants.h"

namespace RTE {

	class Box;
	struct IntRect;
	class MovableObject;

	/// <summary>
	/// A spatial partitioning grid, used to optimize MOID collision checks
	/// </summary>
	class SpatialPartitionGrid {

	public:
#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SpatialPartitionGrid object.
		/// </summary>
		SpatialPartitionGrid() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a SpatialPartitionGrid object.
		/// </summary>
		SpatialPartitionGrid(int width, int height, int cellSize) { Clear(); Create(width, height, cellSize); }

		/// <summary>
		/// Copy constructor method used to instantiate a SpatialPartitionGrid object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A SpatialPartitionGrid object which is passed in by reference.</param>
		SpatialPartitionGrid(const SpatialPartitionGrid &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Makes the SpatialPartitionGrid object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(int width, int height, int cellSize);

		/// <summary>
		/// Creates a SpatialPartitionGrid to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SpatialPartitionGrid to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SpatialPartitionGrid &reference);
#pragma endregion

#pragma region Grid Management
		typedef std::vector<MOID> MOIDList;

		/// <summary>
		/// Resets the spatial partitioning grid, removing everything from it
		/// </summary>
		void Reset();

		/// <summary>
		/// Adds a MOID to the spatial partitioning grid
		/// </summary>
		void Add(const IntRect &rect, const MovableObject &mo);

		/// <summary>
		/// Get MOIDs that are within a box
		/// </summary>
		const std::vector<MovableObject *> & GetMOsInBox(const Box &box, int ignoreTeam) const;

		/// <summary>
		/// Get MOIDs that are within a radius
		/// </summary>
		const std::vector<MovableObject *> & GetMOsInRadius(const Vector &centre, float radius, int ignoreTeam) const;

		/// <summary>
		/// Get MOIDs that are potentially overlapping a pixel
		/// </summary>
		const SpatialPartitionGrid::MOIDList & GetMOIDsAtPosition(int x, int y, int ignoreTeam) const;
#pragma endregion

	private:
		int GetCellIdForCellCoords(int cellX, int cellY) const;

		int m_Width; // Grid width in cells
		int m_Height; // Grid height in cells
		int m_CellSize; // Cell size, in pixels

		// We store a list per team, so overlapping actors don't waste loads of time collision checking against themselves
		// Note, this is this list of moids that are potentially colliding per team, not the list of MOIDS per team
		// So the list for team 1 includes the MOIDs for team 2, 3, 4, and no-team
		typedef std::array<std::vector<MOIDList>, Activity::MaxTeamCount+1> CellsPerTeam;
		CellsPerTeam m_Cells;

		// We also don't want to waste loads of time looping through and unclearing unused cells, so store a set of used cells
		std::unordered_set<int> m_UsedCells;

		/// <summary>
		/// Clears all the member variables of this SpatialPartitionGrid, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif