#ifndef _RTESPATIALPARTITIONGRID_
#define _RTESPATIALPARTITIONGRID_

//TODO Move Team enum into Constants so we can avoid including Activity here.
#include "Activity.h"

#include "Constants.h"

namespace RTE {

	class Box;
	struct IntRect;
	class MovableObject;

	/// <summary>
	/// A spatial partitioning grid, used to optimize MOID collision checks.
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
		/// <summary>
		/// Resets the spatial partitioning grid, removing everything from it
		/// </summary>
		void Reset();

		/// <summary>
		/// Adds the given MovableObject to this SpatialPartitionGrid.
		/// </summary>
		/// <param name="rect">A rectangle defining the space the MovableObject takes up.</param>
		/// <param name="mo">The MovableObject to add.</param>
		void Add(const IntRect &rect, const MovableObject &mo);

		/// <summary>
		/// Gets a vector of pointers to all MovableObjects within the given Box, who aren't of the ignored team.
		/// </summary>
		/// <param name="box">The Box to get MovableObjects within.</param>
		/// <param name="ignoreTeam">The team to ignore when getting MovableObjects.</param>
		/// <returns>A vector of pointers to all MovableObjects within the given Box, who aren't of the ignored team.</returns>
		const std::vector<MovableObject *> & GetMOsInBox(const Box &box, int ignoreTeam) const;

		/// <summary>
		/// Get a vector of pointers to all the MovableObjects within the specified radius of the given center point, who aren't of the ignored team.
		/// </summary>
		/// <param name="center">The center point to get MovableObjects around.</param>
		/// <param name="radius">The radius to get MovableObjects within.</param>
		/// <param name="ignoreTeam">The team to ignore when getting MovableObjects.</param>
		/// <returns>A vector of pointers to all the MovableObjects within the specified radius of the given center point, who aren't of the ignored team.</returns>
		const std::vector<MovableObject *> & GetMOsInRadius(const Vector &center, float radius, int ignoreTeam) const;

		/// <summary>
		/// Gets the MOIDs that are potentially overlapping the given X and Y Scene coordinates.
		/// </summary>
		/// <param name="x">The X coordinate to check.</param>
		/// <param name="y">The Y coordinate to check.</param>
		/// <param name="ignoreTeam">The team to ignore when getting MOIDs.</param>
		/// <returns>A vector of MOIDs that are potentially overlapping the x and y coordinates.</returns>
		const std::vector<int> & GetMOIDsAtPosition(int x, int y, int ignoreTeam) const;
#pragma endregion

	private:

		int m_Width; //!< The width of the SpatialPartitionGrid, in cells.
		int m_Height; //!< The height of the SpatialPartitionGrid, in cells.
		int m_CellSize; //!< The size of each of the SpatialPartitionGrid's cells, in pixels.

		// We store a list per team, so overlapping Actors don't waste loads of time collision checking against themselves.
		// Note that this is this list of MOIDs that are potentially colliding per team, so the list for team 1 contains the MOIDs for team 2, 3, 4, and no-team, as well as anything for team 1 that doesn't ignore team hits.
		std::array<std::vector<std::vector<int>>, Activity::MaxTeamCount + 1> m_Cells; //!< Array of cells for each team. The outside-vector is the vector of cells for the team, and each inside-vector entry contains all MOIDs in the cell's space that can collide with that team.

		std::unordered_set<int> m_UsedCellIds; //!< Set of used cell Ids, maintained to avoid wasting time looping through and clearing unused cells.

		/// <summary>
		/// Gets the Id of the cell at the given SpatialPartitionGrid coordinates, automatically accounting for wrapping.
		/// </summary>
		/// <param name="cellX">The x coordinate of the cell to get the Id of.</param>
		/// <param name="cellY">The y coordinate of the cell to get the Id of.</param>
		/// <returns>The Id of the cell at the given SpatialPartitionGrid coordinates.</returns>
		int GetCellIdForCellCoords(int cellX, int cellY) const;

		/// <summary>
		/// Clears all the member variables of this SpatialPartitionGrid.
		/// </summary>
		void Clear();

		// Disallow the use of an implicit method.
		SpatialPartitionGrid(const SpatialPartitionGrid &reference) = delete;
	};
}
#endif