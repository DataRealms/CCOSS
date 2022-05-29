#ifndef _RTETERRAINOBJECT_
#define _RTETERRAINOBJECT_

#include "SceneObject.h"
#include "ContentFile.h"

namespace RTE {

	class SLTerrain;

	/// <summary>
	/// A feature of the terrain, which includes foreground color layer, material layer and optional background layer.
	/// </summary>
	class TerrainObject : public SceneObject {

	public:

		EntityAllocation(TerrainObject);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TerrainObject object in system memory. Create() should be called before using the object.
		/// </summary>
		TerrainObject() { Clear(); }

		/// <summary>
		/// Makes the TerrainObject object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a TerrainObject to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the TerrainObject to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const TerrainObject &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a TerrainObject object before deletion from system memory.
		/// </summary>
		~TerrainObject() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the TerrainObject object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override { if (!notInherited) { SceneObject::Destroy(); } Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Returns whether this TerrainObject has any foreground color data.
		/// </summary>
		/// <returns>Whether this TerrainOBject has any foreground color data.</returns>
		bool HasFGColorBitmap() const { return m_FGColorBitmap != nullptr; }

		/// <summary>
		/// Gets the BITMAP that this TerrainObject uses for its foreground color representation.
		/// </summary>
		/// <returns>A pointer to the foreground color BITMAP object. Ownership is NOT transferred!</returns>
		BITMAP * GetFGColorBitmap() const { return m_FGColorBitmap; }

		/// <summary>
		/// Returns whether this TerrainObject has any background color data.
		/// </summary>
		/// <returns>Whether this TerrainOBject has any background color data.</returns>
		bool HasBGColorBitmap() const { return m_BGColorBitmap != nullptr; }

		/// <summary>
		/// Gets the BITMAP that this TerrainObject uses for its background color representation, if any.
		/// </summary>
		/// <returns>A pointer to the background color BITMAP object. This may be nullptr if there is no BG bitmap. Ownership is NOT transferred!</returns>
		BITMAP * GetBGColorBitmap() const { return m_BGColorBitmap; }

		/// <summary>
		/// Returns whether this TerrainObject has any material data.
		/// </summary>
		/// <returns>Whether this TerrainOBject has any material data.</returns>
		bool HasMaterialBitmap() const { return m_MaterialBitmap != nullptr; }

		/// <summary>
		/// Gets the BITMAP that this TerrainObject uses for its material representation, if any.
		/// </summary>
		/// <returns>A pointer to the material BITMAP object. Ownership is NOT transferred!</returns>
		BITMAP * GetMaterialBitmap() const { return m_MaterialBitmap; }

		/// <summary>
		/// Gets the offset from the position to the upper left corner of this TerrainObject's BITMAPs.
		/// </summary>
		/// <returns>A Vector describing the bitmap offset, in pixels.</returns>
		const Vector & GetBitmapOffset() const { return m_BitmapOffset; }

		/// <summary>
		/// Gets the width of this TerrainObject's BITMAP. All layers of the TerrainObject are checked for cases where one or two of the layers are not defined.
		/// </summary>
		/// <returns>The width of this TerrainObject.</returns>
		int GetBitmapWidth() const;

		/// <summary>
		/// Gets the height of this TerrainObject. All layers of the TerrainObject are checked for cases where one or two of the layers are not defined.
		/// </summary>
		/// <returns>The height of this TerrainObject.</returns>
		int GetBitmapHeight() const;

		/// <summary>
		/// Gets the list of child objects that should be placed when this TerrainObject is placed.
		/// </summary>
		/// <returns>A reference to the list of child objects. Ownership of the list is NOT transferred!</returns>
		const std::vector<SceneObject::SOPlacer> & GetChildObjects() const { return m_ChildObjects; }

		/// <summary>
		/// Gets a BITMAP showing a good identifiable icon of this, for use in GUI lists.
		/// </summary>
		/// <returns>A good identifiable graphical representation of this in a BITMAP, if available. If not, nullptr is returned. Ownership is NOT transferred!</returns>
		BITMAP * GetGraphicalIcon() const override;

		/// <summary>
		/// Sets which team this TerrainObject belongs to.
		/// </summary>
		/// <param name="team">The assigned team number.</param>
		void SetTeam(int team) override;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Places this TerrainObject and all its children on the specified SLTerrain.
		/// </summary>
		/// <param name="terrain">The SLTerrain to place this TerrainObject on. Ownership is NOT transferred!</param>
		/// <returns>Whether the object was successfully placed on the terrain.</returns>
		bool PlaceOnTerrain(SLTerrain *terrain);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Indicates whether this TerrainObject's current graphical representation overlaps a point in absolute scene coordinates.
		/// </summary>
		/// <param name="scenePoint">The point in absolute scene coordinates.</param>
		/// <returns>Whether this' graphical rep overlaps the scene point.</returns>
		bool IsOnScenePoint(Vector &scenePoint) const override;

		/// <summary>
		/// Draws this TerrainObject's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">In which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this TerrainObject, like indicator arrows or hovering HUD text and so on.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ContentFile m_FGColorFile; //!< ContentFile containing the path to this TerrainObject's foreground color layer representation.
		BITMAP *m_FGColorBitmap; //!< Foreground color BITMAP of this TerrainObject.

		ContentFile m_BGColorFile; //!< ContentFile containing the path to this TerrainObject's background color layer representation.
		BITMAP *m_BGColorBitmap; //!< Background color BITMAP of this TerrainObject.

		ContentFile m_MaterialFile; //!< ContentFile containing the path to this TerrainObject's Material layer representation.
		BITMAP *m_MaterialBitmap; //!< Material BITMAP of this TerrainObject.

		Vector m_BitmapOffset; //!< Offset from the position of this to the top left corner of the bitmap. The inversion of this should point to a corner or pattern in the bitmaps which will snap well with a 24px grid.
		bool m_OffsetDefined; //!< Whether the offset has been defined and shouldn't be automatically set.

		std::vector<SceneObject::SOPlacer> m_ChildObjects; //!< The objects that are placed along with this TerrainObject on the Scene.

	private:

		/// <summary>
		/// Draws this TerrainObject's graphical and material representations to the specified SLTerrain's respective layers.
		/// </summary>
		/// <param name="terrain">The SLTerrain to draw this TerrainObject to. Ownership is NOT transferred!</param>
		void DrawToTerrain(SLTerrain *terrain);

		/// <summary>
		/// Clears all the member variables of this TerrainObject, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TerrainObject(const TerrainObject &reference) = delete;
		void operator=(const TerrainObject &rhs) = delete;
	};
}
#endif