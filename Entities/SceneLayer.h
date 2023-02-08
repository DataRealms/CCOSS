#ifndef _RTESCENELAYER_
#define _RTESCENELAYER_

#include "Entity.h"
#include "ContentFile.h"
#include "Box.h"

namespace RTE {

	/// <summary>
	/// A scrolling layer of the Scene.
	/// </summary>
	template <bool TRACK_DRAWINGS>
	class SceneLayerImpl : public Entity {
		friend class NetworkServer;

	public:

		EntityAllocation(SceneLayerImpl);
		SerializableOverrideMethods;

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SceneLayerImpl object in system memory. Create() should be called before using the object.
		/// </summary>
		SceneLayerImpl() { Clear(); }

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return Entity::Create(); }

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <param name="bitmapFile">The ContentFile to load as this SceneLayer's graphical representation.</param>
		/// <param name="drawMasked">Whether to draw masked (transparent) or not.</param>
		/// <param name="offset">The initial scroll offset.</param>
		/// <param name="wrapX">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.</param>
		/// <param name="wrapY">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.</param>
		/// <param name="scrollInfo">
		/// A vector whose components define two different things, depending on wrap arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// </param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const ContentFile &bitmapFile, bool drawMasked, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo);

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <param name="bitmap">The BITMAP to use for this SceneLayer. Ownership IS transferred!</param>
		/// <param name="drawMasked">Whether to draw masked (transparent) or not.</param>
		/// <param name="offset">The initial scroll offset.</param>
		/// <param name="wrapX">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.</param>
		/// <param name="wrapY">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.</param>
		/// <param name="scrollInfo">
		/// A vector whose components define two different things, depending on wrap arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// </param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(BITMAP *bitmap, bool drawMasked, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo);

		/// <summary>
		/// Creates a SceneLayer to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SceneLayer to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SceneLayerImpl &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SceneLayer object before deletion from system memory.
		/// </summary>
		~SceneLayerImpl() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;
#pragma endregion

#pragma region Data Handling
		/// <summary>
		/// Whether this SceneLayer's bitmap data is loaded from a file or was generated at runtime.
		/// </summary>
		/// <returns>Whether this SceneLayer's bitmap data was loaded from a file or not.</returns>
		virtual bool IsLoadedFromDisk() const { return !m_BitmapFile.GetDataPath().empty(); }

		/// <summary>
		/// Loads previously specified/created data into memory. Has to be done before using this SceneLayer if the bitmap was not generated at runtime.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int LoadData();

		/// <summary>
		/// Saves data currently in memory to disk.
		/// </summary>
		/// <param name="bitmapPath">The filepath to the where to save the bitmap data.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int SaveData(const std::string &bitmapPath);

		/// <summary>
		/// Clears out any previously loaded bitmap data from memory.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int ClearData();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the scroll offset of this SceneLayer.
		/// </summary>
		/// <returns>A Vector with the scroll offset.</returns>
		Vector GetOffset() const { return m_Offset; }

		/// <summary>
		/// Sets the scroll offset of this SceneLayer. Observe that this offset will be modified by the scroll ratio before applied.
		/// </summary>
		/// <param name="newOffset">The new offset Vector.</param>
		void SetOffset(const Vector &newOffset) { m_Offset = newOffset; }

		/// <summary>
		/// Gets the scroll ratio that modifies the offset.
		/// </summary>
		/// <returns>A copy of the ratio.</returns>
		Vector GetScrollRatio() const { return m_ScrollRatio; }

		/// <summary>
		/// Sets the scroll ratio of this SceneLayer. This modifies the offset before any actual scrolling occurs.
		/// </summary>
		/// <param name="newRatio">The new scroll ratio vector.</param>
		void SetScrollRatio(const Vector &newRatio) { m_ScrollRatio = newRatio; }

		/// <summary>
		/// Gets the scale factor that this is drawn in.
		/// </summary>
		/// <returns>The scale factor of this to the target it is drawn to. (2x if this is half the res, etc.)</returns>
		Vector GetScaleFactor() const { return m_ScaleFactor; }

		/// <summary>
		/// Sets the scale that this should be drawn at when using DrawScaled.
		/// </summary>
		/// <param name="newScale">The new scale factor vector.</param>
		void SetScaleFactor(const Vector &newScale);

		/// <summary>
		/// Indicates whether the layer is set to wrap around the X axis when scrolled out of bounds.
		/// </summary>
		/// <returns>Whether this SceneLayer wraps on the X axis or not.</returns>
		bool WrapsX() const { return m_WrapX; }

		/// <summary>
		/// Indicates whether the layer is set to wrap around the Y axis when scrolled out of bounds.
		/// </summary>
		/// <returns>Whether this SceneLayer wraps on the Y axis or not.</returns>
		bool WrapsY() const { return m_WrapY; }

		/// <summary>
		/// Gets a specific pixel from the bitmap of this SceneLayer. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to get.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to get.</param>
		/// <returns>An int specifying the requested pixel's color index.</returns>
		int GetPixel(int pixelX, int pixelY) const;

		/// <summary>
		/// Sets a specific pixel on the bitmap of this SceneLayer to a specific value. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinate of the pixel to set.</param>
		/// <param name="pixelY">The Y coordinate of the pixel to set.</param>
		/// <param name="materialID">The color index to set the pixel to.</param>
		void SetPixel(int pixelX, int pixelY, int materialID);

		/// <summary>
		/// Returns whether the integer coordinates passed in are within the bounds of this SceneLayer.
		/// </summary>
		/// <param name="pixelX">The X coordinates of the pixel.</param>
		/// <param name="pixelY">The Y coordinates of the pixel.</param>
		/// <param name="margin"></param>
		/// <returns>Whether within bounds or not.</returns>
		bool IsWithinBounds(int pixelX, int pixelY, int margin = 0) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Lock the internal bitmap so it can be accessed by GetPixel() etc. UnlockBitmaps() should always be called after accesses are completed.
		/// Doing it in a separate method like this is more efficient because many bitmap accesses can be performed between a lock and unlock.
		/// </summary>
		void LockBitmaps() { /*acquire_bitmap(m_MainBitmap);*/ }

		/// <summary>
		/// Unlocks the internal bitmaps and prevents access to display memory. UnlockBitmaps() should only be called after LockBitmaps().
		/// Doing it in a separate method like this is more efficient because many bitmap accesses can be performed between a lock and an unlock.
		/// </summary>
		void UnlockBitmaps() { /*release_bitmap(m_MainBitmap);*/ }

		/// <summary>
		/// Clears our BITMAP.
		/// </summary>
		/// <param name="clearTo">What color to clear the bitmap to.</param>
		void ClearBitmap(ColorKeys clearTo);

		/// <summary>
		/// Wraps the given position coordinate if it is out of bounds of this SceneLayer and wrapping is enabled on the appropriate axes.
		/// Does not force the position coordinate within bounds if wrapping is not enabled.
		/// </summary>
		/// <param name="posX">The X coordinates of the position to wrap.</param>
		/// <param name="posY">The Y coordinates of the position to wrap.</param>
		/// <returns>Whether wrapping was performed or not.</returns>
		bool WrapPosition(int &posX, int &posY) const;

		/// <summary>
		/// Wraps the given position Vector if it is out of bounds of this SceneLayer and wrapping is enabled on the appropriate axes.
		/// Does not force the position Vector within bounds if wrapping is not enabled.
		/// </summary>
		/// <param name="pos">The vector coordinates of the position to wrap.</param>
		/// <returns>Whether wrapping was performed or not.</returns>
		bool WrapPosition(Vector &pos) const { return ForceBoundsOrWrapPosition(pos, false); }

		/// <summary>
		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// </summary>
		/// <param name="posX">The X coordinates of the position to wrap.</param>
		/// <param name="posY">The Y coordinates of the position to wrap.</param>
		/// <returns>Whether wrapping was performed or not. Does not report on bounding.</returns>
		bool ForceBounds(int &posX, int &posY) const;

		/// <summary>
		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// </summary>
		/// <param name="pos">The Vector coordinates of the position to wrap.</param>
		/// <returns>Whether wrapping was performed or not. Does not report on bounding.</returns>
		bool ForceBounds(Vector &pos) const { return ForceBoundsOrWrapPosition(pos, true); }
#pragma endregion

#pragma region Drawing Tracking
		/// <summary>
		/// Registers an area of the SceneLayer to be drawn upon. These areas will be cleared when ClearBitmap is called.
		/// </summary>
		/// <param name="left">The position of the left side of the area to be drawn upon.</param>
		/// <param name="top">The position of the top of the area to be drawn upon.</param>
		/// <param name="right">The position of the right side of the area to be drawn upon.</param>
		/// <param name="bottom"The position of the bottom of the area to be drawn upon.></param>
		void RegisterDrawing(int left, int top, int right, int bottom);

		/// <summary>
		/// Registers an area of the SceneLayer to be drawn upon. These areas will be cleared when ClearBitmap is called.
		/// </summary>
		/// <param name="center">The position of the center of the area to be drawn upon.</param>
		/// <param name="radius">The radius of the area to be drawn upon.</param>
		void RegisterDrawing(const Vector &center, float radius);
#pragma endregion

#pragma region Virtual Methods
		/// <summary>
		/// Updates the state of this SceneLayer.
		/// </summary>
		virtual void Update() {}

		/// <summary>
		/// Draws this SceneLayer's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="offsetNeedsScrollRatioAdjustment">Whether the offset of this SceneLayer or the passed in offset override need to be adjusted to scroll ratio.</param>
		virtual void Draw(BITMAP *targetBitmap, Box &targetBox, bool offsetNeedsScrollRatioAdjustment = false);
#pragma endregion

	protected:

		ContentFile m_BitmapFile; //!< ContentFile containing the path to this SceneLayer's sprite file.

		BITMAP *m_MainBitmap; //!< The main BITMAP of this SceneLayer.
		BITMAP *m_BackBitmap; //!< The backbuffer BITMAP of this SceneLayer.

		// We use two bitmaps, as a backbuffer. While the main bitmap is being used, the secondary bitmap will be cleared on a separate thread. This is because we tend to want to clear some scene layers every frame and that is costly.
		std::mutex m_BitmapClearMutex; //!< Mutex for clearing BITMAP in background.
		ColorKeys m_LastClearColor; //!< The last color we cleared this SceneLayer to.
		std::vector<IntRect> m_Drawings; //!< All the areas drawn within on this SceneLayer since the last clear.

		bool m_MainBitmapOwned; //!< Whether the main bitmap is owned by this.
		bool m_DrawMasked; //!< Whether pixels marked as transparent (index 0, magenta) are skipped when drawing or not (masked drawing).

		bool m_WrapX; //!< Whether wrapping is enabled on the X axis.
		bool m_WrapY; //!< Whether wrapping is enable on the Y axis.

		Vector m_OriginOffset; //!< Offset of this SceneLayer off the top left edge of the screen.
		Vector m_Offset; //!< The current scrolled offset of this SceneLayer, before being adjusted with the origin offset.

		Vector m_ScrollInfo; //!< The initial scrolling ratio of this SceneLayer as set in INI. Used to calculate the actual scrolling ratios.
		Vector m_ScrollRatio; //!< The scrolling ratios of this SceneLayer, adjusted to the Scene, player screen dimensions and scaling factor as necessary.
		Vector m_ScaleFactor; //!< The scaling factor of this SceneLayer. Used for scaled drawing and adjusting scrolling ratios.
		Vector m_ScaledDimensions; //!< The dimensions of this SceneLayer adjusted to the scaling factor.

		/// <summary>
		/// Initialize the scroll ratios from the scroll info. Must be done after the bitmap has been created.
		/// </summary>
		/// <param name="initForNetworkPlayer"></param>
		/// <param name="player"></param>
		void InitScrollRatios(bool initForNetworkPlayer = false, int player = Players::NoPlayer);

		/// <summary>
		/// Wraps or bounds a position coordinate if it is out of bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// </summary>
		/// <param name="pos">The Vector coordinates of the position to wrap.</param>
		/// <param name="forceBounds">Whether to attempt bounding or wrapping, or just wrapping.</param>
		/// <returns>Whether wrapping was performed or not. Does not report on bounding.</returns>
		bool ForceBoundsOrWrapPosition(Vector &pos, bool forceBounds) const;

#pragma region Draw Breakdown
		/// <summary>
		/// Performs wrapped drawing of this SceneLayer's bitmap to the screen in cases where it is both wider and taller than the target bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="drawScaled">Whether to use scaled drawing routines or not.</param>
		void DrawWrapped(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const;

		/// <summary>
		/// Performs tiled drawing of this SceneLayer's bitmap to the screen in cases where the target bitmap is larger in some dimension.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="drawScaled">Whether to use scaled drawing routines or not.</param>
		void DrawTiled(BITMAP *targetBitmap, const Box &targetBox, bool drawScaled) const;
#pragma endregion

	private:

		/// <summary>
		/// Clears any tracked and drawn-to areas.
		/// </summary>
		/// <param name="clearTo">Color to clear to.</param>
		void ClearDrawings(BITMAP *bitmap, const std::vector<IntRect> &drawings, ColorKeys clearTo) const;

		/// <summary>
		/// Clears all the member variables of this SceneLayer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SceneLayerImpl(const SceneLayerImpl &reference) = delete;
		void operator=(const SceneLayerImpl &rhs) = delete;
	};

	/// <summary>
	/// If we track drawings, then disallow getting non-const access to the underlying bitmap - we must draw through special functions on SceneLayer that'll track the drawings.
	/// </summary>
	class SceneLayerTracked : public SceneLayerImpl<true> {

	public:

		EntityAllocation(SceneLayerTracked);
		ClassInfoGetters;

		/// <summary>
		/// Constructor method used to instantiate a SceneLayerTracked object in system memory. Create() should be called before using the object.
		/// </summary>
		SceneLayerTracked() : SceneLayerImpl<true>() {}

		// TODO: We shouldn't let external users access a non-const version of our bitmap. We should do all drawing to it internally, and track registering our MOID drawings internally too.
		// However, in the interest of time (and my own sanity), given that the old code already does this, we're not doing that yet.
		/// <summary>
		/// Gets the BITMAP that this SceneLayer uses.
		/// </summary>
		/// <returns>A pointer to the BITMAP of this SceneLayer. Ownership is NOT transferred!</returns>
		BITMAP * GetBitmap() const { return m_MainBitmap; }

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
	};

	class SceneLayer : public SceneLayerImpl<false> {

	public:

		EntityAllocation(SceneLayer);
		ClassInfoGetters;

		/// <summary>
		/// Constructor method used to instantiate a SceneLayer object in system memory. Create() should be called before using the object.
		/// </summary>
		SceneLayer() : SceneLayerImpl<false>() {}

		/// <summary>
		/// Gets the BITMAP that this SceneLayer uses.
		/// </summary>
		/// <returns>A pointer to the BITMAP of this SceneLayer. Ownership is NOT transferred!</returns>
		BITMAP * GetBitmap() const { return m_MainBitmap; }

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
	};
}
#endif