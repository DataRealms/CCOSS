#ifndef _RTETERRAINDEBRIS_
#define _RTETERRAINDEBRIS_

#include "Entity.h"
#include "Material.h"

namespace RTE {

	class SLTerrain;

	/// <summary>
	/// Specifies a certain type of debris scattered randomly throughout a Terrain, before TerrainObjects are placed.
	/// </summary>
	class TerrainDebris : public Entity {

	public:

		EntityAllocation(TerrainDebris)
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TerrainDebris object in system memory. Create() should be called before using the object.
		/// </summary>
		TerrainDebris() { Clear(); }

		/// <summary>
		/// Copy constructor method used to instantiate a TerrainDebris object identical to an already existing one.
		/// </summary>
		/// <param name="reference">A TerrainDebris object which is passed in by reference.</param>
		TerrainDebris(const TerrainDebris &reference) { Clear(); Create(reference); }

		/// <summary>
		/// Makes the TerrainDebris object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();

		/// <summary>
		/// Creates a TerrainDebris to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the TerrainDebris to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const TerrainDebris &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a TerrainDebris object before deletion from system memory.
		/// </summary>
		virtual ~TerrainDebris() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the TerrainDebris object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		/// <remarks>Don't delete bitmaps since they are owned in the CoententFile static maps.</remarks>
		virtual void Destroy(bool notInherited = false) { if (!notInherited) { Entity::Destroy(); } Clear(); }

		/// <summary>
		/// Resets the entire TerrainDebris, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region INI Handling
		/// <summary>
		/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
		/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
		/// </summary>
		/// <param name="propName">The name of the property to be read.</param>
		/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
		/// <returns>
		/// An error return value signaling whether the property was successfully read or not.
		/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
		/// </returns>
		virtual int ReadProperty(std::string propName, Reader &reader);

		/// <summary>
		/// Saves the complete state of this TerrainDebris to an output stream for later recreation with Create(Reader &reader).
		/// </summary>
		/// <param name="writer">A Writer that the TerrainDebris will save itself with.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets all the bitmaps of this collection of debris.
		/// </summary>
		/// <returns>An array of pointers to BITMAPs. Ownership is NOT transferred.</returns>
		BITMAP ** GetBitmaps() const { return m_Bitmaps; }

		/// <summary>
		/// Gets the current number of BITMAPs that describe all the different pieces of debris.
		/// </summary>
		/// <returns>The number of BITMAPs this TerrainDebris has.</returns>
		int GetBitmapCount() const { return m_BitmapCount; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Applies the debris to an SLTerrain as its read parameters specify.
		/// </summary>
		/// <param name="pTerrain">Pointer to the terrain to place the debris on. Ownership is NOT transferred!</param>
		void ApplyDebris(SLTerrain *terrain);
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ContentFile m_DebrisFile; //!< Shows where the bitmaps are	
		BITMAP **m_Bitmaps; //!< All the different bitmaps for each chunk of debris. Not owned.	
		unsigned short m_BitmapCount; //!< How many bitmaps we have loaded

		Material m_Material; //!< The material of all this debris	
		Material m_TargetMaterial; //!< The target material in which this debris should only exist in

		bool m_OnlyOnSurface; //!< Whether to only place if the target material is exposed on the surface of the terrain. If false, checking will continue to penetrate down into non-air materials to try to find the target material.	
		bool m_OnlyBuried; //!< Whether to only place a piece of this if we find a spot for it to fit completely buried int eh terrain

		int m_MinDepth; //!< Minimum depth into the terrain contour. This can be negative for debris placed above ground
		int m_MaxDepth; //!< Max depth into the terrain contour. This can be negative for debris placed above ground

		float m_Density; //!< Approximate Density count per meter

	private:

		/// <summary>
		/// Clears all the member variables of this TerrainDebris, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif