#ifndef _RTESLTERRAIN_
#define _RTESLTERRAIN_

#include "SceneLayer.h"
#include "Matrix.h"
#include "Box.h"

namespace RTE {

	class MOPixel;
	class MovableObject;
	class TerrainFrosting;
	class TerrainObject;
	class TerrainDebris;

	/// <summary>
	/// The terrain of the RTE scene.
	/// </summary>
	class SLTerrain : public SceneLayer {

	public:

		EntityAllocation(SLTerrain)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SLTerrain object in system memory. Create() should be called before using the object.
		/// </summary>
		SLTerrain() { Clear(); }

		/// <summary>
		/// Makes the SLTerrain object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Creates a SLTerrain to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SLTerrain to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SLTerrain &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SLTerrain object before deletion from system memory.
		/// </summary>
		~SLTerrain() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SLTerrain object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;
#pragma endregion

#pragma region 
		/// <summary>
		/// Whether this' bitmap data is loaded from a file or was generated.
		/// </summary>
		/// <returns>Whether the data in this' bitmap was loaded from a datafile, or generated.</returns>
		bool IsFileData() const override { return m_FGColorLayer && m_FGColorLayer->IsFileData() && m_BGColorLayer && m_BGColorLayer->IsFileData(); }

		/// <summary>
		/// Actually loads previously specified/created data into memory. Has to be done before using this SceneLayer.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int LoadData() override;

		/// <summary>
		/// Saves data currently in memory to disk.
		/// </summary>
		/// <param name="pathBase">The filepath base to the where to save the Bitmap data. This means everything up to the extension. "FG" and "Mat" etc will be added.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int SaveData(const std::string &pathBase) override;

		/// <summary>
		/// Clears out any previously loaded bitmap data from memory.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int ClearData() override;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the foreground color bitmap of this SLTerrain.
		/// </summary>
		/// <returns>A pointer to the foreground color bitmap.</returns>
		BITMAP * GetFGColorBitmap() { return m_FGColorLayer->GetBitmap(); }

		/// <summary>
		/// Gets the background color bitmap of this SLTerrain.
		/// </summary>
		/// <returns>A pointer to the background color bitmap.</returns>
		BITMAP * GetBGColorBitmap() { return m_BGColorLayer->GetBitmap(); }

		/// <summary>
		/// Gets the material bitmap of this SLTerrain.
		/// </summary>
		/// <returns>A pointer to the material bitmap.</returns>
		BITMAP * GetMaterialBitmap() { return m_MainBitmap; }

		/// <summary>
		/// Gets the structural bitmap of this Terrain.
		/// </summary>
		/// <returns>A pointer to the material bitmap. Ownership is NOT transferred!</returns>
		BITMAP * GetStructuralBitmap() { return m_StructuralBitmap; }

		/// <summary>
		/// Gets a specific pixel from the foreground color bitmap of this. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to get.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to get.</param>
		/// <returns>An int specifying the requested pixel's foreground Color.</returns>
		int GetFGColorPixel(const int pixelX, const int pixelY) const { return GetPixelFromLayer(LayerType::ForegroundLayer, pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the foreground color bitmap of this SLTerrain to a specific color. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to set.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to set.</param>
		/// <param name="color">The color index to set the pixel to.</param>
		void SetFGColorPixel(const int pixelX, const int pixelY, const int color) const { SetPixelOnLayer(LayerType::ForegroundLayer, pixelX, pixelY, color); }

		/// <summary>
		/// Gets a specific pixel from the background color bitmap of this. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to get.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to get.</param>
		/// <returns>An int specifying the requested pixel's background color.</returns>
		int GetBGColorPixel(const int pixelX, const int pixelY) const { return GetPixelFromLayer(LayerType::BackgroundLayer, pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the background color bitmap of this SLTerrain to a specific color. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to set.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to set.</param>
		/// <param name="color">The color index to set the pixel to.</param>
		void SetBGColorPixel(const int pixelX, const int pixelY, const int color) const { SetPixelOnLayer(LayerType::BackgroundLayer, pixelX, pixelY, color); }

		/// <summary>
		/// Gets a specific pixel from the material bitmap of this SceneLayer. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to get.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to get.</param>
		/// <returns>An int specifying the requested pixel's material index.</returns>
		int GetMaterialPixel(const int pixelX, const int pixelY) const { return GetPixelFromLayer(LayerType::MaterialLayer, pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the material bitmap of this SLTerrain to a specific material. LockMaterialBitmap() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to set.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to set.</param>
		/// <param name="materialID">The material index to set the pixel to.</param>
		void SetMaterialPixel(const int pixelX, const int pixelY, const int materialID) const { SetPixelOnLayer(LayerType::MaterialLayer, pixelX, pixelY, materialID); }

		/// <summary>
		/// Gets whether drawing the material layer instead of the normal color layer when drawing this SLTerrain.
		/// </summary>
		/// <returns></returns>
		bool GetToDrawMaterial() const { return m_DrawMaterial; }

		/// <summary>
		/// Sets whether to draw the material layer instead of the normal color layer when drawing this SLTerrain.
		/// </summary>
		/// <param name="drawMaterial">The setting, whether to draw the material later instead of the color layer or not.</param>
		void SetToDrawMaterial(bool drawMaterial) { m_DrawMaterial = drawMaterial; }

		/// <summary>
		/// Indicates whether a terrain pixel is of air or cavity material.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to check for airness.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to check for airness.</param>
		/// <returns>A bool with the answer.</returns>
		bool IsAirPixel(const int pixelX, const int pixelY) const;

		/// <summary>
		/// Checks whether a bounding box is completely buried in the terrain.
		/// </summary>
		/// <param name="checkBox">The box to check.</param>
		/// <returns>Whether the box is completely buried., ie no corner sticks out in the air.</returns>
		bool IsBoxBuried(const Box &checkBox) const;
#pragma endregion

#pragma region
		/// <summary>
		/// Gets a list of unwrapped boxes which show the areas where the material layer has had objects applied to it since last call to ClearUpdatedAreas().
		/// </summary>
		/// <returns>Reference to the list that has been filled with Box:es which are unwrapped and may be out of bounds of the scene!</returns>
		std::list<Box> & GetUpdatedMaterialAreas() { return m_UpdatedMateralAreas; }

		/// <summary>
		/// Adds a notification that an area of the material terrain has been updated.
		/// </summary>
		/// <param name="newArea">The Box defining the newly updated material area that can be unwrapped and may be out of bounds of the scene.</param>
		void AddUpdatedMaterialArea(const Box &newArea) { m_UpdatedMateralAreas.push_back(newArea); }

		/// <summary>
		/// Takes a BITMAP and scans through the pixels on this terrain for pixels which overlap with it. Erases them from the terrain and can optionally generate MOPixel:s based on the erased or 'dislodged' terrain pixels.
		/// </summary>
		/// <param name="pSprite">A pointer to the source BITMAP whose rotozoomed silhouette will be used as a cookie-cutter on the terrain.</param>
		/// <param name="pos">The position coordinates of the sprite.</param>
		/// <param name="pivot"></param>
		/// <param name="rotation">The sprite's current rotation in radians.</param>
		/// <param name="scale">The sprite's current scale coefficient</param>
		/// <param name="makeMOPs">Whether to generate any MOPixel:s from the erased terrain pixels.</param>
		/// <param name="skipMOP">How many pixels to skip making MOPixels from, between each that gets made. 0 means every pixel turns into an MOPixel.</param>
		/// <param name="maxMOPs">The max number of MOPixels to make, if they are to be made.</param>
		/// <returns>A deque filled with the MOPixels of the terrain that are now dislodged. This will be empty if makeMOPs is false. Note that ownership of all the MOPixels in the deque IS transferred! </returns>
		std::deque<MOPixel *> EraseSilhouette(BITMAP *pSprite, Vector pos, Vector pivot, Matrix rotation, float scale, bool makeMOPs = true, int skipMOP = 2, int maxMOPs = 150);

		/// <summary>
		/// Draws a passed in Object's graphical and material representations to this Terrain's respective layers.
		/// </summary>
		/// <param name="entity">The Object to apply to this Terrain. Ownership is NOT transferred!</param>
		/// <returns>Whether successful or not.</returns>
		bool ApplyObject(Entity *entity);

		/// <summary>
		/// Draws a passed in MovableObject's graphical and material representations to this Terrain's respective layers.
		/// </summary>
		/// <param name="movableObject">The MovableObject to apply to this Terrain. Ownership is NOT transferred!</param>
		void ApplyMovableObject(MovableObject *movableObject);

		/// <summary>
		/// Draws a passed in TerrainObject's graphical and material representations to this Terrain's respective layers.
		/// </summary>
		/// <param name="terrainObject">The TerrainObject to apply to this Terrain. Ownership is NOT transferred!</param>
		void ApplyTerrainObject(TerrainObject *terrainObject);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="terrainObject"></param>
		void RegisterTerrainChange(const TerrainObject *terrainObject) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Removes any FG and material pixels completely form the terrain. For the editor mostly.
		/// </summary>
		void ClearAllMaterial();

		/// <summary>
		/// Removes any color pixel in the color layer of this SLTerrain wherever there is an air material pixel in the material layer.
		/// </summary>
		void CleanAir();

		/// <summary>
		/// Removes any color pixel in the color layer of this SLTerrain wherever there is an air material pixel in the material layer inside the specified box.
		/// </summary>
		/// <param name="box">Box to clean.</param>
		/// <param name="wrapsX">Whether the scene is X-wrapped.</param>
		/// <param name="wrapsY">Whether the scene is Y-wrapped.</param>
		void CleanAirBox(const Box &box, bool wrapsX, bool wrapsY);

		/// <summary>
		/// Clears the list of updated areas in the material layer.
		/// </summary>
		void ClearUpdatedAreas() { m_UpdatedMateralAreas.clear(); }

		/// <summary>
		/// Draws this SLTerrain's background layer's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="scrollOverride">If a non-{-1,-1} vector is passed, the internal scroll offset of this is overridden with it. It becomes the new source coordinates.</param>
		void DrawBackground(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1)) const;
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the state of this SLTerrain. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this SLTerrain's foreground's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="scrollOverride">If a non-{-1,-1} vector is passed, the internal scroll offset of this is overridden with it. It becomes the new source coordinates.</param>
		void Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1)) const override;
#pragma endregion

	protected:

		enum class LayerType { ForegroundLayer, BackgroundLayer, MaterialLayer };

		static std::unordered_map<int, BITMAP *> m_TempBitmaps; //!< Intermediate test layers, different sizes for efficiency.

		static Entity::ClassInfo m_sClass;

		std::unique_ptr<SceneLayer> m_FGColorLayer;
		std::unique_ptr<SceneLayer> m_BGColorLayer;
		BITMAP *m_StructuralBitmap;
		ContentFile m_BGTextureFile;

		std::vector<TerrainFrosting *> m_TerrainFrostings;
		std::vector<TerrainDebris *> m_TerrainDebris;
		std::vector<TerrainObject *> m_TerrainObjects;

		std::list<Box> m_UpdatedMateralAreas; //!< List of areas of the material layer which have been affected by the updating of new objects copied to it. These boxes are NOT wrapped, and can be out of bounds!

		bool m_DrawMaterial; //!< Draw the material layer instead of the color layer.

		bool m_NeedToClearFrostings; //!< Indicates, that before processing frostings-related properties for this terrain derived list with frostings must be cleared to avoid duplication when loading scenes.
		bool m_NeedToClearDebris; //!< Indicates, that before processing debris-related properties for this terrain derived list with debris must be cleared to avoid duplication when loading scenes

	private:

		/// <summary>
		/// Load and texturize the FG color bitmap, based on the materials defined in the recently loaded (main) material layer!
		/// </summary>
		void TexturizeTerrain();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="layerType"></param>
		/// <param name="pixelX"></param>
		/// <param name="pixelY"></param>
		/// <returns></returns>
		int GetPixelFromLayer(LayerType layerType, int pixelX, int pixelY) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="layerType"></param>
		/// <param name="pixelX"></param>
		/// <param name="pixelY"></param>
		/// <param name="color"></param>
		void SetPixelOnLayer(LayerType layerType, int pixelX, int pixelY, int color) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="diameter"></param>
		/// <returns></returns>
		BITMAP * GetTempBitmap(int diameter) const;

		/// <summary>
		/// Clears all the member variables of this SLTerrain, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SLTerrain(const SLTerrain &reference) = delete;
		SLTerrain & operator=(const SLTerrain &rhs) = delete;
	};
}
#endif