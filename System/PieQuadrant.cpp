#include "PieQuadrant.h"

#include "RTETools.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieQuadrant::Clear() {
		m_Enabled = true;
		m_Direction = Directions::None;
		m_MiddlePieSlice.reset();
		for (std::unique_ptr<PieSlice> &pieSlice : m_LeftPieSlices) {
			pieSlice.reset();
		}
		for (std::unique_ptr<PieSlice> &pieSlice : m_RightPieSlices) {
			pieSlice.reset();
		}
		m_SlotsForPieSlices.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieQuadrant::Create(const PieQuadrant &reference, const Entity *oldOriginalPieSliceSourceToCheck, const Entity *newOriginalPieSliceSourceToSet) {
		m_Enabled = reference.m_Enabled;
		m_Direction = reference.m_Direction;
		for (const PieSlice *referencePieSlice : reference.GetFlattenedPieSlices()) {
			PieSlice *pieSliceToAdd = dynamic_cast<PieSlice *>(referencePieSlice->Clone());
			if (referencePieSlice->GetOriginalSource() == oldOriginalPieSliceSourceToCheck) {
				pieSliceToAdd->SetOriginalSource(newOriginalPieSliceSourceToSet);
			}
			AddPieSlice(pieSliceToAdd);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<PieSlice *> PieQuadrant::GetFlattenedPieSlices(bool inCCWOrder) const {
		std::vector<PieSlice *> pieSlices;
		if (inCCWOrder) {
			for (auto pieSliceReverseIterator = m_RightPieSlices.crbegin(); pieSliceReverseIterator < m_RightPieSlices.crend(); pieSliceReverseIterator++) {
				if (*pieSliceReverseIterator) { pieSlices.emplace_back((*pieSliceReverseIterator).get()); }
			}
			if (m_MiddlePieSlice) { pieSlices.emplace_back(m_MiddlePieSlice.get()); }
			for (const std::unique_ptr<PieSlice> &pieSlice : m_LeftPieSlices) {
				if (pieSlice) { pieSlices.emplace_back(pieSlice.get()); }
			}
		} else {
			if (m_MiddlePieSlice) { pieSlices.emplace_back(m_MiddlePieSlice.get()); }
			const std::array<std::unique_ptr<PieSlice>, 2> &firstSliceArray = (m_Direction == Directions::Left) ? m_RightPieSlices : m_LeftPieSlices;
			const std::array<std::unique_ptr<PieSlice>, 2> &secondSliceArray = (m_Direction == Directions::Left) ? m_LeftPieSlices : m_RightPieSlices;
			for (int i = 0; i < firstSliceArray.size(); i++) {
				if (firstSliceArray[i]) { pieSlices.emplace_back(firstSliceArray[i].get()); }
				if (secondSliceArray[i]) { pieSlices.emplace_back(secondSliceArray[i].get()); }
			}
		}
		return pieSlices;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PieQuadrant::RealignPieSlices() {
		if ((m_RightPieSlices[0] && !m_LeftPieSlices[0]) || (m_RightPieSlices[1] && !m_LeftPieSlices[1])) {
			for (const PieSlice *pieSliceToRealign : GetFlattenedPieSlices()) {
				AddPieSlice(RemovePieSlice(pieSliceToRealign));
			}
		}

		m_SlotsForPieSlices.fill(nullptr);
		std::vector<PieSlice *> pieSlices = GetFlattenedPieSlices(true);
		int oddRoundedSliceCount = (2 * static_cast<int>(pieSlices.size() / 2)) + 1;
		float angleOffset = NormalizeAngleBetween0And2PI(c_DirectionsToRadiansMap.at(m_Direction) - c_QuarterPI);

		int currentSlot = 0;
		for (PieSlice *pieSlice : pieSlices) {
			int sliceSlotCount = 1;
			if (oddRoundedSliceCount < c_PieQuadrantSlotCount && pieSlice == m_MiddlePieSlice.get()) { sliceSlotCount = (oddRoundedSliceCount == 1) ? c_PieQuadrantSlotCount : c_PieQuadrantSlotCount - 2; }
			if ((currentSlot == 0 || m_SlotsForPieSlices.at(currentSlot - 1) == nullptr) && pieSlices.size() % 2 == 0) {
				currentSlot++;
				angleOffset = NormalizeAngleBetween0And2PI(angleOffset + c_PieSliceSlotSize);
			}

			pieSlice->SetStartAngle(angleOffset);
			pieSlice->SetSlotCount(sliceSlotCount);

			for (int slot = currentSlot; slot < currentSlot + sliceSlotCount; slot++) {
				m_SlotsForPieSlices.at(slot) = pieSlice;
			}
			currentSlot += sliceSlotCount;
			angleOffset = NormalizeAngleBetween0And2PI(angleOffset + (c_PieSliceSlotSize * static_cast<float>(pieSlice->GetSlotCount())));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PieQuadrant::AddPieSlice(PieSlice *pieSliceToAdd) {
		if (!pieSliceToAdd) {
			return false;
		}
		bool sliceWasAdded = false;
		if (!m_MiddlePieSlice && pieSliceToAdd->GetCanBeMiddleSlice()) {
			m_MiddlePieSlice = std::unique_ptr<PieSlice>(pieSliceToAdd);
			sliceWasAdded = true;
		}
		if (!sliceWasAdded && (!m_LeftPieSlices[1] || !m_RightPieSlices[1])) {
			bool bothSidesEqual = ((!m_LeftPieSlices[0] && !m_RightPieSlices[0]) || (m_LeftPieSlices[0] && m_RightPieSlices[0])) && ((!m_LeftPieSlices[1] && !m_RightPieSlices[1]) || (m_LeftPieSlices[1] && m_RightPieSlices[1]));
			bool leftSideHasMoreSlices = !bothSidesEqual && ((m_LeftPieSlices[0] && !m_RightPieSlices[0]) || m_LeftPieSlices[1]);

			std::array<std::unique_ptr<PieSlice>, 2> &sliceArrayToAddTo = leftSideHasMoreSlices ? m_RightPieSlices : m_LeftPieSlices;
			sliceArrayToAddTo[sliceArrayToAddTo[0] ? 1 : 0] = std::unique_ptr<PieSlice>(pieSliceToAdd);
			sliceWasAdded = true;
		}

		return sliceWasAdded;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PieSlice * PieQuadrant::RemovePieSlice(const PieSlice *pieSliceToRemove) {
		if (pieSliceToRemove == m_MiddlePieSlice.get()) {
			return m_MiddlePieSlice.release();
		}
		for (std::unique_ptr<PieSlice> &pieSlice : m_LeftPieSlices) {
			if (pieSliceToRemove == pieSlice.get()) {
				return pieSlice.release();
			}
		}
		for (std::unique_ptr<PieSlice> &pieSlice : m_RightPieSlices) {
			if (pieSliceToRemove == pieSlice.get()) {
				return pieSlice.release();
			}
		}
		return nullptr;
	}
}