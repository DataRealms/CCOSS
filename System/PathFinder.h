#ifndef _RTEPATHFINDER_
#define _RTEPATHFINDER_

#include "Box.h"
#include "System/MicroPather/micropather.h"

using namespace micropather;

namespace RTE {

	class Scene;
	class Material;

	/// <summary>
	/// Contains everything related to a PathNode on the path grid used by PathFinder.
	/// </summary>
	struct PathNode {

		static constexpr int c_MaxAdjacentNodeCount = 8; //!< The maximum number of adjacent PathNodes to any given PathNode. Thusly, also the number of directions for PathNodes to be in.

		Vector Pos; //!< Absolute position of the center of this PathNode in the scene.

		/// <summary>
		/// Pointers to all adjacent PathNodes, in clockwise order with top first. These are not owned, and may be 0 if adjacent to non-wrapping scene border.
		/// </summary>
		std::array<PathNode *, c_MaxAdjacentNodeCount> AdjacentNodes;
		PathNode *&Up = AdjacentNodes[0];
		PathNode *&UpRight = AdjacentNodes[1];
		PathNode *&Right = AdjacentNodes[2];
		PathNode *&RightDown = AdjacentNodes[3];
		PathNode *&Down = AdjacentNodes[4];
		PathNode *&DownLeft = AdjacentNodes[5];
		PathNode *&Left = AdjacentNodes[6];
		PathNode *&LeftUp = AdjacentNodes[7];

		/// <summary>
		/// The strongest material between us and our adjacent PathNodes, in clockwise order with top first.
		/// </summary>
		std::array<const Material *, c_MaxAdjacentNodeCount> AdjacentNodeBlockingMaterials;
		const Material *&UpMaterial = AdjacentNodeBlockingMaterials[0];
		const Material *&UpRightMaterial = AdjacentNodeBlockingMaterials[1];
		const Material *&RightMaterial = AdjacentNodeBlockingMaterials[2];
		const Material *&RightDownMaterial = AdjacentNodeBlockingMaterials[3];
		const Material *&DownMaterial = AdjacentNodeBlockingMaterials[4];
		const Material *&DownLeftMaterial = AdjacentNodeBlockingMaterials[5];
		const Material *&LeftMaterial = AdjacentNodeBlockingMaterials[6];
		const Material *&LeftUpMaterial = AdjacentNodeBlockingMaterials[7];

		/// <summary>
		/// Constructor method used to instantiate a PathNode object in system memory and make it ready for use.
		/// </summary>
		/// <param name="pos">Absolute position of the center of the PathNode in the scene.</param>
		explicit PathNode(const Vector &pos);
	};

	/// <summary>
	/// A class encapsulating and implementing the MicroPather A* pathfinding library.
	/// </summary>
	class PathFinder : public Graph {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PathFinder object.
		/// </summary>
		/// <param name="nodeDimension">The width and height in scene pixels that of each PathNode should represent.</param>
		/// <param name="allocate">The block size that the PathNode cache is allocated from. Should be about a fourth of the total number of PathNodes.</param>
		PathFinder(int nodeDimension) { Clear(); Create(nodeDimension); }

		/// <summary>
		/// Makes the PathFinder object ready for use.
		/// </summary>
		/// <param name="nodeDimension">The width and height in scene pixels that of each PathNode should represent.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(int nodeDimension);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PathFinder object before deletion.
		/// </summary>
		~PathFinder() override { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) this PathFinder object.
		/// </summary>
		void Destroy();

		/// <summary>
		/// Resets the entire PathFinder object to the default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region PathFinding
		/// <summary>
		/// Calculates and returns the least difficult path between two points on the current scene.
		/// </summary>
		/// <param name="start">Start positions on the scene to find the path between.</param>
		/// <param name="end">End positions on the scene to find the path between.</param>
		/// <param name="pathResult">A list which will be filled out with waypoints between the start and end.</param>
		/// <param name="totalCostResult">The total minimum difficulty cost calculated between the two points on the scene.</param>
		/// <param name="digStrength">What material strength the search is capable of digging through.</param>
		/// <returns>Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.</returns>
		int CalculatePath(Vector start, Vector end, std::list<Vector> &pathResult, float &totalCostResult, float digStrength = 1);

		/// <summary>
		/// Recalculates all the costs between all the PathNodes by tracing lines in the material layer and summing all the material strengths for each encountered pixel. Also resets the pather itself.
		/// </summary>
		void RecalculateAllCosts();

		/// <summary>
		/// Recalculates the costs between all the PathNodes touching a deque of specific rectangular areas (which will be wrapped). Also resets the pather itself, if necessary.
		/// </summary>
		/// <param name="boxList">The deque of Boxes representing the updated areas.</param>
		/// <param name="nodeUpdateLimit">The maximum number of PathNodes we'll try to update this frame. True PathNode update count can be higher if we received a big box, as we always do at least 1 box.</param>
		/// <returns>The set of PathNode ids that were updated.</returns>
		std::vector<int> RecalculateAreaCosts(std::deque<Box> &boxList, int nodeUpdateLimit);

		/// <summary>
		/// Updates a set of PathNodes, adjusting their transitions.
		/// This does NOT update the pather, which is required if PathNode costs changed.
		/// </summary>
		/// <param name="nodeVec">The set of PathNode IDs to update.</param>
		/// <returns>Whether any PathNode costs changed.</returns>
		bool UpdateNodeList(const std::vector<int> &nodeVec);

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the least possible cost to get from PathNode A to B, if it all was air.
		/// </summary>
		/// <param name="startState">Pointer to PathNode to start from. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="endState">PathNode to end up at. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <returns>The cost of the absolutely fastest possible way between the two points, as if traveled through air all the way.</returns>
		float LeastCostEstimate(void *startState, void *endState) override;

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the cost to go to any adjacent PathNode of the one passed in.
		/// </summary>
		/// <param name="state">Pointer to PathNode to get to cost of all adjacents for. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="adjacentList">An empty vector which will be filled out with all the valid PathNodes adjacent to the one passed in. If at non-wrapping edge of seam, those non existent PathNodes won't be added.</param>
		void AdjacentCost(void *state, std::vector<micropather::StateCost> *adjacentList) override;
#pragma endregion

#pragma region Misc
		/// <summary>
		/// Implementation of the abstract interface of Graph. This function is only used in DEBUG mode - it dumps output to stdout.
		/// Since void* aren't really human readable, this will print out some concise info without an ending newline.
		/// </summary>
		/// <param name="state">The state to print out info about.</param>
		void PrintStateInfo(void *state) override {}
#pragma endregion

	private:

		static constexpr float c_NodeCostChangeEpsilon = 5.0F; //!< The minimum change in a PathNodes's cost for the pathfinder to recognize a change and reset itself. This is so minor changes (e.g. blood particles) don't force constant pathfinder resets.

		MicroPather *m_Pather; //!< The actual pathing object that does the pathfinding work. Owned.
		std::vector<PathNode> m_NodeGrid;  //!< The array of PathNodes representing the grid on the scene.
		unsigned int m_NodeDimension; //!< The width and height of each PathNode, in pixels on the scene.
		int m_GridWidth; //!< The width of the pathing grid, in PathNodes.
		int m_GridHeight; //!< The height of the pathing grid, in PathNodes.
		bool m_WrapsX; //!< Whether the pathing grid wraps on the X axis.
		bool m_WrapsY; //!< Whether the pathing grid wraps on the Y axis.

		/// <summary>
		/// Gets the pather for this thread. Lazily-initialized for each new thread that needs a pather.
		/// </summary>
		/// <returns>The pather for this thread.</returns>
		MicroPather * GetPather();

#pragma region Path Cost Updates
		/// <summary>
		/// Helper function for getting the strongest material we need to path though between PathNodes.
		/// </summary>
		/// <param name="start">Origin point.</param>
		/// <param name="end">Destination point.</param>
		/// <returns>The strongest material.</returns>
		const Material * StrongestMaterialAlongLine(const Vector &start, const Vector &end) const;

		/// <summary>
		/// Helper function for updating all the values of cost edges going out from a specific PathNodes.
		/// This does NOT update the pather, which is required before solving more paths after calling this.
		/// </summary>
		/// <param name="node">The PathNode to update all costs of. It's safe to pass nullptr here. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <returns>Whether the PathNodes costs changed.</returns>
		bool UpdateNodeCosts(PathNode *node) const;

		/// <summary>
		/// Helper function for getting the PathNode ids in a Box.
		/// </summary>
		/// <param name="box">The Box of which all PathNodes it touches should be returned.</param>
		/// <returns>A list of the PathNode ids inside the box.</returns>
		std::vector<int> GetNodeIdsInBox(Box box);

		/// <summary>
		/// Gets the cost for transitioning through this Material.
		/// </summary>
		/// <param name="material">The Material to get the transition cost for.</param>
		/// <returns>The transition cost for the Material.</returns>
		float GetMaterialTransitionCost(const Material &material) const;

		/// <summary>
		/// Gets the average cost for all transitions out of this PathNode, ignoring infinities/unpathable transitions.
		/// </summary>
		/// <param name="node">The PathNode to get the average transition cost for.</param>
		/// <returns>The average transition cost.</returns>
		float GetNodeAverageTransitionCost(const PathNode &node) const;
#pragma endregion

		/// <summary>
		/// Gets the PathNode at the given coordinates.
		/// </summary>
		/// <param name="x">The X coordinate, in PathNodes.</param>
		/// <param name="y">The Y coordinate, in PathNodes.</param>
		/// <returns>The PathNode at the given coordinates.</returns>
		PathNode * GetPathNodeAtGridCoords(int x, int y);

		/// <summary>
		/// Gets the PathNode id at the given coordinates.
		/// </summary>
		/// <param name="x">The X coordinate, in PathNodes.</param>
		/// <param name="y">The Y coordinate, in PathNodes.</param>
		/// <returns>The PathNode id at the given coordinates.</returns>
		int ConvertCoordsToNodeId(int x, int y);

		/// <summary>
		/// Clears all the member variables of this PathFinder, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PathFinder(const PathFinder &reference) = delete;
		PathFinder & operator=(const PathFinder &rhs) = delete;
	};
}
#endif