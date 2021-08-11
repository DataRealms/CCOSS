#ifndef _RTESCENELAYER_
#define _RTESCENELAYER_

#include "Entity.h"
#include "Box.h"
#include "ContentFile.h"

namespace RTE {

	/// <summary>
	/// A scrolling layer of the Scene.
	/// </summary>
	class SceneLayer : public Entity {
		friend class NetworkServer;

	public:

		EntityAllocation(SceneLayer)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SceneLayer object in system memory. Create() should be called before using the object.
		/// </summary>
		SceneLayer() { Clear(); }

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return (Entity::Create() < 0) ? -1 : 0; }

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <param name="bitmapFile">The ContentFile representing the bitmap file to load as this SceneLayer's graphical representation.</param>
		/// <param name="drawTrans">Whether to draw transparently using a color key specified by the pixel in the upper left corner of the loaded bitmap.</param>
		/// <param name="offset">The initial scroll offset.</param>
		/// <param name="wrapX">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.</param>
		/// <param name="wrapY">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.</param>
		/// <param name="scrollInfo">
		/// A vector whose components define two different things, depending on wrapX/Y arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// </param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const ContentFile &bitmapFile, bool drawTrans, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo);

		/// <summary>
		/// Makes the SceneLayer object ready for use.
		/// </summary>
		/// <param name="bitmap">The prepared BITMAP to use as for this SceneLayer. Ownership IS transferred!</param>
		/// <param name="drawTrans">Whether to draw transparently using a color key specified by the pixel in the upper left corner of the loaded bitmap.</param>
		/// <param name="offset">The initial scroll offset.</param>
		/// <param name="wrapX">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the X axis.</param>
		/// <param name="wrapY">Whether the layer should wrap around or stop when scrolling beyond its bitmap's boundaries on the Y axis.</param>
		/// <param name="scrollInfo">
		/// A vector whose components define two different things, depending on wrapX/Y arguments.
		/// If a wrap argument is set to false, the corresponding component here will be interpreted as the width (X) or height (Y) (in pixels) of the total bitmap area that this layer is allowed to scroll across before stopping at an edge.
		/// If wrapping is set to true, the value in scrollInfo is simply the ratio of offset at which any scroll operations will be done in.
		/// A special command is if wrap is false and the corresponding component is -1.0, that signals that the own width or height should be used as scrollInfo input.
		/// </param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(BITMAP *bitmap, bool drawTrans, const Vector &offset, bool wrapX, bool wrapY, const Vector &scrollInfo);

		/// <summary>
		/// Creates a SceneLayer to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the SceneLayer to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const SceneLayer &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a SceneLayer object before deletion from system memory.
		/// </summary>
		~SceneLayer() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;
#pragma endregion

#pragma region 
		/// <summary>
		/// Whether this' bitmap data is loaded from a file or was generated.
		/// </summary>
		/// <returns>Whether the data in this' bitmap was loaded from a datafile, or generated.</returns>
		virtual bool IsFileData() const { return !m_BitmapFile.GetDataPath().empty(); }

		/// <summary>
		/// Actually loads previously specified/created data into memory. Has to be done before using this SceneLayer.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int LoadData();

		/// <summary>
		/// Saves data currently in memory to disk.
		/// </summary>
		/// <param name="bitmapPath">The filepath to the where to save the Bitmap data.</param>
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
		/// Gets the BITMAP object that this SceneLayer uses.
		/// </summary>
		/// <returns>A pointer to the BITMAP object. Ownership is NOT transferred!</returns>
		BITMAP * GetBitmap() const { return m_MainBitmap; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		size_t GetBitmapHash() const { return m_BitmapFile.GetHash(); }

		/// <summary>
		/// Gets the scroll offset.
		/// </summary>
		/// <returns>A copy of the offset.</returns>
		Vector GetOffset() const { return m_Offset; }

		/// <summary>
		/// Sets the offset of this SceneLayer. Observe that this offset will be modified by the scroll ratio before applied.
		/// </summary>
		/// <param name="newOffset">The new offset vector.</param>
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
		/// Gets the inverse scale factor that this is drawn in.
		/// </summary>
		/// <returns>The scale factor of this to the target it is drawn to. (1/2x if this is half the res, etc.)</returns>
		Vector GetScaleInverse() const { return m_ScaleInverse; }

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
		/// Gets a specific pixel from the main bitmap of this SceneLayer. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to get.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to get.</param>
		/// <returns>An int specifying the requested pixel's value.</returns>
		int GetPixel(const int pixelX, const int pixelY);

		/// <summary>
		/// Sets a specific pixel on the main bitmap of this SceneLayer to a specific value. LockBitmaps() must be called before using this method.
		/// </summary>
		/// <param name="pixelX">The X coordinates of which pixel to set.</param>
		/// <param name="pixelY">The Y coordinates of which pixel to set.</param>
		/// <param name="value">The value to set the pixel to.</param>
		void SetPixel(const int pixelX, const int pixelY, const int value);

		/// <summary>
		/// Returns whether the integer coordinates passed in are within the bounds of this SceneLayer.
		/// </summary>
		/// <param name="pixelX">The X coordinates of the pixel.</param>
		/// <param name="pixelY">The Y coordinates of the pixel.</param>
		/// <param name="margin"></param>
		/// <returns>Whether within bounds or not.</returns>
		// TODO: This doesn't take Y wrapping into account!$@#$
		bool IsWithinBounds(const int pixelX, const int pixelY, const int margin = 0) const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Lock the internal bitmap so it can be accessed by GetPixel() etc. UnlockBitmaps() should always be called after accesses are completed.
		/// Doing it in a separate method like this is more efficient because many bitmap accesses can be performed between a lock and unlock.
		/// </summary>
		void LockBitmaps() { acquire_bitmap(m_MainBitmap); }

		/// <summary>
		/// Unlocks the internal bitmaps and prevents access to display memory. UnlockBitmaps() should only be called after LockBitmaps().
		/// Doing it in a separate method like this is more efficient because many bitmap accesses can be performed between a lock and an unlock.
		/// </summary>
		void UnlockBitmaps() { release_bitmap(m_MainBitmap); }

		/// <summary>
		/// Only wraps a position coordinate if it is off bounds of the SceneLayer and wrapping in the corresponding axes are turned on.
		/// </summary>
		/// <param name="posX">The X coordinates of the position to wrap, if needed.</param>
		/// <param name="posY">The Y coordinates of the position to wrap, if needed.</param>
		/// <param name="scaled">Whether the coordinates above are of this' scale factor, or in its native pixels.</param>
		/// <returns>Whether wrapping was performed or not.</returns>
		bool WrapPosition(int &posX, int &posY, bool scaled = true) const;

		/// <summary>
		/// Only wraps a position coordinate if it is off bounds of the SceneLayer and wrapping in the corresponding axes are turned on.
		/// </summary>
		/// <param name="pos">The vector coordinates of the position to wrap, if needed.</param>
		/// <param name="scaled">Whether the coordinates above are of this' scale factor, or in its native pixels.</param>
		/// <returns>Whether wrapping was performed or not.</returns>
		bool WrapPosition(Vector &pos, bool scaled = true) const { ForceBoundsOrWrapPosition(pos, scaled, false); }

		/// <summary>
		/// Wraps or bounds a position coordinate if it is off bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// </summary>
		/// <param name="posX">The X coordinates of the position to wrap, if needed.</param>
		/// <param name="posY">The Y coordinates of the position to wrap, if needed.</param>
		/// <param name="scaled">Whether the coordinates above are of this' scale factor, or in its native pixels.</param>
		/// <returns>Whether wrapping was performed or not. Does not report on bounding.</returns>
		bool ForceBounds(int &posX, int &posY, bool scaled = true) const;

		/// <summary>
		/// Wraps or bounds a position coordinate if it is off bounds of the SceneLayer, depending on the wrap settings of this SceneLayer.
		/// </summary>
		/// <param name="pos">The Vector coordinates of the position to wrap, if needed.</param>
		/// <param name="scaled">Whether the coordinates above are of this' scale factor, or in its native pixels.</param>
		/// <returns>Whether wrapping was performed or not. Does not report on bounding.</returns>
		bool ForceBounds(Vector &pos, bool scaled = true) const { ForceBoundsOrWrapPosition(pos, scaled, true); }

		/// <summary>
		/// Draws this SceneLayer's current scrolled position to a bitmap, but also scaled according to what has been set with SetScaleFactor.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="scrollOverride">If a non-{-1,-1} vector is passed, the internal scroll offset of this is overridden with it. It becomes the new source coordinates.</param>
		void DrawScaled(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1));
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates the state of this SceneLayer. Supposed to be done every frame.
		/// </summary>
		virtual void Update() {}

		/// <summary>
		/// Draws this SceneLayer's current scrolled position to a bitmap.
		/// </summary>
		/// <param name="targetBitmap">The bitmap to draw to.</param>
		/// <param name="targetBox">The box on the target bitmap to limit drawing to, with the corner of box being where the scroll position lines up.</param>
		/// <param name="scrollOverride">If a non-{-1,-1} vector is passed, the internal scroll offset of this is overridden with it. It becomes the new source coordinates.</param>
		virtual void Draw(BITMAP *targetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1));
#pragma endregion

	protected:

		// Member variables
		static Entity::ClassInfo m_sClass;

		ContentFile m_BitmapFile;

		BITMAP *m_MainBitmap;
		// Whether main bitmap is owned by this
		bool m_MainBitmapOwned;
		bool m_DrawTrans;
		Vector m_Offset;
		// The original ScrollInfo with special encoded info that is then made into the actual scroll ratios
		Vector m_ScrollInfo;
		Vector m_ScrollRatio;
		Vector m_ScaleFactor;
		Vector m_ScaleInverse;
		Vector m_ScaledDimensions;

		bool m_WrapX;
		bool m_WrapY;
		int m_FillLeftColor;
		int m_FillRightColor;
		int m_FillUpColor;
		int m_FillDownColor;

		/// <summary>
		/// Initialize the scroll ratios from the encoded scroll info. Must be done after the bitmap has been created in the derived concrete classes.
		/// </summary>
		/// <param name="initForNetworkPlayer"></param>
		/// <param name="player"></param>
		void InitScrollRatios(bool initForNetworkPlayer = false, int player = Players::NoPlayer);

	private:

#pragma region Drawing
		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap"></param>
		/// <param name="targetBox"></param>
		/// <param name="offsetX"></param>
		/// <param name="offsetY"></param>
		void DrawWrapped(BITMAP *targetBitmap, const Box &targetBox, int offsetX, int offsetY) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap"></param>
		/// <param name="targetBox"></param>
		/// <param name="offsetX"></param>
		/// <param name="offsetY"></param>
		void DrawWrappedScaled(BITMAP *targetBitmap, const Box &targetBox, int offsetX, int offsetY) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap"></param>
		/// <param name="targetBox"></param>
		/// <param name="scrollOverridden"></param>
		/// <param name="offsetX"></param>
		/// <param name="offsetY"></param>
		void DrawTiled(BITMAP *targetBitmap, const Box &targetBox, bool scrollOverridden, int offsetX, int offsetY) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap"></param>
		/// <param name="targetBox"></param>
		/// <param name="scrollOverridden"></param>
		/// <param name="offsetX"></param>
		/// <param name="offsetY"></param>
		void DrawTiledScaled(BITMAP *targetBitmap, const Box &targetBox, bool scrollOverridden, int offsetX, int offsetY) const;
#pragma endregion

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="scaled"></param>
		/// <param name="forceBounds"></param>
		/// <returns></returns>
		bool ForceBoundsOrWrapPosition(Vector &pos, bool scaled, bool forceBounds) const;

		/// <summary>
		/// Clears all the member variables of this SceneLayer, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		SceneLayer(const SceneLayer &reference) = delete;
		void operator=(const SceneLayer &rhs) = delete;
	};
}
#endif