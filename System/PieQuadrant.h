#ifndef _RTEPIEQUADRANT_
#define _RTEPIEQUADRANT_

#include "PieSlice.h"

namespace RTE {

	/// <summary>
	///
	/// </summary>
	class PieQuadrant {

	public:

		static constexpr int c_PieQuadrantSlotCount = 5; //!< The maximum number of PieSlices a PieQuadrant can have.
		static constexpr float c_PieSliceSlotSize = c_HalfPI / static_cast<float>(c_PieQuadrantSlotCount); //!< The size of one PieSlice slot in PieQuadrants.

		bool m_Enabled; //!< Whether this PieQuadrant is enabled and visible or disabled.
		Directions m_Direction; //!< The direction of this PieQuadrant.

		std::unique_ptr<PieSlice> m_MiddlePieSlice; //!< A unique_ptr to the middle PieSlice of this PieQuadrant.
		std::array<std::unique_ptr<PieSlice>, c_PieQuadrantSlotCount / 2> m_LeftPieSlices; //!< An array of unique_ptrs to the left side PieSlices of this PieQuadrant.
		std::array<std::unique_ptr<PieSlice>, c_PieQuadrantSlotCount / 2> m_RightPieSlices; //!< An array of unique_ptrs to the right side PieSlices of this PieQuadrant.
		std::array<const PieSlice *, c_PieQuadrantSlotCount> m_SlotsForPieSlices; //!< An array representing the slots in this PieQuadrant, via pointers to the PieSlices filling each slot.

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PieQuadrant object in system memory and make it ready for use.
		/// </summary>
		PieQuadrant() { Clear(); }

		/// <summary>
		///  Creates a PieQuadrant to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the PieQuadrant to deep copy.</param>
		void Create(const PieQuadrant &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets this PieQuadrant and deletes all slices in it.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets whether or not this PieQuadrant contains the given PieSlice.
		/// </summary>
		/// <returns>Whether or not this PieQuadrant contains the given PieSlice.</returns>
		bool ContainsPieSlice(const PieSlice *sliceToCheck) const { return sliceToCheck == m_MiddlePieSlice.get() || sliceToCheck == m_LeftPieSlices[0].get() || sliceToCheck == m_RightPieSlices[0].get() || sliceToCheck == m_LeftPieSlices[1].get() || sliceToCheck == m_RightPieSlices[1].get(); }

		/// <summary>
		/// Gets a vector of non-owning pointers to the PieSlices in this PieQuadrant.
		/// The vector of PieSlices will default to INI order, i.e. starting with the middle and interleaving left and right slices in order.
		/// Alternatively it can go in CCW order, getting the outermost right slice and moving inwards through the middle and then left slices.
		/// </summary>
		/// <param name="inCCWOrder">Whether to get flattened slices in counter-clockwise order. Defaults to false.</param>
		std::vector<PieSlice *> GetFlattenedPieSlices(bool inCCWOrder = false) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Balances this PieQuadrant be removing and re-adding all PieSlices if needed, then aligns all PieSlices in this PieQuadrant, setting their angle and size details.
		/// </summary>
		void RealignPieSlices();

		/// <summary>
		/// Adds the PieSlice to the quadrant. PieSlices are added to the middle, then the left side, then the right side so things fill evenly. Ownership IS transferred!
		/// </summary>
		bool AddPieSlice(PieSlice *pieSliceToAdd);

		/// <summary>
		/// Removes the passed in PieSlice from this PieQuadrant. Ownership IS transferred to the caller!
		/// </summary>
		/// <param name="pieSliceToRemove">The PieSlice to be removed from this PieQuadrant. Ownership IS transferred to the caller!</param>
		PieSlice * RemovePieSlice(const PieSlice *pieSliceToRemove);
#pragma endregion

	private:

		/// <summary>
		/// Clears all the member variables of this PieQuadrant.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PieQuadrant(const PieQuadrant &reference) = delete;
		PieQuadrant & operator=(const PieQuadrant &rhs) = delete;
	};
}
#endif