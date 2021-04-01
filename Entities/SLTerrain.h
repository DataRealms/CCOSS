#ifndef _RTESLTERRAIN_
#define _RTESLTERRAIN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SLTerrain.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the SLTerrain class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SceneLayer.h"
#include "Matrix.h"
#include "Box.h"
#include "Material.h"

namespace RTE
{

class MOPixel;
class TerrainDebris;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SLTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The terrain of the RTE scene.
// Parent(s):       SceneLayer
// Class history:   01/08/2002 SLTerrain created.

class SLTerrain:
    public SceneLayer
{

    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    TerrainFrosting
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Specifies a 'frosting' layer of material on top of another material
    //                  in the terrain. Gets applied upon loading the scene.
    // Parent(s):       Serializable.
    // Class history:   04/04/2007 TerrainFrosting created.

    class TerrainFrosting:
        public Serializable
    {


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

		SerializableClassNameGetter
		SerializableOverrideMethods


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     TerrainFrosting
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a TerrainFrosting object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        TerrainFrosting() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     TerrainFrosting
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a TerrainFrosting object in system
    //                  memory, deep copying another.
    // Arguments:       None.

        TerrainFrosting(const TerrainFrosting &reference) { Clear(); Create(reference); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a TerrainFrosting to be identical to another, by deep copy.
    // Arguments:       A reference to the TerrainFrosting to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        int Create(const TerrainFrosting &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        void Reset() override { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetTargetMaterial
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the target Material that this TerrainFrosting will appear on top of.
    // Arguments:       None.
    // Return value:    A copy of the target Material

		Material &GetTargetMaterial() { return m_TargetMaterial; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetFrostingMaterial
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the Material of this TerrainFrosting.
    // Arguments:       None.
    // Return value:    A copy of the Material

		Material &GetFrostingMaterial() { return m_FrostingMaterial; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetThicknessSample
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Returns a random thickness sample between the min and max possible
    //                  thicknesses of this TerrainFrosting
    // Arguments:       None.
    // Return value:    The thickness sample.

		int GetThicknessSample() { return m_MinThickness + RandomNum(0, m_MaxThickness - m_MinThickness); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  InAirOnly
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Indicates whether the frosting only appears where there is air in the
    //                  terrain.
    // Arguments:       None.
    // Return value:    Whether only appears in air particles.

		bool InAirOnly() { return m_InAirOnly; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // The material this frosting will sit on top in the terrain
        Material m_TargetMaterial;
        // Material of this frosting that will be piled on top of the target
        Material m_FrostingMaterial;
        // The minimum height in pixels above the target material
        int m_MinThickness;
        // The max thickness
        int m_MaxThickness;
        // Whether the frosting only appears where there is air
        bool m_InAirOnly;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this TerrainFrosting, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(SLTerrain)
SerializableOverrideMethods
ClassInfoGetters


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SLTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SLTerrain object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SLTerrain() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SLTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SLTerrain object before deletion
//                  from system memory.
// Arguments:       None.

	~SLTerrain() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SLTerrain object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SLTerrain object ready for use.
// Arguments:       The path of the bitmap file to load as this SLTerrain's graphical
//                  representation.
//                  Whether to draw transperently using a color key specified by the pixel
//                  in the upper left corner of the loaded bitmap.
//                  The initial scroll offset.
//                  Two bools that define whether the layer should wrap around or stop when
//                  scrolling beyond its bitmap's boundries.
//                  A vector whose components defines two different things, depending on
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
	int Create(char *filename,
                       bool drawTrans,
                       Vector offset,
                       bool wrapX,
                       bool wrapY,
                       Vector scrollInfo);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SLTerrain to be identical to another, by deep copy.
// Arguments:       A reference to the SLTerrain to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const SLTerrain &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this SceneLayer.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadData() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves data currently in memory to disk.
// Arguments:       The filepath base to the where to save the Bitmap data. This means
//                  everything up to the extension. "FG" and "Mat" etc will be added.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int SaveData(std::string pathBase) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int ClearData() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsFileData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether this' bitmap data is loaded from a file or was generated.
// Arguments:       None.
// Return value:    Whether the data in this' bitmap was loaded from a datafile, or generated.

	bool IsFileData() const override { return m_pFGColor && m_pFGColor->IsFileData() && m_pBGColor && m_pBGColor->IsFileData(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SLTerrain, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); SceneLayer::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SLTerrain object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LockBitmaps
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Lock the internal bitmaps so it can be accessed by GetColorPixel() etc.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and unlock.
//                  UnlockBitmaps() should always be called after accesses are completed.
// Arguments:       None.
// Return value:    None.

	void LockBitmaps() override { SceneLayer::LockBitmaps(); acquire_bitmap(m_pMainBitmap); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UnlockBitmaps
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the internal bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockBitmaps() should only be called after LockBitmaps().
// Arguments:       None.
// Return value:    None.

	void UnlockBitmaps() override { SceneLayer::UnlockBitmaps(); release_bitmap(m_pMainBitmap); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFGColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the foreground color bitmap of this SLTerrain.
// Arguments:       None.
// Return value:    A pointer to the foreground color bitmap.

    BITMAP * GetFGColorBitmap() { return m_pFGColor->GetBitmap(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBGColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the background color bitmap of this SLTerrain.
// Arguments:       None.
// Return value:    A pointer to the foreground color bitmap.

    BITMAP * GetBGColorBitmap() { return m_pBGColor->GetBitmap(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the material bitmap of this SLTerrain.
// Arguments:       None.
// Return value:    A pointer to the material bitmap.

    BITMAP * GetMaterialBitmap() { return m_pMainBitmap; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the foreground color bitmap of this.
//                  LockBitmaps() must be called before using this method.
// Arguments:       The X and Y coordinates of which pixel to get.
// Return value:    An unsigned char specifying the requested pixel's BG Color.

    unsigned char GetFGColorPixel(const int pixelX, const int pixelY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the background color bitmap of this.
//                  LockBitmaps() must be called before using this method.
// Arguments:       The X and Y coordinates of which pixel to get.
// Return value:    An unsigned char specifying the requested pixel's BG Color.

    unsigned char GetBGColorPixel(const int pixelX, const int pixelY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the material bitmap of this SceneLayer.
//                  LockBitmaps() must be called before using this method.
// Arguments:       The X and Y coordinates of which pixel to get.
// Return value:    An unsigned char specifying the requested pixel's material index.

    unsigned char GetMaterialPixel(const int pixelX, const int pixelY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAirPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a terrain pixel is of air or cavity material.
// Arguments:       The X and Y coordinates of which pixel to check for airness.
// Return value:    A bool with the answer.

    bool IsAirPixel(const int pixelX, const int pixelY) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the FG Color bitmap of this SLTerrain to a
//                  specific color. LockBitmaps() must be called before using this
//                  method.
// Arguments:       The X and Y coordinates of which pixel to set.
//                  The color index to set the pixel to.
// Return value:    None.

    void SetFGColorPixel(const int pixelX, const int pixelY, const int color);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the BG Color bitmap of this SLTerrain to a
//                  specific color. LockBitmaps() must be called before using this
//                  method.
// Arguments:       The X and Y coordinates of which pixel to set.
//                  The color index to set the pixel to.
// Return value:    None.

    void SetBGColorPixel(const int pixelX, const int pixelY, const int color);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaterialPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the material bitmap of this SLTerrain to a
//                  specific material. LockMaterialBitmap() must be called before using this
//                  method.
// Arguments:       The X and Y coordinates of which pixel to set.
//                  The material index to set the pixel to.
// Return value:    None.

    void SetMaterialPixel(const int pixelX, const int pixelY, const unsigned char material);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetStructuralBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the structural bitmap of this Terrain.
// Arguments:       None.
// Return value:    A pointer to the material bitmap.

    BITMAP * GetStructuralBitmap() { return m_pStructural; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetToDrawMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether drawing the material layer instead of the normal color
//                  layer when drawing this SLTerrain.
// Arguments:       The setting, whether to draw the material later instead of the color
//                  layer or not.
// Return value:    None.

    bool GetToDrawMaterial() { return m_DrawMaterial; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EraseSilhouette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a BITMAP and scans through the pixels on this terrain for pixels
//                  which overlap with it. Erases them from the terrain and can optionally
//                  generate MOPixel:s based on the erased or 'dislodged' terrain pixels.
// Arguments:       A pointer to the source BITMAP whose rotozoomed silhouette will be used
//                  as a cookie-cutter on the terrain.
//                  The position coordinates of the sprite.
//                  The sprite's current rotation in radians.
//                  The sprite's current scale coefficient
//                  Whether to generate any MOPixel:s from the erased terrain pixels.
//                  How many pixels to skip making MOPixels from, between each that gets
//                  made. 0 means every pixel turns into an MOPixel.
//                  The max number of MOPixels to make, if they are to be made.
// Return value:    A deque filled with the MOPixel:s of the terrain that are now dislodged.
//                  Note that ownership of all the MOPixel:s in the deque ARE transferred!
//                  This will be empty if makeMOPs is false.

    std::deque<MOPixel *> EraseSilhouette(BITMAP *pSprite,
                                          Vector pos,
                                          Vector pivot,
                                          Matrix rotation,
                                          float scale,
                                          bool makeMOPs = true,
                                          int skipMOP = 2,
                                          int maxMOPs = 150);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetToDrawMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether to draw the material layer instead of the normal color
//                  layer when drawing this SLTerrain.
// Arguments:       The setting, whether to draw the material later instead of the color
//                  layer or not.
// Return value:    None.

    void SetToDrawMaterial(bool drawMaterial) { m_DrawMaterial = drawMaterial; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in Object's graphical and material representations to
//                  this Terrain's respective layers.
// Arguments:       The Object to apply to this Terrain. Ownership is NOT xferred!
// Return value:    Whether successful or not.

	bool ApplyObject(Entity *pEntity);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyMovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in MovableObject's graphical and material
//                  representations to this Terrain's respective layers.
// Arguments:       The MovableObject to apply to this Terrain. Ownership is NOT xferred!
// Return value:    None.

	void ApplyMovableObject(MovableObject *pMObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyTerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in TerrainObject's graphical and material
//                  representations to this Terrain's respective layers.
// Arguments:       The TerrainObject to apply to this Terrain. Ownership is NOT xferred!
// Return value:    None.

	void ApplyTerrainObject(TerrainObject *pTObject);


	void RegisterTerrainChange(TerrainObject *pTObject);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddUpdatedMaterialArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a notification that an area of the material terrain has been
//                  updated.
// Arguments:       The Box defining the newly updated material area that can be unwrapped
//                  and may be out of bounds of the scene.
// Return value:    None.

    void AddUpdatedMaterialArea(const Box &newArea) { m_UpdatedMateralAreas.push_back(newArea); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUpdatedMaterialAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a list of unwrapped boxes which show the areas where the material
//                  layer has had objects applied to it since last call to ClearUpdatedAreas().
// Arguments:       None.
// Return value:    Reference to the list that has been filled with Box:es which are
//                  unwrapped and may be out of bounds of the scene!

    std::list<Box> & GetUpdatedMaterialAreas() { return m_UpdatedMateralAreas; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBoxBuried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether a bounding box is completely buried in the terrain.
// Arguments:       The box to check.
// Return value:    Whether the box is completely buried., ie no corner sticks out in the
//                  air.

    bool IsBoxBuried(const Box &checkBox) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearUpdatedAreas
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of updated areas in the material layer.
// Arguments:       None.
// Return value:    None.

    void ClearUpdatedAreas() { m_UpdatedMateralAreas.clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanAirBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any color pixel in the color layer of this SLTerrain wherever
//                  there is an air material pixel in the material layer inside the specified box.
// Arguments:       Box to clean. Whether the scene is X-wrapped or Y-wrapped.
// Return value:    None.

    void CleanAirBox(Box box, bool wrapsX, bool wrapsY);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanAir
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any color pixel in the color layer of this SLTerrain wherever
//                  there is an air material pixel in the material layer.
// Arguments:       None.
// Return value:    None.

    void CleanAir();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearAllMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any FG and material pixels completely form teh terrain.
//                  For the editor mostly.
// Arguments:       None.
// Return value:    None.

    void ClearAllMaterial();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SLTerrain. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawDrawBackground
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SLTerrain's background layer's current scrolled position
//                  to a bitmap.
// Arguments:       The bitmap to draw to.
//                  The box on the target bitmap to limit drawing to, with the corner of
//                  box being where the scroll position lines up.
//                  If a non-{-1,-1} vector is passed, the internal scroll offset of this
//                  is overridder with it. It becomes the new source coordinates.
// Return value:    None.

	void DrawBackground(BITMAP *pTargetBitmap, Box& targetBox, const Vector &scrollOverride = Vector(-1, -1));


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SLTerrain's foreground's current scrolled position to a bitmap.
// Arguments:       The bitmap to draw to.
//                  The box on the target bitmap to limit drawing to, with the corner of
//                  box being where the scroll position lines up.
//                  If a non-{-1,-1} vector is passed, the internal scroll offset of this
//                  is overridder with it. It becomes the new source coordinates.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, Box& targetBox, const Vector &scrollOverride = Vector(-1, -1)) const override;

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    SceneLayer *m_pFGColor;
    SceneLayer *m_pBGColor;
    BITMAP *m_pStructural;
    ContentFile m_BGTextureFile;

    std::list<TerrainFrosting> m_TerrainFrostings;
    std::list<TerrainDebris *> m_TerrainDebris;
    std::list<TerrainObject *> m_TerrainObjects;

    // List of areas of the material layer which have been affected by the updating of new objects copied to it
    // These boxes are NOT wrapped, and can be out of bounds!
    std::list<Box> m_UpdatedMateralAreas;

    // Draw the material layer instead of the color layer.
    bool m_DrawMaterial;

    // Intermediate test layers, deffernt sizes for efficiency
    static BITMAP *m_spTempBitmap16;
    static BITMAP *m_spTempBitmap32;
    static BITMAP *m_spTempBitmap64;
    static BITMAP *m_spTempBitmap128;
    static BITMAP *m_spTempBitmap256;
    static BITMAP *m_spTempBitmap512;

	// Indicates, that before processing frostings-related properties for this terrain
	// derived list with frostings must be cleared to avoid duplication when loading scenes
	bool m_NeedToClearFrostings;
	// Indicates, that before processing debris-related properties for this terrain
	// derived list with debris must be cleared to avoid duplication when loading scenes
	bool m_NeedToClearDebris;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SLTerrain, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	SLTerrain(const SLTerrain &reference) = delete;
	SLTerrain & operator=(const SLTerrain &rhs) = delete;

};

} // namespace RTE

#endif // File