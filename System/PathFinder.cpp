#include "PathFinder.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Clear() {
		m_NodeGrid.clear();
		m_NodeDimension = 20;
		m_DigStrength = 1;
		m_pPather = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::Create(Scene *pScene, int nodeDimension, unsigned int allocate) {
		RTEAssert(pScene, "Scene doesn't exist or isn't loaded when creating PathFinder!");

		m_NodeDimension = nodeDimension;
		int sceneWidth = g_SceneMan.GetSceneWidth();
		int sceneHeight = g_SceneMan.GetSceneHeight();

		// Make overlapping nodes at seams if necessary, to make sure all scene pixels are covered
		int nodeXCount = ceilf(static_cast<float>(sceneWidth) / static_cast<float>(m_NodeDimension));
		int nodeYCount = ceilf(static_cast<float>(sceneHeight) / static_cast<float>(m_NodeDimension));

		// Create and assign scene coordinate positions for all nodes
		PathNode *pNode = 0;
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
				pNode = new PathNode(nodePos);
				// Move current position down for the next node in the column
				nodePos.m_Y += nodeDimension;
				// Add the newly created node to the column, transferring ownership to it
				newColumn.push_back(pNode);
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
				pNode = m_NodeGrid[x][y];

				wrappedLeft = x - 1;
				if (wrappedLeft < 0 && pScene->WrapsX()) { wrappedLeft = nodeXCount - 1; }
				wrappedRight = x + 1;
				if (wrappedRight >= nodeXCount && pScene->WrapsX()) { wrappedRight = 0; }
				wrappedUp = y - 1;
				if (wrappedUp < 0 && pScene->WrapsY()) { wrappedUp = nodeYCount - 1; }
				wrappedDown = y + 1;
				if (wrappedDown >= nodeYCount && pScene->WrapsY()) { wrappedDown = 0; }

				// Leave nulls if any are out of bounds, even after wrapping (ie there was no wrapping in effect in that direction)
				if (wrappedUp >= 0) { pNode->m_pUp = m_NodeGrid[x][wrappedUp]; }
				if (wrappedRight < nodeXCount) { pNode->m_pRight = m_NodeGrid[wrappedRight][y]; }
				if (wrappedDown < nodeYCount) { pNode->m_pDown = m_NodeGrid[x][wrappedDown]; }
				if (wrappedLeft >= 0) { pNode->m_pLeft = m_NodeGrid[wrappedLeft][y]; }

				// Diagonals
				if (wrappedUp >= 0 && wrappedRight < nodeXCount) { pNode->m_pUpRight = m_NodeGrid[wrappedRight][wrappedUp]; }
				if (wrappedRight < nodeXCount && wrappedDown < nodeYCount) { pNode->m_pRightDown = m_NodeGrid[wrappedRight][wrappedDown]; }
				if (wrappedDown < nodeYCount && wrappedLeft >= 0) { pNode->m_pDownLeft = m_NodeGrid[wrappedLeft][wrappedDown]; }
				if (wrappedLeft >= 0 && wrappedUp >= 0) { pNode->m_pLeftUp = m_NodeGrid[wrappedLeft][wrappedUp]; }
			}
		}
		// Create and allocate the pather class which will do the work
		m_pPather = new MicroPather(this, allocate);

		// If the scene wraps we must find the cost over the seam before doing RecalculateAllCosts() the first time
		// since the cost is equal to max(pNode->m_LeftCost, pNode->m_pLeft->m_RightCost)
		if (pScene->WrapsX()) {
			for (int y = 0; y < nodeYCount; ++y) {
				pNode = m_NodeGrid[0][y];
				if (pNode->m_pLeft) { pNode->m_pLeft->m_RightCost = CostAlongLine(pNode->m_Pos, pNode->m_pLeft->m_Pos); }
			}
		}
		// Set up all the costs between all nodes
		RecalculateAllCosts();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Destroy() {
		for (int x = 0; x < m_NodeGrid.size(); ++x) {
			for (int y = 0; y < m_NodeGrid[x].size(); ++y) {
				delete m_NodeGrid[x][y];
				m_NodeGrid[x][y] = 0;
			}
		}
		delete m_pPather;
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::CalculatePath(Vector start, Vector end, std::list<Vector> &pathResult, float &totalCostResult, float digStrength) {
		RTEAssert(m_pPather, "No pather exists, can't calculate the path!");

		// Make sure start and end are within scene bounds
		g_SceneMan.ForceBounds(start);
		g_SceneMan.ForceBounds(end);

		// Convert from absolute scene pixel coordinates to path node indices
		int startNodeX = floorf(start.m_X / static_cast<float>(m_NodeDimension));
		int startNodeY = floorf(start.m_Y / static_cast<float>(m_NodeDimension));
		int endNodeX = floorf(end.m_X / static_cast<float>(m_NodeDimension));
		int endNodeY = floorf(end.m_Y / static_cast<float>(m_NodeDimension));

		// Clear out the results if it happens to contain anything
		pathResult.clear();

		// Actors capable of digging can use m_DigStrength to modify the node adjacency cost
		m_DigStrength = digStrength;

		// Do the actual pathfinding, fetch out the list of states that comprise the best path
		std::vector<void *> statePath;
		int result = m_pPather->Solve((void *)(m_NodeGrid[startNodeX][startNodeY]), (void *)(m_NodeGrid[endNodeX][endNodeY]), &statePath, &totalCostResult);

		// We got something back
		if (!statePath.empty()) {
			// Replace the approximate first point from the pathfound path with the exact starting point
			pathResult.push_back(start);
			std::vector<void *>::iterator itr = statePath.begin();
			itr++;

			// Convert from a list of state void pointers to a list of scene position vectors
			for (; itr != statePath.end(); ++itr) { pathResult.push_back(((PathNode *)(*itr))->m_Pos); }

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

		PathNode *pNode = 0;
		for (int x = 0; x < m_NodeGrid.size(); ++x) {
			// Update all the costs going out from each node
			for (int y = 0; y < m_NodeGrid[x].size(); ++y) {
				pNode = m_NodeGrid[x][y];
				UpdateNodeCosts(pNode);
				// Should reset the changed flag since we're about to reset the pather
				pNode->m_IsChanged = false;
			}
		}
		// Reset the pather when costs change, as per the docs
		m_pPather->Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::RecalculateAreaCosts(const std::list<Box> &boxList) {
		// Go through all the boxes and see if any of the node centers are inside each
		Box box;
		for (std::list<Box>::const_iterator bItr = boxList.begin(); bItr != boxList.end(); bItr++) {
			// Get the current area box and make sure it's unflipped
			box = (*bItr);
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
		m_pPather->Reset();

		// Reset the changed flag on all nodes
		for (int x = 0; x < m_NodeGrid.size(); ++x) {
			for (int y = 0; y < m_NodeGrid[x].size(); ++y) {
				m_NodeGrid[x][y]->m_IsChanged = false;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float PathFinder::LeastCostEstimate(void *pStartState, void *pEndState) {
		// TODO: Not .GetLargest()?? - No, because we're calculating cost as the difference between pos's, PLUS the pixel material strength costs summed
		return g_SceneMan.ShortestDistance(((PathNode *)pStartState)->m_Pos, ((PathNode *)pEndState)->m_Pos).GetMagnitude();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::AdjacentCost(void *pState, std::vector<micropather::StateCost> *pAdjacentList) {
		PathNode *pNode = (PathNode *)pState;
		micropather::StateCost adjCost;
		float strength = 0;

		// Add cost for digging upwards
		if (pNode->m_pUp) {
			strength = pNode->m_UpCost;
			adjCost.cost = 1 + (strength > m_DigStrength ? strength * 2000 : strength * 4); // Four times more expensive when digging
			adjCost.state = (void *)pNode->m_pUp;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pRight) {
			strength = pNode->m_RightCost;
			adjCost.cost = 1 + (strength > m_DigStrength ? strength * 1000 : strength);
			adjCost.state = (void *)pNode->m_pRight;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pDown) {
			strength = pNode->m_DownCost;
			adjCost.cost = 1 + (strength > m_DigStrength ? strength * 1000 : strength);
			adjCost.state = (void *)pNode->m_pDown;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pLeft) {
			strength = pNode->m_LeftCost;
			adjCost.cost = 1 + (strength > m_DigStrength ? strength * 1000 : strength);
			adjCost.state = (void *)pNode->m_pLeft;
			pAdjacentList->push_back(adjCost);
		}

		// Add cost for digging at 45 degrees and for digging upwards
		if (pNode->m_pUpRight) {
			strength = pNode->m_UpRightCost;
			adjCost.cost = 1.4 + (strength > m_DigStrength ? strength * 2828 : strength * 4.2);  // Three times more expensive when digging
			adjCost.state = (void *)pNode->m_pUpRight;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pRightDown) {
			strength = pNode->m_RightDownCost;
			adjCost.cost = 1.4 + (strength > m_DigStrength ? strength * 1414 : strength * 1.4);
			adjCost.state = (void *)pNode->m_pRightDown;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pDownLeft) {
			strength = pNode->m_DownLeftCost;
			adjCost.cost = 1.4 + (strength > m_DigStrength ? strength * 1414 : strength * 1.4);
			adjCost.state = (void *)pNode->m_pDownLeft;
			pAdjacentList->push_back(adjCost);
		}
		if (pNode->m_pLeftUp) {
			strength = pNode->m_LeftUpCost;
			adjCost.cost = 1.4 + (strength > m_DigStrength ? strength * 2828 : strength * 4.2);  // Three times more expensive when digging
			adjCost.state = (void *)pNode->m_pLeftUp;
			pAdjacentList->push_back(adjCost);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::UpdateNodeCosts(PathNode *pNode) {
		if (!pNode) {
			return;
		}
		// Look at each existing adjacent node and calculate the cost for each, offset start and end to cover more terrain
		if (pNode->m_pUp) { pNode->m_UpCost = max(pNode->m_pUp->m_DownCost, CostAlongLine(pNode->m_Pos + Vector(3, 0), pNode->m_pUp->m_Pos + Vector(3, 0))); }
		if (pNode->m_pRight) { pNode->m_RightCost = CostAlongLine(pNode->m_Pos + Vector(0, 3), pNode->m_pRight->m_Pos + Vector(0, 3)); }
		if (pNode->m_pDown) { pNode->m_DownCost = CostAlongLine(pNode->m_Pos + Vector(-3, 0), pNode->m_pDown->m_Pos + Vector(-3, 0)); }
		if (pNode->m_pLeft) { pNode->m_LeftCost = max(pNode->m_pLeft->m_RightCost, CostAlongLine(pNode->m_Pos + Vector(0, -3), pNode->m_pLeft->m_Pos + Vector(0, -3))); }

		if (pNode->m_pUpRight) { pNode->m_UpRightCost = max(pNode->m_pUpRight->m_DownLeftCost, CostAlongLine(pNode->m_Pos + Vector(2, 2), pNode->m_pUpRight->m_Pos + Vector(2, 2))); }
		if (pNode->m_pRightDown) { pNode->m_RightDownCost = CostAlongLine(pNode->m_Pos + Vector(2, -2), pNode->m_pRightDown->m_Pos + Vector(2, -2)); }
		if (pNode->m_pDownLeft) { pNode->m_DownLeftCost = CostAlongLine(pNode->m_Pos + Vector(-2, -2), pNode->m_pDownLeft->m_Pos + Vector(-2, -2)); }
		if (pNode->m_pLeftUp) { pNode->m_LeftUpCost = max(pNode->m_pLeftUp->m_RightDownCost, CostAlongLine(pNode->m_Pos + Vector(-2, 2), pNode->m_pLeftUp->m_Pos + Vector(-2, 2))); }

		// Mark this as already changed so the above expensive calculation isn't done redundantly
		pNode->m_IsChanged = true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::UpdateNodeCostsInBox(Box &box) {
		box.Unflip();

		// Get the extents of the box' potential influence on nodes and their connecting edges
		int firstX = floorf((box.m_Corner.m_X / static_cast<float>(m_NodeDimension)) + 0.5F) - 1;
		int lastX = floorf(((box.m_Corner.m_X + box.m_Width) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1;
		int firstY = floorf((box.m_Corner.m_Y / static_cast<float>(m_NodeDimension)) + 0.5F) - 1;
		int lastY = floorf(((box.m_Corner.m_Y + box.m_Height) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1;

		// Truncate the influence
		if (firstX < 0) { firstX = 0; }
		if (lastX >= m_NodeGrid.size()) { lastX = m_NodeGrid.size() - 1; }
		if (firstY < 0) { firstY = 0; }
		if (lastY >= m_NodeGrid[0].size()) { lastY = m_NodeGrid[0].size() - 1; }

		// Only iterate through the grid where the box overlaps any edges
		PathNode *pNode = 0;
		for (int nodeX = firstX; nodeX <= lastX; ++nodeX) {
			for (int nodeY = firstY; nodeY <= lastY; ++nodeY) {
				pNode = m_NodeGrid[nodeX][nodeY];
				// Update all the costs going out from each node which is found to be affected by the box
				if (!pNode->m_IsChanged) { UpdateNodeCosts(pNode); }
			}
		}
	}
}