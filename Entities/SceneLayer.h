#ifndef _RTESCENELAYER_
#define _RTESCENELAYER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneLayer.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the SceneLayer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Box.h"
#include "FrameMan.h"
#include "SceneMan.h"

namespace RTE
{

class ContentFile;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SceneLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A scrolling layer of the scene.
// Parent(s):       Entity.
// Class history:   12/31/2001 SceneLayer created.

class SceneLayer : public Entity {
	friend class NetworkServer;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(SceneLayer);
SerializableOverrideMethods;
ClassInfoGetters;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SceneLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SceneLayer object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SceneLayer() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SceneLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SceneLayer object before deletion
//                  from system memory.
// Arguments:       None.

	~SceneLayer() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.
// Arguments:       The ContentFile representing the bitmap file to load as this
//                  SceneLayer's graphical representation.
//                  Whether to draw transperently using a color key specified by the pixel
//                  in the upper left corner of the loaded bitmap.
//                  The initial scroll offset.
//                  Two bools that define whether the layer should wrap around or stop when
//                  scrolling beyond its bitmap's boundries.
//                  A vector whose components define two different things, depending on
//                  wrapX/Y arguments. If a wrap argument is set to false, the
//                  corresponding component here will be interpreted as the width (X) or
//                  height (Y) (in pixels) of the total bitmap area that this layer is
//                  allowed to scroll across before stopping at an edge. If wrapping is
//                  set to true, the value in scrollInfo is simply the ratio of offset at
//                  which any scroll operations will be done in. A special command is if
//                  wrap is false and the corresponding component is -1.0, that signals
//                  that the own width or height should be used as scrollInfo input.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

// TODO: streamline interface")
	int Create(ContentFile bitmapFile, bool drawTrans, Vector offset, bool wrapX, bool wrapY, Vector scrollInfo);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.
// Arguments:       The prepared BITMAP to use as for this SceneLayer. Ownership IS
//                  transferred!
//                  Whether to draw transperently using a color key specified by the pixel
//                  in the upper left corner of the loaded bitmap.
//                  The initial scroll offset.
//                  Two bools that define whether the layer should wrap around or stop when
//                  scrolling beyond its bitmap's boundries.
//                  A vector whose components define two different things, depending on
//                  wrapX/Y arguments. If a wrap argument is set to false, the
//                  corresponding component here will be interpreted as the width (X) or
//                  height (Y) (in pixels) of the total bitmap area that this layer is
//                  allowed to scroll across before stopping at an edge. If wrapping is
//                  set to true, the value in scrollInfo is simply the ratio of offset at
//                  which any scroll operations will be done in. A special command is if
//                  wrap is false and the corresponding component is -1.0, that signals
//                  that the own width or height should be used as scrollInfo input.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

// TODO: streamline interface")
	int Create(BITMAP *pBitmap, bool drawTrans, Vector offset, bool wrapX, bool wrapY, Vector scrollInfo);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SceneLayer to be identical to another, by deep copy.
// Arguments:       A reference to the SceneLayer to deep copy.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const SceneLayer &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this SceneLayer.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int LoadData();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves data currently in memory to disk.
// Arguments:       The filepath to the where to save the Bitmap data.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int SaveData(std::string bitmapPath);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int ClearData();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFileData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether this' bitmap data is loaded from a file or was generated.
// Arguments:       None.
// Return value:    Whether the data in this' bitmap was loaded from a datafile, or generated.

    virtual bool IsFileData() const { return !m_BitmapFile.GetDataPath().empty();  }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SLTerrain, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the BITMAP object that this SceneLayer uses.
// Arguments:       None.
// Return value:    A pointer to the BITMAP object. Ownership is NOT transferred!

    BITMAP * GetBitmap() const { return m_pMainBitmap; }

	size_t GetBitmapHash() const { return m_BitmapFile.GetHash(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the scroll offset.
// Arguments:       None.
// Return value:    A copy of the offset.

    Vector GetOffset() const { return m_Offset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScrollRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the scroll ratio that modifies the offset.
// Arguments:       None.
// Return value:    A copy of the ratio.

    Vector GetScrollRatio() const { return m_ScrollRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScaleFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the scale factor that this is drawn in.
// Arguments:       None.
// Return value:    The scale factor of this to the target it is drawn to. (2x if this is
//                  half the res, etc.)

    Vector GetScaleFactor() const { return m_ScaleFactor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScaleInverse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the inverse scale factor that this is drawn in.
// Arguments:       None.
// Return value:    The scale factor of this to the target it is drawn to. (1/2x if this is
//                  half the res, etc.)

    Vector GetScaleInverse() const { return m_ScaleInverse; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the layer is set to wrap around the Y axis when
//                  scrolled out of bounds.
// Arguments:       None.
// Return value:    Whether this SceneLayer wraps or not in the Y axis.

    bool WrapsX() const { return m_WrapX; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the layer is set to wrap around the Y axis when
//                  scrolled out of bounds.
// Arguments:       None.
// Return value:    Whether this SceneLayer wraps or not in the Y axis.

    bool WrapsY() const { return m_WrapY; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset of this SceneLayer. Observe that this offset will be
//                  modified by the scroll ratio before applied.
// Arguments:       The new offset vector.
// Return value:    None.

    void SetOffset(const Vector newOffset) { m_Offset = newOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScrollRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the scroll ratio of this SceneLayer. This modifies the offset
//                  before any actual scrolling occurs.
// Arguments:       The new scroll ratio vector.
// Return value:    None.

    void SetScrollRatio(const Vector newRatio) { m_ScrollRatio = newRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScaleFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the scale that this should be drawn at when using DrawScaled.
// Arguments:       The new scale factor vector.
// Return value:    None.

    void SetScaleFactor(const Vector newScale);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LockBitmaps
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Lock the internal bitmaps so it can be accessed by GetColorPixel() etc.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and unlock.
//                  UnlockBitmaps() should always be called after accesses are completed.
// Arguments:       None.
// Return value:    None.

    virtual void LockBitmaps() { acquire_bitmap(m_pMainBitmap); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UnlockBitmaps
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the internal bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockBitmaps() should only be called after LockBitmaps().
// Arguments:       None.
// Return value:    None.

    virtual void UnlockBitmaps() { release_bitmap(m_pMainBitmap); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the main bitmap of this SceneLayer.
//                  LockColorBitmap() must be called before using this method.
// Arguments:       The X and Y coordinates of which pixel to get.
// Return value:    An unsigned char specifying the requested pixel's value.

    unsigned char GetPixel(const int pixelX, const int pixelY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the main bitmap of this SceneLayer to a
//                  specific value. LockColorBitmap() must be called before using this
//                  method.
// Arguments:       The X and Y coordinates of which pixel to set.
//                  The value to set the pixel to.
// Return value:    None.

    void SetPixel(const int pixelX, const int pixelY, const unsigned char value);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsWithinBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether the integer coordinates passed in are within the
//                  bounds of this SceneLayer.
// Arguments:       Int coordinates.
// Return value:    Whether within bounds or not.

    bool IsWithinBounds(const int pixelX, const int pixelY, const int margin = 0)
    {
// TODO: This doesn't take Y wrapping into acocunt!$@#$
        return (m_WrapX || (pixelX >= -margin) && pixelX < (m_pMainBitmap->w + margin)) && pixelY >= -1000 && pixelY < (m_pMainBitmap->h + margin);
    }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  SceneLayer, depending on the wrap settings of this SceneLayer.
// Arguments:       The X and Y coordinates of the position to wrap, if needed.
//                  Whether the coordinates above are of this' scale factor, or in its
//                  native pixels.
// Return value:    Whether wrapping was performed or not. (Does not report on bounding)

    bool ForceBounds(int &posX, int &posY, bool scaled = true) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  SceneLayer, depending on the wrap settings of this SceneLayer.
// Arguments:       The vector coordinates of the position to wrap, if needed.
//                  Whether the coordinates above are of this' scale factor, or in its
//                  native pixels.
// Return value:    Whether wrapping was performed or not. (Does not report on bounding)

    bool ForceBounds(Vector &pos, bool scaled = true) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the SceneLayer
//                  and wrapping in the corresponding axes are turned on.
// Arguments:       The X and Y coordinates of the position to wrap, if needed.
//                  Whether the coordinates above are of this' scale factor, or in its
//                  native pixels.
// Return value:    Whether wrapping was performed or not.

    bool WrapPosition(int &posX, int &posY, bool scaled = true) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the SceneLayer
//                  and wrapping in the corresponding axes are turned on.
// Arguments:       The vector coordinates of the position to wrap, if needed.
//                  Whether the coordinates above are of this' scale factor, or in its
//                  native pixels.
// Return value:    Whether wrapping was performed or not.

    bool WrapPosition(Vector &pos, bool scaled = true) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SceneLayer. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneLayer's current scrolled position to a bitmap.
// Arguments:       The bitmap to draw to.
//                  The box on the target bitmap to limit drawing to, with the corner of
//                  box being where the scroll position lines up.
//                  If a non-{-1,-1} vector is passed, the internal scroll offset of this
//                  is overridder with it. It becomes the new source coordinates.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1)) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawScaled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneLayer's current scrolled position to a bitmap, but also
//                  scaled according to what has been set with SetScaleFactor.
// Arguments:       The bitmap to draw to.
//                  The box on the target bitmap to limit drawing to, with the corner of
//                  box being where the scroll position lines up.
//                  If a non-{-1,-1} vector is passed, the internal scroll offset of this
//                  is overridder with it. It becomes the new source coordinates.
// Return value:    None.

    virtual void DrawScaled(BITMAP *pTargetBitmap, Box &targetBox, const Vector &scrollOverride = Vector(-1, -1)) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitScrollRatios
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Initialize the scroll ratios from the encoded scroll info. Must be
//                  done after the bitmap has been created in the derived concrete classes.
// Arguments:       None.
// Return value:    None.

	void InitScrollRatios();

	void UpdateScrollRatiosForNetworkPlayer(int player);


    // Member variables
    static Entity::ClassInfo m_sClass;

    ContentFile m_BitmapFile;

    BITMAP *m_pMainBitmap;
    // Whether main bitmap is owned by this
    bool m_MainBitmapOwned;
    bool m_DrawTrans;
    Vector m_Offset;
    // The original scrollinfo with special encoded info that is then made into the actual scroll ratios
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


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneLayer, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    SceneLayer(const SceneLayer &reference) = delete;
    void operator=(const SceneLayer &rhs) = delete;

};

} // namespace RTE

#endif // File