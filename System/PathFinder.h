#ifndef _RTEPATHFINDER_
#define _RTEPATHFINDER_

#include "Box.h"
#include "Scene.h"
#include "System/MicroPather/micropather.h"

using namespace micropather;

namespace RTE {

	/// <summary>
	/// Contains everything related to a node on the path grid used by PathFinder.
	/// </summary>
	struct PathNode {

		Vector m_Pos; //! Absolute position of the center of this node in the scene.    
		bool m_IsChanged; //! Whether this has been updated since last call to Reset the pather.

		/// <summary>
		/// Pointers to all adjacent nodes. These are not owned, and may be 0 if adjacent to non-wrapping scene border.
		/// </summary>
		PathNode *m_pUp;
		PathNode *m_pRight;
		PathNode *m_pDown;
		PathNode *m_pLeft;
		PathNode *m_pUpRight;
		PathNode *m_pRightDown;
		PathNode *m_pDownLeft;
		PathNode *m_pLeftUp;

		/// <summary>
		/// Costs to get to each of the adjacent nodes.
		/// </summary>
		float m_UpCost;
		float m_RightCost;
		float m_DownCost;
		float m_LeftCost;
		float m_UpRightCost;
		float m_RightDownCost;
		float m_DownLeftCost;
		float m_LeftUpCost;

		PathNode(Vector pos) {
			m_Pos = pos;
			m_pUp = m_pRight = m_pDown = m_pLeft = m_pUpRight = m_pRightDown = m_pDownLeft = m_pLeftUp = 0;
			// Costs are infinite unless recalculated as otherwise
			m_UpCost = m_RightCost = m_DownCost = m_LeftCost = m_UpRightCost = m_RightDownCost = m_DownLeftCost = m_LeftUpCost = FLT_MAX;
		}
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
		PathFinder(Scene *pScene, int nodeDimension = 20, unsigned int allocate = 2000) { Clear(); Create(pScene, nodeDimension, allocate); }

		/// <summary>
		/// Makes the PathFinder object ready for use.
		/// </summary>
		/// <param name="pScene">The scene to be pathing within.</param>
		/// <param name="nodeDimension">The width and height in scene pixels that of each node should represent.</param>
		/// <param name="allocate">The block size that the node cache is allocated from. Should be about a fourth of the total number of nodes.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(Scene *pScene, int nodeDimension = 20, unsigned int allocate = 2000);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PathFinder object before deletion.
		/// </summary>
		virtual ~PathFinder() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) this PathFinder object.
		/// </summary>
		virtual void Destroy();

		/// <summary>
		/// Resets the entire Box object to the default settings or values.
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
		/// Recalculates the costs between all the nodes touching a list of specific rectangular areas (which will be wrapped). Also resets the pather itself.
		/// </summary>
		/// <param name="boxList">The list of Boxes representing the updated areas.</param>
		void RecalculateAreaCosts(const std::list<Box> &boxList);

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the least possible cost to get from node A to B, if it all was air.
		/// </summary>
		/// <param name="pStartState">Pointer to node to start from. OINT.</param>
		/// <param name="pEndState">Node to end up at. OINT.</param>
		/// <returns>The cost of the absolutely fastest possible way between the two points, as if traveled through air all the way.</returns>
		virtual float LeastCostEstimate(void *pStartState, void *pEndState);

		/// <summary>
		/// Implementation of the abstract interface of Graph.
		/// Gets the cost to go to any adjacent node of the one passed in.
		/// </summary>
		/// <param name="pState">Pointer to node to get to cost of all adjacents for. OINT.</param>
		/// <param name="pAdjacentList">
		/// An empty vector which will be filled out with all the valid nodes adjacent to the one passed in.
		/// If at non-wrapping edge of seam, those non existent nodes won't be added.
		/// </param>
		virtual void AdjacentCost(void *pState, std::vector<micropather::StateCost> *pAdjacentList);
#pragma endregion

#pragma region Misc
		/// <summary>
		/// Implementation of the abstract interface of Graph. This function is only used in DEBUG mode - it dumps output to stdout.
		/// Since void* aren't really human readable, this will print out some concise info without an ending newline.
		/// </summary>
		/// <param name="pState">The state to print out info about.</param>
		virtual void PrintStateInfo(void *pState) { ; }
#pragma endregion

	protected:

		MicroPather *m_pPather; //! The actual pathing object that does the pathfinding work. Owned.
		std::vector<std::vector<PathNode *> > m_NodeGrid;  //! The array of PathNodes representing the grid on the scene. The nodes are owned by this.
		int m_NodeDimension; //! The width and height of each node, in pixels on the scene.

		float m_DigStrenght; //! What material strength the search is capable of digging through.

#pragma region Path Cost Updates
		/// <summary>
		/// Helper function for calculating the real actual cost of going in a straight line between any two points on the scene.
		/// It takes into account distance traveled, as well as the strength of the materials the line has to pass through.
		/// UPDATE: newer version also goes through parallel lines offset to each side from the main one.
		/// </summary>
		/// <param name="start">Origin point.</param>
		/// <param name="end">Destination point.</param>
		/// <returns>The cost value.</returns>
		float CostAlongLine(const Vector &start, const Vector &end) { return g_SceneMan.CastMaxStrengthRay(start, end, 0); }

		/// <summary>
		/// Helper function for updating all the values of cost edges going out from a specific node.
		/// This does NOT update the pather, which is required before solving more paths after calling this.
		/// </summary>
		/// <param name="pNode">The node to update all costs of. OINT. It's safe to pass 0 here.</param>
		void UpdateNodeCosts(PathNode *pNode);

		/// <summary>
		/// Helper function for updating all the values of cost edges crossed by a specific box.
		/// This does NOT update the pather, which is required before solving more paths after calling this. Also it does NOT wrap the box coming in here, only truncates it!
		/// </summary>
		/// <param name="box">The Box of which all edges it touches should be recalculated.</param>
		void UpdateNodeCostsInBox(Box &box);
#pragma endregion

	private:

		/// <summary>
		/// Clears all the member variables of this PathFinder, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif