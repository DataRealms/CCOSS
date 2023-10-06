#ifndef _RTESLTERRAIN_
#define _RTESLTERRAIN_

#include "SceneLayer.h"
#include "Matrix.h"

namespace RTE {

	class MOPixel;
	class TerrainFrosting;
	class TerrainObject;
	class TerrainDebris;

	/// <summary>
	/// Collection of scrolling layers that compose the terrain of the Scene.
	/// </summary>
	class SLTerrain : public SceneLayer {

	public:

		EntityAllocation(SLTerrain);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// <summary>
		/// Enumeration for the different type of layers in the SLTerrain.
		/// </summary>
		enum class LayerType { ForegroundLayer, BackgroundLayer, MaterialLayer };

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
		void Destroy(bool notInherited = false) override { if (!notInherited) { SceneLayer::Destroy(); } Clear(); }
#pragma endregion

#pragma region Data Handling
		/// <summary>
		/// Whether this SLTerrain's bitmap data is loaded from a file or was generated at runtime.
		/// </summary>
		/// <returns>Whether this SLTerrain's bitmap data was loaded from a file or was generated at runtime.</returns>
		bool IsLoadedFromDisk() const override { return (m_FGColorLayer && m_FGColorLayer->IsLoadedFromDisk()) && (m_BGColorLayer && m_BGColorLayer->IsLoadedFromDisk()); }

		/// <summary>
		/// Loads previously specified/created bitmap data into memory. Has to be done before using this SLTerrain if the bitmap was not generated at runtime.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int LoadData() override;

		/// <summary>
		/// Saves bitmap data currently in memory to disk.
		/// </summary>
		/// <param name="pathBase">The filepath base to the where to save the Bitmap data. This means everything up to the extension. "FG" and "Mat" etc will be added.</param>
		/// <param name="doAsyncSaves">Whether or not to save asynchronously.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int SaveData(const std::string &pathBase, bool doAsyncSaves = true) override;

		/// <summary>
		/// Clears out any previously loaded bitmap data from memory.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int ClearData() override;
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the width of this SLTerrain as determined by the main (material) bitmap.
		/// </summary>
		/// <returns>The width of this SLTerrain, in pixels.</returns>
		int GetWidth() const { return m_Width; }

		/// <summary>
		/// Gets the height of this SLTerrain as determined by the main (material) bitmap.
		/// </summary>
		/// <returns>The height of this SLTerrain, in pixels.</returns>
		int GetHeight() const { return m_Height; }

		/// <summary>
		/// Sets the layer of this SLTerrain that should be drawn to the screen when Draw() is called.
		/// </summary>
		/// <param name="layerToDraw">The layer that should be drawn. See LayerType enumeration.</param>
		void SetLayerToDraw(LayerType layerToDraw) { m_LayerToDraw = layerToDraw; }

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
		/// Gets a specific pixel from the foreground color bitmap of this. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to get.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to get.</param>
		/// <returns>An int specifying the requested pixel's foreground color index.</returns>
		int GetFGColorPixel(int pixelX, int pixelY) const { return m_FGColorLayer->GetPixel(pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the foreground color bitmap of this SLTerrain to a specific color. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to set.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to set.</param>
		/// <param name="materialID">The color index to set the pixel to.</param>
		void SetFGColorPixel(int pixelX, int pixelY, const int materialID) const { m_FGColorLayer->SetPixel(pixelX, pixelY, materialID); }

		/// <summary>
		/// Gets a specific pixel from the background color bitmap of this. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to get.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to get.</param>
		/// <returns>An int specifying the requested pixel's background color index.</returns>
		int GetBGColorPixel(int pixelX, int pixelY) const { return m_BGColorLayer->GetPixel(pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the background color bitmap of this SLTerrain to a specific color. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to set.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to set.</param>
		/// <param name="materialID">The color index to set the pixel to.</param>
		void SetBGColorPixel(int pixelX, int pixelY, int materialID) const { m_BGColorLayer->SetPixel(pixelX, pixelY, materialID); }

		/// <summary>
		/// Gets a specific pixel from the material bitmap of this SceneLayer. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to get.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to get.</param>
		/// <returns>An int specifying the requested pixel's material index.</returns>
		int GetMaterialPixel(int pixelX, int pixelY) const { return GetPixel(pixelX, pixelY); }

		/// <summary>
		/// Sets a specific pixel on the material bitmap of this SLTerrain to a specific material. LockMaterialBitmap() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to set.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to set.</param>
		/// <param name="materialID">The material index to set the pixel to.</param>
		void SetMaterialPixel(int pixelX, int pixelY, int materialID) { SetPixel(pixelX, pixelY, materialID); }

		/// <summary>
		/// Indicates whether a terrain pixel is of Air or Cavity material.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to check.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to check.</param>
		/// <returns>Whether the terrain pixel is of Air or Cavity material.</returns>
		bool IsAirPixel(int pixelX, int pixelY) const;

		/// <summary>
		/// Checks whether a bounding box is completely buried in the terrain.
		/// </summary>
		/// <param name="checkBox">The box to check.</param>
		/// <returns>Whether the box is completely buried, i.e. no corner sticks out in the Air or Cavity.</returns>
		bool IsBoxBuried(const Box &checkBox) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Gets a deque of unwrapped boxes which show the areas where the material layer has had objects applied to it since last call to ClearUpdatedMaterialAreas().
		/// </summary>
		/// <returns>Reference to the deque that has been filled with Boxes which are unwrapped and may be out of bounds of the scene!</returns>
		std::deque<Box> & GetUpdatedMaterialAreas() { return m_UpdatedMaterialAreas; }

		/// <summary>
		/// Adds a notification that an area of the material terrain has been updated.
		/// </summary>
		/// <param name="newArea">The Box defining the newly updated material area that can be unwrapped and may be out of bounds of the scene.</param>
		void AddUpdatedMaterialArea(const Box &newArea) { m_UpdatedMaterialAreas.emplace_back(newArea); }

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
		/// Takes a BITMAP and scans through the pixels on this terrain for pixels which overlap with it. Erases them from the terrain and can optionally generate MOPixels based on the erased or 'dislodged' terrain pixels.
		/// </summary>
		/// <param name="sprite">A pointer to the source BITMAP whose silhouette will be used as a cookie-cutter on the terrain.</param>
		/// <param name="pos">The position coordinates of the sprite.</param>
		/// <param name="pivot">The pivot coordinate of the sprite.</param>
		/// <param name="rotation">The sprite's current rotation in radians.</param>
		/// <param name="scale">The sprite's current scale coefficient.</param>
		/// <param name="makeMOPs">Whether to generate any MOPixels from the erased terrain pixels.</param>
		/// <param name="skipMOP">How many pixels to skip making MOPixels from, between each that gets made. 0 means every pixel turns into an MOPixel.</param>
		/// <param name="maxMOPs">The max number of MOPixels to make, if they are to be made.</param>
		/// <returns>A deque filled with the MOPixels of the terrain that are now dislodged. This will be empty if makeMOPs is false. Note that ownership of all the MOPixels in the deque IS transferred!</returns>
		std::deque<MOPixel *> EraseSilhouette(BITMAP *sprite, const Vector &pos, const Vector &pivot, const Matrix &rotation, float scale, bool makeMOPs = true, int skipMOP = 2, int maxMOPs = 150);

		/// <summary>
		/// Returns the direction of the out-of-bounds "orbit" for this scene, where the brain must path to and where dropships/rockets come from.
		/// </summary>
		/// <returns>The orbit direction, either Up, Down, Left or Right..</returns>
		Directions GetOrbitDirection() { return m_OrbitDirection; }
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the state of this SLTerrain.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this SLTerrain's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="offsetNeedsScrollRatioAdjustment">Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.</param>
		void Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment = false) override;
#pragma endregion

	private:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		int m_Width; //!< The width of this SLTerrain as determined by the main (material) bitmap, in pixels.
		int m_Height; //!< The height of this SLTerrain as determined by the main (material) bitmap, in pixels.

		std::unique_ptr<SceneLayer> m_FGColorLayer; //!< The foreground color layer of this SLTerrain.
		std::unique_ptr<SceneLayer> m_BGColorLayer; //!< The background color layer of this SLTerrain.

		LayerType m_LayerToDraw; //!< The layer of this SLTerrain that should be drawn to the screen when Draw() is called. See LayerType enumeration.

		ContentFile m_DefaultBGTextureFile; //!< The background texture file that will be used to texturize Materials that have no defined background texture.

		std::vector<TerrainFrosting *> m_TerrainFrostings; //!< The TerrainFrostings that need to be placed on this SLTerrain.
		std::vector<TerrainDebris *> m_TerrainDebris; //!< The TerrainDebris that need to be  placed on this SLTerrain.
		std::vector<TerrainObject *> m_TerrainObjects; //!< The TerrainObjects that need to be placed on this SLTerrain.

		std::deque<Box> m_UpdatedMaterialAreas; //!< List of areas of the material layer (main bitmap) which have been affected by new objects copied to it. These boxes are NOT wrapped, and can be out of bounds!

		Directions m_OrbitDirection; //!< The direction of the out-of-bounds "orbit" for this scene, where the brain must path to and where dropships/rockets come from.

		/// <summary>
		/// Applies Material textures to the foreground and background color layers, based on the loaded material layer (main bitmap).
		/// </summary>
		void TexturizeTerrain();

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