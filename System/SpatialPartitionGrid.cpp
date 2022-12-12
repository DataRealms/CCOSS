#include "SpatialPartitionGrid.h"

#include "Box.h"
#include "MovableObject.h"

namespace RTE {

	void SpatialPartitionGrid::Clear() {
		m_Width = 0;
		m_Height = 0;
		m_CellSize = 0;
		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			m_Cells[team + 1].clear();
		}
		m_UsedCells.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::Create(int width, int height, int cellSize) {
		m_Width = width / cellSize;
		m_Height = height / cellSize;
		m_CellSize = cellSize;
		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			m_Cells[team + 1].resize(m_Width * m_Height);
		}
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::Create(const SpatialPartitionGrid &reference) {
		m_Width = reference.m_Width;
		m_Height = reference.m_Height;
		m_CellSize = reference.m_CellSize;
		m_Cells = reference.m_Cells;
		m_UsedCells = reference.m_UsedCells;
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SpatialPartitionGrid::Reset() {
		const Activity* activity = g_ActivityMan.GetActivity();
		RTEAssert(activity, "Spatial partition grid with no running activity!");

		for (int team = Activity::NoTeam; team < Activity::MaxTeamCount; ++team) {
			bool teamActive = team == Activity::NoTeam || activity->TeamActive(team);
			if (!teamActive) {
				continue;
			}

			for (int usedCell : m_UsedCells) {
				MOIDList& moidList = m_Cells[team+1][usedCell];
				moidList.clear();
			}
		}

		m_UsedCells.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void SpatialPartitionGrid::Add(const IntRect &rect, const MovableObject &mo) {
		const Activity *activity = g_ActivityMan.GetActivity();
		RTEAssert(activity, "Spatial partition grid with no running activity!");

		const MovableObject &rootParentMo = *mo.GetRootParent();
		if (!rootParentMo.GetsHitByMOs() && !rootParentMo.HitsMOs()) {
			return;
		}

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
					m_Cells[team + 1][cellId].push_back(mo.GetID());
					m_UsedCells.insert(cellId);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SpatialPartitionGrid::MOIDList SpatialPartitionGrid::GetMOIDsInArea(const IntRect &rect, int ignoreTeam) const {
		MOIDList moidList;

		int topLeftCellX = rect.m_Left / m_CellSize;
		int topLeftCellY = rect.m_Top / m_CellSize;
		int bottomRightCellX = rect.m_Right / m_CellSize;
		int bottomRightCellY = rect.m_Bottom / m_CellSize;
		for (int x = topLeftCellX; x <= bottomRightCellX; x++) {
			for (int y = topLeftCellY; y <= bottomRightCellY; y++) {
				const MOIDList &moidsInCell = m_Cells[ignoreTeam + 1][GetCellIdForCellCoords(x, y)];
				moidList.insert(moidList.end(), moidsInCell.begin(), moidsInCell.end());
			}
		}

		return moidList;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const SpatialPartitionGrid::MOIDList& SpatialPartitionGrid::GetMOIDsAtPosition(int x, int y, int ignoreTeam) const {
		int cellX = x / m_CellSize;
		int cellY = y / m_CellSize;

		// Lua sometimes decides to give SceneMan an ignoreTeam value of... -2.
		// Yeah, seriously.
		// So let's sanity check this shit.
		ignoreTeam = ignoreTeam < Activity::NoTeam || ignoreTeam > Activity::MaxTeamCount ? Activity::NoTeam : ignoreTeam;

		return m_Cells[ignoreTeam + 1][GetCellIdForCellCoords(cellX, cellY)];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int SpatialPartitionGrid::GetCellIdForCellCoords(int cellX, int cellY) const {
		// We act like we wrap, even if the scene doesn't. The only cost is some duplicate collision checks, but that's a minor cost to pay :)
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