#ifndef _RTETERRAINFROSTING_
#define _RTETERRAINFROSTING_

#include "Material.h"

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
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire TerrainFrosting, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Applies the frosting to a SLTerrain as its read parameters specify.
		/// </summary>
		/// <param name="terrain">Pointer to the terrain to place the frosting on. Ownership is NOT transferred!</param>
		void ApplyFrosting(SLTerrain *terrain) const;
#pragma endregion

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		Material m_FrostingMaterial; //!< Material of this frosting that will be piled on top of the target.
		Material m_TargetMaterial; //!< The material this frosting will sit on top in the terrain.
		int m_MinThickness; //!< The minimum height in pixels above the target material.
		int m_MaxThickness; //!< The maximum height in pixels above the target material.
		bool m_InAirOnly; //!< Whether the frosting only appears where there is air (i.e. does not appear where the terrain background layer is showing).

		/// <summary>
		/// Clears all the member variables of this TerrainFrosting, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TerrainFrosting(const TerrainFrosting &reference) = delete;
		void operator=(const TerrainFrosting &rhs) = delete;
	};
}
#endif