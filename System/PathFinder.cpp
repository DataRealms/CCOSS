#include "PathFinder.h"

#include "Material.h"
#include "Scene.h"
#include "SceneMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PathNode::PathNode(const Vector &pos) : Pos(pos) {
		const Material *outOfBounds = g_SceneMan.GetMaterialFromID(MaterialColorKeys::g_MaterialOutOfBounds);
		for (int i = 0; i < c_MaxAdjacentNodeCount; i++) {
			AdjacentNodes[i] = nullptr;
			AdjacentNodeBlockingMaterials[i] = outOfBounds; // Costs are infinite unless recalculated as otherwise.
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Clear() {
		m_NodeGrid.clear();
		m_NodeDimension = 20;
		m_DigStrength = 1;
		m_Pather = nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::Create(int nodeDimension, unsigned int allocate) {
		RTEAssert(g_SceneMan.GetScene(), "Scene doesn't exist or isn't loaded when creating PathFinder!");

		m_NodeDimension = nodeDimension;
		int sceneWidth = g_SceneMan.GetSceneWidth();
		int sceneHeight = g_SceneMan.GetSceneHeight();

		// Make overlapping nodes at seams if necessary, to make sure all scene pixels are covered.
		m_GridWidth = std::ceil(static_cast<float>(sceneWidth) / static_cast<float>(m_NodeDimension));
		m_GridHeight = std::ceil(static_cast<float>(sceneHeight) / static_cast<float>(m_NodeDimension));

		m_WrapsX = g_SceneMan.SceneWrapsX();
		m_WrapsY = g_SceneMan.SceneWrapsY();

		// Create and assign scene coordinate positions for all nodes.
		Vector nodePos = Vector(static_cast<float>(nodeDimension) / 2.0F, static_cast<float>(nodeDimension) / 2.0F);
		m_NodeGrid.reserve(m_GridWidth * m_GridHeight);
		for (int y = 0; y < m_GridHeight; ++y) {
			// Make sure no cell centers are off the scene (since they can overlap the far edge of the scene).
			if (nodePos.m_Y >= sceneHeight) {
				nodePos.m_Y = sceneHeight - 1.0F;
			}

			// Start the row over at middle of the leftmost node each new row.
			nodePos.m_X = static_cast<float>(nodeDimension) / 2.0F;

			for (int x = 0; x < m_GridWidth; ++x) {
				// Make sure no cell centers are off the scene (since they can overlap the far edge of the scene).
				if (nodePos.m_X >= sceneWidth) {
					nodePos.m_X = sceneWidth - 1.0F;
				}

				// Add the newly created node to the column. 
				// Warning! Emplace back must be used to ensure this is constructed in-place, as otherwise the Up/Right/Down etc references will be incorrect.
				m_NodeGrid.emplace_back(nodePos);

				nodePos.m_X += static_cast<float>(nodeDimension);
			}

			nodePos.m_Y += static_cast<float>(nodeDimension);
		}

		// Assign all the adjacent nodes on each node. GetPathNodeAtGridCoords handles Scene wrapping.
		for (int x = 0; x < m_GridWidth; ++x) {
			for (int y = 0; y < m_GridHeight; ++y) {
				PathNode &node = *GetPathNodeAtGridCoords(x, y);

				node.Up = GetPathNodeAtGridCoords(x, y - 1);
				node.Right = GetPathNodeAtGridCoords(x + 1, y);
				node.Down = GetPathNodeAtGridCoords(x, y + 1);
				node.Left = GetPathNodeAtGridCoords(x - 1, y);
				node.UpRight = GetPathNodeAtGridCoords(x + 1, y - 1);
				node.RightDown = GetPathNodeAtGridCoords(x + 1, y + 1);
				node.DownLeft = GetPathNodeAtGridCoords(x - 1, y + 1);
				node.LeftUp = GetPathNodeAtGridCoords(x - 1, y - 1);
			}
		}

		// Create and allocate the pather class which will do the work.
		m_Pather = new MicroPather(this, allocate, PathNode::c_MaxAdjacentNodeCount, false);

		RecalculateAllCosts();

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::Destroy() {
		delete m_Pather;
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::CalculatePath(Vector start, Vector end, std::list<Vector> &pathResult, float &totalCostResult, float digStrength) {
		RTEAssert(m_Pather, "No pather exists, can't calculate the path!");

		// Make sure start and end are within scene bounds.
		g_SceneMan.ForceBounds(start);
		g_SceneMan.ForceBounds(end);

		// Convert from absolute scene pixel coordinates to path node indices.
		int startNodeX = std::floor(start.m_X / static_cast<float>(m_NodeDimension));
		int startNodeY = std::floor(start.m_Y / static_cast<float>(m_NodeDimension));
		int endNodeX = std::floor(end.m_X / static_cast<float>(m_NodeDimension));
		int endNodeY = std::floor(end.m_Y / static_cast<float>(m_NodeDimension));

		// Clear out the results if it happens to contain anything
		pathResult.clear();

		if (m_DigStrength != digStrength) {
			// Unfortunately, DigStrength-aware pathing means that we're adjusting node transition costs, so we need to reset our path cache on every call.
			// In future we'll potentially store a different pather for different mobility bands, and reuse pathing costs.
			// But then again it's probably more fruitful to optimize the graph node to make searches faster, instead.
			m_Pather->Reset();
		}

		// Actors capable of digging can use m_DigStrength to modify the node adjacency cost.
		m_DigStrength = digStrength;

		// Do the actual pathfinding, fetch out the list of states that comprise the best path.
		std::vector<void *> statePath;
		int result = m_Pather->Solve(static_cast<void *>(GetPathNodeAtGridCoords(startNodeX, startNodeY)), static_cast<void *>(GetPathNodeAtGridCoords(endNodeX, endNodeY)), &statePath, &totalCostResult);

		if (!statePath.empty()) {
			// Replace the approximate first point from the pathfound path with the exact starting point.
			pathResult.push_back(start);
			std::vector<void *>::iterator itr = statePath.begin();
			itr++;

			// Convert from a list of state void pointers to a list of scene position vectors.
			for (; itr != statePath.end(); ++itr) {
				pathResult.push_back((static_cast<PathNode *>(*itr))->Pos);
			}

			// Adjust the last point to be exactly where the end is supposed to be (really?).
			if (pathResult.size() > 2) {
				pathResult.pop_back();
				pathResult.push_back(end);
			}
		} else {
			// Empty path, give exact start and end.
			pathResult.push_back(start);
			pathResult.push_back(end);
		}
		// TODO: Clean up the path, remove series of nodes in the same direction etc?
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::RecalculateAllCosts() {
		RTEAssert(g_SceneMan.GetScene(), "Scene doesn't exist or isn't loaded when recalculating PathFinder!");

		// I hate this copy, but fuck it.
		std::vector<int> pathNodesIdsVec;
		pathNodesIdsVec.reserve(m_NodeGrid.size());
		for (int i = 0; i < m_NodeGrid.size(); ++i) {
			pathNodesIdsVec.push_back(i);
		}

		UpdateNodeList(pathNodesIdsVec);
		m_Pather->Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<int> PathFinder::RecalculateAreaCosts(std::deque<Box> &boxList, int nodeUpdateLimit) {
		std::unordered_set<int> nodeIDsToUpdate;

		while (!boxList.empty()) {
			std::vector<int> nodesInside = GetNodeIdsInBox(boxList.front());
			for (int nodeId : nodesInside) {
				nodeIDsToUpdate.insert(nodeId);
			}

			boxList.pop_front();
			if (nodeIDsToUpdate.size() > nodeUpdateLimit) {
				break;
			}
		}

		// Note - This copy is necessary because std::for_each with parallel execution doesn't appear to work with std::unordered_set -
		// Using it will cause nodes to randomly fail to update. This should be rechecked when the codebase upgrades to C++20,
		// and then UpdateNodeList can be refactored to take a pair of iterators instead of a vector.
		std::vector<int> nodeVec(nodeIDsToUpdate.begin(), nodeIDsToUpdate.end());

		// If no PathNode costs were changed, clear the set of IDs to update, so it's empty when it's returned.
		if (!UpdateNodeList(nodeVec)) {
			nodeVec.clear();
		}

		return nodeVec;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float PathFinder::LeastCostEstimate(void *startState, void *endState) {
		return g_SceneMan.ShortestDistance((static_cast<PathNode *>(startState))->Pos, (static_cast<PathNode *>(endState))->Pos).GetMagnitude();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PathFinder::AdjacentCost(void *state, std::vector<micropather::StateCost> *adjacentList) {
		const PathNode *node = static_cast<PathNode *>(state);
		micropather::StateCost adjCost;

		// We do a little trick here, where we radiate out a little percentage of our average cost in all directions.
		// This encourages the AI to generally try to give hard surfaces some berth when pathing, so we don't get too close and get stuck.
		const float costRadiationMultiplier = 0.2F;
		float radiatedCost = GetNodeAverageTransitionCost(*node) * costRadiationMultiplier;

		// Cost to discourage us from going up. Until we have jetpack-aware pathing, this it the best we can do!
		const float extraUpCost = 3.0F;

		// Add cost for digging upwards.
		if (node->Up) {
			adjCost.cost = 1.0F + extraUpCost + (GetMaterialTransitionCost(*node->UpMaterial) * 4.0F) + radiatedCost; // Four times more expensive when digging.
			adjCost.state = static_cast<void *>(node->Up);
			adjacentList->push_back(adjCost);
		}
		if (node->Right) {
			adjCost.cost = 1.0F + GetMaterialTransitionCost(*node->RightMaterial) + radiatedCost;
			adjCost.state = static_cast<void *>(node->Right);
			adjacentList->push_back(adjCost);
		}
		if (node->Down) {
			adjCost.cost = 1.0F + GetMaterialTransitionCost(*node->DownMaterial) + radiatedCost;
			adjCost.state = static_cast<void *>(node->Down);
			adjacentList->push_back(adjCost);
		}
		if (node->Left) {
			adjCost.cost = 1.0F + GetMaterialTransitionCost(*node->LeftMaterial) + radiatedCost;
			adjCost.state = static_cast<void *>(node->Left);
			adjacentList->push_back(adjCost);
		}

		// Add cost for digging at 45 degrees and for digging upwards.
		if (node->UpRight) {
			adjCost.cost = 1.4F + extraUpCost + (GetMaterialTransitionCost(*node->UpRightMaterial) * 1.4F * 3.0F) + radiatedCost;  // Three times more expensive when digging.
			adjCost.state = static_cast<void *>(node->UpRight);
			adjacentList->push_back(adjCost);
		}
		if (node->RightDown) {
			adjCost.cost = 1.4F + (GetMaterialTransitionCost(*node->RightDownMaterial) * 1.4F) + radiatedCost;
			adjCost.state = static_cast<void *>(node->RightDown);
			adjacentList->push_back(adjCost);
		}
		if (node->DownLeft) {
			adjCost.cost = 1.4F + (GetMaterialTransitionCost(*node->DownLeftMaterial) * 1.4F) + radiatedCost;
			adjCost.state = static_cast<void *>(node->DownLeft);
			adjacentList->push_back(adjCost);
		}
		if (node->LeftUp) {
			adjCost.cost = 1.4F + extraUpCost + (GetMaterialTransitionCost(*node->LeftUpMaterial) * 1.4F * 3.0F) + radiatedCost;  // Three times more expensive when digging.
			adjCost.state = static_cast<void *>(node->LeftUp);
			adjacentList->push_back(adjCost);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float PathFinder::GetMaterialTransitionCost(const Material &material) const {
		float strength = material.GetIntegrity();
		// Always treat doors as diggable.
		if (strength > m_DigStrength && material.GetIndex() != MaterialColorKeys::g_MaterialDoor) {
			strength *= 1000.0F;
		}
		return strength;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Material * PathFinder::StrongestMaterialAlongLine(const Vector &start, const Vector &end) const {
		return g_SceneMan.CastMaxStrengthRayMaterial(start, end, 0, MaterialColorKeys::g_MaterialAir);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PathFinder::UpdateNodeCosts(PathNode *node) const {
		if (!node) {
			return false;
		}

		std::array<const Material *, PathNode::c_MaxAdjacentNodeCount> oldMaterials = node->AdjacentNodeBlockingMaterials;

		auto getStrongerMaterial = [](const Material *first, const Material *second) {
			return first->GetIntegrity() > second->GetIntegrity() ? first : second;
		};

		// Look at each existing adjacent node and calculate the cost for each. Start and end are offset to cover more terrain.
		// Note that we only calculate transitions to one side (down and right), because for the other side we can pull our up-and-left transition data from the other node's down-and-right.
		if (node->Right) {
			Vector offset(0.0F, 3.0F);
			node->RightMaterial = getStrongerMaterial(StrongestMaterialAlongLine(node->Pos - offset, node->Right->Pos - offset), StrongestMaterialAlongLine(node->Pos + offset, node->Right->Pos + offset));
		}
		if (node->Down) {
			Vector offset(3.0F, 0.0F);
			node->DownMaterial = getStrongerMaterial(StrongestMaterialAlongLine(node->Pos - offset, node->Down->Pos - offset), StrongestMaterialAlongLine(node->Pos + offset, node->Down->Pos + offset));
		}
		if (node->UpRight) {
			Vector offset(2.0F, 2.0F);
			node->UpRightMaterial = getStrongerMaterial(StrongestMaterialAlongLine(node->Pos - offset, node->UpRight->Pos - offset), StrongestMaterialAlongLine(node->Pos + offset, node->UpRight->Pos + offset));
		}
		if (node->RightDown) {
			Vector offset(2.0F, -2.0F);
			node->RightDownMaterial = getStrongerMaterial(StrongestMaterialAlongLine(node->Pos - offset, node->RightDown->Pos - offset), StrongestMaterialAlongLine(node->Pos + offset, node->RightDown->Pos + offset));
		}

		for (int i = 0; i < PathNode::c_MaxAdjacentNodeCount; ++i) {
			const Material *oldMat = oldMaterials[i];
			const Material *newMat = node->AdjacentNodeBlockingMaterials[i];

			// Check if the material strength is more than our delta, or if a door has appeared/disappeared (since we handle their costs in a special manner).
			float delta = std::abs(oldMat->GetIntegrity() - newMat->GetIntegrity());
			bool doorChanged = oldMat != newMat && (oldMat->GetIndex() == MaterialColorKeys::g_MaterialDoor || newMat->GetIndex() == MaterialColorKeys::g_MaterialDoor);
			if (delta > c_NodeCostChangeEpsilon || doorChanged) {
				return true;
			}
		}

		// None of the updates was past our epsilon, so ignore it and pretend it never happened.
		node->AdjacentNodeBlockingMaterials = oldMaterials;
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<int> PathFinder::GetNodeIdsInBox(Box box) {
		std::vector<int> result;

		box.Unflip();

		// Get the extents of the box's potential influence on PathNodes and their connecting edges.
		int firstX = static_cast<int>(std::floor((box.m_Corner.m_X / static_cast<float>(m_NodeDimension)) + 0.5F) - 1);
		int lastX = static_cast<int>(std::floor(((box.m_Corner.m_X + box.m_Width) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1);
		int firstY = static_cast<int>(std::floor((box.m_Corner.m_Y / static_cast<float>(m_NodeDimension)) + 0.5F) - 1);
		int lastY = static_cast<int>(std::floor(((box.m_Corner.m_Y + box.m_Height) / static_cast<float>(m_NodeDimension)) + 0.5F) + 1);

		// Only iterate through the grid where the box overlaps any edges.
		for (int nodeX = firstX; nodeX <= lastX; ++nodeX) {
			for (int nodeY = firstY; nodeY <= lastY; ++nodeY) {
				int nodeId = ConvertCoordsToNodeId(nodeX, nodeY);
				if (nodeId != -1) {
					result.push_back(nodeId);
				}
			}
		}

		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	float PathFinder::GetNodeAverageTransitionCost(const PathNode &node) const {
		float totalCostOfAdjacentNodes = 0.0F;
		int count = 0;
		for (const Material *material : node.AdjacentNodeBlockingMaterials) {
			// Don't use node transition cost, because we don't care about digging.
			float cost = material->GetIntegrity();
			if (cost < std::numeric_limits<float>::max()) {
				totalCostOfAdjacentNodes += cost;
				count++;
			}
		}
		return totalCostOfAdjacentNodes / std::max(static_cast<float>(count), 1.0F);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PathFinder::UpdateNodeList(const std::vector<int> &nodeVec) {
		std::atomic<bool> anyChange = false;

		// Update all the costs going out from each node.
		std::for_each(
			std::execution::par_unseq,
			nodeVec.begin(),
			nodeVec.end(),
			[this, &anyChange](int nodeId) {
				if (UpdateNodeCosts(&m_NodeGrid[nodeId])) {
					anyChange = true;
				}
			}
		);

		if (anyChange) {
			// UpdateNodeCosts only calculates Materials for Right and Down directions, so each PathNode's Up and Left direction Materials need to be matched to the respective neighbor's opposite direction Materials.
			// For example, this PathNode's Left Material is its Left neighbor's Right Material.
			std::for_each(
				std::execution::par_unseq,
				nodeVec.begin(),
				nodeVec.end(),
				[this](int nodeId) {
					PathNode *node = &m_NodeGrid[nodeId];
					if (node->Right) { node->Right->LeftMaterial = node->RightMaterial; }
					if (node->Down) { node->Down->UpMaterial = node->DownMaterial; }
					if (node->UpRight) { node->UpRight->DownLeftMaterial = node->UpRightMaterial; }
					if (node->RightDown) { node->RightDown->LeftUpMaterial = node->RightDownMaterial; }
				}
			);

			m_Pather->Reset();
		}

		return anyChange;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PathNode * PathFinder::GetPathNodeAtGridCoords(int x, int y) {
		int nodeId = ConvertCoordsToNodeId(x, y);
		return nodeId != -1 ? &m_NodeGrid[nodeId] : nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PathFinder::ConvertCoordsToNodeId(int x, int y) {
		if (m_WrapsX) {
			x = x % m_GridWidth;
			x = x < 0 ? x + m_GridWidth : x;
		}

		if (m_WrapsY) {
			y = y % m_GridHeight;
			y = y < 0 ? y + m_GridHeight : y;
		}

		if (x < 0 || x >= m_GridWidth || y < 0 || y >= m_GridHeight) {
			return -1;
		}

		return (y * m_GridWidth) + x;
	}
}
