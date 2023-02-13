#include "SpatialPartitionGrid.h"

#include "Box.h"
#include "MovableObject.h"
#include "SceneMan.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SpatialPartitionGrid::Clear() {
		m_Width = 0;
		m_Height = 0;
		m_CellSize = 0;
		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			m_Cells[team + 1].clear();
			m_PhysicsCells[team + 1].clear();
		}
		m_UsedCellIds.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::Create(int width, int height, int cellSize) {
		m_Width = width / cellSize;
		m_Height = height / cellSize;
		m_CellSize = cellSize;
		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			m_Cells[team + 1].resize(m_Width * m_Height);
			m_PhysicsCells[team + 1].resize(m_Width * m_Height);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::Create(const SpatialPartitionGrid &reference) {
		m_Width = reference.m_Width;
		m_Height = reference.m_Height;
		m_CellSize = reference.m_CellSize;
		m_Cells = reference.m_Cells;
		m_PhysicsCells = reference.m_PhysicsCells;
		m_UsedCellIds = reference.m_UsedCellIds;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SpatialPartitionGrid::Reset() {
		const Activity *activity = g_ActivityMan.GetActivity();
		RTEAssert(activity, "Tried to reset spatial partition grid with no running Activity!");

		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			if (team == Activity::NoTeam || activity->TeamActive(team)) {
				for (int usedCellId : m_UsedCellIds) {
					m_Cells[team + 1][usedCellId].clear();
					m_PhysicsCells[team + 1][usedCellId].clear();
				}
			}
		}

		m_UsedCellIds.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SpatialPartitionGrid::Add(const IntRect &rect, const MovableObject &mo) {
		const Activity *activity = g_ActivityMan.GetActivity();
		RTEAssert(activity, "Tried to add to spatial partition grid with no running Activity!");

		const MovableObject &rootParentMo = *mo.GetRootParent();
		int topLeftCellX = rect.m_Left / m_CellSize;
		int topLeftCellY = rect.m_Top / m_CellSize;
		int bottomRightCellX = rect.m_Right / m_CellSize;
		int bottomRightCellY = rect.m_Bottom / m_CellSize;

		// We handle wrapping in GetCellIdForCellCoords, so make sure we've not already been passed wrapped data...
		RTEAssert(topLeftCellX <= bottomRightCellX && topLeftCellY <= bottomRightCellY, "Invalidly wrapped rect passed to spatial partitioning grid!");

		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			bool teamActive = team == Activity::NoTeam || activity->TeamActive(team);
			bool ignoresThisTeam = team != Activity::NoTeam && rootParentMo.IgnoresTeamHits() && team == rootParentMo.GetTeam();
			if (!teamActive || ignoresThisTeam) {
				continue;
			}

			for (int x = topLeftCellX; x <= bottomRightCellX; x++) {
				for (int y = topLeftCellY; y <= bottomRightCellY; y++) {
					int cellId = GetCellIdForCellCoords(x, y);
					m_UsedCellIds.insert(cellId);

					m_Cells[team + 1][cellId].push_back(mo.GetID());
					if (mo.GetsHitByMOs()) {
						m_PhysicsCells[team + 1][cellId].push_back(mo.GetID());
					}
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<MovableObject *> SpatialPartitionGrid::GetMOsInBox(const Box &box, int ignoreTeam, bool getsHitByMOsOnly) const {
		RTEAssert(ignoreTeam >= Activity::NoTeam && ignoreTeam < Activity::MaxTeamCount, "Invalid ignoreTeam given to SpatialPartitioningGrid::GetMOsInBox()!");

		std::unordered_set<MOID> potentialMOIDs;

		Vector topLeft = box.GetCorner();
		Vector bottomRight = topLeft + Vector(box.GetWidth(), box.GetHeight());

		int topLeftCellX = static_cast<int>(std::floor(topLeft.m_X / static_cast<float>(m_CellSize)));
		int topLeftCellY = static_cast<int>(std::floor(topLeft.m_Y / static_cast<float>(m_CellSize)));
		int bottomRightCellX = static_cast<int>(std::floor(bottomRight.m_X / static_cast<float>(m_CellSize)));
		int bottomRightCellY = static_cast<int>(std::floor(bottomRight.m_Y / static_cast<float>(m_CellSize)));

		// Note - GetCellIdForCellCoords accounts for wrapping automatically, so we don't have to deal with it here.
		auto& cells = getsHitByMOsOnly ? m_PhysicsCells : m_Cells;
		for (int x = topLeftCellX; x <= bottomRightCellX; x++) {
			for (int y = topLeftCellY; y <= bottomRightCellY; y++) {
				const std::vector<MOID> &moidsInCell = cells[ignoreTeam + 1][GetCellIdForCellCoords(x, y)];
				for (MOID moid : moidsInCell) {
					potentialMOIDs.insert(moid);
				}
			}
		}

		std::list<Box> wrappedBoxes;
		g_SceneMan.WrapBox(box, wrappedBoxes);

		std::vector<MovableObject *> MOList;
		for (MOID moid : potentialMOIDs) {
			MovableObject *mo = g_MovableMan.GetMOFromID(moid);
			if (mo && std::any_of(wrappedBoxes.begin(), wrappedBoxes.end(), [&mo](const Box &wrappedBox) { return wrappedBox.IsWithinBox(mo->GetPos()); })) {
				MOList.push_back(mo);
			}
		}

		return MOList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<MovableObject *> SpatialPartitionGrid::GetMOsInRadius(const Vector &center, float radius, int ignoreTeam, bool getsHitByMOsOnly) const {
		RTEAssert(ignoreTeam >= Activity::NoTeam && ignoreTeam < Activity::MaxTeamCount, "Invalid ignoreTeam given to SpatialPartitioningGrid::GetMOsInRadius()!");

		std::unordered_set<MOID> potentialMOIDs;

		int topLeftCellX = static_cast<int>(std::floor((center.m_X - radius) / static_cast<float>(m_CellSize)));
		int topLeftCellY = static_cast<int>(std::floor((center.m_Y - radius) / static_cast<float>(m_CellSize)));
		int bottomRightCellX = static_cast<int>(std::floor((center.m_X + radius) / static_cast<float>(m_CellSize)));
		int bottomRightCellY = static_cast<int>(std::floor((center.m_Y + radius) / static_cast<float>(m_CellSize)));

		// Note - GetCellIdForCellCoords accounts for wrapping automatically, so we don't have to deal with it here.
		auto& cells = getsHitByMOsOnly ? m_PhysicsCells : m_Cells;
		for (int x = topLeftCellX; x <= bottomRightCellX; x++) {
			for (int y = topLeftCellY; y <= bottomRightCellY; y++) {
				const std::vector<MOID> &moidsInCell = cells[ignoreTeam + 1][GetCellIdForCellCoords(x, y)];
				for (MOID moid : moidsInCell) {
					potentialMOIDs.insert(moid);
				}
			}
		}

		std::vector<MovableObject *> MOList;
		for (MOID moid : potentialMOIDs) {
			MovableObject *mo = g_MovableMan.GetMOFromID(moid);
			if (mo && !g_SceneMan.ShortestDistance(center, mo->GetPos()).MagnitudeIsGreaterThan(radius)) {
				MOList.push_back(mo);
			}
		}

		return MOList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<MOID> & SpatialPartitionGrid::GetMOIDsAtPosition(int x, int y, int ignoreTeam, bool getsHitByMOsOnly) const {
		int cellX = x / m_CellSize;
		int cellY = y / m_CellSize;

		// Lua sometimes decides to give SceneMan an ignoreTeam value of... -2.
		// Yeah, seriously.
		// So let's sanity check this shit.
		ignoreTeam = ignoreTeam < Activity::NoTeam || ignoreTeam > Activity::MaxTeamCount ? Activity::NoTeam : ignoreTeam;

		auto& cells = getsHitByMOsOnly ? m_PhysicsCells : m_Cells;
		return cells[ignoreTeam + 1][GetCellIdForCellCoords(cellX, cellY)];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::GetCellIdForCellCoords(int cellX, int cellY) const {
		// We act like we wrap, even if the Scene doesn't. The only cost is some duplicate collision checks, but that's a minor cost to pay :)
		int wrappedX = cellX % m_Width;
		if (wrappedX < 0) {
			wrappedX += m_Width;
		}
		int wrappedY = cellY % m_Height;
		if (wrappedY < 0) {
			wrappedY += m_Height;
		}
		return (wrappedY * m_Width) + wrappedX;
	}
}