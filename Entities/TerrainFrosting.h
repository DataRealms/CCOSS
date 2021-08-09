#ifndef _RTETERRAINFROSTING_
#define _RTETERRAINFROSTING_

#include "Material.h"
#include "RTETools.h"

namespace RTE {

	class SLTerrain;

	/// <summary>
	/// Specifies a 'frosting' layer of material on top of another material in the terrain. Gets applied upon loading the scene.
	/// </summary>
	class TerrainFrosting : public Serializable {

	public:

		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TerrainFrosting object in system memory. Create() should be called before using the object.
		/// </summary>
		TerrainFrosting() { Clear(); }

		/// <summary>
		/// Constructor method used to instantiate a TerrainFrosting object identical to another in system memory, by deep copy.
		/// </summary>
		/// <param name="reference"> A reference to the TerrainFrosting to deep copy.</param>
		TerrainFrosting(const TerrainFrosting &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Creates a TerrainFrosting to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference"> A reference to the TerrainFrosting to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const TerrainFrosting &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire Serializable, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets the target Material that this TerrainFrosting will appear on top of.
		/// </summary>
		/// <returns>A reference to the target Material.</returns>
		const Material & GetTargetMaterial() const { return m_TargetMaterial; }

		/// <summary>
		/// Gets the Material of this TerrainFrosting.
		/// </summary>
		/// <returns>A reference to the frosting Material.</returns>
		const Material & GetFrostingMaterial() const { return m_FrostingMaterial; }

		/// <summary>
		/// Returns a random thickness sample between the min and max possible thicknesses of this TerrainFrosting.
		/// </summary>
		/// <returns>The thickness sample.</returns>
		int GetThicknessSample() const { return m_MinThickness + RandomNum(0, m_MaxThickness - m_MinThickness); }

		/// <summary>
		/// Indicates whether the frosting only appears where there is air in the terrain.
		/// </summary>
		/// <returns>Whether only appears in air particles.</returns>
		bool InAirOnly() const { return m_InAirOnly; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Applies the frosting to an SLTerrain as its read parameters specify.
		/// </summary>
		/// <param name="terrain">Pointer to the terrain to place the frosting on. Ownership is NOT transferred!</param>
		void ApplyFrosting(SLTerrain *terrain) const;
#pragma endregion

	protected:

		Material m_TargetMaterial; //!< The material this frosting will sit on top in the terrain.
		Material m_FrostingMaterial; //!< Material of this frosting that will be piled on top of the target.
		int m_MinThickness; //!< The minimum height in pixels above the target material.
		int m_MaxThickness; //!< The max thickness.
		bool m_InAirOnly; //!< Whether the frosting only appears where there is air.

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// <summary>
		/// Clears all the member variables of this TerrainFrosting, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif