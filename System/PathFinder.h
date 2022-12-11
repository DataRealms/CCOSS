#ifndef _RTEPATHFINDER_
#define _RTEPATHFINDER_

#include "Box.h"
#include "System/MicroPather/micropather.h"

using namespace micropather;

namespace RTE {

	class Scene;
	class Material;

	/// <summary>
	/// Contains everything related to a node on the path grid used by PathFinder.
	/// </summary>
	struct PathNode {

		static constexpr int c_MaxAdjacentNodeCount = 8; //!< The maximum number of adjacent nodes to any given node. Thusly, also the number of directions for nodes to be in.

		Vector Pos; //!< Absolute position of the center of this node in the scene.

		/// <summary>
		/// Pointers to all adjacent nodes, in clockwise order with top first. These are not owned, and may be 0 if adjacent to non-wrapping scene border.
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
		/// The strongest material between us and our adjacent nodes, in clockwise order with top first.
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

		explicit PathNode(const Vector& pos);
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
		/// <param name="pScene">The scene to be pathing within.</param>
		/// <param name="nodeDimension">The width and height in scene pixels that of each node should represent.</param>
		/// <param name="allocate">The block size that the node cache is allocated from. Should be about a fourth of the total number of nodes.</param>
		PathFinder(Scene *scene, int nodeDimension, unsigned int allocate) { Clear(); Create(scene, nodeDimension, allocate); }

		/// <summary>
		/// Makes the PathFinder object ready for use.
		/// </summary>
		/// <param name="pScene">The scene to be pathing within.</param>
		/// <param name="nodeDimension">The width and height in scene pixels that of each node should represent.</param>
		/// <param name="allocate">The block size that the node cache is allocated from. Should be about a fourth of the total number of nodes.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Scene *scene, int nodeDimension, unsigned int allocate);
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
		/// Recalculates all the costs between all the nodes by tracing lines in the material layer and summing all the material strengths for each encountered pixel. Also resets the pather itself.
		/// </summary>
		void RecalculateAllCosts();

		/// <summary>
		/// Recalculates the costs between all the nodes touching a deque of specific rectangular areas (which will be wrapped). Also resets the pather itself, if necessary.
		/// </summary>
		/// <param name="boxList">The deque of Boxes representing the updated areas.</param>
		/// <param name="nodeUpdateLimit">The maximum number of nodes we'll try to update this frame. True node update count can be higher if we received a big box, as we always do at least 1 box.</param>
		/// <returns>The set of nodes that were updated.</returns>
		std::unordered_set<int> RecalculateAreaCosts(std::deque<Box> &boxList, int nodeUpdateLimit);

		/// <summary>
		/// Updates a set of nodes, adjusting their transitions
		/// This does NOT update the pather, which is required if node costs changed.
		/// </summary>
		/// <param name="nodeList">The set of node IDs to update.</param>
		/// <returns>Whether any node costs changed.</returns>
		bool UpdateNodeList(const std::unordered_set<int>& nodeList);

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the least possible cost to get from node A to B, if it all was air.
		/// </summary>
		/// <param name="startState">Pointer to node to start from. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="endState">Node to end up at. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <returns>The cost of the absolutely fastest possible way between the two points, as if traveled through air all the way.</returns>
		float LeastCostEstimate(void *startState, void *endState) override;

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the cost to go to any adjacent node of the one passed in.
		/// </summary>
		/// <param name="state">Pointer to node to get to cost of all adjacents for. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="adjacentList">An empty vector which will be filled out with all the valid nodes adjacent to the one passed in. If at non-wrapping edge of seam, those non existent nodes won't be added.</param>
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
		static constexpr float c_NodeCostChangeEpsilon = 5.0F; //!< The minimum change in a node's cost for the pathfinder to recognize a change and reset itself. This is so minor changes (e.g. blood particles) don't force constant pathfinder resets.

		MicroPather *m_Pather; //!< The actual pathing object that does the pathfinding work. Owned.
		std::vector<PathNode *> m_NodeGrid;  //!< The array of PathNodes representing the grid on the scene. The nodes are owned by this.
		unsigned int m_NodeDimension; //!< The width and height of each node, in pixels on the scene.
		int m_GridWidth; //!< The width of the pathing grid, in nodes.
		int m_GridHeight; //!< The height of the pathing grid, in nodes.
		bool m_WrapsX; //!< Whether the pathing grid wraps on the X axis
		bool m_WrapsY; //!< Whether the pathing grid wraps on the Y axis

		float m_DigStrength; //!< What material strength the search is capable of digging through.

#pragma region Path Cost Updates
		/// <summary>
		/// Helper function for getting the strongest material we need to path though between nodes
		/// </summary>
		/// <param name="start">Origin point.</param>
		/// <param name="end">Destination point.</param>
		/// <returns>The strongest material.</returns>
		const Material * StrongestMaterialAlongLine(const Vector &start, const Vector &end) const;

		/// <summary>
		/// Helper function for updating all the values of cost edges going out from a specific node.
		/// This does NOT update the pather, which is required before solving more paths after calling this.
		/// </summary>
		/// <param name="node">The node to update all costs of. It's safe to pass 0 here. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <returns>Whether the node costs changed.</returns>
		bool UpdateNodeCosts(PathNode *node) const;

		/// <summary>
		/// Helper function for getting the nodes in a box
		/// </summary>
		/// <param name="box">The Box of which all nodes it touches should be returned.</param>
		/// <returns>A list of the node ids inside the box.</returns>
		std::vector<int> GetNodeIdsInBox(Box box);

		/// <summary>
		/// Gets the cost for transitioning through this material
		/// </summary>
		/// <param name="node">The material to get the transition cost for.</param>
		/// <returns>The transition cost.</returns>
		float GetMaterialTransitionCost(const Material *material) const ;

		/// <summary>
		/// Gets the average cost for all transitions out of this node, ignoring infinities/unpathable transitions
		/// </summary>
		/// <param name="node">The node to get the average transition cost for.</param>
		/// <returns>The average transition cost.</returns>
		float GetNodeAverageTransitionCost(const PathNode &node) const;
#pragma endregion

		/// <summary>
		/// Gets a pathnode at the coordinates.
		/// </summary>
		/// <param name="x">The x coordinate, in Nodes.</param>
		/// <param name="y">The y coordinate, in Nodes.</param>
		/// <returns>The node.</returns>
		PathNode* GetNodeForCoords(int x, int y);

		/// <summary>
		/// Converts a 2d x,y into a 1d node ID.
		/// </summary>
		/// <param name="x">The x coordinate, in Nodes.</param>
		/// <param name="y">The y coordinate, in Nodes.</param>
		/// <returns>The node ID.</returns>
		int GetNodeIdForCoords(int x, int y);

		/// <summary>
		/// Clears all the member variables of this PathFinder, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif