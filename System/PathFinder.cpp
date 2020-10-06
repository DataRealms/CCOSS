#include "PathFinder.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Clear() {
		m_NodeGrid.clear();
		m_NodeDimension = 20;
		m_DigStrength = 1;
		m_Pather = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::Create(Scene *scene, int nodeDimension, unsigned int allocate) {
		RTEAssert(scene, "Scene doesn't exist or isn't loaded when creating PathFinder!");

		m_NodeDimension = nodeDimension;
		int sceneWidth = g_SceneMan.GetSceneWidth();
		int sceneHeight = g_SceneMan.GetSceneHeight();

		// Make overlapping nodes at seams if necessary, to make sure all scene pixels are covered
		int nodeXCount = std::ceil(static_cast<float>(sceneWidth) / static_cast<float>(m_NodeDimension));
		int nodeYCount = std::ceil(static_cast<float>(sceneHeight) / static_cast<float>(m_NodeDimension));

		// Create and assign scene coordinate positions for all nodes
		PathNode *node = 0;
		Vector nodePos = Vector(static_cast<float>(nodeDimension) / 2.0F, static_cast<float>(nodeDimension) / 2.0F);
		for (int x = 0; x < nodeXCount; ++x) {
			// Make sure no cell centers are off the scene (since they can overlap the far edge of the scene)
			if (nodePos.m_X >= sceneWidth) { nodePos.m_X = sceneWidth - 1; }
			// Start the column height over at middle of the top node each new column
			nodePos.m_Y = static_cast<float>(nodeDimension) / 2.0F;
			// Create the new column and fill it out
			std::vector<PathNode *> newColumn;
			for (int y = 0; y < nodeYCount; ++y) {
				// Make sure no cell centers are off the scene (since they can overlap the far edge of the scene)
				if (nodePos.m_Y >= sceneHeight) { nodePos.m_Y = sceneHeight - 1; }
				// Create the new node with its in-scene position in the center of it
				node = new PathNode(nodePos);
				// Move current position down for the next node in the column
				nodePos.m_Y += nodeDimension;
				// Add the newly created node to the column, transferring ownership to it
				newColumn.push_back(node);
			}
			// Move current position one to the right for the next column
			nodePos.m_X += nodeDimension;
			// Add the entire new column to the right end of the grid
			m_NodeGrid.push_back(newColumn);
		}
		// Assign all the adjacent nodes on each node, taking into account scene wrapping etc.
		int wrappedUp;
		int wrappedRight;
		int wrappedDown;
		int wrappedLeft;
		for (int x = 0; x < nodeXCount; ++x) {
			for (int y = 0; y < nodeYCount; ++y) {
				node = m_NodeGrid[x][y];

				wrappedLeft = x - 1;
				if (wrappedLeft < 0 && scene->WrapsX()) { wrappedLeft = nodeXCount - 1; }
				wrappedRight = x + 1;
				if (wrappedRight >= nodeXCount && scene->WrapsX()) { wrappedRight = 0; }
				wrappedUp = y - 1;
				if (wrappedUp < 0 && scene->WrapsY()) { wrappedUp = nodeYCount - 1; }
				wrappedDown = y + 1;
				if (wrappedDown >= nodeYCount && scene->WrapsY()) { wrappedDown = 0; }

				// Leave nulls if any are out of bounds, even after wrapping (ie there was no wrapping in effect in that direction)
				if (wrappedUp >= 0) { node->Up = m_NodeGrid[x][wrappedUp]; }
				if (wrappedRight < nodeXCount) { node->Right = m_NodeGrid[wrappedRight][y]; }
				if (wrappedDown < nodeYCount) { node->Down = m_NodeGrid[x][wrappedDown]; }
				if (wrappedLeft >= 0) { node->Left = m_NodeGrid[wrappedLeft][y]; }

				// Diagonals
				if (wrappedUp >= 0 && wrappedRight < nodeXCount) { node->UpRight = m_NodeGrid[wrappedRight][wrappedUp]; }
				if (wrappedRight < nodeXCount && wrappedDown < nodeYCount) { node->RightDown = m_NodeGrid[wrappedRight][wrappedDown]; }
				if (wrappedDown < nodeYCount && wrappedLeft >= 0) { node->DownLeft = m_NodeGrid[wrappedLeft][wrappedDown]; }
				if (wrappedLeft >= 0 && wrappedUp >= 0) { node->LeftUp = m_NodeGrid[wrappedLeft][wrappedUp]; }
			}
		}
		// Create and allocate the pather class which will do the work
		m_Pather = new MicroPather(this, allocate);

		// If the scene wraps we must find the cost over the seam before doing RecalculateAllCosts() the first time
		// since the cost is equal to max(node->LeftCost, node->m_Left->RightCost)
		if (scene->WrapsX()) {
			for (int y = 0; y < nodeYCount; ++y) {
				node = m_NodeGrid[0][y];
				if (node->Left) { node->Left->RightCost = CostAlongLine(node->Pos, node->Left->Pos); }
			}
		}
		// Set up all the costs between all nodes
		RecalculateAllCosts();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Destroy() {
		for (unsigned int x = 0; x < m_NodeGrid.size(); ++x) {
			for (unsigned int y = 0; y < m_NodeGrid[x].size(); ++y) {
				delete m_NodeGrid[x][y];
				m_NodeGrid[x][y] = 0;
			}
		}
		delete m_Pather;
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::CalculatePath(Vector start, Vector end, std::list<Vector> &pathResult, float &totalCostResult, float digStrength) {
		RTEAssert(m_Pather, "No pather exists, can't calculate the path!");

		// Make sure start and end are within scene bounds
		g_SceneMan.ForceBounds(start);
		g_SceneMan.ForceBounds(end);

		// Convert from absolute scene pixel coordinates to path node indices
		int startNodeX = std::floor(start.m_X / static_cast<float>(m_NodeDimension));
		int startNodeY = std::floor(start.m_Y / static_cast<float>(m_NodeDimension));
		int endNodeX = std::floor(end.m_X / static_cast<float>(m_NodeDimension));
		int endNodeY = std::floor(end.m_Y / static_cast<float>(m_NodeDimension));

		// Clear out the results if it happens to contain anything
		pathResult.clear();

		// Actors capable of digging can use m_DigStrength to modify the node adjacency cost
		m_DigStrength = digStrength;

		// Do the actual pathfinding, fetch out the list of states that comprise the best path
		std::vector<void *> statePath;
		int result = m_Pather->Solve(static_cast<void *>(m_NodeGrid[startNodeX][startNodeY]), static_cast<void *>(m_NodeGrid[endNodeX][endNodeY]), &statePath, &totalCostResult);

		// We got something back
		if (!statePath.empty()) {
			// Replace the approximate first point from the pathfound path with the exact starting point
			pathResult.push_back(start);
			std::vector<void *>::iterator itr = statePath.begin();
			itr++;

			// Convert from a list of state void pointers to a list of scene position vectors
			for (; itr != statePath.end(); ++itr) {
				pathResult.push_back((static_cast<PathNode *>(*itr))->Pos);
			}

			// Adjust the last point to be exactly where the end is supposed to be (really?)
			if (pathResult.size() > 2) {
				pathResult.pop_back();
				pathResult.push_back(end);
			}
			// Empty path, give exact start and end
		} else {
			pathResult.push_back(start);
			pathResult.push_back(end);
		}
		// TODO: Clean up the path, remove series of nodes in the same direction etc?
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::RecalculateAllCosts() {
		RTEAssert(g_SceneMan.GetScene(), "Scene doesn't exist or isn't loaded when recalculating PathFinder!");

		// Update all the costs going out from each node
		for (const std::vector<PathNode *> &nodeEntry : m_NodeGrid) {
			for (PathNode *pathNode : nodeEntry) {
				UpdateNodeCosts(pathNode);
				// Should reset the changed flag since we're about to reset the pather
				pathNode->IsChanged = false;
			}
		}
		// Reset the pather when costs change, as per the docs
		m_Pather->Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::RecalculateAreaCosts(const std::list<Box> &boxList) {
		Box box;
		// Go through all the boxes and see if any of the node centers are inside each
		for (const Box &boxListEntry : boxList) {
			// Get the current area box and make sure it's unflipped
			box = boxListEntry;
			box.Unflip();

			// Do the updates
			UpdateNodeCostsInBox(box);

			// Take care of all wrapping situations of the box
			if (g_SceneMan.SceneWrapsX()) {
				Box temp;

				if (box.m_Corner.m_X < 0) {
					temp = Box(Vector(box.m_Corner.m_X + g_SceneMan.GetSceneWidth(), box.m_Corner.m_Y), box.m_Width, box.m_Height);
					UpdateNodeCostsInBox(temp);
				} else if (box.m_Corner.m_X + box.m_Width > g_SceneMan.GetSceneWidth()) {
					temp = Box(Vector(box.m_Corner.m_X - g_SceneMan.GetSceneWidth(), box.m_Corner.m_Y), box.m_Width, box.m_Height);
					UpdateNodeCostsInBox(temp);
				}
			}
			if (g_SceneMan.SceneWrapsY()) {
				Box temp;

				if (box.m_Corner.m_Y < 0) {
					temp = Box(Vector(box.m_Corner.m_X, box.m_Corner.m_Y + g_SceneMan.GetSceneHeight()), box.m_Width, box.m_Height);
					UpdateNodeCostsInBox(temp);
				} else if (box.m_Corner.m_Y + box.m_Height > g_SceneMan.GetSceneHeight()) {
					temp = Box(Vector(box.m_Corner.m_X, box.m_Corner.m_Y - g_SceneMan.GetSceneHeight()), box.m_Width, box.m_Height);
					UpdateNodeCostsInBox(temp);
				}
			}
		}

		// Reset the pather when costs change, as per the docs
		m_Pather->Reset();

		// Reset the changed flag on all nodes
		for (const std::vector<PathNode *> &nodeEntry : m_NodeGrid) {
			for (PathNode *pathNode : nodeEntry) {
				pathNode->IsChanged = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float PathFinder::LeastCostEstimate(void *startState, void *endState) {
		return g_SceneMan.ShortestDistance((static_cast<PathNode *>(startState))->Pos, (static_cast<PathNode *>(endState))->Pos).GetMagnitude();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::AdjacentCost(void *state, std::vector<micropather::StateCost> *adjacentList) {
		const PathNode *node = static_cast<PathNode *>(state);
		micropather::StateCost adjCost;
		float strength = 0.0F;

		// Add cost for digging upwards
		if (node->Up) {
			strength = node->UpCost;
			adjCost.cost = 1.0F + ((strength > m_DigStrength) ? strength * 2000.0F : strength * 4.0F); // Four times more expensive when digging
			adjCost.state = static_cast<void *>(node->Up);
			adjacentList->push_back(adjCost);
		}
		if (node->Right) {
			strength = node->RightCost;
			adjCost.cost = 1.0F + ((strength > m_DigStrength) ? strength * 1000.0F : strength);
			adjCost.state = static_cast<void *>(node->Right);
			adjacentList->push_back(adjCost);
		}
		if (node->Down) {
			strength = node->DownCost;
			adjCost.cost = 1.0F + ((strength > m_DigStrength) ? strength * 1000.0F : strength);
			adjCost.state = static_cast<void *>(node->Down);
			adjacentList->push_back(adjCost);
		}
		if (node->Left) {
			strength = node->LeftCost;
			adjCost.cost = 1.0F + ((strength > m_DigStrength) ? strength * 1000.0F : strength);
			adjCost.state = static_cast<void *>(node->Left);
			adjacentList->push_back(adjCost);
		}

		// Add cost for digging at 45 degrees and for digging upwards
		if (node->UpRight) {
			strength = node->UpRightCost;
			adjCost.cost = 1.4F + ((strength > m_DigStrength) ? strength * 2828.0F : strength * 4.2F);  // Three times more expensive when digging
			adjCost.state = static_cast<void *>(node->UpRight);
			adjacentList->push_back(adjCost);
		}
		if (node->RightDown) {
			strength = node->RightDownCost;
			adjCost.cost = 1.4F + ((strength > m_DigStrength) ? strength * 1414.0F : strength * 1.4F);
			adjCost.state = static_cast<void *>(node->RightDown);
			adjacentList->push_back(adjCost);
		}
		if (node->DownLeft) {
			strength = node->DownLeftCost;
			adjCost.cost = 1.4F + ((strength > m_DigStrength) ? strength * 1414.0F : strength * 1.4F);
			adjCost.state = static_cast<void *>(node->DownLeft);
			adjacentList->push_back(adjCost);
		}
		if (node->LeftUp) {
			strength = node->LeftUpCost;
			adjCost.cost = 1.4F + ((strength > m_DigStrength) ? strength * 2828.0F : strength * 4.2F);  // Three times more expensive when digging
			adjCost.state = static_cast<void *>(node->LeftUp);
			adjacentList->push_back(adjCost);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::UpdateNodeCosts(PathNode *node) {
		if (!node) {
			return;
		}
		// Look at each existing adjacent node and calculate the cost for each, offset start and end to cover more terrain
		if (node->Up) { node->UpCost = std::max(node->Up->DownCost, CostAlongLine(node->Pos + Vector(3, 0), node->Up->Pos + Vector(3, 0))); }
		if (node->Right) { node->RightCost = CostAlongLine(node->Pos + Vector(0, 3), node->Right->Pos + Vector(0, 3)); }
		if (node->Down) { node->DownCost = CostAlongLine(node->Pos + Vector(-3, 0), node->Down->Pos + Vector(-3, 0)); }
		if (node->Left) { node->LeftCost = std::max(node->Left->RightCost, CostAlongLine(node->Pos + Vector(0, -3), node->Left->Pos + Vector(0, -3))); }

		if (node->UpRight) { node->UpRightCost = std::max(node->UpRight->DownLeftCost, CostAlongLine(node->Pos + Vector(2, 2), node->UpRight->Pos + Vector(2, 2))); }
		if (node->RightDown) { node->RightDownCost = CostAlongLine(node->Pos + Vector(2, -2), node->RightDown->Pos + Vector(2, -2)); }
		if (node->DownLeft) { node->DownLeftCost = CostAlongLine(node->Pos + Vector(-2, -2), node->DownLeft->Pos + Vector(-2, -2)); }
		if (node->LeftUp) { node->LeftUpCost = std::max(node->LeftUp->RightDownCost, CostAlongLine(node->Pos + Vector(-2, 2), node->LeftUp->Pos + Vector(-2, 2))); }

		// Mark this as already changed so the above expensive calculation isn't done redundantly
		node->IsChanged = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::UpdateNodeCostsInBox(Box &box) {
		box.Unflip();

		// Get the extents of the box' potential influence on nodes and their connecting edges
		int firstX = std::floor((box.m_Corner.m_X / static_cast<float>(m_NodeDimension)) + 0.5F) - 1;
		int lastX = std::floor(((box.m_Corner.m_X + box.m_Width) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1;
		int firstY = std::floor((box.m_Corner.m_Y / static_cast<float>(m_NodeDimension)) + 0.5F) - 1;
		int lastY = std::floor(((box.m_Corner.m_Y + box.m_Height) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1;

		// Truncate the influence
		if (firstX < 0) { firstX = 0; }
		if (lastX >= m_NodeGrid.size()) { lastX = m_NodeGrid.size() - 1; }
		if (firstY < 0) { firstY = 0; }
		if (lastY >= m_NodeGrid[0].size()) { lastY = m_NodeGrid[0].size() - 1; }

		// Only iterate through the grid where the box overlaps any edges
		PathNode *node = 0;
		for (int nodeX = firstX; nodeX <= lastX; ++nodeX) {
			for (int nodeY = firstY; nodeY <= lastY; ++nodeY) {
				node = m_NodeGrid[nodeX][nodeY];
				// Update all the costs going out from each node which is found to be affected by the box
				if (!node->IsChanged) { UpdateNodeCosts(node); }
			}
		}
	}
}